//============================================================================================
/**
 * @file  b_plist_main.c
 * @brief 戦闘用ポケモンリスト画面
 * @author  Hiroyuki Nakamura
 * @date  05.02.01
 */
//============================================================================================
#include <gflib.h>

#include "arc_def.h"
#include "message.naix"
#include "item/item.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "sound/pm_sndsys.h"
#include "font/font.naix"
#include "waza_tool/wazadata.h"
#include "poke_tool/gauge_tool.h"
#include "app/app_menu_common.h"

#include "../btlv/btlv_effect.h"

/*↑[GS_CONVERT_TAG]*/
/*
#include "gflib/touchpanel.h"
*/
//#include "system/procsys.h"
/*↑[GS_CONVERT_TAG]*/
/*
#include "system/palanm.h"
#include "system/window.h"
*/
//#include "system/lib_pack.h"
/*↑[GS_CONVERT_TAG]*/
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
*/
//#include "system/clact_tool.h"
/*↑[GS_CONVERT_TAG]*/
/*
#include "system/snd_tool.h"
#include "msgdata/msg.naix"
#include "msgdata/msg_b_plist.h"
#include "battle/battle_common.h"
#include "battle/fight_tool.h"
#include "poketool/pokeparty.h"
#include "contest/contest.h"
#include "contest/con_tool.h"
#include "itemtool/item.h"
#include "itemtool/myitem.h"
#include "application/p_status.h"
#include "application/app_tool.h"
#include "b_bag.h"
*/
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

/*
#ifdef PM_DEBUG
#include "debug/d_fight.h"
#endif PM_DEBUG
*/

//============================================================================================
//  定数定義
//============================================================================================
// メインシーケンス
enum {
  SEQ_BPL_INIT = 0,   // 初期化
  SEQ_BPL_SELECT,     // ポケモン選択画面コントロール
  SEQ_BPL_IREKAE,     // 入れ替えページコントロール
  SEQ_BPL_ST_MAIN,    // ステータスメイン画面コントロール
  SEQ_BPL_ST_WAZASEL,   // ステータス技選択画面コントロール
	SEQ_BPL_ST_SKILL,   // ステータス技画面コントロール

  SEQ_BPL_POKESEL_DEAD,		// 瀕死入れ替え選択コントロール
  SEQ_BPL_POKECHG_DEAD,		// 瀕死入れ替えアニメ
  SEQ_BPL_DEAD_ERR_RET,

  SEQ_BPL_PAGE1_CHG,				 // ポケモン選択へ
  SEQ_BPL_PAGECHG_IREKAE,   // 入れ替えページへ
  SEQ_BPL_PAGECHG_STMAIN,   // ステータス詳細へ
  SEQ_BPL_PAGECHG_WAZASEL,  // ステータス技選択へ
  SEQ_BPL_PAGECHG_WAZAINFO, // ステータス技詳細画面へ
  SEQ_BPL_PAGECHG_WAZASET_S,  // 技忘れ選択画面へ
  SEQ_BPL_PAGECHG_WAZASET_I,  // 技忘れ決定へ
  SEQ_BPL_PAGECHG_PPRCV,    // 技回復へ
  SEQ_BPL_PAGECHG_DEAD,    // 瀕死入れ替え選択へ

  SEQ_BPL_POKECHG,    // ポケモン切り替え
  SEQ_BPL_CHG_ERR_SET,  // 入れ替えエラーセット
  SEQ_BPL_IREKAE_ERR,   // 入れ替えエラー終了待ち

  SEQ_BPL_MSG_WAIT,   // メッセージ表示
  SEQ_BPL_TRG_WAIT,   // メッセージ表示後のキー待ち

  SEQ_BPL_WAZADEL_SEL,  // 技忘れ選択
  SEQ_BPL_WAZADEL_MAIN, // 技忘れ決定

  SEQ_BPL_WAZARCV_SEL,  // 技回復選択

  SEQ_BPL_BUTTON_WAIT,  // ボタンアニメ終了待ち

  SEQ_BPL_STRCV,      // ステータス回復
  SEQ_BPL_PPALLRCV,   // PP全回復

  SEQ_BPL_ENDSET,     // 終了フェードセット
  SEQ_BPL_ENDWAIT,    // 終了フェード待ち
  SEQ_BPL_END,        // 終了
};

#define TMP_MSG_BUF_SIZ   ( 512 )   // テンポラリメッセージサイズ

#define PLATE_BG_SX   ( 16 )    // プレートBGサイズX
#define PLATE_BG_SY   ( 6 )   // プレートBGサイズY
#define PLATE_BG_P_PX ( 0 )   // ポケモンがいる場合のプレートBG参照X座標
#define PLATE_BG_P_PY ( 0 )   // ポケモンがいる場合のプレートBG参照Y座標
#define PLATE_BG_N_PX ( 16 )    // ポケモンがいない場合のプレートBG参照X座標
#define PLATE_BG_N_PY ( 13 )    // ポケモンがいない場合のプレートBG参照Y座標

#define PLATE_POKE1_PX  ( 0 )   // ポケモン１のプレートBGX座標
#define PLATE_POKE1_PY  ( 0 )   // ポケモン１のプレートBGY座標
#define PLATE_POKE2_PX  ( 16 )    // ポケモン２のプレートBGX座標
#define PLATE_POKE2_PY  ( 1 )   // ポケモン２のプレートBGY座標
#define PLATE_POKE3_PX  ( 0 )   // ポケモン３のプレートBGX座標
#define PLATE_POKE3_PY  ( 6 )   // ポケモン３のプレートBGY座標
#define PLATE_POKE4_PX  ( 16 )    // ポケモン４のプレートBGX座標
#define PLATE_POKE4_PY  ( 7 )   // ポケモン４のプレートBGY座標
#define PLATE_POKE5_PX  ( 0 )   // ポケモン５のプレートBGX座標
#define PLATE_POKE5_PY  ( 12 )    // ポケモン５のプレートBGY座標
#define PLATE_POKE6_PX  ( 16 )    // ポケモン６のプレートBGX座標
#define PLATE_POKE6_PY  ( 13 )    // ポケモン６のプレートBGY座標

#define P2_EXPGAGE_PX ( 32+14 ) // ページ２の経験値ゲージ書き換えX座標
#define P2_EXPGAGE_PY ( 5 )   // ページ２の経験値ゲージ書き換えY座標
#define P3_EXPGAGE_PX ( 10 )    // ページ３の経験値ゲージ書き換えX座標
#define P3_EXPGAGE_PY ( 8 )   // ページ３の経験値ゲージ書き換えY座標

#define EXP_CGX     ( 0x16 )  // 経験値ゲージキャラ番号
#define EXP_DOT_CHR_MAX ( 8 )   // 経験値ゲージのキャラ数
#define EXP_DOT_MAX   ( 64 )    // 経験値ゲージの最大ドット数

//#define BATTLE_BAGLIST_FADE_SPEED (-8)
#define BATTLE_BAGLIST_FADE_SPEED (0)


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static void BattlePokeList_Main( GFL_TCB* tcb, void * work );

static int BPL_SeqInit( BPLIST_WORK * wk );
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
static int BPL_SeqStRcv( BPLIST_WORK * wk );
static int BPL_SeqPPAllRcv( BPLIST_WORK * wk );
static int BPL_SeqEndSet( BPLIST_WORK * wk );
static int BPL_SeqEndWait( BPLIST_WORK * wk );
static BOOL BPL_SeqEnd( GFL_TCB* tcb, BPLIST_WORK * wk );
/*↑[GS_CONVERT_TAG]*/
static int BPL_SeqWazaSelect( BPLIST_WORK * wk );

static int BPL_PokeItemUse( BPLIST_WORK * wk );

//static void BPL_VramInit(void);
static void BPL_BgInit( BPLIST_WORK * dat );
static void BPL_BgExit(void);
static void BPL_BgGraphicSet( BPLIST_WORK * wk );
static void BPL_MsgManSet( BPLIST_WORK * wk );
static void BPL_MsgManExit( BPLIST_WORK * wk );
static void BPL_PokeDataMake( BPLIST_WORK * wk );

static u8 BPL_PokemonSelect( BPLIST_WORK * wk );
static u8 BPL_IrekaeSelect( BPLIST_WORK * wk );
static u8 BPL_StatusMainSelect( BPLIST_WORK * wk );
static u8 BPL_StInfoWazaSelect( BPLIST_WORK * wk );
static u8 BPL_StWazaSelect( BPLIST_WORK * wk );
static int BPL_TPCheck( BPLIST_WORK * wk, const GFL_UI_TP_HITTBL * tbl );
/*↑[GS_CONVERT_TAG]*/
static BOOL BPL_PageChange( BPLIST_WORK * wk, u8 next_page );
static void BPL_PageChgBgScreenChg( BPLIST_WORK * wk, u8 page );
static u8 BPL_IrekaeCheck( BPLIST_WORK * wk );
static u8 BPL_NextPokeGet( BPLIST_WORK * wk, s32 pos, s32 mv );
static void BPL_PokePlateSet( BPLIST_WORK * wk );
static void BPL_ExpGagePut( BPLIST_WORK * wk, u8 page );
//static void BPL_P2_ExpGagePut( BPLIST_WORK * wk, u16 cgx, u16 px, u16 py );
static void BPL_P3_ExpGagePut( BPLIST_WORK * wk, u16 cgx, u16 px, u16 py );
//static void BPL_ContestWazaHeartPut( BPLIST_WORK * wk, u8 page );
static u8 BPL_HidenCheck( BPLIST_WORK * wk );
static void BPL_HidenOff_Battle( BPLIST_WORK * wk );
//static void BPL_HidenOff_Contest( BPLIST_WORK * wk );
static u8 BPL_TamagoCheck( BPLIST_WORK * wk );

//static void BattleBag_SubItem( BATTLE_WORK * bw, u16 item, u16 page, u32 heap );

static BOOL FightPokeCheck( BPLIST_WORK * wk, u32 pos );
static BOOL ChangePokeCheck( BPLIST_WORK * wk, u32 pos );

static void CursorMoveSet( BPLIST_WORK * wk, u8 page );

static void InitListRow( BPLIST_WORK * wk );
static void ChangeListRow( BPLIST_WORK * wk, u32 pos1, u32 pos2 );
static BOOL CheckListRow( BPLIST_WORK * wk, u32 pos );
//static BOOL CheckRetNum( BPLIST_WORK * wk );
//static void SetRetNum( BPLIST_WORK * wk );
static void SetLog( BPLIST_WORK * wk, u8 pos1, u8 pos2 );
static int BPL_SeqPageChgDead( BPLIST_WORK * wk );
static int BPL_SeqPokeSelDead( BPLIST_WORK * wk );
static int BPL_SeqPokeChgDead( BPLIST_WORK * wk );
static int BPL_SeqDeadErrRet( BPLIST_WORK * wk );
static BOOL CheckDeadPoke( BPLIST_WORK * wk );
static void SetDeadChangeData( BPLIST_WORK * wk );
static BOOL CheckNextDeadSel( BPLIST_WORK * wk );
static void SetSelPosCancel( BPLIST_WORK * wk );



//============================================================================================
//  グローバル変数
//============================================================================================

