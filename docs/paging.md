# 🔵 PHYSICAL RAM LAYOUT

> Physical memory map for the VOS 32-bit x86 kernel structure.

| Block Region | Address Range | Size | Description |
| :--- | :--- | :--- | :--- |
| **Low Memory** | `0x00000000` | 1MB | BIOS, GRUB info, unused lower memory |
| **Kernel Text/Data** | `0x00100000` | Variable | The main kernel loaded directly by GRUB |
| **Kernel End** | `~0x00124000` | N/A | Symbol `KERNEL_END` bounds |
| **Page Directory** | `0x00106000` | 4KB | `pageDirectory` (Mapped to `0xC0106000`) |
| **Page Table** | `0x00107000` | 4KB | `pageTable` (Mapped to `0xC0107000`) |
| **Stack Table** | `0x00108000` | 4KB | `stackPageTable` (Mapped to `0xC0108000`) |
| **Stack Frames** | `>=0x00109000` | Variable | Raw physical frames reserved for the stack |

*Note: The stack is allocated immediately after the kernel's strictly used physical frames, making it contiguous in initial layout.*

---

# 🟢 VIRTUAL MEMORY LAYOUT

> The 4GB virtual address space is properly segmented, moving the kernel to the higher-half (`0xC0000000`+) and placing the kernel stack right below the recursive mapping zone.

| Diagram Map | Virtual Address | Points To | Notes |
| :--- | :--- | :--- | :--- |
| **`[ PDE Rec. ]`** | `0xFFFFF000` | `0x00106000` | Recursive mapping base (Page Directory mapped to itself) |
| **`[ PTE Rec. ]`** | `0xFFC00000` | Various | Recursive mapping of all Page Tables |
| **`[ Stack T. ]`** | `0xFFBFFFFF` | - | `KERNEL_STACK_TOP` (Grows downward) |
| **`[ Stack P4 ]`** | `0xFFBFF000` | Stack Page 4 | Example top stack frame mapping |
| **`[ Stack P3 ]`** | `0xFFBFE000` | Stack Page 3 | Next stack frame |
| **`[ Stack P2 ]`** | `0xFFBFD000` | Stack Page 2 | Next stack frame |
| **`[ Stack P1 ]`** | `0xFFBFC000` | Stack Page 1 | Base stack frame |
| `[ -- Unmapped -- ]` | ... | ... | Huge unmapped chasm for dynamic memory |
| **`[ Kernel Data ]`**| `0xC0108000` | `0x00108000` | Stack Page Table |
| **`[ Kernel Data ]`**| `0xC0107000` | `0x00107000` | Kernel Page Table |
| **`[ Kernel Data ]`**| `0xC0106000` | `0x00106000` | Kernel Page Directory |
| **`[ Kernel Image ]`**| `0xC0001000` | `0x00001000` | Standard higher-half offset (`0x1000` is missing lowest MB) |
| **`[ KERNEL_START ]`**| `0xC0000000` | `0x00000000` | Base offset for higher-half kernel |
| `[ -- Removed -- ]`  | `0x00000000` | - | Lower identity mapping is completely removed |

---

# 🟣 PAGE DIRECTORY STRUCTURE

> `pageDirectory` resides at physical address `0x00106000`.

| Index | Virtual Range Mapped | Target Pointer | Role |
| :--- | :--- | :--- | :--- |
| **`0`** | `0x00000000 - 0x003FFFFF` | None | Initial identity mapping (now cleared out) |
| **`1 - 767`** | `0x00400000 - 0xBFFFFFFF` | None | User space & application regions (unmapped) |
| **`768`** | `0xC0000000 - 0xC03FFFFF` | `pageTable` | Maps the kernel binary + core data |
| **`769 - 1021`** | `0xC0400000 - 0xFF7FFFFF` | None | High memory reserved for Kernel Heap (VMM) |
| **`1022`** | `0xFF800000 - 0xFFBFFFFF` | `stackPT` | The Kernel Stack space (`0xFFC00000` downward mapping) |
| **`1023`** | `0xFFC00000 - 0xFFFFFFFF` | Self | The Recursive Mapping entry for fast PT edits |

---

# 🟡 KERNEL PAGE TABLE 

> `pageTable` resides at physical address `0x00107000` and covers index `768`.

This table maps the 4MB range from `0xC0000000` upwards directly to back to the lowest physical RAM.

- `0xC0000000` ➞ `0x00000000`
- `0xC0001000` ➞ `0x00001000`
- `...`
- `0xC03FF000` ➞ `0x003FF000`
