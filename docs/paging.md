# 🔵 PHYSICAL RAM LAYOUT (UPDATED)

```
Physical Memory (RAM)
================================================================

0x00000000  ─────────────────────────────────────
              BIOS / low memory / unused

0x00100000  ─────────────────────────────────────
              Kernel loaded by GRUB

0x00124XXX  ─────────────────────────────────────
              Kernel end

0x00106000  ─────────────────────────────────────
              pageDirectory (4KB)
              physical = 0x00106000
              virtual  = 0xC0106000

0x00107000  ─────────────────────────────────────
              pageTable (4KB)
              physical = 0x00107000
              virtual  = 0xC0107000

0x00108000  ─────────────────────────────────────
              stackPageTable (4KB)
              physical = 0x00108000
              virtual  = 0xC0108000

0xXXXXXXXX  ─────────────────────────────────────
              Stack page 1

0xXXXXXXXX  ─────────────────────────────────────
+4k           Stack page 2

0xXXXXXXXX  ─────────────────────────────────────
+8k           Stack page 3

0xXXXXXXXX  ─────────────────────────────────────
+12k           Stack page 4

================================================================
```

0xXXXXXXXX is the first aligned address after the kernel code ends. 

The stack is allocated **immediately after the kernel’s used physical frames**✔ 

---

# 🟢 VIRTUAL MEMORY LAYOUT (4GB space)

```
Virtual Address Space (4GB)
================================================================

0xFFFFFFFF  ────────────────────────────────
              Stack top
              ↓ grows downward

0xFFFFF000  → physical address stack page 4
0xFFFFE000  → physical address stack page 3
0xFFFFD000  → physical address stack page 2
0xFFFFC000  → physical address stack page 1

────────────────────────────────────────────
(unmapped space)
────────────────────────────────────────────

virtual address of stackPageTable   → physical address stackPageTable
virtual address of pageTable        → physical address pageTable
virtual address of pageDirectory    → physical address pageDirectory

0xC0001000  → 0x00001000
0xC0000000  → 0x00000000
────────────────────────────────────────────

0x00000000 – identity mapping REMOVED
================================================================
```

---

# 🟣 Page Directory Structure

```
pageDirectory (physical 0x00106000)

Index      Meaning
------------------------------------------------
0          0  (identity removed)
1–767      0
768        →  (kernel page table)
769–1022   0
1023       →  (kernel stack page table)
```

---

# 🟡 Page Table (Kernel)

```
pageTable (phys 0x00107000)

Maps:
0xC0000000 → 0x00000000
0xC0001000 → 0x00001000
...
0xC03FF000 → 0x003FF000
```

Kernel virtual space mirrors first 4MB of RAM.

---


The stack is now physically contiguous with the kernel allocation region, matching your latest paging dump exactly.
