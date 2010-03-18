////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  季節表示イベント
 * @file   event_season_display.c
 * @author obata
 * @date   2009.11.25
 */
////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "fieldmap.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "event_season_display.h"

#include "system/main.h"           // for HEAPID_xxxx
#include "gamesystem/pm_season.h"  // for PMSEASON_xxxx
#include "arc/arc_def.h"           // for ARCID_SEASON_DISPLAY
#include "arc/season_display.naix" // for datid


extern void FIELDMAP_InitBGMode( void );
extern void FIELDMAP_InitBG( FIELDMAP_WORK* fieldWork );


//==============================================================================
// ■BG
//==============================================================================
#define BG_FRAME_NUM       (8)                       // BGフレーム数(メイン＋サブ)
#define BG_FRAME_FIELD     (GFL_BG_FRAME0_M)         // フィールドを表示しているBG面
#define BG_FRAME_SEASON    (GFL_BG_FRAME3_M)         // 季節表示に使用するBG面
#define BG_PRIORITY_OTHER  (3)                       // 使用しないBG面の表示優先順位
#define BG_PRIORITY_FIELD  (1)                       // フィールド表示面の表示優先順位
#define BG_PRIORITY_SEASON (0)                       // 季節表示面の表示優先順位
#define ALPHA_MASK_SEASON  (GX_BLEND_PLANEMASK_BG3)  // αブレンディング第一対象面(季節表示)
#define ALPHA_MASK_FIELD   (GX_BLEND_PLANEMASK_BG0)  // αブレンディング第二対象面(フィールド)
#define ALPHA_MAX          (16)                      // α最大値
#define ALPHA_MIN          (0)                       // α最小値
#define BRIGHT_MAX         (0)                       // 輝度最大値
#define BRIGHT_MIN         (-16)                     // 輝度最小値

// BGヘッダ
static const GFL_BG_SYS_HEADER BGHeader = {
	GX_DISPMODE_GRAPHICS, 
  GX_BGMODE_0, 
  GX_BGMODE_0, 
  GX_BG0_AS_3D
};
// BGコントロール
static const GFL_BG_BGCNT_HEADER BGControl = 
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
  BG_PRIORITY_SEASON,     // 表示プライオリティー
  0,							        // エリアオーバーフラグ
  0,							        // DUMMY
  FALSE,						      // モザイク設定
}; 

//--------------------------------------------------------------
/// ディスプレイ環境データ
//--------------------------------------------------------------
static const GFL_DISP_VRAM dispVram =
{
	GX_VRAM_BG_128_D, //メイン2DエンジンのBGに割り当て 
	GX_VRAM_BGEXTPLTT_NONE, //メイン2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_SUB_BG_32_H, //サブ2DエンジンのBGに割り当て
	GX_VRAM_SUB_BGEXTPLTT_NONE, //サブ2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_OBJ_64_E, //メイン2DエンジンのOBJに割り当て
	GX_VRAM_OBJEXTPLTT_NONE, //メイン2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_SUB_OBJ_16_I, //サブ2DエンジンのOBJに割り当て
	GX_VRAM_SUB_OBJEXTPLTT_NONE, //サブ2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_TEX_012_ABC, //テクスチャイメージスロットに割り当て
	GX_VRAM_TEXPLTT_0_G, //テクスチャパレットスロットに割り当て
	GX_OBJVRAMMODE_CHAR_1D_64K, // メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K, // サブOBJマッピングモード
};


//==============================================================================
// ■定数
//==============================================================================
// 季節遷移の表示 ( EVENT_MODE_MAPCHANGE )
#define MAPCHANGE_FI_FRAME_SHORT   (10)  // フェードイン時間
#define MAPCHANGE_WAIT_FRAME_SHORT (30)  // 待機時間
#define MAPCHANGE_FO_FRAME_SHORT   (10)  // フェードアウト時間
// 現季節の表示 ( EVENT_MODE_MAPCHANGE )
#define MAPCHANGE_FI_FRAME_LONG   (10)  // フェードイン時間
#define MAPCHANGE_WAIT_FRAME_LONG (60)  // 待機時間
#define MAPCHANGE_FO_FRAME_LONG   (20)  // フェードアウト時間
// 現季節の表示 ( EVENT_MODE_GAMESTART )
#define GAMESTART_FI_FRAME   (30)  // フェードイン時間
#define GAMESTART_WAIT_FRAME (120) // 待機時間
#define GAMESTART_FO_FRAME   (60)  // フェードアウト時間

