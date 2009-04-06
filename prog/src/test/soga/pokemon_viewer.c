//============================================================================================
/**
 * @file	pokemon_viewer.c
 * @brief	MCSを利用してポケモンを見るビューワー
 * @author	soga
 * @date	2009.04.01
 */
//============================================================================================

#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include <textprint.h>

#include "system/main.h"
#include "arc_def.h"

#include "battle/btlv/btlv_effect.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#include "gf_mcs.h"

#define	PAD_BUTTON_EXIT	( PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_START )

#define	CAMERA_SPEED		( FX32_ONE * 2 )
#define	MOVE_SPEED			( FX32_ONE >> 2 )
#define	ROTATE_SPEED		( 65536 / 128 ) 
#define	PHI_MAX				( ( 65536 / 360 ) * 89 )
#define	DEFAULT_PHI			( 65536 / 12 )
#define	DEFAULT_THETA		( 65536 / 4 )
#define	DEFAULT_SCALE		( FX32_ONE * 16  )
#define	SCALE_SPEED			( FX32_ONE )
#define	SCALE_MAX			( FX32_ONE * 200 )

#define	HEAD_SIZE			( 4 )

#define	MCS_READ_CH			( 0 )
#define	MCS_WRITE_CH		( 0 )

#define G2D_BACKGROUND_COL	( GX_RGB( 31, 31, 31 ) )
#define G2D_FONT_COL		( GX_RGB(  0,  0,  0 ) )
#define G2D_AA_BGCOL_ON		( GX_RGB( 31,  0,  0 ) )
#define G2D_BB_BGCOL_ON		( GX_RGB(  0, 31,  0 ) )
#define G2D_A_BGCOL_ON		( GX_RGB(  0,  0, 31 ) )
#define G2D_B_BGCOL_ON		( GX_RGB( 31, 31,  0 ) )
#define G2D_C_BGCOL_ON		( GX_RGB( 31,  0, 31 ) )
#define G2D_D_BGCOL_ON		( GX_RGB(  0, 31, 31 ) )
#define G2D_AA_BGCOL_OFF	( GX_RGB( 15,  0,  0 ) )
#define G2D_BB_BGCOL_OFF	( GX_RGB(  0, 15,  0 ) )
#define G2D_A_BGCOL_OFF		( GX_RGB(  0,  0, 15 ) )
#define G2D_B_BGCOL_OFF		( GX_RGB( 15, 15,  0 ) )
#define G2D_C_BGCOL_OFF		( GX_RGB( 15,  0, 15 ) )
#define G2D_D_BGCOL_OFF		( GX_RGB(  0, 15, 15 ) )


enum{
	BGCOL_AA = 2,
	BGCOL_BB,
	BGCOL_A,
	BGCOL_B,
	BGCOL_C,
	BGCOL_D,
};

typedef struct{
	u8 posx;
	u8 posy;
	u8 sizx;
	u8 sizy;

	u8 palnum;
	u8 msgx;
	u8 msgy;
	u8 padding;
}BMP_CREATE_TABLE;

enum{
	SEND_NCBR = 0,
	SEND_NCBR_OK,
	SEND_NCLR,
	SEND_NCLR_OK,
	SEND_NCER,
	SEND_NCER_OK,
	SEND_NMCR,
	SEND_NMCR_OK,
	SEND_NANR,
	SEND_NANR_OK,
	SEND_NMAR,
	SEND_NMAR_OK,
	SEND_NCEC,
	SEND_NCEC_OK,
	SEND_POS,
	SEND_POS_OK,
	SEND_IDLE,
};

enum{
	RESOURCE_NCBR,
	RESOURCE_NCLR,
	RESOURCE_NCER,
	RESOURCE_NMCR,
	RESOURCE_NANR,
	RESOURCE_NMAR,
	RESOURCE_NCEC,
	RESOURCE_MAX,
};

enum{
	SEQ_IDLE = 0,
	SEQ_LOAD_POS_DATA,
	SEQ_LOAD_RESOURCE_DATA,
};

enum{
	BMPWIN_AA = 0,
	BMPWIN_BB,
	BMPWIN_A,
	BMPWIN_B,
	BMPWIN_C,
	BMPWIN_D,

	BMPWIN_MAX
};

