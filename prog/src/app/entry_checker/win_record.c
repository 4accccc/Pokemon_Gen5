//=============================================================================
/**
 *
 *	@file		win_record.c
 *	@brief  エントリーチェッカー(優勝記録)
 *	@author	tamada
 *	@date		2011.03.02
 *
 * 2011.03.02 ui_templateから派生、作成開始
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

//テクスチャをOAMに転送
#include "system/nsbtx_to_clwk.h"
#include "system/bmp_oam.h"
#include "system/bmp_winframe.h"

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
#include "gamesystem/msgspeed.h"

//アプリ共通素材
#include "app/app_menu_common.h"

#include "system/tr2dgra.h"

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"
#include "win_record_gra.naix"

#include "system/wipe.h"

#include "app/app_keycursor.h"
#include "app/app_printsys_common.h"

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
	STEP_LV7 	= 700,
	STEP_LV_MAX = 1000,
	//
	STEP_END	= 10000,
};

// モード
enum{
	MODE_LV1 = 0,
	MODE_LV2,
	MODE_LV3,
};

// パレット
enum{
	PLTID_BG_FONT				= 8,		// font
	//
	MES_CLEAR_COLOR				= 0,		// クリアカラー
	//
	BMPOAM_FONT_COLOR			= 4,		// BmpOamのカラー
};

// 最大値
enum{
	WIN_RECORD_CANVAS_MAXNUM 	= 1,		// canvas
	WIN_RECORD_BMPOAM_MAXNUM	= 8,		// bmpoam
	WIN_RECORD_EMPTY_MAXNUM		= 14*2,		// empty
	//
	WIN_RECORD_CMN_OBJ_MAXNUM	= 4,		// common(obj)
	WIN_RECORD_OBJ_MAXNUM		= 5,		// obj
	//
	WIN_RECORD_BGNUM_MAXNUM		= 8,		// bg
	//
	WIN_RECORD_UP_OBJ_MAXNUM	= 1,		// obj(up)
	//
	WIN_RECORD_MAX_COUNT		= 9999,		// 最大数
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
	CLWKID_PANEL_00 = 0,					// パネル(00)
	CLWKID_PANEL_01,						// パネル
	CLWKID_PANEL_02,						// パネル
	CLWKID_PANEL_03,						// パネル
	CLWKID_BAR,								// バー
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

// empty
enum{
	EMPTY_PANEL_OFFSET 	= 5,				// パネルのオフセット
	//
	EMPTY_PLAYER_NAME	= 11,
	EMPTY_TITLE_NAME	= 12,
	//
	EMPTY_SCROLL_POS	= 4,
};

// タッチペンエリア
enum{
	AREA_TP_LEFT = 0,
	AREA_TP_RIGHT,
	AREA_TP_CROSS,
	AREA_TP_RETURN,
	//
	AREA_TP_SCROLL,
	//
	AREA_TP_MAX,
};

// トーナメントの番号
enum{
	TMT_ALL 	= 0,		// オール
	TMT_CHAMPION,			// チャンピオン
	TMT_NORMAL,				// ノーマル
	TMT_FIRE,				// ほのお
	TMT_WATER,				// みず
	TMT_GRASS,				// くさ
	TMT_ELEC,				// でんき
	TMT_ICE,				// こおり
	TMT_BATTLE,				// かくとう
	TMT_POISON,				// どく
	//
	TMT_GROUND,				// じめん
	TMT_FLY,				// ひこう
	TMT_ESPER,				// エスパー
	TMT_INSECT,				// むし
	TMT_ROCK,				// いわ
	TMT_GHOST,				// ゴースト
	TMT_DRAGON,				// ドラゴン
	TMT_EVIL,				// あく
	TMT_IRON,				// はがね
	//
	TMT_MAX,
};

// CLWK(下画面共通)
enum{
	CLWKID_CMN_LEFT = 0,	// 左
	CLWKID_CMN_RIGHT,		// 右
	CLWKID_CMN_CROSS,		// ×
	CLWKID_CMN_RETURN,		// 戻る
	//
	CLWKID_CMN_MAX,
};

// シーケンス
enum{
	CMN_LEFT_SEQ 		= 4,	// 左
	CMN_RIGHT_SEQ 		= 5,	// 右
	CMN_CROSS_SEQ 		= 0,	// ×
	CMN_RETURN_SEQ 		= 1,	// 戻る
	//
	CMN_LEFT_ON_SEQ 	= 12,	// 左オン
	CMN_LEFT_OFF_SEQ 	= 18,	// 左オフ
	CMN_RIGHT_ON_SEQ 	= 13,	// 右オン
	CMN_RIGHT_OFF_SEQ 	= 19,	// 右オフ
	//
	CMN_CROSS_ON_SEQ	= 8,	// ×オン
	CMN_RETURN_ON_SEQ	= 9,	// 戻るオン
};

// CLWK(up)
enum{
	CLWKID_UP_TRAINER = 0,		// トレーナー
	//
	CLWKID_UP_MAX,
};

//-------------------
// フラグ
//-------------------
// 移動のフラグ(MoveWork.flag)
enum{
	FLG_MOVE_INIT		= (1<<0),		// 初期化
	FLG_MOVE_END		= (1<<1),		// 終了
};

// OBJのフラグ(WIN_RECORD_OBJ_DATA.dispflag)
enum{
	FLG_OBJ_ANIM		= (1<<0),		// アニメあり
	FLG_OBJ_HFLIP		= (1<<1),		// HFLIP
	FLG_OBJ_VFLIP		= (1<<2),		// VFLIP
};

// WIN_RECORD_MAIN_WORK.Flag
enum{
	FLG_MW_ANM_LEFT		= (1<<0),		// 左矢印アニメ
	FLG_MW_ANM_RIGHT	= (1<<1),		// 右矢印アニメ
	FLG_MW_ANM_CROSS	= (1<<2),		// ×アニメ
	FLG_MW_ANM_RETURN	= (1<<3),		// 戻るアニメ
	//
	FLG_MW_UPDATE		= (1<<4),		// 更新
	FLG_MW_BAR_UPDATE	= (1<<5),		// バー更新
	FLG_MW_NO_SCROLL	= (1<<6),		// スクロールなし(MODE_LV1と同じ)
	//
	FLG_MW_RET_END		= (1<<8),		// RETURNで終了
	FLG_MW_CROSS_END	= (1<<9),		// ×で終了
	//
	FLG_MW_END			= (1<<12),		// 終了
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
	GFL_MSGDATA				*msg[2];								// 0:メッセージ,1:未使用
	PRINT_UTIL				PrintUtil[WIN_RECORD_CANVAS_MAXNUM];	// 文字表示用
	//
	BMPOAM_SYS_PTR			BmpOamSys;								// BMPOAM
	GFL_BMP_DATA			*BmpData[WIN_RECORD_BMPOAM_MAXNUM];		// Bmp
	BMPOAM_ACT_PTR			BmpOamWork[WIN_RECORD_BMPOAM_MAXNUM];	// BMPOAM用Work
	//
	s32						Step;							// ステップ
	u32						DebugFlag;						// debug
	s32						Mode;							// モード(0-2)
	s32						TopNum;							// 先頭の番号
	s32						MaxNum;							// 個数
	u32						Flag;							// フラグ
	s32						TrainerId;						// トレーナーID
	s32						CountBuffer[TMT_MAX];			// カウント
	//
	s16						*EmptyBuffer;					// 座標用のバッファ(必ず破棄すること)
	s16						*ParamBuffer;					// パラメータのバッファ(必ず破棄すること)
	GFL_UI_TP_HITTBL		HitBuffer[AREA_TP_MAX+1];		// タッチペン用
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
// BMPOAMの定義
//------------------------------------------
typedef struct{
	u8				width;			// サイズ(8x8単位)(0xffの場合は終了判定)
	u8				height;
	u8				softpri;		// プライオリティ(obj)
	u8				bgpri;			// プライオリティ(bg)
	u8				visible;		// 表示/非表示
	u8				paloffset;		// パレットオフセット(nclの何番目か)
	u8				empty_num;		// empty番号
	u8				pad;
}WIN_RECORD_BMPOAM_DATA;

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
	{ PALTYPE_MAIN_BG,	NARC_win_record_gra_win_record_bg_d_NCLR,	0,	2,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};

// BGCHR
static const WIN_RECORD_BGCHR_DATA wr_bgchr_tbl[] = {
	{ GFL_BG_FRAME2_S,	NARC_win_record_gra_win_record_bg_u_NCGR,	0,	0,},		// 上、BG
	{ GFL_BG_FRAME3_M,	NARC_win_record_gra_win_record_bg_d_NCGR,	0,	0,},		// 下、BG

	{ 0xff, 0, 0, 0,}, // end code
};

// BGSCN
static const WIN_RECORD_BGSCN_DATA wr_bgscn_tbl[] = {
	{ GFL_BG_FRAME2_S,	NARC_win_record_gra_win_record_bg_u_01_NSCR,	0,	0,},		// 上、下地
	{ GFL_BG_FRAME1_S,	NARC_win_record_gra_win_record_bg_u_02_NSCR,	0,	0,},		// 上、中間
	{ GFL_BG_FRAME3_M,	NARC_win_record_gra_win_record_bg_d_01_NSCR,	0,	0,},		// 下、下地
	{ GFL_BG_FRAME2_M,	NARC_win_record_gra_win_record_bg_d_02_NSCR,	0,	0,},		// 下、下地

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
	0,
};

// OBJのテーブルデータ(通常)
static const WIN_RECORD_OBJ_DATA wr_obj_tbl[] = {
	{ 0,	0x30,	2,	1,	0,	1,	0,	0,},		// パネル0
	{ 1,	0x30,	2,	1,	0,	1,	1,	0,},		// パネル1
	{ 2,	0x30,	2,	1,	0,	1,	2,	0,},		// パネル2
	{ 3,	0x30,	2,	1,	0,	1,	3,	0,},		// パネル3
	{ 19,	0x30,	2,	1,	0,	1,	4,	0,},		// ゲージ上

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
	1,
};

// OBJのテーブルデータ(cmn)
static const WIN_RECORD_OBJ_DATA wr_obj_cmn_tbl[] = {
	{ 4,	0x30,	2,	1,	0,	1,	7,	0,},		// 左マーク
	{ 5,	0x30,	2,	1,	0,	1,	8,	0,},		// 右マーク
	{ 0,	0x30,	2,	1,	0,	1,	9,	0,},		// ×マーク
	{ 1,	0x30,	2,	1,	0,	1, 10,	0,},		// 戻るマーク

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
	2,
};

// OBJのテーブルデータ(up)
static const WIN_RECORD_OBJ_DATA wr_obj_up_tbl[] = {
	{ 0,	0x30,	2,	1,	0,	1,	13,	0,},		// トレーナー

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//------------------
// BMPOAMの情報
//------------------
static const WIN_RECORD_BMPOAM_DATA wr_bmpoam_tbl[] = {
	{ 12,	2,	0x38,	1,	1,	4,	5,	0,},		// パネル00(チャンピオン名)
	{ 12,	2,	0x38,	1,	1,	4,	6,	0,},		// パネル00(回数)
	{ 12,	2,	0x38,	1,	1,	4,	5,	0,},		// パネル01(チャンピオン名)
	{ 12,	2,	0x38,	1,	1,	4,	6,	0,},		// パネル01(回数)
	{ 12,	2,	0x38,	1,	1,	4,	5,	0,},		// パネル02(チャンピオン名)
	{ 12,	2,	0x38,	1,	1,	4,	6,	0,},		// パネル02(回数)
	{ 12,	2,	0x38,	1,	1,	4,	5,	0,},		// パネル03(チャンピオン名)
	{ 12,	2,	0x38,	1,	1,	4,	6,	0,},		// パネル03(回数)

	{ 0xff, 0, 0, 0, 0, 0, 0, 0,}, // end code
};

//---------------
// CANVASの設定
//---------------
// (printutilと対応)
static const WIN_RECORD_CANVAS_DATA wr_canvas_tbl[] = {
	{ GFL_BG_FRAME0_M,	1,	1,	30,	2,	0,},		// 上名前

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
static const u8 wr_tmt_gmm_tbl[] = {
	msg_wbt_wincount_all,			// 0
	msg_wbt_wincount_champ,
	msg_wbt_wincount_normal,
	msg_wbt_wincount_honoo,
	msg_wbt_wincount_mizu,
	msg_wbt_wincount_kusa,
	msg_wbt_wincount_denki,
	msg_wbt_wincount_koori,
	msg_wbt_wincount_kakutou,
	msg_wbt_wincount_doku,
	//
	msg_wbt_wincount_zimen,
	msg_wbt_wincount_hikou,
	msg_wbt_wincount_esper,
	msg_wbt_wincount_musi,
	msg_wbt_wincount_iwa,
	msg_wbt_wincount_gorst,
	msg_wbt_wincount_dragon,
	msg_wbt_wincount_aku,
	msg_wbt_wincount_hagane,
	msg_wbt_wincount_hagane,		// pad
};

// シナリオクリア前
static const u8 wr_gmm_00_tbl[] = {
	TMT_ALL,
	0xff,
};

// シナリオクリア後
static const u8 wr_gmm_01_tbl[] = {
	TMT_ALL,
	TMT_NORMAL,
	TMT_FIRE,
	TMT_WATER,
	TMT_GRASS,
	TMT_ELEC,
	TMT_ICE,
	TMT_BATTLE,
	TMT_POISON,
	TMT_GROUND,
	//
	TMT_FLY,
	TMT_ESPER,
	TMT_INSECT,
	TMT_ROCK,
	TMT_GHOST,
	TMT_DRAGON,
	TMT_EVIL,
	TMT_IRON,
	0xff,
	0xff,
};

// シナリオクリアかつチャンピオン後
static const u8 wr_gmm_02_tbl[] = {
	TMT_ALL,
	TMT_CHAMPION,
	TMT_NORMAL,
	TMT_FIRE,
	TMT_WATER,
	TMT_GRASS,
	TMT_ELEC,
	TMT_ICE,
	TMT_BATTLE,
	TMT_POISON,
	//
	TMT_GROUND,
	TMT_FLY,
	TMT_ESPER,
	TMT_INSECT,
	TMT_ROCK,
	TMT_GHOST,
	TMT_DRAGON,
	TMT_EVIL,
	TMT_IRON,
	0xff,
};

//----------------
// まとめテーブル
// (Modeで参照)
//----------------
static const u8 *wr_gmm_all_tbl[] = {
	wr_gmm_00_tbl,
	wr_gmm_01_tbl,
	wr_gmm_02_tbl,
};

// 個数最大(要調整)
static const s16 wr_gmm_max_tbl[] = {
	1,			// シナリオクリア前
	18,			// シナリオクリア後
	19,			// シナリオクリア&チャンピオン後
	-1,			// pad
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

//-------------------------
// 優勝回数の対応状況
// (ここでの並びとの対応)
//-------------------------
static const u8 wbtwin_tbl[] = {
	WBTWIN_COUNT_ALLTOURNAMENT,		// オール
	WBTWIN_COUNT_CHAMPION,			// チャンピオン
	WBTWIN_COUNT_POKETYPE_NORMAL,	// ノーマル
	WBTWIN_COUNT_POKETYPE_HONOO,	// ほのお
	WBTWIN_COUNT_POKETYPE_MIZU,		// みず
	WBTWIN_COUNT_POKETYPE_KUSA,		// くさ
	WBTWIN_COUNT_POKETYPE_DENKI,	// でんき
	WBTWIN_COUNT_POKETYPE_KOORI,	// こおり
	WBTWIN_COUNT_POKETYPE_KAKUTOU,	// かくとう
	WBTWIN_COUNT_POKETYPE_DOKU,		// どく
	//
	WBTWIN_COUNT_POKETYPE_JIMEN,	// じめん
	WBTWIN_COUNT_POKETYPE_HIKOU,	// ひこう
	WBTWIN_COUNT_POKETYPE_ESPER,	// エスパー
	WBTWIN_COUNT_POKETYPE_MUSHI,	// むし
	WBTWIN_COUNT_POKETYPE_IWA,		// いわ
	WBTWIN_COUNT_POKETYPE_GHOST,	// ゴースト
	WBTWIN_COUNT_POKETYPE_DRAGON,	// ドラゴン
	WBTWIN_COUNT_POKETYPE_AKU,		// あく
	WBTWIN_COUNT_POKETYPE_HAGANE,	// はがね
	WBTWIN_COUNT_POKETYPE_NORMAL,	// pad
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
static void wr_up(   WIN_RECORD_MAIN_WORK* wk );
static void wr_down( WIN_RECORD_MAIN_WORK* wk );
static void wr_key_all( WIN_RECORD_MAIN_WORK* wk );
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
static void wr_init_obj_bmpoam( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const WIN_RECORD_BMPOAM_DATA *bmp_adr, const s16 *info_adr );
static void wr_disp_bmpoam( WIN_RECORD_MAIN_WORK* wk, const s32 wnum, const STRBUF *strbuf );
//
static void wr_init_string( WIN_RECORD_MAIN_WORK* wk );
static void wr_disp_string( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 x, const s32 y, const STRBUF* strbuf, const u32 dispmode );
static void wr_update_string( WIN_RECORD_MAIN_WORK* wk, const s32 mode );
static void wr_exit_string( WIN_RECORD_MAIN_WORK* wk );
static void wr_clear_canvas( WIN_RECORD_MAIN_WORK* wk, const s32 mode );
static void wr_disp_playername( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 xpos, const s32 ypos, const u32 dispmode );
static void wr_disp_string_id( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 mesid, const s32 xpos, const s32 ypos, const u32 dispmode );
//
static void wr_set_panel( WIN_RECORD_MAIN_WORK* wk, const s32 wnum, const s32 num, const BOOL bflg );
static void wr_set_panel_all( WIN_RECORD_MAIN_WORK* wk, const s32 wnum, const s32 num, const BOOL dispflg );
//
static void wr_init_param( WIN_RECORD_MAIN_WORK* wk );
static void wr_set_param( WIN_RECORD_MAIN_WORK* wk );
//
static BOOL wr_chk_key_input( WIN_RECORD_MAIN_WORK* wk );
//
static void wr_get_tp_area( WIN_RECORD_MAIN_WORK* wk, const s32 num, GFL_UI_TP_HITTBL *tpbuffer );
static BOOL wr_chk_tp_input( WIN_RECORD_MAIN_WORK* wk );
static void wr_chk_anim( WIN_RECORD_MAIN_WORK* wk );
//
static BOOL wr_set_scrollbar( WIN_RECORD_MAIN_WORK* wk );
static void wr_param_update( WIN_RECORD_MAIN_WORK* wk );
//
static void wr_debug( WIN_RECORD_MAIN_WORK* wk );

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
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_UI_DEBUG, UI_TEMPLATE_HEAP_SIZE );
	wk = GFL_PROC_AllocWork( proc, sizeof(WIN_RECORD_MAIN_WORK), HEAPID_UI_DEBUG );
	GFL_STD_MemClear( wk, sizeof(WIN_RECORD_MAIN_WORK) );
	
	// 初期化
	wk->heapID = HEAPID_UI_DEBUG;
	
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
	WIN_RECORD_MAIN_WORK* wk = mywk;
	
	//----------------
	// 処理を記述
	//----------------
	wr_exit( wk );
	
	//メッセージ破棄
	if(wk->msg[0]){
		GFL_MSG_Delete( wk->msg[0] );
	}
	if(wk->msg[1]){
		GFL_MSG_Delete( wk->msg[1] );
	}
	
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
	PRINTSYS_QUE_Main( wk->print_que );
	
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
	s16			buf[2];
	s32			i;
	
	// 初期化
	// 上、下画面をまとめて処理
	wk->Step 		= STEP_INIT;
	wk->DebugFlag	= 0;
	wk->Mode		= wk->Param->status;	// 外部から
	wk->TopNum		= 0;
	wk->Flag		= 0;
	wk->TrainerId	= 0;					// トレーナーID
	//
	// カウント表示用バッファ
	for(i=0;i<TMT_MAX;i++){
		wk->CountBuffer[i] = i;
	}
	// empty用バッファ取得(必ず破棄すること)
	wk->EmptyBuffer = GFL_ARC_LoadDataAlloc( ARCID_WIN_RECORD_GRA, NARC_win_record_gra_win_record_pos_tblbin, wk->heapID );
	// パラメータ用のバッファ取得(必ず破棄すること)
	wk->ParamBuffer = GFL_ARC_LoadDataAlloc( ARCID_WIN_RECORD_GRA, NARC_win_record_gra_win_record_param_tblbin, wk->heapID );
	//
	//------------------
	// パラメータセット
	//------------------
	wr_set_param( wk );
	
	//---------------
	// bgの初期化
	//---------------
	wr_init_bg( wk );
	
	//---------------
	// 文字列関連の初期化
	//---------------
	
	// パレットは8番に転送
	{
		ARCHANDLE * p_handle  = GFL_ARC_OpenDataHandle( ARCID_FONT, wk->heapID );
		//上画面フォントパレット
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_font_talkwin_nclr, PALTYPE_MAIN_BG, PLTID_BG_FONT*0x20, 0x20, wk->heapID );
		GFL_ARC_CloseDataHandle( p_handle );
	}
	
	// 文字初期化
	wr_init_string( wk );
	
	//------------
	// 文字列表示
	//------------
	// プレイヤ名
	buf[0] = wr_get_empty( wk, EMPTY_PLAYER_NAME*2 );
	buf[1] = wr_get_empty( wk, EMPTY_PLAYER_NAME*2+1 );
	wr_disp_playername( wk, 0, buf[0], buf[1], PRINTTOOL_MODE_LEFT );
	// ゆうしょう回数
	buf[0] = wr_get_empty( wk, EMPTY_TITLE_NAME*2 );
	buf[1] = wr_get_empty( wk, EMPTY_TITLE_NAME*2+1 );
	wr_disp_string_id( wk, 0, wr_gmm_tbl[1], buf[0], buf[1], PRINTTOOL_MODE_RIGHT );
	// 更新
	wr_update_string( wk, 0 );
	//
	//------------
	// OBJ初期化
	//------------
	wr_init_obj( wk );
	
	//-----------------
	// パラメータ関連
	//-----------------
	wr_init_param( wk );
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
	// debug
	wr_debug( wk );
	
	// 上画面
	wr_up( wk );
	// 下画面
	wr_down( wk );
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
		wk->Param->result = TRUE;
	}
	// RETURNで終了 => パソコン画面へ
	else{
		wk->Param->result = FALSE;
	}
#if 1	// 結果
	OS_Printf("result : %d\n",wk->Param->result);
#endif
	// 上、下画面をまとめて処理
	// emptyのバッファクリア
	GFL_HEAP_FreeMemory( wk->EmptyBuffer );
	// paramのバッファクリア
	GFL_HEAP_FreeMemory( wk->ParamBuffer );
	
	// 文字表示用
	wr_exit_string( wk );
	// OBJ破棄
	wr_exit_obj( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  処理の流れ(main)(上画面)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_up( WIN_RECORD_MAIN_WORK* wk )
{
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
	BOOL	bflg;
	
	// 処理
	switch(wk->Step)
	{
		// 初期化
		case STEP_INIT:
			wk->Step = STEP_LV1;
			break;
		
		// LV1
		case STEP_LV1:
			// アニメ待ち
			wr_chk_anim( wk );
			//
			// キー入力
			bflg = wr_chk_key_input( wk );
			if(bflg == FALSE){
				// ないときだけタッチペン
				wr_chk_tp_input( wk );
			}
			// パラメータ更新
			wr_param_update( wk );
			break;
		
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
	s32		i;
	s32		tmp;
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
		GFL_ARCHDL_UTIL_TransVramPalette( handle, padr->arcid, padr->type, padr->offset, tmp, wk->heapID );
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
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OBJ初期化(clwk)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const WIN_RECORD_OBJ_DATA* アドレス
 *	@param	const s16*				アドレス
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
	wk->wk_obj.res[ RESID_CMN_OBJ_PLT ] = GFL_CLGRP_PLTT_RegisterEx(   p_handle, idx[0], drawtype, 0x20*12, 0, 4, wk->heapID );
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
	// BmpOAM初期化
	wk->BmpOamSys = BmpOam_Init( wk->heapID, p_clunit );
	// BMPOAM登録
	wr_init_obj_bmpoam( wk, CLSYS_DRAW_MAIN, &wr_bmpoam_tbl[0], &wr_obj_main_info_tbl[0] );
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
	//
	// BmpOAM後処理
	for(i=0;i<WIN_RECORD_BMPOAM_MAXNUM;i++){
		BmpOam_ActorDel( wk->BmpOamWork[i] );
		GFL_BMP_Delete( wk->BmpData[i] );
	}
	BmpOam_Exit( wk->BmpOamSys );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  objのシーケンスの切り替え
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			(0:通常、1:共通)
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
 *	@param	const s32			(0:通常、1:共通)
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
 *	@param	const s32			(0:通常、1:共通)
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
 *	@brief  OBJ初期化(bmpoam)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32				drawtype
 *	@param	const WIN_RECORD_BMPOAM_DATA*	アドレス
 *	@param	const s16*					アドレス
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_init_obj_bmpoam( WIN_RECORD_MAIN_WORK* wk, const s32 drawtype, const WIN_RECORD_BMPOAM_DATA *bmp_adr, const s16 *info_adr )
{
	s16							tmp;
	s32							i;
	s32							buf[2];
	BMPOAM_ACT_DATA				actdata;
	const WIN_RECORD_BMPOAM_DATA	*adr  = bmp_adr;
	const s16					*iadr = info_adr;
	GFL_CLACTPOS				cpos;
	//
	for(i=0;i<WIN_RECORD_BMPOAM_MAXNUM;i++){
		// 終了判定
		if(adr->width == 0xff){
			break;
		}
		// サイズ(colormodeは16で大丈夫だろう)
		buf[0] = (s32)(adr->width);
		buf[1] = (s32)(adr->height);
		wk->BmpData[i] = GFL_BMP_Create( buf[0], buf[1], GFL_BMP_16_COLOR, wk->heapID );
		// 左側
		tmp = adr->empty_num;
		cpos.x = wr_get_empty( wk, tmp*2 );
		cpos.y = wr_get_empty( wk, tmp*2+1 );
		//
		GFL_STD_MemClear( &actdata, sizeof(BMPOAM_ACT_DATA) );
		actdata.bmp			= wk->BmpData[i];
		actdata.x 			= cpos.x;
		actdata.y 			= cpos.y;
		actdata.pltt_index	= wk->wk_obj.res[ iadr[0+1] ];		// plt
		actdata.soft_pri	= adr->softpri;
		actdata.bg_pri		= adr->bgpri;
		actdata.pal_offset	= adr->paloffset;
		actdata.setSerface	= drawtype;
		actdata.draw_type	= drawtype;		// ?
		// 作成
		wk->BmpOamWork[i]	= BmpOam_ActorAdd( wk->BmpOamSys, &actdata );
		// visible
		if(adr->visible == 0){
			BmpOam_ActorSetDrawEnable( wk->BmpOamWork[i], FALSE );
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  OB文字列表示(bmpoam)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32				ワーク番号
 *	@param	const STRBUF*			表示文字列
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_disp_bmpoam( WIN_RECORD_MAIN_WORK* wk, const s32 wnum, const STRBUF *strbuf )
{
	if(wnum >= WIN_RECORD_BMPOAM_MAXNUM){
		OS_Printf("bmpoamj max error:%d\n",wnum);
		return;
	}
	GFL_BMP_Clear( wk->BmpData[wnum], 0 );
	PRINTSYS_PrintQueColor( wk->print_que, wk->BmpData[wnum], 0, 0, strbuf, wk->font, PRINTSYS_LSB_Make(15,14,0) );
	BmpOam_ActorBmpTrans( wk->BmpOamWork[wnum] );
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
	s32		i;
	const WIN_RECORD_CANVAS_DATA	*adr = &wr_canvas_tbl[0];
	
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
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示
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
//				PRINTSYS_LSB_Make(15,14,0),		// カラー
				PRINTSYS_LSB_Make(9,8,0),		// カラー
				dispmode );						// 位置
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字表示準備(現在、Canvasの作成)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32				Canvasの番号
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_update_string( WIN_RECORD_MAIN_WORK* wk, const s32 mode )
{
	// 転送
	GFL_BMPWIN * bmpwin = wk->PrintUtil[mode].win;
	//
	// これは入れないとだめらしい
	PRINT_UTIL_Trans( &wk->PrintUtil[mode], wk->print_que );
	//
	GFL_BMPWIN_TransVramCharacter( bmpwin );					//transfer characters
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame( bmpwin ) );	//transfer screen
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
 *	@brief  プレイヤ表示
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const mode				(PrintUtil[mode]のmode)
 *	@param	const s32				x座標
 *	@param	const s32				y座標
 *	@param	const u32				表示位置(左、中央、右)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_disp_playername( WIN_RECORD_MAIN_WORK* wk, const s32 mode, const s32 xpos, const s32 ypos, const u32 dispmode )
{
#if 1
	GFL_BMPWIN 	*bmpwin		= wk->PrintUtil[mode].win;
	// 名前取得用
	GAMEDATA 	*gamedata 	= GAMESYSTEM_GetGameData( wk->Param->gamesys );
	const MYSTATUS *mystat	= GAMEDATA_GetMyStatus( gamedata );
	WORDSET		*wordset	= WORDSET_Create( wk->heapID );
	STRBUF 		*strbuf		= GFL_STR_CreateBuffer( 64, wk->heapID );	// src
	STRBUF		*strbuf2;
	STRBUF 		*destbuf;
	//
	// プレイヤの名前(strbuf)をwordsetに登録
	MyStatus_CopyNameString( mystat, strbuf );
	WORDSET_RegisterWord( wordset, 0, strbuf, 0, TRUE, PM_LANG );
	// 最終セットバッファ
	destbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
	// 文字列
	strbuf2 = GFL_MSG_CreateString( wk->msg[0], wr_gmm_tbl[0] );
	// パラメータを展開しMsgBufferに再セット
	WORDSET_ExpandStr( wordset, destbuf, strbuf2 );
	// 表示
	wr_disp_string( wk, mode, xpos, ypos, destbuf, dispmode );
	//
	// WORDSET破棄
	WORDSET_Delete( wordset );
	// 破棄
	GFL_STR_DeleteBuffer( strbuf );
	GFL_STR_DeleteBuffer( strbuf2 );
	GFL_STR_DeleteBuffer( destbuf );
#else
	GFL_BMPWIN 	*bmpwin	= wk->PrintUtil[mode].win;
	STRBUF 	*strbuf		= GFL_STR_CreateBuffer( 64, wk->heapID );
	
	// 名前取得
	GAMEDATA 	*gamedata 	= GAMESYSTEM_GetGameData( wk->Param->gamesys );
	const MYSTATUS *mystat	= GAMEDATA_GetMyStatus( gamedata );
	MyStatus_CopyNameString( mystat, strbuf );
	wr_disp_string( wk, mode, xpos, ypos, strbuf, dispmode );
	// 破棄
	GFL_STR_DeleteBuffer( strbuf );
#endif
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字列表示
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
//-----------------------------------------------------------------------------------------------------------------
/**
 *	@brief  パネル表示まとめ
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			ワーク番号
 *	@param	const s32			直接シーケンス番号(ならびに対応)
 *	@param	const BOOL			(TRUE:表示、FALSE:非表示)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_set_panel( WIN_RECORD_MAIN_WORK* wk, const s32 wnum, const s32 num, const BOOL bflg )
{
	s16				tmp;
	s16				buf[4];
	GFL_CLACTPOS	pos;
	STRBUF			*strbuf;
	STRBUF			*expandbuf;
	STRBUF			*strbuf2;
	STRBUF			*strbuf3;
	WORDSET			*wordset;
	
	// 表示/非表示
	wr_set_obj_visible( wk, CLWKMODE_NORMAL, wnum, bflg );
	// bmpoam
	BmpOam_ActorSetDrawEnable( wk->BmpOamWork[2*wnum], bflg );
	
	// 非表示の時はそのままで終了
	if(bflg == FALSE){
		return;
	}
	// wordset取得
	wordset	= WORDSET_Create( wk->heapID );
	
	// シーケンスの変更
	wr_chg_objseq( wk, CLWKMODE_NORMAL, wnum, num );
	
	//-------------------------------------
	// 対応するbmpoamも文字列は固定なので
	// ここでセット
	//-------------------------------------
	// メッセージバッファセット
	tmp = wr_tmt_gmm_tbl[num];
	strbuf = GFL_MSG_CreateString( wk->msg[0], tmp );
	// 表示
	wr_disp_bmpoam( wk, 2*wnum, strbuf );
	
	//----------
	// 数字表示
	//----------
	strbuf2 = GFL_STR_CreateBuffer( 64, wk->heapID );
	STRTOOL_SetNumber( strbuf2, wk->CountBuffer[num], 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
//	STRTOOL_SetNumber( strbuf2, wk->CountBuffer[num], 4, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
	WORDSET_RegisterWord( wordset, 0, strbuf2, 0, TRUE, PM_LANG );
	// 最終出力バッファ
	expandbuf = GFL_STR_CreateBuffer( 64, wk->heapID );
	// 文字列
	strbuf3 = GFL_MSG_CreateString( wk->msg[0], wr_gmm_tbl[2] );
	// パラメータを展開しMsgBufferに再セット
	WORDSET_ExpandStr( wordset, expandbuf, strbuf3 );
	// 表示
	wr_disp_bmpoam( wk, 2*wnum+1, expandbuf );
	//
	// bmpoamは座標も念のため再セットしておく
	buf[0] = wr_get_empty( wk,  EMPTY_PANEL_OFFSET*2 );
	buf[1] = wr_get_empty( wk,  EMPTY_PANEL_OFFSET*2+1 );
	buf[2] = wr_get_empty( wk, (EMPTY_PANEL_OFFSET+1)*2 );
	buf[3] = wr_get_empty( wk, (EMPTY_PANEL_OFFSET+1)*2+1 );
	//
	// 座標の取得
	GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[wnum], &pos, CLSYS_DEFREND_MAIN );
	//
	// 座標セット
	// (左側)
	BmpOam_ActorSetPos( wk->BmpOamWork[2*wnum],   (pos.x+buf[0]), (pos.y+buf[1]) );
	// (右側)
	BmpOam_ActorSetPos( wk->BmpOamWork[2*wnum+1], (pos.x+buf[2]), (pos.y+buf[3]) );
	//
	// WORDSET破棄
	WORDSET_Delete( wordset );
	// 破棄
	GFL_STR_DeleteBuffer( strbuf );
	GFL_STR_DeleteBuffer( strbuf2 );
	GFL_STR_DeleteBuffer( strbuf3 );
	GFL_STR_DeleteBuffer( expandbuf );
}

//-----------------------------------------------------------------------------------------------------------------
/**
 *	@brief  パネル表示まとめ(呼び出し側)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32			ワーク番号
 *	@param	const s32			テーブルの何番目か(注意:直接シーケンス番号ではない)
 *	@param	const BOOL			表示フラグ(通常:TRUE、強制非表示:FALSE)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_set_panel_all( WIN_RECORD_MAIN_WORK* wk, const s32 wnum, const s32 num, const BOOL dispflg )
{
	s16				tmp=0;
	BOOL			bflg=FALSE;
	const u8		*adr = wr_gmm_all_tbl[wk->Mode];
	
	// 範囲のチェック
	if((wr_gmm_max_tbl[wk->Mode] <= num)||(num < 0)){
		bflg = FALSE;
	}else{
		// テーブルから変更するシーケンス番号を求める
		tmp = adr[num];
		bflg = TRUE;
	}
	// 強制フラグ
	if(dispflg == FALSE){
		bflg = FALSE;
	}
	// 表示
	wr_set_panel( wk, wnum, tmp, bflg );
}

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
	s16					num;
	s16					nmax;
	s16					i;
	GFL_UI_TP_HITTBL	*adr = &wk->HitBuffer[0];
	BOOL				bflg;
	//
	// 最大数を調べる
	num = 4;			// 表示パネル数
	nmax = wr_gmm_max_tbl[wk->Mode];
	if(nmax < 4){
		num = nmax;
	}
	
	// 表示/非表示と座標セット
	for(i=0;i<4;i++){
		bflg = TRUE;
		// チェック
		if(i >= num){
			bflg = FALSE;
		}
		wr_set_panel_all( wk, i, wk->TopNum+i, bflg );
	}
	//
	// タッチペン領域計算
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
	// フラグも用意しておく
	if(wk->Mode == MODE_LV1){
		wk->Flag |= FLG_MW_NO_SCROLL;
	}
	//
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
	u32		tmp;
	
	// 性別を取得
	tmp = MyStatus_GetMySex( wk->Param->my_status );
	wk->TrainerId = 0;			// 男性
	if(tmp == PM_FEMALE){
		wk->TrainerId = 1;		// 女性
	}
	//
	// 優勝回数
	for(i=0;i<TMT_MAX;i++){
		cnt = WBTSAVE_GetWinCount( wk->Param->wbtsave, wbtwin_tbl[i] );
		// 念のため
		if( cnt >= WIN_RECORD_MAX_COUNT ){
			cnt = WIN_RECORD_MAX_COUNT;
		}
		wk->CountBuffer[i] = cnt;
	}
#if 0
	OS_Printf("boy :%d\n",wk->TrainerId);
	for(i=0;i<TMT_MAX;i++){
		OS_Printf("%d, %d: %d\n",i,wbtwin_tbl[i],wk->CountBuffer[i]);
	}
#endif
}

//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *	@brief  キー入力(上下)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL wr_chk_key_input( WIN_RECORD_MAIN_WORK* wk )
{
	s16		i;
	s16		flg=0;
	s32		wnum;
	s32		tmp;
	BOOL	bflg=FALSE;
	
	// 終了判定
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	//
	// 上
	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
		// スクロールありの対応
		if(!(wk->Flag & FLG_MW_NO_SCROLL)){
			wk->TopNum--;
			if(wk->TopNum < 0){
				wk->TopNum = 0;
			}
			flg = 1;
		}
	}
	// 下
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
		// スクロールありの対応
		if(!(wk->Flag & FLG_MW_NO_SCROLL)){
			wk->TopNum++;
			if(wk->TopNum >= (wr_gmm_max_tbl[wk->Mode]-3)){
				wk->TopNum = wr_gmm_max_tbl[wk->Mode]-3-1;
			}
			flg = 1;
		}
	}
	// 左
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
		// スクロールありの対応
		if(!(wk->Flag & FLG_MW_NO_SCROLL)){
			wk->TopNum -= 4;
			if(wk->TopNum < 0){
				wk->TopNum = 0;
			}
			flg = 1;
			// アニメ再生
			wnum = CLWKID_CMN_LEFT;
			wr_chg_objseq( wk, CLWKMODE_CMN, wnum, wr_cmn_btn_tbl[wnum*4+1] );
			wk->Flag |= FLG_MW_ANM_LEFT;
		}
	}
	// 右
	else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
		// スクロールありの対応
		if(!(wk->Flag & FLG_MW_NO_SCROLL)){
			wk->TopNum +=4 ;
			if(wk->TopNum >= (wr_gmm_max_tbl[wk->Mode]-3)){
				wk->TopNum = wr_gmm_max_tbl[wk->Mode]-3-1;
			}
			flg = 1;
			// アニメ再生
			wnum = CLWKID_CMN_RIGHT;
			wr_chg_objseq( wk, CLWKMODE_CMN, wnum, wr_cmn_btn_tbl[wnum*4+1] );
			wk->Flag |= FLG_MW_ANM_RIGHT;
		}
	}
	// 戻る
	else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		// アニメ再生
		wnum = CLWKID_CMN_CROSS;
		wr_chg_objseq( wk, CLWKMODE_CMN, wnum, wr_cmn_btn_tbl[wnum*4+1] );
		wk->Flag |= FLG_MW_ANM_CROSS;
	}
	//
	// 更新
	if(flg == 1){
		wk->Flag |= FLG_MW_UPDATE;
		wk->Flag |= FLG_MW_BAR_UPDATE;
		bflg = TRUE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タッチペン用データ取得
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *	@param	const s32				領域の番号
 *	@param	GFL_UI_TP_HITTBL		タッチペン判定用
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_get_tp_area( WIN_RECORD_MAIN_WORK* wk, const s32 num, GFL_UI_TP_HITTBL *tpbuffer )
{
	s16		i;
	s16		buf[4];
	
	// 領域を取得
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
		if(buf[i] > 255){
			buf[i] = 255;
		}
	}
	// 領域
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
 *	@param	BOOL					(TRUE:タッチあり、FALSE:なし)
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL wr_chk_tp_input( WIN_RECORD_MAIN_WORK* wk )
{
	s16		i;
	s16		flg=0;
	s32		wnum;
	const GFL_UI_TP_HITTBL	*adr = &wk->HitBuffer[0];
	const int ret  = GFL_UI_TP_HitTrg( adr );
	const int cret = GFL_UI_TP_HitCont( adr );
	// なければ、TP_HIT_NONE
	BOOL	bflg=FALSE;
	
	// 終了判定
	if(wk->Flag & FLG_MW_END){
		return( bflg );
	}
	// アニメ再生中
	if(wk->Flag & FLG_MW_ANM_ALL){
		return( bflg );
	}
	
	// 左
	if(ret == AREA_TP_LEFT){
		// スクロールありの対応
		if(!(wk->Flag & FLG_MW_NO_SCROLL)){
			wk->TopNum -= 4;
			if(wk->TopNum < 0){
				wk->TopNum = 0;
			}
			// アニメ再生
			wnum = CLWKID_CMN_LEFT;
			wr_chg_objseq( wk, CLWKMODE_CMN, wnum, wr_cmn_btn_tbl[wnum*4+1] );
			wk->Flag |= FLG_MW_ANM_LEFT;
			flg = 1;
		}
	}
	// 右
	else if(ret == AREA_TP_RIGHT){
		// スクロールありの対応
		if(!(wk->Flag & FLG_MW_NO_SCROLL)){
			wk->TopNum +=4 ;
			if(wk->TopNum >= (wr_gmm_max_tbl[wk->Mode]-3)){
				wk->TopNum = wr_gmm_max_tbl[wk->Mode]-3-1;
			}
			// アニメ再生
			wnum = CLWKID_CMN_RIGHT;
			wr_chg_objseq( wk, CLWKMODE_CMN, wnum, wr_cmn_btn_tbl[wnum*4+1] );
			wk->Flag |= FLG_MW_ANM_RIGHT;
			flg = 1;
		}
	}
	// ×
	else if(ret == AREA_TP_CROSS){
		// アニメ再生
		wnum = CLWKID_CMN_CROSS;
		wr_chg_objseq( wk, CLWKMODE_CMN, wnum, wr_cmn_btn_tbl[wnum*4+1] );
		wk->Flag |= FLG_MW_ANM_CROSS;
		flg = 2;
	}
	// RETURN
	else if(ret == AREA_TP_RETURN){
		// アニメ再生
		wnum = CLWKID_CMN_RETURN;
		wr_chg_objseq( wk, CLWKMODE_CMN, wnum, wr_cmn_btn_tbl[wnum*4+1] );
		wk->Flag |= FLG_MW_ANM_RETURN;
		flg = 2;
	}
	// スクロールバー
	else{
		// スクロールありの対応
		if(!(wk->Flag & FLG_MW_NO_SCROLL)){
			// contで取得
			if(cret == AREA_TP_SCROLL){
				wr_set_scrollbar( wk );
				flg = 3;
			}
		}
	}
	//
	// 更新
	if(flg != 0){
		// 左右の場合は更新
		if(flg == 1){
			wk->Flag |= FLG_MW_BAR_UPDATE;
		}
		wk->Flag |= FLG_MW_UPDATE;
		bflg = TRUE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アニメの終了判定
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
	
	// 左アニメ再生中
	if(wk->Flag & FLG_MW_ANM_LEFT){
		wnum = CLWKID_CMN_LEFT;
		flg = 1;
	}
	// 右アニメ再生中
	else if(wk->Flag & FLG_MW_ANM_RIGHT){
		wnum = CLWKID_CMN_RIGHT;
		flg = 1;
	}
	// ×アニメ再生中
	else if(wk->Flag & FLG_MW_ANM_CROSS){
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
			flg = 2;
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
				// 終了処理
				wk->Step = STEP_END;
			}
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  スクロールバーの処理
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static BOOL wr_set_scrollbar( WIN_RECORD_MAIN_WORK* wk )
{
	u32				cnt;
	u32				buf[2];
	BOOL			bflg=FALSE;
	
	// ここにくるまでにスクロールバーの領域を
	// タッチしたかの判定は済んでいる
	// タッチした座標を取得
	if( GFL_UI_TP_GetPointCont( &buf[0], &buf[1] ) == TRUE ){
		// どの段階かが求まる
		cnt = SCROLLBAR_GetCount( wr_gmm_max_tbl[wk->Mode]-4, buf[1], 32, 160, 16 );
//		OS_Printf("pos :%d\n",cnt);
		// 現在の位置をセット
		wk->TopNum = cnt;
		wk->Flag |= FLG_MW_UPDATE;
		wk->Flag |= FLG_MW_BAR_UPDATE;
		//
		bflg = TRUE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  更新(主にwk->TopNumが更新したときに呼ばれる)
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_param_update( WIN_RECORD_MAIN_WORK* wk )
{
	s16				num;
	s16				nmax;
	s32				i;
	u32				py;
	GFL_CLACTPOS	pos;
	BOOL			bflg;
	
	// バー更新
	if(wk->Flag & FLG_MW_BAR_UPDATE){
		// スクロールありの対応
		if(!(wk->Flag & FLG_MW_NO_SCROLL)){
			py = SCROLLBAR_GetPosY( wr_gmm_max_tbl[wk->Mode]-4, wk->TopNum, 32, 160, 16 );
			// 座標の更新
			GFL_CLACT_WK_GetPos( wk->wk_obj.clwk[CLWKID_BAR], &pos, CLSYS_DEFREND_MAIN );
//			pos.x = 238;	// test
			pos.y = py;
			GFL_CLACT_WK_SetPos( wk->wk_obj.clwk[CLWKID_BAR], &pos, CLSYS_DEFREND_MAIN );
			//
			wk->Flag &= ~FLG_MW_BAR_UPDATE;
		}
	}
	// 更新
	if(wk->Flag & FLG_MW_UPDATE){
		// 最大数を調べる
		num = 4;			// 表示パネル数
		nmax = wr_gmm_max_tbl[wk->Mode];
		if(nmax < 4){
			num = nmax;
		}
		// 表示/非表示と座標セット
		for(i=0;i<4;i++){
			bflg = TRUE;
			if(i >= num){
				bflg = FALSE;
			}
			wr_set_panel_all( wk, i, wk->TopNum+i, bflg );
		}
		//
		wk->Flag &= ~FLG_MW_UPDATE;
	}
}



//-----------------------------------------------------------------------------
/**
 *	@brief  デバッグ
 *
 *	@param	WIN_RECORD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void wr_debug( WIN_RECORD_MAIN_WORK* wk )
{
#ifdef PM_DEBUG

#endif		// PM_DEBUG
}

