//=============================================================================
/**
 *
 *  @file   box_search.c
 *  @brief  ボックス絞り込み画面
 *  @author Akito Mori
 *  @data   2011.02.01
 *
 * UIテンプレートから作成
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/wipe.h"
#include "system/cursor_move.h"
#include "font/font.naix"
#include "ui/frame_list.h"
#include "savedata/zukan_savedata.h"

// 簡易会話表示システム
#include "system/pms_draw.h"

//テクスチャをOAMに転送
#include "system/nsbtx_to_clwk.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"

#include "print/wordset.h"

//INFOWIN
#include "infowin/infowin.h"

//描画設定
#include "box_search_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

//マップモデルのnsbtx
#include "fieldmap/fldmmdl_mdlres.naix"

//タッチバー
#include "ui/touchbar.h"

//ポケパラ、ポケモンNO
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

//ポケアイコン
#include "pokeicon/pokeicon.h"

//MCSS
#include "system/mcss.h"
#include "system/mcss_tool.h"

//タスクメニュー
#include "app/app_taskmenu.h"

//どうぐアイコン
#include "item/item.h"
#include "item_icon.naix"

//ポケモンBG,OBJ読みこみ
#include "system/poke2dgra.h"

//アプリ共通素材
#include "app/app_menu_common.h"

// サウンド
#include "sound/pm_sndsys.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "box_search.h"


//@TODO BG読み込み とりあえずマイクテストのリソース
#include "message.naix"
//#include "mictest.naix" // アーカイブ
#include "box_gra.naix"
#include "msg/msg_boxmenu.h"  // GMM
#include "townmap_gra.naix"   // タッチバーカスタムボタン用サンプルにタウンマップリソース

// 図鑑ソートデータ
#include "zukan_data.naix"
#include "../../../resource/zukan_data/zkn_sort_aiueo_idx.h"


//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
#define BOX_SEARCH_TOUCHBAR
//#define BOX_SEARCH_TASKMENU
//#define BOX_SEARCH_POKE_ICON     // ポケアイコン(ポケアイコン用アイテムアイコン込み)
//#define BOX_SEARCH_OAM_MAPMODEL  // マップモデルをOAMで表示    //nsbtx_to_clwkをオーバーレイに移したのでコメントアウトにしました 10/06/11 nagihashi
//#define BOX_SEARCH_PMSDRAW       // 簡易会話表示

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *                定数定義
 */
//=============================================================================
enum
{ 
  BOX_SEARCH_HEAP_SIZE = 0x30000,  ///< ヒープサイズ
};

enum
{
  SEQ_FADEIN_WAIT=0,
  SEQ_MAIN,
  SEQ_FADEOUT,
  SEQ_FADEOUT_WAIT,
  
};

// 上画面に表示する説明文字列の展開用バッファ（ローカライズも考えて２倍）
#define SUBDISP_TEXT_SIZE   ( 64*2 )

#define	BUTTON_ID_TB_RETURN		( 0xffffffff )		// 終了ボタンのボタンID


//-------------------------------------
/// フレーム
//=====================================
enum
{ 
  BG_FRAME_BAR_M    = GFL_BG_FRAME1_M,
  BG_FRAME_BUTTON_M = GFL_BG_FRAME2_M,
  BG_FRAME_BACK_M   = GFL_BG_FRAME3_M,
  BG_FRAME_BACK_S   = GFL_BG_FRAME2_S,
  BG_FRAME_PLATE_S  = GFL_BG_FRAME1_S,
  BG_FRAME_TEXT_S   = GFL_BG_FRAME0_S, 
};
//-------------------------------------
/// パレット
//=====================================
enum
{ 
  //メインBG
  PLTID_BG_BACK_M       = 0,
  PLTID_BG_POKE_M       = 1,
  PLTID_BG_TASKMENU_M   = 11,
  PLTID_BG_TOUCHBAR_M   = 13,
  PLTID_BG_INFOWIN_M    = 15,
  //サブBG
  PLTID_BG_BACK_S       = 0,

  //メインOBJ
  PLTID_OBJ_TOUCHBAR_M = 0,		// 3本使用
	PLTID_OBJ_BOXSEARCH_M = 3,	// 6本使用
	PLTID_OBJ_MARK_M = 9,				// 1本使用
	PLTID_OBJ_ARROW_M = 10,			// 3本使用
  //サブOBJ
  PLTID_OBJ_PMS_DRAW = 0, // 5本使用
};

// 絞り込みメイン画面OBJ（プレートやボタン)
enum{
  BOXSEARCH_OBJID_PLATE0=0,
  BOXSEARCH_OBJID_PLATE1,
  BOXSEARCH_OBJID_PLATE2,
  BOXSEARCH_OBJID_PLATE3,
  BOXSEARCH_OBJID_PLATE4,
  BOXSEARCH_OBJID_PLATE5,
  BOXSEARCH_OBJID_RESET,
  BOXSEARCH_OBJID_BUTTON_ON,
  BOXSEARCH_OBJID_BUTTON_OFF,
  BOXSEARCH_OBJID_BUTTON_ON_CHECKBOX,
  BOXSEARCH_OBJID_BUTTON_OFF_CHECKBOX,
  BOXSEARCH_OBJID_CHECK0, // 選択リストのチェックボックス
  BOXSEARCH_OBJID_CHECK1,
  BOXSEARCH_OBJID_CHECK2,
  BOXSEARCH_OBJID_CHECK3,
  BOXSEARCH_OBJID_CHECK4,
  BOXSEARCH_OBJID_CHECK5,
  BOXSEARCH_OBJID_CHECK6,
  BOXSEARCH_OBJID_CHECK7,
  BOXSEARCH_OBJID_CHECK8,
  BOXSEARCH_OBJID_CHECK9,
  BOXSEARCH_OBJID_SCROL,
  BOXSEARCH_OBJID_MARK0_0,// トップ画面の横に並んだマーキングマーク
  BOXSEARCH_OBJID_MARK0_1,
  BOXSEARCH_OBJID_MARK0_2,
  BOXSEARCH_OBJID_MARK0_3,
  BOXSEARCH_OBJID_MARK0_4,
  BOXSEARCH_OBJID_MARK0_5,
  BOXSEARCH_OBJID_MARK1_0,// 選択リストで表示するマーキングマーク
  BOXSEARCH_OBJID_MARK1_1,
  BOXSEARCH_OBJID_MARK1_2,
  BOXSEARCH_OBJID_MARK1_3,
  BOXSEARCH_OBJID_MARK1_4,
  BOXSEARCH_OBJID_MARK1_5,
	BOXSEARCH_OBJID_ARROW_L,
	BOXSEARCH_OBJID_ARROW_R,
  BOXSEARCH_OBJ_NUM,
};

// OBJアニメインデックス定義
enum{
  BOXSEARCH_ANM_SCROLL=0,
  BOXSEARCH_ANM_CHECKMARK,
  BOXSEARCH_ANM_TOUCHBOX,
  BOXSEARCH_ANM_TOUCHBOX_ON,
  BOXSEARCH_ANM_CHECKBOX,
  BOXSEARCH_ANM_CHECKON,
  BOXSEARCH_ANM_FRAME,
  BOXSEARCH_ANM_FRAME_ON,
  BOXSEARCH_ANM_FRAME_DECIDE,
  BOXSEARCH_ANM_RESET,
  BOXSEARCH_ANM_RESET_ON,
  BOXSEARCH_ANM_RESET_DECIDE,
  BOXSEARCH_ANM_SEARCHON,
  BOXSEARCH_ANM_SEARCHON_ON,
  BOXSEARCH_ANM_SEARCHON_DECIDE,
  BOXSEARCH_ANM_SEARCHOFF,
  BOXSEARCH_ANM_SEARCHOFF_ON,
  BOXSEARCH_ANM_SEARCHOFF_DECIDE,
};

// CURSORMOVE用定義
enum{
  BOXSEARCH_PLATE_0=0,
  BOXSEARCH_PLATE_1,
  BOXSEARCH_PLATE_2,
  BOXSEARCH_PLATE_3,
  BOXSEARCH_PLATE_4,
  BOXSEARCH_PLATE_5,
  BOXSEARCH_PLATE_RESET,
  BOXSEARCH_PLATE_ON,
  BOXSEARCH_PLATE_OFF,
  BOXSEARCH_PLATE_RETURN,
  BOXSEARCH_PLATE_MAX,
};

// STRBUF定義（初期化時に読み込まれます）
enum{
  BOXSEARCH_STRID_POKEMON,        // 「ポケモン」
  BOXSEARCH_STRID_SEIKAKU,        // 「せいかく」
  BOXSEARCH_STRID_TOKUSEI,        // 「とくせい」
  BOXSEARCH_STRID_SEIBETU,        // 「せいべつ」
  BOXSEARCH_STRID_MOTIMONO,       // 「もちもの」
  BOXSEARCH_STRID_MARK,           // 「マーク」
  BOXSEARCH_STRID_NO_SEARCH,      // 「－」
  BOXSEARCH_STRID_SIBORIKOMI,     // 「ポケモン　しぼりこみ」

  BOXSEARCH_STRID_TITLE,          // タイトル「せいりする」か「バトルボックス」
  BOXSEARCH_STRID_MAX,
};

// BMPWIN用定義
enum{
  BOXSEARCH_BMPID_TITLE=0,
  BOXSEARCH_BMPID_TOPIC0,
  BOXSEARCH_BMPID_TOPIC1,
  BOXSEARCH_BMPID_TOPIC2,
  BOXSEARCH_BMPID_TOPIC3,
  BOXSEARCH_BMPID_TOPIC4,
  BOXSEARCH_BMPID_TOPIC5,
  BOXSEARCH_BMPID_ANSWER0,
  BOXSEARCH_BMPID_ANSWER1,
  BOXSEARCH_BMPID_ANSWER2,
  BOXSEARCH_BMPID_ANSWER3,
  BOXSEARCH_BMPID_ANSWER4,
  BOXSEARCH_BMPID_ANSWER5,
  BOXSEARCH_BMPID_CATEGORY,
  BOXSEARCH_BMPID_TEXT,
  BOXSEARCH_BMPID_NOW_CATEGORY,
  BOXSEARCH_BMPID_OLD_SELECT,
};

// OBJリソースID
enum{
  BOXSEARCH_OBJRES_NORMAL=0,
  BOXSEARCH_OBJRES_MARK,
	BOXSEARCH_OBJRES_ARROW,
  BOXSEARCH_OBJRES_NUM,
};

// FRAMELIST初期化タイプ指定
enum{
  BOXSEARCH_LISTID_POKEMON,        // 「ポケモン」
  BOXSEARCH_LISTID_SEIKAKU,        // 「せいかく」
  BOXSEARCH_LISTID_TOKUSEI,        // 「とくせい」
  BOXSEARCH_LISTID_SEIBETU,        // 「せいべつ」
  BOXSEARCH_LISTID_MOTIMONO,       // 「もちもの」
  BOXSEARCH_LISTID_MARK,           // 「マーク」
  BOXSEARCH_LISTID_NO_SEARCH,      // 「－」
  BOXSEARCH_LISTID_SIBORIKOMI,     // 「ポケモン　しぼりこみ」
  BOXSEARCH_LISTID_NAME_TOPWORD,
  BOXSEARCH_LISTID_NAME_NEXTWORD,
  BOXSEARCH_LISTID_TOKUSEI_TOPWORD,
  BOXSEARCH_LISTID_TOKUSEI_NEXTWORD,
};


// 検索用データ指定定義
enum{
  SEARCH_DATAID_POKEMON=0,
  SEARCH_DATAID_ITEM,
  SEARCH_DATAID_SEIKAKU,
  SEARCH_DATAID_TOKUSEI,
  SEARCH_DATAID_SEX,
  SEARCH_DATAID_MARK,
  SEARCH_DATAID_ON_OFF,
};

//=============================================================================
/**
 *                構造体定義
 */
//=============================================================================
//--------------------------------------------------------------
/// BG管理ワーク
//==============================================================
typedef struct 
{
  int dummy;
} BOX_SEARCH_BG_WORK;

#ifdef BOX_SEARCH_PMSDRAW
#define BOX_SEARCH_PMSDRAW_NUM (3) ///< 簡易会話の個数
#endif // BOX_SEARCH_PMSDRAW 


//--------------------------------------------------------------
/// BMPWIN登録用定義
//==============================================================

typedef struct{
  u32 frame;
  u8  x,y,w,h;
  u16 pal, chr;
}BMPWIN_DAT;
#include "boxsearch_bmp.dat"
#define BOX_SEARCH_BMPWIN_NUM (NELEMS(Window0BmpDataTable))

//--------------------------------------------------------------
/// メインワーク
//==============================================================
typedef struct 
{
  HEAPID heapID;

  BOX_SEARCH_PARAM        *param;
  BOX_SEARCH_BG_WORK       wk_bg;

  //描画設定
  BOX_SEARCH_GRAPHIC_WORK  *graphic;

#ifdef BOX_SEARCH_TOUCHBAR
  //タッチバー
  TOUCHBAR_WORK             *touchbar;
#endif //BOX_SEARCH_TOUCHBAR


  //フォント
  GFL_FONT        *font;

  // 図鑑セーブデータ
  ZUKAN_SAVEDATA *zukan;

  //プリントキュー
  PRINT_QUE       *print_que;
  GFL_MSGDATA     *msg;
  GFL_MSGDATA     *mons_msg;
  GFL_MSGDATA     *tokusei_msg;
  GFL_MSGDATA     *tokusei_info_msg;
  GFL_MSGDATA     *seikaku_msg;
  WORDSET         *wordset;
  STRBUF          *expand_buf;
  STRBUF          *str[BOXSEARCH_STRID_MAX];
  GFL_BMPWIN      *bmp_win[BOX_SEARCH_BMPWIN_NUM];
  PRINT_UTIL      print_util[BOX_SEARCH_BMPWIN_NUM];
  FRAMELIST_WORK  *frame_list_work;
	u32	flist_pos_max;

#ifdef BOX_SEARCH_TASKMENU
  //タスクメニュー
  APP_TASKMENU_RES          *menu_res;
  APP_TASKMENU_WORK         *menu;
#endif //BOX_SEARCH_TASKMENU



#ifdef BOX_SEARCH_POKE_ICON
  // ポケアイコン
  UI_EASY_CLWK_RES      clres_poke_icon;
  GFL_CLWK                  *clwk_poke_icon;
  // ポケアイコン用アイテムアイコン
  UI_EASY_CLWK_RES      clres_poke_item;
  GFL_CLWK                  *clwk_poke_item;
#endif //BOX_SEARCH_POKE_ICON

#ifdef BOX_SEARCH_OAM_MAPMODEL
  // OAMマップモデル
  UI_EASY_CLWK_RES      clres_oam_mmdl;
  GFL_CLWK                  *clwk_oam_mmdl;
#endif //BOX_SEARCH_OAM_MAPMODEL



#ifdef  BOX_SEARCH_PRINT_TOOL
  //プリントユーティリティ
  PRINT_UTIL                print_util;
  u32                       seq;
#endif  //BOX_SEARCH_PRINT_TOOL

#ifdef BOX_SEARCH_PMSDRAW
  GFL_BMPWIN*               pms_win[ BOX_SEARCH_PMSDRAW_NUM ];
  PMS_DRAW_WORK*            pms_draw;
#endif //BOX_SEARCH_PMSDRAW

  
  UI_EASY_CLWK_RES      clres[BOXSEARCH_OBJRES_NUM];
  GFL_CLWK              *clwk[BOXSEARCH_OBJ_NUM];
  CURSORMOVE_WORK       *cmwk;

  u8 cursor_pos;  // カーソル位置
  u8 cursor_old;  // 過去のカーソル位置

  s16 topword;    // 母音位置
  s16 nextword;  // 子音位置
  
  int subseq;
  int next_subseq;
  int main_func_pos;  // トップメニューから選んだ項目

	int	buttonID;		// アニメ中のボタンID
	int	flscroll;		// フレームリストのスクロールシーケンス

} BOX_SEARCH_MAIN_WORK;


// OBJ登録用配置・アニメ定義構造体
typedef struct{
  u8 x,y;
  u8 anm;
  u8 priority;
  u8 appear;
  u8 objres;
  u8 etc;
}CLACT_INIT_DAT;


//=============================================================================
/**
 *              データ定義
 */
//=============================================================================

// OBJリソース転送用定義
static const UI_EASY_CLWK_RES_PARAM objres_prm={
  CLSYS_DRAW_MAIN,         //  描画先タイプ
  UI_EASY_CLWK_RES_COMP_NONE,// 圧縮フラグ
  ARCID_BOX2_GRA,   ///< アーカイブNO
  NARC_box_gra_boxsearch_obj_NCLR,  ///< パレットリソースNO
  NARC_box_gra_boxsearch_obj_NCGR,   ///< キャラクタリソースNO
  NARC_box_gra_boxsearch_obj_NCER,  ///< セルリソースNO
  NARC_box_gra_boxsearch_obj_NANR,   ///< セルアニメリソースNO
  PLTID_OBJ_BOXSEARCH_M, ///< パレット転送先NO
  0,  ///< パレットデータ読み出し開始位置（パレット何本目から転送するか？ 1本=16色）
  6,  ///< パレット転送本数（何本分転送するか？ 1本=16色／0だと全て転送）
};



// OBJ登録初期化パラメータ( x, y, anm, pri, appear, objres )
static const CLACT_INIT_DAT obj_init_dat[]={
  { 184,12+24*0, BOXSEARCH_ANM_FRAME, 1, 1, BOXSEARCH_OBJRES_NORMAL,},   // プレート
  { 184,12+24*1, BOXSEARCH_ANM_FRAME, 1, 1, BOXSEARCH_OBJRES_NORMAL},
  { 184,12+24*2, BOXSEARCH_ANM_FRAME, 1, 1, BOXSEARCH_OBJRES_NORMAL},
  { 184,12+24*3, BOXSEARCH_ANM_FRAME, 1, 1, BOXSEARCH_OBJRES_NORMAL},
  { 184,12+24*4, BOXSEARCH_ANM_FRAME, 1, 1, BOXSEARCH_OBJRES_NORMAL},
  { 184,12+24*5, BOXSEARCH_ANM_FRAME, 1, 1, BOXSEARCH_OBJRES_NORMAL},
  {  48,154,     BOXSEARCH_ANM_RESET, 1, 1, BOXSEARCH_OBJRES_NORMAL},     // 「RESET」
  { 128,154,     BOXSEARCH_ANM_SEARCHON, 1, 1, BOXSEARCH_OBJRES_NORMAL},  // 「ON」
  { 208,154,     BOXSEARCH_ANM_SEARCHOFF, 1, 1, BOXSEARCH_OBJRES_NORMAL}, // 「OFF」
  { 108,154,     BOXSEARCH_ANM_CHECKBOX, 0, 1, BOXSEARCH_OBJRES_NORMAL},  // 「ON」のチェックボックス
  { 188,154,     BOXSEARCH_ANM_CHECKON,  0, 1, BOXSEARCH_OBJRES_NORMAL},  // 「OFF」のチェックボックス
  {  64,128,     BOXSEARCH_ANM_CHECKON, 2, 1, BOXSEARCH_OBJRES_NORMAL},  // チェックボックス10個
  {  64,128,     BOXSEARCH_ANM_CHECKON, 2, 1, BOXSEARCH_OBJRES_NORMAL},
  {  64,128,     BOXSEARCH_ANM_CHECKON, 2, 1, BOXSEARCH_OBJRES_NORMAL},
  {  64,128,     BOXSEARCH_ANM_CHECKON, 2, 1, BOXSEARCH_OBJRES_NORMAL},
  {  64,128,     BOXSEARCH_ANM_CHECKON, 2, 1, BOXSEARCH_OBJRES_NORMAL},
  {  64,128,     BOXSEARCH_ANM_CHECKON, 2, 1, BOXSEARCH_OBJRES_NORMAL},
  {  64,128,     BOXSEARCH_ANM_CHECKON, 2, 1, BOXSEARCH_OBJRES_NORMAL},
  {  64,128,     BOXSEARCH_ANM_CHECKON, 2, 1, BOXSEARCH_OBJRES_NORMAL},
  {  64,128,     BOXSEARCH_ANM_CHECKON, 2, 1, BOXSEARCH_OBJRES_NORMAL},
  {  64,128,     BOXSEARCH_ANM_CHECKON, 2, 1, BOXSEARCH_OBJRES_NORMAL},
  { 244, 12,     BOXSEARCH_ANM_SCROLL,   0, 1, BOXSEARCH_OBJRES_NORMAL},  // スクロールバーのつまみ
  { 17*8, 16*8,  APP_COMMON_POKE_MARK_CIRCLE_WHITE,   0, 1, BOXSEARCH_OBJRES_MARK },
  { 19*8, 16*8,  APP_COMMON_POKE_MARK_TRIANGLE_WHITE, 0, 1, BOXSEARCH_OBJRES_MARK },
  { 21*8, 16*8,  APP_COMMON_POKE_MARK_SQUARE_WHITE,   0, 1, BOXSEARCH_OBJRES_MARK },
  { 23*8, 16*8,  APP_COMMON_POKE_MARK_HEART_WHITE,    0, 1, BOXSEARCH_OBJRES_MARK },
  { 25*8, 16*8,  APP_COMMON_POKE_MARK_STAR_WHITE,     0, 1, BOXSEARCH_OBJRES_MARK },
  { 27*8, 16*8,  APP_COMMON_POKE_MARK_DIAMOND_WHITE,  0, 1, BOXSEARCH_OBJRES_MARK },
  { 22*8, (1+ 0)*8,  APP_COMMON_POKE_MARK_CIRCLE_WHITE,   0, 1, BOXSEARCH_OBJRES_MARK },
  { 22*8, (1+ 3)*8,  APP_COMMON_POKE_MARK_TRIANGLE_WHITE, 0, 1, BOXSEARCH_OBJRES_MARK },
  { 22*8, (1+ 6)*8,  APP_COMMON_POKE_MARK_SQUARE_WHITE,   0, 1, BOXSEARCH_OBJRES_MARK },
  { 22*8, (1+ 9)*8,  APP_COMMON_POKE_MARK_HEART_WHITE,    0, 1, BOXSEARCH_OBJRES_MARK },
  { 22*8, (1+12)*8,  APP_COMMON_POKE_MARK_STAR_WHITE,     0, 1, BOXSEARCH_OBJRES_MARK },
  { 22*8, (1+15)*8,  APP_COMMON_POKE_MARK_DIAMOND_WHITE,  0, 1, BOXSEARCH_OBJRES_MARK },

	{ TOUCHBAR_ICON_X_05, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_CURSOR_LEFT,  0, 1, BOXSEARCH_OBJRES_ARROW },
	{ TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_Y, APP_COMMON_BARICON_CURSOR_RIGHT,  0, 1, BOXSEARCH_OBJRES_ARROW },
};




