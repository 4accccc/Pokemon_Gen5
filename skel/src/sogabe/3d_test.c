
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

#define	POLY_MAX	(16)

typedef struct
{
	fx32			ofs_x[POLY_MAX];
	fx32			ofs_y[POLY_MAX];
	fx32			ofs_z[POLY_MAX];
	fx32			speed[POLY_MAX];
	u16				rotx;
	u16				roty;
	u16				rotz;
	VecFx32			vec;
	int				update;
	MtxFx33			mtx;
	GFL_QUATERNION	*qt;
}G3D_TEST;

//----------------------------------------------------------------------------
/**
 *	@brief	テクスチャデータ
 */
//-----------------------------------------------------------------------------
static	const u16 tex_16plett8x8[] = {
    0x1111, 0x1111,
	0x2222, 0x2222,
    0x3333, 0x3333,
	0x4444, 0x4444,
    0x5555, 0x5555,
	0x6666, 0x6666,
    0x7777, 0x7777,
	0x8888, 0x8888,
};

static	const u16 pal_16plett[] = {
    GX_RGB( 0, 0, 0),GX_RGB( 0, 0,31),GX_RGB( 0,31, 0),GX_RGB( 0,31,31),
	GX_RGB(31, 0, 0),GX_RGB(31, 0,31),GX_RGB(31,31, 0),GX_RGB(31,31,31),
    GX_RGB( 0, 0, 0),GX_RGB( 0, 0, 0),GX_RGB( 0, 0, 0),GX_RGB( 0, 0, 0),
	GX_RGB( 0, 0, 0),GX_RGB( 0, 0, 0),GX_RGB( 0, 0, 0),GX_RGB( 0, 0, 0),
};

#define	ROTATE_VALUE	(1024)

//----------------------------------------------------------------------------
/**
 *	@brief	描画BOXデータ
 */
//-----------------------------------------------------------------------------

#define	BOX_LINE	(FX32_HALF/2)

static	const	VecFx32	BoxData[]={
	{-BOX_LINE, BOX_LINE,-BOX_LINE},
	{ BOX_LINE, BOX_LINE,-BOX_LINE},
	{ BOX_LINE,-BOX_LINE,-BOX_LINE},
	{-BOX_LINE,-BOX_LINE,-BOX_LINE},

	{-BOX_LINE, BOX_LINE,-BOX_LINE},
	{ BOX_LINE, BOX_LINE,-BOX_LINE},
	{ BOX_LINE, BOX_LINE, BOX_LINE},
	{-BOX_LINE, BOX_LINE, BOX_LINE},

	{-BOX_LINE,-BOX_LINE,-BOX_LINE},
	{ BOX_LINE,-BOX_LINE,-BOX_LINE},
	{ BOX_LINE,-BOX_LINE, BOX_LINE},
	{-BOX_LINE,-BOX_LINE, BOX_LINE},

	{-BOX_LINE, BOX_LINE, BOX_LINE},
	{ BOX_LINE, BOX_LINE, BOX_LINE},
	{ BOX_LINE,-BOX_LINE, BOX_LINE},
	{-BOX_LINE,-BOX_LINE, BOX_LINE},

	{-BOX_LINE, BOX_LINE,-BOX_LINE},
	{-BOX_LINE, BOX_LINE, BOX_LINE},
	{-BOX_LINE,-BOX_LINE, BOX_LINE},
	{-BOX_LINE,-BOX_LINE,-BOX_LINE},

	{ BOX_LINE, BOX_LINE,-BOX_LINE},
	{ BOX_LINE, BOX_LINE, BOX_LINE},
	{ BOX_LINE,-BOX_LINE, BOX_LINE},
	{ BOX_LINE,-BOX_LINE,-BOX_LINE},
};

void	YT_Init3DTest(GAME_PARAM *gp);
void	YT_Main3DTest(GAME_PARAM *gp);

static	void	MatrixRotation(MtxFx33 *mtx,u16 rot_x,u16 rot_y,u16 rot_z);
static	void	MatrixNormarize(MtxFx33 *mtx);

