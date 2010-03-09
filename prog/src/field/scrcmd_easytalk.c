//======================================================================
/**
 * @file  scrcmd_easytalk.c
 * @brief  簡易会話スクリプトコマンド
 * @author  Nozomu Satio
 *
 */
//======================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "event_fieldmap_control.h"

#include "scrcmd_easytalk.h"
#include "app/pms_input.h"
#include "savedata/my_pms_data.h"

#include "easytalk_mode_def.h"
#include "app/pms_input.h"  //for PMSI_MODE

typedef struct EASYTALK_EVT_WORK_tag
{
  u32 Mode;
  u16 *Word1;
  u16 *Word2;
  u16 *Ret;
//  PMS_SELECT_PARAM  PmsParam;
  PMSI_PARAM *pmsi;
  MYPMS_PMS_TYPE PmsType;
  BOOL NeedSave;
}EASYTALK_EVT_WORK;

static GMEVENT_RESULT EasyTalkAppCallEvt( GMEVENT* event, int* seq, void* work );

//--------------------------------------------------------------
/**
 * 簡易会話アプリコール
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCallEasyTalkApp( VMHANDLE *core, void *wk )
{
  u16 mode;
  u16 *word1;
  u16 *word2;
  u16 *ret;
  GMEVENT * event;
  int size;
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata =SCRCMD_WORK_GetGameData( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );

  mode = SCRCMD_GetVMWorkValue( core, work );
  word1 = SCRCMD_GetVMWork(core, work);
  word2 = SCRCMD_GetVMWork(core, work);
  ret = SCRCMD_GetVMWork(core, work);

  size = sizeof(EASYTALK_EVT_WORK);

  event = GMEVENT_Create( gsys, NULL, EasyTalkAppCallEvt, size );
  {
    PMSI_MODE pmsi_mode;
    u32 guidance;
    EASYTALK_EVT_WORK *evt_work;
    SAVE_CONTROL_WORK *save_ctrl;
    evt_work = GMEVENT_GetEventWork(event);
    MI_CpuClear8( evt_work, size );
    evt_work->Mode = mode;
    evt_work->Word1 = word1;
    evt_work->Word2 = word2;
    evt_work->Ret = ret;

    *(evt_work->Ret) = FALSE;
    
    switch(mode){
    case EASYTALK_MODE_READY:
      guidance = PMSI_GUIDANCE_BATTLE_READY;  ///< 対戦前         <<@todo
      evt_work->PmsType = MYPMS_PMS_TYPE_BATTLE_READY;  //バトル勝負前挨拶
      evt_work->NeedSave = TRUE;
      pmsi_mode = PMSI_MODE_SENTENCE;
      break;
    case EASYTALK_MODE_WIN:
      guidance = PMSI_GUIDANCE_BATTLE_WON;    ///< 勝った時のコメント <<@todo
      evt_work->PmsType = MYPMS_PMS_TYPE_BATTLE_WON;    //バトル勝ち時セリフ
      evt_work->NeedSave = TRUE;
      pmsi_mode = PMSI_MODE_SENTENCE;
      break;
    case EASYTALK_MODE_LOSE:
      guidance = PMSI_GUIDANCE_BATTLE_LOST; 	///< 負けたときコメント <<@todo
      evt_work->PmsType = MYPMS_PMS_TYPE_BATTLE_LOST;   //バトル負け時セリフ
      evt_work->NeedSave = TRUE;
      pmsi_mode = PMSI_MODE_SENTENCE;
      break;
    case EASYTALK_MODE_TOP:
      guidance = PMSI_GUIDANCE_DEFAULT;   //<<@todo
      evt_work->PmsType = MYPMS_PMS_TYPE_BATTLE_TOP;    ///<　一番になったとき
      evt_work->NeedSave = TRUE;
      pmsi_mode = PMSI_MODE_SENTENCE;
      break;
    case EASYTALK_MODE_PASSWORD:
      guidance = PMSI_GUIDANCE_DEFAULT;   //<<@todo
      evt_work->PmsType = MYPMS_PMS_TYPE_BATTLE_TOP;    ///<　パスワード
      evt_work->NeedSave = FALSE;
      pmsi_mode = PMSI_MODE_DOUBLE;
    default:
      GF_ASSERT(0);
      evt_work->pmsi = NULL;
      SCRIPT_CallEvent( sc, event );
      return VMCMD_RESULT_SUSPEND;
    }

    save_ctrl = GAMEDATA_GetSaveControlWork( gdata );
    evt_work->pmsi = PMSI_PARAM_Create(pmsi_mode, guidance, NULL, FALSE, save_ctrl, GFL_HEAPID_APP );
    //起動時の単語セット
    if (evt_work->NeedSave)
    {
      MYPMS_DATA *my_pms;
      PMS_DATA  pms;
      SAVE_CONTROL_WORK* sv;
      sv = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gsys));
      my_pms = SaveData_GetMyPmsData( sv);
      MYPMS_GetPms( my_pms, evt_work->PmsType, &pms );
      PMSI_PARAM_SetInitializeDataSentence( evt_work->pmsi, &pms );
    }
    SCRIPT_CallEvent( sc, event );

  }
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------------------------------------
/**
 * アプリコールイベント
 *
 * @param   event       イベントポインタ
 * @param   *seq        シーケンサ
 * @param   work        ワークポインタ
 *
 * @return	GMEVENT_RESULT    イベント結果
 */
//--------------------------------------------------------------------------------------------
static GMEVENT_RESULT EasyTalkAppCallEvt( GMEVENT* event, int* seq, void* work )
{
  EASYTALK_EVT_WORK *evt_work;
  PMSI_PARAM *prm;
  evt_work = GMEVENT_GetEventWork(event);
  prm = evt_work->pmsi;

  switch(*seq){
  case 0:
    if ( evt_work->pmsi == NULL ) return GMEVENT_RES_FINISH;
    //アプリコール
    GMEVENT_CallProc( event, FS_OVERLAY_ID(pmsinput), &ProcData_PMSInput, prm );
    (*seq)++;
    break;
  case 1:
    if ( PMSI_PARAM_CheckCanceled(prm) ) *(evt_work->Ret) = FALSE;
    else
    {
      MYPMS_DATA *my_pms;
      MYPMS_PMS_TYPE type;
      PMS_DATA   pms;
      
      PMSI_PARAM_GetInputDataSentence( prm, &pms );
      //セーブする
      if ( evt_work->NeedSave )
      {
        GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
        SAVE_CONTROL_WORK* sv;
        sv = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gsys));
        my_pms = SaveData_GetMyPmsData( sv);
        MYPMS_SetPms( my_pms, evt_work->PmsType, &pms );
      }
      *(evt_work->Ret) = TRUE;
      *(evt_work->Word1) = pms.word[0];
      *(evt_work->Word2) = pms.word[1];
    }
    //解放
    PMSI_PARAM_Delete( evt_work->pmsi );
    //終了
    return GMEVENT_RES_FINISH;
  }

  return GMEVENT_RES_CONTINUE;
}



