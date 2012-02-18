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
#define EVT_MASIC_NUM_FMISSION_00    (20111110)    //フェスミッション配信
#define EVT_MASIC_NUM_FMISSION_01    (30111111)    //フェスミッション配信
#define EVT_MASIC_NUM_FMISSION_02    (10116112)    //フェスミッション配信
#define EVT_MASIC_NUM_FMISSION_03    (20112113)    //フェスミッション配信
#define EVT_MASIC_NUM_UTUSIKAGAMI    (70141519)    //うつしかがみ
#define EVT_MASIC_NUM_RANDOROSU      (24151171)    //ランドロスAR

//マジックナンバーテーブル
const u32 EvtMasicNumTbl[EVT_MAX] = 
{
  EVT_MASIC_NUM_LOCKCAPSULE,
  EVT_MASIC_NUM_VICTYTICKET,
  EVT_MASIC_NUM_FMISSION_00,
  EVT_MASIC_NUM_FMISSION_01,
  EVT_MASIC_NUM_FMISSION_02,
  EVT_MASIC_NUM_FMISSION_03,
  EVT_MASIC_NUM_UTUSIKAGAMI,
  EVT_MASIC_NUM_RANDOROSU,    
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

#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *	@brief  イベントロッククリア
 *
 *	@param	MISC * misc ミスク
 *	@param	int inNo  ナンバー
 *	@param	*my 
 */
//-----------------------------------------------------------------------------
void EVTLOCK_ClearEvtLock( MISC * misc, const int inNo, MYSTATUS *my )
{
  //セーブデータにセット
  MISC_SetEvtRockValue( misc, inNo, 0 );
}
#endif
