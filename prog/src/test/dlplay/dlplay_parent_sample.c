//============================================================================================
/**
 * @file	dlplay_parent_sample.c
 * @brief	ダウンロードプレイサンプル
 * @author	ariizumi
 * @date	2008.10.8
 */
//============================================================================================
#include "gflib.h"
#include "procsys.h"
#include "ui.h"
#include "system/main.h"

#include "net.h"

#include "test/ariizumi/ari_debug.h"
#include "dlplay_parent_sample.h"
#include "dlplay_parent_main.h"


//------------------------------------------------------------------
//	typedef struct
//------------------------------------------------------------------
typedef struct
{
	int heapID_;

	u8	mainState_;
	u8	nextState_;
	u8	subState_;
	BOOL	isInitIchneumon_;
	DLPLAY_SEND_DATA *dlData_;

}DLPLAY_PARENT_DATA;

//------------------------------------------------------------------
//	local
//------------------------------------------------------------------
DLPLAY_PARENT_DATA *parentData;



//BG面定義
#define DLPLAY_MSG_PLANE			(GFL_BG_FRAME3_M)
#define DLPLAY_MSG_PLANE_PRI		(0)
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

void	DLPlay_InitLibCallBack( void* pWork , BOOL isSuccess );

//------------------------------------------------------------------
//  デバッグ用初期化関数
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugDLPlayMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ARIIZUMI_DEBUG, 0x200000 );

	parentData = GFL_HEAP_AllocClearMemory( HEAPID_ARIIZUMI_DEBUG, sizeof( DLPLAY_PARENT_DATA ) );
	parentData->heapID_ = HEAPID_ARIIZUMI_DEBUG;
	parentData->mainState_ = DPM_SEND_IMAGE;
	parentData->nextState_ = DPM_SEND_IMAGE;
	parentData->subState_ = 0;
	parentData->isInitIchneumon_ = FALSE;

	
	//VRAM系初期化
	//BGシステム起動
	GFL_BG_Init( parentData->heapID_ );

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
	GFL_BMPWIN_Init( parentData->heapID_ );

//	GFL_NET_InitIchneumon( DLPlay_InitLibCallBack, (void*)(parentData) );
	
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**  デバッグ用Exit
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugDLPlayMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GFL_BMPWIN_Exit();
	GFL_BG_FreeBGControl( DLPLAY_MSG_PLANE );
	GFL_BG_Exit();

	GFL_HEAP_DeleteHeap( HEAPID_ARIIZUMI_DEBUG );

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**   デバッグ用メイン
 */  
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugDLPlayMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	switch( parentData->mainState_ )
	{
	case DPM_SEND_IMAGE:

		if( parentData->subState_ == 0 ){
			//if( parentData->isInitIchneumon_ == TRUE ){
				parentData->dlData_ = DLPlaySend_Init( parentData->heapID_ );
				parentData->subState_++;
			//}
		}
		else{
			parentData->nextState_ = DLPlaySend_Loop( parentData->dlData_ );
			
			if( parentData->nextState_ != parentData->mainState_ ){
				DLPlaySend_Term( parentData->dlData_ );
				parentData->mainState_ = parentData->nextState_;
				parentData->subState_ = 0;
			}
		}
		break;

	case DPM_SELECT_BOX:
		break;
	
	case DPM_ERROR:
		OS_TPrintf("Error state!!\n");
		break;

	case DPM_END:
		return GFL_PROC_RES_FINISH;
		break;

	}

	if( FALSE ){
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugDLPlayMainProcData = {
	DebugDLPlayMainProcInit,
	DebugDLPlayMainProcMain,
	DebugDLPlayMainProcEnd,
};

void	DLPlay_InitLibCallBack( void* pWork , BOOL isSuccess )
{
	DLPLAY_PARENT_DATA *dlData = (DLPLAY_PARENT_DATA*)pWork;

	if( isSuccess == FALSE ){
		GF_ASSERT("Init ichneumon is failued!!\n");
	}

	dlData->isInitIchneumon_ = TRUE;
	OS_TPrintf("DLPlay init ichunumon\n");
}

