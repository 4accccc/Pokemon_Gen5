//======================================================================
/**
 * @file	情報バー
 * @brief	画面上に常駐する情報表示画面
 * @author	ariizumi
 * @data	09/02/23
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "arc_def.h"
#include "infobar.naix"

#include "infobar/infobar.h"

//======================================================================
//	define
//======================================================================
static const u8 INFOBAR_WIDTH = 32;
static const u8 INFOBAR_HEIGHT = 2;

static const u16 INFOBAR_CLEAR_CHR = 0x00;
static const u16 INFOBAR_BLACK_CHR = 0x10;
static const u16 INFOBAR_LINE_CHRNUM = 0x10;	//1列の数

static const u8 INFOBAR_TIMECHR_START = 0x01;
static const u8 INFOBAR_COLONCHR = 0x0B;
static const u8 INFOBAR_GPFSYNC_CHR_X = 0x00;
static const u8 INFOBAR_GPFSYNC_CHR_Y = INFOBAR_LINE_CHRNUM*2;

static const u8 INFOBAR_TIME_DRAW_X = 1;
static const u8 INFOBAR_TIME_DRAW_WIDTH = 5;
static const u8 INFOBAR_AMPM_DRAW_X = 7;
static const u8 INFOBAR_AMPM_DRAW_WIDTH = 2;

static const u8 INFOBAR_GPFSYNC_DRAW_X = 14;
static const u8 INFOBAR_GPFSYNC_DRAW_WIDTH = 6;

static const u8 INFOBAR_IR_DRAW_X = 20;
static const u8 INFOBAR_IR_DRAW_WIDTH = 2;

static const u8 INFOBAR_WIFI_DRAW_X = 22;
static const u8 INFOBAR_WIFI_DRAW_WIDTH = 4;

static const u8 INFOBAR_BEACON_DRAW_X = 26;
static const u8 INFOBAR_BEACON_DRAW_WIDTH = 4;

static const u8 INFOBAR_WMI_DRAW_X = 30;
static const u8 INFOBAR_WMI_DRAW_WIDTH = 2;

#define INFOBAR_CALC_NUM_SCR(num,pal) (num+INFOBAR_TIMECHR_START+(pal<<12))
#define INFOBAR_CHECK_BIT(value,bit) (value&bit?TRUE:FALSE)
//======================================================================
//	enum
//======================================================================
enum
{
	INFOBAR_REFRESH_TIME		= 0x01,
	INFOBAR_REFRESH_GPF_SYNC	= 0x02,
	INFOBAR_REFRESH_IR			= 0x04,
	INFOBAR_REFRESH_WIFI		= 0x08,
	INFOBAR_REFRESH_BEACON		= 0x10,

	INFOBAR_DISP_GPF_SYNC	= 0x20,
	INFOBAR_DISP_IR			= 0x40,
	INFOBAR_DISP_BEACON		= 0x80,
	
	INFOBAR_REFRESH_MASK = 0x1F,
	INFOBAR_DISP_MASK = 0xE0,
	INFOBAR_FLG_NULL = 0x00,
};


typedef enum
{
	IWS_DISABLE,	//Wifi無効
	IWS_ENABLE,		//Wifi有効(鯖無し
	IWS_LOCKED,		//鍵つきサーバー
	IWS_GENERAL,	//鍵無し通常サーバー
	IWS_NINTENDO,	//NintendoSpot
	
	IWS_MAX,
}INFOBAR_WIFI_STATE;
//======================================================================
//	typedef struct
//======================================================================
typedef struct
{
	u32	ncgPos;
	u8	isRefresh;
	u8	bgplane;
	u8	pltNo;
	
	//時計
	u8		min;
	u8		hour;
	BOOL	isDispColon;
	BOOL	isPm;
	
	//Wi-Fi
	INFOBAR_WIFI_STATE	wifiState;
	
	GFL_TCB *vblankFunc;
}INFOBAR_WORK;

//======================================================================
//	proto
//======================================================================

void	INFOBAR_VBlankFunc( GFL_TCB* tcb , void* work );

void	INFOBAR_InitBg( u8 bgplane , u8 pltNo, HEAPID heapId );

void	INFOBAR_UpdateTime(void);

void	INFOBAR_SetScrFunc( const u16 topChar , const u8 pltNo , const u8 posX , const u8 posY , const u8 sizeX , const u8 sizeY );

INFOBAR_WORK *infoWk = NULL;

//初期化
//	@param bgplane	BG面
//	@param bgOfs	NCGの読み込むVRAMアドレス(0x1000必要)
//	@param pltNo	パレット番号(1本必要)
//	@param heapId	ヒープID
void	INFOBAR_Init( u8 bgplane , u8 pltNo ,HEAPID heapId )
{
	GF_ASSERT_MSG(infoWk == NULL ,"Infobar is initialized!!\n");
	
	infoWk = GFL_HEAP_AllocMemory( heapId , sizeof( INFOBAR_WORK ) );
	
	infoWk->isRefresh = INFOBAR_REFRESH_MASK;
	infoWk->bgplane = bgplane;
	infoWk->pltNo = pltNo;
	INFOBAR_InitBg( bgplane,pltNo,heapId );
	
	infoWk->vblankFunc = GFUser_VIntr_CreateTCB( INFOBAR_VBlankFunc , (void*)infoWk , 1 );

	infoWk->isRefresh |= INFOBAR_DISP_MASK;
	infoWk->wifiState = IWS_GENERAL;

	//ダミー用通信アイコン
	INFOBAR_SetScrFunc( 0x6e,infoWk->pltNo,
						INFOBAR_WMI_DRAW_X,0,
						INFOBAR_WMI_DRAW_WIDTH,2 );
}

void	INFOBAR_Update( void )
{
	INFOBAR_UpdateTime();
}

void	INFOBAR_Term( void )
{
	GF_ASSERT_MSG(infoWk != NULL ,"Infobar is not initialize!!\n");
	
	GFL_TCB_DeleteTask( infoWk->vblankFunc );
	
	GFL_HEAP_FreeMemory(infoWk);
	infoWk = NULL;
}

void	INFOBAR_VBlankFunc( GFL_TCB* tcb , void* work )
{
	BOOL	transReq = FALSE;
	if( INFOBAR_CHECK_BIT(infoWk->isRefresh,INFOBAR_REFRESH_TIME) )
	{
		u8 i;
		u16	scrData[2][5];
		u16	scrDataAmPm[2][2];
		scrData[0][0] = INFOBAR_CALC_NUM_SCR( infoWk->hour/10,infoWk->pltNo);
		scrData[0][1] = INFOBAR_CALC_NUM_SCR( infoWk->hour%10,infoWk->pltNo);
		scrData[0][2] = (infoWk->isDispColon?INFOBAR_COLONCHR:INFOBAR_BLACK_CHR) + (infoWk->pltNo<<12);
		scrData[0][3] = INFOBAR_CALC_NUM_SCR( infoWk->min/10,infoWk->pltNo);
		scrData[0][4] = INFOBAR_CALC_NUM_SCR( infoWk->min%10,infoWk->pltNo);
		scrData[1][0] = scrData[0][0] + INFOBAR_LINE_CHRNUM;
		scrData[1][1] = scrData[0][1] + INFOBAR_LINE_CHRNUM;
		scrData[1][2] = (infoWk->isDispColon?INFOBAR_COLONCHR+INFOBAR_LINE_CHRNUM:INFOBAR_BLACK_CHR) + (infoWk->pltNo<<12);
		scrData[1][3] = scrData[0][3] + INFOBAR_LINE_CHRNUM;
		scrData[1][4] = scrData[0][4] + INFOBAR_LINE_CHRNUM;
		
		GFL_BG_WriteScreenExpand( infoWk->bgplane , 
							INFOBAR_TIME_DRAW_X , 0 , 
							INFOBAR_TIME_DRAW_WIDTH , INFOBAR_HEIGHT ,
							scrData,0,0,5,2);
		
		if( infoWk->isPm == TRUE )
		{
			scrDataAmPm[0][0] = 0x0D+(infoWk->pltNo<<12);
			scrDataAmPm[1][0] = 0x1D+(infoWk->pltNo<<12);
		}
		else
		{
			scrDataAmPm[0][0] = 0x0C+(infoWk->pltNo<<12);
			scrDataAmPm[1][0] = 0x1C+(infoWk->pltNo<<12);
		}
		scrDataAmPm[0][1] = (0x0E)+(infoWk->pltNo<<12);
		scrDataAmPm[1][1] = (0x1E)+(infoWk->pltNo<<12);
		GFL_BG_WriteScreenExpand( infoWk->bgplane , 
							INFOBAR_AMPM_DRAW_X , 0 , 
							INFOBAR_AMPM_DRAW_WIDTH , INFOBAR_HEIGHT ,
							scrDataAmPm,0,0,2,2);
		
		infoWk->isRefresh -= INFOBAR_REFRESH_TIME;
		transReq = TRUE;
	}
	
	if( INFOBAR_CHECK_BIT(infoWk->isRefresh,INFOBAR_REFRESH_GPF_SYNC) )
	{
		static const u16 GPFSYNC_TOP_CHAR[2] = {0x20,0x40};
		const u8	flg = INFOBAR_CHECK_BIT(infoWk->isRefresh,INFOBAR_DISP_GPF_SYNC);
		INFOBAR_SetScrFunc( GPFSYNC_TOP_CHAR[flg],infoWk->pltNo,
							INFOBAR_GPFSYNC_DRAW_X,0,
							INFOBAR_GPFSYNC_DRAW_WIDTH,2 );
		
		infoWk->isRefresh -= INFOBAR_REFRESH_GPF_SYNC;
		transReq = TRUE;
		
	}
	
	if( INFOBAR_CHECK_BIT(infoWk->isRefresh,INFOBAR_REFRESH_IR) )
	{
		static const u16 IR_TOP_CHAR[2] = {0x26,0x46};
		const u8	flg = INFOBAR_CHECK_BIT(infoWk->isRefresh,INFOBAR_DISP_IR);
		INFOBAR_SetScrFunc( IR_TOP_CHAR[flg],infoWk->pltNo,
							INFOBAR_IR_DRAW_X,0,
							INFOBAR_IR_DRAW_WIDTH,2 );
		
		infoWk->isRefresh -= INFOBAR_REFRESH_IR;
		transReq = TRUE;
		
	}
	
	if( INFOBAR_CHECK_BIT(infoWk->isRefresh,INFOBAR_REFRESH_WIFI) )
	{
		static const u16 WIFI_TOP_CHAR[IWS_MAX] = {0x28,0x48,0x60,0x64,0x68};
		INFOBAR_SetScrFunc( WIFI_TOP_CHAR[infoWk->wifiState],infoWk->pltNo,
							INFOBAR_WIFI_DRAW_X,0,
							INFOBAR_WIFI_DRAW_WIDTH,2 );
		
		infoWk->isRefresh -= INFOBAR_REFRESH_WIFI;
		transReq = TRUE;
		
	}
	
	if( INFOBAR_CHECK_BIT(infoWk->isRefresh,INFOBAR_REFRESH_BEACON) )
	{
		static const u16 BEACON_TOP_CHAR[2] = {0x2C,0x4C};
		const u8	flg = INFOBAR_CHECK_BIT(infoWk->isRefresh,INFOBAR_DISP_BEACON);
		INFOBAR_SetScrFunc( BEACON_TOP_CHAR[flg],infoWk->pltNo,
							INFOBAR_BEACON_DRAW_X,0,
							INFOBAR_BEACON_DRAW_WIDTH,2 );
		
		infoWk->isRefresh -= INFOBAR_REFRESH_BEACON;
		transReq = TRUE;
		
	}
	if( transReq == TRUE )
	{
		GFL_BG_LoadScreenReq( infoWk->bgplane );
	}
}

void	INFOBAR_SetScrFunc( const u16 topChar , const u8 pltNo , const u8 posX , const u8 posY , const u8 sizeX , const u8 sizeY )
{
	u8 x,y;
	//今のところ最大サイズで確保大きくなったら要変更
	u16 scrData[12];
	GF_ASSERT_MSG( sizeY<=6,"Ysize over! Plz fix source!!\n");
	for( x=0;x<sizeX;x++ )
	{
		for( y=0;y<sizeY;y++ )
		{
			scrData[y*sizeX+x] = topChar+(pltNo<<12)+x+(y*INFOBAR_LINE_CHRNUM);
		}
	}
	GFL_BG_WriteScreenExpand( infoWk->bgplane , 
						posX , posY , 
						sizeX , sizeY ,
						scrData,0,0,sizeX,sizeY);
}

void	INFOBAR_InitBg( u8 bgplane , u8 pltNo, HEAPID heapId )
{
	const PALTYPE palType = (bgplane<=GFL_BG_FRAME3_M ? PALTYPE_MAIN_BG : PALTYPE_SUB_BG );
	ARCHANDLE *arcHdl = GFL_ARC_OpenDataHandle(ARCID_INFOBAR,heapId);
	
	//領域の確保
	infoWk->ncgPos = GFL_BG_AllocCharacterArea( bgplane , 0x1000 , GFL_BG_CHRAREA_GET_B );
	GF_ASSERT_MSG( infoWk->ncgPos != AREAMAN_POS_NOTFOUND ,"Infobar生成に必要なキャラＶＲＡＭ領域が足りない\n" );

	//データの読み込み
	GFL_ARCHDL_UTIL_TransVramPalette( arcHdl,NARC_infobar_infobar_NCLR,palType,pltNo*2*0x10,2*0x10,heapId );
	GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHdl,NARC_infobar_infobar_NCGR,bgplane,infoWk->ncgPos,0,FALSE,heapId );
	GFL_ARC_CloseDataHandle(arcHdl);
	
	//初期状態BGの設定
	GFL_BG_FillScreen( bgplane,INFOBAR_BLACK_CHR,0,0,INFOBAR_WIDTH,INFOBAR_HEIGHT,pltNo);
	GFL_BG_LoadScreenReq( bgplane );
}



//--------------------------------------------------------------
//	時計の更新
//--------------------------------------------------------------
void	INFOBAR_UpdateTime(void)
{
	RTCTime	time;
	
	if( RTC_GetTime( &time ) == RTC_RESULT_SUCCESS )
	{
		const BOOL isDispColon = (BOOL)(time.second&1);
		if( time.hour != infoWk->hour )
		{
			infoWk->hour = time.hour;
			if( infoWk->hour >= 12 )
			{
				infoWk->hour -=12;
				infoWk->isPm = TRUE;
			}
			else
			{
				infoWk->isPm = FALSE;
			}
			infoWk->isRefresh |= INFOBAR_REFRESH_TIME;
		}
		if( time.minute != infoWk->min )
		{
			infoWk->min = time.minute;
			infoWk->isRefresh |= INFOBAR_REFRESH_TIME;
		}
		if( isDispColon != infoWk->isDispColon )
		{
			infoWk->isDispColon = isDispColon;
			infoWk->isRefresh |= INFOBAR_REFRESH_TIME;
		}
	}
}

