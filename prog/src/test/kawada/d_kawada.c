//============================================================================================
/**
 * @file    d_kawada.c
 * @brief   川田デバッグ処理
 * @author  Koji Kawada
 * @date    2010.03.03
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"
#include "system/wipe.h"
#include "system/bmp_menulist.h"
#include "gamesystem/game_data.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"
#include "item/item.h"
#include "font/font.naix"

#include "app/box2.h"
#include "app/zukan.h"
#include "app/wifi_note.h"
#include "demo/command_demo.h"
#include "../../battle/app/vs_multi_list.h"

#include "arc_def.h"  //ARCID_MESSAGE
#include "debug_message.naix"


#include "msg/debug/msg_d_kawada.h"

// 図鑑登録
#include "app/zukan_toroku.h"

// トライアルハウス結果
#include "pm_version.h"
#include "app/th_award.h"
#include "savedata/trialhouse_save.h"
#include "../../savedata/trialhouse_save_local.h"

// 地方図鑑賞状
#include "savedata/mystatus.h"
#include "app/chihou_zukan_award.h"

// 全国図鑑賞状
#include "app/zenkoku_zukan_award.h"

// 通信対戦後の録画選択画面
#include "net_app/btl_rec_sel.h"

// 二択簡易会話
// 一択簡易会話
// 定型文簡易会話
#include "app/pms_input.h"

// 三匹選択
#include "demo/psel.h"

// 地下鉄路線図
#include "app/subway_map.h"

// タマゴ孵化デモ
#include "field/zonedata.h"
#include "demo/egg_demo.h"

// 進化デモ
#include "poke_tool/shinka_check.h"
#include "demo/shinka_demo.h"

// 図鑑詳細
#include "../../app/zukan/detail/zukan_detail.h"
#include "../../../../resource/fldmapdata/flagwork/flag_define.h"

// ゲーム内マニュアル
#include "app/manual.h"

// テスト
#include "d_test.h"


// オーバーレイ
FS_EXTERN_OVERLAY(zukan_toroku);
FS_EXTERN_OVERLAY(th_award);
FS_EXTERN_OVERLAY(chihou_zukan_award);
FS_EXTERN_OVERLAY(zenkoku_zukan_award);
FS_EXTERN_OVERLAY(btl_rec_sel);
FS_EXTERN_OVERLAY(pmsinput);
FS_EXTERN_OVERLAY(psel);
FS_EXTERN_OVERLAY(subway_map);
FS_EXTERN_OVERLAY(egg_demo);
FS_EXTERN_OVERLAY(shinka_demo);
FS_EXTERN_OVERLAY(zukan_detail);
FS_EXTERN_OVERLAY(manual);


#ifdef PM_DEBUG
//============================================================================================
//  定数定義
//============================================================================================
#define TOP_MENU_SIZ  ( 20 )

#define POKE_LIST_NUM_G (24)
#define POKE_LIST_NUM  (13)


typedef struct {
  u32 main_seq;
  u32 next_seq;
  HEAPID  heapID;

  GFL_FONT * font;          // 通常フォント
  GFL_MSGDATA * mman;       // メッセージデータマネージャ
  PRINT_QUE * que;          // プリントキュー
  GFL_BMPWIN * win;         // BMPWIN
  PRINT_UTIL  util;

  BMP_MENULIST_DATA * ld;
  BMPMENULIST_WORK * lw;

  GAMEDATA * gamedata;

  GFL_PROCSYS*  local_procsys;

  u32 yoin_count;     // 余韻
  u32 yoin_next_seq;  // 余韻が終わった後のシーケンス

  // 図鑑登録
  ZUKAN_TOROKU_PARAM* zukan_toroku_param;
  POKEMON_PARAM*      pp;

  // トライアルハウス結果
  THSV_WORK*          thsv;
  TH_AWARD_PARAM*     th_award_param;

  // 地方図鑑賞状
  MYSTATUS*                    mystatus;
  CHIHOU_ZUKAN_AWARD_PARAM*    chihou_zukan_award_param;
  // 全国図鑑賞状
  ZENKOKU_ZUKAN_AWARD_PARAM*   zenkoku_zukan_award_param;

  // 通信対戦後の録画選択画面
  BTL_REC_SEL_PARAM*     btl_rec_sel_param;

  // 二択簡易会話
  // 一択簡易会話
  // 定型文簡易会話
  PMSI_PARAM*    pmsi_param;

  // 三匹選択
  u16                evwk;
  PSEL_PARAM*        psel_param;

  // 地下鉄路線図
  SUBWAY_MAP_PARAM*  subway_map_param;

  // タマゴ孵化デモ
  EGG_DEMO_PARAM*    egg_demo_param;

  // 進化デモ
  POKEPARTY*         party;
  SHINKA_DEMO_PARAM* shinka_demo_param;

  // 図鑑詳細
  ZUKAN_DETAIL_PARAM* zukan_detail_param;
  u16                 poke_list[POKE_LIST_NUM];

  // マニュアル
  MANUAL_PARAM*       manual_param;

  // テスト
  D_KAWADA_TEST_SHINKA_PARAM*      d_test_shinka_param;
  D_KAWADA_TEST_EGG_PARAM*         d_test_egg_param;
  D_KAWADA_TEST_ZUKAN_FORM_PARAM*  d_test_zukan_form_param;

}KAWADA_MAIN_WORK;

enum {
  MAIN_SEQ_INIT = 0,
  MAIN_SEQ_MAIN,
  MAIN_SEQ_FADE_MAIN,
  MAIN_SEQ_YOIN,

  // ここから
  MAIN_SEQ_ZUKAN_DETAIL_G_CALL,  // top_menu00
  MAIN_SEQ_ZUKAN_TOROKU_CALL,
  MAIN_SEQ_TH_AWARD_CALL,
  MAIN_SEQ_CHIHOU_ZUKAN_AWARD_CALL,
  MAIN_SEQ_ZENKOKU_ZUKAN_AWARD_CALL,
  MAIN_SEQ_BTL_REC_SEL_CALL,
  MAIN_SEQ_PMS_INPUT_DOUBLE_CALL,
  MAIN_SEQ_PMS_INPUT_SINGLE_CALL,
  MAIN_SEQ_PSEL_CALL,
  MAIN_SEQ_SUBWAY_MAP_CALL,
  MAIN_SEQ_PMS_INPUT_SENTENCE_CALL,
  MAIN_SEQ_EGG_DEMO_CALL,
  MAIN_SEQ_SHINKA_DEMO_CALL,
  MAIN_SEQ_ZUKAN_DETAIL_CALL,
  MAIN_SEQ_MANUAL_CALL,
  MAIN_SEQ_D_TEST_SHINKA_CALL,
  MAIN_SEQ_D_TEST_EGG_CALL,
  MAIN_SEQ_D_TEST_ZUKAN_FORM_CENTER_CALL,
  MAIN_SEQ_D_TEST_ZUKAN_FORM_LEFT_CALL,
  MAIN_SEQ_D_TEST_ZUKAN_FORM_RIGHT_CALL,
  // ここまで
  
  MAIN_SEQ_ZUKAN_DETAIL_G_CALL_RETURN,
  MAIN_SEQ_ZUKAN_TOROKU_CALL_RETURN,
  MAIN_SEQ_TH_AWARD_CALL_RETURN,
  MAIN_SEQ_CHIHOU_ZUKAN_AWARD_CALL_RETURN,
  MAIN_SEQ_ZENKOKU_ZUKAN_AWARD_CALL_RETURN,
  MAIN_SEQ_BTL_REC_SEL_CALL_RETURN,
  MAIN_SEQ_PMS_INPUT_DOUBLE_CALL_RETURN,
  MAIN_SEQ_PMS_INPUT_SINGLE_CALL_RETURN,
  MAIN_SEQ_PSEL_CALL_RETURN,
  MAIN_SEQ_SUBWAY_MAP_CALL_RETURN,
  MAIN_SEQ_PMS_INPUT_SENTENCE_CALL_RETURN,
  MAIN_SEQ_EGG_DEMO_CALL_RETURN,
  MAIN_SEQ_SHINKA_DEMO_CALL_RETURN,
  MAIN_SEQ_ZUKAN_DETAIL_CALL_RETURN,
  MAIN_SEQ_MANUAL_CALL_RETURN,
  MAIN_SEQ_D_TEST_SHINKA_CALL_RETURN,
  MAIN_SEQ_D_TEST_EGG_CALL_RETURN,
  MAIN_SEQ_D_TEST_ZUKAN_FORM_CENTER_CALL_RETURN,
  MAIN_SEQ_D_TEST_ZUKAN_FORM_LEFT_CALL_RETURN,
  MAIN_SEQ_D_TEST_ZUKAN_FORM_RIGHT_CALL_RETURN,

  MAIN_SEQ_END,
};


//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static GFL_PROC_RESULT MainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT MainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT MainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

static void FadeInSet( KAWADA_MAIN_WORK * wk, u32 next );
static void FadeOutSet( KAWADA_MAIN_WORK * wk, u32 next );

static void YoinSet( KAWADA_MAIN_WORK * wk, u32 count, u32 next );

static void BgInit( KAWADA_MAIN_WORK * wk );
static void BgExit(void);

static void TopMenuInit( KAWADA_MAIN_WORK * wk );
static void TopMenuExit( KAWADA_MAIN_WORK * wk );


// 図鑑詳細G
static void ZukanDetailGInit( KAWADA_MAIN_WORK* wk );
static void ZukanDetailGExit( KAWADA_MAIN_WORK* wk );

// 図鑑登録
static void ZukanTorokuInit( KAWADA_MAIN_WORK* wk );
static void ZukanTorokuExit( KAWADA_MAIN_WORK* wk );

// トライアルハウス結果
static void ThAwardInit( KAWADA_MAIN_WORK* wk );
static void ThAwardExit( KAWADA_MAIN_WORK* wk );

// 地方図鑑賞状
static void ChihouZukanAwardInit( KAWADA_MAIN_WORK* wk );
static void ChihouZukanAwardExit( KAWADA_MAIN_WORK* wk );

// 全国図鑑賞状
static void ZenkokuZukanAwardInit( KAWADA_MAIN_WORK* wk );
static void ZenkokuZukanAwardExit( KAWADA_MAIN_WORK* wk );

// 通信対戦後の録画選択画面
static void BtlRecSelInit( KAWADA_MAIN_WORK* wk );
static void BtlRecSelExit( KAWADA_MAIN_WORK* wk );

// 二択簡易会話
static void PmsInputDoubleInit( KAWADA_MAIN_WORK* wk );
static void PmsInputDoubleExit( KAWADA_MAIN_WORK* wk );

// 一択簡易会話
static void PmsInputSingleInit( KAWADA_MAIN_WORK* wk );
static void PmsInputSingleExit( KAWADA_MAIN_WORK* wk );

// 三匹選択
static void PselInit( KAWADA_MAIN_WORK* wk );
static void PselExit( KAWADA_MAIN_WORK* wk );

// 地下鉄路線図
static void SubwayMapInit( KAWADA_MAIN_WORK* wk );
static void SubwayMapExit( KAWADA_MAIN_WORK* wk );

// 定型文簡易会話
static void PmsInputSentenceInit( KAWADA_MAIN_WORK* wk );
static void PmsInputSentenceExit( KAWADA_MAIN_WORK* wk );

// タマゴ孵化デモ
static void EggDemoInit( KAWADA_MAIN_WORK* wk );
static void EggDemoExit( KAWADA_MAIN_WORK* wk );

// 進化デモ
static void ShinkaDemoInit( KAWADA_MAIN_WORK* wk );
static void ShinkaDemoExit( KAWADA_MAIN_WORK* wk );

// 図鑑詳細
static void ZukanDetailInit( KAWADA_MAIN_WORK* wk );
static void ZukanDetailExit( KAWADA_MAIN_WORK* wk );

// ゲーム内マニュアル
static void ManualInit( KAWADA_MAIN_WORK* wk );
static void ManualExit( KAWADA_MAIN_WORK* wk );

// テスト
static void D_TestShinkaInit( KAWADA_MAIN_WORK* wk );
static void D_TestShinkaExit( KAWADA_MAIN_WORK* wk );
static void D_TestEggInit( KAWADA_MAIN_WORK* wk );
static void D_TestEggExit( KAWADA_MAIN_WORK* wk );
static void D_TestZukanFormInit( KAWADA_MAIN_WORK* wk, u8 center_left_right );
static void D_TestZukanFormExit( KAWADA_MAIN_WORK* wk );


//============================================================================================
//  グローバル
//============================================================================================
const GFL_PROC_DATA DebugKawadaMainProcData = {
  MainProcInit,
  MainProcMain,
  MainProcEnd,
};

static const BMPMENULIST_HEADER TopMenuListH = {
  NULL, NULL, NULL, NULL,
  TOP_MENU_SIZ,   // リスト項目数
  6,    // 表示最大項目数
  0,    // ラベル表示Ｘ座標
  12,   // 項目表示Ｘ座標
  0,    // カーソル表示Ｘ座標
  0,    // 表示Ｙ座標
  1,    // 表示文字色
  15,   // 表示背景色
  2,    // 表示文字影色
  0,    // 文字間隔Ｘ
  16,   // 文字間隔Ｙ
  BMPMENULIST_LRKEY_SKIP,   // ページスキップタイプ
  0,    // 文字指定(本来はu8だけどそんなに作らないと思うので)
  0,    // ＢＧカーソル(allow)表示フラグ(0:ON,1:OFF)

  NULL,
  
  12,     // 文字サイズX(ドット
  16,     // 文字サイズY(ドット
  NULL,   // 表示に使用するメッセージバッファ
  NULL,   // 表示に使用するプリントユーティリティ
  NULL,   // 表示に使用するプリントキュー
  NULL,   // 表示に使用するフォントハンドル
};


static GFL_PROC_RESULT MainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  KAWADA_MAIN_WORK * wk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_KAWADA_DEBUG, 0x20000 );

  wk = GFL_PROC_AllocWork( proc, sizeof(KAWADA_MAIN_WORK), HEAPID_KAWADA_DEBUG );

  wk->heapID    = HEAPID_KAWADA_DEBUG;
  wk->gamedata  = GAMEDATA_Create( wk->heapID );

  wk->local_procsys = GFL_PROC_LOCAL_boot( wk->heapID );

#if 0
  // フェードインありでスタート
  FadeInSet( wk, MAIN_SEQ_INIT );
  wk->main_seq  = MAIN_SEQ_FADE_MAIN;
#else
  // フェードインなしでスタート
  wk->main_seq  = MAIN_SEQ_INIT;
#endif

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  KAWADA_MAIN_WORK * wk = mywk;

  GFL_PROC_MAIN_STATUS  local_proc_status   =  GFL_PROC_LOCAL_Main( wk->local_procsys );
  if( local_proc_status == GFL_PROC_MAIN_VALID ) return GFL_PROC_RES_CONTINUE;

  switch( wk->main_seq ){
  case MAIN_SEQ_INIT:
    BgInit( wk );
    TopMenuInit( wk );
    wk->main_seq = MAIN_SEQ_MAIN;
    break;

  case MAIN_SEQ_MAIN:
    {
      u32 ret = BmpMenuList_Main( wk->lw );

      switch( ret ){
      case BMPMENULIST_NULL:
      case BMPMENULIST_LABEL:
        break;

      case BMPMENULIST_CANCEL:    // キャンセル
        TopMenuExit( wk );
        BgExit();
        wk->main_seq = MAIN_SEQ_END;
        break;

      default:
        TopMenuExit( wk );
        BgExit();
        FadeOutSet( wk, ret );
        wk->main_seq = MAIN_SEQ_FADE_MAIN;
      }
    }
    break;

  case MAIN_SEQ_FADE_MAIN:
    if( WIPE_SYS_EndCheck() == TRUE ){
      wk->main_seq = wk->next_seq;
    }
    break;
  
  case MAIN_SEQ_YOIN:
    if( wk->yoin_count == 0 )
    {
      wk->main_seq = wk->yoin_next_seq;
      if( wk->yoin_next_seq == MAIN_SEQ_FADE_MAIN )
      {
        FadeInSet( wk, MAIN_SEQ_INIT );
      }
    }
    else
    {
      wk->yoin_count--;
    }
    break;

  case MAIN_SEQ_END:
    OS_Printf( "kawadaデバッグ処理終了しました\n" );
    return GFL_PROC_RES_FINISH;


  // 図鑑詳細G
  case MAIN_SEQ_ZUKAN_DETAIL_G_CALL:
    ZukanDetailGInit(wk);
    wk->main_seq = MAIN_SEQ_ZUKAN_DETAIL_G_CALL_RETURN;
    break;
  case MAIN_SEQ_ZUKAN_DETAIL_G_CALL_RETURN:
    ZukanDetailGExit(wk);
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 図鑑登録
  case MAIN_SEQ_ZUKAN_TOROKU_CALL:
    ZukanTorokuInit(wk);
    wk->main_seq = MAIN_SEQ_ZUKAN_TOROKU_CALL_RETURN;
    break;
  case MAIN_SEQ_ZUKAN_TOROKU_CALL_RETURN:
    ZukanTorokuExit(wk);
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // トライアルハウス結果
  case MAIN_SEQ_TH_AWARD_CALL:
    ThAwardInit(wk); 
    wk->main_seq = MAIN_SEQ_TH_AWARD_CALL_RETURN;
    break;
  case MAIN_SEQ_TH_AWARD_CALL_RETURN:
    ThAwardExit(wk); 
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 地方図鑑賞状
  case MAIN_SEQ_CHIHOU_ZUKAN_AWARD_CALL:
    ChihouZukanAwardInit(wk);
    wk->main_seq = MAIN_SEQ_CHIHOU_ZUKAN_AWARD_CALL_RETURN;
    break;
  case MAIN_SEQ_CHIHOU_ZUKAN_AWARD_CALL_RETURN:
    ChihouZukanAwardExit(wk); 
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 全国図鑑賞状
  case MAIN_SEQ_ZENKOKU_ZUKAN_AWARD_CALL:
    ZenkokuZukanAwardInit(wk);
    wk->main_seq = MAIN_SEQ_ZENKOKU_ZUKAN_AWARD_CALL_RETURN;
    break;
  case MAIN_SEQ_ZENKOKU_ZUKAN_AWARD_CALL_RETURN:
    ZenkokuZukanAwardExit(wk); 
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 通信対戦後の録画選択画面
  case MAIN_SEQ_BTL_REC_SEL_CALL:
    BtlRecSelInit(wk);
    wk->main_seq = MAIN_SEQ_BTL_REC_SEL_CALL_RETURN;
    break;
  case MAIN_SEQ_BTL_REC_SEL_CALL_RETURN:
    BtlRecSelExit(wk); 
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 二択簡易会話
  case MAIN_SEQ_PMS_INPUT_DOUBLE_CALL:
    PmsInputDoubleInit(wk);
    wk->main_seq = MAIN_SEQ_PMS_INPUT_DOUBLE_CALL_RETURN;
    break;
  case MAIN_SEQ_PMS_INPUT_DOUBLE_CALL_RETURN:
    PmsInputDoubleExit(wk); 
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 一択簡易会話
  case MAIN_SEQ_PMS_INPUT_SINGLE_CALL:
    PmsInputSingleInit(wk);
    wk->main_seq = MAIN_SEQ_PMS_INPUT_SINGLE_CALL_RETURN;
    break;
  case MAIN_SEQ_PMS_INPUT_SINGLE_CALL_RETURN:
    PmsInputSingleExit(wk); 
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 三匹選択
  case MAIN_SEQ_PSEL_CALL:
    PselInit(wk);
    wk->main_seq = MAIN_SEQ_PSEL_CALL_RETURN;
    break;
  case MAIN_SEQ_PSEL_CALL_RETURN:
    PselExit(wk); 
    YoinSet( wk, 120, MAIN_SEQ_FADE_MAIN );
    wk->main_seq = MAIN_SEQ_YOIN;
    break;
  
    
  // 地下鉄路線図
  case MAIN_SEQ_SUBWAY_MAP_CALL:
    SubwayMapInit(wk);
    wk->main_seq = MAIN_SEQ_SUBWAY_MAP_CALL_RETURN;
    break;
  case MAIN_SEQ_SUBWAY_MAP_CALL_RETURN:
    SubwayMapExit(wk); 
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 定型文簡易会話
  case MAIN_SEQ_PMS_INPUT_SENTENCE_CALL:
    PmsInputSentenceInit(wk);
    wk->main_seq = MAIN_SEQ_PMS_INPUT_SENTENCE_CALL_RETURN;
    break;
  case MAIN_SEQ_PMS_INPUT_SENTENCE_CALL_RETURN:
    PmsInputSentenceExit(wk); 
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // タマゴ孵化デモ
  case MAIN_SEQ_EGG_DEMO_CALL:
    EggDemoInit(wk);
    wk->main_seq = MAIN_SEQ_EGG_DEMO_CALL_RETURN;
    break;
  case MAIN_SEQ_EGG_DEMO_CALL_RETURN:
    EggDemoExit(wk);
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 進化デモ
  case MAIN_SEQ_SHINKA_DEMO_CALL:
    ShinkaDemoInit(wk);
    wk->main_seq = MAIN_SEQ_SHINKA_DEMO_CALL_RETURN;
    break;
  case MAIN_SEQ_SHINKA_DEMO_CALL_RETURN:
    ShinkaDemoExit(wk);
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // 図鑑詳細
  case MAIN_SEQ_ZUKAN_DETAIL_CALL:
    ZukanDetailInit(wk);
    wk->main_seq = MAIN_SEQ_ZUKAN_DETAIL_CALL_RETURN;
    break;
  case MAIN_SEQ_ZUKAN_DETAIL_CALL_RETURN:
    ZukanDetailExit(wk);
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // ゲーム内マニュアル
  case MAIN_SEQ_MANUAL_CALL:
    ManualInit(wk);
    wk->main_seq = MAIN_SEQ_MANUAL_CALL_RETURN;
    break;
  case MAIN_SEQ_MANUAL_CALL_RETURN:
    ManualExit(wk);
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;


  // テスト
  // 進化デモカメラ
  case MAIN_SEQ_D_TEST_SHINKA_CALL:
    D_TestShinkaInit(wk);
    wk->main_seq = MAIN_SEQ_D_TEST_SHINKA_CALL_RETURN;
    break;
  case MAIN_SEQ_D_TEST_SHINKA_CALL_RETURN:
    D_TestShinkaExit(wk);
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;

  // タマゴ孵化デモカメラ
  case MAIN_SEQ_D_TEST_EGG_CALL:
    D_TestEggInit(wk);
    wk->main_seq = MAIN_SEQ_D_TEST_EGG_CALL_RETURN;
    break;
  case MAIN_SEQ_D_TEST_EGG_CALL_RETURN:
    D_TestEggExit(wk);
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;

  // 図鑑フォルム中カメラ
  case MAIN_SEQ_D_TEST_ZUKAN_FORM_CENTER_CALL:
    D_TestZukanFormInit(wk, 0);
    wk->main_seq = MAIN_SEQ_D_TEST_ZUKAN_FORM_CENTER_CALL_RETURN;
    break;
  case MAIN_SEQ_D_TEST_ZUKAN_FORM_CENTER_CALL_RETURN:
    D_TestZukanFormExit(wk);
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;
  
  // 図鑑フォルム左カメラ
  case MAIN_SEQ_D_TEST_ZUKAN_FORM_LEFT_CALL:
    D_TestZukanFormInit(wk, 1);
    wk->main_seq = MAIN_SEQ_D_TEST_ZUKAN_FORM_LEFT_CALL_RETURN;
    break;
  case MAIN_SEQ_D_TEST_ZUKAN_FORM_LEFT_CALL_RETURN:
    D_TestZukanFormExit(wk);
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;
    
    // 図鑑フォルム右カメラ
  case MAIN_SEQ_D_TEST_ZUKAN_FORM_RIGHT_CALL:
    D_TestZukanFormInit(wk, 2);
    wk->main_seq = MAIN_SEQ_D_TEST_ZUKAN_FORM_RIGHT_CALL_RETURN;
    break;
  case MAIN_SEQ_D_TEST_ZUKAN_FORM_RIGHT_CALL_RETURN:
    D_TestZukanFormExit(wk);
    FadeInSet( wk, MAIN_SEQ_INIT );
    wk->main_seq = MAIN_SEQ_FADE_MAIN;
    break;
 
  
  }

  return GFL_PROC_RES_CONTINUE;
}

static GFL_PROC_RESULT MainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  KAWADA_MAIN_WORK * wk = mywk;
  
  GFL_PROC_LOCAL_Exit( wk->local_procsys ); 

  GAMEDATA_Delete( wk->gamedata );

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_KAWADA_DEBUG );

  return GFL_PROC_RES_FINISH;
}

static void BgInit( KAWADA_MAIN_WORK * wk )
{
  GFL_BG_Init( wk->heapID );
  { // BG SYSTEM
    GFL_BG_SYS_HEADER sysh = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    GFL_BG_SetBGMode( &sysh );
  }
  { // メイン画面：文字
    GFL_BG_BGCNT_HEADER cnth= {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &cnth, GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen( GFL_BG_FRAME0_M );
    GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0, wk->heapID );
  }
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );

  // フォントパレット
  GFL_ARC_UTIL_TransVramPalette(
    ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 0, 0x20, wk->heapID );
}

static void BgExit(void)
{
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_OFF );
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
  GFL_BG_Exit();
}

// フェードイン
static void FadeInSet( KAWADA_MAIN_WORK * wk, u32 next )
{
  WIPE_SYS_Start(
    WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
    WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );

  wk->next_seq = next;
}

// フェードアウト
static void FadeOutSet( KAWADA_MAIN_WORK * wk, u32 next )
{
  WIPE_SYS_Start(
    WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
    WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, wk->heapID );

  wk->next_seq = next;
}

// 余韻
static void YoinSet( KAWADA_MAIN_WORK * wk, u32 count, u32 next )
{
  wk->yoin_count = count;
  wk->yoin_next_seq = next;
}

static void TopMenuInit( KAWADA_MAIN_WORK * wk )
{
  BMPMENULIST_HEADER  lh;
  u32 i;

  GFL_BMPWIN_Init( wk->heapID );

  wk->mman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_d_kawada_dat, wk->heapID );
  wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
  wk->que  = PRINTSYS_QUE_Create( wk->heapID );
  wk->win  = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 1, 1, 16, 12, 0, GFL_BMP_CHRAREA_GET_B );

  wk->ld = BmpMenuWork_ListCreate( TOP_MENU_SIZ, wk->heapID );
  for( i=0; i<TOP_MENU_SIZ; i++ ){
    STRBUF * str = GFL_MSG_CreateString( wk->mman, top_menu00+i );
    BmpMenuWork_ListAddString( &wk->ld[i], str, MAIN_SEQ_ZUKAN_DETAIL_G_CALL+i, wk->heapID );
    GFL_STR_DeleteBuffer( str );
  }

  lh = TopMenuListH;
  lh.list = wk->ld;
  lh.win  = wk->win;
  lh.msgdata = wk->mman;      //表示に使用するメッセージバッファ
  lh.print_util = &wk->util;  //表示に使用するプリントユーティリティ
  lh.print_que  = wk->que;    //表示に使用するプリントキュー
  lh.font_handle = wk->font;  //表示に使用するフォントハンドル

  wk->lw = BmpMenuList_Set( &lh, 0, 0, wk->heapID );
  BmpMenuList_SetCursorBmp( wk->lw, wk->heapID );

  GFL_BMPWIN_TransVramCharacter( wk->win );
  GFL_BMPWIN_MakeScreen( wk->win );
  GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame(wk->win) );
}

static void TopMenuExit( KAWADA_MAIN_WORK * wk )
{
  BmpMenuList_Exit( wk->lw, NULL, NULL );
  BmpMenuWork_ListDelete( wk->ld );

  GFL_BMPWIN_Delete( wk->win );
  PRINTSYS_QUE_Delete( wk->que );
  GFL_FONT_Delete( wk->font );
  GFL_MSG_Delete( wk->mman );

  GFL_BMPWIN_Exit();
}

// 図鑑登録
static void ZukanTorokuInit( KAWADA_MAIN_WORK* wk )
{
  ZONEDATA_Open( wk->heapID );

    //GFL_OVERLAY_Load(FS_OVERLAY_ID(zukan_toroku));
    wk->pp = PP_Create( 1, 1, 0, wk->heapID );
/*
    wk->zukan_toroku_param = ZUKAN_TOROKU_AllocParam(
        wk->heapID,
        ZUKAN_TOROKU_LAUNCH_TOROKU,
        wk->pp,
        FALSE,
        NULL,
        NULL,
        0 );
    wk->zukan_toroku_param->gamedata = wk->gamedata;
*/

    wk->zukan_toroku_param = GFL_HEAP_AllocClearMemory( wk->heapID, sizeof(ZUKAN_TOROKU_PARAM) );
    wk->zukan_toroku_param->launch           = ZUKAN_TOROKU_LAUNCH_TOROKU;
    wk->zukan_toroku_param->pp               = wk->pp;
    wk->zukan_toroku_param->b_zenkoku_flag   = FALSE;
    wk->zukan_toroku_param->box_strbuf       = NULL;
    wk->zukan_toroku_param->box_manager      = NULL;
    wk->zukan_toroku_param->box_tray         = 0;
    wk->zukan_toroku_param->gamedata         = wk->gamedata;
    
    //GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &ZUKAN_TOROKU_ProcData, wk->zukan_toroku_param );
    GFL_PROC_LOCAL_CallProc( wk->local_procsys, FS_OVERLAY_ID(zukan_toroku), &ZUKAN_TOROKU_ProcData, wk->zukan_toroku_param );
}
static void ZukanTorokuExit( KAWADA_MAIN_WORK* wk )
{
    GFL_HEAP_FreeMemory( wk->pp );
    //ZUKAN_TOROKU_FreeParam( wk->zukan_toroku_param );
    
    GFL_HEAP_FreeMemory( wk->zukan_toroku_param );
    
    //GFL_OVERLAY_Unload(FS_OVERLAY_ID(zukan_toroku));

  ZONEDATA_Close();
}

