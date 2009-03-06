//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ミュージカルのステージProc
 * @author	ariizumi
 * @data	09/03/02
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#include "musical/musical_system.h"
#include "musical/musical_stage_sys.h"

#include "musical/stage/sta_acting.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================
enum	//メインシーケンス
{
	STA_SEQ_INIT_ACTING,	//ステージ演技部分
	STA_SEQ_INIT2_ACTING,	//エフェクトの読み込み用
	STA_SEQ_LOOP_ACTING,
	STA_SEQ_TERM_ACTING,
};

//======================================================================
//	typedef struct
//======================================================================

typedef struct
{
	ACTING_INIT_WORK	*actInitWork;
	ACTING_WORK			*actWork;
}STAGE_LOCAL_WORK;

//======================================================================
//	proto
//======================================================================

static GFL_PROC_RESULT MusicalStageProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalStageProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalStageProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

GFL_PROC_DATA MusicalStage_ProcData =
{
	MusicalStageProc_Init,
	MusicalStageProc_Main,
	MusicalStageProc_Term
};

//--------------------------------------------------------------
//	
//--------------------------------------------------------------

static GFL_PROC_RESULT MusicalStageProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	int ePos;
	STAGE_LOCAL_WORK *work;
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MUSICAL_STAGE, 0x60000 );

	work = GFL_PROC_AllocWork( proc, sizeof(STAGE_LOCAL_WORK), HEAPID_MUSICAL_STAGE );
	work->actInitWork = GFL_HEAP_AllocMemory( HEAPID_MUSICAL_STAGE , sizeof(ACTING_INIT_WORK) );
	work->actInitWork->heapId = HEAPID_MUSICAL_STAGE;
	work->actInitWork->musPoke[0].pokePara = PP_Create( MONSNO_PIKUSII , 20 , PTL_SETUP_POW_AUTO , HEAPID_MUSICAL_STAGE );
	work->actInitWork->musPoke[1].pokePara = PP_Create( MONSNO_RAITYUU , 20 , PTL_SETUP_POW_AUTO , HEAPID_MUSICAL_STAGE );
	work->actInitWork->musPoke[2].pokePara = PP_Create( MONSNO_EREBUU , 20 , PTL_SETUP_POW_AUTO , HEAPID_MUSICAL_STAGE );
	work->actInitWork->musPoke[3].pokePara = PP_Create( MONSNO_RUKARIO , 20 , PTL_SETUP_POW_AUTO , HEAPID_MUSICAL_STAGE );
	for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
	{
		work->actInitWork->musPoke[0].equip[ePos].type = MUSICAL_ITEM_INVALID;
		work->actInitWork->musPoke[1].equip[ePos].type = MUSICAL_ITEM_INVALID;
		work->actInitWork->musPoke[2].equip[ePos].type = MUSICAL_ITEM_INVALID;
		work->actInitWork->musPoke[3].equip[ePos].type = MUSICAL_ITEM_INVALID;
	}
	work->actInitWork->musPoke[0].equip[MUS_POKE_EQU_HAND_R].type = 13;
	work->actInitWork->musPoke[0].equip[MUS_POKE_EQU_HEAD].type = 16;
	
	work->actInitWork->musPoke[1].equip[MUS_POKE_EQU_EAR_L].type = 7;
	work->actInitWork->musPoke[1].equip[MUS_POKE_EQU_BODY].type = 9;
	
	work->actInitWork->musPoke[2].equip[MUS_POKE_EQU_HAND_R].type = 31;
	work->actInitWork->musPoke[2].equip[MUS_POKE_EQU_HEAD].type = 15;
	
	work->actInitWork->musPoke[3].equip[MUS_POKE_EQU_HAND_R].type = 30;
	work->actInitWork->musPoke[3].equip[MUS_POKE_EQU_HEAD].type = 21;
	
	
	return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MusicalStageProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	GFL_HEAP_DeleteHeap( HEAPID_MUSICAL_STAGE );

	return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MusicalStageProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	STAGE_LOCAL_WORK *work = mywk;
	switch( *seq )
	{
	case STA_SEQ_INIT_ACTING:
		work->actWork = STA_ACT_InitActing( work->actInitWork );
		*seq = STA_SEQ_INIT2_ACTING;
		break;
		
	case STA_SEQ_INIT2_ACTING:
//		STA_ACT_InitActing_2nd( work->actWork );
		*seq = STA_SEQ_LOOP_ACTING;
		break;
		
	case STA_SEQ_LOOP_ACTING:
		{
			ACTING_RETURN ret;
			ret = STA_ACT_LoopActing( work->actWork );
			if( ret == ACT_RET_GO_END )
			{
				*seq = STA_SEQ_TERM_ACTING;
			}
		}
		break;

	case STA_SEQ_TERM_ACTING:
		STA_ACT_TermActing( work->actWork );
		return GFL_PROC_RES_FINISH;
		break;
	}
	return GFL_PROC_RES_CONTINUE;
}

