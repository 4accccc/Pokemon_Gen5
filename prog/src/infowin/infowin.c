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
#include "system/net_err.h"
#include "net/network_define.h"

#include "arc_def.h"
#include "infowin.naix"

#include "infowin/infowin.h"
#include "test/ariizumi/ari_debug.h"

//======================================================================
//	define
//======================================================================
static const u8 INFOWIN_WIDTH = 32;
static const u8 INFOWIN_HEIGHT = 2;

static const u16 INFOWIN_CLEAR_CHR = 0x00;
static const u16 INFOWIN_BLACK_CHR = 0x10;
static const u16 INFOWIN_LINE_CHRNUM = 0x10;	//1列の数

static const u8 INFOWIN_TIMECHR_START = 0x01;
static const u8 INFOWIN_COLONCHR = 0x0B;
static const u8 INFOWIN_GPFSYNC_CHR_X = 0x00;
static const u8 INFOWIN_GPFSYNC_CHR_Y = INFOWIN_LINE_CHRNUM*2;

static const u8 INFOWIN_TIME_DRAW_X = 1;
static const u8 INFOWIN_TIME_DRAW_WIDTH = 5;
static const u8 INFOWIN_AMPM_DRAW_X = 7;
static const u8 INFOWIN_AMPM_DRAW_WIDTH = 2;

static const u8 INFOWIN_GPFSYNC_DRAW_X = 10;
static const u8 INFOWIN_GPFSYNC_DRAW_WIDTH = 6;

static const u8 INFOWIN_IR_DRAW_X = 16;
static const u8 INFOWIN_IR_DRAW_WIDTH = 4;

static const u8 INFOWIN_WIFI_DRAW_X = 20;
static const u8 INFOWIN_WIFI_DRAW_WIDTH = 4;

static const u8 INFOWIN_BEACON_DRAW_X = 24;
static const u8 INFOWIN_BEACON_DRAW_WIDTH = 4;

static const u8 INFOWIN_WMI_DRAW_X = 30;
static const u8 INFOWIN_WMI_DRAW_WIDTH = 2;

static const u8 INFOWIN_BATTERY_DRAW_X = 28;
static const u8 INFOWIN_BATTERY_DRAW_WIDTH = 2;

static const u16 INFOWIN_CHR_FLIP_X = 0x400;
static const u16 INFOWIN_CHR_FLIP_Y = 0x800;
#define INFOWIN_CALC_NUM_SCR(num,pal) (num+INFOWIN_TIMECHR_START+(pal<<12))
#define INFOWIN_CHECK_BIT(value,bit) (value&bit?TRUE:FALSE)

static const u16 INFOWIN_COLOR_BLACK = 0x0000;
static const u16 INFOWIN_COLOR_WHITE = 0x7FFF;
static const u16 INFOWIN_COLOR_GRAY = 0x318c;
static const u16 INFOWIN_COLOR_RED = 0x187b;
static const u16 INFOWIN_COLOR_YERROW = 0x1e7f;
static const u16 INFOWIN_COLOR_GREEN = 0x0667;
//======================================================================
//	enum
//======================================================================
enum
{
	INFOWIN_REFRESH_TIME		= 0x0001,
	INFOWIN_REFRESH_GPF_SYNC	= 0x0002,
	INFOWIN_REFRESH_IR			= 0x0004,
	INFOWIN_REFRESH_WIFI		= 0x0008,
	INFOWIN_REFRESH_BEACON		= 0x0010,
	INFOWIN_REFRESH_BATTERY		= 0x0020,

	INFOWIN_DISP_GPF_SYNC	= 0x0100,
	INFOWIN_DISP_IR			= 0x0200,
	INFOWIN_DISP_BEACON		= 0x0400,
	
	INFOWIN_REFRESH_MASK = 0x003F,
	INFOWIN_DISP_MASK = 0x0700,
	INFOWIN_FLG_NULL = 0x0000,
};