// トライアルハウス結果
static void ThAwardInit( KAWADA_MAIN_WORK* wk )
{
  SAVE_CONTROL_WORK*  sv  = GAMEDATA_GetSaveControlWork( wk->gamedata );
  CONFIG*             cf  = SaveData_GetConfig( sv );

      u8 sex = PM_MALE;
      u8 type = 0;  // 0=ローカルシングル; 1=ローカルダブル; 2=ダウンロードシングル; 3=ダウンロードダブル;
      BOOL b_me = FALSE;
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ) sex = PM_FEMALE;
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ) type = 1;
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ) type = 2;
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ) type = 3;
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT ) b_me = TRUE;

  CONFIG_SetMojiMode( cf, MOJIMODE_HIRAGANA );
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT ) CONFIG_SetMojiMode( cf, MOJIMODE_KANJI );

      GFL_OVERLAY_Load(FS_OVERLAY_ID(th_award));
      wk->thsv = GFL_HEAP_AllocMemory( wk->heapID, sizeof(THSV_WORK) );
      {
        u8 i;
#if 0 
        u16 name[17] = L"あいうえおかきくけこさしすせそた";
        name[16] = 0xffff;  // gflib/src/string/strbuf.c  // EOMCode
#else
        u16 name[17] = L"あいうえおかきく";
        name[8] = 0xffff;  // gflib/src/string/strbuf.c  // EOMCode
#endif

        wk->thsv->CommonData[0].Valid              = 1;
        wk->thsv->CommonData[0].IsDouble           = 0;
        wk->thsv->CommonData[0].Point              = 6000;
        wk->thsv->CommonData[0].MonsData[0].MonsNo = 1;
        wk->thsv->CommonData[0].MonsData[0].FormNo = 0;
        wk->thsv->CommonData[0].MonsData[0].Sex    = 0;
        wk->thsv->CommonData[0].MonsData[1].MonsNo = 2;
        wk->thsv->CommonData[0].MonsData[1].FormNo = 0;
        wk->thsv->CommonData[0].MonsData[1].Sex    = 0;
        wk->thsv->CommonData[0].MonsData[2].MonsNo = 3;
        wk->thsv->CommonData[0].MonsData[2].FormNo = 0;
        wk->thsv->CommonData[0].MonsData[2].Sex    = 0;
        wk->thsv->CommonData[0].MonsData[3].MonsNo = 6;
        wk->thsv->CommonData[0].MonsData[3].FormNo = 0;
        wk->thsv->CommonData[0].MonsData[3].Sex    = 0;

        wk->thsv->CommonData[1].Valid              = 1;
        wk->thsv->CommonData[1].IsDouble           = 0;
        wk->thsv->CommonData[1].Point              = 0xFFFF;
        wk->thsv->CommonData[1].MonsData[0].MonsNo = 1;
        wk->thsv->CommonData[1].MonsData[0].FormNo = 0;
        wk->thsv->CommonData[1].MonsData[0].Sex    = 0;
        wk->thsv->CommonData[1].MonsData[1].MonsNo = 2;
        wk->thsv->CommonData[1].MonsData[1].FormNo = 0;
        wk->thsv->CommonData[1].MonsData[1].Sex    = 0;
        wk->thsv->CommonData[1].MonsData[2].MonsNo = 3;
        wk->thsv->CommonData[1].MonsData[2].FormNo = 0;
        wk->thsv->CommonData[1].MonsData[2].Sex    = 0;
        wk->thsv->CommonData[1].MonsData[3].MonsNo = 6;
        wk->thsv->CommonData[1].MonsData[3].FormNo = 0;
        wk->thsv->CommonData[1].MonsData[3].Sex    = 0;

        for( i=0; i<16; i++ ) wk->thsv->DownloadBits[i] = 0;
        GFL_STD_MemCopy( name, wk->thsv->Name, 34 );

        switch(type)
        {
        case 0:
          {
          }
          break;
        case 1:
          {
            wk->thsv->CommonData[0].IsDouble           = 1;
          }
          break;
        case 2:
          {
          }
          break;
        case 3:
          {
            wk->thsv->CommonData[1].IsDouble           = 1;
          }
          break;
        }
      }
      wk->th_award_param = TH_AWARD_AllocParam( wk->heapID, sex, wk->thsv, (type==2||type==3)?TRUE:FALSE, b_me );
      GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &TH_AWARD_ProcData, wk->th_award_param );
}
static void ThAwardExit( KAWADA_MAIN_WORK* wk )
{
      TH_AWARD_FreeParam( wk->th_award_param );
      GFL_HEAP_FreeMemory( wk->thsv );
      GFL_OVERLAY_Unload(FS_OVERLAY_ID(th_award));
}

