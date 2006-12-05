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
#include "gfl_use.h"
#include "main.h"

static	void	SkeltonVBlankFunc(void);

void NitroMain(void)
{
	// 初期化して…
	GFLUser_Init();

	//VBLANK割り込み許可
	OS_SetIrqFunction(OS_IE_V_BLANK,SkeltonVBlankFunc);

	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)OS_EnableIrq();

	(void)GX_VBlankIntr(TRUE);

	// 必要なTCBとか登録して…
  
	while(TRUE){
		// メイン処理して…
		GFLUser_Main();

		// 描画に必要な準備して…

		// レンダリングエンジンが参照するデータ群をスワップ
		// ※gflibに適切な関数が出来たら置き換えてください
		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);

		// VBLANK待ち
		// ※gflibに適切な関数が出来たら置き換えてください
		OS_WaitIrq(TRUE,OS_IE_V_BLANK);
	}
}

static	void	SkeltonVBlankFunc(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);

	MI_WaitDma(GX_DEFAULT_DMAID);

	GFLUser_VIntr();
}

/*  */
