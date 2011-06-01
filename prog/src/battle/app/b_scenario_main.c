//============================================================================================
/**
 * @file		b_scenario_main.c
 * @brief		戦闘用シナリオ(セリフ)画面
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
//#include "ui/print_tool.h"

#include "b_scenario.h"
#include "b_scenario_main.h"

#include "b_scenario_sub.h"

#include "b_scenario_gra.naix"
#include "msg/msg_pw_movietitle.h"
#include "msg/msg_b_plist.h"		//

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
	//
	STEP_END		= 10000,
};

//
enum{
	SMODE_INIT			= -2,	// 初期値
	SMODE_END 			= -1,	// 終了
	//
	SMODE_INIT_TOP		= 0,	// トップ
	SMODE_MAIN_TOP,				// トップ
	SMODE_EXIT_TOP,				// トップ
	SMODE_INIT_SCENARIO,		// シナリオ
	SMODE_MAIN_SCENARIO,		// シナリオ
	SMODE_EXIT_SCENARIO,		// シナリオ
	SMODE_INIT_INFO,			// 情報
	SMODE_MAIN_INFO,			// 情報
	SMODE_EXIT_INFO,			// 情報
	//
	SMODE_MAX,					// 最大
};

// 
enum{
	STR_INIT			= 0,	// 初期化
	STR_UPDATE,					// 更新
};

//------------
// フラグ
//------------
enum{
	FLG_SET_INIT		= (1<<0),	// 初期化済み
	FLG_CHG_FONT		= (1<<1),	// フォント切り替え
	FLG_UPDATE			= (1<<2),	// 更新フラグ
	//
	FLG_WAIT_START		= (1<<8),	// 開始
	FLG_WAIT_END		= (1<<9),	// 終了待ち終了
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
static void bs_wait_canvas( BSCENARIO_WORK * wk, const BOOL bflg );
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
//
// test
static void bs_disp_test_string( BSCENARIO_WORK * wk, const s32 mode );


//============================================================================================
//  データ
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

// メッセージ
static const s16 bs_mes_tbl[] = {
	pw_seantitle_001,		// タイトル先頭
	pw__seanmisson_001,		// ミッションの先頭(たぶん入力ミス)
	pw__seanhint_001,		// ヒントの先頭(たぶん入力ミス)
	-1,
};

// メッセージ
static const s16 bs_scene_tbl[] = {
	pw_seantitle_001,		// シーンテスト(test)
	-1,
};

// メッセージ(要修正)
static const s16 bs_info_tbl[] = {
	mes_b_plist_01_001,		// 名前
	mes_b_plist_02_502,		// 男性
	mes_b_plist_02_503,		// 女性
	mes_b_plist_03_000,		// レベル
	mes_b_plist_01_001,		// もちもの
	mes_b_plist_01_001,		// とくせい
	//
	mes_b_plist_01_001,		// 技00
	mes_b_plist_01_001,		// 技00
	mes_b_plist_01_001,		// 技00
	mes_b_plist_01_001,		// 技00
};

//============================================================================================
//  
//============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * @brief		戦闘用シナリオタスク追加
 *
 * @param		dat   シナリオデータ
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
	wk->initFlag	= FALSE;
	//
	// 初期化
	wk->Step 		= STEP_INIT;
	wk->Mode		= SMODE_INIT_TOP;
	wk->NextMode	= SMODE_INIT;
	wk->Flag		= 0;
	//
	wk->SubMode		= STEP_INIT;
	wk->PageNum		= 0;
	wk->ScenarioMax	= 19;		// test
	wk->PokePageNum	= 0;
	wk->TopNum		= 0;
	// ポケモン数
	wk->PokeMax		= PokeParty_GetPokeCount( wk->dat->party );
	//
	for(i=0;i<BSCENARIO_CANVAS_MAXNUM;i++){
		wk->PrintUtil[i].win = 0;
	}
	for(i=0;i<2;i++){		// pal
		wk->ResPal[i] = 0xffffffff;
	}
	for(i=0;i<2;i++){		// cell
		wk->ResCell[i] = 0xffffffff;
	}
	for(i=0;i<6;i++){		// chr
		wk->ResChr[i] = 0xffffffff;
	}
	wk->EmptyBuffer = 0;
	
	// 処理(初期化用)
	bs_main_seq( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインタスク
 *
 * @param		tcb
 * @param		work
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void battle_scenario_main( GFL_TCB * tcb, void * work )
{
	BSCENARIO_WORK * wk = (BSCENARIO_WORK *)work;
	
//	if( wk->dat->comm_err_flg == TRUE ){
//		wk->dat->ret_item = ITEM_DUMMY_DATA;
//		wk->dat->ret_cost = 0;
//		wk->seq = SEQ_BBAG_END;
//		wk->Step = STEP_END;
//	}
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
 * @brief		終了シーケンス
 *
 * @param		wk    ワーク
 *
 * @retval  "TRUE = 終了"
 * @retval  "FALSE = フェード中"
 */
