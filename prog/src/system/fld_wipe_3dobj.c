//============================================================================================
/**
 * @file	fld_wip_3dobj.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "system/gfl_use.h"
#include "arc_def.h"
#include "system/main.h"

#include "arc/shadow_test.naix"

#include "system/fld_wipe_3dobj.h"
//============================================================================================
/**
 *
 * @brief
 *
 */
//============================================================================================
struct _FLD_WIPEOBJ {
	GFL_G3D_RES*	g3Dres;
	GFL_G3D_OBJ*	g3Dobj;
};

#if 0
static void	drawObject(GFL_G3D_OBJ* g3Dobj, GFL_G3D_OBJSTATUS* pStatus);
static void	rotateCalc(MtxFx33* pRotate, u16 offs);
#endif
//-------------------------------------------------------------------------------------
FLD_WIPEOBJ* FLD_WIPEOBJ_Create( HEAPID heapID ) 
{
	FLD_WIPEOBJ* fw = GFL_HEAP_AllocClearMemory(heapID, sizeof(FLD_WIPEOBJ));

	//３Ｄオブジェクト作成
	fw->g3Dres = GFL_G3D_CreateResourceArc(ARCID_SHADOW_TEST, NARC_shadow_test_shadow4_test_nsbmd);
	fw->g3Dobj = GFL_G3D_OBJECT_Create(GFL_G3D_RENDER_Create(fw->g3Dres, 0, NULL), NULL, 0); 

	return fw;
}

//-------------------------------------------------------------------------------------
void FLD_WIPEOBJ_Delete( FLD_WIPEOBJ* fw )
{
	GFL_G3D_RENDER_Delete(GFL_G3D_OBJECT_GetG3Drnd(fw->g3Dobj)); 
	GFL_G3D_OBJECT_Delete(fw->g3Dobj); 
	GFL_G3D_DeleteResource(fw->g3Dres);

	GFL_HEAP_FreeMemory(fw);
}

//-------------------------------------------------------------------------------------
void FLD_WIPEOBJ_Main( FLD_WIPEOBJ* fw, fx32 scale )
{
	GFL_G3D_OBJSTATUS status;
	fx32	length = FX32_ONE*4;

	GF_ASSERT(fw);

	if( scale == 0 ){ return; }

	{
		GFL_G3D_LOOKAT		lookAt;
		VecFx32						vec;
		fx32							ysin;

		GFL_G3D_GetSystemLookAt(&lookAt);
		//status.trans = lookAt.target;
		status.trans = lookAt.camPos;

		VEC_Subtract(&lookAt.target, &lookAt.camPos, &vec);
		VEC_Normalize(&vec, &vec);
		ysin = (vec.y > 0)? vec.y : -vec.y; 
		VEC_Set(&status.scale, scale, FX_Mul(scale,ysin), length);
	}
	MTX_Copy43To33(NNS_G3dGlbGetInvCameraMtx(), &status.rotate);
	{
		NNSG3dResMdl*	pMdl = NNS_G3dRenderObjGetResMdl
						(GFL_G3D_RENDER_GetRenderObj(GFL_G3D_OBJECT_GetG3Drnd(fw->g3Dobj)) );

		NNS_G3dMdlSetMdlLightEnableFlag(pMdl, 0, 0);
		NNS_G3dMdlSetMdlPolygonID(pMdl, 0, 63);
		NNS_G3dMdlSetMdlCullMode(pMdl, 0, GX_CULL_BACK); 
		NNS_G3dMdlSetMdlAlpha(pMdl, 0, 20);
		NNS_G3dMdlSetMdlPolygonMode(pMdl, 0, GX_POLYGONMODE_MODULATE);
	}
	GFL_G3D_DRAW_DrawObject(fw->g3Dobj, &status);
}

#if 0
//--------------------------------------------------------------
static void	rotateCalc(MtxFx33* pRotate, u16 offs)
{
	VecFx32	vecRot;
	MtxFx33 tmpMtx;

	vecRot.x = 0;
	vecRot.y = offs;
	vecRot.z = 0;

	MTX_RotX33(pRotate, FX_SinIdx((u16)vecRot.x), FX_CosIdx((u16)vecRot.x));

	MTX_RotY33(&tmpMtx, FX_SinIdx((u16)vecRot.y), FX_CosIdx((u16)vecRot.y)); 
	MTX_Concat33(pRotate, &tmpMtx, pRotate); 

	MTX_RotZ33(&tmpMtx, FX_SinIdx((u16)vecRot.z), FX_CosIdx((u16)vecRot.z)); 
	MTX_Concat33(pRotate, &tmpMtx, pRotate); 

	MTX_Copy43To33(NNS_G3dGlbGetInvCameraMtx(), &tmpMtx);
	MTX_Concat33(pRotate, &tmpMtx, pRotate); 
}

//--------------------------------------------------------------
static void	drawObject(GFL_G3D_OBJ* g3Dobj, GFL_G3D_OBJSTATUS* pStatus)
{
	NNSG3dRenderObj*	pRnd = GFL_G3D_RENDER_GetRenderObj(GFL_G3D_OBJECT_GetG3Drnd(g3Dobj));
	NNSG3dResMdl*			pMdl = NNS_G3dRenderObjGetResMdl(pRnd);

	NNS_G3dMdlSetMdlLightEnableFlag( pMdl, 0, 0x0 );
	NNS_G3dMdlSetMdlPolygonID( pMdl, 0, 0 );
	NNS_G3dMdlSetMdlCullMode( pMdl, 0, GX_CULL_FRONT ); 
	NNS_G3dMdlSetMdlAlpha( pMdl, 0, 16 );
	NNS_G3dMdlSetMdlPolygonMode( pMdl, 0, GX_POLYGONMODE_SHADOW );
	GFL_G3D_DRAW_DrawObject(g3Dobj, pStatus);

	NNS_G3dMdlSetMdlLightEnableFlag( pMdl, 0, 0x0 );
	NNS_G3dMdlSetMdlPolygonID( pMdl, 0, 1 );
	NNS_G3dMdlSetMdlCullMode( pMdl, 0, GX_CULL_BACK ); 
	NNS_G3dMdlSetMdlAlpha( pMdl, 0, 16 );
	NNS_G3dMdlSetMdlPolygonMode( pMdl, 0, GX_POLYGONMODE_SHADOW );
	GFL_G3D_DRAW_DrawObject(g3Dobj, pStatus);
}
#endif


