//======================================================================
/**
 *
 * @file	dlplay_child_main.c	
 * @brief	ダウンロードプレイ子機
 * @author	ariizumi
 * @data	08/10/21
 */
//======================================================================
#include <gflib.h>
#include "arc_def.h"
#include "system/main.h"

#include "../../pokemon_wb/prog/src/test/dlplay/dlplay_comm_func.h"
#include "../../pokemon_wb/prog/src/test/dlplay/dlplay_func.h"
#include "../../pokemon_wb/prog/src/test/ariizumi/ari_debug.h"
#include "dlplay_data_main.h"

#include "mb_test.naix"
//======================================================================
//	define
//======================================================================
//BG面定義
#define DLPLAY_MSG_PLANE			(GFL_BG_FRAME3_M)
#define DLPLAY_MSG_PLANE_PRI		(2) 
#define DLPLAY_MSGWIN_PLANE			(GFL_BG_FRAME2_M)
#define DLPLAY_MSGWIN_PLANE_PRI		(1)
#define DLPLAY_STR_PLANE			(GFL_BG_FRAME1_M)
#define DLPLAY_STR_PLANE_PRI		(0)

#define DLC_OBJ_TEST (0)
//======================================================================
//	enum
//======================================================================
enum DLPLAY_CHILD_STATE
{
	DCS_INIT_COMM,
	DCS_WAIT_INIT_COMM,
	DCS_WAIT_CONNECT,

	DCS_LOAD_BACKUP_INIT,
	DCS_LOAD_BACKUP,
	DCS_SAVE_BACKUP,

	DCS_SEND_BOX_INDEX,
	
	DCS_ERROR_INIT,
	DCS_ERROR_LOOP,

	DCS_MAX,
	
};


//======================================================================
//	typedef struct
//======================================================================
typedef struct
{
	int	heapID_;

	u8	mainSeq_;
	u8	parentMacAddress_[WM_SIZE_BSSID];
	u8	errorState_;

	DLPLAY_COMM_DATA *commSys_;
	DLPLAY_MSG_SYS	 *msgSys_;
	DLPLAY_DATA_DATA *dataSys_;
#if DLC_OBJ_TEST
	void* pCellRes_;	//データは使い終わったら開放～
	void* pAnmRes_;
	GFL_CLSYS_REND	*renderSys_;
	GFL_CLUNIT		*cellUnit_;
	GFL_CLWK		*cellData_[3];
#endif

}DLPLAY_CHILD_DATA;
//======================================================================
//	proto
//======================================================================

DLPLAY_CHILD_DATA *childData;
void	DLPlayChild_SetProc(void);
static void	DLPlayChild_InitBg(void);
static void	DLPlayChild_InitObj(void);
//============================================================================================
//
//
//	プロセスコントロール
//
//
//============================================================================================
static const	GFL_PROC_DATA DLPlayChild_ProcData;

//------------------------------------------------------------------
/**
 * @brief		プロセス設定
 */
//------------------------------------------------------------------
void	DLPlayChild_SetProc(void)
{
	GFL_PROC_SysCallProc(NO_OVERLAY_ID, &DLPlayChild_ProcData, NULL);
}