// メインシーケンス
static const pBPlistFunc MainSeqFunc[] = {
  BPL_SeqInit,
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

  BPL_SeqWazaRcvSelect,

  BPL_SeqButtonWait,

  BPL_SeqStRcv,
  BPL_SeqPPAllRcv,

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
/*
// ポケモン選択画面移動テーブル
static const POINTSEL_WORK NormalMoveTable[] =
{
  {  0,  0, 16, 6,  6,2,0,1 },    // 0
  { 16,  1, 16, 6,  4,3,0,1 },    // 1
  {  0,  6, 16, 6,  0,4,2,3 },    // 2
  { 16,  7, 16, 6,  1,5,2,3 },    // 3
  {  0, 12, 16, 6,  2,1,4,5 },    // 4
  { 16, 13, 16, 6,  3,6,4,5 },    // 5
  { 25, 19,  7, 5,  5,0,6,6 },    // 6（戻る）
};
*/
/*
// ページ１のタッチパネル座標
static const GFL_UI_TP_HITTBL Page1_HitRect[] =
{
  {  0*8,  6*8-1,  0*8, 16*8-1 },
  {  1*8,  7*8-1, 16*8, 32*8-1 },
  {  6*8, 12*8-1,  0*8, 16*8-1 },
  {  7*8, 13*8-1, 16*8, 32*8-1 },
  { 12*8, 18*8-1,  0*8, 16*8-1 },
  { 13*8, 19*8-1, 16*8, 32*8-1 },
  { 19*8, 24*8-1, 27*8, 32*8-1 },
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};
*/
/*
static const GFL_UI_TP_HITTBL ChgPage_HitRect[] =
{
  {  1*8, 18*8-1,  1*8, 31*8-1 },   // 0 : 入れ替え
  { 19*8, 24*8-1,  0*8, 13*8-1 },   // 1 : 強さを見る
  { 19*8, 24*8-1, 13*8, 26*8-1 },   // 2 : 技を見る
  { 19*8, 24*8-1, 27*8, 32*8-1 },   // 3 : 戻る
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};
*/
/*
// ステータスメインページのタッチパネル座標
static const GFL_UI_TP_HITTBL StMainPage_HitRect[] =
{
  { 19*8, 24*8-1,  0*8,  5*8-1 },   // 0 : 前のポケモンへ
  { 19*8, 24*8-1,  5*8, 10*8-1 },   // 1 : 次のポケモンへ
  { 19*8, 24*8-1, 12*8, 25*8-1 },   // 2 : 技を見る
  { 19*8, 24*8-1, 27*8, 32*8-1 },   // 3 : 戻る
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};
*/
/*
// ステータス技選択ページのタッチパネル座標
static const GFL_UI_TP_HITTBL StWazaSelPage_HitRect[] =
{
  {  6*8, 12*8-1,  0*8, 16*8-1 },   // 0 : 技１
  {  6*8, 12*8-1, 16*8, 32*8-1 },   // 1 : 技２
  { 12*8, 18*8-1,  0*8, 16*8-1 },   // 2 : 技３
  { 12*8, 18*8-1, 16*8, 32*8-1 },   // 3 : 技４
  { 19*8, 24*8-1,  0*8,  5*8-1 },   // 4 : 前のポケモンへ
  { 19*8, 24*8-1,  5*8, 10*8-1 },   // 5 : 次のポケモンへ
  { 19*8, 24*8-1, 12*8, 25*8-1 },   // 6 : 強さを見る
  { 19*8, 24*8-1, 27*8, 32*8-1 },   // 7 : 戻る
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};
*/
/*
// ステータス技選択ページのタッチパネル座標
static const GFL_UI_TP_HITTBL StWazaInfoPage_HitRect[] =
{
  { 19*8, 21*8-1, 11*8, 16*8-1 },   // 0 : 技１
  { 19*8, 21*8-1, 16*8, 21*8-1 },   // 1 : 技２
  { 21*8, 23*8-1, 11*8, 16*8-1 },   // 2 : 技３
  { 21*8, 23*8-1, 16*8, 21*8-1 },   // 3 : 技４
  { 19*8, 24*8-1, 27*8, 32*8-1 },   // 4 : 戻る
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};
*/
/*
// 技忘れ選択ページのタッチパネル座標
static const GFL_UI_TP_HITTBL Page5_HitRect[] =
{
  {  6*8, 12*8-1,  0*8, 16*8-1 },   // 0 : 技１画面へ
  {  6*8, 12*8-1, 16*8, 32*8-1 },   // 1 : 技２画面へ
  { 12*8, 18*8-1,  0*8, 16*8-1 },   // 2 : 技３画面へ
  { 12*8, 18*8-1, 16*8, 32*8-1 },   // 3 : 技４画面へ
  { 18*8, 24*8-1,  8*8, 24*8-1 },   // 4 : 技５画面へ
  {  0*8,  5*8-1, 23*8, 32*8-1 },   // 5 : 戦闘<->コンテストの切り替え
  { 19*8, 24*8-1, 27*8, 32*8-1 },   // 6 : 戻る
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};
*/
/*
// 技忘れ決定ページのタッチパネル座標
static const GFL_UI_TP_HITTBL Page6_HitRect[] =
{
  { 19*8, 24*8-1,  0*8, 26*8-1 },   // 0 : 戦闘へ
  {  0*8,  5*8-1, 23*8, 32*8-1 },   // 1 : 戦闘<->コンテストの切り替え
  { 19*8, 24*8-1, 27*8, 32*8-1 },   // 2 : 戻る
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};
*/
/*
// 技回復ページのタッチパネル座標
static const GFL_UI_TP_HITTBL PPRcvPage_HitRect[] =
{
  {  6*8, 12*8-1,  0*8, 16*8-1 },   // 0 : 技１回復
  {  6*8, 12*8-1, 16*8, 32*8-1 },   // 1 : 技２回復
  { 12*8, 18*8-1,  0*8, 16*8-1 },   // 2 : 技３回復
  { 12*8, 18*8-1, 16*8, 32*8-1 },   // 3 : 技４回復
  { 19*8, 24*8-1, 27*8, 32*8-1 },   // 4 : 戻る
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};
*/


//--------------------------------------------------------------------------------------------
/**
 * 戦闘用ポケモンリストタスク追加
 *
 * @param dat   リストデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void BattlePokeList_TaskAdd( BPLIST_DATA * dat )
{
  BPLIST_WORK * wk;

  // メモリリークをチェックするために仮エリアを作成
//  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BATTLE_APP_TEST, 0x50000 );
//  dat->heap = HEAPID_BATTLE_APP_TEST;

  if( dat->sel_poke > 5 ){ dat->sel_poke = 0; }

  dat->sel_wp = 0;

  wk = GFL_HEAP_AllocClearMemory( dat->heap, sizeof(BPLIST_WORK) );

  GFL_TCB_AddTask( dat->tcb_sys, BattlePokeList_Main, wk, 100 );

  wk->dat = dat;
  wk->pfd = dat->pfd;
  wk->seq = SEQ_BPL_INIT;
  wk->init_poke = dat->sel_poke;

//  wk->multi_pos = BattleWorkClientTypeGet( dat->bw, dat->client_no );
//  wk->multi_pos = 0;
// CLIENT_TYPE_A : 前衛
// CLIENT_TYPE_C : 後衛

/*** テスト ***/
//  wk->dat->mode = BPL_MODE_NORMAL;      // 通常のポケモン選択
//  wk->dat->mode = BPL_MODE_ITEMUSE;     // アイテム使用
//  wk->dat->item = 38;

/*
  wk->dat->mode = BPL_MODE_WAZASET;     // 技忘れ
  wk->dat->sel_poke = 1;
  wk->dat->chg_waza = 20;
//  wk->page = BPLIST_PAGE_PP_RCV;    // PP回復技選択ページ
*/

//  wk->dat->mode = BPL_MODE_CHG_DEAD;   // 瀕死入れ替え時
//  wk->dat->rule = BTL_RULE_SINGLE;
//  wk->dat->rule = BTL_RULE_DOUBLE;
//	wk->dat->rule = BTL_RULE_TRIPLE;

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
 * メインタスク
 *
 * @param tcb
 * @param work
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BattlePokeList_Main( GFL_TCB* tcb, void * work )
{
  BPLIST_WORK * wk = (BPLIST_WORK *)work;

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

//  GFL_CLACT_SYS_Main( wk->crp );
  BattlePokeList_ButtonAnmMain( wk );
  BPLISTBMP_PrintMain( wk );

#if 0
  switch( wk->seq ){
  case SEQ_BPL_INIT:    // 初期化
    wk->seq = BPL_SeqInit( wk );
    break;

  case SEQ_BPL_SELECT:  // ポケモン選択画面コントロール
    wk->seq = BPL_SeqPokeSelect( wk );
    break;

  case SEQ_BPL_IREKAE:
    wk->seq = BPL_SeqPokeIrekae( wk );
    break;

  case SEQ_BPL_ST_MAIN: // ステータスメイン画面コントロール
    wk->seq = BPL_SeqStatusMain( wk );
    break;

  case SEQ_BPL_ST_WAZASEL:  // ステータス技選択画面コントロール
    wk->seq = BPL_SeqWazaSelect( wk );
    break;

  case SEQ_BPL_ST_SKILL:    // 技画面コントロール
    wk->seq = BPL_SeqStInfoWaza( wk );
    break;

  case SEQ_BPL_PAGE1_CHG:
    wk->seq = BPL_SeqPage1Chg( wk );
    break;

  case SEQ_BPL_PAGECHG_IREKAE:  // 入れ替えページへ
    wk->seq = BPL_SeqPageChgIrekae( wk );
    break;

  case SEQ_BPL_PAGECHG_STMAIN:  // ステータス詳細へ
    wk->seq = BPL_SeqPageChgStatus( wk );
    break;

  case SEQ_BPL_PAGECHG_WAZASEL: // 技選択へ
    wk->seq = BPL_SeqPageChgStWazaSel( wk );
    break;

  case SEQ_BPL_PAGECHG_WAZAINFO:
    wk->seq = BPL_SeqPageChgWazaInfo( wk );
    break;

  case SEQ_BPL_PAGECHG_WAZASET_S: // 技忘れ選択画面へ
    wk->seq = BPL_SeqPageChgWazaSetSel( wk );
    break;

  case SEQ_BPL_PAGECHG_WAZASET_I:
    wk->seq = BPL_SeqPageChgWazaSetEnter( wk );
    break;

  case SEQ_BPL_PAGECHG_PPRCV: // 技回復へ
    wk->seq = BPL_SeqPageChgPPRcv( wk );
    break;

  case SEQ_BPL_POKECHG:   // ポケモン切り替え
    wk->seq = BPL_SeqPokeChange( wk );
    break;

  case SEQ_BPL_CHG_ERR_SET: // 入れ替えエラーセット
    wk->seq = BPL_SeqChgErrSet( wk );
    break;

  case SEQ_BPL_IREKAE_ERR:  // 入れ替えエラー終了待ち
    wk->seq = BPL_SeqIrekaeErr( wk );
    break;

  case SEQ_BPL_MSG_WAIT:  // メッセージウェイト
    wk->seq = BPL_SeqMsgWait( wk );
    break;

  case SEQ_BPL_TRG_WAIT:  // メッセージ表示後のキー待ち
    wk->seq = BPL_SeqTrgWait( wk );
    break;

  case SEQ_BPL_WAZADEL_SEL: // 技忘れ選択
    wk->seq = BPL_SeqWazaDelSelect( wk );
    break;

  case SEQ_BPL_WAZADEL_MAIN:  // 技忘れ決定
    wk->seq = BPL_SeqWazaDelMain( wk );
    break;

  case SEQ_BPL_WAZARCV_SEL: // 技回復選択
    wk->seq = BPL_SeqWazaRcvSelect( wk );
    break;

  case SEQ_BPL_BUTTON_WAIT: // ボタンアニメ終了待ち
    wk->seq = BPL_SeqButtonWait( wk );
    break;

  case SEQ_BPL_STRCV:     // ステータス回復
    wk->seq = BPL_SeqStRcv( wk );
    break;

  case SEQ_BPL_PPALLRCV:    // PP全回復
    wk->seq = BPL_SeqPPAllRcv( wk );
    break;

  case SEQ_BPL_ENDSET:  // 終了フェードセット
    wk->seq = BPL_SeqEndSet( wk );
    break;

  case SEQ_BPL_END:   // 終了
    if( BPL_SeqEnd( tcb, wk ) == TRUE ){
      return;
    }
  }

  BPL_PokeIconAnime( wk );

  GFL_CLACT_SYS_Main( wk->crp );
/*↑[GS_CONVERT_TAG]*/
  BattlePokeList_ButtonAnmMain( wk );

#endif

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
static int BPL_SeqInit( BPLIST_WORK * wk )
{
  u8  ret;

  G2S_BlendNone();

  wk->tcbl = GFL_TCBL_Init( wk->dat->heap, wk->dat->heap, 1, 4 );

  if( wk->dat->mode == BPL_MODE_WAZASET ){
    wk->page = BPLIST_PAGE_WAZASET_BS;
    ret = SEQ_BPL_WAZADEL_SEL;
  }else{
    wk->page = BPLIST_PAGE_SELECT;
    ret = SEQ_BPL_SELECT;
  }

//  wk->cmv_wk = BAPP_CursorMoveWorkAlloc( wk->dat->heap );

  wk->cpwk = BAPPTOOL_CreateCursor( wk->dat->heap );

	InitListRow( wk );

  BPL_PokeDataMake( wk );

//  BPL_VramInit();
  BPL_BgInit( wk );
  BPL_BgGraphicSet( wk );
  BPL_MsgManSet( wk );

  BPL_PageChgBgScreenChg( wk, wk->page );
//  BPL_PokePlateSet( wk );
  BattlePokeList_ButtonPageScreenInit( wk, wk->page );
  BattlePokeList_ButtonPalSet( wk, wk->page );

  BattlePokeList_ObjInit( wk );
  BattlePokeList_PageObjSet( wk, wk->page );

  BattlePokeList_BmpInit( wk );
  BattlePokeList_BmpWrite( wk, wk->page );
  BPLISTBMP_SetStrScrn( wk );

//  wk->dat->cursor_flg = 1;
/*
  if( wk->dat->cursor_flg != 0 ){
    BAPP_CursorMvWkSetFlag( wk->cmv_wk, 1 );
  }
*/
  if( *wk->dat->cursor_flg == 1 ){
    wk->cursor_flg = TRUE;
  }else{
    wk->cursor_flg = FALSE;
  }
  BAPPTOOL_VanishCursor( wk->cpwk, wk->cursor_flg );

  // マルチのときの初期位置補正
  if( wk->page == BPLIST_PAGE_SELECT &&
    BattlePokeList_MultiPosCheck( wk, BPLISTMAIN_GetListRow(wk,0) ) == TRUE ){
    wk->dat->sel_poke = 1;
  }

//  BattlePokeList_CursorMoveSet( wk, wk->page );
  BPLISTUI_Init( wk, wk->page, 0 );

  BPL_ExpGagePut( wk, wk->page );

//  GFL_NET_ReloadIcon();
//  PaletteWorkSet_VramCopy( wk->pfd, FADE_SUB_OBJ, 14*16, FADE_PAL_ONE_SIZE );
//  {
/*
    u16 * defWk = PaletteWorkDefaultWorkGet( wk->pfd, FADE_SUB_OBJ );
    u16 * trsWk = PaletteWorkTransWorkGet( wk->pfd, FADE_SUB_OBJ );
//    u32 i;
    defWk[14*16+1] = 0x1f;
    defWk[14*16+2] = 0x3e;
    defWk[14*16+3] = 0x7c;
    defWk[14*16+15] = 0x7fff;
    trsWk[14*16+1] = 0x1f;
    trsWk[14*16+2] = 0x3e;
    trsWk[14*16+3] = 0x7c;
    trsWk[14*16+15] = 0x7fff;
*/
/*
    OS_Printf( "■■■ pal ■■■\n" );
    for( i=0; i<16; i++ ){
      OS_Printf( "0x%x, ", defWk[14*16+i] );
    }
    OS_Printf( "\n■■■ pal ■■■\n" );
    defWk = PaletteWorkTransWorkGet( wk->pfd, FADE_SUB_OBJ );
    for( i=0; i<16; i++ ){
      OS_Printf( "0x%x, ", defWk[14*16+i] );
    }
    OS_Printf( "\n■■■ pal ■■■\n" );
*/
//  }


  PaletteFadeReq(
    wk->pfd, PF_BIT_SUB_ALL, 0xffff, BATTLE_BAGLIST_FADE_SPEED, 16, 0, 0, wk->dat->tcb_sys );


  return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモン選択ページのコントロールシーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPokeSelect( BPLIST_WORK * wk )
{
  if( PaletteFadeCheck( wk->pfd ) != 0 ){ return SEQ_BPL_SELECT; }

  if( BPL_PokemonSelect( wk ) == TRUE ){
    if( wk->dat->sel_poke == BPL_SEL_EXIT ){
			// 瀕死いれかえ時
      if( wk->dat->mode == BPL_MODE_CHG_DEAD ){
				u8	pos1, pos2;
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
					return SEQ_BPL_POKECHG_DEAD;
				}
			// キャンセル無効以外
			}else if( wk->dat->mode != BPL_MODE_NO_CANCEL ){
        PMSND_PlaySE( SEQ_SE_DECIDE2 );
        BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
				SetSelPosCancel( wk );
        return SEQ_BPL_ENDSET;
			}
/*
      if( wk->dat->mode != BPL_MODE_CHG_DEAD ){
        PMSND_PlaySE( SEQ_SE_DECIDE2 );
        BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
				SetSelPosCancel( wk );
        return SEQ_BPL_ENDSET;
      }else{
				u8	pos1, pos2;
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
					return SEQ_BPL_POKECHG_DEAD;
				}
			}
*/
    }else{
      PMSND_PlaySE( SEQ_SE_DECIDE2 );
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
 * アイテム使用
 *
 * @param wk    戦闘ポケリストのワーク
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

  // １つの技のPP回復
  if( ITEM_GetParam( dat->item, ITEM_PRM_PP_RCV, dat->heap ) != 0 &&
      ITEM_GetParam( dat->item, ITEM_PRM_ALL_PP_RCV, dat->heap ) == 0 ){
    wk->ret_seq = SEQ_BPL_PAGECHG_PPRCV;
    return SEQ_BPL_BUTTON_WAIT;
  }

	dat->sel_pos[0] = BPLISTMAIN_GetListRow( wk, dat->sel_poke );

  return SEQ_BPL_ENDSET;

#if 0
  BPLIST_DATA * dat = wk->dat;

  // さしおさえで使用不可
  if( ( dat->sel_poke == 0 && dat->skill_item_use[0] != 0 ) ||
    ( dat->sel_poke == 1 && dat->skill_item_use[1] != 0 ) ){
    BattlePokeList_ItemUseSkillErrMsgSet( wk );
    BattlePokeList_TalkMsgSet( wk );
    wk->dat->sel_poke = BPL_SEL_EXIT;
    wk->ret_seq = SEQ_BPL_ENDSET;
    return SEQ_BPL_MSG_WAIT;
  }

  // １つの技のPP回復
  if( ITEM_GetParam( dat->item, ITEM_PRM_PP_RCV, dat->heap ) != 0 &&
      ITEM_GetParam( dat->item, ITEM_PRM_ALL_PP_RCV, dat->heap ) == 0 &&
      wk->poke[ BPLISTMAIN_GetListRow(wk,dat->sel_poke) ].egg == 0 ){

    wk->ret_seq = SEQ_BPL_PAGECHG_PPRCV;
    return SEQ_BPL_BUTTON_WAIT;
  }

  if( BattleWorkStatusRecover(dat->bw,dat->client_no,dat->list_row[dat->sel_poke],0,dat->item) == TRUE ){
    // 全技PP回復
    if( ITEM_GetParam( dat->item, ITEM_PRM_ALL_PP_RCV, dat->heap ) != 0 ){
      wk->ret_seq = SEQ_BPL_PAGECHG_PPRCV;
    }else{
      // 戦闘に出ているポケモンで、瀕死回復以外
      if( BattlePokeList_PokeSetCheck( wk, dat->sel_poke ) == 1 &&
        ITEM_GetParam( dat->item, ITEM_PRM_DEATH_RCV, dat->heap ) == 0 ){
//        BattleBag_SubItem( dat->bw, dat->item, dat->bag_page, dat->heap );
        wk->poke[ BPLISTMAIN_GetListRow(wk,dat->sel_poke) ].pp =
          BattleWorkPokemonParamGet( dat->bw, dat->client_no, dat->list_row[dat->sel_poke] );
        dat->hp_rcv = PP_Get( wk->poke[ BPLISTMAIN_GetListRow(wk,dat->sel_poke) ].pp, ID_PARA_hp, NULL );
        dat->hp_rcv -= wk->poke[ BPLISTMAIN_GetListRow(wk,dat->sel_poke) ].hp;
        wk->ret_seq = SEQ_BPL_ENDSET;
      }else{
        wk->ret_seq = SEQ_BPL_STRCV;
      }
    }
/*
    if( ITEM_GetParam( dat->item, ITEM_PRM_ALL_PP_RCV, dat->heap ) == 0 ){
      if( BattlePokeList_PokeSetCheck( wk, dat->sel_poke ) == 1 ){
        BattleBag_SubItem( dat->bw, dat->item, dat->bag_page, dat->heap );
        wk->poke[dat->sel_poke].pp =
          BattleWorkPokemonParamGet( dat->bw, dat->client_no, dat->list_row[dat->sel_poke] );
        dat->hp_rcv = PP_Get( wk->poke[dat->sel_poke].pp, ID_PARA_hp, NULL );
        dat->hp_rcv -= wk->poke[dat->sel_poke].hp;
        wk->ret_seq = SEQ_BPL_ENDSET;
      }else{
        wk->ret_seq = SEQ_BPL_STRCV;
      }
    }else{
      wk->ret_seq = SEQ_BPL_PAGECHG_PPRCV;
    }
*/
    wk->rcv_seq = 0;
    return SEQ_BPL_BUTTON_WAIT;
  }else{
    GFL_MSG_GetString( wk->mman, mes_b_plist_m06, wk->msg_buf );
    BattlePokeList_TalkMsgSet( wk );
    wk->dat->sel_poke = BPL_SEL_EXIT;
    wk->ret_seq = SEQ_BPL_ENDSET;
    return SEQ_BPL_MSG_WAIT;
  }
  return SEQ_BPL_MSG_WAIT;
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * 入れ替えページのコントロールシーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPokeIrekae( BPLIST_WORK * wk )
{
  u8  ret = BPL_IrekaeSelect( wk );

  switch( ret ){
  case 0:   // 入れ替える
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_POKE_CHG );
    if( BPL_IrekaeCheck( wk ) == TRUE ){
			// 通常の入れ替え
			if( wk->dat->mode != BPL_MODE_CHG_DEAD ){
				wk->dat->sel_pos[0] = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
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

  case 1:   // 強さを見る
    if( BPL_TamagoCheck( wk ) == TRUE ){ break; }
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_STATUS1 );
    wk->ret_seq = SEQ_BPL_PAGECHG_STMAIN;
    return SEQ_BPL_BUTTON_WAIT;

  case 2:   // 技を見る
    if( BPL_TamagoCheck( wk ) == TRUE ){ break; }
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZASEL1 );
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASEL;
    return SEQ_BPL_BUTTON_WAIT;

  case 3:   // 戻る
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->ret_seq = SEQ_BPL_PAGE1_CHG;
    return SEQ_BPL_BUTTON_WAIT;
  }

  return SEQ_BPL_IREKAE;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータスメインページのコントロールシーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqStatusMain( BPLIST_WORK * wk )
{
  u8  ret = BPL_StatusMainSelect( wk );

  switch( ret ){
  case 0:   // 前のポケモンへ
    {
      u8 pos = BPL_NextPokeGet( wk, wk->dat->sel_poke, -1 );
      if( pos == 0xff ){ break; }
      wk->dat->sel_poke = pos;
    }
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_UP );
    wk->ret_seq = SEQ_BPL_POKECHG;
    return SEQ_BPL_BUTTON_WAIT;

  case 1:   // 次のポケモンへ
    {
      u8 pos = BPL_NextPokeGet( wk, wk->dat->sel_poke, 1 );
      if( pos == 0xff ){ break; }
      wk->dat->sel_poke = pos;
    }
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_DOWN );
    wk->ret_seq = SEQ_BPL_POKECHG;
    return SEQ_BPL_BUTTON_WAIT;

  case 2:   // 技を見る
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZASEL2 );
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASEL;
    return SEQ_BPL_BUTTON_WAIT;

  case 3:   // 戻る
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->chg_page_cp = 1;    // 入れ替えページのカーソル位置を「強さを見る」へ
    wk->ret_seq = SEQ_BPL_PAGECHG_IREKAE;
    return SEQ_BPL_BUTTON_WAIT;
  }

  return SEQ_BPL_ST_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス技選択ページのコントロールシーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqWazaSelect( BPLIST_WORK * wk )
{
  u8  ret = BPL_StWazaSelect( wk );

  switch( ret ){
  case 0:   // 技１
  case 1:   // 技２
  case 2:   // 技３
  case 3:   // 技４
    if( wk->poke[ BPLISTMAIN_GetListRow(wk,wk->dat->sel_poke) ].waza[ret].id == 0 ){
      break;
    }
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZA1+ret );
    wk->dat->sel_wp = ret;
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZAINFO;
    return SEQ_BPL_BUTTON_WAIT;

  case 4:   // 前のポケモンへ
    {
      u8 pos = BPL_NextPokeGet( wk, wk->dat->sel_poke, -1 );
      if( pos == 0xff ){ break; }
      wk->dat->sel_poke = pos;
    }
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_UP );
    wk->ret_seq = SEQ_BPL_POKECHG;
    return SEQ_BPL_BUTTON_WAIT;

  case 5:   // 次のポケモンへ
    {
      u8 pos = BPL_NextPokeGet( wk, wk->dat->sel_poke, 1 );
      if( pos == 0xff ){ break; }
      wk->dat->sel_poke = pos;
    }
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_DOWN );
    wk->ret_seq = SEQ_BPL_POKECHG;
    return SEQ_BPL_BUTTON_WAIT;

  case 6:   // 強さを見る
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_STATUS2 );
    wk->ret_seq = SEQ_BPL_PAGECHG_STMAIN;
    return SEQ_BPL_BUTTON_WAIT;

  case 7:   // 戻る
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->chg_page_cp = 2;    // 入れ替えページのカーソル位置を「技を見る」へ
    wk->ret_seq = SEQ_BPL_PAGECHG_IREKAE;
    return SEQ_BPL_BUTTON_WAIT;
  }

  return SEQ_BPL_ST_WAZASEL;
}


