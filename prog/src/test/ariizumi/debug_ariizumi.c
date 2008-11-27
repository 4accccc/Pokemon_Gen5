//============================================================================================
/**
 * @file	debug_ariizumi.c
 * @brief	デバッグ 有泉
 * @author	ariizumi
 * @date	2008.10.8
 */
//============================================================================================
#include "gflib.h"
#include "procsys.h"
#include "system/main.h"

#include "ari_comm_card.h"
#include "gamesystem/game_init.h"

typedef struct
{
	ARI_COMM_CARD_WORK *cardWork_;
}DEBUG_ARIIZUMI_WORK;

DEBUG_ARIIZUMI_WORK *debWork = NULL;

//------------------------------------------------------------------
//  デバッグ用初期化関数
//------------------------------------------------------------------

extern void	AriFieldBoot( HEAPID heapID );
extern void	AriFieldEnd( void );
extern BOOL	AriFieldMain( void );

static GFL_PROC_RESULT DebugAriizumiMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ARIIZUMI_DEBUG, 0x200000 );
//	AriFieldBoot( HEAPID_WATANABE_DEBUG );

	debWork = GFL_HEAP_AllocMemory( HEAPID_ARIIZUMI_DEBUG , sizeof(DEBUG_ARIIZUMI_WORK));
	debWork->cardWork_ = ARI_COMM_CARD_Init( HEAPID_ARIIZUMI_DEBUG );

	
	return GFL_PROC_RES_FINISH;
}


//------------------------------------------------------------------
/**  デバッグ用Exit
 */
//------------------------------------------------------------------
extern const GFL_PROC_DATA FieldMapProcData;
static GFL_PROC_RESULT DebugAriizumiMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
//	AriFieldEnd();
	ARI_COMM_CARD_Term( debWork->cardWork_ );
	GFL_HEAP_FreeMemory( debWork );
	debWork = NULL;

	GFL_HEAP_DeleteHeap( HEAPID_ARIIZUMI_DEBUG );
	
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**   デバッグ用メイン
 */  
//------------------------------------------------------------------
static GFL_PROC_RESULT DebugAriizumiMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
//	if( AriFieldMain() == TRUE ){
//		return GFL_PROC_RES_FINISH;
//	}

	if( ARI_COMM_CARD_Loop( debWork->cardWork_ ) == TRUE ){
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
const GFL_PROC_DATA DebugAriizumiMainProcData = {
	DebugAriizumiMainProcInit,
	DebugAriizumiMainProcMain,
	DebugAriizumiMainProcEnd,
};

