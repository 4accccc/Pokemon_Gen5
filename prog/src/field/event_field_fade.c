//============================================================================================
/**
 * @file	event_field_fade.c
 * @brief	イベント：フィールドフェード制御ツール
 * @author	tamada GAMEFREAK inc.
 * @date	2008.12.10
 *
 * 2009.12.22 tamada event_fieldmap_controlから分離
 */
//============================================================================================

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "gamesystem/pm_season.h"  // for PMSEASON_xxxx 
#include "field/zonedata.h"        // for ZONEDATA_xxxx
#include "field/fieldmap.h" 
#include "system/screentex.h"
#include "system/wipe.h"

#include "vblank_bg_scale_expand.h"
#include "event_field_fade.h"
#include "event_season_display.h"  // for EVENT_SeasonDisplay
#include "map_change_type.h"  // for MC_TYPE_xxxx

#ifdef PM_DEBUG
BOOL DebugBGInitEnd = FALSE;    //BG初期化監視フラグ
#endif 

#define BG_FRAME_CROSS_FADE ( FLDBG_MFRM_EFF1 ) 


//============================================================================================
// ■非公開関数
//============================================================================================
static int GetBrightFadeMode( FIELD_FADE_TYPE fadeType );
static FIELD_FADE_TYPE GetFadeOutType_byMapChangeType( MC_TYPE prevMapChangeType, MC_TYPE nextMapChangeType );
static FIELD_FADE_TYPE GetFadeInType_byMapChangeType( MC_TYPE prevMapChangeType, MC_TYPE nextMapChangeType );

//--------------------------------------------------------------------------------------------
// □イベント生成関数
//--------------------------------------------------------------------------------------------
static GMEVENT* EVENT_BrightOut( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap,                   // 輝度フェードアウト 
                                 FIELD_FADE_TYPE fadeType, FIELD_FADE_WAIT_FLAG fadeFinishWaitFlag ); //
static GMEVENT* EVENT_BrightIn( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap,                    //
                                 FIELD_FADE_TYPE fadeType,                 // 輝度フェードイン
                                 FIELD_FADE_WAIT_FLAG fadeFinishWaitFlag,  // 
                                 FIELD_FADE_BG_INIT_FLAG BGInitFlag );     //
static GMEVENT* EVENT_CrossOut( GAMESYS_WORK *gameSystem, FIELDMAP_WORK * fieldmap ); // クロスフェードアウト
static GMEVENT* EVENT_CrossIn( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap ); // クロスフェードイン
static GMEVENT* EVENT_SeasonIn( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, u8 startSeason, u8 endSeason ); // 季節フェードイン
static GMEVENT* EVENT_WipeOut( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap ); // ワイプアウト

//--------------------------------------------------------------------------------------------
// □イベント処理関数
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT BrightOutEvent( GMEVENT* event, int* seq, void* wk ); // 輝度フェードアウト
static GMEVENT_RESULT BrightInEvent( GMEVENT* event, int* seq, void* wk );  // 輝度フェードイン
static GMEVENT_RESULT CrossOutEvent( GMEVENT* event, int* seq, void* wk ); // クロスフェードアウト
static GMEVENT_RESULT CrossInEvent( GMEVENT* event, int* seq, void* wk );  // クロスフェードイン
static GMEVENT_RESULT SeasonInEvent( GMEVENT* event, int* seq, void* wk ); // 季節フェードイン
static GMEVENT_RESULT WipeOutEvent( GMEVENT* event, int* seq, void* wk ); // ワイプアウト
static GMEVENT_RESULT QuickOutEvent( GMEVENT* event, int* seq, void* wk ); // 即時フェードアウト
static GMEVENT_RESULT QuickInEvent( GMEVENT* event, int* seq, void* wk );  // 即時フェードイン
static GMEVENT_RESULT FlySkyBrightInEvent( GMEVENT* event, int* seq, void* wk ); // 空を飛ぶ用 輝度フェードイン


