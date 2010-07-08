//==============================================================================
/**
 * @file    game_beacon.c
 * @brief   ゲーム中のビーコン情報管理システム
 * @author  matsuda
 * @date    2009.12.13(日)
 */
//==============================================================================
#include <gflib.h>
#include "gamesystem/game_data.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "msg/msg_beacon_status.h"
#include "savedata/encount_sv.h"
#include "savedata/misc.h"
#include "net/net_whpipe.h"
#include "savedata/my_pms_data.h"
#include "app/townmap_util.h"
#include "net_app\union\union_beacon_tool.h"
#include "app/research_radar/research_data.h"   //QUESTION_NUM_PER_TOPIC
#include "app/research_radar/question_id.h"
#include "app/research_radar/questionnaire_index.h"
#include "savedata/playtime.h"
#include "field/research_team_def.h"
#include "system/gfl_use.h"
#include "field/zonedata.h"

#include "game_beacon_local.h"

//==============================================================================
//  定数定義
//==============================================================================
///配信用マジックキー：ポケモン
#define MAGIC_KEY_DISTRIBUTION_POKE    (0x2932013a)
///配信用マジックキー：アイテム
#define MAGIC_KEY_DISTRIBUTION_ITEM    (0x48841dc1)
///配信用マジックキー：その他
#define MAGIC_KEY_DISTRIBUTION_ETC     (0x701ddc92)
///配信用マジックキー：Gパワー
#define MAGIC_KEY_DISTRIBUTION_GPOWER  (0xfee48201)

//ここのASSERTにひっかかったらupdate_logをbit管理ではなく配列管理に変更する
SDK_COMPILER_ASSERT(GAMEBEACON_SYSTEM_LOG_MAX < 32);


//==============================================================================
//  構造体定義
//==============================================================================

//==============================================================================
//  変数
//==============================================================================
///ゲームビーコン管理システムのポインタ
GAMEBEACON_SYSTEM * GameBeaconSys = NULL;

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void SendBeacon_Update(GAMEBEACON_SEND_MANAGER *send);
static void BeaconInfo_Set(GAMEBEACON_SYSTEM *bsys, const GAMEBEACON_INFO *info);
static GAMEBEACON_INFO * BeaconInfo_GetDirect(GAMEBEACON_SYSTEM *bsys, int log_no);
static GAMEBEACON_INFO * BeaconInfo_Get(GAMEBEACON_SYSTEM *bsys, int log_no);
static void SendBeacon_Init(GAMEBEACON_SEND_MANAGER *send, GAMEDATA * gamedata);

#ifdef PM_DEBUG
GAMEBEACON_INFO * DEBUG_SendBeaconInfo_GetPtr(void);
#endif


//==============================================================================
//  データ
//==============================================================================
#include "cross_comm.cdat"


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * ゲームビーコン管理システムの作成
 */
//==================================================================
void GAMEBEACON_Init(HEAPID heap_id)
{
  GF_ASSERT(GameBeaconSys == NULL);
  
  GameBeaconSys = GFL_HEAP_AllocClearMemory(heap_id, sizeof(GAMEBEACON_SYSTEM));
  GameBeaconSys->end_log = -1;
}

//==================================================================
/**
 * ゲームビーコン管理システムの破棄
 */
//==================================================================
void GAMEBEACON_Exit(void)
{
  GF_ASSERT(GameBeaconSys != NULL);
  
  GFL_HEAP_FreeMemory(GameBeaconSys);
}

//==================================================================
/**
 * ゲームビーコン管理システムの更新処理
 */
//==================================================================
void GAMEBEACON_Update(void)
{
  GAMEBEACON_SYSTEM *bsys = GameBeaconSys;
  int i;
  
  if(GameBeaconSys == NULL || GFL_NET_IsInit() == FALSE){
    return;
  }

  if(bsys->gamedata != NULL){//プレイ時間が規定数を超えていれば送信
    PLAYTIME *sv_playtime = GAMEDATA_GetPlayTimeWork( bsys->gamedata );
    u16 play_hour = PLAYTIME_GetHour( sv_playtime );
    if(bsys->backup_hour != play_hour){
      switch(play_hour){
      case 10:
      case 30:
      case 50:
      case 100:
        GAMEBEACON_Set_PlayTime(play_hour);
        break;
      }
    }
    bsys->backup_hour = play_hour;
  }

  SendBeacon_Update(&bsys->send);
  
  for(i = 0; i < GAMEBEACON_SYSTEM_LOG_MAX; i++){
    if(bsys->log[i].info.version_bit != 0){
      if(bsys->log[i].time < 0xffff){
        bsys->log[i].time++;
      }
    }
  }
}

//==================================================================
/**
 * ゲームビーコンの初期設定を行います
 *
 * @param   gamedata		ゲームデータへのポインタ
 *
 * gamedataに自分の名前などを読み込んだ後に、コールするようにしてください。
 */
//==================================================================
void GAMEBEACON_Setting(GAMEDATA *gamedata)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;

  GameBeaconSys->gamedata = gamedata;
  SendBeacon_Init(send, gamedata);

  {
    PLAYTIME *sv_playtime = GAMEDATA_GetPlayTimeWork( gamedata );
    GameBeaconSys->backup_hour = PLAYTIME_GetHour( sv_playtime );
  }
}

//==================================================================
/**
 * ゲームビーコン送信データをコピー
 *
 * @param   info		コピー先へのポインタ
 */
//==================================================================
void GAMEBEACON_SendDataCopy(GAMEBEACON_INFO *info)
{
  GFL_STD_MemCopy(&GameBeaconSys->send, info, sizeof(GAMEBEACON_INFO));
}

//==================================================================
/**
 * 送信ビーコンに変更があった場合、ネットシステムにリクエストをかける
 *
 * @param   info		
 *
 * 通信がオーバーレイされている時に呼び出すこと
 */
//==================================================================
void GAMEBEACON_SendBeaconUpdate(void)
{
  if(GameBeaconSys->send.beacon_update == TRUE){
    if(GFL_NET_IsInit() == TRUE){
      NET_WHPIPE_BeaconSetInfo();
      GameBeaconSys->send.beacon_update = FALSE;
    }
  }
}

//--------------------------------------------------------------
/**
 * 送信ビーコン更新処理
 *
 * @param   send		
 */
//--------------------------------------------------------------
static void SendBeacon_Update(GAMEBEACON_SEND_MANAGER *send)
{
  if(send->info.action.action_no != GAMEBEACON_ACTION_SEARCH){
    send->life++;
    if(send->life > CrossCommData[send->info.action.action_no].life){
      send->life = 0;
      send->info.action.action_no = GAMEBEACON_ACTION_SEARCH;
    }
  }
}

