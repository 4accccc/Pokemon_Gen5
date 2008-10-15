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
//=============================================
//グラフィックアーカイブ内容ＩＤ定義
enum {
	NARC_skb_skb_NCGR = 0,
	NARC_skb_skb_NCLR = 1,
	NARC_skb_skb_1_NSCR = 2,
	NARC_skb_skb_2_NSCR = 3,
	NARC_skb_skb_3_NSCR = 4
};
#define CURSOR_CGX (0xfb)
//=============================================

typedef struct {
	void*			cgx;
	void*			scr;
	void*			plt;
	u16				control;
	int				offsH;
	int				offsV;
}VRAM_SV;

struct _GFL_SKB {
	HEAPID					heapID;
	void*					strings;

	u32						seq;
	GFL_SKB_SETUP			setup;
	u16						wait;
	u16						mode;

	VRAM_SV					vramSv;

	GFL_BMP_DATA*			strBmp;
	char*					stringSjis;
	u16*					stringUnicode;
	u32						stringP;
	u32						stringCnt;

	u32						panelIdx;
	GFL_BMP_DATA*			cursorBmp;
};

typedef struct {
	u16		sjisCode;
	u16		uniCode;
}STR_DATA;

#define CHR_SIZ			(0x20)
#define PLT_SIZ			(0x20)
#define PUSH_CGX_SIZ	(0x3800)
#define PUSH_SCR_SIZ	(0x800)
#define PUSH_PLT_SIZ	(PLT_SIZ*2)

static void visibleOn( GFL_SKB_BGID bgID );
static void visibleOff( GFL_SKB_BGID bgID );
static void* getCgxPtr( GFL_SKB_BGID bgID );
static void loadCgx( GFL_SKB_BGID bgID, void* src, u32 offs, u32 siz );
static void* getScrPtr( GFL_SKB_BGID bgID );
static void loadScr( GFL_SKB_BGID bgID, void* src, u32 offs, u32 siz );
static void* getPltPtr( GFL_SKB_BGID bgID );
static void loadPlt( GFL_SKB_BGID bgID, void* src, u32 offs );
static void getControl( GFL_SKB_BGID bgID, u16* control );
static void setControl( GFL_SKB_BGID bgID, u16* control );
static void setScroll( GFL_SKB_BGID bgID, int* x, int* y );
static void setAlpha( GFL_SKB_BGID bgID );

static void resetStrings( GFL_SKB* gflSkb );
static void saveStrings( GFL_SKB* gflSkb );
static void putStrings( GFL_SKB* gflSkb );

static void pushVram
		( GFL_SKB_BGID bgID, GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, VRAM_SV* vramSv );
static void popVram
		( GFL_SKB_BGID bgID, GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, VRAM_SV* vramSv );
static void loadScreen( GFL_SKB_MODE mode, GFL_SKB_BGID bgID, 
						GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, HEAPID heapID );
static void loadGraphicData
	( GFL_SKB_BGID bgID, GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, HEAPID heapID );
static void resetScrAttr( u16* scr, GFL_SKB_PALID palID );
static void lightChangePanel( GFL_SKB_BGID bgID, GFL_SKB_PALID palID, u32 panelIdx );
static BOOL getTouchPanel( u32* panelOffs );

static BOOL	SKB_Control( GFL_SKB* gflSkb );

#define BCOL_P (1)
#define FCOL_P (15)

#define STRBMP_SX		(28)	//入力文字列用bitmap Xsize (キャラ単位)
#define STRBMP_SY		(1)		//入力文字列用bitmap Ysize (キャラ単位)
#define CURSORBMP_SX	(1)		//カーソル用bitmap Xsize (キャラ単位)
#define CURSORBMP_SY	(1)		//カーソル用bitmap Ysize (キャラ単位)

#define STRVRAMOFS	(0x3400)		//入力文字列用bitmap キャラアドレスオフセット
#define STRVRAMSIZ	(STRBMP_SX*0x20)//入力文字列用bitmap データサイズ

#define PANEL_PX	(1)		//入力パネルXpos(キャラ単位)
#define PANEL_PY	(5)		//入力パネルYpos(キャラ単位)
#define PANEL_SX	(3)		//入力パネルXsize(キャラ単位)
#define PANEL_SY	(2)		//入力パネルYsize(キャラ単位)
#define PANEL_CNTX	(10)	//入力パネルX個数
#define PANEL_CNTY	(9)		//入力パネルY個数
#define PANEL_IDX_RESET	(0xffffffff)	//入力パネルIDX初期値

