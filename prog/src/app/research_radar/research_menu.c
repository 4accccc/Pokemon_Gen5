////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 初期画面 ( メニュー画面 )
 * @file   research_menu.c
 * @author obata
 * @date   2010.02.03
 */
////////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "bg_font.h"
#include "queue.h"
#include "research_menu.h"
#include "research_menu_index.h"
#include "research_menu_def.h"
#include "research_menu_data.cdat"
#include "research_common.h"

#include "print/gf_font.h"          // for GFL_FONT_xxxx
#include "print/printsys.h"         // for PRINTSYS_xxxx
#include "system/gfl_use.h"         // for GFUser_xxxx
#include "gamesystem/game_beacon.h" // for GAMEBEACON_xxxx
#include "sound/pm_sndsys.h"        // for PMSND_xxxx
#include "sound/wb_sound_data.sadl" // for SEQ_SE_XXXX

#include "system/main.h"                    // for HEAPID_xxxx
#include "arc/arc_def.h"                    // for ARCID_xxxx
#include "arc/research_radar_graphic.naix"  // for NARC_research_radar_xxxx
#include "arc/font/font.naix"               // for NARC_font_xxxx
#include "arc/message.naix"                 // for NARC_message_xxxx
#include "msg/msg_research_radar.h"         // for str_xxxx
#include "obj_NANR_LBLDEFS.h"               // for NANR_obj_xxxx


//==============================================================================================
// ■調査初期画面 ワーク
//==============================================================================================
struct _RESEARCH_MENU_WORK
{ 
  HEAPID       heapID;  // ヒープID
  GFL_FONT*    font;    // フォント
  GFL_MSGDATA* message[ MESSAGE_NUM ]; // メッセージ

  QUEUE*               seqQueue;      // シーケンスキュー
  RESEARCH_MENU_SEQ    seq;           // 処理シーケンス
  u32                  seqCount;      // シーケンスカウンタ
  BOOL                 seqFinishFlag; // 現在のシーケンスが終了したかどうか
  RESEARCH_MENU_RESULT result;        // 画面終了結果

  MENU_ITEM cursorPos;     // カーソル位置
  BOOL      newEntryFlag;  // 新しい人物とすれ違ったかどうか

  // タッチ領域
  GFL_UI_TP_HITTBL touchHitTable[ TOUCH_AREA_NUM ];

  // 文字列描画オブジェクト
  BG_FONT* BGFont[ BG_FONT_NUM ];

  // OBJ
  u32         objResRegisterIdx[ OBJ_RESOURCE_NUM ];  // リソースの登録インデックス
  GFL_CLUNIT* clactUnit[ CLUNIT_NUM ];                // セルアクターユニット
  GFL_CLWK*   clactWork[ CLWK_NUM ];                  // セルアクターワーク
};


