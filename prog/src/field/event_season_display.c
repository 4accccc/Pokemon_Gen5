//////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  季節表示イベント
 * @file   event_season_display.c
 * @author obata
 * @date   2009.11.25
 */
//////////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "fieldmap.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "event_season_display.h"

#include "gamesystem/pm_season.h"  // for PMSEASON_xxxx
#include "arc/arc_def.h"  // for ARCID_SEASON_DISPLAY
#include "arc/season_display.naix"  // for datid
#include "field_status_local.h"  // for FIELD_STATUS


extern void FIELDMAP_InitBGMode( void );
extern void FIELDMAP_InitBG( FIELDMAP_WORK* fieldWork );


//========================================================================================
// ■BG
//========================================================================================
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

// BGコントロール
static const GFL_BG_BGCNT_HEADER bg_header = 
{
  0, 0,					          // 初期表示位置
  0,						          // スクリーンバッファサイズ
  0,							        // スクリーンバッファオフセット
  GFL_BG_SCRSIZ_256x256,	// スクリーンサイズ
  GX_BG_COLORMODE_16,			// カラーモード
  GX_BG_SCRBASE_0x1800,		// スクリーンベースブロック
  GX_BG_CHARBASE_0x08000,	// キャラクタベースブロック
  GFL_BG_CHRSIZ_256x256,	// キャラクタエリアサイズ
  GX_BG_EXTPLTT_01,			  // BG拡張パレットスロット選択
  BG_PRIORITY_SEASON,		        // 表示プライオリティー
  0,							        // エリアオーバーフラグ
  0,							        // DUMMY
  FALSE,						      // モザイク設定
}; 


//========================================================================================
// ■定数
//========================================================================================
// 季節スキップ
#define FADEIN_FRAME_SHORT  (10)  // フェードイン時間
#define WAIT_FRAME_SHORT    (30)  // 待機時間
#define FADEOUT_FRAME_SHORT (10)  // フェードアウト時間
// 現季節表示
#define FADEIN_FRAME_LONG  (10)  // フェードイン時間
#define WAIT_FRAME_LONG    (60)  // 待機時間
#define FADEOUT_FRAME_LONG (20)  // フェードアウト時間

// イベントシーケンス
enum
{
  SEQ_INIT,    // 初期化
  SEQ_PREPARE, // 表示準備
  SEQ_FADEIN,  // フェードイン
  SEQ_WAIT,    // 待機
  SEQ_FADEOUT, // フェードアウト
  SEQ_EXIT,    // 終了
};


//========================================================================================
// ■イベントワーク
//========================================================================================
typedef struct
{
  GAMESYS_WORK* gsys;
  FIELDMAP_WORK* fieldmap;

  u8 currentSeason;  // 現在の季節
  u8 dispSeason;     // 表示中の季節
  u32 count;         // カウンタ
  u32 maxCount;      // カウンタ最大値

} EVENT_WORK;


//========================================================================================
// ■非公開関数のプロトタイプ宣言
//========================================================================================
static void InitBG( EVENT_WORK* work );
static void LoadGraphicData( u8 season, HEAPID heap_id );
static void UpdateAlpha( const EVENT_WORK* work, int seq );
static int GetNextSeq( const EVENT_WORK* work, int seq );
static void SetSeq( EVENT_WORK* work, int* seq, int next );
static void StepNextSeq( EVENT_WORK* work, int* seq );
static void Exit( EVENT_WORK* work );


//========================================================================================
// ■イベント処理関数
//========================================================================================
static GMEVENT_RESULT SeasonDisplay( GMEVENT* event, int* seq, void* wk )
{
  EVENT_WORK* work = (EVENT_WORK*)wk;

  // シーケンス固有処理
  switch( *seq )
  {
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
    Exit( work );
    return GMEVENT_RES_FINISH;
  }

  // 共通処理
  work->count++;
  StepNextSeq( work, seq );
  return GMEVENT_RES_CONTINUE;
}


//========================================================================================
// ■外部公開関数
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief 四季表示イベントを作成する
 *
 * @param gsys     ゲームシステム
 * @param fieldmap フィールドマップ
 * @param start    開始季節
 * @param end      最終季節 
 *
 * @return 作成したイベント
 */
//----------------------------------------------------------------------------------------
GMEVENT* EVENT_SeasonDisplay( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap,
                              u8 start, u8 end )
{
  GMEVENT* event;
  EVENT_WORK* work;
  int* seq;

  // イベント生成
  event = GMEVENT_Create( gsys, NULL, SeasonDisplay, sizeof(EVENT_WORK) ); 
  // イベントワーク初期化
  seq                 = GMEVENT_GetSequenceWork( event );
  work                = GMEVENT_GetEventWork( event );
  work->gsys          = gsys;
  work->fieldmap      = fieldmap;
  work->currentSeason = end;
  work->dispSeason    = PMSEASON_GetPrevSeasonID( start );
  SetSeq( work, seq, SEQ_INIT );
  return event;
}

