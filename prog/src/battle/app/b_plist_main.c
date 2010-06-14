//============================================================================================
/**
 * @file    b_plist_main.c
 * @brief   戦闘用ポケモンリスト画面
 * @author  Hiroyuki Nakamura
 * @date    05.02.01
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "item/item.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "system/wipe.h"
#include "sound/pm_sndsys.h"
#include "font/font.naix"
#include "waza_tool/wazadata.h"
#include "poke_tool/gauge_tool.h"
#include "app/app_menu_common.h"

#include "../btlv/btlv_effect.h"

#include "../../field/field_skill_check.h"

#include "msg/msg_b_plist.h"

#include "b_app_tool.h"

#include "b_plist.h"
#include "b_plist_main.h"
#include "b_plist_obj.h"
#include "b_plist_anm.h"
#include "b_plist_ui.h"
#include "b_plist_bmp.h"
#include "b_plist_bmp_def.h"
#include "b_plist_gra.naix"


#include "debug/debug_hudson.h"

//============================================================================================
//  定数定義
//============================================================================================
// メインシーケンス
enum {
  SEQ_BPL_INIT = 0,					// 初期化
  SEQ_BPL_INIT_PRINT_WAIT,	// 初期化時の描画待ち
  SEQ_BPL_INIT_WIPE_WAIT,		// 初期化時のワイプ待ち

  SEQ_BPL_SELECT,				// ポケモン選択画面コントロール
  SEQ_BPL_IREKAE,				// 入れ替えページコントロール
  SEQ_BPL_ST_MAIN,			// ステータスメイン画面コントロール
  SEQ_BPL_ST_WAZASEL,		// ステータス技選択画面コントロール
  SEQ_BPL_ST_SKILL,			// ステータス技画面コントロール

  SEQ_BPL_POKESEL_DEAD,   // 瀕死入れ替え選択コントロール
  SEQ_BPL_POKECHG_DEAD,   // 瀕死入れ替えアニメ
  SEQ_BPL_DEAD_ERR_RET,		// 瀕死入れ替えエラーからの復帰処理

  SEQ_BPL_PAGE1_CHG,					// ポケモン選択へ
  SEQ_BPL_PAGECHG_IREKAE,			// 入れ替えページへ
  SEQ_BPL_PAGECHG_STMAIN,			// ステータス詳細へ
  SEQ_BPL_PAGECHG_WAZASEL,		// ステータス技選択へ
  SEQ_BPL_PAGECHG_WAZAINFO,		// ステータス技詳細画面へ
  SEQ_BPL_PAGECHG_WAZASET_S,	// 技忘れ選択画面へ
  SEQ_BPL_PAGECHG_WAZASET_I,	// 技忘れ決定へ
  SEQ_BPL_PAGECHG_PPRCV,			// 技回復へ
  SEQ_BPL_PAGECHG_DEAD,				// 瀕死入れ替え選択へ

  SEQ_BPL_POKECHG,			// ポケモン切り替え
  SEQ_BPL_CHG_ERR_SET,	// 入れ替えエラーセット
  SEQ_BPL_IREKAE_ERR,		// 入れ替えエラー終了待ち

  SEQ_BPL_MSG_WAIT,		// メッセージ表示
  SEQ_BPL_TRG_WAIT,		// メッセージ表示後のキー待ち

  SEQ_BPL_WAZADEL_SEL,		// 技忘れ選択
  SEQ_BPL_WAZADEL_MAIN,		// 技忘れ決定
  SEQ_BPL_WAZADEL_ERROR,	// 技忘れエラー

  SEQ_BPL_WAZARCV_SEL,		// 技回復選択

  SEQ_BPL_BUTTON_WAIT,		// ボタンアニメ終了待ち

	SEQ_BPL_WAZAINFOMODE_MAIN,	// 技説明モードメイン

  SEQ_BPL_ENDSET,			// 終了フェードセット
  SEQ_BPL_ENDWAIT,		// 終了フェード待ち
  SEQ_BPL_END,				// 終了
};

#define TMP_MSG_BUF_SIZ   ( 512 )   // テンポラリメッセージサイズ

#define PLATE_BG_SX   ( 16 )		// プレートBGサイズX
#define PLATE_BG_SY   ( 6 )			// プレートBGサイズY
#define PLATE_BG_P_PX ( 0 )			// ポケモンがいる場合のプレートBG参照X座標
#define PLATE_BG_P_PY ( 0 )			// ポケモンがいる場合のプレートBG参照Y座標
#define PLATE_BG_N_PX ( 16 )		// ポケモンがいない場合のプレートBG参照X座標
#define PLATE_BG_N_PY ( 13 )		// ポケモンがいない場合のプレートBG参照Y座標

#define PLATE_POKE1_PX  ( 0 )		// ポケモン１のプレートBGX座標
#define PLATE_POKE1_PY  ( 0 )		// ポケモン１のプレートBGY座標
#define PLATE_POKE2_PX  ( 16 )	// ポケモン２のプレートBGX座標
#define PLATE_POKE2_PY  ( 1 )		// ポケモン２のプレートBGY座標
#define PLATE_POKE3_PX  ( 0 )		// ポケモン３のプレートBGX座標
#define PLATE_POKE3_PY  ( 6 )		// ポケモン３のプレートBGY座標
#define PLATE_POKE4_PX  ( 16 )	// ポケモン４のプレートBGX座標
#define PLATE_POKE4_PY  ( 7 )		// ポケモン４のプレートBGY座標
#define PLATE_POKE5_PX  ( 0 )		// ポケモン５のプレートBGX座標
#define PLATE_POKE5_PY  ( 12 )	// ポケモン５のプレートBGY座標
#define PLATE_POKE6_PX  ( 16 )	// ポケモン６のプレートBGX座標
#define PLATE_POKE6_PY  ( 13 )	// ポケモン６のプレートBGY座標

#define P2_EXPGAGE_PX ( 32+14 )	// ページ２の経験値ゲージ書き換えX座標
#define P2_EXPGAGE_PY ( 5 )			// ページ２の経験値ゲージ書き換えY座標
#define P3_EXPGAGE_PX ( 10 )		// ページ３の経験値ゲージ書き換えX座標
#define P3_EXPGAGE_PY ( 8 )			// ページ３の経験値ゲージ書き換えY座標

#define EXP_CGX					( 0x16 )	// 経験値ゲージキャラ番号
#define EXP_DOT_CHR_MAX	( 8 )			// 経験値ゲージのキャラ数
#define EXP_DOT_MAX			( 64 )		// 経験値ゲージの最大ドット数

#define BATTLE_BAGLIST_FADE_SPEED (0)		// フェード速度


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void BattlePokeList_Main( GFL_TCB* tcb, void * work );

static int BPL_SeqInit( BPLIST_WORK * wk );
static int BPL_SeqInitPrintWait( BPLIST_WORK * wk );
static int BPL_SeqInitWipeWait( BPLIST_WORK * wk );
static int BPL_SeqPokeSelect( BPLIST_WORK * wk );
static int BPL_SeqPokeIrekae( BPLIST_WORK * wk );
static int BPL_SeqStatusMain( BPLIST_WORK * wk );
static int BPL_SeqChgErrSet( BPLIST_WORK * wk );
static int BPL_SeqIrekaeErr( BPLIST_WORK * wk );
static int BPL_SeqMsgWait( BPLIST_WORK * wk );
static int BPL_SeqTrgWait( BPLIST_WORK * wk );
static int BPL_SeqPokeChange( BPLIST_WORK * wk );
static int BPL_SeqStInfoWaza( BPLIST_WORK * wk );
static int BPL_SeqWazaDelSelect( BPLIST_WORK * wk );
static int BPL_SeqWazaDelMain( BPLIST_WORK * wk );
static int BPL_SeqWazaDelError( BPLIST_WORK * wk );
static int BPL_SeqWazaRcvSelect( BPLIST_WORK * wk );
static int BPL_SeqButtonWait( BPLIST_WORK * wk );
static int BPL_SeqPage1Chg( BPLIST_WORK * wk );
static int BPL_SeqPageChgIrekae( BPLIST_WORK * wk );
static int BPL_SeqPageChgStWazaSel( BPLIST_WORK * wk );
static int BPL_SeqPageChgStatus( BPLIST_WORK * wk );
static int BPL_SeqPageChgWazaInfo( BPLIST_WORK * wk );
static int BPL_SeqPageChgWazaSetSel( BPLIST_WORK * wk );
static int BPL_SeqPageChgWazaSetEnter( BPLIST_WORK * wk );
static int BPL_SeqPageChgPPRcv( BPLIST_WORK * wk );
static int BPL_SeqEndSet( BPLIST_WORK * wk );
static int BPL_SeqEndWait( BPLIST_WORK * wk );
static BOOL BPL_SeqEnd( GFL_TCB* tcb, BPLIST_WORK * wk );
static int BPL_SeqWazaSelect( BPLIST_WORK * wk );
static int BPL_SeqWazaInfoModeMain( BPLIST_WORK * wk );

static int BPL_PokeItemUse( BPLIST_WORK * wk );

static void BPL_BgInit( BPLIST_WORK * dat );
static void BPL_BgExit(void);
static void BPL_BgGraphicSet( BPLIST_WORK * wk );
static void BPL_MsgManSet( BPLIST_WORK * wk );
static void BPL_MsgManExit( BPLIST_WORK * wk );
static void BPL_PokeDataMake( BPLIST_WORK * wk );

static u8 BPL_PokemonSelect( BPLIST_WORK * wk );
static BOOL BPL_PageChange( BPLIST_WORK * wk, u8 next_page );
static void BPL_PageChgBgScreenChg( BPLIST_WORK * wk, u8 page );
static u8 BPL_IrekaeCheck( BPLIST_WORK * wk );
static u8 BPL_NextPokeGet( BPLIST_WORK * wk, s32 pos, s32 mv );
static void BPL_PokePlateSet( BPLIST_WORK * wk );
static void BPL_ExpGagePut( BPLIST_WORK * wk, u8 page );
static FIELD_SKILL_CHECK_RET BPL_HidenCheck( BPLIST_WORK * wk );
static u8 BPL_TamagoCheck( BPLIST_WORK * wk );

static BOOL FightPokeCheck( BPLIST_WORK * wk, u32 pos );
static BOOL ChangePokeCheck( BPLIST_WORK * wk, u32 pos );

static void CursorMoveSet( BPLIST_WORK * wk, u8 page );

static void InitListRow( BPLIST_WORK * wk );
static void ChangeListRow( BPLIST_WORK * wk, u32 pos1, u32 pos2 );
static BOOL CheckListRowSelect( BPLIST_WORK * wk, u32 pos );
static void SetLog( BPLIST_WORK * wk, u8 pos1, u8 pos2 );
static int BPL_SeqPageChgDead( BPLIST_WORK * wk );
static int BPL_SeqPokeSelDead( BPLIST_WORK * wk );
static int BPL_SeqPokeChgDead( BPLIST_WORK * wk );
static int BPL_SeqDeadErrRet( BPLIST_WORK * wk );
static BOOL CheckDeadPoke( BPLIST_WORK * wk );
static void SetDeadChangeData( BPLIST_WORK * wk );
static BOOL CheckNextDeadSel( BPLIST_WORK * wk );
static void SetSelPosCancel( BPLIST_WORK * wk );
static BOOL CheckTimeOut( BPLIST_WORK * wk );
static void PlaySE( BPLIST_WORK * wk, int no );
static void PlaySystemSE( BPLIST_WORK * wk, int no );


//============================================================================================
//  グローバル変数
//============================================================================================

// メインシーケンス
static const pBPlistFunc MainSeqFunc[] = {
  BPL_SeqInit,
	BPL_SeqInitPrintWait,
	BPL_SeqInitWipeWait,

  BPL_SeqPokeSelect,
  BPL_SeqPokeIrekae,
  BPL_SeqStatusMain,
  BPL_SeqWazaSelect,
  BPL_SeqStInfoWaza,

  BPL_SeqPokeSelDead,
  BPL_SeqPokeChgDead,
  BPL_SeqDeadErrRet,

  BPL_SeqPage1Chg,
  BPL_SeqPageChgIrekae,
  BPL_SeqPageChgStatus,
  BPL_SeqPageChgStWazaSel,
  BPL_SeqPageChgWazaInfo,
  BPL_SeqPageChgWazaSetSel,
  BPL_SeqPageChgWazaSetEnter,
  BPL_SeqPageChgPPRcv,
  BPL_SeqPageChgDead,

  BPL_SeqPokeChange,
  BPL_SeqChgErrSet,
  BPL_SeqIrekaeErr,

  BPL_SeqMsgWait,
  BPL_SeqTrgWait,

  BPL_SeqWazaDelSelect,
  BPL_SeqWazaDelMain,
  BPL_SeqWazaDelError,

  BPL_SeqWazaRcvSelect,

  BPL_SeqButtonWait,

	BPL_SeqWazaInfoModeMain,

  BPL_SeqEndSet,
  BPL_SeqEndWait,
};

// プレート表示位置
static const u8 PlatePos[][2] =
{
  { PLATE_POKE1_PX, PLATE_POKE1_PY },
  { PLATE_POKE2_PX, PLATE_POKE2_PY },
  { PLATE_POKE3_PX, PLATE_POKE3_PY },
  { PLATE_POKE4_PX, PLATE_POKE4_PY },
  { PLATE_POKE5_PX, PLATE_POKE5_PY },
  { PLATE_POKE6_PX, PLATE_POKE6_PY }
};

// 各ページのスクリーンファイル
static const u32 ScreenArc[][2] =
{ // ボタンBGスクリーン, 背景BGスクリーン
  { NARC_b_plist_gra_poke_sel_t_lz_NSCR, NARC_b_plist_gra_poke_sel_u_lz_NSCR },							// ポケモン選択ページ
  { NARC_b_plist_gra_poke_chg_t_lz_NSCR, NARC_b_plist_gra_poke_chg_u_lz_NSCR },							// ポケモン入れ替えページ
  { NARC_b_plist_gra_st_main_t_lz_NSCR, NARC_b_plist_gra_st_main_u_lz_NSCR },								// ステータスメインページ
  { NARC_b_plist_gra_st_waza_sel_t_lz_NSCR, NARC_b_plist_gra_st_waza_sel_u_lz_NSCR },				// ステータス詳細ページ
  { NARC_b_plist_gra_st_waza_main_t_lz_NSCR, NARC_b_plist_gra_st_waza_main_u_lz_NSCR },			// ステータス技ページ
  { NARC_b_plist_gra_item_waza_t_lz_NSCR, NARC_b_plist_gra_item_waza_u_lz_NSCR },						// PP回復技選択ページ
  { NARC_b_plist_gra_waza_delb_sel_t_lz_NSCR, NARC_b_plist_gra_waza_delb_sel_u_lz_NSCR },		// ステータス技忘れ１ページ（戦闘技選択）
  { NARC_b_plist_gra_waza_delb_main_t_lz_NSCR, NARC_b_plist_gra_waza_delb_main_u_lz_NSCR },	// ステータス技忘れ２ページ（戦闘技詳細）
  { NARC_b_plist_gra_poke_sel_t_lz_NSCR, NARC_b_plist_gra_poke_sel_u_lz_NSCR },							// 瀕死入れ替え選択ページ
};

// マルチバトル時のリストの並び順
static const u8 InitListRowMulti[2][6] =
{
  { 0, 3, 1, 4, 2, 5 },		// 立ち位置・左
  { 3, 0, 4, 1, 5, 2 },		// 立ち位置・右
};


//--------------------------------------------------------------------------------------------
/**
 * @brief		戦闘用ポケモンリストタスク追加
 *
 * @param		dat   リストデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BattlePokeList_TaskAdd( BPLIST_DATA * dat )
{
  BPLIST_WORK * wk;

  if( dat->sel_poke > 5 ){ dat->sel_poke = 0; }

	// 技説明モード以外のとき
	if( dat->mode != BPL_MODE_WAZAINFO ){
		dat->sel_wp = 0;
	}

  wk = GFL_HEAP_AllocClearMemory( dat->heap, sizeof(BPLIST_WORK) );

  GFL_TCB_AddTask( dat->tcb_sys, BattlePokeList_Main, wk, 100 );

  wk->dat = dat;
  wk->pfd = dat->pfd;
  wk->seq = SEQ_BPL_INIT;
  wk->init_poke = dat->sel_poke;
	wk->initFlag  = FALSE;

//  wk->multi_pos = BattleWorkClientTypeGet( dat->bw, dat->client_no );
//  wk->multi_pos = 0;
// CLIENT_TYPE_A : 前衛
// CLIENT_TYPE_C : 後衛

/*** テスト ***/
//  wk->dat->mode = BPL_MODE_NORMAL;      // 通常のポケモン選択
//  wk->dat->mode = BPL_MODE_ITEMUSE;     // アイテム使用
//  wk->dat->item = 38;


