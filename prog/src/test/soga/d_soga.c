//============================================================================================
/**
 * @file	d_soga.c
 * @brief	デバッグ用関数
 * @author	soga
 * @date	2008.09.09
 */
//============================================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include <textprint.h>

#include <npBphImporter.h>
#include <npBpcImporter.h>
#include <nplibc.h>

#include "system/main.h"
#include "arc_def.h"

#include "mcss.h"
#include "pokegra_wb.naix"

#define	CAMERA_SPEED		( FX32_ONE * 2 )
#define	ROTATE_SPEED		( 65536 / 100 ) 
#define	PHI_MAX				( ( 65536 / 360 ) * 89 )
#define	DEFAULT_PHI			( 0 )
#define	DEFAULT_THETA		( 65536 / 4 )
#define	DEFAULT_SCALE		( FX32_ONE * 50 )
#define	SCALE_SPEED			( FX32_ONE )
#define	SCALE_MAX			( FX32_ONE * 200 )

#define	SWITCH_TIME			( 60 * 5 )

#define	INTERVAL		(16)
#define	CH_MAX			(1)
#define	STRM_BUF_SIZE	(INTERVAL*2*CH_MAX*32)
#define	SWAV_HEAD_SIZE	(18)

#define DEF_BPCFILE "/BMSample/aura_02.bpc"

enum {
	DEF_NP_TEXTURE_MAX =	 16,
	DEF_ENTRYTEX_MAX =		  3,
};

//32バイトアライメントでヒープからの取り方がわからないので、とりあえず静的に
static	u8				strmBuffer[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);

typedef struct {
	npCONTEXT	ctx;
	npSYSTEM	sys;
	void		*pvBuf;
	int			textureCount;

 	npPARTICLEEMITCYCLE *pEmit;
	npPARTICLECOMPOSITE *pComp;
	npTEXTURE*			pTex[ DEF_NP_TEXTURE_MAX ];

	HEAPID		heapID;
}PARTICLE_WORK;


typedef struct
{
	VecFx32				camPos;
	VecFx32				camUp;
	VecFx32				target;
	int					phi;
	int					theta;
	fx32				scale;
	GFL_G3D_CAMERA		*camera;
}CAMERA_WORK;

typedef struct
{
	int					seq_no;
	MCSS_SYS_WORK		*mcss_sys;
	MCSS_WORK			*mcss;
	HEAPID				heapID;
	GFL_G3D_RES			*model_resource;
	GFL_G3D_RND			*model_render;
	GFL_G3D_OBJ			*model_obj;
	GFL_G3D_OBJSTATUS	model_status;
	PARTICLE_WORK		*pw;
	CAMERA_WORK			cw;
	NNSSndStrm			strm;
	u8					FS_strmBuffer[STRM_BUF_SIZE];
	int					FSReadPos;
	u32					strmReadPos;
	u32					strmWritePos;
	int					visible_flag;
	int					timer;
}SOGA_WORK;

//カメラ
static	void	InitCamera( CAMERA_WORK *cw, HEAPID heapID );
static	void	MoveCamera( CAMERA_WORK *cw );
static	void	CalcCamera( CAMERA_WORK *cw );

int				rotate=0;

//ストリーム再生
static	void	StrmSetUp(SOGA_WORK *sw);
static	void	StrmCBWaveDataStock(NNSSndStrmCallbackStatus status,
									int numChannels,
									void *buffer[],
									u32	len,
									NNSSndStrmFormat format,
									void *arg);
static	void	StrmBufferCopy(SOGA_WORK *sw,int size);

//パーティクル
static void ParticleInit( PARTICLE_WORK *pw );
static void ParticleDraw( PARTICLE_WORK *pw );
static void ParticleExit( PARTICLE_WORK *pw );
static void Bmt_initSystems( void );
static BOOL Bmt_initEffect( npCONTEXT *pCtx, npSYSTEM *pSys, void **ppvBuf, HEAPID heapID);
static int Bmt_loadTextures( npCONTEXT* pCtx, npTEXTURE** pTex, npPARTICLEEMITCYCLE** ppEmit, npPARTICLECOMPOSITE** ppComp, HEAPID heapID );
static void Bmt_update( npCONTEXT* ctx, npPARTICLECOMPOSITE* pComp, npTEXTURE** pTex );
static void Bmt_releaseTextures( npPARTICLECOMPOSITE* pComp, npCONTEXT* pCtx, npTEXTURE** pTex, int textureCount );
static void Bmt_releaseEffect( npCONTEXT *pCtx, npSYSTEM *pSys, void **ppvBuf);
static void bmt_loadBpc( const char *pszBpc, npPARTICLEEMITCYCLE **ppEmit, HEAPID heapID );
static void bmt_loadTex(const char *pszTex, const char *pszPlt, npTEXTURE *pTex, npU32* texSize, npU32* palSize, HEAPID heapID );
static void bmt_Mtx43ToMtx44(MtxFx44 *pDst, MtxFx43 *pTrc);
static void bmt_updateEffect( npPARTICLECOMPOSITE* pComp,npU32 renew );

