//======================================================================
/**
 * @file	field_proc.c
 * @author	tamada
 * @date	2008.11.18
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"	//HEAPID参照のため

#include "gamesystem/gamesystem.h"
#include "field/fieldmap_proc.h"
#include "field/fieldmap_call.h"  //FIELDMAP_Create/FIELDMAP_Main/FIELDMAP_Delete

#include "field/zonedata.h" //ZONEDATA_IsUnionRoom
//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	FIELDPROC_WORK
//--------------------------------------------------------------
typedef struct {
	FIELDMAP_WORK *fieldWork;
}FIELDPROC_WORK;

//======================================================================
//	proto
//======================================================================
const GFL_PROC_DATA FieldProcData;

//======================================================================
//	
//======================================================================
#ifdef PM_DEBUG
extern BOOL DebugScanOnly;
#endif


//======================================================================
//	フィールドプロセス
//======================================================================
//------------------------------------------------------------------
/**
 * @brief	プロセスの初期化
 * ここでヒープの生成や各種初期化処理を行う。
 * 初期段階ではmywkはNULLだが、GFL_PROC_AllocWorkを使用すると
 * 以降は確保したワークのアドレスとなる。
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT FieldMapProcInit
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	FIELDMAP_WORK * fieldWork;
	FIELDPROC_WORK * fpwk;
	GAMESYS_WORK * gsys = pwk;
  u32 heap_size;

  switch(*seq){
  case 0:
    {
      u16 zone_id = PLAYERWORK_getZoneID( GAMESYSTEM_GetMyPlayerWork(gsys) );
      if (ZONEDATA_IsUnionRoom(zone_id) || ZONEDATA_IsColosseum(zone_id) )
      {
        heap_size = 0x140000 - 0x18000;
      }
      else if ( ZONEDATA_IsMusicalWaitingRoom(zone_id) )
      {
        heap_size = 0xC0000 - 0x18000;
      }
      else
      {
        heap_size = 0x148000 - 0x18000;
      }
    }
  	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_FIELDMAP, heap_size );
  	fpwk = GFL_PROC_AllocWork(proc, sizeof(FIELDPROC_WORK), HEAPID_FIELDMAP);
  	fpwk->fieldWork = FIELDMAP_Create(gsys, HEAPID_FIELDMAP );
  	GAMESYSTEM_SetFieldMapWork(gsys, fpwk->fieldWork);
  	(*seq)++;
  	return GFL_PROC_RES_CONTINUE;
  
  default:
    (*seq)++;
  	return GFL_PROC_RES_CONTINUE;
  case 10:   //@todo ※check　超暫定 早く通信ONにすると停止してしまう
    //常時通信モード
    {
      GAME_COMM_SYS_PTR gcsp = GAMESYSTEM_GetGameCommSysPtr(gsys);
      u16 zone_id = PLAYERWORK_getZoneID( GAMESYSTEM_GetMyPlayerWork(gsys) );
      if(ZONEDATA_IsUnionRoom(zone_id) == FALSE && ZONEDATA_IsColosseum(zone_id) == FALSE){
        if(GAMESYSTEM_GetAlwaysNetFlag(gsys) == TRUE){
          if(GameCommSys_BootCheck(gcsp) == GAME_COMM_NO_NULL){
            OS_TPrintf("常時通信モードの為、サーチ起動\n");
            GameCommSys_Boot(gcsp, GAME_COMM_NO_FIELD_BEACON_SEARCH, gsys);
          }
        }
      #ifdef PM_DEBUG
        else if(DebugScanOnly == TRUE && GameCommSys_BootCheck(gcsp) == GAME_COMM_NO_NULL){
          OS_TPrintf("非通信モードでDebugScanがONの為、ビーコンScanOnly起動\n");
          GameCommSys_Boot(gcsp, GAME_COMM_NO_DEBUG_SCANONLY, gsys);
        }
      #endif
        else{
          if(GameCommSys_BootCheck(gcsp) == GAME_COMM_NO_FIELD_BEACON_SEARCH){
            OS_TPrintf("非通信モードの為、サーチ終了\n");
            GameCommSys_ExitReq(gcsp);
          }
        }
      }
    }
    break;
  }
  
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	プロセスのメイン
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT FieldMapProcMain
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GAMESYS_WORK * gsys = pwk;
	FIELDPROC_WORK * fpwk = mywk;
	if( FIELDMAP_Main(gsys, fpwk->fieldWork) == TRUE ){
		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief	プロセスの終了処理
 *
 * 単に終了した場合、親プロセスに処理が返る。
 * GFL_PROC_SysSetNextProcを呼んでおくと、終了後そのプロセスに
 * 処理が遷移する。
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT FieldMapProcEnd
				( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GAMESYS_WORK * gsys = pwk;
	FIELDPROC_WORK * fpwk = mywk;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  
  //※check　フィールドが無い間はビーコンサーチしない
  if(GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_FIELD_BEACON_SEARCH
      || GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_DEBUG_SCANONLY){
  //  GameCommSys_ExitReq(game_comm);
  }

	FIELDMAP_Delete(fpwk->fieldWork);
	GAMESYSTEM_SetFieldMapWork(gsys, NULL);
	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap( HEAPID_FIELDMAP );

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
///	フィールドプロセス　データ
//------------------------------------------------------------------
const GFL_PROC_DATA FieldProcData = {
	FieldMapProcInit,
	FieldMapProcMain,
	FieldMapProcEnd,
};
