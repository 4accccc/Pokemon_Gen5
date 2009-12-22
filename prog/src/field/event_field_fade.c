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

#include "field/fieldmap_call.h"  //FIELDMAP_IsReady,FIELDMAP_ForceUpdate
#include "field/fieldmap.h"

#include "event_field_fade.h"

#include "system/main.h"      // HEAPID_PROC

#include "event_season_display.h"  // for EVENT_SeasonDisplay
#include "field_status_local.h"  // for FIELD_STATUS

#include "system/screentex.h"

#include "gamesystem/pm_season.h"  // for PMSEASON_xxxx

#ifdef PM_DEBUG
BOOL DebugBGInitEnd = FALSE;    //BG初期化監視フラグ
#endif


#define BG_FRAME_CROSS_FADE ( FLDBG_MFRM_EFF1 )




//============================================================================================
// ■フェードイベントワーク
//============================================================================================ 
typedef struct {
  GAMESYS_WORK* gsys;
  FIELDMAP_WORK* fieldmap;

	FIELD_FADE_TYPE fade_type;
  FIELD_FADE_SEASON_FLAG season_flag;
  FIELD_FADE_WAIT_TYPE wait_type; 

  int brightFadeMode;  // 輝度フェードモード

	int alphaWork;

  BOOL bg_init_flag;

} FADE_EVENT_WORK;

//------------------------------------------------------------------
/**
 * @brief 輝度フェードモードを取得する
 *
 * @param FIELD_FADE_TYPE フェードタイプ
 *
 * @return 指定フェードタイプに応じた輝度フェードモード
 */
//------------------------------------------------------------------
static int GetBrightFadeMode( FIELD_FADE_TYPE type )
{
  int mode;
  switch( type )
  {
  default:
  case FIELD_FADE_BLACK: // 輝度フェード(ブラック)
    mode = GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB;
    break;
  case FIELD_FADE_WHITE: // 輝度フェード(ホワイト)
    mode = GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN | GFL_FADE_MASTER_BRIGHT_WHITEOUT_SUB;
    break;
  }
  return mode;
}


//============================================================================================
// ■フェードアウト
//============================================================================================ 

//------------------------------------------------------------------
/**
 * @brief 輝度フェードアウトイベント
 */
