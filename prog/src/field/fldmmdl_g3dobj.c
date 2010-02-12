//======================================================================
/**
 * @file	fldmmdl_g3dobj.c
 * @brief	動作モデル　G3Dオブジェ管理
 * @author	kagaya
 * @date	05.07.13
 */
//======================================================================
#include "fldmmdl.h"
#include "field_g3dobj.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// RESCODE
//--------------------------------------------------------------
typedef struct
{
  u16 residx;
  u16 code;
}RESCODE;

//--------------------------------------------------------------
/// MMDL_G3DOBJCONT
//--------------------------------------------------------------
struct _TAG_MMDL_G3DOBJCONT
{
  MMDLSYS *mmdlsys;
  FLD_G3DOBJ_CTRL *g3dobj_ctrl;
  
  u16 max;
  RESCODE *restbl;
};

//======================================================================
//  proto
//======================================================================
static void rescode_Init( MMDL_G3DOBJCONT *objcont, HEAPID heapID );
static void rescode_Delete( MMDL_G3DOBJCONT *objcont );
static u16 rescode_RegCodeIdx( MMDL_G3DOBJCONT *objcont, u16 code, u16 idx );
static u16 rescode_GetResIdx( MMDL_G3DOBJCONT *objcont, u16 code );
static u16 rescode_GetOBJCode( MMDL_G3DOBJCONT *objcont, u16 idx );
static u16 rescode_AddResource(
    MMDLSYS *mmdlsys, MMDL_G3DOBJCONT *objcont, const u16 code );
static void rescode_DeleteResource( MMDL_G3DOBJCONT *objcont, u16 code );

//======================================================================
//  動作モデル　G3Dオブジェ管理
//======================================================================
//--------------------------------------------------------------
/**
 * MMDL_G3DOBJCONT G3Dオブジェ管理　セットアップ
 * @param mmdlsys MMDLSYS
 * @param g3dobj_ctrl FLD_G3DOBJ_CTRL
 * @retval nothing
 */
//--------------------------------------------------------------
void MMDL_G3DOBJCONT_Setup( MMDLSYS *mmdlsys, FLD_G3DOBJ_CTRL *g3dobj_ctrl )
{
  HEAPID heapID;
  MMDL_G3DOBJCONT *objcont;
  
  heapID = MMDLSYS_GetHeapID( mmdlsys );
  
  objcont =
    GFL_HEAP_AllocClearMemory( heapID, sizeof(MMDL_G3DOBJCONT) );
  objcont->mmdlsys = mmdlsys;
  objcont->g3dobj_ctrl = g3dobj_ctrl;
  objcont->max = MMDLSYS_GetMMdlMax( mmdlsys );
  
  rescode_Init( objcont, heapID );
  MMDLSYS_SetG3dObjCont( mmdlsys, objcont );
}

//--------------------------------------------------------------
/**
 * MMDL_G3DOBJCONT G3Dオブジェ管理　削除
 * @param mmdlsys MMDLSYS
 * @retval nothing
 */
//--------------------------------------------------------------
void MMDL_G3DOBJCONT_Delete( MMDLSYS *mmdlsys )
{
  MMDL_G3DOBJCONT *objcont;
  objcont = MMDLSYS_GetG3dObjCont( mmdlsys );
  rescode_Delete( objcont );
  GFL_HEAP_FreeMemory( objcont );
}

//--------------------------------------------------------------
/**
 * MMDL_G3DOBJCONT FLD_G3DOBJ_CTRL取得
 * @param mmdlsys MMDLSYS
 * @retval FLD_G3DOBJ_CTRL
 */
//--------------------------------------------------------------
FLD_G3DOBJ_CTRL * MMDL_G3DOBJCONT_GetFldG3dObjCtrl( MMDL *mmdl )
{
  MMDLSYS *mmdlsys;
  MMDL_G3DOBJCONT *objcont;
  mmdlsys = MMDL_GetMMdlSys( mmdl );
  objcont = MMDLSYS_GetG3dObjCont( mmdlsys );
  return( objcont->g3dobj_ctrl );
}

//======================================================================
//  リソース管理
//======================================================================
//--------------------------------------------------------------
/**
 * リソース管理　初期化
 * @param objcont MMDL_G3DOBJCONT
 * @retval nothing
 */
