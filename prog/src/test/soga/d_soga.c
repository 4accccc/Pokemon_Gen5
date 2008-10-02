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

#include "system/main.h"
#include "arc_def.h"

#include "mcss.h"
#include "pokegra_wb.naix"

#define	CAMERA_SPEED		( FX32_ONE * 2 )

#define	INTERVAL		(16)
#define	CH_MAX			(1)
#define	STRM_BUF_SIZE	(INTERVAL*2*CH_MAX*32)
#define	SWAV_HEAD_SIZE	(18)

//32バイトアライメントでヒープからの取り方がわからないので、とりあえず静的に
static	u8				strmBuffer[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);

typedef struct
{
	int				seq_no;
	MCSS_SYS_WORK	*mcss_sys;
	int				heapID;
	VecFx32			camPos;					// カメラの位置(＝視点)
	VecFx32			target;					// カメラの焦点(＝注視点)
	VecFx32			camUp;			
	NNSSndStrm		strm;
	u8				FS_strmBuffer[STRM_BUF_SIZE];
	int				FSReadPos;
	u32				strmReadPos;
	u32				strmWritePos;
}SOGA_WORK;

static	void	SetCamera( SOGA_WORK *wk );

NNSG3dRenderObj	object;
void			*memory;

static	void	StrmSetUp(SOGA_WORK *sw);
static	void	StrmCBWaveDataStock(NNSSndStrmCallbackStatus status,
									int numChannels,
									void *buffer[],
									u32	len,
									NNSSndStrmFormat format,
									void *arg);
static	void	StrmBufferCopy(SOGA_WORK *sw,int size);

//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugSogabeMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	SOGA_WORK* wk;
	u8 chNoList[1]={0};

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SOGABE_DEBUG, 0x10000 );
	wk = GFL_PROC_AllocWork( proc, sizeof( SOGA_WORK ), HEAPID_SOGABE_DEBUG );
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

	wk->camPos.x = 0;
	wk->camPos.y = 0;
	wk->camPos.z = -10 * FX32_ONE;					// カメラの位置(＝視点)
	wk->target.x = 0;
	wk->target.y = 0;
	wk->target.z = 0;
	wk->camUp.x = 0;
	wk->camUp.y = FX32_ONE;
	wk->camUp.z = 0;

	G3X_Reset();
	SetCamera(wk);

	wk->seq_no = 0;
	wk->mcss_sys = MCSS_Init( 2, &wk->camPos, &wk->target, &wk->camUp, wk->heapID );

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
			  NARC_pokegra_wb_multi_test_NMAR );

	MCSS_Add( wk->mcss_sys,
//			  0xfffd0000,0xfffd8000,0xfff6a000,
//			  0xfffd0000,0x00048000,0xfffc0000,
			  0xfffd0000,0x0003a000,0x00060000,
			  ARCID_POKEGRA,
			  NARC_pokegra_wb_multi_test2_NCBR,
			  NARC_pokegra_wb_multi_test2_NCLR,
			  NARC_pokegra_wb_multi_test2_NCER,
			  NARC_pokegra_wb_multi_test2_NANR,
			  NARC_pokegra_wb_multi_test2_NMCR,
			  NARC_pokegra_wb_multi_test2_NMAR );

	//3D Model Load
	{
		NNSG3dResMdl	*model;
		memory = GFL_ARC_LoadDataAlloc( ARCID_POKEGRA, NARC_pokegra_wb_test_battle_nsbmd, wk->heapID );
//		memory = GFL_ARC_LoadDataAlloc( ARCID_POKEGRA, NARC_pokegra_wb_batt_testmap01c_nsbmd, wk->heapID );
//		memory = GFL_ARC_LoadDataAlloc( ARCID_POKEGRA, NARC_pokegra_wb_batt_teststage_nsbmd, wk->heapID );
		DC_StoreRange( memory, ((NNSG3dResFileHeader*)memory)->fileSize );
		NNS_G3dResDefaultSetup( memory );
		model=NNS_G3dGetMdlByIdx( NNS_G3dGetMdlSet(memory), 0 );
		NNS_G3dRenderObjInit( &object, model );
	}

	//ストリーム再生初期化
    NNS_SndInit();
    NNS_SndStrmInit( &wk->strm );
	NNS_SndStrmAllocChannel( &wk->strm, CH_MAX, chNoList);

	StrmSetUp(wk);

	NNS_SndStrmStart(&wk->strm);

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugSogabeMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	SOGA_WORK* wk = mywk;
	int pad = GFL_UI_KEY_GetCont();

	StrmBufferCopy(wk,0);

	if( pad & PAD_KEY_LEFT ){
		wk->camPos.x -= CAMERA_SPEED;
		wk->target.x -= CAMERA_SPEED;
	}
	if( pad & PAD_KEY_RIGHT ){
		wk->camPos.x += CAMERA_SPEED;
		wk->target.x += CAMERA_SPEED;
	}
	if( pad & PAD_KEY_UP ){
		wk->camPos.y += CAMERA_SPEED;
		wk->target.y += CAMERA_SPEED;
	}
	if( pad & PAD_KEY_DOWN ){
		wk->camPos.y -= CAMERA_SPEED;
		wk->target.y -= CAMERA_SPEED;
	}
	if( pad & PAD_BUTTON_L ){
		wk->camPos.z += CAMERA_SPEED;
		wk->target.z += CAMERA_SPEED;
	}
	if( pad & PAD_BUTTON_R ){
		wk->camPos.z -= CAMERA_SPEED;
		wk->target.z -= CAMERA_SPEED;
	}
	if( pad & PAD_BUTTON_X ){
		wk->camPos.x = 0;
		wk->camPos.y = 0;
		wk->camPos.z = -10 * FX32_ONE;					// カメラの位置(＝視点)
		wk->target.x = 0;
		wk->target.y = 0;
		wk->target.z = 0;
		wk->camUp.x = 0;
		wk->camUp.y = FX32_ONE;
		wk->camUp.z = 0;
	}

	MCSS_Main( wk->mcss_sys );
	MCSS_Draw( wk->mcss_sys );

	if( (pad & PAD_BUTTON_Y) == 0 ){
		NNS_G3dDraw( &object );
	}

	SetCamera( wk );

	G3_SwapBuffers( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );

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

 
static	void	SetCamera( SOGA_WORK *wk )
{
	MtxFx33	Rotate;

    //射影行列モードに変更
    G3_MtxMode(GX_MTXMODE_PROJECTION);

	G3_Perspective( FX32_SIN30, FX32_COS30, FX32_ONE * 4 / 3, FX32_ONE, FX32_ONE * 400, NULL);

//	MTX_RotX33( &Rotate, FX_SinIdx(65536/12), FX_CosIdx(65536/12));
    G3_Translate( wk->target.x, wk->target.y, wk->target.z );
//	G3_MultMtx33(&Rotate);

    G3_StoreMtx(0);

    //行列モードを元に戻します
    G3_MtxMode(GX_MTXMODE_POSITION);

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
					  NNS_SND_STRM_TIMER_CLOCK/16000,
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
	FS_OpenFile(&file,"/mh.swav");
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
