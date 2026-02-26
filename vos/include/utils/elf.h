#ifndef ELF_H
#define ELF_H

#include <stdint.h>

#define EI_NIDENT		16
#define EI_MAG0			0
#define EI_MAG1			1
#define EI_MAG2			2
#define EI_MAG3			3
#define EI_CLASS		4
#define EI_DATA			5
#define EI_VERSION		6
#define EI_OSABI		7
#define EI_ABIVERSION	8

#define ELF_MAG0		0x7f
#define ELF_MAG1		'E'
#define ELF_MAG2		'L'
#define ELF_MAG3		'F'

#define ELFCLASS32		1			// 32-bit architecture
#define ELFDATA2LSB		1			// Little endian
#define EV_CURRENT		1			// Current ELF version

// ELF File Types
#define ET_NONE			0
#define ET_REL			1
#define ET_EXEC			2			// Executable file
#define ET_DYN			3			// Shared object file
#define ET_CORE			4

// ELF Machines
#define EM_386			3			// Intel 80386

// Program Header Types
#define PT_NULL			0
#define PT_LOAD			1			// Loadable segment
#define PT_DYNAMIC		2
#define PT_INTERP		3
#define PT_NOTE			4
#define PT_SHLIB		5
#define PT_PHDR			6

// Program Header Flags
#define PF_X			(1 << 0)	// Executable
#define PF_W			(1 << 1)	// Writable
#define PF_R			(1 << 2)	// Readable

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf32_Word;

typedef struct {
	unsigned char	e_ident[EI_NIDENT];	// Magic number and metadata (arch, endianness)
	Elf32_Half		e_type;				// Object file type (ET_EXEC, ET_DYN, etc.)
	Elf32_Half		e_machine;			// Required architecture (EM_386)
	Elf32_Word		e_version;			// Object file version
	Elf32_Addr		e_entry;			// Entry point virtual address
	Elf32_Off		e_phoff;			// Program header table offset in file
	Elf32_Off		e_shoff;			// Section header table offset in file
	Elf32_Word		e_flags;			// Processor-specific flags
	Elf32_Half		e_ehsize;			// size of this ELF header
	Elf32_Half		e_phentsize;		// size of one program header table entry
	Elf32_Half		e_phnum;			// Number of program header table entries
	Elf32_Half		e_shentsize;		// size of one section header table entry
	Elf32_Half		e_shnum;			// Number of section header table entries
	Elf32_Half		e_shstrndx;			// Section header string table index
} Elf32Ehdr;

typedef struct {
	Elf32_Word		p_type;				// Segment type (PT_LOAD, PT_NOTE, etc.)
	Elf32_Off		p_offset;			// Offset of segment in file
	Elf32_Addr		p_vaddr;			// Virtual address to map segment to
	Elf32_Addr		p_paddr;			// Physical address (usually ignored)
	Elf32_Word		p_filesz;			// Size of segment in file
	Elf32_Word		p_memsz;			// Size of segment in memory (can be > filesz)
	Elf32_Word		p_flags;			// Segment permissions (PF_R, PF_W, PF_X)
	Elf32_Word		p_align;			// Segment alignment in memory and file
} Elf32Phdr;

#endif