//=============================================================================
/**
 *              プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
/// PROC
//=====================================
static GFL_PROC_RESULT BOXSEARCHProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT BOXSEARCHProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT BOXSEARCHProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
//-------------------------------------
/// 汎用処理ユーティリティ
//=====================================
static void BOXSEARCH_BG_LoadResource( BOX_SEARCH_BG_WORK* wk, HEAPID heapID );


#ifdef BOX_SEARCH_TOUCHBAR
//-------------------------------------
/// タッチバー
//=====================================
static void BOXSEARCH_TOUCHBAR_Main( TOUCHBAR_WORK  *touchbar );
//以下カスタムボタン使用サンプル用
static TOUCHBAR_WORK * BOXSEARCH_TOUCHBAR_InitEx( BOX_SEARCH_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID );
static void BOXSEARCH_TOUCHBAR_ExitEx( BOX_SEARCH_MAIN_WORK *wk );
#endif //BOX_SEARCH_TOUCHBAR


#ifdef BOX_SEARCH_TASKMENU
//-------------------------------------
/// リストシステムはい、いいえ
//=====================================
static APP_TASKMENU_WORK * BOXSEARCH_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID );
static void BOXSEARCH_TASKMENU_Exit( APP_TASKMENU_WORK *menu );
static void BOXSEARCH_TASKMENU_Main( APP_TASKMENU_WORK *menu );
#endif //BOX_SEARCH_TASKMENU



#ifdef BOX_SEARCH_POKE_ICON
//-------------------------------------
/// ポケアイコン
//=====================================
static void BOXSEARCH_POKE_ICON_CreateCLWK( BOX_SEARCH_MAIN_WORK* wk, u32 mons, u32 form_no, u32 sex, BOOL egg, GFL_CLUNIT* unit, HEAPID heapID );
static void BOXSEARCH_POKE_ICON_DeleteCLWK( BOX_SEARCH_MAIN_WORK* wk );
#endif //BOX_SEARCH_POKE_ICON

#ifdef BOX_SEARCH_OAM_MAPMODEL
//-------------------------------------
/// OAMでマップモデル表示
//=====================================
static void BOXSEARCH_OAM_MAPMODEL_CreateCLWK( BOX_SEARCH_MAIN_WORK *wk, u16 tex_idx, u8 ptn_ofs, GFL_CLUNIT *unit, HEAPID heapID );
static void BOXSEARCH_OAM_MAPMODEL_DeleteCLWK( BOX_SEARCH_MAIN_WORK* wk );
#endif //BOX_SEARCH_OAM_MAPMODEL


#ifdef BOX_SEARCH_PMSDRAW
//-------------------------------------
/// 簡易会話表示
//=====================================
static void BOXSEARCH_PMSDRAW_Init( BOX_SEARCH_MAIN_WORK* wk );
static void BOXSEARCH_PMSDRAW_Exit( BOX_SEARCH_MAIN_WORK* wk );
static void BOXSEARCH_PMSDRAW_Proc( BOX_SEARCH_MAIN_WORK* wk );
#endif // BOX_SEARCH_PMSDRAW


static void SearchBmpwinInit( BOX_SEARCH_MAIN_WORK *wk );
static void SearchBmpinExit( BOX_SEARCH_MAIN_WORK *wk );
static void ObjInit( BOX_SEARCH_MAIN_WORK *wk, GFL_CLUNIT *unit, HEAPID heapID );
static void ObjExit( BOX_SEARCH_MAIN_WORK *wk );
static void StrbufLoad( BOX_SEARCH_MAIN_WORK *wk );
static void StrbufRelease( BOX_SEARCH_MAIN_WORK *wk );
static void PrintStrbuf( BOX_SEARCH_MAIN_WORK *wk, int bmpid, int strbufID, int y );
static void PrintMessage( BOX_SEARCH_MAIN_WORK *wk, int bmpid, int msgID, int y );
static void FrameListInit(BOX_SEARCH_MAIN_WORK *wk, int type);
static void FrameListExit(BOX_SEARCH_MAIN_WORK *wk);
static void FrameListClear( BOX_SEARCH_MAIN_WORK * wk );
static void FrameListChangeClear( BOX_SEARCH_MAIN_WORK * wk );

static void cm_dummy_callback( void *work, int pos, int old );
static void touch_callback( void *work, int pos, int old );
static void cursor_move_callback( void *work, int pos, int old );
static void cursor_off_callback( void *work, int pos, int old );
static void cursor_on_callback( void *work, int pos, int old );
static void change_search_checkbox( BOX_SEARCH_MAIN_WORK *wk, int pos );
static void put_search_checkbox( BOX_SEARCH_MAIN_WORK *wk );
static void _set_subseq_nextseq( BOX_SEARCH_MAIN_WORK *wk,  int next );
static u16* ZukanSortDataGet( int heap, int* p_arry_num );
static void TransMainBGScreen( BOX_SEARCH_MAIN_WORK *wk, int type );
static void SetupScreen( BOX_SEARCH_MAIN_WORK *wk, int type, int category );
static int  _get_search_data( BOX_SEARCH_MAIN_WORK *wk, int type );
static void _set_search_data( BOX_SEARCH_MAIN_WORK *wk, int type, int param );
static void _set_search_data_mark( BOX_SEARCH_MAIN_WORK *wk, int mark, int param );
static BOOL _get_search_data_mark( BOX_SEARCH_MAIN_WORK *wk, int mark );
static void _strbuf_print( BOX_SEARCH_MAIN_WORK *wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int param );
static void PrintStrBuf( BOX_SEARCH_MAIN_WORK * wk, PRINT_UTIL * print_util, PRINT_QUE * que, GFL_MSGDATA * mman, int strIdx, PRINTSYS_LSB col, int y );
static void SetObjAutoAnime( BOX_SEARCH_MAIN_WORK * wk, u32 id, u32 anm );
static void SetMarkDataObj( BOX_SEARCH_MAIN_WORK *wk, int objpos );
static void PutMarkDataObj( BOX_SEARCH_MAIN_WORK * wk, int id );
static void SetMarkDataObjDrawEnable( BOX_SEARCH_MAIN_WORK * wk, int id, BOOL flg );
static void InitScrollBarObj( BOX_SEARCH_MAIN_WORK * wk );
static void PutScrollBarObj( BOX_SEARCH_MAIN_WORK * wk );
static void PrintDefautltSubDisp( BOX_SEARCH_MAIN_WORK * wk );
static void SetupMainMode( BOX_SEARCH_MAIN_WORK *wk );
static void SetDrawEnableMainItemObj( BOX_SEARCH_MAIN_WORK * wk, BOOL flg );
static void _reset_search_info( BOX_SEARCH_MAIN_WORK *wk );
static void DecideInfoPrint( BOX_SEARCH_MAIN_WORK * wk, BOOL flag );
static void SubTextPrint( BOX_SEARCH_MAIN_WORK *wk, int subseq );
static void BmpWinTransScreen( GFL_BMPWIN * bmpwin );
static int SetButtonAnime( BOX_SEARCH_MAIN_WORK * wk, int buttonID, int anmID, int next );
static int SetFrameListScroll( BOX_SEARCH_MAIN_WORK * wk, int buttonID, int anmID, int next );
static void PutListCheckObj( BOX_SEARCH_MAIN_WORK * wk, int id, s16 y );
static void PutCheckObj( BOX_SEARCH_MAIN_WORK * wk, int id, int pos );
static void ScrollListCheckObj( BOX_SEARCH_MAIN_WORK * wk, s8 mv );
static void SetListPageArrowAnime( BOX_SEARCH_MAIN_WORK * wk, BOOL anm );
static BOOL CheckCancelButton( BOX_SEARCH_MAIN_WORK * wk );
static int HitCheckListBox( BOX_SEARCH_MAIN_WORK * wk );
static int get_sort_tokusei_num( int head, int child );
static void PrintTokuseiInfo( BOX_SEARCH_MAIN_WORK * wk, u32 id );

static int Subseq_Start(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_Main(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_Return(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemonInit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemon1(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemon1Exit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemon2Init(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemon2(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemon2Exit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemon3Init(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemon3(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectPokemon3Exit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectSeikakuInit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectSeikaku(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectSeikakuExit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokuseiInit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokusei1(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokusei1Exit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokusei2Init(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokusei2(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokusei2Exit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokusei3Init(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokusei3(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectTokusei3Exit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectSexInit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectSex(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectSexExit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectItemHaveInit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectItemHave(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectItemHaveExit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectMarkInit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectMark(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SelectMarkExit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SearchOn(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SearchOnEnd( BOX_SEARCH_MAIN_WORK * wk, int seq );
static int Subseq_SearchOff(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_SearchReset(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_AnimeWait(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_FramelistInitWait(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_FrameListScroll( BOX_SEARCH_MAIN_WORK * wk, int seq );
static int Subseq_SearchExit(BOX_SEARCH_MAIN_WORK *wk, int seq);
static int Subseq_ButtonAnime( BOX_SEARCH_MAIN_WORK * wk, int seq );
static int Subseq_SelectSet( BOX_SEARCH_MAIN_WORK * wk, int seq );


//=============================================================================
// サブシーケンスENUM定義
//=============================================================================
enum{
  SUBSEQ_START=0,             // 00: 初期画面開始
  SUBSEQ_MAIN,                // 01: 初期画面メイン
  SUBSEQ_RETURN,              //
  SUBSEQ_SELECTPOKEMONINIT,   //
  SUBSEQ_SELECTPOKEMON1,      //
  SUBSEQ_SELECTPOKEMON1EXIT,  //
  SUBSEQ_SELECTPOKEMON2INIT,  //
  SUBSEQ_SELECTPOKEMON2,      //
  SUBSEQ_SELECTPOKEMON2EXIT,  //
  SUBSEQ_SELECTPOKEMON3INIT,  //
  SUBSEQ_SELECTPOKEMON3,      //10
  SUBSEQ_SELECTPOKEMON3EXIT,  //
  SUBSEQ_SELECTSEIKAKUINIT,   //
  SUBSEQ_SELECTSEIKAKU,       //
  SUBSEQ_SELECTSEIKAKUEXIT,   //
  SUBSEQ_SELECTTOKUSEIINIT,   //
  SUBSEQ_SELECTTOKUSEI1,      //
  SUBSEQ_SELECTTOKUSEI1EXIT,  //
  SUBSEQ_SELECTTOKUSEI2INIT,  //
  SUBSEQ_SELECTTOKUSEI2,      //
  SUBSEQ_SELECTTOKUSEI2EXIT,  //20
  SUBSEQ_SELECTTOKUSEI3INIT,  //
  SUBSEQ_SELECTTOKUSEI3,      //
  SUBSEQ_SELECTTOKUSEI3EXIT,  //
  SUBSEQ_SELECTSEXINIT,       //
  SUBSEQ_SELECTSEX,           //
  SUBSEQ_SELECTSEXEXIT,       //
  SUBSEQ_SELECTITEMHAVEINIT,  //
  SUBSEQ_SELECTITEMHAVE,      //
  SUBSEQ_SELECTITEMHAVEEXIT,  //
  SUBSEQ_SELECTMARKINIT,      //30
  SUBSEQ_SELECTMARK,          //
  SUBSEQ_SELECTMARKEXIT,      //
  SUBSEQ_SEARCHON,            // 33: 検索条件ON
  SUBSEQ_SEARCHONEND,         // 34: 検索条件ON
  SUBSEQ_SEARCHOFF,           // 35: 検索条件OFF
  SUBSEQ_SEARCHRESET,         // 36: 検索条件リセット
  SUBSEQ_ANIMEWAIT,           // 
  SUBSEQ_FRAMELISTINITWAIT,   //
  SUBSEQ_FRAMELISTSCROLL,		  //
  SUBSEQ_SEARCHEXIT,          // 40
	SUBSEQ_BUTTON_ANIME,				// 41: ボタンアニメ終了待ち
	SUBSEQ_SELECT_SET,					// 42: 各検索条件選択へ
};

//=============================================================================
/**
 *                サブシーケンス関数テーブル
 */
//=============================================================================
// 関数配列定義
static int (* const FuncTable[])(BOX_SEARCH_MAIN_WORK *wk, int seq)={
  Subseq_Start,                 // 0
  Subseq_Main,                  // 
  Subseq_Return,                // 
  Subseq_SelectPokemonInit,     // 
  Subseq_SelectPokemon1,        // 
  Subseq_SelectPokemon1Exit,    // 
  Subseq_SelectPokemon2Init,    // 
  Subseq_SelectPokemon2,        // 
  Subseq_SelectPokemon2Exit,    // 
  Subseq_SelectPokemon3Init,    // 
  Subseq_SelectPokemon3,        // 10
  Subseq_SelectPokemon3Exit,    // 
  Subseq_SelectSeikakuInit,     // 
  Subseq_SelectSeikaku,         // 
  Subseq_SelectSeikakuExit,     // 
  Subseq_SelectTokuseiInit,     // 
  Subseq_SelectTokusei1,        // 
  Subseq_SelectTokusei1Exit,    // 
  Subseq_SelectTokusei2Init,    // 
  Subseq_SelectTokusei2,        // 
  Subseq_SelectTokusei2Exit,    // 20
  Subseq_SelectTokusei3Init,    // 
  Subseq_SelectTokusei3,        // 
  Subseq_SelectTokusei3Exit,    // 
  Subseq_SelectSexInit,         // 
  Subseq_SelectSex,             // 
  Subseq_SelectSexExit,         // 
  Subseq_SelectItemHaveInit,    // 
  Subseq_SelectItemHave,        // 
  Subseq_SelectItemHaveExit,    // 
  Subseq_SelectMarkInit,        // 30
  Subseq_SelectMark,            // 
  Subseq_SelectMarkExit,        // 
  Subseq_SearchOn,              // 
  Subseq_SearchOnEnd,           // 
  Subseq_SearchOff,             // 
  Subseq_SearchReset,           // 
  Subseq_AnimeWait,             // 
  Subseq_FramelistInitWait,     // 
	Subseq_FrameListScroll,
  Subseq_SearchExit,            // 
	Subseq_ButtonAnime,
	Subseq_SelectSet,
};




static const u16 main_scr_bg_table[]={
  NARC_box_gra_boxsearchbase_bgd_1_NSCR,
  NARC_box_gra_boxsearchbase_bgd_2_on_NSCR,
  NARC_box_gra_boxsearchbase_bgd_2_off_NSCR,
};


// トップ画面マーキング、入力用マーキング、スクロールつまみの制御
static const u8 onoff_tbl[][3]={
  { FALSE, FALSE, TRUE  }, // BOXSEARCH_LISTID_POKEMON,        // 「ポケモン」
  { FALSE, FALSE, TRUE  }, // BOXSEARCH_LISTID_SEIKAKU,        // 「せいかく」
  { FALSE, FALSE, TRUE  }, // BOXSEARCH_LISTID_TOKUSEI,        // 「とくせい」
  { FALSE, FALSE, FALSE }, // BOXSEARCH_LISTID_SEIBETU,        // 「せいべつ」
  { FALSE, FALSE, FALSE }, // BOXSEARCH_LISTID_MOTIMONO,       // 「もちもの」
  { FALSE, TRUE,  FALSE }, // BOXSEARCH_LISTID_MARK,           // 「マーク」
  { FALSE, FALSE, TRUE  }, // BOXSEARCH_LISTID_NO_SEARCH,      // 「－」
  { FALSE, FALSE, TRUE  }, // BOXSEARCH_LISTID_SIBORIKOMI,     // 「ポケモン　しぼりこみ」
  { FALSE, FALSE, TRUE  }, // BOXSEARCH_LISTID_NAME_TOPWORD,   // 名前母音
  { FALSE, FALSE, FALSE }, // BOXSEARCH_LISTID_NAME_NEXTWORD,  // 名前子音
  { FALSE, FALSE, TRUE  }, // BOXSEARCH_LISTID_TOKUSEI_TOPWORD,// 特性母音
  { FALSE, FALSE, FALSE }, // BOXSEARCH_LISTID_TOKUSEI_NEXTWORD,// 特性子音
};

static const u8 now_category_tbl[]={
  BOXSEARCH_STRID_POKEMON, // BOXSEARCH_LISTID_POKEMON,        
  BOXSEARCH_STRID_SEIKAKU, // BOXSEARCH_LISTID_SEIKAKU,        
  BOXSEARCH_STRID_TOKUSEI, // BOXSEARCH_LISTID_TOKUSEI,        
  BOXSEARCH_STRID_SEIBETU, // BOXSEARCH_LISTID_SEIBETU,        
  BOXSEARCH_STRID_MOTIMONO,// BOXSEARCH_LISTID_MOTIMONO,       
  BOXSEARCH_STRID_MARK,    // BOXSEARCH_LISTID_MARK,           
  0, // BOXSEARCH_LISTID_NO_SEARCH,      
  0, // BOXSEARCH_LISTID_SIBORIKOMI,     
  BOXSEARCH_STRID_POKEMON, // BOXSEARCH_LISTID_NAME_TOPWORD,   
  BOXSEARCH_STRID_POKEMON, // BOXSEARCH_LISTID_NAME_NEXTWORD,  
  BOXSEARCH_STRID_TOKUSEI, // BOXSEARCH_LISTID_TOKUSEI_TOPWORD,
  BOXSEARCH_STRID_TOKUSEI, // BOXSEARCH_LISTID_TOKUSEI_NEXTWORD
};


static const u16 topword_tbl[]={
  mes_box_search_hyphen_01,
  mes_box_search_initial_01,
  mes_box_search_initial_06,
  mes_box_search_initial_11,
  mes_box_search_initial_16,
  mes_box_search_initial_21,
  mes_box_search_initial_26,
  mes_box_search_initial_31,
  mes_box_search_initial_36,
  mes_box_search_initial_39,
  mes_box_search_initial_44,
};

static const u8 childword_num[]={
  5,  // ア
  5,  // カ
  5,  // サ
  5,  // タ
  5,  // ナ
  5,  // ハ
  5,  // マ
  3,  // ヤ
  5,  // ラ
  1,  // ワ
};

static const int head2pokename[]={
  0,  // あ
  5,  // か
  10, // さ
  15, // た
  20, // な
  25, // は
  30, // ま
  35, // や
  38, // ら
  43, // わ
};


static const u16 sort_pokename_start_index[]={
    ZKN_SORT_AIUEO_IDX_START_A  ,
    ZKN_SORT_AIUEO_IDX_START_I  ,
    ZKN_SORT_AIUEO_IDX_START_U  ,
    ZKN_SORT_AIUEO_IDX_START_E  ,
    ZKN_SORT_AIUEO_IDX_START_O  ,
    ZKN_SORT_AIUEO_IDX_START_KA   ,
    ZKN_SORT_AIUEO_IDX_START_KI   ,
    ZKN_SORT_AIUEO_IDX_START_KU   ,
    ZKN_SORT_AIUEO_IDX_START_KE   ,
    ZKN_SORT_AIUEO_IDX_START_KO   ,
    ZKN_SORT_AIUEO_IDX_START_SA   ,
    ZKN_SORT_AIUEO_IDX_START_SI   ,
    ZKN_SORT_AIUEO_IDX_START_SU   ,
    ZKN_SORT_AIUEO_IDX_START_SE   ,
    ZKN_SORT_AIUEO_IDX_START_SO   ,
    ZKN_SORT_AIUEO_IDX_START_TA   ,
    ZKN_SORT_AIUEO_IDX_START_TI   ,
    ZKN_SORT_AIUEO_IDX_START_TU   ,
    ZKN_SORT_AIUEO_IDX_START_TE   ,
    ZKN_SORT_AIUEO_IDX_START_TO   ,
    ZKN_SORT_AIUEO_IDX_START_NA   ,
    ZKN_SORT_AIUEO_IDX_START_NI   ,
    ZKN_SORT_AIUEO_IDX_START_NU   ,
    ZKN_SORT_AIUEO_IDX_START_NE   ,
    ZKN_SORT_AIUEO_IDX_START_NO   ,
    ZKN_SORT_AIUEO_IDX_START_HA   ,
    ZKN_SORT_AIUEO_IDX_START_HI   ,
    ZKN_SORT_AIUEO_IDX_START_HU   ,
    ZKN_SORT_AIUEO_IDX_START_HE   ,
    ZKN_SORT_AIUEO_IDX_START_HO   ,
    ZKN_SORT_AIUEO_IDX_START_MA   ,
    ZKN_SORT_AIUEO_IDX_START_MI   ,
    ZKN_SORT_AIUEO_IDX_START_MU   ,
    ZKN_SORT_AIUEO_IDX_START_ME   ,
    ZKN_SORT_AIUEO_IDX_START_MO   ,
    ZKN_SORT_AIUEO_IDX_START_YA   ,
    ZKN_SORT_AIUEO_IDX_START_YU   ,
    ZKN_SORT_AIUEO_IDX_START_YO   ,
    ZKN_SORT_AIUEO_IDX_START_RA   ,
    ZKN_SORT_AIUEO_IDX_START_RI   ,
    ZKN_SORT_AIUEO_IDX_START_RU   ,
    ZKN_SORT_AIUEO_IDX_START_RE   ,
    ZKN_SORT_AIUEO_IDX_START_RO   ,
    ZKN_SORT_AIUEO_IDX_START_WA   ,
};

static const u16 sort_pokename_end_index[]={
    ZKN_SORT_AIUEO_IDX_END_A  ,
    ZKN_SORT_AIUEO_IDX_END_I  ,
    ZKN_SORT_AIUEO_IDX_END_U  ,
    ZKN_SORT_AIUEO_IDX_END_E  ,
    ZKN_SORT_AIUEO_IDX_END_O  ,
    ZKN_SORT_AIUEO_IDX_END_KA   ,
    ZKN_SORT_AIUEO_IDX_END_KI   ,
    ZKN_SORT_AIUEO_IDX_END_KU   ,
    ZKN_SORT_AIUEO_IDX_END_KE   ,
    ZKN_SORT_AIUEO_IDX_END_KO   ,
    ZKN_SORT_AIUEO_IDX_END_SA   ,
    ZKN_SORT_AIUEO_IDX_END_SI   ,
    ZKN_SORT_AIUEO_IDX_END_SU   ,
    ZKN_SORT_AIUEO_IDX_END_SE   ,
    ZKN_SORT_AIUEO_IDX_END_SO   ,
    ZKN_SORT_AIUEO_IDX_END_TA   ,
    ZKN_SORT_AIUEO_IDX_END_TI   ,
    ZKN_SORT_AIUEO_IDX_END_TU   ,
    ZKN_SORT_AIUEO_IDX_END_TE   ,
    ZKN_SORT_AIUEO_IDX_END_TO   ,
    ZKN_SORT_AIUEO_IDX_END_NA   ,
    ZKN_SORT_AIUEO_IDX_END_NI   ,
    ZKN_SORT_AIUEO_IDX_END_NU   ,
    ZKN_SORT_AIUEO_IDX_END_NE   ,
    ZKN_SORT_AIUEO_IDX_END_NO   ,
    ZKN_SORT_AIUEO_IDX_END_HA   ,
    ZKN_SORT_AIUEO_IDX_END_HI   ,
    ZKN_SORT_AIUEO_IDX_END_HU   ,
    ZKN_SORT_AIUEO_IDX_END_HE   ,
    ZKN_SORT_AIUEO_IDX_END_HO   ,
    ZKN_SORT_AIUEO_IDX_END_MA   ,
    ZKN_SORT_AIUEO_IDX_END_MI   ,
    ZKN_SORT_AIUEO_IDX_END_MU   ,
    ZKN_SORT_AIUEO_IDX_END_ME   ,
    ZKN_SORT_AIUEO_IDX_END_MO   ,
    ZKN_SORT_AIUEO_IDX_END_YA   ,
    ZKN_SORT_AIUEO_IDX_END_YU   ,
    ZKN_SORT_AIUEO_IDX_END_YO   ,
    ZKN_SORT_AIUEO_IDX_END_RA   ,
    ZKN_SORT_AIUEO_IDX_END_RI   ,
    ZKN_SORT_AIUEO_IDX_END_RU   ,
    ZKN_SORT_AIUEO_IDX_END_RE   ,
    ZKN_SORT_AIUEO_IDX_END_RO   ,
    ZKN_SORT_AIUEO_IDX_END_WA   ,
};