//パレット番号定義
enum
{
	INFOWIN_PLT_NULL,
	INFOWIN_PLT_COMMON_BLACK,
	INFOWIN_PLT_COMMON_WHITE,
	INFOWIN_PLT_TIME_STR,
	INFOWIN_PLT_GPF_SYNC_BACK,
	INFOWIN_PLT_GPF_SYNC_STR,
	INFOWIN_PLT_IR_BACK,
	INFOWIN_PLT_IR_STR,
	INFOWIN_PLT_WIFI_BACK,
	INFOWIN_PLT_WIFI_STR,
	INFOWIN_PLT_BEACON_BACK,
	INFOWIN_PLT_BEACON_STR,
	INFOWIN_PLT_BATTERY_BACK,
	INFOWIN_PLT_BATTERY_STR,
	INFOWIN_PLT_NOTUSE1,
	INFOWIN_PLT_NOTUSE2,
};


typedef enum
{
	IWS_DISABLE,	//Wifi無効
	IWS_ENABLE,		//Wifi有効(鯖無し
	IWS_LOCKED,		//鍵つきサーバー
	IWS_GENERAL,	//鍵無し通常サーバー
	IWS_NINTENDO,	//NintendoSpot
	
	IWS_MAX,
}INFOWIN_WIFI_STATE;

typedef enum
{
	ICS_STOP,
	ICS_WAIT_INIT,
	ICS_FINISH_INIT,
	ICS_SCAN_BEACON,
	ICS_WAIT_FINISH,
	
	ICS_MAX,
}INFOWIN_COMM_STATE;
//======================================================================
//	typedef struct
//======================================================================
typedef struct
{
	u32	ncgPos;
	u16	isRefresh;
	u8	bgplane;
	u8	pltNo;
	
	//時計
	u8		min;
	u8		hour;
	BOOL	isDispColon;
	BOOL	isPm;
	u8		batteryCnt;
	
	//Wi-Fi
	INFOWIN_WIFI_STATE	wifiState;
	
	//BATTERY
	u8	batteryVol;		//0~3

	GFL_TCB *vblankFunc;
}INFOWIN_WORK;

//======================================================================
//	proto
//======================================================================

static	void	INFOWIN_VBlankFunc( GFL_TCB* tcb , void* work );
static	void	INFOWIN_SetScrFunc( const u16 topChar , const u8 pltNo , const u8 posX , const u8 posY , const u8 sizeX , const u8 sizeY );
static	void	INFOWIN_SetPltFunc( const u8 pltPos , const u16 *col , const u8 num );
static	void	INFOWIN_InitBg( u8 bgplane , u8 pltNo, HEAPID heapId );
static	void	INFOWIN_UpdateTime(void);

static	void	INFOWIN_WaitCommInitCallBack(void* pWork);
static	void	INFOWIN_WaitCommExitCallBack(void* pWork);
static	void	INFOWIN_UpdateSearchBeacon( void );

INFOWIN_WORK *infoWk = NULL;
u8	infoCommState = ICS_STOP;

//初期化
//	@param bgplane	BG面
//	@param bgOfs	NCGの読み込むVRAMアドレス(0x1000必要)
//	@param pltNo	パレット番号(1本必要)
//	@param heapId	ヒープID
void	INFOWIN_Init( u8 bgplane , u8 pltNo ,HEAPID heapId )
{
	GF_ASSERT_MSG(infoWk == NULL ,"Infobar is initialized!!\n");
	
	infoWk = GFL_HEAP_AllocMemory( heapId , sizeof( INFOWIN_WORK ) );
	
	infoWk->bgplane = bgplane;
	infoWk->pltNo = pltNo;
	INFOWIN_InitBg( bgplane,pltNo,heapId );
	
	infoWk->vblankFunc = GFUser_VIntr_CreateTCB( INFOWIN_VBlankFunc , (void*)infoWk , 1 );

	infoWk->wifiState = IWS_DISABLE;
	infoWk->batteryVol = 3;
/*
	//ダミー用通信アイコン
	INFOWIN_SetScrFunc( 0x6e,infoWk->pltNo,
						INFOWIN_WMI_DRAW_X,0,
						INFOWIN_WMI_DRAW_WIDTH,2 );
*/
	//一回強制的に更新
	INFOWIN_Update();
	infoWk->isRefresh = INFOWIN_REFRESH_MASK;
}