typedef struct
{
	HEAPID				heapID;

	int					seq_no;
	int					mcs_enable;
	POKEMON_PARAM		*pp;
	int					mons_no;
	GFL_BMPWIN			*bmpwin[ BMPWIN_MAX ];
	GFL_TEXT_PRINTPARAM	*textParam;
	int					key_repeat_speed;
	int					key_repeat_wait;
	int					read_position;
	int					read_resource;
	int					add_pokemon_req;

	void				*resource_data[ RESOURCE_MAX ][ BTLV_MCSS_POS_MAX ];
}POKEMON_VIEWER_WORK;

static	void	PokemonViewerSequence( POKEMON_VIEWER_WORK *pvw );
static	void	PokemonViewerRead( POKEMON_VIEWER_WORK *pvw );
static	void	PokemonViewerPositionLoad( POKEMON_VIEWER_WORK *pvw );
static	void	PokemonViewerResourceLoad( POKEMON_VIEWER_WORK *pvw );
static	void	PokemonViewerAddPokemon( POKEMON_VIEWER_WORK *pvw );
static	void	PokemonViewerCameraWork( POKEMON_VIEWER_WORK *pvw );

static	void	TextPrint( POKEMON_VIEWER_WORK *pvw, int num, int bmpwin_num );
static	void	NumPrint( POKEMON_VIEWER_WORK *pvw, int num, int bmpwin_num );
static	void	Num16Print( POKEMON_VIEWER_WORK *pvw, int num, int bmpwin_num );

static	void	MoveCamera( POKEMON_VIEWER_WORK *pvw );

#if 0
static	void	set_pokemon( POKEMON_VIEWER_WORK *pvw );
static	void	del_pokemon( POKEMON_VIEWER_WORK *pvw );
#endif

static	const	int	pokemon_pos_table[][2]={ 
	{ BTLV_MCSS_POS_AA, BTLV_MCSS_POS_BB },
	{ BTLV_MCSS_POS_A, BTLV_MCSS_POS_B },
	{ BTLV_MCSS_POS_C, BTLV_MCSS_POS_D }
};

static	const	char	num_char_table[][1]={
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F"
};

static	const	char	btlv_mcss_pos_msg[BMPWIN_MAX][8]={
	"POS AA",
	"POS BB",
	"POS A",
	"POS B",
	"POS C",
	"POS D",
};

static const BMP_CREATE_TABLE bmp_create_table[] = {
	//AA
	{ 11, 12, 10, 12, BGCOL_AA, 24, 40, 0 }, 
	//BB
	{ 11,  0, 10, 12, BGCOL_BB, 24, 40, 0 }, 
	//A
	{  0, 12, 11, 12, BGCOL_A,  28, 40, 0 }, 
	//B
	{ 21,  0, 11, 12, BGCOL_B,  28, 40, 0 }, 
	//C
	{ 21, 12, 11, 12, BGCOL_C,  28, 40, 0 }, 
	//D
	{  0,  0, 11, 12, BGCOL_D,  28, 40, 0 }, 
};

static const GFL_UI_TP_HITTBL TP_HitTbl[] = {
	{  96, 191,  88, 167 },
	{   0,  95,  88, 167 },
	{  96, 191,   0,  87 },
	{   0,  95, 168, 255 },
	{  96, 191, 168, 255 },
	{   0,  95,   0,  87 },
	{ GFL_UI_TP_HIT_END, 0, 0, 0 },
};

FS_EXTERN_OVERLAY(battle);

//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT PokemonViewerProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	POKEMON_VIEWER_WORK *pvw;

	GFL_OVERLAY_Load(FS_OVERLAY_ID(battle));

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SOGABE_DEBUG, 0xc0000 );
	pvw = GFL_PROC_AllocWork( proc, sizeof( POKEMON_VIEWER_WORK ), HEAPID_SOGABE_DEBUG );
	MI_CpuClearFast( pvw, sizeof( POKEMON_VIEWER_WORK ) );
	pvw->heapID = HEAPID_SOGABE_DEBUG;
		
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
			GX_VRAM_TEX_01_BC,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_01_FG,			// テクスチャパレットスロット			
			GX_OBJVRAMMODE_CHAR_1D_64K,		// メインOBJマッピングモード
			GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
		};		
		GFL_DISP_SetBank( &dispvramBank );

		//VRAMクリア
		MI_CpuClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
		MI_CpuClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
		MI_CpuClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
		MI_CpuClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
		MI_CpuFill16((void*)HW_BG_PLTT, 0x0000, HW_BG_PLTT_SIZE);

	}	

	G2_BlendNone();
	GFL_BG_Init( pvw->heapID );
	GFL_BMPWIN_Init( pvw->heapID );

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
		GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_OFF );
		
	}		
	GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

	//3D関連初期化
	{
		GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K, GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0, pvw->heapID, NULL );
		GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
		G3X_AlphaBlend( TRUE );
		G3X_EdgeMarking( TRUE );
		G3X_AntiAlias( TRUE );
		GFL_BG_SetBGControl3D( 1 );
	}

	//戦闘エフェクト初期化
	{
		BTLV_EFFECT_Init( 0, pvw->heapID );
	}