#include "../../../resource/tokusei/tokusei_aiueo_table.cdat"
#include "../../../resource/tokusei/tokusei_aiueo_header_table.cdat"

static const u16 sex_msg_tbl[]={
  mes_box_search_hyphen_01, // ー  （絞り込み対象にしない）
  mes_box_search_sex_01,    // ♂
  mes_box_search_sex_02,    // ♀
  mes_box_search_sex_03,    // せいべつなし
};

static const u16 item_msg_tbl[]={
  mes_box_search_hyphen_01, // ー  （絞り込み対象にしない）
  mes_box_search_item_02,   // もっていない
  mes_box_search_item_01,   // もっている
};




//=============================================================================
/**
 *                外部公開
 */
//=============================================================================
const GFL_PROC_DATA BOXSEARCHProcData = 
{
  BOXSEARCHProc_Init,
  BOXSEARCHProc_Main,
  BOXSEARCHProc_Exit,
};
//=============================================================================
/**
 *                PROC
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 初期化処理
 *
 *  @param  GFL_PROC *procプロセスシステム
 *  @param  *seq          シーケンス
 *  @param  *pwk          BOX_SEARCH_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BOXSEARCHProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
  BOX_SEARCH_MAIN_WORK *wk;
  BOX_SEARCH_PARAM *param;

  //オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
  
  //引数取得
  param = pwk;

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BOX_SEARCH, BOX_SEARCH_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(BOX_SEARCH_MAIN_WORK), HEAPID_BOX_SEARCH );
  GFL_STD_MemClear( wk, sizeof(BOX_SEARCH_MAIN_WORK) );

  wk->param = param;

  // 初期化
  wk->heapID = HEAPID_BOX_SEARCH;

  // 図鑑セーブデータ取得
  wk->zukan = GAMEDATA_GetZukanSave( wk->param->syswk->dat->gamedata );
  
  //描画設定初期化
  wk->graphic = BOX_SEARCH_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heapID );

  //フォント作成
  wk->font      = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
                        GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
  

  //メッセージ
  wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                            NARC_message_boxmenu_dat, wk->heapID );

  wk->seikaku_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                                    NARC_message_chr_dat, wk->heapID );
  wk->tokusei_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                                    NARC_message_tokusei_dat, wk->heapID );
  wk->tokusei_info_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                                    NARC_message_tokuseiinfo_dat, wk->heapID );
  wk->mons_msg    = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                                    NARC_message_monsname_dat, wk->heapID );


  //PRINT_QUE作成
  wk->print_que   = PRINTSYS_QUE_Create( wk->heapID );

  wk->wordset = WORDSET_Create( wk->heapID );

  //BGリソース読み込み
  BOXSEARCH_BG_LoadResource( &wk->wk_bg, wk->heapID );

  // メイン画面のBGスクリーンを転送する
//  TransMainBGScreen( wk, 0 );

  // 文字列読み込み
  StrbufLoad( wk );


#ifdef BOX_SEARCH_TOUCHBAR
  //タッチバーの初期化
  { 
    GFL_CLUNIT  *clunit = BOX_SEARCH_GRAPHIC_GetClunit( wk->graphic );
    wk->touchbar  = BOXSEARCH_TOUCHBAR_InitEx( wk, clunit, wk->heapID );
  }
#endif //BOX_SEARCH_TOUCHBAR


#ifdef BOX_SEARCH_TASKMENU
  //TASKMENUリソース読み込み＆初期化
  wk->menu_res  = APP_TASKMENU_RES_Create( BG_FRAME_BAR_M, PLTID_BG_TASKMENU_M, wk->font, wk->print_que, wk->heapID );
  wk->menu      = BOXSEARCH_TASKMENU_Init( wk->menu_res, wk->msg, wk->heapID );
#endif //BOX_SEARCH_TASKMENU


#ifdef BOX_SEARCH_OAM_MAPMODEL
  //OAMマップモデルの読み込み
  {
    // 主人公は現状 0=後ろ, 1=後ろアニメ1, 2=後ろアニメ2, 3=正面....
    // トレーナーなどは違う可能性があるので注意。
    u16 ptn_ofs = 3;
    GFL_CLUNIT  *clunit = BOX_SEARCH_GRAPHIC_GetClunit( wk->graphic );

    BOXSEARCH_OAM_MAPMODEL_CreateCLWK( wk, NARC_fldmmdl_mdlres_hero_nsbtx, ptn_ofs, clunit, wk->heapID );
  }
#endif //BOX_SEARCH_OAM_MAPMODEL

#ifdef BOX_SEARCH_POKE_ICON
  // ポケアイコンの読み込み
  {
    GFL_CLUNIT  *clunit = BOX_SEARCH_GRAPHIC_GetClunit( wk->graphic );
    BOXSEARCH_POKE_ICON_CreateCLWK( wk, MONSNO_HUSIGIDANE, 0, 0, FALSE, clunit, wk->heapID );
  }
#endif //BOX_SEARCH_POKE_ICON

  // 絞り込みOBJ表示準備
  {
    GFL_CLUNIT  *clunit = BOX_SEARCH_GRAPHIC_GetClunit( wk->graphic );
    ObjInit( wk, clunit, wk->heapID );
  }
  // 文字列描画用領域確保
  SearchBmpwinInit( wk );


#ifdef BOX_SEARCH_PMSDRAW  
  BOXSEARCH_PMSDRAW_Init( wk );
#endif //BOX_SEARCH_PMSDRAW

  
  // 初期状態描画
//  SetupMainMode( wk );
	PrintDefautltSubDisp( wk );
	Subseq_Start( wk, 0 );

  WIPE_SYS_Start(
    WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
    WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );

//  wk->subseq = 0;
  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 終了処理
 *
 *  @param  GFL_PROC *procプロセスシステム
 *  @param  *seq          シーケンス
 *  @param  *pwk          BOX_SEARCH_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BOXSEARCHProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
  BOX_SEARCH_MAIN_WORK* wk = mywk;


  // 文字列破棄
  StrbufRelease( wk );

  // OBJ解放
  ObjExit(wk);

  // BMPWIN解放
  SearchBmpinExit( wk );


#ifdef BOX_SEARCH_OAM_MAPMODEL
  //OAMマップモデルの破棄
  BOXSEARCH_OAM_MAPMODEL_DeleteCLWK( wk );
#endif //BOX_SEARCH_OAM_MAPMODEL

#ifdef BOX_SEARCH_POKE_ICON
   BOXSEARCH_POKE_ICON_DeleteCLWK( wk );
#endif //BOX_SEARCH_POKE_ICON

#ifdef BOX_SEARCH_TASKMENU
  //TASKMENUシステム＆リソース破棄
  BOXSEARCH_TASKMENU_Exit( wk->menu );
  APP_TASKMENU_RES_Delete( wk->menu_res );  
#endif //BOX_SEARCH_TASKMENU


#ifdef BOX_SEARCH_TOUCHBAR
  //タッチバー
  BOXSEARCH_TOUCHBAR_ExitEx( wk );
#endif //BOX_SEARCH_TOUCHBAR


#ifdef BOX_SEARCH_PMSDRAW
  BOXSEARCH_PMSDRAW_Exit( wk );
#endif //BOX_SEARCH_PMSDRAW

  // WORDSET破棄
  WORDSET_Delete( wk->wordset );


  //メッセージ破棄
  GFL_MSG_Delete( wk->mons_msg );
  GFL_MSG_Delete( wk->seikaku_msg );
  GFL_MSG_Delete( wk->tokusei_msg );
  GFL_MSG_Delete( wk->tokusei_info_msg );
  GFL_MSG_Delete( wk->msg );

  //PRINT_QUE
  PRINTSYS_QUE_Delete( wk->print_que );

  //FONT
  GFL_FONT_Delete( wk->font );

  //描画設定破棄
  BOX_SEARCH_GRAPHIC_Exit( wk->graphic );

  //PROC用メモリ解放
  {
    HEAPID heapID = wk->heapID;
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( heapID );
  }

  //オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 主処理
 *
 *  @param  GFL_PROC *procプロセスシステム
 *  @param  *seq          シーケンス
 *  @param  *pwk          BOX_SEARCH_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT BOXSEARCHProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
  BOX_SEARCH_MAIN_WORK* wk = mywk;
  int i;
  switch(*seq){
  case SEQ_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE ){
      *seq = SEQ_MAIN;
//      wk->subseq = SUBSEQ_START;
      wk->subseq = SUBSEQ_MAIN;
    }
    break;
  case SEQ_MAIN:
    if(PRINTSYS_QUE_IsFinished( wk->print_que )){
      wk->subseq = FuncTable[wk->subseq](wk,wk->subseq);
      if(wk->subseq==SUBSEQ_SEARCHEXIT){
        *seq = SEQ_FADEOUT;;
      }
//    KeyControl( wk );
//    BGControl( wk );

    }
    break;
  case SEQ_FADEOUT:
    WIPE_SYS_Start(
      WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
      WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );
    *seq = SEQ_FADEOUT_WAIT;
    break;
  case SEQ_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE ){
      return GFL_PROC_RES_FINISH;
    }
    break;
  }


#ifdef BOX_SEARCH_TOUCHBAR
  //タッチバーメイン処理
  BOXSEARCH_TOUCHBAR_Main( wk->touchbar );
#endif //BOX_SEARCH_TOUCHBAR


#ifdef BOX_SEARCH_TASKMENU
  //タスクメニューメイン処理
  BOXSEARCH_TASKMENU_Main( wk->menu );
#endif //BOX_SEARCH_TASKMENU

  //PRINT_QUE
  PRINTSYS_QUE_Main( wk->print_que );
  for(i=0;i<BOX_SEARCH_BMPWIN_NUM;i++){
    PRINT_UTIL_Trans( &wk->print_util[i], wk->print_que );
  }

#ifdef BOX_SEARCH_PMSDRAW
  BOXSEARCH_PMSDRAW_Proc( wk );
#endif //BOX_SEARCH_PMSDRAW

  //2D描画
  BOX_SEARCH_GRAPHIC_2D_Draw( wk->graphic );

  //3D描画
  BOX_SEARCH_GRAPHIC_3D_StartDraw( wk->graphic );

  BOX_SEARCH_GRAPHIC_3D_EndDraw( wk->graphic );

  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *                static関数
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *  @brief  BG管理モジュール リソース読み込み
 *
 *  @param  BOX_SEARCH_BG_WORK* wk BG管理ワーク
 *  @param  heapID  ヒープID 
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_BG_LoadResource( BOX_SEARCH_BG_WORK* wk, HEAPID heapID )
{
  //@TODO とりあえずマイクテストのリソース
  //@TODO 複数枚ロードできるようにして切り離す？
  enum
  {
    RES_ARC = ARCID_BOX2_GRA,

    RES_PAL_S_SIZE = 0x20,
    RES_PAL_M_SIZE = 0x20*4,

    RES_PAL_S_ID = NARC_box_gra_boxsearch_bgu_NCLR,
    RES_CHR_S_ID = NARC_box_gra_boxsearch_bgu_NCGR,
    RES_SCR_S_ID = NARC_box_gra_boxsearchbase_bgu_NSCR,
    RES_PLATE_S_ID = NARC_box_gra_boxsearch_frame_bgu_NSCR,

    RES_PAL_M_ID = NARC_box_gra_boxsearch_bgd_NCLR,
    RES_CHR_M_ID = NARC_box_gra_boxsearch_bgd_NCGR,

  };

  ARCHANDLE *handle;

  handle  = GFL_ARC_OpenDataHandle( RES_ARC, heapID );

  // 上下画面ＢＧパレット転送
  GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_M_ID, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, RES_PAL_M_SIZE, heapID );
  GFL_ARCHDL_UTIL_TransVramPalette( handle, RES_PAL_S_ID, PALTYPE_SUB_BG,  PLTID_BG_BACK_S, RES_PAL_S_SIZE, heapID );
  
  //  ----- SUB -----
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, RES_CHR_S_ID, BG_FRAME_BACK_S, 0, 0, 0, heapID );
  GFL_ARCHDL_UTIL_TransVramScreen(  handle, RES_SCR_S_ID,   BG_FRAME_BACK_S,  0, 0, 0, heapID ); 
  GFL_ARCHDL_UTIL_TransVramScreen(  handle, RES_PLATE_S_ID, BG_FRAME_PLATE_S, 0, 0, 0, heapID ); 

  //  ----- MAIN -----
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, RES_CHR_M_ID,  BG_FRAME_BACK_M, 0, 0, 0, heapID );

  GFL_ARC_CloseDataHandle( handle );

  // フォントパレット転送
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 14*0x20, 16*2, heapID );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,  14*0x20, 16*2, heapID );

}






#ifdef BOX_SEARCH_TOUCHBAR
//=============================================================================
/**
 *  TOUCHBAR
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBAR初期化  + カスタムアイコン版
 *
 * @param  wk                   ワーク
 *  @param  GFL_CLUNIT *clunit  CLUNIT
 *  @param  heapID              ヒープID
 *
 *  @return TOUCHBAR_WORK
 */
//-----------------------------------------------------------------------------
static TOUCHBAR_WORK * BOXSEARCH_TOUCHBAR_InitEx( BOX_SEARCH_MAIN_WORK *wk, GFL_CLUNIT *clunit, HEAPID heapID )
{
	TOUCHBAR_SETUP	touchbar_setup;
	TOUCHBAR_ITEM_ICON	touchbar_icon_tbl[] =
	{
		{ TOUCHBAR_ICON_RETURN,	{ TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y } },
	};

	GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

	touchbar_setup.p_item   = touchbar_icon_tbl;        //上の窓情報
	touchbar_setup.item_num = NELEMS(touchbar_icon_tbl);//いくつ窓があるか
	touchbar_setup.p_unit   = clunit;                   //OBJ読み込みのためのCLUNIT
	touchbar_setup.bar_frm  = BG_FRAME_BAR_M;           //BG読み込みのためのBG面
	touchbar_setup.bg_plt   = PLTID_BG_TOUCHBAR_M;      //BGﾊﾟﾚｯﾄ
	touchbar_setup.obj_plt  = PLTID_OBJ_TOUCHBAR_M;     //OBJﾊﾟﾚｯﾄ
	touchbar_setup.mapping  = APP_COMMON_MAPPING_128K;  //マッピングモード

	return TOUCHBAR_Init( &touchbar_setup, heapID );
}
//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBAR破棄  カスタムボタン版
 *
 *  @param  TOUCHBAR_WORK *touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_TOUCHBAR_ExitEx( BOX_SEARCH_MAIN_WORK *wk )
{ 
  //タッチバー破棄
  TOUCHBAR_Exit( wk->touchbar );
}

//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBARメイン処理
 *
 *  @param  TOUCHBAR_WORK *touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_TOUCHBAR_Main( TOUCHBAR_WORK  *touchbar )
{ 
  TOUCHBAR_Main( touchbar );
}
#endif //BOX_SEARCH_TOUCHBAR


#ifdef BOX_SEARCH_TASKMENU
//----------------------------------------------------------------------------
/**
 *  @brief  TASKMENUの初期化
 *
 *  @param  menu_res  リソース
 */
//-----------------------------------------------------------------------------
static APP_TASKMENU_WORK * BOXSEARCH_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heapID )
{ 
  int i;
  APP_TASKMENU_INITWORK init;
  APP_TASKMENU_ITEMWORK item[3];
  APP_TASKMENU_WORK     *menu;  

  //窓の設定
  for( i = 0; i < NELEMS(item); i++ )
  { 
    item[i].str = GFL_MSG_CreateString( msg, 0 ); //文字列
    item[i].msgColor  = APP_TASKMENU_ITEM_MSGCOLOR; //文字色
    item[i].type      = APP_TASKMENU_WIN_TYPE_NORMAL; //窓の種類
  }

  //初期化
  init.heapId   = heapID;
  init.itemNum  = NELEMS(item);
  init.itemWork = item;
  init.posType  = ATPT_RIGHT_DOWN;
  init.charPosX = 32;
  init.charPosY = 21;
  init.w        = APP_TASKMENU_PLATE_WIDTH;
  init.h        = APP_TASKMENU_PLATE_HEIGHT;

  menu  = APP_TASKMENU_OpenMenu( &init, menu_res );

  //文字列解放
  for( i = 0; i < NELEMS(item); i++ )
  { 
    GFL_STR_DeleteBuffer( item[i].str );
  }

  return menu;

}
//----------------------------------------------------------------------------
/**
 *  @brief  TASKMENUの破棄
 *
 *  @param  APP_TASKMENU_WORK *menu ワーク
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_TASKMENU_Exit( APP_TASKMENU_WORK *menu )
{ 
  APP_TASKMENU_CloseMenu( menu );
}
//----------------------------------------------------------------------------
/**
 *  @brief  TASKMENUのメイン処理
 *
 *  @param  APP_TASKMENU_WORK *menu ワーク
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_TASKMENU_Main( APP_TASKMENU_WORK *menu )
{ 
  APP_TASKMENU_UpdateMenu( menu );
}
#endif //BOX_SEARCH_TASKMENU




#ifdef BOX_SEARCH_POKE_ICON
//=============================================================================
/**
 *  POKE_ICON
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *  @brief  ポケアイコン 生成
 *
 *  @param  BOX_SEARCH_MAIN_WORK *wk ワーク
 *  @param  unit  ユニット
 *  @param  heapID  ヒープID  
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_POKE_ICON_CreateCLWK( BOX_SEARCH_MAIN_WORK* wk, u32 mons, u32 form_no, u32 sex, BOOL egg, GFL_CLUNIT* unit, HEAPID heapID )
{
  // ポケアイコン本体
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_MAIN;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NCLR;
    prm.arc_id    = ARCID_POKEICON;
    prm.pltt_id   = POKEICON_GetPalArcIndex();
    prm.ncg_id    = POKEICON_GetCgxArcIndexByMonsNumber( mons, form_no, sex, egg );
    prm.cell_id   = POKEICON_GetCellArcIndex(); 
    prm.anm_id    = POKEICON_GetAnmArcIndex();
    prm.pltt_line = PLTID_OBJ_POKEICON_M;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num = 3;
    
    UI_EASY_CLWK_LoadResource( &wk->clres_poke_icon, &prm, unit, heapID );

    // アニメシーケンスで指定( 0=瀕死, 1=HP最大, 2=HP緑, 3=HP黄, 4=HP赤, 5=状態異常 )
    wk->clwk_poke_icon = UI_EASY_CLWK_CreateCLWK( &wk->clres_poke_icon, unit, 48, 32, 1, heapID );

    // 上にアイテムアイコンを描画するので優先度を下げておく
    GFL_CLACT_WK_SetSoftPri( wk->clwk_poke_icon, 1 );

    // オートアニメ ON
    GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk_poke_icon, TRUE );

    {
      u8 pal_num = POKEICON_GetPalNum( mons, form_no, sex, egg );
      GFL_CLACT_WK_SetPlttOffs( wk->clwk_poke_icon, pal_num, CLWK_PLTTOFFS_MODE_OAM_COLOR );
    }
  }

  // ポケアイコン用アイテムアイコン
  {
    UI_EASY_CLWK_RES_PARAM prm;

    prm.draw_type = CLSYS_DRAW_MAIN;
    prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
    prm.arc_id    = APP_COMMON_GetArcId();
    prm.pltt_id   = APP_COMMON_GetPokeItemIconPltArcIdx();
    prm.ncg_id    = APP_COMMON_GetPokeItemIconCharArcIdx();
    prm.cell_id   = APP_COMMON_GetPokeItemIconCellArcIdx( APP_COMMON_MAPPING_128K );
    prm.anm_id    = APP_COMMON_GetPokeItemIconAnimeArcIdx( APP_COMMON_MAPPING_128K );
    prm.pltt_line = PLTID_OBJ_POKEITEM_M;
    prm.pltt_src_ofs  = 0;
    prm.pltt_src_num = 1;
    
    UI_EASY_CLWK_LoadResource( &wk->clres_poke_item, &prm, unit, heapID );

    // アニメシーケンスで指定( 0=どうぐ, 1=メール )
    // ※位置調整はとりあえずの値です。
    wk->clwk_poke_item = UI_EASY_CLWK_CreateCLWK( &wk->clres_poke_item, unit, 48+1*8+4, 32+4, 0, heapID );
  }

}

//-----------------------------------------------------------------------------
/**
 *  @brief  ポケアイコン 破棄
 *
 *  @param  BOX_SEARCH_MAIN_WORK *wk ワーク
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_POKE_ICON_DeleteCLWK( BOX_SEARCH_MAIN_WORK* wk )
{ 
  //CLWK破棄
  GFL_CLACT_WK_Remove( wk->clwk_poke_icon );
  //リソース開放
  UI_EASY_CLWK_UnLoadResource( &wk->clres_poke_icon );
  
  //CLWK破棄
  GFL_CLACT_WK_Remove( wk->clwk_poke_item );
  //リソース開放
  UI_EASY_CLWK_UnLoadResource( &wk->clres_poke_item );
}

#endif //BOX_SEARCH_POKE_ICON

#ifdef BOX_SEARCH_OAM_MAPMODEL
//=============================================================================
/**
 *  OAM_MAPMODEL
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *  @brief  OAMマップモデル 作成 (人物OBJ特化仕様)
 *
 *  @param  BOX_SEARCH_MAIN_WORK *wk ワーク
 *  @param  tex_idx   テクスチャのARC内インデックス
 *  @param  ptn_ofs   テクスチャオフセット（アニメパターンのオフセット）
 *  @param  GFL_CLUNIT *unit  CLUNIT  ユニット
 *  @param  heapID            ヒープID
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_OAM_MAPMODEL_CreateCLWK( BOX_SEARCH_MAIN_WORK *wk, u16 tex_idx, u8 ptn_ofs, GFL_CLUNIT *unit, HEAPID heapID )
{ 
  UI_EASY_CLWK_RES_PARAM prm;

  prm.draw_type = CLSYS_DRAW_MAIN;
  prm.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;
  prm.arc_id    = APP_COMMON_GetArcId();
  prm.pltt_id   = APP_COMMON_GetNull4x4PltArcIdx();
  prm.ncg_id    = APP_COMMON_GetNull4x4CharArcIdx();
  prm.cell_id   = APP_COMMON_GetNull4x4CellArcIdx( APP_COMMON_MAPPING_128K );
  prm.anm_id    = APP_COMMON_GetNull4x4AnimeArcIdx( APP_COMMON_MAPPING_128K );
  prm.pltt_line = PLTID_OBJ_OAM_MAPMODEL_M;
  prm.pltt_src_ofs  = 0;
  prm.pltt_src_num = 1;

  // リソース読み込み
  UI_EASY_CLWK_LoadResource( &wk->clres_oam_mmdl, &prm, unit, heapID );
  // CLWK生成
  wk->clwk_oam_mmdl = UI_EASY_CLWK_CreateCLWK( &wk->clres_oam_mmdl, unit, 80, 32, 0, heapID );

  // テクスチャを転送
  {
    // 人物は基本 4 x 4
    int sx = NSBTX_DEF_SX;
    int sy = NSBTX_DEF_SY;

    CLWK_TransNSBTX( wk->clwk_oam_mmdl, ARCID_MMDL_RES, tex_idx, ptn_ofs, sx, sy, 0, prm.draw_type, heapID );
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  OAMマップモデル 破棄
 *
 *  @param  BOX_SEARCH_MAIN_WORK *wk ワーク
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_OAM_MAPMODEL_DeleteCLWK( BOX_SEARCH_MAIN_WORK* wk )
{
  //CLWK破棄
  GFL_CLACT_WK_Remove( wk->clwk_oam_mmdl );
  //リソース開放
  UI_EASY_CLWK_UnLoadResource( &wk->clres_oam_mmdl );
}

#endif //BOX_SEARCH_OAM_MAPMODEL




#ifdef  BOX_SEARCH_PRINT_TOOL
//=============================================================================
/**
 *  PRINT_TOOL
 */