//--------------------------------------------------------------
static void rescode_Init( MMDL_G3DOBJCONT *objcont, HEAPID heapID )
{
  int i;

  objcont->restbl = GFL_HEAP_AllocClearMemory(
      heapID, sizeof(RESCODE)*objcont->max );
  
  for( i = 0; i < objcont->max; i++ ){
    objcont->restbl[i].code = OBJCODEMAX;
  }
}

//--------------------------------------------------------------
/**
 * リソース管理　削除
 * @param objcont MMDL_G3DOBJCONT
 * @retval nothing
 */
//--------------------------------------------------------------
static void rescode_Delete( MMDL_G3DOBJCONT *objcont )
{
  int i;
  RESCODE *tbl = objcont->restbl;
  
  for( i = 0; i < objcont->max; i++, tbl++ ){
    if( tbl->code != OBJCODEMAX ){
      FLD_G3DOBJ_CTRL_DeleteResource( objcont->g3dobj_ctrl, tbl->residx );
    }
  }
  
  GFL_HEAP_FreeMemory( objcont->restbl );
}

//--------------------------------------------------------------
/**
 * リソース管理　登録
 * @param objcont MMDL_G3DOBJCONT
 * @param code
 * @param idx
 * @retval nothing
 */
//--------------------------------------------------------------
static u16 rescode_RegCodeIdx( MMDL_G3DOBJCONT *objcont, u16 code, u16 idx )
{
  int i;
  RESCODE *tbl = objcont->restbl;
  
  for( i = 0; i < objcont->max; i++, tbl++ ){
    if( tbl->code == OBJCODEMAX ){
      tbl->code = code;
      tbl->residx = idx;
      return( i );
    }
  }
  
  GF_ASSERT( 0 );
  return( 0 );
}

//--------------------------------------------------------------
/**
 * リソース管理　指定コードのリソースインデックスを返す
 * @param objcont MMDL_G3DOBJCONT
 * @retval u16 リソースインデックス FLD_G3DOBJ_IDX_ERROR=未登録
 */
//--------------------------------------------------------------
static u16 rescode_GetResIdx( MMDL_G3DOBJCONT *objcont, u16 code )
{
  int i;
  RESCODE *tbl = objcont->restbl;
  
  for( i = 0; i < objcont->max; i++, tbl++ ){
    if( tbl->code == code ){
      return( tbl->residx );
    }
  }
  return( FLD_G3DOBJ_IDX_ERROR );
}

//--------------------------------------------------------------
/**
 * リソース管理　指定インデックスのコードを返す
 * @param objcont MMDL_G3DOBJCONT
 * @retval u16 リソースインデックス FLD_G3DOBJ_IDX_ERROR=未登録
 */
//--------------------------------------------------------------
static u16 rescode_GetOBJCode( MMDL_G3DOBJCONT *objcont, u16 r_idx )
{
  int i;
  RESCODE *tbl = objcont->restbl;
  
  for( i = 0; i < objcont->max; i++, tbl++ ){
    if( tbl->code != OBJCODEMAX ){
      if( tbl->residx == r_idx ){
        return( tbl->code );
      }
    }
  }
  return( OBJCODEMAX );
}

//--------------------------------------------------------------
/**
 * リソース管理　指定コードのリソースを追加
 * @param objcont MMDL_G3D
 * @param code 追加するOBJコード
 * @retval idx リソースインデックス
 */