//--------------------------------------------------------------
/**
 * ログに新しいビーコン情報をセットする
 *
 * @param   bsys		
 * @param   info		セットするビーコン情報
 */
//--------------------------------------------------------------
static void BeaconInfo_Set(GAMEBEACON_SYSTEM *bsys, const GAMEBEACON_INFO *info)
{
  GAMEBEACON_LOG *setlog;
  int before_end_log;
  
  MATSUDA_Printf("GameBeaconSet chain_no = %d\n", bsys->end_log);

  before_end_log = bsys->end_log;
  bsys->end_log++;
  if(bsys->end_log >= GAMEBEACON_SYSTEM_LOG_MAX){
    bsys->end_log = 0;
  }
  setlog = &bsys->log[bsys->end_log];

  if(bsys->log_num > 0 && bsys->end_log == bsys->start_log){
    bsys->start_log++;
    if(bsys->start_log >= GAMEBEACON_SYSTEM_LOG_MAX){
      bsys->start_log = 0;
    }
  }
  
  //アンケート反映
  {
    SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(bsys->gamedata);
    QUESTIONNAIRE_SAVE_WORK *questsave = SaveData_GetQuestionnaire(sv_ctrl);
    int search_question_id, i;
    u32 answer;
    
    for(i = 0; i < QUESTION_NUM_PER_TOPIC; i++){
      search_question_id = QuestionnaireWork_GetInvestigatingQuestion(questsave, i);
      if(search_question_id != INVESTIGATING_QUESTION_NULL){
        if(search_question_id == QUESTION_ID_PLAY_TIME){
          answer = info->play_hour / 10;
          answer++; //0は無回答なので1インクリメント
          if(answer > GetAnswerNum_byQuestionID(search_question_id)){
            answer = GetAnswerNum_byQuestionID(search_question_id);
          }
        }
        else{
          answer = QuestionnaireAnswer_ReadBit(&info->question_answer, search_question_id);
        }
        
        if(answer != 0){  //0は無回答
          if(QuestionnaireWork_IsTodayAnswerNumFull(questsave, search_question_id, answer) == FALSE){
            QuestionnaireWork_AddTodayCount(questsave, search_question_id, 1);
            QuestionnaireWork_AddTodayAnswerNum(questsave, search_question_id, answer, 1);
            bsys->new_entry = TRUE;
            MATSUDA_Printf("QuestionAnswerSet id=%d, answer=%d\n", search_question_id, answer);
          }
        }
      }
    } 
  }

  // 調査隊反映
  {
    SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(bsys->gamedata);
    QUESTIONNAIRE_SAVE_WORK *questsave = SaveData_GetQuestionnaire(sv_ctrl);
    MISC *misc = SaveData_GetMisc(sv_ctrl);
    int radar_q_id, team_q_id, i, j;
    u32 answer;

    for(i = 0; i < MAX_QNUM_PER_RESEARCH_REQ; i++) {
      team_q_id = MISC_GetResearchQuestionID(misc, i);
      if(team_q_id == QUESTION_ID_DUMMY){ continue; }

      for(j = 0; j < QUESTION_NUM_PER_TOPIC; j++) { 
        radar_q_id = QuestionnaireWork_GetInvestigatingQuestion(questsave, j);
        if(team_q_id == radar_q_id) {
          if(team_q_id == QUESTION_ID_PLAY_TIME){
            answer = info->play_hour / 10;
            answer++; //0は無回答なので1インクリメント
            if(answer > GetAnswerNum_byQuestionID(team_q_id)){
              answer = GetAnswerNum_byQuestionID(team_q_id);
            }
          }
          else {
            answer = QuestionnaireAnswer_ReadBit(&info->question_answer, team_q_id);
          }
          if(answer != 0){  //0は無回答
            MISC_AddResearchCount(misc, i, 1);
          }
        }
      } 
    }
  }

  //ログにセット
  GFL_STD_MemClear(setlog, sizeof(GAMEBEACON_LOG));
  setlog->info = *info;
  setlog->time = 0;
  bsys->update_log |= 1 << bsys->end_log;
  if(bsys->log_num < GAMEBEACON_SYSTEM_LOG_MAX){
    bsys->log_num++;
  }
}

//==================================================================
/**
 * 受信ビーコンをセット
 *
 * @param   info		受信したビーコンデータへのポインタ
 * 
 * @retval  TRUE:セット完了　　FALSE:ログに同じデータを受信済み
 */
//==================================================================
BOOL GAMEBEACON_SetRecvBeacon(const GAMEBEACON_INFO *info)
{
  GAMEBEACON_SYSTEM *bsys = GameBeaconSys;
  int i;
  BOOL same_player = FALSE;
  
  if((info->version_bit & (1 << (PM_VERSION - VERSION_WHITE))) == 0){
    return FALSE; //受け取れないバージョン
  }
  
//  MATSUDA_Printf("SetRecv action_NO = %d\n", info->action.action_no);
  if(info->action.action_no == GAMEBEACON_ACTION_NULL || info->action.action_no >= GAMEBEACON_ACTION_MAX){
    return FALSE;
  }
  if(info->action.action_no == GAMEBEACON_ACTION_THANKYOU
      && GameBeaconSys->send.info.trainer_id != info->action.thanks.target_trainer_id){
    return FALSE;
  }
  
  if(bsys->start_log <= bsys->end_log){
    for(i = bsys->start_log; i <= bsys->end_log; i++){
      if(bsys->log[i].info.trainer_id == info->trainer_id){
        if(info->action.action_no == GAMEBEACON_ACTION_SEARCH 
            || bsys->log[i].info.send_counter == info->send_counter){
          return FALSE; //ログに同じデータを受信済み
        }
        same_player = TRUE; //1プレイヤーは対になった1つのログバッファしか持たない
        break;
      }
    }
  }
  else{
    for(i = bsys->start_log; i < GAMEBEACON_SYSTEM_LOG_MAX; i++){
      if(bsys->log[i].info.trainer_id == info->trainer_id){
        if(info->action.action_no == GAMEBEACON_ACTION_SEARCH 
            || bsys->log[i].info.send_counter == info->send_counter){
          return FALSE; //ログに同じデータを受信済み
        }
        same_player = TRUE; //1プレイヤーは対になった1つのログバッファしか持たない
        break;
      }
    }
    for(i = 0; i <= bsys->end_log; i++){
      if(bsys->log[i].info.trainer_id == info->trainer_id){
        if(info->action.action_no == GAMEBEACON_ACTION_SEARCH 
            || bsys->log[i].info.send_counter == info->send_counter){
          return FALSE; //ログに同じデータを受信済み
        }
        same_player = TRUE; //1プレイヤーは対になった1つのログバッファしか持たない
        break;
      }
    }
  }
  
  //受信ビーコンデータ整合性チェック
  if(info->action.action_no == GAMEBEACON_ACTION_DISTRIBUTION_POKE){
    if(info->action.distribution.magic_key != MAGIC_KEY_DISTRIBUTION_POKE){
      return FALSE; //マジックキー不一致の為、受け取り拒否
    }
  }
  else if(info->action.action_no == GAMEBEACON_ACTION_DISTRIBUTION_ITEM){
    if(info->action.distribution.magic_key != MAGIC_KEY_DISTRIBUTION_ITEM){
      return FALSE; //マジックキー不一致の為、受け取り拒否
    }
  }
  else if(info->action.action_no == GAMEBEACON_ACTION_DISTRIBUTION_ETC){
    if(info->action.distribution.magic_key != MAGIC_KEY_DISTRIBUTION_ETC){
      return FALSE; //マジックキー不一致の為、受け取り拒否
    }
  }
  else if(info->action.action_no == GAMEBEACON_ACTION_DISTRIBUTION_GPOWER){
    if(info->action.distribution.magic_key != MAGIC_KEY_DISTRIBUTION_GPOWER){
      return FALSE; //マジックキー不一致の為、受け取り拒否
    }
  }
  
  if(same_player == TRUE){
    bsys->log[i].info = *info;
    bsys->log[i].time = 0;
    bsys->update_log |= 1 << i;
    MATSUDA_Printf("sameBeacon %d件目\n", i);
  }
  else{
    BeaconInfo_Set(bsys, info);
    bsys->log_count++;
    MATSUDA_Printf("セット完了 %d件目 id=%d\n", bsys->log_count, info->trainer_id);
  }

  return TRUE;
}

