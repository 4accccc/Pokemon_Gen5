//======================================================================
/**
 * @file	fld_exp_obj.h
 * @brief	フィールド3Ｄ拡張オブジェクト
 * @author	Saito
 * @date	08.08.27
 *
 */
//======================================================================
#pragma once

#include <gflib.h>

typedef struct FLD_EXP_OBJ_CNT_tag * FLD_EXP_OBJ_CNT_PTR;
typedef struct EXP_OBJ_ANM_CNT_tag * EXP_OBJ_ANM_CNT_PTR;

extern FLD_EXP_OBJ_CNT_PTR FLD_EXP_OBJ_Create
          ( const int inResCountMax,const int inObjCountMax,  const int inHeapID );
extern void FLD_EXP_OBJ_Delete(FLD_EXP_OBJ_CNT_PTR ptr);
extern void FLD_EXP_OBJ_AddUnit
          (  FLD_EXP_OBJ_CNT_PTR ptr,const GFL_G3D_UTIL_SETUP *inSetup, const u16 inIndex );
extern void FLD_EXP_OBJ_DelUnit( FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx );
extern GFL_G3D_UTIL *FLD_EXP_OBJ_GetUtil(FLD_EXP_OBJ_CNT_PTR ptr);
extern u16 FLD_EXP_OBJ_GetUtilUnitIdx(FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx);
extern GFL_G3D_OBJSTATUS *FLD_EXP_OBJ_GetUnitObjStatus
          (FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx, const u16 inObjIdx);
extern void FLD_EXP_OBJ_SetCulling
          (FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx, const u16 inObjIdx, const BOOL inCullingFlg);
extern GFL_G3D_OBJ *FLD_EXP_OBJ_GetUnitObj
          (FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx, const u16 inObjIdx);
extern void FLD_EXP_OBJ_Draw( FLD_EXP_OBJ_CNT_PTR ptr );
extern void FLD_EXP_OBJ_PlayAnime( FLD_EXP_OBJ_CNT_PTR ptr );

extern EXP_OBJ_ANM_CNT_PTR FLD_EXP_OBJ_GetAnmCnt(
    FLD_EXP_OBJ_CNT_PTR ptr, const u16 inUnitIdx, const u16 inObjIdx, const u16 inAnmIdx);
extern void FLD_EXP_OBJ_ChgAnmStopFlg(EXP_OBJ_ANM_CNT_PTR ptr, const u8 inStop);
extern u8 FLD_EXP_OBJ_GetAnmStopFlg(EXP_OBJ_ANM_CNT_PTR ptr);


extern void FLD_EXP_OBJ_ChgAnmLoopFlg(EXP_OBJ_ANM_CNT_PTR ptr, const u8 inLoop);
extern BOOL FLD_EXP_OBJ_ChkAnmEnd(EXP_OBJ_ANM_CNT_PTR ptr);
extern void FLD_EXP_OBJ_SetObjAnmFrm( FLD_EXP_OBJ_CNT_PTR ptr ,
                                      const u16 inUnitIdx,
                                      const u16 inObjIdx,
                                      const u16 inAnmIdx,
                                      const fx32 inFrm  );
extern fx32 FLD_EXP_OBJ_GetObjAnmFrm( FLD_EXP_OBJ_CNT_PTR ptr ,
                                      const u16 inUnitIdx,
                                      const u16 inObjIdx,
                                      const u16 inAnmIdx );
extern void FLD_EXP_OBJ_ValidCntAnm(
                        FLD_EXP_OBJ_CNT_PTR ptr,
                        const u16 inUnitIdx,
                        const u16 inObjIdx,
                        const u16 inAnmIdx,
                        const BOOL inValid);

extern fx32 FLD_EXP_OBJ_GetAnimeLastFrame(EXP_OBJ_ANM_CNT_PTR ptr );

