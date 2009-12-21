/*
  Project:  NitroSDK - wireless_shared - demos - wh
  File:     wh_config.h

  Copyright 2003-2005 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.


  Revision 1.2  2005/02/28 05:26:35  yosizaki
  do-indent.

  Revision 1.1  2004/12/20 07:17:48  takano_makoto
  アプリケーション毎の無線通信パラメータをwh_config.hとして分離

 */

#pragma once

#include "../net_def.h"

// 無線で使用するDMA番号
//#define WH_DMA_NO                 GFL_DMA_NET_NO

// 通常の MP 通信で使用するポート
//#define WH_DATA_PORT              (4)  //14

// 通常の MP 通信で使用する優先度
#define WH_DATA_PRIO              WM_PRIORITY_NORMAL

// データシェアリングで使用するポート
//#define WH_DS_PORT                13


//#define WH_MP_4CHILD_DATA_SIZE         (GFL_NET_CHILD_DATA_SIZE*2)


