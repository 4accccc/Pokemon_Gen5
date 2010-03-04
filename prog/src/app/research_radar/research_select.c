////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 調査内容変更画面
 * @file   research_select.c
 * @author obata
 * @date   2010.02.03
 */
////////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "bg_font.h"
#include "queue.h"
#include "research_select.h"
#include "research_select_index.h"
#include "research_select_def.h"
#include "research_select_data.cdat"
#include "research_common.h"
#include "research_common_data.cdat"

#include "system/main.h"             // for HEAPID_xxxx
#include "system/gfl_use.h"          // for GFUser_xxxx
#include "system/palanm.h"           // for PaletteFadeXxxx
#include "system/bmp_oam.h"          // for BmpOam_xxxx
#include "gamesystem/game_beacon.h"  // for GAMEBEACON_xxxx
#include "sound/pm_sndsys.h"         // for PMSND_xxxx
#include "sound/wb_sound_data.sadl"  // for SEQ_SE_XXXX
#include "print/gf_font.h"           // for GFL_FONT_xxxx
#include "print/printsys.h"          // for PRINTSYS_xxxx

#include "arc/arc_def.h"                    // for ARCID_xxxx
#include "arc/research_radar_graphic.naix"  // for NARC_research_radar_xxxx
#include "arc/app_menu_common.naix"         // for NARC_app_menu_common_xxxx
#include "arc/font/font.naix"               // for NARC_font_xxxx
#include "arc/message.naix"                 // for NARC_message_xxxx
#include "msg/msg_research_radar.h"         // for str_xxxx
#include "msg/msg_questionnaire.h"          // for str_xxxx
#include "obj_NANR_LBLDEFS.h"               // for NANR_obj_xxxx
#include "topic_id.h"                       // for TOPIC_ID_xxxx
#include "question_id.h"                    // for QUESTION_ID_xxxx

#include "question_id_topic.cdat"        // for QuestionX_topic[]
#include "string_id_question.cdat"       // for StringID_question[]
#include "string_id_topic_title.cdat"    // for StringID_topicTitle[]
#include "string_id_topic_caption.cdat"  // for StringID_topicCaption[]


//==============================================================================================
// ■調査内容変更画面ワーク
//==============================================================================================
struct _RESEARCH_SELECT_WORK
{ 
  RESEARCH_COMMON_WORK* commonWork; // 全画面共通ワーク
  HEAPID                heapID;
  GAMESYS_WORK*         gameSystem;
  GAMEDATA*             gameData;

  GFL_FONT*    font;
  GFL_MSGDATA* message[ MESSAGE_NUM ];

  QUEUE*                 seqQueue;      // シーケンスキュー
  RESEARCH_SELECT_SEQ    seq;           // 現在のシーケンス
  u32                    seqCount;      // シーケンスカウンタ
  BOOL                   seqFinishFlag; // 現在のシーケンスが終了したかどうか
  RESEARCH_SELECT_RESULT result;        // 画面終了結果

  // メニュー項目
  MENU_ITEM menuCursorPos; // カーソル位置

  // 調査項目
  u8 selectedTopicID;    // 選択した調査項目ID 
  u8 topicCursorPos;     // カーソル位置 ( == 調査項目ID )
  u8 topicCursorNextPos; // 移動後のカーソル位置 

  // スクロール
  int scrollCursorPos;  // スクロールカーソル位置
  int scrollStartPos;   // スクロール開始時のカーソル位置
  int scrollEndPos;     // スクロール終了時のカーソル位置
  int scrollFrames;     // スクロールフレーム数
  int scrollFrameCount; // スクロールフレーム数カウンタ

  // VBlank
  GFL_TCBSYS* VBlankTCBSystem; // VBlank期間中のタスク管理システム
  GFL_TCB*    VBlankTask;      // VBlankタイミング中に行うタスク

  // タッチ領域
  GFL_UI_TP_HITTBL menuTouchHitTable[ MENU_TOUCH_AREA_NUM ];
  GFL_UI_TP_HITTBL topicTouchHitTable[ TOPIC_TOUCH_AREA_NUM ];

  // 文字列描画オブジェクト
  BG_FONT* BGFont[ BG_FONT_NUM ];
  BG_FONT* TopicsBGFont[ TOPIC_ID_NUM ]; // 調査項目

  // OBJ
  u32         objResRegisterIdx[ OBJ_RESOURCE_NUM ]; // リソースの登録インデックス
  GFL_CLUNIT* clactUnit[ CLUNIT_NUM ];               // セルアクターユニット
  GFL_CLWK*   clactWork[ CLWK_NUM ];                 // セルアクターワーク

  // BMP-OAM
  BMPOAM_SYS_PTR BmpOamSystem;                    // BMP-OAM システム
  BMPOAM_ACT_PTR BmpOamActor[ BMPOAM_ACTOR_NUM ]; // BMP-OAMアクター
  GFL_BMP_DATA*  BmpData[ BMPOAM_ACTOR_NUM ];     // 各アクターに対応するビットマップデータ

  // カラーパレット
  PALETTE_FADE_PTR paletteFadeSystem; // パレットフェード処理システム
};


