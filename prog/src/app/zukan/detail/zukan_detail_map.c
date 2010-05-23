//============================================================================
/**
 *  @file   zukan_detail_map.c
 *  @brief  図鑑詳細画面の姿画面
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *  モジュール名：ZUKAN_DETAIL_MAP
 */
//============================================================================

#define DEF_SCMD_CHANGE  // これが定義されているとき、CMDが発行されるまで待たずにSCMDが発行されたときに変更を開始する

//#define DEBUG_SET_ANIME_AND_POS  // これが定義されているとき、生息する場所に置くOBJのアニメ指定、位置指定用のデータ調整モードになる
//#define DEBUG_KAWADA


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "pokeicon/pokeicon.h"
#include "gamesystem/gamedata_def.h"
#include "gamesystem/game_data.h"
#include "gamesystem/pm_season.h"
#include "savedata/zukan_savedata.h"
#include "system/bmp_winframe.h"
#include "print/printsys.h"
#include "print/gf_font.h"
#include "print/wordset.h"
#include "sound/pm_sndsys.h"
#include "field/zonedata.h"
#include "app/townmap_data_sys.h"
#include "app/townmap_util.h"

#include "../../../../../resource/fldmapdata/zonetable/zone_id.h"

#if	PM_VERSION == VERSION_BLACK
#include "../../../../../resource/zukan_data/zkn_area_b_zone_group.cdat"
#else
#include "../../../../../resource/zukan_data/zkn_area_w_zone_group.cdat"
#endif

#include "zukan_detail_def.h"
#include "zukan_detail_common.h"
#include "zukan_detail_graphic.h"
#include "zukan_detail_map.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_zkn.h"
#include "townmap_gra.naix"
//#include "zukan_area_w.naix"  // ここに定義されているenum値を
//#include "zukan_area_b.naix"  // 使っていないので不要です。
#include "zukan_gra.naix"

// サウンド

// オーバーレイ
FS_EXTERN_OVERLAY(townmap);


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// メインBGフレーム
#define BG_FRAME_M_ROOT           (GFL_BG_FRAME2_M)
#define BG_FRAME_M_MAP            (GFL_BG_FRAME3_M)
#define BG_FRAME_M_TEXT           (GFL_BG_FRAME0_M)
// メインBGフレームのプライオリティ
#define BG_FRAME_PRI_M_ROOT       (2)
#define BG_FRAME_PRI_M_MAP        (3)
#define BG_FRAME_PRI_M_TEXT       (1)

// メインBGパレット
// 本数
enum
{
  BG_PAL_NUM_M_TEXT          = 1,
};
// 位置
enum
{
  BG_PAL_POS_M_TEXT          = 0,
};

// メインOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_TM           = 5,
  OBJ_PAL_NUM_M_ZUKAN        = 2,
  OBJ_PAL_NUM_M_EXIST        = 1,
};
// 位置
enum
{
  OBJ_PAL_POS_M_TM           = 0,
  OBJ_PAL_POS_M_ZUKAN        = 5,
  OBJ_PAL_POS_M_EXIST        = 7,
};

// サブBGフレーム
#define BG_FRAME_S_TEXT           (GFL_BG_FRAME2_S)
#define BG_FRAME_S_PLACE          (GFL_BG_FRAME3_S)
#define BG_FRAME_S_REAR           (GFL_BG_FRAME0_S)
// サブBGフレームのプライオリティ
#define BG_FRAME_PRI_S_TEXT       (1)
#define BG_FRAME_PRI_S_PLACE      (2)
#define BG_FRAME_PRI_S_REAR       (3)

// サブBGパレット
// 本数
enum
{
  BG_PAL_NUM_S_PLACE             = 8,
  BG_PAL_NUM_S_TEXT              = 1,
  BG_PAL_NUM_S_REAR              = ZKNDTL_COMMON_REAR_BG_PAL_NUM,
};
// 位置
enum
{
  BG_PAL_POS_S_PLACE             = 0,
  BG_PAL_POS_S_TEXT              = 8,
  BG_PAL_POS_S_REAR              = 9,
};

// サブOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_S_POKEICON         = 3,
};
// 位置
enum
{
  OBJ_PAL_POS_S_POKEICON         = 0,
};


// ProcMainのシーケンス
enum
{
  SEQ_START      = 0,
  SEQ_FADE_CHANGE_BEFORE,
  SEQ_PREPARE,
  SEQ_FADE_IN,
  SEQ_BEFORE_MAIN,
  SEQ_MAIN,
  SEQ_AFTER_MAIN,
  SEQ_FADE_OUT,
  SEQ_FADE_CHANGE_AFTER,
  SEQ_END,
};

// 終了命令
typedef enum
{
  END_CMD_NONE,
  END_CMD_INSIDE,
  END_CMD_OUTSIDE,
}
END_CMD;

// BG面設定
typedef struct
{
	u32									frame;
	GFL_BG_BGCNT_HEADER	bgcnt_header;
	u32									mode;
	BOOL								is_visible;
}
BGSETUP;

// GFL_BG_FRAME2_MとGFL_BG_FRAME3_Mをアフィン拡張BGにする
static const BGSETUP sc_bgsetup_affineex[]	=
{	
	//MAIN------------------------
	{	
		GFL_BG_FRAME2_M,	//設定するフレーム
		{
			0, 0, 0x0800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x128,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
          // 後ろから取らないこと！ 256x64が限界！  // 本当はGFL_BG_CHRSIZ256_256x128を指定してやるべきなんだろうなあ
          // GFL_BG_CHRSIZ_256x128は0x4000なのでちょうどサイズが一致している
          // 0x10000～0x14000    64バイト*32キャラ*8キャラ
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
          // GFL_BG_MODE_256X16ではbgExtPlttを使用しないのでてきとーな値
		},
		GFL_BG_MODE_256X16,//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME3_M,	//設定するフレーム
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x14000, GFL_BG_CHRSIZ_256x256,//GFL_BG_CHRSIZ256_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
          // 後ろから取らないこと！ 256x192が限界！  // 本当はGFL_BG_CHRSIZ256_256x256を指定してやるべきなんだろうなあ
          // GFL_BG_CHRSIZ_256x256は0x8000なのでサイズが0xC000には0x4000ほど足りないが大丈夫かな？
          // GFL_BG_CHRSIZ256_256x256は0x10000なので128Kをはみ出してしまうので、はみ出すよりは足りないほうがいいかな？
          // 0x14000～0x20000    64バイト*32キャラ*24キャラ
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
          // GFL_BG_MODE_256X16ではbgExtPlttを使用しないのでGX_BG_EXTPLTT_01はてきとーな値
		},
		GFL_BG_MODE_256X16,//BGの種類
		TRUE,	//初期表示
	},
};

