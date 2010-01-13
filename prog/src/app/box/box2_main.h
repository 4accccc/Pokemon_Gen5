//============================================================================================
/**
 * @file		box2_main.h
 * @brief		ボックス画面 メイン処理
 * @author	Hiroyuki Nakamura
 * @date		09.10.05
 *
 *	モジュール名：BOX2MAIN
 */
//============================================================================================
#pragma	once

#include "pm_define.h"
#include "savedata/box_savedata.h"
#include "system/palanm.h"
#include "system/bgwinfrm.h"
#include "system/cursor_move.h"
#include "system/bmp_oam.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "app/app_taskmenu.h"
#include "app/box2.h"


//============================================================================================
//	定数定義
//============================================================================================

// [ HEAPID_BOX_SYS ] 後方確保用定義
#define	HEAPID_BOX_SYS_L		( GFL_HEAP_LOWID(HEAPID_BOX_SYS) )
// [ HEAPID_BOX_APP ] 後方確保用定義
#define	HEAPID_BOX_APP_L		( GFL_HEAP_LOWID(HEAPID_BOX_APP) )

// BGパレット
#define	BOX2MAIN_BG_PAL_TOUCH_BAR	( 7 )			// タッチバー
#define	BOX2MAIN_BG_PAL_YNWIN			( 8 )			// はい・いいえウィンド
#define	BOX2MAIN_BG_PAL_SYSWIN		( 10 )		// メッセージウィンドウ
#define	BOX2MAIN_BG_PAL_SYSFNT		( 11 )		// メッセージフォント
#define	BOX2MAIN_BG_PAL_SELWIN		( 12 )		// 選択ボタン（２本）
#define	BOX2MAIN_BG_PAL_WALL1			( 14 )		// 壁紙１
#define	BOX2MAIN_BG_PAL_WALL2			( 15 )		// 壁紙２

#define	BOX2MAIN_BG_PAL_SYSFNT_S	( 15 )		// サブ画面フォントパレット

// 選択ウィンドウ定義
#define	BOX2MAIN_SELWIN_CGX_SIZ	( 3 * 4 )
#define	BOX2MAIN_SELWIN_CGX_POS	( 1024 - BOX2MAIN_SELWIN_CGX_SIZ * 2 )
#define	BOX2MAIN_SELWIN_CGX_OFF	( BOX2MAIN_SELWIN_CGX_POS )
// はい・いいえウィンドウ定義
#define	BOX2MAIN_YNWIN_CGX_SIZ	( 9 )
#define	BOX2MAIN_YNWIN_CGX_POS	( BOX2MAIN_SELWIN_CGX_POS - BOX2MAIN_YNWIN_CGX_SIZ )

// タッチバー定義
#define	BOX2MAIN_TOUCH_BAR_PX		( 0 )
#define	BOX2MAIN_TOUCH_BAR_PY		( 21 )
#define	BOX2MAIN_TOUCH_BAR_SX		( 32 )
#define	BOX2MAIN_TOUCH_BAR_SY		( 3 )

// ポケモンアイコン数
#define	BOX2OBJ_POKEICON_H_MAX		( BOX_MAX_COLUMN )
#define	BOX2OBJ_POKEICON_V_MAX		( BOX_MAX_RAW )
#define	BOX2OBJ_POKEICON_TRAY_MAX	( BOX_MAX_POS )
#define	BOX2OBJ_POKEICON_MINE_MAX	( TEMOTI_POKEMAX )
// トレイ＋手持ち
#define	BOX2OBJ_POKEICON_PUT_MAX	( BOX2OBJ_POKEICON_TRAY_MAX + BOX2OBJ_POKEICON_MINE_MAX )
// トレイ＋手持ち＋取得中
#define	BOX2OBJ_POKEICON_MAX		( BOX2OBJ_POKEICON_PUT_MAX + 1 )
// 取得中アイコン位置
#define	BOX2OBJ_POKEICON_GET_POS	( BOX2OBJ_POKEICON_MAX - 1 )

#define	BOX2OBJ_PI_OUTLINE_MAX		( 8 )

#define	BOX2OBJ_TRAYICON_MAX		( 6 )

// ポケモンアイコンのキャラサイズ
#define	BOX2OBJ_POKEICON_CGX_SIZE	( 0x20 * 4 * 4 )

#define	BOX2MAIN_GETPOS_NONE		( 0xff )	// ポケモン未取得

#define	BOX2MAIN_PPP_GET_MINE		( 0xff )	// BOX2MAIN_PPPGet()で手持ちを取得するときのトレイ定義

//#define	BOX2MAIN_BOXMOVE_FLG		( 0x80 )	// 他のトレイに投げるときのフラグ

#define	BOX2MAIN_TRAY_SCROLL_SPD		( 8 )		// トレイスクロール速度
#define	BOX2MAIN_TRAY_SCROLL_CNT		( 23 )	// トレイスクロールカウント


