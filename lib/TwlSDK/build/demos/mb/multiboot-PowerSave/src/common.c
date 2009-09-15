/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-PowerSave
  File:     common.c

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/


#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif
#include "common.h"


/*****************************************************************************/
/* variable */

BOOL    g_power_save_mode;


/*****************************************************************************/
/* function */

/*---------------------------------------------------------------------------*
  Name:         ReadKeySetTrigger

  Description:  キートリガ検出.

  Arguments:    None.

  Returns:      前回呼び出し時に押下状態でなく, かつ
                今回呼び出し時に押下状態であったキー入力のビット集合.
 *---------------------------------------------------------------------------*/
u16 ReadKeySetTrigger(void)
{
    /* IPL 内での A ボタン押下対策 */
    static u16 cont = 0xffff;
    u16     keyset = PAD_Read();
    u16     trigger = (u16)(keyset & (keyset ^ cont));
    cont = keyset;
    return trigger;
}