//=============================================================================
/*
    ・「HP ??/??」を表示するサンプルです
    ・BMPWINのサイズは 20 x 2 です
    ・現在のHP = 618, 最大HP = 999 とします
    ・サンプルメインが FALSE を返すと終了です
*/

// サンプルメイン
static BOOL PrintTool_MainFunc( BOX_SEARCH_MAIN_WORK * wk )
{
  switch( wk->seq ){
  case 0:
    PrintTool_AddBmpWin( wk );      // BMPWIN作成
    PrintTool_PrintHP( wk );        // ＨＰ表示
    PrintTool_ScreenTrans( wk );    // スクリーン転送
    wk->seq = 1;
    break;

  case 1:
    // プリント終了待ち
    if( PRINTSYS_QUE_IsFinished( wk->que ) == TRUE ){
      wk->seq = 2;
    }
    break;

  case 2:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
      PrintTool_DelBmpWin( wk );    // BMPWIN削除
      return FALSE;
    }
  }

  PRINT_UTIL_Trans( &wk->print_util, wk->que );
  return TRUE;
}

// BMPWIN作成
static void PrintTool_AddBmpWin( BOX_SEARCH_MAIN_WORK * wk )
{
  wk->print_util.win = GFL_BMPWIN_Create(
                          GFL_BG_FRAME0_M,          // ＢＧフレーム
                          1, 1,                     // 表示座標
                          20, 2,                    // 表示サイズ
                          15,                       // パレット
                          GFL_BMP_CHRAREA_GET_B );  // キャラ取得方向
}

// BMPWIN削除
static void PrintTool_DelBmpWin( BOX_SEARCH_MAIN_WORK * wk )
{
    GFL_BMPWIN_Delete( wk->print_util.win );
}

// BMPWINスクリーン転送
static void PrintTool_ScreenTrans( BOX_SEARCH_MAIN_WORK * wk )
{
  GFL_BMPWIN_MakeScreen( wk->print_util.win );
  GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->print_util.win) );
}

// ＨＰ表示
static void PrintTool_PrintHP( BOX_SEARCH_MAIN_WORK * wk )
{
  STRBUF * strbuf;

  // BMPWIN内の座標(32,0)を基準に右詰めで「ＨＰ」と表示
  PRINTTOOL_Print(
        &wk->print_util,        // プリントユーティル ( BMPWIN )
        wk->que,                // プリントキュー
        32, 0,                  // 表示基準座標
        strbuf,                 // 文字列（すでに「ＨＰ」が展開されているものとする）
        wk->font,               // フォント
        PRINTTOOL_MODE_RIGHT ); // 右詰

  // BMPWIN内の座標(100,0)を「／」の中心にしてＨＰを表示 ( hp / mhp )
  PRINTTOOL_PrintFraction(
        &wk->print_util,        // プリントユーティル ( BMPWIN )
        wk->que,                // プリントキュー
        wk->font,               // フォント
        100, 0,                 // 表示基準座標
        618,                    // hp
        999,                    // mhp
        wk->heapID );           // ヒープＩＤ
}
#endif  //BOX_SEARCH_PRINT_TOOL

#ifdef BOX_SEARCH_PMSDRAW

//-----------------------------------------------------------------------------
/**
 *  @brief  簡易会話表示 初期化処理
 *
 *  @param  BOX_SEARCH_MAIN_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_PMSDRAW_Init( BOX_SEARCH_MAIN_WORK* wk )
{
  GFL_CLUNIT  *clunit;
  
  clunit = BOX_SEARCH_GRAPHIC_GetClunit( wk->graphic );

  wk->pms_draw  = PMS_DRAW_Init( clunit, CLSYS_DRAW_SUB, wk->print_que, wk->font, 
      PLTID_OBJ_PMS_DRAW, BOX_SEARCH_PMSDRAW_NUM ,wk->heapID );
  
  {
    int i;
    PMS_DATA pms;

    // PMS表示用BMPWIN生成
    for( i=0; i<BOX_SEARCH_PMSDRAW_NUM; i++ )
    {
      wk->pms_win[i] = GFL_BMPWIN_Create(
          BG_FRAME_TEXT_S,          // ＢＧフレーム
          2+4*i, 0 + 6 * i,             // 表示座標(キャラ単位)
          28, 4,                    // 表示サイズ
          15,                       // パレット
          GFL_BMP_CHRAREA_GET_B );  // キャラ取得方向
    }
    
    // 1個目 通常+空欄表示
    PMSDAT_SetDebug( &pms );
    PMSDAT_SetWord( &pms, 1, PMS_WORD_NULL );
    PMS_DRAW_Print( wk->pms_draw, wk->pms_win[0], &pms ,0 );

    // 2個目 デコメ
    PMSDAT_SetDeco( &pms, 1, PMS_DECOID_HERO );
    PMS_DRAW_Print( wk->pms_draw, wk->pms_win[1], &pms ,1 );
    
    // 3個目 デコメ二個表示
    PMSDAT_SetDeco( &pms, 0, PMS_DECOID_TANKS );
    PMSDAT_SetDeco( &pms, 1, PMS_DECOID_LOVE );
    PMS_DRAW_Print( wk->pms_draw, wk->pms_win[2], &pms ,2 );
    
    // 1の要素を2にコピー(移動表現などにご使用ください。)
    PMS_DRAW_Copy( wk->pms_draw, 1, 2 );
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  簡易会話表示 後処理
 *
 *  @param  BOX_SEARCH_MAIN_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_PMSDRAW_Exit( BOX_SEARCH_MAIN_WORK* wk )
{
  PMS_DRAW_Exit( wk->pms_draw );
  {
    int i;
    for( i=0; i<BOX_SEARCH_PMSDRAW_NUM; i++ )
    {
      GFL_BMPWIN_Delete( wk->pms_win[i] );
    }
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  簡易会話表示 主処理
 *
 *  @param  BOX_SEARCH_MAIN_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void BOXSEARCH_PMSDRAW_Proc( BOX_SEARCH_MAIN_WORK* wk )
{
#if 1
  // SELECTでクリア
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    int i;
    for( i=0; i<BOX_SEARCH_PMSDRAW_NUM; i++ )
    {
      PMS_DRAW_Clear( wk->pms_draw, i, TRUE );
    }
  }
  // STARTでランダム挿入（二重登録するとアサート）
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    int i;
    for( i=0; i<BOX_SEARCH_PMSDRAW_NUM; i++ )
    {
      PMS_DATA pms;
      PMSDAT_SetDebugRandom( &pms );
      PMSDAT_SetDeco( &pms, 0, GFUser_GetPublicRand(10)+1 );
      PMS_DRAW_Print( wk->pms_draw, wk->pms_win[i], &pms ,i );
    }
  }
#endif

  PMS_DRAW_Main( wk->pms_draw );
}

#endif // BOX_SEARCH_PMSDRAW


//----------------------------------------------------------------------------------
/**
 * @brief 文字列描画用領域確保
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void   SearchBmpwinInit( BOX_SEARCH_MAIN_WORK *wk )
{
  int i;
  const BMPWIN_DAT *windat;

  // 文字列描画用のBMPWINを確保（上下画面分定義されています）
  for(i=0;i<BOX_SEARCH_BMPWIN_NUM;i++){
    windat = Window0BmpDataTable[i];
    wk->bmp_win[i] = GFL_BMPWIN_Create( windat->frame,
                                       windat->x, windat->y,
                                       windat->w, windat->h,
                                       windat->pal, GFL_BMP_CHRAREA_GET_B );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->bmp_win[i]), 0 );
    PRINT_UTIL_Setup( &wk->print_util[i], wk->bmp_win[i] );
  }

}


//----------------------------------------------------------------------------------
/**
 * @brief 文字列描画用領域解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void SearchBmpinExit( BOX_SEARCH_MAIN_WORK *wk )
{
  int i;
  // 確保したBMP領域を解放
  for(i=0;i<BOX_SEARCH_BMPWIN_NUM;i++){
    GFL_BMPWIN_Delete( wk->bmp_win[i] );
  }
}



//----------------------------------------------------------------------------------
/**
 * @brief 絞り込み画面用ＯＢＪ登録
 *
 * @param   wk    
 * @param   unit    
 * @param   heapID    
 */
//----------------------------------------------------------------------------------
static void ObjInit( BOX_SEARCH_MAIN_WORK *wk, GFL_CLUNIT *unit, HEAPID heapID )
{
  int i;
  UI_EASY_CLWK_RES_PARAM param;

  // ボタン等リソース転送
  param = objres_prm;
  UI_EASY_CLWK_LoadResource( &wk->clres[BOXSEARCH_OBJRES_NORMAL], &param, unit, heapID );

  // マーキングOBJリソース転送
  param.draw_type = CLSYS_DRAW_MAIN;         //  描画先タイプ
  param.comp_flg  = UI_EASY_CLWK_RES_COMP_NONE;// 圧縮フラグ
  param.arc_id    = APP_COMMON_GetArcId();   ///< アーカイブNO
  param.pltt_id   = APP_COMMON_GetPokeMarkPltArcIdx();  ///< パレットリソースNO
  param.ncg_id    = APP_COMMON_GetPokeMarkCharArcIdx(APP_COMMON_MAPPING_128K);   ///< キャラクタリソースNO
  param.cell_id   = APP_COMMON_GetPokeMarkCellArcIdx(APP_COMMON_MAPPING_128K);  ///< セルリソースNO
  param.anm_id    = APP_COMMON_GetPokeMarkAnimeArcIdx(APP_COMMON_MAPPING_128K);   ///< セルアニメリソースNO
  param.pltt_line = PLTID_OBJ_MARK_M;   ///< パレット転送先NO
  param.pltt_src_ofs = 0;  ///< パレットデータ読み出し開始位置（パレット何本目から転送するか？ 1本=16色）
  param.pltt_src_num = 1;  ///< パレット転送本数（何本分転送するか？ 1本=16色／0だと全て転送）
  UI_EASY_CLWK_LoadResource( &wk->clres[BOXSEARCH_OBJRES_MARK], &param, unit, heapID );

  // 矢印OBJリソース転送
	// タッチバーのリソースを参照できないので新規に読み込む
  param.pltt_id   = APP_COMMON_GetBarIconPltArcIdx();
  param.ncg_id    = APP_COMMON_GetBarIconCharArcIdx();
  param.cell_id   = APP_COMMON_GetBarIconCellArcIdx(APP_COMMON_MAPPING_128K);
  param.anm_id    = APP_COMMON_GetBarIconAnimeArcIdx(APP_COMMON_MAPPING_128K);
  param.pltt_line = PLTID_OBJ_ARROW_M;		///< パレット転送先NO
  param.pltt_src_ofs = 0;		///< パレットデータ読み出し開始位置（パレット何本目から転送するか？ 1本=16色）
  param.pltt_src_num = 3;		///< パレット転送本数（何本分転送するか？ 1本=16色／0だと全て転送）
  UI_EASY_CLWK_LoadResource( &wk->clres[BOXSEARCH_OBJRES_ARROW], &param, unit, heapID );

  // ボタンOBJとマーキングOBJ表示登録
  for(i=0;i<BOXSEARCH_OBJ_NUM;i++){
    const CLACT_INIT_DAT *dat = &obj_init_dat[i];
    wk->clwk[i] = UI_EASY_CLWK_CreateCLWK( &wk->clres[dat->objres], unit, 
                                           dat->x, dat->y, dat->anm, heapID );
    GFL_CLACT_WK_SetBgPri( wk->clwk[i], dat->priority );
    GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], dat->appear );
    GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[i], TRUE );
    GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], FALSE );
  }
  
  // マーキングのパレットを上書きする
  GFL_ARC_UTIL_TransVramPalette( ARCID_BOX2_GRA, NARC_box_gra_poke_mark_NCLR, 
                                 PALTYPE_MAIN_OBJ, 9*32, 32, heapID );
}

//----------------------------------------------------------------------------------
/**
 * @brief 絞り込み画面用ＯＢＪ解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void   ObjExit( BOX_SEARCH_MAIN_WORK *wk )
{
  int i;
  for(i=0;i<BOXSEARCH_OBJRES_NUM;i++){
    UI_EASY_CLWK_UnLoadResource( &wk->clres[i] );
  }
}



//----------------------------------------------------------------------------------
/**
 * @brief サブ画面のテキスト情報を描画する
 *
 * @param   wk    
 * @param   subseq    
 */
//----------------------------------------------------------------------------------
static void SubTextPrint( BOX_SEARCH_MAIN_WORK *wk, int subseq )
{

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->bmp_win[BOXSEARCH_BMPID_TEXT] ), 0 );

  switch(subseq){
  case SUBSEQ_START:
    PrintMessage( wk, BOXSEARCH_BMPID_TEXT, mes_box_search_text_02, 0 );
    break;
  case SUBSEQ_SELECTPOKEMONINIT:
    PrintMessage( wk, BOXSEARCH_BMPID_TEXT, mes_box_search_text_03, 0 );
    break;
  case SUBSEQ_SELECTPOKEMON2INIT:
    PrintMessage( wk, BOXSEARCH_BMPID_TEXT, mes_box_search_text_03, 0 );
    break;
  case SUBSEQ_SELECTPOKEMON3INIT:
//    PrintMessage( wk, BOXSEARCH_BMPID_TEXT, mes_box_search_text_03_01, 0 );
    break;
  case SUBSEQ_SELECTTOKUSEIINIT:
    PrintMessage( wk, BOXSEARCH_BMPID_TEXT, mes_box_search_text_05, 0 );
    break;
  case SUBSEQ_SELECTTOKUSEI2INIT:
    PrintMessage( wk, BOXSEARCH_BMPID_TEXT, mes_box_search_text_05, 0 );
    break;
  case SUBSEQ_SELECTTOKUSEI3INIT:
		PrintTokuseiInfo( wk, FRAMELIST_GetItemParam(wk->frame_list_work,0) );
    break;
  case SUBSEQ_SELECTSEXINIT:
    PrintMessage( wk, BOXSEARCH_BMPID_TEXT, mes_box_search_text_06, 0 );
    break;
  case SUBSEQ_SELECTITEMHAVEINIT:
    PrintMessage( wk, BOXSEARCH_BMPID_TEXT, mes_box_search_text_07, 0 );
    break;
  case SUBSEQ_SELECTMARKINIT:
    PrintMessage( wk, BOXSEARCH_BMPID_TEXT, mes_box_search_text_08, 0 );
    break;
  }

	BmpWinTransScreen( wk->bmp_win[BOXSEARCH_BMPID_TEXT] );
}

// BGに転送
static void BmpWinTransScreen( GFL_BMPWIN * bmpwin )
{
	GFL_BMPWIN_MakeScreen( bmpwin );
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(bmpwin) );
}




#define TOPIC_NAME_OFFSET_Y ( 4 )

// タイトルなどを描画
static void PrintDefautltSubDisp( BOX_SEARCH_MAIN_WORK * wk )
{
	u32	i;

	// タイトル描画
	PrintStrbuf( wk, BOXSEARCH_BMPID_TITLE, BOXSEARCH_STRID_TITLE, 0 );
//	GFL_BMPWIN_MakeTransWindow_VBlank( wk->bmp_win[BOXSEARCH_BMPID_TITLE] );
	BmpWinTransScreen( wk->bmp_win[BOXSEARCH_BMPID_TITLE] );

	//「ポケモン　しぼりこみ」
	PrintStrbuf( wk, BOXSEARCH_BMPID_CATEGORY, BOXSEARCH_STRID_SIBORIKOMI, 0 );
//	GFL_BMPWIN_MakeTransWindow_VBlank( wk->bmp_win[BOXSEARCH_BMPID_CATEGORY] );
	BmpWinTransScreen( wk->bmp_win[BOXSEARCH_BMPID_CATEGORY] );

	// 項目描画　※書き換えてないみたいなので、ここで１回だけ
	for( i=0; i<6; i++ ){
		PrintStrbuf( wk, BOXSEARCH_BMPID_TOPIC0+i, i, TOPIC_NAME_OFFSET_Y );
	}
}

// 現在選択中の項目を表示
static void PrintNowCategory( BOX_SEARCH_MAIN_WORK * wk, u32 id )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->bmp_win[BOXSEARCH_BMPID_NOW_CATEGORY] ), 0 );
	PrintStrbuf( wk, BOXSEARCH_BMPID_NOW_CATEGORY, now_category_tbl[id], 0 );
	BmpWinTransScreen( wk->bmp_win[BOXSEARCH_BMPID_NOW_CATEGORY] );
}

// 現在選択中のデータを表示
static void PrintNowSelect( BOX_SEARCH_MAIN_WORK * wk, GFL_MSGDATA * msg, u32 id )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->bmp_win[BOXSEARCH_BMPID_OLD_SELECT] ), 0 );
	PrintStrBuf( wk, &wk->print_util[BOXSEARCH_BMPID_OLD_SELECT], wk->print_que, msg, id, PRINTSYS_MACRO_LSB(15,2,0), 0 );
	BmpWinTransScreen( wk->bmp_win[BOXSEARCH_BMPID_OLD_SELECT] );
}



//----------------------------------------------------------------------------------
/**
 * @brief 初期状態の画面描画を行う
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void SetupMainMode( BOX_SEARCH_MAIN_WORK *wk )
{
	u32	i;

/*
  // 左選択情報BMPWINの消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->bmp_win[BOXSEARCH_BMPID_NOW_CATEGORY] ), 0 );
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->bmp_win[BOXSEARCH_BMPID_NOW_CATEGORY] );
*/

  // 「じょうほうを　えらび　…」
	SubTextPrint( wk, SUBSEQ_START );
  
  // BG・BMPWIN転送
//  SetupScreen( wk, 0, 0 );

	TransMainBGScreen( wk, 0 );

	// 項目表示
	for( i=0; i<6; i++ ){
		BmpWinTransScreen( wk->bmp_win[BOXSEARCH_BMPID_TOPIC0+i] );
//		GFL_BMPWIN_MakeTransWindow_VBlank( wk->bmp_win[BOXSEARCH_BMPID_TOPIC0+i] );
	}
	DecideInfoPrint( wk, TRUE );

	put_search_checkbox(wk);												// サーチON/OFFのチェックボックス反映
	SetMarkDataObj( wk, BOXSEARCH_OBJID_MARK0_0 );	// マーク反映
	PutMarkDataObj( wk, 0 );
	SetDrawEnableMainItemObj( wk, TRUE );


//    ObjOnOff( wk, 0, 0 );
    // マーキング反映
    // 各種BG設定・転送
//    GFL_BG_ClearScreen( BG_FRAME_BUTTON_M );
    // 入力済情報を描画

}

// 初期画面表示をクリアする
static void ClearMainDisp( BOX_SEARCH_MAIN_WORK * wk )
{
	int	i;

	// OBJ非表示
	SetDrawEnableMainItemObj( wk, FALSE );
	for( i=BOXSEARCH_OBJID_RESET; i<=BOXSEARCH_OBJID_BUTTON_OFF_CHECKBOX; i++ ){
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], FALSE );
	}

	// 項目名非表示
	for( i=BOXSEARCH_BMPID_TOPIC0; i<=BOXSEARCH_BMPID_TOPIC5; i++ ){
		GFL_BMPWIN_ClearTransWindow_VBlank( wk->bmp_win[i] );
	}
	// 入力済み条件非表示
	DecideInfoPrint( wk, FALSE );
}

static void SetDrawEnableMainItemObj( BOX_SEARCH_MAIN_WORK * wk, BOOL flg )
{
	u32	i;

	// OBJ非表示
	for( i=BOXSEARCH_OBJID_PLATE0; i<=BOXSEARCH_OBJID_BUTTON_OFF_CHECKBOX; i++ ){
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], flg );
	}
	SetMarkDataObjDrawEnable( wk, BOXSEARCH_OBJID_MARK0_0, flg );
}


//      0
//      1
//      2
//      3
//      4
//      5
// 6   7  8

#define PLATE_TOUCH_X   ( 15*8 )
#define PLATE_TOUCH_W   ( 17*8 )
#define PLATE0_TOUCH_Y  ( 0*24 )
#define PLATE1_TOUCH_Y  ( 1*24 )
#define PLATE2_TOUCH_Y  ( 2*24 )
#define PLATE3_TOUCH_Y  ( 3*24 )
#define PLATE4_TOUCH_Y  ( 4*24 )
#define PLATE5_TOUCH_Y  ( 5*24 )
#define PLATE6_TOUCH_Y  ( 6*24 )
#define PLATE_TOUCH_H   ( 24 )
#define PLATE_SEARCHON_X ( 11*8+4 )
#define PLATE_SEARCHON_W ( 9*8 )
#define PLATE_SEARCHON_Y ( 142 )
#define PLATE_SEARCHON_H (  3*8 )
#define PLATE_SEARCHOFF_X ( 21*8+4 )
#define PLATE_SEARCHOFF_W ( PLATE_SEARCHON_W )
#define PLATE_RESET_X ( 2*8+2 )
#define PLATE_RESET_W ( 8*8-4 )

