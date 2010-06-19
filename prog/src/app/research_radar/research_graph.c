///////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 調査報告確認画面
 * @file   research_graph.c
 * @author obata
 * @date   2010.02.20
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "queue.h"
#include "circle_graph.h"
#include "arrow.h"
#include "percentage.h"
#include "bg_font.h"
#include "palette_anime.h"
#include "research_data.h"
#include "research_graph.h"
#include "research_graph_index.h"
#include "research_graph_def.h"
#include "research_graph_data.cdat"
#include "research_common.h"
#include "research_common_data.cdat"

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

#include "answer_id_question.cdat"         // for AnswerID_question[][]
#include "answer_num_question.cdat"        // for AnswerNum_question[]
#include "question_id_topic.cdat"          // for QuestionX_topic[]
#include "string_id_question.cdat"         // for StringID_question[]
#include "string_id_topic_title.cdat"      // for StringID_topicTitle[]
#include "string_id_topic_caption.cdat"    // for StringID_topicCaption[]
#include "caption_string_id_question.cdat" // for CaptionStringID_question[]
#include "color_r_answer.cdat"             // for ColorR_answer[]
#include "color_g_answer.cdat"             // for ColorG_answer[]
#include "color_b_answer.cdat"             // for ColorB_answer[]



//=========================================================================================
// ■調査報告確認画面 ワーク
//=========================================================================================
struct _RESEARCH_RADAR_GRAPH_WORK
{ 
  RRC_WORK* commonWork; // 全画面共通ワーク
  HEAPID heapID;

  GFL_FONT*    font;                   // フォント
  GFL_MSGDATA* message[ MESSAGE_NUM ]; // メッセージ
  WORDSET*     wordset;                // ワードセット

  // 状態
  QUEUE*    stateQueue;  // 状態キュー
  RRG_STATE state;       // 現在の状態
  u32       stateSeq;    // 状態内シーケンス番号
  u32       stateCount;  // 状態カウンタ
  u32       waitFrame;   // WAIT 状態の待ち時間
  u32       updateCount; // 最後の更新からの経過フレーム数

  // カーソル
  MENU_ITEM cursorPos; // カーソル位置

  // 調査データ
  RESEARCH_DATA researchData;
  u8            questionIdx;  // 表示中の質問インデックス
  u8            answerIdx;    // 選択中の回答インデックス

  // 円グラフ
  CIRCLE_GRAPH*  mainGraph[ GRAPH_DISP_MODE_NUM ]; // 通常時に表示するグラフ
  CIRCLE_GRAPH*  subGraph [ GRAPH_DISP_MODE_NUM ]; // 更新時に表示するグラフ
  GRAPH_DISP_MODE graphMode;      // 今日 or 合計 のどちらのデータを表示するのか

  // 矢印
  ARROW* arrow;

  // ％表示オブジェクト
  PERCENTAGE* percentage[ PERCENTAGE_NUM ];
  u8          percentageNum;     // 有効なオブジェクトの数
  u8          percentageDispNum; // 表示中のオブジェクトの数p

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

  // BMP-OAM
  BMPOAM_SYS_PTR BmpOamSystem;                     // BMP-OAM システム
  BMPOAM_ACT_PTR BmpOamActor[ BMPOAM_ACTOR_NUM ];  // BMP-OAMアクター
  GFL_BMP_DATA*  BmpData[ BMPOAM_ACTOR_NUM ];      // 各アクターに対応するビットマップデータ

  // カラーパレット
  PALETTE_FADE_PTR paletteFadeSystem; // パレットフェード処理システム

  // VBlank
  GFL_TCBSYS* VBlankTCBSystem; // VBlank期間中のタスク管理システム
  GFL_TCB*    VBlankTask;      // VBlankタイミング中に行うタスク

  // フラグ
  BOOL setupFlag;          // セットアップが完了しているかどうか
  BOOL analyzeFlag;        // 解析が済んでいるかどうか
  BOOL analyzeByTouchFlag; // タッチによる解析かどうか
  BOOL updateFlag;         // 更新中かどうか
  BOOL updateEnableFlag;   // 更新が可能かどうか
  BOOL newEntryFlag;       // 新しい人物とすれ違ったかどうか
  BOOL stateEndFlag;       // 現在の状態が終了したかどうか
  BOOL finishFlag;         // グラフ画面の終了フラグ
  RRG_RESULT finishResult; // グラフ画面の終了結果
};



//=========================================================================================
// ◆関数インデックス
//=========================================================================================

//-----------------------------------------------------------------------------------------
// ◇LAYER 6 状態
//-----------------------------------------------------------------------------------------
// 状態処理
static void MainSeq_SETUP( RRG_WORK* work ); // RRG_STATE_SETUP
static void MainSeq_INTRUDE_SHUTDOWN( RRG_WORK* work ); // RRG_STATE_INTRUDE_SHUTDOWN
static void MainSeq_STANDBY( RRG_WORK* work ); // RRG_STATE_STANDBY
static void MainSeq_KEYWAIT( RRG_WORK* work ); // RRG_STATE_KEYWAIT
static void MainSeq_ANALYZE( RRG_WORK* work ); // RRG_STATE_ANALYZE
static void MainSeq_PERCENTAGE( RRG_WORK* work ); // RRG_STATE_PERCENTAGE
static void MainSeq_FLASHOUT( RRG_WORK* work ); // RRG_STATE_FLASHOUT
static void MainSeq_FLASHIN( RRG_WORK* work ); // RRG_STATE_FLASHIN
static void MainSeq_UPDATE_AT_STANDBY( RRG_WORK* work ); // RRG_STATE_UPDATE_AT_STANDBY
static void MainSeq_UPDATE_AT_KEYWAIT( RRG_WORK* work ); // RRG_STATE_UPDATE_AT_KEYWAIT
static void MainSeq_FADEOUT( RRG_WORK* work ); // RRG_STATE_FADEOUT
static void MainSeq_WAIT( RRG_WORK* work ); // RRG_STATE_WAIT
static void MainSeq_CLEANUP( RRG_WORK* work ); // RRG_STATE_CLEANUP 
// 状態制御
static void CountUpStateCount( RRG_WORK* work ); // 状態カウンタを更新する
static void RegisterNextState( RRG_WORK* work, RRG_STATE nextSeq ); // 次の状態をキューに登録する
static void FinishCurrentState( RRG_WORK* work ); // 現在の状態を終了する
static void SwitchState( RRG_WORK* work ); // 処理状態を変更する
static void SetState( RRG_WORK* work, RRG_STATE nextSeq ); // 処理状態を設定する
static u32 GetStateSeq( const RRG_WORK* work ); // 状態内シーケンス番号を取得する
static void IncStateSeq( RRG_WORK* work ); // 状態内シーケンス番号をインクリメントする
static void ResetStateSeq( RRG_WORK* work ); // 状態内シーケンス番号をリセットする
static void SetFinishReason( RRG_WORK* work, SEQ_CHANGE_TRIG reason ); // グラフ画面の終了理由を登録する
static void SetFinishResult( RRG_WORK* work, RRG_RESULT result ); // 画面終了結果を設定する
static void SetWaitFrame( RRG_WORK* work, u32 frame ); // フレーム経過待ち状態の待ち時間を設定する
static u32 GetWaitFrame( const RRG_WORK* work ); // フレーム経過待ち状態の待ち時間を取得する
RRG_STATE GetFirstState( const RRG_WORK* work ); // 最初の状態を取得する    
//-----------------------------------------------------------------------------------------
// ◇LAYER 5 機能
//-----------------------------------------------------------------------------------------
// カーソル
static void MoveMenuCursorUp( RRG_WORK* work ); // 上へ移動する
static void MoveMenuCursorDown( RRG_WORK* work ); // 下へ移動する
static void MoveMenuCursorDirect( RRG_WORK* work, MENU_ITEM menuItem ); // 直接移動する
static void MoveMenuCursorSilent( RRG_WORK* work, MENU_ITEM menuItem ); // 直接移動する ( 点滅・SEなし )
// 表示する質問
static void ChangeQuestionToNext( RRG_WORK* work ); // 次の質問に変更する
static void ChangeQuestionToPrev( RRG_WORK* work ); // 前の質問に変更する
// 表示する回答
static void ChangeAnswerToNext( RRG_WORK* work ); // 次の回答に変更する
static void ChangeAnswerToPrev( RRG_WORK* work ); // 前の回答に変更する
static void ChangeAnswerToFirst( RRG_WORK* work ); // 先頭の回答に変更する
static void ChangeAnswerToTop( RRG_WORK* work ); // ランク1位の回答に変更する
// 調査データの表示タイプ
static void SwitchDataDisplayType( RRG_WORK* work ); // 調査データの表示タイプを切り替える
// 画面のフェードイン・フェードアウト
static void StartFadeIn( void ); // 画面のフェードインを開始する
static void StartFadeOut( void ); // 画面のフェードアウトを開始する
//-----------------------------------------------------------------------------------------
// ◇LAYER 4 個別操作
//-----------------------------------------------------------------------------------------
// メニュー項目カーソル
static void ShiftMenuCursorPos( RRG_WORK* work, int stride ); // カーソル位置を変更する
static void SetMenuCursorPos( RRG_WORK* work, MENU_ITEM menuItem ); // カーソル位置を変更する
// メニュー項目の表示
static void SetMenuCursorOn( RRG_WORK* work ); // カーソルが乗っている状態にする
static void SetMenuCursorOff( RRG_WORK* work ); // カーソルが乗っていない状態にする
// 表示する質問インデックス
static void ShiftQuestionIdx( RRG_WORK* work, int stride ); // 表示する質問のインデックスを変更する
// 選択中の回答インデックス
static void SetAnswerIdxToTop( RRG_WORK* work ); // 選択中の回答インデックスをランク1位の項目にセットする
static void ShiftAnswerIdx( RRG_WORK* work, int stride ); // 選択中の回答インデックスを変更する
static void ResetAnswerIdx( RRG_WORK* work ); // 選択中の回答インデックスをリセットする
//『報告を見る』ボタン
static void UpdateAnalyzeButton( RRG_WORK* work ); //『報告を見る』ボタンを更新する
static void BlinkAnalyzeButton( RRG_WORK* work ); //『報告を見る』ボタンを点滅させる
static void SetAnalyzeButtonCursorOn( RRG_WORK* work ); //『報告を見る』ボタンをカーソルが乗っている状態にする
static void SetAnalyzeButtonCursorOff( RRG_WORK* work ); //『報告を見る』ボタンをカーソルが乗っていない状態にする
static void SetAnalyzeButtonCursorSet( RRG_WORK* work ); //『報告を見る』ボタンをカーソルをセットした状態にする
static void SetAnalyzeButtonActive( RRG_WORK* work ); //『報告を見る』ボタンをアクティブ状態にする
static void SetAnalyzeButtonNotActive( RRG_WORK* work ); //『報告を見る』ボタンを非アクティブ状態にする
//「戻る」ボタン
static void BlinkReturnButton( RRG_WORK* work ); //「戻る」ボタンを点滅させる
// 円グラフ
static void UpdateCircleGraphs( RRG_WORK* work ); // すべての円グラフを更新する
static void DrawCircleGraphs( const RRG_WORK* work ); // すべての円グラフを描画する
static void SetupMainCircleGraph( RRG_WORK* work, GRAPH_DISP_MODE graphMode ); // メイン円グラフを現在の調査データで構成する
static void SetupSubCircleGraph( RRG_WORK* work, GRAPH_DISP_MODE graphMode ); // サブ円グラフを現在の調査データで構成する
static void InterchangeCircleGraph( RRG_WORK* work ); // サブ円グラフとメイン円グラフを入れ替える
static void SetupCenterColorOfGraphComponent( GRAPH_COMPONENT_ADD_DATA* component ); // グラフ構成要素の中心点カラーをセットアップする
static void AdjustCircleGraphLayer( RRG_WORK* work ); // メイン円グラフ, サブ円グラフの重なり方を調整する
// 回答マーカー
static void DrawAnswerMarker( const RRG_WORK* work ); // 回答マーカーを描画する
// 矢印
static void UpdateArrow( RRG_WORK* work ); // 矢印の表示を更新する
// % 表示
static void SetupPercentages( RRG_WORK* work ); // 現在の調査データにあわせてセットアップする
static void VanishAllPercentage( RRG_WORK* work ); // 全ての％表示を消去する
static void DispPercentage( RRG_WORK* work, u8 index ); // ％オブジェクトを表示する
static void DispAllPercentage( RRG_WORK* work ); // 全ての％オブジェクトを表示する
// タッチ範囲
static void UpdateTouchArea( RRG_WORK* work ); // タッチ範囲を更新する
// BG
static void UpdateMainBG_WINDOW( RRG_WORK* work ); // MAIN-BG ( ウィンドウ面 ) を更新する
// BGFont
static void UpdateBGFont_TopicTitle( RRG_WORK* work ); // 調査項目名を更新する
static void UpdateBGFont_QuestionCaption( RRG_WORK* work ); // 質問の補足文を更新する
static void UpdateBGFont_Question( RRG_WORK* work ); // 質問を更新する 
static void UpdateBGFont_Answer( RRG_WORK* work ); // 回答を更新する
static void UpdateBGFont_MyAnswer( RRG_WORK* work ); // 自分の回答を更新する
static void UpdateBGFont_Count( RRG_WORK* work ); // 回答人数を更新する
static void UpdateBGFont_NoData( RRG_WORK* work ); //「ただいま ちょうさちゅう」の表示を更新する
static void UpdateBGFont_DataReceiving( RRG_WORK* work ); //「データしゅとくちゅう」の表示を更新する
// OBJ
static void UpdateControlCursor( RRG_WORK* work ); // 操作カーソルの表示を更新する
static void UpdateMyAnswerIconOnButton( RRG_WORK* work ); // 自分の回答アイコン ( ボタン上 ) を更新する
static void UpdateMyAnswerIconOnGraph( RRG_WORK* work ); // 自分の回答アイコン ( グラフ上 ) を更新する
// BMP-OAM
static void ShowAnalyzeButton( RRG_WORK* work ); //『報告を見る』ボタンを表示する
static void ShowAnalyzeMessage( RRG_WORK* work ); //「…かいせきちゅう…」を表示する
static void HideAnalyzeMessage( RRG_WORK* work ); //「…かいせきちゅう…」を非表示にする
static void BmpOamSetDrawEnable( RRG_WORK* work, BMPOAM_ACTOR_INDEX BmpOamActorIdx, BOOL enable );  // 表示するかどうかを設定する
// パレットアニメーション
static void StartPaletteAnime( RRG_WORK* work, PALETTE_ANIME_INDEX index ); // パレットアニメーションを開始する
static void StopPaletteAnime( RRG_WORK* work, PALETTE_ANIME_INDEX index ); // パレットアニメーションを停止する
static BOOL CheckPaletteAnime( const RRG_WORK* work, PALETTE_ANIME_INDEX index ); // パレットアニメーション中かどうかをチェックする
static void UpdatePaletteAnime( RRG_WORK* work ); // パレットアニメーションを更新する
static void UpdateCommonPaletteAnime( RRG_WORK* work ); // 共通パレットアニメーションを更新する
// パレットフェード
static void StartPaletteFadeFlashOut( RRG_WORK* work ); // パレットアニメによるフラッシュ ( アウト ) を開始する
static void StartPaletteFadeFlashIn ( RRG_WORK* work ); // パレットアニメによるフラッシュ ( イン ) を開始する
static BOOL IsPaletteFadeEnd( RRG_WORK* work ); // パレットのフェードが完了したかどうかを判定する
// VBlankタスク
static void VBlankFunc( GFL_TCB* tcb, void* wk );  // VBlank中の処理
// すれ違いの監視
static void WatchOutNewEntry( RRG_WORK* work ); // 新しい人物とのすれ違いを監視する
static void ClearNewEntryFlag( RRG_WORK* work ); // すれ違いフラグをクリアする
static void WatchOutUpdateEnable( RRG_WORK* work ); // 更新が可能な状態かどうかを監視する
static void DisableUpdate( RRG_WORK* work ); // 一定時間の間, 更新が起こらない状態にする
//-----------------------------------------------------------------------------------------
// ◇LAYER 3 取得・設定・判定
//-----------------------------------------------------------------------------------------
// システム・ワーク
static GAMESYS_WORK* GetGameSystem( const RRG_WORK* work ); // ゲームシステムを取得する
static GAMEDATA* GetGameData( const RRG_WORK* work ); // ゲームデータを取得する
static HEAPID GetHeapID( const RRG_WORK* work ); // ヒープIDを取得する
static void SetHeapID( RRG_WORK* work, HEAPID heapID );// ヒープIDを設定する
static RRC_WORK* GetCommonWork( const RRG_WORK* work ); // 全画面共通ワークを取得する
static void SetCommonWork( RRG_WORK* work, RRC_WORK* commonWork ); // 全画面共通ワークを設定する
static BOOL CheckForceReturnFlag( const RRG_WORK* work ); // 強制終了フラグ
// 調査データ表示タイプ
static void SetDataDisplayType( RRG_WORK* work, GRAPH_DISP_MODE graphMode ); // 調査データの表示タイプを設定する
// 円グラフ
static CIRCLE_GRAPH* GetMainGraph( const RRG_WORK* work ); // 現在表示中のメイン円グラフを取得する
static CIRCLE_GRAPH* GetSubGraph( const RRG_WORK* work ); // 現在表示中のサブ円グラフを取得する
static BOOL CheckAllGraphAnimeEnd( const RRG_WORK* work ); // アニメーション中の円グラフがあるかどうかを判定する
// 調査データ
static u8 GetTopicID( const RRG_WORK* work ); // 現在表示中の調査項目ID
static u8 GetQuestionID( const RRG_WORK* work ); // 現在表示中の質問ID
static u8 GetAnswerNum( const RRG_WORK* work ); // 現在表示中の質問に対する回答選択肢の数
static u16 GetAnswerID( const RRG_WORK* work ); // 現在表示中の回答ID
static u32 GetCountOfQuestion( const RRG_WORK* work ); // 現在表示中の質問に対する, 表示中の回答人数
static u32 GetTodayCountOfQuestion( const RRG_WORK* work ); // 現在表示中の質問に対する, 今日の回答人数
static u32 GetTotalCountOfQuestion( const RRG_WORK* work ); // 現在表示中の質問に対する, 合計の回答人数
static u32 GetCountOfAnswer( const RRG_WORK* work ); // 現在表示中の回答に対する, 表示中の回答人数
static u32 GetTodayCountOfAnswer( const RRG_WORK* work ); // 現在表示中の回答に対する, 今日の回答人数
static u32 GetTotalCountOfAnswer( const RRG_WORK* work ); // 現在表示中の回答に対する, 合計の回答人数
static u8 GetInvestigatingTopicID( const RRG_WORK* work ); // 現在調査中の調査項目IDを取得する
static u8 GetMyAnswerID( const RRG_WORK* work ); // 現在表示中の質問に対する, 自分の回答IDを取得する
static u8 GetMyAnswerID_PlayTime( const RRG_WORK* work ); // 質問『プレイ時間は？』に対する自分の回答IDを取得する
static u32 GetTodayCountOfQuestion_from_SaveData( const RRG_WORK* work ); // セーブデータが持つ, 現在表示中の質問に対する, 今日の回答人数
// OBJ
static u32 GetObjResourceRegisterIndex( const RRG_WORK* work, OBJ_RESOURCE_ID resID ); // OBJリソースの登録インデックスを取得する
static GFL_CLUNIT* GetClactUnit( const RRG_WORK* work, CLUNIT_INDEX unitIdx ); // セルアクターユニットを取得する
static GFL_CLWK* GetClactWork( const RRG_WORK* work, CLWK_INDEX wkIdx ); // セルアクターワークを取得する
//-----------------------------------------------------------------------------------------
// ◇LAYER 2 生成・初期化・破棄
//-----------------------------------------------------------------------------------------
static RRG_WORK* CreateGraphWork( HEAPID heapID ); // グラフ画面管理ワーク 生成
static void InitGraphWork( RRG_WORK* work ); // グラフ画面管理ワーク 初期化
static void DeleteGraphWork( RRG_WORK* work ); // グラフ画面管理ワーク 破棄
static void InitStateQueue( RRG_WORK* work ); // 状態キュー 初期化
static void CreateStateQueue( RRG_WORK* work ); // 状態キュー 作成
static void DeleteStateQueue( RRG_WORK* work ); // 状態キュー 破棄
static void InitFont( RRG_WORK* work ); // フォント 初期化
static void CreateFont( RRG_WORK* work ); // フォント 生成
static void DeleteFont( RRG_WORK* work ); // フォント 破棄
static void InitMessages( RRG_WORK* work ); // メッセージ 初期化
static void CreateMessages( RRG_WORK* work ); // メッセージ 生成
static void DeleteMessages( RRG_WORK* work ); // メッセージ 破棄
static void InitWordset( RRG_WORK* work ); // ワードセット 初期化
static void CreateWordset( RRG_WORK* work ); // ワードセット 生成
static void DeleteWordset( RRG_WORK* work ); // ワードセット 破棄
static void InitCircleGraphs( RRG_WORK* work ); // 円グラフ 初期化
static void CreateCircleGraph( RRG_WORK* work ); // 円グラフ 生成
static void DeleteCircleGraph( RRG_WORK* work ); // 円グラフ 破棄
static void InitResearchData( RRG_WORK* work ); // 調査データ 初期化
static void SetupResearchData( RRG_WORK* work ); // 調査データ 取得
static void SetupTouchArea( RRG_WORK* work ); // タッチ領域 準備
static void InitArrow( RRG_WORK* work ); // 矢印 初期化
static void CreateArrow( RRG_WORK* work ); // 矢印 生成
static void DeleteArrow( RRG_WORK* work ); // 矢印 破棄
static void InitPercentage( RRG_WORK* work ); // % 表示オブジェクト 初期化
static void CreatePercentage( RRG_WORK* work ); // % 表示オブジェクト 生成
static void DeletePercentage( RRG_WORK* work ); // % 表示オブジェクト 破棄
static void Setup3D( void ); // 3D 描画 準備
static void SetupBG( RRG_WORK* work ); // BG 全般 準備
static void SetupSubBG_WINDOW( RRG_WORK* work ); // SUB-BG ( ウィンドウ面 ) 準備
static void SetupSubBG_FONT( RRG_WORK* work ); // SUB-BG ( フォント面 ) 準備
static void SetupMainBG_WINDOW( RRG_WORK* work ); // MAIN-BG ( ウィンドウ面 ) 準備
static void SetupMainBG_FONT( RRG_WORK* work ); // MAIN-BG ( フォント面 ) 準備
static void CleanUpBG( RRG_WORK* work ); // BG 全般 後片付け
static void CleanUpSubBG_WINDOW( RRG_WORK* work ); // SUB-BG ( ウィンドウ面 ) 後片付け
static void CleanUpSubBG_FONT( RRG_WORK* work ); // SUB-BG ( フォント面 ) 後片付け
static void CleanUpMainBG_WINDOW( RRG_WORK* work ); // MAIN-BG ( ウィンドウ面 ) 後片付け
static void CleanUpMainBG_FONT( RRG_WORK* work ); // MAIN-BG ( フォント面 ) 後片付け
static void InitBGFonts( RRG_WORK* work ); // 文字列描画オブジェクト 初期化
static void CreateBGFonts( RRG_WORK* work ); // 文字列描画オブジェクト 生成
static void DeleteBGFonts( RRG_WORK* work ); // 文字列描画オブジェクト 破棄
static void RegisterSubObjResources( RRG_WORK* work ); // SUB-OBJ リソース 登録
static void ReleaseSubObjResources( RRG_WORK* work ); // SUB-OBJ リソース 解放
static void RegisterMainObjResources( RRG_WORK* work ); // MAIN-OBJ リソース 登録
static void ReleaseMainObjResources( RRG_WORK* work ); // MAIN-OBJ リソース 解放
static void InitClactUnits( RRG_WORK* work ); // セルアクターユニット 初期化
static void CreateClactUnits( RRG_WORK* work ); // セルアクターユニット 生成
static void DeleteClactUnits( RRG_WORK* work ); // セルアクターユニット 破棄
static void InitClactWorks( RRG_WORK* work ); // セルアクターワーク 初期化
static void CreateClactWorks( RRG_WORK* work ); // セルアクターワーク 生成
static void DeleteClactWorks( RRG_WORK* work ); // セルアクターワーク 破棄
static void InitBitmapDatas( RRG_WORK* work ); // ビットマップデータ 初期化
static void CreateBitmapDatas( RRG_WORK* work ); // ビットマップデータ 生成
static void SetupBitmapData_forDefault( RRG_WORK* work ); // ビットマップデータ 準備 ( デフォルト ) 
static void SetupBitmapData_forANALYZE_BUTTON( RRG_WORK* work ); // ビットマップデータ 準備 ( 『報告を見る』ボタン ) 
static void DeleteBitmapDatas( RRG_WORK* work ); // ビットマップデータ 破棄
static void SetupBmpOamSystem( RRG_WORK* work ); // BMP-OAM システム 準備
static void CleanUpBmpOamSystem( RRG_WORK* work ); // BMP-OAM システム 後片付け
static void CreateBmpOamActors( RRG_WORK* work ); // BMP-OAM アクター 生成
static void DeleteBmpOamActors( RRG_WORK* work ); // BMP-OAM アクター 破棄
static void InitPaletteFadeSystem( RRG_WORK* work ); // パレットフェードシステム 初期化
static void SetupPaletteFadeSystem( RRG_WORK* work ); // パレットフェードシステム 準備
static void CleanUpPaletteFadeSystem( RRG_WORK* work ); // パレットフェードシステム 後片付け
static void InitPaletteAnime( RRG_WORK* work ); // パレットアニメーションワーク 初期化
static void CreatePaletteAnime( RRG_WORK* work ); // パレットアニメーションワーク 生成
static void DeletePaletteAnime( RRG_WORK* work ); // パレットアニメーションワーク 破棄
static void SetupPaletteAnime( RRG_WORK* work ); // パレットアニメーションワーク 準備
static void CleanUpPaletteAnime( RRG_WORK* work ); // パレットアニメーションワーク 後片付け
static void SetupWirelessIcon( const RRG_WORK* work ); // 通信アイコン 準備
static void RegisterVBlankTask( RRG_WORK* work ); // VBlank タスク 登録
static void ReleaseVBlankTask( RRG_WORK* work ); // VBlank タスク 解除
//-----------------------------------------------------------------------------------------
// ◇LAYER 1 ユーティリティ
//-----------------------------------------------------------------------------------------
static u8 Bind_u8( int num ); // u8 に収まるように丸める
static void IntrudeShutdownRequest( const RRG_WORK* work ); // 侵入切断リクエストを発行する
static BOOL CheckIntrudeShutdown( const RRG_WORK* work ); // 侵入切断の完了をチェックする
static void RecoverIntrudeShutdown( const RRG_WORK* work ); // 侵入接続を許可する




