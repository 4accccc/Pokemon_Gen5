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
  NetErr_App_ReqErrorDisp();
  return EVENT_FieldSubProc(gsys, fieldmap, FS_OVERLAY_ID(fieldmap), &FieldCommErrorProc, gsys);
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