//----------------------------------------------------------------------------------------
/**
 * @brief 季節表示イベントに要するフレーム数を取得する
 *
 * @param gsys ゲームシステム
 *
 * @return イベント起動から終了までにかかるフレーム数
 */
//----------------------------------------------------------------------------------------
u32 GetSeasonDispEventFrame( GAMESYS_WORK* gsys )
{
  GAMEDATA* gdata;
  FIELD_STATUS* fstatus;
  u32 frame;
  u8 last, now, num;

  // 表示する季節の数を求める
  gdata   = GAMESYSTEM_GetGameData( gsys );
  fstatus = GAMEDATA_GetFieldStatus( gdata );
  last    = FIELD_STATUS_GetSeasonDispLast( fstatus );
  now     = GAMEDATA_GetSeasonID( gdata );
  num     = (now - last + PMSEASON_TOTAL) % PMSEASON_TOTAL;

  // フレーム数を計算
  frame  = 0;
  frame += 2;  // INIT, EXIT の分
  frame += (num - 1) *   // スキップ表示の分 (PREPARE, FADEIN, WAIT, FADEOUT)
           (1 + FADEIN_FRAME_SHORT + WAIT_FRAME_SHORT + FADEOUT_FRAME_SHORT);
  frame += (1 + FADEIN_FRAME_LONG + WAIT_FRAME_LONG + FADEOUT_FRAME_LONG); // 現季節表示の分
  return frame;
}


//========================================================================================
// ■非公開関数のプロトタイプ宣言
//========================================================================================

//----------------------------------------------------------------------------------------
/**
 * @brief BGに関する初期設定を行う
 *
 * @param work イベントワーク
 */
//----------------------------------------------------------------------------------------
static void InitBG( EVENT_WORK* work )
{ 
  // 初期設定
  //GFL_BG_SetBGControl( BG_FRAME_SEASON, &bg_header, GFL_BG_MODE_TEXT );
  //G2_SetBlendAlpha( ALPHA_MASK_SEASON, ALPHA_MASK_FIELD, ALPHA_MIN, ALPHA_MAX ); 
  // プライオリティを設定
  GFL_BG_SetPriority( BG_FRAME_FIELD,  BG_PRIORITY_FIELD );
  GFL_BG_SetPriority( BG_FRAME_SEASON, BG_PRIORITY_SEASON ); 
  GFL_BG_SetPriority( GFL_BG_FRAME1_M, BG_PRIORITY_OTHER ); 
  GFL_BG_SetPriority( GFL_BG_FRAME2_M, BG_PRIORITY_OTHER ); 
  // キャラVRAM・スクリーンVRAMをクリア
  GFL_BG_ClearFrame( BG_FRAME_SEASON ); 
}

//----------------------------------------------------------------------------------------
/**
 * @brief 指定した季節の表示に必要なグラフィックデータを読み込む
 *
 * @param season  読み込みを行う季節
 * @param haep_id 使用するヒープID
 */