enum {
	END_CODE = 0xffff,
	P3_CODE = 0xfffe,
	P2_CODE = 0xfffd,
	P1_CODE = 0xfffc,
	BS_CODE = 0xfffb,
	SP_CODE = 0xfffa,
	NON_CODE = 0,
};

enum {
	END_CODE_IDX = (PANEL_CNTX*PANEL_CNTY -1),
	P3_CODE_IDX = (PANEL_CNTX*PANEL_CNTY -2),
	P2_CODE_IDX = (PANEL_CNTX*PANEL_CNTY -3),
	P1_CODE_IDX = (PANEL_CNTX*PANEL_CNTY -4),
	BS_CODE_IDX = (PANEL_CNTX*PANEL_CNTY -5),
	SP_CODE_IDX = (PANEL_CNTX*PANEL_CNTY -6),
};

static const STR_DATA strData1[PANEL_CNTX*PANEL_CNTY];
static const STR_DATA strData2[PANEL_CNTX*PANEL_CNTY];
static const STR_DATA strData3[PANEL_CNTX*PANEL_CNTY];
//============================================================================================
/**
 *
 * @brief	システム起動 & 終了
 *
 */
//============================================================================================
GFL_SKB*	GFL_SKB_Create( void* strings, const GFL_SKB_SETUP* setup, HEAPID heapID )
{
	GFL_SKB* gflSkb;

	gflSkb = GFL_HEAP_AllocClearMemory( GetHeapLowID(heapID), sizeof(GFL_SKB) );

	gflSkb->heapID = heapID;
	gflSkb->strings = strings;
	gflSkb->setup = *setup;
	if( gflSkb->setup.strlen > GFL_SKB_STRLEN_MAX ){
		gflSkb->setup.strlen = GFL_SKB_STRLEN_MAX;
	}

	gflSkb->seq = 0;
	gflSkb->wait = 0;
	gflSkb->vramSv.cgx = GFL_HEAP_AllocClearMemory( GetHeapLowID(heapID), PUSH_CGX_SIZ  );
	gflSkb->vramSv.scr = GFL_HEAP_AllocClearMemory( GetHeapLowID(heapID), PUSH_SCR_SIZ  );
	gflSkb->vramSv.plt = GFL_HEAP_AllocClearMemory( GetHeapLowID(heapID), PUSH_PLT_SIZ );

	gflSkb->strBmp = GFL_BMP_Create
						( STRBMP_SX, STRBMP_SY, GFL_BMP_16_COLOR, GetHeapLowID(heapID) );
	gflSkb->cursorBmp = GFL_BMP_Create
						( CURSORBMP_SX, CURSORBMP_SY, GFL_BMP_16_COLOR, GetHeapLowID(heapID) );
	gflSkb->stringSjis = GFL_HEAP_AllocClearMemory( GetHeapLowID(heapID), GFL_SKB_STRBUF_SIZ );
	gflSkb->stringUnicode = GFL_HEAP_AllocClearMemory( GetHeapLowID(heapID), GFL_SKB_STRBUF_SIZ );
	gflSkb->panelIdx = PANEL_IDX_RESET;
	gflSkb->stringP = 0;
	gflSkb->stringCnt = 0;
	gflSkb->mode = setup->mode;

	return gflSkb;
}

void	GFL_SKB_Delete( GFL_SKB* gflSkb )
{
	if( gflSkb != NULL ){
		GFL_HEAP_FreeMemory( gflSkb->stringUnicode );
		GFL_HEAP_FreeMemory( gflSkb->stringSjis );
		GFL_BMP_Delete( gflSkb->cursorBmp );
		GFL_BMP_Delete( gflSkb->strBmp );

		GFL_HEAP_FreeMemory( gflSkb->vramSv.plt );
		GFL_HEAP_FreeMemory( gflSkb->vramSv.scr );
		GFL_HEAP_FreeMemory( gflSkb->vramSv.cgx );
		GFL_HEAP_FreeMemory( gflSkb );
		gflSkb = NULL;
	}
}

void*	GFL_SKB_CreateSjisCodeBuffer( HEAPID heapID )
{
	return GFL_HEAP_AllocClearMemory( heapID, GFL_SKB_STRBUF_SIZ );
}

