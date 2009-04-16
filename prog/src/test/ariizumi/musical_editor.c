//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ミュージカルのエディタ
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

#include "test/soga/gf_mcs.h"

//======================================================================
//	define
//======================================================================
#pragma mark [>define
#define	HEAD_SIZE			( 4 )

#define	MCS_READ_CH			( 0 )
#define	MCS_WRITE_CH		( 0 )

//======================================================================
//	enum
//======================================================================
#pragma mark [>enum
enum	//メインシーケンス
{
	STA_SEQ_INIT_ACTING,	//ステージ演技部分
	STA_SEQ_LOOP_ACTING,
	STA_SEQ_TERM_ACTING,
};

//MCS用定義
enum
{
	SEND_SEQUENCE = 1,
	SEND_SEQUENCE_OK,
	SEND_RESOURCE,
	SEND_RESOURCE_OK,
	SEND_RECEIVE,
	SEND_RECEIVE_OK,
	SEND_DECIDE,
	SEND_CANCEL,
	SEND_IDLE,
};

//MCS用Seq
typedef enum
{
	MSEQ_WAIT,
	MSEQ_LOAD_SEQ,
	MSEQ_LOAD_RES,
	MSEQ_SEND,
}MSC_SEQ;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [>struct
typedef struct
{
	ACTING_INIT_WORK	*actInitWork;
	ACTING_WORK			*actWork;
	
	MSC_SEQ				mcsSeq;
	void				*scriptData;
}MUS_EDIT_LOCAL_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [>prot
static GFL_PROC_RESULT MusicalEditProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalEditProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalEditProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

static void MusicalEdit_McsMain( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_McsWait( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_McsLoadSeq( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_McsLoadRes( MUS_EDIT_LOCAL_WORK *work );

GFL_PROC_DATA MusicalEdit_ProcData =
{
	MusicalEditProc_Init,
	MusicalEditProc_Main,
	MusicalEditProc_Term
};

FS_EXTERN_OVERLAY(sogabe_debug);

//--------------------------------------------------------------
//	
//--------------------------------------------------------------

static GFL_PROC_RESULT MusicalEditProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	int ePos;
	MUS_EDIT_LOCAL_WORK *work;
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MUSICAL_STAGE, 0x100000 );

	work = GFL_PROC_AllocWork( proc, sizeof(MUS_EDIT_LOCAL_WORK), HEAPID_MUSICAL_STAGE );
	work->actInitWork = GFL_HEAP_AllocMemory( HEAPID_MUSICAL_STAGE , sizeof(ACTING_INIT_WORK) );
	work->actInitWork->heapId = HEAPID_MUSICAL_STAGE;
	work->actInitWork->musPoke[0].pokePara = PP_Create( MONSNO_PIKUSII , 20 , PTL_SETUP_POW_AUTO , HEAPID_MUSICAL_STAGE );
	work->actInitWork->musPoke[1].pokePara = PP_Create( MONSNO_RAITYUU , 20 , PTL_SETUP_POW_AUTO , HEAPID_MUSICAL_STAGE );
	work->actInitWork->musPoke[2].pokePara = PP_Create( MONSNO_EREBUU , 20 , PTL_SETUP_POW_AUTO , HEAPID_MUSICAL_STAGE );
	work->actInitWork->musPoke[3].pokePara = PP_Create( MONSNO_RUKARIO , 20 , PTL_SETUP_POW_AUTO , HEAPID_MUSICAL_STAGE );
	for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
	{
		work->actInitWork->musPoke[0].equip[ePos].itemNo = MUSICAL_ITEM_INVALID;
		work->actInitWork->musPoke[1].equip[ePos].itemNo = MUSICAL_ITEM_INVALID;
		work->actInitWork->musPoke[2].equip[ePos].itemNo = MUSICAL_ITEM_INVALID;
		work->actInitWork->musPoke[3].equip[ePos].itemNo = MUSICAL_ITEM_INVALID;
	}
	
	work->actInitWork->musPoke[0].equip[MUS_POKE_EQU_HAND_R].itemNo = 13;
	work->actInitWork->musPoke[0].equip[MUS_POKE_EQU_HEAD].itemNo = 16;
	
	work->actInitWork->musPoke[1].equip[MUS_POKE_EQU_EAR_L].itemNo = 7;
	work->actInitWork->musPoke[1].equip[MUS_POKE_EQU_BODY].itemNo = 9;
	
	work->actInitWork->musPoke[2].equip[MUS_POKE_EQU_HAND_R].itemNo = 31;
	work->actInitWork->musPoke[2].equip[MUS_POKE_EQU_HEAD].itemNo = 15;
	
	work->actInitWork->musPoke[3].equip[MUS_POKE_EQU_HAND_R].itemNo = 30;
	work->actInitWork->musPoke[3].equip[MUS_POKE_EQU_HEAD].itemNo = 21;

/*
	for( ePos=0;ePos<4;ePos++ )
	{
		work->actInitWork->musPoke[ePos].equip[MUS_POKE_EQU_HAND_R].type = 30;
		work->actInitWork->musPoke[ePos].equip[MUS_POKE_EQU_HAND_L].type = 30;
		work->actInitWork->musPoke[ePos].equip[MUS_POKE_EQU_HEAD].type = 30;
		work->actInitWork->musPoke[ePos].equip[MUS_POKE_EQU_EAR_L].type = 30;
		work->actInitWork->musPoke[ePos].equip[MUS_POKE_EQU_BODY].type = 30;
	}
*/	

	//mcs用初期化
	work->mcsSeq = MSEQ_WAIT;
	GFL_OVERLAY_Load(FS_OVERLAY_ID(sogabe_debug));
	return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MusicalEditProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	MUS_EDIT_LOCAL_WORK *work = mywk;
	

	GFL_OVERLAY_Unload(FS_OVERLAY_ID(sogabe_debug));
	
	GFL_HEAP_FreeMemory( work->actInitWork->musPoke[0].pokePara );
	GFL_HEAP_FreeMemory( work->actInitWork->musPoke[1].pokePara );
	GFL_HEAP_FreeMemory( work->actInitWork->musPoke[2].pokePara );
	GFL_HEAP_FreeMemory( work->actInitWork->musPoke[3].pokePara );
	GFL_HEAP_FreeMemory( work->actInitWork );
	GFL_PROC_FreeWork( proc );
	GFL_HEAP_DeleteHeap( HEAPID_MUSICAL_STAGE );

	return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MusicalEditProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	MUS_EDIT_LOCAL_WORK *work = mywk;
	switch( *seq )
	{
	case STA_SEQ_INIT_ACTING:
		work->actWork = STA_ACT_InitActing( work->actInitWork );
		MCS_Init( HEAPID_MUSICAL_STAGE );
		*seq = STA_SEQ_LOOP_ACTING;
		break;
		
	case STA_SEQ_LOOP_ACTING:
		{
			ACTING_RETURN ret;
			ret = STA_ACT_LoopActing( work->actWork );
			MusicalEdit_McsMain( work );
			if( ret == ACT_RET_GO_END )
			{
				*seq = STA_SEQ_TERM_ACTING;
			}
		}
		break;

	case STA_SEQ_TERM_ACTING:
		STA_ACT_TermActing( work->actWork );
		MCS_Exit();
		return GFL_PROC_RES_FINISH;
		break;
	}
	
	return GFL_PROC_RES_CONTINUE;
}

static void MusicalEdit_McsMain( MUS_EDIT_LOCAL_WORK *work )
{
	u32 size;
	MCS_Main();

	switch(work->mcsSeq)
	{
	case MSEQ_WAIT:
		MusicalEdit_McsWait( work );
		break;
	case MSEQ_LOAD_SEQ:
		MusicalEdit_McsLoadSeq( work );
		break;
	case MSEQ_LOAD_RES:
	case MSEQ_SEND:
		MusicalEdit_McsLoadRes( work );
		break;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
	{
		STA_ACT_EDITOR_StartScript( work->actWork );
	}
}

static void MusicalEdit_McsWait( MUS_EDIT_LOCAL_WORK *work )
{
	const u32 size = MCS_CheckRead();
	if( size > 0 )
	{
		u32	head;
		MCS_Read( &head, HEAD_SIZE );
		if( head == SEND_SEQUENCE )
		{
			work->mcsSeq = MSEQ_LOAD_SEQ;
			head = SEND_SEQUENCE_OK;
			MCS_Write( MCS_WRITE_CH, &head, 4 );
			OS_TPrintf("sequence_data head:%d\n",size);
		}
		else if( head == SEND_RESOURCE )
		{
			work->mcsSeq = MSEQ_LOAD_RES;
			head = SEND_RESOURCE_OK;
			MCS_Write( MCS_WRITE_CH, &head, 4 );
			OS_TPrintf("resource_data head:%d\n",size);
		}
		else if( head == SEND_RECEIVE )
		{
			work->mcsSeq = MSEQ_SEND;
//				evw->sub_seq_no = SUB_SEQ_INIT;
//				evw->answer = 0;
			head = SEND_RECEIVE_OK;
			MCS_Write( MCS_WRITE_CH, &head, 4 );
			OS_TPrintf("receive head:%d\n",size);
		}
	}
}
static void MusicalEdit_McsLoadSeq( MUS_EDIT_LOCAL_WORK *work )
{
	const u32 size = MCS_CheckRead();
	if( size )
	{
		u32	head;
		void *scriptData = GFL_HEAP_AllocClearMemory( HEAPID_MUSICAL_STAGE, size );
		MCS_Read( scriptData, size );
		STA_ACT_EDITOR_SetScript( work->actWork , scriptData );
		head = SEND_IDLE;
		MCS_Write( MCS_WRITE_CH, &head, 4 );
		work->mcsSeq = MSEQ_WAIT;
		OS_TPrintf("sequence_data load:%d\n",size);
		
	}
	
}

static void MusicalEdit_McsLoadRes( MUS_EDIT_LOCAL_WORK *work )
{
	//ダミー読み込み
	const u32	size = MCS_CheckRead();

	if( size )
	{
		u32	head;
		void* data;
		data = GFL_HEAP_AllocClearMemory( HEAPID_MUSICAL_STAGE, size );
		MCS_Read( data, size );
		head = SEND_IDLE;
		MCS_Write( MCS_WRITE_CH, &head, 4 );
		work->mcsSeq = MSEQ_WAIT;

		GFL_HEAP_FreeMemory( data );
		OS_TPrintf("dummy load:%d\n",size);
	}
}
