/*
 *  @file   fskill_amaikaori.c
 *  @brief  フィールド技：「あまいかおり」
 *  @author Miyuki Iwasawa
 *  @date   09.10.27
 */

#include <gflib.h>
#include "system/gfl_use.h"

#include "system/main.h"  //HEAPID_PROC

#include "sound/pm_sndsys.h"
#include "fieldmap.h"
#include "weather.h"

#include "field_skill.h"
#include "fld3d_ci.h"
#include "eventwork.h"

#include "script.h"
#include "fskill_amaikaori.h"
#include "encount_data.h"
#include "field_encount.h"
#include "fldmmdl.h"

#include "../../../resource/fldmapdata/script/field_ev_scr_def.h" // for SCRID_FLE_EV_AMAIKAORI_ENC_FAILED

#define AMAIKAORI_BG_FRAME  ( FLDBG_MFRM_EFF2 )
#define AMAIKAORI_PAL_IDX   ( 1 ) 

#define AMAIKAORI_SE_NO (SEQ_SE_FLD_18)

#define ALPHA_2ND (GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BD|GX_BLEND_PLANEMASK_OBJ)

typedef enum{
 SEQ_CUTIN_INIT,
 SEQ_CUTIN,
 SEQ_CUTIN_WAIT,
 SEQ_WEATHER_CHECK,
 SEQ_ENCOUNT_CHECK,
 SEQ_ENCOUNT_FAILED,
 SEQ_WEATHER_ERR,
 SEQ_END,
}AMAIKAORI_SEQ;

typedef struct _AMAIKAORI_WORK{
  u8  poke_pos;
  GAMESYS_WORK* gsys;
  GAMEDATA* gdata;
  FIELDMAP_WORK *fieldWork;
  FIELD_PLAYER* field_player;
}AMAIKAORI_WORK;

typedef struct _AMAIKAORI_EFFECT{
  u8      bg_frm;
  u8      evy;
  u8      wait;
  u8      wait_ct;
}AMAIKAORI_EFFECT;

static GMEVENT_RESULT FSkillAmaikaoriEvent(GMEVENT * event, int * seq, void *work);
static BOOL amaikaori_WeatherCheck( AMAIKAORI_WORK* wk );

static void EVENT_FieldAmaikaoriEffectCall( GMEVENT* parent_event, GAMESYS_WORK* gsys );
static GMEVENT_RESULT AmaikaoriEffectEvent(GMEVENT * event, int * seq, void *work);
static void amaikaori_BGResInit( AMAIKAORI_EFFECT* wk );
static void amaikaori_BGResRelease( AMAIKAORI_EFFECT* wk);

//------------------------------------------------------------------
/*
 *  @brief  あまいみつイベント起動
 */
//------------------------------------------------------------------
GMEVENT * EVENT_FieldAmaimitu( FIELDMAP_WORK* fieldmap, GAMESYS_WORK* gsys )
{
  return EVENT_FieldSkillAmaikaori( gsys, fieldmap, 0xFF );
}

//------------------------------------------------------------------
/*
 *  @brief  あまいかおりイベント起動
 */
//------------------------------------------------------------------
GMEVENT * EVENT_FieldSkillAmaikaori( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, u8 poke_pos )
{
	GMEVENT * event = GMEVENT_Create( gsys, NULL, FSkillAmaikaoriEvent, sizeof(AMAIKAORI_WORK));
	AMAIKAORI_WORK * wk = GMEVENT_GetEventWork(event);
  
  MI_CpuClear8(wk,sizeof(AMAIKAORI_WORK));
  wk->poke_pos = poke_pos;
  wk->gsys = gsys;
  wk->fieldWork = fieldmap;
  wk->field_player = FIELDMAP_GetFieldPlayer( fieldmap );
  wk->gdata = GAMESYSTEM_GetGameData(gsys);
	return event;
}

static GMEVENT_RESULT FSkillAmaikaoriEvent(GMEVENT * event, int * seq, void *work)
{
	AMAIKAORI_WORK * wk = work;
	
  switch (*seq) {
  case SEQ_CUTIN_INIT:
    if(wk->poke_pos >= 6){
      *seq = SEQ_WEATHER_CHECK;
      break;
    }
    FIELD_PLAYER_ChangeFormRequest( wk->field_player, PLAYER_DRAW_FORM_CUTIN );
    (*seq)++;
    break;
  case SEQ_CUTIN:
    if( FIELD_PLAYER_ChangeFormWait( wk->field_player )){
      GMEVENT *cutin_ev;
      cutin_ev = FLD3D_CI_CreatePokeCutInEvtTemoti( wk->gsys, FIELDMAP_GetFld3dCiPtr(wk->fieldWork), wk->poke_pos );
      GMEVENT_CallEvent( event, cutin_ev );
      (*seq)++;
    }
    break;
  case SEQ_CUTIN_WAIT:
    FIELD_PLAYER_ResetMoveForm( wk->field_player );
    *seq = SEQ_WEATHER_CHECK;
    break;
	case SEQ_WEATHER_CHECK:
    if( amaikaori_WeatherCheck( wk ) == FALSE){
      *seq = SEQ_WEATHER_ERR;
      break;
    }
    //エフェクト起動
    EVENT_FieldAmaikaoriEffectCall( event, wk->gsys );
    *seq = SEQ_ENCOUNT_CHECK;
    break;
  case SEQ_ENCOUNT_CHECK:
    //エンカウントチェック
    {
      GMEVENT* enc_ev = FIELD_ENCOUNT_CheckEncount( FIELDMAP_GetEncount( wk->fieldWork ), ENC_TYPE_FORCE );
      if( enc_ev == NULL ){  //エンカウト失敗
        *seq = SEQ_ENCOUNT_FAILED;
        break;
      }
      GMEVENT_CallEvent( event, enc_ev );
    }
		(*seq) = SEQ_END;
		break;
  case SEQ_ENCOUNT_FAILED:
    SCRIPT_CallScript( event,SCRID_FLD_EV_AMAIKAORI_ENC_FAILED, NULL, NULL, HEAPID_FIELDMAP );
		(*seq) = SEQ_END;
    break;
  case SEQ_WEATHER_ERR:
    SCRIPT_CallScript( event,SCRID_FLD_EV_AMAIKAORI_WEATHER_ERROR, NULL, NULL, HEAPID_FIELDMAP );
		(*seq) = SEQ_END;
    break;
	case SEQ_END:
    MMDLSYS_ClearPauseMoveProc( FIELDMAP_GetMMdlSys( wk->fieldWork ) );
		return GMEVENT_RES_FINISH;
	}
	return GMEVENT_RES_CONTINUE;
}

