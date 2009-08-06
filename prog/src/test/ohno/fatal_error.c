
//============================================================================================
/**
 * @file	fatal_error.c
 * @brief	fatalエラー画面表示
 * @author	ohno
 * @date	2007.02.06
 */
//============================================================================================


#include <gflib.h>
#include "system/gfl_use.h"
#include "system\main.h"

#include "net_err.naix"
#include "fatal_error.h"

#include "arc_def.h"

//static	const	char	*GraphicFileTable[]={
//	"test_graphic/fatal_error.narc",
//};

//----------------------------------------------------------------------------
/**
 *	@brief	エラー表示
 */
//-----------------------------------------------------------------------------
void FatalError_Disp(GFL_NETHANDLE* pNet,int errNo,void* pWork)
{
	//BGシステム初期化
	GFL_BG_Init(GFL_HEAPID_APP);

    //ARCシステム初期化
//	GFL_ARC_Init(&GraphicFileTable[0],1);	gfl_use.cで1回だけ初期化に変更

	//VRAM設定
	{
		GFL_DISP_VRAM vramSetTable = {
			GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
			GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
			GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
			GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
			GX_VRAM_TEX_0_B,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_0_F,			// テクスチャパレットスロット
			GX_OBJVRAMMODE_CHAR_1D_64K,		// メインOBJマッピングモード
			GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
		};
		GFL_DISP_SetBank( &vramSetTable );

		//VRAMクリア
		MI_CpuClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
		MI_CpuClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
		MI_CpuClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
		MI_CpuClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
	}

	// BG SYSTEM
	{
		GFL_BG_SYS_HEADER BGsys_data = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &BGsys_data );
	}

	//メイン画面フレーム設定
	{
		GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
			///<FRAME2_M
			{
				0, 0, 0x2000, 0, GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
			///<FRAME3_M
			{
				0, 0, 0x2000, 0, GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
			///<FRAME2_S
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
			///<FRAME3_S
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
		};
		GFL_BG_SetBGControl(GFL_BG_FRAME2_M, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME2_M );
		GFL_BG_SetBGControl(GFL_BG_FRAME3_M, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME3_M );
		GFL_BG_SetBGControl(GFL_BG_FRAME2_S, &TextBgCntDat[2], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME2_S );
		GFL_BG_SetBGControl(GFL_BG_FRAME3_S, &TextBgCntDat[3], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME3_S );

		GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_BG0, VISIBLE_OFF );
		GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_BG1, VISIBLE_OFF );
		GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_BG3, VISIBLE_OFF );
		GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_BG2, VISIBLE_ON );

		// OBJマッピングモード
		GX_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );
		GXS_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );

//		GFL_DISP_GX_VisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
//		GFL_DISP_GXS_VisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	}

	//画面生成
	GFL_ARC_UTIL_TransVramBgCharacter(ARCID_NET_ERR,NARC_net_err_net_err_NCGR,GFL_BG_FRAME2_M,0,0,0,GFL_HEAPID_APP);
	GFL_ARC_UTIL_TransVramScreen(ARCID_NET_ERR,NARC_net_err_net_err_NSCR,GFL_BG_FRAME2_M,0,0,0,GFL_HEAPID_APP);
	GFL_ARC_UTIL_TransVramPalette(ARCID_NET_ERR,NARC_net_err_net_err_NCLR,PALTYPE_MAIN_BG,0,0x100,GFL_HEAPID_APP);

	GFL_DISP_SetDispOn();
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_SUB );

	GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN|GFL_FADE_MASTER_BRIGHT_WHITEOUT_SUB,16,0,2);

	while(TRUE){
        GFL_UI_Main();
        GFL_FADE_Main();
		// VBLANK待ち
		// ※gflibに適切な関数が出来たら置き換えてください
		OS_WaitIrq(TRUE,OS_IE_V_BLANK);
	}
}