//------------------------------------------------------------------
static GMEVENT_RESULT FieldBrightOutEvent(GMEVENT * event, int *seq, void * work)
{
	FADE_EVENT_WORK * few = work;
	switch (*seq) {
	case 0:
    GFL_FADE_SetMasterBrightReq( few->brightFadeMode, 0, 16, few->wait_type);
		(*seq) ++;
		break;
	case 1:
    if( few->wait_type == FIELD_FADE_NO_WAIT ){ return GMEVENT_RES_FINISH; }
		if( GFL_FADE_CheckFade() == FALSE ){ return GMEVENT_RES_FINISH; }
		break;
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief クロスフェードアウトイベント
 */
//------------------------------------------------------------------
static GMEVENT_RESULT FieldCrossOutEvent(GMEVENT * event, int *seq, void * work)
{
	FADE_EVENT_WORK * few = work;

	switch (*seq)
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

		return GMEVENT_RES_FINISH;
	}

	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief デバッグ用 即時アウトイベント
 */
//------------------------------------------------------------------
static GMEVENT_RESULT QuickFadeOutEvent(GMEVENT * event, int *seq, void * work)
{
  GFL_FADE_SetMasterBrightReq( 
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 
      0, 16, -16 );
  return GMEVENT_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	クロスフェードアウトイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @return GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
static GMEVENT* EVENT_FieldCrossOut( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap )
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldCrossOutEvent, sizeof(FADE_EVENT_WORK));
	FADE_EVENT_WORK * few = GMEVENT_GetEventWork(event);
  few->gsys      = gsys;
  few->fieldmap  = fieldmap;
  few->fade_type = FIELD_FADE_CROSS;

	return event;
}

//------------------------------------------------------------------
/**
 * @brief	ブライトネスアウトイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param wait      フェード完了を待つかどうか
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
static GMEVENT* EVENT_FieldBrightOut( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                                      FIELD_FADE_TYPE type, 
                                      FIELD_FADE_WAIT_TYPE wait )
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldBrightOutEvent, sizeof(FADE_EVENT_WORK));
	FADE_EVENT_WORK * few = GMEVENT_GetEventWork(event);
  few->gsys        = gsys;
  few->fieldmap    = fieldmap;
  few->fade_type   = type;
  few->wait_type   = wait;
  few->brightFadeMode = GetBrightFadeMode( type );

	return event;
}

//------------------------------------------------------------------
/**
 * @brief	フェードアウトイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param wait      フェード完了を待つかどうか(輝度フェード時のみ有効)
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
GMEVENT* EVENT_FieldFadeOut( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                             FIELD_FADE_TYPE type, 
                             FIELD_FADE_WAIT_TYPE wait ) 
{ 
  GMEVENT* event;

  // フェードイベント生成
  switch( type )
  {
  default:
  case FIELD_FADE_CROSS: // クロスフェード
    event = EVENT_FieldCrossOut( gsys, fieldmap );
    break;
  case FIELD_FADE_BLACK: // 輝度フェード(ブラック)
  case FIELD_FADE_WHITE: // 輝度フェード(ホワイト)
    event = EVENT_FieldBrightOut( gsys, fieldmap, type, wait );
    break;
  }
	return event;
}

//------------------------------------------------------------------
/**
 * @brief	空を飛ぶブライトネスアウトイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param wait      フェード完了を待つかどうか
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
GMEVENT* EVENT_FlySkyBrightOut( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                                FIELD_FADE_TYPE type, FIELD_FADE_WAIT_TYPE wait )
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldBrightOutEvent, sizeof(FADE_EVENT_WORK));
	FADE_EVENT_WORK * few = GMEVENT_GetEventWork(event);
  few->gsys      = gsys;
  few->fieldmap  = fieldmap;
	few->fade_type = type;
  few->wait_type = wait;
  few->brightFadeMode = GetBrightFadeMode( type );

	return event;
}

//------------------------------------------------------------------
/**
 * @brief	デバッグ用 即時アウト
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
GMEVENT* DEBUG_EVENT_QuickFadeOut( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap)
{
	GMEVENT* event;
	FADE_EVENT_WORK* few;

  event = GMEVENT_Create(gsys, NULL, QuickFadeOutEvent, sizeof(FADE_EVENT_WORK));
	few = GMEVENT_GetEventWork(event);
  few->gsys     = gsys;
  few->fieldmap = fieldmap;
  return event;
}


//============================================================================================
// ■フェードイン
//============================================================================================ 

//------------------------------------------------------------------
/**
 * @brief クロスフェードインイベント
 */
//------------------------------------------------------------------
static GMEVENT_RESULT FieldCrossInEvent(GMEVENT * event, int *seq, void * work)
{
	FADE_EVENT_WORK* few = work;

	switch (*seq)
  {
	case 0:
    few->alphaWork = 16;
    GFL_BG_SetPriority(BG_FRAME_CROSS_FADE, 0);
    GFL_BG_SetPriority(FLDBG_MFRM_3D, 1);
    GFL_BG_SetVisible( FLDBG_MFRM_3D, VISIBLE_ON );
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG0, few->alphaWork, 0 );

		// サブ画面輝度復帰
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, -2);
    ++ *seq;
    break;

	case 1:	// クロスフェード
		if(few->alphaWork){
			few->alphaWork--;
			G2_ChangeBlendAlpha( few->alphaWork, 16 - few->alphaWork );
		} else {
			GFL_BG_SetVisible( BG_FRAME_CROSS_FADE, VISIBLE_OFF );

			//VRAMクリア
			GX_SetBankForLCDC(GX_VRAM_LCDC_D);
			MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_VRAM_D_SIZE);
			(void)GX_DisableBankForLCDC();

			// メインBGへの割り当て復帰(fieldmap.cと整合性をとること)
			GX_SetBankForBG(GX_VRAM_LCDC_D);	
			++ *seq;
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
		++ *seq;
		break;

	case 3:	// クロスフェードEND
		FIELDMAP_InitBG(few->fieldmap);
		return GMEVENT_RES_FINISH;
	} 
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief 輝度フェードインイベント
 */ 
