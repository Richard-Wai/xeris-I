/* ATRI8                                     */
/* ATRIA Prototyping, Packing and Debug tool */
/*                                           */
/* Copyright (C) 2017, Richard Wai           */
/* All Rights Reserved                       */


/* ATRIA simulation subsystem */

#ifndef SIM_H
#define SIM_H

#include "atria.h"



#define INT_CNT 26

#define IMG_SIZE ((sizeof (struct atria_header)) + (sizeof (uint16_t) * EXE_MAX + 1))

struct exe_partition
{
     uint32_t   off;            /* Global offset    */
     uint32_t   ext;            /* Extent           */
     uint16_t * cursor;         /* Current location */
};

struct op_partition
{
     uint32_t off;              /* Global offset    */
     uint32_t ext;              /* Extent           */
};
     

extern void sim_base ( void );

#endif
