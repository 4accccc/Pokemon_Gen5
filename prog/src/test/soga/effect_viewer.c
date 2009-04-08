//============================================================================================
/**
 * @file	effect_viewer.c
 * @brief	MCSを利用してエフェクトを見るビューワー
 * @author	soga
 * @date	2009.03.02
 */
//============================================================================================

#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include <textprint.h>

#include "system/main.h"
#include "print/printsys.h"
#include "arc_def.h"

#include "battle/btlv/btlv_effect.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#include "gf_mcs.h"

#include "font/font.naix"
#include "message.naix"
#include "msg/msg_d_soga.h"

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

enum{
	SEND_SEQUENCE = 1,
	SEND_SEQUENCE_OK,
	SEND_RESOURCE,
	SEND_RESOURCE_OK,
	SEND_RECEIVE,
	SEND_RECEIVE_OK,
	SEND_IDLE,
};

enum{
	SEQ_IDLE = 0,
	SEQ_LOAD_SEQUENCE_DATA,
	SEQ_LOAD_RESOURCE_DATA,
	SEQ_EFFECT_ENABLE,
	SEQ_EFFECT_WAIT,
	SEQ_RECEIVE_ACTION,
};

enum{
	SUB_SEQ_INIT = 0,
	SUB_SEQ_PARTICLE_PLAY_PARAM,
	SUB_SEQ_PARTICLE_PLAY_PARAM_SEND,
};

typedef struct
{
	HEAPID				heapID;

	int					seq_no;
	int					sub_seq_no;
	int					mcs_enable;
	POKEMON_PARAM		*pp;
	int					mons_no;
	GFL_BMPWIN			*bmpwin;
	GFL_TEXT_PRINTPARAM	*textParam;
	int					key_repeat_speed;
	int					key_repeat_wait;

	GFL_MSGDATA			*msg;
	GFL_FONT			*font;
	STRBUF				*strbuf;

	void				*sequence_data;
	void				*resource_data;
}EFFECT_VIEWER_WORK;

static	void	EffectViewerSequence( EFFECT_VIEWER_WORK *evw );
static	void	EffectViewerRead( EFFECT_VIEWER_WORK *evw );
static	void	EffectViewerSequenceLoad( EFFECT_VIEWER_WORK *evw );
static	void	EffectViewerResourceLoad( EFFECT_VIEWER_WORK *evw );
static	void	EffectViewerRecieveAction( EFFECT_VIEWER_WORK *evw );

static	void	TextPrint(EFFECT_VIEWER_WORK *evw, int num, int bmpwin_num );
static	void	NumPrint( EFFECT_VIEWER_WORK *evw, int num, int bmpwin_num );
static	void	Num16Print( EFFECT_VIEWER_WORK *evw, int num, int bmpwin_num );

static	void	MoveCamera( EFFECT_VIEWER_WORK *evw );

static	void	set_pokemon( EFFECT_VIEWER_WORK *evw );
static	void	del_pokemon( EFFECT_VIEWER_WORK *evw );

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

static	const	char	ProjectionText[2][12]={
	"Perspective",
	"Ortho",
};

FS_EXTERN_OVERLAY(battle);

