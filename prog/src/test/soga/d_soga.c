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

#include <nnsys/mcs.h>

#include "system/main.h"
#include "arc_def.h"

#include "system\gfl_use.h"
#include "mcss.h"
#include "pokegra/pokegra_wb.naix"
#include "battgra/battgra_wb.naix"
#include "spa.naix"

//#define MCS_ENABLE		//MCSを使用する

#define	 NINTEN_SPL			//ニンテンパーティクル使用
#define	 NIN_SPL_MAX	(3)

#define	CAMERA_SPEED		( FX32_ONE * 2 )
#define	MOVE_SPEED			( FX32_ONE >> 2 )
#define	ROTATE_SPEED		( 65536 / 128 ) 
#define	PHI_MAX				( ( 65536 / 360 ) * 89 )
#define	DEFAULT_PHI			( 65536 / 12 )
#define	DEFAULT_THETA		( 65536 / 4 )
#define	DEFAULT_SCALE		( FX32_ONE * 16  )
#define	SCALE_SPEED			( FX32_ONE )
#define	SCALE_MAX			( FX32_ONE * 200 )

#define	SWITCH_TIME			( 60 * 5 )

#define	INTERVAL		(16)
#define	CH_MAX			(1)
#define	STRM_BUF_SIZE	(INTERVAL*2*CH_MAX*32)
#define	SWAV_HEAD_SIZE	(18)

//#define DEF_BPCFILE "/BMSample/aura_02.bpc"
#define DEF_BPCFILE "/BMSample/tornado.bpc"

static	const	VecFx32	poke_pos[]={
//	{ 0x00002000, 0x00001000, 0xffffe000 },
//	{ 0xffffe000, 0x00001000, 0xffffe000 },
//	{ 0xffffe000, 0x00001000, 0x00002000 },
	{ FX_F32_TO_FX32( 3.0f ),	FX_F32_TO_FX32( 0.5f ), FX_F32_TO_FX32( -9.0f ) },
	{ FX_F32_TO_FX32( 6.0f ),	FX_F32_TO_FX32( 0.5f ), FX_F32_TO_FX32( -7.0f ) },
//	{ FX_F32_TO_FX32( -2.5f ),	FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 7.0f ) },
	{ FX_F32_TO_FX32( -3.5f ),	FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 7.0f ) },
	{ FX_F32_TO_FX32( -1.5f ),	FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 8.0f ) },
};


enum {
	DEF_NP_TEXTURE_MAX =	 16,
//	DEF_ENTRYTEX_MAX =		  3,
	DEF_ENTRYTEX_MAX =		  1,
};

// Windowsアプリケーションとの識別で使用するチャンネル値です
//static const u16 MCS_CHANNEL0 = 0;
//static const u16 MCS_CHANNEL1 = 1;

enum{
	MCS_CHANNEL0 = 0,
	MCS_CHANNEL1,
	MCS_CHANNEL2,
	MCS_CHANNEL3,
	MCS_CHANNEL4,
	MCS_CHANNEL5,
	MCS_CHANNEL6,
	MCS_CHANNEL7,
	MCS_CHANNEL8,
	MCS_CHANNEL9,
	MCS_CHANNEL10,
	MCS_CHANNEL11,
	MCS_CHANNEL12,
	MCS_CHANNEL13,
	MCS_CHANNEL14,
	MCS_CHANNEL15,

	MCS_CHANNEL_END,
};

#define	MCS_CH_MAX		( MCS_CHANNEL_END )
#define	MCS_SEND_SIZE	( 0x1800 )

#define	NP_COMP_MAX		( 3 )

//32バイトアライメントでヒープからの取り方がわからないので、とりあえず静的に
static	u8				strmBuffer[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);

int	bm_flag=0;

typedef struct {
	npCONTEXT	ctx;
	npSYSTEM	sys;
	void		*pvBuf;
	int			textureCount;

 	npPARTICLEEMITCYCLE *pEmit;
	npPARTICLECOMPOSITE *pComp[ NP_COMP_MAX ];
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
	int					phi;
	int					theta;
	int					distance;
	int					pos_x;
	int					pos_y;
}MCS_WORK;