//--------------------------------------------------------------------------------------------
static BOOL battle_scenario_end( GFL_TCB * tcb, BSCENARIO_WORK * wk )
{
	s32		i;
	
	if( wk->initFlag == TRUE ){
/*
		if( PRINTSYS_QUE_IsFinished( wk->que ) == FALSE ){
			// 通信エラー時は強制クリア
			if( wk->dat->comm_err_flg == 1 ){
				PRINTSYS_QUE_Clear( wk->que );
			}else{
				return FALSE;
			}
	  }
		if( WIPE_SYS_EndCheck() == FALSE ){
			// 通信エラー時は強制終了
			if( wk->dat->comm_err_flg == 1 ){
				WIPE_SYS_ExeEnd();
			}else{
				return FALSE;
			}
		}

		if( wk->stream != NULL ){
			PRINTSYS_PrintStreamDelete( wk->stream );
		}
*/
//	  BBAGANM_ButtonExit( wk );

//	  BattleBag_ObjFree( wk );
//	  BattleBag_BmpFreeAll( wk );
//	  BBAG_MsgManExit( wk );
		// obj
		BsCmn_ExitObj( wk );
		// bg
		bs_exit_bg( wk );
		//
		GFL_TCBL_Exit( wk->tcbl );
	}
	//メッセージ破棄
	for(i=0;i<BSCENARIO_MES_MAX;i++){
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
	
	GFL_TCB_DeleteTask( tcb );
	GFL_HEAP_FreeMemory( wk );
	
	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		シーケンスまとめ
 *
 * @param		wk    ワーク
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
 * @brief		初期化シーケンス（通常）
 *
 * @param		wk    ワーク
 *
 * @return
 */
//--------------------------------------------------------------------------------------------
static void bs_init_seq( BSCENARIO_WORK * wk )
{
	G2S_BlendNone();
	GXS_SetMasterBrightness( -16 );
	
	wk->tcbl = GFL_TCBL_Init( wk->dat->heap, wk->dat->heap, 1, 4 );
	//
	// フォント作成(大元を使用)
//	wk->font  = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->dat->heap );
	wk->sfont = GFL_FONT_Create( ARCID_FONT, NARC_font_num_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->dat->heap );
	//
	// PRINT_QUE作成
	wk->print_que = PRINTSYS_QUE_Create( wk->dat->heap );
	//
	// WORDSET生成
	wk->Wordset = WORDSET_Create( wk->dat->heap );
	//
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_B_SCENARIO_GRA, NARC_b_scenario_gra_b_scenario_pos_tblbin, wk->dat->heap );
	//
	// 入らなければ、要修正
	wk->MsgData[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pw_movietitle_dat, wk->dat->heap );
	wk->MsgData[1] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_b_plist_dat, wk->dat->heap );			// test
	//
	// bg設定
	bs_init_graphics( wk );
	//
	// bg初期化
	BsCmn_InitBg( wk );
	//
	// 文字列初期化
	BsCmn_InitString( wk, BS_TOP );
	//
	// 表示
	bs_disp_top_string( wk, STR_INIT );
	//
	// obj初期化
	BsCmn_InitObj( wk );
	
	// wipe
	bs_init_wipe( wk );
	// パレットアニメ用
	bs_set_palanim( wk );
	//
	// 初期化終了
	wk->initFlag = TRUE;
	//
	wk->Flag |= FLG_UPDATE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		画面初期化時の描画待ち(wipe初期化)
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
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
 * @brief		画面初期化時のワイプ待ち
 *
 * @param wk			ワーク
 *
 * @return BOOL			(TRUE:処理中、FALSE:終了)
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
 * @brief		終了フェードセットシーケンス
 *
 * @param		wk    ワーク
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
 * @brief		終了フェード待ち
 *
 * @param		wk    ワーク
 *
 * @return		(TRUE:処理中、FALSE:終了)
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
 * @brief		BG初期化
 *
 * @param		wk		ワーク
 *
 * @return  none
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
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000, 0x8000,
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
 * @brief		BG解放
 *
 * @param		wk		ワーク
 *
 * @return  none
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
 * @brief		グラフィックデータセット
 *
 * @param		wk		ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_set_palanim( BSCENARIO_WORK * wk )
{
	// BGのパレット(0-2)
	PaletteWorkSet_Arc( wk->pfd, ARCID_B_SCENARIO_GRA, NARC_b_scenario_gra_pkwd_scenario_NCLR, wk->dat->heap, FADE_SUB_BG, 0x20*3, 0 );
	// フォントパレット(8)
	PaletteWorkSet_Arc( wk->pfd, ARCID_FONT, NARC_font_talkwin_nclr, wk->dat->heap, FADE_SUB_BG, 0x20, 8*16 );
	//
	// 通信アイコンセット
	GFL_NET_ReloadIconTopOrBottom( FALSE, wk->dat->heap );
	//
	// OBJのパレット(0番目、8本分)
	PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_OBJ, 0, 0x20*8 );
	// OBJのパレット(14番目、1本分)
	PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_OBJ, 14*16, 0x20 );
	PaletteFadeReq( wk->pfd, PF_BIT_SUB_ALL, 0xffff, 0, 16, 0, 0, BTLV_EFFECT_GetTCBSYS() );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フォント表示待ち
 *
 * @param		wk		ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_chk_canvas( BSCENARIO_WORK * wk )
{
	BOOL	bflg;
	
	// BsCmn_InitStringで、
	// CanvasをCreateしてDeleteをするため、
	// 切り替え時にゴミが残ることがある
	// =>Queを待って終了してから表示するように変更した
	//
	// フォント切り替えがない=>処理しない
	if(!(wk->Flag & FLG_CHG_FONT)){
		return;
	}
	//
	// 処理が終わってから表示
	bflg = PRINTSYS_QUE_IsFinished( wk->print_que );
	if(bflg == TRUE){
		// wait終了=>フォント面の表示
		bs_wait_canvas( wk, FALSE );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		フォント表示待ち
 *
 * @param		wk		ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_wait_canvas( BSCENARIO_WORK * wk, const BOOL bflg )
{
	// 待ち開始 => 非表示
	if(bflg == TRUE){
		wk->Flag |= FLG_CHG_FONT;
		GFL_BG_SetVisible( GFL_BG_FRAME1_S, FALSE );
	}
	// 待ち終了 => 表示
	else{
		wk->Flag &= ~FLG_CHG_FONT;
		GFL_BG_SetVisible( GFL_BG_FRAME1_S, TRUE );
	}
}

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief		初期化(top)
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_init_top( BSCENARIO_WORK * wk )
{
	// main処理へ
	wk->Mode = SMODE_MAIN_TOP;
	wk->SubMode = STEP_INIT;		// 初期化
	//
	OS_Printf("mode init top\n");
	//
	// bg初期化(1回目だけは転送しない)
	if(wk->Flag & FLG_SET_INIT){
		// スクロール切り替え
		BsCmn_BgScroll( wk, BS_TOP );
		// 表示待ち開始
		bs_wait_canvas( wk, TRUE );
		// canvas初期化
		BsCmn_InitString( wk, BS_TOP );
		// 表示まとめ
		bs_disp_top_string( wk, STR_INIT );
	}
	wk->Flag |= FLG_SET_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		main(top)
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_main_top( BSCENARIO_WORK * wk )
{
	switch( wk->SubMode ){
		// 初期化
		case STEP_INIT:
			wk->SubMode = STEP_LV1;
//			break;
		//
		case STEP_LV1:
			bs_key_top( wk );
			break;
		//
		case STEP_LV2:
			break;
		//
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		exit(top)
 *
 * @param		wk    ワーク
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
 * @brief		main(top)
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_key_top( BSCENARIO_WORK * wk )
{
	s32		flg=0;
	s32		id;
	s32		nmode;
	
	// R:共演者
	if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_R){
		// 終了処理
		nmode = SMODE_INIT_INFO;
		flg = 1;
	}
	// L:脚本
	else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_L){
		// 終了処理
		nmode = SMODE_INIT_SCENARIO;
		flg = 1;
	}
	// B:戻る
	else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_B){
		// 終了処理
		nmode = SMODE_END;
		flg = 1;
	}
	if(flg == 0){
		// タッチペンの判定
		id = BsCmn_ChkTp( wk, BS_TOP );
		// 共演者
		if(id == BS_TP_TOP_LABEL01){
			nmode = SMODE_INIT_INFO;
			flg = 1;
		}
		// 脚本
		else if(id == BS_TP_TOP_LABEL00){
			nmode = SMODE_INIT_SCENARIO;
			flg = 1;
		}
		// 戻る
		else if(id == BS_TP_TOP_RETURN){
			nmode = SMODE_END;
			flg = 1;
		}
	}
	//
	// 入力があった
	if(flg == 1){
		wk->Mode++;
		wk->NextMode = nmode;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列表示(top)
 *
 * @param	BSCENARIO_WORK*		ワーク
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
		// (0-2:test)
		for(i=0;i<3;i++){
			BsCmn_DispStringNumber( wk, BS_CVS_TOP_TITLE+i, 0, 0, mode*1000+i, 8, 0, 0);
		}
		// (6,7:test)
		for(i=0;i<2;i++){
			BsCmn_DispStringNumber( wk, BS_CVS_TOP_LABEL00+i, 0, 0, mode*1000+i, 8, 0, 0);
		}
		// 後でまとめること
		// タイトル
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], bs_mes_tbl[0]+id );
		BsCmn_DispString( wk, BS_CVS_TOP_TITLE_MES, 0, 0, strbuf, 0, PRINTSYS_LSB_Make(15,2,0), 0 );
		GFL_STR_DeleteBuffer( strbuf );
		//
		// あらすじ
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], bs_mes_tbl[1]+id );
		BsCmn_DispString( wk, BS_CVS_TOP_PLOT_MES, 0, 0, strbuf, 0, PRINTSYS_LSB_Make(15,2,0), 0 );
		GFL_STR_DeleteBuffer( strbuf );
		//
		// クリア目標
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], bs_mes_tbl[2]+id );
		BsCmn_DispString( wk, BS_CVS_TOP_GOAL_MES, 0, 0, strbuf, 0, PRINTSYS_LSB_Make(15,2,0), 0 );
		GFL_STR_DeleteBuffer( strbuf );
	}
}