// GFL_BG_FRAME2_MとGFL_BG_FRAME3_MをテキストBGにする
static const BGSETUP sc_bgsetup_text[]	=
{	
	//MAIN------------------------
	{	
		GFL_BG_FRAME2_M,	//設定するフレーム
		{
			0, 0, 0x0800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME3_M,	//設定するフレーム
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x18000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
};

/*
TWL_ProgramingManual.pdf

p43
3.2 TWLプロセッサ内メモリ
図 3-15 BG 拡張パレットスロット メモリマップ
0x00008000
0x00006000  スロット3
0x00004000  スロット2
0x00002000  スロット1
0x00000000  スロット0

p100
6.2.2 BGコントロール
拡張パレットがイネーブルのとき、BG2 は拡張パレットスロット2 を使用し、BG3 は拡張パレットスロット3 を使用し
ます。BG2 および3 において、使用する拡張パレットNo.を変更することはできません。
*/


// メッセージデータ
enum
{
  MSG_ZUKAN,
  MSG_PLACE,
  MSG_MAX
};

// テキスト
enum
{
  TEXT_POKENAME,
  TEXT_PLACENAME,
  TEXT_SEASON,
  TEXT_UNKNOWN,
  TEXT_MAX
};

// テキスト表示に必要なダミーのビットマップウィンドウ
enum
{
  TEXT_DUMMY_UP,
  TEXT_DUMMY_DOWN,
  TEXT_DUMMY_MAX
};

// 文字数
#define STRBUF_POKENAME_LENGTH  ( 64 )  // [ポケモン種族名]のぶんぷ  // buflen.hで確認する必要あり！

// ポケアイコンの位置
static const u8 pokeicon_pos[4] = { 56, 112 +8*8,     // スクロール前(陸上、水上、釣りアイコンが見えていないとき)
                                    56, 112       };  // スクロール後(陸上、水上、釣りアイコンが見えているとき)

// 状態
typedef enum
{
  STATE_TOP,
  STATE_TOP_TO_SELECT,
  STATE_TOP_TO_SELECT_TOUCHBAR_CHANGE,
  STATE_TOP_DRAW_SEASON_AREA,
  STATE_TOP_BEFORE_FADE_OUT,
  STATE_SELECT,
  STATE_SELECT_TO_TOP,
  STATE_SELECT_TO_TOP_TOUCHBAR_CHANGE,
  STATE_SELECT_DRAW_SEASON_AREA,
}
STATE;

typedef enum
{
  DRAW_SEASON_AREA_STEP_NONE,
  DRAW_SEASON_AREA_STEP_OBJ_OFF,
  DRAW_SEASON_AREA_STEP_ALPHA_SET,
  DRAW_SEASON_AREA_STEP_OBJ_ON,
}
DRAW_SEASON_AREA_STEP;

typedef enum
{
  SCROLL_STATE_DOWN,
  SCROLL_STATE_DOWN_TO_UP,
  SCROLL_STATE_UP,
  SCROLL_STATE_UP_TO_DOWN,
}
SCROLL_STATE;

#define SCROLL_VEL (8)  // 上画面のスクロールの速度(1フレームにこれだけ移動する)

#define SEASON_R_ARROW_POS_X  (17*8)
#define SEASON_R_ARROW_POS_Y  (0)
#define SEASON_R_ARROW_SIZE_X (3*8)
#define SEASON_R_ARROW_SIZE_Y (2*8)

#define SEASON_R_ARROW_ANMSEQ_NORMAL (2)
#define SEASON_R_ARROW_ANMSEQ_SELECT (3)

#define SEASON_L_ARROW_POS_X  (0*8)
#define SEASON_L_ARROW_POS_Y  (0)
#define SEASON_L_ARROW_SIZE_X (3*8)
#define SEASON_L_ARROW_SIZE_Y (2*8)

#define SEASON_L_ARROW_ANMSEQ_NORMAL (4)
#define SEASON_L_ARROW_ANMSEQ_SELECT (5)

#define SEASON_PANEL_POS_X  (0)
#define SEASON_PANEL_POS_Y  (0)
#define SEASON_PANEL_SIZE_X (20*8)
#define SEASON_PANEL_SIZE_Y (3*8)

#define DATA_IDX_NONE (0xFF)

// カーソルの移動可能範囲 MIN<= <=MAX
#define CURSOR_MOVE_RECT_X_MIN (0)                     // prog/src/app/townmap/townmap.c  // CURSOR_MOVE_LIMIT_TOPなどを参考にした。
#define CURSOR_MOVE_RECT_Y_MIN (8)//(16)
#define CURSOR_MOVE_RECT_X_MAX (256-8)//(256 -16 -1)
#define CURSOR_MOVE_RECT_Y_MAX (168)//(168 -1)
// カーソルのデフォルト位置
#define CURSOR_DEFAULT_POS_X (128)
#define CURSOR_DEFAULT_POS_Y (96)

// カーソルの移動
#define CURSOR_ADD_SPEED		(3)              // カーソルの移動量
#define PLACE_PULL_R_SQ			(12*12)          // カーソルを吸い込む範囲の二乗
#define PLACE_PULL_STRONG		(FX32_CONST(1))	 // 吸い込む強さ（カーソルCURSOR_ADD_SPEEDより弱く）
#define PLACE_PULL_FORCE    (FX32_SQRT3)     // カーソルが動いていないとき、これ未満の距離なら強制的に吸い込ませる(sqrt(2)< <2を満たす値)

// プレイヤーの位置に配置するOBJのアニメ
#define HERO_ANMSEQ     (6)
// リングのアニメ
#define RING_CUR_ANMSEQ (5)

// OBJ
enum
{
  OBJ_RES_TM_NCG,     // pokemon_wb/prog/src/app/townmap/townmap_grh.c 参考
  OBJ_RES_TM_NCL,
  OBJ_RES_TM_NCE,
  OBJ_RES_ZUKAN_NCG,
  OBJ_RES_ZUKAN_NCL,
  OBJ_RES_ZUKAN_NCE,
  OBJ_RES_EXIST_NCG,
  OBJ_RES_EXIST_NCL,
  OBJ_RES_EXIST_NCE,
  OBJ_RES_MAX,
};
enum
{
  OBJ_ZUKAN_START,                            // OBJ_ZUKAN_START<= <OBJ_ZUKAN_END
  OBJ_SEASON            = OBJ_ZUKAN_START,
  OBJ_SEASON_L,
  OBJ_SEASON_R,
  OBJ_UNKNOWN,
  OBJ_ZUKAN_END,

  OBJ_TM_START          = OBJ_ZUKAN_END,      // OBJ_TM_START<= <OBJ_TM_END
  OBJ_CURSOR            = OBJ_TM_START,
  OBJ_RING_CUR,
  OBJ_HERO,
  OBJ_TM_END,

  OBJ_MAX               = OBJ_TM_END,
  OBJ_PUSH_BUTTON_NONE  = OBJ_MAX,            // 現在押されているボタンがないとき
};

// obj_existのアルファアニメーションsin使うので0～0xFFFFのループ
#define OBJ_EXIST_ALPHA_ANIME_COUNT_ADD (0x400)    // cos使うので0～0xFFFFのループ
#define OBJ_EXIST_ALPHA_ANIME_COUNT_MAX (0x10000)  // 含まない
#define OBJ_EXIST_ALPHA_MIN  ( 1)  // 0<= <=16
#define OBJ_EXIST_ALPHA_MAX  (15)  // OBJ_EXIST_ALPHA_MIN <= <= OBJ_EXIST_ALPHA_MAX


// 各ポケモンの情報
// 一年中か季節ごとか
typedef enum
{
  APPEAR_RULE_YEAR,
  APPEAR_RULE_SEASON,
}
APPEAR_RULE;
// 生息地
typedef enum
{
  HABITAT_UNKNOWN,  // 生息地不明
  HABITAT_KNOWN,    // 生息地判明
}
HABITAT;

// 草原、水、釣りのアイコン
enum
{
  PLACE_ICON_GROUND,
  PLACE_ICON_WATER,
  PLACE_ICON_FISHING,
  PLACE_ICON_MAX,
};
// 草原、水、釣りのアイコンのパレット
static u8 place_icon_bg_pal_pos[PLACE_ICON_MAX][2] =  // 0=OFF, 1=ON
{
  { BG_PAL_POS_S_PLACE   + 5,    BG_PAL_POS_S_PLACE   + 2 },
  { BG_PAL_POS_S_PLACE   + 6,    BG_PAL_POS_S_PLACE   + 0 },
  { BG_PAL_POS_S_PLACE   + 7,    BG_PAL_POS_S_PLACE   + 4 },
};
// 草原、水、釣りのアイコンのスクリーンのキャラ座標、サイズ
static u8 place_icon_screen_chr[PLACE_ICON_MAX][4] =  // 0=X, 1=Y, 2=W, 3=H
{
  {  7, 28,  4,  3 },
  { 14, 28,  4,  3 },
  { 21, 28,  4,  3 },
};

// VBLANK_REQ
enum
{
  VBLANK_REQ_NONE,
  //VBLANK_REQ_OBJ_EXIST_ALPHA_EXIT,
  //VBLANK_REQ_OBJ_UNKNOWN_ALPHA_EXIT,
  VBLANK_REQ_OBJ_EXIST_ALPHA_EXIT_AND_OBJ_UNKNOWN_ALPHA_INIT,
  VBLANK_REQ_OBJ_UNKNOWN_ALPHA_EXIT_AND_OBJ_EXIST_ALPHA_INIT,
  VBLANK_REQ_OBJ_ALL_ALPHA_EXIT_AND_FADE_SET_COLORLESS_IMMEDIATELY,
};


// OBJを差し替える際に乱れないように、2つを交互に表示する
typedef enum
{
  OBJ_SWAP_0,
  OBJ_SWAP_1,
  OBJ_SWAP_MAX,
}
OBJ_SWAP;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// 分布データ(ポケモン1匹分)
//=====================================
enum
{
  PLACE_BITFLAG_NONE         = 0,
  PLACE_BITFLAG_GROUND_L     = 1<< 0,
  PLACE_BITFLAG_GROUND_H     = 1<< 1, 
  PLACE_BITFLAG_GROUND_SP    = 1<< 2,
  PLACE_BITFLAG_WATER        = 1<< 3,
  PLACE_BITFLAG_WATER_SP     = 1<< 4,
  PLACE_BITFLAG_FISHING      = 1<< 5,
  PLACE_BITFLAG_FISHING_SP   = 1<< 6,

  PLACE_BITFLAG_GROUND_ALL   = ( PLACE_BITFLAG_GROUND_L | PLACE_BITFLAG_GROUND_H | PLACE_BITFLAG_GROUND_SP ),
  PLACE_BITFLAG_WATER_ALL    = ( PLACE_BITFLAG_WATER | PLACE_BITFLAG_WATER_SP ),
  PLACE_BITFLAG_FISHING_ALL  = ( PLACE_BITFLAG_FISHING | PLACE_BITFLAG_FISHING_SP ),
};
typedef struct
{
  u8    unknown;  // 0のとき不明でない、1のとき不明
  u8    place_bitflag[TOWNMAP_DATA_MAX];  // TOWNMAP_DATAのインデックスを配列添え字に使えるように並べてある
}
AREA_SEASON_DATA;
typedef struct
{
  u8                 year;  // 0のとき一年中同じでない、1のとき一年中同じ(なので春のデータだけ見ればよい)
  AREA_SEASON_DATA   season_data[PMSEASON_TOTAL];  // PMSEASON_SPRING PMSEASON_SUMMER PMSEASON_AUTUMN PMSEASON_WINTER
}
AREA_DATA;

//-------------------------------------
/// 隠しスポット
//=====================================
typedef enum
{
  HIDE_STATE_NO,          // 隠しスポットではない
  HIDE_STATE_HIDE_TRUE,   // 隠しスポットで隠し中
  HIDE_STATE_HIDE_FALSE,  // 隠しスポットで表示中
}
HIDE_STATE;

typedef struct
{
  HIDE_STATE    state;
  GFL_CLWK*     obj_clwk;  // stateがHIDE_STATE_NOまたはHIDE_STATE_HIDE_TRUEのときはNULL
}
HIDE_SPOT;


//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // 他のところから借用するもの
  GFL_CLUNIT*                 clunit;
  GFL_FONT*                   font;
  //PRINT_QUE*                  print_que;  // 専用のprint_queを用意することにした

  // ここで作成するもの
  ZKNDTL_COMMON_REAR_WORK*    rear_wk_s;

  // メッセージとテキスト
  GFL_MSGDATA*                msgdata[MSG_MAX];
  GFL_BMPWIN*                 text_bmpwin[TEXT_MAX];
  BOOL                        text_trans[TEXT_MAX];  // 転送する必要がある場合TRUE
  PRINT_QUE*                  print_que[TEXT_MAX];
  GFL_BMPWIN*                 text_dummy_bmpwin[TEXT_DUMMY_MAX];

  // 上画面
  u32                         pokeicon_ncg[OBJ_SWAP_MAX];    // pokeicon_clwk[i]がNULLでないときpokeicon_ncg[i]は有効
  u32                         pokeicon_ncl;
  u32                         pokeicon_nce;
  GFL_CLWK*                   pokeicon_clwk[OBJ_SWAP_MAX];   // 使っていないときはpokeicon_clwk[i]をNULLにしておく
  OBJ_SWAP                    pokeicon_swap_curr;            // 今表示しているpokeicon_clwkはpokeicon_clwk[pokeicon_swap_curr]
  
  GFL_ARCUTIL_TRANSINFO       place_tinfo;

  // 状態
  STATE                       state;
  SCROLL_STATE                scroll_state;
  SCROLL_STATE                pokeicon_scroll_state;
  u8                          season_id;  // PMSEASON_SPRING PMSEASON_SUMMER PMSEASON_AUTUMN PMSEASON_WINTER
  u8                          select_data_idx;  // 街もダンジョンも道路も選んでいないときDATA_IDX_NONE

  int                         top_to_select_ktst;  // STATE_TOP_TO_SELECTにしたのはGFL_APP_KTST_TOUCHかGFL_APP_KTST_KEYか覚えておく。
  u8                          top_to_select_x;     // top_to_select_ktst==GFL_APP_KTST_TOUCHのときのみ有効。
  u8                          top_to_select_y;     // タッチ入力でSTATE_TOP_TO_SELECTとなったときのタッチ座標x,yを覚えておく。

  DRAW_SEASON_AREA_STEP       draw_season_area_step;  // STATE_TOP_DRAW_SEASON_AREAとSTATE_TOP_BEFORE_FADE_OUTとSTATE_SELECT_DRAW_SEASON_AREAのときのみ有効。
  BOOL                        draw_season_area_set_user_active_whole;  // TRUEのとき、STATE_TOP_DRAW_SEASON_AREAとSTATE_TOP_BEFORE_FADE_OUTとSTATE_SELECT_DRAW_SEASON_AREAから変更される際に、ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWholeを呼ぶ必要あり。

  // 各ポケモンの情報
  APPEAR_RULE                 appear_rule;
  HABITAT                     habitat;
  AREA_DATA*                  area_data;

  // OBJ
  u32                         obj_res[OBJ_RES_MAX];
  GFL_CLWK*                   obj_clwk[OBJ_MAX];
  u8                          push_button;  // 現在押されているボタン。OBJ_SEASON_LかOBJ_SEASON_RかOBJ_PUSH_BUTTON_NONEのどれか。
  GFL_CLWK*                   obj_exist_clwk[TOWNMAP_DATA_MAX];
  GFL_CLUNIT*                 usual_clunit;  // 不必要だったかも、途中でテストを止めたので最終結論までは出していない。
  GFL_CLSYS_REND*             usual_rend;    // レンダラー全体に対して半透明設定が反映されると思っていてつくったもの。
  int                         obj_exist_ev1;
  u16                         obj_exist_alpha_anime_count;

  // 隠しスポット
  HIDE_SPOT                   hide_spot[TOWNMAP_DATA_MAX];

  // タウンマップデータ
  TOWNMAP_DATA*               townmap_data;

  // タウンマップデータのインデックスからzkn_area_monsno_???.dat中のインデックスを得る配列
  u8 townmap_data_idx_to_zkn_area_idx[TOWNMAP_DATA_MAX];
  // zukan_area_?.narcのハンドルを保持しておく
  ARCHANDLE* area_handle;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;
  u8                          vblank_req;

  // フェード
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_m;  // MAIN画面はこちらを使用
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_s;
  // パレットフェード
  ZKNDTL_COMMON_PF_WORK*      pf_wk;      // SUB画面はこちらを使用

  // 終了命令
  END_CMD                     end_cmd;

  // 入力可不可
  BOOL                        input_enable;  // 入力可のときTRUE

#ifdef DEF_SCMD_CHANGE
  // タッチバーの上下アイコンでポケモンを変更するときだけ使用する変数
  BOOL                        change_poke_by_cur_u_d_flag;                // TRUEのとき、タッチバーの上下アイコンでポケモンを変更している
  BOOL                        change_poke_by_cur_u_d_change_poke_finish;  // ポケモンの変更が完了していたらTRUE
  BOOL                        change_poke_by_cur_u_d_cur_u_d_finish;      // タッチバーの上下アイコンのアニメが完了していたらTRUE
#endif
}
ZUKAN_DETAIL_MAP_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Zukan_Detail_Map_VBlankFunc( GFL_TCB* tcb, void* wk );

// タウンマップデータのインデックスからzkn_area_monsno_???.dat中のインデックスを得る配列を作成する
static void Zukan_Detail_Map_TownmapDataIdxToZknAreaIdxArrayMake( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// アフィン拡張BG
static void Zukan_Detail_Map_AffineExBGInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_AffineExBGExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// OBJ
static void Zukan_Detail_Map_ObjInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_ObjExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_ObjExistInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_ObjExistExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
// OBJ_UNKNOWNを表示するためのアルファ設定
static void Zukan_Detail_Map_ObjUnknownAlphaInit( ZUKAN_DETAIL_MAP_WORK* work );
static void Zukan_Detail_Map_ObjUnknownAlphaExit( ZUKAN_DETAIL_MAP_WORK* work );
// obj_existのアルファアニメーション
static void Zukan_Detail_Map_ObjExistAlphaInit( ZUKAN_DETAIL_MAP_WORK* work );
static void Zukan_Detail_Map_ObjExistAlphaExit( ZUKAN_DETAIL_MAP_WORK* work );
static void Zukan_Detail_Map_ObjExistAlphaMain( ZUKAN_DETAIL_MAP_WORK* work );
static void Zukan_Detail_Map_ObjExistAlphaReset( ZUKAN_DETAIL_MAP_WORK* work );

// 隠しスポット
static void Zukan_Detail_Map_HideSpotInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_HideSpotExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// メッセージとテキスト
static void Zukan_Detail_Map_PrintQueTextInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_PrintQueTextExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_PrintQueMain( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_TextMain( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn, u8 i );

static void Zukan_Detail_Map_MsgTextInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_MsgTextExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 上画面
static void Zukan_Detail_Map_PlaceInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_PlaceExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_PlaceMain( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_PlaceMainScroll( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_PlaceMainIcon( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 下画面
static void Zukan_Detail_Map_SeasonArrowMain( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// ポケアイコン
static void PokeiconBaseInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void PokeiconBaseExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static GFL_CLWK* PokeiconInit( u32* ncg, u32 ncl, u32 nce, GFL_CLUNIT* clunit, HEAPID heap_id, u32 mons, u32 form_no, u32 sex, BOOL egg, u8 x, u8 y );
static void PokeiconExit( u32 ncg, GFL_CLWK* clwk );
// マクロ
#define BLOCK_POKEICON_EXIT(ncg,clwk)                     \
    {                                                     \
      if( clwk ) PokeiconExit( ncg, clwk );               \
      clwk = NULL;                                        \
    }
// NULLを代入し忘れないようにマクロを使うようにしておく

// プレイヤーの位置のデータインデックスを得る
static u8 Zukan_Detail_Map_GetPlayerPosDataIdx( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
// プレイヤーの位置にOBJを配置する
static void Zukan_Detail_Map_SetPlayer( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
// プレイヤーの位置に配置するOBJの表示/非表示を切り替える
static void Zukan_Detail_Map_DrawPlayer( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                         BOOL on_off );
// プレイヤーの位置に配置するOBJを現在表示しているか否か
static BOOL Zukan_Detail_Map_IsDrawPlayer( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// カーソルを配置する
static void Zukan_Detail_Map_SetCursor( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
// プレイヤーの位置にカーソルを配置する
static void Zukan_Detail_Map_SetCursorOnPlayer( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 指定した座標とヒットするデータがあればそのデータインデックスを返す
static u8 Zukan_Detail_Map_Hit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                u8 x, u8 y );
// 指定した座標は吸い込み範囲内か
static BOOL Zukan_Detail_Map_IsPullHit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                        u8 data_idx, u8 x, u8 y, u32* distance_sq );
// 指定した座標を吸い込み範囲内に持つデータがあればそのデータインデックスを返す
static u8 Zukan_Detail_Map_PullHit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                u8 x, u8 y );

// 操作
static void Zukan_Detail_Map_Input( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
// マップ上の拠点の選択操作
static void Zukan_Detail_Map_InputSelect( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 状態を遷移させる
static void Zukan_Detail_Map_StateMain( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_ChangeState( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                 STATE state );
static void Zukan_Detail_Map_ChangePoke( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_DrawSeasonAreaAfterChangePoke( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_OffSeasonAreaBeforeFadeOut( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_ChangePlace( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_ChangeSeason( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn, BOOL b_next, int ktst );

// 分布データ
static AREA_DATA* Zukan_Detail_Map_AreaDataLoad( u16 monsno, HEAPID heap_id, ARCHANDLE* handle );
static void Zukan_Detail_Map_AreaDataUnload( AREA_DATA* area_data );

// 何度も出てくる処理
static void Zukan_Detail_Map_UtilPrintSeason( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_UtilDrawSeasonArea( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_UtilBrightenPlaceIcon( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

static void Zukan_Detail_Map_UtilDrawSeasonAreaObjOff( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_UtilDrawSeasonAreaAlphaSet( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_UtilDrawSeasonAreaObjOn( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_UtilDrawSeasonAreaObjOffCompleteOff( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_UtilDrawSeasonAreaAlphaSetCompleteOff( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_UtilDrawSeasonAreaObjOnCompleteOff( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// アルファ設定
static void Zukan_Detail_Map_AlphaMainInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_AlphaMainExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_AlphaSubInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_AlphaSubExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Map_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Map_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Map_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd );

const ZKNDTL_PROC_DATA    ZUKAN_DETAIL_MAP_ProcData =
{
  Zukan_Detail_Map_ProcInit,
  Zukan_Detail_Map_ProcMain,
  Zukan_Detail_Map_ProcExit,
  Zukan_Detail_Map_CommandFunc,
  NULL,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief           
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void             ZUKAN_DETAIL_MAP_InitParam(
                            ZUKAN_DETAIL_MAP_PARAM*  param,
                            HEAPID                    heap_id )
{
  param->heap_id = heap_id;
}


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Map_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_MAP_PARAM*    param    = (ZUKAN_DETAIL_MAP_PARAM*)pwk;
  ZUKAN_DETAIL_MAP_WORK*     work;

  const GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Map_ProcInit\n" );
  }
#endif

  // オーバーレイ
  GFL_OVERLAY_Load( FS_OVERLAY_ID(townmap) );

  // ヒープ
  {
    work = ZKNDTL_PROC_AllocWork(proc, sizeof(ZUKAN_DETAIL_MAP_WORK), param->heap_id);
    GFL_STD_MemClear( work, sizeof(ZUKAN_DETAIL_MAP_WORK) ); 
  }

  // 他のところから借用するもの
  {
    ZUKAN_DETAIL_GRAPHIC_WORK* graphic = ZKNDTL_COMMON_GetGraphic(cmn);
    work->clunit      = ZUKAN_DETAIL_GRAPHIC_GetClunit(graphic);
    work->font        = ZKNDTL_COMMON_GetFont(cmn);
    //work->print_que   = ZKNDTL_COMMON_GetPrintQue(cmn);  // 専用のprint_queを用意することにした
  }

  // 初期化
  {
    u8 i;
    for( i=0; i<OBJ_SWAP_MAX; i++ )
    {
      work->pokeicon_clwk[i] = NULL;  // NULLで初期化
    }
    work->pokeicon_swap_curr = OBJ_SWAP_0;
  }

  // 各ポケモンの情報を何も表示がされていない状態になるように初期化
  {
    work->appear_rule   = APPEAR_RULE_YEAR;  // 季節が表示されていないように
    work->habitat       = HABITAT_KNOWN;     // 生息地不明が表示されていないように
    work->area_data     = NULL;
  }

  // 状態初期化
  {
    work->state                    = STATE_TOP;
    work->scroll_state             = SCROLL_STATE_DOWN;
    work->pokeicon_scroll_state    = SCROLL_STATE_DOWN;
    work->season_id                = GAMEDATA_GetSeasonID(gamedata);
    work->select_data_idx          = DATA_IDX_NONE;

    work->draw_season_area_step                  = DRAW_SEASON_AREA_STEP_NONE;
    work->draw_season_area_set_user_active_whole = TRUE;
  }

  // タウンマップデータ
  work->townmap_data = TOWNMAP_DATA_Alloc( param->heap_id );

  // タウンマップデータのインデックスからzkn_area_monsno_???.dat中のインデックスを得る配列を作成する
  Zukan_Detail_Map_TownmapDataIdxToZknAreaIdxArrayMake( param, work, cmn );
  // zukan_area_?.narcのハンドルを保持しておく
  work->area_handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_AREA, param->heap_id );  // arc_tool.lstによって、バージョンによってARCID_ZUKAN_AREAで読み込むファイルが変わるようになっている。

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zukan_Detail_Map_VBlankFunc, work, 1 );
  work->vblank_req = VBLANK_REQ_NONE;

  // print_queとテキスト
  Zukan_Detail_Map_PrintQueTextInit( param, work, cmn );
  
  // フェード
  {
    work->fade_wk_m = ZKNDTL_COMMON_FadeInit( param->heap_id );
    work->fade_wk_s = ZKNDTL_COMMON_FadeInit( param->heap_id );
   
    ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_m );
    ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_s );

    ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_M, work->fade_wk_m );
    ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
  }
  // パレットフェード
  {
    work->pf_wk = ZKNDTL_COMMON_PfInitPart( param->heap_id, PF_BIT_SUB_BG | PF_BIT_SUB_OBJ );
  }

  // 終了情報
  work->end_cmd = END_CMD_NONE;

  // 入力可不可
  work->input_enable = TRUE;

#ifdef DEF_SCMD_CHANGE
  // タッチバーの上下アイコンでポケモンを変更するときだけ使用する変数
  work->change_poke_by_cur_u_d_flag                = FALSE;
  work->change_poke_by_cur_u_d_change_poke_finish  = TRUE;
  work->change_poke_by_cur_u_d_cur_u_d_finish      = TRUE;
#endif

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Map_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_MAP_PARAM*    param    = (ZUKAN_DETAIL_MAP_PARAM*)pwk;
  ZUKAN_DETAIL_MAP_WORK*     work     = (ZUKAN_DETAIL_MAP_WORK*)mywk;

  // 分布データ
  Zukan_Detail_Map_AreaDataUnload( work->area_data );

  // ポケアイコン
  {
    u8 i;
    for( i=0; i<OBJ_SWAP_MAX; i++ )
    {
      // NULLチェックはマクロ内で行っている
      BLOCK_POKEICON_EXIT( work->pokeicon_ncg[i], work->pokeicon_clwk[i] )
    }
  }

  // ポケアイコンの不変物
  PokeiconBaseExit( param, work, cmn );

  // 上画面
  Zukan_Detail_Map_PlaceExit( param, work, cmn );
  // メッセージとテキスト 
  Zukan_Detail_Map_MsgTextExit( param, work, cmn );
 
  // 最背面
  ZKNDTL_COMMON_RearExit( work->rear_wk_s );

  // OBJ
  Zukan_Detail_Map_HideSpotExit( param, work, cmn );
  Zukan_Detail_Map_ObjExistExit( param, work, cmn );
  Zukan_Detail_Map_ObjExit( param, work, cmn );

  // パレットフェード
  {
    ZKNDTL_COMMON_PfExit( work->pf_wk );
  }
  // フェード
  {
    ZKNDTL_COMMON_FadeExit( work->fade_wk_s );
    ZKNDTL_COMMON_FadeExit( work->fade_wk_m );
  }

  // print_queとテキスト
  Zukan_Detail_Map_PrintQueTextExit( param, work, cmn );

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // 保持していたzukan_area_?.narcのハンドル
  GFL_ARC_CloseDataHandle( work->area_handle );

  // タウンマップデータ
  TOWNMAP_DATA_Free( work->townmap_data );

  // ヒープ
  ZKNDTL_PROC_FreeWork( proc );

  // オーバーレイ
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(townmap) );

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Map_ProcExit\n" );
  }
#endif

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Map_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_MAP_PARAM*    param    = (ZUKAN_DETAIL_MAP_PARAM*)pwk;
  ZUKAN_DETAIL_MAP_WORK*     work     = (ZUKAN_DETAIL_MAP_WORK*)mywk;

  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  ZUKAN_DETAIL_HEADBAR_WORK*   headbar  = ZKNDTL_COMMON_GetHeadbar(cmn);
 
  switch(*seq)
  {
  case SEQ_START:
    {
      *seq = SEQ_FADE_CHANGE_BEFORE;

      // 切り替え
      {
        // グラフィックスモード設定
        GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_5, GX_BG0_AS_2D );
        
        // BG面設定
        {	
          int i;
          for( i = 0; i < NELEMS(sc_bgsetup_affineex); i++ )
          {	
            GFL_BG_FreeBGControl( sc_bgsetup_affineex[i].frame );
            
            GFL_BG_SetBGControl( sc_bgsetup_affineex[i].frame, &sc_bgsetup_affineex[i].bgcnt_header, sc_bgsetup_affineex[i].mode );
            GFL_BG_ClearFrame( sc_bgsetup_affineex[i].frame );
            GFL_BG_SetVisible( sc_bgsetup_affineex[i].frame, sc_bgsetup_affineex[i].is_visible );
          }
        }
      }

      // BG
      {
        // クリア
        u8 i;
        for( i=GFL_BG_FRAME0_M; i<=GFL_BG_FRAME3_S; i++ )
        {
          if(    i != ZKNDTL_BG_FRAME_M_TOUCHBAR
              && i != ZKNDTL_BG_FRAME_S_HEADBAR )
          {
            GFL_BG_SetScroll( i, GFL_BG_SCROLL_X_SET, 0 );
            GFL_BG_SetScroll( i, GFL_BG_SCROLL_Y_SET, 0 );
			      GFL_BG_ClearFrame(i);
          }
        }

        // メインBG
        GFL_BG_SetPriority( BG_FRAME_M_ROOT, BG_FRAME_PRI_M_ROOT );
        GFL_BG_SetPriority( BG_FRAME_M_MAP,  BG_FRAME_PRI_M_MAP );
        GFL_BG_SetPriority( BG_FRAME_M_TEXT, BG_FRAME_PRI_M_TEXT );
        
        // サブBG
        GFL_BG_SetPriority( BG_FRAME_S_TEXT,  BG_FRAME_PRI_S_TEXT );
        GFL_BG_SetPriority( BG_FRAME_S_PLACE, BG_FRAME_PRI_S_PLACE );
        GFL_BG_SetPriority( BG_FRAME_S_REAR,  BG_FRAME_PRI_S_REAR );
      }

      // アフィン拡張BG
      Zukan_Detail_Map_AffineExBGInit( param, work, cmn );
      // OBJ
      Zukan_Detail_Map_ObjInit( param, work, cmn );
      Zukan_Detail_Map_ObjExistInit( param, work, cmn );
      Zukan_Detail_Map_HideSpotInit( param, work, cmn );
      
      // 最背面
      work->rear_wk_s = ZKNDTL_COMMON_RearInit( param->heap_id, ZKNDTL_COMMON_REAR_TYPE_MAP,
          BG_FRAME_S_REAR, BG_PAL_POS_S_REAR +0, BG_PAL_POS_S_REAR +1 );

      // メッセージとテキスト 
      Zukan_Detail_Map_MsgTextInit( param, work, cmn );
      // 上画面
      Zukan_Detail_Map_PlaceInit( param, work, cmn );

      // ポケアイコンの不変物
      PokeiconBaseInit( param, work, cmn );

      // この画面に来たときに選ばれていたポケモンを表示する
      Zukan_Detail_Map_ChangePoke( param, work, cmn );
      // ポケアイコンの設定を変更しておく
      GFL_CLACT_WK_SetObjMode( work->pokeicon_clwk[work->pokeicon_swap_curr], GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので

      // プレイヤーの位置にOBJを配置する
      Zukan_Detail_Map_SetPlayer( param, work, cmn );
      // プレイヤーの位置にOBJを表示
      if( work->habitat == HABITAT_KNOWN )  // 生息地が判明しているときは、TOPでもSELECTでも表示しておく。
      {
        Zukan_Detail_Map_DrawPlayer( param, work, cmn, TRUE );
      }

      // パレットフェード
      {
        ZKNDTL_COMMON_PfSetPaletteDataFromVram( work->pf_wk );
        ZKNDTL_COMMON_PfSetBlackImmediately( work->pf_wk );
      }
    }
    break;
  case SEQ_FADE_CHANGE_BEFORE:
    {
      *seq = SEQ_PREPARE;

      // フェード
      ZKNDTL_COMMON_FadeSetColorlessImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
     
      // ZKNDTL_COMMON_FadeSetColorlessImmediatelyでG2_SetBlendBrightnessExtやG2S_SetBlendBrightnessExtを設定しているので、
      // その後でZukan_Detail_Map_AlphaInitを呼ぶこと
      //Zukan_Detail_Map_AlphaMainInit( param, work, cmn );
      Zukan_Detail_Map_AlphaSubInit( param, work, cmn );
    }
    break;
  case SEQ_PREPARE:
    {
      *seq = SEQ_FADE_IN;

      // フェード
      ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_m );
      //ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_s );

      // パレットフェード
      {
        ZKNDTL_COMMON_PfSetBlackToColorless( work->pf_wk );
      }

      // タッチバー
      if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) != ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_TOUCHBAR_SetType(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
            ZUKAN_DETAIL_TOUCHBAR_DISP_MAP,
            (ZKNDTL_COMMON_GetPokeNum(cmn)>1)?TRUE:FALSE ); 
        ZUKAN_DETAIL_TOUCHBAR_Appear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
      }
      else
      {
        ZUKAN_DETAIL_TOUCHBAR_SetDispOfGeneral(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_DISP_MAP );
      }
      ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, FALSE );  // ZUKAN_DETAIL_TOUCHBAR_SetTypeのときはUnlock状態なので
      work->draw_season_area_set_user_active_whole = FALSE;
      {
        GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
        ZUKAN_DETAIL_TOUCHBAR_SetCheckFlipOfGeneral(
            touchbar,
            GAMEDATA_GetShortCut( gamedata, SHORTCUT_ID_ZUKAN_MAP ) );
      }
      // タイトルバー
      if( ZUKAN_DETAIL_HEADBAR_GetState( headbar ) != ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_HEADBAR_SetType(
            headbar,
            ZUKAN_DETAIL_HEADBAR_TYPE_MAP );
        ZUKAN_DETAIL_HEADBAR_Appear( headbar );
      }
    }
    break;
  case SEQ_FADE_IN:
    {
      if(    (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          //&& (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_s ))
          && (!ZKNDTL_COMMON_PfIsExecute( work->pf_wk ))
          && ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
        //1フレーム待つことにしたZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, TRUE );  // ZUKAN_DETAIL_TOUCHBAR_SetTypeのときはUnlock状態なので

        //Zukan_Detail_Map_ObjExistAlphaInit( param, work, cmn );
        //Zukan_Detail_Map_AlphaInit( param, work, cmn );

        Zukan_Detail_Map_AlphaMainInit( param, work, cmn );
        
        Zukan_Detail_Map_DrawSeasonAreaAfterChangePoke( param, work, cmn );

        *seq = SEQ_BEFORE_MAIN;
      }
    }
    break;
  case SEQ_BEFORE_MAIN:
    {
      //if( work->vblank_req == VBLANK_REQ_NONE )
      if( work->draw_season_area_step == DRAW_SEASON_AREA_STEP_NONE )
      {
        ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, TRUE );  // ZUKAN_DETAIL_TOUCHBAR_SetTypeのときはUnlock状態なので
        work->draw_season_area_set_user_active_whole = TRUE;
        
        *seq = SEQ_MAIN;
      }
    }
    break;
  case SEQ_MAIN:
    {
      if( work->end_cmd != END_CMD_NONE )
      {
        Zukan_Detail_Map_OffSeasonAreaBeforeFadeOut( param, work, cmn );

        //Zukan_Detail_Map_AlphaExit( param, work, cmn );
        //Zukan_Detail_Map_ObjExistAlphaExit( param, work, cmn );

        *seq = SEQ_AFTER_MAIN;

/*
1フレーム待つことにした
        // フェード
        ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_m );
        //ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_s );
       
        // パレットフェード
        {
          ZKNDTL_COMMON_PfSetColorlessToBlack( work->pf_wk );
        }

        // タイトルバー
        ZUKAN_DETAIL_HEADBAR_Disappear( headbar );
        
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_Disappear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
        }
*/
      }
      else
      {
        if( work->habitat == HABITAT_KNOWN )  // 生息地が判明しているとき
          Zukan_Detail_Map_ObjExistAlphaMain( work );

        // 操作
        Zukan_Detail_Map_Input( param, work, cmn );
      }
    }
    break;
  case SEQ_AFTER_MAIN:
    {
      //if( work->vblank_req == VBLANK_REQ_NONE )
      if( work->draw_season_area_step == DRAW_SEASON_AREA_STEP_NONE )
      {
        Zukan_Detail_Map_AlphaMainExit( param, work, cmn );
        
        *seq = SEQ_FADE_OUT;

        // フェード
        ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_m );
        //ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_s );
       
        // パレットフェード
        {
          ZKNDTL_COMMON_PfSetColorlessToBlack( work->pf_wk );
        }

        // タイトルバー
        ZUKAN_DETAIL_HEADBAR_Disappear( headbar );
        
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_Disappear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
        }
      }
    }
    break;
  case SEQ_FADE_OUT:
    {
      BOOL b_next_seq = FALSE;

      if(    (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          //&& (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_s ))
          && (!ZKNDTL_COMMON_PfIsExecute( work->pf_wk ))
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_DISAPPEAR )
      {
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR )
          {
            b_next_seq = TRUE;
          }
        }
        else
        {
          b_next_seq = TRUE;
        }
      }

      if( b_next_seq )
      {
        *seq = SEQ_FADE_CHANGE_AFTER;
      }
    }
    break;
  case SEQ_FADE_CHANGE_AFTER:
    {
      *seq = SEQ_END;

      // ZKNDTL_COMMON_FadeSetColorlessImmediatelyでG2_SetBlendBrightnessExtやG2S_SetBlendBrightnessExtを設定しているので、
      // その前にZukan_Detail_Map_AlphaExitを呼ぶこと
      //Zukan_Detail_Map_AlphaMainExit( param, work, cmn );
      Zukan_Detail_Map_AlphaSubExit( param, work, cmn );
      
      // フェード
      ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
    }
    break;
  case SEQ_END:
    {
      // アフィン拡張BG
      Zukan_Detail_Map_AffineExBGExit( param, work, cmn );
      
      // 切り替え
      {
        // グラフィックスモード設定
        GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D );

        // BG面設定
        {	
          int i;
          for( i = 0; i < NELEMS(sc_bgsetup_text); i++ )
          {	
            GFL_BG_FreeBGControl( sc_bgsetup_text[i].frame );
            
            GFL_BG_SetBGControl( sc_bgsetup_text[i].frame, &sc_bgsetup_text[i].bgcnt_header, sc_bgsetup_text[i].mode );
            GFL_BG_ClearFrame( sc_bgsetup_text[i].frame );
            GFL_BG_SetVisible( sc_bgsetup_text[i].frame, sc_bgsetup_text[i].is_visible );
          }
        }
      }

      return ZKNDTL_PROC_RES_FINISH;
    }
    break;
  }

  if( *seq >= SEQ_PREPARE )
  {
    // 最背面
    ZKNDTL_COMMON_RearMain( work->rear_wk_s );
    // 上画面
    Zukan_Detail_Map_PlaceMain( param, work, cmn );
    // 下画面
    Zukan_Detail_Map_SeasonArrowMain( param, work, cmn );
  }

  // 状態
  Zukan_Detail_Map_StateMain( param, work, cmn );

  // フェード
  ZKNDTL_COMMON_FadeMain( work->fade_wk_m, work->fade_wk_s );
  // パレットフェード
  {
    ZKNDTL_COMMON_PfMain( work->pf_wk );
  }

  // print_que
  Zukan_Detail_Map_PrintQueMain( param, work, cmn );


#ifdef DEBUG_SET_ANIME_AND_POS
  {
    // 生息する場所に置くOBJのアニメ指定、位置指定
    if(    *seq >= SEQ_PREPARE
        && work->state == STATE_SELECT )
    {
      do
      {
        static u8 idx = 0;
      
        // 現在 
        u16 anmseq = GFL_CLACT_WK_GetAnmSeq( work->obj_exist_clwk[idx] );
        u16 anmseq_num = GFL_CLACT_WK_GetAnmSeqNum( work->obj_exist_clwk[idx] );
        GFL_CLACTPOS pos;
        GFL_CLACT_WK_GetPos( work->obj_exist_clwk[idx], &pos, CLSYS_DEFREND_MAIN );
      
        // 情報表示
        if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
        {
          GFL_CLACT_WK_GetPos( work->obj_exist_clwk[idx], &pos, CLSYS_DEFREND_MAIN );
          OS_Printf( "idx=%d, zkn_anm=%d, zkn_pos_x=%d, zkn_pos_y=%d\n",
              idx,
              anmseq, 
              pos.x,
              pos.y
          );
        }
        // idx切り替え
        else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R )  // Up
        {
          idx++;
          if( idx >= TOWNMAP_DATA_MAX )
            idx = 0;
          break;  // idxを変更したので終了
        }
        else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L )  // Down
        {
          if( idx == 0 )
            idx = TOWNMAP_DATA_MAX -1;
          else
            idx--;
          break;  // idxを変更したので終了
        }
        // アニメ切り替え
        else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )  // Up
        {
          anmseq++;
          if( anmseq >= anmseq_num )
            anmseq = 0;
        }
        else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )  // Down
        {
          if( anmseq == 0 )
            anmseq = anmseq_num -1;
          else
            anmseq--;
        }
        // 移動
        else
        {
          if( GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT )
          {
            if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
              pos.x++;
            if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
              pos.x--;
            if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )
              pos.y--;
            if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
              pos.y++;
          }
          else
          {
            if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
              pos.x++;
            if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
              pos.x--;
            if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )
              pos.y--;
            if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )
              pos.y++;
          }
        }

        // 次
        GFL_CLACT_WK_SetAnmSeq( work->obj_exist_clwk[idx], anmseq );
        GFL_CLACT_WK_SetPos( work->obj_exist_clwk[idx], &pos, CLSYS_DEFREND_MAIN );
      }
      while(0);
    }
  }
#endif


  return ZKNDTL_PROC_RES_CONTINUE;
}

//-------------------------------------
/// PROC 命令処理
//=====================================
static void Zukan_Detail_Map_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd )
{
  if( mywk )
  {
    ZUKAN_DETAIL_MAP_PARAM*    param    = (ZUKAN_DETAIL_MAP_PARAM*)pwk;
    ZUKAN_DETAIL_MAP_WORK*     work     = (ZUKAN_DETAIL_MAP_WORK*)mywk;

    ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
 
    BOOL b_valid_cmd = FALSE;  // cmdがZKNDTL_CMD_NONE以外のZKNDTL_CMD_???のときTRUE。ZKNDTL_CMD_NONEやZKNDTL_SCMD_???のときFALSE。

    // 入力不可
    switch( cmd )
    {
    case ZKNDTL_SCMD_CLOSE:
    case ZKNDTL_SCMD_RETURN:
    case ZKNDTL_SCMD_INFO:
    case ZKNDTL_SCMD_VOICE:
    case ZKNDTL_SCMD_FORM:
    case ZKNDTL_SCMD_CUR_D:
    case ZKNDTL_SCMD_CUR_U:
    case ZKNDTL_SCMD_CHECK:
    case ZKNDTL_SCMD_MAP_RETURN:
      {
        work->input_enable = FALSE;
      }
      break;
    }
    // 入力可
    switch( cmd )
    {
    case ZKNDTL_CMD_CLOSE:
    case ZKNDTL_CMD_RETURN:
    case ZKNDTL_CMD_INFO:
    case ZKNDTL_CMD_VOICE:
    case ZKNDTL_CMD_FORM:
    case ZKNDTL_CMD_CUR_D:
    case ZKNDTL_CMD_CUR_U:
    case ZKNDTL_CMD_CHECK:
    case ZKNDTL_CMD_MAP_RETURN:
      {
        work->input_enable = TRUE;
        b_valid_cmd = TRUE;
      }
      break;
    }


#ifdef DEF_SCMD_CHANGE
    switch( cmd )
    {
    case ZKNDTL_SCMD_CUR_D:
      {
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToNextPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          Zukan_Detail_Map_ChangePoke( param, work, cmn );
          Zukan_Detail_Map_DrawSeasonAreaAfterChangePoke( param, work, cmn );

          // タッチバーの上下アイコンでポケモンを変更するときだけ使用する変数
          work->change_poke_by_cur_u_d_flag                = TRUE;
          work->change_poke_by_cur_u_d_change_poke_finish  = FALSE;
          work->change_poke_by_cur_u_d_cur_u_d_finish      = FALSE;
        }
        else
        {
          // タッチバーの上下アイコンでポケモンを変更するときだけ使用する変数
          work->change_poke_by_cur_u_d_flag                = TRUE;
          work->change_poke_by_cur_u_d_change_poke_finish  = TRUE;    // ポケモンを変更しないので
          work->change_poke_by_cur_u_d_cur_u_d_finish      = FALSE;
        }
      }
      break;
    case ZKNDTL_SCMD_CUR_U:
      {
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToPrevPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          Zukan_Detail_Map_ChangePoke( param, work, cmn );
          Zukan_Detail_Map_DrawSeasonAreaAfterChangePoke( param, work, cmn );

          // タッチバーの上下アイコンでポケモンを変更するときだけ使用する変数
          work->change_poke_by_cur_u_d_flag                = TRUE;
          work->change_poke_by_cur_u_d_change_poke_finish  = FALSE;
          work->change_poke_by_cur_u_d_cur_u_d_finish      = FALSE;
        }
        else
        {
          // タッチバーの上下アイコンでポケモンを変更するときだけ使用する変数
          work->change_poke_by_cur_u_d_flag                = TRUE;
          work->change_poke_by_cur_u_d_change_poke_finish  = TRUE;    // ポケモンを変更しないので
          work->change_poke_by_cur_u_d_cur_u_d_finish      = FALSE;
        }
      }
      break;
    }
