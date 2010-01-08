//======================================================================
/*
 * @file	enceff_pnl2.c
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


GMEVENT *ENCEFF_PNL2_Create(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
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

  param.CharX = 2;
  param.CharY = 2;
  param.StartFunc = StartFunc;
  param.MoveFunc = MoveFunc;

  event = ENCEFF_PNL_CreateEffMainEvt(gsys, &param);

  return event;
}

static void StartFunc(PNL_EFF_WORK *work)
{
  MOVE_START_PRM *prm = &work->StartPrm;

  if ( prm->Count < work->PanelNumW )
  {
    if (prm->Wait == 0)
    {
      int i;
      for(i=0;i<work->PanelNumH;i++)
      {
        PANEL_WK *panel = &work->Panel[(i*work->PanelNumW)+prm->Count];
        panel->MoveOnFlg = TRUE;
      }
      prm->Count++;
      prm->Wait = 1;
    }
    else
    {
      prm->Wait--;
    }
  }
}

static BOOL MoveFunc(PANEL_WK *panel)
{
  if ( panel->Count < 64 )
  {
    panel->Rot.y += 0x200;
    panel->Count++;
  }
  else panel->MoveEndFlg = TRUE;

  return panel->MoveEndFlg;
}


