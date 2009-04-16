//============================================================================================
/**
 * @file	camera_adjust_view.c
 * @brief	カメラ調整用ビューワー
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"
#include "arc_def.h"

#include "test/camera_adjust_view.h"
//============================================================================================
/**
 *
 *
 *
 * @brief	サウンドステータスビューワー
 *
 *
 *
 */
//============================================================================================
// グラフィックアーカイブ内容ＩＤ定義
enum {
	NARC_ncgr = 0,		// NARC_camera_adjust_view_camera_cont_NCGR = 0,
	NARC_nclr = 1,		// NARC_camera_adjust_view_camera_cont_NCLR = 1,
	NARC_nscr = 2,		// NARC_camera_adjust_view_camera_cont_NSCR = 2
};

static const char arc_path[] = {"camera_adjust_view.narc"};

//============================================================================================
struct _GFL_CAMADJUST {
	HEAPID					heapID;
	int						seq;
	GFL_CAMADJUST_SETUP		setup;
	GFL_DISPUT_VRAMSV*		vramSv;

	u16*					pAngleV;
	u16*					pAngleH;
	fx32*					pLength;

	u16						scrnBuf[32*32];
};

#define PLT_SIZ			(0x20)
#define PUSH_CGX_SIZ	(0x2000)
#define PUSH_SCR_SIZ	(0x800)

static void loadGraphicData( GFL_CAMADJUST* gflCamAdjust );

static BOOL CAMADJUST_Control( GFL_CAMADJUST* gflCamAdjust );
static void CAMADJUST_NumPrint( GFL_CAMADJUST* gflCamAdjust, u16 num, u8 x, u8 y );

//============================================================================================
/**
 *
 * @brief	システム起動 & 終了
 *
 */
//============================================================================================
extern GFL_CAMADJUST*	GFL_CAMADJUST_Create( const GFL_CAMADJUST_SETUP* setup, HEAPID heapID )
{
	GFL_CAMADJUST* gflCamAdjust;
	int i;

	gflCamAdjust = GFL_HEAP_AllocClearMemory( GetHeapLowID(heapID), sizeof(GFL_CAMADJUST) );

	gflCamAdjust->heapID = heapID;
	gflCamAdjust->setup = *setup;

	gflCamAdjust->seq = 0;
	{
		u16	bgPalMask = 0x0001 << gflCamAdjust->setup.bgPalID;
		gflCamAdjust->vramSv =  GFL_DISPUT_CreateVramSv(gflCamAdjust->setup.bgID, 
														PUSH_CGX_SIZ, PUSH_SCR_SIZ, 
														bgPalMask, heapID );
	}
	gflCamAdjust->pAngleV = NULL;
	gflCamAdjust->pAngleH = NULL;
	gflCamAdjust->pLength = NULL;
	return gflCamAdjust;
}

void	GFL_CAMADJUST_Delete( GFL_CAMADJUST* gflCamAdjust )
{
	if( gflCamAdjust == NULL ) return;

	GFL_DISPUT_DeleteVramSv( gflCamAdjust->vramSv );
	GFL_HEAP_FreeMemory( gflCamAdjust );
}

