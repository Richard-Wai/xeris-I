/* ATRI8                                     */
/* ATRIA Prototyping, Packing and Debug tool */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */

#ifndef ELFDEF_H
#define ELFDEF_H

#include <stdint.h>

// ELF record structures and definitions

typedef uint32_t    Elf32_Addr;
typedef uint16_t    Elf32_Half;
typedef uint32_t    Elf32_Off;
typedef int32_t     Elf32_Sword;
typedef uint32_t    Elf32_Word;

/*

    ELF Header Structure
    
*/

#define EI_NIDENT   16      // Size of e_ident[]

typedef struct
{
    
    unsigned char   e_ident[EI_NIDENT];
    Elf32_Half      e_type;
    Elf32_Half      e_machine;
    Elf32_Word      e_version;
    Elf32_Addr      e_entry;
    Elf32_Off       e_phoff;
    Elf32_Off       e_shoff;
    Elf32_Word      e_flags;
    Elf32_Half      e_ehsize;
    Elf32_Half      e_phentsize;
    Elf32_Half      e_phnum;
    Elf32_Half      e_shentsize;
    Elf32_Half      e_shnum;
    Elf32_Half      e_shstrndx;
    
} Elf32_Ehdr;

// Elf32_Ehdr.e_type defines
#define ET_NONE     0       // No file type
#define ET_REL      1       // Relocatable file
#define ET_EXEC     2       // Executable file
#define ET_DYN      3       // Shared object file
#define ET_CORE     4       // Core file
#define ET_LOPROC   0xff00  // Processor-specific
#define ET_HIPROC   0xffff  // Processor-specific

// Elf32_Ehdr.e_machine defines
#define EM_NONE     0       // No machine
#define EM_M32      1       // AT&T WE 32100
#define EM_SPARC    2       // SPARC
#define EM_386      3       // Intel 80386
#define EM_68K      4       // Motorola 68000
#define EM_88K      5       // Motorola 88000
#define EM_860      7       // Intel 80860
#define EM_MIPS     8       // MIPS RS3000

#define EM_AVR      0x53

// Elf32_Ehdr.e_version defines
#define EV_NONE     0       // Invalid version
#define EV_CURRENT  1       // Current version

// Elf32_Ehdr.e_ident index defines
#define EI_MAG0     0       // File identification
#define EI_MAG1     1
#define EI_MAG2     2
#define EI_MAG3     3
#define EI_CLASS    4       // File class
#define EI_DATA     5       // Data encoding
#define EI_VERSION  6       // File version
#define EI_PAD      7       // Start of padding bytes

// Elf magic numbers
#define ELFMAG0     0x7f
#define ELFMAG1     'E'
#define ELFMAG2     'L'
#define ELFMAG3     'F'

// Class numbers
#define ELFCLASSNONE    0   // Invalid class
#define ELFCLASS32      1   // 32-bit objects
#define ELFCLASS64      2   // 64-bit objects

// Data encoding - for processor specific data
#define ELFDATANONE     0   // Invalid data encoding
#define ELFDATA2LSB     1   // Little endian
#define ELFDATA2MSB     2   // Big endian

/*
 *  ELF Program Header Structure
 */

typedef struct
{
     Elf32_Word      p_type;
     Elf32_Off       p_offset;
     Elf32_Addr      p_vaddr;
     Elf32_Addr      p_paddr;
     Elf32_Word      p_filesz;
     Elf32_Word      p_memsz;
     Elf32_Word      p_flags;
     Elf32_Word      p_align;
} Elf32_Phdr;

/* Definitions */

/* p_type */
#define PT_NULL     0             /* Unusued entry               */
#define PT_LOAD     1             /* Loadble segment             */
#define PT_DYNAMIC  2             /* Dynamic linking info        */
#define PT_INTERP   3             /* Path to interpreter         */
#define PT_NOTE     4             /* AUX info                    */
#define PT_SHLIB    5             /* Reserved                    */
#define PT_PHDR     6             /* Program header in exe image */
#define PT_LOPROC   0x70000000    /* Processor specific stuff    */
#define PT_HIPROC   0x7fffffff

/* p_flags */
#define PF_X        0x1           /* Execute */
#define PF_W        0x2           /* Write   */
#define PF_R        0x4           /* Read    */

/*

    ELF Section Header Structure
    
*/

// Special Section Indexes
#define SHN_UNDEF       0       // default null header
#define SHN_LORESERVE   0xff00  // lower bound of range of reserved indexes
#define SHN_LOPROC      0xff00  // Reserved for processor specific semantics
#define SHN_HIPROC      0xff1f
#define SHN_ABS         0xfff1  // symbols defined from this section are not relocated
#define SHN_COMMON      0xfff2  // common symbols - such as unallocated extern C variables
#define SHN_HIRESERVE   0xffff

typedef struct
{
    
    Elf32_Word      sh_name;
    Elf32_Word      sh_type;
    Elf32_Word      sh_flags;
    Elf32_Word      sh_addr;
    Elf32_Off       sh_offset;
    Elf32_Word      sh_size;
    Elf32_Word      sh_link;
    Elf32_Word      sh_info;
    Elf32_Word      sh_addralign;
    Elf32_Word      sh_entsize;
    
} Elf32_Shdr;

