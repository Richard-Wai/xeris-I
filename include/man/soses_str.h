/*
 *  XERIS/APEX System I
 *  Autonomous Poly Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  ALL RIGHTS RESERVED.
 */

/*
 *  Standard Operational Status and Errors Specification
 *  Errorcodes as Strings
 *  std/soses.h
 *
 *  Based on <std/soses.h>
 */

#ifndef __XERIS_MAN_SOSES_STR_H
#define __XERIS_MAN_SOSES_STR_H

/* Specifications */

/**************************************************************************
 *         SPEC 0000 - Universal Operational Status Reporting             *
 **************************************************************************/

/* OP-S - Information and Status Group */
#define MAN_CNVT_XSR_OP-S_LEN 6           /* Number of strings */
#define MAN_CNVT_XSR_OP-S \
{ \
"OP-S-INCP_NOTICE",      /* 000 */ \
"OP-S-OPR_OK",           /* 001 */ \
"OP-S-REQUEST_CPL",      /* 002 */ \
"OP-S-READ_ONLY",        /* 003 */ \
"OP-S-WRITE_ONLY",       /* 004 */ \
"OP-S-REQUEST_UKN",      /* 005 */ \
"OP-S-NOMINAN_ENTRY"     /* 006 */ \
} \

/* OP-I - Initilization Messages */
#define MAN_CNVT_XSR_OP-I_LEN 11
#define MAN_CNVT_XSR_OP-I \
{ \
"OP-I-INIT_NORMAL",      /* 000 */ \
"OP-I-INIT_LEVEL",       /* 001 */ \
"OP-I-INIT-ABNORMAL",    /* 002 */ \
"OP-I-INIT-FAILSF_LVL",  /* 003 */ \
"OP-I-INIT-EMERG",       /* 004 */ \
"OP-I-INIT-RECOVER",     /* 005 */ \
"OP-I-INIT-DEFERED",     /* 006 */ \
"OP-I-INIT-EMERG_OK",    /* 007 */ \
"OP-I-INIT-ABNORMAL_OK", /* 008 */ \
"OP-I-INIT-OK",          /* 009 */ \
"OP-I-INIT-PARAM"        /* 00A */ \
} \

/* OP-E - Non-Fatal Error Messages */
#define MAN_CNVT_XSR_OP-E_LEN 2
#define MAN_CNVT_XSR_OP-E \
{ \
"",                      /* 000 */ \
"OP-E-OPR_LIMIT"         /* 001 */ \
} \

/* OP-F - Fatal Error Messages */
#define MAN_CNVT_XSR_OP-F_LEN 8
#define MAN_CNVT_XSR_OP-F \
{ \
"OP-F-INIT_FATAL",       /* 000 */ \
"OP-F-RECOVER_FAIL",     /* 001 */ \
"OP-F-SVC_UNAVAIL",      /* 002 */ \
"OP-F-NOT_PERMITTED",    /* 003 */ \
"OP-F-ACCESS_DENIED",    /* 004 */ \
"OP-F-WRITE_UNABLE",     /* 005 */ \
"OP-F-READ_UNABLE",      /* 006 */ \
"OP-F-INTRFC_UNRSPNSV"   /* 007 */ \
} \
 
#endif