//--------------------------------------------------------------------------------------------
/**
 * ステータス技詳細ページのコントロールシーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqStInfoWaza( BPLIST_WORK * wk )
{
  u8  ret = BPL_StInfoWazaSelect( wk );

  switch( ret ){
  case 0:   // 技１
  case 1:   // 技２
  case 2:   // 技３
  case 3:   // 技４
    if( wk->dat->sel_wp != ret &&
      wk->poke[ BPLISTMAIN_GetListRow(wk,wk->dat->sel_poke) ].waza[ret].id == 0 ){
      break;
    }
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    wk->dat->sel_wp = ret;
    return SEQ_BPL_PAGECHG_WAZAINFO;

  case 4:   // 戻る
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASEL;
    return SEQ_BPL_BUTTON_WAIT;
  }

  return SEQ_BPL_ST_SKILL;
}

//--------------------------------------------------------------------------------------------
/**
 * ページ５のコントロールシーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqWazaDelSelect( BPLIST_WORK * wk )
{
/*
  int ret = BPL_TPCheck( wk, Page5_HitRect );

  if( ret == GFL_UI_TP_HIT_NONE ){
    ret = BAPP_CursorMove( wk->cmv_wk );
    if( ret == BAPP_CMV_CANCEL ){
      ret = 6;
    }
  }else{
    BattlePokeList_CursorOff( wk );
  }

  switch( ret ){
  case 0:   // 技
  case 1:
  case 2:
  case 3:
  case 4:
    wk->dat->sel_wp = (u8)ret;
    wk->wws_page_cp = (u8)ret;
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZADEL1+ret );
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASET_I;
    return SEQ_BPL_BUTTON_WAIT;

  case 5:   // 戦闘<->コンテスト切り替え
    wk->wws_page_cp = (u8)ret;
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_CONTEST );
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASET_S;
    return SEQ_BPL_BUTTON_WAIT;

  case 6:   // キャンセル
    wk->dat->sel_wp = BPL_SEL_WP_CANCEL;
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->ret_seq = SEQ_BPL_ENDSET;
    return SEQ_BPL_BUTTON_WAIT;
  }
  return SEQ_BPL_WAZADEL_SEL;
*/
  u32 ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_WAZADEL_SEL1:    // 技１
  case BPLIST_UI_WAZADEL_SEL2:    // 技２
  case BPLIST_UI_WAZADEL_SEL3:    // 技３
  case BPLIST_UI_WAZADEL_SEL4:    // 技４
  case BPLIST_UI_WAZADEL_SEL5:    // 技５
    wk->dat->sel_wp = (u8)ret;
    wk->wws_page_cp = (u8)ret;
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZADEL1+ret );
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASET_I;
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_WAZADEL_RETURN:  // もどる
  case CURSORMOVE_CANCEL:         // キャンセル
    wk->dat->sel_wp = BPL_SEL_WP_CANCEL;
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->ret_seq = SEQ_BPL_ENDSET;
    return SEQ_BPL_BUTTON_WAIT;

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

  return SEQ_BPL_WAZADEL_SEL;
}

