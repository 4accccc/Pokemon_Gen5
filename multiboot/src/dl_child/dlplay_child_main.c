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

	DCS_LOAD_BACKUP,
	DCS_SAVE_BACKUP,
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
	DLPlayFunc_PutString("",childData->msgSys_);
	childData->commSys_ = DLPlayComm_InitData( childData->heapID_ );
	childData->dataSys_ = DLPlayData_InitSystem( childData->heapID_ , childData->msgSys_ );

	GFL_STD_MemCopy( (void*)&desc.bssid , (void*)childData->parentMacAddress_ , WM_SIZE_BSSID );

	DLPlayFunc_PutString("System Initialize complete.",childData->msgSys_);

#if DEB_ARI
	{
		char str[128];
		sprintf(str,"Parent MacAddress is [%02x:%02x:%02x:%02x:%02x:%02x]"
					,desc.bssid[0]	,desc.bssid[1]	,desc.bssid[2]
					,desc.bssid[3]	,desc.bssid[4]	,desc.bssid[5]	);
		DLPlayFunc_PutString( str , childData->msgSys_ );
	}

	childData->mainSeq_ = DCS_LOAD_BACKUP;
	DLPlayFunc_PutString("Start backup test mode.",childData->msgSys_);

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
			DLPlayFunc_PutString("Commnicate system initialize complete.",childData->msgSys_);
			DLPlayFunc_PutString("Try connect parent.",childData->msgSys_);
			childData->mainSeq_ = DCS_WAIT_CONNECT;
		}
		break;
	case DCS_WAIT_CONNECT:
		if( DLPlayComm_IsFinish_ConnectParent( childData->commSys_ ) == TRUE ){
			DLPlayFunc_PutString("Succsess connect parent!",childData->msgSys_);
			childData->mainSeq_ = DCS_MAX;
			
			DLPlayComm_Send_ConnectSign(childData->commSys_);
		}
		break;
	case DCS_MAX:
			//データ送信テスト
			if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_Y )
			{
				DLPlayComm_Send_ConnectSign(childData->commSys_);
			}
			if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_X )
			{
				u8 i=0;
				DLPLAY_LARGE_PACKET pak;
#if 1
				sprintf(pak.data_[i++],"このデータはテストデータです。");
				sprintf(pak.data_[i++],"●『新ゲームデザイン』を読んで、良かったところはどこですか？");
				sprintf(pak.data_[i++],"　色々なゲームが面白いと思われる理由を、要素ごとに分解して書いてあるところ。");
				sprintf(pak.data_[i++],"　同じようなゲームでも、なぜ後のほうに出たものが面白くないのか？二番煎じと");
				sprintf(pak.data_[i++],"言われるのかという疑問があったのですが、『新ゲームデザイン』で格闘ゲームや");
				sprintf(pak.data_[i++],"シューティングで具体例を挙げて出しているので、とても納得できた。");
				sprintf(pak.data_[i++],"●『新ゲームデザイン』を読んで、不満だったところはどこですか？");
				sprintf(pak.data_[i++],"　参考にしているゲーム、市場が古いところ。");
				sprintf(pak.data_[i++],"　十年以上も前の本なので仕方ないとは思うが、やはり今と比べるとゲーム機のス");
				sprintf(pak.data_[i++],"ペックも市場の価値観も違うと思うので、今のゲーム・市場でこういった内容の本");
				sprintf(pak.data_[i++],"を読みたいと思う。");
				sprintf(pak.data_[i++],"●『新ゲームデザイン』から読み取れるゲームフリークらしさは何だと思いますか？");
				sprintf(pak.data_[i++],"　既存のゲームを模倣するのではなく、何か面白いことをゲームという形にして");
				sprintf(pak.data_[i++],"表現する事。");
				sprintf(pak.data_[i++],"●自分にとって、関心の高い書籍とは、どのようなものですか？");
				sprintf(pak.data_[i++],"　一つの事をトコトン追求している本。");
#else
				for( i=0;i<TEST_DATA_LINE;i++ )
				{
					u16 j;
					for( j=0;j<TEST_DATA_NUM;j++ )
					{
						pak.data_[i][j] =('A'+i);
					}
					pak.data_[i][TEST_DATA_NUM-1] = '\0';
				}
#endif
				DLPlayComm_Send_LargeData( &pak , childData->commSys_ );
			}

		break;

	case DCS_LOAD_BACKUP:
		{
			const BOOL ret = DLPlayData_LoadDataFirst( childData->dataSys_ );
			if( ret == TRUE ){ childData->mainSeq_ = DCS_SAVE_BACKUP; }
		}
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

	//BGモード設定
	GFL_BG_SetBGMode( &bgsysHeader );

	//ＢＧコントロール設定
	GFL_BG_SetBGControl( DLPLAY_MSG_PLANE, &bgCont3, GFL_BG_MODE_TEXT );
	GFL_BG_SetPriority( DLPLAY_MSG_PLANE, DLPLAY_MSG_PLANE_PRI );
	GFL_BG_SetVisible( DLPLAY_MSG_PLANE, VISIBLE_ON );

	//ビットマップウインドウシステムの起動
	GFL_BMPWIN_Init( HEAPID_ARIIZUMI_DEBUG );

}


