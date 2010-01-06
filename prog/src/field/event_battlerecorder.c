//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		event_battlerecorder.c
 *	@brief  バトルレコーダーイベント
 *	@author	Toru=Nagihashi
 *	@data		2009.12.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"      //GFL_HEAPID_APP参照

//フィールド
#include "event_fieldmap_control.h"

//PROC
#include "net_app/wifi_login.h"

//外部公開
#include "field/event_battlerecorder.h"


FS_EXTERN_OVERLAY( battle_recorder );
//============================================================================================
//
//    定数
//
//============================================================================================
//-------------------------------------
///	シーケンス
//=====================================
enum{
  _FIELD_COMM,
  _FIELD_CLOSE,
  _CALL_WIFILOGIN,
  _WAIT_WIFILOGIN,
  _CALL_BR,
  _WAIT_BR,
  _WAIT_NET_END,
  _FIELD_OPEN,
  _FIELD_END
};

//-------------------------------------
///	イベントワーク
//=====================================
typedef struct 
{
  GAMESYS_WORK      * gsys;
  FIELDMAP_WORK     * fieldmap;
  void              * p_sub_wk;
  BR_MODE           mode;
  BtlRule btl_rule;
} EVENT_BR_WORK;


//============================================================================================
//
//    サブイベント
//
//============================================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  イベントメイン処理
 *
 *	@param	* event イベント
 *	@param	*  seq  シーケンス
 *	@param	* work  自分で渡したアドレス
 *
 *	@return 終了コード
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT EVENT_BRMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_BR_WORK * dbw = work;
  GAMESYS_WORK * gsys = dbw->gsys;


  switch (*seq) {
  case _FIELD_COMM:
    if(GAME_COMM_NO_NULL == GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gsys)))
    {
      (*seq)++;
    }
    break;
  case _FIELD_CLOSE:
    GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, dbw->fieldmap));
   // (*seq)++; //@todo
    (*seq)  = _CALL_BR;
    break;
  case _CALL_WIFILOGIN:
    { 
      WIFILOGIN_PARAM *p_param;
      dbw->p_sub_wk = GFL_HEAP_AllocClearMemory(HEAPID_PROC,sizeof(WIFILOGIN_PARAM));
      p_param = dbw->p_sub_wk;
      p_param->gamedata = GAMESYSTEM_GetGameData(gsys);
      GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(wifi_login), &WiFiLogin_ProcData, dbw->p_sub_wk);
    }
    (*seq)++;
    break;
  case _WAIT_WIFILOGIN:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){

      WIFILOGIN_PARAM *p_param  = dbw->p_sub_wk;
      if( p_param->result == WIFILOGIN_RESULT_LOGIN )
      { 
        (*seq) ++;
      }
      else if( p_param->result == WIFILOGIN_RESULT_CANCEL )
      { 
        (*seq)  = _WAIT_NET_END;
      }
      GFL_HEAP_FreeMemory( dbw->p_sub_wk );
    }
    break;
  case _CALL_BR:
    { 
      BATTLERECORDER_PARAM *p_param;
      dbw->p_sub_wk = GFL_HEAP_AllocClearMemory(HEAPID_PROC,sizeof(BATTLERECORDER_PARAM));
      p_param = dbw->p_sub_wk;
      p_param->mode   = dbw->mode;
      p_param->p_gamedata  = GAMESYSTEM_GetGameData(gsys);
      GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(battle_recorder), &BattleRecorder_ProcData, dbw->p_sub_wk );
    }
    (*seq)++;
    break;
  case _WAIT_BR:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){
      GFL_HEAP_FreeMemory( dbw->p_sub_wk );
      (*seq) ++;
    }
    break;
  case _WAIT_NET_END:
      (*seq) ++;
    break;
  case _FIELD_OPEN:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;
  case _FIELD_END:
    return GMEVENT_RES_FINISH;
  default:
    GF_ASSERT(0);
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  WiFi世界対戦コール
 *
 *	@param	GAMESYS_WORK * gsys ゲームシステム
 *	@param	* fieldmap          フィールドマップ
 */
//-----------------------------------------------------------------------------
GMEVENT* EVENT_WifiBattleRecorder( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap, BR_MODE mode )
{
  GMEVENT * event = GMEVENT_Create(gsys, NULL, EVENT_BRMain, sizeof(EVENT_BR_WORK));
  if(GAME_COMM_NO_NULL!= GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gsys))){
    GameCommSys_ExitReq(GAMESYSTEM_GetGameCommSysPtr(gsys));
  }

  { 
    EVENT_BR_WORK * dbw;

    dbw = GMEVENT_GetEventWork(event);
    dbw->gsys = gsys;
    dbw->fieldmap = fieldmap;
    dbw->mode = mode;
  }

  return event;
}


