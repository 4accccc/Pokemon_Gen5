//============================================================================================
/**
 * @file		b_bag_main.c
 * @brief		戦闘用バッグ画面
 * @author	Hiroyuki Nakamura
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
#include "msg/msg_b_bag.h"

#include "b_app_tool.h"

#include "b_bag.h"
#include "b_bag_main.h"
#include "b_bag_bmp.h"
#include "b_bag_item.h"
#include "b_bag_obj.h"
#include "b_bag_anm.h"
#include "b_bag_ui.h"
#include "b_bag_bmp_def.h"
#include "b_bag_gra.naix"


//============================================================================================
//  定数定義
//============================================================================================
// メインシーケンス
enum {
  SEQ_BBAG_INIT = 0,				// 初期化
  SEQ_BBAG_SHOOTER_INIT,	  // 初期化（シューター用）
	SEQ_BBAG_INIT_PRINT_WAIT,	// 初期化描画待ち
	SEQ_BBAG_INIT_WIPE_WAIT,	// 初期化ワイプ待ち

  SEQ_BBAG_POCKET,        // ポケット選択
  SEQ_BBAG_ITEM,          // アイテム選択
  SEQ_BBAG_USE,           // アイテム使用

  SEQ_BBAG_PAGE1_CHG,   // ポケット選択へ
  SEQ_BBAG_PAGE2_CHG,   // アイテム選択へ
  SEQ_BBAG_PAGE3_CHG,   // アイテム使用へ

  SEQ_BBAG_PAGECHG_WAIT,  // ページ切り替え待ち

  SEQ_BBAG_ITEMSEL_NEXT,  // 次のアイテムページへ
  SEQ_BBAG_ITEMSEL_WAIT,  // アイテムページ切り替え待ち

  SEQ_BBAG_ERR,     // エラーメッセージ終了待ち
  SEQ_BBAG_MSG_WAIT,    // メッセージ表示
  SEQ_BBAG_TRG_WAIT,    // トリガーウェイト

  SEQ_BBAG_BUTTON_WAIT, // ボタンアニメ終了待ち

  SEQ_BBAG_GETDEMO,   // 捕獲デモ用

  SEQ_BBAG_ENDSET,    // 終了フェードセット
  SEQ_BBAG_ENDWAIT,   // 終了フェード待ち
  SEQ_BBAG_END        // 終了
};

#define TMP_MSG_BUF_SIZ   ( 512 )   // テンポラリメッセージサイズ

#define BATTLE_BAGLIST_FADE_SPEED (0)


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void BattleBag_Main( GFL_TCB* tcb, void * work );

static int BBAG_SeqInit( BBAG_WORK * wk );
static int BBAG_SeqShooterInit( BBAG_WORK * wk );
static int BBAG_SeqInitPrintWait( BBAG_WORK * wk );
static int BBAG_SeqInitWipeWait( BBAG_WORK * wk );
static int BBAG_SeqPokeSelect( BBAG_WORK * wk );
static int BBAG_SeqItemSelect( BBAG_WORK * wk );
static int BBAG_SeqUseSelect( BBAG_WORK * wk );
static int BBAG_SeqPage1Chg( BBAG_WORK * wk );
static int BBAG_SeqPage2Chg( BBAG_WORK * wk );
static int BBAG_SeqPage3Chg( BBAG_WORK * wk );
static int BBAG_SeqPageChgWait( BBAG_WORK * wk );
static int BBAG_SeqItemSelNext( BBAG_WORK * wk );
static int BBAG_SeqItemSelWait( BBAG_WORK * wk );
static int BBAG_SeqError( BBAG_WORK * wk );
static int BBAG_SeqMsgWait( BBAG_WORK * wk );
static int BBAG_SeqTrgWait( BBAG_WORK * wk );
static int BBAG_SeqButtonWait( BBAG_WORK * wk );
static int BBAG_SeqGetDemoMain( BBAG_WORK * wk );
static int BBAG_SeqEndSet( BBAG_WORK * wk );
static int BBAG_SeqEndWait( BBAG_WORK * wk );
static BOOL BBAG_SeqEnd( GFL_TCB* tcb, BBAG_WORK * wk );

static void BBAG_BgInit( BBAG_WORK * wk );
static void BBAG_BgExit(void);
static void BBAG_BgGraphicSet( BBAG_WORK * wk );
static void BBAG_MsgManSet( BBAG_WORK * wk );
static void BBAG_MsgManExit( BBAG_WORK * wk );
static int BBAG_ItemUse( BBAG_WORK * wk );

static void BBAG_PageChgBgScroll( BBAG_WORK * wk, u8 page );
static void BBAG_PageChange( BBAG_WORK * wk, u8 next_page );

static BOOL CheckTimeOut( BBAG_WORK * wk );

static void PlaySE( BBAG_WORK * wk, int no );
static void PlaySystemSE( BBAG_WORK * wk, int no );


//============================================================================================
//  データ
//============================================================================================

// メインシーケンス
static const pBBagFunc MainSeqFunc[] = {
  BBAG_SeqInit,
  BBAG_SeqShooterInit,
  BBAG_SeqInitPrintWait,
	BBAG_SeqInitWipeWait,

  BBAG_SeqPokeSelect,
  BBAG_SeqItemSelect,
  BBAG_SeqUseSelect,

  BBAG_SeqPage1Chg,
  BBAG_SeqPage2Chg,
  BBAG_SeqPage3Chg,

  BBAG_SeqPageChgWait,

  BBAG_SeqItemSelNext,
  BBAG_SeqItemSelWait,

  BBAG_SeqError,
  BBAG_SeqMsgWait,
  BBAG_SeqTrgWait,

  BBAG_SeqButtonWait,

  BBAG_SeqGetDemoMain,

  BBAG_SeqEndSet,
  BBAG_SeqEndWait,
};



//--------------------------------------------------------------------------------------------
/**
 * @brief		戦闘用バッグタスク追加
 *
 * @param		dat   バッグデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BattleBag_TaskAdd( BBAG_DATA * dat )
{
  BBAG_WORK * wk;

  // シューターモードテスト
//  dat->mode = BBAG_MODE_SHOOTER;
	// ポケモンドリームキャッチモードテスト
//	dat->mode = BBAG_MODE_PDC;

  wk = GFL_HEAP_AllocClearMemory( dat->heap, sizeof(BBAG_WORK) );

  GFL_TCB_AddTask( BTLV_EFFECT_GetTCBSYS(), BattleBag_Main, wk, 100 );

  wk->dat = dat;
  wk->pfd = BTLV_EFFECT_GetPfd();
	wk->initFlag = FALSE;

	// シューター
  if( wk->dat->mode == BBAG_MODE_SHOOTER ){
    wk->page = BBAG_PAGE_MAIN;
    wk->seq  = SEQ_BBAG_SHOOTER_INIT;
		wk->poke_id = 0;
	// ポケモンドリームキャッチ
	}else if( wk->dat->mode == BBAG_MODE_PDC ){
    wk->page = BBAG_PAGE_MAIN;
    wk->seq  = SEQ_BBAG_INIT;
		wk->poke_id = BBAG_POKE_BALL;
	// その他
  }else{
    wk->page = BBAG_PAGE_POCKET;
    wk->seq  = SEQ_BBAG_INIT;
		wk->poke_id = 0;
  }

  {
    u32 i;

    for( i=0; i<BATTLE_BAG_POKE_MAX; i++ ){
      MYITEM_BattleBagCursorGet( wk->dat->bagcursor, i, &wk->dat->item_pos[i], &wk->dat->item_scr[i] );
    }
    wk->used_item = MYITEM_BattleBagLastItemGet( wk->dat->bagcursor );
    wk->used_poke = MYITEM_BattleBagLastPageGet( wk->dat->bagcursor );
  }

  BattleBag_UsedItemChack( wk );
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
static void BattleBag_Main( GFL_TCB * tcb, void * work )
{
  BBAG_WORK * wk = (BBAG_WORK *)work;

	if( wk->dat->comm_err_flg == TRUE ){
		wk->dat->ret_item = ITEM_DUMMY_DATA;
		wk->dat->ret_cost = 0;
		wk->seq = SEQ_BBAG_END;
	}

  if( wk->seq != SEQ_BBAG_END ){
    wk->seq = MainSeqFunc[wk->seq]( wk );
  }

  if( wk->seq == SEQ_BBAG_END ){
    if( BBAG_SeqEnd( tcb, wk ) == TRUE ){
      return;
    }
  }

  GFL_TCBL_Main( wk->tcbl );

  BBAGANM_ButtonAnmMain( wk );
  BGWINFRM_MoveMain( wk->bgwfrm );

  BBAGBMP_PrintMain( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		初期化シーケンス（通常）
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqInit( BBAG_WORK * wk )
{
  G2S_BlendNone();
	GXS_SetMasterBrightness( -16 );

  wk->tcbl = GFL_TCBL_Init( wk->dat->heap, wk->dat->heap, 1, 4 );

  wk->cpwk = BAPPTOOL_CreateCursor( wk->dat->heap );

  BBAG_BgInit( wk );
  BBAG_BgGraphicSet( wk );
  BBAG_MsgManSet( wk );

  if( wk->dat->mode == BBAG_MODE_GETDEMO ){
		BattleBag_GetDemoPocketInit( wk );
	}else{
	  BattleBag_PocketInit( wk );
	}

  BattleBag_BmpInit( wk );
  BattleBag_BmpWrite( wk, wk->page );
  BBAGBMP_SetStrScrn( wk );

  BBAGANM_ButtonInit( wk );
  BBAGANM_PageButtonPut( wk, wk->page );

  BattleBag_ObjInit( wk );
  BattleBag_PageObjSet( wk, wk->page );

  if( *wk->dat->cursor_flg == 1 ){
    wk->cursor_flg = TRUE;
  }else{
    wk->cursor_flg = FALSE;
  }
  BAPPTOOL_VanishCursor( wk->cpwk, wk->cursor_flg );

	if( wk->page == BBAG_PAGE_POCKET ){
		BBAGUI_Init( wk, wk->page, wk->used_poke );
	}else{
		BBAGUI_Init( wk, wk->page, 0 );
	}

	GFL_NET_ReloadIconTopOrBottom( FALSE, wk->dat->heap );
  PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_OBJ, 14*16, 0x20 );

  PaletteFadeReq(
    wk->pfd, PF_BIT_SUB_ALL, 0xffff, BATTLE_BAGLIST_FADE_SPEED, 16, 0, 0, BTLV_EFFECT_GetTCBSYS() );
/*
  if( wk->dat->mode == BBAG_MODE_GETDEMO ){
    return SEQ_BBAG_GETDEMO;
  }else if( wk->dat->mode == BBAG_MODE_PDC ){
		BBAG_PageChgBgScroll( wk, wk->page );
	  return SEQ_BBAG_ITEM;
	}
  return SEQ_BBAG_POCKET;
*/
	if( wk->dat->mode == BBAG_MODE_PDC ){
		BBAG_PageChgBgScroll( wk, wk->page );
	}

	wk->initFlag = TRUE;

	return SEQ_BBAG_INIT_PRINT_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		初期化シーケンス（シューター用）
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqShooterInit( BBAG_WORK * wk )
{
  G2S_BlendNone();
	GXS_SetMasterBrightness( -16 );

  wk->tcbl = GFL_TCBL_Init( wk->dat->heap, wk->dat->heap, 1, 4 );

  wk->cpwk = BAPPTOOL_CreateCursor( wk->dat->heap );

  BBAG_BgInit( wk );
  BBAG_BgGraphicSet( wk );
  BBAG_MsgManSet( wk );

  BattleBag_ShooterPocketInit( wk );

  BBAG_PageChgBgScroll( wk, wk->page );

  BattleBag_BmpInit( wk );
  BattleBag_BmpWrite( wk, wk->page );
  BBAGBMP_SetStrScrn( wk );

  BBAGANM_ButtonInit( wk );
  BBAGANM_PageButtonPut( wk, wk->page );

  BattleBag_ObjInit( wk );
  BattleBag_PageObjSet( wk, wk->page );

  if( *wk->dat->cursor_flg == 1 ){
    wk->cursor_flg = TRUE;
  }else{
    wk->cursor_flg = FALSE;
  }
  BAPPTOOL_VanishCursor( wk->cpwk, wk->cursor_flg );

  BBAGUI_Init( wk, wk->page, 0 );

	GFL_NET_ReloadIconTopOrBottom( FALSE, wk->dat->heap );
  PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_OBJ, 14*16, 0x20 );

  PaletteFadeReq(
    wk->pfd, PF_BIT_SUB_ALL, 0xffff, BATTLE_BAGLIST_FADE_SPEED, 16, 0, 0, BTLV_EFFECT_GetTCBSYS() );

	wk->initFlag = TRUE;

	return SEQ_BBAG_INIT_PRINT_WAIT;
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
static int BBAG_SeqInitPrintWait( BBAG_WORK * wk )
{
  if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
		WIPE_SYS_Start(
			WIPE_PATTERN_S, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
			WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->dat->heap );
		return SEQ_BBAG_INIT_WIPE_WAIT;
	}
	return SEQ_BBAG_INIT_PRINT_WAIT;
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
static int BBAG_SeqInitWipeWait( BBAG_WORK * wk )
{
	if( WIPE_SYS_EndCheck() == TRUE ){
		// 捕獲デモ
	  if( wk->dat->mode == BBAG_MODE_GETDEMO ){
	    return SEQ_BBAG_GETDEMO;
		// シューター
		}else if( wk->dat->mode == BBAG_MODE_SHOOTER ){
			return SEQ_BBAG_ITEM;
		// ポケモンドリームキャッチ
		}else if( wk->dat->mode == BBAG_MODE_PDC ){
		  return SEQ_BBAG_ITEM;
		// その他
	  }else{
		  return SEQ_BBAG_POCKET;
	  }
	}
	return SEQ_BBAG_INIT_WIPE_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ１のコントロールシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqPokeSelect( BBAG_WORK * wk )
{
  if( PaletteFadeCheck( wk->pfd ) == 0 ){

    u32 ret;
		
		if( CheckTimeOut( wk ) == TRUE ){
      return SEQ_BBAG_ENDSET;
		}

		ret = CURSORMOVE_MainCont( wk->cmwk );

    switch( ret ){
    case BBAG_UI_P1_HP_POCKET:        // HP回復ポケット
    case BBAG_UI_P1_STATUS_POCKET:    // 状態回復ポケット
    case BBAG_UI_P1_BALL_POCKET:      // ボールポケット
    case BBAG_UI_P1_BATTLE_POCKET:    // 戦闘用ポケット
      PlaySE( wk, SEQ_SE_DECIDE2 );
      wk->poke_id = (u8)ret;
      wk->ret_seq = SEQ_BBAG_PAGE2_CHG;
      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_POCKET01+ret-BBAG_POKE_HPRCV );
      return SEQ_BBAG_BUTTON_WAIT;

    case BBAG_UI_P1_LAST_ITEM:    // 最後に使用した道具
      if( wk->used_item != ITEM_DUMMY_DATA ){
        PlaySE( wk, SEQ_SE_DECIDE2 );
        wk->poke_id = wk->used_poke;
        wk->ret_seq = SEQ_BBAG_PAGE3_CHG;
        BattleBag_CorsorReset( wk );
        BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_USE );
        return SEQ_BBAG_BUTTON_WAIT;
      }
      break;

    case BBAG_UI_P1_RETURN:   // 戻る
			PlaySE( wk, SEQ_SE_CANCEL2 );
      wk->dat->ret_item = ITEM_DUMMY_DATA;
      wk->dat->ret_page = BBAG_POKE_MAX;
      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_RETURN );
      return SEQ_BBAG_ENDSET;

    case CURSORMOVE_CANCEL:   // キャンセル
			PlaySE( wk, SEQ_SE_CANCEL2 );
      wk->dat->ret_item = ITEM_DUMMY_DATA;
      wk->dat->ret_page = BBAG_POKE_MAX;
      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_RETURN );
      return SEQ_BBAG_ENDSET;

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
  }

  return SEQ_BBAG_POCKET;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ２のコントロールシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqItemSelect( BBAG_WORK * wk )
{
  if( PaletteFadeCheck( wk->pfd ) == 0 ){
	  u32 ret;
	
		if( CheckTimeOut( wk ) == TRUE ){
			return SEQ_BBAG_ENDSET;
		}

		ret = CURSORMOVE_MainCont( wk->cmwk );

	  switch( ret ){
	  case BBAG_UI_P2_ITEM1:    // アイテム１
	  case BBAG_UI_P2_ITEM2:    // アイテム２
	  case BBAG_UI_P2_ITEM3:    // アイテム３
	  case BBAG_UI_P2_ITEM4:    // アイテム４
	  case BBAG_UI_P2_ITEM5:    // アイテム５
	  case BBAG_UI_P2_ITEM6:    // アイテム６
	    if( BattleBag_PosItemCheck( wk, ret ) != 0 ){
	      PlaySE( wk, SEQ_SE_DECIDE2 );
	      wk->dat->item_pos[wk->poke_id] = (u8)ret;
	      wk->ret_seq = SEQ_BBAG_PAGE3_CHG;
	      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_ITEM01+ret );
	      return SEQ_BBAG_BUTTON_WAIT;
	    }
	    break;

	  case BBAG_UI_P2_RETURN:   // 戻る
			PlaySE( wk, SEQ_SE_CANCEL2 );
	    BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_RETURN );
	    if( wk->dat->mode == BBAG_MODE_SHOOTER || wk->dat->mode == BBAG_MODE_PDC ){
	      wk->dat->ret_item = ITEM_DUMMY_DATA;
	      wk->dat->ret_page = BBAG_POKE_MAX;
	      return SEQ_BBAG_ENDSET;
	    }else{
	      wk->ret_seq = SEQ_BBAG_PAGE1_CHG;
	      return SEQ_BBAG_BUTTON_WAIT;
	    }
	    break;

	  case CURSORMOVE_CANCEL:   // キャンセル
			PlaySE( wk, SEQ_SE_CANCEL2 );
	    BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_RETURN );
	    if( wk->dat->mode == BBAG_MODE_SHOOTER || wk->dat->mode == BBAG_MODE_PDC ){
	      wk->dat->ret_item = ITEM_DUMMY_DATA;
	      wk->dat->ret_page = BBAG_POKE_MAX;
	      return SEQ_BBAG_ENDSET;
	    }else{
	      wk->ret_seq = SEQ_BBAG_PAGE1_CHG;
	      return SEQ_BBAG_BUTTON_WAIT;
	    }
	    break;

	  case BBAG_UI_P2_LEFT:     // 前へ
	    if( wk->scr_max[wk->poke_id] != 0 ){
	      PlaySE( wk, SEQ_SE_DECIDE2 );
	      wk->dat->item_pos[wk->poke_id] = 0;
	      wk->ret_seq = SEQ_BBAG_ITEMSEL_NEXT;
	      wk->page_mv = -1;
	      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_LEFT );
	      return SEQ_BBAG_BUTTON_WAIT;
	    }
	    break;

	  case BBAG_UI_P2_RIGHT:    // 次へ
	    if( wk->scr_max[wk->poke_id] != 0 ){
	      PlaySE( wk, SEQ_SE_DECIDE2 );
	      wk->dat->item_pos[wk->poke_id] = 0;
	      wk->ret_seq = SEQ_BBAG_ITEMSEL_NEXT;
	      wk->page_mv = 1;
	      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_RIGHT );
	      return SEQ_BBAG_BUTTON_WAIT;
	    }
	    break;

	  case CURSORMOVE_CURSOR_MOVE:    // 移動
	  case CURSORMOVE_CURSOR_ON:      // カーソル表示
	    PlaySE( wk, SEQ_SE_SELECT1 );
	    break;

	  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
	  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
	  case CURSORMOVE_NONE:           // 動作なし
	    break;

	  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
	    {
	      u8  pos = CURSORMOVE_PosGet( wk->cmwk );
	      if( pos == BBAG_UI_P2_ITEM1 || pos == BBAG_UI_P2_ITEM3 || BBAG_UI_P2_ITEM5 ){
	        if( wk->scr_max[wk->poke_id] != 0 ){
	        PlaySE( wk, SEQ_SE_DECIDE2 );
	          wk->ret_seq = SEQ_BBAG_ITEMSEL_NEXT;
	          wk->page_mv = -1;
	          BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_LEFT );
	          return SEQ_BBAG_BUTTON_WAIT;
	        }
	      }
	    }
	    break;

	  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
	    {
	      u8  pos = CURSORMOVE_PosGet( wk->cmwk );
	      if( pos == BBAG_UI_P2_ITEM2 || pos == BBAG_UI_P2_ITEM4 || BBAG_UI_P2_ITEM6 ){
	        if( wk->scr_max[wk->poke_id] != 0 ){
	        PlaySE( wk, SEQ_SE_DECIDE2 );
	          wk->ret_seq = SEQ_BBAG_ITEMSEL_NEXT;
	          wk->page_mv = 1;
	          BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_RIGHT );
	          return SEQ_BBAG_BUTTON_WAIT;
	        }
	      }
	    }
	    break;
	  }
	}

  return SEQ_BBAG_ITEM;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		次のアイテムページへ
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqItemSelNext( BBAG_WORK * wk )
{
  s8  scr = wk->dat->item_scr[wk->poke_id];

  scr += wk->page_mv;
  if( scr > wk->scr_max[wk->poke_id] ){
    wk->dat->item_scr[wk->poke_id] = 0;
  }else if( scr < 0 ){
    wk->dat->item_scr[wk->poke_id] = wk->scr_max[wk->poke_id];
  }else{
    wk->dat->item_scr[wk->poke_id] = scr;
  }
  BattleBag_Page2_StrItemPut( wk );
  BattleBag_Page2_StrPageNumPut( wk );
  return SEQ_BBAG_ITEMSEL_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		次のアイテムページへ（キュー待ち）
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqItemSelWait( BBAG_WORK * wk )
{
  if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
    BBAGBMP_SetStrScrn( wk );
    BattleBag_PageObjSet( wk, wk->page );
    BBAGANM_PageButtonPut( wk, wk->page );
    return SEQ_BBAG_ITEM;
  }
  return SEQ_BBAG_ITEMSEL_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ３のコントロールシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqUseSelect( BBAG_WORK * wk )
{
  u32 ret;

	if( CheckTimeOut( wk ) == TRUE ){
		return SEQ_BBAG_ENDSET;
	}

	ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BBAG_UI_P3_USE:    // 使う
    PlaySE( wk, SEQ_SE_DECIDE2 );
    wk->dat->ret_item = BattleBag_PosItemCheck( wk, wk->dat->item_pos[wk->poke_id] );
    wk->dat->ret_page = wk->poke_id;
    if( wk->dat->mode == BBAG_MODE_SHOOTER ){
      wk->dat->ret_cost = (u8)BBAGITEM_GetCost( wk->dat->ret_item );
    }else{
      wk->dat->ret_cost = 0;
    }
    BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_USE );
    return BBAG_ItemUse( wk );

  case BBAG_UI_P3_RETURN:   // 戻る
		PlaySE( wk, SEQ_SE_CANCEL2 );
    wk->ret_seq = SEQ_BBAG_PAGE2_CHG;
    BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_RETURN );
    return SEQ_BBAG_BUTTON_WAIT;

  case CURSORMOVE_CANCEL:   // キャンセル
		PlaySE( wk, SEQ_SE_CANCEL2 );
    wk->ret_seq = SEQ_BBAG_PAGE2_CHG;
    BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_RETURN );
    return SEQ_BBAG_BUTTON_WAIT;

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

  return SEQ_BBAG_USE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		アイテム使用
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_ItemUse( BBAG_WORK * wk )
{
	BBAG_DATA * dat = wk->dat;

	// シューター時
	if( dat->mode == BBAG_MODE_SHOOTER ){
		// エネルギーが足りない
		if( dat->energy < BBAGITEM_GetCost( dat->ret_item ) ){
			GFL_MSG_GetString( wk->mman, mes_b_bag_m16, wk->msg_buf );
			BattleBag_TalkMsgSet( wk );
			wk->ret_seq = SEQ_BBAG_ERR;
			return SEQ_BBAG_MSG_WAIT;
		}
	}

	// ボール使用チェック
	if( wk->poke_id == BBAG_POKE_BALL ){
		// 手持ち・ボックスに空きがない
		if( dat->ball_use == BBAG_BALLUSE_POKEMAX ){
			GFL_MSG_GetString( wk->mman, mes_b_bag_m12, wk->msg_buf );
			BattleBag_TalkMsgSet( wk );
			wk->ret_seq = SEQ_BBAG_ERR;
			return SEQ_BBAG_MSG_WAIT;
		}
		// 野生ダブル・敵が２匹のため使用不可
		if( dat->ball_use == BBAG_BALLUSE_DOUBLE ){
			GFL_MSG_GetString( wk->mman, mes_b_bag_m11, wk->msg_buf );
			BattleBag_TalkMsgSet( wk );
			wk->ret_seq = SEQ_BBAG_ERR;
			return SEQ_BBAG_MSG_WAIT;
		}
		// 野生ダブル・先頭のポケモン動作でないので使用不可
		if( dat->ball_use == BBAG_BALLUSE_NOT_FIRST ){
			GFL_MSG_GetString( wk->mman, mes_b_bag_m18, wk->msg_buf );
			BattleBag_TalkMsgSet( wk );
			wk->ret_seq = SEQ_BBAG_ERR;
			return SEQ_BBAG_MSG_WAIT;
		}
		//「そらをとぶ」などで場に相手がいないので使用不可
		if( dat->ball_use == BBAG_BALLUSE_POKE_NONE ){
			GFL_MSG_GetString( wk->mman, mes_b_bag_m14, wk->msg_buf );
			BattleBag_TalkMsgSet( wk );
			wk->ret_seq = SEQ_BBAG_ERR;
			return SEQ_BBAG_MSG_WAIT;
		}
	}

	// 逃げるアイテム使用チェック
	if( dat->wild_flg == 0 && ITEM_GetParam(dat->ret_item,ITEM_PRM_BATTLE,dat->heap) == ITEMUSE_BTL_ESCAPE ){
		GFL_MSG_GetString( wk->mman, mes_b_bag_m17, wk->msg_buf );
		BattleBag_TalkMsgSet( wk );
		wk->ret_seq = SEQ_BBAG_ERR;
		return SEQ_BBAG_MSG_WAIT;
	}

  return SEQ_BBAG_ENDSET;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え：ポケット選択ページへ
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqPage1Chg( BBAG_WORK * wk )
{
  BattleBag_BmpWrite( wk, BBAG_PAGE_POCKET );
  wk->page = BBAG_PAGE_POCKET;
  wk->ret_seq = SEQ_BBAG_POCKET;
  return SEQ_BBAG_PAGECHG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え：アイテム選択ページへ
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqPage2Chg( BBAG_WORK * wk )
{
  BattleBag_BmpWrite( wk, BBAG_PAGE_MAIN );
  wk->page = BBAG_PAGE_MAIN;
  wk->ret_seq = SEQ_BBAG_ITEM;
  return SEQ_BBAG_PAGECHG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え：アイテム使用ページへ
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqPage3Chg( BBAG_WORK * wk )
{
  BattleBag_BmpWrite( wk, BBAG_PAGE_ITEM );
  wk->page = BBAG_PAGE_ITEM;
  wk->ret_seq = SEQ_BBAG_USE;
  return SEQ_BBAG_PAGECHG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ページ切り替え：アイテム使用ページへ（キュー待ち）
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqPageChgWait( BBAG_WORK * wk )
{
  if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
    BBAG_PageChange( wk, wk->page );
    return wk->ret_seq;
  }
  return SEQ_BBAG_PAGECHG_WAIT;
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
static int BBAG_SeqError( BBAG_WORK * wk )
{
  BmpWinFrame_Clear( wk->talk_win, WINDOW_TRANS_ON );
  return SEQ_BBAG_USE;
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
static int BBAG_SeqMsgWait( BBAG_WORK * wk )
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
    return SEQ_BBAG_TRG_WAIT;
  }

  return SEQ_BBAG_MSG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		トリガーウェイトシーケンス
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqTrgWait( BBAG_WORK * wk )
{
	if( CheckTimeOut( wk ) == TRUE ){
		return SEQ_BBAG_ENDSET;
	}

  if( ( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ) || GFL_UI_TP_GetTrg() == TRUE ){
    return wk->ret_seq;
  }
  return SEQ_BBAG_TRG_WAIT;
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
static int BBAG_SeqButtonWait( BBAG_WORK * wk )
{
  if( wk->btn_flg == 0 ){
    return wk->ret_seq;
  }
  return SEQ_BBAG_BUTTON_WAIT;
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
static int BBAG_SeqEndSet( BBAG_WORK * wk )
{
  PaletteFadeReq(
    wk->pfd, PF_BIT_SUB_ALL, 0xffff, BATTLE_BAGLIST_FADE_SPEED, 0, 16, 0, BTLV_EFFECT_GetTCBSYS() );

  return SEQ_BBAG_ENDWAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		終了フェード待ち
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqEndWait( BBAG_WORK * wk )
{
  if( PaletteFadeCheck( wk->pfd ) == 0 ){
    return SEQ_BBAG_END;
  }
  return SEQ_BBAG_ENDWAIT;
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
static BOOL BBAG_SeqEnd( GFL_TCB * tcb, BBAG_WORK * wk )
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

	  BBAGANM_ButtonExit( wk );

	  BattleBag_ObjFree( wk );
	  BattleBag_BmpFreeAll( wk );
	  BBAG_MsgManExit( wk );
	  BBAG_BgExit();

	  GFL_TCBL_Exit( wk->tcbl );
	  BBAGUI_Exit( wk );
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
 * @brief		捕獲デモメイン
 *
 * @param		wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqGetDemoMain( BBAG_WORK * wk )
{
  switch( wk->get_seq ){
	case 0:
		if( BTLV_FINGER_CURSOR_CheckExecute( wk->getdemoCursor ) == TRUE ){
      PlaySE( wk, SEQ_SE_DECIDE2 );
      wk->poke_id = BBAG_POKE_BALL;
      wk->ret_seq = SEQ_BBAG_GETDEMO;
      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_POCKET03 );
      wk->get_seq++;
      return SEQ_BBAG_BUTTON_WAIT;
		}
		break;

  case 1:
		if( BTLV_FINGER_CURSOR_CheckExecute( wk->getdemoCursor ) == FALSE ){
	    BBAG_SeqPage2Chg( wk );
	    wk->get_seq++;
			wk->ret_seq = SEQ_BBAG_GETDEMO;
		  return SEQ_BBAG_PAGECHG_WAIT;
		}
		break;

	case 2:
		if( BTLV_FINGER_CURSOR_CheckExecute( wk->getdemoCursor ) == TRUE ){
      PlaySE( wk, SEQ_SE_DECIDE2 );
      wk->dat->item_pos[wk->poke_id] = 0;
      wk->ret_seq = SEQ_BBAG_GETDEMO;
      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_ITEM01 );
      wk->get_seq++;
      return SEQ_BBAG_BUTTON_WAIT;
		}
		break;

  case 3:
		if( BTLV_FINGER_CURSOR_CheckExecute( wk->getdemoCursor ) == FALSE ){
	    BBAG_SeqPage3Chg( wk );
	    wk->get_seq++;
			wk->ret_seq = SEQ_BBAG_GETDEMO;
		  return SEQ_BBAG_PAGECHG_WAIT;
		}
		break;

	case 4:
		if( BTLV_FINGER_CURSOR_CheckExecute( wk->getdemoCursor ) == TRUE ){
      PlaySE( wk, SEQ_SE_DECIDE2 );
      wk->dat->ret_item = BattleBag_PosItemCheck( wk, wk->dat->item_pos[wk->poke_id] );
      wk->dat->ret_page = wk->poke_id;
      wk->dat->ret_cost = 0;
      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_USE );
	    wk->get_seq++;
		}
		break;

	case 5:
		if( BTLV_FINGER_CURSOR_CheckExecute( wk->getdemoCursor ) == FALSE ){
      return BBAG_ItemUse( wk );
		}
  }

  return SEQ_BBAG_GETDEMO;
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
static void BBAG_BgInit( BBAG_WORK * wk )
{
  { // BG SYSTEM
    GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGModeDisp( &BGsys_data, GFL_BG_SUB_DISP );
  }

  { // BG (CHAR)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xc800, GX_BG_CHARBASE_0x00000,
      0x8000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
  }

  { // BG (CHAR)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x2000, 0,
      GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xd000, GX_BG_CHARBASE_0x00000,
      0x8000,
      GX_BG_EXTPLTT_23, 2, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
  }

  { // PARAM FONT (BMP)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x10000,
      0x8000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( GFL_BG_FRAME1_S );
  }

  { // WINDOW (BMP)
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000,
      0x4000,
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
    GX_PLANEMASK_BG2 | GX_PLANEMASK_BG3,
    VISIBLE_ON );
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
static void BBAG_BgExit(void)
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
static void BBAG_BgGraphicSet( BBAG_WORK * wk )
{
  ARCHANDLE * hdl;

  hdl = GFL_ARC_OpenDataHandle( ARCID_B_BAG_GRA, GFL_HEAP_LOWID(wk->dat->heap) );

  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    hdl, NARC_b_bag_gra_b_bag_bg_lz_NCGR,
    GFL_BG_FRAME2_S, 0, 0, TRUE, wk->dat->heap );

  GFL_ARCHDL_UTIL_TransVramScreen(
    hdl, NARC_b_bag_gra_b_bag_button_lz_NSCR,
    GFL_BG_FRAME2_S, 0, 0, TRUE, wk->dat->heap );

  GFL_ARCHDL_UTIL_TransVramScreen(
    hdl, NARC_b_bag_gra_b_bag_base_lz_NSCR,
    GFL_BG_FRAME3_S, 0, 0, TRUE, wk->dat->heap );

  PaletteWorkSet_ArcHandle(
      wk->pfd, hdl, NARC_b_bag_gra_b_bag_bg_NCLR,
      wk->dat->heap, FADE_SUB_BG, 0x20*7, 0 );

  GFL_ARC_CloseDataHandle( hdl );

  // メッセージウィンドウ
  BmpWinFrame_GraphicSet(
    GFL_BG_FRAME0_S, 1, BBAG_PAL_TALK_WIN, MENU_TYPE_SYSTEM, wk->dat->heap );
  PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_BG, BBAG_PAL_TALK_WIN*16, 0x20 );

  // フォントパレット
  PaletteWorkSet_Arc(
      wk->pfd, ARCID_FONT, NARC_font_default_nclr,
      wk->dat->heap, FADE_SUB_BG, 0x20, BBAG_PAL_TALK_FONT*16 );
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
static void BBAG_MsgManSet( BBAG_WORK * wk )
{
  wk->mman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_b_bag_dat, wk->dat->heap );
  wk->wset = WORDSET_Create( wk->dat->heap );
  wk->que  = PRINTSYS_QUE_Create( wk->dat->heap );

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
static void BBAG_MsgManExit( BBAG_WORK * wk )
{
  GFL_MSG_Delete( wk->mman );
  WORDSET_Delete( wk->wset );
  PRINTSYS_QUE_Delete( wk->que );

  GFL_STR_DeleteBuffer( wk->msg_buf );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief		BGスクロール
 *
 * @param		wk    ワーク
 * @param		page  次のページ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_PageChgBgScroll( BBAG_WORK * wk, u8 page )
{
  switch( page ){
  case BBAG_PAGE_POCKET:    // ポケット選択ページ
    GFL_BG_SetScrollReq( GFL_BG_FRAME2_S, GFL_BG_SCROLL_X_SET, 0 );
    GFL_BG_SetScrollReq( GFL_BG_FRAME2_S, GFL_BG_SCROLL_Y_SET, 0 );
    break;
  case BBAG_PAGE_MAIN:    // アイテム選択ページ
    GFL_BG_SetScrollReq( GFL_BG_FRAME2_S, GFL_BG_SCROLL_X_SET, 256 );
    GFL_BG_SetScrollReq( GFL_BG_FRAME2_S, GFL_BG_SCROLL_Y_SET, 0 );
    break;
  case BBAG_PAGE_ITEM:    // アイテム使用ページ
    GFL_BG_SetScrollReq( GFL_BG_FRAME2_S, GFL_BG_SCROLL_X_SET, 0 );
    GFL_BG_SetScrollReq( GFL_BG_FRAME2_S, GFL_BG_SCROLL_Y_SET, 256 );
    break;
  }
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
static void BBAG_PageChange( BBAG_WORK * wk, u8 next_page )
{
  BBAG_PageChgBgScroll( wk, next_page );

  GFL_BG_ClearScreenCodeVReq( GFL_BG_FRAME0_S, 0 );
  GFL_BG_ClearScreenCodeVReq( GFL_BG_FRAME1_S, 0 );

  BBAGBMP_SetStrScrn( wk );

  BBAGANM_PageButtonPut( wk, next_page );

  BBAGUI_ChangePage( wk, next_page, 0 );

  BattleBag_PageObjSet( wk, next_page );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		強制終了チェック
 *
 * @param		wk		ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static BOOL CheckTimeOut( BBAG_WORK * wk )
{
	if( wk->dat->time_out_flg == TRUE ){
		wk->dat->ret_item = ITEM_DUMMY_DATA;
		wk->dat->ret_cost = 0;
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＳＥ再生
 *
 * @param		wk		ワーク
 * @param		no		ＳＥ番号
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void PlaySE( BBAG_WORK * wk, int no )
{
	if( wk->dat->seFlag == TRUE ){
		PMSND_PlaySE( no );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＳＥ再生（システム）
 *
 * @param		wk    ワーク
 * @param		no		ＳＥ番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void PlaySystemSE( BBAG_WORK * wk, int no )
{
	if( wk->dat->seFlag == TRUE ){
		PMSND_PlaySystemSE( no );
	}
}
