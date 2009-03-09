//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ステージ 演技メイン
 * @author	ariizumi
 * @data	09/03/02
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "font/font.naix"

#include "arc_def.h"
#include "musical_item.naix"
#include "stage_gra.naix"
#include "print/printsys.h"

#include "sound/snd_strm.h"
#include "snd_strm.naix"

#include "infowin/infowin.h"
#include "test/ariizumi/ari_debug.h"
#include "musical/mus_poke_draw.h"
#include "musical/mus_item_draw.h"
#include "musical/musical_camera_def.h"

#include "musical/musical_stage_sys.h"
#include "sta_local_def.h"
#include "sta_acting.h"
#include "sta_act_effect.h"
#include "sta_act_poke.h"
#include "sta_act_obj.h"

#include "eff_def/mus_eff.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define ACT_FRAME_MAIN_3D		GFL_BG_FRAME0_M
#define ACT_FRAME_MAIN_FONT		GFL_BG_FRAME1_M
#define ACT_FRAME_MAIN_CURTAIN	GFL_BG_FRAME2_M
#define ACT_FRAME_MAIN_BG		GFL_BG_FRAME3_M

#define ACT_FRAME_SUB_BG		GFL_BG_FRAME1_S
#define ACT_FRAME_SUB_INFO		GFL_BG_FRAME3_S

#define ACT_PAL_INFO		(0xE)
#define ACT_PAL_FONT		(0xF)


#define ACT_OBJECT_IVALID (-1)

#define ACT_BG_SCROLL_MAX (256)
#define ACT_MAKU_SCROLL_MAX (256)

//Msg系
#define ACT_MSG_POS_X ( 4 )
#define ACT_MSG_POS_Y ( 20 )
#define ACT_MSG_POS_WIDTH  ( 24 )
#define ACT_MSG_POS_HEIGHT ( 4 )

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct


struct _ACTING_WORK
{
	HEAPID heapId;
	
	u16		scrollOffset;
	u16		makuOffset;
	
	ACTING_INIT_WORK *initWork;

	MUS_POKE_DRAW_SYSTEM	*drawSys;
	MUS_ITEM_DRAW_SYSTEM	*itemDrawSys;
	
	STA_POKE_SYS		*pokeSys;
	STA_POKE_WORK		*pokeWork[MUSICAL_POKE_MAX];
	
	STA_OBJ_SYS			*objSys;
	STA_OBJ_WORK		*objWork[ACT_OBJECT_MAX];

	STA_EFF_SYS			*effSys;
	STA_EFF_WORK		*effWork[ACT_EFFECT_MAX];

	GFL_G3D_CAMERA		*camera;
	GFL_BBD_SYS			*bbdSys;
	
	GFL_TCBLSYS			*tcblSys;
	GFL_BMPWIN			*msgWin;
	GFL_FONT			*fontHandle;
	PRINT_STREAM		*printHandle;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
ACTING_WORK*	STA_ACT_InitActing( ACTING_INIT_WORK *initWork );
void	STA_ACT_InitActing_2nd( ACTING_WORK *work );
void	STA_ACT_TermActing( ACTING_WORK *work );
ACTING_RETURN	STA_ACT_LoopActing( ACTING_WORK *work );

static void STA_ACT_SetupGraphic( ACTING_WORK *work );
static void STA_ACT_SetupBg( ACTING_WORK *work );
static void	STA_ACT_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane  , u8 mode );
static void STA_ACT_SetupPokemon( ACTING_WORK *work );
static void STA_ACT_SetupItem( ACTING_WORK *work );
static void STA_ACT_SetupEffect( ACTING_WORK *work );
static void STA_ACT_SetupMessage( ACTING_WORK *work );

static void STA_ACT_UpdateItem( ACTING_WORK *work );
static void STA_ACT_UpdateScroll( ACTING_WORK *work );
static void STA_ACT_UpdateMessage( ACTING_WORK *work );



