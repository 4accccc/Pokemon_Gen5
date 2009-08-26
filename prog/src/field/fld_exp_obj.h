//======================================================================
/**
 * @file	fld_exp_obj.h
 * @brief	フィールド3Ｄ拡張オブジェクト
 * @author	Saito
 * @data	08.08.27
 *
 */
//======================================================================
#ifndef __FLD_EXP_OBJ_H__
#define __FLD_EXP_OBJ_H__

#include <gflib.h>

typedef struct FLD_EXP_OBJ_CNT_tag * FLD_EXP_OBJ_CNT_PTR;

extern FLD_EXP_OBJ_CNT_PTR FLD_EXP_OBJ_Create
          ( const int inResCountMax,const int inObjCountMax,  const int inHeapID );
extern void FLD_EXP_OBJ_Delete(FLD_EXP_OBJ_CNT_PTR ptr);
extern void FLD_EXP_OBJ_AddUnit
          (  FLD_EXP_OBJ_CNT_PTR ptr,const GFL_G3D_UTIL_SETUP *inSetup,
             const GFL_G3D_OBJSTATUS *inStatus, const u16 inIndex );
extern void FLD_EXP_OBJ_DelUnit( FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx );
extern GFL_G3D_UTIL *FLD_EXP_OBJ_GetUtil(FLD_EXP_OBJ_CNT_PTR ptr);
extern u16 FLD_EXP_OBJ_GetUtilUnitIdx(FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx);
extern GFL_G3D_OBJSTATUS *FLD_EXP_OBJ_GetUnitObjStatus(FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx);
extern void FLD_EXP_OBJ_Draw( FLD_EXP_OBJ_CNT_PTR ptr );

#endif  //__FLD_EXP_OBJ_H__
