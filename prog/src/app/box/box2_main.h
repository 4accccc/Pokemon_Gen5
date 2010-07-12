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

#include "bugfix.h"


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

#define	BOX2OBJ_PI_OUTLINE_MAX		( 8 )		// アウトラインで使用するＯＢＪの数

#define	BOX2OBJ_TRAYICON_MAX			( 6 )		// トレイアイコンの数

// ポケモンアイコンのキャラサイズ
#define	BOX2OBJ_POKEICON_CGX_SIZE	( 0x20 * 4 * 4 )

#define	BOX2MAIN_GETPOS_NONE		( 0xff )	// ポケモン未取得

#define	BOX2MAIN_PPP_GET_MINE		( 0xff )	// BOX2MAIN_PPPGet()で手持ちを取得するときのトレイ定義

#define	BOX2MAIN_TRAY_SCROLL_SPD		( 8 )		// トレイスクロール速度
#define	BOX2MAIN_TRAY_SCROLL_CNT		( 23 )	// トレイスクロールカウント


// キャラリソースID
enum {
	BOX2MAIN_CHRRES_POKEICON = 0,	// ポケモンアイコン
	BOX2MAIN_CHRRES_POKEGRA = BOX2MAIN_CHRRES_POKEICON + BOX2OBJ_POKEICON_MAX,
	BOX2MAIN_CHRRES_POKEGRA2,
	BOX2MAIN_CHRRES_ITEMICON,
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

	BOX2OBJ_ID_CONV_BUTTON,	// 便利モードボタン

	BOX2OBJ_ID_ITEMICON,

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
	BOX2BMPWIN_ID_TITLE = 0,	// タイトル
	BOX2BMPWIN_ID_NAME,				// ポケモン名
	BOX2BMPWIN_ID_NICKNAME,		// ニックネーム
	BOX2BMPWIN_ID_LV,					// レベル
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
}BOX2_IRQWK;

// ＯＡＭフォントワーク
typedef struct {
	BMPOAM_ACT_PTR oam;
	GFL_BMP_DATA * bmp;
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

	int	key_repeat_speed;			// キーリピート速度保存
	int	key_repeat_wait;			// キーリピートウェイト保存

	GFL_TCB * vtask;					// TCB ( VBLANK )

	BOX2_IRQWK	vfunk;				// VBLANK関数ワーク
	int	vnext_seq;						// VBLANK関数終了後のシーケンス

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
	u8	winTrans[BOX2BMPWIN_ID_MAX/8+1];	// BMPWINキャラ転送フラグ（割り切れない場合があるので＋１）

	GFL_ARCUTIL_TRANSINFO	syswinInfo;			// システムウィンドウ展開位置

	BOOL	zenkokuFlag;			// 全国図鑑フラグ
	u16 * localNo;					// 地方図鑑番号リスト

	// はい・いいえ関連
	APP_TASKMENU_ITEMWORK	ynList[2];
	APP_TASKMENU_RES * ynRes;
	APP_TASKMENU_WORK * ynWork;
	u16	ynID;												// はい・いいえＩＤ

	BUTTON_ANM_WORK	bawk;		// ボタンアニメワーク

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

	u16	pokegra_swap;				// 上画面ポケモングラフィック切替フラグ

	u32	old_cur_pos;				// 前回のカーソル位置（外部で必要なため）

	u16	get_item;						// 取得アイテム
	u16	get_item_init_pos;	// アイテム取得位置

	u8	menu_rcv_blend;			// メニューに戻るときに半透明の設定を戻す

	u8	mode_chg_flg;				// ドラッグ中のモード切替有効無効フラグ 0 = 有効, 1 = 無効

	u8	msg_put;						// メッセージを表示するか
	u8	mv_err_code;				// ポケモン移動エラーコード

	u32	pokeMark;						// マーキング変更ワーク

	// ボックス切り替え関連
	s8	wall_px;
	u8	wall_area;

	u8	jump_tray;			//「ジャンプする」でトレイを切り替える場合 = 1

	u8	wallpaper_pos;	// 壁紙選択位置
	u8	wp_menu;				// 壁紙メニュー番号
	u8	wp_special;			// 追加壁紙フラグ

	u8	poke_put_key;		// キーでの配置位置
	u8	chg_tray_pos;		// 切り替えるトレイ

	u32	tpx;						// タッチＸ座標
	u32	tpy;						// タッチＹ座標

	int	wipe_seq;				// ワイプ後のシーケンス
	int	wait;						// ウェイト

	void * seqwk;				// シーケンスワーク
	int	sub_seq;				// シーケンス内シーケンス

	int	que_wait_seq;		// 初期化時のキュー待ち後のシーケンス

}BOX2_APP_WORK;