// キャラリソースID
enum {
	BOX2MAIN_CHRRES_POKEICON = 0,	// ポケモンアイコン
	BOX2MAIN_CHRRES_POKEGRA = BOX2MAIN_CHRRES_POKEICON + BOX2OBJ_POKEICON_MAX,
	BOX2MAIN_CHRRES_POKEGRA2,
	BOX2MAIN_CHRRES_ITEMICON,
	BOX2MAIN_CHRRES_ITEMICON_SUB,
	BOX2MAIN_CHRRES_TYPEICON,
	BOX2MAIN_CHRRES_TRAYICON = BOX2MAIN_CHRRES_TYPEICON + POKETYPE_MAX,
	BOX2MAIN_CHRRES_BOXOBJ = BOX2MAIN_CHRRES_TRAYICON + BOX2OBJ_TRAYICON_MAX,
	BOX2MAIN_CHRRES_BOX_BAR,
	BOX2MAIN_CHRRES_TOUCH_BAR,
	BOX2MAIN_CHRRES_POKEMARK,
	BOX2MAIN_CHRRES_POKEMARK_SUB,
	BOX2MAIN_CHRRES_POKERUSICON,
	BOX2MAIN_CHRRES_MAX
};
// パレットリソースID
enum {
	BOX2MAIN_PALRES_POKEICON = 0,
	BOX2MAIN_PALRES_POKEGRA,
	BOX2MAIN_PALRES_POKEGRA2,
	BOX2MAIN_PALRES_ITEMICON,
	BOX2MAIN_PALRES_ITEMICON_SUB,
	BOX2MAIN_PALRES_TYPEICON,
	BOX2MAIN_PALRES_TRAYICON,
	BOX2MAIN_PALRES_BOXOBJ,
	BOX2MAIN_PALRES_TOUCH_BAR,
	BOX2MAIN_PALRES_POKEMARK,
	BOX2MAIN_PALRES_POKEMARK_SUB,
	BOX2MAIN_PALRES_POKERUSICON,
	BOX2MAIN_PALRES_MAX
};
// セルリソースID
enum {
	BOX2MAIN_CELRES_POKEICON = 0,
	BOX2MAIN_CELRES_POKEGRA,
	BOX2MAIN_CELRES_POKEGRA2,
	BOX2MAIN_CELRES_ITEMICON,
	BOX2MAIN_CELRES_TYPEICON,
	BOX2MAIN_CELRES_TRAYICON,
	BOX2MAIN_CELRES_BOXOBJ,
	BOX2MAIN_CELRES_BOX_BAR,
	BOX2MAIN_CELRES_TOUCH_BAR,
	BOX2MAIN_CELRES_POKEMARK,
	BOX2MAIN_CELRES_POKEMARK_SUB,
	BOX2MAIN_CELRES_POKERUSICON,
	BOX2MAIN_CELRES_MAX
};

// OBJ ID
enum {
	BOX2OBJ_ID_L_ARROW = 0,
	BOX2OBJ_ID_R_ARROW,

	BOX2OBJ_ID_TRAY_CUR,	// トレイカーソル
	BOX2OBJ_ID_TRAY_NAME,	// トレイ名背景

	BOX2OBJ_ID_HAND_CURSOR,	// 手カーソル
	BOX2OBJ_ID_HAND_SHADOW,	// 手カーソル影

	BOX2OBJ_ID_TB_CANCEL,		// タッチバー戻るボタン（ボックスメニューへ）
	BOX2OBJ_ID_TB_END,			// タッチバー終了ボタン（C-gearへ）
	BOX2OBJ_ID_TB_STATUS,		// タッチバーステータスボタン

	BOX2OBJ_ID_ITEMICON,
	BOX2OBJ_ID_ITEMICON_SUB,

	BOX2OBJ_ID_POKEGRA,
	BOX2OBJ_ID_POKEGRA2,

	BOX2OBJ_ID_MARK1,
	BOX2OBJ_ID_MARK2,
	BOX2OBJ_ID_MARK3,
	BOX2OBJ_ID_MARK4,
	BOX2OBJ_ID_MARK5,
	BOX2OBJ_ID_MARK6,

	BOX2OBJ_ID_MARK1_S,
	BOX2OBJ_ID_MARK2_S,
	BOX2OBJ_ID_MARK3_S,
	BOX2OBJ_ID_MARK4_S,
	BOX2OBJ_ID_MARK5_S,
	BOX2OBJ_ID_MARK6_S,

	BOX2OBJ_ID_RARE,
	BOX2OBJ_ID_POKERUS,
	BOX2OBJ_ID_POKERUS_ICON,

	BOX2OBJ_ID_TYPEICON,

	BOX2OBJ_ID_TRAYICON = BOX2OBJ_ID_TYPEICON + POKETYPE_MAX,

	BOX2OBJ_ID_POKEICON = BOX2OBJ_ID_TRAYICON + BOX2OBJ_TRAYICON_MAX,

	BOX2OBJ_ID_OUTLINE = BOX2OBJ_ID_POKEICON + BOX2OBJ_POKEICON_MAX,

	BOX2OBJ_ID_MAX = BOX2OBJ_ID_OUTLINE + BOX2OBJ_PI_OUTLINE_MAX
};

// BMPWIN ID
enum {
	// 上画面
	BOX2BMPWIN_ID_NAME = 0,		// ポケモン名
	BOX2BMPWIN_ID_NICKNAME,		// ニックネーム
	BOX2BMPWIN_ID_LV,					// レベル
	BOX2BMPWIN_ID_TITLE,			// タイトル
	BOX2BMPWIN_ID_SEX,				// 性別
	BOX2BMPWIN_ID_ZKNNUM,			// 図鑑番号
	BOX2BMPWIN_ID_SEIKAKU,		// 性格
	BOX2BMPWIN_ID_TOKUSEI,		// 特性
	BOX2BMPWIN_ID_ITEM,				// 持ち物
	BOX2BMPWIN_ID_WAZA,				// 技

	BOX2BMPWIN_ID_WAZA_STR,			//「もっているわざ」
	BOX2BMPWIN_ID_SEIKAKU_STR,	//「せいかく」
	BOX2BMPWIN_ID_TOKUSEI_STR,	//「とくせい」
	BOX2BMPWIN_ID_ITEM_STR,			//「もちもの」

//	BOX2BMPWIN_ID_ITEMLABEL,	// アイテム名（説明ウィンドウのタブ部分）
//	BOX2BMPWIN_ID_ITEMINFO,		// アイテム説明
//	BOX2BMPWIN_ID_ITEMLABEL2,	// アイテム名（説明ウィンドウのタブ部分）（スワップ用）
//	BOX2BMPWIN_ID_ITEMINFO2,	// アイテム説明（スワップ用）

	// 下画面
	BOX2BMPWIN_ID_MENU1,		// メニュー１
	BOX2BMPWIN_ID_MENU2,		// メニュー２
	BOX2BMPWIN_ID_MENU3,		// メニュー３
	BOX2BMPWIN_ID_MENU4,		// メニュー４
	BOX2BMPWIN_ID_MENU5,		// メニュー５
	BOX2BMPWIN_ID_MENU6,		// メニュー６

