//////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 初期画面 ( メニュー画面 )
 * @file   research_menu.c
 * @author obata
 * @date   2010.02.03
 */
//////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "bg_font.h"
#include "queue.h"
#include "palette_anime.h"
#include "research_menu.h"
#include "research_menu_index.h"
#include "research_menu_def.h"
#include "research_menu_data.cdat"
#include "research_common.h"
#include "research_common_data.cdat"

#include "system/main.h"            // for HEAPID_xxxx
#include "system/gfl_use.h"         // for GFUser_xxxx
#include "print/gf_font.h"          // for GFL_FONT_xxxx
#include "print/printsys.h"         // for PRINTSYS_xxxx
#include "gamesystem/game_beacon.h" // for GAMEBEACON_xxxx
#include "sound/pm_sndsys.h"        // for PMSND_xxxx
#include "sound/wb_sound_data.sadl" // for SEQ_SE_XXXX

#include "arc/arc_def.h"                    // for ARCID_xxxx
#include "arc/research_radar_graphic.naix"  // for NARC_research_radar_xxxx
#include "arc/font/font.naix"               // for NARC_font_xxxx
#include "arc/message.naix"                 // for NARC_message_xxxx
#include "msg/msg_research_radar.h"         // for str_xxxx
#include "obj_NANR_LBLDEFS.h"               // for NANR_obj_xxxx


//====================================================================================
// ■調査初期画面 ワーク
//====================================================================================
struct _RESEARCH_MENU_WORK
{ 
  RESEARCH_COMMON_WORK* commonWork; // 全画面共通ワーク
  HEAPID                heapID;
  GAMESYS_WORK*         gameSystem;
  GAMEDATA*             gameData;

  GFL_FONT*    font;
  GFL_MSGDATA* message[ MESSAGE_NUM ];

  QUEUE*               seqQueue;      // シーケンスキュー
  RESEARCH_MENU_SEQ    seq;           // 処理シーケンス
  u32                  seqCount;      // シーケンスカウンタ
  BOOL                 seqFinishFlag; // 現在のシーケンスが終了したかどうか
  RESEARCH_MENU_RESULT result;        // 画面終了結果

  MENU_ITEM cursorPos;     // カーソル位置
  BOOL      newEntryFlag;  // 新しい人物とすれ違ったかどうか

  // タッチ領域
  GFL_UI_TP_HITTBL touchHitTable[ TOUCH_AREA_NUM ];

  // パレットアニメーション
  PALETTE_ANIME* paletteAnime[ PALETTE_ANIME_NUM ];

  // 文字列描画オブジェクト
  BG_FONT* BGFont[ BG_FONT_NUM ];

  // OBJ
  u32         objResRegisterIdx[ OBJ_RESOURCE_NUM ];  // リソースの登録インデックス
  GFL_CLUNIT* clactUnit[ CLUNIT_NUM ];                // セルアクターユニット
  GFL_CLWK*   clactWork[ CLWK_NUM ];                  // セルアクターワーク
};