//  wk->dat->mode = BPL_MODE_WAZASET;     // 技忘れ
//  wk->dat->sel_poke = 0;
//  wk->dat->chg_waza = 20;
//  wk->page = BPLIST_PAGE_PP_RCV;    // PP回復技選択ページ


//  wk->dat->mode = BPL_MODE_CHG_DEAD;   // 瀕死入れ替え時
//  wk->dat->rule = BTL_RULE_SINGLE;
//  wk->dat->rule = BTL_RULE_DOUBLE;
//  wk->dat->rule = BTL_RULE_TRIPLE;

/*
  // マルチ
  wk->dat->multi_pp = wk->dat->pp;
  wk->dat->multiMode = TRUE;
  wk->dat->multiPos = 0;
*/

/**************/
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
static void BattlePokeList_Main( GFL_TCB* tcb, void * work )
{
  BPLIST_WORK * wk = (BPLIST_WORK *)work;

	if( wk->dat->comm_err_flg == TRUE ){
    SetSelPosCancel( wk );
		wk->seq = SEQ_BPL_END;
	}

  if( wk->seq != SEQ_BPL_END ){
    wk->seq = MainSeqFunc[wk->seq]( wk );
  }

  if( wk->seq == SEQ_BPL_END ){
    if( BPL_SeqEnd( tcb, wk ) == TRUE ){
      return;
    }
  }

  GFL_TCBL_Main( wk->tcbl );
  BPL_PokeIconAnime( wk );

  BattlePokeList_ButtonAnmMain( wk );
  BPLISTBMP_PrintMain( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		初期化シーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqInit( BPLIST_WORK * wk )
{
  G2S_BlendNone();
	GXS_SetMasterBrightness( -16 );

  wk->tcbl = GFL_TCBL_Init( wk->dat->heap, wk->dat->heap, 1, 4 );

//	wk->dat->mode = BPL_MODE_WAZAINFO;
//	wk->dat->sel_wp = 3;

	// 技忘れ
  if( wk->dat->mode == BPL_MODE_WAZASET ){
    wk->page = BPLIST_PAGE_WAZASET_BS;
	// 技説明
	}else if( wk->dat->mode == BPL_MODE_WAZAINFO ){
    wk->page = BPLIST_PAGE_SKILL;
	// その他
  }else{
    wk->page = BPLIST_PAGE_SELECT;
  }

  wk->cpwk = BAPPTOOL_CreateCursor( wk->dat->heap );

  InitListRow( wk );

  BPL_PokeDataMake( wk );

  BPL_BgInit( wk );
  BPL_BgGraphicSet( wk );
  BPL_MsgManSet( wk );

  BPL_PageChgBgScreenChg( wk, wk->page );
  BattlePokeList_ButtonPageScreenInit( wk, wk->page );
  BattlePokeList_ButtonPalSet( wk, wk->page );

  BattlePokeList_ObjInit( wk );
	// 技説明の場合は技のタイプアイコンを初期化
	if( wk->dat->mode == BPL_MODE_WAZAINFO ){
		BattlePokelist_WazaTypeSet( wk );
	}
  BattlePokeList_PageObjSet( wk, wk->page );

  BattlePokeList_BmpInit( wk );
  BattlePokeList_BmpWrite( wk, wk->page );
  BPLISTBMP_SetStrScrn( wk );

  if( *wk->dat->cursor_flg == 1 ){
    wk->cursor_flg = TRUE;
  }else{
    wk->cursor_flg = FALSE;
  }
  BAPPTOOL_VanishCursor( wk->cpwk, wk->cursor_flg );

  // マルチのときの初期位置補正
  if( wk->page == BPLIST_PAGE_SELECT ){
    if( BattlePokeList_MultiPosCheck( wk, BPLISTMAIN_GetListRow(wk,0) ) == TRUE ){
      wk->dat->sel_poke = 1;
    }
    BPLISTUI_Init( wk, wk->page, wk->dat->sel_poke );
  }else{
		// 技選択モードのとき
		if( wk->dat->mode == BPL_MODE_WAZAINFO ){
	    BPLISTUI_Init( wk, wk->page, wk->dat->sel_wp );
		}else{
	    BPLISTUI_Init( wk, wk->page, 0 );
		}
  }

  BPL_ExpGagePut( wk, wk->page );

  PaletteFadeReq(
    wk->pfd, PF_BIT_SUB_ALL, 0xffff, BATTLE_BAGLIST_FADE_SPEED, 16, 0, 0, wk->dat->tcb_sys );

	wk->initFlag = TRUE;

	return SEQ_BPL_INIT_PRINT_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		画面初期化時の描画待ち
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqInitPrintWait( BPLIST_WORK * wk )
{
  if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
		WIPE_SYS_Start(
			WIPE_PATTERN_S, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
			WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->dat->heap );
		return SEQ_BPL_INIT_WIPE_WAIT;
	}
	return SEQ_BPL_INIT_PRINT_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		画面初期化時のワイプ待ち
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqInitWipeWait( BPLIST_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		// 技忘れ
	  if( wk->dat->mode == BPL_MODE_WAZASET ){
	    return SEQ_BPL_WAZADEL_SEL;
		// 技説明
		}else if( wk->dat->mode == BPL_MODE_WAZAINFO ){
	    return SEQ_BPL_WAZAINFOMODE_MAIN;
		// その他
	  }else{
	    return SEQ_BPL_SELECT;
	  }
	}
	return SEQ_BPL_INIT_WIPE_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン選択ページのコントロールシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPokeSelect( BPLIST_WORK * wk )
{
  if( PaletteFadeCheck( wk->pfd ) != 0 ){ return SEQ_BPL_SELECT; }

  if( CheckTimeOut( wk ) == TRUE ){
    return SEQ_BPL_ENDSET;
  }

  if( BPL_PokemonSelect( wk ) == TRUE ){
    if( wk->dat->sel_poke == BPL_SEL_EXIT ){
      // 瀕死いれかえ時
      if( wk->dat->mode == BPL_MODE_CHG_DEAD ){
        u8  pos1, pos2;
        if( BPLISTMAIN_GetNewLog( wk, &pos1, &pos2, TRUE ) == TRUE ){
          if( pos1 < pos2 ){
            GF_ASSERT_MSG( pos1 < BPL_SELNUM_MAX, "pos1 = %d\n", pos1 );
            wk->dat->sel_pos[pos1] = BPL_SELPOS_NONE;
          }else{
            GF_ASSERT_MSG( pos2 < BPL_SELNUM_MAX, "pos2 = %d\n", pos2 );
            wk->dat->sel_pos[pos2] = BPL_SELPOS_NONE;
          }
          wk->chg_pos1 = pos1;
          wk->chg_pos2 = pos2;
          wk->btn_seq = 0;
          PlaySE( wk, SEQ_SE_CANCEL2 );
          return SEQ_BPL_POKECHG_DEAD;
        }
      // キャンセル無効以外
      }else if( wk->dat->mode != BPL_MODE_NO_CANCEL ){
        PlaySE( wk, SEQ_SE_CANCEL2 );
        BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
        SetSelPosCancel( wk );
        return SEQ_BPL_ENDSET;
      }
    }else{
      PlaySE( wk, SEQ_SE_DECIDE2 );
      BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_POKE1+wk->dat->sel_poke );
      if( wk->dat->mode == BPL_MODE_ITEMUSE ){
        return BPL_PokeItemUse( wk );
      }else{
        wk->ret_seq = SEQ_BPL_PAGECHG_IREKAE;
      }
      return SEQ_BPL_BUTTON_WAIT;
    }
  }

  return SEQ_BPL_SELECT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		アイテム使用
 *
 * @param		wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_PokeItemUse( BPLIST_WORK * wk )
{
  BPLIST_DATA * dat = wk->dat;

  // タマゴには使えない
  if( wk->poke[ BPLISTMAIN_GetListRow(wk,dat->sel_poke) ].egg != 0 ){
    GFL_MSG_GetString( wk->mman, mes_b_plist_m06, wk->msg_buf );
    BattlePokeList_TalkMsgSet( wk );
    wk->dat->sel_poke = BPL_SEL_EXIT;
    wk->ret_seq = SEQ_BPL_ENDSET;
    SetSelPosCancel( wk );
    return SEQ_BPL_MSG_WAIT;
  }

  // さしおさえで使用不可
  if( dat->skill_item_use[ BPLISTMAIN_GetListRow(wk,dat->sel_poke) ] != 0 ){
    BattlePokeList_ItemUseSkillErrMsgSet( wk );
    BattlePokeList_TalkMsgSet( wk );
    wk->dat->sel_poke = BPL_SEL_EXIT;
    wk->ret_seq = SEQ_BPL_ENDSET;
    SetSelPosCancel( wk );
    return SEQ_BPL_MSG_WAIT;
  }

  // １つの技のPP回復
  if( ITEM_GetParam( dat->item, ITEM_PRM_PP_RCV, dat->heap ) != 0 &&
      ITEM_GetParam( dat->item, ITEM_PRM_ALL_PP_RCV, dat->heap ) == 0 ){
    wk->ret_seq = SEQ_BPL_PAGECHG_PPRCV;
    return SEQ_BPL_BUTTON_WAIT;
  }

  dat->sel_pos[0] = BPLISTMAIN_GetListRow( wk, dat->sel_poke );

  return SEQ_BPL_ENDSET;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		入れ替えページのコントロールシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPokeIrekae( BPLIST_WORK * wk )
{
  u32 ret;

  if( CheckTimeOut( wk ) == TRUE ){
    return SEQ_BPL_ENDSET;
  }

  ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_CHG_ENTER:       // いれかえる
    PlaySE( wk, SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_POKE_CHG );
    if( BPL_IrekaeCheck( wk ) == TRUE ){
      // 通常の入れ替え
      if( wk->dat->mode != BPL_MODE_CHG_DEAD ){
        wk->dat->sel_pos[wk->dat->sel_pos_index] = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
				OS_Printf( "wk->dat->sel_pos[%d] = %d\n", wk->dat->sel_pos_index, wk->dat->sel_pos[wk->dat->sel_pos_index] );
        return SEQ_BPL_ENDSET;
      }
      // 瀕死時の入れ替え
      if( CheckDeadPoke( wk ) == FALSE ){
        // 即戻りなら、ここでワークをセットして終了
        SetDeadChangeData( wk );
        return SEQ_BPL_ENDSET;
      }else{
        wk->ret_seq = SEQ_BPL_PAGECHG_DEAD;
        return SEQ_BPL_BUTTON_WAIT;
      }
    }
    wk->ret_seq = SEQ_BPL_CHG_ERR_SET;
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_CHG_STATUS:      // つよさをみる
    if( BPL_TamagoCheck( wk ) == TRUE ){ break; }
    PlaySE( wk, SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_STATUS1 );
    wk->ret_seq = SEQ_BPL_PAGECHG_STMAIN;
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_CHG_WAZA:        // わざをみる
    if( BPL_TamagoCheck( wk ) == TRUE ){ break; }
    PlaySE( wk, SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZASEL1 );
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASEL;
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_CHG_RETURN:      // もどる
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->ret_seq = SEQ_BPL_PAGE1_CHG;
    return SEQ_BPL_BUTTON_WAIT;

  case CURSORMOVE_CANCEL:         // キャンセル
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->ret_seq = SEQ_BPL_PAGE1_CHG;
    return SEQ_BPL_BUTTON_WAIT;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
    PlaySE( wk, SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_NONE:           // 動作なし
    break;
  }

  return SEQ_BPL_IREKAE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ステータスメインページのコントロールシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqStatusMain( BPLIST_WORK * wk )
{
  u32 ret;

  if( CheckTimeOut( wk ) == TRUE ){
    return SEQ_BPL_ENDSET;
  }

  ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_STATUS_UP:       // 前のポケモンへ
    {
      u8 pos = BPL_NextPokeGet( wk, wk->dat->sel_poke, -1 );
      if( pos == 0xff ){ break; }
      wk->dat->sel_poke = pos;
    }
    PlaySE( wk, SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_UP );
    wk->ret_seq = SEQ_BPL_POKECHG;
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_STATUS_DOWN:     // 次のポケモンへ
    {
      u8 pos = BPL_NextPokeGet( wk, wk->dat->sel_poke, 1 );
      if( pos == 0xff ){ break; }
      wk->dat->sel_poke = pos;
    }
    PlaySE( wk, SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_DOWN );
    wk->ret_seq = SEQ_BPL_POKECHG;
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_STATUS_WAZA:     // わざをみる
    PlaySE( wk, SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZASEL2 );
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASEL;
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_STATUS_RETURN:   // もどる
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->chg_page_cp = 1;    // 入れ替えページのカーソル位置を「強さを見る」へ
    wk->ret_seq = SEQ_BPL_PAGECHG_IREKAE;
    return SEQ_BPL_BUTTON_WAIT;

  case CURSORMOVE_CANCEL:         // キャンセル
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->chg_page_cp = 1;    // 入れ替えページのカーソル位置を「強さを見る」へ
    wk->ret_seq = SEQ_BPL_PAGECHG_IREKAE;
    return SEQ_BPL_BUTTON_WAIT;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
    PlaySE( wk, SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_NONE:           // 動作なし
    break;
  }

  return SEQ_BPL_ST_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ステータス技選択ページのコントロールシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqWazaSelect( BPLIST_WORK * wk )
{
  u32 ret;

  if( CheckTimeOut( wk ) == TRUE ){
    return SEQ_BPL_ENDSET;
  }

  ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_WAZA_SEL1:     // 技１
  case BPLIST_UI_WAZA_SEL2:     // 技２
  case BPLIST_UI_WAZA_SEL3:     // 技３
  case BPLIST_UI_WAZA_SEL4:     // 技４
    if( wk->poke[ BPLISTMAIN_GetListRow(wk,wk->dat->sel_poke) ].waza[ret].id == 0 ){
      break;
    }
    PlaySE( wk, SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZA1+ret );
    wk->dat->sel_wp = ret;
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZAINFO;
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_WAZA_UP:       // 前のポケモンへ
    {
      u8 pos = BPL_NextPokeGet( wk, wk->dat->sel_poke, -1 );
      if( pos == 0xff ){ break; }
      wk->dat->sel_poke = pos;
    }
    PlaySE( wk, SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_UP );
    wk->ret_seq = SEQ_BPL_POKECHG;
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_WAZA_DOWN:     // 次のポケモンへ
    {
      u8 pos = BPL_NextPokeGet( wk, wk->dat->sel_poke, 1 );
      if( pos == 0xff ){ break; }
      wk->dat->sel_poke = pos;
    }
    PlaySE( wk, SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_DOWN );
    wk->ret_seq = SEQ_BPL_POKECHG;
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_WAZA_STATUS:   // つよさをみる
    PlaySE( wk, SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_STATUS2 );
    wk->ret_seq = SEQ_BPL_PAGECHG_STMAIN;
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_WAZA_RETURN:   // もどる
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->chg_page_cp = 2;    // 入れ替えページのカーソル位置を「技を見る」へ
    wk->ret_seq = SEQ_BPL_PAGECHG_IREKAE;
    return SEQ_BPL_BUTTON_WAIT;

  case CURSORMOVE_CANCEL:       // キャンセル
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->chg_page_cp = 2;    // 入れ替えページのカーソル位置を「技を見る」へ
    wk->ret_seq = SEQ_BPL_PAGECHG_IREKAE;
    return SEQ_BPL_BUTTON_WAIT;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
    PlaySE( wk, SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_NONE:           // 動作なし
    break;
  }

  return SEQ_BPL_ST_WAZASEL;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ステータス技詳細ページのコントロールシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqStInfoWaza( BPLIST_WORK * wk )
{
  u32 ret;

  if( CheckTimeOut( wk ) == TRUE ){
    return SEQ_BPL_ENDSET;
  }

  ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_WAZAINFO_SEL1:     // 技１
  case BPLIST_UI_WAZAINFO_SEL2:     // 技２
  case BPLIST_UI_WAZAINFO_SEL3:     // 技３
  case BPLIST_UI_WAZAINFO_SEL4:     // 技４
    if( wk->dat->sel_wp == ret ||
      wk->poke[ BPLISTMAIN_GetListRow(wk,wk->dat->sel_poke) ].waza[ret].id == 0 ){
      break;
    }
    PlaySE( wk, SEQ_SE_DECIDE2 );
    wk->dat->sel_wp = ret;
    return SEQ_BPL_PAGECHG_WAZAINFO;

  case BPLIST_UI_WAZAINFO_RETURN:   // もどる
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASEL;
    return SEQ_BPL_BUTTON_WAIT;

  case CURSORMOVE_CANCEL:         // キャンセル
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASEL;
    return SEQ_BPL_BUTTON_WAIT;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
    PlaySE( wk, SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_NONE:           // 動作なし
    break;
  }

  return SEQ_BPL_ST_SKILL;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		技説明モードメイン処理
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqWazaInfoModeMain( BPLIST_WORK * wk )
{
  u32 ret;

  if( PaletteFadeCheck( wk->pfd ) != 0 ){ return SEQ_BPL_WAZAINFOMODE_MAIN; }

  if( CheckTimeOut( wk ) == TRUE ){
    return SEQ_BPL_ENDSET;
  }

  ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_WAZAINFO_SEL1:     // 技１
  case BPLIST_UI_WAZAINFO_SEL2:     // 技２
  case BPLIST_UI_WAZAINFO_SEL3:     // 技３
  case BPLIST_UI_WAZAINFO_SEL4:     // 技４
    if( wk->dat->sel_wp == ret ||
      wk->poke[ BPLISTMAIN_GetListRow(wk,wk->dat->sel_poke) ].waza[ret].id == 0 ){
      break;
    }
    PlaySE( wk, SEQ_SE_DECIDE2 );
    wk->dat->sel_wp = ret;
    return SEQ_BPL_PAGECHG_WAZAINFO;

  case BPLIST_UI_WAZAINFO_RETURN:   // もどる
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    return SEQ_BPL_ENDSET;

  case CURSORMOVE_CANCEL:         // キャンセル
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    return SEQ_BPL_ENDSET;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
    PlaySE( wk, SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_NONE:           // 動作なし
    break;
  }

  return SEQ_BPL_WAZAINFOMODE_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ５のコントロールシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqWazaDelSelect( BPLIST_WORK * wk )
{
  u32 ret;

  if( PaletteFadeCheck( wk->pfd ) != 0 ){ return SEQ_BPL_WAZADEL_SEL; }

	ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_WAZADEL_SEL1:    // 技１
  case BPLIST_UI_WAZADEL_SEL2:    // 技２
  case BPLIST_UI_WAZADEL_SEL3:    // 技３
  case BPLIST_UI_WAZADEL_SEL4:    // 技４
  case BPLIST_UI_WAZADEL_SEL5:    // 技５
    wk->dat->sel_wp = (u8)ret;
    wk->wws_page_cp = (u8)ret;
    PlaySE( wk, SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZADEL1+ret );
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASET_I;
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_WAZADEL_RETURN:  // もどる
    PlaySE( wk, SEQ_SE_CANCEL2 );
    wk->dat->sel_wp = BPL_SEL_WP_CANCEL;
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
		return SEQ_BPL_ENDSET;

  case CURSORMOVE_CANCEL:         // キャンセル
    PlaySE( wk, SEQ_SE_CANCEL2 );
    wk->dat->sel_wp = BPL_SEL_WP_CANCEL;
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
		return SEQ_BPL_ENDSET;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
    PlaySE( wk, SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_NONE:           // 動作なし
    break;
  }

  return SEQ_BPL_WAZADEL_SEL;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ６のコントロールシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqWazaDelMain( BPLIST_WORK * wk )
{
  u32 ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_DELINFO_ENTER:   // わすれる
    PlaySE( wk, SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZADEL_B );
    // 忘れられる
    if( BPL_HidenCheck( wk ) == FSCR_OK ){
      wk->ret_seq = SEQ_BPL_ENDSET;
    // 忘れられない
    }else{
      wk->ret_seq = SEQ_BPL_WAZADEL_ERROR;
    }
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_DELINFO_RETURN:  // もどる
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->wwm_page_cp = 0;
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASET_S;
    return SEQ_BPL_BUTTON_WAIT;

  case CURSORMOVE_CANCEL:         // キャンセル
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->wwm_page_cp = 0;
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASET_S;
    return SEQ_BPL_BUTTON_WAIT;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
    PlaySE( wk, SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_NONE:           // 動作なし
    break;
  }

  return SEQ_BPL_WAZADEL_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		技忘れエラー処理
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqWazaDelError( BPLIST_WORK * wk )
{
	BPL_HidenMsgPut( wk );
  wk->talk_win_clear = 1;
  wk->ret_seq = SEQ_BPL_WAZADEL_MAIN;
  return SEQ_BPL_MSG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		技回復選択ページのコントロールシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqWazaRcvSelect( BPLIST_WORK * wk )
{
  BPLIST_DATA * dat;
  u32 ret;

  dat = wk->dat;
  ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_PPRCV_WAZA1:     // 技１
  case BPLIST_UI_PPRCV_WAZA2:     // 技２
  case BPLIST_UI_PPRCV_WAZA3:     // 技３
  case BPLIST_UI_PPRCV_WAZA4:     // 技４
    {
      u8  pos = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
      if( wk->poke[pos].waza[ret].id == 0 ){ break; }
      wk->dat->sel_wp = (u8)ret;
      wk->dat->sel_pos[0] = pos;
      PlaySE( wk, SEQ_SE_DECIDE2 );
      BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZARCV1+ret );
      return SEQ_BPL_ENDSET;
    }

  case BPLIST_UI_PPRCV_RETURN:    // もどる
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->ret_seq = SEQ_BPL_PAGE1_CHG;
    return SEQ_BPL_BUTTON_WAIT;

  case CURSORMOVE_CANCEL:         // キャンセル
    PlaySE( wk, SEQ_SE_CANCEL2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->ret_seq = SEQ_BPL_PAGE1_CHG;
    return SEQ_BPL_BUTTON_WAIT;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
    PlaySE( wk, SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_NONE:           // 動作なし
    break;
  }

  return SEQ_BPL_WAZARCV_SEL;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え：ポケモン選択ページへ
 *
 * @param		wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPage1Chg( BPLIST_WORK * wk )
{
  if( BPL_PageChange( wk, BPLIST_PAGE_SELECT ) == FALSE ){
    return SEQ_BPL_PAGE1_CHG;
  }
  return SEQ_BPL_SELECT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え：ポケモン入れ替えページへ
 *
 * @param		wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPageChgIrekae( BPLIST_WORK * wk )
{
  if( BPL_PageChange( wk, BPLIST_PAGE_POKE_CHG ) == FALSE ){
    return SEQ_BPL_PAGECHG_IREKAE;
  }
  return SEQ_BPL_IREKAE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え：ステータスメインページへ
 *
 * @param		wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPageChgStatus( BPLIST_WORK * wk )
{
  if( BPL_PageChange( wk, BPLIST_PAGE_MAIN ) == FALSE ){
    return SEQ_BPL_PAGECHG_STMAIN;
  }
  return SEQ_BPL_ST_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え：ステータス技選択ページへ
 *
 * @param		wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPageChgStWazaSel( BPLIST_WORK * wk )
{
  if( BPL_PageChange( wk, BPLIST_PAGE_WAZA_SEL ) == FALSE ){
    return SEQ_BPL_PAGECHG_WAZASEL;
  }
  return SEQ_BPL_ST_WAZASEL;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え：技詳細ページへ
 *
 * @param		wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPageChgWazaInfo( BPLIST_WORK * wk )
{
  if( BPL_PageChange( wk, BPLIST_PAGE_SKILL ) == FALSE ){
    return SEQ_BPL_PAGECHG_WAZAINFO;
  }
	// 技説明モードの場合
	if( wk->dat->mode == BPL_MODE_WAZAINFO ){
	  return SEQ_BPL_WAZAINFOMODE_MAIN;
	}
  return SEQ_BPL_ST_SKILL;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え：技忘れページへ
 *
 * @param		wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPageChgWazaSetSel( BPLIST_WORK * wk )
{
  if( BPL_PageChange( wk, BPLIST_PAGE_WAZASET_BS ) == FALSE ){
    return SEQ_BPL_PAGECHG_WAZASET_S;
  }
  return SEQ_BPL_WAZADEL_SEL;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え：技忘れ詳細ページへ
 *
 * @param		wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPageChgWazaSetEnter( BPLIST_WORK * wk )
{
  if( BPL_PageChange( wk, BPLIST_PAGE_WAZASET_BI ) == FALSE ){
    return SEQ_BPL_PAGECHG_WAZASET_I;
  }
  return SEQ_BPL_WAZADEL_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え：技回復ページへ
 *
 * @param		wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPageChgPPRcv( BPLIST_WORK * wk )
{
  if( BPL_PageChange( wk, BPLIST_PAGE_PP_RCV ) == FALSE ){
    return SEQ_BPL_PAGECHG_PPRCV;
  }
  return SEQ_BPL_WAZARCV_SEL;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンきりかえシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPokeChange( BPLIST_WORK * wk )
{
  BattlePokeList_PageObjSet( wk, wk->page );
  BattlePokeList_BmpWrite( wk, wk->page );
  BattlePokeList_ButtonPageScreenInit( wk, wk->page );
  BPL_ExpGagePut( wk, wk->page );
  if( wk->page == BPLIST_PAGE_MAIN ){
    return SEQ_BPL_ST_MAIN;
  }
  return SEQ_BPL_ST_WAZASEL;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		入れ替えエラーセット
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqChgErrSet( BPLIST_WORK * wk )
{
  BattlePokeList_TalkMsgSet( wk );
  wk->ret_seq = SEQ_BPL_IREKAE_ERR;
  return SEQ_BPL_MSG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		入れ替えエラーメッセージ終了待ちシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqIrekaeErr( BPLIST_WORK * wk )
{
  BmpWinFrame_Clear( wk->win[WIN_TALK].win, WINDOW_TRANS_ON );
  return SEQ_BPL_IREKAE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ表示シーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqMsgWait( BPLIST_WORK * wk )
{
  switch( PRINTSYS_PrintStreamGetState(wk->stream) ){
  case PRINTSTREAM_STATE_RUNNING:	// 実行中
    if( GFL_UI_TP_GetTrg() == TRUE || (GFL_UI_KEY_GetCont() & (PAD_BUTTON_A|PAD_BUTTON_B)) ){
      PRINTSYS_PrintStreamShortWait( wk->stream, 0 );
    }
    wk->stream_clear_flg = FALSE;
    break;

	case PRINTSTREAM_STATE_PAUSE:		// 一時停止中
    if( wk->stream_clear_flg == FALSE ){
      if( GFL_UI_TP_GetTrg() == TRUE || (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B)) ){
        PlaySystemSE( wk, SEQ_SE_MESSAGE );
        PRINTSYS_PrintStreamReleasePause( wk->stream );
        wk->stream_clear_flg = TRUE;
      }
    }
    break;

  case PRINTSTREAM_STATE_DONE:		// 終了
    PRINTSYS_PrintStreamDelete( wk->stream );
		wk->stream = NULL;
    wk->stream_clear_flg = FALSE;
    return SEQ_BPL_TRG_WAIT;
  }

  return SEQ_BPL_MSG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ表示後のキー待ちシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqTrgWait( BPLIST_WORK * wk )
{
  if( CheckTimeOut( wk ) == TRUE ){
    return SEQ_BPL_ENDSET;
  }

  if( ( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ) || GFL_UI_TP_GetTrg() == TRUE ){
    if( wk->talk_win_clear == 1 ){
      BmpWinFrame_Clear( wk->win[WIN_TALK].win, WINDOW_TRANS_ON );
      wk->talk_win_clear = 0;
    }
    return wk->ret_seq;
  }
  return SEQ_BPL_TRG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボタンアニメ終了待ちシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqButtonWait( BPLIST_WORK * wk )
{
  if( wk->btn_flg == 0 ){
    return wk->ret_seq;
  }
  return SEQ_BPL_BUTTON_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		終了フェードセットシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqEndSet( BPLIST_WORK * wk )
{
  // マルチモード時の選択位置補正
  if( BattlePokeList_MultiCheck( wk ) == TRUE ){
		if( wk->dat->sel_poke != BPL_SEL_EXIT ){
	    wk->dat->sel_poke /= 2;
		}
  }

  PaletteFadeReq(
    wk->pfd, PF_BIT_SUB_ALL, 0xffff, BATTLE_BAGLIST_FADE_SPEED, 0, 16, 0, wk->dat->tcb_sys );
  return SEQ_BPL_ENDWAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		終了フェード待ちシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqEndWait( BPLIST_WORK * wk )
{
  if( PaletteFadeCheck( wk->pfd ) == 0 ){
    return SEQ_BPL_END;
  }
  return SEQ_BPL_ENDWAIT;
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
static BOOL BPL_SeqEnd( GFL_TCB * tcb, BPLIST_WORK * wk )
{
	if( wk->initFlag == TRUE ){
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
		if( wk->chg_wfrm != NULL ){
			BGWINFRM_Exit( wk->chg_wfrm );
		}

	  BPL_MsgManExit( wk );
	  BattlePokeList_ObjFree( wk );
	  BattlePokeList_BmpFreeAll( wk );
	  BPL_BgExit();

	  GFL_TCBL_Exit( wk->tcbl );

	  BPLISTUI_Exit( wk );

	  BAPPTOOL_FreeCursor( wk->cpwk );

	  if( wk->cursor_flg == TRUE ){
	    *wk->dat->cursor_flg = 1;
	  }else{
	    *wk->dat->cursor_flg = 0;
	  }
	}

  wk->dat->end_flg = 1;

  GFL_TCB_DeleteTask( tcb );
  GFL_HEAP_FreeMemory( wk );

  return TRUE;
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
static void BPL_BgInit( BPLIST_WORK * wk )
{
  { // BG SYSTEM
    GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGModeDisp( &BGsys_data, GFL_BG_SUB_DISP );
  }

  { // BG (CHAR)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
  }

  { // BG2 (CHAR)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000, 0x8000,
      GX_BG_EXTPLTT_23, 2, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
  }

  { // PARAM FONT (BMP)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x10000, 0x8000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( GFL_BG_FRAME1_S );
  }

  { // WINDOW (BMP)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x18000, 0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( GFL_BG_FRAME0_S );
  }
  GFL_BG_SetClearCharacter( GFL_BG_FRAME1_S, 32, 0, wk->dat->heap );
  GFL_BG_SetClearCharacter( GFL_BG_FRAME0_S, 32, 0, wk->dat->heap );
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_S );
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_S );

  GFL_DISP_GXS_SetVisibleControl(
    GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 |
    GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3, VISIBLE_ON );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BG解放
 *
 * @param		none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_BgExit(void)
{
  GFL_DISP_GXS_SetVisibleControl(
    GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 |
    GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3 |
    GX_PLANEMASK_OBJ, VISIBLE_OFF );

  GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME1_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME2_S );
  GFL_BG_FreeBGControl( GFL_BG_FRAME3_S );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		グラフィックデータセット
 *
 * @param		wk    ポケモンリスト画面のワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_BgGraphicSet( BPLIST_WORK * wk )
{
  ARCHANDLE* hdl;

  hdl  = GFL_ARC_OpenDataHandle( ARCID_B_PLIST_GRA,  GFL_HEAP_LOWID(wk->dat->heap) );

  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    hdl, NARC_b_plist_gra_b_plist_lz_NCGR, GFL_BG_FRAME3_S, 0, 0, TRUE, wk->dat->heap );

  {
    NNSG2dScreenData * dat;
    void * buf;
    buf = GFL_ARCHDL_UTIL_LoadScreen(
            hdl, NARC_b_plist_gra_b_plist_anm_lz_NSCR, TRUE, &dat, wk->dat->heap );
    BattlePokeList_ButtonScreenMake( wk, (u16 *)dat->rawData );
    GFL_HEAP_FreeMemory( buf );

    buf = GFL_ARCHDL_UTIL_LoadScreen(
            hdl, NARC_b_plist_gra_b_plist_anm2_lz_NSCR, TRUE, &dat, wk->dat->heap );
    BattlePokeList_ButtonScreenMake2( wk, (u16 *)dat->rawData );
    GFL_HEAP_FreeMemory( buf );
  }

  PaletteWorkSet_Arc(
    wk->pfd, ARCID_B_PLIST_GRA, NARC_b_plist_gra_b_plist_NCLR,
    wk->dat->heap, FADE_SUB_BG, 0x20*15, 0 );

  GFL_ARC_CloseDataHandle( hdl );

  {
    u16 * pltt = PaletteWorkDefaultWorkGet( wk->pfd, FADE_SUB_BG );
    GFL_STD_MemCopy( &pltt[BPL_PAL_B_GREEN*16], wk->wb_pal, 0x20 );
  }

  // メッセージウィンドウ
  BmpWinFrame_GraphicSet(
    GFL_BG_FRAME0_S, TALK_WIN_CGX_POS, BPL_PAL_TALK_WIN, MENU_TYPE_SYSTEM, wk->dat->heap );
  PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_BG, BPL_PAL_TALK_WIN*16, 0x20 );

  // フォントパレット
  PaletteWorkSet_Arc(
    wk->pfd, ARCID_FONT, NARC_font_default_nclr,
    wk->dat->heap, FADE_SUB_BG, 0x20, BPL_PAL_SYS_FONT*16 );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ関連セット
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_MsgManSet( BPLIST_WORK * wk )
{
  wk->mman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_b_plist_dat, wk->dat->heap );
  wk->nfnt = GFL_FONT_Create( ARCID_FONT, NARC_font_num_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->dat->heap );
  wk->wset = WORDSET_Create( wk->dat->heap );
  wk->que  = PRINTSYS_QUE_CreateEx( 2048, wk->dat->heap );
  wk->msg_buf = GFL_STR_CreateBuffer( TMP_MSG_BUF_SIZ, wk->dat->heap );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		メッセージ関連削除
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_MsgManExit( BPLIST_WORK * wk )
{
  GFL_MSG_Delete( wk->mman );
  GFL_FONT_Delete( wk->nfnt );
  WORDSET_Delete( wk->wset );
  PRINTSYS_QUE_Delete( wk->que );
  GFL_STR_DeleteBuffer( wk->msg_buf );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンデータ作成コア
 *
 * @param		wk    ワーク
 * @param		pp		POKEMON_PARAM
 * @param		dat		作成データ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void PokeDataMakeCore( BPLIST_WORK * wk, POKEMON_PARAM * pp, BPL_POKEDATA * dat )
{
  u32 j;

  dat->pp = pp;
  if( pp == NULL ){
    dat->mons = 0;
    return;
  }

  dat->mons = PP_Get( pp, ID_PARA_monsno, NULL );
  if( dat->mons == 0 ){
    return;
  }

  dat->pow = PP_Get( dat->pp, ID_PARA_pow, NULL );
  dat->def = PP_Get( dat->pp, ID_PARA_def, NULL );
  dat->agi = PP_Get( dat->pp, ID_PARA_agi, NULL );
  dat->spp = PP_Get( dat->pp, ID_PARA_spepow, NULL );
  dat->spd = PP_Get( dat->pp, ID_PARA_spedef, NULL );
  dat->hp  = PP_Get( dat->pp, ID_PARA_hp, NULL );
  dat->mhp = PP_Get( dat->pp, ID_PARA_hpmax, NULL );

  dat->type1 = (u8)PP_Get( dat->pp, ID_PARA_type1, NULL );
  dat->type2 = (u8)PP_Get( dat->pp, ID_PARA_type2, NULL );

  dat->lv  = (u8)PP_Get( dat->pp, ID_PARA_level, NULL );
  if( PP_Get( dat->pp, ID_PARA_nidoran_nickname, NULL ) == TRUE ){
    dat->sex_put = 0;
  }else{
    dat->sex_put = 1;
  }
  dat->sex = PP_GetSex( dat->pp );
  dat->st  = APP_COMMON_GetStatusIconAnime( dat->pp );
  dat->egg = (u8)PP_Get( dat->pp, ID_PARA_tamago_flag, NULL );

  dat->spa  = (u16)PP_Get( dat->pp, ID_PARA_speabino, NULL );
  dat->item = (u16)PP_Get( dat->pp, ID_PARA_item, NULL );

  dat->form = (u32)PP_Get( dat->pp, ID_PARA_form_no, NULL );

  dat->now_exp     = PP_Get( dat->pp, ID_PARA_exp, NULL );
  dat->now_lv_exp  = PP_GetMinExp( dat->pp );
  if( dat->lv == 100 ){
    dat->next_lv_exp = dat->now_lv_exp;
  }else{
    dat->next_lv_exp = POKETOOL_GetMinExp( dat->mons, dat->form, dat->lv+1 );
  }

  for( j=0; j<4; j++ ){
    BPL_POKEWAZA * waza = &dat->waza[j];

    waza->id   = PP_Get( dat->pp, ID_PARA_waza1+j, NULL );
    if( waza->id == 0 ){ continue; }
    waza->pp   = PP_Get( dat->pp, ID_PARA_pp1+j, NULL );
    waza->mpp  = PP_Get( dat->pp, ID_PARA_pp_count1+j, NULL );
    waza->mpp  = WAZADATA_GetMaxPP( waza->id, waza->mpp );
    waza->type = WAZADATA_GetParam( waza->id, WAZAPARAM_TYPE );
    waza->kind = WAZADATA_GetParam( waza->id, WAZAPARAM_DAMAGE_TYPE );
		if( WAZADATA_IsAlwaysHit(waza->id) == TRUE ){
			waza->hit = 0;
		}else{
			waza->hit = WAZADATA_GetParam( waza->id, WAZAPARAM_HITPER );
		}
    waza->pow  = WAZADATA_GetParam( waza->id, WAZAPARAM_POWER );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンデータ作成
 *
 * @param		wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_PokeDataMake( BPLIST_WORK * wk )
{
  u32 i;

  if( wk->dat->multiMode == FALSE ){
    for( i=0; i<TEMOTI_POKEMAX; i++ ){
      if( i < PokeParty_GetPokeCount(wk->dat->pp) ){
        PokeDataMakeCore( wk, PokeParty_GetMemberPointer(wk->dat->pp,i), &wk->poke[i] );
      }else{
        PokeDataMakeCore( wk, NULL, &wk->poke[i] );
      }
    }
  }else{
    for( i=0; i<TEMOTI_POKEMAX/2; i++ ){
      if( i < PokeParty_GetPokeCount(wk->dat->pp) ){
        PokeDataMakeCore( wk, PokeParty_GetMemberPointer(wk->dat->pp,i), &wk->poke[i] );
      }else{
        PokeDataMakeCore( wk, NULL, &wk->poke[i] );
      }
      if( i < PokeParty_GetPokeCount(wk->dat->multi_pp) ){
        PokeDataMakeCore( wk, PokeParty_GetMemberPointer(wk->dat->multi_pp,i), &wk->poke[i+3] );
      }else{
        PokeDataMakeCore( wk, NULL, &wk->poke[i+3] );
      }
    }
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン選択
 *
 * @param		wk    ワーク
 *
 * @retval  "TRUE = 選択された"
 * @retval  "FALSE = 未選択"
 */
//--------------------------------------------------------------------------------------------
static u8 BPL_PokemonSelect( BPLIST_WORK * wk )
{
  u32 ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_LIST_POKE1:
  case BPLIST_UI_LIST_POKE2:
  case BPLIST_UI_LIST_POKE3:
  case BPLIST_UI_LIST_POKE4:
  case BPLIST_UI_LIST_POKE5:
  case BPLIST_UI_LIST_POKE6:
    if( BattlePokeList_PokeSetCheck( wk, ret ) != 0 ){
      wk->dat->sel_poke = (u8)ret;
#ifdef DEBUG_ONLY_FOR_hudson
      if( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA ) || HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA2 ) )
      {
        // 強制的に4匹目を選択（バトンタッチ等対策）
        wk->dat->sel_poke = BPLIST_UI_LIST_POKE4;
      }
#endif
      return TRUE;
    }
    break;

  case BPLIST_UI_LIST_RETURN:
    wk->dat->sel_poke = BPL_SEL_EXIT;
    return TRUE;

  case CURSORMOVE_CANCEL:         // キャンセル
    wk->dat->sel_poke = BPL_SEL_EXIT;
    return TRUE;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
    PlaySE( wk, SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_NONE:           // 動作なし
    break;
  }

  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		指定位置にポケモンが存在するか
 *
 * @param		wk    ワーク
 * @param		pos   位置
 *
 * @retval  "0 = いない"
 * @retval  "1 = 戦闘中"
 * @retval  "2 = 控え"
 */
//--------------------------------------------------------------------------------------------
u8 BattlePokeList_PokeSetCheck( BPLIST_WORK * wk, s32 pos )
{
  if( wk->poke[ BPLISTMAIN_GetListRow(wk,pos) ].mons == 0 ){
    return 0;
  }
  if( FightPokeCheck( wk, pos ) == TRUE ){
    return 1;
  }
  return 2;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		次のポケモンを取得
 *
 * @param		wk    ワーク
 * @param		pos   現在位置
 * @param		mv    移動方向
 *
 * @retval  "0xff = 次のポケモンなし"
 * @retval  "0xff != 次のポケモンの位置"
 */
//--------------------------------------------------------------------------------------------
static u8 BPL_NextPokeGet( BPLIST_WORK * wk, s32 pos, s32 mv )
{
  s32 now = pos;

  if( BattlePokeList_MultiCheck( wk ) == TRUE ){
    u8 check_tbl[] = { 0, 2, 4, 1, 3, 5 };

    for( pos=0; pos<6; pos++ ){
      if( now == check_tbl[pos] ){ break; }
    }

    while( 1 ){
      pos += mv;

      if( pos < 0 ){
        pos = 5;
      }else if( pos >= 6 ){
        pos = 0;
      }
      if( now == check_tbl[pos] ){ break; }
      if( BattlePokeList_PokeSetCheck( wk, check_tbl[pos] ) != 0 ){
        if( wk->poke[ BPLISTMAIN_GetListRow(wk,check_tbl[pos]) ].egg == 0 ){
          return check_tbl[pos];
        }
      }
    }
  }else{
    while( 1 ){
      pos += mv;
      if( pos < 0 ){
        pos = 5;
      }else if( pos >= 6 ){
        pos = 0;
      }
      if( now == pos ){ break; }
      if( BattlePokeList_PokeSetCheck( wk, pos ) != 0 ){
        if( wk->poke[ BPLISTMAIN_GetListRow(wk,pos) ].egg == 0 ){
          return (u8)pos;
        }
      }
    }
  }

  return 0xff;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		経験値ゲージ書き換え
 *
 * @param		wk    ワーク
 * @param		page  ページ番号
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_ExpGagePut( BPLIST_WORK * wk, u8 page )
{
  BPL_POKEDATA * pd;
  u32 max;
  u32 now;
  u16 cgx;
  u16 px, py;
  u8  dot;
  u8  i;

  if( page != BPLIST_PAGE_MAIN ){ return; }

  pd = &wk->poke[ BPLISTMAIN_GetListRow(wk,wk->dat->sel_poke) ];

/* ルビサファの育て屋でLv100以上の経験値になるため、マイナス表示になってしまう不具合対処 */
  if( pd->lv < 100 ){
    max = pd->next_lv_exp - pd->now_lv_exp;
    now = pd->now_exp - pd->now_lv_exp;
  }else{
    max = 0;
    now = 0;
  }

  dot = GAUGETOOL_GetNumDotto( now, max, EXP_DOT_MAX );

  for( i=0; i<EXP_DOT_CHR_MAX; i++ ){
    if( dot >= 8 ){
      cgx = EXP_CGX + 8;
    }else{
      cgx = EXP_CGX + dot;
    }

		GFL_BG_FillScreen(
			GFL_BG_FRAME3_S, cgx, P3_EXPGAGE_PX+i, P3_EXPGAGE_PY, 1, 1, GFL_BG_SCRWRT_PALNL );

    if( dot < 8 ){
      dot = 0;
    }else{
      dot -= 8;
    }
  }

  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_S );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え
 *
 * @param		wk					ワーク
 * @param		next_page		次のページ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static BOOL BPL_PageChange( BPLIST_WORK * wk, u8 next_page )
{
  switch( wk->page_chg_seq ){
  case 0:
    if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
      BattlePokeList_BmpFree( wk );
      BattlePokeList_BmpAdd( wk, next_page );
      BattlePokeList_BmpWrite( wk, next_page );
      wk->page_chg_seq++;
    }
    break;

  case 1:
    if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
      BPL_PageChgBgScreenChg( wk, next_page );
      GFL_BG_ClearScreenCodeVReq( GFL_BG_FRAME0_S, 0 );
      GFL_BG_ClearScreenCodeVReq( GFL_BG_FRAME1_S, 0 );
      BPLISTBMP_SetStrScrn( wk );
      BPL_ExpGagePut( wk, next_page );
      BattlePokeList_PageObjSet( wk, next_page );
      CursorMoveSet( wk, next_page );
      BattlePokeList_ButtonPageScreenInit( wk, next_page );
      BattlePokeList_ButtonPalSet( wk, next_page );
      wk->page = next_page;
      wk->page_chg_seq = 0;
      return TRUE;
    }
    break;
  }

  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGスクリーン書き換え
 *
 * @param wk    ワーク
 * @param page  ページID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_PageChgBgScreenChg( BPLIST_WORK * wk, u8 page )
{
  NNSG2dScreenData * dat;
  void * buf;
  u32 i;

  for( i=0; i<2; i++ ){
    buf = GFL_ARC_UTIL_LoadScreen(
      ARCID_B_PLIST_GRA, ScreenArc[page][i], TRUE, &dat, wk->dat->heap );
    GFL_BG_WriteScreen( GFL_BG_FRAME2_S+i, (u16 *)dat->rawData, 0, 0, 32, 24 );
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S+i );
    GFL_HEAP_FreeMemory( buf );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		入れ替えチェック
 *
 * @param		wk    ワーク
 *
 * @retval  "TRUE = 入れ替え可"
 * @retval  "FALSE = 入れ替え不可"
 */
//--------------------------------------------------------------------------------------------
static u8 BPL_IrekaeCheck( BPLIST_WORK * wk )
{
  BPL_POKEDATA * dat;
  STRBUF * str;
  u8  pos;

  pos = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
  dat = &wk->poke[pos];

  // 他人
  if( BattlePokeList_MultiPosCheck( wk, pos ) == TRUE ){
    str = GFL_MSG_CreateString( wk->mman, mes_b_plist_m05 );
    WORDSET_ExpandStr( wk->wset, wk->msg_buf, str );
    GFL_STR_DeleteBuffer( str );
    return FALSE;
  }

  // 瀕死
  if( dat->hp == 0 ){
    str = GFL_MSG_CreateString( wk->mman, mes_b_plist_m02 );
    WORDSET_RegisterPokeNickName( wk->wset, 0, dat->pp );
    WORDSET_ExpandStr( wk->wset, wk->msg_buf, str );
    GFL_STR_DeleteBuffer( str );
    return FALSE;
  }

  // 出ている
  if( FightPokeCheck( wk, wk->dat->sel_poke ) == TRUE ){
    str = GFL_MSG_CreateString( wk->mman, mes_b_plist_m01 );
    WORDSET_RegisterPokeNickName( wk->wset, 0, dat->pp );
    WORDSET_ExpandStr( wk->wset, wk->msg_buf, str );
    GFL_STR_DeleteBuffer( str );
    return FALSE;
  }

  // タマゴ
  if( BPL_TamagoCheck( wk ) == TRUE ){
    GFL_MSG_GetString( wk->mman, mes_b_plist_m04, wk->msg_buf );
    return FALSE;
  }

  // 選択されている
  if( ChangePokeCheck( wk, wk->dat->sel_poke ) == TRUE ){
    dat = &wk->poke[ BPLISTMAIN_GetListRow(wk,wk->dat->sel_poke) ];
    str = GFL_MSG_CreateString( wk->mman, mes_b_plist_m18 );
    WORDSET_RegisterPokeNickName( wk->wset, 0, dat->pp );
    WORDSET_ExpandStr( wk->wset, wk->msg_buf, str );
    GFL_STR_DeleteBuffer( str );
    return FALSE;
  }

  // 技
  if( wk->dat->chg_waza != 0 ){
    dat = &wk->poke[ BPLISTMAIN_GetListRow(wk,wk->dat->sel_pos_index) ];
    str = GFL_MSG_CreateString( wk->mman, mes_b_plist_m03 );
    WORDSET_RegisterPokeNickName( wk->wset, 0, dat->pp );
    WORDSET_ExpandStr( wk->wset, wk->msg_buf, str );
    GFL_STR_DeleteBuffer( str );
    return FALSE;
  }

  return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		タマゴチェック
 *
 * @param		wk    ワーク
 *
 * @retval  "TRUE = タマゴ"
 * @retval  "FALSE = タマゴ以外"
 */
//--------------------------------------------------------------------------------------------
static u8 BPL_TamagoCheck( BPLIST_WORK * wk )
{
  if( wk->poke[ BPLISTMAIN_GetListRow(wk,wk->dat->sel_poke) ].egg != 0 ){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		マルチバトルチェック
 *
 * @param		wk    ワーク
 *
 * @retval  "TRUE = マルチバトル"
 * @retval  "FALSE = マルチバトル以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BattlePokeList_MultiCheck( BPLIST_WORK * wk )
{
  return wk->dat->multiMode;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		マルチバトルでパートナーのポケモンかチェック
 *
 * @param		wk    ワーク
 * @param		pos   位置
 *
 * @retval  "TRUE = はい"
 * @retval  "FALSE = いいえ"
 */
//--------------------------------------------------------------------------------------------
u8 BattlePokeList_MultiPosCheck( BPLIST_WORK * wk, u8 pos )
{
  if( BattlePokeList_MultiCheck( wk ) == TRUE ){
    if( pos >= 3 ){ return TRUE; }
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		技忘れチェック
 *
 * @param   wk    ワーク
 *
 * @return  FIELD_SKILL_CHECK_RET
 */
//--------------------------------------------------------------------------------------------
static FIELD_SKILL_CHECK_RET BPL_HidenCheck( BPLIST_WORK * wk )
{
  u16 waza;

  // 新しく覚える技はチェックしなくていいハズ
  if( wk->dat->sel_wp == 4 ){
    return FSCR_OK;
  }

  waza = wk->poke[wk->dat->sel_poke].waza[wk->dat->sel_wp].id;
  return FIELD_SKILL_CHECK_CheckForgetSkill( wk->dat->gamedata, waza, wk->dat->heap );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		指定位置のポケモンが戦闘に出ているか（並び替え対応）
 *
 * @param		wk    戦闘リストワーク
 * @param		pos   位置
 *
 * @retval  "TRUE = 出ている"
 * @retval  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL FightPokeCheck( BPLIST_WORK * wk, u32 pos )
{
  pos = BPLISTMAIN_GetListRow( wk, pos );

  // マルチバトル時
  if( BattlePokeList_MultiCheck( wk ) == TRUE ){
    if( pos == 0 || pos == 3 ){
      return TRUE;
    }
    return FALSE;
  }

	if( pos < wk->dat->fight_poke_max ){
		return TRUE;
	}
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		指定位置のポケモンが戦闘に出ているか（画面上の位置）
 *
 * @param		wk    戦闘リストワーク
 * @param		pos   位置
 *
 * @retval  "TRUE = 出ている"
 * @retval  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL FightPosCheck( BPLIST_WORK * wk, u32 pos )
{
	if( pos < wk->dat->fight_poke_max ){
		return TRUE;
	}
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		すでに選択されているか
 *
 * @param		wk    戦闘リストワーク
 * @param		pos   位置
 *
 * @retval  "TRUE = 選択されている"
 * @retval  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL ChangePokeCheck( BPLIST_WORK * wk, u32 pos )
{
  pos = BPLISTMAIN_GetListRow( wk, pos );

	if( wk->dat->fight_poke_max != 1 ){
    if( pos == wk->dat->change_sel[0] || pos == wk->dat->change_sel[1] ){
      return TRUE;
    }
    return CheckListRowSelect( wk, pos );
	}
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		カーソル移動設定
 *
 * @param		wk    戦闘リストワーク
 * @param		page	ページ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void CursorMoveSet( BPLIST_WORK * wk, u8 page )
{
  u32 pos;

  switch( page ){
  case BPLIST_PAGE_SELECT:    // ポケモン選択ページ
  case BPLIST_PAGE_DEAD:      // 瀕死入れ替え選択ページ
    wk->chg_page_cp = 0;
    wk->dat->sel_wp = 0;
    pos = wk->dat->sel_poke;
    break;

  case BPLIST_PAGE_POKE_CHG:    // ポケモン入れ替えページ
    wk->dat->sel_wp = 0;
    pos = wk->chg_page_cp;
    break;

  case BPLIST_PAGE_WAZA_SEL:    // ステータス技選択ページ
  case BPLIST_PAGE_SKILL:       // ステータス技詳細ページ
    pos = wk->dat->sel_wp;
    break;

  case BPLIST_PAGE_WAZASET_BS:  // ステータス技忘れ１ページ（戦闘技選択）
    pos = wk->wws_page_cp;
    break;

  case BPLIST_PAGE_WAZASET_BI:  // ステータス技忘れ２ページ（戦闘技詳細）
    pos = wk->wwm_page_cp;
    break;

  case BPLIST_PAGE_MAIN:      // ステータスメインページ
  case BPLIST_PAGE_PP_RCV:    // PP回復技選択ページ
    pos = 0;
    break;
  }

  BPLISTUI_ChangePage( wk, page, pos );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		リストの並びを初期化
 *
 * @param		wk    戦闘リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void InitListRow( BPLIST_WORK * wk )
{
  u32 i;

  // この画面での並びを初期化
  if( wk->dat->multiMode == TRUE ){
    for( i=0; i<TEMOTI_POKEMAX; i++ ){
      wk->listRow[i] = InitListRowMulti[wk->dat->multiPos][i];
    }
  }else{
    for( i=0; i<TEMOTI_POKEMAX; i++ ){
      wk->listRow[i] = i;
    }
  }
  //
  for( i=0; i<BPL_SELNUM_MAX; i++ ){
    wk->dat->sel_pos[i] = BPL_SELPOS_NONE;
  }
  // ログ初期化
  for( i=0; i<BPL_SELNUM_MAX-1; i++ ){
    wk->chg_log[i].pos1 = BPL_SELPOS_NONE;
    wk->chg_log[i].pos2 = BPL_SELPOS_NONE;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		リストの並びを入れ替え
 *
 * @param		wk    戦闘リストワーク
 * @param		pos1	位置１
 * @param		pos2	位置２
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void ChangeListRow( BPLIST_WORK * wk, u32 pos1, u32 pos2 )
{
  u8  tmp;

  tmp = wk->listRow[pos1];
  wk->listRow[pos1] = wk->listRow[pos2];
  wk->listRow[pos2] = tmp;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		選択されているか
 *
 * @param		wk    戦闘リストワーク
 * @param		pos		位置（並び替えデータの位置）
 *
 * @retval	"TRUE = 選択されている"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL CheckListRowSelect( BPLIST_WORK * wk, u32 pos )
{
  u32 i;

  for( i=0; i<BPL_SELNUM_MAX; i++ ){
    if( wk->dat->sel_pos[i] == pos ){
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		画面上の位置から並びデータ位置を取得
 *
 * @param		wk    戦闘リストワーク
 * @param		pos		画面上の位置
 *
 * @return	並びデータ位置
 */
//--------------------------------------------------------------------------------------------
u8 BPLISTMAIN_GetListRow( BPLIST_WORK * wk, u32 pos )
{
  return wk->listRow[pos];
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		入れ替えデータ設定
 *
 * @param		wk    戦闘リストワーク
 * @param		pos		入れ替え先の画面上の位置
 * @param		row		入れ替え元の画面上の位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SetRetNum( BPLIST_WORK * wk, u8 pos, u8 row )
{
  wk->dat->sel_pos[pos] = BPLISTMAIN_GetListRow( wk, row );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		入れ替えログ作成
 *
 * @param		wk    戦闘リストワーク
 * @param		pos1	位置１
 * @param		pos2	位置２
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SetLog( BPLIST_WORK * wk, u8 pos1, u8 pos2 )
{
  u8  i;

  // ログ作成
  for( i=0; i<BPL_SELNUM_MAX-1; i++ ){
    if( wk->chg_log[i].pos1 == BPL_SELPOS_NONE ){
      wk->chg_log[i].pos1 = pos1;
      wk->chg_log[i].pos2 = pos2;
      break;
    }
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		最新の入れ替えログを取得
 *
 * @param		wk    戦闘リストワーク
 * @param		pos1	位置１
 * @param		pos2	位置２
 * @param		del		取得後にログを削除するか TRUE = 削除
 *
 * @retval	"TRUE = 取得"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BPLISTMAIN_GetNewLog( BPLIST_WORK * wk, u8 * pos1, u8 * pos2, BOOL del )
{
  s32 i;

  for( i=BPL_SELNUM_MAX-1-1; i>=0; i-- ){
    if( wk->chg_log[i].pos1 != BPL_SELPOS_NONE ){
      *pos1 = wk->chg_log[i].pos1;
      *pos2 = wk->chg_log[i].pos2;
      if( del == TRUE ){
        wk->chg_log[i].pos1 = BPL_SELPOS_NONE;
        wk->chg_log[i].pos2 = BPL_SELPOS_NONE;
      }
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		瀕死時の入れ替え先選択ページへの切り替え処理
 *
 * @param		wk    戦闘リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPageChgDead( BPLIST_WORK * wk )
{
  if( BPL_PageChange( wk, BPLIST_PAGE_DEAD ) == FALSE ){
    wk->chg_poke_sel = wk->dat->sel_poke;
    return SEQ_BPL_PAGECHG_DEAD;
  }
  return SEQ_BPL_POKESEL_DEAD;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		瀕死時の入れ替え先のチェック
 *
 * @param		wk    戦闘リストワーク
 *
 * @retval	"TRUE = 入れ替え可"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL CheckDeadChange( BPLIST_WORK * wk )
{
  BPL_POKEDATA * dat;
  STRBUF * str;
  u8  pos;

  pos = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
  dat = &wk->poke[pos];

  // 他人
  if( BattlePokeList_MultiPosCheck( wk, pos ) == TRUE ){
    str = GFL_MSG_CreateString( wk->mman, mes_b_plist_m05 );
    WORDSET_ExpandStr( wk->wset, wk->msg_buf, str );
    GFL_STR_DeleteBuffer( str );
    return FALSE;
  }

  // 入れ替え可能位置ではない
  if( FightPosCheck( wk, wk->dat->sel_poke ) == FALSE ){
    str = GFL_MSG_CreateString( wk->mman, mes_b_plist_m21 );
    WORDSET_RegisterPokeNickName( wk->wset, 0, dat->pp );
    WORDSET_ExpandStr( wk->wset, wk->msg_buf, str );
    GFL_STR_DeleteBuffer( str );
    return FALSE;
  }

  // 戦闘に出ている（=入れ替え位置なのに生きている）
  if( dat->hp != 0 ){
    str = GFL_MSG_CreateString( wk->mman, mes_b_plist_m01 );
    WORDSET_RegisterPokeNickName( wk->wset, 0, dat->pp );
    WORDSET_ExpandStr( wk->wset, wk->msg_buf, str );
    GFL_STR_DeleteBuffer( str );
    return FALSE;
  }

  return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		瀕死時の入れ替え先選択処理
 *
 * @param		wk    戦闘リストワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPokeSelDead( BPLIST_WORK * wk )
{
  if( PaletteFadeCheck( wk->pfd ) != 0 ){ return SEQ_BPL_POKESEL_DEAD; }

  if( BPL_PokemonSelect( wk ) == TRUE ){
    if( wk->dat->sel_poke == BPL_SEL_EXIT ){
      PlaySE( wk, SEQ_SE_CANCEL2 );
      BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
      wk->dat->sel_poke = wk->chg_poke_sel;
      wk->ret_seq = SEQ_BPL_PAGECHG_IREKAE;
      return SEQ_BPL_BUTTON_WAIT;
    }else if( wk->dat->sel_poke == wk->chg_poke_sel ){
      return SEQ_BPL_PAGECHG_IREKAE;
    }else{
      if( CheckDeadChange( wk ) == TRUE ){
        SetLog( wk, wk->dat->sel_poke, wk->chg_poke_sel );
        SetRetNum( wk, wk->dat->sel_poke, wk->chg_poke_sel );
        wk->chg_pos1 = wk->dat->sel_poke;
        wk->chg_pos2 = wk->chg_poke_sel;
        wk->btn_seq = 0;
        return SEQ_BPL_POKECHG_DEAD;
      }else{
        BattlePokeList_TalkMsgSet( wk );
        wk->ret_seq = SEQ_BPL_DEAD_ERR_RET;
        return SEQ_BPL_MSG_WAIT;
      }
    }
  }

  return SEQ_BPL_POKESEL_DEAD;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		瀕死時の入れ替えエラー復帰
 *
 * @param		wk    戦闘リストワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqDeadErrRet( BPLIST_WORK * wk )
{
  BmpWinFrame_Clear( wk->win[WIN_TALK].win, WINDOW_TRANS_ON );
  BPLISTBMP_DeadSelInfoMesPut( wk );
  BPLISTBMP_SetCommentScrn( wk );
  return SEQ_BPL_POKESEL_DEAD;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		瀕死時の入れ替えプレートのスクリーン位置取得
 *
 * @param		x			Ｘ座標
 * @param		y			Ｙ座標
 * @param		num		位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void GetDeadScrnPos( u8 * x, u8 * y, u8 num )
{
  *x = ( num & 1 ) * BPL_COMM_BSX_PLATE;
  *y = num / 2 * BPL_COMM_BSY_PLATE + ( num & 1 );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		瀕死時の入れ替えプレートデータ作成
 *
 * @param		scrn	スクリーンデータ
 * @param		frm		ＢＧフレーム
 * @param		num		位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MakeDeadChgScrn( u16 * scrn, u8 frm, u8 num )
{
  u16 * buf;
  u8  x, y;
  u8  i;

  buf = GFL_BG_GetScreenBufferAdrs( frm );
  GetDeadScrnPos( &x, &y, num );

  for( i=0; i<BPL_COMM_BSY_PLATE; i++ ){
    GFL_STD_MemCopy16( &buf[(y+i)*32+x], &scrn[i*BPL_COMM_BSX_PLATE], BPL_COMM_BSX_PLATE*2 );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		瀕死時の入れ替えＯＢＪ動作
 *
 * @param		wk    戦闘リストワーク
 * @param		num		位置
 * @param		mode	移動モード
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void MoveDeadChgObj( BPLIST_WORK * wk, u8 num, u8 mode )
{
  u8  pos;
  s8  mv;

  if( num & 1 ){
    if( mode == 0 ){
      mv = 8;
    }else{
      mv = -8;
    }
  }else{
    if( mode == 0 ){
      mv = -8;
    }else{
      mv = 8;
    }
  }

  BPLISTOBJ_MoveDeadChg( wk, num, mv );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		瀕死時の入れ替え動作
 *
 * @param		wk    戦闘リストワーク
 *
 * @retval	"TRUE = 動作中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL MoveDeadChangeMain( BPLIST_WORK * wk )
{
  switch( wk->btn_seq ){
  case 0:
    wk->chg_wfrm = BGWINFRM_Create( BGWINFRM_TRANS_VBLANK, 4, wk->dat->heap );
    BGWINFRM_Add( wk->chg_wfrm, 0, GFL_BG_FRAME1_S, BPL_COMM_BSX_PLATE, BPL_COMM_BSY_PLATE );
    BGWINFRM_Add( wk->chg_wfrm, 1, GFL_BG_FRAME2_S, BPL_COMM_BSX_PLATE, BPL_COMM_BSY_PLATE );
    BGWINFRM_Add( wk->chg_wfrm, 2, GFL_BG_FRAME1_S, BPL_COMM_BSX_PLATE, BPL_COMM_BSY_PLATE );
    BGWINFRM_Add( wk->chg_wfrm, 3, GFL_BG_FRAME2_S, BPL_COMM_BSX_PLATE, BPL_COMM_BSY_PLATE );
    {
      u16 * buf = GFL_HEAP_AllocMemory( GFL_HEAP_LOWID(wk->dat->heap), BPL_COMM_BSX_PLATE*BPL_COMM_BSY_PLATE*2 );
      MakeDeadChgScrn( buf, GFL_BG_FRAME1_S, wk->chg_pos1 );
      BGWINFRM_FrameSet( wk->chg_wfrm, 0, buf );
      MakeDeadChgScrn( buf, GFL_BG_FRAME2_S, wk->chg_pos1 );
      BGWINFRM_FrameSet( wk->chg_wfrm, 1, buf );
      MakeDeadChgScrn( buf, GFL_BG_FRAME1_S, wk->chg_pos2 );
      BGWINFRM_FrameSet( wk->chg_wfrm, 2, buf );
      MakeDeadChgScrn( buf, GFL_BG_FRAME2_S, wk->chg_pos2 );
      BGWINFRM_FrameSet( wk->chg_wfrm, 3, buf );
      GFL_HEAP_FreeMemory( buf );
    }
    {
      u8  x, y;
      GetDeadScrnPos( &x, &y, wk->chg_pos1 );
      BGWINFRM_FramePut( wk->chg_wfrm, 0, x, y );
      BGWINFRM_FramePut( wk->chg_wfrm, 1, x, y );
      GetDeadScrnPos( &x, &y, wk->chg_pos2 );
      BGWINFRM_FramePut( wk->chg_wfrm, 2, x, y );
      BGWINFRM_FramePut( wk->chg_wfrm, 3, x, y );
    }
    BGWINFRM_MoveMain( wk->chg_wfrm );
    wk->btn_seq++;
    break;

  case 1:
    if( wk->chg_pos1 & 1 ){
      BGWINFRM_MoveInit( wk->chg_wfrm, 0, 1, 0, BPL_COMM_BSX_PLATE );
      BGWINFRM_MoveInit( wk->chg_wfrm, 1, 1, 0, BPL_COMM_BSX_PLATE );
    }else{
      BGWINFRM_MoveInit( wk->chg_wfrm, 0, -1, 0, BPL_COMM_BSX_PLATE );
      BGWINFRM_MoveInit( wk->chg_wfrm, 1, -1, 0, BPL_COMM_BSX_PLATE );
    }
    if( wk->chg_pos2 & 1 ){
      BGWINFRM_MoveInit( wk->chg_wfrm, 2, 1, 0, BPL_COMM_BSX_PLATE );
      BGWINFRM_MoveInit( wk->chg_wfrm, 3, 1, 0, BPL_COMM_BSX_PLATE );
    }else{
      BGWINFRM_MoveInit( wk->chg_wfrm, 2, -1, 0, BPL_COMM_BSX_PLATE );
      BGWINFRM_MoveInit( wk->chg_wfrm, 3, -1, 0, BPL_COMM_BSX_PLATE );
    }
    wk->btn_seq++;
    break;

  case 2:
    BGWINFRM_MoveMain( wk->chg_wfrm );
    MoveDeadChgObj( wk, wk->chg_pos1, 0 );
    MoveDeadChgObj( wk, wk->chg_pos2, 0 );
    {
      s8  x1, y1, x2, y2;
      BGWINFRM_PosGet( wk->chg_wfrm, 0, &x1, &y1 );
      BGWINFRM_PosGet( wk->chg_wfrm, 2, &x2, &y2 );
      if( BGWINFRM_MoveCheck( wk->chg_wfrm, 0 ) == FALSE ){
        ChangeListRow( wk, wk->chg_pos1, wk->chg_pos2 );
        BGWINFRM_FramePut( wk->chg_wfrm, 0, x2, y2 );
        BGWINFRM_FramePut( wk->chg_wfrm, 1, x2, y2 );
        BGWINFRM_FramePut( wk->chg_wfrm, 2, x1, y1 );
        BGWINFRM_FramePut( wk->chg_wfrm, 3, x1, y1 );
        BPLISTOBJ_ChgDeadSel( wk, wk->chg_pos1, wk->chg_pos2 );
        wk->btn_seq++;
      }
    }
    break;

  case 3:
    BGWINFRM_MoveMain( wk->chg_wfrm );
    if( wk->chg_pos2 & 1 ){
      BGWINFRM_MoveInit( wk->chg_wfrm, 0, -1, 0, BPL_COMM_BSX_PLATE );
      BGWINFRM_MoveInit( wk->chg_wfrm, 1, -1, 0, BPL_COMM_BSX_PLATE );
    }else{
      BGWINFRM_MoveInit( wk->chg_wfrm, 0, 1, 0, BPL_COMM_BSX_PLATE );
      BGWINFRM_MoveInit( wk->chg_wfrm, 1, 1, 0, BPL_COMM_BSX_PLATE );
    }
    if( wk->chg_pos1 & 1 ){
      BGWINFRM_MoveInit( wk->chg_wfrm, 2, -1, 0, BPL_COMM_BSX_PLATE );
      BGWINFRM_MoveInit( wk->chg_wfrm, 3, -1, 0, BPL_COMM_BSX_PLATE );
    }else{
      BGWINFRM_MoveInit( wk->chg_wfrm, 2, 1, 0, BPL_COMM_BSX_PLATE );
      BGWINFRM_MoveInit( wk->chg_wfrm, 3, 1, 0, BPL_COMM_BSX_PLATE );
    }
    wk->btn_seq++;
    break;

  case 4:
    BGWINFRM_MoveMain( wk->chg_wfrm );
    MoveDeadChgObj( wk, wk->chg_pos1, 1 );
    MoveDeadChgObj( wk, wk->chg_pos2, 1 );
    {
      s8  x1, y1, x2, y2;
      BGWINFRM_PosGet( wk->chg_wfrm, 0, &x1, &y1 );
      BGWINFRM_PosGet( wk->chg_wfrm, 2, &x2, &y2 );
      if( BGWINFRM_MoveCheck( wk->chg_wfrm, 0 ) == FALSE ){
        wk->btn_seq++;
      }
    }
    break;

  case 5:
    BGWINFRM_Exit( wk->chg_wfrm );
		wk->chg_wfrm = NULL;
    wk->btn_seq = 0;
    return FALSE;
  }

  return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		瀕死時の入れ替え動作シーケンス
 *
 * @param		wk    戦闘リストワーク
 *
 * @return	次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPokeChgDead( BPLIST_WORK * wk )
{
  if( MoveDeadChangeMain( wk ) == FALSE ){
    if( CheckNextDeadSel( wk ) == FALSE ){
      return SEQ_BPL_ENDSET;
    }
    BPLISTANM_RetButtonPut( wk );
    BPLISTBMP_PokeSelInfoMesPut( wk );
    BPLISTBMP_SetCommentScrn( wk );
    return SEQ_BPL_SELECT;
  }
  return SEQ_BPL_POKECHG_DEAD;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		瀕死入れ替え動作が必要かどうか
 *
 * @param		wk    戦闘リストワーク
 *
 * @retval	"TRUE = 必要"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL CheckDeadPoke( BPLIST_WORK * wk )
{
  u8  cnt;
  u8  max;
  u8  i;

  // マルチバトル時
  if( BattlePokeList_MultiCheck( wk ) == TRUE ){
    return FALSE;
  }

	if( wk->dat->fight_poke_max == 1 ){
		return FALSE;
	}else{
		max = wk->dat->fight_poke_max;
	}

  // 空きが複数ある
  cnt = 0;
  for( i=0; i<max; i++ ){
    u32 row = BPLISTMAIN_GetListRow( wk, i );
    if( wk->poke[row].hp == 0 ){
      cnt++;
      if( cnt == 2 ){
        return TRUE;
      }
    }
  }

  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		続けて瀕死入れ替えを行うか
 *
 * @param		wk    戦闘リストワーク
 *
 * @retval	"TRUE = やる"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL CheckNextDeadSel( BPLIST_WORK * wk )
{
  u8  max;
  u8  i;

  // マルチバトル時
  if( BattlePokeList_MultiCheck( wk ) == TRUE ){
    return FALSE;
  }

	if( wk->dat->fight_poke_max == 1 ){
		return FALSE;
	}else{
		max = wk->dat->fight_poke_max;
	}

  // 入れ替え可能なポケモンがいるか
  for( i=max; i<TEMOTI_POKEMAX; i++ ){
    u32 row = BPLISTMAIN_GetListRow( wk, i );
    if( wk->poke[row].mons != 0 && wk->poke[row].egg == 0 && wk->poke[row].hp != 0 ){
      return TRUE;
    }
  }

  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		上から検索して瀕死の箇所に強制的にセット
 *
 * @param		wk    戦闘リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SetDeadChangeData( BPLIST_WORK * wk )
{
  // マルチの場合は立ち位置が関係するので、確実に０に入れる
  if( BattlePokeList_MultiCheck( wk ) == TRUE ){
		wk->dat->sel_pos[0] = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
  }else{
    u32 i;

    for( i=0; i<BPL_SELNUM_MAX; i++ ){
      u32 row = BPLISTMAIN_GetListRow( wk, i );
      if( wk->poke[row].hp == 0 ){
        wk->dat->sel_pos[i] = wk->dat->sel_poke;
        break;
      }
    }
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		キャンセル設定
 *
 * @param		wk    戦闘リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void SetSelPosCancel( BPLIST_WORK * wk )
{
  u32 i;

  for( i=0; i<BPL_SELNUM_MAX; i++ ){
    wk->dat->sel_pos[i] = BPL_SELPOS_NONE;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		強制終了チェック
 *
 * @param		wk    戦闘リストワーク
 *
 * @retval	"TRUE = 強制終了"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
static BOOL CheckTimeOut( BPLIST_WORK * wk )
{
  if( wk->dat->time_out_flg == TRUE ){
    SetSelPosCancel( wk );
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＳＥ再生
 *
 * @param		wk    戦闘リストワーク
 * @param		no		ＳＥ番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PlaySE( BPLIST_WORK * wk, int no )
{
	if( wk->dat->commFlag == FALSE ){
		PMSND_PlaySE( no );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＳＥ再生（システム）
 *
 * @param		wk    戦闘リストワーク
 * @param		no		ＳＥ番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PlaySystemSE( BPLIST_WORK * wk, int no )
{
	if( wk->dat->commFlag == FALSE ){
		PMSND_PlaySystemSE( no );
	}
}
