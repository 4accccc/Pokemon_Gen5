//==============================================================================
/**
 * @file    union_oneself.c
 * @brief   ユニオン：自分動作
 * @author  matsuda
 * @date    2009.07.09(木)
 */
//==============================================================================
#include <gflib.h>
#include "union_types.h"
#include "union_local.h"
#include "net_app/union/union_main.h"
#include "union_oneself.h"
#include "field/field_player.h"
#include "field/fieldmap.h"
#include "net_app/union/union_msg.h"
#include "message.naix"
#include "msg/msg_union_room.h"
#include "union_comm_command.h"
#include "system/main.h"
#include "net_app/union/union_event_check.h"
#include "net_app/union/union_subproc.h"
#include "net_app/union/colosseum_comm_command.h"
#include "colosseum.h"
#include "colosseum_tool.h"
#include "union_chara.h"
#include "field/fieldmap_ctrl_grid.h"
#include "savedata/config.h"
#include "pm_define.h"
#include "field\event_colosseum_battle.h"
#include "union_tool.h"
#include "app\pms_input.h"
#include "app\pms_select.h"
#include "poke_tool/regulation_def.h"
#include "poke_tool/poke_regulation.h"
#include "savedata/battle_box_save.h"

//#include "field/event_ircbattle.h"
#include "net_app\irc_compatible.h"
#include "net_app/pokemontrade.h"

#include "gamesystem/game_beacon.h"
#include "field/party_select_list.h"


//==============================================================================
//  定数定義
//==============================================================================
///シーケンス番号
enum{
  ONESELF_SUBPROC_INIT,
  ONESELF_SUBPROC_UPDATE,
  ONESELF_SUBPROC_EXIT,
};

///話しかけてから反応があるまでのタイムアウト時間
#define ONESELF_SERVER_TIMEOUT      (30 * 20)


//==============================================================================
//  型定義
//==============================================================================
///動作型
typedef BOOL (*ONESELF_FUNC)(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);

