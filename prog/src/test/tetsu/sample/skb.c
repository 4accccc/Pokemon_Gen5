//============================================================================================
/**
 * @file	skb.c
 * @brief	ソフトウエアキーボード
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "textprint.h"

#include "skb.h"
//============================================================================================
/**
 *
 *
 *
 * @brief	ソフトウエアキーボードコントロール
 *
 *
 *
 */
//============================================================================================
typedef struct _GFL_SKB GFL_SKB;

typedef struct {
	void*			cgx;
	void*			scr;
	void*			plt;
	u16				control;
	int				offsH;
	int				offsV;
}VRAM_SV;

struct _GFL_SKB {
	HEAPID			heapID;
	u32				seq;
	GFL_SKB_SETUP	setup;
	u16				wait;

	VRAM_SV			vramSv;
};

#define PLT_SIZ			(0x20)
#define PUSH_CGX_SIZ	(0x3800)
#define PUSH_SCR_SIZ	(0x800)
#define PUSH_PLT_SIZ	(PLT_SIZ*2)

static void visibleOn( GFL_SKB_BGID bgID );
static void visibleOff( GFL_SKB_BGID bgID );
static void* getCgxPtr( GFL_SKB_BGID bgID );
static void loadCgx( GFL_SKB_BGID bgID, void* src );
static void* getScrPtr( GFL_SKB_BGID bgID );
static void loadScr( GFL_SKB_BGID bgID, void* src );
static void* getPltPtr( GFL_SKB_BGID bgID );
static void loadPlt( GFL_SKB_BGID bgID, void* src, u32 offs );
static void getControl( GFL_SKB_BGID bgID, u16* control );
static void setControl( GFL_SKB_BGID bgID, u16* control );
static void setScroll( GFL_SKB_BGID bgID, int* x, int* y );
static void setAlpha( GFL_SKB_BGID bgID );

static void pushVram
		( GFL_SKB_BGID bgID, GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, VRAM_SV* vramSv );
static void popVram
		( GFL_SKB_BGID bgID, GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, VRAM_SV* vramSv );

static void makeDisplay
	( GFL_SKB_BGID bgID, GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, HEAPID heapID );
static void changeScrAttr
		( u16* scr, GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, u16 x, u16 y );

static BOOL	SKB_Control( GFL_SKB* gflSkb );
//============================================================================================
/**
 *
 * @brief	システム起動 & 終了
 *
 */
//============================================================================================
GFL_SKB*	GFL_SKB_Boot( HEAPID heapID, const GFL_SKB_SETUP* setup )
{
	GFL_SKB* gflSkb;

	gflSkb = GFL_HEAP_AllocClearMemory( GetHeapLowID(heapID), sizeof(GFL_SKB) );

	gflSkb->heapID = heapID;
	gflSkb->seq = 0;
	gflSkb->setup = *setup;
	gflSkb->wait = 0;
	gflSkb->vramSv.cgx = GFL_HEAP_AllocClearMemory( GetHeapLowID(heapID), PUSH_CGX_SIZ  );
	gflSkb->vramSv.scr = GFL_HEAP_AllocClearMemory( GetHeapLowID(heapID), PUSH_SCR_SIZ  );
	gflSkb->vramSv.plt = GFL_HEAP_AllocClearMemory( GetHeapLowID(heapID), PUSH_PLT_SIZ );

	return gflSkb;
}

void	GFL_SKB_Exit( GFL_SKB* gflSkb )
{
	if( gflSkb != NULL ){
		GFL_HEAP_FreeMemory( gflSkb->vramSv.plt );
		GFL_HEAP_FreeMemory( gflSkb->vramSv.scr );
		GFL_HEAP_FreeMemory( gflSkb->vramSv.cgx );
		GFL_HEAP_FreeMemory( gflSkb );
		gflSkb = NULL;
	}
}

//============================================================================================
/**
 *
 * @brief	システムメイン
 *
 */
