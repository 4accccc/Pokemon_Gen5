//=============================================================================
/**
 *
 *  @file   un_select.c
 *  @brief  国連 フロア選択
 *  @author genya hosaka
 *  @data   2010.02.05
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/bmp_winframe.h" // for BmpWinFrame_
#include "system/brightness.h" // for

// 簡易会話表示システム
#include "system/pms_draw.h"

#include "gamesystem/msgspeed.h"  // for MSGSPEED_GetWait

// 文字列関連
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/printsys.h"
#include "font/font.naix"

//INFOWIN
#include "infowin/infowin.h"

//描画設定
#include "un_select_graphic.h"

//簡易CLWK読み込み＆開放ユーティリティー
#include "ui/ui_easy_clwk.h"

#include "ui/print_tool.h" // for 

//タッチバー
#include "ui/touchbar.h"

// BGフレームリスト
#include "ui/frame_list.h"

// シーンコントローラー
#include "ui/ui_scene.h"

//タスクメニュー
#include "app/app_taskmenu.h"

//アプリ共通素材
#include "app/app_menu_common.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "app/un_select.h"

#include "message.naix"
#include "un_select_gra.naix" // アーカイブ

#include "savedata/wifihistory.h" // for WIFIHISTORY_CheckCountryBit, WIFI_COUNTRY_MAX

// サウンド
#include "sound/pm_sndsys.h"

#include "msg/msg_wifi_place_msg_UN.h"  // GMM

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================
#define UN_SELECT_TOUCHBAR

FS_EXTERN_OVERLAY(ui_common);

//=============================================================================
/**
 *                定数定義
 */
//=============================================================================

//--------------------------------------------------------------
/// フロアテーブル
//==============================================================
static const int g_FloorTable[]={
  country001, //アフガニスタン
  country093, //アイスランド
  country098, //アイルランド
  country014, //アゼルバイジャン
  country220, //アメリカ
  country004, //アメリカりょうサモア
  country221, //アメリカりょうバージンしょとう
  country218, //アラブしゅちょうこく
  country003, //アルジェリア
  country009, //アルゼンチン
  country011, //アルバ
  country002, //アルバニア
  country010, //アルメニア
  country007, //アンギラ
  country006, //アンゴラ
  country008, //アンティグア・バーブーダ
  country005, //アンドラ
  country231, //イエメン
  country219, //イギリス
  country029, //イギリスりょうバージンしょとう
  country100, //イスラエル
  country101, //イタリア
  country097, //イラク
  country096, //イラン
  country094, //インド
  country095, //インドネシア
  country216, //ウガンダ
  country217, //ウクライナ
  country223, //ウズベキスタン
  country222, //ウルグアイ
  country060, //エクアドル
  country061, //エジプト
  country065, //エストニア
  country066, //エチオピア
  country064, //エリトリア
  country062, //エルサルバドル
  country012, //オーストラリア
  country013, //オーストリア
  country157, //オマーン
  country146, //オランダ
  country147, //オランダりょうアンティル
  country078, //ガーナ
  country037, //カーボベルデ
  country088, //ガイアナ
  country106, //カザフスタン
  country105, //カシミール
  country169, //カタール
  country036, //カナダ
  country074, //ガボン
  country035, //カメルーン
  country110, //かんこく
  country075, //ガンビア
  country034, //カンボジア
  country109, //きたちょうせん
  country155, //きたマリアナしょとう
  country086, //ギニア
  country087, //ギニアビサウ
  country054, //キプロス
  country053, //キューバ
  country080, //ギリシャ
  country108, //キリバス
  country112, //キルギス
  country085, //グアテマラ
  country083, //グアドループとう
  country084, //グアム
  country111, //クウェート
  country049, //クックしょとう
  country081, //グリーンランド
  country044, //クリスマスとう
  country076, //グルジア
  country082, //グレナダ
  country052, //クロアチア
  country038, //ケイマンしょとう
  country107, //ケニア
  country051, //コートジボワール
  country050, //コスタリカ
  country046, //コモロ
  country045, //コロンビア
  country048, //コンゴきょうわこく
  country047, //コンゴみんしゅきょうわこく
  country182, //サウジアラビア
  country179, //サモア
  country181, //サントメ・プリンシペ
  country232, //ザンビア
  country177, //サンピエール・ミクロン
  country180, //サンマリノ
  country186, //シエラレオネ
  country057, //ジブチ
  country079, //ジブラルタル
  country102, //ジャマイカ
  country201, //シリア
  country187, //シンガポール
  country233, //ジンバブエ
  country200, //スイス
  country199, //スウェーデン
  country195, //スーダン
  country197, //スバールバル・ヤンマンエンとう
  country193, //スペイン
  country196, //スリナム
  country194, //スリランカ
  country188, //スロバキア
  country189, //スロベニア
  country198, //スワジランド
  country185, //セーシェル
  country063, //せきどうギニア
  country183, //セネガル
  country184, //セルビア・モンテネグロ
  country175, //セントクリストファー・ネービス
  country178, //セントビンセント・グレナディーン
  country174, //セントヘレナとう
  country176, //セントルシア
  country191, //ソマリア
  country190, //ソロモンしょとう
  country214, //タークス・カイコスしょとう
  country205, //タイ
  country202, //たいわん
  country203, //タジキスタン
  country204, //タンザニア
  country055, //チェコ
  country040, //チャド
  country041, //チャネルしょとう
  country039, //ちゅうおうアフリカ
  country043, //ちゅうごく
  country211, //チュニジア
  country042, //チリ
  country215, //ツバル
  country056, //デンマーク
  country077, //ドイツ
  country207, //トーゴ
  country208, //トケラウ
  country058, //ドミニカ
  country059, //ドミニカきょうわこく
  country210, //トリニダード・トバゴ
  country213, //トルクメニスタン
  country212, //トルコ
  country209, //トンガ
  country152, //ナイジェリア
  country144, //ナウル
  country143, //ナミビア
  country153, //ニウエ
  country150, //ニカラグア
  country151, //ニジェール
  country230, //にしサハラ
  country103, //にほん
  country148, //ニューカレドニア
  country149, //ニュージーランド
  country145, //ネパール
  country154, //ノーフォークとう
  country156, //ノルウェー
  country016, //バーレーン
  country089, //ハイチ
  country158, //パキスタン
  country225, //バチカン
  country160, //パナマ
  country224, //バヌアツ
  country015, //バハマ
  country161, //パプアニューギニア
  country023, //バミューダ
  country159, //パラオ
  country162, //パラグアイ
  country018, //バルバドス
  country229, //パレスチナじちく
  country092, //ハンガリー
  country017, //バングラデシュ
  country206, //ひがしティモール
  country165, //ピトケアン
  country069, //フィジーしょとう
  country164, //フィリピン
  country070, //フィンランド
  country024, //ブータン
  country168, //プエルトリコ
  country067, //フェローしょとう
  country068, //フォークランド（マルビナス)しょとう
  country028, //ブラジル
  country071, //フランス
  country072, //フランスりょうギアナ
  country073, //フランスりょうポリネシア
  country031, //ブルガリア
  country032, //ブルキナファソ
  country030, //ブルネイ
  country033, //ブルンジ
  country227, //ベトナム
  country022, //ベナン
  country226, //ベネズエラ
  country019, //ベラルーシ
  country021, //ベリーズ
  country163, //ペルー
  country020, //ベルギー
  country166, //ポーランド
  country026, //ボスニア・ヘルツェゴビナ
  country027, //ボツワナ
  country025, //ボリビア
  country167, //ポルトガル
  country091, //ホンコン
  country090, //ホンジュラス
  country130, //マーシャルしょとう
  country122, //マカオ
  country123, //マケドニアきゅうユーゴスラビア
  country124, //マダガスカル
  country134, //マヨット
  country125, //マラウイ
  country128, //マリ
  country129, //マルタ
  country131, //マルチニーク
  country126, //マレーシア
  country099, //マンとう
  country136, //ミクロネシア
  country192, //みなみアフリカ
  country142, //ミャンマー
  country135, //メキシコ
  country133, //モーリシャス
  country132, //モーリタニア
  country141, //モザンビーク
  country138, //モナコ
  country127, //モルディブ
  country137, //モルドバ
  country140, //モロッコ
  country139, //モンゴル
  country104, //ヨルダン
  country113, //ラオス
  country114, //ラトビア
  country120, //リトアニア
  country118, //リビア
  country119, //リヒテンシュタイン
  country117, //リベリア
  country171, //ルーマニア
  country121, //ルクセンブルク
  country173, //ルワンダ
  country116, //レソト
  country115, //レバノン
  country170, //レユニオン
  country172, //ロシア
  country228, //ワリス・フテュナしょとう
};