typedef struct
{
	int					seq_no;
	MCSS_SYS_WORK		*mcss_sys;
	MCSS_WORK			*mcss[4];
	HEAPID				heapID;
	GFL_G3D_RES			*model_resource;
	GFL_G3D_RND			*model_render;
	GFL_G3D_OBJ			*model_obj;
	GFL_G3D_OBJSTATUS	model_status;
	GFL_G3D_RES			*stage_resource;
	GFL_G3D_RND			*stage_render;
	GFL_G3D_OBJ			*stage_obj;
	GFL_G3D_OBJSTATUS	stage_status[2];
	PARTICLE_WORK		*pw;
	CAMERA_WORK			cw;
	NNSSndStrm			strm;
	u8					FS_strmBuffer[STRM_BUF_SIZE];
	int					FSReadPos;
	u32					strmReadPos;
	u32					strmWritePos;
	int					visible_flag;
	int					timer_flag;
	int					timer;

	GFL_PTC_PTR			ptc;
	u8					spa_work[PARTICLE_LIB_HEAP_SIZE];

	//MCS
    u8*					mcsWorkMem;
    NNSMcsDeviceCaps	deviceCaps;
    NNSMcsRecvCBInfo	recvCBInfo;
	void*				printBuffer;
	void*				recvBuf;
	int					mcs_idle;
	int					mcs_enable;
	MCS_WORK			mw;
	GFL_BBD_SYS			*bbd_sys;
	int					bbd_res_idx;
	int					bbd_obj_idx;
}SOGA_WORK;

//カメラ
static	void	InitCamera( CAMERA_WORK *cw, HEAPID heapID );
static	void	MoveCamera( SOGA_WORK *sw );
static	void	CalcCamera( CAMERA_WORK *cw );

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
static void Bmt_update( npCONTEXT* ctx, npPARTICLECOMPOSITE** pComp, npTEXTURE** pTex );
static void Bmt_releaseTextures( npPARTICLECOMPOSITE** pComp, npCONTEXT* pCtx, npTEXTURE** pTex, int textureCount );
static void Bmt_releaseEffect( npCONTEXT *pCtx, npSYSTEM *pSys, void **ppvBuf);
static void bmt_loadBpc( const char *pszBpc, npPARTICLEEMITCYCLE **ppEmit, HEAPID heapID );
static void bmt_loadTex(const char *pszTex, const char *pszPlt, npTEXTURE *pTex, npU32* texSize, npU32* palSize, HEAPID heapID );
static void bmt_Mtx43ToMtx44(MtxFx44 *pDst, MtxFx43 *pTrc);
static void bmt_updateEffect( npPARTICLECOMPOSITE* pComp, npU32 renew, int comp_no );

//MCS
static BOOL MCS_Init( SOGA_WORK *sw );
static void MCS_Exit( SOGA_WORK *sw );
static void MCS_Read( SOGA_WORK *sw );
static void MCS_Write( SOGA_WORK *sw );
static void MCS_DataRecvCallback( const void* pRecv, u32 recvSize, u32 userData, u32 offset, u32 totalSize );
static void MCS_VBlankIntr( GFL_TCB *tcb, void *work );

//Capture
static void Capture_VBlankIntr( GFL_TCB *tcb, void *work );

typedef struct
{
	s32	x[2];
	s32	y[2];
	s32	percent;
	int	update;
}PERFORMANCE_PARAM;

extern	PERFORMANCE_PARAM	per_para;

static	void	EmitScaleSet(GFL_EMIT_PTR emit);

int	emit_angle;

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
		static const GFL_DISP_VRAM dispvramBank = {
			GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
			GX_VRAM_SUB_BG_32_H,			// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
			GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
			GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
			GX_VRAM_TEX_0_B,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_01_FG,			// テクスチャパレットスロット			
			GX_OBJVRAMMODE_CHAR_1D_64K,		// メインOBJマッピングモード
			GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
		};		
		GFL_DISP_SetBank( &dispvramBank );
	}	

	GX_SetBankForLCDC( GX_VRAM_LCDC_D );
	
	G2_BlendNone();
	GFL_BG_Init( wk->heapID );
	GFL_BMPWIN_Init( wk->heapID );

	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_3D,
//			GX_DISPMODE_VRAM_C, GX_BGMODE_4, GX_BGMODE_3, GX_BG0_AS_3D,
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
		G3X_EdgeMarking( TRUE );
		InitCamera( &wk->cw, wk->heapID );
	}

	wk->seq_no = 0;
	wk->mcss_sys = MCSS_Init( 4, wk->cw.camera, wk->heapID );

	wk->mcss[0] = MCSS_Add( wk->mcss_sys,
			  poke_pos[0].x, poke_pos[0].y, poke_pos[0].z,
			  ARCID_POKEGRA,
			  NARC_pokegra_wb_pfwb_001c_m_NCBR,
			  NARC_pokegra_wb_pmwb_001_n_NCLR,
			  NARC_pokegra_wb_pfwb_001_NCER,
			  NARC_pokegra_wb_pfwb_001_NANR,
			  NARC_pokegra_wb_pfwb_001_NMCR,
			  NARC_pokegra_wb_pfwb_001_NMAR,
			  NARC_pokegra_wb_pfwb_001_NCEC );
