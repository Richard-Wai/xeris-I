/* ATRI8                                     */
/* ATRIA Prototyping, Packing and Debug tool */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */

/* Appology:                                                */
/* This is a prototyping tool. The code is quick and nasty. */

const char * menu_info_header[] =
{
     "Magic",
     "Header size",
     "Class",
     "Encoding",
     "Ident Version",
     "Type",
     "Arch",
     "ELF Version",
     "Entry address",
     "Flags",
     "",
     "Program header",
     "  Offset",
     "  Entries",
     "",
     "Section header",
     "  Offset",
     "  Entires",
     "  Section string index"
};

const char * info_pheader[] =
{
     "Type",
     "Offset",
     "Virtual Address",
     "Physical Address",
     "File Size",
     "Memory Size",
     "Flags",
     "Alignment"
};

const char * info_pheader_type[] =
{
     "null",
     "Load",
     "Dynamic",
     "Interpreter",
     "Note",
     "SHLIB",
     "Program Header",
     "Lo/Hi Proc"
};

const char * info_sheader[] =
{
     "Name",
     "Type",
     "Flags",
     "Address",
     "Offset",
     "Section Size",
     "Link",
     "Info",
     "Align",
     "Entry Size"
};

const char * info_sheader_type[] =
{
     "null",
     "PROGBITS",
     "SYMTAB",
     "STRTAB",
     "RELA",
     "HASH",
     "DYNAMIC",
     "NOTE",
     "NOBITS",
     "REL",
     "SHLIB",
     "DYNSYM",
     "[PROC/USER]"
};

const char * info_symtab[] =
{
     "Name",
     "Value",
     "Size",
     "Binding",
     "Type",
     "(Other)",
     "Section",
     "(index)"
};


const char * info_reloc[] =
{
     "In section:",
     "Offset:",
     "Addend:",
     "Type:",
     "Index:",
     "",
     "[REFRENCED SYMBOL]",
     "Name",
     "Value",
     "Size",
     "Binding",
     "Type",
     "(Other)",
     "Section",
     "(index)"
};


const char * hex_cmd_list[] =
{
     "Selection Region Edit",
     "Centre at selection",
     "New Reference",
     "Execute Reference [ENTER]",
     "Cycle Reference [TAB]",
     "Goto address",
     "Goto index",
     "Quick entry 1",
     "Quick entry 2",
     "Quick entry 3",
     "Reference Operations [SPACE]",
     "Exit current context",
};

const char hex_cmd_keys[] =
{
     'e', 'h', 'n', 'x', 'o', 'g', 'i', '1', '2', '3', 'r', 'q'
};


const char * sel_name_eheader = "ELF Header";
const char * sel_name_pheader = "Program Header Entry";
const char * sel_name_sheader = "Section Header Entry";

const char * sel_type_list[] =
{
     "Default",
     "ELF Header",
     "Program Header Entry",
     "Section Header Entry",
     "SYMTAB",
     "STRTAB",
     "REL",
     "RELA",
     "Offset Pointer",
     "Index Pointer",
     "Address Pointer",
     "Machine Code"
};


const char * sel_refop_list[] =
{
     "Spawn",
     "Select Other",
     "Rename",
     "Delete",
     "Execute",
     "Exit"
};

const char sel_refop_keys[] =
{
     's', 'o', 'r', 'd', 'e', 'q'
};



/* AVR Relocation Types */
const char * reloc_avr[] =
{
     "AVR_NONE",
     "AVR_32",
     "AVR_7_PCREL",
     "AVR_13_PCREL",
     "AVR_16",
     "AVR_16_PM",
     "AVR_LO8_LDI",
     "AVR_HI8_LDI",
     "AVR_HH8_LDI",
     "AVR_LO8_LDI_NEG",
     "AVR_HI8_LDI_NEG",
     "AVR_HH8_LDI_NEG",
     "AVR_LO8_LDI_PM",
     "AVR_HI8_LDI_PM",
     "AVR_HH8_LDI_PM",
     "AVR_LO8_LDI_PM_NEG",
     "AVR_HI8_LDI_PM_NEG",
     "AVR_HH8_LDI_PM_NEG",
     "AVR_CALL",
     "AVR_LDI",
     "AVR_6",
     "AVR_6_ADIW",
     "AVR_MS8_LDI",
     "AVR_MS8_LDI_NEG",
     "AVR_LO8_LDI_GS",
     "AVR_HI8_LDI_GS",
     "AVR_8",
     "AVR_8_LO8",
     "AVR_8_HI8",
     "AVR_8_HLO8",
     "AVR_DIFF8",
     "AVR_DIFF16",
     "AVR_DIFF32",
     "AVR_LDS_STS_16",
     "AVR_PORT6",
     "AVR_PORT5",
     "AVR_32_PCREL"
};