#define	NEARLY_ZERO	FX32_CONST(1.E-16)
#define	ZERO_TOLERANCE	FX32_CONST(1.E-8)

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
	int			i;

	//ジョブワークを確保
	gt=gp->job_work=GFL_HEAP_AllocClearMemory(gp->heapID,sizeof(G3D_TEST));

	G3X_Init();

	GX_SetBankForTex(GX_VRAM_TEX_0_A);
	GX_SetBankForTexPltt(GX_VRAM_TEXPLTT_0_G);

	GX_SetVisiblePlane(GX_PLANEMASK_BG0);
	GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,GX_BGMODE_4,GX_BG0_AS_3D);
	G2_SetBG0Priority(0);

	G3X_SetShading(GX_SHADING_HIGHLIGHT);
	G3X_AntiAlias(TRUE);

	G3_SwapBuffers(GX_SORTMODE_AUTO,GX_BUFFERMODE_W);

	G3_ViewPort(0,0,255,191);
	G3_Perspective(FX32_SIN30,FX32_COS30,FX32_ONE*4/3,FX32_ONE,FX32_ONE*900,NULL);

	GX_DispOn();

	G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);

	DC_FlushRange(tex_16plett8x8,sizeof(tex_16plett8x8));
	DC_FlushRange(pal_16plett,sizeof(pal_16plett));
	GX_BeginLoadTex();    
	GX_LoadTex((void *)&tex_16plett8x8[0],0,32);
	GX_EndLoadTex();
	GX_BeginLoadTexPltt();
	GX_LoadTexPltt((void *)&pal_16plett[0],0,32);
	GX_EndLoadTexPltt();

	//乱数初期化
	{
		RTCDate	date;
		RTCTime	time;
		u32		seed;

		RTC_GetDateTime(&date,&time);

		seed=date.year+date.month*0x100+date.day*0x10000+time.hour*0x100000+(time.minute+time.second)*0x1000000;

		GFL_STD_MtRandInit(seed);
	}

	gt->vec.x=FX32_ONE;
	gt->vec.y=FX32_ONE;
	gt->vec.z=FX32_ONE;
	gt->update=1;

	MTX_Identity33(&gt->mtx);

	gt->qt=GFL_QUAT_Init(gp->heapID);
	GFL_QUAT_Identity(gt->qt);

	YT_JobNoSet(gp,YT_Main3DTestNo);
}

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

	gt->rotx=0;
	gt->roty=0;
	gt->rotz=0;

	if(GFL_UI_KEY_GetCont()&PAD_KEY_UP){
		gt->rotx=ROTATE_VALUE;
		gt->update=1;
	}
	if(GFL_UI_KEY_GetCont()&PAD_KEY_DOWN){
		gt->rotx=-ROTATE_VALUE;
		gt->update=1;
	}
	if(GFL_UI_KEY_GetCont()&PAD_KEY_LEFT){
		gt->roty=ROTATE_VALUE;
		gt->update=1;
	}
	if(GFL_UI_KEY_GetCont()&PAD_KEY_RIGHT){
		gt->roty=-ROTATE_VALUE;
		gt->update=1;
	}
	if(GFL_UI_KEY_GetCont()&PAD_BUTTON_L){
		gt->rotz=-ROTATE_VALUE;
		gt->update=1;
	}
	if(GFL_UI_KEY_GetCont()&PAD_BUTTON_R){
		gt->rotz=ROTATE_VALUE;
		gt->update=1;
	}

	if(gt->update){
		//gt->update=0;

		G3X_Reset();

		{
			VecFx32	Eye={0,0,			FX32_ONE};
			VecFx32	at=	{0,0,			-5*FX32_ONE};
			VecFx32	vUp={0,FX32_ONE,	0};

			G3_LookAt(&Eye,&vUp,&at,NULL);
		}

		G3_PushMtx();

		G3_MtxMode(GX_MTXMODE_TEXTURE);
		G3_Identity();
		G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

		G3_Translate(0,0,-FX32_ONE);

		{
			fx32			norm;
			MtxFx44			mtx;
			GFL_QUATERNION	*qt_p=GFL_QUAT_Init(gp->heapID);

			GFL_QUAT_MakeQuaternionXYZ(qt_p,gt->rotx,gt->roty,gt->rotz);
			GFL_QUAT_Mul(gt->qt,qt_p,gt->qt);
			norm=GFL_QUAT_Norm(gt->qt);
			GFL_QUAT_DivReal(gt->qt,gt->qt,norm);
			GFL_QUAT_MakeRotateMatrix(&mtx,gt->qt);
			G3_MultMtx44(&mtx);

			GFL_QUAT_Free(qt_p);
		}

		G3_MaterialColorDiffAmb(GX_RGB(31,31,31),GX_RGB(16,16,16),FALSE);
		G3_MaterialColorSpecEmi(GX_RGB(16,16,16),GX_RGB(0,0,0),FALSE);
		G3_TexImageParam(GX_TEXFMT_PLTT16,GX_TEXGEN_TEXCOORD,GX_TEXSIZE_S8,GX_TEXSIZE_T8,
						 GX_TEXREPEAT_ST,GX_TEXFLIP_NONE,GX_TEXPLTTCOLOR0_USE,0);
		G3_TexPlttBase(0,GX_TEXFMT_PLTT16);
		G3_PolygonAttr(GX_LIGHTMASK_0,GX_POLYGONMODE_MODULATE,GX_CULL_NONE,1,0,GX_POLYGON_ATTR_MISC_NONE);
		G3_Begin(GX_BEGIN_QUADS);

		for(i=0;i<24;i++){
			G3_TexCoord(0,(i/6)*FX32_ONE);
			G3_Vtx(BoxData[i].x,BoxData[i].y,BoxData[i].z);
		}

		G3_End();
		G3_PopMtx(1);

		G3_SwapBuffers(GX_SORTMODE_MANUAL,GX_BUFFERMODE_Z);
	}
}

