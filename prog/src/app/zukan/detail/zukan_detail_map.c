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
#define DEBUG_KAWADA


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "ui/ui_easy_clwk.h"
#include "pokeicon/pokeicon.h"
#include "gamesystem/gamedata_def.h"
#include "gamesystem/game_data.h"
#include "savedata/zukan_savedata.h"
#include "system/bmp_winframe.h"
#include "print/printsys.h"
#include "print/gf_font.h"
#include "print/wordset.h"
#include "sound/pm_sndsys.h"
#include "app/townmap_data_sys.h"

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
#include "zukan_gra.naix"

// サウンド

// オーバーレイ
FS_EXTERN_OVERLAY(ui_common);
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
  OBJ_PAL_NUM_M_TM           = 4,
  OBJ_PAL_NUM_M_ZUKAN        = 2,
  OBJ_PAL_NUM_M_EXIST        = 1,
};
// 位置
enum
{
  OBJ_PAL_POS_M_TM           = 0,
  OBJ_PAL_POS_M_ZUKAN        = 4,
  OBJ_PAL_POS_M_EXIST        = 6,
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
  SEQ_PREPARE,
  SEQ_FADE_IN,
  SEQ_MAIN,
  SEQ_FADE_OUT,
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
#define STRBUF_UNKNOWN_LENGTH   ( 64 )  // せいそくちふめい

// ポケアイコンの位置
static const u8 pokeicon_pos[4] = { 56, 112 +8*8,     // スクロール前(陸上、水上、釣りアイコンが見えていないとき)
                                    56, 112       };  // スクロール後(陸上、水上、釣りアイコンが見えているとき)

// 状態
typedef enum
{
  STATE_TOP,
  STATE_SELECT,
}
STATE;

typedef enum
{
  SCROLL_STATE_DOWN,
  SCROLL_STATE_DOWN_TO_UP,
  SCROLL_STATE_UP,
  SCROLL_STATE_UP_TO_DOWN,
}
SCROLL_STATE;

#define SCROLL_VEL (8)  // 上画面のスクロールの速度(1フレームにこれだけ移動する)

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
  OBJ_ZUKAN_START,                          // OBJ_ZUKAN_START<= <OBJ_ZUKAN_END
  OBJ_SEASON          = OBJ_ZUKAN_START,
  //OBJ_SEASON_L,
  OBJ_SEASON_R,
  OBJ_UNKNOWN,
  OBJ_ZUKAN_END,

  OBJ_TM_START        = OBJ_ZUKAN_END,      // OBJ_TM_START<= <OBJ_TM_END
  OBJ_CURSOR          = OBJ_TM_START,
  OBJ_RING_CUR,
  OBJ_HERO,
  OBJ_TM_END,

  OBJ_MAX             = OBJ_TM_END,
};

// 季節
typedef enum
{
  SEASON_SPRING,
  SEASON_SUMMER,
  SEASON_AUTUMN,
  SEASON_WINTER,
}
SEASON;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // 他のところから借用するもの
  GFL_CLUNIT*                 clunit;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // ここで作成するもの
  ZKNDTL_COMMON_REAR_WORK*    rear_wk_s;

  // メッセージとテキスト
  GFL_MSGDATA*                msgdata[MSG_MAX];
  GFL_BMPWIN*                 text_bmpwin[TEXT_MAX];
  GFL_BMPWIN*                 text_dummy_bmpwin[TEXT_DUMMY_MAX];

  // 上画面
  UI_EASY_CLWK_RES            pokeicon_res;    // pokeicon_clwkがNULLのとき、使用していない
  GFL_CLWK*                   pokeicon_clwk;   // NULLのときなし
  GFL_ARCUTIL_TRANSINFO       place_tinfo;

  // 状態
  STATE                       state;
  SCROLL_STATE                scroll_state;
  SCROLL_STATE                pokeicon_scroll_state;

  // OBJ
  u32                         obj_res[OBJ_RES_MAX];
  GFL_CLWK*                   obj_clwk[OBJ_MAX];
  GFL_CLWK*                   obj_exist_clwk[TOWNMAP_DATA_MAX];

  // タウンマップデータ
  TOWNMAP_DATA*               townmap_data;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // フェード
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_m;
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_s;

  // 終了命令
  END_CMD                     end_cmd;
}
ZUKAN_DETAIL_MAP_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Zukan_Detail_Map_VBlankFunc( GFL_TCB* tcb, void* wk );

