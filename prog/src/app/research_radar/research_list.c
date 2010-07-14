///////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー リスト画面
 * @file   research_list.c
 * @author obata
 * @date   2010.02.03
 */
///////////////////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include <gflib.h>
#include "bg_font.h"
#include "queue.h"
#include "palette_anime.h"
#include "research_list.h"
#include "research_list_index.h"
#include "research_list_def.h"
#include "research_list_data.cdat"
#include "research_list_recovery.h" // for RRL_RECOVERY_DATA
#include "research_common.h"
#include "research_common_data.cdat"
#include "research_data.h"

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

#include "../../../../resource/fldmapdata/flagwork/flag_define.h" // for FE_xxxx



//=========================================================================================
// ■リスト画面管理ワーク
//=========================================================================================
struct _RESEARCH_RADAR_LIST_WORK { 

  // 全画面共通ワーク
  RRC_WORK* commonWork; 

  // リスト画面の復帰データ
  RRL_RECOVERY_DATA* recoveryData;

  HEAPID       heapID; 
  GFL_FONT*    font;
  GFL_MSGDATA* message[ MESSAGE_NUM ];

  QUEUE*    stateQueue; // 状態キュー
  RRL_STATE state;      // 現在の状態
  u32       stateSeq;   // 状態内シーケンス番号
  u32       stateCount; // 状態カウンタ
  u32       waitFrame;  // フレーム経過待ち状態の待ち時間

  // メニュー項目
  MENU_ITEM menuCursorPos; // カーソル位置

  // 調査項目
  u8 selectableTopicNum; // 選択可能な調査項目の数
  u8 selectedTopicID;    // 選択した調査項目ID 
  u8 topicCursorPos;     // カーソル位置 ( == 調査項目ID )
  u8 topicCursorNextPos; // 移動後のカーソル位置 

  // スクロール
  int scrollCursorPos;      // スクロールカーソル位置
  int scrollCursorPos_prev; // 前回のスクロールカーソル位置
  int scrollStartPos;       // スクロール開始時のカーソル位置
  int scrollEndPos;         // スクロール終了時のカーソル位置
  int scrollFrames;         // スクロールフレーム数
  int scrollFrameCount;     // スクロールフレーム数カウンタ
  int sliderSpeed;          // スライダーの速度
  int sliderAccel;          // スライダーの加速度

  // キー入力
  u32 keyContCount; // 上下キー押しっぱなしカウンタ

  // タッチ領域
  GFL_UI_TP_HITTBL menuTouchHitTable[ MENU_TOUCH_AREA_NUM ];
  GFL_UI_TP_HITTBL topicTouchHitTable[ TOPIC_TOUCH_AREA_NUM ];
  GFL_UI_TP_HITTBL scrollTouchHitTable[ SCROLL_TOUCH_AREA_NUM ];

  // パレットフェード処理システム
  PALETTE_FADE_PTR paletteFadeSystem; 

  // パレットアニメーション
  PALETTE_ANIME* paletteAnime[ PALETTE_ANIME_NUM ];

  // 文字列描画オブジェクト
  BG_FONT* BGFont_topic[ TOPIC_ID_NUM ]; // 調査項目
  BG_FONT* BGFont_string[ BG_FONT_NUM ]; // 文字列

  // OBJ
  u32         objResRegisterIdx[ OBJ_RESOURCE_NUM ]; // リソースの登録インデックス
  GFL_CLUNIT* clactUnit[ CLUNIT_NUM ];               // セルアクターユニット
  GFL_CLWK*   clactWork[ CLWK_NUM ];                 // セルアクターワーク

  // BMP-OAM
  BMPOAM_SYS_PTR BmpOamSystem;                    // BMP-OAM システム
  BMPOAM_ACT_PTR BmpOamActor[ BMPOAM_ACTOR_NUM ]; // BMP-OAMアクター
  GFL_BMP_DATA*  BmpData[ BMPOAM_ACTOR_NUM ];     // 各アクターに対応するビットマップデータ

  // VBlank
  GFL_TCBSYS* VBlankTCBSystem; // VBlank期間中のタスク管理システム
  GFL_TCB*    VBlankTask;      // VBlankタイミング中に行うタスク

  // フラグ
  BOOL stateEndFlag;          // 現在の状態が終了したかどうか
  BOOL palFadeOutFlag;        // フェードアウト中かどうか
  BOOL topicCursorActiveFlag; // カーソルがアクティブかどうか
  BOOL finishFlag;            // リスト画面が終了したかどうか
  RRL_RESULT finishResult;    // リスト画面の終了結果
};



//=========================================================================================
// ◆関数インデックス
//=========================================================================================

//-----------------------------------------------------------------------------------------
// ◆LAYER 5 状態
//-----------------------------------------------------------------------------------------
// 状態ごとの処理
static void MainState_SETUP( RRL_WORK* work ); // RRL_STATE_SETUP
static void MainState_STANDBY( RRL_WORK* work ); // RRL_STATE_STANDBY
static void MainState_KEY_WAIT( RRL_WORK* work ); // RRL_STATE_KEY_WAIT
static void MainState_AUTO_SCROLL( RRL_WORK* work ); // RRL_STATE_AUTO_SCROLL
static void MainState_SLIDE_CONTROL( RRL_WORK* work ); // RRL_STATE_SLIDE_CONTROL
static void MainState_RETURN_FROM_KEYWAIT( RRL_WORK* work ); // RRL_STATE_RETURN_FROM_KEYWAIT
static void MainState_RETURN_FROM_STANDBY( RRL_WORK* work ); // RRL_STATE_RETURN_FROM_STANDBY
static void MainState_TO_CONFIRM_STANDBY( RRL_WORK* work ); // RRL_STATE_TO_CONFIRM_STANDBY
static void MainState_CONFIRM_STANDBY( RRL_WORK* work ); // RRL_STATE_CONFIRM_STANDBY
static void MainState_TO_CONFIRM_KEY_WAIT( RRL_WORK* work ); // RRL_STATE_TO_CONFIRM_KEY_WAIT
static void MainState_CONFIRM_KEY_WAIT( RRL_WORK* work ); // RRL_STATE_CONFIRM_KEY_WAIT
static void MainState_CONFIRM_CANCEL( RRL_WORK* work ); // RRL_STATE_CONFIRM_CANCEL
static void MainState_CONFIRM_DETERMINE( RRL_WORK* work ); // RRL_STATE_CONFIRM_DETERMINE
static void MainState_CLEAN_UP( RRL_WORK* work ); // RRL_STATE_CLEAN_UP 
// 状態制御
static void IncStateCount( RRL_WORK* work ); // 状態カウンタをインクリメントする
static void RegisterNextState( RRL_WORK* work, RRL_STATE nextSeq ); // 次の状態をキューに登録する
static void FinishCurrentState( RRL_WORK* work ); // 現在の状態を終了する
static void SwitchState( RRL_WORK* work ); // 処理状態を変更する
static RRL_STATE GetState( const RRL_WORK* work ); // 状態を取得する
static void SetState( RRL_WORK* work, RRL_STATE nextSeq ); // 処理状態を設定する
static u32 GetStateSeq( const RRL_WORK* work ); // 状態内シーケンス番号を取得する
static void IncStateSeq( RRL_WORK* work ); // 状態内シーケンス番号をインクリメントする
static void ResetStateSeq( RRL_WORK* work ); // 状態内シーケンス番号をリセットする
static SEQ_CHANGE_TRIG GetFinishReason( RRL_WORK* work ); // 画面遷移の方法を取得する
static void SetFinishReason( RRL_WORK* work, SEQ_CHANGE_TRIG reason ); // リスト画面終了の方法を登録する
static void SetFinishResult( RRL_WORK* work, RRL_RESULT result ); // 画面終了結果を設定する
static void SetWaitFrame( RRL_WORK* work, u32 frame ); // フレーム経過待ち状態の待ち時間を設定する
static u32 GetWaitFrame( const RRL_WORK* work ); // フレーム経過待ち状態の待ち時間を取得する
//-----------------------------------------------------------------------------------------
// ◆LAYER 4 機能
//-----------------------------------------------------------------------------------------
// メニュー項目カーソル
static void MoveMenuCursorUp( RRL_WORK* work ); // 上へ移動する
static void MoveMenuCursorDown( RRL_WORK* work ); // 下へ移動する
static void MoveMenuCursorDirect( RRL_WORK* work, MENU_ITEM menuItem ); // 直接移動する
// 調査項目カーソル
static void AdjustTopicCursor_to_ScrollCursor( RRL_WORK* work ); // カーソルが画面内に存在するように調整する
static void MoveTopicCursorDirect( RRL_WORK* work, u8 topicID ); // 直接移動する
//『戻る』ボタン
static void BlinkReturnButton( RRL_WORK* work ); //『戻る』ボタンを明滅させる
// オートスクロール
static void StartAutoScroll_to_Topic( RRL_WORK* work ); // オートスクロールを開始する ( 調査項目カーソル位置へ )
static void StartAutoScroll_for_Reset( RRL_WORK* work ); // オートスクロールを開始する ( 範囲外スクロールをリセット )
static void StartAutoScroll_to_Confirm( RRL_WORK* work ); // オートスクロールを開始する ( 調査項目確定の確認へ )
//-----------------------------------------------------------------------------------------
// ◆LAYER 3 個別操作
//-----------------------------------------------------------------------------------------
// セーブデータ
static u8 GetInvestigatingTopicID( const RRL_WORK* work ); // 現在調査中の調査項目IDを取得する
static void UpdateInvestigatingTopicID( const RRL_WORK* work ); // 調査する項目を更新する
// メニュー項目
static void ShiftMenuCursorPos( RRL_WORK* work, int stride ); // メニュー項目カーソル位置を変更する ( オフセット指定 )
static void SetMenuCursorPos( RRL_WORK* work, MENU_ITEM menuItem ); // メニュー項目カーソル位置を変更する ( 直値指定 ) 
static void SetMenuItemCursorOn( RRL_WORK* work, MENU_ITEM menuItem ); // カーソルが乗っている状態にする
static void SetMenuItemCursorOff( RRL_WORK* work, MENU_ITEM menuItem ); // カーソルが乗っていない状態にする
// 調査項目
static void SetTopicCursorPosDirect( RRL_WORK* work, int topciID ); // 調査項目カーソルの位置を設定する ( 直値指定 ) 
static void SetTopicCursorNextPos( RRL_WORK* work, int stride ); // 調査項目カーソルの移動先を設定する ( オフセット指定 )
static void SetTopicButtonCursorOn( RRL_WORK* work ); // カーソルが乗っている状態にする
static void SetTopicButtonCursorOff( RRL_WORK* work ); // カーソルが乗っていない状態にする
static void SetTopicButtonInvestigating( const RRL_WORK* work, u8 topicID ); // 調査中の状態にする
static void SetTopicButtonNotInvestigating( const RRL_WORK* work, u8 topicID ); // 調査中でない状態に戻す
static u8 CalcScreenLeftOfTopicButton( const RRL_WORK* work, u8 topicID ); // 調査項目の左上x座標を算出する ( スクリーン単位 )
static u8 CalcScreenTopOfTopicButton( const RRL_WORK* work, u8 topicID ); // 調査項目の左上y座標を算出する ( スクリーン単位 )
static int CalcDisplayLeftOfTopicButton( const RRL_WORK* work, u8 topicID ); // 調査項目ボタンの左上x座標を算出する ( ディスプレイ座標系・ドット単位 )
static int CalcDisplayTopOfTopicButton ( const RRL_WORK* work, u8 topicID ); // 調査項目ボタンの左上y座標を算出する ( ディスプレイ座標系・ドット単位 ) 
static int CalcDisplayBottomOfTopicButton ( const RRL_WORK* work, u8 topicID ); // 調査項目ボタンの右下y座標を算出する ( ディスプレイ座標系・ドット単位 ) 
static void UpdateTopicButtonMask( const RRL_WORK* work ); // 調査項目ボタンのスクロール回り込みを隠すためのウィンドウを更新する
static void UpdateInvestigatingIcon( const RRL_WORK* work ); // 調査中アイコンの表示状態を更新する
// 上画面の表示
static void UpdateTopicDetailStrings_at_Now( RRL_WORK* work ); // 上画面の調査項目説明文を更新する ( 現在のカーソル位置に合わせる )
static void UpdateTopicDetailStrings_at_Next( RRL_WORK* work ); // 上画面の調査項目説明文を更新する ( 次のカーソル位置に合わせる )
static void UpdateTopicDetailStrings_at( RRL_WORK* work, u8 topicID ); // 上画面の調査項目説明文を更新する ( 指定カーソル位置に合わせる )
static void HideTopicDetailStrints( RRL_WORK* work ); // 上画面の調査項目説明文の表示を隠す
static void ShowTopicDetailStrings( RRL_WORK* work ); // 上画面の調査項目説明文の表示を開始する
// スライダー
static void UpdateSliderDisp( const RRL_WORK* work ); // 現在のスクロール実効値に合わせて, スライダーの表示位置を更新する
static int CalcSliderPos_byScrollValue( const RRL_WORK* work ); // スクロール実効値から, スライダーの表示位置を計算する
static int CalcScrollCursorPos_bySliderPos( const RRL_WORK* work, int controlPos ); // スライダーの表示位置から, スクロールカーソル位置を算出する
static int GetSliderPos( const RRL_WORK* work ); // スライダーの表示位置を取得する
static void AccelerateSliderSpeed( RRL_WORK* work ); // スライダーの移動速度を更新する
static BOOL IsSliderMoving( const RRL_WORK* work ); // スライダーが動いているかどうかをチェックする
static void ObserveSliderSpeed( RRL_WORK* work ); // スライダーの移動速度を観測する
static void AdjustSliderSpeed_forFreeMove( RRL_WORK* work ); // スライダーの移動速度を調整する ( 自然移動用 )
static void AdjustSliderAccel_forFreeMove( RRL_WORK* work ); // スライダーの加速度を調整する ( 自然移動用 )
// スクロール
static void StartScroll( RRL_WORK* work, int startPos, int endPos, int frames ); // スクロールを開始する
static void UpdateScroll( RRL_WORK* work ); // スクロールを更新する
static BOOL CheckScrollEnd( RRL_WORK* work ); // スクロールが終了したかどうかを判定する
static void ShiftScrollCursorPos( RRL_WORK* work, int offset ); // スクロールカーソル位置を変更する ( オフセット指定 )
static void SetScrollCursorPos( RRL_WORK* work, int pos ); // スクロールカーソル位置を変更する ( 直値指定 )
static void SetScrollCursorPosForce( RRL_WORK* work, int pos ); // スクロールカーソル位置を変更する ( 直値指定・範囲限定なし )
static void AdjustScrollCursor( RRL_WORK* work ); // スクロール実効値に合わせ, スクロールカーソル位置を補正する
static void UpdateScrollValue( const RRL_WORK* work ); // スクロール実効値を更新する
static void AdjustScrollValue( const RRL_WORK* work ); // スクロール実効値をスクロールカーソル位置に合わせて更新する
static int GetScrollValue(); // スクロール実効値を取得する
static void SetScrollValue( int value ); // スクロール実効値を設定する
static int GetMinScrollCursorMarginPos(); // スクロールカーソル余白範囲の最小値を取得する
static int GetMaxScrollCursorMarginPos(); // スクロールカーソル余白範囲の最大値を取得する
static int CalcTopicID_byScrollCursorPos( int pos ); // 指定したスクロールカーソル位置にあるボタンの調査項目IDを計算する
static int CalcScrollCursorPosOfTopicButtonTop( int topicID ); // 指定した調査項目ボタンの上辺に該当するスクロールカーソル位置を計算する
static int CalcScrollCursorPosOfTopicButtonBottom( int topicID ); // 指定した調査項目ボタンの底辺に該当するスクロールカーソル位置を計算する
static int GetMaxScrollValue( const RRL_WORK* work ); // 最大スクロール値を取得する
static int GetMaxScrollCursorPos( const RRL_WORK* work ); // スクロールカーソルの最大値を取得する
static BOOL CheckScrollControlCan( const RRL_WORK* work ); // スクロール操作が可能かどうかを判定する
// タッチ範囲
static void UpdateTopicTouchArea( RRL_WORK* work ); // タッチ範囲を更新する
// 画面のフェードアウト・フェードイン
static void StartFadeIn( void ); // フェードインを開始する
static void StartFadeOut( void ); // フェードアウトを開始する
static BOOL CheckFadeEnd( void ); // フェードの終了をチェックする
// パレットフェード
static void StartPaletteFadeOut( RRL_WORK* work ); // パレットのフェードアウトを開始する
static void StartPaletteFadeIn ( RRL_WORK* work ); // パレットのフェードインを開始する
static BOOL IsPaletteFadeEnd( RRL_WORK* work ); // パレットのフェードが完了したかどうかを判定する
// パレットアニメーション
static void StartCommonPaletteAnime( RRL_WORK* work, COMMON_PALETTE_ANIME_INDEX index ); // 共通パレットアニメーションを開始する
static void StartPaletteAnime( RRL_WORK* work, PALETTE_ANIME_INDEX index ); // パレットアニメーションを開始する
static void StopPaletteAnime( RRL_WORK* work, PALETTE_ANIME_INDEX index ); // パレットアニメーションを停止する
static BOOL CheckPaletteAnimeEnd( const RRL_WORK* work, PALETTE_ANIME_INDEX index ); // パレットアニメーションの終了をチェックする
static void UpdatePaletteAnime( RRL_WORK* work );  // パレットアニメーションを更新する
static void UpdateCommonPaletteAnime( const RRL_WORK* work ); // 共通パレットアニメーションを更新する
// キー入力
static u32 GetKeyContCount( const RRL_WORK* work ); // キー押しっぱなしカウンタを取得する
static void IncKeyContCount( RRL_WORK* work ); // キー押しっぱなしカウンタをインクリメントする
static void ResetKeyContCount( RRL_WORK* work ); // キー押しっぱなしカウンタをリセットする
// VBlank
static void VBlankFunc( GFL_TCB* tcb, void* wk ); // VBlank 中の処理
//-----------------------------------------------------------------------------------------
// ◆LAYER 2 取得・設定・判定
//-----------------------------------------------------------------------------------------
// システム
static GAMESYS_WORK* GetGameSystem( const RRL_WORK* work );
static GAMEDATA* GetGameData( const RRL_WORK* work );
static void SetHeapID( RRL_WORK* work, HEAPID heapID );
static void SetCommonWork( RRL_WORK* work, RRC_WORK* commonWork );
static const RRC_WORK* GetCommonWork( const RRL_WORK* work );
static void SetRecoveryData( RRL_WORK* work, RRL_RECOVERY_DATA* recoveryData );
static void LoadRecoveryData( RRL_WORK* work );
static void SaveRecoveryData( RRL_WORK* work );
static BOOL CheckForceReturnFlag( const RRL_WORK* work );
// 調査項目
static int GetNextTopicID( const RRL_WORK* work, int topicID ); // 次の調査項目IDを取得する
static int GetPrevTopicID( const RRL_WORK* work, int topicID ); // 前の調査項目IDを取得する
static u8 GetSelectableTopicNum( const RRL_WORK* work ); // 選択可能な調査項目の数を取得する
static u8 GetSelectedTopicID( const RRL_WORK* work ); // 選択中の調査項目IDを取得する
static void SetSelectedTopicID( RRL_WORK* work, u8 topicID ); // 調査項目IDを選択する
static void ResetSelectedTopicID( RRL_WORK* work ); // 調査項目IDの選択を解除する
static BOOL IsTopicIDSelected( const RRL_WORK* work ); // 選択済みかを判定する
static BOOL CheckTopicCanSelect( const RRL_WORK* work, u8 topicID ); // 調査項目を選択可能かどうかをチェックする
// OBJ
static u32 GetObjResourceRegisterIndex( const RRL_WORK* work, OBJ_RESOURCE_ID resID ); // OBJリソースの登録インデックスを取得する
static GFL_CLUNIT* GetClactUnit( const RRL_WORK* work, CLUNIT_INDEX unitIdx ); // セルアクターユニットを取得する
static GFL_CLWK* GetClactWork( const RRL_WORK* work, CLWK_INDEX wkIdx ); // セルアクターワークを取得する
// BMP-OAM
static void BmpOamSetDrawEnable( RRL_WORK* work, BMPOAM_ACTOR_INDEX BmpOamActorIdx, BOOL enable ); // 表示するかどうかを設定する
static BMPOAM_ACT_PTR GetBmpOamActorOfMenuItem( const RRL_WORK* work, MENU_ITEM menuItem ); // メニュー項目に対応するBMP-OAMアクターを取得する
//-----------------------------------------------------------------------------------------
// ◆LAYER 1 生成・初期化・準備・後片付け・破棄
//-----------------------------------------------------------------------------------------
static RRL_WORK* CreateListWork( HEAPID heapID ); // リスト画面管理ワーク 生成
static void InitListWork( RRL_WORK* work ); // リスト画面管理ワーク 初期化
static void DeleteListWork( RRL_WORK* work ); // リスト画面管理ワーク 破棄
static void SetupSelectableTopicNum( RRL_WORK* work ); // 選択可能な調査項目の数 準備
static void CreateFont( RRL_WORK* work ); // フォントデータ 生成
static void InitFont( RRL_WORK* work ); // フォントデータ 初期化
static void DeleteFont( RRL_WORK* work ); // フォントデータ 破棄
static void CreateMessages( RRL_WORK* work ); // メッセージデータ 生成
static void InitMessages( RRL_WORK* work ); // メッセージデータ 初期化
static void DeleteMessages( RRL_WORK* work ); // メッセージデータ 破棄
static void CreateStateQueue( RRL_WORK* work ); // 状態キュー 生成
static void InitStateQueue( RRL_WORK* work ); // 状態キュー 初期化
static void DeleteStateQueue( RRL_WORK* work ); // 状態キュー 破棄
static void SetupTouchArea( RRL_WORK* work ); // タッチ領域 準備
static void SetupBG( RRL_WORK* work ); // BG 全般 準備
static void SetupSubBG_WINDOW( RRL_WORK* work ); // SUB-BG ( ウィンドウ面 )  準備
static void SetupSubBG_FONT( RRL_WORK* work ); // SUB-BG ( フォント面 )  準備
static void SetupMainBG_BAR( RRL_WORK* work ); // MAIN-BG ( バー面 )  準備
static void SetupMainBG_WINDOW( RRL_WORK* work ); // MAIN-BG ( ウィンドウ面 )  準備
static void SetupMainBG_FONT( RRL_WORK* work ); // MAIN-BG ( フォント面 )  準備
static void CleanUpBG( RRL_WORK* work ); // BG 全般 後片付け
static void CleanUpSubBG_WINDOW( RRL_WORK* work ); // SUB-BG ( ウィンドウ面 )  後片付け
static void CleanUpSubBG_FONT( RRL_WORK* work ); // SUB-BG ( フォント面 )  後片付け
static void CleanUpMainBG_BAR( RRL_WORK* work ); // MAIN-BG ( バー面 )  後片付け
static void CleanUpMainBG_WINDOW( RRL_WORK* work ); // MAIN-BG ( ウィンドウ面 )  後片付け
static void CleanUpMainBG_FONT( RRL_WORK* work ); // MAIN-BG ( フォント面 )  後片付け
static void CreateBGFonts( RRL_WORK* work ); // 文字列描画オブジェクト 生成
static void InitBGFonts( RRL_WORK* work ); // 文字列描画オブジェクト 初期化
static void DeleteBGFonts( RRL_WORK* work ); // 文字列描画オブジェクト 破棄
static void InitOBJResources( RRL_WORK* work ); // OBJ リソース 初期化
static void RegisterSubObjResources( RRL_WORK* work ); // SUB-OBJ リソース 登録
static void ReleaseSubObjResources( RRL_WORK* work ); // SUB-OBJ リソース 解放
static void RegisterMainObjResources( RRL_WORK* work ); // MAIN-OBJ リソース 登録
static void ReleaseMainObjResources( RRL_WORK* work ); // MAIN-OBJ リソース 解放
static void InitClactUnits( RRL_WORK* work ); // セルアクターユニット 初期化
static void CreateClactUnits( RRL_WORK* work ); // セルアクターユニット 生成
static void DeleteClactUnits( RRL_WORK* work ); // セルアクターユニット 破棄
static void InitClactWorks( RRL_WORK* work ); // セルアクターワーク 初期化
static void CreateClactWorks( RRL_WORK* work ); // セルアクターワーク 生成
static void DeleteClactWorks( RRL_WORK* work ); // セルアクターワーク 破棄
static void InitBitmapDatas( RRL_WORK* work ); // ビットマップデータ 初期化
static void CreateBitmapDatas( RRL_WORK* work ); // ビットマップデータ 作成
static void SetupBitmapData_forDefault( RRL_WORK* work ); // ビットマップデータ 準備 ( デフォルト )
static void SetupBitmapData_forOK( RRL_WORK* work ); // ビットマップデータ 準備 (「けってい」)
static void SetupBitmapData_forCANCEL( RRL_WORK* work ); // ビットマップデータ 準備 (「やめる」)
static void DeleteBitmapDatas( RRL_WORK* work ); // ビットマップデータ 破棄
static void SetupBmpOamSystem( RRL_WORK* work ); // BMP-OAM システム 準備
static void CleanUpBmpOamSystem( RRL_WORK* work ); // BMP-OAM システム 後片付け
static void CreateBmpOamActors( RRL_WORK* work ); // BMP-OAM アクター 作成
static void DeleteBmpOamActors( RRL_WORK* work ); // BMP-OAM アクター 破棄
static void InitPaletteFadeSystem( RRL_WORK* work ); // パレットフェードシステム 初期化
static void SetupPaletteFadeSystem( RRL_WORK* work ); // パレットフェードシステム 準備
static void CleanUpPaletteFadeSystem( RRL_WORK* work ); // パレットフェードシステム 後片付け
static void CreatePaletteAnime( RRL_WORK* work ); // パレットアニメーションワーク 生成
static void InitPaletteAnime( RRL_WORK* work ); // パレットアニメーションワーク 初期化
static void DeletePaletteAnime( RRL_WORK* work ); // パレットアニメーションワーク 破棄
static void SetupPaletteAnime( RRL_WORK* work ); // パレットアニメーションワーク 準備
static void CleanUpPaletteAnime( RRL_WORK* work );  // パレットアニメーションワーク 後片付け
static void SetupScrollBar( RRL_WORK* work ); // スクロールバー 準備
static void SetupWirelessIcon( const RRL_WORK* work ); // 通信アイコン 準備
static void RegisterVBlankTask( RRL_WORK* work ); // VBlank タスク 登録
static void ReleaseVBlankTask ( RRL_WORK* work ); // VBlank タスク 解除




