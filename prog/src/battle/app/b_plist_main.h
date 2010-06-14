//============================================================================================
/**
 * @file	b_plist_main.h
 * @brief	戦闘用ポケモンリスト画面
 * @author	Hiroyuki Nakamura
 * @date	05.02.01
 */
//============================================================================================
#pragma	once

#include "print/wordset.h"
#include "system/palanm.h"
#include "system/cursor_move.h"
#include "system/bgwinfrm.h"
#include "b_app_tool.h"
#include "b_plist.h"
#include "b_plist_comm_def.h"


//============================================================================================
//	定数定義
//============================================================================================

// ページ定義
enum {
	BPLIST_PAGE_SELECT = 0,		// ポケモン選択ページ
	BPLIST_PAGE_POKE_CHG,			// ポケモン入れ替えページ
	BPLIST_PAGE_MAIN,					// ステータスメインページ	
	BPLIST_PAGE_WAZA_SEL,			// ステータス技選択ページ
	BPLIST_PAGE_SKILL,				// ステータス技詳細ページ
	BPLIST_PAGE_PP_RCV,				// PP回復技選択ページ
	BPLIST_PAGE_WAZASET_BS,		// ステータス技忘れ１ページ（戦闘技選択）
	BPLIST_PAGE_WAZASET_BI,		// ステータス技忘れ２ページ（戦闘技詳細）
	BPLIST_PAGE_DEAD,					// 瀕死入れ替え選択ページ
};

// ウィンドウインデックス
enum {
	WIN_COMMENT = 0,	///< コメント用ウィンドウ
	WIN_TALK,			///< 会話用ウィンドウ
	WIN_MAX
};

// ページ１の追加ウィンドウインデックス
enum {
	WIN_P1_POKE1 = 0,	// ポケモン１
	WIN_P1_POKE2,		// ポケモン２
	WIN_P1_POKE3,		// ポケモン３
	WIN_P1_POKE4,		// ポケモン４
	WIN_P1_POKE5,		// ポケモン５
	WIN_P1_POKE6,		// ポケモン６
	WIN_P1_MAX
};

// 入れ替えページの追加ウィンドウインデックス
enum {
	WIN_CHG_NAME = 0,	// 名前
	WIN_CHG_IREKAE,		// 「いれかえる」
	WIN_CHG_STATUS,		// 「つよさをみる」
	WIN_CHG_WAZACHECK,	// 「わざをみる」
	WIN_CHG_MAX
};

// 技選択ページの追加ウィンドウインデックス
enum {
	WIN_STW_NAME = 0,		// 名前
	WIN_STW_SKILL1,			// 技１
	WIN_STW_SKILL2,			// 技２
	WIN_STW_SKILL3,			// 技３
	WIN_STW_SKILL4,			// 技４

	WIN_STW_STATUS,			// 「つよさをみる」

	WIN_STW_MAX
};

// ステータスメインページの追加ウィンドウインデックス
enum {
	WIN_P3_NAME = 0,	// 名前
	WIN_P3_SPANAME,		// 特性名
	WIN_P3_SPAINFO,		// 特性説明
	WIN_P3_ITEMNAME,	// 道具名
	WIN_P3_HPNUM,		// HP値
	WIN_P3_POWNUM,		// 攻撃値
	WIN_P3_DEFNUM,		// 防御値
	WIN_P3_AGINUM,		// 素早さ値
	WIN_P3_SPPNUM,		// 特攻値
	WIN_P3_SPDNUM,		// 特防値
	WIN_P3_HPGAGE,		// HPゲージ
	WIN_P3_LVNUM,		// レベル値
	WIN_P3_NEXTNUM,		// 次のレベル値

	WIN_P3_HP,			// 「HP」
	WIN_P3_POW,			// 「こうげき」
	WIN_P3_DEF,			// 「ぼうぎょ」
	WIN_P3_AGI,			// 「すばやさ」
	WIN_P3_SPP,			// 「とくこう」
	WIN_P3_SPD,			// 「とくぼう」
	WIN_P3_LV,			// 「Lv.」
	WIN_P3_NEXT,		// 「つぎのレベルまで」
	WIN_P3_WAZACHECK,	// 「わざをみる」

	WIN_P3_MAX
};