void	GFL_SKB_DeleteSjisCodeBuffer( void* strbuf )
{
	GFL_HEAP_FreeMemory( strbuf );
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
		loadGraphicData( gflSkb->setup.bgID, gflSkb->setup.bgPalID, 
						gflSkb->setup.bgPalID_on, gflSkb->heapID );
		loadScreen( gflSkb->mode, gflSkb->setup.bgID, gflSkb->setup.bgPalID, 
						gflSkb->setup.bgPalID_on, gflSkb->heapID );
		GFL_STD_MemCopy32( (void*)((u32)getCgxPtr(gflSkb->setup.bgID) + CHR_SIZ*CURSOR_CGX),
							GFL_BMP_GetCharacterAdrs(gflSkb->cursorBmp) , CHR_SIZ );
		putStrings( gflSkb );
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
		return FALSE;
	}
	return TRUE;
}

//============================================================================================
static void resetStrings( GFL_SKB* gflSkb )
{
	if( gflSkb->strings == NULL ){
		return;
	}
	switch( gflSkb->setup.strtype ){
	case GFL_SKB_STRTYPE_STRBUF:
		GFL_STR_ClearBuffer( gflSkb->strings );
		break;
	case GFL_SKB_STRTYPE_SJIS:
		{
			u16* strings = gflSkb->strings;
			int i;

			for( i=0; i<gflSkb->setup.strlen; i++ ){
				strings[i] = 0;
			}
		}
		break;
	}
}

static void saveStrings( GFL_SKB* gflSkb )
{
	if( gflSkb->strings == NULL ){
		return;
	}
	switch( gflSkb->setup.strtype ){
	case GFL_SKB_STRTYPE_STRBUF:
		gflSkb->stringUnicode[gflSkb->stringCnt] = GFL_STR_GetEOMCode();	//終端コード挿入
		GFL_STR_SetStringCode( gflSkb->strings, gflSkb->stringUnicode );
		break;
	case GFL_SKB_STRTYPE_SJIS:
		GFL_STD_MemCopy( gflSkb->stringSjis, gflSkb->strings, gflSkb->setup.strlen*sizeof(u16) );
		break;
	}
}

static void putStrings( GFL_SKB* gflSkb )
{
	GFL_TEXT_PRINTPARAM textParam = { NULL, 0, 0, 1, 1, FCOL_P, BCOL_P, GFL_TEXT_WRITE_16 };

	GFL_BMP_Clear( gflSkb->strBmp, BCOL_P );

	textParam.bmp = gflSkb->strBmp;
	GFL_TEXT_PrintSjisCode( gflSkb->stringSjis, &textParam );

	if( gflSkb->stringCnt < gflSkb->setup.strlen ){
		GFL_BMP_Print( gflSkb->cursorBmp, gflSkb->strBmp, 
						0, 0, textParam.writex, textParam.writey, 8, 8, 0 );
	}
	loadCgx(gflSkb->setup.bgID, GFL_BMP_GetCharacterAdrs(gflSkb->strBmp), STRVRAMOFS, STRVRAMSIZ);
}

//============================================================================================
/**
 *
 * @brief	ローカル関数
 *
 */
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
	loadScr( bgID, vramSv->scr, 0, PUSH_SCR_SIZ );
	loadCgx( bgID, vramSv->cgx, 0, PUSH_CGX_SIZ );
	loadPlt( bgID, vramSv->plt, PLT_SIZ*palID );
	loadPlt( bgID, (void*)((u32)vramSv->plt+PLT_SIZ), PLT_SIZ*palID_on );
}

//============================================================================================
static const char arc_path[] = {"skb.narc"};

static void loadScreen( GFL_SKB_MODE mode, GFL_SKB_BGID bgID, 
						GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, HEAPID heapID )
{
	NNSG2dScreenData*	scr;
	void*				binScr;
	u32					datID;
	u32					panelIdx;

	switch( mode ){
	case GFL_SKB_MODE_HIRAGANA:
		datID = NARC_skb_skb_1_NSCR;
		panelIdx = P1_CODE_IDX;
		break;
	case GFL_SKB_MODE_KATAKANA:
		datID = NARC_skb_skb_2_NSCR;
		panelIdx = P2_CODE_IDX;
		break;
	default:
	case GFL_SKB_MODE_ENGNUM:
		datID = NARC_skb_skb_3_NSCR;
		panelIdx = P3_CODE_IDX;
		break;
	}
	binScr = GFL_ARC_LoadDataFilePathAlloc( arc_path, datID, heapID );

	NNS_G2dGetUnpackedScreenData( binScr, &scr );

	resetScrAttr( (u16*)&scr->rawData[0], palID );

	loadScr( bgID, &scr->rawData[0], 0, PUSH_SCR_SIZ );
	lightChangePanel( bgID, palID_on, panelIdx );

	GFL_HEAP_FreeMemory( binScr );
}

