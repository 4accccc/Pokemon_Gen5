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
#include "research_select.h"
#include "research_select_index.h"
#include "research_select_def.h"
#include "research_select_data.cdat"
#include "research_common.h"

#include "print/gf_font.h"           // for GFL_FONT_xxxx
#include "print/printsys.h"          // for PRINTSYS_xxxx
#include "system/gfl_use.h"          // for GFUser_xxxx
#include "system/palanm.h"           // for PaletteFadeXxxx
#include "system/bmp_oam.h"          // for BmpOam_xxxx
#include "gamesystem/game_beacon.h"  // for GAMEBEACON_xxxx

#include "system/main.h"                    // for HEAPID_xxxx
#include "arc/arc_def.h"                    // for ARCID_xxxx
#include "arc/research_radar_graphic.naix"  // for NARC_research_radar_xxxx
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
// ■調査初期画面 ワーク
//==============================================================================================
struct _RESEARCH_SELECT_WORK
{ 
  HEAPID    heapID;  // ヒープID
  GFL_FONT* font;    // フォント

  RESEARCH_SELECT_SEQ    seq;      // 処理シーケンス
  u32                    seqCount; // シーケンスカウンタ
  RESEARCH_SELECT_RESULT result;   // 画面終了結果

  // VBlank
  GFL_TCBSYS* VBlankTCBSystem; // VBlank期間中のタスク管理システム
  GFL_TCB*    VBlankTask;      // VBlankタイミング中に行うタスク

  // メニュー項目
  MENU_ITEM menuCursorPos;  // カーソル位置

  // 調査項目
  u8 selectedTopicIDs[ SELECT_TOPIC_MAX_NUM ];  // 選択した調査項目ID
  u8 selectedTopicNum;                          // 選択した調査項目の数

  u8  topicCursorPos;         // カーソル位置
  u8  topicCursorNextPos;     // 移動後のカーソル位置 
  int topicScrollFrameCount;  // スクロール フレームカウンタ

  // メッセージ
  GFL_MSGDATA* message[ MESSAGE_NUM ];

  // 文字列描画オブジェクト
  BG_FONT* BGFont[ BG_FONT_NUM ];
  BG_FONT* TopicsBGFont[ TOPIC_ID_NUM ];  // 調査項目

  // OBJ
  u32         objResRegisterIdx[ OBJ_RESOURCE_NUM ];  // リソースの登録インデックス
  GFL_CLUNIT* clactUnit[ CLUNIT_NUM ];                // セルアクターユニット
  GFL_CLWK*   clactWork[ CLWK_NUM ];                  // セルアクターワーク

  // BMP-OAM
  BMPOAM_SYS_PTR BmpOamSystem;                     // BMP-OAM システム
  BMPOAM_ACT_PTR BmpOamActor[ BMPOAM_ACTOR_NUM ];  // BMP-OAMアクター
  GFL_BMP_DATA*  BmpData[ BMPOAM_ACTOR_NUM ];      // 各アクターに対応するビットマップデータ

  // カラーパレット
  PALETTE_FADE_PTR paletteFadeSystem; // パレットフェード処理システム
};