	BMPWIN_MARK_ENTER,			// マーキング決定
	BMPWIN_MARK_CANCEL,			// マーキングキャンセル

	BOX2BMPWIN_ID_TEMOCHI,	//「てもちポケモン」
	BOX2BMPWIN_ID_BOXLIST,	//「ボックスリスト」

	BOX2BMPWIN_ID_MSG1,			// メッセージ１
	BOX2BMPWIN_ID_MSG2,			// メッセージ２
	BOX2BMPWIN_ID_MSG3,			// メッセージ３
	BOX2BMPWIN_ID_MSG4,			// メッセージ４

	BOX2BMPWIN_ID_MAX
};

// BGWINFRAME
enum {
	BOX2MAIN_WINFRM_MENU1 = 0,	// メニューフレーム
	BOX2MAIN_WINFRM_MENU2,			// メニューフレーム
	BOX2MAIN_WINFRM_MENU3,			// メニューフレーム
	BOX2MAIN_WINFRM_MENU4,			// メニューフレーム
	BOX2MAIN_WINFRM_MENU5,			// メニューフレーム
	BOX2MAIN_WINFRM_MENU6,			// メニューフレーム

	BOX2MAIN_WINFRM_TOUCH_BAR,	// タッチバー

	BOX2MAIN_WINFRM_MARK,				// マーキングフレーム
	BOX2MAIN_WINFRM_PARTY,			// 手持ちポケモンフレーム
	BOX2MAIN_WINFRM_MOVE,				// ボックス移動フレーム

	BOX2MAIN_WINFRM_POKE_BTN,			//「てもちポケモン」ボタン
	BOX2MAIN_WINFRM_BOXLIST_BTN,	//「ボックスリスト」ボタン

//	BOX2MAIN_WINFRM_RET_BTN,	//「もどる」ボタン
//	BOX2MAIN_WINFRM_CLOSE_BTN,	//「とじる」ボタン
//	BOX2MAIN_WINFRM_BOXMV_MENU,	// ボックス移動時のメニュー
//	BOX2MAIN_WINFRM_BOXMV_BTN,	// ボックス移動時のボタン
//	BOX2MAIN_WINFRM_Y_ST_BTN,	// Ｙボタンステータス

//	BOX2MAIN_WINFRM_SUBDISP,	// 上画面用（技・持ち物）

	BOX2MAIN_WINFRM_MAX,
};

// ＯＡＭフォント
enum {
	BOX2MAIN_FNTOAM_TRAY_NAME = 0,
	BOX2MAIN_FNTOAM_TRAY_NUM1,
	BOX2MAIN_FNTOAM_TRAY_NUM2,
	BOX2MAIN_FNTOAM_TRAY_NUM3,
	BOX2MAIN_FNTOAM_TRAY_NUM4,
	BOX2MAIN_FNTOAM_TRAY_NUM5,
	BOX2MAIN_FNTOAM_TRAY_NUM6,
	BOX2MAIN_FNTOAM_BOX_NAME1,
	BOX2MAIN_FNTOAM_BOX_NAME2,
	BOX2MAIN_FNTOAM_MAX
};

// VBLANK関数ワーク
typedef struct {
	void * func;
	void * freq;
	void * work;
	u16	seq;
	u16	cnt;
//	BOOL flg;		// 稼動フラグ
}BOX2_IRQWK;

// ＯＡＭフォントワーク
typedef struct {
	BMPOAM_ACT_PTR oam;
	GFL_BMP_DATA * bmp;
//	CHAR_MANAGER_ALLOCDATA	cma;
}BOX2_FONTOAM;

// ボタンアニメモード
enum {
	BOX2MAIN_BTN_ANM_MODE_OBJ = 0,
	BOX2MAIN_BTN_ANM_MODE_BG
};

// ボタンアニメワーク
typedef struct {
	u8	btn_mode:1;
	u8	btn_id:7;			// OBJ = ID, BG = FRM
	u8	btn_pal1:4;
	u8	btn_pal2:4;
	u8	btn_seq;
	u8	btn_cnt;

	u8	btn_px;			// BGのみ
	u8	btn_py;
	u8	btn_sx;
	u8	btn_sy;
}BUTTON_ANM_WORK;

