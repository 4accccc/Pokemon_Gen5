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

#include "gamesystem/fmission_log.h"
#include "gamesystem/fmission_log_accessor_fapp.h"
#include "gamesystem/link_festival.h"
#include "gamesystem/link_festival_fapp.h"

#include "fmission_app_local.h"
#include "fmission_app_sub.h"

#include "message.naix"

#include "fmission_app.naix"

#include "fmission_app_snd_def.h"

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
	FMA_MES_JOIN,					// 参加人数
	FMA_MES_RANK,					// 順位
	FMA_MES_MYSCORE,				// 自分のスコア
	FMA_MES_TOPSCORE,				// トップスコア
	FMA_MES_TOPSCORE_NAME,			// トップスコア名前
	FMA_MES_TOTAL_SCORE,			// トータルスコア(単位)
	FMA_MES_GAIN_RESULT,			// 報酬(結果表示)
	FMA_MES_SCORE_BONUS,			// スコアボーナス
	FMA_MES_DOUBLE,					// ダブルスコア
};

// いろいろ
enum{
	FMA_SEQ_CANCEL	= 9,			// CANCELのSEQ
	//
	FMA_TP_CANCEL	= 0,			// CANCELのTP
	//
	FMA_DATA_MAX	= 32,			// データ最大
	//
	FMA_EMPTY_INFO		= 4,		// infoのempty
	FMA_EMPTY_RESULT	= 7,		// resultのempty
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
	msg_fmission_str_42,		// クリア目標
	msg_fmission_str_03,		// トータルスコア
	msg_fmission_str_04,		// ほうしゅう
	msg_fmission_str_07,		// マイスコア
	msg_fmission_str_08,		// 順位
	msg_fmission_str_09,		// トップスコア
	msg_fmission_str_10,		// ハイスコア
	msg_fmission_str_13,		// クリアタイム
	//
	msg_fmission_str_14,		// スコアボーナス
	msg_fmission_str_15,		// ダブルスコア
	msg_fmission_str_05,		// ○にん
	msg_fmission_str_17,		// ＋○こ
	msg_fmission_str_16,		// どうぐめい○こ
	msg_fmission_str_12,		// ○にんちゅう○ばんめ
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

// 固定文字(詳細、上)
static const s16 fmai_fix_mes_00_tbl[] = {
	msg_fmission_str_00, FMAI_CANVAS_UP_TIME_L,		// のこりじかん
	msg_fmission_str_01, FMAI_CANVAS_UP_JOIN_L,		// さんかにんずう
	msg_fmission_str_42, FMAI_CANVAS_UP_CLEAR_L,	// クリア目標
	msg_fmission_str_03, FMAI_CANVAS_UP_SCORE_L,	// トータルスコア
	-1,-1,
};

// クリアCanvas(詳細、下)
static const s16 fmai_info_clear_tbl[] = {
	FMAI_CANVAS_DOWN_MYSCORE_L,
	FMAI_CANVAS_DOWN_RANK_L,
	FMAI_CANVAS_DOWN_TOPSCORE_L,
	FMAI_CANVAS_DOWN_NAME_L,
	//
	FMAI_CANVAS_DOWN_MYSCORE_R,
	FMAI_CANVAS_DOWN_RANK_R,
	FMAI_CANVAS_DOWN_TOPSCORE_R,
	-1,
};

// タイマ
static const s16 fmar_wait_tbl[] = {
	  0,		// lv0
	 60,		// lv1
	120,		// lv2
	180,		// lv3
	240,		// lv4
	241,		// 待ち
	-1,
};

// ME
static const u16 fmai_me_tbl[] = {
	SND_FMA_ITEM,		// 獲得
	SND_FMA_CLEAR,		// クリア
	SND_FMA_MISS,		// ミス
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
static void fma_init_info( FMISSION_APP_WORK* wk );
static void fma_key_info(   FMISSION_APP_WORK* wk );
static void fma_update_info( FMISSION_APP_WORK* wk );
static void fma_update_up_info( FMISSION_APP_WORK* wk );
static void fma_update_down_info( FMISSION_APP_WORK* wk );
static void fma_chk_diff( FMISSION_APP_WORK* wk );
static void fma_clear_info_down( FMISSION_APP_WORK* wk );
static void fma_chk_scroll( FMISSION_APP_WORK* wk );
//
static void fma_init_result( FMISSION_APP_WORK* wk );
static void fma_key_result( FMISSION_APP_WORK* wk );
static void fma_update_result( FMISSION_APP_WORK* wk );
static void fma_update_up_result( FMISSION_APP_WORK* wk );
static void fma_update_down_result( FMISSION_APP_WORK* wk );
//
static void fma_disp_fes_string( FMISSION_APP_WORK *wk, const s32 mode, const s32 snum, const s32 x, const s32 y, const s32 lmode );
//
static void fma_disp_highscore( FMISSION_APP_WORK *wk );
static void fma_disp_total_score( FMISSION_APP_WORK *wk, const s32 mode );
static void fma_disp_scroll_info( FMISSION_APP_WORK *wk, const s32 num );
static void fma_chk_log_buffer( FMISSION_APP_WORK *wk );
static void fma_chk_latest( FMISSION_APP_WORK *wk, const s32 num );
static void fma_set_latest( FMISSION_APP_WORK *wk, const s32 src, const s32 dest );

//
static void fma_disp_fix_string( FMISSION_APP_WORK *wk, const s32 mode );
static void fma_chk_level_timer( FMISSION_APP_WORK *wk );
//
static BOOL fma_chk_meswin( FMISSION_APP_WORK *wk );
static s32 fma_chk_itemget( FMISSION_APP_WORK *wk, const s32 itemid );
//
static void fma_snd_me( FMISSION_APP_WORK *wk, const s32 mode );
static BOOL fma_is_snd_me_end( FMISSION_APP_WORK *wk );

//
static void fma_debug( FMISSION_APP_WORK *wk );

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

// Initのテーブル
static void (*(fma_init_func_tbl[]))(FMISSION_APP_WORK*) = {
	fma_init_info,		// 詳細
	fma_init_result,	// 評価
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
	
	// 会話ウインドウ用
	wk->MsgTcblSys = GFL_TCBL_Init( wk->heapID, wk->heapID, 32 , 32 );
	wk->PrintStream = 0;
	wk->MsgBuffer	= 0;
	
	// 汎用PrintStream用
	APP_PRINTSYS_COMMON_PrintStreamInit( &wk->PrintSysWork, APP_PRINTSYS_COMMON_TYPE_BOTH );
	// メッセージ送りマーク用
	wk->MsgCursorWork = APP_KEYCURSOR_Create( 15, TRUE, TRUE, wk->heapID );
	
	// ミッション関連
	wk->FMissionManager = FMISSION_DATA_Init( wk->heapID );
	wk->Handle  = GFL_ARC_OpenDataHandle( ARCID_FMISSION_DATA, wk->heapID );
	wk->ArcData = FMISSION_DATA_AllocArcAllByHandle( wk->Handle, wk->heapID );
	// トップスコア用
	wk->TopScore = LINKFEST_LOG_TOP_SCORE_Create( wk->heapID );
	wk->Latest = FMISSION_LOG_LATEST_Create( wk->heapID );
	// ミッションの状態取得
	LINK_FESTIVAL_GetMissionStatus( wk->Param->fest_sys, &wk->MissionStatus  );
	
	//----------------
	// 処理を記述
	//----------------
	fma_init( wk );
	
	// フェスミッションと同じ
	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, 15, 15);
	G2S_SetBlendAlpha(GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, 15, 15);
	
	// ワイプ
	if(wk->Mode == FMISSION_APP_MODE_INFO){		// 詳細(黒で入り)
		WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
					WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
	}else{										// 結果(白で入り)
		WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
					WIPE_FADE_WHITE, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
	}
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
	LINKFEST_LOG_TOP_SCORE_Delete( wk->TopScore );
	FMISSION_LOG_LATEST_Delete( wk->Latest );
	
	// 文字送り用
	APP_KEYCURSOR_Delete( wk->MsgCursorWork );
	
	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );
	// 会話ウインドウ用
	GFL_TCBL_Exit( wk->MsgTcblSys );
	
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
	BOOL	bflg;
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
				//--------
				// Que待ち
				//--------
				bflg = PRINTSYS_QUE_IsFinished( wk->print_que );
				if(bflg == TRUE){
					OS_Printf("que end(fmission_app)\n");
					(*seq) ++;
				}
			}
			break;
		case 2:
			if(wk->Mode == FMISSION_APP_MODE_INFO){		// 詳細(黒で終了)
				WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
							WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
			}else{										// 結果(白で終了)
				WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
							WIPE_FADE_WHITE, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
			}
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
	
	// 会話ウインドウ用
	GFL_TCBL_Main( wk->MsgTcblSys );
	
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
	wk->LevelTimer = 0;
	wk->Level	= 0;
	//
	//
