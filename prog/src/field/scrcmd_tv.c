//======================================================================
/**
 * @file  scrcmd_tv.c
 * @brief  スクリプトコマンド：テレビ
 * @author  Saito
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

#include "savedata/misc.h"               // for MISC

#define RECORD_TV_SEX_MAX (11)
#define RECORD_TV_COMMON_MAX (18)

#define RECORD_TV_MAX (29)    //男女別11　共通18
#define LECTURE_TV_MAX  (50)
#define VARIETY_TV_MAX (80)
#define NOTBADGE_RETRY_TV_MAX   (12)    //バッジ持っていなかったときの再抽選番組数
#define NOTCLEAR_RETRY_TV_MAX   (20)    //クリアしてなかったときの再抽選番組数

#define REC_TAG_MAX (2)

#define NO_REC_ID (0xffffffff)

#define BADGE_MAX (8)
#define AFTER_CL_MAX (15)

#ifdef PM_DEBUG
//デバッグテレビ用番組番号 実体はmisc.c
extern u32 DebugTvNo;
#define TV_MAX ( (RECORD_TV_SEX_MAX*2)+RECORD_TV_COMMON_MAX+LECTURE_TV_MAX+VARIETY_TV_MAX )  //11*2+18+50+80=170

#endif //PM_DEBUG

static const u32 RecordTbl[RECORD_TV_MAX][REC_TAG_MAX+2] = {    //{ レコードＩＤ1、レコードＩＤ2、ＩＤ1の表示桁、ＩＤ2の表示桁 }
  { NO_REC_ID, NO_REC_ID, 0, 0 },                               // 0  なし、なし 0,0
  { RECID_NICKNAME, NO_REC_ID, 6, 0 },                          // 1  ニックネーム、なし 6,0
  { RECID_REPORT_COUNT,  NO_REC_ID, 9, 0 },                     // 2  レポート、なし 9,0
  { RECID_RECOVERY_PC,  NO_REC_ID, 9, 0 },                      // 3  ポケセンでやすんだ、なし 9,0
  { RECID_SHOPPING_CNT,  RECID_SHOPPING_MONEY, 9, 9 },          // 4  買い物をした、使用した合計金額  9,9
  { RECID_WATCH_TV,  NO_REC_ID, 9, 0 },                         // 5  テレビ見た、なし  9,0
  { RECID_CAPTURE_POKE,  NO_REC_ID, 9, 0 },                     // 6  捕獲したポケモン、なし  9,0
  { RECID_BTL_ENCOUNT,  RECID_BTL_TRAINER, 9, 9 },              // 7  エンカウント、トレーナー戦  9,9
  { RECID_POKE_EVOLUTION,  NO_REC_ID, 9, 0 },                   // 8  進化させたポケモン、なし  9,0
  { RECID_FISHING_SUCCESS,  RECID_FISHING_FAILURE, 6, 4 },      // 9  釣り上げ、ばらし  6,4
  { RECID_BATTLE_COUNT,  RECID_DENDOU_CNT, 9, 4 },              // 10  全戦闘回数、殿堂入り 9,4

  { RECID_RIDE_CYCLE,  NO_REC_ID, 9, 0 },                   // 11 自転車、なし  9,0
  { RECID_TAMAGO_HATCHING,  NO_REC_ID, 6, 0 },              // 12 タマゴ孵化、なし 6,0
  { RECID_SODATEYA_CNT,  NO_REC_ID, 9, 0 },                 // 13 育て屋、なし  9,0
  { RECID_GTS_PUT,  NO_REC_ID, 6, 0 },                      // 14 ＧＴＳ、なし  6,0
  { RECID_MAIL_WRITE,  NO_REC_ID, 6, 0 },                   // 15 メール、なし  6,0
  { RECID_PREMIUM_BALL,  NO_REC_ID, 6, 0 },                 // 16 プレミアボール、なし  6,0
  { RECID_RECOVERY_HOME,  NO_REC_ID, 4, 0 },                // 17 家で休んだ、なし 4, 0
  { RECID_WAZA_HANERU,  NO_REC_ID, 4, 0 },                  // 18 はねる、なし 4, 0
  { RECID_WAZA_WARUAGAKI,  NO_REC_ID, 4, 0 },               // 19 わるあがき、なし 4, 0
  { RECID_WAZA_MUKOU,  NO_REC_ID, 4, 0 },                   // 20 無効効果、なし 4, 0
  { RECID_NIGERU_SIPPAI,  NO_REC_ID, 4, 0 },                // 21 逃げそこなった、なし 4, 0
  { RECID_NIGERARETA,  NO_REC_ID, 4, 0 },                   // 22 逃げられた、なし 4, 0
  { RECID_FOSSIL_RESTORE,  NO_REC_ID, 4, 0 },               // 23 化石復元、なし 4, 0
  { RECID_GURUGURU_COUNT,  NO_REC_ID, 4, 0 },               // 24 ぐるぐる交換、なし 4, 0
  { RECID_DAYMAX_KILL,  NO_REC_ID, 4, 0 },                  // 25 1日で倒したポケモン最高、なし 4, 0
  { RECID_DAYMAX_CAPTURE,  NO_REC_ID, 4, 0 },               // 26 1日で捕獲したポケモン最高、なし 4, 0
  { RECID_DAYMAX_TRAINER_BATTLE,  NO_REC_ID, 4, 0 },        // 27 1日で倒したトレーナー最高、なし 4, 0
  { RECID_DAYMAX_EVOLUTION,  NO_REC_ID, 4, 0 },             // 28 1日で進化させたポケモン最高、なし 4, 0
};

//ジムバッジに影響するメッセージ
static const int BadgeMsg[BADGE_MAX] = {
  msg_tv_03_13,
  msg_tv_03_14,
  msg_tv_03_15,
  msg_tv_03_16,
  msg_tv_03_17,
  msg_tv_03_18,
  msg_tv_03_19,
  msg_tv_03_20
};

//クリア後抽選メッセージ
static const int AfterClearMsg[AFTER_CL_MAX] = {
  msg_tv_03_46,
  msg_tv_03_47,
  msg_tv_03_48,
  msg_tv_03_49,
  msg_tv_03_50,
  msg_tv_03_51,
  msg_tv_03_52,
  msg_tv_03_53,
  msg_tv_03_54,
  msg_tv_03_55,
  msg_tv_03_56,
  msg_tv_03_57,
  msg_tv_03_58,
  msg_tv_03_59,
  msg_tv_03_60,
};

static int CheckBadge(GAMEDATA *gdata, const int inMsg);
static int CheckClear(GAMEDATA *gdata, const int inMsg, const BOOL inClear);

//--------------------------------------------------------------
/**
 * テレビ番組抽選
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTV_GetMsg( VMHANDLE *core, void *wk )
{
  u32 minute;
  int msg_base;
  int msg;
  int rnd;
  RTCTime time;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData(gsys);
  MYSTATUS *my = GAMEDATA_GetMyStatus(gdata);
  WORDSET *wordset    = SCRIPT_GetWordSet( sc );

  u16 clear = SCRCMD_GetVMWorkValue( core, work );
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
      if (i == num) GF_ASSERT(0);
    }
  }

  //現在の時間（分を取得）
  GFL_RTC_GetTime(&time);
  minute = time.minute;
  if (minute < 10)        //レコード系
  {
    OS_Printf("レコード番組\n");
    msg_base = msg_tv_01_01;
    rnd = GFUser_GetPublicRand(RECORD_TV_MAX);
    OS_Printf("レコード抽選ナンバー%d\n",rnd);
    //レコード調べる
    {
      int rec1 = 0;
      int rec2 = 0;
      RECORD * rec = GAMEDATA_GetRecordPtr(gdata);
      if ( RecordTbl[rnd][0] != NO_REC_ID )
      {
        rec1 = RECORD_Get(rec, RecordTbl[rnd][0]);
        //タグ展開
        WORDSET_RegisterNumber( wordset, 1, rec1, RecordTbl[rnd][2], STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

      }
      if ( RecordTbl[rnd][1] != NO_REC_ID )
      {
        rec2 = RECORD_Get(rec, RecordTbl[rnd][1]);
        //タグ展開
        WORDSET_RegisterNumber( wordset, 2, rec2, RecordTbl[rnd][3], STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      }

      OS_Printf("レコード1 %d\n",rec1);
      OS_Printf("レコード2 %d\n",rec2);

      //男女別メッセージの場合
      if (rnd < RECORD_TV_SEX_MAX)
      {
        //2つのレコードが必要な場合
        if ( (RecordTbl[rnd][0] != NO_REC_ID)&&(RecordTbl[rnd][1] != NO_REC_ID) )
        {
          //２つのレコードいずれかが０のときは、デフォルトメッセージに変える
          if ( (rec1==0) || (rec2 == 0))
          {
            OS_Printf("両レコードが0なので、デフォルト番組に変更\n");
            rnd = 0;
          }
        }
        //1つ目のレコードだけ必要な場合(2つ目がNO_REC_IDの場合)
        else if( RecordTbl[rnd][1] == NO_REC_ID )
        {
          //1つ目のレコードが０のときは、デフォルトメッセージに変える
          if ( rec1==0 )
          {
            OS_Printf("レコードが0なので、デフォルト番組に変更\n");
            rnd = 0;
          }
        }
      }
    }

    if ( rnd < RECORD_TV_SEX_MAX )    //男女別メッセージの場合
    {
      u8 sex;
      //性別で分岐
      sex = MyStatus_GetMySex(my);
      if (sex == PTL_SEX_FEMALE)  //女
      {
        rnd += RECORD_TV_SEX_MAX;
      }
    }else                             //共通メッセージの場合
    {
      rnd += RECORD_TV_SEX_MAX;
    }
  }
  else if(minute < 35)    //レクチャー系
  {
    OS_Printf("レクチャー番組\n");
    msg_base = msg_tv_02_01;
    rnd = GFUser_GetPublicRand(LECTURE_TV_MAX);
  }
  else                    //バラエティ系
  {
    OS_Printf("バラエティ番組\n");
    msg_base = msg_tv_03_01;
    rnd = GFUser_GetPublicRand(VARIETY_TV_MAX);
  }

  msg = msg_base+rnd;
  
  OS_Printf("テレビ番組抽選結果　min = %d, rnd = %d, msg = %d\n",minute, rnd, msg);

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

    if ( msg < RECORD_TV_SEX_MAX*2)
    {
      u8 idx = msg % RECORD_TV_SEX_MAX;
      //タグ展開
      int rec1 = 0;
      int rec2 = 0;
      RECORD * rec = GAMEDATA_GetRecordPtr(gdata);
      if ( RecordTbl[idx][0] != NO_REC_ID )
      {
        rec1 = RECORD_Get(rec, RecordTbl[idx][0]);
        //タグ展開
        WORDSET_RegisterNumber( wordset, 1, rec1, RecordTbl[idx][2], STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      }
      if ( RecordTbl[idx][1] != NO_REC_ID )
      {
        rec2 = RECORD_Get(rec, RecordTbl[idx][1]);
        //タグ展開
        WORDSET_RegisterNumber( wordset, 2, rec2, RecordTbl[idx][3], STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      }

      OS_Printf("レコード1 %d\n",rec1);
      OS_Printf("レコード2 %d\n",rec2);

      //男女別メッセージの場合
      if (idx < RECORD_TV_SEX_MAX)
      {
        //2つのレコードが必要な場合
        if ( (RecordTbl[idx][0] != NO_REC_ID)&&(RecordTbl[idx][1] != NO_REC_ID) )
        {
          //２つのレコードいずれかが０のときは、デフォルトメッセージに変える
          if ( (rec1==0) || (rec2 == 0))
          {
            OS_Printf("両レコードが0なので、デフォルト番組に変更\n");
            if (msg < RECORD_TV_SEX_MAX ) msg = msg_tv_01_01;
            else msg = msg_tv_01_12;
          }
        }
        //1つ目のレコードだけ必要な場合(2つ目がNO_REC_IDの場合)
        else if( RecordTbl[idx][1] == NO_REC_ID )
        {
          //1つ目のレコードが０のときは、デフォルトメッセージに変える
          if ( rec1==0 )
          {
            OS_Printf("レコードが0なので、デフォルト番組に変更\n");
            if (msg < RECORD_TV_SEX_MAX ) msg = msg_tv_01_01;
            else msg = msg_tv_01_12;
          }
        }
      }
    }
    else if ((RECORD_TV_SEX_MAX*2 <= msg)&& (msg<RECORD_TV_SEX_MAX*2+18)){
      u8 idx = msg - RECORD_TV_SEX_MAX;
      //タグ展開
      int rec1 = 0;
      int rec2 = 0;
      RECORD * rec = GAMEDATA_GetRecordPtr(gdata);
      if ( RecordTbl[idx][0] != NO_REC_ID )
      {
        rec1 = RECORD_Get(rec, RecordTbl[idx][0]);
        //タグ展開
        WORDSET_RegisterNumber( wordset, 1, rec1, RecordTbl[idx][2], STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      }
      if ( RecordTbl[idx][1] != NO_REC_ID )
      {
        rec2 = RECORD_Get(rec, RecordTbl[idx][1]);
        //タグ展開
        WORDSET_RegisterNumber( wordset, 2, rec2, RecordTbl[idx][3], STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
      }

      OS_Printf("レコード1 %d\n",rec1);
      OS_Printf("レコード2 %d\n",rec2);
    }
  }
#endif  //PM_DEBUG

  msg = CheckBadge(gdata, msg);
  msg = CheckClear(gdata, msg, clear);

  *ret = msg;

  return VMCMD_RESULT_CONTINUE;

}

//--------------------------------------------------------------
/**
 * ジムバッジの所持状態によるメッセージ抽選
 * @param   gdata    ゲームデータポインタ
 * @param   inMag     抽選されたメッセージ
 * @retval int      再抽選されたメッセージ
 */