// ボックス画面アプリワーク
typedef struct {
	// ＯＡＭフォント
	BMPOAM_SYS_PTR	fntoam;
	BOX2_FONTOAM	fobj[BOX2MAIN_FNTOAM_MAX];

/*
	// はい・いいえ関連
	TOUCH_SW_SYS * tsw;		// タッチウィンドウ
	u16	ynID;				// はい・いいえＩＤ

	u16	obj_trans_stop;		// VBLANKでのOBJ描画を停止するフラグ ( 1 = stop )


*/
	int	key_repeat_speed;
	int	key_repeat_wait;

	GFL_TCB * vtask;					// TCB ( VBLANK )

	BOX2_IRQWK	vfunk;				// VBLANK関数ワーク
	int	vnext_seq;

	PALETTE_FADE_PTR	pfd;		// パレットフェードデータ

	CURSORMOVE_WORK * cmwk;		// カーソル移動ワーク
	BGWINFRM_WORK * wfrm;			// ウィンドウフレーム

	GFL_FONT * font;					// 通常フォント
	GFL_FONT * nfnt;					// 8x8フォント
	GFL_MSGDATA * mman;				// メッセージデータマネージャ
	WORDSET * wset;						// 単語セット
	STRBUF * exp;							// メッセージ展開領域
	PRINT_QUE * que;					// プリントキュー
	PRINT_STREAM * stream;		// プリントストリーム

	PRINT_UTIL	win[BOX2BMPWIN_ID_MAX];		// BMPWIN

	GFL_ARCUTIL_TRANSINFO	syswinInfo;

	// はい・いいえ関連
//	TOUCH_SW_SYS * tsw;		// タッチウィンドウ
	APP_TASKMENU_ITEMWORK	ynList[2];
	APP_TASKMENU_RES * ynRes;
	APP_TASKMENU_WORK * ynWork;
	u16	ynID;				// はい・いいえＩＤ

	BUTTON_ANM_WORK	bawk;				// ボタンアニメワーク

//	ARCHANDLE * ppd_ah;				// パーソナルデータのアークハンドル

	// ポケモンアイコン
	ARCHANDLE * pokeicon_ah;

	u8	pokeicon_cgx[BOX2OBJ_POKEICON_TRAY_MAX][BOX2OBJ_POKEICON_CGX_SIZE];
	u8	pokeicon_pal[BOX2OBJ_POKEICON_TRAY_MAX];
	u8	pokeicon_id[BOX2OBJ_POKEICON_MAX];
	BOOL	pokeicon_exist[BOX2OBJ_POKEICON_TRAY_MAX];

	// トレイアイコンのキャラデータ
	u8	trayicon_cgx[BOX_MAX_TRAY][0x40*4*4];

	// OBJ
	GFL_CLUNIT * clunit;
	GFL_CLWK * clwk[BOX2OBJ_ID_MAX];
	u32	chrRes[BOX2MAIN_CHRRES_MAX];
	u32	palRes[BOX2MAIN_PALRES_MAX];
	u32	celRes[BOX2MAIN_CELRES_MAX];

	u16	pokegra_swap;		// 上画面ポケモングラフィック切替フラグ

//	BOOL cancel_flg;		// ポケモン取得キャンセルフラグ

	u32	old_cur_pos;		// 前回のカーソル位置（外部で必要なため）

	u16	get_item;						// 取得アイテム
	u16	get_item_init_pos;	// アイテム取得位置

//	u8	subdisp_win_swap:4;		// サブ画面BMPWINのスワップフラグ
//	u8	subdisp_waza_put:3;		// サブ画面技BMPWINのスワップフラグ
//	u8	poke_free_err:1;			// 逃がすが実行できないとき１
	u8	poke_free_err;			// 逃がすが実行できないとき

//	u8	party_frm;			// 手持ちポケモンフレーム制御

	u8	msg_put;				// メッセージを表示するか
	u8	mv_err_code;		// ポケモン移動エラーコード

//	BOOL poke_mv_cur_flg;	// ポケモンアイコン移動後にアウトラインを表示するか

	u32	pokeMark;			// マーキング変更ワーク

	// ボックス切り替え関連
	s8	wall_px;
	u8	wall_area;

	u8	wallpaper_pos;				// 壁紙選択位置
	u8	wp_menu;				// 壁紙メニュー番号

	u8	poke_put_key;		// キーでの配置位置
	u8	chg_tray_pos;		// 切り替えるトレイ

	u32	tpx;				// タッチＸ座標
	u32	tpy;				// タッチＹ座標

	int	wipe_seq;							// ワイプ後のシーケンス
	int	wait;									// ウェイト

	void * seqwk;			// シーケンスワーク
	int	sub_seq;			// シーケンス内シーケンス

}BOX2_APP_WORK;

// ボックス画面システムワーク
typedef struct {

/*
	BOX_DATA * box;			// ボックスデータ
	POKEPARTY * party;		// 手持ちポケモン
	MYITEM * item;			// アイテムデータ
	CONFIG * config;		// コンフィグデータ


	u8	pos;				// 現在のカーソル位置
*/


	BOX2_GFL_PROC_PARAM * dat;	// 外部データ

	void * subProcWork;				// サブプロセスで使用するワーク
	u16	subRet;								// サブプロセスの戻り値などを保存する場所
	u8	subProcType;					// サブプロセスの種類
	u8	subProcMode;					// サブプロセスの呼び出しモード

	u8	tray;										// 現在のトレイ
	u8	trayMax;								// 開放されているトレイ数

//	POKEMON_PARAM * getPP;			// 取得したポケモンをコピーしておく領域

	u8	get_pos;								// 取得位置
	u8	get_tray;								// 取得トレイ
	u8	poke_get_key;						// 掴んでいるか

	u8	box_mv_pos;							// ボックス移動選択位置
//	u8	box_mv_scroll;

	u8	move_mode;			// ポケモン移動モード

	u8	mv_cnv_mode:4;		// 便利モードフラグ 1 = yes, 0 = no
	u8	mv_cnv_end:4;			// 掴む終了フラグ

//	u8	quick_mode;			//「ポケモンいどう」のモード ( 0 = 通常, 1 = トレイポケモン, 2 = 手持ちポケモン )
//	u8	quick_get;			//「ポケモンいどう」メニューでの取得位置

//	u8	y_status_flg;		// Ｙボタンでステータスを開いたかどうか
//	u8	y_status_hand;		// Ｙステータス時に掴んでいるか
	u8	tb_status_pos;		// 掴んでいるときのステータス画面からの戻り位置

	u8	pokechg_mode;		// 入れ替えモード

//	u8	party_sel_flg;		// 手持ち選択フラグ
//	u8	box_mv_flg;			// ボックス移動フラグ

	u8	tb_ret_btn;			// タッチバーに表示している戻るボタンの表示状況
	u8	tb_exit_btn;		// タッチバーに表示している終了ボタンの表示状況
	u8	tb_status_btn;	// タッチバーに表示しているステータスボタンの表示状況

	u32	cur_rcv_pos;		// カーソル表示復帰位置

	int	next_seq;								// 次のシーケンス

	BOX2_APP_WORK * app;				// アプリワーク

}BOX2_SYS_WORK;

typedef int (*pBOX2_FUNC)(BOX2_SYS_WORK*);


