//============================================================================================
/**
 * @file	sample4.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "system/gfl_use.h"
#include "arc_def.h"
#include "system/main.h"

#include "debug_common.h"

#include "field/fld_wipe_3dobj.h"
//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	
 *
 *
 *
 *
 *
 */
//============================================================================================
//============================================================================================
/**
 *
 * @brief	構造体定義
 *
 */
//============================================================================================
typedef struct {
	HEAPID							heapID;
	DWS_SYS*						dws;

	int									seq;

	GFL_G3D_UTIL*				g3Dutil;
	u16									g3DutilUnitIdx;
	GFL_G3D_RES*				g3DresMdl0;
	GFL_G3D_OBJ*				g3Dobj0;
	GFL_G3D_RES*				g3DresMdl1;
	GFL_G3D_OBJ*				g3Dobj1;

	FLD_WIPEOBJ*				fldWipeObj;

	int									target;
	int									mode;
	VecFx32							trans;
	fx32								scale;

	int									timer;
}SAMPLE4_WORK;

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	呼び出し用PROC定義
 *
 *
 *
 *
 *
 */
//============================================================================================
static BOOL	sample4(SAMPLE4_WORK* sw);

//------------------------------------------------------------------
/**
 * @brief	proc関数
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT Sample4Proc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SAMPLE4_WORK* sw = NULL;

	sw = GFL_PROC_AllocWork(proc, sizeof(SAMPLE4_WORK), GFL_HEAPID_APP);
  GFL_STD_MemClear(sw, sizeof(SAMPLE4_WORK));
  sw->heapID = GFL_HEAPID_APP;
	sw->seq = 0;

	sw->dws = DWS_SYS_Setup(sw->heapID);
	GFL_CAMADJUST_SetWipeParam(DWS_GetCamAdjust(sw->dws), &sw->scale);

	VEC_Set(&sw->trans, 0, 0, 0);
	//sw->scale = FX32_ONE;
	DWS_SetG3DcamTarget(sw->dws, &sw->trans);

	{
		u16 light0col = GX_RGB(0,0,31);
		GFL_G3D_LIGHT_SetColor(DWS_GetG3Dlight(sw->dws), 0, &light0col);
	}

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT Sample4Proc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SAMPLE4_WORK*	sw;
	sw = mywk;

	DWS_SYS_Framework(sw->dws);

	sw->timer++;
	if(sample4(sw) == TRUE){
		return GFL_PROC_RES_CONTINUE;
	}
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT Sample4Proc_End(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SAMPLE4_WORK*	sw;
	sw = mywk;

	DWS_SYS_Delete(sw->dws);

	GFL_PROC_FreeWork(proc);

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	procテーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA DebugWatanabeSample4ProcData = {
	Sample4Proc_Init,
	Sample4Proc_Main,
	Sample4Proc_End,
};





//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	メイン
 *
 *
 *
 *
 *
 */
//============================================================================================
#define STRBUF_SIZE		(64*2)
#include "arc/shadow_test.naix"

static const GFL_G3D_OBJSTATUS g3DobjStatus1 = {
	{ 0, 0, 0 },															//座標
	{ FX32_ONE, FX32_ONE, FX32_ONE },										//スケール
	{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },	//回転
};

static void	rotateCalc(SAMPLE4_WORK* sw, MtxFx33* pRotate);
static void	cameraTargetMvCalc(SAMPLE4_WORK* sw);
static void	cameraTargetScaleCalc(SAMPLE4_WORK* sw);
static void	drawObject(GFL_G3D_OBJ* g3Dobj, GFL_G3D_OBJSTATUS* pStatus, int mode);
//============================================================================================
static BOOL	sample4(SAMPLE4_WORK* sw)
{
	switch(sw->seq){

	case 0:
		sw->timer = 0;
		sw->target = 0;
		sw->mode = 0;

		//３Ｄオブジェクト作成
		sw->g3DresMdl0 = GFL_G3D_CreateResourceArc
										(ARCID_SHADOW_TEST, NARC_shadow_test_shadow4_test_nsbmd);
		sw->g3Dobj0 = GFL_G3D_OBJECT_Create(GFL_G3D_RENDER_Create(sw->g3DresMdl0, 0, NULL), NULL, 0); 
		sw->g3DresMdl1 = GFL_G3D_CreateResourceArc
										(ARCID_SHADOW_TEST, NARC_shadow_test_shadow4_test_nsbmd);
		sw->g3Dobj1 = GFL_G3D_OBJECT_Create(GFL_G3D_RENDER_Create(sw->g3DresMdl1, 0, NULL), NULL, 0); 

		//sw->fldWipeObj = FLD_WIPEOBJ_Create(sw->heapID); 

		GFL_G3D_SetSystemSwapBufferMode(GX_SORTMODE_MANUAL, GX_BUFFERMODE_W);

		sw->seq++;
		break;

	case 1:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){ sw->seq++; }
//		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){ sw->target = (sw->target)? 0 : 1; }
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){ sw->mode = (sw->mode)? 0 : 1; }

		cameraTargetMvCalc(sw);
		//cameraTargetScaleCalc(sw);

		//３Ｄ描画
		GFL_G3D_DRAW_Start();			//描画開始
		GFL_G3D_DRAW_SetLookAt();	//カメラグローバルステート設定		

		DWS_DrawLocalOriginBumpPlane(sw->dws, 64*FX32_ONE, GX_RGB(0,31,0), GX_RGB(0,24,0), 32);

		if(sw->target == 0){
			//FLD_WIPEOBJ_Main(sw->fldWipeObj, sw->scale, 20);
		} else {
			GFL_G3D_OBJSTATUS status;

			status = g3DobjStatus1;
			status.trans = sw->trans;
			//status.trans.y = FX32_ONE*16;
			status.scale.y = FX32_ONE*8;
			drawObject(sw->g3Dobj0, &status, sw->mode);
			rotateCalc(sw, &status.rotate);
			drawObject(sw->g3Dobj0, &status, sw->mode);
		}
		GFL_G3D_DRAW_End();				//描画終了（バッファスワップ）					
		break;

	case 2:
		//FLD_WIPEOBJ_Delete(sw->fldWipeObj);

		GFL_G3D_RENDER_Delete(GFL_G3D_OBJECT_GetG3Drnd(sw->g3Dobj1)); 
		GFL_G3D_OBJECT_Delete(sw->g3Dobj1); 
		GFL_G3D_DeleteResource(sw->g3DresMdl1);
		GFL_G3D_RENDER_Delete(GFL_G3D_OBJECT_GetG3Drnd(sw->g3Dobj0)); 
		GFL_G3D_OBJECT_Delete(sw->g3Dobj0); 
		GFL_G3D_DeleteResource(sw->g3DresMdl0);

		return FALSE;
	}
	return TRUE;
}