//=========================================================================================
// □public functions
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査報告確認画面ワークの生成
 *
 * @param commonWork 全画面共通ワーク
 *
 * @return 調査報告確認画面ワーク
 */
//-----------------------------------------------------------------------------------------
RRG_WORK* RRG_CreateWork( RRC_WORK* commonWork )
{
  RRG_WORK* work;
  HEAPID heapID;

  heapID = RRC_GetHeapID( commonWork );

  // ワークを生成
  work = CreateGraphWork( heapID );

  // ワークを初期化
  InitGraphWork( work );
  SetHeapID( work, heapID );
  SetCommonWork( work, commonWork );

  return work;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査報告確認画面ワークの破棄
 *
 * @param heapID
 */
//-----------------------------------------------------------------------------------------
void RRG_DeleteWork( RRG_WORK* work )
{
  DeleteStateQueue( work );
  DeleteGraphWork( work );
} 

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査報告確認画面 メイン動作
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
void RRG_Main( RRG_WORK* work )
{
  // 状態ごとの処理
  switch( work->state ) {
  case RRG_STATE_SETUP:             MainSeq_SETUP( work );             break;
  case RRG_STATE_INTRUDE_SHUTDOWN:  MainSeq_INTRUDE_SHUTDOWN( work );  break;
  case RRG_STATE_STANDBY:           MainSeq_STANDBY( work );           break;
  case RRG_STATE_KEYWAIT:           MainSeq_KEYWAIT( work );           break;
  case RRG_STATE_ANALYZE:           MainSeq_ANALYZE( work );           break;
  case RRG_STATE_PERCENTAGE:        MainSeq_PERCENTAGE( work );        break;
  case RRG_STATE_FLASHOUT:          MainSeq_FLASHOUT( work );          break;
  case RRG_STATE_FLASHIN:           MainSeq_FLASHIN( work );           break;
  case RRG_STATE_UPDATE_AT_STANDBY: MainSeq_UPDATE_AT_STANDBY( work ); break;
  case RRG_STATE_UPDATE_AT_KEYWAIT: MainSeq_UPDATE_AT_KEYWAIT( work ); break;
  case RRG_STATE_FADEOUT:           MainSeq_FADEOUT( work );           break;
  case RRG_STATE_WAIT:              MainSeq_WAIT( work );              break;
  case RRG_STATE_CLEANUP:           MainSeq_CLEANUP( work );           break;
  case RRG_STATE_FINISH:            return;                            
  default: GF_ASSERT(0);
  }

  WatchOutNewEntry( work );
  WatchOutUpdateEnable( work );

  // 描画
  if( work->setupFlag ) { 
    // 2D 描画
    GFL_CLACT_SYS_Main();             // セルアクターシステム メイン処理
    UpdateCircleGraphs( work );       // 円グラフ メイン動作
    ARROW_Main( work->arrow );        // 矢印 メイン動作
    UpdateCommonPaletteAnime( work ); // 共通パレットアニメーションを更新
    UpdatePaletteAnime( work );       // パレットアニメーション更新

    // 3D 描画
    DrawCircleGraphs( work ); // 円グラフを描画
    DrawAnswerMarker( work ); // 回答マーカーを描画
    G3_SwapBuffers( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
  }

  // 状態を更新
  CountUpStateCount( work ); // 状態カウンタ更新
  SwitchState( work );       // 状態更新
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 終了判定
 *
 * @param work
 *
 * @return グラフ画面が終了した場合 TRUE
 */
//-----------------------------------------------------------------------------------------
BOOL RRG_IsFinished( const RRG_WORK* work )
{
  return work->finishFlag;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 終了結果の取得
 *
 * @param work
 *
 * @return グラフ画面の終了結果
 */
//-----------------------------------------------------------------------------------------
RRG_RESULT RRG_GetResult( const RRG_WORK* work )
{
  return work->finishResult;
}



//=========================================================================================
// □private functions
//=========================================================================================

//=========================================================================================
// ■LAYER 6 状態動作
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief 準備状態 ( RRG_STATE_SETUP ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainSeq_SETUP( RRG_WORK* work )
{
  CreateStateQueue( work );
  CreateFont( work );
  CreateMessages( work );
  CreateWordset( work );
  SetupTouchArea( work );
  UpdateTouchArea( work );

  // 3D 初期設定
  Setup3D();

  // BG 準備
  SetupBG( work );
  SetupSubBG_WINDOW ( work );
  SetupSubBG_FONT( work );
  SetupMainBG_WINDOW( work );
  SetupMainBG_FONT( work );

  // 文字列描画オブジェクト 準備
  CreateBGFonts( work );

  // OBJ 準備
  RegisterSubObjResources( work );
  RegisterMainObjResources( work );
  CreateClactUnits( work );
  CreateClactWorks( work );

  // BMP-OAM 準備
  CreateBitmapDatas( work );
  SetupBitmapData_forDefault( work );
  SetupBitmapData_forANALYZE_BUTTON( work );
  SetupBmpOamSystem( work );
  CreateBmpOamActors( work );

  SetupResearchData( work );  // 調査データを取得
  CreateCircleGraph( work );  // 円グラフ 作成
  CreateArrow( work );        // 矢印 作成
  CreatePercentage( work );   // % 表示オブジェクト生成

  SetupPaletteFadeSystem( work ); // パレットフェードシステム 準備
  CreatePaletteAnime( work );     // パレットアニメーションワークを生成
  SetupPaletteAnime( work );      // パレットアニメーションワークをセットアップ
  RegisterVBlankTask( work );     // VBkankタスク登録

  // 通信アイコン
  SetupWirelessIcon( work );

  UpdateBGFont_TopicTitle( work );      // 調査項目名を更新する
  UpdateBGFont_QuestionCaption( work ); // 質問の補足文を更新する
  UpdateBGFont_Question( work );        // 質問を更新する 
  UpdateBGFont_Answer( work );          // 回答を更新する
  UpdateBGFont_MyAnswer( work );        // 自分の回答を更新する
  UpdateBGFont_Count( work );           // 回答人数を更新する
  UpdateBGFont_NoData( work );          //「ただいま ちょうさちゅう」の表示を更新する
  UpdateBGFont_DataReceiving( work );   //「データしゅとくちゅう」の表示を更新する
  UpdateControlCursor( work );          // 左右カーソルを更新
  UpdateMainBG_WINDOW( work );          // MAIN-BG ( ウィンドウ面 ) を更新する
  UpdateMyAnswerIconOnButton( work );   // 自分の回答アイコン ( ボタン上 ) を更新する
  ShowAnalyzeButton( work );            //『報告を見る』ボタンを表示
  StartPaletteAnime( work, PALETTE_ANIME_CURSOR_ON ); // カーソルONパレットアニメを開始
  StartPaletteAnime( work, PALETTE_ANIME_RECEIVE_BUTTON ); //「データ受信中」ボタンのパレットアニメを開始
  UpdateAnalyzeButton( work ); //『報告を見る』ボタンを更新する

  // セットアップ完了
  work->setupFlag = TRUE;

  // 画面フェードイン開始
  StartFadeIn();

  // 次の状態をセット
  {
    RRG_STATE next_state;

    next_state = GetFirstState( work ); 
    RegisterNextState( work, RRG_STATE_INTRUDE_SHUTDOWN ); // 次の状態をセット
    RegisterNextState( work, next_state ); // 次の状態をセット

    if( next_state == RRG_STATE_KEYWAIT ) {
      SetMenuCursorOn( work ); // カーソルが乗っている状態にする
    } 
  }

  // 状態終了
  FinishCurrentState( work ); 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 侵入切断状態 ( RRG_STATE_INTRUDE_SHUTDOWN ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainSeq_INTRUDE_SHUTDOWN( RRG_WORK* work )
{ 
  switch( GetStateSeq( work ) ) {
  // 侵入切断
  case 0:
    IntrudeShutdownRequest( work );
    IncStateSeq( work );
    break;

  // 侵入切断完了待ち
  case 1:
    if( CheckIntrudeShutdown( work ) ) {
      FinishCurrentState( work ); // RRG_STATE_INTRUDE_SHUTDOWN 状態終了
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 準備状態 ( RRG_STATE_STANDBY ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainSeq_STANDBY( RRG_WORK* work )
{
  int key;
  int trg;
  int touch;
  int commonTouch;

  key = GFL_UI_KEY_GetCont();
  trg = GFL_UI_KEY_GetTrg(); 
  touch = GFL_UI_TP_HitTrg( work->touchHitTable );
  commonTouch = GFL_UI_TP_HitTrg( RRC_GetHitTable(work->commonWork) );

#ifdef PM_DEBUG
  //----------------
  // デバッグボタン
  if( work->analyzeFlag && (trg & PAD_BUTTON_DEBUG) ) {
    FinishCurrentState( work );                              // RRG_STATE_STANDBY 状態を終了
    RegisterNextState( work, RRG_STATE_UPDATE_AT_STANDBY );  // => RRG_STATE_UPDATE_AT_STANDBY
    RegisterNextState( work, RRG_STATE_FLASHOUT );           // ==> RRG_STATE_FLASHOUT
    RegisterNextState( work, RRG_STATE_FLASHIN );            // ===> RRG_STATE_FLASHIN
    RegisterNextState( work, RRG_STATE_PERCENTAGE );         // ====> RRG_STATE_PERCENTAGE
    RegisterNextState( work, RRG_STATE_STANDBY );            // =====> RRG_STATE_STANDBY
    return;
  }
#endif

  //------------------------
  // 調査データの更新を検出
  if( work->analyzeFlag && work->newEntryFlag && work->updateEnableFlag ) {
    FinishCurrentState( work );                              // RRG_STATE_STANDBY 状態を終了
    RegisterNextState( work, RRG_STATE_UPDATE_AT_STANDBY );  // => RRG_STATE_UPDATE_AT_STANDBY
    RegisterNextState( work, RRG_STATE_FLASHOUT );           // ==> RRG_STATE_FLASHOUT
    RegisterNextState( work, RRG_STATE_FLASHIN );            // ===> RRG_STATE_FLASHIN
    RegisterNextState( work, RRG_STATE_PERCENTAGE );         // ====> RRG_STATE_PERCENTAGE
    RegisterNextState( work, RRG_STATE_STANDBY );            // =====> RRG_STATE_STANDBY
    return;
  }

  //------------------------------------
  // 強制終了 or 『戻る』ボタンをタッチ
  if( CheckForceReturnFlag( work ) || (commonTouch == COMMON_TOUCH_AREA_RETURN_BUTTON) ) {
    SetFinishReason( work, SEQ_CHANGE_BY_TOUCH ); // 画面遷移のトリガを登録
    BlinkReturnButton( work );                    //『戻る』ボタンを明滅させる
    PMSND_PlaySE( SEQ_SE_CANCEL1 );               // キャンセル音
    FinishCurrentState( work );                   // RRG_STATE_STANDBY 状態を終了
    RegisterNextState( work, RRG_STATE_WAIT );    // => RRG_STATE_WAIT
    RegisterNextState( work, RRG_STATE_FADEOUT ); // ==> RRG_STATE_FADEOUT
    RegisterNextState( work, RRG_STATE_CLEANUP ); // ===> RRG_STATE_CLEANUP
    return;
  }

  //----------
  // キー入力
  if( (trg & PAD_KEY_UP)   || (trg & PAD_KEY_DOWN)  ||
      (trg & PAD_KEY_LEFT) || (trg & PAD_KEY_RIGHT) || (trg & PAD_BUTTON_A) ) {
    SetMenuCursorOn( work );                      // カーソルが乗っている状態にする
    UpdateAnalyzeButton( work );                  //『報告を見る』ボタンを更新する
    FinishCurrentState( work );                   // RRG_STATE_STANDBY 状態を終了
    RegisterNextState( work, RRG_STATE_KEYWAIT ); // => RRG_STATE_KEYWAIT
    return;
  }

  //-------------------
  //「円グラフ」タッチ
  if( touch == TOUCH_AREA_GRAPH ) {
    if( (work->analyzeFlag == FALSE ) && (GetCountOfQuestion(work) != 0) ) {
      work->analyzeByTouchFlag = TRUE;                 // タッチで解析
      BlinkAnalyzeButton( work );                      //『報告を見る』ボタンを明滅させる
      FinishCurrentState( work );                      // RRG_STATE_STANDBY 状態を終了
      RegisterNextState( work, RRG_STATE_ANALYZE );    // => RRG_STATE_ANALYZE 
      RegisterNextState( work, RRG_STATE_FLASHOUT );   // ==> RRG_STATE_FLASHOUT 
      RegisterNextState( work, RRG_STATE_FLASHIN );    // ===> RRG_STATE_FLASHIN 
      RegisterNextState( work, RRG_STATE_PERCENTAGE ); // ====> RRG_STATE_PERCENTAGE 
      RegisterNextState( work, RRG_STATE_STANDBY );    // =====> RRG_STATE_STANDBY 
      return;
    }
  }

  //---------------------
  //『報告を見る』ボタンをタッチ
  if( touch == TOUCH_AREA_ANALYZE_BUTTON ) {
    if( (work->analyzeFlag == FALSE) && (GetCountOfQuestion(work) != 0) ) {
      work->analyzeByTouchFlag = TRUE;                 // タッチで解析
      BlinkAnalyzeButton( work );                      //『報告を見る』ボタンを明滅させる
      FinishCurrentState( work );                      // RRG_STATE_STANDBY 状態を終了
      RegisterNextState( work, RRG_STATE_ANALYZE );    // => RRG_STATE_ANALYZE 
      RegisterNextState( work, RRG_STATE_FLASHOUT );   // ==> RRG_STATE_FLASHOUT 
      RegisterNextState( work, RRG_STATE_FLASHIN );    // ===> RRG_STATE_FLASHIN 
      RegisterNextState( work, RRG_STATE_PERCENTAGE ); // ====> RRG_STATE_PERCENTAGE 
      RegisterNextState( work, RRG_STATE_STANDBY );    // =====> RRG_STATE_STANDBY 
      return;
    }
    else {
      PMSND_PlaySE( SEQ_SE_BEEP );
      return;
    }
  }

  //-----------------------
  //「質問」ボタンをタッチ
  if( touch == TOUCH_AREA_QUESTION ) {
    if( (work->analyzeFlag == FALSE ) && (GetCountOfQuestion(work) != 0) ) {
      work->analyzeByTouchFlag = TRUE;                 // タッチで解析
      BlinkAnalyzeButton( work );                      //『報告を見る』ボタンを明滅させる
      FinishCurrentState( work );                      // RRG_STATE_STANDBY 状態を終了
      RegisterNextState( work, RRG_STATE_ANALYZE );    // => RRG_STATE_ANALYZE 
      RegisterNextState( work, RRG_STATE_FLASHOUT );   // ==> RRG_STATE_FLASHOUT 
      RegisterNextState( work, RRG_STATE_FLASHIN );    // ===> RRG_STATE_FLASHIN 
      RegisterNextState( work, RRG_STATE_PERCENTAGE ); // ====> RRG_STATE_PERCENTAGE 
      RegisterNextState( work, RRG_STATE_STANDBY );    // =====> RRG_STATE_STANDBY 
      return;
    }
    else {
      MoveMenuCursorDirect( work, MENU_ITEM_QUESTION ); // カーソル位置を設定
      return;
    }
  }

  //-----------------------
  //「回答」ボタンをタッチ
  if( work->analyzeFlag && (GetCountOfQuestion(work) != 0) && (touch == TOUCH_AREA_ANSWER ) ) {
    MoveMenuCursorDirect( work, MENU_ITEM_ANSWER ); // カーソル位置を設定
    SetMenuCursorOn( work );                        // カーソルが乗っている状態にする
    UpdateAnalyzeButton( work );                    //『報告を見る』ボタンを更新する
    FinishCurrentState( work );                     // RRG_STATE_STANDBY 状態を終了
    RegisterNextState( work, RRG_STATE_KEYWAIT );   // => RRG_STATE_KEYWAIT
    return;
  } 
  //-----------------------------
  //「自分の回答」ボタンをタッチ
  if( work->analyzeFlag && (GetCountOfQuestion(work) != 0) && (touch == TOUCH_AREA_MY_ANSWER) ) {
    MoveMenuCursorDirect( work, MENU_ITEM_MY_ANSWER ); // カーソル位置を設定
    SetMenuCursorOn( work );                           // カーソルが乗っている状態にする
    UpdateAnalyzeButton( work );                       //『報告を見る』ボタンを更新する
    FinishCurrentState( work );                        // RRG_STATE_STANDBY 状態を終了
    RegisterNextState( work, RRG_STATE_KEYWAIT );      // => RRG_STATE_KEYWAIT
    return;
  } 
  //-----------------------------
  //「回答人数」ボタンをタッチ
  if( touch == TOUCH_AREA_COUNT ) {
    MoveMenuCursorDirect( work, MENU_ITEM_COUNT ); // カーソル位置を設定
    SetMenuCursorOn( work );                       // カーソルが乗っている状態にする
    UpdateAnalyzeButton( work );                   //『報告を見る』ボタンを更新する
    FinishCurrentState( work );                    // RRG_STATE_STANDBY 状態を終了
    RegisterNextState( work, RRG_STATE_KEYWAIT );  // => RRG_STATE_KEYWAIT
    return;
  }

  //---------------------
  // 左カーソルをタッチ
  if( touch == TOUCH_AREA_CONTROL_CURSOR_L ) {
    SetMenuCursorOn( work );                      // カーソルが乗っている状態にする
    UpdateAnalyzeButton( work );                  //『報告を見る』ボタンを更新する
    FinishCurrentState( work );                   // RRG_STATE_STANDBY 状態を終了
    RegisterNextState( work, RRG_STATE_KEYWAIT ); // => RRG_STATE_KEYWAIT
    // カーソル位置に応じた処理
    switch( work->cursorPos ) {
    case MENU_ITEM_QUESTION:  ChangeQuestionToPrev( work );  break;
    case MENU_ITEM_ANSWER:    ChangeAnswerToPrev( work );    break;
    case MENU_ITEM_COUNT:     SwitchDataDisplayType( work ); break;
    }
    return;
  } 
  //---------------------
  // 右カーソルをタッチ
  if( touch == TOUCH_AREA_CONTROL_CURSOR_R ) {
    SetMenuCursorOn( work );                      // カーソルが乗っている状態にする
    UpdateAnalyzeButton( work );                  //『報告を見る』ボタンを更新する
    FinishCurrentState( work );                   // RRG_STATE_STANDBY 状態を終了
    RegisterNextState( work, RRG_STATE_KEYWAIT ); // => RRG_STATE_KEYWAIT
    // カーソル位置に応じた処理
    switch( work->cursorPos ) {
    case MENU_ITEM_QUESTION:  ChangeQuestionToNext( work );  break;
    case MENU_ITEM_ANSWER:    ChangeAnswerToNext( work );    break;
    case MENU_ITEM_COUNT:     SwitchDataDisplayType( work ); break;
    }
    return;
  }

  //----------
  // B ボタン
  if( trg & PAD_BUTTON_B ) {
    SetFinishReason( work, SEQ_CHANGE_BY_BUTTON ); // 画面遷移のトリガを登録
    BlinkReturnButton( work );                     //『戻る』ボタンを明滅させる
    PMSND_PlaySE( SEQ_SE_CANCEL1 );                // キャンセル音
    FinishCurrentState( work );                    // RRG_STATE_STANDBY 状態を終了
    RegisterNextState( work, RRG_STATE_WAIT );     // => RRG_STATE_WAIT
    RegisterNextState( work, RRG_STATE_FADEOUT );  // ==> RRG_STATE_FADEOUT
    RegisterNextState( work, RRG_STATE_CLEANUP );  // ===> RRG_STATE_CLEANUP
    return;
  } 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief キー入力待ち状態 ( RRG_STATE_KEYWAIT ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainSeq_KEYWAIT( RRG_WORK* work )
{
  int key;
  int trg;
  int touch;
  int commonTouch;

  key = GFL_UI_KEY_GetCont();
  trg = GFL_UI_KEY_GetTrg(); 
  touch = GFL_UI_TP_HitTrg( work->touchHitTable );
  commonTouch = GFL_UI_TP_HitTrg( RRC_GetHitTable(work->commonWork) );

#ifdef PM_DEBUG
  //----------------
  // デバッグボタン
  if( work->analyzeFlag && (trg & PAD_BUTTON_DEBUG) ) {
    FinishCurrentState( work );                             // RRG_STATE_KEYWAIT 状態を終了
    RegisterNextState( work, RRG_STATE_UPDATE_AT_KEYWAIT ); // => RRG_STATE_UPDATE_AT_KEYWAIT
    RegisterNextState( work, RRG_STATE_FLASHOUT );          // ==> RRG_STATE_FLASHOUT
    RegisterNextState( work, RRG_STATE_FLASHIN );           // ===> RRG_STATE_FLASHIN
    RegisterNextState( work, RRG_STATE_PERCENTAGE );        // ====> RRG_STATE_PERCENTAGE
    RegisterNextState( work, RRG_STATE_KEYWAIT );           // =====> RRG_STATE_KEYWAIT
    return;
  }
#endif

  //------------------------
  // 調査データの更新を検出
  if( work->analyzeFlag && work->newEntryFlag && work->updateEnableFlag ) {
    FinishCurrentState( work );                             // RRG_STATE_KEYWAIT 状態を終了
    RegisterNextState( work, RRG_STATE_UPDATE_AT_KEYWAIT ); // => RRG_STATE_UPDATE_AT_KEYWAIT
    RegisterNextState( work, RRG_STATE_FLASHOUT );          // ==> RRG_STATE_FLASHOUT
    RegisterNextState( work, RRG_STATE_FLASHIN );           // ===> RRG_STATE_FLASHIN
    RegisterNextState( work, RRG_STATE_PERCENTAGE );        // ====> RRG_STATE_PERCENTAGE
    RegisterNextState( work, RRG_STATE_KEYWAIT );           // =====> RRG_STATE_KEYWAIT
    return;
  } 

  //------------------------------------
  // 強制終了 or 『戻る』ボタンをタッチ
  if( CheckForceReturnFlag( work ) || (commonTouch == COMMON_TOUCH_AREA_RETURN_BUTTON) ) {
    SetFinishReason( work, SEQ_CHANGE_BY_TOUCH );  // 画面遷移のトリガを登録
    BlinkReturnButton( work );                     //『戻る』ボタンを明滅させる
    PMSND_PlaySE( SEQ_SE_CANCEL1 );                // キャンセル音
    FinishCurrentState( work );                    // RRG_STATE_KEYWAIT 状態を終了
    RegisterNextState( work, RRG_STATE_WAIT );     // => RRG_STATE_WAIT 
    RegisterNextState( work, RRG_STATE_FADEOUT );  // ==> RRG_STATE_FADEOUT 
    RegisterNextState( work, RRG_STATE_CLEANUP );  // ===> RRG_STATE_CLEANUP 
    return;
  }

  //--------
  // ↑キー
  if( trg & PAD_KEY_UP ) {
    MoveMenuCursorUp( work ); // カーソル移動
    return;
  } 
  //--------
  // ↓キー
  if( trg & PAD_KEY_DOWN ) {
    MoveMenuCursorDown( work ); // カーソル移動
    return;
  } 

  //-------------------
  //「円グラフ」タッチ
  if( touch == TOUCH_AREA_GRAPH ) {
    if( (work->analyzeFlag == FALSE ) && (GetCountOfQuestion(work) != 0) ) {
      work->analyzeByTouchFlag = TRUE;                 // タッチで解析
      BlinkAnalyzeButton( work );                      //『報告を見る』ボタンを明滅させる
      FinishCurrentState( work );                      // RRG_STATE_KEYWAIT 状態を終了
      RegisterNextState( work, RRG_STATE_ANALYZE );    // => RRG_STATE_ANALYZE 
      RegisterNextState( work, RRG_STATE_FLASHOUT );   // ==> RRG_STATE_FLASHOUT 
      RegisterNextState( work, RRG_STATE_FLASHIN );    // ===> RRG_STATE_FLASHIN 
      RegisterNextState( work, RRG_STATE_PERCENTAGE ); // ====> RRG_STATE_PERCENTAGE 
      RegisterNextState( work, RRG_STATE_STANDBY );    // =====> RRG_STATE_STANDBY 
      return;
    }
  } 
  //----------------------------
  //『報告を見る』ボタンをタッチ
  if( touch == TOUCH_AREA_ANALYZE_BUTTON ) {
    if( (work->analyzeFlag == FALSE ) && (GetCountOfQuestion(work) != 0) ) {
      work->analyzeByTouchFlag = TRUE;                 // タッチで解析
      BlinkAnalyzeButton( work );                      //『報告を見る』ボタンを明滅させる
      FinishCurrentState( work );                      // RRG_STATE_KEYWAIT 状態を終了
      RegisterNextState( work, RRG_STATE_ANALYZE );    // => RRG_STATE_ANALYZE 
      RegisterNextState( work, RRG_STATE_FLASHOUT );   // ==> RRG_STATE_FLASHOUT 
      RegisterNextState( work, RRG_STATE_FLASHIN );    // ===> RRG_STATE_FLASHIN 
      RegisterNextState( work, RRG_STATE_PERCENTAGE ); // ====> RRG_STATE_PERCENTAGE 
      RegisterNextState( work, RRG_STATE_STANDBY );    // =====> RRG_STATE_STANDBY 
      return;
    }
    else {
      PMSND_PlaySE( SEQ_SE_BEEP );
      return;
    }
  }

  //-----------------------
  //「質問」ボタンをタッチ
  if( touch == TOUCH_AREA_QUESTION ) {
    if( (work->analyzeFlag == FALSE ) && (GetCountOfQuestion(work) != 0) ) {
      work->analyzeByTouchFlag = TRUE;                 // タッチで解析
      BlinkAnalyzeButton( work );                      //『報告を見る』ボタンを明滅させる
      FinishCurrentState( work );                      // RRG_STATE_STANDBY 状態を終了
      RegisterNextState( work, RRG_STATE_ANALYZE );    // => RRG_STATE_ANALYZE 
      RegisterNextState( work, RRG_STATE_FLASHOUT );   // ==> RRG_STATE_FLASHOUT 
      RegisterNextState( work, RRG_STATE_FLASHIN );    // ===> RRG_STATE_FLASHIN 
      RegisterNextState( work, RRG_STATE_PERCENTAGE ); // ====> RRG_STATE_PERCENTAGE 
      RegisterNextState( work, RRG_STATE_STANDBY );    // =====> RRG_STATE_STANDBY 
    }
    else {
      MoveMenuCursorDirect( work, MENU_ITEM_QUESTION ); // カーソル位置を設定
    }
    return;
  }
  //-----------------------
  //「回答」ボタンをタッチ
  if( work->analyzeFlag && (GetCountOfQuestion(work) != 0) && (touch == TOUCH_AREA_ANSWER) ) {
    MoveMenuCursorDirect( work, MENU_ITEM_ANSWER ); // カーソル移動
    return;
  } 
  //-----------------------------
  //「自分の回答」ボタンをタッチ
  if( work->analyzeFlag && (GetCountOfQuestion(work) != 0) && (touch == TOUCH_AREA_MY_ANSWER) ) {
    MoveMenuCursorDirect( work, MENU_ITEM_MY_ANSWER ); // カーソル移動
    return;
  } 
  //-----------------------------
  //「回答人数」ボタンをタッチ
  if( touch == TOUCH_AREA_COUNT ) {
    MoveMenuCursorDirect( work, MENU_ITEM_COUNT ); // カーソル移動
    return;
  }

  //-----------------------------
  // ←キー or 左カーソルをタッチ
  if( (trg & PAD_KEY_LEFT) || (touch == TOUCH_AREA_CONTROL_CURSOR_L) ) {
    // カーソル位置に応じた処理
    switch( work->cursorPos ) {
    case MENU_ITEM_QUESTION:  ChangeQuestionToPrev( work );  break;
    case MENU_ITEM_ANSWER:    ChangeAnswerToPrev( work );    break;
    case MENU_ITEM_COUNT:     SwitchDataDisplayType( work ); break;
    }
  } 
  //-----------------------------
  // →キー or 右カーソルをタッチ
  if( (trg & PAD_KEY_RIGHT) || (touch == TOUCH_AREA_CONTROL_CURSOR_R) ) {
    // カーソル位置に応じた処理
    switch( work->cursorPos ) {
    case MENU_ITEM_QUESTION:  ChangeQuestionToNext( work );  break;
    case MENU_ITEM_ANSWER:    ChangeAnswerToNext( work );    break;
    case MENU_ITEM_COUNT:     SwitchDataDisplayType( work ); break;
    }
  }

  //----------
  // A ボタン
  if( trg & PAD_BUTTON_A ) {
    if( (work->analyzeFlag == FALSE ) && (GetCountOfQuestion(work) != 0) && 
        ( (work->cursorPos == MENU_ITEM_QUESTION) || (work->cursorPos == MENU_ITEM_ANALYZE) ) ) {
      work->analyzeByTouchFlag = FALSE;                // ボタンで解析
      BlinkAnalyzeButton( work );                      //『報告を見る』ボタンを明滅させる
      FinishCurrentState( work );                      // RRG_STATE_KEYWAIT 状態を終了
      RegisterNextState( work, RRG_STATE_ANALYZE );    // => RRG_STATE_ANALYZE 
      RegisterNextState( work, RRG_STATE_FLASHOUT );   // ==> RRG_STATE_FLASHOUT 
      RegisterNextState( work, RRG_STATE_FLASHIN );    // ===> RRG_STATE_FLASHIN 
      RegisterNextState( work, RRG_STATE_PERCENTAGE ); // ====> RRG_STATE_PERCENTAGE 
      RegisterNextState( work, RRG_STATE_KEYWAIT );    // =====> RRG_STATE_KEYWAIT 
      return;
    }
  } 

  //----------
  // B ボタン
  if( trg & PAD_BUTTON_B ) {
    SetFinishReason( work, SEQ_CHANGE_BY_BUTTON ); // 画面遷移のトリガを登録
    BlinkReturnButton( work );                     //『戻る』ボタンを明滅させる
    PMSND_PlaySE( SEQ_SE_CANCEL1 );                // キャンセル音
    FinishCurrentState( work );                    // RRG_STATE_KEYWAIT 状態を終了
    RegisterNextState( work, RRG_STATE_WAIT );     // => RRG_STATE_WAIT 
    RegisterNextState( work, RRG_STATE_FADEOUT );  // ==> RRG_STATE_FADEOUT 
    RegisterNextState( work, RRG_STATE_CLEANUP );  // ===> RRG_STATE_CLEANUP 
    return;
  } 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 解析状態 ( RRG_STATE_ANALYZE ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainSeq_ANALYZE( RRG_WORK* work )
{ 
  switch( GetStateSeq(work) ) {
  case 0:
    SetupResearchData( work );                              // 調査データを再セットアップ
    ClearNewEntryFlag( work );                              // この時点でデータは最新
    ShowAnalyzeMessage( work );                             //「…かいせきちゅう…」を表示
    SetupMainCircleGraph( work, GRAPH_DISP_MODE_TODAY );    // 円グラフ作成
    SetupMainCircleGraph( work, GRAPH_DISP_MODE_TOTAL );    // 円グラフ作成
    CIRCLE_GRAPH_SetDrawEnable( GetMainGraph(work), TRUE ); // 円グラフ表示開始
    CIRCLE_GRAPH_AnalyzeReq( GetMainGraph(work) );          // 円グラフ表示開始

    IncStateSeq( work );
    break;

  case 1:
    // SE が停止している
    if( PMSND_CheckPlaySE() == FALSE ) { 
      // 一定時間が経過
      if( STATE_ANALYZE_FRAMES <= work->stateCount ) {
        IncStateSeq( work );
      } 
      else { 
        PMSND_PlaySE( SEQ_SE_SYS_81 ); // データ解析中SEをループさせる
      }
    }
    break;

  case 2:
    // 解析済みフラグを立てる
    work->analyzeFlag = TRUE;

    // 表示を更新
    SetMenuCursorPos( work, MENU_ITEM_ANSWER ); // カーソル位置を『回答』に合わせる
    UpdateControlCursor( work );                // 左右カーソルを更新
    UpdateTouchArea( work );                    // タッチ範囲を更新
    ChangeAnswerToTop( work );                  // ランク1位の回答を表示
    HideAnalyzeMessage( work );                 //「…かいせきちゅう…」を消す
    UpdateMainBG_WINDOW( work );                // MAIN-BG ( ウィンドウ面 ) を更新する
    UpdateBGFont_Answer( work );                // 回答を更新する
    UpdateBGFont_MyAnswer( work );              // 自分の回答を更新する
    UpdateBGFont_DataReceiving( work );         //「データしゅとくちゅう」の表示を更新する
    UpdateBGFont_Count( work );                 // 回答人数を更新する
    UpdateArrow( work );                        // 矢印を更新する
    UpdateMyAnswerIconOnButton( work );         // 自分の回答アイコン ( ボタン上 ) を更新する

    // ボタンで解析
    if( work->analyzeByTouchFlag == FALSE ) {
      SetMenuCursorOn( work ); // カーソルが乗っている状態にする
    }

    // 調査結果表示SE
    PMSND_PlaySE( SEQ_SE_SYS_82 );

    FinishCurrentState( work );
    break;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 解析状態 ( RRG_STATE_PERCENTAGE ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainSeq_PERCENTAGE( RRG_WORK* work )
{ 
  switch( GetStateSeq(work) ) {
  case 0:
    SetupPercentages( work ); // ％表示オブジェクトをセットアップ
    IncStateSeq( work );
    break;

  case 1:
    // 新たな％を表示
    if( work->stateCount % 10 == 0 ) {
      DispPercentage( work, work->percentageDispNum++ );
    }

    // 全ての数値を表示したら次の状態へ
    if( work->percentageNum <= work->percentageDispNum ) {
      FinishCurrentState( work );
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 画面フラッシュシーケンス ( RRG_STATE_FLASHOUT ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainSeq_FLASHOUT( RRG_WORK* work )
{
  switch( GetStateSeq(work) ) {
  case 0:
    StopPaletteAnime( work, PALETTE_ANIME_HOLD ); //『報告を見る』ボタンの暗転アニメを停止
    StartPaletteFadeFlashOut( work ); // パレットフェード開始
    IncStateSeq( work );
    break;

  case 1:
    // パレットフェード終了
    if( IsPaletteFadeEnd( work ) ) {
      FinishCurrentState( work );
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 画面フラッシュシーケンス ( RRG_STATE_FLASHIN ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainSeq_FLASHIN( RRG_WORK* work )
{
  switch( GetStateSeq(work) ) {
  case 0:
    // パレットフェード開始
    StartPaletteFadeFlashIn( work );
    IncStateSeq( work );
    break;

  case 1:
    // パレットフェード終了
    if( IsPaletteFadeEnd( work ) ) {
      UpdateAnalyzeButton( work ); //『報告を見る』ボタンを更新する
      FinishCurrentState( work );
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定の確認状態への準備状態 ( RRG_STATE_UPDATE_AT_STANDBY ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainSeq_UPDATE_AT_STANDBY( RRG_WORK* work )
{
  switch( GetStateSeq(work) ) {
  case 0:
    // 更新開始
    work->updateFlag = TRUE;
    ClearNewEntryFlag( work );
    DisableUpdate( work );

    // 調査データを再セットアップ
    SetupResearchData( work );

    // 表示を更新
    UpdateMainBG_WINDOW( work );        // MAIN-BG ( ウィンドウ面 ) を更新する
    UpdateBGFont_Answer( work );        //「回答」文字列の表示を更新する
    UpdateBGFont_DataReceiving( work ); //「データしゅとくちゅう」の表示を更新する
    UpdateBGFont_NoData( work );        //「まだ調査できていません」の表示を更新する
    UpdateArrow( work );                // 矢印の表示を更新する
    UpdateControlCursor( work );        // 左右のカーソル表示を更新する
    VanishAllPercentage( work );        // ％表示を消去する

    // サブ円グラフ作成
    SetupSubCircleGraph( work, GRAPH_DISP_MODE_TODAY );
    SetupSubCircleGraph( work, GRAPH_DISP_MODE_TOTAL );

    // 円グラフの重なり方を調整
    AdjustCircleGraphLayer( work );

    // 円グラフ表示開始
    CIRCLE_GRAPH_SetDrawEnable( GetSubGraph(work), TRUE );
    CIRCLE_GRAPH_UpdateReq( GetSubGraph(work) );

    IncStateSeq( work );
    break;

  case 1:
    // SE が停止している
    if( PMSND_CheckPlaySE() == FALSE ) { 
      // 一定時間が経過
      if( STATE_UPDATE_FRAMES <= work->stateCount ) {
        IncStateSeq( work );
      } 
      else { 
        PMSND_PlaySE( SEQ_SE_SYS_81 ); // データ受信中SEをループさせる
      }
    }
    break;

  case 2:
    // 更新終了
    work->updateFlag = FALSE;

    // 表示を更新
    InterchangeCircleGraph( work ); // サブ円グラフとメイン円グラフを入れ替える
    CIRCLE_GRAPH_SetDrawEnable( GetSubGraph(work), FALSE ); // サブ円グラフ ( 元メイン ) 表示終了
    UpdateMainBG_WINDOW( work );        // MAIN-BG ( ウィンドウ面 ) を更新する
    UpdateBGFont_DataReceiving( work ); //「データしゅとくちゅう」の表示を更新する
    UpdateBGFont_Answer( work );        // 回答を更新する
    UpdateBGFont_MyAnswer( work );      // 自分の回答を更新する
    UpdateBGFont_Count( work );         // 回答人数を更新する
    UpdateArrow( work );                // 矢印を更新する
    UpdateControlCursor( work );        // 左右のカーソル表示を更新する
    UpdateMyAnswerIconOnGraph( work );  // 自分の回答アイコン ( グラフ上 ) を更新する
    UpdateMyAnswerIconOnButton( work ); // 自分の回答アイコン ( ボタン上 ) を更新する

    // 更新完了SE
    PMSND_PlaySE( SEQ_SE_SYS_82 );

    FinishCurrentState( work );
    break;
  } 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定の確認状態への準備状態 ( RRG_STATE_UPDATE_AT_KEYWAIT ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainSeq_UPDATE_AT_KEYWAIT( RRG_WORK* work )
{
  switch( GetStateSeq(work) ) {
  case 0:
    // 更新開始
    work->updateFlag = TRUE;
    ClearNewEntryFlag( work );
    DisableUpdate( work );

    // 調査データを再セットアップ
    SetupResearchData( work );

    // 表示を更新
    SetMenuCursorOff( work );           // カーソルが乗っていない状態にする
    UpdateMainBG_WINDOW( work );        // MAIN-BG ( ウィンドウ面 ) を更新する
    UpdateBGFont_Answer( work );        //「回答」文字列の表示を更新する
    UpdateBGFont_DataReceiving( work ); //「データしゅとくちゅう」の表示を更新する
    UpdateBGFont_NoData( work );        //「まだ調査できていません」の表示を更新する
    UpdateArrow( work );                // 矢印の表示を更新する
    UpdateControlCursor( work );        // 左右のカーソル表示を更新する
    VanishAllPercentage( work );        // ％表示を消去する

    // サブ円グラフ作成
    SetupSubCircleGraph( work, GRAPH_DISP_MODE_TODAY );
    SetupSubCircleGraph( work, GRAPH_DISP_MODE_TOTAL );

    // 円グラフの重なり方を調整
    AdjustCircleGraphLayer( work );

    // 円グラフ表示開始
    CIRCLE_GRAPH_SetDrawEnable( GetSubGraph(work), TRUE );
    CIRCLE_GRAPH_UpdateReq( GetSubGraph(work) );

    IncStateSeq( work );
    break;

  case 1:
    // SE が停止している
    if( PMSND_CheckPlaySE() == FALSE ) { 
      // 一定時間が経過
      if( STATE_UPDATE_FRAMES <= work->stateCount ) {
        IncStateSeq( work );
      } 
      else { 
        PMSND_PlaySE( SEQ_SE_SYS_81 ); // データ受信中SEをループさせる
      }
    }
    break;

  case 2:
    // 更新終了
    work->updateFlag = FALSE;

    // 表示を更新
    InterchangeCircleGraph( work ); // サブ円グラフとメイン円グラフを入れ替える
    CIRCLE_GRAPH_SetDrawEnable( GetSubGraph(work), FALSE ); // サブ円グラフ ( 元メイン ) 表示終了
    UpdateMainBG_WINDOW( work );        // MAIN-BG ( ウィンドウ面 ) を更新する
    SetMenuCursorOn( work );            // カーソルが乗っている状態にする
    UpdateBGFont_DataReceiving( work ); //「データしゅとくちゅう」の表示を更新する
    UpdateBGFont_Answer( work );        // 回答を更新する
    UpdateBGFont_MyAnswer( work );      // 自分の回答を更新する
    UpdateBGFont_Count( work );         // 回答人数を更新する
    UpdateArrow( work );                // 矢印を更新する
    UpdateControlCursor( work );        // 左右のカーソル表示を更新する
    UpdateMyAnswerIconOnGraph( work );  // 自分の回答アイコン ( グラフ上 ) を更新する
    UpdateMyAnswerIconOnButton( work ); // 自分の回答アイコン ( ボタン上 ) を更新する

    // 更新完了SE
    PMSND_PlaySE( SEQ_SE_SYS_82 );

    FinishCurrentState( work );
    break;
  } 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定の確認状態への準備状態 ( RRG_STATE_FADEOUT ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainSeq_FADEOUT( RRG_WORK* work )
{
  switch( GetStateSeq(work) ) {
  case 0:
    // フェードアウト開始
    StartFadeOut();
    IncStateSeq( work );
    break; 

  case 1:
    // フェードが終了
    if( GFL_FADE_CheckFade() == FALSE ) {
      FinishCurrentState( work );
    } 
    break;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定の確認状態への準備状態 ( RRG_STATE_WAIT ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainSeq_WAIT( RRG_WORK* work )
{
  // 待ち時間が経過
  if( GetWaitFrame(work) < work->stateCount ) {
    FinishCurrentState( work );
  } 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 後片付け状態 ( RRG_STATE_CLEANUP ) の処理
 *
 * @param work
 *
 * @return 状態が変化する場合 次の状態番号
 *         状態が継続する場合 現在の状態番号
 */
//-----------------------------------------------------------------------------------------
static void MainSeq_CLEANUP( RRG_WORK* work )
{ 
  // パレットアニメーションワーク
  CleanUpPaletteAnime( work );
  DeletePaletteAnime( work );

  // 共通パレットアニメーション
  RRC_StopAllPaletteAnime( work->commonWork ); // 停止して, 
  RRC_ResetAllPalette( work->commonWork );     // パレットを元に戻す

  ReleaseVBlankTask( work );        // VBlankタスクを解除
  DeletePercentage( work );         // % 表示オブジェクト破棄
  DeleteArrow( work );              // 矢印 削除
  DeleteCircleGraph( work );        // 円グラフ 削除
  CleanUpPaletteFadeSystem( work ); // パレットフェードシステム 後片付け

  // BMP-OAM 後片付け
  DeleteBitmapDatas( work );
  DeleteBmpOamActors( work );
  CleanUpBmpOamSystem( work );

  // OBJ 後片付け
  DeleteClactWorks( work );
  DeleteClactUnits( work );
  ReleaseSubObjResources ( work );
  ReleaseMainObjResources( work );

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
  SetFinishResult( work, RRG_RESULT_TO_TOP );  

  // 状態終了
  RegisterNextState( work, RRG_STATE_FINISH );
  FinishCurrentState( work );

  // 侵入接続を許可
  RecoverIntrudeShutdown( work );

  // グラフ画面終了
  work->setupFlag  = FALSE;
  work->finishFlag = TRUE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在の状態を終了する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void FinishCurrentState( RRG_WORK* work )
{
  // すでに終了済み
  GF_ASSERT( work->stateEndFlag == FALSE );

  // 終了フラグを立てる
  work->stateEndFlag = TRUE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief グラフ画面の終了理由を登録する
 *
 * @param work
 * @param reason 終了の理由
 */
//-----------------------------------------------------------------------------------------
static void SetFinishReason( RRG_WORK* work, SEQ_CHANGE_TRIG reason )
{
  RRC_SetSeqChangeTrig( work->commonWork, reason );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 画面の終了結果を決定する
 *
 * @param work
 * @param result 結果
 */
//-----------------------------------------------------------------------------------------
static void SetFinishResult( RRG_WORK* work, RRG_RESULT result )
{
  work->finishResult = result;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief フレーム経過待ち状態の待ち時間を設定する
 *
 * @param work
 * @param frame 設定する待ちフレーム数
 */
//-----------------------------------------------------------------------------------------
static void SetWaitFrame( RRG_WORK* work, u32 frame )
{
  work->waitFrame = frame;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief フレーム経過待ち状態の待ち時間を取得する
 *
 * @param work
 *
 * @return 待ちフレーム数
 */
//-----------------------------------------------------------------------------------------
static u32 GetWaitFrame( const RRG_WORK* work )
{
  return work->waitFrame;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 最初の状態を取得する    
 *
 * @param work
 *
 * @return セットアップ後の最初の状態
 */
//-----------------------------------------------------------------------------------------
RRG_STATE GetFirstState( const RRG_WORK* work )
{
  RRC_WORK* commonWork;
  RADAR_SEQ prev_seq;
  SEQ_CHANGE_TRIG trig;

  commonWork = work->commonWork;
  prev_seq   = RRC_GetPrevSeq( commonWork );
  trig       = RRC_GetSeqChangeTrig( commonWork );

  // 前の画面をボタンで終了
  if( (prev_seq != RADAR_SEQ_NULL) && (trig == SEQ_CHANGE_BY_BUTTON) ) {
    return RRG_STATE_KEYWAIT;
  }
  else {
    return RRG_STATE_STANDBY;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態カウンタを更新する
 * 
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CountUpStateCount( RRG_WORK* work )
{
  (work->stateCount)++;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 次の状態を登録する
 *
 * @param work
 * @param nextSeq 登録する状態
 */
//-----------------------------------------------------------------------------------------
static void RegisterNextState( RRG_WORK* work, RRG_STATE nextSeq )
{
  // 状態キューに追加する
  QUEUE_EnQueue( work->stateQueue, nextSeq );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態を変更する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SwitchState( RRG_WORK* work )
{
  RRG_STATE nextSeq;

  if( work->stateEndFlag == FALSE ){ return; }  // 現在の状態が終了していない
  if( QUEUE_IsEmpty( work->stateQueue ) ){ return; } // 状態キューに登録されていない

  // 状態を変更
  nextSeq = QUEUE_DeQueue( work->stateQueue );
  SetState( work, nextSeq ); 
} 

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態を設定する
 *
 * @param work
 * @parma nextSeq 設定する状態
 */
//-----------------------------------------------------------------------------------------
static void SetState( RRG_WORK* work, RRG_STATE nextSeq )
{ 
  work->state        = nextSeq;
  work->stateSeq     = 0;
  work->stateCount   = 0;
  work->stateEndFlag = FALSE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態内シーケンス番号を取得する
 *
 * @param work
 *
 * @return 状態内シーケンス番号
 */
//-----------------------------------------------------------------------------------------
static u32 GetStateSeq( const RRG_WORK* work )
{
  return work->stateSeq;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態内シーケンス番号をインクリメントする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void IncStateSeq( RRG_WORK* work )
{
  (work->stateSeq)++;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態内シーケンス番号をリセットする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ResetStateSeq( RRG_WORK* work )
{
  work->stateSeq = 0;
}


//=========================================================================================
// ■LAYER 5 機能
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを上へ移動する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MoveMenuCursorUp( RRG_WORK* work )
{ 
  BOOL loop = TRUE;

  // 表示を更新
  SetMenuCursorOff( work );  // カーソルが乗っていない状態にする

  // カーソル移動
  ShiftMenuCursorPos( work, -1 );

  // カーソル位置を調整
  while( loop )
  {
    //「回答」or「自分の回答」
    if( (work->cursorPos == MENU_ITEM_ANSWER) || (work->cursorPos == MENU_ITEM_MY_ANSWER) ) {
      // 未解析 or「ただいま調査中」
      if( (work->analyzeFlag == FALSE) || (GetCountOfQuestion(work) == 0) ) {
        ShiftMenuCursorPos( work, -1 ); 
      }
      else {
        loop = FALSE;
      }
    }
    //『報告を見る』ボタン
    else if( work->cursorPos == MENU_ITEM_ANALYZE ) {
      // 解析済み or「ただいま調査中」
      if( (work->analyzeFlag == TRUE) || (GetCountOfQuestion(work) == 0) ) {
        ShiftMenuCursorPos( work, -1 ); 
      }
      else {
        loop = FALSE;
      }
    }
    else {
      loop = FALSE;
    }
  }

  // 表示を更新
  SetMenuCursorOn( work );      // カーソルが乗っている状態にする
  UpdateControlCursor( work );  // 左右カーソルを更新
  UpdateMyAnswerIconOnGraph( work );  // 自分の回答アイコン ( グラフ上 ) を更新する
  StartPaletteAnime( work, PALETTE_ANIME_CURSOR_SET ); // カーソルセットのパレットアニメを開始する

  if( work->cursorPos == MENU_ITEM_ANALYZE ) {
    SetAnalyzeButtonCursorSet( work ); //『報告を見る』ボタンのカーソルセットアニメを開始する
  }

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );

  // タッチ範囲を更新
  UpdateTouchArea( work );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを下へ移動する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MoveMenuCursorDown( RRG_WORK* work )
{
  BOOL loop = TRUE;

  // 表示を更新
  SetMenuCursorOff( work );  // カーソルが乗っていない状態にする

  // カーソル移動
  ShiftMenuCursorPos( work, 1 );

  // カーソル位置を調整
  while( loop )
  {
    //「回答」or「自分の回答」
    if( (work->cursorPos == MENU_ITEM_ANSWER) || (work->cursorPos == MENU_ITEM_MY_ANSWER) ) {
      // 未解析 or「ただいま調査中」
      if( (work->analyzeFlag == FALSE) || (GetCountOfQuestion(work) == 0) ) {
        ShiftMenuCursorPos( work, 1 ); 
      }
      else {
        loop = FALSE;
      }
    }
    //『報告を見る』ボタン
    else if( work->cursorPos == MENU_ITEM_ANALYZE ) {
      // 解析済み or「ただいま調査中」
      if( (work->analyzeFlag == TRUE) || (GetCountOfQuestion(work) == 0) ) {
        ShiftMenuCursorPos( work, 1 ); 
      }
      else {
        loop = FALSE;
      }
    }
    else {
      loop = FALSE;
    }
  }

  // 表示を更新
  SetMenuCursorOn( work );      // カーソルが乗っている状態にする
  UpdateControlCursor( work );  // 左右カーソルを更新
  UpdateMyAnswerIconOnGraph( work );  // 自分の回答アイコン ( グラフ上 ) を更新する
  StartPaletteAnime( work, PALETTE_ANIME_CURSOR_SET ); // カーソルセットのパレットアニメを開始する

  if( work->cursorPos == MENU_ITEM_ANALYZE ) {
    SetAnalyzeButtonCursorSet( work ); //『報告を見る』ボタンのカーソルセットアニメを開始する
  }

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );

  // タッチ範囲を更新
  UpdateTouchArea( work );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを直接移動する
 *
 * @param work
 * @param menuItem 移動先のメニュー項目
 */
//-----------------------------------------------------------------------------------------
static void MoveMenuCursorDirect( RRG_WORK* work, MENU_ITEM menuItem )
{
  MoveMenuCursorSilent( work, menuItem );

  // カーソルセットのパレットアニメを開始する
  StartPaletteAnime( work, PALETTE_ANIME_CURSOR_SET ); 

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 ); 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを直接移動する ( 点滅・SEなし )
 *
 * @param work
 * @param menuItem 移動先のメニュー項目
 */
//-----------------------------------------------------------------------------------------
static void MoveMenuCursorSilent( RRG_WORK* work, MENU_ITEM menuItem )
{
  // 移動先が「回答」「自分の回答」の場合
  if( (menuItem == MENU_ITEM_ANSWER) || (menuItem == MENU_ITEM_MY_ANSWER) )
  {
    if( (work->analyzeFlag == FALSE) ||   // 未解析
        (GetCountOfQuestion(work) == 0) ) //「ただいま ちょうさちゅう」
    { // 移動を受け付けない
      return;
    }
  }

  // 表示を更新
  SetMenuCursorOff( work );  // カーソルが乗っていない状態にする

  // カーソル移動
  SetMenuCursorPos( work, menuItem );

  // 表示を更新
  SetMenuCursorOn( work );      // カーソルが乗っている状態にする
  UpdateControlCursor( work );  // 左右カーソルを更新
  UpdateMyAnswerIconOnGraph( work );  // 自分の回答アイコン ( グラフ上 ) を更新する

  // タッチ範囲を更新
  UpdateTouchArea( work );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 表示する質問を, 次の質問に変更する
 *
 * @param work
 */
//------------------------------------------------------------------------------------------
static void ChangeQuestionToNext( RRG_WORK* work )
{
  // 表示中の円グラフを消去する
  if( work->analyzeFlag && GetCountOfQuestion(work) != 0 ) {
    CIRCLE_GRAPH_DisappearReq( GetMainGraph(work) );
  }

  // データを更新
  SetupResearchData( work );   // 調査データを再セットアップ
  ClearNewEntryFlag( work );   // この時点でデータは最新
  ShiftQuestionIdx( work, 1 ); // 表示する質問インデックスを変更
  work->analyzeFlag = FALSE;   // 解析済みフラグを伏せる

  // 表示を更新
  UpdateMainBG_WINDOW( work );          // MAIN-BG ( ウィンドウ面 ) を更新する
  SetMenuCursorOn( work );              // カーソルが乗っている状態にする
  UpdateBGFont_QuestionCaption( work ); // 質問の補足文を更新する
  UpdateBGFont_Question( work );        // 質問を更新する 
  ChangeAnswerToFirst( work );          // 回答を更新する
  UpdateBGFont_MyAnswer( work );        // 自分の回答を更新する
  UpdateBGFont_Count( work );           // 回答人数を更新する
  UpdateBGFont_NoData( work );          //「ただいま ちょうさちゅう」の表示を更新する
  VanishAllPercentage( work );          // ％表示を全消去
  UpdateMyAnswerIconOnButton( work );   // 自分の回答アイコン ( ボタン上 ) を更新する
  UpdateAnalyzeButton( work );          //『報告を見る』ボタンを更新する

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 表示する質問を, 前の質問に変更する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ChangeQuestionToPrev( RRG_WORK* work )
{
  // 表示中の円グラフを消去する
  if( work->analyzeFlag && GetCountOfQuestion(work) != 0 ) {
    CIRCLE_GRAPH_DisappearReq( GetMainGraph(work) );
  }

  // データを更新
  SetupResearchData( work );    // 調査データを再セットアップ
  ClearNewEntryFlag( work );    // この時点でデータは最新
  ShiftQuestionIdx( work, -1 ); // 表示する質問インデックスを変更
  work->analyzeFlag = FALSE;    // 解析済みフラグを伏せる

  // 表示を更新
  UpdateMainBG_WINDOW( work );          // MAIN-BG ( ウィンドウ面 ) を更新する
  SetMenuCursorOn( work );              // カーソルが乗っている状態にする
  UpdateBGFont_QuestionCaption( work ); // 質問の補足文を更新する
  UpdateBGFont_Question( work );        // 質問を更新する 
  ChangeAnswerToFirst( work );          // 回答を更新する
  UpdateBGFont_MyAnswer( work );        // 自分の回答を更新する
  UpdateBGFont_Count( work );           // 回答人数を更新する
  UpdateBGFont_NoData( work );          //「ただいま ちょうさちゅう」の表示を更新する
  VanishAllPercentage( work );          // ％表示を全消去
  UpdateMyAnswerIconOnButton( work );   // 自分の回答アイコン ( ボタン上 ) を更新する
  UpdateAnalyzeButton( work );          //『報告を見る』ボタンを更新する

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 表示する回答を, 次の回答に変更する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ChangeAnswerToNext( RRG_WORK* work )
{
  ShiftAnswerIdx( work, 1 ); // 表示する回答インデックスを変更

  // 表示を更新
  UpdateBGFont_Answer( work ); // 回答を更新する
  UpdateArrow( work );         // 矢印を更新

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 表示する回答を, 前の回答に変更する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ChangeAnswerToPrev( RRG_WORK* work )
{
  ShiftAnswerIdx( work, -1 ); // 表示する回答インデックスを変更

  // 表示を更新
  UpdateBGFont_Answer( work ); // 回答を更新する
  UpdateArrow( work );         // 矢印を更新

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 表示する回答を, 先頭の回答に変更する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ChangeAnswerToFirst( RRG_WORK* work )
{
  ResetAnswerIdx( work ); // 表示する回答インデックスをリセット

  // 表示を更新
  UpdateBGFont_Answer( work ); // 回答を更新する
  UpdateArrow( work );         // 矢印を更新
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 表示する回答を, ランク1位の回答に変更する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ChangeAnswerToTop( RRG_WORK* work )
{
  SetAnswerIdxToTop( work ); 

  // 表示を更新
  UpdateBGFont_Answer( work ); // 回答を更新する
  UpdateArrow( work );         // 矢印を更新
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査データの表示タイプを切り替える
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SwitchDataDisplayType( RRG_WORK* work )
{
  CIRCLE_GRAPH* graph;
  GRAPH_DISP_MODE nextType;
  BOOL newGraphWait = FALSE; // 新しく表示する円グラフに, 表示するまでの待ち時間が必要かどうか

  // 円グラフのアニメーション中は受け付けない
  if( CheckAllGraphAnimeEnd(work) == FALSE ) { return; }

  // 表示中の円グラフを消去する
  if( work->analyzeFlag && GetCountOfQuestion(work) != 0 ) {
    CIRCLE_GRAPH_DisappearReq( GetMainGraph(work) );
    newGraphWait = TRUE;
  }

  // 表示タイプを変更
  switch( work->graphMode ) {
  case GRAPH_DISP_MODE_TODAY: nextType = GRAPH_DISP_MODE_TOTAL; break;
  case GRAPH_DISP_MODE_TOTAL: nextType = GRAPH_DISP_MODE_TODAY; break;
  default: GF_ASSERT(0);
  } 
  SetDataDisplayType( work, nextType );

  // 表示を更新
  UpdateMainBG_WINDOW( work );        // MAIN-BG ( ウィンドウ面 ) を更新
  SetMenuCursorOn( work );            // カーソルが乗っている状態にする
  UpdateBGFont_Answer( work );        // 回答を更新する
  UpdateBGFont_MyAnswer( work );      // 自分の回答を更新する
  UpdateBGFont_Count( work );         // 回答人数を更新する
  UpdateBGFont_NoData( work );        //「ただいま ちょうさちゅう」の表示を更新する
  UpdateBGFont_DataReceiving( work ); //「データしゅとくちゅう」の表示を更新する
  UpdateMyAnswerIconOnButton( work ); // 自分の回答アイコン ( ボタン上 ) の表示を更新する
  UpdateAnalyzeButton( work );        //「報告を見る」ボタンを更新する
  UpdateArrow( work );                // 矢印を更新する
  VanishAllPercentage( work );        // ％オブジェクトを全消去

  // 解析済みの場合のみの更新
  if( work->analyzeFlag && GetCountOfQuestion(work) != 0 ) {
    SetupPercentages( work );  // ％オブジェクトを再セットアップ
    DispAllPercentage( work ); // ％オブジェクトを全表示
  }

  // 円グラフを出現させる
  if( work->analyzeFlag && GetCountOfQuestion(work) != 0 ) {
    if( newGraphWait ) { CIRCLE_GRAPH_StopGraph( GetMainGraph(work), 20 ); } // 前グラフの消去を待つ
    CIRCLE_GRAPH_AppearReq( GetMainGraph(work) );
    CIRCLE_GRAPH_SetDrawEnable( GetMainGraph(work), TRUE );
  }

  // カーソル移動音
  PMSND_PlaySE( SEQ_SE_SELECT1 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 画面のフェードインを開始する
 */
//-----------------------------------------------------------------------------------------
static void StartFadeIn( void )
{
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 
      16, 0, 0);
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 画面のフェードアウトを開始する
 */
//-----------------------------------------------------------------------------------------
static void StartFadeOut( void )
{
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 
      0, 16, 0);
}


//=========================================================================================
// ◇LAYER 4 個別操作
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを移動する
 *
 * @parma work
 * @param stride 移動量
 */
//-----------------------------------------------------------------------------------------
static void ShiftMenuCursorPos( RRG_WORK* work, int stride )
{
  int nowPos;
  int nextPos;

  // カーソル位置を更新
  nowPos  = work->cursorPos;
  nextPos = (nowPos + stride + MENU_ITEM_NUM) % MENU_ITEM_NUM;
  work->cursorPos = nextPos; 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを直接指定して変更する
 *
 * @param work
 * @param menuItem 変更後のカーソル位置
 */
//-----------------------------------------------------------------------------------------
static void SetMenuCursorPos( RRG_WORK* work, MENU_ITEM menuItem )
{
  // カーソル位置を更新
  work->cursorPos = menuItem;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief カーソル位置にあるメニュー項目を, カーソルが乗っている状態にする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetMenuCursorOn( RRG_WORK* work )
{
  // 該当するスクリーンを更新
  switch( work->cursorPos ) {
  //「質問」「自分の回答」「回答人数」
  case MENU_ITEM_QUESTION:  
  case MENU_ITEM_MY_ANSWER: 
  case MENU_ITEM_COUNT:     
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

  //『報告を見る』ボタン
  case MENU_ITEM_ANALYZE:
    SetAnalyzeButtonCursorOn( work );
    break;

  // エラー
  default:
    GF_ASSERT(0);
  }

  // スクリーン転送リクエスト
  GFL_BG_LoadScreenReq( MAIN_BG_WINDOW ); 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief カーソル位置にあるメニュー項目を, メニュー項目をカーソルが乗っていない状態にする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetMenuCursorOff( RRG_WORK* work )
{
  // 該当するスクリーンを更新
  switch( work->cursorPos ) {
  //「質問」「自分の回答」「回答人数」
  case MENU_ITEM_QUESTION: 
  case MENU_ITEM_MY_ANSWER: 
  case MENU_ITEM_COUNT:
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

  //『報告を見る』ボタン
  case MENU_ITEM_ANALYZE:
    SetAnalyzeButtonCursorOff( work );
    break;

  // エラー
  default:
    GF_ASSERT(0);
  }

  // スクリーン転送リクエスト
  GFL_BG_LoadScreenReq( MAIN_BG_WINDOW );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 表示する質問のインデックスを変更する
 *
 * @param work
 * @param stride 移動量
 */
//-----------------------------------------------------------------------------------------
static void ShiftQuestionIdx( RRG_WORK* work, int stride )
{
  int nowIdx;
  int nextIdx;

  // インデックスを変更
  nowIdx  = work->questionIdx;
  nextIdx = ( nowIdx + stride + QUESTION_NUM_PER_TOPIC ) % QUESTION_NUM_PER_TOPIC;
  work->questionIdx = nextIdx;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 選択中の回答インデックスをランク1位の項目にセットする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetAnswerIdxToTop( RRG_WORK* work )
{
  CIRCLE_GRAPH* graph;
  int questionID;
  int answerID;

  graph      = GetMainGraph( work );
  questionID = GetQuestionID( work );
  answerID   = CIRCLE_GRAPH_GetComponentID_byRank( graph, 0 );

  work->answerIdx = 
    RESEARCH_DATA_GetAnswerIndex_byID( &(work->researchData), questionID, answerID );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 選択中の回答インデックスを変更する
 *
 * @param work
 * @param stride ランク変更量
 */
//-----------------------------------------------------------------------------------------
static void ShiftAnswerIdx( RRG_WORK* work, int stride )
{
  int questionID;
  int answerID, nextAnswerID;
  int answerRank, nextAnswerRank;
  int answerNum;
  int nextAnswerIdx;
  CIRCLE_GRAPH* graph;
  
  // 基本情報を取得
  graph      = GetMainGraph( work );
  questionID = GetQuestionID( work );
  answerNum  = GetAnswerNum( work );

  // 表示中の回答のランクを取得
  answerID   = GetAnswerID( work );
  answerRank = CIRCLE_GRAPH_GetComponentRank_byID( graph, answerID );

  // 更新後のランクの回答インデックスを取得
  nextAnswerRank = (answerRank + stride + answerNum) % answerNum;
  nextAnswerID   = CIRCLE_GRAPH_GetComponentID_byRank( graph, nextAnswerRank );
  nextAnswerIdx  = RESEARCH_DATA_GetAnswerIndex_byID( &(work->researchData), questionID, nextAnswerID );

  // インデックスを変更
  work->answerIdx = nextAnswerIdx;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 選択中の回答インデックスをリセットする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ResetAnswerIdx( RRG_WORK* work )
{
  work->answerIdx = 0;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 『報告を見る』ボタンを更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateAnalyzeButton( RRG_WORK* work )
{
  // ボタンが押せない
  if( (work->analyzeFlag == TRUE) || // 解析済み
      (work->updateFlag == TRUE) || // 更新中
      (GetCountOfQuestion(work) == 0) ) { //「ただいま調査中」
    // 暗転していない
    if( CheckPaletteAnime( work, PALETTE_ANIME_HOLD ) == FALSE ) {
      SetAnalyzeButtonNotActive( work );
    }
  }
  // ボタンが押せる
  else {
    // 暗転している
    if( CheckPaletteAnime( work, PALETTE_ANIME_HOLD ) == TRUE ) {
      SetAnalyzeButtonActive( work );
    }
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 『報告を見る』ボタンを点滅させる 
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void BlinkAnalyzeButton( RRG_WORK* work )
{
  // 適用するパレット番号を変更
  BmpOam_ActorSetPaletteOffset( work->BmpOamActor[ BMPOAM_ACTOR_ANALYZE_BUTTON ], 0 );

  StopPaletteAnime( work, PALETTE_ANIME_HOLD );
  StopPaletteAnime( work, PALETTE_ANIME_RECOVER );
  StartPaletteAnime( work, PALETTE_ANIME_SELECT );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 『報告を見る』ボタンをカーソルが乗っている状態にする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetAnalyzeButtonCursorOn( RRG_WORK* work )
{
  // 適用するパレット番号を変更
  BmpOam_ActorSetPaletteOffset( work->BmpOamActor[ BMPOAM_ACTOR_ANALYZE_BUTTON ], 0 );

  // パレットアニメーション開始
  StartPaletteAnime( work, PALETTE_ANIME_ANALYZE_CURSOR_ON );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 『報告を見る』ボタンをカーソルが乗っていない状態にする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetAnalyzeButtonCursorOff( RRG_WORK* work )
{
  // 適用するパレット番号を変更
  BmpOam_ActorSetPaletteOffset( work->BmpOamActor[ BMPOAM_ACTOR_ANALYZE_BUTTON ], 1 );

  // パレットアニメーション停止
  StopPaletteAnime( work, PALETTE_ANIME_ANALYZE_CURSOR_ON );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 『報告を見る』ボタンをカーソルをセットした状態にする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetAnalyzeButtonCursorSet( RRG_WORK* work )
{
  // 適用するパレット番号を変更
  BmpOam_ActorSetPaletteOffset( work->BmpOamActor[ BMPOAM_ACTOR_ANALYZE_BUTTON ], 0 );

  // パレットアニメーション開始
  StartPaletteAnime( work, PALETTE_ANIME_ANALYZE_CURSOR_SET );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 『報告を見る』ボタンをアクティブ状態にする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetAnalyzeButtonActive( RRG_WORK* work )
{
  // 適用するパレット番号を変更
  BmpOam_ActorSetPaletteOffset( work->BmpOamActor[ BMPOAM_ACTOR_ANALYZE_BUTTON ], 1 );

  // 暗転を終了
  StopPaletteAnime( work, PALETTE_ANIME_HOLD ); 

  // 復帰させる
  StartPaletteAnime( work, PALETTE_ANIME_RECOVER ); 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 『報告を見る』ボタンを非アクティブ状態にする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetAnalyzeButtonNotActive( RRG_WORK* work )
{
  // 適用するパレット番号を変更
  BmpOam_ActorSetPaletteOffset( work->BmpOamActor[ BMPOAM_ACTOR_ANALYZE_BUTTON ], 1 );

  // 暗転させる
  StartPaletteAnime( work, PALETTE_ANIME_HOLD ); 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief「戻る」ボタンを点滅させる 
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void BlinkReturnButton( RRG_WORK* work )
{
  RRC_StartPaletteAnime( work->commonWork, COMMON_PALETTE_ANIME_RETURN );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief サブ円グラフとメイン円グラフを入れ替える
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InterchangeCircleGraph( RRG_WORK* work )
{
  int typeIdx;
  CIRCLE_GRAPH* temp;

  for( typeIdx=0; typeIdx < GRAPH_DISP_MODE_NUM; typeIdx++ )
  {
    CIRCLE_GRAPH* temp         = work->mainGraph[ typeIdx ];
    work->mainGraph[ typeIdx ] = work->subGraph[ typeIdx ];
    work->subGraph[ typeIdx ]  = temp;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @breif グラフ構成要素の中心点カラーをセットアップする
 *
 * @param component グラフ構成要素の追加データ
 */
//-----------------------------------------------------------------------------------------
static void SetupCenterColorOfGraphComponent( GRAPH_COMPONENT_ADD_DATA* component )
{
  const int min = 0; // カラー成分の最小値
  const int max = 31; // カラー成分の最大値
  const int brightness = 3; // 中心点カラーの明るさ
  int R, G, B;
  int cR, cG, cB;

  // 外周の色を取得
  R = component->outerColorR;
  G = component->outerColorG;
  B = component->outerColorB;

  // 値域チェック
  GF_ASSERT( min<=R && R<=max );
  GF_ASSERT( min<=G && G<=max );
  GF_ASSERT( min<=B && B<=max );

  // 中心点の色を決定
  cR = (R + max * (brightness - 1)) / brightness;
  cG = (G + max * (brightness - 1)) / brightness;
  cB = (B + max * (brightness - 1)) / brightness;

  // 値域チェック
  GF_ASSERT( min<=cR && cR<=max );
  GF_ASSERT( min<=cG && cG<=max );
  GF_ASSERT( min<=cB && cB<=max );

  // 中心の色をセット
  component->centerColorR = cR;
  component->centerColorG = cG;
  component->centerColorB = cB;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メイン円グラフ, サブ円グラフの重なり方を調整する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void AdjustCircleGraphLayer( RRG_WORK* work )
{
  // サブ円グラフが手前に表示されるようにz座標を設定する
  CIRCLE_GRAPH_SetCenterZ( GetMainGraph(work), FX16_CONST(-2.0f) );
  CIRCLE_GRAPH_SetCenterZ( GetSubGraph(work), FX16_CONST(0.0f) );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メイン円グラフを現在のデータで構成する
 *
 * @param work
 * @param graphMode 表示タイプ
 */
//-----------------------------------------------------------------------------------------
static void SetupMainCircleGraph( RRG_WORK* work, GRAPH_DISP_MODE graphMode )
{
  int aIdx;
  CIRCLE_GRAPH* graph;
  GRAPH_COMPONENT_ADD_DATA components[ MAX_ANSWER_NUM_PER_QUESTION ];
  u8 answerNum;
  const QUESTION_DATA* questionData;
  const ANSWER_DATA* answerData;

  graph        = work->mainGraph[ graphMode ];
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
    SetupCenterColorOfGraphComponent( &components[ aIdx ] );
    switch( graphMode ) {
    case GRAPH_DISP_MODE_TODAY: components[ aIdx ].value = answerData->todayCount; break;
    case GRAPH_DISP_MODE_TOTAL: components[ aIdx ].value = answerData->totalCount; break;
    default: GF_ASSERT(0);
    }
  }

  // グラフの構成要素をセット
  CIRCLE_GRAPH_SetupComponents( graph, components, answerNum );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief サブ円グラフを現在のデータで構成する
 *
 * @param work
 * @param graphMode 表示タイプ
 */
//-----------------------------------------------------------------------------------------
static void SetupSubCircleGraph( RRG_WORK* work, GRAPH_DISP_MODE graphMode )
{
  int aIdx;
  CIRCLE_GRAPH* graph;
  GRAPH_COMPONENT_ADD_DATA components[ MAX_ANSWER_NUM_PER_QUESTION ];
  u8 answerNum;
  const QUESTION_DATA* questionData;
  const ANSWER_DATA* answerData;

  graph        = work->subGraph[ graphMode ];
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
    SetupCenterColorOfGraphComponent( &components[ aIdx ] );
    switch( graphMode ) {
    case GRAPH_DISP_MODE_TODAY: components[ aIdx ].value = answerData->todayCount; break;
    case GRAPH_DISP_MODE_TOTAL: components[ aIdx ].value = answerData->totalCount; break;
    default: GF_ASSERT(0);
    }
  }

  // グラフの構成要素をセット
  CIRCLE_GRAPH_SetupComponents( graph, components, answerNum );
} 

//-----------------------------------------------------------------------------------------
/**
 * @brief すべての円グラフを更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateCircleGraphs( RRG_WORK* work )
{
  int typeIdx;

  for( typeIdx=0; typeIdx < GRAPH_DISP_MODE_NUM; typeIdx++ )
  {
    CIRCLE_GRAPH_Main( work->mainGraph[ typeIdx ] );
    CIRCLE_GRAPH_Main( work->subGraph[ typeIdx ] );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief すべての円グラフを更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DrawCircleGraphs( const RRG_WORK* work )
{
  int typeIdx;

  for( typeIdx=0; typeIdx < GRAPH_DISP_MODE_NUM; typeIdx++ )
  {
    CIRCLE_GRAPH_Draw( work->mainGraph[ typeIdx ] );
    CIRCLE_GRAPH_Draw( work->subGraph[ typeIdx ] );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 回答マーカーを描画する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DrawAnswerMarker( const RRG_WORK* work )
{
  int colorR, colorG, colorB;
  GXRgb color;
  const RESEARCH_DATA* data;

  // 未解析 or 更新中 or 調査人数０なら表示しない
  if( (work->analyzeFlag == FALSE) || 
      (work->updateFlag == TRUE) || 
      (GetCountOfQuestion(work) == 0) ) {
    return; 
  }

  // 表示中の回答のカラーを取得
  data = &(work->researchData);
  colorR = RESEARCH_DATA_GetColorR( data, work->questionIdx, work->answerIdx );
  colorG = RESEARCH_DATA_GetColorG( data, work->questionIdx, work->answerIdx );
  colorB = RESEARCH_DATA_GetColorB( data, work->questionIdx, work->answerIdx );
  color = GX_RGB( colorR, colorG, colorB );

  // ポリゴン関連属性値を設定
  G3_PolygonAttr( GX_LIGHTMASK_NONE,        // no lights
                  GX_POLYGONMODE_MODULATE,  // modulation mode
                  GX_CULL_NONE,             // cull none
                  0,                        // polygon ID(0 - 63)
                  31,                       // alpha(0 - 31)
                  0 );                      // OR of GXPolygonAttrMisc's value

  // 描画ポリゴンを登録
  G3_Begin( GX_BEGIN_QUADS ); // 四角形ポリゴン
  G3_Color( color ); // 頂点カラー
  G3_Vtx( ANSWER_MARKER_LEFT,  ANSWER_MARKER_TOP,    ANSWER_MARKER_Z ); // 頂点1
  G3_Vtx( ANSWER_MARKER_LEFT,  ANSWER_MARKER_BOTTOM, ANSWER_MARKER_Z ); // 頂点2
  G3_Vtx( ANSWER_MARKER_RIGHT, ANSWER_MARKER_BOTTOM, ANSWER_MARKER_Z ); // 頂点3
  G3_Vtx( ANSWER_MARKER_RIGHT, ANSWER_MARKER_TOP,    ANSWER_MARKER_Z ); // 頂点4 
  G3_End();
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 矢印の表示を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateArrow( RRG_WORK* work )
{
  // 消去
  ARROW_Vanish( work->arrow );

  // 更新中は表示しない
  if( work->updateFlag == TRUE ) { return; }

  // 未解析なら表示しない
  if( !work->analyzeFlag ) { return; }

  // 調査人数がゼロなら表示しない
  if( GetCountOfQuestion(work) == 0 ) { return; }

  // 対象が0%なら表示しない
  if( CIRCLE_GRAPH_GetComponentPercentage_byID( GetMainGraph(work), GetAnswerID(work) ) == 0 ) { return; }

  // 表示開始
  if( work->analyzeFlag ) {
    int endX, endY;
    CIRCLE_GRAPH_GetComponentPointPos_byID( GetMainGraph(work), GetAnswerID(work), &endX, &endY );
    ARROW_Setup( work->arrow, ARROW_START_X, ARROW_START_Y, endX, endY );
    ARROW_StartAnime( work->arrow );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ％表示オブジェクトを, 現在の調査データにあわせてセットアップする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupPercentages( RRG_WORK* work )
{
  int rank;
  int answerNum;
  int percentageCount;
  CIRCLE_GRAPH* graph;

  answerNum = GetAnswerNum( work );
  graph = GetMainGraph( work );
  percentageCount = 0;

  // ランクが低い構成要素から見ていく
  for( rank=answerNum-1; 0 <= rank; rank-- )
  {
    int x, y, num;
    PERCENTAGE* percentage;

    // エラーチェック
    GF_ASSERT( percentageCount < PERCENTAGE_NUM ); // 表示できる最大数をオーバー

    // 数値・座標を取得
    num = CIRCLE_GRAPH_GetComponentPercentage_byRank( graph, rank );
    CIRCLE_GRAPH_GetComponentPointPos_byRank( graph, rank, &x, &y );

    // 一定値未満の要素は無視
    if( num < MIN_PERCENTAGE ) { continue; }

    // 数値・座標を設定
    percentage = work->percentage[ percentageCount ];
    PERCENTAGE_SetValue( percentage, num );
    PERCENTAGE_SetPos( percentage, x, y );

    // 有効な％オブジェクトの数をカウント
    percentageCount++;
  }

  // 有効な％オブジェクトの数を記憶
  work->percentageNum = percentageCount;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 全ての％表示を消去する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void VanishAllPercentage( RRG_WORK* work )
{
  int idx;

  // すべて非表示にする
  for( idx=0; idx < PERCENTAGE_NUM; idx++ )
  {
    PERCENTAGE_SetDrawEnable( work->percentage[ idx ], FALSE );
  }
  work->percentageDispNum = 0;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ％オブジェクトを表示する
 *
 * @param work
 * @param index 表示する％オブジェクトのインデックス
 */
//-----------------------------------------------------------------------------------------
static void DispPercentage( RRG_WORK* work, u8 index )
{
  PERCENTAGE_SetDrawEnable( work->percentage[ index ], TRUE );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 全ての％オブジェクトを表示する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DispAllPercentage( RRG_WORK* work )
{
  int idx;
  int num;

  num = work->percentageNum;

  for( idx=0; idx < num; idx++ )
  {
    DispPercentage( work, idx );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief タッチ範囲を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateTouchArea( RRG_WORK* work )
{
  const MENU_ITEM_DRAW_PARAM* menu;
  GFL_UI_TP_HITTBL* area;

  // 左カーソル
  menu = &( MenuItemDrawParam[ work->cursorPos ] );
  area = &( work->touchHitTable[ TOUCH_AREA_CONTROL_CURSOR_L ] );
  area->rect.left   = Bind_u8( menu->left_dot + menu->leftCursorOffsetX - CLWK_CONTROL_CURSOR_L_WIDTH/2 );
  area->rect.top    = Bind_u8( menu->top_dot  + menu->leftCursorOffsetY - CLWK_CONTROL_CURSOR_L_HEIGHT/2 );
  area->rect.right  = Bind_u8( area->rect.left + TOUCH_AREA_CONTROL_CURSOR_WIDTH );
  area->rect.bottom = Bind_u8( area->rect.top  + TOUCH_AREA_CONTROL_CURSOR_HEIGHT );

  // 右カーソル
  menu = &( MenuItemDrawParam[ work->cursorPos ] );
  area = &( work->touchHitTable[ TOUCH_AREA_CONTROL_CURSOR_R ] );
  area->rect.left   = Bind_u8( menu->left_dot + menu->rightCursorOffsetX - CLWK_CONTROL_CURSOR_R_WIDTH/2 );
  area->rect.top    = Bind_u8( menu->top_dot  + menu->rightCursorOffsetY - CLWK_CONTROL_CURSOR_R_HEIGHT/2 );
  area->rect.right  = Bind_u8( area->rect.left + TOUCH_AREA_CONTROL_CURSOR_WIDTH );
  area->rect.bottom = Bind_u8( area->rect.top  + TOUCH_AREA_CONTROL_CURSOR_HEIGHT );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief MAIN-BG ( ウィンドウ面 ) を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateMainBG_WINDOW( RRG_WORK* work )
{
  ARCHANDLE* handle;
  ARCDATID datID;

  // ハンドルオープン
  handle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR_GRAPHIC, work->heapID ); 

  // 反映させるスクリーンデータを決定
  if( work->updateFlag ) {
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目名を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateBGFont_TopicTitle( RRG_WORK* work )
{
  u32 topicID;
  u32 strID;

  // 調査項目の補足文の文字列IDを取得
  topicID = work->researchData.topicID;
  strID = StringID_topicTitle[ topicID ];

  // BG ( フォント面 ) に対し, 文字列を書き込む
  BG_FONT_SetMessage( work->BGFont[ SUB_BG_FONT_TOPIC_TITLE ], strID );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 質問の補足文を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateBGFont_QuestionCaption( RRG_WORK* work )
{
  u32 questionID;
  u32 strID;

  // 質問の補足文の文字列IDを取得
  questionID  = GetQuestionID( work );
  strID       = CaptionStringID_question[ questionID ];

  // BG ( フォント面 ) に対し, 文字列を書き込む
  BG_FONT_SetMessage( work->BGFont[ SUB_BG_FONT_QUESTION_CAPTION ], strID );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 質問を更新する 
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateBGFont_Question( RRG_WORK* work )
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 回答を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateBGFont_Answer( RRG_WORK* work )
{
  u16 answerID;
  u8 answerRank;
  u32 count;
  u8 percentage;
  BG_FONT* BGFont;
  STRBUF* strbuf_expand; // 展開後の文字列
  STRBUF* strbuf_plain;  // 展開前の文字列
  STRBUF* strbuf_answer; // 回答の文字列
  const CIRCLE_GRAPH* graph;

  BGFont = work->BGFont[ MAIN_BG_FONT_ANSWER ];
  graph  = GetMainGraph( work );

  // 解析前 or 更新中なら表示しない
  if( (work->analyzeFlag == FALSE) || (work->updateFlag == TRUE) ) {
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
  answerRank = CIRCLE_GRAPH_GetComponentRank_byID( graph, answerID );
  count      = GetCountOfAnswer( work );
  percentage = CIRCLE_GRAPH_GetComponentPercentage_byID( graph, answerID );

  // 数値や回答を文字列に展開
  {
    strbuf_plain  = GFL_MSG_CreateString( work->message[ MESSAGE_STATIC ], str_check_answer );
    strbuf_answer = GFL_MSG_CreateString( work->message[ MESSAGE_ANSWER ], answerID );
    strbuf_expand = GFL_STR_CreateBuffer( 128, work->heapID );
    WORDSET_RegisterNumber( work->wordset, 0, answerRank+1, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT ); // 何番目か
    WORDSET_RegisterWord( work->wordset, 1, strbuf_answer, 0, TRUE, PM_LANG ); // 回答文字列
    WORDSET_RegisterNumber( work->wordset, 2, count, 5, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT ); // 人数
    WORDSET_RegisterNumber( work->wordset, 3, percentage, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT ); // パーセンテージ
    WORDSET_ExpandStr( work->wordset, strbuf_expand, strbuf_plain );
  }

  // BG ( フォント面 ) に対し, 文字列を書き込む
  BG_FONT_SetString( BGFont, strbuf_expand );
  BG_FONT_SetDrawEnable( BGFont, TRUE );

  GFL_STR_DeleteBuffer( strbuf_plain );
  GFL_STR_DeleteBuffer( strbuf_expand );
  GFL_STR_DeleteBuffer( strbuf_answer );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 自分の回答を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateBGFont_MyAnswer( RRG_WORK* work )
{
  u16 answerID;
  STRBUF* strbuf_plain;  // 展開前の文字列
  STRBUF* strbuf_expand; // 展開後の文字列
  STRBUF* strbuf_myAnswer;  // 自分の回答文字列

  // 解析前なら表示しない
  if( (work->analyzeFlag == FALSE) || (GetCountOfQuestion(work) == 0) ) {
    BG_FONT_SetDrawEnable( work->BGFont[ MAIN_BG_FONT_MY_ANSWER ], FALSE );
    return;
  }

  // 現在表示中の質問に対する, 自分の回答IDを取得
  answerID = GetMyAnswerID( work );

  // 回答文字列を展開
  {
    strbuf_plain    = GFL_MSG_CreateString( work->message[ MESSAGE_STATIC ], str_check_my_answer );
    strbuf_myAnswer = GFL_MSG_CreateString( work->message[ MESSAGE_ANSWER ], answerID );
    strbuf_expand   = GFL_STR_CreateBuffer( 128, work->heapID );
    WORDSET_RegisterWord( work->wordset, 0, strbuf_myAnswer, 0, TRUE, PM_LANG );
    WORDSET_ExpandStr( work->wordset, strbuf_expand, strbuf_plain );
  }

  // BG ( フォント面 ) に対し, 文字列を書き込む
  BG_FONT_SetDrawEnable( work->BGFont[ MAIN_BG_FONT_MY_ANSWER ], TRUE );
  BG_FONT_SetString( work->BGFont[ MAIN_BG_FONT_MY_ANSWER ], strbuf_expand );

  GFL_STR_DeleteBuffer( strbuf_plain );
  GFL_STR_DeleteBuffer( strbuf_expand );
  GFL_STR_DeleteBuffer( strbuf_myAnswer );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 回答人数を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateBGFont_Count( RRG_WORK* work )
{
  u32 strID; 
  STRBUF* strbuf_plain;  // 展開前の文字列
  STRBUF* strbuf_expand; // 展開後の文字列

  // 文字列IDを決定
  if( work->analyzeFlag ) { // 解析済み
    switch( work->graphMode ) {
    case GRAPH_DISP_MODE_TODAY: strID = str_check_today_count; break;
    case GRAPH_DISP_MODE_TOTAL: strID = str_check_total_count; break;
    default: GF_ASSERT(0);
    }
  }
  else { // 未解析
    switch( work->graphMode ) {
    case GRAPH_DISP_MODE_TODAY: strID = str_check_today_count01; break;
    case GRAPH_DISP_MODE_TOTAL: strID = str_check_total_count01; break;
    default: GF_ASSERT(0);
    }
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
    WORDSET_RegisterNumber( wordset, 0, count, 6, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wordset, strbuf_expand, strbuf_plain );
  }

  // BG ( フォント面 ) に対し, 文字列を書き込む
  BG_FONT_SetString( work->BGFont[ MAIN_BG_FONT_COUNT ], strbuf_expand );

  GFL_STR_DeleteBuffer( strbuf_plain );
  GFL_STR_DeleteBuffer( strbuf_expand );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief「ただいま ちょうさちゅう」の表示を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateBGFont_NoData( RRG_WORK* work )
{
  if( GetCountOfQuestion(work) == 0 ) { 
    BG_FONT_SetDrawEnable( work->BGFont[ MAIN_BG_FONT_NO_DATA ], TRUE ); // 表示
  }
  else { 
    BG_FONT_SetDrawEnable( work->BGFont[ MAIN_BG_FONT_NO_DATA ], FALSE ); // クリア
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @breif「データしゅとくちゅう」の表示を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateBGFont_DataReceiving( RRG_WORK* work )
{
  if( work->updateFlag ) {
    BG_FONT_SetDrawEnable( work->BGFont[ MAIN_BG_FONT_DATA_RECEIVING ], TRUE ); // 表示  
  }
  else {
    BG_FONT_SetDrawEnable( work->BGFont[ MAIN_BG_FONT_DATA_RECEIVING ], FALSE ); // クリア
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 操作カーソルの表示を更新する
 *
 * @param work 
 */
//-----------------------------------------------------------------------------------------
static void UpdateControlCursor( RRG_WORK* work )
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

  // 更新中 or「自分の回答」or 『報告を見る』ボタンを選択している場合は表示しない
  if( (work->updateFlag == TRUE ) || 
      (work->cursorPos == MENU_ITEM_MY_ANSWER) ||
      (work->cursorPos == MENU_ITEM_ANALYZE) ) {
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 自分の回答アイコン ( ボタン上 ) を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateMyAnswerIconOnButton( RRG_WORK* work )
{
  // 未解析なら表示しない
  if( (work->analyzeFlag == FALSE) || (GetCountOfQuestion(work) == 0) ) {
    GFL_CLACT_WK_SetDrawEnable( work->clactWork[ CLWK_MY_ANSWER_ICON_ON_BUTTON ], FALSE );
    return;
  }

  // 表示
  GFL_CLACT_WK_SetDrawEnable( work->clactWork[ CLWK_MY_ANSWER_ICON_ON_BUTTON ], TRUE );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 自分の回答アイコン ( グラフ上 ) を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
void UpdateMyAnswerIconOnGraph( RRG_WORK* work )
{
  int x, y;
  GFL_CLACTPOS pos;
  GFL_CLWK* clwk;
  const CLWK_INIT_DATA* initData;
  int answerID;

  clwk = work->clactWork[ CLWK_MY_ANSWER_ICON_ON_GRAPH ];
  initData = &( ClactWorkInitData[ CLWK_MY_ANSWER_ICON_ON_GRAPH ] );

  // 未解析なら表示しない
  if( work->analyzeFlag == FALSE ) {
    GFL_CLACT_WK_SetDrawEnable( clwk, FALSE );
    return;
  }

  // カーソル位置が「自分の回答」でないなら表示しない
  if( work->cursorPos != MENU_ITEM_MY_ANSWER ) {
    GFL_CLACT_WK_SetDrawEnable( clwk, FALSE );
    return;
  }

  // 自分の回答IDを取得
  answerID = GetMyAnswerID( work );

  // 対象が0%なら表示しない
  if( CIRCLE_GRAPH_GetComponentPercentage_byID( GetMainGraph(work), answerID ) == 0 ) { 
    GFL_CLACT_WK_SetDrawEnable( clwk, FALSE );
    return; 
  }

  // 無回答なら表示しない
  if( answerID == ANSWER_ID_000 ) {
    GFL_CLACT_WK_SetDrawEnable( clwk, FALSE );
    return; 
  }

  // 表示
  CIRCLE_GRAPH_GetComponentPointPos_byID( GetMainGraph(work), answerID, &x, &y );
  pos.x = x;
  pos.y = y;
  GFL_CLACT_WK_SetPos( clwk, &pos, initData->setSurface );
  GFL_CLACT_WK_SetDrawEnable( clwk, TRUE );

  // アニメーション開始
  GFL_CLACT_WK_SetAutoAnmFlag( clwk, TRUE );
  GFL_CLACT_WK_SetAnmMode( clwk, CLSYS_ANMPM_FORWARD_L );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief『報告を見る』ボタンを表示する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ShowAnalyzeButton( RRG_WORK* work )
{
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_ANALYZE_BUTTON, TRUE );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief「…かいせきちゅう…」を表示する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ShowAnalyzeMessage( RRG_WORK* work )
{
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_ANALYZING, TRUE );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief「…かいせきちゅう…」を非表示にする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void HideAnalyzeMessage( RRG_WORK* work )
{
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_ANALYZING, FALSE );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM の表示状態を設定する
 *
 * @param work
 * @param actorIdx 設定対象のBMP-OAMを指定
 * @param enable   表示するかどうか
 */
//-----------------------------------------------------------------------------------------
static void BmpOamSetDrawEnable( RRG_WORK* work, BMPOAM_ACTOR_INDEX actorIdx, BOOL enable )
{
  // インデックスエラー
  GF_ASSERT( actorIdx < BMPOAM_ACTOR_NUM );

  // 表示状態を変更
  BmpOam_ActorSetDrawEnable( work->BmpOamActor[ actorIdx ], enable );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを開始する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void StartPaletteAnime( RRG_WORK* work, PALETTE_ANIME_INDEX index )
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
 * @param index 停止するアニメーションを指定
 */
//------------------------------------------------------------------------------------
static void StopPaletteAnime( RRG_WORK* work, PALETTE_ANIME_INDEX index )
{
  PALETTE_ANIME_Stop( work->paletteAnime[ index ] );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーション中かどうかをチェックする
 *
 * @param work
 * @param index チェックするアニメーションを指定
 *
 * @return アニメーション中なら TRUE
 *         そうでなければ FALSE
 */
//------------------------------------------------------------------------------------
static BOOL CheckPaletteAnime( const RRG_WORK* work, PALETTE_ANIME_INDEX index )
{
  return PALETTE_ANIME_CheckAnime( work->paletteAnime[ index ] );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを更新する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void UpdatePaletteAnime( RRG_WORK* work )
{
  int idx;

  for( idx=0; idx < PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] );

    PALETTE_ANIME_Update( work->paletteAnime[ idx ] );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief 共通パレットアニメーションを更新する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void UpdateCommonPaletteAnime( RRG_WORK* work )
{
  RRC_UpdatePaletteAnime( work->commonWork );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief パレットアニメによるフラッシュ ( アウト ) を開始する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void StartPaletteFadeFlashOut( RRG_WORK* work )
{
  // MAIN-BG
  PaletteFadeReq( work->paletteFadeSystem, 
                  PF_BIT_MAIN_BG,
                  MAIN_BG_PALETTE_FADE_FLASHOUT_TARGET_BITMASK,
                  MAIN_BG_PALETTE_FADE_FLASHOUT_WAIT,
                  MAIN_BG_PALETTE_FADE_FLASHOUT_START_STRENGTH,
                  MAIN_BG_PALETTE_FADE_FLASHOUT_END_STRENGTH,
                  MAIN_BG_PALETTE_FADE_FLASHOUT_COLOR,
                  work->VBlankTCBSystem );

  // MAIN-OBJ
  PaletteFadeReq( work->paletteFadeSystem, 
                  PF_BIT_MAIN_OBJ,
                  MAIN_OBJ_PALETTE_FADE_FLASHOUT_TARGET_BITMASK,
                  MAIN_OBJ_PALETTE_FADE_FLASHOUT_WAIT,
                  MAIN_OBJ_PALETTE_FADE_FLASHOUT_START_STRENGTH,
                  MAIN_OBJ_PALETTE_FADE_FLASHOUT_END_STRENGTH,
                  MAIN_OBJ_PALETTE_FADE_FLASHOUT_COLOR,
                  work->VBlankTCBSystem );
} 

//-----------------------------------------------------------------------------------------
/**
 * @brief パレットアニメによるフラッシュ ( イン ) を開始する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void StartPaletteFadeFlashIn( RRG_WORK* work )
{
  // MAIN-BG
  PaletteFadeReq( work->paletteFadeSystem, 
                  PF_BIT_MAIN_BG,
                  MAIN_BG_PALETTE_FADE_FLASHIN_TARGET_BITMASK,
                  MAIN_BG_PALETTE_FADE_FLASHIN_WAIT,
                  MAIN_BG_PALETTE_FADE_FLASHIN_START_STRENGTH,
                  MAIN_BG_PALETTE_FADE_FLASHIN_END_STRENGTH,
                  MAIN_BG_PALETTE_FADE_FLASHIN_COLOR,
                  work->VBlankTCBSystem );

  // MAIN-OBJ
  PaletteFadeReq( work->paletteFadeSystem, 
                  PF_BIT_MAIN_OBJ,
                  MAIN_OBJ_PALETTE_FADE_FLASHIN_TARGET_BITMASK,
                  MAIN_OBJ_PALETTE_FADE_FLASHIN_WAIT,
                  MAIN_OBJ_PALETTE_FADE_FLASHIN_START_STRENGTH,
                  MAIN_OBJ_PALETTE_FADE_FLASHIN_END_STRENGTH,
                  MAIN_OBJ_PALETTE_FADE_FLASHIN_COLOR,
                  work->VBlankTCBSystem );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief パレットのフェードが完了したかどうかを判定する
 *
 * @param work
 *
 * @return パレットフェードが完了している場合 TRUE
 *         そうでなければ FALSE
 */
//-----------------------------------------------------------------------------------------
static BOOL IsPaletteFadeEnd( RRG_WORK* work )
{
  return (PaletteFadeCheck( work->paletteFadeSystem ) == 0 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief VBlank 割り込み処理
 *
 * @param tcb
 * @parma wk
 */
//-----------------------------------------------------------------------------------------
static void VBlankFunc( GFL_TCB* tcb, void* wk )
{
  RRG_WORK* work = (RRG_WORK*)wk;

  GFL_BG_VBlankFunc(); // BG
  GFL_CLACT_SYS_VBlankFunc(); // OBJ
  PaletteFadeTrans( work->paletteFadeSystem ); // パレットフェード
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 新しい人物とのすれ違いを監視する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void WatchOutNewEntry( RRG_WORK* work )
{
  if( GAMEBEACON_Get_NewEntry() == TRUE ) {
    // 「今日の調査人数」が更新されなければ,「今までの調査人数」も変化しない
    u16 now_today_count = GetTodayCountOfQuestion( work );
    u16 new_today_count = GetTodayCountOfQuestion_from_SaveData( work ); 
    if( now_today_count != new_today_count ) {
      work->newEntryFlag = TRUE;
    }
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief すれ違いフラグをクリアする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ClearNewEntryFlag( RRG_WORK* work )
{
  work->newEntryFlag = FALSE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 更新が可能な状態かどうかを監視する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void WatchOutUpdateEnable( RRG_WORK* work )
{
  if( work->updateCount < UPDATE_INTERVAL ) {
    work->updateCount++;
  }
  else {
    work->updateEnableFlag = TRUE;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 一定時間の間, 更新が起こらない状態にする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DisableUpdate( RRG_WORK* work )
{
  work->updateCount = 0;
  work->updateEnableFlag = FALSE;
}



//=========================================================================================
// ◇LAYER 3 取得・設定・判定
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief ゲームシステムを取得する
 *
 * @param work
 *
 * @return ゲームシステム
 */
//-----------------------------------------------------------------------------------------
static GAMESYS_WORK* GetGameSystem( const RRG_WORK* work )
{
  return RRC_GetGameSystem( work->commonWork );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ゲームデータを取得する
 *
 * @param work
 *
 * @return ゲームデータ
 */
//-----------------------------------------------------------------------------------------
static GAMEDATA* GetGameData( const RRG_WORK* work )
{
  return RRC_GetGameData( work->commonWork );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ヒープIDを取得する
 *
 * @param work
 *
 * @return ヒープID
 */
//-----------------------------------------------------------------------------------------
static HEAPID GetHeapID( const RRG_WORK* work )
{
  return work->heapID;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ヒープIDを設定する
 *
 * @param work
 * @param heapID
 */
//-----------------------------------------------------------------------------------------
static void SetHeapID( RRG_WORK* work, HEAPID heapID )
{
  work->heapID = heapID;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 全画面共通ワークを取得する
 *
 * @param work
 *
 * @return 全画面共通ワーク
 */
//-----------------------------------------------------------------------------------------
static RRC_WORK* GetCommonWork( const RRG_WORK* work )
{
  return work->commonWork;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 全画面共通ワークを設定する
 *
 * @param work
 * @param commonWork
 */
//-----------------------------------------------------------------------------------------
static void SetCommonWork( RRG_WORK* work, RRC_WORK* commonWork )
{
  work->commonWork = commonWork;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 強制終了フラグを取得する
 */
//-----------------------------------------------------------------------------------------
static BOOL CheckForceReturnFlag( const RRG_WORK* work )
{
  const RRC_WORK* commonWork = GetCommonWork( work );
  return RRC_GetForceReturnFlag( commonWork );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査データの表示タイプを設定する
 *
 * @param work
 * @param graphMode 設定する表示タイプ
 */
//-----------------------------------------------------------------------------------------
static void SetDataDisplayType( RRG_WORK* work, GRAPH_DISP_MODE graphMode )
{
  work->graphMode = graphMode;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在表示中のメイン円グラフ を取得する
 *
 * @param work
 *
 * @return 現在表示中のメイン円グラフ
 */
//-----------------------------------------------------------------------------------------
static CIRCLE_GRAPH* GetMainGraph( const RRG_WORK* work )
{
  return work->mainGraph[ work->graphMode ];
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在表示中のサブ円グラフ
 *
 * @param work
 *
 * @return 現在表示中のサブ円グラフ
 */
//-----------------------------------------------------------------------------------------
static CIRCLE_GRAPH* GetSubGraph ( const RRG_WORK* work )
{
  return work->subGraph[ work->graphMode ];
}

//-----------------------------------------------------------------------------------------
/**
 * @brief アニメーション中の円グラフがあるかどうかを判定する
 *
 * @param work
 *
 * @return アニメーションしているグラフがある場合 FALSE
 *         すべてのグラフのアニメーションが終了している場合 TRUE
 */
//-----------------------------------------------------------------------------------------
static BOOL CheckAllGraphAnimeEnd( const RRG_WORK* work )
{
  if( CIRCLE_GRAPH_IsAnime( work->mainGraph[ GRAPH_DISP_MODE_TODAY ] ) ||
      CIRCLE_GRAPH_IsAnime( work->mainGraph[ GRAPH_DISP_MODE_TOTAL ] ) ||
      CIRCLE_GRAPH_IsAnime( work->subGraph[ GRAPH_DISP_MODE_TODAY ] ) ||
      CIRCLE_GRAPH_IsAnime( work->subGraph[ GRAPH_DISP_MODE_TOTAL ] ) ) { return FALSE; }

  return TRUE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の調査項目IDを取得する
 *
 * @param work
 *
 * @return 現在表示中の調査項目ID
 */
//-----------------------------------------------------------------------------------------
static u8 GetTopicID( const RRG_WORK* work )
{
  return work->researchData.topicID;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の質問IDを取得する
 *
 * @param work
 *
 * @return 現在表示中の質問ID
 */
//-----------------------------------------------------------------------------------------
static u8 GetQuestionID( const RRG_WORK* work )
{
  u8 qIdx;
  qIdx = work->questionIdx;
  return work->researchData.questionData[ qIdx ].ID;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の質問に対する回答選択肢の数 を取得する
 *
 * @param work
 *
 * @return 現在表示中の質問に対する, 回答の選択肢の数
 */
//------------------------------------------------------------------------------------------
static u8 GetAnswerNum( const RRG_WORK* work )
{
  u8 questionIdx;

  questionIdx = work->questionIdx;

  return work->researchData.questionData[ questionIdx ].answerNum;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の回答IDを取得する
 *
 * @param work
 *
 * @return 現在表示中の回答ID
 */
//-----------------------------------------------------------------------------------------
static u16 GetAnswerID( const RRG_WORK* work )
{
  u8 qIdx;
  u8 aIdx;

  qIdx = work->questionIdx;
  aIdx = work->answerIdx;

  return work->researchData.questionData[ qIdx ].answerData[ aIdx ].ID;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の質問に対する, 表示の回答人数 を取得する
 *
 * @param work
 *
 * @return 現在表示中の質問に対する, 表示の回答人数
 */
//-----------------------------------------------------------------------------------------
static u32 GetCountOfQuestion( const RRG_WORK* work )
{
  u32 count = 0;

  switch( work->graphMode ) {
  case GRAPH_DISP_MODE_TODAY:  count = GetTodayCountOfQuestion( work ); break;
  case GRAPH_DISP_MODE_TOTAL:  count = GetTotalCountOfQuestion( work ); break;
  default: GF_ASSERT(0);
  }

  return count;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の質問に対する, 今日の回答人数を取得する
 *
 * @param work
 *
 * @return 現在表示中の質問に対する, 今日の回答人数
 */
//-----------------------------------------------------------------------------------------
static u32 GetTodayCountOfQuestion( const RRG_WORK* work )
{
  u8 qIdx;

  qIdx = work->questionIdx;

  return work->researchData.questionData[ qIdx ].todayCount;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の質問に対する, 合計の回答人数
 *
 * @param work
 *
 * @return 現在表示中の質問に対する, 合計の回答人数
 */
//-----------------------------------------------------------------------------------------
static u32 GetTotalCountOfQuestion( const RRG_WORK* work )
{
  u8 qIdx;

  qIdx = work->questionIdx;

  return work->researchData.questionData[ qIdx ].totalCount;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の回答に対する, 表示の回答人数 を取得する
 *
 * @param work
 *
 * @return 現在表示中の回答に対する, 表示の回答人数
 */
//-----------------------------------------------------------------------------------------
static u32 GetCountOfAnswer( const RRG_WORK* work )
{
  switch( work->graphMode ) {
  case GRAPH_DISP_MODE_TODAY: return GetTodayCountOfAnswer( work ); break;
  case GRAPH_DISP_MODE_TOTAL: return GetTotalCountOfAnswer( work ); break;
  default: GF_ASSERT(0);
  }

  // エラー
  GF_ASSERT(0);
  return 0;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の回答に対する, 今日の回答人数
 *
 * @param work
 *
 * @return 現在表示中の回答に対する, 今日の回答人数
 */
//-----------------------------------------------------------------------------------------
static u32 GetTodayCountOfAnswer( const RRG_WORK* work )
{
  u8 qIdx;
  u8 aIdx;

  qIdx = work->questionIdx;
  aIdx = work->answerIdx;

  return work->researchData.questionData[ qIdx ].answerData[ aIdx ].todayCount;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の回答に対する, 合計の回答人数
 *
 * @param work
 *
 * @return 現在表示中の回答に対する, 合計の回答人数
 */
//-----------------------------------------------------------------------------------------
static u32 GetTotalCountOfAnswer( const RRG_WORK* work )
{
  u8 qIdx;
  u8 aIdx;

  qIdx = work->questionIdx;
  aIdx = work->answerIdx;

  return work->researchData.questionData[ qIdx ].answerData[ aIdx ].totalCount;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在調査中の調査項目IDを取得する
 *
 * @param work
 *
 * @return 現在調査中の調査項目ID
 */
//-----------------------------------------------------------------------------------------
static u8 GetInvestigatingTopicID( const RRG_WORK* work )
{
  int qIdx;
  GAMEDATA* gameData;
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* QSave;
  u8 questionID[ QUESTION_NUM_PER_TOPIC ];
  u8 topicID;

  gameData = GetGameData( work );
  save  = GAMEDATA_GetSaveControlWork( gameData );
  QSave = SaveData_GetQuestionnaire( save );

  // 調査中の質問IDを取得
  for( qIdx=0; qIdx < QUESTION_NUM_PER_TOPIC; qIdx++ )
  {
    questionID[ qIdx ] = QuestionnaireWork_GetInvestigatingQuestion( QSave, qIdx );
  }

  // 質問IDから, 調査項目IDを求める
  for( topicID=0; topicID < TOPIC_ID_NUM; topicID++ )
  {
    if( (questionID[0] == Question1_topic[ topicID ]) &&
        (questionID[1] == Question2_topic[ topicID ]) &&
        (questionID[2] == Question3_topic[ topicID ]) ) { return topicID; }
  }

  // 質問の組み合わせに該当する調査項目が存在しない
  GF_ASSERT(0);
  return 0;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在表示中の質問に対する, 自分の回答IDを取得する 
 * 
 * @param work
 *
 * @return 現在表示中の質問に対する, 自分の回答ID
 *         無回答の場合, ANSWER_ID_000
 */
//-----------------------------------------------------------------------------------------
u8 GetMyAnswerID( const RRG_WORK* work )
{
  GAMEDATA* gameData;
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* QSave;
  QUESTIONNAIRE_ANSWER_WORK* myAnswer;
  const RESEARCH_DATA* researchData;
  u8 questionID, questionIdx;
  u16 answerID, answerIdx;

  // セーブデータを取得
  gameData = GetGameData( work );
  save     = GAMEDATA_GetSaveControlWork( gameData );
  QSave    = SaveData_GetQuestionnaire( save );
  myAnswer = Questionnaire_GetAnswerWork( QSave );

  // 質問IDを取得
  questionID = GetQuestionID( work );

  // 質問が『プレイ時間は？』なら個別対処
  if( questionID == QUESTION_ID_PLAY_TIME ) {
    return GetMyAnswerID_PlayTime( work );
  }

  // 現在表示中の質問に対する, 自分の回答インデックスを取得
  answerIdx = QuestionnaireAnswer_ReadBit( myAnswer, questionID );

  // 無回答を考慮する
  if( answerIdx == 0 ) {
    return ANSWER_ID_000;
  }
  else {
    answerIdx--;
  }

  // 回答IDを取得
  researchData = &( work->researchData );
  questionIdx  = work->questionIdx;
  answerID     = RESEARCH_DATA_GetAnswerID_byIndex( researchData, questionIdx, answerIdx );

  return answerID;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 質問『プレイ時間は？』に対する自分の回答IDを取得する
 *
 * @param work
 *
 * @return 質問『プレイ時間は？』に対する自分の回答ID ( ANSWER_ID_xxxx )
 */
//-----------------------------------------------------------------------------------------
static u8 GetMyAnswerID_PlayTime( const RRG_WORK* work )
{
  GAMEDATA* gameData;
  PLAYTIME* time;
  u16 hour;

  gameData = GetGameData( work );
  time     = GAMEDATA_GetPlayTimeWork( gameData );
  hour     = PLAYTIME_GetHour( time );

  if( hour < 10 ) { return ANSWER_ID_135; }
  else if( hour <  20 ) { return ANSWER_ID_136; }
  else if( hour <  30 ) { return ANSWER_ID_137; }
  else if( hour <  40 ) { return ANSWER_ID_138; }
  else if( hour <  50 ) { return ANSWER_ID_139; }
  else if( hour <  60 ) { return ANSWER_ID_140; }
  else if( hour <  70 ) { return ANSWER_ID_141; }
  else if( hour <  80 ) { return ANSWER_ID_142; }
  else if( hour <  90 ) { return ANSWER_ID_143; }
  else if( hour < 100 ) { return ANSWER_ID_144; }
  else { return ANSWER_ID_145; }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief セーブデータが持つ, 現在表示中の質問に対する, 今日の回答人数を取得する
 *
 * @param work
 *
 * @return 現在表示中の質問に対する, 今日の回答人数 ( セーブデータの値 )
 */
//-----------------------------------------------------------------------------------------
static u32 GetTodayCountOfQuestion_from_SaveData( const RRG_WORK* work )
{
  GAMEDATA* gameData;
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* QSave;
  u8 questionID;
  u32 todayCount;

  gameData = GetGameData( work );
  save     = GAMEDATA_GetSaveControlWork( gameData );
  QSave    = SaveData_GetQuestionnaire( save ); 

  questionID = GetQuestionID( work );
  todayCount = QuestionnaireWork_GetTodayCount( QSave, questionID );

  return todayCount;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief OBJ リソースの登録インデックスを取得する
 *
 * @param work
 * @param resID リソースID
 *
 * @return 指定したリソースの登録インデックス
 */
//-----------------------------------------------------------------------------------------
static u32 GetObjResourceRegisterIndex( const RRG_WORK* work, OBJ_RESOURCE_ID resID )
{
  return work->objResRegisterIdx[ resID ];
}

//-----------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを取得する
 *
 * @param work
 * @param unitIdx セルアクターユニットのインデックス
 *
 * @return 指定したセルアクターユニット
 */
//-----------------------------------------------------------------------------------------
static GFL_CLUNIT* GetClactUnit( const RRG_WORK* work, CLUNIT_INDEX unitIdx )
{
  return work->clactUnit[ unitIdx ];
}

//-----------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを取得する
 *
 * @param work
 * @param unitIdx セルアクターワークのインデックス
 *
 * @return 指定したセルアクターワーク
 */
//-----------------------------------------------------------------------------------------
static GFL_CLWK* GetClactWork( const RRG_WORK* work, CLWK_INDEX wkIdx )
{
  return work->clactWork[ wkIdx ];
}


//=========================================================================================
// ◇LAYER 2 生成・初期化・破棄
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief グラフ画面管理ワーク 生成
 *
 * @param work
 *
 * @return グラフ画面管理ワーク
 */
//-----------------------------------------------------------------------------------------
static RRG_WORK* CreateGraphWork( HEAPID heapID )
{
  RRG_WORK* work;

  work = GFL_HEAP_AllocMemory( heapID, sizeof(RRG_WORK) );

  return work;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief グラフ画面管理ワーク 初期化
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitGraphWork( RRG_WORK* work )
{
  int i;

  work->state              = RRG_STATE_SETUP;
  work->stateSeq           = 0;
  work->stateEndFlag       = FALSE;
  work->stateCount         = 0;
  work->waitFrame          = WAIT_FRAME_BUTTON;
  work->updateCount        = 0;
  work->cursorPos          = MENU_ITEM_QUESTION;
  work->setupFlag          = FALSE;
  work->analyzeFlag        = FALSE;
  work->analyzeByTouchFlag = FALSE;
  work->updateFlag         = FALSE;
  work->updateEnableFlag   = FALSE;
  work->newEntryFlag       = FALSE;
  work->questionIdx        = 0;
  work->answerIdx          = 0;
  work->graphMode          = GRAPH_DISP_MODE_TODAY;
  work->VBlankTCBSystem    = GFUser_VIntr_GetTCBSYS();
  work->percentageNum      = 0;
  work->percentageDispNum  = 0;
  work->finishResult       = RRG_RESULT_TO_TOP;
  work->finishFlag         = FALSE;

  for( i=0; i<OBJ_RESOURCE_NUM; i++ ){ work->objResRegisterIdx[i] = 0; }

  InitResearchData( work );
  InitStateQueue( work );
  InitCircleGraphs( work );
  InitArrow( work );
  InitPercentage( work );
  InitMessages( work );
  InitWordset( work );
  InitFont( work );
  InitBGFonts( work );
  InitClactUnits( work );
  InitClactWorks( work );
  InitBitmapDatas( work );
  InitPaletteFadeSystem( work );
  InitPaletteAnime( work );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief グラフ画面管理ワーク 破棄
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteGraphWork( RRG_WORK* work )
{
  GFL_HEAP_FreeMemory( work );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態キューを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitStateQueue( RRG_WORK* work )
{
  work->stateQueue = NULL;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態キューを作成する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CreateStateQueue( RRG_WORK* work )
{
  GF_ASSERT( work->stateQueue == NULL );

  // 作成
  work->stateQueue = QUEUE_Create( STATE_QUEUE_SIZE, work->heapID );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態キューを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteStateQueue( RRG_WORK* work )
{
  GF_ASSERT( work->stateQueue );

  // 削除
  QUEUE_Delete( work->stateQueue );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitFont( RRG_WORK* work )
{
  // 初期化
  work->font = NULL;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを作成する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CreateFont( RRG_WORK* work )
{
  GF_ASSERT( work->font == NULL );

  // 生成
  work->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, 
                                GFL_FONT_LOADTYPE_FILE, FALSE, work->heapID );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteFont( RRG_WORK* work )
{
  GF_ASSERT( work->font );

  // 削除
  GFL_FONT_Delete( work->font );
}


//-----------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitMessages( RRG_WORK* work )
{
  int msgIdx;

  // 初期化
  for( msgIdx=0; msgIdx < MESSAGE_NUM; msgIdx++ )
  {
    work->message[ msgIdx ] = NULL;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを作成する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CreateMessages( RRG_WORK* work )
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteMessages( RRG_WORK* work )
{
  int msgIdx;

  for( msgIdx=0; msgIdx < MESSAGE_NUM; msgIdx++ )
  {
    GF_ASSERT( work->message[ msgIdx ] );

    // 削除
    GFL_MSG_Delete( work->message[ msgIdx ] );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ワードセットを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitWordset( RRG_WORK* work )
{
  // 初期化
  work->wordset = NULL;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ワードセットを作成する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CreateWordset( RRG_WORK* work )
{
  // 多重生成
  GF_ASSERT( work->wordset == NULL );

  // 作成
  work->wordset = WORDSET_Create( work->heapID );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ワードセットを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteWordset( RRG_WORK* work )
{
  // 削除
  WORDSET_Delete( work->wordset );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 円グラフを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitCircleGraphs( RRG_WORK* work )
{
  int idx;

  for( idx=0; idx < GRAPH_DISP_MODE_NUM; idx++ )
  {
    work->mainGraph[ idx ] = NULL;
    work->subGraph[ idx ]  = NULL;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 円グラフを生成する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CreateCircleGraph( RRG_WORK* work )
{
  int idx;

  for( idx=0; idx < GRAPH_DISP_MODE_NUM; idx++ )
  {
    // 多重生成
    GF_ASSERT( work->mainGraph[ idx ] == NULL );
    GF_ASSERT( work->subGraph[ idx ] == NULL );

    work->mainGraph[ idx ] = CIRCLE_GRAPH_Create( work->heapID );
    work->subGraph[ idx ]  = CIRCLE_GRAPH_Create( work->heapID );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 円グラフを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteCircleGraph( RRG_WORK* work )
{
  int idx;

  for( idx=0; idx < GRAPH_DISP_MODE_NUM; idx++ )
  {
    // 未生成
    GF_ASSERT( work->mainGraph[ idx ] != NULL );
    GF_ASSERT( work->subGraph[ idx ] != NULL );

    CIRCLE_GRAPH_Delete( work->mainGraph[ idx ] );
    CIRCLE_GRAPH_Delete( work->subGraph[ idx ] );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査データを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitResearchData( RRG_WORK* work )
{
  // 0クリア
  GFL_STD_MemClear( &( work->researchData ), sizeof(RESEARCH_DATA) );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査データを取得する
 *
 * @parma work
 */
//-----------------------------------------------------------------------------------------
static void SetupResearchData( RRG_WORK* work )
{
  int qIdx, aIdx;
  u8 topicID;
  u8 questionID[ QUESTION_NUM_PER_TOPIC ];
  u8 answerNumOfQuestion[ QUESTION_NUM_PER_TOPIC ];
  u32 todayCountOfQuestion[ QUESTION_NUM_PER_TOPIC ];
  u32 totalCountOfQuestion[ QUESTION_NUM_PER_TOPIC ];
  u16 answerID[ QUESTION_NUM_PER_TOPIC ][ MAX_ANSWER_NUM_PER_QUESTION ];
  u32 todayCountOfAnswer[ QUESTION_NUM_PER_TOPIC ][ MAX_ANSWER_NUM_PER_QUESTION ];
  u32 totalCountOfAnswer[ QUESTION_NUM_PER_TOPIC ][ MAX_ANSWER_NUM_PER_QUESTION ];
  GAMEDATA* gameData;
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* QSave;

  // セーブデータ取得
  gameData = GetGameData( work );
  save = GAMEDATA_GetSaveControlWork( gameData );
  QSave = SaveData_GetQuestionnaire( save );
  topicID = GetInvestigatingTopicID( work ); // 調査項目ID
  questionID[0] = QuestionnaireWork_GetInvestigatingQuestion( QSave, 0 ); // 質問ID
  questionID[1] = QuestionnaireWork_GetInvestigatingQuestion( QSave, 1 ); // 質問ID
  questionID[2] = QuestionnaireWork_GetInvestigatingQuestion( QSave, 2 ); // 質問ID
  answerNumOfQuestion[0] = AnswerNum_question[ questionID[0] ]; // 質問ID ==> 回答選択肢の数
  answerNumOfQuestion[1] = AnswerNum_question[ questionID[1] ]; // 質問ID ==> 回答選択肢の数
  answerNumOfQuestion[2] = AnswerNum_question[ questionID[2] ]; // 質問ID ==> 回答選択肢の数
  todayCountOfQuestion[0] = QuestionnaireWork_GetTodayCount( QSave, questionID[0] ); // 質問に対する, 今日の回答人数
  todayCountOfQuestion[1] = QuestionnaireWork_GetTodayCount( QSave, questionID[1] ); // 質問に対する, 今日の回答人数
  todayCountOfQuestion[2] = QuestionnaireWork_GetTodayCount( QSave, questionID[2] ); // 質問に対する, 今日の回答人数
  totalCountOfQuestion[0] = QuestionnaireWork_GetTotalCount( QSave, questionID[0] ); // 質問に対する, いままでの回答人数
  totalCountOfQuestion[1] = QuestionnaireWork_GetTotalCount( QSave, questionID[1] ); // 質問に対する, いままでの回答人数
  totalCountOfQuestion[2] = QuestionnaireWork_GetTotalCount( QSave, questionID[2] ); // 質問に対する, いままでの回答人数
  for( qIdx=0; qIdx < QUESTION_NUM_PER_TOPIC; qIdx++ )
  {
    u8 qID = questionID[ qIdx ];

    for( aIdx=0; aIdx < answerNumOfQuestion[ qIdx ]; aIdx++ )
    {
      answerID[ qIdx ][ aIdx ] = AnswerID_question[ qID ][ aIdx ]; // 回答ID
      todayCountOfAnswer[ qIdx ][ aIdx ] = QuestionnaireWork_GetTodayAnswerNum( QSave, qID, aIdx + 1 ); // 回答に対する, 今日の回答人数
      totalCountOfAnswer[ qIdx ][ aIdx ] = QuestionnaireWork_GetTotalAnswerNum( QSave, qID, aIdx + 1 ); // 回答に対する, いままでの回答人数 ( 今日の人数を除く )
    }
  }

  // データ設定
  // ※いままで = 今日 + 昨日まで
  work->researchData.topicID = topicID; // 調査項目ID
  for( qIdx=0; qIdx < QUESTION_NUM_PER_TOPIC; qIdx++ )
  {
    work->researchData.questionData[ qIdx ].ID = questionID[ qIdx ]; // 質問ID
    work->researchData.questionData[ qIdx ].answerNum  = answerNumOfQuestion[ qIdx ]; // 回答選択肢の数
    work->researchData.questionData[ qIdx ].todayCount = todayCountOfQuestion[ qIdx ]; // 今日の回答人数
    work->researchData.questionData[ qIdx ].totalCount = todayCountOfQuestion[ qIdx ] + totalCountOfQuestion[ qIdx ]; // いままでの回答人数

    // 表示する「いままでの回答人数」は 999,999 を最大とする
    if( QUESTIONNAIRE_TOTAL_COUNT_MAX < work->researchData.questionData[ qIdx ].totalCount ) {
      work->researchData.questionData[ qIdx ].totalCount = QUESTIONNAIRE_TOTAL_COUNT_MAX;
    }

    for( aIdx=0; aIdx < MAX_ANSWER_NUM_PER_QUESTION; aIdx++ )
    {
      int aID = answerID[ qIdx ][ aIdx ];
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].ID = aID; // 回答ID
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].colorR = ColorR_answer[ aID ]; // 表示カラー(R)
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].colorG = ColorG_answer[ aID ]; // 表示カラー(G)
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].colorB = ColorB_answer[ aID ]; // 表示カラー(B)
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].todayCount = todayCountOfAnswer[ qIdx ][ aIdx ]; // 今日の回答人数
      work->researchData.questionData[ qIdx ].answerData[ aIdx ].totalCount = todayCountOfAnswer[ qIdx ][ aIdx ] + totalCountOfAnswer[ qIdx ][ aIdx ];  // いままでの回答人数
    }
  } 
}

//-----------------------------------------------------------------------------------------
/**
 * @breif タッチ領域の準備を行う
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupTouchArea( RRG_WORK* work )
{
  int idx;

  for( idx=0; idx < TOUCH_AREA_NUM; idx++ )
  {
    work->touchHitTable[ idx ].rect.left   = TouchAreaInitData[ idx ].left;
    work->touchHitTable[ idx ].rect.right  = TouchAreaInitData[ idx ].right;
    work->touchHitTable[ idx ].rect.top    = TouchAreaInitData[ idx ].top;
    work->touchHitTable[ idx ].rect.bottom = TouchAreaInitData[ idx ].bottom;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 矢印を初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitArrow( RRG_WORK* work )
{
  work->arrow = NULL;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 矢印を生成する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CreateArrow( RRG_WORK* work )
{
  ARROW_DISP_PARAM dispParam;

  // 多重生成
  GF_ASSERT( work->arrow == NULL );

  // 矢印を生成する
  dispParam.cgrIndex      = GetObjResourceRegisterIndex( work, OBJ_RESOURCE_MAIN_CHARACTER );
  dispParam.plttIndex     = GetObjResourceRegisterIndex( work, OBJ_RESOURCE_MAIN_PALETTE );
  dispParam.cellAnimIndex = GetObjResourceRegisterIndex( work, OBJ_RESOURCE_MAIN_CELL_ANIME );
  dispParam.setSerface    = CLSYS_DEFREND_MAIN;
  dispParam.anmseqH       = NANR_obj_yoko;
  dispParam.anmseqV       = NANR_obj_tate;
  dispParam.anmseqCorner  = NANR_obj_corner;
  dispParam.anmseqEnd     = NANR_obj_point;
  work->arrow = ARROW_Create( work->heapID, &dispParam );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 矢印を破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteArrow( RRG_WORK* work )
{
  GF_ASSERT( work->arrow );

  // 破棄
  ARROW_Delete( work->arrow );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief % 表示オブジェクトを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitPercentage( RRG_WORK* work )
{
  int idx;

  for( idx=0; idx < PERCENTAGE_NUM; idx++ )
  {
    work->percentage[ idx ] = NULL;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief % 表示オブジェクトを生成する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CreatePercentage( RRG_WORK* work )
{
  int idx; 
  PERCENTAGE_DISP_PARAM param;

  // 表示パラメータを設定
  param.cgrIndex      = GetObjResourceRegisterIndex( work, OBJ_RESOURCE_MAIN_CHARACTER );
  param.plttIndex     = GetObjResourceRegisterIndex( work, OBJ_RESOURCE_MAIN_PALETTE );
  param.cellAnimIndex = GetObjResourceRegisterIndex( work, OBJ_RESOURCE_MAIN_CELL_ANIME );
  param.setSerface    = CLSYS_DEFREND_MAIN;
  param.anmseqNumber  = NANR_obj_nom;
  param.anmseqBase    = NANR_obj_percent;

  // 全回答用のオブジェクトを生成
  for( idx=0; idx < PERCENTAGE_NUM; idx++ )
  {
    GF_ASSERT( work->percentage[ idx ] == NULL );

    // 生成
    work->percentage[ idx ] =
      PERCENTAGE_Create( work->heapID, &param, GetClactUnit(work, CLUNIT_PERCENTAGE) );

    // 非表示に設定
    PERCENTAGE_SetDrawEnable( work->percentage[ idx ], FALSE );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief % 表示オブジェクトを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeletePercentage( RRG_WORK* work )
{
  int idx;

  // 全回答用のオブジェクトを破棄
  for( idx=0; idx < PERCENTAGE_NUM; idx++ )
  {
    GF_ASSERT( work->percentage[ idx ] );

    // 破棄
    PERCENTAGE_Delete( work->percentage[ idx ] );
  }
}

//-------------------------------------------------------------------------------
/**
 * @brief 3D 初期設定
 */
//-------------------------------------------------------------------------------
static void Setup3D( void )
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

//-----------------------------------------------------------------------------------------
/**
 * @brief BG の準備
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupBG( RRG_WORK* work )
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief BG の後片付け
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CleanUpBG( RRG_WORK* work )
{
  GFL_BMPWIN_Exit();

  GFL_BG_FreeBGControl( MAIN_BG_3D );
  GFL_BG_FreeBGControl( MAIN_BG_FONT );
  GFL_BG_FreeBGControl( MAIN_BG_WINDOW );
  GFL_BG_FreeBGControl( SUB_BG_FONT );
  GFL_BG_FreeBGControl( SUB_BG_WINDOW );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 上画面 ウィンドウBG面 準備
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupSubBG_WINDOW( RRG_WORK* work )
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

//-----------------------------------------------------------------------------------------
/**
 * @brief 上画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CleanUpSubBG_WINDOW( RRG_WORK* work )
{
}

//-----------------------------------------------------------------------------------------
/**
 * @brief SUB-BG フォント面の準備
 * 
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupSubBG_FONT( RRG_WORK* work )
{
  // NULLキャラ確保
  GFL_BG_FillCharacter( SUB_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( SUB_BG_FONT );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief SUB-BG フォント面の後片付け
 * 
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CleanUpSubBG_FONT( RRG_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( SUB_BG_FONT, 1, 0 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 準備
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupMainBG_WINDOW( RRG_WORK* work )
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CleanUpMainBG_WINDOW( RRG_WORK* work )
{
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 下画面 フォントBG面 準備
 * 
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupMainBG_FONT( RRG_WORK* work )
{ 
  // NULLキャラ確保
  GFL_BG_FillCharacter( MAIN_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( MAIN_BG_FONT );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 下画面 フォントBG面 後片付け
 * 
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CleanUpMainBG_FONT( RRG_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( MAIN_BG_FONT, 1, 0 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitBGFonts( RRG_WORK* work )
{
  int idx;

  for( idx=0; idx < BG_FONT_NUM; idx++ )
  {
    work->BGFont[ idx ] = NULL; 
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを作成する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CreateBGFonts( RRG_WORK* work )
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteBGFonts( RRG_WORK* work )
{
  int i;
  
  // 通常のBGFont
  for( i=0; i < BG_FONT_NUM; i++ )
  {
    GF_ASSERT( work->BGFont[i] );
    BG_FONT_Delete( work->BGFont[i] );
    work->BGFont[i] = NULL;
  }
} 

//-----------------------------------------------------------------------------------------
/**
 * @brief SUB-OBJ リソースを登録する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void RegisterSubObjResources( RRG_WORK* work )
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

//-----------------------------------------------------------------------------------------
/**
 * @brief SUB-OBJ リソースを解放する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ReleaseSubObjResources( RRG_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CELL_ANIME ] );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief MAIN-OBJ リソースを登録する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void RegisterMainObjResources( RRG_WORK* work )
{
  HEAPID heapID;
  ARCHANDLE* arcHandle;
  u32 character, palette, commonPalette, cellAnime;

  heapID = work->heapID;

  arcHandle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR_GRAPHIC, heapID ); 
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
  GFL_ARC_CloseDataHandle( arcHandle );


  arcHandle = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, heapID ); 
  commonPalette = GFL_CLGRP_PLTT_RegisterEx( arcHandle, 
                                       NARC_app_menu_common_task_menu_NCLR,
                                       CLSYS_DRAW_MAIN,
                                       ONE_PALETTE_SIZE*4, 0, 2, 
                                       heapID );
  GFL_ARC_CloseDataHandle( arcHandle );

  // 登録インデックスを記憶
  work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CHARACTER ]  = character;
  work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_PALETTE ]    = palette;
  work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_COMMON_PALETTE ]    = commonPalette;
  work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CELL_ANIME ] = cellAnime;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief MAIN-OBJ リソースを解放する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ReleaseMainObjResources( RRG_WORK* work )
{
  GFL_CLGRP_CGR_Release     ( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CHARACTER ] );
  GFL_CLGRP_PLTT_Release    ( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_PALETTE ] );
  GFL_CLGRP_PLTT_Release    ( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_COMMON_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CELL_ANIME ] );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitClactUnits( RRG_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < CLUNIT_NUM; unitIdx++ )
  {
    work->clactUnit[ unitIdx ] = NULL;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを作成する
 *
 * @param
 */
//-----------------------------------------------------------------------------------------
static void CreateClactUnits( RRG_WORK* work )
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

//-----------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを破棄する
 *
 * @param
 */
//-----------------------------------------------------------------------------------------
static void DeleteClactUnits( RRG_WORK* work )
{
  int unitIdx;

  for( unitIdx=0; unitIdx < CLUNIT_NUM; unitIdx++ )
  {
    GF_ASSERT( work->clactUnit[ unitIdx ] );
    GFL_CLACT_UNIT_Delete( work->clactUnit[ unitIdx ] );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitClactWorks( RRG_WORK* work )
{
  int wkIdx;

  // 初期化
  for( wkIdx=0; wkIdx < CLWK_NUM; wkIdx++ )
  {
    work->clactWork[ wkIdx ] = NULL;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを作成する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CreateClactWorks( RRG_WORK* work )
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief セルアクターワークを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteClactWorks( RRG_WORK* work )
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

//-----------------------------------------------------------------------------------------
/**
 * @brief ビットマップデータを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitBitmapDatas( RRG_WORK* work )
{
  int idx;

  // 初期化
  for( idx=0; idx < BMPOAM_ACTOR_NUM; idx++ )
  {
    work->BmpData[ idx ] = NULL;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ビットマップデータを作成する
 *
 * @parma work
 */
//-----------------------------------------------------------------------------------------
static void CreateBitmapDatas( RRG_WORK* work )
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief デフォルト初期化処理により, ビットマップデータの準備を行う
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupBitmapData_forDefault( RRG_WORK* work )
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 『報告を見る』ボタン用のビットマップデータを準備する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupBitmapData_forANALYZE_BUTTON( RRG_WORK* work )
{
  int x, y;
  GFL_BMP_DATA* srcBMP; 
  GFL_MSGDATA* msgData;
  STRBUF* strbuf;
  PRINTSYS_LSB color;
  const BITMAP_INIT_DATA* data;

  // コピーするキャラクタ番号
  int charaNo[ BMPOAM_ACTOR_ANALYZE_BUTTON_CHARA_SIZE_Y ][ BMPOAM_ACTOR_ANALYZE_BUTTON_CHARA_SIZE_X ] = 
  {
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5},
    {6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8},
  };

  // 初期化データ取得
  data = &BitmapInitData[ BMPOAM_ACTOR_ANALYZE_BUTTON ]; 

  // キャラクタデータを読み込む
  srcBMP = GFL_BMP_LoadCharacter( data->charaDataArcID, 
                                  data->charaDataArcDatID,
                                  FALSE, 
                                  work->heapID ); 
  // キャラデータをコピーする
  for( y=0; y < BMPOAM_ACTOR_ANALYZE_BUTTON_CHARA_SIZE_Y; y++)
  {
    for(x=0; x < BMPOAM_ACTOR_ANALYZE_BUTTON_CHARA_SIZE_X; x++)
    {
      GFL_BMP_Print( srcBMP, work->BmpData[ BMPOAM_ACTOR_ANALYZE_BUTTON ], 
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

  PRINTSYS_PrintColor( work->BmpData[ BMPOAM_ACTOR_ANALYZE_BUTTON ],
                       data->stringDrawPosX, data->stringDrawPosY,
                       strbuf, work->font, color ); 

  GFL_HEAP_FreeMemory( strbuf );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ビットマップデータを破棄する
 *
 * @parma work
 */
//-----------------------------------------------------------------------------------------
static void DeleteBitmapDatas( RRG_WORK* work )
{
  int idx;

  // 破棄
  for( idx=0; idx < BMPOAM_ACTOR_NUM; idx++ )
  {
    GFL_BMP_Delete( work->BmpData[ idx ] );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM システムの準備を行う
 *
 * @parma work
 */
//-----------------------------------------------------------------------------------------
static void SetupBmpOamSystem( RRG_WORK* work )
{
  // BMP-OAM システムを作成
  work->BmpOamSystem = BmpOam_Init( work->heapID, work->clactUnit[ CLUNIT_BMPOAM ] );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM システムの後片付けを行う
 *
 * @parma work
 */
//-----------------------------------------------------------------------------------------
static void CleanUpBmpOamSystem( RRG_WORK* work )
{
  // BMP-OAM システムを破棄
  BmpOam_Exit( work->BmpOamSystem );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM アクターを作成する
 *
 * @parma work
 */
//-----------------------------------------------------------------------------------------
static void CreateBmpOamActors( RRG_WORK* work )
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM アクターを破棄する
 *
 * @parma work
 */
//-----------------------------------------------------------------------------------------
static void DeleteBmpOamActors( RRG_WORK* work )
{
  int idx;

  for( idx=0; idx < BMPOAM_ACTOR_NUM; idx++ )
  {
    BmpOam_ActorDel( work->BmpOamActor[ idx ] );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitPaletteFadeSystem( RRG_WORK* work )
{ 
  // 初期化
  work->paletteFadeSystem = NULL;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムを準備する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupPaletteFadeSystem( RRG_WORK* work )
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムの後片付けを行う
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CleanUpPaletteFadeSystem( RRG_WORK* work )
{ 
  // パレットフェードのリクエストワークを破棄
  PaletteFadeWorkAllocFree( work->paletteFadeSystem, FADE_MAIN_BG );
  PaletteFadeWorkAllocFree( work->paletteFadeSystem, FADE_MAIN_OBJ );

  // フェード管理システムを破棄
  PaletteFadeFree( work->paletteFadeSystem );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitPaletteAnime( RRG_WORK* work )
{
  int idx;

  for( idx=0; idx < PALETTE_ANIME_NUM; idx++ )
  {
    work->paletteAnime[ idx ] = NULL;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを生成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreatePaletteAnime( RRG_WORK* work )
{
  int idx;

  for( idx=0; idx < PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] == NULL ); // 多重生成

    work->paletteAnime[ idx ] = PALETTE_ANIME_Create( work->heapID );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeletePaletteAnime( RRG_WORK* work )
{
  int idx;

  for( idx=0; idx < PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] );

    PALETTE_ANIME_Delete( work->paletteAnime[ idx ] );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークをセットアップする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupPaletteAnime( RRG_WORK* work )
{
  int idx;

  for( idx=0; idx < PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] );

    PALETTE_ANIME_Setup( work->paletteAnime[ idx ],
                         PaletteAnimeData[ idx ].destAdrs,
                         PaletteAnimeData[ idx ].srcAdrs,
                         PaletteAnimeData[ idx ].colorNum);
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークをクリーンアップする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CleanUpPaletteAnime( RRG_WORK* work )
{
  int idx;

  for( idx=0; idx < PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] );

    // 操作していたパレットを元に戻す
    PALETTE_ANIME_Reset( work->paletteAnime[ idx ] );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief 通信アイコンをセットアップする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void SetupWirelessIcon( const RRG_WORK* work )
{
  GFL_NET_ReloadIconTopOrBottom( TRUE, work->heapID );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief VBlank期間中のタスクを登録する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void RegisterVBlankTask( RRG_WORK* work )
{
  work->VBlankTask = GFUser_VIntr_CreateTCB( VBlankFunc, work, 0 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief VBlank期間中のタスクを解除する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ReleaseVBlankTask( RRG_WORK* work )
{ 
  GFL_TCB_DeleteTask( work->VBlankTask );
}


//=========================================================================================
// ◇LAYER 1 ユーティリティ
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief u8 に収まるように丸める
 *
 * @param num 丸める数値
 *
 * @return num を [0, 255] に丸めた値
 */
//-----------------------------------------------------------------------------------------
static u8 Bind_u8( int num )
{
  if( num < 0 ) { return 0; }
  if( 0xff < num ) { return 0xff; } 
  return num;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 侵入切断リクエストを発行する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void IntrudeShutdownRequest( const RRG_WORK* work )
{
  GAMESYS_WORK*     gameSystem = GetGameSystem( work );
  GAME_COMM_SYS_PTR game_comm  = GAMESYSTEM_GetGameCommSysPtr( gameSystem );

  GameCommSys_SetPalaceNotConnectFlag( game_comm, TRUE );
  if(GameCommSys_BootCheck( game_comm ) == GAME_COMM_NO_INVASION ) {
    GameCommSys_ExitReq( game_comm );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 侵入切断の完了をチェックする
 *
 * @param work
 *
 * @return 切断が完了した場合 TRUE
 */
//-----------------------------------------------------------------------------------------
static BOOL CheckIntrudeShutdown( const RRG_WORK* work )
{
  GAMESYS_WORK*     gameSystem = GetGameSystem( work );
  GAME_COMM_SYS_PTR game_comm  = GAMESYSTEM_GetGameCommSysPtr( gameSystem );
  GAME_COMM_NO      comm_no    = GameCommSys_BootCheck( game_comm );

  switch( comm_no ) {
  case GAME_COMM_NO_INVASION: // 侵入が終了されるのを待つ
    break;

  case GAME_COMM_NO_NULL:
    GAMESYSTEM_CommBootAlways( gameSystem );
    return TRUE;
    break;

  default:
    return TRUE;
    break;
  }

  return FALSE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 侵入接続を許可する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void RecoverIntrudeShutdown( const RRG_WORK* work )
{
  GAMESYS_WORK*     gameSystem = GetGameSystem( work );
  GAME_COMM_SYS_PTR game_comm  = GAMESYSTEM_GetGameCommSysPtr( gameSystem );

  //侵入接続許可
  GameCommSys_SetPalaceNotConnectFlag( game_comm, FALSE );  
}