static const GFL_DISP_VRAM vramBank = {
	GX_VRAM_BG_128_D,				// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_23_G,			// メイン2DエンジンのBG拡張パレット
//	GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_16_F,				// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_NONE,			// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_01_AB,				// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_0123_E,			// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_32K,
	GX_OBJVRAMMODE_CHAR_1D_32K
};
//	A テクスチャ
//	B テクスチャ
//	C SubBg
//	D MainBg
//	E テクスチャパレット
//	F MainObj
//	G MainBgExPlt
//	H None
//	I None

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
ACTING_WORK*	STA_ACT_InitActing( ACTING_INIT_WORK *initWork )
{
	ACTING_WORK *work = GFL_HEAP_AllocMemory( initWork->heapId , sizeof( ACTING_WORK ));
	
	work->heapId = initWork->heapId;
	work->initWork = initWork;
	work->scrollOffset = 0;
	work->makuOffset = 256;
	STA_ACT_SetupGraphic( work );
	STA_ACT_SetupBg( work );
	STA_ACT_SetupMessage( work);
	STA_ACT_SetupPokemon( work );
	STA_ACT_SetupItem( work );
	STA_ACT_SetupEffect( work );
	
	INFOWIN_Init( ACT_FRAME_SUB_INFO,ACT_PAL_INFO,work->heapId);

	SND_STRM_SetUp( ARCID_SNDSTRM, NARC_snd_strm_poketari_swav, SND_STRM_PCM8, SND_STRM_8KHZ, work->heapId );
	SND_STRM_Play();

	//フェードないので仮処理
	GX_SetMasterBrightness(0);	
	GXS_SetMasterBrightness(0);
	
	return work;
}

void	STA_ACT_InitActing_2nd( ACTING_WORK *work )
{
	VecFx32 pos = {ACT_POS_X(128.0f),ACT_POS_X(96.0f),FX32_CONST(190.0f)};
	STA_EFF_CreateEmmitter( work->effWork[0] , MUS_EFF_00_OP_TITLE01 , &pos );
	STA_EFF_CreateEmmitter( work->effWork[1] , MUS_EFF_01_OP_DEMOBG2_BUBBLE1 , &pos );
}

void	STA_ACT_TermActing( ACTING_WORK *work )
{
	//フェードないので仮処理
	GX_SetMasterBrightness(-16);	
	GXS_SetMasterBrightness(-16);

	SND_STRM_Stop();
	SND_STRM_Release();
	
	INFOWIN_Exit();

	GFL_BMPWIN_Delete( work->msgWin );
	GFL_FONT_Delete( work->fontHandle );
	GFL_TCBL_Exit( work->tcblSys );
	
	STA_EFF_ExitSystem( work->effSys );
	STA_OBJ_ExitSystem( work->objSys );
	STA_POKE_ExitSystem( work->pokeSys );

	MUS_POKE_DRAW_TermSystem( work->drawSys );
	MUS_ITEM_DRAW_TermSystem( work->itemDrawSys );

	GFL_BBD_DeleteSys( work->bbdSys );
	GFL_G3D_CAMERA_Delete( work->camera );
	GFL_G3D_Exit();
	GFL_BG_FreeBGControl( ACT_FRAME_MAIN_3D );
	GFL_BG_FreeBGControl( ACT_FRAME_MAIN_FONT );
	GFL_BG_FreeBGControl( ACT_FRAME_MAIN_CURTAIN );
	GFL_BG_FreeBGControl( ACT_FRAME_MAIN_BG );
	GFL_BG_FreeBGControl( ACT_FRAME_SUB_BG );
	GFL_BG_FreeBGControl( ACT_FRAME_SUB_INFO );
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();

	GFL_HEAP_FreeMemory( work );
	
}

ACTING_RETURN	STA_ACT_LoopActing( ACTING_WORK *work )
{
#if DEB_ARI
	if(	GFL_UI_KEY_GetTrg() & PAD_BUTTON_R &&
		(GFL_UI_KEY_GetCont() & PAD_BUTTON_L) == 0)
	{
		STA_ACT_InitActing_2nd( work );
	}
#endif

	INFOWIN_Update();
	STA_ACT_UpdateScroll(work);

#if DEB_ARI
	if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_L) == 0 ||
		GFL_UI_KEY_GetTrg() & PAD_BUTTON_R )
