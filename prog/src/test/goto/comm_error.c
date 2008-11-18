#include <gflib.h>
#include <textprint.h>

#include "system/main.h"
#include "comm_error.h"

// -----------------------------------------
//
//	Push するﾌﾚｰﾑのパラメータ
//
// -----------------------------------------
typedef struct
{
	u8		pri;			///< bg pri
	u8		visible;		///< いらね
	u16		dummy;
		
} FRAME_PARAM;


// -----------------------------------------
//
//	Push して、再編するﾌﾚｰﾑのパラメータ
//
// -----------------------------------------
typedef struct 
{	
	int		scroll_x;
	int		scroll_y;
	fx32	scale_x;
	fx32	scale_y;
	int		cx;
	int		cy;
	u16		rad;
	u16		padding;
	
} TARGET_FRAME_PARAM;


// -----------------------------------------
//
//	Pusu Pop 通信エラー画面構築用ワーク
//
// -----------------------------------------
typedef struct
{
	u32		seq;			///< seq
	u32		heapID;			///< heap ID
	BOOL	active;			///< ﾌﾚｰﾑ指定とか終わった？ﾌﾗｸﾞ
	u16		push_frame;		///< push frame

	u32		scrbuf_size;	///< scr size
	void*	scrbuf_adrs;	///< scr adrs
	u16*	scrbuf_temp;	///< scr temp

	u32		chrbuf_size;	///< chr size
	void*	chrbuf_adrs;	///< chr adrs
	u32*	chrbuf_temp;	///< chr temp

	u32		palbuf_size;	///< pal size
	u16*	palbuf_adrs;	///< pal adrs
	u16*	palbuf_temp;	///< pal temp

	///< 文字描画
	GFL_BMPWIN*				bmpwin;			///< BMPWIN
	GFL_TEXT_PRINTPARAM		textParam;		///< テキスト表示用
	
	///< BG
	int						main_visible;	///< main の visibleパラメータ
	int						sub_visible;	///< sub　の visibleパラメータ
	FRAME_PARAM				frame_param[ GFL_BG_FRAME_MAX ];	///< mian sub の全ﾌﾚｰﾑPushワーク
	TARGET_FRAME_PARAM		target_frame_param;					///< 再構築用のPushワーク

} COMM_ERROR_SYS_WORK;

static COMM_ERROR_SYS_WORK g_pCommErrorSys = { 0 };
static const char error_text[] = { "つうしんエラー　WBのテストがめん\n\nなにか　おすと　もどります" };	
static const GFL_TEXT_PRINTPARAM default_param = { NULL, 0, 0, 1, 1, 1, 0, GFL_TEXT_WRITE_16 };

//==============================================================
// Prototype
//==============================================================
static u16* CommErrorSys_GetBgPaletteAdrs( void );
extern void CommErrorSys_Setup( void );
extern void CommErrorSys_Init( int push_frame );
extern void CommErrorSys_Call( void );
extern BOOL CommErrorSys_IsActive( void );
extern void CommErrorSys_PushFrameParam( void );
extern void CommErrorSys_PopFrameParam( void );
extern void CommErrorSys_PushFrameVram( void );
extern void CommErrorSys_PopFrameVram( void );
extern void CommErrorSys_Draw( void );
static u16* CommErrorSys_GetBgPaletteAdrs( void );


