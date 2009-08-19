//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		worldtimer.c
 *	@brief		世界時計
 *	@author		tomoya takahashi
 *	@data		2007.09.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

//#include "system/d3dobj.h"
#include "system/wipe.h"
//#include "system/pm_overlay.h"
//#include "system/fontproc.h"
#include "print\gf_font.h"
#include "print/wordset.h"
#include "system/touch_subwindow.h"

#include "net\network_define.h"
#include "net_app/net_bugfix.h"

#include "savedata/wifihistory.h"
#include "savedata/config.h"

#include "worldtimer.naix"

#include "message.naix"
#include "msg/msg_worldtimer.h"
#include "msg/msg_wifi_place_msg_world.h"
#include "msg/msg_wifi_place_msg_GBR.h"

#include "net_app/wifi_lobby/worldtimer.h"
#include "net_app/wifi_lobby/wldtimer_snd.h"
#include "wifi_earth_place.naix"
#include "wifi_earth.naix"
#include "net_app/wifi_country.h"

#include "net_app/wifi_lobby/worldtimer_place.h"

#include "wflby_snd.h"
#include "system/gfl_use.h"
#include "system/bmp_winframe.h"
#include "system/actor_tool.h"
#include "system/main.h"
#include <calctool.h>
#include "print/printsys.h"
#include "gamesystem/msgspeed.h"


//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/**
 *				デバック設定
 */
//-----------------------------------------------------------------------------
#ifdef  PM_DEBUG
#define WLDTIMER_DEBUG_ALLPLACEOPEN		// 全地域の表示
//#define WLDTIMER_DEBUG_TIMEZONE	// タイムゾーンをコントロールする
#endif

// タイムゾーンコントロール
#ifdef WLDTIMER_DEBUG_TIMEZONE
static s32 s_WLDTIMER_DEBUG_TimeZone;
#endif

#ifdef WLDTIMER_DEBUG_ALLPLACEOPEN
static BOOL s_WLDTIMER_DEBUG_ALLPLACEOPEN_FLAG = FALSE;
#endif


//-----------------------------------------------------------------------------
//	src/net_app/wifi_earth/wifi_earthからコピー
//-----------------------------------------------------------------------------

//地点登録最大数
#define WORLD_PLACE_NUM_MAX	(0x400)	//だいたい１０００国くらいとる

//地球モデル初期化定義
#define INIT_EARTH_TRANS_XVAL	(0)
#define INIT_EARTH_TRANS_YVAL	(0)
#define INIT_EARTH_TRANS_ZVAL	(0)
#define INIT_EARTH_SCALE_XVAL	(FX32_ONE)
#define INIT_EARTH_SCALE_YVAL	(FX32_ONE)
#define INIT_EARTH_SCALE_ZVAL	(FX32_ONE)
#define INIT_EARTH_ROTATE_XVAL	(0x1980)	//東京をデフォルト位置にする
#define INIT_EARTH_ROTATE_YVAL	(0xcfe0)	//東京をデフォルト位置にする
#define INIT_EARTH_ROTATE_ZVAL	(0)

//カメラ初期化定義
#define INIT_CAMERA_TARGET_XVAL	(0)
#define INIT_CAMERA_TARGET_YVAL	(0)
#define INIT_CAMERA_TARGET_ZVAL	(0)
#define INIT_CAMERA_POS_XVAL	(0)
#define INIT_CAMERA_POS_YVAL	(0)
#define INIT_CAMERA_POS_ZVAL	(0x128000)
#define INIT_CAMERA_DISTANCE_NEAR	(0x050000)
#define INIT_CAMERA_DISTANCE_FAR	(0x128000)
#define INIT_CAMERA_PERSPWAY	(0x05c1)
#define INIT_CAMERA_CLIP_NEAR	(0)
#define INIT_CAMERA_CLIP_FAR	(FX32_ONE*100)
#define CAMERA_ANGLE_MIN		(-0x4000+0x200)
#define CAMERA_ANGLE_MAX		(0x4000-0x200)

//ライト初期化定義
#define USE_LIGHT_NUM			(0)
#define LIGHT_VECDEF			(FX32_ONE-1)
#define INIT_LIGHT_ANGLE_XVAL	(0)
#define INIT_LIGHT_ANGLE_YVAL	(0)
#define INIT_LIGHT_ANGLE_ZVAL	(-LIGHT_VECDEF)

//動作処理用定数
#define CAMERA_MOVE_SPEED_NEAR	(0x20)	//カメラ近距離時上下左右移動スピード	
#define CAMERA_MOVE_SPEED_FAR	(0x200)	//カメラ遠距離時上下左右移動スピード
#define CAMERA_INOUT_SPEED	(0x8000)	//カメラ遠近移動スピード
#define MARK_SCALE_INCDEC	(0x80)		//地点マーク遠近移動中拡縮比率
#define EARTH_LIMIT_ROTATE_XMIN	(0x1300)	//日本拡大地図の縦回転リミット下限（沖縄）
#define EARTH_LIMIT_ROTATE_XMAX	(0x2020)	//日本拡大地図の縦回転リミット上限（北海道稚内）
#define EARTH_LIMIT_ROTATE_YMIN	(0xcc80)	//日本拡大地図の横回転リミット下限（北海道釧路）
#define EARTH_LIMIT_ROTATE_YMAX	(0xd820)	//日本拡大地図の横回転リミット上限（沖縄）

// アイコンウィンドウ
#define	EARTH_ICON_WIN_PX		(25)
#define	EARTH_ICON_WIN_PY		(21)
#define	EARTH_ICON_WIN_SX		(6)
#define	EARTH_ICON_WIN_SY		(2)
#define DOTSIZE					(8)

//地球回転モード定義
enum {
	JAPAN_MODE = 0,
	GLOBAL_MODE,
};

//カメラ距離フラグ定義
enum {
	CAMERA_FAR = 0,
	CAMERA_NEAR,
};

//リストデータ構造体（バイナリデータ→実際に使用するリスト用データに変換）
typedef struct EARTH_DEMO_LIST_DATA_tag
{
	s16 x;
	s16 y;
	MtxFx33 rotate;	//相対回転行列をあらかじめ計算して保存
	u16	col;
	u16	nationID;
	u16	areaID;
}EARTH_DEMO_LIST_DATA;

//リストデータ構造体（バイナリデータ→実際に使用するリスト用データに変換）
typedef struct EARTH_DEMO_LIST_tag
{
	u32	listcount;
	EARTH_DEMO_LIST_DATA	place[WORLD_PLACE_NUM_MAX];
}EARTH_DEMO_LIST;
//------------------------------------------------------------------------------







//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	にほん　国コード
//=====================================
#define WLDTIMER_JPN_NATIONID	(country103)



//-------------------------------------
///	メッセージバッファ数
//=====================================
#define WLDTIMER_MSGMAN_STRBUFNUM	(128)


//-------------------------------------
///	Vram転送マネージャ　タスク数
//=====================================
#define WLDTIMER_VRAMTRANS_TASKNUM	( 64 )


//-------------------------------------
///	動作管理
//=====================================
enum{
	WLDTIMER_SEQ_FADEIN,
	WLDTIMER_SEQ_FADEINWAIT,
	WLDTIMER_SEQ_MAIN,
	WLDTIMER_SEQ_ENDCHECK,
	WLDTIMER_SEQ_ENDWAIT,
	WLDTIMER_SEQ_FADEOUT,
	WLDTIMER_SEQ_FADEOUTWAIT,
};
enum{	// 地球儀画面のシーケンス
	WLDTIMER_MAINSEQ_CONTROL,
	WLDTIMER_MAINSEQ_CAMERAMOVE,

	WLDTIMER_MAINSEQ_ENDCHECK,
	WLDTIMER_MAINSEQ_ENDCHECKWAIT,
};
enum{	// タスク画面のシーケンス
	WLDTIMER_SUB_SEQ_CONTROL,
};

//-------------------------------------
///	地域設定
//=====================================
enum{
	WLDTIMER_PLACE_COL_IN,		// 入室カラー
	WLDTIMER_PLACE_COL_OUT,		// 退室カラー
	WLDTIMER_PLACE_COL_TOUCH,	// タッチ位置カラー
	WLDTIMER_PLACE_COL_NUM,		// カラー数
	
	// 部屋にいない地域に設定されているカラー
	WLDTIMER_PLACE_COL_NONE = WLDTIMER_PLACE_COL_NUM,	
};
static const u32 sc_WLDTIMER_PLACE_MDL[ WLDTIMER_PLACE_COL_NUM ] = {	// モデルリソース
	NARC_worldtimer_earth_mark_1_nsbmd,
	NARC_worldtimer_earth_mark_2_nsbmd,
	NARC_worldtimer_earth_mark_3_nsbmd,
};
#define WLDTIMER_PLACE_TOUCH_SCALE_Z_ADD	( FX32_CONST(0.1) )


//-------------------------------------
///	BANK設定
//=====================================
static const GFL_DISP_VRAM sc_WLDTIMER_BANK = {
	GX_VRAM_BG_16_F,				// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_16_G,				// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_01_AB,				// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_0123_E,			// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_32K,		// メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
};

//-------------------------------------
///	BG設定
//=====================================
static const GFL_BG_SYS_HEADER sc_BGINIT = {
	GX_DISPMODE_GRAPHICS,
	GX_BGMODE_0,
	GX_BGMODE_0,
	GX_BG0_AS_3D
};


//-------------------------------------
///	ＢＧコントロールデータ
//=====================================
#define WLDTIMER_BGCNT_NUM	( 5 )	// ＢＧコントロールテーブル数
static const u32 sc_WLDTIMER_BGCNT_FRM[ WLDTIMER_BGCNT_NUM ] = {
	GFL_BG_FRAME1_M,
	GFL_BG_FRAME2_S,
	GFL_BG_FRAME3_S,
	GFL_BG_FRAME0_S,
	GFL_BG_FRAME1_S,
};
static const GFL_BG_BGCNT_HEADER sc_WLDTIMER_BGCNT_DATA[ WLDTIMER_BGCNT_NUM ] = {
	{	// GFL_BG_FRAME1_M
		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x3800, GX_BG_CHARBASE_0x00000, 0x3800, GX_BG_EXTPLTT_01,
		0, 0, 0, FALSE
	},
	{	// GFL_BG_FRAME2_S
		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0xd000, GX_BG_CHARBASE_0x00000, 0x3800, GX_BG_EXTPLTT_01,
		2, 0, 0, FALSE
	},
	{	// GFL_BG_FRAME3_S
		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000, 0x3800, GX_BG_EXTPLTT_01,
		3, 0, 0, FALSE
	},

	// サブ画面０，１は同じキャラクタオフセット
	{	// GFL_BG_FRAME0_S
		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x10000, 0x8000, GX_BG_EXTPLTT_01,
		0, 0, 0, FALSE
	},
	{	// GFL_BG_FRAME1_S
		0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0xd800, GX_BG_CHARBASE_0x10000, 0x8000, GX_BG_EXTPLTT_01,
		1, 0, 0, FALSE
	},
};


//-------------------------------------
///	ＢＧパレット構成
//=====================================
enum{	// サブ
	WLDTIMER_PALSUB_BACK00,
	WLDTIMER_PALSUB_BACK01,
	WLDTIMER_PALSUB_BACK02,
	WLDTIMER_PALSUB_BACK03,
	WLDTIMER_PALSUB_NONE00,
	WLDTIMER_PALSUB_BACK05,
	WLDTIMER_PALSUB_NONE02,
	WLDTIMER_PALSUB_NONE03,
	WLDTIMER_PALSUB_NONE04,
	WLDTIMER_PALSUB_NONE05,
	WLDTIMER_PALSUB_TALKWIN,
	WLDTIMER_PALSUB_FONT,
	WLDTIMER_PALSUB_NONE08,
	WLDTIMER_PALSUB_NONE09,
	WLDTIMER_PALSUB_SPFONT,
	WLDTIMER_PALSUB_BACK04,
};
enum{	// メイン
	WLDTIMER_PALMAIN_SYSTEM,
	WLDTIMER_PALMAIN_FONT,
	WLDTIMER_PALMAIN_TALK,
	WLDTIMER_PALMAIN_BTTN,
	WLDTIMER_PALMAIN_NONE02,
	WLDTIMER_PALMAIN_NONE03,
	WLDTIMER_PALMAIN_NONE04,
	WLDTIMER_PALMAIN_NONE05,
	WLDTIMER_PALMAIN_NONE06,
	WLDTIMER_PALMAIN_NONE07,
	WLDTIMER_PALMAIN_NONE08,
	WLDTIMER_PALMAIN_NONE09,
	WLDTIMER_PALMAIN_NONE10,
	WLDTIMER_PALMAIN_NONE11,
	WLDTIMER_PALMAIN_NONE12,
	WLDTIMER_PALMAIN_NONE13,
};

//-------------------------------------
///	OAM設定
//=====================================
#define WLDTIMER_OAM_CONTNUM	( 32 )
#define WLDTIMER_RESMAN_NUM		( 4 )	// OAMリソースマネージャ数
#define WLDTIMER_SF_MAT_Y		( FX32_CONST(256) )
#if WB_FIX
static const CHAR_MANAGER_MAKE sc_WLDTIMER_CHARMAN_INIT = {
	WLDTIMER_OAM_CONTNUM,
	16*1024,	// 16K
	16*1024,	// 16K
	HEAPID_WLDTIMER
};
#endif

//-------------------------------------
///	カメラ
//=====================================
static const VecFx32 sc_WLDTIMER_CAMERA_TARGET={
	INIT_CAMERA_TARGET_XVAL,
	INIT_CAMERA_TARGET_YVAL,
	INIT_CAMERA_TARGET_ZVAL
};
static const VecFx32 sc_WLDTIMER_CAMERA_POS={
	INIT_CAMERA_POS_XVAL,
	INIT_CAMERA_POS_YVAL,
	INIT_CAMERA_POS_ZVAL
};

//-------------------------------------
///	接近しているときの地域スケール値
//=====================================
#define INIT_EARTH_SCALE_NEAR_XVAL	(0x300)
#define INIT_EARTH_SCALE_NEAR_YVAL	(0x300)
#define INIT_EARTH_SCALE_NEAR_ZVAL	(FX32_ONE)


//-------------------------------------
///	地球儀管理管理
//=====================================
enum{
	WLDTIMER_EARTHCONT_RET_NONE,			// 何もなし
	WLDTIMER_EARTHCONT_RET_CAMERA_MOVE,		// カメラ動作
	WLDTIMER_EARTHCONT_RET_END,				// 終了
};
#define WLDTIMER_POINTPLANE_HITAREA	( 0x80 )	// 地域にポイントがあたっているかチェックするエリア


//-------------------------------------
///	タッチ
//=====================================
enum{
	WLDTIMER_TOUCH_PM_RESULT,
	WLDTIMER_TOUCH_PM_LENX,
	WLDTIMER_TOUCH_PM_LENY,
};
#define WLDTIMER_TOUCH_ZOOMWAIT_COUNT	( 4 )	// trgからこのシンク以内にタッチをやめたらカメラを動かす
#define WLDTIMER_TOUCH_LEN_LIMIT		( 0x3f )	// スライド距離リミット
#define WLDTIMER_TOUCH_EARTHSPEED_DIVNEAR	( 3 )	// タッチで地球儀を回すときに割る値
#define WLDTIMER_TOUCH_EARTHSPEED_DIVFAR	( 6 )	// タッチで地球儀を回すときに割る値
#define	WLDTIMER_TOUCH_END_MSG_COL	( GF_PRINTCOLOR_MAKE( 2, 1, 15 ) )




//-------------------------------------
///	 時間帯データ	ZONETYPE
//=====================================
enum{
	WLDTIMER_ZONETYPE_MORNING,	// 朝
	WLDTIMER_ZONETYPE_NOON,		// 昼
	WLDTIMER_ZONETYPE_EVENING,	// 夕方
	WLDTIMER_ZONETYPE_NIGHT,	// 夜
	WLDTIMER_ZONETYPE_MIDNIGHT,	// 深夜
	WLDTIMER_ZONETYPE_NUM,		// 数
};
enum{	// ポケモン
	WLDTIMER_TIME_POKE_EEFI= 0,		// エーフィー
	WLDTIMER_TIME_POKE_POPPO,		// ポッポ
	WLDTIMER_TIME_POKE_MITSUHANII,	// ミツハニー
	WLDTIMER_TIME_POKE_CHERIMU,		// チェリム
	WLDTIMER_TIME_POKE_SORUROKKU,	// ソルロック
	WLDTIMER_TIME_POKE_KIMAWARI,	// キマワリ
	WLDTIMER_TIME_POKE_YAMIKARASU,	// ヤミカラス	
	WLDTIMER_TIME_POKE_FUWANTE,		// フワンテ
	WLDTIMER_TIME_POKE_SURIPA,		// スリーパー
	WLDTIMER_TIME_POKE_GOOSUTO,		// ゴースト
	WLDTIMER_TIME_POKE_RUNATON,		// ルナトーン
	WLDTIMER_TIME_POKE_BURAKKI,		// ブラッキー
	WLDTIMER_TIME_POKE_NUM,
};
#define	WLDTIMER_TIMEZONE_DATANUM	( 24 )	// タイムゾーン数
#define WLDTIMER_TIMEZONE_START_HOUR	(4)	// インデックス0の時間
typedef struct{
	u8			zone;		// ゾーンID
	u8			mons;		// ポケモンの絵
} WLDTIMER_TIMERZONE;
static const WLDTIMER_TIMERZONE	sc_WLDTIMER_TIMEZONE_DATA[ WLDTIMER_TIMEZONE_DATANUM ] ={
	{	// 4hour
		WLDTIMER_ZONETYPE_MORNING,	WLDTIMER_TIME_POKE_EEFI
	},
	{	// 5hour
		WLDTIMER_ZONETYPE_MORNING,	WLDTIMER_TIME_POKE_EEFI
	},
	{	// 6hour
		WLDTIMER_ZONETYPE_MORNING,	WLDTIMER_TIME_POKE_POPPO
	},
	{	// 7hour
		WLDTIMER_ZONETYPE_MORNING,	WLDTIMER_TIME_POKE_POPPO  
	},
	{	// 8hour
		WLDTIMER_ZONETYPE_MORNING,	WLDTIMER_TIME_POKE_MITSUHANII
	},
	{	// 9hour
		WLDTIMER_ZONETYPE_MORNING,	WLDTIMER_TIME_POKE_MITSUHANII
	},
	{	// 10hour
		WLDTIMER_ZONETYPE_NOON,		WLDTIMER_TIME_POKE_CHERIMU
	},
	{	// 11hour
		WLDTIMER_ZONETYPE_NOON,		WLDTIMER_TIME_POKE_CHERIMU
	},
	{	// 12hour
		WLDTIMER_ZONETYPE_NOON,		WLDTIMER_TIME_POKE_SORUROKKU
	},
	{	// 13hour
		WLDTIMER_ZONETYPE_NOON,		WLDTIMER_TIME_POKE_SORUROKKU
	},
	{	// 14hour
		WLDTIMER_ZONETYPE_NOON,		WLDTIMER_TIME_POKE_KIMAWARI
	},
	{	// 15hour
		WLDTIMER_ZONETYPE_NOON,		WLDTIMER_TIME_POKE_KIMAWARI
	},
	{	// 16hour
		WLDTIMER_ZONETYPE_EVENING,	WLDTIMER_TIME_POKE_YAMIKARASU
	},
	{	// 17hour
		WLDTIMER_ZONETYPE_EVENING,	WLDTIMER_TIME_POKE_YAMIKARASU
	},
	{	// 18hour
		WLDTIMER_ZONETYPE_EVENING,	WLDTIMER_TIME_POKE_FUWANTE
	},
	{	// 19hour
		WLDTIMER_ZONETYPE_EVENING,	WLDTIMER_TIME_POKE_FUWANTE
	},
	{	// 20hour
		WLDTIMER_ZONETYPE_NIGHT,	WLDTIMER_TIME_POKE_SURIPA
	},
	{	// 21hour
		WLDTIMER_ZONETYPE_NIGHT,	WLDTIMER_TIME_POKE_SURIPA
	},
	{	// 22hour
		WLDTIMER_ZONETYPE_NIGHT,	WLDTIMER_TIME_POKE_GOOSUTO
	},
	{	// 23hour
		WLDTIMER_ZONETYPE_NIGHT,	WLDTIMER_TIME_POKE_GOOSUTO
	},
	{	// 0hour
		WLDTIMER_ZONETYPE_MIDNIGHT,	WLDTIMER_TIME_POKE_RUNATON
	},
	{	// 1hour
		WLDTIMER_ZONETYPE_MIDNIGHT,	WLDTIMER_TIME_POKE_RUNATON
	},
	{	// 2hour
		WLDTIMER_ZONETYPE_MIDNIGHT,	WLDTIMER_TIME_POKE_BURAKKI
	},
	{	// 3hour
		WLDTIMER_ZONETYPE_MIDNIGHT,	WLDTIMER_TIME_POKE_BURAKKI
	},
};




//-------------------------------------
///	時間表示画面	
//=====================================
//	シーケンス
enum{
	WLDTIMER_VIEWER_SEQ_MAIN,
	WLDTIMER_VIEWER_SEQ_FADE_INIT,
	WLDTIMER_VIEWER_SEQ_FADE,
	WLDTIMER_VIEWER_SEQ_FADE_SKIP_INIT,
	WLDTIMER_VIEWER_SEQ_FADE_SKIP,
};

// 表示データバッファ数
// 表示データ3つ  
// キュー最後尾用バッファ1つ
#define WLDTIMER_VIEWER_BUFFNUM	( 4 )	
enum{	// 表示件数
	WLDTIMER_VIEWER_DRAW_TOP,	// 上
	WLDTIMER_VIEWER_DRAW_MID,	// 中
	WLDTIMER_VIEWER_DRAW_UND,	// 下
	WLDTIMER_VIEWER_DRAWNUM
};
// スクリーン転送位置
#define WLDTIMER_VIEWER_SCRN_X	( 0 )	// 表示開始位置
#define WLDTIMER_VIEWER_SCRN_Y	( 0 )	
#define WLDTIMER_VIEWER_SCRN_SX	( 32 )	// スクリーン横サイズ
#define WLDTIMER_VIEWER_SCRN_SY	( 6 )	// スクリーン縦サイズ
// アニメデータ
static const u8 WLDTIMER_VIEWER_ANM_FRAME[ WLDTIMER_ZONETYPE_NUM ] = {
	30,		// 朝
	30,		// 昼
	30,		// 夕方
	30,		// 夜
	30,		// 深夜
};
// メッセージ
#define WLDTIMER_VIEWER_MSG_TITLE_X		( 0 )
#define WLDTIMER_VIEWER_MSG_TITLE_Y		( 0 )
#define WLDTIMER_VIEWER_MSG_NATION_X	( 0 )
#define WLDTIMER_VIEWER_MSG_NATION_Y	( 16 )
#define WLDTIMER_VIEWER_MSG_AREA_X		( 0 )
#define WLDTIMER_VIEWER_MSG_AREA_Y		( 32 )
#define	WLDTIMER_VIEWER_MSG_NATION_COL	( GF_PRINTCOLOR_MAKE( 1, 2, 0 ) )
#define	WLDTIMER_VIEWER_MSG_AREA_COL	( GF_PRINTCOLOR_MAKE( 1, 2, 0 ) )
static const GF_PRINTCOLOR WLDTIMER_VIEWER_MSG_TITLE_COL[ WLDTIMER_ZONETYPE_NUM ] = {
	GF_PRINTCOLOR_MAKE( 5, 6, 0 ),
	GF_PRINTCOLOR_MAKE( 7, 8, 0 ),
	GF_PRINTCOLOR_MAKE( 9, 10, 0 ),
	GF_PRINTCOLOR_MAKE( 11, 12, 0 ),
	GF_PRINTCOLOR_MAKE( 11, 12, 0 ),
};

// フェード分割数
#define WLDTIMER_VIEWER_FADE_DIV_ONE	( 6 )	// １ゾーンタイプの分割数
#define WLDTIMER_VIEWER_FADE_DIV		( 3*6 )	// 全分割数
#define WLDTIMER_VIEWER_FADE_DIV_START	( 2 )	// WLDTIMER_VIEWER_FADE_DIV_STARTづつフェードしていく
enum{
	WLDTIMER_VIEWER_FADE_DIV_SEQ_CHANGE00,
//	WLDTIMER_VIEWER_FADE_DIV_SEQ_WAIT00,
//	WLDTIMER_VIEWER_FADE_DIV_SEQ_WAIT01,
//	WLDTIMER_VIEWER_FADE_DIV_SEQ_WAIT02,
	WLDTIMER_VIEWER_FADE_DIV_SEQ_CHANGE01,
	WLDTIMER_VIEWER_FADE_DIV_SEQ_NUM,
};
#define WLDTIMER_VIEWER_FADE_SKIP_ONE		( 2 )	// スキップ時一気にフェードさせるライン
#define WLDTIMER_VIEWER_FADE_SKIP_COUNTNUM	( WLDTIMER_VIEWER_FADE_DIV/WLDTIMER_VIEWER_FADE_SKIP_ONE )
#define WLDTIMER_VIEWER_FADE_SKIP01_START	( 1 )	// スキップ時一気にフェードさせるライン
// ポケモンバルーン
#define WLDTIMER_VIEWER_POKEBLN_NCGR_GET(x)	(NARC_worldtimer_world_w_balloon00_NCGR + ((x)*3))
#define WLDTIMER_VIEWER_POKEBLN_NCLR_GET(x)	(NARC_worldtimer_world_w_balloon_NCLR)
#define WLDTIMER_VIEWER_POKEBLN_NCER_GET(x)	(NARC_worldtimer_world_w_balloon00_NCER + ((x)*3))
#define WLDTIMER_VIEWER_POKEBLN_NANR_GET(x)	(NARC_worldtimer_world_w_balloon00_NANR + ((x)*3))
#define WLDTIMER_VIEWER_POKEBLN_NCLR_CONTID	( 0 )	// パレット管理ID
#define WLDTIMER_VIEWER_POKEBLN_PL_NUM		(12)		// 読み込むパレット数
#define WLDTIMER_VIEWER_POKEBLN_BG_PRI		(1)		// BG優先順位
#define WLDTIMER_VIEWER_POKEBLN_SFT_PRI		(64)	// ソフト優先順位
#define WLDTIMER_VIEWER_POKEBLN_WAIT		(128)	// 次の表示までのウエイト
#define WLDTIMER_VIEWER_POKEBLN_OAM_SIZ		(32)	
#define WLDTIMER_VIEWER_POKEBLN_OAM_HFSIZ	(16)	



