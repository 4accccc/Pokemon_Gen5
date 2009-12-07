//======================================================================
/**
 * @file  pleasure_boat_st.c
 * @brief   遊覧船常駐部
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "pleasure_boat.h"
#include "pleasure_boat_def.h"

#include "arc/fieldmap/fldmmdl_objcode.h"  //for OBJCODE

//--------------------------------------------------------------
/**
 * @brief　部屋の人物情報を取得　ＯＢＪコード
 * @note    ゾーンチェンジ時にスクリプトからコールされる
 * @param   work      PL_BOAT_WORK_PTR
 * @apram   inRoomIdx 部屋番号
 * @retval  int       ＯＢＪコード
*/
//--------------------------------------------------------------
int PL_BOAT_GetObjCode(PL_BOAT_WORK_PTR work, const int inRoomIdx)
{
  if ( work==NULL ) return BOY1;
  return work->RoomParam[inRoomIdx].ObjCode;
}

//--------------------------------------------------------------
/**
 * @brief　部屋の人物がダブルバトルするかをチェック
 * @param   work      PL_BOAT_WORK_PTR
 * @apram   inRoomIdx 部屋番号
 * @retval  BOOL       TRUEでダブルバトル
*/
//--------------------------------------------------------------
BOOL PL_BOAT_CheckDblBtl(PL_BOAT_WORK_PTR work, const int inRoomIdx)
{
  BOOL rc = FALSE;

  if ( work==NULL ) return rc;

  if ( (inRoomIdx == 2)&&(work->DblBtl[0]) ) rc = TRUE;
  else if ( (inRoomIdx == 7)&&(work->DblBtl[1]) ) rc = TRUE;
    
  return rc;
}