void	GFL_CAMADJUST_SetCameraParam
		( GFL_CAMADJUST* gflCamAdjust, u16* pAngleV, u16* pAngleH, fx32* pLength )
{
	if( gflCamAdjust == NULL ) return;

	gflCamAdjust->pAngleV = pAngleV;
	gflCamAdjust->pAngleH = pAngleH;
	gflCamAdjust->pLength = pLength;
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

BOOL	GFL_CAMADJUST_Main( GFL_CAMADJUST* gflCamAdjust )
{
	if( gflCamAdjust == NULL ){ return FALSE; }

	switch( gflCamAdjust->seq ){

	case SEQ_PUSHVRAM:
		GFL_DISPUT_VisibleOff( gflCamAdjust->setup.bgID );

		GFL_DISPUT_PushVram( gflCamAdjust->vramSv );

		gflCamAdjust->seq = SEQ_MAKEDISP;
		break;

	case SEQ_MAKEDISP:
		loadGraphicData( gflCamAdjust );
		GFL_DISPUT_SetAlpha( gflCamAdjust->setup.bgID, 31, 6 );

		gflCamAdjust->seq = SEQ_DISPON;
		break;

	case SEQ_DISPON:
		GFL_DISPUT_VisibleOn( gflCamAdjust->setup.bgID );
		gflCamAdjust->seq = SEQ_MAIN;
		break;

	case SEQ_MAIN:
		if( gflCamAdjust->setup.cancelKey ){
			if( GFL_UI_KEY_GetTrg() == gflCamAdjust->setup.cancelKey ){
				gflCamAdjust->seq = SEQ_POPVRAM;
				break;
			}
		}
		if( CAMADJUST_Control( gflCamAdjust ) == FALSE ){
			gflCamAdjust->seq = SEQ_POPVRAM;
		}
		CAMADJUST_NumPrint( gflCamAdjust, *gflCamAdjust->pAngleV, 7, 5 );
		CAMADJUST_NumPrint( gflCamAdjust, *gflCamAdjust->pAngleH, 7, 7 );
		CAMADJUST_NumPrint( gflCamAdjust, *gflCamAdjust->pLength / FX32_ONE, 7, 9 );

		GFL_DISPUT_LoadScr(gflCamAdjust->setup.bgID, gflCamAdjust->scrnBuf, 0, PUSH_SCR_SIZ);
		break;

	case SEQ_POPVRAM:
		GFL_DISPUT_VisibleOff( gflCamAdjust->setup.bgID );

		GFL_DISPUT_PopVram( gflCamAdjust->vramSv );

		gflCamAdjust->seq = SEQ_END;
		break;

	case SEQ_END:
		GFL_DISPUT_VisibleOn( gflCamAdjust->setup.bgID );
		return FALSE;
	}
	return TRUE;
}

//============================================================================================
/**
 *
 * @brief	描画関数
 *
 */
//============================================================================================
static void resetScrAttr( u16* scr, GFL_DISPUT_PALID palID )
{
	int i;
	u16 palMask = palID <<12;

	for( i=0; i<32*32; i++ ){
		scr[i] &= 0x0fff;
		scr[i] |= palMask;
	}
}

//============================================================================================
static void loadGraphicData( GFL_CAMADJUST* gflCamAdjust )
{
//	void* binCgx = GFL_ARC_LoadDataFilePathAlloc( arc_path, NARC_ncgr, gflCamAdjust->heapID );
//	void* binScr = GFL_ARC_LoadDataFilePathAlloc( arc_path, NARC_nscr, gflCamAdjust->heapID );
//	void* binPlt = GFL_ARC_LoadDataFilePathAlloc( arc_path, NARC_nclr, gflCamAdjust->heapID );
	void* binCgx = GFL_ARC_LoadDataAlloc( ARCID_CAMERACONT, NARC_ncgr, gflCamAdjust->heapID );
	void* binScr = GFL_ARC_LoadDataAlloc( ARCID_CAMERACONT, NARC_nscr, gflCamAdjust->heapID );
	void* binPlt = GFL_ARC_LoadDataAlloc( ARCID_CAMERACONT, NARC_nclr, gflCamAdjust->heapID );
	void* cgx = GFL_DISPUT_GetCgxDataNNSbin( binCgx );
	void* scr = GFL_DISPUT_GetScrDataNNSbin( binScr ); 
	void* plt = GFL_DISPUT_GetPltDataNNSbin( binPlt );

	GFL_STD_MemCopy32( scr, (void*)gflCamAdjust->scrnBuf, PUSH_SCR_SIZ );
	resetScrAttr( gflCamAdjust->scrnBuf, gflCamAdjust->setup.bgPalID );

	GFL_DISPUT_LoadCgx( gflCamAdjust->setup.bgID, cgx, 0, PUSH_CGX_SIZ );
	GFL_DISPUT_LoadScr( gflCamAdjust->setup.bgID, gflCamAdjust->scrnBuf, 0, PUSH_SCR_SIZ );
	GFL_DISPUT_LoadPlt( gflCamAdjust->setup.bgID, plt, PLT_SIZ * gflCamAdjust->setup.bgPalID );

	GFL_HEAP_FreeMemory( binPlt );
	GFL_HEAP_FreeMemory( binScr );
	GFL_HEAP_FreeMemory( binCgx );
}

//============================================================================================
/**
 *
 * @brief	情報の取得と表示
 *
 */
//============================================================================================
//============================================================================================
/**
 *
 * @brief	タッチパネル判定テーブル
 *
 */
//============================================================================================
#define BTN_SX	(32-1)
#define BTN_SY	(32-1)

#define EXIT_PX			(232)
#define EXIT_PY			(0)
//------------------------------------------------------------------
enum {
	TPBTN_ANGLE_U = 0,
	TPBTN_ANGLE_D,
	TPBTN_ANGLE_L,
	TPBTN_ANGLE_R,
	TPBTN_LENDOWN,
	TPBTN_LENUP,

	//TP_EXIT,

	TPBTN_MAX,
};

static const GFL_UI_TP_HITTBL eventTouchPanelTable[TPBTN_MAX + 1] = {
	{  5*8,  5*8+BTN_SY, 14*8, 14*8+BTN_SX },	//TPBTN_ANGLE_U
	{ 19*8, 19*8+BTN_SY, 14*8, 14*8+BTN_SX },	//TPBTN_ANGLE_D
	{ 12*8, 12*8+BTN_SY,  7*8,  7*8+BTN_SX },	//TPBTN_ANGLE_L
	{ 12*8, 12*8+BTN_SY, 21*8, 21*8+BTN_SX },	//TPBTN_ANGLE_R
	{ 10*8, 10*8+BTN_SY, 14*8, 14*8+BTN_SX },	//TPBTN_LENDOWN
	{ 14*8, 14*8+BTN_SY, 14*8, 14*8+BTN_SX },	//TPBTN_LENUP

	//{ EXIT_PY, EXIT_PY+7, EXIT_PX, EXIT_PX+23 },

	{GFL_UI_TP_HIT_END,0,0,0},			//終了データ
};

//============================================================================================
/**
 *
 * @brief	タッチパネルイベント判定
 *
 */
//============================================================================================
//============================================================================================
/**
 *
 * @brief	メインコントロール	
 *
 */
//============================================================================================
#define ANGLE_ROTATE_SPD		(0x200)
#define	CAMERA_TARGET_HEIGHT	(4 * FX32_ONE)
#define CAMLEN_MVSPD			(8 * FX32_ONE)
#define	CAMANGLEH_MAX			(0x4000 - ANGLE_ROTATE_SPD)
#define	CAMANGLEH_MIN			(0)
#define	CAMLEN_MAX				(4096 * FX32_ONE)
#define	CAMLEN_MIN				(16 * FX32_ONE)

static BOOL CAMADJUST_Control( GFL_CAMADJUST* gflCamAdjust )
{
	int tblPos = GFL_UI_TP_HitCont(eventTouchPanelTable);

	if(gflCamAdjust->pAngleV == NULL){ return TRUE; }
	if(gflCamAdjust->pAngleH == NULL){ return TRUE; }
	if(gflCamAdjust->pLength == NULL){ return TRUE; }

	if(tblPos == GFL_UI_TP_HIT_NONE){
		return TRUE;
	} 
	switch(tblPos){

	case TPBTN_ANGLE_U:
			*(gflCamAdjust->pAngleH) += ANGLE_ROTATE_SPD;
			{
				s16 value = (s16)(*gflCamAdjust->pAngleH);
				if( value > CAMANGLEH_MAX ){ *gflCamAdjust->pAngleH = CAMANGLEH_MAX; }
			}
			break;
	case TPBTN_ANGLE_D:
			*(gflCamAdjust->pAngleH) -= ANGLE_ROTATE_SPD;
			{
				s16 value = (s16)(*gflCamAdjust->pAngleH);
				if( value < CAMANGLEH_MIN ){ *gflCamAdjust->pAngleH = CAMANGLEH_MIN; }
			}
			break;
	case TPBTN_ANGLE_L:
			*(gflCamAdjust->pAngleV) += ANGLE_ROTATE_SPD;
			break;
	case TPBTN_ANGLE_R:
			*(gflCamAdjust->pAngleV) -= ANGLE_ROTATE_SPD;
			break;
	case TPBTN_LENDOWN:
			*(gflCamAdjust->pLength) -= CAMLEN_MVSPD;
			if( *gflCamAdjust->pLength < CAMLEN_MIN ){ *(gflCamAdjust->pLength) = CAMLEN_MIN; }
			break;
	case TPBTN_LENUP:
			*(gflCamAdjust->pLength) += CAMLEN_MVSPD;
			if( *gflCamAdjust->pLength > CAMLEN_MAX ){ *(gflCamAdjust->pLength) = CAMLEN_MAX; }
			break;
	}
	return TRUE;
}

//============================================================================================
/**
 *
 * @brief	数値表示
 *
 */
//============================================================================================
static void CAMADJUST_NumPrint( GFL_CAMADJUST* gflCamAdjust, u16 num, u8 x, u8 y )
{
	u16*	scrnBuf = gflCamAdjust->scrnBuf;
	u16		palMask = gflCamAdjust->setup.bgPalID << 12;
	u16		pat[4];
	int		i;

	pat[0] = (num & 0xf000) >> 12;
	pat[1] = (num & 0x0f00) >> 8;
	pat[2] = (num & 0x00f0) >> 4;
	pat[3] = (num & 0x000f) >> 0;

	for( i= 0; i<4; i++ ){ scrnBuf[ y*32 + x + i ] = (pat[i] + 0xf0) | palMask; }
}


