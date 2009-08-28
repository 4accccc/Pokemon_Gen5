//======================================================================
/**
 * @file	fld_exp_obj.c
 * @brief	フィールド3Ｄ拡張オブジェクト
 * @author	Saito
 * @data	08.08.27
 *
 */
//======================================================================

#include "fld_exp_obj.h"

#define FLD_EXP_OBJ_UNIT_MAX  (4)

typedef struct EXP_UNIT_tag
{
  BOOL Valid;
  GFL_G3D_OBJSTATUS *ObjStatus;
  u16 UtilUnitIdx;
  u16 pading;
}EXP_UNIT;

typedef struct FLD_EXP_OBJ_CNT_tag
{
  int HeapID;
  GFL_G3D_UTIL* ObjUtil;
///  GFL_G3D_OBJSTATUS ObjStatus[FLD_EXP_OBJ_UNIT_MAX];
///  BOOL Valid[FLD_EXP_OBJ_UNIT_MAX];
  EXP_UNIT  Unit[FLD_EXP_OBJ_UNIT_MAX];

}FLD_EXP_OBJ_CNT;


//--------------------------------------------------------------------------------------------
/**
 * モジュール作成
 *
 * @param   inResCountMax			設定リソース最大数定
 * @param   inObjCountMax			設定オブジェクト最大数
 * @param   inHeapID				ヒープＩＤ
 *
 * @return	FLD_EXP_OBJ_CNT_PTR   モジュールポインタ
 */
//--------------------------------------------------------------------------------------------
FLD_EXP_OBJ_CNT_PTR FLD_EXP_OBJ_Create( const int inResCountMax,
                                        const int inObjCountMax,
                                        const int inHeapID )
{
  FLD_EXP_OBJ_CNT_PTR ptr;
  ptr = GFL_HEAP_AllocClearMemory(inHeapID, sizeof(FLD_EXP_OBJ_CNT));
  ptr->ObjUtil = GFL_G3D_UTIL_Create( inResCountMax, inObjCountMax, inHeapID );

  return ptr;
}

//--------------------------------------------------------------------------------------------
/**
 * モジュール破棄
 *
 * @param   ptr     モジュールポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD_EXP_OBJ_Delete(FLD_EXP_OBJ_CNT_PTR ptr)
{
  u16 i;
  //解放忘れがあるかもしれないので、ユニット廃棄もしておく
  for (i=0;i<FLD_EXP_OBJ_UNIT_MAX;i++){
    if ( ptr->Unit[i].Valid){
      FLD_EXP_OBJ_DelUnit( ptr, i );
    }
  }
  GFL_G3D_UTIL_Delete( ptr->ObjUtil );
  GFL_HEAP_FreeMemory( ptr );
}

//--------------------------------------------------------------------------------------------
/**
 * ユニット追加
 *
 * @param	ptr     モジュールポインタ
 * @param	inSetup		設定データ
 * @param inIndex   登録するインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD_EXP_OBJ_AddUnit(  FLD_EXP_OBJ_CNT_PTR ptr,
                          const GFL_G3D_UTIL_SETUP *inSetup,
                          const u16 inIndex)
{
  u16 i;
  u16 unitIdx;
  GFL_G3D_OBJSTATUS *status;
  u16 obj_num;

  if ( ptr->Unit[inIndex].Valid == TRUE ){
    GF_ASSERT_MSG(0,"CAN NOT ADD UNIT\n");
  }

  unitIdx = GFL_G3D_UTIL_AddUnit( ptr->ObjUtil, inSetup );
  obj_num = inSetup->objCount;
  if (obj_num == 0){
    GF_ASSERT_MSG(0,"OBJ NOTHING\n");
  }

  status = GFL_HEAP_AllocClearMemory(ptr->HeapID, sizeof(GFL_G3D_OBJSTATUS)*obj_num);
  ptr->Unit[inIndex].ObjStatus = status;
  for(i=0;i<obj_num;i++){
    VEC_Set( &status[i].scale, FX32_ONE, FX32_ONE, FX32_ONE );
	  MTX_Identity33( &status[i].rotate );
    VEC_Set( &status[i].trans, 0, 0, 0 );
  }
  ptr->Unit[inIndex].Valid = TRUE;
  ptr->Unit[inIndex].UtilUnitIdx = unitIdx;
}

//--------------------------------------------------------------------------------------------
/**
 * ユニット破棄
 *
 * @param	ptr         モジュールポインタ
 * @param	inUnitIdx   ユニットインデックス 
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD_EXP_OBJ_DelUnit( FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx )
{
  if (ptr->Unit[inUnitIdx].Valid == FALSE){
    GF_ASSERT_MSG(0,"%d UNIT INVALID\n",inUnitIdx);
    return;
  }
  if (inUnitIdx >= FLD_EXP_OBJ_UNIT_MAX){
    GF_ASSERT_MSG(0,"%d UNIT IDX OVER\n",inUnitIdx);
    return;
  }

  GFL_HEAP_FreeMemory( ptr->Unit[inUnitIdx].ObjStatus );
  GFL_G3D_UTIL_DelUnit( ptr->ObjUtil, ptr->Unit[inUnitIdx].UtilUnitIdx );

  ptr->Unit[inUnitIdx].Valid = FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 *  ユーティリティポインタ取得 
 *
 * @param	ptr               モジュールポインタ
 *
 * @return	GFL_G3D_UTIL*   ユーティリティポインタ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_UTIL *FLD_EXP_OBJ_GetUtil(FLD_EXP_OBJ_CNT_PTR ptr)
{
  GF_ASSERT(ptr != NULL);
  return ptr->ObjUtil;
}

//--------------------------------------------------------------------------------------------
/**
 *  ユーティリティが管理するユニットインデックスを取得
 *
 * @param	ptr               モジュールポインタ
 * @param inUnitIdx         モジュールが管理するユニットのインデックス
 *
 * @return	u16   ユーティリティの持つユニットのインデックス
 */