//--------------------------------------------------------------
static void	rotateCalc(SAMPLE4_WORK* sw, MtxFx33* pRotate)
{
	VecFx32	vecRot;
	MtxFx33 tmpMtx;

	vecRot.x = 0;
//	vecRot.y = 0;
//	vecRot.z = sw->timer * 0x080;
	vecRot.y = 0x8000;
	vecRot.z = 0;

	MTX_RotX33(pRotate, FX_SinIdx((u16)vecRot.x), FX_CosIdx((u16)vecRot.x));

	MTX_RotY33(&tmpMtx, FX_SinIdx((u16)vecRot.y), FX_CosIdx((u16)vecRot.y)); 
	MTX_Concat33(pRotate, &tmpMtx, pRotate); 

	MTX_RotZ33(&tmpMtx, FX_SinIdx((u16)vecRot.z), FX_CosIdx((u16)vecRot.z)); 
	MTX_Concat33(pRotate, &tmpMtx, pRotate); 
}

//--------------------------------------------------------------
#define MV_SP (FX32_ONE/2)
static void	cameraTargetMvCalc(SAMPLE4_WORK* sw)
{
	VecFx32 vecMv;
	u16			angle, angleOffs;

	if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )					{ angleOffs = 0x8000; }
	else if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )	{ angleOffs = 0x0000; }
	else if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )	{ angleOffs = 0xc000; }
	else if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )	{ angleOffs = 0x4000; }
	else { return; }

	angle = DWS_GetCamAngleV(sw->dws);
	angle += angleOffs;

	VEC_Set(&vecMv, FX_SinIdx(angle), 0, FX_CosIdx(angle));
	VEC_Normalize(&vecMv, &vecMv);

	VEC_MultAdd(MV_SP, &vecMv, &sw->trans, &sw->trans);
}

//--------------------------------------------------------------
#define SCALE_SP (FX32_ONE/128)
static void	cameraTargetScaleCalc(SAMPLE4_WORK* sw)
{
	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_B ){
		sw->scale -= SCALE_SP;
		if(sw->scale < FX32_ONE/2){ sw->scale = FX32_ONE/2; }
	} else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ){
		sw->scale += SCALE_SP;
		if(sw->scale > FX32_ONE*2){ sw->scale = FX32_ONE*2; }
	}
}

//--------------------------------------------------------------
static void	drawObject(GFL_G3D_OBJ* g3Dobj, GFL_G3D_OBJSTATUS* pStatus, int mode)
{
	NNSG3dRenderObj*	pRnd = GFL_G3D_RENDER_GetRenderObj(GFL_G3D_OBJECT_GetG3Drnd(g3Dobj));
	NNSG3dResMdl*			pMdl = NNS_G3dRenderObjGetResMdl(pRnd);

	if(mode == 0){
		NNS_G3dMdlSetMdlLightEnableFlag( pMdl, 0, 0x0 );
		NNS_G3dMdlSetMdlPolygonID( pMdl, 0, 1 );
		NNS_G3dMdlSetMdlCullMode( pMdl, 0, GX_CULL_NONE ); 
		NNS_G3dMdlSetMdlAlpha( pMdl, 0, 16 );
		NNS_G3dMdlSetMdlPolygonMode( pMdl, 0, GX_POLYGONMODE_MODULATE );

		GFL_G3D_DRAW_DrawObject(g3Dobj, pStatus);
	} else {
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
}
