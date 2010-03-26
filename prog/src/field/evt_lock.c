//======================================================================
/*
 * @file	evt_lock.c
 * @brief	イベントロック
 * @author saito
 */
//======================================================================
#include <gflib.h>

#include "fieldmap.h"
#include "system/main.h"

#include "evt_lock.h"

#define EVT_MASIC_NUM_ROCKCAPSULE    (20100325)    //ロックカプセル

//マジックナンバーテーブル
const u32 EvtMasicNumTbl[8] = 
{
  EVT_MASIC_NUM_ROCKCAPSULE,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};


//--------------------------------------------------------------
/**
 * ＩＤを指定して、キーを作成して、セーブデータに書き込む
 * @param misc
 * @param inNo
 * @param my
 * @retval none
 */
//--------------------------------------------------------------
void EVTROCK_SetEvtRock( MISC * misc, const int inNo, MYSTATUS *my )
{
  u32 masic;
  u32 key;
  u32 val;

  if (inNo >= EVT_MAX)
  {
    GF_ASSERT_MSG(0,"NO Error");
    return;
  }

  masic = EvtMasicNumTbl[inNo];
  key = MyStatus_GetID(my);
  //ＸＯＲをとる
  val = masic^key;

  //セーブデータにセット
  MISC_SetEvtRockValue( misc, inNo, val );
}

//--------------------------------------------------------------
/**
 * 指定したＩＤのイベントロック値が正規のものかを調べる
 * @param misc
 * @param inNo
 * @param my
 * @retval BOOL
 */
//--------------------------------------------------------------
BOOL EVTROCK_ChekcEvtRock( MISC * misc, const int inNo, MYSTATUS *my )
{
  u32 masic;
  u32 key;
  u32 val;

  if (inNo>=EVT_MAX) return FALSE;

  key = MyStatus_GetID(my);
  //セーブデータ取得
  val = MISC_GetEvtRockValue( misc, inNo );
  masic = val^key;
  if (masic == EvtMasicNumTbl[inNo]) return TRUE;
  
  return FALSE;
}