//--------------------------------------------------------------------------------------------
/**
 * ページ６のコントロールシーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqWazaDelMain( BPLIST_WORK * wk )
{
/*
  int ret = BPL_TPCheck( wk, Page6_HitRect );

  if( ret == GFL_UI_TP_HIT_NONE ){
    ret = BAPP_CursorMove( wk->cmv_wk );
    if( ret == BAPP_CMV_CANCEL ){
      ret = 2;
    }
  }else{
    BattlePokeList_CursorOff( wk );
  }

  switch( ret ){
  case 0:   // わすれる
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZADEL_B );
#if 0
    if( BPL_HidenCheck( wk ) == TRUE ){
      BPL_HidenMsgPut( wk );
      BPL_HidenOff_Battle( wk );
      wk->ret_seq = SEQ_BPL_WAZADEL_MAIN;
    }else{

      wk->ret_seq = SEQ_BPL_ENDSET;
    }
#else
    wk->ret_seq = SEQ_BPL_ENDSET;
#endif
    return SEQ_BPL_BUTTON_WAIT;

  case 1:   // 戦闘<->コンテスト切り替え
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_CONTEST );
    wk->wwm_page_cp = (u8)ret;
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASET_I;
    return SEQ_BPL_BUTTON_WAIT;

  case 2:   // キャンセル
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->wwm_page_cp = 0;
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASET_S;
    return SEQ_BPL_BUTTON_WAIT;
  }
  return SEQ_BPL_WAZADEL_MAIN;
*/
  u32 ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_DELINFO_ENTER:   // わすれる
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZADEL_B );
/*
    if( BPL_HidenCheck( wk ) == TRUE ){
      BPL_HidenMsgPut( wk );
      BPL_HidenOff_Battle( wk );
      wk->ret_seq = SEQ_BPL_WAZADEL_MAIN;
    }else{
      wk->ret_seq = SEQ_BPL_ENDSET;
    }
*/
    wk->ret_seq = SEQ_BPL_ENDSET;
    return SEQ_BPL_BUTTON_WAIT;

  case BPLIST_UI_DELINFO_RETURN:  // もどる
  case CURSORMOVE_CANCEL:         // キャンセル
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->wwm_page_cp = 0;
    wk->ret_seq = SEQ_BPL_PAGECHG_WAZASET_S;
    return SEQ_BPL_BUTTON_WAIT;

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

  return SEQ_BPL_WAZADEL_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * 技回復選択ページのコントロールシーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqWazaRcvSelect( BPLIST_WORK * wk )
{
/*
  BPLIST_DATA * dat;
  int ret;

  dat = wk->dat;
  ret = BPL_TPCheck( wk, PPRcvPage_HitRect );
  if( ret == GFL_UI_TP_HIT_NONE ){
    ret = BAPP_CursorMove( wk->cmv_wk );
    if( ret == BAPP_CMV_CANCEL ){
      ret = 4;
    }
  }else{
    BattlePokeList_CursorOff( wk );
  }

  switch( ret ){
  case 0:   // 技
  case 1:
  case 2:
  case 3:
#if 0
    if( wk->poke[dat->sel_poke].waza[ret].id == 0 ){ break; }
    wk->dat->sel_wp = (u8)ret;
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZARCV1+ret );
//    if( BattleWorkStatusRecover(dat->bw,dat->client_no,dat->list_row[dat->sel_poke],ret,dat->item) == TRUE ){
    if( 0 ){
      wk->rcv_seq = 0;
      wk->ret_seq = SEQ_BPL_STRCV;
      return SEQ_BPL_BUTTON_WAIT;
    }else{
      GFL_MSG_GetString( wk->mman, mes_b_plist_m06, wk->msg_buf );
      BattlePokeList_TalkMsgSet( wk );
      wk->dat->sel_poke = BPL_SEL_EXIT;
      wk->ret_seq = SEQ_BPL_ENDSET;
      return SEQ_BPL_MSG_WAIT;
    }
    break;
#endif
    if( wk->poke[dat->sel_poke].waza[ret].id == 0 ){ break; }
    wk->dat->sel_wp = (u8)ret;
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZARCV1+ret );
    return SEQ_BPL_ENDSET;

  case 4:   // キャンセル
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->ret_seq = SEQ_BPL_PAGE1_CHG;
    return SEQ_BPL_BUTTON_WAIT;
  }
  return SEQ_BPL_WAZARCV_SEL;
*/
  BPLIST_DATA * dat;
  u32 ret;

  dat = wk->dat;
  ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_PPRCV_WAZA1:     // 技１
  case BPLIST_UI_PPRCV_WAZA2:     // 技２
  case BPLIST_UI_PPRCV_WAZA3:     // 技３
  case BPLIST_UI_PPRCV_WAZA4:     // 技４
/*
    if( wk->poke[dat->sel_poke].waza[ret].id == 0 ){ break; }
    wk->dat->sel_wp = (u8)ret;
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZARCV1+ret );
//    if( BattleWorkStatusRecover(dat->bw,dat->client_no,dat->list_row[dat->sel_poke],ret,dat->item) == TRUE ){
    if( 0 ){
      wk->rcv_seq = 0;
      wk->ret_seq = SEQ_BPL_STRCV;
      return SEQ_BPL_BUTTON_WAIT;
    }else{
      GFL_MSG_GetString( wk->mman, mes_b_plist_m06, wk->msg_buf );
      BattlePokeList_TalkMsgSet( wk );
      wk->dat->sel_poke = BPL_SEL_EXIT;
      wk->ret_seq = SEQ_BPL_ENDSET;
      return SEQ_BPL_MSG_WAIT;
    }
    break;
*/
		{
			u8	pos = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
	    if( wk->poke[pos].waza[ret].id == 0 ){ break; }
	    wk->dat->sel_wp = (u8)ret;
			wk->dat->sel_pos[0] = pos;
	    PMSND_PlaySE( SEQ_SE_DECIDE2 );
	    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_WAZARCV1+ret );
	    return SEQ_BPL_ENDSET;
		}

  case BPLIST_UI_PPRCV_RETURN:    // もどる
  case CURSORMOVE_CANCEL:         // キャンセル
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
    BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
    wk->ret_seq = SEQ_BPL_PAGE1_CHG;
    return SEQ_BPL_BUTTON_WAIT;

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

  return SEQ_BPL_WAZARCV_SEL;
}


//--------------------------------------------------------------------------------------------
/**
 * ページ切り替え：ポケモン選択ページへ
 *
 * @param wk    戦闘ポケリストのワーク
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
 * ページ切り替え：ポケモン入れ替えページへ
 *
 * @param wk    戦闘ポケリストのワーク
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
 * ページ切り替え：ステータスメインページへ
 *
 * @param wk    戦闘ポケリストのワーク
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
 * ページ切り替え：ステータス技選択ページへ
 *
 * @param wk    戦闘ポケリストのワーク
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
 * ページ切り替え：技詳細ページへ
 *
 * @param wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPageChgWazaInfo( BPLIST_WORK * wk )
{
  if( BPL_PageChange( wk, BPLIST_PAGE_SKILL ) == FALSE ){
    return SEQ_BPL_PAGECHG_WAZAINFO;
  }
  return SEQ_BPL_ST_SKILL;
}

//--------------------------------------------------------------------------------------------
/**
 * ページ切り替え：技忘れページへ
 *
 * @param wk    戦闘ポケリストのワーク
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
 * ページ切り替え：技忘れ詳細ページへ
 *
 * @param wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPageChgWazaSetEnter( BPLIST_WORK * wk )
{
/*
  if( PRINTSYS_QUE_IsFinished( wk->que ) == FALSE ){
    return SEQ_BPL_PAGECHG_WAZASET_I;
  }
  BattlePokelist_WazaTypeSet( wk );
  BPL_PageChange( wk, BPLIST_PAGE_WAZASET_BI );
*/
	if( BPL_PageChange( wk, BPLIST_PAGE_WAZASET_BI ) == FALSE ){
		return SEQ_BPL_PAGECHG_WAZASET_I;
	}
//  BattlePokelist_WazaTypeSet( wk );
  return SEQ_BPL_WAZADEL_MAIN;
}