static	void	MatrixRotation(MtxFx33 *mtx,u16 rot_x,u16 rot_y,u16 rot_z)
{
	MtxFx33	dMtx_X;
	MtxFx33	dMtx_Y;
	MtxFx33	dMtx_Z;

	MTX_RotX33(&dMtx_X,FX_SinIdx(rot_x),FX_CosIdx(rot_x));
	MTX_RotY33(&dMtx_Y,FX_SinIdx(rot_y),FX_CosIdx(rot_y));
	MTX_RotZ33(&dMtx_Z,FX_SinIdx(rot_z),FX_CosIdx(rot_z));
	MTX_Concat33(mtx,&dMtx_X,mtx);
	MTX_Concat33(mtx,&dMtx_Y,mtx);
	MTX_Concat33(mtx,&dMtx_Z,mtx);
	MatrixNormarize(mtx);
}

static	void	MatrixNormarize(MtxFx33 *mtx)
{
	fx32	m0;
	fx32	m1;
	fx32	m2;
	fx32	m3;
	fx32	m4;
	fx32	m5;
	fx32	u;

	m0 = FX_Mul(mtx->_00,mtx->_11);
	m1 = FX_Mul(mtx->_10,mtx->_21);
	m2 = FX_Mul(mtx->_20,mtx->_01);
	m3 = FX_Mul(mtx->_00,mtx->_12);
	m4 = FX_Mul(mtx->_01,mtx->_10);
	m5 = FX_Mul(mtx->_02,mtx->_11);

	m0 = FX_Mul(m0,mtx->_22);
	m1 = FX_Mul(m1,mtx->_02);
	m2 = FX_Mul(m2,mtx->_12);
	m3 = FX_Mul(m3,mtx->_21);
	m4 = FX_Mul(m4,mtx->_22);
	m5 = FX_Mul(m5,mtx->_20);

	u =	m0 + m1 + m2 - m3 - m4 - m5;

	mtx->_00=FX_Div(mtx->_00,u);
	mtx->_01=FX_Div(mtx->_01,u);
	mtx->_02=FX_Div(mtx->_02,u);
	mtx->_10=FX_Div(mtx->_10,u);
	mtx->_11=FX_Div(mtx->_11,u);
	mtx->_12=FX_Div(mtx->_12,u);
	mtx->_20=FX_Div(mtx->_20,u);
	mtx->_21=FX_Div(mtx->_21,u);
	mtx->_22=FX_Div(mtx->_22,u);
}


//過去に作った実験
#if 0
//------------------------------------------------------------------
/**
 * @brief		ワーク構造体
 */
//------------------------------------------------------------------

#define	POLY_MAX	(16)

typedef struct
{
	fx32		ofs_x[POLY_MAX];
	fx32		ofs_y[POLY_MAX];
	fx32		ofs_z[POLY_MAX];
	fx32		speed[POLY_MAX];
	u16			rotx[POLY_MAX];
	u16			roty[POLY_MAX];
	u16			rotz[POLY_MAX];
}G3D_TEST;

const	static	u16	TexData[]={
#if 0
	GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),
	GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),
	GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),
	GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),
	GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),
	GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),
	GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),
	GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),GX_RGB(31,0,0),
