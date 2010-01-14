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
GMEVENT *ENCEFF_CI_Create1(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, 0 );
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