//--------------------------------------------------------------------------------------------
/**
 * ページ切り替え：技回復ページへ
 *
 * @param wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPageChgPPRcv( BPLIST_WORK * wk )
{
  if( BPL_PageChange( wk, BPLIST_PAGE_PP_RCV ) == FALSE ){
    return SEQ_BPL_PAGECHG_PPRCV;
  }

  if( ITEM_GetParam( wk->dat->item, ITEM_PRM_ALL_PP_RCV, wk->dat->heap ) != 0 ){
    return SEQ_BPL_PPALLRCV;
  }
  return SEQ_BPL_WAZARCV_SEL;
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンきりかえシーケンス
 *
 * @param wk    ワーク
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
 * 入れ替えエラーセット
 *
 * @param wk    ワーク
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
 * 入れ替えエラーメッセージ終了待ちシーケンス
 *
 * @param wk    ワーク
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
 * メッセージ表示シーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqMsgWait( BPLIST_WORK * wk )
{
  if( PRINTSYS_PrintStreamGetState(wk->stream) == PRINTSTREAM_STATE_DONE ){
    PRINTSYS_PrintStreamDelete( wk->stream );
    return SEQ_BPL_TRG_WAIT;
  }
  return SEQ_BPL_MSG_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * メッセージ表示後のキー待ちシーケンス
 *
 * @param wk    ワーク
 *
 * @return  移行するシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqTrgWait( BPLIST_WORK * wk )
{
  if( ( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL) ) || GFL_UI_TP_GetTrg() == TRUE ){
    return wk->ret_seq;
  }
  return SEQ_BPL_TRG_WAIT;
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
static int BPL_SeqButtonWait( BPLIST_WORK * wk )
{
  if( wk->btn_flg == 0 ){
    return wk->ret_seq;
  }
  return SEQ_BPL_BUTTON_WAIT;
}

//--------------------------------------------------------------------------------------------
/**
 * 回復シーケンス
 *
 * @param wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqStRcv( BPLIST_WORK * wk )
{
  BPLIST_DATA * dat;
	u32	pos;
	
	dat = wk->dat;
	pos = BPLISTMAIN_GetListRow( wk, dat->sel_poke );

  switch( wk->rcv_seq ){
  case 0:
    wk->poke[pos].pp =
//      BattleWorkPokemonParamGet( dat->bw, dat->client_no, dat->list_row[dat->sel_poke] );
        NULL;
    BattlePokeList_ItemUseMsgSet( wk );
    if( wk->page == BPLIST_PAGE_PP_RCV ){
  //    wk->rcv_pp[0] = (u16)PP_Get(wk->poke[dat->sel_poke].pp,ID_PARA_pp1+dat->sel_wp,NULL);
      wk->rcv_pp[0] = PP_Get( wk->poke[pos].pp,ID_PARA_pp1+dat->sel_wp,NULL);
      wk->rcv_seq = 2;
    }else{
      wk->poke[pos].st = APP_COMMON_GetStatusIconAnime( wk->poke[pos].pp );
      if( wk->poke[pos].st == APP_COMMON_ST_ICON_NONE ){
//        GFL_CLACT_UNIT_SetDrawEnableCap( wk->cap[BPL_CA_STATUS1+dat->sel_poke], 0 );
        BattlePokeList_P1_LvPut( wk, dat->sel_poke );
      }
      wk->rcv_hp = PP_Get( wk->poke[pos].pp, ID_PARA_hp, NULL );
      wk->rcv_seq = 4;
    }
//    PMSND_PlaySE( SEQ_SE_DP_KAIFUKU );
    break;

  case 1:   // HP回復
    if( wk->poke[pos].hp != wk->rcv_hp ){
      wk->poke[pos].hp++;
      BattlePokeList_P1_HPPut( wk, dat->sel_poke );
      break;
    }
    wk->rcv_seq = 3;
    break;

  case 2:   // PP回復
    if( wk->poke[pos].waza[dat->sel_wp].pp != wk->rcv_pp[0] ){
      wk->poke[pos].waza[dat->sel_wp].pp++;
      BattlePokeList_PPRcv( wk, WIN_P7_SKILL1+dat->sel_wp, dat->sel_wp );
      break;
    }
    wk->rcv_seq = 3;
    break;

  case 3:
//    BattleBag_SubItem( dat->bw, dat->item, dat->bag_page, dat->heap );
    BattlePokeList_TalkMsgSet( wk );
    wk->ret_seq = SEQ_BPL_ENDSET;
		wk->dat->sel_pos[0] = pos;
    return SEQ_BPL_MSG_WAIT;

  case 4:   // 瀕死回復のために１度だけ呼ぶ
    if( wk->poke[pos].hp != wk->rcv_hp ){
      wk->poke[pos].hp++;
      BattlePokeList_P1_HPPut( wk, dat->sel_poke );
      BPL_HPRcvButtonPut( wk );
    }
    wk->rcv_seq = 1;
    break;

  }
  return SEQ_BPL_STRCV;
}

//--------------------------------------------------------------------------------------------
/**
 * PP全回復シーケンス
 *
 * @param wk    戦闘ポケリストのワーク
 *
 * @return  次のシーケンス
 */