//----------------------------------------------------------------------------------------------
// □LAYER 5 シーケンス動作
//----------------------------------------------------------------------------------------------
// シーケンス処理
static void Main_SETUP        ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_SETUP
static void Main_STANDBY      ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_STANDBY
static void Main_KEY_WAIT     ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_KEY_WAIT
static void Main_SCROLL_WAIT  ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_SCROLL_WAIT
static void Main_CONFIRM      ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_CONFIRM
static void Main_DETERMINE    ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_DETERMINE
static void Main_FADE_IN      ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_FADE_IN 
static void Main_FADE_OUT     ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_FADE_OUT
static void Main_PALETTE_RESET( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_PALETTE_RESET
static void Main_CLEAN_UP     ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_CLEAN_UP

// シーケンス制御
static void CountUpSeqCount( RESEARCH_SELECT_WORK* work ); // シーケンスカウンタを更新する
static void SetNextSequence( RESEARCH_SELECT_WORK* work, RESEARCH_SELECT_SEQ nextSeq ); // 次のシーケンスを登録する
static void FinishCurrentSequence( RESEARCH_SELECT_WORK* work ); // 現在のシーケンスを終了する
static void SwitchSequence( RESEARCH_SELECT_WORK* work ); // 処理シーケンスを変更する
static void SetSequence   ( RESEARCH_SELECT_WORK* work, RESEARCH_SELECT_SEQ nextSeq ); // 処理シーケンスを設定する
static void SetResult( RESEARCH_SELECT_WORK* work, RESEARCH_SELECT_RESULT result ); // 画面終了結果を設定する

// シーケンス初期化処理
static void InitSequence_SETUP        ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_SETUP
static void InitSequence_STANDBY      ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_STANDBY
static void InitSequence_KEY_WAIT     ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_KEY_WAIT
static void InitSequence_SCROLL_WAIT  ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_SCROLL_WAIT
static void InitSequence_CONFIRM      ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_CONFIRM
static void InitSequence_DETERMINE    ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_DETERMINE
static void InitSequence_FADE_IN      ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_FADE_IN 
static void InitSequence_FADE_OUT     ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_FADE_OUT
static void InitSequence_PALETTE_RESET( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_PALETTE_RESET
static void InitSequence_CLEAN_UP     ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_CLEAN_UP

// シーケンス終了処理
static void FinishSequence_SETUP        ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_SETUP
static void FinishSequence_STANDBY      ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_STANDBY
static void FinishSequence_KEY_WAIT     ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_KEY_WAIT
static void FinishSequence_SCROLL_WAIT  ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_SCROLL_WAIT
static void FinishSequence_CONFIRM      ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_CONFIRM
static void FinishSequence_DETERMINE    ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_DETERMINE
static void FinishSequence_FADE_IN      ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_FADE_IN 
static void FinishSequence_FADE_OUT     ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_FADE_OUT
static void FinishSequence_PALETTE_RESET( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_PALETTE_RESET
static void FinishSequence_CLEAN_UP     ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_CLEAN_UP

// VBlankタスク
static void VBlankFunc( GFL_TCB* tcb, void* wk );  // VBlank中の処理

//----------------------------------------------------------------------------------------------
// □LAYER 3 機能
//---------------------------------------------------------------------------------------------- 
// メニュー項目カーソル
static void MoveMenuCursorUp  ( RESEARCH_SELECT_WORK* work ); // 上へ移動する
static void MoveMenuCursorDown( RESEARCH_SELECT_WORK* work ); // 下へ移動する

// 調査項目カーソル
static void MoveTopicCursorUp    ( RESEARCH_SELECT_WORK* work ); // 上へ移動する
static void MoveTopicCursorDown  ( RESEARCH_SELECT_WORK* work ); // 下へ移動する
static void MoveTopicCursorDirect( RESEARCH_SELECT_WORK* work, u8 topicID ); // 直接移動する

//----------------------------------------------------------------------------------------------
// □LAYER 3 個別操作
//---------------------------------------------------------------------------------------------- 
// メニュー項目カーソル
static void ShiftMenuCursorPos( RESEARCH_SELECT_WORK* work, int stride ); // メニュー項目カーソル位置を変更する

// 調査項目カーソル
static void SetTopicCursorNextPos( RESEARCH_SELECT_WORK* work, int stride ); // 調査項目カーソルの移動先を設定する ( オフセット移動 )
static void SetTopicCursorPosDirect( RESEARCH_SELECT_WORK* work, int topciID ); // 調査項目カーソルの移動先を設定する ( ダイレクト移動 )

// タッチ範囲
static void UpdateTopicTouchArea( RESEARCH_SELECT_WORK* work ); // タッチ範囲を更新する

// 選択した調査項目ID
static u8 GetSelectedTopicID( const RESEARCH_SELECT_WORK* work ); // 選択中の調査項目IDを取得する
static void SetSelectedTopicID( RESEARCH_SELECT_WORK* work, u8 topicID ); // 調査項目IDを選択する
static void ResetSelectedTopicID( RESEARCH_SELECT_WORK* work ); // 調査項目IDの選択を解除する
static BOOL IsTopicIDSelected( const RESEARCH_SELECT_WORK* work ); // 選択済みかを判定する

// メニュー項目の表示
static void SetMenuCursorOn( RESEARCH_SELECT_WORK* work ); // カーソルが乗っている状態にする
static void SetMenuCursorOff( RESEARCH_SELECT_WORK* work ); // カーソルが乗っていない状態にする

// 調査項目の表示
static void SetTopicButtonCursorOn ( const RESEARCH_SELECT_WORK* work ); // カーソルが乗っている状態にする
static void SetTopicButtonCursorOff( const RESEARCH_SELECT_WORK* work ); // カーソルが乗っていない状態にする
static void SetTopicButtonInvestigating( const RESEARCH_SELECT_WORK* work, u8 topicID ); // 調査中の状態にする
static void SetTopicButtonNotInvestigating( const RESEARCH_SELECT_WORK* work, u8 topicID ); // 調査中でない状態に戻す
static void UpdateSubDisplayStrings( RESEARCH_SELECT_WORK* work ); // 上画面のカーソル依存文字列表示を更新する
static u8  CalcTopicButtonScreenLeft( const RESEARCH_SELECT_WORK* work, u8 topicID ); // 調査項目の左上x座標を算出する (スクリーン単位)
static u8  CalcTopicButtonScreenTop ( const RESEARCH_SELECT_WORK* work, u8 topicID ); // 調査項目の左上y座標を算出する (スクリーン単位)
static int CalcTopicButtonDisplayLeft( const RESEARCH_SELECT_WORK* work, u8 topicID ); // 調査項目ボタンの左上x座標を算出する (ディスプレイ座標系・ドット単位)
static int CalcTopicButtonDisplayTop ( const RESEARCH_SELECT_WORK* work, u8 topicID ); // 調査項目ボタンの左上y座標を算出する (ディスプレイ座標系・ドット単位)

// スクロールバー
static void UpdateScrollControlPos( const RESEARCH_SELECT_WORK* work ); // スクロールバーのつまみ部分の位置を更新する
static int CalcScrollControlPos(); // スクロールバーのつまみ部分の位置を計算する

// スクロール
static void StartScroll( RESEARCH_SELECT_WORK* work, int startPos, int endPos, int frames ); // スクロールを開始する
static void UpdateScroll( RESEARCH_SELECT_WORK* work ); // スクロールを更新する
static void FinishScroll( RESEARCH_SELECT_WORK* work ); // スクロール完了時の処理
static BOOL CheckScrollEnd( RESEARCH_SELECT_WORK* work ); // スクロールが終了したかどうかを判定する
static void ShiftScrollCursor( RESEARCH_SELECT_WORK* work, int offset ); // スクロールカーソル位置を変更する ( オフセット指定 )
static void SetScrollCursor( RESEARCH_SELECT_WORK* work, int pos ); // スクロールカーソル位置を変更する ( 直値指定 )
static void AdjustScrollCursor( RESEARCH_SELECT_WORK* work ); // スクロールカーソル位置を補正する
static void UpdateScrollValue( const RESEARCH_SELECT_WORK* work ); // スクロール実効値を更新する
static int GetScrollValue(); // スクロール実効値を取得する
static int GetMinScrollCursorMarginPos(); // スクロールカーソル余白範囲の最小値を取得する
static int GetMaxScrollCursorMarginPos(); // スクロールカーソル余白範囲の最大値を取得する

// 調査中アイコン
static void UpdateTopicSelectIcon( const RESEARCH_SELECT_WORK* work ); // 調査中アイコンの表示状態を更新する

// パレットフェード
static void StartPaletteFadeOut( RESEARCH_SELECT_WORK* work ); // パレットのフェードアウトを開始する
static void StartPaletteFadeIn ( RESEARCH_SELECT_WORK* work ); // パレットのフェードインを開始する
static BOOL IsPaletteFadeEnd   ( RESEARCH_SELECT_WORK* work ); // パレットのフェードが完了したかどうかを判定する

// BMP-OAM
static void BmpOamSetDrawEnable( RESEARCH_SELECT_WORK* work, BMPOAM_ACTOR_INDEX BmpOamActorIdx, BOOL enable );  // 表示するかどうかを設定する

//----------------------------------------------------------------------------------------------
// □LAYER 2 データ アクセス
//----------------------------------------------------------------------------------------------
// OBJ
static u32 GetObjResourceRegisterIndex( const RESEARCH_SELECT_WORK* work, OBJ_RESOURCE_ID resID ); // OBJリソースの登録インデックス
static GFL_CLUNIT* GetClactUnit( const RESEARCH_SELECT_WORK* work, CLUNIT_INDEX unitIdx ); // セルアクターユニット
static GFL_CLWK* GetClactWork( const RESEARCH_SELECT_WORK* work, CLWK_INDEX wkIdx ); // セルアクターワーク

// BMP-OAM
static BMPOAM_ACT_PTR GetBmpOamActorOfMenuItem( const RESEARCH_SELECT_WORK* work, MENU_ITEM menuItem ); // メニュー項目に対応するBMP-OAMアクター


//----------------------------------------------------------------------------------------------
// □LAYER 1 セーブデータ アクセス
//----------------------------------------------------------------------------------------------
static u8 GetInvestigatingTopicID( const RESEARCH_SELECT_WORK* work ); // 現在調査中の調査項目IDを取得する
static void UpdateInvestigatingTopicID( const RESEARCH_SELECT_WORK* work ); // 調査する項目を更新する

//----------------------------------------------------------------------------------------------
// □LAYER 0 初期化処理/終了処理
//----------------------------------------------------------------------------------------------
// 画面の準備/後片付け ( BG )
static void SetupBG  ( RESEARCH_SELECT_WORK* work ); // BG 準備
static void CleanUpBG( RESEARCH_SELECT_WORK* work ); // BG 後片付け
static void SetupSubBG_WINDOW  ( RESEARCH_SELECT_WORK* work ); // SUB-BG ( ウィンドウ面 ) 準備
static void CleanUpSubBG_WINDOW( RESEARCH_SELECT_WORK* work ); // SUB-BG ( ウィンドウ面 ) 後片付け
static void SetupSubBG_FONT  ( RESEARCH_SELECT_WORK* work ); // SUB-BG ( フォント面 ) 準備
static void CleanUpSubBG_FONT( RESEARCH_SELECT_WORK* work ); // SUB-BG ( フォント面 ) 後片付け
static void SetupMainBG_BAR  ( RESEARCH_SELECT_WORK* work ); // MAIN-BG ( バー面 ) 準備
static void CleanUpMainBG_BAR( RESEARCH_SELECT_WORK* work ); // MAIN-BG ( バー面 ) 後片付け
static void SetupMainBG_WINDOW  ( RESEARCH_SELECT_WORK* work ); // MAIN-BG ( ウィンドウ面 ) 準備
static void CleanUpMainBG_WINDOW( RESEARCH_SELECT_WORK* work ); // MAIN-BG ( ウィンドウ面 ) 後片付け
static void SetupMainBG_FONT  ( RESEARCH_SELECT_WORK* work ); // MAIN-BG ( フォント面 ) 準備
static void CleanUpMainBG_FONT( RESEARCH_SELECT_WORK* work ); // MAIN-BG ( フォント面 ) 後片付け

// 画面の準備/後片付け ( OBJ )
static void CreateClactSystem( RESEARCH_SELECT_WORK* work ); // OBJ システム 生成
static void DeleteClactSystem( RESEARCH_SELECT_WORK* work ); // OBJ システム 破棄
static void InitOBJResources( RESEARCH_SELECT_WORK* work );  // OBJ リソース 初期化
static void RegisterSubObjResources( RESEARCH_SELECT_WORK* work ); // SUB-OBJ リソース 登録
static void ReleaseSubObjResources ( RESEARCH_SELECT_WORK* work ); // SUB-OBJ リソース 解放
static void RegisterMainObjResources( RESEARCH_SELECT_WORK* work ); // MAIN-OBJ リソース 登録
static void ReleaseMainObjResources ( RESEARCH_SELECT_WORK* work ); // MAIN-OBJ リソース 解放
static void InitClactUnits  ( RESEARCH_SELECT_WORK* work ); // セルアクターユニット 初期化
static void CreateClactUnits( RESEARCH_SELECT_WORK* work ); // セルアクターユニット 生成
static void DeleteClactUnits( RESEARCH_SELECT_WORK* work ); // セルアクターユニット 破棄
static void InitClactWorks  ( RESEARCH_SELECT_WORK* work ); // セルアクターワーク 初期化
static void CreateClactWorks( RESEARCH_SELECT_WORK* work ); // セルアクターワーク 生成
static void DeleteClactWorks( RESEARCH_SELECT_WORK* work ); // セルアクターワーク 破棄

// BMP-OAM の準備/後片付け
static void InitBitmapDatas        ( RESEARCH_SELECT_WORK* work ); // ビットマップデータ 初期化
static void CreateBitmapDatas      ( RESEARCH_SELECT_WORK* work ); // ビットマップデータ 作成
static void SetupBitmapData_forDefault( RESEARCH_SELECT_WORK* work ); // ビットマップデータ 準備 ( デフォルト )
static void SetupBitmapData_forOK( RESEARCH_SELECT_WORK* work ); // ビットマップデータ 準備 (「けってい」)
static void SetupBitmapData_forCANCEL( RESEARCH_SELECT_WORK* work ); // ビットマップデータ 準備 (「やめる」)
static void DeleteBitmapDatas( RESEARCH_SELECT_WORK* work ); // ビットマップデータ 破棄
static void SetupBmpOamSystem  ( RESEARCH_SELECT_WORK* work ); // BMP-OAM システム 準備
static void CleanUpBmpOamSystem( RESEARCH_SELECT_WORK* work ); // BMP-OAM システム 後片付け
static void CreateBmpOamActors( RESEARCH_SELECT_WORK* work ); // BMP-OAM アクター 作成
static void DeleteBmpOamActors( RESEARCH_SELECT_WORK* work ); // BMP-OAM アクター 破棄

// VBlankタスクの登録解除
static void RegisterVBlankTask( RESEARCH_SELECT_WORK* work );
static void ReleaseVBlankTask ( RESEARCH_SELECT_WORK* work );

// パレットフェードシステムの準備/後片付け
static void InitPaletteFadeSystem   ( RESEARCH_SELECT_WORK* work ); // パレットフェードシステム 初期化
static void SetupPaletteFadeSystem  ( RESEARCH_SELECT_WORK* work ); // パレットフェードシステム 準備
static void CleanUpPaletteFadeSystem( RESEARCH_SELECT_WORK* work ); // パレットフェードシステム 後片付け

// データの初期化/生成/破棄
static void InitSeqQueue  ( RESEARCH_SELECT_WORK* work ); // シーケンスキュー 初期化
static void CreateSeqQueue( RESEARCH_SELECT_WORK* work ); // シーケンスキュー 作成
static void DeleteSeqQueue( RESEARCH_SELECT_WORK* work ); // シーケンスキュー 破棄
static void InitFont  ( RESEARCH_SELECT_WORK* work ); // フォント 初期化
static void CreateFont( RESEARCH_SELECT_WORK* work ); // フォント 生成
static void DeleteFont( RESEARCH_SELECT_WORK* work ); // フォント 破棄
static void InitMessages  ( RESEARCH_SELECT_WORK* work ); // メッセージ 初期化
static void CreateMessages( RESEARCH_SELECT_WORK* work ); // メッセージ 生成
static void DeleteMessages( RESEARCH_SELECT_WORK* work ); // メッセージ 破棄
static void InitBGFonts  ( RESEARCH_SELECT_WORK* work ); // 文字列描画オブジェクト 初期化
static void CreateBGFonts( RESEARCH_SELECT_WORK* work ); // 文字列描画オブジェクト 生成
static void DeleteBGFonts( RESEARCH_SELECT_WORK* work ); // 文字列描画オブジェクト 破棄
static void SetupTouchArea( RESEARCH_SELECT_WORK* work ); // タッチ領域 準備

//----------------------------------------------------------------------------------------------
// □LAYER -1 デバッグ
//----------------------------------------------------------------------------------------------
// DEBUG:
static void DebugPrint_seqQueue( const RESEARCH_SELECT_WORK* work ); // シーケンスキューの中身を表示する


//==============================================================================================
// □調査初期画面 制御関数
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査初期画面ワークの生成
 *
 * @param commonWork 全画面共通ワーク
 *
 * @return 調査内容変更画面ワーク
 */
//----------------------------------------------------------------------------------------------
RESEARCH_SELECT_WORK* CreateResearchSelectWork( RESEARCH_COMMON_WORK* commonWork )
{
  int i;
  RESEARCH_SELECT_WORK* work;
  HEAPID heapID;

  heapID = RESEARCH_COMMON_GetHeapID( commonWork );

  // 生成
  work = GFL_HEAP_AllocMemory( heapID, sizeof(RESEARCH_SELECT_WORK) );

  // 初期化
  work->commonWork            = commonWork;
  work->heapID                = heapID;
  work->gameSystem            = RESEARCH_COMMON_GetGameSystem( commonWork );
  work->gameData              = RESEARCH_COMMON_GetGameData( commonWork );
  work->seq                   = RESEARCH_SELECT_SEQ_SETUP;
  work->seqCount              = 0;
  work->seqFinishFlag         = FALSE;
  work->result                = RESEARCH_SELECT_RESULT_NONE;
  work->menuCursorPos         = MENU_ITEM_DETERMINATION_OK;
  work->topicCursorPos        = 0;
  work->topicCursorNextPos    = 0;
  work->selectedTopicID       = TOPIC_ID_DUMMY;
  work->VBlankTCBSystem       = GFUser_VIntr_GetTCBSYS();
  work->scrollCursorPos       = 0;
  work->scrollStartPos        = 0;
  work->scrollEndPos          = 0;
  work->scrollFrames          = 0;
  work->scrollFrameCount      = 0;

  InitSeqQueue( work );
  InitMessages( work );
  InitFont( work );
  InitBGFonts( work );
  InitClactUnits( work );
  InitClactWorks( work );
  InitOBJResources( work );
  InitPaletteFadeSystem( work );
  InitBitmapDatas( work );

  CreateSeqQueue( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: create work\n" );

  return work;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査初期画面ワークの破棄
 *
 * @param heapID
 */
//----------------------------------------------------------------------------------------------
void DeleteResearchSelectWork( RESEARCH_SELECT_WORK* work )
{
  if( work == NULL )
  {
    GF_ASSERT(0);
    return;
  }
  DeleteSeqQueue( work ); // シーケンスキュー
  GFL_HEAP_FreeMemory( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: delete work\n" );
} 

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査初期画面 メイン動作
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
RESEARCH_SELECT_RESULT ResearchSelectMain( RESEARCH_SELECT_WORK* work )
{
  // シーケンスごとの処理
  switch( work->seq )
  {
  case RESEARCH_SELECT_SEQ_SETUP:        Main_SETUP( work );        break;
  case RESEARCH_SELECT_SEQ_STANDBY:      Main_STANDBY( work );      break;
  case RESEARCH_SELECT_SEQ_KEY_WAIT:     Main_KEY_WAIT( work );     break;
  case RESEARCH_SELECT_SEQ_SCROLL_WAIT:  Main_SCROLL_WAIT( work );  break;
  case RESEARCH_SELECT_SEQ_CONFIRM:      Main_CONFIRM( work );      break;
  case RESEARCH_SELECT_SEQ_DETERMINE:    Main_DETERMINE( work );    break;
  case RESEARCH_SELECT_SEQ_FADE_IN:      Main_FADE_IN( work );      break;
  case RESEARCH_SELECT_SEQ_FADE_OUT:     Main_FADE_OUT( work );     break;
  case RESEARCH_SELECT_SEQ_PALETTE_RESET:Main_PALETTE_RESET( work );break;
  case RESEARCH_SELECT_SEQ_CLEAN_UP:     Main_CLEAN_UP( work );     break;
  case RESEARCH_SELECT_SEQ_FINISH:       return work->result; // 終了
  default:  GF_ASSERT(0);
  }

  // セルアクターシステム メイン処理
  GFL_CLACT_SYS_Main();

  CountUpSeqCount( work ); // シーケンスカウンタ更新
  SwitchSequence( work );  // シーケンス更新

  // 継続
  return RESEARCH_SELECT_RESULT_CONTINUE;
}





//==============================================================================================
// ■LAYER 4 シーケンス動作
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief 準備シーケンス ( RESEARCH_SELECT_SEQ_SETUP ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_SETUP( RESEARCH_SELECT_WORK* work )
{
  CreateFont( work );
  CreateMessages( work );
  SetupTouchArea( work );
  UpdateTopicTouchArea( work );

  // BG 準備
  SetupBG           ( work );
  SetupSubBG_WINDOW ( work );
  SetupSubBG_FONT   ( work );
  SetupMainBG_BAR   ( work );
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

  // BMP-OAM 準備
  CreateBitmapDatas( work );
  SetupBitmapData_forDefault( work );
  SetupBitmapData_forOK( work );
  SetupBitmapData_forCANCEL( work );
  SetupBmpOamSystem( work );
  CreateBmpOamActors( work );

  // パレットフェードシステム 準備
  SetupPaletteFadeSystem( work );

  // VBkankタスク登録
  RegisterVBlankTask( work );

  // 次のシーケンスをセット
  SetNextSequence( work, RESEARCH_SELECT_SEQ_SCROLL_WAIT ); 
  SetNextSequence( work, RESEARCH_SELECT_SEQ_FADE_IN ); 
  SetNextSequence( work, RESEARCH_SELECT_SEQ_STANDBY ); 

  // シーケンス終了
  FinishCurrentSequence( work );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 最初のキー入力待ちシーケンス ( RESEARCH_SELECT_SEQ_STANDBY ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_STANDBY( RESEARCH_SELECT_WORK* work )
{
  int trg;
  int touch;
  int commonTouch;

  trg   = GFL_UI_KEY_GetTrg();
  touch = GFL_UI_TP_HitTrg( work->topicTouchHitTable );
  commonTouch = GFL_UI_TP_HitTrg( RESEARCH_COMMON_GetHitTable(work->commonWork) );

  //「もどる」ボタン
  if( commonTouch == COMMON_TOUCH_AREA_RETURN_BUTTON ) {
    PMSND_PlaySE( SEQ_SE_CANCEL1 );      // キャンセル音
    SetNextSequence( work, RESEARCH_SELECT_SEQ_FADE_OUT );
    SetNextSequence( work, RESEARCH_SELECT_SEQ_PALETTE_RESET );
    SetNextSequence( work, RESEARCH_SELECT_SEQ_CLEAN_UP );
    FinishCurrentSequence( work );
    return;
  }

  // 十字キー or A or B
  if( (trg & PAD_KEY_UP) ||
      (trg & PAD_KEY_DOWN) ||
      (trg & PAD_KEY_LEFT) ||
      (trg & PAD_KEY_RIGHT) ||
      (trg & PAD_BUTTON_A) || 
      (trg & PAD_BUTTON_B) ) {
    SetNextSequence( work, RESEARCH_SELECT_SEQ_KEY_WAIT );
    FinishCurrentSequence( work );
    return;
  }

  //「調査項目」ボタン
  if( (TOPIC_TOUCH_AREA_TOPIC_0 <= touch) && (touch <= TOPIC_TOUCH_AREA_TOPIC_9) ) 
  {
    MoveTopicCursorDirect( work, touch );               // カーソル移動
    SetSelectedTopicID( work, work->topicCursorPos );   // カーソル位置の調査項目を選択
    UpdateSubDisplayStrings( work );                    // 上画面のカーソル依存文字列を更新
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                     // 決定音
    SetNextSequence( work, RESEARCH_SELECT_SEQ_CONFIRM );
    FinishCurrentSequence( work );
    return;
  } 
}

//----------------------------------------------------------------------------------------------
/**
 * @brief キー入力待ちシーケンス ( RESEARCH_SELECT_SEQ_KEY_WAIT ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_KEY_WAIT( RESEARCH_SELECT_WORK* work )
{ 
  int trg;
  int touch;
  int commonTouch;
  BOOL select = FALSE;

  trg   = GFL_UI_KEY_GetTrg();
  touch = GFL_UI_TP_HitTrg( work->topicTouchHitTable );
  commonTouch = GFL_UI_TP_HitTrg( RESEARCH_COMMON_GetHitTable(work->commonWork) );

  //「もどる」ボタン
  if( commonTouch == COMMON_TOUCH_AREA_RETURN_BUTTON ) {
    PMSND_PlaySE( SEQ_SE_CANCEL1 );      // キャンセル音
    SetNextSequence( work, RESEARCH_SELECT_SEQ_FADE_OUT );
    SetNextSequence( work, RESEARCH_SELECT_SEQ_PALETTE_RESET );
    SetNextSequence( work, RESEARCH_SELECT_SEQ_CLEAN_UP );
    FinishCurrentSequence( work );
    return;
  }

  // ↑キー
  if( trg & PAD_KEY_UP ) {
    MoveTopicCursorUp( work );
    SetNextSequence( work, RESEARCH_SELECT_SEQ_SCROLL_WAIT );
    SetNextSequence( work, RESEARCH_SELECT_SEQ_KEY_WAIT );
    FinishCurrentSequence( work );
    return;
  }

  // ↓キー
  if( trg & PAD_KEY_DOWN ) {
    MoveTopicCursorDown( work );
    SetNextSequence( work, RESEARCH_SELECT_SEQ_SCROLL_WAIT );
    SetNextSequence( work, RESEARCH_SELECT_SEQ_KEY_WAIT );
    FinishCurrentSequence( work );
    return;
  } 

  // Aボタン
  if( trg & PAD_BUTTON_A ) {
    SetSelectedTopicID( work, work->topicCursorPos ); // カーソル位置の調査項目を選択
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                   // 決定音
    SetNextSequence( work, RESEARCH_SELECT_SEQ_CONFIRM );
    FinishCurrentSequence( work );
    return;
  } 

  //「調査項目」ボタン
  if( (TOPIC_TOUCH_AREA_TOPIC_0 <= touch) && (touch <= TOPIC_TOUCH_AREA_TOPIC_9) ) 
  {
    MoveTopicCursorDirect( work, touch );             // カーソル移動
    UpdateSubDisplayStrings( work );                  // 上画面のカーソル依存文字列を更新
    SetSelectedTopicID( work, work->topicCursorPos ); // カーソル位置の調査項目を選択
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                   // 決定音
    SetNextSequence( work, RESEARCH_SELECT_SEQ_CONFIRM );
    FinishCurrentSequence( work );
    return;
  }

  // Bボタン
  if( trg & PAD_BUTTON_B ) {
    PMSND_PlaySE( SEQ_SE_CANCEL1 );      // キャンセル音
    SetNextSequence( work, RESEARCH_SELECT_SEQ_FADE_OUT );
    SetNextSequence( work, RESEARCH_SELECT_SEQ_PALETTE_RESET );
    SetNextSequence( work, RESEARCH_SELECT_SEQ_CLEAN_UP );
    FinishCurrentSequence( work );
    return;
  } 
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロール完了待ちシーケンス ( RESEARCH_SELECT_SEQ_SCROLL_WAIT ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_SCROLL_WAIT( RESEARCH_SELECT_WORK* work )
{
  // スクロール処理
  UpdateScroll( work );           // スクロールを更新
  UpdateScrollValue( work );      // スクロール実効値を更新
  UpdateTopicTouchArea( work );   // タッチ範囲を更新する
  UpdateScrollControlPos( work ); // スクロールバーのつまみ部分
  UpdateTopicSelectIcon( work );  // 調査項目選択アイコン

  // スクロール終了
  if( CheckScrollEnd(work) ) {
    FinishScroll( work );
    FinishCurrentSequence( work );
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定の確認シーケンス ( RESEARCH_SELECT_SEQ_CONFIRM ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_CONFIRM( RESEARCH_SELECT_WORK* work )
{
  int trg;
  int touch;

  trg = GFL_UI_KEY_GetTrg();
  touch = GFL_UI_TP_HitTrg( work->menuTouchHitTable );

  if( trg & PAD_KEY_UP ) {
    MoveMenuCursorUp( work );
  }
  if( trg & PAD_KEY_DOWN ) {
    MoveMenuCursorDown( work );
  } 

  // Aボタン
  if( trg & PAD_BUTTON_A ) {
    switch( work->menuCursorPos )
    {
    case MENU_ITEM_DETERMINATION_OK:
      SetNextSequence( work, RESEARCH_SELECT_SEQ_DETERMINE );
      break;
    case MENU_ITEM_DETERMINATION_CANCEL:
      PMSND_PlaySE( SEQ_SE_CANCEL1 ); // キャンセル音
      SetNextSequence( work, RESEARCH_SELECT_SEQ_PALETTE_RESET );
      SetNextSequence( work, RESEARCH_SELECT_SEQ_KEY_WAIT );
      break;
    default:
      GF_ASSERT(0);
    }
    FinishCurrentSequence( work );
    return;
  } 

  // 「けってい」ボタン
  if( touch == MENU_TOUCH_AREA_OK_BUTTON ) {
    SetNextSequence( work, RESEARCH_SELECT_SEQ_DETERMINE );
    FinishCurrentSequence( work );
    return;
  } 

  // Bボタン or 「やめる」ボタン
  if( (trg & PAD_BUTTON_B) || 
      (touch == MENU_TOUCH_AREA_CANCEL_BUTTON) ) {
    PMSND_PlaySE( SEQ_SE_CANCEL1 ); // キャンセル音
    SetNextSequence( work, RESEARCH_SELECT_SEQ_PALETTE_RESET );
    SetNextSequence( work, RESEARCH_SELECT_SEQ_KEY_WAIT );
    FinishCurrentSequence( work );
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定シーケンス ( RESEARCH_SELECT_SEQ_DETERMINE ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_DETERMINE( RESEARCH_SELECT_WORK* work )
{
  // 一定時間が経過
  if( SEQ_DETERMINE_WAIT_FRAMES <= work->seqCount ) {
    SetNextSequence( work, RESEARCH_SELECT_SEQ_FADE_OUT );
    SetNextSequence( work, RESEARCH_SELECT_SEQ_PALETTE_RESET );
    SetNextSequence( work, RESEARCH_SELECT_SEQ_CLEAN_UP );
    FinishCurrentSequence( work );
  } 
}

//----------------------------------------------------------------------------------------------
/**
 * @brief フェードイン シーケンス ( RESEARCH_SELECT_SEQ_FADE_IN ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_FADE_IN( RESEARCH_SELECT_WORK* work )
{
  // フェードが終了
  if( GFL_FADE_CheckFade() == FALSE ) {
    FinishCurrentSequence( work );
  } 
}

//----------------------------------------------------------------------------------------------
/**
 * @brief フェードアウト シーケンス ( RESEARCH_SELECT_SEQ_FADE_OUT ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_FADE_OUT( RESEARCH_SELECT_WORK* work )
{
  // フェードが終了
  if( GFL_FADE_CheckFade() == FALSE ) {
    FinishCurrentSequence( work );
  } 
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレット復帰シーケンス ( RESEARCH_SELECT_SEQ_PALETTE_RESET ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_PALETTE_RESET( RESEARCH_SELECT_WORK* work )
{
  // パレットフェード完了
  if( IsPaletteFadeEnd( work ) ) {
    FinishCurrentSequence( work );
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_SELECT_SEQ_CLEAN_UP ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_CLEAN_UP( RESEARCH_SELECT_WORK* work )
{ 
  // VBlankタスクを解除
  ReleaseVBlankTask( work );

  // パレットフェードシステム 後片付け
  CleanUpPaletteFadeSystem( work );

  // BMP-OAM 後片付け
  DeleteBitmapDatas( work );
  DeleteBmpOamActors( work );
  CleanUpBmpOamSystem( work );

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
  CleanUpMainBG_BAR   ( work );
  CleanUpMainBG_WINDOW( work );
  CleanUpSubBG_FONT   ( work );
  CleanUpSubBG_WINDOW ( work );
  CleanUpBG           ( work );

  DeleteMessages( work );
  DeleteFont( work );

  // 画面終了結果を決定
  SetResult( work, RESEARCH_SELECT_RESULT_TO_MENU );  
  SetNextSequence( work, RESEARCH_SELECT_SEQ_FINISH );
  FinishCurrentSequence( work );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスカウンタを更新する
 * 
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CountUpSeqCount( RESEARCH_SELECT_WORK* work )
{
  u32 maxCount;

  // インクリメント
  work->seqCount++;

  // 最大値を決定
  switch( work->seq )
  {
  case RESEARCH_SELECT_SEQ_SETUP:         maxCount = 0xffffffff; break;
  case RESEARCH_SELECT_SEQ_STANDBY:       maxCount = 0xffffffff; break;
  case RESEARCH_SELECT_SEQ_KEY_WAIT:      maxCount = 0xffffffff; break;
  case RESEARCH_SELECT_SEQ_SCROLL_WAIT:   maxCount = 0xffffffff; break;
  case RESEARCH_SELECT_SEQ_CONFIRM:       maxCount = 0xffffffff; break;
  case RESEARCH_SELECT_SEQ_DETERMINE:     maxCount = SEQ_DETERMINE_WAIT_FRAMES; break;
  case RESEARCH_SELECT_SEQ_FADE_IN:       maxCount = 0xffffffff; break;
  case RESEARCH_SELECT_SEQ_FADE_OUT:      maxCount = 0xffffffff; break;
  case RESEARCH_SELECT_SEQ_PALETTE_RESET: maxCount = 0xffffffff; break;
  case RESEARCH_SELECT_SEQ_CLEAN_UP:      maxCount = 0xffffffff; break;
  case RESEARCH_SELECT_SEQ_FINISH:        maxCount = 0xffffffff; break;
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
static void SetNextSequence( RESEARCH_SELECT_WORK* work, RESEARCH_SELECT_SEQ nextSeq )
{
  // シーケンスキューに追加する
  QUEUE_EnQueue( work->seqQueue, nextSeq );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: set next seq\n" );
  DebugPrint_seqQueue( work );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在のシーケンスを終了する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishCurrentSequence( RESEARCH_SELECT_WORK* work )
{
  // すでに終了済み
  GF_ASSERT( work->seqFinishFlag == FALSE );

  // 終了フラグを立てる
  work->seqFinishFlag = TRUE;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: finish current sequence\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 画面の終了結果を決定する
 *
 * @param work
 * @param result 結果
 */
//----------------------------------------------------------------------------------------------
static void SetResult( RESEARCH_SELECT_WORK* work, RESEARCH_SELECT_RESULT result )
{
  // 多重設定
  GF_ASSERT( work->result == RESEARCH_SELECT_RESULT_NONE );

  // 設定
  work->result = result;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: set result (%d)\n", result );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを変更する
 *
 * @param work
 * @param nextSeq 変更後のシーケンス
 */
//----------------------------------------------------------------------------------------------
static void SwitchSequence( RESEARCH_SELECT_WORK* work )
{
  RESEARCH_SELECT_SEQ nextSeq;

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
static void SetSequence( RESEARCH_SELECT_WORK* work, RESEARCH_SELECT_SEQ nextSeq )
{ 
  // シーケンスの終了処理
  switch( work->seq )
  {
  case RESEARCH_SELECT_SEQ_SETUP:         FinishSequence_SETUP( work );         break;
  case RESEARCH_SELECT_SEQ_STANDBY:       FinishSequence_STANDBY( work );       break;
  case RESEARCH_SELECT_SEQ_KEY_WAIT:      FinishSequence_KEY_WAIT( work );      break;
  case RESEARCH_SELECT_SEQ_SCROLL_WAIT:   FinishSequence_SCROLL_WAIT( work );   break;
  case RESEARCH_SELECT_SEQ_CONFIRM:       FinishSequence_CONFIRM( work );       break;
  case RESEARCH_SELECT_SEQ_DETERMINE:     FinishSequence_DETERMINE( work );     break;
  case RESEARCH_SELECT_SEQ_FADE_IN:       FinishSequence_FADE_IN( work );       break;
  case RESEARCH_SELECT_SEQ_FADE_OUT:      FinishSequence_FADE_OUT( work );      break;
  case RESEARCH_SELECT_SEQ_PALETTE_RESET: FinishSequence_PALETTE_RESET( work ); break;
  case RESEARCH_SELECT_SEQ_CLEAN_UP:      FinishSequence_CLEAN_UP( work );      break;
  case RESEARCH_SELECT_SEQ_FINISH:        break;
  default:  GF_ASSERT(0);
  }

  // 更新
  work->seq           = nextSeq;
  work->seqCount      = 0;
  work->seqFinishFlag = FALSE;

  // シーケンスの初期化処理
  switch( nextSeq )
  {
  case RESEARCH_SELECT_SEQ_SETUP:         InitSequence_SETUP( work );         break;
  case RESEARCH_SELECT_SEQ_STANDBY:       InitSequence_STANDBY( work );       break;
  case RESEARCH_SELECT_SEQ_KEY_WAIT:      InitSequence_KEY_WAIT( work );      break;
  case RESEARCH_SELECT_SEQ_SCROLL_WAIT:   InitSequence_SCROLL_WAIT( work );   break;
  case RESEARCH_SELECT_SEQ_CONFIRM:       InitSequence_CONFIRM( work );       break;
  case RESEARCH_SELECT_SEQ_DETERMINE:     InitSequence_DETERMINE( work );     break;
  case RESEARCH_SELECT_SEQ_FADE_IN:       InitSequence_FADE_IN( work );       break;
  case RESEARCH_SELECT_SEQ_FADE_OUT:      InitSequence_FADE_OUT( work );      break;
  case RESEARCH_SELECT_SEQ_PALETTE_RESET: InitSequence_PALETTE_RESET( work ); break;
  case RESEARCH_SELECT_SEQ_CLEAN_UP:      InitSequence_CLEAN_UP( work );      break;
  case RESEARCH_SELECT_SEQ_FINISH:        break;
  default:  GF_ASSERT(0);
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: set seq ==> " );
  switch( nextSeq )
  {
  case RESEARCH_SELECT_SEQ_SETUP:         OS_TFPrintf( PRINT_TARGET, "SETUP\n" );         break;
  case RESEARCH_SELECT_SEQ_STANDBY:       OS_TFPrintf( PRINT_TARGET, "STANDBY\n" );       break;
  case RESEARCH_SELECT_SEQ_KEY_WAIT:      OS_TFPrintf( PRINT_TARGET, "KEY_WAIT\n" );      break;
  case RESEARCH_SELECT_SEQ_SCROLL_WAIT:   OS_TFPrintf( PRINT_TARGET, "SCROLL_WAIT\n" );   break;
  case RESEARCH_SELECT_SEQ_CONFIRM:       OS_TFPrintf( PRINT_TARGET, "CONFIRM\n" );       break;
  case RESEARCH_SELECT_SEQ_DETERMINE:     OS_TFPrintf( PRINT_TARGET, "DETERMINE\n" );     break;
  case RESEARCH_SELECT_SEQ_FADE_IN:       OS_TFPrintf( PRINT_TARGET, "FADE_IN\n" );       break;
  case RESEARCH_SELECT_SEQ_FADE_OUT:      OS_TFPrintf( PRINT_TARGET, "FADE_OUT\n" );      break;
  case RESEARCH_SELECT_SEQ_PALETTE_RESET: OS_TFPrintf( PRINT_TARGET, "PALETTE_RESET\n" ); break;
  case RESEARCH_SELECT_SEQ_CLEAN_UP:      OS_TFPrintf( PRINT_TARGET, "CLEAN_UP\n" );      break;
  case RESEARCH_SELECT_SEQ_FINISH:        OS_TFPrintf( PRINT_TARGET, "FINISH\n" );        break;
  default:  GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_SELECT_SEQ_SETUP )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_SETUP( RESEARCH_SELECT_WORK* work )
{ 
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq SETUP\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_SELECT_SEQ_STANDBY )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_STANDBY( RESEARCH_SELECT_WORK* work )
{
  SetTopicButtonCursorOff( work ); // カーソルが乗っていない状態にする

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq STANDBY\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_SELECT_SEQ_KEY_WAIT )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_KEY_WAIT( RESEARCH_SELECT_WORK* work )
{
  SetTopicButtonCursorOn( work ); // カーソルが乗っている状態にする

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq KEY_WAIT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_SELECT_SEQ_SCROLL_WAIT )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_SCROLL_WAIT( RESEARCH_SELECT_WORK* work )
{
  {
    int startPos = CalcTopicButtonScreenTop( work, work->topicCursorPos ) * DOT_PER_CHARA;
    int endPos   = CalcTopicButtonScreenTop( work, work->topicCursorNextPos ) * DOT_PER_CHARA;
    int frames   = 10;
    StartScroll( work, startPos, endPos, frames  );  // スクロール開始
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq SCROLL_WAIT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_SELECT_SEQ_CONFIRM )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_CONFIRM( RESEARCH_SELECT_WORK* work )
{
  // カーソル位置を初期化
  work->menuCursorPos = MENU_ITEM_DETERMINATION_OK;

  // 確認メッセージと選択項目を表示
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CONFIRM, TRUE );
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_OK, TRUE );
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CANCEL, TRUE );

  // パレットフェードアウト開始
  StartPaletteFadeOut( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq CONFIRM\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_SELECT_SEQ_DETERMINE )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_DETERMINE( RESEARCH_SELECT_WORK* work )
{
  // 確認メッセージと選択項目を消去
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CONFIRM, FALSE );
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_OK, FALSE );
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CANCEL, FALSE );

  // 調査中だった項目のボタンを, 調査していない状態に戻す
  SetTopicButtonNotInvestigating( work, GetInvestigatingTopicID(work) );

  // 調査する項目を更新
  UpdateInvestigatingTopicID( work ); 

  // 新たに調査する項目のボタンを, 調査中の状態にする
  SetTopicButtonInvestigating( work, GetSelectedTopicID(work) );
  UpdateTopicSelectIcon( work );

  //「ちょうさを　かいしします！」を表示
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_DETERMINE, TRUE ); 

  // 調査開始SE
  PMSND_PlaySE( SEQ_SE_SYS_80 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq DETERMINE\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_SELECT_SEQ_FADE_IN )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_FADE_IN( RESEARCH_SELECT_WORK* work )
{
  // フェードイン開始
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 0);

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq FADE IN\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_SELECT_SEQ_FADE_OUT )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_FADE_OUT( RESEARCH_SELECT_WORK* work )
{
  // フェードアウト開始
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 0);

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq FADE OUT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_SELECT_SEQ_PALETTE_RESET )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_PALETTE_RESET( RESEARCH_SELECT_WORK* work )
{
  // パレットのフェードインを開始する
  StartPaletteFadeIn( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq PALETTE_RESET\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_SELECT_SEQ_CLEAN_UP )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_CLEAN_UP( RESEARCH_SELECT_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq CLEAN_UP\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_SELECT_SEQ_SETUP )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_SETUP( RESEARCH_SELECT_WORK* work )
{
  u8 nowTopicID;

  // 調査中の項目IDを取得
  nowTopicID = GetInvestigatingTopicID( work );

  // 調査中の項目がある場合
  if( nowTopicID != INVESTIGATING_QUESTION_NULL ) {
    // 調査中の項目を選択状態にする
    SetTopicButtonInvestigating( work, nowTopicID ); // 選択している状態にする
    SetSelectedTopicID( work, nowTopicID );     // 調査中の項目を選択
    UpdateTopicSelectIcon( work );              // 調査項目選択アイコンを更新
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: finish seq SETUP\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_SELECT_SEQ_STANDBY )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_STANDBY( RESEARCH_SELECT_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: finish seq STANDBY\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_SELECT_SEQ_KEY_WAIT )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_KEY_WAIT( RESEARCH_SELECT_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: finish seq KEY_WAIT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_SELECT_SEQ_SCROLL_WAIT )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_SCROLL_WAIT( RESEARCH_SELECT_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: finish seq SCROLL_WAIT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_SELECT_SEQ_CONFIRM )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_CONFIRM( RESEARCH_SELECT_WORK* work )
{
  // 確認メッセージと選択項目を消去
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CONFIRM, FALSE );
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_OK, FALSE );
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CANCEL, FALSE );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: finish seq CONFIRM\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_SELECT_SEQ_DETERMINE )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_DETERMINE( RESEARCH_SELECT_WORK* work )
{ 
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: finish seq DETERMINE\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_SELECT_SEQ_FADE_IN )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_FADE_IN( RESEARCH_SELECT_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: finish seq FADE_IN\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_SELECT_SEQ_FADE_OUT )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_FADE_OUT( RESEARCH_SELECT_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: finish seq FADE_OUT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_SELECT_SEQ_PALETTE_RESET )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_PALETTE_RESET( RESEARCH_SELECT_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: finish seq PALETTE_RESET\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_SELECT_SEQ_CLEAN_UP )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_CLEAN_UP( RESEARCH_SELECT_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: finish seq CLEAN_UP\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief VBlank 割り込み処理
 *
 * @param tcb
 * @parma wk
 */
//----------------------------------------------------------------------------------------------
static void VBlankFunc( GFL_TCB* tcb, void* wk )
{
  RESEARCH_SELECT_WORK* work = (RESEARCH_SELECT_WORK*)wk;

  GFL_BG_VBlankFunc();
  GFL_CLACT_SYS_VBlankFunc();
  PaletteFadeTrans( work->paletteFadeSystem );
}



//==============================================================================================
// ■LAYER 3 機能
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを上へ移動する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void MoveMenuCursorUp( RESEARCH_SELECT_WORK* work )
{ 
  SetMenuCursorOff( work );        // カーソルが乗っていない状態にする
  ShiftMenuCursorPos( work, -1 );  // カーソル移動
  SetMenuCursorOn( work );         // カーソルが乗っている状態にする
  PMSND_PlaySE( SEQ_SE_SELECT1 );  // カーソル移動音
}

//----------------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを下へ移動する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void MoveMenuCursorDown( RESEARCH_SELECT_WORK* work )
{
  SetMenuCursorOff( work );       // カーソルが乗っていない状態にする
  ShiftMenuCursorPos( work, 1 );  // カーソル移動
  SetMenuCursorOn( work );        // カーソルが乗っている状態にする
  PMSND_PlaySE( SEQ_SE_SELECT1 ); // カーソル移動音
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目カーソルを上に移動する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void MoveTopicCursorUp( RESEARCH_SELECT_WORK* work )
{
  SetTopicButtonCursorOff( work );   // 移動前の項目を元に戻す
  SetTopicCursorNextPos( work, -1 ); // 移動先を設定
  PMSND_PlaySE( SEQ_SE_SELECT1 );    // カーソル移動音
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目カーソルを下に移動する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void MoveTopicCursorDown( RESEARCH_SELECT_WORK* work )
{
  SetTopicButtonCursorOff( work );   // 移動前の項目を元に戻す
  SetTopicCursorNextPos( work, 1 );  // 移動先を設定
  PMSND_PlaySE( SEQ_SE_SELECT1 );    // カーソル移動音
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目カーソルを直接移動する
 *
 * @param work
 * @param topicID 移動先の調査項目ID
 */
//----------------------------------------------------------------------------------------------
static void MoveTopicCursorDirect( RESEARCH_SELECT_WORK* work, u8 topicID )
{
  SetTopicButtonCursorOff( work );          // 移動前の項目を元に戻す
  SetTopicCursorPosDirect( work, topicID ); // カーソル位置を更新
  SetTopicButtonCursorOn( work );           // 移動後の項目を選択状態にする
  PMSND_PlaySE( SEQ_SE_SELECT1 );           // カーソル移動音
}




//==============================================================================================
// □LAYER 2 個別操作
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを移動する
 *
 * @parma work
 * @param stride 移動量
 */
//----------------------------------------------------------------------------------------------
static void ShiftMenuCursorPos( RESEARCH_SELECT_WORK* work, int stride )
{
  int nowPos;
  int nextPos;

  // カーソル位置を更新
  nowPos  = work->menuCursorPos;
  nextPos = (nowPos + stride + MENU_ITEM_NUM) % MENU_ITEM_NUM;
  work->menuCursorPos = nextPos; 

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: move menu cursor ==> %d\n", nextPos );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目カーソルの移動先を設定する
 *
 * @param work
 * @param stride 移動量
 */
//----------------------------------------------------------------------------------------------
static void SetTopicCursorNextPos( RESEARCH_SELECT_WORK* work, int stride )
{
  int nowPos, nextPos;

  // 移動後のカーソル位置を算出
  nowPos  = work->topicCursorPos;
  nextPos = (nowPos + stride + TOPIC_ID_NUM) % TOPIC_ID_NUM;

  // カーソルの移動先を設定
  work->topicCursorNextPos = nextPos;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: set topic cursor next pos ==> %d\n", nextPos );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目カーソルの位置を設定する
 *
 * @param work
 * @param topicID 設定する調査項目ID
 */
//----------------------------------------------------------------------------------------------
static void SetTopicCursorPosDirect( RESEARCH_SELECT_WORK* work, int topicID )
{
  // カーソル位置を更新
  work->topicCursorPos = topicID;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: set topic cursor pos direct ==> %d\n", topicID );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目のタッチ範囲を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateTopicTouchArea( RESEARCH_SELECT_WORK* work )
{
  int idx;
  
  for( idx=TOPIC_TOUCH_AREA_TOPIC_0; idx <= TOPIC_TOUCH_AREA_TOPIC_9; idx++ )
  {
    int left, top;
    int width, height;

    left   = CalcTopicButtonDisplayLeft( work, idx );
    top    = CalcTopicButtonDisplayTop( work, idx );
    width  = TOPIC_BUTTON_WIDTH * DOT_PER_CHARA;
    height = TOPIC_BUTTON_HEIGHT * DOT_PER_CHARA;

    work->topicTouchHitTable[ idx ].rect.left   = left;
    work->topicTouchHitTable[ idx ].rect.right  = left + width;
    work->topicTouchHitTable[ idx ].rect.top    = top; 
    work->topicTouchHitTable[ idx ].rect.bottom = top + height;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: update touch area\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 選択中の調査項目IDを取得する
 *
 * @parma work
 *
 * @return 選択中の調査項目ID ( TOPIC_ID_XXXX )
 *         未選択の場合 TOPIC_ID_DUMMY
 */
//----------------------------------------------------------------------------------------------
static u8 GetSelectedTopicID( const RESEARCH_SELECT_WORK* work )
{
  return work->selectedTopicID;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目を選択する
 *
 * @param work
 * @param topicID 選択する調査項目ID
 */
//----------------------------------------------------------------------------------------------
static void SetSelectedTopicID( RESEARCH_SELECT_WORK* work, u8 topicID )
{
  work->selectedTopicID = topicID;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: selected topicID ==>%d\n", topicID );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief カーソル位置にある調査項目の選択をキャンセルする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ResetSelectedTopicID( RESEARCH_SELECT_WORK* work )
{
  work->selectedTopicID = TOPIC_ID_DUMMY;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: release topicID\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目を選択しているかどうかを判定する
 *
 * @param work
 *
 * @return 調査項目を選択している場合 TRUE
 *         そうでなければ FALSE
 */
//----------------------------------------------------------------------------------------------
static BOOL IsTopicIDSelected( const RESEARCH_SELECT_WORK* work )
{
  return (work->selectedTopicID != TOPIC_ID_DUMMY);
}

//----------------------------------------------------------------------------------------------
/**
 * @brief カーソル位置にあるメニュー項目を, カーソルが乗っている状態にする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetMenuCursorOn( RESEARCH_SELECT_WORK* work )
{
  BMPOAM_ACT_PTR BmpOamActor;

  // カーソル位置のメニュー項目に対応するBMP-OAM アクターを取得
  BmpOamActor = GetBmpOamActorOfMenuItem( work, work->menuCursorPos );

  // BMP-OAM アクターのパレットオフセットを変更
  BmpOam_ActorSetPaletteOffset( BmpOamActor, MAIN_OBJ_PALETTE_MENU_ITEM_ON );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief カーソル位置にあるメニュー項目を, メニュー項目をカーソルが乗っている状態にする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetMenuCursorOff( RESEARCH_SELECT_WORK* work )
{
  BMPOAM_ACT_PTR BmpOamActor;

  // カーソル位置のメニュー項目に対応するBMP-OAM アクターを取得
  BmpOamActor = GetBmpOamActorOfMenuItem( work, work->menuCursorPos );

  // BMP-OAM アクターのパレットオフセットを変更
  BmpOam_ActorSetPaletteOffset( BmpOamActor, MAIN_OBJ_PALETTE_MENU_ITEM_OFF );
}

//----------------------------------------------------------------------------------------------
/**
 * @biref カーソル位置にある調査項目を, カーソルが乗っている状態にする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetTopicButtonCursorOn( const RESEARCH_SELECT_WORK* work )
{
  u8 topicID;
  u8 BGFrame;
  u8 left, top, width, height;
  u8 paletteNo;

  // スクリーン更新パラメータを決定
  topicID   = work->topicCursorPos;
  left      = CalcTopicButtonScreenLeft( work, topicID );
  top       = CalcTopicButtonScreenTop( work, topicID );
  width     = TOPIC_BUTTON_WIDTH;
  height    = TOPIC_BUTTON_HEIGHT; 
  BGFrame   = MAIN_BG_WINDOW;
  paletteNo = MAIN_BG_PALETTE_WINDOW_ON;

  // スクリーン更新
  GFL_BG_ChangeScreenPalette( BGFrame, left, top, width, height, paletteNo );
  GFL_BG_LoadScreenReq( BGFrame );
}

//----------------------------------------------------------------------------------------------
/**
 * @biref カーソル位置にある調査項目を, カーソルが乗っていない状態にする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetTopicButtonCursorOff( const RESEARCH_SELECT_WORK* work )
{
  u8 topicID;
  u8 BGFrame;
  u8 left, top, width, height;
  u8 paletteNo;

  // スクリーン更新パラメータを決定
  topicID   = work->topicCursorPos;
  left      = CalcTopicButtonScreenLeft( work, topicID );
  top       = CalcTopicButtonScreenTop( work, topicID );
  width     = TOPIC_BUTTON_WIDTH;
  height    = TOPIC_BUTTON_HEIGHT; 
  BGFrame   = MAIN_BG_WINDOW;
  paletteNo = MAIN_BG_PALETTE_WINDOW_OFF;

  // スクリーン更新
  GFL_BG_ChangeScreenPalette( BGFrame, left, top, width, height, paletteNo );
  GFL_BG_LoadScreenReq( BGFrame );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 指定した調査項目のボタンを, 調査中の状態に設定する
 *
 * @param work
 * @param topicID 調査項目ID
 */
//----------------------------------------------------------------------------------------------
static void SetTopicButtonInvestigating( const RESEARCH_SELECT_WORK* work, u8 topicID )
{
  u16* screenBuffer1;
  u16* screenBuffer2;
  int xOffset, yOffset;

  // スクリーンバッファを取得
  screenBuffer1 = GFL_BG_GetScreenBufferAdrs( MAIN_BG_WINDOW );
  screenBuffer2 = GFL_BG_GetScreenBufferAdrs( MAIN_BG_FONT );

  // 該当するスクリーンデータを左に1キャラ分シフトする
  for( yOffset=0; yOffset < TOPIC_BUTTON_HEIGHT; yOffset++ )
  {
    for( xOffset=0; xOffset < TOPIC_BUTTON_WIDTH; xOffset++ )
    {
      int left, top, x, y;
      int srcPos, destPos;

      left    = TOPIC_BUTTON_X;
      top     = CalcTopicButtonScreenTop( work, topicID );
      x       = left + xOffset;
      y       = top  + yOffset;
      srcPos  = 32 * y + x;
      destPos = srcPos - 1;

      screenBuffer1[ destPos ] = screenBuffer1[ srcPos ];
      screenBuffer2[ destPos ] = screenBuffer2[ srcPos ];
    }
  }

  // 移動して空になったスクリーンをクリア
  {
    int left, top, x, y;
    int srcPos, destPos;

    left    = TOPIC_BUTTON_X;
    top     = CalcTopicButtonScreenTop( work, topicID );
    x       = left + TOPIC_BUTTON_WIDTH - 1;
    y       = top;
    srcPos  = 0;
    destPos = 32 * y + x;

    screenBuffer1[ destPos ] = screenBuffer1[ srcPos ];
    screenBuffer2[ destPos ] = screenBuffer2[ srcPos ];

    y++;
    srcPos  = 0;
    destPos = 32 * y + x;

    screenBuffer1[ destPos ] = screenBuffer1[ srcPos ];
    screenBuffer2[ destPos ] = screenBuffer2[ srcPos ];

    y++;
    srcPos  = 0;
    destPos = 32 * y + x;

    screenBuffer1[ destPos ] = screenBuffer1[ srcPos ];
    screenBuffer2[ destPos ] = screenBuffer2[ srcPos ];
  }
  GFL_BG_LoadScreenReq( MAIN_BG_WINDOW );
  GFL_BG_LoadScreenReq( MAIN_BG_FONT );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 指定した調査項目のボタンを, 調査中でない状態に戻す
 *
 * @param work
 * @param topicID 調査項目ID
 */
//----------------------------------------------------------------------------------------------
static void SetTopicButtonNotInvestigating( const RESEARCH_SELECT_WORK* work, u8 topicID )
{
  u16* screenBuffer1;
  u16* screenBuffer2;
  int xOffset, yOffset;

  // スクリーンバッファを取得
  screenBuffer1 = GFL_BG_GetScreenBufferAdrs( MAIN_BG_WINDOW );
  screenBuffer2 = GFL_BG_GetScreenBufferAdrs( MAIN_BG_FONT );

  // 該当するスクリーンデータを右に1キャラ分シフトする
  for( yOffset=TOPIC_BUTTON_HEIGHT-1; 0 <= yOffset; yOffset-- )
  {
    for( xOffset=TOPIC_BUTTON_WIDTH-1; 0 <= xOffset; xOffset-- )
    {
      int left, top, x, y;
      int srcPos, destPos;

      left    = TOPIC_BUTTON_X - 1;
      top     = CalcTopicButtonScreenTop( work, topicID );
      x       = left + xOffset;
      y       = top  + yOffset;
      srcPos  = 32 * y + x;
      destPos = srcPos + 1;

      screenBuffer1[ destPos ] = screenBuffer1[ srcPos ];
      screenBuffer2[ destPos ] = screenBuffer2[ srcPos ];
    }
  }

  // 移動して空になったスクリーンをクリア
  {
    int left, top, x, y;
    int srcPos, destPos;

    left    = TOPIC_BUTTON_X - 1;
    top     = CalcTopicButtonScreenTop( work, topicID );
    x       = left;
    y       = top;
    srcPos  = 0;
    destPos = 32 * y + x;

    screenBuffer1[ destPos ] = screenBuffer1[ srcPos ];
    screenBuffer2[ destPos ] = screenBuffer2[ srcPos ];

    y++;
    srcPos  = 0;
    destPos = 32 * y + x;

    screenBuffer1[ destPos ] = screenBuffer1[ srcPos ];
    screenBuffer2[ destPos ] = screenBuffer2[ srcPos ];

    y++;
    srcPos  = 0;
    destPos = 32 * y + x;

    screenBuffer1[ destPos ] = screenBuffer1[ srcPos ];
    screenBuffer2[ destPos ] = screenBuffer2[ srcPos ];
  }
  GFL_BG_LoadScreenReq( MAIN_BG_WINDOW );
  GFL_BG_LoadScreenReq( MAIN_BG_FONT );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 上画面のカーソル依存文字列表示を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateSubDisplayStrings( RESEARCH_SELECT_WORK* work )
{
  int nowPos;

  nowPos = work->topicCursorPos;

  // 調査項目 題名/補足
  BG_FONT_SetMessage( work->BGFont[ BG_FONT_TOPIC_TITLE ],   StringID_topicTitle[ nowPos ] );
  BG_FONT_SetMessage( work->BGFont[ BG_FONT_TOPIC_CAPTION ], StringID_topicCaption[ nowPos ] );

  // 質問
  BG_FONT_SetMessage( work->BGFont[ BG_FONT_QUESTION_1 ], StringID_question[ Question1_topic[ nowPos ] ] );
  BG_FONT_SetMessage( work->BGFont[ BG_FONT_QUESTION_2 ], StringID_question[ Question2_topic[ nowPos ] ] );
  BG_FONT_SetMessage( work->BGFont[ BG_FONT_QUESTION_3 ], StringID_question[ Question3_topic[ nowPos ] ] );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: update sub display strings \n" );
}


//----------------------------------------------------------------------------------------------
/**
 * @brief スクロール完了時の処理を行う
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishScroll( RESEARCH_SELECT_WORK* work )
{
  work->topicCursorPos = work->topicCursorNextPos; // カーソル位置を更新
  UpdateSubDisplayStrings( work );                 // 上画面のカーソル依存文字列を更新

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: finish scroll\n" );
}


//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目のスクリーン x 座標を取得する
 *
 * @param work
 * @param topicID 調査項目ID
 *
 * @return 指定した調査項目IDが該当するスクリーン範囲の左上x座標 (スクリーン単位)
 */
//----------------------------------------------------------------------------------------------
static u8 CalcTopicButtonScreenLeft( const RESEARCH_SELECT_WORK* work, u8 topicID )
{
  u8 left;

  // デフォルトの位置
  left = TOPIC_BUTTON_X;

  // 調査中の場合, 1キャラ分左にある
  if( GetInvestigatingTopicID(work) == topicID ){ left -= 1; }

  return left;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目のスクリーン y 座標を取得する
 *
 * @param work
 * @param topicID 調査項目ID
 *
 * @return 指定した調査項目IDが該当するスクリーン範囲の左上y座標 (スクリーン単位)
 */
//----------------------------------------------------------------------------------------------
static u8 CalcTopicButtonScreenTop( const RESEARCH_SELECT_WORK* work, u8 topicID )
{
  u8 top;

  // デフォルトの位置
  top = TOPIC_BUTTON_Y + TOPIC_BUTTON_HEIGHT * topicID;

  return top;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目の左上x座標を取得する (ドット単位)
 *
 * @param work
 * @param topicID 調査項目ID
 *
 * @return 指定した調査項目の左上x座標 (ドット単位)
 */
//----------------------------------------------------------------------------------------------
static int CalcTopicButtonDisplayLeft( const RESEARCH_SELECT_WORK* work, u8 topicID )
{
  int left;

  // デフォルトの位置を算出
  left = CalcTopicButtonScreenLeft( work, topicID ) * DOT_PER_CHARA;

  // BGスクロールの分を考慮する
  left -= GFL_BG_GetScreenScrollX( MAIN_BG_WINDOW );

  return left;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目の左上y座標を取得する (ドット単位)
 *
 * @param work
 * @param topicID 調査項目ID
 *
 * @return 指定した調査項目の左上y座標 (ドット単位)
 */
//----------------------------------------------------------------------------------------------
static int CalcTopicButtonDisplayTop( const RESEARCH_SELECT_WORK* work, u8 topicID )
{
  int top;

  // デフォルトの位置を算出
  top = CalcTopicButtonScreenTop( work, topicID ) * DOT_PER_CHARA;

  // BGスクロールの分を考慮する
  top -= GetScrollValue();

  return top;
}




//----------------------------------------------------------------------------------------------
/**
 * @breif タッチ領域の準備を行う
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupTouchArea( RESEARCH_SELECT_WORK* work )
{
  int idx;

  for( idx=0; idx < MENU_TOUCH_AREA_NUM; idx++ )
  {
    work->menuTouchHitTable[ idx ].rect.left   = MenuTouchAreaInitData[ idx ].left;
    work->menuTouchHitTable[ idx ].rect.right  = MenuTouchAreaInitData[ idx ].right;
    work->menuTouchHitTable[ idx ].rect.top    = MenuTouchAreaInitData[ idx ].top;
    work->menuTouchHitTable[ idx ].rect.bottom = MenuTouchAreaInitData[ idx ].bottom;
  }
  for( idx=0; idx < TOPIC_TOUCH_AREA_NUM; idx++ )
  {
    work->topicTouchHitTable[ idx ].rect.left   = TopicTouchAreaInitData[ idx ].left;
    work->topicTouchHitTable[ idx ].rect.right  = TopicTouchAreaInitData[ idx ].right;
    work->topicTouchHitTable[ idx ].rect.top    = TopicTouchAreaInitData[ idx ].top;
    work->topicTouchHitTable[ idx ].rect.bottom = TopicTouchAreaInitData[ idx ].bottom;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: create touch hit table\n" );
}




//==============================================================================================
// ■画面終了結果
//==============================================================================================


//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSeqQueue( RESEARCH_SELECT_WORK* work )
{
  // 初期化
  work->seqQueue = NULL;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq queue\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateSeqQueue( RESEARCH_SELECT_WORK* work )
{
  GF_ASSERT( work->seqQueue == NULL );

  // 作成
  work->seqQueue = QUEUE_Create( SEQ_QUEUE_SIZE, work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: create seq queue\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteSeqQueue( RESEARCH_SELECT_WORK* work )
{
  GF_ASSERT( work->seqQueue );

  // 削除
  QUEUE_Delete( work->seqQueue );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: delete seq queue\n" );
}


//----------------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitFont( RESEARCH_SELECT_WORK* work )
{
  // 初期化
  work->font = NULL;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init font\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateFont( RESEARCH_SELECT_WORK* work )
{
  GF_ASSERT( work->font == NULL );

  // 生成
  work->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, 
                                GFL_FONT_LOADTYPE_FILE, TRUE, work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: create font\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteFont( RESEARCH_SELECT_WORK* work )
{
  GF_ASSERT( work->font );

  // 削除
  GFL_FONT_Delete( work->font );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: delete font\n" );
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
static void InitMessages( RESEARCH_SELECT_WORK* work )
{
  int msgIdx;

  // 初期化
  for( msgIdx=0; msgIdx < MESSAGE_NUM; msgIdx++ )
  {
    work->message[ msgIdx ] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init messages\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateMessages( RESEARCH_SELECT_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: create messages\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteMessages( RESEARCH_SELECT_WORK* work )
{
  int msgIdx;

  for( msgIdx=0; msgIdx < MESSAGE_NUM; msgIdx++ )
  {
    GF_ASSERT( work->message[ msgIdx ] );

    // 削除
    GFL_MSG_Delete( work->message[ msgIdx ] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: delete messages\n" );
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
static void SetupBG( RESEARCH_SELECT_WORK* work )
{ 
  // BG モード
  GFL_BG_SetBGMode( &BGSysHeader2D );

  // BG コントロール
  GFL_BG_SetBGControl( SUB_BG_WINDOW,  &SubBGControl_WINDOW,  GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( SUB_BG_FONT,    &SubBGControl_FONT,    GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( MAIN_BG_BAR,    &MainBGControl_BAR,    GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( MAIN_BG_WINDOW, &MainBGControl_WINDOW, GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( MAIN_BG_FONT,   &MainBGControl_FONT,   GFL_BG_MODE_TEXT );

  // 可視設定
  GFL_BG_SetVisible( SUB_BG_BACK,    VISIBLE_ON );
  GFL_BG_SetVisible( SUB_BG_RADAR,   VISIBLE_ON );
  GFL_BG_SetVisible( SUB_BG_WINDOW,  VISIBLE_ON );
  GFL_BG_SetVisible( SUB_BG_FONT,    VISIBLE_ON );
  GFL_BG_SetVisible( MAIN_BG_BACK,   VISIBLE_ON );
  GFL_BG_SetVisible( MAIN_BG_BAR,    VISIBLE_ON );
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: setup BG\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief BG の後片付け
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpBG( RESEARCH_SELECT_WORK* work )
{
  GFL_BMPWIN_Exit();

  GFL_BG_FreeBGControl( MAIN_BG_FONT );
  GFL_BG_FreeBGControl( MAIN_BG_WINDOW );
  GFL_BG_FreeBGControl( MAIN_BG_BAR );
  GFL_BG_FreeBGControl( SUB_BG_FONT );
  GFL_BG_FreeBGControl( SUB_BG_WINDOW );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: clean up BG\n" );
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
static void SetupSubBG_WINDOW( RESEARCH_SELECT_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: setup SUB-BG-WINDOW\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 上画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpSubBG_WINDOW( RESEARCH_SELECT_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: clean up SUB-BG-WINDOW\n" );
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
static void SetupSubBG_FONT( RESEARCH_SELECT_WORK* work )
{
  // NULLキャラ確保
  GFL_BG_FillCharacter( SUB_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( SUB_BG_FONT );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: setup SUB-BG-FONT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief SUB-BG フォント面の後片付け
 * 
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpSubBG_FONT( RESEARCH_SELECT_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( SUB_BG_FONT, 1, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: clean up SUB-BG-FONT\n" );
}


//----------------------------------------------------------------------------------------------
/**
 * @brief 下画面 バーBG面 準備
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupMainBG_BAR( RESEARCH_SELECT_WORK* work )
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
      datID = NARC_research_radar_graphic_bgd_title_NSCR;
      src   = GFL_ARC_LoadDataAllocByHandle( handle, datID, work->heapID );
      NNS_G2dGetUnpackedScreenData( src, &data );
      GFL_BG_WriteScreen( MAIN_BG_BAR, data->rawData, 0, 0, 32, 24 );
      GFL_BG_LoadScreenReq( MAIN_BG_BAR );
      GFL_HEAP_FreeMemory( src );
    }

    // ハンドルクローズ
    GFL_ARC_CloseDataHandle( handle );
  } 

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: setup MAIN-BG-BAR\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 下画面 バーBG面 後片付け
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpMainBG_BAR( RESEARCH_SELECT_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: clean up MAIN-BG-BAR\n" );
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
static void SetupMainBG_WINDOW( RESEARCH_SELECT_WORK* work )
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
      datID = NARC_research_radar_graphic_bgd_searchbtn_NSCR;
      src   = GFL_ARC_LoadDataAllocByHandle( handle, datID, work->heapID );
      NNS_G2dGetUnpackedScreenData( src, &data );
      GFL_BG_WriteScreen( MAIN_BG_WINDOW, data->rawData, 0, 0, 32, 32 );
      GFL_BG_LoadScreenReq( MAIN_BG_WINDOW );
      GFL_HEAP_FreeMemory( src );
    }

    // ハンドルクローズ
    GFL_ARC_CloseDataHandle( handle );
  } 

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: setup MAIN-BG-WINDOW\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpMainBG_WINDOW( RESEARCH_SELECT_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: clean up MAIN-BG-WINDOW\n" );
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
static void SetupMainBG_FONT( RESEARCH_SELECT_WORK* work )
{ 
  // NULLキャラ確保
  GFL_BG_FillCharacter( MAIN_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( MAIN_BG_FONT );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: setup MAIN-BG-FONT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 下画面 フォントBG面 後片付け
 * 
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpMainBG_FONT( RESEARCH_SELECT_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( MAIN_BG_FONT, 1, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: clean up MAIN-BG-FONT\n" );
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
static void InitBGFonts( RESEARCH_SELECT_WORK* work )
{
  int idx;

  // 通常のBGFont
  for( idx=0; idx < BG_FONT_NUM; idx++ )
  {
    work->BGFont[ idx ] = NULL; 
  }
  // 調査項目BGFont
  for( idx=0; idx < TOPIC_ID_NUM; idx++ )
  {
    work->TopicsBGFont[ idx ] = NULL; 
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateBGFonts( RESEARCH_SELECT_WORK* work )
{
  int i;

  // 通常のBGFont
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

  // 調査項目BGFont
   for( i=0; i < TOPIC_ID_NUM; i++ )
  {
    BG_FONT_PARAM param;
    GFL_MSGDATA* msgData;
    u32 strID;

    GF_ASSERT( work->TopicsBGFont[i] == NULL ); 

    // 生成パラメータ選択
    param.BGFrame       = TopicsBGFontInitData[i].BGFrame;
    param.posX          = TopicsBGFontInitData[i].posX;
    param.posY          = TopicsBGFontInitData[i].posY;
    param.sizeX         = TopicsBGFontInitData[i].sizeX;
    param.sizeY         = TopicsBGFontInitData[i].sizeY;
    param.offsetX       = TopicsBGFontInitData[i].offsetX;
    param.offsetY       = TopicsBGFontInitData[i].offsetY;
    param.paletteNo     = TopicsBGFontInitData[i].paletteNo;
    param.colorNo_L     = TopicsBGFontInitData[i].colorNo_L;
    param.colorNo_S     = TopicsBGFontInitData[i].colorNo_S;
    param.colorNo_B     = TopicsBGFontInitData[i].colorNo_B;
    param.centeringFlag = TopicsBGFontInitData[i].softCentering; 
    msgData             = work->message[ TopicsBGFontInitData[i].messageIdx ];
    strID               = TopicsBGFontInitData[i].stringIdx;

    // 生成
    work->TopicsBGFont[i] = BG_FONT_Create( &param, work->font, msgData, work->heapID );

    // 文字列を設定
    BG_FONT_SetMessage( work->TopicsBGFont[i], strID );
  } 

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: create BGFonts\n" ); 
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteBGFonts( RESEARCH_SELECT_WORK* work )
{
  int i;
  
  // 通常のBGFont
  for( i=0; i < BG_FONT_NUM; i++ )
  {
    GF_ASSERT( work->BGFont[i] );
    BG_FONT_Delete( work->BGFont[i] );
    work->BGFont[i] = NULL;
  }
  // 調査項目BGFont
  for( i=0; i < TOPIC_ID_NUM; i++ )
  {
    GF_ASSERT( work->TopicsBGFont[i] );
    BG_FONT_Delete( work->TopicsBGFont[i] );
    work->TopicsBGFont[i] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: delete BGFonts\n" ); 
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
static void CreateClactSystem( RESEARCH_SELECT_WORK* work )
{
  // システム作成
  //GFL_CLACT_SYS_Create( &ClactSystemInitData, &VRAMBankSettings, work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: create clact system\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターシステムを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteClactSystem( RESEARCH_SELECT_WORK* work )
{ 
  // システム破棄
  //GFL_CLACT_SYS_Delete();

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: delete clact system\n" );
}



//----------------------------------------------------------------------------------------------
/**
 * @brief OBJ リソースを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitOBJResources( RESEARCH_SELECT_WORK* work )
{
  int i;

  for( i=0; i<OBJ_RESOURCE_NUM; i++ )
  {
    work->objResRegisterIdx[i] = 0; 
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init OBJ resources\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief SUB-OBJ リソースを登録する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void RegisterSubObjResources( RESEARCH_SELECT_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: register SUB-OBJ resources\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief SUB-OBJ リソースを解放する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ReleaseSubObjResources( RESEARCH_SELECT_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CELL_ANIME ] );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: release SUB-OBJ resources\n" );
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
static void RegisterMainObjResources( RESEARCH_SELECT_WORK* work )
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
                                       ONE_PALETTE_SIZE*4, 0, 2, 
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: register MAIN-OBJ resources\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief MAIN-OBJ リソースを解放する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ReleaseMainObjResources( RESEARCH_SELECT_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CELL_ANIME ] );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: release MAIN-OBJ resources\n" );
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
static void InitClactUnits( RESEARCH_SELECT_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < CLUNIT_NUM; unitIdx++ )
  {
    work->clactUnit[ unitIdx ] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init clact units\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを作成する
 *
 * @param
 */
//----------------------------------------------------------------------------------------------
static void CreateClactUnits( RESEARCH_SELECT_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: create clact units\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを破棄する
 *
 * @param
 */
//----------------------------------------------------------------------------------------------
static void DeleteClactUnits( RESEARCH_SELECT_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < CLUNIT_NUM; unitIdx++ )
  {
    GF_ASSERT( work->clactUnit[ unitIdx ] );
    GFL_CLACT_UNIT_Delete( work->clactUnit[ unitIdx ] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: delete clact units\n" );
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
static void InitClactWorks( RESEARCH_SELECT_WORK* work )
{
  int wkIdx;

  // 初期化
  for( wkIdx=0; wkIdx < CLWK_NUM; wkIdx++ )
  {
    work->clactWork[ wkIdx ] = NULL;
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init clact works\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateClactWorks( RESEARCH_SELECT_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: create clact works\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteClactWorks( RESEARCH_SELECT_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: delete clact works\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief ビットマップデータを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitBitmapDatas( RESEARCH_SELECT_WORK* work )
{
  int idx;

  // 初期化
  for( idx=0; idx < BMPOAM_ACTOR_NUM; idx++ )
  {
    work->BmpData[ idx ] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init bitmap datas\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief ビットマップデータを作成する
 *
 * @parma work
 */
//----------------------------------------------------------------------------------------------
static void CreateBitmapDatas( RESEARCH_SELECT_WORK* work )
{
  int idx;

  for( idx=0; idx < BMPOAM_ACTOR_NUM; idx++ )
  {
    const BITMAP_INIT_DATA* data;

    // 初期化データを取得
    data = &BitmapInitData[ idx ]; 

    // 作成
    work->BmpData[ idx ] = GFL_BMP_Create( data->width, data->height, data->colorMode, work->heapID );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: create bitmap datas\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief デフォルト初期化処理により, ビットマップデータの準備を行う
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupBitmapData_forDefault( RESEARCH_SELECT_WORK* work )
{
  int idx;

  for( idx=0; idx < BMPOAM_ACTOR_NUM; idx++ )
  {
    GFL_BMP_DATA* bmp;
    const BITMAP_INIT_DATA* data;
    GFL_MSGDATA* msgData;
    STRBUF* strbuf;
    PRINTSYS_LSB color;

    // 初期化データ取得
    data = &BitmapInitData[ idx ]; 

    // デフォルトセットアップ対象でない
    if( data->defaultSetupFlag == FALSE ) { continue; }

    // 背景のキャラクタをコピーする
    bmp = GFL_BMP_LoadCharacter( data->charaDataArcID, data->charaDataArcDatID, FALSE, work->heapID );
    GFL_BMP_Print( bmp, work->BmpData[ idx ], 0, 0, 0, 0, 
                   data->width * DOT_PER_CHARA, data->height * DOT_PER_CHARA, data->colorNo_B );

    // 文字列を書き込む
    msgData = work->message[ data->messageIdx ];
    color   = PRINTSYS_LSB_Make( data->colorNo_L, data->colorNo_S, data->colorNo_B );
    strbuf  = GFL_MSG_CreateString( msgData, data->stringID );

    PRINTSYS_PrintColor( work->BmpData[ idx ],
                         data->stringDrawPosX, data->stringDrawPosY,
                         strbuf, work->font, color ); 

    GFL_HEAP_FreeMemory( strbuf );
    GFL_BMP_Delete( bmp );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: setup bitmap data for Default\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief「けってい」ボタン用のビットマップデータを準備する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupBitmapData_forOK( RESEARCH_SELECT_WORK* work )
{
  int x, y;
  GFL_BMP_DATA* srcBMP; 
  GFL_MSGDATA* msgData;
  STRBUF* strbuf;
  PRINTSYS_LSB color;
  const BITMAP_INIT_DATA* data;

  // コピーするキャラクタ番号
  int charaNo[ BMPOAM_ACTOR_OK_CHARA_SIZE_Y ][ BMPOAM_ACTOR_OK_CHARA_SIZE_X ] = 
  {
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {3, 4, 4, 4, 4, 4, 4, 4, 4, 5},
    {6, 7, 7, 7, 7, 7, 7, 7, 7, 8},
  };

  // 初期化データ取得
  data = &BitmapInitData[ BMPOAM_ACTOR_OK ]; 

  // キャラクタデータを読み込む
  srcBMP = GFL_BMP_LoadCharacter( data->charaDataArcID, 
                                  data->charaDataArcDatID,
                                  FALSE, 
                                  work->heapID ); 
  // キャラデータをコピーする
  for( y=0; y < BMPOAM_ACTOR_OK_CHARA_SIZE_Y; y++)
  {
    for(x=0; x < BMPOAM_ACTOR_OK_CHARA_SIZE_X; x++)
    {
      GFL_BMP_Print( srcBMP, work->BmpData[ BMPOAM_ACTOR_OK ], 
                     (charaNo[y][x] % 3) * DOT_PER_CHARA, 
                     (charaNo[y][x] / 3) * DOT_PER_CHARA, 
                     x * DOT_PER_CHARA, y * DOT_PER_CHARA, 
                     DOT_PER_CHARA, DOT_PER_CHARA, 0 );
    }
  }
  GFL_BMP_Delete( srcBMP );

  // 文字列を書き込む
  msgData = work->message[ data->messageIdx ];
  color   = PRINTSYS_LSB_Make( data->colorNo_L, data->colorNo_S, data->colorNo_B );
  strbuf  = GFL_MSG_CreateString( msgData, data->stringID );

  PRINTSYS_PrintColor( work->BmpData[ BMPOAM_ACTOR_OK ],
                       data->stringDrawPosX, data->stringDrawPosY,
                       strbuf, work->font, color ); 

  GFL_HEAP_FreeMemory( strbuf );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: setup bitmap data for OK\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief「やめる」ボタン用のビットマップデータを準備する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupBitmapData_forCANCEL( RESEARCH_SELECT_WORK* work )
{
  int x, y;
  GFL_BMP_DATA* srcBMP; 
  GFL_MSGDATA* msgData;
  STRBUF* strbuf;
  PRINTSYS_LSB color;
  const BITMAP_INIT_DATA* data;

  // コピーするキャラクタ番号
  int charaNo[ BMPOAM_ACTOR_CANCEL_CHARA_SIZE_Y ][ BMPOAM_ACTOR_CANCEL_CHARA_SIZE_X ] = 
  {
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {3, 4, 4, 4, 4, 4, 4, 4, 4, 5},
    {6, 7, 7, 7, 7, 7, 7, 7, 7, 8},
  };

  // 初期化データ取得
  data = &BitmapInitData[ BMPOAM_ACTOR_CANCEL ]; 

  // キャラクタデータを読み込む
  srcBMP = GFL_BMP_LoadCharacter( data->charaDataArcID, 
                                  data->charaDataArcDatID,
                                  FALSE, 
                                  work->heapID ); 
  // キャラデータをコピーする
  for( y=0; y < BMPOAM_ACTOR_CANCEL_CHARA_SIZE_Y; y++)
  {
    for(x=0; x < BMPOAM_ACTOR_CANCEL_CHARA_SIZE_X; x++)
    {
      GFL_BMP_Print( srcBMP, work->BmpData[ BMPOAM_ACTOR_CANCEL ], 
                     (charaNo[y][x] % 3) * DOT_PER_CHARA, 
                     (charaNo[y][x] / 3) * DOT_PER_CHARA, 
                     x * DOT_PER_CHARA, y * DOT_PER_CHARA, 
                     DOT_PER_CHARA, DOT_PER_CHARA, 0 );
    }
  }
  GFL_BMP_Delete( srcBMP );

  // 文字列を書き込む
  msgData = work->message[ data->messageIdx ];
  color   = PRINTSYS_LSB_Make( data->colorNo_L, data->colorNo_S, data->colorNo_B );
  strbuf  = GFL_MSG_CreateString( msgData, data->stringID );

  PRINTSYS_PrintColor( work->BmpData[ BMPOAM_ACTOR_CANCEL ],
                       data->stringDrawPosX, data->stringDrawPosY,
                       strbuf, work->font, color ); 

  GFL_HEAP_FreeMemory( strbuf );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: setup bitmap data for CANCEL\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief ビットマップデータを破棄する
 *
 * @parma work
 */
//----------------------------------------------------------------------------------------------
static void DeleteBitmapDatas( RESEARCH_SELECT_WORK* work )
{
  int idx;

  // 破棄
  for( idx=0; idx < BMPOAM_ACTOR_NUM; idx++ )
  {
    GFL_BMP_Delete( work->BmpData[ idx ] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: delete bitmap datas\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM システムの準備を行う
 *
 * @parma work
 */
//----------------------------------------------------------------------------------------------
static void SetupBmpOamSystem( RESEARCH_SELECT_WORK* work )
{
  // BMP-OAM システムを作成
  work->BmpOamSystem = BmpOam_Init( work->heapID, work->clactUnit[ CLUNIT_BMPOAM ] );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: setup BMP-OAM system\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM システムの後片付けを行う
 *
 * @parma work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpBmpOamSystem( RESEARCH_SELECT_WORK* work )
{
  // BMP-OAM システムを破棄
  BmpOam_Exit( work->BmpOamSystem );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: clean up BMP-OAM system\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM アクターを作成する
 *
 * @parma work
 */
//----------------------------------------------------------------------------------------------
static void CreateBmpOamActors( RESEARCH_SELECT_WORK* work )
{
  int idx;
  BMPOAM_ACT_DATA head;

  for( idx=0; idx < BMPOAM_ACTOR_NUM; idx++ )
  {
    // 初期パラメータを作成
    head.bmp        = work->BmpData[ idx ];
    head.x          = BmpOamActorInitData[ idx ].x;
    head.y          = BmpOamActorInitData[ idx ].y;
    head.pltt_index = GetObjResourceRegisterIndex( work, BmpOamActorInitData[ idx ].paletteResID );
    head.pal_offset = BmpOamActorInitData[ idx ].paletteOffset;
    head.soft_pri   = BmpOamActorInitData[ idx ].softPriority;
    head.bg_pri     = BmpOamActorInitData[ idx ].BGPriority;
    head.setSerface = BmpOamActorInitData[ idx ].setSurface;
    head.draw_type  = BmpOamActorInitData[ idx ].drawType;

    // BMP-OAM アクターを追加
    work->BmpOamActor[ idx ] = BmpOam_ActorAdd( work->BmpOamSystem, &head );

    // 非表示に設定
    BmpOam_ActorSetDrawEnable( work->BmpOamActor[ idx ], FALSE );

    BmpOam_ActorBmpTrans( work->BmpOamActor[ idx ] );

  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: create BMP-OAM actors\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM アクターを破棄する
 *
 * @parma work
 */
//----------------------------------------------------------------------------------------------
static void DeleteBmpOamActors( RESEARCH_SELECT_WORK* work )
{
  int idx;

  for( idx=0; idx < BMPOAM_ACTOR_NUM; idx++ )
  {
    BmpOam_ActorDel( work->BmpOamActor[ idx ] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: delete BMP-OAM actors\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief VBlank期間中のタスクを登録する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void RegisterVBlankTask( RESEARCH_SELECT_WORK* work )
{
  work->VBlankTask = GFUser_VIntr_CreateTCB( VBlankFunc, work, 0 );

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: register VBlank task\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief VBlank期間中のタスクを解除する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ReleaseVBlankTask( RESEARCH_SELECT_WORK* work )
{ 
  GFL_TCB_DeleteTask( work->VBlankTask );

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: release VBlank task\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitPaletteFadeSystem( RESEARCH_SELECT_WORK* work )
{ 
  // 初期化
  work->paletteFadeSystem = NULL;

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init palette fade system\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムを準備する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupPaletteFadeSystem( RESEARCH_SELECT_WORK* work )
{
  u32 tcbWorkSize;

  // 多重生成
  GF_ASSERT( work->paletteFadeSystem == NULL );

  // フェード処理システム作成
  work->paletteFadeSystem = PaletteFadeInit( work->heapID ); 

  // パレットフェードのリクエストワークを生成
  PaletteFadeWorkAllocSet( work->paletteFadeSystem, FADE_MAIN_BG,  FULL_PALETTE_SIZE, work->heapID );
  PaletteFadeWorkAllocSet( work->paletteFadeSystem, FADE_MAIN_OBJ, FULL_PALETTE_SIZE, work->heapID );

  // リクエストワーク初期化
  PaletteWorkSet_VramCopy( work->paletteFadeSystem, FADE_MAIN_BG,  0, FULL_PALETTE_SIZE );
  PaletteWorkSet_VramCopy( work->paletteFadeSystem, FADE_MAIN_OBJ, 0, FULL_PALETTE_SIZE );

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: setup palette fade system\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムの後片付けを行う
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpPaletteFadeSystem( RESEARCH_SELECT_WORK* work )
{ 
  // パレットフェードのリクエストワークを破棄
  PaletteFadeWorkAllocFree( work->paletteFadeSystem, FADE_MAIN_BG );
  PaletteFadeWorkAllocFree( work->paletteFadeSystem, FADE_MAIN_OBJ );

  // フェード管理システムを破棄
  PaletteFadeFree( work->paletteFadeSystem );

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: clean up palette fade system\n" );
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
static u32 GetObjResourceRegisterIndex( const RESEARCH_SELECT_WORK* work, OBJ_RESOURCE_ID resID )
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
static GFL_CLUNIT* GetClactUnit( const RESEARCH_SELECT_WORK* work, CLUNIT_INDEX unitIdx )
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
static GFL_CLWK* GetClactWork( const RESEARCH_SELECT_WORK* work, CLWK_INDEX wkIdx )
{
  return work->clactWork[ wkIdx ];
}

//----------------------------------------------------------------------------------------------
/**
 * @brief メニュー項目に対応するBMP-OAMアクター
 * 
 * @param work
 * @param menuItem メニュー項目のインデックス
 *
 * @return 指定したメニュー項目に対応するBMP-OAM アクター
 */
//----------------------------------------------------------------------------------------------
static BMPOAM_ACT_PTR GetBmpOamActorOfMenuItem( const RESEARCH_SELECT_WORK* work, MENU_ITEM menuItem )
{
  BMPOAM_ACTOR_INDEX BmpOamActorIdx;

  // BMP-OAM アクターのインデックスを取得
  BmpOamActorIdx = BmpOamIndexOfMenu[ menuItem ];

  // BMP-OAM アクターを返す
  return work->BmpOamActor[ BmpOamActorIdx ];
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在調査中の調査項目IDを取得する
 *
 * @param work
 *
 * @return 現在調査中の調査項目ID
 */
//----------------------------------------------------------------------------------------------
static u8 GetInvestigatingTopicID( const RESEARCH_SELECT_WORK* work )
{
  SAVE_CONTROL_WORK* saveControlWork;
  QUESTIONNAIRE_SAVE_WORK* questionnaireSave;

  saveControlWork   = GAMEDATA_GetSaveControlWork( work->gameData );
  questionnaireSave = SaveData_GetQuestionnaire( saveControlWork );

  // セーブデータから取得
  return QuestionnaireWork_GetInvestigatingQuestion( questionnaireSave, 0 );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査する項目を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateInvestigatingTopicID( const RESEARCH_SELECT_WORK* work )
{
  SAVE_CONTROL_WORK* saveControlWork;
  QUESTIONNAIRE_SAVE_WORK* questionnaireSave;
  u8 topicID;

  // 項目を選択していない
  GF_ASSERT( IsTopicIDSelected(work) );

  topicID           = GetSelectedTopicID( work ); // 選択した調査項目ID
  saveControlWork   = GAMEDATA_GetSaveControlWork( work->gameData );
  questionnaireSave = SaveData_GetQuestionnaire( saveControlWork );
  
  // セーブデータを更新
  QuestionnaireWork_SetInvestigatingQuestion( questionnaireSave, topicID, 0 );
}


//==============================================================================================
// ■OBJ 表示
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロールバーのつまみ部分の位置を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateScrollControlPos( const RESEARCH_SELECT_WORK* work )
{
  GFL_CLWK* clactWork;
  GFL_CLACTPOS pos;
  u16 setSurface;

  // セルアクターワークを取得
  clactWork  = GetClactWork( work, CLWK_SCROLL_CONTROL ); 
  setSurface = ClactWorkInitData[ CLWK_SCROLL_CONTROL ].setSurface;

  // 表示位置を算出
  pos.x = SCROLL_CONTROL_LEFT;
  pos.y = CalcScrollControlPos();

  // 表示位置を変更
  GFL_CLACT_WK_SetPos( clactWork, &pos, setSurface );
  GFL_CLACT_WK_SetDrawEnable( clactWork, TRUE );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロールバーのつまみ部分の位置を計算する
 *
 * @return 現在のスクロール実効値から求めた, スクロールバーつまみ部分のy座標
 */
//----------------------------------------------------------------------------------------------
static int CalcScrollControlPos()
{
  int controlRange;
  int valueRange;
  int value;
  int pos;

  controlRange = SCROLL_CONTROL_BOTTOM - SCROLL_CONTROL_TOP;
  valueRange = MAX_SCROLL_VALUE - MIN_SCROLL_VALUE;
  value = GetScrollValue() - MIN_SCROLL_VALUE;
  pos = SCROLL_CONTROL_TOP + controlRange * value / valueRange;

  return pos;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロールを開始する
 *
 * @param work
 * @param startPos 開始スクロールカーソル位置
 * @param endPos   終了スクロールカーソル位置
 * @param frames   フレーム数
 */
//----------------------------------------------------------------------------------------------
static void StartScroll( RESEARCH_SELECT_WORK* work, int startPos, int endPos, int frames )
{
  // スクロールパラメータを設定
  work->scrollStartPos   = startPos;
  work->scrollEndPos     = endPos;
  work->scrollFrames     = frames;
  work->scrollFrameCount = 0;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: start scroll\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロールを更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateScroll( RESEARCH_SELECT_WORK* work )
{
  int maxFrame, nowFrame;
  int startPos, endPos, nowPos;

  // すでに終了している
  if( CheckScrollEnd(work) ) { return; }

  // フレーム数を更新
  work->scrollFrameCount++;

  // 更新後のカーソル位置を算出
  maxFrame = work->scrollFrames;
  nowFrame = work->scrollFrameCount;
  startPos = work->scrollStartPos;
  endPos   = work->scrollEndPos; 
  nowPos   = startPos + (endPos - startPos) * nowFrame / maxFrame;

  // スクロールカーソル位置を更新
  SetScrollCursor( work, nowPos );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロールが終了したかどうかを判定する
 *
 * @param work
 *
 * @return スクロールが終了していたら TRUE
 *         そうでなければ FALSE
 */
//----------------------------------------------------------------------------------------------
static BOOL CheckScrollEnd( RESEARCH_SELECT_WORK* work )
{
  int maxFrame, nowFrame;

  maxFrame = work->scrollFrames;
  nowFrame = work->scrollFrameCount;

  // 終了
  if( maxFrame <= nowFrame ) { return TRUE; }

  // 終了していない
  return FALSE;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロールカーソル位置を変更する ( オフセット指定 )
 *
 * @param work
 * @param offset 移動オフセット値
 */
//----------------------------------------------------------------------------------------------
static void ShiftScrollCursor( RESEARCH_SELECT_WORK* work, int offset )
{
  int now;
  int next;

  // 移動後のカーソル位置を決定
  now  = work->scrollCursorPos;
  next = now + offset; 
  if( next < MIN_SCROLL_CURSOR_POS ) { next = MIN_SCROLL_CURSOR_POS; } // 最小値補正
  if( MAX_SCROLL_CURSOR_POS < next ) { next = MAX_SCROLL_CURSOR_POS; } // 最大値補正

  // カーソル位置を更新
  work->scrollCursorPos = next;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: shift scroll cursor ==> %d\n", next );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロールカーソル位置を変更する ( 直値指定 )
 *
 * @param work
 * @param pos 設定するカーソル位置
 */
//----------------------------------------------------------------------------------------------
static void SetScrollCursor( RESEARCH_SELECT_WORK* work, int pos )
{
  // 引数の不正値チェック
  GF_ASSERT( MIN_SCROLL_CURSOR_POS <= pos ); // 小さすぎる
  GF_ASSERT( pos <= MAX_SCROLL_CURSOR_POS ); // 大きすぎる

  // カーソル位置を更新
  work->scrollCursorPos = pos;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: set scroll cursor ==> %d\n", pos );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロールカーソル位置を補正する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void AdjustScrollCursor( RESEARCH_SELECT_WORK* work )
{
  int minPos, maxPos, adjustPos;

  minPos    = GetMinScrollCursorMarginPos(); // カーソル最小位置
  maxPos    = GetMaxScrollCursorMarginPos(); // カーソル最大位置
  adjustPos = work->scrollCursorPos;         // 補正後の座標

  OS_Printf( "min:%d, max%d, now:%d\n", minPos, maxPos, adjustPos );

  // スクロール実効値に合わせて補正する
  if( adjustPos < minPos ) { adjustPos = minPos; } // 最小値補正
  if( maxPos < adjustPos ) { adjustPos = maxPos; } // 最大値補正

  // スクロールカーソル位置を更新
  work->scrollCursorPos = adjustPos;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: adjust scroll cursor ==> %d\n", adjustPos );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロール実効値を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateScrollValue( const RESEARCH_SELECT_WORK* work )
{
  int nowPos, minPos, maxPos;
  int value;

  nowPos = work->scrollCursorPos;         // カーソル現在位置
  minPos = GetMinScrollCursorMarginPos(); // カーソル余白範囲の最小値
  maxPos = GetMaxScrollCursorMarginPos(); // カーソル余白範囲の最大値

  // カーソルが余白範囲外にあったら更新する
  if( nowPos < minPos ) {
    value = nowPos - SCROLL_CURSOR_TOP_MARGIN;
    GFL_BG_SetScroll( MAIN_BG_WINDOW, GFL_BG_SCROLL_Y_SET, value );
    GFL_BG_SetScroll( MAIN_BG_FONT,   GFL_BG_SCROLL_Y_SET, value );
  }
  else if( maxPos < nowPos ) {
    value = nowPos + SCROLL_CURSOR_BOTTOM_MARGIN - 192;
    GFL_BG_SetScroll( MAIN_BG_WINDOW, GFL_BG_SCROLL_Y_SET, value );
    GFL_BG_SetScroll( MAIN_BG_FONT,   GFL_BG_SCROLL_Y_SET, value );
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロール実効値を取得する
 *
 * @return 現在のスクロール実効値
 */
//----------------------------------------------------------------------------------------------
static int GetScrollValue()
{
  return GFL_BG_GetScrollY( MAIN_BG_WINDOW );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロールカーソル余白範囲の最小値を取得する
 *
 * @return スクロールカーソル余白移動範囲の最小値
 */
//----------------------------------------------------------------------------------------------
static int GetMinScrollCursorMarginPos()
{
  return (GetScrollValue() + SCROLL_CURSOR_TOP_MARGIN);
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロールカーソル余白範囲の最大値を取得する
 *
 * @return スクロールカーソル余白移動範囲の最大値
 */
//----------------------------------------------------------------------------------------------
static int GetMaxScrollCursorMarginPos()
{
  return (GetScrollValue() + 192 - SCROLL_CURSOR_BOTTOM_MARGIN);
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目選択アイコンの表示状態を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateTopicSelectIcon( const RESEARCH_SELECT_WORK* work )
{
  int topicID;
  GFL_CLWK* clactWork;

  topicID   = GetInvestigatingTopicID( work );          // 調査中の項目ID
  clactWork = GetClactWork( work, CLWK_SELECT_ICON_0 ); // アイコンのセルアクターワーク

  // 調査中の項目が無い
  if( topicID == INVESTIGATING_QUESTION_NULL ) {
    GFL_CLACT_WK_SetDrawEnable( clactWork, FALSE ); // 非表示
  }
  else {
    GFL_CLACTPOS pos;
    u16 setSurface;

    // 表示状態を更新
    pos.x      = CalcTopicButtonDisplayLeft( work, topicID ) + SELECT_ICON_DRAW_OFFSET_X;
    pos.y      = CalcTopicButtonDisplayTop( work, topicID ) + SELECT_ICON_DRAW_OFFSET_Y;
    setSurface = ClactWorkInitData[ CLWK_SELECT_ICON_0 ].setSurface;
    GFL_CLACT_WK_SetPos( clactWork, &pos, setSurface );
    GFL_CLACT_WK_SetAutoAnmFlag( clactWork, TRUE );
    GFL_CLACT_WK_SetDrawEnable( clactWork, TRUE );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: update topic select icon\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットのフェードアウトを開始する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void StartPaletteFadeOut( RESEARCH_SELECT_WORK* work )
{
  // MAIN-BG
  PaletteFadeReq( work->paletteFadeSystem, 
                  PF_BIT_MAIN_BG,
                  MAIN_BG_PALETTE_FADE_OUT_TARGET_BITMASK,
                  MAIN_BG_PALETTE_FADE_OUT_WAIT,
                  MAIN_BG_PALETTE_FADE_OUT_START_STRENGTH,
                  MAIN_BG_PALETTE_FADE_OUT_END_STRENGTH,
                  MAIN_BG_PALETTE_FADE_OUT_COLOR,
                  work->VBlankTCBSystem );

  // MAIN-OBJ
  PaletteFadeReq( work->paletteFadeSystem, 
                  PF_BIT_MAIN_OBJ,
                  MAIN_OBJ_PALETTE_FADE_OUT_TARGET_BITMASK,
                  MAIN_OBJ_PALETTE_FADE_OUT_WAIT,
                  MAIN_OBJ_PALETTE_FADE_OUT_START_STRENGTH,
                  MAIN_OBJ_PALETTE_FADE_OUT_END_STRENGTH,
                  MAIN_OBJ_PALETTE_FADE_OUT_COLOR,
                  work->VBlankTCBSystem );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: start palette fade out\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットのフェードインを開始する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void StartPaletteFadeIn( RESEARCH_SELECT_WORK* work )
{
  // MAIN-BG
  PaletteFadeReq( work->paletteFadeSystem, 
                  PF_BIT_MAIN_BG,
                  MAIN_BG_PALETTE_FADE_IN_TARGET_BITMASK,
                  MAIN_BG_PALETTE_FADE_IN_WAIT,
                  MAIN_BG_PALETTE_FADE_IN_START_STRENGTH,
                  MAIN_BG_PALETTE_FADE_IN_END_STRENGTH,
                  MAIN_BG_PALETTE_FADE_IN_COLOR,
                  work->VBlankTCBSystem );

  // MAIN-OBJ
  PaletteFadeReq( work->paletteFadeSystem, 
                  PF_BIT_MAIN_OBJ,
                  MAIN_OBJ_PALETTE_FADE_IN_TARGET_BITMASK,
                  MAIN_OBJ_PALETTE_FADE_IN_WAIT,
                  MAIN_OBJ_PALETTE_FADE_IN_START_STRENGTH,
                  MAIN_OBJ_PALETTE_FADE_IN_END_STRENGTH,
                  MAIN_OBJ_PALETTE_FADE_IN_COLOR,
                  work->VBlankTCBSystem );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: start palette fade in\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットのフェードが完了したかどうかを判定する
 *
 * @param work
 *
 * @return パレットフェードが完了している場合 TRUE
 *         そうでなければ FALSE
 */
//----------------------------------------------------------------------------------------------
static BOOL IsPaletteFadeEnd( RESEARCH_SELECT_WORK* work )
{
  return (PaletteFadeCheck( work->paletteFadeSystem ) == 0);
}

//----------------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM の表示状態を設定する
 *
 * @param work
 * @param actorIdx 設定対象のBMP-OAMを指定
 * @param enable   表示するかどうか
 */
//----------------------------------------------------------------------------------------------
static void BmpOamSetDrawEnable( RESEARCH_SELECT_WORK* work, BMPOAM_ACTOR_INDEX actorIdx, BOOL enable )
{
  // インデックスエラー
  GF_ASSERT( actorIdx < BMPOAM_ACTOR_NUM );

  // 表示状態を変更
  BmpOam_ActorSetDrawEnable( work->BmpOamActor[ actorIdx ], enable );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, 
              "RESEARCH-SELECT: set draw enable BMP-OAM [%d] ==> %d\n", actorIdx, enable );
}


//==============================================================================================
// ■DEBUG:
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューの中身を表示する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DebugPrint_seqQueue( const RESEARCH_SELECT_WORK* work )
{
  int i;
  int dataNum;
  int value;

  // キュー内のデータの個数を取得
  dataNum = QUEUE_GetDataNum( work->seqQueue );

  // 全てのデータを出力
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: seq queue = " );
  for( i=0; i < dataNum; i++ )
  { 
    value = QUEUE_PeekData( work->seqQueue, i );
    
    switch( value )
    {
    case RESEARCH_SELECT_SEQ_SETUP:         OS_TFPrintf( PRINT_TARGET, "SETUP " );         break;
    case RESEARCH_SELECT_SEQ_STANDBY:       OS_TFPrintf( PRINT_TARGET, "STANDBY " );       break;
    case RESEARCH_SELECT_SEQ_KEY_WAIT:      OS_TFPrintf( PRINT_TARGET, "KEY_WAIT " );      break;
    case RESEARCH_SELECT_SEQ_SCROLL_WAIT:   OS_TFPrintf( PRINT_TARGET, "SCROLL_WAIT " );   break;
    case RESEARCH_SELECT_SEQ_CONFIRM:       OS_TFPrintf( PRINT_TARGET, "CONFIRM " );       break;
    case RESEARCH_SELECT_SEQ_DETERMINE:     OS_TFPrintf( PRINT_TARGET, "DETERMINE " );     break;
    case RESEARCH_SELECT_SEQ_FADE_IN:       OS_TFPrintf( PRINT_TARGET, "FADE_IN " );       break;
    case RESEARCH_SELECT_SEQ_FADE_OUT:      OS_TFPrintf( PRINT_TARGET, "FADE_OUT " );      break;
    case RESEARCH_SELECT_SEQ_PALETTE_RESET: OS_TFPrintf( PRINT_TARGET, "PELETTE_RESET " ); break;
    case RESEARCH_SELECT_SEQ_CLEAN_UP:      OS_TFPrintf( PRINT_TARGET, "CLEAN_UP " );      break;
    case RESEARCH_SELECT_SEQ_FINISH:        OS_TFPrintf( PRINT_TARGET, "FINISH " );        break;
    default: GF_ASSERT(0);
    }
  }
  OS_TFPrintf( PRINT_TARGET, "\n" );
} 