// ステータス技詳細ページの追加ウィンドウインデックス
enum {
	WIN_P4_SKILL = 0,	// 技名
	WIN_P4_PPNUM,		// PP/PP
	WIN_P4_HITNUM,		// 命中値
	WIN_P4_POWNUM,		// 威力値
	WIN_P4_INFO,		// 技説明
	WIN_P4_BRNAME,		// 分類名

	WIN_P4_NAME,		// 名前
	WIN_P4_PP,			// PP
	WIN_P4_HIT,			// 「めいちゅう」
	WIN_P4_POW,			// 「いりょく」
	WIN_P4_BUNRUI,		// 「ぶんるい」

	WIN_P4_MAX
};

// 技忘れ選択ページの追加ウィンドウインデックス
enum {
	WIN_P5_NAME = 0,	// 名前
	WIN_P5_SKILL1,		// 技１
	WIN_P5_SKILL2,		// 技２
	WIN_P5_SKILL3,		// 技３
	WIN_P5_SKILL4,		// 技４
	WIN_P5_SKILL5,		// 技５
	WIN_P5_MAX
};

// ページ６の追加ウィンドウインデックス
enum {
	WIN_P6_NAME = 0,	// 名前
	WIN_P6_SKILL,		// 技名
	WIN_P6_PP,			// PP
	WIN_P6_PPNUM,		// PP/PP
	WIN_P6_HIT,			// 「めいちゅう」
	WIN_P6_POW,			// 「いりょく」
	WIN_P6_HITNUM,		// 命中値
	WIN_P6_POWNUM,		// 威力値
	WIN_P6_INFO,		// 技説明
	WIN_P6_BUNRUI,		// 「ぶんるい」
	WIN_P6_BRNAME,		// 分類名
	WIN_P6_WASURERU,	// 「わすれる」
	WIN_P6_MAX
};

// 技回復選択ページの追加ウィンドウインデックス
enum {
	WIN_P7_NAME = 0,	// 名前
	WIN_P7_SKILL1,		// 技１
	WIN_P7_SKILL2,		// 技２
	WIN_P7_SKILL3,		// 技３
	WIN_P7_SKILL4,		// 技４
	WIN_P7_MAX
};

// セルアクターのID
enum {
	BPL_CA_ITEM1 = 0,
	BPL_CA_ITEM2,
	BPL_CA_ITEM3,
	BPL_CA_ITEM4,
	BPL_CA_ITEM5,
	BPL_CA_ITEM6,
	BPL_CA_ITEM7,

	BPL_CA_POKE1,
	BPL_CA_POKE2,
	BPL_CA_POKE3,
	BPL_CA_POKE4,
	BPL_CA_POKE5,
	BPL_CA_POKE6,

	BPL_CA_STATUS1,
	BPL_CA_STATUS2,
	BPL_CA_STATUS3,
	BPL_CA_STATUS4,
	BPL_CA_STATUS5,
	BPL_CA_STATUS6,

	BPL_CA_HPGAUGE1,
	BPL_CA_HPGAUGE2,
	BPL_CA_HPGAUGE3,
	BPL_CA_HPGAUGE4,
	BPL_CA_HPGAUGE5,
	BPL_CA_HPGAUGE6,

	BPL_CA_POKETYPE1,
	BPL_CA_POKETYPE2,
	BPL_CA_POKETYPE3,
	BPL_CA_POKETYPE4,

	BPL_CA_WAZATYPE1,
	BPL_CA_WAZATYPE2,
	BPL_CA_WAZATYPE3,
	BPL_CA_WAZATYPE4,
	BPL_CA_WAZATYPE5,
	BPL_CA_WAZATYPE6,
	BPL_CA_WAZATYPE7,
	BPL_CA_WAZATYPE8,
	BPL_CA_WAZATYPE9,
	BPL_CA_WAZATYPE10,

	BPL_CA_BUNRUI,
	BPL_CA_BUNRUI2,

	BPL_CA_MAX
};