// ボックス画面システムワーク
typedef struct {

	BOX2_GFL_PROC_PARAM * dat;					// 外部データ

	GFL_PROCSYS * localProc;						// ローカルＰＲＯＣ
	GFL_PROC_MAIN_STATUS	procStatus;		// ローカルＰＲＯＣの戻り値

	void * subProcWork;				// サブプロセスで使用するワーク
	u16	subRet;								// サブプロセスの戻り値などを保存する場所
	u8	subProcType;					// サブプロセスの種類
	u8	subProcMode;					// サブプロセスの呼び出しモード

	u8	tray;									// 現在のトレイ
	u8	trayMax;							// 開放されているトレイ数

	u8	get_pos;							// 取得位置
	u8	get_tray;							// 取得トレイ
	u8	poke_get_key;					// 掴んでいるか

	u8	box_mv_pos;						// ボックス移動選択位置

	u8	move_mode;				// ポケモン移動モード
	u8	get_start_mode;		// アイコンを掴んだときのモード 0 = トレイから, 1 = 手持ちから

	u8	mv_cnv_mode:4;		// 便利モードフラグ 1 = yes, 0 = no
	u8	mv_cnv_end:4;			// 掴む終了フラグ

	u8	tb_status_pos;		// 掴んでいるときのステータス画面からの戻り位置

	u8	pokechg_mode;			// 入れ替えモード

	u8	tb_ret_btn;				// タッチバーに表示している戻るボタンの表示状況
	u8	tb_exit_btn;			// タッチバーに表示している終了ボタンの表示状況
	u8	tb_status_btn;		// タッチバーに表示しているステータスボタンの表示状況

	u32	cur_rcv_pos;			// カーソル表示復帰位置

	int	next_seq;					// 次のシーケンス

	BOX2_APP_WORK * app;	// アプリワーク

}BOX2_SYS_WORK;

typedef int (*pBOX2_FUNC)(BOX2_SYS_WORK*);

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


#define	BOX2MAIN_TRAYICON_SCROLL_CNT		( 5 )		// トレイアイコンスクロールカウント


