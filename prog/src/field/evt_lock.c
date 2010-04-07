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

#define EVT_MASIC_NUM_LOCKCAPSULE    (20100325)    //ロックカプセル
#define EVT_MASIC_NUM_VICTYTICKET    (20100406)    //ビクティのチケット

//マジックナンバーテーブル
const u32 EvtMasicNumTbl[EVT_MAX] = 
{
  EVT_MASIC_NUM_LOCKCAPSULE,
  EVT_MASIC_NUM_VICTYTICKET,
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
void EVTLOCK_SetEvtLock( MISC * misc, const int inNo, MYSTATUS *my )
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
BOOL EVTLOCK_CheckEvtLock( MISC * misc, const int inNo, MYSTATUS *my )
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