#ifdef PM_DEBUG
	// R押しながら(debug)
	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ){
//		wk->Flag |= FLG_FMA_DEBUG;
	}
#endif
	wk->OldWork.Timer	= -1;
	wk->OldWork.Num 	= -1;
	wk->OldWork.Score 	= -1;
	wk->OldWork.Rank	= -1;
	//
	wk->ScoreLog.strbuf = 0;
	wk->ScoreLog.score  = 0;
	wk->ScoreLog.tr_id	= 0;
	//
	wk->LatestLocal.num = 0;
	for(i=0;i<FMISSION_LOG_LATEST_NUM_MAX;i++){
		wk->LatestLocal.log[i].name  = 0;	// 破棄忘れずに
		wk->LatestLocal.log[i].score = 0;
		wk->LatestLocal.log[i].tr_id = 0;
	}
	for(i=0;i<FMA_CANVAS_MAXNUM;i++){
		wk->PrintUtil[i].win = 0;
	}
	for(i=0;i<FMA_OBJ_MAXNUM;i++){
		wk->wk_obj.clwk[i] = 0;
	}
	for(i=0;i<FMA_CMN_OBJ_MAXNUM;i++){
		wk->wk_obj.cmn_clwk[i] = 0;
	}
	for(i=0;i<FMA_OBJ_RD_MAXNUM;i++){
		wk->wk_obj.rd_clwk[i] = 0;
	}
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_FMISSION_APP, NARC_fmission_app_fmission_app_pos_tblbin, wk->heapID );
	//
	// データ読み込み(bg+obj+canvas)
	FmaCmn_data_init( wk, wk->Mode );
	//
	// 個別の初期化
	fma_init_func_tbl[wk->Mode]( wk );
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
	s32		i;
	//
	// データ破棄
	FmaCmn_data_exit( wk );
	//
	// 破棄
	if(wk->ScoreLog.strbuf){
		GFL_STR_DeleteBuffer( wk->ScoreLog.strbuf );
		wk->ScoreLog.strbuf = 0;
	}
	// 破棄
	for(i=0;i<FMISSION_LOG_LATEST_NUM_MAX;i++){
		if(wk->LatestLocal.log[i].name){
			GFL_STR_DeleteBuffer( wk->LatestLocal.log[i].name );
			wk->LatestLocal.log[i].name = 0;
		}
	}
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
	
	// 停止
	if(wk->Flag & FLG_FMA_SCROLL_STOP){
		return;
	}
	
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
 *	@brief  初期化(Info)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_init_info( FMISSION_APP_WORK* wk )
{
	// 1人の場合
	if( wk->MissionStatus.entry_num <= 1 ){
		wk->Flag |= FLG_FMA_ONLY;
	}
}

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
		// 終了待ち
		case STEP_LV2:
			bflg = FmaCmn_is_anime_end( wk, FMA_CLWKMODE_CMN, FMA_CLWK_CMN_BACK );
			if(bflg == FALSE){
				wk->Step = STEP_END;
			}
			break;
		//
		// END
		case STEP_END:
			break;
		//
		default:
			break;
	}
	// スクロール処理
	fma_chk_scroll( wk );
	
	// 差分チェック(タイマなど)
	fma_chk_diff( wk );
	
	// BGスクロール
	fma_scroll_bg( wk );
	//
	// debug
	fma_debug( wk );
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
	s32		id;
	
	// タッチ
	id = FmaCmn_chk_tp( wk );
	if(id == FMA_TP_CANCEL){
		flg = 1;
	}
	else{
		// キー入力
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
			flg = 1;
		}
	}
	// 判定
	if(flg == 1){
		wk->Step = STEP_LV2;
		FmaCmn_chg_objseq( wk, FMA_CLWKMODE_CMN, FMA_CLWK_CMN_BACK, FMA_SEQ_CANCEL );
		// SE
		PMSND_PlaySE( SND_FMA_CANCEL );
	}
	//
	// 更新
	if(wk->Flag & FLG_FMA_UPDATE){
		fma_update_up_info( wk );
		fma_update_down_info( wk );
		//
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
	s32		buf[2];
	
	// 1回目のみ
	if(!(wk->Flag & FLG_FMA_DISP_1ST)){
		//----------
		// タイトル
		//----------
		fma_disp_fes_string( wk, FMA_MES_TITLE, FMAI_CANVAS_UP_TITLE, 0, 4, PRINTTOOL_MODE_LEFT );
		//
		// 星(obj)
		FmaCmn_disp_star( wk, FMAI_CLWK_STAR );
		//
		//----------
		// 左側
		//----------
		// 固定文字列(残り時間、参加人数、トータル、報酬)
		fma_disp_fix_string( wk, 0 );
		//
		// クリア目標
		fma_disp_fes_string( wk, FMA_MES_CLEAR, FMAI_CANVAS_UP_CLEAR_R, 48, 0, PRINTTOOL_MODE_RIGHT );
		//
		//----------
		// 右側
		//----------
		// トータル(単位)
		fma_disp_fes_string( wk, FMA_MES_TOTAL_SCORE, FMAI_CANVAS_UP_SCORE_R, 48, 0, PRINTTOOL_MODE_RIGHT );
	}
	
	//----------
	// 右側(可変)
	//----------
	// 参加人数
	FmaCmn_clear_canvas( wk, FMAI_CANVAS_UP_JOIN_R );
	fma_disp_fes_string( wk, FMA_MES_JOIN, FMAI_CANVAS_UP_JOIN_R, 48, 0, PRINTTOOL_MODE_RIGHT );
	//
	// トータルスコア
	fma_disp_total_score( wk, 0 );
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
		//
		//------------
		// ハイスコア
		//------------
//		fma_disp_highscore( wk );
	}
	
	//---------------------------------------
	// on/offがあるので項目ごとにまとめておく
	//---------------------------------------
	// 一度クリアしておく
	fma_clear_info_down( wk );
	//
	// 各項目表示
	if(!(wk->Flag & FLG_FMA_ONLY)){
		//------------
		// マイスコア
		//------------
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[5] );
		FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_MYSCORE_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer( strbuf );
		//
		// マイスコア(右側)
		fma_disp_fes_string( wk, FMA_MES_MYSCORE, FMAI_CANVAS_DOWN_MYSCORE_R, 64, 0, PRINTTOOL_MODE_RIGHT );
		
		//------
		// 順位
		//------
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[6] );
		FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_RANK_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer( strbuf );
		//
		// 順位(右側)
		fma_disp_fes_string( wk, FMA_MES_RANK, FMAI_CANVAS_DOWN_RANK_R, 152, 0, PRINTTOOL_MODE_RIGHT );
		
		//--------------
		// トップスコア
		//--------------
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[7] );
		FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_TOPSCORE_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer( strbuf );
		//
		// トップスコア名前
		fma_disp_fes_string( wk, FMA_MES_TOPSCORE_NAME, FMAI_CANVAS_DOWN_NAME_L, 0, 0, PRINTTOOL_MODE_LEFT );
		// トップスコア
		fma_disp_fes_string( wk, FMA_MES_TOPSCORE, FMAI_CANVAS_DOWN_TOPSCORE_R, 64, 0, PRINTTOOL_MODE_RIGHT );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  差分チェック
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_chk_diff( FMISSION_APP_WORK* wk )
{
	s16		buf[2];
	u16		timer;
	s16		i;
	s16		num;
	s16		score;
	s16		rank;
	BOOL	bflg;
	//
	// 終了時は処理しない
	if(wk->Step == STEP_END){
		return;
	}
	// 情報の更新(毎フレームでいいらしい)
	LINK_FESTIVAL_GetMissionStatus( wk->Param->fest_sys, &wk->MissionStatus  );
	//
	// 時間
	if(wk->OldWork.Timer != 0){
		timer = LINK_FESTIVAL_GetMissionLife( wk->Param->fest_sys );
		if(wk->OldWork.Timer != timer){
			//
			buf[0] = timer/60;
			buf[1] = timer%60;
			// 残り時間
			FmaCmn_disp_time( wk, FMA_CLWKMODE_NORMAL, FMAI_CLWK_TIME_00, buf[0], buf[1] );
			// 10秒以下は色変え
			if(timer < 10){
				for(i=0;i<5;i++){
					FmaCmn_chg_pal( wk, FMA_CLWKMODE_NORMAL, FMAI_CLWK_TIME_00+i, 1 );
				}
				// SE
				PMSND_PlaySE( SND_FMA_COUNT );
			}
			wk->OldWork.Timer = timer;
		}
	}
	//
	// 人数(重要)
	num = wk->MissionStatus.entry_num;
	if(wk->OldWork.Num != num){
		// 1人の場合
		if(num <= 1){
			wk->Flag |= FLG_FMA_ONLY;
			wk->Flag |= FLG_FMA_SCROLL_STOP;
			wk->BgOffX = 0;
			FmaCmn_clear_canvas( wk, FMAI_CANVAS_UP_SCROLL_00 );
			FmaCmn_clear_canvas( wk, FMAI_CANVAS_UP_SCROLL_01 );
		}else{
			wk->Flag &= ~FLG_FMA_ONLY;
			wk->Flag &= ~FLG_FMA_SCROLL_STOP;
		}
		wk->Flag |= FLG_FMA_UPDATE;
		wk->OldWork.Num = num;
	}
	//
	// トータルスコア
	score = wk->MissionStatus.total_score;
	if(wk->OldWork.Score != score){
		wk->Flag |= FLG_FMA_UPDATE;
		wk->OldWork.Score = score;
	}
	//
	// 順位
	rank = wk->MissionStatus.my_score;
	if(wk->OldWork.Rank != rank){
		wk->Flag |= FLG_FMA_UPDATE;
		wk->OldWork.Rank = rank;
	}
	//
	// トップスコア
	bflg = LINK_FESTIVAL_TopScoreIsUpdate( wk->Param->fest_sys );
	if(bflg == TRUE){
		wk->Flag |= FLG_FMA_UPDATE;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  詳細下画面クリア
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_clear_info_down( FMISSION_APP_WORK* wk )
{
	s32		i;
	const s16 *adr = &fmai_info_clear_tbl[0];
	
	// クリア
	for(i=0;i<FMA_DATA_MAX;i++){
		if( adr[i] == -1){
			break;
		}
		// クリア
		FmaCmn_clear_canvas( wk, adr[i] );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  スクロールチェック(要調整)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_chk_scroll( FMISSION_APP_WORK* wk )
{
	s32		flg=0;
	//
	// 終了時は処理しない
	if(wk->Step == STEP_END){
		return;
	}
	// ログのチェック(毎フレーム必要)
	fma_chk_log_buffer( wk );
	
	// 一人モードの場合
	if(wk->Flag & FLG_FMA_ONLY){
		return;
	}
	// 切れのいいときだけ表示
	if((wk->BgOffX != 0)&&(wk->BgOffX != 0x10000)){
		return;
	}
	// 書き換え
	if(wk->BgOffX == 0){		// 0の時=>256側を書き換える
		flg |= 1;
	}
	if(wk->BgOffX == 0x10000){	// 256の時=>0側を書き換える
		flg |= 2;
	}
	// 0側
	if(flg & 2){
		FmaCmn_clear_canvas( wk, FMAI_CANVAS_UP_SCROLL_00 );
		fma_disp_scroll_info( wk, FMAI_CANVAS_UP_SCROLL_00 );
	}
	// 256側
	if(flg & 1){
		FmaCmn_clear_canvas( wk, FMAI_CANVAS_UP_SCROLL_01 );
		fma_disp_scroll_info( wk, FMAI_CANVAS_UP_SCROLL_01 );
	}
}

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  初期化(Result)
 *
 *	@param	FMISSION_APP_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void fma_init_result( FMISSION_APP_WORK* wk )
{
	u8		cbuf[2];
	s32		id;
	s32		lv;
	s32		timer[2];
	s32		buf[2];
	SAVE_CONTROL_WORK *work = GAMEDATA_GetSaveControlWork( wk->Param->gdata );
	FMISSION_SAVE *fsave 	= SAVEDATA_GetFMissionSave( work );
	
	// cmnを非表示
	FmaCmn_set_obj_visible( wk, FMA_CLWKMODE_CMN, FMA_CLWK_CMN_BACK, FALSE );
	//
	// 1人以下の場合
	if( wk->MissionStatus.entry_num <= 1){
		wk->Flag |= FLG_FMA_ONLY;
	}
	// 結果表示の反映
	id = wk->Param->fmwk->beacon.m_id;
	lv = wk->Param->fmwk->beacon.m_level;
	LINK_FESTIVAL_GetMissionResult( wk->Param->fest_sys, &wk->MissionResult, wk->ArcData[id].param.level[lv].time );
	//
	// 成功したかどうか
	if(wk->MissionResult.clear_f == TRUE){
		wk->Flag |= FLG_FMA_SUCCESS;
		// アイテム取得可能か
		if(wk->MissionResult.item_num > 0){
			buf[0] = FmaCmn_chk_itemget( wk, wk->MissionResult.item );
			if(buf[0] > 0){
				wk->Flag |= FLG_FMA_GET;
			}
		}
	}
	// ボーナスの判定
	if(wk->MissionResult.bonus_num > 0){
		wk->Flag |= FLG_FMA_BONUS;
	}
	// ダブルスコアの判定
	// =>単純にsp_itemで判定すること
	if(wk->MissionResult.sp_item > 0){
		wk->Flag |= FLG_FMA_DOUBLE;
		// アイテム取得可能か
		if(wk->MissionResult.sp_item_num > 0){
			buf[0] = FmaCmn_chk_itemget( wk, wk->MissionResult.sp_item );
			if(buf[0] > 0){
				wk->Flag |= FLG_FMA_GET2;
			}
		}
	}
	// クリアタイム(スコア達成型の場合のみ表示)の判定
	if(wk->Param->fmwk->beacon.end_type == FMISSION_ENDTYPE_SCORE){
		wk->Flag |= FLG_FMA_CLEAR_TIME;
	}
	// トータルスコアの判定
	if(wk->MissionResult.new_record_score_f == TRUE){
		wk->Flag |= FLG_FMA_NEW_TS;
	}
	// ハイリンクレベルアップの判定
	if(wk->MissionResult.hilink_level_up > 0){
		wk->Flag |= FLG_FMA_HILINK;
	}
	// マイスコアの判定
	if(wk->MissionResult.new_record_my_score_f == TRUE){
		wk->Flag |= FLG_FMA_NEW_RD;
	}
	//
#ifdef PM_DEBUG
	// debug
	if(wk->Flag & FLG_FMA_DEBUG){
		wk->Flag |= (FLG_FMA_SUCCESS|FLG_FMA_BONUS|FLG_FMA_DOUBLE|FLG_FMA_CLEAR_TIME|FLG_FMA_GET|FLG_FMA_GET2|FLG_FMA_HILINK);
		wk->Flag |= (FLG_FMA_NEW_TS|FLG_FMA_NEW_CT);
	}
#endif

#ifdef PM_DEBUG
	OS_Printf("clear, ver, vernum, score :%d,%d,%d,%d\n",
			wk->MissionResult.clear_f,
			wk->MissionResult.hilink_version,
			wk->MissionResult.hilink_level_up,
			wk->MissionResult.score);
	//
	OS_Printf("item, itemnum, bonusnum, spitem, spitemnum, %d,%d,%d,%d,%d\n",
			wk->MissionResult.item,
			wk->MissionResult.item_num,
			wk->MissionResult.bonus_num,
			wk->MissionResult.sp_item,
			wk->MissionResult.sp_item_num);
#endif
}

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
		// ME待ち
		case STEP_LV1+5:
			// MEを鳴らしている場合
			if( wk->Flag & FLG_FMA_ME_WAIT ){
				bflg = fma_is_snd_me_end( wk );
				if(bflg == FALSE){
					wk->Flag &= ~FLG_FMA_ME_WAIT;
					wk->Flag |= FLG_FMA_ME_MES;		// 矢印表示
					wk->Step = STEP_LV1+10;
				}
			}
			break;
		//
		// stream待ち(1発表示)
		case STEP_LV1+10:
			// メッセージ終了
			if(wk->Flag & FLG_FMA_MSG_END){
				wk->Step = STEP_LV1;
				wk->Flag |= FLG_FMA_ME_MES;			// 矢印表示
			}else{
				// MEのメッセージ=>矢印を表示する(毎フレーム)
				if(wk->Flag & FLG_FMA_ME_MES){
					FmaCmn_disp_arrow( wk, TRUE );
				}
				// キー待ち
				if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ){
					wk->Flag &= ~FLG_FMA_ME_MES;
					fma_chk_meswin( wk );
					wk->Step = STEP_LV2;
					// SE
					PMSND_PlaySE( SND_FMA_SKIP );
				}
			}
			break;
		//
		// stream待ち
		case STEP_LV2:
			bflg = FmaCmn_main_meswin( wk );
			if(bflg == FALSE){
				// 入力待ち
				if(wk->Flag & FLG_FMA_MSG_END){
					// MEを鳴らしている場合
					if( wk->Flag & FLG_FMA_ME_WAIT ){
						bflg = fma_is_snd_me_end( wk );
						if(bflg == FALSE){
							wk->Flag &= ~FLG_FMA_ME_WAIT;
							wk->Flag |= FLG_FMA_ME_MES;
							wk->Step = STEP_LV1;
						}
					}else{
						wk->Flag |= FLG_FMA_ME_MES;
						wk->Step = STEP_LV1;
					}
				}else{
					// MEを鳴らしている場合
					if( wk->Flag & FLG_FMA_ME_WAIT ){
						bflg = fma_is_snd_me_end( wk );
						if(bflg == FALSE){
							wk->Flag &= ~FLG_FMA_ME_WAIT;
							wk->Flag |= FLG_FMA_ME_MES;
						}
					}else{
						// MEのメッセージ=>矢印を表示する(毎フレーム)
						if(wk->Flag & FLG_FMA_ME_MES){
							FmaCmn_disp_arrow( wk, TRUE );
						}
						if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ){
							wk->Flag &= ~FLG_FMA_ME_MES;
							// 次のメッセージがあるかの判定
							fma_chk_meswin( wk );
							// SE
							PMSND_PlaySE( SND_FMA_SKIP );
						}
					}
				}
			}
			break;
		//
		// END
		case STEP_END:
			break;
		//
		default:
			break;
	}
	// タイマ
	fma_chk_level_timer( wk );
	
	// debug
	fma_debug( wk );
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
	// 戻るアニメなしなのでそのままとしておく
	if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ){
		// タイマ停止までは入力不可
		if(wk->Flag & FLG_FMA_MSG_END){
			wk->Step = STEP_END;
			// SE
			PMSND_PlaySE( SND_FMA_SKIP );
		}
	}
	//
	// 更新
	if(wk->Flag & FLG_FMA_UPDATE){
		fma_update_up_result( wk );
		fma_update_down_result( wk );
		wk->Flag &= ~FLG_FMA_UPDATE;
		wk->Flag &= ~(FLG_FMA_LEVEL0|FLG_FMA_LEVEL1|FLG_FMA_LEVEL2|FLG_FMA_LEVEL3|FLG_FMA_LEVEL4);
	}
	//
	//
	// メッセージ終了で矢印表示=>最後の矢印待ち
	if(wk->Flag & FLG_FMA_MSG_END){
		// MEのメッセージ=>矢印を表示する
		if(wk->Flag & FLG_FMA_ME_MES){
			FmaCmn_disp_arrow( wk, TRUE );
		}
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
	s32		flg=0;
	s32		timer;
	s32		id;
	s32		buf[2];
	STRBUF	*strbuf;
	
	//----------------------------------------
	// 段階で表示していくので項目ごとに並べる
	//----------------------------------------
	//-----------------------
	// LV0の場合(固定のもの)
	//------------------------
	if(wk->Flag & FLG_FMA_LEVEL0){
		// タイトル
		fma_disp_fes_string( wk, FMA_MES_TITLE, FMAR_CANVAS_UP_TITLE, 0, 4, PRINTTOOL_MODE_LEFT );
		// 星(obj)
		FmaCmn_disp_star( wk, FMAR_CLWK_STAR );
		//
		// 参加人数(左)
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[1] );
		FmaCmn_disp_string( wk, FMAR_CANVAS_UP_JOIN_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer( strbuf );
		// 人アイコン
		FmaCmn_set_obj_visible( wk, FMA_CLWKMODE_NORMAL, FMAR_CLWK_ICON_01, TRUE );
		//
		// クリア目標
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[2] );
		FmaCmn_disp_string( wk, FMAR_CANVAS_UP_CLEAR_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer( strbuf );
		//
		// トータルスコア(左)
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[3] );
		FmaCmn_disp_string( wk, FMAR_CANVAS_UP_SCORE_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer( strbuf );
		//
		// クリアタイム
		if(wk->Flag & FLG_FMA_CLEAR_TIME){
			// クリアタイム(左)
			strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[9] );
			FmaCmn_disp_string( wk, FMAR_CANVAS_UP_TIME_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
			GFL_STR_DeleteBuffer( strbuf );
		}
	}
	//-----------
	// LV1の場合
	//-----------
	if(wk->Flag & FLG_FMA_LEVEL1){
		//----------
		// 1段階目
		//----------
		// 参加人数(右)
		fma_disp_fes_string( wk, FMA_MES_JOIN, FMAR_CANVAS_UP_JOIN_R, 48, 0, PRINTTOOL_MODE_RIGHT );
		// SE
		PMSND_PlaySE( SND_FMA_RESULT );
	}
	//-----------
	// LV2の場合
	//-----------
	if(wk->Flag & FLG_FMA_LEVEL2){
		//----------
		// 2段階目
		//----------
		// クリア目標(右)
		fma_disp_fes_string( wk, FMA_MES_CLEAR, FMAR_CANVAS_UP_CLEAR_R, 48, 0, PRINTTOOL_MODE_RIGHT );
		// SE
		PMSND_PlaySE( SND_FMA_RESULT );
	}
	//-----------
	// LV3の場合
	//-----------
	if(wk->Flag & FLG_FMA_LEVEL3){
		//------------
		// 3段階目
		//------------
		// トータルスコア(右)
		fma_disp_total_score( wk, 1 );
		//
		// トータル(単位)
		id = wk->Param->fmwk->beacon.m_id;
		buf[0] = wk->ArcData[id].param.unit;
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_01_tbl[4*buf[0]+1] );
		FmaCmn_disp_string( wk, FMAR_CANVAS_UP_SCORE_R, 48, 0, strbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer( strbuf );
		//
		// クリアタイム
		if(wk->Flag & FLG_FMA_CLEAR_TIME){
			// クリアタイム(右)
			timer = LINK_FESTIVAL_GetMissionLife( wk->Param->fest_sys );
			buf[0] = timer/60;		// 分
			buf[1] = timer%60;		// 秒
			FmaCmn_disp_time( wk, FMA_CLWKMODE_NORMAL, FMAR_CLWK_TIME_00, buf[0], buf[1] );
		}
		//
		// トータルスコア(NEW)
		if(wk->Flag & FLG_FMA_NEW_TS){
			FmaCmn_set_obj_visible( wk, FMA_CLWKMODE_NORMAL, FMAR_CLWK_RECORD_00, TRUE );
			flg = 1;
		}
		// SE
		if(flg == 0){		// 通常
			PMSND_PlaySE( SND_FMA_RESULT );
		}else{				// ハイスコア
			PMSND_PlaySE( SND_FMA_HISCORE );
		}
	}
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
	
	//----------------------------------------
	// 段階で表示していくので項目ごとに並べる
	// =>仕様変更により一括で表示
	//----------------------------------------
	
	//----------
	// LV4の場合
	//----------
	if(wk->Flag & FLG_FMA_LEVEL4){
		// 仕様変更によりマイスコア、順位は1人でも表示
		// マイスコア(左)
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[5] );
		FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_MYSCORE_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer( strbuf );
		//
		// マイスコア(右)
		fma_disp_fes_string( wk, FMA_MES_MYSCORE, FMAR_CANVAS_DOWN_MYSCORE_R, 64, 0, PRINTTOOL_MODE_RIGHT );
		//
		// マイスコア(NEWRECORD)
		if(wk->Flag & FLG_FMA_NEW_RD){
			FmaCmn_set_obj_visible( wk, FMA_CLWKMODE_RD, FMA_CLWK_RESULT_RECORD, TRUE );
		}
		//
		// 順位(左)
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[6] );
		FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_RANK_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer( strbuf );
		//
		// 順位(右)
		fma_disp_fes_string( wk, FMA_MES_RANK, FMAR_CANVAS_DOWN_RANK_R, 152, 0, PRINTTOOL_MODE_RIGHT );
		
		// 一人の時は表示しない
		if(!(wk->Flag & FLG_FMA_ONLY)){
			// トップスコア(左)
			strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[7] );
			FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_TOPSCORE_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
			GFL_STR_DeleteBuffer( strbuf );
			//
			// トップスコア(中)
			fma_disp_fes_string( wk, FMA_MES_TOPSCORE_NAME, FMAR_CANVAS_DOWN_NAME_L, 0, 0, PRINTTOOL_MODE_LEFT );
			//
			// トップスコア(右)
			fma_disp_fes_string( wk, FMA_MES_TOPSCORE, FMAR_CANVAS_DOWN_TOPSCORE_R, 64, 0, PRINTTOOL_MODE_RIGHT );
		}
		//
		// 成功時のみ表示
		if(wk->Flag & FLG_FMA_SUCCESS){
			// 報酬(左)
			strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[4] );
			FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_GAIN_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
			GFL_STR_DeleteBuffer( strbuf );
			//
			// 報酬(右)
			fma_disp_fes_string( wk, FMA_MES_GAIN_RESULT, FMAR_CANVAS_DOWN_GAIN_R, 128, 0, PRINTTOOL_MODE_RIGHT );
		}
		//
		// ボーナス時のみ表示
		if(wk->Flag & FLG_FMA_BONUS){
			// スコアボーナス(左)
			strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[10] );
			FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_BONUS_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
			GFL_STR_DeleteBuffer( strbuf );
			//
			// スコアボーナス(右)
			fma_disp_fes_string( wk, FMA_MES_SCORE_BONUS, FMAR_CANVAS_DOWN_BONUS_R, 48, 0, PRINTTOOL_MODE_RIGHT );
		}
		//
		// ダブルスコア時のみ表示
		if(wk->Flag & FLG_FMA_DOUBLE){
			// ダブルスコア(左)
			strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[11] );
			FmaCmn_disp_string( wk, FMAR_CANVAS_DOWN_DOUBLE_L, 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
			GFL_STR_DeleteBuffer( strbuf );
			//
			// ダブルスコア(右)
			fma_disp_fes_string( wk, FMA_MES_DOUBLE, FMAR_CANVAS_DOWN_DOUBLE_R, 128, 0, PRINTTOOL_MODE_RIGHT );
		}
	}
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
		buf[1] = wk->ArcData[id].param.unit;
		WORDSET_RegisterNumber( wk->Wordset, 0, buf[0], 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
		str = GFL_STR_CreateBuffer( 64, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_01_tbl[4*buf[1]+1] );
		WORDSET_ExpandStr( wk->Wordset, str, str_tmp );
		GFL_STR_DeleteBuffer(str_tmp);
		//
		OS_Printf("%d,%d\n",id,buf[1]);
	}
