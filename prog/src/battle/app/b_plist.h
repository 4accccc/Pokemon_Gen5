//============================================================================================
/**
 * @file	b_plist.h
 * @brief	戦闘用ポケモンリスト画面　外部公開ヘッダファイル
 * @author	Hiroyuki Nakamura
 * @date	05.02.01
 */
//============================================================================================
#ifndef B_PLIST_H
#define B_PLIST_H


#include "pm_define.h"
#include "print/printsys.h"
#include "poke_tool/pokeparty.h"
#include "battle/battle.h"
#include "system/palanm.h"


//============================================================================================
//	定数定義
//============================================================================================
#define	BPL_SEL_EXIT			( 6 )	// キャンセル
#define	BPL_SEL_WP_CANCEL	( 4 )	// 技忘れキャンセル（新しく覚える技位置）

#define	BPL_CHANGE_SEL_NONE		( BPL_SEL_EXIT )	// ダブル/トリプルですでに選択されているポケモンがいないとき

#define	BPL_SELNUM_MAX		( 3 )			// 入れ替え時に選択できる最大数

#define	BPL_SELPOS_NONE		( 0xff )	// 入れ替えなし

// リスト処理定義
enum {
	BPL_MODE_NORMAL = 0,	// 通常のポケモン選択
	BPL_MODE_CHG_DEAD,		// 瀕死入れ替え選択
	BPL_MODE_ITEMUSE,			// アイテム使用
	BPL_MODE_WAZASET,			// 技忘れ
};

// リストデータ
typedef struct {
	// [in]
	POKEPARTY * pp;					// ポケモンデータ
	POKEPARTY * multi_pp;		// マルチの相手側ポケモンデータ

	GFL_FONT * font;

	HEAPID	heap;			// ヒープID

	BtlRule	rule;			// シングル／ダブル／トリプル

	BOOL	multiMode;		// マルチバトルかどうか TRUE = マルチ, FALSE = それ以外
	u8	multiPos;				// マルチの立ち位置
	u8	change_sel[2];	// ダブル/トリプルですでに選択されているポケモン（いない場合はBPL_CHANGE_SEL_NONE）
	u8	mode;						// リストモード

	u16	item;						// アイテム
	u16	chg_waza;				// いれかえ禁止技・新しく覚える技

  GFL_TCBSYS* tcb_sys;
	PALETTE_FADE_PTR pfd;	// パレットフェードデータ

	// [in & out]
	u8	sel_poke;									// 選択されたポケモン or 戻る
	u8 * cursor_flg;							// カーソル表示フラグ

	// [out]
	u8	sel_pos[BPL_SELNUM_MAX];	// 選択されたポケモン
	u8	sel_wp;			// 選択された技位置
	u8	end_flg;		// 終了フラグ



//	void * myitem;		// アイテムデータ

//	BATTLE_WORK * bw;		// 戦闘用ワーク

//	u8	ret_mode;				// 選択された処理
//	u32	skill_item_use[2];	// 技で使用不可
//	u16	hp_rcv;			// HP回復量
//	s32 client_no;						// クライアントNo
//	u8	list_row[TEMOTI_POKEMAX];	// リストロウ
//	u8	bag_page;		// バッグのページ
}BPLIST_DATA;


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 戦闘用ポケモンリストタスク追加
 *
 * @param	dat		リストデータ
 *
 * @return none
 */
//--------------------------------------------------------------------------------------------
extern void BattlePokeList_TaskAdd( BPLIST_DATA * dat );


#endif	/* B_PLIST_H */
