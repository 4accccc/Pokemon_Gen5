/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 初期画面 ( メニュー画面 )
 * @file   research_menu.c
 * @author obata
 * @date   2010.02.03
 */
///////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "research_menu.h"
#include "research_common.h"
#include "bg_font.h"

#include "print/gf_font.h"   // for GFL_FONT_xxxx
#include "print/printsys.h"  // for PRINTSYS_xxxx

#include "system/main.h"            // for HEAPID_xxxx
#include "arc/arc_def.h"            // for ARCID_xxxx
#include "arc/research_radar.naix"  // for NARC_research_radar_xxxx
#include "arc/font/font.naix"       // for NARC_font_xxxx
#include "arc/message.naix"         // for NARC_message_xxxx


//===============================================================================
// ■定数
//===============================================================================
// パレット番号
#define MAIN_BG_PALETTE_BACK_GROUND_0 (0x0)
#define MAIN_BG_PALETTE_BACK_GROUND_1 (0x1)
#define MAIN_BG_PALETTE_BACK_GROUND_2 (0x2)
#define MAIN_BG_PALETTE_BACK_GROUND_3 (0x3)
#define MAIN_BG_PALETTE_BACK_GROUND_4 (0x4)
#define MAIN_BG_PALETTE_BACK_GROUND_5 (0x5)
#define MAIN_BG_PALETTE_WINDOW_ON     (0xa)  // @todo 仮あて
#define MAIN_BG_PALETTE_WINDOW_OFF    (0x7)
#define MAIN_BG_PALETTE_FONT          (0x8)

#define SUB_BG_PALETTE_BACK_GROUND_0 (0x0)
#define SUB_BG_PALETTE_BACK_GROUND_1 (0x1)
#define SUB_BG_PALETTE_BACK_GROUND_2 (0x2)
#define SUB_BG_PALETTE_BACK_GROUND_3 (0x3)
#define SUB_BG_PALETTE_BACK_GROUND_4 (0x4)
#define SUB_BG_PALETTE_BACK_GROUND_5 (0x5)
#define SUB_BG_PALETTE_WINDOW        (0x6)
#define SUB_BG_PALETTE_FONT          (0x7)

// SUB-BG
#define SUB_BG_WINDOW  (GFL_BG_FRAME1_S)  // ウィンドウ面
#define SUB_BG_FONT    (GFL_BG_FRAME2_S)  // フォント面
// SUB-BG 表示優先順位
#define SUB_BG_PRIORITY_WINDOW (2)  // ウィンドウ面
#define SUB_BG_PRIORITY_FONT   (1)  // フォント面

// MAIN-BG
#define MAIN_BG_WINDOW (GFL_BG_FRAME2_M)  // ウィンドウ面
#define MAIN_BG_FONT   (GFL_BG_FRAME3_M)  // フォント面
// MAIN-BG 表示優先順位
#define MAIN_BG_PRIORITY_WINDOW (2)  // ウィンドウ面
#define MAIN_BG_PRIORITY_FONT   (1)  // フォント面

// 上画面 説明文ウィンドウ
#define DIRECTION_WINDOW_X               (0)   // X座標   (キャラクター単位)
#define DIRECTION_WINDOW_Y               (17)  // Y座標   (キャラクター単位)
#define DIRECTION_WINDOW_WIDTH           (32)  // Xサイズ (キャラクター単位)
#define DIRECTION_WINDOW_HEIGHT          (6)   // Yサイズ (キャラクター単位)
#define DIRECTION_WINDOW_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define DIRECTION_WINDOW_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define DIRECTION_WINDOW_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define DIRECTION_WINDOW_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define DIRECTION_WINDOW_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 下画面 ボタン ( 調査内容の決定 )
#define CHANGE_BUTTON_X               (3)   // X座標   (キャラクター単位)
#define CHANGE_BUTTON_Y               (6)   // Y座標   (キャラクター単位)
#define CHANGE_BUTTON_WIDTH           (26)  // Xサイズ (キャラクター単位)
#define CHANGE_BUTTON_HEIGHT          (4)   // Yサイズ (キャラクター単位)
#define CHANGE_BUTTON_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define CHANGE_BUTTON_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define CHANGE_BUTTON_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define CHANGE_BUTTON_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define CHANGE_BUTTON_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号