// 地方図鑑賞状
static void ChihouZukanAwardInit( KAWADA_MAIN_WORK* wk )
{
  SAVE_CONTROL_WORK*  sv  = GAMEDATA_GetSaveControlWork( wk->gamedata );
  CONFIG*             cf  = SaveData_GetConfig( sv );
  BOOL b_fix = TRUE;

  u8 i;
  u16 name[6] = L"ゴモジノコ";
  name[5] = 0xffff;  // gflib/src/string/strbuf.c  // EOMCode

  GFL_OVERLAY_Load(FS_OVERLAY_ID(chihou_zukan_award));

  CONFIG_SetMojiMode( cf, MOJIMODE_HIRAGANA );
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ) CONFIG_SetMojiMode( cf, MOJIMODE_KANJI );
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ) b_fix = FALSE;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  {
    u16 min_name[2] = L"1";
    min_name[1] = 0xffff;  // gflib/src/string/strbuf.c  // EOMCode
    for( i=0; i<2; i++ )
    {
      name[i] = min_name[i];
    }
  }

  wk->mystatus = GAMEDATA_GetMyStatus( wk->gamedata );
  
  for( i=0; i<6; i++ )
  {
    wk->mystatus->name[i] = name[i];
  }
  wk->mystatus->sex = PM_MALE;

  wk->chihou_zukan_award_param = CHIHOU_ZUKAN_AWARD_AllocParam( wk->heapID, wk->gamedata, b_fix );
  
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &CHIHOU_ZUKAN_AWARD_ProcData, wk->chihou_zukan_award_param );
}
static void ChihouZukanAwardExit( KAWADA_MAIN_WORK* wk )
{
  CHIHOU_ZUKAN_AWARD_FreeParam( wk->chihou_zukan_award_param );
  GFL_OVERLAY_Unload(FS_OVERLAY_ID(chihou_zukan_award));
}