#if 0
	else if(mode == FMA_MES_GAIN){		// 報酬
		// 仕様変更により非表示となった
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
#endif
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
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[15] );
		WORDSET_ExpandStr( wk->Wordset, str, str_tmp );
		GFL_STR_DeleteBuffer(str_tmp);
	}
	else if(mode == FMA_MES_MYSCORE){	// 自分のスコア
		// 自分のスコア
		buf[0] = wk->MissionStatus.my_score;
		// 単位
		buf[1] = wk->ArcData[id].param.unit;
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
		buf[1] = wk->ArcData[id].param.unit;
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
	else if(mode == FMA_MES_TOTAL_SCORE){	// トータルスコア
		// 単位
		buf[0] = wk->ArcData[id].param.unit;
		str = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_01_tbl[4*buf[0]+1] );
	}
	else if(mode == FMA_MES_GAIN_RESULT){		// 報酬(結果)
		// 報酬アイテム取得
		buf[0] = wk->MissionResult.item;
		// 報酬数取得
		buf[1] = wk->MissionResult.item_num;
		//
		WORDSET_RegisterItemName( wk->Wordset, 0, buf[0] );
		WORDSET_RegisterNumber(   wk->Wordset, 1, buf[1], 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		str = GFL_STR_CreateBuffer( 128, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_02_tbl[0] );
		WORDSET_ExpandStr( wk->Wordset, str, str_tmp );
		GFL_STR_DeleteBuffer(str_tmp);
	}
	else if(mode == FMA_MES_SCORE_BONUS){		// スコアボーナス
		// スコアボーナス
		buf[0] = wk->MissionResult.bonus_num;
		WORDSET_RegisterNumber( wk->Wordset, 0, buf[0], 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		str = GFL_STR_CreateBuffer( 128, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[13] );
		WORDSET_ExpandStr( wk->Wordset, str, str_tmp );
		GFL_STR_DeleteBuffer(str_tmp);
	}
	else if(mode == FMA_MES_DOUBLE){		// ダブルスコア
		// ボーナスアイテム
		buf[0] = wk->MissionResult.sp_item;
		// ボーナスアイテム数
		buf[1] = wk->MissionResult.sp_item_num;
		//
		WORDSET_RegisterItemName( wk->Wordset, 0, buf[0] );
		WORDSET_RegisterNumber(   wk->Wordset, 1, buf[1], 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		str = GFL_STR_CreateBuffer( 128, wk->heapID );
		str_tmp = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[14] );
		WORDSET_ExpandStr( wk->Wordset, str, str_tmp );
		GFL_STR_DeleteBuffer(str_tmp);
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
#if 0
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
	// 仕様変更によりスコアを表示
	flg = 0;
	num = FMISSION_SAVE_GetMissionStatusHighScore( fsave, id );
	if(num == 0){
		dflg = 1;
	}
	//
	// 一旦クリア
	FmaCmn_clear_canvas( wk, FMAI_CANVAS_DOWN_HIGH_L );
	FmaCmn_clear_canvas( wk, FMAI_CANVAS_DOWN_HIGH_R );
	//
	if(dflg == 0){
		// ハイスコア
		str = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_tbl[8] );
		FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_HIGH_L, 0, 0, str, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer( str );
		//
		WORDSET_RegisterNumber(wk->Wordset, 0, num, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
		buf[1] = wk->ArcData[id].param.unit;
		str = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_01_tbl[4*buf[1]+1] );
		//
		expbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
		WORDSET_ExpandStr( wk->Wordset, expbuf, str );
		// 表示
		FmaCmn_disp_string( wk, FMAI_CANVAS_DOWN_HIGH_R, 64, 0, expbuf, PRINTTOOL_MODE_RIGHT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer(str);
		GFL_STR_DeleteBuffer(expbuf);
	}
#endif
}

//--------------------------------------------------------------
/**
 * トータル表示(要整理)
 *
 * @param   FMISSION_APP_WORK		*wk
 */
//--------------------------------------------------------------
static void fma_disp_total_score( FMISSION_APP_WORK *wk, const s32 mode )
{
	s32		id;
	s32		buf[2];
	u16		len;
	s16		sbuf[2];
	STRBUF	*str;
	
	// 単位
	id = wk->Param->fmwk->beacon.m_id;
	buf[0] = wk->ArcData[id].param.unit;
	str = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_01_tbl[4*buf[0]+1] );
	// 単位の長さを求める(基準を24dotの「ひき」としてobjを配置)
	len = PRINTSYS_GetStrWidth( str, wk->font, 0 );
	// オフセットを求める
	buf[1] = 24-len;
	//
//	OS_Printf("len :%d\n",buf[1]);
	GFL_STR_DeleteBuffer( str );
	//
	if(mode == 0){		// 詳細
		sbuf[0] = FmaCmn_get_empty( wk, FMA_EMPTY_INFO*2 );
		sbuf[1] = FmaCmn_get_empty( wk, FMA_EMPTY_INFO*2+1 );
		//
		buf[0] = wk->MissionStatus.total_score;
		FmaCmn_disp_number( wk, FMA_CLWKMODE_NORMAL, FMAI_CLWK_SCORE_00, buf[0], 4, 0, sbuf[0]+buf[1], sbuf[1] );
	}else{				// 結果
		sbuf[0] = FmaCmn_get_empty( wk, FMA_EMPTY_RESULT*2 );
		sbuf[1] = FmaCmn_get_empty( wk, FMA_EMPTY_RESULT*2+1 );
		//
		buf[0] = wk->MissionResult.score;
		FmaCmn_disp_number( wk, FMA_CLWKMODE_NORMAL, FMAR_CLWK_SCORE_00, buf[0], 4, 0, sbuf[0]+buf[1], sbuf[1] );
	}
}

//--------------------------------------------------------------
/**
 * スクロール文字セット
 *
 * @param   FMISSION_APP_WORK		*wk
 * @param   const s32				(番号)
 */
//--------------------------------------------------------------
static void fma_disp_scroll_info( FMISSION_APP_WORK *wk, const s32 num )
{
	s32			buf[4];
	s32			i;
	s32			j;
	s32			id;
	STRBUF		*strbuf;
	STRBUF		*expbuf;
	FMISSION_LOG_TBL* tbl;
	BOOL		bflg;
	
	id = wk->Param->fmwk->beacon.m_id;
	buf[0] = wk->ArcData[id].param.unit;
	
	// データがなにもない
	if(wk->LatestLocal.num == 0){
		return;
	}
	//
	// まだ表示してない => 0番を表示
	if(wk->ScoreLog.strbuf == 0){
		// ログの情報をコピー
		wk->ScoreLog.strbuf = GFL_STR_CreateCopyBuffer( wk->LatestLocal.log[0].name, wk->heapID );
		wk->ScoreLog.score  = wk->LatestLocal.log[0].score;
		wk->ScoreLog.tr_id  = wk->LatestLocal.log[0].tr_id;
	}
	else{
		// (すでに表示した)同じIDを探す
		j=-1;
		for(i=0;i<wk->LatestLocal.num;i++){
			if(wk->LatestLocal.log[i].tr_id == wk->ScoreLog.tr_id){
				j = i;
				break;
			}
		}
		// ここまでで何番目(j)に名前の番号が入る
		// (該当なし(-1)もありうる)
		i = j+1;
		if(i >= wk->LatestLocal.num){
			i = 0;
		}
		// iに求める番号が入っているはず
		// ログの情報をコピー
		GFL_STR_DeleteBuffer( wk->ScoreLog.strbuf );
		wk->ScoreLog.strbuf = GFL_STR_CreateCopyBuffer( wk->LatestLocal.log[i].name, wk->heapID );
		wk->ScoreLog.score  = wk->LatestLocal.log[i].score;
		wk->ScoreLog.tr_id  = wk->LatestLocal.log[i].tr_id;
	}
	//
	// (00 or 01)
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], fmai_mes_01_tbl[4*buf[0]+2] );
	WORDSET_RegisterWord( wk->Wordset,  0, wk->ScoreLog.strbuf, 0, TRUE, PM_LANG );
	WORDSET_RegisterNumber(wk->Wordset, 1, wk->ScoreLog.score,  4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
	expbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
	WORDSET_ExpandStr( wk->Wordset, expbuf, strbuf );
	FmaCmn_disp_string( wk, num, 0, 0, expbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
	GFL_STR_DeleteBuffer( strbuf );
	GFL_STR_DeleteBuffer( expbuf );
}

//--------------------------------------------------------------
/**
 * スコアログのチェック(毎フレーム必要)
 *
 * @param   FMISSION_APP_WORK		*wk
 */
//--------------------------------------------------------------
static void fma_chk_log_buffer( FMISSION_APP_WORK *wk )
{
	s32		i;
	s32		j;
	FMISSION_LOG_TBL* tbl;
	
	// 最新の3人分の情報を取得
	tbl = LINK_FESTIVAL_GetMissionLogStackTbl( wk->Param->fest_sys );
	FMISSION_LOG_StackTbl_GetLogLatest( tbl, wk->Latest );
	//
	// チェック
	for(i=0;i<FMISSION_LOG_LATEST_NUM_MAX;i++){
		fma_chk_latest( wk, i );
	}
	//
	j=0;
	for(i=0;i<FMISSION_LOG_LATEST_NUM_MAX;i++){
		if(wk->LatestLocal.log[i].tr_id != 0){
			j++;
		}
	}
	wk->LatestLocal.num = j;
}

//--------------------------------------------------------------
/**
 * スコアログの最新バッファがローカルバッファ同じかチェックし、
 * 適切な場所にセット
 *
 * @param   FMISSION_APP_WORK		*wk
 * @param   const s32				スコアログのログ番号(0-2)
 */
//--------------------------------------------------------------
static void fma_chk_latest( FMISSION_APP_WORK *wk, const s32 num )
{
	s32		j;
	s32		i;
	s32		flg=0;
	BOOL	bflg;
	
	// idが0は除く
	if(wk->Latest->log[num].tr_id == 0){
		return;
	}
	
	// tr_idと名前が同じかをチェック
	j=-1;
	for(i=0;i<FMISSION_LOG_LATEST_NUM_MAX;i++){
		// idが同じ
		if(wk->Latest->log[num].tr_id == wk->LatestLocal.log[i].tr_id){
			// 名前が同じ
			bflg = GFL_STR_CompareBuffer( wk->Latest->log[num].name, wk->LatestLocal.log[i].name );
			if(bflg == TRUE){
				j=i;
				break;
			}
		}
	}
	// ここまでで(j)番目のデータと同じかがわかる
	// 該当なし(-1)もわかる
	if(j == 0){			// 0番目の情報を置き換える
		wk->LatestLocal.log[j].tr_id = wk->Latest->log[num].tr_id;
		wk->LatestLocal.log[j].score = wk->Latest->log[num].score;
		if(wk->LatestLocal.log[j].name){
			GFL_STR_DeleteBuffer( wk->LatestLocal.log[j].name );
		}
		wk->LatestLocal.log[j].name = GFL_STR_CreateCopyBuffer( wk->Latest->log[num].name, wk->heapID );
	}
	else if(j == 1){	// 0番目の情報を1に写し、0番目に情報をセット
		// 0番を1番にセット
		fma_set_latest( wk, 0, 1 );
		//
		// 0番に情報をセット
		wk->LatestLocal.log[0].tr_id = wk->Latest->log[num].tr_id;
		wk->LatestLocal.log[0].score = wk->Latest->log[num].score;
		if(wk->LatestLocal.log[0].name){
			GFL_STR_DeleteBuffer( wk->LatestLocal.log[0].name );
		}
		wk->LatestLocal.log[0].name = GFL_STR_CreateCopyBuffer( wk->Latest->log[num].name, wk->heapID );
	}
	else if((j == 2)||(j == -1)){	// 1->2へ、0->1にして、0番目に情報をセット
		// 1番を2番にセット
		if(wk->LatestLocal.log[1].tr_id != 0){
			fma_set_latest( wk, 1, 2 );
		}
		// 0番を1番にセット
		if(wk->LatestLocal.log[0].tr_id != 0){
			fma_set_latest( wk, 0, 1 );
		}
		//
		// 0番に情報をセット
		wk->LatestLocal.log[0].tr_id = wk->Latest->log[num].tr_id;
		wk->LatestLocal.log[0].score = wk->Latest->log[num].score;
		if(wk->LatestLocal.log[0].name){
			GFL_STR_DeleteBuffer( wk->LatestLocal.log[0].name );
		}
		wk->LatestLocal.log[0].name = GFL_STR_CreateCopyBuffer( wk->Latest->log[num].name, wk->heapID );
	}
#if 0
	//
	OS_Printf("buffer : ");
	for(i=0;i<3;i++){
		OS_Printf("(%d,%d) ",wk->Latest->log[i].tr_id,wk->Latest->log[i].score);
	}
	OS_Printf("\n");
#endif
}

//--------------------------------------------------------------
/**
 * スコアログの情報の移動
 *
 * @param   FMISSION_APP_WORK		*wk
 * @param   const s32				元
 * @param   const s32				先
 */
//--------------------------------------------------------------
static void fma_set_latest( FMISSION_APP_WORK *wk, const s32 src, const s32 dest )
{
	// クリア
	if(src == -1){
		wk->LatestLocal.log[dest].tr_id = 0;
		wk->LatestLocal.log[dest].score = 0;
		if(wk->LatestLocal.log[dest].name){
			GFL_STR_DeleteBuffer( wk->LatestLocal.log[dest].name );
		}
	}else{
		wk->LatestLocal.log[dest].tr_id = wk->LatestLocal.log[src].tr_id;
		wk->LatestLocal.log[dest].score = wk->LatestLocal.log[src].score;
		if(wk->LatestLocal.log[dest].name){
			GFL_STR_DeleteBuffer( wk->LatestLocal.log[dest].name );
		}
		wk->LatestLocal.log[dest].name = GFL_STR_CreateCopyBuffer( wk->LatestLocal.log[src].name, wk->heapID );
	}
}

//--------------------------------------------------------------
/**
 * 固定文字列表示(1回だけ表示)
 *
 * @param   FMISSION_APP_WORK		*wk
 * @param   const s32				番号(未使用)
 */
//--------------------------------------------------------------
static void fma_disp_fix_string( FMISSION_APP_WORK *wk, const s32 mode )
{
	STRBUF	*strbuf;
	s32		i;
	const s16 *adr = &fmai_fix_mes_00_tbl[0];
	//
	for(i=0;i<FMA_DATA_MAX;i++){
		if( adr[2*i] == -1){
			break;
		}
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], adr[2*i] );
		FmaCmn_disp_string( wk, adr[2*i+1], 0, 0, strbuf, PRINTTOOL_MODE_LEFT, PRINTSYS_LSB_Make(15,2,0));
		GFL_STR_DeleteBuffer( strbuf );
	}
}

