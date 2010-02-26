////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 調査報告確認画面
 * @file   research_check.c
 * @author obata
 * @date   2010.02.20
 */
////////////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "queue.h"
#include "bg_font.h"
#include "circle_graph.h"
#include "research_data.h"
#include "research_check.h"
#include "research_check_index.h"
#include "research_check_def.h"
#include "research_check_data.cdat"
#include "research_common.h"

#include "system/gfl_use.h"              // for GFUser_xxxx
#include "system/palanm.h"               // for PaletteFadeXxxx
#include "system/bmp_oam.h"              // for BmpOam_xxxx
#include "gamesystem/gamesystem.h"       // for GAMESYS_WORK
#include "gamesystem/game_beacon.h"      // for GAMEBEACON_xxxx
#include "savedata/save_control.h"       // for SaveControl_xxxx
#include "savedata/questionnaire_save.h" // for QuestionnareWork_xxxx
#include "sound/pm_sndsys.h"             // for PMSND_xxxx
#include "sound/wb_sound_data.sadl"      // for SEQ_SE_XXXX
#include "print/gf_font.h"               // for GFL_FONT_xxxx
#include "print/printsys.h"              // for PRINTSYS_xxxx
#include "print/wordset.h"               // for WORDSET_xxxx

#include "arc/arc_def.h"                    // for ARCID_xxxx
#include "arc/research_radar_graphic.naix"  // for NARC_research_radar_xxxx
#include "arc/font/font.naix"               // for NARC_font_xxxx
#include "arc/message.naix"                 // for NARC_message_xxxx
#include "msg/msg_research_radar.h"         // for str_xxxx
#include "msg/msg_questionnaire.h"          // for str_xxxx
#include "obj_NANR_LBLDEFS.h"               // for NANR_obj_xxxx
#include "topic_id.h"                       // for TOPIC_ID_xxxx
#include "question_id.h"                    // for QUESTION_ID_xxxx
#include "answer_id.h"                      // for ANSWER_ID_xxxx

#include "answer_id_question.cdat"         // for AnswerID_question[17][]
#include "answer_num_question.cdat"        // for AnswerNum_question[]
#include "question_id_topic.cdat"          // for QuestionX_topic[]
#include "string_id_answer.cdat"           // for StringID_answer[]
#include "string_id_question.cdat"         // for StringID_question[]
#include "string_id_topic_title.cdat"      // for StringID_topicTitle[]
#include "string_id_topic_caption.cdat"    // for StringID_topicCaption[]
#include "caption_string_id_question.cdat" // for CaptionStringID_question[]
#include "color_r_answer.cdat"             // for ColorR_answer[]
#include "color_g_answer.cdat"             // for ColorG_answer[]
#include "color_b_answer.cdat"             // for ColorB_answer[]



//==============================================================================================
// ■調査報告確認画面 ワーク
//==============================================================================================
struct _RESEARCH_CHECK_WORK
{ 
  RESEARCH_COMMON_WORK* commonWork; // 全画面共通ワーク
  HEAPID                heapID;
  GAMESYS_WORK*         gameSystem;
  GAMEDATA*             gameData;

  GFL_FONT*    font;                   // フォント
  GFL_MSGDATA* message[ MESSAGE_NUM ]; // メッセージ
  WORDSET*     wordset;                // ワードセット

  QUEUE*                seqQueue;      // シーケンスキュー
  RESEARCH_CHECK_SEQ    seq;           // 現在の処理シーケンス
  BOOL                  seqFinishFlag; // 現在のシーケンスが終了したかどうか
  u32                   seqCount;      // シーケンスカウンタ
  RESEARCH_CHECK_RESULT result;        // 画面終了結果

  MENU_ITEM      cursorPos;     // カーソル位置
  RESEARCH_DATA  researchData;  // 調査データ
  BOOL           analyzeFlag;   // 解析が済んでいるかどうか
  u8             questionIdx;   // 表示中の質問インデックス
  u8             answerIdx;     // 選択中の回答インデックス
  DATA_DISP_TYPE dispType;      // 今日 or 合計 のどちらのデータを表示するのか

  // 円グラフ
  CIRCLE_GRAPH* mainGraph[ DATA_DISP_TYPE_NUM ]; // 通常時に表示するグラフ
  CIRCLE_GRAPH* subGraph [ DATA_DISP_TYPE_NUM ]; // 更新時に表示するグラフ

  // タッチ領域
  GFL_UI_TP_HITTBL touchHitTable[ TOUCH_AREA_NUM ];

  // 文字列描画オブジェクト
  BG_FONT* BGFont[ BG_FONT_NUM ];

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

  // VBlank
  GFL_TCBSYS* VBlankTCBSystem; // VBlank期間中のタスク管理システム
  GFL_TCB*    VBlankTask;      // VBlankタイミング中に行うタスク
};


