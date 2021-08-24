/*
 *  XERIS/APEX System I
 *  Autonomous Poly-Executive
 *
 *  Release 1
 *
 *  Copyright (C) 2016, Richard Wai
 *  ALL RIGHTS RESERVED.
 */

/*
 *  Standard Operational Status and Errors Specification
 *  (xeris?soses*)
 *  std/soses.h
 */

#ifndef __XERIS_STD_SOSES_H
#define __XERIS_STD_SOSES_H

/* Specifications */

/**************************************************************************
 *         SPEC 0000 - Universal Operational Status Reporting             *
 **************************************************************************/

#define XSR_OP    0x0000        /* Universal Simple Status Reporting */



#define XSR_OP_S  0x0000
#define XSR_OP_I  0x1000
#define XSR_OP_E  0x3000
#define XSR_OP_F  0x5000

/* OP-S - Information and Status Group */
#define XSR_OP_S_INCP_NOTICE      0x0000  /* Inception Notice       */
#define XSR_OP_S_OPR_OK           0x0001  /* Operation Successful   */
#define XSR_OP_S_REQUEST_CPL      0x0002  /* Request Complete       */
#define XSR_OP_S_READ_ONLY        0x0003  /* Read_Only              */
#define XSR_OP_S_WRITE_ONLY       0x0004  /* Write_Only             */
#define XSR_OP_S_REQUEST_UKN      0x0005  /* Unknown Request        */
#define XSR_OP_S_NOMINAL_ENTRY    0x0006  /* Nominal Entry          */
#define XSR_OP_S_AUTH_OK          0x0007  /* Authorization OK       */
#define XSR_OP_S_TEST_PASS        0x0008  /* Test Passed            */
#define XSR_OP_S_ACCEPTED         0x0009  /* Submission acccepted   */
#define XSR_OP_S_APPROVED         0x000a  /* Application Approved   */

/* OP-I - Initilization Messages */
#define XSR_OP_I_INIT_NORMAL      0x1000  /* Init Normal            */
#define XSR_OP_I_INIT_LEVEL       0x1001  /* Init Level             */
#define XSR_OP_I_INIT_ABNORMAL    0x1002  /* Init Abnormal          */
#define XSR_OP_I_INIT_FAILSF_LVL  0x1003  /* Init Failsafe Level    */
#define XSR_OP_I_INIT_EMERG       0x1004  /* Init Emergency         */
#define XSR_OP_I_INIT_RECOVER     0x1005  /* Init Recovery          */
#define XSR_OP_I_INIT_DEFERED     0x1006  /* Init Defered           */
#define XSR_OP_I_INIT_EMERG_OK    0x1007  /* Emergency Init OK      */
#define XSR_OP_I_INIT_ABNORMAL_OK 0x1008  /* Abnormal Init OK       */
#define XSR_OP_I_INIT_OK          0x1009  /* Init OK                */
#define XSR_OP_I_INIT_PARAM       0x100a  /* Init Parameters        */
#define XSR_OP_I_INIT_AUTO        0x100b  /* Init Autonomous        */
#define XSR_OP_I_INIT_INTV        0x100c  /* Init Intervention      */
#define XSR_OP_I_INIT_LATE        0x100d  /* Late Init              */
#define XSR_OP_I_INIT_CONFIG      0x100e  /* Init configuration     */
#define XSR_OP_I_INIT_FAIL        0x100f  /* Init failed            */
#define XSR_OP_I_INIT_REQUIRED    0x1010  /* Init required          */
#define XSR_OP_I_INIT_ILLEGAL     0x1011  /* Init not allowed       */

/* OP-E - Non_Fatal Error Messages */
#define XSR_OP_E_OPR_LIMIT        0x3001  /* Operation Limited      */