//=========================================================================================
// ■public functions
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief リスト画面管理ワークを生成する
 *
 * @param commonWork   全画面共通ワーク
 * @param recoveryData リスト画面の復帰データ
 *
 * @return リスト画面管理ワーク
 */
//-----------------------------------------------------------------------------------------
RRL_WORK* RRL_CreateWork( RRC_WORK* commonWork, RRL_RECOVERY_DATA* recoveryData )
{
  RRL_WORK* work;
  HEAPID heapID;

  heapID = RRC_GetHeapID( commonWork );

  // ワークを生成
  work = CreateListWork( heapID );

  // ワークを初期化
  InitListWork( work );
  SetHeapID( work, heapID );
  SetCommonWork( work, commonWork );
  SetRecoveryData( work, recoveryData );

  return work;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査初期画面ワークの破棄
 *
 * @param heapID
 */
//-----------------------------------------------------------------------------------------
void RRL_DeleteWork( RRL_WORK* work )
{
  DeleteStateQueue( work );
  DeleteListWork( work );
} 

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査初期画面 メイン動作
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
void RRL_Main( RRL_WORK* work )
{
  switch( GetState(work) ) {
  case RRL_STATE_SETUP:               MainState_SETUP( work );               break;
  case RRL_STATE_STANDBY:             MainState_STANDBY( work );             break;
  case RRL_STATE_KEY_WAIT:            MainState_KEY_WAIT( work );            break;
  case RRL_STATE_AUTO_SCROLL:         MainState_AUTO_SCROLL( work );         break;
  case RRL_STATE_SLIDE_CONTROL:       MainState_SLIDE_CONTROL( work );       break;
  case RRL_STATE_RETURN_FROM_KEYWAIT: MainState_RETURN_FROM_KEYWAIT( work ); break;
  case RRL_STATE_RETURN_FROM_STANDBY: MainState_RETURN_FROM_STANDBY( work ); break;
  case RRL_STATE_TO_CONFIRM_STANDBY:  MainState_TO_CONFIRM_STANDBY( work );  break;
  case RRL_STATE_CONFIRM_STANDBY:     MainState_CONFIRM_STANDBY( work );     break;
  case RRL_STATE_TO_CONFIRM_KEY_WAIT: MainState_TO_CONFIRM_KEY_WAIT( work ); break;
  case RRL_STATE_CONFIRM_KEY_WAIT:    MainState_CONFIRM_KEY_WAIT( work );    break;
  case RRL_STATE_CONFIRM_CANCEL:      MainState_CONFIRM_CANCEL( work );      break;
  case RRL_STATE_CONFIRM_DETERMINE:   MainState_CONFIRM_DETERMINE( work );   break;
  case RRL_STATE_CLEAN_UP:            MainState_CLEAN_UP( work );            break;
  case RRL_STATE_FINISH:              break;
  default:  GF_ASSERT(0);
  }

  if( RRL_IsFinished(work) == FALSE ) {
    UpdateCommonPaletteAnime( work ); // 共通パレットアニメーションを更新
    UpdatePaletteAnime( work );       // パレットアニメーションを更新
    GFL_CLACT_SYS_Main();             // セルアクターシステム メイン処理
    IncStateCount( work );            // 状態カウンタをインクリメント
    SwitchState( work );              // 状態を更新
  }
}


//-----------------------------------------------------------------------------------------
/**
 * @brief リスト画面の終了をチェックする
 *
 * @param work
 *
 * @return リスト画面が終了している場合 TRUE
 */
//-----------------------------------------------------------------------------------------
RRL_IsFinished( const RRL_WORK* work )
{
  return work->finishFlag;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief リスト画面の終了結果を取得する
 *
 * @param work
 *
 * @return リスト画面の終了結果
 */
//-----------------------------------------------------------------------------------------
RRL_RESULT RRL_GetResult( const RRL_WORK* work )
{
  return work->finishResult;
}



//=========================================================================================
// ■LAYER 5 状態
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief 準備状態 ( RRL_STATE_SETUP ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_SETUP( RRL_WORK* work )
{ 
  CreateStateQueue( work );
  CreateFont( work );
  CreateMessages( work );
  SetupTouchArea( work );
  UpdateTopicTouchArea( work );
  SetupSelectableTopicNum( work );

  // BG 準備
  SetupBG( work );
  SetupSubBG_WINDOW( work );
  SetupSubBG_FONT( work );
  SetupMainBG_BAR( work );
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
  SetupBitmapData_forOK( work );
  SetupBitmapData_forCANCEL( work );
  SetupBmpOamSystem( work );
  CreateBmpOamActors( work );

  // パレットフェードシステム 準備
  SetupPaletteFadeSystem( work );

  // パレットアニメーション
  CreatePaletteAnime( work );
  SetupPaletteAnime( work );

  RegisterVBlankTask( work ); // VBkankタスク登録
  SetupWirelessIcon( work );  // 通信アイコン
  SetupScrollBar( work );     // スクロールバー

  // パレットアニメーション開始
  StartPaletteAnime( work, PALETTE_ANIME_TOPIC_CURSOR_ON ); 

  // 確認メッセージと選択項目を消去
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CONFIRM, FALSE );
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_OK, FALSE );
  BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CANCEL, FALSE );

  // 画面復帰データを反映
  LoadRecoveryData( work );

  // 調査中の項目がある場合
  {
    u8 topic_id = GetInvestigatingTopicID( work );
    
    if( topic_id != INVESTIGATING_QUESTION_NULL ) {
      // 調査中の項目を選択状態にする
      SetTopicButtonInvestigating( work, topic_id ); // 選択している状態にする
      SetSelectedTopicID( work, topic_id );          // 調査中の項目を選択
      UpdateInvestigatingIcon( work );               // 調査項目選択アイコンを更新
    }
  } 

  UpdateTopicTouchArea( work );            // タッチ範囲を更新
  UpdateSliderDisp( work );                // スライダー
  UpdateInvestigatingIcon( work );         // 調査項目選択アイコンを更新
  UpdateTopicButtonMask( work );           // 調査項目のマスクウィンドを更新
  UpdateTopicDetailStrings_at_Now( work ); // 上画面の詳細表示をカーソル位置に合わせる

  // 次の状態遷移をセット
  if( GetFinishReason( work ) == SEQ_CHANGE_BY_BUTTON ) {
    SetTopicButtonCursorOn( work );                // カーソルが乗っている状態にする
    ShowTopicDetailStrings( work );                // 上画面の詳細表示開始
    FinishCurrentState( work );                    // RRL_STATE_SETUP 状態終了
    RegisterNextState( work, RRL_STATE_KEY_WAIT ); // => RRL_STATE_KEY_WAIT 
  }
  else {
    SetTopicButtonCursorOff( work );               // カーソルが乗っていない状態にする
    HideTopicDetailStrints( work );                // 上画面の詳細文字列は非表示
    FinishCurrentState( work );                    // RRL_STATE_SETUP 状態終了
    RegisterNextState( work, RRL_STATE_STANDBY );  // => RRL_STATE_STANDBY 
  }

  // フェードイン開始
  StartFadeIn(); 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 最初のキー入力待ち状態 ( RRL_STATE_STANDBY ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_STANDBY( RRL_WORK* work )
{
  int trg;
  int touchTrg;
  int commonTouch;
  u32 touch_x, touch_y;

  trg   = GFL_UI_KEY_GetTrg();
  touchTrg = GFL_UI_TP_HitTrg( work->topicTouchHitTable );
  commonTouch = GFL_UI_TP_HitTrg( RRC_GetHitTable(work->commonWork) ); 
  GFL_UI_TP_GetPointCont( &touch_x, &touch_y );

  //--------------------------------------
  // 強制終了 or 「もどる」ボタンをタッチ
  if( CheckForceReturnFlag( work ) || (commonTouch == COMMON_TOUCH_AREA_RETURN_BUTTON) ) {
    PMSND_PlaySE( SEQ_SE_CANCEL1 );                           // キャンセル音
    BlinkReturnButton( work );                                //『戻る』ボタンを明滅させる
    SetFinishReason( work, SEQ_CHANGE_BY_TOUCH );             // タッチで終了
    FinishCurrentState( work );                               // RRL_STATE_STANDBY 状態終了
    RegisterNextState( work, RRL_STATE_RETURN_FROM_STANDBY ); // => RRL_STATE_RETURN_FROM_STANDBY 
    return;
  }

  //-----------
  // B ボタン
  if( trg & PAD_BUTTON_B ) {
    PMSND_PlaySE( SEQ_SE_CANCEL1 );                           // キャンセル音
    BlinkReturnButton( work );                                //『戻る』ボタンを明滅させる
    SetFinishReason( work, SEQ_CHANGE_BY_BUTTON );            // ボタンで終了
    FinishCurrentState( work );                               // RRL_STATE_STANDBY 状態終了
    RegisterNextState( work, RRL_STATE_RETURN_FROM_STANDBY ); // => RRL_STATE_RETURN_FROM_STANDBY 
    return;
  }

  //----------------------
  // 十字キー or A ボタン
  if( (trg & PAD_KEY_UP) || (trg & PAD_KEY_DOWN) ||
      (trg & PAD_KEY_LEFT) || (trg & PAD_KEY_RIGHT) || (trg & PAD_BUTTON_A) ) {
    SetTopicButtonCursorOn( work );                 // カーソルが乗っている状態にする
    ShowTopicDetailStrings( work );                 // 上画面の詳細表示開始
    FinishCurrentState( work );                     // RRL_STATE_STANDBY 状態終了
    RegisterNextState( work, RRL_STATE_KEY_WAIT );  // => RRL_STATE_KEY_WAIT 
    return;
  }

  //----------------------------
  //「調査項目」ボタンをタッチ
  if( (TOPIC_TOUCH_AREA_TOPIC_0 <= touchTrg) && (touchTrg <= TOPIC_TOUCH_AREA_TOPIC_9) ) {
    // 範囲内をタッチ
    if( (TOPIC_BUTTON_TOUCH_AREA_TOP <= touch_y) && (touch_y <= TOPIC_BUTTON_TOUCH_AREA_BOTTOM) ) {
      // 選択可能
      if( CheckTopicCanSelect( work, touchTrg ) == TRUE ) {
        MoveTopicCursorDirect( work, touchTrg );                 // カーソル移動
        SetSelectedTopicID( work, work->topicCursorPos );        // カーソル位置の調査項目を選択
        PMSND_PlaySE( SEQ_SE_DECIDE1 );                          // 決定音
        StartPaletteAnime( work, PALETTE_ANIME_TOPIC_SELECT );   // 項目選択時のパレットアニメ開始
        UpdateTopicDetailStrings_at_Now( work );                 // 上画面の調査項目説明文を更新
        ShowTopicDetailStrings( work );                          // 上画面の詳細表示開始
        FinishCurrentState( work );                              // RRL_STATE_STANDBY 状態終了
        RegisterNextState( work, RRL_STATE_TO_CONFIRM_STANDBY ); // => RRL_STATE_TO_CONFIRM_STANDBY 
      }
    }
    return;
  } 

  //----------------
  // スクロールバー
  if( ( GFL_UI_TP_HitCont( work->scrollTouchHitTable ) == SCROLL_TOUCH_AREA_BAR ) ||
      ( GFL_UI_TP_HitTrg( work->scrollTouchHitTable ) == SCROLL_TOUCH_AREA_BAR ) ) {
    // スクロール操作可能
    if( CheckScrollControlCan( work ) == TRUE ) {
      FinishCurrentState( work );                         // RRL_STATE_KEY_WAIT 状態終了
      RegisterNextState( work, RRL_STATE_SLIDE_CONTROL ); // => RRL_STATE_SLIDE_CONTROL 
      RegisterNextState( work, RRL_STATE_STANDBY );       // ==> RRL_STATE_STANDBY 
    }
    return;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief キー入力待ち状態 ( RRL_STATE_KEY_WAIT ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_KEY_WAIT( RRL_WORK* work )
{ 
  int key, trg;
  int touchTrg;
  int commonTouch;
  u32 touch_x, touch_y;

  key = GFL_UI_KEY_GetCont();
  trg = GFL_UI_KEY_GetTrg();
  touchTrg = GFL_UI_TP_HitTrg( work->topicTouchHitTable );
  commonTouch = GFL_UI_TP_HitTrg( RRC_GetHitTable(work->commonWork) );
  GFL_UI_TP_GetPointCont( &touch_x, &touch_y );

  // 押しっぱなしチェック
  if( key & (PAD_KEY_UP + PAD_KEY_DOWN) ) {
    IncKeyContCount( work );
  }
  else {
    ResetKeyContCount( work );
  }

  //--------------------------------------
  // 強制終了 or 「もどる」ボタンをタッチ
  if( CheckForceReturnFlag( work ) || (commonTouch == COMMON_TOUCH_AREA_RETURN_BUTTON) ) {
    PMSND_PlaySE( SEQ_SE_CANCEL1 );                           // キャンセル音
    BlinkReturnButton( work );                                //『戻る』ボタンを明滅させる
    SetFinishReason( work, SEQ_CHANGE_BY_TOUCH );             // タッチで終了
    FinishCurrentState( work );                               // RRL_STATE_KEY_WAIT 状態終了
    RegisterNextState( work, RRL_STATE_RETURN_FROM_KEYWAIT ); // => RRL_STATE_RETURN_FROM_KEYWAIT 
    return;
  }

  //--------
  // ↑キー 
  if( ( trg & PAD_KEY_UP ) ||
      ( (key & PAD_KEY_UP) && (0 < work->topicCursorPos) && (10 < GetKeyContCount(work)) ) ) {
    SetTopicButtonCursorOff( work );                   // 移動前の項目を元に戻す
    SetTopicCursorNextPos( work, -1 );                 // 移動先を設定
    FinishCurrentState( work );                        // RRL_STATE_KEY_WAIT 状態終了
    RegisterNextState( work, RRL_STATE_AUTO_SCROLL );  // => RRL_STATE_AUTO_SCROLL 
    RegisterNextState( work, RRL_STATE_KEY_WAIT );     // ==> RRL_STATE_KEY_WAIT 
    return;
  } 
  //--------
  // ↓キー
  if( ( trg & PAD_KEY_DOWN ) ||
      ( (key & PAD_KEY_DOWN) && (work->topicCursorPos < GetSelectableTopicNum(work)-1) && (10 < GetKeyContCount(work)) ) ) {
    SetTopicButtonCursorOff( work );                   // 移動前の項目を元に戻す
    SetTopicCursorNextPos( work, 1 );                  // 移動先を設定
    FinishCurrentState( work );                        // RRL_STATE_KEY_WAIT 状態終了
    RegisterNextState( work, RRL_STATE_AUTO_SCROLL );  // => RRL_STATE_AUTO_SCROLL 
    RegisterNextState( work, RRL_STATE_KEY_WAIT );     // ==> RRL_STATE_KEY_WAIT 
    return;
  } 

  //----------
  // A ボタン
  if( trg & PAD_BUTTON_A ) {
    StartPaletteAnime( work, PALETTE_ANIME_TOPIC_SELECT );    // 項目選択時のパレットアニメ開始
    SetSelectedTopicID( work, work->topicCursorPos );         // カーソル位置の調査項目を選択
    PMSND_PlaySE( SEQ_SE_DECIDE1 );                           // 決定音
    RegisterNextState( work, RRL_STATE_TO_CONFIRM_KEY_WAIT ); // => RRL_STATE_TO_CONFIRM_KEY_WAIT 
    FinishCurrentState( work );                               // RRL_STATE_KEY_WAIT 状態終了
    return;
  } 

  //----------------------------
  //「調査項目」ボタン をタッチ
  if( (TOPIC_TOUCH_AREA_TOPIC_0 <= touchTrg) && (touchTrg <= TOPIC_TOUCH_AREA_TOPIC_9) ) {
    // 範囲内をタッチ
    if( (TOPIC_BUTTON_TOUCH_AREA_TOP <= touch_y) && (touch_y <= TOPIC_BUTTON_TOUCH_AREA_BOTTOM) ) {
      // 選択可能
      if( CheckTopicCanSelect( work, touchTrg ) == TRUE ) {
        MoveTopicCursorDirect( work, touchTrg );                 // カーソル移動
        SetSelectedTopicID( work, work->topicCursorPos );        // カーソル位置の調査項目を選択
        StartPaletteAnime( work, PALETTE_ANIME_TOPIC_SELECT );   // 項目選択時のパレットアニメ開始
        PMSND_PlaySE( SEQ_SE_DECIDE1 );                          // 決定音
        UpdateTopicDetailStrings_at_Now( work );                 // 上画面の調査項目説明文を更新
        FinishCurrentState( work );                              // RRL_STATE_KEY_WAIT 状態終了
        RegisterNextState( work, RRL_STATE_TO_CONFIRM_STANDBY ); // => RRL_STATE_TO_CONFIRM_STANDBY 
      }
    }
    return;
  }

  //----------
  // B ボタン
  if( trg & PAD_BUTTON_B ) {
    PMSND_PlaySE( SEQ_SE_CANCEL1 );                           // キャンセル音
    BlinkReturnButton( work );                                //『戻る』ボタンを明滅させる
    SetFinishReason( work, SEQ_CHANGE_BY_BUTTON );            // ボタンで終了
    FinishCurrentState( work );                               // RRL_STATE_KEY_WAIT 状態終了
    RegisterNextState( work, RRL_STATE_RETURN_FROM_KEYWAIT ); // => RRL_STATE_RETURN_FROM_KEYWAIT 
    return;
  } 

  //----------------
  // スクロールバー
  if( ( GFL_UI_TP_HitCont( work->scrollTouchHitTable ) == SCROLL_TOUCH_AREA_BAR ) ||
      ( GFL_UI_TP_HitTrg( work->scrollTouchHitTable ) == SCROLL_TOUCH_AREA_BAR ) ) {
    // スクロール操作可能
    if( CheckScrollControlCan( work ) == TRUE ) {
      SetTopicButtonCursorOff( work );                    // カーソルを隠す
      HideTopicDetailStrints( work );                     // 上画面の詳細表示を隠す
      FinishCurrentState( work );                         // RRL_STATE_KEY_WAIT 状態終了
      RegisterNextState( work, RRL_STATE_SLIDE_CONTROL ); // => RRL_STATE_SLIDE_CONTROL 
      RegisterNextState( work, RRL_STATE_STANDBY );       // ==> RRL_STATE_STANDBY 
    }
    return;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロール完了待ち状態 ( RRL_STATE_AUTO_SCROLL ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_AUTO_SCROLL( RRL_WORK* work )
{
  switch( GetStateSeq( work ) ) {
  case 0:
    StartAutoScroll_to_Topic( work );                         // オートスクロール開始
    MoveTopicCursorDirect( work, work->topicCursorNextPos );  // カーソルを移動
    UpdateTopicDetailStrings_at_Now( work );                  // 上画面の調査項目説明文を更新
    IncStateSeq( work );
    break;

  case 1:
    // スクロール処理
    UpdateScroll( work );            // スクロールを更新
    UpdateScrollValue( work );       // スクロール実効値を更新
    UpdateTopicTouchArea( work );    // タッチ範囲を更新
    UpdateSliderDisp( work );        // スライダー
    UpdateInvestigatingIcon( work ); // 調査項目選択アイコンを更新
    UpdateTopicButtonMask( work );   // 調査項目のマスクウィンドを更新

    // スクロール終了
    if( CheckScrollEnd(work) ) {
      UpdateTopicButtonMask( work ); // ウィンドウを切る
      FinishCurrentState( work );    // RRL_STATE_AUTO_SCROLL 状態終了
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロール操作状態 ( RRL_STATE_SLIDE_CONTROL ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_SLIDE_CONTROL( RRL_WORK* work )
{
  u32 x, y;
  BOOL touch;

  touch = GFL_UI_TP_GetPointCont( &x, &y );

  switch( GetStateSeq(work) ) {
  case 0: 
    // タッチが離された
    if( touch == FALSE ) {
      AdjustSliderSpeed_forFreeMove( work ); // スライダーの速度を調整
      AdjustSliderAccel_forFreeMove( work ); // スライダーの加速度を調整
      IncStateSeq( work ); 
      break;
    }

    // スクロール処理
    {
      int scrollCursorPos;
      scrollCursorPos = CalcScrollCursorPos_bySliderPos( work, y ); // タッチ場所から, スクロールカーソル位置を算出
      SetScrollCursorPos( work, scrollCursorPos );// スクロールカーソル位置を更新
      AdjustScrollValue( work );                  // スクロール実効値を更新
      UpdateTopicTouchArea( work );               // タッチ範囲を更新する
      UpdateSliderDisp( work );                   // スライダーを更新
      UpdateInvestigatingIcon( work );            // 調査項目選択アイコンを更新
      ObserveSliderSpeed( work );                 // スライダーの移動速度を観測
    }

    break;

  case 1:
    // スクロール処理
    {
      int scrollCursorPos;
      scrollCursorPos = work->scrollCursorPos + work->sliderSpeed;
      SetScrollCursorPos( work, scrollCursorPos );// スクロールカーソル位置を更新
      AdjustScrollValue( work );                  // スクロール実効値を更新
      UpdateTopicTouchArea( work );               // タッチ範囲を更新する
      UpdateSliderDisp( work );                   // スライダーを更新
      UpdateInvestigatingIcon( work );            // 調査項目選択アイコンを更新
    } 

    // スライダーの速度を更新
    AccelerateSliderSpeed( work );

    // スライダーが停止
    if( IsSliderMoving( work ) == FALSE ) {
      UpdateTopicDetailStrings_at_Now( work ); // 上画面の調査項目説明文を更新
      FinishCurrentState( work );
    }
    break;
  }

  // カーソル位置調整
  AdjustTopicCursor_to_ScrollCursor( work );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定の確認状態 ( RRL_STATE_CONFIRM_STANDBY ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_CONFIRM_STANDBY( RRL_WORK* work )
{
  int trg;
  int touchTrg;

  trg = GFL_UI_KEY_GetTrg();
  touchTrg = GFL_UI_TP_HitTrg( work->menuTouchHitTable );

  //----------------------
  // 十字キー or A ボタン
  if( (trg & PAD_KEY_UP) || (trg & PAD_KEY_DOWN) ||
      (trg & PAD_KEY_LEFT) || (trg & PAD_KEY_RIGHT) || (trg & PAD_BUTTON_A) ) {
    FinishCurrentState( work );                               // RRL_STATE_CONFIRM_STANDBY 状態終了
    RegisterNextState( work, RRL_STATE_TO_CONFIRM_KEY_WAIT ); // => RRL_STATE_TO_CONFIRM_KEY_WAIT 
    return;
  }

  //---------------------------
  //「けってい」ボタンをタッチ
  if( touchTrg == MENU_TOUCH_AREA_OK_BUTTON ) {
    MoveMenuCursorDirect( work, MENU_ITEM_DETERMINATION_OK ); // カーソル移動
    StopPaletteAnime( work, PALETTE_ANIME_MENU_CURSOR_ON );   // カーソルが乗っている時のパレットアニメを停止
    StartPaletteAnime( work, PALETTE_ANIME_MENU_SELECT );     // 選択時のパレットアニメを開始
    SetFinishReason( work, SEQ_CHANGE_BY_TOUCH );             // タッチで終了
    FinishCurrentState( work );                               // RRL_STATE_CONFIRM_STANDBY 状態終了
    RegisterNextState( work, RRL_STATE_CONFIRM_DETERMINE );   // => RRL_STATE_CONFIRM_DETERMINE 
    return;
  } 

  //-----------------------------------------
  // 強制終了 or B ボタン or 「やめる」ボタン
  if( CheckForceReturnFlag( work ) || 
      (trg & PAD_BUTTON_B) || (touchTrg == MENU_TOUCH_AREA_CANCEL_BUTTON) ) {
    MoveMenuCursorDirect( work, MENU_ITEM_DETERMINATION_CANCEL ); // カーソル移動
    StopPaletteAnime( work, PALETTE_ANIME_MENU_CURSOR_ON );       // カーソルが乗っている時のパレットアニメを停止
    StartPaletteAnime( work, PALETTE_ANIME_MENU_SELECT );         // 選択時のパレットアニメを開始
    PMSND_PlaySE( SEQ_SE_CANCEL1 );                               // キャンセル音
    FinishCurrentState( work );                                   // RRL_STATE_CONFIRM_STANDBY 状態終了
    RegisterNextState( work, RRL_STATE_CONFIRM_CANCEL );          // => RRL_STATE_CONFIRM_CANCEL 
    RegisterNextState( work, RRL_STATE_KEY_WAIT );                // ==> RRL_STATE_KEY_WAIT 
    return;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定の確認状態 ( RRL_STATE_CONFIRM_KEY_WAIT ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_CONFIRM_KEY_WAIT( RRL_WORK* work )
{
  int trg;
  int touchTrg;

  trg = GFL_UI_KEY_GetTrg();
  touchTrg = GFL_UI_TP_HitTrg( work->menuTouchHitTable );

  //--------
  // ↑キー
  if( trg & PAD_KEY_UP ) {
    MoveMenuCursorUp( work ); // カーソル移動
  }
  //--------
  // ↓キー
  if( trg & PAD_KEY_DOWN ) {
    MoveMenuCursorDown( work ); // カーソル移動
  } 

  //----------
  // A ボタン
  if( trg & PAD_BUTTON_A ) {
    StopPaletteAnime( work, PALETTE_ANIME_MENU_CURSOR_ON ); // カーソルが乗っている際のパレットアニメを停止
    StartPaletteAnime( work, PALETTE_ANIME_MENU_SELECT );   // 項目選択時のパレットアニメを開始
    // カーソル位置に応じた処理
    switch( work->menuCursorPos ) {
    case MENU_ITEM_DETERMINATION_OK:
      SetFinishReason( work, SEQ_CHANGE_BY_BUTTON );          // ボタンで終了
      FinishCurrentState( work );                             // RRL_STATE_CONFIRM_KEY_WAIT 状態終了
      RegisterNextState( work, RRL_STATE_CONFIRM_DETERMINE ); // => RRL_STATE_CONFIRM_DETERMINE 
      break;
    case MENU_ITEM_DETERMINATION_CANCEL:
      PMSND_PlaySE( SEQ_SE_CANCEL1 );                       // キャンセル音
      FinishCurrentState( work );                           // RRL_STATE_CONFIRM_KEY_WAIT 状態終了
      RegisterNextState( work, RRL_STATE_CONFIRM_CANCEL );  // => RRL_STATE_CONFIRM_CANCEL 
      RegisterNextState( work, RRL_STATE_KEY_WAIT );        // ==> RRL_STATE_KEY_WAIT 
      break;
    default:
      GF_ASSERT(0);
    }
    return;
  } 

  //----------------------------
  //「けってい」ボタンをタッチ
  if( touchTrg == MENU_TOUCH_AREA_OK_BUTTON ) {
    MoveMenuCursorDirect( work, MENU_ITEM_DETERMINATION_OK ); // カーソル移動
    StopPaletteAnime( work, PALETTE_ANIME_MENU_CURSOR_ON );   // カーソルが乗っている際のパレットアニメを停止
    StartPaletteAnime( work, PALETTE_ANIME_MENU_SELECT );     // 項目選択時のパレットアニメを開始
    SetFinishReason( work, SEQ_CHANGE_BY_TOUCH );             // タッチで終了
    FinishCurrentState( work );                               // RRL_STATE_CONFIRM_KEY_WAIT 状態終了
    RegisterNextState( work, RRL_STATE_CONFIRM_DETERMINE );   // => RRL_STATE_CONFIRM_DETERMINE 
    return;
  } 

  //-----------------------------------------
  // 強制終了 or B ボタン or 「やめる」ボタン
  if( CheckForceReturnFlag( work ) || 
      (trg & PAD_BUTTON_B) || (touchTrg == MENU_TOUCH_AREA_CANCEL_BUTTON) ) {
    MoveMenuCursorDirect( work, MENU_ITEM_DETERMINATION_CANCEL ); // カーソル移動
    StopPaletteAnime( work, PALETTE_ANIME_MENU_CURSOR_ON );       // カーソルが乗っている際のパレットアニメを停止
    StartPaletteAnime( work, PALETTE_ANIME_MENU_SELECT );         // 項目選択時のパレットアニメを開始
    PMSND_PlaySE( SEQ_SE_CANCEL1 );                               // キャンセル音
    FinishCurrentState( work );                                   // RRL_STATE_CONFIRM_KEY_WAIT 状態終了
    RegisterNextState( work, RRL_STATE_CONFIRM_CANCEL );          // => RRL_STATE_CONFIRM_CANCEL 
    RegisterNextState( work, RRL_STATE_KEY_WAIT );                // ==> RRL_STATE_KEY_WAIT 
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定の確認状態 ( RRL_STATE_TO_CONFIRM_STANDBY ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_TO_CONFIRM_STANDBY( RRL_WORK* work )
{
  switch( GetStateSeq( work ) ) {
  case 0:
    SetMenuItemCursorOff( work, MENU_ITEM_DETERMINATION_CANCEL ); // カーソルが乗っていない状態にする
    SetMenuItemCursorOff( work, MENU_ITEM_DETERMINATION_OK );     // カーソルが乗っていない状態にする

    // 確認メッセージと選択項目を表示
    BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CONFIRM, TRUE );
    BmpOamSetDrawEnable( work, BMPOAM_ACTOR_OK, TRUE );
    BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CANCEL, TRUE );

    if( work->palFadeOutFlag == FALSE ) {
      BG_FONT_SetPalette( work->BGFont_topic[ work->topicCursorPos ], 0xe ); // フォントのパレットを変更 ( パレットフェードが無効になるように )
      StartPaletteFadeOut( work ); // パレットフェードアウト開始
    }

    // スクロール開始
    StartAutoScroll_to_Confirm( work );

    IncStateSeq( work );
    break;

  case 1:
    // スクロール処理
    UpdateScroll( work );             // スクロールを更新
    UpdateScrollValue( work );        // スクロール実効値を更新
    UpdateTopicTouchArea( work );     // タッチ範囲を更新
    UpdateSliderDisp( work );         // スライダーを更新
    UpdateInvestigatingIcon( work );  // 調査項目選択アイコンを更新
    UpdateTopicButtonMask( work );    // 調査項目のマスクウィンドを更新

    // スクロール終了
    if( CheckScrollEnd( work ) ) {
      FinishCurrentState( work );                           // RRL_STATE_TO_CONFIRM_STANDBY 状態終了
      RegisterNextState( work, RRL_STATE_CONFIRM_STANDBY ); // => RRL_STATE_CONFIRM_STANDBY
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定の確認状態 ( RRL_STATE_TO_CONFIRM_KEY_WAIT ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_TO_CONFIRM_KEY_WAIT( RRL_WORK* work )
{
  switch( GetStateSeq( work ) ) {
  case 0:
    SetMenuCursorPos( work, MENU_ITEM_DETERMINATION_OK );         // カーソル位置を初期化
    SetMenuItemCursorOff( work, MENU_ITEM_DETERMINATION_CANCEL ); // カーソルが乗っていない状態にする
    SetMenuItemCursorOn( work, MENU_ITEM_DETERMINATION_OK );      // カーソルが乗っている状態にする
    StartPaletteAnime( work, PALETTE_ANIME_MENU_CURSOR_ON );      // パレットアニメーション開始

    // 確認メッセージと選択項目を表示
    BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CONFIRM, TRUE );
    BmpOamSetDrawEnable( work, BMPOAM_ACTOR_OK, TRUE );
    BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CANCEL, TRUE );

    if( work->palFadeOutFlag == FALSE ) {
      BG_FONT_SetPalette( work->BGFont_topic[ work->topicCursorPos ], 0xe ); // フォントのパレットを変更 ( パレットフェードが無効になるように )
      StartPaletteFadeOut( work ); // パレットフェードアウト開始
    }

    // スクロール開始
    StartAutoScroll_to_Confirm( work );

    IncStateSeq( work );
    break;

  case 1:
    // スクロール処理
    UpdateScroll( work );             // スクロールを更新
    UpdateScrollValue( work );        // スクロール実効値を更新
    UpdateTopicTouchArea( work );     // タッチ範囲を更新
    UpdateSliderDisp( work );         // スライダーを更新
    UpdateInvestigatingIcon( work );  // 調査項目選択アイコンを更新
    UpdateTopicButtonMask( work );    // 調査項目のマスクウィンドを更新

    // スクロール終了
    if( CheckScrollEnd( work ) ) {
      FinishCurrentState( work );                            // RRL_STATE_TO_CONFIRM_KEY_WAIT 状態終了
      RegisterNextState( work, RRL_STATE_CONFIRM_KEY_WAIT ); // => RRL_STATE_CONFIRM_KEY_WAIT
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目確定状態 ( RRL_STATE_CONFIRM_DETERMINE ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_CONFIRM_DETERMINE( RRL_WORK* work )
{
  switch( GetStateSeq( work ) ) {
  case 0:
    SetTopicButtonNotInvestigating( work, GetInvestigatingTopicID(work) ); // 調査中だった項目のボタンを, 調査していない状態に戻す
    UpdateInvestigatingTopicID( work );                                    // 調査する項目を更新
    SetTopicButtonInvestigating( work, GetSelectedTopicID(work) );         // 新たに調査する項目のボタンを, 調査中の状態にする
    UpdateInvestigatingIcon( work );                                       // 調査中アイコンを更新
    BmpOamSetDrawEnable( work, BMPOAM_ACTOR_DETERMINE, TRUE );             //「ちょうさを　かいしします！」を表示
    PMSND_PlaySE( SEQ_SE_SYS_80 );                                         // 調査開始 SE 
    IncStateSeq( work );
    break;

  case 1:
    // 一定時間が経過
    if( SEQ_DETERMINE_WAIT_FRAMES <= work->stateCount ) {
      StartFadeOut(); // フェードアウト開始
      IncStateSeq( work );
    }
    break;

  case 2:
    // フェードが終了
    if( CheckFadeEnd() ) {
      StartPaletteFadeIn( work ); // パレットのフェードインを開始する
      IncStateSeq( work );
    } 
    break;

  case 3:
    // パレットフェード完了
    if( IsPaletteFadeEnd( work ) ) {
      FinishCurrentState( work );                    // RRL_STATE_CONFIRM_DETERMINE 状態終了
      RegisterNextState( work, RRL_STATE_CLEAN_UP ); // ===> RRL_STATE_CLEAN_UP 
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief フェードアウト 状態 ( RRL_STATE_RETURN_FROM_KEYWAIT ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_RETURN_FROM_KEYWAIT( RRL_WORK* work )
{
  switch( GetStateSeq( work ) ) {
  case 0:
    // 待ち時間が経過
    if( GetWaitFrame(work) < work->stateCount ) {
      StartFadeOut(); // フェードアウト開始
      IncStateSeq( work );
    }
    break;

  case 1:
    // フェードが終了
    if( CheckFadeEnd() ) {
      StartPaletteFadeIn( work );    // パレットのフェードインを開始する
      IncStateSeq( work );
    } 
    break;

  case 2:
    // パレットフェード完了
    if( IsPaletteFadeEnd( work ) ) {
      BG_FONT_SetPalette( work->BGFont_topic[ work->topicCursorPos ], 0xf); // フォントのパレットを元に戻す
      FinishCurrentState( work );                                           // RRL_STATE_RETURN_FROM_KEYWAIT 状態終了
      RegisterNextState( work, RRL_STATE_CLEAN_UP );                        // => RRL_STATE_CLEAN_UP 
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief フレーム経過待ち 状態 ( RRL_STATE_RETURN_FROM_STANDBY ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_RETURN_FROM_STANDBY( RRL_WORK* work )
{
  switch( GetStateSeq( work ) ) {
  case 0:
    // 待ち時間が経過
    if( GetWaitFrame(work) < work->stateCount ) {
      StartFadeOut(); // フェードアウト開始
      IncStateSeq( work );
    }
    break;

  case 1:
    // フェードが終了
    if( CheckFadeEnd() ) {
      StartPaletteFadeIn( work );    // パレットのフェードインを開始する
      IncStateSeq( work );
    } 
    break;

  case 2:
    // パレットフェード完了
    if( IsPaletteFadeEnd( work ) ) {
      BG_FONT_SetPalette( work->BGFont_topic[ work->topicCursorPos ], 0xf); // フォントのパレットを元に戻す
      FinishCurrentState( work );                                           // RRL_STATE_RETURN_FROM_KEYWAIT 状態終了
      RegisterNextState( work, RRL_STATE_CLEAN_UP );                        // => RRL_STATE_CLEAN_UP 
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief パレット復帰状態 ( RRL_STATE_CONFIRM_CANCEL ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_CONFIRM_CANCEL( RRL_WORK* work )
{
  switch( GetStateSeq( work ) ) {
  case 0:
    StartAutoScroll_for_Reset( work ); // スクロール開始
    IncStateSeq( work );
    break;

  case 1:
    // スクロール処理
    UpdateScroll( work );            // スクロールを更新
    UpdateScrollValue( work );       // スクロール実効値を更新
    UpdateTopicTouchArea( work );    // タッチ範囲を更新する
    UpdateSliderDisp( work );        // スライダー
    UpdateInvestigatingIcon( work ); // 調査項目選択アイコン
    UpdateTopicButtonMask( work );   // 調査項目のマスクウィンドを更新する

    // スクロールが終了
    if( CheckScrollEnd(work) ) {
      IncStateSeq( work );
    }
    break;

  case 2:
    // パレットアニメが終了
    if( CheckPaletteAnimeEnd(work, PALETTE_ANIME_MENU_SELECT ) ) {
      // 確認メッセージと選択項目を消去
      BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CONFIRM, FALSE );
      BmpOamSetDrawEnable( work, BMPOAM_ACTOR_OK, FALSE );
      BmpOamSetDrawEnable( work, BMPOAM_ACTOR_CANCEL, FALSE );
      UpdateTopicButtonMask( work ); // ウィンドウを切る
      StartPaletteFadeIn( work );    // パレットのフェードインを開始する
      IncStateSeq( work );
    }
    break;

  case 3:
    // パレットフェード完了
    if( IsPaletteFadeEnd( work ) ) {
      BG_FONT_SetPalette( work->BGFont_topic[ work->topicCursorPos ], 0xf); // フォントのパレットを元に戻す
      FinishCurrentState( work ); // 次の状態へ
    }
    break;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 後片付け状態 ( RRL_STATE_CLEAN_UP ) の処理
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MainState_CLEAN_UP( RRL_WORK* work )
{ 
  SaveRecoveryData( work );

  // VBlankタスクを解除
  ReleaseVBlankTask( work );

  // パレットアニメーション 
  CleanUpPaletteAnime( work );
  DeletePaletteAnime( work );

  // 共通パレットアニメーション
  RRC_StopAllPaletteAnime( work->commonWork ); // 停止して, 
  RRC_ResetAllPalette( work->commonWork );     // パレットを元に戻す

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
  SetFinishResult( work, RRL_RESULT_TO_TOP );   // リスト画面の終了結果を設定
  FinishCurrentState( work );                   // RRL_STATE_CLEAN_UP 状態終了
  RegisterNextState( work, RRL_STATE_FINISH );  // => RRL_STATE_FINISH 
  work->finishFlag = TRUE;
  GX_SetVisibleWnd( GX_WNDMASK_NONE ); //ウィンドウを無効に
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態カウンタをインクリメントする
 * 
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void IncStateCount( RRL_WORK* work )
{
  work->stateCount++;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 次の状態をキューに登録する
 *
 * @param work
 * @param nextSeq 登録する状態
 */
//-----------------------------------------------------------------------------------------
static void RegisterNextState( RRL_WORK* work, RRL_STATE nextSeq )
{
  QUEUE_EnQueue( work->stateQueue, nextSeq );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在の状態を終了する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void FinishCurrentState( RRL_WORK* work )
{
  // すでに終了済み
  GF_ASSERT( work->stateEndFlag == FALSE );

  // 終了フラグを立てる
  work->stateEndFlag = TRUE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 画面遷移の方法を取得する
 *
 * @param work
 *
 * @return 画面遷移の方法
 */
//-----------------------------------------------------------------------------------------
static SEQ_CHANGE_TRIG GetFinishReason( RRL_WORK* work )
{
  return RRC_GetSeqChangeTrig( work->commonWork );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief リスト画面終了の方法を登録する
 *
 * @param work
 * @param reason 終了理由
 */
//-----------------------------------------------------------------------------------------
static void SetFinishReason( RRL_WORK* work, SEQ_CHANGE_TRIG reason )
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
static void SetFinishResult( RRL_WORK* work, RRL_RESULT result )
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
static void SetWaitFrame( RRL_WORK* work, u32 frame )
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
static u32 GetWaitFrame( const RRL_WORK* work )
{
  return work->waitFrame;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態を変更する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SwitchState( RRL_WORK* work )
{
  RRL_STATE nextSeq;

  if( work->stateEndFlag == FALSE ){ return; }  // 現在の状態が終了していない
  if( QUEUE_IsEmpty( work->stateQueue ) ){ return; } // 状態キューに登録されていない

  // 変更
  nextSeq = QUEUE_DeQueue( work->stateQueue );
  SetState( work, nextSeq ); 
} 

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態を取得する
 *
 * @param work
 *
 * @return 現在の状態
 */
//-----------------------------------------------------------------------------------------
static RRL_STATE GetState( const RRL_WORK* work )
{
  return work->state;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態を設定する
 *
 * @param work
 * @parma nextSeq 設定する状態
 */
//-----------------------------------------------------------------------------------------
static void SetState( RRL_WORK* work, RRL_STATE nextSeq )
{ 
  // 更新
  work->state        = nextSeq;
  work->stateCount   = 0;
  work->stateEndFlag = FALSE;
  ResetStateSeq( work );
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
static u32 GetStateSeq( const RRL_WORK* work )
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
static void IncStateSeq( RRL_WORK* work )
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
static void ResetStateSeq( RRL_WORK* work )
{
  work->stateSeq = 0;
}



//=========================================================================================
// ◆LAYER 4 機能
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを上へ移動する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MoveMenuCursorUp( RRL_WORK* work )
{ 
  SetMenuItemCursorOff( work, work->menuCursorPos );        // カーソルが乗っていない状態にする
  ShiftMenuCursorPos( work, -1 );                           // カーソル移動
  SetMenuItemCursorOn( work, work->menuCursorPos );         // カーソルが乗っている状態にする
  PMSND_PlaySE( SEQ_SE_SELECT1 );                           // カーソル移動音
  StartPaletteAnime( work, PALETTE_ANIME_MENU_CURSOR_SET ); // パレットアニメ開始
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを下へ移動する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void MoveMenuCursorDown( RRL_WORK* work )
{
  SetMenuItemCursorOff( work, work->menuCursorPos );        // カーソルが乗っていない状態にする
  ShiftMenuCursorPos( work, 1 );                            // カーソル移動
  SetMenuItemCursorOn( work, work->menuCursorPos );         // カーソルが乗っている状態にする
  PMSND_PlaySE( SEQ_SE_SELECT1 );                           // カーソル移動音
  StartPaletteAnime( work, PALETTE_ANIME_MENU_CURSOR_SET ); // パレットアニメ開始
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを直接移動する
 *
 * @param work
 * @param menuItem 移動先のメニュー項目
 */
//-----------------------------------------------------------------------------------------
static void MoveMenuCursorDirect( RRL_WORK* work, MENU_ITEM menuItem )
{
  SetMenuItemCursorOff( work, work->menuCursorPos );        // カーソルが乗っていない状態にする
  SetMenuCursorPos( work, menuItem );                       // カーソル移動
  SetMenuItemCursorOn( work, work->menuCursorPos );         // カーソルが乗っている状態にする
  PMSND_PlaySE( SEQ_SE_SELECT1 );                           // カーソル移動音
  StartPaletteAnime( work, PALETTE_ANIME_MENU_CURSOR_SET ); // パレットアニメ開始
}

//-----------------------------------------------------------------------------------------
/**
 * @brief カーソルが画面内に存在するように調整する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void AdjustTopicCursor_to_ScrollCursor( RRL_WORK* work )
{
  int min = GetMinScrollCursorMarginPos();
  int max = GetMaxScrollCursorMarginPos();
  int curTop = CalcScrollCursorPosOfTopicButtonTop( work->topicCursorPos );
  int curBottom = CalcScrollCursorPosOfTopicButtonBottom( work->topicCursorPos );

  if( curBottom <= min ) {
    u8 newTopicID = CalcTopicID_byScrollCursorPos( curBottom + TOPIC_BUTTON_HEIGHT/2 );

    if( work->topicCursorActiveFlag ) {
      MoveTopicCursorDirect( work, newTopicID );
    }
    else {
      SetTopicCursorPosDirect( work, newTopicID ); // カーソル位置の更新のみ
    }
  }

  if( max <= curTop ) {
    u8 newTopicID = CalcTopicID_byScrollCursorPos( curTop - TOPIC_BUTTON_HEIGHT/2 );

    if( work->topicCursorActiveFlag ) {
      MoveTopicCursorDirect( work, newTopicID );
    }
    else {
      SetTopicCursorPosDirect( work, newTopicID ); // カーソル位置の更新のみ
    }
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目カーソルを直接移動する
 *
 * @param work
 * @param topicID 移動先の調査項目ID
 */
//-----------------------------------------------------------------------------------------
static void MoveTopicCursorDirect( RRL_WORK* work, u8 topicID )
{
  SetTopicButtonCursorOff( work );                           // 移動前の項目を元に戻す
  SetTopicCursorPosDirect( work, topicID );                  // カーソル位置を更新
  SetTopicButtonCursorOn( work );                            // 移動後の項目を選択状態にする
  PMSND_PlaySE( SEQ_SE_SELECT1 );                            // カーソル移動音
  StartPaletteAnime( work, PALETTE_ANIME_TOPIC_CURSOR_SET ); // パレットアニメ開始
}

//-----------------------------------------------------------------------------------------
/**
 * @brief『戻る』ボタンを明滅させる
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void BlinkReturnButton( RRL_WORK* work )
{
  StartCommonPaletteAnime( work, COMMON_PALETTE_ANIME_RETURN );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief オートスクロールを開始する ( 調査項目カーソル位置へ )
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void StartAutoScroll_to_Topic( RRL_WORK* work )
{
  int startPos = work->scrollCursorPos;
  int endPos;
  int frames;

  // 下へ進む場合
  if( work->topicCursorPos < work->topicCursorNextPos ) {
    // スクロールカーソルの終点はボタンの底辺位置 + 1
    endPos = CalcScrollCursorPosOfTopicButtonBottom( work->topicCursorNextPos ) + 1;
  }
  // 上へ進む場合
  else {
    // スクロールカーソルの終点はボタンの上辺位置
    endPos = CalcScrollCursorPosOfTopicButtonTop( work->topicCursorNextPos );
  }

  // スクロール実効値に変更があるスクロールの場合,
  // スクロールカーソル始点座標を端にセットする
  {
    int min = GetMinScrollCursorMarginPos();
    int max = GetMaxScrollCursorMarginPos();
    if( (endPos < min) || (max < endPos) ) {
      if( startPos < endPos ) {
        startPos = max;
      }
      else {
        startPos = min;
      }
    }
  }

  frames = ( MATH_MAX(startPos, endPos) - MATH_MIN(startPos, endPos) ) / 4;

  // スクロール実効値に変化があるかどうか
  {
    int min = GetMinScrollCursorMarginPos();
    int max = GetMaxScrollCursorMarginPos();
    // スクロール実効値に変更がない
    if( ( min <= startPos) && (startPos <= max) && 
        ( min <= endPos) && (endPos <= max)  ) {
      frames = 0;
    }
  } 
  StartScroll( work, startPos, endPos, frames );  
}

//-----------------------------------------------------------------------------------------
/**
 * @brief オートスクロールを開始する ( 範囲外スクロールをリセット )
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void StartAutoScroll_for_Reset( RRL_WORK* work )
{
  int startPos = work->scrollCursorPos;
  int endPos = startPos;
  int frames;

  // スクロールカーソル終点位置を決定
  if( GetMaxScrollCursorPos(work) < startPos ) {
    endPos = GetMaxScrollValue(work) + SCROLL_CURSOR_TOP_MARGIN;
  }
  else if( startPos < MIN_SCROLL_CURSOR_POS ) {
    endPos = MIN_SCROLL_VALUE + DISP_DOT_HEIGHT - SCROLL_CURSOR_BOTTOM_MARGIN;
  }
  // スクロール実効値に変更があるスクロールの場合,
  // スクロールカーソル始点座標を端にセットする
  {
    int min = GetMinScrollCursorMarginPos();
    int max = GetMaxScrollCursorMarginPos();
    if( (endPos < min) || (max < endPos) )
    {
      if( startPos < endPos ) {
        startPos = max;
      }
      else {
        startPos = min;
      }
    }
  }
  frames = ( MATH_MAX(startPos, endPos) - MATH_MIN(startPos, endPos) ) / 3;
  // スクロール実効値に変更がないスクロールの場合,
  // 1フレームで処理する
  {
    int min = GetMinScrollCursorMarginPos();
    int max = GetMaxScrollCursorMarginPos();
    if( ( min <= startPos) && (startPos <= max) && 
        ( min <= endPos) && (endPos <= max)  ) 
    {
      frames = 1;
    }
  } 
  StartScroll( work, startPos, endPos, frames  );  
}

//-----------------------------------------------------------------------------------------
/**
 * @brief オートスクロールを開始する ( 調査項目確定の確認へ )
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void StartAutoScroll_to_Confirm( RRL_WORK* work )
{
  int centerPos = ( GetMinScrollCursorMarginPos() + GetMaxScrollCursorMarginPos() ) / 2;
  int startPos  = work->scrollCursorPos;
  int endPos    = CalcScreenTopOfTopicButton( work, work->topicCursorPos ) * DOT_PER_CHARA;
  int frames;
  // カーソル位置の調査項目が画面の任意の位置まで来るようにスクロールさせる
  if( centerPos < endPos ) { 
    // 進む方向でカーソル位置の始点・終点を変える
    startPos = GetMaxScrollCursorMarginPos();
    endPos   = GetMaxScrollCursorMarginPos() + (endPos - centerPos);
  }
  else {
    startPos = GetMinScrollCursorMarginPos();
    endPos   = GetMinScrollCursorMarginPos() - (centerPos - endPos);
  }
  frames = ( MATH_MAX(startPos, endPos) - MATH_MIN(startPos, endPos) ) / 3;
  StartScroll( work, startPos, endPos, frames  );  
}


//=========================================================================================
// ◆LAYER 3 個別操作
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief 現在調査中の調査項目IDを取得する
 *
 * @param work
 *
 * @return 現在調査中の調査項目ID
 */
//-----------------------------------------------------------------------------------------
static u8 GetInvestigatingTopicID( const RRL_WORK* work )
{
  int qIdx;
  GAMEDATA* gameData;
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* QSave;
  u8 questionID[ QUESTION_NUM_PER_TOPIC ];
  u8 topicID;

  gameData = GetGameData( work );
  save     = GAMEDATA_GetSaveControlWork( gameData );
  QSave    = SaveData_GetQuestionnaire( save );

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
 * @brief 調査する項目を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateInvestigatingTopicID( const RRL_WORK* work )
{
  GAMEDATA* gameData;
  SAVE_CONTROL_WORK* save;
  QUESTIONNAIRE_SAVE_WORK* QSave;
  u8 topicID;

  // 項目を選択していない
  GF_ASSERT( IsTopicIDSelected(work) );

  topicID  = GetSelectedTopicID( work ); // 選択した調査項目ID
  gameData = GetGameData( work );
  save     = GAMEDATA_GetSaveControlWork( gameData );
  QSave    = SaveData_GetQuestionnaire( save );
  
  // セーブデータを更新
  QuestionnaireWork_SetInvestigatingQuestion( QSave, Question1_topic[ topicID ], 0 );
  QuestionnaireWork_SetInvestigatingQuestion( QSave, Question2_topic[ topicID ], 1 );
  QuestionnaireWork_SetInvestigatingQuestion( QSave, Question3_topic[ topicID ], 2 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソルを移動する
 *
 * @parma work
 * @param stride 移動量
 */
//-----------------------------------------------------------------------------------------
static void ShiftMenuCursorPos( RRL_WORK* work, int stride )
{
  int nowPos;
  int nextPos;

  // カーソル位置を更新
  nowPos  = work->menuCursorPos;
  nextPos = (nowPos + stride + MENU_ITEM_NUM) % MENU_ITEM_NUM;
  work->menuCursorPos = nextPos; 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メニュー項目カーソル位置を変更する ( 直値指定 )
 *
 * @param work
 * @param menuItem 設定するメニュー項目位置
 */
//-----------------------------------------------------------------------------------------
static void SetMenuCursorPos( RRL_WORK* work, MENU_ITEM menuItem )
{
  // カーソル位置を更新
  work->menuCursorPos = menuItem;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 指定したメニュー項目を, カーソルが乗っている状態にする
 *
 * @param work
 * @param menuItem メニュー項目
 */
//-----------------------------------------------------------------------------------------
static void SetMenuItemCursorOn( RRL_WORK* work, MENU_ITEM menuItem )
{
  BMPOAM_ACT_PTR BmpOamActor;

  // カーソル位置のメニュー項目に対応するBMP-OAM アクターを取得
  BmpOamActor = GetBmpOamActorOfMenuItem( work, menuItem );

  // BMP-OAM アクターのパレットオフセットを変更
  BmpOam_ActorSetPaletteOffset( BmpOamActor, 0 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 指定したメニュー項目を, メニュー項目をカーソルが乗っている状態にする
 *
 * @param work
 * @param menuItem メニュー項目
 */
//-----------------------------------------------------------------------------------------
static void SetMenuItemCursorOff( RRL_WORK* work, MENU_ITEM menuItem )
{
  BMPOAM_ACT_PTR BmpOamActor;

  // カーソル位置のメニュー項目に対応するBMP-OAM アクターを取得
  BmpOamActor = GetBmpOamActorOfMenuItem( work, menuItem );

  // BMP-OAM アクターのパレットオフセットを変更
  BmpOam_ActorSetPaletteOffset( BmpOamActor, 1 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目カーソルの位置を設定する
 *
 * @param work
 * @param topicID 設定する調査項目ID
 */
//-----------------------------------------------------------------------------------------
static void SetTopicCursorPosDirect( RRL_WORK* work, int topicID )
{
  // カーソル位置を更新
  work->topicCursorPos = topicID;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目カーソルの移動先を設定する
 *
 * @param work
 * @param stride 移動量
 */
//-----------------------------------------------------------------------------------------
static void SetTopicCursorNextPos( RRL_WORK* work, int stride )
{
  int nowPos, nextPos;

  // 移動後のカーソル位置を算出
  nowPos  = work->topicCursorPos;
  nextPos = (nowPos + stride + GetSelectableTopicNum(work)) % GetSelectableTopicNum(work);

  // カーソルの移動先を設定
  work->topicCursorNextPos = nextPos;
}

//-----------------------------------------------------------------------------------------
/**
 * @biref カーソル位置にある調査項目を, カーソルが乗っている状態にする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetTopicButtonCursorOn( RRL_WORK* work )
{
  u8 topicID;
  u8 BGFrame;
  u8 left, top, width, height;
  u8 paletteNo;

  // スクリーン更新パラメータを決定
  topicID   = work->topicCursorPos;
  left      = CalcScreenLeftOfTopicButton( work, topicID );
  top       = CalcScreenTopOfTopicButton( work, topicID );
  width     = TOPIC_BUTTON_WIDTH;
  height    = TOPIC_BUTTON_HEIGHT; 
  BGFrame   = MAIN_BG_WINDOW;
  paletteNo = MAIN_BG_PALETTE_WINDOW_ON;

  // スクリーン更新
  GFL_BG_ChangeScreenPalette( BGFrame, left, top, width, height, paletteNo );
  GFL_BG_LoadScreenV_Req( BGFrame );

  // フラグセット
  work->topicCursorActiveFlag = TRUE;
}

//-----------------------------------------------------------------------------------------
/**
 * @biref カーソル位置にある調査項目を, カーソルが乗っていない状態にする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetTopicButtonCursorOff( RRL_WORK* work )
{
  u8 topicID;
  u8 BGFrame;
  u8 left, top, width, height;
  u8 paletteNo;

  // スクリーン更新パラメータを決定
  topicID   = work->topicCursorPos;
  left      = CalcScreenLeftOfTopicButton( work, topicID );
  top       = CalcScreenTopOfTopicButton( work, topicID );
  width     = TOPIC_BUTTON_WIDTH;
  height    = TOPIC_BUTTON_HEIGHT; 
  BGFrame   = MAIN_BG_WINDOW;
  paletteNo = MAIN_BG_PALETTE_WINDOW_OFF;

  // スクリーン更新
  GFL_BG_ChangeScreenPalette( BGFrame, left, top, width, height, paletteNo );
  GFL_BG_LoadScreenV_Req( BGFrame );

  // フラグリセット
  work->topicCursorActiveFlag = FALSE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 指定した調査項目のボタンを, 調査中の状態に設定する
 *
 * @param work
 * @param topicID 調査項目ID
 */
//-----------------------------------------------------------------------------------------
static void SetTopicButtonInvestigating( const RRL_WORK* work, u8 topicID )
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
      top     = CalcScreenTopOfTopicButton( work, topicID );
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
    top     = CalcScreenTopOfTopicButton( work, topicID );
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
  GFL_BG_LoadScreenV_Req( MAIN_BG_WINDOW );
  GFL_BG_LoadScreenV_Req( MAIN_BG_FONT );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 指定した調査項目のボタンを, 調査中でない状態に戻す
 *
 * @param work
 * @param topicID 調査項目ID
 */
//-----------------------------------------------------------------------------------------
static void SetTopicButtonNotInvestigating( const RRL_WORK* work, u8 topicID )
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
      top     = CalcScreenTopOfTopicButton( work, topicID );
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
    top     = CalcScreenTopOfTopicButton( work, topicID );
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
  GFL_BG_LoadScreenV_Req( MAIN_BG_WINDOW );
  GFL_BG_LoadScreenV_Req( MAIN_BG_FONT );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目のスクリーン x 座標を取得する
 *
 * @param work
 * @param topicID 調査項目ID
 *
 * @return 指定した調査項目IDが該当するスクリーン範囲の左上x座標 (スクリーン単位)
 */
//-----------------------------------------------------------------------------------------
static u8 CalcScreenLeftOfTopicButton( const RRL_WORK* work, u8 topicID )
{
  u8 left;

  // デフォルトの位置
  left = TOPIC_BUTTON_X;

  // 調査中の場合, 1キャラ分左にある
  if( GetInvestigatingTopicID(work) == topicID ){ left -= 1; }

  return left;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目のスクリーン y 座標を取得する
 *
 * @param work
 * @param topicID 調査項目ID
 *
 * @return 指定した調査項目IDが該当するスクリーン範囲の左上y座標 (スクリーン単位)
 */
//-----------------------------------------------------------------------------------------
static u8 CalcScreenTopOfTopicButton( const RRL_WORK* work, u8 topicID )
{
  u8 top;

  // デフォルトの位置
  top = (TOPIC_BUTTON_Y + TOPIC_BUTTON_HEIGHT) * topicID;

  return top;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目の左上x座標を取得する (ドット単位)
 *
 * @param work
 * @param topicID 調査項目ID
 *
 * @return 指定した調査項目の左上x座標 (ドット単位)
 */
//-----------------------------------------------------------------------------------------
static int CalcDisplayLeftOfTopicButton( const RRL_WORK* work, u8 topicID )
{
  int left;

  // デフォルトの位置を算出
  left = CalcScreenLeftOfTopicButton( work, topicID ) * DOT_PER_CHARA;

  // BGスクロールの分を考慮する
  left -= GFL_BG_GetScreenScrollX( MAIN_BG_WINDOW );

  return left;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目の左上y座標を取得する (ドット単位)
 *
 * @param work
 * @param topicID 調査項目ID
 *
 * @return 指定した調査項目の左上y座標 (ドット単位)
 */
//-----------------------------------------------------------------------------------------
static int CalcDisplayTopOfTopicButton( const RRL_WORK* work, u8 topicID )
{
  int top;

  // デフォルトの位置を算出
  top = CalcScreenTopOfTopicButton( work, topicID ) * DOT_PER_CHARA;

  // BGスクロールの分を考慮する
  top -= GetScrollValue();

  return top;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目の右下y座標を取得する (ドット単位)
 *
 * @param work
 * @param topicID 調査項目ID
 *
 * @return 指定した調査項目の左上y座標 (ドット単位)
 */
//-----------------------------------------------------------------------------------------
static int CalcDisplayBottomOfTopicButton( const RRL_WORK* work, u8 topicID )
{
  int bottom;

  // デフォルトの位置を算出
  bottom = CalcScreenTopOfTopicButton( work, topicID ) * DOT_PER_CHARA + TOPIC_BUTTON_HEIGHT * DOT_PER_CHARA - 1;

  // BGスクロールの分を考慮する
  bottom -= GetScrollValue();

  return bottom;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目ボタンのスクロール回り込みを隠すためのウィンドウを更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateTopicButtonMask( const RRL_WORK* work )
{
  int top, bottom;
  BOOL wndEnable = FALSE;
  int wndLeft, wndRight, wndTop, wndBottom;

  // 調査項目ボタンの上下両端のディスプレイ座標を取得
  top = CalcDisplayTopOfTopicButton( work, 0 );
  bottom = CalcDisplayBottomOfTopicButton( work, TOPIC_ID_MAX );

  // ウィンドウのパラメータを決定
  if( 0 < top ) { // 画面上部に回り込みが起きている
    wndEnable = TRUE;
    wndLeft   = 0;
    wndRight  = 240;
    wndTop    = 0;
    wndBottom = top - 1;
  }
  else if( bottom < DISP_DOT_HEIGHT ) { // 画面下部に回りこみが起きている
    wndEnable = TRUE;
    wndLeft   = 0;
    wndRight  = 240;
    wndTop    = bottom + 1;
    wndBottom = DISP_DOT_HEIGHT;
  }
  else {
  }

  // ウィンドウを設定
  if( wndEnable ) {
    GX_SetVisibleWnd( GX_WNDMASK_W0 ); // ウィンドウを有効に
    G2_SetWnd0Position( wndLeft, wndTop, wndRight, wndBottom ); // ウィンドウの座標を設定
    G2_SetWndOutsidePlane( 
        GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | 
        GX_WND_PLANEMASK_BG2 | GX_WND_PLANEMASK_BG3 | GX_WND_PLANEMASK_OBJ,
        TRUE ); // ウィンドウの外側を設定
    G2_SetWnd0InsidePlane( 
        GX_WND_PLANEMASK_BG0 | GX_WND_PLANEMASK_BG1 | GX_WND_PLANEMASK_OBJ,
        TRUE ); // ウィンドウの内側を設定
  }
  else {
    GX_SetVisibleWnd( GX_WNDMASK_NONE ); //ウィンドウを無効に
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目選択アイコンの表示状態を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateInvestigatingIcon( const RRL_WORK* work )
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
    pos.x      = CalcDisplayLeftOfTopicButton( work, topicID ) + SELECT_ICON_DRAW_OFFSET_X;
    pos.y      = CalcDisplayTopOfTopicButton( work, topicID ) + SELECT_ICON_DRAW_OFFSET_Y;
    setSurface = ClactWorkInitData[ CLWK_SELECT_ICON_0 ].setSurface;
    GFL_CLACT_WK_SetPos( clactWork, &pos, setSurface );
    GFL_CLACT_WK_SetAutoAnmFlag( clactWork, TRUE );
    GFL_CLACT_WK_SetDrawEnable( clactWork, TRUE );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 上画面の調査項目説明文を更新する ( 現在のカーソル位置に合わせる )
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateTopicDetailStrings_at_Now( RRL_WORK* work )
{
  UpdateTopicDetailStrings_at( work, work->topicCursorPos );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 上画面の調査項目説明文を更新する ( 次のカーソル位置に合わせる )
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateTopicDetailStrings_at_Next( RRL_WORK* work )
{
  UpdateTopicDetailStrings_at( work, work->topicCursorNextPos );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 上画面の調査項目説明文を更新する ( 指定カーソル位置に合わせる )
 *
 * @param work
 * @param topicID カーソル位置 ( 調査項目ID )
 */
//-----------------------------------------------------------------------------------------
static void UpdateTopicDetailStrings_at( RRL_WORK* work, u8 topicID )
{
  // 調査項目 題名/補足
  BG_FONT_SetMessage( work->BGFont_string[ BG_FONT_TOPIC_TITLE ],   StringID_topicTitle[ topicID ] );
  BG_FONT_SetMessage( work->BGFont_string[ BG_FONT_TOPIC_CAPTION ], StringID_topicCaption[ topicID ] );

  // 質問
  BG_FONT_SetMessage( work->BGFont_string[ BG_FONT_QUESTION_1 ], StringID_question[ Question1_topic[ topicID ] ] );
  BG_FONT_SetMessage( work->BGFont_string[ BG_FONT_QUESTION_2 ], StringID_question[ Question2_topic[ topicID ] ] );
  BG_FONT_SetMessage( work->BGFont_string[ BG_FONT_QUESTION_3 ], StringID_question[ Question3_topic[ topicID ] ] );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 上画面の調査項目説明文の表示を隠す
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void HideTopicDetailStrints( RRL_WORK* work )
{
  BG_FONT_SetDrawEnable( work->BGFont_string[ BG_FONT_TOPIC_TITLE ], FALSE );
  BG_FONT_SetDrawEnable( work->BGFont_string[ BG_FONT_TOPIC_CAPTION ], FALSE );
  BG_FONT_SetDrawEnable( work->BGFont_string[ BG_FONT_QUESTION_1 ], FALSE );
  BG_FONT_SetDrawEnable( work->BGFont_string[ BG_FONT_QUESTION_2 ], FALSE );
  BG_FONT_SetDrawEnable( work->BGFont_string[ BG_FONT_QUESTION_3 ], FALSE );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 上画面の調査項目説明文の表示を開始する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ShowTopicDetailStrings( RRL_WORK* work )
{
  BG_FONT_SetDrawEnable( work->BGFont_string[ BG_FONT_TOPIC_TITLE ], TRUE );
  BG_FONT_SetDrawEnable( work->BGFont_string[ BG_FONT_TOPIC_CAPTION ], TRUE );
  BG_FONT_SetDrawEnable( work->BGFont_string[ BG_FONT_QUESTION_1 ], TRUE );
  BG_FONT_SetDrawEnable( work->BGFont_string[ BG_FONT_QUESTION_2 ], TRUE );
  BG_FONT_SetDrawEnable( work->BGFont_string[ BG_FONT_QUESTION_3 ], TRUE );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スライダーの表示位置を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateSliderDisp( const RRL_WORK* work )
{
  GFL_CLWK* clactWork;

  // セルアクターワークを取得
  clactWork  = GetClactWork( work, CLWK_SCROLL_CONTROL ); 

  // 表示しない
  if( CheckScrollControlCan( work ) == FALSE ) {
    GFL_CLACT_WK_SetDrawEnable( clactWork, FALSE );
  }
  // 表示する
  else { 
    GFL_CLACTPOS pos;
    u16 setSurface;

    // 表示位置を算出
    pos.x = SCROLL_CONTROL_LEFT;
    pos.y = CalcSliderPos_byScrollValue( work );
    setSurface = ClactWorkInitData[ CLWK_SCROLL_CONTROL ].setSurface;

    // 表示位置を変更
    GFL_CLACT_WK_SetPos( clactWork, &pos, setSurface );
    GFL_CLACT_WK_SetDrawEnable( clactWork, TRUE );
  } 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スライダーの表示位置を計算する
 *
 * @param work
 *
 * @return 指定したスクロール実効値から求めた, スクロールバーつまみ部分のy座標
 */
//-----------------------------------------------------------------------------------------
static int CalcSliderPos_byScrollValue( const RRL_WORK* work )
{
  int controlRange;
  int valueRange;
  int value;
  int pos;
  float rate;

  rate = (float)(GetScrollValue() - MIN_SCROLL_VALUE) / (float)(GetMaxScrollValue(work) - MIN_SCROLL_VALUE);
  pos = SCROLL_CONTROL_TOP + (SCROLL_CONTROL_BOTTOM - SCROLL_CONTROL_TOP) * rate;

  if( pos < SCROLL_CONTROL_TOP ) { pos = SCROLL_CONTROL_TOP; } // 最小値補正
  if( SCROLL_CONTROL_BOTTOM < pos ) { pos = SCROLL_CONTROL_BOTTOM; } // 最大値補正

  return pos;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スライダーの表示位置から, スクロールカーソル位置を算出する
 *
 * @param work
 * @param controlPos つまみ部分の表示位置
 *
 * @return スクロールカーソルの位置
 */
//-----------------------------------------------------------------------------------------
static int CalcScrollCursorPos_bySliderPos( const RRL_WORK* work, int controlPos )
{
  float rate;
  int min, max;
  int cursorPos;

  if( controlPos < SCROLL_CONTROL_TOP ) { controlPos = SCROLL_CONTROL_TOP; }
  if( SCROLL_CONTROL_BOTTOM < controlPos ) { controlPos = SCROLL_CONTROL_BOTTOM; }

  rate = (float)(controlPos - SCROLL_CONTROL_TOP) / (float)(SCROLL_CONTROL_BOTTOM - SCROLL_CONTROL_TOP);
  min = MIN_SCROLL_VALUE + SCROLL_CURSOR_TOP_MARGIN;
  max = GetMaxScrollValue(work) + SCROLL_CURSOR_TOP_MARGIN;
  cursorPos = min + (max - min) * rate;

  return cursorPos;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スライダーの表示位置を取得する
 *
 * @param work
 *
 * @return スライダーの表示位置
 */
//-----------------------------------------------------------------------------------------
static int GetSliderPos( const RRL_WORK* work )
{
  int controlPos;
  GFL_CLWK* clwk;
  GFL_CLACTPOS pos;
  u16 setSurface;

  clwk = GetClactWork( work, CLWK_SCROLL_CONTROL );
  setSurface = ClactWorkInitData[ CLWK_SCROLL_CONTROL ].setSurface;
  GFL_CLACT_WK_GetPos( clwk, &pos, setSurface );
  controlPos = pos.y;

  return controlPos;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スライダーの移動速度を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void AccelerateSliderSpeed( RRL_WORK* work )
{
  int prev_speed;
  int accl_speed;

  // 加速度を適用
  prev_speed = work->sliderSpeed;
  accl_speed = work->sliderSpeed + work->sliderAccel;

  // 符号が変化
  if( prev_speed * accl_speed <= 0 ) { accl_speed = 0; }

  // 速度を更新
  work->sliderSpeed = accl_speed;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スライダーが動いているかどうかをチェックする
 *
 * @param work
 *
 * @return スライダーが動いている場合 TRUE
 */
//-----------------------------------------------------------------------------------------
static BOOL IsSliderMoving( const RRL_WORK* work )
{
  if( work->sliderSpeed == 0 ) {
    return FALSE;
  }
  else {
    return TRUE;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スライダーの移動速度を観測する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ObserveSliderSpeed( RRL_WORK* work )
{
  // スクロールカーソルの変化量で速度を決定
  work->sliderSpeed = work->scrollCursorPos - work->scrollCursorPos_prev;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スライダーの移動速度を調整する ( 自然移動用 )
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void AdjustSliderSpeed_forFreeMove( RRL_WORK* work )
{
  // 滑りやすいように調整
  work->sliderSpeed *= 1.5;

  // 最大値補正
  if( SLIDER_MAX_SPEED_AT_FREE_MOVE < work->sliderSpeed ) {
    work->sliderSpeed = SLIDER_MAX_SPEED_AT_FREE_MOVE;
  }
  // 最小値補正
  else if( work->sliderSpeed < SLIDER_MIN_SPEED_AT_FREE_MOVE ) {
    work->sliderSpeed = SLIDER_MIN_SPEED_AT_FREE_MOVE;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スライダーの加速度を調整する ( 自然移動用 )
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void AdjustSliderAccel_forFreeMove( RRL_WORK* work )
{
  if( work->sliderSpeed < 0 ) {
    work->sliderAccel = SLIDER_ACCEL_AT_FREE_MOVE;
  }
  else {
    work->sliderAccel = -SLIDER_ACCEL_AT_FREE_MOVE;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロールを開始する
 *
 * @param work
 * @param startPos 開始スクロールカーソル位置
 * @param endPos   終了スクロールカーソル位置
 * @param frames   フレーム数
 */
//-----------------------------------------------------------------------------------------
static void StartScroll( RRL_WORK* work, int startPos, int endPos, int frames )
{
  // スクロールパラメータを設定
  work->scrollStartPos   = startPos;
  work->scrollEndPos     = endPos;
  work->scrollFrames     = frames;
  work->scrollFrameCount = 0;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロールを更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateScroll( RRL_WORK* work )
{
  int maxFrame, nowFrame;
  int startPos, endPos, nowPos;

  GF_ASSERT( work->scrollFrameCount <= work->scrollFrames );

  // フレーム数を更新
  work->scrollFrameCount++;

  // 更新後のカーソル位置を算出
  maxFrame = work->scrollFrames;
  nowFrame = work->scrollFrameCount;
  startPos = work->scrollStartPos;
  endPos   = work->scrollEndPos; 
  nowPos   = startPos + (endPos - startPos) * nowFrame / maxFrame;

  // スクロールカーソル位置を更新
  SetScrollCursorPosForce( work, nowPos );
}


//-----------------------------------------------------------------------------------------
/**
 * @brief スクロールが終了したかどうかを判定する
 *
 * @param work
 *
 * @return スクロールが終了していたら TRUE
 *         そうでなければ FALSE
 */
//-----------------------------------------------------------------------------------------
static BOOL CheckScrollEnd( RRL_WORK* work )
{
  int maxFrame, nowFrame;

  maxFrame = work->scrollFrames;
  nowFrame = work->scrollFrameCount;

  // 終了
  if( maxFrame <= nowFrame ) { 
    return TRUE; 
  }
  else {
    return FALSE;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロールカーソル位置を変更する ( オフセット指定 )
 *
 * @param work
 * @param offset 移動オフセット値
 */
//-----------------------------------------------------------------------------------------
static void ShiftScrollCursorPos( RRL_WORK* work, int offset )
{
  int now;
  int next;

  // 移動後のカーソル位置を決定
  now  = work->scrollCursorPos;
  next = now + offset; 
  if( next < MIN_SCROLL_CURSOR_POS ) { next = MIN_SCROLL_CURSOR_POS; } // 最小値補正
  if( GetMaxScrollCursorPos(work) < next ) { next = GetMaxScrollCursorPos(work); } // 最大値補正

  // カーソル位置を更新
  work->scrollCursorPos_prev = now;
  work->scrollCursorPos      = next;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロールカーソル位置を変更する ( 直値指定 )
 *
 * @param work
 * @param pos 設定するカーソル位置
 */
//-----------------------------------------------------------------------------------------
static void SetScrollCursorPos( RRL_WORK* work, int pos )
{
  // 引数を補正
  if( pos < MIN_SCROLL_CURSOR_POS ) { pos = MIN_SCROLL_CURSOR_POS; } // 最小値補正
  if( GetMaxScrollCursorPos(work) < pos ) { pos = GetMaxScrollCursorPos(work); } // 最大値補正

  // カーソル位置を更新
  work->scrollCursorPos_prev = work->scrollCursorPos;
  work->scrollCursorPos      = pos;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロールカーソル位置を変更する ( 直値指定・範囲限定なし )
 *
 * @param work
 * @param pos 設定するカーソル位置
 */
//-----------------------------------------------------------------------------------------
static void SetScrollCursorPosForce( RRL_WORK* work, int pos )
{
  // カーソル位置を更新
  work->scrollCursorPos_prev = pos;
  work->scrollCursorPos      = pos;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロールカーソル位置を補正する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void AdjustScrollCursor( RRL_WORK* work )
{
  int minPos, maxPos, adjustPos;

  minPos    = GetMinScrollCursorMarginPos(); // カーソル最小位置
  maxPos    = GetMaxScrollCursorMarginPos(); // カーソル最大位置
  adjustPos = work->scrollCursorPos;         // 補正後の座標

  // スクロール実効値に合わせて補正する
  if( adjustPos < minPos ) { adjustPos = minPos; } // 最小値補正
  if( maxPos < adjustPos ) { adjustPos = maxPos; } // 最大値補正

  // スクロールカーソル位置を更新
  work->scrollCursorPos = adjustPos;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロール実効値を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateScrollValue( const RRL_WORK* work )
{
  int nowPos, minPos, maxPos;
  int value;

  nowPos = work->scrollCursorPos;         // カーソル現在位置
  minPos = GetMinScrollCursorMarginPos(); // カーソル余白範囲の最小値
  maxPos = GetMaxScrollCursorMarginPos(); // カーソル余白範囲の最大値

  // カーソルが余白範囲外にあったら更新する
  if( nowPos < minPos ) {
    value = nowPos - SCROLL_CURSOR_TOP_MARGIN;
    GFL_BG_SetScrollReq( MAIN_BG_WINDOW, GFL_BG_SCROLL_Y_SET, value );
    GFL_BG_SetScrollReq( MAIN_BG_FONT,   GFL_BG_SCROLL_Y_SET, value );
  }
  else if( maxPos < nowPos ) {
    value = nowPos + SCROLL_CURSOR_BOTTOM_MARGIN - DISP_DOT_HEIGHT;
    GFL_BG_SetScrollReq( MAIN_BG_WINDOW, GFL_BG_SCROLL_Y_SET, value );
    GFL_BG_SetScrollReq( MAIN_BG_FONT,   GFL_BG_SCROLL_Y_SET, value );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロール実効値をスクロールカーソル位置に合わせて更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void AdjustScrollValue( const RRL_WORK* work )
{
  int value;

  // スクロール実効値を計算
  value = work->scrollCursorPos - SCROLL_CURSOR_TOP_MARGIN;

  // スクロール実効値を補正
  if( value < MIN_SCROLL_VALUE ) { value = MIN_SCROLL_VALUE; } // 最小値補正
  if( GetMaxScrollValue(work) < value ) { value = GetMaxScrollValue(work); } // 最大値補正

  // スクロール実効値を更新
  SetScrollValue( value );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロール実効値を取得する
 *
 * @return 現在のスクロール実効値
 */
//-----------------------------------------------------------------------------------------
static int GetScrollValue()
{
  return GFL_BG_GetScrollY( MAIN_BG_WINDOW );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロール実効値を設定する
 */
//-----------------------------------------------------------------------------------------
static void SetScrollValue( int value )
{
  GFL_BG_SetScroll( MAIN_BG_WINDOW, GFL_BG_SCROLL_Y_SET, value );
  GFL_BG_SetScroll( MAIN_BG_FONT,   GFL_BG_SCROLL_Y_SET, value );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロールカーソル余白範囲の最小値を取得する
 *
 * @return スクロールカーソル余白移動範囲の最小値
 */
//-----------------------------------------------------------------------------------------
static int GetMinScrollCursorMarginPos()
{
  return (GetScrollValue() + SCROLL_CURSOR_TOP_MARGIN);
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロールカーソル余白範囲の最大値を取得する
 *
 * @return スクロールカーソル余白移動範囲の最大値
 */
//-----------------------------------------------------------------------------------------
static int GetMaxScrollCursorMarginPos()
{
  return (GetScrollValue() + DISP_DOT_HEIGHT - SCROLL_CURSOR_BOTTOM_MARGIN);
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 指定したスクロールカーソル位置にあるボタンの調査項目IDを計算する
 *
 * @param pos スクロールカーソル位置
 *
 * @param 指定したスクロールカーソル位置に該当するボタンの調査項目ID
 */
//-----------------------------------------------------------------------------------------
static int CalcTopicID_byScrollCursorPos( int pos )
{
  return (((TOPIC_BUTTON_Y * DOT_PER_CHARA) + pos) / (TOPIC_BUTTON_HEIGHT * DOT_PER_CHARA));
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 指定した調査項目ボタンの上辺に該当するスクロールカーソル位置を計算する
 *
 * @param pos 
 *
 * @param 指定した調査項目ボタンの上辺に該当するスクロールカーソル位置
 */
//-----------------------------------------------------------------------------------------
static int CalcScrollCursorPosOfTopicButtonTop( int topicID )
{
  return (TOPIC_BUTTON_Y * DOT_PER_CHARA) + (topicID * TOPIC_BUTTON_HEIGHT * DOT_PER_CHARA);
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 指定した調査項目ボタンの上辺に該当するスクロールカーソル位置を計算する
 *
 * @param pos 
 *
 * @param 指定した調査項目ボタンの上辺に該当するスクロールカーソル位置
 */
//-----------------------------------------------------------------------------------------
static int CalcScrollCursorPosOfTopicButtonBottom( int topicID )
{
  return CalcScrollCursorPosOfTopicButtonTop(topicID) + TOPIC_BUTTON_HEIGHT * DOT_PER_CHARA - 1; 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 最大スクロール値を取得する
 *
 * @param work
 *
 * @return 最大スクロール値
 */
//-----------------------------------------------------------------------------------------
static int GetMaxScrollValue( const RRL_WORK* work )
{
  int pos;

  pos = 
    GetSelectableTopicNum( work ) * TOPIC_BUTTON_HEIGHT * DOT_PER_CHARA 
    + SCROLL_CURSOR_BOTTOM_MARGIN 
    - DISP_DOT_HEIGHT;

  return pos;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロールカーソルの最大値を取得する
 *
 * @param work
 *
 * @return スクロールカーソルの最大値
 */
//-----------------------------------------------------------------------------------------
static int GetMaxScrollCursorPos( const RRL_WORK* work )
{
  int pos;

  pos = GetMaxScrollValue( work ) + DISP_DOT_HEIGHT - SCROLL_CURSOR_BOTTOM_MARGIN;

  return pos;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロール操作が可能かどうかを判定する
 *
 * @param work
 *
 * @return スクロール操作が可能なら TRUE
 *         そうでないなら FALSE
 */
//-----------------------------------------------------------------------------------------
static BOOL CheckScrollControlCan( const RRL_WORK* work )
{
  int height;
  int min;

  // 項目ボタンの高さ ( ドット単位 ) を算出
  height = GetSelectableTopicNum( work ) * TOPIC_BUTTON_HEIGHT * DOT_PER_CHARA;
  min    = DISP_DOT_HEIGHT - SCROLL_CURSOR_TOP_MARGIN - SCROLL_CURSOR_BOTTOM_MARGIN;

  // 項目ボタンが一画面に収まらない場合にのみ, スクロール操作が可能
  if( min < height ) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目のタッチ範囲を更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void UpdateTopicTouchArea( RRL_WORK* work )
{
  int idx;
  
  for( idx=TOPIC_TOUCH_AREA_TOPIC_0; idx <= TOPIC_TOUCH_AREA_TOPIC_9; idx++ )
  {
    int left, top;

    left = CalcDisplayLeftOfTopicButton( work, idx );
    top  = CalcDisplayTopOfTopicButton( work, idx );

    work->topicTouchHitTable[ idx ].rect.left   = left;
    work->topicTouchHitTable[ idx ].rect.right  = left + TOPIC_BUTTON_TOUCH_AREA_WIDTH;
    work->topicTouchHitTable[ idx ].rect.top    = top; 
    work->topicTouchHitTable[ idx ].rect.bottom = top + TOPIC_BUTTON_TOUCH_AREA_HEIGHT;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief フェードインを開始する
 *
 * @param work
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
 * @brief フェードアウトを開始する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void StartFadeOut( void )
{
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 
      0, 16, 0);
}

//-----------------------------------------------------------------------------------------
/**
 * @brief フェードの終了をチェックする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static BOOL CheckFadeEnd( void )
{
  // フェード終了
  if( GFL_FADE_CheckFade() == FALSE ) {
    return TRUE;
  }
  else {
    return FALSE;
  }
} 

//-----------------------------------------------------------------------------------------
/**
 * @brief パレットのフェードアウトを開始する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void StartPaletteFadeOut( RRL_WORK* work )
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

  // フラグを立てる
  work->palFadeOutFlag = TRUE;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief パレットのフェードインを開始する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void StartPaletteFadeIn( RRL_WORK* work )
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

  // フラグを落とす
  work->palFadeOutFlag = FALSE;
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
static BOOL IsPaletteFadeEnd( RRL_WORK* work )
{
  return (PaletteFadeCheck( work->paletteFadeSystem ) == 0);
}

//------------------------------------------------------------------------------------
/**
 * @brief 共通パレットアニメーションを開始する
 *
 * @param work
 * @param index 開始するパレットアニメのインデックス
 */
//------------------------------------------------------------------------------------
static void StartCommonPaletteAnime( RRL_WORK* work, COMMON_PALETTE_ANIME_INDEX index )
{
  RRC_StartPaletteAnime( work->commonWork, index );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを開始する
 *
 * @param work
 * @param index 開始するパレットアニメのインデックス
 */
//------------------------------------------------------------------------------------
static void StartPaletteAnime( RRL_WORK* work, PALETTE_ANIME_INDEX index )
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
 * @param index 停止するパレットアニメのインデックス
 */
//------------------------------------------------------------------------------------
static void StopPaletteAnime( RRL_WORK* work, PALETTE_ANIME_INDEX index )
{
  PALETTE_ANIME_Stop( work->paletteAnime[ index ] );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションの終了をチェックする
 *
 * @param work
 * @param index 停止するパレットアニメのインデックス
 *
 * @return パレットアニメが終了している場合 TRUE
 */
//------------------------------------------------------------------------------------
static BOOL CheckPaletteAnimeEnd( const RRL_WORK* work, PALETTE_ANIME_INDEX index )
{
  if( PALETTE_ANIME_CheckAnime( work->paletteAnime[ index ] ) ) {
    return FALSE;
  }
  else {
    return TRUE;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションを更新する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void UpdatePaletteAnime( RRL_WORK* work )
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
static void UpdateCommonPaletteAnime( const RRL_WORK* work )
{
  RRC_UpdatePaletteAnime( work->commonWork );
}

//------------------------------------------------------------------------------------
/**
 * @brief キー押しっぱなしカウンタを取得する
 *
 * @param work
 *
 * @return キー押しっぱなしカウンタの値
 */
//------------------------------------------------------------------------------------
static u32 GetKeyContCount( const RRL_WORK* work )
{
  return work->keyContCount;
}

//------------------------------------------------------------------------------------
/**
 * @brief キー押しっぱなしカウンタをインクリメントする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void IncKeyContCount( RRL_WORK* work )
{
  (work->keyContCount)++;
}

//------------------------------------------------------------------------------------
/**
 * @brief キー押しっぱなしカウンタをリセットする
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void ResetKeyContCount( RRL_WORK* work )
{
  work->keyContCount = 0;
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
  RRL_WORK* work = (RRL_WORK*)wk;

  GFL_BG_VBlankFunc();
  GFL_CLACT_SYS_VBlankFunc();
  PaletteFadeTrans( work->paletteFadeSystem );
}


//=========================================================================================
// ◆LAYER 2 取得・設定・判定
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
static GAMESYS_WORK* GetGameSystem( const RRL_WORK* work )
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
static GAMEDATA* GetGameData( const RRL_WORK* work )
{
  return RRC_GetGameData( work->commonWork );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ヒープIDを設定する
 *
 * @param work
 * @param heapID
 */
//-----------------------------------------------------------------------------------------
static void SetHeapID( RRL_WORK* work, HEAPID heapID )
{
  work->heapID = heapID;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 全画面共通ワークを設定する
 *
 * @param work
 * @param commonWork
 */
//-----------------------------------------------------------------------------------------
static void SetCommonWork( RRL_WORK* work, RRC_WORK* commonWork )
{
  work->commonWork = commonWork;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 全画面共通ワークを取得する
 */
//-----------------------------------------------------------------------------------------
static const RRC_WORK* GetCommonWork( const RRL_WORK* work )
{
  return work->commonWork;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief リスト画面の復帰データを設定する
 *
 * @param work
 * @param recoveryData
 */
//-----------------------------------------------------------------------------------------
static void SetRecoveryData( RRL_WORK* work, RRL_RECOVERY_DATA* recoveryData )
{
  work->recoveryData = recoveryData;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief リスト画面の復帰データを反映する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void LoadRecoveryData( RRL_WORK* work )
{
  if( work->recoveryData ) {
    // スクロール実効値
    {
      int scroll_value = RRL_RECOVERY_GetScrollValue( work->recoveryData );
      if( scroll_value < MIN_SCROLL_VALUE ) { scroll_value = MIN_SCROLL_VALUE; }
      if( MAX_SCROLL_VALUE < scroll_value ) { scroll_value = MAX_SCROLL_VALUE; }
      SetScrollValue( scroll_value );
    }
    // スクロールカーソル位置
    {
      int scroll_cursor_pos = RRL_RECOVERY_GetScrollCursorPos( work->recoveryData );
      if( scroll_cursor_pos < MIN_SCROLL_CURSOR_POS ) { scroll_cursor_pos = MIN_SCROLL_CURSOR_POS; }
      if( MAX_SCROLL_CURSOR_POS < scroll_cursor_pos ) { scroll_cursor_pos = MAX_SCROLL_CURSOR_POS; }
      work->scrollCursorPos = scroll_cursor_pos;
    }
    // 調査項目カーソル位置
    work->topicCursorPos = RRL_RECOVERY_GetTopicCursorPos( work->recoveryData );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief リスト画面の復帰データを記録する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SaveRecoveryData( RRL_WORK* work )
{
  if( work->recoveryData ) {
    // スクロール実効値
    {
      int scroll_value = GetScrollValue();
      if( scroll_value < MIN_SCROLL_VALUE ) { scroll_value = MIN_SCROLL_VALUE; }
      if( MAX_SCROLL_VALUE < scroll_value ) { scroll_value = MAX_SCROLL_VALUE; }
      RRL_RECOVERY_SetScrollValue( work->recoveryData, scroll_value );
    }
    // スクロールカーソル位置
    {
      int scroll_cursor_pos = work->scrollCursorPos;
      if( scroll_cursor_pos < MIN_SCROLL_CURSOR_POS ) { scroll_cursor_pos = MIN_SCROLL_CURSOR_POS; }
      if( MAX_SCROLL_CURSOR_POS < scroll_cursor_pos ) { scroll_cursor_pos = MAX_SCROLL_CURSOR_POS; }
      RRL_RECOVERY_SetScrollCursorPos( work->recoveryData, scroll_cursor_pos );
    }
    // 調査項目カーソル位置
    RRL_RECOVERY_SetTopicCursorPos( work->recoveryData, work->topicCursorPos );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 強制終了フラグを取得する
 */
//-----------------------------------------------------------------------------------------
static BOOL CheckForceReturnFlag( const RRL_WORK* work )
{
  const RRC_WORK* commonWork = GetCommonWork( work );
  return RRC_GetForceReturnFlag( commonWork );
}


//-----------------------------------------------------------------------------------------
/**
 * @brief 次の調査項目IDを取得する
 *
 * @param work
 * @param topicID 調査項目ID
 *
 * @return 次の調査項目ID ( 端はループする )
 */
//-----------------------------------------------------------------------------------------
static int GetNextTopicID( const RRL_WORK* work, int topicID )
{
  return (topicID + 1) % GetSelectableTopicNum( work );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 前の調査項目IDを取得する
 *
 * @param work
 * @param topicID 調査項目ID
 *
 * @return 前の調査項目ID ( 端はループする )
 */
//-----------------------------------------------------------------------------------------
static int GetPrevTopicID( const RRL_WORK* work, int topicID )
{
  return (topicID - 1 + GetSelectableTopicNum(work)) % GetSelectableTopicNum(work);
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 選択可能な調査項目の数を取得する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static u8 GetSelectableTopicNum( const RRL_WORK* work )
{
  return work->selectableTopicNum;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 選択中の調査項目IDを取得する
 *
 * @parma work
 *
 * @return 選択中の調査項目ID ( TOPIC_ID_XXXX )
 *         未選択の場合 TOPIC_ID_DUMMY
 */
//-----------------------------------------------------------------------------------------
static u8 GetSelectedTopicID( const RRL_WORK* work )
{
  return work->selectedTopicID;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目を選択する
 *
 * @param work
 * @param topicID 選択する調査項目ID
 */
//-----------------------------------------------------------------------------------------
static void SetSelectedTopicID( RRL_WORK* work, u8 topicID )
{
  work->selectedTopicID = topicID;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief カーソル位置にある調査項目の選択をキャンセルする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ResetSelectedTopicID( RRL_WORK* work )
{
  work->selectedTopicID = TOPIC_ID_DUMMY;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目を選択しているかどうかを判定する
 *
 * @param work
 *
 * @return 調査項目を選択している場合 TRUE
 *         そうでなければ FALSE
 */
//-----------------------------------------------------------------------------------------
static BOOL IsTopicIDSelected( const RRL_WORK* work )
{
  return (work->selectedTopicID != TOPIC_ID_DUMMY);
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 調査項目を選択可能かどうかをチェックする
 *
 * @param work
 * @param topicID 判定する調査項目のID
 *
 * @return 指定した調査項目を選択可能なら TRUE
 *         そうでないなら FALSE
 */
//-----------------------------------------------------------------------------------------
static BOOL CheckTopicCanSelect( const RRL_WORK* work, u8 topicID )
{
  if( topicID < GetSelectableTopicNum( work ) ) {
    return TRUE;
  }
  else {
    return FALSE;
  }
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
static u32 GetObjResourceRegisterIndex( const RRL_WORK* work, OBJ_RESOURCE_ID resID )
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
static GFL_CLUNIT* GetClactUnit( const RRL_WORK* work, CLUNIT_INDEX unitIdx )
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
static GFL_CLWK* GetClactWork( const RRL_WORK* work, CLWK_INDEX wkIdx )
{
  return work->clactWork[ wkIdx ];
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
static void BmpOamSetDrawEnable( RRL_WORK* work, BMPOAM_ACTOR_INDEX actorIdx, BOOL enable )
{
  GF_ASSERT( actorIdx < BMPOAM_ACTOR_NUM );

  // 表示状態を変更
  BmpOam_ActorSetDrawEnable( work->BmpOamActor[ actorIdx ], enable );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メニュー項目に対応するBMP-OAMアクターを取得する
 * 
 * @param work
 * @param menuItem メニュー項目のインデックス
 *
 * @return 指定したメニュー項目に対応するBMP-OAM アクター
 */
//-----------------------------------------------------------------------------------------
static BMPOAM_ACT_PTR GetBmpOamActorOfMenuItem( const RRL_WORK* work, MENU_ITEM menuItem )
{
  BMPOAM_ACTOR_INDEX BmpOamActorIdx;

  BmpOamActorIdx = BmpOamIndexOfMenu[ menuItem ];
  return work->BmpOamActor[ BmpOamActorIdx ];
}


//=========================================================================================
// ◆LAYER 1 生成・初期化・準備・後片付け・破棄
//=========================================================================================

//-----------------------------------------------------------------------------------------
/**
 * @brief リスト画面管理ワークを生成する
 *
 * @param heapID
 *
 * @return リスト画面管理ワーク
 */
//-----------------------------------------------------------------------------------------
static RRL_WORK* CreateListWork( HEAPID heapID )
{
  RRL_WORK* work;

  work = GFL_HEAP_AllocMemory( heapID, sizeof(RRL_WORK) );

  return work;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief リスト画面管理ワークを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitListWork( RRL_WORK* work )
{
  work->recoveryData          = NULL;
  work->state                 = RRL_STATE_SETUP;
  work->stateCount            = 0;
  work->stateEndFlag          = FALSE;
  work->waitFrame             = WAIT_FRAME_BUTTON;
  work->menuCursorPos         = MENU_ITEM_DETERMINATION_OK;
  work->topicCursorPos        = 0;
  work->topicCursorNextPos    = 0;
  work->selectableTopicNum    = 0;
  work->selectedTopicID       = TOPIC_ID_DUMMY;
  work->VBlankTCBSystem       = GFUser_VIntr_GetTCBSYS();
  work->scrollCursorPos       = MIN_SCROLL_CURSOR_POS;
  work->scrollCursorPos_prev  = MIN_SCROLL_CURSOR_POS;
  work->scrollStartPos        = 0;
  work->scrollEndPos          = 0;
  work->scrollFrames          = 0;
  work->scrollFrameCount      = 0;
  work->palFadeOutFlag        = FALSE;
  work->topicCursorActiveFlag = FALSE;
  work->finishResult          = RRL_RESULT_TO_TOP;
  work->finishFlag            = FALSE;
  work->VBlankTask            = NULL;

  InitStateQueue( work );
  InitMessages( work );
  InitFont( work );
  InitBGFonts( work );
  InitClactUnits( work );
  InitClactWorks( work );
  InitOBJResources( work );
  InitPaletteFadeSystem( work );
  InitBitmapDatas( work );
  InitPaletteAnime( work );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief リスト画面管理ワークを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteListWork( RRL_WORK* work )
{
  GFL_HEAP_FreeMemory( work );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 選択可能な調査項目の数をセットアップする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupSelectableTopicNum( RRL_WORK* work )
{
  GAMEDATA* gameData;
  EVENTWORK* evwork;
  int num;

  gameData = GetGameData( work );
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

  // 選択可能な調査項目の数を記憶
  work->selectableTopicNum = num;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを作成する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CreateFont( RRL_WORK* work )
{
  GF_ASSERT( work->font == NULL );

  // 生成
  work->font = GFL_FONT_Create( 
      ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heapID );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitFont( RRL_WORK* work )
{
  work->font = NULL;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief フォントハンドラを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteFont( RRL_WORK* work )
{
  GF_ASSERT( work->font );

  GFL_FONT_Delete( work->font );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを作成する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CreateMessages( RRL_WORK* work )
{
  int msgIdx;

  for( msgIdx=0; msgIdx < MESSAGE_NUM; msgIdx++ )
  {
    GF_ASSERT( work->message[ msgIdx ] == NULL );

    work->message[ msgIdx ] = GFL_MSG_Create( 
        GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, MessageDataID[ msgIdx ], work->heapID ); 
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitMessages( RRL_WORK* work )
{
  int msgIdx;

  for( msgIdx=0; msgIdx < MESSAGE_NUM; msgIdx++ )
  {
    work->message[ msgIdx ] = NULL;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief メッセージデータを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteMessages( RRL_WORK* work )
{
  int msgIdx;

  for( msgIdx=0; msgIdx < MESSAGE_NUM; msgIdx++ )
  {
    GF_ASSERT( work->message[ msgIdx ] );

    GFL_MSG_Delete( work->message[ msgIdx ] );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態キューを作成する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CreateStateQueue( RRL_WORK* work )
{
  GF_ASSERT( work->stateQueue == NULL );

  work->stateQueue = QUEUE_Create( SEQ_QUEUE_SIZE, work->heapID );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態キューを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitStateQueue( RRL_WORK* work )
{
  work->stateQueue = NULL;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 状態キューを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteStateQueue( RRL_WORK* work )
{
  GF_ASSERT( work->stateQueue );

  QUEUE_Delete( work->stateQueue );
}

//-----------------------------------------------------------------------------------------
/**
 * @breif タッチ領域の準備を行う
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupTouchArea( RRL_WORK* work )
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
  for( idx=0; idx < SCROLL_TOUCH_AREA_NUM; idx++ )
  {
    work->scrollTouchHitTable[ idx ].rect.left   = ScrollTouchAreaInitData[ idx ].left;
    work->scrollTouchHitTable[ idx ].rect.right  = ScrollTouchAreaInitData[ idx ].right;
    work->scrollTouchHitTable[ idx ].rect.top    = ScrollTouchAreaInitData[ idx ].top;
    work->scrollTouchHitTable[ idx ].rect.bottom = ScrollTouchAreaInitData[ idx ].bottom;
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief BG の準備
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupBG( RRL_WORK* work )
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief BG の後片付け
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CleanUpBG( RRL_WORK* work )
{
  GFL_BMPWIN_Exit();

  GFL_BG_FreeBGControl( MAIN_BG_FONT );
  GFL_BG_FreeBGControl( MAIN_BG_WINDOW );
  GFL_BG_FreeBGControl( MAIN_BG_BAR );
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
static void SetupSubBG_WINDOW( RRL_WORK* work )
{
  ARCHANDLE* handle;

  // ハンドルオープン
  handle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR_GRAPHIC, GetHeapLowID(work->heapID) ); 

  // スクリーンデータ
  {
    void* src;
    ARCDATID datID;
    NNSG2dScreenData* data;
    datID = NARC_research_radar_graphic_bgu_win1_NSCR;
    src   = GFL_ARC_LoadDataAllocByHandle( handle, datID, work->heapID );
    NNS_G2dGetUnpackedScreenData( src, &data );
    GFL_BG_WriteScreen( SUB_BG_WINDOW, data->rawData, 0, 0, 32, 24 );
    GFL_BG_LoadScreenV_Req( SUB_BG_WINDOW );
    GFL_HEAP_FreeMemory( src );
  }

  // ハンドルクローズ
  GFL_ARC_CloseDataHandle( handle );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 上画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CleanUpSubBG_WINDOW( RRL_WORK* work )
{
}

//-----------------------------------------------------------------------------------------
/**
 * @brief SUB-BG フォント面の準備
 * 
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupSubBG_FONT( RRL_WORK* work )
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
static void CleanUpSubBG_FONT( RRL_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( SUB_BG_FONT, 1, 0 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 下画面 バーBG面 準備
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupMainBG_BAR( RRL_WORK* work )
{
  ARCHANDLE* handle;

  // ハンドルオープン
  handle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR_GRAPHIC, GetHeapLowID(work->heapID) ); 

  // スクリーンデータ
  {
    void* src;
    ARCDATID datID;
    NNSG2dScreenData* data;
    datID = NARC_research_radar_graphic_bgd_title_NSCR;
    src   = GFL_ARC_LoadDataAllocByHandle( handle, datID, work->heapID );
    NNS_G2dGetUnpackedScreenData( src, &data );
    GFL_BG_WriteScreen( MAIN_BG_BAR, data->rawData, 0, 0, 32, 24 );
    GFL_BG_LoadScreenV_Req( MAIN_BG_BAR );
    GFL_HEAP_FreeMemory( src );
  }

  // ハンドルクローズ
  GFL_ARC_CloseDataHandle( handle );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 下画面 バーBG面 後片付け
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CleanUpMainBG_BAR( RRL_WORK* work )
{
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 準備
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupMainBG_WINDOW( RRL_WORK* work )
{
  ARCHANDLE* handle;

  // ハンドルオープン
  handle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR_GRAPHIC, GetHeapLowID(work->heapID) ); 

  // スクリーンデータを読み込む
  {
    void* src;
    ARCDATID datID; 
    NNSG2dScreenData* data;
    int sx, sy;

    sx    = 32;
    sy    = GetSelectableTopicNum( work ) * TOPIC_BUTTON_HEIGHT;
    datID = NARC_research_radar_graphic_bgd_searchbtn_NSCR;
    src   = GFL_ARC_LoadDataAllocByHandle( handle, datID, work->heapID ); 

    NNS_G2dGetUnpackedScreenData( src, &data );
    GFL_BG_WriteScreen( MAIN_BG_WINDOW, data->rawData, 0, 0, sx, sy );
    GFL_BG_LoadScreenV_Req( MAIN_BG_WINDOW );
    GFL_HEAP_FreeMemory( src );
  }

  // ハンドルクローズ
  GFL_ARC_CloseDataHandle( handle );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CleanUpMainBG_WINDOW( RRL_WORK* work )
{
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 下画面 フォントBG面 準備
 * 
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupMainBG_FONT( RRL_WORK* work )
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
static void CleanUpMainBG_FONT( RRL_WORK* work )
{ 
  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( MAIN_BG_FONT, 1, 0 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを作成する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void CreateBGFonts( RRL_WORK* work )
{
  int i;

  // 通常のBGFont
  for( i=0; i<BG_FONT_NUM; i++ )
  {
    BG_FONT_PARAM param;
    GFL_MSGDATA* msgData;
    u32 strID;

    GF_ASSERT( work->BGFont_string[i] == NULL ); 

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
    work->BGFont_string[i] = BG_FONT_Create( &param, work->font, msgData, work->heapID );

    // 文字列を設定
    BG_FONT_SetMessage( work->BGFont_string[i], strID );
  } 

  // 調査項目BGFont
   for( i=0; i < GetSelectableTopicNum(work); i++ )
  {
    BG_FONT_PARAM param;
    GFL_MSGDATA* msgData;
    u32 strID;

    GF_ASSERT( work->BGFont_topic[i] == NULL ); 

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
    work->BGFont_topic[i] = BG_FONT_Create( &param, work->font, msgData, work->heapID );

    // 文字列を設定
    BG_FONT_SetMessage( work->BGFont_topic[i], strID );
  } 
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitBGFonts( RRL_WORK* work )
{
  int idx;

  // 通常のBGFont
  for( idx=0; idx < BG_FONT_NUM; idx++ )
  {
    work->BGFont_string[ idx ] = NULL; 
  }
  // 調査項目BGFont
  for( idx=0; idx < TOPIC_ID_NUM; idx++ )
  {
    work->BGFont_topic[ idx ] = NULL; 
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 文字列描画オブジェクトを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void DeleteBGFonts( RRL_WORK* work )
{
  int i;
  
  // 通常のBGFont
  for( i=0; i < BG_FONT_NUM; i++ )
  {
    GF_ASSERT( work->BGFont_string[i] );
    BG_FONT_Delete( work->BGFont_string[i] );
    work->BGFont_string[i] = NULL;
  }
  // 調査項目BGFont
  for( i=0; i < GetSelectableTopicNum(work); i++ )
  {
    GF_ASSERT( work->BGFont_topic[i] );
    BG_FONT_Delete( work->BGFont_topic[i] );
    work->BGFont_topic[i] = NULL;
  }
} 

//-----------------------------------------------------------------------------------------
/**
 * @brief OBJ リソースを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitOBJResources( RRL_WORK* work )
{
  int i;

  for( i=0; i<OBJ_RESOURCE_NUM; i++ )
  {
    work->objResRegisterIdx[i] = 0; 
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief SUB-OBJ リソースを登録する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void RegisterSubObjResources( RRL_WORK* work )
{
  HEAPID heapID;
  ARCHANDLE* arcHandle;
  u32 character, palette, cellAnime;

  heapID    = work->heapID;
  arcHandle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR_GRAPHIC, GetHeapLowID(heapID) );

  // リソースを登録
  character = GFL_CLGRP_CGR_Register( 
      arcHandle, NARC_research_radar_graphic_obj_NCGR, FALSE, CLSYS_DRAW_SUB, heapID ); 

  palette = GFL_CLGRP_PLTT_Register( 
      arcHandle, NARC_research_radar_graphic_obj_NCLR, CLSYS_DRAW_SUB, 0, heapID );

  cellAnime = GFL_CLGRP_CELLANIM_Register( 
      arcHandle, NARC_research_radar_graphic_obj_NCER, NARC_research_radar_graphic_obj_NANR, heapID ); 

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
static void ReleaseSubObjResources( RRL_WORK* work )
{
  GFL_CLGRP_CGR_Release( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CHARACTER ] );
  GFL_CLGRP_PLTT_Release( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_SUB_CELL_ANIME ] );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief MAIN-OBJ リソースを登録する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void RegisterMainObjResources( RRL_WORK* work )
{
  HEAPID heapID;
  ARCHANDLE* arcHandle;
  u32 character, palette, palette3, cellAnime;

  heapID    = work->heapID;
  arcHandle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR_GRAPHIC, GetHeapLowID(heapID) );

  character = GFL_CLGRP_CGR_Register( 
      arcHandle, NARC_research_radar_graphic_obj_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID ); 

  palette = GFL_CLGRP_PLTT_RegisterEx( 
      arcHandle, NARC_research_radar_graphic_obj_NCLR, CLSYS_DRAW_MAIN, ONE_PALETTE_SIZE*6, 0, 4, heapID ); 

  cellAnime = GFL_CLGRP_CELLANIM_Register( 
      arcHandle, NARC_research_radar_graphic_obj_NCER, NARC_research_radar_graphic_obj_NANR, heapID ); 

  GFL_ARC_CloseDataHandle( arcHandle );

  arcHandle = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON, heapID ); 

  palette3 = GFL_CLGRP_PLTT_RegisterEx( 
      arcHandle, NARC_app_menu_common_task_menu_NCLR, CLSYS_DRAW_MAIN, ONE_PALETTE_SIZE*4, 0, 2, heapID );

  GFL_ARC_CloseDataHandle( arcHandle );

  // 登録インデックスを記憶
  work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CHARACTER ]  = character;
  work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_PALETTE ]    = palette;
  work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CELL_ANIME ] = cellAnime;
  work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_PALETTE_COMMON_BUTTON ] = palette3;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief MAIN-OBJ リソースを解放する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ReleaseMainObjResources( RRL_WORK* work )
{
  GFL_CLGRP_CGR_Release( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CHARACTER ] );
  GFL_CLGRP_PLTT_Release( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_PALETTE ] );
  GFL_CLGRP_CELLANIM_Release( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_CELL_ANIME ] );
  GFL_CLGRP_PLTT_Release( work->objResRegisterIdx[ OBJ_RESOURCE_MAIN_PALETTE_COMMON_BUTTON ] );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief セルアクターユニットを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void InitClactUnits( RRL_WORK* work )
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
static void CreateClactUnits( RRL_WORK* work )
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
static void DeleteClactUnits( RRL_WORK* work )
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
static void InitClactWorks( RRL_WORK* work )
{
  int wkIdx;

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
static void CreateClactWorks( RRL_WORK* work )
{
  int wkIdx;

  for( wkIdx=0; wkIdx < CLWK_NUM; wkIdx++ )
  {
    GFL_CLUNIT* unit;
    GFL_CLWK_DATA wkData;
    u32 charaIdx, paletteIdx, cellAnimeIdx;
    u16 surface;

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
static void DeleteClactWorks( RRL_WORK* work )
{
  int wkIdx;

  for( wkIdx=0; wkIdx < CLWK_NUM; wkIdx++ )
  {
    GF_ASSERT( work->clactWork[ wkIdx ] );

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
static void InitBitmapDatas( RRL_WORK* work )
{
  int idx;

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
static void CreateBitmapDatas( RRL_WORK* work )
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
static void SetupBitmapData_forDefault( RRL_WORK* work )
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
 * @brief「けってい」ボタン用のビットマップデータを準備する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupBitmapData_forOK( RRL_WORK* work )
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief「やめる」ボタン用のビットマップデータを準備する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupBitmapData_forCANCEL( RRL_WORK* work )
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
}

//-----------------------------------------------------------------------------------------
/**
 * @brief ビットマップデータを破棄する
 *
 * @parma work
 */
//-----------------------------------------------------------------------------------------
static void DeleteBitmapDatas( RRL_WORK* work )
{
  int idx;

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
static void SetupBmpOamSystem( RRL_WORK* work )
{
  work->BmpOamSystem = BmpOam_Init( work->heapID, work->clactUnit[ CLUNIT_BMPOAM ] );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM システムの後片付けを行う
 *
 * @parma work
 */
//-----------------------------------------------------------------------------------------
static void CleanUpBmpOamSystem( RRL_WORK* work )
{
  BmpOam_Exit( work->BmpOamSystem );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief BMP-OAM アクターを作成する
 *
 * @parma work
 */
//-----------------------------------------------------------------------------------------
static void CreateBmpOamActors( RRL_WORK* work )
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
static void DeleteBmpOamActors( RRL_WORK* work )
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
static void InitPaletteFadeSystem( RRL_WORK* work )
{ 
  work->paletteFadeSystem = NULL;
}

//-----------------------------------------------------------------------------------------
/**
 * @brief パレットフェードシステムを準備する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupPaletteFadeSystem( RRL_WORK* work )
{
  u32 tcbWorkSize;

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
static void CleanUpPaletteFadeSystem( RRL_WORK* work )
{ 
  // パレットフェードのリクエストワークを破棄
  PaletteFadeWorkAllocFree( work->paletteFadeSystem, FADE_MAIN_BG );
  PaletteFadeWorkAllocFree( work->paletteFadeSystem, FADE_MAIN_OBJ );

  // フェード管理システムを破棄
  PaletteFadeFree( work->paletteFadeSystem );
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを生成する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void CreatePaletteAnime( RRL_WORK* work )
{
  int idx;

  for( idx=0; idx < PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] == NULL );

    work->paletteAnime[ idx ] = PALETTE_ANIME_Create( work->heapID );
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを初期化する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void InitPaletteAnime( RRL_WORK* work )
{
  int idx;

  for( idx=0; idx < PALETTE_ANIME_NUM; idx++ )
  {
    work->paletteAnime[ idx ] = NULL;
  }
}

//------------------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを破棄する
 *
 * @param work
 */
//------------------------------------------------------------------------------------
static void DeletePaletteAnime( RRL_WORK* work )
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
static void SetupPaletteAnime( RRL_WORK* work )
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
static void CleanUpPaletteAnime( RRL_WORK* work )
{
  int idx;

  for( idx=0; idx < PALETTE_ANIME_NUM; idx++ )
  {
    GF_ASSERT( work->paletteAnime[ idx ] );

    // 操作していたパレットを元に戻す
    PALETTE_ANIME_Reset( work->paletteAnime[ idx ] );
  }
}

//-----------------------------------------------------------------------------------------
/**
 * @brief スクロールバーをセットアップする
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupScrollBar( RRL_WORK* work )
{
  GFL_CLWK* scrollbar;
  GFL_CLACTPOS pos;
  u16 setSurface;

  scrollbar = GetClactWork( work, CLWK_SCROLL_BAR );
  pos.x = 128;
  pos.y = 96;
  setSurface = ClactWorkInitData[ CLWK_SCROLL_BAR ].setSurface; 

  GFL_CLACT_WK_SetPos( scrollbar, &pos, setSurface );
  GFL_CLACT_WK_SetDrawEnable( scrollbar, TRUE );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief 通信アイコンをセットアップする
 * 
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void SetupWirelessIcon( const RRL_WORK* work )
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
static void RegisterVBlankTask( RRL_WORK* work )
{
  GF_ASSERT( work->VBlankTask == NULL );

  work->VBlankTask = GFUser_VIntr_CreateTCB( VBlankFunc, work, 0 );
}

//-----------------------------------------------------------------------------------------
/**
 * @brief VBlank期間中のタスクを解除する
 *
 * @param work
 */
//-----------------------------------------------------------------------------------------
static void ReleaseVBlankTask( RRL_WORK* work )
{ 
  GF_ASSERT( work->VBlankTask );

  GFL_TCB_DeleteTask( work->VBlankTask );
  work->VBlankTask = NULL;
} 