//--------------------------------------------------------------
/**
 * 段階表示用
 *
 * @param   FMISSION_APP_WORK		*wk
 */
//--------------------------------------------------------------
static void fma_chk_level_timer( FMISSION_APP_WORK *wk )
{
	// 終了時は処理しない
	if(wk->Step == STEP_END){
		return;
	}
	// 念のため
	if(wk->Flag & FLG_FMA_TIME_STOP){
		return;
	}
	// 終了判定
	wk->LevelTimer++;
	if(wk->LevelTimer >= fmar_wait_tbl[wk->Level]){
		wk->Level++;
		if(fmar_wait_tbl[wk->Level] == -1){
			wk->Flag |= FLG_FMA_TIME_STOP;
			// メッセージウインドウ表示
			fma_chk_meswin( wk );
			wk->Step = STEP_LV1+5;
			//
		}else{
			// FLG_FMA_LEVEL*と対応
			wk->Flag |= (1<<(16+(wk->Level-1)));
			wk->Flag |= FLG_FMA_UPDATE;
		}
	}
}

//--------------------------------------------------------------
/**
 * メッセージ表示(win)
 *
 * @param   FMISSION_APP_WORK		*wk
 *
 * @ret     BOOL					(TRUE:表示、FALSE:表示しない)
 */