// ポケモン移動エラーコード
enum {
	BOX2MAIN_ERR_CODE_NONE = 0,		// エラーなし
	BOX2MAIN_ERR_CODE_BOXMAX,			// ボックスがいっぱい
	BOX2MAIN_ERR_CODE_MAIL,				// メールを持っている
	BOX2MAIN_ERR_CODE_BATTLE,			// 戦えるポケモンがいなくなる
	BOX2MAIN_ERR_CODE_EGG,				// タマゴはえらべない
	BOX2MAIN_ERR_CODE_ROCK,				// ロックされている（バトルボックス専用）
	BOX2MAIN_ERR_CODE_SLEEP,			// 眠くない（寝かせる専用）
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief   VBLANK関数設定
 *
 * @param   syswk   ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_InitVBlank( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief   VBLANK関数削除
 *
 * @param   syswk   ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_ExitVBlank( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK内で呼ばれる関数を設定
 *
 * @param		appwk		ボックス画面アプリワーク
 * @param		func		関数ポインタ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_VFuncSet( BOX2_APP_WORK * appwk, void * func );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK内で呼ばれる関数を予約
 *
 * @param		appwk		ボックス画面アプリワーク
 * @param		func		関数ポインタ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_VFuncReq( BOX2_APP_WORK * appwk, void * func );

//--------------------------------------------------------------------------------------------
/**
 * @brief		予約したVBLANK関数をセット
 *
 * @param		appwk		ボックス画面アプリワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_VFuncReqSet( BOX2_APP_WORK * appwk );

//--------------------------------------------------------------------------------------------
/**
 * @brief   VRAM設定
 *
 * @param   none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_VramBankSet(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief   VRAM設定取得
 *
 * @param   none
 *
 * @return  設定データ
 */
//--------------------------------------------------------------------------------------------
extern const GFL_DISP_VRAM * BOX2MAIN_GetVramBankData(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief   ＢＧ初期化
 *
 * @param   syswk   ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_BgInit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief   ＢＧ解放
 *
 * @param   syswk   ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_BgExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief   ＢＧグラフィック読み込み
 *
 * @param   syswk   ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_BgGraphicLoad( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief   パレットフェード初期化
 *
 * @param   syswk   ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PaletteFadeInit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief   パレットフェード解放
 *
 * @param   syswk   ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PaletteFadeExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief  アルファブレンド設定
 *
 * @param   flg   TRUE = 有効, FALSE = 無効
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_SetBlendAlpha( BOOL flg );

//--------------------------------------------------------------------------------------------
/**
 * @brief   メッセージ関連初期化
 *
 * @param   syswk   ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_MsgInit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief   メッセージ関連解放
 *
 * @param   syswk   ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_MsgExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえ処理初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_YesNoWinInit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえ処理解放
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_YesNoWinExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		はい・いいえセット
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			カーソル位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_YesNoWinSet( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボタンアニメ処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "TRUE = アニメ中"
 * @retval  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_ButtonAnmMain( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief   ＢＧボタンアニメセット
 *
 * @param   syswk     ボックス画面システムワーク
 * @param   wfrmID    ＢＧウィンドウフレームＩＤ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_InitBgButtonAnm( BOX2_SYS_WORK * syswk, u32 wfrmID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		タッチバーをリセット
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_ResetTouchBar( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックススクロール初期化
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		mvID		スクロールＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_InitTrayScroll( BOX2_SYS_WORK * syswk, u32 mvID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイアイコンスクロール初期化
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		mvID		スクロール方向
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_InitTrayIconScroll( BOX2_SYS_WORK * syswk, s32 mv );

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイ選択フレームスクロール初期化
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_InitBoxMoveFrameScroll( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ボックスクロール方向取得
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		now			現在のトレイ
 * @param		chg			移動するトレイ
 *
 * @return	スクロール方向
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2MAIN_GetTrayScrollDir( BOX2_SYS_WORK * syswk, u32 now, u32 chg );

//--------------------------------------------------------------------------------------------
/**
 * @brief   専用ボタンを押したときのカーソル表示切り替え
 *
 * @param   syswk   ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_ChgCursorButton( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief   ポケモンデータ取得
 *
 * @param   syswk   ボックス画面システムワーク
 * @param   pos     位置
 * @param   tray    トレイ
 * @param   prm     取得パラメータ
 * @param   buf     取得バッファ
 *
 * @return  取得データ
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2MAIN_PokeParaGet( BOX2_SYS_WORK * syswk, u16 pos, u16 tray, int prm, void * buf );

//--------------------------------------------------------------------------------------------
/**
 * @brief   ポケモンデータ設定
 *
 * @param   syswk   ボックス画面システムワーク
 * @param   pos     位置
 * @param   prm     設定パラメータ
 * @param   buf     設定データ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeParaPut( BOX2_SYS_WORK * syswk, u32 pos, u32 tray, int prm, u32 buf );

//--------------------------------------------------------------------------------------------
/**
 * @brief   POKEMON_PASO_PARAM取得
 *
 * @param   syswk   ボックス画面システムワーク
 * @param   tray    トレイ番号
 * @param   pos     位置
 *
 * @return  POKEMON_PASO_PARAM
 */
//--------------------------------------------------------------------------------------------
extern POKEMON_PASO_PARAM * BOX2MAIN_PPPGet( BOX2_SYS_WORK * syswk, u32 tray, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンデータクリア
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		tray		トレイ番号
 * @param		pos			位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeDataClear( BOX2_SYS_WORK * syswk, u32 tray, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンデータ移動
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeDataMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		指定位置以外に戦えるポケモンがいるか
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			対象外のポケモン位置
 *
 * @retval  "TRUE = いる"
 * @retval  "FALSE = いない"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_BattlePokeCheck( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		持ち物変更によるフォルムチェンジ
 *
 * @param		ppp   POKEMON_PASO_PARAM
 *
 * @retval  "TRUE = フォルムチェンジ"
 * @retval  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_PokeItemFormChange( BOX2_SYS_WORK * syswk, POKEMON_PASO_PARAM * ppp );

//--------------------------------------------------------------------------------------------
/**
 * @brief		手持ちからボックスに移動させたときのシェイミのフォルムチェンジ
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		b_pos		移動前の位置
 * @param		a_pos		移動後の位置
 * @param		tray		トレイ番号
 *
 * @return  none
 *
 *  データは入れ替えたあとに呼ぶ
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_FormChangeSheimi( BOX2_SYS_WORK * syswk, u32 b_pos, u32 a_pos, u32 tray );

//--------------------------------------------------------------------------------------------
/**
 * @brief		フォルムチェンジ後にPOKEMON_PARAMを再設定
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_FormChangeRenew( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief   地方図鑑番号テーブル読み込み
 *
 * @param   syswk   ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_LoadLocalNoList( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief   地方図鑑番号テーブル解放
 *
 * @param   syswk   ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_FreeLocalNoList( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		指定位置のポケモンが寝かせられるか
 *
 * @param   syswk   ボックス画面システムワーク
 * @param		pos			位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_CheckSleep( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		取得アイテムを指定場所のポケモンにセットできるかチェック
 *
 * @param		syswk			ボックス画面システムワーク
 * @param		get_pos		取得場所
 * @param		put_pos		配置位置
 *
 * @retval  "TRUE = 可"
 * @retval  "FALSE = 不可"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_PokeItemMoveCheck( BOX2_SYS_WORK * syswk, u32 get_pos, u32 put_pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンを逃がす」ワーク作成
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeFreeCreate( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		「ポケモンを逃がす」ワーク解放
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeFreeExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		逃がせるかどうか技チェック
 *
 * @param		syswk ボックス画面システムワーク
 *
 * @return  "TRUE = チェック中"
 * @return  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_PokeFreeWazaCheck( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		終了時に手持ちにペラップがいない場合はぺタップボイスの存在フラグを落とす
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_CheckPartyPerapu( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		トレイアイコン表示位置のボックス番号を取得
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		mv			デフォルト位置からの差分
 *
 * @retval  "TRUE = 移動中"
 * @retval  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern u8 BOX2MAIN_GetBoxMoveTrayNum( BOX2_SYS_WORK * syswk, s8 mv );

//--------------------------------------------------------------------------------------------
/**
 * @brief   壁紙セット
 *
 * @param   syswk   ボックス画面システムワーク
 * @param   id      壁紙ＩＤ
 * @param   mv      スクロール方向
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_WallPaperSet( BOX2_SYS_WORK * syswk, u32 id, u32 mv );

//--------------------------------------------------------------------------------------------
/**
 * @brief		壁紙変更
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		id			壁紙番号
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_WallPaperChange( BOX2_SYS_WORK * syswk, u32 id );

//--------------------------------------------------------------------------------------------
/**
 * @brief   壁紙番号取得
 *
 * @param   syswk   ボックス画面システムワーク
 * @param   num     トレイ番号
 *
 * @return  壁紙番号
 *
 *  デフォルト壁紙からの通し番号にする
 */
//--------------------------------------------------------------------------------------------
extern u32 BOX2MAIN_GetWallPaperNumber( BOX2_SYS_WORK * syswk, u32 num );

//--------------------------------------------------------------------------------------------
/**
 * @brief		指定位置のポケモン情報表示（トレイ指定）
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		tray		トレイ番号
 * @param		pos			位置
 *
 * @retval  "TRUE = 表示"
 * @retval  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_TrayPokeInfoPut( BOX2_SYS_WORK * syswk, u32 tray, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		指定位置のポケモン情報表示（現在のトレイ）
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 *
 * @retval  "TRUE = 表示"
 * @retval  "FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL BOX2MAIN_PokeInfoPut( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		上画面の一部を再描画
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		pos			位置
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeInfoRewrite( BOX2_SYS_WORK * syswk, u32 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		上画面表示オフ
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeInfoOff( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		非選択に切り替え
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_PokeSelectOff( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		マーキングフレームのマークを変更
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		mark		ON/OFFビットテーブル
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_MainDispMarkingChange( BOX2_SYS_WORK * syswk, u32 mark );

//--------------------------------------------------------------------------------------------
/**
 * @brief		上画面のマーキングを変更
 *
 * @param		syswk		ボックス画面システムワーク
 * @param		mark		ON/OFFビットテーブル
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_SubDispMarkingChange( BOX2_SYS_WORK * syswk, u32 mark );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ステータス画面呼び出し
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  0
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_PokeStatusCall( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ステータス画面終了
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  0
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_PokeStatusExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		バッグ画面呼び出し
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  0
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_BagCall( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		バッグ画面終了
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  0
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_BagExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		名前入力画面呼び出し
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  0
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_NameInCall( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		名前入力画面終了
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @return  0
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_NameInExit( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 取得ポケモン移動（パーティ）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncGetPokeMoveParty( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 取得ポケモン移動（パーティ＆トレイ）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncGetPokeMoveBoxParty( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 手持ちを逃がした後のOBJ動作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncPartyPokeFreeSort( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : ポケモンを手持ちに加える動作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncPartyInPokeMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : トレイをスクロール（左ボタンが押されたとき）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncTrayScrollLeft( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : トレイをスクロール（右ボタンが押されたとき）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncTrayScrollRight( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : ポケモンメニュー動作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncPokeMenuMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : マーキングフレーム動作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncMarkingFrameMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 手持ちポケモンフレーム動作
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncPartyFrameMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : ボックス選択ウィンドウイン
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncBoxMoveFrmIn( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : ボックス選択ウィンドウアウト
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncBoxMoveFrmOut( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 取得ポケモン移動（ポケモンをあずける）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_GetPartyPokeMoveDrop( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 取得ポケモン移動（ポケモンをつれていく）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_GetTrayPokeMoveDrop( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 手カーソル移動
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncCursorMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 戻るを消してメニューを出す
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncMenuMove( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンアイコンを手で取得した状態にする
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern void BOX2MAIN_HandGetPokeSet( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 手カーソルでポケモン取得
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncPokeMoveGetKey( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : ポケモンを放す
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncPokeMovePutKey( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : ポケモンを放す（預ける・キー配置時）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncPokeMovePutKey2( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : アイテム取得（持ち物整理・キー操作）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemArrangeGetKey( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : アイテム取得（持ち物整理・タッチ操作）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemGetTouch( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : メニューを消して戻るを出す（持ち物整理）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemArrangeMenuClose( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 持ち物整理・アイテムメニューを閉じる
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemGetMenuClose( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 持ち物整理・バッグで持たせた場合のアイテムアイコン非表示処理
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemArrangeIconClose( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 持ち物整理・トレイ/手持ち取得（タッチ）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemMoveTouch( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 持ち物整理・トレイ/手持ち入れ替え（タッチ）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemIconChgTouch( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 手カーソルでアイテム取得
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemArrangeBoxPartyIconGetKey( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : アイテムを放す
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemIconPutKey( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : アイテムを放す・キャンセル
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemIconPutKeyCancel( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 手持ちポケモンのアイテム取得（持ち物整理・タッチ操作）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemPartyGetTouch( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : アイテムアイコン入れ替え動作（手持ちポケモン）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncItemIconPartyChange( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : トレイアイコンスクロール（上）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//-------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncTrayIconScrollUp( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : トレイアイコンスクロール（下）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//-------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncTrayIconScrollDown( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : トレイアイコンスクロール（上ジャンプ）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//-------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncTrayIconScrollUpJump( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : トレイアイコンスクロール（下ジャンプ）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//-------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncTrayIconScrollDownJump( BOX2_SYS_WORK * syswk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK FUNC : 取得ポケモン移動（バトルボックス・メインから）
 *
 * @param		syswk		ボックス画面システムワーク
 *
 * @retval  "1 = 処理中"
 * @retval  "0 = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern int BOX2MAIN_VFuncGetPokeMoveBattleBoxMain( BOX2_SYS_WORK * syswk );
