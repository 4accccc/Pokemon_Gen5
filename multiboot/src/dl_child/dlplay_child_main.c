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

#include "system/main.h"

#include "../../pokemon_wb/prog/src/test/dlplay/dlplay_comm_func.h"
#include "../../pokemon_wb/prog/src/test/dlplay/dlplay_func.h"
#include "../../pokemon_wb/prog/src/test/ariizumi/ari_debug.h"

#include "dlplay_data_main.h"

//======================================================================
//	define
//======================================================================
//BG面定義
#define DLPLAY_MSG_PLANE			(GFL_BG_FRAME3_M)
#define DLPLAY_MSG_PLANE_PRI		(0)

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

	DLPLAY_COMM_DATA *commSys_;
	DLPLAY_MSG_SYS	 *msgSys_;
	DLPLAY_DATA_DATA *dataSys_;
}DLPLAY_CHILD_DATA;
//======================================================================
//	proto
//======================================================================

DLPLAY_CHILD_DATA *childData;


void	DLPlayChild_SetProc(void);
void	DLPlayChild_InitBg(void);

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

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ARIIZUMI_DEBUG, 0x200000 );

	DLPlayChild_InitBg();


	childData = GFL_HEAP_AllocClearMemory( HEAPID_ARIIZUMI_DEBUG, sizeof( DLPLAY_CHILD_DATA ) );

	childData->heapID_ = HEAPID_ARIIZUMI_DEBUG;
	childData->mainSeq_ = DCS_INIT_COMM;

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
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief		メイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DLPlayChild_ProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
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
		}
		break;
	case DCS_WAIT_CONNECT:
		if( DLPlayComm_IsFinish_ConnectParent( childData->commSys_ ) == TRUE ){
			childData->mainSeq_ = DCS_LOAD_BACKUP_INIT;
			DLPlayComm_Send_ConnectSign(childData->commSys_);
			
			DLPlayFunc_PutString("Succsess connect parent!",childData->msgSys_);
			DLPlayFunc_PutString("Select backup type.",childData->msgSys_);
			DLPlayFunc_PutString("Y = DP : X = PT",childData->msgSys_);
		}
		break;
	
	case DCS_LOAD_BACKUP_INIT:
		if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_X ){
			DLPlayData_SetCardType( childData->dataSys_ , CARD_TYPE_PT ) ;
			childData->mainSeq_ = DCS_LOAD_BACKUP;
		}
		if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_Y ){
			childData->mainSeq_ = DCS_LOAD_BACKUP;
			DLPlayData_SetCardType( childData->dataSys_ , CARD_TYPE_DP );
		}
		break;

	case DCS_LOAD_BACKUP:
		{
			const BOOL ret = DLPlayData_LoadDataFirst( childData->dataSys_ );
			if( ret == TRUE ){ 
				childData->mainSeq_ = DCS_SEND_BOX_INDEX;
				
				DLPlayFunc_PutString("Data load is complete.",childData->msgSys_);
				DLPlayFunc_PutString("Press A button to start send data.",childData->msgSys_);
			}
		}
		break;
	
	case DCS_SEND_BOX_INDEX:
		if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
		{
			DLPLAY_BOX_INDEX *boxIndex = DLPlayComm_GetBoxIndexBuff( childData->commSys_ );
			DLPlayData_SetBoxIndex( childData->dataSys_ , boxIndex );
			DLPlayComm_Send_BoxIndex( childData->commSys_ );
		}
#if 0
			//データ送信
			if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
			{
				DLPLAY_LARGE_PACKET *pak = DLPlayComm_GetLargePacketBuff( childData->commSys_ );
				pak->cardType_ = DLPlayData_GetCardType( childData->dataSys_ );
				{
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
	}

	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief		終了
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DLPlayChild_ProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
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
static const GFL_BG_BGCNT_HEADER bgCont3 = {
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x2800, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
};

void	DLPlayChild_InitBg(void)
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
	GFL_BG_SetBGControl( DLPLAY_MSG_PLANE, &bgCont3, GFL_BG_MODE_TEXT );
	GFL_BG_SetPriority( DLPLAY_MSG_PLANE, DLPLAY_MSG_PLANE_PRI );
	GFL_BG_SetVisible( DLPLAY_MSG_PLANE, VISIBLE_ON );

	//ビットマップウインドウシステムの起動
	GFL_BMPWIN_Init( HEAPID_ARIIZUMI_DEBUG );

}