static void loadGraphicData
	( GFL_SKB_BGID bgID, GFL_SKB_PALID palID, GFL_SKB_PALID palID_on, HEAPID heapID )
{
	NNSG2dCharacterData*	cgx;
	NNSG2dPaletteData*		plt;
	int scroll_h = 0;
	int scroll_v = 0;

	void* binCgx = GFL_ARC_LoadDataFilePathAlloc( arc_path, NARC_skb_skb_NCGR, heapID );
	void* binPlt = GFL_ARC_LoadDataFilePathAlloc( arc_path, NARC_skb_skb_NCLR, heapID );

	NNS_G2dGetUnpackedBGCharacterData( binCgx, &cgx );
	NNS_G2dGetUnpackedPaletteData( binPlt, &plt );

	loadCgx( bgID, cgx->pRawData, 0, PUSH_CGX_SIZ );
	loadPlt( bgID, plt->pRawData, PLT_SIZ*palID );
	loadPlt( bgID, (void*)((u32)plt->pRawData + PLT_SIZ), PLT_SIZ*palID_on );
	setScroll( bgID, &scroll_h, &scroll_v );
	setAlpha( bgID );

	GFL_HEAP_FreeMemory( binPlt );
	GFL_HEAP_FreeMemory( binCgx );
}

//============================================================================================
static void resetScrAttr( u16* scr, GFL_SKB_PALID palID )
{
	int i;
	u16 palMask = palID <<12;

	for( i=0; i<32*32; i++ ){
		scr[i] &= 0x0fff;
		scr[i] |= palMask;
	}
}

//============================================================================================
#define SCR_XDATSIZ (32)
static void lightChangePanel( GFL_SKB_BGID bgID, GFL_SKB_PALID palID, u32 panelIdx )
{
	u16*	scr = getScrPtr( bgID );
	u16		x, y;
	u32		panelOffs;
	u16		palMask = palID <<12;
	u16		panelScr[PANEL_SX * PANEL_SY];
	int		i;

	if( panelIdx >= (PANEL_CNTX*PANEL_CNTY) ){
		return;
	}
	y = panelIdx / PANEL_CNTX;
	x = panelIdx % PANEL_CNTX;

	panelOffs = (PANEL_PY*SCR_XDATSIZ) + (y*(SCR_XDATSIZ*PANEL_SY)) + PANEL_PX + (x*PANEL_SX);

	for( i=0; i<PANEL_SY; i++ ){
		GFL_STD_MemCopy16( &scr[panelOffs + SCR_XDATSIZ*i], &panelScr[PANEL_SX*i], 
							sizeof(u16)*PANEL_SX );
	}
	for( i=0; i<(PANEL_SX * PANEL_SY); i++ ){
		panelScr[i] &= 0x0fff;  
		panelScr[i] |= palMask;  
	}
	for( i=0; i<PANEL_SY; i++ ){
		loadScr( bgID, &panelScr[PANEL_SX*i], (panelOffs+SCR_XDATSIZ*i)*2, sizeof(u16)*PANEL_SX );
	}
}

//============================================================================================
static BOOL getTouchPanel( u32* panelOffs )
{
	u32 tpx, tpy;
	
	if( GFL_UI_TP_GetPointTrg( &tpx, &tpy ) == TRUE ){
		u32 board_x0 = PANEL_PX*8;
		u32 board_x1 = (PANEL_PX + (PANEL_CNTX*PANEL_SX))*8;
		u32 board_y0 = PANEL_PY*8;
		u32 board_y1 = (PANEL_PY + (PANEL_CNTY*PANEL_SY))*8;

		if((tpx>=board_x0)&&(tpx<=board_x1)&&(tpy>=board_y0)&&(tpy<=board_y1)){
			u32 panel_x = (tpx - (PANEL_PX*8)) / (PANEL_SX*8);
			u32 panel_xf = (tpx - (PANEL_PX*8)) % (PANEL_SX*8);
			u32 panel_y = (tpy - (PANEL_PY*8)) / (PANEL_SY*8);
			u32 panel_yf = (tpy - (PANEL_PY*8)) % (PANEL_SY*8);

			if((panel_xf>0)&&(panel_xf<PANEL_SX*8-1)&&(panel_yf>0)&&(panel_yf<PANEL_SY*8-1)){
				*panelOffs = panel_y * PANEL_CNTX + panel_x;
				return TRUE;
			}
		}
	}
	*panelOffs = 0xffffffff;
	return FALSE;
}

