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

#include "message.naix"
#include "msg\msg_pms_word06.h"
#include "msg\msg_pms_word07.h"
#include "msg\msg_pms_word08.h"
#include "msg\msg_pms_word09.h"
#include "msg\msg_pms_word10.h"
#include "msg\msg_pms_word11.h"
#include "msg\msg_pms_word12.h"


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
  NOZOMU_Printf("w1=%d w2=%d\n",word1,word2);

  //答え比較
  switch(no){
  case 1:     //1問目
    {
      PMS_WORD pms_word;
      pms_word = PMSW_GetWordNumberByGmmID( NARC_message_pms_word07_dat, pms_word07_35 ); //「みんな」
      ans_word1 = pms_word & 0x7ff;   //11ビットマスク
      pms_word = PMSW_GetWordNumberByGmmID( NARC_message_pms_word10_dat, pms_word10_33 ); //「ハッピー」
      ans_word2 = pms_word & 0x7ff;   //11ビットマスク
    }
    break;
  case 2:   //2問目
    {
      PMS_WORD pms_word;
      pms_word = PMSW_GetWordNumberByGmmID( NARC_message_pms_word10_dat, pms_word10_10 ); //「かんたん」
      ans_word1 = pms_word & 0x7ff;   //11ビットマスク
      pms_word = PMSW_GetWordNumberByGmmID( NARC_message_pms_word12_dat, pms_word12_16 ); //「つうしん」
      ans_word2 = pms_word & 0x7ff;   //11ビットマスク
    }
    break;
  default:
    GF_ASSERT(0);
    *ret = FALSE;
    return VMCMD_RESULT_CONTINUE;
  }

  NOZOMU_Printf("aw1=%d aw2=%d\n",ans_word1, ans_word2);

  if ( (ans_word1 == word1)&&(ans_word2 == word2) ) *ret = TRUE;
  else *ret = FALSE;
  
  return VMCMD_RESULT_CONTINUE;
}