// 全国図鑑賞状
static void ZenkokuZukanAwardInit( KAWADA_MAIN_WORK* wk )
{
  SAVE_CONTROL_WORK*  sv  = GAMEDATA_GetSaveControlWork( wk->gamedata );
  CONFIG*             cf  = SaveData_GetConfig( sv );
  BOOL b_fix = TRUE;

  u8 i;
  u16 name[6] = L"ゴモジノコ";
  name[5] = 0xffff;  // gflib/src/string/strbuf.c  // EOMCode

  GFL_OVERLAY_Load(FS_OVERLAY_ID(zenkoku_zukan_award));

  CONFIG_SetMojiMode( cf, MOJIMODE_HIRAGANA );
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ) CONFIG_SetMojiMode( cf, MOJIMODE_KANJI );
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ) b_fix = FALSE;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  {
    u16 min_name[2] = L"1";
    min_name[1] = 0xffff;  // gflib/src/string/strbuf.c  // EOMCode
    for( i=0; i<2; i++ )
    {
      name[i] = min_name[i];
    }
  }

  wk->mystatus = GAMEDATA_GetMyStatus( wk->gamedata );
  
  for( i=0; i<6; i++ )
  {
    wk->mystatus->name[i] = name[i];
  }
  wk->mystatus->sex = PM_FEMALE;

  wk->zenkoku_zukan_award_param = ZENKOKU_ZUKAN_AWARD_AllocParam( wk->heapID, wk->gamedata, b_fix );
  
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &ZENKOKU_ZUKAN_AWARD_ProcData, wk->zenkoku_zukan_award_param );
}
static void ZenkokuZukanAwardExit( KAWADA_MAIN_WORK* wk )
{
  ZENKOKU_ZUKAN_AWARD_FreeParam( wk->zenkoku_zukan_award_param );
  GFL_OVERLAY_Unload(FS_OVERLAY_ID(zenkoku_zukan_award));
}

