/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CTRDG - libraries - ARM9
  File:     ctrdg_flash_common.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/

#include <nitro.h>


// extern data declaration----------------------
extern u16 CTRDGi_PollingSR1MCOMMON(u16 phase, u8 *adr, u16 lastData);

// define data----------------------------------

// function's prototype declaration-------------
extern u16 CTRDGi_EraseFlashChipMX(void);
extern u16 CTRDGi_EraseFlashSectorMX(u16 secNo);
extern u16 CTRDGi_WriteFlashSectorMX(u16 secNo, u8 *src);
extern void CTRDGi_EraseFlashChipAsyncMX(CTRDG_TASK_FUNC callback);
extern void CTRDGi_EraseFlashSectorAsyncMX(u16 secNo, CTRDG_TASK_FUNC callback);
extern void CTRDGi_WriteFlashSectorAsyncMX(u16 secNo, u8 *src, CTRDG_TASK_FUNC callback);

// const data-----------------------------------
static const u16 LeMaxTime[] = {
    10,                                // common       10ms
    10,                                // program      10ms(exactly:20usec)
    2000,                              // sector erase 2s
    5000,                              // chip   erase 5s
};

const CTRDGiFlashTypePlus LE26FV10N1TS_10 = {
    CTRDGi_WriteFlashSectorMX,
    CTRDGi_EraseFlashChipMX,
    CTRDGi_EraseFlashSectorMX,
    CTRDGi_WriteFlashSectorAsyncMX,
    CTRDGi_EraseFlashChipAsyncMX,
    CTRDGi_EraseFlashSectorAsyncMX,
    CTRDGi_PollingSR1MCOMMON,
    LeMaxTime,
    {
/* デバッグ用 */
//#ifndef   __FLASH_DEBUG
        0x00020000,                       // ROM size
        {0x00001000, 12, 32, 0},          // sector size,shift,count,top
//#else
//      0x00004000,
//      {0x00000200,  9, 32,  0},
//#endif
        {MI_CTRDG_RAMCYCLE_18, MI_CTRDG_RAMCYCLE_8},       // agb read cycle=8, write cycle=3
        0x62,                             // maker ID
        0x13,                             // device ID
    },
};
