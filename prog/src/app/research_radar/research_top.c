//////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー トップ画面
 * @file   research_top.c
 * @author obata
 * @date   2010.02.03
 */
//////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "bg_font.h"
#include "queue.h"
#include "palette_anime.h"
#include "research_top.h"
#include "research_top_index.h"
#include "research_top_def.h"
#include "research_top_data.cdat"
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

#include "../../../../resource/fldmapdata/flagwork/flag_define.h" // for FE_xxxx


//====================================================================================
// ■トップ画面管理ワーク
//====================================================================================
struct _RESEARCH_RADAR_TOP_WORK { 

  RRC_WORK* commonWork; // 全画面共通ワーク
  HEAPID heapID;

  GFL_FONT*    font;
  GFL_MSGDATA* message[ MESSAGE_NUM ];

  QUEUE*    stateQueue; // 状態キュー
  RRT_STATE state;      // 処理状態
  u32       stateSeq;   // 状態内シーケンス番号
  u32       stateCount; // 状態カウンタ
  u32       waitFrame;  // WAIT 状態での待ちフレーム数

  MENU_ITEM cursorPos; // カーソル位置

  GFL_UI_TP_HITTBL touchHitTable[ TOUCH_AREA_NUM ]; // タッチ領域
  PALETTE_ANIME*   paletteAnime[ PALETTE_ANIME_NUM ]; // パレットアニメーション
  BG_FONT*         BGFont[ BG_FONT_NUM ]; // 文字列描画オブジェクト

  // OBJ
  u32         objResRegisterIdx[ OBJ_RESOURCE_NUM ];  // リソースの登録インデックス
  GFL_CLUNIT* clactUnit[ CLUNIT_NUM ];                // セルアクターユニット
  GFL_CLWK*   clactWork[ CLWK_NUM ];                  // セルアクターワーク

  // フラグ
  BOOL nowStateEndFlag; // 現在の状態が終了したかどうか
  BOOL newEntryFlag;  // 新しい人物とすれ違ったかどうか
  BOOL finishFlag;    // トップ画面終了フラグ
  RRT_RESULT finishResult; // 終了結果
};


//====================================================================================
// ■関数インデックス
//====================================================================================
//------------------------------------------------------------------------------------
// ◆LAYER 4 状態
//------------------------------------------------------------------------------------
// 状態処理
static void Main_SETUP( RRT_WORK* work ); // RRT_STATE_SETUP
static void Main_STANDBY( RRT_WORK* work ); // RRT_STATE_STANDBY
static void Main_KEYWAIT( RRT_WORK* work ); // RRT_STATE_KEYWAIT
static void Main_WAIT( RRT_WORK* work ); // RRT_STATE_WAIT
static void Main_FADEOUT( RRT_WORK* work ); // RRT_STATE_FADEOUT
static void Main_CLEANUP( RRT_WORK* work ); // RRT_STATE_CLEANUP 
// 状態制御
static void RegisterNextState( RRT_WORK* work, RRT_STATE nextState ); // 次の状態をキューに登録する
static void FinishNowState( RRT_WORK* work ); // 現在の状態を終了する
static void SwitchState( RRT_WORK* work ); // 処理状態を変更する
static void SetState( RRT_WORK* work, RRT_STATE nextState ); // 処理状態を設定する
static RRT_STATE GetFirstState( const RRT_WORK* work ); // 最初の状態を取得する
static u32 GetStateSeq( const RRT_WORK* work ); // 状態内シーケンス番号を取得する
static void IncStateSeq( RRT_WORK* work ); // 状態内シーケンス番号をインクリメントする
static void SetFinishReason( const RRT_WORK* work, SEQ_CHANGE_TRIG reason ); // トップ画面の終了理由を登録する
static void SetFinishResult( RRT_WORK* work, RRT_RESULT result ); // 終了結果を設定する
//------------------------------------------------------------------------------------
// ◆LAYER 3 機能
//------------------------------------------------------------------------------------
// データ更新
static void CheckNewEntry( RRT_WORK* work ); // 新しい人物とすれ違ったかどうかをチェックする
// カーソル移動
static void MoveCursorUp( RRT_WORK* work ); // カーソルを上に移動する
static void MoveCursorDown( RRT_WORK* work ); // カーソルを下に移動する
static void MoveCursorDirect( RRT_WORK* work, MENU_ITEM menuItem ); // カーソルを直接移動する
// メニュー項目ボタン
static void MenuItemSetCursorOn( MENU_ITEM menuItem ); // カーソルが乗っている状態にする
static void MenuItemSetCursorOff( MENU_ITEM menuItem ); // カーソルが乗っていない状態にする
static void SetChangeButtonNotActive( RRT_WORK* work ); //『調査を決める』ボタンを非アクティブ状態にする
//『戻る』ボタン
static void BlinkReturnButton( RRT_WORK* work ); //『戻る』ボタンを明滅させる
// "new" アイコン
static void NewIconDispOn( const RRT_WORK* work ); // "new" アイコンを表示する
static void NewIconDispOff( const RRT_WORK* work ); // "new" アイコンを非表示にする
// 画面のフェードイン・フェードアウト
static void StartFadeIn( void ); // フェードインを開始する
static void StartFadeOut( void ); // フェードアウトを開始する
//------------------------------------------------------------------------------------
// ◆LAYER 2 取得・設定・判定
//------------------------------------------------------------------------------------
// 各種カウンタ
static void CountUpStateCount( RRT_WORK* work ); // 状態カウンタを更新する
static u32 GetStateCount( const RRT_WORK* work ); // 状態カウンタの値を取得する
static void ResetStateCount( RRT_WORK* work ); // 状態カウンタをリセットする
static void SetWaitFrame( RRT_WORK* work, u32 frame ); // WAIT 状態の待ち時間を設定する
static u32 GetWaitFrame( const RRT_WORK* work ); // WAIT 状態の待ち時間を取得する
// カーソル位置
static void ShiftCursorPos( RRT_WORK* work, int offset ); // カーソル位置を変更する ( オフセット指定 )
static void SetCursorPos( RRT_WORK* work, MENU_ITEM menuItem ); // カーソル位置を変更する ( 直値指定 )
static BOOL CheckCursorSetEnable( RRT_WORK* work, MENU_ITEM menuItem ); // カーソルを指定位置にセットできるかどうかをチェックする
// OBJ
static u32 GetOBJResRegisterIndex( const RRT_WORK* work, OBJ_RESOURCE_ID resID ); // OBJリソースの登録インデックスを取得する
static GFL_CLUNIT* GetClactUnit( const RRT_WORK* work, CLUNIT_INDEX unitIdx ); // セルアクターユニットを取得する
static GFL_CLWK* GetClactWork( const RRT_WORK* work, CLWK_INDEX wkIdx ); // セルアクターワークを取得する
// パレットアニメーション
static void StartPaletteAnime( RRT_WORK* work, PALETTE_ANIME_INDEX index ); // パレットアニメーションを開始する
static void StopPaletteAnime( RRT_WORK* work, PALETTE_ANIME_INDEX index ); // パレットアニメーションを停止する
static void UpdatePaletteAnime( RRT_WORK* work ); // パレットアニメーションを更新する
// 調査項目
static u8 GetSelectableTopicNum( const RRT_WORK* work ); // 選択可能な調査項目の数を取得する
// 全画面共通ワーク
static RRC_WORK* GetCommonWork( const RRT_WORK* work ); // 全画面共通ワークを取得する
static void SetCommonWork( RRT_WORK* work, RRC_WORK* commonWork ); // 全画面共通ワークを設定する
// ヒープID
static HEAPID GetHeapID( const RRT_WORK* work ); // ヒープIDを取得する
static void SetHeapID( RRT_WORK* work, HEAPID heapID ); // ヒープIDを設定する
// 強制終了フラグ
static BOOL CheckForceReturnFlag( const RRT_WORK* work );
//------------------------------------------------------------------------------------
// ◆LAYER 1 生成・初期化・破棄
//------------------------------------------------------------------------------------
// トップ画面ワーク
static RRT_WORK* CreateTopWork( HEAPID heapID ); // トップ画面ワークを生成する
static void DeleteTopWork( RRT_WORK* work ); // トップ画面ワークを破棄する
static void InitTopWork( RRT_WORK* work ); // トップ画面ワークを初期化する
// カーソル位置
static void SetupCursorPos( RRT_WORK* work ); // カーソル位置をセットアップする
// フォント
static void InitFont( RRT_WORK* work ); // フォントデータを初期化する
static void CreateFont( RRT_WORK* work ); // フォントデータを生成する
static void DeleteFont( RRT_WORK* work ); // フォントデータを破棄する
// メッセージ
static void InitMessages( RRT_WORK* work ); // メッセージデータを初期化する
static void CreateMessages( RRT_WORK* work ); // メッセージデータを生成する
static void DeleteMessages( RRT_WORK* work ); // メッセージデータを破棄する
// タッチ領域
static void SetupTouchArea( RRT_WORK* work ); // タッチ領域をセットアップする
// 状態キュー
static void InitStateQueue( RRT_WORK* work ); // 状態キューを初期化する
static void CreateStateQueue( RRT_WORK* work ); // 状態キューを生成する
static void DeleteStateQueue( RRT_WORK* work ); // 状態キューを破棄する
// BG
static void SetupBG( RRT_WORK* work ); // BG全般のセットアップを行う
static void SetupSubBG_WINDOW( RRT_WORK* work ); // SUB-BG ( ウィンドウ面 ) のセットアップを行う
static void SetupSubBG_FONT( RRT_WORK* work ); // SUB-BG ( フォント面 ) のセットアップを行う
static void SetupMainBG_WINDOW( RRT_WORK* work ); // MAIN-BG ( ウィンドウ面 ) のセットアップを行う
static void SetupMainBG_FONT( RRT_WORK* work ); // MAIN-BG ( フォント面 ) のセットアップを行う
static void CleanUpBG( RRT_WORK* work ); // BG全般のクリーンアップを行う
static void CleanUpSubBG_WINDOW( RRT_WORK* work ); // SUB-BG ( ウィンドウ面 ) のクリーンアップを行う
static void CleanUpSubBG_FONT( RRT_WORK* work ); // SUB-BG ( フォント面 ) のクリーンアップを行う
static void CleanUpMainBG_WINDOW( RRT_WORK* work ); // MAIN-BG ( ウィンドウ面 ) のクリーンアップを行う
static void CleanUpMainBG_FONT( RRT_WORK* work ); // MAIN-BG ( フォント面 ) のクリーンアップを行う
// 文字列描画オブジェクト
static void InitBGFonts( RRT_WORK* work ); // 文字列描画オブジェクトを初期化する
static void CreateBGFonts( RRT_WORK* work ); // 文字列描画オブジェクトを生成する
static void DeleteBGFonts( RRT_WORK* work ); // 文字列描画オブジェクトを破棄する
// OBJ リソース
static void InitOBJResources( RRT_WORK* work ); // OBJリソースを初期化する
static void RegisterSubOBJResources( RRT_WORK* work ); // OBJリソースを登録する ( SUB-OBJ )
static void RegisterMainOBJResources( RRT_WORK* work ); // OBJリソースを登録する ( MAIN-OBJ )
static void ReleaseSubOBJResources( RRT_WORK* work ); // OBJリソースの登録を解除する ( SUB-OBJ )
static void ReleaseMainOBJResources( RRT_WORK* work ); // OBJリソースの登録を解除する ( MAIN-OBJ )
// セルアクターユニット
static void InitClactUnits( RRT_WORK* work ); // セルアクターユニットを初期化する
static void CreateClactUnits( RRT_WORK* work ); // セルアクターユニットを生成する
static void DeleteClactUnits( RRT_WORK* work ); // セルアクターユニットを破棄する
// セルアクターワーク
static void InitClactWorks( RRT_WORK* work ); // セルアクターワークを初期化する
static void CreateClactWorks( RRT_WORK* work ); // セルアクターワークを生成する
static void DeleteClactWorks( RRT_WORK* work ); // セルアクターワークを破棄する
// パレットアニメーション
static void InitPaletteAnime( RRT_WORK* work ); // パレットアニメーションワークを初期化する
static void CreatePaletteAnime( RRT_WORK* work ); // パレットアニメーションワークを生成する
static void DeletePaletteAnime( RRT_WORK* work ); // パレットアニメーションワークを破棄する
static void SetupPaletteAnime( RRT_WORK* work ); // パレットアニメーションワークをセットアップする
static void CleanUpPaletteAnime( RRT_WORK* work ); // パレットアニメーションワークをクリーンアップする
// VBlank 割り込み
static void SetVBlankFunc( RRT_WORK* work ); // VBlank 割り込みを設定
static void ResetVBlankFunc( RRT_WORK* work ); // VBlank 割り込みを解除
// 通信アイコン
static void SetupWirelessIcon( const RRT_WORK* work ); // 通信アイコンをセットアップする