// 通信対戦後の録画選択画面
static void BtlRecSelInit( KAWADA_MAIN_WORK* wk )
{
#if 0
  BOOL        b_rec       = TRUE;
  BOOL        b_sync      = TRUE;
  MYSTATUS*   mystatus    = GAMEDATA_GetMyStatus( wk->gamedata );
  MYITEM_PTR  myitem_ptr  = GAMEDATA_GetMyItem( wk->gamedata );
  if( MYITEM_CheckItem( myitem_ptr, ITEM_BATORUREKOODAA, 1, wk->heapID ) )
  {
    MYITEM_SubItem( myitem_ptr, ITEM_BATORUREKOODAA, 1, wk->heapID );
  }

  GFL_OVERLAY_Load(FS_OVERLAY_ID(btl_rec_sel));

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ) b_rec = FALSE;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ) b_sync = FALSE;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
    mystatus->sex = 0;
  }
  else
  {
    mystatus->sex = 1;
  }
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  {
    MYITEM_AddItem( myitem_ptr, ITEM_BATORUREKOODAA, 1, wk->heapID );
  }

  wk->btl_rec_sel_param = BTL_REC_SEL_AllocParam( wk->heapID, wk->gamedata, b_rec, b_sync,0,0 );
  
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &BTL_REC_SEL_ProcData, wk->btl_rec_sel_param );
#else
  GFL_OVERLAY_Load(FS_OVERLAY_ID(btl_rec_sel));
  wk->btl_rec_sel_param = BTL_REC_SEL_AllocParam( wk->heapID, wk->gamedata, FALSE, FALSE, 0, 0 );
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &BTL_REC_SEL_ProcData, wk->btl_rec_sel_param );
#endif
}
static void BtlRecSelExit( KAWADA_MAIN_WORK* wk )
{
  BTL_REC_SEL_FreeParam( wk->btl_rec_sel_param );
  GFL_OVERLAY_Unload(FS_OVERLAY_ID(btl_rec_sel));
}