//============================================================================================
// ■フェードイベントワーク
//============================================================================================ 
typedef struct 
{
  GAMESYS_WORK*      gameSystem;
  FIELDMAP_WORK*     fieldmap;

	FIELD_FADE_TYPE fadeType;  // フェードの種類 ( クロスフェード, 輝度フェードなど )

  // 輝度フェード
  int                     brightFadeMode;      // フェードモード
  FIELD_FADE_WAIT_FLAG    fadeFinishWaitFlag;  // フェード処理の完了を待つかどうか
  FIELD_FADE_BG_INIT_FLAG BGInitFlag;          // BG面の初期化を行うかどうか

  // クロスフェード
	int alphaWork;

  // 季節フェード
  u8 startSeason;  // 最初に表示する季節
  u8 endSeason;    // 最後に表示する季節

} FADE_EVENT_WORK;



//============================================================================================
// ■フェード判定関数
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief 前後のゾーンIDから, フェードアウトの種類を決定する
 *
 * @param prevZoneID 遷移後のゾーンID
 * @param nextZoneID 遷移前のゾーンID
 *
 * @return 指定したゾーン間を遷移する際のフェードアウトの種類
 */
//--------------------------------------------------------------------------------------------
extern FIELD_FADE_TYPE FIELD_FADE_GetFadeOutType( u16 prevZoneID, u16 nextZoneID )
{
  MC_TYPE prevMapChangeType;
  MC_TYPE nextMapChangeType;

  // 前後のゾーンのマップ切り替えタイプを取得
  prevMapChangeType = ZONEDATA_GetMapChangeType( prevZoneID );
  nextMapChangeType = ZONEDATA_GetMapChangeType( nextZoneID );

  return GetFadeOutType_byMapChangeType( prevMapChangeType, nextMapChangeType );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 前後のゾーンIDから, フェードインの種類を決定する
 *
 * @param prevZoneID 遷移後のゾーンID
 * @param nextZoneID 遷移前のゾーンID
 *
 * @return 指定したゾーン間を遷移する際のフェードインの種類
 */
//--------------------------------------------------------------------------------------------
extern FIELD_FADE_TYPE FIELD_FADE_GetFadeInType( u16 prevZoneID, u16 nextZoneID )
{
  MC_TYPE prevMapChangeType;
  MC_TYPE nextMapChangeType;

  // 前後のゾーンのマップ切り替えタイプを取得
  prevMapChangeType = ZONEDATA_GetMapChangeType( prevZoneID );
  nextMapChangeType = ZONEDATA_GetMapChangeType( nextZoneID );

  return GetFadeInType_byMapChangeType( prevMapChangeType, nextMapChangeType );
}


//============================================================================================
// ■基本フェード関数
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief	フェードアウトイベント生成
 *
 * @param	gameSystem
 * @param	fieldmap
 * @param	fadeType		       フェードの種類指定
 * @param fadeFinishWaitFlag フェード完了を待つかどうか(輝度フェード時のみ有効)
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
GMEVENT* EVENT_FieldFadeOut( GAMESYS_WORK *gameSystem, FIELDMAP_WORK * fieldmap, 
                             FIELD_FADE_TYPE fadeType, 
                             FIELD_FADE_WAIT_FLAG fadeFinishWaitFlag ) 
{ 
  GMEVENT* event;

  // フェードイベント生成
  switch( fadeType )
  {
  default:
  case FIELD_FADE_CROSS: // クロスフェード
    event = EVENT_CrossOut( gameSystem, fieldmap );
    break;
  case FIELD_FADE_BLACK:  // 輝度フェード(ブラック)
  case FIELD_FADE_WHITE:  // 輝度フェード(ホワイト)
  case FIELD_FADE_SEASON: // 季節フェード
    event = EVENT_BrightOut( gameSystem, fieldmap, fadeType, fadeFinishWaitFlag );
    break;
  case FIELD_FADE_WIPE: // ワイプアウト
    event = EVENT_WipeOut( gameSystem, fieldmap );
    break;
  }
	return event;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	フェードイン イベント生成
 *
 * @param	gameSystem
 * @param	fieldmap 
 * @param	fadeType		       フェードの種類指定
 * @param fadeFinishWaitFlag フェード完了を待つかどうか ( 輝度フェード時のみ有効 )
 * @param BGInitFlag         BG初期化を内部でおこなうかどうか ( 輝度フェード時のみ有効 )
 * @param startSeason        最初に表示する季節 ( 季節フェード時のみ有効 )
 * @param endSeason          最後に表示する季節 ( 季節フェード時のみ有効 )
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
GMEVENT* EVENT_FieldFadeIn( GAMESYS_WORK *gameSystem, FIELDMAP_WORK * fieldmap, 
                            FIELD_FADE_TYPE fadeType, 
                            FIELD_FADE_WAIT_FLAG fadeFinishWaitFlag, 
                            FIELD_FADE_BG_INIT_FLAG BGInitFlag,
                            u8 startSeason, u8 endSeason )
{
  GMEVENT* event;

  switch( fadeType )
  {
  default:
  case FIELD_FADE_CROSS: // クロスフェード
    event = EVENT_CrossIn( gameSystem, fieldmap );
    break;
  case FIELD_FADE_BLACK: // 輝度フェード(ブラック)
  case FIELD_FADE_WHITE: // 輝度フェード(ホワイト)
    event = EVENT_BrightIn( gameSystem, fieldmap, fadeType, fadeFinishWaitFlag, BGInitFlag );
    break;
  case FIELD_FADE_SEASON:
    event = EVENT_SeasonIn( gameSystem, fieldmap, startSeason, endSeason );
    break;
  }
	return event; 
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	空を飛ぶ 輝度フェードイン イベント生成
 *
 * @param	gameSystem
 * @param	fieldmap
 * @param	fadeType		       フェードの種類指定 ( FADE_TYPE_BLACK or FADE_TYPE_WHITE )
 * @param fadeFinishWaitFlag フェード完了を待つかどうか
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
GMEVENT* EVENT_FlySkyBrightIn( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, 
                               FIELD_FADE_TYPE fadeType, FIELD_FADE_WAIT_FLAG fadeFinishWaitFlag )
{
	GMEVENT * event;
	FADE_EVENT_WORK * work;

	event = GMEVENT_Create( gameSystem, NULL, FlySkyBrightInEvent, sizeof(FADE_EVENT_WORK) );
	work  = GMEVENT_GetEventWork(event);

  // イベントワーク初期化
  work->gameSystem         = gameSystem;
  work->fieldmap           = fieldmap;
	work->fadeType           = fadeType;
  work->fadeFinishWaitFlag = fadeFinishWaitFlag;
  work->brightFadeMode     = GetBrightFadeMode( fadeType );

	return event;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	デバッグ用 即時アウト イベント生成
 *
 * @param	gameSystem
 * @param	fieldmap
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
GMEVENT* DEBUG_EVENT_QuickFadeOut( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap )
{
	GMEVENT* event;
	FADE_EVENT_WORK* work;

  event = GMEVENT_Create( gameSystem, NULL, QuickOutEvent, sizeof(FADE_EVENT_WORK) );
	work  = GMEVENT_GetEventWork( event );

  // イベントワーク初期化
  work->gameSystem = gameSystem;
  work->fieldmap   = fieldmap;

  return event;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	空を飛ぶ 輝度フェードアウト イベント生成
 *
 * @param	gameSystem
 * @param	fieldmap
 * @param	fadeType	         フェードの種類指定 ( FIELD_FADE_BLACK or FIELD_FADE_WHITE )
 * @param fadeFinishWaitFlag フェード完了を待つかどうか
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
GMEVENT* EVENT_FlySkyBrightOut( GAMESYS_WORK *gameSystem, FIELDMAP_WORK * fieldmap, 
                                FIELD_FADE_TYPE fadeType, FIELD_FADE_WAIT_FLAG fadeFinishWaitFlag )
{
	GMEVENT* event;
	FADE_EVENT_WORK* work;

	event = GMEVENT_Create( gameSystem, NULL, BrightOutEvent, sizeof(FADE_EVENT_WORK) );
	work  = GMEVENT_GetEventWork(event);

  // イベントワーク初期化
  work->gameSystem         = gameSystem;
  work->fieldmap           = fieldmap;
	work->fadeType           = fadeType;
  work->fadeFinishWaitFlag = fadeFinishWaitFlag;
  work->brightFadeMode     = GetBrightFadeMode( fadeType );

	return event;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	デバッグ用 即時アウト
 *
 * @param	gameSystem
 * @param	fieldmap
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
GMEVENT* DEBUG_EVENT_QuickFadeIn( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap )
{
	GMEVENT* event;
  FADE_EVENT_WORK* work;

  event = GMEVENT_Create( gameSystem, NULL, QuickInEvent, sizeof(FADE_EVENT_WORK) );
	work  = GMEVENT_GetEventWork(event);

  // イベントワーク初期化
  work->gameSystem = gameSystem;
  work->fieldmap   = fieldmap;

  return event;
} 


//============================================================================================
//============================================================================================ 

//-------------------------------------------------------------------------------------------
/**
 * @brief 輝度フェードモードを取得する
 *
 * @param fadeType フェードタイプ ( クロスフェード, 輝度フェード など )
 *
 * @return 指定フェードタイプに応じた輝度フェードモード
 */
//-------------------------------------------------------------------------------------------
static int GetBrightFadeMode( FIELD_FADE_TYPE fadeType )
{
  int brightFadeMode;

  switch( fadeType )
  {
  default:
  case FIELD_FADE_BLACK: // 輝度フェード(ブラック)
    brightFadeMode = GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB;
    break;
  case FIELD_FADE_WHITE: // 輝度フェード(ホワイト)
    brightFadeMode = GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN | GFL_FADE_MASTER_BRIGHT_WHITEOUT_SUB;
    break;
  }

  return brightFadeMode;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief マップ切り替えタイプから, フェードアウトの種類を決定する
 *
 * @param prevMapChangeType 遷移前ゾーンのマップ切り替えタイプ
 * @param nextMapChangeType 遷移後ゾーンのマップ切り替えタイプ
 *
 * @return 指定したゾーン間を遷移する際のフェードアウトの種類
 */
//--------------------------------------------------------------------------------------------
static FIELD_FADE_TYPE GetFadeOutType_byMapChangeType( MC_TYPE prevMapChangeType, MC_TYPE nextMapChangeType )
{
  // フェードタイプ [ 前MC_TYPE ][ 後MC_TYPE ]
  const FIELD_FADE_TYPE fadeType[ MC_TYPE_NUM ][MC_TYPE_NUM ] =
  {
                /* FIELD             ROOM              GATE              DUNGEON           BRIDGE           */
    /* FIELD   */{ FIELD_FADE_CROSS, FIELD_FADE_BLACK, FIELD_FADE_CROSS, FIELD_FADE_WIPE , FIELD_FADE_CROSS },
    /* ROOM    */{ FIELD_FADE_BLACK, FIELD_FADE_BLACK, FIELD_FADE_BLACK, FIELD_FADE_WIPE , FIELD_FADE_BLACK }, 
    /* GATE    */{ FIELD_FADE_CROSS, FIELD_FADE_BLACK, FIELD_FADE_CROSS, FIELD_FADE_BLACK, FIELD_FADE_CROSS },
    /* DUNGEON */{ FIELD_FADE_WHITE, FIELD_FADE_WHITE, FIELD_FADE_WHITE, FIELD_FADE_BLACK, FIELD_FADE_WHITE },
    /* BRIDGE  */{ FIELD_FADE_CROSS, FIELD_FADE_BLACK, FIELD_FADE_CROSS, FIELD_FADE_BLACK, FIELD_FADE_CROSS },
  };

  return fadeType[ prevMapChangeType ][ nextMapChangeType ];
}

//--------------------------------------------------------------------------------------------
/**
 * @brief マップ切り替えタイプから, フェードインの種類を決定する
 *
 * @param prevMapChangeType 遷移前ゾーンのマップ切り替えタイプ
 * @param nextMapChangeType 遷移後ゾーンのマップ切り替えタイプ
 *
 * @return 指定したゾーン間を遷移する際のフェードインの種類
 */
//--------------------------------------------------------------------------------------------
static FIELD_FADE_TYPE GetFadeInType_byMapChangeType( MC_TYPE prevMapChangeType, MC_TYPE nextMapChangeType )
{
  // フェードタイプ [ 前MC_TYPE ][ 後MC_TYPE ]
  const FIELD_FADE_TYPE fadeType[ MC_TYPE_NUM ][ MC_TYPE_NUM ] =
  {
                /* FIELD             ROOM              GATE              DUNGEON           BRIDGE           */
    /* FIELD   */{ FIELD_FADE_CROSS, FIELD_FADE_BLACK, FIELD_FADE_CROSS, FIELD_FADE_BLACK, FIELD_FADE_CROSS },
    /* ROOM    */{ FIELD_FADE_BLACK, FIELD_FADE_BLACK, FIELD_FADE_BLACK, FIELD_FADE_BLACK, FIELD_FADE_BLACK },
    /* GATE    */{ FIELD_FADE_CROSS, FIELD_FADE_BLACK, FIELD_FADE_CROSS, FIELD_FADE_BLACK, FIELD_FADE_CROSS },
    /* DUNGEON */{ FIELD_FADE_WHITE, FIELD_FADE_WHITE, FIELD_FADE_WHITE, FIELD_FADE_BLACK, FIELD_FADE_WHITE },
    /* BRIDGE  */{ FIELD_FADE_CROSS, FIELD_FADE_BLACK, FIELD_FADE_CROSS, FIELD_FADE_BLACK, FIELD_FADE_CROSS },
  };

  return fadeType[ prevMapChangeType ][ nextMapChangeType ];
}


//============================================================================================
// □イベント生成関数
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief	輝度フェードアウト イベント生成
 *
 * @param	gameSystem
 * @param	fieldmap
 * @param	fadeType		        フェードの種類指定 ( FIELD_FADE_BLACK or FIELD_FADE_WHITE )
 * @param fadeFinishWaitFlag  フェード完了を待つかどうか
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT* EVENT_BrightOut( GAMESYS_WORK* gameSystem, 
                                      FIELDMAP_WORK* fieldmap, 
                                      FIELD_FADE_TYPE fadeType, 
                                      FIELD_FADE_WAIT_FLAG fadeFinishWaitFlag )
{
	GMEVENT* event;
	FADE_EVENT_WORK* work;

	event = GMEVENT_Create( gameSystem, NULL, BrightOutEvent, sizeof(FADE_EVENT_WORK) );
	work  = GMEVENT_GetEventWork(event);

  // イベントワーク初期化
  work->gameSystem         = gameSystem;
  work->fieldmap           = fieldmap;
  work->fadeType           = fadeType;
  work->fadeFinishWaitFlag = fadeFinishWaitFlag;
  work->brightFadeMode     = GetBrightFadeMode( fadeType );

	return event;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	輝度フェードイン イベント生成
 *
 * @param	gameSystem
 * @param	fieldmap
 * @param	fadeType		       フェードの種類指定 ( FADE_TYPE_BLACK or FADE_TYPE_WHITE )
 * @param fadeFinishWaitFlag フェード完了を待つかどうか
 * @param BGInitFlag         BG初期化を内部でおこなうかどうか
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT* EVENT_BrightIn( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, 
                                     FIELD_FADE_TYPE fadeType, 
                                     FIELD_FADE_WAIT_FLAG fadeFinishWaitFlag,
                                     FIELD_FADE_BG_INIT_FLAG BGInitFlag )
{
	GMEVENT* event;
	FADE_EVENT_WORK* work;

	event = GMEVENT_Create( gameSystem, NULL, BrightInEvent, sizeof(FADE_EVENT_WORK) );
	work  = GMEVENT_GetEventWork(event);

  // イベントワーク初期化
  work->gameSystem          = gameSystem;
  work->fieldmap            = fieldmap;
	work->fadeType            = fadeType;
  work->fadeFinishWaitFlag  = fadeFinishWaitFlag;
  work->brightFadeMode      = GetBrightFadeMode( fadeType );
  work->BGInitFlag          = BGInitFlag;

	return event;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	クロスフェードアウト イベント生成
 *
 * @param	gameSystem
 * @param	fieldmap
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT* EVENT_CrossOut( GAMESYS_WORK *gameSystem, FIELDMAP_WORK * fieldmap )
{
	GMEVENT* event;
	FADE_EVENT_WORK* work;

	event = GMEVENT_Create( gameSystem, NULL, CrossOutEvent, sizeof(FADE_EVENT_WORK) );
	work  = GMEVENT_GetEventWork(event);

  // イベントワーク初期化
  work->gameSystem = gameSystem;
  work->fieldmap   = fieldmap;
  work->fadeType   = FIELD_FADE_CROSS;

	return event;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	クロスフェードイン イベント生成
 * @param	gameSystem
 * @param	fieldmap 
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT* EVENT_CrossIn( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap )
{
	GMEVENT* event;
	FADE_EVENT_WORK* work;

	event = GMEVENT_Create( gameSystem, NULL, CrossInEvent, sizeof(FADE_EVENT_WORK) );
	work  = GMEVENT_GetEventWork(event);

  // イベントワーク初期化
  work->gameSystem = gameSystem;
  work->fieldmap   = fieldmap;
  work->fadeType   = FIELD_FADE_CROSS;

	return event;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	季節フェードイン イベント生成
 *
 * @param	gameSystem
 * @param	fieldmap
 * @param startSeason 表示する最初の季節
 * @param endSeason   表示する最後の季節
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT* EVENT_SeasonIn( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap, 
                                     u8 startSeason, u8 endSeason )
{
	GMEVENT* event;
	FADE_EVENT_WORK* work;

	event = GMEVENT_Create( gameSystem, NULL, SeasonInEvent, sizeof(FADE_EVENT_WORK) );
	work  = GMEVENT_GetEventWork(event);

  // イベントワーク初期化
  work->gameSystem  = gameSystem;
  work->fieldmap    = fieldmap;
	work->fadeType    = FIELD_FADE_SEASON;
  work->startSeason = startSeason;
  work->endSeason   = endSeason;

	return event;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief ワイプアウト イベント生成
 *
 * @param gameSystem
 * @param fieldmap
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT* EVENT_WipeOut( GAMESYS_WORK* gameSystem, FIELDMAP_WORK* fieldmap )
{
	GMEVENT* event;
	FADE_EVENT_WORK* work;

	event = GMEVENT_Create( gameSystem, NULL, WipeOutEvent, sizeof(FADE_EVENT_WORK) );
	work  = GMEVENT_GetEventWork(event);

  // イベントワーク初期化
  work->gameSystem  = gameSystem;
  work->fieldmap    = fieldmap;
	work->fadeType    = FIELD_FADE_WIPE;

	return event;
}


//============================================================================================
// ■イベント処理関数
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief 輝度フェードアウト イベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT BrightOutEvent( GMEVENT* event, int* seq, void* wk )
{
	FADE_EVENT_WORK* work = wk;

	switch( *seq )
  {
	case 0:
    GFL_FADE_SetMasterBrightReq( work->brightFadeMode, 0, 16, 0 );
		(*seq)++;
		break;
	case 1:
    if( work->fadeFinishWaitFlag == FIELD_FADE_NO_WAIT ){ return GMEVENT_RES_FINISH; } // フェード待たない
		if( GFL_FADE_CheckFade() == FALSE ){ return GMEVENT_RES_FINISH; }  // フェード完了
		break;
	}
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 輝度フェードイン イベント
 */ 
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT BrightInEvent( GMEVENT* event, int* seq, void* wk )
{
	FADE_EVENT_WORK* work = wk;

	switch( *seq )
  {
  case 0:
    // BGモード設定と表示設定の復帰
    if ( work->BGInitFlag )
    {
      int mv = GFL_DISP_GetMainVisible();
      FIELDMAP_InitBGMode();
      GFL_DISP_GX_SetVisibleControlDirect( mv );
      FIELDMAP_InitBG(work->fieldmap);
    }
    GFL_FADE_SetMasterBrightReq(work->brightFadeMode, 16, 0, 0);
		(*seq)++;
		break;
	case 1:
    if( work->fadeFinishWaitFlag == FIELD_FADE_NO_WAIT ){ return GMEVENT_RES_FINISH; }  // フェード待たない
    if( GFL_FADE_CheckFade() == FALSE ){ return GMEVENT_RES_FINISH; }  // フェード完了
		break;
	} 
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief クロスフェードアウト イベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT CrossOutEvent( GMEVENT* event, int* seq, void* wk )
{
	FADE_EVENT_WORK* work = wk;

	switch( *seq )
  {
	case 0:
		// サブ画面だけブラックアウト
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, -8);

		// 画面キャプチャ→VRAM_D
		GX_SetBankForLCDC(GX_VRAM_LCDC_D);
		GX_SetCapture(GX_CAPTURE_SIZE_256x192,				// Capture size
				          GX_CAPTURE_MODE_A,							// Capture mode
				          GX_CAPTURE_SRCA_2D3D,						// Blend src A
				          GX_CAPTURE_SRCB_VRAM_0x00000,		// Blend src B
									GX_CAPTURE_DEST_VRAM_D_0x00000,	// dst
								  16, 0);             // Blend parameter for src A, B

		OS_WaitVBlankIntr();	// 0ライン待ちウエイト
		OS_WaitVBlankIntr();	// キャプチャー待ちウエイト
#if 0
		(*seq)++;
		break;

	case 1:	// 0ライン待ちウエイト
	case 2:	// キャプチャー待ちウエイト
		(*seq)++;
		break;

	case 3:
#endif
		//描画モード変更
		{
			const GFL_BG_SYS_HEADER bg_sys_header = 
						{ GX_DISPMODE_GRAPHICS,GX_BGMODE_5,GX_BGMODE_0,GX_BG0_AS_3D };
			GFL_BG_SetBGMode( &bg_sys_header );
		}

		//ＢＧセットアップ
		G2_SetBG2ControlDCBmp
			(GX_BG_SCRSIZE_DCBMP_256x256, GX_BG_AREAOVER_XLU, GX_BG_BMPSCRBASE_0x00000);
		GX_SetBankForBG(GX_VRAM_BG_128_D);

		//アルファブレンド
		//G2_SetBlendAlpha(GX_BLEND_PLANEMASK_NONE, GX_BLEND_PLANEMASK_BG2, 0,0);
		GFL_BG_SetPriority(BG_FRAME_CROSS_FADE, 0);
		GFL_BG_SetVisible( BG_FRAME_CROSS_FADE, VISIBLE_ON );

    // キャプチャ面の拡大開始
    StartVBlankBGExpand();

		return GMEVENT_RES_FINISH;
	}

	return GMEVENT_RES_CONTINUE;
} 

//--------------------------------------------------------------------------------------------
/**
 * @brief クロスフェードインイベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT CrossInEvent( GMEVENT* event, int* seq, void* wk )
{
	FADE_EVENT_WORK* work = wk;

	switch( *seq )
  {
	case 0:
    work->alphaWork = 16;
    GFL_BG_SetPriority(BG_FRAME_CROSS_FADE, 0);
    GFL_BG_SetPriority(FLDBG_MFRM_3D, 1);
    GFL_BG_SetVisible( FLDBG_MFRM_3D, VISIBLE_ON );
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG0, work->alphaWork, 0 );

		// サブ画面輝度復帰
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, -2);
    (*seq)++;
    break;

	case 1:	// クロスフェード
		if(work->alphaWork){
			work->alphaWork--;
			G2_ChangeBlendAlpha( work->alphaWork, 16 - work->alphaWork );
		} else {
			GFL_BG_SetVisible( BG_FRAME_CROSS_FADE, VISIBLE_OFF );

			//VRAMクリア
			GX_SetBankForLCDC(GX_VRAM_LCDC_D);
			MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_VRAM_D_SIZE);
			(void)GX_DisableBankForLCDC();

			// メインBGへの割り当て復帰(fieldmap.cと整合性をとること)
			GX_SetBankForBG(GX_VRAM_LCDC_D);	

      // キャプチャBG面の拡大終了
      EndVBlankBGExpand();

      (*seq)++;
		}
		break;

	case 2:	// クロスフェードEND
		//OS_WaitVBlankIntr();	// 画面ちらつき防止用ウエイト
    // BGモード設定と表示設定の復帰
    {
      int mv = GFL_DISP_GetMainVisible();
      FIELDMAP_InitBGMode();
      GFL_DISP_GX_SetVisibleControlDirect( mv );
    }
    (*seq)++;
		break;

	case 3:	// クロスフェードEND
		FIELDMAP_InitBG(work->fieldmap);
		return GMEVENT_RES_FINISH;
	} 
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief 季節フェードイン イベント
 */ 
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT SeasonInEvent( GMEVENT* event, int* seq, void* wk )
{
	FADE_EVENT_WORK* work       = wk;
  GAMESYS_WORK*    gameSystem = work->gameSystem;
  FIELDMAP_WORK*   fieldmap   = work->fieldmap;

	switch( *seq )
  {
	case 0:
    // 季節表示イベント
    GMEVENT_CallEvent( event, 
        EVENT_SeasonDisplay( gameSystem, fieldmap, work->startSeason, work->endSeason ) );
		(*seq)++;
		break;
  case 1:
    return GMEVENT_RES_FINISH;
	} 
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief ワイプアウト イベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT WipeOutEvent( GMEVENT* event, int* seq, void* wk )
{
	FADE_EVENT_WORK* work       = wk;
  GAMESYS_WORK*    gameSystem = work->gameSystem;
  FIELDMAP_WORK*   fieldmap   = work->fieldmap;

	switch( *seq )
  {
  // ワイプアウト開始
	case 0: 
    WIPE_SYS_Start( WIPE_PATTERN_M, WIPE_TYPE_HOLEOUT, WIPE_TYPE_HOLEOUT, 0x0000, 
                    WIPE_DEF_DIV, WIPE_DEF_SYNC, FIELDMAP_GetHeapID(fieldmap) );
    (*seq)++;
		break;

  // ワイプアウト完了待ち
  case 1:
    if( WIPE_SYS_EndCheck() )
    {
      (*seq)++;
    }
    break;

  // イベント
  case 2:
    return GMEVENT_RES_FINISH;
	} 
	return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief デバッグ用 即時アウトイベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT QuickOutEvent( GMEVENT* event, int* seq, void* wk )
{
  GFL_FADE_SetMasterBrightReq( 
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, -16 );

  return GMEVENT_RES_FINISH;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief デバッグ用 即時フェードイン
 */ 
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT QuickInEvent( GMEVENT* event, int* seq, void* wk )
{
	FADE_EVENT_WORK* work = wk;

  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 
      16, 0, -16);

  // BGモード設定と表示設定の復帰
  {
    int mv = GFL_DISP_GetMainVisible();
    FIELDMAP_InitBGMode();
    GFL_DISP_GX_SetVisibleControlDirect( mv );
  }
  FIELDMAP_InitBG(work->fieldmap);

  return GMEVENT_RES_FINISH;
} 

//--------------------------------------------------------------------------------------------
/**
 * @brief 空を飛ぶ 輝度フェードイン イベント
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT FlySkyBrightInEvent( GMEVENT* event, int* seq, void* wk )
{
	FADE_EVENT_WORK* work = wk;

	switch( *seq )
  {
	case 0:
    GFL_FADE_SetMasterBrightReq(work->brightFadeMode, 16, 0, work->fadeFinishWaitFlag);
    // BGモード設定と表示設定の復帰
    {
      int mv = GFL_DISP_GetMainVisible();
      FIELDMAP_InitBGMode();
      GFL_DISP_GX_SetVisibleControlDirect( mv );
      FIELDMAP_InitBG(work->fieldmap);
    }
		(*seq)++;
		break;
	case 1:
    if( work->fadeFinishWaitFlag == FIELD_FADE_NO_WAIT ){ return GMEVENT_RES_FINISH; }  // フェード待たない
    if( GFL_FADE_CheckFade() == FALSE ){ return GMEVENT_RES_FINISH; }  // フェード完了
		break;
	} 
	return GMEVENT_RES_CONTINUE;
} 