// セルアクターのリソース
enum {
	// キャラリソースID
	BPLIST_CHRRES_POKE1 = 0,		// ポケモンアイコン：１匹目
	BPLIST_CHRRES_POKE2,				// ポケモンアイコン：２匹目
	BPLIST_CHRRES_POKE3,				// ポケモンアイコン：３匹目
	BPLIST_CHRRES_POKE4,				// ポケモンアイコン：４匹目
	BPLIST_CHRRES_POKE5,				// ポケモンアイコン：５匹目
	BPLIST_CHRRES_POKE6,				// ポケモンアイコン：６匹目
	BPLIST_CHRRES_STATUS,				// 状態異常アイコン
	BPLIST_CHRRES_ITEM,					// アイテムアイコン
	BPLIST_CHRRES_POKETYPE1,		// ポケモンタイプアイコン１
	BPLIST_CHRRES_POKETYPE2,		// ポケモンタイプアイコン２
	BPLIST_CHRRES_POKETYPE3,		// ポケモンタイプアイコン
	BPLIST_CHRRES_POKETYPE4,		// ポケモンタイプアイコン
	BPLIST_CHRRES_WAZATYPE1,		// 技タイプアイコン１
	BPLIST_CHRRES_WAZATYPE2,		// 技タイプアイコン２
	BPLIST_CHRRES_WAZATYPE3,		// 技タイプアイコン３
	BPLIST_CHRRES_WAZATYPE4,		// 技タイプアイコン４
	BPLIST_CHRRES_WAZATYPE5,		// 技タイプアイコン５
	BPLIST_CHRRES_WAZATYPE6,		// 技タイプアイコン
	BPLIST_CHRRES_WAZATYPE7,		// 技タイプアイコン
	BPLIST_CHRRES_WAZATYPE8,		// 技タイプアイコン
	BPLIST_CHRRES_WAZATYPE9,		// 技タイプアイコン
	BPLIST_CHRRES_WAZATYPE10,		// 技タイプアイコン
	BPLIST_CHRRES_BUNRUI,				// 分類アイコン
	BPLIST_CHRRES_BUNRUI2,			// 分類アイコン
	BPLIST_CHRRES_CURSOR,				// カーソル
	BPLIST_CHRRES_HPGAUGE,			// ＨＰゲージ
	BPLIST_CHRRES_MAX,

	// パレットリソースID
	BPLIST_PALRES_POKE = 0,			// ポケモンアイコン（３本）
	BPLIST_PALRES_STATUS,				// 状態異常アイコン（１本）
	BPLIST_PALRES_ITEM,					// アイテムアイコン（１本）
	BPLIST_PALRES_TYPE,					// タイプ/分類アイコン（３本）
	BPLIST_PALRES_CURSOR,				// カーソル（１本）
	BPLIST_PALRES_HPGAUGE,			// ＨＰゲージ（１本）
	BPLIST_PALRES_MAX,

	// セルリソースID
	BPLIST_CELRES_POKE = 0,			// ポケモンアイコン
	BPLIST_CELRES_STATUS,				// 状態異常アイコン
	BPLIST_CELRES_ITEM,					// アイテムアイコン
	BPLIST_CELRES_TYPE,					// タイプ/分類アイコン
	BPLIST_CELRES_CURSOR,				// カーソル
	BPLIST_CELRES_HPGAUGE,			// ＨＰゲージ
	BPLIST_CELRES_MAX,
};

// 技データ
typedef struct {
	u16	id;		// 技番号
	u8	pp;		// PP
	u8	mpp;	// 最大PP

	u8	type;	// タイプ
	u8	kind;	// 分類
	u8	hit;	// 命中
	u8	pow;	// 威力
}BPL_POKEWAZA;

// ポケモンデータ
typedef struct {
	POKEMON_PARAM * pp;		// ポケモンデータ

	u16	mons;		// ポケモン番号
	u16	pow;		// 攻撃

	u16	def;		// 防御
	u16	agi;		// 素早さ

	u16	spp;		// 特攻
	u16	spd;		// 特防

	u16	hp;			// HP
	u16	mhp;		// 最大HP

	u8	type1;			// タイプ１
	u8	type2;			// タイプ２
	u8	lv:7;				// レベル
	u8	sex_put:1;	// 性別表示（ニドラン用）0=ON, 1=OFF
	u8	sex:3;			// 性別
	u8	st:4;				// 状態異常
	u8	egg:1;			// タマゴ

	u16	spa;		// 特性
	u16	item;		// アイテム

	u32	now_exp;			// 現在の経験値
	u32	now_lv_exp;		// 現在のレベルの経験値
	u32	next_lv_exp;	// 次のレベルの経験値

	u32	form;			// フォルムNo

	BPL_POKEWAZA	waza[4];	// 技データ

}BPL_POKEDATA;