//----------------------------------------------------------------------------------------
static void LoadGraphicData( u8 season, HEAPID heap_id )
{
  ARCDATID datid_pltt, datid_char, datid_scrn;

  // データIDを決定
  switch( season )
  {
  default:
    OBATA_Printf( "====================================================\n" );
    OBATA_Printf( "EVENT-SEASON-DISPLAY: 範囲外の季節が指定されました。\n" );
    OBATA_Printf( "====================================================\n" );
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

//----------------------------------------------------------------------------------------
/**
 * @brief αブレンディングを更新する
 *
 * @param work イベントワーク
 * @param seq  現在のシーケンス
 */
//----------------------------------------------------------------------------------------
static void UpdateAlpha( const EVENT_WORK* work, int seq )
{
  if( seq == SEQ_FADEIN )
  { // フェードイン
    int bright;
    float rate;
    rate = (float)work->count / (float)work->maxCount;
    bright = BRIGHT_MIN + (BRIGHT_MAX - BRIGHT_MIN) * rate;
    G2_SetBlendBrightness( ALPHA_MASK_SEASON, bright );
  }
  else if( seq == SEQ_FADEOUT )
  { // フェードアウト
    if( work->dispSeason == work->currentSeason )  // if(最後のフェードアウト)
    { 
      // 最後の季節表示はαブレンド
      {
        int alpha1, alpha2;
        float rate;
        rate = (float)work->count / (float)work->maxCount;
        alpha2 = ALPHA_MAX * rate;
        alpha1 = ALPHA_MAX - alpha2;
        G2_SetBlendAlpha( ALPHA_MASK_SEASON, ALPHA_MASK_FIELD, alpha1, alpha2 );
      }
      // 最後の季節表示フェードアウト時は下画面を輝度フェードインさせる
      {
        int bright;
        float rate;
        rate = (float)work->count / (float)work->maxCount;
        bright = BRIGHT_MIN + (BRIGHT_MAX - BRIGHT_MIN) * rate;
        GXS_SetMasterBrightness( bright );
      }
    }
    else
    { // スキップ時は輝度
      int bright;
      float rate;
      rate = (float)work->count / (float)work->maxCount;
      bright = BRIGHT_MAX - (BRIGHT_MAX - BRIGHT_MIN) * rate;
      G2_SetBlendBrightness( ALPHA_MASK_SEASON, bright );
    }
  }
}

//----------------------------------------------------------------------------------------
/**
 * @brief 次の処理シーケンスを取得する
 *
 * @param work イベントワーク
 * @param seq  現在のシーケンス
 *
 * @return 次のシーケンス番号( 変化がなければ現在と同じシーケンス番号 )
 */
//----------------------------------------------------------------------------------------
static int GetNextSeq( const EVENT_WORK* work, int seq )
{
  int next = seq;

  switch( seq )
  {
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
    if( work->maxCount < work->count ){ next = SEQ_WAIT; }
    break;
  // 待機
  case SEQ_WAIT:
    if( work->maxCount < work->count ){ next = SEQ_FADEOUT; }
    break;
  // フェードアウト
  case SEQ_FADEOUT:
    if( work->maxCount < work->count )
    {
      // 現在の季節を表示し終えたら, イベント終了
      if( work->currentSeason == work->dispSeason ){ next = SEQ_EXIT; }
      else{ next = SEQ_PREPARE; }
    }
    break;
  // 終了
  case SEQ_EXIT:
    break;
  } 

  return next;
}

//----------------------------------------------------------------------------------------
/**
 * @brief 処理シーケンスの設定と, それに伴う初期化を行う
 *
 * @param work イベントワーク
 * @param seq  シーケンスワークへのポインタ
 * @param next 次のシーケンス
 */
//----------------------------------------------------------------------------------------
static void SetSeq( EVENT_WORK* work, int* seq, int next )
{
  switch( next )
  {
  // 初期化
  case SEQ_INIT:
    // BGモード設定と表示設定の復帰
    {
      int mv = GFL_DISP_GetMainVisible();
      FIELDMAP_InitBGMode();  // この中で表示設定がクリアされる
      GFL_DISP_GX_SetVisibleControlDirect( mv );
    }
    // BG初期設定
    InitBG( work );
    break;
  // 表示準備
  case SEQ_PREPARE:
    // 表示する季節を決定
    work->dispSeason = PMSEASON_GetNextSeasonID( work->dispSeason );
    // グラフィックデータ読み込み
    {
      HEAPID heap_id = FIELDMAP_GetHeapID( work->fieldmap );
      LoadGraphicData( work->dispSeason, heap_id );
    }
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
    if( work->dispSeason == work->currentSeason ){ work->maxCount = FADEIN_FRAME_LONG; }
    else{ work->maxCount = FADEIN_FRAME_SHORT; }
    break;
  // 待機
  case SEQ_WAIT:
    // 待機時間を決定
    if( work->dispSeason == work->currentSeason ){ work->maxCount = WAIT_FRAME_LONG; }
    else{ work->maxCount = WAIT_FRAME_SHORT; }
    break;
  // フェードアウト
  case SEQ_FADEOUT:
    // フェード時間を決定
    if( work->dispSeason == work->currentSeason ){ work->maxCount = FADEOUT_FRAME_LONG; }
    else{ work->maxCount = FADEOUT_FRAME_SHORT; }
    // 最後の表示をフェードアウトさせる時は, 裏にフィールドを表示する
    if( work->dispSeason == work->currentSeason )
    {
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
  switch( next )
  {
  case SEQ_INIT:    OBATA_Printf( "INIT\n" );    break;
  case SEQ_PREPARE: OBATA_Printf( "PREPARE\n" ); break;
  case SEQ_FADEIN:  OBATA_Printf( "FADEIN\n" );  break;
  case SEQ_WAIT:    OBATA_Printf( "WAIT\n" );    break;
  case SEQ_FADEOUT: OBATA_Printf( "FADEOUT\n" ); break;
  case SEQ_EXIT:    OBATA_Printf( "EXIT\n" );    break;
  }
}

//----------------------------------------------------------------------------------------
/**
 * @brief 次のシーケンスに移行する
 *
 * @param work イベントワーク
 * @param seq  シーケンスワークへのポインタ
 */
//----------------------------------------------------------------------------------------
static void StepNextSeq( EVENT_WORK* work, int* seq )
{
  int next;
  
  // 次のシーケンスを取得
  next = GetNextSeq( work, *seq );

  // 変化したら遷移する
  if( *seq != next )
  {
    SetSeq( work, seq, next );
  }
}

//----------------------------------------------------------------------------------------
/**
 * @brief イベント終了処理
 *
 * @param work イベントワーク
 */
//----------------------------------------------------------------------------------------
static void Exit( EVENT_WORK* work )
{
  // BGを非表示に設定
  GFL_BG_SetVisible( BG_FRAME_SEASON, VISIBLE_OFF ); 
}