// 下画面 ボタン ( 調査報告を見る )
#define CHECK_BUTTON_X               (3)   // X座標   (キャラクター単位)
#define CHECK_BUTTON_Y               (13)  // Y座標   (キャラクター単位)
#define CHECK_BUTTON_WIDTH           (26)  // Xサイズ (キャラクター単位)
#define CHECK_BUTTON_HEIGHT          (4)   // Yサイズ (キャラクター単位)
#define CHECK_BUTTON_STRING_OFFSET_X (0)   // 文字列の書き込み先オフセットX
#define CHECK_BUTTON_STRING_OFFSET_Y (0)   // 文字列の書き込み先オフセットY
#define CHECK_BUTTON_STRING_COLOR_L  (1)   // フォント (文字) のカラー番号
#define CHECK_BUTTON_STRING_COLOR_S  (2)   // フォント (影)　 のカラー番号
#define CHECK_BUTTON_STRING_COLOR_B  (0)   // フォント (背景) のカラー番号


// 処理シーケンス
typedef enum{
  RESEARCH_MENU_SEQ_SETUP,    // 準備
  RESEARCH_MENU_SEQ_KEY_WAIT, // キー入力待ち
  RESEARCH_MENU_SEQ_CLEAN_UP, // 後片付け
  RESEARCH_MENU_SEQ_FINISH,   // 終了
} RESEARCH_MENU_SEQ;

// メニュー項目
typedef enum{
  MENU_ITEM_CHANGE_RESEARCH,  //「調査内容を変更する」
  MENU_ITEM_CHECK_RESEARCH,   //「調査報告を確認する」
  MENU_ITEM_NUM,              // メニュー項目数
} MENU_ITEM;


// 上画面 説明文
static const BG_FONT_PARAM BGFontParam_direction = 
{
  SUB_BG_FONT,
  DIRECTION_WINDOW_X,
  DIRECTION_WINDOW_Y,
  DIRECTION_WINDOW_WIDTH,
  DIRECTION_WINDOW_HEIGHT,
  DIRECTION_WINDOW_STRING_OFFSET_X,
  DIRECTION_WINDOW_STRING_OFFSET_Y,
  SUB_BG_PALETTE_FONT,
  DIRECTION_WINDOW_STRING_COLOR_L,
  DIRECTION_WINDOW_STRING_COLOR_S,
  DIRECTION_WINDOW_STRING_COLOR_B,
};

// 下画面「調査内容の決定」
static const BG_FONT_PARAM BGFontParam_changeButton = 
{
  MAIN_BG_FONT,
  CHANGE_BUTTON_X,
  CHANGE_BUTTON_Y,
  CHANGE_BUTTON_WIDTH,
  CHANGE_BUTTON_HEIGHT,
  CHANGE_BUTTON_STRING_OFFSET_X,
  CHANGE_BUTTON_STRING_OFFSET_Y,
  MAIN_BG_PALETTE_FONT,
  CHANGE_BUTTON_STRING_COLOR_L,
  CHANGE_BUTTON_STRING_COLOR_S,
  CHANGE_BUTTON_STRING_COLOR_B,
};

// 下画面「調査報告の確認」
static const BG_FONT_PARAM BGFontParam_checkButton = 
{
  MAIN_BG_FONT,
  CHECK_BUTTON_X,
  CHECK_BUTTON_Y,
  CHECK_BUTTON_WIDTH,
  CHECK_BUTTON_HEIGHT,
  CHECK_BUTTON_STRING_OFFSET_X,
  CHECK_BUTTON_STRING_OFFSET_Y,
  MAIN_BG_PALETTE_FONT,
  CHECK_BUTTON_STRING_COLOR_L,
  CHECK_BUTTON_STRING_COLOR_S,
  CHECK_BUTTON_STRING_COLOR_B,
};

