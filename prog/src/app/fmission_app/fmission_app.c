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

// 情報の項目
enum{
	FMA_MES_EXPLAIN	= 0,			// 説明文
	FMA_MES_TITLE,					// タイトル
	FMA_MES_CLEAR,					// クリア目標
	FMA_MES_GAIN,					// 報酬
	FMA_MES_JOIN,					// 参加人数
	FMA_MES_RANK,					// 順位
	FMA_MES_MYSCORE,				// 自分のスコア
	FMA_MES_TOPSCORE,				// トップスコア
	FMA_MES_TOPSCORE_NAME,			// トップスコア名前
};

//
#define _DEBUG_FMA_
#undef  _DEBUG_FMA_

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
	msg_fmission_str_02,		// クリア目標(fmai_mes_01_tblを使用)
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

// 単位(クリア目標、通常、スクロール文字、pad)
static const s16 fmai_mes_01_tbl[] = {
	msg_fmission_str_24, msg_fmission_str_20, msg_fmission_str_32, 0,		// なし
	msg_fmission_str_21, msg_fmission_str_05, msg_fmission_str_29, 0,		// 「にん」
	msg_fmission_str_02, msg_fmission_str_11, msg_fmission_str_28, 0,		// 「ひき」
	msg_fmission_str_22, msg_fmission_str_18, msg_fmission_str_30, 0,		// 「こ」
	msg_fmission_str_23, msg_fmission_str_19, msg_fmission_str_31, 0,		// 「かい」
};

// 報酬
static const s16 fmai_mes_02_tbl[] = {
	msg_fmission_str_16,		// 「○こ」
	msg_fmission_str_25,		// 「○こ～」
};

// ハイスコア
static const s16 fmai_mes_03_tbl[] = {
	msg_fmission_str_11,		// ○ひき
	msg_fmission_str_26,		// ○ふん○びょう
	msg_fmission_str_27,		// ○ふん
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
static void fma_chk_timer( FMISSION_APP_WORK* wk );

//
static void fma_key_result( FMISSION_APP_WORK* wk );
static void fma_update_result( FMISSION_APP_WORK* wk );
static void fma_update_up_result( FMISSION_APP_WORK* wk );
static void fma_update_down_result( FMISSION_APP_WORK* wk );
//
static void fma_disp_fes_string( FMISSION_APP_WORK *wk, const s32 mode, const s32 snum, const s32 x, const s32 y, const s32 lmode );
//
static void fma_disp_highscore( FMISSION_APP_WORK *wk );
static void fma_disp_scroll_info( FMISSION_APP_WORK *wk, const s32 num, const s32 snum );

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
	
	// WORDSET生成
	wk->Wordset = WORDSET_Create( wk->heapID );
	
	//メッセージ
	wk->MsgData[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_fmission_app_dat, wk->heapID );
	wk->MsgData[1] = 0;
	
	//PRINT_QUE作成
	wk->print_que = PRINTSYS_QUE_Create( wk->heapID );
	
	wk->FMissionManager = FMISSION_DATA_Init( wk->heapID );
	wk->Handle  = GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, wk->heapID );
	wk->ArcData = FMISSION_DATA_AllocArcAllByHandle( wk->Handle, wk->heapID );
	// トップスコア用
#ifdef _DEBUG_FMA_
	wk->TopScore = LINKFEST_LOG_TOP_SCORE_Create( wk->heapID );
#endif
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
	
	// WORDSET破棄
	WORDSET_Delete( wk->Wordset );
	
	// 破棄
	FMISSION_DATA_Release( wk->FMissionManager );
	wk->FMissionManager = 0;
	GFL_ARC_CloseDataHandle( wk->Handle );
	GFL_HEAP_FreeMemory( wk->ArcData );
	//
#ifdef _DEBUG_FMA_
	LINKFEST_LOG_TOP_SCORE_Delete( wk->TopScore );
#endif
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
	wk->Timer	= 1;
	//
	for(i=0;i<FMA_CANVAS_MAXNUM;i++){
		wk->PrintUtil[i].win = 0;
	}
	for(i=0;i<FMA_OBJ_MAXNUM;i++){
		wk->wk_obj.clwk[i] = 0;
	}
	for(i=0;i<FMA_CMN_OBJ_MAXNUM;i++){
		wk->wk_obj.cmn_clwk[i] = 0;
	}
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_FMISSION_APP, NARC_fmission_app_fmission_app_pos_tblbin, wk->heapID );
	//
	// データ読み込み(bg+obj+canvas)
	FmaCmn_data_init( wk, wk->Mode );
	//
	// 初期パラメータの取得
	FmaCmn_main_festival( wk, 0 );
	//
	wk->Flag |= FLG_FMA_UPDATE;
	//
	// よぶとだめみたいだ..??
