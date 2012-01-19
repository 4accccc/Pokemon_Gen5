//======================================================================
/**
 * @file  scrcmd_tv.c
 * @brief  スクリプトコマンド：テレビ
 * @author  Saito
 *          swan >> ichiraku_katsuhiko
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
#include "scrcmd_tv.h"

#include "msg/script/msg_tv_01_scr.h"
#include "../../../resource/fldmapdata/script/tv_scr_local.h"

#include "savedata/misc.h"               // for MISC
#include "savedata/person_state.h"       // for LLC

#include "poke_tool/monsno_def.h"        // MONSNO_XXX

#define TALENT_TV_NORMAL_MAX  (22)
#define TALENT_TV_SEX_MAX     (10)
#define TALENT_TV_LLC_MAX     (4)
#define TALENT_TV_MAX   (TALENT_TV_NORMAL_MAX+TALENT_TV_SEX_MAX+TALENT_TV_LLC_MAX)  // 
#define BATTLE_TV_MAX   (50)
#define VARIETY_TV_MAX  (80)
#define CM_TV_MAX       (8)

#define MSG_SEX_MAX     (13)

#ifdef PM_DEBUG
//デバッグテレビ用番組番号 実体はmisc.c
extern u32 DebugTvNo;
#define TV_MAX ( TALENT_TV_MAX+TALENT_TV_SEX_MAX+TALENT_TV_LLC_MAX+BATTLE_TV_MAX+VARIETY_TV_MAX+CM_TV_MAX )  //36+10+4+50+80+8=188

#endif //PM_DEBUG

// LLCのデートフラグ見るメッセージ
// が性別メッセージ始まりから何番目か
#define LLC_DATE_MSG_DEFAULT    (10)
#define LLC_DATE_MSG_AFTER      (11)

// LLCの交換フラグ見るメッセージ
// が性別メッセージ始まりから何番目か
#define LLC_CHANGE_MSG_DEFAULT  (12)
#define LLC_CHANGE_MSG_AFTER    (13)

// CM確立
#define CM_RAND_MAX     (100)
#define CM_RAND_FIRST   (20)    // 初回
#define CM_RAND_AFTER   (10)    // 初回以降

//--------------------------------------------------------------
/**
 * テレビ番組抽選
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTV_GetMsg( VMHANDLE *core, void *wk )
{
  int msg_base;
  int msg;
  u32 rnd;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData(gsys);
  MYSTATUS *my = GAMEDATA_GetMyStatus(gdata);
  WORDSET *wordset    = SCRIPT_GetWordSet( sc );

  u16 select_tv = SCRCMD_GetVMWorkValue( core, work );
  u16 *ret = SCRCMD_GetVMWork( core, work );

  //固定タグ展開
  {
    //0番にプレーヤー名
    WORDSET_RegisterPlayerName( wordset, 0, my );
    //3番に手持ちの先頭ポケモンの種族名(タマゴを除く)
    {
      int i;
      int num;
      POKEPARTY*     party = GAMEDATA_GetMyPokemon( gdata );
      num = PokeParty_GetPokeCount( party );
      for (i=0;i<num;i++)
      {
        BOOL tamago_flg;
        POKEMON_PARAM* pp = PokeParty_GetMemberPointer( party, i );
        //タマゴではないポケモン
        tamago_flg = PP_Get( pp, ID_PARA_tamago_flag, NULL );
        if ( !tamago_flg )
        {
          u32 monsno = PP_Get( pp, ID_PARA_monsno, NULL );
          WORDSET_RegisterPokeMonsNameNo( wordset, 3, monsno );
          break;
        }
      }
      //タマゴではないポケモンを見つけられなかった
      if (i == num){
        WORDSET_RegisterPokeMonsNameNo( wordset, 3, MONSNO_TIRATIINO );
      }
    }

    // 1番にLCCの隠しキャラと交換をした場合のみ、自分が交換に出したポケモンの種族名(タマゴを除く)
    {
      PERSON_STATE  *p_person_state = PERSON_STATE_GetWork( GAMEDATA_GetSaveControlWork(gdata) );

      // 交換してる場合
      if( PERSON_STATE_GetTradeFlag( p_person_state ) == TRUE )
      {
        u16 monsno = PERSON_STATE_GetTradeMonsno( p_person_state );
        WORDSET_RegisterPokeMonsNameNo( wordset, 1, monsno );
        ICHI_Printf("LCC隠しキャラとの交換に出したポケモンNO:%d\n", monsno);
      }
    }

  }

  // 何を選択したかでメッセージ分岐
  //
  switch( select_tv ){
  case SELECT_TALENT:
    {
      u8 sex;
      int *msg_tbl;
      int i;

      OS_Printf("タレント番組\n");
      msg_base = msg_tv_01_01;
      rnd = GFUser_GetPublicRand( TALENT_TV_MAX );

      // 性別条件のメッセージなら
      if( rnd >= TALENT_TV_NORMAL_MAX )
      {
        OS_Printf("性別別メッセージ\n");
        // 性別メッセージからの乱数にする
        rnd = rnd - TALENT_TV_NORMAL_MAX;

        GF_ASSERT( rnd < (TALENT_TV_SEX_MAX+TALENT_TV_LLC_MAX) );
        
        //性別で分岐
        sex = MyStatus_GetMySex(my);
        // メッセージのテーブル
        if( sex == PM_MALE ){ msg_base = msg_tv_01_37; }
        else                { msg_base = msg_tv_01_23; }

        // LLCメッセージか判定
        if( rnd >= TALENT_TV_SEX_MAX )
        {
          // LLCのセーブデータ取得
          SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork( gdata );
          PERSON_STATE* person_sv = PERSON_STATE_GetWork( save );
          
          // デート後メッセージ判定
          if( rnd == LLC_DATE_MSG_AFTER )
          {
            // フラグが立っていないなら、通常メッセージ
            if( PERSON_STATE_GetDateEndFlag( person_sv ) == FALSE )
            {
              rnd -= 1;   // 1つ前がフラグ立っていない版メッセージ
            }
          }
          // デート前メッセージ判定
          else if( rnd == LLC_DATE_MSG_DEFAULT )
          {
            // フラグが立っているなら、通常後メッセージ
            if( PERSON_STATE_GetDateEndFlag( person_sv ) == TRUE )
            {
              rnd += 1;   // 1つ前がフラグ立っていない版メッセージ
            }
          }

          // 交換後メッセージ判定
          if( rnd == LLC_CHANGE_MSG_AFTER )
          {
            // フラグが立っていないなら、通常メッセージ
            if( PERSON_STATE_GetTradeFlag( person_sv ) == FALSE )
            {
              rnd -= 1;   // 1つ前がフラグ立っていない版メッセージ
            }
          }
          // 交換前メッセージ判定
          else if( rnd == LLC_CHANGE_MSG_DEFAULT ){
            // フラグが立っているなら、通常後メッセージ
            if( PERSON_STATE_GetTradeFlag( person_sv ) == TRUE )
            {
              rnd += 1;   // 1つ前がフラグ立っていない版メッセージ
            }
          }
        }
      }
    }
    break;
  case SELECT_BATTLE:
    {
      OS_Printf("バトル番組\n");
      msg_base = msg_tv_02_01;
      rnd = GFUser_GetPublicRand( BATTLE_TV_MAX );
    }
    break;
  case SELECT_VARIETY:
    {
      OS_Printf("バラエティ番組\n");
      msg_base = msg_tv_03_01;
      rnd = GFUser_GetPublicRand( VARIETY_TV_MAX );
    }
    break;
  default:
    *ret = msg_tv_01_01;
    GF_ASSERT_MSG( 0,"存在しない番組です");
    return VMCMD_RESULT_CONTINUE;
  }

  msg = msg_base + rnd;
  
  OS_Printf("テレビ番組抽選結果　rnd = %d, msg = %d\n", rnd, msg);


#ifdef PM_DEBUG
  if (DebugTvNo)
  {
    OS_Printf("デバッグ設定されているので書き換え　%d ==> %d\n",msg, DebugTvNo-1);
    if (DebugTvNo > TV_MAX)
    {
      GF_ASSERT_MSG(0,"debug tv_no error %d",DebugTvNo);
      msg = TV_MAX-1;
    }
    else{
      msg = DebugTvNo-1;
    }
  }
#endif  //PM_DEBUG

  *ret = msg;

  return VMCMD_RESULT_CONTINUE;

}

//--------------------------------------------------------------
/**
 * @brief テレビ番組CM挿入チェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTV_CM_Check( VMHANDLE *core, void *wk )
{
  u32   rnd;
  BOOL  insert_flag = FALSE;
  SCRCMD_WORK *work = wk;

  u16 cm_insert = SCRCMD_GetVMWorkValue( core, work );
  u16 *ret = SCRCMD_GetVMWork( core, work );

  rnd = GFUser_GetPublicRand( CM_RAND_MAX );

  if( cm_insert )
  {
    OS_Printf("CM 抽選 10% ");
    // CM挿入されたことあるので確立下がる
    // 10%
    if( rnd < CM_RAND_AFTER )
    {
      insert_flag = TRUE;
    }
  }
  else
  {
    OS_Printf("CM 抽選 20% ");
    // 20%
    if( rnd < CM_RAND_FIRST )
    {
      insert_flag = TRUE;
    }
  }

#ifdef PM_DEBUG
  {
    if(GFL_UI_KEY_GetCont() & PAD_BUTTON_L)
    {
      insert_flag = TRUE;
      OS_Printf("デバッグ強制 ");
    }
  }
#endif  //PM_DEBUG

  if( insert_flag == TRUE )
  {
    OS_Printf(" 挿入する!!\n");
    *ret = 1;
  }
  else
  {
    OS_Printf(" 挿入しない\n");
    *ret = 0;
  }

  return VMCMD_RESULT_CONTINUE;

}

//--------------------------------------------------------------
/**
 * @brief テレビ番組CMメッセージ抽選
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTV_CM_GetMsg( VMHANDLE *core, void *wk )
{
  int msg;
  u32 rnd;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData(gsys);
  SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork( gdata );

  u16 *ret = SCRCMD_GetVMWork( core, work );

  // どの項目を紹介するか？
  rnd = GFUser_GetPublicRand( CM_TV_MAX );
  msg = msg_tv_04_01;

  *ret = ( msg + rnd );

  return VMCMD_RESULT_CONTINUE;

}