//------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief		初期化(scenario)
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_init_scenario( BSCENARIO_WORK * wk )
{
	// 次の処理へ
	wk->Mode = SMODE_MAIN_SCENARIO;
	wk->SubMode = STEP_INIT;		// 初期化
	//
	OS_Printf("mode init scenario\n");
	// スクロール切り替え
	BsCmn_BgScroll( wk, BS_SCENARIO );
	// 表示待ち開始
	bs_wait_canvas( wk, TRUE );
	// canvas初期化
	BsCmn_InitString( wk, BS_SCENARIO );
	// 表示
	bs_disp_scenario_string( wk, STR_INIT );
	//
	wk->Flag |= FLG_UPDATE;
	//
	bs_main_scenario( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		main(scenario)
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_main_scenario( BSCENARIO_WORK * wk )
{
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
			break;
		//
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		main(scenario)
 *
 * @param		wk    ワーク
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
 * @brief		main(top)
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_key_scenario( BSCENARIO_WORK * wk )
{
	s32		flg=0;
	s32		mflg=0;
	s32		id;
	s32		nmode;
	s32		buf[2];
	s32		tmp;
	
	// 更新あり
	if(wk->Flag & FLG_UPDATE){
		mflg = 1;
		wk->Flag &= ~FLG_UPDATE;
	}
	// 左
	if(GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT){
		wk->PageNum--;
		if(wk->PageNum <= 0){
			wk->PageNum = 0;
		}
		mflg = 1;
	}
	// 右
	else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT){
		buf[0] = wk->ScenarioMax/2;
		buf[1] = wk->ScenarioMax%2;
		tmp = buf[0];
		tmp += buf[1];		// 端数があれば1ページ追加
		//
		wk->PageNum++;
		if(wk->PageNum >= (tmp-1)){
			wk->PageNum = (tmp-1);
		}
		mflg = 1;
	}
	//
	if(mflg == 0){
		// B:戻る
		if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_B){
			// 終了処理=>TOP
			nmode = SMODE_INIT_TOP;
			flg = 1;
		}
		if(flg == 0){
			// タッチペンの判定
			id = BsCmn_ChkTp( wk, BS_SCENARIO );
			// 共演者
			if(id == BS_TP_SCENARIO_LABEL00){
				nmode = SMODE_INIT_INFO;
				flg = 1;
			}
			// 戻る=>TOP
			else if(id == BS_TP_SCENARIO_RETURN){
				nmode = SMODE_INIT_TOP;
				flg = 1;
			}
			// 左
			else if(id == BS_TP_SCENARIO_LEFT){
				wk->PageNum--;
				if(wk->PageNum <= 0){
					wk->PageNum = 0;
				}
				mflg = 1;
			}
			// 右
			else if(id == BS_TP_SCENARIO_RIGHT){
				buf[0] = wk->ScenarioMax/2;
				buf[1] = wk->ScenarioMax%2;
				tmp = buf[0];
				tmp += buf[1];		// 端数があれば1ページ追加
				//
				wk->PageNum++;
				if(wk->PageNum >= (tmp-1)){
					wk->PageNum = (tmp-1);
				}
				mflg = 1;
			}
		}
	}
	//
	// 入力があった
	if(flg == 1){
		wk->Mode++;
		wk->NextMode = nmode;
	}
	else{
		// 左右キー
		if(mflg == 1){
			// メッセージ更新
			bs_disp_scenario_string( wk, STR_UPDATE );
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列表示(scenario)
 *
 * @param	BSCENARIO_WORK*	ワーク
 * @param	const s32		(0:初期化、1:更新)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_disp_scenario_string( BSCENARIO_WORK * wk, const s32 mode )
{
	s32		i;
	s32		buf[2];
	s32		tmp;
	STRBUF	*strbuf;
	
	// 初期化
	if(mode == STR_INIT){
		// タイトル(シナリオ)
		BsCmn_DispStringNumber( wk, BS_CVS_SCENARIO_SCENARIO, 0, 0, mode*1000, 8, 0, 0);
		// 共演者
		BsCmn_DispStringNumber( wk, BS_CVS_SCENARIO_LABEL00, 0, 0, mode*1000+1, 8, 0, 0);
	}
	//
	// 更新
	//-----------
	// シーン00
	//-----------
	BsCmn_DispStringNumber( wk, BS_CVS_SCENARIO_SCENE00, 0, 0, 2*wk->PageNum, 8, 0, 0 );
	strbuf = GFL_MSG_CreateString( wk->MsgData[0], bs_scene_tbl[0]+2*wk->PageNum );
	BsCmn_DispString( wk, BS_CVS_SCENARIO_SCENE00_MES, 0, 0, strbuf, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
	GFL_STR_DeleteBuffer( strbuf );
	
	//-----------
	// シーン01
	//-----------
	if((2*wk->PageNum+1) < wk->ScenarioMax){
		// シーン01
		BsCmn_DispStringNumber( wk, BS_CVS_SCENARIO_SCENE01, 0, 0, 2*wk->PageNum+1, 8, 0, 0 );
		// シーン01-mes
		strbuf = GFL_MSG_CreateString( wk->MsgData[0], bs_scene_tbl[0]+2*wk->PageNum+1 );
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
}

//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief		初期化(info)
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_init_info( BSCENARIO_WORK * wk )
{
	// 次の処理へ
	wk->Mode = SMODE_MAIN_INFO;
	wk->SubMode = STEP_INIT;
	//
	OS_Printf("mode init info\n");
	// スクロール切り替え
	BsCmn_BgScroll( wk, BS_INFO );
	// 表示待ち開始
	bs_wait_canvas( wk, TRUE );
	// canvas初期化
	BsCmn_InitString( wk, BS_INFO );
	// 表示
	bs_disp_info_string( wk, STR_INIT );
	// 表示
	bs_disp_info_obj( wk, 0 );
	//
	wk->Flag |= FLG_UPDATE;
	//
	bs_main_info( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		main(top)
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_main_info( BSCENARIO_WORK * wk )
{
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
		case STEP_LV2:
			break;
		//
		default:
			break;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		main(top)
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_exit_info( BSCENARIO_WORK * wk )
{
	// 終了判定
	bs_chk_end( wk );
	// obj非表示
	bs_hide_info_obj( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		main(top)
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_key_info( BSCENARIO_WORK * wk )
{
	s32		i;
	s32		flg=0;
	s32		mflg=0;
	s32		id;
	s32		nmode;
	
	// 更新あり
	if(wk->Flag & FLG_UPDATE){
		mflg = 1;
		wk->Flag &= ~FLG_UPDATE;
	}
	// 左
	if(GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT){
		wk->PokePageNum--;
		if(wk->PokePageNum < 0){
			wk->PokePageNum = (wk->PokeMax-1);
		}
		mflg = 1;
	}
	// 右
	else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT){
		wk->PokePageNum++;
		if(wk->PokePageNum >= wk->PokeMax){
			wk->PokePageNum = 0;
		}
		mflg = 1;
	}
	//
	if(mflg == 0){
		// B:戻る
		if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_B){
			// 終了処理=>TOP
			nmode = SMODE_INIT_TOP;
			flg = 1;
		}
		if(flg == 0){
			// タッチペンの判定
			id = BsCmn_ChkTp( wk, BS_INFO );
			// 共演者
			if(id == BS_TP_INFO_LABEL00){
				nmode = SMODE_INIT_SCENARIO;
				flg = 1;
			}
			// 戻る=>TOP
			else if(id == BS_TP_INFO_RETURN){
				nmode = SMODE_INIT_TOP;
				flg = 1;
			}
			// 左
			else if(id == BS_TP_INFO_LEFT){
				wk->PokePageNum--;
				if(wk->PokePageNum < 0){
					wk->PokePageNum = (wk->PokeMax-1);
				}
				mflg = 1;
			}
			// 右
			else if(id == BS_TP_INFO_RIGHT){
				wk->PokePageNum++;
				if(wk->PokePageNum >= wk->PokeMax){
					wk->PokePageNum = 0;
				}
				mflg = 1;
			}
		}
	}
	//
	// 入力があった
	if(flg == 1){
		wk->Mode++;
		wk->NextMode = nmode;
	}
	else{
		// 左右キー
		if(mflg == 1){
			// メッセージ更新
			bs_disp_info_string( wk, STR_UPDATE );
			// OBJ表示
			bs_disp_info_obj( wk, 1 );
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		文字列表示(info)(要整理)
 *
 * @param	BSCENARIO_WORK*	wk	ワーク
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
		// タイトル(キャスト)
		BsCmn_DispStringNumber( wk, BS_CVS_INFO_CAST, 0, 0, mode*1000, 8, 0, 0);
		// 脚本
		BsCmn_DispStringNumber( wk, BS_CVS_INFO_LABEL00, 0, 0, mode*1000, 8, 0, 0);
	}
	// 更新
	//
	// ポケモン情報
	pp = PokeParty_GetMemberPointer( wk->dat->party, wk->PokePageNum );
	//
	// 1回だけ確保
	exp = GFL_STR_CreateBuffer( 128, wk->dat->heap );
	//
	// 名前
	buf[0] = PP_Get( pp, ID_PARA_monsno, 0 );
	str = GFL_MSG_CreateString( wk->MsgData[1], bs_info_tbl[0] );
	WORDSET_RegisterPokeMonsNameNo( wk->Wordset, 0, buf[0] );
	WORDSET_ExpandStr( wk->Wordset, exp, str );
	BsCmn_DispString( wk, BS_CVS_INFO_NAME, 0, 0, exp, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
	GFL_STR_DeleteBuffer( str );
	//
	// 性別
	tmp = 1;
	pcol   = PRINTSYS_LSB_Make(6,5,0);
	buf[0] = PP_GetSex( pp );
	if(buf[0] == PTL_SEX_FEMALE){
		tmp  = 2;
		pcol = PRINTSYS_LSB_Make(4,3,0);
	}
	str = GFL_MSG_CreateString( wk->MsgData[1], bs_info_tbl[tmp] );
	BsCmn_DispString( wk, BS_CVS_INFO_SEX, 0, 0, str, 0, pcol, 0 );
	GFL_STR_DeleteBuffer( str );
	//
	// レベル(要修正)
//	str = GFL_MSG_CreateString( wk->MsgData[1], bs_info_tbl[3] );
//	BsCmn_DispString( wk, BS_CVS_INFO_LEVEL, 0, 0, str, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
//	GFL_STR_DeleteBuffer( str );
	//
	buf[0] = PP_Get( pp, ID_PARA_level, 0 );
	BsCmn_DispStringNumber( wk, BS_CVS_INFO_LEVEL, 32, 0, buf[0], 3, 0, 0);
	//
	// もちもの
	buf[0] = PP_Get( pp, ID_PARA_item, 0 );
	str = GFL_MSG_CreateString( wk->MsgData[1], bs_info_tbl[0] );
	WORDSET_RegisterItemName( wk->Wordset, 0, buf[0] );
	WORDSET_ExpandStr( wk->Wordset, exp, str );
	BsCmn_DispString( wk, BS_CVS_INFO_ITEM, 0, 0, exp, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
	GFL_STR_DeleteBuffer( str );
	//
	// 技
	for(i=0;i<4;i++){
		buf[0] = PP_Get( pp, ID_PARA_waza1+i, 0 );
		if(buf[0] != 0){
			str = GFL_MSG_CreateString( wk->MsgData[1], bs_info_tbl[0] );
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
	buf[0] = PP_Get( pp, ID_PARA_speabino, 0 );
	str = GFL_MSG_CreateString( wk->MsgData[1], bs_info_tbl[0] );
	WORDSET_RegisterTokuseiName( wk->Wordset, 0, buf[0] );
	WORDSET_ExpandStr( wk->Wordset, exp, str );
	BsCmn_DispString( wk, BS_CVS_INFO_STATUS, 0, 0, exp, 0, PRINTSYS_LSB_Make(1,2,0), 0 );
	GFL_STR_DeleteBuffer( str );
	//
	// 破棄
	GFL_STR_DeleteBuffer( exp );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		obj表示
 *
 * @param	BSCENARIO_WORK*	wk	ワーク
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
	
	// ポケモン情報
	pp = PokeParty_GetMemberPointer( wk->dat->party, wk->PokePageNum );
	//
	//--------
	// ポケモン
	//--------
	for(i=0;i<6;i++){
		if(i == wk->PokePageNum){
			buf[0] = PP_Get( pp, ID_PARA_monsno, 0 );			// id
			buf[1] = PP_GetSex( pp );
			BsCmn_ChgTypePoke( wk, i, buf[0], buf[1] );
			BsCmn_ObjVisible( wk, CLWKMODE_POKE, i, TRUE );
		}else{
			BsCmn_ObjVisible( wk, CLWKMODE_POKE, i, FALSE );
		}
	}
	
	//--------
	// タイプ
	//--------
	// type1
	buf[0] = PP_Get( pp, ID_PARA_type1, 0 );
	BsCmn_ChgTypeObj( wk, 0, 0, buf[0] );
	BsCmn_ObjVisible( wk, CLWKMODE_NORMAL, 0, TRUE );
	// type2
	buf[1] = PP_Get( pp, ID_PARA_type2, 0 );
	if(buf[0] != buf[1]){
		BsCmn_ChgTypeObj( wk, 1, 0, buf[1] );
		BsCmn_ObjVisible( wk, CLWKMODE_NORMAL, 1, TRUE );
	}else{
		BsCmn_ObjVisible( wk, CLWKMODE_NORMAL, 1, FALSE );
	}
//	OS_Printf("type :%d,%d\n",buf[0],buf[1]);
	
	//--------
	// タイプ
	//--------
	for(i=0;i<4;i++){
		buf[0] = PP_Get( pp, ID_PARA_waza1+i, 0 );
		if(buf[0] != 0){
			// 技のタイプを取得
			buf[1] = WAZADATA_GetParam( buf[0], WAZAPARAM_TYPE );
			BsCmn_ChgTypeObj( wk, 2+i, 1, buf[1] );
			BsCmn_ObjVisible( wk, CLWKMODE_NORMAL, 2+i, TRUE );
		}else{
			BsCmn_ObjVisible( wk, CLWKMODE_NORMAL, 2+i, FALSE );
		}
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		obj全非表示
 *
 * @param	BSCENARIO_WORK*	wk	ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_hide_info_obj( BSCENARIO_WORK * wk )
{
	s32			i;
	
	// タイプ
	for(i=0;i<2;i++){
		BsCmn_ObjVisible( wk, CLWKMODE_NORMAL, i, FALSE );
	}
	// 技タイプ
	for(i=0;i<4;i++){
		BsCmn_ObjVisible( wk, CLWKMODE_NORMAL, 2+i, FALSE );
	}
	// ポケモン
	for(i=0;i<6;i++){
		BsCmn_ObjVisible( wk, CLWKMODE_POKE, i, FALSE );
	}
}

//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief		処理まとめ
 *
 * @param		wk    ワーク
 *
 * @return		BOOL		(TRUE:制御中、FALSE:終了)
 */
//--------------------------------------------------------------------------------------------
static BOOL bs_main_loop( BSCENARIO_WORK * wk )
{
	BOOL	bflg=TRUE;
	
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
 * @brief		終了処理まとめ
 *
 * @param		wk    ワーク
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

//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief		文字表示テスト(InitStringで確保したらなにか表示すること)
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void bs_disp_test_string( BSCENARIO_WORK * wk, const s32 mode )
{
	s32		i;
	
	for(i=0;i<BSCENARIO_CANVAS_MAXNUM;i++){
		if(wk->PrintUtil[i].win){
			BsCmn_DispStringNumber( wk, i, 0, 0, mode*1000+i, 8, 0, 0);
		}
	}
}

