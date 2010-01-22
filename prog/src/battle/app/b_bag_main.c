//============================================================================================
/**
 * @file  b_bag_main.c
 * @brief 戦闘用バッグ画面
 * @author  Hiroyuki Nakamura
 * @date  05.02.10
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "sound/pm_sndsys.h"
#include "font/font.naix"
//#include "savedata/myitem_savedata.h"

#include "../btlv/btlv_effect.h"
/*
#include "gflib/touchpanel.h"
*/
//#include "system/lib_pack.h"
/*↑[GS_CONVERT_TAG]*/
/*
#include "system/window.h"
*/
//#include "system/arc_tool.h"
/*↑[GS_CONVERT_TAG]*/
//#include "system/arc_util.h"
/*↑[GS_CONVERT_TAG]*/
/*
#include "system/fontproc.h"
#include "system/msgdata.h"
#include "system/numfont.h"
#include "system/wordset.h"
#include "system/font_arc.h"
#include "system/snd_tool.h"
#include "battle/battle_common.h"
#include "battle/fight_tool.h"
#include "battle/server_tool.h"
#include "battle/battle_server.h"
#include "battle/wazano_def.h"
#include "application/app_tool.h"
#include "msgdata/msg.naix"
*/
#include "msg/msg_b_bag.h"
/*
#include "msg/msg_common_scr.h"
#include "itemtool/item.h"
#include "itemtool/myitem.h"
#include "itemtool/itemuse_def.h"
*/
#include "b_app_tool.h"

//#include "system/procsys.h"
/*↑[GS_CONVERT_TAG]*/
//#include "system/clact_tool.h"
/*↑[GS_CONVERT_TAG]*/
/*
#include "poketool/pokeparty.h"
#include "application/p_status.h"
*/

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
  SEQ_BBAG_INIT = 0,      // 初期化
  SEQ_BBAG_SHOOTER_INIT,  // 初期化（シューター用）
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

//#define BATTLE_BAGLIST_FADE_SPEED (-8)
#define BATTLE_BAGLIST_FADE_SPEED (0)


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void BattleBag_Main( GFL_TCB* tcb, void * work );
/*↑[GS_CONVERT_TAG]*/

static int BBAG_SeqInit( BBAG_WORK * wk );
static int BBAG_SeqShooterInit( BBAG_WORK * wk );
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
/*↑[GS_CONVERT_TAG]*/

//static void BBAG_VramInit(void);
static void BBAG_BgInit( BBAG_WORK * wk );
static void BBAG_BgExit(void);
static void BBAG_BgGraphicSet( BBAG_WORK * wk );
static void BBAG_MsgManSet( BBAG_WORK * wk );
static void BBAG_MsgManExit( BBAG_WORK * wk );
static int BBAG_ItemUse( BBAG_WORK * wk );

static void BBAG_PageChgBgScroll( BBAG_WORK * wk, u8 page );
static void BBAG_PageChange( BBAG_WORK * wk, u8 next_page );

static BOOL CheckTimeOut( BBAG_WORK * wk );

//static void BattleBag_SubItem( BATTLE_WORK * bw, u16 item, u16 page, u32 heap );


//============================================================================================
//  データ
//============================================================================================

