
//============================================================================================
/**
 * @file	3d_test.c
 * @brief	3D描画テスト
 * @author	sogabe
 * @date	2007.05.16
 */
//============================================================================================


#include "gflib.h"
#include "calctool.h"

#include "yt_common.h"

//------------------------------------------------------------------
/**
 * @brief		ワーク構造体
 */
//------------------------------------------------------------------
typedef struct
{
	u8			spa_work[PARTICLE_LIB_HEAP_SIZE];
	GFL_PTC_PTR	ptc;
	u16			rotx;
	u16			roty;
	u16			rotz;
}G3D_TEST;

static u32 sAllocTex(u32 size, BOOL is4x4comp);
static u32 sAllocTexPalette(u32 size, BOOL is4pltt);

#define	ROTATE_VALUE	(256)

//----------------------------------------------------------------------------
/**
 *	@brief	3D初期化
 *	
 *	@param	gp		ゲームパラメータポインタ
 */
//-----------------------------------------------------------------------------
void	YT_Init3DTest(GAME_PARAM *gp)
{
	G3D_TEST	*gt;

	//ジョブワークを確保
	gt=gp->job_work=GFL_HEAP_AllocClearMemory(gp->heapID,sizeof(G3D_TEST));

	G3X_Init();

	GX_SetVisiblePlane(GX_PLANEMASK_BG0);
	GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,GX_BGMODE_4,GX_BG0_AS_3D);
	G2_SetBG0Priority(0);

	G3X_SetShading(GX_SHADING_HIGHLIGHT);
	G3X_AntiAlias(TRUE);

	G3_SwapBuffers(GX_SORTMODE_AUTO,GX_BUFFERMODE_W);

	G3_ViewPort(0,0,255,191);

	G3_LightVector(GX_LIGHTID_0,0,-FX32_ONE+1,0);
	G3_LightColor(GX_LIGHTID_0,GX_RGB(31,31,31));

	GX_DispOn();

	NNS_GfdInitFrmTexVramManager(2,TRUE);
	NNS_GfdInitFrmPlttVramManager(0x8000,TRUE);

	GFL_PTC_Init();
	gt->ptc=GFL_PTC_Create(gt->spa_work,PARTICLE_LIB_HEAP_SIZE,FALSE,gp->heapID);
	GFL_PTC_SetResource(gt->ptc,GFL_PTC_LoadArcResource(2,0,gp->heapID),TRUE,NULL);

	G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);

	YT_JobNoSet(gp,YT_Main3DTestNo);
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画BOXデータ
 */
//-----------------------------------------------------------------------------

//回転計算などは、自前でやってみる
typedef struct
{
	VecFx32	line[3];
	VecFx32	housen;
}TRIANGLE_DATA;

typedef struct
{
	VecFx32	line[4];
	VecFx32	housen;
}SQUARE_DATA;

#define	Z_POS	(10)