//============================================================================================
/**
 *
 * @brief	ハードアクセス関数
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

static void loadCgx( GFL_SKB_BGID bgID, void* src, u32 offs, u32 siz )
{
	DC_FlushRange( src, siz );

	switch( bgID ){
	case GFL_SKB_BGID_M0:
		GX_LoadBG0Char( src, offs, siz );
		break;
	case GFL_SKB_BGID_M1:
		GX_LoadBG1Char( src, offs, siz );
		break;
	case GFL_SKB_BGID_M2:
		GX_LoadBG2Char( src, offs, siz );
		break;
	case GFL_SKB_BGID_M3:
		GX_LoadBG3Char( src, offs, siz );
		break;
	case GFL_SKB_BGID_S0:
		GXS_LoadBG0Char( src, offs, siz );
		break;
	case GFL_SKB_BGID_S1:
		GXS_LoadBG1Char( src, offs, siz );
		break;
	case GFL_SKB_BGID_S2:
		GXS_LoadBG2Char( src, offs, siz );
		break;
	case GFL_SKB_BGID_S3:
		GXS_LoadBG3Char( src, offs, siz );
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

static void loadScr( GFL_SKB_BGID bgID, void* src, u32 offs, u32 siz )
{
	DC_FlushRange( src, PUSH_SCR_SIZ );

	switch( bgID ){
	case GFL_SKB_BGID_M0:
		GX_LoadBG0Scr( src, offs, siz );
		break;
	case GFL_SKB_BGID_M1:
		GX_LoadBG1Scr( src, offs, siz );
		break;
	case GFL_SKB_BGID_M2:
		GX_LoadBG2Scr( src, offs, siz );
		break;
	case GFL_SKB_BGID_M3:
		GX_LoadBG3Scr( src, offs, siz );
		break;
	case GFL_SKB_BGID_S0:
		GXS_LoadBG0Scr( src, offs, siz );
		break;
	case GFL_SKB_BGID_S1:
		GXS_LoadBG1Scr( src, offs, siz );
		break;
	case GFL_SKB_BGID_S2:
		GXS_LoadBG2Scr( src, offs, siz );
		break;
	case GFL_SKB_BGID_S3:
		GXS_LoadBG3Scr( src, offs, siz );
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
	int e2 = 6;
	
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

//============================================================================================
/**
 *
 * @brief	メインコントロール	
 *
 */
static BOOL insertCode( GFL_SKB* gflSkb, const STR_DATA* strData );
//============================================================================================
static BOOL SKB_Control( GFL_SKB* gflSkb )
{
	u32 panelIdxNew;
	const STR_DATA* strData;

	if( gflSkb->setup.cancelKey ){
		if( GFL_UI_KEY_GetTrg() == gflSkb->setup.cancelKey ){
			resetStrings( gflSkb );
			return FALSE;
		}
	}

	if( getTouchPanel( &panelIdxNew ) == TRUE ){
		lightChangePanel( gflSkb->setup.bgID, gflSkb->setup.bgPalID, gflSkb->panelIdx );
		gflSkb->panelIdx = panelIdxNew;

		switch( gflSkb->mode ){
		case GFL_SKB_MODE_HIRAGANA:
			strData = strData1;
			break;
		case GFL_SKB_MODE_KATAKANA:
			strData = strData2;
			break;
		default:
		case GFL_SKB_MODE_ENGNUM:
			strData = strData3;
			break;
		}
		lightChangePanel( gflSkb->setup.bgID, gflSkb->setup.bgPalID_on, gflSkb->panelIdx );
		if( insertCode( gflSkb, &strData[gflSkb->panelIdx] ) == FALSE ){
			//生成した文字列をモードに合わせてコピー
			saveStrings( gflSkb );
			return FALSE;
		}
		putStrings( gflSkb );
	}
	return TRUE;
}