#endif


    // コマンド
    switch( cmd )
    {
    case ZKNDTL_CMD_NONE:
      {
      }
      break;
    case ZKNDTL_CMD_CLOSE:
    case ZKNDTL_CMD_RETURN:
      {
        work->end_cmd = END_CMD_OUTSIDE;
      }
      break;
    case ZKNDTL_CMD_INFO:
    case ZKNDTL_CMD_VOICE:
    case ZKNDTL_CMD_FORM:
      {
        work->end_cmd = END_CMD_INSIDE;
      }
      break;
    case ZKNDTL_CMD_CUR_D:
      {
#ifndef DEF_SCMD_CHANGE
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToNextPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          Zukan_Detail_Map_ChangePoke( param, work, cmn );
          Zukan_Detail_Map_DrawSeasonAreaAfterChangePoke( param, work, cmn );
        }
        //ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
#endif

#ifdef DEF_SCMD_CHANGE
        if( work->change_poke_by_cur_u_d_flag )
        {
          work->change_poke_by_cur_u_d_cur_u_d_finish = TRUE;
          if( work->change_poke_by_cur_u_d_change_poke_finish )
          {
            work->change_poke_by_cur_u_d_flag = FALSE;
            ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
          }
        }
#endif 
      }
      break;
    case ZKNDTL_CMD_CUR_U:
      {
#ifndef DEF_SCMD_CHANGE
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToPrevPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          Zukan_Detail_Map_ChangePoke( param, work, cmn );
          Zukan_Detail_Map_DrawSeasonAreaAfterChangePoke( param, work, cmn );
        }
        //ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
#endif

#ifdef DEF_SCMD_CHANGE
        if( work->change_poke_by_cur_u_d_flag )
        {
          work->change_poke_by_cur_u_d_cur_u_d_finish = TRUE;
          if( work->change_poke_by_cur_u_d_change_poke_finish )
          {
            work->change_poke_by_cur_u_d_flag = FALSE;
            ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
          }
        }
#endif 
      }
      break;
    case ZKNDTL_CMD_CHECK:
      {
        GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
        GAMEDATA_SetShortCut(
            gamedata,
            SHORTCUT_ID_ZUKAN_MAP,
            ZUKAN_DETAIL_TOUCHBAR_GetCheckFlipOfGeneral( touchbar ) );
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_MAP_RETURN:
      {
        Zukan_Detail_Map_ChangeState( param, work, cmn, STATE_SELECT_TO_TOP );
      }
      break;
    default:
      {
        if( b_valid_cmd )
        {
          ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
        }
      }
      break;
    }
  }
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Zukan_Detail_Map_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  ZUKAN_DETAIL_MAP_WORK*       work     = (ZUKAN_DETAIL_MAP_WORK*)wk;

  switch( work->vblank_req )
  {
/*
  case VBLANK_REQ_OBJ_EXIST_ALPHA_EXIT:
    {
      Zukan_Detail_Map_ObjExistAlphaExit( work );
    }
    break;
  case VBLANK_REQ_OBJ_UNKNOWN_ALPHA_EXIT:
    {
      Zukan_Detail_Map_ObjUnknownAlphaExit( work );
    }
    break;
*/
  case VBLANK_REQ_OBJ_EXIST_ALPHA_EXIT_AND_OBJ_UNKNOWN_ALPHA_INIT:
    {
      Zukan_Detail_Map_ObjExistAlphaExit( work );
      Zukan_Detail_Map_ObjUnknownAlphaInit( work );
    }
    break;
  case VBLANK_REQ_OBJ_UNKNOWN_ALPHA_EXIT_AND_OBJ_EXIST_ALPHA_INIT:
    {
      Zukan_Detail_Map_ObjUnknownAlphaExit( work );
      Zukan_Detail_Map_ObjExistAlphaInit( work );
    }
    break;
  case VBLANK_REQ_OBJ_ALL_ALPHA_EXIT_AND_FADE_SET_COLORLESS_IMMEDIATELY:
    {
      Zukan_Detail_Map_ObjExistAlphaExit( work );
      Zukan_Detail_Map_ObjUnknownAlphaExit( work );
      ZKNDTL_COMMON_FadeSetColorlessImmediately( ZKNDTL_COMMON_FADE_DISP_M, work->fade_wk_m );  // すぐにAlphaの設定を反映してやらないと、BGとともにこのOBJも暗くしたいというOBJに、前のAlphaが当たってしまう。
    }
    break;
  }
  work->vblank_req = VBLANK_REQ_NONE;

  // パレットフェード
  {
    ZKNDTL_COMMON_PfTrans( work->pf_wk );
  }
}

//-------------------------------------
/// タウンマップデータのインデックスからzkn_area_monsno_???.dat中のインデックスを得る配列を作成する
//=====================================
static void Zukan_Detail_Map_TownmapDataIdxToZknAreaIdxArrayMake( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;
  u8 j;

  // 1度だけ作成すればよい

  // 使い方
  // u8 zkn_area_idx = townmap_data_idx_to_zkn_area_idx[townmap_data_idx];
  // townmap_data_idxはTOWNMAP_DATA_GetParamに渡すidxと同じ。
  // zkn_area_idxはzkn_area_monsno_???.dat中の何番目のデータかを表す。
  // 即ち、AREA_SEASON_DATAでplace_bitflag[zkn_area_idx]という風に使える。
  // タウンマップデータとzkn_area_monsno_???.datが同じ並びをしているなら
  // u8 townmap_data_idx_to_zkn_area_idx[TOWNMAP_DATA_MAX] = { 0, 1, 2, 3, ... , TOWNMAP_DATA_MAX-1 };
  // となる。
  // おそらく同じ並びだろうが、念のためこのような配列を間にはさむことにした。

  GF_ASSERT_MSG( TOWNMAP_DATA_MAX == ZKN_AREA_ZONE_GROUP_MAX, "ZUKAN_DETAIL_MAP : タウンマップと図鑑データで拠点の数が異なります。\n" );

  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
  {
    BOOL exist   = FALSE;
    u16  zone_id = TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_ZONE_ID );
    for( j=0; j<ZKN_AREA_ZONE_GROUP_MAX; j++ )
    {
      if( zone_id == zkn_area_zone_group[j] )
      {
        work->townmap_data_idx_to_zkn_area_idx[i] = j;
        exist = TRUE;
        break;
      }
    }
    GF_ASSERT_MSG( exist, "ZUKAN_DETAIL_MAP : タウンマップにある拠点が図鑑データにありません。\n" );
  }
}

//-------------------------------------
/// アフィン拡張BG
//=====================================
static void Zukan_Detail_Map_AffineExBGInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 読み込んで転送
  ARCHANDLE* handle;
  handle = GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, param->heap_id );

  // 2面で使用拡張パレットスロットが違うので2回転送している
  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_townmap_gra_tmap_bg_d_NCLR,
      PALTYPE_MAIN_BG_EX, 0x6000, 2*16*16*16, param->heap_id );  // BG3用にスロット3(0x6000)へ  // 2バイト*16色*16ライン*16ページのパレット
  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_townmap_gra_tmap_bg_d_NCLR,
      PALTYPE_MAIN_BG_EX, 0x4000, 2*16*16*16, param->heap_id );  // BG2用にスロット2(0x4000)へ

  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_townmap_gra_tmap_bg_d_NCGR,
      BG_FRAME_M_ROOT, 0, 64*32*8, FALSE, param->heap_id );  // 32キャラ*8キャラ=64バイト*32*8(これが限界の最大値)
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_townmap_gra_tmap_map_d_NCGR,
      BG_FRAME_M_MAP, 0, 64*32*24, FALSE, param->heap_id );  // 32キャラ*24キャラ=64バイト*32*24(これが限界の最大値)

  GFL_ARCHDL_UTIL_TransVramScreen( handle, NARC_townmap_gra_tmap_root_d_NSCR,
      BG_FRAME_M_ROOT, 0, 0x800, FALSE, param->heap_id );  // 32キャラ*32キャラ=2バイト*32*32=2K
  GFL_ARCHDL_UTIL_TransVramScreen( handle, NARC_townmap_gra_tmap_map_d_NSCR,
      BG_FRAME_M_MAP, 0, 0x800, FALSE, param->heap_id );  // 32キャラ*32キャラ=2バイト*32*32=2K

  GFL_ARC_CloseDataHandle( handle );
}
static void Zukan_Detail_Map_AffineExBGExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 特に何もしなくてよい
}

