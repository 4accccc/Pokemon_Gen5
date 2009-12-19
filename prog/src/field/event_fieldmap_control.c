//============================================================================================
/**
 * @file	event_fieldmap_control.c
 * @brief	イベント：フィールドマップ制御ツール
 * @author	tamada GAMEFREAK inc.
 * @date	2008.12.10
 */
//============================================================================================

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap_call.h"  //FIELDMAP_IsReady,FIELDMAP_ForceUpdate
#include "field/fieldmap.h"

#include "./event_fieldmap_control.h"

#include "system/main.h"      // HEAPID_PROC

#include "event_season_display.h"  // for EVENT_SeasonDisplay
#include "field_status_local.h"  // for FIELD_STATUS

#include "system/screentex.h"

#include "gamesystem/pm_season.h"  // for PMSEASON_xxxx

FS_EXTERN_OVERLAY(pokelist);
FS_EXTERN_OVERLAY(poke_status);

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
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
static GMEVENT* EVENT_FieldBrightIn( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                                     FIELD_FADE_TYPE type, 
                                     FIELD_FADE_SEASON_FLAG season, 
                                     FIELD_FADE_WAIT_TYPE wait )
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldBrightInEvent, sizeof(FADE_EVENT_WORK));
	FADE_EVENT_WORK * few = GMEVENT_GetEventWork(event);
  few->gsys        = gsys;
  few->fieldmap    = fieldmap;
	few->fade_type   = type;
  few->season_flag = season;
  few->wait_type   = wait;
  few->brightFadeMode = GetBrightFadeMode( type );

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
 * @return	GMEVENT	生成したイベントへのポインタ
 */
//------------------------------------------------------------------
GMEVENT* EVENT_FieldFadeIn( GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap, 
                            FIELD_FADE_TYPE type, 
                            FIELD_FADE_SEASON_FLAG season,
                            FIELD_FADE_WAIT_TYPE wait )
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
    event = EVENT_FieldBrightIn( gsys, fieldmap, type, season, wait );
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