//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugSogabeMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	SOGA_WORK* wk;
	u8 chNoList[1]={0};

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SOGABE_DEBUG, 0xc0000 );
	wk = GFL_PROC_AllocWork( proc, sizeof( SOGA_WORK ), HEAPID_SOGABE_DEBUG );
	MI_CpuClearFast( wk, sizeof( SOGA_WORK ) );
	wk->heapID = HEAPID_SOGABE_DEBUG;
		
	{
		static const GFL_BG_DISPVRAM dispvramBank = {
			GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
			GX_VRAM_SUB_BG_32_H,			// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
			GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
			GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
			GX_VRAM_TEX_01_BC,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_01_FG			// テクスチャパレットスロット			
		};		
		GFL_DISP_SetBank( &dispvramBank );
	}	
	
	G2_BlendNone();
	GFL_BG_Init( wk->heapID );
	GFL_BMPWIN_Init( wk->heapID );

	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_3D,
		};
		GFL_BG_SetBGMode( &sysHeader );
	}
		
	{
		///< main
		GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

		///< sub
		GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_OFF );
		
	}		
	GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x7f00 );

	//3D関連初期化
	{
		GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K, GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0, wk->heapID, NULL );
		GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );
		InitCamera( &wk->cw, wk->heapID );
	}

	wk->seq_no = 0;
	wk->mcss_sys = MCSS_Init( 3, wk->cw.camera, wk->heapID );

#if 1
	MCSS_Add( wk->mcss_sys,
//			  0x00040000,0x0001a000,0xfff60000,
//			  0x00040000,0x00038000,0xfff80000,
			  0x00028000,0x0002a000,0xffff0000,
			  ARCID_POKEGRA,
			  NARC_pokegra_wb_multi_test_NCBR,
			  NARC_pokegra_wb_multi_test_NCLR,
			  NARC_pokegra_wb_multi_test_NCER,
			  NARC_pokegra_wb_multi_test_NANR,
			  NARC_pokegra_wb_multi_test_NMCR,
			  NARC_pokegra_wb_multi_test_NMAR,
			  NARC_pokegra_wb_multi_test_NCEC );
	wk->mcss = MCSS_Add( wk->mcss_sys,
			  0x00060000,0x00018000,0x00010000,
			  ARCID_POKEGRA,
			  NARC_pokegra_wb_multi_test3_NCBR,
			  NARC_pokegra_wb_multi_test3_NCLR,
			  NARC_pokegra_wb_multi_test3_NCER,
			  NARC_pokegra_wb_multi_test3_NANR,
			  NARC_pokegra_wb_multi_test3_NMCR,
			  NARC_pokegra_wb_multi_test3_NMAR,
			  NARC_pokegra_wb_multi_test3_NCEC );
#endif
#if 1
	MCSS_Add( wk->mcss_sys,
//			  0xfffd0000,0xfffd8000,0xfff6a000,
//			  0xfffd0000,0x00048000,0xfffc0000,
			  0xfffd0000,0x0003a000,0x00020000,
			  ARCID_POKEGRA,
			  NARC_pokegra_wb_multi_test2_NCBR,
			  NARC_pokegra_wb_multi_test2_NCLR,
			  NARC_pokegra_wb_multi_test2_NCER,
			  NARC_pokegra_wb_multi_test2_NANR,
			  NARC_pokegra_wb_multi_test2_NMCR,
			  NARC_pokegra_wb_multi_test2_NMAR,
			  NARC_pokegra_wb_multi_test2_NCEC );