// OBJ リソースインデックス
typedef enum{
  OBJ_RESOURCE_CHR,    // キャラクタ
  OBJ_RESOURCE_PLT,    // パレット
  OBJ_RESOURCE_CEL,    // セル
  OBJ_RESOURCE_ANM,    // アニメーション
} OBJ_RESOURCE_INDEX;


//=============================================================================== 
// ■調査初期画面 ワーク
//=============================================================================== 
struct _RESEARCH_MENU_WORK
{ 
  RESEARCH_MENU_SEQ    seq;        // 処理シーケンス
  RESEARCH_MENU_RESULT result;     // 画面終了結果
  MENU_ITEM            cursorPos;  // カーソル位置

  HEAPID       heapID;   // ヒープID
  GFL_FONT*    font;     // フォント
  GFL_MSGDATA* message;  // メッセージ

  // 上画面
  BG_FONT* BGFont_direction;  // 説明文

  // 下画面
  BG_FONT* BGFont_changeButton;  //「調査内容の決定」ボタン
  BG_FONT* BGFont_checkButton;   //「調査報告を見る」ボタン
};


//===============================================================================
// ■非公開関数
//===============================================================================
// シーケンス処理
static RESEARCH_MENU_SEQ Main_SETUP   ( RESEARCH_MENU_WORK* work );
static RESEARCH_MENU_SEQ Main_KEY_WAIT( RESEARCH_MENU_WORK* work );
static RESEARCH_MENU_SEQ Main_CLEAN_UP( RESEARCH_MENU_WORK* work );
// シーケンス制御
static void SwitchSeq( RESEARCH_MENU_WORK* work, RESEARCH_MENU_SEQ nextSeq );
static void SetSeq   ( RESEARCH_MENU_WORK* work, RESEARCH_MENU_SEQ seq );
// カーソル移動
static void MoveCursorUp  ( RESEARCH_MENU_WORK* work );
static void MoveCursorDown( RESEARCH_MENU_WORK* work );
// 画面終了結果
static void SetResult( RESEARCH_MENU_WORK* work, RESEARCH_MENU_RESULT result );

// フォント
static void SetupFont  ( RESEARCH_MENU_WORK* work );
static void CleanUpFont( RESEARCH_MENU_WORK* work );
// メッセージ
static void SetupMessage  ( RESEARCH_MENU_WORK* work );
static void CleanUpMessage( RESEARCH_MENU_WORK* work );

// BG
static void SetupBG  ( RESEARCH_MENU_WORK* work );
static void CleanUpBG( RESEARCH_MENU_WORK* work );
// 上画面 ウィンドウBG面
static void SetupSubBG_window  ( RESEARCH_MENU_WORK* work );
static void CleanUpSubBG_window( RESEARCH_MENU_WORK* work );
// 上画面 フォントBG面
static void SetupSubBG_font  ( RESEARCH_MENU_WORK* work );
static void CleanUpSubBG_font( RESEARCH_MENU_WORK* work );
// 下画面 ウィンドウBG面
static void SetupMainBG_window  ( RESEARCH_MENU_WORK* work );
static void CleanUpMainBG_window( RESEARCH_MENU_WORK* work );
// 下画面 フォントBG面
static void SetupMainBG_font  ( RESEARCH_MENU_WORK* work );
static void CleanUpMainBG_font( RESEARCH_MENU_WORK* work );

// メニュー項目操作
static void MenuItemSwitchOn ( MENU_ITEM menuItem );
static void MenuItemSwitchOff( MENU_ITEM menuItem );


//=============================================================================== 
// □調査初期画面 制御関数
//=============================================================================== 

//-------------------------------------------------------------------------------
/**
 * @brief 調査初期画面ワークの生成
 *
 * @param heapID
 */
//-------------------------------------------------------------------------------
RESEARCH_MENU_WORK* CreateResearchMenuWork( HEAPID heapID )
{
  RESEARCH_MENU_WORK* work;

  // 生成
  work = GFL_HEAP_AllocMemory( heapID, sizeof(RESEARCH_MENU_WORK) );

  // 初期化
  work->seq                 = RESEARCH_MENU_SEQ_SETUP;
  work->result              = RESEARCH_MENU_RESULT_NONE;
  work->cursorPos           = MENU_ITEM_CHANGE_RESEARCH;
  work->heapID              = heapID;
  work->font                = NULL;
  work->message             = NULL;
  work->BGFont_direction    = NULL;
  work->BGFont_changeButton = NULL;
  work->BGFont_checkButton  = NULL;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: create work\n" );

  return work;
}

