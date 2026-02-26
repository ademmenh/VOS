#include "utils/elf.h"
#include "storage/vfs.h"
#include "memory/vmm.h"
#include "memory/pmm.h"
#include "memory/vma.h"
#include "memory/heap.h"
#include "schedulers/task.h"
#include "utils/string.h"
#include "utils/vga.h"
#include "syscalls/handler.h"

extern VfsMount* vfs_root;

static uint32_t elfToVmmFlags(uint32_t p_flags) {
    uint32_t flags = PAGE_USER | PAGE_PRESENT;
    if (p_flags & PF_W) flags |= PAGE_RW;
    return flags;
}

static int elfToProtFlags(uint32_t p_flags) {
    int prot = PROT_NONE;
    if (p_flags & PF_R) prot |= PROT_READ;
    if (p_flags & PF_W) prot |= PROT_WRITE;
    if (p_flags & PF_X) prot |= PROT_EXEC;
    return prot;
}

int loadElf(Task *task, const char *path, uint32_t *entry_out) {
    VfsNode *node = openVfsPath(vfs_root, path);
    if (!node) {
        printk("ELF Error: Could not open %s\n", path);
        return -1;
    }

    Elf32Ehdr ehdr;
    if (readVfsNode(node, 0, sizeof(Elf32Ehdr), (uint8_t*)&ehdr) != sizeof(Elf32Ehdr)) {
        printk("ELF Error: Could not read Ehdr\n");
        return -1;
    }

    // Validate magic
    if (ehdr.e_ident[EI_MAG0] != ELF_MAG0 ||
        ehdr.e_ident[EI_MAG1] != ELF_MAG1 ||
        ehdr.e_ident[EI_MAG2] != ELF_MAG2 ||
        ehdr.e_ident[EI_MAG3] != ELF_MAG3) {
        printk("ELF Error: Invalid magic number\n");
        return -1;
    }

    if (ehdr.e_ident[EI_CLASS] != ELFCLASS32) {
        printk("ELF Error: Not a 32-bit ELF\n");
        return -1;
    }

    if (ehdr.e_type != ET_EXEC) {
        printk("ELF Error: Not an executable file\n");
        return -1;
    }

    for (int i = 0; i < ehdr.e_phnum; i++) {
        Elf32Phdr phdr;
        uint32_t offset = ehdr.e_phoff + (i * ehdr.e_phentsize);
        if (readVfsNode(node, offset, sizeof(Elf32Phdr), (uint8_t*)&phdr) != sizeof(Elf32Phdr)) {
            printk("ELF Error: Could not read Phdr %d\n", i);
            return -1;
        }

        if (phdr.p_type == PT_LOAD) {
            uint32_t vaddr = phdr.p_vaddr;
            uint32_t memsz = phdr.p_memsz;
            uint32_t filesz = phdr.p_filesz;
            uint32_t file_off = phdr.p_offset;
            // Align vaddr and calculate size including alignment overhead
            uint32_t vaddr_aligned = vaddr & PAGE_MASK;
            uint32_t offset_in_page = vaddr & PAGE_OFFSET_MASK;
            uint32_t total_size = (memsz + offset_in_page + PAGE_OFFSET_MASK) & PAGE_MASK;
            // Register VMA
            addVma(task, vaddr_aligned, total_size, elf_to_prot_flags(phdr.p_flags), MAP_PRIVATE);
            // Allocate and map pages
            uint32_t vmm_flags = elf_to_vmm_flags(phdr.p_flags);
            for (uint32_t v = 0; v < total_size; v += PAGE_SIZE) {
                int frame = allocPhysicalPage();
                if (frame < 0) return -1;
                uint32_t phys = (uint32_t)frame * PAGE_SIZE;
                mapPage(task->pageDirectory, vaddr_aligned + v, phys, vmm_flags);
                // Zero-fill page first
                memset((void*)physicalToVirtual(phys), 0, PAGE_SIZE);
                // Copy data from file if within filesz
                if (v + PAGE_SIZE > offset_in_page && v < offset_in_page + filesz) {
                    uint32_t page_data_off = (v < offset_in_page) ? offset_in_page - v : 0;
                    uint32_t file_data_off = (v < offset_in_page) ? 0 : v - offset_in_page;
                    uint32_t bytes_to_read = filesz - file_data_off;
                    if (bytes_to_read > PAGE_SIZE - page_data_off) bytes_to_read = PAGE_SIZE - page_data_off;
                    readVfsNode(node, file_off + file_data_off, bytes_to_read, (uint8_t*)physicalToVirtual(phys) + page_data_off);
                }
            }
        }
    }
    *entry_out = ehdr.e_entry;
    return 0;
}
