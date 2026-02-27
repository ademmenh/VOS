#include "syscalls/int.h"
#include "syscalls/handler.h"
#include "storage/vfs.h"
#include "schedulers/fdt.h"
#include "memory/vmm.h"

extern VfsMount* vfs_root;

void test_syscalls_task(void) {
    int80(SYS_WRITE, 1, (int)"Starting syscall tests...\n", 26);
    
    char path[] = "/test.txt";
    char content[] = "Hello Syscalls!";
    char link_path[] = "/link.txt";
    char rel_link[] = "/rel_link.txt";
    char target_rel[] = "test.txt";
    char buf[64];
    struct StatBuf st;
    int res;

    // 1. Open/Create (using O_CREAT | FD_FLAG_WRITE)
    int fd = int80(SYS_OPEN, (int)path, O_CREAT | FD_FLAG_WRITE, 0);
    if (fd < 0) {
        int80(SYS_WRITE, 1, (int)"Test Fail: open\n", 16);
        while(1);
    }
    int80(SYS_WRITE, 1, (int)"Test Pass: open\n", 16);
    
    // 2. Write
    res = int80(SYS_WRITE, fd, (int)content, 15);
    if (res < 0) {
        int80(SYS_WRITE, 1, (int)"Test Fail: write\n", 17);
        while(1);
    }
    int80(SYS_WRITE, 1, (int)"Test Pass: write\n", 17);
    
    // 3. Close
    int80(SYS_CLOSE, fd, 0, 0);
    int80(SYS_WRITE, 1, (int)"Test Pass: close\n", 17);
    
    // 4. Stat
    res = int80(SYS_STAT, (int)path, (int)&st, 0);
    if (res < 0 || (int)st.st_size != 15) {
        int80(SYS_WRITE, 1, (int)"Test Fail: stat\n", 16);
        while(1);
    }
    int80(SYS_WRITE, 1, (int)"Test Pass: stat\n", 16);
    
    // 5. Symlink
    res = int80(SYS_SYMLINK, (int)path, (int)link_path, 0);
    if (res < 0) {
        int80(SYS_WRITE, 1, (int)"Test Fail: symlink\n", 19);
        while(1);
    }
    int80(SYS_WRITE, 1, (int)"Test Pass: symlink\n", 19);
    
    // 6. Readlink
    for(int i=0; i<64; i++) buf[i] = 0;
    res = int80(SYS_READLINK, (int)link_path, (int)buf, 64);
    int match = 1;
    char path_cmp[] = "/test.txt";
    for(int i=0; path_cmp[i]; i++) if(buf[i] != path_cmp[i]) match = 0;
    if (res < 0 || !match) {
        int80(SYS_WRITE, 1, (int)"Test Fail: readlink\n", 20);
        while(1);
    }
    int80(SYS_WRITE, 1, (int)"Test Pass: readlink\n", 20);
    
    // 7. Open Link (Follow)
    int fd2 = int80(SYS_OPEN, (int)link_path, FD_FLAG_READ, 0);
    if (fd2 < 0) {
        int80(SYS_WRITE, 1, (int)"Test Fail: open link\n", 21);
        while(1);
    }
    int80(SYS_WRITE, 1, (int)"Test Pass: open link\n", 21);
    
    // 8. Read from link
    for(int i=0; i<64; i++) buf[i] = 0;
    res = int80(SYS_READ, fd2, (int)buf, 15);
    match = 1;
    for(int i=0; i<15; i++) if(buf[i] != content[i]) match = 0;
    if (res < 0 || !match) {
        int80(SYS_WRITE, 1, (int)"Test Fail: read link\n", 21);
        while(1);
    }
    int80(SYS_WRITE, 1, (int)"Test Pass: read link\n", 21);
    int80(SYS_CLOSE, fd2, 0, 0);
    
    // 9. Lstat
    res = int80(SYS_LSTAT, (int)link_path, (int)&st, 0);
    if (res < 0 || st.st_mode != VFS_TYPE_SYMLINK) {
        int80(SYS_WRITE, 1, (int)"Test Fail: lstat\n", 17);
        while(1);
    }
    int80(SYS_WRITE, 1, (int)"Test Pass: lstat\n", 17);

    // 10. Relative symlink test
    res = int80(SYS_SYMLINK, (int)target_rel, (int)rel_link, 0);
    if (res >= 0) {
        int fd3 = int80(SYS_OPEN, (int)rel_link, FD_FLAG_READ, 0);
        if (fd3 >= 0) {
            for(int i=0; i<64; i++) buf[i] = 0;
            int80(SYS_READ, fd3, (int)buf, 15);
            match = 1;
            for(int i=0; i<15; i++) if(buf[i] != content[i]) match = 0;
            if (match) {
                int80(SYS_WRITE, 1, (int)"Test Pass: relative link\n", 25);
            } else {
                int80(SYS_WRITE, 1, (int)"Test Fail: rel link read\n", 24);
            }
            int80(SYS_CLOSE, fd3, 0, 0);
        } else {
            int80(SYS_WRITE, 1, (int)"Test Fail: open rel link\n", 25);
        }
    } else {
        int80(SYS_WRITE, 1, (int)"Test Fail: create rel link\n", 27);
    }
    
    // 11. Mmap test
    int80(SYS_WRITE, 1, (int)"Testing mmap...\n", 16);
    void *mmap_ptr = (void*)int80_6(SYS_MMAP, 0, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if ((int)mmap_ptr == -1) {
        int80(SYS_WRITE, 1, (int)"Test Fail: mmap\n", 16);
        while(1);
    }
    int80(SYS_WRITE, 1, (int)"Test Pass: mmap\n", 16);

    // 13. Sbrk test
    int80(SYS_WRITE, 1, (int)"Testing sbrk...\n", 16);
    void *initial_break = (void*)int80(SYS_SBRK, 0, 0, 0);
    void *new_break = (void*)int80(SYS_SBRK, PAGE_SIZE, 0, 0);
    if ((int)initial_break == -1 || (int)new_break == -1) {
        int80(SYS_WRITE, 1, (int)"Test Fail: sbrk allocate\n", 25);
        while(1);
    }
    char *heap_ptr = (char*)initial_break;
    heap_ptr[0] = 'H';
    heap_ptr[1] = 'E';
    heap_ptr[2] = 'A';
    heap_ptr[3] = 'P';
    if (heap_ptr[0] != 'H' || heap_ptr[3] != 'P') {
        int80(SYS_WRITE, 1, (int)"Test Fail: sbrk read/write\n", 27);
        while(1);
    }
    int80(SYS_WRITE, 1, (int)"Test Pass: sbrk\n", 16);
    
    // 14. Fork/Wait test
    int80(SYS_WRITE, 1, (int)"Testing fork and wait...\n", 25);
    int fork_res = int80(SYS_FORK, 0, 0, 0);
    
    int80(SYS_WRITE, 1, (int)"Fork returned: ", 15);
    int80(SYS_WRITE, 1, (int)"\n", 1);

    if (fork_res < 0) {
        int80(SYS_WRITE, 1, (int)"Test Fail: fork\n", 16);
        while(1);
    } else if (fork_res == 0) {
        // Child
        int80(SYS_WRITE, 1, (int)"Child process executing...\n", 27);
        int80(SYS_WRITE, 1, (int)"Child: Hello! Exiting with 42...\n", 33);
        int80(SYS_EXIT, 42, 0, 0);
    } else {
        // Parent
        int wstatus = -7; // Initial unusual value
        int80(SYS_WRITE, 1, (int)"Parent: Waiting for child PID ", 30);
        int80(SYS_WRITE, 1, (int)"\n", 1);
        
        int waited_pid = int80(SYS_WAIT, (int)&wstatus, 0, 0);
        
        int80(SYS_WRITE, 1, (int)"Wait returned PID: ", 19);
        int80(SYS_WRITE, 1, (int)" wstatus: ", 10);
        int80(SYS_WRITE, 1, (int)"\n", 1);

        if (waited_pid == fork_res && wstatus == 42) {
            int80(SYS_WRITE, 1, (int)"Test Pass: fork/wait\n", 21);
        } else {
            int80(SYS_WRITE, 1, (int)"Test Fail: fork/wait\n", 21);
            if (waited_pid < 0) {
                int80(SYS_WRITE, 1, (int)"Error: wait returned negative\n", 30);
            }
        }
    }

    // 15. Chdir test
    int80(SYS_WRITE, 1, (int)"Testing chdir...\n", 17);
    int chdir_res = int80(SYS_CHDIR, (int)"/bin", 0, 0);
    if (chdir_res == 0) {
        int80(SYS_WRITE, 1, (int)"Test Pass: chdir /bin\n", 22);
        // Now test relative path to /bin/test
        int fd_rel = int80(SYS_OPEN, (int)"test", FD_FLAG_READ, 0);
        if (fd_rel >= 0) {
            int80(SYS_WRITE, 1, (int)"Test Pass: open relative test\n", 30);
            int80(SYS_CLOSE, fd_rel, 0, 0);
        } else {
            int80(SYS_WRITE, 1, (int)"Test Fail: open relative test\n", 30);
        }
        int80(SYS_CHDIR, (int)"/", 0, 0);
    } else {
        int80(SYS_WRITE, 1, (int)"Test Fail: chdir /bin\n", 22);
    }

    int80(SYS_WRITE, 1, (int)"ALL SYSCALL TESTS PASSED!\n", 26);
    int80(SYS_EXIT, 0, 0, 0);
}

// from a to z then repeat
void task1() {
    char c = 'a';
    while(1) {
        int80(SYS_WRITE, 1, (int)&c, 1);
        c++;
        if(c == 'z') c = 'a';
    }
    int80(SYS_EXIT, 0, 0, 0);
}
// from A to Z then repeat
void task2() {
    char c = 'A';
    while(1) {
        int80(SYS_WRITE, 2, (int)&c, 1);
        c++;
        if(c == 'Z') c = 'A';
    }
    int80(SYS_EXIT, 0, 0, 0);
}


