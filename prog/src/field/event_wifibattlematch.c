//============================================================================================
/**
 * @file     event_wifibattlematch.c
 * @brief	  イベント：WIFI世界対戦
 * @author   Toru=Nagihashi
 * @date     2009.11.24
 */
//============================================================================================
//ライブラリ
#include <gflib.h>

//システム
#include "system/main.h"      //GFL_HEAPID_APP参照

//フィールド
#include "event_fieldmap_control.h"

//PROC
#include "net_app/wifibattlematch.h"
#include "net_app/wifi_login.h"

//外部公開
#include "field/event_wifibattlematch.h"



FS_EXTERN_OVERLAY(wifibattlematch);

//============================================================================================
//
//    定数
//
//============================================================================================
//-------------------------------------
///	シーケンス
//=====================================
enum{
  _FIELD_CLOSE,
  _CALL_WIFILOGIN,
  _WAIT_WIFILOGIN,
  _CALL_WIFIBTLMATCH,
  _WAIT_WIFIBTLMATCH,
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
  SAVE_CONTROL_WORK * ctrl;
  MYSTATUS          * pStatus[2];
  void              * p_sub_wk;
} EVENT_WIFIBTLMATCH_WORK;


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
static GMEVENT_RESULT EVENT_WifiBattleMatchMain(GMEVENT * event, int *  seq, void * work)
{
  EVENT_WIFIBTLMATCH_WORK * dbw = work;
  GAMESYS_WORK * gsys = dbw->gsys;


  switch (*seq) {
  case _FIELD_CLOSE:
    if(GAME_COMM_NO_NULL == GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gsys)))
    {
      GMEVENT_CallEvent(event, EVENT_FieldClose(gsys, dbw->fieldmap));
      (*seq)++;
    }
    break;
  case _CALL_WIFILOGIN:
    GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(wifi_login), &WiFiLogin_ProcData, dbw);
    (*seq)++;
    break;
  case _WAIT_WIFILOGIN:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){
      (*seq) ++;
    }
    break;
  case _CALL_WIFIBTLMATCH:
    { 
      WIFIBATTLEMATCH_PARAM *p_param;
      dbw->p_sub_wk = GFL_HEAP_AllocClearMemory(HEAPID_PROC,sizeof(WIFIBATTLEMATCH_PARAM));
      p_param = dbw->p_sub_wk;
      p_param->mode = WIFIBATTLEMATCH_MODE_RANDOM_FREE;
      p_param->p_my = dbw->pStatus[0];
      p_param->p_save = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gsys));
      GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(wifibattlematch), &WifiBattleMaptch_ProcData, dbw->p_sub_wk );
    }
    (*seq)++;
    break;
  case _WAIT_WIFIBTLMATCH:
    if (GAMESYSTEM_IsProcExists(gsys) == GFL_PROC_MAIN_NULL){
      GFL_HEAP_FreeMemory( dbw->p_sub_wk );
      (*seq) ++;
    }
    break;
  case _WAIT_NET_END:
    if(GFL_NET_IsExit()){
      (*seq) ++;
    }
    break;
  case _FIELD_OPEN:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(gsys));
    (*seq) ++;
    break;
  case _FIELD_END:
    GFL_HEAP_FreeMemory(dbw->pStatus[0]);
    GFL_HEAP_FreeMemory(dbw->pStatus[1]);
    return GMEVENT_RES_FINISH;
  default:
    GF_ASSERT(0);
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

/*
 *  @brief  WiFiクラブ呼び出しパラメータセット
 */
static void wifi_SetEventParam( GMEVENT* event, GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap )
{
 // BATTLE_SETUP_PARAM * para;
  EVENT_WIFIBTLMATCH_WORK * dbw;

  if(GAME_COMM_NO_NULL!= GameCommSys_BootCheck(GAMESYSTEM_GetGameCommSysPtr(gsys))){
    GameCommSys_ExitReq(GAMESYSTEM_GetGameCommSysPtr(gsys));
  }

  dbw = GMEVENT_GetEventWork(event);
  dbw->gsys = gsys;
  dbw->fieldmap = fieldmap;
  dbw->ctrl = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gsys));
  dbw->pStatus[0] = GFL_HEAP_AllocClearMemory(HEAPID_PROC,MyStatus_GetWorkSize());
  dbw->pStatus[1] = GFL_HEAP_AllocClearMemory(HEAPID_PROC,MyStatus_GetWorkSize());
  {
    MYSTATUS * pMy =GAMEDATA_GetMyStatusPlayer(GAMESYSTEM_GetGameData(gsys), 0);
    GFL_STD_MemCopy(pMy,dbw->pStatus[0], MyStatus_GetWorkSize());
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  WiFi世界対戦コール
 *
 *	@param	GAMESYS_WORK * gsys ゲームシステム
 *	@param	* fieldmap          フィールドマップ
 */
//-----------------------------------------------------------------------------
GMEVENT* EVENT_WifiBattleMatch( GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap )
{
  GMEVENT * event = GMEVENT_Create(gsys, NULL, EVENT_WifiBattleMatchMain, sizeof(EVENT_WIFIBTLMATCH_WORK));
  wifi_SetEventParam( event, gsys, fieldmap );

  return event;
}


