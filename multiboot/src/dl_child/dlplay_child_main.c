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

#include "../prog/src/test/dlplay/dlplay_comm_func.h"
#include "../prog/src/test/dlplay/dlplay_func.h"
#include "../prog/src/test/ariizumi/ari_debug.h"
#include "dlplay_data_main.h"
#include "print/printsys.h"

#include "mb_test.naix"
#include "message_dl.naix"
#include "font.naix"
//======================================================================
//	define
//======================================================================
//BG面定義
#define DLPLAY_FONT_MSG_PLANE		(GFL_BG_FRAME3_M)
#define DLPLAY_FONT_MSG_PLANE_PRI	(0) 
#define DLPLAY_MSG_PLANE			(GFL_BG_FRAME0_M)
#define DLPLAY_MSG_PLANE_PRI		(2) 
#define DLPLAY_MSGWIN_PLANE			(GFL_BG_FRAME2_M)
#define DLPLAY_MSGWIN_PLANE_PRI		(1)

#define DLC_OBJ_TEST (0)
//======================================================================
//	enum
//======================================================================
enum DLPLAY_CHILD_STATE
{
	DCS_INIT_COMM,		//通信初期化
	DCS_WAIT_INIT_COMM,	//初期化完了待ち
	DCS_WAIT_CONNECT,	//親機に接続

	DCS_LOAD_BACKUP_INIT,	//データ読み込み
	DCS_LOAD_BACKUP,		//
	DCS_SAVE_BACKUP,		//データ保存
	DCS_SAVE_WAIT,			//

	DCS_SEND_BOX_INDEX,		//インデックス送信
	DCS_WAIT_POST_BOX_INDEX,//送信連絡待ち
	DCS_WAIT_SELECT_BOX,	//親機選択中

	DCS_SEND_BOX_DATA,		//ボックスデータ送信
	DCS_WAIT_SEND_BOX_DATA,	//送信連絡待ち

	DCS_DISCONNECT,			//切断
	DCS_DISCONNECT_WAIT,			//切断
	
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
	u8	subSeq_;
	u8	parentMacAddress_[WM_SIZE_BSSID];
	u8	errorState_;
	u16	waitCounter_;
	u16	waitTime_;

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
static void DLPlayChild_SaveMain(void);

//============================================================================================
//
//	プロセスコントロール
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

	PRINTSYS_Init( HEAPID_ARIIZUMI_DEBUG );
	DLPlayChild_InitBg();


	childData = GFL_HEAP_AllocClearMemory( HEAPID_ARIIZUMI_DEBUG, sizeof( DLPLAY_CHILD_DATA ) );
	GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_ARIIZUMI_DEBUG );
	childData->heapID_ = HEAPID_ARIIZUMI_DEBUG;
	childData->mainSeq_ = DCS_INIT_COMM;
	childData->errorState_ = DES_NONE;

	childData->msgSys_	= DLPlayFunc_MsgInit( childData->heapID_ , DLPLAY_MSG_PLANE );	 
	GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_ARIIZUMI_DEBUG );
	childData->commSys_ = DLPlayComm_InitData( childData->heapID_ );
	GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_ARIIZUMI_DEBUG );
	childData->dataSys_ = DLPlayData_InitSystem( childData->heapID_ , childData->msgSys_ );
	GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_ARIIZUMI_DEBUG );
	DLPlayFunc_FontInit( ARCID_FONT_DL , NARC_font_large_nftr ,
						ARCID_MESSAGE_DL , NARC_message_dl_d_dlplay_dat ,
						ARCID_FONT_DL , NARC_font_default_nclr , 
						DLPLAY_FONT_MSG_PLANE , childData->msgSys_ );
	GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_ARIIZUMI_DEBUG );

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
 * @brief		終了
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DLPlayChild_ProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	
	DLPlayFunc_FontTerm( childData->msgSys_ );
	DLPlayComm_TermData( childData->commSys_ );
	DLPlayFunc_MsgTerm( childData->msgSys_ );
	DLPlayData_TermSystem( childData->dataSys_ );

	GFL_BMPWIN_Exit();
	GFL_BG_FreeBGControl( DLPLAY_MSG_PLANE );
	GFL_BG_FreeBGControl( DLPLAY_MSGWIN_PLANE );
	GFL_BG_FreeBGControl( DLPLAY_FONT_MSG_PLANE );
	GFL_BG_Exit();

	GFL_HEAP_FreeMemory( childData );

	GFL_HEAP_DeleteHeap( HEAPID_ARIIZUMI_DEBUG );

	return GFL_PROC_RES_FINISH;
}