//	set_pokemon( pvw );

	//2D画面初期化
	{
		GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
			///<FRAME1_M
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
			///<FRAME2_M
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
			///<FRAME3_M
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
			///<FRAME1_S
			{
				0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
				GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,
				GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			},
		};
		GFL_BG_SetBGControl(GFL_BG_FRAME1_M, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME1_M );
		GFL_BG_SetBGControl(GFL_BG_FRAME2_M, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME2_M );
		GFL_BG_SetBGControl(GFL_BG_FRAME3_M, &TextBgCntDat[2], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME3_M );
		GFL_BG_SetBGControl(GFL_BG_FRAME1_S, &TextBgCntDat[3], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME1_S );
	}

	{
		static const GFL_TEXT_PRINTPARAM default_param = { NULL, 0, 0, 1, 1, 1, 0, GFL_TEXT_WRITE_16 };
		int	i;

		pvw->textParam = GFL_HEAP_AllocMemory( pvw->heapID, sizeof( GFL_TEXT_PRINTPARAM ) );
		*pvw->textParam = default_param;


		//フォントパレット作成＆転送
		{
			static	u16 plt[16] = {
				G2D_BACKGROUND_COL, G2D_FONT_COL,
				G2D_AA_BGCOL_ON, G2D_BB_BGCOL_ON, G2D_A_BGCOL_ON, G2D_B_BGCOL_ON, G2D_C_BGCOL_ON, G2D_D_BGCOL_ON,
				G2D_AA_BGCOL_OFF, G2D_BB_BGCOL_OFF, G2D_A_BGCOL_OFF, G2D_B_BGCOL_OFF, G2D_C_BGCOL_OFF, G2D_D_BGCOL_OFF,
				0, 0 };
			GFL_BG_LoadPalette( GFL_BG_FRAME1_S, &plt, 16*2, 0 );
		}

		for( i = 0 ; i < BMPWIN_MAX ; i++ ){
			pvw->bmpwin[ i ] = GFL_BMPWIN_Create( GFL_BG_FRAME1_S,
												 bmp_create_table[ i ].posx,
												 bmp_create_table[ i ].posy,
												 bmp_create_table[ i ].sizx,
												 bmp_create_table[ i ].sizy,
												 0,
												 GFL_BG_CHRAREA_GET_B );
			TextPrint( pvw, i, i );
		}
		GFL_BG_LoadScreenReq( GFL_BG_FRAME1_S );
	}

	//ウインドマスク設定（画面両端のエッジマーキングのゴミを消す）
	{
		G2_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG0 |
							   GX_WND_PLANEMASK_BG1 |
							   GX_WND_PLANEMASK_BG2 |
							   GX_WND_PLANEMASK_BG3 |
							   GX_WND_PLANEMASK_OBJ,
							   FALSE );
		G2_SetWndOutsidePlane( GX_WND_PLANEMASK_NONE, FALSE );
		G2_SetWnd0Position( 1, 1, 255, 191 );
		GX_SetVisibleWnd( GX_WNDMASK_W0 );
	}

	GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x0000 );
	
	//フェードイン
	GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, 2 );

	GFL_UI_KEY_GetRepeatSpeed( &pvw->key_repeat_speed, &pvw->key_repeat_wait );
	GFL_UI_KEY_SetRepeatSpeed( pvw->key_repeat_speed / 4, pvw->key_repeat_wait );

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT PokemonViewerProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	int pad = GFL_UI_KEY_GetCont();
	int trg = GFL_UI_KEY_GetTrg();
	int rep = GFL_UI_KEY_GetRepeat();
	POKEMON_VIEWER_WORK *pvw = mywk;
	int	i, j;

	if( pvw->mcs_enable ){
		MCS_Main();
//		MCS_Write();
	}

	if( trg & PAD_BUTTON_START ){
		if( pvw->mcs_enable ){
#if 0
			pvw->seq_no = SEQ_IDLE;
			MCS_Exit();
			pvw->mcs_enable = 0;
			for( i = 0 ; i < BTLV_MCSS_POS_MAX ; i++ ){
				for( j = 0 ; j < RESOURCE_MAX ; j++ ){
					if( pvw->resource_data[ j ][ i ] ){
						if( j == RESOURCE_NCBR ){
							BTLV_EFFECT_DelPokemon( i );
						}
						pvw->resource_data[ j ][ i ] = NULL;
					}
				}
			}
#endif
			int	mcss_pos;
			VecFx32	pos,target;
			VecFx32	scale_value[] = {
				{ FX32_ONE * 16 * 2, FX32_ONE * 16 * 2, 0 },
				{ FX32_ONE * 16, FX32_ONE * 16, 0 }
			};

			BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
			BTLV_CAMERA_MoveCameraInterpolation( BTLV_EFFECT_GetCameraWork(), &pos, &target, 20, 0, 20 );

			for( mcss_pos = BTLV_MCSS_POS_AA ; mcss_pos < BTLV_MCSS_POS_MAX ; mcss_pos++ ){
				if( BTLV_MCSS_CheckExistPokemon( BTLV_EFFECT_GetMcssWork(), mcss_pos ) == TRUE ){
					BTLV_MCSS_MoveScale( BTLV_EFFECT_GetMcssWork(),
										 mcss_pos,
										 EFFTOOL_CALCTYPE_INTERPOLATION,
										 &scale_value[ mcss_pos & 1 ],
										 10, 1, 0 );
				}
			}
		}
		else{
			if( MCS_Init( pvw->heapID ) == FALSE ){
				pvw->mcs_enable = 1;
			}
		}
	}
	if( (trg & PAD_BUTTON_SELECT ) && ( BTLV_EFFECT_CheckExecute() == FALSE ) ){
		PokemonViewerCameraWork( pvw );
	}

	{
		int hit = GFL_UI_TP_HitTrg( TP_HitTbl );
		if( hit != GFL_UI_TP_HIT_NONE ){
			if( BTLV_MCSS_CheckExistPokemon( BTLV_EFFECT_GetMcssWork(), hit ) == TRUE ){
				BTLV_MCSS_SetVanishFlag( BTLV_EFFECT_GetMcssWork(), hit, BTLV_MCSS_VANISH_FLIP );
				TextPrint( pvw, hit, hit );
			}
		}
	}

	PokemonViewerSequence( pvw );

	MoveCamera( pvw );
	BTLV_EFFECT_Main();

	if( pad == PAD_BUTTON_EXIT ){
		GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 0, 16, 2 );
		return GFL_PROC_RES_FINISH;	
	}
	else{
		return GFL_PROC_RES_CONTINUE;	
	}
}
//--------------------------------------------------------------------------
/**
 * PROC Exit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT PokemonViewerProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	POKEMON_VIEWER_WORK *pvw = mywk;

	if( GFL_FADE_CheckFade() == TRUE ){
		return GFL_PROC_RES_CONTINUE;	
	}

	GFL_UI_KEY_SetRepeatSpeed( pvw->key_repeat_speed, pvw->key_repeat_wait );

	BTLV_EFFECT_Exit();

	GFL_G3D_Exit();

	GFL_BG_Exit();
	GFL_BMPWIN_Exit();

	GFL_PROC_FreeWork( proc );

	GFL_HEAP_DeleteHeap( HEAPID_SOGABE_DEBUG );

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA		PokemonViewerProcData = {
	PokemonViewerProcInit,
	PokemonViewerProcMain,
	PokemonViewerProcExit,
};

//======================================================================
//	メインシーケンス
//======================================================================
static	void	PokemonViewerSequence( POKEMON_VIEWER_WORK *pvw )
{
	switch( pvw->seq_no ){
	default:
	case SEQ_IDLE:
		PokemonViewerAddPokemon( pvw );
		PokemonViewerRead( pvw );
		break;
	case SEQ_LOAD_POS_DATA:
		PokemonViewerPositionLoad( pvw );
		break;
	case SEQ_LOAD_RESOURCE_DATA:
		PokemonViewerResourceLoad( pvw );
		break;
	}
}

//======================================================================
//	MCSからの読み込みチェック
//======================================================================
static	void	PokemonViewerRead( POKEMON_VIEWER_WORK *pvw )
{
	u32	size;
	u32	head;

	if( pvw->mcs_enable == 0 ){
		return;
	}

	size = MCS_CheckRead();

	if( size ){
		MCS_Read( &head, HEAD_SIZE );
		if( head == SEND_POS ){
			pvw->seq_no = SEQ_LOAD_POS_DATA;
		}
		else{
			pvw->seq_no = SEQ_LOAD_RESOURCE_DATA;
		}
		if( head == SEND_NCEC ){
			pvw->add_pokemon_req = 1;
		}
		pvw->read_resource = head / 2;
		head++;
		MCS_Write( MCS_WRITE_CH, &head, 4 );
	}
}

//======================================================================
//	MCSからのポジションデータ読み込みチェック
//======================================================================
static	void	PokemonViewerPositionLoad( POKEMON_VIEWER_WORK *pvw )
{
	u32	size;
	u32	head;
	int	res;

	size = MCS_CheckRead();

	if( size ){
		MCS_Read( &pvw->read_position, size );
		head = SEND_IDLE;
		MCS_Write( MCS_WRITE_CH, &head, 4 );
		pvw->seq_no = SEQ_IDLE;
		for( res = 0 ; res < RESOURCE_MAX ; res++ ){
			if( pvw->resource_data[ res ][ pvw->read_position ] ){
				if( res == RESOURCE_NCBR ){
					BTLV_EFFECT_DelPokemon( pvw->read_position );
				}
				pvw->resource_data[ res ][ pvw->read_position ] = NULL;
			}
		}
	}
}

//======================================================================
//	MCSからのシーケンス読み込みチェック
//======================================================================
static	void	PokemonViewerResourceLoad( POKEMON_VIEWER_WORK *pvw )
{
	u32	size;
	u32	head;

	size = MCS_CheckRead();

	if( size ){
		pvw->resource_data[ pvw->read_resource ][ pvw->read_position ] = GFL_HEAP_AllocClearMemory( pvw->heapID, size );
		MCS_Read( pvw->resource_data[ pvw->read_resource ][ pvw->read_position ], size );
		head = SEND_IDLE;
		MCS_Write( MCS_WRITE_CH, &head, 4 );
		pvw->seq_no = SEQ_IDLE;
	}
}

//======================================================================
//	テキスト表示
//======================================================================
static	void	TextPrint( POKEMON_VIEWER_WORK *pvw, int num, int bmpwin_num )
{
	int	flag = 0;

	if( BTLV_MCSS_CheckExistPokemon( BTLV_EFFECT_GetMcssWork(), num ) == TRUE ){
	   flag = BTLV_MCSS_GetVanishFlag( BTLV_EFFECT_GetMcssWork(), num );
	}

	pvw->textParam->writex = bmp_create_table[ bmpwin_num ].msgx;
	pvw->textParam->writey = bmp_create_table[ bmpwin_num ].msgy;
	pvw->textParam->bmp = GFL_BMPWIN_GetBmp( pvw->bmpwin[ bmpwin_num ] );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( pvw->bmpwin[ bmpwin_num ] ), bmp_create_table[ bmpwin_num ].palnum + 6 * flag );
	GFL_TEXT_PrintSjisCode( &btlv_mcss_pos_msg[ num ][ 0 ], pvw->textParam );
	GFL_BMPWIN_TransVramCharacter( pvw->bmpwin[ bmpwin_num ] );
	GFL_BMPWIN_MakeScreen( pvw->bmpwin[ bmpwin_num ] );
}

//======================================================================
//	数字表示
//======================================================================
static	void	NumPrint( POKEMON_VIEWER_WORK *pvw, int num, int bmpwin_num )
{
	char	num_char[ 4 ] = "\0\0\0\0";
	int		num_100,num_10,num_1;

	num_100 = num / 100;
	num_10  = ( num - ( num_100 * 100 ) ) / 10;
	num_1   = num - ( num_100 * 100 ) - (num_10 * 10 );

	num_char[ 0 ] = num_char_table[ num_100 ][ 0 ];
	num_char[ 1 ] = num_char_table[ num_10  ][ 0 ];
	num_char[ 2 ] = num_char_table[ num_1   ][ 0 ];

	pvw->textParam->writex = 0;
	pvw->textParam->writey = 0;
	pvw->textParam->bmp = GFL_BMPWIN_GetBmp( pvw->bmpwin[ bmpwin_num ] );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( pvw->bmpwin[ bmpwin_num ] ), 0 );
	GFL_TEXT_PrintSjisCode( &num_char[0], pvw->textParam );
	GFL_BMPWIN_TransVramCharacter( pvw->bmpwin[ bmpwin_num ] );
	GFL_BMPWIN_MakeScreen( pvw->bmpwin[ bmpwin_num ] );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
}

static	void	Num16Print( POKEMON_VIEWER_WORK *pvw, int num, int bmpwin_num )
{
	int		i;
	char	num_char[ 9 ] = "\0\0\0\0\0\0\0\0\0";

	for( i = 0 ; i < 8 ; i++ ){
		num_char[ i ] = num_char_table[ ( num >> ( 28 - 4 * i ) ) & 0x0000000f ][ 0 ];
	}

	pvw->textParam->writex = 0;
	pvw->textParam->writey = 0;
	pvw->textParam->bmp = GFL_BMPWIN_GetBmp( pvw->bmpwin[ bmpwin_num ] );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( pvw->bmpwin[ bmpwin_num ] ), 0 );
	GFL_TEXT_PrintSjisCode( &num_char[0], pvw->textParam );
	GFL_BMPWIN_TransVramCharacter( pvw->bmpwin[ bmpwin_num ] );
	GFL_BMPWIN_MakeScreen( pvw->bmpwin[ bmpwin_num ] );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
}

//======================================================================
//	カメラの制御
//======================================================================
static	void	MoveCamera( POKEMON_VIEWER_WORK *pvw )
{
	int pad = GFL_UI_KEY_GetCont();
	VecFx32	pos,ofsx,ofsz,camPos, camTarget;
	fx32	xofs=0,yofs=0,zofs=0;		

	if( pad & PAD_BUTTON_L ){
		if( pad & PAD_KEY_LEFT ){
			BTLV_CAMERA_MoveCameraAngle( BTLV_EFFECT_GetCameraWork(), 0, -ROTATE_SPEED );
		}
		if( pad & PAD_KEY_RIGHT ){
			BTLV_CAMERA_MoveCameraAngle( BTLV_EFFECT_GetCameraWork(), 0,  ROTATE_SPEED );
		}
		if( pad & PAD_KEY_UP ){
			BTLV_CAMERA_MoveCameraAngle( BTLV_EFFECT_GetCameraWork(), -ROTATE_SPEED, 0 );
		}
		if( pad & PAD_KEY_DOWN ){
			BTLV_CAMERA_MoveCameraAngle( BTLV_EFFECT_GetCameraWork(),  ROTATE_SPEED, 0 );
		}
	}
	else{
		BTLV_CAMERA_GetCameraPosition( BTLV_EFFECT_GetCameraWork(), &camPos, &camTarget );

		if( pad & PAD_KEY_LEFT ){
			xofs = -MOVE_SPEED;
		}
		if( pad & PAD_KEY_RIGHT ){
			xofs = MOVE_SPEED;
		}

		if( pad & PAD_BUTTON_R ){
			if( pad & PAD_KEY_UP ){
				zofs = MOVE_SPEED;
			}
			if( pad & PAD_KEY_DOWN ){
				zofs = -MOVE_SPEED;
			}
		}
		else{
			if( pad & PAD_KEY_UP ){
				yofs = MOVE_SPEED;
			}
			if( pad & PAD_KEY_DOWN ){
				yofs = -MOVE_SPEED;
			}
		}
		pos.x = camPos.x - camTarget.x;
		pos.y = 0;
		pos.z = camPos.z - camTarget.z;
		VEC_Normalize( &pos, &pos );

		ofsx.x = FX_MUL( pos.z, xofs );
		ofsx.y = 0;
		ofsx.z = -FX_MUL( pos.x, xofs );

		ofsz.x = -FX_MUL( pos.x, zofs );
		ofsz.y = yofs;
		ofsz.z = -FX_MUL( pos.z, zofs );

		camPos.x += ofsx.x + ofsz.x;
		camPos.y += ofsx.y + ofsz.y;
		camPos.z += ofsx.z + ofsz.z;
		camTarget.x += ofsx.x + ofsz.x;
		camTarget.y += ofsx.y + ofsz.y;
		camTarget.z += ofsx.z + ofsz.z;

		BTLV_CAMERA_MoveCameraPosition( BTLV_EFFECT_GetCameraWork(), &camPos, &camTarget );
	}

}

static	void	PokemonViewerAddPokemon( POKEMON_VIEWER_WORK *pvw )
{
	MCSS_ADD_DEBUG_WORK madw;

	if( pvw->add_pokemon_req ){
		pvw->add_pokemon_req = 0;
		madw.ncbr = pvw->resource_data[ RESOURCE_NCBR ][ pvw->read_position ];
		madw.nclr = pvw->resource_data[ RESOURCE_NCLR ][ pvw->read_position ];
		madw.ncer = pvw->resource_data[ RESOURCE_NCER ][ pvw->read_position ];
		madw.nmcr = pvw->resource_data[ RESOURCE_NMCR ][ pvw->read_position ];
		madw.nanr = pvw->resource_data[ RESOURCE_NANR ][ pvw->read_position ];
		madw.nmar = pvw->resource_data[ RESOURCE_NMAR ][ pvw->read_position ];
		madw.ncec = pvw->resource_data[ RESOURCE_NCEC ][ pvw->read_position ];

		BTLV_EFFECT_SetPokemonDebug( &madw, pvw->read_position );
	}
}

static	void	PokemonViewerCameraWork( POKEMON_VIEWER_WORK *pvw )
{
	int	mcss_pos;
	VecFx32	pos,target;
	VecFx32	scale_value[] = {
		{ FX32_ONE * 16, FX32_ONE * 16, 0 },
		{ FX32_ONE * 16 / 2, FX32_ONE * 16 / 2, 0 },
	};

	BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
	target.x += FX32_ONE * 4;
	target.y += FX32_ONE * 1;
	target.z += FX32_ONE * 16;
	pos.x += FX32_ONE * 4;
	pos.y += FX32_ONE * 1;
	pos.z += FX32_ONE * 16;
	BTLV_CAMERA_MoveCameraInterpolation( BTLV_EFFECT_GetCameraWork(), &pos, &target, 32, 0, 32 );

	for( mcss_pos = BTLV_MCSS_POS_AA ; mcss_pos < BTLV_MCSS_POS_MAX ; mcss_pos++ ){
		if( BTLV_MCSS_CheckExistPokemon( BTLV_EFFECT_GetMcssWork(), mcss_pos ) == TRUE ){
			BTLV_MCSS_MoveScale( BTLV_EFFECT_GetMcssWork(),
								 mcss_pos,
								 EFFTOOL_CALCTYPE_INTERPOLATION,
								 &scale_value[ mcss_pos & 1 ],
								 16, 1, 0 );
		}
	}
}
 
#if 0
static	void	set_pokemon( POKEMON_VIEWER_WORK *pvw )
{
	//POKEMON_PARAM生成
	POKEMON_PARAM	*pp = GFL_HEAP_AllocMemory( pvw->heapID, POKETOOL_GetWorkSize() );
	PP_SetupEx( pp, 0, 0, 0, 0, 255 );
	
	PP_Put( pp, ID_PARA_monsno, MONSNO_RIZAADON );
	PP_Put( pp, ID_PARA_id_no, 0x10 );
	BTLV_EFFECT_SetPokemon( pp, BTLV_MCSS_POS_AA );
	BTLV_EFFECT_SetPokemon( pp, BTLV_MCSS_POS_BB );

	GFL_HEAP_FreeMemory( pp );
}

static	void	del_pokemon( POKEMON_VIEWER_WORK *pvw )
{
	BTLV_MCSS_Del( BTLV_EFFECT_GetMcssWork(), BTLV_MCSS_POS_AA );
	BTLV_MCSS_Del( BTLV_EFFECT_GetMcssWork(), BTLV_MCSS_POS_BB );
}
#endif