// メインシーケンス
static const pBBagFunc MainSeqFunc[] = {
  BBAG_SeqInit,
  BBAG_SeqShooterInit,
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
 * 戦闘用バッグタスク追加
 *
 * @param dat   バッグデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static const u16 testItem[][2] =
{
  { ITEM_MASUTAABOORU, 1 },   // マスターボール
  { ITEM_HAIPAABOORU, 11 },   // ハイパーボール
  { ITEM_SUUPAABOORU, 22 },   // スーパーボール
  { ITEM_MONSUTAABOORU, 22 }, // モンスターボール
  { ITEM_NETTOBOORU, 33 },    // ネットボール
  { ITEM_DAIBUBOORU, 44 },    // ダイブボール
  { ITEM_NESUTOBOORU, 55 },   // ネストボール
  { ITEM_RIPIITOBOORU, 66 },  // リピートボール
  { ITEM_TAIMAABOORU, 77 },   // タイマーボール
  { ITEM_GOOZYASUBOORU, 88 }, // ゴージャスボール
  { ITEM_PUREMIABOORU, 99 },  // プレミアボール
  { ITEM_DAAKUBOORU, 100 },   // ダークボール
  { ITEM_HIIRUBOORU, 111 },   // ヒールボール
  { ITEM_KUIKKUBOORU, 222 },  // クイックボール
  { ITEM_PURESYASUBOORU, 333 }, // プレシャスボール
  { ITEM_KIZUGUSURI, 444 },     // キズぐすり
  { ITEM_DOKUKESI, 555 },       // どくけし
  { ITEM_YAKEDONAOSI, 666 },    // やけどなおし
  { ITEM_KOORINAOSI, 777 },     // こおりなおし
  { ITEM_NEMUKEZAMASI, 888 },   // ねむけざまし
  { ITEM_MAHINAOSI, 999 },      // まひなおし
  { ITEM_KAIHUKUNOKUSURI, 99 }, // かいふくのくすり
  { ITEM_MANTANNOKUSURI, 999 }, // まんたんのくすり
  { ITEM_SUGOIKIZUGUSURI, 99 }, // すごいキズぐすり
  { ITEM_IIKIZUGUSURI, 999 },   // いいキズぐすり
  { ITEM_NANDEMONAOSI, 999 },   // なんでもなおし
  { ITEM_GENKINOKAKERA, 999 },  // げんきのかけら
  { ITEM_GENKINOKATAMARI, 99 }, // げんきのかたまり
  { 0, 0 },
};

void BattleBag_TaskAdd( BBAG_DATA * dat )
{
  BBAG_WORK * wk;

  // メモリリークをチェックするために仮エリアを作成
//  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BATTLE_APP_TEST, 0x70000 );
//  dat->heap = HEAPID_BATTLE_APP_TEST;
  // シューターモードテスト
//  dat->mode = BBAG_MODE_SHOOTER;

  wk = GFL_HEAP_AllocClearMemory( dat->heap, sizeof(BBAG_WORK) );

  GFL_TCB_AddTask( BTLV_EFFECT_GetTCBSYS(), BattleBag_Main, wk, 100 );

  wk->dat = dat;
  wk->pfd = BTLV_EFFECT_GetPfd();

  if( wk->dat->mode == BBAG_MODE_NORMAL ){
    wk->page = BBAG_PAGE_POCKET;
    wk->seq  = SEQ_BBAG_INIT;
  }else{
    wk->page = BBAG_PAGE_MAIN;
    wk->seq  = SEQ_BBAG_SHOOTER_INIT;
  }

//  テスト処理
/*
  {
    u32 i = 0;
    while(1){
      if( testItem[i][0] == 0 || testItem[i][1] == 0 ){
        break;
      }
      MYITEM_AddItem( dat->myitem, testItem[i][0], testItem[i][1], dat->heap );
      i++;
    }
  }
*/

  {
    u32 i;

    for( i=0; i<BATTLE_BAG_POKE_MAX; i++ ){
      MYITEM_BattleBagCursorGet( wk->dat->bagcursor, i, &wk->dat->item_pos[i], &wk->dat->item_scr[i] );
    }
    wk->used_item = MYITEM_BattleBagLastItemGet( wk->dat->bagcursor );
    wk->used_poke = MYITEM_BattleBagLastPageGet( wk->dat->bagcursor );
  }

  BattleBag_UsedItemChack( wk );
//  wk->used_item = 2;
//  wk->used_poke = 1;

//  wk->dat->energy = 7;
//  wk->dat->reserved_energy = 3;


/*
  if( BattleWorkFightTypeGet(wk->dat->bw) & FIGHT_TYPE_GET_DEMO ){
    wk->dat->mode = BBAG_MODE_GETDEMO;
  }
*/

/** デバッグ処理 **/
//  wk->dat->myitem = MyItem_AllocWork( wk->dat->heap );
//  Debug_MyItem_MakeBag( wk->dat->myitem, wk->dat->heap );
//  wk->dat->mode = BBAG_MODE_GETDEMO;
}

//--------------------------------------------------------------------------------------------
/**
 * メインタスク
 *
 * @param tcb
 * @param work
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BattleBag_Main( GFL_TCB * tcb, void * work )
{
  BBAG_WORK * wk = (BBAG_WORK *)work;

  if( wk->seq != SEQ_BBAG_END ){
    wk->seq = MainSeqFunc[wk->seq]( wk );
  }

  if( wk->seq == SEQ_BBAG_END ){
    if( BBAG_SeqEnd( tcb, wk ) == TRUE ){
      return;
    }
  }

  GFL_TCBL_Main( wk->tcbl );

//  BattleBag_ButtonAnmMain( wk );
  BBAGANM_ButtonAnmMain( wk );
  BGWINFRM_MoveMain( wk->bgwfrm );
//  BattleBag_GetDemoCursorAnm( wk );
//  GFL_CLACT_SYS_Main( wk->crp );

  BBAGBMP_PrintMain( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * 初期化シーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqInit( BBAG_WORK * wk )
{
  G2S_BlendNone();

  wk->tcbl = GFL_TCBL_Init( wk->dat->heap, wk->dat->heap, 1, 4 );

//  wk->cmv_wk = BAPP_CursorMoveWorkAlloc( wk->dat->heap );
  wk->cpwk = BAPPTOOL_CreateCursor( wk->dat->heap );

//  BBAG_VramInit();
  BBAG_BgInit( wk );
  BBAG_BgGraphicSet( wk );
  BBAG_MsgManSet( wk );

//  FontProc_LoadFont( FONT_TOUCH, wk->dat->heap );

//  wk->poke_id = (u8)MyItem_BattleBagPocketPagePosGet( BattleWorkBagCursorGet(wk->dat->bw) );
//  wk->poke_id = (u8)MYITEM_BattleBagPocketPagePosGet( wk->cur );
  wk->poke_id = 0;

  BattleBag_PocketInit( wk );

  BattleBag_BmpInit( wk );
  BattleBag_BmpWrite( wk, wk->page );
  BBAGBMP_SetStrScrn( wk );

//  BattleBag_ButtonPageScreenInit( wk, wk->page );
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

/*
  if( wk->dat->cursor_flg != 0 ){
    BAPP_CursorMvWkSetFlag( wk->cmv_wk, 1 );
  }
*/
  BattleBag_CursorMoveSet( wk, wk->page );
  BBAG_GetDemoCursorSet( wk, wk->page );

  GFL_NET_ReloadIcon();
  PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_OBJ, 14*16, 0x20 );

  PaletteFadeReq(
    wk->pfd, PF_BIT_SUB_ALL, 0xffff, BATTLE_BAGLIST_FADE_SPEED, 16, 0, 0, BTLV_EFFECT_GetTCBSYS() );

  if( wk->dat->mode == BBAG_MODE_GETDEMO ){
    return SEQ_BBAG_GETDEMO;
  }
  return SEQ_BBAG_POCKET;
}