/*
// キー操作データ取得定義
enum {
	BOX2MAIN_KEY_JUMP_UP = 0,
	BOX2MAIN_KEY_JUMP_DOWN,
	BOX2MAIN_KEY_JUMP_LEFT,
	BOX2MAIN_KEY_JUMP_RIGHT,
};

// 場所タイプ取得定義
enum {
	BOX2MAIN_POS_TYPE_BOXNAME = 0,
	BOX2MAIN_POS_TYPE_SCROLL_L,
	BOX2MAIN_POS_TYPE_SCROLL_R,
	BOX2MAIN_POS_TYPE_BOXPOKE,
	BOX2MAIN_POS_TYPE_PARTYPOKE,
};
*/
// ボックス切り替え方向
enum {
	BOX2MAIN_TRAY_SCROLL_L = 0,
	BOX2MAIN_TRAY_SCROLL_R,
	BOX2MAIN_TRAY_SCROLL_NONE,
};

// ポケモンを逃がす処理のワーク
typedef struct {
	GFL_CLWK * clwk;		// アイコンセルアクター
	u16	check_cnt;			// 技チェックカウント
	u8	check_flg;			// 技チェックフラグ ( 0xff = def, 0 = none, 1 = hit )
	u8	scale_cnt;			// 拡縮カウント
	f32	scale;				// 拡縮値
}BOX2MAIN_POKEFREE_WORK;

// 手持ちフレーム動作定義
#define	BOX2MAIN_PARTYPOKE_FRM_SPD	( 8 )		// 移動量
#define	BOX2MAIN_PARTYPOKE_FRM_CNT	( 18 )		// カウント

#define	BOX2MAIN_PARTYPOKE_FRM_H_SPD	( 8 )	// 手持ちポケモンフレーム横スクロール速度
#define	BOX2MAIN_PARTYPOKE_FRM_H_CNT	( 19 )	// 手持ちポケモンフレーム横スクロールカウント

// ポケモン移動モード
enum {
	BOX2MAIN_POKEMOVE_MODE_TRAY = 0,
	BOX2MAIN_POKEMOVE_MODE_PARTY,
	BOX2MAIN_POKEMOVE_MODE_ALL,
};

// 取得ポケモン移動
typedef struct {
	u16	df_pos;		// 移動させるアイコンのデータ位置
	u16	mv_pos;		// 移動先の画面位置
	u16	gt_pos;		// 移動元の画面位置
	u16	flg;
	u32	mx;
	u32	my;
	s16	vx;
	s16	vy;
	s16	dx;
	s16	dy;
}BOX2MAIN_POKEMOVE_DATA;

typedef struct {
	BOX2MAIN_POKEMOVE_DATA	dat[BOX2OBJ_POKEICON_MINE_MAX+1];	// 手持ち＋トレイ選択分
	u32	cnt;
	u32	get_pos;
	u32	put_pos;
	u32	set_pos;
}BOX2MAIN_POKEMOVE_WORK;

// 取得アイテム移動
typedef struct {
	u16	put_pos;
	u16	set_pos;
	u32	mv_mode;

	u32	now_x;
	u32	now_y;
	u32	mx;
	u32	my;
	u32	mv_x:1;
	u32	mv_y:1;
	u32	cnt:30;
}BOX2MAIN_ITEMMOVE_WORK;

// 表示されているトレイアイコンのトレイ番号の基準値を取得
#define	BOX2MAIN_MV_TRAY_DEF(a)	( a / BOX2OBJ_TRAYICON_MAX * BOX2OBJ_TRAYICON_MAX )

// カーソル移動
typedef struct {
	u8	px;			// 移動先Ｘ座標
	u8	py;			// 移動先Ｙ座標
	u8	vx;			// Ｘ移動量
	u8	vy;			// Ｙ移動量
	u32	mx:1;		// Ｘ移動方向
	u32	my:1;		// Ｙ移動方向
	u32	cnt:30;		// カウンタ
}BOX2MAIN_CURSORMOVE_WORK;

// 上画面表示用データ
typedef struct {
	POKEMON_PASO_PARAM * ppp;

	u16	mons;					// ポケモン番号
	u16	item;					// アイテム番号

	u32	rand;					// 個性乱数

	u8	type1;				// タイプ１
	u8	type2;				// タイプ２
	u8	tokusei;			// 特性
	u8	seikaku;			// 性格

	u16	mark;					// ボックスマーク
	u8	lv:7;					// レベル
	u8	tamago:1;			// タマゴフラグ
	u8	sex:4;				// 性別
	u8	sex_put:1;		// 性別表示（ニドラン用）1=ON, 0=OFF
	u8	rare:1;				// レア
	u8	pokerus:2;		// ポケルス

	u16	waza[4];			// 技

}BOX2_POKEINFO_DATA;


//#define	BOX2MAIN_TRAYICON_SCROLL_CNT		( 9 )
#define	BOX2MAIN_TRAYICON_SCROLL_CNT		( 5 )