//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT EffectViewerProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	EFFECT_VIEWER_WORK *evw;

	GFL_OVERLAY_Load(FS_OVERLAY_ID(battle));

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SOGABE_DEBUG, 0xc0000 );
	evw = GFL_PROC_AllocWork( proc, sizeof( EFFECT_VIEWER_WORK ), HEAPID_SOGABE_DEBUG );
	MI_CpuClearFast( evw, sizeof( EFFECT_VIEWER_WORK ) );
	evw->heapID = HEAPID_SOGABE_DEBUG;
		
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
	GFL_BG_Init( evw->heapID );
	GFL_BMPWIN_Init( evw->heapID );

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
		GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_OFF );
		
	}		
	GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

	//3D関連初期化
	{
		GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K, GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0, evw->heapID, NULL );
		GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
		G3X_AlphaBlend( TRUE );
		G3X_EdgeMarking( TRUE );
		G3X_AntiAlias( TRUE );
		GFL_BG_SetBGControl3D( 1 );
	}

	//戦闘エフェクト初期化
	{
		BTLV_EFFECT_Init( 0, evw->heapID );
	}

	set_pokemon( evw );

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
			///<FRAME0_S
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
		GFL_BG_SetBGControl(GFL_BG_FRAME0_S, &TextBgCntDat[3], GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen(GFL_BG_FRAME0_S );
	}

	{
		static const GFL_TEXT_PRINTPARAM default_param = { NULL, 0, 0, 1, 1, 1, 0, GFL_TEXT_WRITE_16 };
		int	i;

		evw->textParam = GFL_HEAP_AllocMemory( evw->heapID, sizeof( GFL_TEXT_PRINTPARAM ) );
		*evw->textParam = default_param;


		//フォントパレット作成＆転送
		{
			static	u16 plt[16] = {
				G2D_BACKGROUND_COL, G2D_FONT_COL,
				0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0,
				0, 0 };
			GFL_BG_LoadPalette( GFL_BG_FRAME0_S, &plt, 16*2, 0 );
		}

		evw->bmpwin = GFL_BMPWIN_Create( GFL_BG_FRAME0_S, 0, 0, 32, 24, 0, GFL_BG_CHRAREA_GET_F );

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

	GFL_UI_KEY_GetRepeatSpeed( &evw->key_repeat_speed, &evw->key_repeat_wait );
	GFL_UI_KEY_SetRepeatSpeed( evw->key_repeat_speed / 4, evw->key_repeat_wait );

	//メッセージ系初期化
	GFL_FONTSYS_Init();
	evw->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_soga_dat, evw->heapID );
	evw->font = GFL_FONT_Create( ARCID_FONT, NARC_font_small_nftr, GFL_FONT_LOADTYPE_FILE, FALSE, evw->heapID );

	evw->strbuf = GFL_MSG_CreateString( evw->msg,  EVMSG_ATTACK );
	PRINTSYS_Print( GFL_BMPWIN_GetBmp( evw->bmpwin ), 0, 0, evw->strbuf, evw->font );
	GFL_BMPWIN_TransVramCharacter( evw->bmpwin );
	GFL_BMPWIN_MakeScreen( evw->bmpwin );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	GFL_HEAP_FreeMemory( evw->strbuf );

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT EffectViewerProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	int pad = GFL_UI_KEY_GetCont();
	int trg = GFL_UI_KEY_GetTrg();
	int rep = GFL_UI_KEY_GetRepeat();
	int tp = GFL_UI_TP_GetTrg();
	EFFECT_VIEWER_WORK *evw = mywk;

	if( evw->mcs_enable ){
		MCS_Main();
//		MCS_Write();
	}

	if( trg & PAD_BUTTON_START ){
		if( evw->mcs_enable ){
			evw->seq_no = SEQ_IDLE;
			MCS_Exit();
			evw->mcs_enable = 0;
			if( evw->sequence_data ){
				GFL_HEAP_FreeMemory( evw->sequence_data );
			}
			if( evw->resource_data ){
				GFL_HEAP_FreeMemory( evw->resource_data );
			}
			evw->sequence_data = NULL;
			evw->resource_data = NULL;
		}
		else{
			if( MCS_Init( evw->heapID ) == FALSE ){
				evw->mcs_enable = 1;
			}
		}
	}

	EffectViewerSequence( evw );

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
static GFL_PROC_RESULT EffectViewerProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	EFFECT_VIEWER_WORK *evw = mywk;

	if( GFL_FADE_CheckFade() == TRUE ){
		return GFL_PROC_RES_CONTINUE;	
	}

	GFL_UI_KEY_SetRepeatSpeed( evw->key_repeat_speed, evw->key_repeat_wait );

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
const GFL_PROC_DATA		EffectViewerProcData = {
	EffectViewerProcInit,
	EffectViewerProcMain,
	EffectViewerProcExit,
};

//======================================================================
//	メインシーケンス
//======================================================================
static	void	EffectViewerSequence( EFFECT_VIEWER_WORK *evw )
{
	int tp = GFL_UI_TP_GetTrg();
	int cont = GFL_UI_KEY_GetCont();

	switch( evw->seq_no ){
	default:
	case SEQ_IDLE:
		if( cont ){
			evw->seq_no = SEQ_EFFECT_ENABLE;
		}
		EffectViewerRead( evw );
		break;
	case SEQ_LOAD_SEQUENCE_DATA:
		EffectViewerSequenceLoad( evw );
		break;
	case SEQ_LOAD_RESOURCE_DATA:
		EffectViewerResourceLoad( evw );
		break;
	case SEQ_EFFECT_ENABLE:
		if( cont == PAD_BUTTON_A ){
			BTLV_EFFVM_StartDebug( BTLV_EFFECT_GetVMHandle(), BTLV_MCSS_POS_BB, BTLV_MCSS_POS_AA, evw->sequence_data, evw->resource_data );
		}
		else if( cont == PAD_BUTTON_B ){
			BTLV_EFFVM_StartDebug( BTLV_EFFECT_GetVMHandle(), BTLV_MCSS_POS_AA, BTLV_MCSS_POS_BB, evw->sequence_data, evw->resource_data );
		}
		evw->seq_no++;
		break;
	case SEQ_EFFECT_WAIT:
		if( BTLV_EFFECT_CheckExecute() == FALSE ){
			evw->seq_no = SEQ_IDLE;
		}
		break;
	case SEQ_RECEIVE_ACTION:
		EffectViewerRecieveAction( evw );
		break;
	}
}

