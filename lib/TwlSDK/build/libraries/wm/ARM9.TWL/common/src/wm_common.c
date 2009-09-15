/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_common.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include "wm_common.h"

/*---------------------------------------------------------------------------*
  Name:         WM_CheckInitialized

  Description:  WM が使用されているかをチェックする。

  Arguments:    None.

  Returns:      int -   WM_ERRCODE_*型の処理結果を返す。
 *---------------------------------------------------------------------------*/
WMErrCode WM_CheckInitialized(void)
{
		return WMi_CheckInitialized();
}

/*---------------------------------------------------------------------------*
  Name:         NWM_CheckInitialized

  Description:  New WM が使用されているかをチェックする。

  Arguments:    None.

  Returns:      int -   NWM_ERRCODE_*型の処理結果を返す。。
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_CheckInitialized(void)
{
#ifdef SDK_TWL
	if(OS_IsRunOnTwl()) //TWL 上で動作している場合のみ、旧無線ライブラリの状態を確認
	{
		return NWMi_CheckInitialized();
	}
#endif
    //NITRO 上では、新無線チップが存在しないので、一意に NWM_RETCODE_SUCCESS を返す。
	return NWM_RETCODE_ILLEGAL_STATE;
}