// アフィン拡張BG
static void Zukan_Detail_Map_AffineExBGInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_AffineExBGExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// OBJ
static void Zukan_Detail_Map_ObjInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_ObjExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_ObjExistInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_ObjExistExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// メッセージとテキスト
static void Zukan_Detail_Map_MsgTextInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_MsgTextExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 上画面
static void Zukan_Detail_Map_PlaceInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_PlaceExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_PlaceMain( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_PlaceMainScroll( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_PlaceMainIcon( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// ポケアイコン
static GFL_CLWK* PokeiconInit( UI_EASY_CLWK_RES* res, GFL_CLUNIT* clunit, HEAPID heap_id, u32 mons, u32 form_no, BOOL egg, u8 x, u8 y );
static void PokeiconExit( UI_EASY_CLWK_RES* res, GFL_CLWK* clwk );
// マクロ
#define BLOCK_POKEICON_EXIT(res,clwk)                     \
    {                                                     \
      if( clwk ) PokeiconExit( res, clwk );               \
      clwk = NULL;                                        \
    }
// NULLを代入し忘れないようにマクロを使うようにしておく

// プレイヤーの位置にOBJを配置する
static void Zukan_Detail_Map_SetPlayer( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 操作
static void Zukan_Detail_Map_Input( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
// 状態を遷移させる
static void Zukan_Detail_Map_ChangeState( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                 STATE state );
static void Zukan_Detail_Map_ChangePoke( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_ChangePlace( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );


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

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Map_ProcInit\n" );
  }
#endif

  // オーバーレイ
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common) );
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
    work->print_que   = ZKNDTL_COMMON_GetPrintQue(cmn);
  }

  // NULL初期化
  {
    work->pokeicon_clwk = NULL;
  }

  // 状態初期化
  {
    work->state                    = STATE_TOP;
    work->scroll_state             = SCROLL_STATE_DOWN;
    work->pokeicon_scroll_state    = SCROLL_STATE_DOWN;
  }

  // タウンマップデータ
  work->townmap_data = TOWNMAP_DATA_Alloc( param->heap_id );

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zukan_Detail_Map_VBlankFunc, work, 1 );

  // フェード
  {
    work->fade_wk_m = ZKNDTL_COMMON_FadeInit( param->heap_id );
    work->fade_wk_s = ZKNDTL_COMMON_FadeInit( param->heap_id );
   
    ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_m );
    ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_s );

    ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_M, work->fade_wk_m );
    ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
  }

  // 終了情報
  work->end_cmd = END_CMD_NONE;

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Map_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_MAP_PARAM*    param    = (ZUKAN_DETAIL_MAP_PARAM*)pwk;
  ZUKAN_DETAIL_MAP_WORK*     work     = (ZUKAN_DETAIL_MAP_WORK*)mywk;

  // ポケアイコン
  BLOCK_POKEICON_EXIT( &work->pokeicon_res, work->pokeicon_clwk )

  // 上画面
  Zukan_Detail_Map_PlaceExit( param, work, cmn );
  // メッセージとテキスト 
  Zukan_Detail_Map_MsgTextExit( param, work, cmn );
 
  // 最背面
  ZKNDTL_COMMON_RearExit( work->rear_wk_s );

  // OBJ
  Zukan_Detail_Map_ObjExistExit( param, work, cmn );
  Zukan_Detail_Map_ObjExit( param, work, cmn );

  // フェード
  {
    ZKNDTL_COMMON_FadeExit( work->fade_wk_s );
    ZKNDTL_COMMON_FadeExit( work->fade_wk_m );
  }

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // タウンマップデータ
  TOWNMAP_DATA_Free( work->townmap_data );

  // ヒープ
  ZKNDTL_PROC_FreeWork( proc );

  // オーバーレイ
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(townmap) );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

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
      *seq = SEQ_PREPARE;

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
      
      // 最背面
      work->rear_wk_s = ZKNDTL_COMMON_RearInit( param->heap_id, ZKNDTL_COMMON_REAR_TYPE_MAP,
          BG_FRAME_S_REAR, BG_PAL_POS_S_REAR +0, BG_PAL_POS_S_REAR +1 );

      // メッセージとテキスト 
      Zukan_Detail_Map_MsgTextInit( param, work, cmn );
      // 上画面
      Zukan_Detail_Map_PlaceInit( param, work, cmn );

      // プレイヤーの位置にOBJを配置する
      Zukan_Detail_Map_SetPlayer( param, work, cmn );

      // この画面に来たときに選ばれていたポケモンを表示する
      Zukan_Detail_Map_ChangePoke( param, work, cmn );
    }
    break;
  case SEQ_PREPARE:
    {
      *seq = SEQ_FADE_IN;

      // フェード
      ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_m );
      ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_s );

      // タッチバー
      if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) != ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_TOUCHBAR_SetType(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
            ZUKAN_DETAIL_TOUCHBAR_DISP_MAP );
        ZUKAN_DETAIL_TOUCHBAR_Appear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
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
          && (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_s ))
          && ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );

        *seq = SEQ_MAIN;
      }
    }
    break;
  case SEQ_MAIN:
    {
      if( work->end_cmd != END_CMD_NONE )
      {
        *seq = SEQ_FADE_OUT;

        // フェード
        ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_m );
        ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_s );
        
        // タイトルバー
        ZUKAN_DETAIL_HEADBAR_Disappear( headbar );
        
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_Disappear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
        }
      }
      else
      {
        // 操作
        Zukan_Detail_Map_Input( param, work, cmn );
      }
    }
    break;
  case SEQ_FADE_OUT:
    {
      if(    (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          && (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_s ))
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_DISAPPEAR )
      {
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR )
          {
            *seq = SEQ_END;
          }
        }
        else
        {
          *seq = SEQ_END;
        }
      }
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
  }

  // フェード
  ZKNDTL_COMMON_FadeMain( work->fade_wk_m, work->fade_wk_s );

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
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToNextPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          Zukan_Detail_Map_ChangePoke( param, work, cmn );
        }
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_CUR_U:
      {
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToPrevPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          Zukan_Detail_Map_ChangePoke( param, work, cmn );
        }
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_CHECK:
      {
        GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
        ZUKAN_SAVEDATA* zukan_savedata = GAMEDATA_GetZukanSave( gamedata );
        u16 monsno_curr;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZUKANSAVE_SetShortcutMons( zukan_savedata, monsno_curr );
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_MAP_RETURN:
      {
        Zukan_Detail_Map_ChangeState( param, work, cmn, STATE_TOP );
      }
      break;
    default:
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
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
  ZUKAN_DETAIL_MAP_WORK*     work     = (ZUKAN_DETAIL_MAP_WORK*)wk;
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
      //2,
      2,
      0,

      // OBJ_TM_
      4,
      5,
      6,
    };
    u8 bgpri[OBJ_MAX] =
    {
      // OBJ_ZUKAN_
      BG_FRAME_PRI_M_ROOT,
      //BG_FRAME_PRI_M_ROOT,
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
      //1,
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
                             work->clunit,
                             work->obj_res[OBJ_RES_TM_NCG], work->obj_res[OBJ_RES_TM_NCL], work->obj_res[OBJ_RES_TM_NCE],
                             &cldata, CLSYS_DEFREND_MAIN, param->heap_id );
    }
    
    for( i=OBJ_ZUKAN_START; i<OBJ_ZUKAN_END; i++ )
    {	
      work->obj_clwk[i] = GFL_CLACT_WK_Create(
                             work->clunit,
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
}

static void Zukan_Detail_Map_ObjExistInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // CLWK作成
  {
    u8 softpri = 4;  // obj_clwkのsoftpriよりプライオリティを低くしておく

    u8 i;
    GFL_CLWK_DATA cldata;
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
 
    for( i=0; i<TOWNMAP_DATA_MAX; i++ )
    {	
      work->obj_exist_clwk[i] = GFL_CLACT_WK_Create(
                             work->clunit,
                             work->obj_res[OBJ_RES_EXIST_NCG], work->obj_res[OBJ_RES_EXIST_NCL], work->obj_res[OBJ_RES_EXIST_NCE],
                             &cldata, CLSYS_DEFREND_MAIN, param->heap_id );

      GFL_CLACT_WK_SetAnmSeq( work->obj_exist_clwk[i], 0 );
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
/// メッセージとテキスト
//-------------------------------------
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
    GFL_BMPWIN_Delete( work->text_bmpwin[i] );
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
  if( work->pokeicon_clwk )
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
        GFL_CLACT_WK_GetPos( work->pokeicon_clwk, &pos, CLSYS_DEFREND_SUB );
        if( pos.y <= pokeicon_pos[3] )
        {
          pos.y = pokeicon_pos[3];
          work->pokeicon_scroll_state = SCROLL_STATE_UP; 
        }
        else
        {
          pos.y -= SCROLL_VEL;
        }
        GFL_CLACT_WK_SetPos( work->pokeicon_clwk, &pos, CLSYS_DEFREND_SUB );
      }
      break;
    case SCROLL_STATE_UP:
      {
      }
      break;
    case SCROLL_STATE_UP_TO_DOWN:
      {
        GFL_CLACT_WK_GetPos( work->pokeicon_clwk, &pos, CLSYS_DEFREND_SUB );
        if( pos.y >= pokeicon_pos[1] )
        {
          pos.y = pokeicon_pos[1];
          work->pokeicon_scroll_state = SCROLL_STATE_DOWN; 
        }
        else
        {
          pos.y += SCROLL_VEL;
        }
        GFL_CLACT_WK_SetPos( work->pokeicon_clwk, &pos, CLSYS_DEFREND_SUB );
      }
      break;
    }
  }
}
static void Zukan_Detail_Map_PlaceMainIcon( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 陸上、水上、釣りアイコンの明滅アニメーション
}

