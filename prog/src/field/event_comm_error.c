//==============================================================================
/**
 * @file    event_comm_error.c
 * @brief   フィールドでの通信エラー画面呼び出し
 * @author  matsuda
 * @date    2009.09.14(月)
 */
//==============================================================================
#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"

#include "field/fieldmap.h"
#include "gamesystem\gamesystem.h"

#include "./event_fieldmap_control.h"
#include "event_comm_error.h"
#include "system/net_err.h"
#include "field/game_beacon_search.h"
#include "field/fieldmap_call.h"  //FIELDMAP_IsReady


//==============================================================================
//  
//==============================================================================
FS_EXTERN_OVERLAY(fieldmap);

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static GFL_PROC_RESULT FieldCommErrorProc_Init(GFL_PROC * proc, int * seq , void *pwk, void *mywk);
static GFL_PROC_RESULT FieldCommErrorProc_Main(GFL_PROC * proc, int * seq , void *pwk, void *mywk);
static GFL_PROC_RESULT FieldCommErrorProc_Exit(GFL_PROC * proc, int * seq , void *pwk, void *mywk);
static GMEVENT_RESULT _EventCommErrorExit( GMEVENT *event, int *seq, void *wk );
static GMEVENT_RESULT _EventCommError_Manage( GMEVENT *event, int *seq, void *wk );

//==============================================================================
//  データ
//==============================================================================
static const GFL_PROC_DATA FieldCommErrorProc = {
  FieldCommErrorProc_Init,
  FieldCommErrorProc_Main,
  FieldCommErrorProc_Exit,
};



//==================================================================
/**
 * フィールドでの通信エラー画面呼び出しイベント
 *
 * @param   gsys		
 * @param   fieldmap		
 *
 * @retval  GMEVENT *		
 */
//==================================================================
GMEVENT * EVENT_FieldCommErrorProc(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap)
{
  return GMEVENT_Create(gsys, NULL, _EventCommError_Manage, 0);
}

//--------------------------------------------------------------
/**
 * 
 *
 * @param   event		
 * @param   seq		
 * @param   wk		
 *
 * @retval  GMEVENT_RESULT		
 */
//--------------------------------------------------------------
static GMEVENT_RESULT _EventCommError_Manage( GMEVENT *event, int *seq, void *wk )
{
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);

  switch(*seq){
  case 0:
    MMDLSYS_PauseMoveProc( FIELDMAP_GetMMdlSys( fieldWork ) );
    
    {
      GAME_COMM_NO comm_no = GameCommSys_GetLastCommNo(GAMESYSTEM_GetGameCommSysPtr(gsys));
      GMEVENT *child_event;
      
      if(comm_no == GAME_COMM_NO_INVASION){
        if(GAMEDATA_GetIntrudeReverseArea(GAMESYSTEM_GetGameData(gsys)) == TRUE){
          //裏フィールド侵入中はエラーを出す
          NetErr_App_ReqErrorDisp();
          child_event = EVENT_FieldSubProc(
            gsys, fieldWork, FS_OVERLAY_ID(fieldmap), &FieldCommErrorProc, gsys);
        }
        else{
        	child_event = GMEVENT_Create( gsys, NULL, _EventCommErrorExit, 0 );
        }
      }
      else{ //侵入以外では通常通りエラー画面を表示
        NetErr_App_ReqErrorDisp();
        child_event = EVENT_FieldSubProc(
          gsys, fieldWork, FS_OVERLAY_ID(fieldmap), &FieldCommErrorProc, gsys);
      }
      
      GMEVENT_CallEvent(event, child_event);
    }
    
    (*seq)++;
    break;
  case 1:
    if(fieldWork != NULL && FIELDMAP_IsReady(fieldWork) == TRUE){
   	  MMDLSYS_ClearPauseMoveProc(FIELDMAP_GetMMdlSys(fieldWork));
      return GMEVENT_RES_FINISH;
    }
    break;
  }
  
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 通信エラー画面を出さずに、通信終了し、エラー情報も初期化するイベント
 * @param	event	GMEVENT
 * @param	seq		シーケンス
 * @param	wk		event dis_wk
 */
//--------------------------------------------------------------
static GMEVENT_RESULT _EventCommErrorExit( GMEVENT *event, int *seq, void *wk )
{
	GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
	GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
	
  OS_TPrintf("_EventCommErrorExit seq=%d\n", *seq);
	switch( *seq ){
	case 0:
    if(GFL_NET_IsInit() )
    { 
      GFL_NET_Exit(NULL);
    }
    (*seq)++;
    break;
  case 1:
    if(GFL_NET_IsExit() == TRUE){
      NetErr_ErrWorkInit();
      GAMESYSTEM_SetFieldCommErrorReq(gsys, FALSE);
      (*seq)++;
    }
    break;
  case 2:
    if(GAMESYSTEM_CommBootAlways( gsys ) == TRUE){  //再び常時通信を起動
      (*seq)++;
    }
    else{
      return GMEVENT_RES_FINISH;
    }
    break;
  case 3:
    {//常時通信を起動出来たならばウェイトをセット
      GAME_BEACON_SYS_PTR gbs_ptr = GameCommSys_GetAppWork(game_comm);
      if ( gbs_ptr != NULL && GameCommSys_BootCheck( game_comm ) != GAME_COMM_NO_NULL ){
        GameBeacon_SetErrorWait(gbs_ptr);
        return GMEVENT_RES_FINISH;
      }
    }
    break;
	}
	
	return GMEVENT_RES_CONTINUE;
}

//==============================================================================
//
//  ダミーPROC
//      PROC遷移でエラー画面を表示させる為だけのダミーPROC
//
//==============================================================================
static GFL_PROC_RESULT FieldCommErrorProc_Init(GFL_PROC * proc, int * seq , void *pwk, void *mywk)
{
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT FieldCommErrorProc_Main(GFL_PROC * proc, int * seq , void *pwk, void *mywk)
{
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT FieldCommErrorProc_Exit(GFL_PROC * proc, int * seq , void *pwk, void *mywk)
{
  GAMESYS_WORK *gsys = pwk;
  
  GAMESYSTEM_SetFieldCommErrorReq(gsys, FALSE);
  return GFL_PROC_RES_FINISH;
}