//-------------------------------------
/// OBJ
//-------------------------------------
static void Zukan_Detail_Map_ObjInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // レンダラーシステム、セルアクターユニット作成
  {
    const GFL_REND_SURFACE_INIT surface_init    = { 0, 0, 256, 192, CLSYS_DRAW_MAIN, CLSYS_REND_CULLING_TYPE_NORMAL };
    const u16                   clunit_work_num = 16; 
    
    // レンダラーシステム
    work->usual_rend = GFL_CLACT_USERREND_Create( &surface_init, 1, param->heap_id );

    // セルアクターユニット
  	work->usual_clunit	= GFL_CLACT_UNIT_Create( clunit_work_num, 0, param->heap_id );
  	GFL_CLACT_UNIT_SetUserRend( work->usual_clunit, work->usual_rend );
    
    // CLSYS_DEFREND_MAINと同じ0番しかつくっていないので
    // GFL_CLACT_WK_SetPos( work->obj_clwk[i], &pos, CLSYS_DEFREND_MAIN );
    // は、CLSYS_DEFREND_MAINのまま変更しなくて済んでいる。
  }

  // リソース読み込み
  {
    ARCHANDLE* handle;
    
    handle = GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, param->heap_id );
    work->obj_res[OBJ_RES_TM_NCL] = GFL_CLGRP_PLTT_RegisterEx( 
                                     handle,
                                     NARC_townmap_gra_tmap_obj_NCLR,
                                     CLSYS_DRAW_MAIN,
                                     OBJ_PAL_POS_M_TM*0x20,
                                     0,
                                     OBJ_PAL_NUM_M_TM,
                                     param->heap_id );	
    work->obj_res[OBJ_RES_TM_NCG] = GFL_CLGRP_CGR_Register(
                                     handle,
                                     NARC_townmap_gra_tmap_obj_d_NCGR,
                                     FALSE,
                                     CLSYS_DRAW_MAIN,
                                     param->heap_id );
    work->obj_res[OBJ_RES_TM_NCE] = GFL_CLGRP_CELLANIM_Register(
                                     handle,
                                     NARC_townmap_gra_tmap_obj_d_NCER,
                                     NARC_townmap_gra_tmap_obj_d_NANR,
                                     param->heap_id );
    GFL_ARC_CloseDataHandle( handle );

    handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, param->heap_id );
    work->obj_res[OBJ_RES_ZUKAN_NCL] = GFL_CLGRP_PLTT_RegisterEx( 
                                     handle,
                                     NARC_zukan_gra_info_mapwin_obj_NCLR,
                                     CLSYS_DRAW_MAIN,
                                     OBJ_PAL_POS_M_ZUKAN*0x20,
                                     0, 
                                     OBJ_PAL_NUM_M_ZUKAN,
                                     param->heap_id );	
    work->obj_res[OBJ_RES_ZUKAN_NCG] = GFL_CLGRP_CGR_Register(
                                     handle,
                                     NARC_zukan_gra_info_mapwin_obj_NCGR,
                                     FALSE,
                                     CLSYS_DRAW_MAIN,
                                     param->heap_id );
    work->obj_res[OBJ_RES_ZUKAN_NCE] = GFL_CLGRP_CELLANIM_Register(
                                     handle,
                                     NARC_zukan_gra_info_mapwin_obj_NCER,
                                     NARC_zukan_gra_info_mapwin_obj_NANR,
                                     param->heap_id );
    GFL_ARC_CloseDataHandle( handle );

    handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, param->heap_id );
    work->obj_res[OBJ_RES_EXIST_NCL] = GFL_CLGRP_PLTT_RegisterEx( 
                                     handle,
                                     NARC_zukan_gra_info_map_obj_NCLR,
                                     CLSYS_DRAW_MAIN,
                                     OBJ_PAL_POS_M_EXIST*0x20,
                                     0, 
                                     OBJ_PAL_NUM_M_EXIST,
                                     param->heap_id );	
    work->obj_res[OBJ_RES_EXIST_NCG] = GFL_CLGRP_CGR_Register(
                                     handle,
                                     NARC_zukan_gra_info_map_obj_NCGR,
                                     FALSE,
                                     CLSYS_DRAW_MAIN,
                                     param->heap_id );
    work->obj_res[OBJ_RES_EXIST_NCE] = GFL_CLGRP_CELLANIM_Register(
                                     handle,
                                     NARC_zukan_gra_info_map_obj_NCER,
                                     NARC_zukan_gra_info_map_obj_NANR,
                                     param->heap_id );
    GFL_ARC_CloseDataHandle( handle );
  }

  // CLWK作成
  {
    u8 anmseq[OBJ_MAX] =
    {
      // OBJ_ZUKAN_
      1,
      SEASON_L_ARROW_ANMSEQ_NORMAL,
      SEASON_R_ARROW_ANMSEQ_NORMAL,
      0,

      // OBJ_TM_
      4,
      RING_CUR_ANMSEQ,
      HERO_ANMSEQ,
    };
    u8 bgpri[OBJ_MAX] =
    {
      // OBJ_ZUKAN_
      BG_FRAME_PRI_M_ROOT,
      BG_FRAME_PRI_M_ROOT,
      BG_FRAME_PRI_M_ROOT,
      BG_FRAME_PRI_M_ROOT,

      // OBJ_TM_
      BG_FRAME_PRI_M_TEXT,
      BG_FRAME_PRI_M_TEXT,
      BG_FRAME_PRI_M_TEXT,
    };
    u8 softpri[OBJ_MAX] =
    {
      // OBJ_ZUKAN_
      2,
      1,
      0,
      3,

      // OBJ_TM_
      0,
      1,
      2,
    };

    u8 i;
    GFL_CLWK_DATA cldata;
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
   
    for( i=OBJ_TM_START; i<OBJ_TM_END; i++ )
    {	
      work->obj_clwk[i] = GFL_CLACT_WK_Create(
                             work->usual_clunit,
                             work->obj_res[OBJ_RES_TM_NCG], work->obj_res[OBJ_RES_TM_NCL], work->obj_res[OBJ_RES_TM_NCE],
                             &cldata, CLSYS_DEFREND_MAIN, param->heap_id );
    }
    
    for( i=OBJ_ZUKAN_START; i<OBJ_ZUKAN_END; i++ )
    {	
      work->obj_clwk[i] = GFL_CLACT_WK_Create(
                             work->usual_clunit,
                             work->obj_res[OBJ_RES_ZUKAN_NCG], work->obj_res[OBJ_RES_ZUKAN_NCL], work->obj_res[OBJ_RES_ZUKAN_NCE],
                             &cldata, CLSYS_DEFREND_MAIN, param->heap_id );
    }

    for( i=0; i<OBJ_MAX; i++ )
    {
      GFL_CLACT_WK_SetAnmSeq( work->obj_clwk[i], anmseq[i] );
      GFL_CLACT_WK_SetAutoAnmFlag( work->obj_clwk[i], TRUE );
      GFL_CLACT_WK_SetBgPri( work->obj_clwk[i], bgpri[i] );
      GFL_CLACT_WK_SetSoftPri( work->obj_clwk[i], softpri[i] );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[i], FALSE );
      GFL_CLACT_WK_SetObjMode( work->obj_clwk[i], GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
    }

    // 個別設定
    {
      GFL_CLACTPOS pos;
      pos.x = 16*8;
      pos.y = 12*8;
      for( i=OBJ_ZUKAN_START; i<OBJ_ZUKAN_END; i++ )
        GFL_CLACT_WK_SetPos( work->obj_clwk[i], &pos, CLSYS_DEFREND_MAIN );
    }
  }

  work->push_button = OBJ_PUSH_BUTTON_NONE;
}
static void Zukan_Detail_Map_ObjExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // CLWK破棄
  {
    u8 i;
    for( i=0; i<OBJ_MAX; i++ )
    {
      GFL_CLACT_WK_Remove( work->obj_clwk[i] );
    }
  }

  // リソース破棄
  {
    GFL_CLGRP_CELLANIM_Release( work->obj_res[OBJ_RES_EXIST_NCE] );
    GFL_CLGRP_CGR_Release( work->obj_res[OBJ_RES_EXIST_NCG] );
    GFL_CLGRP_PLTT_Release( work->obj_res[OBJ_RES_EXIST_NCL] );
    GFL_CLGRP_CELLANIM_Release( work->obj_res[OBJ_RES_ZUKAN_NCE] );
    GFL_CLGRP_CGR_Release( work->obj_res[OBJ_RES_ZUKAN_NCG] );
    GFL_CLGRP_PLTT_Release( work->obj_res[OBJ_RES_ZUKAN_NCL] );	
    GFL_CLGRP_CELLANIM_Release( work->obj_res[OBJ_RES_TM_NCE] );
    GFL_CLGRP_CGR_Release( work->obj_res[OBJ_RES_TM_NCG] );
    GFL_CLGRP_PLTT_Release( work->obj_res[OBJ_RES_TM_NCL] );
  }

  // レンダラーシステム、セルアクターユニット破棄
  {
    // セルアクターユニット
	  GFL_CLACT_UNIT_Delete( work->usual_clunit );
  	
    // レンダラーシステム
    GFL_CLACT_USERREND_Delete( work->usual_rend );
  }
}

static void Zukan_Detail_Map_ObjExistInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // CLWK作成
  {
    u8 softpri = 4;  // obj_clwkのsoftpriよりプライオリティを低くしておく、HIDE_SPOTのobj_clwkのsoftpriよりプライオリティを高くしておく

    u8 i;
    GFL_CLWK_DATA cldata;
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
 
    for( i=0; i<TOWNMAP_DATA_MAX; i++ )
    {
      cldata.pos_x = (s16)TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_ZKN_POS_X );
      cldata.pos_y = (s16)TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_ZKN_POS_Y );

      work->obj_exist_clwk[i] = GFL_CLACT_WK_Create(
                             work->clunit,
                             work->obj_res[OBJ_RES_EXIST_NCG], work->obj_res[OBJ_RES_EXIST_NCL], work->obj_res[OBJ_RES_EXIST_NCE],
                             &cldata, CLSYS_DEFREND_MAIN, param->heap_id );

      GFL_CLACT_WK_SetAnmSeq( work->obj_exist_clwk[i], TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_ZKN_ANM ) );
      GFL_CLACT_WK_SetAutoAnmFlag( work->obj_exist_clwk[i], TRUE );
      GFL_CLACT_WK_SetBgPri( work->obj_exist_clwk[i], BG_FRAME_PRI_M_ROOT );
      GFL_CLACT_WK_SetSoftPri( work->obj_exist_clwk[i], softpri );
      GFL_CLACT_WK_SetDrawEnable( work->obj_exist_clwk[i], FALSE );
      GFL_CLACT_WK_SetObjMode( work->obj_exist_clwk[i], GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
    }
  }
}
static void Zukan_Detail_Map_ObjExistExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // CLWK破棄
  {
    u8 i;
    for( i=0; i<TOWNMAP_DATA_MAX; i++ )
    {
      GFL_CLACT_WK_Remove( work->obj_exist_clwk[i] );
    }
  }
}

//-------------------------------------
/// 隠しスポット
//-------------------------------------
static void Zukan_Detail_Map_HideSpotInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // Zukan_Detail_Map_ObjInitが済んでから呼ぶこと

  GAMEDATA*  gamedata  = ZKNDTL_COMMON_GetGamedata(cmn);
  u8 i;
  
  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
  {
    u16 hide_flag	= TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_HIDE_FLAG );
    if( hide_flag != TOWNMAP_DATA_ERROR )
    {
      BOOL is_hide = !TOWNMAP_UTIL_CheckFlag( gamedata, hide_flag );
#ifdef DEBUG_SET_ANIME_AND_POS
      is_hide = FALSE;
#endif
      if( is_hide )
      {
        work->hide_spot[i].state    = HIDE_STATE_HIDE_TRUE;
        work->hide_spot[i].obj_clwk = NULL;
      }
      else
      {
        u16 placetype = TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_PLACE_TYPE );

        GFL_CLWK_DATA cldata;
        cldata.pos_x    = (s16)TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_POS_X );
        cldata.pos_y    = (s16)TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_POS_Y );
        cldata.anmseq   = (placetype==TOWNMAP_PLACETYPE_HIDE)?(14):(16);  // 偽の場合はTOWNMAP_PLACETYPE_HIDE_TOWN_Sとなっているはず
        cldata.softpri  = 5;
        cldata.bgpri    = BG_FRAME_PRI_M_ROOT;

        work->hide_spot[i].state    = HIDE_STATE_HIDE_FALSE;
        
        work->hide_spot[i].obj_clwk = GFL_CLACT_WK_Create(
            work->usual_clunit,
            work->obj_res[OBJ_RES_TM_NCG], work->obj_res[OBJ_RES_TM_NCL], work->obj_res[OBJ_RES_TM_NCE],
            &cldata, CLSYS_DEFREND_MAIN, param->heap_id );
        GFL_CLACT_WK_SetAutoAnmFlag( work->hide_spot[i].obj_clwk, TRUE );
        GFL_CLACT_WK_SetDrawEnable( work->hide_spot[i].obj_clwk, TRUE );
        GFL_CLACT_WK_SetObjMode( work->hide_spot[i].obj_clwk, GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
      }
    }
    else
    {
      work->hide_spot[i].state    = HIDE_STATE_NO;
      work->hide_spot[i].obj_clwk = NULL;
    }
  }
}
static void Zukan_Detail_Map_HideSpotExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;
  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
  {
    if( work->hide_spot[i].state == HIDE_STATE_HIDE_FALSE )
    {
      GFL_CLACT_WK_Remove( work->hide_spot[i].obj_clwk );
    }
  }
}

//-------------------------------------
/// メッセージとテキスト
//-------------------------------------
static void Zukan_Detail_Map_PrintQueTextInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;
  for( i=0; i<TEXT_MAX; i++ )
  {
    work->print_que[i] = PRINTSYS_QUE_Create( param->heap_id );
    work->text_trans[i] = FALSE;
  }
}
static void Zukan_Detail_Map_PrintQueTextExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;
  for( i=0; i<TEXT_MAX; i++ )
  {
    work->text_trans[i] = FALSE;
    PRINTSYS_QUE_Clear( work->print_que[i] );
    PRINTSYS_QUE_Delete( work->print_que[i] );
  }
}
static void Zukan_Detail_Map_PrintQueMain( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;
  for( i=0; i<TEXT_MAX; i++ )
  {
    PRINTSYS_QUE_Main( work->print_que[i] );
    Zukan_Detail_Map_TextMain( param, work, cmn, i );
  }
}
static void Zukan_Detail_Map_TextMain( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn, u8 i )
{
  if( work->text_trans[i] )
  {
    if( !PRINTSYS_QUE_IsExistTarget( work->print_que[i], GFL_BMPWIN_GetBmp(work->text_bmpwin[i]) ) )
    {
      GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[i] );
      work->text_trans[i] = FALSE;
    }
  }
}

static void Zukan_Detail_Map_MsgTextInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 dummy_bmpwin_setup[TEXT_DUMMY_MAX][7] =
  {
    { BG_FRAME_S_TEXT, 0, 0, 1, 1, BG_PAL_POS_S_TEXT, GFL_BMP_CHRAREA_GET_F },
    { BG_FRAME_M_TEXT, 0, 0, 1, 1, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_F },
  };
  u8 bmpwin_setup[TEXT_MAX][7] =
  {
    { BG_FRAME_S_TEXT, 12, 21, 16,  3, BG_PAL_POS_S_TEXT, GFL_BMP_CHRAREA_GET_F },
    { BG_FRAME_S_TEXT,  9, 25, 16,  2, BG_PAL_POS_S_TEXT, GFL_BMP_CHRAREA_GET_F },
    { BG_FRAME_M_TEXT,  2,  0, 15,  2, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_F },
    { BG_FRAME_M_TEXT,  8, 10, 16,  2, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_F },
  };

  u8 i;

  // パレット
  GFL_ARC_UTIL_TransVramPaletteEx( ARCID_FONT, NARC_font_default_nclr,
       PALTYPE_SUB_BG,
       0,
       BG_PAL_POS_S_TEXT * 0x20,
       BG_PAL_NUM_S_TEXT * 0x20,
       param->heap_id );
   GFL_ARC_UTIL_TransVramPaletteEx( ARCID_FONT, NARC_font_default_nclr,
       PALTYPE_MAIN_BG,
       0,
       BG_PAL_POS_M_TEXT * 0x20,
       BG_PAL_NUM_M_TEXT * 0x20,
       param->heap_id );

  // テキスト表示に必要なダミーのビットマップウィンドウ
  for( i=0; i<TEXT_DUMMY_MAX; i++ )
  {
    work->text_dummy_bmpwin[i] = GFL_BMPWIN_Create(
                                     dummy_bmpwin_setup[i][0],
                                     dummy_bmpwin_setup[i][1], dummy_bmpwin_setup[i][2], dummy_bmpwin_setup[i][3], dummy_bmpwin_setup[i][4],
                                     dummy_bmpwin_setup[i][5], dummy_bmpwin_setup[i][6] );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_dummy_bmpwin[i]), 0 );
    GFL_BMPWIN_TransVramCharacter( work->text_dummy_bmpwin[i] );
  }

  // ビットマップウィンドウ
  for( i=0; i<TEXT_MAX; i++ )
  {
    work->text_bmpwin[i] = GFL_BMPWIN_Create(
                                     bmpwin_setup[i][0],
                                     bmpwin_setup[i][1], bmpwin_setup[i][2], bmpwin_setup[i][3], bmpwin_setup[i][4],
                                     bmpwin_setup[i][5], bmpwin_setup[i][6] );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[i]), 0 );
    GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[i] );
  }

  // メッセージ
  work->msgdata[MSG_ZUKAN] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
                           ARCID_MESSAGE, NARC_message_zkn_dat,
                           param->heap_id );
  work->msgdata[MSG_PLACE] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
                           ARCID_MESSAGE, NARC_message_place_name_dat,
                           param->heap_id );
}
static void Zukan_Detail_Map_MsgTextExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;
  for( i=0; i<MSG_MAX; i++ )
    GFL_MSG_Delete( work->msgdata[i] );
  for( i=0; i<TEXT_MAX; i++ )
  {
    work->text_trans[i] = FALSE;
    PRINTSYS_QUE_Clear( work->print_que[i] );
    GFL_BMPWIN_Delete( work->text_bmpwin[i] );
  }
  for( i=0; i<TEXT_DUMMY_MAX; i++ )
    GFL_BMPWIN_Delete( work->text_dummy_bmpwin[i] );
}

//-------------------------------------
/// 上画面
//-------------------------------------
static void Zukan_Detail_Map_PlaceInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  ARCHANDLE* handle;

  // BG PLACE
  handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, param->heap_id );
  // パレット
  GFL_ARCHDL_UTIL_TransVramPaletteEx(
                               handle,
                               NARC_zukan_gra_info_info_bgu_NCLR,
                               PALTYPE_SUB_BG,
                               0 * 0x20,
                               BG_PAL_POS_S_PLACE * 0x20,
                               BG_PAL_NUM_S_PLACE * 0x20,
                               param->heap_id );
  // キャラ
  work->place_tinfo = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(
                            handle,
                            NARC_zukan_gra_info_info_bgu_NCGR,
                            BG_FRAME_S_PLACE,
                            0,
                            FALSE,
                            param->heap_id );
  GF_ASSERT_MSG( work->place_tinfo != GFL_ARCUTIL_TRANSINFO_FAIL, "ZUKAN_DETAIL_MAP : BGキャラ領域が足りませんでした。\n" );
  // スクリーン
  GFL_ARCHDL_UTIL_TransVramScreenCharOfs(
        handle,
        NARC_zukan_gra_info_mapwin_bgu_NSCR,
        BG_FRAME_S_PLACE,
        0,
        GFL_ARCUTIL_TRANSINFO_GetPos( work->place_tinfo ),
        0,
        FALSE,
        param->heap_id );

  GFL_ARC_CloseDataHandle( handle );
}
static void Zukan_Detail_Map_PlaceExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  GFL_BG_FreeCharacterArea( BG_FRAME_S_PLACE,
                            GFL_ARCUTIL_TRANSINFO_GetPos( work->place_tinfo ),
                            GFL_ARCUTIL_TRANSINFO_GetSize( work->place_tinfo ) );
}
static void Zukan_Detail_Map_PlaceMain( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  Zukan_Detail_Map_PlaceMainScroll( param, work, cmn );
  Zukan_Detail_Map_PlaceMainIcon( param, work, cmn );
}
static void Zukan_Detail_Map_PlaceMainScroll( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // BGのスクロールアニメーション
  switch( work->scroll_state )
  {
  case SCROLL_STATE_DOWN:
    {
    }
    break;
  case SCROLL_STATE_DOWN_TO_UP:
    {
      int y = GFL_BG_GetScrollY( BG_FRAME_S_PLACE );
      if( y >= 8*8 )
      {
        GFL_BG_SetScrollReq( BG_FRAME_S_PLACE, GFL_BG_SCROLL_Y_SET, 8*8 );
        work->scroll_state = SCROLL_STATE_UP; 
      }
      else
      {
        GFL_BG_SetScrollReq( BG_FRAME_S_PLACE, GFL_BG_SCROLL_Y_INC, SCROLL_VEL );
        GFL_BG_SetScrollReq( BG_FRAME_S_TEXT, GFL_BG_SCROLL_Y_INC, SCROLL_VEL );
      }
    }
    break;
  case SCROLL_STATE_UP:
    {
    }
    break;
  case SCROLL_STATE_UP_TO_DOWN:
    {
      int y = GFL_BG_GetScrollY( BG_FRAME_S_PLACE );
      if( y <= 0 )
      {
        GFL_BG_SetScrollReq( BG_FRAME_S_PLACE, GFL_BG_SCROLL_Y_SET, 0 );
        work->scroll_state = SCROLL_STATE_DOWN; 
      }
      else
      {
        GFL_BG_SetScrollReq( BG_FRAME_S_PLACE, GFL_BG_SCROLL_Y_DEC, SCROLL_VEL );
        GFL_BG_SetScrollReq( BG_FRAME_S_TEXT, GFL_BG_SCROLL_Y_DEC, SCROLL_VEL );
      }
    }
    break;
  }

  // ポケアイコンのスクロールアニメーション
  if( work->pokeicon_clwk[work->pokeicon_swap_curr] )
  {
    GFL_CLACTPOS pos;
    switch( work->pokeicon_scroll_state )
    {
    case SCROLL_STATE_DOWN:
      {
      }
      break;
    case SCROLL_STATE_DOWN_TO_UP:
      {
        GFL_CLACT_WK_GetPos( work->pokeicon_clwk[work->pokeicon_swap_curr], &pos, CLSYS_DEFREND_SUB );
        if( pos.y <= pokeicon_pos[3] )
        {
          pos.y = pokeicon_pos[3];
          work->pokeicon_scroll_state = SCROLL_STATE_UP; 
        }
        else
        {
          pos.y -= SCROLL_VEL;
        }
        GFL_CLACT_WK_SetPos( work->pokeicon_clwk[work->pokeicon_swap_curr], &pos, CLSYS_DEFREND_SUB );
      }
      break;
    case SCROLL_STATE_UP:
      {
      }
      break;
    case SCROLL_STATE_UP_TO_DOWN:
      {
        GFL_CLACT_WK_GetPos( work->pokeicon_clwk[work->pokeicon_swap_curr], &pos, CLSYS_DEFREND_SUB );
        if( pos.y >= pokeicon_pos[1] )
        {
          pos.y = pokeicon_pos[1];
          work->pokeicon_scroll_state = SCROLL_STATE_DOWN; 
        }
        else
        {
          pos.y += SCROLL_VEL;
        }
        GFL_CLACT_WK_SetPos( work->pokeicon_clwk[work->pokeicon_swap_curr], &pos, CLSYS_DEFREND_SUB );
      }
      break;
    }
  }
}
static void Zukan_Detail_Map_PlaceMainIcon( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 陸上、水上、釣りアイコンの明滅アニメーション
  // 明滅アニメーションはしなくてよくなったので、何もしない
}

