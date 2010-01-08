//======================================================================
/*
 * @file	enceff_pnl3.c
 * @brief	イベント：エンカウントエフェクト
 * @author saito
 */
//======================================================================

#include "enceff.h"
#include "enceff_prg.h"
#include "fieldmap.h"

#include "system/main.h"

#include "enceff_pnl.h"

static GMEVENT *CreateEffMainEvt(GAMESYS_WORK *gsys);

static BOOL MoveFunc(PANEL_WK *panel);
static void StartFunc(PNL_EFF_WORK *work);


GMEVENT *ENCEFF_PNL3_Create(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  GMEVENT * event;
  VecFx32 pos = {0, 0, FX32_ONE*(264)};
  //オーバーレイロード
  ;
  event = ENCEFF_PRG_Create( gsys, &pos, CreateEffMainEvt, ENCEFF_PNL_DrawMesh );
  
  return event;
}

//--------------------------------------------------------------------------------------------
/**
 * イベント作成
 *
 * @param   gsys        ゲームシステムポインタ
 *
 * @return	event       イベントポインタ
 */
//--------------------------------------------------------------------------------------------
static GMEVENT *CreateEffMainEvt(GAMESYS_WORK *gsys)
{
  GMEVENT * event;
  PNL_EFF_PARAM param;

  param.CharX = 1;
  param.CharY = 1;
  param.StartFunc = StartFunc;
  param.MoveFunc = MoveFunc;

  event = ENCEFF_PNL_CreateEffMainEvt(gsys, &param);

  return event;
}

static void StartFunc(PNL_EFF_WORK *work)
{
  MOVE_START_PRM *prm = &work->StartPrm;

  if ( prm->Count < work->PanelNumW * work->PanelNumH)
  {
    if (prm->Wait == 0)
    {
      int idx = ((prm->Count%work->PanelNumH)*work->PanelNumW) + (prm->Count/work->PanelNumH);
      PANEL_WK *panel = &work->Panel[idx];
      panel->MoveOnFlg = TRUE;
      prm->Count++;
      prm->Wait = 0;
    }
    else
    {
      prm->Wait--;
    }
  }
}

static BOOL MoveFunc(PANEL_WK *panel)
{
  if ( panel->Count < 32 )
  {
    panel->Pos.z = FX32_ONE;
    panel->Pos.x -= FX32_ONE*8;
    panel->Rot.z += 0x1000;
    if ( panel->Rot.z >= 0x10000){
      panel->Rot.z = panel->Rot.z-0x10000;
    }
    panel->Count++;
  }
  else panel->MoveEndFlg = TRUE;

  return panel->MoveEndFlg;
}


