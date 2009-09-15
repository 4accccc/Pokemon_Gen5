/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - simple-2
  File:     common.c

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


#include <nitro.h>


#include "common.h"

/* function */

/* キートリガ検出 */
u16 ReadKeySetTrigger(u16 keyset)
{
    static u16 cont = 0xffff;          /* IPL 内での A ボタン押下対策 */
    u16 trigger = (u16)(keyset & (keyset ^ cont));
    cont = keyset;
    return trigger;
}