//======================================================================
//	MCSからの読み込みチェック
//======================================================================
static	void	EffectViewerRead( EFFECT_VIEWER_WORK *evw )
{
	u32	size;
	u32	head;

	if( evw->mcs_enable == 0 ){
		return;
	}

	size = MCS_CheckRead();

	if( size ){
		MCS_Read( &head, HEAD_SIZE );
		if( head == SEND_SEQUENCE ){
			evw->seq_no = SEQ_LOAD_SEQUENCE_DATA;
			head = SEND_SEQUENCE_OK;
			MCS_Write( MCS_WRITE_CH, &head, 4 );
			OS_TPrintf("sequence_data head:%d\n",size);
		}
		else if( head == SEND_RESOURCE ){
			evw->seq_no = SEQ_LOAD_RESOURCE_DATA;
			head = SEND_RESOURCE_OK;
			MCS_Write( MCS_WRITE_CH, &head, 4 );
			OS_TPrintf("resource_data head:%d\n",size);
		}
		else if( head == SEND_RECEIVE ){
			evw->seq_no = SEQ_RECEIVE_ACTION;
			evw->sub_seq_no = SUB_SEQ_INIT;
			head = SEND_RECEIVE_OK;
			MCS_Write( MCS_WRITE_CH, &head, 4 );
			OS_TPrintf("receive head:%d\n",size);
		}
	}
}

//======================================================================
//	MCSからのシーケンス読み込みチェック
//======================================================================
static	void	EffectViewerSequenceLoad( EFFECT_VIEWER_WORK *evw )
{
	u32	size;
	u32	head;

	size = MCS_CheckRead();

	if( size ){
		if( evw->sequence_data ){
			GFL_HEAP_FreeMemory( evw->sequence_data );
		}
		evw->sequence_data = GFL_HEAP_AllocClearMemory( evw->heapID, size );
		MCS_Read( evw->sequence_data, size );
		head = SEND_IDLE;
		MCS_Write( MCS_WRITE_CH, &head, 4 );
		evw->seq_no = SEQ_IDLE;
		OS_TPrintf("sequence_data load:%d\n",size);
	}
}

//======================================================================
//	MCSからのシーケンス読み込みチェック
//======================================================================
static	void	EffectViewerResourceLoad( EFFECT_VIEWER_WORK *evw )
{
	u32	size;
	u32	head;

	size = MCS_CheckRead();

	if( size ){
		if( evw->resource_data ){
			GFL_HEAP_FreeMemory( evw->resource_data );
		}
		evw->resource_data = GFL_HEAP_AllocClearMemory( evw->heapID, size );
		MCS_Read( evw->resource_data, size );
		head = SEND_IDLE;
		MCS_Write( MCS_WRITE_CH, &head, 4 );
		evw->seq_no = SEQ_IDLE;
		OS_TPrintf("resource_data load:%d\n",size);
	}
}

//======================================================================
//	DSからPCへデータを受け渡す
//======================================================================
static	void	EffectViewerRecieveAction( EFFECT_VIEWER_WORK *evw )
{
	switch( evw->sub_seq_no ){
	case SUB_SEQ_INIT:
		{
			int	*start_ofs = (int *)evw->sequence_data ;
			u8 *start = (u8 *)evw->sequence_data;
			u16 *com_start = (u16 *)&start[ start_ofs[ 0 ] ];

			switch( com_start[ 0 ] ){
			case EC_CAMERA_MOVE:
			case EC_PARTICLE_LOAD:
				break;
			case EC_PARTICLE_PLAY:
				evw->sub_seq_no = SUB_SEQ_PARTICLE_PLAY_PARAM;
				break;
			case EC_POKEMON_MOVE:
			case EC_POKEMON_SCALE:
			case EC_POKEMON_ROTATE:
			case EC_POKEMON_SET_MEPACHI_FLAG:
			case EC_POKEMON_SET_ANM_FLAG:
			case EC_EFFECT_END_WAIT:
				break;
			}
		}
		break;
	case SUB_SEQ_PARTICLE_PLAY_PARAM:
		break;
	case SUB_SEQ_PARTICLE_PLAY_PARAM_SEND:
		break;
	}
}

