//============================================================================================
/**
 * @file		b_bag_main.h
 * @brief		戦闘用バッグ画面
 * @author	Hiroyuki Nakamura
 * @date		05.02.10
 */
//============================================================================================
#pragma	once

#include "print/wordset.h"
#include "system/palanm.h"
#include "system/bgwinfrm.h"
#include "system/cursor_move.h"
#include "savedata/myitem_savedata.h"
#include "item/item.h"
#include "../btlv/btlv_finger_cursor.h"


//============================================================================================
//	定数定義
//============================================================================================
// パレット定義
#define	BBAG_PAL_TALK_WIN		( 11 )		///< パレット11：会話ウィンドウ
#define	BBAG_PAL_TALK_FONT	( 12 )		///< パレット12：会話フォント

// BMPウィンドウインデックス
enum {
	// ページ１
	WIN_P1_HP = 0,		// 「HP/PPかいふく」
	WIN_P1_ZYOUTAI,		// 「じょうたいかいふく」
	WIN_P1_BALL,		// 「ボール」
	WIN_P1_BATTLE,		// 「せんとうよう」
	WIN_P1_LASTITEM,	// 「さいごにつかったどうぐ」

	// ページ２
	WIN_P2_NAME1,		// 道具名１
	WIN_P2_NUM1,		// 道具数１
	WIN_P2_NAME2,		// 道具名２
	WIN_P2_NUM2,		// 道具数２
	WIN_P2_NAME3,		// 道具名３
	WIN_P2_NUM3,		// 道具数３
	WIN_P2_NAME4,		// 道具名４
	WIN_P2_NUM4,		// 道具数４
	WIN_P2_NAME5,		// 道具名５
	WIN_P2_NUM5,		// 道具数５
	WIN_P2_NAME6,		// 道具名６
	WIN_P2_NUM6,		// 道具数６

	WIN_P2_NAME1_S,		// 道具名１（スワップ用）
	WIN_P2_NUM1_S,		// 道具数１（スワップ用）
	WIN_P2_NAME2_S,		// 道具名２（スワップ用）
	WIN_P2_NUM2_S,		// 道具数２（スワップ用）
	WIN_P2_NAME3_S,		// 道具名３（スワップ用）
	WIN_P2_NUM3_S,		// 道具数３（スワップ用）
	WIN_P2_NAME4_S,		// 道具名４（スワップ用）
	WIN_P2_NUM4_S,		// 道具数４（スワップ用）
	WIN_P2_NAME5_S,		// 道具名５（スワップ用）
	WIN_P2_NUM5_S,		// 道具数５（スワップ用）
	WIN_P2_NAME6_S,		// 道具名６（スワップ用）
	WIN_P2_NUM6_S,		// 道具数６（スワップ用）

	WIN_P2_POCKET,		// ポケット名
	WIN_P2_PAGENUM,		// ページ数（アイテム数）

	// ページ３
	WIN_P3_NAME,		// 道具名
	WIN_P3_NUM,			// 個数
	WIN_P3_INFO,		// 説明
	WIN_P3_USE,			// 「つかう」

	WIN_MAX
};

// ページID
enum {
	BBAG_PAGE_POCKET = 0,	// ポケット選択ページ
	BBAG_PAGE_MAIN,			// アイテム選択ページ
	BBAG_PAGE_ITEM			// アイテム使用ページ
};

// セルアクターのID
enum {
	BBAG_CA_ITEM1 = 0,
	BBAG_CA_ITEM2,
	BBAG_CA_ITEM3,
	BBAG_CA_ITEM4,
	BBAG_CA_ITEM5,
	BBAG_CA_ITEM6,
	BBAG_CA_ITEM7,

	BBAG_CA_COST1_NUM,
	BBAG_CA_COST1_G1,
	BBAG_CA_COST1_G2,
	BBAG_CA_COST1_G3,
	BBAG_CA_COST1_G4,
	BBAG_CA_COST1_G5,
	BBAG_CA_COST1_G6,
	BBAG_CA_COST1_G7,

	BBAG_CA_COST2_NUM,
	BBAG_CA_COST2_G1,
	BBAG_CA_COST2_G2,
	BBAG_CA_COST2_G3,
	BBAG_CA_COST2_G4,
	BBAG_CA_COST2_G5,
	BBAG_CA_COST2_G6,
	BBAG_CA_COST2_G7,

	BBAG_CA_COST3_NUM,
	BBAG_CA_COST3_G1,
	BBAG_CA_COST3_G2,
	BBAG_CA_COST3_G3,
	BBAG_CA_COST3_G4,
	BBAG_CA_COST3_G5,
	BBAG_CA_COST3_G6,
	BBAG_CA_COST3_G7,

	BBAG_CA_COST4_NUM,
	BBAG_CA_COST4_G1,
	BBAG_CA_COST4_G2,
	BBAG_CA_COST4_G3,
	BBAG_CA_COST4_G4,
	BBAG_CA_COST4_G5,
	BBAG_CA_COST4_G6,
	BBAG_CA_COST4_G7,

	BBAG_CA_COST5_NUM,
	BBAG_CA_COST5_G1,
	BBAG_CA_COST5_G2,
	BBAG_CA_COST5_G3,
	BBAG_CA_COST5_G4,
	BBAG_CA_COST5_G5,
	BBAG_CA_COST5_G6,
	BBAG_CA_COST5_G7,