//============================================================================================
enum {
	SEQ_PUSHVRAM = 0,
	SEQ_MAKEDISP,
	SEQ_DISPON,
	SEQ_MAIN,
	SEQ_POPVRAM,
	SEQ_END,
};

BOOL	GFL_SKB_Main( GFL_SKB* gflSkb )
{
	if( gflSkb == NULL ){ return FALSE; }

	switch( gflSkb->seq ){

	case SEQ_PUSHVRAM:
		visibleOff( gflSkb->setup.bgID );

		pushVram
		( gflSkb->setup.bgID, gflSkb->setup.bgPalID, gflSkb->setup.bgPalID_on, &gflSkb->vramSv );

		gflSkb->seq = SEQ_MAKEDISP;
		break;

	case SEQ_MAKEDISP:
		makeDisplay( gflSkb->setup.bgID, gflSkb->setup.bgPalID, 
						gflSkb->setup.bgPalID_on, gflSkb->heapID );
		gflSkb->seq = SEQ_DISPON;
		break;

	case SEQ_DISPON:
		visibleOn( gflSkb->setup.bgID );
		gflSkb->seq = SEQ_MAIN;
		break;

	case SEQ_MAIN:
		if( SKB_Control( gflSkb ) == FALSE ){
			gflSkb->seq = SEQ_POPVRAM;
		}
		break;

	case SEQ_POPVRAM:
		visibleOff( gflSkb->setup.bgID );

		popVram
		( gflSkb->setup.bgID, gflSkb->setup.bgPalID, gflSkb->setup.bgPalID_on, &gflSkb->vramSv );

		gflSkb->seq = SEQ_END;
		break;

	case SEQ_END:
		visibleOn( gflSkb->setup.bgID );

		GFL_SKB_Exit( gflSkb );
		return FALSE;
	}
	return TRUE;
}

//============================================================================================
static void pushVram
		( GFL_SKB_BGID bgID, GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, VRAM_SV* vramSv )
{
	void* src;

	src = getScrPtr( bgID );
	GFL_STD_MemCopy32( src, vramSv->scr, PUSH_SCR_SIZ );

	src = getCgxPtr( bgID );
	GFL_STD_MemCopy32( src, vramSv->cgx, PUSH_CGX_SIZ );

	src = getPltPtr( bgID );
	GFL_STD_MemCopy32
		( (void*)((u32)src+PLT_SIZ*palID), vramSv->plt, PLT_SIZ );
	GFL_STD_MemCopy32
		( (void*)((u32)src+PLT_SIZ*palID_on), (void*)((u32)vramSv->plt+PLT_SIZ), PLT_SIZ );

	getControl( bgID, &vramSv->control );
}

static void popVram
		( GFL_SKB_BGID bgID, GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, VRAM_SV* vramSv )
{
	setControl( bgID, &vramSv->control );
	loadScr( bgID, vramSv->scr );
	loadCgx( bgID, vramSv->cgx );
	loadPlt( bgID, vramSv->plt, PLT_SIZ*palID );
	loadPlt( bgID, (void*)((u32)vramSv->plt+PLT_SIZ), PLT_SIZ*palID_on );
}

//============================================================================================
static const char arc_path[] = {"skb.narc"};

static void makeDisplay
	( GFL_SKB_BGID bgID, GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, HEAPID heapID )
{
	NNSG2dScreenData*		scr;
	NNSG2dCharacterData*	cgx;
	NNSG2dPaletteData*		plt;
	int scroll_h = 0;
	int scroll_v = 0;

	void* binScr = GFL_ARC_LoadDataFilePathAlloc( arc_path, NARC_skb_skb_NSCR, heapID );
	void* binCgx = GFL_ARC_LoadDataFilePathAlloc( arc_path, NARC_skb_skb_NCGR, heapID );
	void* binPlt = GFL_ARC_LoadDataFilePathAlloc( arc_path, NARC_skb_skb_NCLR, heapID );

	NNS_G2dGetUnpackedScreenData( binScr, &scr );
	NNS_G2dGetUnpackedBGCharacterData( binCgx, &cgx );
	NNS_G2dGetUnpackedPaletteData( binPlt, &plt );

	changeScrAttr( (u16*)&scr->rawData[0], palID, GFL_SKB_PALID_NONE, 0, 0 );	//リセット

	loadScr( bgID, &scr->rawData[0] );
	loadCgx( bgID, cgx->pRawData );
	loadPlt( bgID, plt->pRawData, PLT_SIZ*palID );
	loadPlt( bgID, (void*)((u32)plt->pRawData + PLT_SIZ), PLT_SIZ*palID_on );
	setScroll( bgID, &scroll_h, &scroll_v );
	setAlpha( bgID );

	GFL_HEAP_FreeMemory( binPlt );
	GFL_HEAP_FreeMemory( binCgx );
	GFL_HEAP_FreeMemory( binScr );
}

