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
 * @brief 部屋のOBJ数を返す
 * @note    ゾーンチェンジ時にスクリプトからコールされる
 * @param * work            国連ワークポインタ
 * @retval  u32     OBJ数 0～5
 */
//--------------------------------------------------------------
u32 UN_GetRoomObjNum(UNSV_WORK* work)
{
  return work->ObjNum;
}