static const CURSORMOVE_DATA cm_boxsearch_main[]={
  { 0,0,0,0, 7,1,0,0, { PLATE0_TOUCH_Y, PLATE0_TOUCH_Y+PLATE_TOUCH_H-1, PLATE_TOUCH_X, PLATE_TOUCH_X+PLATE_TOUCH_W-1 }, },
  { 0,0,0,0, 0,2,1,1, { PLATE1_TOUCH_Y, PLATE1_TOUCH_Y+PLATE_TOUCH_H-1, PLATE_TOUCH_X, PLATE_TOUCH_X+PLATE_TOUCH_W-1 }, },
  { 0,0,0,0, 1,3,2,2, { PLATE2_TOUCH_Y, PLATE2_TOUCH_Y+PLATE_TOUCH_H-1, PLATE_TOUCH_X, PLATE_TOUCH_X+PLATE_TOUCH_W-1 }, },
  { 0,0,0,0, 2,4,3,3, { PLATE3_TOUCH_Y, PLATE3_TOUCH_Y+PLATE_TOUCH_H-1, PLATE_TOUCH_X, PLATE_TOUCH_X+PLATE_TOUCH_W-1 }, },
  { 0,0,0,0, 3,5,4,4, { PLATE4_TOUCH_Y, PLATE4_TOUCH_Y+PLATE_TOUCH_H-1, PLATE_TOUCH_X, PLATE_TOUCH_X+PLATE_TOUCH_W-1 }, },
  { 0,0,0,0, 4,7,5,5, { PLATE5_TOUCH_Y, PLATE5_TOUCH_Y+PLATE_TOUCH_H-1, PLATE_TOUCH_X, PLATE_TOUCH_X+PLATE_TOUCH_W-1 }, },
  { 0,0,0,0, 5,0,6,7, { PLATE_SEARCHON_Y, PLATE_SEARCHON_Y+PLATE_SEARCHON_H-1, PLATE_RESET_X, PLATE_RESET_X+PLATE_RESET_W-1 }, },
  { 0,0,0,0, 5,0,6,8, { PLATE_SEARCHON_Y, PLATE_SEARCHON_Y+PLATE_SEARCHON_H-1, PLATE_SEARCHON_X, PLATE_SEARCHON_X+PLATE_SEARCHON_W-1 }, },
  { 0,0,0,0, 5,0,7,8, { PLATE_SEARCHON_Y, PLATE_SEARCHON_Y+PLATE_SEARCHON_H-1, PLATE_SEARCHOFF_X, PLATE_SEARCHOFF_X+PLATE_SEARCHOFF_W-1 }, },
  { 0, 0, 0, 0, 0, 0, 0, 0, { GFL_UI_TP_HIT_END, 0, 0, 0 } }
};

static const CURSORMOVE_CALLBACK cm_callback[]={
  cursor_on_callback,
  cursor_off_callback,
  cursor_move_callback,
  touch_callback,
};



//----------------------------------------------------------------------------------
/**
 * @brief [SEQ]絞り込みメイン
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int Subseq_Start(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	if( GFL_UI_CheckTouchOrKey() == 0 ){
		wk->cmwk = CURSORMOVE_Create(
								cm_boxsearch_main,
								cm_callback,
								wk,
								TRUE,
								wk->main_func_pos,
								wk->heapID );
		cursor_on_callback( wk, wk->main_func_pos, wk->main_func_pos );
	}else{
		wk->cmwk = CURSORMOVE_Create(
								cm_boxsearch_main,
								cm_callback,
								wk,
								FALSE,
								wk->main_func_pos,
								wk->heapID );
	}
	CURSORMOVE_VanishModeSet( wk->cmwk );

	FrameListClear( wk );	// リストクリア
  SetupMainMode(wk);

  return SUBSEQ_MAIN;
}

static int func_table[]={
/*
	SUBSEQ_SELECTPOKEMONINIT,		// 名前
*/
	SUBSEQ_START,								// 名前
	SUBSEQ_SELECTSEIKAKUINIT,		// 性格
	SUBSEQ_SELECTTOKUSEIINIT,		// 特性
	SUBSEQ_SELECTSEXINIT,				// 性別
	SUBSEQ_SELECTITEMHAVEINIT,	// 持ち物
	SUBSEQ_SELECTMARKINIT,			// マーク
};

//----------------------------------------------------------------------------------
/**
 * @brief ボタン毎の機能を呼び出す
 *
 * @param   wk    
 * @param   seq   
 * @param   pos   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _set_mainmode_function( BOX_SEARCH_MAIN_WORK *wk, int seq, int pos )
{
  switch(pos){
  case BOXSEARCH_PLATE_ON:
		seq = SetButtonAnime( wk, BOXSEARCH_OBJID_BUTTON_ON, BOXSEARCH_ANM_SEARCHON_DECIDE, SUBSEQ_SEARCHON );
    break;

  case BOXSEARCH_PLATE_OFF:
		seq = SetButtonAnime( wk, BOXSEARCH_OBJID_BUTTON_OFF, BOXSEARCH_ANM_SEARCHOFF_DECIDE, SUBSEQ_SEARCHOFF );
    break;

  case BOXSEARCH_PLATE_0: case BOXSEARCH_PLATE_1: case BOXSEARCH_PLATE_2:
  case BOXSEARCH_PLATE_3: case BOXSEARCH_PLATE_4: case BOXSEARCH_PLATE_5:
    CURSORMOVE_Exit( wk->cmwk );
		seq = SetButtonAnime( wk, BOXSEARCH_OBJID_PLATE0+pos-BOXSEARCH_PLATE_0, BOXSEARCH_ANM_FRAME_DECIDE, SUBSEQ_SELECT_SET );
    break;

  case BOXSEARCH_PLATE_RESET: // 条件リセット
		seq = SetButtonAnime( wk, BOXSEARCH_OBJID_RESET, BOXSEARCH_ANM_RESET_DECIDE, SUBSEQ_SEARCHRESET );
		break;

	case BOXSEARCH_PLATE_RETURN:
		CURSORMOVE_Exit( wk->cmwk );
		seq = SetButtonAnime( wk, BUTTON_ID_TB_RETURN, 0, SUBSEQ_SEARCHEXIT );
		break;
  }

  wk->main_func_pos = pos;

  return seq;
}

//----------------------------------------------------------------------------------
/**
 * @brief トップ画面メイン
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int Subseq_Main(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	u32	result;

	if( CheckCancelButton( wk ) == TRUE ){
		CURSORMOVE_Exit( wk->cmwk );
		return SetButtonAnime( wk, BUTTON_ID_TB_RETURN, 0, SUBSEQ_SEARCHEXIT );
	}

	// リセット
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
		CURSORMOVE_PosSet( wk->cmwk, BOXSEARCH_PLATE_RESET );
		CURSORMOVE_CursorOnOffSet( wk->cmwk, TRUE );
		return SetButtonAnime( wk, BOXSEARCH_OBJID_RESET, BOXSEARCH_ANM_RESET_DECIDE, SUBSEQ_SEARCHRESET );
	}
	// ONにして終了
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
		CURSORMOVE_PosSet( wk->cmwk, BOXSEARCH_PLATE_ON );
		CURSORMOVE_CursorOnOffSet( wk->cmwk, TRUE );
		CURSORMOVE_Exit( wk->cmwk );
		return SetButtonAnime( wk, BOXSEARCH_OBJID_BUTTON_ON, BOXSEARCH_ANM_SEARCHON_DECIDE, SUBSEQ_SEARCHONEND );
	}

	result = CURSORMOVE_Main( wk->cmwk );

	switch( result ){
	case CURSORMOVE_NO_MOVE_UP:			// 十字キー上が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_DOWN:		// 十字キー下が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_LEFT:		// 十字キー左が押されたが、移動なし
	case CURSORMOVE_NO_MOVE_RIGHT:	// 十字キー右が押されたが、移動なし
	case CURSORMOVE_CURSOR_ON:			// カーソル表示
	case CURSORMOVE_CURSOR_MOVE:		// 移動
	case CURSORMOVE_CANCEL:					// キャンセル
	case CURSORMOVE_NONE:						// 動作なし
		break;

	default:
		seq = _set_mainmode_function( wk, seq, result );
	}

	return seq;
}


static int Subseq_Return(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
//----------------------------------------------------------------------------------
/**
 * @brief ポケモン母音選択初期化
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int Subseq_SelectPokemonInit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  OS_Printf("touch or key=%d\n", GFL_UI_CheckTouchOrKey());

  FrameListInit(wk, BOXSEARCH_LISTID_NAME_TOPWORD);
  SetupScreen( wk, 1, BOXSEARCH_LISTID_NAME_TOPWORD );

  // FRAMELIST初期化待ちの後ポケモン母音選択へ
  _set_subseq_nextseq( wk,  SUBSEQ_SELECTPOKEMON1 );
  SubTextPrint( wk, SUBSEQ_SELECTPOKEMONINIT );
  seq = SUBSEQ_FRAMELISTINITWAIT;
  return seq;
}

//----------------------------------------------------------------------------------
/**
 * @brief ポケモン母音選択メイン
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int Subseq_SelectPokemon1(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  
  u32 result =  FRAMELIST_Main( wk->frame_list_work );
  if(TOUCHBAR_GetTrg(wk->touchbar)==TOUCHBAR_ICON_RETURN){
    wk->topword = -1;
    seq = SUBSEQ_SELECTPOKEMON1EXIT;
  }else if(TOUCHBAR_IsDecide(wk->touchbar)==FALSE){
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_DECIDE){
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      wk->topword = FRAMELIST_GetListPos( wk->frame_list_work );
      if(wk->topword==0){
        _set_search_data( wk, SEARCH_DATAID_POKEMON, 0);
      }
      seq = SUBSEQ_SELECTPOKEMON1EXIT;
    }
  }
  return seq;
}

//----------------------------------------------------------------------------------
/**
 * @brief ポケモン母音選択終了
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int Subseq_SelectPokemon1Exit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  FRAMELIST_Exit( wk->frame_list_work );
  if(wk->topword==-1 || wk->topword==0){
    TransMainBGScreen( wk, 0 );
    seq = SUBSEQ_START;
  }else{
    seq = SUBSEQ_SELECTPOKEMON2INIT;
  }
  return seq;
}


//----------------------------------------------------------------------------------
/**
 * @brief ポケモン子音選択初期化
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int Subseq_SelectPokemon2Init(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  FrameListInit(wk, BOXSEARCH_LISTID_NAME_NEXTWORD);
  SetupScreen( wk, 2, BOXSEARCH_LISTID_NAME_NEXTWORD );
  SubTextPrint( wk, SUBSEQ_SELECTPOKEMON2INIT );

  // FRAMELIST初期化待ちの後ポケモン子音選択へ
  _set_subseq_nextseq( wk,  SUBSEQ_SELECTPOKEMON2 );
  seq = SUBSEQ_FRAMELISTINITWAIT;
  
  return seq;
}

//----------------------------------------------------------------------------------
/**
 * @brief ポケモン子音選択メイン
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int Subseq_SelectPokemon2(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  u32 result =  FRAMELIST_Main( wk->frame_list_work );

  if(TOUCHBAR_GetTrg(wk->touchbar)==TOUCHBAR_ICON_RETURN){
    wk->nextword = -1;
    seq = SUBSEQ_SELECTPOKEMON2EXIT;
  }else if(TOUCHBAR_IsDecide(wk->touchbar)==FALSE){
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_DECIDE){
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      wk->nextword = FRAMELIST_GetListPos( wk->frame_list_work );
      seq = SUBSEQ_SELECTPOKEMON2EXIT;
    }
  }
  return seq;
}

//----------------------------------------------------------------------------------
/**
 * @brief ポケモン子音選択終了
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int Subseq_SelectPokemon2Exit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  FRAMELIST_Exit( wk->frame_list_work );
  if(wk->nextword==-1){
    TransMainBGScreen( wk, 0 );
    seq = SUBSEQ_START;
  }else{
    seq = SUBSEQ_SELECTPOKEMON3INIT;
  }
  return seq;
}

//----------------------------------------------------------------------------------
/**
 * @brief ポケモン名選択初期化
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int Subseq_SelectPokemon3Init(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  FrameListInit(wk, BOXSEARCH_LISTID_POKEMON);
  SetupScreen( wk, 1, BOXSEARCH_LISTID_POKEMON );
  SubTextPrint( wk, SUBSEQ_SELECTPOKEMON3INIT );

  // FRAMELIST初期化待ちの後ポケモン名選択へ
  _set_subseq_nextseq( wk,  SUBSEQ_SELECTPOKEMON3 );
  seq = SUBSEQ_FRAMELISTINITWAIT;
  return seq;
}

//----------------------------------------------------------------------------------
/**
 * @brief ポケモン名選択メイン
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int Subseq_SelectPokemon3(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  u32 result =  FRAMELIST_Main( wk->frame_list_work );
  if(TOUCHBAR_GetTrg(wk->touchbar)==TOUCHBAR_ICON_RETURN){
    //_set_search_data( wk, SEARCH_DATAID_POKEMON, 0);
    seq = SUBSEQ_SELECTPOKEMON3EXIT;
  }else if(TOUCHBAR_IsDecide(wk->touchbar)==FALSE){
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_DECIDE){
      int pos = FRAMELIST_GetListPos( wk->frame_list_work );
      _set_search_data( wk, SEARCH_DATAID_POKEMON, FRAMELIST_GetItemParam( wk->frame_list_work, pos ));
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      seq = SUBSEQ_SELECTPOKEMON3EXIT;
    }
  }
  return seq;
}

//----------------------------------------------------------------------------------
/**
 * @brief ポケモン名選択終了
 *
 * @param   wk    
 * @param   seq   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int Subseq_SelectPokemon3Exit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  FRAMELIST_Exit( wk->frame_list_work );
  TransMainBGScreen( wk, 0 );
  seq = SUBSEQ_START;
  return seq;
}

static int Subseq_SelectSeikakuInit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	ClearMainDisp( wk );

	FrameListInit( wk, BOXSEARCH_LISTID_SEIKAKU );
	SetListPageArrowAnime( wk, FALSE );

	SubTextPrint( wk, SUBSEQ_SELECTSEIKAKUINIT );

	TransMainBGScreen( wk, 1 );

	PrintNowCategory( wk, BOXSEARCH_LISTID_SEIKAKU );
	if( wk->param->syswk->search_data.seikaku == 0 ){
		PrintNowSelect( wk, wk->msg, mes_box_search_hyphen_01 );
	}else{
		PrintNowSelect( wk, wk->seikaku_msg, wk->param->syswk->search_data.seikaku-1 );
	}

	// スクロールバー
	InitScrollBarObj( wk );

  _set_subseq_nextseq( wk, SUBSEQ_SELECTSEIKAKU );

	return SUBSEQ_FRAMELISTINITWAIT;
}
static int Subseq_SelectSeikaku(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	u32	result = FRAMELIST_Main( wk->frame_list_work );

	switch( result ){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		_set_search_data( wk, SEARCH_DATAID_SEIKAKU, FRAMELIST_GetListPos(wk->frame_list_work) );
		PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0, result );
		if( wk->param->syswk->search_data.seikaku == 0 ){
			PrintNowSelect( wk, wk->msg, mes_box_search_hyphen_01 );
		}else{
			PrintNowSelect( wk, wk->seikaku_msg, wk->param->syswk->search_data.seikaku-1 );
		}
		break;


	case FRAMELIST_RET_MOVE:				// カーソル移動
	case FRAMELIST_RET_SCROLL:			// スクロール通常
	case FRAMELIST_RET_RAIL:				// レールスクロール
	case FRAMELIST_RET_SLIDE:				// スライドスクロール
		SetListPageArrowAnime( wk, FALSE );
		break;

	case FRAMELIST_RET_PAGE_UP:			// １ページ上へ
		return SetFrameListScroll( wk, BOXSEARCH_OBJID_ARROW_L, APP_COMMON_BARICON_CURSOR_LEFT_ON, SUBSEQ_SELECTSEIKAKU );

	case FRAMELIST_RET_PAGE_DOWN:		// １ページ下へ
		return SetFrameListScroll( wk, BOXSEARCH_OBJID_ARROW_R, APP_COMMON_BARICON_CURSOR_RIGHT_ON, SUBSEQ_SELECTSEIKAKU );

	case FRAMELIST_RET_NONE:
		if( CheckCancelButton( wk ) == TRUE ){
			seq = SetButtonAnime( wk, BUTTON_ID_TB_RETURN, 0, SUBSEQ_SELECTSEIKAKUEXIT );
		}else{
			int	ret = HitCheckListBox( wk );
			if( ret >= 0 && ret <= 6 ){
				_set_search_data( wk, SEARCH_DATAID_SEIKAKU, ret+FRAMELIST_GetScrollCount(wk->frame_list_work) );
				PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0, ret );
				if( wk->param->syswk->search_data.seikaku == 0 ){
					PrintNowSelect( wk, wk->msg, mes_box_search_hyphen_01 );
				}else{
					PrintNowSelect( wk, wk->seikaku_msg, wk->param->syswk->search_data.seikaku-1 );
				}
				FRAMELIST_SetCursorPos( wk->frame_list_work, ret );
			}
		}
		break;
	}

	PutScrollBarObj( wk );

	return seq;
}
static int Subseq_SelectSeikakuExit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	FRAMELIST_Exit( wk->frame_list_work );
	return SUBSEQ_START;
}

static int Subseq_SelectTokuseiInit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	ClearMainDisp( wk );

	FrameListInit( wk, BOXSEARCH_LISTID_TOKUSEI_TOPWORD );
	SetListPageArrowAnime( wk, FALSE );

	SubTextPrint( wk, SUBSEQ_SELECTTOKUSEIINIT );

	TransMainBGScreen( wk, 1 );

	PrintNowCategory( wk, BOXSEARCH_LISTID_TOKUSEI_TOPWORD );
	if( wk->param->syswk->search_data.tokusei == 0 ){
		PrintNowSelect( wk, wk->msg, mes_box_search_hyphen_01 );
	}else{
		PrintNowSelect( wk, wk->tokusei_msg, wk->param->syswk->search_data.tokusei );
	}

	// スクロールバー
	InitScrollBarObj( wk );

  _set_subseq_nextseq( wk, SUBSEQ_SELECTTOKUSEI1 );

	return SUBSEQ_FRAMELISTINITWAIT;
}
static int Subseq_SelectTokusei1(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	u32	result = FRAMELIST_Main( wk->frame_list_work );

	switch( result ){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		{
			u32	pos = FRAMELIST_GetListPos( wk->frame_list_work );
			if( pos == 0 ){
				_set_search_data( wk, SEARCH_DATAID_TOKUSEI, 0 );
				PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0, 0 );
				PrintNowSelect( wk, wk->msg, mes_box_search_hyphen_01 );
			}else{
				wk->topword = pos;
				GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_CHECK0], FALSE );
				return SUBSEQ_SELECTTOKUSEI1EXIT;
			}
		}
		break;

	case FRAMELIST_RET_MOVE:				// カーソル移動
	case FRAMELIST_RET_SCROLL:			// スクロール通常
	case FRAMELIST_RET_RAIL:				// レールスクロール
	case FRAMELIST_RET_SLIDE:				// スライドスクロール
		SetListPageArrowAnime( wk, FALSE );
		break;

	case FRAMELIST_RET_PAGE_UP:			// １ページ上へ
		return SetFrameListScroll( wk, BOXSEARCH_OBJID_ARROW_L, APP_COMMON_BARICON_CURSOR_LEFT_ON, SUBSEQ_SELECTTOKUSEI1 );

	case FRAMELIST_RET_PAGE_DOWN:		// １ページ下へ
		return SetFrameListScroll( wk, BOXSEARCH_OBJID_ARROW_R, APP_COMMON_BARICON_CURSOR_RIGHT_ON, SUBSEQ_SELECTTOKUSEI1 );

	case FRAMELIST_RET_NONE:
		if( CheckCancelButton( wk ) == TRUE ){
			wk->topword = 0;
			seq = SetButtonAnime( wk, BUTTON_ID_TB_RETURN, 0, SUBSEQ_SELECTTOKUSEI1EXIT );
		}else{
			int	ret = HitCheckListBox( wk );
			if( ret >= 0 && ret <= 6 ){
				u32	pos = FRAMELIST_GetScrollCount( wk->frame_list_work ) + ret;
				if( pos == 0 ){
					_set_search_data( wk, SEARCH_DATAID_TOKUSEI, 0 );
					PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0, ret );
					PrintNowSelect( wk, wk->msg, mes_box_search_hyphen_01 );
					FRAMELIST_SetCursorPos( wk->frame_list_work, ret );
				}else{
					wk->topword = pos;
					GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_CHECK0], FALSE );
					FRAMELIST_SetCursorPos( wk->frame_list_work, ret );
					return SUBSEQ_SELECTTOKUSEI1EXIT;
				}
			}
		}
		break;
	}

	PutScrollBarObj( wk );

	return seq;
}
static int Subseq_SelectTokusei1Exit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	FRAMELIST_Exit( wk->frame_list_work );
	if( wk->topword != 0 ){
		return SUBSEQ_SELECTTOKUSEI2INIT;
	}
	return SUBSEQ_START;
}

static int Subseq_SelectTokusei2Init(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	FrameListChangeClear( wk );

	FrameListInit( wk, BOXSEARCH_LISTID_TOKUSEI_NEXTWORD );

	SubTextPrint( wk, SUBSEQ_SELECTTOKUSEI2INIT );

	TransMainBGScreen( wk, 2 );

  _set_subseq_nextseq( wk, SUBSEQ_SELECTTOKUSEI2 );

	return SUBSEQ_FRAMELISTINITWAIT;
}
static int Subseq_SelectTokusei2(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	u32	result = FRAMELIST_Main( wk->frame_list_work );

	switch( result ){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		if( get_sort_tokusei_num(wk->topword-1,result) != 0 ){
			wk->nextword = result;
			return SUBSEQ_SELECTTOKUSEI2EXIT;
		}
		break;

	case FRAMELIST_RET_NONE:
		if( CheckCancelButton( wk ) == TRUE ){
			wk->nextword = -1;
			seq = SetButtonAnime( wk, BUTTON_ID_TB_RETURN, 0, SUBSEQ_SELECTTOKUSEI2EXIT );
		}else{
			int	ret = HitCheckListBox( wk );
			if( ret >= 0 && ret <= 4 ){
				if( get_sort_tokusei_num(wk->topword-1,ret) != 0 ){
					wk->nextword = ret;
					FRAMELIST_SetCursorPos( wk->frame_list_work, ret );
					return SUBSEQ_SELECTTOKUSEI2EXIT;
				}else{
					FRAMELIST_SetCursorPos( wk->frame_list_work, ret );
				}
			}
		}
		break;
	}

	return seq;
}
static int Subseq_SelectTokusei2Exit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	FRAMELIST_Exit( wk->frame_list_work );
	if( wk->nextword != -1 ){
		return SUBSEQ_SELECTTOKUSEI3INIT;
	}
  return SUBSEQ_START;
}

static int Subseq_SelectTokusei3Init(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	u16	i, max;

	FrameListChangeClear( wk );

	FrameListInit( wk, BOXSEARCH_LISTID_TOKUSEI );

	SubTextPrint( wk, SUBSEQ_SELECTTOKUSEI3INIT );

	max = get_sort_tokusei_num( wk->topword-1, wk->nextword );
	if( max <= 7 ){
		TransMainBGScreen( wk, 2 );
	}else{
		TransMainBGScreen( wk, 1 );
		SetListPageArrowAnime( wk, FALSE );
		InitScrollBarObj( wk );
		max = 7;
	}

	// チェックボックス表示
	for( i=0; i<max; i++ ){
		if( wk->param->syswk->search_data.tokusei == FRAMELIST_GetItemParam(wk->frame_list_work,i) ){
			PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0, i );
			break;
		}
	}

  _set_subseq_nextseq( wk, SUBSEQ_SELECTTOKUSEI3 );

	return SUBSEQ_FRAMELISTINITWAIT;
}
static int Subseq_SelectTokusei3(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	u32	result = FRAMELIST_Main( wk->frame_list_work );

	switch( result ){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		_set_search_data(
			wk, SEARCH_DATAID_TOKUSEI,
			FRAMELIST_GetItemParam( wk->frame_list_work, FRAMELIST_GetListPos(wk->frame_list_work) ) );
		PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0, result );
		PrintNowSelect( wk, wk->tokusei_msg, wk->param->syswk->search_data.tokusei );
		break;

	case FRAMELIST_RET_MOVE:				// カーソル移動
	case FRAMELIST_RET_SCROLL:			// スクロール通常
	case FRAMELIST_RET_RAIL:				// レールスクロール
	case FRAMELIST_RET_SLIDE:				// スライドスクロール
		if( get_sort_tokusei_num(wk->topword-1,wk->nextword) > 7 ){
			SetListPageArrowAnime( wk, FALSE );
		}
		break;

	case FRAMELIST_RET_PAGE_UP:			// １ページ上へ
		if( get_sort_tokusei_num(wk->topword-1,wk->nextword) > 7 ){
			return SetFrameListScroll( wk, BOXSEARCH_OBJID_ARROW_L, APP_COMMON_BARICON_CURSOR_LEFT_ON, SUBSEQ_SELECTTOKUSEI3 );
		}
		break;

	case FRAMELIST_RET_PAGE_DOWN:		// １ページ下へ
		if( get_sort_tokusei_num(wk->topword-1,wk->nextword) > 7 ){
			return SetFrameListScroll( wk, BOXSEARCH_OBJID_ARROW_R, APP_COMMON_BARICON_CURSOR_RIGHT_ON, SUBSEQ_SELECTTOKUSEI3 );
		}
		break;

	case FRAMELIST_RET_NONE:
		if( CheckCancelButton( wk ) == TRUE ){
			seq = SetButtonAnime( wk, BUTTON_ID_TB_RETURN, 0, SUBSEQ_SELECTTOKUSEI3EXIT );
		}else{
			int	ret = HitCheckListBox( wk );
			if( ret >= 0 && ret <= 6 ){
				_set_search_data(
					wk, SEARCH_DATAID_TOKUSEI,
					FRAMELIST_GetItemParam( wk->frame_list_work, ret+FRAMELIST_GetScrollCount(wk->frame_list_work) ) );
				PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0, ret );
				PrintNowSelect( wk, wk->tokusei_msg, wk->param->syswk->search_data.tokusei );
				FRAMELIST_SetCursorPos( wk->frame_list_work, ret );
			}
		}
		break;
	}

	if( get_sort_tokusei_num(wk->topword-1,wk->nextword) > 7 ){
		PutScrollBarObj( wk );
	}

	return seq;
}
static int Subseq_SelectTokusei3Exit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	FRAMELIST_Exit( wk->frame_list_work );
	return SUBSEQ_START;
}

static int Subseq_SelectSexInit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	ClearMainDisp( wk );

	FrameListInit( wk, BOXSEARCH_LISTID_SEIBETU );
	
	SubTextPrint( wk, SUBSEQ_SELECTSEXINIT );

	TransMainBGScreen( wk, 2 );

	PrintNowCategory( wk, BOXSEARCH_LISTID_SEIBETU );
	PrintNowSelect( wk, wk->msg, sex_msg_tbl[wk->param->syswk->search_data.sex] );

	// チェックボックス表示
	PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0, wk->param->syswk->search_data.sex );

	// FRAMELIST初期化待ちの後ポケモン名選択へ
	_set_subseq_nextseq( wk,  SUBSEQ_SELECTSEX );

	return SUBSEQ_FRAMELISTINITWAIT;
}
static int Subseq_SelectSex(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	u32	result = FRAMELIST_Main( wk->frame_list_work );

	switch( result ){
	case 0:
	case 1:
	case 2:
	case 3:
		_set_search_data( wk, SEARCH_DATAID_SEX, result );
		PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0, result );
		PrintNowSelect( wk, wk->msg, sex_msg_tbl[wk->param->syswk->search_data.sex] );
		break;

	case FRAMELIST_RET_NONE:
		if( CheckCancelButton( wk ) == TRUE ){
			seq = SetButtonAnime( wk, BUTTON_ID_TB_RETURN, 0, SUBSEQ_SELECTSEXEXIT );
		}else{
			int	ret = HitCheckListBox( wk );
			if( ret >= 0 && ret <= 3 ){
				_set_search_data( wk, SEARCH_DATAID_SEX, ret );
				PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0, ret );
				PrintNowSelect( wk, wk->msg, sex_msg_tbl[wk->param->syswk->search_data.sex] );
				FRAMELIST_SetCursorPos( wk->frame_list_work, ret );
			}
		}
		break;
	}

	return seq;
}
static int Subseq_SelectSexExit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	FRAMELIST_Exit( wk->frame_list_work );
	return SUBSEQ_START;
}

static int Subseq_SelectItemHaveInit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	ClearMainDisp( wk );

	FrameListInit(wk, BOXSEARCH_LISTID_MOTIMONO );

	SubTextPrint( wk, SUBSEQ_SELECTITEMHAVEINIT );

	TransMainBGScreen( wk, 2 );

	PrintNowCategory( wk, BOXSEARCH_LISTID_MOTIMONO );
	PrintNowSelect( wk, wk->msg, item_msg_tbl[wk->param->syswk->search_data.item] );

	// チェックボックス表示
	PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0, wk->param->syswk->search_data.item );

	// FRAMELIST初期化待ちの後ポケモン名選択へ
	_set_subseq_nextseq( wk,  SUBSEQ_SELECTITEMHAVE );
  
	return SUBSEQ_FRAMELISTINITWAIT;
}
static int Subseq_SelectItemHave(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	u32	result = FRAMELIST_Main( wk->frame_list_work );

	switch( result ){
	case 0:
	case 1:
	case 2:
		_set_search_data( wk, SEARCH_DATAID_ITEM, result );
		PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0, result );
		PrintNowSelect( wk, wk->msg, item_msg_tbl[wk->param->syswk->search_data.item] );
		break;

	case FRAMELIST_RET_NONE:
		if( CheckCancelButton( wk ) == TRUE ){
			seq = SetButtonAnime( wk, BUTTON_ID_TB_RETURN, 0, SUBSEQ_SELECTITEMHAVEEXIT );
		}else{
			int	ret = HitCheckListBox( wk );
			if( ret >= 0 && ret <= 2 ){
				_set_search_data( wk, SEARCH_DATAID_ITEM, ret );
				PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0, ret );
				PrintNowSelect( wk, wk->msg, item_msg_tbl[wk->param->syswk->search_data.item] );
				FRAMELIST_SetCursorPos( wk->frame_list_work, ret );
			}
		}
		break;
	}

	return seq;
}
static int Subseq_SelectItemHaveExit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	FRAMELIST_Exit( wk->frame_list_work );
	return SUBSEQ_START;
}

static int Subseq_SelectMarkInit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	int	i;

	ClearMainDisp( wk );

  FrameListInit( wk, BOXSEARCH_LISTID_MARK );

  SubTextPrint( wk, SUBSEQ_SELECTMARKINIT );

	TransMainBGScreen( wk, 2 );

	PrintNowCategory( wk, BOXSEARCH_LISTID_MARK );

	// チェックボックス表示
	for( i=0; i<6; i++ ){
		PutCheckObj( wk, BOXSEARCH_OBJID_CHECK0+i, i );
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_CHECK0+i], _get_search_data_mark(wk,i) );
	}

	SetMarkDataObj( wk, BOXSEARCH_OBJID_MARK0_0 );
	SetMarkDataObj( wk, BOXSEARCH_OBJID_MARK1_0 );
	PutMarkDataObj( wk, 1 );
	SetMarkDataObjDrawEnable( wk, BOXSEARCH_OBJID_MARK0_0, TRUE );
	SetMarkDataObjDrawEnable( wk, BOXSEARCH_OBJID_MARK1_0, TRUE );

	// FRAMELIST初期化待ちの後ポケモン名選択へ
	_set_subseq_nextseq( wk,  SUBSEQ_SELECTMARK );

	return SUBSEQ_FRAMELISTINITWAIT;
}
static int Subseq_SelectMark( BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	u32	result = FRAMELIST_Main( wk->frame_list_work );

	switch( result ){
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		{
			int	prm = _get_search_data_mark(wk,result) ^ 1;
			_set_search_data_mark( wk, result, prm );
			GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_CHECK0+result], prm );
		}
		SetMarkDataObj( wk, BOXSEARCH_OBJID_MARK0_0 );
		SetMarkDataObj( wk, BOXSEARCH_OBJID_MARK1_0 );
		break;

	case FRAMELIST_RET_NONE:
		if( CheckCancelButton( wk ) == TRUE ){
			seq = SetButtonAnime( wk, BUTTON_ID_TB_RETURN, 0, SUBSEQ_SELECTSEXEXIT );
		}else{
			int	ret = HitCheckListBox( wk );
			if( ret >= 0 && ret <= 5 ){
				int	prm = _get_search_data_mark(wk,ret) ^ 1;
				_set_search_data_mark( wk, ret, prm );
				GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_CHECK0+ret], prm );
				FRAMELIST_SetCursorPos( wk->frame_list_work, ret );
				SetMarkDataObj( wk, BOXSEARCH_OBJID_MARK0_0 );
				SetMarkDataObj( wk, BOXSEARCH_OBJID_MARK1_0 );
			}
		}
		break;
	}

  return seq;
}
static int Subseq_SelectMarkExit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	FRAMELIST_Exit( wk->frame_list_work );
	return SUBSEQ_START;
}

// 検索条件ON
static int Subseq_SearchOn(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	change_search_checkbox( wk, BOXSEARCH_PLATE_ON );
	_set_search_data( wk, SEARCH_DATAID_ON_OFF, TRUE );

	if( CURSORMOVE_CursorOnOffGet( wk->cmwk ) == TRUE ){
		GFL_CLACT_WK_SetAnmSeq( wk->clwk[BOXSEARCH_OBJID_BUTTON_ON], BOXSEARCH_ANM_SEARCHON_ON );
	}
	return SUBSEQ_MAIN;
}
// 検索条件をONにして終了
static int Subseq_SearchOnEnd( BOX_SEARCH_MAIN_WORK * wk, int seq )
{
	Subseq_SearchOn( wk, 0 );
	return SUBSEQ_SEARCHEXIT;
}

// 検索条件OFF
static int Subseq_SearchOff(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	change_search_checkbox( wk, BOXSEARCH_PLATE_OFF );
	_set_search_data( wk, SEARCH_DATAID_ON_OFF, FALSE );

	if( CURSORMOVE_CursorOnOffGet( wk->cmwk ) == TRUE ){
		GFL_CLACT_WK_SetAnmSeq( wk->clwk[BOXSEARCH_OBJID_BUTTON_OFF], BOXSEARCH_ANM_SEARCHOFF_ON );
	}
	return SUBSEQ_MAIN;
}
// 検索条件リセット
static int Subseq_SearchReset(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	_reset_search_info( wk );
	// 入力済情報を描画
	DecideInfoPrint( wk, TRUE );    
	// マーキング反映
	SetMarkDataObj( wk, BOXSEARCH_OBJID_MARK0_0 );

	if( CURSORMOVE_CursorOnOffGet( wk->cmwk ) == TRUE ){
		GFL_CLACT_WK_SetAnmSeq( wk->clwk[BOXSEARCH_OBJID_RESET], BOXSEARCH_ANM_RESET_ON );
	}

	return SUBSEQ_MAIN;
}
static int Subseq_AnimeWait(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}
static int Subseq_FramelistInitWait(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
	if( FRAMELIST_Init( wk->frame_list_work )==FALSE ){
		if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY ){
			FRAMELIST_SetCursorPosPalette( wk->frame_list_work );
		}
		seq = wk->next_subseq;
	}
	return seq;
}

static int Subseq_FrameListScroll( BOX_SEARCH_MAIN_WORK * wk, int seq )
{
	switch( wk->flscroll ){
	case 0:
		if( FRAMELIST_Main( wk->frame_list_work ) == FRAMELIST_RET_NONE ){
			wk->flscroll++;
		}
		PutScrollBarObj( wk );
		break;
	case 1:
		if( GFL_CLACT_WK_CheckAnmActive( wk->clwk[wk->buttonID] ) == FALSE ){
			wk->flscroll = 0;
			SetListPageArrowAnime( wk, FALSE );
			return wk->next_subseq;
		}
		break;
	}
	return SUBSEQ_FRAMELISTSCROLL;
}


static int Subseq_SearchExit(BOX_SEARCH_MAIN_WORK *wk, int seq)
{
  return seq;
}

// ボタンアニメ終了待ち
static int Subseq_ButtonAnime( BOX_SEARCH_MAIN_WORK * wk, int seq )
{
	if( wk->buttonID == BUTTON_ID_TB_RETURN ){
		if( TOUCHBAR_GetTrg(wk->touchbar) == TOUCHBAR_ICON_RETURN ){
			return wk->next_subseq;
		}
	}else{
		if( GFL_CLACT_WK_CheckAnmActive( wk->clwk[wk->buttonID] ) == FALSE ){
			return wk->next_subseq;
		}
	}

	return seq;
}

// 各検索条件選択へ
static int Subseq_SelectSet( BOX_SEARCH_MAIN_WORK * wk, int seq )
{
	return func_table[wk->main_func_pos-BOXSEARCH_PLATE_0];
}



static void _set_subseq_nextseq( BOX_SEARCH_MAIN_WORK *wk, int next )
{
  wk->next_subseq = next;
}

// ボタンアニメセット
static int SetButtonAnime( BOX_SEARCH_MAIN_WORK * wk, int buttonID, int anmID, int next )
{
	if( buttonID != BUTTON_ID_TB_RETURN ){
		SetObjAutoAnime( wk, buttonID, anmID );
	}
	wk->buttonID = buttonID;
	_set_subseq_nextseq( wk, next );
	return SUBSEQ_BUTTON_ANIME;
}

// フレームリストスクロールセット
static int SetFrameListScroll( BOX_SEARCH_MAIN_WORK * wk, int buttonID, int anmID, int next )
{
	SetObjAutoAnime( wk, buttonID, anmID );
	wk->buttonID = buttonID;
	wk->flscroll = 0;
	_set_subseq_nextseq( wk, next );
	return SUBSEQ_FRAMELISTSCROLL;
}


//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
// タッチ・カーソル制御
//=======================================================================================
static const u8 plate_anm_tbl[][3]={
  { BOXSEARCH_ANM_FRAME, BOXSEARCH_ANM_FRAME_ON, BOXSEARCH_ANM_FRAME_DECIDE },
  { BOXSEARCH_ANM_FRAME, BOXSEARCH_ANM_FRAME_ON, BOXSEARCH_ANM_FRAME_DECIDE },
  { BOXSEARCH_ANM_FRAME, BOXSEARCH_ANM_FRAME_ON, BOXSEARCH_ANM_FRAME_DECIDE },
  { BOXSEARCH_ANM_FRAME, BOXSEARCH_ANM_FRAME_ON, BOXSEARCH_ANM_FRAME_DECIDE },
  { BOXSEARCH_ANM_FRAME, BOXSEARCH_ANM_FRAME_ON, BOXSEARCH_ANM_FRAME_DECIDE },
  { BOXSEARCH_ANM_FRAME, BOXSEARCH_ANM_FRAME_ON, BOXSEARCH_ANM_FRAME_DECIDE },
  { BOXSEARCH_ANM_RESET, BOXSEARCH_ANM_RESET_ON, BOXSEARCH_ANM_RESET_DECIDE },
  { BOXSEARCH_ANM_SEARCHON,  BOXSEARCH_ANM_SEARCHON_ON,  BOXSEARCH_ANM_SEARCHON_DECIDE  },
  { BOXSEARCH_ANM_SEARCHOFF, BOXSEARCH_ANM_SEARCHOFF_ON, BOXSEARCH_ANM_SEARCHOFF_DECIDE },
};

//----------------------------------------------------------------------------------
/**
 * @brief カーソル表示コールバック
 *
 * @param   work    
 * @param   pos   
 * @param   old   
 */