//------------------------------------------------------------------
static GMEVENT_RESULT FieldBrightInEvent(GMEVENT * event, int *seq, void * work)
{
	FADE_EVENT_WORK* few = work;
	switch (*seq) {
	case 0:
    { 
      GAMEDATA*       gdata = GAMESYSTEM_GetGameData( few->gsys );
      FIELD_STATUS* fstatus = GAMEDATA_GetFieldStatus( gdata );

      if( FIELD_STATUS_GetSeasonDispFlag(fstatus) )  // if(季節表示フラグON)
      { // 四季表示
        u8 start, end;
        start = (FIELD_STATUS_GetSeasonDispLast( fstatus ) + 1) % PMSEASON_TOTAL;
        end   = GAMEDATA_GetSeasonID( gdata );
        GMEVENT_CallEvent( event, EVENT_SeasonDisplay( few->gsys, few->fieldmap, start, end ) );
        FIELD_STATUS_SetSeasonDispFlag( fstatus, FALSE );
        FIELD_STATUS_SetSeasonDispLast( fstatus, end );
        *seq = 2;
        break;
      }
      else
      {
        GFL_FADE_SetMasterBrightReq(few->brightFadeMode, 16, 0, 0);

        if ( few->bg_init_flag )
        {
          // BGモード設定と表示設定の復帰
          {
            int mv = GFL_DISP_GetMainVisible();
            FIELDMAP_InitBGMode();
            GFL_DISP_GX_SetVisibleControlDirect( mv );
          }
          FIELDMAP_InitBG(few->fieldmap);
        }
      }
    }
		(*seq) ++;
		break;
	case 1:
    {
      BOOL rc = FALSE;
      if( few->wait_type == FIELD_FADE_NO_WAIT ) rc = TRUE;
		  if( GFL_FADE_CheckFade() == FALSE ) rc = TRUE; 
      if( rc ){ (*seq) ++; }
    }
		break;
  case 2:
    return GMEVENT_RES_FINISH;
	}

	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief 空を飛ぶ輝度フェードアウトイベント
 */
//------------------------------------------------------------------
static GMEVENT_RESULT FlySkyBrightInEvent(GMEVENT * event, int *seq, void * work)
{
	FADE_EVENT_WORK* few = work;
	switch (*seq) {
	case 0:
    { 
      GAMEDATA*       gdata = GAMESYSTEM_GetGameData( few->gsys );
      FIELD_STATUS* fstatus = GAMEDATA_GetFieldStatus( gdata );
      // if(四季表示指定有 && 季節表示フラグON)
      if( (few->season_flag == FIELD_FADE_SEASON_ON) && FIELD_STATUS_GetSeasonDispFlag(fstatus) )  
      { // 四季表示
        u8 start, end;
        start = (FIELD_STATUS_GetSeasonDispLast( fstatus ) + 1) % PMSEASON_TOTAL;
        end   = GAMEDATA_GetSeasonID( gdata );
        GMEVENT_CallEvent( event, EVENT_SeasonDisplay( few->gsys, few->fieldmap, start, end ) );
        FIELD_STATUS_SetSeasonDispFlag( fstatus, FALSE );
        FIELD_STATUS_SetSeasonDispLast( fstatus, end );
        *seq = 2;
        break;
      }
      else
      {
        GFL_FADE_SetMasterBrightReq(few->brightFadeMode, 16, 0, few->wait_type);
        // BGモード設定と表示設定の復帰
        {
          int mv = GFL_DISP_GetMainVisible();
          FIELDMAP_InitBGMode();
          GFL_DISP_GX_SetVisibleControlDirect( mv );
        }
        FIELDMAP_InitBG(few->fieldmap);
      }
    }
		(*seq) ++;
		break;
	case 1:
    {
      BOOL rc = FALSE;
      if( few->wait_type == FIELD_FADE_NO_WAIT ) rc = TRUE;
		  if( GFL_FADE_CheckFade() == FALSE ) rc = TRUE; 
      if( rc ){ (*seq) ++; }
    }
		break;
  case 2:
    return GMEVENT_RES_FINISH;
	}

	return GMEVENT_RES_CONTINUE;
} 

//------------------------------------------------------------------
/**
 * @brief デバッグ用 即時フェードイン
 */ 
//------------------------------------------------------------------
static GMEVENT_RESULT QuickFadeInEvent(GMEVENT * event, int *seq, void * work)
{
	FADE_EVENT_WORK* few = work;

  GFL_FADE_SetMasterBrightReq(
      GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 
      16, 0, -16);

  // BGモード設定と表示設定の復帰
  {
    int mv = GFL_DISP_GetMainVisible();
    FIELDMAP_InitBGMode();
    GFL_DISP_GX_SetVisibleControlDirect( mv );
  }
  FIELDMAP_InitBG(few->fieldmap);

  return GMEVENT_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	クロスフェードインイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap  フィールドマップワークへのポインタ
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
static GMEVENT* EVENT_FieldCrossIn( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap )
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldCrossInEvent, sizeof(FADE_EVENT_WORK));
	FADE_EVENT_WORK * few = GMEVENT_GetEventWork(event);
  few->gsys      = gsys;
  few->fieldmap  = fieldmap;
  few->fade_type = FIELD_FADE_CROSS;

	return event;
}