//--------------------------------------------------------------
static BOOL fma_chk_meswin( FMISSION_APP_WORK *wk )
{
	BOOL	bflg=FALSE;
	s32		id;
	
	// 成功失敗(必ず)
	if(!(wk->Flag & FLG_FMA_MSG_00)){
		// 判定
		if(wk->Flag & FLG_FMA_SUCCESS){
			id = FMA_MSGWIN_00;
			// ME再生
			fma_snd_me( wk, 1 );
		}else{
			id = FMA_MSGWIN_01;
			// ME再生
			fma_snd_me( wk, 2 );
			// 終了(失敗の場合は終了)
			wk->Flag |= FLG_FMA_MSG_END;
		}
		// 一発だし
		FmaCmn_disp_winmes( wk, FMAR_CANVAS_DOWN_WIN, id, 0 );
		wk->Flag |= FLG_FMA_MSG_00;
		return( TRUE );
	}
	//
	// 報酬を手に入れたか
	if(!(wk->Flag & FLG_FMA_MSG_01)){
		// 判定
		if(wk->Flag & FLG_FMA_GET){
			id = FMA_MSGWIN_02;
			// ME再生
			fma_snd_me( wk, 0 );
		}else{
			id = FMA_MSGWIN_03;
			// 報酬手に入らず=>矢印表示
			wk->Flag |= FLG_FMA_ME_MES;
		}
		// ダブルスコアの有無
		if(!(wk->Flag & FLG_FMA_DOUBLE)){
			// ハイリンクの有無
			if(!(wk->Flag & FLG_FMA_HILINK)){
				// どちらもない => ここで終わり
				wk->Flag |= FLG_FMA_MSG_END;
			}
		}
		// stream
		FmaCmn_disp_winmes( wk, FMAR_CANVAS_DOWN_WIN, id, 1 );
		wk->Flag |= FLG_FMA_MSG_01;
		return( TRUE );
	}
	// ダブルスコア
	if(!(wk->Flag & FLG_FMA_MSG_02)){
		// 判定
		if(wk->Flag & FLG_FMA_DOUBLE){
			id = FMA_MSGWIN_04;
			// stream
			FmaCmn_disp_winmes( wk, FMAR_CANVAS_DOWN_WIN, id, 1 );
			wk->Flag |= FLG_FMA_MSG_02;
			// 矢印表示
			wk->Flag |= FLG_FMA_ME_MES;
			return( TRUE );
		}
		else{
			// なにもしないで次の判定へ
			wk->Flag |= FLG_FMA_MSG_02;
		}
	}
	// 獲得2(ダブルスコア)
	if(!(wk->Flag & FLG_FMA_MSG_03)){
		// 判定
		if(wk->Flag & FLG_FMA_DOUBLE){
			if(wk->Flag & FLG_FMA_GET2){
				id = FMA_MSGWIN_02A;
				// ME再生
				fma_snd_me( wk, 0 );
			}else{
				id = FMA_MSGWIN_03A;
				// 手に入らず
				wk->Flag |= FLG_FMA_ME_MES;
			}
			// ハイリンクの有無
			if(!(wk->Flag & FLG_FMA_HILINK)){
				// ない場合=>ここで終わり
				wk->Flag |= FLG_FMA_MSG_END;
			}
			// stream
			FmaCmn_disp_winmes( wk, FMAR_CANVAS_DOWN_WIN, id, 1 );
			wk->Flag |= FLG_FMA_MSG_03;
			return( TRUE );
		}
		else{
			// なにもしないで次の判定へ
			wk->Flag |= FLG_FMA_MSG_03;
		}
	}
	// ハイリンク
	if(!(wk->Flag & FLG_FMA_MSG_04)){
		// 判定
		if(wk->Flag & FLG_FMA_HILINK){
			if(wk->Flag & FLG_FMA_ONLY){
				id = FMA_MSGWIN_05;
			}else{
				id = FMA_MSGWIN_06;
			}
			// stream
			FmaCmn_disp_winmes( wk, FMAR_CANVAS_DOWN_WIN, id, 1 );
			wk->Flag |= FLG_FMA_MSG_04;
			wk->Flag |= FLG_FMA_MSG_END;
			return( TRUE );
		}
		else{
			// なにもしないで次の判定へ
			wk->Flag |= FLG_FMA_MSG_04;
			wk->Flag |= FLG_FMA_MSG_END;
		}
	}
	// 終了
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * ME再生
 *
 * @param   FMISSION_APP_WORK		*wk
 * @param   const s32				(0:アイテム、1:クリア、2:失敗)
 */
//--------------------------------------------------------------
static void fma_snd_me( FMISSION_APP_WORK *wk, const s32 mode )
{
	PMSND_PauseBGM( TRUE );
	PMSND_PushBGM();
	PMSND_PlayBGM( fmai_me_tbl[mode] );
	//
	wk->Flag |= FLG_FMA_ME_WAIT;
}

//--------------------------------------------------------------
/**
 * ME再生かどうか
 *
 * @param   FMISSION_APP_WORK		*wk
 *
 * @ret		BOOL					(TRUE:再生中、FALSE:終了)
 */
//--------------------------------------------------------------
static BOOL fma_is_snd_me_end( FMISSION_APP_WORK *wk )
{
	BOOL	bflg=TRUE;
	
	if(PMSND_CheckPlayBGM() == FALSE){
		PMSND_PopBGM();
		PMSND_PauseBGM( FALSE );
		bflg = FALSE;
	}
	return( bflg );
}

//--------------------------------------------------------------
/**
 * debug
 *
 * @param   FMISSION_APP_WORK		*wk
 */
//--------------------------------------------------------------
static void fma_debug( FMISSION_APP_WORK *wk )
{
//#ifdef PM_DEBUG
#if 0
	// debug
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
		if(wk->Flag & FLG_FMA_ONLY){
			wk->Flag &= ~FLG_FMA_ONLY;
			wk->Flag &= ~FLG_FMA_SCROLL_STOP;
		}else{
			wk->Flag |= FLG_FMA_ONLY;
			wk->Flag |= FLG_FMA_SCROLL_STOP;
			wk->BgOffX = 0;
			FmaCmn_clear_canvas( wk, FMAI_CANVAS_UP_SCROLL_00 );
			FmaCmn_clear_canvas( wk, FMAI_CANVAS_UP_SCROLL_01 );
		}
		wk->Flag |= FLG_FMA_UPDATE;
	}
#endif	// PM_DEBUG
}