//----------------------------------------------------------------------------------
static void cursor_on_callback( void *work, int pos, int old )
{
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK*)work;
  int i;
  for(i=0;i<=BOXSEARCH_PLATE_OFF;i++){
    if(pos==i){
      GFL_CLACT_WK_SetAnmSeq( wk->clwk[i], plate_anm_tbl[i][1] );
    }else{
      GFL_CLACT_WK_SetAnmSeq( wk->clwk[i], plate_anm_tbl[i][0] );
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief カーソル非表示コールバック
 *
 * @param   work    
 * @param   pos   
 * @param   old   
 */
//----------------------------------------------------------------------------------
static void cursor_off_callback( void *work, int pos, int old )
{
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK*)work;
  int i;

  for(i=0;i<=BOXSEARCH_PLATE_OFF;i++){
    GFL_CLACT_WK_SetAnmSeq( wk->clwk[i], plate_anm_tbl[i][0] );
  }
  
}

//----------------------------------------------------------------------------------
/**
 * @brief カーソル移動コールバック
 *
 * @param   work    
 * @param   pos   
 * @param   old   
 */
//----------------------------------------------------------------------------------
static void cursor_move_callback( void *work, int pos, int old )
{
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK*)work;
  int i;
  for(i=0;i<=BOXSEARCH_PLATE_OFF;i++){
    if(pos==i){
      GFL_CLACT_WK_SetAnmSeq( wk->clwk[i], plate_anm_tbl[i][1] );
    }else{
      GFL_CLACT_WK_SetAnmSeq( wk->clwk[i], plate_anm_tbl[i][0] );
    }
  }
  
}

//----------------------------------------------------------------------------------
/**
 * @brief タッチコールバック
 *
 * @param   work    
 * @param   pos   
 * @param   old   
 */
//----------------------------------------------------------------------------------
static void touch_callback( void *work, int pos, int old )
{
/*
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK*)work;
  int i;
  for(i=0;i<BOXSEARCH_PLATE_MAX;i++){
    if(pos==i){
      GFL_CLACT_WK_SetAnmSeq( wk->clwk[i], plate_anm_tbl[i][2] );
    }else{
      GFL_CLACT_WK_SetAnmSeq( wk->clwk[i], plate_anm_tbl[i][0] );
    }
  }
  if(pos==BOXSEARCH_PLATE_ON || pos==BOXSEARCH_PLATE_OFF){
    change_search_checkbox(wk,pos);
  }
*/
}

/*
static void cm_dummy_callback( void *work, int pos, int old )
{
}
*/


static const checkbox_tbl[][2]={
  {BOXSEARCH_ANM_CHECKON,   BOXSEARCH_ANM_CHECKBOX,},
  {BOXSEARCH_ANM_CHECKBOX,  BOXSEARCH_ANM_CHECKON,},
};

//----------------------------------------------------------------------------------
/**
 * @brief チェックボックスの表示状態を変更する
 *
 * @param   wk    
 * @param   pos   カーソル位置（SEARCH_ON か SEARCH_OFF しかこないはず）
 */
//----------------------------------------------------------------------------------
static void change_search_checkbox( BOX_SEARCH_MAIN_WORK *wk, int pos )
{
  GF_ASSERT( pos==BOXSEARCH_PLATE_ON || pos==BOXSEARCH_PLATE_OFF );

  GFL_CLACT_WK_SetAnmSeq( wk->clwk[BOXSEARCH_OBJID_BUTTON_ON_CHECKBOX], 
                          checkbox_tbl[pos-BOXSEARCH_PLATE_ON][0] );
  GFL_CLACT_WK_SetAnmSeq( wk->clwk[BOXSEARCH_OBJID_BUTTON_OFF_CHECKBOX], 
                          checkbox_tbl[pos-BOXSEARCH_PLATE_ON][1] );
}


//----------------------------------------------------------------------------------
/**
 * @brief 検索データの内容をチェックボックスに反映させる
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void put_search_checkbox( BOX_SEARCH_MAIN_WORK *wk )
{
  BOX_SEARCH_DATA *data = &wk->param->syswk->search_data;
  if(data->search_on){
    change_search_checkbox(wk,BOXSEARCH_PLATE_ON);
  }else{
    change_search_checkbox(wk,BOXSEARCH_PLATE_OFF);
  }
}

//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
// テキスト関連処理
//=======================================================================================

static const u16 str_table[]={
  mes_box_search_type_01,     // BOXSEARCH_STRID_POKEMON,  
  mes_box_search_type_02,     // BOXSEARCH_STRID_SEIKAKU,  
  mes_box_search_type_03,     // BOXSEARCH_STRID_TOKUSEI,  
  mes_box_search_type_04,     // BOXSEARCH_STRID_SEIBETU,  
  mes_box_search_type_05,     // BOXSEARCH_STRID_MOTIMONO, 
  mes_box_search_type_06,     // BOXSEARCH_STRID_MARK,     
  mes_box_search_hyphen_01,   // BOXSEARCH_STRID_NO_SEARCH,
  mes_box_search_text_01,     // BOXSEARCH_STRID_SIBORIKOMI
                              // STRID_TITLEは分岐するので別で処理
};

//----------------------------------------------------------------------------------
/**
 * @brief 何度も表示する文字列を読み込んでおく
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void StrbufLoad( BOX_SEARCH_MAIN_WORK *wk )
{
  int i;
  for(i=0;i<BOXSEARCH_STRID_MAX-1;i++){
    wk->str[i] = GFL_MSG_CreateString( wk->msg, str_table[i] );
  }

  // せいりするかバトルボックスかで分岐
	if( wk->param->box_dat->callMode == BOX_MODE_SEIRI ){
		wk->str[BOXSEARCH_STRID_TITLE] = GFL_MSG_CreateString( wk->msg, mes_box_search_cap_01 );
	}else{
		wk->str[BOXSEARCH_STRID_TITLE] = GFL_MSG_CreateString( wk->msg, mes_box_search_cap_02 );
	}
  
  wk->expand_buf = GFL_STR_CreateBuffer( SUBDISP_TEXT_SIZE, wk->heapID );

}

//----------------------------------------------------------------------------------
/**
 * @brief 事前読み込みの文字列を破棄
 */