// イベントシーケンス
enum {
  SEQ_INIT,    // 初期化
  SEQ_PREPARE, // 表示準備
  SEQ_FADEIN,  // フェードイン
  SEQ_WAIT,    // 待機
  SEQ_FADEOUT, // フェードアウト
  SEQ_EXIT,    // 終了
};

// 動作モード
typedef enum {
  EVENT_MODE_MAPCHANGE, // マップチェンジとしての季節表示
  EVENT_MODE_GAMESTART, //『さいしょから』でゲームを始めたときの季節表示
} EVENT_MODE;


//==============================================================================
// ■イベントワーク
//==============================================================================
typedef struct {
  GAMESYS_WORK* gameSystem;
  FIELDMAP_WORK* fieldmap;
  HEAPID heapID;

  EVENT_MODE mode;       // 動作モード
  BOOL       skipFlag;   // スキップするかどうか
  u8         endSeason;  // 表示する最後の季節
  u8         dispSeason; // 表示中の季節
  u32        count;      // カウンタ
  u32        maxCount;   // カウンタ最大値
} EVENT_WORK;


//==============================================================================
// ■関数インデックス
//==============================================================================
static void SetupBG( EVENT_WORK* work ); // BG をセットアップする
static void SetupBG_MAPCHANGE( EVENT_WORK* work ); // BG をセットアップする ( EVENT_MODE_MAPCHANGE )
static void SetupBG_GAMESTART( EVENT_WORK* work ); // BG をセットアップする ( EVENT_MODE_GAMESTART )
static void CleanUpBG( EVENT_WORK* work ); // BG をクリーンアップする
static void CleanUpBG_MAPCHANGE( EVENT_WORK* work ); // BG をクリーンアップする ( EVENT_MODE_MAPCHANGE )
static void CleanUpBG_GAMESTART( EVENT_WORK* work ); // BG をクリーンアップする ( EVENT_MODE_GAMESTART )
static void LoadGraphicData( u8 season, HEAPID heap_id ); // グラフィックデータを読み込む
static void UpdateAlpha( const EVENT_WORK* work, int seq ); // α値を更新する
static void UpdateAlpha_MAPCHANGE( const EVENT_WORK* work, int seq ); // α値を更新する ( EVENT_MODE_MAPCHANGE )
static void UpdateAlpha_GAMESTART( const EVENT_WORK* work, int seq ); // α値を更新する ( EVENT_MODE_GAMESTART )
static int GetNextSeq( const EVENT_WORK* work, int seq ); // 次のシーケンスを取得する
static void SetSeq( EVENT_WORK* work, int* seq, int next ); // シーケンスを設定する
static void StepNextSeq( EVENT_WORK* work, int* seq ); // シーケンスを変更する
static void EventExit( EVENT_WORK* work ); // イベント終了時の処理
static EVENT_MODE GetMode( const EVENT_WORK* work ); // 動作モードを取得する
static BOOL IsSkipOn( const EVENT_WORK* work ); // スキップするかどうかを取得する
static void SetSkipOn( EVENT_WORK* work ); // スキップONに設定する
static BOOL CheckCountOver( const EVENT_WORK* work ); // カウンタが最大値をオーバーしたかどうかを判定する
static int GetMaxCountOfFadeIn( const EVENT_WORK* work ); // フェードインの最大フレーム数を取得する
static int GetMaxCountOfFadeOut( const EVENT_WORK* work ); // フェードアウトの最大フレーム数を取得する
static int GetMaxCountOfWait( const EVENT_WORK* work ); // 待機シーケンスの最大フレーム数を取得する