/* OP-F - Fatal Error Messages */
#define XSR_OP_F_INIT_FATAL       0x5000  /* Initialization fatal   */
#define XSR_OP_F_RECOVER_FAIL     0x5001  /* Recovery failed        */
#define XSR_OP_F_SVC_UNAVAIL      0x5002  /* Service unavilable     */
#define XSR_OP_F_NOT_PERMITTED    0x5003  /* Not permitted          */
#define XSR_OP_F_ACCESS_DENIED    0x5004  /* Access denied          */
#define XSR_OP_F_WRITE_UNABLE     0x5005  /* Unable to write        */
#define XSR_OP_F_READ_UNABLE      0x5006  /* Unable to read         */
#define XSR_OP_F_INTRFC_UNRSPNSV  0x5007  /* Interface Unresponsive */
#define XSR_OP_F_UNACPT_ORDER     0x5008  /* Unacceptable Work Order*/
#define XSR_OP_F_NO_OPERATOR      0x5009  /* No such operator       */
#define XSR_OP_F_OPR_UNAVAIL      0x500a  /* Operator unavailable   */
#define XSR_OP_F_NO_AUTH          0x500b  /* Authorization Failed   */
#define XSR_OP_F_TEST_FAIL        0x500c  /* Test Failed            */
#define XSR_OP_F_PARAM_EXCUR      0x500d  /* Parametr out of bounds */
#define XSR_OP_F_DATA_CORRUPT     0x500e  /* Data Corruption        */
#define XSR_OP_F_EC_UNABLE        0x500f  /* Error Correction Fail  */
#define XSR_OP_F_VERIFY_FAIL      0x5010  /* Verification Failed    */
#define XSR_OP_F_OPTION_UNKN      0x5011  /* Option not recognised  */
#define XSR_OP_F_CB_CONFIG        0x5012  /* bad case board config  */
#define XSR_OP_F_INSUF_RESRC      0x5013  /* Insufficient Resources */
#define XSR_OP_F_NO_SECRETARIAT   0x5014  /* Secretariat not found  */
#define XSR_OP_F_OVERFLOW         0x5015  /* Overflow error         */
#define XSR_OP_F_SIGNAL_ERROR     0x5016  /* Signal non-sensical    */
#define XSR_OP_F_PROC_VIOL        0x5017  /* Procedure violation    */
#define XSR_OP_F_UNDEFINED        0x5018  /* Error not programmed   */
#define XSR_OP_F_NO_RECORD        0x5019  /* No record registered   */



/**************************************************************************
 *         SPEC 0001 - XERIS/APEX Core Error and Intervention             *
 **************************************************************************/

#define XSR_SYS    0x0001        /* SPEC Code */

#define XSR_SYS_S  0x0000
#define XSR_SYS_I  0x1000
#define XSR_SYS_E  0x3000
#define XSR_SYS_F  0x5000
#define XSR_SYS_P  0x7000       /* Illegal Action */

/* SYS-I - Initialization Messages */
#define XST_SYS_I_INIT_STRAP      0x1000  /* Init bootstrap    */
#define XST_SYS_I_INIT_CONFIG     0x1001  /* Init config block */

/* SYS-W - Warning Messages */
#define XSR_SYS_W_ZONE_THRESHOLD  0x2000  /* Stack at zone threshold    */
#define XSR_SYS_W_CONFIG_SIZE     0x2001  /* Bad config block size      */
#define XSR_SYS_W_NO_CONFIG       0x2002  /* Config block not available */

/* SYS-d - Denial Messages */
#define XSR_SYS_D_NO_TELCOM       0x3000  /* No telemetry commissioner */

/* SYS-C - Critical Error - Intervention Required */
#define XSR_SYS_C_LOCAL_EXCURSION 0x4000  /* Stack breached work area */

/* SYS-F - Fatal Error */
#define XSR_SYS_F_ZONE_EXCURSION  0x5000  /* Stack breaked zone */
#define XSR_SYS_F_EMRG_LOCKDOWN   0x5001  /* Emergency Lockdown */

/* SYS-P - Illegal Action Messages */
#define XSR_SYS_P_CERT_INVALID    0x7000  /* Certificate Invalid            */
#define XSR_SYS_P_TX_NO_ACCNT     0x7001  /* No transaction account defined */
#define XSR_SYS_P_TX_EVENT_CON    0x7002  /* Event containment              */
#define XSR_SYS_P_SYS_STUB        0x7003  /* SYS stub illegal               */
#define XSR_SYS_P_TICKET          0x7004  /* Ticket as submitted is illegal */


/**************************************************************************
 *                Specific Telemetry Encoding Groups                      *
 **************************************************************************/

#define XSR_TLM_OPSEC      0x0010
#define XSR_TLM_FLATLINE   0x0011
#define XSR_TLM_ATRIA      0x0012
#define XSR_TLM_SIPLEX     0x0013


#endif