//-------------------------------------
///	ポケモンバルーンアニメデータ
//=====================================
typedef struct {
	s16 sx;				// 開始位置
	s16 sy;
	s16 ex;				// 終了位置
	s16 ey;
	s16 countmax;		// 動作カウント最大値
	s16 wx;				// ゆれ幅
	s16 wcountmax;		// ゆれ幅カウント最大
	u16 pad;
} WLDTIMER_POKEBLN_MOVEDATA;
#define WLDTIMER_POKEBLN_MOVEDATANUM	(WLDTIMER_TIME_POKE_NUM)
#define WLDTIMER_POKEBLN_WX_LIGHT		( 12 )
#define WLDTIMER_POKEBLN_WC_LIGHT		( 54 )
#define WLDTIMER_POKEBLN_WX_MIDDLE		( 8 )
#define WLDTIMER_POKEBLN_WC_MIDDLE		( 80 )
#define WLDTIMER_POKEBLN_WX_HEAVY		( 4 )
#define WLDTIMER_POKEBLN_WC_HEAVY		( 100 )
#define WLDTIMER_POKEBLN_CM_LIGHT		( 220 )	// 軽い風船の上にいくスピード
#define WLDTIMER_POKEBLN_CM_MIDDLE		( 319 )	// 普通風船の上にいくスピード
#define WLDTIMER_POKEBLN_CM_HEAVY		( 399 )	// 重い風船の上にいくスピード
static const WLDTIMER_POKEBLN_MOVEDATA sc_WLDTIMER_POKEBLN_MOVEDATA_TBL[ WLDTIMER_POKEBLN_MOVEDATANUM ][ WLDTIMER_VIEWER_DRAWNUM ] = {
	//		sx		sy		ex		ey	cmax						wx							wcmax
	{	//エーフィーポケモン
		{	160,	48,		190,	0,	WLDTIMER_POKEBLN_CM_MIDDLE,	WLDTIMER_POKEBLN_WX_MIDDLE,	WLDTIMER_POKEBLN_WC_MIDDLE },	// 上
		{	206,	96,		160,	48,	WLDTIMER_POKEBLN_CM_MIDDLE,	WLDTIMER_POKEBLN_WX_MIDDLE,	WLDTIMER_POKEBLN_WC_MIDDLE },	// 中
		{	150,	144,	206,	96,	WLDTIMER_POKEBLN_CM_MIDDLE,	WLDTIMER_POKEBLN_WX_MIDDLE,	WLDTIMER_POKEBLN_WC_MIDDLE },	// 下
	},
	{	// ぽっぽポケモン
		{	190,	48,		140,	0,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 上
		{	150,	96,		190,	48,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 中
		{	200,	144,	150,	96,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 下
	},
	{	// ミツハニーポケモン
		{	128,	48,		160,	0,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 上
		{	210,	96,		128,	48,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 中
		{	160,	144,	210,	96,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 下
	},
	{	// チェリムポケモン
		{	180,	48,		138,	0,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 上
		{	138,	96,		180,	48,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 中
		{	180,	144,	138,	96,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 下
	},
	{	// ソルロックポケモン
		{	180,	48,		160,	0,	WLDTIMER_POKEBLN_CM_HEAVY,	WLDTIMER_POKEBLN_WX_HEAVY,	WLDTIMER_POKEBLN_WC_HEAVY },	// 上
		{	130,	96,		180,	48,	WLDTIMER_POKEBLN_CM_HEAVY,	WLDTIMER_POKEBLN_WX_HEAVY,	WLDTIMER_POKEBLN_WC_HEAVY },	// 中
		{	128,	144,	130,	96,	WLDTIMER_POKEBLN_CM_HEAVY,	WLDTIMER_POKEBLN_WX_HEAVY,	WLDTIMER_POKEBLN_WC_HEAVY },	// 下
	},
	{	// キマワリポケモン
		{	180,	48,		116,	0,	WLDTIMER_POKEBLN_CM_MIDDLE,	WLDTIMER_POKEBLN_WX_MIDDLE,	WLDTIMER_POKEBLN_WC_MIDDLE },	// 上
		{	116,	96,		180,	48,	WLDTIMER_POKEBLN_CM_MIDDLE,	WLDTIMER_POKEBLN_WX_MIDDLE,	WLDTIMER_POKEBLN_WC_MIDDLE },	// 中
		{	180,	144,	116,	96,	WLDTIMER_POKEBLN_CM_MIDDLE,	WLDTIMER_POKEBLN_WX_MIDDLE,	WLDTIMER_POKEBLN_WC_MIDDLE },	// 下
	},
	{	// ヤミカラスポケモン
		{	180,	48,		240,	0,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 上
		{	160,	96,		180,	48,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 中
		{	96,		144,	160,	96,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 下
	},
	{	// フワンテポケモン
		{	144,	48,		218,	0,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 上
		{	218,	96,		144,	48,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 中
		{	144,	144,	218,	96,	WLDTIMER_POKEBLN_CM_LIGHT,	WLDTIMER_POKEBLN_WX_LIGHT,	WLDTIMER_POKEBLN_WC_LIGHT },	// 下
	},
	{	// スリーパーポケモン
		{	160,	48,		192,	0,	WLDTIMER_POKEBLN_CM_HEAVY,	WLDTIMER_POKEBLN_WX_HEAVY,	WLDTIMER_POKEBLN_WC_HEAVY },	// 上
		{	192,	96,		160,	48,	WLDTIMER_POKEBLN_CM_HEAVY,	WLDTIMER_POKEBLN_WX_HEAVY,	WLDTIMER_POKEBLN_WC_HEAVY },	// 中
		{	160,	144,	192,	96,	WLDTIMER_POKEBLN_CM_HEAVY,	WLDTIMER_POKEBLN_WX_HEAVY,	WLDTIMER_POKEBLN_WC_HEAVY },	// 下
	},
	{	// ゴーストポケモン
		{	140,	48,		96,		0,	WLDTIMER_POKEBLN_CM_MIDDLE,	WLDTIMER_POKEBLN_WX_MIDDLE,	WLDTIMER_POKEBLN_WC_MIDDLE },	// 上
		{	190,	96,		140,	48,	WLDTIMER_POKEBLN_CM_MIDDLE,	WLDTIMER_POKEBLN_WX_MIDDLE,	WLDTIMER_POKEBLN_WC_MIDDLE },	// 中
		{	240,	144,	190,	96,	WLDTIMER_POKEBLN_CM_MIDDLE,	WLDTIMER_POKEBLN_WX_MIDDLE,	WLDTIMER_POKEBLN_WC_MIDDLE },	// 下
	},
	{	// ルナトーンポケモン
		{	212,	48,		160,	0,	WLDTIMER_POKEBLN_CM_HEAVY,	WLDTIMER_POKEBLN_WX_HEAVY,	WLDTIMER_POKEBLN_WC_HEAVY },	// 上
		{	212,	96,		212,	48,	WLDTIMER_POKEBLN_CM_HEAVY,	WLDTIMER_POKEBLN_WX_HEAVY,	WLDTIMER_POKEBLN_WC_HEAVY },	// 中
		{	160,	144,	212,	96,	WLDTIMER_POKEBLN_CM_HEAVY,	WLDTIMER_POKEBLN_WX_HEAVY,	WLDTIMER_POKEBLN_WC_HEAVY },	// 下
	},
	{	// ブラッキーポケモン
		{	123,	48,		170,	0,	WLDTIMER_POKEBLN_CM_MIDDLE,	WLDTIMER_POKEBLN_WX_MIDDLE,	WLDTIMER_POKEBLN_WC_MIDDLE },	// 上
		{	198,	96,		123,	48,	WLDTIMER_POKEBLN_CM_MIDDLE,	WLDTIMER_POKEBLN_WX_MIDDLE,	WLDTIMER_POKEBLN_WC_MIDDLE },	// 中
		{	133,	144,	198,	96,	WLDTIMER_POKEBLN_CM_MIDDLE,	WLDTIMER_POKEBLN_WX_MIDDLE,	WLDTIMER_POKEBLN_WC_MIDDLE },	// 下
	},
};
 
//-------------------------------------
///	ウィンドウ関連
//
//----------------------------------------------------------------------------
/**
//----------------------------------------------------------------------------
/*	国と地域からその位置を中心に持っていく*
 *	@brief
 @param			ワーク
 */	
//-----------------------------------------------------------------------------
//=====================================
// サブ画面のトークウィンドウ
#define WLDTIMER_SUB_TALKWIN_CGX	(10)	// フレームキャラクタの後に続く
#define WLDTIMER_SUB_TALKWIN_PAL	(WLDTIMER_PALSUB_TALKWIN)
#define WLDTIMER_SUB_TALKWIN_CGXEND	(WLDTIMER_SUB_TALKWIN_CGX+TALK_WIN_CGX_SIZ)

// メイン画面の会話ウィンドウ
#define WLDTIMER_MAIN_TALKWIN_CGX		(1)
#define WLDTIMER_MAIN_TALKWIN_PAL		(WLDTIMER_PALMAIN_TALK)
#define WLDTIMER_MAIN_TALKWIN_CGXEND	(WLDTIMER_MAIN_TALKWIN_CGX+TALK_WIN_CGX_SIZ)

// メイン画面のシステムウィンドウ
#define WLDTIMER_MAIN_SYSTEMWIN_CGX		(WLDTIMER_MAIN_TALKWIN_CGXEND)
#define WLDTIMER_MAIN_SYSTEMWIN_PAL		(WLDTIMER_PALMAIN_SYSTEM)
#define WLDTIMER_MAIN_SYSTEMWIN_CGXEND	(WLDTIMER_MAIN_SYSTEMWIN_CGX+MENU_WIN_CGX_SIZ)

										
//-------------------------------------
///	メイン画面ボタン
//=====================================
#define WLDTIMER_MAIN_BTTNBMP_X		(EARTH_ICON_WIN_PX)
#define WLDTIMER_MAIN_BTTNBMP_Y		(EARTH_ICON_WIN_PY)
#define WLDTIMER_MAIN_BTTNBMP_SIZX	(EARTH_ICON_WIN_SX)
#define WLDTIMER_MAIN_BTTNBMP_SIZY	(EARTH_ICON_WIN_SY)
#define WLDTIMER_MAIN_BTTNBMP_CGX	( WLDTIMER_MAIN_SYSTEMWIN_CGXEND )
#define WLDTIMER_MAIN_BTTNBMP_PAL	( WLDTIMER_PALMAIN_FONT )
#define WLDTIMER_MAIN_BTTNBMP_CGXEND ( WLDTIMER_MAIN_BTTNBMP_CGX+(WLDTIMER_MAIN_BTTNBMP_SIZX*WLDTIMER_MAIN_BTTNBMP_SIZY) )

//-------------------------------------
///	メイン画面会話ウィンドウ
//=====================================
#define WLDTIMER_MAIN_TALKBMP_X		(2)
#define WLDTIMER_MAIN_TALKBMP_Y		(1)
#define WLDTIMER_MAIN_TALKBMP_SIZX	(27)
#define WLDTIMER_MAIN_TALKBMP_SIZY	(4)
#define WLDTIMER_MAIN_TALKBMP_CGX	( WLDTIMER_MAIN_BTTNBMP_CGXEND )
#define WLDTIMER_MAIN_TALKBMP_PAL	( WLDTIMER_PALMAIN_FONT )
#define WLDTIMER_MAIN_TALKBMP_CGXEND ( WLDTIMER_MAIN_TALKBMP_CGX+(WLDTIMER_MAIN_TALKBMP_SIZX*WLDTIMER_MAIN_TALKBMP_SIZY) )

//-------------------------------------
///	メイン画面タッチボタンウィンドウ
//=====================================
#define WLDTIMER_MAIN_SUBBTTN_CGX	( WLDTIMER_MAIN_TALKBMP_CGXEND )
#define WLDTIMER_MAIN_SUBBTTN_PLTT	( WLDTIMER_PALMAIN_BTTN )
#define WLDTIMER_MAIN_SUBBTTN_X		( 25 )
#define WLDTIMER_MAIN_SUBBTTN_Y		( 6 )
#define WLDTIMER_MAIN_SUBBTTN_CGXEND	( WLDTIMER_MAIN_SUBBTTN_CGX+TOUCH_SW_USE_CHAR_NUM )
static const TOUCH_SW_PARAM sc_TOUCH_SW_PARAM = {
	GFL_BG_FRAME1_M,
	WLDTIMER_MAIN_SUBBTTN_CGX,
	WLDTIMER_MAIN_SUBBTTN_PLTT,
	WLDTIMER_MAIN_SUBBTTN_X,
	WLDTIMER_MAIN_SUBBTTN_Y,
	GFL_APP_KTST_TOUCH,
	0,
	TOUCH_SW_TYPE_S,
};


//-------------------------------------
///	サブ画面会話ウィンドウ
//=====================================
#define WLDTIMER_SUB_TALKBMP_X		(2)
#define WLDTIMER_SUB_TALKBMP_Y		(19)
#define WLDTIMER_SUB_TALKBMP_SIZX	(27)
#define WLDTIMER_SUB_TALKBMP_SIZY	(4)
#define WLDTIMER_SUB_TALKBMP_CGX	( WLDTIMER_SUB_TALKWIN_CGXEND )
#define WLDTIMER_SUB_TALKBMP_PAL	( WLDTIMER_PALSUB_FONT )
#define WLDTIMER_SUB_TALKBMP_CGSIZ	( WLDTIMER_SUB_TALKBMP_SIZX*WLDTIMER_SUB_TALKBMP_SIZY )

//-------------------------------------
///	ビューアーメッセージ
//=====================================
#define WLDTIMER_VIEWER_MSGBMP_X		( 0 )
#define WLDTIMER_VIEWER_MSGBMP_Y		( 0 )
#define WLDTIMER_VIEWER_MSGBMP_SIZX		( 27 )
#define WLDTIMER_VIEWER_MSGBMP_SIZY		( 6 )
#define WLDTIMER_VIEWER_MSGBMP_CGX		( WLDTIMER_SUB_TALKBMP_CGX+WLDTIMER_SUB_TALKBMP_CGSIZ )
#define WLDTIMER_VIEWER_MSGBMP_PAL		( WLDTIMER_PALSUB_SPFONT )
#define WLDTIMER_VIEWER_MSGBMP_CGSIZ	( WLDTIMER_VIEWER_MSGBMP_SIZX*WLDTIMER_VIEWER_MSGBMP_SIZY )


//-------------------------------------
///	タイムゾーンアニメ初期化データ
//=====================================
#define WLDTIMER_TIMEZONEANM_SCRNANM_MAX	(2)
typedef struct {
	// スクリーンアニメデータ
	u16	scrn_idx[ WLDTIMER_TIMEZONEANM_SCRNANM_MAX ];	// ないときNULL
	u16	scrn_frame;

	// パレットアニメデータ
	u16		pltt_idx;		// ないときはどんな値でもOK
	u16		pltt_frame;		// 無いときは０
	u16		pltt_no;		// 転送パレット
} WLDTIMER_TIMEZONEANM_INIT;

static const WLDTIMER_TIMEZONEANM_INIT	sc_WLDTIMER_TIMEZONE_ANMINIT[ WLDTIMER_ZONETYPE_NUM ] ={
	{	// 朝
		{
			NARC_worldtimer_world_watch1_NSCR,
		},
		1,
		NARC_worldtimer_world_w0anim_NCLR,	4, 0,
	},
	{	// 昼
		{
			NARC_worldtimer_world_watch2a_NSCR,
			NARC_worldtimer_world_watch2b_NSCR,
		},
		2,
		0,	0, 1,
	},
	{	// 夕方
		{
			NARC_worldtimer_world_watch3_NSCR,
		},
		1,
		NARC_worldtimer_world_w2anim_NCLR,	4, 2,
	},
	{	// 夜
		{
			NARC_worldtimer_world_watch4_NSCR,
		},
		1,
		NARC_worldtimer_world_w3anim_NCLR,	16, 3,
	},
	{	// 深夜
		{
			NARC_worldtimer_world_watch5_NSCR,
		},
		1,
		NARC_worldtimer_world_w3anim_NCLR,	16, 3,
	},
};

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	動作フラグ
//=====================================
typedef struct{
	u32	world	:1;		// 世界表示モードかどうか
	u32	japan	:1;		// 日本ROMかどうか
	u32	pad		:31;
}WLDTIMER_FLAG;

//-------------------------------------
///	表示システム
//=====================================
typedef struct {
	// BG
	//GF_BGL_INI*				p_bgl;

	// OAM
    GFL_CLUNIT*           p_clactset;		// セルアクターセット
  	PLTTSLOT_SYS_PTR	plttslot;
#if WB_FIX
    CLACT_U_EASYRENDER_DATA renddata;       // 簡易レンダーデータ
    CLACT_U_RES_MANAGER_PTR p_resman[WLDTIMER_RESMAN_NUM]; // キャラ・パレットリソースマネージャ
#endif
	
	// アーカイブ
	ARCHANDLE* p_handle;
} WLDTIMER_DRAWSYS;


//-------------------------------------
///	地球儀ワーク
//=====================================
typedef struct {
	// 表示データ
	VecFx32 trans;
	VecFx32 scale;
	VecFx32	rotate;

	// グラフィック
    GFL_G3D_OBJSTATUS	obj;
    VecFx32 obj_rotate;
#if WB_FIX
	GFL_G3D_OBJ	  mdl;
#else
	GFL_G3D_OBJ*	g3dobj;
  GFL_G3D_RES*  p_mdlres;
  GFL_G3D_RND*  rnder;
#endif
} WLDTIMER_EARTH;


//-------------------------------------
///	地域データ
//=====================================
typedef struct {
	// データ
	EARTH_DEMO_LIST	placelist;

	// 表示データ
	VecFx32	list_scale;
    GFL_G3D_OBJSTATUS	obj[WLDTIMER_PLACE_COL_NUM];
    VecFx32 obj_rotate[WLDTIMER_PLACE_COL_NUM];
#if WB_FIX
	GFL_G3D_OBJ 	mdl[WLDTIMER_PLACE_COL_NUM];
#else
	GFL_G3D_OBJ* 	g3dobj[WLDTIMER_PLACE_COL_NUM];
  GFL_G3D_RES*  p_mdlres[WLDTIMER_PLACE_COL_NUM];
  GFL_G3D_RND*  rnder[WLDTIMER_PLACE_COL_NUM];
#endif
} WLDTIMER_PLACE;

//-------------------------------------
///	カメラワーク
//=====================================
typedef struct {
	GFL_G3D_CAMERA *	p_camera;	// カメラオブジェ
#if WB_FIX    //このメンバ自体が使われていないので 2009.06.10(水)
	CAMERA_ANGLE	angle;		// カメラアングル
#endif
	fx32			dist;		// カメラ距離
	u16				status;		// カメラ状態
	u16				move;		// 動作フラグ
} WLDTIMER_CAMERA;

//-------------------------------------
///	地域情報１データ
//	サブ画面の情報ウインドウに渡すデータ
//=====================================
typedef struct {
	u32		timezone;		// 時間帯	sc_WLDTIMER_TIMEZONE_DATAのインデックス
	u32		nation;			// 国ID
	u32		area;			// 地域ID
} WLDTIMER_POINTDATA;

//-------------------------------------
///	朝、昼、夕方、夜、深夜　アニメデータ
//=====================================
typedef struct {

	// カウンタ
	u16 count;
	u16 count_max;

	// 各部分アニメONOFF
	u8	drawflag[4];		// 上中下

	// アニメリソース
	void*				p_scrnbuff[ WLDTIMER_TIMEZONEANM_SCRNANM_MAX ];	// スクリーンアニメバッファ
	NNSG2dScreenData*	p_scrndata[ WLDTIMER_TIMEZONEANM_SCRNANM_MAX ];	// frame数文スクリーンファイルがある
	u16					scrnframe;
	u16					scrnframe_now;
	void*				p_plttbuff;		// パレットアニメバッファ
	NNSG2dPaletteData*	p_plttdata;		// plttframe分の本数パレットがある
	u8					plttframe;
	u8					plttframe_now;	
	u8					plttno;			// 転送パレット
	u8					pad;
	
} WLDTIMER_TIMEZONEANM;

//-------------------------------------
///		フェードデータ
//=====================================
typedef struct {
	u8	start;	// カウント動作スタートフラグ
	s8	count;	// カウンター
	u8	pad[2];
} WLDTIMER_VIEWER_FADE;

//-------------------------------------
///	ポケモンバルーン動作
//	立て動作と横動作
//=====================================
typedef struct {
	// 現在座標
	s16 x;	
	s16 y;	

	s16 count;	// 動作カウント
	s16 wcount;	// ゆれカウント

	// ３つの位置が同じポケモンかのフラグ
	u16 under_equaldata;	
	u16 top_equaldata;	
	

	// 動作データ
	const WLDTIMER_POKEBLN_MOVEDATA* cp_data;

} WLDTIMER_POKEBLN_MOVE;


//-------------------------------------
///	ポケモン　風船
//=====================================
typedef struct {
	// データ
	WLDTIMER_POKEBLN_MOVE	move[ WLDTIMER_VIEWER_DRAWNUM ];
	s16						drawtype;		// 現在の表示タイプ	
	s16						wait;			// 次の開始時間
	
	// 描画フラグ
	u8					drawflag[4];	// 上中下
	u8					pokegra[4];		// 描画ポケモン

	// グラフィック
	GFL_CLWK*		p_act[ WLDTIMER_TIME_POKE_NUM ];
#if WB_FIX
	CLACT_U_RES_OBJ_PTR	p_res[ WLDTIMER_TIME_POKE_NUM ][ WLDTIMER_RESMAN_NUM ];
#else
  u32         p_resobj_index[ WLDTIMER_TIME_POKE_NUM ][ WLDTIMER_RESMAN_NUM ];
#endif
} WLDTIMER_POKEBALLOON;

//-------------------------------------
///	サブ画面ウィンドウワーク
//=====================================
typedef struct {
	s16 wnd0;		// データ設定フラグ
	s16 wnd1;		// データ設定フラグ
	s16 wnd0_sy;
	s16 wnd0_ey;
	s16 wnd1_sy;
	s16 wnd1_ey;

	GFL_TCB* p_tcb;	// Vブランクタスク
} WLDTIMER_VWND;


//-------------------------------------
///	サブ画面情報ウインドウ
//=====================================
typedef struct {
	// シーケンス
	u32 seq;
	
	// データキュー
	WLDTIMER_POINTDATA	data[ WLDTIMER_VIEWER_BUFFNUM ];
	u16 top;		// 先頭
	u16	tail;		// 末尾
	u32 datacount;	// 今入っている件数

	// フェード
	u16						fadecount;	// フェードカウンタ
	u16						fade_divnum;// 分割してフェードしていく数
	WLDTIMER_VIEWER_FADE	fade[ WLDTIMER_VIEWER_FADE_DIV ];
	u32						drawtype_zonetype[ WLDTIMER_VIEWER_DRAWNUM ];
	void*					p_fadescrnbuff;	// フェード用スクリーンアニメバッファ
	NNSG2dScreenData*		p_fadescrndata;	// フェード用スクリーンファイル

	// 描画データ
	u16						msg_now;						// 今のメッセージ
	u16						pad;							// アニメカウンタ
	WLDTIMER_TIMEZONEANM	anm[ WLDTIMER_ZONETYPE_NUM ];	// タイムゾーンアニメデータ
	GFL_BMPWIN*			msg[ WLDTIMER_VIEWER_DRAWNUM ];	// メッセージデータ
	GFL_BMPWIN*			dummy;							// フェード用メッセージ
	GFL_BMPWIN*			talkwin;						// 会話メッセージデータ
	WLDTIMER_POKEBALLOON	poke;							// ポケモンBALLOON
	WLDTIMER_VWND			wnd;							// ウィンドウ
	
	PRINT_UTIL print_util[WLDTIMER_VIEWER_DRAWNUM];
	PRINT_UTIL print_util_talk;
} WLDTIMER_VIEWER;



//-------------------------------------
///	タッチ管理
//=====================================
typedef struct {
	int tp_result;	// タッチをパッド情報に変更したもの
	int tp_seq;		// タッチシーケンス
	int tp_x;		// タッチｘ座標
	int tp_y;		// タッチｙ座標
	int tp_lenx;	// スライドｘ座標
	int tp_leny;	// スライドｙ座標
	int tp_count;	// ホールドカウント

	// ボタン表示
	GFL_BMPWIN *bttn;
  
  PRINT_UTIL print_util;
} WLDTIMER_TOUCH;


//-------------------------------------
///	終了表示
//=====================================
typedef struct {
	u32				seq;
#if WB_FIX
	u32				msg_no;
#else
	PRINT_STREAM*	print_stream;
#endif
	int				msg_wait;
	STRBUF*			p_str;
	GFL_BMPWIN	*win;
	TOUCH_SW_SYS*	p_touch_sw;
} WLDTIMER_END_MSG;


//-------------------------------------
///	メッセージシステム
//=====================================
typedef struct {
	GFL_MSGDATA*	p_msgman;		// 基本メッセージ
	WORDSET*			p_wordset;		// ワードセット
	STRBUF*				p_msgstr;		// メッセージバッファ
	STRBUF*				p_msgtmp;		// メッセージバッファ
} WLDTIMER_MSGMAN;



//-------------------------------------
///	世界時計ワーク
//=====================================
typedef struct {

	// 動作管理
	u16				main_seq;	// メイン画面シーケンス
	u16				sub_seq;	// サブ画面シーケンス
	WLDTIMER_FLAG	flag;		// 動作フラグ
	

	//セーブデータポインタ
	WIFI_HISTORY*	p_wifisv;
	CONFIG*			p_config;

	// 自分の登録地域
	int		my_nation;
	int		my_area;

	// 世界時間
	WFLBY_TIME	worldtime;

	// 表示システム
	WLDTIMER_DRAWSYS drawsys;

	// メッセージ管理システム
	WLDTIMER_MSGMAN msgman;

	// 地球儀管理
	WLDTIMER_EARTH earth;

	// 地点リスト
	WLDTIMER_PLACE place;
	
	// カメラ
	WLDTIMER_CAMERA camera;

	// タッチ管理
	WLDTIMER_TOUCH touch;

	// 情報ビューアー
	WLDTIMER_VIEWER view;

	// した画面終了チェック
	WLDTIMER_END_MSG end_msg;

	GFL_TCB *vintr_tcb;
	GFL_FONT *font_handle;
	PRINT_QUE *printQue;
  GFL_TCBLSYS *tcblsys;
} WLDTIMER_WK;






//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//	src/net_app/wifi_earth/wifi_earthからコピー
//	ただ、完全なコピーではなく、構造体の構成など変更してあります。
//-----------------------------------------------------------------------------
static void WLDTIMER_EarthListLoad( WLDTIMER_PLACE* p_wk, const WFLBY_WLDTIMER* cp_data, u32 heapID );
static void WLDTIMER_EarthListSet( WLDTIMER_PLACE* p_wk,u32 index, s16 x,s16 y,u16 nationID,u16 areaID, const WFLBY_WLDTIMER* cp_data );

static void WLDTIMER_EarthVecFx32_to_MtxFx33( MtxFx33* dst, const VecFx32* src );
static void WLDTIMER_EarthVecFx32_to_MtxFx33_place( MtxFx33* dst, const VecFx32* src );
static void WLDTIMER_Earth_TouchPanelParamGet( int prevx,int prevy,int* dirx_p,int* lenx_p,int* diry_p,int* leny_p );
static u32 WLDTIMER_Earth3D_Control( WLDTIMER_WK* p_wk,int keytrg,int keycont );

//-----------------------------------------------------------------------------


// ツール
static void WLDTIMER_MinusRotateChange( VecFx32* p_rot );
static BOOL WLDTIMER_RotateCheck( s32 minx, s32 maxx, u16 rotx );

// 全体
static BOOL WLDTIMER_WkMainControl( WLDTIMER_WK* p_wk );
static void WLDTIMER_WkSubControl( WLDTIMER_WK* p_wk );
static void WLDTIMER_WkDraw( WLDTIMER_WK* p_wk );
static void WLDTIMER_WkVBlank(GFL_TCB *tcb, void *p_work);

// 地球儀管理
static u32 WLDTIMER_EarthControl( WLDTIMER_WK* p_wk );
static void WLDTIMER_EarthStartSetUp( WLDTIMER_WK* p_wk, const WLDTIMER_PARAM* cp_param );
static BOOL WLDTIMER_EarthGetPointData( const WLDTIMER_WK* cp_wk, WLDTIMER_POINTDATA* p_data );
static u32 WLDTIMER_EarthGetPlaceData( const WLDTIMER_WK* cp_wk );
static u32 WLDTIMER_EarthGetRotateDist( const VecFx32* cp_earth, const VecFx32* cp_place );
static u32 WLDTIMER_EarthGetTimeZone( const WLDTIMER_WK* cp_wk, u32 placeindex, WFLBY_TIME worldtime );
static BOOL WLDTIMER_EarthSetNationAreaRotate( WLDTIMER_WK* p_wk, u8 nationID, u8 areaID );

// 動作フラグ 
static void WLDTIMER_FlagControl( WLDTIMER_WK* p_wk, const WLDTIMER_PARAM* cp_param );

// 表示システム
static void WLDTIMER_DrawSysInit( WLDTIMER_DRAWSYS* p_wk, CONFIG* p_config, u32 heapID );
static void WLDTIMER_DrawSysExit( WLDTIMER_DRAWSYS* p_wk );
static void WLDTIMER_DrawSysDraw( WLDTIMER_DRAWSYS* p_wk );
static void WLDTIMER_DrawSysVBlank( WLDTIMER_DRAWSYS* p_wk );
static void WLDTIMER_DrawSysBgInit( WLDTIMER_DRAWSYS* p_wk, CONFIG* p_config, u32 heapID );
static void WLDTIMER_DrawSysBgExit( WLDTIMER_DRAWSYS* p_wk );
static void WLDTIMER_DrawSysOamInit( WLDTIMER_DRAWSYS* p_wk, u32 heapID );
static void WLDTIMER_DrawSysOamExit( WLDTIMER_DRAWSYS* p_wk );
static void WLDTIMER_DrawSys3DInit( WLDTIMER_DRAWSYS* p_wk, u32 heapID );
static void WLDTIMER_DrawSys3DExit( WLDTIMER_DRAWSYS* p_wk );
static void WLDTIMER_DrawSys3DSetUp( void );

// 地球儀データ
static void WLDTIMER_EarthInit( WLDTIMER_EARTH* p_wk, WLDTIMER_DRAWSYS* p_drawsys, u32 heapID );
static void WLDTIMER_EarthExit( WLDTIMER_EARTH* p_wk );
static void WLDTIMER_EarthDraw( WLDTIMER_EARTH* p_wk );
static void WLDTIMER_EarthGetTrans( const WLDTIMER_EARTH* cp_wk, VecFx32* p_mat );
static void WLDTIMER_EarthGetRotMtx( const WLDTIMER_EARTH* cp_wk, MtxFx33* p_rot );
static void WLDTIMER_EarthGetRotaVec( const WLDTIMER_EARTH* cp_wk, VecFx32* p_vec );
static void WLDTIMER_EarthSetRotaVec( WLDTIMER_EARTH* p_wk, const VecFx32* cp_vec );

// 地域データ
static void WLDTIMER_PlaceInit( WLDTIMER_PLACE* p_wk, const WFLBY_WLDTIMER* cp_data, WLDTIMER_DRAWSYS* p_drawsys, WLDTIMER_FLAG flag, u32 heapID );
static void WLDTIMER_PlaceExit( WLDTIMER_PLACE* p_wk );
static void WLDTIMER_PlaceDraw( WLDTIMER_PLACE* p_wk, const WLDTIMER_EARTH* cp_earth );
static u32 WLDTIMER_PlaceGetCol( const WFLBY_WLDTIMER* cp_data, u16 nationID, u16 areaID );
static u32 WLDTIMER_PlaceGetData( const WLDTIMER_PLACE* cp_wk, u16 nationID, u16 areaID, VecFx32* p_vec );
static u32 WLDTIMER_PlaceGetIdx( const WLDTIMER_PLACE* cp_wk, u16 nationID, u16 areaID );
static void WLDTIMER_PlaceGetIdxRotVec( const WLDTIMER_PLACE* cp_wk, VecFx32* p_vec, int index );
static u32 WLDTIMER_PlaceGetIdxCol( const WLDTIMER_PLACE* cp_wk, int index );
static u32 WLDTIMER_PlaceGetIdxNationID( const WLDTIMER_PLACE* cp_wk, int index );
static u32 WLDTIMER_PlaceGetIdxAreaID( const WLDTIMER_PLACE* cp_wk, int index );
static u32 WLDTIMER_PlaceGetListNum( const WLDTIMER_PLACE* cp_wk );

// カメラデータ
static void WLDTIMER_CameraInit( WLDTIMER_CAMERA* p_wk, WLDTIMER_FLAG flag, u32 heapID );
static void WLDTIMER_CameraExit( WLDTIMER_CAMERA* p_wk );
static void WLDTIMER_CameraDraw( const WLDTIMER_CAMERA* cp_wk );
static void WLDTIMER_CameraMoveReq( WLDTIMER_CAMERA* p_wk );
static BOOL WLDTIMER_CameraMove( WLDTIMER_CAMERA* p_wk, WLDTIMER_PLACE* p_place );
static u32	WLDTIMER_CameraGetStatus( const WLDTIMER_CAMERA* cp_wk );

// タッチ管理
static void WLDTIMER_TouchInit( WLDTIMER_TOUCH* p_wk, WLDTIMER_DRAWSYS* p_drawsys, WLDTIMER_MSGMAN* p_msgman, u32 heapID, PRINT_QUE *printQue, GFL_FONT *font_handle );
static void WLDTIMER_TouchExit( WLDTIMER_TOUCH* p_wk );
static void WLDTIMER_TouchSetParam( WLDTIMER_TOUCH* p_touch );
static int WLDTIMER_TouchGetParam( const WLDTIMER_TOUCH* cp_touch, u32 type );
static void WLDTIMER_TouchBttnOff( WLDTIMER_TOUCH* p_wk );
static void WLDTIMER_TouchBttnOn( WLDTIMER_TOUCH* p_wk );

// 終了チェック管理
static void WLDTIMER_EndMsgInit( WLDTIMER_END_MSG* p_wk, WLDTIMER_DRAWSYS* p_drawsys, WLDTIMER_MSGMAN* p_msgman, SAVE_CONTROL_WORK* p_save, u32 heapID );
static void WLDTIMER_EndMsgExit( WLDTIMER_END_MSG* p_wk );
static void WLDTIMER_EndMsgStart( WLDTIMER_END_MSG* p_wk, GFL_FONT *font_handle, GFL_TCBLSYS *tcblsys, HEAPID heapID );
static u32 WLDTIMER_EndMsgMain( WLDTIMER_END_MSG* p_wk );
static void WLDTIMER_EndMsgEnd( WLDTIMER_END_MSG* p_wk );


// サブ画面情報ウィンドウ
static void WLDTIMER_ViewerInit( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys, WLDTIMER_MSGMAN* p_msgman, u32 heapID, GFL_FONT *font_handle, PRINT_QUE *printQue );
static void WLDTIMER_ViewerExit( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys );
static void WLDTIMER_ViewerMain( WLDTIMER_VIEWER* p_wk, WLDTIMER_MSGMAN* p_msgman, WLDTIMER_DRAWSYS* p_drawsys, PRINT_QUE *printQue, GFL_FONT *font_handle );
static void WLDTIMER_ViewerTalkWinOff( WLDTIMER_VIEWER* p_wk );
static void WLDTIMER_ViewerTalkWinOn( WLDTIMER_VIEWER* p_wk );
static BOOL WLDTIMER_ViewerPushData( WLDTIMER_VIEWER* p_wk, const WLDTIMER_POINTDATA* cp_data );
static BOOL WLDTIMER_ViewerEqualDataCheck( const WLDTIMER_VIEWER* cp_wk, const WLDTIMER_POINTDATA* cp_data );
static void WLDTIMER_ViewerQInit( WLDTIMER_VIEWER* p_wk );
static void WLDTIMER_ViewerQExit( WLDTIMER_VIEWER* p_wk );
static void WLDTIMER_ViewerQPush( WLDTIMER_VIEWER* p_wk, const WLDTIMER_POINTDATA* cp_data );
static void WLDTIMER_ViewerQPop( WLDTIMER_VIEWER* p_wk );
static BOOL WLDTIMER_ViewerQGetData( const WLDTIMER_VIEWER* cp_wk, WLDTIMER_POINTDATA* p_data, u32 index );
static void WLDTIMER_ViewerAnmCont( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys );
static void WLDTIMER_ViewerFadeInit( WLDTIMER_VIEWER* p_wk, WLDTIMER_MSGMAN* p_msgman, WLDTIMER_DRAWSYS* p_drawsys, PRINT_QUE *printQue, GFL_FONT *font_handle );
static BOOL WLDTIMER_ViewerFade( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys );
static void WLDTIMER_ViewerFade_SkipInit( WLDTIMER_VIEWER* p_wk );
static BOOL WLDTIMER_ViewerFade_Skip( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys );
static void WLDTIMER_ViewerFade_SkipInit01( WLDTIMER_VIEWER* p_wk );
static BOOL WLDTIMER_ViewerFade_Skip01( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys );
static BOOL WLDTIMER_ViewerFade_Main( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys, int fade_timing );
static void WLDTIMER_ViewerFadeDiv_Start( WLDTIMER_VIEWER* p_wk, u32 idx );
static BOOL WLDTIMER_ViewerFadeDiv_Main( WLDTIMER_VIEWER* p_wk, u32 idx, WLDTIMER_DRAWSYS* p_drawsys );
static void WLDTIMER_ViewerFadeScrn_LineTrans( WLDTIMER_VIEWER* p_wk, u32 y, WLDTIMER_DRAWSYS* p_drawsys );
static void WLDTIMER_ViewerWndInit( WLDTIMER_VWND* p_wk );
static void WLDTIMER_ViewerWndExit( WLDTIMER_VWND* p_wk );
static void WLDTIMER_ViewerWnd0SetPos( WLDTIMER_VWND* p_wk, s16 sy, s16 ey );
static void WLDTIMER_ViewerWnd1SetPos( WLDTIMER_VWND* p_wk, s16 sy, s16 ey );
static void WLDTIMER_ViewerWndTcb( GFL_TCB* p_tcb, void* p_work );


// 国文字列書き込み
static void WLDTIMER_ViewerMsgInit( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys, u32 heapID );
static void WLDTIMER_ViewerMsgExit( WLDTIMER_VIEWER* p_wk ); 
static void WLDTIMER_ViewerMsgWrite( WLDTIMER_VIEWER* p_wk, u32 drawtype, const WLDTIMER_POINTDATA* cp_data, WLDTIMER_MSGMAN* p_msgman, PRINT_QUE *printQue, GFL_FONT *font_handle );
static u8* WLDTIMER_ViewerMsgGetCharBuffPtr( WLDTIMER_VIEWER* p_wk, u32 drawtype, u32 y );
static u8* WLDTIMER_ViewerDummyGetCharBuffPtr( WLDTIMER_VIEWER* p_wk, u32 y );
static void WLDTIMER_ViewerMsgCharTrans( WLDTIMER_VIEWER* p_wk, u32 drawtype, u32 y, const u8* cp_buff );


// タイムゾーンアニメ
static void WLDTIMER_TimeZoneAnm_Init( WLDTIMER_TIMEZONEANM* p_wk, WLDTIMER_DRAWSYS* p_drawsys, const WLDTIMER_TIMEZONEANM_INIT* cp_init, u16 count_max, u32 heapID );
static void WLDTIMER_TimeZoneAnm_Exit( WLDTIMER_TIMEZONEANM* p_wk );
static void WLDTIMER_TimeZoneAnm_Main( WLDTIMER_TIMEZONEANM* p_wk, WLDTIMER_DRAWSYS* p_drawsys );
static void WLDTIMER_TimeZoneAnm_SetFlag( WLDTIMER_TIMEZONEANM* p_wk, u32 drawtype, BOOL flag );
static void WLDTIMER_TimeZoneAnm_ResetFlag( WLDTIMER_TIMEZONEANM* p_wk );
static void WLDTIMER_TimeZoneAnm_LineTrans( WLDTIMER_TIMEZONEANM* p_wk, u32 y, WLDTIMER_DRAWSYS* p_drawsys );


// ポケモンバルーン
static void WLDTIMER_PokeBaloon_Init( WLDTIMER_POKEBALLOON* p_wk, WLDTIMER_DRAWSYS* p_drawsys, WLDTIMER_VWND* p_wnd, u32 heapID );
static void WLDTIMER_PokeBaloon_Exit( WLDTIMER_POKEBALLOON* p_wk, WLDTIMER_DRAWSYS* p_drawsys );
static void WLDTIMER_PokeBaloon_Reset( WLDTIMER_POKEBALLOON* p_wk );
static void WLDTIMER_PokeBaloon_SetDraw( WLDTIMER_POKEBALLOON* p_wk, u8 drawtype, u8 timezone, u8 last_timezone );
static void WLDTIMER_PokeBaloon_CleanDraw( WLDTIMER_POKEBALLOON* p_wk );
static void WLDTIMER_PokeBaloon_Main( WLDTIMER_POKEBALLOON* p_wk, WLDTIMER_VWND* p_wnd );
static BOOL WLDTIMER_PokeBaloon_Start( WLDTIMER_POKEBALLOON* p_wk, u32 drawtype, WLDTIMER_VWND* p_wnd );
static BOOL WLDTIMER_PokeBaloon_Move( WLDTIMER_POKEBALLOON* p_wk, u32 drawtype, WLDTIMER_VWND* p_wnd );
static void WLDTIMER_PokeBln_MoveInit( WLDTIMER_POKEBLN_MOVE* p_wk, const WLDTIMER_POKEBLN_MOVEDATA* cp_data );
static void WLDTIMER_PokeBln_UnderEqualDataSet( WLDTIMER_POKEBLN_MOVE* p_wk, BOOL flag );
static void WLDTIMER_PokeBln_TopEqualDataSet( WLDTIMER_POKEBLN_MOVE* p_wk, BOOL flag );
static BOOL WLDTIMER_PokeBln_MoveMain( WLDTIMER_POKEBLN_MOVE* p_wk, WLDTIMER_VWND* p_wnd );
static void WLDTIMER_PokeBln_MoveReset( WLDTIMER_POKEBLN_MOVE* p_wk, WLDTIMER_VWND* p_wnd );
static void WLDTIMER_PokeBln_MoveGetPos( const WLDTIMER_POKEBLN_MOVE* cp_wk, VecFx32* p_pos );
static void WLDTIMER_PokeBln_MatrixCalc( WLDTIMER_POKEBLN_MOVE* p_wk );
static void WLDTIMER_PokeBln_WndMskSet( const WLDTIMER_POKEBLN_MOVE* cp_wk, WLDTIMER_VWND* p_wnd );
static void WLDTIMER_PokeBln_ActSetMatrix( WLDTIMER_POKEBALLOON* p_wk, u32 drawtype );


// タイムゾーンデータ取得
static u16 WLDTIMER_TIMEZONE_GetZoneType( u32 timezone );
static u16 WLDTIMER_TIMEZONE_GetMons( u32 timezone );

// メッセージシステム
static void WLDTIMER_MsgManInit( WLDTIMER_MSGMAN* p_wk, u32 heapID );
static void WLDTIMER_MsgManExit( WLDTIMER_MSGMAN* p_wk );
static STRBUF* WLDTIMER_MsgManGetStr( WLDTIMER_MSGMAN* p_wk, u32 msg );
static STRBUF* WLDTIMER_MsgManCountryGetStr( WLDTIMER_MSGMAN* p_wk, u32 nationID );
static STRBUF* WLDTIMER_MsgManPlaceGetStr( WLDTIMER_MSGMAN* p_wk, u32 nationID, u32 areaID );
static void WLDTIMER_MsgManGetStrBuff( WLDTIMER_MSGMAN* p_wk, u32 msg, STRBUF* p_str );

// 表示データベース
static u32 WLDTIMER_WFLBYDATA_GetFirst( const WFLBY_WLDTIMER* cp_data );

static void _SetCameraDistance(GFL_G3D_CAMERA *camera, fx32 distance);




//----------------------------------------------------------------------------
/**
 *	@brief	世界時計	初期化
 *
 *	@param	p_proc		プロックワーク
 *	@param	p_seq		シーケンス
 *
 *	@retval	GFL_PROC_RES_CONTINUE = 0,		///<動作継続中
 *	@retval	GFL_PROC_RES_FINISH,			///<動作終了
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WLDTIMER_Init(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk)
{
	WLDTIMER_WK* p_wk;
	WLDTIMER_PARAM* p_param;

	// 全地域表示でバック機能
#ifdef WLDTIMER_DEBUG_ALLPLACEOPEN
	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ){
		s_WLDTIMER_DEBUG_ALLPLACEOPEN_FLAG = TRUE;
	}else{
		s_WLDTIMER_DEBUG_ALLPLACEOPEN_FLAG = FALSE;
	}
#endif
	
	p_param = pwk;

	OS_TPrintf( "end cgx %d\n", WLDTIMER_MAIN_SUBBTTN_CGXEND );

	//ヒープエリア作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WLDTIMER, 0x50000 );

	// ワーク作成
	p_wk = GFL_PROC_AllocWork( p_proc, sizeof(WLDTIMER_WK), HEAPID_WLDTIMER );
	GFL_STD_MemFill( p_wk, 0, sizeof(WLDTIMER_WK) );

	// セーブデータ取得
	p_wk->p_wifisv = SaveData_GetWifiHistory( p_param->p_save );
	p_wk->p_config = SaveData_GetConfig( p_param->p_save );

	// 自分の登録地域取得
	p_wk->my_nation = WIFIHISTORY_GetMyNation( p_wk->p_wifisv );
	p_wk->my_area = WIFIHISTORY_GetMyArea( p_wk->p_wifisv );

	// 世界時間
	p_wk->worldtime = p_param->worldtime;

//	OS_TPrintf( "world time hour[%d] minute[%d] second[%d]\n", p_wk->worldtime.hour, p_wk->worldtime.minute, p_wk->worldtime.second );

  //フォント読み込み
  p_wk->font_handle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_nftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, HEAPID_WLDTIMER );
	p_wk->printQue = PRINTSYS_QUE_Create(HEAPID_WLDTIMER);
	p_wk->tcblsys = GFL_TCBL_Init(HEAPID_WLDTIMER, HEAPID_WLDTIMER, 8, 32);
  OS_TPrintf("0 printQue = %x\n", p_wk->printQue);
  
	// フラグ初期化
	WLDTIMER_FlagControl( p_wk, p_param );
  OS_TPrintf("1 printQue = %x\n", p_wk->printQue);

	// グラフィック初期化
	WLDTIMER_DrawSysInit( &p_wk->drawsys, p_wk->p_config, HEAPID_WLDTIMER );
  OS_TPrintf("2 printQue = %x\n", p_wk->printQue);

	// 地球儀初期化
	WLDTIMER_EarthInit( &p_wk->earth, &p_wk->drawsys, HEAPID_WLDTIMER );
  OS_TPrintf("3 printQue = %x\n", p_wk->printQue);

	// 文字システム初期化
	WLDTIMER_MsgManInit( &p_wk->msgman, HEAPID_WLDTIMER );
  OS_TPrintf("4 printQue = %x\n", p_wk->printQue);

	// 地域初期化
	WLDTIMER_PlaceInit( &p_wk->place, p_param->cp_data, &p_wk->drawsys, p_wk->flag, HEAPID_WLDTIMER );
  OS_TPrintf("5 printQue = %x\n", p_wk->printQue);

	// カメラ初期化
	WLDTIMER_CameraInit( &p_wk->camera, p_wk->flag, HEAPID_WLDTIMER );
  OS_TPrintf("6 printQue = %x\n", p_wk->printQue);

	// タッチ初期化
	WLDTIMER_TouchInit( &p_wk->touch, &p_wk->drawsys, &p_wk->msgman, HEAPID_WLDTIMER, p_wk->printQue, p_wk->font_handle );
  OS_TPrintf("6 printQue = %x\n", p_wk->printQue);

	// 終了チェック初期化
	WLDTIMER_EndMsgInit( &p_wk->end_msg, &p_wk->drawsys, &p_wk->msgman, p_param->p_save, HEAPID_WLDTIMER );
  OS_TPrintf("7 printQue = %x\n", p_wk->printQue);

	// ビューアー初期化
	WLDTIMER_ViewerInit( &p_wk->view, &p_wk->drawsys, &p_wk->msgman, HEAPID_WLDTIMER, p_wk->font_handle, p_wk->printQue );
  OS_TPrintf("8 printQue = %x\n", p_wk->printQue);

	// 地球儀開始セットアップ
	WLDTIMER_EarthStartSetUp( p_wk, p_param );

	
	// 割り込み設定
	p_wk->vintr_tcb = GFUser_VIntr_CreateTCB(WLDTIMER_WkVBlank, p_wk, 200);
	//sys_HBlankIntrStop();	//HBlank割り込み停止

	return	GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	世界時計	メイン
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WLDTIMER_Main(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk)
{
	WLDTIMER_WK* p_wk;
	WLDTIMER_PARAM* p_param;
	BOOL result;
	
	p_wk	= mywk;
	p_param = pwk;



#ifdef WFLBY_DEBUG_ROOM_WLDTIMER_AUTO
	WFLBY_DEBUG_ROOM_WFLBY_TIMER_AUTO = TRUE;
	GFL_UI_KEY_GetTrg()		|= PAD_BUTTON_B;
	GFL_UI_KEY_GetCont()	|= PAD_BUTTON_B;
#endif
	
  GFL_TCBL_Main(p_wk->tcblsys);
	PRINTSYS_QUE_Main(p_wk->printQue);
	{
    int i;
    PRINT_UTIL_Trans(&p_wk->touch.print_util, p_wk->printQue);
    for(i = 0; i < WLDTIMER_VIEWER_DRAWNUM; i++){
      PRINT_UTIL_Trans(&p_wk->view.print_util[i], p_wk->printQue);
    }
    PRINT_UTIL_Trans(&p_wk->view.print_util_talk, p_wk->printQue);
  }

	switch( *p_seq ){
	case WLDTIMER_SEQ_FADEIN:
		WIPE_SYS_Start(WIPE_PATTERN_WMS, 
				WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, 
				WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_WLDTIMER );

		// BGMをフェードアウト
		WFLBY_SYSTEM_SetBGMVolumeDown( p_param->p_system, TRUE );

		// ロビーに入ってきた音を出す
//		PMSND_PlaySE( WFLBY_SND_WLDTMIN );

		(*p_seq) ++;
		break;
		
	case WLDTIMER_SEQ_FADEINWAIT:
		result = WIPE_SYS_EndCheck();
		if( result == TRUE ){
			(*p_seq) ++;
		}
		break;
		
	case WLDTIMER_SEQ_MAIN:
		result = WLDTIMER_WkMainControl( p_wk );
		WLDTIMER_WkSubControl( p_wk );

		if( WFLBY_SYSTEM_Event_GetEndCM( p_param->p_system ) == TRUE ){
			WFLBY_SYSTEM_APLFLAG_SetForceEnd( p_param->p_system );
			result = TRUE;
		}

		if( WFLBY_ERR_CheckError() == TRUE ){
			result = TRUE;
		}
		
		if( result == TRUE ){	// 終了チェック
			(*p_seq) = WLDTIMER_SEQ_FADEOUT;
		}
		break;
		
	case WLDTIMER_SEQ_FADEOUT:
		WIPE_SYS_Start(WIPE_PATTERN_WMS, 
				WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, 
				WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, HEAPID_WLDTIMER );
		(*p_seq) ++;
		break;
		
	case WLDTIMER_SEQ_FADEOUTWAIT:
		result = WIPE_SYS_EndCheck();
		if( result == TRUE ){
			return GFL_PROC_RES_FINISH;
		}
		break;
	}

	// 描画
	WLDTIMER_WkDraw( p_wk );

	return	GFL_PROC_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	世界時計	破棄
 */
//-----------------------------------------------------------------------------
GFL_PROC_RESULT WLDTIMER_Exit(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk)
{
	WLDTIMER_WK* p_wk;
	WLDTIMER_PARAM* p_param;
	
	p_wk	= mywk;
	p_param = pwk;

	// 割り込み設定
	GFL_TCB_DeleteTask(p_wk->vintr_tcb);
	//sys_HBlankIntrStop();	//HBlank割り込み停止

	// ビューアー破棄
	WLDTIMER_ViewerExit( &p_wk->view, &p_wk->drawsys );

	// 終了チェック破棄
	WLDTIMER_EndMsgExit( &p_wk->end_msg );

	// タッチ破棄
	WLDTIMER_TouchExit( &p_wk->touch );

	// カメラ破棄
	WLDTIMER_CameraExit( &p_wk->camera );

	// 地球儀破棄
	WLDTIMER_EarthExit( &p_wk->earth );

	// 地域データ破棄
	WLDTIMER_PlaceExit( &p_wk->place );

	// 文字システム破棄
	WLDTIMER_MsgManExit( &p_wk->msgman );

	// 描画システム破棄
	WLDTIMER_DrawSysExit( &p_wk->drawsys );

  //フォント破棄
	GFL_FONT_Delete(p_wk->font_handle);
	//PrintQue破棄
	PRINTSYS_QUE_Delete(p_wk->printQue);
	//TCBL破棄
	GFL_TCBL_Exit(p_wk->tcblsys);

	//ワーク破棄
	GFL_PROC_FreeWork( p_proc );
	
	//ヒープ破棄
	GFL_HEAP_DeleteHeap( HEAPID_WLDTIMER );

	return	GFL_PROC_RES_FINISH;
}




#ifdef  PM_DEBUG
GFL_PROC_RESULT WLDTIMER_DebugInit(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk)
{

	return WLDTIMER_Init( p_proc, p_seq, pwk, mywk );
}

GFL_PROC_RESULT WLDTIMER_DebugExit(GFL_PROC* p_proc, int* p_seq, void * pwk, void * mywk)
{
	GFL_PROC_RESULT result;

	result = WLDTIMER_Exit( p_proc, p_seq, pwk, mywk );

	return result;
}
#endif



//-----------------------------------------------------------------------------
/**
 *			プライベート関数
 */
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//	src/net_app/wifi_earth/wifi_earthからコピー
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	地点リストを読み込む
 *
 *	@param	p_wk	ワーク
 *	@param	cp_data	表示地域データバッファ
 *	@param	heapID	ヒープID
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EarthListLoad( WLDTIMER_PLACE* p_wk, const WFLBY_WLDTIMER* cp_data, u32 heapID )
{
	ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_WIFI_EARTH_PLACE, heapID );
	
	//地点リスト総数初期化
	p_wk->placelist.listcount = 0;

	{//地点マーク回転初期化（国データバイナリデータロード）
		void* filep;
		EARTH_DATA_NATION* listp;
		u32	size;
		int	i,listcount;

		filep = GFL_ARCHDL_UTIL_LoadEx( p_handle, NARC_wifi_earth_place_place_pos_wrd_dat, 
								FALSE, heapID, &size );

		listp = (EARTH_DATA_NATION*)filep;	//ファイル読み込み用に変換
		listcount = size/6;				//地点数取得（データ長：１地点につき６バイト）

		listp++;	//1originのため読み飛ばし
		for(i=1;i<listcount;i++){
			if(listp->flag != 2){	//2の場合は地域リストが存在する
				WLDTIMER_EarthListSet(p_wk,p_wk->placelist.listcount,
						listp->x,listp->y,i,0, cp_data);
				p_wk->placelist.listcount++;
			}
			listp++;
		}
		GFL_HEAP_FreeMemory(filep);
	}
	{//地点マーク回転初期化（地域データバイナリデータロード）
		void* filep;
		EARTH_DATA_AREA* listp;
		u32	size, data_id;
		int	i,datLen,index,listcount;

		index = 1;	//1orgin
		datLen = WIFI_COUNTRY_GetDataLen();

		while(index < datLen){

			data_id = WIFI_COUNTRY_DataIndexToPlaceDataID( index );
			filep = GFL_ARCHDL_UTIL_LoadEx( p_handle, data_id, FALSE, 
									heapID, &size );

			listp = (EARTH_DATA_AREA*)filep;	//ファイル読み込み用に変換
			listcount = size/4;		//地点数取得（データ長：１地点につき４バイト）

			listp++;	//1originのため読み飛ばし
			for(i=1;i<listcount;i++){//1orgin
				WLDTIMER_EarthListSet(p_wk,p_wk->placelist.listcount,listp->x,listp->y,
								WIFI_COUNTRY_DataIndexToCountryCode(index),i, cp_data);
				p_wk->placelist.listcount++;
				listp++;
			}
			GFL_HEAP_FreeMemory(filep);
			index++;
		}
	}

	GFL_ARC_CloseDataHandle( p_handle );
}

//----------------------------------------------------------------------------
/**
 *	@brief	地点情報をリストに設定
 *
 *	@param	p_wk		ワーク
 *	@param	index		インデックス
 *	@param	x			ｘ相対回転角度
 *	@param	y			ｙ相対回転角度
 *	@param	nationID	国ID
 *	@param	areaID		地域ID
 *	@param	cp_data		表示地域データバッファ
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EarthListSet( WLDTIMER_PLACE* p_wk,u32 index, s16 x,s16 y,u16 nationID,u16 areaID, const WFLBY_WLDTIMER* cp_data )
{
	MtxFx33 rotMtx = {FX32_ONE,0,0,0,FX32_ONE,0,0,0,FX32_ONE};
	VecFx32 rotVec;

	p_wk->placelist.place[index].x = x;	//Ｘ回転オフセット取得
	p_wk->placelist.place[index].y = y;	//Ｙ回転オフセット取得
	
	rotVec.x = x;
	rotVec.y = y;
	rotVec.z = INIT_EARTH_ROTATE_ZVAL;
	WLDTIMER_EarthVecFx32_to_MtxFx33_place(&rotMtx,&rotVec);	//初期位置からの相対座標計算
	p_wk->placelist.place[index].rotate = rotMtx;

	//マーク色設定(退室フラグにあわせる)
#ifdef WLDTIMER_DEBUG_ALLPLACEOPEN
	if( s_WLDTIMER_DEBUG_ALLPLACEOPEN_FLAG == TRUE ){
		p_wk->placelist.place[index].col = WLDTIMER_PLACE_COL_IN;
	}else{
		if( cp_data != NULL ){
			p_wk->placelist.place[index].col = WLDTIMER_PlaceGetCol( cp_data, nationID, areaID );
		}
	}
#else
	if( cp_data != NULL ){
		p_wk->placelist.place[index].col = WLDTIMER_PlaceGetCol( cp_data, nationID, areaID );
	}
#endif
//	OS_Printf( "p_wk->placelist.place[index].col %d  nation %d  area %d\n", p_wk->placelist.place[index].col, nationID, areaID );

	p_wk->placelist.place[index].nationID = nationID;	//該当国ＩＤ
	p_wk->placelist.place[index].areaID = areaID;		//該当地域ＩＤ
}


//----------------------------------------------------------------------------
/**
 *	@brief	3D回転計算
 *
 *	@param	dst		書き出し先行列
 *	@param	src		回転ベクトル
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EarthVecFx32_to_MtxFx33( MtxFx33* dst, const VecFx32* src )
{
	MtxFx33 tmp;

	MTX_RotY33(	dst,FX_SinIdx((u16)src->y),FX_CosIdx((u16)src->y));

	MTX_RotX33(	&tmp,FX_SinIdx((u16)src->x),FX_CosIdx((u16)src->x));
	MTX_Concat33(dst,&tmp,dst);

	MTX_RotZ33(	&tmp,FX_SinIdx((u16)src->z),FX_CosIdx((u16)src->z));
	MTX_Concat33(dst,&tmp,dst);
}

//----------------------------------------------------------------------------
/**
 *	@brief	３D回転計算	相対回転
 *
 *	@param	dst		書き出し先行列
 *	@param	src		回転ベクトル
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EarthVecFx32_to_MtxFx33_place( MtxFx33* dst, const VecFx32* src )
{
	MtxFx33 tmp;

	MTX_RotY33(	dst,FX_SinIdx((u16)src->x),FX_CosIdx((u16)src->x));

	MTX_RotX33(	&tmp,FX_SinIdx((u16)-src->y),FX_CosIdx((u16)-src->y));
	MTX_Concat33(dst,&tmp,dst);

	MTX_RotZ33(	&tmp,FX_CosIdx((u16)src->z),FX_SinIdx((u16)src->z));
	MTX_Concat33(dst,&tmp,dst);
}

static void WLDTIMER_Earth_TouchPanelParamGet( int prevx,int prevy,int* dirx_p,int* lenx_p,int* diry_p,int* leny_p )
{
	int x_dir = 0;
	int y_dir = 0;
	int x_len = 0;
	int y_len = 0;
  u32 tp_x = 0, tp_y = 0;
  
	GFL_UI_TP_GetPointCont(&tp_x, &tp_y);
	
	//Ｘ方向＆移動幅取得
	if(tp_x != 0xffff){
		x_len = tp_x - prevx;
		if(x_len < 0){
			x_len ^= -1;
			x_dir = PAD_KEY_RIGHT;
		}else{
			if(x_len > 0){
				x_dir = PAD_KEY_LEFT;
			}
		}
	}
	x_len &= WLDTIMER_TOUCH_LEN_LIMIT;	//リミッター
	*dirx_p = x_dir;
	*lenx_p = x_len;

	//Ｙ方向＆移動幅取得
	if(tp_y != 0xffff){
		y_len = tp_y - prevy;
		if(y_len < 0){
			y_len ^= -1;
			y_dir = PAD_KEY_DOWN;
		}else{
			if(y_len > 0){
				y_dir = PAD_KEY_UP;
			}
		}
	}
	y_len &= WLDTIMER_TOUCH_LEN_LIMIT;	//リミッター
	*diry_p = y_dir;
	*leny_p = y_len;
}

//----------------------------------
//地球操作関数
//----------------------------------
static u32 WLDTIMER_Earth3D_Control( WLDTIMER_WK* p_wk,int keytrg,int keycont )
{
	u16 rotate_speed_x;
	u16 rotate_speed_y;
	s16 rotate_x;
	s16 rotate_y;
	u32 result = WLDTIMER_EARTHCONT_RET_NONE;
	VecFx32 rotate;
	int tp_result;
	int tp_lenx, tp_leny;
	u32 camera_status;

	// 各データ取得
	WLDTIMER_EarthGetRotaVec( &p_wk->earth, &rotate );	// 地球儀回転ベクトル
	tp_result	= WLDTIMER_TouchGetParam(				// タッチ結果パッド値
					&p_wk->touch, WLDTIMER_TOUCH_PM_RESULT );
	tp_lenx		= WLDTIMER_TouchGetParam(				// タッチスライドｘ値
					&p_wk->touch, WLDTIMER_TOUCH_PM_LENX );
	tp_leny		= WLDTIMER_TouchGetParam(				// タッチスライドｙ値
					&p_wk->touch, WLDTIMER_TOUCH_PM_LENY );
	camera_status = WLDTIMER_CameraGetStatus(			// カメラ状態
					&p_wk->camera );	

	rotate_x = rotate.x;
	rotate_y = rotate.y;

	//カメラ遠近移動判定（世界地球儀モードのみ）
	if((keytrg & PAD_BUTTON_A)||(tp_result & PAD_BUTTON_A)){
		if(p_wk->flag.world == GLOBAL_MODE){
			result = WLDTIMER_EARTHCONT_RET_CAMERA_MOVE;
		}
		return result;
	}
	//カメラ移動スピード初期設定
	if(camera_status == CAMERA_FAR){
		//遠距離
		if((tp_lenx)||(tp_leny)){ //タッチパネル入力あり
			rotate_speed_x = (CAMERA_MOVE_SPEED_FAR/WLDTIMER_TOUCH_EARTHSPEED_DIVFAR) * tp_lenx;
			rotate_speed_y = (CAMERA_MOVE_SPEED_FAR/WLDTIMER_TOUCH_EARTHSPEED_DIVFAR) * tp_leny;
		}else{
			rotate_speed_x = CAMERA_MOVE_SPEED_FAR;
			rotate_speed_y = CAMERA_MOVE_SPEED_FAR;
		}
	}else{
		//近距離
		if((tp_lenx)||(tp_leny)){ //タッチパネル入力あり
			rotate_speed_x = (CAMERA_MOVE_SPEED_NEAR/WLDTIMER_TOUCH_EARTHSPEED_DIVNEAR) * tp_lenx;
			rotate_speed_y = (CAMERA_MOVE_SPEED_NEAR/WLDTIMER_TOUCH_EARTHSPEED_DIVNEAR) * tp_leny;
		}else{
			rotate_speed_x = CAMERA_MOVE_SPEED_NEAR;
			rotate_speed_y = CAMERA_MOVE_SPEED_NEAR;
		}
	}
	//カメラ上下左右移動判定
	if((keycont & PAD_KEY_LEFT)||(tp_result & PAD_KEY_LEFT)){
		if(p_wk->flag.world == GLOBAL_MODE){
			rotate.y += rotate_speed_x;
		}else{
			//if((u16)wk->rotate.y < EARTH_LIMIT_ROTATE_YMAX){
			if(rotate_y < (s16)EARTH_LIMIT_ROTATE_YMAX){
				rotate.y += rotate_speed_x;
			}
		}
	}
	if((keycont & PAD_KEY_RIGHT)||(tp_result & PAD_KEY_RIGHT)){
		if(p_wk->flag.world == GLOBAL_MODE){
			rotate.y -= rotate_speed_x;
		}else{
			//if((u16)wk->rotate.y > EARTH_LIMIT_ROTATE_YMIN){
			if(rotate_y > (s16)EARTH_LIMIT_ROTATE_YMIN){
				rotate.y -= rotate_speed_x;
			}
		}
	}
	if((keycont & PAD_KEY_UP)||(tp_result & PAD_KEY_UP)){
		if(p_wk->flag.world == GLOBAL_MODE){
			//if(((u16)(wk->rotate.x + rotate_speed_y)) < CAMERA_ANGLE_MAX){
			if((rotate_x + rotate_speed_y) < CAMERA_ANGLE_MAX){
				rotate.x += rotate_speed_y;
			}else{
				rotate.x = CAMERA_ANGLE_MAX;
			}
		}else{
			//if((u16)wk->rotate.x < EARTH_LIMIT_ROTATE_XMAX){
			if(rotate_x < (s16)EARTH_LIMIT_ROTATE_XMAX){
				rotate.x += rotate_speed_y;
			}
		}
	}
	if((keycont & PAD_KEY_DOWN)||(tp_result & PAD_KEY_DOWN)){
		if(p_wk->flag.world == GLOBAL_MODE){
			//if(((u16)(wk->rotate.x - rotate_speed_y)) > CAMERA_ANGLE_MIN){
			if((rotate_x - rotate_speed_y) > CAMERA_ANGLE_MIN){
				rotate.x -= rotate_speed_y;
			}else{
				rotate.x = CAMERA_ANGLE_MIN;
			}
		}else{
			//if((u16)wk->rotate.x > EARTH_LIMIT_ROTATE_XMIN){
			if(rotate_x > (s16)EARTH_LIMIT_ROTATE_XMIN){
				rotate.x -= rotate_speed_y;
			}
		}
	}
	WLDTIMER_EarthSetRotaVec( &p_wk->earth, &rotate );
	return result;
}
//------------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief	マイナスの値の回転角度や３６０度以上の回転角度をなくす
 *
 *	@param	p_rot	回転ベクトル
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_MinusRotateChange( VecFx32* p_rot )
{
	if( p_rot->x >= 0 ){
		p_rot->x = p_rot->x % 0xffff;
	}else{
		p_rot->x = p_rot->x + ( 0xffff* ((MATH_ABS( p_rot->x )/0xffff)+1) );
	}
	if( p_rot->y >= 0 ){
		p_rot->y = p_rot->y % 0xffff;
	}else{
		p_rot->y = p_rot->y + ( 0xffff* ((MATH_ABS( p_rot->y )/0xffff)+1) );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	回転角度チェック
 *
 *	@retval	TRUE	はいっている
 *	@retval	FALSE	はいっていない
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_RotateCheck( s32 minx, s32 maxx, u16 rotx )
{
	if( minx < 0 ){
		minx += 0xffff;
		if( ((minx < rotx) && (0xffff > rotx)) ||
			((0 < rotx) && (maxx > rotx)) ){
			return TRUE;
		}
	}else{
		if( ((minx < rotx) && (maxx > rotx)) ){ 
			return TRUE;
		}
	}
	return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief	地球儀画面側制御
 *
 *	@param	p_wk	ワーク
 *
 *	@retval	TRUE	終了
 *	@retval	FALSE	続行
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_WkMainControl( WLDTIMER_WK* p_wk )
{
	u32 result;

#ifdef WLDTIMER_DEBUG_TIMEZONE
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){
		s_WLDTIMER_DEBUG_TimeZone = (s_WLDTIMER_DEBUG_TimeZone + 2) % WLDTIMER_TIMEZONE_DATANUM;
		OS_Printf( "debug timezone %d\n", s_WLDTIMER_DEBUG_TimeZone );
	}
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){
		if( (s_WLDTIMER_DEBUG_TimeZone - 1) >= 0 ){
			s_WLDTIMER_DEBUG_TimeZone--;
		}else{
			s_WLDTIMER_DEBUG_TimeZone = WLDTIMER_TIMEZONE_DATANUM-2;
		}
		OS_Printf( "debug timezone %d\n", s_WLDTIMER_DEBUG_TimeZone );
	}
#endif

	
	switch( p_wk->main_seq ){
	// タッチ処理
	case WLDTIMER_MAINSEQ_CONTROL:
		result = WLDTIMER_EarthControl( p_wk );
		switch( result ){
		case WLDTIMER_EARTHCONT_RET_NONE:		
			break;
			
		case WLDTIMER_EARTHCONT_RET_CAMERA_MOVE:
			WLDTIMER_CameraMoveReq( &p_wk->camera );
			p_wk->main_seq = WLDTIMER_MAINSEQ_CAMERAMOVE;
			break;
			
		case WLDTIMER_EARTHCONT_RET_END:
			p_wk->main_seq = WLDTIMER_MAINSEQ_ENDCHECK;	// 終了チェック
			break;
		}
		break;
		
	// カメラ動作
	case WLDTIMER_MAINSEQ_CAMERAMOVE:
		result = WLDTIMER_CameraMove( &p_wk->camera, &p_wk->place );
		if( result == TRUE ){
			p_wk->main_seq = WLDTIMER_MAINSEQ_CONTROL;
		}
		break;

	case WLDTIMER_MAINSEQ_ENDCHECK:
		WLDTIMER_EndMsgStart( &p_wk->end_msg, p_wk->font_handle, p_wk->tcblsys, HEAPID_WLDTIMER );
		WLDTIMER_ViewerTalkWinOff( &p_wk->view );
		WLDTIMER_TouchBttnOff( &p_wk->touch );
		p_wk->main_seq ++;
		break;
		
	case WLDTIMER_MAINSEQ_ENDCHECKWAIT:
		{
			u32 result;
			result = WLDTIMER_EndMsgMain( &p_wk->end_msg );
			switch( result ){
			case TOUCH_SW_RET_NORMAL:	// 何もなし
				break;

			case TOUCH_SW_RET_NO:		// いいえ
				WLDTIMER_ViewerTalkWinOn( &p_wk->view );
				WLDTIMER_EndMsgEnd( &p_wk->end_msg );
				WLDTIMER_TouchBttnOn( &p_wk->touch );
				p_wk->main_seq = WLDTIMER_MAINSEQ_CONTROL;
				break;;

			default:
			case TOUCH_SW_RET_YES:		// はい
				return TRUE;

			}
		}
		break;

	default:
		GF_ASSERT(0);
		break;
	}

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	タスク画面側制御
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_WkSubControl( WLDTIMER_WK* p_wk )
{
	switch( p_wk->sub_seq ){
	case WLDTIMER_SUB_SEQ_CONTROL:
		WLDTIMER_ViewerMain( &p_wk->view,  &p_wk->msgman, &p_wk->drawsys, p_wk->printQue, p_wk->font_handle );
		break;
	}

}

//----------------------------------------------------------------------------
/**
 *	@brief	表示処理
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_WkDraw( WLDTIMER_WK* p_wk )
{
	GFL_G3D_DRAW_Start();
	
	// カメラ設定
	WLDTIMER_CameraDraw( &p_wk->camera );
	
	// 地球儀描画
	WLDTIMER_EarthDraw( &p_wk->earth );

	// 地域データ描画
	WLDTIMER_PlaceDraw( &p_wk->place, &p_wk->earth );

	// 描画システム描画
	WLDTIMER_DrawSysDraw( &p_wk->drawsys );

	// 
	GFL_G3D_DRAW_End();
}

//----------------------------------------------------------------------------
/**
 *	@brief	VBLANK処理
 *
 *	@param	p_work 
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_WkVBlank(GFL_TCB *tcb, void *p_work)
{
	WLDTIMER_WK* p_wk = p_work;

	WLDTIMER_DrawSysVBlank( &p_wk->drawsys );
}


//----------------------------------------------------------------------------
/**
 *	@brief	地球儀管理
 *
 *	@param	p_wk	ワーク
 *
 *	@retval	WLDTIMER_EARTHCONT_RET_NONE,			// 何もなし
 *	@retval	WLDTIMER_EARTHCONT_RET_CAMERA_MOVE,		// カメラ動作
 *	@retval	WLDTIMER_EARTHCONT_RET_END,				// 終了
 */
//-----------------------------------------------------------------------------
static u32 WLDTIMER_EarthControl( WLDTIMER_WK* p_wk )
{
	u32 ret;
	BOOL result;
	
	WLDTIMER_TouchSetParam( &p_wk->touch );

	//終了判定
	if((p_wk->touch.tp_result & PAD_BUTTON_B))
	{
		PMSND_PlaySE( WLDTIMER_SND_YAMERU );
		return WLDTIMER_EARTHCONT_RET_END;
	}
	else{
		// 位置選択
		if( (GFL_UI_KEY_GetTrg() & PAD_BUTTON_X) ){
			// 地球儀がさしている位置の地域データを取得
			{
				WLDTIMER_POINTDATA draw_point;
				result = WLDTIMER_EarthGetPointData( p_wk, &draw_point );
				if( result == TRUE ){

#ifdef WLDTIMER_DEBUG_TIMEZONE
					result = FALSE;	// 重複チェックOFF
#else
					// 一緒のデータがないかチェック
					result = WLDTIMER_ViewerEqualDataCheck( &p_wk->view, &draw_point );
#endif
					if( result == FALSE ){

						// さしている地域があったので情報画面に表示
						OS_Printf( "あった！\n" );
						result = WLDTIMER_ViewerPushData( &p_wk->view, &draw_point );
						if( result == FALSE ){
							OS_Printf( "今は無理みたい。\n" );
						}else{
							// その位置にあわせる
							WLDTIMER_EarthSetNationAreaRotate( p_wk, 
									draw_point.nation, draw_point.area );
							PMSND_PlaySE( WLDTIMER_SND_XSELECT );
						}
					}
				}
			}
		}else{

			//地球回転コントロール
			ret = WLDTIMER_Earth3D_Control(p_wk, GFL_UI_KEY_GetTrg(), GFL_UI_KEY_GetCont());
		}
	}

	return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief	地球儀回転角度の初期設定
 *
 *	@param	p_wk		ワーク
 *	@param	cp_param	表示データベース
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EarthStartSetUp( WLDTIMER_WK* p_wk, const WLDTIMER_PARAM* cp_param )
{
	int index;
	int nation;
	int area;
	BOOL result;
	

	// 自分の地域を設定する
	result = WLDTIMER_EarthSetNationAreaRotate( p_wk, p_wk->my_nation, p_wk->my_area );
	
	if( result == FALSE ){
		
		// 表示するものになかったらデータベースの１番目の地域に設定
		index = WLDTIMER_WFLBYDATA_GetFirst( cp_param->cp_data );
		
		if( index != WFLBY_WLDTIMER_DATA_MAX ){		// 表示するかチェック
			
			nation = WFLBY_WLDTIMER_GetNation( cp_param->cp_data, index );
			area = WFLBY_WLDTIMER_GetArea( cp_param->cp_data, index );

			result = WLDTIMER_EarthSetNationAreaRotate( p_wk, nation, area );


			// ここは地域データがないと困る
			GF_ASSERT( result == TRUE );
		}else{

			// 最初のデータすらないとき
		}
		
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	地球儀のポイント位置がさしている地域データの取得
 *
 *	@param	cp_wk		ワーク
 *	@param	p_data		地域データ格納先
 *
 *	@retval	TRUE	地域があった
 *	@retval	FALSE	地域がなかった
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_EarthGetPointData( const WLDTIMER_WK* cp_wk, WLDTIMER_POINTDATA* p_data )
{
	u32 index;
	
	// 近い位置にある地域データを求める
	index = WLDTIMER_EarthGetPlaceData( cp_wk );

	// インデックスがあったかチェック
	if( index >= WLDTIMER_PlaceGetListNum( &cp_wk->place ) ){
		return FALSE;	// なかった
	}
	
	// その地域の情報を格納
	p_data->timezone	= WLDTIMER_EarthGetTimeZone( cp_wk, index, cp_wk->worldtime );
	p_data->nation		= WLDTIMER_PlaceGetIdxNationID( &cp_wk->place, index );
	p_data->area		= WLDTIMER_PlaceGetIdxAreaID( &cp_wk->place, index );

	return TRUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief	近くにある地域データを取得する
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	地域データのindex
 *	@retval	cp_wk->place.planelist.listcount	なかった
 */
//-----------------------------------------------------------------------------
static u32 WLDTIMER_EarthGetPlaceData( const WLDTIMER_WK* cp_wk )
{
	int i;
	s16		minx;
	s16		maxx;
	s16		miny;
	s16		maxy;
	u32 dist;		// 距離計算
	u32 mindist;	// 最小距離
	VecFx32 rotate;
	VecFx32 place_rotate;
	u32	place_col;
	u32 listnum;
	u32 minindex;
	BOOL xcheck;
	BOOL ycheck;

	// 地域リスト数取得
	listnum = WLDTIMER_PlaceGetListNum( &cp_wk->place );

	// 地球儀の回転ベクトル取得
	WLDTIMER_EarthGetRotaVec( &cp_wk->earth, &rotate );
	
	// 判定矩形を求める
	minx = (s16)(rotate.x - WLDTIMER_POINTPLANE_HITAREA);
	maxx = (s16)(rotate.x + WLDTIMER_POINTPLANE_HITAREA);
	miny = (s16)(rotate.y - WLDTIMER_POINTPLANE_HITAREA);
	maxy = (s16)(rotate.y + WLDTIMER_POINTPLANE_HITAREA);	

	// 最小距離をありえない距離で初期化
	mindist = WLDTIMER_POINTPLANE_HITAREA*2;	
	minindex = listnum;

	for( i=0; i<listnum; i++ ){
		
		// その地域に回転ベクトル取得
		WLDTIMER_PlaceGetIdxRotVec( &cp_wk->place, &place_rotate, i );
		// その地域の表示カラー取得
		place_col = WLDTIMER_PlaceGetIdxCol( &cp_wk->place, i );

		if( (place_rotate.x > minx)&&(place_rotate.x < maxx) &&
			(place_rotate.y > miny)&&(place_rotate.y < maxy) &&
			(place_col != WLDTIMER_PLACE_COL_NONE) ){

			// 距離を求める為に回転角度を調整
			WLDTIMER_MinusRotateChange( &rotate );
			WLDTIMER_MinusRotateChange( &place_rotate );

			// 回転角度の距離を取得
			dist = WLDTIMER_EarthGetRotateDist( &rotate, &place_rotate );

			if( dist < mindist ){
				mindist = dist;
				minindex = i;
			}
		}
	}

	// 最小の距離のインデックスを返す
	return minindex;



#if 0
	int i;
	s32		minx;
	s32		maxx;
	s32		miny;
	s32		maxy;
	u32 dist;		// 距離計算
	u32 mindist;	// 最小距離
	VecFx32 rotate;
	VecFx32 place_rotate;
	u32	place_col;
	u32 listnum;
	u32 minindex;
	BOOL xcheck;
	BOOL ycheck;

	// 地域リスト数取得
	listnum = WLDTIMER_PlaceGetListNum( &cp_wk->place );

	// 地球儀の回転ベクトル取得
	WLDTIMER_EarthGetRotaVec( &cp_wk->earth, &rotate );

	// 回転角度を調整
	WLDTIMER_MinusRotateChange( &rotate );
	
	// 判定矩形を求める
	minx = (s32)(rotate.x - WLDTIMER_POINTPLANE_HITAREA);
	maxx = (s32)(rotate.x + WLDTIMER_POINTPLANE_HITAREA);
	miny = (s32)(rotate.y - WLDTIMER_POINTPLANE_HITAREA);
	maxy = (s32)(rotate.y + WLDTIMER_POINTPLANE_HITAREA);	

	// 最小距離をありえない距離で初期化
	mindist = WLDTIMER_POINTPLANE_HITAREA*2;	
	minindex = listnum;

	for( i=0; i<listnum; i++ ){
		
		// その地域に回転ベクトル取得
		WLDTIMER_PlaceGetIdxRotVec( &cp_wk->place, &place_rotate, i );
		// 回転角度を調整
		WLDTIMER_MinusRotateChange( &place_rotate );
		// その地域の表示カラー取得
		place_col = WLDTIMER_PlaceGetIdxCol( &cp_wk->place, i );

		xcheck = WLDTIMER_RotateCheck( minx, maxx, place_rotate.x );
		ycheck = WLDTIMER_RotateCheck( miny, maxy, place_rotate.y );

		if( (xcheck == TRUE) &&
			(ycheck == TRUE) &&
			(place_col != WLDTIMER_PLACE_COL_NONE) ){

			// 回転角度の距離を取得
			dist = WLDTIMER_EarthGetRotateDist( &rotate, &place_rotate );

			if( dist < mindist ){
				mindist = dist;
				minindex = i;
			}
		}
	}

	// 最小の距離のインデックスを返す
	return minindex;
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	回転ベクトルの距離を求める
 *
 *	@param	cp_earth		地球儀の回転ベクトル
 *	@param	cp_place		地域の回転ベクトル
 *
 *	@return	距離（u32単位）
 */
//-----------------------------------------------------------------------------
static u32 WLDTIMER_EarthGetRotateDist( const VecFx32* cp_earth, const VecFx32* cp_place )
{
	s32 dif_x, dif_y;
	u32 dist;

	// 距離が近いほうを選ぶ
	dif_x = MATH_ABS(cp_earth->x - cp_place->x);
	dif_y = MATH_ABS(cp_earth->y - cp_place->y);
#if WB_FIX
	if( dif_x > RotKey(180) ){
#else
	if( dif_x > GFL_CALC_RotKey(180) ){
#endif
		dif_x = 0xffff - dif_x;
	}
#if WB_FIX
	if( dif_y > RotKey(180) ){
#else
	if( dif_y > GFL_CALC_RotKey(180) ){
#endif
		dif_y = 0xffff - dif_y;
	}

	dist = FX_Sqrt( ((dif_x*dif_x)+(dif_y*dif_y)) << FX32_SHIFT ) >> FX32_SHIFT;

	return dist;
}

//----------------------------------------------------------------------------
/**
 *	@brief	時間ゾーンの取得
 *
 *	@param	cp_wk		ワーク
 *	@param	placeindex	地域データのインデックス
 *	@param	worldtime	世界時間
 *
 *	@return	時間ゾーン
 */
//-----------------------------------------------------------------------------
static u32 WLDTIMER_EarthGetTimeZone( const WLDTIMER_WK* cp_wk, u32 placeindex, WFLBY_TIME worldtime )
{
#ifdef WLDTIMER_DEBUG_TIMEZONE
	return s_WLDTIMER_DEBUG_TimeZone;
#else
	s32 timezone;	// 時間ゾーン
	u16 rota;
	VecFx32 gmt_rotate;	// 基準回転ベクトル
	VecFx32 place_rotate;	// 地域の回転ベクトル
	int index;

	// 世界時間から基準タイムゾーンを求める
	timezone = worldtime.hour - WLDTIMER_TIMEZONE_START_HOUR;

	// 基準回転ベクトルの取得
	index = WLDTIMER_PlaceGetIdx( &cp_wk->place, 
			WLDTIMER_GMT_NATIONID, WLDTIMER_GMT_AREAID );
	GF_ASSERT( index < WLDTIMER_PlaceGetListNum( &cp_wk->place ) );
	WLDTIMER_PlaceGetIdxRotVec( &cp_wk->place, &gmt_rotate, index );
	
	// 地域の回転ベクトル取得
	WLDTIMER_PlaceGetIdxRotVec( &cp_wk->place, &place_rotate, placeindex );

	// Y軸回転角度の差から時間を求める
	rota = gmt_rotate.y - place_rotate.y;
	timezone += (rota*WLDTIMER_TIMEZONE_DATANUM)/0xffff;	// 回転値からタイムテーブルを求める

//	OS_Printf( "timezone %d  rota %d\n", timezone, rota );

	// マイナスの値だったり、オーバーしたりしているはずなので調整
	if( timezone < 0 ){
		timezone += WLDTIMER_TIMEZONE_DATANUM;
	}
	if( timezone >= WLDTIMER_TIMEZONE_DATANUM ){
		timezone %= WLDTIMER_TIMEZONE_DATANUM;
	}


	return timezone;
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	国と地域からその位置を中心に持っていく
 *
 *	@param	p_wk		ワーク
 *	@param	nationID	国ID
 *	@param	areaID		地域ID
 *
 *	@retval	TRUE	成功
 *	@retval	FALSE	失敗
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_EarthSetNationAreaRotate( WLDTIMER_WK* p_wk, u8 nationID, u8 areaID )
{
	VecFx32 vec;
	VecFx32 earth_vec;
	u32 col;

	// 地域の表示色と相対回転ベクトルを取得
	// 表示色は、その地域が表示するデータにあるかを表す
	col = WLDTIMER_PlaceGetData( &p_wk->place, nationID, areaID, &vec );
	
	if( col != WLDTIMER_PLACE_COL_NONE ){			// 表示するかチェック
		
		WLDTIMER_EarthGetRotaVec( &p_wk->earth, &earth_vec );
		vec.z = earth_vec.z;						// ｚは地球儀を引き継ぐ
		WLDTIMER_EarthSetRotaVec( &p_wk->earth, &vec );
		return TRUE;
	}
	return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief	フラグ設定
 *
 *	@param	p_flag		フラグワーク
 *	@param	cp_param	表示データ
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_FlagControl( WLDTIMER_WK* p_wk, const WLDTIMER_PARAM* cp_param )
{
	int i;

	// フラグを初期化
	p_wk->flag.japan = 0;
	p_wk->flag.world = JAPAN_MODE;

	// 日本ROMかチェック
#if (PM_LANG == LANG_JAPAN)
	p_wk->flag.japan = TRUE;
#else
	p_wk->flag.japan = FALSE;
#endif

	// 世界表示モードか調べる
	if( p_wk->flag.japan == TRUE ){
		for( i=0; i<WFLBY_WLDTIMER_DATA_MAX; i++ ){
			
			// データ有無
			if( cp_param->cp_data->data[i].flag == TRUE ){
				// 日本以外の国があったら世界表示モードにする
				if( cp_param->cp_data->data[i].nation != WLDTIMER_JPN_NATIONID ){
					p_wk->flag.world = GLOBAL_MODE;
				}
			}
		}
	}else{
		p_wk->flag.world = GLOBAL_MODE;
	}


#ifdef WLDTIMER_DEBUG_ALLPLACEOPEN
	if( s_WLDTIMER_DEBUG_ALLPLACEOPEN_FLAG == TRUE ){
		p_wk->flag.world = GLOBAL_MODE;
	}
#endif
}


//----------------------------------------------------------------------------
/**
 *	@brief	表示物の初期化
 *
 *	@param	p_wk		表示システムワーク
 *	@param	p_config	コンフィグデータ
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_DrawSysInit( WLDTIMER_DRAWSYS* p_wk, CONFIG* p_config, u32 heapID )
{
	// アーカイブハンドル
	p_wk->p_handle = GFL_ARC_OpenDataHandle( ARCID_WORLDTIMER, heapID );

#if WB_FIX
	// Vram転送マネージャ作成
	initVramTransferManagerHeap( WLDTIMER_VRAMTRANS_TASKNUM, heapID );
#endif
	
	// バンク設定
	GFL_DISP_SetBank( &sc_WLDTIMER_BANK );
	//VRAMクリア	2009.03.28(土) 追加 matsuda
	GFL_STD_MemClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
	GFL_STD_MemClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
	GFL_STD_MemClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
	GFL_STD_MemClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);

	// BG設定
	WLDTIMER_DrawSysBgInit( p_wk, p_config, heapID );

	// OAM設定
	WLDTIMER_DrawSysOamInit( p_wk, heapID );

	// 3D設定
	WLDTIMER_DrawSys3DInit( p_wk, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	表示物の破棄
 *
 *	@param	p_wk		表示システムワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_DrawSysExit( WLDTIMER_DRAWSYS* p_wk )
{
	// アーカイブハンドル
	GFL_ARC_CloseDataHandle( p_wk->p_handle );

#if WB_TEMP_FIX
	// Vram転送マネージャ破棄
	DellVramTransferManager();
#endif
	
	// BG設定
	WLDTIMER_DrawSysBgExit( p_wk );

	// OAM設定
	WLDTIMER_DrawSysOamExit( p_wk );

	// 3D設定
	WLDTIMER_DrawSys3DExit( p_wk );

}

//----------------------------------------------------------------------------
/**
 *	@brief	描画処理
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_DrawSysDraw( WLDTIMER_DRAWSYS* p_wk )
{
#if WB_FIX
	CLACT_Draw( p_wk->p_clactset );
#else
  GFL_CLACT_SYS_Main();
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	VBlank処理
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_DrawSysVBlank( WLDTIMER_DRAWSYS* p_wk )
{
    // BG書き換え
    GFL_BG_VBlankFunc();

#if WB_FIX
    // レンダラ共有OAMマネージャVram転送
    REND_OAMTrans();
#else
  	GFL_CLACT_SYS_VBlankFunc();
#endif

#if WB_TEMP_FIX
	// Vram転送
	DoVramTransferManager();
#endif
}

// BG
static void WLDTIMER_DrawSysBgInit( WLDTIMER_DRAWSYS* p_wk, CONFIG* p_config, u32 heapID )
{
	// BG設定
	GFL_BG_SetBGMode(&sc_BGINIT);
	
	// BGL作成
	GFL_BG_Init( heapID );
	GFL_BMPWIN_Init(heapID);

	// メインとサブを切り替える
	GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_SUB);
	GFL_DISP_SetDispOn();


	// BGコントロール設定
	{
		int i;

		for( i=0; i<WLDTIMER_BGCNT_NUM; i++ ){
			GFL_BG_SetBGControl( 
					sc_WLDTIMER_BGCNT_FRM[i], &sc_WLDTIMER_BGCNT_DATA[i],
					GFL_BG_MODE_TEXT );
			GFL_BG_SetVisible(sc_WLDTIMER_BGCNT_FRM[i], VISIBLE_ON);
			GFL_BG_SetClearCharacter( sc_WLDTIMER_BGCNT_FRM[i], 32, 0, heapID);
			GFL_BG_ClearScreen( sc_WLDTIMER_BGCNT_FRM[i] );
		}
	}

	// 基本キャラクタパレットフレーム
	// サブ画面
	GFL_ARCHDL_UTIL_TransVramPalette( p_wk->p_handle, NARC_worldtimer_world_watch_NCLR,
			PALTYPE_SUB_BG, 0, 0, heapID );
	
	// フレーム
	{
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_wk->p_handle, 
				NARC_worldtimer_world_watch_frame_NCGR, 
				GFL_BG_FRAME0_S, 0, 0, FALSE, heapID );

		GFL_ARCHDL_UTIL_TransVramScreen( p_wk->p_handle,
				NARC_worldtimer_world_watch_frame_NSCR, 
				GFL_BG_FRAME0_S, 0, 0, FALSE, heapID );
	}

	// 背景
	{
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_wk->p_handle,
				NARC_worldtimer_world_watch_NCGR, 
				GFL_BG_FRAME2_S, 0, 0, FALSE, heapID );

		GFL_ARCHDL_UTIL_TransVramScreen( p_wk->p_handle,
				NARC_worldtimer_world_watch_wall_NSCR, 
				GFL_BG_FRAME3_S, 0, 0, FALSE, heapID );
	}
	
	// フォントカラー
    TalkFontPaletteLoad( PALTYPE_MAIN_BG, WLDTIMER_PALMAIN_FONT*0x20, heapID );
    TalkFontPaletteLoad( PALTYPE_SUB_BG, WLDTIMER_PALSUB_FONT*0x20, heapID );

    BmpWinFrame_GraphicSet(
        GFL_BG_FRAME1_M, 
		WLDTIMER_MAIN_SYSTEMWIN_CGX, WLDTIMER_PALMAIN_SYSTEM, 0, heapID );

	// システムウィンドウ
    BmpWinFrame_GraphicSet(
        GFL_BG_FRAME1_M, 
		WLDTIMER_MAIN_SYSTEMWIN_CGX, WLDTIMER_PALMAIN_SYSTEM, 0, heapID );

	// トークウィンドウ
	{
		u8 win_num = CONFIG_GetWindowType( p_config );
		TalkWinFrame_GraphicSet( GFL_BG_FRAME0_S,
				WLDTIMER_SUB_TALKWIN_CGX, WLDTIMER_SUB_TALKWIN_PAL,
				win_num, heapID );

		TalkWinFrame_GraphicSet( GFL_BG_FRAME1_M,
				WLDTIMER_MAIN_TALKWIN_CGX, WLDTIMER_MAIN_TALKWIN_PAL,
				win_num, heapID );
	}

	// バックグラウンドカラー設定
	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x72ca );
}
static void WLDTIMER_DrawSysBgExit( WLDTIMER_DRAWSYS* p_wk )
{
	// ＢＧコントロール破棄
	{
		int i;

		for( i=0; i<WLDTIMER_BGCNT_NUM; i++ ){
			GFL_BG_FreeBGControl( sc_WLDTIMER_BGCNT_FRM[i] );
		}
	}
	
	// BGL破棄
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();

	// メインとサブを元に戻す
	GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
	GFL_DISP_SetDispOn();
}

// OAM
static void WLDTIMER_DrawSysOamInit( WLDTIMER_DRAWSYS* p_wk, u32 heapID )
{
    int i;

#if WB_FIX
    // OAMマネージャーの初期化
    NNS_G2dInitOamManagerModule();

    // 共有OAMマネージャ作成
    // レンダラ用OAMマネージャ作成
    // ここで作成したOAMマネージャをみんなで共有する
    REND_OAMInit(
        0, 126,     // メイン画面OAM管理領域
        0, 31,      // メイン画面アフィン管理領域
        0, 126,     // サブ画面OAM管理領域
        0, 31,      // サブ画面アフィン管理領域
        heapID);

    // キャラクタマネージャー初期化
    InitCharManagerReg(&sc_WLDTIMER_CHARMAN_INIT, GX_OBJVRAMMODE_CHAR_1D_32K, GX_OBJVRAMMODE_CHAR_1D_32K );
    // パレットマネージャー初期化
    InitPlttManager(WLDTIMER_OAM_CONTNUM, heapID);

    // 読み込み開始位置を初期化
    CharLoadStartAll();
    PlttLoadStartAll();

    //通信アイコン用にキャラ＆パレット制限
    CLACT_U_WmIcon_SetReserveAreaCharManager(NNS_G2D_VRAM_TYPE_2DMAIN, GX_OBJVRAMMODE_CHAR_1D_32K);
    CLACT_U_WmIcon_SetReserveAreaPlttManager(NNS_G2D_VRAM_TYPE_2DMAIN);
#endif
    

#if WB_FIX
    // セルアクターセット作成
    p_wk->p_clactset = CLACT_U_SetEasyInit( WLDTIMER_OAM_CONTNUM, &p_wk->renddata, heapID );

    // サーフェース位置を移動させる
    CLACT_U_SetSubSurfaceMatrix( &p_wk->renddata, 0, WLDTIMER_SF_MAT_Y );
    
    // キャラとパレットのリソースマネージャ作成
    for( i=0; i<WLDTIMER_RESMAN_NUM; i++ ){
        p_wk->p_resman[i] = CLACT_U_ResManagerInit(WLDTIMER_OAM_CONTNUM, i, heapID);
    }
#else
		{
			GFL_CLSYS_INIT clsys_init = GFL_CLSYSINIT_DEF_DIVSCREEN;
			
			clsys_init.oamst_main = GFL_CLSYS_OAMMAN_INTERVAL;	//通信アイコンの分
			clsys_init.oamnum_main = 128-GFL_CLSYS_OAMMAN_INTERVAL;
			clsys_init.tr_cell = 32;	//セルVram転送管理数

			GFL_CLACT_SYS_Create( &clsys_init, &sc_WLDTIMER_BANK, heapID );
			p_wk->p_clactset = GFL_CLACT_UNIT_Create( WLDTIMER_OAM_CONTNUM, 0, heapID );
			GFL_CLACT_UNIT_SetDefaultRend(p_wk->p_clactset);
			p_wk->plttslot = PLTTSLOT_Init(heapID, 16, 16);
		}
#endif

	// 下画面に通信アイコンを出す
#if WB_TEMP_FIX
	WirelessIconEasy();  // 接続中なのでアイコン表示
#endif


	// 表示開始
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
}
static void WLDTIMER_DrawSysOamExit( WLDTIMER_DRAWSYS* p_wk )
{
#if WB_FIX
    int i;

    // アクターの破棄
    CLACT_DestSet( p_wk->p_clactset );

    for( i=0; i<WLDTIMER_RESMAN_NUM; i++ ){
        CLACT_U_ResManagerDelete( p_wk->p_resman[i] );
    }

    // リソース解放
    DeleteCharManager();
    DeletePlttManager();

    //OAMレンダラー破棄
    REND_OAM_Delete();
#else
		GFL_CLACT_UNIT_Delete(p_wk->p_clactset);
		GFL_CLACT_SYS_Delete();
		PLTTSLOT_Exit(p_wk->plttslot);
#endif
}

// 3D
static void WLDTIMER_DrawSys3DInit( WLDTIMER_DRAWSYS* p_wk, u32 heapID )
{
	// ３D設定
	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT64K,
						0x1000, heapID, WLDTIMER_DrawSys3DSetUp);
	GFL_G3D_SetSystemSwapBufferMode(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

	// ライト初期化
	NNS_G3dGlbLightVector(USE_LIGHT_NUM,
			INIT_LIGHT_ANGLE_XVAL,INIT_LIGHT_ANGLE_YVAL,INIT_LIGHT_ANGLE_ZVAL);
}
static void WLDTIMER_DrawSys3DExit( WLDTIMER_DRAWSYS* p_wk )
{
    GFL_G3D_Exit();
}
static void WLDTIMER_DrawSys3DSetUp( void )
{
	// ３Ｄ使用面の設定(表示＆プライオリティー)
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
    G2_SetBG0Priority(1);

	// 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
    G3X_SetShading( GX_SHADING_TOON );
    G3X_AntiAlias( TRUE );
	G3X_AlphaTest( FALSE, 0 );	// アルファテスト　　オフ
	G3X_AlphaBlend( TRUE );		// アルファブレンド　オン
	G3X_EdgeMarking( TRUE );
	G3X_SetFog( FALSE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x8000, 0 );

	// クリアカラーの設定
    G3X_SetClearColor(GX_RGB(26,26,26),0,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog
	// ビューポートの設定
    G3_ViewPort(0, 0, 255, 191);

	// バックカラー設定
}


//----------------------------------------------------------------------------
/**
 *	@brief	地球儀初期化
 *
 *	@param	p_wk		ワーク
 *	@param	p_drawsys	描画システム
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EarthInit( WLDTIMER_EARTH* p_wk, WLDTIMER_DRAWSYS* p_drawsys, u32 heapID )
{
	// 表示データ初期化
	{
		VEC_Set( &p_wk->trans,			// 座標
				INIT_EARTH_TRANS_XVAL,
				INIT_EARTH_TRANS_YVAL,
				INIT_EARTH_TRANS_ZVAL );

		VEC_Set( &p_wk->scale,			// 拡大
				INIT_EARTH_SCALE_XVAL,
				INIT_EARTH_SCALE_YVAL,
				INIT_EARTH_SCALE_ZVAL );

		VEC_Set( &p_wk->rotate,			// 回転
				INIT_EARTH_ROTATE_XVAL,
				INIT_EARTH_ROTATE_YVAL,
				INIT_EARTH_ROTATE_ZVAL );
	}

	// 表示リソース読み込み
#if WB_FIX
	{
		D3DOBJ_MdlLoadH( &p_wk->mdl, p_drawsys->p_handle, 
				NARC_worldtimer_wifi_earth_nsbmd, heapID );
		D3DOBJ_Init( &p_wk->obj, &p_wk->mdl );
	}
#else
  {
    p_wk->p_mdlres = GFL_G3D_CreateResourceHandle( p_drawsys->p_handle, NARC_worldtimer_wifi_earth_nsbmd );
    GFL_G3D_TransVramTexture(p_wk->p_mdlres);
		p_wk->rnder = GFL_G3D_RENDER_Create( p_wk->p_mdlres, 0, p_wk->p_mdlres );
    p_wk->g3dobj = GFL_G3D_OBJECT_Create(p_wk->rnder, NULL, 0);
    
  	VEC_Set(&p_wk->obj.trans, 0, 0, 0);
  	VEC_Set(&p_wk->obj.scale, FX32_ONE, FX32_ONE, FX32_ONE);
  	MTX_Identity33(&p_wk->obj.rotate);
  	VEC_Set(&p_wk->obj_rotate, 0, 0, 0);
  }
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	地球儀破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EarthExit( WLDTIMER_EARTH* p_wk )
{
	// リソースの破棄
	{
	#if WB_FIX
		D3DOBJ_MdlDelete( &p_wk->mdl );
	#else
	  GFL_G3D_OBJECT_Delete(p_wk->g3dobj);
	  GFL_G3D_DeleteResource(p_wk->p_mdlres);
	  GFL_G3D_RENDER_Delete(p_wk->rnder);
	#endif
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	地球儀描画
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EarthDraw( WLDTIMER_EARTH* p_wk )
{
	MtxFx33 rotmtx;


	// 回転行列取得
	WLDTIMER_EarthGetRotMtx( p_wk, &rotmtx );
	
	// 座標、拡大を設定
#if WB_FIX
	D3DOBJ_SetMatrix( &p_wk->obj, 
			p_wk->trans.x, p_wk->trans.y, p_wk->trans.z );
	D3DOBJ_SetScale( &p_wk->obj, 
			p_wk->scale.x, p_wk->scale.y, p_wk->scale.z );
#else
  VEC_Set(&p_wk->obj.trans, p_wk->trans.x, p_wk->trans.y, p_wk->trans.z );
  VEC_Set(&p_wk->obj.scale, p_wk->scale.x, p_wk->scale.y, p_wk->scale.z );
#endif


	// 表示
#if WB_FIX
	D3DOBJ_DrawRMtx( &p_wk->obj,
					 &rotmtx );
#else
  p_wk->obj.rotate = rotmtx;
	GFL_G3D_DRAW_DrawObject(p_wk->g3dobj, &p_wk->obj);
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	地球儀表示座標を取得
 *
 *	@param	cp_wk		ワーク
 *	@param	p_mat		座標格納先
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EarthGetTrans( const WLDTIMER_EARTH* cp_wk, VecFx32* p_mat )
{
	*p_mat = cp_wk->trans;
}

//----------------------------------------------------------------------------
/**
 *	@brief	地球儀回転行列を取得
 *
 *	@param	cp_wk	ワーク
 *	@param	p_rot	行列格納先
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EarthGetRotMtx( const WLDTIMER_EARTH* cp_wk, MtxFx33* p_rot )
{
	WLDTIMER_EarthVecFx32_to_MtxFx33( p_rot, &cp_wk->rotate );
}

//----------------------------------------------------------------------------
/**
 *	@brief	地球儀	回転ベクトル取得
 *
 *	@param	cp_wk	ワーク
 *	@param	p_vec	回転ベクトル
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EarthGetRotaVec( const WLDTIMER_EARTH* cp_wk, VecFx32* p_vec )
{
	*p_vec = cp_wk->rotate;
}

//----------------------------------------------------------------------------
/**
 *	@brief	地球儀	回転ベクトル設定
 *
 *	@param	p_wk		ワーク
 *	@param	cp_vec		回転ベクトル
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EarthSetRotaVec( WLDTIMER_EARTH* p_wk, const VecFx32* cp_vec )
{
	p_wk->rotate = *cp_vec;
}


//----------------------------------------------------------------------------
/**
 *	@brief	地域データ初期化
 *
 *	@param	p_wk		ワーク
 *	@param	cp_data		表示地域データ
 *	@param	flag		フラグデータ
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PlaceInit( WLDTIMER_PLACE* p_wk, const WFLBY_WLDTIMER* cp_data, WLDTIMER_DRAWSYS* p_drawsys, WLDTIMER_FLAG flag, u32 heapID )
{
	int i;
	
	// 地域リスト読み込み
	WLDTIMER_EarthListLoad( p_wk, cp_data, heapID );

	// 描画リソース読み込み
	for( i=0; i<WLDTIMER_PLACE_COL_NUM; i++ ){
	#if WB_FIX
		D3DOBJ_MdlLoadH( &p_wk->mdl[i], p_drawsys->p_handle,// 読み込み
				sc_WLDTIMER_PLACE_MDL[i], heapID );
		D3DOBJ_Init( &p_wk->obj[i], &p_wk->mdl[i] );		// 表示オブジェ初期化
	#else
    p_wk->p_mdlres[i] = GFL_G3D_CreateResourceHandle( 
      p_drawsys->p_handle, sc_WLDTIMER_PLACE_MDL[i] );
    GFL_G3D_TransVramTexture(p_wk->p_mdlres[i]);
		p_wk->rnder[i] = GFL_G3D_RENDER_Create( p_wk->p_mdlres[i], 0, p_wk->p_mdlres[i] );
    p_wk->g3dobj[i] = GFL_G3D_OBJECT_Create(p_wk->rnder[i], NULL, 0);

  	VEC_Set(&p_wk->obj[i].trans, 0, 0, 0);
  	VEC_Set(&p_wk->obj[i].scale, FX32_ONE, FX32_ONE, FX32_ONE);
  	MTX_Identity33(&p_wk->obj[i].rotate);
  	VEC_Set(&p_wk->obj_rotate[i], 0, 0, 0);
	#endif
	}

	// マークスケール初期化
	if( flag.world == JAPAN_MODE ){
		VEC_Set( &p_wk->list_scale,
				 INIT_EARTH_SCALE_NEAR_XVAL,
				 INIT_EARTH_SCALE_NEAR_YVAL,
				 INIT_EARTH_SCALE_NEAR_ZVAL );
	}else{
		VEC_Set( &p_wk->list_scale,
				 INIT_EARTH_SCALE_XVAL,
				 INIT_EARTH_SCALE_YVAL,
				 INIT_EARTH_SCALE_ZVAL );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	地域データ破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PlaceExit( WLDTIMER_PLACE* p_wk )
{
	int i;
	
	// 描画リソース破棄
	for( i=0; i<WLDTIMER_PLACE_COL_NUM; i++ ){
	#if WB_FIX
		D3DOBJ_MdlDelete( &p_wk->mdl[i] );
	#else
	  GFL_G3D_OBJECT_Delete(p_wk->g3dobj[i]);
	  GFL_G3D_DeleteResource(p_wk->p_mdlres[i]);
	  GFL_G3D_RENDER_Delete(p_wk->rnder[i]);
	#endif
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	地域データの表示
 *	
 *	@param	p_wk		ワーク
 *	@param	cp_earth	地球ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PlaceDraw( WLDTIMER_PLACE* p_wk, const WLDTIMER_EARTH* cp_earth )
{
	MtxFx33 rotate_tmp;
	int i;
	MtxFx33 earthrotmtx;
	VecFx32 trans;

	// 地球儀の表示データ取得
	WLDTIMER_EarthGetTrans( cp_earth, &trans );
	WLDTIMER_EarthGetRotMtx( cp_earth, &earthrotmtx );

	// 表示モデルに表示データを格納
	for( i=0; i<WLDTIMER_PLACE_COL_NUM; i++ ){
	#if WB_FIX
		D3DOBJ_SetMatrix( &p_wk->obj[i], trans.x, trans.y, trans.z );
		// タッチマークは絶対に全部のマークの上に表示される
		if( i==WLDTIMER_PLACE_COL_TOUCH ){
			D3DOBJ_SetScale( &p_wk->obj[i], 
					p_wk->list_scale.x, 
					p_wk->list_scale.y,
					p_wk->list_scale.z+WLDTIMER_PLACE_TOUCH_SCALE_Z_ADD );
		}else{
			D3DOBJ_SetScale( &p_wk->obj[i], 
					p_wk->list_scale.x, p_wk->list_scale.y, p_wk->list_scale.z );
		}
	#else
	  VEC_Set(&p_wk->obj[i].trans, trans.x, trans.y, trans.z );
		// タッチマークは絶対に全部のマークの上に表示される
		if( i==WLDTIMER_PLACE_COL_TOUCH ){
			VEC_Set( &p_wk->obj[i].scale, 
					p_wk->list_scale.x, 
					p_wk->list_scale.y,
					p_wk->list_scale.z+WLDTIMER_PLACE_TOUCH_SCALE_Z_ADD );
		}else{
			VEC_Set( &p_wk->obj[i].scale, 
					p_wk->list_scale.x, p_wk->list_scale.y, p_wk->list_scale.z );
		}
	#endif
	}

	// 触っている位置の表示
	{
		MTX_Identity33( &rotate_tmp );
	#if WB_FIX
		D3DOBJ_DrawRMtx( &p_wk->obj[ WLDTIMER_PLACE_COL_TOUCH ],
						 &rotate_tmp );
	#else
    p_wk->obj[WLDTIMER_PLACE_COL_TOUCH].rotate = rotate_tmp;
  	GFL_G3D_DRAW_DrawObject(p_wk->g3dobj[WLDTIMER_PLACE_COL_TOUCH], &p_wk->obj[WLDTIMER_PLACE_COL_TOUCH]);
	#endif
	}
	

	// 各地点の表示
	{
		for(i=0;i<p_wk->placelist.listcount;i++){
			MTX_Concat33(&p_wk->placelist.place[i].rotate,&earthrotmtx,&rotate_tmp);

			if(p_wk->placelist.place[i].col != WLDTIMER_PLACE_COL_NONE){
			#if WB_FIX
				D3DOBJ_DrawRMtx( &p_wk->obj[ p_wk->placelist.place[i].col ],
								 &rotate_tmp );
			#else
        p_wk->obj[i].rotate = rotate_tmp;
      	GFL_G3D_DRAW_DrawObject(p_wk->g3dobj[i], &p_wk->obj[i]);
			#endif
			}
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	国IDと地域IDからその地域を何色で表示したらよいかを返す
 *
 *	@param	cp_data			表示地域データ
 *	@param	nationID		国ID
 *	@param	areaID			エリアID
 *
 *	@retval WLDTIMER_PLACE_COL_IN,		// 入室カラー
 *	@retval WLDTIMER_PLACE_COL_OUT,		// 退室カラー
 *	@retval	WLDTIMER_PLACE_COL_NONE,	// 部屋にいない地域に設定されているカラー
 */
//-----------------------------------------------------------------------------
static u32 WLDTIMER_PlaceGetCol( const WFLBY_WLDTIMER* cp_data, u16 nationID, u16 areaID )
{
	int i;
	BOOL outside;
	BOOL dataflag;
	u8 nation, area;

	for( i=0; i<WFLBY_WLDTIMER_DATA_MAX; i++ ){
		dataflag = WFLBY_WLDTIMER_GetDataFlag( cp_data, i );	// データ有無
		if( dataflag ){
			outside = WFLBY_WLDTIMER_GetOutSide( cp_data, i );	// 退室フラグ
			nation	= WFLBY_WLDTIMER_GetNation( cp_data, i );	// 国ID
			area	= WFLBY_WLDTIMER_GetArea( cp_data, i );		// 地域ID

			// 国と地域が合うかチェック
			if( (nation == nationID) && (area == areaID) ){
				
				// 退室したかチェック
				if( outside == TRUE ){
					return	WLDTIMER_PLACE_COL_OUT; 
				}
				return	WLDTIMER_PLACE_COL_IN; 
			}
		}
	}

	return WLDTIMER_PLACE_COL_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	地域データの取得
 *
 *	@param	cp_wk		ワーク
 *	@param	nationID	国ID
 *	@param	areaID		エリアID
 *	@param	p_vec		相対回転ベクトル取得
 *
 *	@return	表示カラー取得
 */
//-----------------------------------------------------------------------------
static u32 WLDTIMER_PlaceGetData( const WLDTIMER_PLACE* cp_wk, u16 nationID, u16 areaID, VecFx32* p_vec )
{
	u32 col = WLDTIMER_PLACE_COL_NONE;
	int index;
	int listnum;

	listnum = WLDTIMER_PlaceGetListNum( cp_wk );

	// インデックスの取得
	index = WLDTIMER_PlaceGetIdx( cp_wk, nationID, areaID );
	if( index < listnum ){

		// カラー取得
		col = WLDTIMER_PlaceGetIdxCol( cp_wk, index );

		// 回転ベクトル取得
		WLDTIMER_PlaceGetIdxRotVec( cp_wk, p_vec, index );
	}

	return col;
}

//----------------------------------------------------------------------------
/**
 *	@brief	国IDと地域IDから地域データベースのインデックス値を取得
 *
 *	@param	cp_wk			地域データベース
 *	@param	nationID		国ID
 *	@param	areaID			地域ID
 *
 *	@return	index			インデックス
 *	@retval	cp_wk->placelist.listcount	なら当てはまるのもがない
 */
//-----------------------------------------------------------------------------
static u32 WLDTIMER_PlaceGetIdx( const WLDTIMER_PLACE* cp_wk, u16 nationID, u16 areaID )
{
	int i;
	
	for( i=0; i<cp_wk->placelist.listcount; i++ ){

		if( (cp_wk->placelist.place[i].nationID == nationID) &&
			(cp_wk->placelist.place[i].areaID == areaID) ){
			return i;
		}
	}

	return cp_wk->placelist.listcount;
}

//----------------------------------------------------------------------------
/**
 *	@brief	インデックスの回転ベクトルを取得
 *
 *	@param	cp_wk		ワーク
 *	@param	p_vec		回転ベクトル格納先
 *	@param	index		インデックス
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PlaceGetIdxRotVec( const WLDTIMER_PLACE* cp_wk, VecFx32* p_vec, int index )
{
	GF_ASSERT( index < cp_wk->placelist.listcount );
	
	p_vec->x = cp_wk->placelist.place[ index ].x;
	p_vec->y = cp_wk->placelist.place[ index ].y;
	p_vec->z = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	インデックスのカラーを取得
 *
 *	@param	cp_wk		ワーク
 *	@param	index		インデックス
 *
 *	@return	カラー
 */
//-----------------------------------------------------------------------------
static u32 WLDTIMER_PlaceGetIdxCol( const WLDTIMER_PLACE* cp_wk, int index )
{
	GF_ASSERT( index < cp_wk->placelist.listcount );
	
	return cp_wk->placelist.place[ index ].col;
}

//----------------------------------------------------------------------------
/**
 *	@brief	インデックスの国IDを取得
 *
 *	@param	cp_wk		ワーク
 *	@param	index		インデックス
 *
 *	@return	国ID
 */
//-----------------------------------------------------------------------------
static u32 WLDTIMER_PlaceGetIdxNationID( const WLDTIMER_PLACE* cp_wk, int index )
{
	GF_ASSERT( index < cp_wk->placelist.listcount );
	
	return cp_wk->placelist.place[ index ].nationID;
}

//----------------------------------------------------------------------------
/**
 *	@brief	インデックスの地域IDを取得
 *
 *	@param	cp_wk		ワーク
 *	@param	index		インデックス
 *
 *	@return	地域ID
 */
//-----------------------------------------------------------------------------
static u32 WLDTIMER_PlaceGetIdxAreaID( const WLDTIMER_PLACE* cp_wk, int index )
{
	GF_ASSERT( index < cp_wk->placelist.listcount );
	
	return cp_wk->placelist.place[ index ].areaID;
}

//----------------------------------------------------------------------------
/**
 *	@brief	インデックスの国と地域のリスト数取得
 *
 *	@param	cp_wk	ワーク
 *		
 *	@return	リスト数
 */
//-----------------------------------------------------------------------------
static u32 WLDTIMER_PlaceGetListNum( const WLDTIMER_PLACE* cp_wk )
{
	return cp_wk->placelist.listcount;
}


//----------------------------------------------------------------------------
/**
 *	@brief	カメラ初期化
 *
 *	@param	p_wk	ワーク
 *	@param	flag	動作フラグ
 *	@param	heapID	ヒープID
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_CameraInit( WLDTIMER_CAMERA* p_wk, WLDTIMER_FLAG flag, u32 heapID )
{
#if WB_FIX
	// カメラ作成
	p_wk->p_camera = GFC_AllocCamera( heapID );
	
	//カメラライブラリ初期化
	GFC_InitCameraTC(	&sc_WLDTIMER_CAMERA_TARGET,&sc_WLDTIMER_CAMERA_POS,
						INIT_CAMERA_PERSPWAY,
						GF_CAMERA_PERSPECTIV,
						FALSE,
						p_wk->p_camera);

	//クリップ関連設定
	GFC_SetCameraClip(INIT_CAMERA_CLIP_NEAR,INIT_CAMERA_CLIP_FAR,p_wk->p_camera);
	GFC_SetCameraView(GF_CAMERA_PERSPECTIV,p_wk->p_camera);
	//カメラＯＮ
	GFC_AttachCamera(p_wk->p_camera);
#else
	VecFx32 camUp = {0, FX32_ONE, 0};

	// カメラ作成
	p_wk->p_camera = GFL_G3D_CAMERA_Create(	GFL_G3D_PRJPERS, 
		FX_SinIdx( INIT_CAMERA_PERSPWAY ), FX_CosIdx( INIT_CAMERA_PERSPWAY ),
		defaultCameraAspect, 0, INIT_CAMERA_CLIP_NEAR, INIT_CAMERA_CLIP_FAR, 0,
		&sc_WLDTIMER_CAMERA_POS, &camUp, &sc_WLDTIMER_CAMERA_TARGET, heapID );
	
	//カメラＯＮ
	GFL_G3D_CAMERA_Switching(p_wk->p_camera);
#endif

	if(flag.world == JAPAN_MODE){
		//世界modeじゃないので、近くから
		p_wk->status = CAMERA_NEAR;
		p_wk->dist = INIT_CAMERA_DISTANCE_NEAR;
	}else{
		//世界modeなので遠くから
		p_wk->status = CAMERA_FAR;
		p_wk->dist = INIT_CAMERA_DISTANCE_FAR;
	}

	// 初期カメラ設定
#if WB_FIX
	GFC_SetCameraDistance(p_wk->dist,p_wk->p_camera);
#else
  _SetCameraDistance(p_wk->p_camera, p_wk->dist);
  GFL_G3D_CAMERA_Switching(p_wk->p_camera);
#endif
}

//------------------------------------------------------------
/**
 *	方向ベクトルから回転角を計算（ＺＸ平面対応）
 *	@param	*inVec1		基準方向ベクトル
 *	@param	*inVec2		指定方向ベクトル
 *
 *	@retval	rad			回転角
*/
//------------------------------------------------------------
static fx32 _GetRad(const VecFx32 *inVec1, const VecFx32 *inVec2 )
{
	VecFx32 vec1,vec2;
	fx32 sin,cos;
	fx32 rad;
	VEC_Normalize(inVec1, &vec1);
	VEC_Normalize(inVec2, &vec2);
	//内積を使ってコサインを求める
	//cos = VEC_DotProduct(&vec1, &vec2);
	cos = FX_Mul(vec1.z, vec2.z) + FX_Mul(vec1.x, vec2.x); 
	//外積を使ってサインを求める
	sin = FX_Mul(vec1.z, vec2.x) - FX_Mul(vec1.x, vec2.z);
	//XZ平面上での計算なので時計回りが正方向となる（反時計回りにするため、サインの向きを逆にする）
	///sin = FX_Mul(sin, -FX32_ONE);
	if (cos == 0){//アークコサインが使えない場合
		if (sin > 0){
			rad = 0x4000;
		}else{
			rad = 0xc000;
		}
	}else{
		//アークタンジェントを使って角度を求める
		rad = FX_Atan2Idx(sin, cos);
	}
	return rad;
}

//---------------------------------------------------------------------------
/**
 * @brief	カメラ位置を注視点、距離、アングルから算出する
 * 
 * @param	camera_ptr	カメラポインタ
 * 
 * @return	none
 */
//---------------------------------------------------------------------------
static void _SetCameraDistance(GFL_G3D_CAMERA *camera, fx32 distance)
{
  VecFx32 dst, campos, target;
  u16 angle_x, angle_y, angle_z;
  
  GFL_G3D_CAMERA_GetPos( camera, &campos );
  GFL_G3D_CAMERA_GetTarget( camera, &target );
  
	//注視点からのカメラ位置の距離を設定
	VEC_Subtract(&campos, &target , &dst);

  //angle取得
	{
		VecFx32 x_vec = {0,0,0};
		VecFx32 y_vec = {0,0,0};
		VecFx32 vec;
		VecFx32 base_vec = {0, 0, FX32_ONE};

		vec = dst;
		vec.y = 0;
		angle_y = _GetRad(&base_vec, &vec );
		
		base_vec.x = FX32_ONE;
		base_vec.y = 0;
		base_vec.z = 0;
		vec.x = dst.z;
		vec.z = dst.y;
		vec.y = 0;
		angle_x = _GetRad(&base_vec, &vec );

		angle_z = 0;

		OS_Printf("ANGLE_%x,%x,%x\n",angle_x, angle_y, angle_z);
		OS_TPrintf("distance = %x\n", distance);
	}
	
	{
    u16 calc_x;
    
  	//仰角⇒地面からの傾きに変換
  	calc_x = -angle_x;
  	
  	/*== カメラ座標を求める ==*/
  	campos.x 
  	  = FX_Mul( FX_Mul( FX_SinIdx( angle_y ), distance ), FX_CosIdx( angle_x ) );
  	
  	campos.z = FX_Mul( FX_Mul(FX_CosIdx(angle_y), distance),
  		   FX_CosIdx(angle_x) );
  	
  	campos.y = FX_Mul( FX_SinIdx( calc_x ), distance );
  
    OS_TPrintf("campos.x = %x, y = %x, z = %x\n", campos.x, campos.y, campos.z);
  	/*== 視点からの距離にする ==*/
  	VEC_Add(&campos, &target, &campos);
  	GFL_G3D_CAMERA_SetPos(camera, &campos);
    OS_TPrintf("target.x = %x, y = %x, z = %x\n", target.x, target.y, target.z);
    OS_TPrintf("set campos.x = %x, y = %x, z = %x\n", campos.x, campos.y, campos.z);
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_CameraExit( WLDTIMER_CAMERA* p_wk )
{
	GFL_G3D_CAMERA_Delete( p_wk->p_camera );
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ表示
 *
 *	@param	cp_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_CameraDraw( const WLDTIMER_CAMERA* cp_wk )
{
	GFL_G3D_DRAW_SetLookAt();
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ動作	リクエスト
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_CameraMoveReq( WLDTIMER_CAMERA* p_wk )
{
	if( p_wk->status == CAMERA_FAR ){
		p_wk->status = CAMERA_NEAR;
		PMSND_PlaySE( WLDTIMER_SND_ZOMEIN );
	}else{
		p_wk->status = CAMERA_FAR;
		PMSND_PlaySE( WLDTIMER_SND_ZOMEOUT );
	}
	p_wk->move = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ動作	
 *
 *	@param	p_wk	ワーク
 *	@param	p_place	地域ワーク
 *
 *	@retval	TRUE	完了
 *	@retval	FALSE	途中
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_CameraMove( WLDTIMER_CAMERA* p_wk, WLDTIMER_PLACE* p_place )
{
	if( p_wk->move == FALSE ){
		return TRUE;
	}

	switch( p_wk->status ){
	case CAMERA_NEAR:
		if(p_wk->dist > (INIT_CAMERA_DISTANCE_NEAR + CAMERA_INOUT_SPEED)){
			p_wk->dist -= CAMERA_INOUT_SPEED;
			p_place->list_scale.x -= MARK_SCALE_INCDEC;
			p_place->list_scale.y = p_place->list_scale.x;
		}else{
			p_wk->dist = INIT_CAMERA_DISTANCE_NEAR;
			p_wk->move = FALSE;
		}
		break;

	case CAMERA_FAR:
		if(p_wk->dist < (INIT_CAMERA_DISTANCE_FAR - CAMERA_INOUT_SPEED)){
			p_wk->dist += CAMERA_INOUT_SPEED;
			p_place->list_scale.x += MARK_SCALE_INCDEC;
			p_place->list_scale.y = p_place->list_scale.x;
		}else{
			p_wk->dist = INIT_CAMERA_DISTANCE_FAR;
			p_wk->move = FALSE;
		}
		break;
	}
#if WB_FIX
	GFC_SetCameraDistance(p_wk->dist,p_wk->p_camera);
#else
  _SetCameraDistance(p_wk->p_camera, p_wk->dist);
  GFL_G3D_CAMERA_Switching(p_wk->p_camera);
#endif

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カメラ状態の取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	CAMERA_FAR = 0,	// 遠距離
 *	@retval	CAMERA_NEAR,	// 近距離
 */
//-----------------------------------------------------------------------------
static u32	WLDTIMER_CameraGetStatus( const WLDTIMER_CAMERA* cp_wk )
{
	return cp_wk->status;
}


//----------------------------------------------------------------------------
/**
 *	@brief	タッチ管理初期化
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_TouchInit( WLDTIMER_TOUCH* p_wk, WLDTIMER_DRAWSYS* p_drawsys, WLDTIMER_MSGMAN* p_msgman, u32 heapID, PRINT_QUE *printQue, GFL_FONT *font_handle )
{
	GFL_STD_MemFill( p_wk, 0, sizeof(WLDTIMER_TOUCH) );

	// ボタンビットマップ作成
	p_wk->bttn = GFL_BMPWIN_Create(
				GFL_BG_FRAME1_M,
				WLDTIMER_MAIN_BTTNBMP_X, WLDTIMER_MAIN_BTTNBMP_Y,
				WLDTIMER_MAIN_BTTNBMP_SIZX, WLDTIMER_MAIN_BTTNBMP_SIZY, 
				WLDTIMER_MAIN_BTTNBMP_PAL, GFL_BMP_CHRAREA_GET_B );
	
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->bttn), 15 );
	GFL_BMPWIN_MakeScreen(p_wk->bttn);
  PRINT_UTIL_Setup( &p_wk->print_util, p_wk->bttn );
	
	// やめる描画
	{
		STRBUF* p_str;
		p_str = WLDTIMER_MsgManGetStr( p_msgman, msg_01 );

	#if WB_FIX
		FontProc_LoadFont( NET_FONT_BUTTON, heapID );	//ボタンフォントのロード
		PRINT_UTIL_PrintColor(/*引数内はまだ未移植*/&p_wk->bttn,NET_FONT_BUTTON,p_str,
				4,0,MSG_NO_PUT, WLDTIMER_TOUCH_END_MSG_COL, NULL);
		FontProc_UnloadFont( NET_FONT_BUTTON );				//ボタンフォントの破棄
	#else
		PRINT_UTIL_PrintColor(&p_wk->print_util, printQue, 
		  4, 0, p_str, font_handle, WLDTIMER_TOUCH_END_MSG_COL);
	#endif
	}

	// ウィンドウ描画
	BmpWinFrame_Write(p_wk->bttn,WINDOW_TRANS_ON,
			WLDTIMER_MAIN_SYSTEMWIN_CGX,WLDTIMER_MAIN_SYSTEMWIN_PAL);
}

//----------------------------------------------------------------------------
/**
 *	@brief	タッチ管理破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_TouchExit( WLDTIMER_TOUCH* p_wk )
{
	// ビットマップ破棄
	GFL_BMPWIN_Delete( p_wk->bttn );
	
}

//----------------------------------------------------------------------------
/**
 *	@brief	現状のタッチパネル情報をワークに設定
 *
 *	@param	p_touch		ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_TouchSetParam( WLDTIMER_TOUCH* p_touch )
{
	int dirx,lenx,diry,leny;
  u32 tp_x, tp_y;
  
	p_touch->tp_result = 0;

#if WB_FIX
	if(sys.tp_trg){
#else
  if(GFL_UI_TP_GetPointTrg(&tp_x, &tp_y)){
#endif
		if(	(tp_x >= ((EARTH_ICON_WIN_PX) * DOTSIZE))&&
			(tp_x <= ((EARTH_ICON_WIN_PX + EARTH_ICON_WIN_SX) * DOTSIZE))&&
			(tp_y >= ((EARTH_ICON_WIN_PY) * DOTSIZE))&&
			(tp_y <= ((EARTH_ICON_WIN_PY + EARTH_ICON_WIN_SY) * DOTSIZE))){
			//「やめる」
			p_touch->tp_result = PAD_BUTTON_B;
			return;
		} else {
			p_touch->tp_seq = 0;
			p_touch->tp_lenx = 0;
			p_touch->tp_leny = 0;
			p_touch->tp_count = 0;
			p_touch->tp_result = 0;
			//初回の検出位置を保存
			p_touch->tp_x = tp_x;
			p_touch->tp_y = tp_y;
			p_touch->tp_count = WLDTIMER_TOUCH_ZOOMWAIT_COUNT;
		}
	}
#if WB_FIX
	if(sys.tp_cont){
#else
  if(GFL_UI_TP_GetPointCont(&tp_x, &tp_y)){
#endif
		switch(p_touch->tp_seq){
		case 0:
			//最初のカウントはトリガー認識用に無視
			if(!p_touch->tp_count){
				p_touch->tp_seq++;
			}else{
				p_touch->tp_count--;
			}
		case 1:
			WLDTIMER_Earth_TouchPanelParamGet(p_touch->tp_x,p_touch->tp_y,&dirx,&lenx,&diry,&leny);
			p_touch->tp_result = dirx | diry;
			p_touch->tp_lenx = lenx;
			p_touch->tp_leny = leny;
			p_touch->tp_x = tp_x;
			p_touch->tp_y = tp_y;
			break;
		}
	}else{
		if(p_touch->tp_count){
			p_touch->tp_result = PAD_BUTTON_A;
		}
		p_touch->tp_seq = 0;
		p_touch->tp_lenx = 0;
		p_touch->tp_leny = 0;
		p_touch->tp_count = 0;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	タッチパネルパラメータ取得
 *
 *	@param	cp_touch	タッチワーク
 *	@param	type		取得パラメータタイプ
 *
 *	@return	パラメータ
 */
//-----------------------------------------------------------------------------
static int WLDTIMER_TouchGetParam( const WLDTIMER_TOUCH* cp_touch, u32 type )
{
	int ret;
	switch( type ){
	case WLDTIMER_TOUCH_PM_RESULT:
		ret = cp_touch->tp_result;
		break;
		
	case WLDTIMER_TOUCH_PM_LENX:
		ret = cp_touch->tp_lenx;
		break;
		
	case WLDTIMER_TOUCH_PM_LENY:
		ret = cp_touch->tp_leny;
		break;

	default:
		break;
	}
	return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタン表示OFF
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_TouchBttnOff( WLDTIMER_TOUCH* p_wk )
{
	BmpWinFrame_Clear( p_wk->bttn, WINDOW_TRANS_OFF );
	BmpWinFrame_TransScreen( p_wk->bttn ,WINDOW_TRANS_ON_V);
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタン表示ON
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_TouchBttnOn( WLDTIMER_TOUCH* p_wk )
{
  GFL_BMPWIN_MakeScreen(p_wk->bttn);
	BmpWinFrame_TransScreen( p_wk->bttn ,WINDOW_TRANS_ON_V);
	BmpWinFrame_Write(p_wk->bttn,WINDOW_TRANS_ON,
			WLDTIMER_MAIN_SYSTEMWIN_CGX,WLDTIMER_MAIN_SYSTEMWIN_PAL);
}


//----------------------------------------------------------------------------
/**
 *	@brief	終了チェック	初期化
 *
 *	@param	p_wk			ワーク
 *	@param	p_drawsys		描画ワーク
 *	@param	p_msgman		メッセージ管理
 *	@param	p_save			セーブデータ
 *	@param	heapID			ヒープ
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EndMsgInit( WLDTIMER_END_MSG* p_wk, WLDTIMER_DRAWSYS* p_drawsys, WLDTIMER_MSGMAN* p_msgman, SAVE_CONTROL_WORK* p_save, u32 heapID )
{
	GFL_STD_MemFill( p_wk, 0, sizeof(WLDTIMER_END_MSG) );

	// メッセージスピード
	{
		CONFIG* p_config;
		p_config = SaveData_GetConfig( p_save );
		p_wk->msg_wait = MSGSPEED_GetWait();//CONFIG_GetMsgPrintSpeed( p_config );
	}

	// メッセージバッファ
	p_wk->p_str = GFL_STR_CreateBuffer( WLDTIMER_MSGMAN_STRBUFNUM, heapID );

	// ボタンビットマップ作成
	p_wk->win = GFL_BMPWIN_Create(
				GFL_BG_FRAME1_M,
				WLDTIMER_MAIN_TALKBMP_X, WLDTIMER_MAIN_TALKBMP_Y,
				WLDTIMER_MAIN_TALKBMP_SIZX, WLDTIMER_MAIN_TALKBMP_SIZY, 
				WLDTIMER_MAIN_TALKBMP_PAL, GFL_BMP_CHRAREA_GET_B );
	
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->win), 15 );
	GFL_BMPWIN_MakeScreen(p_wk->win);
	
	// 描画
	WLDTIMER_MsgManGetStrBuff( p_msgman, msg_05, p_wk->p_str );

	// サブウィンドウタッチワーク初期化
	p_wk->p_touch_sw = TOUCH_SW_AllocWork( heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了チェック	ワーク破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EndMsgExit( WLDTIMER_END_MSG* p_wk )
{
	// メッセージ表示中ならOFF
#if WB_FIX
	if( GF_MSG_PrintEndCheck( p_wk->msg_no ) != 0 ){
		GF_STR_PrintForceStop( p_wk->msg_no  );
	}
#else
	if(p_wk->print_stream != NULL){
		PRINTSYS_PrintStreamDelete(p_wk->print_stream);
		p_wk->print_stream = NULL;
	}
#endif

	// メッセージバッファ破棄
	GFL_STR_DeleteBuffer( p_wk->p_str );
	
	// サブウィンドウタッチワーク破棄
	TOUCH_SW_FreeWork( p_wk->p_touch_sw );

	// ビットマップ破棄
	GFL_BMPWIN_Delete( p_wk->win );
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了チェック	開始
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EndMsgStart( WLDTIMER_END_MSG* p_wk, GFL_FONT *font_handle, GFL_TCBLSYS *tcblsys, HEAPID heapID )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->win), 15 );
	GFL_BMPWIN_MakeScreen(p_wk->win);
	// ボタンとメッセージ表示
	TalkWinFrame_Write( p_wk->win, WINDOW_TRANS_OFF, 
			WLDTIMER_MAIN_TALKWIN_CGX, WLDTIMER_MAIN_TALKWIN_PAL );
	BmpWinFrame_TransScreen( p_wk->win ,WINDOW_TRANS_ON_V);

#if WB_FIX
	p_wk->msg_no = PRINTSYS_PrintStream(/*引数内はまだ未対応*/&p_wk->win,FONT_TALK,p_wk->p_str,
			0,0,p_wk->msg_wait, NULL);
#else
  GF_ASSERT(p_wk->print_stream == NULL || PRINTSYS_PrintStreamGetState(p_wk->print_stream) == PRINTSTREAM_STATE_DONE);
  if(p_wk->print_stream != NULL){
    PRINTSYS_PrintStreamDelete(p_wk->print_stream);
  }
  p_wk->print_stream = PRINTSYS_PrintStream(p_wk->win, 0, 0, p_wk->p_str, font_handle, 
    p_wk->msg_wait, tcblsys, 10, heapID, 15);
#endif

	p_wk->seq = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了チェック	メイン
 *
 *	@param	p_wk	ワーク
 *
 *	@retval	TOUCH_SW_RET_NORMAL	// 何もなし
 *	@retval	TOUCH_SW_RET_YES	// はい
 *	@retval	TOUCH_SW_RET_NO		// いいえ
 */
//-----------------------------------------------------------------------------
static u32 WLDTIMER_EndMsgMain( WLDTIMER_END_MSG* p_wk )
{
	u32 ret;
	switch( p_wk->seq ){
	case 0:
		ret = TOUCH_SW_RET_NORMAL;
		// メッセージ終了待ち
#if WB_FIX
		if( GF_MSG_PrintEndCheck( p_wk->msg_no ) == 0 ){
#else
  	if(p_wk->print_stream == NULL || PRINTSYS_PrintStreamGetState(p_wk->print_stream) == PRINTSTREAM_STATE_DONE){
#endif
			TOUCH_SW_PARAM param;
			param		= sc_TOUCH_SW_PARAM;
			TOUCH_SW_Init( p_wk->p_touch_sw, &param );
#if WB_FIX
      ;
#else
      if(p_wk->print_stream != NULL){
        PRINTSYS_PrintStreamDelete(p_wk->print_stream);
        p_wk->print_stream = NULL;
      }
#endif
			p_wk->seq ++;
		}
		break;
	case 1:
		ret = TOUCH_SW_Main( p_wk->p_touch_sw );
		break;
	}
	return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了チェック　消す
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_EndMsgEnd( WLDTIMER_END_MSG* p_wk )
{
	TOUCH_SW_Reset( p_wk->p_touch_sw );

	TalkWinFrame_Clear( p_wk->win, WINDOW_TRANS_OFF );
	BmpWinFrame_TransScreen( p_wk->win ,WINDOW_TRANS_ON_V);
}



//----------------------------------------------------------------------------
/**
 *	@brief	ビューアー初期化
 *
 *	@param	p_wk		ワーク
 *	@param	p_drawsys	描画システム
 *	@param	p_msgman	メッセージ
 *	@param	heapID		ヒープＩＤ
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerInit( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys, WLDTIMER_MSGMAN* p_msgman, u32 heapID, GFL_FONT *font_handle, PRINT_QUE *printQue )
{
	p_wk->seq = WLDTIMER_VIEWER_SEQ_MAIN;

	// キューパラメータ初期化
	WLDTIMER_ViewerQInit( p_wk );

	// タイムゾーンアニメ初期化
	{
		int i;
		
		for( i=0; i<WLDTIMER_ZONETYPE_NUM; i++ ){
			WLDTIMER_TimeZoneAnm_Init( &p_wk->anm[ i ], p_drawsys, &sc_WLDTIMER_TIMEZONE_ANMINIT[i], WLDTIMER_VIEWER_ANM_FRAME[i], heapID );
		}
	}

	// トークメッセージ表示
	{
		STRBUF* p_str;

		p_wk->talkwin = GFL_BMPWIN_Create(
					GFL_BG_FRAME0_S,
					WLDTIMER_SUB_TALKBMP_X, WLDTIMER_SUB_TALKBMP_Y,
					WLDTIMER_SUB_TALKBMP_SIZX, WLDTIMER_SUB_TALKBMP_SIZY, 
					WLDTIMER_SUB_TALKBMP_PAL, GFL_BMP_CHRAREA_GET_B );
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->talkwin), 15 );
		GFL_BMPWIN_MakeScreen(p_wk->talkwin);
		PRINT_UTIL_Setup( &p_wk->print_util_talk, p_wk->talkwin );
		
		p_str = WLDTIMER_MsgManGetStr( p_msgman, msg_00 );
#if WB_FIX
		GF_STR_PrintSimple(&p_wk->talkwin,FONT_TALK,p_str,0,0,MSG_NO_PUT,NULL);
#else
  	PRINT_UTIL_Print( &p_wk->print_util_talk, printQue, 0, 0, p_str, font_handle );
#endif

		TalkWinFrame_Write( p_wk->talkwin, WINDOW_TRANS_ON, 
				WLDTIMER_SUB_TALKWIN_CGX, WLDTIMER_SUB_TALKWIN_PAL );
	}

	// 地域メッセージ初期化
	WLDTIMER_ViewerMsgInit( p_wk, p_drawsys, heapID );


	// フェード用スクリーンデータ読み込み
	{
		p_wk->p_fadescrnbuff = GFL_ARCHDL_UTIL_LoadScreen(p_drawsys->p_handle, 
				NARC_worldtimer_world_watch_roll_NSCR, FALSE,
				&p_wk->p_fadescrndata, heapID);

	}

	// ポケモンバルーン初期化
	WLDTIMER_PokeBaloon_Init( &p_wk->poke, p_drawsys, &p_wk->wnd, heapID );

	// ウィンドウマスク設定
	WLDTIMER_ViewerWndInit( &p_wk->wnd );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ビューアー破棄
 *
 *	@param	p_wk		ワーク
 *	@param	p_drawsys	描画システム
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerExit( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys )
{
	// ウィンドウマスク
	WLDTIMER_ViewerWndExit( &p_wk->wnd );

	// ポケモンバルーン破棄
	WLDTIMER_PokeBaloon_Exit( &p_wk->poke, p_drawsys );

	// フェードスクリーンデータ破棄
	GFL_HEAP_FreeMemory( p_wk->p_fadescrnbuff );

	// 地域メッセージ破棄
	WLDTIMER_ViewerMsgExit( p_wk );
	
	// メッセージ破棄
	GFL_BMPWIN_Delete( p_wk->talkwin );
	
	// キューパラメータ破棄
	WLDTIMER_ViewerQExit( p_wk );
	
	// タイムゾーンアニメ破棄
	{
		int i;
		
		for( i=0; i<WLDTIMER_ZONETYPE_NUM; i++ ){
			WLDTIMER_TimeZoneAnm_Exit( &p_wk->anm[i] );
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ビューアーメイン
 *	
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerMain( WLDTIMER_VIEWER* p_wk, WLDTIMER_MSGMAN* p_msgman, WLDTIMER_DRAWSYS* p_drawsys, PRINT_QUE *printQue, GFL_FONT *font_handle )
{
	BOOL result;
	
	switch( p_wk->seq ){

	// メイン処理
	case WLDTIMER_VIEWER_SEQ_MAIN:
		// タイムゾーンアニメメイン
		WLDTIMER_ViewerAnmCont( p_wk, p_drawsys );

		// ポケモンバルーンメイン
		WLDTIMER_PokeBaloon_Main( &p_wk->poke, &p_wk->wnd );
		break;
		
	// フェード初期化
	case WLDTIMER_VIEWER_SEQ_FADE_INIT:
		WLDTIMER_ViewerFadeInit( p_wk, p_msgman, p_drawsys, printQue, font_handle );
		p_wk->seq = WLDTIMER_VIEWER_SEQ_FADE;
		break;
	
	// フェード処理
	case WLDTIMER_VIEWER_SEQ_FADE:
		result = WLDTIMER_ViewerFade( p_wk, p_drawsys );
		if( result == TRUE ){
			p_wk->seq = WLDTIMER_VIEWER_SEQ_MAIN;
		}
		break;

	// フェードスキップ初期化
	case WLDTIMER_VIEWER_SEQ_FADE_SKIP_INIT:
		WLDTIMER_ViewerFade_SkipInit01( p_wk );
		WLDTIMER_ViewerFade_Skip01( p_wk, p_drawsys );	// １かい動かす
		p_wk->seq = WLDTIMER_VIEWER_SEQ_FADE_SKIP;
		break;

	// フェードスキップ処理
	case WLDTIMER_VIEWER_SEQ_FADE_SKIP:
		result = WLDTIMER_ViewerFade_Skip01( p_wk, p_drawsys );
		if( result == TRUE ){
			p_wk->seq = WLDTIMER_VIEWER_SEQ_MAIN;
		}
		break;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話ウィンドウのOFF
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerTalkWinOff( WLDTIMER_VIEWER* p_wk )
{
	TalkWinFrame_Clear( p_wk->talkwin, WINDOW_TRANS_OFF );
	BmpWinFrame_TransScreen( p_wk->talkwin ,WINDOW_TRANS_ON_V);
}

//----------------------------------------------------------------------------
/**
 *	@brief	会話ウィンドウのON
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerTalkWinOn( WLDTIMER_VIEWER* p_wk )
{
  GFL_BMPWIN_MakeScreen(p_wk->talkwin);
	BmpWinFrame_TransScreen( p_wk->talkwin ,WINDOW_TRANS_ON_V);
	TalkWinFrame_Write( p_wk->talkwin, WINDOW_TRANS_OFF, 
			WLDTIMER_SUB_TALKWIN_CGX, WLDTIMER_SUB_TALKWIN_PAL );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ビューアー情報をプッシュ
 *
 *	@param	p_wk		ワーク
 *	@param	cp_data		追加データ
 *
 *	@retval	TRUE	成功
 *	@retval	FALSE	タイミングがわるい
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_ViewerPushData( WLDTIMER_VIEWER* p_wk, const WLDTIMER_POINTDATA* cp_data )
{
	// 切り替えアニメへ
	switch( p_wk->seq ){
	case WLDTIMER_VIEWER_SEQ_MAIN:
		p_wk->seq = WLDTIMER_VIEWER_SEQ_FADE_INIT;
		break;
		
	case WLDTIMER_VIEWER_SEQ_FADE:
		p_wk->seq = WLDTIMER_VIEWER_SEQ_FADE_SKIP_INIT;
		return FALSE;
		
	// タイミングがわるい・・・
	case WLDTIMER_VIEWER_SEQ_FADE_SKIP_INIT:
	case WLDTIMER_VIEWER_SEQ_FADE_SKIP:
	case WLDTIMER_VIEWER_SEQ_FADE_INIT:
		return FALSE;
	}

	// データをプッシュ
	WLDTIMER_ViewerQPush( p_wk, cp_data );

	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	同じデータがあるかチェック
 *
 *	@param	cp_wk		ワーク
 *	@param	cp_data		チェックするデータ
 *
 *	@retval	TRUE	データがある
 *	@retval	FALSE	データがない
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_ViewerEqualDataCheck( const WLDTIMER_VIEWER* cp_wk, const WLDTIMER_POINTDATA* cp_data )
{
	int i;
	WLDTIMER_POINTDATA point;
	BOOL result;

	for( i=0; i<WLDTIMER_VIEWER_DRAWNUM; i++ ){
		result = WLDTIMER_ViewerQGetData( cp_wk, &point, i );
		if( result == FALSE ){
			return FALSE;	// 一緒のなし
		}

		if( (point.nation == cp_data->nation) &&
			(point.area == cp_data->area) ){
			return TRUE;
		}
	}
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ビューアーＱの処理
 */
//-----------------------------------------------------------------------------
// 初期化
static void WLDTIMER_ViewerQInit( WLDTIMER_VIEWER* p_wk )
{
	p_wk->top = 0;
	p_wk->tail = 0;
	p_wk->datacount = 0;
}
// 破棄
static void WLDTIMER_ViewerQExit( WLDTIMER_VIEWER* p_wk )
{
	p_wk->top = 0;
	p_wk->tail = 0;
	p_wk->datacount = 0;
}
// プッシュ
static void WLDTIMER_ViewerQPush( WLDTIMER_VIEWER* p_wk, const WLDTIMER_POINTDATA* cp_data )
{
	// 末尾+1が先頭の時満杯
	if( ((p_wk->tail + 1)%WLDTIMER_VIEWER_BUFFNUM) == p_wk->top ){
		// 満杯
		// 1つデータをポップしてデータ格納
		WLDTIMER_ViewerQPop( p_wk );
	}

	// データ設定
	p_wk->data[ p_wk->tail ] = *cp_data;

	// 末尾位置を動かす
	p_wk->tail = (p_wk->tail + 1)%WLDTIMER_VIEWER_BUFFNUM;

	// データ数カウント
	p_wk->datacount ++;

//	OS_Printf( "set nation %d  area %d time %d\n", cp_data->nation, cp_data->area, ((cp_data->timezone + WLDTIMER_TIMEZONE_START_HOUR) % WLDTIMER_TIMEZONE_DATANUM) );
}
// ポップ
static void WLDTIMER_ViewerQPop( WLDTIMER_VIEWER* p_wk )
{
	// 先頭=末尾	データがない
	if( p_wk->tail == p_wk->top ){
		return ;
	}

	// 先頭位置を動かす
	p_wk->top = (p_wk->top + 1)%WLDTIMER_VIEWER_BUFFNUM;

	p_wk->datacount --;
}
//----------------------------------------------------------------------------
/**
 *	@brief	ビューアーＱから今入っているデータを取得する
 *
 *	@param	cp_wk		ワーク
 *	@param	p_data		データ
 *	@param	index		tail位置からのインデックス
 *
 *	@retval	TRUE	データある
 *	@retval	FALSE	データなし
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_ViewerQGetData( const WLDTIMER_VIEWER* cp_wk, WLDTIMER_POINTDATA* p_data, u32 index )
{
	s32 index_num;
	
	if( index >= cp_wk->datacount ){
		return FALSE;
	}

	
	// 取得するバッファのインデックスを求める
	index_num = (cp_wk->tail-1) - index;
	if( index_num < 0 ){
		index_num += WLDTIMER_VIEWER_BUFFNUM;
	}

	// 
	*p_data =  cp_wk->data[ index_num ];

	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ビューアーアニメ管理
 *
 *	@param	p_wk		ワーク
 *	@param	p_drawsys	描画システム
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerAnmCont( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys )
{
	int i;

	for( i=0; i<WLDTIMER_ZONETYPE_NUM; i++ ){
		WLDTIMER_TimeZoneAnm_Main( &p_wk->anm[i], p_drawsys );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェード処理初期化
 *
 *	@param	p_wk		ワーク
 *	@param	p_msgman	メッセージマネージャ
 *	@param	p_drawsys	描画システム
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerFadeInit( WLDTIMER_VIEWER* p_wk, WLDTIMER_MSGMAN* p_msgman, WLDTIMER_DRAWSYS* p_drawsys, PRINT_QUE *printQue, GFL_FONT *font_handle )
{
	// フェードワーク初期化
	GFL_STD_MemFill( p_wk->fade, 0, sizeof(WLDTIMER_VIEWER_FADE)*WLDTIMER_VIEWER_FADE_DIV );
	p_wk->fadecount = 0;
	p_wk->fade_divnum = 0;
	
	// 新しいメッセージとアニメフラグを設定
	// アニメも初期化
	{
		int i;
		int drawtype;
		int zonetype;
		BOOL dataflag;
		int timezone_tmp;
		WLDTIMER_POINTDATA pointdata;

		// アニメフラグをリセット
		for( i=0; i<WLDTIMER_ZONETYPE_NUM; i++ ){
			WLDTIMER_TimeZoneAnm_ResetFlag( &p_wk->anm[i] );
		}

		// ポケモンバルーンをリセット
		WLDTIMER_PokeBaloon_Reset( &p_wk->poke );

		// タイムゾーンチェックワーク初期化
		timezone_tmp = -1;

		// UNDERからTOPへとデータを取得
		for( i=0; i<WLDTIMER_VIEWER_DRAWNUM; i++ ){
			drawtype = WLDTIMER_VIEWER_DRAW_UND - i;
			dataflag = WLDTIMER_ViewerQGetData( p_wk, &pointdata, i );

			if( dataflag == TRUE ){

				// ゾーンタイプ取得
				zonetype = WLDTIMER_TIMEZONE_GetZoneType( pointdata.timezone );
					
				// アニメフラグ設定
				WLDTIMER_TimeZoneAnm_SetFlag( &p_wk->anm[ zonetype ], drawtype, TRUE );

				// 地域データ書き込み
				WLDTIMER_ViewerMsgWrite( p_wk, drawtype, &pointdata, p_msgman, printQue, font_handle );

				// 描画タイプー＞タイムゾーンテーブルに格納
				p_wk->drawtype_zonetype[ drawtype ] = zonetype;

				// ポケモンバルーン設定
				WLDTIMER_PokeBaloon_SetDraw( &p_wk->poke, drawtype, 
						pointdata.timezone, timezone_tmp );

				// フェードする分割の数を求める
				p_wk->fade_divnum += WLDTIMER_VIEWER_FADE_DIV_ONE;

				// タイムゾーン保存
				timezone_tmp = pointdata.timezone;
			}else{
				// 描画タイプー＞タイムゾーンテーブルに格納
				p_wk->drawtype_zonetype[ drawtype ] = WLDTIMER_ZONETYPE_NUM;

				// タイムゾーン初期化
				timezone_tmp = -1;
			}
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードメイン
 *
 *	@param	p_wk		ワーク	
 *	@param	p_drawsys	描画システム
 *
 *	@retval	TRUE	終了
 *	@retval	FALSE	途中
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_ViewerFade( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys )
{
	return WLDTIMER_ViewerFade_Main( p_wk, p_drawsys, WLDTIMER_VIEWER_FADE_DIV_START );
}

//----------------------------------------------------------------------------
/**
 *	@brief	スキップ時のフェード初期化
 *
 *	@param	p_wk		ワーク
 *
 *	スキップ処理は１度　ViewerFadeInitが１度行われた状態で呼ばれる必要がある
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerFade_SkipInit( WLDTIMER_VIEWER* p_wk )
{
	int startnum;	// フェード開始数
	int restnum;
	int i;
	
	// フェードカウンタからスキップ用フェードカウンタを求める
	startnum = p_wk->fadecount / WLDTIMER_VIEWER_FADE_DIV_START;

	// これからスキップしていくときに、
	// 一気にフェードさせる数で割り切れるちょうどいい値に調整しておく
	restnum = (startnum % WLDTIMER_VIEWER_FADE_SKIP_ONE);
	if( restnum > 0 ){
		startnum += WLDTIMER_VIEWER_FADE_SKIP_ONE-restnum;
	}

	for( i=0; i<startnum; i++ ){
		WLDTIMER_ViewerFadeDiv_Start( p_wk, i );
	}

	// フェードかウント値を調整する
	p_wk->fadecount = startnum / WLDTIMER_VIEWER_FADE_SKIP_ONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	スキップ時のフェード処理
 *
 *	@param	p_wk		ワーク
 *	@param	p_drawsys	描画システム
 *
 *	@retval	TRUE	終了
 *	@retval	FALSE	途中
 */
//-----------------------------------------------------------------------------
BOOL WLDTIMER_ViewerFade_Skip( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys )
{
	int i;
	BOOL result;
	BOOL ret = TRUE;


	// フェード開始チェック
	if( p_wk->fadecount < WLDTIMER_VIEWER_FADE_SKIP_COUNTNUM ){
		for( i=0;  i<WLDTIMER_VIEWER_FADE_SKIP_ONE; i++ ){
			WLDTIMER_ViewerFadeDiv_Start( p_wk, 
					(p_wk->fadecount*WLDTIMER_VIEWER_FADE_SKIP_ONE)+i );
		}
	}

	// カウント処理
	if( (p_wk->fadecount+1) < WLDTIMER_VIEWER_FADE_SKIP_COUNTNUM ){
		p_wk->fadecount ++;
	}

	//　全部開始させながら動かす
	for( i=0; i<p_wk->fade_divnum; i++ ){
		result = WLDTIMER_ViewerFadeDiv_Main( p_wk, i, p_drawsys );

		// 1つでも終わってなければ、続ける
		if( result == FALSE ){
			ret = FALSE;
		}
	}

	// 終了時にポケモンバルーンの表示をOFFする
	if( ret == TRUE ){
		WLDTIMER_PokeBaloon_CleanDraw( &p_wk->poke );
		WLDTIMER_ViewerWnd1SetPos( &p_wk->wnd, 0, 0 );
	}

	return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief	スキップ処理	上下から
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerFade_SkipInit01( WLDTIMER_VIEWER* p_wk )
{
	int startnum;
	
	// フェードカウンタからスキップ用フェードカウンタを求める
	startnum = p_wk->fadecount / WLDTIMER_VIEWER_FADE_DIV_START;
	p_wk->fadecount = startnum * WLDTIMER_VIEWER_FADE_SKIP01_START;
}
static BOOL WLDTIMER_ViewerFade_Skip01( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys )
{
	return WLDTIMER_ViewerFade_Main( p_wk, p_drawsys, WLDTIMER_VIEWER_FADE_SKIP01_START );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェード共通処理
 *
 *	@param	p_wk		ワーク
 *	@param	p_drawsys	描画システム
 *	@param	fade_timing	タイミング
 *
 *	@retval	TRUE	終了
 *	@retval	FALSE	途中
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_ViewerFade_Main( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys, int fade_timing )
{
	u32 start;
	int i;
	BOOL result;
	BOOL ret = TRUE;
	
	// フェード開始チェック
	if((p_wk->fadecount % fade_timing) == 0){
		start = p_wk->fadecount / fade_timing;
		WLDTIMER_ViewerFadeDiv_Start( p_wk, start );
	}

	// カウント処理
	if( (p_wk->fadecount+1) < (WLDTIMER_VIEWER_FADE_DIV*fade_timing) ){
		p_wk->fadecount ++;
	}

	// メイン処理
	for( i=0; i<p_wk->fade_divnum; i++ ){
		result = WLDTIMER_ViewerFadeDiv_Main( p_wk, i, p_drawsys );

		// 1つでも終わってなければ、続ける
		if( result == FALSE ){
			ret = FALSE;
		}
	}

	// 終了時にポケモンバルーンの表示をOFFする
	if( ret == TRUE ){
		WLDTIMER_PokeBaloon_CleanDraw( &p_wk->poke );
		WLDTIMER_ViewerWnd1SetPos( &p_wk->wnd, 0, 0 );
	}

	return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief	分割フェード処理	開始
 *
 *	@param	p_wk	ワーク
 *	@param	idx		インデックス
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerFadeDiv_Start( WLDTIMER_VIEWER* p_wk, u32 idx )
{
	p_wk->fade[ idx ].start = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	分割フェード処理	メイン
 *
 *	@param	p_wk		ワーク
 *	@param	idx			フェードインデックス
 *	@param	p_drawsys	描画システム
 *
 *	@retval	TRUE	完了
 *	@retval	FALSE	途中
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_ViewerFadeDiv_Main( WLDTIMER_VIEWER* p_wk, u32 idx, WLDTIMER_DRAWSYS* p_drawsys )
{
	u32 y;
	u32 ofs_y;
	u32 drawtype;
	u32 zonetype;
	
	if( p_wk->fade[idx].start == FALSE ){
		return FALSE;	// はじまってもいない
	}

	// 終了チェック
	if( p_wk->fade[idx].count >= WLDTIMER_VIEWER_FADE_DIV_SEQ_NUM ){
		return TRUE;
	}

	// 自分の描画タイプ、タイムゾーンを求める
	y		 = (WLDTIMER_VIEWER_FADE_DIV - idx) - 1;	//1orgを0orgにする
	drawtype = y / WLDTIMER_VIEWER_FADE_DIV_ONE;
	ofs_y	 = y % WLDTIMER_VIEWER_FADE_DIV_ONE;
	zonetype = p_wk->drawtype_zonetype[ drawtype ];

//	OS_Printf( "fade idx[%d] y[%d] drawtype[%d] zonetype[%d] count[%d]\n", idx, y, drawtype, zonetype, p_wk->fade[idx].count );

	switch( p_wk->fade[idx].count ){

	// ひっくり返しフレームを出す
	case WLDTIMER_VIEWER_FADE_DIV_SEQ_CHANGE00:
		// ひっくり返しスクリーンに切り替え
		// クリーンキャラクタの転送
		{
			u8* p_buff;
			p_buff = WLDTIMER_ViewerDummyGetCharBuffPtr( p_wk, ofs_y );
			WLDTIMER_ViewerMsgCharTrans( p_wk, drawtype, ofs_y, p_buff );
		}
		// 背景のすくりーんをひっくり返す
		{
			WLDTIMER_ViewerFadeScrn_LineTrans( p_wk, y, p_drawsys );
		}
		// OAMウィンドウ設定
		{
			WLDTIMER_ViewerWnd1SetPos( &p_wk->wnd, y*8, WLDTIMER_VIEWER_FADE_DIV*8 );
		}
		break;

	// 新しい面を出す
	case WLDTIMER_VIEWER_FADE_DIV_SEQ_CHANGE01:
		// 新スクリーン出す
		// 新キャラクタの転送
		{
			u8* p_buff;
			p_buff = WLDTIMER_ViewerMsgGetCharBuffPtr( p_wk, drawtype, ofs_y );
			WLDTIMER_ViewerMsgCharTrans( p_wk, drawtype, ofs_y, p_buff );
		}
		// 背景のすくりーんをひっくり返す
		{
			WLDTIMER_TimeZoneAnm_LineTrans( &p_wk->anm[ zonetype ], y, p_drawsys );
		}
		break;

	// 待ち
	default:
		break;
	}

	p_wk->fade[idx].count++;
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェード用スクリーンデータを転送する
 *
 *	@param	p_wk		ワーク
 *	@param	drawtyep	描画タイプ
 *	@param	y			ｙ座標
 *	@param	p_drawsys	描画システム
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerFadeScrn_LineTrans( WLDTIMER_VIEWER* p_wk, u32 y, WLDTIMER_DRAWSYS* p_drawsys )
{

	GFL_BG_WriteScreenExpand(
			GFL_BG_FRAME2_S, 
			WLDTIMER_VIEWER_SCRN_X, 
			WLDTIMER_VIEWER_SCRN_Y+y,
			WLDTIMER_VIEWER_SCRN_SX, 1,
			p_wk->p_fadescrndata->rawData,
			WLDTIMER_VIEWER_SCRN_X,
			WLDTIMER_VIEWER_SCRN_Y+y,
			p_wk->p_fadescrndata->screenWidth/8,
			p_wk->p_fadescrndata->screenHeight/8 );

	// 転送フラグを立てる
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ウィンドウマスク初期化
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerWndInit( WLDTIMER_VWND* p_wk )
{
	// ウィンドウの中はOBJは表示しない
	GXS_SetVisibleWnd( GX_WNDMASK_W0|GX_WNDMASK_W1 );
	G2S_SetWndOutsidePlane( GX_WND_PLANEMASK_BG0|GX_WND_PLANEMASK_BG1|GX_WND_PLANEMASK_BG2|GX_WND_PLANEMASK_BG3|GX_WND_PLANEMASK_OBJ, FALSE );
	G2S_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG0|GX_WND_PLANEMASK_BG1|GX_WND_PLANEMASK_BG2|GX_WND_PLANEMASK_BG3, FALSE );
	G2S_SetWnd1InsidePlane( GX_WND_PLANEMASK_BG0|GX_WND_PLANEMASK_BG1|GX_WND_PLANEMASK_BG2|GX_WND_PLANEMASK_BG3, FALSE );

	WLDTIMER_ViewerWnd0SetPos( p_wk, 0, 0 );
	WLDTIMER_ViewerWnd1SetPos( p_wk, 0, 0 );

	// ウィンドウデータ設定タスク
	p_wk->p_tcb = GFUser_VIntr_CreateTCB( WLDTIMER_ViewerWndTcb, p_wk, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ウィンドウマスク破棄
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerWndExit( WLDTIMER_VWND* p_wk )
{
	GFL_TCB_DeleteTask( p_wk->p_tcb );
	GXS_SetVisibleWnd( GX_WNDMASK_NONE );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ウィンドウマスクの座標を設定
 *
 *	@param	sy	Y開始
 *	@param	ey	Y終了
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerWnd0SetPos( WLDTIMER_VWND* p_wk, s16 sy, s16 ey )
{
	if( sy < 0 ){
		sy = 0;
	}
	if( ey < 0 ){
		ey = 0;
	}
	p_wk->wnd0 = TRUE;
	p_wk->wnd0_sy = sy;
	p_wk->wnd0_ey = ey;
}
static void WLDTIMER_ViewerWnd1SetPos( WLDTIMER_VWND* p_wk, s16 sy, s16 ey )
{
	if( sy < 0 ){
		sy = 0;
	}
	if( ey < 0 ){
		ey = 0;
	}
	p_wk->wnd1 = TRUE;
	p_wk->wnd1_sy = sy;
	p_wk->wnd1_ey = ey;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ウィンドウデータ設定タスク
 *
 *	@param	p_tcb	TCB
 *	@param	p_work	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerWndTcb( GFL_TCB* p_tcb, void* p_work )
{
	WLDTIMER_VWND* p_wk = p_work;

	if( p_wk->wnd0 ){
		G2S_SetWnd0Position( 0, p_wk->wnd0_sy, 255, p_wk->wnd0_ey );
		p_wk->wnd0 = FALSE;
	}
	if( p_wk->wnd1 ){
		G2S_SetWnd1Position( 0, p_wk->wnd1_sy, 255, p_wk->wnd1_ey );
		p_wk->wnd1 = FALSE;
	}
}



//----------------------------------------------------------------------------
/**
 *	@brief	ビューアーメッセージ初期化
 *
 *	@param	p_wk		ワーク
 *	@param	p_drawsys	描画システム
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerMsgInit( WLDTIMER_VIEWER* p_wk, WLDTIMER_DRAWSYS* p_drawsys, u32 heapID )
{
	int i;
	int cgx;


	cgx = WLDTIMER_VIEWER_MSGBMP_CGX;
	for( i=0; i<WLDTIMER_VIEWER_DRAWNUM; i++ ){

		p_wk->msg[i] = GFL_BMPWIN_Create(
			GFL_BG_FRAME1_S,
			WLDTIMER_VIEWER_MSGBMP_X, 
			WLDTIMER_VIEWER_MSGBMP_Y + (WLDTIMER_VIEWER_MSGBMP_SIZY*i),
			WLDTIMER_VIEWER_MSGBMP_SIZX, WLDTIMER_VIEWER_MSGBMP_SIZY,
			WLDTIMER_VIEWER_MSGBMP_PAL, GFL_BMP_CHRAREA_GET_B );

		cgx += WLDTIMER_VIEWER_MSGBMP_CGSIZ;

		GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->msg[i]), 0 );
		//GFL_BMPWIN_MakeScreen(p_wk->msg[i]);  GFL_BMPWIN_MakeTransWindowでやるので。
		PRINT_UTIL_Setup(&p_wk->print_util[i], p_wk->msg[i]);
	
	#if WB_FIX
		GF_BGL_BmpWinOn( &p_wk->msg[i] );
	#else
	  GFL_BMPWIN_MakeTransWindow(p_wk->msg[i]);
	#endif
	}

	p_wk->dummy = GFL_BMPWIN_Create(
		GFL_BG_FRAME1_S,
		WLDTIMER_VIEWER_MSGBMP_X, 
		WLDTIMER_VIEWER_MSGBMP_Y,
		WLDTIMER_VIEWER_MSGBMP_SIZX, WLDTIMER_VIEWER_MSGBMP_SIZY,
		WLDTIMER_VIEWER_MSGBMP_PAL, GFL_BMP_CHRAREA_GET_B );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_wk->dummy), 0 );
	GFL_BMPWIN_MakeScreen(p_wk->dummy);
}

//----------------------------------------------------------------------------
/**
 *	@brief	ビューアーメッセージ破棄
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerMsgExit( WLDTIMER_VIEWER* p_wk )
{
	int i;

	for( i=0; i<WLDTIMER_VIEWER_DRAWNUM; i++ ){
		GFL_BMPWIN_Delete( p_wk->msg[i] );
	}
	GFL_BMPWIN_Delete( p_wk->dummy );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ビューアーメッセージの書き込み
 *
 *	@param	p_wk		ワーク
 *	@param	drawtype	描画位置
 *	@param	cp_data		描画データ
 *	@param	p_msgman	メッセージマネージャ
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerMsgWrite( WLDTIMER_VIEWER* p_wk, u32 drawtype, const WLDTIMER_POINTDATA* cp_data, WLDTIMER_MSGMAN* p_msgman, PRINT_QUE *printQue, GFL_FONT *font_handle )
{
	STRBUF* p_str;
	GFL_BMPWIN* p_bmp;
	u32 zonetype;
	PRINT_UTIL *p_printutil;
	
	GF_ASSERT( drawtype < WLDTIMER_VIEWER_DRAWNUM );
	
	p_bmp = p_wk->msg[drawtype];
	p_printutil = &p_wk->print_util[drawtype];
	
	// クリアカラーで初期化
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_bmp), 0 );

	// ゾーンタイプ取得
	zonetype = WLDTIMER_TIMEZONE_GetZoneType( cp_data->timezone );

	// タイトル表示
	{
		p_str = WLDTIMER_MsgManGetStr( p_msgman, msg_02 );
	#if WB_FIX
		PRINT_UTIL_PrintColor(/*引数内はまだ未移植*/p_bmp,FONT_TALK,p_str,
				WLDTIMER_VIEWER_MSG_TITLE_X,WLDTIMER_VIEWER_MSG_TITLE_Y,
				MSG_NO_PUT, WLDTIMER_VIEWER_MSG_TITLE_COL[ zonetype ], NULL);
	#else
		PRINT_UTIL_PrintColor(p_printutil, printQue, 
		  WLDTIMER_VIEWER_MSG_TITLE_X,WLDTIMER_VIEWER_MSG_TITLE_Y, 
		  p_str, font_handle, WLDTIMER_VIEWER_MSG_TITLE_COL[ zonetype ]);
	#endif
	}

	// 国表示
	{
		p_str = WLDTIMER_MsgManCountryGetStr( p_msgman, cp_data->nation );
		PRINT_UTIL_PrintColor(p_printutil, printQue, 
			WLDTIMER_VIEWER_MSG_NATION_X,WLDTIMER_VIEWER_MSG_NATION_Y,
		  p_str, font_handle, WLDTIMER_VIEWER_MSG_NATION_COL);
	}

	// 地域表示
	{
		p_str = WLDTIMER_MsgManPlaceGetStr( p_msgman, cp_data->nation, cp_data->area );
		PRINT_UTIL_PrintColor(p_printutil, printQue, 
			WLDTIMER_VIEWER_MSG_AREA_X,WLDTIMER_VIEWER_MSG_AREA_Y,
		  p_str, font_handle, WLDTIMER_VIEWER_MSG_NATION_COL);
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ビューアーメッセージ	描画位置とさらにｙ位置のキャラクタポインタ取得
 *			ひっくり返しエフェクト用
 *
 *	@param	p_wk		ワーク
 *	@param	drawtype	描画位置
 *	@param	y			描画位置内のY座標
 */
//-----------------------------------------------------------------------------
static u8* WLDTIMER_ViewerMsgGetCharBuffPtr( WLDTIMER_VIEWER* p_wk, u32 drawtype, u32 y )
{
	u8* p_buff;
	GF_ASSERT( drawtype < WLDTIMER_VIEWER_DRAWNUM );

	// キャラクタバッファ取得
#if WB_FIX
	p_buff = p_wk->msg[ drawtype ].chrbuf;
#else
  p_buff = GFL_BMP_GetCharacterAdrs(GFL_BMPWIN_GetBmp(p_wk->msg[drawtype]));
#endif

	// 目的位置のポインタを返す
	return &p_buff[((WLDTIMER_VIEWER_MSGBMP_SIZX*y)*32)];
}

//----------------------------------------------------------------------------
/**
 *	@brief	ビューアーダミーメッセージ	ｙ位置のキャラクタポインタ取得
 *			ひっくり返しエフェクト用
 *
 *	@param	p_wk		ワーク
 *	@param	y			描画位置内のY座標
 */
//-----------------------------------------------------------------------------
static u8* WLDTIMER_ViewerDummyGetCharBuffPtr( WLDTIMER_VIEWER* p_wk, u32 y )
{
	u8* p_buff;

	// キャラクタバッファ取得
#if WB_FIX
	p_buff = p_wk->dummy.chrbuf;
#else
  p_buff = GFL_BMP_GetCharacterAdrs(GFL_BMPWIN_GetBmp(p_wk->dummy));
#endif

	// 目的位置のポインタを返す
	return &p_buff[((WLDTIMER_VIEWER_MSGBMP_SIZX*y)*32)];
}

//----------------------------------------------------------------------------
/**
 *	@brief	キャラクタデータ転送
 *
 *	@param	p_wk		ワーク
 *	@param	drawtype	描画タイプ
 *	@param	y			ｙ座標
 *	@param	cp_buff		キャラクタバッファ
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_ViewerMsgCharTrans( WLDTIMER_VIEWER* p_wk, u32 drawtype, u32 y, const u8* cp_buff )
{
	u16 cgx;

	// 転送先オフセット取得
#if WB_FIX
	cgx = GF_BGL_BmpWinGet_Chrofs( &p_wk->msg[ drawtype ] );
#else
  cgx = GFL_BMPWIN_GetChrNum(p_wk->msg[drawtype]);
#endif

	// Y座標のオフセットにする
	cgx += (WLDTIMER_VIEWER_MSGBMP_SIZX*y);

	// 転送
	NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG1_CHAR_SUB, cgx*32, 
			(void*)cp_buff, WLDTIMER_VIEWER_MSGBMP_SIZX*32 );
}



//----------------------------------------------------------------------------
/**
 *	@brief	タイムゾーンアニメ	初期化
 *
 *	@param	p_wk		ワーク
 *	@param	p_drawsys	描画システム
 *	@param	cp_init		初期化データ
 *	@param	カウント最大値
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_TimeZoneAnm_Init( WLDTIMER_TIMEZONEANM* p_wk, WLDTIMER_DRAWSYS* p_drawsys, const WLDTIMER_TIMEZONEANM_INIT* cp_init, u16 count_max, u32 heapID )
{
	int i;
	
	GFL_STD_MemFill( p_wk, 0, sizeof(WLDTIMER_TIMEZONEANM) );

	// カウントデータ
	p_wk->count_max = count_max;

	// スクリーン読み込み
	if( cp_init->scrn_frame > 0 ){
		p_wk->scrnframe = cp_init->scrn_frame;	// 数
		for( i=0; i<p_wk->scrnframe; i++ ){
			
			p_wk->p_scrnbuff[i] = GFL_ARCHDL_UTIL_LoadScreen( 
					p_drawsys->p_handle, cp_init->scrn_idx[i],
					FALSE, &p_wk->p_scrndata[i], heapID );
		}
	}

	// パレット読み込み
	if( cp_init->pltt_frame > 0 ){
		
		p_wk->plttframe = cp_init->pltt_frame;
		p_wk->plttno	= cp_init->pltt_no;
		
		p_wk->p_plttbuff = GFL_ARCHDL_UTIL_LoadPalette( 
				p_drawsys->p_handle, cp_init->pltt_idx,
				&p_wk->p_plttdata, heapID );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	タイムゾーンアニメ	破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_TimeZoneAnm_Exit( WLDTIMER_TIMEZONEANM* p_wk )
{
	int i;
	
	// スクリーン破棄
	if( p_wk->scrnframe > 0 ){
		for( i=0; i<p_wk->scrnframe; i++ ){
			GFL_HEAP_FreeMemory( p_wk->p_scrnbuff[i] );
		}
	}

	// パレット読み込み
	if( p_wk->plttframe > 0 ){
		GFL_HEAP_FreeMemory( p_wk->p_plttbuff );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	タイムゾーンアニメ	メイン
 *
 *	@param	p_wk		ワーク
 *	@param	p_drawsys	描画システム
 *	@param	count		カウント値
 *	@param	count_max	最大カウント
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_TimeZoneAnm_Main( WLDTIMER_TIMEZONEANM* p_wk, WLDTIMER_DRAWSYS* p_drawsys )
{
	int i;
	u32 pltt_frame, scrn_frame;

	// フレーム計算
	scrn_frame = (p_wk->count * p_wk->scrnframe) / p_wk->count_max;
	pltt_frame = (p_wk->count * p_wk->plttframe) / p_wk->count_max;

	// カウント処理
	p_wk->count = (p_wk->count + 1) % p_wk->count_max;
		
	// スクリーンアニメ
	if( (p_wk->scrnframe>0) && (scrn_frame!=p_wk->scrnframe_now) ){
		p_wk->scrnframe_now = scrn_frame;	// 保存
		for( i=0; i<WLDTIMER_VIEWER_DRAWNUM; i++ ){
			if( p_wk->drawflag[ i ] == TRUE ){
				// 転送処理
				GFL_BG_WriteScreenExpand(
						GFL_BG_FRAME2_S, 
						WLDTIMER_VIEWER_SCRN_X, 
						WLDTIMER_VIEWER_SCRN_Y+(WLDTIMER_VIEWER_SCRN_SY*i),
						WLDTIMER_VIEWER_SCRN_SX, WLDTIMER_VIEWER_SCRN_SY,
						p_wk->p_scrndata[ p_wk->scrnframe_now ]->rawData,
						WLDTIMER_VIEWER_SCRN_X,
						WLDTIMER_VIEWER_SCRN_Y+(WLDTIMER_VIEWER_SCRN_SY*i),
						p_wk->p_scrndata[ p_wk->scrnframe_now ]->screenWidth/8,
						p_wk->p_scrndata[ p_wk->scrnframe_now ]->screenHeight/8 );

				// 転送フラグを立てる
				GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S );
			}
		}
	}

	// パレットアニメ
	if( (p_wk->plttframe>0) && (pltt_frame!=p_wk->plttframe_now) ){
		BOOL result;
		u8* p_data;
		
		p_wk->plttframe_now = pltt_frame;	// 保存

		// 転送
		p_data = (u8*)p_wk->p_plttdata->pRawData;
		result = NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_SUB,
				p_wk->plttno*32, &p_data[p_wk->plttframe_now*32], 32 );
		GF_ASSERT( result );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメONOFFフラグを設定する
 *
 *	@param	p_wk		ワーク
 *	@param	drawtype	描画タイプ
 *	@param	flag		フラグ
 *	
 *	drawtype
		WLDTIMER_VIEWER_DRAW_TOP,	// 上
		WLDTIMER_VIEWER_DRAW_MID,	// 中
		WLDTIMER_VIEWER_DRAW_UND,	// 下
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_TimeZoneAnm_SetFlag( WLDTIMER_TIMEZONEANM* p_wk, u32 drawtype, BOOL flag )
{
	GF_ASSERT( drawtype < WLDTIMER_VIEWER_DRAWNUM );
	p_wk->drawflag[ drawtype ] = flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメフラグ状態を全部OFFにする
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_TimeZoneAnm_ResetFlag( WLDTIMER_TIMEZONEANM* p_wk )
{
	GFL_STD_MemFill( p_wk->drawflag, 0, sizeof(u8)*4 );
}

//----------------------------------------------------------------------------
/**
 *	@brief	スクリーンデータをラインで転送する
 *
 *	@param	p_wk		ワーク
 *	@param	y			Y座標
 *	@param	p_drawsys	描画システム
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_TimeZoneAnm_LineTrans( WLDTIMER_TIMEZONEANM* p_wk, u32 y, WLDTIMER_DRAWSYS* p_drawsys )
{
	// フレームのスクリーンデータのYラインだけを転送
	if( p_wk->scrnframe>0 ){
		GFL_BG_WriteScreenExpand(
				GFL_BG_FRAME2_S, 
				WLDTIMER_VIEWER_SCRN_X, 
				WLDTIMER_VIEWER_SCRN_Y+y,
				WLDTIMER_VIEWER_SCRN_SX, 1,
				p_wk->p_scrndata[ p_wk->scrnframe_now ]->rawData,
				WLDTIMER_VIEWER_SCRN_X,
				WLDTIMER_VIEWER_SCRN_Y+y,
				p_wk->p_scrndata[ p_wk->scrnframe_now ]->screenWidth/8,
				p_wk->p_scrndata[ p_wk->scrnframe_now ]->screenHeight/8 );

		// 転送フラグを立てる
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S );
	}
}



//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンバルーン	初期化
 *
 *	@param	p_wk		ワーク
 *	@param	p_drawsys	描画システム
 *	@param	p_wnd		ウィンドウシステム
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBaloon_Init( WLDTIMER_POKEBALLOON* p_wk, WLDTIMER_DRAWSYS* p_drawsys, WLDTIMER_VWND* p_wnd, u32 heapID )
{
	int i;
	BOOL result;
#if WB_FIX
	CLACT_HEADER header;
	CLACT_ADD_SIMPLE add = {NULL};
#endif

	// 0クリア
	GFL_STD_MemFill( p_wk, 0, sizeof(WLDTIMER_POKEBALLOON) );

	// データ初期化
	p_wk->drawtype	= WLDTIMER_VIEWER_DRAW_UND;	//  下から表示
	p_wk->wait		= WLDTIMER_VIEWER_POKEBLN_WAIT;
	

#if WB_FIX
	// 登録基本情報作成
	add.ClActSet	= p_drawsys->p_clactset;
	add.ClActHeader = &header;
	add.pri			= WLDTIMER_VIEWER_POKEBLN_SFT_PRI;
	add.DrawArea	= NNS_G2D_VRAM_TYPE_2DSUB;
	add.heap		= heapID;
#endif

	// パレットは共通
#if WB_FIX
	p_wk->p_res[ 0 ][ 1 ] = CLACT_U_ResManagerResAddArcPltt_ArcHandle( 
			p_drawsys->p_resman[1], p_drawsys->p_handle,
			WLDTIMER_VIEWER_POKEBLN_NCLR_GET(i),
			FALSE, WLDTIMER_VIEWER_POKEBLN_NCLR_CONTID, 
			NNS_G2D_VRAM_TYPE_2DSUB, WLDTIMER_VIEWER_POKEBLN_PL_NUM, heapID );
	result =CLACT_U_PlttManagerSetCleanArea( p_wk->p_res[ 0 ][ 1 ] );
	GF_ASSERT( result );
	CLACT_U_ResManagerResOnlyDelete( p_wk->p_res[ 0 ][ 1 ] );
#else
  p_wk->p_resobj_index[0][1] = 
    PLTTSLOT_ResourceSet(p_drawsys->plttslot, p_drawsys->p_handle, 
    WLDTIMER_VIEWER_POKEBLN_NCLR_GET(i), CLSYS_DRAW_SUB, WLDTIMER_VIEWER_POKEBLN_PL_NUM, heapID);
#endif
	
	
	
	// ポケモンのグラフィック読み込み
	for( i=0; i<WLDTIMER_TIME_POKE_NUM; i++ ){
#if WB_FIX
		// キャラクタ読み込み
		p_wk->p_res[ i ][ 0 ] = CLACT_U_ResManagerResAddArcChar_ArcHandle( 
				p_drawsys->p_resman[0], p_drawsys->p_handle,
				WLDTIMER_VIEWER_POKEBLN_NCGR_GET(i),
				FALSE, i, NNS_G2D_VRAM_TYPE_2DSUB, heapID );
		
		// セル
		p_wk->p_res[ i ][ 2 ] = CLACT_U_ResManagerResAddArcKindCell_ArcHandle( 
				p_drawsys->p_resman[2], p_drawsys->p_handle,
				WLDTIMER_VIEWER_POKEBLN_NCER_GET(i),
				FALSE, i, CLACT_U_CELL_RES, heapID );

		// セルアニメ
		p_wk->p_res[ i ][ 3 ] = CLACT_U_ResManagerResAddArcKindCell_ArcHandle( 
				p_drawsys->p_resman[3], p_drawsys->p_handle,
				WLDTIMER_VIEWER_POKEBLN_NANR_GET(i),
				FALSE, i, CLACT_U_CELLANM_RES, heapID );

		// VRAM転送
		result =CLACT_U_CharManagerSetAreaCont( p_wk->p_res[ i ][ 0 ] );
		GF_ASSERT( result );
		CLACT_U_ResManagerResOnlyDelete( p_wk->p_res[ i ][ 0 ] );

		// ヘッダー作成
		CLACT_U_MakeHeader( &header, i, WLDTIMER_VIEWER_POKEBLN_NCLR_CONTID, i, i,
				CLACT_U_HEADER_DATA_NONE, CLACT_U_HEADER_DATA_NONE,
				0, WLDTIMER_VIEWER_POKEBLN_BG_PRI,
				p_drawsys->p_resman[0],
				p_drawsys->p_resman[1],
				p_drawsys->p_resman[2],
				p_drawsys->p_resman[3],
				NULL, NULL );

		// アクター作成
		p_wk->p_act[ i ] = CLACT_AddSimple( &add );

		// オートアニメON
		GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_act[ i ], TRUE );
#else
    //キャラクタ読み込み
    p_wk->p_resobj_index[i][0] = GFL_CLGRP_CGR_Register(p_drawsys->p_handle, 
      WLDTIMER_VIEWER_POKEBLN_NCGR_GET(i), FALSE, CLSYS_DRAW_SUB, heapID);
    //セル＆アニメ読み込み
    p_wk->p_resobj_index[i][2] = GFL_CLGRP_CELLANIM_Register(p_drawsys->p_handle, WLDTIMER_VIEWER_POKEBLN_NCER_GET(i), WLDTIMER_VIEWER_POKEBLN_NANR_GET(i), heapID);

    {
      GFL_CLWK_DATA HeadClwkData;
      
      HeadClwkData.pos_x = 0;
      HeadClwkData.pos_y = 0;
      HeadClwkData.anmseq = 0;
      HeadClwkData.softpri = WLDTIMER_VIEWER_POKEBLN_SFT_PRI;
      HeadClwkData.bgpri = 0;

      p_wk->p_act[i] = GFL_CLACT_WK_Create(p_drawsys->p_clactset, p_wk->p_resobj_index[i][0], 
        p_wk->p_resobj_index[0][1], p_wk->p_resobj_index[i][2], 
        &HeadClwkData, CLSYS_DEFREND_SUB, heapID);
      GFL_CLACT_WK_SetAutoAnmFlag(p_wk->p_act[i], TRUE);
    }
#endif
	
	#if WB_FIX
		CLACT_SetAnmFrame( p_wk->p_act[ i ], FX32_ONE );
	#else
	  GFL_CLACT_WK_SetAutoAnmSpeed(p_wk->p_act[i], FX32_ONE);
	#endif
	}

	// OAM非表示
	WLDTIMER_PokeBaloon_CleanDraw( p_wk );
	WLDTIMER_ViewerWnd1SetPos( p_wnd, 0, 0 );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンバルーン	破棄
 *
 *	@param	p_wk		ワーク
 *	@param	p_drawsys	描画システム
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBaloon_Exit( WLDTIMER_POKEBALLOON* p_wk, WLDTIMER_DRAWSYS* p_drawsys )
{
	int i, j;

	// 音がなってたら停止
#if WB_TEMP_FIX
	Snd_SeStopBySeqNo( WLDTIMER_SND_BALLOON, 0 );	// 風船音停止
#endif


	// リソースとワーク破棄
#if WB_FIX
	for( i=0; i<WLDTIMER_TIME_POKE_NUM; i++ ){
		// ワーク破棄
		GFL_CLACT_WK_Remove( p_wk->p_act[i] );

		// VRAM開放
		CLACT_U_CharManagerDelete( p_wk->p_res[i][0] );
		if( i==0 ){
			// パレット破棄
			CLACT_U_PlttManagerDelete( p_wk->p_res[i][1] );
			CLACT_U_ResManagerResDelete( p_drawsys->p_resman[1], p_wk->p_res[i][1] );
		}
		
		// リソース破棄
		for( j=0; j<WLDTIMER_RESMAN_NUM; j++ ){
			if(j!=1){	// パレット以外破棄
				CLACT_U_ResManagerResDelete( p_drawsys->p_resman[j], p_wk->p_res[i][j] );
			}
		}
	}
#else
	for( i=0; i<WLDTIMER_TIME_POKE_NUM; i++ ){
	  GFL_CLACT_WK_Remove(p_wk->p_act[i]);
	  GFL_CLGRP_CGR_Release(p_wk->p_resobj_index[i][0]);
	  GFL_CLGRP_CELLANIM_Release(p_wk->p_resobj_index[i][2]);
	}
	PLTTSLOT_ResourceFree(p_drawsys->plttslot, p_wk->p_resobj_index[0][1], CLSYS_DRAW_SUB);
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンバルーン	アニメデータ　リセット
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBaloon_Reset( WLDTIMER_POKEBALLOON* p_wk )
{
	GFL_STD_MemFill( p_wk->drawflag, 0, sizeof(u8)*4 );
	GFL_STD_MemFill( p_wk->pokegra, 0, sizeof(u8)*4 );

	p_wk->drawtype	= WLDTIMER_VIEWER_DRAW_UND;
	p_wk->wait		= WLDTIMER_VIEWER_POKEBLN_WAIT;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンバルーン	描画フラグ設定
 *
 *	@param	p_wk		ワーク
 *	@param	drawtype	描画タイプ
 *	@param	timezone	タイムゾーン
 *	@param	last_timezone	１つまえのタイムゾーン
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBaloon_SetDraw( WLDTIMER_POKEBALLOON* p_wk, u8 drawtype, u8 timezone, u8 last_timezone )
{
	int  last_pokegra;
	p_wk->drawflag[ drawtype ]	= TRUE;
	p_wk->pokegra[ drawtype ]	= WLDTIMER_TIMEZONE_GetMons( timezone );

	// 動作初期化
	WLDTIMER_PokeBln_MoveInit( &p_wk->move[drawtype],
			&sc_WLDTIMER_POKEBLN_MOVEDATA_TBL[ p_wk->pokegra[drawtype] ][drawtype] );

	// 1つ前のバルーンの上equalフラグ設定
	// 今のバルーンの下equalフラグ設定
	if( last_timezone<WLDTIMER_TIMEZONE_DATANUM ){
		last_pokegra = WLDTIMER_TIMEZONE_GetMons( last_timezone );
		if( last_pokegra == p_wk->pokegra[ drawtype ] ){
			if( drawtype < WLDTIMER_VIEWER_DRAW_UND ){
				WLDTIMER_PokeBln_TopEqualDataSet( &p_wk->move[drawtype+1], TRUE );
				WLDTIMER_PokeBln_UnderEqualDataSet( &p_wk->move[drawtype], TRUE );
			}
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンバルーン	描画クリーン
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBaloon_CleanDraw( WLDTIMER_POKEBALLOON* p_wk )
{
	int i;
	for( i=0; i<WLDTIMER_TIME_POKE_NUM; i++ ){
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_act[i], FALSE );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンバルーン	メイン処理
 *
 *	@param	p_wk	ワーク
 *	@param	p_wnd	ウィンドウシステム
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBaloon_Main( WLDTIMER_POKEBALLOON* p_wk, WLDTIMER_VWND* p_wnd )
{
	BOOL result;
	
	// 開始ウエイト
	if( p_wk->wait > 0 ){
		p_wk->wait --;

		// ウエイトが０なら動作開始処理を入れる
		if( p_wk->wait == 0 ){
			result = WLDTIMER_PokeBaloon_Start( p_wk, p_wk->drawtype, p_wnd );
			if( result == TRUE ){
				PMSND_PlaySE( WLDTIMER_SND_BALLOON );	// 風船音
			}
		}
		return ;
	}
	
	// 動作
	result = WLDTIMER_PokeBaloon_Move( p_wk, p_wk->drawtype, p_wnd );

	if( result == TRUE ){
		if( (p_wk->drawtype-1) >= 0 ){
			p_wk->drawtype--;
			WLDTIMER_PokeBaloon_Start( p_wk, p_wk->drawtype, p_wnd );
		}else{
#if WB_TEMP_FIX
			Snd_SeStopBySeqNo( WLDTIMER_SND_BALLOON, 0 );	// 風船音停止
#endif
			p_wk->drawtype = WLDTIMER_VIEWER_DRAW_UND;
			p_wk->wait = WLDTIMER_VIEWER_POKEBLN_WAIT;
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	バルーンオブジェクト	動作開始
 *
 *	@param	p_wk		ワーク
 *	@param	drawtype	描画タイプ
 *	@param	p_wnd		ウィンドウ
 *
 *	@retval	TRUE	動作開始した
 *	@retval	FALSE	開始しなかった
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_PokeBaloon_Start( WLDTIMER_POKEBALLOON* p_wk, u32 drawtype, WLDTIMER_VWND* p_wnd )
{
	// アニメ有無チェック
	if( p_wk->drawflag[ drawtype ] == FALSE ){
		return FALSE;
	}

	// 動作リセット
	WLDTIMER_PokeBln_MoveReset( &p_wk->move[drawtype], p_wnd );

	// 表示ON
	GFL_CLACT_WK_SetDrawEnable( p_wk->p_act[ p_wk->pokegra[ drawtype ] ], TRUE );

	// 座標設定
	WLDTIMER_PokeBln_ActSetMatrix( p_wk, drawtype );

	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	バルーンオブジェ動作
 *
 *	@param	p_wk		ワーク
 *	@param	drawtype	動作させる描画タイプ
 *	@param	p_wnd		ウィンドウシステム
 *
 *	@retval	TRUE	動作完了
 *	@retval	FALSE	動作途中
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_PokeBaloon_Move( WLDTIMER_POKEBALLOON* p_wk, u32 drawtype, WLDTIMER_VWND* p_wnd )
{
	BOOL result;

	// アニメ終了チェック
	if( p_wk->drawflag[ drawtype ] == FALSE ){
		return TRUE;
	}

	// アニメメイン
	result = WLDTIMER_PokeBln_MoveMain( &p_wk->move[ drawtype ], p_wnd );	// 動作
	WLDTIMER_PokeBln_ActSetMatrix( p_wk, drawtype );				// 座標設定

	if( result == TRUE ){
		GFL_CLACT_WK_SetDrawEnable( p_wk->p_act[ p_wk->pokegra[ drawtype ] ], FALSE );
	}

	return result;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンバルーン　動作ワーク初期化
 *
 *	@param	p_wk		ワーク
 *	@param	cp_data		データ
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBln_MoveInit( WLDTIMER_POKEBLN_MOVE* p_wk, const WLDTIMER_POKEBLN_MOVEDATA* cp_data )
{
	p_wk->cp_data	= cp_data;
	p_wk->x			= cp_data->sx;
	p_wk->y			= cp_data->sy + WLDTIMER_VIEWER_POKEBLN_OAM_HFSIZ;
	p_wk->count		= 0;
	p_wk->wcount	= 0;
	p_wk->under_equaldata	= FALSE;
	p_wk->top_equaldata		= FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３つの位置が同じポケモンかのフラグ設定
 *
 *	@param	p_wk		ワーク
 *	@param	mskon 
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBln_UnderEqualDataSet( WLDTIMER_POKEBLN_MOVE* p_wk, BOOL flag )
{
	p_wk->under_equaldata = flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief	３つの位置が同じポケモンかのフラグ設定
 *
 *	@param	p_wk		ワーク
 *	@param	mskon 
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBln_TopEqualDataSet( WLDTIMER_POKEBLN_MOVE* p_wk, BOOL flag )
{
	p_wk->top_equaldata = flag;
}

//----------------------------------------------------------------------------
/**
 *	@brief	動作メイン
 *
 *	@param	p_wk	ワーク
 *	@param	p_wnd	ウィンドウ
 *
 *	@retval	TRUE	終了
 *	@retval	FLASE	途中
 */
//-----------------------------------------------------------------------------
static BOOL WLDTIMER_PokeBln_MoveMain( WLDTIMER_POKEBLN_MOVE* p_wk, WLDTIMER_VWND* p_wnd )
{
	
	// 終了チェック
	if( p_wk->count > p_wk->cp_data->countmax ){
		return TRUE;
	}

	// 座標を求める
	WLDTIMER_PokeBln_MatrixCalc( p_wk );


	// カウント処理
	p_wk->count ++;	// 通常動作
	if( (p_wk->wcount+1) > p_wk->cp_data->wcountmax ){	// ゆらゆらアニメ
		p_wk->wcount = 0;
	}else{
		p_wk->wcount++;
	}

	// ウィンドウマスク設定
	WLDTIMER_PokeBln_WndMskSet( p_wk, p_wnd );

	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンバルーン動作	リセット
 *
 *	@param	p_wk	ワーク
 *	@param	p_wnd	ウィンドウ
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBln_MoveReset( WLDTIMER_POKEBLN_MOVE* p_wk, WLDTIMER_VWND* p_wnd )
{
	p_wk->count = 0;
	p_wk->wcount = 0;

	// 座標を求める
	WLDTIMER_PokeBln_MatrixCalc( p_wk );
	// ウィンドウマスク設定
	WLDTIMER_PokeBln_WndMskSet( p_wk, p_wnd );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンバルーン表示座標を取得
 *
 *	@param	cp_wk		ワーク
 *	@param	p_pos		表示座標
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBln_MoveGetPos( const WLDTIMER_POKEBLN_MOVE* cp_wk, VecFx32* p_pos )
{
	p_pos->x = cp_wk->x << FX32_SHIFT;
	p_pos->y = (cp_wk->y << FX32_SHIFT) + WLDTIMER_SF_MAT_Y;
}

//----------------------------------------------------------------------------
/**
 *	@brief	座標の計算と格納のみ行う
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBln_MatrixCalc( WLDTIMER_POKEBLN_MOVE* p_wk )
{
	u16 rota;
	s16 sy, ey;

	// 上が一緒
	if( p_wk->top_equaldata == FALSE ){
		ey = p_wk->cp_data->ey - WLDTIMER_VIEWER_POKEBLN_OAM_HFSIZ;
	}else{
		ey = p_wk->cp_data->ey;
	}

	// 下が一緒
	if( p_wk->under_equaldata == FALSE ){
	  	sy = p_wk->cp_data->sy + WLDTIMER_VIEWER_POKEBLN_OAM_HFSIZ;
	}else{
	  	sy = p_wk->cp_data->sy;
	}

	// 通常移動座標計算
	p_wk->x = p_wk->cp_data->sx + (((p_wk->cp_data->ex - p_wk->cp_data->sx) * p_wk->count) / p_wk->cp_data->countmax);
	p_wk->y = sy + (((ey - sy) * p_wk->count) / p_wk->cp_data->countmax);

	// ゆれ計算
	rota = (p_wk->wcount*0xffff)/p_wk->cp_data->wcountmax;
	p_wk->x += FX_Mul( FX32_CONST(p_wk->cp_data->wx), FX_SinIdx( rota ) ) >> FX32_SHIFT;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ウィンドウマスクを設定
 *
 *	@param	cp_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBln_WndMskSet( const WLDTIMER_POKEBLN_MOVE* cp_wk, WLDTIMER_VWND* p_wnd )
{
	// Y座標でウィンドウマスクを設定する
	WLDTIMER_ViewerWnd0SetPos( p_wnd, 0, 0 );	// 設定の解除
	// 下に設定するかちぇっく
	if( cp_wk->under_equaldata == FALSE ){
		if( cp_wk->y > (cp_wk->cp_data->sy - WLDTIMER_VIEWER_POKEBLN_OAM_HFSIZ) ){
			WLDTIMER_ViewerWnd0SetPos( p_wnd, cp_wk->cp_data->sy, 
					cp_wk->cp_data->sy+WLDTIMER_VIEWER_POKEBLN_OAM_SIZ );
		}
	}
	// 上に設定するかチェック
	if( cp_wk->top_equaldata == FALSE ){
		if( (cp_wk->y - WLDTIMER_VIEWER_POKEBLN_OAM_HFSIZ) <= cp_wk->cp_data->ey ){
			WLDTIMER_ViewerWnd0SetPos( p_wnd, cp_wk->cp_data->ey-WLDTIMER_VIEWER_POKEBLN_OAM_SIZ, 
					cp_wk->cp_data->ey );
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	今のポケモンに今の座標を設定する
 *	
 *	@param	p_wk		ワーク
 *	@param	drawtype	描画タイプ
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_PokeBln_ActSetMatrix( WLDTIMER_POKEBALLOON* p_wk, u32 drawtype )
{
	VecFx32 pos;
	GFL_CLWK* p_obj;
  GFL_CLACTPOS actpos;
  
	WLDTIMER_PokeBln_MoveGetPos( &p_wk->move[ drawtype ], &pos );	// 座標設定
	
	p_obj = p_wk->p_act[ p_wk->pokegra[ drawtype ] ];
#if WB_FIX
	CLACT_SetMatrix( p_obj, &pos );
#else
  actpos.x = FX_Whole(pos.x);
  actpos.y = FX_Whole(pos.y);
  GFL_CLACT_WK_SetPos( p_obj, &actpos, CLSYS_DEFREND_SUB);
#endif
}




//----------------------------------------------------------------------------
/**
 *	@brief	タイムゾーンからゾーンタイプを取得
 *
 *	@param	timezone	タイムゾーン
 *
 *	@return	ゾーンタイプ
 */
//-----------------------------------------------------------------------------
static u16 WLDTIMER_TIMEZONE_GetZoneType( u32 timezone )
{
	GF_ASSERT( timezone<WLDTIMER_TIMEZONE_DATANUM );
	return sc_WLDTIMER_TIMEZONE_DATA[timezone].zone;
}

//----------------------------------------------------------------------------
/**
 *	@brief	タイムゾーンから表示モンスターを取得
 *
 *	@param	timezone	タイムゾーン
 *
 *	@return	表示モンスター
 */
//-----------------------------------------------------------------------------
static u16 WLDTIMER_TIMEZONE_GetMons( u32 timezone )
{
	GF_ASSERT( timezone<WLDTIMER_TIMEZONE_DATANUM );
	return sc_WLDTIMER_TIMEZONE_DATA[timezone].mons;
}


//----------------------------------------------------------------------------
/**
 *	@brief	メッセージマネージャ初期化
 *
 *	@param	p_wk		ワーク
 *	@param	heapID		ヒープ
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_MsgManInit( WLDTIMER_MSGMAN* p_wk, u32 heapID )
{
	// 基本マネージャ作成
	{
		p_wk->p_msgman = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL,ARCID_MESSAGE,NARC_message_worldtimer_dat,heapID );
	}

	// 地域メッセージ
	{
		p_wk->p_wordset = WORDSET_CreateEx( WORDSET_DEFAULT_SETNUM, WORDSET_COUNTRY_BUFLEN, heapID );
	}

	// 共有メッセージ領域確保
	p_wk->p_msgstr = GFL_STR_CreateBuffer( WLDTIMER_MSGMAN_STRBUFNUM, heapID );
	p_wk->p_msgtmp = GFL_STR_CreateBuffer( WLDTIMER_MSGMAN_STRBUFNUM, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メッセージマネージャ破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_MsgManExit( WLDTIMER_MSGMAN* p_wk )
{
	// 基本マネージャ
	{
		GFL_MSG_Delete(p_wk->p_msgman);
	}

	// 地域マネージャ
	{
		WORDSET_Delete( p_wk->p_wordset );
	}

	// 共有メッセージバッファ
	GFL_STR_DeleteBuffer( p_wk->p_msgstr );
	GFL_STR_DeleteBuffer( p_wk->p_msgtmp );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メッセージを取得する
 *
 *	@param	p_wk	ワーク
 *	@param	msg		メッセージ
 *
 *	@return	文字列
 */
//-----------------------------------------------------------------------------
static STRBUF* WLDTIMER_MsgManGetStr( WLDTIMER_MSGMAN* p_wk, u32 msg )
{
	GFL_MSG_GetString( p_wk->p_msgman, msg, p_wk->p_msgstr );
	return p_wk->p_msgstr;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メッセージを取得
 *
 *	@param	p_wk		ワーク
 *	@param	msg			メッセージ
 *	@param	p_str		バッファ
 */
//-----------------------------------------------------------------------------
static void WLDTIMER_MsgManGetStrBuff( WLDTIMER_MSGMAN* p_wk, u32 msg, STRBUF* p_str )
{
	GFL_MSG_GetString( p_wk->p_msgman, msg, p_str );
}

//----------------------------------------------------------------------------
/**
 *	@brief	国の名前を取得
 *
 *	@param	p_wk		ワーク
 *	@param	nationID	国ＩＤ
 *
 *	@return	文字列
 */
//-----------------------------------------------------------------------------
static STRBUF* WLDTIMER_MsgManCountryGetStr( WLDTIMER_MSGMAN* p_wk, u32 nationID )
{
	GFL_MSG_GetString( p_wk->p_msgman, msg_03, p_wk->p_msgtmp );
	WORDSET_RegisterCountryName( p_wk->p_wordset, 0, nationID );
	WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_msgstr, p_wk->p_msgtmp );
	return p_wk->p_msgstr;
}

//----------------------------------------------------------------------------
/**
 *	@brief	地域名を取得
 *
 *	@param	p_wk		ワーク
 *	@param	nationID	国ＩＤ
 *	@param	areaID		地域ＩＤ
 *
 *	@return	文字列
 */
//-----------------------------------------------------------------------------
static STRBUF* WLDTIMER_MsgManPlaceGetStr( WLDTIMER_MSGMAN* p_wk, u32 nationID, u32 areaID )
{
#if PL_G0237_081106_FIX
	WORDSET_ClearAllBuffer( p_wk->p_wordset );
#endif
	
	GFL_MSG_GetString( p_wk->p_msgman, msg_03, p_wk->p_msgtmp );
	WORDSET_RegisterLocalPlaceName( p_wk->p_wordset, 0, nationID, areaID );
	WORDSET_ExpandStr( p_wk->p_wordset, p_wk->p_msgstr, p_wk->p_msgtmp );
	return p_wk->p_msgstr;
}


//----------------------------------------------------------------------------
/**
 *	@brief	世界地図表示地域データから最初に発見されるデータを返す
 *
 *	@param	cp_data	世界地図表示地域データ
 *
 *	@retval	データのINDEX		
 *	@retval	WFLBY_WLDTIMER_DATA_MAX	みつからなかった
 */
//-----------------------------------------------------------------------------
static u32 WLDTIMER_WFLBYDATA_GetFirst( const WFLBY_WLDTIMER* cp_data )
{
	int i;
	u8 dataflag;

	for( i=0; i<WFLBY_WLDTIMER_DATA_MAX; i++ ){
		dataflag = WFLBY_WLDTIMER_GetDataFlag( cp_data, i );
		if( dataflag == TRUE ){
			return i;
		}
	}
	return WFLBY_WLDTIMER_DATA_MAX;
}