//--------------------------------------------------------------
/**
 * @brief	１度だけ呼ばれる
 *
 * @param	none	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CommErrorSys_Setup( void )
{
	g_pCommErrorSys.push_frame = COMM_ERROR_IRREAGULARITY;
	g_pCommErrorSys.active	   = FALSE;
	g_pCommErrorSys.seq		   = 0;
}


//--------------------------------------------------------------
/**
 * @brief	初期化
 *
 * @param	push_frame		再編するﾌﾚｰﾑ
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CommErrorSys_Init( int push_frame )
{
	g_pCommErrorSys.heapID	   = GFL_HEAPID_APP;
	g_pCommErrorSys.push_frame = push_frame;
	g_pCommErrorSys.seq		   = 0;
}


//--------------------------------------------------------------
/**
 * @brief	エラーの呼び出し
 *
 * @param	none	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CommErrorSys_Call( void )
{
	GF_ASSERT_MSG( g_pCommErrorSys.push_frame != COMM_ERROR_IRREAGULARITY, "!! フレーム未指定" );
	g_pCommErrorSys.active = TRUE;
	g_pCommErrorSys.seq	   = 0;	
}


//--------------------------------------------------------------
/**
 * @brief	エラーが呼ばれたかチェック
 *
 * @param	none	
 *
 * @retval	BOOL	
 *
 */
//--------------------------------------------------------------
BOOL CommErrorSys_IsActive( void )
{
	return g_pCommErrorSys.active;
}