//-------------------------------------
/// 下画面
//=====================================
static void Zukan_Detail_Map_SeasonArrowMain( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);

  u16 push_anmseq;
  u8  pair_button;  // アニメーションを合わせておく必要があるボタン
  u16 pair_anmseq;

  if( work->push_button == OBJ_SEASON_R )
  {
    push_anmseq = SEASON_R_ARROW_ANMSEQ_NORMAL;
    pair_button = OBJ_SEASON_L;
    pair_anmseq = SEASON_L_ARROW_ANMSEQ_NORMAL;
  }
  else if( work->push_button == OBJ_SEASON_L )
  {
    push_anmseq = SEASON_L_ARROW_ANMSEQ_NORMAL;
    pair_button = OBJ_SEASON_R;
    pair_anmseq = SEASON_R_ARROW_ANMSEQ_NORMAL;
  }

  if( work->push_button != OBJ_PUSH_BUTTON_NONE )
  {
    if( !GFL_CLACT_WK_CheckAnmActive( work->obj_clwk[work->push_button] ) )
    {
      GFL_CLACT_WK_SetAnmSeq( work->obj_clwk[work->push_button], push_anmseq );

      GFL_CLACT_WK_SetAutoAnmFlag( work->obj_clwk[pair_button], TRUE );
      GFL_CLACT_WK_SetAnmSeq( work->obj_clwk[pair_button], pair_anmseq );

      work->push_button = OBJ_PUSH_BUTTON_NONE;

      if(    work->state == STATE_TOP       // 矢印アニメの点滅の終了とOBJの表示OFFから表示ONまでの一連の流れの終了とどちらが先か分からないので。
          || work->state == STATE_SELECT )
      {
        ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, TRUE );
      }
    }
  }
}

//-------------------------------------
/// ポケアイコン
//=====================================
static void PokeiconBaseInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 不変物の生成
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_POKEICON, param->heap_id );

/*
  work->pokeicon_ncl = GFL_CLGRP_PLTT_RegisterEx(
                           handle,
                           POKEICON_GetPalArcIndex(),
                           CLSYS_DRAW_SUB,
                           OBJ_PAL_POS_S_POKEICON * 0x20,
                           0,
                           OBJ_PAL_NUM_S_POKEICON,
                           param->heap_id );
*/
  work->pokeicon_ncl = GFL_CLGRP_PLTT_RegisterComp(  // ポケアイコンのパレットは圧縮されている
                           handle,
                           POKEICON_GetPalArcIndex(),
                           CLSYS_DRAW_SUB,
                           OBJ_PAL_POS_S_POKEICON * 0x20,
                           param->heap_id );

  work->pokeicon_nce = GFL_CLGRP_CELLANIM_Register(
                           handle,
                           POKEICON_GetCellSubArcIndex(),
                           POKEICON_GetAnmSubArcIndex(),
                           param->heap_id );

  GFL_ARC_CloseDataHandle( handle );
}
static void PokeiconBaseExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 不変物の生成
  GFL_CLGRP_CELLANIM_Release( work->pokeicon_nce );
  GFL_CLGRP_PLTT_Release( work->pokeicon_ncl );
}

static GFL_CLWK* PokeiconInit( u32* ncg, u32 ncl, u32 nce, GFL_CLUNIT* clunit, HEAPID heap_id, u32 mons, u32 form_no, u32 sex, BOOL egg, u8 x, u8 y )
{
  GFL_CLWK* clwk;
  GFL_CLWK_DATA data;

  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_POKEICON, heap_id );

  data.pos_x   = x;
  data.pos_y   = y;
  data.anmseq  = 1;  // アニメシーケンスで指定( 0=瀕死, 1=HP最大, 2=HP緑, 3=HP黄, 4=HP赤, 5=状態異常 )
  data.softpri = 1;  // 上にアイテムアイコンを描画するので優先度を下げておく
  data.bgpri   = 0;

  *ncg = GFL_CLGRP_CGR_Register(
             handle,
             POKEICON_GetCgxArcIndexByMonsNumber( mons, form_no, sex, egg ),
             FALSE,
             CLSYS_DRAW_SUB,
             heap_id );
    
  clwk = GFL_CLACT_WK_Create(
             clunit,
             *ncg,
             ncl,
             nce,
             &data,
             CLSYS_DEFREND_SUB,
             heap_id );
   
  // オートアニメ OFF
  GFL_CLACT_WK_SetAutoAnmFlag( clwk, FALSE );

  {
    u8 pal_num = POKEICON_GetPalNum( mons, form_no, sex, egg );
    GFL_CLACT_WK_SetPlttOffs( clwk, pal_num, CLWK_PLTTOFFS_MODE_OAM_COLOR );
  }

  //GFL_CLACT_WK_SetObjMode( clwk, GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
  GFL_CLACT_WK_SetObjMode( clwk, GX_OAM_MODE_NORMAL );  // BGとともにこのOBJも暗くしたいのは、フェードインとフェードアウトのときだけであり、
                                                        // ポケモンを変更しただけのときは、暗くしたり半透明にしたりしたくない。
                                                        // BGとともにこのOBJも暗くしたいときは、この関数の後に設定すること。

  GFL_ARC_CloseDataHandle( handle );

  return clwk;
}
static void PokeiconExit( u32 ncg, GFL_CLWK* clwk )
{
  // CLWK破棄
  GFL_CLACT_WK_Remove( clwk );
  // リソース開放
  GFL_CLGRP_CGR_Release( ncg );
}

//-------------------------------------
/// プレイヤーの位置のデータインデックスを得る
//=====================================
static u8 Zukan_Detail_Map_GetPlayerPosDataIdx( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  GAMEDATA*       gamedata  = ZKNDTL_COMMON_GetGamedata(cmn);
  PLAYER_WORK*    player_wk = GAMEDATA_GetMyPlayerWork( gamedata );
  u16             zone_id   = PLAYERWORK_getZoneID( player_wk );
  u16             escape_id = GAMEDATA_GetEscapeLocation( gamedata )->zone_id;
 
  u8   player_pos_data_idx = DATA_IDX_NONE;

  u8 i;

  // ゾーンと完全対応する場所を取得
  if( player_pos_data_idx == DATA_IDX_NONE )
  {
	  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
	  {	
	  	if( zone_id == TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_ZONE_ID ) )
      {
        player_pos_data_idx = i;
        break;
      }
    }
  }
  // 現在地がフィールドではないならば、エスケープID検索
  if( player_pos_data_idx == DATA_IDX_NONE )
  {
	  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
	  {	
	  	if( escape_id == TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_ZONE_ID ) )
      {
        player_pos_data_idx = i;
        break;
      }
    }
  }

  return player_pos_data_idx;
}

//-------------------------------------
/// プレイヤーの位置にOBJを配置する
//=====================================
static void Zukan_Detail_Map_SetPlayer( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  GFL_CLACTPOS pos;

  u8 player_pos_data_idx = Zukan_Detail_Map_GetPlayerPosDataIdx( param, work, cmn );
  
  if( player_pos_data_idx != DATA_IDX_NONE )
  {
    pos.x = (s16)TOWNMAP_DATA_GetParam( work->townmap_data, player_pos_data_idx, TOWNMAP_DATA_PARAM_POS_X );
    pos.y = (s16)TOWNMAP_DATA_GetParam( work->townmap_data, player_pos_data_idx, TOWNMAP_DATA_PARAM_POS_Y );
    GFL_CLACT_WK_SetPos( work->obj_clwk[OBJ_HERO], &pos, CLSYS_DEFREND_MAIN );
    // 表示はZukan_Detail_Map_DrawPlayerに任せる
  }
}
//-------------------------------------
/// プレイヤーの位置に配置するOBJの表示/非表示を切り替える
//=====================================
static void Zukan_Detail_Map_DrawPlayer( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                         BOOL on_off )
{
  // この関数より前に1度、Zukan_Detail_Map_SetPlayer関数を呼び出して、配置場所を設定しておくこと。
  if( on_off )
  {
    u8 player_pos_data_idx = Zukan_Detail_Map_GetPlayerPosDataIdx( param, work, cmn );
    if( player_pos_data_idx != DATA_IDX_NONE )
    {
      GFL_CLACT_WK_SetAnmSeq( work->obj_clwk[OBJ_HERO], HERO_ANMSEQ );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_HERO], TRUE );
    }
  }
  else
  {
    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_HERO], FALSE );
  }
}
//-------------------------------------
/// プレイヤーの位置に配置するOBJを現在表示しているか否か
//=====================================
static BOOL Zukan_Detail_Map_IsDrawPlayer( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  return GFL_CLACT_WK_GetDrawEnable( work->obj_clwk[OBJ_HERO] );
}

//-------------------------------------
/// カーソルを配置する
//=====================================
static void Zukan_Detail_Map_SetCursor( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  GFL_CLACTPOS pos = { CURSOR_DEFAULT_POS_X, CURSOR_DEFAULT_POS_Y };  // デフォルト位置

  // どこの拠点も選択していないときは、プレイヤーの位置にカーソルを合わせておく
  if( work->select_data_idx == DATA_IDX_NONE )
  {
    work->select_data_idx = Zukan_Detail_Map_GetPlayerPosDataIdx( param, work, cmn );
  }

  if( work->select_data_idx != DATA_IDX_NONE )
  {
    pos.x = (s16)TOWNMAP_DATA_GetParam( work->townmap_data, work->select_data_idx, TOWNMAP_DATA_PARAM_POS_X );
    pos.y = (s16)TOWNMAP_DATA_GetParam( work->townmap_data, work->select_data_idx, TOWNMAP_DATA_PARAM_POS_Y );
  }
  
  GFL_CLACT_WK_SetPos( work->obj_clwk[OBJ_CURSOR], &pos, CLSYS_DEFREND_MAIN );
  GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_CURSOR], TRUE );
}

//-------------------------------------
/// プレイヤーの位置にカーソルを配置する
//=====================================
static void Zukan_Detail_Map_SetCursorOnPlayer( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 絶対にプレイヤーの位置にカーソルを合わせたいので、プレイヤーの位置の拠点を選択しておく
  work->select_data_idx = Zukan_Detail_Map_GetPlayerPosDataIdx( param, work, cmn );

  Zukan_Detail_Map_SetCursor( param, work, cmn );
}

//-------------------------------------
/// 指定した座標とヒットするデータがあればそのデータインデックスを返す
//=====================================
static u8 Zukan_Detail_Map_Hit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                u8 x, u8 y )
{
  // 最小距離のものを残す
  fx32  distance_min;
  u8    distance_min_hit_data_idx = DATA_IDX_NONE;
  u8  i;

	GFL_COLLISION3D_CIRCLE		circle;
	VecFx32	v;

  v.x	= FX32_CONST( x );
  v.y	= FX32_CONST( y );
  v.z	= 0;
  GFL_COLLISION3D_CIRCLE_SetData( &circle, &v, 0 );

  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
  {
	  GFL_COLLISION3D_CYLINDER	cylinder;
	  VecFx32	v1, v2;
	  fx32 w;
    GFL_COLLISION3D_CYLXCIR_RESULT  result;

    if( work->hide_spot[i].state == HIDE_STATE_HIDE_TRUE )  // 隠しマップなら絶対にあたらない
    {
      continue;
    }
  	v1.x	= FX32_CONST( TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_HIT_START_X ) );
  	v1.y	= FX32_CONST( TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_HIT_START_Y ) );
  	v1.z	= 0;
  	v2.x	= FX32_CONST( TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_HIT_END_X ) );
  	v2.y	= FX32_CONST( TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_HIT_END_Y ) );
  	v2.z	= 0;
	  w			= FX32_CONST( TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_HIT_WIDTH ) );
	  GFL_COLLISION3D_CYLINDER_SetData( &cylinder, &v1, &v2, w );

    // 最初距離か
	  if( GFL_COLLISION3D_CYLXCIR_Check( &cylinder, &circle, &result ) )
    {
      if( distance_min_hit_data_idx == DATA_IDX_NONE )
      {
        distance_min = result.dist;
        distance_min_hit_data_idx = i;
      }
      else
      {
        if( distance_min > result.dist )
        {
          distance_min = result.dist;
          distance_min_hit_data_idx = i;
        }
      }
    }
  }

  return distance_min_hit_data_idx;
}

//-------------------------------------
/// 指定した座標は吸い込み範囲内か
//=====================================
static BOOL Zukan_Detail_Map_IsPullHit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                        u8 data_idx, u8 x, u8 y, u32* distance_sq )
{
  u32 data_x, data_y;
  u32 d2;

  if( work->hide_spot[data_idx].state == HIDE_STATE_HIDE_TRUE )  // 隠しマップなら絶対にあたらない
  {
    return FALSE;
  }

  data_x = TOWNMAP_DATA_GetParam( work->townmap_data, data_idx, TOWNMAP_DATA_PARAM_CURSOR_X );
  data_y = TOWNMAP_DATA_GetParam( work->townmap_data, data_idx, TOWNMAP_DATA_PARAM_CURSOR_Y );

  d2 = ( data_x - x ) * ( data_x - x ) + ( data_y - y ) * ( data_y - y );

  if( d2 < PLACE_PULL_R_SQ )
  {
    if( distance_sq ) *distance_sq = d2;
    return TRUE;
  }

  return FALSE;
}

//-------------------------------------
/// 指定した座標を吸い込み範囲内に持つデータがあればそのデータインデックスを返す
//=====================================
static u8 Zukan_Detail_Map_PullHit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                u8 x, u8 y )
{
	u32 now_distance_sq;
	u32 best_distance_sq;
  u8  best_idx           = DATA_IDX_NONE;
  u8  i;

	// 複数衝突していたならば一番近くのものを返す
  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
  {
		if( Zukan_Detail_Map_IsPullHit( param, work, cmn, i, x, y, &now_distance_sq ) )
    {
      if( best_idx == DATA_IDX_NONE )
      {
        best_idx = i;
        best_distance_sq = now_distance_sq;
      }
      else
      {
        if( best_distance_sq > now_distance_sq )
        {
          best_idx = i;
          best_distance_sq = now_distance_sq;
        }
      }
    }
  }
  return best_idx;
}

//-------------------------------------
/// 入力
//=====================================
static void Zukan_Detail_Map_Input( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);

  if( (!(work->input_enable)) || (work->push_button != OBJ_PUSH_BUTTON_NONE) ) return;

  switch(work->state)
  {
  case STATE_TOP:
    {
      BOOL is_input = FALSE;
      u32 x, y;
      BOOL change_state = FALSE;

      // キー入力
      if( !is_input )
      {
        if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
        {
          change_state = TRUE;
          is_input = TRUE;
        }
        else if( work->appear_rule != APPEAR_RULE_YEAR )
        {
          if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_R )
          {
            // 変更後の季節にする
            work->season_id++;
            if( work->season_id >= PMSEASON_TOTAL ) work->season_id = PMSEASON_SPRING;
          
            Zukan_Detail_Map_ChangeSeason( param, work, cmn, TRUE, GFL_APP_KTST_KEY );
            PMSND_PlaySE( SEQ_SE_SELECT3 );

            is_input = TRUE;
          }
          else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_L )
          {
            // 変更後の季節にする
            if( work->season_id == PMSEASON_SPRING ) work->season_id = PMSEASON_WINTER;
            else                                     work->season_id--;
          
            Zukan_Detail_Map_ChangeSeason( param, work, cmn, FALSE, GFL_APP_KTST_KEY );
            PMSND_PlaySE( SEQ_SE_SELECT3 );
          
            is_input = TRUE;
          }
        }
        if( is_input )  // キー入力確定
        {
          GFL_UI_SetTouchOrKey( GFL_APP_KTST_KEY );
          if( change_state )
          {
            work->top_to_select_ktst = GFL_APP_KTST_KEY;
          }
        }
      }

      // タッチ入力
      if( !is_input ) 
      {
		    if( GFL_UI_TP_GetPointTrg( &x, &y ) )
        {
          if( work->appear_rule == APPEAR_RULE_YEAR )
          {
            if( 0<=y&&y<168)
            {
              change_state = TRUE;
              is_input = TRUE;
            }
          }
          else
          {
            if(    SEASON_R_ARROW_POS_X<=x&&x<SEASON_R_ARROW_POS_X+SEASON_R_ARROW_SIZE_X
                && SEASON_R_ARROW_POS_Y<=y&&y<SEASON_R_ARROW_POS_Y+SEASON_R_ARROW_SIZE_Y )
            {
              // 変更後の季節にする
              work->season_id++;
              if( work->season_id >= PMSEASON_TOTAL ) work->season_id = PMSEASON_SPRING;

              Zukan_Detail_Map_ChangeSeason( param, work, cmn, TRUE, GFL_APP_KTST_TOUCH );
              PMSND_PlaySE( SEQ_SE_SELECT3 );
              is_input = TRUE;
            }
            else if(    SEASON_L_ARROW_POS_X<=x&&x<SEASON_L_ARROW_POS_X+SEASON_L_ARROW_SIZE_X
                     && SEASON_L_ARROW_POS_Y<=y&&y<SEASON_L_ARROW_POS_Y+SEASON_L_ARROW_SIZE_Y )
            {
              // 変更後の季節にする
              if( work->season_id == PMSEASON_SPRING ) work->season_id = PMSEASON_WINTER;
              else                                     work->season_id--;

              Zukan_Detail_Map_ChangeSeason( param, work, cmn, FALSE, GFL_APP_KTST_TOUCH );
              PMSND_PlaySE( SEQ_SE_SELECT3 );
              is_input = TRUE;
            }
            else if(    SEASON_PANEL_POS_X<=x&&x<SEASON_PANEL_POS_X+SEASON_PANEL_SIZE_X
                     && SEASON_PANEL_POS_Y<=y&&y<SEASON_PANEL_POS_Y+SEASON_PANEL_SIZE_Y )
            {
              // 季節名を表示している場所をタッチしたときは、何もしない
            }
            else if(0<=y&&y<168)
            {
              change_state = TRUE;
              is_input = TRUE;
            }
          }
        }
        if( is_input )  // タッチ入力確定
        {
          GFL_UI_SetTouchOrKey( GFL_APP_KTST_TOUCH );
          if( change_state )
          {
            work->top_to_select_ktst = GFL_APP_KTST_TOUCH;
            work->top_to_select_x    = (u8)x;
            work->top_to_select_y    = (u8)y;
          }
        }
      }

      // 状態変更
      if( change_state )
      {
        // タッチorキーを切り替えているのに、一年中生息地不明の場合はここでその入力を無効にしている。
        // しかし、この図鑑分布画面を抜けるにはこの後にタッチorキー入力が必要だし、
        // このSTATE_TOPの1階層下のSTATE_SELECTに行くにはタッチorキー入力が必要だから、
        // もう一度何かしら入力があるはずだから、ここでは無効にしたままでいい。
        if( !( work->appear_rule == APPEAR_RULE_YEAR && work->habitat == HABITAT_UNKNOWN ) )  // 一年中生息地不明の場合はトップから降りられない
        {
          Zukan_Detail_Map_ChangeState( param, work, cmn, STATE_TOP_TO_SELECT );
          PMSND_PlaySE( SEQ_SE_DECIDE1 );

          ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, FALSE );  // タッチバーが有効のままなので、無効にしておく。
                                                                        // ここからの状態遷移の最後STATE_SELECTになるときに、SetTypeで有効になる。
        }
      }
    }
    break;
  case STATE_SELECT:
    {
      BOOL is_input = FALSE;
      u32 x, y;
      BOOL select_enable = TRUE;  // is_inputと完全には一致しない(入力としては無効な場所をタッチすることがあるので)

      // キー入力
      if( !is_input )
      {
        if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )  // タッチバーの戻るボタンのとき
        {
          work->input_enable = FALSE;  // Zukan_Detail_Map_CommandFuncでinput_enableをFALSEにするのはここより2フレームも遅れるので、ここでFALSEにしておく。
          
          is_input = TRUE;
          select_enable = FALSE;
        }
        else if( work->appear_rule != APPEAR_RULE_YEAR )
        {
          if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_R )
          {
            // 変更後の季節にする
            work->season_id++;
            if( work->season_id >= PMSEASON_TOTAL ) work->season_id = PMSEASON_SPRING;
          
            Zukan_Detail_Map_ChangeSeason( param, work, cmn, TRUE, GFL_APP_KTST_KEY );
            PMSND_PlaySE( SEQ_SE_SELECT3 );

            is_input = TRUE;
            select_enable = FALSE;
          }
          else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_L )
          {
            // 変更後の季節にする
            if( work->season_id == PMSEASON_SPRING ) work->season_id = PMSEASON_WINTER;
            else                                     work->season_id--;
          
            Zukan_Detail_Map_ChangeSeason( param, work, cmn, FALSE, GFL_APP_KTST_KEY );
            PMSND_PlaySE( SEQ_SE_SELECT3 );
          
            is_input = TRUE;
            select_enable = FALSE;
          }
        }
        if( is_input )  // キー入力確定
        {
          GFL_UI_SetTouchOrKey( GFL_APP_KTST_KEY );
        }
      }

      // タッチ入力
      if( !is_input )
      {
        if( GFL_UI_TP_GetPointTrg( &x, &y ) )
        {
          if( work->appear_rule == APPEAR_RULE_YEAR )
          {
            if( 0<=y&&y<168)
            {
              // 何もしない
            }
            else
            {
              // タッチバーを表示している場所をタッチしたときは、マップ上の拠点の選択操作はしていないものとする
              select_enable = FALSE;
            }
          }
          else if( work->appear_rule == APPEAR_RULE_SEASON )
          {
            if(    SEASON_R_ARROW_POS_X<=x&&x<SEASON_R_ARROW_POS_X+SEASON_R_ARROW_SIZE_X
                && SEASON_R_ARROW_POS_Y<=y&&y<SEASON_R_ARROW_POS_Y+SEASON_R_ARROW_SIZE_Y )
            {
              // 変更後の季節にする
              work->season_id++;
              if( work->season_id >= PMSEASON_TOTAL ) work->season_id = PMSEASON_SPRING;

              Zukan_Detail_Map_ChangeSeason( param, work, cmn, TRUE, GFL_APP_KTST_TOUCH );
              PMSND_PlaySE( SEQ_SE_SELECT3 );
              is_input = TRUE;
              select_enable = FALSE;
            }
            else if(    SEASON_L_ARROW_POS_X<=x&&x<SEASON_L_ARROW_POS_X+SEASON_L_ARROW_SIZE_X
                     && SEASON_L_ARROW_POS_Y<=y&&y<SEASON_L_ARROW_POS_Y+SEASON_L_ARROW_SIZE_Y )
            {
              // 変更後の季節にする
              if( work->season_id == PMSEASON_SPRING ) work->season_id = PMSEASON_WINTER;
              else                                     work->season_id--;

              Zukan_Detail_Map_ChangeSeason( param, work, cmn, FALSE, GFL_APP_KTST_TOUCH );
              PMSND_PlaySE( SEQ_SE_SELECT3 );
              is_input = TRUE;
              select_enable = FALSE;
            }
            else if(    SEASON_PANEL_POS_X<=x&&x<SEASON_PANEL_POS_X+SEASON_PANEL_SIZE_X
                     && SEASON_PANEL_POS_Y<=y&&y<SEASON_PANEL_POS_Y+SEASON_PANEL_SIZE_Y )
            {
              // 季節名を表示している場所をタッチしたときは、マップ上の拠点の選択操作はしていないものとする
              select_enable = FALSE;
            }
            else if( 0<=y&&y<168)
            {
              // 何もしない
            }
            else
            {
              // タッチバーを表示している場所をタッチしたときは、マップ上の拠点の選択操作はしていないものとする
              select_enable = FALSE;
            }
          }
        }
        if( is_input )  // タッチ入力確定
        {
          GFL_UI_SetTouchOrKey( GFL_APP_KTST_TOUCH );
        }
      }

      if( select_enable )
      {
        // マップ上の拠点の選択操作
        Zukan_Detail_Map_InputSelect( param, work, cmn );
      }
    }
    break;
  }
}