//-------------------------------------------------------------------------------
/**
 * @brief 調査初期画面ワークの破棄
 *
 * @param heapID
 */
//-------------------------------------------------------------------------------
void DeleteResearchMenuWork( RESEARCH_MENU_WORK* work )
{
  if( work == NULL )
  {
    GF_ASSERT(0);
    return;
  }
  GFL_HEAP_FreeMemory( work );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: delete work\n" );
} 

//-------------------------------------------------------------------------------
/**
 * @brief 調査初期画面 メイン動作
 *
 * @param work
 */
//-------------------------------------------------------------------------------
RESEARCH_MENU_RESULT ResearchMenuMain( RESEARCH_MENU_WORK* work )
{
  RESEARCH_MENU_SEQ nextSeq;

  // シーケンスごとの処理
  switch( work->seq )
  {
  case RESEARCH_MENU_SEQ_SETUP:     nextSeq = Main_SETUP   ( work );  break;
  case RESEARCH_MENU_SEQ_KEY_WAIT:  nextSeq = Main_KEY_WAIT( work );  break;
  case RESEARCH_MENU_SEQ_CLEAN_UP:  nextSeq = Main_CLEAN_UP( work );  break;
  case RESEARCH_MENU_SEQ_FINISH:    return work->result;
  default:  GF_ASSERT(0);
  }

  // シーケンス更新
  SwitchSeq( work, nextSeq );

  return RESEARCH_MENU_RESULT_CONTINUE;
}


//===============================================================================
// ■シーケンス処理
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief 準備シーケンス ( RESEARCH_MENU_SEQ_SETUP ) の処理
 *
 * @param work
 *
 * @return シーケンスが変化する場合 次のシーケンス番号
 *         シーケンスが継続する場合 現在のシーケンス番号
 */
//-------------------------------------------------------------------------------
static RESEARCH_MENU_SEQ Main_SETUP( RESEARCH_MENU_WORK* work )
{
  // BG 準備
  SetupFont         ( work );
  SetupMessage      ( work );
  SetupBG           ( work );
  SetupSubBG_window ( work );
  SetupSubBG_font   ( work );
  SetupMainBG_window( work );
  SetupMainBG_font  ( work );

  // 初期カーソル位置のメニュー項目を選択状態にする
  MenuItemSwitchOn( work->cursorPos );

  // 画面フェードイン
  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, 0);

  return RESEARCH_MENU_SEQ_KEY_WAIT;
}

//-------------------------------------------------------------------------------
/**
 * @brief キー入力待ちシーケンス ( RESEARCH_MENU_SEQ_KEY_WAIT ) の処理
 *
 * @param work
 *
 * @return シーケンスが変化する場合 次のシーケンス番号
 *         シーケンスが継続する場合 現在のシーケンス番号
 */
//-------------------------------------------------------------------------------
static RESEARCH_MENU_SEQ Main_KEY_WAIT( RESEARCH_MENU_WORK* work )
{
  RESEARCH_MENU_SEQ nextSeq;
  int trg;

  nextSeq = work->seq;
  trg     = GFL_UI_KEY_GetTrg();

  //--------
  // 上キー
  if( trg & PAD_KEY_UP )
  {
    MoveCursorUp( work );
  }

  //--------
  // 下キー
  if( trg & PAD_KEY_DOWN )
  {
    MoveCursorDown( work );
  }

  //--------
  // Aボタン
  if( trg & PAD_BUTTON_A )
  {
    switch( work->cursorPos )
    {
    //「調査内容を変更する」
    case MENU_ITEM_CHANGE_RESEARCH:
      SetResult( work, RESEARCH_MENU_RESULT_TO_SELECT );  // 画面終了結果を決定
      nextSeq = RESEARCH_MENU_SEQ_CLEAN_UP;               // シーケンス遷移
      break;

    //「調査報告を確認する」
    case MENU_ITEM_CHECK_RESEARCH:
      SetResult( work, RESEARCH_MENU_RESULT_TO_CHECK );  // 画面終了結果を決定
      nextSeq = RESEARCH_MENU_SEQ_CLEAN_UP;              // シーケンス遷移
      break;

    // エラー
    default:
      GF_ASSERT(0);
    }
  } 
  return nextSeq;
}

