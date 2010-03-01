//==============================================================================
/**
 * @file	d_matsu.c
 * @brief	松田デバッグソース
 * @author	matsuda
 * @date	2008.08.26(火)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include "system\main.h"
#include <backup_system.h>
//#include "savedata/contest_savedata.h"
#include "savedata/save_tbl.h"
#include "savedata/save_control.h"


//==============================================================================
//	構造体定義
//==============================================================================
typedef struct {

	u16		seq;
	HEAPID	heapID;
	int debug_mode;
	
	//セーブ関連
	GFL_SAVEDATA *sv_normal;	///<ノーマルセーブデータへのポインタ
}D_MATSU_WORK;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static BOOL DebugMatsuda_SaveSystemTest(D_MATSU_WORK *wk);
static BOOL DebugMatsuda_SaveTest(D_MATSU_WORK *wk);



//--------------------------------------------------------------
/**
 * @brief   
 *
 * @param   proc		
 * @param   seq		
 * @param   pwk		
 * @param   mywk		
 *
 * @retval  
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT DebugMatsudaMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	static const GFL_DISP_VRAM vramBank = {
		GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_0_F,			// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_0123_E,			// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
		GX_OBJVRAMMODE_CHAR_1D_128K,	// サブOBJマッピングモード
	};

	D_MATSU_WORK* wk;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MATSUDA_DEBUG, 0x70000 );
	wk = GFL_PROC_AllocWork( proc, sizeof(D_MATSU_WORK), HEAPID_MATSUDA_DEBUG );
	MI_CpuClear8(wk, sizeof(D_MATSU_WORK));
	wk->heapID = HEAPID_MATSUDA_DEBUG;

//	GFL_ARC_Init( arcFiles, NELEMS(arcFiles) );	gfl_use.cで1回だけ初期化に変更

	GFL_DISP_SetBank( &vramBank );

	// 各種効果レジスタ初期化
	G2_BlendNone();

	// BGsystem初期化
	GFL_BG_Init( wk->heapID );
	GFL_BMPWIN_Init( wk->heapID );

	//ＢＧモード設定
	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysHeader );
	}

	// 個別フレーム設定
	{
		static const GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};

		GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcntText,   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcntText,   GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

//		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0x22, wk->heapID );
		GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0xff, 1, 0 );

//		void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
		GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0x00, 1, 0 );
		GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	}

	// 上下画面設定
	GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

	return GFL_PROC_RES_FINISH;
}
//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugMatsudaMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	D_MATSU_WORK* wk = mywk;
	BOOL ret = 0;
	
	wk->debug_mode = 1;
	switch(wk->debug_mode){
	case 0:		//セーブシステムの作成テスト
		ret = DebugMatsuda_SaveSystemTest(wk);
		break;
	case 1:		//メインで作成されているセーブシステムを使用してセーブテスト
		ret = DebugMatsuda_SaveTest(wk);
		break;
	default:
		ret = TRUE;
	}
	
	if(ret == TRUE){
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}
//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugMatsudaMainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_MATSUDA_DEBUG);
	
	return GFL_PROC_RES_FINISH;
}



//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   セーブテスト(セーブシステムそのものを作る所からのテスト)
 *
 * @param   wk		
 *
 * @retval  TRUE:終了。　FALSE:処理継続中
 */
//--------------------------------------------------------------
static BOOL DebugMatsuda_SaveSystemTest(D_MATSU_WORK *wk)
{
#if 0
	LOAD_RESULT load_ret;
	SAVE_RESULT save_ret;
	
	GF_ASSERT(wk);

	switch( wk->seq ){
	case 0:
		//ノーマルセーブ領域作成
		OS_TPrintf("ノーマルセーブ領域作成\n");
		wk->sv_normal = GFL_SAVEDATA_Create(&SaveParam_Normal, GFL_HEAPID_APP);
		wk->seq++;
		break;
	case 1:
		//コンテストデータを引っ張ってきて中身を表示
		load_ret = GFL_BACKUP_Load(wk->sv_normal, GFL_HEAPID_APP);
		switch(load_ret){
		case LOAD_RESULT_NULL:		///<データなし
			OS_TPrintf("LOAD:データが存在しない\n");
			GFL_SAVEDATA_Clear(wk->sv_normal);
			break;
		case LOAD_RESULT_OK:				///<データ正常読み込み
			OS_TPrintf("LOAD:正常読み込み\n");
			{
				CONTEST_DATA *condata;
				u16 value;
				
				condata = GFL_SAVEDATA_Get(wk->sv_normal, GMDATA_ID_CONTEST);
				value = CONDATA_GetValue(condata, 0,0);
				OS_TPrintf("value = %d\n", value);
			}
			break;
		case LOAD_RESULT_NG:				///<データ異常
			OS_TPrintf("LOAD:データ異常\n");
			GFL_SAVEDATA_Clear(wk->sv_normal);
			break;
		case LOAD_RESULT_BREAK:			///<破壊、復旧不能
			OS_TPrintf("LOAD:データ破壊\n");
			GFL_SAVEDATA_Clear(wk->sv_normal);
			break;
		case LOAD_RESULT_ERROR:			///<機器故障などで読み取り不能
			OS_TPrintf("LOAD:読み取り不能\n");
			GFL_SAVEDATA_Clear(wk->sv_normal);
			break;
		default:
			GF_ASSERT("LOAD:例外エラー！");
			break;
		}
		wk->seq++;
		break;
	case 2:
		//コンテストデータの中身を変更してセーブ実行
		//次回起動した時に中身が変わっているか確認！
		{
			CONTEST_DATA *condata;
			u16 value;
			
			condata = GFL_SAVEDATA_Get(wk->sv_normal, GMDATA_ID_CONTEST);
			value = CONDATA_GetValue(condata, 0,0);
			CONDATA_RecordAdd(condata, 0, 0);
			OS_TPrintf("セーブ実行\n");
			OS_TPrintf("before value = %d, after value = %d\n", value, value+1);
			save_ret = GFL_BACKUP_Save(wk->sv_normal);
			switch(save_ret){
			case SAVE_RESULT_CONTINUE:		///<セーブ処理継続中
				OS_TPrintf("SAVE:継続中\n");
				break;
			case SAVE_RESULT_LAST:			///<セーブ処理継続中、最後の一つ前
				OS_TPrintf("SAVE:継続中 最後の1つ前\n");
				break;
			case SAVE_RESULT_OK:			///<セーブ正常終了
				OS_TPrintf("SAVE:正常終了\n");
				break;
			case SAVE_RESULT_NG:			///<セーブ失敗終了
				OS_TPrintf("SAVE:失敗\n");
				break;
			}
		}
		wk->seq++;
		break;
	case 3:
		OS_TPrintf("セーブシステム破棄\n");
		GFL_SAVEDATA_Delete(wk->sv_normal);
		
		return TRUE;
	}
#endif
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   セーブテスト(セーブシステムはメインで作られているものを使用)
 *
 * @param   wk		
 *
 * @retval  TRUE:終了。　FALSE:処理継続中
 */
//--------------------------------------------------------------
static BOOL DebugMatsuda_SaveTest(D_MATSU_WORK *wk)
{
	return FALSE;
}


//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA DebugMatsudaMainProcData = {
	DebugMatsudaMainProcInit,
	DebugMatsudaMainProcMain,
	DebugMatsudaMainProcEnd,
};