//--------------------------------------------------------------
/**
 * @brief	ﾌﾚｰﾑの状態をPush
 *
 * @param	none	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CommErrorSys_PushFrameParam( void )
{
	int i;
	int	frame = g_pCommErrorSys.push_frame;
	
	g_pCommErrorSys.main_visible	= GFL_DISP_GetMainVisible();
	g_pCommErrorSys.sub_visible		= GFL_DISP_GetSubVisible();

	for ( i = 0; i < GFL_BG_FRAME_MAX; i++ )
	{
		g_pCommErrorSys.frame_param[ i ].pri = GFL_BG_GetPriority( i );
	}
	g_pCommErrorSys.target_frame_param.scroll_x	= GFL_BG_GetScrollX( frame );
	g_pCommErrorSys.target_frame_param.scroll_y	= GFL_BG_GetScrollY( frame );
	g_pCommErrorSys.target_frame_param.scale_x	= GFL_BG_GetScaleX( frame );
	g_pCommErrorSys.target_frame_param.scale_y	= GFL_BG_GetScaleY( frame );
	g_pCommErrorSys.target_frame_param.cx		= GFL_BG_GetCenterX( frame );
	g_pCommErrorSys.target_frame_param.cy		= GFL_BG_GetCenterX( frame );
	g_pCommErrorSys.target_frame_param.rad		= GFL_BG_GetRadian( frame );
	
	GFL_DISP_GX_SetVisibleControl( g_pCommErrorSys.push_frame, VISIBLE_OFF );
}


//--------------------------------------------------------------
/**
 * @brief	ﾌﾚｰﾑの状態をPop
 *
 * @param	none	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CommErrorSys_PopFrameParam( void )
{
	int i;
	int frame = g_pCommErrorSys.push_frame;
	
	for ( i = 0; i < GFL_BG_FRAME_MAX; i++ )
	{
		GFL_BG_SetPriority( frame, g_pCommErrorSys.frame_param[ i ].pri );
	}	
	GFL_BG_SetScroll( frame, GFL_BG_SCROLL_X_SET, g_pCommErrorSys.target_frame_param.scroll_x );
	GFL_BG_SetScroll( frame, GFL_BG_SCROLL_Y_SET, g_pCommErrorSys.target_frame_param.scroll_y );
#if 0
	g_pCommErrorSys.target_frame_param.scale_x	= GFL_BG_GetScaleX( frame );
	g_pCommErrorSys.target_frame_param.scale_y	= GFL_BG_GetScaleY( frame );
	g_pCommErrorSys.target_frame_param.cx		= GFL_BG_GetCenterX( frame );
	g_pCommErrorSys.target_frame_param.cy		= GFL_BG_GetCenterX( frame );
	g_pCommErrorSys.target_frame_param.rad		= GFL_BG_GetRadian( frame );	
#endif
	
	GFL_DISP_GX_SetVisibleControlDirect( g_pCommErrorSys.main_visible );
	GFL_DISP_GXS_SetVisibleControlDirect( g_pCommErrorSys.sub_visible );
}


//--------------------------------------------------------------
/**
 * @brief	VramをPush
 *
 * @param	none	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CommErrorSys_PushFrameVram( void )
{
	g_pCommErrorSys.scrbuf_size	= GFL_BG_GetScreenBufferSize( g_pCommErrorSys.push_frame );
	g_pCommErrorSys.scrbuf_adrs	= GFL_BG_GetScreenBufferAdrs( g_pCommErrorSys.push_frame );
	g_pCommErrorSys.scrbuf_temp	= GFL_HEAP_AllocClearMemory( g_pCommErrorSys.heapID, g_pCommErrorSys.scrbuf_size );
	MI_CpuCopy16( g_pCommErrorSys.scrbuf_adrs, g_pCommErrorSys.scrbuf_temp, g_pCommErrorSys.scrbuf_size );

	g_pCommErrorSys.chrbuf_size	= GFL_BG_GetBaseCharacterSize( g_pCommErrorSys.push_frame ) * 1024;
	g_pCommErrorSys.chrbuf_adrs	= GFL_BG_GetCharacterAdrs( g_pCommErrorSys.push_frame );
	g_pCommErrorSys.chrbuf_temp	= GFL_HEAP_AllocClearMemory( g_pCommErrorSys.heapID, g_pCommErrorSys.chrbuf_size );
	MI_CpuCopy32( g_pCommErrorSys.chrbuf_adrs, g_pCommErrorSys.chrbuf_temp, g_pCommErrorSys.chrbuf_size );

	g_pCommErrorSys.palbuf_size	= 0x20;
	g_pCommErrorSys.palbuf_adrs	= ( u16* )CommErrorSys_GetBgPaletteAdrs();
	g_pCommErrorSys.palbuf_temp	= GFL_HEAP_AllocClearMemory( g_pCommErrorSys.heapID, g_pCommErrorSys.palbuf_size );
	MI_CpuCopy16( g_pCommErrorSys.palbuf_adrs, g_pCommErrorSys.palbuf_temp, g_pCommErrorSys.palbuf_size );	
	
	CommErrorSys_PushFrameParam();

	OS_TPrintf( "size = %d\n", g_pCommErrorSys.scrbuf_size );
	OS_TPrintf( "size = %d\n", g_pCommErrorSys.chrbuf_size );
	OS_TPrintf( "size = %d\n", g_pCommErrorSys.palbuf_size );
}


//--------------------------------------------------------------
/**
 * @brief	VramをPos
 *
 * @param	none	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CommErrorSys_PopFrameVram( void )
{
	MI_CpuCopy16( g_pCommErrorSys.scrbuf_temp, g_pCommErrorSys.scrbuf_adrs, g_pCommErrorSys.scrbuf_size );
	MI_CpuCopy32( g_pCommErrorSys.chrbuf_temp, g_pCommErrorSys.chrbuf_adrs, g_pCommErrorSys.chrbuf_size );
	MI_CpuCopy16( g_pCommErrorSys.palbuf_temp, g_pCommErrorSys.palbuf_adrs, g_pCommErrorSys.palbuf_size );
	GFL_BG_LoadScreenReq( g_pCommErrorSys.push_frame );

	GFL_HEAP_FreeMemory( g_pCommErrorSys.scrbuf_temp ); 
	GFL_HEAP_FreeMemory( g_pCommErrorSys.chrbuf_temp ); 
	GFL_HEAP_FreeMemory( g_pCommErrorSys.palbuf_temp );
	g_pCommErrorSys.scrbuf_temp = NULL;
	g_pCommErrorSys.chrbuf_temp = NULL;
	g_pCommErrorSys.palbuf_temp = NULL;	
	
	CommErrorSys_PopFrameParam();
	
//	g_pCommErrorSys.active = FALSE;
//	g_pCommErrorSys.push_frame = COMM_ERROR_IRREAGULARITY;
}


//--------------------------------------------------------------
/**
 * @brief	警告画面を描画
 *
 * @param	none	
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void CommErrorSys_Draw( void )
{
	///< 適当に画面を作る	
	{
		u16* plt = GFL_HEAP_AllocClearMemoryLo( g_pCommErrorSys.heapID, 0x20 );
		plt[0] = 0x0000;
		plt[1] = 0x001F;
		GFL_BG_LoadPalette( g_pCommErrorSys.push_frame, plt, 0x20, 0x20 * COMM_ERROR_PALETTE_POS );	
		GFL_HEAP_FreeMemory( plt );
	}
	{
		GFL_BG_SetPriority( g_pCommErrorSys.push_frame, 0 );
		GFL_BG_SetScroll( g_pCommErrorSys.push_frame, GFL_BG_SCROLL_X_SET, 0 );
		GFL_BG_SetScroll( g_pCommErrorSys.push_frame, GFL_BG_SCROLL_Y_SET, 0 );
	}

	GFL_BG_ClearScreen( g_pCommErrorSys.push_frame );
	g_pCommErrorSys.bmpwin = GFL_BMPWIN_Create( g_pCommErrorSys.push_frame,
												1, 1, 30, 30, COMM_ERROR_PALETTE_POS, GFL_BG_CHRAREA_GET_F );

	g_pCommErrorSys.textParam			= default_param;
	g_pCommErrorSys.textParam.bmp		= GFL_BMPWIN_GetBmp( g_pCommErrorSys.bmpwin );
	g_pCommErrorSys.textParam.writex	= 0;
	g_pCommErrorSys.textParam.writey	= 0;
	GFL_TEXT_PrintSjisCode( error_text, &g_pCommErrorSys.textParam );
	
	GFL_BMPWIN_TransVramCharacter( g_pCommErrorSys.bmpwin );	
	GFL_BMPWIN_MakeScreen( g_pCommErrorSys.bmpwin );
	GFL_BG_LoadScreenReq( g_pCommErrorSys.push_frame );
	
	GFL_BMPWIN_Delete( g_pCommErrorSys.bmpwin );

	GFL_DISP_GX_SetVisibleControl( g_pCommErrorSys.push_frame, VISIBLE_ON );
}


//--------------------------------------------------------------
/**
 * @brief	CommErrorSys で表示させるBGのPaletteVramAdrs
 *
 * @param	none	
 *
 * @retval	static u16*	
 *
 */
