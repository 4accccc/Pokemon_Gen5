//======================================================================
/**
 * @file	field_g3dobj.h
 * @brief フィールド
 * @author
 * @date
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "system/gfl_use.h"
#include "arc_def.h"

//======================================================================
//  define 
//======================================================================
#define FLD_G3DOBJ_IDX_ERROR (0xffff)
#define FLD_G3DOBJ_ANM_MAX (8)

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// インデックス型
//--------------------------------------------------------------
typedef u16 FLD_G3DOBJ_RESIDX; ///<リソースインデックス
typedef u16 FLD_G3DOBJ_OBJIDX; ///<オブジェクトインデックス

//--------------------------------------------------------------
/// FLD_G3DOBJ_CTRL
//--------------------------------------------------------------
typedef struct _TAG_FLD_G3DOBJ_CTRL FLD_G3DOBJ_CTRL;

//--------------------------------------------------------------
/// FLD_G3DOBJ_RES_HEADER
//--------------------------------------------------------------
typedef struct
{
  ARCHANDLE *arcHandleMdl;
  ARCHANDLE *arcHandleTex; //arcHandleMdl内テクスチャ参照の際はNULL
  ARCHANDLE *arcHandleAnm;
  u16 arcIdxMdl;
  u16 arcIdxTex;
  u16 arcIdxAnmTbl[FLD_G3DOBJ_ANM_MAX];
  u8 anmCount;
  u8 padding[3]; //byte rest
}FLD_G3DOBJ_RES_HEADER;
  
//======================================================================
//  extern
//======================================================================
extern FLD_G3DOBJ_CTRL * FLD_G3DOBJ_CTRL_Create(
    HEAPID heapID, u16 res_max, u16 obj_max );
extern void FLD_G3DOBJ_CTRL_Delete( FLD_G3DOBJ_CTRL *ctrl );
extern void FLD_G3DOBJ_CTRL_Trans( FLD_G3DOBJ_CTRL *ctrl );
extern void FLD_G3DOBJ_CTRL_Draw( FLD_G3DOBJ_CTRL *ctrl );
extern u16 FLD_G3DOBJ_CTRL_CreateResource( FLD_G3DOBJ_CTRL *ctrl,
    const FLD_G3DOBJ_RES_HEADER *head, BOOL transFlag );
extern void FLD_G3DOBJ_CTRL_DeleteResource(FLD_G3DOBJ_CTRL *ctrl, u16 idx);
extern u16 FLD_G3DOBJ_CTRL_AddObject(
    FLD_G3DOBJ_CTRL *ctrl, const u16 resIdx, u16 mdlIdx );
extern void FLD_G3DOBJ_CTRL_DeleteObject( FLD_G3DOBJ_CTRL *ctrl, u16 idx );
extern GFL_G3D_OBJ * FLD_G3DOBJ_CTRL_GetObject(
    FLD_G3DOBJ_CTRL *ctrl, u16 idx );
extern GFL_G3D_OBJSTATUS * FLD_G3DOBJ_CTRL_GetObjStatus(
    FLD_G3DOBJ_CTRL *ctrl, u16 idx );
extern void FLD_G3DOBJ_CTRL_SetObjPos(
    FLD_G3DOBJ_CTRL *ctrl, u16 idx, const VecFx32 *pos );
extern void FLD_G3DOBJ_CTRL_SetObjCullingFlag(
    FLD_G3DOBJ_CTRL *ctrl, u16 idx, BOOL flag );
extern void FLD_G3DOBJ_CTRL_SetObjVanishFlag(
    FLD_G3DOBJ_CTRL *ctrl, u16 idx, BOOL flag );
extern BOOL FLD_G3DOBJ_CTRL_AnimeObject(
    FLD_G3DOBJ_CTRL *ctrl, u16 idx, fx32 frame );
extern BOOL FLD_G3DOBJ_CTRL_LoopAnimeObject(
    FLD_G3DOBJ_CTRL *ctrl, u16 idx, fx32 frame );

extern void FLD_G3DOBJ_RES_HEADER_Init( FLD_G3DOBJ_RES_HEADER *head );
extern void FLD_G3DOBJ_RES_HEADER_SetMdl(
    FLD_G3DOBJ_RES_HEADER *head, ARCHANDLE *handle, u16 idx );
extern void FLD_G3DOBJ_RES_HEADER_SetTex(
    FLD_G3DOBJ_RES_HEADER *head, ARCHANDLE *handle, u16 idx );
extern void FLD_G3DOBJ_RES_HEADER_SetAnmArcHandle(
    FLD_G3DOBJ_RES_HEADER *head, ARCHANDLE *handle );
extern void FLD_G3DOBJ_RES_HEADER_SetAnmArcIdx(
    FLD_G3DOBJ_RES_HEADER *head, u16 idx );