#endif
	GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),
	GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),
	GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),
	GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),
	GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),
	GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),
	GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),
	GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),GX_RGB(0,31,0),
};
const u16 tex_16plett64x64[] = {
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1161, 0x1111,
    0x1181, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1686, 0x1166,
    0x6815, 0x8161, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x6111, 0x6558, 0x1586,
    0x8855, 0x8655, 0x1118, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1511, 0x1588, 0x8686, 0x5668,
    0x8564, 0x6645, 0x1466, 0x1114, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x5451, 0x6565, 0x6618, 0x4566,
    0x6544, 0x6546, 0x2414, 0x1124, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x6111, 0x4415, 0x5656, 0x8658, 0x4456,
    0x4444, 0x6455, 0x4544, 0x2544, 0x1141, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x4551, 0x1644, 0x5555, 0x6865, 0x2586,
    0x2422, 0x4454, 0x5454, 0x2456, 0x1422, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x5511, 0x5445, 0x6555, 0x6886, 0x4461,
    0x4224, 0x2242, 0x5542, 0x4565, 0x4442, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x4455, 0x5651, 0x6b61, 0x6666,
    0x2254, 0x4424, 0x5444, 0x4564, 0x1244, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x5164, 0x6645, 0x6656, 0x5614, 0x8566,
    0x2258, 0x4422, 0x5554, 0x6555, 0x2464, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x4111, 0x4524, 0x4445, 0x6458, 0x224b, 0x6588,
    0x245b, 0x4554, 0x5456, 0x4664, 0x2252, 0x1112, 0x1111, 0x1111,
    0x1111, 0x1111, 0x6611, 0x2442, 0x4644, 0x5485, 0x22b6, 0x56b5,
    0x5645, 0x2542, 0x6646, 0x2644, 0x2322, 0x1112, 0x1111, 0x1111,
    0x1111, 0x1111, 0x2611, 0x2244, 0x4442, 0x4482, 0x2284, 0x5442,
    0x6655, 0x2424, 0x6242, 0x3222, 0x2333, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x6116, 0x2445, 0x4422, 0x4425, 0x2244, 0x1422,
    0x2464, 0x2222, 0x2223, 0x3232, 0x1233, 0x1212, 0x1111, 0x1111,
    0x1111, 0x5511, 0x1586, 0x2244, 0x5542, 0x4254, 0x2222, 0x4444,
    0x2222, 0x2322, 0x2422, 0x3324, 0x4233, 0x2222, 0x1111, 0x1111,
    0x1111, 0x5681, 0x6865, 0x4525, 0x5232, 0x4652, 0x2444, 0x2244,
    0x1222, 0x3322, 0x2643, 0x2324, 0x2222, 0x2222, 0x1112, 0x1111,
    0x8111, 0x4448, 0x5586, 0x3222, 0x3333, 0x4222, 0x4455, 0xb464,
    0x2124, 0x3323, 0x2433, 0x4242, 0x2442, 0x4222, 0x1122, 0x1111,
    0x6511, 0x4465, 0x4458, 0x5244, 0x2233, 0x4422, 0x4445, 0x4465,
    0x2266, 0x3332, 0x2232, 0x2244, 0x2424, 0x5424, 0x1112, 0x1111,
    0x5111, 0x6655, 0x6645, 0x3564, 0x2333, 0x5221, 0x5665, 0x44b6,
    0x2464, 0x3222, 0x4223, 0x2222, 0x4424, 0x4544, 0x1111, 0x1111,
    0x1111, 0x5665, 0x4644, 0x3264, 0x1334, 0x4422, 0x66b6, 0x8445,
    0x4246, 0x2245, 0x2232, 0x2422, 0x4422, 0x4242, 0x1114, 0x1111,
    0x5111, 0x5541, 0x4444, 0x2224, 0x2235, 0x4222, 0x5586, 0x8855,
    0x5442, 0x2454, 0x3245, 0x4422, 0x5442, 0x5222, 0x1154, 0x1111,
    0x2111, 0x6425, 0x4558, 0x6244, 0x4222, 0x4452, 0x8555, 0x4558,
    0x5655, 0x2224, 0x2225, 0x4445, 0x5445, 0x5225, 0x1324, 0x1111,
    0x5211, 0x8422, 0x6624, 0x4545, 0x4422, 0x868b, 0x4466, 0x5666,
    0x5564, 0x2242, 0x2242, 0x4544, 0x5255, 0x4252, 0x2332, 0x1111,
    0x4411, 0x4224, 0x5622, 0x6654, 0xb445, 0x5645, 0x658b, 0x8486,
    0x6585, 0x4252, 0x4222, 0x6554, 0x4264, 0x2223, 0x4322, 0x1112,
    0x4541, 0x2222, 0x2522, 0x2624, 0x4244, 0x4664, 0x625b, 0x224b,
    0x25b2, 0x5544, 0x4224, 0x6422, 0x2242, 0x2322, 0x3332, 0x1111,
    0x4411, 0x2244, 0x2222, 0x2822, 0x2224, 0x4482, 0x4222, 0x2248,
    0x2242, 0x2254, 0x4222, 0x2252, 0x2424, 0x2224, 0x3222, 0x1114,
    0x4551, 0x4221, 0x2222, 0x2242, 0x2232, 0x2452, 0x2222, 0x2224,
    0x2222, 0x4244, 0x3224, 0x2424, 0x2322, 0x4422, 0x2242, 0x2124,
    0x5111, 0x2455, 0x2222, 0x2242, 0x4332, 0x2422, 0x4222, 0x2254,
    0x2222, 0x2422, 0x3242, 0x2244, 0x5142, 0x2244, 0x2224, 0x2422,
    0x8511, 0x4558, 0x2224, 0x2422, 0x2333, 0x4544, 0x2242, 0x2252,
    0x2222, 0x2444, 0x2322, 0x9223, 0x39aa, 0x4223, 0x2224, 0x4432,
    0x8461, 0x5455, 0x2244, 0x4322, 0x2233, 0x4655, 0x2455, 0x2222,
    0x4422, 0x4622, 0x2332, 0x9972, 0x3377, 0x4333, 0x4224, 0x2332,
    0x4816, 0x5564, 0x4445, 0x3324, 0x4233, 0x5862, 0x5458, 0x4224,
    0x5244, 0x2425, 0x7233, 0x3237, 0x3333, 0x4423, 0x2222, 0x3233,
    0x2458, 0x6642, 0x4545, 0x3222, 0x2333, 0x4823, 0x6484, 0x4244,
    0x2225, 0x2226, 0x7773, 0x2333, 0x3333, 0x2233, 0x2334, 0x3233,
    0x5258, 0x5445, 0x5555, 0x4565, 0x4322, 0x6426, 0x2244, 0x2252,
    0x2226, 0x4222, 0x3334, 0x2323, 0x2333, 0x2223, 0x3333, 0x1333,
    0x6252, 0x4484, 0x5456, 0x2455, 0x7422, 0x2542, 0x2222, 0x2262,
    0x2222, 0x3222, 0x3234, 0x2442, 0x3322, 0x3323, 0x3332, 0x2333,
    0x6222, 0x2224, 0x2588, 0x6585, 0x2225, 0x2223, 0x4222, 0x5422,
    0x4244, 0x4422, 0x4232, 0x2222, 0x3332, 0x3233, 0x3333, 0x3132,
    0x2223, 0x2234, 0x2485, 0x2464, 0x3326, 0x2233, 0x6422, 0x5422,
    0x4225, 0x4234, 0x2422, 0x2142, 0x3233, 0x3333, 0x2323, 0x4342,
    0x2333, 0x2232, 0x2282, 0x4228, 0x3422, 0x4431, 0x6444, 0x5942,
    0x3322, 0x3224, 0x2233, 0x1214, 0x3222, 0x3322, 0x2233, 0x2424,
    0x2333, 0x3333, 0x2223, 0x6222, 0x3322, 0x2522, 0x2524, 0x2932,
    0x3322, 0x3332, 0x3322, 0x2142, 0x2242, 0x4322, 0x4422, 0x3442,
    0x3333, 0x4333, 0x2224, 0x3244, 0x3332, 0x2323, 0x3525, 0x4235,
    0x2327, 0x3333, 0x3423, 0x4222, 0x2232, 0x2243, 0x2444, 0x4423,
    0x2232, 0x5333, 0x4423, 0x3343, 0x3333, 0x2233, 0x4244, 0x2722,
    0x3727, 0x3333, 0x2432, 0x2323, 0x3337, 0x2333, 0x3322, 0x2433,
    0x2242, 0x3232, 0x3523, 0x2233, 0x2333, 0x4423, 0x2272, 0x7722,
    0x3277, 0x3233, 0x3222, 0x7732, 0x3333, 0x3323, 0x3342, 0x2232,
    0x3212, 0x4221, 0x3234, 0x2243, 0x2332, 0x2122, 0x2221, 0x7717,
    0x2177, 0x3233, 0x3223, 0x3377, 0x2333, 0x3344, 0x3323, 0x2233,
    0x3321, 0x4233, 0x7777, 0x2377, 0x3332, 0x1212, 0x7121, 0x7767,
    0x2377, 0x3323, 0x7333, 0x3233, 0x3333, 0x2222, 0x3432, 0x2333,
    0x3111, 0x3333, 0x3333, 0x3223, 0x3237, 0x1223, 0x7222, 0x9733,
    0x3377, 0x3331, 0x3732, 0x2333, 0x3232, 0x2223, 0x3233, 0x3333,
    0x1111, 0x3332, 0x3333, 0x2322, 0x2273, 0x2222, 0x4642, 0x9732,
    0x1377, 0x3322, 0x3377, 0x3333, 0x3222, 0x3323, 0x3333, 0x2233,
    0x1111, 0x2333, 0x3333, 0x2323, 0x7733, 0x3222, 0x2421, 0x9732,
    0x3377, 0x7921, 0x3337, 0x3323, 0x3332, 0x3233, 0x3332, 0x2322,
    0x3111, 0x3232, 0x3333, 0x3333, 0x3333, 0x3277, 0x3223, 0x9322,
    0x3377, 0x7793, 0x3333, 0x2333, 0x3223, 0x3232, 0x2234, 0x3332,
    0x1111, 0x2321, 0x3133, 0x3333, 0x3333, 0x3233, 0x3333, 0x9712,
    0x3379, 0x3779, 0x3333, 0x3333, 0x2333, 0x3333, 0x1222, 0x1433,
    0x1111, 0x1311, 0x1333, 0x2233, 0x3333, 0x2323, 0x3333, 0x9723,
    0x9777, 0x3377, 0x3333, 0x3333, 0x3233, 0x2332, 0x1134, 0x1111,
    0x1111, 0x1111, 0x1131, 0x2121, 0x3322, 0x3333, 0x3333, 0x9733,
    0x7a77, 0x3337, 0x3333, 0x3233, 0x3333, 0x3323, 0x1433, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x3331, 0x3333, 0x3222, 0xa733,
    0x7997, 0x2233, 0x3332, 0x2133, 0x2323, 0x3222, 0x1113, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x3322, 0x2322, 0x2332, 0xa733,
    0x3777, 0x3223, 0x3322, 0x2223, 0x3132, 0x2332, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x2241, 0x2223, 0x3333, 0x9733,
    0x3797, 0x2123, 0x1133, 0x3132, 0x1312, 0x1311, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x2311, 0x3233, 0x3233, 0x9933,
    0x3799, 0x1212, 0x3112, 0x1323, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1212, 0x2223, 0x9a33,
    0x379a, 0x3323, 0x1111, 0x1121, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x2211, 0xa932,
    0x379a, 0x1331, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0xa912,
    0x179a, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0xa911,
    0x199c, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0xa911,
    0x199c, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0xa911,
    0x199c, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0xaa11,
    0x19ac, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0xaa11,
    0x19ac, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,

};