#if 0
			  NARC_pokegra_wb_pfwb_122c_m_NCBR,
			  NARC_pokegra_wb_pmwb_122_n_NCLR,
			  NARC_pokegra_wb_pfwb_122_NCER,
			  NARC_pokegra_wb_pfwb_122_NANR,
			  NARC_pokegra_wb_pfwb_122_NMCR,
			  NARC_pokegra_wb_pfwb_122_NMAR,
			  NARC_pokegra_wb_pfwb_122_NCEC );
#endif
	MCSS_SetScaleX( wk->mcss[0], FX_F32_TO_FX32( 1.2f ) );
	MCSS_SetScaleY( wk->mcss[0], FX_F32_TO_FX32( 1.2f ) );
	wk->mcss[1] = MCSS_Add( wk->mcss_sys,
			  poke_pos[1].x, poke_pos[1].y, poke_pos[1].z,
			  ARCID_POKEGRA,
			  NARC_pokegra_wb_pfwb_053c_m_NCBR,
			  NARC_pokegra_wb_pmwb_053_n_NCLR,
			  NARC_pokegra_wb_pfwb_053_NCER,
			  NARC_pokegra_wb_pfwb_053_NANR,
			  NARC_pokegra_wb_pfwb_053_NMCR,
			  NARC_pokegra_wb_pfwb_053_NMAR,
			  NARC_pokegra_wb_pfwb_053_NCEC );
#if 0
			  NARC_pokegra_wb_pfwb_006c_m_NCBR,
			  NARC_pokegra_wb_pmwb_006_n_NCLR,
			  NARC_pokegra_wb_pfwb_006_NCER,
			  NARC_pokegra_wb_pfwb_006_NANR,
			  NARC_pokegra_wb_pfwb_006_NMCR,
			  NARC_pokegra_wb_pfwb_006_NMAR,
			  NARC_pokegra_wb_pfwb_006_NCEC );