//-------------------------------------------------------------------------------
/**
 * @brief 後片付けシーケンス ( RESEARCH_MENU_SEQ_CLEAN_UP ) の処理
 *
 * @param work
 *
 * @return シーケンスが変化する場合 次のシーケンス番号
 *         シーケンスが継続する場合 現在のシーケンス番号
 */
//-------------------------------------------------------------------------------
static RESEARCH_MENU_SEQ Main_CLEAN_UP( RESEARCH_MENU_WORK* work )
{
  // BG 後片付け
  CleanUpMainBG_font  ( work );
  CleanUpMainBG_window( work );
  CleanUpSubBG_font   ( work );
  CleanUpSubBG_window ( work );
  CleanUpBG           ( work );
  CleanUpMessage      ( work );
  CleanUpFont         ( work );

  return RESEARCH_MENU_SEQ_FINISH;
}


//===============================================================================
// ■シーケンス制御
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief シーケンスを変更する
 *
 * @param work
 * @param nextSeq 変更後のシーケンス
 */
//-------------------------------------------------------------------------------
static void SwitchSeq( RESEARCH_MENU_WORK* work, RESEARCH_MENU_SEQ nextSeq )
{
  // 変化なし
  if( work->seq == nextSeq){ return; }

  // 変更
  SetSeq( work, nextSeq ); 
} 

//-------------------------------------------------------------------------------
/**
 * @brief シーケンスを設定する
 *
 * @param work
 * @parma seq  設定するシーケンス
 */
//-------------------------------------------------------------------------------
static void SetSeq( RESEARCH_MENU_WORK* work, RESEARCH_MENU_SEQ seq )
{
  work->seq = seq;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: set seq ==> %d\n", seq );
}


//===============================================================================
// ■カーソル移動
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief カーソルを上へ移動する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
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

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: move cursor ==> %d\n", work->cursorPos );
}

//-------------------------------------------------------------------------------
/**
 * @brief カーソルを下へ移動する
 *
 * @param work
 */
//-------------------------------------------------------------------------------
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

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: move cursor ==> %d\n", work->cursorPos );
}


//===============================================================================
// ■画面終了結果
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief 画面の終了結果を決定する
 *
 * @param work
 * @param result 結果
 */
//-------------------------------------------------------------------------------
static void SetResult( RESEARCH_MENU_WORK* work, RESEARCH_MENU_RESULT result )
{
  // 多重設定
  GF_ASSERT( work->result == RESEARCH_MENU_RESULT_NONE );

  // 設定
  work->result = result;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: set result (%d)\n", result );
}


//===============================================================================
// ■フォント
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief フォントハンドラ 準備
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void SetupFont( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->font == NULL );

  // 生成
  work->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, 
                                GFL_FONT_LOADTYPE_FILE, TRUE, work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: setup font\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief フォントハンドラ 後片付け
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CleanUpFont( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->font );

  // 削除
  GFL_FONT_Delete( work->font );
  work->font = NULL;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up font\n" );
}


//===============================================================================
// ■メッセージ
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief メッセージデータ 準備
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void SetupMessage( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->message == NULL );

  // 作成
  work->message = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 0, work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: setup message\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief メッセージデータ 後片付け
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CleanUpMessage( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->message );

  // 削除
  GFL_MSG_Delete( work->message );
  work->message = NULL;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up message\n" );
}


//===============================================================================
// ■BG
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief BGを初期化する
 */