static int BBAG_SeqShooterInit( BBAG_WORK * wk )
{
  G2S_BlendNone();

  wk->tcbl = GFL_TCBL_Init( wk->dat->heap, wk->dat->heap, 1, 4 );

//  wk->cmv_wk = BAPP_CursorMoveWorkAlloc( wk->dat->heap );
  wk->cpwk = BAPPTOOL_CreateCursor( wk->dat->heap );

//  BBAG_VramInit();
  BBAG_BgInit( wk );
  BBAG_BgGraphicSet( wk );
  BBAG_MsgManSet( wk );

//  wk->poke_id = (u8)MYITEM_BattleBagPocketPagePosGet( wk->cur );
  wk->poke_id = 0;

  BattleBag_ShooterPocketInit( wk );

  BBAG_PageChgBgScroll( wk, wk->page );

  BattleBag_BmpInit( wk );
  BattleBag_BmpWrite( wk, wk->page );
  BBAGBMP_SetStrScrn( wk );

//  BattleBag_ButtonPageScreenInit( wk, wk->page );
  BBAGANM_ButtonInit( wk );
  BBAGANM_PageButtonPut( wk, wk->page );

  BattleBag_ObjInit( wk );
  BattleBag_PageObjSet( wk, wk->page );

  BBAGUI_Init( wk, wk->page, 0 );

/*
  if( wk->dat->cursor_flg != 0 ){
    BAPP_CursorMvWkSetFlag( wk->cmv_wk, 1 );
  }
*/
  BattleBag_CursorMoveSet( wk, wk->page );
  BBAG_GetDemoCursorSet( wk, wk->page );

  GFL_NET_ReloadIcon();
  PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_OBJ, 14*16, 0x20 );

  PaletteFadeReq(
    wk->pfd, PF_BIT_SUB_ALL, 0xffff, BATTLE_BAGLIST_FADE_SPEED, 16, 0, 0, BTLV_EFFECT_GetTCBSYS() );

  return SEQ_BBAG_ITEM;
}