//======================================================================
//	テキスト表示
//======================================================================
static	void	TextPrint( EFFECT_VIEWER_WORK *evw, int num, int bmpwin_num )
{
#if 0
	evw->textParam->writex = 0;
	evw->textParam->writey = 0;
	evw->textParam->bmp = GFL_BMPWIN_GetBmp( evw->bmpwin[ bmpwin_num ] );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( evw->bmpwin[ bmpwin_num ] ), 0 );
	GFL_TEXT_PrintSjisCode( &ProjectionText[ num ][ 0 ], evw->textParam );
	GFL_BMPWIN_TransVramCharacter( evw->bmpwin[ bmpwin_num ] );
	GFL_BMPWIN_MakeScreen( evw->bmpwin[ bmpwin_num ] );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
#endif
}

//======================================================================
//	数字表示
//======================================================================
static	void	NumPrint( EFFECT_VIEWER_WORK *evw, int num, int bmpwin_num )
{
#if 0
	char	num_char[ 4 ] = "\0\0\0\0";
	int		num_100,num_10,num_1;

	num_100 = num / 100;
	num_10  = ( num - ( num_100 * 100 ) ) / 10;
	num_1   = num - ( num_100 * 100 ) - (num_10 * 10 );

	num_char[ 0 ] = num_char_table[ num_100 ][ 0 ];
	num_char[ 1 ] = num_char_table[ num_10  ][ 0 ];
	num_char[ 2 ] = num_char_table[ num_1   ][ 0 ];

	evw->textParam->writex = 0;
	evw->textParam->writey = 0;
	evw->textParam->bmp = GFL_BMPWIN_GetBmp( evw->bmpwin[ bmpwin_num ] );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( evw->bmpwin[ bmpwin_num ] ), 0 );
	GFL_TEXT_PrintSjisCode( &num_char[0], evw->textParam );
	GFL_BMPWIN_TransVramCharacter( evw->bmpwin[ bmpwin_num ] );
	GFL_BMPWIN_MakeScreen( evw->bmpwin[ bmpwin_num ] );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
#endif
}

static	void	Num16Print( EFFECT_VIEWER_WORK *evw, int num, int bmpwin_num )
{
#if 0
	int		i;
	char	num_char[ 9 ] = "\0\0\0\0\0\0\0\0\0";

	for( i = 0 ; i < 8 ; i++ ){
		num_char[ i ] = num_char_table[ ( num >> ( 28 - 4 * i ) ) & 0x0000000f ][ 0 ];
	}

	evw->textParam->writex = 0;
	evw->textParam->writey = 0;
	evw->textParam->bmp = GFL_BMPWIN_GetBmp( evw->bmpwin[ bmpwin_num ] );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( evw->bmpwin[ bmpwin_num ] ), 0 );
	GFL_TEXT_PrintSjisCode( &num_char[0], evw->textParam );
	GFL_BMPWIN_TransVramCharacter( evw->bmpwin[ bmpwin_num ] );
	GFL_BMPWIN_MakeScreen( evw->bmpwin[ bmpwin_num ] );
	GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
#endif
}

//======================================================================
//	カメラの制御
//======================================================================
static	void	MoveCamera( EFFECT_VIEWER_WORK *evw )
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
/*
		OS_TPrintf("pos_x:%08x pos_y:%08x pos_z:%08x tar_x:%08x tar_y:%08x tar_z:%08x\n",
			camPos.x,
			camPos.y,
			camPos.z,
			camTarget.x,
			camTarget.y,
			camTarget.z );
*/
	}

}
 
static	void	set_pokemon( EFFECT_VIEWER_WORK *evw )
{
	//POKEMON_PARAM生成
	POKEMON_PARAM	*pp = GFL_HEAP_AllocMemory( evw->heapID, POKETOOL_GetWorkSize() );
	PP_SetupEx( pp, 0, 0, 0, 0, 255 );
	
	PP_Put( pp, ID_PARA_monsno, MONSNO_RIZAADON );
	PP_Put( pp, ID_PARA_id_no, 0x10 );
	BTLV_EFFECT_SetPokemon( pp, BTLV_MCSS_POS_AA );
	BTLV_EFFECT_SetPokemon( pp, BTLV_MCSS_POS_BB );

	GFL_HEAP_FreeMemory( pp );
}

static	void	del_pokemon( EFFECT_VIEWER_WORK *evw )
{
	BTLV_MCSS_Del( BTLV_EFFECT_GetMcssWork(), BTLV_MCSS_POS_AA );
	BTLV_MCSS_Del( BTLV_EFFECT_GetMcssWork(), BTLV_MCSS_POS_BB );
}