//--------------------------------------------------------------------------------------------
static int BPL_SeqPPAllRcv( BPLIST_WORK * wk )
{
  BPLIST_DATA * dat;
	u32	pos;
  u32 i, j;
	
	dat = wk->dat;
	pos = BPLISTMAIN_GetListRow( wk, dat->sel_poke );

  switch( wk->rcv_seq ){
  case 0:
    wk->poke[pos].pp =
//      BattleWorkPokemonParamGet( dat->bw, dat->client_no, dat->list_row[dat->sel_poke] );
      NULL;
    for( i=0; i<4; i++ ){
      if( wk->poke[pos].waza[i].id == 0 ){ continue; }
      wk->rcv_pp[i] = (u16)PP_Get(wk->poke[pos].pp,ID_PARA_pp1+i,NULL);
    }
    BattlePokeList_ItemUseMsgSet( wk );
//    PMSND_PlaySE( SEQ_SE_DP_KAIFUKU );
    wk->rcv_seq = 1;
    break;

  case 1:   // PP回復
    j = 0;
    for( i=0; i<4; i++ ){
      if( wk->poke[pos].waza[i].id == 0 ){
        j++;
        continue;
      }
      if( wk->poke[pos].waza[i].pp != wk->rcv_pp[i] ){
        wk->poke[pos].waza[i].pp++;
        BattlePokeList_PPRcv( wk, WIN_P7_SKILL1+i, i );
      }else{
        j++;
      }
    }
    if( j == 4 ){ wk->rcv_seq = 2; }
    break;

  case 2:
//    BattleBag_SubItem( dat->bw, dat->item, dat->bag_page, dat->heap );
    BattlePokeList_TalkMsgSet( wk );
    wk->ret_seq = SEQ_BPL_ENDSET;
    return SEQ_BPL_MSG_WAIT;
  }

  return SEQ_BPL_PPALLRCV;
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
static int BPL_SeqEndSet( BPLIST_WORK * wk )
{
	// マルチモード時の選択位置補正
	if( BattlePokeList_MultiCheck( wk ) == TRUE ){
		u32	i;
		for( i=0; i<BPL_SELNUM_MAX; i++ ){
			if( wk->dat->sel_pos[i] != BPL_SELPOS_NONE ){
				wk->dat->sel_pos[i] /= 2;
			}
		}
		wk->dat->sel_poke /= 2;
	}
/*
	OS_Printf( "\n■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n" );
	OS_Printf( "sel_poke = %d\n", wk->dat->sel_poke );
	{
		u32	i;
		for( i=0; i<BPL_SELNUM_MAX; i++ ){
			OS_Printf( "sel_pos = %d\n", wk->dat->sel_pos[i] );
		}
	}
	OS_Printf( "■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■\n" );
*/

  PaletteFadeReq(
    wk->pfd, PF_BIT_SUB_ALL, 0xffff, BATTLE_BAGLIST_FADE_SPEED, 0, 16, 0, wk->dat->tcb_sys );
  return SEQ_BPL_ENDWAIT;
}

static int BPL_SeqEndWait( BPLIST_WORK * wk )
{
  if( PaletteFadeCheck( wk->pfd ) == 0 ){
    return SEQ_BPL_END;
  }
  return SEQ_BPL_ENDWAIT;
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
static BOOL BPL_SeqEnd( GFL_TCB * tcb, BPLIST_WORK * wk )
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
//  wk->dat->cursor_flg = BAPP_CursorMvWkGetFlag( wk->cmv_wk );

  BPL_MsgManExit( wk );
  BattlePokeList_ObjFree( wk );
  BattlePokeList_BmpFreeAll( wk );
  BPL_BgExit();

  GFL_TCBL_Exit( wk->tcbl );

//  BAPP_CursorMoveWorkFree( wk->cmv_wk );
  BPLISTUI_Exit( wk );

  BAPPTOOL_FreeCursor( wk->cpwk );

//  FontProc_UnloadFont( FONT_TOUCH );

  GFL_TCB_DeleteTask( tcb );
  GFL_HEAP_FreeMemory( wk );

//  GFL_HEAP_DeleteHeap( HEAPID_BATTLE_APP_TEST );

  return TRUE;
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
static void BPL_VramInit(void)
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
 * BG解放
 *
 * @param ini   BGLデータ
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

/*
#define PCOL_N_BLUE   ( PRINTSYS_LSB_MAKE( 7, 8, 0 ) )    // フォントカラー：青
#define PCOL_N_RED    ( PRINTSYS_LSB_MAKE( 3, 4, 0 ) )    // フォントカラー：赤
#define PCOL_B_BLUE   ( PRINTSYS_LSB_MAKE( 10, 11, 0 ) )    // フォントカラー：ボタン用青
#define PCOL_B_RED    ( PRINTSYS_LSB_MAKE( 12, 13, 0 ) )    // フォントカラー：ボタン用赤
*/
#define SFNT_COL_BLUE ( 7 )
#define SFNT_COL_RED  ( 3 )
#define BFNT_COL_BLUE ( 10 )
#define BFNT_COL_RED  ( 12 )

//--------------------------------------------------------------------------------------------
/**
 * グラフィックデータセット
 *
 * @param wk    ポケモンリスト画面のワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_BgGraphicSet( BPLIST_WORK * wk )
{
  ARCHANDLE* hdl;

  hdl  = GFL_ARC_OpenDataHandle( ARCID_B_PLIST_GRA,  wk->dat->heap );

  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    hdl, NARC_b_plist_gra_b_plist_lz_NCGR, GFL_BG_FRAME3_S, 0, 0, TRUE, wk->dat->heap );

  {
    NNSG2dScreenData * dat;
    void * buf;
/*
    buf = GFL_ARC_LoadDataAllocByHandle(
            hdl, NARC_b_plist_gra_b_plist_anm_NSCR, wk->dat->heap );
    NNS_G2dGetUnpackedScreenData( buf, &dat );
*/
    buf = GFL_ARCHDL_UTIL_LoadScreen(
            hdl, NARC_b_plist_gra_b_plist_anm_lz_NSCR, TRUE, &dat, wk->dat->heap );
    BattlePokeList_ButtonScreenMake( wk, (u16 *)dat->rawData );
    GFL_HEAP_FreeMemory( buf );

/*
    buf = GFL_ARC_LoadDataAllocByHandle(
            hdl, NARC_b_plist_gra_b_plist_anm2_NSCR, wk->dat->heap );
    NNS_G2dGetUnpackedScreenData( buf, &dat );
*/
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
//    GFL_STD_MemCopy( &pltt[BPL_PAL_B_GREEN*16], wk->wb_pal, 0x20*2 );
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

/*
  // システムフォントパレットセット
  PaletteWorkSet_Arc(
    wk->pfd, ARC_FONT, NARC_font_system_ncrl,
    wk->dat->heap, FADE_SUB_BG, 0x20, BPL_PAL_SYS_FONT*16 );

  // 会話フォントパレットセット
  PaletteWorkSet_Arc(
    wk->pfd, ARC_FONT, NARC_font_talk_ncrl,
    wk->dat->heap, FADE_SUB_BG, 0x20, BPL_PAL_TALK_FONT*16 );

  // 会話ウィンドウセット
  {
    WINTYPE wt = BattleWorkConfigWinTypeGet( wk->dat->bw );
    GFL_ARC_UTIL_TransVramBgCharacter(
      ARC_WINFRAME, TalkWinCgxArcGet(wt),
      GFL_BG_FRAME0_S, TALK_WIN_CGX_POS, 0, 0, wk->dat->heap );
    PaletteWorkSet_Arc(
      wk->pfd, ARC_WINFRAME, TalkWinPalArcGet(wt),
      wk->dat->heap, FADE_SUB_BG, 0x20, BPL_PAL_TALK_WIN*16 );
  }

  { // ボタンフォント用パレットの青と赤をシステムフォント用のパレットにコピー
    u16 * pal = PaletteWorkDefaultWorkGet( wk->pfd, FADE_SUB_BG );
    u16 * buf = GFL_HEAP_AllocMemory( wk->dat->heap, 0x20 );

    GFL_STD_MemCopy( &pal[BPL_PAL_SYS_FONT*16], buf, 0x20 );
    GFL_STD_MemCopy( &pal[BPL_PAL_HPGAGE*16+BFNT_COL_BLUE], &buf[SFNT_COL_BLUE], 4 );
    GFL_STD_MemCopy( &pal[BPL_PAL_HPGAGE*16+BFNT_COL_RED], &buf[SFNT_COL_RED], 4 );
    PaletteWorkSet( wk->pfd, buf, FADE_SUB_BG, BPL_PAL_SYS_FONT*16, 0x20 );
    GFL_HEAP_FreeMemory( buf );
  }
*/
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
static void BPL_MsgManSet( BPLIST_WORK * wk )
{
  wk->mman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_b_plist_dat, wk->dat->heap );
//  wk->nfnt = NUMFONT_Create( 15, 14, FBMP_COL_NULL, wk->dat->heap );
  wk->nfnt = GFL_FONT_Create( ARCID_FONT, NARC_font_small_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->dat->heap );
  wk->wset = WORDSET_Create( wk->dat->heap );
//  wk->que  = PRINTSYS_QUE_Create( wk->dat->heap );
  wk->que  = PRINTSYS_QUE_CreateEx( 2048, wk->dat->heap );
  wk->msg_buf = GFL_STR_CreateBuffer( TMP_MSG_BUF_SIZ, wk->dat->heap );

//  PRINTSYS_QUE_ForceCommMode( wk->que, TRUE );      // テスト
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
static void BPL_MsgManExit( BPLIST_WORK * wk )
{
  GFL_MSG_Delete( wk->mman );
//  NUMFONT_Delete( wk->nfnt );
  GFL_FONT_Delete( wk->nfnt );
  WORDSET_Delete( wk->wset );
  PRINTSYS_QUE_Delete( wk->que );
  GFL_STR_DeleteBuffer( wk->msg_buf );
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモンデータ作成
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void PokeDataMakeCore( BPLIST_WORK * wk, POKEMON_PARAM * pp, BPL_POKEDATA * dat )
{
	u32	j;

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

  dat->style     = (u8)PP_Get( dat->pp, ID_PARA_style, NULL );
  dat->beautiful = (u8)PP_Get( dat->pp, ID_PARA_beautiful, NULL );
  dat->cute      = (u8)PP_Get( dat->pp, ID_PARA_cute, NULL );
  dat->clever    = (u8)PP_Get( dat->pp, ID_PARA_clever, NULL );
  dat->strong    = (u8)PP_Get( dat->pp, ID_PARA_strong, NULL );

  dat->cb = (u16)PP_Get( dat->pp, ID_PARA_cb_id, NULL );
  dat->form = (u8)PP_Get( dat->pp, ID_PARA_form_no, NULL );

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
    waza->hit  = WAZADATA_GetParam( waza->id, WAZAPARAM_HITPER );
    waza->pow  = WAZADATA_GetParam( waza->id, WAZAPARAM_POWER );
  }
}

static void BPL_PokeDataMake( BPLIST_WORK * wk )
{
	u32	i;

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


//	wk->poke[0].hp = 0;
//  wk->poke[1].hp = 0;
//  wk->poke[2].hp = 0;
//  wk->poke[3].hp = 0;
}

//--------------------------------------------------------------------------------------------
/**
 * ポケモン選択
 *
 * @param wk    ワーク
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
      return TRUE;
    }
    break;

  case BPLIST_UI_LIST_RETURN:
  case CURSORMOVE_CANCEL:         // キャンセル
    wk->dat->sel_poke = BPL_SEL_EXIT;
    return TRUE;

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

  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 入れ替えページ選択
 *
 * @param wk    ワーク
 *
 * @retval  "0xff != 選択された"
 * @retval  "0xff = 未選択"
 */
//--------------------------------------------------------------------------------------------
static u8 BPL_IrekaeSelect( BPLIST_WORK * wk )
{
/*
  int ret = BPL_TPCheck( wk, ChgPage_HitRect );

  if( ret == GFL_UI_TP_HIT_NONE ){
    ret = BAPP_CursorMove( wk->cmv_wk );
    if( ret == BAPP_CMV_CANCEL ){
      ret = 3;
    }else if( ret == BAPP_CMV_NONE ){
      return 0xff;
    }
  }else{
    BattlePokeList_CursorOff( wk );
  }
  return (u8)ret;
*/
  u32 ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_CHG_ENTER:       // いれかえる
  case BPLIST_UI_CHG_STATUS:      // つよさをみる
  case BPLIST_UI_CHG_WAZA:        // わざをみる
  case BPLIST_UI_CHG_RETURN:      // もどる
    break;

  case CURSORMOVE_CANCEL:         // キャンセル
    ret = BPLIST_UI_CHG_RETURN;
    break;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
    PMSND_PlaySE( SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
  case CURSORMOVE_NONE:           // 動作なし
    ret = 0xff;
    break;
  }

  return (u8)ret;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス選択
 *
 * @param wk    ワーク
 *
 * @retval  "0xff != 選択された"
 * @retval  "0xff = 未選択"
 */
//--------------------------------------------------------------------------------------------
static u8 BPL_StatusMainSelect( BPLIST_WORK * wk )
{
/*
  int ret = BPL_TPCheck( wk, StMainPage_HitRect );

  if( ret == GFL_UI_TP_HIT_NONE ){
    ret = BAPP_CursorMove( wk->cmv_wk );
    if( ret == BAPP_CMV_CANCEL ){
      ret = 3;
    }else if( ret == BAPP_CMV_NONE ){
      return 0xff;
    }
  }else{
    BattlePokeList_CursorOff( wk );
  }
  return (u8)ret;
*/
  u32 ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_STATUS_UP:       // 前のポケモンへ
  case BPLIST_UI_STATUS_DOWN:     // 次のポケモンへ
  case BPLIST_UI_STATUS_WAZA:     // わざをみる
  case BPLIST_UI_STATUS_RETURN:   // もどる
    break;

  case CURSORMOVE_CANCEL:         // キャンセル
    ret = BPLIST_UI_STATUS_RETURN;
    break;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
    PMSND_PlaySE( SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
  case CURSORMOVE_NONE:           // 動作なし
    ret = 0xff;
    break;
  }

  return (u8)ret;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス技選択
 *
 * @param wk    ワーク
 *
 * @retval  "0xff != 選択された"
 * @retval  "0xff = 未選択"
 */
//--------------------------------------------------------------------------------------------
static u8 BPL_StWazaSelect( BPLIST_WORK * wk )
{
/*
  int ret = BPL_TPCheck( wk, StWazaSelPage_HitRect );

  if( ret == GFL_UI_TP_HIT_NONE ){
    ret = BAPP_CursorMove( wk->cmv_wk );
    if( ret == BAPP_CMV_CANCEL ){
      ret = 7;
    }else if( ret == BAPP_CMV_NONE ){
      return 0xff;
    }
  }else{
    BattlePokeList_CursorOff( wk );
  }
  return (u8)ret;
*/
  u32 ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_WAZA_SEL1:     // 技１
  case BPLIST_UI_WAZA_SEL2:     // 技２
  case BPLIST_UI_WAZA_SEL3:     // 技３
  case BPLIST_UI_WAZA_SEL4:     // 技４
  case BPLIST_UI_WAZA_UP:       // 前のポケモンへ
  case BPLIST_UI_WAZA_DOWN:     // 次のポケモンへ
  case BPLIST_UI_WAZA_STATUS:   // つよさをみる
  case BPLIST_UI_WAZA_RETURN:   // もどる
    break;

  case CURSORMOVE_CANCEL:       // キャンセル
    ret = BPLIST_UI_WAZA_RETURN;
    break;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
    PMSND_PlaySE( SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
  case CURSORMOVE_NONE:           // 動作なし
    ret = 0xff;
    break;
  }

  return (u8)ret;
}

//--------------------------------------------------------------------------------------------
/**
 * ステータス技詳細選択
 *
 * @param wk    ワーク
 *
 * @retval  "0xff != 選択された"
 * @retval  "0xff = 未選択"
 */
//--------------------------------------------------------------------------------------------
static u8 BPL_StInfoWazaSelect( BPLIST_WORK * wk )
{
/*
  int ret = BPL_TPCheck( wk, StWazaInfoPage_HitRect );

  if( ret == GFL_UI_TP_HIT_NONE ){
    ret = BAPP_CursorMove( wk->cmv_wk );
    if( ret == BAPP_CMV_CANCEL ){
      ret = 4;
    }else if( ret == BAPP_CMV_NONE ){
      return 0xff;
    }
  }else{
    BattlePokeList_CursorOff( wk );
  }
  return (u8)ret;
*/
  u32 ret = CURSORMOVE_MainCont( wk->cmwk );

  switch( ret ){
  case BPLIST_UI_WAZAINFO_SEL1:     // 技１
  case BPLIST_UI_WAZAINFO_SEL2:     // 技２
  case BPLIST_UI_WAZAINFO_SEL3:     // 技３
  case BPLIST_UI_WAZAINFO_SEL4:     // 技４
  case BPLIST_UI_WAZAINFO_RETURN:   // もどる
    break;

  case CURSORMOVE_CANCEL:         // キャンセル
    ret = BPLIST_UI_WAZAINFO_RETURN;
    break;

  case CURSORMOVE_CURSOR_MOVE:    // 移動
    PMSND_PlaySE( SEQ_SE_SELECT1 );
    break;

  case CURSORMOVE_NO_MOVE_UP:     // 十字キー上が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_DOWN:   // 十字キー下が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_LEFT:   // 十字キー左が押されたが、移動なし
  case CURSORMOVE_NO_MOVE_RIGHT:  // 十字キー右が押されたが、移動なし
  case CURSORMOVE_CURSOR_ON:      // カーソル表示
  case CURSORMOVE_NONE:           // 動作なし
    ret = 0xff;
    break;
  }

  return (u8)ret;
}


//--------------------------------------------------------------------------------------------
/**
 * タッチパネルのボタン押し判定
 *
 * @param wk    ワーク
 * @param tbl   TPデータテーブル
 *
 * @retval  "GFL_UI_TP_HIT_NONE = 押されていない"
 * @retval  "GFL_UI_TP_HIT_NONE != 押したボタン番号"
 */
//--------------------------------------------------------------------------------------------
static int BPL_TPCheck( BPLIST_WORK * wk, const GFL_UI_TP_HITTBL * tbl )
{
  int ret = GFL_UI_TP_HitTrg( tbl );
/*
  if( ret != GFL_UI_TP_HIT_NONE ){
    u16 pat = 0xfffe;
    if( GFL_BG_CheckDot( wk->bgl, GFL_BG_FRAME3_S, sys.tp_x, sys.tp_y, &pat ) == FALSE ){
      return GFL_UI_TP_HIT_NONE;
    }
  }
*/
  return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * 指定位置にポケモンが存在するか
 *
 * @param wk    ワーク
 * @param pos   位置
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
/*
  if( pos == 0 ||
    ( (BattlePokeList_DoubleCheck(wk)||BattlePokeList_MultiCheck(wk)) && pos == 1 ) ){
    return 1;
  }
*/
  if( FightPokeCheck( wk, pos ) == TRUE ){
    return 1;
  }
  return 2;
}

//--------------------------------------------------------------------------------------------
/**
 * 次のポケモンを取得
 *
 * @param wk    ワーク
 * @param pos   現在位置
 * @param mv    移動方向
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
 * プレート書き換え
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
/*
static void BPL_PokePlateSet( BPLIST_WORK * wk )
{
  s16 i, j;

  if( wk->dat->mode == BPL_MODE_WAZASET ){ return; }

  for( i=0; i<TEMOTI_POKEMAX; i++ ){
    j = BattlePokeList_PokeSetCheck( wk, i );
    if( j == 1 ){
      GFL_BG_WriteScreen(
        wk->bgl, GFL_BG_FRAME2_S, wk->btn_plate1[0],
        PlatePos[i][0], PlatePos[i][1], PLATE_BG_SX, PLATE_BG_SY );
    }else if( j == 2 ){
      GFL_BG_WriteScreen(
        wk->bgl, GFL_BG_FRAME2_S, wk->btn_plate2[0],
        PlatePos[i][0], PlatePos[i][1], PLATE_BG_SX, PLATE_BG_SY );
    }
  }
  GFL_BG_LoadScreenV_Req( wk->bgl, GFL_BG_FRAME2_S );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 経験値ゲージ書き換え
 *
 * @param wk    ワーク
 * @param page  ページ番号
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

    BPL_P3_ExpGagePut( wk, cgx, P3_EXPGAGE_PX+i, P3_EXPGAGE_PY );

    if( dot < 8 ){
      dot = 0;
    }else{
      dot -= 8;
    }
  }

  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_S );
}

/*
static void BPL_P2_ExpGagePut( BPLIST_WORK * wk, u16 cgx, u16 px, u16 py )
{
  GFL_BG_FillScreen( wk->bgl, GFL_BG_FRAME3_S, cgx, px, py, 1, 1, GFL_BG_SCRWRT_PALNL );
  GFL_BG_FillScreen( wk->bgl, GFL_BG_FRAME3_S, cgx+32, px, py+1, 1, 1, GFL_BG_SCRWRT_PALNL );
}
*/

static void BPL_P3_ExpGagePut( BPLIST_WORK * wk, u16 cgx, u16 px, u16 py )
{
  GFL_BG_FillScreen( GFL_BG_FRAME3_S, cgx, px, py, 1, 1, GFL_BG_SCRWRT_PALNL );
}

#define HEART1_CHR_NUM  ( 0x140 )
#define HEART2_CHR_NUM  ( 0x125 )
#define HEART_PX    ( 2 )
#define HEART_PY    ( 14 )
#define HEART_MAX   ( 6 )

//--------------------------------------------------------------------------------------------
/**
 * コンテスト技のハート書き換え
 *
 * @param wk    ワーク
 * @param chr   使用キャラ
 * @param num   何個目のハートか
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_ContestWazaHeartPutMain( BPLIST_WORK * wk, u16 chr, u8 num )
{
  GFL_BG_FillScreen(
    GFL_BG_FRAME3_S, chr, HEART_PX+num*2, HEART_PY, 1, 1, GFL_BG_SCRWRT_PALNL );
  GFL_BG_FillScreen(
    GFL_BG_FRAME3_S, chr+1, HEART_PX+num*2+1, HEART_PY, 1, 1, GFL_BG_SCRWRT_PALNL );
  GFL_BG_FillScreen(
    GFL_BG_FRAME3_S, chr+32, HEART_PX+num*2, HEART_PY+1, 1, 1, GFL_BG_SCRWRT_PALNL );
  GFL_BG_FillScreen(
    GFL_BG_FRAME3_S, chr+33, HEART_PX+num*2+1, HEART_PY+1, 1, 1, GFL_BG_SCRWRT_PALNL );
}

//--------------------------------------------------------------------------------------------
/**
 * コンテスト技のハート初期化
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_ContestWazaHeartInit( BPLIST_WORK * wk )
{
  u16 i;

  for( i=0; i<HEART_MAX; i++ ){
    BPL_ContestWazaHeartPutMain( wk, HEART2_CHR_NUM, i );
  }
}




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
static BOOL BPL_PageChange( BPLIST_WORK * wk, u8 next_page )
{
/*
  if( PRINTSYS_QUE_IsFinished( wk->que ) == FALSE ){
    return FALSE;
  }

  BPL_PageChgBgScreenChg( wk, next_page );

  GFL_BG_ClearScreenCodeVReq( GFL_BG_FRAME0_S, 0 );
  GFL_BG_ClearScreenCodeVReq( GFL_BG_FRAME1_S, 0 );

  BattlePokeList_PageObjSet( wk, next_page );
  BattlePokeList_BmpFree( wk );
  BattlePokeList_BmpAdd( wk, next_page );
  BattlePokeList_BmpWrite( wk, next_page );
  BPL_ExpGagePut( wk, next_page );

//  BPL_ContestWazaHeartPut( wk, next_page );

//  BattlePokeList_CursorMoveSet( wk, next_page );
//  BPLISTUI_ChangePage( wk, next_page, 0 );
  CursorMoveSet( wk, next_page );

  BattlePokeList_ButtonPageScreenInit( wk, next_page );
  BattlePokeList_ButtonPalSet( wk, next_page );

  wk->page = next_page;

  return TRUE;
*/
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

static const u32 ScreenArc[][2] =
{ // ボタンBGスクリーン, 背景BGスクリーン
  { NARC_b_plist_gra_poke_sel_t_lz_NSCR, NARC_b_plist_gra_poke_sel_u_lz_NSCR },       // ポケモン選択ページ
  { NARC_b_plist_gra_poke_chg_t_lz_NSCR, NARC_b_plist_gra_poke_chg_u_lz_NSCR },       // ポケモン入れ替えページ
  { NARC_b_plist_gra_st_main_t_lz_NSCR, NARC_b_plist_gra_st_main_u_lz_NSCR },       // ステータスメインページ
  { NARC_b_plist_gra_st_waza_sel_t_lz_NSCR, NARC_b_plist_gra_st_waza_sel_u_lz_NSCR },   // ステータス詳細ページ
  { NARC_b_plist_gra_st_waza_main_t_lz_NSCR, NARC_b_plist_gra_st_waza_main_u_lz_NSCR },   // ステータス技ページ
  { NARC_b_plist_gra_item_waza_t_lz_NSCR, NARC_b_plist_gra_item_waza_u_lz_NSCR },     // PP回復技選択ページ
  { NARC_b_plist_gra_waza_delb_sel_t_lz_NSCR, NARC_b_plist_gra_waza_delb_sel_u_lz_NSCR }, // ステータス技忘れ１ページ（戦闘技選択）
  { NARC_b_plist_gra_waza_delb_main_t_lz_NSCR, NARC_b_plist_gra_waza_delb_main_u_lz_NSCR }, // ステータス技忘れ２ページ（戦闘技詳細）
//  { NARC_b_plist_gra_waza_delc_sel_t_NSCR, NARC_b_plist_gra_waza_delc_sel_u_NSCR }, // ステータス技忘れ３ページ（コンテスト技詳細）
//  { NARC_b_plist_gra_waza_delc_main_t_NSCR, NARC_b_plist_gra_waza_delc_main_u_NSCR }, // ステータス技忘れ４ページ（コンテスト技選択）
  { NARC_b_plist_gra_poke_sel_t_lz_NSCR, NARC_b_plist_gra_poke_sel_u_lz_NSCR },       // 瀕死入れ替え選択ページ
};

//--------------------------------------------------------------------------------------------
/**
 * BGスクリーン書き換え
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
/*
    buf = GFL_ARC_LoadDataAlloc( ARCID_B_PLIST_GRA, ScreenArc[page][i], wk->dat->heap );
    NNS_G2dGetUnpackedScreenData( buf, &dat );
*/
    buf = GFL_ARC_UTIL_LoadScreen(
      ARCID_B_PLIST_GRA, ScreenArc[page][i], TRUE, &dat, wk->dat->heap );
    GFL_BG_WriteScreen( GFL_BG_FRAME2_S+i, (u16 *)dat->rawData, 0, 0, 32, 24 );
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S+i );
    GFL_HEAP_FreeMemory( buf );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * 入れ替えチェック
 *
 * @param wk    ワーク
 *
 * @retval  "TRUE = 入れ替え可"
 * @retval  "FALSE = 入れ替え不可"
 */
//--------------------------------------------------------------------------------------------
static u8 BPL_IrekaeCheck( BPLIST_WORK * wk )
{
  BPL_POKEDATA * dat;
  STRBUF * str;
	u8	pos;

	pos = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
  dat = &wk->poke[pos];

  // 他人
  if( BattlePokeList_MultiPosCheck( wk, pos ) == TRUE ){
    str = GFL_MSG_CreateString( wk->mman, mes_b_plist_m05 );
/*
    {
      int pcn;

      pcn = BattleWorkPartnerClientNoGet( wk->dat->bw, wk->dat->client_no );
//      WORDSET_RegisterPlayerName( wk->wset, 0, BattleWorkMyStatusGet(wk->dat->bw,pcn) );
      WORDSET_RegisterTrainerNameBattle(
        wk->wset, 0, BattleWorkTrainerDataGet(wk->dat->bw,pcn) );
    }
*/
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
//  if( BattlePokeList_PokeSetCheck( wk, wk->dat->sel_poke ) == 1 ){
/*
  if( wk->dat->list_row[wk->dat->sel_poke] == wk->dat->fight_poke1 ||
    wk->dat->list_row[wk->dat->sel_poke] == wk->dat->fight_poke2 ){
*/
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
//  if( wk->dat->double_sel != 6 && wk->dat->list_row[wk->dat->sel_poke] == wk->dat->double_sel ){
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
    dat = &wk->poke[ BPLISTMAIN_GetListRow(wk,wk->init_poke) ];
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
 * タマゴチェック
 *
 * @param wk    ワーク
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
 * ダブルバトルチェック
 *
 * @param wk    ワーク
 *
 * @retval  "TRUE = ダブルバトル"
 * @retval  "FALSE = ダブルバトル以外"
 */
//--------------------------------------------------------------------------------------------
u8 BattlePokeList_DoubleCheck( BPLIST_WORK * wk )
{
/*
  u32 type = BattleWorkFightTypeGet(wk->dat->bw);

  if( type != FIGHT_TYPE_2vs2_YASEI && type != FIGHT_TYPE_AI_MULTI &&
    (type & (FIGHT_TYPE_2vs2|FIGHT_TYPE_TAG)) ){
    return TRUE;
  }
  return FALSE;
*/
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * マルチバトルチェック
 *
 * @param wk    ワーク
 *
 * @retval  "TRUE = マルチバトル"
 * @retval  "FALSE = マルチバトル以外"
 */
//--------------------------------------------------------------------------------------------
BOOL BattlePokeList_MultiCheck( BPLIST_WORK * wk )
{
	return wk->dat->multiMode;
/*
  u32 type = BattleWorkFightTypeGet(wk->dat->bw);

  if( type != FIGHT_TYPE_2vs2_YASEI && type != FIGHT_TYPE_AI_MULTI && (type&FIGHT_TYPE_MULTI) ){
    return TRUE;
  }
  return FALSE;
*/
}

//--------------------------------------------------------------------------------------------
/**
 * マルチバトルでパートナーのポケモンかチェック
 *
 * @param wk    ワーク
 * @param pos   位置
 *
 * @retval  "TRUE = はい"
 * @retval  "FALSE = いいえ"
 */
//--------------------------------------------------------------------------------------------
u8 BattlePokeList_MultiPosCheck( BPLIST_WORK * wk, u8 pos )
{
  if( BattlePokeList_MultiCheck( wk ) == TRUE ){
/*
		if( wk->dat->multiPos == 0 ){
			if( pos >= 3 ){ return TRUE; }
		}else{
			if( pos < 3 ){ return TRUE; }
		}
*/
		if( pos >= 3 ){ return TRUE; }
	}
	return FALSE;

/*
  if( BattlePokeList_MultiCheck( wk ) == TRUE ){
    if( wk->dat->multiPos == CLIENT_TYPE_A ){
      if( ( pos & 1 ) != 0 ){ return TRUE; }
    }else{
      if( ( pos & 1 ) == 0 ){ return TRUE; }
    }
  }
  return FALSE;
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 秘伝技チェック
 *
 * @param wk    ワーク
 *
 * @retval  "TRUE = 秘伝技"
 * @retval  "FALSE = 秘伝技以外"
 */
//--------------------------------------------------------------------------------------------
static u8 BPL_HidenCheck( BPLIST_WORK * wk )
{
/*
  u16 waza;

  if( wk->dat->sel_wp == 4 ){
    waza = wk->dat->chg_waza;
  }else{
    waza = wk->poke[wk->dat->sel_poke].waza[wk->dat->sel_wp].id;
  }
  return HidenWazaCheck( waza );
*/
  return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 技パラメータを非表示に（戦闘）
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BPL_HidenOff_Battle( BPLIST_WORK * wk )
{
/*
  // 分類アイコン
  GFL_CLACT_UNIT_SetDrawEnableCap( wk->cap[BPL_CA_BUNRUI], 0 );

  // 分類
  GF_BGL_BmpWinOffVReq( &wk->add_win[WIN_P6_BRNAME] );

  // 命中
  GF_BGL_BmpWinOffVReq( &wk->add_win[WIN_P6_HITNUM] );

  // 威力
  GF_BGL_BmpWinOffVReq( &wk->add_win[WIN_P6_POWNUM] );
*/
}

//--------------------------------------------------------------------------------------------
/**
 * 技パラメータを非表示に（コンテスト）
 *
 * @param wk    ワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
/*
static void BPL_HidenOff_Contest( BPLIST_WORK * wk )
{
  // ハート
  BPL_ContestWazaHeartInit( wk );
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_S );
}
*/

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
  if( item != ITEM_AOIBIIDORO && item != ITEM_AKAIBIIDORO && item != ITEM_KIIROBIIDORO ){
    MyItem_SubItem( BattleWorkMyItemGet(bw), item, 1, heap );
  }
  MyItem_BattleBagLastItemSet( BattleWorkBagCursorGet(bw), item, page );
}
*/

//--------------------------------------------------------------------------------------------
/**
 * 指定位置のポケモンが戦闘に出ているか
 *
 * @param wk    戦闘リストワーク
 * @param pos   位置
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

  switch( wk->dat->rule ){
  case BTL_RULE_SINGLE:   // シングル
    if( pos == 0 ){
      return TRUE;
    }
    break;

  case BTL_RULE_DOUBLE:   // ダブル
    if( pos == 0 || pos == 1 ){
      return TRUE;
    }
    break;

  case BTL_RULE_TRIPLE:   // トリプル
  case BTL_RULE_ROTATION: // ローテーション
    if( pos == 0 || pos == 1 || pos == 2 ){
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL FightPosCheck( BPLIST_WORK * wk, u32 pos )
{
  switch( wk->dat->rule ){
  case BTL_RULE_SINGLE:   // シングル
    if( pos == 0 ){
      return TRUE;
    }
    break;

  case BTL_RULE_DOUBLE:   // ダブル
    if( pos == 0 || pos == 1 ){
      return TRUE;
    }
    break;

  case BTL_RULE_TRIPLE:   // トリプル
  case BTL_RULE_ROTATION: // ローテーション
    if( pos == 0 || pos == 1 || pos == 2 ){
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL ChangePokeCheck( BPLIST_WORK * wk, u32 pos )
{
	pos = BPLISTMAIN_GetListRow( wk, pos );

  switch( wk->dat->rule ){
  case BTL_RULE_SINGLE:   // シングル
    break;

  case BTL_RULE_DOUBLE:   // ダブル
  case BTL_RULE_TRIPLE:   // トリプル
  case BTL_RULE_ROTATION: // ローテーション
		return CheckListRow( wk, pos );
  }
  return FALSE;
}



static void CursorMoveSet( BPLIST_WORK * wk, u8 page )
{
  u32 pos;

  switch( page ){
  case BPLIST_PAGE_SELECT:    // ポケモン選択ページ
	case BPLIST_PAGE_DEAD:			// 瀕死入れ替え選択ページ
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





//============================================================================================
//============================================================================================
//============================================================================================
//============================================================================================
//============================================================================================
//============================================================================================
static const u8 InitListRowMulti[2][6] =
{
	{ 0, 3, 1, 4, 2, 5 },
	{ 3, 0, 4, 1, 5, 2 },
};
static void InitListRow( BPLIST_WORK * wk )
{
	u32	i;

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

static void ChangeListRow( BPLIST_WORK * wk, u32 pos1, u32 pos2 )
{
	u8	tmp;
	
	tmp = wk->listRow[pos1];
	wk->listRow[pos1] = wk->listRow[pos2];
	wk->listRow[pos2] = tmp;
}

static BOOL CheckListRow( BPLIST_WORK * wk, u32 pos )
{
	u32	i;

	for( i=0; i<BPL_SELNUM_MAX; i++ ){
		if( wk->dat->sel_pos[i] == pos ){
			return TRUE;
		}
	}
	return FALSE;
}

u8 BPLISTMAIN_GetListRow( BPLIST_WORK * wk, u32 pos )
{
	return wk->listRow[pos];
}

/*
static BOOL CheckRetNum( BPLIST_WORK * wk )
{
	u16	i, j;

	j = 0;
	for( i=0; i<BPL_SELNUM_MAX; i++ ){
		if( wk->dat->sel_pos[i] == BPL_SELPOS_SET ){
			j++;
			if( j == 2 ){
				return FALSE;
			}
		}
	}
	return TRUE;
}
*/
/*
static void SetRetNum( BPLIST_WORK * wk )
{
	u32	i;

	for( i=0; i<BPL_SELNUM_MAX; i++ ){
		if( wk->dat->sel_pos[i] == BPL_SELPOS_SET ){
			wk->dat->sel_pos[i] = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
			break;
		}
	}

	ChangeListRow( wk, i, wk->dat->sel_poke );
}
*/

static void SetRetNum( BPLIST_WORK * wk, u8 pos, u8 row )
{
	wk->dat->sel_pos[pos] = BPLISTMAIN_GetListRow( wk, row );
}

static void SetLog( BPLIST_WORK * wk, u8 pos1, u8 pos2 )
{
	u8	i;

	// ログ作成
	for( i=0; i<BPL_SELNUM_MAX-1; i++ ){
		if( wk->chg_log[i].pos1 == BPL_SELPOS_NONE ){
			wk->chg_log[i].pos1 = pos1;
			wk->chg_log[i].pos2 = pos2;
			break;
		}
	}
}

BOOL BPLISTMAIN_GetNewLog( BPLIST_WORK * wk, u8 * pos1, u8 * pos2, BOOL del )
{
	s32	i;

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



static int BPL_SeqPageChgDead( BPLIST_WORK * wk )
{
  if( BPL_PageChange( wk, BPLIST_PAGE_DEAD ) == FALSE ){
		wk->chg_poke_sel = wk->dat->sel_poke;
    return SEQ_BPL_PAGECHG_DEAD;
  }
  return SEQ_BPL_POKESEL_DEAD;
}

static BOOL CheckDeadChange( BPLIST_WORK * wk )
{
  BPL_POKEDATA * dat;
  STRBUF * str;
	u8	pos;

	pos = BPLISTMAIN_GetListRow( wk, wk->dat->sel_poke );
  dat = &wk->poke[pos];

  // 他人
  if( BattlePokeList_MultiPosCheck( wk, pos ) == TRUE ){
    str = GFL_MSG_CreateString( wk->mman, mes_b_plist_m05 );
/*
    {
      int pcn;

      pcn = BattleWorkPartnerClientNoGet( wk->dat->bw, wk->dat->client_no );
//      WORDSET_RegisterPlayerName( wk->wset, 0, BattleWorkMyStatusGet(wk->dat->bw,pcn) );
      WORDSET_RegisterTrainerNameBattle(
        wk->wset, 0, BattleWorkTrainerDataGet(wk->dat->bw,pcn) );
    }
*/
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

  // 瀕死
  if( dat->hp != 0 ){
    str = GFL_MSG_CreateString( wk->mman, mes_b_plist_m01 );
    WORDSET_RegisterPokeNickName( wk->wset, 0, dat->pp );
    WORDSET_ExpandStr( wk->wset, wk->msg_buf, str );
    GFL_STR_DeleteBuffer( str );
    return FALSE;
  }

/*
  // タマゴ
  if( BPL_TamagoCheck( wk ) == TRUE ){
    GFL_MSG_GetString( wk->mman, mes_b_plist_m04, wk->msg_buf );
    return FALSE;
  }

  // 選択されている
//  if( wk->dat->double_sel != 6 && wk->dat->list_row[wk->dat->sel_poke] == wk->dat->double_sel ){
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
    dat = &wk->poke[ BPLISTMAIN_GetListRow(wk,wk->init_poke) ];
    str = GFL_MSG_CreateString( wk->mman, mes_b_plist_m03 );
    WORDSET_RegisterPokeNickName( wk->wset, 0, dat->pp );
    WORDSET_ExpandStr( wk->wset, wk->msg_buf, str );
    GFL_STR_DeleteBuffer( str );
    return FALSE;
  }
*/

  return TRUE;
}

static int BPL_SeqPokeSelDead( BPLIST_WORK * wk )
{
  if( PaletteFadeCheck( wk->pfd ) != 0 ){ return SEQ_BPL_POKESEL_DEAD; }

  if( BPL_PokemonSelect( wk ) == TRUE ){
    if( wk->dat->sel_poke == BPL_SEL_EXIT ){
      BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_RET );
			wk->dat->sel_poke = wk->chg_poke_sel;
      wk->ret_seq = SEQ_BPL_PAGECHG_IREKAE;
			return SEQ_BPL_BUTTON_WAIT;
		}else	if( wk->dat->sel_poke == wk->chg_poke_sel ){
			return SEQ_BPL_PAGECHG_IREKAE;
    }else{
/*
      PMSND_PlaySE( SEQ_SE_DECIDE2 );
      BattlePokeList_ButtonAnmInit( wk, BPL_BUTTON_POKE1+wk->dat->sel_poke );
      if( wk->dat->mode == BPL_MODE_ITEMUSE ){
        return BPL_PokeItemUse( wk );
      }else{
        wk->ret_seq = SEQ_BPL_PAGECHG_IREKAE;
      }
      return SEQ_BPL_BUTTON_WAIT;
*/
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

static int BPL_SeqDeadErrRet( BPLIST_WORK * wk )
{
  BmpWinFrame_Clear( wk->win[WIN_TALK].win, WINDOW_TRANS_ON );
	BPLISTBMP_DeadSelInfoMesPut( wk );
	BPLISTBMP_SetCommentScen( wk );
  return SEQ_BPL_POKESEL_DEAD;
}



static void GetDeadScrnPos( u8 * x, u8 * y, u8 num )
{
	*x = ( num & 1 ) * BPL_COMM_BSX_PLATE;
	*y = num / 2 * BPL_COMM_BSY_PLATE + ( num & 1 );
}

static void MakeDeadChgScrn( u16 * scrn, u8 frm, u8 num )
{
	u16 * buf;
	u8	x, y;
	u8	i;

	buf = GFL_BG_GetScreenBufferAdrs( frm );
	GetDeadScrnPos( &x, &y, num );

	for( i=0; i<BPL_COMM_BSY_PLATE; i++ ){
		GFL_STD_MemCopy16( &buf[(y+i)*32+x], &scrn[i*BPL_COMM_BSX_PLATE], BPL_COMM_BSX_PLATE*2 );
	}
}

static void MoveDeadChgObj( BPLIST_WORK * wk, u8 num, u8 mode )
{
	u8	pos;
	s8	mv;

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
			u16 * buf = GFL_HEAP_AllocMemory( wk->dat->heap, BPL_COMM_BSX_PLATE*BPL_COMM_BSY_PLATE*2 );
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
			u8	x, y;
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
			s8	x1, y1, x2, y2;
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
			s8	x1, y1, x2, y2;
			BGWINFRM_PosGet( wk->chg_wfrm, 0, &x1, &y1 );
			BGWINFRM_PosGet( wk->chg_wfrm, 2, &x2, &y2 );
			if( BGWINFRM_MoveCheck( wk->chg_wfrm, 0 ) == FALSE ){
				wk->btn_seq++;
			}
		}
		break;

	case 5:
		BGWINFRM_Exit( wk->chg_wfrm );
		wk->btn_seq = 0;
	  return FALSE;
	}

	return TRUE;
}

static int BPL_SeqPokeChgDead( BPLIST_WORK * wk )
{
	if( MoveDeadChangeMain( wk ) == FALSE ){
		if( CheckNextDeadSel( wk ) == FALSE ){
			return SEQ_BPL_ENDSET;
		}
		BPLISTANM_RetButtonPut( wk );
		BPLISTBMP_PokeSelInfoMesPut( wk );
		BPLISTBMP_SetCommentScen( wk );
	  return SEQ_BPL_SELECT;
	}
	return SEQ_BPL_POKECHG_DEAD;
}

// 瀕死入れ替え動作が必要かどうか
static BOOL CheckDeadPoke( BPLIST_WORK * wk )
{
	u8	cnt;
	u8	max;
	u8	i;

	// マルチバトル時
	if( BattlePokeList_MultiCheck( wk ) == TRUE ){
		return FALSE;
	}

	if( wk->dat->rule == BTL_RULE_SINGLE ){
		return FALSE;
	}else if( wk->dat->rule == BTL_RULE_DOUBLE ){
		max = 2;
	}else{
		max = 3;
	}

	// 空きが複数ある
	cnt = 0;
	for( i=0; i<max; i++ ){
		u32	row = BPLISTMAIN_GetListRow( wk, i );
		if( wk->poke[row].hp == 0 ){
			cnt++;
			if( cnt == 2 ){
				return TRUE;
			}
		}
	}

/*
	// 入れ替え可能なポケモンが複数いるか
	cnt = 0;
	for( i=max; i<TEMOTI_POKEMAX; i++ ){
		if( wk->poke[i].mons != 0 && wk->poke[i].egg != 0 ){
			cnt++;
			if( cnt == 2 ){
				return TRUE;
			}
		}
	}
*/

	return FALSE;
}

static BOOL CheckNextDeadSel( BPLIST_WORK * wk )
{
	u8	max;
	u8	i;

	// マルチバトル時
	if( BattlePokeList_MultiCheck( wk ) == TRUE ){
		return FALSE;
	}

	if( wk->dat->rule == BTL_RULE_SINGLE ){
		return FALSE;
	}else if( wk->dat->rule == BTL_RULE_DOUBLE ){
		max = 2;
	}else{
		max = 3;
	}

	// 入れ替え可能なポケモンがいるか
	for( i=max; i<TEMOTI_POKEMAX; i++ ){
		u32	row = BPLISTMAIN_GetListRow( wk, i );
		if( wk->poke[row].mons != 0 && wk->poke[row].egg == 0 && wk->poke[row].hp != 0 ){
			return TRUE;
		}
	}

	return FALSE;
}

// 上から検索して瀕死の箇所に強制的にセット
static void SetDeadChangeData( BPLIST_WORK * wk )
{
	u32	i;

	for( i=0; i<BPL_SELNUM_MAX; i++ ){
		u32	row = BPLISTMAIN_GetListRow( wk, i );
		if( wk->poke[row].hp == 0 ){
			wk->dat->sel_pos[i] = wk->dat->sel_poke;
			break;
		}
	}
}

// キャンセル設定
static void SetSelPosCancel( BPLIST_WORK * wk )
{
	u32	i;

	for( i=0; i<BPL_SELNUM_MAX; i++ ){
		wk->dat->sel_pos[i] = BPL_SELPOS_NONE;
	}
}