//--------------------------------------------------------------------------------------------
/**
 * ページ１のコントロールシーケンス
 *
 * @param wk    ワーク
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
      PMSND_PlaySE( SEQ_SE_DECIDE2 );
      wk->poke_id = (u8)ret;
      wk->ret_seq = SEQ_BBAG_PAGE2_CHG;
      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_POCKET01+ret-BBAG_POKE_HPRCV );
      return SEQ_BBAG_BUTTON_WAIT;

    case BBAG_UI_P1_LAST_ITEM:    // 最後に使用した道具
      if( wk->used_item != ITEM_DUMMY_DATA ){
        PMSND_PlaySE( SEQ_SE_DECIDE2 );
        wk->poke_id = wk->used_poke;
        wk->ret_seq = SEQ_BBAG_PAGE3_CHG;
        BattleBag_CorsorReset( wk );
        BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_USE );
        return SEQ_BBAG_BUTTON_WAIT;
      }
      break;

    case BBAG_UI_P1_RETURN:   // 戻る
    case CURSORMOVE_CANCEL:   // キャンセル
      PMSND_PlaySE( SEQ_SE_DECIDE2 );
      wk->dat->ret_item = ITEM_DUMMY_DATA;
      wk->dat->ret_page = BBAG_POKE_MAX;
      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_RETURN );
      return SEQ_BBAG_ENDSET;

    case CURSORMOVE_CURSOR_MOVE:    // 移動
      PMSND_PlaySE( SEQ_SE_SELECT1 );
      break;

    case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
    case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
    case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
    case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
    case CURSORMOVE_CURSOR_ON:      // カーソル表示
    case CURSORMOVE_NONE:           // 動作なし
      break;
    }
  }

  return SEQ_BBAG_POCKET;
}

//--------------------------------------------------------------------------------------------
/**
 * ページ２のコントロールシーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqItemSelect( BBAG_WORK * wk )
{
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
      PMSND_PlaySE( SEQ_SE_DECIDE2 );
      wk->dat->item_pos[wk->poke_id] = (u8)ret;
      wk->ret_seq = SEQ_BBAG_PAGE3_CHG;
      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_ITEM01+ret );
      return SEQ_BBAG_BUTTON_WAIT;
    }
    break;

  case BBAG_UI_P2_RETURN:   // 戻る
  case CURSORMOVE_CANCEL:   // キャンセル
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_RETURN );
    if( wk->dat->mode == BBAG_MODE_SHOOTER ){
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
      PMSND_PlaySE( SEQ_SE_DECIDE2 );
      wk->dat->item_pos[wk->poke_id] = 0;
      wk->ret_seq = SEQ_BBAG_ITEMSEL_NEXT;
      wk->page_mv = -1;
      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_LEFT );
      return SEQ_BBAG_BUTTON_WAIT;
    }
    break;

  case BBAG_UI_P2_RIGHT:    // 次へ
    if( wk->scr_max[wk->poke_id] != 0 ){
      PMSND_PlaySE( SEQ_SE_DECIDE2 );
      wk->dat->item_pos[wk->poke_id] = 0;
      wk->ret_seq = SEQ_BBAG_ITEMSEL_NEXT;
      wk->page_mv = 1;
      BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_RIGHT );
      return SEQ_BBAG_BUTTON_WAIT;
    }
    break;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
    PMSND_PlaySE( SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
  case CURSORMOVE_NONE:           // 動作なし
    break;

  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
    {
      u8  pos = CURSORMOVE_PosGet( wk->cmwk );
      if( pos == BBAG_UI_P2_ITEM1 || pos == BBAG_UI_P2_ITEM3 || BBAG_UI_P2_ITEM5 ){
        if( wk->scr_max[wk->poke_id] != 0 ){
        PMSND_PlaySE( SEQ_SE_DECIDE2 );
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
        PMSND_PlaySE( SEQ_SE_DECIDE2 );
          wk->ret_seq = SEQ_BBAG_ITEMSEL_NEXT;
          wk->page_mv = 1;
          BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_RIGHT );
          return SEQ_BBAG_BUTTON_WAIT;
        }
      }
    }
    break;
  }

  return SEQ_BBAG_ITEM;
}

//--------------------------------------------------------------------------------------------
/**
 * 次のアイテムページへ
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqItemSelNext( BBAG_WORK * wk )
{
  s8  scr = wk->dat->item_scr[wk->poke_id];

//  wk->dat->item_pos[wk->poke_id] = 0;
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
//  BattleBag_PageObjSet( wk, wk->page );
//  BBAGANM_PageButtonPut( wk, wk->page );
//  BattleBag_ButtonPageScreenInit( wk, wk->page );
//  BBAG_P2CursorMvTblMake( wk );
//  return SEQ_BBAG_ITEM;
  return SEQ_BBAG_ITEMSEL_WAIT;
}

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
 * ページ３のコントロールシーケンス
 *
 * @param wk    ワーク
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
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
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
  case CURSORMOVE_CANCEL:   // キャンセル
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    wk->ret_seq = SEQ_BBAG_PAGE2_CHG;
    BBAGANM_ButtonAnmInit( wk, BBAG_BGWF_RETURN );
    return SEQ_BBAG_BUTTON_WAIT;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
    PMSND_PlaySE( SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
  case CURSORMOVE_NONE:           // 動作なし
      break;
  }

  return SEQ_BBAG_USE;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム使用
 *
 * @param wk    ワーク
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

/*
  // エラーメッセージテスト
  GFL_MSG_GetString( wk->mman, mes_b_bag_m01, wk->msg_buf );
  BattleBag_TalkMsgSet( wk );
  wk->ret_seq = SEQ_BBAG_ERR;
  return SEQ_BBAG_MSG_WAIT;
*/