//--------------------------------------------------------------
/**
 * ログからビーコン情報を取得する
 *
 * @param   bsys		
 * @param   log_no  ログのバッファ番号
 *
 * @retval  GAMEBEACON_INFO *		NULLの場合はデータ無し
 */
//--------------------------------------------------------------
static GAMEBEACON_INFO * BeaconInfo_GetDirect(GAMEBEACON_SYSTEM *bsys, int log_no)
{
  if(bsys->log_num == 0 || bsys->log_num - log_no <= 0){
    return NULL;  //データ無し
  }
  
  return &bsys->log[log_no].info;
}

//==================================================================
/**
 * ログからビーコン情報を取得する
 *
 * @param   log_no  ログのバッファ番号
 *
 * @retval  GAMEBEACON_INFO *		NULLの場合はデータ無し
 */
//==================================================================
const GAMEBEACON_INFO * GAMEBEACON_Get_BeaconLogDirect(int log_no)
{
  return BeaconInfo_GetDirect(GameBeaconSys, log_no);
}

//--------------------------------------------------------------
/**
 * ログからビーコン情報を取得する
 *
 * @param   bsys		
 * @param   log_no  ログ番号
 *
 * @retval  GAMEBEACON_INFO *		NULLの場合はデータ無し
 */
//--------------------------------------------------------------
static GAMEBEACON_INFO * BeaconInfo_Get(GAMEBEACON_SYSTEM *bsys, int log_no)
{
  int log_pos, offset;
  
  if(bsys->log_count == 0 || bsys->log_count - log_no <= 0){
    return NULL;  //データ無し
  }
  
  offset = (bsys->log_count - 1) - log_no;
  log_pos = bsys->end_log - offset;
  if(MATH_ABS(log_pos) >= GAMEBEACON_SYSTEM_LOG_MAX){
    return NULL;  //受信バッファ数を超えたバッファ値にアクセスしようとしている
  }
  
  if(log_pos < 0){
    log_pos = GAMEBEACON_SYSTEM_LOG_MAX + log_pos;
  }
  return &bsys->log[log_pos].info;
}

//==================================================================
/**
 * ログからビーコン情報を取得する
 *
 * @param   log_no  ログ番号
 *
 * @retval  GAMEBEACON_INFO *		NULLの場合はデータ無し
 */
//==================================================================
const GAMEBEACON_INFO * GAMEBEACON_Get_BeaconLog(int log_no)
{
  return BeaconInfo_Get(GameBeaconSys, log_no);
}

//==================================================================
/**
 * 現在までのトータル受信ログ件数を取得
 *
 * @retval  u32		
 */
//==================================================================
u32 GAMEBEACON_Get_LogCount(void)
{
  return GameBeaconSys->log_count;
}

//==================================================================
/**
 * 新着のあったログ番号を取得する
 *
 * @param   start_log_no		検索開始位置(先頭から検索する場合は0を指定)
 *
 * @retval  int		新着のあったログ番号(GAMEBEACON_SYSTEM_LOG_MAXの場合は新着無し)
 *
 * start_log_noの位置から調べ始めて、最初に新着があったログに引っかかった時点で
 * returnします。
 * start_log_noは新着に引っかかって終了した場合、start_log_noは引っかかった位置+1の状態になります
 */
//==================================================================
int GAMEBEACON_Get_UpdateLogNo(int *start_log_no)
{
  for( ; (*start_log_no) < GAMEBEACON_SYSTEM_LOG_MAX; (*start_log_no)++){
    if(GameBeaconSys->update_log & (1 << (*start_log_no))){
      (*start_log_no)++;
      return (*start_log_no) - 1;
    }
  }
  return GAMEBEACON_SYSTEM_LOG_MAX;
}

//==================================================================
/**
 * 新着フラグをリセットする
 *
 * @param   log_no		
 */
//==================================================================
void GAMEBEACON_Reset_UpdateFlag(int log_no)
{
  GF_ASSERT(log_no < GAMEBEACON_SYSTEM_LOG_MAX);
  GameBeaconSys->update_log &= 0xffffffff ^ (1 << log_no);
}

//==================================================================
/**
 * 受信バッファの中に同じ人のビーコン情報を検索し、見つかれば新着フラグをONにする
 * @param   info		検索対象のプレイヤーのビーコン情報
 * @retval  BOOL		TRUE:ONにした。　FALSE:見つからなかった
 */
//==================================================================
BOOL GAMEBEACON_Set_SearchUpdateFlag(const GAMEBEACON_INFO *info)
{
  GAMEBEACON_SYSTEM *bsys = GameBeaconSys;
  int i;
  
  for(i = 0; i < GAMEBEACON_SYSTEM_LOG_MAX; i++){
    if(bsys->log[i].info.trainer_id == info->trainer_id){
      bsys->update_log |= 1 << i;
      MATSUDA_Printf("update on %d\n", i);
      return TRUE;
    }
  }
  return FALSE;
}