//------------------------------------------------------------------
/**
 * @brief	ブライトネスインイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap  フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param season    季節表示を許可するかどうか
 * @param wait      フェード完了を待つかどうか
 * @param bg_init_flag  BG初期化を内部でおこなうかどうか
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
static GMEVENT* EVENT_FieldBrightIn( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                                     FIELD_FADE_TYPE type, 
                                     FIELD_FADE_SEASON_FLAG season, 
                                     FIELD_FADE_WAIT_TYPE wait,
                                     BOOL bg_init_flag )
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldBrightInEvent, sizeof(FADE_EVENT_WORK));
	FADE_EVENT_WORK * few = GMEVENT_GetEventWork(event);
  few->gsys        = gsys;
  few->fieldmap    = fieldmap;
	few->fade_type   = type;
  few->season_flag = season;
  few->wait_type   = wait;
  few->brightFadeMode = GetBrightFadeMode( type );
  few->bg_init_flag = bg_init_flag;

	return event;
}

//------------------------------------------------------------------
/**
 * @brief	フェードインイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap  フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param season    季節表示を許可するかどうか(輝度フェード時のみ有効)
 * @param wait      フェード完了を待つかどうか(輝度フェード時のみ有効)
 * @param bg_init_flag  BG初期化を内部でおこなうかどうか(輝度フェード時のみ有効)
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
GMEVENT* EVENT_FieldFadeIn( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                            FIELD_FADE_TYPE type, 
                            FIELD_FADE_SEASON_FLAG season,
                            FIELD_FADE_WAIT_TYPE wait,
                            BOOL bg_init_flag )
{
  GMEVENT* event;

  // フェードイベント生成
  switch( type )
  {
  default:
  case FIELD_FADE_CROSS: // クロスフェード
    event = EVENT_FieldCrossIn( gsys, fieldmap );
    break;
  case FIELD_FADE_BLACK: // 輝度フェード(ブラック)
  case FIELD_FADE_WHITE: // 輝度フェード(ホワイト)
    event = EVENT_FieldBrightIn( gsys, fieldmap, type, season, wait, bg_init_flag );
    break;
  }
	return event; 
}

//------------------------------------------------------------------
/**
 * @brief	空を飛ぶブライトネスインイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap  フィールドマップワークへのポインタ
 * @param	type		  フェードの種類指定
 * @param wait      フェード完了を待つかどうか
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
GMEVENT * EVENT_FlySkyBrightIn( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                               FIELD_FADE_TYPE type, FIELD_FADE_WAIT_TYPE wait )
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FlySkyBrightInEvent, sizeof(FADE_EVENT_WORK));
	FADE_EVENT_WORK * few = GMEVENT_GetEventWork(event);
  few->gsys      = gsys;
  few->fieldmap  = fieldmap;
	few->fade_type = type;
  few->wait_type = wait;
  few->brightFadeMode = GetBrightFadeMode( type );

	return event;
}

//------------------------------------------------------------------
/**
 * @brief	デバッグ用 即時アウト
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
GMEVENT* DEBUG_EVENT_QuickFadeIn( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap)
{
	GMEVENT* event;
  FADE_EVENT_WORK* few;

  event = GMEVENT_Create(gsys, NULL, QuickFadeInEvent, sizeof(FADE_EVENT_WORK));
	few = GMEVENT_GetEventWork(event);
  few->gsys      = gsys;
  few->fieldmap = fieldmap;
  return event;
}