//============================================================================================
static void changeScrAttr
		( u16* scr, GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, u16 x, u16 y )
{
	int i;
	u16 palMask = palID <<12;
	u16 palMaskOn = palID_on <<12;

	for( i=0; i<32*32; i++ ){
		scr[i] &= 0x0fff;
		scr[i] |= palMask;
	}
	if( palID_on != GFL_SKB_PALID_NONE ){
		scr[ y*32 + x ] &= 0x0fff; 
		scr[ y*32 + x ] |= palMaskOn;
	}
}

//============================================================================================
/**
 *
 * @brief	
 *
 */
//============================================================================================
//============================================================================================
/**
 *
 * @brief	
 *
 */
//============================================================================================
static BOOL SKB_Control( GFL_SKB* gflSkb )
{
	if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_START ){
		return FALSE;
	}
	return TRUE;
}

//============================================================================================
/**
 *
 * @brief	
 *
 */
//============================================================================================
static const int visibleMaskTbl[] = {
	GX_PLANEMASK_BG0, GX_PLANEMASK_BG1, GX_PLANEMASK_BG2, GX_PLANEMASK_BG3,
	GX_PLANEMASK_BG0, GX_PLANEMASK_BG1, GX_PLANEMASK_BG2, GX_PLANEMASK_BG3,
};

static void visibleOn( GFL_SKB_BGID bgID )
{
	int mask = visibleMaskTbl[bgID];
	int visiblePlane;

	switch( bgID ){
	case GFL_SKB_BGID_M0:
	case GFL_SKB_BGID_M1:
	case GFL_SKB_BGID_M2:
	case GFL_SKB_BGID_M3:
		visiblePlane = GX_GetVisiblePlane(); 
		visiblePlane |= mask;
		GX_SetVisiblePlane( visiblePlane ); 
		break;
	case GFL_SKB_BGID_S0:
	case GFL_SKB_BGID_S1:
	case GFL_SKB_BGID_S2:
	case GFL_SKB_BGID_S3:
		visiblePlane = GXS_GetVisiblePlane(); 
		visiblePlane |= mask;
		GXS_SetVisiblePlane( visiblePlane ); 
		break;
	}
}

static void visibleOff( GFL_SKB_BGID bgID )
{
	int mask = visibleMaskTbl[bgID];
	int visiblePlane;

	switch( bgID ){
	case GFL_SKB_BGID_M0:
	case GFL_SKB_BGID_M1:
	case GFL_SKB_BGID_M2:
	case GFL_SKB_BGID_M3:
		visiblePlane = GX_GetVisiblePlane(); 
		visiblePlane &= (mask^0xffffffff);
		GX_SetVisiblePlane( visiblePlane ); 
		break;
	case GFL_SKB_BGID_S0:
	case GFL_SKB_BGID_S1:
	case GFL_SKB_BGID_S2:
	case GFL_SKB_BGID_S3:
		visiblePlane = GXS_GetVisiblePlane(); 
		visiblePlane &= (mask^0xffffffff);
		GXS_SetVisiblePlane( visiblePlane ); 
		break;
	}
}