//==============================================================================
// ■イベント処理関数
//==============================================================================
static GMEVENT_RESULT SeasonDisplay( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = (EVENT_WORK*)wk;

  // シーケンス固有処理
  switch( *seq ) {
  // 初期化
  case SEQ_INIT:
    break;

  // 表示準備
  case SEQ_PREPARE:
    break;

  // フェードイン
  case SEQ_FADEIN:
    UpdateAlpha( work, *seq );  // α値更新
    break;

  // 待機
  case SEQ_WAIT:
    break;

  // フェードアウト
  case SEQ_FADEOUT:
    UpdateAlpha( work, *seq );  // α値更新
    break;

  // 終了
  case SEQ_EXIT:
    EventExit( work );
    return GMEVENT_RES_FINISH;
  }

  // 共通処理
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ) {
    // マップチェンジモードはスキップ可
    if( GetMode(work) == EVENT_MODE_MAPCHANGE ) { SetSkipOn(work); }
  }
  work->count++;
  StepNextSeq( work, seq ); 
  return GMEVENT_RES_CONTINUE;
}


//==============================================================================
// ■外部公開関数
//==============================================================================

//------------------------------------------------------------------------------
/**
 * @brief 四季表示イベントを生成する
 *
 * @param gameSystem
 * @param fieldmap 
 * @param start    開始季節
 * @param end      最終季節 
 *
 * @return マップチェンジとして機能する四季表示イベント
 */
//------------------------------------------------------------------------------
GMEVENT* EVENT_SeasonDisplay( 
    GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, u8 start, u8 end )
{
  GMEVENT* event;
  EVENT_WORK* work;
  int* seq;

  // イベント生成
  event = GMEVENT_Create( gameSystem, NULL, SeasonDisplay, sizeof(EVENT_WORK) ); 
  work  = GMEVENT_GetEventWork( event );
  seq   = GMEVENT_GetSequenceWork( event );

  // イベントワーク初期化
  work->gameSystem = gameSystem;
  work->fieldmap   = fieldmap;
  work->heapID     = FIELDMAP_GetHeapID( fieldmap );
  work->mode       = EVENT_MODE_MAPCHANGE;
  work->skipFlag   = FALSE;
  work->endSeason  = end;
  work->dispSeason = PMSEASON_GetPrevSeasonID( start );

  // シーケンス初期化
  SetSeq( work, seq, SEQ_INIT );

  return event;
}

//------------------------------------------------------------------------------
/**
 * @brief 四季表示イベントを生成する
 *
 * @param gameSystem
 * @param start    開始季節
 * @param end      最終季節 
 *
 * @return 単独動作を想定した四季表示イベント
 */
//------------------------------------------------------------------------------
GMEVENT* EVENT_SimpleSeasonDisplay( GAMESYS_WORK* gameSystem, u8 start, u8 end )
{
  GMEVENT* event;
  EVENT_WORK* work;
  int* seq;

  // イベント生成
  event = GMEVENT_Create( gameSystem, NULL, SeasonDisplay, sizeof(EVENT_WORK) ); 
  work  = GMEVENT_GetEventWork( event );
  seq   = GMEVENT_GetSequenceWork( event );

  // イベントワーク初期化
  work->gameSystem = gameSystem;
  work->fieldmap   = NULL;
  work->heapID     = HEAPID_PROC;
  work->mode       = EVENT_MODE_GAMESTART;
  work->skipFlag   = FALSE;
  work->endSeason  = end;
  work->dispSeason = PMSEASON_GetPrevSeasonID( start );

  // シーケンス初期化
  SetSeq( work, seq, SEQ_INIT );

  return event;
}


//==============================================================================
// ■非公開関数のプロトタイプ宣言
//==============================================================================

//------------------------------------------------------------------------------
/**
 * @brief BGに関する初期設定を行う
 *
 * @param work イベントワーク
 */