///動作セット
typedef struct{
  ONESELF_FUNC func_init;
  ONESELF_FUNC func_update;
  ONESELF_FUNC func_exit;
}ONESELF_FUNC_DATA;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static BOOL OneselfSeq_NormalInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_NormalUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_Leave(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ChatCallUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectReqInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectReqUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectReqExit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectAnswerInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectAnswerUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ConnectAnswerExit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkInit_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkUpdate_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkExit_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkListSendUpdate_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkInit_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkUpdate_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkExit_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkPlayGameUpdate_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TalkPlayGameUpdate_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TrainerCardUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_TradeUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_IntrudeUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ShutdownUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_Talk_Battle_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_MultiColosseumUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumMemberWaitUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumFirstDataSharingUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumNormal(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumStandPosition(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumPokelist(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumAllBattleReadyWait(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumBattle(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumStandingBack(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumUsePartySelect(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumPokelistReady(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumPokelistBeforeDataShare(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumLeaveUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);
static BOOL OneselfSeq_ColosseumTrainerCardUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq);


//==============================================================================
//  データ
//==============================================================================
static const ONESELF_FUNC_DATA OneselfFuncTbl[] = {
  {//UNION_STATUS_NORMAL
    OneselfSeq_NormalInit,
    OneselfSeq_NormalUpdate,
    NULL,
  },
  {//UNION_STATUS_ENTER
    NULL,
    NULL,
    NULL,
  },
  {//UNION_STATUS_LEAVE
    NULL,
    OneselfSeq_Leave,
    NULL,
  },
  {//UNION_STATUS_CHAT_CALL
    NULL,
    OneselfSeq_ChatCallUpdate,
    NULL,
  },
  {//UNION_STATUS_CONNECT_REQ
    OneselfSeq_ConnectReqInit,
    OneselfSeq_ConnectReqUpdate,
    OneselfSeq_ConnectReqExit,
  },
  {//UNION_STATUS_CONNECT_ANSWER
    OneselfSeq_ConnectAnswerInit,
    OneselfSeq_ConnectAnswerUpdate,
    OneselfSeq_ConnectAnswerExit,
  },
  {//UNION_STATUS_TALK_PARENT
    OneselfSeq_TalkInit_Parent,
    OneselfSeq_TalkUpdate_Parent,
    OneselfSeq_TalkExit_Parent,
  },
  {//UNION_STATUS_TALK_LIST_SEND_PARENT
    NULL,
    OneselfSeq_TalkListSendUpdate_Parent,
    NULL,
  },
  {//UNION_STATUS_TALK_CHILD
    OneselfSeq_TalkInit_Child,
    OneselfSeq_TalkUpdate_Child,
    OneselfSeq_TalkExit_Child,
  },
  {//UNION_STATUS_TALK_BATTLE_PARENT
    NULL,
    OneselfSeq_Talk_Battle_Parent,
    NULL,
  },
  {//UNION_STATUS_TALK_PLAYGAME_PARENT
    NULL,
    OneselfSeq_TalkPlayGameUpdate_Parent,
    NULL,
  },
  {//UNION_STATUS_TALK_PLAYGAME_CHILD
    NULL,
    OneselfSeq_TalkPlayGameUpdate_Child,
    NULL,
  },
  {//UNION_STATUS_TRAINERCARD
    NULL,
    OneselfSeq_TrainerCardUpdate,
    NULL,
  },
  {//UNION_STATUS_PICTURE
    NULL,
    OneselfSeq_ShutdownUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_SINGLE_50_SHOOTER
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_SINGLE_50
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_SINGLE_FREE_SHOOTER
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_SINGLE_FREE
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_SINGLE_STANDARD_SHOOTER
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_SINGLE_STANDARD
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_DOUBLE_50_SHOOTER
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_DOUBLE_50
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_DOUBLE_FREE_SHOOTER
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_DOUBLE_FREE
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_DOUBLE_STANDARD_SHOOTER
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_DOUBLE_STANDARD
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_TRIPLE_50_SHOOTER
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_TRIPLE_50
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_TRIPLE_FREE_SHOOTER
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_TRIPLE_FREE
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_TRIPLE_STANDARD_SHOOTER
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_TRIPLE_STANDARD
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_ROTATION_50_SHOOTER
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_ROTATION_50
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_ROTATION_FREE_SHOOTER
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_ROTATION_FREE
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_ROTATION_STANDARD_SHOOTER
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_1VS1_ROTATION_STANDARD
    NULL,
    OneselfSeq_ColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_BATTLE_MULTI
    NULL,
    OneselfSeq_MultiColosseumUpdate,
    NULL,
  },
  {//UNION_STATUS_TRADE
    NULL,
    OneselfSeq_TradeUpdate,
    NULL,
  },
  {//UNION_STATUS_GURUGURU
    NULL,
    OneselfSeq_ShutdownUpdate,
    NULL,
  },
  {//UNION_STATUS_RECORD
    NULL,
    OneselfSeq_ShutdownUpdate,
    NULL,
  },
  {//UNION_STATUS_INTRUDE
    NULL,
    OneselfSeq_IntrudeUpdate,
    NULL,
  },
  {//UNION_STATUS_SHUTDOWN
    NULL,
    OneselfSeq_ShutdownUpdate,
    NULL,
  },
  {//UNION_STATUS_COLOSSEUM_MEMBER_WAIT
    NULL,
    OneselfSeq_ColosseumMemberWaitUpdate,
    NULL,
  },
  {//UNION_STATUS_COLOSSEUM_FIRST_DATA_SHARING
    NULL,
    OneselfSeq_ColosseumFirstDataSharingUpdate,
    NULL,
  },
  {//UNION_STATUS_COLOSSEUM_NORMAL
    OneselfSeq_ColosseumInit,
    OneselfSeq_ColosseumNormal,
    NULL,
  },
  {//UNION_STATUS_COLOSSEUM_STANDPOSITION
    NULL,
    OneselfSeq_ColosseumStandPosition,
    NULL,
  },
  {//UNION_STATUS_COLOSSEUM_STANDING_BACK
    NULL,
    OneselfSeq_ColosseumStandingBack,
    NULL,
  },
  {//UNION_STATUS_COLOSSEUM_USE_PARTY_SELECT
    NULL,
    OneselfSeq_ColosseumUsePartySelect,
    NULL,
  },
  {//UNION_STATUS_COLOSSEUM_POKELIST_READY
    NULL,
    OneselfSeq_ColosseumPokelistReady,
    NULL,
  },
  {//UNION_STATUS_COLOSSEUM_POKELIST_BEFORE_DATA_SHARE
    NULL,
    OneselfSeq_ColosseumPokelistBeforeDataShare,
    NULL,
  },
  {//UNION_STATUS_COLOSSEUM_POKELIST
    NULL,
    OneselfSeq_ColosseumPokelist,
    NULL,
  },
  {//UNION_STATUS_COLOSSEUM_BATTLE_READY_WAIT
    NULL,
    OneselfSeq_ColosseumAllBattleReadyWait,
    NULL,
  },
  {//UNION_STATUS_COLOSSEUM_BATTLE
    NULL,
    OneselfSeq_ColosseumBattle,
    NULL,
  },
  {//UNION_STATUS_COLOSSEUM_LEAVE
    NULL,
    OneselfSeq_ColosseumLeaveUpdate,
    NULL,
  },
  {//UNION_STATUS_COLOSSEUM_TRAINER_CARD
    NULL,
    OneselfSeq_ColosseumTrainerCardUpdate,
    NULL,
  },
  {//UNION_STATUS_CHAT
    NULL,
    NULL,
    NULL,
  },
};
SDK_COMPILER_ASSERT(NELEMS(OneselfFuncTbl) == UNION_STATUS_MAX);



//==============================================================================
//
//  
//
//==============================================================================
int UnionOneself_Update(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  ONESELF_FUNC func;
  BOOL next_seq = TRUE;
  
  switch(situ->func_proc){
  case ONESELF_SUBPROC_INIT:
    func = OneselfFuncTbl[situ->union_status].func_init;
    break;
  case ONESELF_SUBPROC_UPDATE:
    func = OneselfFuncTbl[situ->union_status].func_update;
    break;
  case ONESELF_SUBPROC_EXIT:
    func = OneselfFuncTbl[situ->union_status].func_exit;
    break;
  }

  if(func != NULL){
    next_seq = func(unisys, situ, fieldWork, &situ->func_seq);
  }
  
  if(next_seq == TRUE){
    situ->func_seq = 0;
    situ->func_proc++;
    if(situ->func_proc > ONESELF_SUBPROC_EXIT){
      situ->func_proc = 0;
      situ->before_union_status = situ->union_status;
      situ->union_status = situ->next_union_status;
      situ->next_union_status = UNION_STATUS_NORMAL;
    }
  }
  
  return situ->union_status;
}

//==================================================================
/**
 * 次のUNION_STATUSをセットする
 *
 * @param   unisys		    
 * @param   req_status		UNION_STATUS_???
 *
 * @retval  BOOL		TRUE：成功
 */
//==================================================================
BOOL UnionOneself_ReqStatus(UNION_SYSTEM_PTR unisys, int req_status)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;

  situ->next_union_status = req_status;
  return TRUE;
}

//--------------------------------------------------------------
/**
 * メニューIndexからバトルレギュレーション番号を取得する
 * @param   play_category		
 * @retval  int		レギュレーションNo
 */
//--------------------------------------------------------------
static int Union_GetPlayCategory_to_RegulationNo(u32 play_category)
{
  switch(play_category){
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50_SHOOTER:        //対戦:1VS1:シングル:LV50
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50:        //対戦:1VS1:シングル:LV50
    return REG_LV50_SINGLE;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER:      //対戦:1VS1:シングル:制限なし
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE:      //対戦:1VS1:シングル:制限なし
    return REG_FREE_SINGLE;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD_SHOOTER://対戦:1VS1:シングル:スタンダード
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD:  //対戦:1VS1:シングル:スタンダード
    return REG_STANDARD_SINGLE;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_50_SHOOTER:        //対戦:1VS1:ダブル:LV50
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_50:        //対戦:1VS1:ダブル:LV50
    return REG_LV50_DOUBLE;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER:      //対戦:1VS1:ダブル:制限なし
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE:      //対戦:1VS1:ダブル:制限なし
    return REG_FREE_DOUBLE;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_STANDARD_SHOOTER:  //対戦:1VS1:ダブル:スタンダード
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_STANDARD:  //対戦:1VS1:ダブル:スタンダード
    return REG_STANDARD_DOUBLE;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_50_SHOOTER:
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_50:
    return REG_LV50_TRIPLE;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER:
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE:
    return REG_FREE_TRIPLE;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_STANDARD_SHOOTER:
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_STANDARD:
    return REG_STANDARD_TRIPLE;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_50_SHOOTER:
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_50:
    return REG_LV50_ROTATION;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER:
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE:
    return REG_FREE_ROTATION;
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_STANDARD_SHOOTER:
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_STANDARD:
    return REG_STANDARD_ROTATION;
  case UNION_PLAY_CATEGORY_COLOSSEUM_MULTI:                 //対戦:マルチ
    return REG_STANDARD_MALTI;
  default:
    GF_ASSERT(0);
    return REG_FREE_SINGLE;
  }
}

//--------------------------------------------------------------
/**
 * play_categoryからシューターの有無を取得する
 * @param   play_category		
 * @retval  int		TRUE:シューターあり
 * @retval  int		FALSE:シューター無し
 */
//--------------------------------------------------------------
static BOOL Union_GetPlayCategory_to_Shooter(u32 play_category)
{
  switch(play_category){
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50_SHOOTER:        //対戦:1VS1:シングル:LV50
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER:      //対戦:1VS1:シングル:制限なし
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD_SHOOTER://対戦:1VS1:シングル:スタンダード
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_50_SHOOTER:        //対戦:1VS1:ダブル:LV50
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER:      //対戦:1VS1:ダブル:制限なし
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_STANDARD_SHOOTER:  //対戦:1VS1:ダブル:スタンダード
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_50_SHOOTER:
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER:
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_STANDARD_SHOOTER:
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_50_SHOOTER:
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER:
  case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_STANDARD_SHOOTER:
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * プレイカテゴリーが戦闘のものかチェックする
 *
 * @param   play_category		
 *
 * @retval  BOOL		TRUE:戦闘　FALSE:戦闘以外
 */
//--------------------------------------------------------------
static BOOL Union_CheckPlayCategoryBattle(u32 play_category)
{
  if(play_category < UNION_PLAY_CATEGORY_COLOSSEUM_START 
      || play_category > UNION_PLAY_CATEGORY_COLOSSEUM_END){
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
/**
 * バトルボックスのPOKEPARTYをAllocして作成します
 *
 * @param   unisys		
 *
 * @retval  POKEPARTY *		AllocしたPOKEPARTY (バトルボックスが無い場合はNULL)
 */
//--------------------------------------------------------------
static POKEPARTY * _BBox_PokePartyAlloc(UNION_SYSTEM_PTR unisys)
{
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(unisys->uniparent->game_data);
  BATTLE_BOX_SAVE *bb_save = BATTLE_BOX_SAVE_GetBattleBoxSave( sv_ctrl );
  
  if(BATTLE_BOX_SAVE_IsIn( bb_save ) == FALSE){
    return NULL;
  }
  
  return BATTLE_BOX_SAVE_MakePokeParty( bb_save, HEAPID_UNION );
}

//--------------------------------------------------------------
/**
 * バトルボックスのPOKEPARTYをFreeします
 * @param   party		
 */
//--------------------------------------------------------------
static void _BBox_PokePartyFree(POKEPARTY *party)
{
  GF_ASSERT(party != NULL);
  GFL_HEAP_FreeMemory(party);
}

//--------------------------------------------------------------
/**
 * unisys->alloc.regulationにplay_categoryに対応したレギュレーションデータをロードします
 *
 * @param   play_category		
 */
//--------------------------------------------------------------
static void _Load_PlayCategory_to_Regulation(UNION_SYSTEM_PTR unisys, u32 play_category)
{
  int reg_no;

  GF_ASSERT(Union_CheckPlayCategoryBattle(play_category) == TRUE);
  
  reg_no = Union_GetPlayCategory_to_RegulationNo(play_category);
  PokeRegulation_LoadData(reg_no, unisys->alloc.regulation);
}

//--------------------------------------------------------------
/**
 * 手持ちポケモンのレギュレーションチェック
 *
 * @param   unisys		
 * @param   fail_bit		
 *
 * @retval  POKE_REG_RETURN_ENUM		
 *
 * unisys->alloc.regulationにレギュレーションデータがロードされている必要があります
 */
//--------------------------------------------------------------
static POKE_REG_RETURN_ENUM _CheckRegulation_Temoti(UNION_SYSTEM_PTR unisys, u32 *fail_bit)
{
  POKE_REG_RETURN_ENUM reg_ret;
  
  *fail_bit = 0;
  reg_ret = PokeRegulationMatchLookAtPokeParty(unisys->alloc.regulation, 
    GAMEDATA_GetMyPokemon(unisys->uniparent->game_data), fail_bit);
  return reg_ret;
}

//--------------------------------------------------------------
/**
 * バトルボックスのレギュレーションチェック
 *
 * @param   unisys		
 * @param   fail_bit		
 *
 * @retval  POKE_REG_RETURN_ENUM		
 *
 * unisys->alloc.regulationにレギュレーションデータがロードされている必要があります
 */
//--------------------------------------------------------------
static POKE_REG_RETURN_ENUM _CheckRegulation_BBox(UNION_SYSTEM_PTR unisys, u32 *fail_bit)
{
  POKE_REG_RETURN_ENUM reg_ret;
  POKEPARTY *bb_party = _BBox_PokePartyAlloc(unisys);
  
  *fail_bit = 0;
  
  if(bb_party != NULL){
    POKEPARTY *bb_party = BATTLE_BOX_SAVE_MakePokeParty( bb_save, HEAPID_UNION );
    reg_ret = PokeRegulationMatchLookAtPokeParty(
      unisys->alloc.regulation, bb_party, fail_bit);
    _BBox_PokePartyFree(bb_party);
  }
  else{ //バトルボックスのセーブデータが存在しない
    *fail_bit = 0xffffffff;
  }
  return reg_ret;
}

//--------------------------------------------------------------
/**
 * 選択メニューが戦闘だった場合、参加出来るかレギュレーションチェック
 *
 * @param   unisys		
 * @param   menu_index		
 * @param   temoti_fail_bit      手持ちNGbit代入先
 * @param   bbox_fail_bit        バトルボックスNGbit代入先
 *
 * @retval  BOOL		TRUE:どちらか片方だけでも参加OK　FALSE:両方とも参加NG
 */
//--------------------------------------------------------------
static BOOL Union_CheckEntryBattleRegulation(UNION_SYSTEM_PTR unisys, u32 menu_index,u32 *temoti_fail_bit, u32 *bbox_fail_bit)
{
  POKE_REG_RETURN_ENUM temoti_ret, bbox_ret;
  
  *temoti_fail_bit = 0;
  *bbox_fail_bit = 0;
  
  if(Union_CheckPlayCategoryBattle(menu_index) == FALSE){
    return TRUE;  //戦闘ではないのでTRUE
  }
  
  _Load_PlayCategory_to_Regulation(unisys, menu_index);

  //レギュレーションチェック
  temoti_ret = _CheckRegulation_Temoti(unisys, temoti_fail_bit);
  bbox_ret = _CheckRegulation_BBox(unisys, bbox_fail_bit);

  if(temoti_ret == POKE_REG_OK || bbox_ret == POKE_REG_OK){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 自機にポーズをかける
 *
 * @param   fieldWork		
 * @param   pause_flag		TRUE:ポーズ　FALSE:ポーズ解除
 */
//--------------------------------------------------------------
static void _PlayerMinePause(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, int pause_flag)
{
  unisys->player_pause = pause_flag;
  FIELDMAP_CTRL_GRID_SetPlayerPause( fieldWork, pause_flag );
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * 通常状態(何もしていない)：初期化
 *
 * @param   unisys		
 * @param   situ		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_NormalInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  UnionMyComm_Init(unisys, &situ->mycomm);
  UnionMySituation_SetParam(
    unisys, UNION_MYSITU_PARAM_IDX_PLAY_CATEGORY, (void*)UNION_PLAY_CATEGORY_UNION);

  UnionMsg_AllDel(unisys);

  _PlayerMinePause(unisys, fieldWork, FALSE);
  

  return TRUE;
}

//--------------------------------------------------------------
/**
 * 通常状態(何もしていない)：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_NormalUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldWork);
  MMDLSYS *fldMdlSys = FIELDMAP_GetMMdlSys( fieldWork );
  u16 obj_id, buf_no;
  s16 check_gx, check_gy, check_gz;
  MMDL *target_pc;
  
  //外部から動作リクエストを受けているなら切り替える
  if(situ->next_union_status != UNION_STATUS_NORMAL){
    OS_TPrintf("NormalUpdate:外部リクエスト受信。切り替えます next_status=%d\n", situ->next_union_status);
    return TRUE;
  }

  if(GFL_NET_GetConnectNum() > 1){
//    GF_ASSERT_MSG(0, "Normalなのに接続！！！！\n");
    OS_TPrintf("Normalなのに接続！！！！\n");
    //※check　暫定で先頭のビーコンデータを接続相手として代入しておく
    //         本来であればNormalで接続は出来ないようにする
    UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_PARENT);
    UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CONNECT_PC, situ->last_calling_pc);
    _PlayerMinePause(unisys, fieldWork, TRUE);
    return TRUE;
  }
  
  if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_A){
    FIELD_PLAYER_GetFrontGridPos(player, &check_gx, &check_gy, &check_gz);
    target_pc = MMDLSYS_SearchGridPos(fldMdlSys, check_gx, check_gz, FALSE);
    if(target_pc == NULL){
      return FALSE;
    }
    
    obj_id = MMDL_GetOBJID(target_pc);
    situ->mycomm.talk_obj_id = obj_id;
    buf_no = UNION_CHARA_GetCharaIndex_to_ParentNo(obj_id);
    OS_TPrintf("ターゲット発見! buf_no = %d, gx=%d, gz=%d\n", buf_no, check_gx, check_gz);
    if(UNION_CHARA_CheckCharaIndex(obj_id) == UNION_CHARA_INDEX_PARENT){
      if(unisys->receive_beacon[buf_no].beacon.play_category == UNION_PLAY_CATEGORY_UNION){
        UnionMySituation_SetParam(unisys, 
          UNION_MYSITU_PARAM_IDX_CALLING_PC, &unisys->receive_beacon[buf_no]);
        situ->last_calling_pc = &unisys->receive_beacon[buf_no];
        UnionOneself_ReqStatus(unisys, UNION_STATUS_CONNECT_REQ);
      }
      else{
        UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_PLAYGAME_PARENT);
      }
    }
    else{
      UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_PLAYGAME_CHILD);
    }
    _PlayerMinePause(unisys, fieldWork, TRUE);
    return TRUE;
  }
  
  //チャット入力のショートカットボタン押下チェック
  {
    FIELD_SUBSCREEN_WORK * subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);

    if(FIELD_SUBSCREEN_GetAction(subscreen) == FIELD_SUBSCREEN_ACTION_UNION_CHAT){
      UnionOneself_ReqStatus(unisys, UNION_STATUS_CHAT_CALL);
      return TRUE;
    }
  }

  //出口チェック
  if(UnionTool_CheckWayOut(fieldWork) == TRUE){
    if(unisys->debug_wayout_walk == TRUE){
      _PlayerMinePause(unisys, fieldWork, TRUE);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_LEAVE);
    }
  }
  else{
    unisys->debug_wayout_walk = TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * ユニオンルーム退出：更新
 *
 * @param   unisys		
 * @param   situ		  
 * @param   seq		    
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_Leave(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:
    GAMEBEACON_Set_UnionOut();
    unisys->finish = TRUE;
    (*seq)++;
    break;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * ユニオンルーム退出：更新
 *
 * @param   unisys		
 * @param   situ		  
 * @param   seq		    
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ChatCallUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:
    {
    	PMS_SELECT_PARAM	*initParam;
    	PMS_DATA  pmsDat;

    	initParam             = GFL_HEAP_AllocClearMemory( GFL_HEAPID_APP, sizeof( PMS_SELECT_PARAM ) );
      initParam->save_ctrl  = GAMEDATA_GetSaveControlWork(unisys->uniparent->game_data);

      unisys->parent_work = initParam;
      UnionSubProc_EventSet(unisys, UNION_SUBPROC_ID_CHAT, initParam);
    }
    (*seq)++;
    break;
  case 1:
    if(UnionSubProc_IsExits(unisys) == FALSE){
      OS_TPrintf("サブPROC終了\n");
      (*seq)++;
    }
    break;
  case 2:
    {
     	PMS_SELECT_PARAM	*initParam = unisys->parent_work;
    	PMS_DATA* pmsdata = initParam->out_pms_data;
     	
    	// 簡易会話を更新したか？
    	if( initParam->out_cancel_flag == FALSE ){
      	// ビーコンデータの簡易会話を書き換える & 通信データに反映
        UnionChat_SetMyPmsData(unisys, pmsdata);
  	  }
    }
    GFL_HEAP_FreeMemory(unisys->parent_work);
    unisys->parent_work = NULL;
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 接続リクエスト：初期化
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ConnectReqInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_000);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続リクエスト実行：更新
 *
 * @param   unisys		
 * @param   situ		  
 * @param   seq		    
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ConnectReqUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  enum{
    LOCALSEQ_INIT,
    LOCALSEQ_WAIT,
    LOCALSEQ_END,
  };
  
  switch(*seq){
  case LOCALSEQ_INIT:
    OS_TPrintf("ChangeOver モード切替：親固定\n");
    GFL_NET_ChangeoverModeSet(GFL_NET_CHANGEOVER_MODE_FIX_PARENT, TRUE, NULL);
    situ->wait = 0;
    (*seq)++;
    break;
  case LOCALSEQ_WAIT:
    if(UnionMsg_TalkStream_Check(unisys) == FALSE){
      break;
    }
    
    if(GFL_NET_GetConnectNum() > 1){
      OS_TPrintf("接続しました！：親\n");
      UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_PARENT);
      UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CONNECT_PC,situ->mycomm.calling_pc);
      UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CALLING_PC, NULL);
      (*seq)++;
    }
    else{
      situ->wait++;
      if(situ->wait > ONESELF_SERVER_TIMEOUT || (GFL_UI_KEY_GetTrg() & PAD_BUTTON_B)){
        GFL_NET_ChangeoverModeSet(GFL_NET_CHANGEOVER_MODE_NORMAL, FALSE, NULL);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
        UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CALLING_PC, NULL);
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_007);
        OS_TPrintf("子が来なかった為キャンセルしました\n");
        (*seq)++;
      }
    }
    break;
  case LOCALSEQ_END:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 接続リクエスト：終了
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ConnectReqExit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CALLING_PC, NULL);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続リクエストを受けたので接続を試みる：初期化
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ConnectAnswerInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_001);
  _PlayerMinePause(unisys, fieldWork, TRUE);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続リクエストを受けたので接続を試みる：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ConnectAnswerUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:
    OS_TPrintf("親へ接続しにいきます\n");
    GF_ASSERT(situ->mycomm.answer_pc != NULL);
    OS_TPrintf("ChangeOver モード切替：子固定\n");
    GFL_NET_ChangeoverModeSet(
      GFL_NET_CHANGEOVER_MODE_FIX_CHILD, TRUE, situ->mycomm.answer_pc->mac_address);
    situ->wait = 0;
    (*seq)++;
    break;
  case 1:
    if(UnionMsg_TalkStream_Check(unisys) == FALSE){
      break;
    }
    
    if(GFL_NET_GetConnectNum() > 1){
      OS_TPrintf("接続しました！：子\n");
      UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_CHILD);
      UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CONNECT_PC, situ->mycomm.answer_pc);
      UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_ANSWER_PC, NULL);
      (*seq)++;
    }
    else{
      situ->wait++;
      if(situ->wait > ONESELF_SERVER_TIMEOUT || (GFL_UI_KEY_GetTrg() & PAD_BUTTON_B)){
        GFL_NET_ChangeoverModeSet(GFL_NET_CHANGEOVER_MODE_NORMAL, FALSE, NULL);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
        UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_ANSWER_PC, NULL);
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_007);
        OS_TPrintf("親と接続出来なかった為キャンセルしました\n");
        (*seq)++;
      }
    }
    break;
  case 2:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 接続リクエストを受けたので接続を試みる：終了
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ConnectAnswerExit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_ANSWER_PC, NULL);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話（親）：初期化
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkInit_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  UnionMyComm_InitMenuParam(&situ->mycomm);
  UnionMySituation_SetParam(
    unisys, UNION_MYSITU_PARAM_IDX_PLAY_CATEGORY, (void*)UNION_PLAY_CATEGORY_TALK);
  UnionMyComm_PartyAdd(&situ->mycomm, situ->mycomm.connect_pc);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話（親）：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkUpdate_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:
    if(situ->mycomm.connect_pc->beacon.sex == PM_MALE){
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_002);
    }
    else{
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_002);
    }
    (*seq)++;
    break;
  case 1:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      (*seq)++;
    }
    break;
  case 2:   //メインメニュー描画
    UnionMsg_Menu_MainMenuSetup(unisys, fieldWork);
    (*seq)++;
    break;
  case 3:
    {
      u32 select_list;
      
      select_list = UnionMsg_Menu_MainMenuSelectLoop(unisys);
      switch(select_list){
      case FLDMENUFUNC_NULL:
        break;
      case FLDMENUFUNC_CANCEL:
      case UNION_MENU_SELECT_CANCEL:
        OS_TPrintf("メニューをキャンセルしました\n");
        situ->mycomm.mainmenu_select = UNION_MENU_SELECT_CANCEL;
        UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_LIST_SEND_PARENT);
        UnionMsg_Menu_MainMenuDel(unisys);
        return TRUE;
      case UNION_MENU_NO_SEND_BATTLE:
        OS_TPrintf("対戦メニューを選択\n");
        UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_BATTLE_PARENT);
        UnionMsg_Menu_MainMenuDel(unisys);
        return TRUE;
      default:
        situ->mycomm.mainmenu_select = select_list;
        UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_LIST_SEND_PARENT);
        UnionMsg_Menu_MainMenuDel(unisys);
        return TRUE;
      }
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話（親）：終了
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkExit_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  UnionMsg_Menu_MainMenuDel(unisys);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続確立後、選択したメニュー項目の送信と子の返事待ち（親）：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkListSendUpdate_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0: //相手に選択したメニューを通知
		if(UnionSend_MainMenuListResult(situ->mycomm.mainmenu_select) == TRUE){
      OS_TPrintf("リスト結果送信成功 : %d\n", situ->mycomm.mainmenu_select);
      if(situ->mycomm.mainmenu_select == UNION_MENU_SELECT_CANCEL){
        UnionOneself_ReqStatus(unisys, UNION_STATUS_SHUTDOWN);
        return TRUE;
      }
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_004);
      (*seq)++;
    }
    break;
  case 1: //返事待ち
    if(UnionMsg_TalkStream_Check(unisys) == FALSE){
      break;
    }
    if(situ->mycomm.mainmenu_yesno_result == TRUE){
      OS_TPrintf("「はい」受信\n");
      UnionOneself_ReqStatus(unisys, 
        UNION_STATUS_TRAINERCARD + situ->mycomm.mainmenu_select - UNION_PLAY_CATEGORY_TRAINERCARD);
      (*seq)++;
    }
    else if(situ->mycomm.mainmenu_yesno_result == FALSE){
      OS_TPrintf("「いいえ」受信\n");
      UnionOneself_ReqStatus(unisys, UNION_STATUS_SHUTDOWN);
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_006);
      (*seq)++;
    }
    break;
  case 2:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話（子）：初期化
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkInit_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  UnionMyComm_InitMenuParam(&situ->mycomm);
  UnionMySituation_SetParam(
    unisys, UNION_MYSITU_PARAM_IDX_PLAY_CATEGORY, (void*)UNION_PLAY_CATEGORY_TALK);
  UnionMyComm_PartyAdd(&situ->mycomm, situ->mycomm.connect_pc);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話（子）：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkUpdate_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  enum{
    _INIT,
    _SELECT_WAIT,
    _BATTLE_REG_PRINT_WAIT,
    _YESNO_SELECT,
    _YESNO_WAIT,
    _YESNO_RESULT_SEND_WAIT,
    _REG_PRINT_TEMOTI,
    _REG_PRINT_TEMOTI_WAIT,
    _REG_PRINT_BBOX,
    _REG_PRINT_BBOX_WAIT,
    _FINISH,
  };

  if(UnionMsg_TalkStream_Check(unisys) == FALSE){
    return FALSE;
  }

  switch(*seq){
  case _INIT:
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_008);
    (*seq)++;
    break;
  case _SELECT_WAIT:
    if(situ->mycomm.mainmenu_select == UNION_MENU_SELECT_NULL){
      break;
    }
    else if(situ->mycomm.mainmenu_select == UNION_MENU_SELECT_CANCEL){
      OS_TPrintf("選択メニュー受信：キャンセル\n");
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_003_06);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_SHUTDOWN);
      return TRUE;
    }
    else if(situ->mycomm.mainmenu_select < UNION_PLAY_CATEGORY_MAX){
      OS_TPrintf("選択メニュー受信：%d\n", situ->mycomm.mainmenu_select);
      UnionMsg_TalkStream_PrintPack(
        unisys, fieldWork, 
        msg_union_test_003 + situ->mycomm.mainmenu_select - UNION_PLAY_CATEGORY_TRAINERCARD);

      //戦闘の時はレギュレーション表示
      if(Union_CheckPlayCategoryBattle(situ->mycomm.mainmenu_select) == TRUE){
        _Load_PlayCategory_to_Regulation(unisys, situ->mycomm.mainmenu_select);
        UnionMsg_Menu_RegulationSetup(unisys, fieldWork, 0, 
          Union_GetPlayCategory_to_Shooter(situ->mycomm.mainmenu_select), REGWIN_TYPE_RULE);
        (*seq) = _BATTLE_REG_PRINT_WAIT;
      }
      else{
        (*seq) = _YESNO_SELECT;
      }
    }
    else{
      if(UnionSend_MainMenuListResultAnswer(FALSE) == TRUE){
        OS_TPrintf("未知の選択メニュー受信：%d\n", situ->mycomm.mainmenu_select);
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_009);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_SHUTDOWN);
        return TRUE;
      }
    }
    break;
  
  case _BATTLE_REG_PRINT_WAIT:   //戦闘であればレギュレーション表示
    if(UnionMsg_Menu_RegulationWait(unisys, fieldWork) == TRUE){
      *seq = _YESNO_SELECT;
    }
    break;
    
  case _YESNO_SELECT:   //「はい・いいえ」選択
    UnionMsg_YesNo_Setup(unisys, fieldWork);
    (*seq)++;
    break;
  case _YESNO_WAIT:
    {
      BOOL result;
      if(UnionMsg_YesNo_SelectLoop(unisys, &result) == TRUE){
        UnionMsg_YesNo_Del(unisys);
        UnionMsg_Menu_RegulationDel(unisys);
        //戦闘の時はレギュレーションを見て参加可能かチェック
        if(Union_CheckEntryBattleRegulation(unisys, situ->mycomm.mainmenu_select, 
            &situ->reg_temoti_fail_bit, &situ->reg_bbox_fail_bit) == FALSE){
          UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_010);
          result = FALSE; //強制で「いいえ」を返す
        }
        situ->mycomm.mainmenu_yesno_result = result;
        (*seq)++;
      }
    }
    break;
  case _YESNO_RESULT_SEND_WAIT: //「はい・いいえ」選択結果送信
    if(UnionSend_MainMenuListResultAnswer(situ->mycomm.mainmenu_yesno_result) == TRUE){
      if(situ->reg_temoti_fail_bit > 0 && situ->reg_bbox_fail_bit > 0){
        *seq = _REG_PRINT_TEMOTI;
      }
      else{
        *seq = _FINISH;
      }
      break;
    }
    break;
    
  case _REG_PRINT_TEMOTI:     //手持ちNGレギュレーション表示
    UnionMsg_Menu_RegulationSetup(unisys, fieldWork, situ->reg_temoti_fail_bit, 
      Union_GetPlayCategory_to_Shooter(situ->mycomm.mainmenu_select), REGWIN_TYPE_NG_TEMOTI);
    (*seq)++;
    break;
  case _REG_PRINT_TEMOTI_WAIT:
    if(UnionMsg_Menu_RegulationWait(unisys, fieldWork) == TRUE
        && (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL))){
      UnionMsg_Menu_RegulationDel(unisys);
      if(situ->reg_bbox_fail_bit == 0xffffffff){  //バトルボックスを作っていないので表示スキップ
        *seq = _FINISH;
      }
      else{
        (*seq)++;
      }
    }
    break;
  case _REG_PRINT_BBOX:       //バトルボックスNGレギュレーション表示
    UnionMsg_Menu_RegulationSetup(unisys, fieldWork, situ->reg_temoti_fail_bit, 
      Union_GetPlayCategory_to_Shooter(situ->mycomm.mainmenu_select), REGWIN_TYPE_NG_BBOX);
    (*seq)++;
    break;
  case _REG_PRINT_BBOX_WAIT:
    if(UnionMsg_Menu_RegulationWait(unisys, fieldWork) == TRUE
        && (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL))){
      UnionMsg_Menu_RegulationDel(unisys);
      (*seq) = _FINISH;
    }
    break;
    
  case _FINISH: //「はい・いいえ」選択結果送信
    if(situ->mycomm.mainmenu_yesno_result == FALSE){
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_006_01);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_SHUTDOWN);
    }
    else{
      UnionOneself_ReqStatus(unisys, 
        UNION_STATUS_TRAINERCARD + situ->mycomm.mainmenu_select-UNION_PLAY_CATEGORY_TRAINERCARD);
    }
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 接続確立後の会話（子）：終了
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkExit_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  return TRUE;
}

//--------------------------------------------------------------
/**
 * 対戦の会話（親）：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_Talk_Battle_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  BOOL next_menu, look;
  u32 select_ret;
  enum{
    LOCALSEQ_INIT,
    LOCALSEQ_MENU_SETUP,
    LOCALSEQ_MENU_LOOP,
    _RULE_LOOK,
    _RULE_LOOK_WAIT,
    _REG_PRINT_TEMOTI,
    _REG_PRINT_TEMOTI_WAIT,
    _REG_PRINT_BBOX,
    _REG_PRINT_BBOX_WAIT,
  };
  
  if(UnionMsg_TalkStream_Check(unisys) == FALSE){
    return FALSE;
  }

  switch(*seq){
  case LOCALSEQ_INIT:
    situ->work = 0;
    (*seq)++;
    break;
  case LOCALSEQ_MENU_SETUP:
    OS_TPrintf("対戦メニューセットアップ：%d\n", situ->work);
    UnionMsg_Menu_BattleMenuSetup(unisys, fieldWork, situ->work, &situ->menu_reg);  //1vs1, 2vs2
    (*seq)++;
    break;
  case LOCALSEQ_MENU_LOOP:
    select_ret = UnionMsg_Menu_BattleMenuSelectLoop(unisys, &next_menu, &situ->menu_reg, &look);
    if(next_menu == TRUE){
      situ->work = select_ret;
      UnionMsg_Menu_BattleMenuDel(unisys);
      (*seq)--;
    }
    else if(select_ret == FLDMENUFUNC_CANCEL || select_ret == UNION_MENU_SELECT_CANCEL){
      UnionMsg_Menu_MainMenuDel(unisys);
      if(situ->work > 0){
        situ->work--;
        (*seq)--;
      }
      else{
        OS_TPrintf("メニューをキャンセルしました\n");
        situ->mycomm.mainmenu_select = UNION_MENU_SELECT_CANCEL;
        UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_LIST_SEND_PARENT);
        return TRUE;
      }
    }
    else if(look == TRUE){  //ルールを見る
      UnionMsg_Menu_BattleMenuDel(unisys);
      //レギュレーションデータのロード
      _Load_PlayCategory_to_Regulation(unisys, select_ret);
      situ->mycomm.mainmenu_select = select_ret;
      *seq = _RULE_LOOK;
    }
    else if(select_ret != FLDMENUFUNC_NULL){
      UnionMsg_Menu_BattleMenuDel(unisys);
      if(Union_CheckEntryBattleRegulation(unisys, select_ret, 
          &situ->reg_temoti_fail_bit, &situ->reg_bbox_fail_bit) == FALSE){
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_010);
        (*seq) = _REG_PRINT_TEMOTI;
        break;
      }
      situ->mycomm.mainmenu_select = select_ret;
      UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_LIST_SEND_PARENT);
      return TRUE;
    }
    break;

  case _RULE_LOOK:    //ルールを見る
    UnionMsg_Menu_RegulationSetup(unisys, fieldWork, 0, 
      Union_GetPlayCategory_to_Shooter(situ->mycomm.mainmenu_select), REGWIN_TYPE_RULE);
    (*seq)++;
    break;
  case _RULE_LOOK_WAIT:
    if(UnionMsg_Menu_RegulationWait(unisys, fieldWork) == TRUE
        && (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL))){
      UnionMsg_Menu_RegulationDel(unisys);
      *seq = LOCALSEQ_MENU_SETUP;
    }
    break;

  case _REG_PRINT_TEMOTI:     //手持ちNGレギュレーション表示
    UnionMsg_Menu_RegulationSetup(unisys, fieldWork, situ->reg_temoti_fail_bit, 
      Union_GetPlayCategory_to_Shooter(situ->mycomm.mainmenu_select), REGWIN_TYPE_NG_TEMOTI);
    (*seq)++;
    break;
  case _REG_PRINT_TEMOTI_WAIT:
    if(UnionMsg_Menu_RegulationWait(unisys, fieldWork) == TRUE
        && (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL))){
      UnionMsg_Menu_RegulationDel(unisys);
      if(situ->reg_bbox_fail_bit == 0xffffffff){  //バトルボックスを作っていないので表示スキップ
        *seq = LOCALSEQ_INIT;
      }
      else{
        (*seq)++;
      }
    }
    break;
  case _REG_PRINT_BBOX:       //バトルボックスNGレギュレーション表示
    UnionMsg_Menu_RegulationSetup(unisys, fieldWork, situ->reg_temoti_fail_bit, 
      Union_GetPlayCategory_to_Shooter(situ->mycomm.mainmenu_select), REGWIN_TYPE_NG_BBOX);
    (*seq)++;
    break;
  case _REG_PRINT_BBOX_WAIT:
    if(UnionMsg_Menu_RegulationWait(unisys, fieldWork) == TRUE
        && (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL))){
      UnionMsg_Menu_RegulationDel(unisys);
      (*seq) = LOCALSEQ_INIT;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 既に遊んでいる親に話しかけた：更新
 *
 * @param   unisys		
 * @param   situ		  
 * @param   seq		    
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkPlayGameUpdate_Parent(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  UNION_PLAY_CATEGORY play_category;
  u16 buf_no;
  enum{
    LOCALSEQ_INIT,
    LOCALSEQ_END,
    
    LOCALSEQ_REGWIN_PRINT_WAIT,
    
    LOCALSEQ_YESNO_SETUP,
    LOCALSEQ_YESNO,

    _REG_PRINT_TEMOTI,
    _REG_PRINT_TEMOTI_WAIT,
    _REG_PRINT_BBOX,
    _REG_PRINT_BBOX_WAIT,
  };
  
  buf_no = UNION_CHARA_GetCharaIndex_to_ParentNo(situ->mycomm.talk_obj_id);

  switch(*seq){
  case LOCALSEQ_INIT:
    play_category = unisys->receive_beacon[buf_no].beacon.play_category;
    switch(play_category){
    case UNION_PLAY_CATEGORY_TALK:           //会話中
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_013);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
      (*seq) = LOCALSEQ_END;
      break;
    case UNION_PLAY_CATEGORY_TRAINERCARD:    //トレーナーカード
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_014);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
      (*seq) = LOCALSEQ_END;
      break;
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50_SHOOTER:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD_SHOOTER:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_50_SHOOTER:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_50:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_STANDARD_SHOOTER:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_STANDARD:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_50_SHOOTER:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_50:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_STANDARD_SHOOTER:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_STANDARD:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_50_SHOOTER:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_50:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_STANDARD_SHOOTER:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_STANDARD:
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_015);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
      (*seq) = LOCALSEQ_END;
      break;
    case UNION_PLAY_CATEGORY_COLOSSEUM_MULTI:      //マルチ
      if(unisys->receive_beacon[buf_no].beacon.connect_num < UnionMsg_GetMemberMax(play_category)){
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_016);

        //乱入可の時はレギュレーション表示
        _Load_PlayCategory_to_Regulation(unisys, play_category);
        UnionMsg_Menu_RegulationSetup(unisys, fieldWork, 0, 
          Union_GetPlayCategory_to_Shooter(play_category), REGWIN_TYPE_RULE);

        (*seq) = LOCALSEQ_REGWIN_PRINT_WAIT;
      }
      else{ //人数がいっぱい
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_016_01);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
        (*seq) = LOCALSEQ_END;
      }
      break;
    default:
      OS_TPrintf("未知の遊び play_category = %d\n", situ->play_category);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
      (*seq) = LOCALSEQ_END;
      break;
    }
    break;
  case LOCALSEQ_END:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      return TRUE;
    }
    break;
  
  case LOCALSEQ_REGWIN_PRINT_WAIT:
    if(UnionMsg_Menu_RegulationWait(unisys, fieldWork) == TRUE){
      *seq = LOCALSEQ_YESNO_SETUP;
    }
    break;
    
  case LOCALSEQ_YESNO_SETUP:   //「はい・いいえ」選択
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      UnionMsg_YesNo_Setup(unisys, fieldWork);
      (*seq)++;
    }
    break;
  case LOCALSEQ_YESNO:
    {
      BOOL result;
      if(UnionMsg_YesNo_SelectLoop(unisys, &result) == TRUE){
        UnionMsg_YesNo_Del(unisys);
        UnionMsg_Menu_RegulationDel(unisys);
        if(result == FALSE){
          UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_018);
          UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
          (*seq) = LOCALSEQ_END;
        }
        else{
          //レギュレーションを見て参加可能かチェック
          if(Union_CheckEntryBattleRegulation(unisys, situ->mycomm.mainmenu_select,
              &situ->reg_temoti_fail_bit, &situ->reg_bbox_fail_bit) == FALSE){
            UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_010);
            UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
            (*seq) = _REG_PRINT_TEMOTI;
          }
          else{
            situ->mycomm.mainmenu_yesno_result = result;
            UnionOneself_ReqStatus(unisys, UNION_STATUS_INTRUDE);
            (*seq) = LOCALSEQ_END;
          }
        }
      }
    }
    break;

  case _REG_PRINT_TEMOTI:     //手持ちNGレギュレーション表示
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      return TRUE;
    }
    UnionMsg_Menu_RegulationSetup(unisys, fieldWork, situ->reg_temoti_fail_bit, 
      Union_GetPlayCategory_to_Shooter(situ->mycomm.mainmenu_select), REGWIN_TYPE_NG_TEMOTI);
    (*seq)++;
    break;
  case _REG_PRINT_TEMOTI_WAIT:
    if(UnionMsg_Menu_RegulationWait(unisys, fieldWork) == TRUE
        && (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL))){
      UnionMsg_Menu_RegulationDel(unisys);
      if(situ->reg_bbox_fail_bit == 0xffffffff){  //バトルボックスを作っていないので表示スキップ
        *seq = LOCALSEQ_END;
      }
      else{
        (*seq)++;
      }
    }
    break;
  case _REG_PRINT_BBOX:       //バトルボックスNGレギュレーション表示
    UnionMsg_Menu_RegulationSetup(unisys, fieldWork, situ->reg_temoti_fail_bit, 
      Union_GetPlayCategory_to_Shooter(situ->mycomm.mainmenu_select), REGWIN_TYPE_NG_BBOX);
    (*seq)++;
    break;
  case _REG_PRINT_BBOX_WAIT:
    if(UnionMsg_Menu_RegulationWait(unisys, fieldWork) == TRUE
        && (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL))){
      UnionMsg_Menu_RegulationDel(unisys);
      (*seq) = LOCALSEQ_END;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 既に遊んでいる子に話しかけた：更新
 *
 * @param   unisys		
 * @param   situ		  
 * @param   seq		    
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TalkPlayGameUpdate_Child(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  UNION_PLAY_CATEGORY play_category;
  u16 buf_no;
  
  switch(*seq){
  case 0:
    buf_no = UNION_CHARA_GetCharaIndex_to_ParentNo(situ->mycomm.talk_obj_id);
    play_category = unisys->receive_beacon[buf_no].beacon.play_category;
    switch(play_category){
    case UNION_PLAY_CATEGORY_TALK:           //会話中
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_013);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
      break;
    case UNION_PLAY_CATEGORY_TRAINERCARD:    //トレーナーカード
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_014);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
      break;
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50_SHOOTER:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD_SHOOTER:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_50_SHOOTER:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_50:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_STANDARD_SHOOTER:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_STANDARD:      //コロシアム
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_50_SHOOTER:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_50:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_STANDARD_SHOOTER:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_STANDARD:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_50_SHOOTER:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_50:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_STANDARD_SHOOTER:
    case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_STANDARD:
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_015);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
      break;
    case UNION_PLAY_CATEGORY_COLOSSEUM_MULTI:      //コロシアム
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_016_01);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
      break;
    default:
      OS_TPrintf("未知の遊び play_category = %d\n", situ->play_category);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
      break;
    }
    (*seq)++;
    break;
  case 1:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * トレーナーカード呼び出し：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TrainerCardUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:   //トレーナーカードの情報を送りあう
    //同期取りの前に送信データと受信バッファを作成
    GF_ASSERT(situ->mycomm.trcard.my_card == NULL && situ->mycomm.trcard.target_card == NULL);
    situ->mycomm.trcard.my_card 
      = GFL_HEAP_AllocClearMemory(HEAPID_UNION, sizeof(TR_CARD_DATA));
    situ->mycomm.trcard.target_card 
      = GFL_HEAP_AllocClearMemory(HEAPID_UNION, sizeof(TR_CARD_DATA));
    TRAINERCARD_GetSelfData(situ->mycomm.trcard.my_card, unisys->uniparent->game_data, TRUE);
    GFL_STD_MemCopy(  //一応何かの事故で受け取れなかった時のケアの為、自分のをコピーしておく
      situ->mycomm.trcard.my_card, situ->mycomm.trcard.target_card, sizeof(TR_CARD_DATA));
    
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRAINERCARD_PARAM);
    OS_TPrintf("トレーナーカード前の同期取り開始\n");

    //「はい、どうぞ！」
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_005);

    (*seq)++;
    break;
  case 1:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRAINERCARD_PARAM) == TRUE){
      OS_TPrintf("トレーナーカード前の同期取り成功\n");
      (*seq)++;
    }
    break;
  case 2:
    if(UnionSend_TrainerCardParam(unisys) == TRUE){
      OS_TPrintf("トレーナーカード情報送信\n");
      (*seq)++;
    }
    break;
  case 3:
    if(situ->mycomm.trcard.target_card_receive == TRUE
        && UnionMsg_TalkStream_Check(unisys) == TRUE){
      OS_TPrintf("相手のカード受信\n");
      situ->mycomm.trcard.card_param = TRAINERCASR_CreateCallParam_CommData(
        unisys->uniparent->game_data, situ->mycomm.trcard.target_card, HEAPID_UNION);
      (*seq)++;
    }
    break;
  case 4:   //画面切り替え前の同期取り
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRAINERCARD_PROC_BEFORE);
    OS_TPrintf("トレーナーカード画面切り替え前の同期取り開始\n");
    (*seq)++;
    break;
  case 5:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRAINERCARD_PROC_BEFORE) == TRUE){
      OS_TPrintf("トレーナーカード画面切り替え前の同期取り成功\n");
      (*seq)++;
    }
    break;
  case 6:
    UnionSubProc_EventSet(unisys, UNION_SUBPROC_ID_TRAINERCARD, situ->mycomm.trcard.card_param);
    (*seq)++;
    break;
  case 7:
    if(UnionSubProc_IsExits(unisys) == FALSE){
      OS_TPrintf("サブPROC終了\n");
      (*seq)++;
    }
    break;
  case 8:   //トレーナーカード画面終了後の同期取り
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_008);
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRAINERCARD_PROC_AFTER);
    OS_TPrintf("トレーナーカード終了後の同期取り開始\n");
    (*seq)++;
    break;
  case 9:
    if(UnionMsg_TalkStream_Check(unisys) == FALSE){
      break;
    }
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRAINERCARD_PROC_AFTER) == TRUE){
      OS_TPrintf("トレーナーカード終了後の同期取り成功\n");
    
    #if 0//トレーナーカードのParentWorkはトレーナーカードのProc内で解放されるのでここでは解放しない
      GFL_HEAP_FreeMemory(situ->mycomm.trcard.card_param);
    #endif
      GFL_HEAP_FreeMemory(situ->mycomm.trcard.my_card);
      GFL_HEAP_FreeMemory(situ->mycomm.trcard.target_card);
      situ->mycomm.trcard.card_param = NULL;
      situ->mycomm.trcard.my_card = NULL;
      situ->mycomm.trcard.target_card = NULL;
      
      if(GFL_NET_IsParentMachine() == TRUE){
        UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_PARENT);
      }
      else{
        UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_CHILD);
      }
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * トレーナーカード呼び出し：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_TradeUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:   //トレーナーカードの情報を送りあう
    //「はい、どうぞ！」
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_005);

    (*seq)++;
    break;
  case 1:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      GFL_NET_HANDLE_TimingSyncStart(
        GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRADE_PROC_BEFORE);
      OS_TPrintf("ポケモン交換前の同期取り開始\n");
      (*seq)++;
    }
    break;
  case 2:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRADE_PROC_BEFORE) == TRUE){
      OS_TPrintf("ポケモン交換前の同期取り成功\n");
      (*seq)++;
    }
    break;
  case 3:
    {
      POKEMONTRADE_PARAM* eibw;

      eibw = GFL_HEAP_AllocClearMemory(HEAPID_UNION, sizeof(POKEMONTRADE_PARAM));
      
      eibw->gsys = unisys->uniparent->gsys;
      eibw->type = POKEMONTRADE_UNION;
      
      unisys->parent_work = eibw;
      UnionSubProc_EventSet(unisys, UNION_SUBPROC_ID_TRADE, eibw);
      (*seq)++;
    }
    break;
  case 4:
    if(UnionSubProc_IsExits(unisys) == FALSE){
      GFL_HEAP_FreeMemory(unisys->parent_work);
      unisys->parent_work = NULL;
      OS_TPrintf("サブPROC終了\n");
      (*seq)++;
    }
    break;
  case 5:
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_008);
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRADE_PROC_AFTER);
    OS_TPrintf("ポケモン交換画面終了後の同期取り開始\n");
    (*seq)++;
    break;
  case 6:
    if(UnionMsg_TalkStream_Check(unisys) == FALSE){
      break;
    }
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_TRADE_PROC_AFTER) == TRUE){
      OS_TPrintf("ポケモン交換画面終了後の同期取り成功\n");
    
      if(GFL_NET_IsParentMachine() == TRUE){
        UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_PARENT);
      }
      else{
        UnionOneself_ReqStatus(unisys, UNION_STATUS_TALK_CHILD);
      }
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 乱入処理：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_IntrudeUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  u16 buf_no = UNION_CHARA_GetCharaIndex_to_ParentNo(situ->mycomm.talk_obj_id);

  switch(*seq){
  case 0:
    {
      OS_TPrintf("乱入開始します\n");
      OS_TPrintf("ChangeOver モード切替：子固定\n");
      GFL_NET_ChangeoverModeSet(
        GFL_NET_CHANGEOVER_MODE_FIX_CHILD, TRUE, unisys->receive_beacon[buf_no].mac_address);
    }
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_012);
    situ->wait = 0;
    (*seq)++;
    break;
  case 1:
    if(UnionMsg_TalkStream_Check(unisys) == FALSE){
      break;
    }
    
    if(GFL_NET_GetConnectNum() > 1){
      OS_TPrintf("接続しました！：子\n");
      UnionMySituation_SetParam(
        unisys, UNION_MYSITU_PARAM_IDX_CONNECT_PC, &unisys->receive_beacon[buf_no]);
      situ->mycomm.intrude = TRUE;
      situ->mycomm.mainmenu_select = unisys->receive_beacon[buf_no].beacon.play_category;
      switch(situ->mycomm.mainmenu_select){
      case UNION_PLAY_CATEGORY_COLOSSEUM_MULTI:
        UnionOneself_ReqStatus(unisys, UNION_STATUS_BATTLE_MULTI);
        break;
      default:
        OS_TPrintf("設定されていないcategory = %d\n", situ->mycomm.mainmenu_select);
        GF_ASSERT(0);
        break;
      }
      (*seq)++;
    }
    else{
      situ->wait++;
      if(situ->wait > ONESELF_SERVER_TIMEOUT || (GFL_UI_KEY_GetTrg() & PAD_BUTTON_B)){
        GFL_NET_ChangeoverModeSet(GFL_NET_CHANGEOVER_MODE_NORMAL, FALSE, NULL);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_NORMAL);
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_007);
        OS_TPrintf("親と接続出来なかった為キャンセルしました\n");
        (*seq)++;
      }
    }
    break;
  case 2:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 切断処理：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ShutdownUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  if(UnionMsg_TalkStream_Check(unisys) == FALSE){
    return FALSE;
  }

  switch(*seq){
  case 0:
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_SHUTDOWN);
    OS_TPrintf("切断前の同期取り：開始\n");
    (*seq)++;
    break;
  case 1:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_SHUTDOWN) == TRUE){
      OS_TPrintf("切断前の同期取り：成功\n");
      (*seq)++;
    }
    break;
  case 2:
    UnionComm_Req_ShutdownRestarts(unisys);
    (*seq)++;
    break;
  case 3:
    if(UnionComm_Check_ShutdownRestarts(unisys) == FALSE){
      UnionMyComm_PartyDel(&situ->mycomm, situ->mycomm.connect_pc);
      UnionMySituation_SetParam(unisys, UNION_MYSITU_PARAM_IDX_CONNECT_PC, NULL);
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアム遷移：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_talkboy_03_02);
    (*seq)++;
    break;
  case 1:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      (*seq)++;
    }
    break;
  case 2:
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_COLOSSEUM_PROC_BEFORE);
    OS_TPrintf("コロシアム遷移前の同期取り開始\n");
    (*seq)++;
    break;
  case 3:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_COLOSSEUM_PROC_BEFORE) == TRUE){
      OS_TPrintf("コロシアム遷移前の同期取り成功\n");
      (*seq)++;
    }
    break;
  case 4:
    {
      int subproc_id;
      subproc_id = UNION_SUBPROC_ID_COLOSSEUM_WARP_START 
        + (situ->mycomm.mainmenu_select - UNION_PLAY_CATEGORY_COLOSSEUM_START);
      UnionSubProc_EventSet(unisys, subproc_id, NULL);
    }
    (*seq)++;
    break;
  case 5:
    if(UnionSubProc_IsExits(unisys) == FALSE){
      OS_TPrintf("コロシアム遷移のサブPROC終了\n");

      //通信プレイヤー制御システムの生成
      GF_ASSERT(unisys->colosseum_sys == NULL);
      unisys->colosseum_sys = Colosseum_InitSystem(unisys->uniparent->game_data, 
        unisys->uniparent->gsys, unisys->uniparent->mystatus, situ->mycomm.intrude);

      _PlayerMinePause(unisys, fieldWork, TRUE);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_MEMBER_WAIT);
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * マルチ：コロシアム遷移：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_MultiColosseumUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  switch(*seq){
  case 0:
    OS_TPrintf("マルチ対戦\n");
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_talkboy_03_02);
    (*seq)++;
    break;
  case 1:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      (*seq)++;
    }
    break;
  case 2:
    if(situ->mycomm.intrude == FALSE){  //乱入の場合は同期取りなし
      GFL_NET_HANDLE_TimingSyncStart(
        GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_COLOSSEUM_PROC_BEFORE);
      OS_TPrintf("コロシアム遷移前の同期取り開始\n");
    }
    (*seq)++;
    break;
  case 3:
    if(situ->mycomm.intrude == FALSE){  //乱入の場合は同期取りなし
  		if(GFL_NET_HANDLE_IsTimingSync(
  		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_COLOSSEUM_PROC_BEFORE) == TRUE){
        OS_TPrintf("コロシアム遷移前の同期取り成功\n");
        (*seq)++;
      }
    }
    else{
      (*seq)++;
    }
    break;
  case 4:
    {
      int subproc_id;
      subproc_id = UNION_SUBPROC_ID_COLOSSEUM_WARP_START 
        + (situ->mycomm.mainmenu_select - UNION_PLAY_CATEGORY_COLOSSEUM_START);
      UnionSubProc_EventSet(unisys, subproc_id, NULL);
    }
    (*seq)++;
    break;
  case 5:
    if(UnionSubProc_IsExits(unisys) == FALSE){
      OS_TPrintf("コロシアム遷移のサブPROC終了\n");

      //通信プレイヤー制御システムの生成
      GF_ASSERT(unisys->colosseum_sys == NULL);
      unisys->colosseum_sys = Colosseum_InitSystem(unisys->uniparent->game_data, 
        unisys->uniparent->gsys, unisys->uniparent->mystatus, situ->mycomm.intrude);

      _PlayerMinePause(unisys, fieldWork, TRUE);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_MEMBER_WAIT);
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアム、メンバー集合待ち：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumMemberWaitUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  int my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  int i;
  
  GF_ASSERT(clsys != NULL);
  
  switch(*seq){
  case 0:
    if(UnionMsg_GetMemberMax(situ->mycomm.mainmenu_select) == 2){ //1vs1
      clsys->mine.entry_answer = COMM_ENTRY_ANSWER_COMPLETION;
      if(GFL_NET_IsParentMachine() == TRUE){
        if(UnionSend_ColosseumEntryAllReady() == TRUE){
          *seq = 100;
        }
      }
      else{
        *seq = 100;
      }
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_011);
      break;
    }
    
    //マルチ：募集処理を行う
    if(GFL_NET_IsParentMachine() == TRUE){
      clsys->entry_menu = CommEntryMenu_Setup(fieldWork, 
        UnionMsg_GetMemberMax(situ->mycomm.mainmenu_select),
        UnionMsg_GetMemberMax(situ->mycomm.mainmenu_select), HEAPID_UNION);
    }
    else{
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_011);
    }
    (*seq)++;
    break;
  case 1:
    if(UnionSend_ColosseumEntryStatus(&clsys->basic_status[my_net_id]) == TRUE){
      if(GFL_NET_IsParentMachine() == TRUE){
        (*seq)++;
      }
      else{
        (*seq) = 100;
      }
    }
    break;
  case 2:
    if(CommEntryMenu_Update(clsys->entry_menu) == TRUE){
      (*seq)++;
    }
    for(i = 0; i < COMM_ENTRY_USER_MAX; i++){
      COMM_ENTRY_ANSWER answer;
      answer = CommEntryMenu_GetAnswer(clsys->entry_menu, i, FALSE);
      if(answer != COMM_ENTRY_ANSWER_NULL){
        if(UnionSend_ColosseumEntryAnswer(i, answer) == TRUE){
          CommEntryMenu_SetSendFlag(clsys->entry_menu, i);
        }
      }
    }
    break;
  case 3:
    {
      int count = 0;
      for(i = 0; i < COMM_ENTRY_USER_MAX; i++){
        COMM_ENTRY_ANSWER answer;
        answer = CommEntryMenu_GetAnswer(clsys->entry_menu, i, TRUE);
        if(answer != COMM_ENTRY_ANSWER_NULL){
          count++;
          if(UnionSend_ColosseumEntryAnswer(i, answer) == TRUE){
            CommEntryMenu_SetSendFlag(clsys->entry_menu, i);
          }
        }
      }
      
      //全員にエントリー結果を送信済みで、接続人数と受け入れ人数が一致していれば次に進む
      if(count == 0 
          && (GFL_NET_GetConnectNum() == CommEntryMenu_GetCompletionNum(clsys->entry_menu))){
        if(UnionSend_ColosseumEntryAllReady() == TRUE){
          CommEntryMenu_Exit(clsys->entry_menu);
          clsys->entry_menu = NULL;
          (*seq) = 100;
        }
      }
    }
    break;
    
  case 100:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      if(clsys->mine.entry_answer == COMM_ENTRY_ANSWER_COMPLETION){
        (*seq)++;
      }
      else if(clsys->mine.entry_answer == COMM_ENTRY_ANSWER_REFUSE){
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_019);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_LEAVE);
        _PlayerMinePause(unisys, fieldWork, TRUE);
        return TRUE;
      }
    }
    break;
  case 101:
    if(clsys->entry_all_ready == TRUE){
      OS_TPrintf("全員集まった\n");
      UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_FIRST_DATA_SHARING);
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアム、最初のデータ共有：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumFirstDataSharingUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  int my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());

  GF_ASSERT(clsys != NULL);
  
  switch(*seq){
  case 0:
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_COLOSSEUM_MEMBER_ENTRY_AFTER);
    OS_TPrintf("最初のデータ共有前の同期取り開始\n");
    (*seq)++;
    break;
  case 1:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_COLOSSEUM_MEMBER_ENTRY_AFTER) == TRUE){
      OS_TPrintf("最初のデータ共有前の同期取り成功\n");
      (*seq)++;
    }
    break;
  case 2:
    OS_TPrintf("コロシアムの通信テーブルAdd\n");
    Colosseum_AddCommandTable(unisys);
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_COLOSSEUM_ADD_CMD_TBL_AFTER);
    OS_TPrintf("通信テーブルをAddしたので同期取り開始\n");
    (*seq)++;
    break;
  case 3:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_COLOSSEUM_ADD_CMD_TBL_AFTER) == TRUE){
      OS_TPrintf("通信テーブルをAddしたので同期取り成功\n");
      (*seq)++;
    }
    break;
  case 4:
    OS_TPrintf("全員分の基本情報を交換し合います\n");
    if(ColosseumSend_BasicStatus(&clsys->basic_status[my_net_id], FALSE) == TRUE){
      (*seq)++;
    }
    break;
  case 5:
    if(ColosseumTool_AllReceiveCheck_BasicStatus(clsys) == TRUE){
      int i, member_num;
      member_num = GFL_NET_GetConnectNum();
      for(i = 0; i < member_num; i++){
        if(clsys->basic_status[i].occ == TRUE){
          UnionMyComm_PartyAddParam(&situ->mycomm, clsys->basic_status[i].mac_address, 
            clsys->basic_status[i].trainer_view, clsys->basic_status[i].sex);
        }
      }
      OS_TPrintf("全員分の基本情報を受信完了\n");
      (*seq)++;
    }
    break;
  case 6:
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_COLOSSEUM_CARD_BEFORE);
    OS_TPrintf("トレーナーカード交換前の同期取り開始\n");
    (*seq)++;
    break;
  case 7:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_COLOSSEUM_CARD_BEFORE) == TRUE){
      OS_TPrintf("トレーナーカード交換前の同期取り成功\n");
      (*seq)++;
    }
    break;
  case 8:
    OS_TPrintf("全員分のトレーナーカードを交換し合います\n");
    if(ColosseumSend_TrainerCard(clsys->recvbuf.tr_card[my_net_id]) == TRUE){
      (*seq)++;
    }
    break;
  case 9:
    if(ColosseumTool_AllReceiveCheck_TrainerCard(clsys) == TRUE){
      OS_TPrintf("全員分のトレーナーカードを受信完了\n");
      (*seq)++;
    }
    break;
  case 10:
    OS_TPrintf("コロシアムの準備OK! 歩き回り開始！\n");
    Colosseum_CommReadySet(clsys, TRUE);
    UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_NORMAL);
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアム、フリー移動：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumInit(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  UnionMsg_AllDel(unisys);
  _PlayerMinePause(unisys, fieldWork, FALSE);

  return TRUE;
}

//--------------------------------------------------------------
/**
 * コロシアム、フリー移動：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumNormal(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldWork);
  s16 check_gx, check_gy, check_gz;
  u32 out_index;
  
  switch(*seq){
  case 0:
    {
      int stand_pos;
   
      //立ち位置チェック
      if(ColosseumTool_CheckStandingPosition(fieldWork, 
          UnionMsg_GetMemberMax(situ->mycomm.mainmenu_select), &stand_pos) == TRUE){
        Colosseum_Mine_SetStandingPostion(clsys, stand_pos);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_STANDPOSITION);
        _PlayerMinePause(unisys, fieldWork, TRUE);
        return TRUE;
      }
      
      //出口 or 誰かの退出受信チェック
      if(ColosseumTool_ReceiveCheck_Leave(clsys) == TRUE 
          || ColosseumTool_CheckWayOut(fieldWork) == TRUE){
        UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_LEAVE);
        _PlayerMinePause(unisys, fieldWork, TRUE);
        return TRUE;
      }
      
      //話しかけチェック
      if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_A){
        FIELD_PLAYER_GetFrontGridPos(player, &check_gx, &check_gy, &check_gz);
        if(CommPlayer_SearchGridPos(clsys->cps, check_gx, check_gz, &out_index) == TRUE){
          clsys->talk_obj_id = out_index;
          OS_TPrintf("ターゲット発見! net_id = %d, gx=%d, gz=%d\n", 
            clsys->talk_obj_id, check_gx, check_gz);
          UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_TRAINER_CARD);
          _PlayerMinePause(unisys, fieldWork, TRUE);
          return TRUE;
        }
      }
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアム、立ち位置にたった：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumStandPosition(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  
  switch(*seq){
  case 0:
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_012);
    (*seq)++;
    break;
  case 1:
    if(ColosseumSend_StandingPositionConfirm(clsys) == TRUE){
      OS_TPrintf("親に立ち位置の許可要求を送信\n");
      (*seq)++;
    }
    break;
  case 2:
    if(UnionMsg_TalkStream_Check(unisys) == FALSE){
      break;
    }
    
    {
      int ret;
      ret = Colosseum_Mine_GetAnswerStandingPosition(clsys);
      if(ret == TRUE){
        OS_TPrintf("立ち位置OK\n");
        UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_USE_PARTY_SELECT);
        return TRUE;
      }
      else if(ret == FALSE){
        OS_TPrintf("立ち位置NG\n");
        UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_STANDING_BACK);
        return TRUE;
      }
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアム、立ち位置から後退する：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumStandingBack(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer(fieldWork);
  MMDL *player_mmdl = FIELD_PLAYER_GetMMdl(player);
  int stand_pos;
  u16 anm_code;
  
  switch(*seq){
  case 0:
    Colosseum_Mine_SetStandingPostion(clsys, COLOSSEUM_STANDING_POSITION_NULL);
    (*seq)++;
    break;
  case 1:
    if(ColosseumSend_StandingPositionConfirm(clsys) == TRUE){
      OS_TPrintf("親に立ち位置から退きます。を送信\n");
      (*seq)++;
    }
    break;
  case 2:
    if(Colosseum_Mine_GetAnswerStandingPosition(clsys) != COLOSSEUM_STANDING_POSITION_NULL){
      OS_TPrintf("受信：どいてOK\n");
      (*seq)++;
    }
    break;
  case 3:
    UnionMsg_AllDel(unisys);
    (*seq)++;
    break;
  case 4:
    if(MMDL_CheckPossibleAcmd(player_mmdl) == TRUE){
      ColosseumTool_CheckStandingPosition(
        fieldWork, UnionMsg_GetMemberMax(situ->mycomm.mainmenu_select), &stand_pos);
      anm_code = (stand_pos & 1) ? AC_WALK_R_16F : AC_WALK_L_16F;
      MMDL_SetAcmd(player_mmdl, anm_code);
      (*seq)++;
    }
    else{
      OS_TPrintf("MMDL_CheckPossibleAcmd待ち\n");
    }
    break;
  case 5:
    if(MMDL_CheckEndAcmd(player_mmdl) == TRUE){
      MMDL_EndAcmd(player_mmdl);
      UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_NORMAL);
      return TRUE;
    }
    break;
  }

  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアム、手持ち、バトルボックス選択：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumUsePartySelect(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  enum{
    _INIT,
    _FIRST_CHECK,
    _MENU,
    _MENU_WAIT,
    _REG_CHECK,
    _REG_FAIL_PRINT_WAIT,
    _REG_FAIL_PRINT_KEY_WAIT,
  };
  
  if(UnionMsg_TalkStream_Check(unisys) == FALSE){
    return FALSE;
  }
  
  switch(*seq){
  case _INIT:
    situ->work = 0;
    (*seq)++;
    //break;
  case _FIRST_CHECK:
    {
      SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(unisys->uniparent->game_data);
      BATTLE_BOX_SAVE *bb_save = BATTLE_BOX_SAVE_GetBattleBoxSave( sv_ctrl );
      if(BATTLE_BOX_SAVE_IsIn( bb_save ) == TRUE){
#if 0
        UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_battle_01_22);
#endif
        (*seq)++;
      }
      else{ //バトルボックスのセーブデータが存在していないなら手持ち選択したことにする
        clsys->select_pokeparty = COLOSSEUM_SELECT_PARTY_TEMOTI;
        situ->work++; //手持ちしかない事の印
        *seq = _REG_CHECK;
      }
    }
    break;
#if 0
  case _MENU:   //メインメニュー描画
    UnionMsg_Menu_PokePartySelectMenuSetup(unisys, fieldWork);
    (*seq)++;
    break;
  case _MENU_WAIT:
    {
      u32 select_list;
      
      select_list = UnionMsg_Menu_PokePartySelectMenuSelectLoop(unisys);
      switch(select_list){
      case FLDMENUFUNC_NULL:
        break;
      case FLDMENUFUNC_CANCEL:
      case COLOSSEUM_SELECT_PARTY_CANCEL:
        OS_TPrintf("メニューをキャンセルしました\n");
        UnionMsg_Menu_PokePartySelectMenuDel(unisys);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_STANDING_BACK);
        return TRUE;
      case COLOSSEUM_SELECT_PARTY_TEMOTI:
      case COLOSSEUM_SELECT_PARTY_BOX:
        UnionMsg_Menu_PokePartySelectMenuDel(unisys);
        clsys->select_pokeparty = select_list;
        (*seq)++;
        break;
      }
    }
    break;
#else

  case _MENU:   //メインメニュー描画
    {
      u32 temoti_ng_bit, bbox_ng_bit;
      BOOL temoti_fail, bbox_fail;
      
      //POKEPARTY選択リスト表示前にこちらで使っている関連したものを全て解放
      UnionMsg_AllDel(unisys);
      
      _CheckRegulation_Temoti(unisys, &temoti_ng_bit);
      _CheckRegulation_BBox(unisys, &bbox_ng_bit);
      temoti_fail = temoti_ng_bit > 0 ? TRUE : FALSE;
      bbox_fail = bbox_ng_bit > 0 ? TRUE : FALSE;
      unisys->alloc.bbox_party = _BBox_PokePartyAlloc(unisys);
      unisys->alloc.psl = PARTY_SELECT_LIST_Setup(fieldWork, 
        GAMEDATA_GetMyPokemon(unisys->uniparent->game_data),
        unisys->alloc.bbox_party, temoti_fail, bbox_fail, HEAPID_FIELDMAP);
    }
    (*seq)++;
    break;
        
  case _MENU_WAIT:
    if(PARTY_SELECT_LIST_Main(unisys->alloc.psl) == TRUE){
      SELECT_PARTY select_party = PARTY_SELECT_LIST_Exit(unisys->alloc.psl);
      unisys->alloc.psl = NULL;
      if(select_party == SELECT_PARTY_CANCEL){
        OS_TPrintf("メニューをキャンセルしました\n");
        UnionMsg_Menu_PokePartySelectMenuDel(unisys);
        UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_STANDING_BACK);
        return TRUE;
      }
      else{
        switch(select_party){
        case SELECT_PARTY_TEMOTI:
          clsys->select_pokeparty = COLOSSEUM_SELECT_PARTY_TEMOTI;
          break;
        case SELECT_PARTY_BBOX:
          clsys->select_pokeparty = COLOSSEUM_SELECT_PARTY_BOX;
          break;
        }
        _BBox_PokePartyFree(unisys->alloc.bbox_party);
        unisys->alloc.bbox_party = NULL;
        UnionMsg_Menu_PokePartySelectMenuDel(unisys);
        (*seq)++;
      }
    }
    break;

#endif
  case _REG_CHECK:
    {
      POKE_REG_RETURN_ENUM reg_ret = 0;
      u32 fail_bit = 0;
      REGWIN_TYPE regwin_type;
      
      if(clsys->select_pokeparty == COLOSSEUM_SELECT_PARTY_TEMOTI){
        reg_ret = _CheckRegulation_Temoti(unisys, &fail_bit);
        regwin_type = REGWIN_TYPE_NG_TEMOTI;
      }
      else{
        reg_ret = _CheckRegulation_BBox(unisys, &fail_bit);
        regwin_type = REGWIN_TYPE_NG_BBOX;
      }
      
      if(fail_bit > 0){
        //NGレギュレーションの表示
        UnionMsg_TalkStream_WindowSetup(unisys, fieldWork); //msgdataなどのセット
        UnionMsg_Menu_RegulationSetup(unisys, fieldWork, fail_bit, 
          Union_GetPlayCategory_to_Shooter(situ->play_category), regwin_type);
        *seq = _REG_FAIL_PRINT_WAIT;
      }
      else{
        UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_POKELIST_READY);
        return TRUE;
      }
    }
    break;
  case _REG_FAIL_PRINT_WAIT:
    if(UnionMsg_Menu_RegulationWait(unisys, fieldWork) == TRUE){
      //参加条件を満たしていない
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_battle_01_26);
      (*seq)++;
    }
    break;
  case _REG_FAIL_PRINT_KEY_WAIT:
    if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE | PAD_BUTTON_CANCEL)){
      UnionMsg_Menu_RegulationDel(unisys);
      if(situ->work == 0){
        *seq = _INIT;
      }
      else{ //手持ちしかなくてレギュレーションに引っかかっているので後退するしかない
        UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_STANDING_BACK);
        return TRUE;
      }
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアム、ポケモンリスト呼び出し前の全員待ち：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumPokelistReady(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  
  if(clsys->all_battle_ready == TRUE){
    UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_POKELIST_BEFORE_DATA_SHARE);
    clsys->all_battle_ready = FALSE;
    clsys->mine.battle_ready = FALSE;
    return TRUE;
  }
    
  switch(*seq){
  case 0:
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_battle_01_27);
    (*seq)++;
    break;
  case 1:
    if(ColosseumSend_BattleReady() == TRUE){
      clsys->mine.battle_ready = TRUE;
      (*seq)++;
    }
    break;
  case 2:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_B){
        *seq = 100;
        break;
      }
    }
    break;
  case 100:
    if(ColosseumSend_BattleReadyCancel() == TRUE){
      (*seq)++;
    }
    break;
  case 101:
    if(clsys->mine.battle_ready == FALSE){
      UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_STANDING_BACK);
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアム、ポケモンリスト呼び出し前の全員データ送受信：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumPokelistBeforeDataShare(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  int my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  
  switch(*seq){
  case 0:
    //自分の受信バッファに送信データとなるPOKEPARTYをセット
    switch(clsys->select_pokeparty){
    default:
      GF_ASSERT(0);
    case COLOSSEUM_SELECT_PARTY_TEMOTI:
      PokeParty_Copy(GAMEDATA_GetMyPokemon(unisys->uniparent->game_data), 
        clsys->recvbuf.pokeparty[my_net_id]);
      break;
    case COLOSSEUM_SELECT_PARTY_BOX:
      {
        POKEPARTY *bbox_party = _BBox_PokePartyAlloc(unisys);
        PokeParty_Copy(bbox_party, clsys->recvbuf.pokeparty[my_net_id]);
        _BBox_PokePartyFree(bbox_party);
      }
      break;
    }
    (*seq)++;
    break;
  case 1:
    ColosseumTool_Clear_ReceivePokeParty(clsys, TRUE);
    ColosseumTool_Clear_ReceiveStandingPos(clsys);
    ColosseumTool_Clear_ReceivePokeListSelected(clsys);

    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_LIST_POKEPARTY_BEFORE);
    OS_TPrintf("リスト用のPOKEPARTY送受信前の同期取り開始\n");
    (*seq)++;
    break;
  case 2:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_LIST_POKEPARTY_BEFORE) == TRUE){
      OS_TPrintf("リスト用のPOKEPARTY送受信前の同期取り成功\n");
      (*seq)++;
    }
    break;
  case 3:
    if(ColosseumSend_Pokeparty(clsys->recvbuf.pokeparty[my_net_id]) == TRUE){
      OS_TPrintf("リスト用POKEPARTY送信\n");
      (*seq)++;
    }
    break;
  case 4:
    if(ColosseumTool_AllReceiveCheck_Pokeparty(clsys) == TRUE){
      OS_TPrintf("全員分のリスト用POKEPARTY受信\n");
      (*seq)++;
    }
    break;
  case 5:
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_BATTLE_STANDINGPOS_BEFORE);
    OS_TPrintf("立ち位置送信前の同期開始\n");
    (*seq)++;
    break;
  case 6:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_BATTLE_STANDINGPOS_BEFORE) == TRUE){
      OS_TPrintf("立ち位置送信前の同期成功\n");
      (*seq)++;
    }
    break;
  case 7:
    if(GFL_NET_IsParentMachine() == TRUE){
      if(ColosseumSend_StandingPos(clsys->parentsys.stand_position) == TRUE){
        OS_TPrintf("みんなの立ち位置を親機が送信\n");
        (*seq)++;
      }
    }
    else{
      (*seq)++;
    }
    break;
  case 8:
    if(ColosseumTool_ReceiveCheck_StandingPos(clsys) == TRUE){
      OS_TPrintf("立ち位置受信\n");
      UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_POKELIST);
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアム、ポケモンリスト呼び出し：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumPokelist(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  PLIST_DATA *plist;
  UNION_SUBPROC_PARENT_POKELIST *parent_list;
  int my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  
  switch(*seq){
  case 0: //「参加するポケモンを選んでください」
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      u32 msg_id;
      switch(situ->mycomm.mainmenu_select){
      default:
        GF_ASSERT(0);
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50_SHOOTER:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_50:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_50_SHOOTER:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_50:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE:
        msg_id = msg_union_battle_01_28;
        break;
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD_SHOOTER:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_STANDARD:
        msg_id = msg_union_battle_01_29;
        break;
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_STANDARD_SHOOTER:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_STANDARD:
        msg_id = msg_union_battle_01_30;
        break;
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_50_SHOOTER:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_50:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_STANDARD_SHOOTER:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_STANDARD:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_50_SHOOTER:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_50:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_STANDARD_SHOOTER:
      case UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_STANDARD:
        msg_id = msg_union_battle_01_31;
        break;
      case UNION_PLAY_CATEGORY_COLOSSEUM_MULTI:
        msg_id = msg_union_battle_01_29;
        break;
      }
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_id);
      (*seq)++;
    }
    break;
  case 1:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      UnionMsg_AllDel(unisys);
      (*seq)++;
    }
    break;
  
  case 2: //ポケモンリスト画面呼び出し
    parent_list = GFL_HEAP_AllocClearMemory(HEAPID_UNION, sizeof(UNION_SUBPROC_PARENT_POKELIST));

    plist = &parent_list->plist;
    plist->pp = clsys->recvbuf.pokeparty[my_net_id];
    plist->myitem = GAMEDATA_GetMyItem(unisys->uniparent->game_data);
    plist->cfg = SaveData_GetConfig(GAMEDATA_GetSaveControlWork(unisys->uniparent->game_data));
    plist->mode = PL_MODE_BATTLE;
    plist->reg = unisys->alloc.regulation;
    
    {//通信相手の情報をセット
      int net_id, enemy_player = 0;
      PL_COMM_PLAYER_TYPE comm_player;
      
      for(net_id = 0; net_id < GFL_NET_GetConnectNum(); net_id++){
        if(net_id != my_net_id){
          if((my_net_id & 1) == (clsys->recvbuf.stand_position[net_id] & 1)){
            comm_player = PL_COMM_PLAYER_TYPE_ALLY;
          }
          else{
            comm_player = PL_COMM_PLAYER_TYPE_ENEMY_A + enemy_player;
            enemy_player++;
          }
          plist->comm_battle[comm_player].pp = clsys->recvbuf.pokeparty[net_id];
          plist->comm_battle[comm_player].name = clsys->basic_status[net_id].name;
          plist->comm_battle[comm_player].sex = clsys->basic_status[net_id].sex;
          OS_TPrintf("plist set net_id = %d, comm_player=%d, pp=%x\n", net_id, comm_player, clsys->recvbuf.pokeparty[net_id]);
        }
      }
    }
    if( situ->mycomm.mainmenu_select == UNION_PLAY_CATEGORY_COLOSSEUM_MULTI )
    {
      plist->comm_type = PL_COMM_MULTI;
    }
    else
    {
      plist->comm_type = PL_COMM_SINGLE;
    }
    plist->is_disp_party = TRUE;
    plist->use_tile_limit = FALSE;
    plist->time_limit = 0;
    
    unisys->parent_work = parent_list;
    UnionSubProc_EventSet(unisys, UNION_SUBPROC_ID_POKELIST, parent_list);

    (*seq)++;
    break;
  case 3:
    if(UnionSubProc_IsExits(unisys) == TRUE){
      break;
    }
    
    parent_list = unisys->parent_work;
    plist = &parent_list->plist;

    GF_ASSERT_MSG(plist->ret_mode == PL_RET_NORMAL, "plist->ret_mode 不正 %d\n", plist->ret_mode);
    switch(plist->ret_sel){
    case PL_SEL_POS_EXIT:
      GF_ASSERT(0); //キャンセルで終了は選択肢として無くなった 2009.09.16(水)
      UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_STANDING_BACK);
      (*seq)++;
      break;
    case PL_SEL_POS_ENTER:
      {//自分の受信バッファにリストで選んだ順にポケモンデータをセット
        int entry_no, temoti_no;
        POKEPARTY *temoti_party;
        POKEPARTY *dest_party = clsys->recvbuf.pokeparty[my_net_id];
        
        temoti_party = PokeParty_AllocPartyWork(HEAPID_UNION);
        PokeParty_Copy(dest_party, temoti_party);
        PokeParty_InitWork(dest_party);
        
        for(entry_no = 0; entry_no < TEMOTI_POKEMAX; entry_no++){
          if(plist->in_num[entry_no] == 0){
            break;
          }
          PokeParty_Add(dest_party, 
            PokeParty_GetMemberPointer(temoti_party, plist->in_num[entry_no] - 1));
          OS_TPrintf("ポケモン手持ち登録 entry_no=%d, in_num=%d\n", entry_no, plist->in_num[entry_no]);
        }
        
        GFL_HEAP_FreeMemory(temoti_party);
      }
      UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_BATTLE_READY_WAIT);
      (*seq)++;
      break;
    default:
      OS_TPrintf("plist->ret_sel 不正 %d\n", plist->ret_sel);
      GF_ASSERT(0);
      break;
    }
    break;
  case 4:
    GFL_HEAP_FreeMemory(unisys->parent_work);
    unisys->parent_work = NULL;
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアム、全員が戦闘の準備が完了するのを待つ：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumAllBattleReadyWait(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  
  switch(*seq){
  case 0:
    UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_battle_01_32);
    (*seq)++;
    break;
  case 1:
    if(ColosseumSend_PokeListSelected() == TRUE){
      (*seq)++;
    }
    break;
  case 2:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_BATTLE);
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアム、戦闘画面呼び出し：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumBattle(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  COLOSSEUM_BATTLE_SETUP *battle_setup;
  int my_net_id = GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle());
  int member_num, i;
  
  switch(*seq){
  case 0:
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      ColosseumTool_Clear_ReceivePokeParty(clsys, TRUE);

      GFL_NET_HANDLE_TimingSyncStart(
        GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_BATTLE_POKEPARTY_BEFORE);
      OS_TPrintf("バトル用のPOKEPARTY送受信前の同期取り開始\n");
      (*seq)++;
    }
    break;
  case 1:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_BATTLE_POKEPARTY_BEFORE) == TRUE){
      OS_TPrintf("バトル用のPOKEPARTY送受信前の同期取り成功\n");
      (*seq)++;
    }
    break;
  case 2:
    if(ColosseumSend_Pokeparty(clsys->recvbuf.pokeparty[my_net_id]) == TRUE){
      OS_TPrintf("POKEPARTY送信\n");
      (*seq)++;
    }
    break;
  case 3:
    if(ColosseumTool_AllReceiveCheck_Pokeparty(clsys) == TRUE){
      OS_TPrintf("全員分のPOKEPARTY受信\n");
      (*seq)++;
    }
    break;
  case 4:
    battle_setup = GFL_HEAP_AllocClearMemory(HEAPID_UNION, sizeof(COLOSSEUM_BATTLE_SETUP));
    
    battle_setup->partyPlayer = clsys->recvbuf.pokeparty[my_net_id];
    battle_setup->standing_pos = clsys->recvbuf.stand_position[my_net_id];
    
    unisys->parent_work = battle_setup;
    UnionSubProc_EventSet(unisys, UNION_SUBPROC_ID_BATTLE, battle_setup);

    (*seq)++;
    break;
  case 5:
    if(UnionSubProc_IsExits(unisys) == TRUE){
      break;
    }
    
    UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_STANDING_BACK);

    GFL_HEAP_FreeMemory(unisys->parent_work);
    unisys->parent_work = NULL;
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアム退出処理：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumLeaveUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;
  enum{
    LEAVE_SEQ_INIT,
    LEAVE_SEQ_SEND_LEAVE,
    LEAVE_SEQ_WAIT_ALL_LEAVE,
    LEAVE_SEQ_MSG_WAIT,
    LEAVE_SEQ_TIMING_START,
    LEAVE_SEQ_TIMING_WAIT,
    LEAVE_SEQ_SHUTDOWN,
    LEAVE_SEQ_SHUTDOWN_WAIT,
    LEAVE_SEQ_WARP_UNION,
    LEAVE_SEQ_WARP_UNION_WAIT,
    LEAVE_SEQ_BEACON_RESTART,
  };
  
  switch(*seq){
  case LEAVE_SEQ_INIT:
    _PlayerMinePause(unisys, fieldWork, TRUE);
    if(clsys->mine.entry_answer == COMM_ENTRY_ANSWER_REFUSE){
      if(UnionMsg_TalkStream_Check(unisys) == TRUE){
        *seq = LEAVE_SEQ_SHUTDOWN;
      }
    }
    else{
      UnionMsg_TalkStream_PrintPack(unisys, fieldWork, msg_union_test_019);
      (*seq)++;
    }
    break;
  case LEAVE_SEQ_SEND_LEAVE:
    if(ColosseumSend_Leave() == TRUE){
      OS_TPrintf("「退出します」送信完了\n");
      (*seq)++;
    }
    break;
  case LEAVE_SEQ_WAIT_ALL_LEAVE:
    if(ColosseumTool_AllReceiveCheck_Leave(clsys) == TRUE){
      OS_TPrintf("全員分の「退出します」を受信完了\n");
      (*seq)++;
    }
    break;
  case LEAVE_SEQ_MSG_WAIT:
    //同期後は全員同時に切断が出来るように、先にメッセージの終了を待ってから同期を始める
    if(UnionMsg_TalkStream_Check(unisys) == TRUE){
      (*seq)++;
    }
    break;
  case LEAVE_SEQ_TIMING_START:
    GFL_NET_HANDLE_TimingSyncStart(
      GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_COLOSSEUM_LEAVE);
    OS_TPrintf("コロシアム切断前の同期取り：開始\n");
    (*seq)++;
    break;
  case LEAVE_SEQ_TIMING_WAIT:
		if(GFL_NET_HANDLE_IsTimingSync(
		    GFL_NET_HANDLE_GetCurrentHandle(), UNION_TIMING_COLOSSEUM_LEAVE) == TRUE){
      OS_TPrintf("コロシアム切断前の同期取り：成功\n");
      (*seq)++;
    }
    break;
  case LEAVE_SEQ_SHUTDOWN:
    UnionComm_Req_Shutdown(unisys);
    (*seq)++;
    break;
  case LEAVE_SEQ_SHUTDOWN_WAIT:
    if(UnionComm_Check_ShutdownRestarts(unisys) == FALSE){
      OS_TPrintf("コロシアム切断完了\n");
      (*seq)++;
    }
    break;
  case LEAVE_SEQ_WARP_UNION:
    clsys->colosseum_leave = TRUE;
    UnionBeacon_ClearAllReceiveData(unisys);
    UnionMyComm_Init(unisys, &situ->mycomm);
    UnionSubProc_EventSet(unisys, UNION_SUBPROC_ID_UNION_WARP, NULL);
    (*seq)++;
    break;
  case LEAVE_SEQ_WARP_UNION_WAIT:
    if(UnionSubProc_IsExits(unisys) == FALSE){
      OS_TPrintf("ユニオン遷移のサブPROC終了\n");
      
      UnionComm_Req_Restarts(unisys);
      (*seq)++;
    }
    break;
  case LEAVE_SEQ_BEACON_RESTART:
    if(UnionComm_Check_ShutdownRestarts(unisys) == FALSE){
      OS_TPrintf("ビーコンスキャン再開完了\n");
      return TRUE;
    }
    break;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * コロシアムでのトレーナーカード呼び出し：更新
 *
 * @param   unisys		
 * @param   situ		
 * @param   fieldWork		
 * @param   seq		
 *
 * @retval  BOOL		
 */
//--------------------------------------------------------------
static BOOL OneselfSeq_ColosseumTrainerCardUpdate(UNION_SYSTEM_PTR unisys, UNION_MY_SITUATION *situ, FIELDMAP_WORK *fieldWork, u8 *seq)
{
  COLOSSEUM_SYSTEM_PTR clsys = unisys->colosseum_sys;

  switch(*seq){
  case 0:
    _PlayerMinePause(unisys, fieldWork, TRUE);

    unisys->parent_work = TRAINERCASR_CreateCallParam_CommData(
      unisys->uniparent->game_data, clsys->recvbuf.tr_card[clsys->talk_obj_id], HEAPID_UNION);

    UnionSubProc_EventSet(unisys, UNION_SUBPROC_ID_COLOSSEUM_TRAINERCARD, unisys->parent_work);
    (*seq)++;
    break;
  case 1:
    if(UnionSubProc_IsExits(unisys) == FALSE){
    #if 0//トレーナーカードのParentWorkはトレーナーカードのProc内で解放されるのでここでは解放しない
      GFL_HEAP_FreeMemory(unisys->parent_work);
    #endif
      unisys->parent_work = NULL;
      OS_TPrintf("サブPROC終了\n");
      (*seq)++;
    }
    break;
  case 2:
    UnionOneself_ReqStatus(unisys, UNION_STATUS_COLOSSEUM_NORMAL);
    return TRUE;
  }
  
  return FALSE;
}