#define	BPL_BANM_MAX_N	( 3 )		// ボタンアニメパターン数（通常）
#define	BPL_BANM_MAX_E	( 4 )		// ボタンアニメパターン数（押せないボタンあり）
// 命令ボタンサイズ
#define	BPL_BSX_COMMAND	( 13 )
#define	BPL_BSY_COMMAND	( 5 )
// 切り替えボタンサイズ
#define	BPL_BSX_UPDOWN	( 5 )
#define	BPL_BSY_UPDOWN	( 5 )
// 戻るボタンサイズ
#define	BPL_BSX_RET		( 5 )
#define	BPL_BSY_RET		( 5 )
// 入れ替えボタン
#define	BPL_BSX_CHG		( 30 )
#define	BPL_BSY_CHG		( 17 )
// 技ボタンサイズ
#define	BPL_BSX_WAZA	( 16 )
#define	BPL_BSY_WAZA	( 6 )
// 忘れるボタンサイズ
#define	BPL_BSX_DEL		( 26 )
#define	BPL_BSY_DEL		( 5 )
// コンテスト切り替えボタンサイズ
#define	BPL_BSX_CONTEST	( 9 )
#define	BPL_BSY_CONTEST	( 4 )
// 技位置ボタン
#define	BPL_BSX_WP		( 5 )
#define	BPL_BSY_WP		( 2 )

// 入れ替えログデータ
typedef struct {
	u16	pos1;
	u16	pos2;
}BPLIST_CHGLOG;