// Elf32_Shdr.sh_type defines
#define SHT_NULL        0
#define SHT_PROGBITS    1
#define SHT_SYMTAB      2
#define SHT_STRTAB      3
#define SHT_RELA        4
#define SHT_HASH        5
#define SHT_DYNAMIC     6
#define SHT_NOTE        7
#define SHT_NOBITS      8
#define SHT_REL         9
#define SHT_SHLIB       10
#define SHT_DYNSYM      11
#define SHT_LOPROC      0x70000000
#define SHT_HIPROC      0x7fffffff
#define SHT_LOUSER      0x80000000
#define SHT_HIUSER      0xffffffff

// Elf32_Shdr.sh_flags defines
#define SHF_WRITE       0x1
#define SHF_ALLOC       0x2
#define SHF_EXECINSTR   0x4
#define SHF_MASKPROC    0xf0000000

/*

    ELF Symbol Table Structure
    
*/

typedef struct
{
    
    Elf32_Word      st_name;
    Elf32_Addr      st_value;
    Elf32_Word      st_size;
    unsigned char   st_info;
    unsigned char   st_other;
    Elf32_Half      st_shndx;
    
} Elf32_Sym;

// Elf32_Sym.st_info defines
#define STB_LOCAL       0
#define STB_GLOBAL      1
#define STB_WEAK        2
#define STB_LOPROC      13
#define STB_HIPROC      15

#define STT_NOTYPE      0
#define STT_OBJECT      1
#define STT_FUNC        2
#define STT_SECTION     3
#define STT_FILE        4
#define STT_LOPROC      13
#define STT_HIPROC      15

/*

    ELF Recloation Entries
    
*/

typedef struct
{
    
    Elf32_Addr      r_offset;       // Byte offset from start of section for relocation storage element
    Elf32_Word      r_info;         // Symbol table index of the symbol to which the relocation is to be made to
    
} Elf32_Rel;

typedef struct
{
    
    Elf32_Addr      r_offset;
    Elf32_Word      r_info;
    Elf32_Word      r_addend;       // constant addend used to compute the value of the relocation
    
} Elf32_Rela;

// Special symbol/type macros, apply to Elf32_Rela.r_info
#define ELF32_R_SYM(i)      ((i)>>8)
#define ELF32_R_TYPE(i)     ((unsigned char)(i))
#define ELF32_R_INFO(s,t)   (((s)<<8)+(unsigned char)(t))




/* Target-specific ELF Info */

/* AVR */
/* E-Header Flag values */
#define EF_AVR_LINKRELAX_PREPARED 0x80

#define E_AVR_MACH_AVR1     1
#define E_AVR_MACH_AVR2     2
#define E_AVR_MACH_AVR25   25
#define E_AVR_MACH_AVR3     3
#define E_AVR_MACH_AVR31   31
#define E_AVR_MACH_AVR35   35
#define E_AVR_MACH_AVR4     4
#define E_AVR_MACH_AVR5     5
#define E_AVR_MACH_AVR51   51
#define E_AVR_MACH_AVR6     6 
#define E_AVR_MACH_AVRTINY 100
#define E_AVR_MACH_XMEGA1  101
#define E_AVR_MACH_XMEGA2  102
#define E_AVR_MACH_XMEGA3  103
#define E_AVR_MACH_XMEGA4  104
#define E_AVR_MACH_XMEGA5  105
#define E_AVR_MACH_XMEGA6  106
#define E_AVR_MACH_XMEGA7  107


/* Relocation Type Definitions */
#define R_AVR_NONE              0
#define R_AVR_32                1
#define R_AVR_7_PCREL           2
#define R_AVR_13_PCREL          3
#define R_AVR_16                4
#define R_AVR_16_PM             5
#define R_AVR_LO8_LDI           6
#define R_AVR_HI8_LDI           7
#define R_AVR_HH8_LDI           8
#define R_AVR_LO8_LDI_NEG       9
#define R_AVR_HI8_LDI_NEG       10
#define R_AVR_HH8_LDI_NEG       11
#define R_AVR_LO8_LDI_PM        12
#define R_AVR_HI8_LDI_PM        13
#define R_AVR_HH8_LDI_PM        14
#define R_AVR_LO8_LDI_PM_NEG    15
#define R_AVR_HI8_LDI_PM_NEG    16
#define R_AVR_HH8_LDI_PM_NEG    17
#define R_AVR_CALL              18
#define R_AVR_LDI               19
#define R_AVR_6                 20
#define R_AVR_6_ADIW            21
#define R_AVR_MS8_LDI           22
#define R_AVR_MS8_LDI_NEG       23
#define R_AVR_LO8_LDI_GS        24
#define R_AVR_HI8_LDI_GS        25
#define R_AVR_8                 26
#define R_AVR_8_LO8             27
#define R_AVR_8_HI8             28
#define R_AVR_8_HLO8            29
#define R_AVR_DIFF8             30
#define R_AVR_DIFF16            31
#define R_AVR_DIFF32            32
#define R_AVR_LDS_STS_16        33
#define R_AVR_PORT6             34
#define R_AVR_PORT5             35
#define R_AVR_32_PCREL          36

#define R_AVR_MAXTYPES          36

#endif