//-------------------------------------
/// ポケアイコン
//=====================================
static GFL_CLWK* PokeiconInit( UI_EASY_CLWK_RES* res, GFL_CLUNIT* clunit, HEAPID heap_id, u32 mons, u32 form_no, BOOL egg, u8 x, u8 y )
{
  GFL_CLWK* clwk;
  
  UI_EASY_CLWK_RES_PARAM prm;
  prm.draw_type    = CLSYS_DRAW_SUB;
  prm.comp_flg     = UI_EASY_CLWK_RES_COMP_NCLR;
  prm.arc_id       = ARCID_POKEICON;
  prm.pltt_id      = POKEICON_GetPalArcIndex();
  prm.ncg_id       = POKEICON_GetCgxArcIndexByMonsNumber( mons, form_no, egg );
  prm.cell_id      = POKEICON_GetCellArcIndex(); 
  prm.anm_id       = POKEICON_GetAnmArcIndex();
  prm.pltt_line    = OBJ_PAL_POS_S_POKEICON;
  prm.pltt_src_ofs = 0;
  prm.pltt_src_num = OBJ_PAL_NUM_S_POKEICON;
 
  UI_EASY_CLWK_LoadResource( res, &prm, clunit, heap_id );
  
  // アニメシーケンスで指定( 0=瀕死, 1=HP最大, 2=HP緑, 3=HP黄, 4=HP赤, 5=状態異常 )
  clwk = UI_EASY_CLWK_CreateCLWK( res, clunit, x, y, 1, heap_id );

  // 上にアイテムアイコンを描画するので優先度を下げておく
  GFL_CLACT_WK_SetSoftPri( clwk, 1 );

  // オートアニメ OFF
  GFL_CLACT_WK_SetAutoAnmFlag( clwk, FALSE );

  {
    u8 pal_num = POKEICON_GetPalNum( mons, form_no, egg );
    GFL_CLACT_WK_SetPlttOffs( clwk, pal_num, CLWK_PLTTOFFS_MODE_OAM_COLOR );
  }

  GFL_CLACT_WK_SetObjMode( clwk, GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので

  return clwk;
}
static void PokeiconExit( UI_EASY_CLWK_RES* res, GFL_CLWK* clwk )
{
  // CLWK破棄
  GFL_CLACT_WK_Remove( clwk );
  // リソース開放
  UI_EASY_CLWK_UnLoadResource( res );
}

//-------------------------------------
/// プレイヤーの位置にOBJを配置する
//=====================================
static void Zukan_Detail_Map_SetPlayer( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  GAMEDATA*       gamedata  = ZKNDTL_COMMON_GetGamedata(cmn);
  PLAYER_WORK*    player_wk = GAMEDATA_GetMyPlayerWork( gamedata );
  u16             zone_id   = PLAYERWORK_getZoneID( player_wk );
  u16             escape_id = GAMEDATA_GetEscapeLocation( gamedata )->zone_id;
 
  BOOL b_find = FALSE;
  u8   player_pos_idx;

  GFL_CLACTPOS pos;
  u8 i;

  // ゾーンと完全対応する場所を取得
  if( !b_find )
  {
	  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
	  {	
	  	if( zone_id == TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_ZONE_ID ) )
      {
        b_find = TRUE;
        player_pos_idx = i;
        break;
      }
    }
  }
  // 現在地がフィールドではないならば、エスケープID検索
  if( !b_find )
  {
	  for( i=0; i<TOWNMAP_DATA_MAX; i++ )
	  {	
	  	if( escape_id == TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_ZONE_ID ) )
      {
        b_find = TRUE;
        player_pos_idx = i;
        break;
      }
    }
  }

  if( b_find )
  {
    pos.x = TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_POS_X );
    pos.y = TOWNMAP_DATA_GetParam( work->townmap_data, i, TOWNMAP_DATA_PARAM_POS_Y );
    GFL_CLACT_WK_SetPos( work->obj_clwk[OBJ_HERO], &pos, CLSYS_DEFREND_MAIN );
    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_HERO], TRUE );
  }
}

