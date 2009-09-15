/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - include -
  File:     scheduler.h

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef	NITRO_WXC_SCHEDULER_H_
#define	NITRO_WXC_SCHEDULER_H_


#include <nitro.h>


/*****************************************************************************/
/* constant */

/* 親子変動パターン */
#define WXC_SCHEDULER_PATTERN_MAX   4
#define WXC_SCHEDULER_SEQ_MAX       4


/*****************************************************************************/
/* declaration */

/* 親機モード,子機モードの切り替えを管理する構造体 */
typedef struct WXCScheduler
{
    /* 現在の切り替えシーケンス */
    int     seq;
    int     pattern;
    int     start;
    BOOL    child_mode;
    /* 切り替えテーブル (TRUE は親機, FALSE は子機) */
    BOOL    table[WXC_SCHEDULER_PATTERN_MAX][WXC_SCHEDULER_SEQ_MAX];
}
WXCScheduler;


/*****************************************************************************/
/* function */

#ifdef __cplusplus
extern "C"
{
#endif


/*---------------------------------------------------------------------------*
  Name:         WXCi_InitScheduler

  Description:  モード切り替えスケジューラを初期化.

  Arguments:    p             WXCScheduler 構造体

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXCi_InitScheduler(WXCScheduler * p);

/*---------------------------------------------------------------------------*
  Name:         WXCi_SetChildMode

  Description:  スケジューラを子機側固定で動作するよう設定.

  Arguments:    p             WXCScheduler 構造体
                enable        子機側でしか起動させない場合は TRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXCi_SetChildMode(WXCScheduler * p, BOOL enable);

/*---------------------------------------------------------------------------*
  Name:         WXCi_UpdateScheduler

  Description:  モード切り替えスケジューラを更新.

  Arguments:    p             WXCScheduler 構造体

  Returns:      現在親機モードなら TRUE を, 子機モードなら FALSE を返す.
 *---------------------------------------------------------------------------*/
BOOL    WXCi_UpdateScheduler(WXCScheduler * p);


#ifdef __cplusplus
}
#endif


#endif /* NITRO_WXC_SCHEDULER_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