void	INFOWIN_Update( void )
{
	INFOWIN_UpdateTime();
	//バッテリー
	infoWk->batteryCnt++;
	if(	infoWk->batteryCnt > 200 )

	{
		PMBattery buf;
		if( PM_GetBattery(&buf) == PM_RESULT_SUCCESS )
		{
			const u8 nowVol = (buf==PM_BATTERY_HIGH?3:0);
			if( infoWk->batteryVol != nowVol )
			{
				infoWk->batteryVol = nowVol;
				infoWk->batteryVol |= INFOWIN_REFRESH_BATTERY;
			}
			infoWk->batteryCnt = 0;
		}
	}
	if( INFOWIN_IsStartComm() == TRUE )
	{
		INFOWIN_UpdateSearchBeacon();
	}
}

void	INFOWIN_Exit( void )
{
	GF_ASSERT_MSG(infoWk != NULL ,"Infobar is not initialize!!\n");
	
	GFL_TCB_DeleteTask( infoWk->vblankFunc );
	
	GFL_HEAP_FreeMemory(infoWk);
	infoWk = NULL;
}

static	void	INFOWIN_VBlankFunc( GFL_TCB* tcb , void* work )
{
	BOOL	transReq = FALSE;
	if( INFOWIN_CHECK_BIT(infoWk->isRefresh,INFOWIN_REFRESH_TIME) )
	{
		u8 i;
		u16	scrData[2][5];
		u16	scrDataAmPm[2][2];
		scrData[0][0] = INFOWIN_CALC_NUM_SCR( infoWk->hour/10,infoWk->pltNo)+infoWk->ncgPos;
		scrData[0][1] = INFOWIN_CALC_NUM_SCR( infoWk->hour%10,infoWk->pltNo)+infoWk->ncgPos;
		scrData[0][2] = (infoWk->isDispColon?INFOWIN_COLONCHR:INFOWIN_BLACK_CHR) + (infoWk->pltNo<<12)+infoWk->ncgPos;
		scrData[0][3] = INFOWIN_CALC_NUM_SCR( infoWk->min/10,infoWk->pltNo)+infoWk->ncgPos;
		scrData[0][4] = INFOWIN_CALC_NUM_SCR( infoWk->min%10,infoWk->pltNo)+infoWk->ncgPos;
		scrData[1][0] = scrData[0][0] + INFOWIN_LINE_CHRNUM;
		scrData[1][1] = scrData[0][1] + INFOWIN_LINE_CHRNUM;
		scrData[1][2] = (infoWk->isDispColon?INFOWIN_COLONCHR+INFOWIN_LINE_CHRNUM:INFOWIN_BLACK_CHR) + (infoWk->pltNo<<12)+infoWk->ncgPos;
		scrData[1][3] = scrData[0][3] + INFOWIN_LINE_CHRNUM;
		scrData[1][4] = scrData[0][4] + INFOWIN_LINE_CHRNUM;
		
		
		GFL_BG_WriteScreenExpand( infoWk->bgplane , 
							INFOWIN_TIME_DRAW_X , 0 , 
							INFOWIN_TIME_DRAW_WIDTH , INFOWIN_HEIGHT ,
							scrData,0,0,5,2);
		
		if( infoWk->isPm == TRUE )
		{
			scrDataAmPm[0][0] = 0x0D+(infoWk->pltNo<<12)+infoWk->ncgPos;
			scrDataAmPm[1][0] = 0x1D+(infoWk->pltNo<<12)+infoWk->ncgPos;
		}
		else
		{
			scrDataAmPm[0][0] = 0x0C+(infoWk->pltNo<<12)+infoWk->ncgPos;
			scrDataAmPm[1][0] = 0x1C+(infoWk->pltNo<<12)+infoWk->ncgPos;
		}
		scrDataAmPm[0][1] = (0x0E)+(infoWk->pltNo<<12)+infoWk->ncgPos;
		scrDataAmPm[1][1] = (0x1E)+(infoWk->pltNo<<12)+infoWk->ncgPos;
		GFL_BG_WriteScreenExpand( infoWk->bgplane , 
							INFOWIN_AMPM_DRAW_X , 0 , 
							INFOWIN_AMPM_DRAW_WIDTH , INFOWIN_HEIGHT ,
							scrDataAmPm,0,0,2,2);
		
		infoWk->isRefresh -= INFOWIN_REFRESH_TIME;
		transReq = TRUE;
	}
	
	if( INFOWIN_CHECK_BIT(infoWk->isRefresh,INFOWIN_REFRESH_GPF_SYNC) )
	{
		static const u16 GPFSYNC_TOP_CHAR[2] = {0x20,0x20};
		const u8	flg = INFOWIN_CHECK_BIT(infoWk->isRefresh,INFOWIN_DISP_GPF_SYNC);
		INFOWIN_SetScrFunc( GPFSYNC_TOP_CHAR[flg],infoWk->pltNo,
							INFOWIN_GPFSYNC_DRAW_X,0,
							INFOWIN_GPFSYNC_DRAW_WIDTH,2 );
		{//Pallet
			static const u16 GPFSYNC_BACK_COLOR[2][2] = {
						{INFOWIN_COLOR_GRAY,INFOWIN_COLOR_BLACK},
						{INFOWIN_COLOR_WHITE,INFOWIN_COLOR_BLACK} };
						
			INFOWIN_SetPltFunc( INFOWIN_PLT_GPF_SYNC_BACK , GPFSYNC_BACK_COLOR[flg] , 2 );
		}
		
		infoWk->isRefresh -= INFOWIN_REFRESH_GPF_SYNC;
		transReq = TRUE;
		
	}
	
	if( INFOWIN_CHECK_BIT(infoWk->isRefresh,INFOWIN_REFRESH_IR) )
	{
		static const u16 IR_TOP_CHAR[2] = {0x48,0x48};
		const u8	flg = INFOWIN_CHECK_BIT(infoWk->isRefresh,INFOWIN_DISP_IR);
		INFOWIN_SetScrFunc( IR_TOP_CHAR[flg],infoWk->pltNo,
							INFOWIN_IR_DRAW_X,0,
							INFOWIN_IR_DRAW_WIDTH,2 );
		
		{//Pallet
			static const u16 IR_BACK_COLOR[2][2] = {
						{INFOWIN_COLOR_GRAY,INFOWIN_COLOR_BLACK},
						{INFOWIN_COLOR_WHITE,INFOWIN_COLOR_BLACK} };
						
			INFOWIN_SetPltFunc( INFOWIN_PLT_IR_BACK , IR_BACK_COLOR[flg] , 2 );
		}
		infoWk->isRefresh -= INFOWIN_REFRESH_IR;
		transReq = TRUE;
		
	}
	
	if( INFOWIN_CHECK_BIT(infoWk->isRefresh,INFOWIN_REFRESH_WIFI) )
	{
		static const u16 WIFI_TOP_CHAR[IWS_MAX] = {0x28,0x28,0x28,0x28,0x28};
		INFOWIN_SetScrFunc( WIFI_TOP_CHAR[infoWk->wifiState],infoWk->pltNo,
							INFOWIN_WIFI_DRAW_X,0,
							INFOWIN_WIFI_DRAW_WIDTH,2 );
		
		{//Pallet
			static const u16 WIFI_BACK_COLOR[IWS_MAX][2] = {
						{INFOWIN_COLOR_GRAY,INFOWIN_COLOR_BLACK},
						{INFOWIN_COLOR_WHITE,INFOWIN_COLOR_BLACK},
						{INFOWIN_COLOR_RED,INFOWIN_COLOR_BLACK},
						{INFOWIN_COLOR_YERROW,INFOWIN_COLOR_BLACK},
						{INFOWIN_COLOR_GREEN,INFOWIN_COLOR_BLACK} };
						
			INFOWIN_SetPltFunc( INFOWIN_PLT_WIFI_BACK , WIFI_BACK_COLOR[infoWk->wifiState] , 2 );
		}
		infoWk->isRefresh -= INFOWIN_REFRESH_WIFI;
		transReq = TRUE;
		
	}
	
	if( INFOWIN_CHECK_BIT(infoWk->isRefresh,INFOWIN_REFRESH_BEACON) )
	{
		static const u16 BEACON_TOP_CHAR[2] = {0x2C,0x2C};
		const u8	flg = INFOWIN_CHECK_BIT(infoWk->isRefresh,INFOWIN_DISP_BEACON);
		INFOWIN_SetScrFunc( BEACON_TOP_CHAR[flg],infoWk->pltNo,
							INFOWIN_BEACON_DRAW_X,0,
							INFOWIN_BEACON_DRAW_WIDTH,2 );
		
		{//Pallet
			static const u16 BEACON_BACK_COLOR[2][2] = {
						{INFOWIN_COLOR_GRAY,INFOWIN_COLOR_BLACK},
						{INFOWIN_COLOR_WHITE,INFOWIN_COLOR_BLACK} };
						
			INFOWIN_SetPltFunc( INFOWIN_PLT_BEACON_BACK , BEACON_BACK_COLOR[flg] , 2 );
		}
		infoWk->isRefresh -= INFOWIN_REFRESH_BEACON;
		transReq = TRUE;
		
	}
	
	if( INFOWIN_CHECK_BIT(infoWk->isRefresh,INFOWIN_REFRESH_BATTERY) )
	{
		static const u16 BATTERY_TOP_CHAR[4] = {0x40,0x41,0x42,0x43};
		const u8	flg = INFOWIN_CHECK_BIT(infoWk->isRefresh,INFOWIN_DISP_BEACON);
		u16	scrData[2][2];
		scrData[0][0] = BATTERY_TOP_CHAR[infoWk->batteryVol] + (infoWk->pltNo<<12) + infoWk->ncgPos;
		scrData[1][0] = scrData[0][0] + INFOWIN_LINE_CHRNUM;
		scrData[0][1] = scrData[0][0] + INFOWIN_CHR_FLIP_X;
		scrData[1][1] = scrData[1][0] + INFOWIN_CHR_FLIP_X;
		
		GFL_BG_WriteScreenExpand( infoWk->bgplane , 
							INFOWIN_BATTERY_DRAW_X , 0, 
							INFOWIN_BATTERY_DRAW_WIDTH , INFOWIN_HEIGHT ,
							scrData,0,0,INFOWIN_BATTERY_DRAW_WIDTH,INFOWIN_HEIGHT);
		{//Pallet
			static const u16 BEACON_BACK_COLOR[4][2] = {
						{INFOWIN_COLOR_RED,INFOWIN_COLOR_WHITE},
						{INFOWIN_COLOR_BLACK,INFOWIN_COLOR_RED},
						{INFOWIN_COLOR_BLACK,INFOWIN_COLOR_WHITE},
						{INFOWIN_COLOR_BLACK,INFOWIN_COLOR_WHITE}};
						
			INFOWIN_SetPltFunc( INFOWIN_PLT_BATTERY_BACK , BEACON_BACK_COLOR[infoWk->batteryVol] , 2 );
		}
		infoWk->isRefresh -= INFOWIN_REFRESH_BATTERY;
		transReq = TRUE;
		
	}
	if( transReq == TRUE )
	{
		GFL_BG_LoadScreenReq( infoWk->bgplane );
	}
}