/*
  BBAG_DATA * dat = wk->dat;

  if( wk->poke_id == BBAG_POKE_BATTLE ){
    int smn = BattleBag_SelMonsNoGet( wk );
    u32 prm = ItemParamGet( dat->ret_item, ITEM_PRM_BATTLE, dat->heap );

    // さしおさえ
    if( dat->skill_item_use != 0 && dat->ret_item != ITEM_EFEKUTOGAADO && prm != ITEMUSE_BTL_ESCAPE ){

      POKEMON_PARAM * pp;
      STRBUF * str;

      pp  = BattleWorkPokemonParamGet( dat->bw, dat->client_no, smn );
      str = GFL_MSG_CreateString( wk->mman, mes_b_bag_m13 );
      WORDSET_RegisterPokeNickName( wk->wset, 0, PPPPointerGet(pp) );
      WORDSET_RegisterWazaName( wk->wset, 1, WAZANO_SASIOSAE );
      WORDSET_ExpandStr( wk->wset, wk->msg_buf, str );
      GFL_STR_DeleteBuffer( str );
      BattleBag_TalkMsgSet( wk );
      wk->ret_seq = SEQ_BBAG_ERR;
      return SEQ_BBAG_MSG_WAIT;
    }

    if( BattleWorkStatusRecover(dat->bw,dat->client_no,smn,0,dat->ret_item) == TRUE ){
      BattleBag_SubItem( dat->bw, dat->ret_item, wk->poke_id, dat->heap );
      return SEQ_BBAG_ENDSET;
    }else if( prm == ITEMUSE_BTL_ESCAPE ){
      if( !(BattleWorkFightTypeGet(dat->bw) & FIGHT_TYPE_TRAINER) ){
        BattleBag_SubItem( dat->bw, dat->ret_item, wk->poke_id, dat->heap );
        return SEQ_BBAG_ENDSET;
      }else{
        GFL_MSGDATA * mman;
        STRBUF * str;
        mman = MSGMAN_Create( GflMsgLoadType_DIRECT, ARC_MSG, NARC_msg_common_scr_dat, dat->heap );
        str  = GFL_MSG_CreateString( mman, msg_item_ng_01 );
        WORDSET_RegisterPlayerName( wk->wset, 0, dat->myst );
        WORDSET_ExpandStr( wk->wset, wk->msg_buf, str );
        GFL_STR_DeleteBuffer( str );
        MSGMAN_Delete( mman );
        BattleBag_TalkMsgSet( wk );
        wk->ret_seq = SEQ_BBAG_ERR;
        return SEQ_BBAG_MSG_WAIT;
      }
    }else{
      MSGMAN_GetString( wk->mman, mes_b_bag_m01, wk->msg_buf );
      BattleBag_TalkMsgSet( wk );
      wk->ret_seq = SEQ_BBAG_ERR;
      return SEQ_BBAG_MSG_WAIT;
    }
  }else if( wk->poke_id == BBAG_POKE_BALL ){
    // 野生ダブル
    if( dat->enc_double == 1 ){
      MSGMAN_GetString( wk->mman, mes_b_bag_m11, wk->msg_buf );
      BattleBag_TalkMsgSet( wk );
      wk->ret_seq = SEQ_BBAG_ERR;
      return SEQ_BBAG_MSG_WAIT;
    }

    // 空を飛ぶとか
    if( dat->waza_vanish == 1 ){
      MSGMAN_GetString( wk->mman, mes_b_bag_m14, wk->msg_buf );
      BattleBag_TalkMsgSet( wk );
      wk->ret_seq = SEQ_BBAG_ERR;
      return SEQ_BBAG_MSG_WAIT;
    }

    // 身代わり
    if( dat->waza_migawari == 1 ){
      MSGMAN_GetString( wk->mman, mes_b_bag_m15, wk->msg_buf );
      BattleBag_TalkMsgSet( wk );
      wk->ret_seq = SEQ_BBAG_ERR;
      return SEQ_BBAG_MSG_WAIT;
    }

    { // 手持ち・ボックスいっぱい
      POKEPARTY * party = BattleWorkPokePartyGet( dat->bw, dat->client_no );
      BOX_MANAGER * box = BattleWorkBoxDataGet( dat->bw );

      if( PokeParty_GetPokeCount( party ) == 6 &&
        BOXDAT_GetEmptyTrayNumber( box ) == BOXDAT_TRAYNUM_ERROR ){

        MSGMAN_GetString( wk->mman, mes_b_bag_m12, wk->msg_buf );
        BattleBag_TalkMsgSet( wk );
        wk->ret_seq = SEQ_BBAG_ERR;
        return SEQ_BBAG_MSG_WAIT;
      }
    }
  }
*/
  return SEQ_BBAG_ENDSET;
}