//==================================================================
/**
 * 受信ビーコンバッファから指定トレーナーIDのビーコンを検索し、最後のデータを受信してから何フレーム経過しているかを取得する
 *
 * @param   trainer_id		検索対象のトレーナーID(下位2バイト)
 *
 * @retval  u16		最後のデータを受信してからの経過フレーム数
 *                受信バッファに指定したユーザーが居ない場合は0xffff
 */
//==================================================================
u16 GAMEBEACON_Get_RecvBeaconTime(u16 trainer_id)
{
  GAMEBEACON_SYSTEM *bsys = GameBeaconSys;
  int i;
  
  for(i = 0; i < GAMEBEACON_SYSTEM_LOG_MAX; i++){
    if(bsys->log[i].info.version_bit != 0 && bsys->log[i].info.trainer_id == trainer_id){
      return bsys->log[i].time;
    }
  }
  return 0xffff;
}



//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * 送信ビーコンに初期データをセット
 *
 * @param   send		
 * @param   gamedata		
 */
//--------------------------------------------------------------
static void SendBeacon_Init(GAMEBEACON_SEND_MANAGER *send, GAMEDATA * gamedata)
{
  GAMEBEACON_INFO *info = &send->info;
  MYSTATUS *myst = GAMEDATA_GetMyStatus(gamedata);
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata);
  const MISC *misc = SaveData_GetMisc( sv );
  const MYPMS_DATA *mypms = SaveData_GetMyPmsDataConst( sv );
  OSOwnerInfo owner_info;
  PMS_DATA pms;
  
  OS_GetOwnerInfo(&owner_info);
  
  info->version_bit = 0xffff; //全バージョン指定
  info->zone_id = PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork(gamedata));
  if(ZONEDATA_IsUnionRoom(info->zone_id) == TRUE || ZONEDATA_IsColosseum(info->zone_id) == TRUE){
    const LOCATION *loc = GAMEDATA_GetSpecialLocation(gamedata);
    info->zone_id = loc->zone_id;
  }
  info->townmap_root_zone_id = TOWNMAP_UTIL_GetRootZoneID(gamedata, info->zone_id);
  info->g_power_id = GPOWER_ID_NULL;
  info->trainer_id = MyStatus_GetID_Low(myst);
  info->favorite_color_index = owner_info.favoriteColor;
  info->sex = MyStatus_GetMySex( myst );
  if(info->sex == PM_MALE){
    info->trainer_view = MyStatus_GetTrainerView(myst);
  }
  else{ //3bitに収める
    info->trainer_view = MyStatus_GetTrainerView(myst) - UNION_VIEW_INDEX_WOMAN_START;
  }
  info->pm_version = GET_VERSION(); //PM_VERSION;
  info->language = PM_LANG;
  info->nation = MyStatus_GetMyNation(myst);
  info->area = MyStatus_GetMyArea(myst);
  info->research_team_rank = MISC_CrossComm_GetResearchTeamRank(misc);
  
  info->thanks_recv_count = MISC_CrossComm_GetThanksRecvCount(misc);
  info->suretigai_count = MISC_CrossComm_GetSuretigaiCount(misc);
  
  {
    PLAYTIME *sv_playtime = GAMEDATA_GetPlayTimeWork( gamedata );
    info->play_hour = PLAYTIME_GetHour( sv_playtime );
    info->play_min = PLAYTIME_GetMinute( sv_playtime );
  }
  
  {
    QUESTIONNAIRE_SAVE_WORK *qsw = SaveData_GetQuestionnaire(sv);
    QUESTIONNAIRE_ANSWER_WORK *ans = Questionnaire_GetAnswerWork(qsw);
    GFL_STD_MemCopy(ans, &info->question_answer, sizeof(QUESTIONNAIRE_ANSWER_WORK));
  }
  
  MYPMS_GetPms( mypms, MYPMS_PMS_TYPE_INTRODUCTION, &pms );
  GAMEBEACON_Set_Details_IntroductionPms(&pms);

  { //名前＆自己紹介文コピー　文字数がFullの場合はEOMを差し込まないので
    //STRTOOL_Copyは使用せずに独自コピー
    STRCODE code_eom = GFL_STR_GetEOMCode();
    const STRCODE *src_code;
    int i;
    
    src_code = MyStatus_GetMyName(myst);
    for(i = 0; i < PERSON_NAME_SIZE; i++){
      info->name[i] = src_code[i];
      if(src_code[i] == code_eom){
        break;
      }
    }

    src_code = MISC_CrossComm_GetSelfIntroduction(misc);
    for(i = 0; i < GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN; i++){
      info->self_introduction[i] = src_code[i];
      if(src_code[i] == code_eom){
        break;
      }
    }
  }
  
  //詳細情報
  BEACONINFO_Set_Details_Walk(info);

  //行動パラメータ
  info->action.action_no = GAMEBEACON_ACTION_NULL;
}

//--------------------------------------------------------------
/**
 * 送信ビーコンをセットした時の共通処理
 *
 * @param   send		
 * @param   info		
 */
//--------------------------------------------------------------
void GAMEBEACON_SUB_SendBeaconSetCommon(GAMEBEACON_SEND_MANAGER *send)
{
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(GameBeaconSys->gamedata);
  PLAYTIME *sv_playtime = GAMEDATA_GetPlayTimeWork( GameBeaconSys->gamedata );
  const MISC *misc = SaveData_GetMisc( sv_ctrl );
  
  send->info.play_hour = PLAYTIME_GetHour( sv_playtime );
  send->info.play_min = PLAYTIME_GetMinute( sv_playtime );

  send->info.thanks_recv_count = MISC_CrossComm_GetThanksRecvCount(misc);
  send->info.suretigai_count = MISC_CrossComm_GetSuretigaiCount(misc);

  send->info.send_counter++;
  send->life = 0;
  send->beacon_update = TRUE;
}

//--------------------------------------------------------------
/**
 * アクション番号のプライオリティチェック
 *
 * @param   action_no		
 *
 * @retval  BOOL		    TRUE:プライオリティが同等以上。　FALSE:プライオリティが下
 */
//--------------------------------------------------------------
BOOL GAMEBEACON_SUB_CheckPriority(u16 action_no)
{
#ifdef PM_DEBUG
  if(GameBeaconSys->deb_beacon_priority_egnore){
    //TRUEならビーコンリクエスト時にプライオリティ判定を無視する
    return TRUE;
  }
#endif
  if(CrossCommData[action_no].priority < CrossCommData[GameBeaconSys->send.info.action.action_no].priority){
    return FALSE;
  }
  return TRUE;
}

