//======================================================================
/**
 * @file  united_nations_st.c
 * @brief   国連常駐部
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "arc/fieldmap/fldmmdl_objcode.h"  //for OBJCODE
#include "savedata/un_savedata.h"
#include "savedata/un_savedata_local.h"
#include "un_roominfo_def.h"

//--------------------------------------------------------------
/**
 * @brief　部屋の人物情報を取得　ＯＢＪコード
 * @note    ゾーンチェンジ時にスクリプトからコールされる
 * @param   work      ワークポインタ
 * @apram   inObjIdx インデックス番号 0～UN_ROOM_OBJ_MAX
 * @retval  u32       ＯＢＪコード
*/
//--------------------------------------------------------------
u32 UN_GetRoomObjCode(UNSV_WORK* work, const u32 inObjIdx)
{
  if (inObjIdx >= work->ObjNum)
  {
    if (inObjIdx >= UN_ROOM_OBJ_MAX)
    {
      GF_ASSERT_MSG(0,"idx error idx=%d",inObjIdx);
      return BOY2;
    }
    //定員数以内であれば、表示はしないはずだが、定数を返しておく
    return BOY2;
  }
  
  return work->UnObjCode[inObjIdx];
}

//--------------------------------------------------------------
/**
 * @brief　部屋情報取得
 * @note  　ゾーンチェンジ時にスクリプトからコールで部屋のお客の数を取得するので常駐に置く
 * @note    取得できる情報は、客数、部屋の国コード、フロア
 * @param   work      ワークポインタ
 * @param   inType    取得情報タイプ  un_roominfo_def.h 参照
 * @retval  u32       取得結果
*/
//--------------------------------------------------------------
u32 UN_GetRoomInfo(UNSV_WORK* work, const u32 inType)
{
  u32 val;
  switch(inType){
  case UN_ROOMINFO_TYPE_OBJNUM:
    val = work->ObjNum;
    break;
  case UN_ROOMINFO_TYPE_COUNTRY:
    val = work->TargetCountryCode;
    break;
  case UN_ROOMINFO_TYPE_FLOOR:
    val = work->Floor;
    break;
  default:
    GF_ASSERT_MSG(0,"info code error %d",inType);
    val = 0;
  }
  return val;
}
