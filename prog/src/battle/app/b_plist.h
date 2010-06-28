//============================================================================================
/**
 * @file		b_plist.h
 * @brief		戦闘用ポケモンリスト画面　外部公開ヘッダファイル
 * @author	Hiroyuki Nakamura
 * @date		05.02.01
 */
//============================================================================================
#pragma	once

#include "pm_define.h"
#include "print/printsys.h"
#include "poke_tool/pokeparty.h"
#include "battle/battle.h"
#include "system/palanm.h"


//============================================================================================
//  定数定義
//============================================================================================
#define BPL_SEL_EXIT      ( 6 ) // キャンセル
#define BPL_SEL_WP_CANCEL ( 4 ) // 技忘れキャンセル（新しく覚える技位置）

#define BPL_CHANGE_SEL_NONE   ( BPL_SEL_EXIT )  // ダブル/トリプルですでに選択されているポケモンがいないとき

#define BPL_SELNUM_MAX    ( 3 )     // 入れ替え時に選択できる最大数

#define BPL_SELPOS_NONE   ( 0xff )  // 入れ替えなし

// リスト処理定義
enum {
  BPL_MODE_NORMAL = 0,  // 通常のポケモン選択
  BPL_MODE_NO_CANCEL,   // キャンセル無効のポケモン選択
  BPL_MODE_CHG_DEAD,    // 瀕死入れ替え選択
  BPL_MODE_ITEMUSE,     // アイテム使用
  BPL_MODE_WAZASET,     // 技忘れ
  BPL_MODE_WAZAINFO,    // 技確認
};

// リストデータ
typedef struct {
  // [in]
	GAMEDATA * gamedata;		// ゲームデータ
  POKEPARTY * pp;         // ポケモンデータ
  POKEPARTY * multi_pp;   // マルチの相手側ポケモンデータ

  GFL_FONT * font;	// フォント

  HEAPID  heap;     // ヒープID

  BtlRule rule;     // シングル／ダブル／トリプル　※マルチバトル時のルールはダブル！

  BOOL  multiMode;    // マルチバトルかどうか TRUE = マルチ, FALSE = それ以外
  u8  multiPos;       // マルチの立ち位置
  u8  change_sel[2];  // ダブル/トリプルですでに選択されているポケモン（いない場合はBPL_CHANGE_SEL_NONE）
  u8  mode;           // リストモード

  u8  sel_poke;       // 選択されたポケモン or 戻る　（初期位置用）
	u8	sel_pos_index;	//「いれかえ」が押されたときに、sel_pos[ sel_pos_index ] に選択されたポケモン位置が入る

	u8	fight_poke_max;	// 戦闘に出ている数

  u16 item;           // アイテム
  u16 chg_waza;       // いれかえ禁止技・新しく覚える技

  GFL_TCBSYS* tcb_sys;
  PALETTE_FADE_PTR pfd; // パレットフェードデータ

  BOOL time_out_flg;    // 強制終了フラグ TRUE = 強制終了　※バトル側がリアルタイムで変更します

	BOOL comm_err_flg;		// 通信エラー時の強制終了フラグ

  u8  skill_item_use[ TEMOTI_POKEMAX ]; // 技で使用不可

  BOOL  seFlag;     // SEを再生するかどうか TRUE = 再生

  // [in & out]
  u8 * cursor_flg;              // カーソル表示フラグ

  // [out]
  u8  sel_pos[BPL_SELNUM_MAX];  // 選択されたポケモン（キャンセル時は、全体がBPL_SELPOS_NONEになる）
  u8  sel_wp;     // 選択された技位置
  u8  end_flg;    // 終了フラグ

}BPLIST_DATA;


//============================================================================================
//  プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		戦闘用ポケモンリストタスク追加
 *
 * @param		dat   リストデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BattlePokeList_TaskAdd( BPLIST_DATA * dat );