//-------------------------------------
/// 入力
//=====================================
static void Zukan_Detail_Map_Input( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  switch(work->state)
  {
  case STATE_TOP:
    {
      u32 x, y;
      BOOL change_state = FALSE;
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        change_state = TRUE;
      }
      else
      {
		    if( GFL_UI_TP_GetPointTrg( &x, &y ) )
        {
          if( 0<=y&&y<168) change_state = TRUE;
        }
      }
      if( change_state )
      {
        Zukan_Detail_Map_ChangeState( param, work, cmn, STATE_SELECT );
        PMSND_PlaySE( SEQ_SE_DECIDE1 );
      }
    }
    break;
  case STATE_SELECT:
    {
    }
    break;
  }
}

//-------------------------------------
/// 状態を遷移させる
//=====================================
static void Zukan_Detail_Map_ChangeState( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                 STATE state )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
 
  // 遷移前の状態 
  switch(work->state)
  {
  case STATE_TOP:
    {
      if( state == STATE_SELECT )  // 遷移後の状態がSTATE_SELECTなら
      {
        work->scroll_state             = SCROLL_STATE_DOWN_TO_UP;
        work->pokeicon_scroll_state    = SCROLL_STATE_DOWN_TO_UP;

        // タッチバー
        ZUKAN_DETAIL_TOUCHBAR_SetType(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_TYPE_MAP,
            ZUKAN_DETAIL_TOUCHBAR_DISP_MAP );
      }
    }
    break;
  case STATE_SELECT:
    {
      if( state == STATE_TOP )  // 遷移後の状態がSTATE_TOPなら
      {
        work->scroll_state             = SCROLL_STATE_UP_TO_DOWN;
        work->pokeicon_scroll_state    = SCROLL_STATE_UP_TO_DOWN;
        
        // タッチバー
        ZUKAN_DETAIL_TOUCHBAR_SetType(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
            ZUKAN_DETAIL_TOUCHBAR_DISP_MAP );
      }
    }
    break;
  }

  // 遷移後の状態
  work->state = state;
}

