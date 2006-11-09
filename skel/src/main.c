//===================================================================
/**
 * @file	main.c
 * @brief	アプリケーション　サンプルスケルトン
 * @author	GAME FREAK Inc.
 * @date	06.11.08
 *
 * $LastChangedDate$
 * $Id$
 */
//===================================================================
#include <nitro.h>
#include <nnsys.h>
#include "gflib.h"


void NitroMain(void)
{
  // 初期化して…
  InitSystem();

  // 必要なTCBとか登録して…
  
  while(TRUE){
    // キー入力して…

    // メイン処理して…

    // 描画に必要な準備して…

    // レンダリングエンジンが参照するデータ群をスワップ
    // ※gflibに適切な関数が出来たら置き換えてください
    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);

    // VBLANK待ち
    // ※gflibに適切な関数が出来たら置き換えてください
    SVC_WaitVBlankIntr();
  }
}

/*  */