//----------------------------------------------------------------------------------------------
// □LAYER 4 シーケンス動作
//----------------------------------------------------------------------------------------------
// シーケンス処理
static void Main_SETUP   ( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_SETUP
static void Main_KEY_WAIT( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_KEY_WAIT
static void Main_ANALYZE ( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_ANALYZE
static void Main_FLASH   ( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_FLASH
static void Main_UPDATE  ( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_UPDATE
static void Main_FADE_OUT( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_FADE_OUT
static void Main_CLEAN_UP( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_CLEAN_UP

// シーケンス制御
static void CountUpSeqCount( RESEARCH_CHECK_WORK* work ); // シーケンスカウンタを更新する
static void SetNextSequence( RESEARCH_CHECK_WORK* work, RESEARCH_CHECK_SEQ nextSeq ); // 次のシーケンスを登録する
static void FinishCurrentSequence( RESEARCH_CHECK_WORK* work ); // 現在のシーケンスを終了する
static void SwitchSequence( RESEARCH_CHECK_WORK* work ); // 処理シーケンスを変更する
static void SetSequence( RESEARCH_CHECK_WORK* work, RESEARCH_CHECK_SEQ nextSeq ); // 処理シーケンスを設定する
static void SetResult( RESEARCH_CHECK_WORK* work, RESEARCH_CHECK_RESULT result ); // 画面終了結果を設定する

// シーケンス初期化処理
static void InitSequence_SETUP   ( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_SETUP
static void InitSequence_KEY_WAIT( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_KEY_WAIT
static void InitSequence_ANALYZE ( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_ANALYZE
static void InitSequence_FLASH   ( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_FLASH
static void InitSequence_UPDATE  ( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_UPDATE
static void InitSequence_FADE_OUT( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_FADE_OUT
static void InitSequence_CLEAN_UP( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_CLEAN_UP

// シーケンス終了処理
static void FinishSequence_SETUP   ( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_SETUP
static void FinishSequence_KEY_WAIT( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_KEY_WAIT
static void FinishSequence_ANALYZE ( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_ANALYZE
static void FinishSequence_FLASH   ( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_FLASH
static void FinishSequence_UPDATE  ( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_UPDATE
static void FinishSequence_FADE_OUT( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_FADE_OUT
static void FinishSequence_CLEAN_UP( RESEARCH_CHECK_WORK* work ); // RESEARCH_CHECK_SEQ_CLEAN_UP

// 円グラフ メイン動作
static void UpdateCircleGraphs( RESEARCH_CHECK_WORK* work ); // すべての円グラフを更新する

// VBlankタスク
static void VBlankFunc( GFL_TCB* tcb, void* wk );  // VBlank中の処理

//----------------------------------------------------------------------------------------------
// □LAYER 3 機能
//---------------------------------------------------------------------------------------------- 
// カーソル
static void MoveMenuCursorUp  ( RESEARCH_CHECK_WORK* work ); // 上へ移動する
static void MoveMenuCursorDown( RESEARCH_CHECK_WORK* work ); // 下へ移動する

// 表示する質問
static void ChangeQuestionToNext( RESEARCH_CHECK_WORK* work ); // 次の質問に変更する
static void ChangeQuestionToPrev( RESEARCH_CHECK_WORK* work ); // 前の質問に変更する

// 表示する回答
static void ChangeAnswerToNext( RESEARCH_CHECK_WORK* work ); // 次の回答に変更する
static void ChangeAnswerToPrev( RESEARCH_CHECK_WORK* work ); // 前の回答に変更する
static void ChangeAnswerToTop ( RESEARCH_CHECK_WORK* work ); // 先頭の回答に変更する

// 調査データの表示タイプ
static void SwitchDataDisplayType( RESEARCH_CHECK_WORK* work ); // 調査データの表示タイプを切り替える

//----------------------------------------------------------------------------------------------
// □LAYER 2 個別操作
//---------------------------------------------------------------------------------------------- 
// カーソル
static void ShiftMenuCursorPos( RESEARCH_CHECK_WORK* work, int stride ); // カーソル位置を変更する

// 表示する質問インデックス
static void ShiftQuestionIdx( RESEARCH_CHECK_WORK* work, int stride ); // 表示する質問のインデックスを変更する

// 選択中の回答インデックス
static void ShiftAnswerIdx( RESEARCH_CHECK_WORK* work, int stride ); // 選択中の回答インデックスを変更する
static void ResetAnswerIdx( RESEARCH_CHECK_WORK* work );             // 選択中の回答インデックスをリセットする

// 調査データ表示タイプ
static void SetDataDisplayType( RESEARCH_CHECK_WORK* work, DATA_DISP_TYPE dispType ); // 調査データの表示タイプを設定する

// メニュー項目の表示
static void SetMenuCursorOn ( RESEARCH_CHECK_WORK* work ); // カーソルが乗っている状態にする
static void SetMenuCursorOff( RESEARCH_CHECK_WORK* work ); // カーソルが乗っていない状態にする

// タッチ範囲
static void UpdateTouchArea( RESEARCH_CHECK_WORK* work ); // タッチ範囲を更新する

// BG
static void UpdateMainBG_WINDOW( RESEARCH_CHECK_WORK* work ); // MAIN-BG ( ウィンドウ面 ) を更新する

// BGFont
static void UpdateBGFont_TopicCaption   ( RESEARCH_CHECK_WORK* work ); // 調査項目の補足文を更新する
static void UpdateBGFont_QuestionCaption( RESEARCH_CHECK_WORK* work ); // 質問の補足文を更新する
static void UpdateBGFont_Question       ( RESEARCH_CHECK_WORK* work ); // 質問を更新する 
static void UpdateBGFont_Answer         ( RESEARCH_CHECK_WORK* work ); // 回答を更新する
static void UpdateBGFont_MyAnswer       ( RESEARCH_CHECK_WORK* work ); // 自分の回答を更新する
static void UpdateBGFont_Count          ( RESEARCH_CHECK_WORK* work ); // 回答人数を更新する
static void UpdateBGFont_NoData         ( RESEARCH_CHECK_WORK* work ); //「ただいま ちょうさちゅう」の表示を更新する
static void UpdateBGFont_DataReceiving  ( RESEARCH_CHECK_WORK* work ); //「データしゅとくちゅう」の表示を更新する

// OBJの表示
static void UpdateControlCursor( RESEARCH_CHECK_WORK* work ); // 操作カーソルの表示を更新する

// BMP-OAM
static void BmpOamSetDrawEnable( RESEARCH_CHECK_WORK* work, BMPOAM_ACTOR_INDEX BmpOamActorIdx, BOOL enable );  // 表示するかどうかを設定する

// パレットフェード
static void StartPaletteFadeOut( RESEARCH_CHECK_WORK* work ); // パレットのフェードアウトを開始する
static void StartPaletteFadeIn ( RESEARCH_CHECK_WORK* work ); // パレットのフェードインを開始する
static BOOL IsPaletteFadeEnd   ( RESEARCH_CHECK_WORK* work ); // パレットのフェードが完了したかどうかを判定する

// 円グラフ
static void SetupMainCircleGraph( RESEARCH_CHECK_WORK* work, DATA_DISP_TYPE dispType ); // メイン円グラフを現在の調査データで構成する
static void SetupSubCircleGraph ( RESEARCH_CHECK_WORK* work, DATA_DISP_TYPE dispType ); // サブ円グラフを現在の調査データで構成する
static void InterchangeCircleGraph( RESEARCH_CHECK_WORK* work ); // サブ円グラフとメイン円グラフを入れ替える

//----------------------------------------------------------------------------------------------
// □LAYER 1 データアクセス
//----------------------------------------------------------------------------------------------
// 調査データ
static u8 GetTopicID( const RESEARCH_CHECK_WORK* work ); // 現在表示中の調査項目ID
static u8 GetQuestionID( const RESEARCH_CHECK_WORK* work ); // 現在表示中の質問ID
static u8 GetAnswerNum( const RESEARCH_CHECK_WORK* work ); // 現在表示中の質問に対する回答選択肢の数
static u16 GetAnswerID( const RESEARCH_CHECK_WORK* work ); // 現在表示中の回答ID
static u16 GetCountOfQuestion     ( const RESEARCH_CHECK_WORK* work ); // 現在表示中の質問に対する, 表示中の回答人数
static u16 GetTodayCountOfQuestion( const RESEARCH_CHECK_WORK* work ); // 現在表示中の質問に対する, 今日の回答人数
static u16 GetTotalCountOfQuestion( const RESEARCH_CHECK_WORK* work ); // 現在表示中の質問に対する, 合計の回答人数
static u16 GetCountOfAnswer     ( const RESEARCH_CHECK_WORK* work ); // 現在表示中の回答に対する, 表示中の回答人数
static u16 GetTodayCountOfAnswer( const RESEARCH_CHECK_WORK* work ); // 現在表示中の回答に対する, 今日の回答人数
static u16 GetTotalCountOfAnswer( const RESEARCH_CHECK_WORK* work ); // 現在表示中の回答に対する, 合計の回答人数

// 円グラフ
static CIRCLE_GRAPH* GetMainGraph( const RESEARCH_CHECK_WORK* work ); // 現在表示中のメイン円グラフ
static CIRCLE_GRAPH* GetSubGraph ( const RESEARCH_CHECK_WORK* work ); // 現在表示中のサブ円グラフ

// OBJ
static u32 GetObjResourceRegisterIndex( const RESEARCH_CHECK_WORK* work, OBJ_RESOURCE_ID resID ); // OBJリソースの登録インデックス
static GFL_CLUNIT* GetClactUnit( const RESEARCH_CHECK_WORK* work, CLUNIT_INDEX unitIdx ); // セルアクターユニット
static GFL_CLWK* GetClactWork( const RESEARCH_CHECK_WORK* work, CLWK_INDEX wkIdx ); // セルアクターワーク

// BMP-OAM
static BMPOAM_ACT_PTR GetBmpOamActorOfMenuItem( const RESEARCH_CHECK_WORK* work, MENU_ITEM menuItem ); // メニュー項目に対応するBMP-OAMアクター

//----------------------------------------------------------------------------------------------
// □LAYER 0 初期化処理/終了処理
//----------------------------------------------------------------------------------------------
// 画面の準備/後片付け ( BG )
static void SetupBG  ( RESEARCH_CHECK_WORK* work ); // BG 準備
static void CleanUpBG( RESEARCH_CHECK_WORK* work ); // BG 後片付け
static void SetupSubBG_WINDOW  ( RESEARCH_CHECK_WORK* work ); // SUB-BG ( ウィンドウ面 ) 準備
static void CleanUpSubBG_WINDOW( RESEARCH_CHECK_WORK* work ); // SUB-BG ( ウィンドウ面 ) 後片付け
static void SetupSubBG_FONT  ( RESEARCH_CHECK_WORK* work ); // SUB-BG ( フォント面 ) 準備
static void CleanUpSubBG_FONT( RESEARCH_CHECK_WORK* work ); // SUB-BG ( フォント面 ) 後片付け
static void SetupMainBG_WINDOW  ( RESEARCH_CHECK_WORK* work ); // MAIN-BG ( ウィンドウ面 ) 準備
static void CleanUpMainBG_WINDOW( RESEARCH_CHECK_WORK* work ); // MAIN-BG ( ウィンドウ面 ) 後片付け
static void SetupMainBG_FONT  ( RESEARCH_CHECK_WORK* work ); // MAIN-BG ( フォント面 ) 準備
static void CleanUpMainBG_FONT( RESEARCH_CHECK_WORK* work ); // MAIN-BG ( フォント面 ) 後片付け

// 画面の準備/後片付け ( OBJ )
static void CreateClactSystem( RESEARCH_CHECK_WORK* work ); // OBJ システム 生成
static void DeleteClactSystem( RESEARCH_CHECK_WORK* work ); // OBJ システム 破棄
static void RegisterSubObjResources( RESEARCH_CHECK_WORK* work ); // SUB-OBJ リソース 登録
static void ReleaseSubObjResources ( RESEARCH_CHECK_WORK* work ); // SUB-OBJ リソース 解放
static void RegisterMainObjResources( RESEARCH_CHECK_WORK* work ); // MAIN-OBJ リソース 登録
static void ReleaseMainObjResources ( RESEARCH_CHECK_WORK* work ); // MAIN-OBJ リソース 解放
static void InitClactUnits  ( RESEARCH_CHECK_WORK* work ); // セルアクターユニット 初期化
static void CreateClactUnits( RESEARCH_CHECK_WORK* work ); // セルアクターユニット 生成
static void DeleteClactUnits( RESEARCH_CHECK_WORK* work ); // セルアクターユニット 破棄
static void InitClactWorks  ( RESEARCH_CHECK_WORK* work ); // セルアクターワーク 初期化
static void CreateClactWorks( RESEARCH_CHECK_WORK* work ); // セルアクターワーク 生成
static void DeleteClactWorks( RESEARCH_CHECK_WORK* work ); // セルアクターワーク 破棄

// BMP-OAM の準備/後片付け
static void InitBitmapDatas  ( RESEARCH_CHECK_WORK* work ); // ビットマップデータ 初期化
static void CreateBitmapDatas( RESEARCH_CHECK_WORK* work ); // ビットマップデータ 作成
static void SetupBitmapDatas ( RESEARCH_CHECK_WORK* work ); // ビットマップデータ 準備
static void DeleteBitmapDatas( RESEARCH_CHECK_WORK* work ); // ビットマップデータ 破棄
static void SetupBmpOamSystem  ( RESEARCH_CHECK_WORK* work ); // BMP-OAM システム 準備
static void CleanUpBmpOamSystem( RESEARCH_CHECK_WORK* work ); // BMP-OAM システム 後片付け
static void CreateBmpOamActors( RESEARCH_CHECK_WORK* work ); // BMP-OAM アクター 作成
static void DeleteBmpOamActors( RESEARCH_CHECK_WORK* work ); // BMP-OAM アクター 破棄

// 3D 準備
static void Setup3D();

// VBlankタスクの登録/解除
static void RegisterVBlankTask( RESEARCH_CHECK_WORK* work );
static void ReleaseVBlankTask ( RESEARCH_CHECK_WORK* work );

// パレットフェードシステムの準備/後片付け
static void InitPaletteFadeSystem   ( RESEARCH_CHECK_WORK* work ); // パレットフェードシステム 初期化
static void SetupPaletteFadeSystem  ( RESEARCH_CHECK_WORK* work ); // パレットフェードシステム 準備
static void CleanUpPaletteFadeSystem( RESEARCH_CHECK_WORK* work ); // パレットフェードシステム 後片付け

// データの初期化/生成/破棄
static void InitSeqQueue  ( RESEARCH_CHECK_WORK* work ); // シーケンスキュー 初期化
static void CreateSeqQueue( RESEARCH_CHECK_WORK* work ); // シーケンスキュー 作成
static void DeleteSeqQueue( RESEARCH_CHECK_WORK* work ); // シーケンスキュー 破棄
static void InitFont  ( RESEARCH_CHECK_WORK* work ); // フォント 初期化
static void CreateFont( RESEARCH_CHECK_WORK* work ); // フォント 生成
static void DeleteFont( RESEARCH_CHECK_WORK* work ); // フォント 破棄
static void InitMessages  ( RESEARCH_CHECK_WORK* work ); // メッセージ 初期化
static void CreateMessages( RESEARCH_CHECK_WORK* work ); // メッセージ 生成
static void DeleteMessages( RESEARCH_CHECK_WORK* work ); // メッセージ 破棄
static void InitWordset  ( RESEARCH_CHECK_WORK* work ); // ワードセット 初期化
static void CreateWordset( RESEARCH_CHECK_WORK* work ); // ワードセット 生成
static void DeleteWordset( RESEARCH_CHECK_WORK* work ); // ワードセット 破棄
static void InitBGFonts  ( RESEARCH_CHECK_WORK* work ); // 文字列描画オブジェクト 初期化
static void CreateBGFonts( RESEARCH_CHECK_WORK* work ); // 文字列描画オブジェクト 生成
static void DeleteBGFonts( RESEARCH_CHECK_WORK* work ); // 文字列描画オブジェクト 破棄
static void InitCircleGraphs ( RESEARCH_CHECK_WORK* work ); // 円グラフ 初期化
static void CreateCircleGraph( RESEARCH_CHECK_WORK* work ); // 円グラフ 生成
static void DeleteCircleGraph( RESEARCH_CHECK_WORK* work ); // 円グラフ 破棄
static void InitResearchData( RESEARCH_CHECK_WORK* work ); // 調査データ 初期化
static void SetupResearchData( RESEARCH_CHECK_WORK* work ); // 調査データ 取得
static void SetupTouchArea( RESEARCH_CHECK_WORK* work ); // タッチ領域 準備

//----------------------------------------------------------------------------------------------
// □LAYER -1 デバッグ
//----------------------------------------------------------------------------------------------
static void DebugPrint_seqQueue( const RESEARCH_CHECK_WORK* work ); // シーケンスキューの中身を表示する
static void DebugPrint_researchData( const RESEARCH_CHECK_WORK* work ); // 調査データを表示する
static void Debug_SetupResearchData( RESEARCH_CHECK_WORK* work ); // 調査データを設定する


//==============================================================================================
// □調査報告確認画面 制御関数
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査報告確認画面ワークの生成
 *
 * @param commonWork 全画面共通ワーク
 *
 * @return 調査報告確認画面ワーク
 */
//----------------------------------------------------------------------------------------------
RESEARCH_CHECK_WORK* CreateResearchCheckWork( RESEARCH_COMMON_WORK* commonWork )
{
  int i;
  RESEARCH_CHECK_WORK* work;
  HEAPID heapID;

  heapID = RESEARCH_COMMON_GetHeapID( commonWork );

  // 生成
  work = GFL_HEAP_AllocMemory( heapID, sizeof(RESEARCH_CHECK_WORK) );

  // 初期化
  work->commonWork      = commonWork;
  work->heapID          = heapID;
  work->gameSystem      = RESEARCH_COMMON_GetGameSystem( commonWork );
  work->gameData        = RESEARCH_COMMON_GetGameData( commonWork );
  work->seq             = RESEARCH_CHECK_SEQ_SETUP;
  work->seqFinishFlag   = FALSE;
  work->seqCount        = 0;
  work->result          = RESEARCH_CHECK_RESULT_NONE;
  work->cursorPos       = MENU_ITEM_QUESTION;
  work->analyzeFlag     = FALSE;
  work->questionIdx     = 0;
  work->answerIdx       = 0;
  work->dispType        = DATA_DISP_TYPE_TODAY;
  work->VBlankTCBSystem = GFUser_VIntr_GetTCBSYS();

  for( i=0; i<OBJ_RESOURCE_NUM; i++ ){ work->objResRegisterIdx[i] = 0; }

  InitResearchData( work );
  InitSeqQueue( work );
  InitCircleGraphs( work );
  InitMessages( work );
  InitWordset( work );
  InitFont( work );
  InitBGFonts( work );
  InitClactUnits( work );
  InitClactWorks( work );
  InitBitmapDatas( work );
  InitPaletteFadeSystem( work );

  CreateSeqQueue( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: create work\n" );

  return work;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査報告確認画面ワークの破棄
 *
 * @param heapID
 */
//----------------------------------------------------------------------------------------------
void DeleteResearchCheckWork( RESEARCH_CHECK_WORK* work )
{
  if( work == NULL )
  {
    GF_ASSERT(0);
    return;
  }
  DeleteSeqQueue( work ); // シーケンスキュー
  GFL_HEAP_FreeMemory( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: delete work\n" );
} 

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査報告確認画面 メイン動作
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
RESEARCH_CHECK_RESULT ResearchCheckMain( RESEARCH_CHECK_WORK* work )
{
  // シーケンスごとの処理
  switch( work->seq )
  {
  case RESEARCH_CHECK_SEQ_SETUP:     Main_SETUP( work );     break;
  case RESEARCH_CHECK_SEQ_KEY_WAIT:  Main_KEY_WAIT( work );  break;
  case RESEARCH_CHECK_SEQ_ANALYZE:   Main_ANALYZE( work );   break;
  case RESEARCH_CHECK_SEQ_FLASH:     Main_FLASH( work );     break;
  case RESEARCH_CHECK_SEQ_UPDATE:    Main_UPDATE( work );    break;
  case RESEARCH_CHECK_SEQ_FADE_OUT:  Main_FADE_OUT( work );  break;
  case RESEARCH_CHECK_SEQ_CLEAN_UP:  Main_CLEAN_UP( work );  break;
  case RESEARCH_CHECK_SEQ_FINISH:    return work->result;
  default: GF_ASSERT(0);
  }

  // セルアクターシステム メイン処理
  GFL_CLACT_SYS_Main();

  // 円グラフ メイン動作
  UpdateCircleGraphs( work );

  // シーケンスカウンタ更新
  CountUpSeqCount( work );

  // シーケンス更新
  SwitchSequence( work ); 

  // 3D描画
  G3_SwapBuffers( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );

  return RESEARCH_CHECK_RESULT_CONTINUE;
}




//==============================================================================================
// ■LAYER 4 シーケンス動作
//==============================================================================================

//----------------------------------------------------------------------------------------------
/**
 * @brief 準備シーケンス ( RESEARCH_CHECK_SEQ_SETUP ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_SETUP( RESEARCH_CHECK_WORK* work )
{
  CreateFont( work );
  CreateMessages( work );
  CreateWordset( work );
  SetupTouchArea( work );
  UpdateTouchArea( work );

  // 3D 初期設定
  Setup3D();

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

  // BMP-OAM 準備
  CreateBitmapDatas( work );
  SetupBitmapDatas( work );
  SetupBmpOamSystem( work );
  CreateBmpOamActors( work );

  // データを取得
  //SetupResearchData( work );
  Debug_SetupResearchData( work ); // TEST:
  DebugPrint_researchData( work ); // TEST:

  // 円グラフ 作成
  CreateCircleGraph( work );

  // パレットフェードシステム 準備
  SetupPaletteFadeSystem( work );

  // VBkankタスク登録
  RegisterVBlankTask( work );

  // 画面フェードイン
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 0);

  // 次のシーケンスをセット
  SetNextSequence( work, RESEARCH_CHECK_SEQ_KEY_WAIT ); 

  // シーケンス終了
  FinishCurrentSequence( work );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief キー入力待ちシーケンス ( RESEARCH_CHECK_SEQ_KEY_WAIT ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_KEY_WAIT( RESEARCH_CHECK_WORK* work )
{
  int key;
  int trg;
  int touchedAreaIdx;

  key = GFL_UI_KEY_GetCont();
  trg = GFL_UI_KEY_GetTrg(); 
  touchedAreaIdx = GFL_UI_TP_HitTrg( work->touchHitTable );

  // TEST:
  if( trg & PAD_BUTTON_DEBUG )
  {
    SetNextSequence( work, RESEARCH_CHECK_SEQ_UPDATE );
    SetNextSequence( work, RESEARCH_CHECK_SEQ_KEY_WAIT );
    FinishCurrentSequence( work );
    return;
  }

  //--------
  // ↑キー
  if( trg & PAD_KEY_UP ) {
    MoveMenuCursorUp( work );
    return;
  }

  //--------
  // ↓キー
  if( trg & PAD_KEY_DOWN ) {
    MoveMenuCursorDown( work );
    return;
  } 

  //----------------------
  // ←キー or 左カーソル
  if( (trg & PAD_KEY_LEFT) ||
      (touchedAreaIdx == TOUCH_AREA_CONTROL_CURSOR_L) ) {
    switch( work->cursorPos )
    {
    case MENU_ITEM_QUESTION:  ChangeQuestionToPrev( work );  break;
    case MENU_ITEM_ANSWER:    ChangeAnswerToPrev( work );    break;
    case MENU_ITEM_MY_ANSWER: break;
    case MENU_ITEM_COUNT:     SwitchDataDisplayType( work ); break;
    default: GF_ASSERT(0);
    }
  }

  //----------------------
  // →キー or 右カーソル
  if( (trg & PAD_KEY_RIGHT) ||
      (touchedAreaIdx == TOUCH_AREA_CONTROL_CURSOR_R) ) {
    switch( work->cursorPos )
    {
    case MENU_ITEM_QUESTION:  ChangeQuestionToNext( work );  break;
    case MENU_ITEM_ANSWER:    ChangeAnswerToNext( work );    break;
    case MENU_ITEM_MY_ANSWER: break;
    case MENU_ITEM_COUNT:     SwitchDataDisplayType( work ); break;
    default: GF_ASSERT(0);
    }
  }

  if( touchedAreaIdx == TOUCH_AREA_QUESTION ) {
    if( (work->analyzeFlag == FALSE ) &&
        (GetCountOfQuestion(work) != 0) ) {
      // シーケンス変更
      SetNextSequence( work, RESEARCH_CHECK_SEQ_ANALYZE );
      SetNextSequence( work, RESEARCH_CHECK_SEQ_FLASH );
      SetNextSequence( work, RESEARCH_CHECK_SEQ_KEY_WAIT );
      FinishCurrentSequence( work );
      return;
    }
  }

  if( trg & PAD_BUTTON_A ) {
    if( (work->analyzeFlag == FALSE ) &&
        (GetCountOfQuestion(work) != 0) && 
        (work->cursorPos == MENU_ITEM_QUESTION)  ) {
      // シーケンス変更
      SetNextSequence( work, RESEARCH_CHECK_SEQ_ANALYZE );
      SetNextSequence( work, RESEARCH_CHECK_SEQ_FLASH );
      SetNextSequence( work, RESEARCH_CHECK_SEQ_KEY_WAIT );
      FinishCurrentSequence( work );
      return;
    }
  } 

  //----------
  // B ボタン
  if( trg & PAD_BUTTON_B ) {
    // シーケンス変更
    SetNextSequence( work, RESEARCH_CHECK_SEQ_FADE_OUT );
    SetNextSequence( work, RESEARCH_CHECK_SEQ_CLEAN_UP );
    FinishCurrentSequence( work );
    return;
  }

  //-----------------
  //「もどる」ボタン
  if( touchedAreaIdx == TOUCH_AREA_RETURN_BUTTON ) {
    // シーケンス変更
    SetNextSequence( work, RESEARCH_CHECK_SEQ_FADE_OUT );
    SetNextSequence( work, RESEARCH_CHECK_SEQ_CLEAN_UP );
    FinishCurrentSequence( work );
    return;
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 解析シーケンス ( RESEARCH_CHECK_SEQ_ANALYZE ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_ANALYZE( RESEARCH_CHECK_WORK* work )
{ 
  // SE が停止している
  if( PMSND_CheckPlaySE() == FALSE ) { 
    // 一定時間が経過で次のシーケンスへ
    if( SEQ_ANALYZE_FRAMES <= work->seqCount ) {
      FinishCurrentSequence( work );
    } 
    else { 
      // データ解析中SEをループさせる
      PMSND_PlaySE( SEQ_SE_SYS_81 );
    }
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 画面フラッシュシーケンス ( RESEARCH_CHECK_SEQ_FLASH ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_FLASH( RESEARCH_CHECK_WORK* work )
{
  // 一定時間が経過で次のシーケンスへ
  if( SEQ_FLASH_FRAMES <= work->seqCount )
  {
    FinishCurrentSequence( work );
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定の確認シーケンスへの準備シーケンス ( RESEARCH_CHECK_SEQ_UPDATE ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_UPDATE( RESEARCH_CHECK_WORK* work )
{
  // 一定時間が経過で次のシーケンスへ
  if( SEQ_UPDATE_FRAMES <= work->seqCount )
  {
    FinishCurrentSequence( work );
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定の確認シーケンスへの準備シーケンス ( RESEARCH_CHECK_SEQ_FADE_OUT ) の処理
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Main_FADE_OUT( RESEARCH_CHECK_WORK* work )
{
  // フェードが終了
  if( GFL_FADE_CheckFade() == FALSE ) {
    FinishCurrentSequence( work );
  } 
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_CHECK_SEQ_CLEAN_UP ) の処理
 *
 * @param work
 *
 * @return シーケンスが変化する場合 次のシーケンス番号
 *         シーケンスが継続する場合 現在のシーケンス番号
 */
//----------------------------------------------------------------------------------------------
static void Main_CLEAN_UP( RESEARCH_CHECK_WORK* work )
{ 
  // VBlankタスクを解除
  ReleaseVBlankTask( work );

  // 円グラフ 削除
  DeleteCircleGraph( work ); 

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
  CleanUpMainBG_WINDOW( work );
  CleanUpSubBG_FONT   ( work );
  CleanUpSubBG_WINDOW ( work );
  CleanUpBG           ( work );

  DeleteWordset( work );
  DeleteMessages( work );
  DeleteFont( work );

  // 画面終了結果を決定
  SetResult( work, RESEARCH_CHECK_RESULT_TO_MENU );  

  // シーケンス終了
  SetNextSequence( work, RESEARCH_CHECK_SEQ_FINISH );
  FinishCurrentSequence( work );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在のシーケンスを終了する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishCurrentSequence( RESEARCH_CHECK_WORK* work )
{
  // すでに終了済み
  GF_ASSERT( work->seqFinishFlag == FALSE );

  // 終了フラグを立てる
  work->seqFinishFlag = TRUE;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: finish current sequence\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 画面の終了結果を決定する
 *
 * @param work
 * @param result 結果
 */
//----------------------------------------------------------------------------------------------
static void SetResult( RESEARCH_CHECK_WORK* work, RESEARCH_CHECK_RESULT result )
{
  // 多重設定
  GF_ASSERT( work->result == RESEARCH_CHECK_RESULT_NONE );

  // 設定
  work->result = result;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: set result (%d)\n", result );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスカウンタを更新する
 * 
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CountUpSeqCount( RESEARCH_CHECK_WORK* work )
{
  u32 maxCount;

  // インクリメント
  work->seqCount++;

  // 最大値を決定
  switch( work->seq )
  {
  case RESEARCH_CHECK_SEQ_SETUP:    maxCount = 0xffffffff;         break;
  case RESEARCH_CHECK_SEQ_KEY_WAIT: maxCount = 0xffffffff;         break;
  case RESEARCH_CHECK_SEQ_ANALYZE:  maxCount = SEQ_ANALYZE_FRAMES; break;
  case RESEARCH_CHECK_SEQ_FLASH:    maxCount = SEQ_FLASH_FRAMES;   break;
  case RESEARCH_CHECK_SEQ_UPDATE:   maxCount = SEQ_UPDATE_FRAMES;  break;
  case RESEARCH_CHECK_SEQ_FADE_OUT: maxCount = 0xffffffff;         break;
  case RESEARCH_CHECK_SEQ_CLEAN_UP: maxCount = 0xffffffff;         break;
  case RESEARCH_CHECK_SEQ_FINISH:   maxCount = 0xffffffff;         break;
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
static void SetNextSequence( RESEARCH_CHECK_WORK* work, RESEARCH_CHECK_SEQ nextSeq )
{
  // シーケンスキューに追加する
  QUEUE_EnQueue( work->seqQueue, nextSeq );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: set next seq\n" );
  DebugPrint_seqQueue( work );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを変更する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SwitchSequence( RESEARCH_CHECK_WORK* work )
{
  RESEARCH_CHECK_SEQ nextSeq;

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
static void SetSequence( RESEARCH_CHECK_WORK* work, RESEARCH_CHECK_SEQ nextSeq )
{ 
  // シーケンスの終了処理
  switch( work->seq )
  {
  case RESEARCH_CHECK_SEQ_SETUP:    FinishSequence_SETUP( work );     break;
  case RESEARCH_CHECK_SEQ_KEY_WAIT: FinishSequence_KEY_WAIT( work );  break;
  case RESEARCH_CHECK_SEQ_ANALYZE:  FinishSequence_ANALYZE( work );   break;
  case RESEARCH_CHECK_SEQ_FLASH:    FinishSequence_FLASH( work );     break;
  case RESEARCH_CHECK_SEQ_UPDATE:   FinishSequence_UPDATE( work );    break;
  case RESEARCH_CHECK_SEQ_FADE_OUT: FinishSequence_FADE_OUT( work );    break;
  case RESEARCH_CHECK_SEQ_CLEAN_UP: FinishSequence_CLEAN_UP( work );  break;
  case RESEARCH_CHECK_SEQ_FINISH:   break;
  default:  GF_ASSERT(0);
  }

  // 更新
  work->seq           = nextSeq;
  work->seqCount      = 0;
  work->seqFinishFlag = FALSE;

  // シーケンスの初期化処理
  switch( nextSeq )
  {
  case RESEARCH_CHECK_SEQ_SETUP:    InitSequence_SETUP( work );     break;
  case RESEARCH_CHECK_SEQ_KEY_WAIT: InitSequence_KEY_WAIT( work );  break;
  case RESEARCH_CHECK_SEQ_ANALYZE:  InitSequence_ANALYZE( work );   break;
  case RESEARCH_CHECK_SEQ_FLASH:    InitSequence_FLASH( work );     break;
  case RESEARCH_CHECK_SEQ_UPDATE:   InitSequence_UPDATE( work );    break;
  case RESEARCH_CHECK_SEQ_FADE_OUT: InitSequence_FADE_OUT( work );    break;
  case RESEARCH_CHECK_SEQ_CLEAN_UP: InitSequence_CLEAN_UP( work );  break;
  case RESEARCH_CHECK_SEQ_FINISH:   break;
  default:  GF_ASSERT(0);
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: set seq ==> " );
  switch( nextSeq )
  {
  case RESEARCH_CHECK_SEQ_SETUP:    OS_TFPrintf( PRINT_TARGET, "SETUP" );     break;
  case RESEARCH_CHECK_SEQ_KEY_WAIT: OS_TFPrintf( PRINT_TARGET, "KEY_WAIT" );  break;
  case RESEARCH_CHECK_SEQ_ANALYZE:  OS_TFPrintf( PRINT_TARGET, "ANALYZE" );   break;
  case RESEARCH_CHECK_SEQ_FLASH:    OS_TFPrintf( PRINT_TARGET, "FLASH" );     break;
  case RESEARCH_CHECK_SEQ_UPDATE:   OS_TFPrintf( PRINT_TARGET, "UPDATE" );    break;
  case RESEARCH_CHECK_SEQ_FADE_OUT: OS_TFPrintf( PRINT_TARGET, "FADE_OUT" );    break;
  case RESEARCH_CHECK_SEQ_CLEAN_UP: OS_TFPrintf( PRINT_TARGET, "CLEAN_UP" );  break;
  case RESEARCH_CHECK_SEQ_FINISH:   OS_TFPrintf( PRINT_TARGET, "FINISH" );    break;
  default:  GF_ASSERT(0);
  }
  OS_TFPrintf( PRINT_TARGET, "\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_CHECK_SEQ_SETUP )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_SETUP( RESEARCH_CHECK_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init seq SETUP\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_CHECK_SEQ_KEY_WAIT )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_KEY_WAIT( RESEARCH_CHECK_WORK* work )
{
  UpdateMainBG_WINDOW( work );        // MAIN-BG ( ウィンドウ面 ) を更新する
  SetMenuCursorOn( work );            // カーソルが乗っている状態にする
  UpdateBGFont_DataReceiving( work ); //「データしゅとくちゅう」の表示を更新する
  UpdateBGFont_Answer( work );        // 回答を更新する

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init seq KEY_WAIT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_CHECK_SEQ_ANALYZE )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_ANALYZE( RESEARCH_CHECK_WORK* work )
{
  //「…かいせきちゅう…」を表示
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_ANALYZING, TRUE );

  // データを取得
  //SetupResearchData( work );
  Debug_SetupResearchData( work ); // TEST:
  DebugPrint_researchData( work ); // TEST:

  // 円グラフ作成
  SetupMainCircleGraph( work, DATA_DISP_TYPE_TODAY );
  SetupMainCircleGraph( work, DATA_DISP_TYPE_TOTAL );

  // 円グラフ表示開始
  CIRCLE_GRAPH_SetDrawEnable( GetMainGraph(work), TRUE );
  CIRCLE_GRAPH_AnalyzeReq( GetMainGraph(work) );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init seq ANALYZE\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_CHECK_SEQ_FLASH )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_FLASH( RESEARCH_CHECK_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init seq FLASH\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_CHECK_SEQ_UPDATE )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_UPDATE( RESEARCH_CHECK_WORK* work )
{
  UpdateMainBG_WINDOW( work );        // MAIN-BG ( ウィンドウ面 ) を更新する
  SetMenuCursorOn( work );            // カーソルが乗っている状態にする
  UpdateBGFont_DataReceiving( work ); //「データしゅとくちゅう」の表示を更新する

  // 調査データを取得
  //SetupResearchData( work );
  Debug_SetupResearchData( work ); // TEST:
  DebugPrint_researchData( work ); // TEST:

  // サブ円グラフ作成
  SetupSubCircleGraph( work, DATA_DISP_TYPE_TODAY );
  SetupSubCircleGraph( work, DATA_DISP_TYPE_TOTAL );

  // 円グラフ表示開始
  CIRCLE_GRAPH_SetDrawEnable( GetSubGraph(work), TRUE );
  CIRCLE_GRAPH_UpdateReq( GetSubGraph(work) );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init seq UPDATE\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_CHECK_SEQ_FADE_OUT )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_FADE_OUT( RESEARCH_CHECK_WORK* work )
{ 
  // フェードアウト開始
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 0);

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init seq FADE_OUT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスを初期化する ( ==> RESEARCH_CHECK_SEQ_CLEAN_UP )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSequence_CLEAN_UP( RESEARCH_CHECK_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init seq CLEAN_UP\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_CHECK_SEQ_SETUP )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_SETUP( RESEARCH_CHECK_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: finish seq SETUP\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_CHECK_SEQ_KEY_WAIT )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_KEY_WAIT( RESEARCH_CHECK_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: finish seq KEY_WAIT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_CHECK_SEQ_ANALYZE )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_ANALYZE( RESEARCH_CHECK_WORK* work )
{
  // 解析済みフラグを立てる
  work->analyzeFlag = TRUE;

  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_ANALYZING, FALSE ); //「…かいせきちゅう…」を消す
  UpdateMainBG_WINDOW( work );   // MAIN-BG ( ウィンドウ面 ) を更新する
  SetMenuCursorOn( work );       // カーソルが乗っている状態にする
  UpdateBGFont_Answer( work );   // 回答を更新する
  UpdateBGFont_MyAnswer( work ); // 自分の回答を更新する
  UpdateBGFont_Count( work );    // 回答人数を更新する

  // 調査結果表示SE
  PMSND_PlaySE( SEQ_SE_SYS_82 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: finish seq ANALYZE\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_CHECK_SEQ_FLASH )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_FLASH( RESEARCH_CHECK_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: finish seq FLASH\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_CHECK_SEQ_UPDATE )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_UPDATE( RESEARCH_CHECK_WORK* work )
{
  UpdateMainBG_WINDOW( work );        // MAIN-BG ( ウィンドウ面 ) を更新する
  SetMenuCursorOn( work );            // カーソルが乗っている状態にする
  UpdateBGFont_DataReceiving( work ); //「データしゅとくちゅう」の表示を更新する

  // サブ円グラフとメイン円グラフを入れ替える
  InterchangeCircleGraph( work );

  // サブ円グラフ ( 元メイン ) 表示終了
  CIRCLE_GRAPH_SetDrawEnable( GetSubGraph(work), FALSE );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: finish seq UPDATE\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_CHECK_SEQ_FADE_OUT )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_FADE_OUT( RESEARCH_CHECK_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: finish seq FADE_OUT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンス終了処理 ( ==> RESEARCH_CHECK_SEQ_CLEAN_UP )
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void FinishSequence_CLEAN_UP( RESEARCH_CHECK_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: finish seq CLEAN_UP\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief すべての円グラフを更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateCircleGraphs( RESEARCH_CHECK_WORK* work )
{
  int typeIdx;

  for( typeIdx=0; typeIdx < DATA_DISP_TYPE_NUM; typeIdx++ )
  {
    CIRCLE_GRAPH_Main( work->mainGraph[ typeIdx ] );
    CIRCLE_GRAPH_Main( work->subGraph[ typeIdx ] );
  }
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
  RESEARCH_CHECK_WORK* work = (RESEARCH_CHECK_WORK*)wk;

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
static void MoveMenuCursorUp( RESEARCH_CHECK_WORK* work )
{ 
  // 表示を更新
  SetMenuCursorOff( work );  // カーソルが乗っていない状態にする

  // カーソル移動
  ShiftMenuCursorPos( work, -1 );

  // カーソルが「回答」の位置にある場合
  if( work->cursorPos == MENU_ITEM_ANSWER )
  {
    if( work->analyzeFlag == FALSE ) { // 未解析
      ShiftMenuCursorPos( work, -1 ); 
    }
    else if( GetCountOfQuestion(work) == 0 ) { //「ただいま ちょうさちゅう」
      ShiftMenuCursorPos( work, -1 ); 
    }
  }

  // 表示を更新
  SetMenuCursorOn( work );      // カーソルが乗っている状態にする
  UpdateControlCursor( work );  // 左右カーソルを更新

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );

  // タッチ範囲を更新
  UpdateTouchArea( work );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを下へ移動する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void MoveMenuCursorDown( RESEARCH_CHECK_WORK* work )
{
  // 表示を更新
  SetMenuCursorOff( work );  // カーソルが乗っていない状態にする

  // カーソル移動
  ShiftMenuCursorPos( work, 1 );

  // カーソルが「回答」の位置にある場合
  if( work->cursorPos == MENU_ITEM_ANSWER )
  {
    if( work->analyzeFlag == FALSE ) { // 未解析
      ShiftMenuCursorPos( work, 1 ); 
    }
    else if( GetCountOfQuestion(work) == 0 ) { //「ただいま ちょうさちゅう」
      ShiftMenuCursorPos( work, 1 ); 
    }
  }

  // 表示を更新
  SetMenuCursorOn( work );      // カーソルが乗っている状態にする
  UpdateControlCursor( work );  // 左右カーソルを更新

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );

  // タッチ範囲を更新
  UpdateTouchArea( work );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 表示する質問を, 次の質問に変更する
 *
 * @param work
 */
//---------------------------------------------------------------------------------------------- 
static void ChangeQuestionToNext( RESEARCH_CHECK_WORK* work )
{
  // 表示中の円グラフを消去する
  if( work->analyzeFlag && GetCountOfQuestion(work) != 0 ) {
    CIRCLE_GRAPH_DisappearReq( GetMainGraph(work) );
  }

  // データを更新
  ShiftQuestionIdx( work, 1 ); // 表示する質問インデックスを変更
  work->analyzeFlag = FALSE;   // 解析済みフラグを伏せる

  // 表示を更新
  UpdateMainBG_WINDOW( work );          // MAIN-BG ( ウィンドウ面 ) を更新する
  SetMenuCursorOn( work );              // カーソルが乗っている状態にする
  UpdateBGFont_QuestionCaption( work ); // 質問の補足文を更新する
  UpdateBGFont_Question( work );        // 質問を更新する 
  ChangeAnswerToTop( work );            // 回答を更新する
  UpdateBGFont_MyAnswer( work );        // 自分の回答を更新する
  UpdateBGFont_Count( work );           // 回答人数を更新する
  UpdateBGFont_NoData( work );          //「ただいま ちょうさちゅう」の表示を更新する

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 表示する質問を, 前の質問に変更する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ChangeQuestionToPrev( RESEARCH_CHECK_WORK* work )
{
  // 表示中の円グラフを消去する
  if( work->analyzeFlag && GetCountOfQuestion(work) != 0 ) {
    CIRCLE_GRAPH_DisappearReq( GetMainGraph(work) );
  }

  // データを更新
  ShiftQuestionIdx( work, -1 ); // 表示する質問インデックスを変更
  work->analyzeFlag = FALSE;    // 解析済みフラグを伏せる

  // 表示を更新
  UpdateMainBG_WINDOW( work );          // MAIN-BG ( ウィンドウ面 ) を更新する
  SetMenuCursorOn( work );              // カーソルが乗っている状態にする
  UpdateBGFont_QuestionCaption( work ); // 質問の補足文を更新する
  UpdateBGFont_Question( work );        // 質問を更新する 
  ChangeAnswerToTop( work );            // 回答を更新する
  UpdateBGFont_MyAnswer( work );        // 自分の回答を更新する
  UpdateBGFont_Count( work );           // 回答人数を更新する
  UpdateBGFont_NoData( work );          //「ただいま ちょうさちゅう」の表示を更新する

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 表示する回答を, 次の回答に変更する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ChangeAnswerToNext( RESEARCH_CHECK_WORK* work )
{
  ShiftAnswerIdx( work, 1 ); // 表示する回答インデックスを変更

  // 表示を更新
  UpdateBGFont_Answer( work ); // 回答を更新する

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 表示する回答を, 前の回答に変更する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ChangeAnswerToPrev( RESEARCH_CHECK_WORK* work )
{
  ShiftAnswerIdx( work, -1 ); // 表示する回答インデックスを変更

  // 表示を更新
  UpdateBGFont_Answer( work ); // 回答を更新する

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 表示する回答を, 先頭の回答に変更する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ChangeAnswerToTop( RESEARCH_CHECK_WORK* work )
{
  ResetAnswerIdx( work ); // 表示する回答インデックスをリセット

  // 表示を更新
  UpdateBGFont_Answer( work ); // 回答を更新する
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査データの表示タイプを切り替える
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SwitchDataDisplayType( RESEARCH_CHECK_WORK* work )
{
  DATA_DISP_TYPE nextType;
  BOOL newGraphWait = FALSE; // 新しく表示する円グラフに, 表示するまでの待ち時間が必要かどうか

  // 表示中の円グラフを消去する
  if( work->analyzeFlag && GetCountOfQuestion(work) != 0 ) {
    CIRCLE_GRAPH_DisappearReq( GetMainGraph(work) );
    newGraphWait = TRUE;
  }

  // 変更後の表示タイプを選択
  switch( work->dispType )
  {
  case DATA_DISP_TYPE_TODAY: nextType = DATA_DISP_TYPE_TOTAL; break;
  case DATA_DISP_TYPE_TOTAL: nextType = DATA_DISP_TYPE_TODAY; break;
  default: GF_ASSERT(0);
  }

  // 表示タイプを変更
  SetDataDisplayType( work, nextType );

  // 表示を更新
  UpdateMainBG_WINDOW( work ); // MAIN-BG ( ウィンドウ面 ) を更新
  SetMenuCursorOn( work );     // カーソルが乗っている状態にする
  UpdateBGFont_Answer( work ); // 回答を更新する
  UpdateBGFont_Count( work );  // 回答人数を更新する
  UpdateBGFont_NoData( work ); //「ただいま ちょうさちゅう」の表示を更新する

  // 円グラフを出現させる
  if( work->analyzeFlag && GetCountOfQuestion(work) != 0 ) {
    if( newGraphWait ) { CIRCLE_GRAPH_StopGraph( GetMainGraph(work), 20 ); } // 前グラフの消去を待つ
    CIRCLE_GRAPH_AppearReq( GetMainGraph(work) );
    CIRCLE_GRAPH_SetDrawEnable( GetMainGraph(work), TRUE );
  }

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );
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
static void ShiftMenuCursorPos( RESEARCH_CHECK_WORK* work, int stride )
{
  int nowPos;
  int nextPos;

  // カーソル位置を更新
  nowPos  = work->cursorPos;
  nextPos = (nowPos + stride + MENU_ITEM_NUM) % MENU_ITEM_NUM;
  work->cursorPos = nextPos; 

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: shift menu cursor ==> %d\n", nextPos );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 表示する質問のインデックスを変更する
 *
 * @param work
 * @param stride 移動量
 */
//----------------------------------------------------------------------------------------------
static void ShiftQuestionIdx( RESEARCH_CHECK_WORK* work, int stride )
{
  int nowIdx;
  int nextIdx;

  // インデックスを変更
  nowIdx  = work->questionIdx;
  nextIdx = ( nowIdx + stride + QUESTION_NUM_PER_TOPIC ) % QUESTION_NUM_PER_TOPIC;
  work->questionIdx = nextIdx;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: shift question index ==> %d\n", nextIdx );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 選択中の回答インデックスを変更する
 *
 * @param work
 * @param stride 変更量
 */
//----------------------------------------------------------------------------------------------
static void ShiftAnswerIdx( RESEARCH_CHECK_WORK* work, int stride )
{
  int nowIdx;
  int nextIdx;
  int answerNum;

  // インデックスを変更
  answerNum = GetAnswerNum( work );
  nowIdx    = work->answerIdx;
  nextIdx   = ( nowIdx + stride + answerNum ) % answerNum;
  work->answerIdx = nextIdx;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: shift answer index ==> %d\n", nextIdx );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 選択中の回答インデックスをリセットする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ResetAnswerIdx( RESEARCH_CHECK_WORK* work )
{
  work->answerIdx = 0;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: reset answer index ==> %d\n", work->answerIdx );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査データの表示タイプを設定する
 *
 * @param work
 * @param dispType 設定する表示タイプ
 */
//----------------------------------------------------------------------------------------------
static void SetDataDisplayType( RESEARCH_CHECK_WORK* work, DATA_DISP_TYPE dispType )
{
  work->dispType = dispType;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: set data display type ==> " );
  switch( dispType )
  {
  case DATA_DISP_TYPE_TODAY: OS_TFPrintf( PRINT_TARGET, "TODAY" ); break;
  case DATA_DISP_TYPE_TOTAL: OS_TFPrintf( PRINT_TARGET, "TOTAL" ); break;
  default: GF_ASSERT(0);
  }
  OS_TFPrintf( PRINT_TARGET, "\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief カーソル位置にあるメニュー項目を, カーソルが乗っている状態にする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetMenuCursorOn( RESEARCH_CHECK_WORK* work )
{
  // 該当するスクリーンを更新
  switch( work->cursorPos )
  {
  case MENU_ITEM_QUESTION:  // 質問
  case MENU_ITEM_MY_ANSWER: // 自分の回答
  case MENU_ITEM_COUNT:     // 回答人数
    GFL_BG_ChangeScreenPalette( MAIN_BG_WINDOW, 
                                MenuItemDrawParam[ work->cursorPos ].left_chara,
                                MenuItemDrawParam[ work->cursorPos ].top_chara,
                                MenuItemDrawParam[ work->cursorPos ].width_chara,
                                MenuItemDrawParam[ work->cursorPos ].height_chara,
                                MAIN_BG_PALETTE_MENU_ON );
    break;

  // 回答 ( 上下2分割しているため, 個別処理 )
  case MENU_ITEM_ANSWER:
    GFL_BG_ChangeScreenPalette( MAIN_BG_WINDOW,
                                ANSWER_UPPER_X, ANSWER_UPPER_Y, 
                                ANSWER_UPPER_WIDTH, ANSWER_UPPER_HEIGHT,
                                MAIN_BG_PALETTE_MENU_ON );
    GFL_BG_ChangeScreenPalette( MAIN_BG_WINDOW,
                                ANSWER_LOWER_X, ANSWER_LOWER_Y, 
                                ANSWER_LOWER_WIDTH, ANSWER_LOWER_HEIGHT,
                                MAIN_BG_PALETTE_MENU_ON );
    break;

  // エラー
  default:
    GF_ASSERT(0);
  }

  // スクリーン転送リクエスト
  GFL_BG_LoadScreenReq( MAIN_BG_WINDOW );


  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: set menu cursor on (%d)\n", work->cursorPos );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief カーソル位置にあるメニュー項目を, メニュー項目をカーソルが乗っている状態にする
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetMenuCursorOff( RESEARCH_CHECK_WORK* work )
{
  // 該当するスクリーンを更新
  switch( work->cursorPos )
  {
  case MENU_ITEM_QUESTION:  // 質問
  case MENU_ITEM_MY_ANSWER: // 自分の回答
  case MENU_ITEM_COUNT:     // 回答人数
    GFL_BG_ChangeScreenPalette( MAIN_BG_WINDOW, 
                                MenuItemDrawParam[ work->cursorPos ].left_chara,
                                MenuItemDrawParam[ work->cursorPos ].top_chara,
                                MenuItemDrawParam[ work->cursorPos ].width_chara,
                                MenuItemDrawParam[ work->cursorPos ].height_chara,
                                MAIN_BG_PALETTE_MENU_OFF );
    break;

  // 回答 ( 上下2分割しているため, 個別処理 )
  case MENU_ITEM_ANSWER:
    GFL_BG_ChangeScreenPalette( MAIN_BG_WINDOW,
                                ANSWER_UPPER_X, ANSWER_UPPER_Y, 
                                ANSWER_UPPER_WIDTH, ANSWER_UPPER_HEIGHT,
                                MAIN_BG_PALETTE_MENU_OFF );
    GFL_BG_ChangeScreenPalette( MAIN_BG_WINDOW,
                                ANSWER_LOWER_X, ANSWER_LOWER_Y, 
                                ANSWER_LOWER_WIDTH, ANSWER_LOWER_HEIGHT,
                                MAIN_BG_PALETTE_MENU_OFF );
    break;

  // エラー
  default:
    GF_ASSERT(0);
  }

  // スクリーン転送リクエスト
  GFL_BG_LoadScreenReq( MAIN_BG_WINDOW );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: set menu cursor off (%d)\n", work->cursorPos );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief タッチ範囲を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateTouchArea( RESEARCH_CHECK_WORK* work )
{
  const MENU_ITEM_DRAW_PARAM* menu;
  GFL_UI_TP_HITTBL* area;

  // 左カーソル
  menu = &( MenuItemDrawParam[ work->cursorPos ] );
  area = &( work->touchHitTable[ TOUCH_AREA_CONTROL_CURSOR_L ] );
  area->rect.left   = menu->left_dot + menu->leftCursorOffsetX - CLWK_CONTROL_CURSOR_L_WIDTH/2;
  area->rect.top    = menu->top_dot  + menu->leftCursorOffsetY - CLWK_CONTROL_CURSOR_L_HEIGHT/2;
  area->rect.right  = area->rect.left + TOUCH_AREA_CONTROL_CURSOR_WIDTH;
  area->rect.bottom = area->rect.top  + TOUCH_AREA_CONTROL_CURSOR_HEIGHT;

  // 右カーソル
  menu = &( MenuItemDrawParam[ work->cursorPos ] );
  area = &( work->touchHitTable[ TOUCH_AREA_CONTROL_CURSOR_R ] );
  area->rect.left   = menu->left_dot + menu->rightCursorOffsetX - CLWK_CONTROL_CURSOR_R_WIDTH/2;
  area->rect.top    = menu->top_dot  + menu->rightCursorOffsetY - CLWK_CONTROL_CURSOR_R_HEIGHT/2;
  area->rect.right  = area->rect.left + TOUCH_AREA_CONTROL_CURSOR_WIDTH;
  area->rect.bottom = area->rect.top  + TOUCH_AREA_CONTROL_CURSOR_HEIGHT;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: update touch area\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief MAIN-BG ( ウィンドウ面 ) を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateMainBG_WINDOW( RESEARCH_CHECK_WORK* work )
{
  ARCHANDLE* handle;
  ARCDATID datID;

  // ハンドルオープン
  handle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR_GRAPHIC, work->heapID ); 

  // 反映させるスクリーンデータを決定
  if( work->seq == RESEARCH_CHECK_SEQ_UPDATE ) {
    datID = NARC_research_radar_graphic_bgd_graphbtn3_NSCR; //「データ しゅとくちゅう」
  }
  else if( GetCountOfQuestion( work ) == 0 ) {
    datID = NARC_research_radar_graphic_bgd_graphbtn1_NSCR; //「ただいま ちょうさちゅう」
  }
  else if( work->analyzeFlag == FALSE ) {
    datID = NARC_research_radar_graphic_bgd_graphbtn1_NSCR; // 未解析
  }
  else {
    datID = NARC_research_radar_graphic_bgd_graphbtn2_NSCR; // 通常表示
  }

  // スクリーンデータ読み込み
  {
    void* src;
    NNSG2dScreenData* data;
    src = GFL_ARC_LoadDataAllocByHandle( handle, datID, work->heapID );
    NNS_G2dGetUnpackedScreenData( src, &data );
    GFL_BG_WriteScreen( MAIN_BG_WINDOW, data->rawData, 0, 0, 32, 32 );
    GFL_BG_LoadScreenReq( MAIN_BG_WINDOW );
    GFL_HEAP_FreeMemory( src );
  }

  // ハンドルクローズ
  GFL_ARC_CloseDataHandle( handle );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: setup MAIN-BG-WINDOW\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査項目の補足文を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateBGFont_TopicCaption( RESEARCH_CHECK_WORK* work )
{
  u32 topicID;
  u32 strID;

  // 調査項目の補足文の文字列IDを取得
  topicID = work->researchData.topicID;
  strID = StringID_topicCaption[ topicID ];

  // BG ( フォント面 ) に対し, 文字列を書き込む
  BG_FONT_SetMessage( work->BGFont[ SUB_BG_FONT_TOPIC_CAPTION ], strID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: update BGFont topic caption\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 質問の補足文を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateBGFont_QuestionCaption( RESEARCH_CHECK_WORK* work )
{
  u32 questionID;
  u32 strID;

  // 質問の補足文の文字列IDを取得
  questionID  = GetQuestionID( work );
  strID       = CaptionStringID_question[ questionID ];

  // BG ( フォント面 ) に対し, 文字列を書き込む
  BG_FONT_SetMessage( work->BGFont[ SUB_BG_FONT_QUESTION_CAPTION ], strID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: update BGFont question caption\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 質問を更新する 
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateBGFont_Question( RESEARCH_CHECK_WORK* work )
{
  u32 questionID;
  u32 strID;
  BG_FONT* BGFont;

  BGFont = work->BGFont[ MAIN_BG_FONT_QUESTION ];

  // 質問の文字列IDを取得
  questionID = GetQuestionID( work );
  strID      = StringID_question[ questionID ];

  // BG ( フォント面 ) に対し, 文字列を書き込む
  BG_FONT_SetMessage( BGFont, strID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: update BGFont question\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 回答を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateBGFont_Answer( RESEARCH_CHECK_WORK* work )
{
  u16 answerID;
  u16 answerIdx;
  u8 answerRank;
  u16 count;
  u8 percentage;
  BG_FONT* BGFont;
  STRBUF* strbuf_expand; // 展開後の文字列
  STRBUF* strbuf_plain;  // 展開前の文字列
  STRBUF* strbuf_answer; // 回答の文字列
  const CIRCLE_GRAPH* graph;

  BGFont = work->BGFont[ MAIN_BG_FONT_ANSWER ];
  graph  = GetMainGraph( work );

  // 解析前なら表示しない
  if( work->analyzeFlag == FALSE ) {
    BG_FONT_SetDrawEnable( BGFont, FALSE );
    return;
  }

  //「ただいま ちょうさちゅう」なら, 表示しない
  if( GetCountOfQuestion(work) == 0 ) {
    BG_FONT_SetDrawEnable( BGFont, FALSE );
    return;
  }

  // 回答の文字列ID・順番を取得
  answerID   = GetAnswerID( work );
  answerIdx  = work->answerIdx;
  answerRank = CIRCLE_GRAPH_GetComponentRank( graph, answerID );
  count      = GetCountOfAnswer( work );
  percentage = CIRCLE_GRAPH_GetComponentPercentage( graph, answerID );

  // 数値や回答を文字列に展開
  {
    strbuf_plain  = GFL_MSG_CreateString( work->message[ MESSAGE_STATIC ], str_check_answer );
    strbuf_answer = GFL_MSG_CreateString( work->message[ MESSAGE_QUESTIONNAIRE ], StringID_answer[ answerID ] );
    strbuf_expand = GFL_STR_CreateBuffer( 128, work->heapID );
    WORDSET_RegisterNumber( work->wordset, 0, answerRank, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT ); // 何番目か
    WORDSET_RegisterWord( work->wordset, 1, strbuf_answer, 0, TRUE, PM_LANG ); // 回答文字列
    WORDSET_RegisterNumber( work->wordset, 2, count, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT ); // 人数
    WORDSET_RegisterNumber( work->wordset, 3, percentage, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT ); // パーセンテージ
    WORDSET_ExpandStr( work->wordset, strbuf_expand, strbuf_plain );
  }

  // BG ( フォント面 ) に対し, 文字列を書き込む
  BG_FONT_SetString( BGFont, strbuf_expand );

  GFL_STR_DeleteBuffer( strbuf_plain );
  GFL_STR_DeleteBuffer( strbuf_expand );
  GFL_STR_DeleteBuffer( strbuf_answer );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: update BGFont answer\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 自分の回答を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateBGFont_MyAnswer( RESEARCH_CHECK_WORK* work )
{
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* questionnaireSave;
  QUESTIONNAIRE_ANSWER_WORK* myAnswerWork;
  u8 questionID;
  u16 answerID;
  STRBUF* strbuf_plain;  // 展開前の文字列
  STRBUF* strbuf_expand; // 展開後の文字列
  STRBUF* strbuf_myAnswer;  // 自分の回答文字列

  // 解析前なら表示しない
  if( work->analyzeFlag == FALSE ) {
    BG_FONT_SetDrawEnable( work->BGFont[ MAIN_BG_FONT_MY_ANSWER ], FALSE );
    return;
  }

  // セーブデータを取得
  save              = GAMEDATA_GetSaveControlWork( work->gameData );
  questionnaireSave = SaveData_GetQuestionnaire( save );
  myAnswerWork      = Questionnaire_GetAnswerWork( questionnaireSave );

  // 現在表示中の質問に対する, 自分の回答IDを取得
  questionID = GetQuestionID( work );
  answerID   = QuestionnaireAnswer_ReadBit( myAnswerWork, questionID );

  // 回答文字列を展開
  {
    strbuf_plain    = GFL_MSG_CreateString( work->message[ MESSAGE_STATIC ], str_check_my_answer );
    strbuf_myAnswer = GFL_MSG_CreateString( work->message[ MESSAGE_QUESTIONNAIRE ], StringID_answer[ answerID ] );
    strbuf_expand   = GFL_STR_CreateBuffer( 128, work->heapID );
    WORDSET_RegisterWord( work->wordset, 0, strbuf_myAnswer, 0, TRUE, PM_LANG );
    WORDSET_ExpandStr( work->wordset, strbuf_expand, strbuf_plain );
  }

  // BG ( フォント面 ) に対し, 文字列を書き込む
  BG_FONT_SetString( work->BGFont[ MAIN_BG_FONT_MY_ANSWER ], strbuf_expand );

  GFL_STR_DeleteBuffer( strbuf_plain );
  GFL_STR_DeleteBuffer( strbuf_expand );
  GFL_STR_DeleteBuffer( strbuf_myAnswer );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: update BGFont my answer\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 回答人数を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateBGFont_Count( RESEARCH_CHECK_WORK* work )
{
  u32 strID; 
  STRBUF* strbuf_plain;  // 展開前の文字列
  STRBUF* strbuf_expand; // 展開後の文字列

  // 解析前なら表示しない
  if( work->analyzeFlag == FALSE ) {
    BG_FONT_SetDrawEnable( work->BGFont[ MAIN_BG_FONT_COUNT ], FALSE );
    return;
  }

  // 文字列IDを決定
  switch( work->dispType )
  {
  case DATA_DISP_TYPE_TODAY: strID = str_check_today_count; break;
  case DATA_DISP_TYPE_TOTAL: strID = str_check_total_count; break;
  default: GF_ASSERT(0);
  }

  // 回答人数を展開
  {
    s32 count;
    WORDSET* wordset;
    GFL_MSGDATA* message;

    count   = GetCountOfQuestion( work );
    message = work->message[ MESSAGE_STATIC ];
    wordset = work->wordset;
    strbuf_plain  = GFL_MSG_CreateString( message, strID );
    strbuf_expand = GFL_STR_CreateBuffer( 128, work->heapID ); 
    WORDSET_RegisterNumber( wordset, 0, count, 4, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wordset, strbuf_expand, strbuf_plain );
  }

  // BG ( フォント面 ) に対し, 文字列を書き込む
  BG_FONT_SetString( work->BGFont[ MAIN_BG_FONT_COUNT ], strbuf_expand );

  GFL_STR_DeleteBuffer( strbuf_plain );
  GFL_STR_DeleteBuffer( strbuf_expand );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: update BGFont count\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief「ただいま ちょうさちゅう」の表示を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateBGFont_NoData( RESEARCH_CHECK_WORK* work )
{
  if( GetCountOfQuestion(work) == 0 ) { 
    BG_FONT_SetDrawEnable( work->BGFont[ MAIN_BG_FONT_NO_DATA ], TRUE ); // 表示
  }
  else { 
    BG_FONT_SetDrawEnable( work->BGFont[ MAIN_BG_FONT_NO_DATA ], FALSE ); // クリア
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: update BGFont no data\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @breif「データしゅとくちゅう」の表示を更新する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void UpdateBGFont_DataReceiving( RESEARCH_CHECK_WORK* work )
{
  if( work->seq == RESEARCH_CHECK_SEQ_UPDATE ) {
    BG_FONT_SetDrawEnable( work->BGFont[ MAIN_BG_FONT_DATA_RECEIVING ], TRUE ); // 表示  
  }
  else {
    BG_FONT_SetDrawEnable( work->BGFont[ MAIN_BG_FONT_DATA_RECEIVING ], FALSE ); // クリア
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: update BGFont data receiving\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 操作カーソルの表示を更新する
 *
 * @param work 
 */
//----------------------------------------------------------------------------------------------
static void UpdateControlCursor( RESEARCH_CHECK_WORK* work )
{
  const MENU_ITEM_DRAW_PARAM* menuParam;
  GFL_CLACTPOS pos;
  u16 setsf;
  GFL_CLWK* cursorL;
  GFL_CLWK* cursorR;

  // カーソル位置のメニューの表示パラメータを取得
  menuParam = &MenuItemDrawParam[ work->cursorPos ]; 

  // セルアクターワークを取得
  cursorL = GetClactWork( work, CLWK_CONTROL_CURSOR_L );
  cursorR = GetClactWork( work, CLWK_CONTROL_CURSOR_R );


  //「自分の回答」を選択している場合は表示しない
  if( work->cursorPos == MENU_ITEM_MY_ANSWER ) {
    GFL_CLACT_WK_SetDrawEnable( cursorL, FALSE );
    GFL_CLACT_WK_SetDrawEnable( cursorR, FALSE );
  }
  else { 
    // 表示座標を変更
    pos.x = menuParam->left_dot + menuParam->leftCursorOffsetX;
    pos.y = menuParam->top_dot + menuParam->leftCursorOffsetY;
    setsf = ClactWorkInitData[ CLWK_CONTROL_CURSOR_L ].setSurface;
    GFL_CLACT_WK_SetPos( cursorL, &pos, setsf );

    pos.x = menuParam->left_dot + menuParam->rightCursorOffsetX;
    pos.y = menuParam->top_dot + menuParam->rightCursorOffsetY;
    setsf = ClactWorkInitData[ CLWK_CONTROL_CURSOR_R ].setSurface;
    GFL_CLACT_WK_SetPos( cursorR, &pos, setsf );

    // 表示状態を設定
    GFL_CLACT_WK_SetDrawEnable( cursorL, TRUE );
    GFL_CLACT_WK_SetDrawEnable( cursorR, TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( cursorL, TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( cursorR, TRUE );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: update control cursor\n" );
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
static void BmpOamSetDrawEnable( RESEARCH_CHECK_WORK* work, BMPOAM_ACTOR_INDEX actorIdx, BOOL enable )
{
  // インデックスエラー
  GF_ASSERT( actorIdx < BMPOAM_ACTOR_NUM );

  // 表示状態を変更
  BmpOam_ActorSetDrawEnable( work->BmpOamActor[ actorIdx ], enable );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, 
              "RESEARCH-CHECK: set draw enable BMP-OAM [%d] ==> %d\n", actorIdx, enable );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitSeqQueue( RESEARCH_CHECK_WORK* work )
{
  // 初期化
  work->seqQueue = NULL;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init seq queue\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateSeqQueue( RESEARCH_CHECK_WORK* work )
{
  GF_ASSERT( work->seqQueue == NULL );

  // 作成
  work->seqQueue = QUEUE_Create( SEQ_QUEUE_SIZE, work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: create seq queue\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteSeqQueue( RESEARCH_CHECK_WORK* work )
{
  GF_ASSERT( work->seqQueue );

  // 削除
  QUEUE_Delete( work->seqQueue );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: delete seq queue\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitFont( RESEARCH_CHECK_WORK* work )
{
  // 初期化
  work->font = NULL;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init font\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateFont( RESEARCH_CHECK_WORK* work )
{
  GF_ASSERT( work->font == NULL );

  // 生成
  work->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, 
                                GFL_FONT_LOADTYPE_FILE, TRUE, work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: create font\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteFont( RESEARCH_CHECK_WORK* work )
{
  GF_ASSERT( work->font );

  // 削除
  GFL_FONT_Delete( work->font );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: delete font\n" );
}


//----------------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitMessages( RESEARCH_CHECK_WORK* work )
{
  int msgIdx;

  // 初期化
  for( msgIdx=0; msgIdx < MESSAGE_NUM; msgIdx++ )
  {
    work->message[ msgIdx ] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init messages\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateMessages( RESEARCH_CHECK_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: create messages\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteMessages( RESEARCH_CHECK_WORK* work )
{
  int msgIdx;

  for( msgIdx=0; msgIdx < MESSAGE_NUM; msgIdx++ )
  {
    GF_ASSERT( work->message[ msgIdx ] );

    // 削除
    GFL_MSG_Delete( work->message[ msgIdx ] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: delete messages\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief ワードセットを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitWordset( RESEARCH_CHECK_WORK* work )
{
  // 初期化
  work->wordset = NULL;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init wordset\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief ワードセットを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateWordset( RESEARCH_CHECK_WORK* work )
{
  // 多重生成
  GF_ASSERT( work->wordset == NULL );

  // 作成
  work->wordset = WORDSET_Create( work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: create wordset\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief ワードセットを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteWordset( RESEARCH_CHECK_WORK* work )
{
  // 削除
  WORDSET_Delete( work->wordset );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: delete wordset\n" );
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
static void SetupBG( RESEARCH_CHECK_WORK* work )
{ 
  // BG モード
  GFL_BG_SetBGMode( &BGSysHeader3D );

  // BG コントロール
  GFL_BG_SetBGControl3D( MAIN_BG_3D_PRIORITY );
  GFL_BG_SetBGControl( SUB_BG_WINDOW,  &SubBGControl_WINDOW,  GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( SUB_BG_FONT,    &SubBGControl_FONT,    GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( MAIN_BG_WINDOW, &MainBGControl_WINDOW, GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( MAIN_BG_FONT,   &MainBGControl_FONT,   GFL_BG_MODE_TEXT );

  // 可視設定
  GFL_BG_SetVisible( SUB_BG_BACK,    VISIBLE_ON );
  GFL_BG_SetVisible( SUB_BG_RADAR,   VISIBLE_ON );
  GFL_BG_SetVisible( SUB_BG_WINDOW,  VISIBLE_ON );
  GFL_BG_SetVisible( SUB_BG_FONT,    VISIBLE_ON );
  GFL_BG_SetVisible( MAIN_BG_3D,     VISIBLE_ON );
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: setup BG\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief BG の後片付け
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpBG( RESEARCH_CHECK_WORK* work )
{
  GFL_BMPWIN_Exit();

  GFL_BG_FreeBGControl( MAIN_BG_3D );
  GFL_BG_FreeBGControl( MAIN_BG_FONT );
  GFL_BG_FreeBGControl( MAIN_BG_WINDOW );
  GFL_BG_FreeBGControl( SUB_BG_FONT );
  GFL_BG_FreeBGControl( SUB_BG_WINDOW );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: clean up BG\n" );
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
static void SetupSubBG_WINDOW( RESEARCH_CHECK_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: setup SUB-BG-WINDOW\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 上画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpSubBG_WINDOW( RESEARCH_CHECK_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: clean up SUB-BG-WINDOW\n" );
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
static void SetupSubBG_FONT( RESEARCH_CHECK_WORK* work )
{
  // NULLキャラ確保
  GFL_BG_FillCharacter( SUB_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( SUB_BG_FONT );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: setup SUB-BG-FONT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief SUB-BG フォント面の後片付け
 * 
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpSubBG_FONT( RESEARCH_CHECK_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( SUB_BG_FONT, 1, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: clean up SUB-BG-FONT\n" );
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
static void SetupMainBG_WINDOW( RESEARCH_CHECK_WORK* work )
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
      datID = NARC_research_radar_graphic_bgd_graphbtn2_NSCR;
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: setup MAIN-BG-WINDOW\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpMainBG_WINDOW( RESEARCH_CHECK_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: clean up MAIN-BG-WINDOW\n" );
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
static void SetupMainBG_FONT( RESEARCH_CHECK_WORK* work )
{ 
  // NULLキャラ確保
  GFL_BG_FillCharacter( MAIN_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( MAIN_BG_FONT );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: setup MAIN-BG-FONT\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 下画面 フォントBG面 後片付け
 * 
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpMainBG_FONT( RESEARCH_CHECK_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( MAIN_BG_FONT, 1, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: clean up MAIN-BG-FONT\n" );
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
static void InitBGFonts( RESEARCH_CHECK_WORK* work )
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
static void CreateBGFonts( RESEARCH_CHECK_WORK* work )
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

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: create BGFonts\n" ); 
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteBGFonts( RESEARCH_CHECK_WORK* work )
{
  int i;
  
  // 通常のBGFont
  for( i=0; i < BG_FONT_NUM; i++ )
  {
    GF_ASSERT( work->BGFont[i] );
    BG_FONT_Delete( work->BGFont[i] );
    work->BGFont[i] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: delete BGFonts\n" ); 
} 

//----------------------------------------------------------------------------------------------
/**
 * @brief 円グラフを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitCircleGraphs( RESEARCH_CHECK_WORK* work )
{
  int idx;

  for( idx=0; idx < DATA_DISP_TYPE_NUM; idx++ )
  {
    work->mainGraph[ idx ] = NULL;
    work->subGraph[ idx ]  = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init circle graph\n" ); 
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 円グラフを生成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateCircleGraph( RESEARCH_CHECK_WORK* work )
{
  int idx;

  for( idx=0; idx < DATA_DISP_TYPE_NUM; idx++ )
  {
    // 多重生成
    GF_ASSERT( work->mainGraph[ idx ] == NULL );
    GF_ASSERT( work->subGraph[ idx ] == NULL );

    work->mainGraph[ idx ] = CIRCLE_GRAPH_Create( work->heapID );
    work->subGraph[ idx ]  = CIRCLE_GRAPH_Create( work->heapID );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: create circle graph\n" ); 
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 円グラフを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteCircleGraph( RESEARCH_CHECK_WORK* work )
{
  int idx;

  for( idx=0; idx < DATA_DISP_TYPE_NUM; idx++ )
  {
    // 未生成
    GF_ASSERT( work->mainGraph[ idx ] != NULL );
    GF_ASSERT( work->subGraph[ idx ] != NULL );

    CIRCLE_GRAPH_Delete( work->mainGraph[ idx ] );
    CIRCLE_GRAPH_Delete( work->subGraph[ idx ] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: delete circle graph\n" ); 
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査データを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitResearchData( RESEARCH_CHECK_WORK* work )
{
  // 0クリア
  GFL_STD_MemClear( &( work->researchData ), sizeof(RESEARCH_DATA) );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init research data\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査データを取得する
 *
 * @parma work
 */
//----------------------------------------------------------------------------------------------
static void SetupResearchData( RESEARCH_CHECK_WORK* work )
{
  int qIdx, aIdx;
  u8 topicID;
  u8 questionID[ QUESTION_NUM_PER_TOPIC ];
  u8 answerNum[ QUESTION_NUM_PER_TOPIC ];
  u16 todayCount_question[ QUESTION_NUM_PER_TOPIC ];
  u16 totalCount_question[ QUESTION_NUM_PER_TOPIC ];
  u16 answerID[ QUESTION_NUM_PER_TOPIC ][ MAX_ANSWER_NUM_PER_QUESTION ];
  u16 todayCount_answer[ QUESTION_NUM_PER_TOPIC ][ MAX_ANSWER_NUM_PER_QUESTION ];
  u16 totalCount_answer[ QUESTION_NUM_PER_TOPIC ][ MAX_ANSWER_NUM_PER_QUESTION ];
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* questionnaireSave;

  // セーブデータ取得
  save              = GAMEDATA_GetSaveControlWork( work->gameData );
  questionnaireSave = SaveData_GetQuestionnaire( save );
  topicID = QuestionnaireWork_GetInvestigatingQuestion( questionnaireSave, 0 ); // 調査項目ID
  questionID[0] = Question1_topic[ topicID ]; // 質問ID
  questionID[1] = Question2_topic[ topicID ]; // 質問ID
  questionID[2] = Question3_topic[ topicID ]; // 質問ID
  answerNum[0] = AnswerNum_question[ questionID[0] ]; // 回答選択肢の数
  answerNum[1] = AnswerNum_question[ questionID[1] ]; // 回答選択肢の数
  answerNum[2] = AnswerNum_question[ questionID[2] ]; // 回答選択肢の数
  todayCount_question[0] = QuestionnaireWork_GetTodayCount( questionnaireSave, questionID[0] ); // 今日の回答人数
  todayCount_question[1] = QuestionnaireWork_GetTodayCount( questionnaireSave, questionID[1] ); // 今日の回答人数
  todayCount_question[2] = QuestionnaireWork_GetTodayCount( questionnaireSave, questionID[2] ); // 今日の回答人数
  totalCount_question[0] = QuestionnaireWork_GetTotalCount( questionnaireSave, questionID[0] ); // いままでの回答人数
  totalCount_question[1] = QuestionnaireWork_GetTotalCount( questionnaireSave, questionID[1] ); // いままでの回答人数
  totalCount_question[2] = QuestionnaireWork_GetTotalCount( questionnaireSave, questionID[2] ); // いままでの回答人数
  for( qIdx=0; qIdx < QUESTION_NUM_PER_TOPIC; qIdx++ )
  {
    u8 qID = questionID[ qIdx ];
    for( aIdx=0; aIdx < answerNum[ qIdx ]; aIdx++ )
    {
      answerID[ qIdx ][ aIdx ] = AnswerID_question[ qID ][ aIdx ]; // 回答ID
      todayCount_answer[ qIdx ][ aIdx ] = QuestionnaireWork_GetTodayAnswerNum( questionnaireSave, qIdx, aIdx ); // 今日の回答人数
      totalCount_answer[ qIdx ][ aIdx ] = QuestionnaireWork_GetTotalAnswerNum( questionnaireSave, qIdx, aIdx ); // いままでの回答人数
    }
  }

  // データ設定
  work->researchData.topicID = topicID; // 調査項目ID
  for( qIdx=0; qIdx < QUESTION_NUM_PER_TOPIC; qIdx++ )
  {
    work->researchData.questionData[ qIdx ].ID         = questionID[ qIdx ];           // 質問ID
    work->researchData.questionData[ qIdx ].answerNum  = answerNum[ qIdx ];            // 回答選択肢の数
    work->researchData.questionData[ qIdx ].todayCount = todayCount_question[ qIdx ];  // 今日の回答人数
    work->researchData.questionData[ qIdx ].totalCount = totalCount_question[ qIdx ];  // いままでの回答人数

    for( aIdx=0; aIdx < MAX_ANSWER_NUM_PER_QUESTION; aIdx++ )
    {
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].ID = answerID[ qIdx ][ aIdx ]; // 回答ID
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].colorR = (aIdx * 3) % 31; // 表示カラー(R)
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].colorG = (aIdx * 5) % 31; // 表示カラー(G)
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].colorB = (aIdx * 7) % 31; // 表示カラー(B)
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].todayCount = todayCount_answer[ qIdx ][ aIdx ]; // 今日の回答人数
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].totalCount = totalCount_answer[ qIdx ][ aIdx ];  // いままでの回答人数
    }
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @breif タッチ領域の準備を行う
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupTouchArea( RESEARCH_CHECK_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: create touch hit table\n" );
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
static void CreateClactSystem( RESEARCH_CHECK_WORK* work )
{
  // システム作成
  //GFL_CLACT_SYS_Create( &ClactSystemInitData, &VRAMBankSettings, work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: create clact system\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターシステムを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteClactSystem( RESEARCH_CHECK_WORK* work )
{ 
  // システム破棄
  //GFL_CLACT_SYS_Delete();

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: delete clact system\n" );
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
static void RegisterSubObjResources( RESEARCH_CHECK_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: register SUB-OBJ resources\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief SUB-OBJ リソースを解放する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ReleaseSubObjResources( RESEARCH_CHECK_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CELL_ANIME ] );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: release SUB-OBJ resources\n" );
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
static void RegisterMainObjResources( RESEARCH_CHECK_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: register MAIN-OBJ resources\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief MAIN-OBJ リソースを解放する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ReleaseMainObjResources( RESEARCH_CHECK_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CELL_ANIME ] );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: release MAIN-OBJ resources\n" );
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
static void InitClactUnits( RESEARCH_CHECK_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < CLUNIT_NUM; unitIdx++ )
  {
    work->clactUnit[ unitIdx ] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init clact units\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを作成する
 *
 * @param
 */
//----------------------------------------------------------------------------------------------
static void CreateClactUnits( RESEARCH_CHECK_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: create clact units\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを破棄する
 *
 * @param
 */
//----------------------------------------------------------------------------------------------
static void DeleteClactUnits( RESEARCH_CHECK_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < CLUNIT_NUM; unitIdx++ )
  {
    GF_ASSERT( work->clactUnit[ unitIdx ] );
    GFL_CLACT_UNIT_Delete( work->clactUnit[ unitIdx ] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: delete clact units\n" );
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
static void InitClactWorks( RESEARCH_CHECK_WORK* work )
{
  int wkIdx;

  // 初期化
  for( wkIdx=0; wkIdx < CLWK_NUM; wkIdx++ )
  {
    work->clactWork[ wkIdx ] = NULL;
  }

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init clact works\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを作成する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CreateClactWorks( RESEARCH_CHECK_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: create clact works\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを破棄する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DeleteClactWorks( RESEARCH_CHECK_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: delete clact works\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief ビットマップデータを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitBitmapDatas( RESEARCH_CHECK_WORK* work )
{
  int idx;

  // 初期化
  for( idx=0; idx < BMPOAM_ACTOR_NUM; idx++ )
  {
    work->BmpData[ idx ] = NULL;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init bitmap datas\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief ビットマップデータを作成する
 *
 * @parma work
 */
//----------------------------------------------------------------------------------------------
static void CreateBitmapDatas( RESEARCH_CHECK_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: create bitmap datas\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief ビットマップデータの準備を行う
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupBitmapDatas( RESEARCH_CHECK_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: setup bitmap datas\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief ビットマップデータを破棄する
 *
 * @parma work
 */
//----------------------------------------------------------------------------------------------
static void DeleteBitmapDatas( RESEARCH_CHECK_WORK* work )
{
  int idx;

  // 破棄
  for( idx=0; idx < BMPOAM_ACTOR_NUM; idx++ )
  {
    GFL_BMP_Delete( work->BmpData[ idx ] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: delete bitmap datas\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM システムの準備を行う
 *
 * @parma work
 */
//----------------------------------------------------------------------------------------------
static void SetupBmpOamSystem( RESEARCH_CHECK_WORK* work )
{
  // BMP-OAM システムを作成
  work->BmpOamSystem = BmpOam_Init( work->heapID, work->clactUnit[ CLUNIT_BMPOAM ] );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: setup BMP-OAM system\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM システムの後片付けを行う
 *
 * @parma work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpBmpOamSystem( RESEARCH_CHECK_WORK* work )
{
  // BMP-OAM システムを破棄
  BmpOam_Exit( work->BmpOamSystem );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: clean up BMP-OAM system\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM アクターを作成する
 *
 * @parma work
 */
//----------------------------------------------------------------------------------------------
static void CreateBmpOamActors( RESEARCH_CHECK_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: create BMP-OAM actors\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM アクターを破棄する
 *
 * @parma work
 */
//----------------------------------------------------------------------------------------------
static void DeleteBmpOamActors( RESEARCH_CHECK_WORK* work )
{
  int idx;

  for( idx=0; idx < BMPOAM_ACTOR_NUM; idx++ )
  {
    BmpOam_ActorDel( work->BmpOamActor[ idx ] );
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: delete BMP-OAM actors\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 3D 初期設定
 */
//-------------------------------------------------------------------------------
static void Setup3D()
{
  G3X_Init();
  G3X_InitMtxStack();

  G3_MtxMode( GX_MTXMODE_PROJECTION );
  G3_Identity();
  G3_MtxMode( GX_MTXMODE_POSITION_VECTOR );
  G3_Identity(); 

  G3X_AntiAlias( TRUE );   // アンチエイリアス
  G3X_EdgeMarking( TRUE ); // エッジマーキング
  G3X_SetEdgeColorTable( EdgeColor );  // エッジカラー
}


//----------------------------------------------------------------------------------------------
/**
 * @brief VBlank期間中のタスクを登録する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void RegisterVBlankTask( RESEARCH_CHECK_WORK* work )
{
  work->VBlankTask = GFUser_VIntr_CreateTCB( VBlankFunc, work, 0 );

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: register VBlank task\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief VBlank期間中のタスクを解除する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void ReleaseVBlankTask( RESEARCH_CHECK_WORK* work )
{ 
  GFL_TCB_DeleteTask( work->VBlankTask );

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: release VBlank task\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムを初期化する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InitPaletteFadeSystem( RESEARCH_CHECK_WORK* work )
{ 
  // 初期化
  work->paletteFadeSystem = NULL;

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: init palette fade system\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムを準備する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void SetupPaletteFadeSystem( RESEARCH_CHECK_WORK* work )
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
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: setup palette fade system\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムの後片付けを行う
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void CleanUpPaletteFadeSystem( RESEARCH_CHECK_WORK* work )
{ 
  // パレットフェードのリクエストワークを破棄
  PaletteFadeWorkAllocFree( work->paletteFadeSystem, FADE_MAIN_BG );
  PaletteFadeWorkAllocFree( work->paletteFadeSystem, FADE_MAIN_OBJ );

  // フェード管理システムを破棄
  PaletteFadeFree( work->paletteFadeSystem );

  // DEBUG;
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: clean up palette fade system\n" );
}


//----------------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の調査項目IDを取得する
 *
 * @param work
 *
 * @return 現在表示中の調査項目ID
 */
//----------------------------------------------------------------------------------------------
static u8 GetTopicID( const RESEARCH_CHECK_WORK* work )
{
  return work->researchData.topicID;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の質問IDを取得する
 *
 * @param work
 *
 * @return 現在表示中の質問ID
 */
//----------------------------------------------------------------------------------------------
static u8 GetQuestionID( const RESEARCH_CHECK_WORK* work )
{
  u8 qIdx;
  qIdx = work->questionIdx;
  return work->researchData.questionData[ qIdx ].ID;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の質問に対する回答選択肢の数 を取得する
 *
 * @param work
 *
 * @return 現在表示中の質問に対する, 回答の選択肢の数
 */
//---------------------------------------------------------------------------------------------- 
static u8 GetAnswerNum( const RESEARCH_CHECK_WORK* work )
{
  u8 questionIdx;

  questionIdx = work->questionIdx;

  return work->researchData.questionData[ questionIdx ].answerNum;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の回答IDを取得する
 *
 * @param work
 *
 * @return 現在表示中の回答ID
 */
//----------------------------------------------------------------------------------------------
static u16 GetAnswerID( const RESEARCH_CHECK_WORK* work )
{
  u8 qIdx;
  u8 aIdx;

  qIdx = work->questionIdx;
  aIdx = work->answerIdx;

  return work->researchData.questionData[ qIdx ].answerData[ aIdx ].ID;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の質問に対する, 表示の回答人数 を取得する
 *
 * @param work
 *
 * @return 現在表示中の質問に対する, 表示の回答人数
 */
//----------------------------------------------------------------------------------------------
static u16 GetCountOfQuestion( const RESEARCH_CHECK_WORK* work )
{
  switch( work->dispType )
  {
  case DATA_DISP_TYPE_TODAY: return GetTodayCountOfQuestion( work ); break;
  case DATA_DISP_TYPE_TOTAL: return GetTotalCountOfQuestion( work ); break;
  default: GF_ASSERT(0);
  }

  // エラー
  GF_ASSERT(0);
  return 0;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の質問に対する, 今日の回答人数を取得する
 *
 * @param work
 *
 * @return 現在表示中の質問に対する, 今日の回答人数
 */
//----------------------------------------------------------------------------------------------
static u16 GetTodayCountOfQuestion( const RESEARCH_CHECK_WORK* work )
{
  u8 qIdx;

  qIdx = work->questionIdx;

  return work->researchData.questionData[ qIdx ].todayCount;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の質問に対する, 合計の回答人数
 *
 * @param work
 *
 * @return 現在表示中の質問に対する, 合計の回答人数
 */
//----------------------------------------------------------------------------------------------
static u16 GetTotalCountOfQuestion( const RESEARCH_CHECK_WORK* work )
{
  u8 qIdx;

  qIdx = work->questionIdx;

  return work->researchData.questionData[ qIdx ].totalCount;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の回答に対する, 表示の回答人数 を取得する
 *
 * @param work
 *
 * @return 現在表示中の回答に対する, 表示の回答人数
 */
//----------------------------------------------------------------------------------------------
static u16 GetCountOfAnswer( const RESEARCH_CHECK_WORK* work )
{
  switch( work->dispType )
  {
  case DATA_DISP_TYPE_TODAY: return GetTodayCountOfAnswer( work ); break;
  case DATA_DISP_TYPE_TOTAL: return GetTotalCountOfAnswer( work ); break;
  default: GF_ASSERT(0);
  }

  // エラー
  GF_ASSERT(0);
  return 0;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の回答に対する, 今日の回答人数
 *
 * @param work
 *
 * @return 現在表示中の回答に対する, 今日の回答人数
 */
//----------------------------------------------------------------------------------------------
static u16 GetTodayCountOfAnswer( const RESEARCH_CHECK_WORK* work )
{
  u8 qIdx;
  u8 aIdx;

  qIdx = work->questionIdx;
  aIdx = work->answerIdx;

  return work->researchData.questionData[ qIdx ].answerData[ aIdx ].todayCount;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の回答に対する, 合計の回答人数
 *
 * @param work
 *
 * @return 現在表示中の回答に対する, 合計の回答人数
 */
//----------------------------------------------------------------------------------------------
static u16 GetTotalCountOfAnswer( const RESEARCH_CHECK_WORK* work )
{
  u8 qIdx;
  u8 aIdx;

  qIdx = work->questionIdx;
  aIdx = work->answerIdx;

  return work->researchData.questionData[ qIdx ].answerData[ aIdx ].totalCount;
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在表示中のメイン円グラフ を取得する
 *
 * @param work
 *
 * @return 現在表示中のメイン円グラフ
 */
//----------------------------------------------------------------------------------------------
static CIRCLE_GRAPH* GetMainGraph( const RESEARCH_CHECK_WORK* work )
{
  return work->mainGraph[ work->dispType ];
}

//----------------------------------------------------------------------------------------------
/**
 * @brief 現在表示中のサブ円グラフ
 *
 * @param work
 *
 * @return 現在表示中のサブ円グラフ
 */
//----------------------------------------------------------------------------------------------
static CIRCLE_GRAPH* GetSubGraph ( const RESEARCH_CHECK_WORK* work )
{
  return work->subGraph[ work->dispType ];
}

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
static u32 GetObjResourceRegisterIndex( const RESEARCH_CHECK_WORK* work, OBJ_RESOURCE_ID resID )
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
static GFL_CLUNIT* GetClactUnit( const RESEARCH_CHECK_WORK* work, CLUNIT_INDEX unitIdx )
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
static GFL_CLWK* GetClactWork( const RESEARCH_CHECK_WORK* work, CLWK_INDEX wkIdx )
{
  return work->clactWork[ wkIdx ];
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットのフェードアウトを開始する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void StartPaletteFadeOut( RESEARCH_CHECK_WORK* work )
{
#if 0
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
#endif 
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: start palette fade out\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief パレットのフェードインを開始する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void StartPaletteFadeIn( RESEARCH_CHECK_WORK* work )
{
#if 0
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
#endif

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: start palette fade in\n" );
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
static BOOL IsPaletteFadeEnd( RESEARCH_CHECK_WORK* work )
{
  return PaletteFadeCheck( work->paletteFadeSystem );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief サブ円グラフとメイン円グラフを入れ替える
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void InterchangeCircleGraph( RESEARCH_CHECK_WORK* work )
{
  int typeIdx;
  CIRCLE_GRAPH* temp;

  for( typeIdx=0; typeIdx < DATA_DISP_TYPE_NUM; typeIdx++ )
  {
    CIRCLE_GRAPH* temp         = work->mainGraph[ typeIdx ];
    work->mainGraph[ typeIdx ] = work->subGraph[ typeIdx ];
    work->subGraph[ typeIdx ]  = temp;
  }

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: interchange circle graph\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief メイン円グラフを現在のデータで構成する
 *
 * @param work
 * @param dispType 表示タイプ
 */
//----------------------------------------------------------------------------------------------
static void SetupMainCircleGraph( RESEARCH_CHECK_WORK* work, DATA_DISP_TYPE dispType )
{
  int aIdx;
  CIRCLE_GRAPH* graph;
  GRAPH_COMPONENT_ADD_DATA components[ MAX_ANSWER_NUM_PER_QUESTION ];
  u8 answerNum;
  const QUESTION_DATA* questionData;
  const ANSWER_DATA* answerData;

  graph        = work->mainGraph[ dispType ];
  answerNum    = GetAnswerNum( work );
  questionData = &( work->researchData.questionData[ work->questionIdx ] );

  // グラフの構成要素データを作成
  for( aIdx=0; aIdx < answerNum; aIdx++ )
  {
    answerData = &( questionData->answerData[ aIdx ] );
    components[ aIdx ].ID = answerData->ID;
    components[ aIdx ].outerColorR = answerData->colorR;
    components[ aIdx ].outerColorG = answerData->colorG;
    components[ aIdx ].outerColorB = answerData->colorB;
    components[ aIdx ].centerColorR = answerData->colorR;
    components[ aIdx ].centerColorG = answerData->colorG;
    components[ aIdx ].centerColorB = answerData->colorB;
    switch( dispType )
    {
    case DATA_DISP_TYPE_TODAY: components[ aIdx ].value = answerData->todayCount; break;
    case DATA_DISP_TYPE_TOTAL: components[ aIdx ].value = answerData->totalCount; break;
    default: GF_ASSERT(0);
    }
  }

  // グラフの構成要素をセット
  CIRCLE_GRAPH_SetupComponents( graph, components, answerNum );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: setup main circle graph\n" );
}

//----------------------------------------------------------------------------------------------
/**
 * @brief サブ円グラフを現在のデータで構成する
 *
 * @param work
 * @param dispType 表示タイプ
 */
//----------------------------------------------------------------------------------------------
static void SetupSubCircleGraph( RESEARCH_CHECK_WORK* work, DATA_DISP_TYPE dispType )
{
  int aIdx;
  CIRCLE_GRAPH* graph;
  GRAPH_COMPONENT_ADD_DATA components[ MAX_ANSWER_NUM_PER_QUESTION ];
  u8 answerNum;
  const QUESTION_DATA* questionData;
  const ANSWER_DATA* answerData;

  graph        = work->subGraph[ dispType ];
  answerNum    = GetAnswerNum( work );
  questionData = &( work->researchData.questionData[ work->questionIdx ] );

  // グラフの構成要素データを作成
  for( aIdx=0; aIdx < answerNum; aIdx++ )
  {
    answerData = &( questionData->answerData[ aIdx ] );
    components[ aIdx ].ID = answerData->ID;
    components[ aIdx ].outerColorR = answerData->colorR;
    components[ aIdx ].outerColorG = answerData->colorG;
    components[ aIdx ].outerColorB = answerData->colorB;
    components[ aIdx ].centerColorR = 31;
    components[ aIdx ].centerColorG = 31;
    components[ aIdx ].centerColorB = 31;
    switch( dispType )
    {
    case DATA_DISP_TYPE_TODAY: components[ aIdx ].value = answerData->todayCount; break;
    case DATA_DISP_TYPE_TOTAL: components[ aIdx ].value = answerData->totalCount; break;
    default: GF_ASSERT(0);
    }
  }

  // グラフの構成要素をセット
  CIRCLE_GRAPH_SetupComponents( graph, components, answerNum );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: setup sub circle graph\n" );
} 

//----------------------------------------------------------------------------------------------
/**
 * @brief シーケンスキューの中身を表示する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DebugPrint_seqQueue( const RESEARCH_CHECK_WORK* work )
{
  int i;
  int dataNum;
  int value;

  // キュー内のデータの個数を取得
  dataNum = QUEUE_GetDataNum( work->seqQueue );

  // 全てのデータを出力
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: seq queue = " );
  for( i=0; i < dataNum; i++ )
  { 
    value = QUEUE_PeekData( work->seqQueue, i );
    
    switch( value )
    {
    case RESEARCH_CHECK_SEQ_SETUP:     OS_TFPrintf( PRINT_TARGET, "SETUP " );    break;
    case RESEARCH_CHECK_SEQ_KEY_WAIT:  OS_TFPrintf( PRINT_TARGET, "KEY-WAIT " ); break;
    case RESEARCH_CHECK_SEQ_ANALYZE:   OS_TFPrintf( PRINT_TARGET, "ANALYZE " );  break;
    case RESEARCH_CHECK_SEQ_FLASH:     OS_TFPrintf( PRINT_TARGET, "FLASH " );    break;
    case RESEARCH_CHECK_SEQ_UPDATE:    OS_TFPrintf( PRINT_TARGET, "UPDATE " );   break;
    case RESEARCH_CHECK_SEQ_FADE_OUT:  OS_TFPrintf( PRINT_TARGET, "FADE_OUT " );   break;
    case RESEARCH_CHECK_SEQ_CLEAN_UP:  OS_TFPrintf( PRINT_TARGET, "CLEAN-UP " ); break;
    case RESEARCH_CHECK_SEQ_FINISH:    OS_TFPrintf( PRINT_TARGET, "FINISH " );   break;
    default: GF_ASSERT(0);
    }
  }
  OS_TFPrintf( PRINT_TARGET, "\n" );
} 

//----------------------------------------------------------------------------------------------
/**
 * @brief 調査データを表示する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void DebugPrint_researchData( const RESEARCH_CHECK_WORK* work )
{
  const RESEARCH_DATA* research = &( work->researchData );
  int qIdx, aIdx;

  // 調査項目ID
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: topicID = %d\n", research->topicID );

  // 質問データ
  for( qIdx=0; qIdx < QUESTION_NUM_PER_TOPIC; qIdx++ )
  {
    const QUESTION_DATA* question = &( research->questionData[ qIdx ] );
    OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: questionID = %d\n", question->ID );
    OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: answerNum = %d\n",  question->answerNum );
    OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: todayCount = %d\n", question->todayCount );
    OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: totalCount = %d\n", question->totalCount );

    // 回答データ
    for( aIdx=0; aIdx < MAX_ANSWER_NUM_PER_QUESTION; aIdx++ )
    {
      const ANSWER_DATA* answer = &( question->answerData[ aIdx ] );
      OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: answerID = %d\n", answer->ID );
      OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: colorR = %d\n", answer->colorR );
      OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: colorG = %d\n", answer->colorG );
      OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: colorB = %d\n", answer->colorB );
      OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: todayCount = %d\n", answer->todayCount );
      OS_TFPrintf( PRINT_TARGET, "RESEARCH-CHECK: totalCount = %d\n", answer->totalCount );
    }
  }
}

//----------------------------------------------------------------------------------------------
/**
 * @breif 調査データを設定する
 *
 * @param work
 */
//----------------------------------------------------------------------------------------------
static void Debug_SetupResearchData( RESEARCH_CHECK_WORK* work )
{
  int qIdx, aIdx;
  u8 topicID;
  u8 questionID[ QUESTION_NUM_PER_TOPIC ];
  u16 answerID[ QUESTION_NUM_PER_TOPIC ][ MAX_ANSWER_NUM_PER_QUESTION ];
  u8 answerNum[ QUESTION_NUM_PER_TOPIC ];
  u16 todayCount_question[ QUESTION_NUM_PER_TOPIC ] = { 0, 200, 300 };
  u16 totalCount_question[ QUESTION_NUM_PER_TOPIC ] = { 300, 500, 900 };
  u16 todayCount_answer[ QUESTION_NUM_PER_TOPIC ][ MAX_ANSWER_NUM_PER_QUESTION ] = 
  {
    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 },
    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 },
    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 },
  };
  u16 totalCount_answer[ QUESTION_NUM_PER_TOPIC ][ MAX_ANSWER_NUM_PER_QUESTION ] = 
  {
    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 },
    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 },
    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 },
  };

  topicID = TOPIC_ID_STYLE; // 調査項目ID
  questionID[0] = Question1_topic[ topicID ]; // 質問ID
  questionID[1] = Question2_topic[ topicID ]; // 質問ID
  questionID[2] = Question3_topic[ topicID ]; // 質問ID
  answerNum[0] = AnswerNum_question[ questionID[0] ]; // 回答選択肢の数
  answerNum[1] = AnswerNum_question[ questionID[1] ]; // 回答選択肢の数
  answerNum[2] = AnswerNum_question[ questionID[2] ]; // 回答選択肢の数
  for( qIdx=0; qIdx < QUESTION_NUM_PER_TOPIC; qIdx++ )
  {
    u8 qID = questionID[ qIdx ];
    todayCount_question[ qIdx ] = GFUser_GetPublicRand( 10 );
    totalCount_question[ qIdx ] = GFUser_GetPublicRand( 9999 );
    for( aIdx=0; aIdx < answerNum[ qIdx ]; aIdx++ )
    {
      answerID[ qIdx ][ aIdx ] = AnswerID_question[ qID ][ aIdx ]; // 回答ID
      todayCount_answer[ qIdx ][ aIdx ] = GFUser_GetPublicRand( 10 ); 
      totalCount_answer[ qIdx ][ aIdx ] = GFUser_GetPublicRand( 500 ); 
    }
  }

  // データ設定
  work->researchData.topicID = topicID; // 調査項目ID
  for( qIdx=0; qIdx < QUESTION_NUM_PER_TOPIC; qIdx++ )
  {
    work->researchData.questionData[ qIdx ].ID         = questionID[ qIdx ];           // 質問ID
    work->researchData.questionData[ qIdx ].answerNum  = answerNum[ qIdx ];            // 回答選択肢の数
    work->researchData.questionData[ qIdx ].todayCount = todayCount_question[ qIdx ];  // 今日の回答人数
    work->researchData.questionData[ qIdx ].totalCount = totalCount_question[ qIdx ];  // いままでの回答人数

    for( aIdx=0; aIdx < MAX_ANSWER_NUM_PER_QUESTION; aIdx++ )
    {
      u16 aID = answerID[ qIdx ][ aIdx ];
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].ID = aID; // 回答ID
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].colorR = ColorR_answer[ aID ]; // 表示カラー(R)
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].colorG = ColorG_answer[ aID ]; // 表示カラー(G)
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].colorB = ColorB_answer[ aID ]; // 表示カラー(B)
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].todayCount = todayCount_answer[ qIdx ][ aIdx ]; // 今日の回答人数
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].totalCount = totalCount_answer[ qIdx ][ aIdx ];  // いままでの回答人数
    }
  }
}