//--------------------------------------------------------------------------------------------
/**
 * ページ切り替え：ポケット選択ページへ
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqPage1Chg( BBAG_WORK * wk )
{
//  BBAG_PageChange( wk, BBAG_PAGE_POCKET );
  BattleBag_BmpWrite( wk, BBAG_PAGE_POCKET );
  wk->page = BBAG_PAGE_POCKET;
  wk->ret_seq = SEQ_BBAG_POCKET;
  return SEQ_BBAG_PAGECHG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * ページ切り替え：アイテム選択ページへ
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqPage2Chg( BBAG_WORK * wk )
{
//  BBAG_PageChange( wk, BBAG_PAGE_MAIN );
  BattleBag_BmpWrite( wk, BBAG_PAGE_MAIN );
  wk->page = BBAG_PAGE_MAIN;
  wk->ret_seq = SEQ_BBAG_ITEM;
  return SEQ_BBAG_PAGECHG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * ページ切り替え：アイテム使用ページへ
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqPage3Chg( BBAG_WORK * wk )
{
//  BBAG_PageChange( wk, BBAG_PAGE_ITEM );
  BattleBag_BmpWrite( wk, BBAG_PAGE_ITEM );
  wk->page = BBAG_PAGE_ITEM;
  wk->ret_seq = SEQ_BBAG_USE;
  return SEQ_BBAG_PAGECHG_WAIT;
}

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
 * 入れ替えエラーメッセージ終了待ちシーケンス
 *
 * @param wk    ワーク
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
 * メッセージ表示シーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqMsgWait( BBAG_WORK * wk )
{
  if( PRINTSYS_PrintStreamGetState(wk->stream) == PRINTSTREAM_STATE_DONE ){
    PRINTSYS_PrintStreamDelete( wk->stream );
    return SEQ_BBAG_TRG_WAIT;
  }
  return SEQ_BBAG_MSG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * トリガーウェイトシーケンス
 *
 * @param wk    ワーク
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
 * ボタンアニメ終了待ちシーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqButtonWait( BBAG_WORK * wk )
{
  if( wk->btn_flg == 0 ){
    return wk->ret_seq;
  }
/*
  if( wk->btn_seq == 2 ){
    return wk->ret_seq;
  }
*/
  return SEQ_BBAG_BUTTON_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * 終了フェードセットシーケンス
 *
 * @param wk    ワーク
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

static int BBAG_SeqEndWait( BBAG_WORK * wk )
{
  if( PaletteFadeCheck( wk->pfd ) == 0 ){
    return SEQ_BBAG_END;
  }
  return SEQ_BBAG_ENDWAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * 終了シーケンス
 *
 * @param wk    ワーク
 *
 * @retval  "TRUE = 終了"
 * @retval  "FALSE = フェード中"
 */
//--------------------------------------------------------------------------------------------
static BOOL BBAG_SeqEnd( GFL_TCB * tcb, BBAG_WORK * wk )
{
  if( PRINTSYS_QUE_IsFinished( wk->que ) == FALSE ){
    return FALSE;
  }

  wk->dat->end_flg = 1;
  if( wk->cursor_flg == TRUE ){
    *wk->dat->cursor_flg = 1;
  }else{
    *wk->dat->cursor_flg = 0;
  }

  BBAGANM_ButtonExit( wk );

  BattleBag_ObjFree( wk );
  BattleBag_BmpFreeAll( wk );
  BBAG_MsgManExit( wk );
  BBAG_BgExit();

  GFL_TCBL_Exit( wk->tcbl );
//  BAPP_CursorMoveWorkFree( wk->cmv_wk );
  BBAGUI_Exit( wk );
  BAPPTOOL_FreeCursor( wk->cpwk );

  GFL_TCB_DeleteTask( tcb );
  GFL_HEAP_FreeMemory( wk );

//  GFL_HEAP_DeleteHeap( HEAPID_BATTLE_APP_TEST );

  return TRUE;

#if 0
  if( PaletteFadeCheck( wk->pfd ) != 0 ){ return FALSE; }

  BattleBag_ObjFree( wk );
  BattleBag_BmpFreeAll( wk );
  BBAG_MsgManExit( wk );
  BBAG_BgExit();

  wk->dat->cursor_flg = BAPP_CursorMvWkGetFlag( wk->cmv_wk );

  BAPP_CursorMoveWorkFree( wk->cmv_wk );

//  FontProc_UnloadFont( FONT_TOUCH );

  if( wk->dat->ret_item != ITEM_DUMMY_DATA ){
/*
    BAG_CURSOR * cur;
    u8  i;

    cur = BattleWorkBagCursorGet( wk->dat->bw );
    for( i=0; i<5; i++ ){
      MyItem_BattleBagCursorSet( cur, i, wk->dat->item_pos[i], wk->dat->item_scr[i] );
    }
    MyItem_BattleBagPocketPagePosSet( cur, wk->poke_id );
*/
  }

  wk->dat->end_flg = 1;
//  PMDS_taskDel( tcb );

  return TRUE;
#endif
}




#define BBAG_GETDEMO_WAIT_COUNT   ( 60 )    // 捕獲デモウェイト

//--------------------------------------------------------------------------------------------
/**
 * 捕獲デモメイン
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BBAG_SeqGetDemoMain( BBAG_WORK * wk )
{
/*
  if( PaletteFadeCheck( wk->pfd ) != 0 ){ return SEQ_BBAG_GETDEMO; }

  switch( wk->get_seq ){
  case 0:   // wait 2 sec
//    if( wk->get_cnt == BBAG_GETDEMO_WAIT_COUNT ){
    if( FINGER_TouchAnimeCheck( wk->finger ) == TRUE ){
      PMSND_PlaySE( SEQ_SE_DECIDE2 );
      wk->poke_id = BBAG_POKE_BALL;
      wk->ret_seq = SEQ_BBAG_GETDEMO;
      BattleBag_ButtonAnmInit( wk, BBAG_BTNANM_PAGE3, BBAG_BTNANM_MODE_END );
//      BattleBag_GetDemoCursorPush( wk );
//      wk->get_anm = 1;
      wk->get_cnt = 0;
      wk->get_seq++;
      return SEQ_BBAG_BUTTON_WAIT;
    }else{
      wk->get_cnt++;
    }
    break;
  case 1:   // page change
//    wk->get_anm = 0;
    BBAG_SeqPage2Chg( wk );
    wk->get_seq++;
    break;

  case 2:   // wait 2 sec
//    if( wk->get_cnt == BBAG_GETDEMO_WAIT_COUNT ){
    if( FINGER_TouchAnimeCheck( wk->finger ) == TRUE ){
      PMSND_PlaySE( SEQ_SE_DECIDE2 );
      wk->dat->item_pos[wk->poke_id] = 0;
      wk->ret_seq = SEQ_BBAG_GETDEMO;
      BattleBag_ButtonAnmInit( wk, BBAG_BTNANM_ITEM1, BBAG_BTNANM_MODE_END );
//      BattleBag_GetDemoCursorPush( wk );
//      wk->get_anm = 1;
      wk->get_cnt = 0;
      wk->get_seq++;
      return SEQ_BBAG_BUTTON_WAIT;
    }else{
      wk->get_cnt++;
    }
    break;
  case 3:   // page change
//    wk->get_anm = 0;
    BBAG_SeqPage3Chg( wk );
    wk->get_seq++;
    break;

  case 4:   // wait 2 sec
//    if( wk->get_cnt == BBAG_GETDEMO_WAIT_COUNT ){
    if( FINGER_TouchAnimeCheck( wk->finger ) == TRUE ){
      PMSND_PlaySE( SEQ_SE_DECIDE2 );
      wk->dat->ret_item = BattleBag_PosItemCheck( wk, wk->dat->item_pos[wk->poke_id] );
      wk->dat->ret_page = wk->poke_id;
      BattleBag_ButtonAnmInit( wk, BBAG_BTNANM_USE, BBAG_BTNANM_MODE_END );
//      BattleBag_GetDemoCursorPush( wk );
//      wk->get_anm = 1;
      return BBAG_ItemUse( wk );
    }else{
      wk->get_cnt++;
    }
    break;
  }
*/
  return SEQ_BBAG_GETDEMO;
}