const u16 pal_16plett[] = {
    0x0000, 0xffff, 0x8d60, 0x98c0, 0x8204, 0x8288, 0x830c, 0x94c7,
    0x8390, 0x992a, 0x998d, 0xc3d9, 0x9def, 0x8000, 0x8000, 0x8000,
};

#define	ROTATE_VALUE	(256/4)

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
	G3_Perspective(FX32_SIN30,FX32_COS30,FX32_ONE*4/3,FX32_ONE,FX32_ONE*900,NULL);

	GX_DispOn();

	G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);

	GX_SetBankForTex(GX_VRAM_TEX_0_A);
#if 0
	GX_BeginLoadTex();
	DC_FlushRange(TexData,sizeof(TexData));
	GX_LoadTex(TexData,0,sizeof(TexData));
	GX_EndLoadTex();
#endif
	DC_FlushRange(tex_16plett64x64,sizeof(tex_16plett64x64));
	DC_FlushRange(pal_16plett,sizeof(pal_16plett));
	GX_BeginLoadTex();    
	GX_LoadTex((void *)&tex_16plett64x64[0],0x1000,2048);
	GX_EndLoadTex();
	GX_BeginLoadTexPltt();
	GX_LoadTexPltt((void *)&pal_16plett[0],0x1000,32);
	GX_EndLoadTexPltt();

	//乱数初期化
	{
		RTCDate	date;
		RTCTime	time;
		u32		seed;

		RTC_GetDateTime(&date,&time);

		seed=date.year+date.month*0x100+date.day*0x10000+time.hour*0x100000+(time.minute+time.second)*0x1000000;

		GFL_STD_MtRandInit(seed);
	}

	YT_JobNoSet(gp,YT_Main3DTestNo);
}