#endif

	//3D Model Load
	{
		BOOL	ret;

		//リソース読み込み
//		wk->model_resource = GFL_G3D_CreateResourceArc( ARCID_POKEGRA, NARC_pokegra_wb_batt_testmap01c_nsbmd );
		wk->model_resource = GFL_G3D_CreateResourceArc( ARCID_POKEGRA, NARC_pokegra_wb_test_battle_nsbmd );
		ret = GFL_G3D_TransVramTexture( wk->model_resource );
		GF_ASSERT( ret == TRUE );
		//RENDER生成
		wk->model_render = GFL_G3D_RENDER_Create( wk->model_resource, 0, wk->model_resource );
		//OBJ生成
		wk->model_obj = GFL_G3D_OBJECT_Create( wk->model_render, NULL, 0 );

		wk->model_status.trans.x = 0;
		wk->model_status.trans.y = 0;
		wk->model_status.trans.z = 0;
		MTX_Identity33(&wk->model_status.rotate);
		wk->model_status.scale.x = FX32_ONE;
		wk->model_status.scale.y = FX32_ONE;
		wk->model_status.scale.z = FX32_ONE;
	}

	//2D画面初期化
	{
		GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
			///<FRAME1_M
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
			///<FRAME2_M
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
		};
		GFL_BG_SetBGControl(GFL_BG_FRAME1_M, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME1_M );
		GFL_BG_SetBGControl(GFL_BG_FRAME2_M, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME2_M );

		GFL_ARC_UTIL_TransVramBgCharacter(ARCID_POKEGRA,NARC_pokegra_wb_batt_bg1_NCGR,GFL_BG_FRAME1_M,0,0,0,wk->heapID);
		GFL_ARC_UTIL_TransVramScreen(ARCID_POKEGRA,NARC_pokegra_wb_batt_bg1_NSCR,GFL_BG_FRAME1_M,0,0,0,wk->heapID);
		GFL_ARC_UTIL_TransVramBgCharacter(ARCID_POKEGRA,NARC_pokegra_wb_batt_bg2_NCGR,GFL_BG_FRAME2_M,0,0,0,wk->heapID);
		GFL_ARC_UTIL_TransVramScreen(ARCID_POKEGRA,NARC_pokegra_wb_batt_bg2_NSCR,GFL_BG_FRAME2_M,0,0,0,wk->heapID);
		GFL_ARC_UTIL_TransVramPalette(ARCID_POKEGRA,NARC_pokegra_wb_batt_bg_NCLR,PALTYPE_MAIN_BG,0,0x100,wk->heapID);
	}

	//ストリーム再生初期化
    NNS_SndInit();
    NNS_SndStrmInit( &wk->strm );
	NNS_SndStrmAllocChannel( &wk->strm, CH_MAX, chNoList);

	StrmSetUp(wk);

	NNS_SndStrmStart(&wk->strm);
	
	//パーティクル初期化
	wk->pw = GFL_HEAP_AllocMemory( wk->heapID, sizeof(PARTICLE_WORK) );
	wk->pw->heapID = wk->heapID;
	ParticleInit(wk->pw);

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugSogabeMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	int pad = GFL_UI_KEY_GetCont();
	SOGA_WORK* wk = mywk;

	//画面切り替え実験
	wk->timer++;
	if(wk->timer > SWITCH_TIME ){
		wk->timer=0;
		wk->visible_flag ^= 1;
		if( wk->visible_flag ){
			GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
			GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_ON );
			GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_ON );
		}
		else{
			GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
			GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
			GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
		}
	}

	StrmBufferCopy(wk,0);

	if( pad & PAD_BUTTON_A ){
		wk->mcss->mepachi_flag = 1;
	}
	else{
		wk->mcss->mepachi_flag = 0;
	}

	MoveCamera( &wk->cw );

	MCSS_Main( wk->mcss_sys );

	//3D描画
	{
		GFL_G3D_DRAW_Start();
		GFL_G3D_CAMERA_Switching( wk->cw.camera );
		{
			GFL_G3D_DRAW_DrawObject( wk->model_obj, &wk->model_status );
			MCSS_Draw( wk->mcss_sys );
			ParticleDraw(wk->pw);
		}
		GFL_G3D_DRAW_End();
	}

	return GFL_PROC_RES_CONTINUE;	
}
//--------------------------------------------------------------------------
/**
 * PROC Exit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugSogabeMainProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA		DebugSogabeMainProcData = {
	DebugSogabeMainProcInit,
	DebugSogabeMainProcMain,
	DebugSogabeMainProcExit,
};

 
//======================================================================
//	カメラの初期化
//======================================================================
static	void	InitCamera( CAMERA_WORK *cw, HEAPID heapID )
{
	cw->target.x = 0;
	cw->target.y = 0;
	cw->target.z = 0;
	cw->camUp.x	= 0;
	cw->camUp.y	= FX32_ONE;
	cw->camUp.z	= 0;

	cw->phi		= DEFAULT_PHI;
	cw->theta	= DEFAULT_THETA;
	cw->scale	= DEFAULT_SCALE;

	CalcCamera( cw );

	cw->camera = GFL_G3D_CAMERA_Create( GFL_G3D_PRJPERS, 
										FX32_SIN30,
										FX32_COS30,
										FX32_ONE * 4 / 3,
										NULL,
										FX32_ONE,
										FX32_ONE * 1000,
										NULL,
										&cw->camPos,
										&cw->camUp,
										&cw->target,
										heapID );
}

//======================================================================
//	カメラの制御
//======================================================================
static	void	MoveCamera( CAMERA_WORK *cw )
{
	int pad = GFL_UI_KEY_GetCont();
	VecFx32	pos,ofsx,ofsz;
	fx32	xofs=0,yofs=0,zofs=0;		

	if( pad & PAD_BUTTON_Y ){
		if( pad & PAD_KEY_LEFT ){
			cw->theta -= ROTATE_SPEED;
		}
		if( pad & PAD_KEY_RIGHT ){
			cw->theta += ROTATE_SPEED;
		}
		if( pad & PAD_KEY_UP ){
			cw->phi -= ROTATE_SPEED;
			if( cw->phi <= -PHI_MAX ){
				cw->phi = -PHI_MAX;
			}
		}
		if( pad & PAD_KEY_DOWN ){
			cw->phi += ROTATE_SPEED;
			if( cw->phi >= PHI_MAX ){
				cw->phi = PHI_MAX;
			}
		}
	}
	else{
		if( pad & PAD_KEY_LEFT ){
			xofs = -FX32_ONE;
		}
		if( pad & PAD_KEY_RIGHT ){
			xofs = FX32_ONE;
		}
		if( pad & PAD_KEY_UP ){
			yofs = FX32_ONE;
		}
		if( pad & PAD_KEY_DOWN ){
			yofs = -FX32_ONE;
		}
		if( pad & PAD_BUTTON_X ){
			zofs = FX32_ONE;
		}
		if( pad & PAD_BUTTON_B ){
			zofs = -FX32_ONE;
		}
		pos.x = cw->camPos.x - cw->target.x;
		pos.y = 0;
		pos.z = cw->camPos.z - cw->target.z;
		VEC_Normalize( &pos, &pos );

		ofsx.x = FX_MUL( pos.z, xofs );
		ofsx.y = 0;
		ofsx.z = -FX_MUL( pos.x, xofs );

		ofsz.x = -FX_MUL( pos.x, zofs );
		ofsz.y = yofs;
		ofsz.z = -FX_MUL( pos.z, zofs );

		cw->target.x += ofsx.x + ofsz.x;
		cw->target.y += ofsx.y + ofsz.y;
		cw->target.z += ofsx.z + ofsz.z;
	}
	if( pad & PAD_BUTTON_L ){
		if( cw->scale > 0 ){
			cw->scale -= SCALE_SPEED;
		}
	}
	if( pad & PAD_BUTTON_R ){
		if( cw->scale < SCALE_MAX ){
			cw->scale += SCALE_SPEED;
		}
	}
	CalcCamera( cw );
	GFL_G3D_CAMERA_SetPos( cw->camera, &cw->camPos );
	GFL_G3D_CAMERA_SetTarget( cw->camera, &cw->target );
}

//======================================================================
//	カメラ位置の計算
//======================================================================
static	void	CalcCamera( CAMERA_WORK *cw )
{
	MtxFx43	scale,trans;
	VecFx32	pos;

	//phiとthetaとscaleからcamPosを計算
	pos.x = FX_MUL( FX_CosIdx( cw->theta & 0xffff ), FX_CosIdx( cw->phi & 0xffff ) );
	pos.y = FX_SinIdx( cw->phi & 0xffff );
	pos.z = FX_MUL( FX_SinIdx( cw->theta & 0xffff ), FX_CosIdx( cw->phi & 0xffff ) );
	MTX_Scale43( &scale, cw->scale, cw->scale, cw->scale );
	MTX_MultVec43( &pos, &scale, &pos );
	MTX_Identity43( &trans );
	MTX_TransApply43( &trans, &trans, cw->target.x,	cw->target.y, cw->target.z );
	MTX_MultVec43( &pos, &trans, &cw->camPos );
}

//---------------------------------------------------------------------------
//	ストリーム再生関数
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static	void	StrmSetUp(SOGA_WORK *sw)
{
	sw->FSReadPos=SWAV_HEAD_SIZE;
	sw->strmReadPos=0;
	sw->strmWritePos=0;
	StrmBufferCopy(sw,STRM_BUF_SIZE);
    
	NNS_SndStrmSetup( &sw->strm,
					  NNS_SND_STRM_FORMAT_PCM8,
					  &strmBuffer[0],
					  STRM_BUF_SIZE,
					  NNS_SND_STRM_TIMER_CLOCK/8000,
					  INTERVAL,
					  StrmCBWaveDataStock,
					  sw);
}

//---------------------------------------------------------------------------
// 波形データを補充するコールバックが割り込み禁止で呼ばれるので、
// FS_～が中から呼べないので、この関数でReadしておく
//---------------------------------------------------------------------------
static	void	StrmBufferCopy(SOGA_WORK *sw,int size)
{
	FSFile	file;
	s32		ret;

    OS_TPrintf("---\n");
    FS_InitFile(&file);
	FS_OpenFile(&file,"/pm_battle.swav");
	FS_SeekFile(&file,sw->FSReadPos,FS_SEEK_SET);

	if(size){
		ret=FS_ReadFile(&file,&sw->FS_strmBuffer[0],size);
		sw->FSReadPos+=size;
	}
	else{
		while(sw->strmReadPos!=sw->strmWritePos){
			ret=FS_ReadFile(&file,&sw->FS_strmBuffer[sw->strmWritePos],32);
			if(ret==0){
				sw->FSReadPos=SWAV_HEAD_SIZE;
				FS_SeekFile(&file,sw->FSReadPos,FS_SEEK_SET);
				continue;
			}
			sw->FSReadPos+=32;
			sw->strmWritePos+=32;
			if(sw->strmWritePos>=STRM_BUF_SIZE){
				sw->strmWritePos=0;
			}
		}
	}
	FS_CloseFile(&file);
}

//---------------------------------------------------------------------------
// 波形データを補充するコールバック
//---------------------------------------------------------------------------
static	void	StrmCBWaveDataStock(NNSSndStrmCallbackStatus status,
									int numChannels,
									void *buffer[],
									u32	len,
									NNSSndStrmFormat format,
									void *arg)
{
	SOGA_WORK *sw=(SOGA_WORK *)arg;
	int	i;
	u8	*strBuf;

	strBuf=buffer[0];
	
	for(i=0;i<len;i++){
		strBuf[i]=sw->FS_strmBuffer[i+sw->strmReadPos];
	}

	sw->strmReadPos+=len;
	if(sw->strmReadPos>=STRM_BUF_SIZE){
		sw->strmReadPos=0;
	}
    OS_TPrintf("count %d \n",len);

	DC_FlushRange(strBuf,len);
}

//
//	パーティクル描画実験
//

static void ParticleInit( PARTICLE_WORK *pw )
{
	Bmt_initSystems();
	Bmt_initEffect( &pw->ctx, &pw->sys, &pw->pvBuf, pw->heapID );
	OS_TPrintf("BM init effect OK.\n");
	pw->textureCount = Bmt_loadTextures( &pw->ctx, &pw->pTex[0], &pw->pEmit, &pw->pComp, pw->heapID );
	OS_TPrintf("BM load texture OK.\n");
	npSetPolygonIDMin( &pw->ctx, 30 );
	npSetPolygonIDMax( &pw->ctx, 50 );
}

static void ParticleDraw( PARTICLE_WORK *pw )
{
	Bmt_update( &pw->ctx, pw->pComp, &pw->pTex[0] );
}

static void ParticleExit( PARTICLE_WORK *pw )
{
	Bmt_releaseTextures( pw->pComp, &pw->ctx, &pw->pTex[0], pw->textureCount );
	OS_TPrintf("BM init release texture  OK.\n");
	Bmt_releaseEffect( &pw->ctx, &pw->sys, &pw->pvBuf );
	OS_TPrintf("BM init release effect  OK.\n");
}

// DS描画系初期化
static void Bmt_initSystems( void )
{
   // G3X_Init();                        // initialize the 3D graphics states
   // G3X_InitMtxStack();                // initialize the matrix stack

   // GX_SetBankForTex( GX_VRAM_TEX_0_A );			// VRAM-A for texture images
   // GX_SetBankForTexPltt(GX_VRAM_TEXPLTT_0123_E);	// VRAM-E for texture palettes

	#if 0
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,    // graphics mode
                       GX_BGMODE_4,    // BGMODE is 4
                       GX_BG0_AS_3D);  // BG #0 is for 3D

    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    G2_SetBG0Priority(0);
    #endif

    G3X_SetShading(GX_SHADING_TOON);   // shading mode is toon
    G3X_AntiAlias(TRUE);               // enable antialias(without additional computing costs)
	G3X_AlphaBlend(TRUE);
//    G3_ViewPort(0, 0, 255, 191);       // Viewport

	#if 0
	{
		void *nstart;
		void *heapStart;
		static OSHeapHandle _hdl = NULL;

		nstart = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 2);
		OS_SetMainArenaLo(nstart);
		
	    heapStart = OS_AllocFromMainArenaLo(MAIN_HEAP_SIZE, 32);
		_hdl = OS_CreateHeap(OS_ARENA_MAIN, heapStart, (void *)((u32)heapStart + MAIN_HEAP_SIZE));
		OS_SetCurrentHeap(OS_ARENA_MAIN, _hdl);
	}
	#endif
}

//--------------------------------------------------
// BMシステム初期化:成功時TRUE
//--------------------------------------------------
static BOOL Bmt_initEffect( npCONTEXT *pCtx, npSYSTEM *pSys, void **ppvBuf, HEAPID heapID)
{
	enum {
		TEXTURE_MAX = 			 32,
		SHADER_MAX = 			 16,
		EFF_COMPONENT_MAX = 	128,
		EFF_SUBJECT_MAX = 		128,
		EFF_FKEY_MAX = 			256,
	};

	npSYSTEMDESC	sysDesc;
	npSIZE			nSize	= 0;
	static npBYTE*			pByte = NP_NULL;
	npERROR			nResult;

	void *pvNativeWnd = NULL;
	void *pvNativeDc = NULL;
	void *pvNativeRc = NULL;

	/* initialize nplibc */
	MI_CpuClear8(&sysDesc, sizeof(npSYSTEMDESC));
	sysDesc.MaxContext = 1;

	nSize = npCheckSystemHeapSize( &sysDesc )			+
		npCheckTextureLibraryHeapSize( TEXTURE_MAX )	+
		npParticleCheckComponentHeapSize( EFF_COMPONENT_MAX ) +
		npParticleCheckFKeyHeapSize( EFF_FKEY_MAX )	+ 
		npParticleCheckSubjectHeapSize( EFF_SUBJECT_MAX );

	OS_TPrintf("BM必要バッファサイズ=0x%x bytes\n", nSize);

	*ppvBuf = GFL_HEAP_AllocMemory( heapID, nSize );
	pByte = (npBYTE*)(*ppvBuf);//ポインタを渡しておく

	/* memory size */
	nSize = npCheckSystemHeapSize( &sysDesc );

	/* initialize system */
	nResult = npInitSystem( NP_NULL, &sysDesc, pByte, nSize, &pSys );

	if(nResult != NP_SUCCESS) {
		OS_TPrintf("FAILED : npInitSystem\n");
		return FALSE;
	}

	/* create rendering context	*/
	nResult = npCreateContext( pSys, NP_NULL, &pCtx );

	if(nResult != NP_SUCCESS) {
		OS_TPrintf("FAILED : npCreateContext\n");
		return FALSE;
	}

	pCtx->m_nWidth = 256;
	pCtx->m_nHeight = 192;

	/* reset rendering context */
	nResult = npResetEnvironment( pCtx );

	if ( nResult != NP_SUCCESS )
	{
		return FALSE;
	}

	// BackGround Color
	(void)npSetClearColor(pCtx, 0xFFFF);

	/* set render state */
	npSetRenderState( pCtx, NP_SHADE_CULLMODE, NP_CULL_NONE );

	pByte += nSize;

	/* initialize texture library */
	nSize = npCheckTextureLibraryHeapSize( TEXTURE_MAX );
	nResult = npInitTextureLibrary( pCtx, pByte, nSize );
	if ( nResult != NP_SUCCESS )
	{
		OS_Printf("ERROR: TextureLibrary\n");
		return FALSE;
	}
	pByte += nSize;

	/* initialize particle component */
	nSize = npParticleCheckComponentHeapSize( EFF_COMPONENT_MAX );
	nResult = npParticleInitComponentFactory( pByte, nSize );
	if(nResult != NP_SUCCESS) {
		OS_Printf("ERROR: ParticleComponent\n");
		return FALSE;
	}
	pByte += nSize;

	/* initialize particle key frame  */
	nSize = npParticleCheckFKeyHeapSize( EFF_FKEY_MAX );
	nResult = npParticleInitFKeyFactory( pByte, nSize );
	if(nResult != NP_SUCCESS) {
		OS_Printf("ERROR: ParticleComponent\n");
		return FALSE;
	}
	pByte += nSize;

	/* initialize particle subject */
	nSize = npParticleCheckSubjectHeapSize( EFF_SUBJECT_MAX );
	nResult = npParticleInitSubjectFactory( pByte, nSize );
	if(nResult != NP_SUCCESS) {
		OS_Printf("ERROR: ParticleSubject\n");
		return FALSE;
	}
	pByte += nSize;

	return TRUE;
}
//--------------------------------------------------
// BMシステム解放
//--------------------------------------------------
static void Bmt_releaseEffect( npCONTEXT *pCtx, npSYSTEM *pSys, void **ppvBuf)
{
	// subject memory の開放
	npParticleExitSubjectFactory();
	// key frame memory の開放
	npParticleExitFkeyFactory();
	// component memory の開放
	npParticleExitComponentFactory();
	// texture memory の開放
	npExitTextureLibrary( pCtx );
	// context の開放
	npReleaseContext( pCtx );
	// system の開放
	npExitSystem( pSys );
	
	GFL_HEAP_FreeMemory( *ppvBuf );
	*ppvBuf = NULL;
}
//--------------------------------------------------
// テクスチャロード : return = 読み込みテクスチャ数
//--------------------------------------------------
static int Bmt_loadTextures( npCONTEXT* pCtx, npTEXTURE** pTex, npPARTICLEEMITCYCLE** ppEmit, npPARTICLECOMPOSITE** ppComp, HEAPID heapID )
{
	typedef struct __TEX_LIST
	{
		char		cNtftName[32];
		char		cNtfpName[32];
		npU32			width;
		npU32			height;
		GXTexFmt	fomat;
	} TEX_LIST;

	static const TEX_LIST texList[] = 
	{
		{ "/BmSample/aura_01.ntft"			, "/BmSample/aura_01.ntfp"			, 32, 64,	GX_TEXFMT_A5I3} ,
		{ "/BmSample/sphere_06.ntft"		, "/BmSample/sphere_06.ntfp"		, 256, 256,	GX_TEXFMT_A5I3} ,
		{ "/BmSample/sphere_hole_01.ntft"	, "/BmSample/sphere_hole_01.ntfp"	, 32, 32,	GX_TEXFMT_A5I3} ,
	};

	npU32 ntftAddr = 0x1c000;
	npU32 ntfpAddr = 0x1000;
	npU32 texSize, palSize;
	int i;

	bmt_loadBpc( DEF_BPCFILE, ppEmit, heapID );

	npParticleCreateComposite( *ppEmit, ppComp );

	// テクスチャファイル・パレットファイルのサイズに注意
	for(i=0; i<DEF_ENTRYTEX_MAX; i++)
	{
		
		npCreateTextureFromRef( pCtx, NULL, &pTex[i] );
		npSetTextureParam(pTex[i], texList[i].width, texList[i].height, texList[i].fomat, ntftAddr, ntfpAddr);	
		bmt_loadTex( texList[i].cNtftName, texList[i].cNtfpName, pTex[i], &texSize, &palSize, heapID );
		ntftAddr += texSize;
		ntfpAddr += palSize;
	}
	return i;
}