static void Zukan_Detail_Map_ChangePoke( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // ポケモンを変更したとき

  // 現在の状態
  SEASON season_curr = SEASON_SPRING;

  // 変更後のポケモン
  u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);

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

    // 前のを破棄
    BLOCK_POKEICON_EXIT( &work->pokeicon_res, work->pokeicon_clwk )
    // 次のを生成
    {
      u32 form_no = 0;
      work->pokeicon_clwk = PokeiconInit( &work->pokeicon_res, work->clunit, param->heap_id, monsno, form_no, 0, x, y );
    }
  }

  // ポケモン名のぶんぷ
  // 前のをクリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_POKENAME]), 0 );
  GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_POKENAME] );
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
    PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_POKENAME]),
                            0, 5, strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
    GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[TEXT_POKENAME] );
    GFL_STR_DeleteBuffer( strbuf );
  }

  // 分布
  // 前のをクリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_UNKNOWN]), 0 );
  GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_UNKNOWN] );
  GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_UNKNOWN], FALSE );
  GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_SEASON], FALSE );
  GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_SEASON_R], FALSE );
  // 次のを表示
  // 生息地不明のとき
  {
    WORDSET* wordset;
    STRBUF* src_strbuf;
    STRBUF* strbuf;
    u16     str_width;
    u16     bmp_width;
    u16     x;
    wordset = WORDSET_Create( param->heap_id );
    src_strbuf = GFL_MSG_CreateString( work->msgdata[MSG_ZUKAN], ZNK_RANGE_00 );
    strbuf = GFL_STR_CreateBuffer( STRBUF_UNKNOWN_LENGTH, param->heap_id );
    WORDSET_RegisterPokeMonsNameNo( wordset, 0, monsno );
    WORDSET_ExpandStr( wordset, strbuf, src_strbuf );
    GFL_STR_DeleteBuffer( src_strbuf );
    WORDSET_Delete( wordset );
    str_width = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
    bmp_width = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_UNKNOWN]) );
    x = ( bmp_width - str_width ) / 2;
    PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_UNKNOWN]),
                            x, 1, strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
    GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[TEXT_UNKNOWN] );
    GFL_STR_DeleteBuffer( strbuf );

    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_UNKNOWN], TRUE );
  }
  // 1年中同じ分布のとき
  {
  }
  // 季節ごとに分布が変わるとき
  {
    STRBUF* strbuf;
    u16     str_width;
    u16     bmp_width;
    u16     x;
    strbuf = GFL_MSG_CreateString( work->msgdata[MSG_ZUKAN], ZKN_MAP_SPRING + season_curr );
    str_width = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
    bmp_width = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_SEASON]) );
    x = ( bmp_width - str_width ) / 2;
    PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_SEASON]),
                            x, 1, strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
    GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[TEXT_SEASON] );
    GFL_STR_DeleteBuffer( strbuf );

    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_SEASON], TRUE );
    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_SEASON_R], TRUE );
  }
}

static void Zukan_Detail_Map_ChangePlace( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 場所を変更したとき
  
  // 前のをクリア
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_PLACENAME]), 0 );
  GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_PLACENAME] );
}