#endif
	MCSS_SetScaleX( wk->mcss[1], FX_F32_TO_FX32( 1.1f ) );
	MCSS_SetScaleY( wk->mcss[1], FX_F32_TO_FX32( 1.1f ) );
	wk->mcss[2] = MCSS_Add( wk->mcss_sys,
			  poke_pos[2].x, poke_pos[2].y, poke_pos[2].z,
			  ARCID_POKEGRA,
			  NARC_pokegra_wb_pbwb_006c_m_NCBR,
			  NARC_pokegra_wb_pmwb_006_n_NCLR,
			  NARC_pokegra_wb_pbwb_006_NCER,
			  NARC_pokegra_wb_pbwb_006_NANR,
			  NARC_pokegra_wb_pbwb_006_NMCR,
			  NARC_pokegra_wb_pbwb_006_NMAR,
			  NARC_pokegra_wb_pbwb_006_NCEC );
	MCSS_SetScaleX( wk->mcss[2], FX_F32_TO_FX32( 0.8f ) );
	MCSS_SetScaleY( wk->mcss[2], FX_F32_TO_FX32( 0.8f ) );
	wk->mcss[3] = MCSS_Add( wk->mcss_sys,
			  poke_pos[3].x, poke_pos[3].y, poke_pos[3].z,
			  ARCID_POKEGRA,
			  NARC_pokegra_wb_pbwb_006c_m_NCBR,
			  NARC_pokegra_wb_pmwb_006_n_NCLR,
			  NARC_pokegra_wb_pbwb_006_NCER,
			  NARC_pokegra_wb_pbwb_006_NANR,
			  NARC_pokegra_wb_pbwb_006_NMCR,
			  NARC_pokegra_wb_pbwb_006_NMAR,
			  NARC_pokegra_wb_pbwb_006_NCEC );
	MCSS_SetScaleX( wk->mcss[3], FX_F32_TO_FX32( 0.8f ) );
	MCSS_SetScaleY( wk->mcss[3], FX_F32_TO_FX32( 0.8f ) );

	//3D Model Load
	{
		BOOL	ret;

		//リソース読み込み
		wk->model_resource = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, NARC_battgra_wb_batt_field01_nsbmd );
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

		//リソース読み込み
		wk->stage_resource = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, NARC_battgra_wb_batt_stage01_nsbmd );
		ret = GFL_G3D_TransVramTexture( wk->stage_resource );
		GF_ASSERT( ret == TRUE );
		//RENDER生成
		wk->stage_render = GFL_G3D_RENDER_Create( wk->stage_resource, 0, wk->stage_resource );
		//OBJ生成
		wk->stage_obj = GFL_G3D_OBJECT_Create( wk->stage_render, NULL, 0 );

		wk->stage_status[0].trans.x = FX_F32_TO_FX32( 4.5f );
		wk->stage_status[0].trans.y = 0;
		wk->stage_status[0].trans.z = FX_F32_TO_FX32( -10.0f );
		MTX_Identity33(&wk->stage_status[0].rotate);
		wk->stage_status[0].scale.x = FX_F32_TO_FX32( 0.5f );
		wk->stage_status[0].scale.y = FX_F32_TO_FX32( 0.5f );
		wk->stage_status[0].scale.z = FX_F32_TO_FX32( 0.5f );

		wk->stage_status[1].trans.x = FX_F32_TO_FX32( -2.5f );
		wk->stage_status[1].trans.y = 0;
		wk->stage_status[1].trans.z = FX_F32_TO_FX32( 4.6f );
		MTX_Identity33(&wk->stage_status[1].rotate);
		wk->stage_status[1].scale.x = FX_F32_TO_FX32( 0.5f );
		wk->stage_status[1].scale.y = FX_F32_TO_FX32( 0.5f );
		wk->stage_status[1].scale.z = FX_F32_TO_FX32( 0.5f );
	}

	//ビルボード初期化
	{
		static const GFL_BBD_SETUP setup = {
			64, 128, 
			{ FX32_ONE * 32, FX32_ONE * 32, FX32_ONE * 32 },
			GX_RGB(31, 31, 31),
			GX_RGB(16, 16, 16),
			GX_RGB(16, 16, 16), 
			GX_RGB(0, 0, 0),
			63,
		};
		VecFx32 pos = { 0, 0, FX_F32_TO_FX32( -47.0f ) };
		static	const BOOL	flag = TRUE;

		wk->bbd_sys = GFL_BBD_CreateSys( &setup, wk->heapID );
		wk->bbd_res_idx = GFL_BBD_AddResourceArc( wk->bbd_sys, ARCID_BATTGRA, NARC_battgra_wb_batt_bg01_nsbtx, GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x32, 32, 32 );
		wk->bbd_obj_idx = GFL_BBD_AddObject( wk->bbd_sys, wk->bbd_res_idx, FX16_ONE * 2 , FX16_ONE , &pos, 31, GFL_BBD_LIGHT_NONE );
		GFL_BBD_SetObjectDrawEnable( wk->bbd_sys, wk->bbd_obj_idx, &flag );
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

		GFL_ARC_UTIL_TransVramBgCharacter(ARCID_BATTGRA,NARC_battgra_wb_batt_bg1_NCGR,GFL_BG_FRAME1_M,0,0,0,wk->heapID);
		GFL_ARC_UTIL_TransVramScreen(ARCID_BATTGRA,NARC_battgra_wb_batt_bg1_NSCR,GFL_BG_FRAME1_M,0,0,0,wk->heapID);
		GFL_ARC_UTIL_TransVramBgCharacter(ARCID_BATTGRA,NARC_battgra_wb_batt_bg2_NCGR,GFL_BG_FRAME2_M,0,0,0,wk->heapID);
		GFL_ARC_UTIL_TransVramScreen(ARCID_BATTGRA,NARC_battgra_wb_batt_bg2_NSCR,GFL_BG_FRAME2_M,0,0,0,wk->heapID);
		GFL_ARC_UTIL_TransVramPalette(ARCID_BATTGRA,NARC_battgra_wb_batt_bg_NCLR,PALTYPE_MAIN_BG,0,0x100,wk->heapID);
	}

	//ストリーム再生初期化
    NNS_SndInit();
    NNS_SndStrmInit( &wk->strm );
	NNS_SndStrmAllocChannel( &wk->strm, CH_MAX, chNoList);

	StrmSetUp(wk);

	NNS_SndStrmStart(&wk->strm);
	
	//パーティクル初期化
#ifdef NINTEN_SPL
	GFL_PTC_Init( wk->heapID );
	wk->ptc = GFL_PTC_Create( wk->spa_work, PARTICLE_LIB_HEAP_SIZE, FALSE, wk->heapID );
	GFL_PTC_SetResource( wk->ptc, GFL_PTC_LoadArcResource( ARCID_PTC, NARC_spa_w_467_spa, wk->heapID ), TRUE, NULL );
#else
	wk->pw = GFL_HEAP_AllocClearMemory( wk->heapID, sizeof(PARTICLE_WORK) );
	wk->pw->heapID = wk->heapID;
	ParticleInit( wk->pw );