static void bmt_loadBpc( const char *pszBpc, npPARTICLEEMITCYCLE **ppEmit, HEAPID heapID )
{
	FSFile file;
	unsigned int uiSize;
	void *pv;

	NP_BPC_DESC desc;
	desc.coordinateSystem = NP_BPC_RIGHT_HANDED;

	FS_InitFile(&file);
	if(!FS_OpenFile(&file, pszBpc)) {
		OS_Printf("Cannot Open File %s\n", pszBpc);
		GF_ASSERT(0);
		return;
	}

	uiSize = FS_GetLength(&file);
	pv = GFL_HEAP_AllocMemory( heapID, uiSize );

	FS_ReadFile( &file, pv, (long)uiSize );
	FS_CloseFile(&file);

	if( !npUtilParticleLoadBpc(pv, uiSize, &desc, ppEmit))
	{
		OS_TPrintf("FAILED: npUtilParticleLoadBpc\n");
		GF_ASSERT(0);
		return;
	}

	GFL_HEAP_FreeMemory( pv );
}

static void bmt_loadTex(const char *pszTex, const char *pszPlt, npTEXTURE *pTex, npU32* texSize, npU32* palSize, HEAPID heapID )
{
	FSFile file;
	int hasPalette = 0;

	*texSize = *palSize = 0;

	// Open Texture File
	FS_InitFile(&file);
	if(FS_OpenFile(&file, pszTex)) {
		u32 uReadSize = FS_GetLength(&file);
		u32 uTexSize = ((uReadSize+3)/4)*4;
		void *pvTex = GFL_HEAP_AllocMemory( heapID, uTexSize );
		*texSize = uTexSize;
		if(pvTex)
		{
			FS_ReadFile( &file, pvTex, (long)uReadSize );
			FS_CloseFile( &file );

			DC_FlushRange(pvTex, uTexSize);
		    GX_BeginLoadTex();
			GX_LoadTex(pvTex, pTex->m_TexAddr, uTexSize);
		    GX_EndLoadTex();

		    GFL_HEAP_FreeMemory( pvTex );
		    pvTex = NULL;
		}
		else
		{
    		OS_TPrintf("CANNOT ALLOCATE MEMORY\n");
    		GF_ASSERT(0);
		}
	}
	else {
		OS_TPrintf("CANNOT OPEN FILE %s\n", pszTex);
		GF_ASSERT(0);
		return;
	}

	switch(pTex->m_TexFmt) {
	case GX_TEXFMT_PLTT4:
	case GX_TEXFMT_PLTT16:
	case GX_TEXFMT_PLTT256:
	case GX_TEXFMT_A3I5:
	case GX_TEXFMT_A5I3:
		hasPalette = 1;
		break;
	default:
		hasPalette = 0;
		break;
	}
	
	// Open Palette File
	if(hasPalette) {
		FS_InitFile(&file);
		if(FS_OpenFile(&file, pszPlt)) {
			u32 uReadSize = FS_GetLength(&file);
			u32 uPltSize = ((uReadSize+3)/4)*4;
			void *pvPlt = GFL_HEAP_AllocMemory( heapID, uPltSize );
			*palSize = uPltSize;
			if(pvPlt) {
				FS_ReadFile(&file, pvPlt, (long)uReadSize);
				FS_CloseFile(&file);

				// LoadPalette
				DC_FlushRange(pvPlt, uPltSize);
				GX_BeginLoadTexPltt();
			    GX_LoadTexPltt(pvPlt, pTex->m_PltAddr, uPltSize);
			    GX_EndLoadTexPltt();

			    GFL_HEAP_FreeMemory( pvPlt );
			    pvPlt = NULL;
			}
			else
			{
	    		OS_Printf("CANNOT ALLOCATE MEMORY\n");
	    		GF_ASSERT(0);
			}
		}
		else {
			OS_Printf("CANNOT OPEN FILE %s\n", pszPlt);
			GF_ASSERT(0);
			return;
		}
	}
}
//--------------------------------------------------
// テクスチャ解放
//--------------------------------------------------
static void Bmt_releaseTextures( npPARTICLECOMPOSITE* pComp, npCONTEXT* pCtx, npTEXTURE** pTex, int textureCount )
{
	int i;

	npParticleReleaseComposite( pComp );

	for(i=0; i<textureCount; i++)
	{
		npReleaseTexture( pCtx, pTex[i] );
	}
}
//--------------------------------------------------
// 描画更新
//--------------------------------------------------
static void Bmt_update( npCONTEXT* ctx, npPARTICLECOMPOSITE* pComp, npTEXTURE** pTex )
{
	MtxFx44 matProj, matView;
	MtxFx43 matView43;
	npSIZE	nPolyMax = 63;
	npSIZE	nPolyMin = 0;

#if 0
	G3X_Reset();

	//---------------------------------------------------------------------------
	// Set up a camera matrix
	//---------------------------------------------------------------------------
	{
		VecFx32 Eye = { 0*FX32_ONE, 0, 10*FX32_ONE };   // Eye position
		VecFx32 at = { 0, 0, 0 };  // Viewpoint
		VecFx32 vUp = { 0, FX32_ONE, 0 };   // Up

		fx32 fovSin = FX_SinIdx(182 * 45);
		fx32 fovCos = FX_CosIdx(182 * 45);
		fx32 aspect = FX_F32_TO_FX32(1.33333333f);
		fx32 n = FX32_ONE;
		fx32 f = 500 * FX32_ONE;

		G3_Perspective(fovSin, fovCos, aspect, n, f, &matProj);
		G3_LookAt(&Eye, &vUp, &at, &matView43);

		MTX_Identity44(&matView);
		bmt_Mtx43ToMtx44(&matView, &matView43);
	}
#endif

    G3_PushMtx();
    G3_MtxMode(GX_MTXMODE_TEXTURE);
    G3_Identity();
    // Use an identity matrix for the texture matrix for simplicity
    G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

	npSetTransformFMATRIX( ctx, NP_PROJECTION, (npFMATRIX *)NNS_G3dGlbGetProjectionMtx() );
	npSetTransformFMATRIX( ctx, NP_VIEW, (npFMATRIX *)NNS_G3dGlbGetCameraMtx() );

	// UPDATE & RENDER
	{
		bmt_updateEffect( pComp, NP_NTSC60 );
		npParticleRenderComposite( ctx, pComp, pTex, DEF_ENTRYTEX_MAX );
	}

	G3_PopMtx(1);

	// swapping the polygon list RAM, the vertex RAM, etc.
//	G3_SwapBuffers(GX_SORTMODE_MANUAL, GX_BUFFERMODE_W);
}

