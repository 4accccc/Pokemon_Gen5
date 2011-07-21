//=============================================================================
/**
 *
 *	@file		win_record.c
 *	@brief  エントリーチェッカー(優勝記録)
 *	@author
 *	@date
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"

#include "print/wordset.h"

//描画設定
#include "win_record_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"
// print
#include "ui/print_tool.h"

//アプリ共通素材
#include "app/app_menu_common.h"

#include "system/tr2dgra.h"

// サウンド
#include "sound/pm_sndsys.h"
#include "win_record_snd_def.h"

//アーカイブ
#include "arc_def.h"
#include "win_record_gra.naix"

#include "system/wipe.h"

#include "ui/frame_list.h"

#include "app/win_record.h"

#include "system/scroll_bar.h"

// メッセージ
#include "message.naix"
#include "msg/msg_wbt_wincount.h"

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
enum
{
	UI_TEMPLATE_HEAP_SIZE = 0x30000,  ///< ヒープサイズ
};

// 段階
enum{
	STEP_INIT	= 0,					// 初期化
	STEP_LV1 	= 100,
	STEP_LV2 	= 200,
	STEP_LV3 	= 300,
	STEP_LV4 	= 400,
	STEP_LV5 	= 500,
	STEP_LV6 	= 600,
	//
	STEP_END	= 10000,
};

// モード
enum{
	MODE_LV1 = 0,							// シナリオクリア前
	MODE_LV2,								// シナリオクリア後
	MODE_LV3,								// チャンピオンバトル出現
};

// パレット
// 上：BG  (0-2:BG)
// 上：OBJ (0:キャラ、14:通信アイコン)
//
// 下：BG  (0,1:BG、2:パネル、8:FONT)
// 下：OBJ (0,1:obj、11-14:cmn)
enum{
	// 何本目か(bg)
	PLTID_BG_FONT				= 8,		// font
	PLTID_BG_BASE				= 4,		// 通常bg
	//
	//
	// (obj)
	PLTID_OBJ_CMN				= 11,		// cmnのpalのoffset
	PLTID_OBJ_CMN_NUM			= 4,		// cmnのpalの本数
	//
	// 何番目か
	MES_CLEAR_COLOR				= 0,		// クリアカラー
};

// 最大値
enum{
	WIN_RECORD_CANVAS_MAXNUM 	= 1,		// canvas
	WIN_RECORD_BMPOAM_MAXNUM	= 8,		// bmpoam
	WIN_RECORD_EMPTY_MAXNUM		= 15*2,		// empty
	//
	WIN_RECORD_CMN_OBJ_MAXNUM	= 4,		// common(obj)
	WIN_RECORD_OBJ_MAXNUM		= 2,		// obj
	//
	WIN_RECORD_BGNUM_MAXNUM		= 8,		// bg
	WIN_RECORD_MES_MAXNUM		= 2,		// msg
	//
	WIN_RECORD_UP_OBJ_MAXNUM	= 1,		// obj(up)
	//
	WIN_RECORD_MAX_COUNT		= 9999,		// 最大数
	//
	DISP_PANEL_MAXNUM			= 4,		// 1画面のpanel最大表示数
	WIN_RECORD_DISP_PANEL_MAX	= 6,		// 1画面内の表示数
};


// OBJのリソースのインデックス
enum{
	// common
	RESID_CMN_OBJ_PLT = 0,					// パレット
	RESID_CMN_OBJ_CHR,						// キャラ
	RESID_CMN_OBJ_CEL,						// セルアニメ
	// 通常
	RESID_OBJ_PLT,							// パレット
	RESID_OBJ_CHR,							// キャラ
	RESID_OBJ_CEL,							// セルアニメ
	// 通常(up)
	RESID_UP_OBJ_PLT,						// パレット
	RESID_UP_OBJ_CHR,						// キャラ
	RESID_UP_OBJ_CEL,						// セルアニメ
	//
	RESID_OBJ_MAX,
};

// CLWK(下画面)
enum
{
	CLWKID_BAR = 0,							// バー
	CLWKID_BAR2,							// バー下地
	//
	CLWKID_MAX,
};

// clwork
enum{
	CLWKMODE_NORMAL = 0,					// 通常(down)
	CLWKMODE_CMN,							// 共通(down)
	//
	CLWKMODE_UP,							// トレーナー(up)
};

// タッチペンエリア
// (excelのテーブルと対応)
enum{
	AREA_TP_LEFT = 0,						// 左矢印
	AREA_TP_RIGHT,							// 右矢印
	AREA_TP_CROSS,							// ×
	AREA_TP_RETURN,							// RETURN
	//
	AREA_TP_SCROLL,							// スクロール領域
	//
	AREA_TP_MAX,
};

// トーナメントの番号
enum{
	TMT_HODOMOE = 0,						// ホドモエ
	TMT_ISSYU,								// イッシュ
	TMT_KANTO,								// カントー
	TMT_JOUTO,								// ジョウト
	TMT_HOUEN,								// ホウエン
	TMT_SINOU,								// シンオウ
	TMT_WORLD,								// ワールド
	TMT_CHAMPION,							// チャンピオン
	TMT_WCS,								// WCS2011
	TMT_RENTAL,								// レンタル
	//
	TMT_RENTAL_MASTER,						// レンタルマスター
	TMT_SHAFFLE,							// シャッフル
	TMT_SHAFFLE_MASTER,						// シャッフルマスター
	TMT_NORMAL,								// ノーマル
	TMT_FIRE,								// ほのお
	TMT_WATER,								// みず
	TMT_GRASS,								// くさ
	TMT_ELEC,								// でんき
	TMT_ICE,								// こおり
	TMT_BATTLE,								// かくとう
	//
	TMT_POISON,								// どく
	TMT_GROUND,								// じめん
	TMT_FLY,								// ひこう
	TMT_ESPER,								// エスパー
	TMT_INSECT,								// むし
	TMT_ROCK,								// いわ
	TMT_GHOST,								// ゴースト
	TMT_DRAGON,								// ドラゴン
	TMT_EVIL,								// あく
	TMT_IRON,								// はがね
	//
	TMT_MAX,
};

// CLWK(下画面共通)
enum{
	CLWKID_CMN_LEFT = 0,					// 左
	CLWKID_CMN_RIGHT,						// 右
	CLWKID_CMN_CROSS,						// ×
	CLWKID_CMN_RETURN,						// 戻る
	//
	CLWKID_CMN_MAX,
};

// シーケンス
enum{
	CMN_LEFT_SEQ 		= 4,				// 左
	CMN_RIGHT_SEQ 		= 5,				// 右
	CMN_CROSS_SEQ 		= 0,				// ×
	CMN_RETURN_SEQ 		= 1,				// 戻る
	//
	CMN_LEFT_ON_SEQ 	= 12,				// 左オン
	CMN_LEFT_OFF_SEQ 	= 18,				// 左オフ
	CMN_RIGHT_ON_SEQ 	= 13,				// 右オン
	CMN_RIGHT_OFF_SEQ 	= 19,				// 右オフ
	//
	CMN_CROSS_ON_SEQ	= 8,				// ×オン
	CMN_RETURN_ON_SEQ	= 9,				// 戻るオン
};

// CLWK(up)
enum{
	CLWKID_UP_TRAINER	= 0,				// トレーナー
	//
	CLWKID_UP_MAX,
};

// スクロール
enum{
	SCRL_TOP_Y			= 32,				// レール上部のY座標
	SCRL_DOWN_Y			= 160,				// レール下部のY座標
	SCRL_BAR_SIZE		= 16,				// バーのサイズ
};

//-------------------
// フラグ
//-------------------
// OBJのフラグ(WIN_RECORD_OBJ_DATA.dispflag)
enum{
	FLG_OBJ_ANIM		= (1<<0),			// アニメあり
	FLG_OBJ_HFLIP		= (1<<1),			// HFLIP
	FLG_OBJ_VFLIP		= (1<<2),			// VFLIP
};

// WIN_RECORD_MAIN_WORK.Flag
enum{
	FLG_MW_ANM_LEFT		= (1<<0),			// 左矢印アニメ
	FLG_MW_ANM_RIGHT	= (1<<1),			// 右矢印アニメ
	FLG_MW_ANM_CROSS	= (1<<2),			// ×アニメ
	FLG_MW_ANM_RETURN	= (1<<3),			// 戻るアニメ
	//
	FLG_MW_UPDATE		= (1<<4),			// 更新
	FLG_MW_BAR_UPDATE	= (1<<5),			// バー更新
	FLG_MW_NO_SCROLL	= (1<<6),			// スクロールなし(MODE_LV1と同じ)
	//
	FLG_MW_RET_END		= (1<<8),			// RETURNで終了
	FLG_MW_CROSS_END	= (1<<9),			// ×で終了
	FLG_MW_KEY_END		= (1<<10),			// キーで終了
	FLG_MW_1ST_CHK		= (1<<11),			// 1回チェック
	//
	FLG_MW_END			= (1<<12),			// 終了
	//
	// フラグまとめ(アニメ再生中か)
	FLG_MW_ANM_ALL		= (FLG_MW_ANM_LEFT|FLG_MW_ANM_RIGHT|FLG_MW_ANM_CROSS|FLG_MW_ANM_RETURN),
};

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//-------------------------------------
/// OBJ管理ワーク
//-------------------------------------
typedef struct
{
	u32			res[RESID_OBJ_MAX]; 					// リソース
	// 共通
	GFL_CLWK	*cmn_clwk[WIN_RECORD_CMN_OBJ_MAXNUM];	// CLWK(common)
	// 通常
	GFL_CLWK	*clwk[WIN_RECORD_OBJ_MAXNUM];			// CLWK
	//
	GFL_CLWK	*up_clwk[WIN_RECORD_UP_OBJ_MAXNUM];		// CLWK(up)
} WIN_RECORD_OBJ_WORK;

//--------------------------------------------------------------
///	メインワーク
//--------------------------------------------------------------
typedef struct
{
	HEAPID					heapID;
	// OBJ
	WIN_RECORD_OBJ_WORK		wk_obj;
	//描画設定
	WIN_RECORD_GRAPHIC_WORK	*graphic;
	//
	WIN_RECORD_PARAM 		*Param;							// 参照用として保存
	
	//フォント
	GFL_FONT				*font;
	//プリントキュー
	PRINT_QUE				*print_que;
	GFL_MSGDATA				*msg[WIN_RECORD_MES_MAXNUM];			// 0:メッセージ,1:未使用
	PRINT_UTIL				PrintUtil[WIN_RECORD_CANVAS_MAXNUM];	// 文字表示用
	WORDSET					*Wordset;								// wordset
	//
	s32						Step;							// ステップ
	s32						Mode;							// モード(0-2)
	u32						Flag;							// フラグ
	s32						TrainerId;						// トレーナーID
	s32						ScrollOffy;						// オフセット
	s32						DispMaxNum;						// 最大数
	s16						CountBuffer[TMT_MAX+2];			// カウント
	s16						DispBuffer[TMT_MAX+2];			// 表示のバッファ
	// list
	FRAMELIST_WORK			*frame_list_work;				// リスト
	//
	s16						*EmptyBuffer;					// 座標用のバッファ(必ず破棄すること)
	s16						*ParamBuffer;					// パラメータのバッファ(必ず破棄すること)
	GFL_UI_TP_HITTBL		HitBuffer[AREA_TP_MAX+1];		// タッチペン用(+1は終了コードを入れるため)
} WIN_RECORD_MAIN_WORK;

//-----------------------------------------
// OBJ用の定義
//------------------------------------------
typedef struct{
	u8				anmseq;			// アニメシーケンス(0xff終了コード)
	u8				softpri;		// OBJ(プライオリティ)
	u8				bgpri;			// BG(プライオリティ)
	u8				visible;		// 表示/非表示
	u8				palnum;			// パレット番号(現在未使用だが、しばらく残し)
	u8				dispflag;		// 表示フラグ
	u8				empty_num;		// empty番号(これから座標を参照)
	u8				pad;
}WIN_RECORD_OBJ_DATA;

//-----------------------------------------
// 文字用の定義(表示領域の設定:CANVAS)
//------------------------------------------
typedef struct{
	u8				frmnumber;		// BG上下(frame定義なので注意)(0xffの場合は終了判定)
	u8				x;				// 座標
	u8				y;
	u8				width;			// サイズ
	u8				height;
	u8				palnum;			// パレット番号
}WIN_RECORD_CANVAS_DATA;

//-----------------------------------------
// BGPALの定義
//------------------------------------------
typedef struct{
	u8				type;			// タイプ(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				size;			// サイズ(*0x20)
}WIN_RECORD_BGPAL_DATA;

//-----------------------------------------
// BGCHRの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}WIN_RECORD_BGCHR_DATA;

//-----------------------------------------
// BGSCNの定義
//------------------------------------------
typedef struct{
	u8				frmnumber;		// frame(0xffの場合は終了判定)
	u8				arcid;			// id
	u8				offset;			// オフセット
	u8				pad;
}WIN_RECORD_BGSCN_DATA;

//=============================================================================
/**
 *							データ定義(要整理)
 */