#endif

	//キャプチャセット
	GFUser_VIntr_CreateTCB( Capture_VBlankIntr, NULL, 0 );

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
	int trg = GFL_UI_KEY_GetTrg();
	SOGA_WORK* wk = mywk;

	if( wk->mcs_enable ){
		NNS_McsPollingIdle();
		MCS_Read( wk );
//		MCS_Write( wk );
	}

	//画面切り替え実験
	if( wk->timer_flag ){
		wk->timer++;
	}
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

#ifdef MCS_ENABLE
	if( ( trg & PAD_BUTTON_START ) ||
		wk->mcs_idle ){
		if( wk->mcs_enable ){
			MCS_Exit( wk );
		}
		else{
			wk->mcs_idle = MCS_Init( wk );
		}
	}
#else
	if( trg & PAD_BUTTON_START ){
		InitCamera( &wk->cw, wk->heapID );
	}
#endif

	if( trg & PAD_BUTTON_SELECT ){
		wk->timer_flag ^= 1;
	}

//	if( trg & PAD_BUTTON_START ){
//		bm_flag++;
//	}

	if( pad & PAD_BUTTON_A ){
		wk->mcss[0]->mepachi_flag = 1;
		wk->mcss[1]->mepachi_flag = 1;
		wk->mcss[2]->mepachi_flag = 1;
		wk->mcss[3]->mepachi_flag = 1;
	}
	else{
		wk->mcss[0]->mepachi_flag = 0;
		wk->mcss[1]->mepachi_flag = 0;
		wk->mcss[2]->mepachi_flag = 0;
		wk->mcss[3]->mepachi_flag = 0;
	}

	MoveCamera( wk );

	MCSS_Main( wk->mcss_sys );

#ifdef NINTEN_SPL
	if( GFL_PTC_GetEmitterNum( wk->ptc ) == 0 ){
		GFL_PTC_CreateEmitter( wk->ptc, 0, &poke_pos[0] );
		GFL_PTC_CreateEmitter( wk->ptc, 0, &poke_pos[1] );
		GFL_PTC_CreateEmitter( wk->ptc, 0, &poke_pos[2] );
		GFL_PTC_CreateEmitter( wk->ptc, 0, &poke_pos[3] );
//		GFL_PTC_CreateEmitterCallback( wk->ptc, 0, EmitScaleSet, NULL );
//		GFL_PTC_CreateEmitter( wk->ptc, 1, &poke_pos[1] );
//		GFL_PTC_CreateEmitterCallback( wk->ptc, 0, EmitScaleSet, &poke_pos[1] );
//		GFL_PTC_CreateEmitterCallback( wk->ptc, 1, EmitScaleSet, &poke_pos[1] );
	}
#endif

	//3D描画
	{
		GFL_BBD_Draw( wk->bbd_sys, wk->cw.camera, NULL );
		GFL_G3D_DRAW_Start();
		GFL_G3D_DRAW_SetLookAt();
		{
			GFL_G3D_DRAW_DrawObject( wk->model_obj, &wk->model_status );
//			GFL_G3D_DRAW_DrawObject( wk->model_obj, &wk->stage_status[0] );
//			GFL_G3D_DRAW_DrawObject( wk->model_obj, &wk->stage_status[1] );
			GFL_G3D_DRAW_DrawObject( wk->stage_obj, &wk->stage_status[0] );
			GFL_G3D_DRAW_DrawObject( wk->stage_obj, &wk->stage_status[1] );
			MCSS_Draw( wk->mcss_sys );
#ifdef NINTEN_SPL
			{
				s32	particle_count;

				particle_count = G3X_GetPolygonListRamCount();

				GFL_PTC_Main();

				particle_count = G3X_GetPolygonListRamCount() - particle_count;

				OS_TPrintf("poly:%d\n",particle_count);
			}
#else
			ParticleDraw(wk->pw);
#endif
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
//	VecFx32	cam_pos = { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 16.5f ), FX_F32_TO_FX32( 38.0f ) };
	VecFx32	cam_pos = { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 7.0f ), FX_F32_TO_FX32( 18.6f ) };

	cw->target.x = FX_F32_TO_FX32( 0.0f );
	cw->target.y = FX_F32_TO_FX32( 2.6f );
	cw->target.z = FX_F32_TO_FX32( 0.0f );
	cw->camUp.x	= 0;
	cw->camUp.y	= FX32_ONE;
	cw->camUp.z	= 0;

	cw->phi		= FX_Atan2Idx( cam_pos.y, cam_pos.z );
	cw->theta	= FX_Atan2Idx( cam_pos.y, cam_pos.x );
	cw->scale	= VEC_Mag( &cam_pos );

	CalcCamera( cw );

	if( cw->camera == NULL ){
		cw->camera = GFL_G3D_CAMERA_Create( GFL_G3D_PRJPERS, 
											FX32_SIN13,
											FX32_COS13,
//											FX32_ONE * 4 / 3,
											FX_F32_TO_FX32( 1.33f ),
											NULL,
											FX32_ONE,
											FX32_ONE * 1000,
											NULL,
											&cw->camPos,
											&cw->camUp,
											&cw->target,
											heapID );
	}
}