//==================================================================
/**
 * ポケモン関連ビーコン タイプチェック
 *
 * @param   monsno
 */
//==================================================================
BOOL GAMEBEACON_SUB_PokeBeaconTypeIsSpecial(const monsno )
{
  int i;
  static const u16 sp_poke[] = {
    MONSNO_530,	//ゾロアーク
    MONSNO_645, //MONSNO_ATOSU, 
    MONSNO_646, //MONSNO_PORUTOSU, 
    MONSNO_647, //MONSNO_ARAMISU, 
    MONSNO_648, //MONSNO_KAZAKAMI, 
    MONSNO_649, //MONSNO_RAIKAMI, 
    MONSNO_650, //MONSNO_SIN, 
    MONSNO_651, //MONSNO_MU, 
    MONSNO_652, //MONSNO_TUTINOKAMI, 
    MONSNO_653, //MONSNO_RAI, 
    MONSNO_654, //MONSNO_DARUTANISU, 
    MONSNO_655, //MONSNO_MERODHIA, 
    MONSNO_656, //MONSNO_INSEKUTA, 
    MONSNO_657, //MONSNO_BIKUTHI, 
  };

  for(i = 0;i < NELEMS(sp_poke);i++){
    if( sp_poke[i] == monsno ){
      return TRUE;
    }
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * ニックネームのコピー　※文字数がFullの場合はEOMを差し込まないので独自コピー処理
 *
 * @param   nickname		    コピー元ニックネーム
 * @param   dest_nickname		ニックネームコピー先
 */
//--------------------------------------------------------------
void GAMEBEACON_SUB_StrbufNicknameCopy(const STRBUF *nickname, STRCODE *dest_nickname)
{
  STRCODE temp_name[BUFLEN_POKEMON_NAME];
  STRCODE code_eom = GFL_STR_GetEOMCode();
  int i;

  //文字数がFullの場合はEOMを差し込まないので独自コピー

  //STRBUFに入っているSTRCODEをEOMも込みで展開できる一時バッファに取り出す
  GFL_STR_GetStringCode(nickname, temp_name, BUFLEN_POKEMON_NAME);
  for(i = 0; i < MONS_NAME_SIZE; i++){
    dest_nickname[i] = temp_name[i];
    if(temp_name[i] == code_eom){
      break;
    }
  }
}

//==============================================================================
//
//  アンケート
//
//==============================================================================
//==================================================================
/**
 * アンケート用：新しい人物とすれ違ったかを取得する
 *
 * @retval  BOOL		TRUE:新しい人物とすれ違った　　FALSE:すれ違っていない
 */
//==================================================================
BOOL GAMEBEACON_Get_NewEntry(void)
{
  int new_entry;
  
  if(GameBeaconSys == NULL){
    return FALSE;
  }
  new_entry = GameBeaconSys->new_entry;
  GameBeaconSys->new_entry = FALSE;
  return new_entry;
}


//==============================================================================
//
//  送信データ更新
//      次回のビーコン送信に備えて送信バッファの値を更新する
//      send_counterは増やさない為、既に同じアクションを受け取っている人に反映されるのは
//      別のアクションが発生してからになる。
//
//==============================================================================
//==================================================================
/**
 * 送信データ更新：自分の最新のアンケートデータをビーコン送信バッファに反映する
 *
 * @param   my_ans		自分のアンケートデータへのポインタ
 */
//==================================================================
void GAMEBEACON_SendDataUpdate_Questionnaire(QUESTIONNAIRE_ANSWER_WORK *my_ans)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  GAMEBEACON_INFO *info = &send->info;

  GFL_STD_MemCopy(my_ans, &info->question_answer, sizeof(QUESTIONNAIRE_ANSWER_WORK));
  send->beacon_update = TRUE;
}

//==================================================================
/**
 * 送信データ更新：国コード、地域コードをビーコン送信バッファに反映する
 *
 * @param   nation		国コード
 * @param   area		  地域コード
 */
//==================================================================
void GAMEBEACON_SendDataUpdate_NationArea(u8 nation, u8 area)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  GAMEBEACON_INFO *info = &send->info;

  info->nation = nation;  //MyStatus_GetMyNation(myst);
  info->area = area;  //MyStatus_GetMyArea(myst);
  send->beacon_update = TRUE;
}

//==================================================================
/**
 * 送信データ更新：調査隊員ランクを送信バッファに反映する
 *
 * @param   research_team_rank		
 */
//==================================================================
void GAMEBEACON_SendDataUpdate_ResearchTeamRank(u8 research_team_rank)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  GAMEBEACON_INFO *info = &send->info;
  info->research_team_rank = research_team_rank;  //MISC_CrossComm_GetResearchTeamRank(misc);
  send->beacon_update = TRUE;
}

//==================================================================
/**
 * 送信データ更新：トレーナーカードの自己紹介文を送信バッファに反映する
 *
 * @param   pms		
 */
//==================================================================
void GAMEBEACON_SendDataUpdate_TrCardIntroduction(const PMS_DATA *pms)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  GAMEBEACON_Set_Details_IntroductionPms(pms);
  send->beacon_update = TRUE;
}

//==================================================================
/**
 * 送信データ更新：トレーナーカードのユニオンの見た目を送信バッファに反映する
 *
 * @param   trainer_view		
 */
//==================================================================
void GAMEBEACON_SendDataUpdate_TrainerView(int trainer_view)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  GAMEBEACON_INFO *info = &send->info;
  info->trainer_view = trainer_view;
  send->beacon_update = TRUE;
}

//==================================================================
/**
 * 送信データ更新：すれ違い通信の自己紹介文を送信バッファに反映する
 */
//==================================================================
void GAMEBEACON_SendDataUpdate_SelfIntroduction(void)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  GAMEBEACON_INFO *info = &send->info;
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(GameBeaconSys->gamedata);
  const MISC *misc = SaveData_GetMisc( sv_ctrl );
  const STRCODE *src_code;
  STRCODE code_eom = GFL_STR_GetEOMCode();
  int i;
  
  src_code = MISC_CrossComm_GetSelfIntroduction(misc);
  for(i = 0; i < GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN; i++){
    info->self_introduction[i] = src_code[i];
    if(src_code[i] == code_eom){
      break;
    }
  }
  send->beacon_update = TRUE;
}



