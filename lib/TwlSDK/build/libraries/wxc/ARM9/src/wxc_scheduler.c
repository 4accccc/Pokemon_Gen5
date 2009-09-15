/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - libraries -
  File:     wxc_scheduler.c

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-16#$
  $Rev: 9661 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

#include <nitro/wxc/common.h>
#include <nitro/wxc/scheduler.h>


/*****************************************************************************/
/* function */

/*---------------------------------------------------------------------------*
  Name:         WXCi_InitScheduler

  Description:  モード切り替えスケジューラを初期化.

  Arguments:    p             WXCScheduler 構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXCi_InitScheduler(WXCScheduler * p)
{
    /*
     * 経験的に適切と思われるデフォルトのスケジュールで初期化.
     * デバッグ用途以外では基本的にこの設定を変更しない.
     */
    static const BOOL default_table[WXC_SCHEDULER_PATTERN_MAX][WXC_SCHEDULER_SEQ_MAX] = {
        {TRUE, FALSE, TRUE, TRUE},
        {FALSE, TRUE, TRUE, TRUE},
        {FALSE, TRUE, TRUE, TRUE},
        {TRUE, FALSE, TRUE, TRUE},
    };
    p->seq = (int)((OS_GetTick() >> 0) % WXC_SCHEDULER_SEQ_MAX);
    p->pattern = (int)((OS_GetTick() >> 2) % WXC_SCHEDULER_PATTERN_MAX);
    p->start = 0;
    p->child_mode = FALSE;
    MI_CpuCopy32(default_table, p->table, sizeof(default_table));
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_SetChildMode

  Description:  スケジューラを子機側固定で動作するよう設定.

  Arguments:    p             WXCScheduler 構造体
                enable        子機側でしか起動させない場合は TRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXCi_SetChildMode(WXCScheduler * p, BOOL enable)
{
    p->child_mode = enable;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_UpdateScheduler

  Description:  モード切り替えスケジューラを更新.

  Arguments:    p             WXCScheduler 構造体

  Returns:      現在親機モードなら TRUE を, 子機モードなら FALSE を返す.
 *---------------------------------------------------------------------------*/
BOOL WXCi_UpdateScheduler(WXCScheduler * p)
{
    if (++p->seq >= WXC_SCHEDULER_SEQ_MAX)
    {
        p->seq = 0;
        if (++p->pattern >= WXC_SCHEDULER_PATTERN_MAX)
        {
            p->pattern = 0;
        }
        if (p->pattern == p->start)
        {
            /* 乱数のつもり */
            p->start = (int)(OS_GetTick() % WXC_SCHEDULER_PATTERN_MAX);
            p->pattern = p->start;
        }
    }
    return p->table[p->pattern][p->seq] && !p->child_mode;
}