//============================================================================================
static void* getCgxPtr( GFL_SKB_BGID bgID )
{
	switch( bgID ){
	case GFL_SKB_BGID_M0:
		return G2_GetBG0CharPtr();
	case GFL_SKB_BGID_M1:
		return G2_GetBG1CharPtr();
	case GFL_SKB_BGID_M2:
		return G2_GetBG2CharPtr();
	case GFL_SKB_BGID_M3:
		return G2_GetBG3CharPtr();
	case GFL_SKB_BGID_S0:
		return G2S_GetBG0CharPtr();
	case GFL_SKB_BGID_S1:
		return G2S_GetBG1CharPtr();
	case GFL_SKB_BGID_S2:
		return G2S_GetBG2CharPtr();
	case GFL_SKB_BGID_S3:
		return G2S_GetBG3CharPtr();
	}
	GF_ASSERT(0);
	return NULL;
}

static void loadCgx( GFL_SKB_BGID bgID, void* src )
{
	DC_FlushRange( src, PUSH_CGX_SIZ );

	switch( bgID ){
	case GFL_SKB_BGID_M0:
		GX_LoadBG0Char( src, 0, PUSH_CGX_SIZ );
		break;
	case GFL_SKB_BGID_M1:
		GX_LoadBG1Char( src, 0, PUSH_CGX_SIZ );
		break;
	case GFL_SKB_BGID_M2:
		GX_LoadBG2Char( src, 0, PUSH_CGX_SIZ );
		break;
	case GFL_SKB_BGID_M3:
		GX_LoadBG3Char( src, 0, PUSH_CGX_SIZ );
		break;
	case GFL_SKB_BGID_S0:
		GXS_LoadBG0Char( src, 0, PUSH_CGX_SIZ );
		break;
	case GFL_SKB_BGID_S1:
		GXS_LoadBG1Char( src, 0, PUSH_CGX_SIZ );
		break;
	case GFL_SKB_BGID_S2:
		GXS_LoadBG2Char( src, 0, PUSH_CGX_SIZ );
		break;
	case GFL_SKB_BGID_S3:
		GXS_LoadBG3Char( src, 0, PUSH_CGX_SIZ );
		break;
	}
}

static void* getScrPtr( GFL_SKB_BGID bgID )
{
	switch( bgID ){
	case GFL_SKB_BGID_M0:
		return G2_GetBG0ScrPtr();
	case GFL_SKB_BGID_M1:
		return G2_GetBG1ScrPtr();
	case GFL_SKB_BGID_M2:
		return G2_GetBG2ScrPtr();
	case GFL_SKB_BGID_M3:
		return G2_GetBG3ScrPtr();
	case GFL_SKB_BGID_S0:
		return G2S_GetBG0ScrPtr();
	case GFL_SKB_BGID_S1:
		return G2S_GetBG1ScrPtr();
	case GFL_SKB_BGID_S2:
		return G2S_GetBG2ScrPtr();
	case GFL_SKB_BGID_S3:
		return G2S_GetBG3ScrPtr();
	}
	GF_ASSERT(0);
	return NULL;
}

static void loadScr( GFL_SKB_BGID bgID, void* src )
{
	DC_FlushRange( src, PUSH_SCR_SIZ );

	switch( bgID ){
	case GFL_SKB_BGID_M0:
		GX_LoadBG0Scr( src, 0, PUSH_SCR_SIZ );
		break;
	case GFL_SKB_BGID_M1:
		GX_LoadBG1Scr( src, 0, PUSH_SCR_SIZ );
		break;
	case GFL_SKB_BGID_M2:
		GX_LoadBG2Scr( src, 0, PUSH_SCR_SIZ );
		break;
	case GFL_SKB_BGID_M3:
		GX_LoadBG3Scr( src, 0, PUSH_SCR_SIZ );
		break;
	case GFL_SKB_BGID_S0:
		GXS_LoadBG0Scr( src, 0, PUSH_SCR_SIZ );
		break;
	case GFL_SKB_BGID_S1:
		GXS_LoadBG1Scr( src, 0, PUSH_SCR_SIZ );
		break;
	case GFL_SKB_BGID_S2:
		GXS_LoadBG2Scr( src, 0, PUSH_SCR_SIZ );
		break;
	case GFL_SKB_BGID_S3:
		GXS_LoadBG3Scr( src, 0, PUSH_SCR_SIZ );
		break;
	}
}