//==============================================================================
//
//  ビーコンセット
//
//==============================================================================
//==================================================================
/**
 * 送信ビーコンセット：ポケモン捕獲
 *
 * @param   monsno    ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_PokemonGet(u16 monsno)
{
  if( GAMEBEACON_SUB_PokeBeaconTypeIsSpecial( monsno )){
    GAMEBEACON_Set_SpecialPokemonGet(monsno);
  }else{
    GAMEBEACON_Set_WildPokemonGet(monsno);
  }
}

//==================================================================
/**
 * 送信ビーコンセット：野生ポケモン捕獲
 *
 * @param   monsno    ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_WildPokemonGet(u16 monsno)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_POKE_GET) == FALSE){ return; };
  BEACONINFO_Set_WildPokemonGet(&GameBeaconSys->send.info, monsno);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ポケモン捕獲
 *
 * @param   monsno    ポケモン番号
 */
//==================================================================
void BEACONINFO_Set_WildPokemonGet(GAMEBEACON_INFO *info, u16 monsno)
{
  info->action.action_no = GAMEBEACON_ACTION_POKE_GET;
  info->action.monsno = monsno;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：特別なポケモン捕獲
 *
 * @param   monsno    ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_SpecialPokemonGet(u16 monsno)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_SP_POKE_GET) == FALSE){ return; };
  BEACONINFO_Set_SpecialPokemonGet(&GameBeaconSys->send.info, monsno);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：特別なポケモン捕獲
 *
 * @param   monsno    ポケモン番号
 */
//==================================================================
void BEACONINFO_Set_SpecialPokemonGet(GAMEBEACON_INFO *info, u16 monsno)
{
  info->action.action_no = GAMEBEACON_ACTION_SP_POKE_GET;
  info->action.monsno = monsno;

  BEACONINFO_Set_Details_Walk(info);
}


//==================================================================
/**
 * 送信ビーコンセット：ポケモンレベルアップ
 *
 * @param   nickname		対象ポケモンのニックネーム
 */
//==================================================================
void GAMEBEACON_Set_PokemonLevelUp(const STRBUF *nickname)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_POKE_LVUP) == FALSE){ return; };
  BEACONINFO_Set_PokemonLevelUp(&GameBeaconSys->send.info, nickname);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ポケモンレベルアップ
 *
 * @param   nickname		対象ポケモンのニックネーム
 */
//==================================================================
void BEACONINFO_Set_PokemonLevelUp(GAMEBEACON_INFO *info, const STRBUF *nickname)
{
  info->action.action_no = GAMEBEACON_ACTION_POKE_LVUP;
  GAMEBEACON_SUB_StrbufNicknameCopy(nickname, info->action.normal.nickname);

  //フィールドと戦闘、両方から呼ばれるため、詳細は直前を維持
}

//==================================================================
/**
 * 送信ビーコンセット：ポケモン進化
 *
 * @param   monsno  		対象ポケモンのポケモン番号
 * @param   nickname		対象ポケモンのニックネーム
 */
//==================================================================
void GAMEBEACON_Set_PokemonEvolution(u16 monsno, const STRBUF *nickname)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_POKE_EVOLUTION) == FALSE){ return; };
  BEACONINFO_Set_PokemonEvolution(&GameBeaconSys->send.info, monsno, nickname);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ポケモン進化
 *
 * @param   monsno  		対象ポケモンのポケモン番号
 * @param   nickname		対象ポケモンのニックネーム
 */
//==================================================================
void BEACONINFO_Set_PokemonEvolution(GAMEBEACON_INFO *info, u16 monsno, const STRBUF *nickname)
{
  info->action.action_no = GAMEBEACON_ACTION_POKE_EVOLUTION;
  info->action.monsno = monsno;
  GAMEBEACON_SUB_StrbufNicknameCopy(nickname, info->action.normal.nickname);

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：プレイヤーのプレイ時間が規定数を超えた
 *
 * @param   hour    時
 */
//==================================================================
void GAMEBEACON_Set_PlayTime(u32 hour)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_PLAYTIME) == FALSE){ return; };
  BEACONINFO_Set_PlayTime(&GameBeaconSys->send.info, hour);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：プレイヤーのプレイ時間が規定数を超えた
 *
 * @param   hour    時
 */
//==================================================================
void BEACONINFO_Set_PlayTime(GAMEBEACON_INFO *info, u32 hour)
{
  info->action.action_no = GAMEBEACON_ACTION_PLAYTIME;
  info->action.hour = hour;

  BEACONINFO_Set_Details_Walk(info);
}


#ifdef PM_DEBUG //イベント用の配布ROMでしか発信はしないのでテスト用
//==================================================================
/**
 * 送信ビーコンセット：ダウンロード配信(ポケモン)実施中
 *
 * @param   monsno		ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_DistributionPoke(u16 monsno)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_DISTRIBUTION_POKE) == FALSE){ return; };
  BEACONINFO_Set_DistributionPoke(&GameBeaconSys->send.info, monsno);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ダウンロード配信(ポケモン)実施中
 *
 * @param   monsno		ポケモン番号
 */
//==================================================================
void BEACONINFO_Set_DistributionPoke(GAMEBEACON_INFO *info, u16 monsno)
{
  info->action.action_no = GAMEBEACON_ACTION_DISTRIBUTION_POKE;
  info->action.distribution.monsno = monsno;
  info->action.distribution.magic_key = MAGIC_KEY_DISTRIBUTION_POKE;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：ダウンロード配信(アイテム)実施中
 *
 * @param   item		アイテム番号
 */
//==================================================================
void GAMEBEACON_Set_DistributionItem(u16 item)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_DISTRIBUTION_ITEM) == FALSE){ return; };
  BEACONINFO_Set_DistributionItem(&GameBeaconSys->send.info, item);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ダウンロード配信(アイテム)実施中
 *
 * @param   item		アイテム番号
 */
//==================================================================
void BEACONINFO_Set_DistributionItem(GAMEBEACON_INFO *info, u16 item)
{
  info->action.action_no = GAMEBEACON_ACTION_DISTRIBUTION_ITEM;
  info->action.distribution.itemno = item;
  info->action.distribution.magic_key = MAGIC_KEY_DISTRIBUTION_ITEM;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：ダウンロード配信(その他)実施中
 */
//==================================================================
void GAMEBEACON_Set_DistributionEtc(void)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_DISTRIBUTION_ETC) == FALSE){ return; };
  BEACONINFO_Set_DistributionEtc(&GameBeaconSys->send.info);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ダウンロード配信(その他)実施中
 */