//=============================================================================
/**
 *                定数定義
 */
//=============================================================================
enum
{ 
  UN_SELECT_HEAP_SIZE = 0x30000,  ///< ヒープサイズ

  UN_LIST_MAX = WIFI_COUNTRY_MAX, ///< 項目数
};

// パッシブ定数
#define YESNO_MASK_DISP ( MASK_DOUBLE_DISPLAY )
#define YESNO_MASK_PLANE ( PLANEMASK_BG3 | PLANEMASK_OBJ | PLANEMASK_BD )
#define BRIGHT_PASSIVE_SYNC (8)
#define BRIGHT_PASSIVE_VOL (-8)

// フォントカラー
#define FCOL_WP01WN   ( PRINTSYS_LSB_Make(14,15,0) )  // フォントカラー：明日

//両用０１白抜

//--------------------------------------------------------------
/// アプリ終了モード
//==============================================================
typedef enum { 
  END_MODE_DECIDE,
  END_MODE_CANCEL
} END_MODE;

//-------------------------------------
/// フレーム
//=====================================
enum
{ 
  BG_FRAME_TEXT_M = GFL_BG_FRAME0_M,
  BG_FRAME_MENU_M = GFL_BG_FRAME1_M,
  BG_FRAME_LIST_M = GFL_BG_FRAME2_M,
  BG_FRAME_BACK_M = GFL_BG_FRAME3_M,

  BG_FRAME_TEXT_S = GFL_BG_FRAME0_S, 
  BG_FRAME_LIST_S = GFL_BG_FRAME1_S,
  BG_FRAME_BACK_S = GFL_BG_FRAME2_S,
};

//-------------------------------------
/// パレット
//=====================================
enum
{ 
  //メインBG
  PLTID_BG_BACK_M       = 0,
  PLTID_BG_TEXT_M       = 1,
  PLTID_BG_TEXT_WIN_M   = 2,
  PLTID_BG_TASKMENU_M   = 11,
  PLTID_BG_TOUCHBAR_M   = 13,
  PLTID_BG_INFOWIN_M    = 15,
  //サブBG
  PLTID_BG_BACK_S       = 0,

  //メインOBJ
  PLTID_OBJ_TOUCHBAR_M  = 0, // 3本使用
  PLTID_OBJ_TYPEICON_M  = 3, // 3本使用
  PLTID_OBJ_OAM_MAPMODEL_M = 6, // 1本使用
  PLTID_OBJ_POKEICON_M = 7,     // 3本使用
  PLTID_OBJ_POKEITEM_M = 10,    // 1本使用
  PLTID_OBJ_ITEMICON_M = 11,
  PLTID_OBJ_POKE_M = 12,
  PLTID_OBJ_BALLICON_M = 13, // 1本使用
  PLTID_OBJ_TOWNMAP_M = 14,   

  //サブOBJ
  PLTID_OBJ_PMS_DRAW = 0, // 5本使用
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
} UN_SELECT_BG_WORK;

//--------------------------------------------------------------
/// メッセージ管理ワーク
//==============================================================
typedef struct
{ 
  //[IN]
  HEAPID  heap_id;

  //[PRIVATE]
  GFL_TCBLSYS   *tcblsys;     ///< タスクシステム
  GFL_MSGDATA   *msghandle;   ///< メッセージハンドル
  GFL_FONT      *font;        ///< フォント
  PRINT_QUE     *print_que;   ///< プリントキュー
  WORDSET       *wordset;     ///< ワードセット

  STRBUF        *strbuf;
  STRBUF        *exp_strbuf;

  // ストリーム再生
  PRINT_STREAM* print_stream;
  GFL_BMPWIN    *win_talk;

  STRBUF* name[ UN_LIST_MAX ]; ///< 名前バッファ

} UN_SELECT_MSG_CNT_WORK;