#endif
	{
		STA_POKE_UpdateSystem( work->pokeSys );
		STA_OBJ_UpdateSystem( work->objSys );
		STA_EFF_UpdateSystem( work->effSys );
		STA_ACT_UpdateMessage( work );
		
		MUS_POKE_DRAW_UpdateSystem( work->drawSys ); 
	}

	//3D描画	
	GFL_G3D_DRAW_Start();
	GFL_G3D_DRAW_SetLookAt();
	{
//		STA_POKE_DrawSystem( work->pokeSys );
		MUS_POKE_DRAW_DrawSystem( work->drawSys ); 
//		STA_OBJ_DrawSystem( work->objSys );
		STA_POKE_UpdateSystem_Item( work->pokeSys );	//ポケの描画後に入れること
		GFL_BBD_Draw( work->bbdSys , work->camera , NULL );
		STA_EFF_DrawSystem( work->effSys );
	}
	GFL_G3D_DRAW_End();

	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT &&
		GFL_UI_KEY_GetCont() & PAD_BUTTON_START )
	{
		return ACT_RET_GO_END;
	}

	return ACT_RET_CONTINUE;
}


//--------------------------------------------------------------
//	描画系初期化
//--------------------------------------------------------------
static void STA_ACT_SetupGraphic( ACTING_WORK *work )
{
	GFL_DISP_GX_InitVisibleControl();
	GFL_DISP_GXS_InitVisibleControl();
	GX_SetVisiblePlane( 0 );
	GXS_SetVisiblePlane( 0 );
	WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
	WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
	WIPE_ResetWndMask(WIPE_DISP_MAIN);
	WIPE_ResetWndMask(WIPE_DISP_SUB);
	
	GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
	GFL_DISP_SetBank( &vramBank );
	
	GFL_BG_Init( work->heapId );
	GFL_BMPWIN_Init( work->heapId );
	
	//Vram割り当ての設定
	{
		static const GFL_BG_SYS_HEADER sys_data = {
				GX_DISPMODE_GRAPHICS, GX_BGMODE_3, GX_BGMODE_0, GX_BG0_AS_3D,
		};
		
		// BG1 MAIN (文字
		static const GFL_BG_BGCNT_HEADER header_main1 = {
			0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x4800, GX_BG_CHARBASE_0x08000,0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
		};
		// BG2 MAIN (幕
		static const GFL_BG_BGCNT_HEADER header_main2 = {
			0, 0, 0x1000, 0,	// scrX, scrY, scrbufSize, scrbufofs,
			GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x5000, GX_BG_CHARBASE_0x00000,0x4000,
			GX_BG_EXTPLTT_01, 1, 1, 0, FALSE	// pal, pri, areaover, dmy, mosaic
		};
		// BG3 MAIN (背景 256*16色
		static const GFL_BG_BGCNT_HEADER header_main3 = {
			0, 0, 0x2000, 0,	// scrX, scrY, scrbufSize, scrbufofs,
			GFL_BG_SCRSIZ_512x512, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x10000,0x10000,
			GX_BG_EXTPLTT_23, 3, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
		};

		// BG1 SUB (背景
		static const GFL_BG_BGCNT_HEADER header_sub1 = {
			0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000,0x8000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
		};
		// BG3 SUB (Info
		static const GFL_BG_BGCNT_HEADER header_sub3 = {
			0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000,0x6000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
		};

		GFL_BG_SetBGMode( &sys_data );
		STA_ACT_SetupBgFunc( &header_main1, ACT_FRAME_MAIN_FONT , GFL_BG_MODE_TEXT);
		STA_ACT_SetupBgFunc( &header_main2, ACT_FRAME_MAIN_CURTAIN , GFL_BG_MODE_TEXT);
		STA_ACT_SetupBgFunc( &header_main3, ACT_FRAME_MAIN_BG , GFL_BG_MODE_256X16);
		STA_ACT_SetupBgFunc( &header_sub1 , ACT_FRAME_SUB_BG , GFL_BG_MODE_TEXT);
		STA_ACT_SetupBgFunc( &header_sub3 , ACT_FRAME_SUB_INFO , GFL_BG_MODE_TEXT);
		
		GFL_BG_SetVisible( ACT_FRAME_MAIN_3D , TRUE );
	}
	
	{	//3D系の設定
		static const VecFx32 cam_pos = MUSICAL_CAMERA_POS;
		static const VecFx32 cam_target = MUSICAL_CAMERA_TRG;
		static const VecFx32 cam_up = MUSICAL_CAMERA_UP;
		//エッジマーキングカラー
		static	const	GXRgb stage_edge_color_table[8]=
			{ GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 0, 0, 0, 0, 0, 0 };
		GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT80K,
						0, work->heapId, NULL );
		GFL_BG_SetBGControl3D( 2 );	//NNS_g3dInitの中で表示優先順位変わる・・・
		GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );
#if 0	//透視射影カメラ
		work->camera = 	GFL_G3D_CAMERA_Create( GFL_G3D_PRJPERS, 
											 FX32_SIN13,
											 FX32_COS13,
											 FX_F32_TO_FX32( 1.33f ),
											 NULL,
											 FX32_ONE,
											 FX32_ONE * 300,
											 NULL,
											 &cam_pos,
											 &cam_up,
											 &cam_target,
											 work->heapId );
#else
		//正射影カメラ
		work->camera = 	GFL_G3D_CAMERA_Create( GFL_G3D_PRJORTH, 
											 FX32_ONE*12.0f,
											 0,
											 0,
											 FX32_ONE*16.0f,
											 MUSICAL_CAMERA_NEAR,
											 MUSICAL_CAMERA_FAR,
											 NULL,
											 &cam_pos,
											 &cam_up,
											 &cam_target,
											 work->heapId );
#endif
		
		GFL_G3D_CAMERA_Switching( work->camera );
		//エッジマーキングカラーセット
		G3X_SetEdgeColorTable( &stage_edge_color_table[0] );
		G3X_EdgeMarking( TRUE );
		G3X_AntiAlias( FALSE );
		G3X_AlphaBlend( TRUE );
		
		GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO , GX_BUFFERMODE_Z );
	}
	{
		GFL_BBD_SETUP bbdSetup = {
			128,128,
			{FX32_ONE,FX32_ONE,FX32_ONE},
			GX_RGB(0,0,0),
			GX_RGB(0,0,0),
			GX_RGB(0,0,0),
			GX_RGB(0,0,0),
			0
		};
		//ビルボードシステム構築
		work->bbdSys = GFL_BBD_CreateSys( &bbdSetup , work->heapId );
	}
	
	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0 , GX_BLEND_PLANEMASK_BG3 , 31 , 31 );
}

//--------------------------------------------------------------
//	BGの初期化
//--------------------------------------------------------------
static void STA_ACT_SetupBg( ACTING_WORK *work )
{
	ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_STAGE_GRA , work->heapId );

	GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_stage_gra_stage_bg00_NCLR , 
										PALTYPE_MAIN_BG_EX , 0x6000 , 0 , work->heapId );
	GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_stage_gra_stage_bg00_NCGR ,
										ACT_FRAME_MAIN_BG , 0 , 0, FALSE , work->heapId );
	GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_stage_gra_stage_bg_NSCR , 
										ACT_FRAME_MAIN_BG ,  0 , 0, FALSE , work->heapId );

	GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_stage_gra_maku_NCLR , 
										PALTYPE_MAIN_BG , 0 , 0 , work->heapId );
	GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_stage_gra_maku_NCGR ,
										ACT_FRAME_MAIN_CURTAIN , 0 , 0, FALSE , work->heapId );
	GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_stage_gra_maku_NSCR , 
										ACT_FRAME_MAIN_CURTAIN ,  0 , 0, FALSE , work->heapId );

	GFL_ARCHDL_UTIL_TransVramPalette( arcHandle , NARC_stage_gra_audience_NCLR , 
										PALTYPE_SUB_BG , 0 , 0 , work->heapId );
	GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHandle , NARC_stage_gra_audience_NCGR ,
										ACT_FRAME_SUB_BG , 0 , 0, FALSE , work->heapId );
	GFL_ARCHDL_UTIL_TransVramScreen( arcHandle , NARC_stage_gra_audience_NSCR , 
										ACT_FRAME_SUB_BG ,  0 , 0, FALSE , work->heapId );

	GFL_ARC_CloseDataHandle(arcHandle);

	GFL_BG_LoadScreenReq(ACT_FRAME_MAIN_BG);
	GFL_BG_LoadScreenReq(ACT_FRAME_SUB_BG);

}

//--------------------------------------------------------------------------
//	Bg初期化 機能部
//--------------------------------------------------------------------------
static void	STA_ACT_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
	GFL_BG_SetBGControl( bgPlane, bgCont, mode );
	GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
	GFL_BG_ClearFrame( bgPlane );
	GFL_BG_LoadScreenReq( bgPlane );
}