//------------------------------------------------------------------------------
static void SetupBG( EVENT_WORK* work )
{ 
  switch( GetMode(work) ) {
  case EVENT_MODE_MAPCHANGE: SetupBG_MAPCHANGE( work ); break;
  case EVENT_MODE_GAMESTART: SetupBG_GAMESTART( work ); break;
  default: GF_ASSERT(0);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief BGに関する初期設定を行う ( EVENT_MODE_MAPCHANGE )
 *
 * @param work イベントワーク
 */
//------------------------------------------------------------------------------
static void SetupBG_MAPCHANGE( EVENT_WORK* work )
{ 
  // 不正呼び出し
  GF_ASSERT( GetMode(work) == EVENT_MODE_MAPCHANGE );

  // BGモード設定と表示設定の復帰
  {
    int mv = GFL_DISP_GetMainVisible();
    FIELDMAP_InitBGMode();  // この中で表示設定がクリアされる
    GFL_DISP_GX_SetVisibleControlDirect( mv );
  }

  // プライオリティを設定
  GFL_BG_SetPriority( BG_FRAME_FIELD,  BG_PRIORITY_FIELD );
  GFL_BG_SetPriority( BG_FRAME_SEASON, BG_PRIORITY_SEASON ); 
  GFL_BG_SetPriority( GFL_BG_FRAME1_M, BG_PRIORITY_OTHER ); 
  GFL_BG_SetPriority( GFL_BG_FRAME2_M, BG_PRIORITY_OTHER ); 

  // キャラVRAM・スクリーンVRAMをクリア
  GFL_BG_ClearFrame( BG_FRAME_SEASON ); 
}

//------------------------------------------------------------------------------
/**
 * @brief BGに関する初期設定を行う ( EVENT_MODE_GAMESTART )
 *
 * @param work イベントワーク
 */
//------------------------------------------------------------------------------
static void SetupBG_GAMESTART( EVENT_WORK* work )
{ 
  // 不正呼び出し
  GF_ASSERT( GetMode(work) == EVENT_MODE_GAMESTART );

  GFL_DISP_SetBank( &dispVram );
  GFL_BG_Init( work->heapID );
  GFL_BG_SetBGMode( &BGHeader );
  GFL_BG_SetBGControl( BG_FRAME_SEASON, &BGControl, GFL_BG_MODE_TEXT );

  // プライオリティを設定
  GFL_BG_SetPriority( BG_FRAME_FIELD,  BG_PRIORITY_FIELD );
  GFL_BG_SetPriority( BG_FRAME_SEASON, BG_PRIORITY_SEASON ); 
  GFL_BG_SetPriority( GFL_BG_FRAME1_M, BG_PRIORITY_OTHER ); 
  GFL_BG_SetPriority( GFL_BG_FRAME2_M, BG_PRIORITY_OTHER ); 

  // キャラVRAM・スクリーンVRAMをクリア
  GFL_BG_ClearFrame( BG_FRAME_SEASON ); 
}

//------------------------------------------------------------------------------
/**
 * @brief BGに関する後始末を行う
 *
 * @param work
 */
//------------------------------------------------------------------------------
static void CleanUpBG( EVENT_WORK* work )
{
  switch( GetMode(work) ) {
  case EVENT_MODE_MAPCHANGE: CleanUpBG_MAPCHANGE( work ); break;
  case EVENT_MODE_GAMESTART: CleanUpBG_GAMESTART( work ); break;
  default: GF_ASSERT(0);
  }
}

//------------------------------------------------------------------------------
/**
 * @brief BGに関する後始末を行う ( EVENT_MODE_MAPCHANGE )
 *
 * @param work
 */
//------------------------------------------------------------------------------
static void CleanUpBG_MAPCHANGE( EVENT_WORK* work )
{
  // 不正呼び出し
  GF_ASSERT( GetMode(work) == EVENT_MODE_MAPCHANGE );
}

//------------------------------------------------------------------------------
/**
 * @brief BGに関する後始末を行う ( EVENT_MODE_GAMESTART )
 *
 * @param work
 */
//------------------------------------------------------------------------------
static void CleanUpBG_GAMESTART( EVENT_WORK* work )
{
  // 不正呼び出し
  GF_ASSERT( GetMode(work) == EVENT_MODE_GAMESTART );

  GFL_BG_FreeBGControl( BG_FRAME_SEASON );
}

//------------------------------------------------------------------------------
/**
 * @brief 指定した季節の表示に必要なグラフィックデータを読み込む
 *
 * @param season  読み込みを行う季節
 * @param haep_id 使用するヒープID
 */
//------------------------------------------------------------------------------
static void LoadGraphicData( u8 season, HEAPID heap_id )
{
  ARCDATID datid_pltt, datid_char, datid_scrn;

  // データIDを決定
  switch( season ) {
  default:
    OBATA_Printf( "EVENT-SEASON-DISPLAY: 想定外の季節IDが指定されました。\n" );
  case PMSEASON_SPRING:
    OBATA_Printf( "EVENT-SEASON-DISPLAY: load graphic data (spring)\n" );
    datid_pltt = NARC_season_display_spring_nclr;
    datid_char = NARC_season_display_spring_ncgr;
    datid_scrn = NARC_season_display_spring_nscr;
    break;
  case PMSEASON_SUMMER:
    OBATA_Printf( "EVENT-SEASON-DISPLAY: load graphic data (summer)\n" );
    datid_pltt = NARC_season_display_summer_nclr;
    datid_char = NARC_season_display_summer_ncgr;
    datid_scrn = NARC_season_display_summer_nscr;
    break;
  case PMSEASON_AUTUMN:
    OBATA_Printf( "EVENT-SEASON-DISPLAY: load graphic data (autumn)\n" );
    datid_pltt = NARC_season_display_autumn_nclr;
    datid_char = NARC_season_display_autumn_ncgr;
    datid_scrn = NARC_season_display_autumn_nscr;
    break;
  case PMSEASON_WINTER:
    OBATA_Printf( "EVENT-SEASON-DISPLAY: load graphic data (winter)\n" );
    datid_pltt = NARC_season_display_winter_nclr;
    datid_char = NARC_season_display_winter_ncgr;
    datid_scrn = NARC_season_display_winter_nscr;
    break;
  }

  // 読み込み
  {
    ARCHANDLE* handle;
    handle = GFL_ARC_OpenDataHandle( ARCID_SEASON_DISPLAY, heap_id ); 
    { // パレットデータ
      void* src;
      NNSG2dPaletteData* pltt_data;
      src = GFL_ARC_LoadDataAllocByHandle( handle, datid_pltt, heap_id );
      NNS_G2dGetUnpackedPaletteData( src, &pltt_data );
      GFL_BG_LoadPalette( BG_FRAME_SEASON, pltt_data->pRawData, 0x20, 0 );
      GFL_HEAP_FreeMemory( src );
    }
    { // キャラクタデータ
      void* src;
      NNSG2dCharacterData* char_data;
      src = GFL_ARC_LoadDataAllocByHandle( handle, datid_char, heap_id );
      NNS_G2dGetUnpackedBGCharacterData( src, &char_data );
      GFL_BG_LoadCharacter( BG_FRAME_SEASON, char_data->pRawData, char_data->szByte, 0 );
      GFL_HEAP_FreeMemory( src );
    }
    { // スクリーンデータ
      void* src;
      NNSG2dScreenData* scrn_data;
      src = GFL_ARC_LoadDataAllocByHandle( handle, datid_scrn, heap_id );
      NNS_G2dGetUnpackedScreenData( src, &scrn_data );
      GFL_BG_LoadScreen( BG_FRAME_SEASON, scrn_data->rawData, scrn_data->szByte, 0 );
      GFL_HEAP_FreeMemory( src );
    }
    GFL_ARC_CloseDataHandle( handle );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief αブレンディングを更新する
 *
 * @param work イベントワーク
 * @param seq  現在のシーケンス
 */
//------------------------------------------------------------------------------
static void UpdateAlpha( const EVENT_WORK* work, int seq )
{
  switch( GetMode(work) ) {
  case EVENT_MODE_MAPCHANGE: UpdateAlpha_MAPCHANGE( work, seq ); break;
  case EVENT_MODE_GAMESTART: UpdateAlpha_GAMESTART( work, seq ); break;
  default: GF_ASSERT(0);
  }
}
//------------------------------------------------------------------------------
/**
 * @brief αブレンディングを更新する ( EVENT_MODE_MAPCHANGE )
 *
 * @param work イベントワーク
 * @param seq  現在のシーケンス
 */
//------------------------------------------------------------------------------
static void UpdateAlpha_MAPCHANGE( const EVENT_WORK* work, int seq )
{
  // 不正呼び出し
  GF_ASSERT( GetMode(work) == EVENT_MODE_MAPCHANGE );

  // フェードイン
  if( seq == SEQ_FADEIN ) { 
    float rate   = (float)work->count / (float)work->maxCount;
    int   bright = BRIGHT_MIN + (BRIGHT_MAX - BRIGHT_MIN) * rate;
    G2_SetBlendBrightness( ALPHA_MASK_SEASON, bright );
  }
  // フェードアウト
  else if( seq == SEQ_FADEOUT ) { 
    // 最後のフェードアウト
    if( work->dispSeason == work->endSeason ) { 
      // 最後のフェードはαブレンド
      {
        float rate   = (float)work->count / (float)work->maxCount;
        int   alpha2 = ALPHA_MAX * rate;
        int   alpha1 = ALPHA_MAX - alpha2;
        G2_SetBlendAlpha( ALPHA_MASK_SEASON, ALPHA_MASK_FIELD, alpha1, alpha2 );
      }
      // 最後のフェードは下画面を輝度フェードインさせる
      {
        float rate   = (float)work->count / (float)work->maxCount;
        int   bright = BRIGHT_MIN + (BRIGHT_MAX - BRIGHT_MIN) * rate;
        GXS_SetMasterBrightness( bright );
      }
    }
    // 最後以外のフェードアウト
    else { 
      float rate   = (float)work->count / (float)work->maxCount;
      int   bright = BRIGHT_MAX - (BRIGHT_MAX - BRIGHT_MIN) * rate;
      G2_SetBlendBrightness( ALPHA_MASK_SEASON, bright );
    }
  }
}
//------------------------------------------------------------------------------
/**
 * @brief αブレンディングを更新する ( EVENT_MODE_GAMESTART )
 *
 * @param work イベントワーク
 * @param seq  現在のシーケンス
 */
//------------------------------------------------------------------------------
static void UpdateAlpha_GAMESTART( const EVENT_WORK* work, int seq )
{
  // 不正呼び出し
  GF_ASSERT( GetMode(work) == EVENT_MODE_GAMESTART );

  // フェードイン
  if( seq == SEQ_FADEIN ) { 
    float rate   = (float)work->count / (float)work->maxCount;
    int   bright = BRIGHT_MIN + (BRIGHT_MAX - BRIGHT_MIN) * rate;
    G2_SetBlendBrightness( ALPHA_MASK_SEASON, bright );
  }
  // フェードアウト
  else if( seq == SEQ_FADEOUT ) { 
    float rate   = (float)work->count / (float)work->maxCount;
    int   bright = BRIGHT_MAX - (BRIGHT_MAX - BRIGHT_MIN) * rate;
    G2_SetBlendBrightness( ALPHA_MASK_SEASON, bright );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief 次の処理シーケンスを取得する
 *
 * @param work イベントワーク
 * @param seq  現在のシーケンス
 *
 * @return 次のシーケンス番号( 変化がなければ現在と同じシーケンス番号 )
 */
//------------------------------------------------------------------------------
static int GetNextSeq( const EVENT_WORK* work, int seq )
{
  int next = seq;

  switch( seq ) {
  // 初期化
  case SEQ_INIT:
    next = SEQ_PREPARE;
    break;

  // 表示準備
  case SEQ_PREPARE:
    next = SEQ_FADEIN;
    break;

  // フェードイン
  case SEQ_FADEIN:
    if( CheckCountOver(work) == TRUE ) {
      next = SEQ_WAIT;
    }
    break;

  // 待機
  case SEQ_WAIT:
    if( (CheckCountOver(work) == TRUE) || (IsSkipOn(work) == TRUE) ) {
      next = SEQ_FADEOUT; 
    }
    break;

  // フェードアウト
  case SEQ_FADEOUT:
    if( CheckCountOver(work) == TRUE ) {
      // 現在の季節を表示し終えたら, イベント終了
      if( work->endSeason == work->dispSeason ) {
        next = SEQ_EXIT; 
      }
      else { 
        next = SEQ_PREPARE;
      }
    }
    break;

  // 終了
  case SEQ_EXIT:
    break;
  } 

  return next;
}

//------------------------------------------------------------------------------
/**
 * @brief 処理シーケンスの設定と, それに伴う初期化を行う
 *
 * @param work イベントワーク
 * @param seq  シーケンスワークへのポインタ
 * @param next 次のシーケンス
 */
//------------------------------------------------------------------------------
static void SetSeq( EVENT_WORK* work, int* seq, int next )
{
  switch( next ) {
  // 初期化
  case SEQ_INIT:
    SetupBG( work ); // BG初期設定
    break;

  // 表示準備
  case SEQ_PREPARE:
    // 表示する季節を決定
    work->dispSeason = PMSEASON_GetNextSeasonID( work->dispSeason );
    // グラフィックデータ読み込み
    LoadGraphicData( work->dispSeason, work->heapID );
    break;

  // フェードイン
  case SEQ_FADEIN:
    // BG表示開始
    GFL_BG_SetVisible( BG_FRAME_SEASON, VISIBLE_ON ); 
    // フィールド非表示
    GFL_BG_SetVisible( BG_FRAME_FIELD, VISIBLE_OFF ); 
    // 画面フェード無効化
    GX_SetMasterBrightness(0);
    G2_SetBlendBrightness( ALPHA_MASK_SEASON, BRIGHT_MIN );
    // フェード時間を決定
    work->maxCount = GetMaxCountOfFadeIn( work );
    break;

  // 待機
  case SEQ_WAIT:
    // 待機時間を決定
    work->maxCount = GetMaxCountOfWait( work );
    break;

  // フェードアウト
  case SEQ_FADEOUT:
    // フェード時間を決定
    work->maxCount = GetMaxCountOfFadeOut( work );
    // 最後の表示をフェードアウトさせる時は, 裏にフィールドを表示する
    if( (GetMode(work) == EVENT_MODE_MAPCHANGE) && 
        (work->dispSeason == work->endSeason) ) {
      FIELDMAP_InitBG( work->fieldmap );
      GFL_BG_SetVisible( BG_FRAME_SEASON, VISIBLE_ON ); 
    }
    break;

  // 終了
  case SEQ_EXIT:
    break;
  }

  // シーケンス変更
  *seq = next;
  work->count = 0;

  // DEBUG:
  OBATA_Printf( "EVENT-SEASON-DISPLAY: seq ==> " );
  switch( next ) {
  case SEQ_INIT:    OBATA_Printf( "INIT\n" );    break;
  case SEQ_PREPARE: OBATA_Printf( "PREPARE\n" ); break;
  case SEQ_FADEIN:  OBATA_Printf( "FADEIN\n" );  break;
  case SEQ_WAIT:    OBATA_Printf( "WAIT\n" );    break;
  case SEQ_FADEOUT: OBATA_Printf( "FADEOUT\n" ); break;
  case SEQ_EXIT:    OBATA_Printf( "EXIT\n" );    break;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief 次のシーケンスに移行する
 *
 * @param work イベントワーク
 * @param seq  シーケンスワークへのポインタ
 */
//------------------------------------------------------------------------------
static void StepNextSeq( EVENT_WORK* work, int* seq )
{
  int next;
  
  // 次のシーケンスを取得
  next = GetNextSeq( work, *seq );

  // 変化したら遷移する
  if( *seq != next ) {
    SetSeq( work, seq, next );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief イベント終了処理
 *
 * @param work イベントワーク
 */
//------------------------------------------------------------------------------
static void EventExit( EVENT_WORK* work )
{
  // BGを非表示に設定
  GFL_BG_SetVisible( BG_FRAME_SEASON, VISIBLE_OFF ); 
}

//------------------------------------------------------------------------------
/**
 * @brief 動作モードを取得する
 *
 * @param work
 *
 * @return 動作モード ( EVENT_MODE_xxxx )
 */
//------------------------------------------------------------------------------
static EVENT_MODE GetMode( const EVENT_WORK* work )
{
  return work->mode;
}

//------------------------------------------------------------------------------
/**
 * @brief スキップするかどうかを取得する
 *
 * @param work
 *
 * @return スキップする場合 TRUE, そうでなければ FALSE
 */
//------------------------------------------------------------------------------
static BOOL IsSkipOn( const EVENT_WORK* work )
{
  return work->skipFlag;
}

//------------------------------------------------------------------------------
/**
 * @brief スキップONに設定する
 *
 * @param work
 */
//------------------------------------------------------------------------------
static void SetSkipOn( EVENT_WORK* work )
{
  work->skipFlag = TRUE;
}

//------------------------------------------------------------------------------
/**
 * @brief カウンタが最大値をオーバーしたかどうかを判定する
 *
 * @param work
 *
 * @return カウンタが最大値を超えている場合 TRUE, そうでなければ FALSE
 */
//------------------------------------------------------------------------------
static BOOL CheckCountOver( const EVENT_WORK* work )
{
  return ( work->maxCount < work->count );
}

//------------------------------------------------------------------------------
/**
 * @brief フェードインの最大フレーム数を取得する
 *
 * @param work
 *
 * @return フェードインの最大フレーム数
 */
//------------------------------------------------------------------------------
static int GetMaxCountOfFadeIn( const EVENT_WORK* work )
{
  //『さいしょから』の時は固定
  if( GetMode(work) == EVENT_MODE_GAMESTART ) {
    return GAMESTART_FI_FRAME;
  }

  // マップチェンジ中なら, 表示する季節に応じて変化
  if( work->dispSeason == work->endSeason ) {
    return MAPCHANGE_FI_FRAME_LONG; // 現在の季節は長め
  }
  else {
    return MAPCHANGE_FI_FRAME_SHORT; // 移ろう季節は短め
  }
}
//------------------------------------------------------------------------------
/**
 * @brief フェードアウトの最大フレーム数を取得する
 *
 * @param work
 *
 * @return フェードアウトの最大フレーム数
 */
//------------------------------------------------------------------------------
static int GetMaxCountOfFadeOut( const EVENT_WORK* work )
{
  //『さいしょから』の時は固定
  if( GetMode(work) == EVENT_MODE_GAMESTART ) {
    return GAMESTART_FO_FRAME;
  }

  // フェード時間を決定
  if( work->dispSeason == work->endSeason ) {
    return MAPCHANGE_FO_FRAME_LONG; // 現在の季節は長め
  }
  else {
    return MAPCHANGE_FO_FRAME_SHORT; // 移ろう季節は短め
  }
}
//------------------------------------------------------------------------------
/**
 * @brief 待機シーケンスの最大フレーム数を取得する
 *
 * @param work
 *
 * @return 待機シーケンスの最大フレーム数
 */
//------------------------------------------------------------------------------
static int GetMaxCountOfWait( const EVENT_WORK* work )
{
  //『さいしょから』の時は固定
  if( GetMode(work) == EVENT_MODE_GAMESTART ) {
    return GAMESTART_WAIT_FRAME;
  }

  // マップチェンジ中なら, 表示する季節に応じて変化
  if( work->dispSeason == work->endSeason ) {
    return MAPCHANGE_WAIT_FRAME_LONG; // 現在の季節は長め
  }
  else {
    return MAPCHANGE_WAIT_FRAME_SHORT; // 移ろう季節は短め 
  }
}