//======================================================================
//	カメラの制御
//======================================================================
static	void	MoveCamera( SOGA_WORK *wk )
{
	CAMERA_WORK	*cw = &wk->cw;
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
			xofs = -MOVE_SPEED;
		}
		if( pad & PAD_KEY_RIGHT ){
			xofs = MOVE_SPEED;
		}
		if( pad & PAD_KEY_UP ){
			yofs = MOVE_SPEED;
		}
		if( pad & PAD_KEY_DOWN ){
			yofs = -MOVE_SPEED;
		}
		if( pad & PAD_BUTTON_X ){
			zofs = MOVE_SPEED;
		}
		if( pad & PAD_BUTTON_B ){
			zofs = -MOVE_SPEED;
		}
		if( wk->mw.pos_x ){
			xofs = MOVE_SPEED * wk->mw.pos_x;
			wk->mw.pos_x = 0;
		}
		if( wk->mw.pos_y ){
			yofs = MOVE_SPEED * wk->mw.pos_y;
			wk->mw.pos_y = 0;
		}
		if( wk->mw.distance ){
			zofs = MOVE_SPEED * (wk->mw.distance / 50);
			wk->mw.distance = 0;
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
	if( wk->mw.theta ){
		cw->theta -= wk->mw.theta * 32 ;
		wk->mw.theta = 0;
	}

	if( wk->mw.phi ){
		cw->phi -= wk->mw.phi * 32 ;
		if( cw->phi <= -PHI_MAX ){
			cw->phi = -PHI_MAX;
		}
		if( cw->phi >= PHI_MAX ){
			cw->phi = PHI_MAX;
		}
		wk->mw.phi = 0;
	}

	CalcCamera( cw );
	GFL_G3D_CAMERA_SetPos( cw->camera, &cw->camPos );
	GFL_G3D_CAMERA_SetTarget( cw->camera, &cw->target );
	GFL_G3D_CAMERA_Switching( cw->camera );
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
//	MTX_TransApply43( &trans, &trans, cw->target.x,	cw->target.y, cw->target.z );
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
	pw->textureCount = Bmt_loadTextures( &pw->ctx, &pw->pTex[0], &pw->pEmit, &pw->pComp[0], pw->heapID );
	pw->textureCount = Bmt_loadTextures( &pw->ctx, &pw->pTex[0], &pw->pEmit, &pw->pComp[1], pw->heapID );
	pw->textureCount = Bmt_loadTextures( &pw->ctx, &pw->pTex[0], &pw->pEmit, &pw->pComp[2], pw->heapID );
	OS_TPrintf("BM load texture OK.\n");
	npSetPolygonIDMin( &pw->ctx, 30 );
	npSetPolygonIDMax( &pw->ctx, 50 );
}

static void ParticleDraw( PARTICLE_WORK *pw )
{
	Bmt_update( &pw->ctx, &pw->pComp[0], &pw->pTex[0] );
}