//--------------------------------------------------------------
//	表示ポケモンの初期化
//--------------------------------------------------------------
static void STA_ACT_SetupPokemon( ACTING_WORK *work )
{
	u8 i;
	VecFx32 pos = {0,FX32_CONST(160.0f),FX32_CONST(170.0f)};
	const fx32 XBase = FX32_CONST(512.0f/(MUSICAL_POKE_MAX+1));
	
	work->drawSys = MUS_POKE_DRAW_InitSystem( work->heapId );
	work->itemDrawSys = MUS_ITEM_DRAW_InitSystem( work->bbdSys , MUSICAL_POKE_MAX*MUS_POKE_EQUIP_MAX, work->heapId );
	
	work->pokeSys = STA_POKE_InitSystem( work->heapId , work->drawSys , work->itemDrawSys );

	for( i=0;i<MUSICAL_POKE_MAX;i++ )
	{
		work->pokeWork[i] = NULL;
	}

	
	for( i=0;i<MUSICAL_POKE_MAX;i++ )
	{
		pos.x = XBase*(i+1);
		work->pokeWork[i] = STA_POKE_CreatePoke( work->pokeSys , &work->initWork->musPoke[i] );
		STA_POKE_SetPosition( work->pokeSys , work->pokeWork[i] , &pos );

		pos.z -= FX32_CONST(30.0f);	//ひとキャラの厚みは30と見る
	}
}

//--------------------------------------------------------------
//アイテムの初期化
//--------------------------------------------------------------
static void STA_ACT_SetupItem( ACTING_WORK *work )
{
	int i;
	VecFx32 pos = {0,0,0};
	
	for( i=0;i<ACT_OBJECT_MAX;i++ )
	{
		work->objWork[i] = NULL;
	}

	//背景Obj
	{
		work->objSys = STA_OBJ_InitSystem( work->heapId , work->bbdSys );

		pos.x = FX32_CONST(448.0f );
		pos.y = FX32_CONST( 96.0f );
		pos.z = FX32_CONST( 50.0f);
		work->objWork[0] = STA_OBJ_CreateObject( work->objSys , 0 );
		STA_OBJ_SetPosition( work->objSys , work->objWork[0] , &pos );

		pos.x = FX32_CONST( 192.0f );
		pos.y = FX32_CONST( 100.0f );
		pos.z = FX32_CONST( 50.0f );
		work->objWork[1] = STA_OBJ_CreateObject( work->objSys , 1 );
		STA_OBJ_SetPosition( work->objSys , work->objWork[1] , &pos );

		pos.x = FX32_CONST(  80.0f );
		pos.y = FX32_CONST( 160.0f );
		pos.z = FX32_CONST( 180.0f );
		work->objWork[2] = STA_OBJ_CreateObject( work->objSys , 1 );
		STA_OBJ_SetPosition( work->objSys , work->objWork[2] , &pos );
	}


}

static void STA_ACT_SetupEffect( ACTING_WORK *work )
{
	int i;
	work->effSys = STA_EFF_InitSystem( work->heapId);

	for( i=0;i<ACT_EFFECT_MAX;i++ )
	{
		work->effWork[i] = NULL;
	}

	work->effWork[0] = STA_EFF_CreateEffect( work->effSys , NARC_stage_gra_mus_eff_00_spa );
	work->effWork[1] = STA_EFF_CreateEffect( work->effSys , NARC_stage_gra_mus_eff_01_spa );
}

static void STA_ACT_SetupMessage( ACTING_WORK *work )
{
	//メッセージ用処理
	work->msgWin = GFL_BMPWIN_Create( ACT_FRAME_MAIN_FONT , ACT_MSG_POS_X , ACT_MSG_POS_Y ,
									ACT_MSG_POS_WIDTH , ACT_MSG_POS_HEIGHT , ACT_PAL_FONT ,
									GFL_BMP_CHRAREA_GET_B );
	GFL_BMPWIN_TransVramCharacter( work->msgWin );
	GFL_BMPWIN_MakeScreen( work->msgWin );
	GFL_BG_LoadScreenReq(ACT_FRAME_MAIN_FONT);
	
	
	//フォント読み込み
	work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_nftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );

	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , ACT_PAL_FONT*0x20, 16*1, work->heapId );
	
	work->tcblSys = GFL_TCBL_Init( work->heapId , work->heapId , 3 , 0x100 );
	work->printHandle = NULL;
}