static	const	TRIANGLE_DATA	TriangleData[]={
	{
		{
			{-FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE},
			{ FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE},
			{ 0,		-FX32_ONE, Z_POS*FX32_ONE+FX32_HALF},
		},
		{0,-FX32_SIN45,-FX32_COS45},
	},
	{
		{
			{ FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE},
			{ FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
			{ 0,		-FX32_ONE, Z_POS*FX32_ONE+FX32_HALF},
		},
		{FX32_COS45,-FX32_SIN45,0},
	},
	{
		{
			{ FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
			{-FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
			{ 0,		-FX32_ONE, Z_POS*FX32_ONE+FX32_HALF},
		},
		{0,-FX32_SIN45,FX32_COS45},
	},
	{
		{
			{-FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
			{-FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE},
			{ 0,		-FX32_ONE, Z_POS*FX32_ONE+FX32_HALF},
		},
		{-FX32_COS45,-FX32_SIN45,0},
	},
};

static	const	SQUARE_DATA	SquareData[]={
	{
		{
			{-FX32_HALF, FX32_HALF,Z_POS*FX32_ONE},
			{-FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE},
			{ FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE},
			{ FX32_HALF, FX32_HALF,Z_POS*FX32_ONE},
		},
		{0,0,-FX32_ONE},
	},
	{
		{
			{-FX32_HALF, FX32_HALF,Z_POS*FX32_ONE},
			{-FX32_HALF, FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
			{ FX32_HALF, FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
			{ FX32_HALF, FX32_HALF,Z_POS*FX32_ONE},
		},
		{0,FX32_ONE,0},
	},
/*
	{
		{
			{-FX32_HALF,-FX32_HALF,8*FX32_ONE},
			{-FX32_HALF,-FX32_HALF,8*FX32_ONE+FX32_ONE},
			{ FX32_HALF,-FX32_HALF,8*FX32_ONE+FX32_ONE},
			{ FX32_HALF,-FX32_HALF,8*FX32_ONE},
		},
		{0,-FX32_ONE,0},
	},
*/
	{
		{
			{-FX32_HALF, FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
			{-FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
			{ FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
			{ FX32_HALF, FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
		},
		{0,0,FX32_ONE},
	},
	{
		{
			{-FX32_HALF, FX32_HALF,Z_POS*FX32_ONE},
			{-FX32_HALF, FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
			{-FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
			{-FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE},
		},
		{-FX32_ONE,0,0},
	},
	{
		{
			{ FX32_HALF, FX32_HALF,Z_POS*FX32_ONE},
			{ FX32_HALF, FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
			{ FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE+FX32_ONE},
			{ FX32_HALF,-FX32_HALF,Z_POS*FX32_ONE},
		},
		{ FX32_ONE,0,0},
	},
};

#define	TRIANGLE_MAX	NELEMS(TriangleData)
#define	SQUARE_MAX		NELEMS(SquareData)

#if 1
//回転計算などは、ハードまかせ
//----------------------------------------------------------------------------
/**
 *	@brief	3D描画メイン
 *	
 *	@param	gp		ゲームパラメータポインタ
 */
//-----------------------------------------------------------------------------
void	YT_Main3DTest(GAME_PARAM *gp)
{
	G3D_TEST	*gt=(G3D_TEST *)gp->job_work;
	int			plate,line;
	fx16	s;
	fx16	c;
	VecFx32	Eye={0,0,			16*FX32_ONE};
	VecFx32	at=	{0,0,			-5*FX32_ONE};
	VecFx32	vUp={0,FX32_ONE,	0};

	G3X_Reset();

	if(GFL_UI_KEY_GetCont()&PAD_KEY_UP){
		gt->rotx-=ROTATE_VALUE;
	}
	if(GFL_UI_KEY_GetCont()&PAD_KEY_DOWN){
		gt->rotx+=ROTATE_VALUE;
	}
	if(GFL_UI_KEY_GetCont()&PAD_KEY_LEFT){
		gt->roty-=ROTATE_VALUE;
	}
	if(GFL_UI_KEY_GetCont()&PAD_KEY_RIGHT){
		gt->roty+=ROTATE_VALUE;
	}
	if(GFL_UI_KEY_GetCont()&PAD_BUTTON_L){
		gt->rotz+=ROTATE_VALUE;
	}
	if(GFL_UI_KEY_GetCont()&PAD_BUTTON_R){
		gt->rotz-=ROTATE_VALUE;
	}
	if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_A){
		{
			VecFx32	pos={0,0,0};
			GFL_PTC_CreateEmitter(gt->ptc,0,&pos);
			OS_TPrintf("emit:%d\n",GFL_PTC_GetEmitterNum(gt->ptc));
		}
	}

	G3_Perspective(FX32_SIN45,FX32_COS45,FX32_ONE*4/3,FX32_ONE,FX32_ONE*900,NULL);

	G3_LookAt(&Eye,&vUp,&at,NULL);
	NNS_G3dGlbLookAt(&Eye,&vUp,&at);

	G3_PushMtx();

	G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
	G3_Identity();

	//G3_Scale(2*FX32_ONE,2*FX32_ONE,FX32_ONE);

	G3_Translate(0,0,-7*FX32_ONE+FX32_HALF);

	s=FX_SinIdx(gt->rotx);
	c=FX_CosIdx(gt->rotx);
	G3_RotX(s,c);
	s=FX_SinIdx(gt->roty);
	c=FX_CosIdx(gt->roty);
	G3_RotY(s,c);
	s=FX_SinIdx(gt->rotz);
	c=FX_CosIdx(gt->rotz);
	G3_RotZ(s,c);

	G3_Translate(0,0,7*FX32_ONE-FX32_HALF);

	G3_MaterialColorDiffAmb(GX_RGB(31,31,31),GX_RGB(16,16,16),FALSE);
	G3_MaterialColorSpecEmi(GX_RGB(16,16,16),GX_RGB(0,0,0),FALSE);
	G3_TexImageParam(GX_TEXFMT_NONE,GX_TEXGEN_NONE,GX_TEXSIZE_S8,GX_TEXSIZE_T8,
					 GX_TEXREPEAT_NONE,GX_TEXFLIP_NONE,GX_TEXPLTTCOLOR0_USE,0);
	G3_PolygonAttr(GX_LIGHTMASK_0,GX_POLYGONMODE_MODULATE,GX_CULL_NONE,1,0,GX_POLYGON_ATTR_MISC_NONE);

	G3_Begin(GX_BEGIN_QUADS);
	{
		MtxFx33	mtx33_x;
		MtxFx33	mtx33_y;
		MtxFx33	mtx33_z;
		MtxFx33	mtx33;
		VecFx32	vec;
		VecFx16	vec2;
		VecFx16	eye={0,0,FX16_ONE};

		for(plate=0;plate<SQUARE_MAX;plate++){
			MTX_RotX33(&mtx33_x,FX_SinIdx(gt->rotx),FX_CosIdx(gt->rotx));
			MTX_RotY33(&mtx33_y,FX_SinIdx(gt->roty),FX_CosIdx(gt->roty));
			MTX_RotZ33(&mtx33_z,FX_SinIdx(gt->rotz),FX_CosIdx(gt->rotz));
			MTX_Concat33(&mtx33_y,&mtx33_x,&mtx33);
			MTX_Concat33(&mtx33_z,&mtx33,&mtx33);
			MTX_MultVec33(&SquareData[plate].housen,&mtx33,&vec);
			vec2.x=vec.x;
			vec2.y=vec.y;
			vec2.z=vec.z;
			if(VEC_Fx16DotProduct(&eye,&vec2)>0){
				for(line=0;line<4;line++){
					G3_Vtx(SquareData[plate].line[line].x,SquareData[plate].line[line].y,SquareData[plate].line[line].z);
				}
			}
		}
	}
	G3_End();

	G3_Begin(GX_BEGIN_TRIANGLES);
	{
		MtxFx33	mtx33_x;
		MtxFx33	mtx33_y;
		MtxFx33	mtx33_z;
		MtxFx33	mtx33;
		VecFx32	vec;
		VecFx16	vec2;
		VecFx16	eye={0,0,FX16_ONE};

		for(plate=0;plate<TRIANGLE_MAX;plate++){
			MTX_RotX33(&mtx33_x,FX_SinIdx(gt->rotx),FX_CosIdx(gt->rotx));
			MTX_RotY33(&mtx33_y,FX_SinIdx(gt->roty),FX_CosIdx(gt->roty));
			MTX_RotZ33(&mtx33_z,FX_SinIdx(gt->rotz),FX_CosIdx(gt->rotz));
			MTX_Concat33(&mtx33_y,&mtx33_x,&mtx33);
			MTX_Concat33(&mtx33_z,&mtx33,&mtx33);
			MTX_MultVec33(&TriangleData[plate].housen,&mtx33,&vec);
			vec2.x=vec.x;
			vec2.y=vec.y;
			vec2.z=vec.z;
			if(VEC_Fx16DotProduct(&eye,&vec2)>0){
				for(line=0;line<3;line++){
					G3_Vtx(TriangleData[plate].line[line].x,TriangleData[plate].line[line].y,TriangleData[plate].line[line].z);
				}
			}
		}
	}
	G3_End();

	G3_PopMtx(1);

	GFL_PTC_Main();

	G3_SwapBuffers(GX_SORTMODE_MANUAL,GX_BUFFERMODE_W);
}

#else
//----------------------------------------------------------------------------
/**
 *	@brief	3D描画メイン
 *	
 *	@param	gp		ゲームパラメータポインタ
 */
//-----------------------------------------------------------------------------
void	YT_Main3DTest(GAME_PARAM *gp)
{
	G3D_TEST	*gt=(G3D_TEST *)gp->job_work;
	int			i,j;
	fx16	s;
	fx16	c;

	if(GFL_UI_KEY_GetCont()&PAD_KEY_UP){
		gt->rotx-=ROTATE_VALUE;
	}
	if(GFL_UI_KEY_GetCont()&PAD_KEY_DOWN){
		gt->rotx+=ROTATE_VALUE;
	}
	if(GFL_UI_KEY_GetCont()&PAD_KEY_LEFT){
		gt->roty-=ROTATE_VALUE;
	}
	if(GFL_UI_KEY_GetCont()&PAD_KEY_RIGHT){
		gt->roty+=ROTATE_VALUE;
	}
	if(GFL_UI_KEY_GetCont()&PAD_BUTTON_L){
		gt->rotz+=ROTATE_VALUE;
	}
	if(GFL_UI_KEY_GetCont()&PAD_BUTTON_R){
		gt->rotz-=ROTATE_VALUE;
	}

	G3_Perspective(FX32_SIN30,FX32_COS30,FX32_ONE*4/3,FX32_ONE,FX32_ONE*400,NULL);

	{
		VecFx32	Eye={0,0,			FX32_ONE};
		VecFx32	at=	{0,0,			-5*FX32_ONE};
		VecFx32	vUp={0,FX32_ONE,	0};

		G3_LookAt(&Eye,&vUp,&at,NULL);
	}

	G3_MtxMode(GX_MTXMODE_TEXTURE);
	G3_Identity();
	G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
	G3_PushMtx();

	G3_MaterialColorDiffAmb(GX_RGB(31,31,31),GX_RGB(16,16,16),FALSE);
	G3_MaterialColorSpecEmi(GX_RGB(16,16,16),GX_RGB(0,0,0),FALSE);
	G3_TexImageParam(GX_TEXFMT_NONE,GX_TEXGEN_NONE,GX_TEXSIZE_S8,GX_TEXSIZE_T8,
					 GX_TEXREPEAT_NONE,GX_TEXFLIP_NONE,GX_TEXPLTTCOLOR0_USE,0);
	G3_PolygonAttr(GX_LIGHTMASK_0,GX_POLYGONMODE_MODULATE,GX_CULL_NONE,1,0,GX_POLYGON_ATTR_MISC_NONE);
	G3_Begin(GX_BEGIN_QUADS);

	{
		MtxFx33	mtx33_x;
		MtxFx33	mtx33_y;
		MtxFx33	mtx33_z;
		VecFx32	vec;

		for(i=0;i<4;i++){
			for(j=0;j<4;j++){
				MTX_RotX33(&mtx33_x,FX_SinIdx(gt->rotx),FX_CosIdx(gt->rotx));
				MTX_RotY33(&mtx33_y,FX_SinIdx(gt->roty),FX_CosIdx(gt->roty));
				MTX_RotZ33(&mtx33_z,FX_SinIdx(gt->rotz),FX_CosIdx(gt->rotz));
				MTX_MultVec33(&BoxData[i].line[j],&mtx33_x,&vec);
//				MTX_MultVec33(&vec,&mtx33_y,&vec);
//				MTX_MultVec33(&vec,&mtx33_z,&vec);
				G3_Vtx(vec.x,vec.y,vec.z);
			}
		}
	};

	G3_End();
	G3_PopMtx(1);

	GFL_PTC_Main();

	G3_SwapBuffers(GX_SORTMODE_MANUAL,GX_BUFFERMODE_Z);
}
#endif