//==================================================================
void BEACONINFO_Set_DistributionEtc(GAMEBEACON_INFO *info)
{
  info->action.action_no = GAMEBEACON_ACTION_DISTRIBUTION_ETC;
  info->action.distribution.magic_key = MAGIC_KEY_DISTRIBUTION_ETC;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：イベントGパワー配布
 *
 * @param   g_power_id		GパワーID
 */
//==================================================================
void GAMEBEACON_Set_DistributionGPower(GPOWER_ID g_power_id)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_DISTRIBUTION_GPOWER) == FALSE){ return; };
  BEACONINFO_Set_DistributionGPower(&GameBeaconSys->send.info, g_power_id);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：イベントGパワー配布
 *
 * @param   g_power_id		GパワーID
 */
//==================================================================
void BEACONINFO_Set_DistributionGPower(GAMEBEACON_INFO *info, GPOWER_ID g_power_id)
{
  info->action.action_no = GAMEBEACON_ACTION_DISTRIBUTION_GPOWER;
  info->action.distribution.magic_key = MAGIC_KEY_DISTRIBUTION_GPOWER;
  info->action.distribution.gpower_id = g_power_id;

  BEACONINFO_Set_Details_Walk(info);
}
#endif  //PM_DEBUG

//==================================================================
/**
 * 送信ビーコンセット：戦闘：プレイヤーのポケモンが急所に攻撃をあてた
 *
 * @param   nickname		対象のポケモンのニックネーム(攻撃をしたポケモン)
 */
//==================================================================
void GAMEBEACON_Set_CriticalHit(const STRBUF *nickname)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_CRITICAL_HIT) == FALSE){ return; };
  BEACONINFO_Set_CriticalHit(&GameBeaconSys->send.info, nickname);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：戦闘：プレイヤーのポケモンが急所に攻撃をあてた
 *
 * @param   info		
 * @param   nickname		対象のポケモンのニックネーム(攻撃をしたポケモン)
 */
//==================================================================
void BEACONINFO_Set_CriticalHit(GAMEBEACON_INFO *info, const STRBUF *nickname)
{
  info->action.action_no = GAMEBEACON_ACTION_CRITICAL_HIT;
  GAMEBEACON_SUB_StrbufNicknameCopy(nickname, info->action.normal.nickname);

//  詳細は直前のを維持
}

//==================================================================
/**
 * 送信ビーコンセット：戦闘：プレイヤーのポケモンが急所に攻撃を受けた
 *
 * @param   nickname		対象のポケモンのニックネーム(攻撃を受けたポケモン)
 */
//==================================================================
void GAMEBEACON_Set_CriticalDamage(const STRBUF *nickname)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_CRITICAL_DAMAGE) == FALSE){ return; };
  BEACONINFO_Set_CriticalDamage(&GameBeaconSys->send.info, nickname);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：戦闘：プレイヤーのポケモンが急所に攻撃を受けた
 *
 * @param   info		
 * @param   nickname		対象のポケモンのニックネーム(攻撃を受けたポケモン)
 */
//==================================================================
void BEACONINFO_Set_CriticalDamage(GAMEBEACON_INFO *info, const STRBUF *nickname)
{
  info->action.action_no = GAMEBEACON_ACTION_CRITICAL_DAMAGE;
  GAMEBEACON_SUB_StrbufNicknameCopy(nickname, info->action.normal.nickname);

//  詳細は直前のを維持
}

//==================================================================
/**
 * 送信ビーコンセット：プレイヤーが道具を使用
 *
 * @param   item_no   アイテム番号
 */
//==================================================================
void GAMEBEACON_Set_UseItem(u16 item_no)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_USE_ITEM) == FALSE){ return; };
  BEACONINFO_Set_UseItem(&GameBeaconSys->send.info, item_no);
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * 送信ビーコンセット：プレイヤーが道具を使用
 *
 * @param   info		
 * @param   item_no   アイテム番号
 */
//==================================================================
void BEACONINFO_Set_UseItem(GAMEBEACON_INFO *info, u16 item_no)
{
  info->action.action_no = GAMEBEACON_ACTION_USE_ITEM;
  info->action.itemno = item_no;
  
  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信その他セット：ゾーン切り替え
 *
 * @param   zone_id   現在地
 */
//==================================================================
void GAMEBEACON_Set_ZoneChange(ZONEID zone_id, const GAMEDATA *cp_gamedata)
{
  BEACONINFO_Set_ZoneChange(&GameBeaconSys->send.info, zone_id, cp_gamedata);
  GameBeaconSys->send.beacon_update = TRUE;
}

//==================================================================
/**
 * その他セット：ゾーン切り替え
 *
 * @param   zone_id   現在地
 */
//==================================================================
void BEACONINFO_Set_ZoneChange(GAMEBEACON_INFO *info, ZONEID zone_id, const GAMEDATA *cp_gamedata)
{
  info->zone_id = zone_id;
  info->townmap_root_zone_id = TOWNMAP_UTIL_GetRootZoneID(cp_gamedata, zone_id);
  info->details.details_no = GAMEBEACON_DETAILS_NO_WALK;
  BEACONINFO_Set_Details_Walk(info);
}

//==============================================================================
//  詳細パラメータ
//==============================================================================
//--------------------------------------------------------------
/**
 * 詳細セット：野生のポケモンと対戦中
 * @param   monsno		ポケモン番号
 */
//--------------------------------------------------------------
void BEACONINFO_Set_Details_BattleWildPoke(GAMEBEACON_INFO *info, u16 monsno)
{
  info->details.details_no = GAMEBEACON_DETAILS_NO_BATTLE_WILD_POKE;
  info->details.battle_monsno = monsno;
}

//--------------------------------------------------------------
/**
 * 特別なポケモンと対戦中
 * @param   monsno		ポケモン番号
 */
//--------------------------------------------------------------
void BEACONINFO_Set_Details_BattleSpPoke(GAMEBEACON_INFO *info, u16 monsno)
{
  info->details.details_no = GAMEBEACON_DETAILS_NO_BATTLE_SP_POKE;
  info->details.battle_monsno = monsno;
}

//--------------------------------------------------------------
/**
 * トレーナーと対戦中
 * @param   tr_no		トレーナー番号
 */
//--------------------------------------------------------------
void BEACONINFO_Set_Details_BattleTrainer(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->details.details_no = GAMEBEACON_DETAILS_NO_BATTLE_TRAINER;
  info->details.battle_trainer = tr_no;
}

//--------------------------------------------------------------
/**
 * ジムリーダーと対戦中
 * @param   tr_no		トレーナー番号
 */
//--------------------------------------------------------------
void BEACONINFO_Set_Details_BattleLeader(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->details.details_no = GAMEBEACON_DETAILS_NO_BATTLE_JYMLEADER;
  info->details.battle_trainer = tr_no;
}

//--------------------------------------------------------------
/**
 * 特別なトレーナーと対戦中
 * @param   tr_no		トレーナー番号
 */
//--------------------------------------------------------------
void BEACONINFO_Set_Details_BattleSpTrainer(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->details.details_no = GAMEBEACON_DETAILS_NO_BATTLE_SP_TRAINER;
  info->details.battle_trainer = tr_no;
}

//--------------------------------------------------------------
/**
 * 「街、道路、ダンジョン」を移動中
 */
//--------------------------------------------------------------
void BEACONINFO_Set_Details_Walk(GAMEBEACON_INFO *info)
{
  info->details.details_no = GAMEBEACON_DETAILS_NO_WALK;
  //ZONE_IDはGAMEBEACON_Set_ZoneChangeを使用して別にセットしてもらう。
  //ここでは詳細Noを移動中に戻すことで、最後にセットしていた
  //GAMEBEACON_Set_ZoneChangeのゾーンIDが表示されるようにしている
  //ゾーンIDはゾーン切り替えした時にセットしてもらうイメージ。
  //※そのようにしなければ詳細Noが「移動中」のビーコンセット命令全てで
  //　ゾーンIDを渡す必要が出てくる
}

//==================================================================
/**
 * ビーコン詳細セット：自己紹介簡易会話
 *
 * @param   pms		簡易会話データへのポインタ
 */
//==================================================================
void BEACONINFO_Set_Details_IntroductionPms(GAMEBEACON_INFO *info, const PMS_DATA *pms)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  
  send->info.details.sentence_type = pms->sentence_type;
  send->info.details.sentence_id = pms->sentence_id;
  send->info.details.word[0] = pms->word[0];
  send->info.details.word[1] = pms->word[1];
}