//--------------------------------------------------------------------------------------------
u16 FLD_EXP_OBJ_GetUtilUnitIdx(FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx)
{
  GF_ASSERT(ptr != NULL);
  return ptr->Unit[inUnitIdx].UtilUnitIdx;
}

//--------------------------------------------------------------------------------------------
/**
 *  ユニットステータスポインタ取得
 *
 * @param   ptr               モジュールポインタ
 * @param   inUnitIdx         ユニットインデックス
 * @param   inObjIdx          ユニット内ＯＢＪインデックス
 *
 * @return	GFL_G3D_OBJSTATUS*    ステータスポインタ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_OBJSTATUS *FLD_EXP_OBJ_GetUnitObjStatus(FLD_EXP_OBJ_CNT_PTR ptr,
                                                const u16 inUnitIdx,
                                                const u16 inObjIdx)
{
  GF_ASSERT(ptr != NULL);
  if (ptr->Unit[inUnitIdx].Valid == FALSE){
    GF_ASSERT_MSG(0,"%d UNIT INVALID\n",inUnitIdx);
    return NULL;
  }
  if (inUnitIdx >= FLD_EXP_OBJ_UNIT_MAX){
    GF_ASSERT_MSG(0,"%d UNIT IDX OVER\n",inUnitIdx);
    return NULL;
  }
  return &ptr->Unit[inUnitIdx].ObjStatus[inObjIdx];
}

//--------------------------------------------------------------------------------------------
/**
 *  ユニット描画
 *
 * @param   ptr               モジュールポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void FLD_EXP_OBJ_Draw( FLD_EXP_OBJ_CNT_PTR ptr )
{
  u8 i,j;
  //ユニット数分ループ
  for (i=0;i<FLD_EXP_OBJ_UNIT_MAX;i++){
    if (ptr->Unit[i].Valid == TRUE){
      u16 obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( ptr->ObjUtil, ptr->Unit[i].UtilUnitIdx );
      u16 obj_count = GFL_G3D_UTIL_GetUnitObjCount( ptr->ObjUtil, ptr->Unit[i].UtilUnitIdx );
      for (j=0;j<obj_count;j++){
        GFL_G3D_OBJ* pObj;
        pObj = GFL_G3D_UTIL_GetObjHandle(ptr->ObjUtil, obj_idx+j);
        GFL_G3D_DRAW_DrawObject( pObj, &ptr->Unit[i].ObjStatus[j] );
      }
    }
  }
}