//	LINK_FESTIVAL_TopScoreGet( wk->Param->fest_sys, wk->TopScore );
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
	// 処理
	bflg = FmaCmn_main_festival( wk, 1 );
	
	// タイマ表示
	fma_chk_timer( wk );
	
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
		wk->Flag |= FLG_FMA_DISP_1ST;
		wk->Flag &= ~FLG_FMA_UPDATE;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  詳細表示(上)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_update_up_info( FMISSION_APP_WORK* wk )
{
	s32		id;
	s32		buf[2];
	STRBUF	*strbuf;
	
	// 1回目のみ
	if(!(wk->Flag & FLG_FMA_DISP_1ST)){
		//----------
		// タイトル
		//----------
		fma_disp_fes_string( wk, FMA_MES_TITLE, FMAI_CANVAS_UP_TITLE, 0, 4, PRINTTOOL_MODE_LEFT );
		//
		// 星
		FmaCmn_disp_star( wk );
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
		fma_disp_fes_string( wk, FMA_MES_CLEAR, FMAI_CANVAS_UP_CLEAR_L, 0, 0, PRINTTOOL_MODE_LEFT );
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
		// トータル(単位)
		id = wk->Param->fmwk->beacon.m_id;
		buf[0] = wk->ArcData[id].common.unit;
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_01_tbl[4*buf[0]+1] );
		FmaCmn_disp_string( wk, FMAI_CANVAS_UP_SCORE_R, 48, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer( strbuf );
		//
		// 報酬
		fma_disp_fes_string( wk, FMA_MES_GAIN, FMAI_CANVAS_UP_GAIN_R, 128, 0, PRINTTOOL_MODE_RIGHT );
	}
	
	//----------
	// 右側(可変)
	//----------
	// 参加人数
	fma_disp_fes_string( wk, FMA_MES_JOIN, FMAI_CANVAS_UP_JOIN_R, 48, 0, PRINTTOOL_MODE_RIGHT );
	//
	// トータルスコア
	buf[0] = wk->MissionStatus.total_score;
	FmaCmn_disp_number( wk, FMA_CLWKMODE_NORMAL, FMAI_CLWK_SCORE_00, buf[0], 4, 0 );
	
	
	//------------
	// スクロール
	//------------
	fma_disp_scroll_info( wk, FMAI_CANVAS_UP_SCROLL_00, 0 );
	fma_disp_scroll_info( wk, FMAI_CANVAS_UP_SCROLL_01, 0 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  詳細表示(下)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_update_down_info( FMISSION_APP_WORK* wk )
{
	STRBUF		*strbuf;
	
	// 1回目のみ
	if(!(wk->Flag & FLG_FMA_DISP_1ST)){
		//----------
		// 左側
		//----------
		// 説明文
		fma_disp_fes_string( wk, FMA_MES_EXPLAIN, FMAI_CANVAS_DOWN_EXPLAIN_L, 0, 0, PRINTTOOL_MODE_LEFT );
	}
	
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
	
	//----------
	// 右側
	//----------
	// マイスコア
	fma_disp_fes_string( wk, FMA_MES_MYSCORE, FMAI_CANVAS_DOWN_MYSCORE_R, 64, 0, PRINTTOOL_MODE_RIGHT );
	//
	// 順位
	fma_disp_fes_string( wk, FMA_MES_RANK, FMAI_CANVAS_DOWN_RANK_R, 152, 0, PRINTTOOL_MODE_RIGHT );
	//
#ifdef _DEBUG_FMA_
	// トップスコア名前(保留)
	fma_disp_fes_string( wk, FMA_MES_TOPSCORE_NAME, FMAI_CANVAS_DOWN_NAME_L, 0, 0, PRINTTOOL_MODE_LEFT );
	// トップスコア(保留)
	fma_disp_fes_string( wk, FMA_MES_TOPSCORE, FMAI_CANVAS_DOWN_TOPSCORE_R, 64, 0, PRINTTOOL_MODE_RIGHT );
#endif
	// ハイスコア
	fma_disp_highscore( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  残り時間表示
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_chk_timer( FMISSION_APP_WORK* wk )
{
	s16		buf[2];
	u16		timer;
	//
	// 念のため
	if(wk->Timer == 0){
		return;
	}
	timer = LINK_FESTIVAL_GetMissionLife( wk->Param->fest_sys );
	if(wk->Timer != timer){
		buf[0] = timer/60;
		buf[1] = timer%60;
		// 残り時間
		FmaCmn_disp_time( wk, FMA_CLWKMODE_NORMAL, FMAI_CLWK_TIME_00, buf[0], buf[1] );
		wk->Timer = timer;
	}
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
 *	@brief  結果表示(上)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_update_up_result( FMISSION_APP_WORK* wk )
{
	s32		id;
	s32		buf[2];
	STRBUF	*strbuf;
	
	// タイトル
	fma_disp_fes_string( wk, FMA_MES_TITLE, FMAR_CANVAS_UP_TITLE, 0, 4, PRINTTOOL_MODE_LEFT );
	// 星
	FmaCmn_disp_star( wk );
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
	fma_disp_fes_string( wk, FMA_MES_CLEAR, FMAR_CANVAS_UP_CLEAR_L, 0, 0, PRINTTOOL_MODE_LEFT );
	//
	// トータルスコア
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
	fma_disp_fes_string( wk, FMA_MES_JOIN, FMAR_CANVAS_UP_JOIN_R, 48, 0, PRINTTOOL_MODE_RIGHT );
	//
	// トータル
	buf[0] = wk->MissionStatus.total_score;
	FmaCmn_disp_number( wk, FMA_CLWKMODE_NORMAL, FMAR_CLWK_SCORE_00, buf[0], 4, 0 );
	
	// トータル(単位)
	id = wk->Param->fmwk->beacon.m_id;
	buf[0] = wk->ArcData[id].common.unit;
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_01_tbl[4*buf[0]+1] );
	FmaCmn_disp_string( wk, FMAR_CANVAS_UP_SCORE_R, 48, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	
	// クリア時間
	FmaCmn_disp_time( wk, FMA_CLWKMODE_NORMAL, FMAR_CLWK_TIME_00, 0, 8 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  結果表示(下)
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
	fma_disp_fes_string( wk, FMA_MES_MYSCORE, FMAR_CANVAS_DOWN_MYSCORE_R, 64, 0, PRINTTOOL_MODE_RIGHT );
	//
	// 順位
	fma_disp_fes_string( wk, FMA_MES_RANK, FMAR_CANVAS_DOWN_RANK_R, 152, 0, PRINTTOOL_MODE_RIGHT );
	//
#ifdef _DEBUG_FMA_
	// 名前(保留)
	fma_disp_fes_string( wk, FMA_MES_TOPSCORE_NAME, FMAR_CANVAS_DOWN_NAME_L, 0, 0, PRINTTOOL_MODE_LEFT );
	// トップスコア(保留)
	fma_disp_fes_string( wk, FMA_MES_TOPSCORE, FMAR_CANVAS_DOWN_TOPSCORE_R, 64, 0, PRINTTOOL_MODE_RIGHT );
#endif
	// 報酬
	fma_disp_fes_string( wk, FMA_MES_GAIN, FMAR_CANVAS_DOWN_GAIN_R, 128, 0, PRINTTOOL_MODE_RIGHT );
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

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//--------------------------------------------------------------
/**
 * 文字列表示
 *
 * @param   FMISSION_APP_WORK		*wk
 * @param   const s32				(FMA_MES_*)
 * @param   const s32				(文字列番号)
 * @param   const s32,s32			(座標)
 * @param   const s32				(モード)
 */
//--------------------------------------------------------------
static void fma_disp_fes_string( FMISSION_APP_WORK *wk, const s32 mode, const s32 snum, const s32 x, const s32 y, const s32 lmode )
{
	s32			id;
	s32			lv;
	s32			buf[2];
	STRBUF		*str;
	STRBUF		*str_tmp;
	//
	// パラメータ
	id = wk->Param->fmwk->beacon.m_id;
	lv = wk->Param->fmwk->beacon.m_level;
	//
	if(mode == FMA_MES_EXPLAIN){		// 説明文
		str = GFL_STR_CreateBuffer( 256, wk->heapID );
		FMISSION_DATA_GetExpStr( wk->FMissionManager, str, wk->Param->fmwk, wk->heapID );
	}
	else if(mode == FMA_MES_TITLE){		// タイトル
		str = GFL_STR_CreateBuffer( 128, wk->heapID );
		FMISSION_DATA_GetTitleStr( wk->FMissionManager, str, wk->Param->fmwk, wk->heapID );
	}
	else if(mode == FMA_MES_CLEAR){		// クリア目標
		// 目標取得
		buf[0] = wk->ArcData[id].param.level[lv].goal;
		// 単位
		buf[1] = wk->ArcData[id].common.unit;
		WORDSET_RegisterNumber( wk->Wordset, 0, buf[0], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
		str = GFL_STR_CreateBuffer( 64, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_01_tbl[4*buf[1]] );
		WORDSET_ExpandStr( wk->Wordset, str, str_tmp );
		GFL_STR_DeleteBuffer(str_tmp);
	}
	else if(mode == FMA_MES_GAIN){		// 報酬
		// 報酬アイテム取得
		buf[0] = wk->ArcData[id].param.item;
		// 報酬数取得
		buf[1] = wk->ArcData[id].param.level[lv].pay;
		//
		WORDSET_RegisterItemName( wk->Wordset, 0, buf[0] );
		WORDSET_RegisterNumber(   wk->Wordset, 1, buf[1], 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		str = GFL_STR_CreateBuffer( 128, wk->heapID );
		if(wk->ArcData[id].param.pay_bonus == FMISSION_PAY_BONUS_NONE){
			str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_02_tbl[0] );
		}else{
			str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_02_tbl[1] );
		}
		WORDSET_ExpandStr( wk->Wordset, str, str_tmp );
		GFL_STR_DeleteBuffer(str_tmp);
	}
	else if(mode == FMA_MES_JOIN){		// 参加人数
		// 参加人数
		buf[0] = wk->MissionStatus.entry_num;
		WORDSET_RegisterNumber( wk->Wordset, 0, buf[0], 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		str = GFL_STR_CreateBuffer( 128, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[12] );
		WORDSET_ExpandStr( wk->Wordset, str, str_tmp );
		GFL_STR_DeleteBuffer(str_tmp);
	}
	else if(mode == FMA_MES_RANK){		// 順位
		// 参加人数
		buf[0] = wk->MissionStatus.entry_num;
		// 自分の順位
		buf[1] = wk->MissionStatus.my_ranking;
		//
		WORDSET_RegisterNumber( wk->Wordset, 0, buf[0], 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		WORDSET_RegisterNumber( wk->Wordset, 1, buf[1], 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		str = GFL_STR_CreateBuffer( 128, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[16] );
		WORDSET_ExpandStr( wk->Wordset, str, str_tmp );
		GFL_STR_DeleteBuffer(str_tmp);
	}
	else if(mode == FMA_MES_MYSCORE){	// 自分のスコア
		// 自分のスコア
		buf[0] = wk->MissionStatus.my_score;
		// 単位
		buf[1] = wk->ArcData[id].common.unit;
		WORDSET_RegisterNumber( wk->Wordset, 0, buf[0], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		str = GFL_STR_CreateBuffer( 128, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_01_tbl[4*buf[1]+1] );
		WORDSET_ExpandStr( wk->Wordset, str, str_tmp );
		GFL_STR_DeleteBuffer(str_tmp);
	}
	else if(mode == FMA_MES_TOPSCORE){	// トップスコア
		// トップスコア
		LINK_FESTIVAL_TopScoreGet( wk->Param->fest_sys, wk->TopScore );
		if(wk->TopScore->name == 0){
			OS_Printf("name error\n");
			return;
		}
		buf[0] = wk->TopScore->score;
		// 単位
		buf[1] = wk->ArcData[id].common.unit;
		WORDSET_RegisterNumber( wk->Wordset, 0, buf[0], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		str = GFL_STR_CreateBuffer( 128, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_01_tbl[4*buf[1]+1] );
		WORDSET_ExpandStr( wk->Wordset, str, str_tmp );
		GFL_STR_DeleteBuffer(str_tmp);
	}
	else if(mode == FMA_MES_TOPSCORE_NAME){	// トップスコア名前
		// トップスコア名前
		LINK_FESTIVAL_TopScoreGet( wk->Param->fest_sys, wk->TopScore );
		if(wk->TopScore->name == 0){
			OS_Printf("name error\n");
			return;
		}
		// 名前
		FmaCmn_disp_string( wk, snum, x, y, wk->TopScore->name, lmode, PRINTSYS_LSB_Make(15,2,0));
		return;		// ここだけ破棄しない
	}
	FmaCmn_disp_string( wk, snum, x, y, str, lmode, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer(str);
}

//--------------------------------------------------------------
/**
 * ハイスコア表示
 *
 * @param   FMISSION_APP_WORK		*wk
 */
//--------------------------------------------------------------
static void fma_disp_highscore( FMISSION_APP_WORK *wk )
{
	u8			buf[2];
	s32			id;
	s32			flg=0;
	s32			dflg=0;
	s32			num;
	STRBUF		*str;
	STRBUF		*expbuf;
	SAVE_CONTROL_WORK *work = GAMEDATA_GetSaveControlWork( wk->Param->gdata );
	FMISSION_SAVE *fsave 	= SAVEDATA_GetFMissionSave( work );
	//
	id = wk->Param->fmwk->beacon.m_id;
	//
	if(wk->Param->fmwk->beacon.end_type == FMISSION_ENDTYPE_TIME){
		// 時間優先=>スコアを表示
		flg = 0;
		num = FMISSION_SAVE_GetMissionStatusHighScoreNum( fsave, id );
		if(num == 0){
			dflg = 1;
		}
	}
	else if(wk->Param->fmwk->beacon.end_type == FMISSION_ENDTYPE_SCORE){
		// スコア優先 => 時間を表示
		flg = 1;
		FMISSION_SAVE_GetMissionStatusHighScoreTime( fsave, id, &buf[0], &buf[1] );
		if((buf[0] == 0)&&(buf[1] == 0)){
			dflg = 1;
		}
	}
//  // 一旦クリア
//  fmission_clear_string( wk, BMPWIN_SCORE_L );
//  fmission_clear_string( wk, BMPWIN_SCORE_R );
	//
	// test
//	if(dflg == 0){
		// ハイスコア
		str = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[8] );
		FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_HIGH_L, 0, 0, str, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer( str );
		//
		if(flg == 0){		// スコア
			WORDSET_RegisterNumber(wk->Wordset, 0, num, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
			str = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_03_tbl[0] );
		}else{			// 時間
			if(buf[1] == 0){     // 分のみ
				WORDSET_RegisterNumber(wk->Wordset, 0, buf[0], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
				str = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_03_tbl[2] );
			}else{               // 秒こみ
				WORDSET_RegisterNumber(wk->Wordset, 0, buf[0], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
				WORDSET_RegisterNumber(wk->Wordset, 1, buf[1], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
				str = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_03_tbl[1] );
			}
		}
		expbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
		WORDSET_ExpandStr( wk->Wordset, expbuf, str );
		// 表示
		FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_HIGH_R, 64, 0, expbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer(str);
		GFL_STR_DeleteBuffer(expbuf);
//	}
}

//--------------------------------------------------------------
/**
 * スクロール文字セット
 *
 * @param   FMISSION_APP_WORK		*wk
 * @param   const s32				(番号)
 * @param   const s32				(人の情報(仮))
 */
//--------------------------------------------------------------
static void fma_disp_scroll_info( FMISSION_APP_WORK *wk, const s32 num, const s32 snum )
{
	s32			buf[4];
	s32			id;
	STRBUF		*strbuf;
	STRBUF		*expbuf;
	
	id = wk->Param->fmwk->beacon.m_id;
	buf[0] = wk->ArcData[id].common.unit;
	buf[1] = 0;
	buf[2] = 9876;
	
	//----------
	// スクロール
	//----------
	// (00 or 01)
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_01_tbl[4*buf[0]+2] );
	WORDSET_RegisterNumber(wk->Wordset, 0, buf[1], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);	// test
	WORDSET_RegisterNumber(wk->Wordset, 1, buf[2], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
	expbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
	WORDSET_ExpandStr( wk->Wordset, expbuf, strbuf );
	FmaCmn_disp_string( wk, num, 0, 0, expbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	GFL_STR_DeleteBuffer( expbuf );
}