//--------------------------------------------------------------
static u16* CommErrorSys_GetBgPaletteAdrs( void )
{
	u16* adrs = ( u16* )COMM_ERROR_BG_PLTT_ADRS_MAIN;
	
	switch ( g_pCommErrorSys.push_frame ){
	case GFL_BG_FRAME0_M:
	case GFL_BG_FRAME1_M:
	case GFL_BG_FRAME2_M:
	case GFL_BG_FRAME3_M:
		adrs = ( u16* )COMM_ERROR_BG_PLTT_ADRS_MAIN;
		break;
	case GFL_BG_FRAME0_S:
	case GFL_BG_FRAME1_S:
	case GFL_BG_FRAME2_S:
	case GFL_BG_FRAME3_S:
		adrs = ( u16* )COMM_ERROR_BG_PLTT_ADRS_SUB;
		break;
	default:
		GF_ASSERT( 0 );
		break;
	}
	
	return adrs;
}


//--------------------------------------------------------------
/**
 * @brief	Sample用
 *
 * @param	none	
 *
 * @retval	void	
 *
 */
//--------------------------------------------------------------
void CommErrorSys_SampleView( void )
{
	if ( CommErrorSys_IsActive() )
	{
		CommErrorSys_PushFrameVram();
		CommErrorSys_Draw();
				
		do {
			GFL_UI_Main();
		}
		while( !GFL_UI_KEY_GetTrg() );
		
		CommErrorSys_PopFrameVram();
	}	
}