static void STA_ACT_UpdateItem( ACTING_WORK *work )
{
	int i;
	int ePos;
	VecFx32 pos;
/*	
	for( i=0;i<MUSICAL_POKE_MAX;i++ )
	{
		VecFx32 pokePos;
		MUS_POKE_DRAW_GetPosition( work->drawWork[i] , &pokePos);
		for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
		{
			if( work->itemWork[i][ePos] != NULL )
			{
				MUS_POKE_EQUIP_DATA *equipData = MUS_POKE_DRAW_GetEquipData( work->drawWork[i] , ePos );
				if( equipData->isEnable == TRUE )
				{
					pos.x = ACT_POS_X_FX(equipData->pos.x+FX32_CONST(128.0f));
					pos.y = ACT_POS_Y_FX(equipData->pos.y+FX32_CONST(96.0f));
					pos.z = pokePos.z+FX32_HALF;	//とりあえずポケの前に出す
					//OS_Printf("[%.2f][%.2f]\n",F32_CONST(equipData->pos.z),F32_CONST(pokePos.z));
					MUS_ITEM_DRAW_SetPosition(	work->itemDrawSys , 
												work->itemWork[i][ePos] ,
												&pos );
					MUS_ITEM_DRAW_SetRotation(	work->itemDrawSys , 
												work->itemWork[i][ePos] ,
												equipData->rot );
					
				}
			}
		}
	}
*/
}

static void STA_ACT_UpdateScroll( ACTING_WORK *work )
{
	BOOL isUpdate = FALSE;
	const u8 spd = 2;
	if(	GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
	{
		if( work->scrollOffset + spd < ACT_BG_SCROLL_MAX )
		{
			work->scrollOffset += spd;
		}
		else
		{
			work->scrollOffset = ACT_BG_SCROLL_MAX;
		}
		isUpdate = TRUE;
	}
	if(	GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
	{
		if( work->scrollOffset - spd > 0 )
		{
			work->scrollOffset -= spd;
		}
		else
		{
			work->scrollOffset = 0;
		}
		
		isUpdate = TRUE;
	}
	
	if( isUpdate == TRUE )
	{
		GFL_BG_SetScroll( ACT_FRAME_MAIN_BG , GFL_BG_SCROLL_X_SET , work->scrollOffset );
		GFL_BG_SetScroll( ACT_FRAME_MAIN_CURTAIN , GFL_BG_SCROLL_X_SET , work->scrollOffset );
		STA_POKE_System_SetScrollOffset( work->pokeSys , work->scrollOffset ); 
		STA_OBJ_System_SetScrollOffset( work->objSys , work->scrollOffset ); 
	}
	
	if(	GFL_UI_KEY_GetCont() & PAD_KEY_UP )
	{
		if( work->makuOffset + spd < ACT_MAKU_SCROLL_MAX )
		{
			work->makuOffset += spd;
		}
		else
		{
			work->makuOffset = ACT_MAKU_SCROLL_MAX;
		}
	}
	if(	GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )
	{
		if( work->makuOffset - spd > 0 )
		{
			work->makuOffset -= spd;
		}
		else
		{
			work->makuOffset = 0;
		}
		
	}
	GFL_BG_SetScroll( ACT_FRAME_MAIN_CURTAIN , GFL_BG_SCROLL_Y_SET , work->makuOffset );
}

#include <wchar.h>	//wcslen
static void STA_ACT_UpdateMessage( ACTING_WORK *work )
{
	GFL_TCBL_Main( work->tcblSys );

	if( work->printHandle != NULL  )
	{
		if( PRINTSYS_PrintStreamGetState( work->printHandle ) == PRINTSTREAM_STATE_DONE )
		{
			PRINTSYS_PrintStreamDelete( work->printHandle );
			work->printHandle = NULL;
		}
	}

#if DEB_ARI
	if( work->printHandle == NULL )
	{
		if( GFL_UI_KEY_GetCont() & PAD_BUTTON_A )
		{
			STRCODE str[48] = L"文字列表示テスト";
			STRBUF	*testStr = GFL_STR_CreateBuffer( 48 , work->heapId );
			const u16 strLen = wcslen(str);
			
			GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->msgWin ) , 0 );
			
			str[strLen] = GFL_STR_GetEOMCode();
			GFL_STR_SetStringCode( testStr , str );

			work->printHandle = PRINTSYS_PrintStream( work->msgWin , 0,0, testStr ,work->fontHandle ,
													1 , work->tcblSys , 2 , work->heapId , 0 );
			GFL_STR_DeleteBuffer( testStr );
		}
	}
#endif
}