//--------------------------------------------------------------------------------------------
/**
 * VRAM初期化
 *
 * @param none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
/*
static void BBAG_VramInit(void)
{
  GFL_DISP_VRAM tbl = {
    GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_64_E,       // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,     // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_0_B,        // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_01_FG     // テクスチャパレットスロット
  };
  GFL_DISP_SetBank( &tbl );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * BG初期化
 *
 * @param dat   リストデータ
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
 * BG解放
 *
 * @param ini   BGLデータ
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
 * グラフィックデータセット
 *
 * @param wk    ポケモンリスト画面のワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_BgGraphicSet( BBAG_WORK * wk )
{
  ARCHANDLE * hdl;

  hdl = GFL_ARC_OpenDataHandle( ARCID_B_BAG_GRA, wk->dat->heap );

  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    hdl, NARC_b_bag_gra_b_bag_bg_lz_NCGR,
    GFL_BG_FRAME2_S, 0, 0, TRUE, wk->dat->heap );

  GFL_ARCHDL_UTIL_TransVramScreen(
    hdl, NARC_b_bag_gra_b_bag_button_lz_NSCR,
    GFL_BG_FRAME2_S, 0, 0, TRUE, wk->dat->heap );

  GFL_ARCHDL_UTIL_TransVramScreen(
    hdl, NARC_b_bag_gra_b_bag_base_lz_NSCR,
    GFL_BG_FRAME3_S, 0, 0, TRUE, wk->dat->heap );

/*
  {
    NNSG2dScreenData * dat;
    void * buf;
    u16 * scrn;

    buf = GFL_ARC_LoadDataAllocByHandle(
            hdl, NARC_b_bag_gra_b_bag_anm_NSCR, wk->dat->heap );
    NNS_G2dGetUnpackedScreenData( buf, &dat );
    scrn = (u16 *)dat->rawData;
    BattleBag_ButtonScreenMake( wk, scrn );
    GFL_HEAP_FreeMemory( buf );
  }
*/

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
 * メッセージ関連セット
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_MsgManSet( BBAG_WORK * wk )
{
  wk->mman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_b_bag_dat, wk->dat->heap );
//  wk->nfnt = NUMFONT_Create( 15, 14, FBMP_COL_NULL, wk->dat->heap );
  wk->wset = WORDSET_Create( wk->dat->heap );
  wk->que  = PRINTSYS_QUE_Create( wk->dat->heap );
