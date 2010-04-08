//======================================================================
/**
 * @file  scrcmd_password.c
 * @brief  パスワード答え比較
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

#include "scrcmd_password.h"

#include "../../../resource/quiz/pms_answer.h"

//--------------------------------------------------------------
/**
 * 回答チェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCheckPassword( VMHANDLE *core, void *wk )
{
  u16 ans_word1;
  u16 ans_word2;
  SCRCMD_WORK*   work     = (SCRCMD_WORK*)wk;
  
  u16 no = SCRCMD_GetVMWorkValue( core, work ); //1 or 2
  u16 word1 = SCRCMD_GetVMWorkValue( core, work );
  u16 word2 = SCRCMD_GetVMWorkValue( core, work );
  u16 *ret = SCRCMD_GetVMWork( core, work );

  //１１ビットマスクして、純粋な単語コードにする
  {
    word1 &= 0x7ff;   //11ビットマスク
    word2 &= 0x7ff;   //11ビットマスク
  }

  OS_Printf("w1=%d w2=%d\n",word1,word2);

  //答え比較
  switch(no){
  case 1: //1問目
    {
      PMS_WORD pms_word;
      pms_word = password01; //「みんな」
      ans_word1 = pms_word & 0x7ff;   //11ビットマスク
      pms_word = password02; //「ハッピー」
      ans_word2 = pms_word & 0x7ff;   //11ビットマスク
    }
    break;
  case 2: //2問目
    {
      PMS_WORD pms_word;
      pms_word = password03; //「かんたん」
      ans_word1 = pms_word & 0x7ff;   //11ビットマスク
      pms_word = password04; //「つうしん」
      ans_word2 = pms_word & 0x7ff;   //11ビットマスク
    }
    break;
  default:
    GF_ASSERT(0);
    *ret = FALSE;
    return VMCMD_RESULT_CONTINUE;
  }

  OS_Printf("aw1=%d aw2=%d\n",ans_word1, ans_word2);

  if ( (ans_word1 == word1)&&(ans_word2 == word2) ) *ret = TRUE;
  else *ret = FALSE;
  
  return VMCMD_RESULT_CONTINUE;
}