//------------------------------------------------------------------
/**
 * @brief		メイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DLPlayChild_ProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	DLPlayFunc_UpdateFont( childData->msgSys_ );
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
			DLPlayFunc_ChangeBgMsg( MSG_CONNECT_PARENT , childData->msgSys_ );
		}
		break;
	case DCS_WAIT_CONNECT:
		if( DLPlayComm_IsFinish_Negotiation( childData->commSys_ ) == TRUE ){
			childData->mainSeq_ = DCS_LOAD_BACKUP_INIT;
			DLPlayComm_Send_ConnectSign(childData->commSys_);
			
			DLPlayFunc_PutString("Succsess connect parent!",childData->msgSys_);
			DLPlayFunc_PutString("Select backup type.",childData->msgSys_);
			DLPlayFunc_PutString("Y = DP : X = PT",childData->msgSys_);
			DLPlayFunc_ChangeBgMsg( MSG_CONNECTED_PARENT , childData->msgSys_ );
			//カードタイプがROMヘッダから確定していたら即ロード
			if( DLPlayData_GetCardType( childData->dataSys_ ) != CARD_TYPE_INVALID )
			{
				childData->mainSeq_ = DCS_LOAD_BACKUP;
				DLPlayFunc_ChangeBgMsg( MSG_WAIT_LOAD , childData->msgSys_ );
			}
		}
		break;
	
	case DCS_LOAD_BACKUP_INIT:
		if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_X ){
			DLPlayData_SetCardType( childData->dataSys_ , CARD_TYPE_PT ) ;
			childData->mainSeq_ = DCS_LOAD_BACKUP;
			DLPlayFunc_ChangeBgMsg( MSG_WAIT_LOAD , childData->msgSys_ );
		}
		if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_Y ){
			childData->mainSeq_ = DCS_LOAD_BACKUP;
			DLPlayData_SetCardType( childData->dataSys_ , CARD_TYPE_DP );
			DLPlayFunc_ChangeBgMsg( MSG_WAIT_LOAD , childData->msgSys_ );
		}
		break;
     
	case DCS_LOAD_BACKUP:
		{
			const BOOL ret = DLPlayData_LoadDataFirst( childData->dataSys_ );
			if( ret == TRUE ){ 
				childData->mainSeq_ = DCS_SEND_BOX_INDEX;
				
				DLPlayFunc_PutString("Data load is complete.",childData->msgSys_);
				DLPlayFunc_PutString("Press A button to start send data.",childData->msgSys_);
				DLPlayFunc_ChangeBgMsg( MSG_SEND_DATA_PARENT , childData->msgSys_ );
			}
		}
		break;
	
	case DCS_SEND_BOX_INDEX:
		{
			DLPLAY_BOX_INDEX *boxIndex = DLPlayComm_GetBoxIndexBuff( childData->commSys_ );
			DLPlayData_SetBoxIndex( childData->dataSys_ , boxIndex );
			DLPlayComm_Send_BoxIndex( childData->commSys_ );
			childData->mainSeq_ = DCS_WAIT_POST_BOX_INDEX;
		}
		break;
	
	case DCS_WAIT_POST_BOX_INDEX:
		if( DLPlayComm_IsPost_SendIndex( childData->commSys_ ) == TRUE )
		{
			childData->mainSeq_ = DCS_WAIT_SELECT_BOX;
			DLPlayFunc_ChangeBgMsg( MSG_PARENT_SELECT_BOX , childData->msgSys_ );
		}
		break;

	case DCS_WAIT_SELECT_BOX:
		if( DLPlayComm_GetSelectBoxNumber(childData->commSys_) != SELECT_BOX_INVALID )
		{
			DLPlayData_SetSelectBoxNumber( DLPlayComm_GetSelectBoxNumber(childData->commSys_),
											childData->dataSys_ );
			childData->mainSeq_ = DCS_SEND_BOX_DATA;
			DLPlayFunc_ChangeBgMsg( MSG_SEND_DATA_PARENT , childData->msgSys_ );
		}
		break;
	case DCS_SEND_BOX_DATA:
		{
			DLPLAY_LARGE_PACKET *lPacket;
			lPacket = DLPlayComm_GetLargePacketBuff( childData->commSys_ );
			DLPlayData_GetPokeSendData( lPacket , 
					DLPlayComm_GetSelectBoxNumber( childData->commSys_ ),
					childData->dataSys_ );
			DLPlayComm_Send_LargeData( childData->commSys_ );
			childData->mainSeq_ = DCS_WAIT_SEND_BOX_DATA;
		}
		break;
		
	case DCS_WAIT_SEND_BOX_DATA:
		if( DLPlayComm_IsPost_SendData( childData->commSys_ ) )
		{
			childData->mainSeq_ = DCS_SAVE_BACKUP;
			childData->subSeq_ = 0;
			DLPlayFunc_ChangeBgMsg( MSG_SAVE , childData->msgSys_ );
		}
		break;

	case DCS_SAVE_BACKUP:
		DLPlayChild_SaveMain();
		break;
	
	case DCS_DISCONNECT:
		DLPlayComm_TermSystem( childData->commSys_ );
		childData->mainSeq_ = DCS_DISCONNECT_WAIT;
		DLPlayFunc_ChangeBgMsg( MSG_CHILD_END , childData->msgSys_ );
		break;

	case DCS_DISCONNECT_WAIT:
		if( DLPlayComm_IsFinish_TermSystem( childData->commSys_ ) == TRUE &&
			DLPlayFunc_CanFontTerm( childData->msgSys_ ) == TRUE )
		{
			OS_TPrintf("END.\n");
			return GFL_PROC_RES_FINISH;
		}
		break;

	case DCS_ERROR_INIT:
		childData->mainSeq_ = DCS_ERROR_LOOP;
		if( childData->errorState_ == DES_MISS_LOAD_BACKUP )
		{
			DLPlayFunc_ChangeBgMsg( MSG_MISS_LOAD_BACKUP , childData->msgSys_ );
		}
		else
		{
			DLPlayFunc_ChangeBgMsg( MSG_ERROR , childData->msgSys_ );
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
#if DLC_OBJ_TEST
	GFL_CLACT_UNIT_Draw( childData->cellUnit_ );
	GFL_CLACT_Main();
//	GFL_CLACT_ClearOamBuff();
#endif
	return GFL_PROC_RES_CONTINUE;
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
static const GFL_BG_BGCNT_HEADER bgContFontMsg = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, DLPLAY_FONT_MSG_PLANE_PRI, 0, 0, FALSE
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
	GX_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_128K );
	GXS_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );

	//BGモード設定
	GFL_BG_SetBGMode( &bgsysHeader );

	//ＢＧコントロール設定
	GFL_BG_SetBGControl( DLPLAY_MSG_PLANE, &bgContStr, GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( DLPLAY_MSGWIN_PLANE, &bgContStrWin, GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( DLPLAY_FONT_MSG_PLANE, &bgContFontMsg, GFL_BG_MODE_TEXT );
	GFL_BG_SetVisible( DLPLAY_MSG_PLANE, VISIBLE_ON );
	GFL_BG_SetVisible( DLPLAY_MSGWIN_PLANE, VISIBLE_ON );
	GFL_BG_SetVisible( DLPLAY_FONT_MSG_PLANE, VISIBLE_ON );
	GFL_BG_FillCharacter( DLPLAY_MSG_PLANE, 0x00, 1, 0 );
	GFL_BG_FillCharacter( DLPLAY_MSGWIN_PLANE , 0x00, 1, 0 );
	GFL_BG_FillCharacter( DLPLAY_FONT_MSG_PLANE , 0x00, 1, 0 );
	GFL_BG_FillScreen( DLPLAY_MSG_PLANE,
		0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( DLPLAY_MSGWIN_PLANE,
		0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( DLPLAY_FONT_MSG_PLANE,
		0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_LoadScreenReq( DLPLAY_MSG_PLANE );
	GFL_BG_LoadScreenReq( DLPLAY_MSGWIN_PLANE );
	GFL_BG_LoadScreenReq( DLPLAY_FONT_MSG_PLANE );

	//BG読み込み開始
	GFL_ARC_UTIL_TransVramBgCharacter( ARCID_MB_TEST , NARC_mb_test_test_bg_NCGR ,
			DLPLAY_MSGWIN_PLANE , 0 , 0 , FALSE , HEAPID_ARIIZUMI_DEBUG );
	GFL_ARC_UTIL_TransVramScreen( ARCID_MB_TEST , NARC_mb_test_test_bg_NSCR ,
			DLPLAY_MSGWIN_PLANE , 0 , 0 , FALSE , HEAPID_ARIIZUMI_DEBUG );
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

static void DLPlayChild_SaveMain(void)
{
	DLPlayData_SaveData( childData->dataSys_ );
	
	switch( childData->subSeq_ )
	{
	case 0:	//最初のセーブ待ち
		if( DLPlayData_IsFinishSaveFirst( childData->dataSys_ ) == TRUE )
		{
			DLPlayComm_Send_CommonFlg( DC_FLG_FINISH_SAVE1_CHILD , 0 , childData->commSys_ );
			childData->subSeq_++;
		}
		break;
	case 1:	//同期待ち
		if( DLPlayComm_IsFinishSaveFlg( DC_FLG_PERMIT_LASTBIT_SAVE , childData->commSys_ ) > 0 )
		{
			childData->waitTime_ = DLPlayComm_IsFinishSaveFlg( DC_FLG_PERMIT_LASTBIT_SAVE , childData->commSys_ );
			DLPlayFunc_InitCounter( &childData->waitCounter_ );
			childData->subSeq_++;
		}
		break;
	case 2:
		if( DLPlayFunc_UpdateCounter( &childData->waitCounter_ , childData->waitTime_ ) == TRUE )
		{
			DLPlayData_PermitLastSaveFirst( childData->dataSys_ );
			childData->subSeq_++;
		}
		break;
	case 3:	//次のセーブ待ち
		if( DLPlayData_IsFinishSaveSecond( childData->dataSys_ ) == TRUE )
		{
			childData->subSeq_++;
		}
		break;
	case 4:	//同期待ち
		if( TRUE )
		{
			DLPlayData_PermitLastSaveSecond( childData->dataSys_ );
			childData->subSeq_++;
		}
		break;
	case 5:
		if( DLPlayData_IsFinishSaveAll( childData->dataSys_ ) == TRUE )
		{
			//子機からフラグは送らない・・・
			childData->subSeq_++;
		}
		break;
	case 6:
		if( DLPlayComm_IsFinishSaveFlg( DC_FLG_FINISH_SAVE_ALL , childData->commSys_ ) > 0 )
		{
			childData->subSeq_  = 0;
			childData->mainSeq_ = DCS_DISCONNECT;
			DLPlayFunc_ChangeBgMsg( MSG_SAVE_END , childData->msgSys_ );
		}
	}
}