// 二択簡易会話
static void PmsInputDoubleInit( KAWADA_MAIN_WORK* wk )
{
  SAVE_CONTROL_WORK*  sv  = GAMEDATA_GetSaveControlWork( wk->gamedata );
  GFL_OVERLAY_Load( FS_OVERLAY_ID(pmsinput) );
  wk->pmsi_param = PMSI_PARAM_Create( PMSI_MODE_DOUBLE, PMSI_GUIDANCE_DEFAULT, NULL, TRUE, sv, wk->heapID );
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &ProcData_PMSInput, wk->pmsi_param );
}
static void PmsInputDoubleExit( KAWADA_MAIN_WORK* wk )
{
  if( PMSI_PARAM_CheckModified( wk->pmsi_param ) )
  { 
    //PMS_DATA pms;
    //PMSI_PARAM_GetInputDataSentence( wk->pmsi_param, &pms );
    //OS_Printf( "%d, %d\n", pms.word[0], pms.word[1] );
    PMS_WORD word[2];
    PMSI_PARAM_GetInputDataDouble(  wk->pmsi_param, word );
    OS_Printf( "double: %d, %d\n", word[0], word[1] );
  }
  else
  {
    OS_Printf( "double: not modified\n" );
  }
  PMSI_PARAM_Delete( wk->pmsi_param );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(pmsinput) );
}

// 一択簡易会話
static void PmsInputSingleInit( KAWADA_MAIN_WORK* wk )
{
  SAVE_CONTROL_WORK*  sv  = GAMEDATA_GetSaveControlWork( wk->gamedata );
  GFL_OVERLAY_Load( FS_OVERLAY_ID(pmsinput) );
  wk->pmsi_param = PMSI_PARAM_Create( PMSI_MODE_SINGLE, PMSI_GUIDANCE_DEFAULT, NULL, TRUE, sv, wk->heapID );
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &ProcData_PMSInput, wk->pmsi_param );
}
static void PmsInputSingleExit( KAWADA_MAIN_WORK* wk )
{
  if( PMSI_PARAM_CheckModified( wk->pmsi_param ) )
  { 
    PMS_WORD word = PMSI_PARAM_GetInputDataSingle( wk->pmsi_param );
    OS_Printf( "single: %d\n", word );
  }
  else
  {
    OS_Printf( "single: not modified\n" );
  }
  PMSI_PARAM_Delete( wk->pmsi_param );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(pmsinput) );
}

// 三匹選択
static void PselInit( KAWADA_MAIN_WORK* wk )
{
  GFL_OVERLAY_Load( FS_OVERLAY_ID(psel) );
  wk->psel_param = PSEL_AllocParam( wk->heapID, &(wk->evwk) );
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &PSEL_ProcData, wk->psel_param );
}
static void PselExit( KAWADA_MAIN_WORK* wk )
{
  OS_Printf( "DebugKawada: evwk=%d\n", wk->evwk );
  PSEL_FreeParam( wk->psel_param );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(psel) );
}

// 地下鉄路線図
static void SubwayMapInit( KAWADA_MAIN_WORK* wk )
{
  GFL_OVERLAY_Load(FS_OVERLAY_ID(subway_map));

  wk->mystatus = GFL_HEAP_AllocMemory( wk->heapID, MYSTATUS_SAVE_SIZE );
  wk->mystatus->sex = PM_MALE;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R ) wk->mystatus->sex = PM_FEMALE;

  wk->subway_map_param = SUBWAY_MAP_AllocParam( wk->heapID, wk->mystatus );
  
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &SUBWAY_MAP_ProcData, wk->subway_map_param );
}
static void SubwayMapExit( KAWADA_MAIN_WORK* wk )
{
  SUBWAY_MAP_FreeParam( wk->subway_map_param );
  GFL_HEAP_FreeMemory( wk->mystatus );
  GFL_OVERLAY_Unload(FS_OVERLAY_ID(subway_map));
}

// 定型文簡易会話
static void PmsInputSentenceInit( KAWADA_MAIN_WORK* wk )
{
                                              // 定型文  初期入力  デコ文字
#define SENTENCE_CHANGE_NOSET_PICTURE    (0)  // 変更可  なし      あり
#define SENTENCE_CHANGE_SET_PICTURE      (1)  // 変更可  あり      あり
#define SENTENCE_NOCHANGE_NOSET_PICTURE  (2)  // 固定    なし      あり
#define SENTENCE_NOCHANGE_SET_PICTURE    (3)  // 固定    あり      あり
#define SENTENCE_CHANGE_NOSET_NOPICTURE  (4)  // 固定    なし      なし

  u8 sentence_mode = SENTENCE_CHANGE_NOSET_PICTURE;

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )  sentence_mode = SENTENCE_CHANGE_SET_PICTURE;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )  sentence_mode = SENTENCE_NOCHANGE_NOSET_PICTURE;
  if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )  sentence_mode = SENTENCE_NOCHANGE_SET_PICTURE;
  if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT ) sentence_mode = SENTENCE_CHANGE_NOSET_NOPICTURE;

  switch( sentence_mode )
  {
  case SENTENCE_CHANGE_NOSET_PICTURE:
    {
    }
    break;
  case SENTENCE_CHANGE_SET_PICTURE:
    {
    }
    break;
  case SENTENCE_NOCHANGE_NOSET_PICTURE:
    {
    }
    break;
  case SENTENCE_NOCHANGE_SET_PICTURE:
    {
    }
    break;
  case SENTENCE_CHANGE_NOSET_NOPICTURE:
    {
    }
    break;
  }
}
static void PmsInputSentenceExit( KAWADA_MAIN_WORK* wk )
{
}

// タマゴ孵化デモ
static void EggDemoInit( KAWADA_MAIN_WORK* wk )
{
  u16 monsno = MONSNO_KAMEKKUSU;

  ZONEDATA_Open( wk->heapID );

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )  monsno = MONSNO_MANAFI;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )  monsno = MONSNO_PIZYOTTO;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )  monsno = MONSNO_PIZYON;

  wk->pp = PP_Create( monsno, 1, 0, wk->heapID );
  PP_Put( wk->pp, ID_PARA_tamago_flag, 1 );
     
  wk->egg_demo_param = GFL_HEAP_AllocMemory( wk->heapID, sizeof(EGG_DEMO_PARAM) );
  wk->egg_demo_param->gamedata  = wk->gamedata;
  wk->egg_demo_param->pp        = wk->pp;
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, FS_OVERLAY_ID(egg_demo), &EGG_DEMO_ProcData, wk->egg_demo_param );
}
static void EggDemoExit( KAWADA_MAIN_WORK* wk )
{
  GFL_HEAP_FreeMemory( wk->egg_demo_param );
  GFL_HEAP_FreeMemory( wk->pp );

  ZONEDATA_Close();
}

