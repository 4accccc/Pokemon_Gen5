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
  int Count;
  int Type;
  BOOL IsWhiteFade;
}ENCEFF_CI_WORK;

static GMEVENT *CreateEffCommon(  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const int inType, const BOOL inIsFadeWhite );
static GMEVENT_RESULT ev_encEffectFunc( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT BlackOutEvtIllegal( GMEVENT* event, int* seq, void* work );

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateRival(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_RIVAL, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateSupport(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_SUPPORT, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym01A(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM1A, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym01B(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM1B, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym01C(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM1C, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym02(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM2, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym03(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM3, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym04(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM4, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym05(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM5, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym06(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM6, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym07(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM7, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym08A(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM8A, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateGym08B(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_GYM8B, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateBigFour1(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_BIGFOUR1, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateBigFour2(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_BIGFOUR2, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateBigFour3(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_BIGFOUR3, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateBigFour4(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_BIGFOUR4, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateChamp(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_CHAMP, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateBoss(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_BOSS, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateSage(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_SAGE, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreatePlasma(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_PLASMA, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreatePackage(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_PACKAGE, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateMovePoke(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_MOVEPOKE, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateThree(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_THREE, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateCave(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_CAVE, inIsFadeWhite );
  return( event );
}

//--------------------------------------------------------------
/**
 * イベント作成
 * @param gsys  GAMESYS_WORK
 * @param fieldWork FIELDMAP_WORK
 * @param   inIsFadeWhite エフェクト終了はホワイトアウトか？
 * @retval GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT *ENCEFF_CI_CreateDesert(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const BOOL inIsFadeWhite)
{
  GMEVENT *event;
  event = CreateEffCommon( gsys, fieldWork, ENC_CUTIN_DESERT, inIsFadeWhite );
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
static GMEVENT *CreateEffCommon(  GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, const int inType, const BOOL inIsFadeWhite )
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
  work->Count = 0;
  work->Type = inType;
  work->IsWhiteFade = inIsFadeWhite;

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
        GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, FLASH_SPEED );
    (*seq)++;
  case 1:
    if( GFL_FADE_CheckFade() == FALSE ){
      GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 16, 0, FLASH_SPEED );
      (*seq)++;
    }
    break;
  case 2:
    if( GFL_FADE_CheckFade() == FALSE ){
      work->Count++;
      
      if( work->Count < FLASH_COUNT ){
        GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN, 0, 16, FLASH_SPEED );
        (*seq) = 1;
      }
      else
      {
        GMEVENT *call_event;
        FLD3D_CI_PTR cutin_ptr;
        cutin_ptr = FIELDMAP_GetFld3dCiPtr(fieldmap);
        //カットインイベントコール
        call_event = FLD3D_CI_CreateEncCutInEvt( gsys, cutin_ptr, work->Type, work->IsWhiteFade );
        if (call_event != NULL)
        {
          //イベントコール
          GMEVENT_CallEvent( event, call_event );
        }
        else    //カットインが呼べなかった場合の例外処理
        {
          call_event = GMEVENT_Create(gsys, event, BlackOutEvtIllegal, 0);
          GMEVENT_CallEvent( event, call_event );
        }
				(*seq)++;
      }
    }
    break;
  case 3:
		
    return( GMEVENT_RES_FINISH );

  }
  
  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------------------------------------
/**
 * カットインが呼べなかった場合の例外ブラックインイベント
 *
 * @param   event       イベントポインタ
 * @param   *seq        シーケンサ
 * @param   work        ワークポインタ
 *
 * @return	GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT BlackOutEvtIllegal( GMEVENT* event, int* seq, void* work )
{
  GAMESYS_WORK *gsys;
  FIELDMAP_WORK * fieldmap;
  gsys = GMEVENT_GetGameSysWork(event);
  fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  switch(*seq){
  case 0:
    GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 0, 16, 3 );
    (*seq)++;
    break;
  case 1:
    if ( GFL_FADE_CheckFade() == FALSE )
    {
      return GMEVENT_RES_FINISH;
    }
  }

  return GMEVENT_RES_CONTINUE;
}



