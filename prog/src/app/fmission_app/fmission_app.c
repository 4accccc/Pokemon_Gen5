//=============================================================================
/**
 *
 *	@file		fmission_app.c
 *	@brief  フェスミッションアプリ メイン 
 *	@author Miyuki Iwasawa	
 *	@data	  2011.04.22	
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/wipe.h"

#include "print/printsys.h"
#include "ui/print_tool.h"

#include "fmission_app_local.h"
#include "fmission_app_sub.h"

#include "message.naix"

#include "fmission_app.naix"

#include "msg/msg_fmission_app.h"

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
// 段階
enum{
	STEP_INIT	= 0,				// 初期化
	STEP_LV1 	= 100,
	STEP_LV2 	= 200,
	STEP_LV3 	= 300,
	STEP_LV4 	= 400,
	//
	STEP_END	= 10000,
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//=============================================================================
/**
 *							データ定義
 */
//=============================================================================
static const s16 fmai_mes_tbl[] = {
	msg_fmission_str_00,		// のこりじかん
	msg_fmission_str_01,		// さんかにんずう
	msg_fmission_str_02,		// クリア目標
	msg_fmission_str_03,		// トータルスコア
	msg_fmission_str_04,		// ほうしゅう
	msg_fmission_str_07,		// マイスコア
	msg_fmission_str_08,		// 順位
	msg_fmission_str_09,		// トップスコア
	msg_fmission_str_10,		// ハイスコア
	msg_fmission_str_13,		// クリアタイム
	msg_fmission_str_14,		// スコアボーナス
	msg_fmission_str_15,		// ダブルスコア
	//
	//
	msg_fmission_str_05,		// ○にん
	msg_fmission_str_06,		// ひき
	msg_fmission_str_16,		// どうぐめい○こ
	msg_fmission_str_11,		// ○ひき
	msg_fmission_str_12,		// ○にんちゅう○ばんめ
	msg_fmission_str_17,		// ＋○こ
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT FMissionAppProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT FMissionAppProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT FMissionAppProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
//
static void fma_init( FMISSION_APP_WORK* wk );
static void fma_main( FMISSION_APP_WORK* wk );
static void fma_down( FMISSION_APP_WORK* wk );
static void fma_exit( FMISSION_APP_WORK* wk );
static void fma_print_que( FMISSION_APP_WORK* wk );
//
static void fma_scroll_bg( FMISSION_APP_WORK* wk );
//
static void fma_key_info(   FMISSION_APP_WORK* wk );
static void fma_update_info( FMISSION_APP_WORK* wk );
static void fma_update_up_info( FMISSION_APP_WORK* wk );
static void fma_update_down_info( FMISSION_APP_WORK* wk );

//
static void fma_key_result( FMISSION_APP_WORK* wk );
static void fma_update_result( FMISSION_APP_WORK* wk );
static void fma_update_up_result( FMISSION_APP_WORK* wk );
static void fma_update_down_result( FMISSION_APP_WORK* wk );

//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA FMissionAppProcData = 
{
	FMissionAppProc_Init,
	FMissionAppProc_Main,
	FMissionAppProc_Exit,
};

// Mainのテーブル
static void (*(fma_main_func_tbl[]))(FMISSION_APP_WORK*) = {
	fma_key_info,		// 詳細
	fma_key_result,		// 評価
};

//=============================================================================
/**
 *								PROC
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 初期化処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					FMISSION_APP_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT FMissionAppProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	FMISSION_APP_WORK	*wk;
	FMISSION_APP_PARAM	*param;
	
	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//引数取得
	param	= pwk;
	
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_FMISSION_APP, FMISSION_HEAP_SIZE );
	wk = GFL_PROC_AllocWork( proc, sizeof(FMISSION_APP_WORK), HEAPID_FMISSION_APP );
	GFL_STD_MemClear( wk, sizeof(FMISSION_APP_WORK) );
	
	// 初期化
	wk->heapID = HEAPID_FMISSION_APP;
	
	wk->Param = param;
	
	//描画設定初期化
	wk->graphic	= FMISSION_APP_GRA_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );
	
	//フォント作成
	wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	
	//メッセージ
	wk->MsgData[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_fmission_app_dat, wk->heapID );
	wk->MsgData[1] = 0;
	
	//PRINT_QUE作成
	wk->print_que = PRINTSYS_QUE_Create( wk->heapID );
	
	//----------------
	// 処理を記述
	//----------------
	fma_init( wk );
	
	// フェスミッションと同じ
	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, 15, 15);
	G2S_SetBlendAlpha(GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, 15, 15);
	
	// ワイプ
	WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
					WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
	//
	return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					FMISSION_APP_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT FMissionAppProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	s32		i;
	FMISSION_APP_WORK* wk = mywk;
	
	//--------------
	// 破棄
	//--------------
	fma_exit( wk );
	
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	
	//メッセージ破棄
	for(i=0;i<FMA_MSG_MAXNUM;i++){
		if(wk->MsgData[i]){
			GFL_MSG_Delete( wk->MsgData[i] );
			wk->MsgData[i] = 0;
		}
	}
	
	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );
	
	//FONT
	GFL_FONT_Delete( wk->font );
	
	//描画設定破棄
	FMISSION_APP_GRA_Exit( wk->graphic );
	
	//PROC用メモリ解放
	{
		HEAPID heapID = wk->heapID;
		GFL_PROC_FreeWork( proc );
		GFL_HEAP_DeleteHeap( heapID );
	}
	//オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));
	//
	return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					FMISSION_APP_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT FMissionAppProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	FMISSION_APP_WORK* wk = mywk;
	
	switch ( *seq )
	{
		case 0:
			if (WIPE_SYS_EndCheck())
			{
				(*seq) ++;
			}
			break;
		case 1:
			// 終了判定
			if(wk->Step == STEP_END){
				(*seq) ++;
			}
			break;
		case 2:
			WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
							WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
			(*seq) ++;
			break;
		//
		// ここで最終終了待ちをしているようだ
		case 3:
			if (WIPE_SYS_EndCheck())
			{
				return GFL_PROC_RES_FINISH;
			}
			break;
		default:
			break;
	}
	//
	//----------------
	// 処理を追加
	//----------------
	fma_main( wk );
	
	//PRINT_QUE
	fma_print_que( wk );
	
	//2D描画
	FMISSION_APP_GRA_2D_Draw( wk->graphic );
	
	//3D描画
	FMISSION_APP_GRA_3D_StartDraw( wk->graphic );
	FMISSION_APP_GRA_3D_EndDraw( wk->graphic );
	
	return GFL_PROC_RES_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  初期化(init)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_init( FMISSION_APP_WORK* wk )
{
	s32		i;
	
	// 明示的に初期化
	wk->Step	= STEP_INIT;
	wk->Mode	= wk->Param->mode;		// これで分岐
	wk->Flag	= 0;
	wk->BgOffX	= 0;
	//
	for(i=0;i<FMA_CANVAS_MAXNUM;i++){
		wk->PrintUtil[i].win = 0;
	}
	for(i=0;i<FMA_OBJ_MAXNUM;i++){
		wk->wk_obj.clwk[i] = 0;
	}
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_FMISSION_APP, NARC_fmission_app_fmission_app_pos_tblbin, wk->heapID );
	
	// データ読み込み(bg+obj+canvas)
	FmaCmn_data_init( wk, wk->Mode );
	//
	wk->Flag |= FLG_FMA_UPDATE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理(main)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_main( FMISSION_APP_WORK* wk )
{
	fma_down( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(下画面)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_down( FMISSION_APP_WORK* wk )
{
	// 念のため
	if((wk->Mode < 0)||(wk->Mode >= FMISSION_APP_MODE_MAX)){
		OS_Printf("mode error\n");
		return;
	}
	// 分岐
	fma_main_func_tbl[wk->Mode]( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_exit( FMISSION_APP_WORK* wk )
{
	// データ破棄
	FmaCmn_data_exit( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  破棄
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_print_que( FMISSION_APP_WORK* wk )
{
	s32		i;
	
	// Queの処理
	PRINTSYS_QUE_Main( wk->print_que );
	//
	for(i=0;i<FMA_CANVAS_MAXNUM;i++){
		if(wk->PrintUtil[i].win){
			PRINT_UTIL_Trans( &wk->PrintUtil[i], wk->print_que );
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  スクロール
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_scroll_bg( FMISSION_APP_WORK* wk )
{
	s32		off;
	
	off = wk->BgOffX>>8;
	GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_X_SET, off );
	//
	wk->BgOffX += 0x80;
	if(wk->BgOffX >= 0x20000){
		wk->BgOffX -= 0x20000;
	}
}

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理(Info)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_key_info( FMISSION_APP_WORK* wk )
{
	s32		i;
	s32		tmp;
	u32		ret;
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
//			break;
		//
		// 本体
		case STEP_LV1:
			fma_update_info( wk );
			break;
		//
		// END
		case STEP_END:
			break;
		//
		default:
			break;
	}
	// BGスクロール
	fma_scroll_bg( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  情報表示(まとめ)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_update_info( FMISSION_APP_WORK* wk )
{
	s32		flg=0;
	
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		wk->Step = STEP_END;
	}
	// 更新
	if((flg == 1)||(wk->Flag & FLG_FMA_UPDATE)){
		fma_update_up_info( wk );
		fma_update_down_info( wk );
		wk->Flag &= ~FLG_FMA_UPDATE;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  情報表示(上)(要整理)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_update_up_info( FMISSION_APP_WORK* wk )
{
	STRBUF	*strbuf;
	
	// タイトル
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[0] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_UP_TITLE, 0, 4, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	//----------
	// 左側
	//----------
	// 残り時間
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[0] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_UP_TIME_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 参加人数
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[1] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_UP_JOIN_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// クリア目標
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[2] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_UP_CLEAR_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// トータル
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[3] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_UP_SCORE_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 報酬
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[4] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_UP_GAIN_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	
	//----------
	// 右側
	//----------
	// 参加人数
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[12] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_UP_JOIN_R,  48, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// トータル
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[13] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_UP_SCORE_R, 48, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 報酬
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[14] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_UP_GAIN_R, 128, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	
	// 残り時間
	FmaCmn_disp_time( wk, FMA_CLWKMODE_NORMAL, FMAI_CLWK_TIME_00, 5, 26 );
	// トータルスコア
	FmaCmn_disp_number( wk, FMA_CLWKMODE_NORMAL, FMAI_CLWK_SCORE_00, 976, 4, 0 );
	
	//----------
	// スクロール
	//----------
	// 00
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[6] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_UP_SCROLL_00, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 01
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[7] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_UP_SCROLL_01, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  情報表示(下)(要整理)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_update_down_info( FMISSION_APP_WORK* wk )
{
	STRBUF		*strbuf;
	
	//----------
	// 左側
	//----------
	// マイスコア
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[5] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_MYSCORE_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 順位
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[6] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_RANK_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// トップスコア
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[7] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_TOPSCORE_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 名前
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[4] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_NAME_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// ハイスコア
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[8] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_HIGH_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 説明文
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[6] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_EXPLAIN_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	
	//----------
	// 右側
	//----------
	// マイスコア
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[15] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_MYSCORE_R, 64, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 順位
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[16] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_RANK_R, 152, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// トップスコア
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[15] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_TOPSCORE_R, 64, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// ハイスコア
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[15] );
	FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_HIGH_R, 64, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
}

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理(Result)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_key_result( FMISSION_APP_WORK* wk )
{
	s32		i;
	s32		tmp;
	u32		ret;
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
//			break;
		//
		// 本体
		case STEP_LV1:
			fma_update_result( wk );
			break;
		//
		// END
		case STEP_END:
			break;
		//
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  情報表示(Result)(まとめ)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_update_result( FMISSION_APP_WORK* wk )
{
	s32		flg=0;
	
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		wk->Step = STEP_END;
	}
	// 更新
	if((flg == 1)||(wk->Flag & FLG_FMA_UPDATE)){
		fma_update_up_result( wk );
		fma_update_down_result( wk );
		wk->Flag &= ~FLG_FMA_UPDATE;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  情報表示(上)(要整理)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_update_up_result( FMISSION_APP_WORK* wk )
{
	STRBUF	*strbuf;
	
	// タイトル
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[0] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_UP_TITLE, 0, 4, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	//----------
	// 左側
	//----------
	// 参加人数
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[1] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_UP_JOIN_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// クリア目標
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[2] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_UP_CLEAR_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// トータル
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[3] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_UP_SCORE_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// クリアタイム
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[9] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_UP_TIME_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	
	//----------
	// 右側
	//----------
	// 参加人数
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[12] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_UP_JOIN_R,  48, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// トータル
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[13] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_UP_SCORE_R, 48, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	
	// トータルスコア
	FmaCmn_disp_number( wk, FMA_CLWKMODE_NORMAL, FMAR_CLWK_SCORE_00, 35, 4, 0 );
	// クリア時間
	FmaCmn_disp_time( wk, FMA_CLWKMODE_NORMAL, FMAR_CLWK_TIME_00, 0, 8 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  情報表示(下)(要整理)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_update_down_result( FMISSION_APP_WORK* wk )
{
	STRBUF		*strbuf;
	
	//----------
	// 左側
	//----------
	// マイスコア
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[5] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_MYSCORE_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 順位
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[6] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_RANK_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// トップスコア
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[7] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_TOPSCORE_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 名前
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[4] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_NAME_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 報酬
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[4] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_GAIN_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// スコアボーナス
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[10] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_BONUS_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// ダブルスコア
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[11] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_DOUBLE_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	
	//----------
	// 右側
	//----------
	// マイスコア
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[15] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_MYSCORE_R, 64, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 順位
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[16] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_RANK_R, 152, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// トップスコア
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[15] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_TOPSCORE_R, 64, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 報酬
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[14] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_GAIN_R, 128, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// スコアボーナス
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[17] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_BONUS_R, 48, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	//
	// ダブルスコア
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[14] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_DOUBLE_R, 128, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
}

