#include "syscalls/handler.h"
#include "schedulers/scheduler.h"
#include "memory/vmm.h"
#include "memory/pmm.h"
#include "memory/vma.h"
#include "memory/heap.h"
#include "utils/elf.h"
#include "utils/string.h"
#include "utils/vga.h"

extern Scheduler scheduler;

int sys_execve(const char *path, char *const argv[], char *const envp[], InterruptRegisters *regs) {
    Task *task = getCurrentTask();
    char full_path[MAX_PATH];
    resolvePath(path, task->cwd, full_path);
    // Save arguments to kernel
    int argc = 0;
    if (argv) while (argv[argc]) argc++;
    char **kargv = NULL;
    if (argc > 0) {
        kargv = (char**)kmalloc(argc * sizeof(char*));
        for (int i = 0; i < argc; i++) kargv[i] = kstrdup(argv[i]);
    }
    int envc = 0;
    if (envp) while (envp[envc]) envc++;
    char **kenvp = NULL;
    if (envc > 0) {
        kenvp = (char**)kmalloc(envc * sizeof(char*));
        for (int i = 0; i < envc; i++) kenvp[i] = kstrdup(envp[i]);
    }
    // Free user-space VMAs
    Vma *curr_vma = task->vma_list;
    while (curr_vma) {
        Vma *next = curr_vma->next;
        for (uint32_t a = curr_vma->virt_addr; a < curr_vma->virt_addr + curr_vma->size; a += PAGE_SIZE) {
            unmapPage(task->pageDirectory, a);
        }
        kfree(curr_vma);
        curr_vma = next;
    }
    task->vma_list = NULL;
    // Load the new ELF binary
    uint32_t entry;
    if (loadElf(task, full_path, &entry) < 0) {
        printk("sys_execve: Failed to load ELF %s\n", full_path);
        // Cleanup kernel buffers if failed
        if (kargv) {
            for (int i = 0; i < argc; i++) kfree(kargv[i]);
            kfree(kargv);
        }
        if (kenvp) {
            for (int i = 0; i < envc; i++) kfree(kenvp[i]);
            kfree(kenvp);
        }
        sys_exit(-1);
        return -1;
    }
    // Re-initialize structures
    task->heap_start = USER_HEAP_START;
    task->heap_break = task->heap_start;
    addVma(task, task->heap_start, 0, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS);
    uint32_t user_phys_top;
    if (!allocateStack(task->pageDirectory, USER_STACK_BASE, USER_STACK_SIZE, PAGE_RW | PAGE_USER, &user_phys_top)) {
        sys_exit(-1);
        return -1;
    }
    task->ustack_top = USER_STACK_TOP;
    addVma(task, USER_STACK_BASE, USER_STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE);
    // Push arguments to user stack
    uint32_t esp = USER_STACK_TOP;
    uint32_t *uargv_ptrs = (uint32_t*)kmalloc(argc * sizeof(uint32_t));
    uint32_t *uenvp_ptrs = (uint32_t*)kmalloc(envc * sizeof(uint32_t));
    // Copy envp strings
    for (int i = envc - 1; i >= 0; i--) {
        size_t len = strlen(kenvp[i]) + 1;
        esp -= len;
        memcpy((void*)esp, kenvp[i], len);
        uenvp_ptrs[i] = esp;
        kfree(kenvp[i]);
    }
    if (kenvp) kfree(kenvp);
    // Copy argv strings
    for (int i = argc - 1; i >= 0; i--) {
        size_t len = strlen(kargv[i]) + 1;
        esp -= len;
        memcpy((void*)esp, kargv[i], len);
        uargv_ptrs[i] = esp;
        kfree(kargv[i]);
    }
    if (kargv) kfree(kargv);
    // Align stack
    esp &= ~0x3;
    // Push ptrs
    esp -= 4; *(uint32_t*)esp = 0; // envp NULL
    for (int i = envc - 1; i >= 0; i--) {
        esp -= 4; *(uint32_t*)esp = uenvp_ptrs[i];
    }
    uint32_t uenvp = esp;
    esp -= 4; *(uint32_t*)esp = 0; // argv NULL
    for (int i = argc - 1; i >= 0; i--) {
        esp -= 4; *(uint32_t*)esp = uargv_ptrs[i];
    }
    uint32_t uargv = esp;
    kfree(uargv_ptrs);
    kfree(uenvp_ptrs);
    // Initial stack frame for main(argc, argv, envp)
    esp -= 4; *(uint32_t*)esp = uenvp;
    esp -= 4; *(uint32_t*)esp = uargv;
    esp -= 4; *(uint32_t*)esp = (uint32_t)argc;
    esp -= 4; *(uint32_t*)esp = 0; // dummy return address
    // Update registers
    regs->eip = entry;
    regs->useresp = esp;
    regs->cs = 0x1B;
    regs->ss = 0x23;
    regs->ds = 0x23;
    regs->es = 0x23;
    regs->fs = 0x23;
    regs->gs = 0x23;
    regs->eax = 0;
    regs->ebx = 0;
    regs->ecx = 0;
    regs->edx = 0;
    regs->esi = 0;
    regs->edi = 0;
    regs->ebp = 0;
    return 0;
}
