//============================================================================================
/**
 * @file		b_scenario_main.c
 * @brief		ポケウッド台本画面
 * @author
 * @date		05.02.10
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "system/wipe.h"
#include "sound/pm_sndsys.h"
#include "font/font.naix"
#include "item/itemuse_def.h"

#include "../btlv/btlv_effect.h"

#include "b_app_tool.h"
//
#include "b_scenario.h"
#include "b_scenario_main.h"

#include "b_scenario_sub.h"

#include "b_scenario_snd_def.h"

#include "b_scenario_gra.naix"
#include "msg/msg_pw_movietitle.h"
#include "msg/msg_pw_hint001.h"

//============================================================================================
//  定数定義
//============================================================================================
//
// 遷移(Step,SubMode)
enum{
	STEP_INIT		= 0,
	STEP_LV1		= 100,
	STEP_LV2		= 200,
	STEP_LV3		= 300,
	STEP_LV4		= 400,
	//
	STEP_END		= 10000,
};

//
enum{
	SMODE_INIT			= -2,		// 初期値
	SMODE_END 			= -1,		// 終了
	//
	SMODE_INIT_TOP		= 0,		// トップ
	SMODE_MAIN_TOP,					// トップ
	SMODE_EXIT_TOP,					// トップ
	SMODE_INIT_SCENARIO,			// シナリオ
	SMODE_MAIN_SCENARIO,			// シナリオ
	SMODE_EXIT_SCENARIO,			// シナリオ
	SMODE_INIT_INFO,				// 情報
	SMODE_MAIN_INFO,				// 情報
	SMODE_EXIT_INFO,				// 情報
	//
	SMODE_MAX,						// 最大
};

// 
enum{
	STR_INIT			= 0,		// 初期化
	STR_UPDATE,						// 更新
};

// 固定文字列
enum{
	MES_ID_TOP_01		= 0,		// タイトル名
	MES_ID_TOP_02,					// あらすじ
	MES_ID_TOP_03,					// クリア目標
	MES_ID_ALL_01,					// 脚本
	MES_ID_ALL_02,					// 共演者
	MES_ID_INFO_01,					// キャスト
	MES_ID_INFO_02,					// もちもの
	MES_ID_INFO_03,					// とくせい
	//
	MES_ID_SCENARIO,				// シナリオTOP
};

// いろいろ
enum{
	MES_END_ID 			= pw_index_00,			// 終了番号([なし]の記述番号)
	MES_HINT_MAX		= 20,
};

//------------
// フラグ
//------------
// BSCENARIO.Flag
enum{
	FLG_SET_INIT		= (1<<0),	// 初期化済み
	FLG_CHG_FONT		= (1<<1),	// フォント切り替え
	FLG_CHG_TITLE		= (1<<2),	// タイトル切り替え
	FLG_UPDATE			= (1<<3),	// 表示更新(矢印)
	//
	FLG_WAIT_START		= (1<<8),	// 開始
	FLG_WAIT_END		= (1<<9),	// 終了待ち終了
	FLG_KEY_INPUT		= (1<<10),	// キー入力選択があった
	FLG_KEY_INPUT_END	= (1<<11),	// キー入力選択で終了
	//
	FLG_LEFT_GRAY		= (1<<16),	// 左灰色
	FLG_RIGHT_GRAY		= (1<<17),	// 右灰色
	FLG_LEFT_UPDATE		= (1<<18),	// 左更新
	FLG_RIGHT_UPDATE	= (1<<19),	// 右更新
	//
	FLG_LEFT_LIMIT		= (1<<20),	// 左リミット
	FLG_RIGHT_LIMIT		= (1<<21),	// 右リミット
};

//============================================================================================
//  データ
//============================================================================================
//-------------
// fixメッセージ
//--------------
static const s16 bs_fix_mestbl[] = {
	pw_topindex_01,			// TOP:TITLE映画の名前
	pw_topindex_02,			// TOP:PLOTあらすじ
	pw_topindex_03,			// TOP:GOALクリア目標
	pw_topbutton_01,		// ALL:脚本
	pw_topbutton_02,		// ALL:共演者
	pw_castindex_03,		// INFO:キャスト共演者
	pw_castindex_04,		// INFO:もちもの
	pw_castindex_05,		// INFO:とくせい
	//
	pw_scriptindex_00,		// タイトル(mesdagaは違い)
};

//--------------
// TOPメッセージ
//--------------
static const s16 bs_mes_tbl[] = {
	pw_title_00,			// タイトル
	pw_plot_000,			// プロット
	pw_goal_00,				// ゴール
};

//-------------------
// ヒント関連メッセージ
//-------------------
static const s16 bs_scene_tbl[] = {
	pw_scriptindex_01,		// タイトル(数字の開始)(1-20)
	//
	// 以下に、ヒントを追加
	pw_movie01_hint_00,		// movie01
	pw_movie01_hint_00,		// movie02
	pw_movie01_hint_00,		// movie03
};

// メッセージ(要修正)
static const s16 bs_info_tbl[] = {
	pw_castindex_06,		// 名前
	pw_castindex_07,		// 男性
	pw_castindex_08,		// 女性
	pw_castindex_09,		// レベル
	pw_castindex_10,		// もちもの
	pw_castindex_11,		// とくせい
	pw_castindex_12,		// 技00
	pw_castindex_13,		// 技01
	pw_castindex_14,		// 技02
	pw_castindex_15,		// 技03
	//
	pw_castindex_16,		// slash
};

//-------------
// fixメッセージ
//-------------
// TOP
static const u16 bs_fmes00_tbl[] = {
	MES_ID_TOP_01,	BS_CVS_TOP_TITLE, 	0, 0,		// タイトル
	MES_ID_TOP_02,	BS_CVS_TOP_PLOT,  	0, 0,		// PLOT
	MES_ID_TOP_03,	BS_CVS_TOP_GOAL,  	0, 0,		// GOAL
	MES_ID_ALL_01,	BS_CVS_TOP_LABEL00, 0, 0,		// 脚本
	MES_ID_ALL_02,	BS_CVS_TOP_LABEL01, 0, 0,		// 共演者
	//
	0xffff, 0, 0, 0,		// end
};
// SCENARIO
static const u16 bs_fmes01_tbl[] = {
	MES_ID_ALL_02,	BS_CVS_SCENARIO_LABEL00, 0, 0,	// 共演者
	//
	MES_ID_SCENARIO,BS_CVS_SCENARIO_SCENARIO, 0, 1,	// シナリオタイトル(mesdata違い)
	//
	0xffff, 0, 0, 0,		// end
};
// INFO
static const u16 bs_fmes02_tbl[] = {
	MES_ID_INFO_01,	BS_CVS_INFO_CAST,	 0, 0,		// タイトル
	MES_ID_INFO_02,	BS_CVS_INFO_ITEM,	 1, 0,		// もちもの
	MES_ID_INFO_03,	BS_CVS_INFO_STATUS,	 1, 0,		// とくせい
	MES_ID_ALL_01,	BS_CVS_INFO_LABEL00, 0, 0,		// 脚本
	//
	0xffff, 0, 0, 0,		// end
};

// fixメッセージ(まとめ)
static const u16 *bs_fmes_tbl[] = {
	bs_fmes00_tbl,
	bs_fmes01_tbl,
	bs_fmes02_tbl,
};

//----------
// 半透明
//----------
static const s16 bs_rate_tbl[] = {
	10,6,		// blend
};

//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void battle_scenario_main( GFL_TCB* tcb, void * work );
static BOOL battle_scenario_end(  GFL_TCB* tcb, BSCENARIO_WORK * wk );

// まとめ
static void bs_main_seq( BSCENARIO_WORK * wk );

static void bs_init_seq(  BSCENARIO_WORK * wk );
static void bs_init_wipe( BSCENARIO_WORK * wk );
static BOOL bs_is_wipe(   BSCENARIO_WORK * wk );
//
static void bs_set_endfade(  BSCENARIO_WORK * wk );
static BOOL bs_is_endfade( BSCENARIO_WORK * wk );
//
static void bs_init_graphics( BSCENARIO_WORK * wk );
static void bs_exit_bg( BSCENARIO_WORK * wk);
static void bs_set_palanim( BSCENARIO_WORK * wk );
//
// 文字列
static void bs_chk_canvas( BSCENARIO_WORK * wk );
//
//---------
// 各処理
//---------
// top
static void bs_init_top( BSCENARIO_WORK * wk );
static void bs_main_top( BSCENARIO_WORK * wk );
static void bs_exit_top( BSCENARIO_WORK * wk );
static void bs_key_top( BSCENARIO_WORK * wk );
static void bs_disp_top_string( BSCENARIO_WORK * wk, const s32 mode );
//
// scenario
static void bs_init_scenario( BSCENARIO_WORK * wk );
static void bs_main_scenario( BSCENARIO_WORK * wk );
static void bs_exit_scenario( BSCENARIO_WORK * wk );
static void bs_key_scenario( BSCENARIO_WORK * wk );
static void bs_disp_scenario_string( BSCENARIO_WORK * wk, const s32 mode );
//
// info
static void bs_init_info( BSCENARIO_WORK * wk );
static void bs_main_info( BSCENARIO_WORK * wk );
static void bs_exit_info( BSCENARIO_WORK * wk );
static void bs_key_info( BSCENARIO_WORK * wk );
static void bs_disp_info_string( BSCENARIO_WORK * wk, const s32 mode );
static void bs_disp_info_obj( BSCENARIO_WORK * wk, const s32 mode );
static void bs_hide_info_obj( BSCENARIO_WORK * wk );

//
// 処理
static BOOL bs_main_loop( BSCENARIO_WORK * wk );
static void bs_chk_end( BSCENARIO_WORK * wk );
// ワーク計算
static void bs_set_pokework( BSCENARIO_WORK * wk );
// 固定文字列
static void bs_disp_fix_string( BSCENARIO_WORK * wk, const s32 mode );
// 個数計算
static void bs_calc_hintnum( BSCENARIO_WORK * wk );
//
static void bs_chk_limit( BSCENARIO_WORK * wk, const s32 mode, const s32 smode );
static void bs_disp_arrow( BSCENARIO_WORK * wk );
//
static void bs_disp_cursor( BSCENARIO_WORK * wk, const s32 mode, const s32 num );
//
static BOOL bs_chk_timeout( BSCENARIO_WORK * wk );

//============================================================================================
//  データ2
//============================================================================================
//------------------
// 関数のテーブル
//------------------
static void (*(bs_functbl[]))(BSCENARIO_WORK *) = {
	bs_init_top,			// top(init)
	bs_main_top,			// top(main)
	bs_exit_top,			// top(exit)
	bs_init_scenario,		// scenario(init)
	bs_main_scenario,		// scenario(main)
	bs_exit_scenario,		// scenario(exit)
	bs_init_info,			// info(init)
	bs_main_info,			// info(main)
	bs_exit_info,			// info(exit)
};

//============================================================================================
//============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * @brief ポケウッド台本タスク追加
 *
 * @param BSCENARIO_DATA*		シナリオデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BattleScenario_TaskAdd( BSCENARIO_DATA * dat )
{
	s32				i;
	BSCENARIO_WORK*	wk;
	//
	// メモリ確保
	wk = GFL_HEAP_AllocClearMemory( dat->heap, sizeof(BSCENARIO_WORK) );
	//
	// battle_scenario_mainを登録する
	GFL_TCB_AddTask( BTLV_EFFECT_GetTCBSYS(), battle_scenario_main, wk, 100 );
	//
	//
	wk->dat			= dat;
	wk->pfd 		= BTLV_EFFECT_GetPfd();
	//
	// 初期化
	wk->Step 		= STEP_INIT;
	wk->Mode		= SMODE_INIT_TOP;
	wk->NextMode	= SMODE_INIT;
	wk->Flag		= 0;
	wk->SubMode		= STEP_INIT;
	wk->TitleMode	= BS_TOP;
	wk->AnimFlag	= 0;
	wk->PFlag		= 0;
	//
	// 台本番号
	wk->TopNum		= dat->scenario_num;
	wk->TopCurPos	= 0;
	//
	wk->PageNum		= 0;
	// wk->ScenarioMaxは初期化時(bs_init_seq)に決まる
	wk->ScenarioCurPos	= 0;
	//
	wk->PokePageNum	= 0;
	// パーティ内の数
	wk->PokeMax		= PokeParty_GetPokeCount( wk->dat->party );
	wk->InfoCurPos	= 0;
	//
	for(i=0;i<BS_TIMER_MAX;i++){
		wk->Timer[i] = 0;
	}
	OS_Printf("scenario num:%d\n",wk->TopNum);
	//
	//
	for(i=0;i<BS_CANVAS_MAXNUM;i++){
		wk->PrintUtil[i].win = 0;
	}
	for(i=0;i<BS_RES_PAL_MAX;i++){		// pal
		wk->ResPal[i] = 0xffffffff;
	}
	for(i=0;i<BS_RES_CELL_MAX;i++){		// cell
		wk->ResCell[i] = 0xffffffff;
	}
	for(i=0;i<BS_RES_CHR_MAX;i++){		// chr
		wk->ResChr[i] = 0xffffffff;
	}
	wk->EmptyBuffer = 0;
	
	// 処理(初期化用)
	bs_main_seq( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	メインタスク
 *
 * @param	GFL_TCB*			tcb
 * @param	void*				work
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void battle_scenario_main( GFL_TCB * tcb, void * work )
{
	BSCENARIO_WORK * wk = (BSCENARIO_WORK *)work;
	
	// 通信エラー時
	if( wk->dat->comm_err_flg == TRUE ){
		wk->Step = STEP_END;
	}
	//
	// 処理
	bs_main_seq( wk );
	//
	// 終了処理
	if( wk->Step == STEP_END ){
    	if( battle_scenario_end( tcb, wk ) == TRUE ){
      		return;
    	}
    }
    //
	// 処理
	GFL_TCBL_Main( wk->tcbl );
	// que
	BsCmn_MainPrint( wk );
	// canvasのウエイト
	bs_chk_canvas( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	終了シーケンス
 *
 * @param	GFL_TCB*			tcb
 * @param	BSCENARIO_WORK*		wk
 *
 * @retval  BOOL				(TRUE:終了)
 */