//====================================================================================
// ■関数インデックス
//====================================================================================
//------------------------------------------------------------------------------------
// ◆LAYER 3 シーケンス
//------------------------------------------------------------------------------------
// シーケンス初期化処理
static void InitSeq_SETUP( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_SETUP
static void InitSeq_STAND_BY( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_STAND_BY
static void InitSeq_KEY_WAIT( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_WAIT
static void InitSeq_FADE_OUT( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_FADE_OUT
static void InitSeq_CLEAN_UP( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_CLEAN_UP 
// シーケンス処理
static void Main_SETUP( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_SETUP
static void Main_STAND_BY( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_STAND_BY
static void Main_KEY_WAIT( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_WAIT
static void Main_FADE_OUT( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_FADE_OUT
static void Main_CLEAN_UP( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_CLEAN_UP 
// シーケンス終了処理
static void FinishSeq_SETUP( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_SETUP
static void FinishSeq_STAND_BY( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_STAND_BY
static void FinishSeq_KEY_WAIT( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_WAIT
static void FinishSeq_FADE_OUT( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_FADE_OUT
static void FinishSeq_CLEAN_UP( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_CLEAN_UP 
// シーケンス制御
static void CountUpSeqCount( RESEARCH_MENU_WORK* work ); // シーケンスカウンタを更新する
static void SetNextSeq( RESEARCH_MENU_WORK* work, RESEARCH_MENU_SEQ nextSeq ); // 次のシーケンスをキューに登録する
static void FinishCurrentSeq( RESEARCH_MENU_WORK* work ); // 現在のシーケンスを終了する
static void SwitchSeq( RESEARCH_MENU_WORK* work ); // 処理シーケンスを変更する
static void SetSeq( RESEARCH_MENU_WORK* work, RESEARCH_MENU_SEQ nextSeq ); // 処理シーケンスを設定する
static void SetResult( RESEARCH_MENU_WORK* work, RESEARCH_MENU_RESULT result ); // 画面終了結果を設定する
//------------------------------------------------------------------------------------
// ◆LAYER 2 機能
//------------------------------------------------------------------------------------
// データ更新
static void CheckNewEntry( RESEARCH_MENU_WORK* work ); // 新しい人物とすれ違ったかどうかをチェックする
// カーソル移動
static void MoveCursorUp( RESEARCH_MENU_WORK* work ); // カーソルを上に移動する
static void MoveCursorDown( RESEARCH_MENU_WORK* work ); // カーソルを下に移動する
static void MoveCursorDirect( RESEARCH_MENU_WORK* work, MENU_ITEM menuItem ); // カーソルを直接移動する
// メニュー項目ボタン
static void MenuItemSetCursorOn( MENU_ITEM menuItem ); // カーソルが乗っている状態にする
static void MenuItemSetCursorOff( MENU_ITEM menuItem ); // カーソルが乗っていない状態にする
// "new" アイコン
static void NewIconDispOn( const RESEARCH_MENU_WORK* work ); // "new" アイコンを表示する
static void NewIconDispOff( const RESEARCH_MENU_WORK* work ); // "new" アイコンを非表示にする
//------------------------------------------------------------------------------------
// ◆LAYER 1 個別操作
//------------------------------------------------------------------------------------
// カーソル位置
static void ShiftCursorPos( RESEARCH_MENU_WORK* work, int offset ); // カーソル位置を変更する ( オフセット指定 )
static void SetCursorPos( RESEARCH_MENU_WORK* work, MENU_ITEM menuItem ); // カーソル位置を変更する ( 直値指定 )
// 調査初期画面ワーク
static void CreateMenuWork( HEAPID heapID ); // 調査初期画面ワークを生成する
static void DeleteMenuWork( RESEARCH_MENU_WORK* work ); // 調査初期画面ワークを破棄する
static void InitMenuWork( RESEARCH_MENU_WORK* work ); // 調査初期画面ワークを初期化する
static void SetupMenuWork( RESEARCH_MENU_WORK* work ); // 調査初期画面ワークをセットアップする
static void CleanUpMenuWork( RESEARCH_MENU_WORK* work ); // 調査初期画面ワークをクリーンアップする
// フォント
static void InitFont( RESEARCH_MENU_WORK* work ); // フォントデータを初期化する
static void CreateFont( RESEARCH_MENU_WORK* work ); // フォントデータを生成する
static void DeleteFont( RESEARCH_MENU_WORK* work ); // フォントデータを破棄する
// メッセージ
static void InitMessages( RESEARCH_MENU_WORK* work ); // メッセージデータを初期化する
static void CreateMessages( RESEARCH_MENU_WORK* work ); // メッセージデータを生成する
static void DeleteMessages( RESEARCH_MENU_WORK* work ); // メッセージデータを破棄する
// タッチ領域
static void SetupTouchArea( RESEARCH_MENU_WORK* work ); // タッチ領域をセットアップする
// シーケンスキュー
static void InitSeqQueue( RESEARCH_MENU_WORK* work ); // シーケンスキューを初期化する
static void CreateSeqQueue( RESEARCH_MENU_WORK* work ); // シーケンスキューを生成する
static void DeleteSeqQueue( RESEARCH_MENU_WORK* work ); // シーケンスキューを破棄する
// BG
static void SetupBG( RESEARCH_MENU_WORK* work ); // BG全般のセットアップを行う
static void SetupSubBG_WINDOW( RESEARCH_MENU_WORK* work ); // SUB-BG ( ウィンドウ面 ) のセットアップを行う
static void SetupSubBG_FONT( RESEARCH_MENU_WORK* work ); // SUB-BG ( フォント面 ) のセットアップを行う
static void SetupMainBG_WINDOW( RESEARCH_MENU_WORK* work ); // MAIN-BG ( ウィンドウ面 ) のセットアップを行う
static void SetupMainBG_FONT( RESEARCH_MENU_WORK* work ); // MAIN-BG ( フォント面 ) のセットアップを行う
static void CleanUpBG( RESEARCH_MENU_WORK* work ); // BG全般のクリーンアップを行う
static void CleanUpSubBG_WINDOW( RESEARCH_MENU_WORK* work ); // SUB-BG ( ウィンドウ面 ) のクリーンアップを行う
static void CleanUpSubBG_FONT( RESEARCH_MENU_WORK* work ); // SUB-BG ( フォント面 ) のクリーンアップを行う
static void CleanUpMainBG_WINDOW( RESEARCH_MENU_WORK* work ); // MAIN-BG ( ウィンドウ面 ) のクリーンアップを行う
static void CleanUpMainBG_FONT( RESEARCH_MENU_WORK* work ); // MAIN-BG ( フォント面 ) のクリーンアップを行う
// 文字列描画オブジェクト
static void InitBGFonts( RESEARCH_MENU_WORK* work ); // 文字列描画オブジェクトを初期化する
static void CreateBGFonts( RESEARCH_MENU_WORK* work ); // 文字列描画オブジェクトを生成する
static void DeleteBGFonts( RESEARCH_MENU_WORK* work ); // 文字列描画オブジェクトを破棄する
// OBJ リソース
static void InitOBJResources( RESEARCH_MENU_WORK* work ); // OBJリソースを初期化する
static void RegisterSubOBJResources( RESEARCH_MENU_WORK* work ); // OBJリソースを登録する ( SUB-OBJ )
static void RegisterMainOBJResources( RESEARCH_MENU_WORK* work ); // OBJリソースを登録する ( MAIN-OBJ )
static void ReleaseSubOBJResources( RESEARCH_MENU_WORK* work ); // OBJリソースの登録を解除する ( SUB-OBJ )
static void ReleaseMainOBJResources( RESEARCH_MENU_WORK* work ); // OBJリソースの登録を解除する ( MAIN-OBJ )
static u32 GetOBJResRegisterIndex( const RESEARCH_MENU_WORK* work, OBJ_RESOURCE_ID resID ); // OBJリソースの登録インデックスを取得する
// セルアクターシステム
static void CreateClactSystem( RESEARCH_MENU_WORK* work ); // セルアクターシステムを生成する
static void DeleteClactSystem( RESEARCH_MENU_WORK* work ); // セルアクターシステムを破棄する
// セルアクターユニット
static void InitClactUnits( RESEARCH_MENU_WORK* work ); // セルアクターユニットを初期化する
static void CreateClactUnits( RESEARCH_MENU_WORK* work ); // セルアクターユニットを生成する
static void DeleteClactUnits( RESEARCH_MENU_WORK* work ); // セルアクターユニットを破棄する
static GFL_CLUNIT* GetClactUnit( const RESEARCH_MENU_WORK* work, CLUNIT_INDEX unitIdx ); // セルアクターユニットを取得する
// セルアクターワーク
static void InitClactWorks( RESEARCH_MENU_WORK* work ); // セルアクターワークを初期化する
static void CreateClactWorks( RESEARCH_MENU_WORK* work ); // セルアクターワークを生成する
static void DeleteClactWorks( RESEARCH_MENU_WORK* work ); // セルアクターワークを破棄する
static GFL_CLWK* GetClactWork( const RESEARCH_MENU_WORK* work, CLWK_INDEX wkIdx ); // セルアクターワークを取得する
// パレットアニメーション
static void InitPaletteAnime( RESEARCH_MENU_WORK* work ); // パレットアニメーションワークを初期化する
static void CreatePaletteAnime( RESEARCH_MENU_WORK* work ); // パレットアニメーションワークを生成する
static void DeletePaletteAnime( RESEARCH_MENU_WORK* work ); // パレットアニメーションワークを破棄する
static void SetupPaletteAnime( RESEARCH_MENU_WORK* work ); // パレットアニメーションワークをセットアップする
static void CleanUpPaletteAnime( RESEARCH_MENU_WORK* work ); // パレットアニメーションワークをクリーンアップする
static void StartPaletteAnime( RESEARCH_MENU_WORK* work, PALETTE_ANIME_INDEX index ); // パレットアニメーションを開始する
static void StopPaletteAnime( RESEARCH_MENU_WORK* work, PALETTE_ANIME_INDEX index ); // パレットアニメーションを停止する
static void UpdatePaletteAnime( RESEARCH_MENU_WORK* work ); // パレットアニメーションを更新する
//------------------------------------------------------------------------------------
// ◆LAYER 0 デバッグ
//------------------------------------------------------------------------------------
static void DebugPrint_seqQueue( const RESEARCH_MENU_WORK* work ); // シーケンスキューを出力する




//====================================================================================
// □調査初期画面 制御関数
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 調査初期画面ワークを生成する
 *
 * @param commonWork 全画面共通ワーク
 *
 * @return 生成した調査初期画面ワーク
 */
//------------------------------------------------------------------------------------
RESEARCH_MENU_WORK* CreateResearchMenuWork( RESEARCH_COMMON_WORK* commonWork )
{
  int i;
  RESEARCH_MENU_WORK* work;
  HEAPID heapID;

  heapID = RESEARCH_COMMON_GetHeapID( commonWork );

  // 生成
  work = GFL_HEAP_AllocMemory( heapID, sizeof(RESEARCH_MENU_WORK) );

  // 初期化
  work->commonWork   = commonWork;
  work->heapID       = heapID;
  work->gameSystem   = RESEARCH_COMMON_GetGameSystem( commonWork );
  work->gameData     = RESEARCH_COMMON_GetGameData( commonWork );
  work->seq          = RESEARCH_MENU_SEQ_SETUP;
  work->seqFinishFlag= FALSE;
  work->seqCount     = 0;
  work->result       = RESEARCH_MENU_RESULT_NONE;
  work->cursorPos    = MENU_ITEM_CHANGE_RESEARCH;
  work->newEntryFlag = FALSE;

  InitOBJResources( work );
  InitSeqQueue( work );
  InitMessages( work );
  InitFont( work );
  InitBGFonts( work );
  InitClactUnits( work );
  InitClactWorks( work );
  InitPaletteAnime( work );

  CreateSeqQueue( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create work\n" );

  return work;
}

//------------------------------------------------------------------------------------
/**
 * @brief 調査初期画面ワークの破棄
 *
 * @param heapID
 */
//------------------------------------------------------------------------------------
void DeleteResearchMenuWork( RESEARCH_MENU_WORK* work )
{
  if( work == NULL ) {
    GF_ASSERT(0);
    return;
  }

  DeleteSeqQueue( work ); // シーケンスキュー
  GFL_HEAP_FreeMemory( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete work\n" );
} 

//------------------------------------------------------------------------------------
/**
 * @brief 調査初期画面 メイン動作
 *
 * @param work
 */
//------------------------------------------------------------------------------------
RESEARCH_MENU_RESULT ResearchMenuMain( RESEARCH_MENU_WORK* work )
{
  // シーケンスごとの処理
  switch( work->seq ) {
  case RESEARCH_MENU_SEQ_SETUP:    Main_SETUP   ( work ); break;
  case RESEARCH_MENU_SEQ_STAND_BY: Main_STAND_BY( work ); break;
  case RESEARCH_MENU_SEQ_KEY_WAIT: Main_KEY_WAIT( work ); break;
  case RESEARCH_MENU_SEQ_FADE_OUT: Main_FADE_OUT( work ); break;
  case RESEARCH_MENU_SEQ_CLEAN_UP: Main_CLEAN_UP( work ); break;
  case RESEARCH_MENU_SEQ_FINISH:   return work->result;
  default:  GF_ASSERT(0);
  }

  CheckNewEntry( work );      // データ更新
  UpdatePaletteAnime( work ); // パレットアニメーション更新
  RESEARCH_COMMON_UpdatePaletteAnime( work->commonWork ); // 共通パレットアニメーションを更新
  GFL_CLACT_SYS_Main();       // セルアクターシステム メイン処理

  CountUpSeqCount( work ); // シーケンスカウンタ更新
  SwitchSeq( work );  // シーケンス更新

  return RESEARCH_MENU_RESULT_CONTINUE;
}


//====================================================================================
// ■シーケンス処理
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 準備シーケンス ( RESEARCH_MENU_SEQ_SETUP ) の初期化処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitSeq_SETUP( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init seq SETUP\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief 最初のキー待ちシーケンス ( RESEARCH_MENU_SEQ_STAND_BY ) の初期化処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitSeq_STAND_BY( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init seq STAND_BY\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief キー入力待ちシーケンス ( RESEARCH_MENU_SEQ_KEY_WAIT ) の初期化処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitSeq_KEY_WAIT( RESEARCH_MENU_WORK* work )
{
  MenuItemSetCursorOn( work->cursorPos ); // カーソル位置のメニュー項目を選択状態にする
  StartPaletteAnime( work, PALETTE_ANIME_CURSOR_ON ); // カーソルONパレットアニメを開始

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init seq KEY_WAIT\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_MENU_SEQ_CLEAN_UP ) の初期化処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitSeq_CLEAN_UP( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init seq CLEAN_UP\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_MENU_SEQ_FADE_OUT ) の初期化処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitSeq_FADE_OUT( RESEARCH_MENU_WORK* work )
{
  // フェードアウト開始
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 
      0, 16, 0);

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init seq FADE_OUT\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief 準備シーケンス ( RESEARCH_MENU_SEQ_SETUP ) の初期化処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void FinishSeq_SETUP( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: finish seq SETUP\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief 最初のキー入力待ちシーケンス ( RESEARCH_MENU_SEQ_STAND_BY ) の初期化処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void FinishSeq_STAND_BY( RESEARCH_MENU_WORK* work )
{
  // カーソル位置のメニュー項目を選択状態にする
  MenuItemSetCursorOn( work->cursorPos );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: finish seq STAND_BY\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief キー入力待ちシーケンス ( RESEARCH_MENU_SEQ_KEY_WAIT ) の初期化処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void FinishSeq_KEY_WAIT( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: finish seq KEY_WAIT\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_MENU_SEQ_CLEAN_UP ) の初期化処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void FinishSeq_CLEAN_UP( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: finish seq CLEAN_UP\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_MENU_SEQ_FADE_OUT ) の初期化処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void FinishSeq_FADE_OUT( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: finish seq FADE_OUT\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief 準備シーケンス ( RESEARCH_MENU_SEQ_SETUP ) の処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void Main_SETUP( RESEARCH_MENU_WORK* work )
{
  CreateFont( work );
  CreateMessages( work );
  SetupTouchArea( work );

  // BG 準備
  SetupBG           ( work );
  SetupSubBG_WINDOW ( work );
  SetupSubBG_FONT   ( work );
  SetupMainBG_WINDOW( work );
  SetupMainBG_FONT  ( work );

  // 文字列描画オブジェクト 準備
  CreateBGFonts( work );

  // OBJ 準備
  CreateClactSystem( work );
  RegisterSubOBJResources( work );
  RegisterMainOBJResources( work );
  CreateClactUnits( work );
  CreateClactWorks( work );

  // パレットアニメーション
  CreatePaletteAnime( work );
  SetupPaletteAnime( work );

  // 画面フェードイン
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 
      16, 0, 0);

  // 次のシーケンスをセット
  SetNextSeq( work, RESEARCH_MENU_SEQ_STAND_BY ); 

  // シーケンス終了
  FinishCurrentSeq( work );
}

//------------------------------------------------------------------------------------
/**
 * @brief 最初のキー待ちシーケンス ( RESEARCH_MENU_SEQ_STAND_BY ) の処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void Main_STAND_BY( RESEARCH_MENU_WORK* work )
{
  int trg;
  int touch;
  int commonTouch;

  trg = GFL_UI_KEY_GetTrg();
  touch = GFL_UI_TP_HitTrg( work->touchHitTable );
  commonTouch = GFL_UI_TP_HitTrg( RESEARCH_COMMON_GetHitTable(work->commonWork) );

  //-----------------
  //「もどる」ボタン
  if( commonTouch == COMMON_TOUCH_AREA_RETURN_BUTTON ) {
    RESEARCH_COMMON_StartPaletteAnime( 
        work->commonWork, COMMON_PALETTE_ANIME_RETURN ); // 選択パレットアニメ開始
    PMSND_PlaySE( SEQ_SE_CANCEL1 );                      // キャンセル音
    SetResult( work, RESEARCH_MENU_RESULT_EXIT );        // 画面終了結果を決定
    SetNextSeq( work, RESEARCH_MENU_SEQ_FADE_OUT );
    SetNextSeq( work, RESEARCH_MENU_SEQ_CLEAN_UP );
    FinishCurrentSeq( work );
    return;
  }

  //--------------------
  // 十字キー or A or B
  if( (trg & PAD_KEY_UP)   || (trg & PAD_KEY_DOWN)  ||
      (trg & PAD_KEY_LEFT) || (trg & PAD_KEY_RIGHT) ||
      (trg & PAD_BUTTON_A) || (trg & PAD_BUTTON_B) ) {
    SetNextSeq( work, RESEARCH_MENU_SEQ_KEY_WAIT );
    FinishCurrentSeq( work );
  }

  //-------------------------------------
  //「調査内容を変更する」ボタンをタッチ
  if( touch == TOUCH_AREA_CHANGE_BUTTON ) {
    MoveCursorDirect( work, MENU_ITEM_CHANGE_RESEARCH ); // カーソル位置を更新
    StopPaletteAnime( work, PALETTE_ANIME_CURSOR_ON );   // カーソルONパレットアニメ終了
    StartPaletteAnime( work, PALETTE_ANIME_SELECT );     // 選択パレットアニメ開始
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                      // 決定音
    SetResult( work, RESEARCH_MENU_RESULT_TO_SELECT );   // 画面終了結果を決定
    SetNextSeq( work, RESEARCH_MENU_SEQ_FADE_OUT );
    SetNextSeq( work, RESEARCH_MENU_SEQ_CLEAN_UP );
    FinishCurrentSeq( work );
    return;
  }
  //-------------------------------------
  //「調査報告を確認する」ボタンをタッチ
  if( touch == TOUCH_AREA_CHECK_BUTTON ) {
    MoveCursorDirect( work, MENU_ITEM_CHECK_RESEARCH ); // カーソル位置を更新
    StopPaletteAnime( work, PALETTE_ANIME_CURSOR_ON );  // カーソルONパレットアニメ終了
    StartPaletteAnime( work, PALETTE_ANIME_SELECT );    // 選択パレットアニメ開始
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                     // 決定音
    SetResult( work, RESEARCH_MENU_RESULT_TO_CHECK );   // 画面終了結果を決定
    SetNextSeq( work, RESEARCH_MENU_SEQ_FADE_OUT );
    SetNextSeq( work, RESEARCH_MENU_SEQ_CLEAN_UP );
    FinishCurrentSeq( work );
    return;
  }

  //------------------
  // L ボタン (DEBUG)
  if( trg & PAD_BUTTON_L ) {
    DEBUG_GAMEBEACON_Set_NewEntry();
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief キー入力待ちシーケンス ( RESEARCH_MENU_SEQ_KEY_WAIT ) の処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void Main_KEY_WAIT( RESEARCH_MENU_WORK* work )
{
  int trg;
  int touch;
  int commonTouch;

  trg = GFL_UI_KEY_GetTrg();
  touch = GFL_UI_TP_HitTrg( work->touchHitTable );
  commonTouch = GFL_UI_TP_HitTrg( RESEARCH_COMMON_GetHitTable(work->commonWork) );

  //-----------------
  //「もどる」ボタン
  if( commonTouch == COMMON_TOUCH_AREA_RETURN_BUTTON ) {
    RESEARCH_COMMON_StartPaletteAnime( 
        work->commonWork, COMMON_PALETTE_ANIME_RETURN ); // 選択パレットアニメ開始
    PMSND_PlaySE( SEQ_SE_CANCEL1 );                      // キャンセル音
    SetResult( work, RESEARCH_MENU_RESULT_EXIT );        // 画面終了結果を決定
    SetNextSeq( work, RESEARCH_MENU_SEQ_FADE_OUT );
    SetNextSeq( work, RESEARCH_MENU_SEQ_CLEAN_UP );
    FinishCurrentSeq( work );
    return;
  }

  //--------
  // 上 キー
  if( trg & PAD_KEY_UP ) {
    MoveCursorUp( work );
  } 
  //--------
  // 下 キー
  if( trg & PAD_KEY_DOWN ) {
    MoveCursorDown( work );
  }

  //----------
  // A ボタン
  if( trg & PAD_BUTTON_A ) {
    StopPaletteAnime( work, PALETTE_ANIME_CURSOR_ON ); // カーソルONパレットアニメ終了
    StartPaletteAnime( work, PALETTE_ANIME_SELECT );   // 選択パレットアニメ開始
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                    // 決定音
    switch( work->cursorPos ) { // 画面終了結果を決定
    case MENU_ITEM_CHANGE_RESEARCH: SetResult( work, RESEARCH_MENU_RESULT_TO_SELECT ); break;
    case MENU_ITEM_CHECK_RESEARCH:  SetResult( work, RESEARCH_MENU_RESULT_TO_CHECK );  break;
    default: GF_ASSERT(0)
    }
    SetNextSeq( work, RESEARCH_MENU_SEQ_FADE_OUT );
    SetNextSeq( work, RESEARCH_MENU_SEQ_CLEAN_UP );
    FinishCurrentSeq( work );
    return;
  }

  //----------------------------
  //「調査内容を変更する」ボタン
  if( touch == TOUCH_AREA_CHANGE_BUTTON ) {
    MoveCursorDirect( work, MENU_ITEM_CHANGE_RESEARCH );  // カーソル位置を更新
    StopPaletteAnime( work, PALETTE_ANIME_CURSOR_ON );    // カーソルONパレットアニメ終了
    StartPaletteAnime( work, PALETTE_ANIME_SELECT );      // 選択パレットアニメ開始
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                       // 決定音
    SetResult( work, RESEARCH_MENU_RESULT_TO_SELECT );    // 画面終了結果を決定
    SetNextSeq( work, RESEARCH_MENU_SEQ_FADE_OUT );
    SetNextSeq( work, RESEARCH_MENU_SEQ_CLEAN_UP );
    FinishCurrentSeq( work );
    return;
  }
  //----------------------------
  //「調査報告を確認する」ボタン
  if( touch == TOUCH_AREA_CHECK_BUTTON ) {
    MoveCursorDirect( work, MENU_ITEM_CHECK_RESEARCH ); // カーソル位置を更新
    StopPaletteAnime( work, PALETTE_ANIME_CURSOR_ON );  // カーソルONパレットアニメ終了
    StartPaletteAnime( work, PALETTE_ANIME_SELECT );    // 選択パレットアニメ開始
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                     // 決定音
    SetResult( work, RESEARCH_MENU_RESULT_TO_CHECK );   // 画面終了結果を決定
    SetNextSeq( work, RESEARCH_MENU_SEQ_FADE_OUT );
    SetNextSeq( work, RESEARCH_MENU_SEQ_CLEAN_UP );
    FinishCurrentSeq( work );
    return;
  }

  //----------
  // B ボタン
  if( trg & PAD_BUTTON_B ) {
    PMSND_PlaySE( SEQ_SE_CANCEL1 );               // キャンセル音
    SetResult( work, RESEARCH_MENU_RESULT_EXIT ); // 画面終了結果を決定
    SetNextSeq( work, RESEARCH_MENU_SEQ_FADE_OUT );
    SetNextSeq( work, RESEARCH_MENU_SEQ_CLEAN_UP );
    FinishCurrentSeq( work );
    return;
  }

  // TEST:
  if( trg & PAD_BUTTON_L ) {
    DEBUG_GAMEBEACON_Set_NewEntry();
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_MENU_SEQ_FADE_OUT ) の処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void Main_FADE_OUT( RESEARCH_MENU_WORK* work )
{
  // フェードが終了
  if( GFL_FADE_CheckFade() == FALSE ) {
    FinishCurrentSeq( work );
  } 
}

//------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_MENU_SEQ_CLEAN_UP ) の処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void Main_CLEAN_UP( RESEARCH_MENU_WORK* work )
{
  // 共通パレットアニメーション
  RESEARCH_COMMON_StopAllPaletteAnime( work->commonWork ); // 停止して, 
  RESEARCH_COMMON_ResetAllPalette( work->commonWork );     // パレットを元に戻す

  // パレットアニメーション
  CleanUpPaletteAnime( work );
  DeletePaletteAnime( work );

  // OBJ 後片付け
  DeleteClactWorks( work );
  DeleteClactUnits( work );
  ReleaseSubOBJResources ( work );
  ReleaseMainOBJResources( work );
  DeleteClactSystem ( work );

  // 文字列描画オブジェクト 後片付け
  DeleteBGFonts( work );

  // BG 後片付け
  CleanUpMainBG_FONT  ( work );
  CleanUpMainBG_WINDOW( work );
  CleanUpSubBG_FONT   ( work );
  CleanUpSubBG_WINDOW ( work );
  CleanUpBG           ( work );

  DeleteMessages( work );
  DeleteFont( work );

  SetNextSeq( work, RESEARCH_MENU_SEQ_FINISH );
  FinishCurrentSeq( work );
}


//====================================================================================
// ■シーケンス制御
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief シーケンスカウンタを更新する
 * 
 * @param work
 */
//------------------------------------------------------------------------------------
static void CountUpSeqCount( RESEARCH_MENU_WORK* work )
{
  u32 maxCount;

  // インクリメント
  work->seqCount++;

  // 最大値を決定
  switch( work->seq ) {
  case RESEARCH_MENU_SEQ_SETUP:     maxCount = 0xffffffff;  break;
  case RESEARCH_MENU_SEQ_STAND_BY:  maxCount = 0xffffffff;  break;
  case RESEARCH_MENU_SEQ_KEY_WAIT:  maxCount = 0xffffffff;  break;
  case RESEARCH_MENU_SEQ_FADE_OUT:  maxCount = 0xffffffff;  break;
  case RESEARCH_MENU_SEQ_CLEAN_UP:  maxCount = 0xffffffff;  break;
  case RESEARCH_MENU_SEQ_FINISH:    maxCount = 0xffffffff;  break;
  default: GF_ASSERT(0);
  }

  // 最大値補正
  if( maxCount < work->seqCount )
  { 
    work->seqCount = maxCount;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief 次のシーケンスをキューに登録する
 *
 * @param work
 * @param nextSeq 登録するシーケンス
 */
//------------------------------------------------------------------------------------
static void SetNextSeq( RESEARCH_MENU_WORK* work, RESEARCH_MENU_SEQ nextSeq )
{
  // シーケンスキューに追加する
  QUEUE_EnQueue( work->seqQueue, nextSeq );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: set next seq\n" );
  DebugPrint_seqQueue( work );
}

//------------------------------------------------------------------------------------
/**
 * @brief 現在のシーケンスを終了する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void FinishCurrentSeq( RESEARCH_MENU_WORK* work )
{
  // すでに終了済み
  GF_ASSERT( work->seqFinishFlag == FALSE );

  // 終了フラグを立てる
  work->seqFinishFlag = TRUE;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: finish current sequence\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief シーケンスを変更する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SwitchSeq( RESEARCH_MENU_WORK* work )
{
  RESEARCH_MENU_SEQ nextSeq;

  if( work->seqFinishFlag == FALSE ){ return; }  // 現在のシーケンスが終了していない
  if( QUEUE_IsEmpty( work->seqQueue ) ){ return; } // シーケンスキューに登録されていない

  // 変更
  nextSeq = QUEUE_DeQueue( work->seqQueue );
  SetSeq( work, nextSeq ); 

  // DEBUG: シーケンスキューを表示
  DebugPrint_seqQueue( work );
} 

//------------------------------------------------------------------------------------
/**
 * @brief シーケンスを設定する
 *
 * @param work
 * @parma nextSeq 設定するシーケンス
 */
//------------------------------------------------------------------------------------
static void SetSeq( RESEARCH_MENU_WORK* work, RESEARCH_MENU_SEQ nextSeq )
{ 
  // シーケンスの終了処理
  switch( work->seq ) {
  case RESEARCH_MENU_SEQ_SETUP:    FinishSeq_SETUP( work );     break;
  case RESEARCH_MENU_SEQ_STAND_BY: FinishSeq_STAND_BY( work );  break;
  case RESEARCH_MENU_SEQ_KEY_WAIT: FinishSeq_KEY_WAIT( work );  break;
  case RESEARCH_MENU_SEQ_FADE_OUT: FinishSeq_FADE_OUT( work );  break;
  case RESEARCH_MENU_SEQ_CLEAN_UP: FinishSeq_CLEAN_UP( work );  break;
  case RESEARCH_MENU_SEQ_FINISH:   break;
  default:  GF_ASSERT(0);
  }

  // 更新
  work->seq           = nextSeq;
  work->seqCount      = 0;
  work->seqFinishFlag = FALSE;

  // シーケンスの初期化処理
  switch( nextSeq ) {
  case RESEARCH_MENU_SEQ_SETUP:    InitSeq_SETUP( work );     break;
  case RESEARCH_MENU_SEQ_STAND_BY: InitSeq_STAND_BY( work );  break;
  case RESEARCH_MENU_SEQ_KEY_WAIT: InitSeq_KEY_WAIT( work );  break;
  case RESEARCH_MENU_SEQ_FADE_OUT: InitSeq_FADE_OUT( work );  break;
  case RESEARCH_MENU_SEQ_CLEAN_UP: InitSeq_CLEAN_UP( work );  break;
  case RESEARCH_MENU_SEQ_FINISH:   break;
  default:  GF_ASSERT(0);
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: set seq ==> " );
  switch( nextSeq ) {
  case RESEARCH_MENU_SEQ_SETUP:    OS_TFPrintf( PRINT_TARGET, "SETUP" );     break;
  case RESEARCH_MENU_SEQ_STAND_BY: OS_TFPrintf( PRINT_TARGET, "STAND_BY" );  break;
  case RESEARCH_MENU_SEQ_KEY_WAIT: OS_TFPrintf( PRINT_TARGET, "KEY_WAIT" );  break;
  case RESEARCH_MENU_SEQ_FADE_OUT: OS_TFPrintf( PRINT_TARGET, "FADE_OUT" );  break;
  case RESEARCH_MENU_SEQ_CLEAN_UP: OS_TFPrintf( PRINT_TARGET, "CLEAN_UP" );  break;
  case RESEARCH_MENU_SEQ_FINISH:   OS_TFPrintf( PRINT_TARGET, "FINISH" );    break;
  default:  GF_ASSERT(0);
  }
  OS_TFPrintf( PRINT_TARGET, "\n" );
}

//====================================================================================
// ■データ更新
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 新しい人物とすれ違ったかどうかをチェックする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CheckNewEntry( RESEARCH_MENU_WORK* work )
{
  if( work->newEntryFlag == FALSE ) {
    // 新しい人物とすれ違った
    if( GAMEBEACON_Get_NewEntry() == TRUE ) {
      work->newEntryFlag = TRUE; // フラグを立てる
      NewIconDispOn( work );     // "new" アイコンを表示する
    }
  }
}

//====================================================================================
// ■カーソル移動
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief カーソルを上へ移動する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void MoveCursorUp( RESEARCH_MENU_WORK* work )
{
  int nowPos;
  int nextPos;

  // 項目の表示を更新
  MenuItemSetCursorOff( work->cursorPos );

  // カーソル位置を更新
  ShiftCursorPos( work, -1 );

  // 項目の表示を更新
  MenuItemSetCursorOn( work->cursorPos );

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );

  // パレットアニメーションを開始
  StartPaletteAnime( work, PALETTE_ANIME_CURSOR_SET );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: move cursor up\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief カーソルを下へ移動する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void MoveCursorDown( RESEARCH_MENU_WORK* work )
{
  // 項目の表示を更新
  MenuItemSetCursorOff( work->cursorPos );

  // カーソル位置を更新
  ShiftCursorPos( work, 1 );

  // 項目の表示を更新
  MenuItemSetCursorOn( work->cursorPos );

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );

  // パレットアニメーションを開始
  StartPaletteAnime( work, PALETTE_ANIME_CURSOR_SET );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: move cursor down\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief カーソルを直接移動する
 *
 * @param work
 * @param menuItem 移動先のメニュー項目
 */
//------------------------------------------------------------------------------------
static void MoveCursorDirect( RESEARCH_MENU_WORK* work, MENU_ITEM menuItem )
{
  // 項目の表示を更新
  MenuItemSetCursorOff( work->cursorPos );

  // カーソル位置を更新
  SetCursorPos( work, menuItem );

  // 項目の表示を更新
  MenuItemSetCursorOn( work->cursorPos );

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );

  // パレットアニメーションを開始
  StartPaletteAnime( work, PALETTE_ANIME_CURSOR_SET );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: move cursor direct\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief カーソル位置を変更する ( オフセット指定 )
 *
 * @param work
 * @param offset 移動量
 */
//------------------------------------------------------------------------------------
static void ShiftCursorPos( RESEARCH_MENU_WORK* work, int offset )
{
  int nowPos;
  int nextPos;

  // 更新後のカーソル位置を算出
  nowPos  = work->cursorPos;
  nextPos = (nowPos + offset + MENU_ITEM_NUM) % MENU_ITEM_NUM;

  // カーソル位置を更新
  work->cursorPos = nextPos;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: shift cursor pos ==> %d\n", nextPos );
}

//------------------------------------------------------------------------------------
/**
 * @brief カーソル位置を変更する ( 直値指定 )
 *
 * @param work
 * @param menuItem 移動先
 */
//------------------------------------------------------------------------------------
static void SetCursorPos( RESEARCH_MENU_WORK* work, MENU_ITEM menuItem )
{
  // カーソル位置を更新
  work->cursorPos = menuItem;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: set cursor pos ==> %d\n", menuItem );
}


//====================================================================================
// ■画面終了結果
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 画面の終了結果を決定する
 *
 * @param work
 * @param result 結果
 */
//------------------------------------------------------------------------------------
static void SetResult( RESEARCH_MENU_WORK* work, RESEARCH_MENU_RESULT result )
{
  // 多重設定
  GF_ASSERT( work->result == RESEARCH_MENU_RESULT_NONE );

  // 設定
  work->result = result;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: set result (%d)\n", result );
}


//====================================================================================
// ■フォント
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief フォントデータを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitFont( RESEARCH_MENU_WORK* work )
{
  work->font = NULL;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init font\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief フォントデータを生成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreateFont( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->font == NULL ); // 初期化されていない

  work->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, 
                                GFL_FONT_LOADTYPE_FILE, TRUE, work->heapID ); 
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create font\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief フォントデータを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeleteFont( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->font ); // 生成されていない

  GFL_FONT_Delete( work->font );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete font\n" );
}


//====================================================================================
// ■メッセージ
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitMessages( RESEARCH_MENU_WORK* work )
{
  int i;

  for( i=0; i < MESSAGE_NUM; i++ )
  {
    work->message[i] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init messages\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを作成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreateMessages( RESEARCH_MENU_WORK* work )
{
  int i;

  for( i=0; i < MESSAGE_NUM; i++ )
  {
    GF_ASSERT( work->message[i] == NULL ); // 初期化されていない

    work->message[i] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
                                       ARCID_MESSAGE, 
                                       MessageDataID[i],
                                       work->heapID ); 
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create messages\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeleteMessages( RESEARCH_MENU_WORK* work )
{
  int i;

  for( i=0; i < MESSAGE_NUM; i++ )
  {
    GF_ASSERT( work->message[i] ); // 生成されてない

    GFL_MSG_Delete( work->message[i] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete messages\n" );
}


//------------------------------------------------------------------------------------
/**
 * @brief タッチ領域をセットアップする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupTouchArea( RESEARCH_MENU_WORK* work )
{
  int i;

  for( i=0; i < TOUCH_AREA_NUM; i++ )
  {
    work->touchHitTable[i].rect.left   = TouchAreaInitData[i].left;
    work->touchHitTable[i].rect.right  = TouchAreaInitData[i].right;
    work->touchHitTable[i].rect.top    = TouchAreaInitData[i].top;
    work->touchHitTable[i].rect.bottom = TouchAreaInitData[i].bottom;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create touch hit table\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitSeqQueue( RESEARCH_MENU_WORK* work )
{
  work->seqQueue = NULL;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init seq queue\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューを作成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreateSeqQueue( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->seqQueue == NULL ); // 初期化されていない

  work->seqQueue = QUEUE_Create( SEQ_QUEUE_SIZE, work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create seq queue\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeleteSeqQueue( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->seqQueue ); // 生成されていない

  QUEUE_Delete( work->seqQueue );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete seq queue\n" );
}


//====================================================================================
// ■BG
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief BG の準備
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupBG( RESEARCH_MENU_WORK* work )
{ 
  // BG モード
  GFL_BG_SetBGMode( &BGSysHeader2D );

  // BG コントロール
  GFL_BG_SetBGControl( SUB_BG_WINDOW,  &SubBGControl_WINDOW,  GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( SUB_BG_FONT,    &SubBGControl_FONT,    GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( MAIN_BG_WINDOW, &MainBGControl_WINDOW, GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( MAIN_BG_FONT,   &MainBGControl_FONT,   GFL_BG_MODE_TEXT );

  // 可視設定
  GFL_BG_SetVisible( SUB_BG_BACK,    VISIBLE_ON );
  GFL_BG_SetVisible( SUB_BG_RADAR,   VISIBLE_ON );
  GFL_BG_SetVisible( SUB_BG_WINDOW,  VISIBLE_ON );
  GFL_BG_SetVisible( SUB_BG_FONT,    VISIBLE_ON );
  GFL_BG_SetVisible( MAIN_BG_BACK,   VISIBLE_ON );
  GFL_BG_SetVisible( MAIN_BG_WINDOW, VISIBLE_ON );
  GFL_BG_SetVisible( MAIN_BG_FONT,   VISIBLE_ON );

  // αブレンディング
  G2S_SetBlendAlpha( SUB_BG_BLEND_TARGET_1, SUB_BG_BLEND_TARGET_2, 
                     SUB_BG_BLEND_WEIGHT_1, SUB_BG_BLEND_WEIGHT_2 );
  G2_SetBlendAlpha( MAIN_BG_BLEND_TARGET_1, MAIN_BG_BLEND_TARGET_2, 
                    MAIN_BG_BLEND_WEIGHT_1, MAIN_BG_BLEND_WEIGHT_2 );

  // ビットマップウィンドウ システム初期化
  GFL_BMPWIN_Init( work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: setup BG\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief BG の後片付け
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CleanUpBG( RESEARCH_MENU_WORK* work )
{
  GFL_BMPWIN_Exit();

  GFL_BG_FreeBGControl( MAIN_BG_FONT );
  GFL_BG_FreeBGControl( MAIN_BG_WINDOW );
  GFL_BG_FreeBGControl( SUB_BG_FONT );
  GFL_BG_FreeBGControl( SUB_BG_WINDOW );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up BG\n" );
}


//====================================================================================
// ■上画面 ウィンドウBG面
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 上画面 ウィンドウBG面 準備
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupSubBG_WINDOW( RESEARCH_MENU_WORK* work )
{
  // データ読み込み
  {
    ARCHANDLE* handle;

    // ハンドルオープン
    handle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR_GRAPHIC, work->heapID ); 

    // スクリーンデータ
    {
      void* src;
      ARCDATID datID;
      NNSG2dScreenData* data;
      datID = NARC_research_radar_graphic_bgu_win1_NSCR;
      src   = GFL_ARC_LoadDataAllocByHandle( handle, datID, work->heapID );
      NNS_G2dGetUnpackedScreenData( src, &data );
      GFL_BG_WriteScreen( SUB_BG_WINDOW, data->rawData, 0, 0, 32, 24 );
      GFL_BG_LoadScreenReq( SUB_BG_WINDOW );
      GFL_HEAP_FreeMemory( src );
    }

    // ハンドルクローズ
    GFL_ARC_CloseDataHandle( handle );
  } 

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: setup SUB-BG-WINDOW\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief 上画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CleanUpSubBG_WINDOW( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up SUB-BG-WINDOW\n" );
}


//====================================================================================
// ■上画面 フォントBG面
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief SUB-BG フォント面の準備
 * 
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupSubBG_FONT( RESEARCH_MENU_WORK* work )
{
  // NULLキャラを確保
  GFL_BG_FillCharacter( SUB_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( SUB_BG_FONT );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: setup SUB-BG-FONT\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief SUB-BG フォント面の後片付け
 * 
 * @param work
 */
//------------------------------------------------------------------------------------
static void CleanUpSubBG_FONT( RESEARCH_MENU_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( SUB_BG_FONT, 1, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up SUB-BG-FONT\n" );
}


//====================================================================================
// ■下画面 ウィンドウBG面
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 準備
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupMainBG_WINDOW( RESEARCH_MENU_WORK* work )
{
  // データ読み込み
  {
    ARCHANDLE* handle;

    // ハンドルオープン
    handle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR_GRAPHIC, work->heapID ); 

    // スクリーンデータ
    {
      void* src;
      ARCDATID datID;
      NNSG2dScreenData* data;
      datID = NARC_research_radar_graphic_bgd_topbtn_NSCR;
      src   = GFL_ARC_LoadDataAllocByHandle( handle, datID, work->heapID );
      NNS_G2dGetUnpackedScreenData( src, &data );
      GFL_BG_WriteScreen( MAIN_BG_WINDOW, data->rawData, 0, 0, 32, 24 );
      GFL_BG_LoadScreenReq( MAIN_BG_WINDOW );
      GFL_HEAP_FreeMemory( src );
    }

    // ハンドルクローズ
    GFL_ARC_CloseDataHandle( handle );
  } 

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: setup MAIN-BG-WINDOW\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CleanUpMainBG_WINDOW( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up MAIN-BG-WINDOW\n" );
}


//====================================================================================
// ■下画面 フォントBG面
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 下画面 フォントBG面 準備
 * 
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupMainBG_FONT( RESEARCH_MENU_WORK* work )
{ 
  // NULLキャラ確保
  GFL_BG_FillCharacter( MAIN_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( MAIN_BG_FONT );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: setup MAIN-BG-FONT\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief 下画面 フォントBG面 後片付け
 * 
 * @param work
 */
//------------------------------------------------------------------------------------
static void CleanUpMainBG_FONT( RESEARCH_MENU_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( MAIN_BG_FONT, 1, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up MAIN-BG-FONT\n" );
}


//====================================================================================
// ■文字列描画オブジェクト
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitBGFonts( RESEARCH_MENU_WORK* work )
{
  int i;

  for( i=0; i < BG_FONT_NUM; i++ )
  {
    work->BGFont[i] = NULL; 
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを作成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreateBGFonts( RESEARCH_MENU_WORK* work )
{
  int i;

  // 文字列描画オブジェクト作成
  for( i=0; i<BG_FONT_NUM; i++ )
  {
    BG_FONT_PARAM param;
    GFL_MSGDATA* msgData;
    u32 strID;

    GF_ASSERT( work->BGFont[i] == NULL ); 

    // 生成パラメータ選択
    param.BGFrame       = BGFontInitData[i].BGFrame;
    param.posX          = BGFontInitData[i].posX;
    param.posY          = BGFontInitData[i].posY;
    param.sizeX         = BGFontInitData[i].sizeX;
    param.sizeY         = BGFontInitData[i].sizeY;
    param.offsetX       = BGFontInitData[i].offsetX;
    param.offsetY       = BGFontInitData[i].offsetY;
    param.paletteNo     = BGFontInitData[i].paletteNo;
    param.colorNo_L     = BGFontInitData[i].colorNo_L;
    param.colorNo_S     = BGFontInitData[i].colorNo_S;
    param.colorNo_B     = BGFontInitData[i].colorNo_B;
    param.centeringFlag = BGFontInitData[i].softCentering;
    msgData             = work->message[ BGFontInitData[i].messageIdx ];
    strID               = BGFontInitData[i].stringIdx;

    // 生成
    work->BGFont[i] = BG_FONT_Create( &param, work->font, msgData, work->heapID );

    // 文字列を設定
    BG_FONT_SetMessage( work->BGFont[i], strID );
  } 

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create BGFonts\n" ); 
}

//------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeleteBGFonts( RESEARCH_MENU_WORK* work )
{
  int i;
  
  // 文字列描画オブジェクト破棄
  for( i=0; i<BG_FONT_NUM; i++ )
  {
    GF_ASSERT( work->BGFont[i] );
    BG_FONT_Delete( work->BGFont[i] );
    work->BGFont[i] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete BGFonts\n" ); 
}


//====================================================================================
// ■メニュー項目操作
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @biref メニュー項目を選択状態にする
 *
 * @param menuItem 選択状態にするメニュー項目を指定
 */
//------------------------------------------------------------------------------------
static void MenuItemSetCursorOn( MENU_ITEM menuItem )
{
  u8 BGFrame;
  u8 x, y, width, height;
  u8 paletteNo;

  // パラメータを決定
  switch( menuItem )
  {
  //「調査内容を変更する」
  case MENU_ITEM_CHANGE_RESEARCH:
    BGFrame   = MAIN_BG_WINDOW;
    x         = CHANGE_BUTTON_X;
    y         = CHANGE_BUTTON_Y;
    width     = CHANGE_BUTTON_WIDTH;
    height    = CHANGE_BUTTON_HEIGHT;
    paletteNo = MAIN_BG_PALETTE_WINDOW_ON;
    break;

  //「調査報告を確認する」
  case MENU_ITEM_CHECK_RESEARCH:
    BGFrame   = MAIN_BG_WINDOW;
    x         = CHECK_BUTTON_X;
    y         = CHECK_BUTTON_Y;
    width     = CHECK_BUTTON_WIDTH;
    height    = CHECK_BUTTON_HEIGHT;
    paletteNo = MAIN_BG_PALETTE_WINDOW_ON;
    break;

  // エラー
  default:
    GF_ASSERT(0);
  }

  // スクリーン更新
  GFL_BG_ChangeScreenPalette( BGFrame, x, y, width, height, paletteNo );
  GFL_BG_LoadScreenReq( BGFrame );
}

//------------------------------------------------------------------------------------
/**
 * @biref メニュー項目を非選択状態にする
 *
 * @param menuItem 非選択状態にするメニュー項目を指定
 */
//------------------------------------------------------------------------------------
static void MenuItemSetCursorOff( MENU_ITEM menuItem )
{
  u8 BGFrame;
  u8 x, y, width, height;
  u8 paletteNo;

  // パラメータを決定
  switch( menuItem )
  {
  //「調査内容を変更する」
  case MENU_ITEM_CHANGE_RESEARCH:
    BGFrame   = MAIN_BG_WINDOW;
    x         = CHANGE_BUTTON_X;
    y         = CHANGE_BUTTON_Y;
    width     = CHANGE_BUTTON_WIDTH;
    height    = CHANGE_BUTTON_HEIGHT;
    paletteNo = MAIN_BG_PALETTE_WINDOW_OFF;
    break;

  //「調査報告を確認する」
  case MENU_ITEM_CHECK_RESEARCH:
    BGFrame   = MAIN_BG_WINDOW;
    x         = CHECK_BUTTON_X;
    y         = CHECK_BUTTON_Y;
    width     = CHECK_BUTTON_WIDTH;
    height    = CHECK_BUTTON_HEIGHT;
    paletteNo = MAIN_BG_PALETTE_WINDOW_OFF;
    break;

  // エラー
  default:
    GF_ASSERT(0);
  }

  // スクリーン更新
  GFL_BG_ChangeScreenPalette( BGFrame, x, y, width, height, paletteNo );
  GFL_BG_LoadScreenReq( BGFrame );
}

//====================================================================================
// ■OBJ
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターシステムを作成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreateClactSystem( RESEARCH_MENU_WORK* work )
{
  // システム作成
  //GFL_CLACT_SYS_Create( &ClactSystemInitData, &VRAMBankSettings, work->heapID );

  // VBlank 割り込み関数を登録
  GFUser_SetVIntrFunc( GFL_CLACT_SYS_VBlankFunc );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create clact system\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターシステムを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeleteClactSystem( RESEARCH_MENU_WORK* work )
{
  // VBkank 割り込み関数を解除
  GFUser_ResetVIntrFunc();

  // システム破棄
  //GFL_CLACT_SYS_Delete();

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete clact system\n" );
}


//====================================================================================
// ■SUB-OBJ リソース
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief OBJ のリソースを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitOBJResources( RESEARCH_MENU_WORK* work )
{
  int i;

  for( i=0; i<OBJ_RESOURCE_NUM; i++ )
  {
    work->objResRegisterIdx[i] = 0;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init OBJ resources\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief SUB-OBJ リソースを登録する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void RegisterSubOBJResources( RESEARCH_MENU_WORK* work )
{
  HEAPID heapID;
  ARCHANDLE* arcHandle;
  u32 character, palette, cellAnime;

  heapID    = work->heapID;
  arcHandle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR_GRAPHIC, heapID );

  // リソースを登録
  character = GFL_CLGRP_CGR_Register( arcHandle, 
                                      NARC_research_radar_graphic_obj_NCGR, 
                                      FALSE, CLSYS_DRAW_SUB, heapID ); 

  palette = GFL_CLGRP_PLTT_Register( arcHandle, 
                                     NARC_research_radar_graphic_obj_NCLR,
                                     CLSYS_DRAW_SUB, 0, heapID );

  cellAnime = GFL_CLGRP_CELLANIM_Register( arcHandle,
                                           NARC_research_radar_graphic_obj_NCER,
                                           NARC_research_radar_graphic_obj_NANR,
                                           heapID ); 
  // 登録インデックスを記憶
  work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CHARACTER ]  = character;
  work->objResRegisterIdx[ OBJ_RESOURCE_SUB_PALETTE ]    = palette;
  work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CELL_ANIME ] = cellAnime;

  GFL_ARC_CloseDataHandle( arcHandle );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: register SUB-OBJ resources\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief SUB-OBJ リソースを解放する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void ReleaseSubOBJResources( RESEARCH_MENU_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CELL_ANIME ] );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: release SUB-OBJ resources\n" );
}


//====================================================================================
// ■MAIN-OBJ リソース
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief MAIN-OBJ リソースを登録する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void RegisterMainOBJResources( RESEARCH_MENU_WORK* work )
{
  HEAPID heapID;
  ARCHANDLE* arcHandle;
  u32 character, palette, cellAnime;

  heapID    = work->heapID;
  arcHandle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR_GRAPHIC, heapID );

  // リソースを登録
  character = GFL_CLGRP_CGR_Register( arcHandle, 
                                      NARC_research_radar_graphic_obj_NCGR, 
                                      FALSE, CLSYS_DRAW_MAIN, heapID ); 

  palette = GFL_CLGRP_PLTT_RegisterEx( arcHandle, 
                                       NARC_research_radar_graphic_obj_NCLR,
                                       CLSYS_DRAW_MAIN, 
                                       ONE_PALETTE_SIZE*6, 0, 3, 
                                       heapID );

  cellAnime = GFL_CLGRP_CELLANIM_Register( arcHandle,
                                           NARC_research_radar_graphic_obj_NCER,
                                           NARC_research_radar_graphic_obj_NANR,
                                           heapID ); 
  // 登録インデックスを記憶
  work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CHARACTER ]  = character;
  work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_PALETTE ]    = palette;
  work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CELL_ANIME ] = cellAnime;

  GFL_ARC_CloseDataHandle( arcHandle );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: register MAIN-OBJ resources\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief MAIN-OBJ リソースを解放する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void ReleaseMainOBJResources( RESEARCH_MENU_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CELL_ANIME ] );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: release MAIN-OBJ resources\n" );
}


//====================================================================================
// ■セルアクターユニット
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitClactUnits( RESEARCH_MENU_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < CLUNIT_NUM; unitIdx++ )
  {
    work->clactUnit[ unitIdx ] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init clact units\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを作成する
 *
 * @param
 */
//------------------------------------------------------------------------------------
static void CreateClactUnits( RESEARCH_MENU_WORK* work )
{
  int unitIdx;
  u16 workNum;
  u8 priority;

  for( unitIdx=0; unitIdx < CLUNIT_NUM; unitIdx++ )
  {
    GF_ASSERT( work->clactUnit[ unitIdx ] == NULL );

    workNum  = ClactUnitWorkSize[ unitIdx ];
    priority = ClactUnitPriority[ unitIdx ];
    work->clactUnit[ unitIdx ] = GFL_CLACT_UNIT_Create( workNum, priority, work->heapID );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create clact units\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを破棄する
 *
 * @param
 */
//------------------------------------------------------------------------------------
static void DeleteClactUnits( RESEARCH_MENU_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < CLUNIT_NUM; unitIdx++ )
  {
    GF_ASSERT( work->clactUnit[ unitIdx ] );
    GFL_CLACT_UNIT_Delete( work->clactUnit[ unitIdx ] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete clact units\n" );
}


//====================================================================================
// □セルアクターワーク
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitClactWorks( RESEARCH_MENU_WORK* work )
{
  int wkIdx;

  // 初期化
  for( wkIdx=0; wkIdx < CLWK_NUM; wkIdx++ )
  {
    work->clactWork[ wkIdx ] = NULL;
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init clact works\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを作成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreateClactWorks( RESEARCH_MENU_WORK* work )
{
  int wkIdx;

  for( wkIdx=0; wkIdx < CLWK_NUM; wkIdx++ )
  {
    GFL_CLUNIT* unit;
    GFL_CLWK_DATA wkData;
    u32 charaIdx, paletteIdx, cellAnimeIdx;
    u16 surface;

    // 多重生成
    GF_ASSERT( work->clactWork[ wkIdx ] == NULL );

    // 生成パラメータ選択
    wkData.pos_x   = ClactWorkInitData[ wkIdx ].posX;
    wkData.pos_y   = ClactWorkInitData[ wkIdx ].posY;
    wkData.anmseq  = ClactWorkInitData[ wkIdx ].animeSeq;
    wkData.softpri = ClactWorkInitData[ wkIdx ].softPriority; 
    wkData.bgpri   = ClactWorkInitData[ wkIdx ].BGPriority; 
    unit           = GetClactUnit( work, ClactWorkInitData[ wkIdx ].unitIdx );
    charaIdx       = GetOBJResRegisterIndex( work, ClactWorkInitData[ wkIdx ].characterResID );
    paletteIdx     = GetOBJResRegisterIndex( work, ClactWorkInitData[ wkIdx ].paletteResID );
    cellAnimeIdx   = GetOBJResRegisterIndex( work, ClactWorkInitData[ wkIdx ].cellAnimeResID );
    surface        = ClactWorkInitData[ wkIdx ].setSurface;

    // 生成
    work->clactWork[ wkIdx ] = GFL_CLACT_WK_Create( 
        unit, charaIdx, paletteIdx, cellAnimeIdx, &wkData, surface, work->heapID );

    // 非表示に設定
    GFL_CLACT_WK_SetDrawEnable( work->clactWork[ wkIdx ], FALSE );
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create clact works\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeleteClactWorks( RESEARCH_MENU_WORK* work )
{
  int wkIdx;

  for( wkIdx=0; wkIdx < CLWK_NUM; wkIdx++ )
  {
    // 生成されていない
    GF_ASSERT( work->clactWork[ wkIdx ] );

    // 破棄
    GFL_CLACT_WK_Remove( work->clactWork[ wkIdx ] );
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete clact works\n" );
}


//====================================================================================
// ■OBJ アクセス
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief OBJ リソースの登録インデックスを取得する
 *
 * @param work
 * @param resID リソースID
 *
 * @return 指定したリソースの登録インデックス
 */
//------------------------------------------------------------------------------------
static u32 GetOBJResRegisterIndex( const RESEARCH_MENU_WORK* work, OBJ_RESOURCE_ID resID )
{
  return work->objResRegisterIdx[ resID ];
}

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを取得する
 *
 * @param work
 * @param unitIdx セルアクターユニットのインデックス
 *
 * @return 指定したセルアクターユニット
 */
//------------------------------------------------------------------------------------
static GFL_CLUNIT* GetClactUnit( const RESEARCH_MENU_WORK* work, CLUNIT_INDEX unitIdx )
{
  return work->clactUnit[ unitIdx ];
}

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを取得する
 *
 * @param work
 * @param unitIdx セルアクターワークのインデックス
 *
 * @return 指定したセルアクターワーク
 */
//------------------------------------------------------------------------------------
static GFL_CLWK* GetClactWork( const RESEARCH_MENU_WORK* work, CLWK_INDEX wkIdx )
{
  return work->clactWork[ wkIdx ];
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitPaletteAnime( RESEARCH_MENU_WORK* work )
{
  int i;

  for( i=0; i < PALETTE_ANIME_NUM; i++ )
  {
    work->paletteAnime[i] = NULL;
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init palette anime\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを生成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreatePaletteAnime( RESEARCH_MENU_WORK* work )
{
  int i;

  for( i=0; i < PALETTE_ANIME_NUM; i++ )
  {
    GF_ASSERT( work->paletteAnime[i] == NULL ); // 初期化されていない

    work->paletteAnime[i] = PALETTE_ANIME_Create( work->heapID );
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create palette anime\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeletePaletteAnime( RESEARCH_MENU_WORK* work )
{
  int i;

  for( i=0; i < PALETTE_ANIME_NUM; i++ )
  {
    GF_ASSERT( work->paletteAnime[i] );

    PALETTE_ANIME_Delete( work->paletteAnime[i] );
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete palette anime\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークをセットアップする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupPaletteAnime( RESEARCH_MENU_WORK* work )
{
  int i;

  for( i=0; i < PALETTE_ANIME_NUM; i++ )
  {
    GF_ASSERT( work->paletteAnime[i] );

    PALETTE_ANIME_Setup( work->paletteAnime[i],
                         PaletteAnimeData[i].destAdrs,
                         PaletteAnimeData[i].srcAdrs,
                         PaletteAnimeData[i].colorNum);
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: setup palette anime\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークをクリーンアップする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CleanUpPaletteAnime( RESEARCH_MENU_WORK* work )
{
  int i;

  for( i=0; i < PALETTE_ANIME_NUM; i++ )
  {
    GF_ASSERT( work->paletteAnime[i] );

    // 操作していたパレットを元に戻す
    PALETTE_ANIME_Reset( work->paletteAnime[i] );
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up palette anime\n" );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを開始する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void StartPaletteAnime( RESEARCH_MENU_WORK* work, PALETTE_ANIME_INDEX index )
{
  PALETTE_ANIME_Start( work->paletteAnime[ index ], 
                       PaletteAnimeData[ index ].animeType,
                       PaletteAnimeData[ index ].fadeColor );
  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: start palette anime [%d]\n", index );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを停止する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void StopPaletteAnime( RESEARCH_MENU_WORK* work, PALETTE_ANIME_INDEX index )
{
  PALETTE_ANIME_Stop( work->paletteAnime[ index ] );

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: stop palette anime [%d]\n", index );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを更新する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void UpdatePaletteAnime( RESEARCH_MENU_WORK* work )
{
  int i;

  for( i=0; i < PALETTE_ANIME_NUM; i++ )
  {
    GF_ASSERT( work->paletteAnime[i] );

    PALETTE_ANIME_Update( work->paletteAnime[i] );
  }
}



//====================================================================================
// ■"new" アイコン
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * "new" アイコンを表示する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void NewIconDispOn( const RESEARCH_MENU_WORK* work )
{
  GFL_CLWK* clactWork;

  clactWork = GetClactWork( work, CLWK_NEW_ICON );
  GFL_CLACT_WK_SetDrawEnable( clactWork, TRUE );
  GFL_CLACT_WK_SetAutoAnmFlag( clactWork, TRUE );
  GFL_CLACT_WK_SetAutoAnmSpeed( clactWork, FX32_ONE );
  GFL_CLACT_WK_SetAnmFrame( clactWork, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: new icon disp on\n" );
}

//------------------------------------------------------------------------------------
/**
 * "new" アイコンを非表示にする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void NewIconDispOff( const RESEARCH_MENU_WORK* work )
{
  GFL_CLWK* clactWork;

  clactWork = GetClactWork( work, CLWK_NEW_ICON );
  GFL_CLACT_WK_SetDrawEnable( clactWork, FALSE );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: new icon disp off\n" );
}


//------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューの中身を表示する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DebugPrint_seqQueue( const RESEARCH_MENU_WORK* work )
{
  int i;
  int dataNum;
  int value;

  // キュー内のデータの個数を取得
  dataNum = QUEUE_GetDataNum( work->seqQueue );

  // 全てのデータを出力
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: seq queue = " );
  for( i=0; i < dataNum; i++ )
  { 
    value = QUEUE_PeekData( work->seqQueue, i );
    
    switch( value ) {
    case RESEARCH_MENU_SEQ_SETUP:    OS_TFPrintf( PRINT_TARGET, "SETUP " );    break;
    case RESEARCH_MENU_SEQ_STAND_BY: OS_TFPrintf( PRINT_TARGET, "STAND_BY " ); break;
    case RESEARCH_MENU_SEQ_KEY_WAIT: OS_TFPrintf( PRINT_TARGET, "KEY-WAIT " ); break;
    case RESEARCH_MENU_SEQ_FADE_OUT: OS_TFPrintf( PRINT_TARGET, "FADE_OUT " ); break;
    case RESEARCH_MENU_SEQ_CLEAN_UP: OS_TFPrintf( PRINT_TARGET, "CLEAN-UP " ); break;
    case RESEARCH_MENU_SEQ_FINISH:   OS_TFPrintf( PRINT_TARGET, "FINISH " );   break;
    default: GF_ASSERT(0);
    }
  }
  OS_TFPrintf( PRINT_TARGET, "\n" );
} 
