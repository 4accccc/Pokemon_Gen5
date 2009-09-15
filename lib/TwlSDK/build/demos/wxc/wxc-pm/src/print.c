/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - wxc-pm
  File:     print.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/
 
#include    "print.h"
 
extern u16 gScreen[32 * 32];           // 仮想スクリーン
 
/*---------------------------------------------------------------------------*
  Name:         ClearStringY

  Description:  仮想スクリーンをクリアする。(一行）

  Arguments:    y - 消去する行

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ClearStringY(s16 y)
{
    MI_CpuClearFast((void *)(&gScreen[(y * 32)]), sizeof(u16)*32);
}


/*---------------------------------------------------------------------------*
  Name:         ClearString

  Description:  仮想スクリーンをクリアする。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ClearString(void)
{
    MI_CpuClearFast((void *)gScreen, sizeof(gScreen));
}


/*---------------------------------------------------------------------------*
  Name:         PrintString

  Description:  仮想スクリーンに文字列を配置する。文字列は32文字まで。

  Arguments:    x       - 文字列の先頭を配置する x 座標( × 8 ドット )。
                y       - 文字列の先頭を配置する y 座標( × 8 ドット )。
                palette - 文字の色をパレット番号で指定。
                text    - 配置する文字列。終端文字はNULL。
                ...     - 仮想引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PrintString(s16 x, s16 y, u8 palette, char *text, ...)
{
    va_list vlist;
    char temp[32 + 2];
    s32 i;

    va_start(vlist, text);
    (void)OS_VSNPrintf(temp, 33, text, vlist);
    va_end(vlist);

    *(u16 *)(&temp[32]) = 0x0000;
    for (i = 0;; i++)
    {
        if (temp[i] == 0x00)
        {
            break;
        }
        gScreen[((y * 32) + x + i) % (32 * 32)] = (u16)((palette << 12) | temp[i]);
    }
}