//--------------------------------------------------------------
/// メインワーク
//==============================================================
typedef struct 
{
  HEAPID heap_id;

  UN_SELECT_BG_WORK       wk_bg;

  //描画設定
  UN_SELECT_GRAPHIC_WORK  *graphic;

#ifdef UN_SELECT_TOUCHBAR
  //タッチバー
  TOUCHBAR_WORK             *touchbar;
  //以下カスタムボタン使用する場合のサンプルリソース
  u32                       ncg_btn;
  u32                       ncl_btn;
  u32                       nce_btn;
#endif //UN_SELECT_TOUCHBAR
  
  //タスクメニュー
  APP_TASKMENU_RES          *menu_res;
  APP_TASKMENU_WORK         *menu;

  UN_SELECT_PARAM* pwk;

  UI_SCENE_CNT_PTR cnt_scene;

  UN_SELECT_MSG_CNT_WORK* cnt_msg;

  FRAMELIST_WORK* lwk;
  
} UN_SELECT_MAIN_WORK;

//=============================================================================
/**
 *              シーン定義
 */
//=============================================================================

// リスト生成
static BOOL SceneListMake( UI_SCENE_CNT_PTR cnt, void* work );

// フロア選択
static BOOL SceneSelectFloor_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneSelectFloor_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneSelectFloor_End( UI_SCENE_CNT_PTR cnt, void* work );

// 確認画面
static BOOL SceneConfirm_Init( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneConfirm_Main( UI_SCENE_CNT_PTR cnt, void* work );
static BOOL SceneConfirm_End( UI_SCENE_CNT_PTR cnt, void* work );

//--------------------------------------------------------------
/// SceneID
//==============================================================
typedef enum
{ 
  UNS_SCENE_ID_LIST_MAKE = 0, ///< リスト生成
  UNS_SCENE_ID_SELECT_FLOOR,  ///< フロア選択
  UNS_SCENE_ID_CONFIRM,       ///< 確認画面

  UNS_SCENE_ID_MAX,
} UNS_SCENE_ID;

//--------------------------------------------------------------
/// SceneFunc
//==============================================================
static const UI_SCENE_FUNC_SET c_scene_func_tbl[ UNS_SCENE_ID_MAX ] = 
{
  // UNS_SCENE_ID_LIST_MAKE
  {
    NULL,
    NULL,
    SceneListMake,
    NULL,
    NULL,
  },
  // UNS_SCENE_ID_SELECT_FLOOR
  {
    SceneSelectFloor_Init,
    NULL,
    SceneSelectFloor_Main,
    NULL,
    SceneSelectFloor_End,
  },
  // UNS_SCENE_ID_CONFIRM
  {
    SceneConfirm_Init,
    NULL,
    SceneConfirm_Main,
    NULL,
    SceneConfirm_End,
  },
};

//=============================================================================
/**
 *              プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
/// PROC
//=====================================
static GFL_PROC_RESULT UNSelectProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT UNSelectProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT UNSelectProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
//-------------------------------------
/// 汎用処理ユーティリティ
//=====================================
static void UNSelect_BG_LoadResource( UN_SELECT_BG_WORK* wk, HEAPID heap_id );

#ifdef UN_SELECT_TOUCHBAR
//-------------------------------------
/// タッチバー
//=====================================
static TOUCHBAR_WORK * UNSelect_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heap_id );
static void UNSelect_TOUCHBAR_Exit( TOUCHBAR_WORK *touchbar );
static void UNSelect_TOUCHBAR_Main( TOUCHBAR_WORK *touchbar );
#endif //UN_SELECT_TOUCHBAR

//-------------------------------------
/// リストシステムはい、いいえ
//=====================================
static APP_TASKMENU_WORK * UNSelect_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heap_id );
static void UNSelect_TASKMENU_Exit( APP_TASKMENU_WORK *menu );
static void UNSelect_TASKMENU_Main( APP_TASKMENU_WORK *menu );

//-------------------------------------
// パッシブ
//=====================================
static void PASSIVE_Request( void );
static void PASSIVE_Reset( void );

//-------------------------------------
// メッセージ管理モジュール
//=====================================
static UN_SELECT_MSG_CNT_WORK* MSG_CNT_Create( HEAPID heap_id );
static void MSG_CNT_Delete( UN_SELECT_MSG_CNT_WORK* wk );
static void MSG_CNT_Main( UN_SELECT_MSG_CNT_WORK* wk );
static void MSG_CNT_SetPrint( UN_SELECT_MSG_CNT_WORK* wk, int str_id );
static BOOL MSG_CNT_PrintProc( UN_SELECT_MSG_CNT_WORK* wk );
static void MSG_CNT_PrintClear( UN_SELECT_MSG_CNT_WORK* wk );
static GFL_FONT* MSG_CNT_GetFont( UN_SELECT_MSG_CNT_WORK* wk );
static PRINT_QUE* MSG_CNT_GetPrintQue( UN_SELECT_MSG_CNT_WORK* wk );
static GFL_MSGDATA* MSG_CNT_GetMsgData( UN_SELECT_MSG_CNT_WORK* wk );

// PROTOTYPE
static UN_SELECT_MAIN_WORK* app_init( GFL_PROC* proc, UN_SELECT_PARAM* prm );
static void app_end( UN_SELECT_MAIN_WORK* wk, END_MODE end_mode );

static void LIST_Make( UN_SELECT_MAIN_WORK* wk );
static void LIST_Delete( UN_SELECT_MAIN_WORK* wk );

//=============================================================================
/**
 *                外部公開
 */
//=============================================================================
const GFL_PROC_DATA UNSelectProcData = 
{
  UNSelectProc_Init,
  UNSelectProc_Main,
  UNSelectProc_Exit,
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
 *  @param  *pwk          UN_SELECT_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT UNSelectProc_Init( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
  UN_SELECT_MAIN_WORK *wk;
  
  GF_ASSERT( pwk );

  switch( *seq )
  {
  case 0:
    wk = app_init( proc, pwk );
  
    // フェードイン リクエスト
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 1 );

    (*seq)++;
    break;

  case 1:
    if( GFL_FADE_CheckFade() == FALSE )
    {
      return GFL_PROC_RES_FINISH;
    }
    break;

  default : GF_ASSERT(0);
  }

  return GFL_PROC_RES_CONTINUE;
}
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 終了処理
 *
 *  @param  GFL_PROC *procプロセスシステム
 *  @param  *seq          シーケンス
 *  @param  *pwk          UN_SELECT_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT UNSelectProc_Exit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
  UN_SELECT_MAIN_WORK* wk = mywk;
    
  if( GFL_FADE_CheckFade() == TRUE )
  {
     return GFL_PROC_RES_CONTINUE;
  }
    
  // リスト開放
  LIST_Delete( wk ); 

  // シーンコントーラ削除
  UI_SCENE_CNT_Delete( wk->cnt_scene );

  // メッセージ消去
  MSG_CNT_Delete( wk->cnt_msg );

  //TASKMENU リソース破棄
  APP_TASKMENU_RES_Delete( wk->menu_res );  

#ifdef UN_SELECT_TOUCHBAR
  //タッチバー
  UNSelect_TOUCHBAR_Exit( wk->touchbar );
#endif //UN_SELECT_TOUCHBAR

  //描画設定破棄
  UN_SELECT_GRAPHIC_Exit( wk->graphic );

  //PROC用メモリ解放
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( wk->heap_id );

  //オーバーレイ破棄
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common) );

  HOSAKA_Printf(" PROC終了！ \n");

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 主処理
 *
 *  @param  GFL_PROC *procプロセスシステム
 *  @param  *seq          シーケンス
 *  @param  *pwk          UN_SELECT_PARAM
 *  @param  *mywk         PROCワーク
 *
 *  @retval 終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT UNSelectProc_Main( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
  UN_SELECT_MAIN_WORK* wk = mywk;
  
  // SCENE
  if( UI_SCENE_CNT_Main( wk->cnt_scene ) )
  {
    return GFL_PROC_RES_FINISH;
  }

  // メッセージ主処理
  MSG_CNT_Main( wk->cnt_msg );

  //2D描画
  UN_SELECT_GRAPHIC_2D_Draw( wk->graphic );

  //3D描画
  UN_SELECT_GRAPHIC_3D_StartDraw( wk->graphic );

  UN_SELECT_GRAPHIC_3D_EndDraw( wk->graphic );

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
 *  @param  UN_SELECT_BG_WORK* wk BG管理ワーク
 *  @param  heap_id  ヒープID 
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void UNSelect_BG_LoadResource( UN_SELECT_BG_WORK* wk, HEAPID heap_id )
{
  ARCHANDLE *handle;

  handle  = GFL_ARC_OpenDataHandle( ARCID_UN_SELECT_GRA, heap_id );

  // 上下画面ＢＧパレット転送
  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_un_select_gra_kokuren_bg_NCLR, PALTYPE_MAIN_BG, PLTID_BG_BACK_M, 0, heap_id );
  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_un_select_gra_kokuren_bg_NCLR, PALTYPE_SUB_BG, PLTID_BG_BACK_S, 0, heap_id );
  
  //  ----- 下画面 -----
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_un_select_gra_kokuren_bg_NCGR,
            BG_FRAME_BACK_S, 0, 0, 0, heap_id );
  GFL_ARCHDL_UTIL_TransVramScreen(  handle, NARC_un_select_gra_kokuren_bgu_NSCR,
            BG_FRAME_BACK_S, 0, 0, 0, heap_id );  
  
  //  ----- 上画面 -----
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_un_select_gra_kokuren_bg_NCGR,
            BG_FRAME_BACK_M, 0, 0, 0, heap_id );
  GFL_ARCHDL_UTIL_TransVramScreen(  handle, NARC_un_select_gra_kokuren_bgd_NSCR,
            BG_FRAME_BACK_M, 0, 0, 0, heap_id );    
  
  // ----- リストバー -----
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_un_select_gra_kokuren_bg_listframe_NCGR,
            BG_FRAME_LIST_M, 0, 0, 0, heap_id );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_un_select_gra_kokuren_bg_listframe_NCGR,
            BG_FRAME_LIST_S, 0, 0, 0, heap_id );

  GFL_ARC_CloseDataHandle( handle );
}