//============================================================================================
static BOOL insertCode( GFL_SKB* gflSkb, const STR_DATA* strData )
{
	u8	sig;
	u16 sjisCode;
	u16 uniCode;
	int i, p;

	sjisCode = strData->sjisCode;
	uniCode = strData->uniCode;

	switch( sjisCode ){
	case NON_CODE:
		break;
	case END_CODE:
		return FALSE;

	case P1_CODE:
		if( gflSkb->setup.modeChange == TRUE ){
			gflSkb->mode = GFL_SKB_MODE_HIRAGANA;
			loadScreen( gflSkb->mode, gflSkb->setup.bgID, gflSkb->setup.bgPalID, 
						gflSkb->setup.bgPalID_on, gflSkb->heapID );
		}
		gflSkb->panelIdx = PANEL_IDX_RESET;
		break;
	case P2_CODE:
		if( gflSkb->setup.modeChange == TRUE ){
			gflSkb->mode = GFL_SKB_MODE_KATAKANA;
			loadScreen( gflSkb->mode, gflSkb->setup.bgID, gflSkb->setup.bgPalID, 
						gflSkb->setup.bgPalID_on, gflSkb->heapID );
		}
		gflSkb->panelIdx = PANEL_IDX_RESET;
		break;
	case P3_CODE:
		if( gflSkb->setup.modeChange == TRUE ){
			gflSkb->mode = GFL_SKB_MODE_ENGNUM;
			loadScreen( gflSkb->mode, gflSkb->setup.bgID, gflSkb->setup.bgPalID, 
						gflSkb->setup.bgPalID_on, gflSkb->heapID );
		}
		gflSkb->panelIdx = PANEL_IDX_RESET;
		break;

	case BS_CODE:
		p = 0;
		if( gflSkb->stringCnt == 0 ){
			break;		gflSkb->stringUnicode[gflSkb->stringCnt] = uniCode;

		}
		gflSkb->stringCnt--;

		//sjisCodeの削除
		for( i=0; i<gflSkb->stringCnt; i++ ){
			sig = gflSkb->stringSjis[p];

			if( ((sig >= 0x81)&&(sig <= 0x9f)) || ((sig >= 0xe0)&&(sig <= 0xff)) ){
				p++;
			}
			p++;
		}
		gflSkb->stringP = p;

		while( p<GFL_SKB_STRBUF_SIZ ){
			gflSkb->stringSjis[p] = 0;
			p++;
		}

		//uniCodeの削除
		gflSkb->stringUnicode[gflSkb->stringCnt] = 0;
		break;

	default:
		if( gflSkb->stringCnt >= gflSkb->setup.strlen ){
			break;
		}
		//sjisCodeの挿入
		sig = (sjisCode >>8)&0x00ff;

		if( ((sig >= 0x81)&&(sig <= 0x9f)) || ((sig >= 0xe0)&&(sig <= 0xff)) ){
			gflSkb->stringSjis[gflSkb->stringP] = sig;
			gflSkb->stringP++;
		}
		gflSkb->stringSjis[gflSkb->stringP] = sjisCode & 0x00ff;
		gflSkb->stringP++;

		//uniCodeの挿入
		gflSkb->stringUnicode[gflSkb->stringCnt] = uniCode;

		gflSkb->stringCnt++;
		break;
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
static const STR_DATA strData1[PANEL_CNTX*PANEL_CNTY] = {
{0x82a0,0x3042},{0x82a2,0x3044},{0x82a4,0x3046},{0x82a6,0x3048},{0x82a8,0x304a},//あ,い,う,え,お
{0x829f,0x3041},{0x82a1,0x3043},{0x82a3,0x3045},{0x82a5,0x3047},{0x82a7,0x3049},//ぁ,ぃ,ぅ,ぇ,ぉ

{0x82a9,0x304b},{0x82ab,0x304d},{0x82ad,0x304f},{0x82af,0x3051},{0x82b1,0x3053},//か,き,く,け,こ
{0x82aa,0x304c},{0x82ac,0x304e},{0x82ae,0x3050},{0x82b0,0x3052},{0x82b2,0x3054},//が,ぎ,ぐ,げ,ご

{0x82b3,0x3055},{0x82b5,0x3057},{0x82b7,0x3059},{0x82b9,0x305b},{0x82bb,0x305d},//さ,し,す,せ,そ
{0x82b4,0x3056},{0x82b6,0x3058},{0x82b8,0x305a},{0x82ba,0x305c},{0x82bc,0x305e},//ざ,じ,ず,ぜ,ぞ

{0x82bd,0x305f},{0x82bf,0x3061},{0x82c2,0x3064},{0x82c4,0x3066},{0x82c6,0x3068},//た,ち,つ,て,と
{0x82be,0x3060},{0x82c0,0x3062},{0x82c3,0x3065},{0x82c5,0x3067},{0x82c7,0x3069},//だ,ぢ,づ,で,ど

{0x82c8,0x306a},{0x82c9,0x306b},{0x82ca,0x306c},{0x82cb,0x306d},{0x82cc,0x306e},//な,に,ぬ,ね,の
{0x82ce,0x3070},{0x82d1,0x3073},{0x82d4,0x3076},{0x82d7,0x3079},{0x82da,0x307c},//ば,び,ぶ,べ,ぼ

{0x82cd,0x306f},{0x82d0,0x3072},{0x82d3,0x3075},{0x82d6,0x3078},{0x82d9,0x307b},//は,ひ,ふ,へ,ほ
{0x82cf,0x3071},{0x82d2,0x3074},{0x82d5,0x3077},{0x82d8,0x307a},{0x82db,0x307d},//ぱ,ぴ,ぷ,ぺ,ぽ

{0x82dc,0x307e},{0x82dd,0x307f},{0x82de,0x3080},{0x82df,0x3081},{0x82e0,0x3082},//ま,み,む,め,も
{0x82c1,0x3063},{0x82e1,0x3083},{0x82e3,0x3085},{0x82e5,0x3087},{0x82f0,0x3092},//っ,ゃ,ゅ,ょ,を

{0x82e2,0x3084},{0x82e4,0x3086},{0x82e6,0x3088},{0x82e7,0x3089},{0x82e8,0x308a},//や,ゆ,よ,ら,り
{0x82e9,0x308b},{0x82ea,0x308c},{0x82eb,0x308d},{0x82ed,0x308f},{0x82f1,0x3093},//る,れ,ろ,わ,ん

{0x0021,0x0021},{0x003f,0x003f},{0x815b,0x30fc},{NON_CODE,NON_CODE},{0x8140,0x3000},
{BS_CODE,BS_CODE},{P1_CODE,P1_CODE},{P2_CODE,P2_CODE},{P3_CODE,P3_CODE},{END_CODE,END_CODE},
};

static const STR_DATA strData2[PANEL_CNTX*PANEL_CNTY] = {
{0x8341,0x30a2},{0x8343,0x30a4},{0x8345,0x30a6},{0x8347,0x30a8},{0x8349,0x30aa},//ア,イ,ウ,エ,オ
{0x8340,0x30a1},{0x8342,0x30a3},{0x8344,0x30a5},{0x8346,0x30a7},{0x8348,0x30a9},//ァ,ィ,ゥ,ェ,ォ

{0x834a,0x30ab},{0x834c,0x30ad},{0x834e,0x30af},{0x8350,0x30b1},{0x8352,0x30b3},//カ,キ,ク,ケ,コ
{0x834b,0x30ac},{0x834d,0x30ae},{0x834f,0x30b0},{0x8351,0x30b2},{0x8353,0x30b4},//ガ,ギ,グ,ゲ,ゴ

{0x8354,0x30b5},{0x8356,0x30b7},{0x8358,0x30b9},{0x835a,0x30bb},{0x835c,0x30bd},//サ,シ,ス,セ,ソ
{0x8355,0x30b6},{0x8357,0x30b8},{0x8359,0x30ba},{0x835b,0x30bc},{0x835d,0x30be},//ザ,ジ,ズ,ゼ,ゾ

{0x835e,0x30bf},{0x8360,0x30c1},{0x8363,0x30c4},{0x8365,0x30c6},{0x8367,0x30c8},//タ,チ,ツ,テ,ト
{0x835f,0x30c0},{0x8361,0x30c2},{0x8364,0x30c5},{0x8366,0x30c7},{0x8368,0x30c9},//ダ,ヂ,ヅ,デ,ド

{0x8369,0x30ca},{0x836a,0x30cb},{0x836b,0x30cc},{0x836c,0x30cd},{0x836d,0x30ce},//ナ,ニ,ヌ,ネ,ノ
{0x836f,0x30d0},{0x8372,0x30d3},{0x8375,0x30d6},{0x8378,0x30d9},{0x837b,0x30dc},//バ,ビ,ブ,ベ,ボ

{0x836e,0x30cf},{0x8371,0x30d2},{0x8374,0x30d5},{0x8377,0x30d8},{0x837a,0x30db},//ハ,ヒ,フ,ヘ,ホ
{0x8370,0x30d1},{0x8373,0x30d4},{0x8376,0x30d7},{0x8379,0x30da},{0x837c,0x30dd},//パ,ピ,プ,ペ,ポ

{0x837d,0x30de},{0x837e,0x30df},{0x8380,0x30e0},{0x8381,0x30e1},{0x8382,0x30e2},//マ,ミ,ム,メ,モ
{0x8362,0x30c3},{0x8383,0x30e3},{0x8385,0x30e5},{0x8387,0x30e7},{0x8392,0x30f2},//ッ,ャ,ュ,ョ,ヲ

{0x8384,0x30e4},{0x8386,0x30e6},{0x8388,0x30e8},{0x8389,0x30e9},{0x838a,0x30ea},//ヤ,ユ,ヨ,ラ,リ
{0x838b,0x30eb},{0x838c,0x30ec},{0x838d,0x30ed},{0x838f,0x30ef},{0x8393,0x30f3},//ル,レ,ロ,ワ,ン

{0x0021,0x0021},{0x003f,0x003f},{0x815b,0x30fc},{NON_CODE,NON_CODE},{0x8140,0x3000},
{BS_CODE,BS_CODE},{P1_CODE,P1_CODE},{P2_CODE,P2_CODE},{P3_CODE,P3_CODE},{END_CODE,END_CODE},
};

static const STR_DATA strData3[PANEL_CNTX*PANEL_CNTY] = {
{0x0041,0x0041},{0x0042,0x0042},{0x0043,0x0043},{0x0044,0x0044},{0x0045,0x0045},//A,B,C,D,E
{0x0046,0x0046},{0x0047,0x0047},{0x0048,0x0048},{0x0049,0x0049},{0x004a,0x004a},//F,G,H,I,J

{0x004b,0x004b},{0x004c,0x004c},{0x004d,0x004d},{0x004e,0x004e},{0x004f,0x004f},//K,L,M,N,O
{0x0050,0x0050},{0x0051,0x0051},{0x0052,0x0052},{0x0053,0x0053},{0x0054,0x0054},//P,Q,R,S,T

{0x0055,0x0055},{0x0056,0x0056},{0x0057,0x0057},{0x0058,0x0058},{0x0059,0x0059},//U,V,W,X,Y
{0x005a,0x005a},{0x002e,0x002e},{0x002c,0x002c},{0x0027,0x0027},{0x0022,0x0022},//Z,

{0x0061,0x0061},{0x0062,0x0062},{0x0063,0x0063},{0x0064,0x0064},{0x0065,0x0065},//a,b,c,d,e
{0x0066,0x0066},{0x0067,0x0067},{0x0068,0x0068},{0x0069,0x0069},{0x006a,0x006a},//f,g,h,i,j

{0x006b,0x006b},{0x006c,0x006c},{0x006d,0x006d},{0x006e,0x006e},{0x006f,0x006f},//k,l,m,n,o
{0x0070,0x0070},{0x0071,0x0071},{0x0072,0x0072},{0x0073,0x0073},{0x0074,0x0074},//p,q,r,s,t

{0x0075,0x0075},{0x0076,0x0076},{0x0077,0x0077},{0x0078,0x0078},{0x0079,0x0079},//u,v,w,x,y
{0x007a,0x007a},{0x002f,0x002f},{0x003a,0x003a},{0x003b,0x003b},{0x005f,0x005f},//z

{0x0030,0x0030},{0x0031,0x0031},{0x0032,0x0032},{0x0033,0x0033},{0x0034,0x0034},//0,1,2,3,4
{0x0035,0x0035},{0x0036,0x0036},{0x0037,0x0037},{0x0038,0x0038},{0x0039,0x0039},//5,6,7,8,9

{0x005b,0x005b},{0x005d,0x005d},{0x002b,0x002b},{0x002d,0x002d},{0x002a,0x002a},
{0x002f,0x002f},{0x003d,0x003d},{0x0025,0x0025},{0x0026,0x0026},{0x005c,0x005c},

{0x0021,0x0021},{0x003f,0x003f},{0x0028,0x0028},{0x0029,0x0029},{0x0020,0x0020},
{BS_CODE,BS_CODE},{P1_CODE,P1_CODE},{P2_CODE,P2_CODE},{P3_CODE,P3_CODE},{END_CODE,END_CODE},
};

