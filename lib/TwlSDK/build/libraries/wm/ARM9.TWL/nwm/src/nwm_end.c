/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NWM - libraries
  File:     nwm_end.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include "nwm_common_private.h"
#include "nwm_arm9_private.h"


/*---------------------------------------------------------------------------*
  Name:         NWM_End

  Description:  NWMライブラリの終了処理を行う。
                ARM9側の終了のみを行う同期関数。

  Arguments:    None

  Returns:      NWMRetCode   -  処理結果を返す。
 *---------------------------------------------------------------------------*/

NWMRetCode NWM_End(void)
{
    NWMRetCode result;
    OSIntrMode e;
    extern NWMArm9Buf *nwm9buf;
    extern u8 nwmInitialized;

    SDK_NULL_ASSERT(nwm9buf);
    
    // INITIALIZEDステート以外では実行不可
    result = NWMi_CheckState(1, NWM_STATE_INITIALIZED);
    NWM_CHECK_RESULT(result);

    e = OS_DisableInterrupts();
    
    nwm9buf->status->state = NWM_STATE_NONE;  // 例外的にここでだけ初期Stateをセット(本来はARM7の仕事)

    // FIFOバッファ書き込み許可フラグをクリア
    NWMi_ClearFifoRecvFlag();
    
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_WMW, NULL);
    nwm9buf = 0;
    
    // 正常終了
    nwmInitialized = 0;
    (void)OS_RestoreInterrupts(e);
    return NWM_RETCODE_SUCCESS;
}