//====================================================================================
// ■public functions
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief トップ画面ワークを生成する
 *
 * @param commonWork 全画面共通ワーク
 *
 * @return 生成したトップ画面ワーク
 */
//------------------------------------------------------------------------------------
RRT_WORK* RRT_CreateWork( RRC_WORK* commonWork )
{
  RRT_WORK* work;
  HEAPID heapID;

  heapID = RRC_GetHeapID( commonWork );

  // ワークを生成
  work = CreateTopWork( heapID );

  // ワークを初期化
  InitTopWork( work ); 
  SetHeapID( work, heapID );
  SetCommonWork( work, commonWork );

  return work;
}

//------------------------------------------------------------------------------------
/**
 * @brief トップ画面ワークの破棄
 *
 * @param heapID
 */
//------------------------------------------------------------------------------------
void RRT_DeleteWork( RRT_WORK* work )
{
  DeleteStateQueue( work );
  DeleteTopWork( work );
} 

//------------------------------------------------------------------------------------
/**
 * @brief トップ画面 メイン動作
 *
 * @param work
 */
//------------------------------------------------------------------------------------
void RRT_Main( RRT_WORK* work )
{
  // 状態ごとの処理
  switch( work->state ) {
  case RRT_STATE_SETUP:    Main_SETUP( work );    break;
  case RRT_STATE_STANDBY:  Main_STANDBY( work );  break;
  case RRT_STATE_KEYWAIT:  Main_KEYWAIT( work );  break;
  case RRT_STATE_WAIT:     Main_WAIT( work );     break;
  case RRT_STATE_FADEOUT:  Main_FADEOUT( work );  break;
  case RRT_STATE_CLEANUP:  Main_CLEANUP( work );  break;
  case RRT_STATE_FINISH:   return;
  default:  GF_ASSERT(0);
  }

  if( RRT_IsFinished(work) == FALSE ) { // 破棄後のワークを操作しないようにする
    CheckNewEntry( work );
    UpdatePaletteAnime( work );
    RRC_UpdatePaletteAnime( work->commonWork );
    GFL_CLACT_SYS_Main();
  }

  // 状態の更新
  CountUpStateCount( work );
  SwitchState( work );
}

//------------------------------------------------------------------------------------
/**
 * @breif 終了判定
 *
 * @param work
 *
 * @return トップ画面が終了した場合 TRUE
 */
