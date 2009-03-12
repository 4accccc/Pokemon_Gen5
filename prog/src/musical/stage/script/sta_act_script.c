//======================================================================
/**
 * @file	sta_act_script.c
 * @brief	ステージ スクリプト処理
 * @author	ariizumi
 * @data	09/03/06
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "sta_act_script.h"
#include "sta_act_script_def.h"
#include "script_table.h"
//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define SCRIPT_TCB_TASK_NUM (16)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static BOOL STA_SCRIPT_UpdateScript( STA_SCRIPT_SYS *work , STA_SCRIPT_WORK *scriptWork );

//--------------------------------------------------------------
//	
//--------------------------------------------------------------

STA_SCRIPT_SYS* STA_SCRIPT_InitSystem( HEAPID heapId ,ACTING_WORK *actWork)
{
	u8 i;
	STA_SCRIPT_SYS *work;
	work = GFL_HEAP_AllocMemory( heapId , sizeof( STA_SCRIPT_SYS ) );
	
	work->heapId = heapId;
	work->actWork = actWork;
	work->tcbWork = GFL_HEAP_AllocMemory( work->heapId , GFL_TCB_CalcSystemWorkSize(SCRIPT_TCB_TASK_NUM) );
	work->tcbSys = GFL_TCB_Init( SCRIPT_TCB_TASK_NUM , work->tcbWork );
	
	for( i=0;i<SCRIPT_NUM;i++ )
	{
		work->scriptWork[i] = NULL;
	}
	
	return work;
}

void STA_SCRIPT_ExitSystem( STA_SCRIPT_SYS *work )
{
	u8 i;
	for( i=0;i<SCRIPT_NUM;i++ )
	{
		if( work->scriptWork[i] != NULL )
		{
			GFL_HEAP_FreeMemory( work->scriptWork[i] );
			work->scriptWork[i] = NULL;
		}
	}

	GFL_TCB_Exit( work->tcbSys );
	GFL_HEAP_FreeMemory( work->tcbWork );
	GFL_HEAP_FreeMemory( work );
}

void STA_SCRIPT_UpdateSystem( STA_SCRIPT_SYS *work )
{
	u8 i;
	for( i=0;i<SCRIPT_NUM;i++ )
	{
		if( work->scriptWork[i] != NULL )
		{
			const BOOL ret = STA_SCRIPT_UpdateScript( work , work->scriptWork[i] );
			if( ret == TRUE )
			{
				GFL_HEAP_FreeMemory( work->scriptWork[i] );
				work->scriptWork[i] = NULL;
			}
		}
	}
	GFL_TCB_Main( work->tcbSys );
}

void STA_SCRIPT_SetScript( STA_SCRIPT_SYS *work , void *scriptData )
{
	u8 i;
	for( i=0;i<SCRIPT_NUM;i++ )
	{
		if( work->scriptWork[i] == NULL )
		{
			break;
		}
	}

	GF_ASSERT_MSG( i<SCRIPT_NUM , "Stage script is full!!\n" );
	
	work->scriptWork[i] = GFL_HEAP_AllocMemory( work->heapId , sizeof( STA_SCRIPT_WORK ) );
	work->scriptWork[i]->scriptData = scriptData;
	work->scriptWork[i]->loadPos = work->scriptWork[i]->scriptData;
	work->scriptWork[i]->waitCnt = 0;
	work->scriptWork[i]->frame = 0;
}

static BOOL STA_SCRIPT_UpdateScript( STA_SCRIPT_SYS *work , STA_SCRIPT_WORK *scriptWork )
{
	SCRIPT_FINISH_TYPE ret = SFT_CONTINUE;
	if( scriptWork->waitCnt > 0 )
	{
		scriptWork->waitCnt--;
	}

	if( scriptWork->waitCnt == 0 )
	{
		while( ret == SFT_CONTINUE )
		{
			s32 *scriptNo = scriptWork->loadPos;
			((s32*)scriptWork->loadPos)++;
			GF_ASSERT_MSG( *scriptNo < SCRIPT_ENUM_MAX , "ScriptNo[%d] is invalid!!\n",*scriptNo);
			ret = STA_ScriptFuncArr[*scriptNo](work,scriptWork);
		}
		if( ret == SFT_END )
		{
			//FIXME ここでスクリプトデータをFreeすべきか？
			//今はファイル読み込みではないためで開放できず・・・
	//		GFL_HEAP_FreeMemory( scriptWork->scriptData );
			return TRUE;
		}
	}
	scriptWork->frame++;
	return FALSE;
}