#ifdef UN_SELECT_TOUCHBAR
//=============================================================================
/**
 *  TOUCHBAR
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBAR初期化
 *
 *  @param  GFL_CLUNIT *clunit  CLUNIT
 *  @param  heap_id             ヒープID
 *
 *  @return TOUCHBAR_WORK
 */
//-----------------------------------------------------------------------------
static TOUCHBAR_WORK * UNSelect_TOUCHBAR_Init( GFL_CLUNIT *clunit, HEAPID heap_id )
{ 
  TOUCHBAR_WORK* touchbar;

  //アイコンの設定
  //数分作る
  TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]  =
  { 
    { 
      TOUCHBAR_ICON_RETURN,
      { TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
    },
    { 
      TOUCHBAR_ICON_CUR_L,
      { TOUCHBAR_ICON_X_00, TOUCHBAR_ICON_Y },
    },
    { 
      TOUCHBAR_ICON_CUR_R,
      { TOUCHBAR_ICON_X_02, TOUCHBAR_ICON_Y },
    },
  };

  //設定構造体
  //さきほどの窓情報＋リソース情報をいれる
  TOUCHBAR_SETUP  touchbar_setup;
  GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

  touchbar_setup.p_item   = touchbar_icon_tbl;        //上の窓情報
  touchbar_setup.item_num = NELEMS(touchbar_icon_tbl);//いくつ窓があるか
  touchbar_setup.p_unit   = clunit;                   //OBJ読み込みのためのCLUNIT
  touchbar_setup.bar_frm  = BG_FRAME_MENU_M;            //BG読み込みのためのBG面
  touchbar_setup.bg_plt   = PLTID_BG_TOUCHBAR_M;      //BGﾊﾟﾚｯﾄ
  touchbar_setup.obj_plt  = PLTID_OBJ_TOUCHBAR_M;     //OBJﾊﾟﾚｯﾄ
  touchbar_setup.mapping  = APP_COMMON_MAPPING_128K;  //マッピングモード

  touchbar =  TOUCHBAR_Init( &touchbar_setup, heap_id );

  TOUCHBAR_SetBGPriorityAll( touchbar, 1 ); // BGプライオリティを設定

  return touchbar;
}
//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBAR破棄
 *
 *  @param  TOUCHBAR_WORK *touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void UNSelect_TOUCHBAR_Exit( TOUCHBAR_WORK *touchbar )
{ 
  TOUCHBAR_Exit( touchbar );
}

//----------------------------------------------------------------------------
/**
 *  @brief  TOUCHBARメイン処理
 *
 *  @param  TOUCHBAR_WORK *touchbar タッチバー
 */
//-----------------------------------------------------------------------------
static void UNSelect_TOUCHBAR_Main( TOUCHBAR_WORK *touchbar )
{ 
  TOUCHBAR_Main( touchbar );
}
#endif //UN_SELECT_TOUCHBAR

//----------------------------------------------------------------------------
/**
 *  @brief  TASKMENUの初期化
 *
 *  @param  menu_res  リソース
 */