//==================================================================
/**
 * 送信ビーコン詳細セット：自己紹介簡易会話
 *
 * @param   pms		簡易会話データへのポインタ
 */
//==================================================================
void GAMEBEACON_Set_Details_IntroductionPms(const PMS_DATA *pms)
{
  BEACONINFO_Set_Details_IntroductionPms(&GameBeaconSys->send.info, pms);
}


// 赤外線検出用
#ifdef AMPROTECT_FUNC
#include "system/irc_internal_another.h"
FS_EXTERN_OVERLAY(irc_check_another);
static void _Amprotect_TcbClearRecvBuff( GFL_TCB *tcb, void * work );
#endif  //AMPROTECT_FUNC

//==================================================================
/**
 * マジコン対策：受信バッファを常にクリアし続けるタスクを生成する
 */
//==================================================================
void GAMEBEACON_AMPROTECT_SetTask(void)
{
#ifdef AMPROTECT_FUNC
  BOOL fIRCExist=FALSE;
  
  // WILD・TRAINER戦の中に入ってくるということは一人用でプレイしている
  // =通信対戦・IR対戦ではないので無線チェックオーバーレイの使用OKなはず
  GFL_OVERLAY_Load( FS_OVERLAY_ID(irc_check_another));
  fIRCExist = IRC_Check_Another();
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(irc_check_another));
  if(fIRCExist == FALSE){
    //IRが無かったら受信バッファを消すタスクを生成する
    GFUser_HIntr_CreateTCB(_Amprotect_TcbClearRecvBuff, NULL, 3);
    MATSUDA_Printf("マジコン対策：受信バッファ消去タスク生成\n");
  }
#endif  //AMPROTECT_FUNC
}

//--------------------------------------------------------------
/**
 * マジコン対策：受信バッファを常にクリアし続ける
 *
 * @param   tcb		
 * @param   work		NULL
 */
//--------------------------------------------------------------
#ifdef AMPROTECT_FUNC
static void _Amprotect_TcbClearRecvBuff( GFL_TCB *tcb, void * work )
{
  if(GameBeaconSys != NULL){
    GameBeaconSys->update_log = 0;
    GameBeaconSys->send.beacon_update = 0;
    GameBeaconSys->update_log = 0;
  }
}
#endif  //AMPROTECT_FUNC


#ifdef PM_DEBUG

//==================================================================
/**
 * デバッグ用：強制で新しい人物とすれ違ったフラグを立てる
 */
//==================================================================
void DEBUG_GAMEBEACON_Set_NewEntry(void)
{
  GameBeaconSys->new_entry = TRUE;
}

//==================================================================
/**
 * デバッグ用：自分の送信ビーコン情報バッファのポインタを取得する
 * @retval  GAMEBEACON_INFO *		自分の送信ビーコン情報バッファへのポインタ
 */
//==================================================================
GAMEBEACON_INFO * DEBUG_SendBeaconInfo_GetPtr(void)
{
  GF_ASSERT(GameBeaconSys != NULL);
  return &GameBeaconSys->send.info;
}

//==================================================================
/**
 * デバッグ用：現在の受信ログバッファのコピーを取得
 */
//==================================================================
void DEBUG_GetBeaconRecvLogCopy( void* log_buf, s8* log_num, s8* start_log, s8* end_log )
{
  GAMEBEACON_SYSTEM *bsys = GameBeaconSys;
  GF_ASSERT( bsys != NULL);

  MI_CpuCopy32( bsys->log, log_buf, sizeof( GAMEBEACON_LOG )*GAMEBEACON_SYSTEM_LOG_MAX);
  *log_num = bsys->log_num;
  *start_log = bsys->start_log;
  *end_log = bsys->end_log;
}

//==================================================================
/**
 * デバッグ用：自分のビーコン送信キャンセル
 *
 * @note  直前にセットされたビーコン送信をキャンセルする
 */
//==================================================================
void DEBUG_SendBeaconCancel(void)
{
  GAMEBEACON_SEND_MANAGER* send = &GameBeaconSys->send;

  send->info.send_counter--;
  send->life = 0;
  send->beacon_update = FALSE;
}
//==================================================================
/**
 * デバッグ用：デバッグビーコンリクエスト時に、プライオリティを無視するためのフラグをセット
 */
//==================================================================
void DEBUG_SendBeaconPriorityEgnoreFlagSet( BOOL flag )
{
  GF_ASSERT(GameBeaconSys != NULL);
  GameBeaconSys->deb_beacon_priority_egnore = flag;
}

//==================================================================
/**
 * デバッグ用：受信ログを全件クリアする
 */
//==================================================================
void DEBUG_RecvBeaconBufferClear(void)
{
  GFL_STD_MemClear(GameBeaconSys->log, sizeof(GAMEBEACON_LOG) * GAMEBEACON_SYSTEM_LOG_MAX);
  GameBeaconSys->log_num = 0;
  GameBeaconSys->start_log = 0;
  GameBeaconSys->end_log = -1;
  GameBeaconSys->update_log = 0;
}

#endif  //PM_DEBUG

