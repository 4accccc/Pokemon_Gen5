//======================================================================
/**
 * @file  scrcmd_quiz.c
 * @brief  クイズ取得
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

#include "scrcmd_quiz.h"

#include "../../../resource/quiz/pms_answer.h"
#include "../../../resource/message/dst/script/msg_c07r0201.h"

#include "ev_time.h"
#include "savedata/system_data.h"

#define NORMAL_QUIZ_NUM_MAX  (30)
#define QUIZ_NUM_MAX  (NORMAL_QUIZ_NUM_MAX+1)   //通常30問+誕生日問題1問


//--------------------------------------------------------------
/**
 * 回答チェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetQuiz( VMHANDLE *core, void *wk )
{
  int ev_month, ev_day;
  int sys_birth_month, sys_birth_day;

  SCRCMD_WORK*   work     = (SCRCMD_WORK*)wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( wk );
  
  u16 *quiz = SCRCMD_GetVMWork( core, work );
  u16 *hint = SCRCMD_GetVMWork( core, work );
  u16 *ans = SCRCMD_GetVMWork( core, work );

  u16 answer[QUIZ_NUM_MAX] = {
    answer00,answer01,answer02,answer03,answer04,
    answer05,answer06,answer07,answer08,answer09,
    answer10,answer11,answer12,answer13,answer14,
    answer15,answer16,answer17,answer18,answer19,
    answer20,answer21,answer22,answer23,answer24,
    answer25,answer26,answer27,answer28,answer29,
    answer30,
  };

  {
    SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork( gdata );
    SYSTEMDATA *sys_dat = SaveData_GetSystemData( sv );
    ev_month = EVTIME_GetMonth(gdata);
    ev_day = EVTIME_GetDay(gdata);
    sys_birth_month = SYSTEMDATA_GetBirthMonth(sys_dat);
    sys_birth_day = SYSTEMDATA_GetBirthDay(sys_dat);
  }
  //今日は自分（ＤＳで設定した）誕生日か?
  if ( (ev_day == sys_birth_day)&&(ev_month == sys_birth_month) )
  {
    *quiz = msg_quiz_31;
    *hint = msg_quiz_hint_31;
    *ans = answer[QUIZ_NUM_MAX-1];
  }
  else
  {
    u32 rnd;
    //0～29の乱数をとる
    rnd = GFUser_GetPublicRand(NORMAL_QUIZ_NUM_MAX);
    *quiz = msg_quiz_01 + rnd;
    *hint = msg_quiz_hint_01 + rnd;
    *ans = answer[rnd];
  }

  return VMCMD_RESULT_CONTINUE;
}