//-----------------------------------------------------------------------------
static APP_TASKMENU_WORK * UNSelect_TASKMENU_Init( APP_TASKMENU_RES *menu_res, GFL_MSGDATA *msg, HEAPID heap_id )
{ 
  int i;
  APP_TASKMENU_INITWORK init;
  APP_TASKMENU_ITEMWORK item[2];
  APP_TASKMENU_WORK     *menu;  

  //窓の設定
  for( i = 0; i < NELEMS(item); i++ )
  { 
    item[i].str       = GFL_MSG_CreateString( msg, un_reception_msg_02 + i ); //文字列
    item[i].msgColor  = APP_TASKMENU_ITEM_MSGCOLOR; //文字色
    item[i].type      = APP_TASKMENU_WIN_TYPE_NORMAL; //窓の種類
  }

  //初期化
  init.heapId   = heap_id;
  init.itemNum  = NELEMS(item);
  init.itemWork = item;
  init.posType  = ATPT_RIGHT_DOWN;
  init.charPosX = 32;
  init.charPosY = 18;
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
static void UNSelect_TASKMENU_Exit( APP_TASKMENU_WORK *menu )
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
static void UNSelect_TASKMENU_Main( APP_TASKMENU_WORK *menu )
{ 
  APP_TASKMENU_UpdateMenu( menu );
}


//-----------------------------------------------------------------------------
/**
 *  @brief  パッシブリクエスト
 *
 *  @param  void 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void PASSIVE_Request( void )
{
  BrightnessChgReset( YESNO_MASK_DISP );
  ChangeBrightnessRequest( BRIGHT_PASSIVE_SYNC, BRIGHT_PASSIVE_VOL, 0, YESNO_MASK_PLANE, YESNO_MASK_DISP );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  パッシブリセット
 *
 *  @param  void 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void PASSIVE_Reset( void )
{
  BrightnessChgReset( YESNO_MASK_DISP ); // パッシブ解除
  ChangeBrightnessRequest( BRIGHT_PASSIVE_SYNC, 0, BRIGHT_PASSIVE_VOL, YESNO_MASK_PLANE, YESNO_MASK_DISP );
}


//=============================================================================
//
//
// メッセージ管理クラス
//
//
//=============================================================================
enum
{ 
  CGX_BMPWIN_FRAME_POS = 1,
  STRBUF_SIZE = 1600,
};

#define MSG_CNT_SKIP_BTN (PAD_BUTTON_A|PAD_BUTTON_B)

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス 生成
 *
 *  @param  HEAPID heap_id 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static UN_SELECT_MSG_CNT_WORK* MSG_CNT_Create( HEAPID heap_id )
{
  UN_SELECT_MSG_CNT_WORK* wk;

  // メモリ アロック
  wk = GFL_HEAP_AllocClearMemory( heap_id, sizeof( UN_SELECT_MSG_CNT_WORK ) );

  // メンバ初期化
  wk->heap_id = heap_id;
  
  // メッセージ用フォント転送
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 0x20*PLTID_BG_TEXT_M, 0x20, heap_id );
  
  // フレームウィンドウ用のキャラを用意
  BmpWinFrame_GraphicSet( BG_FRAME_TEXT_M, CGX_BMPWIN_FRAME_POS, PLTID_BG_TEXT_WIN_M, MENU_TYPE_FIELD, heap_id );
  
  // ワードセット生成
  wk->wordset = WORDSET_Create( heap_id );

  //メッセージ
  wk->msghandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
      NARC_message_wifi_place_msg_UN_dat, heap_id );

  //PRINT_QUE作成
  wk->print_que   = PRINTSYS_QUE_Create( heap_id );

  //フォント作成
  wk->font      = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
                        GFL_FONT_LOADTYPE_FILE, FALSE, heap_id );

  // STRBU 生成
  wk->strbuf      = GFL_STR_CreateBuffer( STRBUF_SIZE, heap_id );
  wk->exp_strbuf  = GFL_STR_CreateBuffer( STRBUF_SIZE, heap_id );

  wk->tcblsys = GFL_TCBL_Init( heap_id, heap_id, 1, 0 );
  
  // ウィンドウ生成
  wk->win_talk = GFL_BMPWIN_Create( BG_FRAME_TEXT_M, 1, 19, 30, 4, PLTID_BG_TEXT_M, GFL_BMP_CHRAREA_GET_B );
  
  return wk;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス 破棄
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void MSG_CNT_Delete( UN_SELECT_MSG_CNT_WORK* wk )
{
  // ストリーム破棄
  if( wk->print_stream )
  {
    PRINTSYS_PrintStreamDelete( wk->print_stream );
    wk->print_stream = NULL;
  }

  // BMPWIN 破棄
  GFL_BMPWIN_Delete( wk->win_talk );

  // STRBUF 破棄
  GFL_STR_DeleteBuffer( wk->strbuf );
  GFL_STR_DeleteBuffer( wk->exp_strbuf );
  //メッセージ破棄
  GFL_MSG_Delete( wk->msghandle );
  //PRINT_QUE
  PRINTSYS_QUE_Delete( wk->print_que );
  //FONT
  GFL_FONT_Delete( wk->font );
  // TCBL
  GFL_TCBL_Exit( wk->tcblsys );
  // ワードセット 破棄
  WORDSET_Delete( wk->wordset );

  // メモリ破棄
  GFL_HEAP_FreeMemory( wk );
}


//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス 主処理
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void MSG_CNT_Main( UN_SELECT_MSG_CNT_WORK* wk )
{
  PRINTSYS_QUE_Main( wk->print_que );
  
  GFL_TCBL_Main( wk->tcblsys );
}


// 国名が２行になる場合の文字数の境界（これ以上は改行が必要・これ以下は改行はいっていないはず）
#define MULTI_LINE_WORD_NUM ( 10 )

//-----------------------------------------------------------------------------
/**
 *  @brief  リスト項目描画(国名描画)
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk
 *  @param  util 
 *  @pawram itemNum
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void MSG_CNT_DrawListElem( UN_SELECT_MSG_CNT_WORK* wk, PRINT_UTIL* util, PRINT_QUE *que, GFL_FONT *font, u32 itemNum )
{
  int ypos = 12;
  int idx;
  int floor;

  GF_ASSERT(wk);

  // フロア表示
  idx = itemNum;
  floor = UN_LIST_MAX - itemNum + 1; //フロアは2Ｆから
  GFL_MSG_GetString( wk->msghandle, un_reception_msg_04, wk->strbuf );
  WORDSET_RegisterNumber( wk->wordset, 2, floor, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_ExpandStr( wk->wordset, wk->exp_strbuf, wk->strbuf );

  PRINTTOOL_PrintColor( util, que, 0, 12, wk->exp_strbuf, font, FCOL_WP01WN, PRINTTOOL_MODE_LEFT );

  // 国名 表示(１行・２行対応）
  if(GFL_STR_GetBufferLength(wk->name[idx]) > MULTI_LINE_WORD_NUM){
    ypos = 4;
  }
  PRINTTOOL_PrintColor( util, que, 32, ypos, wk->name[idx], font, FCOL_WP01WN, PRINTTOOL_MODE_LEFT );

  return;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス メッセージプリント リクエスト
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk
 *  @param  str_id 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void MSG_CNT_SetPrint( UN_SELECT_MSG_CNT_WORK* wk, int str_id )
{
  const u8 clear_color = 15;
  GFL_BMPWIN* win;
  int msgspeed;

  GF_ASSERT(wk);
  
  msgspeed  = MSGSPEED_GetWait();
  win       = wk->win_talk;
  
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(win), clear_color);
  GFL_FONTSYS_SetColor(1, 2, clear_color);

  GFL_MSG_GetString( wk->msghandle, str_id, wk->strbuf );

#ifdef PM_DEBUG
  GFL_STR_CheckBufferValid( wk->strbuf ); ///< 破損チェック
#endif
  
  WORDSET_ExpandStr( wk->wordset, wk->exp_strbuf, wk->strbuf );

#ifdef PM_DEBUG
  GFL_STR_CheckBufferValid( wk->strbuf ); ///< 破損チェック
#endif

  wk->print_stream = PRINTSYS_PrintStream( win, 4, 0, wk->exp_strbuf, wk->font, msgspeed,
                                           wk->tcblsys, 0xffff, wk->heap_id, clear_color );
  
  // フレーム生成
  BmpWinFrame_Write( wk->win_talk, WINDOW_TRANS_OFF, CGX_BMPWIN_FRAME_POS, GFL_BMPWIN_GetPalette(wk->win_talk) );

  // 転送
  { 
    GFL_BMPWIN_TransVramCharacter( win );
    GFL_BMPWIN_MakeScreen( win );

    GFL_BG_LoadScreenV_Req( GFL_BMPWIN_GetFrame(win) );
  }

}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス メッセージ処理
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval TRUE:プリント終了
 */
//-----------------------------------------------------------------------------
static BOOL MSG_CNT_PrintProc( UN_SELECT_MSG_CNT_WORK* wk )
{
  PRINTSTREAM_STATE state;

  GF_ASSERT(wk);

//  HOSAKA_Printf("print state= %d \n", state );

  if( wk->print_stream )
  {
    state = PRINTSYS_PrintStreamGetState( wk->print_stream );

    switch( state )
    {
    case PRINTSTREAM_STATE_DONE : // 終了
      PRINTSYS_PrintStreamDelete( wk->print_stream );
      wk->print_stream = NULL;
      return TRUE;

    case PRINTSTREAM_STATE_PAUSE : // 一時停止中
      // キー入力待ち
      if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_DECIDE || (  GFL_UI_KEY_GetTrg() == PAD_BUTTON_CANCEL ) || GFL_UI_TP_GetTrg() )
      {
        PRINTSYS_PrintStreamReleasePause( wk->print_stream );
        GFL_SOUND_PlaySE( SEQ_SE_DECIDE1 );
      }
      break;

    case PRINTSTREAM_STATE_RUNNING :  // 実行中
      // メッセージスキップ
      if( (GFL_UI_KEY_GetCont() & MSG_CNT_SKIP_BTN) || GFL_UI_TP_GetCont() )
      {
        PRINTSYS_PrintStreamShortWait( wk->print_stream, 0 );
      }
      break;

    default : GF_ASSERT(0);
    }

    return FALSE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージフレームをクリア
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void MSG_CNT_PrintClear( UN_SELECT_MSG_CNT_WORK* wk )
{
  BmpWinFrame_Clear( wk->win_talk, WINDOW_TRANS_ON_V );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス フォント取得
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static GFL_FONT* MSG_CNT_GetFont( UN_SELECT_MSG_CNT_WORK* wk )
{
  GF_ASSERT(wk);

  return wk->font;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス プリントキュー取得
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static PRINT_QUE* MSG_CNT_GetPrintQue( UN_SELECT_MSG_CNT_WORK* wk )
{
  GF_ASSERT(wk);

  return wk->print_que;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  メッセージ管理クラス メッセージハンドルを取得
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static GFL_MSGDATA* MSG_CNT_GetMsgData( UN_SELECT_MSG_CNT_WORK* wk )
{
  GF_ASSERT(wk);

  return wk->msghandle;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ワードセット
 *
 *  @param  UN_SELECT_MSG_CNT_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static WORDSET* MSG_CNT_GetWordset( UN_SELECT_MSG_CNT_WORK* wk )
{
  GF_ASSERT(wk);

  return wk->wordset;
}

//=============================================================================
//
//
// リスト管理クラス
//
//
//=============================================================================
static void ListCallBack_Draw( void * work, u32 itemNum, PRINT_UTIL * util, s16 py, BOOL disp );
static void ListCallBack_Move( void * work, u32 listPos, BOOL flg );
static void ListCallBack_Scroll( void * work, s8 mv );

//--------------------------------------------------------------
/// 
//==============================================================
static const FRAMELIST_CALLBACK FRMListCallBack = {
  ListCallBack_Draw,    // 描画処理
  ListCallBack_Move,    // 移動処理
  ListCallBack_Scroll,  // スクロール
};

//--------------------------------------------------------------
/// 
//==============================================================
static const FRAMELIST_TOUCH_DATA TouchHitTbl[] =
{
  { { 0,    8*5-1,  8*12, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },   // 00: 
  { { 8*5,  8*10-1, 8*12, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },   // 01: 
  { { 8*10, 8*15-1, 8*12, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },   // 02: 
  { { 8*15, 8*20-1, 8*12, 231 }, FRAMELIST_TOUCH_PARAM_SLIDE },   // 03: 

  { { 8, 159, 240, 255 }, FRAMELIST_TOUCH_PARAM_RAIL },         // 07: レール

  { { 168, 191, 16, 16+3*8-1 }, FRAMELIST_TOUCH_PARAM_PAGE_UP },      // 08: 左
  { { 168, 191, 80, 80+3*8-1 }, FRAMELIST_TOUCH_PARAM_PAGE_DOWN },    // 09: 右

  { { GFL_UI_TP_HIT_END, 0, 0, 0 }, 0 },
};

//-----------------------------------------------------------------------------
/**
 *  @brief  リスト描画
 *
 *  @param  void * work
 *  @param  itemNum
 *  @param  * util
 *  @param  py
 *  @param  disp 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void ListCallBack_Draw( void * work, u32 itemNum, PRINT_UTIL * util, s16 py, BOOL disp )
{ 
  UN_SELECT_MAIN_WORK* wk = work;
  u32 prm;
  
  prm = FRAMELIST_GetItemParam( wk->lwk, itemNum );

  {
    PRINT_QUE * que;
    GFL_FONT* font;
    WORDSET* wset;
    
    que  = FRAMELIST_GetPrintQue( wk->lwk );
    font = MSG_CNT_GetFont( wk->cnt_msg );

    // 各階フロア名描画（国名）
    MSG_CNT_DrawListElem( wk->cnt_msg, util, que, font, itemNum );
  }

  HOSAKA_Printf("draw!\n");
}

//-----------------------------------------------------------------------------
/**
 *  @brief
 *
 *  @param  void * work
 *  @param  listPos
 *  @param  flg 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void ListCallBack_Move( void * work, u32 listPos, BOOL flg )
{ 
  UN_SELECT_MAIN_WORK* wk = work;
  HOSAKA_Printf("move!\n");
}

//-----------------------------------------------------------------------------
/**
 *  @brief
 *
 *  @param  void * work
 *  @param  mv 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void ListCallBack_Scroll( void * work, s8 mv )
{
  UN_SELECT_MAIN_WORK* wk = work;
  HOSAKA_Printf("scroll!\n");
}

//-----------------------------------------------------------------------------
/**
 *  @brief  リスト生成
 *
 *  @param  UN_SELECT_MAIN_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void LIST_Make( UN_SELECT_MAIN_WORK* wk )
{
  ARCHANDLE* ah;

  static FRAMELIST_HEADER header = {
    BG_FRAME_LIST_M,
    BG_FRAME_LIST_S,

    11,         // 項目フレーム表示開始Ｘ座標
    0,          // 項目フレーム表示開始Ｙ座標
    18,         // 項目フレーム表示Ｘサイズ
    5,          // 項目フレーム表示Ｙサイズ

    1,              // フレーム内に表示するBMPWINの表示Ｘ座標
    0,              // フレーム内に表示するBMPWINの表示Ｙ座標
    15,             // フレーム内に表示するBMPWINの表示Ｘサイズ
    5,              // フレーム内に表示するBMPWINの表示Ｙサイズ
    1,              // フレーム内に表示するBMPWINのパレット

    { 40, 20, 10, 8, 5, 4 },    // スクロール速度 [0] = 最速 ※itemSizYを割り切れる値であること！

    3,              // 選択項目のパレット

    8,              // スクロールバーのＹサイズ

    UN_LIST_MAX,    // 項目登録数
    2,              // 背景登録数

    0,              // 初期位置
    4,              // カーソル移動範囲
    UN_LIST_MAX-5,  // 初期スクロール値 //@TODO

    TouchHitTbl,      // タッチデータ

    &FRMListCallBack, // コールバック関数
    NULL,
  };

  header.cbWork = wk;

  wk->lwk = FRAMELIST_Create( &header, wk->heap_id );

  // アーカイブ オープン
  ah = GFL_ARC_OpenDataHandle( ARCID_UN_SELECT_GRA, wk->heap_id );

  // 項目背景設定
  FRAMELIST_LoadFrameGraphicAH( wk->lwk, ah, NARC_un_select_gra_kokuren_bg_listframe_NSCR, FALSE, 0 );
  FRAMELIST_LoadFrameGraphicAH( wk->lwk, ah, NARC_un_select_gra_kokuren_bg_listframe2_NSCR, FALSE, 1 );

  // 点滅アニメパレット設定
  FRAMELIST_LoadBlinkPalette( wk->lwk, ah, NARC_un_select_gra_kokuren_bg_NCLR, 2, 3 );

//  FRAMELIST_ChangePosPalette(

  // アーカイブ クローズ
  GFL_ARC_CloseDataHandle( ah );
        
  // リスト項目生成
  {
    int i;
    GFL_MSGDATA* mman;
    
    mman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
        NARC_message_wifi_place_msg_UN_dat, wk->heap_id );

    for( i=0; i<UN_LIST_MAX; i++)
    {
      int type;
      int idx;
      idx = UN_LIST_MAX - i - 1;

      //@TODO 条件、項目の存在
      type = GFUser_GetPublicRand0( 2 );

      // パラメータは純粋に順列ID
      FRAMELIST_AddItem( wk->lwk, type, idx );

      // 項目あり
      if( type == 0 )
      {
        // 項目用文字列 取得
        wk->cnt_msg->name[i] = GFL_MSG_CreateString( mman, idx );
      }
      // 項目なし
      else
      {
        // 項目用文字列 取得
        wk->cnt_msg->name[i] = GFL_MSG_CreateString( mman, un_reception_msg_05 );
      }
    }
      
    // メッセージ生成
//    MSG_CNT_CreateNameBuffer( wk->cnt_msg, i, buf );
    
    GFL_MSG_Delete( mman );
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  リスト開放
 *
 *  @param  UN_SELECT_MAIN_WORK* wk 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void LIST_Delete( UN_SELECT_MAIN_WORK* wk )
{
  int i;
  
  for( i=0; i<UN_LIST_MAX; i++)
  {
    // 項目用文字列 開放
    GFL_STR_DeleteBuffer( wk->cnt_msg->name[i] );
  }
  
  // フレームリスト 開放
  FRAMELIST_Exit( wk->lwk );
}
  
//-----------------------------------------------------------------------------
/**
 *  @brief  PROC 初期化
 *
 *  @param  GFL_PROC* proc
 *  @param  prm 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static UN_SELECT_MAIN_WORK* app_init( GFL_PROC* proc, UN_SELECT_PARAM* prm )
{
  UN_SELECT_MAIN_WORK* wk;

  //オーバーレイ読み込み
  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
  
  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_UN_SELECT, UN_SELECT_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(UN_SELECT_MAIN_WORK), HEAPID_UN_SELECT );
  GFL_STD_MemClear( wk, sizeof(UN_SELECT_MAIN_WORK) );

  // 初期化
  wk->heap_id = HEAPID_UN_SELECT;
  wk->pwk = prm;
  
  //描画設定初期化
  wk->graphic = UN_SELECT_GRAPHIC_Init( GX_DISP_SELECT_SUB_MAIN, wk->heap_id );

  // メッセージ生成
  wk->cnt_msg = MSG_CNT_Create( wk->heap_id );
  
  // シーンコントローラ作成
  wk->cnt_scene = UI_SCENE_CNT_Create( 
      wk->heap_id, c_scene_func_tbl, UNS_SCENE_ID_MAX, 
      UNS_SCENE_ID_LIST_MAKE, wk );

  //BGリソース読み込み
  UNSelect_BG_LoadResource( &wk->wk_bg, wk->heap_id );

#ifdef UN_SELECT_TOUCHBAR
  //タッチバーの初期化
  { 
    GFL_CLUNIT  *clunit = UN_SELECT_GRAPHIC_GetClunit( wk->graphic );
    wk->touchbar  = UNSelect_TOUCHBAR_Init( clunit, wk->heap_id );
  }
#endif //UN_SELECT_TOUCHBAR

  //TASKMENUリソース読み込み＆初期化
  wk->menu_res  = APP_TASKMENU_RES_Create( BG_FRAME_MENU_M, PLTID_BG_TASKMENU_M,
      MSG_CNT_GetFont( wk->cnt_msg ),
      MSG_CNT_GetPrintQue( wk->cnt_msg ),
      wk->heap_id );
  
  // リスト生成
  LIST_Make( wk );

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  アプリケーション終了処理
 *
 *  @param  UN_SELECT_MAIN_WORK* wk
 *  @param  end_mode 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void app_end( UN_SELECT_MAIN_WORK* wk, END_MODE end_mode )
{
  switch( end_mode )
  {
  case END_MODE_DECIDE: 
      // データ吐き出し
      //@TODO 選択フロア
    break;
  case END_MODE_CANCEL:
      // データ吐き出し
      //@TODO キャンセル
    break;
  }
  
  // フェードアウト リクエスト
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 1 );
}


//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE リスト生成 主処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneListMake( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;
  u8 seq = UI_SCENE_CNT_GetSubSeq( cnt );

  switch( seq )
  {
  case 0:
    // リスト初期化
    if( FRAMELIST_Init( wk->lwk ) == FALSE )
    {
      UI_SCENE_CNT_IncSubSeq( cnt );
    }
    break;
  case 1:
    // フロア選択へ
    UI_SCENE_CNT_SetNextScene( cnt, UNS_SCENE_ID_SELECT_FLOOR );
    HOSAKA_Printf("リスト生成完了\n");
    return TRUE;
  default : GF_ASSERT(0);
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE フロア選択 初期化処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneSelectFloor_Init( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE フロア選択 主処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneSelectFloor_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  // フロア選択処理
  // @TODO タッチ及びカーソルで選択
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    UI_SCENE_CNT_SetNextScene( cnt, UNS_SCENE_ID_CONFIRM );
    return TRUE;
  }

  FRAMELIST_Main( wk->lwk );

#ifdef UN_SELECT_TOUCHBAR
  //タッチバーメイン処理
  UNSelect_TOUCHBAR_Main( wk->touchbar );
#endif //UN_SELECT_TOUCHBAR

  switch( TOUCHBAR_GetTrg( wk->touchbar ) )
  {
  case TOUCHBAR_ICON_CUR_L : // ←
    //@TODO
    HOSAKA_Printf("L!\n");
    break;

  case TOUCHBAR_ICON_CUR_R : // →
    //@TODO
    HOSAKA_Printf("R!\n"); 
    break;

  case TOUCHBAR_ICON_RETURN : // リターンボタン
    // 終了
    app_end( wk, END_MODE_CANCEL ); 
    UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
    return TRUE;

  case TOUCHBAR_SELECT_NONE : // 選択なし
    break;

  default : GF_ASSERT(0);
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE フロア選択 後処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneSelectFloor_End( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE 確認画面 初期化処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneConfirm_Init( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;
  u8 seq = UI_SCENE_CNT_GetSubSeq( cnt );

  switch(seq)
  {
  case 0:
    // パッシブ状態に遷移
    PASSIVE_Request();

#if 0
    TOUCHBAR_SetVisible( wk->touchbar, TOUCHBAR_ICON_CUR_L, FALSE );
    TOUCHBAR_SetVisible( wk->touchbar, TOUCHBAR_ICON_CUR_R, FALSE );
    TOUCHBAR_SetVisible( wk->touchbar, TOUCHBAR_ICON_RETURN, FALSE );
#endif

    MSG_CNT_SetPrint( wk->cnt_msg, un_reception_msg_01 );
    UI_SCENE_CNT_IncSubSeq( cnt );
    break;

  case 1:
    if( MSG_CNT_PrintProc(wk->cnt_msg) )
    {
      UI_SCENE_CNT_IncSubSeq( cnt );
    }
    break;

  case 2:
    // タスクメニュー表示
    wk->menu = UNSelect_TASKMENU_Init( 
        wk->menu_res, 
        MSG_CNT_GetMsgData(wk->cnt_msg), 
        wk->heap_id );

    return TRUE;

  default : GF_ASSERT(0);
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE 確認画面 主処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneConfirm_Main( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  //タスクメニューメイン処理
  UNSelect_TASKMENU_Main( wk->menu );

  if( APP_TASKMENU_IsFinish( wk->menu ) )
  {
    u8 pos = APP_TASKMENU_GetCursorPos( wk->menu );

    switch( pos )
    {
    case 0 :
      // 終了
      app_end( wk, END_MODE_DECIDE );
      UI_SCENE_CNT_SetNextScene( cnt, UI_SCENE_ID_END );
      break;

    case 1 :
      // 選択画面に戻る
      PASSIVE_Reset();
      MSG_CNT_PrintClear( wk->cnt_msg );
      UNSelect_TASKMENU_Exit( wk->menu );
      UI_SCENE_CNT_SetNextScene( cnt, UNS_SCENE_ID_SELECT_FLOOR );
      break;

    default : GF_ASSERT(0);
    }

    return TRUE;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  SCENE 確認画面 後処理
 *
 *  @param  UI_SCENE_CNT_PTR cnt
 *  @param  work 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL SceneConfirm_End( UI_SCENE_CNT_PTR cnt, void* work )
{
  UN_SELECT_MAIN_WORK* wk = work;

  return TRUE;
}