//------------------------------------------------------------------------------------
BOOL RRT_IsFinished( const RRT_WORK* work )
{
  if( work->finishFlag ) {
    return TRUE;
  }
  else { 
    return FALSE;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief 終了結果の取得
 *
 * @param work
 *
 * @return トップ画面の終了結果
 */
//------------------------------------------------------------------------------------
RRT_RESULT RRT_GetResult( const RRT_WORK* work )
{
  GF_ASSERT( work->finishFlag );

  return work->finishResult;
}


//====================================================================================
// ■private functions
//====================================================================================

//====================================================================================
// ◆LAYER 4 状態
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 準備状態 ( RRT_STATE_SETUP ) の処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void Main_SETUP( RRT_WORK* work )
{
  // データ生成
  CreateStateQueue( work );
  CreateFont( work );
  CreateMessages( work );
  SetupTouchArea( work );
  SetupCursorPos( work );

  // BG 準備
  SetupBG           ( work );
  SetupSubBG_WINDOW ( work );
  SetupSubBG_FONT   ( work );
  SetupMainBG_WINDOW( work );
  SetupMainBG_FONT  ( work );

  // 文字列描画オブジェクト 準備
  CreateBGFonts( work );

  // OBJ 準備
  RegisterSubOBJResources( work );
  RegisterMainOBJResources( work );
  CreateClactUnits( work );
  CreateClactWorks( work );

  // パレットアニメーション
  CreatePaletteAnime( work );
  SetupPaletteAnime( work );

  // 通信アイコン
  SetupWirelessIcon( work );

  // VBlank 割り込み
  SetVBlankFunc( work );

  //『調査を決める』ボタンを非アクティブ状態にする
  if( CheckCursorSetEnable( work, MENU_ITEM_CHANGE_RESEARCH ) == FALSE ) {
    SetChangeButtonNotActive( work );
  }

  // 次の状態を設定
  {
    RRT_STATE next_state;

    next_state = GetFirstState( work );
    RegisterNextState( work, GetFirstState(work) ); // 次の状態をセット

    if( next_state == RRT_STATE_KEYWAIT ) {
      MenuItemSetCursorOn( work->cursorPos ); // カーソル位置のメニュー項目を選択状態にする
      StartPaletteAnime( work, PALETTE_ANIME_CURSOR_ON ); // カーソルONパレットアニメを開始
    } 
  }

  // 画面フェードイン開始
  StartFadeIn();

  // 状態終了
  FinishNowState( work ); 
}

//------------------------------------------------------------------------------------
/**
 * @brief 最初のキー待ち状態 ( RRT_STATE_STANDBY ) の処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void Main_STANDBY( RRT_WORK* work )
{
  int trg;
  int touch;
  int commonTouch;

  trg = GFL_UI_KEY_GetTrg();
  touch = GFL_UI_TP_HitTrg( work->touchHitTable );
  commonTouch = GFL_UI_TP_HitTrg( RRC_GetHitTable(work->commonWork) );

  //-----------------------------
  // 強制終了 or 「もどる」ボタン 
  if( CheckForceReturnFlag( work ) || (commonTouch == COMMON_TOUCH_AREA_RETURN_BUTTON) ) {
    SetFinishReason( work, SEQ_CHANGE_BY_TOUCH );  // 画面遷移のトリガを登録
    SetFinishResult( work, RRT_RESULT_EXIT );      // 画面終了結果を決定
    BlinkReturnButton( work );                     //『戻る』ボタンを光らせる
    PMSND_PlaySE( SEQ_SE_CANCEL1 );                // キャンセル音
    SetWaitFrame( work, WAIT_FRAME_BUTTON );       // 待ち時間を設定
    FinishNowState( work );                        // RRT_STATE_STANDBY 状態を終了
    RegisterNextState( work, RRT_STATE_WAIT );     // => RRT_STATE_WAIT
    RegisterNextState( work, RRT_STATE_FADEOUT );  // ==> RRT_STATE_FADEOUT
    RegisterNextState( work, RRT_STATE_CLEANUP );  // ===> RRT_STATE_CLEANUP
    return;
  }

  //----
  // B
  if( trg & PAD_BUTTON_B ) {
    SetFinishReason( work, SEQ_CHANGE_BY_BUTTON ); // 画面遷移のトリガを登録
    SetFinishResult( work, RRT_RESULT_EXIT );      // 画面終了結果を決定
    BlinkReturnButton( work );                     //『戻る』ボタンを光らせる
    PMSND_PlaySE( SEQ_SE_CANCEL1 );                // キャンセル音
    SetWaitFrame( work, WAIT_FRAME_BUTTON );       // 待ち時間を設定
    FinishNowState( work );                        // RRT_STATE_STANDBY 状態を終了
    RegisterNextState( work, RRT_STATE_WAIT );     // => RRT_STATE_WAIT
    RegisterNextState( work, RRT_STATE_FADEOUT );  // ==> RRT_STATE_FADEOUT
    RegisterNextState( work, RRT_STATE_CLEANUP );  // ===> RRT_STATE_CLEANUP
    return;
  }

  //----------------
  // 十字キー or A
  if( (trg & PAD_KEY_UP)   || (trg & PAD_KEY_DOWN)  ||
      (trg & PAD_KEY_LEFT) || (trg & PAD_KEY_RIGHT) || (trg & PAD_BUTTON_A) ) {
    MenuItemSetCursorOn( work->cursorPos );             // カーソル位置のメニュー項目を選択状態にする
    StartPaletteAnime( work, PALETTE_ANIME_CURSOR_ON ); // カーソルONパレットアニメを開始
    FinishNowState( work );                             // RRT_STATE_STANDBY 状態を終了
    RegisterNextState( work, RRT_STATE_KEYWAIT );       // => RRT_STATE_KEYWAIT
    return;
  }

  //--------------------------------
  //『調査を決める』ボタンをタッチ
  if( touch == TOUCH_AREA_CHANGE_BUTTON ) {
    // 選択可
    if( CheckCursorSetEnable(work, MENU_ITEM_CHANGE_RESEARCH) ) {
      SetFinishReason( work, SEQ_CHANGE_BY_TOUCH );        // 画面遷移のトリガを登録
      SetFinishResult( work, RRT_RESULT_TO_LIST );         // 画面終了結果を決定
      MoveCursorDirect( work, MENU_ITEM_CHANGE_RESEARCH ); // カーソル位置を更新
      MenuItemSetCursorOn( work->cursorPos );              // カーソル位置のメニュー項目を選択状態にする
      StopPaletteAnime( work, PALETTE_ANIME_CURSOR_ON );   // カーソルONパレットアニメ終了
      StartPaletteAnime( work, PALETTE_ANIME_SELECT );     // 選択パレットアニメ開始
      PMSND_PlaySE( SEQ_SE_DECIDE1 );                      // 決定音
      SetWaitFrame( work, WAIT_FRAME_BUTTON );             // 待ち時間を設定
      FinishNowState( work );                              // RRT_STATE_STANDBY 状態を終了
      RegisterNextState( work, RRT_STATE_WAIT );           // => RRT_STATE_WAIT
      RegisterNextState( work, RRT_STATE_FADEOUT );        // ==> RRT_STATE_FADEOUT
      RegisterNextState( work, RRT_STATE_CLEANUP );        // ===> RRT_STATE_CLEANUP
      return;
    }
    // 選択不可
    else {
      PMSND_PlaySE( SEQ_SE_BEEP );
    }
    return;
  }
  //------------------------------
  //『調査を見る』ボタンをタッチ
  if( touch == TOUCH_AREA_CHECK_BUTTON ) {
    // 選択可
    if( CheckCursorSetEnable(work, MENU_ITEM_CHECK_RESEARCH) ) {
      SetFinishReason( work, SEQ_CHANGE_BY_TOUCH );       // 画面遷移のトリガを登録
      SetFinishResult( work, RRT_RESULT_TO_GRAPH );       // 画面終了結果を決定
      MoveCursorDirect( work, MENU_ITEM_CHECK_RESEARCH ); // カーソル位置を更新
      MenuItemSetCursorOn( work->cursorPos );             // カーソル位置のメニュー項目を選択状態にする
      StopPaletteAnime( work, PALETTE_ANIME_CURSOR_ON );  // カーソルONパレットアニメ終了
      StartPaletteAnime( work, PALETTE_ANIME_SELECT );    // 選択パレットアニメ開始
      PMSND_PlaySE( SEQ_SE_DECIDE1 );                     // 決定音
      SetWaitFrame( work, WAIT_FRAME_BUTTON );            // 待ち時間を設定
      FinishNowState( work );                             // RRT_STATE_STANDBY 状態を終了
      RegisterNextState( work, RRT_STATE_WAIT );          // => RRT_STATE_WAIT
      RegisterNextState( work, RRT_STATE_FADEOUT );       // ==> RRT_STATE_FADEOUT
      RegisterNextState( work, RRT_STATE_CLEANUP );       // ===> RRT_STATE_CLEANUP
      return;
    }
    // 選択不可
    else {
      PMSND_PlaySE( SEQ_SE_BEEP );
    }
    return;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief キー入力待ち状態 ( RRT_STATE_KEYWAIT ) の処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void Main_KEYWAIT( RRT_WORK* work )
{
  int trg;
  int touch;
  int commonTouch;

  trg = GFL_UI_KEY_GetTrg();
  touch = GFL_UI_TP_HitTrg( work->touchHitTable );
  commonTouch = GFL_UI_TP_HitTrg( RRC_GetHitTable(work->commonWork) );

  //-----------------------------
  // 強制終了 or 「もどる」ボタン 
  if( CheckForceReturnFlag( work ) || (commonTouch == COMMON_TOUCH_AREA_RETURN_BUTTON) ) {
    SetFinishReason( work, SEQ_CHANGE_BY_TOUCH ); // 画面遷移のトリガを登録
    SetFinishResult( work, RRT_RESULT_EXIT );     // 画面終了結果を決定
    BlinkReturnButton( work );                    //『戻る』ボタンを光らせる
    PMSND_PlaySE( SEQ_SE_CANCEL1 );               // キャンセル音
    SetWaitFrame( work, WAIT_FRAME_BUTTON );      // 待ち時間を設定
    FinishNowState( work );                       // RRT_STATE_KEYWAIT 状態を終了
    RegisterNextState( work, RRT_STATE_WAIT );    // => RRT_STATE_WAIT
    RegisterNextState( work, RRT_STATE_FADEOUT ); // ==> RRT_STATE_FADEOUT
    RegisterNextState( work, RRT_STATE_CLEANUP ); // ===> RRT_STATE_CLEANUP
    return;
  }

  //--------
  // 上 キー
  if( trg & PAD_KEY_UP ) {
    MoveCursorUp( work ); // カーソルを移動
  } 
  //--------
  // 下 キー
  if( trg & PAD_KEY_DOWN ) {
    MoveCursorDown( work ); // カーソルを移動
  }

  //----------
  // A ボタン
  if( trg & PAD_BUTTON_A ) {
    SetFinishReason( work, SEQ_CHANGE_BY_BUTTON );     // 画面遷移のトリガを登録
    StopPaletteAnime( work, PALETTE_ANIME_CURSOR_ON ); // カーソルONパレットアニメ終了
    StartPaletteAnime( work, PALETTE_ANIME_SELECT );   // 選択パレットアニメ開始
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                    // 決定音
    SetWaitFrame( work, WAIT_FRAME_BUTTON );           // 待ち時間を設定
    FinishNowState( work );                            // RRT_STATE_KEYWAIT 状態を終了
    RegisterNextState( work, RRT_STATE_WAIT );         // => RRT_STATE_WAIT
    RegisterNextState( work, RRT_STATE_FADEOUT );      // ==> RRT_STATE_FADEOUT
    RegisterNextState( work, RRT_STATE_CLEANUP );      // ===> RRT_STATE_CLEANUP
    // 画面終了結果を決定
    switch( work->cursorPos ) { 
    case MENU_ITEM_CHANGE_RESEARCH: SetFinishResult( work, RRT_RESULT_TO_LIST );  break;
    case MENU_ITEM_CHECK_RESEARCH:  SetFinishResult( work, RRT_RESULT_TO_GRAPH ); break;
    default:                        SetFinishResult( work, RRT_RESULT_EXIT );     break;
    }
    return;
  }

  //-----------------------
  //『調査を決める』ボタン
  if( touch == TOUCH_AREA_CHANGE_BUTTON ) {
    // 選択可
    if( CheckCursorSetEnable(work, MENU_ITEM_CHANGE_RESEARCH) ) {
      SetFinishReason( work, SEQ_CHANGE_BY_TOUCH );         // 画面遷移のトリガを登録
      MoveCursorDirect( work, MENU_ITEM_CHANGE_RESEARCH );  // カーソル位置を更新
      StopPaletteAnime( work, PALETTE_ANIME_CURSOR_ON );    // カーソルONパレットアニメ終了
      StartPaletteAnime( work, PALETTE_ANIME_SELECT );      // 選択パレットアニメ開始
      PMSND_PlaySE( SEQ_SE_DECIDE1 );                       // 決定音
      SetFinishResult( work, RRT_RESULT_TO_LIST );          // 画面終了結果を決定
      SetWaitFrame( work, WAIT_FRAME_BUTTON );              // 待ち時間を設定
      FinishNowState( work );                               // RRT_STATE_KEYWAIT 状態を終了
      RegisterNextState( work, RRT_STATE_WAIT );            // => RRT_STATE_WAIT
      RegisterNextState( work, RRT_STATE_FADEOUT );         // ==> RRT_STATE_FADEOUT       
      RegisterNextState( work, RRT_STATE_CLEANUP );         // ===> RRT_STATE_CLEANUP      
    }
    // 選択不可
    else {
      PMSND_PlaySE( SEQ_SE_BEEP );
    }
    return;
  }

  //----------------------
  //『調査を見る』ボタン
  if( touch == TOUCH_AREA_CHECK_BUTTON ) {
    // 選択可
    if( CheckCursorSetEnable(work, MENU_ITEM_CHECK_RESEARCH) ) {
      SetFinishReason( work, SEQ_CHANGE_BY_TOUCH );       // 画面遷移のトリガを登録
      MoveCursorDirect( work, MENU_ITEM_CHECK_RESEARCH ); // カーソル位置を更新
      StopPaletteAnime( work, PALETTE_ANIME_CURSOR_ON );  // カーソルONパレットアニメ終了
      StartPaletteAnime( work, PALETTE_ANIME_SELECT );    // 選択パレットアニメ開始
      PMSND_PlaySE( SEQ_SE_DECIDE1 );                     // 決定音
      SetFinishResult( work, RRT_RESULT_TO_GRAPH );       // 画面終了結果を決定
      SetWaitFrame( work, WAIT_FRAME_BUTTON );            // 待ち時間を設定
      FinishNowState( work );                             // RRT_STATE_KEYWAIT 状態を終了
      RegisterNextState( work, RRT_STATE_WAIT );          // => RRT_STATE_WAIT
      RegisterNextState( work, RRT_STATE_FADEOUT );       // ==> RRT_STATE_FADEOUT       
      RegisterNextState( work, RRT_STATE_CLEANUP );       // ===> RRT_STATE_CLEANUP      
    }
    // 選択不可
    else {
      PMSND_PlaySE( SEQ_SE_BEEP );
    }
    return;
  }

  //----------
  // B ボタン
  if( trg & PAD_BUTTON_B ) {
    BlinkReturnButton( work );                    //『戻る』ボタンを光らせる
    PMSND_PlaySE( SEQ_SE_CANCEL1 );               // キャンセル音
    SetFinishResult( work, RRT_RESULT_EXIT );     // 画面終了結果を決定
    SetWaitFrame( work, WAIT_FRAME_BUTTON );      // 待ち時間を設定
    FinishNowState( work );                       // RRT_STATE_KEYWAIT 状態を終了
    RegisterNextState( work, RRT_STATE_WAIT );    // => RRT_STATE_WAIT
    RegisterNextState( work, RRT_STATE_FADEOUT ); // ==> RRT_STATE_FADEOUT       
    RegisterNextState( work, RRT_STATE_CLEANUP ); // ===> RRT_STATE_CLEANUP      
    return;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief フレーム経過待ち状態 ( RRT_STATE_WAIT ) の処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void Main_WAIT( RRT_WORK* work )
{
  // 待ち時間が経過
  if( GetWaitFrame(work) < GetStateCount(work) ) {
    FinishNowState( work );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief 後片付け状態 ( RRT_STATE_FADEOUT ) の処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void Main_FADEOUT( RRT_WORK* work )
{
  switch( GetStateSeq(work) ) {
  // フェードアウト開始
  case 0:
    StartFadeOut(); 
    IncStateSeq( work );
    break;

  // フェード完了待ち
  case 1:
    if( GFL_FADE_CheckFade() == FALSE ) {
      FinishNowState( work );
    } 
    break;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief 後片付け状態 ( RRT_STATE_CLEANUP ) の処理
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void Main_CLEANUP( RRT_WORK* work )
{
  ResetVBlankFunc ( work );

  // 共通パレットアニメーション
  RRC_StopAllPaletteAnime( work->commonWork ); // 停止して, 
  RRC_ResetAllPalette( work->commonWork );     // パレットを元に戻す

  // パレットアニメーション
  CleanUpPaletteAnime( work );
  DeletePaletteAnime( work );

  // OBJ 後片付け
  DeleteClactWorks( work );
  DeleteClactUnits( work );
  ReleaseSubOBJResources ( work );
  ReleaseMainOBJResources( work );

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

  work->finishFlag = TRUE;
  RegisterNextState( work, RRT_STATE_FINISH );
  FinishNowState( work );
}

//------------------------------------------------------------------------------------
/**
 * @brief 次の状態をキューに登録する
 *
 * @param work
 * @param nextState 登録する状態
 */
//------------------------------------------------------------------------------------
static void RegisterNextState( RRT_WORK* work, RRT_STATE nextState )
{
  // 状態キューに追加する
  QUEUE_EnQueue( work->stateQueue, nextState );
}

//------------------------------------------------------------------------------------
/**
 * @brief 現在の状態を終了する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void FinishNowState( RRT_WORK* work )
{
  // すでに終了済み
  GF_ASSERT( work->nowStateEndFlag == FALSE );

  // 終了フラグを立てる
  work->nowStateEndFlag = TRUE;
}

//------------------------------------------------------------------------------------
/**
 * @brief 状態を変更する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SwitchState( RRT_WORK* work )
{
  RRT_STATE now_seq, next_seq;

  if( work->nowStateEndFlag == FALSE ){ return; }  // 現在の状態が終了していない
  if( QUEUE_IsEmpty( work->stateQueue ) ){ return; } // 状態キューに登録されていない

  now_seq  = work->state;
  next_seq = QUEUE_DeQueue( work->stateQueue );

  // 状態を更新
  SetState( work, next_seq ); 
} 

//------------------------------------------------------------------------------------
/**
 * @brief 状態を設定する
 *
 * @param work
 * @parma nextState 設定する状態
 */
//------------------------------------------------------------------------------------
static void SetState( RRT_WORK* work, RRT_STATE nextState )
{ 
  // 更新
  work->state           = nextState;
  work->stateSeq        = 0;
  work->stateCount      = 0;
  work->nowStateEndFlag = FALSE;
}

//------------------------------------------------------------------------------------
/**
 * @brief 状態内シーケンス番号を取得する
 *
 * @param work
 *
 * @return 状態内シーケンス番号
 */
//------------------------------------------------------------------------------------
static u32 GetStateSeq( const RRT_WORK* work )
{
  return work->stateSeq;
}

//------------------------------------------------------------------------------------
/**
 * @brief 状態内シーケンス番号をインクリメントする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void IncStateSeq( RRT_WORK* work )
{
  (work->stateSeq)++;
}

//------------------------------------------------------------------------------------
/**
 * @brief 最初の状態を取得する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static RRT_STATE GetFirstState( const RRT_WORK* work )
{
  RRC_WORK* commonWork;
  RADAR_SEQ prev_seq;
  SEQ_CHANGE_TRIG trig;
  MENU_ITEM cursor_pos;

  commonWork = work->commonWork;
  prev_seq   = RRC_GetPrevSeq( commonWork );
  trig       = RRC_GetSeqChangeTrig( commonWork );

  // 前の画面をボタンで終了
  if( (prev_seq != RADAR_SEQ_NULL) && (trig == SEQ_CHANGE_BY_BUTTON) ) {
    return RRT_STATE_KEYWAIT;
  }
  else {
    return RRT_STATE_STANDBY;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief トップ画面の終了理由を登録する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetFinishReason( const RRT_WORK* work, SEQ_CHANGE_TRIG reason )
{
  RRC_SetSeqChangeTrig( work->commonWork, reason );
}

//------------------------------------------------------------------------------------
/**
 * @brief 画面の終了結果を決定する
 *
 * @param work
 * @param result 終了結果
 */
//------------------------------------------------------------------------------------
static void SetFinishResult( RRT_WORK* work, RRT_RESULT result )
{
  work->finishResult = result;
}


//==================================================================================== 
// ◆LAYER 3 機能
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 新しい人物とすれ違ったかどうかをチェックする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CheckNewEntry( RRT_WORK* work )
{
  if( work->newEntryFlag == FALSE ) {
    // 新しい人物とすれ違った
    if( GAMEBEACON_Get_NewEntry() == TRUE ) {
      work->newEntryFlag = TRUE; // フラグを立てる
      NewIconDispOn( work );     // "new" アイコンを表示する
    }
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief カーソルを上へ移動する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void MoveCursorUp( RRT_WORK* work )
{
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
}

//------------------------------------------------------------------------------------
/**
 * @brief カーソルを下へ移動する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void MoveCursorDown( RRT_WORK* work )
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
}

//------------------------------------------------------------------------------------
/**
 * @brief カーソルを直接移動する
 *
 * @param work
 * @param menuItem 移動先のメニュー項目
 */
//------------------------------------------------------------------------------------
static void MoveCursorDirect( RRT_WORK* work, MENU_ITEM menuItem )
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
}

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
  switch( menuItem ) {
  //『調査を決める』
  case MENU_ITEM_CHANGE_RESEARCH:
    BGFrame   = MAIN_BG_WINDOW;
    x         = CHANGE_BUTTON_X;
    y         = CHANGE_BUTTON_Y;
    width     = CHANGE_BUTTON_WIDTH;
    height    = CHANGE_BUTTON_HEIGHT;
    paletteNo = MAIN_BG_PALETTE_WINDOW_ON;
    break;

  //『調査を見る』
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
    return;
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
  switch( menuItem ) {
  //『調査を決める』
  case MENU_ITEM_CHANGE_RESEARCH:
    BGFrame   = MAIN_BG_WINDOW;
    x         = CHANGE_BUTTON_X;
    y         = CHANGE_BUTTON_Y;
    width     = CHANGE_BUTTON_WIDTH;
    height    = CHANGE_BUTTON_HEIGHT;
    paletteNo = MAIN_BG_PALETTE_WINDOW_OFF_CHANGE;
    break;

  //『調査を見る』
  case MENU_ITEM_CHECK_RESEARCH:
    BGFrame   = MAIN_BG_WINDOW;
    x         = CHECK_BUTTON_X;
    y         = CHECK_BUTTON_Y;
    width     = CHECK_BUTTON_WIDTH;
    height    = CHECK_BUTTON_HEIGHT;
    paletteNo = MAIN_BG_PALETTE_WINDOW_OFF_CHECK;
    break;

  // エラー
  default:
    GF_ASSERT(0);
    return;
  }

  // スクリーン更新
  GFL_BG_ChangeScreenPalette( BGFrame, x, y, width, height, paletteNo );
  GFL_BG_LoadScreenReq( BGFrame );
}

//------------------------------------------------------------------------------------
/**
 * @brief『調査を決める』ボタンを非アクティブ状態にする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetChangeButtonNotActive( RRT_WORK* work )
{
  BG_FONT_SetPalette( work->BGFont[ BG_FONT_CHANGE_BUTTON ], MAIN_BG_PALETTE_FONT_NOT_ACTIVE );
  StartPaletteAnime( work, PALETTE_ANIME_CHANGE_BUTTON_HOLD_WINDOW );
  StartPaletteAnime( work, PALETTE_ANIME_CHANGE_BUTTON_HOLD_FONT );
} 

//------------------------------------------------------------------------------------
/**
 * @brief『戻る』ボタンを明滅させる 
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void BlinkReturnButton( RRT_WORK* work )
{
  // パレットアニメ開始
  RRC_StartPaletteAnime( work->commonWork, COMMON_PALETTE_ANIME_RETURN );
}

//------------------------------------------------------------------------------------
/**
 * "new" アイコンを表示する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void NewIconDispOn( const RRT_WORK* work )
{
  GFL_CLWK* clactWork;

  clactWork = GetClactWork( work, CLWK_NEW_ICON );
  GFL_CLACT_WK_SetDrawEnable( clactWork, TRUE );
  GFL_CLACT_WK_SetAutoAnmFlag( clactWork, TRUE );
  GFL_CLACT_WK_SetAutoAnmSpeed( clactWork, FX32_ONE );
  GFL_CLACT_WK_SetAnmFrame( clactWork, 0 );
}

//------------------------------------------------------------------------------------
/**
 * "new" アイコンを非表示にする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void NewIconDispOff( const RRT_WORK* work )
{
  GFL_CLWK* clactWork;

  clactWork = GetClactWork( work, CLWK_NEW_ICON );
  GFL_CLACT_WK_SetDrawEnable( clactWork, FALSE );
}

//------------------------------------------------------------------------------------
/**
 * @brief 画面のフェードインを開始する
 */
//------------------------------------------------------------------------------------
static void StartFadeIn( void )
{
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 
      16, 0, 0);
}

//------------------------------------------------------------------------------------
/**
 * @brief 画面のフェードアウトを開始する
 */
//------------------------------------------------------------------------------------
static void StartFadeOut( void )
{
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 
      0, 16, 0);
}


//====================================================================================
// ◆LAYER 2 取得・設定・判定
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief 状態カウンタを更新する
 * 
 * @param work
 */
//------------------------------------------------------------------------------------
static void CountUpStateCount( RRT_WORK* work )
{
  work->stateCount++;
}

//------------------------------------------------------------------------------------
/**
 * @brief 状態カウンタの値を取得する
 *
 * @param work
 *
 * @return 状態カウンタの値
 */
//------------------------------------------------------------------------------------
static u32 GetStateCount( const RRT_WORK* work )
{
  return work->stateCount;
}

//------------------------------------------------------------------------------------
/**
 * @brief 状態カウンタをリセットする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void ResetStateCount( RRT_WORK* work )
{
  work->stateCount = 0;
}

//------------------------------------------------------------------------------------
/**
 * @brief WAIT 状態の待ち時間を設定する
 *
 * @param work
 * @param frame 待ちフレーム数
 */
//------------------------------------------------------------------------------------
static void SetWaitFrame( RRT_WORK* work, u32 frame )
{
  work->waitFrame = frame;
}

//------------------------------------------------------------------------------------
/**
 * @brief WAIT 状態の待ち時間を取得する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static u32 GetWaitFrame( const RRT_WORK* work )
{
  return work->waitFrame;
}

//------------------------------------------------------------------------------------
/**
 * @brief カーソル位置を変更する ( オフセット指定 )
 *
 * @param work
 * @param offset 移動量
 */
//------------------------------------------------------------------------------------
static void ShiftCursorPos( RRT_WORK* work, int offset )
{
  int nowPos;
  int nextPos;

  nowPos  = work->cursorPos;
  nextPos = nowPos;

  // 更新後のカーソル位置を決定
  do {
    nextPos = (nextPos + offset + MENU_ITEM_NUM) % MENU_ITEM_NUM;
  } while( CheckCursorSetEnable( work, nextPos ) == FALSE );

  // カーソル位置を更新
  work->cursorPos = nextPos;
}

//------------------------------------------------------------------------------------
/**
 * @brief カーソル位置を変更する ( 直値指定 )
 *
 * @param work
 * @param menuItem 移動先
 */
//------------------------------------------------------------------------------------
static void SetCursorPos( RRT_WORK* work, MENU_ITEM menuItem )
{
  // カーソル位置を更新
  work->cursorPos = menuItem;
}

//------------------------------------------------------------------------------------
/**
 * @brief カーソルを指定位置にセットできるかどうかをチェックする
 *
 * @param work
 * @param menuItem チェック対象の項目
 *
 * @return 指定したメニュー項目にカーソルをセットできる場合 TRUE
 *         そうでなければ FALSE
 */
//------------------------------------------------------------------------------------
static BOOL CheckCursorSetEnable( RRT_WORK* work, MENU_ITEM menuItem )
{
  //『調査を決める』
  if( menuItem == MENU_ITEM_CHANGE_RESEARCH ) {
    // 選択可能な項目が２個以上
    if( 1 < GetSelectableTopicNum(work) ) {
      return TRUE;
    }
    // 選択可能な項目は１つだけ
    else {
      return FALSE;
    }
  }

  //『調査を決める』以外は無条件でセット可能
  return TRUE;
}

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
static u32 GetOBJResRegisterIndex( const RRT_WORK* work, OBJ_RESOURCE_ID resID )
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
static GFL_CLUNIT* GetClactUnit( const RRT_WORK* work, CLUNIT_INDEX unitIdx )
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
static GFL_CLWK* GetClactWork( const RRT_WORK* work, CLWK_INDEX wkIdx )
{
  return work->clactWork[ wkIdx ];
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを開始する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void StartPaletteAnime( RRT_WORK* work, PALETTE_ANIME_INDEX index )
{
  PALETTE_ANIME_Start( work->paletteAnime[ index ], 
                       PaletteAnimeData[ index ].animeType,
                       PaletteAnimeData[ index ].fadeColor );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを停止する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void StopPaletteAnime( RRT_WORK* work, PALETTE_ANIME_INDEX index )
{
  PALETTE_ANIME_Stop( work->paletteAnime[ index ] );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを更新する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void UpdatePaletteAnime( RRT_WORK* work )
{
  int i;

  for( i=0; i < PALETTE_ANIME_NUM; i++ )
  {
    GF_ASSERT( work->paletteAnime[i] );

    PALETTE_ANIME_Update( work->paletteAnime[i] );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief 選択可能な調査項目の数を取得する
 *
 * @param work
 *
 * @return 選択可能な調査項目の数
 */
//------------------------------------------------------------------------------------
static u8 GetSelectableTopicNum( const RRT_WORK* work )
{
  GAMEDATA* gameData;
  EVENTWORK* evwork;
  int num;

  gameData = RRC_GetGameData( work->commonWork );
  evwork   = GAMEDATA_GetEventWork( gameData );

  // アンケートに答えた数をカウント
  num = 1; // 先頭項目は最初から選べる
  if( EVENTWORK_CheckEventFlag( evwork, FE_RES_QUESTION_YOU      ) == TRUE ) { num++; }
  if( EVENTWORK_CheckEventFlag( evwork, FE_RES_QUESTION_FAVARITE ) == TRUE ) { num++; }
  if( EVENTWORK_CheckEventFlag( evwork, FE_RES_QUESTION_WISH     ) == TRUE ) { num++; }
  if( EVENTWORK_CheckEventFlag( evwork, FE_RES_QUESTION_PARTNER  ) == TRUE ) { num++; }
  if( EVENTWORK_CheckEventFlag( evwork, FE_RES_QUESTION_TASTE    ) == TRUE ) { num++; }
  if( EVENTWORK_CheckEventFlag( evwork, FE_RES_QUESTION_HOBBY    ) == TRUE ) { num++; }
  if( EVENTWORK_CheckEventFlag( evwork, FE_RES_QUESTION_SCHOOL   ) == TRUE ) { num++; }
  if( EVENTWORK_CheckEventFlag( evwork, FE_RES_QUESTION_PLAY     ) == TRUE ) { num++; }
  if( EVENTWORK_CheckEventFlag( evwork, FE_RES_QUESTION_POKEMON  ) == TRUE ) { num++; }

  return num;
}

//------------------------------------------------------------------------------------
/**
 * @brief 全画面共通ワークを取得する
 *
 * @param work
 *
 * @return 全画面共通ワーク
 */
//------------------------------------------------------------------------------------
static RRC_WORK* GetCommonWork( const RRT_WORK* work )
{
  return work->commonWork;
}

//------------------------------------------------------------------------------------
/**
 * @brief 全画面共通ワークを設定する
 *
 * @param work
 * @param commonWork
 */
//------------------------------------------------------------------------------------
static void SetCommonWork( RRT_WORK* work, RRC_WORK* commonWork )
{
  work->commonWork = commonWork;
}

//------------------------------------------------------------------------------------
/**
 * @brief ヒープIDを取得する
 *
 * @param work
 *
 * @return ヒープID
 */
//------------------------------------------------------------------------------------
static HEAPID GetHeapID( const RRT_WORK* work )
{
  return work->heapID;
}

//------------------------------------------------------------------------------------
/**
 * @brief ヒープIDを設定する
 *
 * @param work
 * @param heapID
 */
//------------------------------------------------------------------------------------
static void SetHeapID( RRT_WORK* work, HEAPID heapID )
{
  work->heapID = heapID;
}

//------------------------------------------------------------------------------------
/**
 * @brief 強制終了フラグをチェックする
 */
//------------------------------------------------------------------------------------
static BOOL CheckForceReturnFlag( const RRT_WORK* work )
{
  const RRC_WORK* commonWork = GetCommonWork( work );
  return RRC_GetForceReturnFlag( commonWork );
}


//====================================================================================
// ◆LAYER 1 生成・初期化・破棄
//====================================================================================

//------------------------------------------------------------------------------------
/**
 * @brief トップ画面ワークを生成する
 *
 * @param heapID
 *
 * @return トップ画面ワーク
 */
//------------------------------------------------------------------------------------
static RRT_WORK* CreateTopWork( HEAPID heapID )
{
  RRT_WORK* work;

  work = GFL_HEAP_AllocMemory( heapID, sizeof(RRT_WORK) );

  return work;
}

//------------------------------------------------------------------------------------
/**
 * @brief トップ画面ワークを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeleteTopWork( RRT_WORK* work )
{
  GFL_HEAP_FreeMemory( work );
}

//------------------------------------------------------------------------------------
/**
 * @brief トップ画面ワークを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitTopWork( RRT_WORK* work )
{
  work->state           = RRT_STATE_SETUP;
  work->nowStateEndFlag = FALSE;
  work->stateCount      = 0;
  work->waitFrame       = 0;
  work->cursorPos       = MENU_ITEM_CHANGE_RESEARCH;
  work->newEntryFlag    = FALSE;
  work->finishFlag      = FALSE;
  work->finishResult    = RRT_RESULT_EXIT;
  work->stateSeq        = 0;

  InitOBJResources( work );
  InitStateQueue( work );
  InitMessages( work );
  InitFont( work );
  InitBGFonts( work );
  InitClactUnits( work );
  InitClactWorks( work );
  InitPaletteAnime( work );
}

//------------------------------------------------------------------------------------
/**
 * @brief カーソル位置をセットアップする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupCursorPos( RRT_WORK* work )
{
  RRC_WORK* commonWork;
  RADAR_SEQ prev_seq;
  SEQ_CHANGE_TRIG trig;
  MENU_ITEM cursor_pos;

  commonWork = work->commonWork;
  prev_seq   = RRC_GetPrevSeq( commonWork );
  trig       = RRC_GetSeqChangeTrig( commonWork );

  // 前の画面をボタンで終了
  if( (prev_seq != RADAR_SEQ_NULL) && (trig == SEQ_CHANGE_BY_BUTTON) ) {
    // カーソル位置を初期化
    switch( prev_seq ) {
    default: GF_ASSERT(0);
    case RADAR_SEQ_SELECT: cursor_pos = MENU_ITEM_CHANGE_RESEARCH; break;
    case RADAR_SEQ_CHECK:  cursor_pos = MENU_ITEM_CHECK_RESEARCH;  break;
    }
    SetCursorPos( work, cursor_pos );
  }
  else {
    //『調査を決める』を選択可
    if( CheckCursorSetEnable( work, MENU_ITEM_CHANGE_RESEARCH ) ) {
      SetCursorPos( work, MENU_ITEM_CHANGE_RESEARCH );
    }
    //『調査を決める』を選択不可
    else {
      SetCursorPos( work, MENU_ITEM_CHECK_RESEARCH );
    }
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief フォントデータを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitFont( RRT_WORK* work )
{
  work->font = NULL;
}

//------------------------------------------------------------------------------------
/**
 * @brief フォントデータを生成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreateFont( RRT_WORK* work )
{
  GF_ASSERT( work->font == NULL ); // 初期化されていない

  work->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, 
                                GFL_FONT_LOADTYPE_FILE, FALSE, work->heapID ); 
}

//------------------------------------------------------------------------------------
/**
 * @brief フォントデータを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeleteFont( RRT_WORK* work )
{
  GF_ASSERT( work->font ); // 生成されていない

  GFL_FONT_Delete( work->font );
}

//------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitMessages( RRT_WORK* work )
{
  int i;

  for( i=0; i < MESSAGE_NUM; i++ )
  {
    work->message[i] = NULL;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを作成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreateMessages( RRT_WORK* work )
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
}

//------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeleteMessages( RRT_WORK* work )
{
  int i;

  for( i=0; i < MESSAGE_NUM; i++ )
  {
    GF_ASSERT( work->message[i] ); // 生成されてない

    GFL_MSG_Delete( work->message[i] );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief タッチ領域をセットアップする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupTouchArea( RRT_WORK* work )
{
  int i;

  for( i=0; i < TOUCH_AREA_NUM; i++ )
  {
    work->touchHitTable[i].rect.left   = TouchAreaInitData[i].left;
    work->touchHitTable[i].rect.right  = TouchAreaInitData[i].right;
    work->touchHitTable[i].rect.top    = TouchAreaInitData[i].top;
    work->touchHitTable[i].rect.bottom = TouchAreaInitData[i].bottom;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief 状態キューを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitStateQueue( RRT_WORK* work )
{
  work->stateQueue = NULL;
}

//------------------------------------------------------------------------------------
/**
 * @brief 状態キューを作成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreateStateQueue( RRT_WORK* work )
{
  GF_ASSERT( work->stateQueue == NULL ); // 初期化されていない

  work->stateQueue = QUEUE_Create( SEQ_QUEUE_SIZE, work->heapID );
}

//------------------------------------------------------------------------------------
/**
 * @brief 状態キューを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeleteStateQueue( RRT_WORK* work )
{
  GF_ASSERT( work->stateQueue ); // 生成されていない

  QUEUE_Delete( work->stateQueue );
}

//------------------------------------------------------------------------------------
/**
 * @brief BG の準備
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupBG( RRT_WORK* work )
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
}

//------------------------------------------------------------------------------------
/**
 * @brief BG の後片付け
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CleanUpBG( RRT_WORK* work )
{
  GFL_BMPWIN_Exit();

  GFL_BG_FreeBGControl( MAIN_BG_FONT );
  GFL_BG_FreeBGControl( MAIN_BG_WINDOW );
  GFL_BG_FreeBGControl( SUB_BG_FONT );
  GFL_BG_FreeBGControl( SUB_BG_WINDOW );
}

//------------------------------------------------------------------------------------
/**
 * @brief 上画面 ウィンドウBG面 準備
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupSubBG_WINDOW( RRT_WORK* work )
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
}

//------------------------------------------------------------------------------------
/**
 * @brief 上画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CleanUpSubBG_WINDOW( RRT_WORK* work )
{
}

//------------------------------------------------------------------------------------
/**
 * @brief SUB-BG フォント面の準備
 * 
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupSubBG_FONT( RRT_WORK* work )
{
  // NULLキャラを確保
  GFL_BG_FillCharacter( SUB_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( SUB_BG_FONT );
}

//------------------------------------------------------------------------------------
/**
 * @brief SUB-BG フォント面の後片付け
 * 
 * @param work
 */
//------------------------------------------------------------------------------------
static void CleanUpSubBG_FONT( RRT_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( SUB_BG_FONT, 1, 0 );
}

//------------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 準備
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupMainBG_WINDOW( RRT_WORK* work )
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
}

//------------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CleanUpMainBG_WINDOW( RRT_WORK* work )
{
}

//------------------------------------------------------------------------------------
/**
 * @brief 下画面 フォントBG面 準備
 * 
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupMainBG_FONT( RRT_WORK* work )
{ 
  // NULLキャラ確保
  GFL_BG_FillCharacter( MAIN_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( MAIN_BG_FONT );
}

//------------------------------------------------------------------------------------
/**
 * @brief 下画面 フォントBG面 後片付け
 * 
 * @param work
 */
//------------------------------------------------------------------------------------
static void CleanUpMainBG_FONT( RRT_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( MAIN_BG_FONT, 1, 0 );
}

//------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitBGFonts( RRT_WORK* work )
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
static void CreateBGFonts( RRT_WORK* work )
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
}

//------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeleteBGFonts( RRT_WORK* work )
{
  int i;
  
  // 文字列描画オブジェクト破棄
  for( i=0; i<BG_FONT_NUM; i++ )
  {
    GF_ASSERT( work->BGFont[i] );
    BG_FONT_Delete( work->BGFont[i] );
    work->BGFont[i] = NULL;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief OBJ のリソースを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitOBJResources( RRT_WORK* work )
{
  int i;

  for( i=0; i<OBJ_RESOURCE_NUM; i++ )
  {
    work->objResRegisterIdx[i] = 0;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief SUB-OBJ リソースを登録する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void RegisterSubOBJResources( RRT_WORK* work )
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
}

//------------------------------------------------------------------------------------
/**
 * @brief SUB-OBJ リソースを解放する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void ReleaseSubOBJResources( RRT_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CELL_ANIME ] );
}

//------------------------------------------------------------------------------------
/**
 * @brief MAIN-OBJ リソースを登録する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void RegisterMainOBJResources( RRT_WORK* work )
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
                                       ONE_PALETTE_SIZE*6, 0, 4, 
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
}

//------------------------------------------------------------------------------------
/**
 * @brief MAIN-OBJ リソースを解放する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void ReleaseMainOBJResources( RRT_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CELL_ANIME ] );
}

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitClactUnits( RRT_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < CLUNIT_NUM; unitIdx++ )
  {
    work->clactUnit[ unitIdx ] = NULL;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを作成する
 *
 * @param
 */
//------------------------------------------------------------------------------------
static void CreateClactUnits( RRT_WORK* work )
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
}

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを破棄する
 *
 * @param
 */
//------------------------------------------------------------------------------------
static void DeleteClactUnits( RRT_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < CLUNIT_NUM; unitIdx++ )
  {
    GF_ASSERT( work->clactUnit[ unitIdx ] );
    GFL_CLACT_UNIT_Delete( work->clactUnit[ unitIdx ] );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitClactWorks( RRT_WORK* work )
{
  int wkIdx;

  // 初期化
  for( wkIdx=0; wkIdx < CLWK_NUM; wkIdx++ )
  {
    work->clactWork[ wkIdx ] = NULL;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを作成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreateClactWorks( RRT_WORK* work )
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
}

//------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeleteClactWorks( RRT_WORK* work )
{
  int wkIdx;

  for( wkIdx=0; wkIdx < CLWK_NUM; wkIdx++ )
  {
    // 生成されていない
    GF_ASSERT( work->clactWork[ wkIdx ] );

    // 破棄
    GFL_CLACT_WK_Remove( work->clactWork[ wkIdx ] );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitPaletteAnime( RRT_WORK* work )
{
  int i;

  for( i=0; i < PALETTE_ANIME_NUM; i++ )
  {
    work->paletteAnime[i] = NULL;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを生成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreatePaletteAnime( RRT_WORK* work )
{
  int i;

  for( i=0; i < PALETTE_ANIME_NUM; i++ )
  {
    GF_ASSERT( work->paletteAnime[i] == NULL ); // 初期化されていない

    work->paletteAnime[i] = PALETTE_ANIME_Create( work->heapID );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeletePaletteAnime( RRT_WORK* work )
{
  int i;

  for( i=0; i < PALETTE_ANIME_NUM; i++ )
  {
    GF_ASSERT( work->paletteAnime[i] );

    PALETTE_ANIME_Delete( work->paletteAnime[i] );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークをセットアップする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupPaletteAnime( RRT_WORK* work )
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
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークをクリーンアップする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CleanUpPaletteAnime( RRT_WORK* work )
{
  int i;

  for( i=0; i < PALETTE_ANIME_NUM; i++ )
  {
    GF_ASSERT( work->paletteAnime[i] );

    // 操作していたパレットを元に戻す
    PALETTE_ANIME_Reset( work->paletteAnime[i] );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief VBlank 割り込み関数
 *
 * @param pWork
 */
//------------------------------------------------------------------------------------
static void VBlankClactFunc( void *pWork )
{
  GFL_CLACT_SYS_VBlankFunc();
}

//------------------------------------------------------------------------------------
/**
 * @brief VBlank 割り込みを設定
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetVBlankFunc( RRT_WORK* work )
{
  GFUser_SetVIntrFunc( VBlankClactFunc , NULL );
}

//------------------------------------------------------------------------------------
/**
 * @brief VBlank 割り込みを解除
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void ResetVBlankFunc( RRT_WORK* work )
{
  GFUser_ResetVIntrFunc();
}


//------------------------------------------------------------------------------------
/**
 * @brief 通信アイコンをセットアップする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupWirelessIcon( const RRT_WORK* work )
{
  GFL_NET_ReloadIconTopOrBottom( TRUE, work->heapID );
}
