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
u32 UN_GetObjCode(UNSV_WORK* work, const u32 inObjIdx)
{
  if (inObjIdx >= UN_ROOM_OBJ_MAX)
  {
    GF_ASSERT(0);
    return BOY1;
  }
  if (inObjIdx >= work->ObjNum)
  {
    GF_ASSERT_MSG(0,"idx error idx=%d max=%d",inObjIdx, work->ObjNum);
    return BOY1;
  }
  
  return work->UnObjCode[inObjIdx];
}

