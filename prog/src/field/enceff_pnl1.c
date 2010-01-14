//======================================================================
/*
 * @file	enceff_pnl1.c
 * @brief	イベント：エンカウントエフェクト
 * @author saito
 */
//======================================================================

#include "enceff.h"
#include "enceff_prg.h"
#include "fieldmap.h"

#include "system/main.h"

#include "enceff_pnl.h"

#define ROT_NUM (1)

static GMEVENT *CreateEffMainEvt(GAMESYS_WORK *gsys);

static BOOL MoveFunc(PANEL_WK *panel);
static void StartFunc(PNL_EFF_WORK *work);


GMEVENT *ENCEFF_PNL1_Create(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
{
  ENCEFF_CNT_PTR eff_cnt_ptr;
  GMEVENT * event;
  VecFx32 pos = {0, 0, FX32_ONE*(264)};
  
  //オーバーレイロード(パネルエフェクトオーバーレイ)
  eff_cnt_ptr = FIELDMAP_GetEncEffCntPtr(fieldWork);
  ENCEFF_LoadPanelEffOverlay(eff_cnt_ptr);

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

  param.CharX = 4;
  param.CharY = 4;
  param.InitFunc = NULL;
  param.StartFunc = StartFunc;
  param.MoveFunc = MoveFunc;

  event = ENCEFF_PNL_CreateEffMainEvt(gsys, &param);

  return event;
}

static void StartFunc(PNL_EFF_WORK *work)
{
  MOVE_START_PRM *prm = &work->StartPrm;

  if ( prm->Count < work->PanelNumW*work->PanelNumH )
  {
    if (prm->Wait == 0)
    {
      PANEL_WK *panel;
      panel = &work->Panel[prm->Count];
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
  if ( panel->Count < ROT_NUM )
  {
    panel->Rot.y += 0x1000;
    if ( panel->Rot.y >= 0x10000 )
    {
      panel->Rot.y -= 0x10000;
      panel->Count++;
    }
  }
  else if (panel->Count == ROT_NUM ){
    panel->Rot.y += 0x1000;
    if ( panel->Rot.y >= 0x8000 )
    {
      panel->Rot.y = 0x8000;
      panel->Count++;
    }
  }
  else panel->MoveEndFlg = TRUE;

  return panel->MoveEndFlg;
}