typedef struct {
	BPLIST_DATA * dat;	// 外部参照データ

	BPL_POKEDATA	poke[TEMOTI_POKEMAX];	// ポケモンデータ
	u8	listRow[TEMOTI_POKEMAX];				// リストの並び

	GFL_TCBLSYS * tcbl;		// TCBL

	PALETTE_FADE_PTR pfd;	// パレットフェードデータ

	u16	btn_plate1[BPL_BANM_MAX_E][BPL_COMM_BSX_PLATE*BPL_COMM_BSY_PLATE];		// 戦闘中のプレートボタン
	u16	btn_plate2[BPL_BANM_MAX_E][BPL_COMM_BSX_PLATE*BPL_COMM_BSY_PLATE];		// 控えのプレートボタン
	u16	btn_command[BPL_BANM_MAX_E][BPL_BSX_COMMAND*BPL_BSY_COMMAND];	// 命令ボタン
	u16	btn_up[BPL_BANM_MAX_E][BPL_BSX_UPDOWN*BPL_BSY_UPDOWN];			// 切り替えボタン（上）
	u16	btn_down[BPL_BANM_MAX_E][BPL_BSX_UPDOWN*BPL_BSY_UPDOWN];		// 切り替えボタン（下）
	u16	btn_ret[BPL_BANM_MAX_E][BPL_BSX_RET*BPL_BSY_RET];				// 戻るボタン
	u16	btn_chg[BPL_BANM_MAX_N][BPL_BSX_CHG*BPL_BSY_CHG];				// 入れ替えボタン
	u16	btn_waza[BPL_BANM_MAX_E][BPL_BSX_WAZA*BPL_BSY_WAZA];			// 技ボタン
	u16	btn_del[BPL_BANM_MAX_N][BPL_BSX_DEL*BPL_BSY_DEL];				// 忘れるボタン
	u16	btn_wp[BPL_BANM_MAX_N][BPL_BSX_WP*BPL_BSY_WP];					// 技位置ボタン

	u16	wb_pal[16];			// 技ボタンのパレット

	u8	btn_seq;
	u8	btn_cnt;
	u8	btn_id;
	u8	btn_mode:4;
	u8	btn_pat:3;
	u8	btn_flg:1;

	GFL_FONT * nfnt;				// 8x8フォント
	GFL_MSGDATA * mman;			// メッセージデータマネージャ
	WORDSET * wset;					// 単語セット
	STRBUF * msg_buf;				// メッセージ用バッファ
	PRINT_QUE * que;				// プリントキュー
	PRINT_STREAM * stream;	// プリントストリーム
	BOOL	stream_clear_flg;

	GFL_CLUNIT * clunit;
	GFL_CLWK * clwk[BPL_CA_MAX];

	PRINT_UTIL	win[WIN_MAX];	// BMPウィンドウデータ（通常）
	PRINT_UTIL	add_win[64];	// BMPウィンドウデータ（追加）
	GFL_BMPWIN * dmy_win;			// BMPウィンドウデータ（ダミー）
	u8	bmp_add_max;					// 追加したBMPの数

	u8	bmp_swap:5;							// スワップ描画フラグ
	u8	poke_type_swap:1;				// ポケモンタイプアイコンスワップ描画フラグ
	u8	waza_type_swap:1;				// 技タイプアイコンスワップ描画フラグ
	u8	waza_kind_swap:1;				// 分類アイコンスワップ描画フラグ

	u8	init_poke;		// 入れ替え対象のポケモン位置

	u8	page_chg_seq:4;
	u8	page_chg_comm:4;

	int	seq;			// メインシーケンス
	int	ret_seq;		// 復帰シーケンス
	u8	page;			// 現在のページ
	u8	midx;			// メッセージインデックス

	CURSORMOVE_WORK * cmwk;					// カーソル制御ワーク
	BAPP_CURSOR_PUT_WORK * cpwk;		// カーソル表示
	BOOL cursor_flg;
	u8	chg_page_cp;			// ポケモン入れ替えページのカーソル位置
	u8	wws_page_cp;			// ステータス技忘れ技選択ページのカーソル位置
	u8	wwm_page_cp;			// ステータス技忘れ技詳細ページのカーソル位置

	u8	talk_win_clear;		// メッセージ表示後にメッセージウィンドウをクリア

	// 瀕死入れ替え
	BGWINFRM_WORK * chg_wfrm;
	u16	chg_scrn1[2][BPL_COMM_BSX_PLATE*BPL_COMM_BSY_PLATE];
	u16	chg_scrn2[2][BPL_COMM_BSX_PLATE*BPL_COMM_BSY_PLATE];
	u8	chg_poke_sel;						// 瀕死入れ替え時の位置保持用
	u8	chg_pos1;
	u8	chg_pos2;
	BPLIST_CHGLOG	chg_log[BPL_SELNUM_MAX-1];

	u32	chrRes[BPLIST_CHRRES_MAX];
	u32	palRes[BPLIST_PALRES_MAX];
	u32	celRes[BPLIST_CELRES_MAX];

	const u8 * putWin;

	BOOL	initFlag;		// 初期化したか（初期化前に外部から終了要求されると止まるので）

}BPLIST_WORK;

typedef int (*pBPlistFunc)(BPLIST_WORK*);


// 入れ替えエラーチェック
enum {
	BPL_IREKAE_ERR_NONE = 0,		// エラーなし
	BPL_IREKAE_ERR_PARTNER,			// パートナーのポケモン
	BPL_IREKAE_ERR_DEAD,				// 瀕死
	BPL_IREKAE_ERR_BATTLE,			// 出ている
	BPL_IREKAE_ERR_SELECT,			// 選択済み
	BPL_IREKAE_ERR_EGG,					// タマゴ
	BPL_IREKAE_ERR_WAZA,				// 技・特性
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

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
extern u8 BattlePokeList_PokeSetCheck( BPLIST_WORK * wk, s32 pos );

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
extern BOOL BattlePokeList_MultiCheck( BPLIST_WORK * wk );

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
extern u8 BattlePokeList_MultiPosCheck( BPLIST_WORK * wk, u8 pos );

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
extern u8 BPLISTMAIN_GetListRow( BPLIST_WORK * wk, u32 pos );

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
extern BOOL BPLISTMAIN_GetNewLog( BPLIST_WORK * wk, u8 * pos1, u8 * pos2, BOOL del );

//--------------------------------------------------------------------------------------------
/**
 * @brief		入れ替えエラーチェック
 *
 * @param		wk    ワーク
 *
 * @return	エラーコード
 */
//--------------------------------------------------------------------------------------------
extern u8 BPLISTMAIN_CheckIrekaeError( BPLIST_WORK * wk );