//----------------------------------------------------------------------------------
static void StrbufRelease( BOX_SEARCH_MAIN_WORK *wk )
{
  int i;

  GFL_STR_DeleteBuffer( wk->expand_buf );

  for(i=0;i<BOXSEARCH_STRID_MAX;i++){
    GFL_STR_DeleteBuffer( wk->str[i] );
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief 既に読み込み済みの文字列を描画する
 *
 * @param   wk    
 * @param   bmpid   
 * @param   strbufID    
 */
//----------------------------------------------------------------------------------
static void PrintStrbuf( BOX_SEARCH_MAIN_WORK *wk, int bmpid, int strbufID, int y )
{
 
  PRINT_UTIL_PrintColor( &wk->print_util[bmpid], wk->print_que, 0, y, 
                    wk->str[strbufID], wk->font, Window0BmpDataTable[bmpid]->chr);
}

//----------------------------------------------------------------------------------
/**
 * @brief メッセージデータから読み込んで文字列を描画する
 *
 * @param   wk    
 * @param   bmpid   
 * @param   msgID   
 */
//----------------------------------------------------------------------------------
static void PrintMessage( BOX_SEARCH_MAIN_WORK *wk, int bmpid, int msgID, int y )
{
  STRBUF *strbuf = GFL_MSG_CreateString( wk->msg, msgID );
  PRINT_UTIL_PrintColor( &wk->print_util[bmpid], wk->print_que, 
                         0, y, strbuf, wk->font, Window0BmpDataTable[bmpid]->chr );
  GFL_STR_DeleteBuffer( strbuf );
}


//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
// リスト関連処理
//=======================================================================================
// リストタッチデータ
static const FRAMELIST_TOUCH_DATA FrameListTouchTbl[] =
{
  { {   0,  23, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 00: 
  { {  24,  47, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 01: 
  { {  48,  71, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 02: 
  { {  72,  95, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 03: 
  { {  96, 119, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 04: 
  { { 120, 143, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 05: 
  { { 144, 167, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 06: 

  { {   0, 168, 232, 255 }, FRAMELIST_TOUCH_PARAM_RAIL },     // 07: レール

  { { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+23, TOUCHBAR_ICON_X_05, TOUCHBAR_ICON_X_05+23 }, FRAMELIST_TOUCH_PARAM_PAGE_UP },  // 08: 左
  { { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+23, TOUCHBAR_ICON_X_06, TOUCHBAR_ICON_X_06+23 }, FRAMELIST_TOUCH_PARAM_PAGE_DOWN },// 09: 右

  { { GFL_UI_TP_HIT_END, 0, 0, 0 }, 0 },
};
// スクロールなしのタッチデータ
static const FRAMELIST_TOUCH_DATA FrameListTouchNoScrollTbl[] =
{
  { {   0,  23, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 00: 
  { {  24,  47, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 01: 
  { {  48,  71, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 02: 
  { {  72,  95, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 03: 
  { {  96, 119, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 04: 
  { { 120, 143, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 05: 
  { { 144, 167, 136, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },    // 06: 

  { { GFL_UI_TP_HIT_END, 0, 0, 0 }, 0 },
};


static const FRAMELIST_HEADER FrameListHeader = {
  BG_FRAME_BUTTON_M,      // 下画面ＢＧ
  FRAMELIST_SUB_BG_NONE,  // 上画面ＢＧ ( FRAMELIST_SUB_BG_NONE = 未使用 )
      
  14,   // 項目フレーム表示開始Ｘ座標
  0,    // 項目フレーム表示開始Ｙ座標
  16,   // 項目フレーム表示Ｘサイズ
  3,    // 項目フレーム表示Ｙサイズ
      
  3,    // フレーム内に表示するBMPWINの表示Ｘ座標
  0,    // フレーム内に表示するBMPWINの表示Ｙ座標
  12,   // フレーム内に表示するBMPWINの表示Ｘサイズ
  3,    // フレーム内に表示するBMPWINの表示Ｙサイズ
  1,    // フレーム内に表示するBMPWINのパレット
      
  {24,12,8,6,4,3,},    // スクロール速度 [0] = 最速 ※itemSizY*8を割り切れる値であること！
   5,   // 選択項目のパレット
  21,   // スクロールバーのＹサイズ（キャラ単位）
   7,   // 項目登録数
   2,   // 背景登録数
   0,   // 初期カーソル位置
   7,   // カーソル移動範囲
   0,   // 初期スクロール値
      
  FrameListTouchTbl,  // タッチデータ
      
  NULL, // コールバック関数
  NULL, // コールバック関数に渡されるワーク
};


static void ListCallBackHeadword_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp );
static void ListCallBackHeadword_Move( void *work, u32 pos, BOOL flag);
static void ListCallBackHeadword_Scroll( void *work, s8 mv );
static void ListCallBackHeadword2_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp );
static void ListCallBackHeadword2_Move( void *work, u32 pos, BOOL flag);
static void ListCallBackHeadword2_Scroll( void *work, s8 mv );

static void ListCallBackSeikaku_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp );
static void ListCallBackSeikaku_Move( void *work, u32 pos, BOOL flag);
static void ListCallBackSeikaku_Scroll( void * work, s8 mv );

static void ListCallBackTokusei_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp );
static void ListCallBackTokusei_Move( void *work, u32 pos, BOOL flag);
static void ListCallBackTokusei_Scroll( void * work, s8 mv );

static void ListCallBackMark_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp );
static void ListCallBackPokemon_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp );
static void ListCallBackPokemon_Move(void *work, u32 pos, BOOL flag);
static void ListCallBackSex_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp );
static void ListCallBackItemHave_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp );

static void _liststr_print(BOX_SEARCH_MAIN_WORK *wk, int pos, PRINT_UTIL *print_util, GFL_MSGDATA *msg );
static void _liststr_print_null( BOX_SEARCH_MAIN_WORK *wk, int pos, PRINT_UTIL *print_util, GFL_MSGDATA *msg );

// リストコールバック
static const FRAMELIST_CALLBACK FrameListHeadwordCallback = {
  ListCallBackHeadword_Draw,
  ListCallBackHeadword_Move,
  ListCallBackHeadword_Scroll,
};
static const FRAMELIST_CALLBACK FrameListHeadword2Callback = {
  ListCallBackHeadword2_Draw,
  ListCallBackHeadword2_Move,
  ListCallBackHeadword2_Scroll,
};
static const FRAMELIST_CALLBACK FrameListPokemonCallback = {
  ListCallBackPokemon_Draw,
  ListCallBackPokemon_Move,
  ListCallBackHeadword_Scroll,
};
static const FRAMELIST_CALLBACK FrameListSeikakuCallback = {
  ListCallBackSeikaku_Draw,
  ListCallBackSeikaku_Move,
  ListCallBackSeikaku_Scroll,
};
static const FRAMELIST_CALLBACK FrameListTokuseiCallback = {
  ListCallBackTokusei_Draw,
  ListCallBackTokusei_Move,
  ListCallBackTokusei_Scroll,
};
static const FRAMELIST_CALLBACK FrameListSexCallback = {
  ListCallBackSex_Draw,
  ListCallBackHeadword_Move,
  ListCallBackHeadword_Scroll,
};
static const FRAMELIST_CALLBACK FrameListItemHaveCallback = {
  ListCallBackItemHave_Draw,
  ListCallBackHeadword_Move,
  ListCallBackHeadword_Scroll,
};
static const FRAMELIST_CALLBACK FrameListMarkCallback = {
  ListCallBackMark_Draw,
  ListCallBackHeadword_Move,
  ListCallBackHeadword_Scroll,
};

//----------------------------------------------------------------------------------
/**
 * @brief 母音・子音選択時リスト表示コールバック
 *
 * @param   work    
 * @param   pos   
 * @param   print_util    
 * @param   y   
 * @param   disp    
 */
//----------------------------------------------------------------------------------
static void ListCallBackHeadword_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp )
{
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK *)work;
  _liststr_print( wk, pos, print_util, wk->msg );

	if( pos == 0 && wk->param->syswk->search_data.tokusei == 0 ){
		PutListCheckObj( wk, BOXSEARCH_OBJID_CHECK0, y+12 );
	}
}
static void ListCallBackHeadword_Move( void *work, u32 pos, BOOL flag)
{
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK *)work;
}
static void ListCallBackHeadword_Scroll( void *work, s8 mv )
{
	BOX_SEARCH_MAIN_WORK * wk = (BOX_SEARCH_MAIN_WORK *)work;

	ScrollListCheckObj( wk, mv*-1 );
	SetListPageArrowAnime( wk, TRUE );
}

static void ListCallBackHeadword2_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp )
{
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK *)work;

	if( get_sort_tokusei_num(wk->topword-1,pos) != 0 ){
		_liststr_print( wk, pos, print_util, wk->msg );
	}else{
		_liststr_print_null( wk, pos, print_util, wk->msg );
	}
}
static void ListCallBackHeadword2_Move( void *work, u32 pos, BOOL flag)
{
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK *)work;
}
static void ListCallBackHeadword2_Scroll( void *work, s8 mv )
{
	BOX_SEARCH_MAIN_WORK * wk = (BOX_SEARCH_MAIN_WORK *)work;
}


static void ListCallBackPokemon_Move(void *work, u32 pos, BOOL flag)
{
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK *)work;
  STRBUF *strbuf = GFL_MSG_CreateString( wk->msg, mes_box_search_text_03_01 );
  int poke = FRAMELIST_GetItemParam( wk->frame_list_work, pos );

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->bmp_win[BOXSEARCH_BMPID_TEXT] ), 0 );
  WORDSET_RegisterPokeMonsNameNo( wk->wordset, 0, poke);
  {
    int type1,type2;
    POKEMON_PERSONAL_DATA *personal =  POKE_PERSONAL_OpenHandle( poke, 0, wk->heapID );
    type1 = POKE_PERSONAL_GetParam( personal, POKEPER_ID_type1 );
    type2 = POKE_PERSONAL_GetParam( personal, POKEPER_ID_type2 );
    WORDSET_RegisterPokeTypeName( wk->wordset, 1, type1 );
    WORDSET_RegisterPokeTypeName( wk->wordset, 2, type2 );
    POKE_PERSONAL_CloseHandle( personal );
  }
  WORDSET_ExpandStr( wk->wordset, wk->expand_buf, strbuf );
  PRINT_UTIL_PrintColor( &wk->print_util[BOXSEARCH_BMPID_TEXT], wk->print_que, 
                     0, 0, wk->expand_buf, wk->font, Window0BmpDataTable[BOXSEARCH_BMPID_TEXT]->chr );
  GFL_STR_DeleteBuffer( strbuf );
  GFL_BMPWIN_MakeTransWindow_VBlank( wk->bmp_win[BOXSEARCH_BMPID_TEXT] );
}

static void ListCallBackPokemon_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp )
{
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK *)work;
  int param = FRAMELIST_GetItemParam( wk->frame_list_work, pos );
  if(param==wk->param->syswk->search_data.pokemon_id){
    GFL_CLACTPOS cp_pos = { 127, 0 };
    cp_pos.y = y+13;
    GFL_CLACT_WK_SetWldPos( wk->clwk[ BOXSEARCH_OBJID_CHECK0 ], &cp_pos );
    OS_Printf("checkbox hit =%d\n", wk->param->syswk->search_data.pokemon_id);
  }

  _liststr_print( wk, pos, print_util, wk->mons_msg );
  
}



static void ListCallBackSeikaku_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp )
{
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK *)work;
  int width,bmp_width,x;
  PRINT_QUE *que = FRAMELIST_GetPrintQue( wk->frame_list_work );
  int param = FRAMELIST_GetItemParam( wk->frame_list_work, pos );
  STRBUF *strbuf = NULL;

  if(param==0){
    strbuf = wk->str[BOXSEARCH_STRID_NO_SEARCH];
  }else{
    strbuf = GFL_MSG_CreateString( wk->seikaku_msg, param-1 );
  }
  width     = PRINTSYS_GetStrWidth( strbuf, wk->font, 0 );
  bmp_width = GFL_BMPWIN_GetSizeX( print_util->win )*8;
  x         = (bmp_width - width)/2;

  PRINT_UTIL_PrintColor( print_util, que, x, 4, strbuf, wk->font, PRINTSYS_MACRO_LSB(15, 14, 0) );

  // 性格を描画した時だけ文字列が確保されているので解放
  if(param!=0){
    GFL_STR_DeleteBuffer( strbuf );
  }

	if( wk->param->syswk->search_data.seikaku == param ){
		PutListCheckObj( wk, BOXSEARCH_OBJID_CHECK0, y+12 );
	}
}
static void ListCallBackSeikaku_Move( void * work, u32 pos, BOOL flag )
{
	BOX_SEARCH_MAIN_WORK * wk = (BOX_SEARCH_MAIN_WORK *)work;

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->bmp_win[BOXSEARCH_BMPID_TEXT]), 0 );

	if( pos != 0 ){
		STRBUF * strbuf = GFL_MSG_CreateString( wk->msg, mes_box_search_text_04_01+pos-1 );
		PRINT_UTIL_PrintColor(
			&wk->print_util[BOXSEARCH_BMPID_TEXT], wk->print_que,
			0, 0, strbuf, wk->font, Window0BmpDataTable[BOXSEARCH_BMPID_TEXT]->chr );
		GFL_STR_DeleteBuffer( strbuf );
		BmpWinTransScreen( wk->bmp_win[BOXSEARCH_BMPID_TEXT] );
	}else{
		GFL_BMPWIN_MakeTransWindow_VBlank( wk->bmp_win[BOXSEARCH_BMPID_TEXT] );
	}
}
static void ListCallBackSeikaku_Scroll( void * work, s8 mv )
{
	BOX_SEARCH_MAIN_WORK * wk = (BOX_SEARCH_MAIN_WORK *)work;

	ScrollListCheckObj( wk, mv*-1 );
	SetListPageArrowAnime( wk, TRUE );
}

static void ListCallBackTokusei_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp )
{
	BOX_SEARCH_MAIN_WORK * wk = (BOX_SEARCH_MAIN_WORK *)work;

	_liststr_print( wk, pos, print_util, wk->tokusei_msg );

	if( wk->param->syswk->search_data.tokusei == FRAMELIST_GetItemParam(wk->frame_list_work,pos) ){
		PutListCheckObj( wk, BOXSEARCH_OBJID_CHECK0, y+12 );
	}
}
static void ListCallBackTokusei_Move( void *work, u32 pos, BOOL flag)
{
	BOX_SEARCH_MAIN_WORK * wk = (BOX_SEARCH_MAIN_WORK *)work;

	PrintTokuseiInfo( wk, FRAMELIST_GetItemParam(wk->frame_list_work,pos) );
}
static void ListCallBackTokusei_Scroll( void * work, s8 mv )
{
	BOX_SEARCH_MAIN_WORK * wk = (BOX_SEARCH_MAIN_WORK *)work;

	if( get_sort_tokusei_num(wk->topword-1,wk->nextword) > 7 ){
		ScrollListCheckObj( wk, mv*-1 );
		SetListPageArrowAnime( wk, TRUE );
	}
}


static void ListCallBackSex_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp )
{
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK *)work;
  
  _liststr_print( wk, pos, print_util, wk->msg );
}

static void ListCallBackItemHave_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp )
{
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK *)work;
  _liststr_print( wk, pos, print_util, wk->msg );
}
static void ListCallBackMark_Draw( void *work, u32 pos, PRINT_UTIL *print_util, s16 y, BOOL disp )
{
  BOX_SEARCH_MAIN_WORK *wk = (BOX_SEARCH_MAIN_WORK *)work;
  PRINT_QUE *que = FRAMELIST_GetPrintQue( wk->frame_list_work );
  
  // 無色の文字を描画しておく
  PRINT_UTIL_PrintColor( print_util, que, 0, 4, 
                         wk->str[BOXSEARCH_STRID_NO_SEARCH], 
                         wk->font, PRINTSYS_MACRO_LSB(0, 0, 0) );
  
}

static void PrintStrBuf( BOX_SEARCH_MAIN_WORK * wk, PRINT_UTIL * print_util, PRINT_QUE * que, GFL_MSGDATA * mman, int strIdx, PRINTSYS_LSB col, int y )
{
	int width, bmp_width, x;
	STRBUF * strbuf = GFL_MSG_CreateString( mman, strIdx );

  width     = PRINTSYS_GetStrWidth( strbuf, wk->font, 0 );
	bmp_width = GFL_BMPWIN_GetSizeX( print_util->win ) * 8;
  x         = (bmp_width - width)/2;

  PRINT_UTIL_PrintColor( print_util, que, x, y, strbuf, wk->font, col );
  GFL_STR_DeleteBuffer( strbuf );
}

static void _strbuf_print( BOX_SEARCH_MAIN_WORK *wk, PRINT_UTIL *print_util, PRINT_QUE *que, GFL_MSGDATA *msg, int param )
{
	PrintStrBuf( wk, print_util, que, msg, param, PRINTSYS_MACRO_LSB(15,14,0), 4 );
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
static void _liststr_print( BOX_SEARCH_MAIN_WORK *wk, int pos, PRINT_UTIL *print_util, GFL_MSGDATA *msg )
{
  PRINT_QUE *que = FRAMELIST_GetPrintQue( wk->frame_list_work );
  int param = FRAMELIST_GetItemParam( wk->frame_list_work, pos );

  _strbuf_print( wk, print_util, que, msg, param );
}
static void _liststr_print_null( BOX_SEARCH_MAIN_WORK *wk, int pos, PRINT_UTIL *print_util, GFL_MSGDATA *msg )
{
  PRINT_QUE *que = FRAMELIST_GetPrintQue( wk->frame_list_work );
  int param = FRAMELIST_GetItemParam( wk->frame_list_work, pos );

	PrintStrBuf( wk, print_util, que, msg, param, PRINTSYS_MACRO_LSB(13,12,0), 4 );
}


//----------------------------------------------------------------------------------
/**
 * @brief 母音と子音が決定した後のポケモンの数を返す
 *
 * @param   head    
 * @param   child   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int get_sort_pokename_num( int head, int child )
{
  int decide = head2pokename[head]+child;
  OS_Printf("topword=%d, nextword=%d\n", head, child);
  return sort_pokename_end_index[decide] - sort_pokename_start_index[decide];
}

static int get_sort_tokusei_num( int head, int child )
{
  int decide = head2pokename[head]+child;
  return tokusei_header_aiueo_table[decide][1] - tokusei_header_aiueo_table[decide][0];
  
}

// 上画面に特性表示
static void PrintTokuseiInfo( BOX_SEARCH_MAIN_WORK * wk, u32 id )
{
	STRBUF * strbuf;

	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->bmp_win[BOXSEARCH_BMPID_TEXT]), 0 );

	strbuf = GFL_MSG_CreateString( wk->msg, mes_box_search_text_05_01 );
	WORDSET_RegisterTokuseiName( wk->wordset, 0, id );
	WORDSET_ExpandStr( wk->wordset, wk->expand_buf, strbuf );
	PRINT_UTIL_PrintColor(
		&wk->print_util[BOXSEARCH_BMPID_TEXT], wk->print_que, 
		0, 0, wk->expand_buf, wk->font, Window0BmpDataTable[BOXSEARCH_BMPID_TEXT]->chr );
	GFL_STR_DeleteBuffer( strbuf );

	strbuf = GFL_MSG_CreateString( wk->tokusei_info_msg, id );
	PRINT_UTIL_PrintColor(
		&wk->print_util[BOXSEARCH_BMPID_TEXT], wk->print_que, 
		0, 48, strbuf, wk->font, Window0BmpDataTable[BOXSEARCH_BMPID_TEXT]->chr );
	GFL_STR_DeleteBuffer( strbuf );

	BmpWinTransScreen( wk->bmp_win[BOXSEARCH_BMPID_TEXT] );
}



static void FrameListInit(BOX_SEARCH_MAIN_WORK *wk, int type)
{
  ARCHANDLE * arc_handle;
  FRAMELIST_HEADER  header = FrameListHeader;
  int i;

  header.cbWork = wk;
  arc_handle = GFL_ARC_OpenDataHandle( ARCID_BOX2_GRA, wk->heapID );

	wk->flist_pos_max = 7;

  switch(type){
  case BOXSEARCH_LISTID_NAME_TOPWORD:   // 「ポケモン」母音指定
    header.itemMax = 11;
    header.cbFunc  = &FrameListHeadwordCallback;
    break;
  case BOXSEARCH_LISTID_NAME_NEXTWORD:  // 「ポケモン」子音指定
    header.itemMax = childword_num[wk->topword-1];
    header.cbFunc  = &FrameListHeadword2Callback;
    break;
  case BOXSEARCH_LISTID_POKEMON:        // 「ポケモン」
    {
      int size,count=0,i;
      u16 *sort_data;
      int max   = get_sort_pokename_num(wk->topword-1, wk->nextword);
      int start = sort_pokename_start_index[head2pokename[wk->topword-1]+wk->nextword];
      sort_data = ZukanSortDataGet( wk->heapID, &size );
      for(i=0;i<max;i++){
        if(ZUKANSAVE_GetPokeSeeFlag( wk->zukan, sort_data[start+i])){
          count++;
        }
      }
      GFL_HEAP_FreeMemory( sort_data );
      header.itemMax = count;
      header.cbFunc  = &FrameListPokemonCallback;
    }
    break;
  case BOXSEARCH_LISTID_SEIKAKU:        // 「せいかく」
    header.cbFunc  = &FrameListSeikakuCallback;
    header.itemMax = 26;
    break;
  case BOXSEARCH_LISTID_TOKUSEI_TOPWORD:  // 「とくせい」母音指定
    header.cbFunc  = &FrameListHeadwordCallback;
    header.itemMax = 11;
    break;
  case BOXSEARCH_LISTID_TOKUSEI_NEXTWORD: // 「とくせい」子音指定
    header.cbFunc  = &FrameListHeadword2Callback;
    header.itemMax = childword_num[wk->topword-1];
    break;
  case BOXSEARCH_LISTID_TOKUSEI:        // 「とくせい」
    header.cbFunc  = &FrameListTokuseiCallback;
    header.itemMax = get_sort_tokusei_num(wk->topword-1, wk->nextword);
    break;
  case BOXSEARCH_LISTID_SEIBETU:        // 「せいべつ」
    header.cbFunc  = &FrameListSexCallback;
    header.itemMax = 4;
		wk->flist_pos_max = 4;
    break;
  case BOXSEARCH_LISTID_MOTIMONO:       // 「もちもの」
    header.cbFunc  = &FrameListItemHaveCallback;
    header.itemMax = 3;
		wk->flist_pos_max = 3;
    break;
  case BOXSEARCH_LISTID_MARK:           // 「マーク」
    header.cbFunc  = &FrameListMarkCallback;
    header.itemMax = 6;
		wk->flist_pos_max = 6;
    break;
  }

	if( header.itemMax <= 7 ){
		header.touch   = FrameListTouchNoScrollTbl;
	}
  wk->frame_list_work = FRAMELIST_Create( &header, wk->heapID );
//  FRAMELIST_SetCursorPos( wk->frame_list_work, 0 );

  // 項目背景設定
  FRAMELIST_LoadFrameGraphicAH( wk->frame_list_work, arc_handle, NARC_box_gra_boxsearch_frame_2_NSCR, FALSE, 0 );
  FRAMELIST_LoadFrameGraphicAH( wk->frame_list_work, arc_handle, NARC_box_gra_boxsearch_frame_1_NSCR, FALSE, 1 );

  // 点滅アニメパレット設定
  FRAMELIST_LoadBlinkPalette( wk->frame_list_work, arc_handle, NARC_box_gra_boxsearch_bgd_NCLR, 1, 5 );

  GFL_ARC_CloseDataHandle( arc_handle );

  
  switch(type){
  case BOXSEARCH_LISTID_NAME_TOPWORD:   // 「ポケモン」母音指定
		FRAMELIST_AddItem( wk->frame_list_work, 0, topword_tbl[0] );
    for(i=1;i<header.itemMax;i++){
      FRAMELIST_AddItem( wk->frame_list_work, 1, topword_tbl[i] );
    }
    break;
  case BOXSEARCH_LISTID_NAME_NEXTWORD:  // 「ポケモン」子音指定
    for(i=0;i<header.itemMax;i++){
      FRAMELIST_AddItem( wk->frame_list_work, 1, topword_tbl[wk->topword]+i );
    }
    break;
  case BOXSEARCH_LISTID_POKEMON:        // 「ポケモン」
    {
      int size;
      u16 *sort_data; 
      int start = sort_pokename_start_index[head2pokename[wk->topword-1]+wk->nextword];
      sort_data = ZukanSortDataGet( wk->heapID, &size );
      for(i=0;i<header.itemMax;i++){
        if(ZUKANSAVE_GetPokeSeeFlag( wk->zukan, sort_data[start+i])){
          FRAMELIST_AddItem( wk->frame_list_work, 0, sort_data[start+i] );
        }
      }
      GFL_HEAP_FreeMemory( sort_data );
    }
    break;
  case BOXSEARCH_LISTID_SEIKAKU:        // 「せいかく」
    for(i=0;i<header.itemMax;i++){
      FRAMELIST_AddItem( wk->frame_list_work, 0, i );
    }
    break;
  case BOXSEARCH_LISTID_TOKUSEI_TOPWORD:  // 「とくせい」母音指定
		FRAMELIST_AddItem( wk->frame_list_work, 0, topword_tbl[0] );
    for(i=1;i<header.itemMax;i++){
      FRAMELIST_AddItem( wk->frame_list_work, 1, topword_tbl[i] );
    }
    break;
  case BOXSEARCH_LISTID_TOKUSEI_NEXTWORD: // 「とくせい」子音指定
    for(i=0;i<header.itemMax;i++){
      FRAMELIST_AddItem( wk->frame_list_work, 1, topword_tbl[wk->topword]+i );
    }
    break;
  case BOXSEARCH_LISTID_TOKUSEI:        // 「とくせい」
    {
      int start = tokusei_header_aiueo_table[head2pokename[wk->topword-1]+wk->nextword][0]-1;
      for(i=0;i<header.itemMax;i++){
        FRAMELIST_AddItem( wk->frame_list_work, 0, tokusei_aiueo_table[start+i] );
      }
    }
    break;
  case BOXSEARCH_LISTID_SEIBETU:        // 「せいべつ」
    for(i=0;i<header.itemMax;i++){
      FRAMELIST_AddItem( wk->frame_list_work, 0, sex_msg_tbl[i] );
    }
    break;
  case BOXSEARCH_LISTID_MOTIMONO:       // 「もちもの」
    for(i=0;i<header.itemMax;i++){
      FRAMELIST_AddItem( wk->frame_list_work, 0, item_msg_tbl[i] );
    }
    break;
  case BOXSEARCH_LISTID_MARK:           // 「マーク」(中身は全部０）
    for(i=0;i<header.itemMax;i++){
      FRAMELIST_AddItem( wk->frame_list_work, 0, 0 );
    }
    break;
  }
  
}

//----------------------------------------------------------------------------------
/**
 * @brief FRAMELIST解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void FrameListExit(BOX_SEARCH_MAIN_WORK *wk)
{
  FRAMELIST_Exit( wk->frame_list_work );
}

static void FrameListClear( BOX_SEARCH_MAIN_WORK * wk )
{
	GFL_BG_ClearScreen( GFL_BG_FRAME0_M );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_M );
	FrameListChangeClear( wk );
}

static void FrameListChangeClear( BOX_SEARCH_MAIN_WORK * wk )
{
	u32	i;

	GFL_BG_ClearScreen( GFL_BG_FRAME2_M );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_M );

	for( i=BOXSEARCH_OBJID_CHECK0; i<=BOXSEARCH_OBJID_SCROL; i++ ){
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], FALSE );
	}

	SetMarkDataObjDrawEnable( wk, BOXSEARCH_OBJID_MARK0_0, FALSE );
	SetMarkDataObjDrawEnable( wk, BOXSEARCH_OBJID_MARK1_0, FALSE );

	GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_ARROW_L], FALSE );
	GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_ARROW_R], FALSE );
}


// チェックマーク表示
static void PutListCheckObj( BOX_SEARCH_MAIN_WORK * wk, int id, s16 y )
{
	GFL_CLACTPOS cp_pos;
	cp_pos.x = 127;
	cp_pos.y = y;
	GFL_CLACT_WK_SetWldPos( wk->clwk[id], &cp_pos );
	GFL_CLACT_WK_SetDrawEnable( wk->clwk[id], TRUE );
}
static void PutCheckObj( BOX_SEARCH_MAIN_WORK * wk, int id, int pos )
{
	PutListCheckObj( wk, id, pos * 24 + 12 );
}

static void ScrollListCheckObj( BOX_SEARCH_MAIN_WORK * wk, s8 mv )
{
	GFL_CLACTPOS cp_pos;
	u32	i;

	for( i=BOXSEARCH_OBJID_CHECK0; i<=BOXSEARCH_OBJID_CHECK9; i++ ){
		if( GFL_CLACT_WK_GetDrawEnable( wk->clwk[i] ) == FALSE ){
			continue;
		}
		GFL_CLACT_WK_GetWldPos( wk->clwk[i], &cp_pos );
		cp_pos.y += mv;
		GFL_CLACT_WK_SetWldPos( wk->clwk[i], &cp_pos );

		if( cp_pos.y <= -12 || cp_pos.y >= 204 ){
			GFL_CLACT_WK_SetDrawEnable( wk->clwk[i], FALSE );
		}
	}
}




#define ZKN_SORTDATA_ONESIZE  ( sizeof(u16) )

//------------------------------------------------------------------
/**
 * @brief   ずかんソートデータ取得（あういえお順）。解放はじぶんで。
 *      
 * @param   heap    ヒープID
 * @param   p_arry_num  通常図鑑と全国図鑑の総数
 *
 * @retval  u16*    読み込んだソートデータ
 */
//------------------------------------------------------------------
static u16* ZukanSortDataGet( int heap, int* p_arry_num )
{
  u32 size;
  u16* p_buf;
  // 読み込み
  p_buf = GFL_ARC_UTIL_LoadEx( ARCID_ZUKAN_DATA, NARC_zukan_data_zkn_sort_aiueo_dat, FALSE, heap, &size );

  *p_arry_num = size / ZKN_SORTDATA_ONESIZE;

  return p_buf;
}



//----------------------------------------------------------------------------------
/**
 * @brief BG転送
 *
 * @param   wk    
 * @param   type    
 */
//----------------------------------------------------------------------------------
static void TransMainBGScreen( BOX_SEARCH_MAIN_WORK *wk, int type )
{
/*
  ARCHANDLE *handle;
  GF_ASSERT(type<3);
  
  handle  = GFL_ARC_OpenDataHandle( ARCID_BOX2_GRA, wk->heapID );
  GFL_ARCHDL_UTIL_TransVramScreen(  handle, main_scr_bg_table[type],  BG_FRAME_BACK_M,  0, 0, 0, wk->heapID );   
  GFL_ARC_CloseDataHandle( handle );
*/
	NNSG2dScreenData * scrn;
	void * buf;

	buf = GFL_ARC_UTIL_LoadScreen( ARCID_BOX2_GRA, main_scr_bg_table[type], FALSE, &scrn, wk->heapID );
	GFL_BG_WriteScreen( BG_FRAME_BACK_M, (const void *)scrn->rawData, 0, 0, 32, 24 );
	GFL_HEAP_FreeMemory( buf );
	GFL_BG_LoadScreenV_Req( BG_FRAME_BACK_M );
}

static void ObjOnOff( BOX_SEARCH_MAIN_WORK *wk, int type, int category )
{
  int i;
  // メイン画面
  if(type==0){
    // プレート表示
    for(i=0;i<11;i++){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_PLATE0+i],TRUE);
    }
    // チェックボックス表示
    for(i=0;i<10;i++){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_CHECK0+i],FALSE);
    }
    // 選択肢のマーキング非表示
    for(i=0;i<6;i++){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_MARK0_0+i], TRUE );
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_MARK1_0+i], FALSE );
    }
    // スクロールつまみ非表示
    GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_SCROL], FALSE );
  }else{
    // プレート非表示
    for(i=0;i<11;i++){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_PLATE0+i],FALSE);
    }
    // チェックボックス1個表示
    GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_CHECK0],TRUE);
    // マーキング表示・非表示
    for(i=0;i<6;i++){
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_MARK0_0+i], onoff_tbl[category][0] );
      GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_MARK1_0+i], onoff_tbl[category][1] );
    }
    // スクロールつまみ表示
    GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_SCROL], onoff_tbl[category][2] );
  }
}