//=============================================================================
//---------------
// BG関連
//---------------
// BGPAL
static const WIN_RECORD_BGPAL_DATA wr_bgpal_tbl[] = {
	{ PALTYPE_SUB_BG,	NARC_win_record_gra_win_record_bg_u_NCLR,	0,	3,},		// 上、BG
	{ PALTYPE_MAIN_BG,	NARC_win_record_gra_win_record_bg_d_NCLR,	0,	3,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};

// BGCHR
static const WIN_RECORD_BGCHR_DATA wr_bgchr_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_win_record_gra_win_record_bg_u_NCGR,	0,	0,},		// 上、BG
	{ GFL_BG_FRAME2_S,	NARC_win_record_gra_win_record_bg_u_NCGR,	0,	0,},		// 上、BG
	{ GFL_BG_FRAME1_S,	NARC_win_record_gra_win_record_bg_u_NCGR,	0,	0,},		// 上、BG
	{ GFL_BG_FRAME3_M,	NARC_win_record_gra_win_record_bg_d_NCGR,	0,	0,},		// 下、BG
	{ GFL_BG_FRAME1_M,	NARC_win_record_gra_win_record_bg_d_NCGR,	0,	0,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};

// BGSCN
static const WIN_RECORD_BGSCN_DATA wr_bgscn_tbl[] = {
	{ GFL_BG_FRAME3_S,	NARC_win_record_gra_win_record_bg_u_01_NSCR,	0,	0,},		// 上、下地
	{ GFL_BG_FRAME2_S,	NARC_win_record_gra_win_record_bg_u_02_NSCR,	0,	0,},		// 上、中間
	{ GFL_BG_FRAME1_S,	NARC_win_record_gra_win_record_bg_u_03_NSCR,	0,	0,},		// 上、中間
	{ GFL_BG_FRAME3_M,	NARC_win_record_gra_win_record_bg_d_01_NSCR,	0,	0,},		// 下、下地
	{ GFL_BG_FRAME1_M,	NARC_win_record_gra_win_record_bg_d_02_NSCR,	0,	0,},		// 下、下地

	{ 0xff, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報(通常)
//------------------
// narc, resid, paloffset, palnum
static const s16 wr_obj_main_info_tbl[] = {
	NARC_win_record_gra_win_record_obj_d_NCLR, RESID_OBJ_PLT, 0, 11, // NCL
	NARC_win_record_gra_win_record_obj_d_NCGR, RESID_OBJ_CHR, 0, 0,	// NCG
	NARC_win_record_gra_win_record_obj_d_NCER, RESID_OBJ_CEL, 0, 0, // NCE
	NARC_win_record_gra_win_record_obj_d_NANR, -1, 0, 0,			// NAR?
};

// 情報
static const s16 wr_obj_main_itbl[] = {
	RESID_OBJ_PLT,
	RESID_OBJ_CHR,
	RESID_OBJ_CEL,
	//
	WIN_RECORD_OBJ_MAXNUM,
	CLWKMODE_NORMAL,
};

// OBJのテーブルデータ(通常)
static const WIN_RECORD_OBJ_DATA wr_obj_tbl[] = {
	{ 0,	0x30,	2,	1,	0,	1,	4,	0,},		// ゲージ上
	{ 1,	0x38,	2,	1,	0,	1,	14,	0,},		// ゲージ

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報(cmn)
//------------------
// 情報
static const s16 wr_obj_cmn_itbl[] = {
	RESID_CMN_OBJ_PLT,
	RESID_CMN_OBJ_CHR,
	RESID_CMN_OBJ_CEL,
	//
	WIN_RECORD_CMN_OBJ_MAXNUM,
	CLWKMODE_CMN,
};

// OBJのテーブルデータ(cmn)
static const WIN_RECORD_OBJ_DATA wr_obj_cmn_tbl[] = {
	{ 4,	0x30,	1,	1,	0,	1,	7,	0,},		// 左マーク
	{ 5,	0x30,	1,	1,	0,	1,	8,	0,},		// 右マーク
	{ 0,	0x30,	1,	1,	0,	1,	9,	0,},		// ×マーク
	{ 1,	0x30,	1,	1,	0,	1, 10,	0,},		// 戻るマーク

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// OBJの情報(up)
//------------------
// 情報
static const s16 wr_obj_up_itbl[] = {
	RESID_UP_OBJ_PLT,
	RESID_UP_OBJ_CHR,
	RESID_UP_OBJ_CEL,
	//
	WIN_RECORD_UP_OBJ_MAXNUM,
	CLWKMODE_UP,
};

// OBJのテーブルデータ(up)
static const WIN_RECORD_OBJ_DATA wr_obj_up_tbl[] = {
	{ 0,	0x30,	1,	1,	0,	1,	13,	0,},		// トレーナー

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//---------------
// CANVASの設定
//---------------
// (printutilと対応)
static const WIN_RECORD_CANVAS_DATA wr_canvas_tbl[] = {
	{ GFL_BG_FRAME0_M,	1,	1,	30,	2,	8,},		// 上名前

	{ 0xff, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// メッセージ関連
//------------------
// その他メッセージ
static const u8 wr_gmm_tbl[] = {
	msg_wbt_wincount_01,			// 「プレイヤ名」の
	msg_wbt_wincount_02,			// ゆうしょうかいすう
	msg_wbt_wincount_03,			// ○○かい
	0xff,
};

// トーナメント名
// (TMT_)と対応
static const s16 wr_tmt_gmm_tbl[] = {
	msg_wbt_wincount_000,			// ホドモエ
	msg_wbt_wincount_001,			// イッシュ
	msg_wbt_wincount_002,			// カントー
	msg_wbt_wincount_003,			// ジョウト
	msg_wbt_wincount_004,			// ホウエン
	msg_wbt_wincount_005,			// シンオウ
	msg_wbt_wincount_006,			// ワールド
	msg_wbt_wincount_007,			// チャンピオン
	msg_wbt_wincount_008,			// WCS
	msg_wbt_wincount_009,			// レンタル
	//
	msg_wbt_wincount_010,			// レンタルマスター
	msg_wbt_wincount_011,			// シャッフル
	msg_wbt_wincount_012,			// シャッフルマスター
	msg_wbt_wincount_normal,		// ノーマル
	msg_wbt_wincount_honoo,			// ほのお
	msg_wbt_wincount_mizu,			// みず
	msg_wbt_wincount_kusa,			// くさ
	msg_wbt_wincount_denki,			// でんき
	msg_wbt_wincount_koori,			// こおり
	msg_wbt_wincount_kakutou,		// かくとう
	//
	msg_wbt_wincount_doku,			// どく
	msg_wbt_wincount_zimen,			// じめん
	msg_wbt_wincount_hikou,			// ひこう
	msg_wbt_wincount_esper,			// エスパー
	msg_wbt_wincount_musi,			// むし
	msg_wbt_wincount_iwa,			// いわ
	msg_wbt_wincount_gorst,			// ゴースト
	msg_wbt_wincount_dragon,		// ドラゴン
	msg_wbt_wincount_aku,			// あく
	msg_wbt_wincount_hagane,		// はがね
};

//-------------------------
// 優勝回数の対応状況
// (ここでの並びとの対応)
//-------------------------
static const s16 wbtwin_tbl[] = {
	WBTWIN_COUNT_HODOMOE,			// ホドモエ
	WBTWIN_COUNT_ISSYU,				// イッシュ
	WBTWIN_COUNT_KANTO,				// カントー
	WBTWIN_COUNT_JOUTO,				// ジョウト
	WBTWIN_COUNT_HOUEN,				// ホウエン
	WBTWIN_COUNT_SINOU,				// シンオウ
	WBTWIN_COUNT_WORLD,				// ワールド
	WBTWIN_COUNT_CHAMPION,			// チャンピオンズ
	WBTWIN_COUNT_WCS,				// WCS
	WBTWIN_COUNT_RENTAL,			// レンタル
	//
	WBTWIN_COUNT_RENTALMASTER,		// レンタルマスター
	WBTWIN_COUNT_SHUFFLE,			// シャッフル
	WBTWIN_COUNT_SHUFFLEMASTER,		// シャッフルマスター
	WBTWIN_COUNT_POKETYPE_NORMAL,	// ノーマル
	WBTWIN_COUNT_POKETYPE_HONOO,	// ほのお
	WBTWIN_COUNT_POKETYPE_MIZU,		// みず
	WBTWIN_COUNT_POKETYPE_KUSA,		// くさ
	WBTWIN_COUNT_POKETYPE_DENKI,	// でんき
	WBTWIN_COUNT_POKETYPE_KOORI,	// こおり
	WBTWIN_COUNT_POKETYPE_KAKUTOU,	// かくとう
	//
	WBTWIN_COUNT_POKETYPE_DOKU,		// どく
	WBTWIN_COUNT_POKETYPE_JIMEN,	// じめん
	WBTWIN_COUNT_POKETYPE_HIKOU,	// ひこう
	WBTWIN_COUNT_POKETYPE_ESPER,	// エスパー
	WBTWIN_COUNT_POKETYPE_MUSHI,	// むし
	WBTWIN_COUNT_POKETYPE_IWA,		// いわ
	WBTWIN_COUNT_POKETYPE_GHOST,	// ゴースト
	WBTWIN_COUNT_POKETYPE_DRAGON,	// ドラゴン
	WBTWIN_COUNT_POKETYPE_AKU,		// あく
	WBTWIN_COUNT_POKETYPE_HAGANE,	// はがね
};

//---------------------------------
// 共通ボタン
// (通常、オン、オフ(操作不可)、0)
//---------------------------------
static const s16 wr_cmn_btn_tbl[] = {
	CMN_LEFT_SEQ,   CMN_LEFT_ON_SEQ,   CMN_LEFT_OFF_SEQ,  0,	// 左
	CMN_RIGHT_SEQ,  CMN_RIGHT_ON_SEQ,  CMN_RIGHT_OFF_SEQ, 0,	// 右
	CMN_CROSS_SEQ,  CMN_CROSS_ON_SEQ,  CMN_CROSS_SEQ,     0,	// ×
	CMN_RETURN_SEQ, CMN_RETURN_ON_SEQ, CMN_RETURN_SEQ,    0,	// 戻る
};

//----------------
// リスト処理
//-----------------
// リストタッチデータ
static const FRAMELIST_TOUCH_DATA wr_list_tp_tbl[] =
{
  // スライドの数は上からのパネル分(7)にしないどだめらしい
  // (リストの処理で問題が起こるようだ...)
  { {   0,  23, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 00:(ダミー)
  { {  24,  47, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 00: 
  { {  48,  71, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 01: 
  { {  72,  95, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 02: 
  { {  96, 119, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 03: 
  { { 120, 143, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 04: 
  { { 144, 167, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 05: 
  //
  { {  32, 160, 244, 252 }, FRAMELIST_TOUCH_PARAM_RAIL },     // 06: レール

  { { 168, 192, 136, 160 }, FRAMELIST_TOUCH_PARAM_PAGE_UP },  // 07: 左
  { { 168, 192, 168, 192 }, FRAMELIST_TOUCH_PARAM_PAGE_DOWN },// 08: 右

  { { GFL_UI_TP_HIT_END, 0, 0, 0 }, 0 },
};

// スクロールなしのタッチデータ
static const FRAMELIST_TOUCH_DATA wr_list_tp2_tbl[] =
{
#if 1	// なしでよい
  { {   0,  23, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 00: 
  { {  24,  47, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 00: 
  { {  48,  71, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 01: 
  { {  72,  95, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 02: 
  { {  96, 119, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 03: 
  { { 120, 143, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 04: 
  { { 144, 167, 8, 232 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 05: 
  //
  { { 168, 192, 136, 160 }, FRAMELIST_TOUCH_PARAM_PAGE_UP },  // 08: 左
  { { 168, 192, 168, 192 }, FRAMELIST_TOUCH_PARAM_PAGE_DOWN },// 09: 右
#endif
  { { GFL_UI_TP_HIT_END, 0, 0, 0 }, 0 },
};

// 
static const FRAMELIST_HEADER wr_frame_list_header = {
  GFL_BG_FRAME2_M,			// 上
  FRAMELIST_SUB_BG_NONE,	// 下
  
  1,   // 項目フレーム表示開始Ｘ座標
//  0,    // 項目フレーム表示開始Ｙ座標(●0以外だとリストが長い場合にうまくいかない?)
  3,    // 項目フレーム表示開始Ｙ座標(●0以外だとリストが長い場合にうまくいかない?)
  28,   // 項目フレーム表示Ｘサイズ
  3,    // 項目フレーム表示Ｙサイズ
  
  1,    // フレーム内に表示するBMPWINの表示Ｘ座標
  0,    // フレーム内に表示するBMPWINの表示Ｙ座標
  26,   // フレーム内に表示するBMPWINの表示Ｘサイズ
  3,    // フレーム内に表示するBMPWINの表示Ｙサイズ(itemSizY)
  2,    // フレーム内に表示するBMPWINのパレット
  
  {24,12,8,6,4,3,},    // スクロール速度 [0] = 最速 ※itemSizY*8を割り切れる値であること！
   2,   // 選択項目のパレット(●この番号のパレットが選択中となるようだ)
  16,   // スクロールバーのＹサイズ
   7,   // 項目登録数(●初期化前に変更すればよい)
   1,   // 背景登録数
   0,   // 初期カーソル位置
//   7,   // カーソル移動範囲
   6,   // カーソル移動範囲
   0,   // 初期スクロール値
  
  wr_list_tp_tbl,  // タッチデータ
  
  NULL, // コールバック関数
  NULL, // コールバック関数に渡されるワーク
};

// パネル上の表示位置(x座標)
static const s16 wr_mes_pos_tbl[] = {
	0, 144,
};

// (0x100 = 1)
// (0.25ずつ移動で32dotでスクロール)
static const s16 wr_scroll_tbl[] = {
	0x40, 0x2000,
};


//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT WinRecordProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT WinRecordProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT WinRecordProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );

//-------------------------------------
///	以下に追加
//=====================================
static void wr_init( WIN_RECORD_MAIN_WORK* wk );
static void wr_main( WIN_RECORD_MAIN_WORK* wk );
static void wr_exit( WIN_RECORD_MAIN_WORK* wk );
static void wr_down( WIN_RECORD_MAIN_WORK* wk );
static void wr_key_all( WIN_RECORD_MAIN_WORK* wk );
static void wr_main_print( WIN_RECORD_MAIN_WORK* wk );
//
static s16 wr_get_empty( WIN_RECORD_MAIN_WORK *wk, const s32 num );
//
static void wr_init_bg( WIN_RECORD_MAIN_WORK* wk );
//
static void wr_init_obj_clwk( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const WIN_RECORD_OBJ_DATA *obj_adr, const s16 *info_adr );
static void wr_init_obj_res( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr );
static void wr_init_obj_res_cmn( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype );
static void wr_init_obj_res_up( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const s32 id );
static void wr_init_obj( WIN_RECORD_MAIN_WORK* wk );
static void wr_exit_obj( WIN_RECORD_MAIN_WORK* wk );
static void wr_chg_objseq( WIN_RECORD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq );
static void wr_set_obj_visible( WIN_RECORD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg );
static BOOL wr_is_anime_end( WIN_RECORD_MAIN_WORK *wk, const s32 wmode, const s32 wnum );
//
// 文字列関連
static void wr_init_string( WIN_RECORD_MAIN_WORK* wk );
static void wr_disp_string( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 dispmode );
static void wr_exit_string( WIN_RECORD_MAIN_WORK* wk );
static void wr_clear_canvas( WIN_RECORD_MAIN_WORK* wk, const s32 mode );
static void wr_disp_string_id( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 mesid, const s32 xpos, const s32 ypos, const u32 dispmode );
//
// パラメータ関連
static void wr_init_param( WIN_RECORD_MAIN_WORK* wk );
static void wr_set_param( WIN_RECORD_MAIN_WORK* wk );
static void wr_set_cmn_anim( WIN_RECORD_MAIN_WORK* wk, const s32 mode );
//
// 入力(キー、タッチペン)
static void wr_chk_input( WIN_RECORD_MAIN_WORK* wk );
static BOOL wr_chk_key_input( WIN_RECORD_MAIN_WORK* wk );
static void wr_get_tp_area( WIN_RECORD_MAIN_WORK* wk, const s32 num, GFL_UI_TP_HITTBL *tpbuffer );
static BOOL wr_chk_tp_input( WIN_RECORD_MAIN_WORK* wk );
static void wr_chk_anim( WIN_RECORD_MAIN_WORK* wk );
//
// フラグ
static BOOL wr_chk_dispflag( WIN_RECORD_MAIN_WORK* wk, const s32 mode );

// リスト
static void wr_init_framelist( WIN_RECORD_MAIN_WORK * wk );
static u32  wr_main_framelist( WIN_RECORD_MAIN_WORK * wk );
static s32  wr_is_main_framelist( WIN_RECORD_MAIN_WORK * wk, const u32 mode );
static void wr_exit_framelist( WIN_RECORD_MAIN_WORK * wk );
//
static void wr_listcallback_draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp );
static void wr_listcallback_move(void *work, u32 pos, BOOL flag);
static void wr_listcallback_scroll( void *work, s8 mv );
//
static void _liststr_print( WIN_RECORD_MAIN_WORK *wk, int pos, PRINT_UTIL *print_util, GFL_MSGDATA *msg );
static void PrintStrBuf( WIN_RECORD_MAIN_WORK *wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int strIdx, PRINTSYS_LSB col, int y, int pos );
static void _strbuf_print( WIN_RECORD_MAIN_WORK *wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int param, int pos );
//
static void wr_main_scrollbar( WIN_RECORD_MAIN_WORK * wk );
static void wr_main_bg_scroll( WIN_RECORD_MAIN_WORK * wk );
static void wr_main_etc_input( WIN_RECORD_MAIN_WORK * wk );

//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA WinRecordProcData = 
{
	WinRecordProc_Init,
	WinRecordProc_Main,
	WinRecordProc_Exit,
};

//---------------------
// リストのコールバック
//---------------------
static const FRAMELIST_CALLBACK wr_list_callback = {
	wr_listcallback_draw,
	wr_listcallback_move,
	wr_listcallback_scroll,
};

//=============================================================================
/**
 *								PROC
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 初期化処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					WIN_RECORD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WinRecordProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	WIN_RECORD_MAIN_WORK 	*wk;
	
	//オーバーレイ読み込み
	GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
	
	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ENTRY_CHECKER, UI_TEMPLATE_HEAP_SIZE );
	wk = GFL_PROC_AllocWork( proc, sizeof(WIN_RECORD_MAIN_WORK), HEAPID_ENTRY_CHECKER );
	GFL_STD_MemClear( wk, sizeof(WIN_RECORD_MAIN_WORK) );
	
	// 初期化
	wk->heapID = HEAPID_ENTRY_CHECKER;
	
	// 参照用
	wk->Param = pwk;
	
	//描画設定初期化(上:サブ、下:メイン)
	wk->graphic	= WIN_RECORD_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );
	
	//フォント作成
	wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
	
	// メッセージ
	wk->msg[0] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wbt_wincount_dat, wk->heapID );
	wk->msg[1] = 0;
	
	//PRINT_QUE作成
	wk->print_que = PRINTSYS_QUE_Create( wk->heapID );
	//
	// WORDSET生成
	wk->Wordset = WORDSET_Create( wk->heapID );
	
	//
	//----------------
	// 通常はこの中で初期化
	//----------------
	wr_init( wk );
	
	// ワイプ
	WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
					WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
	
	return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					WIN_RECORD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WinRecordProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	s32		i;
	WIN_RECORD_MAIN_WORK* wk = mywk;
	
	// 終了判定
	if(wk->Flag & FLG_MW_KEY_END){
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
	}else{
		GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
	}
	
	//----------------
	// 処理を記述
	//----------------
	wr_exit( wk );
	
	//メッセージ破棄
	for(i=0;i<WIN_RECORD_MES_MAXNUM;i++){
		if(wk->msg[i]){
			GFL_MSG_Delete( wk->msg[0] );
			wk->msg[i] = 0;
		}
	}
	
	// WORDSET破棄
	WORDSET_Delete( wk->Wordset );
	
	//PRINT_QUE
	PRINTSYS_QUE_Delete( wk->print_que );
	//
	//FONT
	GFL_FONT_Delete( wk->font );
	
	//描画設定破棄
	WIN_RECORD_GRAPHIC_Exit( wk->graphic );
	
	//PROC用メモリ解放
	{
		HEAPID heapID = wk->heapID;
		GFL_PROC_FreeWork( proc );
		GFL_HEAP_DeleteHeap( heapID );
	}
	//オーバーレイ破棄
	GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));
	//
	return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					WIN_RECORD_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT WinRecordProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	WIN_RECORD_MAIN_WORK* wk = mywk;
	
	switch ( *seq )
	{
		case 0:
			if (WIPE_SYS_EndCheck())
			{
				(*seq) ++;
			}
			break;
		case 1:
			if( wk->Step == STEP_END )
			{
				(*seq) ++;
			}
			break;
		case 2:
			WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
							WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
			(*seq) ++;
			break;
		//
		// ここで最終終了待ちをしているようだ
		case 3:
			if (WIPE_SYS_EndCheck())
			{
				return GFL_PROC_RES_FINISH;
			}
			break;
		default:
			break;
	}
	
	//----------------
	// 処理を追加
	//----------------
	wr_main( wk );
	
	//PRINT_QUE
	wr_main_print( wk );
	
	//2D描画
	WIN_RECORD_GRAPHIC_2D_Draw( wk->graphic );
	
	//3D描画
	WIN_RECORD_GRAPHIC_3D_StartDraw( wk->graphic );
	
	WIN_RECORD_GRAPHIC_3D_EndDraw( wk->graphic );
	
	return GFL_PROC_RES_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(init)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init( WIN_RECORD_MAIN_WORK* wk )
{
	s32			i;
	
	//----------
	// 初期化
	//----------
	// 上、下画面をまとめて処理
	wk->Step 		= STEP_INIT;
	wk->Mode		= wk->Param->status;	// 外部から
	wk->Flag		= 0;
	wk->TrainerId	= 0;
	wk->DispMaxNum	= 0;
	wk->ScrollOffy	= 0;
	//
	// カウント表示用バッファ
	for(i=0;i<TMT_MAX+2;i++){
		wk->CountBuffer[i] = i;
		wk->DispBuffer[i] = 0;
	}
	//
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_WIN_RECORD_GRA, NARC_win_record_gra_win_record_pos_tblbin, wk->heapID );
	// パラメータ用のバッファ取得(必ず破棄すること)
	wk->ParamBuffer = GFL_ARC_LoadDataAlloc( ARCID_WIN_RECORD_GRA, NARC_win_record_gra_win_record_param_tblbin, wk->heapID );
	//
	//---------------------------
	// 外部パラメータセット
	// (ここでDispMaxNumがでる)
	//---------------------------
	wr_set_param( wk );
	
	//---------------
	// bgの初期化
	//---------------
	wr_init_bg( wk );
	
	//---------------
	// 文字列関連の初期化
	//---------------
	wr_init_string( wk );
	
	//------------
	// OBJ初期化
	//------------
	wr_init_obj( wk );
	
	//-----------------
	// 内部パラメータセット
	//-----------------
	wr_init_param( wk );
	
	//-----------------
	// リスト処理
	//-----------------
	wr_init_framelist( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_main( WIN_RECORD_MAIN_WORK* wk )
{
	// 下画面
	wr_down( wk );
	//
	// 背景bgスクロール
	wr_main_bg_scroll( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(exit)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_exit( WIN_RECORD_MAIN_WORK* wk )
{
	//-----------------
	// 終了パラメータ
	//------------------
	// ×で終了 => フリームーブとなる
	if(wk->Flag & FLG_MW_CROSS_END){
		if(wk->Param->result_work != NULL){
			*wk->Param->result_work = TRUE;
		}
	}
	// RETURNで終了 => パソコン画面へ
	else{
		if(wk->Param->result_work != NULL){
			*wk->Param->result_work = FALSE;
		}
	}
	if(wk->Param->result_work != NULL){
		OS_Printf("result : %d\n",*wk->Param->result_work);
	}else{
		OS_Printf("result : null(debug)\n");
	}
	
	// 上、下画面をまとめて処理
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	// paramのバッファクリア
	GFL_HEAP_FreeMemory( wk->ParamBuffer );
	
	// リスト処理
	wr_exit_framelist( wk );
	
	// 文字表示用
	wr_exit_string( wk );
	// OBJ破棄
	wr_exit_obj( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(下画面)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_down( WIN_RECORD_MAIN_WORK* wk )
{
	wr_key_all( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力処理まとめ
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_key_all( WIN_RECORD_MAIN_WORK* wk )
{
	s32		wnum;
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
			break;
		//
		// 入力まとめ
		case STEP_LV1:
			wr_chk_input( wk );
			break;
		//
		// 右アニメ
		case STEP_LV1+10:
			wnum = CLWKID_CMN_RIGHT;
			bflg = wr_is_anime_end( wk, CLWKMODE_CMN, wnum );
			if(bflg == FALSE){
				wr_chg_objseq( wk, CLWKMODE_CMN, wnum, wr_cmn_btn_tbl[wnum*4] );
				wk->Flag &= ~(1<<wnum);
				wk->Step = STEP_LV1;
			}
			break;
		//
		// 左アニメ
		case STEP_LV1+20:
			wnum = CLWKID_CMN_LEFT;
			bflg = wr_is_anime_end( wk, CLWKMODE_CMN, wnum );
			if(bflg == FALSE){
				wr_chg_objseq( wk, CLWKMODE_CMN, wnum, wr_cmn_btn_tbl[wnum*4] );
				wk->Flag &= ~(1<<wnum);
				wk->Step = STEP_LV1;
			}
			break;
		
		// 終了アニメ待ち
		case STEP_LV2:
			wr_chk_anim( wk );
			if( wk->Flag & FLG_MW_END ){
				wk->Step = STEP_END;
			}
			break;
		//
		// END
		case STEP_END:
			break;
		//
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  Queの処理(Main)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_main_print( WIN_RECORD_MAIN_WORK* wk )
{
	s32		i;
	
	// Queの処理
	PRINTSYS_QUE_Main( wk->print_que );
	//
	for(i=0;i<WIN_RECORD_CANVAS_MAXNUM;i++){
		PRINT_UTIL_Trans( &wk->PrintUtil[i], wk->print_que );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  emptyの取得
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			座標index(x,yが2つセットとなっている)
 *
 *	@retval s16					座標
 */
//-----------------------------------------------------------------------------
static s16 wr_get_empty( WIN_RECORD_MAIN_WORK *wk, const s32 num )
{
	s16	*adr;
	
	// 念のため
	if((num < 0)||(num >= WIN_RECORD_EMPTY_MAXNUM)){
		OS_Printf("empty error :%d\n",num);
		return(0);
	}
	
	adr = wk->EmptyBuffer;
	return( adr[num] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BG関連の初期化
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_bg( WIN_RECORD_MAIN_WORK* wk )
{
	u16		*adr;
	s32		i;
	s32		j;
	s32		tmp;
	s32		off;
	const WIN_RECORD_BGPAL_DATA	*padr = &wr_bgpal_tbl[0];
	const WIN_RECORD_BGCHR_DATA	*cadr = &wr_bgchr_tbl[0];
	const WIN_RECORD_BGSCN_DATA	*sadr = &wr_bgscn_tbl[0];
	//
	ARCHANDLE		*handle;
	//
	handle	= GFL_ARC_OpenDataHandle( ARCID_WIN_RECORD_GRA, wk->heapID );
	
	// パレット転送
	for(i=0;i<WIN_RECORD_BGNUM_MAXNUM;i++){
		// 終了条件
		if(padr->type == 0xff){
			break;
		}
		tmp = padr->size;
		tmp *= 0x20;		// 転送サイズ
		off = padr->offset;
		off *= 0x20;
		GFL_ARCHDL_UTIL_TransVramPalette( handle, padr->arcid, padr->type, off, tmp, wk->heapID );
		padr++;
	}
	//
	// キャラ転送
	for(i=0;i<WIN_RECORD_BGNUM_MAXNUM;i++){
		// 終了条件
		if(cadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, cadr->arcid, cadr->frmnumber, cadr->offset, 0, 0, wk->heapID );
		cadr++;
	}
	
	// スクリーン転送
	for(i=0;i<WIN_RECORD_BGNUM_MAXNUM;i++){
		// 終了条件
		if(sadr->frmnumber == 0xff){
			break;
		}
		GFL_ARCHDL_UTIL_TransVramScreen( handle, sadr->arcid, sadr->frmnumber, sadr->offset, 0, 0, wk->heapID );
		sadr++;
	}
	// 終了
	GFL_ARC_CloseDataHandle( handle );
	
#if 0
	//--------------------
	// スクリーンの書き換え
	//--------------------
	// スクリーンバッファ指定
	adr = GFL_BG_GetScreenBufferAdrs( GFL_BG_FRAME3_M );
	for(i=0;i<32;i++){
		for(j=0;j<32;j++){
			// オフセットを加算
			adr[i*32+j] += ((PLTID_BG_BASE&0x0f)<<12);
		}
	}
	
	// スクリーンバッファ指定
	adr = GFL_BG_GetScreenBufferAdrs( GFL_BG_FRAME1_M );
	for(i=0;i<32;i++){
		for(j=0;j<32;j++){
			// オフセットを加算
			adr[i*32+j] += ((PLTID_BG_BASE&0x0f)<<12);
		}
	}
	//
	GFL_BG_LoadScreenReq( GFL_BG_FRAME3_M );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
#endif
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(clwk)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32					drawtype
 *	@param	const WIN_RECORD_OBJ_DATA*	アドレス
 *	@param	const s16*					アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_obj_clwk( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const WIN_RECORD_OBJ_DATA *obj_adr, const s16 *info_adr )
{
	s16						tmp;
	s16						pos[2];
	s16						buf[6];
	s32 					i;
	BOOL					bflg;
	GFL_CLWK_DATA 			cldata;
	CLWK_FLIP_TYPE			flip;
    GFL_CLUNIT*				p_clunit 	= WIN_RECORD_GRAPHIC_GetClunit( wk->graphic );
	// アドレスセット
	const WIN_RECORD_OBJ_DATA *adr 	= obj_adr;
	const s16 				*iadr 	= info_adr;
	GFL_CLWK				**clwk;			// アドレス注意
	
	GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
	// チェック
	buf[0] = iadr[0];		// plt
	buf[1] = iadr[1];		// chr
	buf[2] = iadr[2];		// cel
	buf[3] = iadr[3];		// max
	buf[4] = iadr[4];		// mode(0:通常、1:cmn)
	//
	// アドレスセット
	if(buf[4] == CLWKMODE_NORMAL){			// 通常
		clwk = &wk->wk_obj.clwk[0];
	}
	else if(buf[4] == CLWKMODE_CMN){		// cmn
		clwk = &wk->wk_obj.cmn_clwk[0];
	}
	else{									// up
		clwk = &wk->wk_obj.up_clwk[0];
	}
	
	for(i=0;i<buf[3];i++){
		// 終了判定
		if(adr->anmseq == 0xff){
			break;
		}
		tmp = adr->empty_num;
		pos[0] = wr_get_empty(wk, tmp*2);
		pos[1] = wr_get_empty(wk, tmp*2+1);
		cldata.pos_x 	= pos[0];
		cldata.pos_y 	= pos[1];
		cldata.anmseq 	= adr->anmseq;
		cldata.softpri	= adr->softpri;
		cldata.bgpri	= adr->bgpri;
		//
		clwk[i] =   GFL_CLACT_WK_Create( p_clunit, 
											wk->wk_obj.res[ buf[1] ],			// chr
											wk->wk_obj.res[ buf[0] ],			// plt
											wk->wk_obj.res[ buf[2] ],			// cel
											&cldata,
											drawtype,
											wk->heapID );
		//-------------
		// 表示/非表示
		//-------------
		bflg = FALSE;
		if(adr->visible == 1){
			bflg = TRUE;
		}
		GFL_CLACT_WK_SetDrawEnable( clwk[i], bflg );
		//----------
		// フラグ
		//----------
		// アニメ
		bflg = FALSE;
		if(adr->dispflag & FLG_OBJ_ANIM){
			bflg = TRUE;
		}
		GFL_CLACT_WK_SetAutoAnmFlag( clwk[i], bflg );
		//
		// Hフリップ
		if(adr->dispflag & FLG_OBJ_HFLIP){
			GFL_CLACT_WK_SetFlip( clwk[i], CLWK_FLIP_H, TRUE );
		}
		// Vフリップ
		if(adr->dispflag & FLG_OBJ_VFLIP){
			GFL_CLACT_WK_SetFlip( clwk[i], CLWK_FLIP_V, TRUE );
		}
		// 次のデータへ
		adr++;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(res)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *	@param	const s16*			アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_obj_res( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const s16 *info_adr )
{
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( ARCID_WIN_RECORD_GRA, wk->heapID );
    GFL_CLUNIT*	p_clunit = WIN_RECORD_GRAPHIC_GetClunit( wk->graphic );
	const s16 *adr		 = info_adr;		// アドレスセット
	//
	// セルアニメ用の処理
	wk->wk_obj.res[ adr[0+1] ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, adr[0], drawtype, adr[2], 0, adr[3], wk->heapID );
	wk->wk_obj.res[ adr[4+1] ] = GFL_CLGRP_CGR_Register(      p_handle, adr[4], FALSE, drawtype, wk->heapID );
	wk->wk_obj.res[ adr[8+1] ] = GFL_CLGRP_CELLANIM_Register( p_handle, adr[8], adr[12], wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(cmn res)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_obj_res_cmn( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype )
{
	// cmnのidを取得
	const u32 	arcid    = APP_COMMON_GetArcId();
	ARCHANDLE*	p_handle = GFL_ARC_OpenDataHandle( arcid, wk->heapID );
    GFL_CLUNIT*	p_clunit = WIN_RECORD_GRAPHIC_GetClunit( wk->graphic );
	u32			idx[4];
	//
	idx[0] = APP_COMMON_GetBarIconPltArcIdx();		// pal
	idx[1] = APP_COMMON_GetBarIconCharArcIdx();		// ncg
	idx[2] = APP_COMMON_GetBarIconCellArcIdx(  APP_COMMON_MAPPING_128K );	// cell
	idx[3] = APP_COMMON_GetBarIconAnimeArcIdx( APP_COMMON_MAPPING_128K );	// anim
	//
	// セルアニメ用の処理
	wk->wk_obj.res[ RESID_CMN_OBJ_PLT ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, idx[0], drawtype, 0x20*PLTID_OBJ_CMN, 0, PLTID_OBJ_CMN_NUM, wk->heapID );
	wk->wk_obj.res[ RESID_CMN_OBJ_CHR ] = GFL_CLGRP_CGR_Register(      p_handle, idx[1], FALSE, drawtype, wk->heapID );
	wk->wk_obj.res[ RESID_CMN_OBJ_CEL ] = GFL_CLGRP_CELLANIM_Register( p_handle, idx[2], idx[3], wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(res up)(トレーナーデータ専用)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			drawtype
 *	@param	const s32			id
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_obj_res_up( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const s32 id )
{
	ARCHANDLE*	p_handle = TR2DGRA_OpenHandle( wk->heapID );
	//
	wk->wk_obj.res[ RESID_UP_OBJ_PLT ] = TR2DGRA_OBJ_PLTT_Register( p_handle, id, drawtype, 0, wk->heapID );
	wk->wk_obj.res[ RESID_UP_OBJ_CHR ] = TR2DGRA_OBJ_CGR_Register(  p_handle, id, drawtype, wk->heapID );
	wk->wk_obj.res[ RESID_UP_OBJ_CEL ] = TR2DGRA_OBJ_CELLANM_Register( id, APP_COMMON_MAPPING_32K, drawtype, wk->heapID );
	//
	GFL_ARC_CloseDataHandle( p_handle );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化まとめ
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_obj( WIN_RECORD_MAIN_WORK* wk )
{
    GFL_CLUNIT*	p_clunit = WIN_RECORD_GRAPHIC_GetClunit( wk->graphic );
	// リソース読み込み(通常)
	wr_init_obj_res( wk, CLSYS_DRAW_MAIN, &wr_obj_main_info_tbl[0] );
	// リソース読み込み(cmn)
	wr_init_obj_res_cmn( wk, CLSYS_DRAW_MAIN );
	// リソース読み込み(up)
	wr_init_obj_res_up( wk, CLSYS_DRAW_SUB, wk->TrainerId );
	// CLWK登録(通常)
	wr_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &wr_obj_tbl[0], &wr_obj_main_itbl[0] );
	// CLWK登録(cmn)
	wr_init_obj_clwk( wk, CLSYS_DRAW_MAIN, &wr_obj_cmn_tbl[0], &wr_obj_cmn_itbl[0] );
	// CLWK登録(up)
	wr_init_obj_clwk( wk, CLSYS_DRAW_SUB, &wr_obj_up_tbl[0], &wr_obj_up_itbl[0] );
	//
	// 通信アイコンセット(上画面)
	GFL_NET_ReloadIconTopOrBottom( TRUE, wk->heapID );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ破棄
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_exit_obj( WIN_RECORD_MAIN_WORK* wk )
{
	s32		i;
	
	// OBJ後処理
	for(i=0;i<WIN_RECORD_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.clwk[i] );
	}
	for(i=0;i<WIN_RECORD_CMN_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.cmn_clwk[i] );
	}
	for(i=0;i<WIN_RECORD_UP_OBJ_MAXNUM;i++){
		GFL_CLACT_WK_Remove( wk->wk_obj.up_clwk[i] );
	}
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_OBJ_CEL] );
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_CMN_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_CMN_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_CMN_OBJ_CEL] );
	//リソース破棄 
	GFL_CLGRP_PLTT_Release( wk->wk_obj.res[RESID_UP_OBJ_PLT] );
	GFL_CLGRP_CGR_Release(  wk->wk_obj.res[RESID_UP_OBJ_CHR] );
	GFL_CLGRP_CELLANIM_Release( wk->wk_obj.res[RESID_UP_OBJ_CEL] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objのシーケンスの切り替え
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通、2:上画面)
 *	@param	const s32			objワーク番号
 *	@param	const s32			シーケンス番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_chg_objseq( WIN_RECORD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const s32 seq )
{
	// 切り替え
	if(wmode == CLWKMODE_NORMAL){			// 通常
		if(wnum >= CLWKID_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.clwk[wnum], TRUE );
	}
	else if(wmode == CLWKMODE_CMN){			// 共通
		if(wnum >= CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.cmn_clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.cmn_clwk[wnum], TRUE );
	}
	else if(wmode == CLWKMODE_UP){			// 上画面
		if(wnum >= CLWKID_UP_MAX){
			OS_Printf("up obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetAnmSeq( wk->wk_obj.up_clwk[wnum], seq );
		GFL_CLACT_WK_SetAutoAnmFlag( wk->wk_obj.up_clwk[wnum], TRUE );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJの表示/非表示
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通、2:上画面)
 *	@param	const s32			objワーク番号
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_set_obj_visible( WIN_RECORD_MAIN_WORK *wk, const s32 wmode, const s32 wnum, const BOOL bflg )
{
	// 表示/非表示
	if(wmode == CLWKMODE_NORMAL){		// 通常
		if(wnum >= CLWKID_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.clwk[wnum], bflg );
	}
	else if(wmode == CLWKMODE_CMN){		// 共通
		if(wnum >= CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.cmn_clwk[wnum], bflg );
	}
	else if(wmode == CLWKMODE_UP){		// up
		if(wnum >= CLWKID_UP_MAX){
			OS_Printf("up obj max error:%d\n",wnum);
			return;
		}
		GFL_CLACT_WK_SetDrawEnable(  wk->wk_obj.up_clwk[wnum], bflg );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJのアニメの終了待ち
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通、2:上画面)
 *	@param	const s32			objワーク番号
 *
 *	@retval	const BOOL			(TRUE:再生中、FALSE:停止、終了)
 */
//-----------------------------------------------------------------------------
static BOOL wr_is_anime_end( WIN_RECORD_MAIN_WORK *wk, const s32 wmode, const s32 wnum )
{
	BOOL	bflg=TRUE;
	
	// 終了判定らしい
	if(wmode == CLWKMODE_NORMAL){		// 通常
		if(wnum >= CLWKID_MAX){
			OS_Printf("obj max error:%d\n",wnum);
			return( FALSE );
		}
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	else if(wmode == CLWKMODE_CMN){		// 共通
		if(wnum >= CLWKID_CMN_MAX){
			OS_Printf("cmn obj max error:%d\n",wnum);
			return( FALSE );
		}
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.cmn_clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	else if(wmode == CLWKMODE_UP){		// up
		if(wnum >= CLWKID_UP_MAX){
			OS_Printf("up obj max error:%d\n",wnum);
			return( FALSE );
		}
		if( GFL_CLACT_WK_CheckAnmActive( wk->wk_obj.up_clwk[wnum] ) == FALSE ){
			bflg = FALSE;
		}
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(現在、Canvasの作成)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_string( WIN_RECORD_MAIN_WORK* wk )
{
	s16		buf[2];
	s32		i;
	const WIN_RECORD_CANVAS_DATA	*adr = &wr_canvas_tbl[0];
	
	// Canvasの設定
	for(i=0;i<WIN_RECORD_CANVAS_MAXNUM;i++){
		// 終了条件
		if(adr->frmnumber == 0xff){
			break;
		}
		// セット
		wk->PrintUtil[i].win = GFL_BMPWIN_Create( adr->frmnumber, adr->x, adr->y, adr->width, adr->height, adr->palnum, GFL_BMP_CHRAREA_GET_B );
		// スクリーンは一度でよいはず
		GFL_BMPWIN_MakeScreen( wk->PrintUtil[i].win );
		GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->PrintUtil[i].win) );
		adr++;
	}
	//
	// fontパレットの転送
	{
		ARCHANDLE * p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, wk->heapID );
		//上画面フォントパレット
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_MAIN_BG, PLTID_BG_FONT*0x20, 0x20, wk->heapID );
		GFL_ARC_CloseDataHandle( p_handle );
	}
	//
	// ゆうしょう回数
	wr_disp_string_id( wk, 0, wr_gmm_tbl[1], 0, 0, PRINTTOOL_MODE_LEFT );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示(カラーに注意:要修正)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			Canvasの番号
 *	@param	const s32			表示座標(x)
 *	@param	const s32			表示座標(y)
 *	@param	const STRBUF* 		表示文字列
 *	@param	const u32			表示位置(左、右、中央など)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_disp_string( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 dispmode )
{
	// 表示
	PRINTTOOL_PrintColor(
				&wk->PrintUtil[mode],			// プリントユーティル ( BMPWIN )
				wk->print_que,					// プリントキュー
				x, y,							// 表示基準座標
				strbuf,							// 文字列
				wk->font,						// フォント
				PRINTSYS_LSB_Make(15,2,0),		// カラー
				dispmode );						// 位置
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示終了
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_exit_string( WIN_RECORD_MAIN_WORK* wk )
{
	s32		i;
	
	for(i=0;i<WIN_RECORD_CANVAS_MAXNUM;i++){
		GFL_BMPWIN_Delete( wk->PrintUtil[i].win );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ウインドウクリア
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32				キャンバスの番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_clear_canvas( WIN_RECORD_MAIN_WORK* wk, const s32 mode )
{
	// clear
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->PrintUtil[mode].win ), MES_CLEAR_COLOR );
//	GFL_BMPWIN_TransVramCharacter( wk->PrintUtil[mode].win );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字列表示(id参照)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const mode				(PrintUtil[mode]のmode)
 *	@param	const s32				mesid
 *	@param	const s32				x座標
 *	@param	const s32				y座標
 *	@param	const u32				表示位置(左、中央、右)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_disp_string_id( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 mesid, const s32 xpos, const s32 ypos, const u32 dispmode )
{
	STRBUF		*strbuf;
	//
	strbuf = GFL_MSG_CreateString( wk->msg[0], mesid );
	wr_disp_string( wk, mode, xpos, ypos, strbuf, dispmode );
	// 破棄
	GFL_STR_DeleteBuffer( strbuf );
}

//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  パラメータ初期化(ここでパラメータを参照)
 *
 *	@param	WIN_ROUND_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_param( WIN_RECORD_MAIN_WORK* wk )
{
	s16					i;
	GFL_UI_TP_HITTBL	*adr = &wk->HitBuffer[0];
	BOOL				bflg;
	//
	//--------------------
	// タッチペン領域計算
	//--------------------
	for(i=0;i<AREA_TP_MAX;i++){
		wr_get_tp_area( wk, AREA_TP_LEFT+i, adr );
		adr++;
	}
	// 終了コード
	adr->rect.top		= GFL_UI_TP_HIT_END;
	adr->rect.bottom	= 0;
	adr->rect.left		= 0;
	adr->rect.right		= 0;
	//
	//--------------------
	// スクロール関連
	//--------------------
	// スクロール関連を非表示
	if(wk->Flag & FLG_MW_NO_SCROLL){
		// バー非表示
		wr_set_obj_visible( wk, CLWKMODE_NORMAL, CLWKID_BAR, FALSE );
		// 左右矢印非表示
		wr_set_obj_visible( wk, CLWKMODE_CMN, CLWKID_CMN_LEFT,  FALSE );
		wr_set_obj_visible( wk, CLWKMODE_CMN, CLWKID_CMN_RIGHT, FALSE );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  パラメータセット
 *
 *	@param	WIN_ROUND_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_set_param( WIN_RECORD_MAIN_WORK* wk )
{
	u16		cnt;
	s32		i;
	s32		j;
	u32		tmp;
	BOOL	bflg;
	
	//------------
	// 性別を取得
	//------------
	tmp = MyStatus_GetMySex( wk->Param->my_status );
	wk->TrainerId = 0;			// 男性
	if(tmp == PM_FEMALE){
		wk->TrainerId = 1;		// 女性
	}
	//-----------
	// 優勝回数
	//-----------
	for(i=0;i<TMT_MAX;i++){
		cnt = WBTSAVE_GetWinCount( wk->Param->wbtsave, wbtwin_tbl[i] );
		// 念のため
		if( cnt >= WIN_RECORD_MAX_COUNT ){
			cnt = WIN_RECORD_MAX_COUNT;
		}
		wk->CountBuffer[i] = cnt;
//		wk->CountBuffer[i] = i;		// test
	}
	// 判定
	j=0;
#if 0
	// リストの処理でoffyが0以外だと
	// 正しく動作しないようだ(暫定処理)
	// 0番にダミーで表示させ
	// 上に黒帯を出して調整した
	wk->DispBuffer[j] = 0;		// 0番をダミーで登録
	j++;
#endif
	for(i=0;i<TMT_MAX;i++){
		// 表示の判定
		bflg = wr_chk_dispflag( wk, i );
		if(bflg == TRUE){
			// 表示
			wk->DispBuffer[j] = i;
			j++;
		}
	}
	wk->DispMaxNum = j;
	if(wk->DispMaxNum <= WIN_RECORD_DISP_PANEL_MAX){
		wk->Flag |= FLG_MW_NO_SCROLL;
	}
	
#if 0
	OS_Printf("boy :%d\n",wk->TrainerId);
	for(i=0;i<TMT_MAX;i++){
		OS_Printf("%d, %d: %d\n",i,wbtwin_tbl[i],wk->CountBuffer[i]);
	}
#endif
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アニメのセット
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32				(CMNの番号:フラグと対応)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_set_cmn_anim( WIN_RECORD_MAIN_WORK* wk, const s32 mode )
{
	s32		wnum;
	
	// アニメ再生
	wnum = mode;
	wr_chg_objseq( wk, CLWKMODE_CMN, wnum, wr_cmn_btn_tbl[wnum*4+1] );
	wk->Flag |= (1<<mode);
}

//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力まとめ
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_chk_input( WIN_RECORD_MAIN_WORK* wk )
{
	s32		flg=0;
	u32		ret;
	s32		listflg=0;
	BOOL	bflg;
	
	// アニメ待ち
	wr_chk_anim( wk );
	
	// リスト処理
	ret = wr_main_framelist( wk );
	listflg = wr_is_main_framelist( wk, ret );
	if(listflg != 0){
		flg = 1;
	}
	
	// 判定
	// (ほとんどがリストのシステムでやってくれる)
	if(flg == 0){
		// タッチペン
		bflg = wr_chk_tp_input( wk );
		if(bflg == FALSE){
			// キー
			wr_chk_key_input( wk );
		}
	}
	// スクロールバー
	wr_main_scrollbar( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力(上下)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval BOOL					(TRUE:入力あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL wr_chk_key_input( WIN_RECORD_MAIN_WORK* wk )
{
	s16		flg=0;
	BOOL	bflg=FALSE;
	
	// 終了判定(なくても大丈夫のはず)
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	// ×
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
		// アニメ再生
		wr_set_cmn_anim( wk, CLWKID_CMN_CROSS );
		flg = 1;
		// SE
		PMSND_PlaySE( SND_WIN_RECORD_CANCEL );
	}
	// 戻る
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
		// アニメ再生
		wr_set_cmn_anim( wk, CLWKID_CMN_RETURN );
		flg = 1;
		// SE
		PMSND_PlaySE( SND_WIN_RECORD_CANCEL );
	}
	//
	// 更新
	if(flg == 1){
		bflg = TRUE;
		wk->Step = STEP_LV2;
		// キーで終了
		wk->Flag |= FLG_MW_KEY_END;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン用データセット
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32				領域の番号
 *	@param	GFL_UI_TP_HITTBL*		タッチペン判定用
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_get_tp_area( WIN_RECORD_MAIN_WORK* wk, const s32 num, GFL_UI_TP_HITTBL *tpbuffer )
{
	s16		i;
	s16		buf[4];
	
	// 領域を取得
	// (左上(x,y)、サイズ(w,h)の4個で1つのデータ)
	for(i=0;i<4;i++){
		buf[i] = wk->ParamBuffer[4*num+i];
	}
	// ParamBufferは、(x,y,width,height)なので加算
	buf[2] += buf[0];
	buf[3] += buf[1];
	
	// 領域チェック
	for(i=0;i<4;i++){
		if(buf[i] < 0){
			buf[i] = 0;
		}
		// 判定はu8なのでmaxを255としておく
		if(buf[i] > 255){
			buf[i] = 255;
		}
	}
	// 領域セット
	tpbuffer[0].rect.left	= (u8)buf[0];
	tpbuffer[0].rect.top    = (u8)buf[1];
	tpbuffer[0].rect.right	= (u8)buf[2];
	tpbuffer[0].rect.bottom	= (u8)buf[3];
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン入力
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param
 *
 *	@retval BOOL					(TRUE:タッチあり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL wr_chk_tp_input( WIN_RECORD_MAIN_WORK* wk )
{
	s16		flg=0;
	const GFL_UI_TP_HITTBL	*adr = &wk->HitBuffer[0];
	const int ret  = GFL_UI_TP_HitTrg( adr );
	// なければ、TP_HIT_NONE
	BOOL	bflg=FALSE;
	
	// 終了判定(なくても大丈夫のはず)
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	
	// ×
	if(ret == AREA_TP_CROSS){
		// アニメ再生
		wr_set_cmn_anim( wk, CLWKID_CMN_CROSS );
		flg = 2;
		// SE
		PMSND_PlaySE( SND_WIN_RECORD_CANCEL );
	}
	// RETURN
	else if(ret == AREA_TP_RETURN){
		// アニメ再生
		wr_set_cmn_anim( wk, CLWKID_CMN_RETURN );
		flg = 2;
		// SE
		PMSND_PlaySE( SND_WIN_RECORD_CANCEL );
	}
	//
	// 更新
	if(flg != 0){
		bflg = TRUE;
		wk->Step = STEP_LV2;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アニメの終了判定(左右は別で管理)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_chk_anim( WIN_RECORD_MAIN_WORK* wk )
{
	s16		flg=0;
	s16		wnum=0;
	BOOL	bflg=FALSE;
	
	// ×アニメ再生中
	if(wk->Flag & FLG_MW_ANM_CROSS){
		wnum = CLWKID_CMN_CROSS;
		flg = 1;
	}
	// 戻るアニメ再生中
	else if(wk->Flag & FLG_MW_ANM_RETURN){
		wnum = CLWKID_CMN_RETURN;
		flg = 1;
	}
	// アニメ終了判定を行う
	if(flg == 1){
		bflg = wr_is_anime_end( wk, CLWKMODE_CMN, wnum );
		if(bflg == FALSE){
			// アニメの終了
			// とりあえず通常に戻しておく
			wr_chg_objseq( wk, CLWKMODE_CMN, wnum, wr_cmn_btn_tbl[wnum*4] );
			wk->Flag &= ~(1<<wnum);
			//
			// 戻るボタン(飛び先変更はある)
			if((wnum == CLWKID_CMN_CROSS)||(wnum == CLWKID_CMN_RETURN)){
				wk->Flag |= FLG_MW_END;
				//
				// ×ボタン終了
				if(wnum == CLWKID_CMN_CROSS){
					wk->Flag |= FLG_MW_CROSS_END;
				}
				// RETURNボタン終了
				else if(wnum == CLWKID_CMN_RETURN){
					wk->Flag |= FLG_MW_RET_END;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  表示の判定
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32				(番号)
 *
 *	@retval BOOL					(TRUE:変更あり、FALSE:なし)
 */
//-----------------------------------------------------------------------------
static BOOL wr_chk_dispflag( WIN_RECORD_MAIN_WORK* wk, const s32 mode )
{
	BOOL		bflg=FALSE;
	
	// フラグを返す
	if( wk->Param->open_flag[mode] == 1 ){
		bflg = TRUE;
	}
	//
	// 判定
	switch(mode){
		//
		// 初期(念のため)
		case TMT_HODOMOE:		// ホドモエ
		case TMT_RENTAL:		// レンタル
		case TMT_SHAFFLE:		// シャッフル
			bflg = TRUE;
			break;
		//
		default:
			break;
	}
	//
#ifdef PM_DEBUG
	// テスト
	if(wk->Mode == MODE_LV3){
		bflg = TRUE;
	}
#endif
	
	//
	return( bflg );
}

//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームリスト初期化
 *
 * @param	WIN_RECORD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void wr_init_framelist( WIN_RECORD_MAIN_WORK * wk )
{
	FRAMELIST_HEADER  header = wr_frame_list_header;
	ARCHANDLE * arc_handle;
	s32		i;
	//
	header.cbWork = wk;
	//
	// 破棄
	wr_exit_framelist( wk );
	//
	// 読み込み
	arc_handle = GFL_ARC_OpenDataHandle( ARCID_WIN_RECORD_GRA, wk->heapID );
	
	//----------------------------
	// オリジナルデータの読み込み
	// (パレットは元BGに含まれている)
	//----------------------------
	GFL_ARCHDL_UTIL_TransVramBgCharacter( arc_handle, NARC_win_record_gra_win_record_bg_d_NCGR, 	GFL_BG_FRAME2_M, 0, 0, 0, wk->heapID );
	
	//---------------
	// FrameListの設定
	//---------------
	// パラメータ設定
	header.cbFunc  		= &wr_list_callback;
	header.itemMax 		= wk->DispMaxNum;			// 重要
	header.initPos		= 0;
	header.initScroll	= 0;
	if(wk->DispMaxNum <= WIN_RECORD_DISP_PANEL_MAX){
		header.touch = wr_list_tp2_tbl;
	}
	
	// 生成
	wk->frame_list_work = FRAMELIST_Create( &header, wk->heapID );
	
	// 項目背景設定
	FRAMELIST_LoadFrameGraphicAH( wk->frame_list_work, arc_handle, NARC_win_record_gra_win_record_bg_d_03_NSCR, FALSE, 0 );		// 通常
	// 点滅アニメパレット設定
	// (互換性のため設定しておく)
	FRAMELIST_LoadBlinkPalette(   wk->frame_list_work, arc_handle, NARC_win_record_gra_win_record_bg_d_NCLR, 2, 2 );			// 大丈夫?
	//
	GFL_ARC_CloseDataHandle( arc_handle );
	//
	// 項目追加(タイトル文字をセット)
	for(i=0;i<header.itemMax;i++){
		FRAMELIST_AddItem( wk->frame_list_work, 0, wr_tmt_gmm_tbl[wk->DispBuffer[i]] );
	}
	// 初期化
	FRAMELIST_Init( wk->frame_list_work );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームMain
 *
 * @param	WIN_RECORD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static u32 wr_main_framelist( WIN_RECORD_MAIN_WORK * wk )
{
	u32		ret = FRAMELIST_RET_NONE;
	
	// 1回チェック済み=>リストの処理はしない
	if(wk->Flag & FLG_MW_1ST_CHK){
		return( ret );
	}
	
	// リスト処理をだます処理
	wr_main_etc_input( wk );
	
	// Main
	if( wk->frame_list_work ){
		// PAGEUP、PAGEDOWNの場合は、スクロールが終了時に、
		// パラメータをセットする必要がある(タイミング注意)
		ret = FRAMELIST_Main( wk->frame_list_work );
	}
	// スクロールなしの場合は、
	// リストは表示のみで、処理はしない
	if(wk->Flag & FLG_MW_NO_SCROLL){
		wk->Flag |= FLG_MW_1ST_CHK;
	}
	
#if 0	// debug
	if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_Y) ){		//
		u32		buf[3];
		
		buf[0] = FRAMELIST_GetListPos(     wk->frame_list_work );
		buf[1] = FRAMELIST_GetCursorPos(   wk->frame_list_work );
		buf[2] = FRAMELIST_GetScrollCount( wk->frame_list_work );
		//
		OS_Printf("%d,%d,%d\n",buf[0],buf[1],buf[2]);
	}
#endif
	return( ret );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームMainのchk
 *
 * @param	WIN_RECORD_MAIN_WORK*	wk
 *
 * @return  s32						(0:なし、1:カーソル移動、2:決定)
 */
//--------------------------------------------------------------------------------------------
static s32 wr_is_main_framelist( WIN_RECORD_MAIN_WORK * wk, const u32 mode )
{
	s16			flg=0;
	
	switch(mode){
		case 0:								// 項目をタッチ
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			flg = 2;
			break;
		//
		case FRAMELIST_RET_PAGE_UP:			// page up
		case FRAMELIST_RET_PAGE_UP_NONE:	// page up 2
			// アニメ再生
			wr_set_cmn_anim( wk, CLWKID_CMN_LEFT );
			flg = 1;
			wk->Step = STEP_LV1+20;
			break;
		case FRAMELIST_RET_PAGE_DOWN:		// page down
		case FRAMELIST_RET_PAGE_DOWN_NONE:	// page down 2
			// アニメ再生
			wr_set_cmn_anim( wk, CLWKID_CMN_RIGHT );
			flg = 1;
			wk->Step = STEP_LV1+10;
			break;
		//
		case FRAMELIST_RET_JUMP_TOP:		// top
		case FRAMELIST_RET_JUMP_BOTTOM:		// bottom
			flg = 1;
			break;
		//
		case FRAMELIST_RET_MOVE:			// move
		case FRAMELIST_RET_SCROLL:			// move
			flg = 1;
			break;
		default:
			break;
	}
	// リスト移動
	if(flg == 1){
	}
	// 決定
	else if(flg == 2){
//		// カーソル位置の取得
//		wr_calc_framelist( wk );
	}
	return( flg );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレーム破棄
 *
 * @param	WIN_RECORD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void wr_exit_framelist( WIN_RECORD_MAIN_WORK * wk )
{
	// 破棄
	if( wk->frame_list_work ){
		FRAMELIST_Exit( wk->frame_list_work );
		wk->frame_list_work = 0;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フレームのcallback関数(draw,move,scroll)
 *
 * @param	WIN_RECORD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
// draw
static void wr_listcallback_draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp )
{
	WIN_RECORD_MAIN_WORK * wk = (WIN_RECORD_MAIN_WORK *)work;
	// タイトル表示
	_liststr_print( wk, pos, print_util, wk->msg[0] );
}

// move
static void wr_listcallback_move(void *work, u32 pos, BOOL flag)
{
//	WIN_RECORD_MAIN_WORK * wk = (WIN_RECORD_MAIN_WORK *)work;
//	// 再計算
//	wr_calc_framelist( wk );
}

// scroll
static void wr_listcallback_scroll( void *work, s8 mv )
{
	WIN_RECORD_MAIN_WORK * wk = (WIN_RECORD_MAIN_WORK *)work;
	// 再計算
//	wr_chk_newmark( wk, 0 );
//	wr_scroll_newmark( wk, mv );
}

//----------------------------------------------------------------------------------
/**
 * @brief コールバック関数共通文字列描画処理
 *
 * @param   wk    
 * @param   pos   
 * @param   print_util    
 * @param   msg   
 */
//----------------------------------------------------------------------------------
static void _liststr_print( WIN_RECORD_MAIN_WORK *wk, int pos, PRINT_UTIL *print_util, GFL_MSGDATA *msg )
{
	PRINT_QUE *que = FRAMELIST_GetPrintQue( wk->frame_list_work );
	int param = FRAMELIST_GetItemParam( wk->frame_list_work, pos );
	//
	_strbuf_print( wk, print_util, que, msg, param, pos );
}

static void PrintStrBuf( WIN_RECORD_MAIN_WORK *wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int strIdx, PRINTSYS_LSB col, int y, int pos )
{
	int width, bmp_width, x;
	STRBUF* strbuf = GFL_MSG_CreateString( msg, strIdx );
	STRBUF* strbuf2;
	STRBUF* strbuf3;
	STRBUF* expandbuf;
	//
	// トーナメント名
	x = wr_mes_pos_tbl[0];
	PRINT_UTIL_PrintColor( print_util, que, x, y, strbuf, wk->font, col );
	GFL_STR_DeleteBuffer( strbuf );
	//
	// 回数表示
	strbuf2 = GFL_STR_CreateBuffer( 64, wk->heapID );
	STRTOOL_SetNumber( strbuf2, wk->CountBuffer[wk->DispBuffer[pos]], 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
//	STRTOOL_SetNumber( strbuf2, pos, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
	WORDSET_RegisterWord( wk->Wordset, 0, strbuf2, 0, TRUE, PM_LANG );
	// 最終出力バッファ
	expandbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
	strbuf3 = GFL_MSG_CreateString( wk->msg[0], wr_gmm_tbl[2] );
	WORDSET_ExpandStr( wk->Wordset, expandbuf, strbuf3 );
	//
	x = wr_mes_pos_tbl[1];
	PRINT_UTIL_PrintColor( print_util, que, x, y, expandbuf, wk->font, col );
	GFL_STR_DeleteBuffer( strbuf2 );
	GFL_STR_DeleteBuffer( strbuf3 );
	GFL_STR_DeleteBuffer( expandbuf );
}

static void _strbuf_print( WIN_RECORD_MAIN_WORK *wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int param, int pos )
{
	// 情報表示
	PrintStrBuf( wk, print_util, que, msg, param, PRINTSYS_MACRO_LSB(14,15,0), 4, pos );
}


//-----------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/**
 * @brief	スクロールバー処理
 *
 * @param	WIN_RECORD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void wr_main_scrollbar( WIN_RECORD_MAIN_WORK * wk )
{
	// 表示
	GFL_CLACTPOS cp_pos;
	
	// 念のため
	if(wk->Flag & FLG_MW_NO_SCROLL){
		return;
	}
	
	// 座標の取得
	GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[CLWKID_BAR], &cp_pos, CLSYS_DEFREND_MAIN );
	cp_pos.y = FRAMELIST_GetScrollBarPY2( wk->frame_list_work, cp_pos.y );
//	cp_pos.x = 240;		// test
	//
	// RAILの領域の(+-8補正:アイコンの半分の大きさ補正)
	if( cp_pos.y < 40 ){
		cp_pos.y = 40;
	}else if( cp_pos.y > 152 ){
		cp_pos.y = 152;
	}
	GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[CLWKID_BAR], &cp_pos, CLSYS_DEFREND_MAIN );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	背景BGスクロール
 *
 * @param	WIN_RECORD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void wr_main_bg_scroll( WIN_RECORD_MAIN_WORK * wk )
{
	// 背景スクロール
	int		offy;
	//
	// 0.25ずつ移動、32dotで戻し(0x100 = 1.0)
//	wk->ScrollOffy += 0x40;
//	if(wk->ScrollOffy >= 0x2000){
	wk->ScrollOffy += wr_scroll_tbl[0];
	if(wk->ScrollOffy >= wr_scroll_tbl[1]){
		wk->ScrollOffy = 0;
	}
	offy = (wk->ScrollOffy>>8)&0xff;
	// セット
	GFL_BG_SetScrollReq( GFL_BG_FRAME3_M, GFL_BG_SCROLL_Y_SET, -offy );
	GFL_BG_SetScrollReq( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, -offy );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	リスト処理用の補正
 *
 * @param	WIN_RECORD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void wr_main_etc_input( WIN_RECORD_MAIN_WORK * wk )
{
	u32		buf[2];
	int		kflg= GFL_UI_CheckTouchOrKey();
	//
	// スクロールなし
	if(wk->Flag & FLG_MW_NO_SCROLL){
		return;
	}
	//
	// タッチで終了の場合はキーで終了にしておく
	// 内部的には常にキーで終了とさせておく
	if(kflg == GFL_APP_END_TOUCH){
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
	}
	// 上下のボタンを押したらすぐに、
	// スクロールが開始するようにするための処理
	// =>(カーソル位置を両端に移動)
	// 上
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
		FRAMELIST_SetCursorPos( wk->frame_list_work, 0 );
	}
	// 下
	else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
		FRAMELIST_SetCursorPos( wk->frame_list_work, WIN_RECORD_DISP_PANEL_MAX-1 );
	}
}