//==============================================================================================
// ■非公開関数
//==============================================================================================
// シーケンス処理
static void Main_SETUP   ( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_SETUP
static void Main_STAND_BY( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_STAND_BY
static void Main_KEY_WAIT( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_WAIT
static void Main_FADE_OUT( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_FADE_OUT
static void Main_CLEAN_UP( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_CLEAN_UP

// シーケンス制御
static void CountUpSeqCount( RESEARCH_MENU_WORK* work ); // シーケンスカウンタを更新する
static void SetNextSequence( RESEARCH_MENU_WORK* work, RESEARCH_MENU_SEQ nextSeq ); // 次のシーケンスを登録する
static void FinishCurrentSequence( RESEARCH_MENU_WORK* work ); // 現在のシーケンスを終了する
static void SwitchSequence( RESEARCH_MENU_WORK* work ); // 処理シーケンスを変更する
static void SetSequence( RESEARCH_MENU_WORK* work, RESEARCH_MENU_SEQ nextSeq ); // 処理シーケンスを設定する
static void SetResult( RESEARCH_MENU_WORK* work, RESEARCH_MENU_RESULT result ); // 画面終了結果を設定する

// シーケンス初期化処理
static void InitSequence_SETUP   ( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_SETUP
static void InitSequence_STAND_BY( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_STAND_BY
static void InitSequence_KEY_WAIT( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_WAIT
static void InitSequence_FADE_OUT( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_FADE_OUT
static void InitSequence_CLEAN_UP( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_CLEAN_UP

// シーケンス終了処理
static void FinishSequence_SETUP   ( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_SETUP
static void FinishSequence_STAND_BY( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_STAND_BY
static void FinishSequence_KEY_WAIT( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_WAIT
static void FinishSequence_FADE_OUT( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_FADE_OUT
static void FinishSequence_CLEAN_UP( RESEARCH_MENU_WORK* work ); // RESEARCH_MENU_SEQ_CLEAN_UP

// データ更新
static void CheckNewEntry( RESEARCH_MENU_WORK* work );
// カーソル移動
static void MoveCursorUp  ( RESEARCH_MENU_WORK* work );
static void MoveCursorDown( RESEARCH_MENU_WORK* work );

// フォント
static void InitFont  ( RESEARCH_MENU_WORK* work );
static void CreateFont( RESEARCH_MENU_WORK* work );
static void DeleteFont( RESEARCH_MENU_WORK* work );
// メッセージ
static void InitMessages  ( RESEARCH_MENU_WORK* work );
static void CreateMessages( RESEARCH_MENU_WORK* work );
static void DeleteMessages( RESEARCH_MENU_WORK* work );
// タッチ領域
static void SetupTouchArea( RESEARCH_MENU_WORK* work );

static void InitSeqQueue  ( RESEARCH_MENU_WORK* work ); // シーケンスキュー 初期化
static void CreateSeqQueue( RESEARCH_MENU_WORK* work ); // シーケンスキュー 作成
static void DeleteSeqQueue( RESEARCH_MENU_WORK* work ); // シーケンスキュー 破棄

// BG
static void SetupBG  ( RESEARCH_MENU_WORK* work );
static void CleanUpBG( RESEARCH_MENU_WORK* work );
// SUB-BG ( ウィンドウ面 )
static void SetupSubBG_WINDOW  ( RESEARCH_MENU_WORK* work );
static void CleanUpSubBG_WINDOW( RESEARCH_MENU_WORK* work );
// SUB-BG ( フォント面 )
static void SetupSubBG_FONT  ( RESEARCH_MENU_WORK* work );
static void CleanUpSubBG_FONT( RESEARCH_MENU_WORK* work );
// MAIN-BG ( ウィンドウ面 )
static void SetupMainBG_WINDOW  ( RESEARCH_MENU_WORK* work );
static void CleanUpMainBG_WINDOW( RESEARCH_MENU_WORK* work );
// MAIN-BG ( フォント面 )
static void SetupMainBG_FONT  ( RESEARCH_MENU_WORK* work );
static void CleanUpMainBG_FONT( RESEARCH_MENU_WORK* work );

// 文字列描画オブジェクト
static void InitBGFonts  ( RESEARCH_MENU_WORK* work );
static void CreateBGFonts( RESEARCH_MENU_WORK* work );
static void DeleteBGFonts( RESEARCH_MENU_WORK* work );

// メニュー項目操作
static void MenuItemSwitchOn ( MENU_ITEM menuItem );
static void MenuItemSwitchOff( MENU_ITEM menuItem );

// OBJ システム
static void CreateClactSystem( RESEARCH_MENU_WORK* work );
static void DeleteClactSystem( RESEARCH_MENU_WORK* work );
// SUB-OBJ リソース
static void RegisterSubObjResources( RESEARCH_MENU_WORK* work );
static void ReleaseSubObjResources ( RESEARCH_MENU_WORK* work );
// MAIN-OBJ リソース
static void RegisterMainObjResources( RESEARCH_MENU_WORK* work );
static void ReleaseMainObjResources ( RESEARCH_MENU_WORK* work );
// セルアクターユニット
static void InitClactUnits  ( RESEARCH_MENU_WORK* work );
static void CreateClactUnits( RESEARCH_MENU_WORK* work );
static void DeleteClactUnits( RESEARCH_MENU_WORK* work );
// セルアクターワーク
static void InitClactWorks  ( RESEARCH_MENU_WORK* work );
static void CreateClactWorks( RESEARCH_MENU_WORK* work );
static void DeleteClactWorks( RESEARCH_MENU_WORK* work );
// OBJ アクセス
static u32 GetObjResourceRegisterIndex( const RESEARCH_MENU_WORK* work, OBJ_RESOURCE_ID resID );
static GFL_CLUNIT* GetClactUnit( const RESEARCH_MENU_WORK* work, CLUNIT_INDEX unitIdx );
static GFL_CLWK*   GetClactWork( const RESEARCH_MENU_WORK* work, CLWK_INDEX wkIdx );

// "new" アイコン
static void NewIconDispOn ( const RESEARCH_MENU_WORK* work );
static void NewIconDispOff( const RESEARCH_MENU_WORK* work );

// デバッグ
static void DebugPrint_seqQueue( const RESEARCH_MENU_WORK* work );


//==============================================================================================
// □調査初期画面 制御関数
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査初期画面ワークの生成
 *
 * @param heapID
 */
//----------------------------------------------------------------------------------------------
RESEARCH_MENU_WORK* CreateResearchMenuWork( HEAPID heapID )
{
  int i;
  RESEARCH_MENU_WORK* work;

  // 生成
  work = GFL_HEAP_AllocMemory( heapID, sizeof(RESEARCH_MENU_WORK) );

  // 初期化
  work->seq          = RESEARCH_MENU_SEQ_SETUP;
  work->seqFinishFlag= FALSE;
  work->seqCount     = 0;
  work->result       = RESEARCH_MENU_RESULT_NONE;
  work->cursorPos    = MENU_ITEM_CHANGE_RESEARCH;
  work->newEntryFlag = FALSE;
  work->heapID       = heapID;

  for( i=0; i<OBJ_RESOURCE_NUM; i++ ){ work->objResRegisterIdx[i] = 0; }

  InitSeqQueue( work );
  InitMessages( work );
  InitFont( work );
  InitBGFonts( work );
  InitClactUnits( work );
  InitClactWorks( work );

  CreateSeqQueue( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create work\n" );

  return work;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査初期画面ワークの破棄
 *
 * @param heapID
 */
//----------------------------------------------------------------------------------------------
void DeleteResearchMenuWork( RESEARCH_MENU_WORK* work )
{
  if( work == NULL )
  {
    GF_ASSERT(0);
    return;
  }
  DeleteSeqQueue( work ); // シーケンスキュー
  GFL_HEAP_FreeMemory( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete work\n" );
} 

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査初期画面 メイン動作
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
RESEARCH_MENU_RESULT ResearchMenuMain( RESEARCH_MENU_WORK* work )
{
  // シーケンスごとの処理
  switch( work->seq )
  {
  case RESEARCH_MENU_SEQ_SETUP:     Main_SETUP   ( work );  break;
  case RESEARCH_MENU_SEQ_STAND_BY:  Main_STAND_BY( work );  break;
  case RESEARCH_MENU_SEQ_KEY_WAIT:  Main_KEY_WAIT( work );  break;
  case RESEARCH_MENU_SEQ_FADE_OUT:  Main_FADE_OUT( work );  break;
  case RESEARCH_MENU_SEQ_CLEAN_UP:  Main_CLEAN_UP( work );  break;
  case RESEARCH_MENU_SEQ_FINISH:    return work->result;
  default:  GF_ASSERT(0);
  }

  // データ更新
  CheckNewEntry( work );

  // セルアクターシステム メイン処理
  GFL_CLACT_SYS_Main();

  // シーケンスカウンタ更新
  CountUpSeqCount( work ); 

  // シーケンス更新
  SwitchSequence( work );

  return RESEARCH_MENU_RESULT_CONTINUE;
}


//==============================================================================================
// ■シーケンス処理
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief 準備シーケンス ( RESEARCH_MENU_SEQ_SETUP ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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
  RegisterSubObjResources( work );
  RegisterMainObjResources( work );
  CreateClactUnits( work );
  CreateClactWorks( work );

  // 画面フェードイン
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 0);

  // 次のシーケンスをセット
  SetNextSequence( work, RESEARCH_MENU_SEQ_STAND_BY ); 

  // シーケンス終了
  FinishCurrentSequence( work );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 準備シーケンス ( RESEARCH_MENU_SEQ_STAND_BY ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_STAND_BY( RESEARCH_MENU_WORK* work )
{
  int trg;
  int touchedAreaIdx;

  trg            = GFL_UI_KEY_GetTrg();
  touchedAreaIdx = GFL_UI_TP_HitTrg( work->touchHitTable );

  //--------------------
  // 十字キー or A or B
  if( (trg & PAD_KEY_UP) ||
      (trg & PAD_KEY_DOWN) ||
      (trg & PAD_KEY_LEFT) ||
      (trg & PAD_KEY_RIGHT) ||
      (trg & PAD_BUTTON_A) || 
      (trg & PAD_BUTTON_B) ) {
    SetNextSequence( work, RESEARCH_MENU_SEQ_KEY_WAIT );
    FinishCurrentSequence( work );
  }

  //-------------------------------------
  //「調査内容を変更する」ボタンをタッチ
  if( touchedAreaIdx == TOUCH_AREA_CHANGE_BUTTON ) {
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                    // 決定音
    SetResult( work, RESEARCH_MENU_RESULT_TO_SELECT ); // 画面終了結果を決定
    SetNextSequence( work, RESEARCH_MENU_SEQ_FADE_OUT );
    SetNextSequence( work, RESEARCH_MENU_SEQ_CLEAN_UP );
    FinishCurrentSequence( work );
    return;
  }
  //-------------------------------------
  //「調査報告を確認する」ボタンをタッチ
  if( touchedAreaIdx == TOUCH_AREA_CHECK_BUTTON ) {
    MoveCursorDown( work );
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                    // 決定音
    SetResult( work, RESEARCH_MENU_RESULT_TO_CHECK );  // 画面終了結果を決定
    SetNextSequence( work, RESEARCH_MENU_SEQ_FADE_OUT );
    SetNextSequence( work, RESEARCH_MENU_SEQ_CLEAN_UP );
    FinishCurrentSequence( work );
    return;
  }

  //------------------
  // L ボタン (DEBUG)
  if( trg & PAD_BUTTON_L ) {
    DEBUG_GAMEBEACON_Set_NewEntry();
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief キー入力待ちシーケンス ( RESEARCH_MENU_SEQ_KEY_WAIT ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_KEY_WAIT( RESEARCH_MENU_WORK* work )
{
  int trg;
  int touchedAreaIdx;

  trg            = GFL_UI_KEY_GetTrg();
  touchedAreaIdx = GFL_UI_TP_HitTrg( work->touchHitTable );

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

  //----------------------------
  //「調査内容を変更する」ボタン
  if( ( (trg & PAD_BUTTON_A) && (work->cursorPos == MENU_ITEM_CHANGE_RESEARCH) ) ||
      ( touchedAreaIdx == TOUCH_AREA_CHANGE_BUTTON ) ) {
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                    // 決定音
    SetResult( work, RESEARCH_MENU_RESULT_TO_SELECT ); // 画面終了結果を決定
    SetNextSequence( work, RESEARCH_MENU_SEQ_FADE_OUT );
    SetNextSequence( work, RESEARCH_MENU_SEQ_CLEAN_UP );
    FinishCurrentSequence( work );
    return;
  }
  //----------------------------
  //「調査報告を確認する」ボタン
  if( ( (trg & PAD_BUTTON_A) && (work->cursorPos == MENU_ITEM_CHECK_RESEARCH) ) ||
      ( touchedAreaIdx == TOUCH_AREA_CHECK_BUTTON ) ) {
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                   // 決定音
    SetResult( work, RESEARCH_MENU_RESULT_TO_CHECK ); // 画面終了結果を決定
    SetNextSequence( work, RESEARCH_MENU_SEQ_FADE_OUT );
    SetNextSequence( work, RESEARCH_MENU_SEQ_CLEAN_UP );
    FinishCurrentSequence( work );
    return;
  }

  //----------
  // B ボタン
  if( trg & PAD_BUTTON_B ) {
    PMSND_PlaySE( SEQ_SE_CANCEL1 );               // キャンセル音
    SetResult( work, RESEARCH_MENU_RESULT_EXIT ); // 画面終了結果を決定
    SetNextSequence( work, RESEARCH_MENU_SEQ_FADE_OUT );
    SetNextSequence( work, RESEARCH_MENU_SEQ_CLEAN_UP );
    FinishCurrentSequence( work );
    return;
  }

  //------------------
  // L ボタン (DEBUG)
  if( trg & PAD_BUTTON_L ) {
    DEBUG_GAMEBEACON_Set_NewEntry();
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_MENU_SEQ_FADE_OUT ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_FADE_OUT( RESEARCH_MENU_WORK* work )
{
  // フェードが終了
  if( GFL_FADE_CheckFade() == FALSE ) {
    FinishCurrentSequence( work );
  } 
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_MENU_SEQ_CLEAN_UP ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_CLEAN_UP( RESEARCH_MENU_WORK* work )
{
  // OBJ 後片付け
  DeleteClactWorks( work );
  DeleteClactUnits( work );
  ReleaseSubObjResources ( work );
  ReleaseMainObjResources( work );
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

  SetNextSequence( work, RESEARCH_MENU_SEQ_FINISH );
  FinishCurrentSequence( work );
}


//==============================================================================================
// ■シーケンス制御
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスカウンタを更新する
 * 
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CountUpSeqCount( RESEARCH_MENU_WORK* work )
{
  u32 maxCount;

  // インクリメント
  work->seqCount++;

  // 最大値を決定
  switch( work->seq )
  {
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

//----------------------------------------------------------------------------------------------
/**
 * @brief 次のシーケンスを登録する
 *
 * @param work
 * @param nextSeq 登録するシーケンス
 */
//----------------------------------------------------------------------------------------------
static void SetNextSequence( RESEARCH_MENU_WORK* work, RESEARCH_MENU_SEQ nextSeq )
{
  // シーケンスキューに追加する
  QUEUE_EnQueue( work->seqQueue, nextSeq );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: set next seq\n" );
  DebugPrint_seqQueue( work );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在のシーケンスを終了する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishCurrentSequence( RESEARCH_MENU_WORK* work )
{
  // すでに終了済み
  GF_ASSERT( work->seqFinishFlag == FALSE );

  // 終了フラグを立てる
  work->seqFinishFlag = TRUE;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: finish current sequence\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを変更する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SwitchSequence( RESEARCH_MENU_WORK* work )
{
  RESEARCH_MENU_SEQ nextSeq;

  if( work->seqFinishFlag == FALSE ){ return; }  // 現在のシーケンスが終了していない
  if( QUEUE_IsEmpty( work->seqQueue ) ){ return; } // シーケンスキューに登録されていない

  // 変更
  nextSeq = QUEUE_DeQueue( work->seqQueue );
  SetSequence( work, nextSeq ); 

  // DEBUG: シーケンスキューを表示
  DebugPrint_seqQueue( work );
} 

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを設定する
 *
 * @param work
 * @parma nextSeq 設定するシーケンス
 */
//----------------------------------------------------------------------------------------------
static void SetSequence( RESEARCH_MENU_WORK* work, RESEARCH_MENU_SEQ nextSeq )
{ 
  // シーケンスの終了処理
  switch( work->seq )
  {
  case RESEARCH_MENU_SEQ_SETUP:    FinishSequence_SETUP( work );     break;
  case RESEARCH_MENU_SEQ_STAND_BY: FinishSequence_STAND_BY( work );  break;
  case RESEARCH_MENU_SEQ_KEY_WAIT: FinishSequence_KEY_WAIT( work );  break;
  case RESEARCH_MENU_SEQ_FADE_OUT: FinishSequence_FADE_OUT( work );  break;
  case RESEARCH_MENU_SEQ_CLEAN_UP: FinishSequence_CLEAN_UP( work );  break;
  case RESEARCH_MENU_SEQ_FINISH:   break;
  default:  GF_ASSERT(0);
  }

  // 更新
  work->seq           = nextSeq;
  work->seqCount      = 0;
  work->seqFinishFlag = FALSE;

  // シーケンスの初期化処理
  switch( nextSeq )
  {
  case RESEARCH_MENU_SEQ_SETUP:    InitSequence_SETUP( work );     break;
  case RESEARCH_MENU_SEQ_STAND_BY: InitSequence_STAND_BY( work );  break;
  case RESEARCH_MENU_SEQ_KEY_WAIT: InitSequence_KEY_WAIT( work );  break;
  case RESEARCH_MENU_SEQ_FADE_OUT: InitSequence_FADE_OUT( work );  break;
  case RESEARCH_MENU_SEQ_CLEAN_UP: InitSequence_CLEAN_UP( work );  break;
  case RESEARCH_MENU_SEQ_FINISH:   break;
  default:  GF_ASSERT(0);
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: set seq ==> " );
  switch( nextSeq )
  {
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

//----------------------------------------------------------------------------------------------
/**
 * @brief 準備シーケンス ( RESEARCH_MENU_SEQ_SETUP ) の初期化処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_SETUP( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init seq SETUP\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 最初のキー待ちシーケンス ( RESEARCH_MENU_SEQ_STAND_BY ) の初期化処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_STAND_BY( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init seq STAND_BY\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief キー入力待ちシーケンス ( RESEARCH_MENU_SEQ_KEY_WAIT ) の初期化処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_KEY_WAIT( RESEARCH_MENU_WORK* work )
{
  // カーソル位置のメニュー項目を選択状態にする
  MenuItemSwitchOn( work->cursorPos );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init seq KEY_WAIT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_MENU_SEQ_CLEAN_UP ) の初期化処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_CLEAN_UP( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init seq CLEAN_UP\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_MENU_SEQ_FADE_OUT ) の初期化処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_FADE_OUT( RESEARCH_MENU_WORK* work )
{
  // フェードアウト開始
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 0);

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init seq FADE_OUT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 準備シーケンス ( RESEARCH_MENU_SEQ_SETUP ) の初期化処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_SETUP( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: finish seq SETUP\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 最初のキー入力待ちシーケンス ( RESEARCH_MENU_SEQ_STAND_BY ) の初期化処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_STAND_BY( RESEARCH_MENU_WORK* work )
{
  // カーソル位置のメニュー項目を選択状態にする
  MenuItemSwitchOn( work->cursorPos );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: finish seq STAND_BY\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief キー入力待ちシーケンス ( RESEARCH_MENU_SEQ_KEY_WAIT ) の初期化処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_KEY_WAIT( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: finish seq KEY_WAIT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_MENU_SEQ_CLEAN_UP ) の初期化処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_CLEAN_UP( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: finish seq CLEAN_UP\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_MENU_SEQ_FADE_OUT ) の初期化処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_FADE_OUT( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: finish seq FADE_OUT\n" );
}

//==============================================================================================
// ■データ更新
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief 新しい人物とすれ違ったかどうかをチェックする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CheckNewEntry( RESEARCH_MENU_WORK* work )
{
  if( work->newEntryFlag == FALSE )
  {
    // 新しい人物とすれ違った
    if( GAMEBEACON_Get_NewEntry() == TRUE )
    {
      work->newEntryFlag = TRUE;
      NewIconDispOn( work );      // "new" アイコンを表示する
    }
  }
}

//==============================================================================================
// ■カーソル移動
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief カーソルを上へ移動する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void MoveCursorUp( RESEARCH_MENU_WORK* work )
{
  int nowPos;
  int nextPos;

  // 更新後のカーソル位置を算出
  nowPos  = work->cursorPos;
  nextPos = (nowPos - 1 + MENU_ITEM_NUM) % MENU_ITEM_NUM;

  // 項目の表示を更新
  MenuItemSwitchOff( nowPos );
  MenuItemSwitchOn( nextPos );

  // カーソル位置を更新
  work->cursorPos = nextPos;

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: move cursor ==> %d\n", work->cursorPos );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief カーソルを下へ移動する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void MoveCursorDown( RESEARCH_MENU_WORK* work )
{
  int nowPos;
  int nextPos;

  // 更新後のカーソル位置を算出
  nowPos  = work->cursorPos;
  nextPos = (nowPos + 1) % MENU_ITEM_NUM;

  // 項目の表示を更新
  MenuItemSwitchOff( nowPos );
  MenuItemSwitchOn( nextPos );

  // カーソル位置を更新
  work->cursorPos = nextPos;

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: move cursor ==> %d\n", work->cursorPos );
}


//==============================================================================================
// ■画面終了結果
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief 画面の終了結果を決定する
 *
 * @param work
 * @param result 結果
 */
//----------------------------------------------------------------------------------------------
static void SetResult( RESEARCH_MENU_WORK* work, RESEARCH_MENU_RESULT result )
{
  // 多重設定
  GF_ASSERT( work->result == RESEARCH_MENU_RESULT_NONE );

  // 設定
  work->result = result;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: set result (%d)\n", result );
}


//==============================================================================================
// ■フォント
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitFont( RESEARCH_MENU_WORK* work )
{
  // 初期化
  work->font = NULL;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init font\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateFont( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->font == NULL );

  // 生成
  work->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, 
                                GFL_FONT_LOADTYPE_FILE, TRUE, work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create font\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteFont( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->font );

  // 削除
  GFL_FONT_Delete( work->font );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete font\n" );
}


//==============================================================================================
// ■メッセージ
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitMessages( RESEARCH_MENU_WORK* work )
{
  int msgIdx;

  // 初期化
  for( msgIdx=0; msgIdx < MESSAGE_NUM; msgIdx++ )
  {
    work->message[ msgIdx ] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init messages\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateMessages( RESEARCH_MENU_WORK* work )
{
  int msgIdx;

  for( msgIdx=0; msgIdx < MESSAGE_NUM; msgIdx++ )
  {
    // 多重生成
    GF_ASSERT( work->message[ msgIdx ] == NULL );

    // 作成
    work->message[ msgIdx ] = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, 
                                              ARCID_MESSAGE, 
                                              MessageDataID[ msgIdx ],
                                              work->heapID ); 
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create messages\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteMessages( RESEARCH_MENU_WORK* work )
{
  int msgIdx;

  for( msgIdx=0; msgIdx < MESSAGE_NUM; msgIdx++ )
  {
    GF_ASSERT( work->message[ msgIdx ] );

    // 削除
    GFL_MSG_Delete( work->message[ msgIdx ] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete messages\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief タッチ範囲を作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupTouchArea( RESEARCH_MENU_WORK* work )
{
  int idx;

  for( idx=0; idx < TOUCH_AREA_NUM; idx++ )
  {
    work->touchHitTable[ idx ].rect.left   = TouchAreaInitData[ idx ].left;
    work->touchHitTable[ idx ].rect.right  = TouchAreaInitData[ idx ].right;
    work->touchHitTable[ idx ].rect.top    = TouchAreaInitData[ idx ].top;
    work->touchHitTable[ idx ].rect.bottom = TouchAreaInitData[ idx ].bottom;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create touch hit table\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSeqQueue( RESEARCH_MENU_WORK* work )
{
  // 初期化
  work->seqQueue = NULL;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: init seq queue\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateSeqQueue( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->seqQueue == NULL );

  // 作成
  work->seqQueue = QUEUE_Create( SEQ_QUEUE_SIZE, work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create seq queue\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteSeqQueue( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->seqQueue );

  // 削除
  QUEUE_Delete( work->seqQueue );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete seq queue\n" );
}


//==============================================================================================
// ■BG
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief BG の準備
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------
/**
 * @brief BG の後片付け
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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


//==============================================================================================
// ■上画面 ウィンドウBG面
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief 上画面 ウィンドウBG面 準備
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------
/**
 * @brief 上画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpSubBG_WINDOW( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up SUB-BG-WINDOW\n" );
}


//==============================================================================================
// ■上画面 フォントBG面
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief SUB-BG フォント面の準備
 * 
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupSubBG_FONT( RESEARCH_MENU_WORK* work )
{
  // NULLキャラ確保
  GFL_BG_FillCharacter( SUB_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( SUB_BG_FONT );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: setup SUB-BG-FONT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief SUB-BG フォント面の後片付け
 * 
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpSubBG_FONT( RESEARCH_MENU_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( SUB_BG_FONT, 1, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up SUB-BG-FONT\n" );
}


//==============================================================================================
// ■下画面 ウィンドウBG面
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 準備
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpMainBG_WINDOW( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up MAIN-BG-WINDOW\n" );
}


//==============================================================================================
// ■下画面 フォントBG面
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief 下画面 フォントBG面 準備
 * 
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupMainBG_FONT( RESEARCH_MENU_WORK* work )
{ 
  // NULLキャラ確保
  GFL_BG_FillCharacter( MAIN_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( MAIN_BG_FONT );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: setup MAIN-BG-FONT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 下画面 フォントBG面 後片付け
 * 
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpMainBG_FONT( RESEARCH_MENU_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( MAIN_BG_FONT, 1, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up MAIN-BG-FONT\n" );
}


//==============================================================================================
// ■文字列描画オブジェクト
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitBGFonts( RESEARCH_MENU_WORK* work )
{
  int idx;

  for( idx=0; idx < BG_FONT_NUM; idx++ )
  {
    work->BGFont[ idx ] = NULL; 
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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


//==============================================================================================
// ■メニュー項目操作
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @biref メニュー項目を選択状態にする
 *
 * @param menuItem 選択状態にするメニュー項目を指定
 */
//----------------------------------------------------------------------------------------------
static void MenuItemSwitchOn( MENU_ITEM menuItem )
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

//----------------------------------------------------------------------------------------------
/**
 * @biref メニュー項目を非選択状態にする
 *
 * @param menuItem 非選択状態にするメニュー項目を指定
 */
//----------------------------------------------------------------------------------------------
static void MenuItemSwitchOff( MENU_ITEM menuItem )
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

//==============================================================================================
// ■OBJ
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターシステムを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateClactSystem( RESEARCH_MENU_WORK* work )
{
  // システム作成
  GFL_CLACT_SYS_Create( &ClactSystemInitData, &VRAMBankSettings, work->heapID );

  // VBlank 割り込み関数を登録
  GFUser_SetVIntrFunc( GFL_CLACT_SYS_VBlankFunc );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create clact system\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターシステムを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteClactSystem( RESEARCH_MENU_WORK* work )
{
  // VBkank 割り込み関数を解除
  GFUser_ResetVIntrFunc();

  // システム破棄
  GFL_CLACT_SYS_Delete();

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete clact system\n" );
}


//==============================================================================================
// ■SUB-OBJ リソース
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief SUB-OBJ リソースを登録する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void RegisterSubObjResources( RESEARCH_MENU_WORK* work )
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

//----------------------------------------------------------------------------------------------
/**
 * @brief SUB-OBJ リソースを解放する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ReleaseSubObjResources( RESEARCH_MENU_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CELL_ANIME ] );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: release SUB-OBJ resources\n" );
}


//==============================================================================================
// ■MAIN-OBJ リソース
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief MAIN-OBJ リソースを登録する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void RegisterMainObjResources( RESEARCH_MENU_WORK* work )
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

  palette = GFL_CLGRP_PLTT_Register( arcHandle, 
                                     NARC_research_radar_graphic_obj_NCLR,
                                     CLSYS_DRAW_MAIN, 0, heapID );

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

//----------------------------------------------------------------------------------------------
/**
 * @brief MAIN-OBJ リソースを解放する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ReleaseMainObjResources( RESEARCH_MENU_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CELL_ANIME ] );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: release MAIN-OBJ resources\n" );
}


//==============================================================================================
// ■セルアクターユニット
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを作成する
 *
 * @param
 */
//----------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを破棄する
 *
 * @param
 */
//----------------------------------------------------------------------------------------------
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


//==============================================================================================
// □セルアクターワーク
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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
    charaIdx       = GetObjResourceRegisterIndex( work, ClactWorkInitData[ wkIdx ].characterResID );
    paletteIdx     = GetObjResourceRegisterIndex( work, ClactWorkInitData[ wkIdx ].paletteResID );
    cellAnimeIdx   = GetObjResourceRegisterIndex( work, ClactWorkInitData[ wkIdx ].cellAnimeResID );
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

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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


//==============================================================================================
// ■OBJ アクセス
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief OBJ リソースの登録インデックスを取得する
 *
 * @param work
 * @param resID リソースID
 *
 * @return 指定したリソースの登録インデックス
 */
//----------------------------------------------------------------------------------------------
static u32 GetObjResourceRegisterIndex( const RESEARCH_MENU_WORK* work, OBJ_RESOURCE_ID resID )
{
  return work->objResRegisterIdx[ resID ];
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを取得する
 *
 * @param work
 * @param unitIdx セルアクターユニットのインデックス
 *
 * @return 指定したセルアクターユニット
 */
//----------------------------------------------------------------------------------------------
static GFL_CLUNIT* GetClactUnit( const RESEARCH_MENU_WORK* work, CLUNIT_INDEX unitIdx )
{
  return work->clactUnit[ unitIdx ];
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを取得する
 *
 * @param work
 * @param unitIdx セルアクターワークのインデックス
 *
 * @return 指定したセルアクターワーク
 */
//----------------------------------------------------------------------------------------------
static GFL_CLWK* GetClactWork( const RESEARCH_MENU_WORK* work, CLWK_INDEX wkIdx )
{
  return work->clactWork[ wkIdx ];
}


//==============================================================================================
// ■"new" アイコン
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * "new" アイコンを表示する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------
/**
 * "new" アイコンを非表示にする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void NewIconDispOff( const RESEARCH_MENU_WORK* work )
{
  GFL_CLWK* clactWork;

  clactWork = GetClactWork( work, CLWK_NEW_ICON );
  GFL_CLACT_WK_SetDrawEnable( clactWork, FALSE );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: new icon disp off\n" );
}


//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューの中身を表示する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
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
    
    switch( value )
    {
    case RESEARCH_MENU_SEQ_SETUP:     OS_TFPrintf( PRINT_TARGET, "SETUP " );    break;
    case RESEARCH_MENU_SEQ_STAND_BY:  OS_TFPrintf( PRINT_TARGET, "STAND_BY " );    break;
    case RESEARCH_MENU_SEQ_KEY_WAIT:  OS_TFPrintf( PRINT_TARGET, "KEY-WAIT " ); break;
    case RESEARCH_MENU_SEQ_FADE_OUT:  OS_TFPrintf( PRINT_TARGET, "FADE_OUT " ); break;
    case RESEARCH_MENU_SEQ_CLEAN_UP:  OS_TFPrintf( PRINT_TARGET, "CLEAN-UP " ); break;
    case RESEARCH_MENU_SEQ_FINISH:    OS_TFPrintf( PRINT_TARGET, "FINISH " );   break;
    default: GF_ASSERT(0);
    }
  }
  OS_TFPrintf( PRINT_TARGET, "\n" );
} 