static void bmt_Mtx43ToMtx44(MtxFx44 *pDst, MtxFx43 *pTrc)
{
	int i, j;
	for(i=0; i<4; i++) {
		for(j=0; j<3; j++) {
			pDst->m[i][j] = pTrc->m[i][j];
		}
	}
}


static void bmt_updateEffect( npPARTICLECOMPOSITE* pComp,npU32 renew )
{
	MtxFx44	mtx,trans;
	// scalingEffect
	npFMATRIX effMat;

	MTX_Scale44( &mtx, FX32_ONE * 12, FX32_ONE * 4, FX32_ONE * 12 );
	MTX_Identity44( &trans );
//	MTX_TransApply44( &trans, &trans, FX32_ONE * 6 , FX32_ONE * 2, 0 );
	MTX_TransApply44( &trans, &trans, 0x00060000,0x00001000,0x00010000 );
	MTX_Concat44( &mtx, &trans, &mtx );
//	npUnitFMATRIX( (npFMATRIX *)&trans );

//	npParticleSetScaling( pComp, (npFVECTOR *)&scale );
	(void)npParticleSetGlobalFMATRIX((npPARTICLEOBJECT *)pComp, (npFMATRIX *)&mtx);
	(void)npParticleUpdateComposite( pComp, renew );
}