//--------------------------------------------------------------
static u16 rescode_AddResource(
    MMDLSYS *mmdlsys, MMDL_G3DOBJCONT *objcont, const u16 code )
{
  u16 res_idx;
  const OBJCODE_PARAM *prm;
  const OBJCODE_PARAM_BUF_MDL *prm_mdl;
  FLD_G3DOBJ_RES_HEADER head;
  ARCHANDLE *handle = MMDLSYS_GetResArcHandle( mmdlsys );
  
  prm = MMDLSYS_GetOBJCodeParam( objcont->mmdlsys, code );
  prm_mdl = MMDL_GetOBJCodeParamBufMDL( prm );
  
  FLD_G3DOBJ_RES_HEADER_Init( &head );
  
  if( prm_mdl->res_idx_mdl != 0xffff ){
    FLD_G3DOBJ_RES_HEADER_SetMdl( &head, handle, prm_mdl->res_idx_mdl );
  }
  
  if( prm_mdl->res_idx_tex != 0xffff ){
    FLD_G3DOBJ_RES_HEADER_SetMdl( &head, handle, prm_mdl->res_idx_tex );
  }
  
  if( prm_mdl->res_idx_anm[0] != 0xffff ){
    int i;
    FLD_G3DOBJ_RES_HEADER_SetAnmArcHandle( &head, handle );
    
    for( i = 0; prm_mdl->res_idx_anm[i] != 0xffff; i++ ){
      FLD_G3DOBJ_RES_HEADER_SetAnmArcIdx( &head, prm_mdl->res_idx_anm[i] );
    }
  }
  
  KAGAYA_Printf(
      "動作モデル G3DObject追加 code=%dH, mdl idx = %d, anm count =%xH\n",
      code, head.arcIdxMdl, head.anmCount );

  res_idx = FLD_G3DOBJ_CTRL_CreateResource(
      objcont->g3dobj_ctrl, &head, FALSE );
  rescode_RegCodeIdx( objcont, code, res_idx );
  return( res_idx );
}

//--------------------------------------------------------------
/**
 * リソース管理　指定リソースを削除
 * @param objcont MMDL_G3D
 * @param code 削除するOBJコード
 * @retval nothing
 */
//--------------------------------------------------------------
static void rescode_DeleteResource( MMDL_G3DOBJCONT *objcont, u16 code )
{
  int i;
  RESCODE *tbl = objcont->restbl;
  
  for( i = 0; i < objcont->max; i++, tbl++ ){
    if( tbl->code == code ){
      FLD_G3DOBJ_CTRL_DeleteResource( objcont->g3dobj_ctrl, tbl->residx );
      tbl->code = OBJCODEMAX;
      return;
    }
  }
  
  GF_ASSERT( 0 ); //未登録
}

//======================================================================
//  OBJ関連
//======================================================================
//--------------------------------------------------------------
/**
 * OBJ追加
 * @param mmdl MMDL*
 * @param code 追加するOBJコード
 * @retval u16 追加されたオブジェインデックス
 */
//--------------------------------------------------------------
FLD_G3DOBJ_OBJIDX MMDL_G3DOBJCONT_AddObject( MMDL *mmdl, u16 code )
{
  MMDLSYS *mmdlsys;
  MMDL_G3DOBJCONT *objcont;
  FLD_G3DOBJ_RESIDX residx;
  FLD_G3DOBJ_OBJIDX objidx;
  
  mmdlsys = MMDL_GetMMdlSys( mmdl );
  objcont = MMDLSYS_GetG3dObjCont( mmdlsys );
  residx = rescode_GetResIdx( objcont, code );
  
  if( residx == FLD_G3DOBJ_IDX_ERROR ){
    residx = rescode_AddResource( mmdlsys, objcont, code );
  }
  
  objidx = FLD_G3DOBJ_CTRL_AddObject( objcont->g3dobj_ctrl, residx, 0 );
  return( objidx );
}

//--------------------------------------------------------------
/**
 * OBJ削除
 * @param mmdl MMDL*
 * @param o_idx 削除するオブジェインデックス
 * @retval nothing
 */
//--------------------------------------------------------------
void MMDL_G3DOBJCONT_DeleteObject( MMDL *mmdl, FLD_G3DOBJ_OBJIDX o_idx )
{
  u16 code;
  MMDLSYS *mmdlsys;
  MMDL_G3DOBJCONT *objcont;
  
  mmdlsys = MMDL_GetMMdlSys( mmdl );
  objcont = MMDLSYS_GetG3dObjCont( mmdlsys );
#if 0
  code = rescode_GetOBJCode( objcont, r_idx );
  GF_ASSERT( code != OBJCODEMAX );
#else
  code = MMDL_GetOBJCode( mmdl );
#endif
  
  FLD_G3DOBJ_DeleteObject( objcont->g3dobj_ctrl, o_idx );
  
  if( MMDL_SearchUseOBJCode(mmdl,code) == FALSE ){
    KAGAYA_Printf( "FLDMMDL MDL code(%d) delete resource\n", code );
    rescode_DeleteResource( objcont, code ); //他が未使用ならリソース削除
  }
}