//----------------------------------------------------------------------------
/**
 *	@brief	描画BOXデータ
 */
//-----------------------------------------------------------------------------

#if 0
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

#else

#define	Z_POS	(8)
#define	C_Z_POS	(8)

static	const	fx32	BoxData[]={
	 GX_BEGIN_TRIANGLES,
	 4,
	 //2
	 0,-FX32_SIN45+1,FX32_COS45-1,
	 0,0,
	-FX32_HALF,-FX32_HALF,-FX32_HALF,
	 64*FX32_ONE,0,
	 FX32_HALF,-FX32_HALF,-FX32_HALF,
	 32,64*FX32_ONE,
	 0,-FX32_ONE,0,

	 //14
	 FX32_COS45-1,-FX32_SIN45+1,0,
	 0,0,
	 FX32_HALF,-FX32_HALF,-FX32_HALF,
	 64*FX32_ONE,0,
	 FX32_HALF,-FX32_HALF,FX32_HALF,
	 32,64*FX32_ONE,
	 0,-FX32_ONE,0,

	 //26
	 0,-FX32_SIN45+1,-FX32_COS45+1,
	 0,0,
	 FX32_HALF,-FX32_HALF,FX32_HALF,
	 64*FX32_ONE,0,
	-FX32_HALF,-FX32_HALF,FX32_HALF,
	 32,64*FX32_ONE,
	 0,-FX32_ONE,0,

	 //38
	-FX32_COS45+1,-FX32_SIN45+1,0,
	 0,0,
	-FX32_HALF,-FX32_HALF,FX32_HALF,
	 64*FX32_ONE,0,
	-FX32_HALF,-FX32_HALF,-FX32_HALF,
	 32,64*FX32_ONE,
	 0,-FX32_ONE,0,

	 GX_BEGIN_QUADS,
	 5,
	 0,0,FX32_ONE-1,
	 0,0,
	-FX32_HALF,-FX32_HALF,-FX32_HALF,
	 0,64*FX32_ONE,
	-FX32_HALF, FX32_HALF,-FX32_HALF,
	 64*FX32_ONE,64*FX32_ONE,
	 FX32_HALF, FX32_HALF,-FX32_HALF,
	 64*FX32_ONE,0,
	 FX32_HALF,-FX32_HALF,-FX32_HALF,

	 0,FX32_ONE-1,0,
	 0,64*FX32_ONE,
	-FX32_HALF, FX32_HALF,-FX32_HALF,
	 0,0,
	-FX32_HALF, FX32_HALF,FX32_HALF,
	 64*FX32_ONE,0,
	 FX32_HALF, FX32_HALF,FX32_HALF,
	 64*FX32_ONE,64*FX32_ONE,
	 FX32_HALF, FX32_HALF,-FX32_HALF,

	 0,0,-FX32_ONE+1,
	 64*FX32_ONE,0,
	 FX32_HALF,-FX32_HALF,FX32_HALF,
	 64*FX32_ONE,64*FX32_ONE,
	 FX32_HALF, FX32_HALF,FX32_HALF,
	 0,64*FX32_ONE,
	-FX32_HALF, FX32_HALF,FX32_HALF,
	 0,0,
	-FX32_HALF,-FX32_HALF,FX32_HALF,

	-FX32_ONE+1,0,0,
	 64*FX32_ONE,64*FX32_ONE,
	-FX32_HALF,-FX32_HALF,FX32_HALF,
	 64*FX32_ONE,0,
	-FX32_HALF, FX32_HALF,FX32_HALF,
	 0,0,
	-FX32_HALF, FX32_HALF,-FX32_HALF,
	 0,64*FX32_ONE,
	-FX32_HALF,-FX32_HALF,-FX32_HALF,

	 FX32_ONE-1,0,0,
	 0,0,
	 FX32_HALF,-FX32_HALF,-FX32_HALF,
	 0,64*FX32_ONE,
	 FX32_HALF, FX32_HALF,-FX32_HALF,
	 64*FX32_ONE,64*FX32_ONE,
	 FX32_HALF, FX32_HALF,FX32_HALF,
	 64*FX32_ONE,0,
	 FX32_HALF,-FX32_HALF,FX32_HALF,

	 0xffffffff,
};