static void ParticleExit( PARTICLE_WORK *pw )
{
	Bmt_releaseTextures( &pw->pComp[0], &pw->ctx, &pw->pTex[0], pw->textureCount );
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

	*ppvBuf = GFL_HEAP_AllocMemory( heapID, nSize + 32 );

#if 0
	//32バイトアライメントをしてみる
	{
		u32 addr;

		OS_TPrintf("ppvBuf:0x%08x\n",*ppvBuf);
		addr = (u32)*ppvBuf;
		addr += ( 32 - ( addr % 32 ) );
		*ppvBuf = (void*)addr;
		OS_TPrintf("ppvBuf:0x%08x\n",*ppvBuf);
	}
#endif

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
		{ "/BmSample/kemuri2.ntft"		, "/BmSample/kemuri2.ntfp"		, 64, 64,	GX_TEXFMT_A5I3} ,
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
static void Bmt_releaseTextures( npPARTICLECOMPOSITE** pComp, npCONTEXT* pCtx, npTEXTURE** pTex, int textureCount )
{
	int i;

	for( i = 0 ; i < NP_COMP_MAX ; i++ ){
		if( pComp[i] != NULL ){
			npParticleReleaseComposite( pComp[i] );
		}
	}

	for(i=0; i<textureCount; i++)
	{
		npReleaseTexture( pCtx, pTex[i] );
	}
}
//--------------------------------------------------
// 描画更新
//--------------------------------------------------
static void Bmt_update( npCONTEXT* ctx, npPARTICLECOMPOSITE** pComp, npTEXTURE** pTex )
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

    G3_MtxMode(GX_MTXMODE_POSITION);

	// UPDATE & RENDER
	{
		int	i;

		for( i = 0 ; i < NP_COMP_MAX ; i++ ){
			bmt_updateEffect( pComp[i], NP_NTSC60, i );
			npParticleRenderComposite( ctx, pComp[i], pTex, DEF_ENTRYTEX_MAX );
		}
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


static void bmt_updateEffect( npPARTICLECOMPOSITE* pComp, npU32 renew, int comp_no )
{
	MtxFx44	mtx,trans;
	// scalingEffect
	npFMATRIX effMat;
	npFVECTOR scale;
	VecFx32	pos[]={
		{ 0x00028000, 0x00001000, 0xffff0000 },
		{ 0x00060000, 0x00001000, 0x00010000 },
		{ 0xfffd0000, 0x00001000, 0x00020000 },
	};

#if 1
//	MTX_Scale44( &mtx, FX32_ONE * 12, FX32_ONE * 4, FX32_ONE * 12 );
//	MTX_Identity44( &trans );
//	MTX_TransApply44( &trans, &trans, pos[ comp_no ].x, pos[ comp_no ].y, pos[ comp_no ].z );
//	MTX_Concat44( &mtx, &trans, &mtx );
	MTX_Identity44( &trans );
	MTX_TransApply44( &trans, &mtx, poke_pos[ comp_no ].x, poke_pos[ comp_no ].y, poke_pos[ comp_no ].z );
#else
	MTX_Identity44( &mtx );
	MTX_TransApply44( &mtx, &mtx, 0x00060000,0x00001000,0x00010000 );
	scale[0] = FX32_ONE * 1;
	scale[1] = FX32_ONE * 1;
	scale[2] = FX32_ONE * 1;
	scale[3] = FX32_ONE * 1;
	(void)npParticleSetScaling( pComp, &scale );
#endif
	(void)npParticleSetGlobalFMATRIX((npPARTICLEOBJECT *)pComp, (npFMATRIX *)&mtx);
	(void)npParticleUpdateComposite( pComp, renew );
}


//
//	MCS制御実験
//
static BOOL MCS_Init( SOGA_WORK *wk )
{
	if(	wk->mcs_enable ) return FALSE;

	wk->mw.phi = 0;
	wk->mw.theta = 0;

	// mcsの初期化
	wk->mcsWorkMem = GFL_HEAP_AllocMemory( wk->heapID, NNS_MCS_WORKMEM_SIZE ); // MCSのワーク用メモリを確保
	NNS_McsInit( wk->mcsWorkMem );

	GFUser_VIntr_CreateTCB( MCS_VBlankIntr, NULL, 0 );

	// デバイスのオープン
	if( NNS_McsGetMaxCaps() > 0 && NNS_McsOpen( &wk->deviceCaps ) )
	{
		wk->printBuffer = GFL_HEAP_AllocMemory( wk->heapID, 1024 );        // プリント用のバッファの確保
		wk->recvBuf = GFL_HEAP_AllocMemory( wk->heapID, 1024 );       // 受信用バッファの確保

        NNS_NULL_ASSERT(wk->printBuffer);
        NNS_NULL_ASSERT(wk->recvBuf);

        // OS_Printfによる出力
        OS_Printf("device open\n");

        // mcs文字列出力の初期化
		NNS_McsInitPrint( wk->printBuffer, 1024 );

        // NNS_McsPrintfによる出力
        // このタイミングでmcsサーバが接続していれば、コンソールに表示されます。
		(void)NNS_McsPrintf("device ID %08X\n", wk->deviceCaps.deviceID );

        // 読み取り用バッファの登録
		NNS_McsRegisterStreamRecvBuffer(MCS_CHANNEL0, wk->recvBuf, 1024 );

        // 受信コールバック関数の登録
//		NNS_McsRegisterRecvCallback( &wk->recvCBInfo, MCS_CHANNEL1, MCS_DataRecvCallback, (u32)&wk->mw );

		wk->mcs_enable = 1;

        return FALSE;
    }
	OS_Printf("device open fail.\n");
	return TRUE;
}

static void MCS_Exit( SOGA_WORK *wk )
{
//	NNS_McsUnregisterRecvResource(MCS_CHANNEL1);
	NNS_McsUnregisterRecvResource(MCS_CHANNEL0);

	GFL_HEAP_FreeMemory( wk->recvBuf );
	GFL_HEAP_FreeMemory( wk->printBuffer );

	// NNS_McsPutStringによる出力
	(void)NNS_McsPutString("device close\n");

	// デバイスをクローズ
	(void)NNS_McsClose();

	wk->mcs_enable = 0;
	wk->mcs_idle = 0;
}

static void MCS_Read( SOGA_WORK *wk )
{
	int			len;
	MCS_WORK	buf;

	// 受信可能なデータサイズの取得
	len = NNS_McsGetStreamReadableSize( MCS_CHANNEL0 );

	if( len > 0 ){
		u32 readSize;
		// データの読み取り
		if( NNS_McsReadStream( MCS_CHANNEL0, &buf, sizeof(MCS_WORK), &readSize)){
			wk->mw = buf;
        }
	}
}

static void MCS_Write( SOGA_WORK *wk )
{
#if 0 
	if( per_para.update ){
		per_para.update = 0;
		// データの書き込み
		NNS_McsWriteStream( MCS_CHANNEL0, &per_para, sizeof(PERFORMANCE_PARAM) - 4 );
	}
#else
//	NNS_McsWriteStream( MCS_CHANNEL0, (const void *)HW_LCDC_VRAM_D, 256*192*2 );
	int ch;

	for( ch = 0 ; ch < MCS_CH_MAX ; ch++ ){
		NNS_McsWriteStream( MCS_CHANNEL0 + ch, (const void *)(HW_LCDC_VRAM_D + MCS_SEND_SIZE * ch), MCS_SEND_SIZE );
	}
#endif
}

/*---------------------------------------------------------------------------*
  Name:         DataRecvCallback

  Description:  PC側からデータを受信したときに呼ばれるコールバック関数です。

                登録するコールバック関数内ではデータの送受信を行わないでください。
                また、割り込みが禁止されている場合があるため、
                割り込み待ちループも行わないでください。

  Arguments:    recv:       受信したデータの一部あるいは全部を格納している
                            バッファへのポインタ。
                recvSize:   recvによって示されるバッファに格納されている
                            データのサイズ。
                userData:   NNS_McsRegisterRecvCallback()の引数userDataで
                            指定した値。
                offset:     受信したデータの全部がrecvによって示されるバッファに
                            格納されている場合は0。
                            受信したデータの一部が格納されている場合は、
                            受信したデータ全体に対する0を基準としたオフセット位置。
                totalSize:  受信したデータの全体のサイズ。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void
MCS_DataRecvCallback(
    const void* pRecv,
    u32         recvSize,
    u32         userData,
    u32         offset,
    u32         totalSize
)
{
    MCS_WORK *mw = (MCS_WORK *)userData;

	OS_Printf( " Callback OK!\n");

	// 受信バッファチェック
	if (pRecv != NULL && recvSize == sizeof(MCS_WORK) && offset == 0)
	{
        mw = (MCS_WORK *)pRecv;
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Vブランク割り込みハンドラです。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void MCS_VBlankIntr( GFL_TCB *tcb, void *work )
{
    NNS_McsVBlankInterrupt();
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Vブランク割り込みハンドラです。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void Capture_VBlankIntr( GFL_TCB *tcb, void *work )
{
	//---------------------------------------------------------------------------
	// Execute capture
	// Blend rendered image and displayed image (VRAM), and output to VRAM-C
	GX_SetCapture(GX_CAPTURE_SIZE_256x192,			// Capture size
				  GX_CAPTURE_MODE_A,				// Capture mode
				  GX_CAPTURE_SRCA_2D3D,				// Blend src A
				  GX_CAPTURE_SRCB_VRAM_0x00000,		// Blend src B
				  GX_CAPTURE_DEST_VRAM_D_0x00000,	// Output VRAM
				  16,								// Blend parameter for src A
				  16);								// Blend parameter for src B
	//---------------------------------------------------------------------------
}

static	void	EmitScaleSet(GFL_EMIT_PTR emit)
{
	VecFx16	vec;
	VecFx32	pos;

	pos.x = poke_pos[2].x;
	pos.y = poke_pos[2].y + FX32_ONE * 4;
	pos.z = poke_pos[2].z - FX32_HALF;

	GFL_PTC_SetEmitterPosition( emit, &pos );

	vec.x = poke_pos[ emit_angle ].x - poke_pos[2].x;
	vec.y = poke_pos[ emit_angle ].y - poke_pos[2].y;
	vec.z = poke_pos[ emit_angle ].z - poke_pos[2].z;

	VEC_Fx16Normalize( &vec, &vec );

	emit_angle ^= 1;

	GFL_PTC_SetEmitterAxis( emit, &vec );
}