//--------------------------------------------------------------------------------------------
static BOOL battle_scenario_end( GFL_TCB * tcb, BSCENARIO_WORK * wk )
{
	s32		i;
	s32		mode;
	
	// キュー待ち
	if( PRINTSYS_QUE_IsFinished( wk->print_que ) == FALSE ){
		// 通信エラー時は強制クリア
		if( wk->dat->comm_err_flg == TRUE ){
			PRINTSYS_QUE_Clear( wk->print_que );
		}else{
			return FALSE;
		}
	}
	// ワイプ待ちの場合
	if( WIPE_SYS_EndCheck() == FALSE ){
		// 通信エラー時は強制終了
		if( wk->dat->comm_err_flg == TRUE ){
			WIPE_SYS_ExeEnd();
		}else{
			return FALSE;
		}
	}
	//
	//---------------------------
	// 以下の処理になると完全終了
	//---------------------------
	// obj
	BsCmn_ExitObj( wk );
	// bg
	bs_exit_bg( wk );
	//
	GFL_TCBL_Exit( wk->tcbl );
	//
	//メッセージ破棄
	for(i=0;i<BS_MES_MAX;i++){
		if(wk->MsgData[i]){
			GFL_MSG_Delete( wk->MsgData[i] );
			wk->MsgData[i] = 0;
		}
	}
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	//
	// WORDSET破棄
	WORDSET_Delete( wk->Wordset );
	//
	BsCmn_ExitString( wk );
	// PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );
	//FONT(大元を使用)
//	GFL_FONT_Delete( wk->font );
	GFL_FONT_Delete( wk->sfont );
	
	wk->dat->end_flg = 1;
	
	// キー入力で終了したか
	mode = GFL_APP_END_TOUCH;
	if(wk->Flag & FLG_KEY_INPUT_END){
		mode = GFL_APP_END_KEY;
	}
	GFL_UI_SetTouchOrKey( mode );
	
	GFL_TCB_DeleteTask( tcb );
	GFL_HEAP_FreeMemory( wk );
	
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	シーケンスまとめ
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void bs_main_seq( BSCENARIO_WORK * wk )
{
	BOOL	bflg;
	//
	switch(wk->Step){
		// 初期化
		case STEP_INIT:
			bs_init_seq( wk );
			wk->Step = STEP_INIT+10;
			break;
		//
		// ワイプ待ち
		case STEP_INIT+10:
			bflg = bs_is_wipe( wk );
			if(bflg == FALSE){
				wk->Step = STEP_LV1;
			}
			break;
		//
		// main処理
		case STEP_LV1:
			// 内部で分岐処理
			bflg = bs_main_loop( wk );
			if(bflg == FALSE){
				wk->Step = STEP_LV2;
				bs_set_endfade( wk );
			}
			break;
		//
		// フェードアウト待ち
		case STEP_LV2:
			bflg = bs_is_endfade( wk );
			if(bflg == FALSE){
				wk->Step = STEP_END;
			}
			break;
		//
		// 終了待ち
		case STEP_END:
			break;
		//
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	初期化シーケンス
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void bs_init_seq( BSCENARIO_WORK * wk )
{
	s32			flg;
	
	G2S_BlendNone();
	GXS_SetMasterBrightness( -16 );
	
	wk->tcbl = GFL_TCBL_Init( wk->dat->heap, wk->dat->heap, 1, 4 );
	//
	// フォント作成(大元を使用)
//	wk->font  = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->dat->heap );
	wk->sfont = GFL_FONT_Create( ARCID_FONT, NARC_font_num_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->dat->heap );
	//
	// PRINT_QUE作成
	// defaultの1024では、画面切り替え時に間に合わないことがあり、
	// 画面がちらつくことがあったので2048に変更した
	wk->print_que = PRINTSYS_QUE_CreateEx( 2048, wk->dat->heap );
	//
	// WORDSET生成
	wk->Wordset = WORDSET_Create( wk->dat->heap );
	//
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_B_SCENARIO_GRA, NARC_b_scenario_gra_b_scenario_pos_tblbin, wk->dat->heap );
	//
	// 入らなければ、要修正
	wk->MsgData[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pw_movietitle_dat, wk->dat->heap );
	wk->MsgData[1] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pw_hint001_dat, wk->dat->heap );
	//
	// bg設定
	bs_init_graphics( wk );
	//
	// bg初期化
	BsCmn_InitBg( wk );
	//
	// ヒント数を比較
	// (内部でScenarioMaxが決まる)
	bs_calc_hintnum( wk );
	//
	// 文字列初期化
	BsCmn_InitString( wk, BS_TOP );
	//
	// パラメータセット
	bs_set_pokework( wk );
	//
	// 表示
	bs_disp_top_string( wk, STR_INIT );
	//
	// obj初期化
	BsCmn_InitObj( wk );
	//
	// wipe
	bs_init_wipe( wk );
	// パレットアニメ用
	bs_set_palanim( wk );
	//
	wk->Flag |= FLG_UPDATE;
	//
	// キー入力で終了して、ここに来たかどうか
	flg = GFL_UI_CheckTouchOrKey();
	if(flg == GFL_APP_END_KEY){
		wk->Flag |= FLG_KEY_INPUT;
	}
	//
	// 半透明設定
	G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_NONE,
		GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD,
		bs_rate_tbl[0], bs_rate_tbl[1] );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	画面初期化時の描画待ち(wipe初期化)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void bs_init_wipe( BSCENARIO_WORK * wk )
{
//  if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
		WIPE_SYS_Start(
			WIPE_PATTERN_S, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
			WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->dat->heap );
//	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	画面初期化時のワイプ待ち
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return	BOOL				(TRUE:処理中、FALSE:終了)
 */
//--------------------------------------------------------------------------------------------
static BOOL bs_is_wipe( BSCENARIO_WORK * wk )
{
	BOOL	bflg = TRUE;
	
	// ワイプ判定
	if( WIPE_SYS_EndCheck() == TRUE ){
		bflg = FALSE;
	}
	return( bflg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	終了フェード
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void bs_set_endfade( BSCENARIO_WORK * wk )
{
	PaletteFadeReq( wk->pfd, PF_BIT_SUB_ALL, 0xffff, 0, 0, 16, 0, BTLV_EFFECT_GetTCBSYS() );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	終了フェード待ち
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return	BOOL				(TRUE:処理中、FALSE:終了)
 */
//--------------------------------------------------------------------------------------------
static BOOL bs_is_endfade( BSCENARIO_WORK * wk )
{
	BOOL	bflg=TRUE;
	//
	if( PaletteFadeCheck( wk->pfd ) == 0 ){
		bflg = FALSE;
	}
	return( bflg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	BG初期化
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void bs_init_graphics( BSCENARIO_WORK *wk )
{
  { // BG SYSTEM
    GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGModeDisp( &BGsys_data, GFL_BG_SUB_DISP );
  }

  { // BG3(背景：offsetを切り替える方式)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x2000, 0,											// X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
      GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16,				// ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
      GX_BG_SCRBASE_0xc800, GX_BG_CHARBASE_0x00000, 0x8000,		// ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE							// 拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
  }

  { // BG2
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x0800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x18000, 0x8000,
      GX_BG_EXTPLTT_23, 2, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
  }

  { // BG1(win)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x10000, 0x8000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( GFL_BG_FRAME1_S );
  }

  { // BG0
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x4000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( GFL_BG_FRAME0_S );
  }
  //
  GFL_BG_SetClearCharacter( GFL_BG_FRAME1_S, 32, 0, wk->dat->heap );
  GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S, 32, 0, wk->dat->heap );
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_S );
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_S );

  GFL_DISP_GXS_SetVisibleControl(
    GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 |
    GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3 | GX_PLANEMASK_OBJ,
    VISIBLE_ON );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	BG解放
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void bs_exit_bg( BSCENARIO_WORK *wk )
{
  GFL_DISP_GXS_SetVisibleControl(
    GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 |
    GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3 | GX_PLANEMASK_OBJ,
    VISIBLE_OFF );
  
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	パレットアニメ
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void bs_set_palanim( BSCENARIO_WORK * wk )
{
	// ここまでで、パレットの転送は終えておくこと
	//
	// BGのパレット(7本分)
	PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_BG, 0, 0x20*8 );
	// BGのパレット(10番から3本分)
	PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_BG, 10*16, 0x20*3 );
	// フォントパレット(8)
	PaletteWorkSet_Arc( wk->pfd, ARCID_FONT, NARC_font_talkwin_nclr, wk->dat->heap, FADE_SUB_BG, 0x20, 8*16 );
	//
	// 通信アイコンセット
	GFL_NET_ReloadIconTopOrBottom( FALSE, wk->dat->heap );
	//
	// OBJのパレット(0番目、15本分まとめ)
	PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_OBJ, 0, 0x20*15 );
	PaletteFadeReq( wk->pfd, PF_BIT_SUB_ALL, 0xffff, 0, 16, 0, 0, BTLV_EFFECT_GetTCBSYS() );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フォント表示待ち(要修正)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void bs_chk_canvas( BSCENARIO_WORK * wk )
{
	s32		num;
	BOOL	bflg;
	
	// タイトルの切り替えがあったときだけ処理
	if(!(wk->Flag & FLG_CHG_TITLE)){
		return;
	}
	//
	// 処理が終わってからBG切り替えなどを行う
	bflg = PRINTSYS_QUE_IsFinished( wk->print_que );
	if(bflg == TRUE){
		// スクロール切り替え
		BsCmn_BgScroll( wk, wk->TitleMode );
		// bgスクリーン
		BsCmn_DispArrow( wk, wk->TitleMode, TRUE );
		// 上マーク切替
		BsCmn_DispUpMark( wk, wk->TitleMode );
		//
		// obj非表示(このタイミングにしないとちらつく)
		if( wk->TitleMode != BS_INFO ){
			bs_hide_info_obj( wk );
		}
		// 処理
		num = wk->TopCurPos;
		if(wk->TitleMode == BS_SCENARIO){
			num = wk->ScenarioCurPos;
		}else if(wk->TitleMode == BS_INFO){
			num = wk->InfoCurPos;
		}
		bs_disp_cursor( wk, wk->TitleMode, num );
		
		// 切り替え終了
		wk->Flag &= ~FLG_CHG_TITLE;
	}
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	初期化(top)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_init_top( BSCENARIO_WORK * wk )
{
	// main処理へ
	wk->Mode 		= SMODE_MAIN_TOP;
	wk->SubMode 	= STEP_INIT;		// 初期化
	wk->TitleMode 	= BS_TOP;
	//
	// bg初期化(1回目だけは転送しない)
	if(wk->Flag & FLG_SET_INIT){
		// canvas初期化
		BsCmn_InitString( wk, BS_TOP );
		// 表示まとめ
		bs_disp_top_string( wk, STR_INIT );
		//
		wk->Flag |= FLG_UPDATE;
	}
	wk->Flag |= FLG_SET_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	main(top)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_main_top( BSCENARIO_WORK * wk )
{
	BOOL	bflg;
	
	switch( wk->SubMode ){
		// 初期化
		case STEP_INIT:
			wk->SubMode = STEP_LV1;
//			break;
		
		// 通常
		case STEP_LV1:
			bs_key_top( wk );
			break;
		
		// アニメ待ち(obj)
		case STEP_LV2:
			// キャンセル待ち
			bflg = BsCmn_IsAnimEnd( wk, BS_CLWK_ARROW_BACK );
			if(bflg == FALSE){
				// 終了内容
				wk->Mode++;
				wk->NextMode = SMODE_END;
			}
			break;
		//
		// アニメ待ち(脚本)
		case STEP_LV3:
			// アニメ待ち
			BsCmn_MainScreenAnime( wk );
			bflg = BsCmn_IsScreenAnimeEnd( wk );
			if(bflg == FALSE){
				// 終了内容
				wk->Mode++;
				wk->NextMode = SMODE_INIT_SCENARIO;
			}
			break;
		//
		// アニメ待ち(シナリオ)
		case STEP_LV3+10:
			// アニメ待ち
			BsCmn_MainScreenAnime( wk );
			bflg = BsCmn_IsScreenAnimeEnd( wk );
			if(bflg == FALSE){
				// 終了内容
				wk->Mode++;
				wk->NextMode = SMODE_INIT_INFO;
			}
			break;
		//
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	exit(top)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_exit_top( BSCENARIO_WORK * wk )
{
	// 終了判定
	bs_chk_end( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	入力(top)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_key_top( BSCENARIO_WORK * wk )
{
	s32		flg=0;
	s32		tflg=0;
	s32		mflg=0;
	s32		id;
	s32		nmode;
	BOOL	bflg;
	
	// 更新あり
	if(wk->Flag & FLG_UPDATE){
		mflg = 1;
		wk->Flag &= ~FLG_UPDATE;
	}
	//
	if(mflg == 0){
		// タッチペンの判定
		id = BsCmn_ChkTp( wk, BS_TOP );
		// 共演者
		if(id == BS_TP_TOP_LABEL01){
			flg = 5;
			tflg = 1;
		}
		// 脚本
		else if(id == BS_TP_TOP_LABEL00){
			flg = 6;
			tflg = 1;
		}
		// 戻る
		else if(id == BS_TP_TOP_RETURN){
			flg = 4;
			tflg = 1;
		}
		//
		// キー入力
		if(flg == 0){
			// 左
			if(GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT){
				flg = 1;
			}
			// 右
			else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT){
				flg = 2;
			}
			// A:決定
			else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE){
				flg = 3;
			}
			// B:戻る
			else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
				flg = 4;
			}
		}
	}
	//
	// 入力があった
	if(flg != 0){
		// タッチペンではない
		if(tflg == 0){
			// キー入力があった => 表示可能 => 決定等の反映
			if(wk->Flag & FLG_KEY_INPUT){
				if(flg == 1){			// 左
					if(wk->TopCurPos != 0){
						// カーソル
						PMSND_PlaySE( SND_PKWD_BS_CURSOR );
					}
					wk->TopCurPos--;
					if(wk->TopCurPos < 0){
						wk->TopCurPos = 0;
					}
					// カーソル更新
					mflg = 1;
				}
				else if(flg == 2){		// 右
					if(wk->TopCurPos != 2){
						// カーソル
						PMSND_PlaySE( SND_PKWD_BS_CURSOR );
					}
					wk->TopCurPos++;
					if(wk->TopCurPos > 2){
						wk->TopCurPos = 2;
					}
					// カーソル更新
					mflg = 1;
				}
				else if(flg == 3){		// 決定
					if(wk->TopCurPos == 0){			// 脚本
						BsCmn_InitScreenAnime( wk, BS_SCR_TOP_00 );
						wk->SubMode = STEP_LV3;
						wk->ScenarioCurPos = 0;
						// 決定
						PMSND_PlaySE( SND_PKWD_BS_DECIDE );
					}
					else if(wk->TopCurPos == 1){	// 共演者
						BsCmn_InitScreenAnime( wk, BS_SCR_TOP_01 );
						wk->SubMode = STEP_LV3+10;
						wk->ScenarioCurPos = 0;
						// 決定
						PMSND_PlaySE( SND_PKWD_BS_DECIDE );
					}
					else{							// 戻る
						// 戻るアニメ
						BsCmn_ChgSeq( wk, BS_CLWK_ARROW_BACK, BS_SEQ_BACK_01 );
						// アニメ終了待ち
						wk->SubMode = STEP_LV2;
						wk->Flag |= FLG_KEY_INPUT_END;
						// 戻る
						PMSND_PlaySE( SND_PKWD_BS_CANCEL );
					}
				}
				else if(flg == 4){		// 戻る
					// 戻るアニメ
					BsCmn_ChgSeq( wk, BS_CLWK_ARROW_BACK, BS_SEQ_BACK_01 );
					// アニメ終了待ち
					wk->SubMode = STEP_LV2;
					wk->Flag |= FLG_KEY_INPUT_END;
					// 戻る
					PMSND_PlaySE( SND_PKWD_BS_CANCEL );
				}
			}
			// 表示 => キーでの決定はしない
			else{
				wk->Flag |= FLG_KEY_INPUT;
				wk->TopCurPos = 0;
				mflg = 1;
				// カーソル
				PMSND_PlaySE( SND_PKWD_BS_CURSOR );
			}
		}
		//
		// タッチペンは関係なく処理
		else{
			// 入力を破棄
			wk->Flag &= ~FLG_KEY_INPUT;
			mflg = 1;
			//
			if(flg == 4){			// 戻る
				// 戻るアニメ
				BsCmn_ChgSeq( wk, BS_CLWK_ARROW_BACK, BS_SEQ_BACK_01 );
				// アニメ終了待ち
				wk->SubMode = STEP_LV2;
				// 戻る
				PMSND_PlaySE( SND_PKWD_BS_CANCEL );
			}
			else if(flg == 5){		// 共演者
				BsCmn_InitScreenAnime( wk, BS_SCR_TOP_01 );
				wk->SubMode = STEP_LV3+10;
				wk->InfoCurPos = 0;
				// 決定
				PMSND_PlaySE( SND_PKWD_BS_DECIDE );
			}
			else if(flg == 6){		// シナリオ
				BsCmn_InitScreenAnime( wk, BS_SCR_TOP_00 );
				wk->SubMode = STEP_LV3;
				//
				wk->ScenarioCurPos = 0;
				// 決定
				PMSND_PlaySE( SND_PKWD_BS_DECIDE );
			}
		}
	}
	// 更新
	// カーソル更新
	if(mflg == 1){
		// 処理
		bs_disp_cursor( wk, BS_TOP, wk->TopCurPos );
	}
	//
	// 次の画面へ
//	else if(mflg == 2){
//		wk->Mode++;
//		wk->NextMode = nmode;
//	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	文字列表示(top)
 *
 * @param	BSCENARIO_WORK*		wk
 * @param	const s32			(0:初期化、1:更新)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_disp_top_string( BSCENARIO_WORK * wk, const s32 mode )
{
	s32			i;
	s32			id = wk->TopNum;
	STRBUF		*strbuf;
	
	// 初期のみでよいはず
	if(mode == STR_INIT){
		// 固定文字列
		bs_disp_fix_string( wk, BS_TOP );
		//
		// タイトル、あらすじ、クリア目標
		for(i=0;i<3;i++){
			strbuf = GFL_MSG_CreateString( wk->MsgData[0], bs_mes_tbl[i]+id );
			BsCmn_DispString( wk, BS_CVS_TOP_TITLE_MES+i, 0, 0, strbuf, 0, PRINTSYS_LSB_Make(15,2,0), 0 );
			GFL_STR_DeleteBuffer( strbuf );
		}
		// 切り替え
		wk->Flag |= FLG_CHG_TITLE;
	}
}

//------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	初期化(scenario)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_init_scenario( BSCENARIO_WORK * wk )
{
	// 次の処理へ
	wk->Mode 		= SMODE_MAIN_SCENARIO;
	wk->SubMode 	= STEP_INIT;		// 初期化
	wk->TitleMode 	= BS_SCENARIO;
	//
	// canvas初期化
	BsCmn_InitString( wk, BS_SCENARIO );
	//
	// 表示(パラメータをセットしておくこと)
	bs_disp_scenario_string( wk, STR_INIT );
	//
	// リミットチェック
	bs_chk_limit( wk, BS_SCENARIO, 1 );
	
	wk->Flag |= FLG_UPDATE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	main(scenario)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_main_scenario( BSCENARIO_WORK * wk )
{
	BOOL	bflg;
	
	switch( wk->SubMode ){
		// 初期化
		case STEP_INIT:
			wk->SubMode = STEP_LV1;
//			break;
		//
		case STEP_LV1:
			bs_key_scenario( wk );
			break;
		//
		case STEP_LV2:
			// キャンセル待ち
			bflg = BsCmn_IsAnimEnd( wk, BS_CLWK_ARROW_BACK );
			if(bflg == FALSE){
				// 終了内容
				wk->Mode++;
				wk->NextMode = SMODE_INIT_TOP;
			}
			break;
		//
		// 左アニメ待ち=>戻る
		case STEP_LV2+10:
			bflg = BsCmn_IsAnimEnd( wk, BS_CLWK_ARROW_LEFT );
			if(bflg == FALSE){
				// 終了内容
				wk->SubMode = STEP_LV1;
				//
				if(wk->Flag & FLG_LEFT_LIMIT){
					BsCmn_ChgSeq( wk, BS_CLWK_ARROW_LEFT, BS_SEQ_LEFT_02 );
				}
			}
			break;
		//
		// 右アニメ待ち=>戻る
		case STEP_LV2+20:
			bflg = BsCmn_IsAnimEnd( wk, BS_CLWK_ARROW_RIGHT );
			if(bflg == FALSE){
				// 終了内容
				wk->SubMode = STEP_LV1;
				//
				if(wk->Flag & FLG_RIGHT_LIMIT){
					BsCmn_ChgSeq( wk, BS_CLWK_ARROW_RIGHT, BS_SEQ_RIGHT_02 );
				}
			}
			break;
		//
		// アニメ待ち(脚本)
		case STEP_LV3:
			// アニメ待ち
			BsCmn_MainScreenAnime( wk );
			bflg = BsCmn_IsScreenAnimeEnd( wk );
			if(bflg == FALSE){
				// 終了内容
				wk->Mode++;
				wk->NextMode = SMODE_INIT_INFO;
			}
			break;
		//
		case STEP_LV4:
			break;
		//
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	exit(scenario)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_exit_scenario( BSCENARIO_WORK * wk )
{
	// 終了判定
	bs_chk_end( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	入力(scenario)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_key_scenario( BSCENARIO_WORK * wk )
{
	s32		flg=0;
	s32		tflg=0;
	s32		mflg=0;
	s32		dflg=0;
	s32		id;
	s32		nmode;
	s32		buf[2];
	s32		tmp;
	BOOL	bflg;
	
	// キー表示の更新
	if(wk->Flag & FLG_UPDATE){
		mflg = 1;
		wk->Flag &= ~FLG_UPDATE;
	}
	//
	if(mflg == 0){
		// タッチペンの判定
		id = BsCmn_ChkTp( wk, BS_SCENARIO );
		// 共演者
		if(id == BS_TP_SCENARIO_LABEL00){
			flg = 5;
			tflg = 1;
		}
		// 左
		else if(id == BS_TP_SCENARIO_LEFT){
			flg = 6;
			tflg = 1;
		}
		// 右
		else if(id == BS_TP_SCENARIO_RIGHT){
			flg = 7;
			tflg = 1;
		}
		// 戻る
		else if(id == BS_TP_SCENARIO_RETURN){
			flg = 4;
			tflg = 1;
		}
		//
		// キー入力
		if(flg == 0){
			// 左
			if(GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT){
				flg = 1;
			}
			// 右
			else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT){
				flg = 2;
			}
			// A:決定
			else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE){
				flg = 3;
			}
			// B:戻る
			else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
				flg = 4;
			}
		}
	}
	
	//
	// 入力があった
	if(flg != 0){
		// タッチペンではない
		if(tflg == 0){
			// キー入力があった => 表示可能 => 決定等の反映
			if(wk->Flag & FLG_KEY_INPUT){
				if(flg == 1){			// 左
					if(wk->ScenarioCurPos != 0){
						// カーソル
						PMSND_PlaySE( SND_PKWD_BS_CURSOR );
					}
					wk->ScenarioCurPos--;
					if(wk->ScenarioCurPos < 0){
						wk->ScenarioCurPos = 0;
					}
					// カーソル更新
					mflg = 1;
				}
				else if(flg == 2){		// 右
					if(wk->ScenarioCurPos != 3){
						// カーソル
						PMSND_PlaySE( SND_PKWD_BS_CURSOR );
					}
					wk->ScenarioCurPos++;
					if(wk->ScenarioCurPos > 3){
						wk->ScenarioCurPos = 3;
					}
					// カーソル更新
					mflg = 1;
				}
				else if(flg == 3){		// 決定
					if(wk->ScenarioCurPos == 0){		// 脚本
						BsCmn_InitScreenAnime( wk, BS_SCR_SCENARIO_00 );
						wk->SubMode = STEP_LV3;
						// 決定
						PMSND_PlaySE( SND_PKWD_BS_DECIDE );
					}
					else if(wk->ScenarioCurPos == 1){	// 左
						if(wk->PageNum != 0){
							// (カーソルでも)決定
							PMSND_PlaySE( SND_PKWD_BS_DECIDE );
						}
						wk->PageNum--;
						if(wk->PageNum < 0){
							wk->PageNum = 0;
							// リミット
							wk->Flag |= FLG_LEFT_GRAY;
							wk->Flag |= FLG_LEFT_UPDATE;
						}else{
							dflg = 1;
							// 左アニメ
							BsCmn_ChgSeq( wk, BS_CLWK_ARROW_LEFT, BS_SEQ_LEFT_01 );
							// アニメ終了待ち
							wk->SubMode = STEP_LV2+10;
							//
							wk->Flag &= ~FLG_RIGHT_GRAY;
							wk->Flag |= FLG_RIGHT_UPDATE;
						}
						// リミットチェック
						bs_chk_limit( wk, BS_SCENARIO, 0 );
					}
					else if(wk->ScenarioCurPos == 2){	// 右
						buf[0] = wk->ScenarioMax/2;
						buf[1] = wk->ScenarioMax%2;
						tmp = buf[0];
						tmp += buf[1];		// 端数があれば1ページ追加
						//
						if(wk->PageNum != (tmp-1)){
							// (カーソルでも)決定
							PMSND_PlaySE( SND_PKWD_BS_DECIDE );
						}
						wk->PageNum++;
						if(wk->PageNum > (tmp-1)){
							wk->PageNum = (tmp-1);
							// リミット
							wk->Flag |= FLG_RIGHT_GRAY;
							wk->Flag |= FLG_RIGHT_UPDATE;
						}else{
							// 右アニメ
							BsCmn_ChgSeq( wk, BS_CLWK_ARROW_RIGHT, BS_SEQ_RIGHT_01 );
							// アニメ終了待ち
							wk->SubMode = STEP_LV2+20;
							dflg = 1;
							//
							wk->Flag &= ~FLG_LEFT_GRAY;
							wk->Flag |= FLG_LEFT_UPDATE;
						}
						// リミットチェック
						bs_chk_limit( wk, BS_SCENARIO, 0 );
					}
					else{							// 戻る
						// 戻るアニメ
						BsCmn_ChgSeq( wk, BS_CLWK_ARROW_BACK, BS_SEQ_BACK_01 );
						// アニメ終了待ち
						wk->SubMode = STEP_LV2;
						wk->ScenarioCurPos = 0;
						wk->TopCurPos = 0;
						// 戻る
						PMSND_PlaySE( SND_PKWD_BS_CANCEL );
					}
				}
				else if(flg == 4){		// 戻る
					// 戻るアニメ
					BsCmn_ChgSeq( wk, BS_CLWK_ARROW_BACK, BS_SEQ_BACK_01 );
					// アニメ終了待ち
					wk->SubMode = STEP_LV2;
					wk->ScenarioCurPos = 0;
					wk->TopCurPos = 0;
					// 戻る
					PMSND_PlaySE( SND_PKWD_BS_CANCEL );
				}
			}
			// 表示 => キーでの決定はしない
			else{
				wk->Flag |= FLG_KEY_INPUT;
				wk->ScenarioCurPos = 0;
				mflg = 1;
				// カーソル
				PMSND_PlaySE( SND_PKWD_BS_CURSOR );
			}
		}
		//
		// タッチペンは関係なく処理
		else{
			if(flg == 4){			// 戻る
				// 戻るアニメ
				BsCmn_ChgSeq( wk, BS_CLWK_ARROW_BACK, BS_SEQ_BACK_01 );
				// アニメ終了待ち
				wk->SubMode = STEP_LV2;
				wk->Flag &= ~FLG_KEY_INPUT;
				wk->ScenarioCurPos = 0;
				wk->TopCurPos = 0;
				// 戻る
				PMSND_PlaySE( SND_PKWD_BS_CANCEL );
			}
			else if(flg == 5){		// 共演者
				BsCmn_InitScreenAnime( wk, BS_SCR_SCENARIO_00 );
				wk->SubMode = STEP_LV3;
				//
				wk->Flag &= ~FLG_KEY_INPUT;
				wk->ScenarioCurPos = 0;
				// 決定
				PMSND_PlaySE( SND_PKWD_BS_DECIDE );
			}
			else if(flg == 6){		// 左
				if(wk->PageNum != 0){
					// (カーソルでも)決定
					PMSND_PlaySE( SND_PKWD_BS_DECIDE );
				}
				wk->PageNum--;
				if(wk->PageNum < 0){
					wk->PageNum = 0;
					// リミット
					wk->Flag |= FLG_LEFT_GRAY;
					wk->Flag |= FLG_LEFT_UPDATE;
				}else{
					dflg = 1;
					//
					wk->Flag &= ~FLG_KEY_INPUT;
					mflg = 1;
					// 左アニメ
					BsCmn_ChgSeq( wk, BS_CLWK_ARROW_LEFT, BS_SEQ_LEFT_01 );
					// アニメ終了待ち
					wk->SubMode = STEP_LV2+10;
					//
					wk->Flag &= ~FLG_RIGHT_GRAY;
					wk->Flag |= FLG_RIGHT_UPDATE;
				}
				// リミットチェック
				bs_chk_limit( wk, BS_SCENARIO, 0 );
			}
			else if(flg == 7){		// 右
				buf[0] = wk->ScenarioMax/2;
				buf[1] = wk->ScenarioMax%2;
				tmp = buf[0];
				tmp += buf[1];		// 端数があれば1ページ追加
				//
				if(wk->PageNum != (tmp-1)){
					// (カーソルでも)決定
					PMSND_PlaySE( SND_PKWD_BS_DECIDE );
				}
				wk->PageNum++;
				if(wk->PageNum > (tmp-1)){
					wk->PageNum = (tmp-1);
					// リミット
					wk->Flag |= FLG_RIGHT_GRAY;
					wk->Flag |= FLG_RIGHT_UPDATE;
				}else{
					dflg = 1;
					//
					wk->Flag &= ~FLG_KEY_INPUT;
					mflg = 1;
					// 右アニメ
					BsCmn_ChgSeq( wk, BS_CLWK_ARROW_RIGHT, BS_SEQ_RIGHT_01 );
					// アニメ終了待ち
					wk->SubMode = STEP_LV2+20;
					//
					wk->Flag &= ~FLG_LEFT_GRAY;
					wk->Flag |= FLG_LEFT_UPDATE;
				}
				// リミットチェック
				bs_chk_limit( wk, BS_SCENARIO, 0 );
			}
		}
	}
	
	// 更新
	// カーソル更新
	if(mflg == 1){
		// 処理
		bs_disp_cursor( wk, BS_SCENARIO, wk->ScenarioCurPos );
	}
	//
	// 次の画面へ
//	else if(mflg == 2){
//		wk->Mode++;
//		wk->NextMode = nmode;
//	}
	//
	if(dflg == 1){
		// メッセージ更新
		bs_disp_scenario_string( wk, STR_UPDATE );
	}
	//
	// 矢印処理
	bs_disp_arrow( wk );
	
	//
	//
	// debugテスト切り替え
#if 0
	// R
	if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_R){
		BsCmn_ObjVisible( wk, BS_CLWKMODE_ARROW, BS_CLWK_ARROW_LEFT,  TRUE );
		BsCmn_ObjVisible( wk, BS_CLWKMODE_ARROW, BS_CLWK_ARROW_RIGHT, TRUE );
		BsCmn_ChgSeq( wk, BS_CLWK_ARROW_LEFT,  BS_SEQ_LEFT_02 );
		BsCmn_ChgSeq( wk, BS_CLWK_ARROW_RIGHT, BS_SEQ_RIGHT_02 );
	}
	// L
	else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_L){
		BsCmn_ObjVisible( wk, BS_CLWKMODE_ARROW, BS_CLWK_ARROW_LEFT,  FALSE );
		BsCmn_ObjVisible( wk, BS_CLWKMODE_ARROW, BS_CLWK_ARROW_RIGHT, FALSE );
	}
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	文字列表示(scenario)
 *
 * @param	BSCENARIO_WORK*		wk
 * @param	const s32			(0:初期化、1:更新)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_disp_scenario_string( BSCENARIO_WORK * wk, const s32 mode )
{
	s32		i;
	s32		mesid;
	s32		topid;
	s32		buf[2];
	s32		tmp;
	STRBUF	*strbuf;
	
	// 初期化
	if(mode == STR_INIT){
		// fix
		bs_disp_fix_string( wk, BS_SCENARIO );
		// slash
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], bs_info_tbl[10] );
		BsCmn_DispString( wk, BS_CVS_SCENARIO_SLASH, 0, 0, strbuf, 0, PRINTSYS_LSB_Make(1,2,0), 1 );
		GFL_STR_DeleteBuffer( strbuf );
		// 切り替え
		wk->Flag |= FLG_CHG_TITLE;
	}
//	else{
		//-------
		// 更新
		//-------
		// 先頭のidをセット(そこから連番のはず)
		topid = bs_scene_tbl[0];
		mesid = bs_scene_tbl[1+wk->TopNum];		// 本当はこれ
		//
		//-----------
		// シーン00
		//-----------
		// 見出し
		strbuf = GFL_MSG_CreateString( wk->MsgData[1], topid+2*wk->PageNum );
		BsCmn_DispString( wk, BS_CVS_SCENARIO_SCENE00, 0, 0, strbuf, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
		GFL_STR_DeleteBuffer( strbuf );
		//ヒント
		strbuf = GFL_MSG_CreateString( wk->MsgData[1], mesid+2*wk->PageNum );
		BsCmn_DispString( wk, BS_CVS_SCENARIO_SCENE00_MES, 0, 0, strbuf, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
		GFL_STR_DeleteBuffer( strbuf );
		
		//-----------
		// シーン01
		//-----------
		if((2*wk->PageNum+1) < wk->ScenarioMax){
			// 見出し
			strbuf = GFL_MSG_CreateString( wk->MsgData[1], topid+2*wk->PageNum+1 );
			BsCmn_DispString( wk, BS_CVS_SCENARIO_SCENE01, 0, 0, strbuf, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
			GFL_STR_DeleteBuffer( strbuf );
			// ヒント
			strbuf = GFL_MSG_CreateString( wk->MsgData[1], mesid+2*wk->PageNum+1 );
			BsCmn_DispString( wk, BS_CVS_SCENARIO_SCENE01_MES, 0, 0, strbuf, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
			GFL_STR_DeleteBuffer( strbuf );
		}
		else{
			// 非表示
			BsCmn_ClearCanvas( wk, BS_CVS_SCENARIO_SCENE01 );
			BsCmn_ClearCanvas( wk, BS_CVS_SCENARIO_SCENE01_MES );
		}
		//
		//-------------
		// ページ数
		//-------------
		// 数字(now)表示は1はじまり(左そろえ)
		BsCmn_DispStringNumber( wk, BS_CVS_SCENARIO_NUM00, 0, 0, (wk->PageNum+1), 2, 0, 1 );
		// 数字(max)
		buf[0] = wk->ScenarioMax/2;
		buf[1] = wk->ScenarioMax%2;
		tmp = buf[0];
		tmp += buf[1];		// 端数があれば1ページ追加
		// 数字(max)
		BsCmn_DispStringNumber( wk, BS_CVS_SCENARIO_NUM01, 0, 0, tmp, 2, 0, 1 );
//	}
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	初期化(info)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_init_info( BSCENARIO_WORK * wk )
{
	// 次の処理へ
	wk->Mode 		= SMODE_MAIN_INFO;
	wk->SubMode 	= STEP_INIT;
	wk->TitleMode 	= BS_INFO;
	//
	// canvas初期化
	BsCmn_InitString( wk, BS_INFO );
	//
	// 表示(初期パラメータをセットしておくこと)
	bs_disp_info_string( wk, STR_INIT );
	//
	// 表示
	bs_disp_info_obj( wk, 0 );
	//
	// リミットチェック
	bs_chk_limit( wk, BS_INFO, 1 );
	//
	wk->Flag |= FLG_UPDATE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	main(info)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_main_info( BSCENARIO_WORK * wk )
{
	BOOL	bflg;
	
	switch( wk->SubMode ){
		// 初期化
		case STEP_INIT:
			wk->SubMode = STEP_LV1;
//			break;
		//
		case STEP_LV1:
			bs_key_info( wk );
			break;
		//
		// キャンセルアニメ待ち=>戻る
		case STEP_LV2:
			bflg = BsCmn_IsAnimEnd( wk, BS_CLWK_ARROW_BACK );
			if(bflg == FALSE){
				// 終了内容
				wk->Mode++;
				wk->NextMode = SMODE_INIT_TOP;
			}
			break;
		//
		// 左アニメ待ち=>戻る
		case STEP_LV2+10:
			bflg = BsCmn_IsAnimEnd( wk, BS_CLWK_ARROW_LEFT );
			if(bflg == FALSE){
				// 終了内容
				wk->SubMode = STEP_LV1;
			}
			break;
		//
		// 右アニメ待ち=>戻る
		case STEP_LV2+20:
			bflg = BsCmn_IsAnimEnd( wk, BS_CLWK_ARROW_RIGHT );
			if(bflg == FALSE){
				// 終了内容
				wk->SubMode = STEP_LV1;
			}
			break;
		//
		// アニメ待ち(共演者)
		case STEP_LV3:
			// アニメ待ち
			BsCmn_MainScreenAnime( wk );
			bflg = BsCmn_IsScreenAnimeEnd( wk );
			if(bflg == FALSE){
				// 終了内容
				wk->Mode++;
				wk->NextMode = SMODE_INIT_SCENARIO;
			}
			break;
		//
		case STEP_LV4:
			break;
		//
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	exit(info)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_exit_info( BSCENARIO_WORK * wk )
{
	// 終了判定
	bs_chk_end( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	入力(info)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
#if 0	// test
static s32 rx=16;
static s32 ry=0;
#endif
//
static void bs_key_info( BSCENARIO_WORK * wk )
{
	s32		flg=0;
	s32		tflg=0;
	s32		mflg=0;
	s32		dflg=0;
	s32		id;
	s32		nmode;
	s32		buf[2];
	s32		tmp;
	BOOL	bflg;
	
	// 更新
	if(wk->Flag & FLG_UPDATE){
		mflg = 1;
		wk->Flag &= ~FLG_UPDATE;
	}
	//
	if(mflg == 0){
		// タッチペンの判定
		id = BsCmn_ChkTp( wk, BS_INFO );
		// 共演者
		if(id == BS_TP_INFO_LABEL00){
			flg = 5;
			tflg = 1;
		}
		// 左
		else if(id == BS_TP_INFO_LEFT){
			flg = 6;
			tflg = 1;
		}
		// 右
		else if(id == BS_TP_INFO_RIGHT){
			flg = 7;
			tflg = 1;
		}
		// 戻る
		else if(id == BS_TP_INFO_RETURN){
			flg = 4;
			tflg = 1;
		}
		//
		// キー入力
		if(flg == 0){
			// 左
			if(GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT){
				flg = 1;
			}
			// 右
			else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT){
				flg = 2;
			}
			// A:決定
			else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE){
				flg = 3;
			}
			// B:戻る
			else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL){
				flg = 4;
			}
		}
	}
	
	//
	// 入力があった
	if(flg != 0){
		// タッチペンではない
		if(tflg == 0){
			// キー入力があった => 表示可能 => 決定等の反映
			if(wk->Flag & FLG_KEY_INPUT){
				if(flg == 1){			// 左
					if(wk->InfoCurPos != 0){
						// カーソル
						PMSND_PlaySE( SND_PKWD_BS_CURSOR );
					}
					wk->InfoCurPos--;
					if(wk->InfoCurPos < 0){
						wk->InfoCurPos = 0;
					}
					// カーソル更新
					mflg = 1;
				}
				else if(flg == 2){		// 右
					if(wk->InfoCurPos != 3){
						// カーソル
						PMSND_PlaySE( SND_PKWD_BS_CURSOR );
					}
					wk->InfoCurPos++;
					if(wk->InfoCurPos > 3){
						wk->InfoCurPos = 3;
					}
					// カーソル更新
					mflg = 1;
				}
				else if(flg == 3){		// 決定
					if(wk->InfoCurPos == 0){		// 脚本
						BsCmn_InitScreenAnime( wk, BS_SCR_INFO_00 );
						wk->SubMode = STEP_LV3;
						// 決定
						PMSND_PlaySE( SND_PKWD_BS_DECIDE );
					}
					else if(wk->InfoCurPos == 1){	// 左
						if(!(wk->Flag & FLG_LEFT_LIMIT)){
							// (カーソルでも)決定
							PMSND_PlaySE( SND_PKWD_BS_DECIDE );
							//
							wk->PokePageNum--;
							if(wk->PokePageNum < 0){
								wk->PokePageNum = (wk->PokeMax-1);
							}
							dflg = 1;
							//
							// 左アニメ
							BsCmn_ChgSeq( wk, BS_CLWK_ARROW_LEFT, BS_SEQ_LEFT_01 );
							// アニメ終了待ち
							wk->SubMode = STEP_LV2+10;
						}
					}
					else if(wk->InfoCurPos == 2){	// 右
						if(!(wk->Flag & FLG_RIGHT_LIMIT)){
							// (カーソルでも)決定
							PMSND_PlaySE( SND_PKWD_BS_DECIDE );
							//
							wk->PokePageNum++;
							if(wk->PokePageNum >= wk->PokeMax){
								wk->PokePageNum = 0;
							}
							dflg = 1;
							//
							// 右アニメ
							BsCmn_ChgSeq( wk, BS_CLWK_ARROW_RIGHT, BS_SEQ_RIGHT_01 );
							// アニメ終了待ち
							wk->SubMode = STEP_LV2+20;
						}
					}
					else{							// 戻る
						// 戻るアニメ
						BsCmn_ChgSeq( wk, BS_CLWK_ARROW_BACK, BS_SEQ_BACK_01 );
						// アニメ終了待ち
						wk->SubMode = STEP_LV2;
						wk->InfoCurPos = 0;
						wk->TopCurPos = 0;
						// 戻る
						PMSND_PlaySE( SND_PKWD_BS_CANCEL );
					}
				}
				else if(flg == 4){		// 戻る
					// 戻るアニメ
					BsCmn_ChgSeq( wk, BS_CLWK_ARROW_BACK, BS_SEQ_BACK_01 );
					// アニメ終了待ち
					wk->SubMode = STEP_LV2;
					wk->InfoCurPos = 0;
					wk->TopCurPos = 0;
					// 戻る
					PMSND_PlaySE( SND_PKWD_BS_CANCEL );
				}
			}
			// 表示 => キーでの決定はしない
			else{
				wk->Flag |= FLG_KEY_INPUT;
				wk->InfoCurPos = 0;
				mflg = 1;
				// カーソル
				PMSND_PlaySE( SND_PKWD_BS_CURSOR );
			}
		}
		//
		// タッチペンは関係なく処理
		else{
			if(flg == 4){			// 戻る
				// 戻るアニメ
				BsCmn_ChgSeq( wk, BS_CLWK_ARROW_BACK, BS_SEQ_BACK_01 );
				// アニメ終了待ち
				wk->SubMode = STEP_LV2;
				wk->Flag &= ~FLG_KEY_INPUT;
				wk->InfoCurPos = 0;
				wk->TopCurPos = 0;
				// 戻る
				PMSND_PlaySE( SND_PKWD_BS_CANCEL );
			}
			else if(flg == 5){		// 脚本
				BsCmn_InitScreenAnime( wk, BS_SCR_INFO_00 );
				wk->SubMode = STEP_LV3;
				//
				wk->Flag &= ~FLG_KEY_INPUT;
				wk->InfoCurPos = 0;
				// 決定
				PMSND_PlaySE( SND_PKWD_BS_DECIDE );
			}
			else if(flg == 6){		// 左
				if(!(wk->Flag & FLG_LEFT_LIMIT)){
					wk->PokePageNum--;
					if(wk->PokePageNum < 0){
						wk->PokePageNum = (wk->PokeMax-1);
					}
					dflg = 1;
					// (カーソルでも)決定
					PMSND_PlaySE( SND_PKWD_BS_DECIDE );
					//
					wk->Flag &= ~FLG_KEY_INPUT;
					mflg = 1;
					// 左アニメ
					BsCmn_ChgSeq( wk, BS_CLWK_ARROW_LEFT, BS_SEQ_LEFT_01 );
					// アニメ終了待ち
					wk->SubMode = STEP_LV2+10;
				}
			}
			else if(flg == 7){		// 右
				if(!(wk->Flag & FLG_RIGHT_LIMIT)){
					wk->PokePageNum++;
					if(wk->PokePageNum >= wk->PokeMax){
						wk->PokePageNum = 0;
					}
					dflg = 1;
					// (カーソルでも)決定
					PMSND_PlaySE( SND_PKWD_BS_DECIDE );
					//
					wk->Flag &= ~FLG_KEY_INPUT;
					mflg = 1;
					// 右アニメ
					BsCmn_ChgSeq( wk, BS_CLWK_ARROW_RIGHT, BS_SEQ_RIGHT_01 );
					// アニメ終了待ち
					wk->SubMode = STEP_LV2+20;
				}
			}
		}
	}
	
	// 更新
	// カーソル更新
	if(mflg == 1){
		// 処理
		bs_disp_cursor( wk, BS_INFO, wk->InfoCurPos );
	}
	//
	// 次の画面へ
//	else if(mflg == 2){
//		wk->Mode++;
//		wk->NextMode = nmode;
//	}
	//
	if(dflg == 1){
		// メッセージ更新
		bs_disp_info_string( wk, STR_UPDATE );
		// OBJ表示
		bs_disp_info_obj( wk, 1 );
	}

#if 0	// test
	flg = 0;
	// L
	if(GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT){
		rx--;
		flg = 1;
	}
	// R
	else if(GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT){
		rx++;
		flg = 1;
	}
	// UP
	if(GFL_UI_KEY_GetTrg() & PAD_KEY_UP){
		ry--;
		flg = 1;
	}
	// DOWN
	else if(GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN){
		ry++;
		flg = 1;
	}
	//
	if(flg == 1){
		G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_NONE,
			GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD,
			rx, ry);
		//
		OS_Printf("%d,%d\n",rx,ry);
	}
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	文字列表示(info)(要整理)
 *
 * @param	BSCENARIO_WORK*		wk
 * @param	const s32			(0:初期化、1:更新)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_disp_info_string( BSCENARIO_WORK * wk, const s32 mode )
{
	s32				tmp;
	s32				i;
	s32				buf[2];
	u32				pcol;
	POKEMON_PARAM	*pp;
	STRBUF			*exp;
	STRBUF			*str;
	
	// 初期化
	if(mode == STR_INIT){
		// fix
		bs_disp_fix_string( wk, BS_INFO );
		// 切り替え
		wk->Flag |= FLG_CHG_TITLE;
	}
//	else{
		// 更新
		//
		// ポケモン情報
		pp = PokeParty_GetMemberPointer( wk->dat->party, wk->PokePageNum );
		//
		// 1回だけ確保
		exp = GFL_STR_CreateBuffer( 128, wk->dat->heap );
		//
		// 名前
		buf[0] = wk->PokeWork[wk->PokePageNum].id;
		str = GFL_MSG_CreateString( wk->MsgData[0], bs_info_tbl[0] );
		WORDSET_RegisterPokeMonsNameNo( wk->Wordset, 0, buf[0] );
		WORDSET_ExpandStr( wk->Wordset, exp, str );
		BsCmn_DispString( wk, BS_CVS_INFO_NAME, 0, 0, exp, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
		GFL_STR_DeleteBuffer( str );
		//
		// 性別
		tmp = 1;
		pcol   = PRINTSYS_LSB_Make(6,5,0);
		buf[0] = wk->PokeWork[wk->PokePageNum].sex;
		if(buf[0] == PTL_SEX_FEMALE){
			tmp  = 2;
			pcol = PRINTSYS_LSB_Make(4,3,0);
		}
		str = GFL_MSG_CreateString( wk->MsgData[0], bs_info_tbl[tmp] );
		BsCmn_DispString( wk, BS_CVS_INFO_SEX, 0, 0, str, 0, pcol, 0 );
		GFL_STR_DeleteBuffer( str );
		//
		// レベル
		buf[0] = wk->PokeWork[wk->PokePageNum].level;
		str = GFL_MSG_CreateString( wk->MsgData[0], bs_info_tbl[3] );
		WORDSET_RegisterNumber( wk->Wordset, 0, buf[0], 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_ZENKAKU);
		WORDSET_ExpandStr( wk->Wordset, exp, str );
		BsCmn_DispString( wk, BS_CVS_INFO_LEVEL, 0, 0, exp, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
		GFL_STR_DeleteBuffer( str );
		//
		// もちもの
		buf[0] = wk->PokeWork[wk->PokePageNum].item;
		str = GFL_MSG_CreateString( wk->MsgData[0], bs_info_tbl[4] );
		WORDSET_RegisterItemName( wk->Wordset, 0, buf[0] );
		WORDSET_ExpandStr( wk->Wordset, exp, str );
		BsCmn_DispString( wk, BS_CVS_INFO_ITEM01, 0, 0, exp, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
		GFL_STR_DeleteBuffer( str );
		//
		// 技
		for(i=0;i<4;i++){
			buf[0] = wk->PokeWork[wk->PokePageNum].skill[i];
			if(buf[0] != 0){
				str = GFL_MSG_CreateString( wk->MsgData[0], bs_info_tbl[6+i] );
				WORDSET_RegisterWazaName( wk->Wordset, 0, buf[0] );
				WORDSET_ExpandStr( wk->Wordset, exp, str );
				// 表示
				BsCmn_DispString( wk, BS_CVS_INFO_SKILL00+i, 0, 0, exp, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
				GFL_STR_DeleteBuffer( str );
			}else{
				BsCmn_ClearCanvas( wk, BS_CVS_INFO_SKILL00+i );
			}
		}
		//
		// 特性
		buf[0] = wk->PokeWork[wk->PokePageNum].stat;
		str = GFL_MSG_CreateString( wk->MsgData[0], bs_info_tbl[5] );
		WORDSET_RegisterTokuseiName( wk->Wordset, 0, buf[0] );
		WORDSET_ExpandStr( wk->Wordset, exp, str );
		BsCmn_DispString( wk, BS_CVS_INFO_STATUS01, 0, 0, exp, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
		GFL_STR_DeleteBuffer( str );
		//
		// 破棄
		GFL_STR_DeleteBuffer( exp );
//	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	obj表示(info)
 *
 * @param	BSCENARIO_WORK*		wk
 * @param	const s32			(0:初期化、1:更新)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_disp_info_obj( BSCENARIO_WORK * wk, const s32 mode )
{
	u32				buf[2];
	s32				i;
	POKEMON_PARAM	*pp;
	BOOL			bflg;
	
	// ポケモン情報
	pp = PokeParty_GetMemberPointer( wk->dat->party, wk->PokePageNum );
	//
	//--------
	// ポケモン
	//--------
	for(i=0;i<BS_POKE_MAX;i++){
		if(i == wk->PokePageNum){
			BsCmn_ChgTypePoke( wk, i, wk->PokeWork[i].id, wk->PokeWork[i].sex );
			bflg = TRUE;
		}else{
			bflg = FALSE;
		}
		BsCmn_ObjVisible( wk, BS_CLWKMODE_POKE, i, bflg );
	}
	
	//--------
	// タイプ
	//--------
	// type1
	buf[0] = wk->PokeWork[wk->PokePageNum].poke_type[0];
	BsCmn_ChgTypeObj( wk, 0, 0, buf[0] );
	BsCmn_ObjVisible( wk, BS_CLWKMODE_NORMAL, 0, TRUE );
	//
	// type2(違う場合のみ表示)
	buf[1] = wk->PokeWork[wk->PokePageNum].poke_type[1];
	if(buf[0] != buf[1]){
		BsCmn_ChgTypeObj( wk, 1, 0, buf[1] );
		bflg = TRUE;
	}else{
		bflg = FALSE;
	}
	BsCmn_ObjVisible( wk, BS_CLWKMODE_NORMAL, 1, bflg );
	
//	OS_Printf("type :%d,%d\n",buf[0],buf[1]);
	
	//--------
	// タイプ
	//--------
	for(i=0;i<4;i++){
		// 技
		buf[0] = wk->PokeWork[wk->PokePageNum].skill[i];
		if(buf[0] != 0){
			// 技のタイプ
			BsCmn_ChgTypeObj( wk, 2+i, 1, wk->PokeWork[wk->PokePageNum].skill_type[i] );
			bflg = TRUE;
		}else{
			bflg = FALSE;
		}
		BsCmn_ObjVisible( wk, BS_CLWKMODE_NORMAL, 2+i, bflg );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	obj全非表示(info)
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_hide_info_obj( BSCENARIO_WORK * wk )
{
	s32			i;
	
	// タイプ、技タイプ
	for(i=0;i<BS_CLWK_MAX;i++){
		BsCmn_ObjVisible( wk, BS_CLWKMODE_NORMAL, i, FALSE );
	}
	// ポケモン
	for(i=0;i<BS_CLWK_POKE_MAX;i++){
		BsCmn_ObjVisible( wk, BS_CLWKMODE_POKE, i, FALSE );
	}
}

//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	処理まとめ
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return	BOOL				(TRUE:制御中、FALSE:終了)
 */
//--------------------------------------------------------------------------------------------
static BOOL bs_main_loop( BSCENARIO_WORK * wk )
{
	BOOL	bflg=TRUE;
	BOOL	tflg;
	
	// タイムアウトエラー判定
	tflg = bs_chk_timeout( wk );
	if(tflg == TRUE){
		wk->NextMode = SMODE_END;
		return( FALSE );
	}
	//
	if((wk->Mode < 0)||(wk->Mode >= SMODE_MAX)){
		OS_Printf("mode error\n");
		return( FALSE );
	}
	// 処理の分岐
	bs_functbl[wk->Mode]( wk );
	//
	//
	// 終了判定
	if(wk->NextMode == SMODE_END){
		bflg = FALSE;
	}
	return( bflg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	終了処理まとめ
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_chk_end( BSCENARIO_WORK * wk )
{
	// 次の処理
	if(wk->NextMode != SMODE_END){
		wk->Mode = wk->NextMode;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	ポケモンパラメータセット
 *
 * @param	BSCENARIO_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_set_pokework( BSCENARIO_WORK * wk )
{
	s32				i;
	s32				j;
	POKEMON_PARAM	*pp;
	
	// 初期化(なくてもよいはず)
	for(i=0;i<BS_POKE_MAX;i++){
		wk->PokeWork[i].id		= 0;
		wk->PokeWork[i].sex		= 0;
		wk->PokeWork[i].level	= 0;
		wk->PokeWork[i].item	= 0;
		wk->PokeWork[i].poke_type[0] = 0;
		wk->PokeWork[i].poke_type[1] = 0;
		//
		for(j=0;j<4;j++){
			wk->PokeWork[i].skill[j] = 0;
			wk->PokeWork[i].skill_type[j] = 0;
		}
	}
	
	// PP_Get関連は重たい(?)ようなので、
	// 初期化時にあらかじめ取得しておく
	// セット
	for(i=0;i<wk->PokeMax;i++){
		// ポケモン情報
		pp = PokeParty_GetMemberPointer( wk->dat->party, i );
		// 名前
		wk->PokeWork[i].id = PP_Get( pp, ID_PARA_monsno, 0 );
		// 性別
		wk->PokeWork[i].sex = PP_GetSex( pp );
		// レベル
		wk->PokeWork[i].level = PP_Get( pp, ID_PARA_level, 0 );
		// もちもの
		wk->PokeWork[i].item = PP_Get( pp, ID_PARA_item, 0 );
		// もちもの
		wk->PokeWork[i].stat = PP_Get( pp, ID_PARA_speabino, 0 );
		// タイプ
		wk->PokeWork[i].poke_type[0] = PP_Get( pp, ID_PARA_type1, 0 );
		wk->PokeWork[i].poke_type[1] = PP_Get( pp, ID_PARA_type2, 0 );
		// 技、技タイプ
		for(j=0;j<4;j++){
			wk->PokeWork[i].skill[j] = PP_Get( pp, ID_PARA_waza1+j, 0 );
			if(wk->PokeWork[i].skill[j] != 0){
				wk->PokeWork[i].skill_type[j] = WAZADATA_GetParam( wk->PokeWork[i].skill[j], WAZAPARAM_TYPE );
			}
		}
	}
#if 0
	// debug表示
	for(i=0;i<BS_POKE_MAX;i++){
		OS_Printf("id, sex, level, item, stat, poke_type[0], poke_type[1] :%d,%d,%d,%d,%d,%d,%d\n",
			wk->PokeWork[i].id, wk->PokeWork[i].sex, wk->PokeWork[i].level, 
			wk->PokeWork[i].item, wk->PokeWork[i].stat, wk->PokeWork[i].poke_type[0], wk->PokeWork[i].poke_type[1] );
		OS_Printf("skill :%d,%d,%d,%d\n",
			wk->PokeWork[i].skill[0], wk->PokeWork[i].skill[1], wk->PokeWork[i].skill[2], wk->PokeWork[i].skill[3] );
		OS_Printf("skill type :%d,%d,%d,%d\n",
			wk->PokeWork[i].skill_type[0], wk->PokeWork[i].skill_type[1], wk->PokeWork[i].skill_type[2], wk->PokeWork[i].skill_type[3] );
		
	}
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	fix文字表示
 *
 * @param	BSCENARIO_WORK*		wk
 * @param	const s32			モード(0-2)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_disp_fix_string( BSCENARIO_WORK * wk, const s32 mode )
{
	u16		buf[4];
	const u16	*adr;
	u16		col;
	s32		i;
	STRBUF	*strbuf;
	
	adr = bs_fmes_tbl[mode];
	for(i=0;i<BS_CANVAS_MAXNUM;i++){
		// 終了判定
		if(adr[4*i] == 0xffff){
			break;
		}
		buf[0] = adr[4*i];		// mesid
		buf[1] = adr[4*i+1];	// id
		buf[2] = adr[4*i+2];	// col
		buf[3] = adr[4*i+3];	// num
		// 表示
		col = PRINTSYS_LSB_Make(15,2,0);
		if(buf[2] == 1){
			col = PRINTSYS_LSB_Make(1,2,0);
		}
		strbuf = GFL_MSG_CreateString( wk->MsgData[ buf[3] ], bs_fix_mestbl[ buf[0] ] );
		BsCmn_DispString( wk, buf[1], 0, 0, strbuf, 0, col, 0 );
		GFL_STR_DeleteBuffer( strbuf );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	ページ数の計算
 *
 * @param	BSCENARIO_WORK*	wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_calc_hintnum( BSCENARIO_WORK * wk )
{
	s16		mesid;
	s32		i,j;
	BOOL	bflg;
	STRBUF	*strbuf;		// 元
	STRBUF	*strbuf2;		// 比較するもの(読み替え)
	
	// 先頭のid
	mesid = bs_scene_tbl[1+wk->TopNum];
	//
	// 判定のメッセージを取得
	strbuf = GFL_MSG_CreateString( wk->MsgData[1], MES_END_ID );
	//ヒント数をカウント
	j=0;
	for(i=0;i<MES_HINT_MAX;i++){
		strbuf2 = GFL_MSG_CreateString( wk->MsgData[1], mesid+i );
		// 文字列を比較
		bflg = GFL_STR_CompareBuffer( strbuf, strbuf2 );
		// 同じ=>終了と判定
		if(bflg == TRUE){
			GFL_STR_DeleteBuffer( strbuf2 );
			break;
		}else{
			GFL_STR_DeleteBuffer( strbuf2 );
		}
		j++;
	}
	// ここまでで数が出る
	OS_Printf("hit num :%d\n",j);
	//
	wk->ScenarioMax = j;
//	wk->ScenarioMax = 5;		// test
	//
	GFL_STR_DeleteBuffer( strbuf );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	矢印表示
 *
 * @param	BSCENARIO_WORK*	wk
 * @param	const s32		(番号:(0-2))
 * @param	const s32		(0:通常、1:初期化時)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_chk_limit( BSCENARIO_WORK * wk, const s32 mode, const s32 smode )
{
	s32		tmp;
	s32		buf[2];
	s32		seq;
	s32		id;
	
	// シナリオ
	if(mode == BS_SCENARIO){
		wk->Flag &= ~(FLG_LEFT_LIMIT|FLG_RIGHT_LIMIT);
		//
		// 左端チェック
		if(wk->PageNum <= 0){
			wk->Flag |= FLG_LEFT_LIMIT;
		}
		//
		// 右端チェック
		buf[0] = wk->ScenarioMax/2;
		buf[1] = wk->ScenarioMax%2;
		tmp = buf[0];
		tmp += buf[1];		// 端数があれば1ページ追加
		if(wk->PageNum >= (tmp-1)){
			wk->Flag |= FLG_RIGHT_LIMIT;
		}
		// 初期化時
		if(smode == 1){
			// 左
			id  = BS_CLWK_ARROW_LEFT;
			if(wk->Flag & FLG_LEFT_LIMIT){	// 灰色
				seq = BS_SEQ_LEFT_02;
			}else{							// 通常
				seq = BS_SEQ_LEFT_00;
			}
			BsCmn_ChgSeq( wk, id, seq );
			
			// 右
			id  = BS_CLWK_ARROW_RIGHT;
			if(wk->Flag & FLG_RIGHT_LIMIT){	// 灰色
				seq = BS_SEQ_RIGHT_02;
			}else{
				seq = BS_SEQ_RIGHT_00;		// 通常
			}
			BsCmn_ChgSeq( wk, id, seq );
		}
	}
	// 情報画面は選択カーソルはロールするので、
	// 判定は初期化時だけでよいはず
	else if(mode == BS_INFO){
		wk->Flag &= ~(FLG_LEFT_LIMIT|FLG_RIGHT_LIMIT);
		//
		// 最大数が1以下の場合
		// => 左右どちらも選択不可
		if(wk->PokeMax <= 1){
			wk->Flag |= (FLG_LEFT_LIMIT|FLG_RIGHT_LIMIT);
		}
		//
		// 初期化時
		if(smode == 1){
			// 左
			id  = BS_CLWK_ARROW_LEFT;
			if(wk->Flag & FLG_LEFT_LIMIT){	// 灰色
				seq = BS_SEQ_LEFT_02;
			}else{							// 通常
				seq = BS_SEQ_LEFT_00;
			}
			BsCmn_ChgSeq( wk, id, seq );
			
			// 右
			id  = BS_CLWK_ARROW_RIGHT;
			if(wk->Flag & FLG_RIGHT_LIMIT){	// 灰色
				seq = BS_SEQ_RIGHT_02;
			}else{							// 通常
				seq = BS_SEQ_RIGHT_00;
			}
			BsCmn_ChgSeq( wk, id, seq );
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	矢印表示
 *
 * @param	BSCENARIO_WORK*	wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_disp_arrow( BSCENARIO_WORK * wk )
{
	s32		id;
	s32		seq;
	
	// 左更新
	if(wk->Flag & FLG_LEFT_UPDATE){
		// 左
		id  = BS_CLWK_ARROW_LEFT;
		if(wk->Flag & FLG_LEFT_GRAY){	// 灰色
			seq = BS_SEQ_LEFT_02;
		}else{							// 通常
			seq = BS_SEQ_LEFT_00;
		}
		BsCmn_ChgSeq( wk, id, seq );
		//
		wk->Flag &= ~FLG_LEFT_UPDATE;
	}
	// 右更新
	if(wk->Flag & FLG_RIGHT_UPDATE){
		// 右
		id  = BS_CLWK_ARROW_RIGHT;
		if(wk->Flag & FLG_RIGHT_GRAY){	// 灰色
			seq = BS_SEQ_RIGHT_02;
		}else{							// 通常
			seq = BS_SEQ_RIGHT_00;
		}
		BsCmn_ChgSeq( wk, id, seq );
		//
		wk->Flag &= ~FLG_RIGHT_UPDATE;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	カーソル表示判定
 *
 * @param	BSCENARIO_WORK*	wk
 * @param	const s32		(番号(0-2))
 * @param	const s32		(カーソル位置)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_disp_cursor( BSCENARIO_WORK * wk, const s32 mode, const s32 num )
{
	BOOL	bflg;
	
	if(wk->Flag & FLG_KEY_INPUT){
		bflg = TRUE;
	}else{
		bflg = FALSE;
	}
	BsCmn_DispCursor( wk, bflg, mode, num );
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	強制終了チェック
 *
 * @param	BSCENARIO_WORK*	wk
 *
 * @return  BOOL			(TRUE:終了、FALSE:まだ)
 */
//--------------------------------------------------------------------------------------------
static BOOL bs_chk_timeout( BSCENARIO_WORK * wk )
{
	BOOL	bflg=FALSE;
	
	// 判定
	if( wk->dat->time_out_flg == TRUE ){
		bflg = TRUE;
	}
	return( bflg );
}

