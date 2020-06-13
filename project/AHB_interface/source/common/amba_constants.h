/*
------------------------------------------------------------------------
--
-- File :                       amba_constants.v
-- Author:                      Tao Li
-- Date :                       $Date: 2014/2/25 14:24:45 $
-- Version      :               $Revision: 1.0 $
-- Abstract     :               Global DW_ocb definitions.
--                              This file contains definitions of all
--                              the AMBA Rev 2 control definitions,
--                              and some general purpose definitions.
--
-- Modification History:
-- Date                 By      Version     Change Description
------------------------------------------------------------------------
*/

#ifndef AMBA_CONSTANTS_H_INCLUDED
#define AMBA_CONSTANTS_H_INCLUDED

// AMBA Bus constant definitions


// Description:  Burst Bus Width
// DefaultValue: 3
// MinValue:     -2147483648
// MaxValue:     2147483647
#define HBURST_WIDTH 3

#define HMASTER_WIDTH 4
#define HPROT_WIDTH 4
#define HRESP_WIDTH 2
#define HSIZE_WIDTH 3
#define HSPLIT_WIDTH 16
#define HTRANS_WIDTH 2

/* HBURST values */
#define SINGLE       0
#define INCR         1
#define WRAP4        2
#define INCR4        3
#define WRAP8        4
#define INCR8        5
#define WRAP16       6
#define INCR16       7

/* HRESP values */
#define OKAY  0
#define EROR  1
#define RETRY 2
#define SPLIT 3

/* HSIZE values */
#define BYTE         0     // 8    bits
#define HWORD        1     // 16   bits
#define WORD         2     // 32   bits
#define LWORD        3     // 64   bits
#define DWORD        4     // 128  bits
#define WORD4        5     // 256  bits
#define WORD8        6     // 512  bits
#define WORD16       7     // 1024 bits

/* HTRANS values */
#define IDLE       0
#define BUSY       1
#define NONSEQ     2
#define SEQ        3

/* HWRITE/PWRITE values */
#define READ       0
#define WRITE      1

/* other useful definitions */
/* generic definitions */
#define TRUE       1
#define FALSE      0

#define zero8      0x00
#define zero16     0x0000
#define zero32     0x00000000

/* Define 1KBYTE block size */
#define KBYTE 1024
#define BEAT_SIZE 10
#define BUFFER_SIZE 1024
#define DEFAULT_ADDR 0x0;
#define BUSWIDTH 32
#define DEFAULT_DATA 0x0;

#endif // AMBA_CONSTANTS_H_INCLUDED