//----------------------------------------------------------------------------------
/**
 * @brief トップページに入力済情報を表示
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void DecideInfoPrint( BOX_SEARCH_MAIN_WORK * wk, BOOL flag )
{
	BOX_SEARCH_DATA *dat = &wk->param->syswk->search_data;
	int i;

	if( flag == FALSE ){
		for( i=BOXSEARCH_BMPID_ANSWER0; i<=BOXSEARCH_BMPID_ANSWER4; i++ ){
			GFL_BMPWIN_ClearScreen( wk->bmp_win[i] );
		}
	}else{
		for( i=BOXSEARCH_BMPID_ANSWER0; i<=BOXSEARCH_BMPID_ANSWER4; i++ ){
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->bmp_win[i] ), 0 );
		}
		// ポケモン名
		if( dat->pokemon_id == 0 ){
			_strbuf_print( wk, &wk->print_util[BOXSEARCH_BMPID_ANSWER0], wk->print_que, wk->msg, mes_box_search_hyphen_01 );
		}else{
			_strbuf_print( wk, &wk->print_util[BOXSEARCH_BMPID_ANSWER0], wk->print_que, wk->mons_msg, dat->pokemon_id );
		}
		// せいかく
		if( dat->seikaku == 0 ){
			_strbuf_print( wk, &wk->print_util[BOXSEARCH_BMPID_ANSWER1], wk->print_que, wk->msg, mes_box_search_hyphen_01 );
		}else{
			_strbuf_print( wk, &wk->print_util[BOXSEARCH_BMPID_ANSWER1], wk->print_que, wk->seikaku_msg, dat->seikaku-1 );
		}
		// とくせい
		if( dat->tokusei == 0 ){
			_strbuf_print( wk, &wk->print_util[BOXSEARCH_BMPID_ANSWER2], wk->print_que, wk->msg, mes_box_search_hyphen_01 );
		}else{
			_strbuf_print( wk, &wk->print_util[BOXSEARCH_BMPID_ANSWER2], wk->print_que, wk->tokusei_msg, dat->tokusei );
		}
		// せいべつ
		_strbuf_print( wk, &wk->print_util[BOXSEARCH_BMPID_ANSWER3], wk->print_que, wk->msg, sex_msg_tbl[dat->sex] );
		// もちもの
		_strbuf_print( wk, &wk->print_util[BOXSEARCH_BMPID_ANSWER4], wk->print_que, wk->msg, item_msg_tbl[dat->item] );

		// BMPWIN転送
		for( i=BOXSEARCH_BMPID_ANSWER0; i<=BOXSEARCH_BMPID_ANSWER4; i++ ){
//			GFL_BMPWIN_TransVramCharacter( wk->bmp_win[i] );
			GFL_BMPWIN_MakeScreen( wk->bmp_win[i] );
		}
	}
	// スクリーン転送
	GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(wk->bmp_win[BOXSEARCH_BMPID_ANSWER0]) );
}


//----------------------------------------------------------------------------------
/**
 * @brief BG面セットアップ（BG転送・BMPWIN描画・OBJ ON/OFF)
 *
 * @param   wk    
 * @param   type    
 * @param   category    
 */
//----------------------------------------------------------------------------------
static void SetupScreen( BOX_SEARCH_MAIN_WORK *wk, int type, int category )
{
  int i;
  switch(type){
  case 0:
    // 項目描画
    for(i=0;i<6;i++){
      PrintStrbuf( wk, BOXSEARCH_BMPID_TOPIC0+i, i, TOPIC_NAME_OFFSET_Y );
      GFL_BMPWIN_MakeTransWindow_VBlank( wk->bmp_win[BOXSEARCH_BMPID_TOPIC0+i] );
    }
    
    ObjOnOff( wk, 0, 0 );
    // マーキング反映
    SetMarkDataObj( wk, BOXSEARCH_OBJID_MARK0_0 );
    // 各種BG設定・転送
    TransMainBGScreen( wk, 0 );
    GFL_BG_ClearScreen( BG_FRAME_BUTTON_M );
    // 入力済情報を描画
    DecideInfoPrint( wk, 1 );    

    break;
  case 1:
  case 2:
    // マーキング反映
    SetMarkDataObj( wk, BOXSEARCH_OBJID_MARK1_0 );
    // 入力済情報を隠す
    DecideInfoPrint( wk, 0 );
    // トップ画面の６つの項目名非表示
    for(i=0;i<6;i++){
      PrintStrbuf( wk, BOXSEARCH_BMPID_TOPIC0+i, i, TOPIC_NAME_OFFSET_Y );
      GFL_BMPWIN_ClearScreen( wk->bmp_win[BOXSEARCH_BMPID_TOPIC0+i] );
    }
    GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(wk->bmp_win[BOXSEARCH_BMPID_TOPIC0]) );
    // 項目名を１つだけ左中央に表示
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( wk->bmp_win[BOXSEARCH_BMPID_NOW_CATEGORY] ), 0 );
    PrintStrbuf( wk, BOXSEARCH_BMPID_NOW_CATEGORY, now_category_tbl[category], 0 );
    GFL_BMPWIN_MakeTransWindow_VBlank( wk->bmp_win[BOXSEARCH_BMPID_NOW_CATEGORY] );
    ObjOnOff( wk, type, category );
    TransMainBGScreen( wk, type );
    break;
  case 3:
    break;
  }
}


//----------------------------------------------------------------------------------
/**
 * @brief 検索条件をセット
 *
 * @param   wk    
 * @param   type    
 * @param   param   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static void _set_search_data( BOX_SEARCH_MAIN_WORK *wk, int type, int param )
{
  BOX_SEARCH_DATA *dat = &wk->param->syswk->search_data;
  switch(type){
  case SEARCH_DATAID_POKEMON:
    dat->pokemon_id = param;
    break;
  case SEARCH_DATAID_ITEM:
    dat->item       = param;
    break;
  case SEARCH_DATAID_SEIKAKU:
    dat->seikaku    = param;
    break;
  case SEARCH_DATAID_TOKUSEI:
    dat->tokusei    = param;
    break;
  case SEARCH_DATAID_SEX:
    dat->sex        = param;
    break;
  case SEARCH_DATAID_MARK:
    dat->mark       = param;
    break;
  case SEARCH_DATAID_ON_OFF:
    dat->search_on  = param;
    break;
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief マークデータの書き換え
 *
 * @param   wk    
 * @param   mark    
 * @param   param   
 */
//----------------------------------------------------------------------------------
static void _set_search_data_mark( BOX_SEARCH_MAIN_WORK *wk, int mark, int param )
{
  BOX_SEARCH_DATA *dat = &wk->param->syswk->search_data;
  u8 tmp = dat->mark&(0xff^(1<<mark));
  
  dat->mark = tmp|(param<<mark);
}
//----------------------------------------------------------------------------------
/**
 * @brief マークデータの状態を取得
 *
 * @param   wk    
 * @param   mark    
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL _get_search_data_mark( BOX_SEARCH_MAIN_WORK *wk, int mark )
{
  BOX_SEARCH_DATA *dat = &wk->param->syswk->search_data;
  return (dat->mark>>mark)&1;
}

//----------------------------------------------------------------------------------
/**
 * @brief 検索条件を取得
 *
 * @param   wk    
 * @param   type    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _get_search_data( BOX_SEARCH_MAIN_WORK *wk, int type )
{
  BOX_SEARCH_DATA *dat = &wk->param->syswk->search_data;
  switch(type){
  case SEARCH_DATAID_POKEMON:
    return dat->pokemon_id;
  case SEARCH_DATAID_ITEM:
    return dat->item;
  case SEARCH_DATAID_SEIKAKU:
    return dat->seikaku;
  case SEARCH_DATAID_TOKUSEI:
    return dat->tokusei;
  case SEARCH_DATAID_SEX:
    return dat->sex;
  case SEARCH_DATAID_ON_OFF:
    return dat->search_on;
  }
  
  GF_ASSERT(0);
  return 0;
}



static void SetObjAutoAnime( BOX_SEARCH_MAIN_WORK * wk, u32 id, u32 anm )
{
	GFL_CLACT_WK_SetAnmFrame( wk->clwk[id], 0 );
	GFL_CLACT_WK_SetAnmSeq( wk->clwk[id], anm );
	GFL_CLACT_WK_SetAutoAnmFlag( wk->clwk[id], TRUE );
}

//----------------------------------------------------------------------------------
/**
 * @brief マーキングOBJのON/OFFを反映させる
 *
 * @param   wk      
 * @param   objpos  マーキングOBJのスタートNO
 */
//----------------------------------------------------------------------------------
static void SetMarkDataObj( BOX_SEARCH_MAIN_WORK * wk, int objpos )
{
  int i;
  GF_ASSERT( objpos>=BOXSEARCH_OBJID_MARK0_0 && objpos<=BOXSEARCH_OBJID_MARK1_5 );
  for(i=0;i<6;i++){
    int flag = _get_search_data_mark( wk, i );
    GFL_CLACT_WK_SetAnmSeq( wk->clwk[objpos+i], i*2+flag );
  }
}

static void PutMarkDataObj( BOX_SEARCH_MAIN_WORK * wk, int id )
{
	GFL_CLACTPOS  pos;
	u32 i;

	if( id == 0 ){
		pos.x = 17*8;
		pos.y = 16*8;
	}else{
		pos.x = 2*8-4;
		pos.y = 12*8-4;
	}

	for( i=BOXSEARCH_OBJID_MARK0_0; i<=BOXSEARCH_OBJID_MARK0_5; i++ ){
		GFL_CLACT_WK_SetPos( wk->clwk[i], &pos, CLSYS_DEFREND_MAIN );
		pos.x += 16;
  }
}

static void SetMarkDataObjDrawEnable( BOX_SEARCH_MAIN_WORK * wk, int id, BOOL flg )
{
	int	i;

	for( i=0; i<6; i++ ){
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[id+i], flg );
  }
}

static void InitScrollBarObj( BOX_SEARCH_MAIN_WORK * wk )
{
	GFL_CLACTPOS  pos = { 244, 12 };
	GFL_CLACT_WK_SetPos( wk->clwk[BOXSEARCH_OBJID_SCROL], &pos, CLSYS_DEFREND_MAIN );
	GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_SCROL], TRUE );
}

static void PutScrollBarObj( BOX_SEARCH_MAIN_WORK * wk )
{
	GFL_CLACTPOS cp_pos;

	GFL_CLACT_WK_GetWldPos( wk->clwk[BOXSEARCH_OBJID_SCROL], &cp_pos );
	cp_pos.y = FRAMELIST_GetScrollBarPY2( wk->frame_list_work, cp_pos.y );
	if( cp_pos.y < 12 ){
		cp_pos.y = 12;
	}else if( cp_pos.y > 156 ){
		cp_pos.y = 156;
	}
  GFL_CLACT_WK_SetWldPos( wk->clwk[BOXSEARCH_OBJID_SCROL], &cp_pos );
}

static void SetListPageArrowAnime( BOX_SEARCH_MAIN_WORK * wk, BOOL anm )
{
	u32	pos = FRAMELIST_GetCursorPos( wk->frame_list_work );

	if( anm == FALSE ){
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_ARROW_L], TRUE );
		GFL_CLACT_WK_SetDrawEnable( wk->clwk[BOXSEARCH_OBJID_ARROW_R], TRUE );
	}

	if( GFL_CLACT_WK_GetAnmSeq( wk->clwk[BOXSEARCH_OBJID_ARROW_L] ) != APP_COMMON_BARICON_CURSOR_LEFT_ON ||
			anm == FALSE ){
		if( FRAMELIST_GetScrollCount( wk->frame_list_work ) == 0 && pos == 0 ){
			SetObjAutoAnime( wk, BOXSEARCH_OBJID_ARROW_L, APP_COMMON_BARICON_CURSOR_LEFT_OFF );
		}else{
			SetObjAutoAnime( wk, BOXSEARCH_OBJID_ARROW_L, APP_COMMON_BARICON_CURSOR_LEFT );
		}
	}
	if( GFL_CLACT_WK_GetAnmSeq( wk->clwk[BOXSEARCH_OBJID_ARROW_R] ) != APP_COMMON_BARICON_CURSOR_RIGHT_ON ||
			anm == FALSE ){
		if( FRAMELIST_CheckScrollMax( wk->frame_list_work ) == FALSE && pos == (wk->flist_pos_max-1) ){
			SetObjAutoAnime( wk, BOXSEARCH_OBJID_ARROW_R, APP_COMMON_BARICON_CURSOR_RIGHT_OFF );
		}else{
			SetObjAutoAnime( wk, BOXSEARCH_OBJID_ARROW_R, APP_COMMON_BARICON_CURSOR_RIGHT );
		}
	}
}


//----------------------------------------------------------------------------------
/**
 * @brief 検索条件のパラメータリセット
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void _reset_search_info( BOX_SEARCH_MAIN_WORK *wk )
{
  _set_search_data( wk, SEARCH_DATAID_POKEMON, 0 );
  _set_search_data( wk, SEARCH_DATAID_ITEM,    0 );
  _set_search_data( wk, SEARCH_DATAID_SEIKAKU, 0 );
  _set_search_data( wk, SEARCH_DATAID_TOKUSEI, 0 );
  _set_search_data( wk, SEARCH_DATAID_SEX,     0 );
  _set_search_data( wk, SEARCH_DATAID_MARK,    0 );

}

// キャンセルチェック
static BOOL CheckCancelButton( BOX_SEARCH_MAIN_WORK * wk )
{
	if( TOUCHBAR_GetTouch( wk->touchbar ) == TOUCHBAR_ICON_RETURN ){
		return TRUE;
	}
/*
	if( TOUCHBAR_IsDecide( wk->touchbar ) == TRUE ){
		return TRUE;
	}
*/
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		return TRUE;
	}
	return FALSE;
}



static const GFL_UI_TP_HITTBL CheckBoxHitTbl[] =
{
  {   0,  23, 112, 135 },
  {  24,  47, 112, 135 },
  {  48,  71, 112, 135 },
  {  72,  95, 112, 135 },
  {  96, 119, 112, 135 },
  { 120, 143, 112, 135 },
  { 144, 167, 112, 135 },
  { GFL_UI_TP_HIT_END, 0, 0, 0 }
};

static int HitCheckListBox( BOX_SEARCH_MAIN_WORK * wk )
{
	int	ret = GFL_UI_TP_HitTrg( CheckBoxHitTbl );

	if( ret >= 0 && ret < wk->flist_pos_max ){
		if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY ){
			GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
		}
	}

	return ret;
}
