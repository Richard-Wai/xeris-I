/* ATRI8                                   */
/* ATRIA Simulator, Packing and Debug tool */
/*                                         */
/* Copyright (C) 2017, Richard Wai         */
/* All Rights Reserved                     */

#ifndef ATRIA_H
#define ATRIA_H

#include <stdint.h>

/* atmega328p */
#define EXE_MIN 0x00000000
#define EXE_MAX 0x00003fff
#define OP_MIN  0x00000100
#define OP_MAX  0x000008ff

#define INTR_MAX 25

/* ISR Trap code */
#define ISR_TRAP_1 0x2788     /* clr r24 */
#define ISR_TRAP_2 0x9508     /* ret */

/* Board size (bytes) */
#define BOARD_SIZE 256

/* Tag types */
#define TAG_PRT 0x00f0
#define TAG_ADR 0x00f1
#define TAG_UNI 0x00f2
#define TAG_COM 0x00f3
#define TAG_DAT 0x00f4
#define TAG_MAN 0x00f5
#define TAG_XFR 0x00f6

#define XST_MSG 0x0000


/* Partition Tag and Defintions */

typedef struct
{
     uint16_t ident;

     uint16_t word;             /* Data Tag    */
     uint16_t align;            /* Data Tag    */
     uint16_t ovrly;            /* Unit Tag    */
     uint16_t extnt;            /* Address Tag */
} tag_prt;

/* Reserved idents */
#define PART_NUL 0x0000         /* Null Partition              */
#define PART_EXE 0x0001         /* Executive Partition         */
#define PART_OP1 0x0002         /* First Operations Partition  */
#define PART_OP2 0x0003         /* Second Operations Partition */
#define PART_REF 0x0004         /* Static Reference Partition  */
#define PART_SYS 0x0005         /* System/Control Partition    */
#define PART_XFL 0x0006         /* Trans-FLATLINE Partition    */
#define PART_MAN 0x0007         /* Human Reference Partition   */



/* Address tag */
typedef struct
{
     uint16_t ident;

     uint16_t part;             /* Parition Tag */
     uint16_t segt;             /* Data Tag     */
     uint16_t offs;             /* Data Tag     */
     uint16_t unit;             /* Unit Tag     */
} tag_adr;


/* Unit Tag */
typedef struct
{
     uint16_t ident;

     uint16_t top;              /* Address Tag         */
     uint16_t bot;              /* Address Tag         */
     uint16_t com;              /* Commission Tag      */
     uint16_t man;              /* Human Reference Tag */
} tag_uni;


/* Commission Tag */
typedef struct
{
     uint16_t ident;

     uint16_t type;
     uint16_t endp;             /* Adddress Tag */
     uint16_t ref;              /* Unit Tag     */
     uint16_t annx;             /* Data Tag     */
} tag_com;

/* Generic Commission types */
#define GEN_DDR 0x0000
#define GEN_TXA 0x0001
#define GEN_MAN 0x0002
#define GEN_ISR 0x0003
#define GEN_SEC 0x0004
#define GEN_PRI 0x0005

#define GEN_MOD 0x000f

#define GEN_LDR 0x0010
#define GEN_STR 0x0011
#define GEN_LDI 0x0012
#define GEN_ROI 0x0013
#define GEN_STI 0x0014
#define GEN_LDM 0x0015
#define GEN_STM 0x0016
#define GEN_TAM 0x0017

#define GEN_SYS 0x1000
#define GEN_JMP 0x1001
#define GEN_REL 0x1002
#define GEN_IND 0x1003


/* AVR Commission types */
#define AVR_ILO 0xa000          /* Immediate lo8 (AVR_LO8_LDI) */
#define AVR_IHI 0xa001          /* Immediate hi8 (AVR_HI8_LDI) */
#define AVR_INL 0xa002          /* Immediate negative lo8 */
#define AVR_INH 0xa003          /* Immediate negative hi8 */
#define AVR_IWL 0xa004          /* Immediate word address lo8 */
#define AVR_IWH 0xa005          /* Immediate word address hi8 */

#define AVR_ML8 0xa006          /* Mod low 8  */
#define AVR_MH8 0xa007          /* Mod high 8 */

/* XERIS Commission type */
#define XERIS_FLATLINE 0xf000
#define XERIS_ATRIA    0xf001

#define XERIS_LINK_SRF  0xf002
#define XERIS_LINK_SISR 0xf003


/* Associated AVR opcodes */
#define AVR_OPC_ANDI 0b0111
#define AVR_OPC_CPI  0b0011
#define AVR_OPC_LDI  0b1110
#define AVR_OPC_ORI  0b0110
#define AVR_OPC_SBCI 0b0100
#define AVR_OPC_SUBI 0b0101

#define AVR_OPC_JMP  0b110
#define AVR_OPC_CALL 0b111




/* Data Tag */
typedef struct
{
     uint16_t ident;

     uint64_t data;
} tag_dat;


/* Human Reference Tag */
typedef struct
{
     uint16_t ident;

     uint32_t first;
     uint32_t last;
} tag_man;


/* Transfer Slip */
typedef struct
{
     uint16_t unit;

     uint32_t seq;
     uint32_t bsz;
} tag_xfr;

/* XERIS standard message ticket */
typedef struct
{
     uint16_t ref;

     union
     {
          uint8_t  a08[8];
          uint16_t a16[4];
          uint32_t a32[2];
          uint64_t a64;
     } arg;
} xst_msg;

/* Message codes (ref) */
#define NEW_REG      0x00ff


/* Ticket prototype */
typedef struct
{
     uint16_t s;

     union
     {
          xst_msg msg;
          tag_prt prt;
          tag_adr adr;
          tag_uni uni;
          tag_com com;
          tag_dat dat;
          tag_man man;
          tag_xfr xfr;
     } t;
} tag;


/* Atria registration files */
struct atria_soc
{
     uint16_t  sid;
     uint16_t  dd;
     uint16_t  bpri;
     uint16_t  dsec;
     uint16_t  usec;

     uint16_t  account;
};

struct atria_sisr
{
     uint8_t   flags;
     uint16_t  isr;
     uint16_t  office;
} __attribute__((packed));

struct atria_srf
{
     struct
     {
          uint8_t top[3];
          uint8_t bot[3];
     } __attribute__((packed)) text;

     struct
     {
          uint8_t  ofs[3];
          uint16_t lda;
          uint16_t siz;
     } __attribute__((packed)) data;

     struct
     {
          uint16_t lda;
          uint16_t siz;
     } __attribute__((packed)) bss;

     struct atria_soc   soc;
} __attribute__((packed));


struct block_index
{
     uint16_t com_flag;
     uint16_t index[16];
};
          

/* Simulation file header */
#define AMAGIC "ATRIA"

/* Header offset: 0x2E */

struct atria_header
{
     unsigned char    magic[16];

     uint32_t         srf_alpha;
     uint32_t         sisr_top;

     uint32_t         last_sid;
     uint32_t         img_ext;
     uint32_t         op_ext;

     uint32_t         sys_opsec_flatline;
     uint32_t         sys_opsec_atria;
};
     


#endif