static	void	INFOWIN_SetScrFunc( const u16 topChar , const u8 pltNo , const u8 posX , const u8 posY , const u8 sizeX , const u8 sizeY )
{
	u8 x,y;
	//今のところ最大サイズで確保大きくなったら要変更
	u16 scrData[12];
	GF_ASSERT_MSG( sizeY<=6,"Ysize over! Plz fix source!!\n");
	for( x=0;x<sizeX;x++ )
	{
		for( y=0;y<sizeY;y++ )
		{
			scrData[y*sizeX+x] = topChar+(pltNo<<12)+x+(y*INFOWIN_LINE_CHRNUM)+infoWk->ncgPos;
		}
	}
	GFL_BG_WriteScreenExpand( infoWk->bgplane , 
						posX , posY , 
						sizeX , sizeY ,
						scrData,0,0,sizeX,sizeY);
}

static	void	INFOWIN_SetPltFunc( const u8 pltPos , const u16 *col , const u8 num )
{
	GFL_BG_LoadPalette( infoWk->bgplane , (void*)col , sizeof(u16)*num , sizeof(u16)*(infoWk->pltNo*16 + pltPos) );
}


static	void	INFOWIN_InitBg( u8 bgplane , u8 pltNo, HEAPID heapId )
{
	const PALTYPE palType = (bgplane<=GFL_BG_FRAME3_M ? PALTYPE_MAIN_BG : PALTYPE_SUB_BG );
	ARCHANDLE *arcHdl = GFL_ARC_OpenDataHandle(ARCID_INFOWIN,heapId);
	
	//領域の確保
	infoWk->ncgPos = GFL_BG_AllocCharacterArea( bgplane , 0x1000 , GFL_BG_CHRAREA_GET_B );
	GF_ASSERT_MSG( infoWk->ncgPos != AREAMAN_POS_NOTFOUND ,"Infobar生成に必要なキャラＶＲＡＭ領域が足りない\n" );

	//データの読み込み
	GFL_ARCHDL_UTIL_TransVramPalette( arcHdl,NARC_infowin_infobar_NCLR,palType,pltNo*2*0x10,2*0x10,heapId );
	GFL_ARCHDL_UTIL_TransVramBgCharacter( arcHdl,NARC_infowin_infobar_NCGR,bgplane,infoWk->ncgPos,0,FALSE,heapId );
	GFL_ARC_CloseDataHandle(arcHdl);
	
	//初期状態BGの設定
	GFL_BG_FillScreen( bgplane,INFOWIN_BLACK_CHR+infoWk->ncgPos,0,0,INFOWIN_WIDTH,INFOWIN_HEIGHT,pltNo);
	GFL_BG_LoadScreenReq( bgplane );
}