//-------------------------------------
/// マップ上の拠点の選択操作
//=====================================
static void Zukan_Detail_Map_InputSelect( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u32 x, y;

  // 生息地が不明のときは、マップ上の拠点を選べないことにしておく 
  if( work->habitat == HABITAT_UNKNOWN ) return;

  // ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )が真となる
  // タッチバーの戻るボタンのときは、
  // Zukan_Detail_Map_Inputで処理を済ませているので、ここには来ない。

  // キー or タッチの変更
  if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
  {
    if( GFL_UI_TP_GetPointTrg( &x, &y ) )
    {
      GFL_UI_SetTouchOrKey( GFL_APP_KTST_TOUCH );
      // カーソルを非表示にする
      {
        GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_CURSOR], FALSE );
      }
      // タッチに変更になった場合は、タッチの処理を続ける
    }
  }
  else
  {
    if( GFL_UI_KEY_GetTrg() )
    {
      {
        GFL_UI_SetTouchOrKey( GFL_APP_KTST_KEY );
        // カーソルを配置する
        Zukan_Detail_Map_SetCursor( param, work, cmn );
        Zukan_Detail_Map_ChangePlace( param, work, cmn );
        // 草原、水、釣りのアイコンの明るさを変更する
        Zukan_Detail_Map_UtilBrightenPlaceIcon( param, work, cmn );

        // キーに変更になった場合は、キーに変更しただけで処理を終える
        return;
      }
    }
  }

  // キー入力で選択操作
  if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
  {
    u8            select_data_idx_prev  = work->select_data_idx;
    GFL_CLACTPOS  pos;
    int           key_cont;
    BOOL          user_move             = FALSE;  // ユーザの上下左右の入力による移動があるならTRUE
    BOOL          pull_move             = FALSE;  // 吸い込みによる移動があるならTRUE
	  GFL_CLACTPOS  pull_pos;
    BOOL          pull_force            = FALSE;  // もし吸い込みしかない場合、強制的に吸い込ませるか否か
    VecFx32	norm	= {0,0,0};
	  VecFx32	pull	= {0,0,0};

    // 移動前の位置
    GFL_CLACT_WK_GetPos( work->obj_clwk[OBJ_CURSOR], &pos, CLSYS_DEFREND_MAIN );

    // 移動後の位置
    // ユーザの上下左右の入力による移動
    key_cont = GFL_UI_KEY_GetCont();  // キー入力中から変更なしなのでGFL_UI_SetTouchOrKeyは呼ばなくていい
    if(    ( key_cont & PAD_KEY_RIGHT )
        || ( key_cont & PAD_KEY_LEFT )
        || ( key_cont & PAD_KEY_UP )
        || ( key_cont & PAD_KEY_DOWN ) )
    {
      user_move = TRUE;
      
      if( key_cont & PAD_KEY_RIGHT ) norm.x += FX32_ONE;
      if( key_cont & PAD_KEY_LEFT )  norm.x -= FX32_ONE;
      if( key_cont & PAD_KEY_UP )    norm.y -= FX32_ONE;
      if( key_cont & PAD_KEY_DOWN )  norm.y += FX32_ONE;
    }

    // 吸い込まれる処理
    // 吸い込み範囲内ならば吸い込まれる
    {
      u8 pull_data_idx = Zukan_Detail_Map_PullHit( param, work, cmn, (u8)pos.x, (u8)pos.y ); 
      if( pull_data_idx != DATA_IDX_NONE )
      {
        VecFx32  place_pos;
        VecFx32  now_pos;
        
        pull_move = TRUE;
        pull_pos.x = (s16)TOWNMAP_DATA_GetParam( work->townmap_data, pull_data_idx, TOWNMAP_DATA_PARAM_CURSOR_X );
        pull_pos.y = (s16)TOWNMAP_DATA_GetParam( work->townmap_data, pull_data_idx, TOWNMAP_DATA_PARAM_CURSOR_Y );

        place_pos.x = FX32_CONST( pull_pos.x );
        place_pos.y = FX32_CONST( pull_pos.y );

        place_pos.z = 0;

        now_pos.x = FX32_CONST( pos.x );
        now_pos.y = FX32_CONST( pos.y );
        now_pos.z = 0;

        VEC_Subtract( &place_pos, &now_pos, &pull );

        // もし吸い込みしかない場合、強制的に吸い込ませるか否か
        if( VEC_Mag(&pull) < PLACE_PULL_FORCE )
        {
          pull_force = TRUE;
        }
        // もし一定以上の距離ならば、最大値までにおさめる
        if( VEC_Mag(&pull) > PLACE_PULL_STRONG )
        {
          VEC_Normalize( &pull, &pull );
          GFL_CALC3D_VEC_MulScalar( &pull, PLACE_PULL_STRONG, &pull );
        }
      }
    }

    // 移動計算
    if( user_move || pull_move )
    {
      // 吸い込みしかなく、強制的に吸い込ませる場合
      if( (!user_move) && pull_move && pull_force )
      {
        pos = pull_pos;
      }
      else
      {
    		VecFx32	add;
	    	VEC_Normalize( &norm, &norm );

		    add.x	= (norm.x * CURSOR_ADD_SPEED)	+ pull.x;
		    add.y	= (norm.y * CURSOR_ADD_SPEED)	+ pull.y;
		    add.z	=	0;

	    	if( 0 < add.x && add.x < FX32_ONE )
		    {	
			    add.x	= FX32_ONE;
    		}
	    	else if( -FX32_ONE < add.x && add.x < 0 )
		    {	
		    	add.x = -FX32_ONE;
	    	}
	    	if( 0 < add.y && add.y < FX32_ONE )
		    {	
		    	add.y	= FX32_ONE;
	    	}
	    	else if( -FX32_ONE < add.y && add.y < 0 )
		    {	
			    add.y = -FX32_ONE;
    		}

	    	pos.x	+= add.x >> FX32_SHIFT; 
	    	pos.y	+= add.y >> FX32_SHIFT;
      }

      pos.x = MATH_CLAMP( pos.x, CURSOR_MOVE_RECT_X_MIN, CURSOR_MOVE_RECT_X_MAX );
      pos.y = MATH_CLAMP( pos.y, CURSOR_MOVE_RECT_Y_MIN, CURSOR_MOVE_RECT_Y_MAX );
      
      GFL_CLACT_WK_SetPos( work->obj_clwk[OBJ_CURSOR], &pos, CLSYS_DEFREND_MAIN );
    
      // ヒット判定
      work->select_data_idx = Zukan_Detail_Map_Hit( param, work, cmn, (u8)pos.x, (u8)pos.y );

      if( work->select_data_idx != select_data_idx_prev )
      {
        // 場所更新
        Zukan_Detail_Map_ChangePlace( param, work, cmn );
        // 草原、水、釣りのアイコンの明るさを変更する
        Zukan_Detail_Map_UtilBrightenPlaceIcon( param, work, cmn );

        if( work->select_data_idx != DATA_IDX_NONE )
        {
          PMSND_PlaySE( SEQ_SE_SELECT1 );
        }
      }
    }
  }
  // タッチ入力で選択操作
  else
  {
    if( GFL_UI_TP_GetPointTrg( &x, &y ) )  // タッチ入力中から変更なしなのでGFL_UI_SetTouchOrKeyは呼ばなくていい
    {
      // ヒット判定
      work->select_data_idx = Zukan_Detail_Map_Hit( param, work, cmn, (u8)x, (u8)y );

      // 場所更新
      Zukan_Detail_Map_ChangePlace( param, work, cmn );
      // 草原、水、釣りのアイコンの明るさを変更する
      Zukan_Detail_Map_UtilBrightenPlaceIcon( param, work, cmn );

      if( work->select_data_idx != DATA_IDX_NONE )
      {
        PMSND_PlaySE( SEQ_SE_DECIDE1 );
      }
    }
  }
}

//-------------------------------------
/// 状態を遷移させる
//=====================================
static void Zukan_Detail_Map_StateMain( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 自動的に状態を遷移させる必要があるときは、状態を遷移させる
  switch(work->state)
  {
  case STATE_TOP:
    {
      // 何もしない
    }
    break;
  case STATE_TOP_TO_SELECT:
    {
      Zukan_Detail_Map_ChangeState( param, work, cmn, STATE_TOP_TO_SELECT_TOUCHBAR_CHANGE );
    }
    break;
  case STATE_TOP_TO_SELECT_TOUCHBAR_CHANGE:
    {
      Zukan_Detail_Map_ChangeState( param, work, cmn, STATE_SELECT );
    }
    break;
  case STATE_TOP_DRAW_SEASON_AREA:
    {
      switch(work->draw_season_area_step)
      {
      case DRAW_SEASON_AREA_STEP_OBJ_OFF:
        {
          Zukan_Detail_Map_UtilDrawSeasonAreaObjOff( param, work, cmn );
          work->draw_season_area_step = DRAW_SEASON_AREA_STEP_ALPHA_SET;
        }
        break;
      case DRAW_SEASON_AREA_STEP_ALPHA_SET:
        {
          Zukan_Detail_Map_UtilDrawSeasonAreaAlphaSet( param, work, cmn );
          work->draw_season_area_step = DRAW_SEASON_AREA_STEP_OBJ_ON;
        }
        break;
      case DRAW_SEASON_AREA_STEP_OBJ_ON:
        {
          Zukan_Detail_Map_UtilDrawSeasonAreaObjOn( param, work, cmn );
          work->draw_season_area_step = DRAW_SEASON_AREA_STEP_NONE;
          Zukan_Detail_Map_ChangeState( param, work, cmn, STATE_TOP );
        }
        break;
      }
    }
    break;
  case STATE_TOP_BEFORE_FADE_OUT:
    {
      switch(work->draw_season_area_step)
      {
      case DRAW_SEASON_AREA_STEP_OBJ_OFF:
        {
          Zukan_Detail_Map_UtilDrawSeasonAreaObjOffCompleteOff( param, work, cmn );
          work->draw_season_area_step = DRAW_SEASON_AREA_STEP_ALPHA_SET;
        }
        break;
      case DRAW_SEASON_AREA_STEP_ALPHA_SET:
        {
          Zukan_Detail_Map_UtilDrawSeasonAreaAlphaSetCompleteOff( param, work, cmn );
          work->draw_season_area_step = DRAW_SEASON_AREA_STEP_OBJ_ON;
        }
        break;
      case DRAW_SEASON_AREA_STEP_OBJ_ON:
        {
          Zukan_Detail_Map_UtilDrawSeasonAreaObjOnCompleteOff( param, work, cmn );
          work->draw_season_area_step = DRAW_SEASON_AREA_STEP_NONE;  // Zukan_Detail_Map_ChangeStateせずに、終わっていい
        }
        break;
      }
    }
    break;
  case STATE_SELECT:
    {
      // 何もしない
    }
    break;
  case STATE_SELECT_TO_TOP:
    {
      Zukan_Detail_Map_ChangeState( param, work, cmn, STATE_SELECT_TO_TOP_TOUCHBAR_CHANGE );
    }
    break;
  case STATE_SELECT_TO_TOP_TOUCHBAR_CHANGE:
    {
      Zukan_Detail_Map_ChangeState( param, work, cmn, STATE_TOP );
    }
    break;
  case STATE_SELECT_DRAW_SEASON_AREA:
    {
      switch(work->draw_season_area_step)
      {
      case DRAW_SEASON_AREA_STEP_OBJ_OFF:
        {
          Zukan_Detail_Map_UtilDrawSeasonAreaObjOff( param, work, cmn );
          work->draw_season_area_step = DRAW_SEASON_AREA_STEP_ALPHA_SET;
        }
        break;
      case DRAW_SEASON_AREA_STEP_ALPHA_SET:
        {
          Zukan_Detail_Map_UtilDrawSeasonAreaAlphaSet( param, work, cmn );
          work->draw_season_area_step = DRAW_SEASON_AREA_STEP_OBJ_ON;
        }
        break;
      case DRAW_SEASON_AREA_STEP_OBJ_ON:
        {
          Zukan_Detail_Map_UtilDrawSeasonAreaObjOn( param, work, cmn );
          work->draw_season_area_step = DRAW_SEASON_AREA_STEP_NONE;
          Zukan_Detail_Map_ChangeState( param, work, cmn, STATE_SELECT );
        }
        break;
      }
    }
    break;
  }
}

static void Zukan_Detail_Map_ChangeState( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                 STATE state )
{
  // 遷移前のSTATE = work->state
  // 遷移後のSTATE = state 
  
  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
 
  // 遷移前の状態 
  switch(work->state)
  {
  case STATE_TOP:
    {
      if( state == STATE_TOP_TO_SELECT )  // 遷移後の状態
      {
        // 何もしない
      }
      else if( state == STATE_TOP_DRAW_SEASON_AREA )  // 遷移後の状態
      {
        work->draw_season_area_step = DRAW_SEASON_AREA_STEP_OBJ_OFF;
      }
      else if( state == STATE_TOP_BEFORE_FADE_OUT )  // 遷移後の状態
      {
        work->draw_season_area_step = DRAW_SEASON_AREA_STEP_OBJ_OFF;
      }
    }
    break;
  case STATE_TOP_TO_SELECT:
    {
      if( state == STATE_TOP_TO_SELECT_TOUCHBAR_CHANGE )  // 遷移後の状態
      {
        // タッチバー
        ZUKAN_DETAIL_TOUCHBAR_SetVisibleAll( touchbar, FALSE );
      }
    }
    break;
  case STATE_TOP_TO_SELECT_TOUCHBAR_CHANGE:
    {
      if( state == STATE_SELECT )  // 遷移後の状態
      {
        work->scroll_state             = SCROLL_STATE_DOWN_TO_UP;
        work->pokeicon_scroll_state    = SCROLL_STATE_DOWN_TO_UP;

        // カーソルを表示する
        if( work->habitat == HABITAT_KNOWN )
        {
          //if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
          if( work->top_to_select_ktst == GFL_APP_KTST_KEY )
          {
            Zukan_Detail_Map_SetCursorOnPlayer( param, work, cmn );
          }
          else
          {
            {
              // ヒット判定
              work->select_data_idx = Zukan_Detail_Map_Hit( param, work, cmn, work->top_to_select_x, work->top_to_select_y );
              if( work->select_data_idx != DATA_IDX_NONE )
              {
                // リングを表示する
                GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_RING_CUR], TRUE );

                //PMSND_PlaySE( SEQ_SE_DECIDE1 );  SEは鳴らさない、状態をTOPからSELECTにする際に鳴らしている音と被るので。
              }
            }
          }
          Zukan_Detail_Map_ChangePlace( param, work, cmn );
          // 草原、水、釣りのアイコンの明るさを変更する
          Zukan_Detail_Map_UtilBrightenPlaceIcon( param, work, cmn );

          // プレイヤーの位置にOBJを表示
          //Zukan_Detail_Map_DrawPlayer( param, work, cmn, TRUE );  // 生息地が判明しているときは、TOPでもSELECTでも表示しておく。
        }

        // タッチバー
        ZUKAN_DETAIL_TOUCHBAR_SetType(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_TYPE_MAP,
            ZUKAN_DETAIL_TOUCHBAR_DISP_MAP,
            (ZKNDTL_COMMON_GetPokeNum(cmn)>1)?TRUE:FALSE ); 
      }
    }
    break;
  case STATE_TOP_DRAW_SEASON_AREA:
    {
      if( state == STATE_TOP )  // 遷移後の状態
      {
#ifndef DEF_SCMD_CHANGE
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
#else
        if( work->change_poke_by_cur_u_d_flag )
        {
          work->change_poke_by_cur_u_d_change_poke_finish = TRUE;
          if( work->change_poke_by_cur_u_d_cur_u_d_finish )
          {
            work->change_poke_by_cur_u_d_flag = FALSE;
            ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
          }
        }
#endif

        if( work->push_button == OBJ_PUSH_BUTTON_NONE )  // 矢印アニメの点滅の終了とOBJの表示OFFから表示ONまでの一連の流れの終了とどちらが先か分からないので。
        {
          if( work->draw_season_area_set_user_active_whole )
            ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, TRUE );
        }
      }
    }
    break;
  case STATE_TOP_BEFORE_FADE_OUT:
    {
      // この状態のときに、stateを変更する要求をかけることはない
    }
    break;
  case STATE_SELECT:
    {
      if( state == STATE_SELECT_TO_TOP )  // 遷移後の状態
      {
        // 何もしない
      }
      else if( state == STATE_SELECT_DRAW_SEASON_AREA )  // 遷移後の状態
      {
        work->draw_season_area_step = DRAW_SEASON_AREA_STEP_OBJ_OFF;
      }
    }
    break;
  case STATE_SELECT_TO_TOP:
    {
      if( state == STATE_SELECT_TO_TOP_TOUCHBAR_CHANGE )  // 遷移後の状態
      {
        // タッチバー
        ZUKAN_DETAIL_TOUCHBAR_SetVisibleAll( touchbar, FALSE );
      }
    }
    break;
  case STATE_SELECT_TO_TOP_TOUCHBAR_CHANGE:
    {
      if( state == STATE_TOP )  // 遷移後の状態
      {
        work->scroll_state             = SCROLL_STATE_UP_TO_DOWN;
        work->pokeicon_scroll_state    = SCROLL_STATE_UP_TO_DOWN;
       
        // カーソルを非表示にする
        {
          GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_CURSOR], FALSE );
        }
        // リングを非表示にする
        {
          GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_RING_CUR], FALSE );
        }

        // プレイヤーの位置のOBJを非表示
        //Zukan_Detail_Map_DrawPlayer( param, work, cmn, FALSE );  // 生息地が判明しているときは、TOPでもSELECTでも表示しておく。

        // タッチバー
        ZUKAN_DETAIL_TOUCHBAR_SetType(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
            ZUKAN_DETAIL_TOUCHBAR_DISP_MAP,
            (ZKNDTL_COMMON_GetPokeNum(cmn)>1)?TRUE:FALSE ); 
        {
          GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
          ZUKAN_DETAIL_TOUCHBAR_SetCheckFlipOfGeneral(
              touchbar,
              GAMEDATA_GetShortCut( gamedata, SHORTCUT_ID_ZUKAN_MAP ) );
        }
      }
    }
    break;
  case STATE_SELECT_DRAW_SEASON_AREA:
    {
      if( state == STATE_SELECT )  // 遷移後の状態
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
        if( work->push_button == OBJ_PUSH_BUTTON_NONE )  // 矢印アニメの点滅の終了とOBJの表示OFFから表示ONまでの一連の流れの終了とどちらが先か分からないので。
        {
          if( work->draw_season_area_set_user_active_whole )
            ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, TRUE );
        }
      }
    }
    break;
  }

  // 遷移後の状態
  work->state = state;
}