// 進化デモ
static void ShinkaDemoInit( KAWADA_MAIN_WORK* wk )
{
  BOOL b_field = TRUE;

  u16 monsno_before;
  u16 monsno_after;

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
    //monsno_before = MONSNO_650;
    //monsno_after  = MONSNO_651;
    
    //monsno_before = MONSNO_563;
    //monsno_after  = MONSNO_564;
    
    monsno_before = MONSNO_564;
    monsno_after  = MONSNO_565;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    //monsno_before = MONSNO_557;
    //monsno_after  = MONSNO_558;

    monsno_before = MONSNO_563;
    monsno_after  = MONSNO_564;
  }
  else
  {
    //monsno_before = MONSNO_PIZYON;//MONSNO_KAMEERU;
    //monsno_after  = MONSNO_PIZYOTTO;//MONSNO_RIZAADON;
    
    //monsno_before = MONSNO_IIBUI;
    //monsno_after  = MONSNO_BURAKKII;
    
    monsno_before = MONSNO_ZUBATTO;
    monsno_after  = MONSNO_PIZYON;
  }

  ZONEDATA_Open( wk->heapID );
  GAMEBEACON_Setting(wk->gamedata);

  wk->pp = PP_Create( monsno_before, 98, 0, wk->heapID );
  
  wk->party = PokeParty_AllocPartyWork(wk->heapID);
  PokeParty_InitWork(wk->party);
  PokeParty_Init(wk->party, 6);
  PokeParty_Add( wk->party, wk->pp );
 
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ) b_field = FALSE;

  {
    SHINKA_DEMO_PARAM* sdp = GFL_HEAP_AllocMemory( wk->heapID, sizeof( SHINKA_DEMO_PARAM ) );
    sdp->gamedata          = wk->gamedata;
    sdp->ppt               = wk->party;
    sdp->after_mons_no     = monsno_after;
    sdp->shinka_pos        = 0;
    sdp->shinka_cond       = SHINKA_COND_LEVELUP;
    sdp->b_field           = b_field;
    sdp->b_enable_cancel   = TRUE;
    wk->shinka_demo_param  = sdp;
  }
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, FS_OVERLAY_ID(shinka_demo), &ShinkaDemoProcData, wk->shinka_demo_param );
}
static void ShinkaDemoExit( KAWADA_MAIN_WORK* wk )
{
  GFL_HEAP_FreeMemory( wk->shinka_demo_param );
  GFL_HEAP_FreeMemory( wk->party );
  GFL_HEAP_FreeMemory( wk->pp );
  ZONEDATA_Close();
}

// 図鑑詳細
static void ZukanDetailInit( KAWADA_MAIN_WORK* wk )
{
  u16 poke_list_num = POKE_LIST_NUM;
  const u8 lang_code[6] =
  {
    LANG_ENGLISH,
    LANG_FRANCE,
    LANG_GERMANY,
    LANG_ITALY,
    LANG_SPAIN, 
    LANG_KOREA, 
  };
  ZUKAN_SAVEDATA* zukan_savedata = GAMEDATA_GetZukanSave( wk->gamedata );
  u16 i;
  u16 lc;
  EVENTWORK* eventwork = GAMEDATA_GetEventWork( wk->gamedata );
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ) EVENTWORK_SetEventFlag( eventwork, SYS_FLAG_ARRIVE_T01 );

  ZONEDATA_Open( wk->heapID );
  
  ZUKANSAVE_SetGraphicVersionUpFlag( zukan_savedata );
 
  wk->poke_list[ 0] = 201;
  wk->poke_list[ 1] = 202;
  wk->poke_list[ 2] = 151;
  wk->poke_list[ 3] = 487;
  wk->poke_list[ 4] =  17;  // MONSNO_PIZYON
  wk->poke_list[ 5] =  18;  // MONSNO_PIZYOTTO
  wk->poke_list[ 6] = 582;  // MONSNO_TURURI
  wk->poke_list[ 7] = 583;  // MONSNO_TURUTTO
  wk->poke_list[ 8] = 584;  // MONSNO_TURUTURUDA
  wk->poke_list[ 9] = 585;  // MONSNO_SIKIZIKA
  wk->poke_list[10] = 586;  // MONSNO_ANTORESU
  wk->poke_list[11] = 382;  // MONSNO_KAIOOGA
  wk->poke_list[12] = 534;  // MONSNO_SIHANDON

  for( i=0; i<poke_list_num; i++ )
  {
    POKEMON_PARAM* pp = PP_Create( wk->poke_list[i] , 1, 0, wk->heapID );
    ZUKANSAVE_SetPokeSee( zukan_savedata, pp );  // 見た  // 図鑑フラグをセットする
    ZUKANSAVE_SetPokeGet( zukan_savedata, pp );  // 捕まえた  // 図鑑フラグをセットする
    
    if( wk->poke_list[i] == 201 )
    {
      u16 j;
      for( j=1; j<=6; j++ )
      {
        PP_Put( pp, ID_PARA_form_no, j );
        ZUKANSAVE_SetPokeSee( zukan_savedata, pp );  // 見た  // 図鑑フラグをセットする
        ZUKANSAVE_SetPokeGet( zukan_savedata, pp );  // 捕まえた  // 図鑑フラグをセットする
      }
    }
    if( wk->poke_list[i] == 487 )
    {
      u16 j;
      for( j=1; j<=1; j++ )
      {
        PP_Put( pp, ID_PARA_form_no, j );
        ZUKANSAVE_SetPokeSee( zukan_savedata, pp );  // 見た  // 図鑑フラグをセットする
        ZUKANSAVE_SetPokeGet( zukan_savedata, pp );  // 捕まえた  // 図鑑フラグをセットする
      }
    }
    if( wk->poke_list[i] == 17 )
    {
/*
      PP_Put( pp, ID_PARA_sex, 0 );
      ZUKANSAVE_SetPokeSee( zukan_savedata, pp );  // 見た  // 図鑑フラグをセットする
      ZUKANSAVE_SetPokeGet( zukan_savedata, pp );  // 捕まえた  // 図鑑フラグをセットする
      
      PP_Put( pp, ID_PARA_sex, 1 );
      ZUKANSAVE_SetPokeSee( zukan_savedata, pp );  // 見た  // 図鑑フラグをセットする
      ZUKANSAVE_SetPokeGet( zukan_savedata, pp );  // 捕まえた  // 図鑑フラグをセットする
*/
      GFL_HEAP_FreeMemory( pp );
      pp = PP_CreateEx( wk->poke_list[i] , 1, 0, PTL_SETUP_POW_AUTO, PTL_SETUP_RND_RARE, wk->heapID );
//      PP_Put( pp, ID_PARA_sex, 0 );
      ZUKANSAVE_SetPokeSee( zukan_savedata, pp );  // 見た  // 図鑑フラグをセットする
      ZUKANSAVE_SetPokeGet( zukan_savedata, pp );  // 捕まえた  // 図鑑フラグをセットする

      GFL_HEAP_FreeMemory( pp );
      pp = PP_CreateEx( wk->poke_list[i] , 1, 0, PTL_SETUP_POW_AUTO, PTL_SETUP_RND_RARE, wk->heapID );
 //     PP_Put( pp, ID_PARA_sex, 1 );
      ZUKANSAVE_SetPokeSee( zukan_savedata, pp );  // 見た  // 図鑑フラグをセットする
      ZUKANSAVE_SetPokeGet( zukan_savedata, pp );  // 捕まえた  // 図鑑フラグをセットする
    }
    if( wk->poke_list[i] == 382 )
    {
      GFL_HEAP_FreeMemory( pp );
      pp = PP_CreateEx( wk->poke_list[i] , 1, 0, PTL_SETUP_POW_AUTO, PTL_SETUP_RND_RARE, wk->heapID );
      ZUKANSAVE_SetPokeSee( zukan_savedata, pp );  // 見た  // 図鑑フラグをセットする
      ZUKANSAVE_SetPokeGet( zukan_savedata, pp );  // 捕まえた  // 図鑑フラグをセットする
    }
    if( wk->poke_list[i] == 534 )
    {
      GFL_HEAP_FreeMemory( pp );
      pp = PP_CreateEx( wk->poke_list[i] , 1, 0, PTL_SETUP_POW_AUTO, PTL_SETUP_RND_RARE, wk->heapID );
      ZUKANSAVE_SetPokeSee( zukan_savedata, pp );  // 見た  // 図鑑フラグをセットする
      ZUKANSAVE_SetPokeGet( zukan_savedata, pp );  // 捕まえた  // 図鑑フラグをセットする
    }

    for( lc=0; lc<6; lc++ )
    {
      PP_Put( pp, ID_PARA_country_code, lang_code[lc] );
      ZUKANSAVE_SetPokeSee( zukan_savedata, pp );  // 見た  // 図鑑フラグをセットする
      ZUKANSAVE_SetPokeGet( zukan_savedata, pp );  // 捕まえた  // 図鑑フラグをセットする
    }

    GFL_HEAP_FreeMemory( pp );
  }

  wk->zukan_detail_param = GFL_HEAP_AllocMemory( wk->heapID, sizeof(ZUKAN_DETAIL_PARAM) );
  wk->zukan_detail_param->gamedata = wk->gamedata;
  wk->zukan_detail_param->type     = ZUKAN_DETAIL_TYPE_INFO;
  wk->zukan_detail_param->list     = wk->poke_list;
  wk->zukan_detail_param->num      = poke_list_num;
  wk->zukan_detail_param->no       = 0;

  GFL_PROC_LOCAL_CallProc( wk->local_procsys, FS_OVERLAY_ID(zukan_detail), &ZUKAN_DETAIL_ProcData, wk->zukan_detail_param );
}
static void ZukanDetailExit( KAWADA_MAIN_WORK* wk )
{
  GFL_HEAP_FreeMemory( wk->zukan_detail_param );
  
  ZONEDATA_Close();
}