//--------------------------------------------------------------
//	時計の更新
//--------------------------------------------------------------
static	void	INFOWIN_UpdateTime(void)
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
			infoWk->isRefresh |= INFOWIN_REFRESH_TIME;
		}
		if( time.minute != infoWk->min )
		{
			infoWk->min = time.minute;
			infoWk->isRefresh |= INFOWIN_REFRESH_TIME;
		}
		if( isDispColon != infoWk->isDispColon )
		{
			infoWk->isDispColon = isDispColon;
			infoWk->isRefresh |= INFOWIN_REFRESH_TIME;
		}
	}
}

#pragma mark [>comm func

const BOOL	INFOWIN_IsStartComm( void )
{
	if( infoCommState==ICS_STOP ||
		infoCommState==ICS_WAIT_FINISH )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
	
}

const BOOL	INFOWIN_IsStopComm( void )
{
	if( infoCommState==ICS_STOP )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
	
}

void	INFOWIN_StartComm( void )
{
	if( infoCommState != ICS_STOP )
	{
		OS_TPrintf("Infobar comm is started!!\n");
		return;
	}
	{
	GFLNetInitializeStruct aGFLNetInit = {
		NULL,		//NetSamplePacketTbl,  // 受信関数テーブル
		0,			// 受信テーブル要素数
		NULL,		///< ハードで接続した時に呼ばれる
		NULL,		///< ネゴシエーション完了時にコール
		NULL,		// ユーザー同士が交換するデータのポインタ取得関数
		NULL,		// ユーザー同士が交換するデータのサイズ取得関数
		NULL,		// ビーコンデータ取得関数  
		NULL,		// ビーコンデータサイズ取得関数 
		NULL,		// ビーコンのサービスを比較して繋いで良いかどうか判断する
		NULL,		// 通信不能なエラーが起こった場合呼ばれる
		NULL,		//FatalError
		NULL,		// 通信切断時に呼ばれる関数(終了時
		NULL,		// オート接続で親になった場合
#if GFL_NET_WIFI
		NULL,		///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
		NULL,		///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
		NULL,		///< wifiフレンドリスト削除コールバック
		NULL,		///< DWC形式の友達リスト	
		NULL,		///< DWCのユーザデータ（自分のデータ）
		0,			///< DWCへのHEAPサイズ
		TRUE,		///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
		0x444,		//ggid  DP=0x333,RANGER=0x178,WII=0x346
		GFL_HEAPID_APP,		//元になるheapid
		HEAPID_NETWORK + HEAPDIR_MASK,		//通信用にcreateされるHEAPID
		HEAPID_WIFI + HEAPDIR_MASK,		//wifi用にcreateされるHEAPID
		HEAPID_NETWORK + HEAPDIR_MASK,		//
		GFL_WICON_POSX,GFL_WICON_POSY,	// 通信アイコンXY位置
		4,//_MAXNUM,	//最大接続人数
		48,//_MAXSIZE,	//最大送信バイト数
		4,//_BCON_GET_NUM,  // 最大ビーコン収集数
		TRUE,		// CRC計算
		FALSE,		// MP通信＝親子型通信モードかどうか
		GFL_NET_TYPE_WIRELESS_SCANONLY,		//通信タイプの指定
		TRUE,		// 親が再度初期化した場合、つながらないようにする場合TRUE
		WB_NET_FIELDMOVE_SERVICEID,	//GameServiceID
#if GFL_NET_IRC
		IRC_TIMEOUT_STANDARD,	// 赤外線タイムアウト時間
#endif
		};
		
		GFL_NET_Init( &aGFLNetInit , INFOWIN_WaitCommInitCallBack , (void*)infoWk );	
	}
	
	infoCommState = ICS_WAIT_INIT;
	
}
void	INFOWIN_StopComm( void )
{
	if( infoCommState == ICS_STOP )
	{
		OS_TPrintf("Infobar comm is stoped!!\n");
		return;
	}
	GFL_NET_Exit(INFOWIN_WaitCommExitCallBack);
	infoCommState = ICS_WAIT_FINISH;
}