static void Zukan_Detail_Map_ChangePoke( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 変更前のポケモン = 取得不可
  // 変更後のポケモン = ZKNDTL_COMMON_GetCurrPoke(cmn)

  // ポケモンを変更したとき

  // 変更前の情報を覚えておく
  APPEAR_RULE prev_appear_rule = work->appear_rule;
  HABITAT     prev_habitat     = work->habitat;

  // 変更後のポケモン
  u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);

  Zukan_Detail_Map_AreaDataUnload( work->area_data );
  work->area_data = Zukan_Detail_Map_AreaDataLoad( monsno, param->heap_id, work->area_handle );

  work->appear_rule  = ( work->area_data->year == 0 )?( APPEAR_RULE_SEASON ):( APPEAR_RULE_YEAR );
  if( work->appear_rule == APPEAR_RULE_YEAR )
  {
    work->habitat      = ( work->area_data->season_data[PMSEASON_SPRING].unknown == 0 )?( HABITAT_KNOWN ):( HABITAT_UNKNOWN );  // 一年中同じときは春のデータしか見ない
  }
  else
  {
    work->habitat      = ( work->area_data->season_data[work->season_id].unknown == 0 )?( HABITAT_KNOWN ):( HABITAT_UNKNOWN );
  }

  // ポケアイコン
  {
    // 前のの位置を次のにも反映する
    u8 x, y;
/* いったん下に下げないとポケモンを変更できないので、下の位置を渡せばよい。
    if( work->pokeicon_clwk )
    {
      GFL_CLACTPOS pos;
      GFL_CLACT_WK_GetPos( work->pokeicon_clwk, &pos, CLSYS_DEFREND_SUB );
      x = (u8)(pos.x);
      y = (u8)(pos.y);
    }
    else
*/
    {
      x = pokeicon_pos[0];
      y = pokeicon_pos[1];
    }

    // 今表示しているものを非表示にし、次のものを表示するようスワップを入れ替える
    {
      // 今表示しているものを非表示にする
      if( work->pokeicon_clwk[work->pokeicon_swap_curr] )
      {
        GFL_CLACT_WK_SetDrawEnable( work->pokeicon_clwk[work->pokeicon_swap_curr], FALSE );
      }
      // 次のものを表示するようスワップを入れ替える
      work->pokeicon_swap_curr = ( work->pokeicon_swap_curr +1 ) %OBJ_SWAP_MAX;
    }
    // 前のを破棄
    if( work->pokeicon_clwk[work->pokeicon_swap_curr] )
    {
      // NULLチェックはマクロ内で行っている
      BLOCK_POKEICON_EXIT( work->pokeicon_ncg[work->pokeicon_swap_curr], work->pokeicon_clwk[work->pokeicon_swap_curr] )
    } 
    // 次のを生成
    {
      u32 sex;
      BOOL rare;
      u32 form;

      GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
      ZUKAN_SAVEDATA* zkn_sv = GAMEDATA_GetZukanSave( gamedata );
      ZUKANSAVE_GetDrawData( zkn_sv, monsno, &sex, &rare, &form, param->heap_id );

      work->pokeicon_clwk[work->pokeicon_swap_curr] = PokeiconInit(
                                                          &(work->pokeicon_ncg[work->pokeicon_swap_curr]),
                                                          work->pokeicon_ncl,
                                                          work->pokeicon_nce,
                                                          work->clunit,
                                                          param->heap_id,
                                                          monsno,
                                                          form,
                                                          sex,
                                                          FALSE,
                                                          x,
                                                          y );
    }
  }

  // ポケモン名のぶんぷ
  // 前のをクリア
  work->text_trans[TEXT_POKENAME] = FALSE;
  PRINTSYS_QUE_Clear( work->print_que[TEXT_POKENAME] );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_POKENAME]), 0 );
  //GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_POKENAME] );  // ちらつくのでコメントアウト
  // 次のを表示 
  {
    WORDSET* wordset;
    STRBUF* src_strbuf;
    STRBUF* strbuf;
    wordset = WORDSET_Create( param->heap_id );
    src_strbuf = GFL_MSG_CreateString( work->msgdata[MSG_ZUKAN], ZKN_MAP_TEXT_01 );
    strbuf = GFL_STR_CreateBuffer( STRBUF_POKENAME_LENGTH, param->heap_id );
    WORDSET_RegisterPokeMonsNameNo( wordset, 0, monsno );
    WORDSET_ExpandStr( wordset, strbuf, src_strbuf );
    GFL_STR_DeleteBuffer( src_strbuf );
    WORDSET_Delete( wordset );
    PRINTSYS_PrintQueColor( work->print_que[TEXT_POKENAME], GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_POKENAME]),
                            0, 5, strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
    GFL_STR_DeleteBuffer( strbuf );
    work->text_trans[TEXT_POKENAME] = TRUE;

    // 既に済んでいるかもしれないので、Mainを1度呼んでおく
    Zukan_Detail_Map_TextMain( param, work, cmn, TEXT_POKENAME );
  }

  // 季節表示
  // 次のを表示
  // 1年中同じ分布のとき
  if( work->appear_rule == APPEAR_RULE_YEAR )
  {
    if( prev_appear_rule == APPEAR_RULE_SEASON )
    {
      // 前のをクリア
      work->text_trans[TEXT_SEASON] = FALSE;
      PRINTSYS_QUE_Clear( work->print_que[TEXT_SEASON] );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_SEASON]), 0 );
      GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_SEASON] );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_SEASON], FALSE );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_SEASON_R], FALSE );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_SEASON_L], FALSE );
    }
  }
  // 季節ごとに分布が変わるとき
  else
  {
    // 次のを表示
    if( prev_appear_rule == APPEAR_RULE_YEAR )
    {
      Zukan_Detail_Map_UtilPrintSeason( param, work, cmn );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_SEASON], TRUE );

      GFL_CLACT_WK_SetAnmSeq( work->obj_clwk[OBJ_SEASON_R], SEASON_R_ARROW_ANMSEQ_NORMAL );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_SEASON_R], TRUE );
      GFL_CLACT_WK_SetAnmSeq( work->obj_clwk[OBJ_SEASON_L], SEASON_L_ARROW_ANMSEQ_NORMAL );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_SEASON_L], TRUE );
    }
  }

/*
Map画面に切り替わった直後はまだ生息地を表示したくないので、Zukan_Detail_Map_DrawSeasonAreaAfterChangePoke関数を呼び出し元から呼んでもらうことにした。
  // 今の季節の生息地を表示する
  Zukan_Detail_Map_UtilDrawSeasonArea( param, work, cmn );

  // 草原、水、釣りのアイコンの明るさを変更する
  Zukan_Detail_Map_UtilBrightenPlaceIcon( param, work, cmn );
*/
}

// Map画面に切り替わった直後はまだ生息地を表示したくないので、このような生息地表示専門の関数を用意した。
static void Zukan_Detail_Map_DrawSeasonAreaAfterChangePoke( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 今の季節の生息地を表示する
  Zukan_Detail_Map_UtilDrawSeasonArea( param, work, cmn );

  // 草原、水、釣りのアイコンの明るさを変更する
  Zukan_Detail_Map_UtilBrightenPlaceIcon( param, work, cmn );
}

// Map画面から切り替わる直前に生息地を非表示にしたいので、このような生息地非表示専門の関数を用意した。
static void Zukan_Detail_Map_OffSeasonAreaBeforeFadeOut( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
#if 0
  BOOL b_disp_unknown = GFL_CLACT_WK_GetDrawEnable( work->obj_clwk[OBJ_UNKNOWN] );  // 今の今まで生息地不明を表示しているとき、b_disp_unknownはTRUEになる

  u8 i;

  if( b_disp_unknown )  // 生息地不明を表示していたとき
  {
    //Zukan_Detail_Map_ObjUnknownAlphaExit( work );
    work->vblank_req = VBLANK_REQ_OBJ_UNKNOWN_ALPHA_EXIT;

    work->text_trans[TEXT_UNKNOWN] = FALSE;
    PRINTSYS_QUE_Clear( work->print_que[TEXT_UNKNOWN] );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_UNKNOWN]), 0 );
    GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_UNKNOWN] );
    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_UNKNOWN], FALSE );
  }
  else  // 生息地が判明していたので、obj_exist_clwkを表示していたとき
  {
    //Zukan_Detail_Map_ObjExistAlphaExit( work );
    work->vblank_req = VBLANK_REQ_OBJ_EXIST_ALPHA_EXIT;

    for( i=0; i<TOWNMAP_DATA_MAX; i++ )
    {
      GFL_CLACT_WK_SetDrawEnable( work->obj_exist_clwk[i], FALSE );
    }
  }
#endif

  // 今は( work->state == STATE_TOP )である。
  Zukan_Detail_Map_ChangeState( param, work, cmn, STATE_TOP_BEFORE_FADE_OUT );
}

static void Zukan_Detail_Map_ChangePlace( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 変更前の場所 = 取得不可
  // 変更後の場所 = work->select_data_idx
  
  // 場所を変更したとき
  
  // 前のをクリア
  work->text_trans[TEXT_PLACENAME] = FALSE;
  PRINTSYS_QUE_Clear( work->print_que[TEXT_PLACENAME] );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_PLACENAME]), 0 );

  // 次のを表示
  if( work->select_data_idx != DATA_IDX_NONE )
  {
    // 場所名を表示するとき
    STRBUF* strbuf;
		u16 zone_id = TOWNMAP_DATA_GetParam( work->townmap_data, work->select_data_idx, TOWNMAP_DATA_PARAM_ZONE_ID );
		GF_ASSERT( zone_id != TOWNMAP_DATA_ERROR );
    strbuf = GFL_MSG_CreateString( work->msgdata[MSG_PLACE], ZONEDATA_GetPlaceNameID( zone_id ) );
    PRINTSYS_PrintQueColor( work->print_que[TEXT_PLACENAME], GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_PLACENAME]),
                            0, 1, strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
    GFL_STR_DeleteBuffer( strbuf );
    work->text_trans[TEXT_PLACENAME] = TRUE;

    // 既に済んでいるかもしれないので、Mainを1度呼んでおく
    Zukan_Detail_Map_TextMain( param, work, cmn, TEXT_PLACENAME );

    {
      GFL_CLACTPOS pos;
      pos.x = (s16)TOWNMAP_DATA_GetParam( work->townmap_data, work->select_data_idx, TOWNMAP_DATA_PARAM_POS_X );
      pos.y = (s16)TOWNMAP_DATA_GetParam( work->townmap_data, work->select_data_idx, TOWNMAP_DATA_PARAM_POS_Y );
      GFL_CLACT_WK_SetAnmSeq( work->obj_clwk[OBJ_RING_CUR], RING_CUR_ANMSEQ );
      GFL_CLACT_WK_SetPos( work->obj_clwk[OBJ_RING_CUR], &pos, CLSYS_DEFREND_MAIN );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_RING_CUR], TRUE );
    }
  }
  else
  {
    // 場所名を表示しないとき
    GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_PLACENAME] );  // クリア状態を転送して何も表示されないようにしておく
    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_RING_CUR], FALSE );
  }
}

static void Zukan_Detail_Map_ChangeSeason( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn, BOOL b_next, int ktst )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
 
  u16 push_anmseq;
  u8  pair_button;  // アニメーションを合わせておく必要があるボタン

  // 変更前の季節 = 取得不可
  // 変更後の季節 = work->season_id

  // 季節を変更したとき

  // 変更前の情報を覚えておく
  HABITAT     prev_habitat     = work->habitat;

  // 1年中同じ分布のときは、この関数は呼び出されない

  // 変更後の季節
  if( work->appear_rule == APPEAR_RULE_YEAR )
  {
    // 1年中同じ分布のときは、この関数は呼び出されないはずなので、このif分岐内には来ないはず
    work->habitat      = ( work->area_data->season_data[PMSEASON_SPRING].unknown == 0 )?( HABITAT_KNOWN ):( HABITAT_UNKNOWN );  // 一年中同じときは春のデータしか見ない
  }
  else
  {
    work->habitat      = ( work->area_data->season_data[work->season_id].unknown == 0 )?( HABITAT_KNOWN ):( HABITAT_UNKNOWN );
  }

  // 季節表示
  // 前のをクリア
  work->text_trans[TEXT_SEASON] = FALSE;
  PRINTSYS_QUE_Clear( work->print_que[TEXT_SEASON] );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_SEASON]), 0 );
  //GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_SEASON] );  // ちらつくのでコメントアウト

  // 次のを表示
  Zukan_Detail_Map_UtilPrintSeason( param, work, cmn );
 
  if( b_next )
  {
    work->push_button = OBJ_SEASON_R;
    push_anmseq       = SEASON_R_ARROW_ANMSEQ_SELECT;
    pair_button       = OBJ_SEASON_L; 
  }
  else
  {
    work->push_button = OBJ_SEASON_L;
    push_anmseq       = SEASON_L_ARROW_ANMSEQ_SELECT;
    pair_button       = OBJ_SEASON_R; 
  }
  GFL_CLACT_WK_SetAnmSeq( work->obj_clwk[work->push_button], push_anmseq );
  GFL_CLACT_WK_SetAutoAnmFlag( work->obj_clwk[pair_button], FALSE );
  GFL_CLACT_WK_SetAnmFrame( work->obj_clwk[pair_button], 0 );
  ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, FALSE );

  // タッチ入力されてここに来た場合は、カーソルを非表示にする
  if( ktst == GFL_APP_KTST_TOUCH )
  {
    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_CURSOR], FALSE );
  }
  // キー入力されてここに来た場合は、カーソルを現在の位置 or プレイヤーの位置に表示する
  else
  {
    if(    work->state == STATE_SELECT       // マップの拠点を選べる状態でなければ、カーソルは表示しなくていい
        && work->habitat == HABITAT_KNOWN )  // 生息地が不明のときは、カーソルは表示しなくていい
    {
      Zukan_Detail_Map_SetCursor( param, work, cmn );

      // 場所更新
      Zukan_Detail_Map_ChangePlace( param, work, cmn );
    }
  }

  // 今の季節の生息地を表示する
  Zukan_Detail_Map_UtilDrawSeasonArea( param, work, cmn );

  // 草原、水、釣りのアイコンの明るさを変更する
  Zukan_Detail_Map_UtilBrightenPlaceIcon( param, work, cmn );
}

//-------------------------------------
/// 分布データ
//=====================================
static AREA_DATA* Zukan_Detail_Map_AreaDataLoad( u16 monsno, HEAPID heap_id, ARCHANDLE* handle )
{
//#if 0
  // 本データ
  u32 size;
  //AREA_DATA* area_data = GFL_ARCHDL_UTIL_LoadEx( handle, NARC_zukan_area_w_zkn_area_w_monsno_001_dat + monsno -1, FALSE, heap_id, &size );
  //AREA_DATA* area_data = GFL_ARCHDL_UTIL_LoadEx( handle, NARC_zukan_area_b_zkn_area_b_monsno_001_dat + monsno -1, FALSE, heap_id, &size );
  AREA_DATA* area_data = GFL_ARCHDL_UTIL_LoadEx( handle, monsno -1, FALSE, heap_id, &size );
  GF_ASSERT_MSG( sizeof(AREA_DATA) == size, "ZUKAN_DETAIL_MAP : AREA_DATAのサイズが異なります。\n" );
  return area_data;
//#endif

#if 0
  // 仮データ
  AREA_DATA* area_data = GFL_HEAP_AllocClearMemory( heap_id, sizeof(AREA_DATA) );
  s32 case_no = monsno %3;
  if( monsno == 517 ) case_no = 0;
  if( monsno == 534 ) case_no = 1;
  if( monsno == 569 ) case_no = 2;
  switch( case_no )
  {
  case 0:
    {
      area_data->year = 0;
      area_data->season_data[PMSEASON_SPRING].unknown = 0;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[0] = PLACE_BITFLAG_GROUND_L | PLACE_BITFLAG_WATER | PLACE_BITFLAG_FISHING;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[1] = PLACE_BITFLAG_GROUND_L;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[2] = PLACE_BITFLAG_WATER;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[3] = PLACE_BITFLAG_FISHING;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[4] = PLACE_BITFLAG_WATER | PLACE_BITFLAG_FISHING;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[5] = PLACE_BITFLAG_GROUND_L | PLACE_BITFLAG_FISHING;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[6] = PLACE_BITFLAG_GROUND_L | PLACE_BITFLAG_WATER;
      area_data->season_data[PMSEASON_SUMMER].unknown = 0;
      area_data->season_data[PMSEASON_AUTUMN].unknown = 1;
    }
    break;
  case 1:
    {
      area_data->year = 1;
      area_data->season_data[PMSEASON_SPRING].unknown = 0;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[0] = PLACE_BITFLAG_GROUND_L | PLACE_BITFLAG_WATER | PLACE_BITFLAG_FISHING;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[1] = PLACE_BITFLAG_GROUND_L;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[2] = PLACE_BITFLAG_WATER;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[3] = PLACE_BITFLAG_FISHING;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[4] = PLACE_BITFLAG_WATER | PLACE_BITFLAG_FISHING;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[5] = PLACE_BITFLAG_GROUND_L | PLACE_BITFLAG_FISHING;
      area_data->season_data[PMSEASON_SPRING].place_bitflag[6] = PLACE_BITFLAG_GROUND_L | PLACE_BITFLAG_WATER;
    }
    break;
  case 2:
    {
      area_data->year = 1;
      area_data->season_data[PMSEASON_SPRING].unknown = 1;
    }
    break;
  }
  return area_data;
#endif


#ifdef DEBUG_SET_ANIME_AND_POS
  // 生息する場所に置くOBJのアニメ指定、位置指定用のデータ
  AREA_DATA* area_data = GFL_HEAP_AllocClearMemory( heap_id, sizeof(AREA_DATA) );
  u8 i;
  {
    area_data->year = 1;
    area_data->season_data[PMSEASON_SPRING].unknown = 0;

    for( i=0; i<TOWNMAP_DATA_MAX; i++ )
    {
      area_data->season_data[PMSEASON_SPRING].place_bitflag[i] = PLACE_BITFLAG_GROUND_ALL | PLACE_BITFLAG_WATER_ALL | PLACE_BITFLAG_FISHING_ALL;
    }
  }
  return area_data;
#endif


}
static void Zukan_Detail_Map_AreaDataUnload( AREA_DATA* area_data )
{
  if( area_data ) GFL_HEAP_FreeMemory( area_data );
  area_data = NULL;
}

//-------------------------------------
/// 何度も出てくる処理
//=====================================
static void Zukan_Detail_Map_UtilPrintSeason( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  STRBUF* strbuf;
  u16     str_width;
  u16     bmp_width;
  u16     x;
  strbuf = GFL_MSG_CreateString( work->msgdata[MSG_ZUKAN], ZKN_MAP_SPRING + work->season_id );
  str_width = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
  bmp_width = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_SEASON]) );
  x = ( bmp_width - str_width ) / 2;
  PRINTSYS_PrintQueColor( work->print_que[TEXT_SEASON], GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_SEASON]),
                          x, 1, strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
  GFL_STR_DeleteBuffer( strbuf );
  work->text_trans[TEXT_SEASON] = TRUE;

  // 既に済んでいるかもしれないので、Mainを1度呼んでおく
  Zukan_Detail_Map_TextMain( param, work, cmn, TEXT_SEASON );
}

static void Zukan_Detail_Map_UtilDrawSeasonArea( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
#if 0
  BOOL b_disp_unknown = GFL_CLACT_WK_GetDrawEnable( work->obj_clwk[OBJ_UNKNOWN] );  // 今の今まで生息地不明を表示しているとき、b_disp_unknownはTRUEになる

  u8 i;

  // 今の今まで生息地判明していて、表示しているかもしれないので、一度全部消しておく。
  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
  {
    GFL_CLACT_WK_SetDrawEnable( work->obj_exist_clwk[i], FALSE );
  }

  // これからの季節の生息地を表示する
  
  // これからの季節の生息地が不明のとき
  if( work->habitat == HABITAT_UNKNOWN )
  {
    if( !b_disp_unknown )
    {
      STRBUF* strbuf;
      u16     str_width;
      u16     bmp_width;
      u16     x;
      strbuf = GFL_MSG_CreateString( work->msgdata[MSG_ZUKAN], ZNK_RANGE_00 );
      str_width = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
      bmp_width = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_UNKNOWN]) );
      x = ( bmp_width - str_width ) / 2;
      PRINTSYS_PrintQueColor( work->print_que[TEXT_UNKNOWN], GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_UNKNOWN]),
                            x, 1, strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
      GFL_STR_DeleteBuffer( strbuf );
      work->text_trans[TEXT_UNKNOWN] = TRUE;

      // 既に済んでいるかもしれないので、Mainを1度呼んでおく
      Zukan_Detail_Map_TextMain( param, work, cmn, TEXT_UNKNOWN );

      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_UNKNOWN], TRUE );

      //Zukan_Detail_Map_ObjExistAlphaExit( work ); 
      //Zukan_Detail_Map_ObjUnknownAlphaInit( work ); 
      work->vblank_req = VBLANK_REQ_OBJ_EXIST_ALPHA_EXIT_AND_OBJ_UNKNOWN_ALPHA_INIT;
    } 
  }
  // これからの季節の生息地が判明しているとき
  else
  {
    u8 season_id = ( work->appear_rule == APPEAR_RULE_YEAR )?(PMSEASON_SPRING):(work->season_id);  // 一年中同じときは春のデータしか見ない
  
    // 前の生息地不明をクリア
    if( b_disp_unknown )
    {
      work->text_trans[TEXT_UNKNOWN] = FALSE;
      PRINTSYS_QUE_Clear( work->print_que[TEXT_UNKNOWN] );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_UNKNOWN]), 0 );
      GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_UNKNOWN] );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_UNKNOWN], FALSE );
      
      //Zukan_Detail_Map_ObjUnknownAlphaExit( work );
    }

    // 生息場所にOBJを表示する
    for( i=0; i<TOWNMAP_DATA_MAX; i++ )
    {
      u8 zkn_area_idx = work->townmap_data_idx_to_zkn_area_idx[i];
      if( work->area_data->season_data[season_id].place_bitflag[zkn_area_idx] != PLACE_BITFLAG_NONE )
      {
        if( work->hide_spot[i].state != HIDE_STATE_HIDE_TRUE )  // 隠しマップなら絶対にあたらない
        {
          GFL_CLACT_WK_SetDrawEnable( work->obj_exist_clwk[i], TRUE );
        }
      }
    }

    //Zukan_Detail_Map_ObjExistAlphaInit( work );
    work->vblank_req = VBLANK_REQ_OBJ_UNKNOWN_ALPHA_EXIT_AND_OBJ_EXIST_ALPHA_INIT;
  }