static void* getPltPtr( GFL_SKB_BGID bgID )
{
	switch( bgID ){
	case GFL_SKB_BGID_M0:
	case GFL_SKB_BGID_M1:
	case GFL_SKB_BGID_M2:
	case GFL_SKB_BGID_M3:
		return  (void*)HW_BG_PLTT;
	case GFL_SKB_BGID_S0:
	case GFL_SKB_BGID_S1:
	case GFL_SKB_BGID_S2:
	case GFL_SKB_BGID_S3:
		return (void*)HW_DB_BG_PLTT;
	}
	GF_ASSERT(0);
	return NULL;
}

static void loadPlt( GFL_SKB_BGID bgID, void* src, u32 offs )
{
	DC_FlushRange( (void*)( (u32)src + 2 ), PLT_SIZ - 2 );

	switch( bgID ){
	case GFL_SKB_BGID_M0:
	case GFL_SKB_BGID_M1:
	case GFL_SKB_BGID_M2:
	case GFL_SKB_BGID_M3:
		GX_LoadBGPltt( (void*)( (u32)src + 2 ), offs + 2, PLT_SIZ - 2 );
		break;
	case GFL_SKB_BGID_S0:
	case GFL_SKB_BGID_S1:
	case GFL_SKB_BGID_S2:
	case GFL_SKB_BGID_S3:
		GXS_LoadBGPltt( (void*)( (u32)src + 2 ), offs + 2, PLT_SIZ - 2 );
		break;
	}
}

static void getControl( GFL_SKB_BGID bgID, u16* control )
{
	switch( bgID ){
	case GFL_SKB_BGID_M0:
		*control = G2_GetBG0Control().raw;
		break;
	case GFL_SKB_BGID_M1:
		*control = G2_GetBG1Control().raw;
		break;
	case GFL_SKB_BGID_M2:
		*control = G2_GetBG2ControlText().raw;
		break;
	case GFL_SKB_BGID_M3:
		*control = G2_GetBG3ControlText().raw;
		break;
	case GFL_SKB_BGID_S0:
		*control = G2S_GetBG0Control().raw;
		break;
	case GFL_SKB_BGID_S1:
		*control = G2S_GetBG1Control().raw;
		break;
	case GFL_SKB_BGID_S2:
		*control = G2S_GetBG2ControlText().raw;
		break;
	case GFL_SKB_BGID_S3:
		*control = G2S_GetBG3ControlText().raw;
		break;
	}
}