////////////////////////////////////////////////
// parts
////////////////////////////////////////////////
static BOOL amaikaori_WeatherCheck( AMAIKAORI_WORK* wk )
{
  FIELD_WEATHER* fld_weather = FIELDMAP_GetFieldWeather( wk->fieldWork );
  WEATHER_NO weather = FIELD_WEATHER_GetWeatherNo( fld_weather );

  if( weather == WEATHER_NO_SUNNY ){
    return TRUE;
  }
  return FALSE;
}

//------------------------------------------------------------------
/*
 *  @brief  あまいかおりエフェクトイベント起動
 */
//------------------------------------------------------------------
static void EVENT_FieldAmaikaoriEffectCall( GMEVENT* parent_event, GAMESYS_WORK* gsys )
{
	GMEVENT * event = GMEVENT_Create( gsys, NULL, AmaikaoriEffectEvent, sizeof(AMAIKAORI_EFFECT));
	AMAIKAORI_EFFECT * wk = GMEVENT_GetEventWork(event);
  
  wk->bg_frm = FLDBG_MFRM_EFF2;
  wk->evy = 0;
  wk->wait = 2;
  wk->wait_ct = 0;
  GMEVENT_CallEvent( parent_event, event );
}

static GMEVENT_RESULT AmaikaoriEffectEvent(GMEVENT * event, int * seq, void *work)
{
	AMAIKAORI_EFFECT * wk = work;
	
  switch (*seq) {
  case 0:
    amaikaori_BGResInit( wk );
    PMSND_PlaySE( AMAIKAORI_SE_NO );
    (*seq)++;
    break;
  case 1:
    if(wk->wait_ct++ >= wk->wait ) {
      wk->evy++;
    	G2_ChangeBlendAlpha( wk->evy, (16-wk->evy) );
      wk->wait_ct = 0;
    }
    if(wk->evy >= 8){
      (*seq)++;
    }
    break;
  case 2:
    if(wk->wait_ct++ < 30 ) {
      (*seq)++;
    }
    break;
  case 3:
    if(wk->wait_ct++ >= wk->wait ) {
      wk->evy--;
    	G2_ChangeBlendAlpha( wk->evy, (16-wk->evy) );
      wk->wait_ct = 0;
    }
    if(wk->evy <= 0){
      (*seq)++;
    }
    break;
  default:
    amaikaori_BGResRelease( wk );
		return GMEVENT_RES_FINISH;
  }
	return GMEVENT_RES_CONTINUE;
}

static void amaikaori_BGResInit( AMAIKAORI_EFFECT* wk )
{
  u16 pal[2] = {0x0000,GX_RGB(31,10,23)};

   GFL_BG_BGCNT_HEADER bgcntText = {
		0, 0, 0x800, 0,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0xd000, GX_BG_CHARBASE_0x08000, 0x100,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
	};
  GFL_BG_SetVisible( wk->bg_frm, VISIBLE_OFF ); //いったん表示Off

//	GFL_BG_SetBGControl( wk->bg_frm, &bgcntText, GFL_BG_MODE_TEXT );
		
  GFL_BG_LoadPalette( wk->bg_frm, pal, sizeof(16)*2, AMAIKAORI_PAL_IDX*0x20);
	GFL_BG_FillCharacter( wk->bg_frm, 0x01, 1, 0x100 );
	GFL_BG_FillScreen( wk->bg_frm,0x0100, 0, 0, 32, 32, AMAIKAORI_PAL_IDX );
	GFL_BG_LoadScreenReq( wk->bg_frm );
	
  //ＢＧコントロール設定
  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG3, ALPHA_2ND, wk->evy, (16-wk->evy) );
  GFL_BG_SetVisible( wk->bg_frm, VISIBLE_ON );
}

static void amaikaori_BGResRelease( AMAIKAORI_EFFECT* wk)
{
  u16 pal[2] = {0x0000,0x0000};

  GFL_BG_SetVisible( wk->bg_frm, VISIBLE_OFF );
  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_NONE, GX_BLEND_PLANEMASK_NONE, 0, 31 );

  GFL_BG_LoadPalette( wk->bg_frm, pal, sizeof(16)*2, AMAIKAORI_PAL_IDX*0x20);
	GFL_BG_FillScreen( wk->bg_frm,0x0000, 0, 0, 32, 32, AMAIKAORI_PAL_IDX );
	GFL_BG_LoadScreenReq( wk->bg_frm );
  GFL_BG_FillCharacterRelease( wk->bg_frm, 1, 0x100 );

//  GFL_BG_FreeBGControl( wk->bg_frm );
}