static void	INFOWIN_WaitCommInitCallBack(void* pWork)
{
	infoCommState = ICS_FINISH_INIT;
}


static void	INFOWIN_WaitCommExitCallBack(void* pWork)
{
	infoCommState = ICS_STOP;
}

//--------------------------------------------------------------
//	ビーコンの捜索
//--------------------------------------------------------------
static	void INFOWIN_UpdateSearchBeacon( void )
{
	u8 bcnIdx = 0;
	int targetIdx = -1;
	void *bcnData;
	BOOL flg = FALSE;

	if( infoCommState == ICS_FINISH_INIT )
	{
		infoCommState = ICS_SCAN_BEACON;
		GFL_NET_StartBeaconScan();
	}

	while( GFL_NET_GetBeaconData(bcnIdx) != NULL )
	{
		bcnData = GFL_NET_GetBeaconData( bcnIdx );
		flg = TRUE;
		bcnIdx++;
	}
	
	if( flg != INFOWIN_CHECK_BIT( infoWk->isRefresh , INFOWIN_DISP_BEACON ) )
	{
		if( flg == TRUE )
		{
			infoWk->isRefresh |= INFOWIN_DISP_BEACON;
			ARI_TPrintf("INFO:beacon is find!\n");
		}
		else
		{
			infoWk->isRefresh -= INFOWIN_DISP_BEACON;
			ARI_TPrintf("INFO:beacon is lost!\n");
		}
		infoWk->isRefresh |= INFOWIN_REFRESH_BEACON;
	}
}