	BBAG_CA_COST6_NUM,
	BBAG_CA_COST6_G1,
	BBAG_CA_COST6_G2,
	BBAG_CA_COST6_G3,
	BBAG_CA_COST6_G4,
	BBAG_CA_COST6_G5,
	BBAG_CA_COST6_G6,
	BBAG_CA_COST6_G7,

	BBAG_CA_ENERGIE_NUM,
	BBAG_CA_ENERGIE_G1,
	BBAG_CA_ENERGIE_G2,
	BBAG_CA_ENERGIE_G3,
	BBAG_CA_ENERGIE_G4,
	BBAG_CA_ENERGIE_G5,
	BBAG_CA_ENERGIE_G6,
	BBAG_CA_ENERGIE_G7,

	BBAG_CA_MAX
};

#define	BBAG_POCKET_IN_MAX		( 36 )		// 各ポケットに入る数

// ポケットボタンサイズ
#define	BBAG_BSX_POCKET	( 16 )
#define	BBAG_BSY_POCKET	( 9 )
// 使用ボタンサイズ
#define	BBAG_BSX_USE	( 26 )
#define	BBAG_BSY_USE	( 5 )
// 戻るボタンサイズ
#define	BBAG_BSX_RET	( 5 )
#define	BBAG_BSY_RET	( 5 )
// アイテムボタンサイズ
#define	BBAG_BSX_ITEM	( 16 )
#define	BBAG_BSY_ITEM	( 6 )
// 前のページへボタンサイズ
#define	BBAG_BSX_UP		( 5 )
#define	BBAG_BSY_UP		( 5 )
// 次のページへボタンサイズ
#define	BBAG_BSX_DOWN	( 5 )
#define	BBAG_BSY_DOWN	( 5 )
// ポケットボタンのアイコンサイズ
#define	BBAG_BSX_ICON	( 4 )
#define	BBAG_BSY_ICON	( 4 )

// OBJリソース
enum {
	// キャラ
	BBAG_CHRRES_ITEM1 = 0,
	BBAG_CHRRES_ITEM2,
	BBAG_CHRRES_ITEM3,
	BBAG_CHRRES_ITEM4,
	BBAG_CHRRES_ITEM5,
	BBAG_CHRRES_ITEM6,
	BBAG_CHRRES_ITEM7,
	BBAG_CHRRES_COST,
	BBAG_CHRRES_CURSOR,
	BBAG_CHRRES_MAX,

	// パレット
	BBAG_PALRES_ITEM1 = 0,
	BBAG_PALRES_ITEM2,
	BBAG_PALRES_ITEM3,
	BBAG_PALRES_ITEM4,
	BBAG_PALRES_ITEM5,
	BBAG_PALRES_ITEM6,
	BBAG_PALRES_ITEM7,
	BBAG_PALRES_COST,
	BBAG_PALRES_CURSOR,			// ※この次に捕獲デモカーソルを読み込んでいます！
	BBAG_PALRES_MAX,

	// セル・アニメ
	BBAG_CELRES_ITEM = 0,
	BBAG_CELRES_COST,
	BBAG_CELRES_CURSOR,
	BBAG_CELRES_MAX,
};

// ワーク
typedef struct {
	BBAG_DATA * dat;		// 外部参照データ

	GFL_TCBLSYS * tcbl;		// TCBL

	PALETTE_FADE_PTR pfd;	// パレットフェードデータ

	GFL_MSGDATA * mman;			// メッセージデータマネージャ
	WORDSET * wset;					// 単語セット
	STRBUF * msg_buf;				// メッセージ用バッファ
	PRINT_QUE * que;				// プリントキュー
	PRINT_STREAM * stream;	// プリントストリーム

	GFL_BMPWIN * talk_win;
	PRINT_UTIL	add_win[WIN_MAX];

	u8	bmp_add_max;			// 追加したBMPの数
	u8	p2_swap;					// ページ２のスワップ描画フラグ

	CURSORMOVE_WORK * cmwk;					// カーソル制御ワーク
	BAPP_CURSOR_PUT_WORK * cpwk;		// カーソル表示

	BTLV_FINGER_CURSOR_WORK * getdemoCursor;

	ITEM_ST	pocket[5][BBAG_POCKET_IN_MAX];	// ポケットデータ

	GFL_CLUNIT * clunit;
	GFL_CLWK * clwk[BBAG_CA_MAX];

	// ボタンアニメスクリーンデータ
	BGWINFRM_WORK * bgwfrm;

	u8	btn_seq;
	u8	btn_cnt;
	u8	btn_id;
	u8	btn_flg;
	u8	btn_win[8];

	u8	seq;				// メインシーケンス
	u8	ret_seq;		// 復帰シーケンス
	u8	page;				// ページID
	u8	poke_id;		// ポケットID

	s8	page_mv;		// アイテム選択ページの移動方向

	u8	item_max[BATTLE_BAG_POKE_MAX];	// アイテム数
	u8	scr_max[BATTLE_BAG_POKE_MAX];		// アイテムスクロールカウンタ最大値

	u8	get_seq;		// 捕獲デモシーケンス
	u8	get_cnt;		// 捕獲デモカウンタ

  u16 used_item;    // 前回使用したアイテム
  u8  used_poke;    // 前回使用したアイテムのポケット

//	WB追加
	u32	chrRes[BBAG_CHRRES_MAX];
	u32	palRes[BBAG_PALRES_MAX];
	u32	celRes[BBAG_CELRES_MAX];
	BOOL cursor_flg;

	const u8 * putWin;

}BBAG_WORK;

typedef int (*pBBagFunc)(BBAG_WORK*);

