//======================================================================
/*
 * @file	enceff_ci.c
 * @brief	イベント：エンカウントエフェクト
 * @author saito
 */
//======================================================================
#include <gflib.h>

#include "enceff.h"

#include "fieldmap.h"
#include "system/main.h"
#include "fld3d_ci.h"

#include "enc_cutin_no.h"   //for ENC_CUTIN_～

//--------------------------------------------------------------
/// ENCEFF_CI_WORK
//--------------------------------------------------------------
typedef struct
{
  int count;
  int Type;
}ENCEFF_CI_WORK;

static GMEVENT *CreateEffCommon(  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const int inType );
static GMEVENT_RESULT ev_encEffectFunc( GMEVENT *event, int *seq, void *wk );

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateRival(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_RIVAL );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateSupport(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_SUPPORT );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym01A(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM1A );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym01B(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM1B );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym01C(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM1C );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym02(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM2 );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym03(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM3 );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym04(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM4 );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym05(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM5 );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym06(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM6 );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym07(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM7 );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym08A(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM8A );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym08B(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM8B );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateBigFour1(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_BIGFOUR1 );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateBigFour2(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_BIGFOUR2 );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateBigFour3(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_BIGFOUR3 );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateBigFour4(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_BIGFOUR4 );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateChamp(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_CHAMP );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateBoss(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_BOSS );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateSage(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_SAGE );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreatePlasma(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_PLASMA );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成共通
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param inType
 *
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
static GMEVENT *CreateEffCommon(  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const int inType )
{
  ENCEFF_CNT_PTR cnt_ptr;
  GMEVENT *event;
  ENCEFF_CI_WORK *work;
  int size;

  size = sizeof(ENCEFF_CI_WORK);

  //ワークを確保
  {
    FIELDMAP_WORK * fieldmap;
    fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
    cnt_ptr = FIELDMAP_GetEncEffCntPtr(fieldmap);
  }
  work = ENCEFF_AllocUserWork(cnt_ptr, size, HEAPID_FLD3DCUTIN);
  work->Type = inType;

  event = GMEVENT_Create( gsys, NULL, ev_encEffectFunc, 0 );

  return( event );

}


//--------------------------------------------------------------
/**
 * イベント　エンカウントエフェクト
 * @param event GMEVENT
 * @param seq イベントシーケンス
 * @param wk イベントワーク
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ev_encEffectFunc( GMEVENT *event, int *seq, void *wk )
{
  ENCEFF_CI_WORK *work;
  GAMESYS_WORK * gsys;
  FIELDMAP_WORK * fieldmap;
  ENCEFF_CNT_PTR cnt_ptr;

  gsys = GMEVENT_GetGameSysWork(event);
  fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  cnt_ptr = FIELDMAP_GetEncEffCntPtr(fieldmap);
  work = ENCEFF_GetUserWorkPtr(cnt_ptr);
  
  switch( (*seq) )
  {
  case 0:
    GFL_FADE_SetMasterBrightReq(
        GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, -1 );
    (*seq)++;
  case 1:
    if( GFL_FADE_CheckFade() == FALSE ){
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 16, 0, -1 );
      (*seq)++;
    }
    break;
  case 2:
    if( GFL_FADE_CheckFade() == FALSE ){
      work->count++;
      
      if( work->count < 3 ){
        GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, -1 );
        (*seq) = 1;
      }
      else
      {
        GMEVENT *call_event;
        FLD3D_CI_PTR cutin_ptr;
        cutin_ptr = FIELDMAP_GetFld3dCiPtr(fieldmap);
        //カットインイベントコール
        call_event = FLD3D_CI_CreateEncCutInEvt( gsys, cutin_ptr, work->Type );
        //イベントコール
        GMEVENT_CallEvent( event, call_event );
				(*seq)++;
      }
    }
    break;
  case 3:
		
    return( GMEVENT_RES_FINISH );

  }
  
  return( GMEVENT_RES_CONTINUE );
}