#endif
 
  if( work->state == STATE_TOP )
  {
    Zukan_Detail_Map_ChangeState( param, work, cmn, STATE_TOP_DRAW_SEASON_AREA );
  }
  else if( work->state == STATE_SELECT )
  {
    Zukan_Detail_Map_ChangeState( param, work, cmn, STATE_SELECT_DRAW_SEASON_AREA );
  }
}

static void Zukan_Detail_Map_UtilDrawSeasonAreaObjOff( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  BOOL b_disp_unknown = GFL_CLACT_WK_GetDrawEnable( work->obj_clwk[OBJ_UNKNOWN] );  // 今の今まで生息地不明を表示しているとき、b_disp_unknownはTRUEになる
 
  // 今の今まで生息地不明を表示している
  if( b_disp_unknown )
  {
    // これからの季節の生息地が判明しているとき
    if( work->habitat == HABITAT_KNOWN )
    {
      work->text_trans[TEXT_UNKNOWN] = FALSE;
      PRINTSYS_QUE_Clear( work->print_que[TEXT_UNKNOWN] );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_UNKNOWN]), 0 );
      GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_UNKNOWN] );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_UNKNOWN], FALSE );
    }
  }
  // 今の今まで生息地判明しているとき
  else
  {
    u8 i;
    for( i=0; i<TOWNMAP_DATA_MAX; i++ )  // 生息地は、ポケモン、季節によって変わるので、毎回1度消す。
    {
      GFL_CLACT_WK_SetDrawEnable( work->obj_exist_clwk[i], FALSE );
    }

    // これからの季節の生息地が不明のとき 
    if( work->habitat == HABITAT_UNKNOWN )
    {
      Zukan_Detail_Map_DrawPlayer( param, work, cmn, FALSE );
  
      // カーソルもリングも非表示にするので、どこの拠点も選択していない状態にしておく
      work->select_data_idx = DATA_IDX_NONE;

      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_CURSOR], FALSE );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_RING_CUR], FALSE );
      Zukan_Detail_Map_ChangePlace( param, work, cmn );
      // 草原、水、釣りのアイコンの明るさを変更する
      Zukan_Detail_Map_UtilBrightenPlaceIcon( param, work, cmn );
    }
  }
}
static void Zukan_Detail_Map_UtilDrawSeasonAreaAlphaSet( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  BOOL b_disp_unknown = GFL_CLACT_WK_GetDrawEnable( work->obj_clwk[OBJ_UNKNOWN] );  // 生息地不明が表示されたままのとき、b_disp_unknownはTRUEになる

  // 生息地不明を表示している(即ち、今の今までも、これからも、生息地不明のまま変わらないとき)
  if( b_disp_unknown )
  {
    // 生息地不明のまま変わらないので、何もしなくてよい
  }
  // 生息地判明しているとき or 「今の今までは生息地が判明していたが、これからは生息地が不明になるとき」
  else
  {
    // 初めてMapに来たときは、必ずここを通るように、obj_clwk[OBJ_UNKNOWN]を非表示で初期化してある。

    // これからの季節の生息地が不明のとき
    if( work->habitat == HABITAT_UNKNOWN )
    {
/*
      Zukan_Detail_Map_ObjExistAlphaExit( work );
      Zukan_Detail_Map_ObjUnknownAlphaInit( work );
      // Zukan_Detail_Map_ObjUnknownAlphaInitはすぐにAlphaの設定を反映してくれる。
*/
      work->vblank_req = VBLANK_REQ_OBJ_EXIST_ALPHA_EXIT_AND_OBJ_UNKNOWN_ALPHA_INIT;  // vblankでやっても完全不透明OBJの乱れは直らなかった。
    }
    // これからの季節の生息地が判明しているとき
    else
    {
/*
      Zukan_Detail_Map_ObjUnknownAlphaExit( work );
      Zukan_Detail_Map_ObjExistAlphaInit( work );  // 最初のAlpha設定を適切に行いたいので、Zukan_Detail_Map_ObjExistAlphaResetでは不十分。
      // Zukan_Detail_Map_ObjExistAlphaInitはZukan_Detail_Map_ObjExistAlphaResetですぐにAlphaの設定を反映してくれる。
*/ 
      work->vblank_req = VBLANK_REQ_OBJ_UNKNOWN_ALPHA_EXIT_AND_OBJ_EXIST_ALPHA_INIT;  // vblankでやっても完全不透明OBJの乱れは直らなかった。
    }
  }
}
static void Zukan_Detail_Map_UtilDrawSeasonAreaObjOn( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  BOOL b_disp_unknown = GFL_CLACT_WK_GetDrawEnable( work->obj_clwk[OBJ_UNKNOWN] );  // 生息地不明が表示されたままのとき、b_disp_unknownはTRUEになる

  // これからの季節の生息地が不明のとき
  if( work->habitat == HABITAT_UNKNOWN )
  {
    // 生息地不明を表示していないとき、これから表示する
    if( !b_disp_unknown )
    {
      STRBUF* strbuf;
      u16     str_width;
      u16     bmp_width;
      u16     x;
      strbuf = GFL_MSG_CreateString( work->msgdata[MSG_ZUKAN], ZNK_RANGE_00 );
      str_width = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
      bmp_width = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_UNKNOWN]) );
      x = ( bmp_width - str_width ) / 2;
      PRINTSYS_PrintQueColor( work->print_que[TEXT_UNKNOWN], GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_UNKNOWN]),
                            x, 1, strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
      GFL_STR_DeleteBuffer( strbuf );
      work->text_trans[TEXT_UNKNOWN] = TRUE;

      // 既に済んでいるかもしれないので、Mainを1度呼んでおく
      Zukan_Detail_Map_TextMain( param, work, cmn, TEXT_UNKNOWN );

      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_UNKNOWN], TRUE );
    } 
  }
  // これからの季節の生息地が判明しているとき
  else
  {
    // 生息場所にOBJを表示する
    u8 season_id = ( work->appear_rule == APPEAR_RULE_YEAR )?(PMSEASON_SPRING):(work->season_id);  // 一年中同じときは春のデータしか見ない
    u8 i;
    for( i=0; i<TOWNMAP_DATA_MAX; i++ )
    {
      u8 zkn_area_idx = work->townmap_data_idx_to_zkn_area_idx[i];
      if( work->area_data->season_data[season_id].place_bitflag[zkn_area_idx] != PLACE_BITFLAG_NONE )
      {
        if( work->hide_spot[i].state != HIDE_STATE_HIDE_TRUE )  // 隠しマップなら絶対にあたらない
        {
          GFL_CLACT_WK_SetDrawEnable( work->obj_exist_clwk[i], TRUE );
        }
      }
    }

    //if( work->state == STATE_SELECT_DRAW_SEASON_AREA )  // 生息地が判明しているときは、TOPでもSELECTでも表示しておく。
    {
      // プレイヤーの位置に配置するOBJを現在表示していない場合
      if( !Zukan_Detail_Map_IsDrawPlayer( param, work, cmn ) )
      {
        Zukan_Detail_Map_DrawPlayer( param, work, cmn, TRUE );
      }
    }
  }
}

static void Zukan_Detail_Map_UtilDrawSeasonAreaObjOffCompleteOff( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  BOOL b_disp_unknown = GFL_CLACT_WK_GetDrawEnable( work->obj_clwk[OBJ_UNKNOWN] );  // 今の今まで生息地不明を表示しているとき、b_disp_unknownはTRUEになる
 
  // 今の今まで生息地不明を表示している
  if( b_disp_unknown )
  {
    // とにかく消す
    if( 1 )
    {
      work->text_trans[TEXT_UNKNOWN] = FALSE;
      PRINTSYS_QUE_Clear( work->print_que[TEXT_UNKNOWN] );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_UNKNOWN]), 0 );
      GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_UNKNOWN] );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_UNKNOWN], FALSE );
    }
  }
  // 今の今まで生息地判明しているとき
  else
  {
    u8 i;
    for( i=0; i<TOWNMAP_DATA_MAX; i++ )  // 生息地は、ポケモン、季節によって変わるので、毎回1度消す。
    {
      GFL_CLACT_WK_SetDrawEnable( work->obj_exist_clwk[i], FALSE );
    }
    
    //Zukan_Detail_Map_DrawPlayer( param, work, cmn, FALSE );は呼ばなくてもよい
  }
}
static void Zukan_Detail_Map_UtilDrawSeasonAreaAlphaSetCompleteOff( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
/*
  // 今の今までのAlpha設定がどちらか分からないので、両方呼んでおく
  Zukan_Detail_Map_ObjExistAlphaExit( work );
  Zukan_Detail_Map_ObjUnknownAlphaExit( work );
  ZKNDTL_COMMON_FadeSetColorlessImmediately( ZKNDTL_COMMON_FADE_DISP_M, work->fade_wk_m );  // すぐにAlphaの設定を反映してやらないと、BGとともにこのOBJも暗くしたいというOBJに、前のAlphaが当たってしまう。
*/
  work->vblank_req = VBLANK_REQ_OBJ_ALL_ALPHA_EXIT_AND_FADE_SET_COLORLESS_IMMEDIATELY;  // vblankでやっても完全不透明OBJの乱れは直らなかった。
}
static void Zukan_Detail_Map_UtilDrawSeasonAreaObjOnCompleteOff( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 何もしない
}

static void Zukan_Detail_Map_UtilBrightenPlaceIcon( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 草原、水、釣りのアイコンの明るさを変更する

  u8 i;
  u8 brighten[PLACE_ICON_MAX] = { 0, 0, 0 };

  // 今の季節の生息地が判明しているとき
  if( work->habitat == HABITAT_KNOWN )
  {
    if( work->select_data_idx != DATA_IDX_NONE )
    {
      u8 season_id = ( work->appear_rule == APPEAR_RULE_YEAR )?(PMSEASON_SPRING):(work->season_id);  // 一年中同じときは春のデータしか見ない
      u8 zkn_area_idx = work->townmap_data_idx_to_zkn_area_idx[work->select_data_idx];
      
      if( work->area_data->season_data[season_id].place_bitflag[zkn_area_idx] & PLACE_BITFLAG_GROUND_ALL )
      {
        brighten[PLACE_ICON_GROUND] = 1;
      }
      if( work->area_data->season_data[season_id].place_bitflag[zkn_area_idx] & PLACE_BITFLAG_WATER_ALL )
      {
        brighten[PLACE_ICON_WATER] = 1;
      }
      if( work->area_data->season_data[season_id].place_bitflag[zkn_area_idx] & PLACE_BITFLAG_FISHING_ALL )
      {
        brighten[PLACE_ICON_FISHING] = 1;
      }
    }
  }

  // アイコンの明るさを変更する
  for( i=0; i<PLACE_ICON_MAX; i++ )
  {
    GFL_BG_ChangeScreenPalette(
        BG_FRAME_S_PLACE,
        place_icon_screen_chr[i][0],
        place_icon_screen_chr[i][1],
        place_icon_screen_chr[i][2],
        place_icon_screen_chr[i][3],
        place_icon_bg_pal_pos[i][brighten[i]] );
  }

  GFL_BG_LoadScreenV_Req( BG_FRAME_S_PLACE );
}

//-------------------------------------
/// OBJ_UNKNOWNを表示するためのアルファ設定
//=====================================
static void Zukan_Detail_Map_ObjUnknownAlphaInit( ZUKAN_DETAIL_MAP_WORK* work )
{
  int ev1 = 12;

#if 0
  何かUnknownとExistを変えるたびにここを変えると完全不透明のOBJの表示が乱れるので、
  完全不透明のOBJはZukan_Detail_Map_AlphaMainInitとZukan_Detail_Map_AlphaMainExitだけで
  Map表示中1回しか変更しないようにした。
  
  u8 i;

  // 半透明にしないOBJの設定を変更する
  for( i=0; i<OBJ_MAX; i++ )
  {
    if( i != OBJ_UNKNOWN )
      GFL_CLACT_WK_SetObjMode( work->obj_clwk[i], GX_OAM_MODE_NORMAL );  // アルファアニメーションの影響を受けないようにする
  }
  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
  {
    if( work->hide_spot[i].state == HIDE_STATE_HIDE_FALSE )
    {
      GFL_CLACT_WK_SetObjMode( work->hide_spot[i].obj_clwk, GX_OAM_MODE_NORMAL );  // アルファアニメーションの影響を受けないようにする
    }
  }
#endif

  G2_SetBlendAlpha(
        GX_BLEND_PLANEMASK_NONE,
        GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
        ev1,
        16 - ev1 );
}
static void Zukan_Detail_Map_ObjUnknownAlphaExit( ZUKAN_DETAIL_MAP_WORK* work )
{
#if 0
  何かUnknownとExistを変えるたびにここを変えると完全不透明のOBJの表示が乱れるので、
  完全不透明のOBJはZukan_Detail_Map_AlphaMainInitとZukan_Detail_Map_AlphaMainExitだけで
  Map表示中1回しか変更しないようにした。
  
  u8 i;

  // 半透明にしないOBJの設定を元に戻す
  for( i=0; i<OBJ_MAX; i++ )
  {
    if( i != OBJ_UNKNOWN )
      GFL_CLACT_WK_SetObjMode( work->obj_clwk[i], GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
  }
  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
  {
    if( work->hide_spot[i].state == HIDE_STATE_HIDE_FALSE )
    {
      GFL_CLACT_WK_SetObjMode( work->hide_spot[i].obj_clwk, GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
    }
  }
#endif

  // 一部分フェードの設定を元に戻す
  ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_m );
}

//-------------------------------------
/// obj_existのアルファアニメーション
//=====================================
static void Zukan_Detail_Map_ObjExistAlphaInit( ZUKAN_DETAIL_MAP_WORK* work )
{
#if 0
  何かUnknownとExistを変えるたびにここを変えると完全不透明のOBJの表示が乱れるので、
  完全不透明のOBJはZukan_Detail_Map_AlphaMainInitとZukan_Detail_Map_AlphaMainExitだけで
  Map表示中1回しか変更しないようにした。
  
  u8 i;

  // 半透明にしないがwndに入ってしまうOBJの設定を変更する
  for( i=0; i<OBJ_MAX; i++ )
  {
    GFL_CLACT_WK_SetObjMode( work->obj_clwk[i], GX_OAM_MODE_NORMAL );  // アルファアニメーションの影響を受けないようにする
  }
  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
  {
    if( work->hide_spot[i].state == HIDE_STATE_HIDE_FALSE )
    {
      GFL_CLACT_WK_SetObjMode( work->hide_spot[i].obj_clwk, GX_OAM_MODE_NORMAL );  // アルファアニメーションの影響を受けないようにする
    }
  }
#endif

  // wnd
  GX_SetVisibleWnd( GX_WNDMASK_W0 | GX_WNDMASK_W1 );
  G2_SetWnd0Position( SEASON_PANEL_POS_X, 0, SEASON_PANEL_POS_X+SEASON_PANEL_SIZE_X, 168 );
  G2_SetWnd1Position( SEASON_PANEL_POS_X+SEASON_PANEL_SIZE_X, 0, 0/*256*/, 168 );

  G2_SetWnd0InsidePlane(
    GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ,
    TRUE );
  G2_SetWnd1InsidePlane(
    GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ,
    TRUE );
  G2_SetWndOutsidePlane(
    GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ,
    FALSE );

  Zukan_Detail_Map_ObjExistAlphaReset( work );
}
static void Zukan_Detail_Map_ObjExistAlphaExit( ZUKAN_DETAIL_MAP_WORK* work )
{
  u8 i;

  // wnd
  GX_SetVisibleWnd( GX_WNDMASK_NONE );

#if 0
  何かUnknownとExistを変えるたびにここを変えると完全不透明のOBJの表示が乱れるので、
  完全不透明のOBJはZukan_Detail_Map_AlphaMainInitとZukan_Detail_Map_AlphaMainExitだけで
  Map表示中1回しか変更しないようにした。

  // 半透明にしないがwndに入ってしまうOBJの設定を元に戻す
  for( i=0; i<OBJ_MAX; i++ )
  {
    GFL_CLACT_WK_SetObjMode( work->obj_clwk[i], GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
  }
  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
  {
    if( work->hide_spot[i].state == HIDE_STATE_HIDE_FALSE )
    {
      GFL_CLACT_WK_SetObjMode( work->hide_spot[i].obj_clwk, GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
    }
  }
#endif

  // 一部分フェードの設定を元に戻す
  ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_m );
}
static void Zukan_Detail_Map_ObjExistAlphaMain( ZUKAN_DETAIL_MAP_WORK* work )
{
  // 更新
  {
    G2_SetBlendAlpha(
        GX_BLEND_PLANEMASK_NONE,/*GX_BLEND_PLANEMASK_OBJ,*/
        GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
        work->obj_exist_ev1,
        16 - work->obj_exist_ev1 );
  }

  // カウントを進める
  if( (u32)work->obj_exist_alpha_anime_count + OBJ_EXIST_ALPHA_ANIME_COUNT_ADD >= OBJ_EXIST_ALPHA_ANIME_COUNT_MAX )
  {
    work->obj_exist_alpha_anime_count = (u16)( (u32)work->obj_exist_alpha_anime_count + OBJ_EXIST_ALPHA_ANIME_COUNT_ADD - OBJ_EXIST_ALPHA_ANIME_COUNT_MAX );
  }
  else
  {
    work->obj_exist_alpha_anime_count += OBJ_EXIST_ALPHA_ANIME_COUNT_ADD;
  }

  // ev1を求める 
  {
    // 1～0に変換
    fx16 cos = ( FX_CosIdx(work->obj_exist_alpha_anime_count) + FX16_ONE ) /2;
    work->obj_exist_ev1 = OBJ_EXIST_ALPHA_MIN + ( ( (OBJ_EXIST_ALPHA_MAX - OBJ_EXIST_ALPHA_MIN) * cos ) >>FX16_SHIFT );
  }

// TWLプログラミングマニュアル
// 第 1 対象面でOBJ=1 の場合は、OBJ の種類に関わらずすべてのOBJに対して処理が実行されます。
// OBJ=0 の場合は、半透明OBJ、ビットマップOBJ、3D 面を描画したBG0 面の場合のみ処理が実行されます。
//
// つまり、GX_BLEND_PLANEMASK_OBJにしてしまうと、GX_OAM_MODE_NORMALも半透明になってしまう。
// GX_BLEND_PLANEMASK_NONEにしておけば、GX_OAM_MODE_XLUだけが半透明になる。
}
static void Zukan_Detail_Map_ObjExistAlphaReset( ZUKAN_DETAIL_MAP_WORK* work )
{
  work->obj_exist_ev1                  = OBJ_EXIST_ALPHA_MIN;
  work->obj_exist_alpha_anime_count    = OBJ_EXIST_ALPHA_ANIME_COUNT_MAX / 2;  // cosで1～0にしているので

  // さっそくこのリセットした値でAlphaを
  // 更新
  {
    G2_SetBlendAlpha(
        GX_BLEND_PLANEMASK_NONE,/*GX_BLEND_PLANEMASK_OBJ,*/
        GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
        work->obj_exist_ev1,
        16 - work->obj_exist_ev1 );
  }
}

//-------------------------------------
/// アルファ設定
//=====================================
static void Zukan_Detail_Map_AlphaMainInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // Main画面

  u8 i;

  // 半透明にしないOBJの設定を変更する
  for( i=0; i<OBJ_MAX; i++ )
  {
    if( i != OBJ_UNKNOWN )
      GFL_CLACT_WK_SetObjMode( work->obj_clwk[i], GX_OAM_MODE_NORMAL );  // アルファアニメーションの影響を受けないようにする
  }
  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
  {
    if( work->hide_spot[i].state == HIDE_STATE_HIDE_FALSE )
    {
      GFL_CLACT_WK_SetObjMode( work->hide_spot[i].obj_clwk, GX_OAM_MODE_NORMAL );  // アルファアニメーションの影響を受けないようにする
    }
  }
}
static void Zukan_Detail_Map_AlphaMainExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // Main画面

  u8 i;

  // 半透明にしないOBJの設定を元に戻す
  for( i=0; i<OBJ_MAX; i++ )
  {
    if( i != OBJ_UNKNOWN )
      GFL_CLACT_WK_SetObjMode( work->obj_clwk[i], GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
  }
  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
  {
    if( work->hide_spot[i].state == HIDE_STATE_HIDE_FALSE )
    {
      GFL_CLACT_WK_SetObjMode( work->hide_spot[i].obj_clwk, GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
    }
  }
}

static void Zukan_Detail_Map_AlphaSubInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // Sub画面

  int ev1 = 12;

  // 半透明にしないOBJの設定を変更する
  {
    u8 i;
    for( i=0; i<OBJ_SWAP_MAX; i++ )
    {
      if( work->pokeicon_clwk[i] )
      {
        GFL_CLACT_WK_SetObjMode( work->pokeicon_clwk[i], GX_OAM_MODE_NORMAL );  // アルファアニメーションの影響を受けないようにする
      }
    }
  }

  G2S_SetBlendAlpha(
        GX_BLEND_PLANEMASK_BG3,
        GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
        ev1,
        16 - ev1 );
}
static void Zukan_Detail_Map_AlphaSubExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // Sub画面

  // 半透明にしないOBJの設定を元に戻す
  {
    u8 i;
    for( i=0; i<OBJ_SWAP_MAX; i++ )
    {
      if( work->pokeicon_clwk[i] )
      {
        GFL_CLACT_WK_SetObjMode( work->pokeicon_clwk[i], GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
      }
    }
  }

  // 一部分フェードの設定を元に戻す
  ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_s );
}