//----------------------------------------------------------------------------------------------
// □LAYER 4 シーケンス動作
//----------------------------------------------------------------------------------------------
// シーケンス処理
static RESEARCH_SELECT_SEQ Main_SETUP      ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_SETUP
static RESEARCH_SELECT_SEQ Main_KEY_WAIT   ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_KEY_WAIT
static RESEARCH_SELECT_SEQ Main_SCROLL_WAIT( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_SCROLL_WAIT
static RESEARCH_SELECT_SEQ Main_TO_CONFIRM ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_TO_CONFIRM
static RESEARCH_SELECT_SEQ Main_CONFIRM    ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_CONFIRM
static RESEARCH_SELECT_SEQ Main_TO_KEY_WAIT( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_TO_KEY_WAIT
static RESEARCH_SELECT_SEQ Main_DETERMINE  ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_DETERMINE
static RESEARCH_SELECT_SEQ Main_CLEAN_UP   ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_CLEAN_UP

// シーケンス制御
static void SetResult( RESEARCH_SELECT_WORK* work, RESEARCH_SELECT_RESULT result ); // 画面終了結果を設定する
static void SwitchSequence( RESEARCH_SELECT_WORK* work, RESEARCH_SELECT_SEQ nextSeq ); // 処理シーケンスを変更する
static void SetSequence   ( RESEARCH_SELECT_WORK* work, RESEARCH_SELECT_SEQ nextSeq ); // 処理シーケンスを設定する

// シーケンス初期化処理
static void InitSequence_SETUP      ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_SETUP
static void InitSequence_KEY_WAIT   ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_KEY_WAIT
static void InitSequence_SCROLL_WAIT( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_SCROLL_WAIT
static void InitSequence_TO_CONFIRM ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_TO_CONFIRM
static void InitSequence_CONFIRM    ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_CONFIRM
static void InitSequence_TO_KEY_WAIT( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_TO_KEY_WAIT
static void InitSequence_DETERMINE  ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_DETERMINE
static void InitSequence_CLEAN_UP   ( RESEARCH_SELECT_WORK* work ); // RESEARCH_SELECT_SEQ_CLEAN_UP

// VBlankタスク
static void VBlankFunc( GFL_TCB* tcb, void* wk );  // VBlank中の処理

//----------------------------------------------------------------------------------------------
// □LAYER 3 機能
//---------------------------------------------------------------------------------------------- 
// メニュー項目カーソル
static void MoveMenuCursorUp  ( RESEARCH_SELECT_WORK* work ); // 上へ移動する
static void MoveMenuCursorDown( RESEARCH_SELECT_WORK* work ); // 下へ移動する

// 調査項目カーソル
static void MoveTopicCursorUp  ( RESEARCH_SELECT_WORK* work ); // 上へ移動する
static void MoveTopicCursorDown( RESEARCH_SELECT_WORK* work ); // 下へ移動する
static BOOL SelectTopic( RESEARCH_SELECT_WORK* work ); // 調査項目を選択する

//----------------------------------------------------------------------------------------------
// □LAYER 2 個別操作
//---------------------------------------------------------------------------------------------- 
// メニュー項目カーソル
static void ShiftMenuCursorPos( RESEARCH_SELECT_WORK* work, int stride ); // メニュー項目カーソル位置を変更する

// 調査項目カーソル
static void SetTopicCursorNextPos( RESEARCH_SELECT_WORK* work, int stride ); // 調査項目カーソルの移動先を設定する
static void TopicCursorScrollStart( RESEARCH_SELECT_WORK* work );  // 調査項目カーソルのスクロールを開始する

// 選択した調査項目ID
static void RegisterTopicID( RESEARCH_SELECT_WORK* work ); // 調査項目IDを登録する
static void ReleaseTopicID ( RESEARCH_SELECT_WORK* work ); // 調査項目IDを解除する
static BOOL IsTopicIDRegistered( const RESEARCH_SELECT_WORK* work, u8 topicID ); // 選択済みかを判定する

// メニュー項目の表示
static void SetMenuCursorOn( RESEARCH_SELECT_WORK* work ); // カーソルが乗っている状態にする
static void SetMenuCursorOff( RESEARCH_SELECT_WORK* work ); // カーソルが乗っていない状態にする

// 調査項目の表示
static void SetTopicCursorOn ( const RESEARCH_SELECT_WORK* work ); // カーソルが乗っている状態にする
static void SetTopicCursorOff( const RESEARCH_SELECT_WORK* work ); // カーソルが乗っていない状態にする
static void SetTopicSelected   ( const RESEARCH_SELECT_WORK* work ); // 選択されている状態にする
static void SetTopicNotSelected( const RESEARCH_SELECT_WORK* work ); // 選択されていない状態にする
static void UpdateSubDisplayStrings( RESEARCH_SELECT_WORK* work ); // 上画面のカーソル依存文字列表示を更新する
static void UpdateTopicScroll( RESEARCH_SELECT_WORK* work );       // 調査項目のスクロール状態を更新する
static void FinishTopicScroll( RESEARCH_SELECT_WORK* work );       // 調査項目のスクロール状態を完了させる
static BOOL IsTopicScrollEnd ( const RESEARCH_SELECT_WORK* work ); // 調査項目のスクロールが完了したかどうかを判定する
static u8  CalcTopicScreenPosLeft ( const RESEARCH_SELECT_WORK* work, u8 topicID ); // 調査項目の左上x座標を算出する (スクリーン単位)
static u8  CalcTopicScreenPosTop  ( const RESEARCH_SELECT_WORK* work, u8 topicID ); // 調査項目の左上y座標を算出する (スクリーン単位)
static int CalcTopicDisplayPosLeft( const RESEARCH_SELECT_WORK* work, u8 topicID ); // 調査項目の左上x座標を算出する (ドット単位)
static int CalcTopicDisplayPosTop ( const RESEARCH_SELECT_WORK* work, u8 topicID ); // 調査項目の左上y座標を算出する (ドット単位)
static int CalcScreenScrollY      ( const RESEARCH_SELECT_WORK* work );             // y軸方向のスクロール量を算出する

// OBJの表示
static void UpdateScrollControlPos( const RESEARCH_SELECT_WORK* work );  // スクロールバーのつまみ部分の位置を更新する
static void UpdateTopicSelectIcons( const RESEARCH_SELECT_WORK* work );  // 調査項目選択アイコンの表示状態を更新する

// パレットフェード
static void StartPaletteFadeOut( RESEARCH_SELECT_WORK* work ); // パレットのフェードアウトを開始する
static void StartPaletteFadeIn ( RESEARCH_SELECT_WORK* work ); // パレットのフェードインを開始する
static BOOL IsPaletteFadeEnd   ( RESEARCH_SELECT_WORK* work ); // パレットのフェードが完了したかどうかを判定する

// BMP-OAM
static void BmpOamSetDrawEnable( RESEARCH_SELECT_WORK* work, BMPOAM_ACTOR_INDEX BmpOamActorIdx, BOOL enable );  // 表示するかどうかを設定する

//----------------------------------------------------------------------------------------------
// □LAYER 1 データアクセス
//----------------------------------------------------------------------------------------------
// OBJ
static u32 GetObjResourceRegisterIndex( const RESEARCH_SELECT_WORK* work, OBJ_RESOURCE_ID resID ); // OBJリソースの登録インデックス
static GFL_CLUNIT* GetClactUnit( const RESEARCH_SELECT_WORK* work, CLUNIT_INDEX unitIdx ); // セルアクターユニット
static GFL_CLWK* GetClactWork( const RESEARCH_SELECT_WORK* work, CLWK_INDEX wkIdx ); // セルアクターワーク

// BMP-OAM
static BMPOAM_ACT_PTR GetBmpOamActorOfMenuItem( const RESEARCH_SELECT_WORK* work, MENU_ITEM menuItem ); // メニュー項目に対応するBMP-OAMアクター

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
static void InitBitmapDatas  ( RESEARCH_SELECT_WORK* work ); // ビットマップデータ 初期化
static void CreateBitmapDatas( RESEARCH_SELECT_WORK* work ); // ビットマップデータ 作成
static void SetupBitmapDatas ( RESEARCH_SELECT_WORK* work ); // ビットマップデータ 準備
static void DeleteBitmapDatas( RESEARCH_SELECT_WORK* work ); // ビットマップデータ 破棄
static void SetupBmpOamSystem  ( RESEARCH_SELECT_WORK* work ); // BMP-OAM システム 準備
static void CleanUpBmpOamSystem( RESEARCH_SELECT_WORK* work ); // BMP-OAM システム 後片付け
static void CreateBmpOamActors( RESEARCH_SELECT_WORK* work ); // BMP-OAM アクター 作成
static void DeleteBmpOamActors( RESEARCH_SELECT_WORK* work ); // BMP-OAM アクター 破棄

// VBlankタスクの登録/解除
static void RegisterVBlankTask( RESEARCH_SELECT_WORK* work );
static void ReleaseVBlankTask ( RESEARCH_SELECT_WORK* work );

// パレットフェードシステムの準備/後片付け
static void InitPaletteFadeSystem   ( RESEARCH_SELECT_WORK* work ); // パレットフェードシステム 初期化
static void SetupPaletteFadeSystem  ( RESEARCH_SELECT_WORK* work ); // パレットフェードシステム 準備
static void CleanUpPaletteFadeSystem( RESEARCH_SELECT_WORK* work ); // パレットフェードシステム 後片付け

// データの初期化/生成/破棄
static void InitFont  ( RESEARCH_SELECT_WORK* work ); // フォント 初期化
static void CreateFont( RESEARCH_SELECT_WORK* work ); // フォント 生成
static void DeleteFont( RESEARCH_SELECT_WORK* work ); // フォント 破棄
static void InitMessages  ( RESEARCH_SELECT_WORK* work ); // メッセージ 初期化
static void CreateMessages( RESEARCH_SELECT_WORK* work ); // メッセージ 生成
static void DeleteMessages( RESEARCH_SELECT_WORK* work ); // メッセージ 破棄
static void InitBGFonts  ( RESEARCH_SELECT_WORK* work ); // 文字列描画オブジェクト 初期化
static void CreateBGFonts( RESEARCH_SELECT_WORK* work ); // 文字列描画オブジェクト 生成
static void DeleteBGFonts( RESEARCH_SELECT_WORK* work ); // 文字列描画オブジェクト 破棄
static void InitSelectedTopicIDs( RESEARCH_SELECT_WORK* work ); // 選択した調査項目ID

// DEBUG:
static void DebugPrint_SelectedTopicIDs( const RESEARCH_SELECT_WORK* work );  // 登録済みの調査項目IDを出力する


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
RESEARCH_SELECT_WORK* CreateResearchSelectWork( HEAPID heapID )
{
  int i;
  RESEARCH_SELECT_WORK* work;

  // 生成
  work = GFL_HEAP_AllocMemory( heapID, sizeof(RESEARCH_SELECT_WORK) );

  // 初期化
  work->seq                   = RESEARCH_SELECT_SEQ_SETUP;
  work->seqCount              = 0;
  work->result                = RESEARCH_SELECT_RESULT_NONE;
  work->heapID                = heapID;
  work->menuCursorPos         = MENU_ITEM_DETERMINATION_OK;
  work->topicCursorPos        = 0;
  work->topicCursorNextPos    = 0;
  work->topicScrollFrameCount = 0;
  work->VBlankTCBSystem       = GFUser_VIntr_GetTCBSYS();

  for( i=0; i<OBJ_RESOURCE_NUM; i++ ){ work->objResRegisterIdx[i] = 0; }

  InitMessages( work );
  InitFont( work );
  InitBGFonts( work );
  InitClactUnits( work );
  InitClactWorks( work );
  InitSelectedTopicIDs( work );
  InitPaletteFadeSystem( work );
  InitBitmapDatas( work );

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
  RESEARCH_SELECT_SEQ nextSeq;

  // シーケンスごとの処理
  switch( work->seq )
  {
  case RESEARCH_SELECT_SEQ_SETUP:        nextSeq = Main_SETUP   ( work );     break;
  case RESEARCH_SELECT_SEQ_KEY_WAIT:     nextSeq = Main_KEY_WAIT( work );     break;
  case RESEARCH_SELECT_SEQ_SCROLL_WAIT:  nextSeq = Main_SCROLL_WAIT( work );  break;
  case RESEARCH_SELECT_SEQ_TO_CONFIRM:   nextSeq = Main_TO_CONFIRM( work );   break;
  case RESEARCH_SELECT_SEQ_CONFIRM:      nextSeq = Main_CONFIRM( work );      break;
  case RESEARCH_SELECT_SEQ_TO_KEY_WAIT:  nextSeq = Main_TO_KEY_WAIT( work );  break;
  case RESEARCH_SELECT_SEQ_DETERMINE:    nextSeq = Main_DETERMINE( work );    break;
  case RESEARCH_SELECT_SEQ_CLEAN_UP:     nextSeq = Main_CLEAN_UP( work );     break;
  case RESEARCH_SELECT_SEQ_FINISH:       return work->result;
  default:  GF_ASSERT(0);
  }

  // シーケンスカウンタ更新
  work->seqCount++;

  // シーケンス更新
  SwitchSequence( work, nextSeq );

  // セルアクターシステム メイン処理
  GFL_CLACT_SYS_Main();

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
 *
 * @return シーケンスが変化する場合 次のシーケンス番号
 *         シーケンスが継続する場合 現在のシーケンス番号
 */
//----------------------------------------------------------------------------------------------
static RESEARCH_SELECT_SEQ Main_SETUP( RESEARCH_SELECT_WORK* work )
{
  CreateFont( work );
  CreateMessages( work );

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
  SetupBitmapDatas( work );
  SetupBmpOamSystem( work );
  CreateBmpOamActors( work );

  // パレットフェードシステム 準備
  SetupPaletteFadeSystem( work );

  // VBkankタスク登録
  RegisterVBlankTask( work );

  // 画面フェードイン
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 0);

  return RESEARCH_SELECT_SEQ_KEY_WAIT;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief キー入力待ちシーケンス ( RESEARCH_SELECT_SEQ_KEY_WAIT ) の処理
 *
 * @param work
 *
 * @return シーケンスが変化する場合 次のシーケンス番号
 *         シーケンスが継続する場合 現在のシーケンス番号
 */
//----------------------------------------------------------------------------------------------
static RESEARCH_SELECT_SEQ Main_KEY_WAIT( RESEARCH_SELECT_WORK* work )
{
  RESEARCH_SELECT_SEQ nextSeq;
  int trg;

  nextSeq = work->seq;
  trg     = GFL_UI_KEY_GetTrg();


  if( trg & PAD_KEY_UP ) // 上 キー
  {
    MoveTopicCursorUp( work );
    nextSeq = RESEARCH_SELECT_SEQ_SCROLL_WAIT;  // --> スクロール待ちシーケンスへ
  }
  else if( trg & PAD_KEY_DOWN ) // 下 キー
  {
    MoveTopicCursorDown( work );
    nextSeq = RESEARCH_SELECT_SEQ_SCROLL_WAIT;  // --> スクロール待ちシーケンスへ
  } 
  else if( trg & PAD_BUTTON_A ) // A ボタン
  {
    // 最大数の調査項目を選択
    if( SelectTopic(work) && (work->selectedTopicNum == SELECT_TOPIC_MAX_NUM) )
    {
      nextSeq = RESEARCH_SELECT_SEQ_TO_CONFIRM;   // --> 調査項目確認シーケンスへ
    }
  } 
  else if( trg & PAD_BUTTON_B ) // B ボタン
  {
    nextSeq = RESEARCH_SELECT_SEQ_CLEAN_UP;   // --> 後片付けシーケンスへ
  }
  else if( trg & PAD_BUTTON_DEBUG )  // デバッグボタン
  {
    BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CONFIRM, TRUE );
  }

  return nextSeq;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロール完了待ちシーケンス ( RESEARCH_SELECT_SEQ_SCROLL_WAIT ) の処理
 *
 * @param work
 *
 * @return シーケンスが変化する場合 次のシーケンス番号
 *         シーケンスが継続する場合 現在のシーケンス番号
 */
//----------------------------------------------------------------------------------------------
static RESEARCH_SELECT_SEQ Main_SCROLL_WAIT( RESEARCH_SELECT_WORK* work )
{
  RESEARCH_SELECT_SEQ nextSeq;

  nextSeq = work->seq;

  // スクロール処理
  UpdateTopicScroll( work );       // 調査項目ボタン
  UpdateScrollControlPos( work );  // スクロールバーのつまみ部分
  UpdateTopicSelectIcons( work );  // 調査項目選択アイコン
  work->topicScrollFrameCount++;

  // スクロール終了
  if( IsTopicScrollEnd(work) )
  {
    FinishTopicScroll( work );
    nextSeq = RESEARCH_SELECT_SEQ_KEY_WAIT;
  }

  return nextSeq;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定の確認シーケンスへの準備シーケンス ( RESEARCH_SELECT_SEQ_TO_CONFIRM ) の処理
 *
 * @param work
 *
 * @return シーケンスが変化する場合 次のシーケンス番号
 *         シーケンスが継続する場合 現在のシーケンス番号
 */
//----------------------------------------------------------------------------------------------
static RESEARCH_SELECT_SEQ Main_TO_CONFIRM( RESEARCH_SELECT_WORK* work )
{
  RESEARCH_SELECT_SEQ nextSeq;

  nextSeq = work->seq;

  // パレットフェードが完了
  if( IsPaletteFadeEnd( work ) )
  {
    nextSeq = RESEARCH_SELECT_SEQ_CONFIRM;
  } 
  return nextSeq;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定の確認シーケンス ( RESEARCH_SELECT_SEQ_CONFIRM ) の処理
 *
 * @param work
 *
 * @return シーケンスが変化する場合 次のシーケンス番号
 *         シーケンスが継続する場合 現在のシーケンス番号
 */
//----------------------------------------------------------------------------------------------
static RESEARCH_SELECT_SEQ Main_CONFIRM( RESEARCH_SELECT_WORK* work )
{
  RESEARCH_SELECT_SEQ nextSeq;
  int trg;

  nextSeq = work->seq;
  trg     = GFL_UI_KEY_GetTrg();


  if( trg & PAD_KEY_UP ) // 上 キー
  {
    MoveMenuCursorUp( work );
  }
  else if( trg & PAD_KEY_DOWN ) // 下 キー
  {
    MoveMenuCursorDown( work );
  } 
  else if( trg & PAD_BUTTON_A ) // A ボタン
  {
    nextSeq = MenuItemNextSequence[ work->menuCursorPos ]; // カーソル位置に応じたシーケンス遷移
  } 
  else if( trg & PAD_BUTTON_B ) // B ボタン
  {
    nextSeq = RESEARCH_SELECT_SEQ_KEY_WAIT;    // シーケンス遷移
  }

  return nextSeq;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief キー入力待ちへ戻るシーケンス ( RESEARCH_SELECT_SEQ_TO_KEY_WAIT ) の処理
 *
 * @param work
 *
 * @return シーケンスが変化する場合 次のシーケンス番号
 *         シーケンスが継続する場合 現在のシーケンス番号
 */
//----------------------------------------------------------------------------------------------
static RESEARCH_SELECT_SEQ Main_TO_KEY_WAIT( RESEARCH_SELECT_WORK* work )
{
  RESEARCH_SELECT_SEQ nextSeq;

  nextSeq = work->seq;

  // パレットフェードが完了
  if( IsPaletteFadeEnd( work ) )
  {
    nextSeq = RESEARCH_SELECT_SEQ_KEY_WAIT;
  } 
  return nextSeq;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定シーケンス ( RESEARCH_SELECT_SEQ_DETERMINE ) の処理
 *
 * @param work
 *
 * @return シーケンスが変化する場合 次のシーケンス番号
 *         シーケンスが継続する場合 現在のシーケンス番号
 */
//----------------------------------------------------------------------------------------------
static RESEARCH_SELECT_SEQ Main_DETERMINE( RESEARCH_SELECT_WORK* work )
{
  RESEARCH_SELECT_SEQ nextSeq;

  nextSeq = work->seq;

  // 一定時間が経過
  if( SEQ_DETERMINE_WAIT_FRAMES < work->seqCount )
  {
    // 後片付けシーケンスへ
    nextSeq = RESEARCH_SELECT_SEQ_CLEAN_UP;
  } 
  return nextSeq;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_SELECT_SEQ_CLEAN_UP ) の処理
 *
 * @param work
 *
 * @return シーケンスが変化する場合 次のシーケンス番号
 *         シーケンスが継続する場合 現在のシーケンス番号
 */
//----------------------------------------------------------------------------------------------
static RESEARCH_SELECT_SEQ Main_CLEAN_UP( RESEARCH_SELECT_WORK* work )
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

  return RESEARCH_SELECT_SEQ_FINISH;
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
static void SwitchSequence( RESEARCH_SELECT_WORK* work, RESEARCH_SELECT_SEQ nextSeq )
{
  // 変化なし
  if( work->seq == nextSeq){ return; }

  // 変更
  SetSequence( work, nextSeq ); 
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
  // シーケンスごとの初期化
  switch( nextSeq )
  {
  case RESEARCH_SELECT_SEQ_SETUP:        InitSequence_SETUP( work );       break;
  case RESEARCH_SELECT_SEQ_KEY_WAIT:     InitSequence_KEY_WAIT( work );    break;
  case RESEARCH_SELECT_SEQ_SCROLL_WAIT:  InitSequence_SCROLL_WAIT( work ); break;
  case RESEARCH_SELECT_SEQ_TO_CONFIRM:   InitSequence_TO_CONFIRM( work );  break;
  case RESEARCH_SELECT_SEQ_CONFIRM:      InitSequence_CONFIRM( work );     break;
  case RESEARCH_SELECT_SEQ_TO_KEY_WAIT:  InitSequence_TO_KEY_WAIT( work ); break;
  case RESEARCH_SELECT_SEQ_DETERMINE:    InitSequence_DETERMINE( work );   break;
  case RESEARCH_SELECT_SEQ_CLEAN_UP:     InitSequence_CLEAN_UP( work );    break;
  case RESEARCH_SELECT_SEQ_FINISH:                                         break;
  default:  GF_ASSERT(0);
  }

  // 更新
  work->seq = nextSeq;
  work->seqCount = 0;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: set seq ==> " );
  switch( nextSeq )
  {
  case RESEARCH_SELECT_SEQ_SETUP:        OS_TFPrintf( PRINT_TARGET, "SETUP\n" );       break;
  case RESEARCH_SELECT_SEQ_KEY_WAIT:     OS_TFPrintf( PRINT_TARGET, "KEY_WAIT\n" );    break;
  case RESEARCH_SELECT_SEQ_SCROLL_WAIT:  OS_TFPrintf( PRINT_TARGET, "SCROLL_WAIT\n" ); break;
  case RESEARCH_SELECT_SEQ_TO_CONFIRM:   OS_TFPrintf( PRINT_TARGET, "TO_CONFIRM\n" );  break;
  case RESEARCH_SELECT_SEQ_CONFIRM:      OS_TFPrintf( PRINT_TARGET, "CONFIRM\n" );     break;
  case RESEARCH_SELECT_SEQ_TO_KEY_WAIT:  OS_TFPrintf( PRINT_TARGET, "TO_KEY_WAIT\n" ); break;
  case RESEARCH_SELECT_SEQ_DETERMINE:    OS_TFPrintf( PRINT_TARGET, "DETERMINE\n" );   break;
  case RESEARCH_SELECT_SEQ_CLEAN_UP:     OS_TFPrintf( PRINT_TARGET, "CLEAN_UP\n" );    break;
  case RESEARCH_SELECT_SEQ_FINISH:       OS_TFPrintf( PRINT_TARGET, "FINISH\n" );      break;
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
 * @brief シーケンスを初期化する ( ==> RESEARCH_SELECT_SEQ_KEY_WAIT )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_KEY_WAIT( RESEARCH_SELECT_WORK* work )
{
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
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq SCROLL_WAIT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_SELECT_SEQ_TO_CONFIRM )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_TO_CONFIRM ( RESEARCH_SELECT_WORK* work )
{
  // パレットフェードアウト開始
  StartPaletteFadeOut( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq TO_CONFIRM\n" );
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


  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq CONFIRM\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_SELECT_SEQ_TO_KEY_WAIT )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_TO_KEY_WAIT( RESEARCH_SELECT_WORK* work )
{ 
  // 確認メッセージと選択項目を消去
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CONFIRM, FALSE );
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_OK, FALSE );
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CANCEL, FALSE );

  // パレットフェードイン開始
  StartPaletteFadeIn( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq TO_KEY_WAIT" );
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

  //「ちょうさを　かいしします！」を表示
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_DETERMINE, TRUE ); 

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init seq DETERMINE\n" );
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
  SetMenuCursorOff( work );         // カーソルが乗っていない状態にする
  ShiftMenuCursorPos( work, -1 );   // カーソル移動
  SetMenuCursorOn( work );          // カーソルが乗っている状態にする
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
  SetMenuCursorOff( work );        // カーソルが乗っていない状態にする
  ShiftMenuCursorPos( work, 1 );   // カーソル移動
  SetMenuCursorOn( work );         // カーソルが乗っている状態にする
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
  SetTopicCursorOff( work );          // 移動前の項目を元に戻す
  SetTopicCursorNextPos( work, -1 );  // 移動先を設定
  TopicCursorScrollStart( work );     // スクロール開始
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
  SetTopicCursorOff( work );          // 移動前の項目を元に戻す
  SetTopicCursorNextPos( work, 1 );   // 移動先を設定
  TopicCursorScrollStart( work );     // スクロール開始
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
 * @brief 調査項目カーソルのスクロールを開始する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void TopicCursorScrollStart( RESEARCH_SELECT_WORK* work )
{
  // フレームカウンタをリセット
  work->topicScrollFrameCount = 0;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: topic cursor scroll start\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief カーソル位置にある調査項目を選択する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void RegisterTopicID( RESEARCH_SELECT_WORK* work )
{
  int topicID;
  int registerPos;

  // 選択対象と その登録場所を決定
  topicID     = work->topicCursorPos;
  registerPos = work->selectedTopicNum;

  // エラーチェック
  GF_ASSERT( registerPos < SELECT_TOPIC_MAX_NUM ); // 選択数オーバー
  GF_ASSERT( work->selectedTopicIDs[ registerPos ] == TOPIC_ID_DUMMY );  // 処理の整合性が取れていない

  // 選択した調査項目として登録
  work->selectedTopicIDs[ registerPos ] = topicID;
  work->selectedTopicNum++;

  // DEBUG:
  DebugPrint_SelectedTopicIDs( work );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief カーソル位置にある調査項目の選択をキャンセルする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ReleaseTopicID( RESEARCH_SELECT_WORK* work )
{
  int topicID;
  int shiftPos;
  int registerPos;

  // キャンセル対象を決定
  topicID = work->topicCursorPos;

  // 選択されているIDの中から, カーソル位置にある項目のIDを検索
  for( registerPos=0; registerPos < work->selectedTopicNum; registerPos++ )
  {
    // 発見
    if( work->selectedTopicIDs[ registerPos ] == topicID ){ break; }
  }

  // エラーチェック
  GF_ASSERT( registerPos < SELECT_TOPIC_MAX_NUM ); // 選択されていない

  // 解除
  for( shiftPos = registerPos; shiftPos < SELECT_TOPIC_MAX_NUM - 1; shiftPos++ )
  {
    work->selectedTopicIDs[ shiftPos ] = work->selectedTopicIDs[ shiftPos + 1 ];
  }
  work->selectedTopicIDs[ SELECT_TOPIC_MAX_NUM - 1 ] = TOPIC_ID_DUMMY;
  work->selectedTopicNum--;

  // DEBUG:
  DebugPrint_SelectedTopicIDs( work );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目が選択されているかどうかを判定する
 *
 * @param work
 * @parma topicID 調査項目ID
 *
 * @return 指定した調査項目が選択されている場合 TRUE
 *         そうでなければ FALSE
 */
//----------------------------------------------------------------------------------------------
static BOOL IsTopicIDRegistered( const RESEARCH_SELECT_WORK* work, u8 topicID )
{
  int idx;
  int selectedNum;

  // 選択済み調査項目の数
  selectedNum = work->selectedTopicNum;

  // 選択済み調査項目リストから, 指定された項目を検索
  for( idx=0; idx < selectedNum; idx++ )
  {
    // 発見
    if( work->selectedTopicIDs[ idx ] == topicID )
    {
      return TRUE;
    }
  }
  return FALSE;
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
static void SetTopicCursorOn( const RESEARCH_SELECT_WORK* work )
{
  u8 topicID;
  u8 BGFrame;
  u8 left, top, width, height;
  u8 paletteNo;

  // スクリーン更新パラメータを決定
  topicID   = work->topicCursorPos;
  left      = CalcTopicScreenPosLeft( work, topicID );
  top       = CalcTopicScreenPosTop( work, topicID );
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
static void SetTopicCursorOff( const RESEARCH_SELECT_WORK* work )
{
  u8 topicID;
  u8 BGFrame;
  u8 left, top, width, height;
  u8 paletteNo;

  // スクリーン更新パラメータを決定
  topicID   = work->topicCursorPos;
  left      = CalcTopicScreenPosLeft( work, topicID );
  top       = CalcTopicScreenPosTop( work, topicID );
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
 * @brief カーソル位置にある調査項目を 選択された状態にする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetTopicSelected( const RESEARCH_SELECT_WORK* work )
{
  u8 topicID;
  u16* screenBuffer1;
  u16* screenBuffer2;
  int xOffset, yOffset;

  topicID = work->topicCursorPos;

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

      left    = CalcTopicScreenPosLeft( work, topicID );
      top     = CalcTopicScreenPosTop( work, topicID );
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

    left    = CalcTopicScreenPosLeft( work, topicID );
    top     = CalcTopicScreenPosTop( work, topicID );
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
 * @brief カーソル位置にある調査項目を 選択されていない状態にする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetTopicNotSelected( const RESEARCH_SELECT_WORK* work )
{
  u8 topicID;
  u16* screenBuffer1;
  u16* screenBuffer2;
  int xOffset, yOffset;

  topicID = work->topicCursorPos;

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

      left    = CalcTopicScreenPosLeft( work, topicID );
      top     = CalcTopicScreenPosTop( work, topicID );
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

    left    = CalcTopicScreenPosLeft( work, topicID );
    top     = CalcTopicScreenPosTop( work, topicID );
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
  BG_FONT_SetString( work->BGFont[ BG_FONT_TOPIC_TITLE ],   StringID_topicTitle[ nowPos ] );
  BG_FONT_SetString( work->BGFont[ BG_FONT_TOPIC_CAPTION ], StringID_topicCaption[ nowPos ] );

  // 質問
  BG_FONT_SetString( work->BGFont[ BG_FONT_QUESTION_1 ], StringID_question[ Question1_topic[ nowPos ] ] );
  BG_FONT_SetString( work->BGFont[ BG_FONT_QUESTION_2 ], StringID_question[ Question2_topic[ nowPos ] ] );
  BG_FONT_SetString( work->BGFont[ BG_FONT_QUESTION_3 ], StringID_question[ Question3_topic[ nowPos ] ] );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: update sub display strings \n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目のスクロール状態を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateTopicScroll( RESEARCH_SELECT_WORK* work )
{
  int scrollSize;

  // スクロール量を算出
  scrollSize = CalcScreenScrollY( work );

  // スクロールリクエスト
  GFL_BG_SetScrollReq( MAIN_BG_WINDOW, GFL_BG_SCROLL_Y_SET, scrollSize );
  GFL_BG_SetScrollReq( MAIN_BG_FONT,   GFL_BG_SCROLL_Y_SET, scrollSize );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目スクロール完了時の処理を行う
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishTopicScroll( RESEARCH_SELECT_WORK* work )
{
  work->topicCursorPos = work->topicCursorNextPos; // カーソル位置を更新
  UpdateSubDisplayStrings( work );                 // 上画面のカーソル依存文字列を更新
  SetTopicCursorOn( work );                        // カーソルが乗っている状態にする

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: finish topic scroll\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief スクロールが終了したかどうかを判定する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static BOOL IsTopicScrollEnd ( const RESEARCH_SELECT_WORK* work )
{
  return ( SCROLL_FRAME < work->topicScrollFrameCount);
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
static u8 CalcTopicScreenPosLeft( const RESEARCH_SELECT_WORK* work, u8 topicID )
{
  u8 left;

  // デフォルトの位置
  left = TOPIC_BUTTON_X;

  // 選択されている場合, 1キャラ分左にある
  if( IsTopicIDRegistered( work, topicID ) ){ left -= 1; }

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
static u8 CalcTopicScreenPosTop( const RESEARCH_SELECT_WORK* work, u8 topicID )
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
static int CalcTopicDisplayPosLeft( const RESEARCH_SELECT_WORK* work, u8 topicID )
{
  int left;

  // デフォルトの位置を算出
  left = CalcTopicScreenPosLeft( work, topicID ) * DOT_PER_CHARA;

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
static int CalcTopicDisplayPosTop( const RESEARCH_SELECT_WORK* work, u8 topicID )
{
  int top;

  // デフォルトの位置を算出
  top = CalcTopicScreenPosTop( work, topicID ) * DOT_PER_CHARA;

  // BGスクロールの分を考慮する
  top -= CalcScreenScrollY( work );

  return top;
}


//----------------------------------------------------------------------------------------------
/**
 * @brief y軸方向のスクロール量を算出する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static int CalcScreenScrollY( const RESEARCH_SELECT_WORK* work )
{
  int scrollSize;
  int frame;
  int start, end, now;
  int screenTop, screenBottom;
  int min, max;


  // スクロール終了済み
  if( IsTopicScrollEnd(work) )
  {
    frame = SCROLL_FRAME;
  }
  else
  {
    frame = work->topicScrollFrameCount;
  }

  start        = (TOPIC_BUTTON_Y + TOPIC_BUTTON_HEIGHT * work->topicCursorPos) * DOT_PER_CHARA;
  end          = (TOPIC_BUTTON_Y + TOPIC_BUTTON_HEIGHT * work->topicCursorNextPos) * DOT_PER_CHARA;
  now          = start + (end - start) * frame / SCROLL_FRAME ;
  screenTop    = GFL_BG_GetScreenScrollY( MAIN_BG_WINDOW );
  screenBottom = screenTop + 192;
  min          = screenTop + SCROLL_TOP_MARGIN;
  max          = screenBottom - SCROLL_BOTTOM_MARGIN;
  scrollSize   = screenTop;

  if( now < min )
  {
    scrollSize = now - SCROLL_TOP_MARGIN;
  }
  else if( max < now )
  {
    scrollSize = now - 192 + SCROLL_BOTTOM_MARGIN; 
  } 
  return scrollSize;
}


















//==============================================================================================
// ■調査項目
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief カーソル位置にある調査項目を選択する。
 *        すでに選択済みなら登録を解除する。
 *        そうでなければ, 登録する。
 *
 * @param work
 *
 * @return 新しい項目を選択した場合 TRUE
 *         そうでなければ FALSE
 */
//----------------------------------------------------------------------------------------------
static BOOL SelectTopic( RESEARCH_SELECT_WORK* work )
{
  BOOL select = FALSE;  // 新しい項目を選択したかどうか


  if( IsTopicIDRegistered( work, work->topicCursorPos ) ) // 登録済み
  {
    SetTopicNotSelected( work );  // 選択していない状態に戻す
    ReleaseTopicID( work );       // 登録していた項目IDを解除する
  }
  else if( work->selectedTopicNum == SELECT_TOPIC_MAX_NUM ) // すでに最大数を選択済み
  {
  }
  else // 新しい項目を選択
  {
    SetTopicSelected( work );  // 選択している状態にする
    RegisterTopicID( work );   // 項目IDを登録する
    select = TRUE;
  }

  // 調査項目選択アイコンを更新
  UpdateTopicSelectIcons( work );

  return select;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 選択済み調査項目IDリストを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSelectedTopicIDs( RESEARCH_SELECT_WORK* work )
{
  int i;

  // 選択済み調査項目IDを初期化
  for( i=0; i < SELECT_TOPIC_MAX_NUM; i++ )
  { 
    work->selectedTopicIDs[i] = TOPIC_ID_DUMMY;
  }

  // 選択した調査項目の数を初期化
  work->selectedTopicNum = 0;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: init select topic IDs\n" );
}





//==============================================================================================
// ■画面終了結果
//==============================================================================================



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
    param.BGFrame   = BGFontInitData[i].BGFrame;
    param.posX      = BGFontInitData[i].posX;
    param.posY      = BGFontInitData[i].posY;
    param.sizeX     = BGFontInitData[i].sizeX;
    param.sizeY     = BGFontInitData[i].sizeY;
    param.offsetX   = BGFontInitData[i].offsetX;
    param.offsetY   = BGFontInitData[i].offsetY;
    param.paletteNo = BGFontInitData[i].paletteNo;
    param.colorNo_L = BGFontInitData[i].colorNo_L;
    param.colorNo_S = BGFontInitData[i].colorNo_S;
    param.colorNo_B = BGFontInitData[i].colorNo_B;
    msgData         = work->message[ BGFontInitData[i].messageIdx ];
    strID           = BGFontInitData[i].stringIdx;

    // 生成
    work->BGFont[i] = BG_FONT_Create( &param, work->font, msgData, work->heapID );

    // 文字列を設定
    BG_FONT_SetString( work->BGFont[i], strID );
  } 

  // 調査項目BGFont
   for( i=0; i < TOPIC_ID_NUM; i++ )
  {
    BG_FONT_PARAM param;
    GFL_MSGDATA* msgData;
    u32 strID;

    GF_ASSERT( work->TopicsBGFont[i] == NULL ); 

    // 生成パラメータ選択
    param.BGFrame   = TopicsBGFontInitData[i].BGFrame;
    param.posX      = TopicsBGFontInitData[i].posX;
    param.posY      = TopicsBGFontInitData[i].posY;
    param.sizeX     = TopicsBGFontInitData[i].sizeX;
    param.sizeY     = TopicsBGFontInitData[i].sizeY;
    param.offsetX   = TopicsBGFontInitData[i].offsetX;
    param.offsetY   = TopicsBGFontInitData[i].offsetY;
    param.paletteNo = TopicsBGFontInitData[i].paletteNo;
    param.colorNo_L = TopicsBGFontInitData[i].colorNo_L;
    param.colorNo_S = TopicsBGFontInitData[i].colorNo_S;
    param.colorNo_B = TopicsBGFontInitData[i].colorNo_B;
    msgData         = work->message[ TopicsBGFontInitData[i].messageIdx ];
    strID           = TopicsBGFontInitData[i].stringIdx;

    // 生成
    work->TopicsBGFont[i] = BG_FONT_Create( &param, work->font, msgData, work->heapID );

    // 文字列を設定
    BG_FONT_SetString( work->TopicsBGFont[i], strID );
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
  GFL_CLACT_SYS_Create( &ClactSystemInitData, &VRAMBankSettings, work->heapID );

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
  GFL_CLACT_SYS_Delete();

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: delete clact system\n" );
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
 * @brief ビットマップデータの準備を行う
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupBitmapDatas( RESEARCH_SELECT_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: setup bitmap datas\n" );
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
  int frame;
  int start, end;

  // セルアクターワークを取得
  clactWork  = GetClactWork( work, CLWK_SCROLL_CONTROL ); 
  setSurface = ClactWorkInitData[ CLWK_SCROLL_CONTROL ].setSurface;

  // スクロール開始位置と終了位置を算出
  frame = work->topicScrollFrameCount;
  start = SCROLL_CONTROL_TOP 
        + (SCROLL_CONTROL_BOTTOM - SCROLL_CONTROL_TOP) * work->topicCursorPos / (SCROLL_CONTROL_STEP_NUM - 1); 
  end   = SCROLL_CONTROL_TOP 
        + (SCROLL_CONTROL_BOTTOM - SCROLL_CONTROL_TOP) * work->topicCursorNextPos / (SCROLL_CONTROL_STEP_NUM - 1);

  // 表示位置を算出
  pos.x = SCROLL_CONTROL_LEFT;
  pos.y = start + (end - start) * frame / SCROLL_FRAME;

  // 表示位置を変更
  GFL_CLACT_WK_SetPos( clactWork, &pos, setSurface );
  GFL_CLACT_WK_SetDrawEnable( clactWork, TRUE );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目選択アイコンの表示状態を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateTopicSelectIcons( const RESEARCH_SELECT_WORK* work )
{
  int idx;
  int selectedTopicNum = work->selectedTopicNum;
  CLWK_INDEX iconClactWorkIdx[ SELECT_TOPIC_MAX_NUM ] = 
  {
    CLWK_SELECT_ICON_0,
    CLWK_SELECT_ICON_1,
    CLWK_SELECT_ICON_2,
  };

  // 全アイコンを消去
  for( idx=0; idx < SELECT_TOPIC_MAX_NUM; idx++ )
  {
    GFL_CLWK* clactWork;
    CLWK_INDEX wkIdx;

    wkIdx     = iconClactWorkIdx[ idx ];
    clactWork = GetClactWork( work, wkIdx );
    GFL_CLACT_WK_SetDrawEnable( clactWork, FALSE );
  }

  // 選択項目の数だけ表示
  for( idx=0; idx < selectedTopicNum; idx++ )
  { 
    GFL_CLACTPOS pos;
    GFL_CLWK* clactWork;
    CLWK_INDEX wkIdx;
    u16 setSurface;

    wkIdx      = iconClactWorkIdx[ idx ];
    clactWork  = GetClactWork( work, wkIdx );
    pos.x      = CalcTopicDisplayPosLeft( work, work->selectedTopicIDs[ idx ] ) + SELECT_ICON_DRAW_OFFSET_X;
    pos.y      = CalcTopicDisplayPosTop( work, work->selectedTopicIDs[ idx ] ) + SELECT_ICON_DRAW_OFFSET_Y;
    setSurface = ClactWorkInitData[ wkIdx ].setSurface;
    GFL_CLACT_WK_SetPos( clactWork, &pos, setSurface );
    GFL_CLACT_WK_SetAutoAnmFlag( clactWork, TRUE );
    GFL_CLACT_WK_SetDrawEnable( clactWork, TRUE );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: update topic select icons\n" );
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
  return PaletteFadeCheck( work->paletteFadeSystem );
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
 * @brief 選択済みの調査項目IDを表示する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DebugPrint_SelectedTopicIDs( const RESEARCH_SELECT_WORK* work )
{
  int i;

  OS_TFPrintf( PRINT_TARGET, "RESEARCH-SELECT: selected topic IDs =" );
  for( i=0; i < work->selectedTopicNum; i++ )
  {
    OS_TFPrintf( PRINT_TARGET, " %d", work->selectedTopicIDs[i] );
  }
  OS_TFPrintf( PRINT_TARGET, "\n" );
}