// ポケモン移動エラーコード
enum {
	BOX2MAIN_ERR_CODE_NONE = 0,		// エラーなし
	BOX2MAIN_ERR_CODE_BOXMAX,			// ボックスがいっぱい
	BOX2MAIN_ERR_CODE_MAIL,				// メールを持っている
	BOX2MAIN_ERR_CODE_BATTLE,			// 戦えるポケモンがいなくなる
	BOX2MAIN_ERR_CODE_EGG,				// タマゴはえらべない（バトルボックス専用）
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数設定
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_InitVBlank( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数削除
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_ExitVBlank( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VRAM設定
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_VramBankSet(void);

extern const GFL_DISP_VRAM * BOX2MAIN_GetVramBankData(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧ初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_BgInit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧ解放
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_BgExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＧグラフィック読み込み
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_BgGraphicLoad( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェード初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PaletteFadeInit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェード解放
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PaletteFadeExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief	 アルファブレンド設定
 *
 * @param		flg		TRUE = 有効, FALSE = 無効
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_SetBlendAlpha( BOOL flg );

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_MsgInit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連解放
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_MsgExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンデータ取得
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 * @param		prm			取得パラメータ
 * @param		buf			取得バッファ
 *
 * @return	取得データ
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2MAIN_PokeParaGet( BOX2_SYS_WORK * syswk, u16 pos, u16 tray, int prm, void * buf );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンデータ設定
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 * @param		prm			設定パラメータ
 * @param		buf			設定データ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeParaPut( BOX2_SYS_WORK * syswk, u32 pos, u32 tray, int prm, u32 buf );

//--------------------------------------------------------------------------------------------
/**
 * @brief		POKEMON_PASO_PARAM取得
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		tray		トレイ番号
 * @param		pos			位置
 *
 * @return	POKEMON_PASO_PARAM
 */
//--------------------------------------------------------------------------------------------
extern POKEMON_PASO_PARAM * BOX2MAIN_PPPGet( BOX2_SYS_WORK * syswk, u32 tray, u32 pos );

// ポケモンデータ取得
//extern void BOX2MAIN_GetPokeData( BOX2_SYS_WORK * syswk, u32 tray, u32 pos );

// 取得したポケモンデータを配置
//extern void BOX2MAIN_PutPokeData( BOX2_SYS_WORK * syswk, u32 tray, u32 pos );


//--------------------------------------------------------------------------------------------
/**
 * @brief		壁紙セット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		id			壁紙ＩＤ
 * @param		mv			スクロール方向
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_WallPaperSet( BOX2_SYS_WORK * syswk, u32 id, u32 mv );

//--------------------------------------------------------------------------------------------
/**
 * @brief		壁紙番号取得
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		num			トレイ番号
 *
 * @return	壁紙番号
 *
 *	デフォルト壁紙からの通し番号にする
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2MAIN_GetWallPaperNumber( BOX2_SYS_WORK * syswk, u32 num );



extern u8 BOX2MAIN_GetBoxMoveTrayNum( BOX2_SYS_WORK * syswk, s8 mv );











//============================================================================================
//	システム関連
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * VBLANK処理
 *
 * @param	tcb
 * @param	work
 *
 * @return	none
 *
 *	1/60のための処理
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_VBlank( GFL_TCB* tcb, void * work );
//↑[GS_CONVERT_TAG]

//--------------------------------------------------------------------------------------------
/**
 * VBLANK内で呼ばれる関数を設定
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	func	関数ポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_VFuncSet( BOX2_APP_WORK * appwk, void * func );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK内で呼ばれる関数を予約
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	func	関数ポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_VFuncReq( BOX2_APP_WORK * appwk, void * func );

//--------------------------------------------------------------------------------------------
/**
 * 予約したVBLANK関数をセット
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_VFuncReqSet( BOX2_APP_WORK * appwk );

//--------------------------------------------------------------------------------------------
/**
 * VRAM設定
 *
 * @param	none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_VramBankSet(void);

//--------------------------------------------------------------------------------------------
/**
 * ＢＧ初期化
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_BgInit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * ＢＧ解放
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_BgExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * ＢＧグラフィック読み込み
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_BgGraphicLoad( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * メッセージ関連初期化
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_MsgInit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * メッセージ関連解放
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_MsgExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * ウィンドウ読み込み
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_WindowLoad( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * パレットフェード初期化
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PaletteFadeInit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * パレットフェード解放
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PaletteFadeExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * アルファブレンド設定
 *
 * @param	flg		TRUE = 有効, FALSE = 無効
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_SetBlendAlpha( BOOL flg );

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえ処理初期化
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_YesNoWinInit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえ処理解放
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_YesNoWinExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえセット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		カーソル位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_YesNoWinSet( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * ボタンアニメ処理
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"TRUE = アニメ中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_ButtonAnmMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * キータッチステータスセット
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none 
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_KeyTouchStatusSet( BOX2_SYS_WORK * syswk );


extern void BOX2MAIN_ResetTouchBar( BOX2_SYS_WORK * syswk );

extern void BOX2MAIN_InitTrayScroll( BOX2_SYS_WORK * syswk, u32 mvID );

extern void BOX2MAIN_InitTrayIconScroll( BOX2_SYS_WORK * syswk, s32 mv );

extern void BOX2MAIN_InitBoxMoveFrameScroll( BOX2_SYS_WORK * syswk );

extern u32 BOX2MAIN_GetTrayScrollDir( BOX2_SYS_WORK * syswk, u32 now, u32 chg );


//============================================================================================
//	ポケモンデータ
//============================================================================================


//--------------------------------------------------------------------------------------------
/**
 * ポケモンデータクリア
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	tray	トレイ番号
 * @param	pos		位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeDataClear( BOX2_SYS_WORK * syswk, u32 tray, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * ポケモンデータ移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeDataMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 指定位置以外に戦えるポケモンがいるか
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		対象外のポケモン位置
 *
 * @retval	"TRUE = いる"
 * @retval	"FALSE = いない"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_BattlePokeCheck( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * 持ち物変更によるフォルムチェンジ
 *
 * @param	ppp		POKEMON_PASO_PARAM
 *
 * @retval	"TRUE = フォルムチェンジ"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_PokeItemFormChange( POKEMON_PASO_PARAM * ppp );

//--------------------------------------------------------------------------------------------
/**
 * 手持ちからボックスに移動させたときのシェイミのフォルムチェンジ
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	b_pos	移動前の位置
 * @param	a_pos	移動後の位置
 *
 * @return	none
 *
 *	データは入れ替えたあとに呼ぶ
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_FormChangeSheimi( BOX2_SYS_WORK * syswk, u32 b_pos, u32 a_pos );

//--------------------------------------------------------------------------------------------
/**
 * 取得アイテムを指定場所のポケモンにセットできるかチェック
 *
 * @param	syswk		ボックス画面システムワーク
 * @param	get_pos		取得場所
 * @param	put_pos		配置位置
 *
 * @retval	"TRUE = 可"
 * @retval	"FALSE = 不可"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_PokeItemMoveCheck( BOX2_SYS_WORK * syswk, u32 get_pos, u32 put_pos );


//============================================================================================
//	ポケモンを逃がす
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンを逃がす」ワーク作成
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeFreeCreate( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンを逃がす」ワーク解放
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeFreeExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 逃がせるかどうか技チェック
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	"TRUE = チェック中"
 * @return	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_PokeFreeWazaCheck( BOX2_SYS_WORK * syswk );


//============================================================================================
//	データ表示関連
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 選択中のポケモンの情報表示
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_PokeInfoPut( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * 上画面の一部を再描画
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeInfoRewrite( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * 上画面表示オフ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeInfoOff( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 非選択に切り替え
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeSelectOff( BOX2_SYS_WORK * syswk );

//============================================================================================
//	壁紙
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 壁紙セット
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	id		壁紙ＩＤ
 * @param	mv		スクロール方向
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_WallPaperSet( BOX2_SYS_WORK * syswk, u32 id, u32 mv );

//--------------------------------------------------------------------------------------------
/**
 * 壁紙変更
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	id		壁紙番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_WallPaperChange( BOX2_SYS_WORK * syswk, u32 id );

//--------------------------------------------------------------------------------------------
/**
 * 壁紙番号取得
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	num		トレイ番号
 *
 * @return	壁紙番号
 *
 *	デフォルト壁紙からの通し番号にする
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2MAIN_GetWallPaperNumber( BOX2_SYS_WORK * syswk, u32 num );


//============================================================================================
//	ＢＧフレーム
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * ＢＧフレームワーク作成（全体）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_BgFrameWorkMake( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * ＢＧフレームワーク解放
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_BgFrameWorkExit( BOX2_APP_WORK * appwk );



//============================================================================================
//	マーキングフレーム
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * マークキングフレーム表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_MarkingFramePut( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * マーク表示切り替え
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	pos		マーク位置
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_MarkingSwitch( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * マーキングフレームインセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_MarkingFrameInSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * マーキングフレームアウトセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_MarkingFrameOutSet( BGWINFRM_WORK * wk );


//============================================================================================
//	手持ちポケモンフレーム
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム表示
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PartyPokeFramePut( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム表示（右）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PartyPokeFramePutRight( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム非表示
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PartyPokeFrameOff( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム初期位置設定（左）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PartyPokeFrameInitPutLeft( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム初期位置設定（右）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PartyPokeFrameInitPutRight( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレームインセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PartyPokeFrameInSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレームアウトセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PartyPokeFrameOutSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム右移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PartyPokeFrameRightMoveSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム右移動セット（「ポケモンをあずける」用）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_AzukeruPartyPokeFrameRightMoveSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム左移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PartyPokeFrameLeftMoveSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 手持ちポケモンフレーム左移動セット（「ポケモンをあずける」用）
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_AzukeruPartyPokeFrameLeftMoveSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * ＢＧウィンドウフレームに手持ちポケモンフレームグラフィックをセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 * @param	index	ＢＧウィンドウフレームインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PartyPokeFrameLoadArrange2( BGWINFRM_WORK * wk, u32 index );

//--------------------------------------------------------------------------------------------
/**
 * 「てもちポケモンボタン」ボタン配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_ButtonPutTemochi( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「てもちポケモンボタン」ボタンを画面外に配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_ButtonOutPutTemochi( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「てもちポケモンボタン」ボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_TemochiButtonOutSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 「てもちポケモンボタン」ボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_TemochiButtonInSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンいどう」ボタン配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_ButtonPutIdou( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンいどう」ボタンを画面外に配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_ButtonOutPutIdou( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンいどう」ボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_IdouButtonOutSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 「ポケモンいどう」ボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_IdouButtonInSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 「もどる」ボタン配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_ButtonPutModoru( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「やめる」ボタン配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_ButtonPutYameru( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * キャンセルボタンを画面外に配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_RetButtonOutPut( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * キャンセルボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_RetButtonOutSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * キャンセルボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_RetButtonInSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 「とじる」ボタン配置チェック
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = 配置済み"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
//extern BOOL BOX2MAIN_CloseButtonPutCheck( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 「てもちポケモン」「ポケモンいどう」ボタン非表示
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_BoxPartyButtonVanish( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニューボタンを画面内に配置
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PokeMenuOpenPosSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニューボタン非表示
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PokeMenuOff( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニューボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PokeMenuInSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニューボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_PokeMenuOutSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * ポケモンメニュー配置チェック
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = 配置済み"
 * @retval	"FLASE = それ以外"
 */
//--------------------------------------------------------------------------------------------
//extern BOOL BOX2MAIN_PokeMenuPutCheck( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * ボックステーマ変更フレーム画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_BoxThemaMenuInSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * ボックステーマ変更フレーム画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_BoxThemaMenuOutSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」ボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_BoxMoveButtonInSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」ボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_BoxMoveButtonOutSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 「ボックスをきりかえる」ボタン配置チェック
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = 配置済み"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
//extern BOOL BOX2MAIN_BoxMoveButtonCheck( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_YStatusButtonInSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタンを画面内へ配置
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_YStatusButtonPut( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_YStatusButtonOutSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * Ｙステータスボタン配置チェック
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @retval	"TRUE = 配置済み"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
//extern BOOL BOX2MAIN_YStatusButtonCheck( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * トレイ選択フレームを画面内へ配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 *
 *	他画面から復帰時にボックス移動フレームを表示
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_BoxMoveFrmPut( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * トレイ選択フレーム画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_BoxMoveFrmInSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * トレイ選択フレーム画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_BoxMoveFrmOutSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 壁紙変更フレームインセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_WallPaperFrameInSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 壁紙変更フレームアウトセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_WallPaperFrameOutSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * ボックス切り替えメニューボタン画面内への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 *
 * @li	壁紙変更、預けるボックス選択で使用してます
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_BoxMoveMenuInSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * ボックス切り替えメニューボタン画面外への移動セット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 *
 * @li	壁紙変更、預けるボックス選択で使用してます
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_BoxMoveMenuOutSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * ボックステーマ変更フレームを画面内へ配置
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 *
 *	他画面から復帰時にボックステーマ変更関連フレームを表示
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_BoxThemaFrmPut( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 「てもちポケモン」「ポケモンいどう」を非表示にする
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_ArrangeUnderButtonDel( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * サブ画面技フレームインセット
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_SubDispWazaFrmInSet( BOX2_APP_WORK * appwk );

//--------------------------------------------------------------------------------------------
/**
 * サブ画面技フレームアウトセット
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_SubDispWazaFrmOutSet( BOX2_APP_WORK * appwk );

//--------------------------------------------------------------------------------------------
/**
 * サブ画面技フレーム画面内に配置
 *
 * @param	appwk	ボックス画面アプリワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_SubDispWazaFrmInPosSet( BOX2_APP_WORK * appwk );

//--------------------------------------------------------------------------------------------
/**
 * サブ画面技フレーム画面外に配置
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_SubDispWazaFrmOutPosSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * サブ画面持ち物フレームインセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_SubDispItemFrmInSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * サブ画面持ち物フレームアウトセット
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_SubDispItemFrmOutSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * サブ画面持ち物フレームを画面内に配置
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_SubDispItemFrmInPosSet( BGWINFRM_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * サブ画面持ち物フレームを画面外に配置
 *
 * @param	wk		ＢＧウィンドウフレームワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
//extern void BOX2MAIN_SubDispItemFrmOutPosSet( BGWINFRM_WORK * wk );


extern void BOX2MAIN_MainDispMarkingChange( BOX2_SYS_WORK * syswk, u32 mark );

//--------------------------------------------------------------------------------------------
/**
 * 上画面のマーキングを変更
 *
 * @param	syswk	ボックス画面システムワーク
 * @param	maek	マークデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_SubDispMarkingChange( BOX2_SYS_WORK * syswk, u32 mark );


//============================================================================================
//	サブプロセス
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * ステータス画面呼び出し
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	0
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_PokeStatusCall( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * ステータス画面終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	0
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_PokeStatusExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * バッグ画面呼び出し
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	0
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_BagCall( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * バッグ画面終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	0
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_BagExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 名前入力画面呼び出し
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	0
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_NameInCall( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * 名前入力画面終了
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @return	0
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_NameInExit( BOX2_SYS_WORK * syswk );


//============================================================================================
//	VBLANK FUNC
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 取得ポケモン移動（パーティ）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncGetPokeMoveParty( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 取得ポケモン移動（パーティ＆トレイ）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncGetPokeMoveBoxParty( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手持ちを逃がした後のOBJ動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncPartyPokeFreeSort( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : ポケモンを手持ちに加える動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncPartyInPokeMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : トレイをスクロール（左ボタンが押されたとき）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncTrayScrollLeft( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : トレイをスクロール（右ボタンが押されたとき）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncTrayScrollRight( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : ポケモンメニュー動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncPokeMenuMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : メニュー・「てもちポケモン」・「ポケモンいどう」・「もどる」動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
//extern int BOX2MAIN_VFuncPokeFreeMenuMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : マーキングフレーム動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncMarkingFrameMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 「てもちポケモン」「ポケモンいどう」「とじる」「もどる」などのボタン動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
//extern int BOX2MAIN_VFuncUnderButtonMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手持ちポケモンフレーム動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncPartyFrameMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手持ちポケモンフレームを右へ
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
//extern int BOX2MAIN_VFuncPartyFrmRight( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : ボックス選択ウィンドウイン
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncBoxMoveFrmIn( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : ボックス選択ウィンドウアウト
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncBoxMoveFrmOut( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 取得ポケモン移動（ポケモンをあずける）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_GetPartyPokeMoveDrop( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 取得ポケモン移動（ポケモンをつれていく）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_GetTrayPokeMoveDrop( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手カーソル移動
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncCursorMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 戻るを消してメニューを出す
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncMenuMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : メニューを消して戻るを出す
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
//extern int BOX2MAIN_VFuncMenuCloseKey( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : メニューを消して戻るを出す（ボックス整理）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
//extern int BOX2MAIN_VFuncMenuCloseKeyArrange( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手カーソルでポケモン取得
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncPokeMoveGetKey( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : ポケモンを放す
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncPokeMovePutKey( BOX2_SYS_WORK * syswk );
extern int BOX2MAIN_VFuncPokeMovePutKey2( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : アイテム取得（持ち物整理・キー操作）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemArrangeGetKey( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : アイテム取得（持ち物整理・タッチ操作）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemGetTouch( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : メニューを消して戻るを出す（持ち物整理）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemArrangeMenuClose( BOX2_SYS_WORK * syswk );

extern int BOX2MAIN_VFuncItemGetMenuClose( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : アイテムアイコン入れ替え動作
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
//extern int BOX2MAIN_VFuncItemIconChange( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 持ち物整理・トレイ/手持ち取得（タッチ）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemMoveTouch( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 持ち物整理・トレイ/手持ち入れ替え（タッチ）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemIconChgTouch( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手カーソルでアイテム取得
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemArrangeBoxPartyIconGetKey( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : アイテムを放す
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemIconPutKey( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : アイテムを放す・キャンセル
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemIconPutKeyCancel( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : 手持ちポケモンのアイテム取得（持ち物整理・タッチ操作）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemPartyGetTouch( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * VBLANK FUNC : アイテムアイコン入れ替え動作（手持ちポケモン）
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemIconPartyChange( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 *ポケモンアイコンを手で取得した状態にする
 *
 * @param	syswk	ボックス画面システムワーク
 *
 * @retval	"1 = 処理中"
 * @retval	"0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_HandGetPokeSet( BOX2_SYS_WORK * syswk );



extern int BOX2MAIN_VFuncTrayIconScrollUp( BOX2_SYS_WORK * syswk );
extern int BOX2MAIN_VFuncTrayIconScrollDown( BOX2_SYS_WORK * syswk );

extern int BOX2MAIN_VFuncTrayIconScrollUpJump( BOX2_SYS_WORK * syswk );
extern int BOX2MAIN_VFuncTrayIconScrollDownJump( BOX2_SYS_WORK * syswk );

extern int BOX2MAIN_VFuncGetPokeMoveBattleBoxMain( BOX2_SYS_WORK * syswk );