fx32	BoxDataGet(int *pos,BOOL flag)
{
	int	data_pos;

	data_pos=pos[0];
	if(flag==TRUE){
		pos[0]++;
	}

	return BoxData[data_pos];
}

#endif

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
	int			plate,line,poly;
	fx32		poly_type;	
	fx16	s;
	fx16	c;
	VecFx32	Eye={0,4*FX32_ONE,FX32_ONE};
	VecFx32	at=	{0,0,-5*FX32_ONE};
	VecFx32	vUp={0,FX32_ONE,0};

	G3X_Reset();

	G3_LookAt(&Eye,&vUp,&at,NULL);

	G3_PushMtx();

	G3_MtxMode(GX_MTXMODE_TEXTURE);
	G3_Identity();
	G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

	G3_TexImageParam(GX_TEXFMT_PLTT16,GX_TEXGEN_TEXCOORD,GX_TEXSIZE_S64,GX_TEXSIZE_T64,
					 GX_TEXREPEAT_NONE,GX_TEXFLIP_NONE,GX_TEXPLTTCOLOR0_USE,0x1000);
	G3_TexPlttBase(0x1000,GX_TEXFMT_PLTT16);

	G3_PolygonAttr(GX_LIGHTMASK_0,GX_POLYGONMODE_MODULATE,GX_CULL_NONE,0,31,GX_POLYGON_ATTR_MISC_NONE);
	G3_Translate(0,-15*FX32_HALF,-24*FX32_ONE);
	G3_Scale(48*FX32_ONE,0,48*FX32_ONE);

	G3_Begin(GX_BEGIN_QUADS);
	G3_Normal(0,FX32_ONE,0);
	G3_Vtx(-FX32_HALF,-FX32_HALF,-FX32_HALF);
	G3_Vtx( FX32_HALF,-FX32_HALF,-FX32_HALF);
	G3_Vtx( FX32_HALF,-FX32_HALF, FX32_HALF);
	G3_Vtx(-FX32_HALF,-FX32_HALF, FX32_HALF);
	G3_End();

	G3_PopMtx(1);

	for(poly=0;poly<POLY_MAX;poly++){

	G3X_Reset();

#if 0
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
	if(GFL_UI_KEY_GetCont()&PAD_BUTTON_Y){
		gt->ofs_x-=0x100;
	}
	if(GFL_UI_KEY_GetCont()&PAD_BUTTON_A){
		gt->ofs_x+=0x100;
	}
	if(GFL_UI_KEY_GetCont()&PAD_BUTTON_SELECT){
		gt->ofs_y-=0x100;
	}
	if(GFL_UI_KEY_GetCont()&PAD_BUTTON_START){
		gt->ofs_y+=0x100;
	}
	if(GFL_UI_KEY_GetCont()&PAD_BUTTON_X){
		gt->ofs_z-=0x100;
	}
	if(GFL_UI_KEY_GetCont()&PAD_BUTTON_B){
		gt->ofs_z+=0x100;
	}
#endif
	gt->rotx[poly]+=ROTATE_VALUE;
	gt->roty[poly]+=ROTATE_VALUE;
	gt->rotz[poly]+=ROTATE_VALUE;
	if(gt->ofs_z[poly]>=0){
		gt->ofs_x[poly]=(15-__GFL_STD_MtRand()%30)*FX32_HALF;
		gt->ofs_y[poly]=(15-__GFL_STD_MtRand()%30)*FX32_HALF;
		gt->ofs_z[poly]=-120*FX32_HALF;
		gt->rotx[poly]=(__GFL_STD_MtRand()%65535);
		gt->roty[poly]=(__GFL_STD_MtRand()%65535);
		gt->rotz[poly]=(__GFL_STD_MtRand()%65535);
		gt->speed[poly]=0x100+(__GFL_STD_MtRand()%0x100);
	}
	else{
		gt->ofs_z[poly]+=gt->speed[poly];
	}

	G3_LookAt(&Eye,&vUp,&at,NULL);

	G3_LightVector(GX_LIGHTID_0,FX16_SQRT1_3,-FX16_SQRT1_3,-FX16_SQRT1_3);
//	G3_LightVector(GX_LIGHTID_0,0,0,-FX16_ONE);
	G3_LightColor(GX_LIGHTID_0,GX_RGB(31,31,31));

	G3_PushMtx();

	G3_MtxMode(GX_MTXMODE_TEXTURE);
	G3_Identity();
	G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

	G3_Translate(gt->ofs_x[poly],gt->ofs_y[poly],gt->ofs_z[poly]);

	s=FX_SinIdx(gt->rotx[poly]);
	c=FX_CosIdx(gt->rotx[poly]);
	G3_RotX(s,c);
	s=FX_SinIdx(gt->roty[poly]);
	c=FX_CosIdx(gt->roty[poly]);
	G3_RotY(s,c);
	s=FX_SinIdx(gt->rotz[poly]);
	c=FX_CosIdx(gt->rotz[poly]);
	G3_RotZ(s,c);

	G3_MaterialColorDiffAmb(GX_RGB(31,31,31),GX_RGB(16,16,16),FALSE);
	G3_MaterialColorSpecEmi(GX_RGB(16,16,16),GX_RGB(0,0,0),FALSE);
	G3_TexImageParam(GX_TEXFMT_PLTT16,GX_TEXGEN_TEXCOORD,GX_TEXSIZE_S64,GX_TEXSIZE_T64,
					 GX_TEXREPEAT_NONE,GX_TEXFLIP_NONE,GX_TEXPLTTCOLOR0_USE,0x1000);
//	G3_TexImageParam(GX_TEXFMT_NONE,GX_TEXGEN_NONE,GX_TEXSIZE_S8,GX_TEXSIZE_T8,
//					 GX_TEXREPEAT_NONE,GX_TEXFLIP_NONE,GX_TEXPLTTCOLOR0_USE,0);
	G3_TexPlttBase(0x1000,GX_TEXFMT_PLTT16);
	G3_PolygonAttr(GX_LIGHTMASK_0,GX_POLYGONMODE_MODULATE,GX_CULL_NONE,0,31,GX_POLYGON_ATTR_MISC_NONE);

#if 1
	plate=0;
	while(1){
		poly_type=BoxDataGet(&plate,TRUE);
		if(poly_type==0xffffffff){
			break;
		}
		G3_Begin(poly_type);
		{
			MtxFx33	mtx33_x;
			MtxFx33	mtx33_y;
			MtxFx33	mtx33_z;
			MtxFx33	mtx33;
			VecFx32	vec;
			VecFx16	vec2;
			VecFx16	eye={0,0,-FX16_ONE*2};
			int		block,block_max;

			block_max=BoxDataGet(&plate,TRUE);

//			MTX_RotX33(&mtx33_x,FX_SinIdx(gt->rotx),FX_CosIdx(gt->rotx));
//			MTX_RotY33(&mtx33_y,FX_SinIdx(gt->roty),FX_CosIdx(gt->roty));
//			MTX_RotZ33(&mtx33_z,FX_SinIdx(gt->rotz),FX_CosIdx(gt->rotz));
//			MTX_Concat33(&mtx33_y,&mtx33_x,&mtx33);
//			MTX_Concat33(&mtx33_z,&mtx33,&mtx33);
			for(block=0;block<block_max;block++){
				vec.x=BoxDataGet(&plate,TRUE);
				vec.y=BoxDataGet(&plate,TRUE);
				vec.z=BoxDataGet(&plate,TRUE);
//				MTX_MultVec33(&vec,&mtx33,&vec);
				vec2.x=vec.x;
				vec2.y=vec.y;
				vec2.z=vec.z;
//				if(VEC_Fx16DotProduct(&eye,&vec2)>0){
					for(line=0;line<((poly_type==GX_BEGIN_QUADS)?4:3);line++){
						G3_TexCoord(BoxDataGet(&plate,TRUE),BoxDataGet(&plate,TRUE));
						G3_Normal(vec2.x,vec2.y,vec2.z);
						G3_Vtx(BoxDataGet(&plate,TRUE),BoxDataGet(&plate,TRUE),BoxDataGet(&plate,TRUE));
					}
//				}
//				else{
//					plate+=((poly_type==GX_BEGIN_QUADS)?4:3)*3;
//				}
			}
		}
		G3_End();
	}
#else
	G3_Begin(GX_BEGIN_QUADS);
	{
		MtxFx33	mtx33_x;
		MtxFx33	mtx33_y;
		MtxFx33	mtx33_z;
		MtxFx33	mtx33;
		VecFx32	vec;
		VecFx16	vec2;
		VecFx16	eye={0,0,FX16_ONE};

		MTX_RotX33(&mtx33_x,FX_SinIdx(gt->rotx),FX_CosIdx(gt->rotx));
		MTX_RotY33(&mtx33_y,FX_SinIdx(gt->roty),FX_CosIdx(gt->roty));
		MTX_RotZ33(&mtx33_z,FX_SinIdx(gt->rotz),FX_CosIdx(gt->rotz));
		MTX_Concat33(&mtx33_y,&mtx33_x,&mtx33);
		MTX_Concat33(&mtx33_z,&mtx33,&mtx33);

		for(plate=0;plate<SQUARE_MAX;plate++){
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

		MTX_RotX33(&mtx33_x,FX_SinIdx(gt->rotx),FX_CosIdx(gt->rotx));
		MTX_RotY33(&mtx33_y,FX_SinIdx(gt->roty),FX_CosIdx(gt->roty));
		MTX_RotZ33(&mtx33_z,FX_SinIdx(gt->rotz),FX_CosIdx(gt->rotz));
		MTX_Concat33(&mtx33_y,&mtx33_x,&mtx33);
		MTX_Concat33(&mtx33_z,&mtx33,&mtx33);

		for(plate=0;plate<TRIANGLE_MAX;plate++){
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
#endif
	G3_PopMtx(1);
	}

	G3_SwapBuffers(GX_SORTMODE_AUTO,GX_BUFFERMODE_W);
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

	G3_Perspective(FX32_SIN45,FX32_COS45,FX32_ONE*4/3,FX32_ONE,FX32_ONE*900,NULL);

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

	G3_SwapBuffers(GX_SORTMODE_MANUAL,GX_BUFFERMODE_Z);
}
#endif
#endif