//-------------------------------------------------------------------------------
static void SetupBG( RESEARCH_MENU_WORK* work )
{
  // BG システム ヘッダ
  static const GFL_BG_SYS_HEADER BGSysHeader = 
  {
    GX_DISPMODE_GRAPHICS,   // 表示モード指定
    GX_BGMODE_0,            // ＢＧモード指定(メインスクリーン)
    GX_BGMODE_0,            // ＢＧモード指定(サブスクリーン)
    GX_BG0_AS_2D            // ＢＧ０の２Ｄ、３Ｄモード選択
  };
  // BG コントロール ヘッダ
  const GFL_BG_BGCNT_HEADER BGCnt1S = 
  {
    0, 0,					          // 初期表示位置
    0x800,						      // スクリーンバッファサイズ
    0,							        // スクリーンバッファオフセット
    GFL_BG_SCRSIZ_256x256,	// スクリーンサイズ
    GX_BG_COLORMODE_16,			// カラーモード
    GX_BG_SCRBASE_0x0800,		// スクリーンベースブロック
    GX_BG_CHARBASE_0x04000,	// キャラクタベースブロック
    GFL_BG_CHRSIZ_256x256,	// キャラクタエリアサイズ
    GX_BG_EXTPLTT_01,			  // BG拡張パレットスロット選択
    SUB_BG_PRIORITY_WINDOW, // 表示プライオリティー
    0,							        // エリアオーバーフラグ
    0,							        // DUMMY
    FALSE,						      // モザイク設定
  }; 
  const GFL_BG_BGCNT_HEADER BGCnt2S = 
  {
    0, 0,					          // 初期表示位置
    0x800,						      // スクリーンバッファサイズ
    0,							        // スクリーンバッファオフセット
    GFL_BG_SCRSIZ_256x256,	// スクリーンサイズ
    GX_BG_COLORMODE_16,			// カラーモード
    GX_BG_SCRBASE_0x1000,		// スクリーンベースブロック
    GX_BG_CHARBASE_0x10000,	// キャラクタベースブロック
    GFL_BG_CHRSIZ_256x256,	// キャラクタエリアサイズ
    GX_BG_EXTPLTT_01,			  // BG拡張パレットスロット選択
    SUB_BG_PRIORITY_FONT,   // 表示プライオリティー
    0,							        // エリアオーバーフラグ
    0,							        // DUMMY
    FALSE,						      // モザイク設定
  }; 
  const GFL_BG_BGCNT_HEADER BGCnt2M = 
  {
    0, 0,					          // 初期表示位置
    0x800,						      // スクリーンバッファサイズ
    0,							        // スクリーンバッファオフセット
    GFL_BG_SCRSIZ_256x256,	// スクリーンサイズ
    GX_BG_COLORMODE_16,			// カラーモード
    GX_BG_SCRBASE_0x0800,		// スクリーンベースブロック
    GX_BG_CHARBASE_0x04000,	// キャラクタベースブロック
    GFL_BG_CHRSIZ_256x256,	// キャラクタエリアサイズ
    GX_BG_EXTPLTT_01,			  // BG拡張パレットスロット選択
    MAIN_BG_PRIORITY_WINDOW,// 表示プライオリティー
    0,							        // エリアオーバーフラグ
    0,							        // DUMMY
    FALSE,						      // モザイク設定
  }; 
  const GFL_BG_BGCNT_HEADER BGCnt3M = 
  {
    0, 0,					          // 初期表示位置
    0x800,						      // スクリーンバッファサイズ
    0,							        // スクリーンバッファオフセット
    GFL_BG_SCRSIZ_256x256,	// スクリーンサイズ
    GX_BG_COLORMODE_16,			// カラーモード
    GX_BG_SCRBASE_0x1000,		// スクリーンベースブロック
    GX_BG_CHARBASE_0x10000,	// キャラクタベースブロック
    GFL_BG_CHRSIZ_256x256,	// キャラクタエリアサイズ
    GX_BG_EXTPLTT_01,			  // BG拡張パレットスロット選択
    MAIN_BG_PRIORITY_FONT,  // 表示プライオリティー
    0,							        // エリアオーバーフラグ
    0,							        // DUMMY
    FALSE,						      // モザイク設定
  }; 

  GFL_BG_SetBGMode( &BGSysHeader );
  GFL_BG_SetBGControl( SUB_BG_WINDOW,  &BGCnt1S, GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( SUB_BG_FONT,    &BGCnt2S, GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( MAIN_BG_WINDOW, &BGCnt2M, GFL_BG_MODE_TEXT );
  GFL_BG_SetBGControl( MAIN_BG_FONT,   &BGCnt3M, GFL_BG_MODE_TEXT );

  GFL_BG_SetVisible( SUB_BG_BACK_GROUND,  VISIBLE_ON );
  GFL_BG_SetVisible( SUB_BG_WINDOW,       VISIBLE_ON );
  GFL_BG_SetVisible( SUB_BG_FONT,         VISIBLE_ON );
  GFL_BG_SetVisible( MAIN_BG_BACK_GROUND, VISIBLE_ON );
  GFL_BG_SetVisible( MAIN_BG_WINDOW,      VISIBLE_ON );
  GFL_BG_SetVisible( MAIN_BG_FONT,        VISIBLE_ON );

  GFL_BMPWIN_Init( work->heapID );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: setup BG\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief BG後片付け
 */
//-------------------------------------------------------------------------------
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


//===============================================================================
// ■上画面 ウィンドウBG面
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief 上画面 ウィンドウBG面 準備
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void SetupSubBG_window( RESEARCH_MENU_WORK* work )
{
  // データ読み込み
  {
    ARCHANDLE* handle;

    // ハンドルオープン
    handle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR, work->heapID ); 

    // パレットデータ
    // ※背景BG面と同じデータを参照
    /*
    {
      void* src;
      u16 offset;
      ARCDATID datID;
      NNSG2dPaletteData* data;
      datID = NARC_research_radar_bgu_NCLR;
      src   = GFL_ARC_LoadDataAllocByHandle( handle, datID, work->heapID );
      NNS_G2dGetUnpackedPaletteData( src, &data );
      GFL_BG_LoadPalette( SUB_BG_WINDOW, data->pRawData,
                          ONE_PALETTE_SIZE * 1,
                          ONE_PALETTE_SIZE * 6 );
      GFL_HEAP_FreeMemory( src );
    }
    */
    // キャラクタデータ
    // ※背景BG面と同じデータを参照
    /*
    {
      void* src;
      ARCDATID datID;
      NNSG2dCharacterData* data;
      datID = NARC_research_radar_bgd_NCGR;
      src   = GFL_ARC_LoadDataAllocByHandle( handle, datID, heapID );
      NNS_G2dGetUnpackedBGCharacterData( src, &data );
      GFL_BG_LoadCharacter( MAIN_BG_BACK_GROUND, data->pRawData, data->szByte, 0 );
      GFL_HEAP_FreeMemory( src );
    }
    */
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

//-------------------------------------------------------------------------------
/**
 * @brief 上画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CleanUpSubBG_window( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up SUB-BG-WINDOW\n" );
}


//===============================================================================
// ■上画面 フォントBG面
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief 上画面 フォントBG面 準備
 * 
 * @param work
 */
//-------------------------------------------------------------------------------
static void SetupSubBG_font( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->BGFont_direction == NULL ); 

  // NULLキャラ確保
  GFL_BG_FillCharacter( SUB_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( SUB_BG_FONT );

  // 文字描画オブジェクト作成
  work->BGFont_direction = BG_FONT_Create( &BGFontParam_direction, 
                                           work->font, work->message, work->heapID );
  BG_FONT_SetString( work->BGFont_direction, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: setup SUB-BG-FONT\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 上画面 フォントBG面 後片付け
 * 
 * @param work
 */
//-------------------------------------------------------------------------------
static void CleanUpSubBG_font( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->BGFont_direction );

  // 文字描画オブジェクト破棄
  BG_FONT_Delete( work->BGFont_direction );

  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( SUB_BG_FONT, 1, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up SUB-BG-FONT\n" );
}


//===============================================================================
// ■下画面 ウィンドウBG面
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 準備
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void SetupMainBG_window( RESEARCH_MENU_WORK* work )
{
  // データ読み込み
  {
    ARCHANDLE* handle;

    // ハンドルオープン
    handle = GFL_ARC_OpenDataHandle( ARCID_RESEARCH_RADAR, work->heapID ); 

    // パレットデータ
    // ※背景BG面と同じデータを参照
    /*
    {
      void* src;
      u16 offset;
      ARCDATID datID;
      NNSG2dPaletteData* data;
      datID = NARC_research_radar_bgu_NCLR;
      src   = GFL_ARC_LoadDataAllocByHandle( handle, datID, work->heapID );
      NNS_G2dGetUnpackedPaletteData( src, &data );
      GFL_BG_LoadPalette( SUB_BG_WINDOW, data->pRawData,
                          ONE_PALETTE_SIZE * 1,
                          ONE_PALETTE_SIZE * 6 );
      GFL_HEAP_FreeMemory( src );
    }
    */
    // キャラクタデータ
    // ※背景BG面と同じデータを参照
    /*
    {
      void* src;
      ARCDATID datID;
      NNSG2dCharacterData* data;
      datID = NARC_research_radar_bgd_NCGR;
      src   = GFL_ARC_LoadDataAllocByHandle( handle, datID, heapID );
      NNS_G2dGetUnpackedBGCharacterData( src, &data );
      GFL_BG_LoadCharacter( MAIN_BG_BACK_GROUND, data->pRawData, data->szByte, 0 );
      GFL_HEAP_FreeMemory( src );
    }
    */
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

//-------------------------------------------------------------------------------
/**
 * @brief 下画面 ウィンドウBG面 後片付け
 *
 * @param work
 */
//-------------------------------------------------------------------------------
static void CleanUpMainBG_window( RESEARCH_MENU_WORK* work )
{
  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up MAIN-BG-WINDOW\n" );
}


//===============================================================================
// ■下画面 フォントBG面
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @brief 下画面 フォントBG面 準備
 * 
 * @param work
 */
//-------------------------------------------------------------------------------
static void SetupMainBG_font( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->BGFont_changeButton == NULL ); 
  GF_ASSERT( work->BGFont_checkButton == NULL ); 

  // NULLキャラ確保
  GFL_BG_FillCharacter( MAIN_BG_FONT, 0, 1, 0 );

  // クリア
  GFL_BG_ClearScreen( MAIN_BG_FONT );

  // 文字列描画オブジェクト作成
  work->BGFont_changeButton = BG_FONT_Create( &BGFontParam_changeButton, 
                                              work->font, work->message, work->heapID );
  work->BGFont_checkButton = BG_FONT_Create( &BGFontParam_checkButton, 
                                             work->font, work->message, work->heapID );
  BG_FONT_SetString( work->BGFont_changeButton, 0 );
  BG_FONT_SetString( work->BGFont_checkButton, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: setup MAIN-BG-FONT\n" );
}

//-------------------------------------------------------------------------------
/**
 * @brief 下画面 フォントBG面 後片付け
 * 
 * @param work
 */
//-------------------------------------------------------------------------------
static void CleanUpMainBG_font( RESEARCH_MENU_WORK* work )
{
  GF_ASSERT( work->BGFont_checkButton );
  GF_ASSERT( work->BGFont_changeButton );
  
  // 文字列描画オブジェクト破棄
  BG_FONT_Delete( work->BGFont_checkButton );
  BG_FONT_Delete( work->BGFont_changeButton ); 
  work->BGFont_checkButton  = NULL;
  work->BGFont_changeButton = NULL;

  // NULLキャラ解放
  GFL_BG_FillCharacterRelease( MAIN_BG_FONT, 1, 0 );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "RESEARCH-MENU: clean up MAIN-BG-FONT\n" );
}


//===============================================================================
// ■メニュー項目操作
//===============================================================================

//-------------------------------------------------------------------------------
/**
 * @biref メニュー項目を選択状態にする
 *
 * @param menuItem 選択状態にするメニュー項目を指定
 */
//-------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------
/**
 * @biref メニュー項目を非選択状態にする
 *
 * @param menuItem 非選択状態にするメニュー項目を指定
 */
//-------------------------------------------------------------------------------
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
