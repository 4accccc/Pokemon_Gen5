/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-wfs - parent
  File:     main.c

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-08#$
  $Rev: 9555 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
    WBT を駆動して無線マルチブート時の子機がファイルシステムを
    利用するためのサンプルプログラムです。

    HOWTO:
        1. 親機として通信を開始するためにＡボタンを押して下さい。
           周辺に同じ multiboot-wfs デモの子機が見つかり次第、自動的にその子機と
           通信を開始します。同時に通信できる子機は15台までです。
        2. 接続中のさまざまな機能については、親機 / 子機の画面表示と
           ソースコード内の注釈を参照ください。
 *---------------------------------------------------------------------------*/


#ifdef SDK_TWL
#include    <twl.h>
#else
#include    <nitro.h>
#endif
#include <nitro/wm.h>
#include <nitro/wbt.h>
#include <nitro/fs.h>

#include    "mbp.h"
#include    "wfs.h"
#include    "wh.h"

#include    "util.h"
#include    "common.h"


/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  初期化及びメインループ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    /* サンプル用描画フレームワーク初期化 */
    UTIL_Init();

    /*
     * ファイルシステム初期化.
     * 親機の場合には有効な DMA チャンネルを指定することも可能です.
     * 子機の場合には使用されないので単に無視されます.
     */
    FS_Init(FS_DMA_NOT_USE);

    /* LCD表示開始 */
    GX_DispOn();
    GXS_DispOn();

    /* 状態遷移メインループ */
    for (;;)
    {
        /* 前回の WFS がすでに起動中であれば一度終了 */
        if (WFS_GetStatus() != WFS_STATE_STOP)
        {
            WFS_End();
            (void)WH_End();
            while (WH_GetSystemState() != WH_SYSSTATE_STOP) {}
        }

        /* DS ダウンロードプレイを開始 */
        ModeMultiboot();
        
        /* WH モジュールの初期化 */ 
        ModeInitialize();
        /* 親機終了完了まで待機 */
        ModeWorking();
        
        /* モードを設定して親機を開始 */
        ModeSelect();
        /* 親機終了完了まで待機 */
        ModeWorking();
        
        /* 起動したら親機画面を表示 */
        ModeParent();
        
        /* エラーならここで停止 */
        ModeError();
    }
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