//------------------------------------------------------------------
/**
 * @brief		初期化
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DLPlayChild_ProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	u8	parentMacAddress[ WM_SIZE_BSSID ];
	WMBssDesc desc;		//親機情報
	//親機の情報を取得してみる
	if( MB_IsMultiBootChild() == FALSE ){
#if DEB_ARI
		//青箱の04080852のMacAddress(手打ち)
		desc.ssid[0] = 0x00;
		desc.ssid[1] = 0x09;
		desc.ssid[2] = 0xBF;
		desc.ssid[3] = 0x08;
		desc.ssid[4] = 0x23;
		desc.ssid[5] = 0x0D;
#else
		GF_ASSERT("This DS is not multiboot child!!\n");
#endif
	}
	else{
		MB_ReadMultiBootParentBssDesc( &desc , 32 , 32 , FALSE , FALSE );
	}

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ARIIZUMI_DEBUG  , 0x200000 );

	DLPlayChild_InitBg();


	childData = GFL_HEAP_AllocClearMemory( HEAPID_ARIIZUMI_DEBUG, sizeof( DLPLAY_CHILD_DATA ) );

	childData->heapID_ = HEAPID_ARIIZUMI_DEBUG;
	childData->mainSeq_ = DCS_INIT_COMM;
	childData->errorState_ = DES_NONE;

	childData->msgSys_	= DLPlayFunc_MsgInit( childData->heapID_ , DLPLAY_MSG_PLANE );	 
	childData->commSys_ = DLPlayComm_InitData( childData->heapID_ );
	childData->dataSys_ = DLPlayData_InitSystem( childData->heapID_ , childData->msgSys_ );

	GFL_STD_MemCopy( (void*)&desc.bssid , (void*)childData->parentMacAddress_ , WM_SIZE_BSSID );
#if DLPLAY_FUNC_USE_PRINT
	DLPlayFunc_PutString("",childData->msgSys_);
	DLPlayFunc_PutString("System Initialize complete.",childData->msgSys_);
	DLPlayComm_SetMsgSys( childData->msgSys_ , childData->commSys_ );
#if DEB_ARI&0
	{
		char str[128];
		sprintf(str,"Parent MacAddress is [%02x:%02x:%02x:%02x:%02x:%02x]"
					,desc.bssid[0]	,desc.bssid[1]	,desc.bssid[2]
					,desc.bssid[3]	,desc.bssid[4]	,desc.bssid[5]	);
		DLPlayFunc_PutString( str , childData->msgSys_ );
	}

	childData->mainSeq_ = DCS_LOAD_BACKUP_INIT;
	DLPlayFunc_PutString("Start backup test mode.",childData->msgSys_);
	DLPlayFunc_PutString("Y = DP : X = PT",childData->msgSys_);
#endif

#endif

	DLPlayChild_InitObj();

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief		メイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DLPlayChild_ProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	if( childData->mainSeq_ != DCS_ERROR_LOOP &&
		childData->mainSeq_ != DCS_ERROR_INIT )
	{
		if(	childData->errorState_ != DES_NONE )
		{
			childData->mainSeq_ = DCS_ERROR_INIT;
		}
		else if( DLPlayData_GetErrorState( childData->dataSys_ ) != DES_NONE )
		{
			childData->errorState_ = DLPlayData_GetErrorState( childData->dataSys_ );
			childData->mainSeq_ = DCS_ERROR_INIT;
		}
		else if( DLPlayComm_GetPostErrorState( childData->commSys_ ) != DES_NONE )
		{
			childData->errorState_ = DLPlayComm_GetPostErrorState( childData->commSys_ );
			childData->mainSeq_ = DCS_ERROR_INIT;
		}
	}
	switch( childData->mainSeq_ )
	{
	case DCS_INIT_COMM:
		DLPlayComm_InitSystem( childData->commSys_ );
		childData->mainSeq_ = DCS_WAIT_INIT_COMM;
		break;

	case DCS_WAIT_INIT_COMM:
		if( DLPlayComm_IsFinish_InitSystem( childData->commSys_ ) ){
			DLPlayComm_InitChild( childData->commSys_ , childData->parentMacAddress_ );
			childData->mainSeq_ = DCS_WAIT_CONNECT;
			
			DLPlayFunc_PutString("Commnicate system initialize complete.",childData->msgSys_);
			DLPlayFunc_PutString("Try connect parent.",childData->msgSys_);
			DLPlayFunc_ChangeBgMsg( MSG_CONNECT_PARENT , DLPLAY_STR_PLANE );
		}
		break;
	case DCS_WAIT_CONNECT:
		if( DLPlayComm_IsFinish_ConnectParent( childData->commSys_ ) == TRUE ){
			childData->mainSeq_ = DCS_LOAD_BACKUP_INIT;
			DLPlayComm_Send_ConnectSign(childData->commSys_);
			
			DLPlayFunc_PutString("Succsess connect parent!",childData->msgSys_);
			DLPlayFunc_PutString("Select backup type.",childData->msgSys_);
			DLPlayFunc_PutString("Y = DP : X = PT",childData->msgSys_);
			DLPlayFunc_ChangeBgMsg( MSG_CONNECTED_PARENT , DLPLAY_STR_PLANE );
			//カードタイプがROMヘッダから確定していたら即ロード
			if( DLPlayData_GetCardType( childData->dataSys_ ) != CARD_TYPE_INVALID )
			{
				childData->mainSeq_ = DCS_LOAD_BACKUP;
				DLPlayFunc_ChangeBgMsg( MSG_WAIT_LOAD , DLPLAY_STR_PLANE );
			}
		}
		break;
	
	case DCS_LOAD_BACKUP_INIT:
		if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_X ){
			DLPlayData_SetCardType( childData->dataSys_ , CARD_TYPE_PT ) ;
			childData->mainSeq_ = DCS_LOAD_BACKUP;
			DLPlayFunc_ChangeBgMsg( MSG_WAIT_LOAD , DLPLAY_STR_PLANE );
		}
		if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_Y ){
			childData->mainSeq_ = DCS_LOAD_BACKUP;
			DLPlayData_SetCardType( childData->dataSys_ , CARD_TYPE_DP );
			DLPlayFunc_ChangeBgMsg( MSG_WAIT_LOAD , DLPLAY_STR_PLANE );
		}
		break;
     
	case DCS_LOAD_BACKUP:
		{
			const BOOL ret = DLPlayData_LoadDataFirst( childData->dataSys_ );
			if( ret == TRUE ){ 
				childData->mainSeq_ = DCS_SEND_BOX_INDEX;
				
				DLPlayFunc_PutString("Data load is complete.",childData->msgSys_);
				DLPlayFunc_PutString("Press A button to start send data.",childData->msgSys_);
				DLPlayFunc_ChangeBgMsg( MSG_SEND_DATA_PARENT , DLPLAY_STR_PLANE );
			}
		}
		break;
	
	case DCS_SEND_BOX_INDEX:
		//if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
		{
			DLPLAY_BOX_INDEX *boxIndex = DLPlayComm_GetBoxIndexBuff( childData->commSys_ );
			DLPlayData_SetBoxIndex( childData->dataSys_ , boxIndex );
			DLPlayComm_Send_BoxIndex( childData->commSys_ );
			childData->mainSeq_ = DCS_MAX;
		}
		if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_Y )
		{
			//セーブテスト用
			//childData->mainSeq_ = DCS_SAVE_BACKUP;
		}

#if 0
			//データ送信
			if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
			{
				DLPLAY_LARGE_PACKET *pak = DLPlayComm_GetLargePacketBuff( childData->commSys_ );
				pak->cardType_ = DLPlayData_GetCardType( childData->dataSys_ );
	DLC_OBJ_TEST			{
					const u8* pokeData = DLPlayData_GetPokeSendData( childData->dataSys_ );
					GFL_STD_MemCopy( (void*)pokeData , (void*)pak->pokeData_ , LARGEPACKET_POKE_SIZE );
				}
				DLPlayComm_Send_LargeData( childData->commSys_ );
			}
#endif
		break;

	case DCS_SAVE_BACKUP:
		DLPlayData_SaveData( childData->dataSys_ );
		break;
	case DCS_ERROR_INIT:
		childData->mainSeq_ = DCS_ERROR_LOOP;
		if( childData->errorState_ == DES_MISS_LOAD_BACKUP )
		{
			DLPlayFunc_ChangeBgMsg( MSG_MISS_LOAD_BACKUP , DLPLAY_STR_PLANE );
		}
		else
		{
			DLPlayFunc_ChangeBgMsg( MSG_ERROR , DLPLAY_STR_PLANE );
		}
		if( DLPlayComm_GetPostErrorState( childData->commSys_ ) == DES_NONE )
		{
			//受信してエラーに来たときは送らない
			DLPlayComm_Send_ErrorState( childData->errorState_ , childData->commSys_ );
		}
		break;

	case DCS_ERROR_LOOP:
		break;
	}
#if DLC_OBJ_TEST
	GFL_CLACT_UNIT_Draw( childData->cellUnit_ );
	GFL_CLACT_Main();
//	GFL_CLACT_ClearOamBuff();
#endif
	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief		終了
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DLPlayChild_ProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
//	GFL_CLACT_USERREND_Delete( childData->renderSys_ );
	GFL_CLACT_Exit();
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief		プロセス関数テーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA DLPlayChild_ProcData = {
	DLPlayChild_ProcInit,
	DLPlayChild_ProcMain,
	DLPlayChild_ProcEnd,
};


//------------------------------------------------------------------
//BG初期化
//------------------------------------------------------------------
//------------------------------------------------------------------
//	VRAM用定義
//------------------------------------------------------------------

static const GFL_BG_SYS_HEADER bgsysHeader = {
	GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
};	
static const GFL_BG_BGCNT_HEADER bgContStr = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, DLPLAY_MSG_PLANE_PRI, 0, 0, FALSE
};

static const GFL_BG_BGCNT_HEADER bgContStrWin = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, DLPLAY_MSGWIN_PLANE_PRI, 0, 0, FALSE
};
static const GFL_BG_BGCNT_HEADER bgContStrMsg = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x08000, 0,//GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, DLPLAY_STR_PLANE_PRI, 0, 0, FALSE
};


static void	DLPlayChild_InitBg(void)
{
	//VRAM系初期化
	//BGシステム起動
	GFL_BG_Init( HEAPID_ARIIZUMI_DEBUG );

	//VRAM設定
	GX_SetBankForBG( GX_VRAM_BG_128_A );
	GX_SetBankForOBJ( GX_VRAM_OBJ_128_B );
	GX_SetBankForSubOBJ( GX_VRAM_SUB_OBJ_16_I );

	//BGモード設定
	GFL_BG_SetBGMode( &bgsysHeader );

	//ＢＧコントロール設定
	GFL_BG_SetBGControl( DLPLAY_MSG_PLANE, &bgContStr, GFL_BG_MODE_TEXT );
	//GFL_BG_SetPriority( DLPLAY_MSG_PLANE, DLPLAY_MSG_PLANE_PRI );
	GFL_BG_SetVisible( DLPLAY_MSG_PLANE, VISIBLE_ON );
	
	GFL_BG_SetBGControl( DLPLAY_MSGWIN_PLANE, &bgContStrWin, GFL_BG_MODE_TEXT );
	//GFL_BG_SetPriority( DLPLAY_MSGWIN_PLANE, DLPLAY_MSGWIN_PLANE_PRI );
	GFL_BG_SetVisible( DLPLAY_MSGWIN_PLANE, VISIBLE_ON );

	GFL_BG_SetBGControl( DLPLAY_STR_PLANE, &bgContStrMsg, GFL_BG_MODE_TEXT );
	//GFL_BG_SetPriority( DLPLAY_MSGWIN_PLANE, DLPLAY_MSGWIN_PLANE_PRI );
	GFL_BG_SetVisible( DLPLAY_STR_PLANE, VISIBLE_ON );

	//BG読み込み開始
	GFL_ARC_UTIL_TransVramBgCharacter( ARCID_MB_TEST , NARC_mb_test_test_bg_NCGR ,
			DLPLAY_MSGWIN_PLANE , 0 , 0 , FALSE , HEAPID_ARIIZUMI_DEBUG );
	GFL_ARC_UTIL_TransVramScreen( ARCID_MB_TEST , NARC_mb_test_test_bg_NSCR ,
			DLPLAY_MSGWIN_PLANE , 0 , 0 , FALSE , HEAPID_ARIIZUMI_DEBUG );
	GFL_ARC_UTIL_TransVramScreen( ARCID_MB_TEST , NARC_mb_test_test_bg2_NSCR ,
			DLPLAY_STR_PLANE , 0 , 0 , FALSE , HEAPID_ARIIZUMI_DEBUG );
	GFL_ARC_UTIL_TransVramPalette( ARCID_MB_TEST , NARC_mb_test_test_bg_NCLR ,
			PALTYPE_MAIN_BG , 0 , 0 , HEAPID_ARIIZUMI_DEBUG );
	

	//ビットマップウインドウシステムの起動
	GFL_BMPWIN_Init( HEAPID_ARIIZUMI_DEBUG );

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , VISIBLE_ON );
}

static void	DLPlayChild_InitObj(void)
{
#if DLC_OBJ_TEST
	//OBJ関係初期化
	{
	static const GFL_REND_SURFACE_INIT renderSurfaceInitData =
		{ 0,0,256,196,CLSYS_DRAW_MAIN };
	GFL_CLWK_DATA cellInitData = { 0,0,0,0,0};
	GFL_CLWK_RES	cellRes;
	GFL_CLWK_RES	cellRes2;
	NNSG2dImageProxy		imgProxy;
	NNSG2dImageProxy		imgProxy2;
	NNSG2dImagePaletteProxy pltProxy;
	NNSG2dCellDataBank		*cellBank = NULL;
	NNSG2dCellAnimBankData	*anmBank = NULL;
	const int heapID = HEAPID_ARIIZUMI_DEBUG;
	GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
	cellSysInitData.oamst_main = 0x10;	//通信アイコンの分
	cellSysInitData.oamnum_main = 128-0x10;

	GFL_CLACT_Init( &cellSysInitData , heapID );
	childData->cellUnit_  = GFL_CLACT_UNIT_Create( 3 , heapID );
	GFL_CLACT_UNIT_SetDefaultRend( childData->cellUnit_ );

	NNS_G2dInitImageProxy( &imgProxy );
	NNS_G2dInitImageProxy( &imgProxy2 );
	NNS_G2dInitImagePaletteProxy( &pltProxy );
	GFL_ARC_UTIL_TransVramPaletteMakeProxy( ARCID_MB_TEST , NARC_mb_test_test_obj_NCLR ,
				NNS_G2D_VRAM_TYPE_2DMAIN , 0 , heapID , &pltProxy );

	GFL_ARC_UTIL_TransVramCharacterMakeProxy( ARCID_MB_TEST , NARC_mb_test_test_obj_NCGR , FALSE ,
				CHAR_MAP_1D , 0 , NNS_G2D_VRAM_TYPE_2DMAIN , 0x4000 , heapID , &imgProxy );
	GFL_ARC_UTIL_TransVramCharacterMakeProxy( ARCID_MB_TEST , NARC_mb_test_poke_icon_mnf_NCGR , FALSE ,
				CHAR_MAP_1D , 0 , NNS_G2D_VRAM_TYPE_2DMAIN , 0x5000 , heapID , &imgProxy2 );
	
	childData->pCellRes_ = GFL_ARC_UTIL_LoadCellBank( ARCID_MB_TEST , NARC_mb_test_test_obj_NCER , 
				FALSE , &cellBank , heapID );

	childData->pAnmRes_ = GFL_ARC_UTIL_LoadAnimeBank( ARCID_MB_TEST , NARC_mb_test_test_obj_NANR ,
				FALSE , &anmBank , heapID );
 
	GFL_CLACT_WK_SetCellResData( &cellRes  , &imgProxy  , &pltProxy , cellBank , anmBank );
	GFL_CLACT_WK_SetCellResData( &cellRes2 , &imgProxy2 , &pltProxy , cellBank , anmBank );
	childData->cellData_[0] = GFL_CLACT_WK_Add( childData->cellUnit_ , &cellInitData , &cellRes ,
				CLWK_SETSF_NONE , heapID );
	childData->cellData_[1] = GFL_CLACT_WK_Add( childData->cellUnit_ , &cellInitData , &cellRes2 ,
				CLWK_SETSF_NONE , heapID );
	childData->cellData_[2] = GFL_CLACT_WK_Add( childData->cellUnit_ , &cellInitData , &cellRes ,
				CLWK_SETSF_NONE , heapID );
	{
		GFL_CLACTPOS pos;
		pos.x = 128;
		pos.y = 64;
		GFL_CLACT_WK_SetDrawFlag( childData->cellData_[0] , TRUE );
		GFL_CLACT_WK_SetDrawFlag( childData->cellData_[1] , TRUE );
		GFL_CLACT_WK_SetDrawFlag( childData->cellData_[2] , TRUE );
		
		GFL_CLACT_WK_SetWldPos( childData->cellData_[0] , &pos );
		GFL_CLACT_WK_SetAutoAnmFlag( childData->cellData_[0] , TRUE );
		pos.y = 80;
		GFL_CLACT_WK_SetWldPos( childData->cellData_[1] , &pos );
		GFL_CLACT_WK_SetAutoAnmFlag( childData->cellData_[1] , TRUE );
		pos.y = 96;
		GFL_CLACT_WK_SetWldPos( childData->cellData_[2] , &pos );
		GFL_CLACT_WK_SetAutoAnmFlag( childData->cellData_[2] , TRUE );
	}

#endif

}


