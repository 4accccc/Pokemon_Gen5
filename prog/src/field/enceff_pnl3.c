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

#define BUFF_SIZE (96)
static u16 BuffIdx[BUFF_SIZE];
static u16 BuffIdxNext[BUFF_SIZE];
static int BuffCount = 0;
static int BuffCountNext = 0;


static GMEVENT *CreateEffMainEvt(GAMESYS_WORK *gsys);

static BOOL MoveFunc(PANEL_WK *panel);
static void StartFunc(PNL_EFF_WORK *work);

static BOOL CheckEntry(const int inIdx);
static void EntryCore(PNL_EFF_WORK *work, const int inX, const int inZ);
static void Entry(PNL_EFF_WORK *work, const int inIdx);
static void ReferenceEntry(PNL_EFF_WORK *work);


GMEVENT *ENCEFF_PNL3_Create(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork)
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

  param.CharX = 1;
  param.CharY = 1;
  param.StartFunc = StartFunc;
  param.MoveFunc = MoveFunc;

  event = ENCEFF_PNL_CreateEffMainEvt(gsys, &param);

  BuffCountNext = 0;
  BuffCount = 0;
  
  {
    int x,y;
    int idx;
    x = 5;
    y = 5;
    idx = 32*y+x;
    BuffIdxNext[0] = idx;
    BuffCountNext++;

    x = 10;
    y = 18;
    idx = 32*y+x;
    BuffIdxNext[1] = idx;
    BuffCountNext++;

    x = 22;
    y = 10;
    idx = 32*y+x;
    BuffIdxNext[2] = idx;
    BuffCountNext++;
  }

  return event;
}

static void _StartFunc(PNL_EFF_WORK *work)
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

static void StartFunc(PNL_EFF_WORK *work)
{
  MOVE_START_PRM *prm = &work->StartPrm;

  if ( prm->Count < work->PanelNumW * work->PanelNumH)
  {
    if (prm->Wait == 0)
    {
      int i;
      for (i=0;i<BuffCount;i++)
      {
        Entry(work, BuffIdx[i]);
      }
      ReferenceEntry(work);

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
//  if ( panel->Count < 32 )
  if ( panel->Pos.x < FX32_ONE*256 )
  {
    panel->Pos.z = FX32_ONE;
    panel->Pos.x += panel->Spd.x;
    panel->Spd.x += FX32_ONE;
/**    
    panel->Rot.z += 0x1000;
    if ( panel->Rot.z >= 0x10000){
      panel->Rot.z = panel->Rot.z-0x10000;
    }
*/
    if ( panel->Rot.y < 0x8000){
      panel->Rot.y += 0x200;
    }
    else panel->Rot.y = 0x8000;

//    panel->Count++;
  }
  else panel->MoveEndFlg = TRUE;

  return panel->MoveEndFlg;
}

static BOOL CheckEntry(const int inIdx)
{
  int i;
  for(i=0;i<BuffCountNext;i++)
  {
    if ( BuffIdxNext[i] == inIdx ) return TRUE;
  }

  return FALSE;
}

static void EntryCore(PNL_EFF_WORK *work, const int inX, const int inZ)
{
  int idx = inZ * work->PanelNumW + inX;
  PANEL_WK *panel = &work->Panel[idx];
  if ( !panel->MoveOnFlg )
  {
    GF_ASSERT(BuffCountNext<BUFF_SIZE);
    if ( !CheckEntry(idx) )
    {
      BuffIdxNext[BuffCountNext] = idx;
      BuffCountNext++;
    }
  }
}

//指定パネルの周囲のパネルをバッファに登録
static void Entry(PNL_EFF_WORK *work, const int inIdx)
{
  int base_x, base_z, x, z;
  base_x = inIdx % work->PanelNumW;
  base_z = inIdx / work->PanelNumW;

  //上
  {
    x = base_x;
    z = base_z-1;
    if (z >= 0) EntryCore(work, x, z);
  }
  //右
  {
    x = base_x+1;
    z = base_z;
    if (x < work->PanelNumW) EntryCore(work, x, z);
  }
  //下
  {
    x = base_x;
    z = base_z+1;
    if (z < work->PanelNumH) EntryCore(work, x, z);
  }
  //左
  {
    x = base_x-1;
    z = base_z;
    if (x >= 0) EntryCore(work, x, z);
  }
}

//エントリを反映
static void ReferenceEntry(PNL_EFF_WORK *work)
{
  int i;
  MOVE_START_PRM *prm = &work->StartPrm;

  for (i=0;i<BuffCountNext;i++)
  {
    PANEL_WK *panel;
    int idx;
    idx = BuffIdxNext[i];
    panel = &work->Panel[idx];

    if ( panel->MoveOnFlg ) continue;
    
    panel->MoveOnFlg = TRUE;
    prm->Count++;
    //初期スピード
    panel->Spd.x = FX32_ONE;

    BuffIdx[i] = BuffIdxNext[i];
  }

  BuffCount = BuffCountNext;
  BuffCountNext = 0;
}