// 図鑑詳細G
static void ZukanDetailGInit( KAWADA_MAIN_WORK* wk )
{
  u16 poke_list_num = POKE_LIST_NUM_G;
  ZUKAN_SAVEDATA* zukan_savedata = GAMEDATA_GetZukanSave( wk->gamedata );
  u16 i;
  u16 lc;
  EVENTWORK* eventwork = GAMEDATA_GetEventWork( wk->gamedata );
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ) EVENTWORK_SetEventFlag( eventwork, SYS_FLAG_ARRIVE_T01 );

  ZONEDATA_Open( wk->heapID );
  
  ZUKANSAVE_SetGraphicVersionUpFlag( zukan_savedata );


  wk->poke_list[ 0] = MONSNO_529;//ゾロア
  wk->poke_list[ 1] = MONSNO_530;//ゾロアーク

  wk->poke_list[ 2] = MONSNO_551;//ツタージャ
  wk->poke_list[ 3] = MONSNO_554;//ポカブ
  wk->poke_list[ 4] = MONSNO_557;//ミジュマル
  wk->poke_list[ 5] = MONSNO_650;//レシラム
  wk->poke_list[ 6] = MONSNO_651;//ゼクロム

  wk->poke_list[ 7] = MONSNO_504;//メグロコ
  wk->poke_list[ 8] = MONSNO_540;//ムンナ
  wk->poke_list[ 9] = MONSNO_544;//ギアル
  wk->poke_list[10] = MONSNO_514;//マメパト
  wk->poke_list[11] = MONSNO_570;//シママ
  wk->poke_list[12] = MONSNO_503;//ヒヒダルマ
  wk->poke_list[13] = MONSNO_533;//チラーミィ

  wk->poke_list[14] = MONSNO_541;//キバゴ
  wk->poke_list[15] = MONSNO_537;//ミネズミ
  wk->poke_list[16] = MONSNO_520;//コロモリ
  wk->poke_list[17] = MONSNO_585;//ランクルス
  wk->poke_list[18] = MONSNO_608;//ゴチルゼル
  wk->poke_list[19] = MONSNO_516;//ムシャーナ
  wk->poke_list[20] = MONSNO_567;//ウォーグル

  wk->poke_list[21] = MONSNO_611;//ツルリ
  wk->poke_list[22] = MONSNO_612;//ツルット
  wk->poke_list[23] = MONSNO_613;//ツルツルダ


  for( i=0; i<poke_list_num; i++ )
  {
    POKEMON_PARAM* pp = PP_Create( wk->poke_list[i] , 1, 0, wk->heapID );
    ZUKANSAVE_SetPokeSee( zukan_savedata, pp );  // 見た  // 図鑑フラグをセットする
    ZUKANSAVE_SetPokeGet( zukan_savedata, pp );  // 捕まえた  // 図鑑フラグをセットする

    GFL_HEAP_FreeMemory( pp );
    pp = PP_CreateEx( wk->poke_list[i] , 1, 0, PTL_SETUP_POW_AUTO, PTL_SETUP_RND_RARE, wk->heapID );
    ZUKANSAVE_SetPokeSee( zukan_savedata, pp );  // 見た  // 図鑑フラグをセットする
    ZUKANSAVE_SetPokeGet( zukan_savedata, pp );  // 捕まえた  // 図鑑フラグをセットする

    GFL_HEAP_FreeMemory( pp );
  }

  wk->zukan_detail_param = GFL_HEAP_AllocMemory( wk->heapID, sizeof(ZUKAN_DETAIL_PARAM) );
  wk->zukan_detail_param->gamedata = wk->gamedata;
  wk->zukan_detail_param->type     = ZUKAN_DETAIL_TYPE_INFO;
  wk->zukan_detail_param->list     = wk->poke_list;
  wk->zukan_detail_param->num      = poke_list_num;
  wk->zukan_detail_param->no       = 0;

  GFL_PROC_LOCAL_CallProc( wk->local_procsys, FS_OVERLAY_ID(zukan_detail), &ZUKAN_DETAIL_ProcData, wk->zukan_detail_param );
}
static void ZukanDetailGExit( KAWADA_MAIN_WORK* wk )
{
  GFL_HEAP_FreeMemory( wk->zukan_detail_param );
  
  ZONEDATA_Close();
}

// ゲーム内マニュアル
static void ManualInit( KAWADA_MAIN_WORK* wk )
{
  wk->manual_param            = GFL_HEAP_AllocMemory( wk->heapID, sizeof( MANUAL_PARAM ) );
  wk->manual_param->gamedata  = wk->gamedata;
  
  GFL_PROC_LOCAL_CallProc( wk->local_procsys, FS_OVERLAY_ID(manual), &MANUAL_ProcData, wk->manual_param );
}
static void ManualExit( KAWADA_MAIN_WORK* wk )
{
  GFL_HEAP_FreeMemory( wk->manual_param );
}

// テスト
static void D_TestShinkaInit( KAWADA_MAIN_WORK* wk )
{
  wk->d_test_shinka_param = GFL_HEAP_AllocMemory( wk->heapID, sizeof( D_KAWADA_TEST_SHINKA_PARAM ) );
 
  //wk->d_test_shinka_param-> = ;
  //wk->d_test_shinka_param-> = ;

  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &D_KAWADA_TEST_ShinkaProcData, wk->d_test_shinka_param );
}
static void D_TestShinkaExit( KAWADA_MAIN_WORK* wk )
{
  GFL_HEAP_FreeMemory( wk->d_test_shinka_param );
}

static void D_TestEggInit( KAWADA_MAIN_WORK* wk )
{
  wk->d_test_egg_param = GFL_HEAP_AllocMemory( wk->heapID, sizeof( D_KAWADA_TEST_EGG_PARAM ) );

  //wk->d_test_egg_param-> = ;
  //wk->d_test_egg_param-> = ;

  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &D_KAWADA_TEST_EggProcData, wk->d_test_egg_param );
}
static void D_TestEggExit( KAWADA_MAIN_WORK* wk )
{
  GFL_HEAP_FreeMemory( wk->d_test_egg_param );
}

static void D_TestZukanFormInit( KAWADA_MAIN_WORK* wk, u8 center_left_right )
{
  wk->d_test_zukan_form_param = GFL_HEAP_AllocMemory( wk->heapID, sizeof( D_KAWADA_TEST_ZUKAN_FORM_PARAM ) );

  wk->d_test_zukan_form_param->mode = center_left_right;

  GFL_PROC_LOCAL_CallProc( wk->local_procsys, NO_OVERLAY_ID, &D_KAWADA_TEST_ZukanFormProcData, wk->d_test_zukan_form_param );
}
static void D_TestZukanFormExit( KAWADA_MAIN_WORK* wk )
{
  GFL_HEAP_FreeMemory( wk->d_test_zukan_form_param );
}

#endif