//  PRINTSYS_QUE_ForceCommMode( wk->que, TRUE );      // テスト

  wk->msg_buf = GFL_STR_CreateBuffer( TMP_MSG_BUF_SIZ, wk->dat->heap );
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージ関連削除
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_MsgManExit( BBAG_WORK * wk )
{
  GFL_MSG_Delete( wk->mman );
//  NUMFONT_Delete( wk->nfnt );
  WORDSET_Delete( wk->wset );
  PRINTSYS_QUE_Delete( wk->que );

  GFL_STR_DeleteBuffer( wk->msg_buf );
}














//--------------------------------------------------------------------------------------------
/**
 * BGスクロール
 *
 * @param wk    ワーク
 * @param page  次のページ
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

/*
#define ITEM_PAGE_NAME_BG_PX  ( 2 )
#define ITEM_PAGE_NAME_BG_PY  ( 35 )
#define ITEM_PAGE_NAME_BG_SX  ( 28 )
#define ITEM_PAGE_NAME_BG_SY  ( 4 )
#define ITEM_PAGE_INFO_BG_PX  ( 2 )
#define ITEM_PAGE_INFO_BG_PY  ( 40 )
#define ITEM_PAGE_INFO_BG_SX  ( 28 )
#define ITEM_PAGE_INFO_BG_SY  ( 8 )
*/
//--------------------------------------------------------------------------------------------
/**
 * アイテム使用ページのBGパレット変更
 *
 * @param wk    ワーク
 * @param page  ページID
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
/*
static void BBAG_PageBgPalChg( BBAG_WORK * wk, u8 page )
{
  if( page != BBAG_PAGE_ITEM ){ return; }

  GFL_BG_ChangeScreenPalette(
    GFL_BG_FRAME2_S, ITEM_PAGE_NAME_BG_PX,
    ITEM_PAGE_NAME_BG_PY, ITEM_PAGE_NAME_BG_SX, ITEM_PAGE_NAME_BG_SY, 8+wk->poke_id );
  GFL_BG_ChangeScreenPalette(
    GFL_BG_FRAME2_S, ITEM_PAGE_INFO_BG_PX,
    ITEM_PAGE_INFO_BG_PY, ITEM_PAGE_INFO_BG_SX, ITEM_PAGE_INFO_BG_SY, 8+wk->poke_id );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * ページ切り替え
 *
 * @param wk      ワーク
 * @param next_page 次のページ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BBAG_PageChange( BBAG_WORK * wk, u8 next_page )
{
//  BBAG_PageBgPalChg( wk, next_page );
  BBAG_PageChgBgScroll( wk, next_page );

  GFL_BG_ClearScreenCodeVReq( GFL_BG_FRAME0_S, 0 );
  GFL_BG_ClearScreenCodeVReq( GFL_BG_FRAME1_S, 0 );

//  BattleBag_BmpFree( wk );
//  BattleBag_BmpAdd( wk, next_page );
//  BattleBag_BmpWrite( wk, next_page );
  BBAGBMP_SetStrScrn( wk );

//  BattleBag_ButtonPageScreenInit( wk, next_page );
  BBAGANM_PageButtonPut( wk, next_page );

  BBAGUI_ChangePage( wk, next_page, 0 );

  BattleBag_CursorMoveSet( wk, next_page );
  BBAG_GetDemoCursorSet( wk, next_page );

//  wk->page = next_page;

//  BattleBag_PageObjSet( wk, wk->page );
  BattleBag_PageObjSet( wk, next_page );
}

//--------------------------------------------------------------------------------------------
/**
 * 使用するポケモンを取得
 *
 * @param wk    ワーク
 *
 * @return  ポケモン位置
 */
//--------------------------------------------------------------------------------------------
int BattleBag_SelMonsNoGet( BBAG_WORK * wk )
{
  return 0;
/*
  int ret;

  ret = ST_ServerParamDataGet(
      wk->dat->bw,
      BattleWorkServerParamGet(wk->dat->bw), ID_SP_sel_mons_no, wk->dat->client_no );

  return ret;
*/
}

//--------------------------------------------------------------------------------------------
/**
 * アイテムを減らす
 *
 * @param bw    戦闘のワーク
 * @param item  アイテム番号
 * @param page  ポケット番号
 * @param heap  ヒープID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
/*
static void BattleBag_SubItem( BATTLE_WORK * bw, u16 item, u16 page, u32 heap )
{
  MyItem_SubItem( BattleWorkMyItemGet(bw), item, 1, heap );
  MyItem_BattleBagLastItemSet( BattleWorkBagCursorGet(bw), item, page );
}
*/


// 強制終了チェック
static BOOL CheckTimeOut( BBAG_WORK * wk )
{
/*
	// テスト
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		wk->dat->ret_item = ITEM_DUMMY_DATA;
		wk->dat->ret_cost = 0;
		return TRUE;
	}
*/
	if( wk->dat->time_out_flg == TRUE ){
		wk->dat->ret_item = ITEM_DUMMY_DATA;
		wk->dat->ret_cost = 0;
		return TRUE;
	}
	return FALSE;
}