//============================================================================================
// ■フィールドマップの開始/終了
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK * gsys;
	FIELDMAP_WORK * fieldmap;
	GAMEDATA * gamedata;
}FIELD_OPENCLOSE_WORK;
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT FieldCloseEvent(GMEVENT * event, int * seq, void *work)
{
	FIELD_OPENCLOSE_WORK * focw = work;
	GAMESYS_WORK * gsys = focw->gsys;
	FIELDMAP_WORK * fieldmap = focw->fieldmap;
	switch (*seq) {
	case 0:
		FIELDMAP_Close(fieldmap);
		(*seq) ++;
		break;
	case 1:
		if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL) break;
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_FieldClose(GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldCloseEvent, sizeof(FIELD_OPENCLOSE_WORK));
	FIELD_OPENCLOSE_WORK * focw = GMEVENT_GetEventWork(event);
	focw->gsys = gsys;
	focw->fieldmap = fieldmap;
	focw->gamedata = GAMESYSTEM_GetGameData(gsys);
	return event;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT FieldOpenEvent(GMEVENT * event, int *seq, void*work)
{
	FIELD_OPENCLOSE_WORK * focw = work;
	GAMESYS_WORK * gsys = focw->gsys;
	FIELDMAP_WORK * fieldmap;	// = focw->fieldmap;
	switch(*seq) {
	case 0:
		if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL) break;
#if 0
		GFL_FADE_SetMasterBrightReq(
				GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB,
        16, 16, 0);
#endif
		GAMESYSTEM_CallFieldProc(gsys);
		(*seq) ++;
		break;
	case 1:
		fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
		if (FIELDMAP_IsReady(fieldmap) == FALSE) break;
		(*seq) ++;
		break;
	case 2:
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_FieldOpen(GAMESYS_WORK *gsys)
{
	GMEVENT * event = GMEVENT_Create(gsys, NULL, FieldOpenEvent, sizeof(FIELD_OPENCLOSE_WORK));
	FIELD_OPENCLOSE_WORK * focw = GMEVENT_GetEventWork(event);
	focw->gsys = gsys;
	focw->fieldmap = NULL;
	focw->gamedata = GAMESYSTEM_GetGameData(gsys);
	return event;
}


//============================================================================================
// ■サブプロセス呼び出し
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct
{
	GAMESYS_WORK*             gsys;
	FIELDMAP_WORK*        fieldmap;
	FSOverlayID              ov_id;
	const GFL_PROC_DATA* proc_data;
	void*                proc_work;

} CHANGE_SAMPLE_WORK;
//------------------------------------------------------------------
/**
 * @brief イベント処理関数
 */
//------------------------------------------------------------------
static GMEVENT_RESULT GameChangeEvent( GMEVENT* event, int* seq, void* work )
{
	CHANGE_SAMPLE_WORK* csw = work;
	GAMESYS_WORK*      gsys = csw->gsys;

	switch( *seq )
  {
	case 0:
    { // フェードアウト
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeOut_Black(gsys, csw->fieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
		(*seq) ++;
		break;
	case 1:
    { // サブプロセス呼び出しイベント
      GMEVENT* ev_sub;
      ev_sub = EVENT_FieldSubProcNoFade( csw->gsys, csw->fieldmap, 
                                        csw->ov_id, csw->proc_data, csw->proc_work );
      GMEVENT_CallEvent( event, ev_sub );
    }
		(*seq) ++;
		break;
	case 2:
    { // フェードイン
      GMEVENT* fade_event;
      fade_event = EVENT_FieldFadeIn_Black(gsys, csw->fieldmap, FIELD_FADE_WAIT);
      GMEVENT_CallEvent(event, fade_event);
    }
		(*seq) ++;
		break;
	case 3:
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
}
//------------------------------------------------------------------
/**
 * @brief イベント処理関数(フェードなし)
 */
//------------------------------------------------------------------
static GMEVENT_RESULT GameChangeEvent_NoFade(GMEVENT * event, int * seq, void * work)
{
	CHANGE_SAMPLE_WORK* csw = work;
	GAMESYS_WORK*      gsys = csw->gsys;

	switch( *seq )
  {
	case 0:  // フィールドマップ破棄
		GMEVENT_CallEvent( event, EVENT_FieldClose(gsys, csw->fieldmap) );
		(*seq) ++;
		break;
	case 1:  // サブプロセス呼び出し
		GAMESYSTEM_CallProc( gsys, csw->ov_id, csw->proc_data, csw->proc_work );
		(*seq) ++;
		break;
	case 2:  // サブプロセス終了待ち
		if( GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL ) break;
		(*seq) ++;
		break;
	case 3:  // フィールドマップ再開
		GMEVENT_CallEvent( event, EVENT_FieldOpen(gsys) );
		(*seq) ++;
		break;
	case 4:
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
}


//------------------------------------------------------------------
/**
 * @brief	サブプロセス呼び出しイベント生成
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @param	ov_id		  遷移するサブプロセスのオーバーレイ指定
 * @param	proc_data	遷移するサブプロセスのプロセスデータへのポインタ
 * @param	proc_work	遷移するサブプロセスで使用するワークへのポインタ
 * @return 生成したイベントへのポインタ
 *
 * フェードアウト→フィールドマップ終了→サブプロセス呼び出し
 * →フィールドマップ再開→フェードインを処理する
 */
//------------------------------------------------------------------
GMEVENT* EVENT_FieldSubProc( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap,
		                         FSOverlayID ov_id, 
                             const GFL_PROC_DATA* proc_data, void* proc_work )
{
	GMEVENT* event;
	CHANGE_SAMPLE_WORK* csw; 

  // 生成
  event = GMEVENT_Create( gsys, NULL, GameChangeEvent, sizeof(CHANGE_SAMPLE_WORK) );
  // 初期化
	csw = GMEVENT_GetEventWork( event );
	csw->gsys      = gsys;
	csw->fieldmap  = fieldmap;
	csw->ov_id     = ov_id;
	csw->proc_data = proc_data;
	csw->proc_work = proc_work;
	return event;
}

//------------------------------------------------------------------
/**
 * @brief	サブプロセス呼び出しイベント生成(フェードなし)
 * @param	gsys		  GAMESYS_WORKへのポインタ
 * @param	fieldmap	フィールドマップワークへのポインタ
 * @param	ov_id		  遷移するサブプロセスのオーバーレイ指定
 * @param	proc_data	遷移するサブプロセスのプロセスデータへのポインタ
 * @param	proc_work	遷移するサブプロセスで使用するワークへのポインタ
 * @return 生成したイベントへのポインタ
 *
 * フィールドマップ終了 → サブプロセス呼び出し → フィールドマップ再開
 */
//------------------------------------------------------------------
GMEVENT* EVENT_FieldSubProcNoFade( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap,
		                               FSOverlayID ov_id, 
                                   const GFL_PROC_DATA* proc_data, void* proc_work )
{
	GMEVENT* event;
	CHANGE_SAMPLE_WORK* csw; 

  // 生成
  event = GMEVENT_Create( gsys, NULL, GameChangeEvent_NoFade, sizeof(CHANGE_SAMPLE_WORK) );
  // 初期化
	csw = GMEVENT_GetEventWork( event );
	csw->gsys      = gsys;
	csw->fieldmap  = fieldmap;
	csw->ov_id     = ov_id;
	csw->proc_data = proc_data;
	csw->proc_work = proc_work;
	return event;
}


//============================================================================================
//
//	イベント：別画面呼び出し(コールバック呼び出し付き)
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct
{
	GAMESYS_WORK*             gsys;
	FIELDMAP_WORK*        fieldmap;
	FSOverlayID              ov_id;
	const GFL_PROC_DATA* proc_data;
	void*                proc_work;
  void ( *callback )( void* );      // コールバック関数
  void*            callback_work;   // コールバック関数の引数

} SUBPROC_WORK;
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT GameChangeEvent_Callback(GMEVENT * event, int * seq, void * work)
{
	SUBPROC_WORK * spw = work;
	GAMESYS_WORK *gsys = spw->gsys;

	switch(*seq) 
  {
	case 0:
    { // サブプロセス呼び出しイベント
      GMEVENT* ev_sub;
      ev_sub = EVENT_FieldSubProc( spw->gsys, spw->fieldmap, 
                                  spw->ov_id, spw->proc_data, spw->proc_work );
      GMEVENT_CallEvent( event, ev_sub );
    }
		(*seq) ++;
		break;
	case 1: // コールバック関数呼び出し
    if( spw->callback ) spw->callback( spw->callback_work );
		(*seq) ++;
    break;
  case 2:  // コールバックワーク破棄
    if( spw->callback_work ){ GFL_HEAP_FreeMemory( spw->callback_work ); }
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
} 
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT GameChangeEventNoFade_Callback(GMEVENT * event, int * seq, void * work)
{
	SUBPROC_WORK * spw = work;
	GAMESYS_WORK *gsys = spw->gsys;

	switch(*seq) 
  {
	case 0:
    { // サブプロセス呼び出しイベント
      GMEVENT* ev_sub;
      ev_sub = EVENT_FieldSubProcNoFade( spw->gsys, spw->fieldmap, 
                                        spw->ov_id, spw->proc_data, spw->proc_work );
      GMEVENT_CallEvent( event, ev_sub );
    }
		(*seq) ++;
		break;
	case 1: // コールバック関数呼び出し
    if( spw->callback ) spw->callback( spw->callback_work );
		(*seq) ++;
    break;
  case 2:  // コールバックワーク破棄
    if( spw->callback_work ){ GFL_HEAP_FreeMemory( spw->callback_work ); }
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
}


//------------------------------------------------------------------
/**
 * @brief	サブプロセス呼び出しイベント生成(コールバック関数付き)
 * @param	gsys		      GAMESYS_WORKへのポインタ
 * @param	fieldmap	    フィールドマップワークへのポインタ
 * @param	ov_id		      遷移するサブプロセスのオーバーレイ指定
 * @param	proc_data	    遷移するサブプロセスのプロセスデータへのポインタ
 * @param	proc_work	    遷移するサブプロセスで使用するワークへのポインタ
 * @param callback      コールバック関数
 * @param callback_work コールバック関数にわたすポインタ
 * @return	GMEVENT		生成したイベントへのポインタ
 *
 * フェードアウト→フィールドマップ終了→サブプロセス呼び出し
 * →フィールドマップ再開→フェードインを処理する
 */
//------------------------------------------------------------------
GMEVENT * EVENT_FieldSubProc_Callback(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		FSOverlayID ov_id, const GFL_PROC_DATA * proc_data, void * proc_work,
    void (*callback)(void*), void* callback_work )
{
	GMEVENT *     event = GMEVENT_Create(gsys, NULL, GameChangeEvent_Callback, sizeof(SUBPROC_WORK));
	SUBPROC_WORK *  spw = GMEVENT_GetEventWork(event);
	spw->gsys          = gsys;
	spw->fieldmap      = fieldmap;
	spw->ov_id         = ov_id;
	spw->proc_data     = proc_data;
	spw->proc_work     = proc_work;
  spw->callback      = callback;
  spw->callback_work = callback_work;
	return event;
}

//------------------------------------------------------------------
/**
 * @brief	サブプロセス呼び出しイベント生成(フェードなし, コールバック関数付き)
 * @param	gsys		      GAMESYS_WORKへのポインタ
 * @param	fieldmap	    フィールドマップワークへのポインタ
 * @param	ov_id		      遷移するサブプロセスのオーバーレイ指定
 * @param	proc_data	    遷移するサブプロセスのプロセスデータへのポインタ
 * @param	proc_work	    遷移するサブプロセスで使用するワークへのポインタ
 * @param callback      コールバック関数
 * @param callback_work コールバック関数にわたすポインタ
 * @return	GMEVENT		生成したイベントへのポインタ
 *
 * フェードアウト→フィールドマップ終了→サブプロセス呼び出し
 * →フィールドマップ再開→フェードインを処理する
 */
//------------------------------------------------------------------
GMEVENT * EVENT_FieldSubProcNoFade_Callback(
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
		FSOverlayID ov_id, const GFL_PROC_DATA * proc_data, void * proc_work,
    void (*callback)(void*), void* callback_work )
{
	GMEVENT*     event = GMEVENT_Create(gsys, NULL, GameChangeEventNoFade_Callback, sizeof(SUBPROC_WORK));
	SUBPROC_WORK*  spw = GMEVENT_GetEventWork(event);
	spw->gsys          = gsys;
	spw->fieldmap      = fieldmap;
	spw->ov_id         = ov_id;
	spw->proc_data     = proc_data;
	spw->proc_work     = proc_work;
  spw->callback      = callback;
  spw->callback_work = callback_work;
	return event;
}



//============================================================================================
//
//	イベント：ポケモン選択
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	GAMESYS_WORK * gsys;
	FIELDMAP_WORK * fieldmap;
  PLIST_DATA* plData;
  PSTATUS_DATA* psData;
}POKE_SELECT_WORK;
//------------------------------------------------------------------
//------------------------------------------------------------------
static GMEVENT_RESULT EVENT_FUNC_PokeSelect(GMEVENT * event, int * seq, void * work)
{
	POKE_SELECT_WORK * psw = work;
	GAMESYS_WORK *gsys = psw->gsys;

  // シーケンス定義
  enum
  {
    SEQ_CALL_POKELIST = 0,
    SEQ_WAIT_POKELIST,
    SEQ_EXIT_POKELIST,
    SEQ_CALL_POKESTATUS,
    SEQ_WAIT_POKESTATUS,
    SEQ_END,
  };

	switch( *seq ) 
  {
	case SEQ_CALL_POKELIST: //// ポケモンリスト呼び出し
		GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(pokelist), &PokeList_ProcData, psw->plData);
    *seq = SEQ_WAIT_POKELIST;
		break;
	case SEQ_WAIT_POKELIST: //// プロック終了待ち
		if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL) break;
    *seq = SEQ_EXIT_POKELIST;
		break;
  case SEQ_EXIT_POKELIST: //// ポケモンリストの終了状態で分岐
    if( psw->plData->ret_mode == PL_RET_NORMAL )   // 選択終了
    {
      *seq = SEQ_END;
    }
    else if( psw->plData->ret_mode == PL_RET_STATUS )  //「つよさをみる」を選択
    {
      psw->psData->pos = psw->plData->ret_sel;   // 表示するデータ位置 = 選択ポケ
      *seq = SEQ_CALL_POKESTATUS;
    }
    else  // 未対応な項目を選択
    {
      OBATA_Printf( "----------------------------------------------------\n" );
      OBATA_Printf( "ポケモン選択イベント: 未対応な項目が選択されました。\n" );
      OBATA_Printf( "----------------------------------------------------\n" );
      *seq = SEQ_END;
    }
    break;
  case SEQ_CALL_POKESTATUS: //// ポケモンステータス呼び出し
		GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(poke_status), &PokeStatus_ProcData, psw->psData);
    *seq = SEQ_WAIT_POKESTATUS;
		break;
	case SEQ_WAIT_POKESTATUS: //// プロック終了待ち
		if (GAMESYSTEM_IsProcExists(gsys) != GFL_PROC_MAIN_NULL) break;
    psw->plData->ret_sel = psw->psData->pos;  // ステータス画面終了時のポケモンにカーソルを合わせる
    *seq = SEQ_CALL_POKELIST;
		break;
	case SEQ_END: //// イベント終了
		return GMEVENT_RES_FINISH;
		
	}
	return GMEVENT_RES_CONTINUE;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
GMEVENT * EVENT_PokeSelect( 
    GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap,
    PLIST_DATA* list_data, PSTATUS_DATA* status_data )
{
	GMEVENT* event;
	POKE_SELECT_WORK* psw;

  // イベント生成
  event = GMEVENT_Create(gsys, NULL, EVENT_FUNC_PokeSelect, sizeof(POKE_SELECT_WORK));
  psw   = GMEVENT_GetEventWork(event);
	psw->gsys      = gsys;
	psw->fieldmap  = fieldmap;
  psw->plData    = list_data;
  psw->psData    = status_data;
	return event;
}