static void setControl( GFL_SKB_BGID bgID, u16* control )
{
	GXBg01Control*		bg01 = (GXBg01Control*)control;
	GXBg23ControlText*	bg23 = (GXBg23ControlText*)control;

	switch( bgID ){
	case GFL_SKB_BGID_M0:
		G2_SetBG0Control(	bg01->screenSize, bg01->colorMode, bg01->screenBase,
							bg01->charBase, bg01->bgExtPltt );
		G2_SetBG0Priority( bg01->priority );
		G2_BG0Mosaic( bg01->mosaic );
		break;
	case GFL_SKB_BGID_M1:
		G2_SetBG1Control(	bg01->screenSize, bg01->colorMode, bg01->screenBase,
							bg01->charBase, bg01->bgExtPltt );
		G2_SetBG1Priority( bg01->priority );
		G2_BG1Mosaic( bg01->mosaic );
		break;
	case GFL_SKB_BGID_M2:
		G2_SetBG2ControlText(	bg23->screenSize, bg23->colorMode, bg23->screenBase,
								bg23->charBase );
		G2_SetBG2Priority( bg23->priority );
		G2_BG2Mosaic( bg23->mosaic );
		break;
	case GFL_SKB_BGID_M3:
		G2_SetBG3ControlText(	bg23->screenSize, bg23->colorMode, bg23->screenBase,
								bg23->charBase );
		G2_SetBG3Priority( bg23->priority );
		G2_BG3Mosaic( bg23->mosaic );
		break;
	case GFL_SKB_BGID_S0:
		G2S_SetBG0Control(	bg01->screenSize, bg01->colorMode, bg01->screenBase,
							bg01->charBase, bg01->bgExtPltt );
		G2S_SetBG0Priority( bg01->priority );
		G2S_BG0Mosaic( bg01->mosaic );
		break;
	case GFL_SKB_BGID_S1:
		G2S_SetBG1Control(	bg01->screenSize, bg01->colorMode, bg01->screenBase,
							bg01->charBase, bg01->bgExtPltt );
		G2S_SetBG1Priority( bg01->priority );
		G2S_BG1Mosaic( bg01->mosaic );
		break;
	case GFL_SKB_BGID_S2:
		G2S_SetBG2ControlText(	bg23->screenSize, bg23->colorMode, bg23->screenBase,
								bg23->charBase );
		G2S_SetBG2Priority( bg23->priority );
		G2S_BG2Mosaic( bg23->mosaic );
		break;
	case GFL_SKB_BGID_S3:
		G2S_SetBG3ControlText(	bg23->screenSize, bg23->colorMode, bg23->screenBase,
								bg23->charBase );
		G2S_SetBG3Priority( bg23->priority );
		G2S_BG3Mosaic( bg23->mosaic );
		break;
	}
}

static void setScroll( GFL_SKB_BGID bgID, int* x, int* y )
{
	switch( bgID ){
	case GFL_SKB_BGID_M0:
		G2_SetBG0Offset( *x, *y );
		break;
	case GFL_SKB_BGID_M1:
		G2_SetBG1Offset( *x, *y );
		break;
	case GFL_SKB_BGID_M2:
		G2_SetBG2Offset( *x, *y );
		break;
	case GFL_SKB_BGID_M3:
		G2_SetBG3Offset( *x, *y );
		break;
	case GFL_SKB_BGID_S0:
		G2S_SetBG0Offset( *x, *y );
		break;
	case GFL_SKB_BGID_S1:
		G2S_SetBG1Offset( *x, *y );
		break;
	case GFL_SKB_BGID_S2:
		G2S_SetBG2Offset( *x, *y );
		break;
	case GFL_SKB_BGID_S3:
		G2S_SetBG3Offset( *x, *y );
		break;
	}
}

static void setAlpha( GFL_SKB_BGID bgID )
{
	int mask =	GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|
				GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD;
	int e1 = 31;
	int e2 = 8;
	
	switch( bgID ){
	case GFL_SKB_BGID_M0:
		G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0, mask&(GX_BLEND_PLANEMASK_BG0^0xffff), e1, e2 );
		break;
	case GFL_SKB_BGID_M1:
		G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1, mask&(GX_BLEND_PLANEMASK_BG1^0xffff), e1, e2 );
		break;
	case GFL_SKB_BGID_M2:
		G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, mask&(GX_BLEND_PLANEMASK_BG2^0xffff), e1, e2 ); 
		break;
	case GFL_SKB_BGID_M3:
		G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG3, mask&(GX_BLEND_PLANEMASK_BG3^0xffff), e1, e2 ); 
		break;
	case GFL_SKB_BGID_S0:
		G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0, mask&(GX_BLEND_PLANEMASK_BG0^0xffff), e1, e2 ); 
		break;
	case GFL_SKB_BGID_S1:
		G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1, mask&(GX_BLEND_PLANEMASK_BG1^0xffff), e1, e2 ); 
		break;
	case GFL_SKB_BGID_S2:
		G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, mask&(GX_BLEND_PLANEMASK_BG2^0xffff), e1, e2 );
		break;
	case GFL_SKB_BGID_S3:
		G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG3, mask&(GX_BLEND_PLANEMASK_BG3^0xffff), e1, e2 );
		break;
	}
}