//--------------------------------------------------------------
static int CheckBadge(GAMEDATA *gdata, const int inMsg)
{
  int i;
  int msg;
  MISC* misc;
  // セーブデータ取得
  {
    SAVE_CONTROL_WORK* save;
    save = GAMEDATA_GetSaveControlWork( gdata );
    misc = SaveData_GetMisc( save );
  }

  msg = inMsg;
  for (i=0;i<BADGE_MAX;i++)
  {
    if ( BadgeMsg[i] == msg )
    {
      //バッジチェック
      if ( MISC_GetBadgeFlag( misc, BADGE_ID_01+i ) )
      {
        //持っているなら放送ＯＫ
        return msg;
      }
      else
      {
        //もって無い場合は別のメッセージ
        msg = msg_tv_03_01 + GFUser_GetPublicRand(NOTBADGE_RETRY_TV_MAX);
        OS_Printf("%dバッジ番組抽選したけど、持って無いから別のに変更 >> %d\n",i, msg);
        return msg;
      }
    }
  }
  //バッジ関係ないメッセージ
  return msg;
}

//--------------------------------------------------------------
/**
 * クリア状態によるメッセージ抽選
 * @param   gdata    ゲームデータポインタ
 * @param   inMag     抽選されたメッセージ
 * @param   inClear   クリアしてるか？
 * @retval int      再抽選されたメッセージ
 */
//--------------------------------------------------------------
static int CheckClear(GAMEDATA *gdata, const int inMsg, const BOOL inClear)
{
  int i;
  int msg;
  msg = inMsg;

  //クリアしていないなら再抽選を検討する
  if (!inClear)
  {
    for (i=0;i<AFTER_CL_MAX;i++)
    {
      if ( AfterClearMsg[i] == msg )
      {
        //クリアしていない場合は別のメッセージ
        msg = msg_tv_03_21 + GFUser_GetPublicRand(NOTCLEAR_RETRY_TV_MAX);
        OS_Printf("%dクリアしてないから別のに変更 >> %d\n",i, msg);
        return msg;
      }
    }
  }
  //クリア関係ないメッセージ
  return msg;
}
