//==============================================================================
/**
 * @file    game_beacon.c
 * @brief   ゲーム中のビーコン情報管理システム
 * @author  matsuda
 * @date    2009.12.13(日)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/game_beacon_types.h"
#include "gamesystem/fest_mission.h"
#include "gamesystem/fmission_log_local.h"
#include "gamesystem/link_festival_log.h"
#include "gamesystem/game_beacon_accessor.h"
#include "gamesystem/g_power.h"
#include "item/item.h"
#include "msg/msg_beacon_status.h"
#include "savedata/encount_sv.h"
#include "savedata/misc.h"
#include "savedata/medalbox.h"
#include "net/net_whpipe.h"
#include "savedata/my_pms_data.h"
#include "app/townmap_util.h"
#include "net_app\union\union_beacon_tool.h"
#include "app/research_radar/research_data.h"   //QUESTION_NUM_PER_TOPIC
#include "app/research_radar/question_id.h"
#include "savedata/questionnaire_index.h"
#include "savedata/playtime.h"
#include "field/research_team_def.h"
#include "system/gfl_use.h"
#include "field/zonedata.h"
#include "field/research_team.h"
#include "savedata/resonance_resort_sv.h"
#include "net/network_define.h"
#include "arc/fieldmap/zone_id.h"

#include "game_beacon_local.h"
#include "debug/debug_flg.h"

#include "debug/debug_nagihashi.h"

#ifdef PM_DEBUG
//#define DEBUG_MAIN_TIME_AVERAGE_MASTER_ON_BEACON
#endif

#ifdef DEBUG_JA_EXTEND_BUFF_CHECK
static BOOL s_ja_extend_check_flag    = FALSE;
static u8 s_ja_extend_check_buff[20]  = {0};
#endif

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

//リゾート用に拡張パックを変えていくためのフレーム最大
#ifdef PM_DEBUG
//numinputで使っている
u16 sc_extend_frame_max[RESORT_EXTEND_TYPE_MAX] =
#else
static const u16 sc_extend_frame_max[RESORT_EXTEND_TYPE_MAX] =
#endif
{
  30,
  30,
  30,
  30,
  30,
  30,
  30,
};

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
static BOOL SendBeacon_Update(GAMEBEACON_SEND_MANAGER *send);
static void BeaconInfo_Set(GAMEBEACON_SYSTEM *bsys, const GAMEBEACON_INFO *info);
static void BeaconInfo_BeaconTypeOverwrite( GAMEBEACON_INFO *info, GAMEBEACON_TYPE type );
static void BeaconInfo_BeaconDetailOverwrite( GAMEBEACON_INFO *info, BOOL recv_f );
static void BeaconInfo_SetFestMissionLog(GAMEBEACON_SYSTEM *bsys, const GAMEBEACON_INFO *info);
static GAMEBEACON_INFO * BeaconInfo_GetDirect(GAMEBEACON_SYSTEM *bsys, int log_no, GAMEBEACON_RECV_CLOCK* clock);
static GAMEBEACON_INFO * BeaconInfo_Get(GAMEBEACON_SYSTEM *bsys, int log_no);
static void SendBeacon_Init(GAMEBEACON_SEND_MANAGER *send, GAMEDATA * gamedata);
static BOOL BeaconInfo_RecvResortDataStart( GAMEBEACON_SYSTEM *bsys, const GAMEBEACON_INFO *info, RESORT_COMM_TYPE comm_type );
static BOOL BeaconInfo_RecvResortDataMain( GAMEBEACON_SYSTEM *bsys );
static void GAMEBEACON_Set_SendResortData( GAMEBEACON_SYSTEM *bsys );
static BOOL SendBeacon_Update_ResortData( GAMEBEACON_SYSTEM *bsys );
static void GAMEBEACON_Set_WildPokemonGet(u16 monsno);
static void GAMEBEACON_Set_SpecialPokemonGet(u16 monsno);
static BOOL GAMEBEACON_Set_FMissionWildPokemonGet( u16 monsno, BOOL eff_enc_f, BOOL mushroom_enc_f );

#ifdef PM_DEBUG
GAMEBEACON_INFO * DEBUG_SendBeaconInfo_GetPtr(void);
#endif


//==============================================================================
//  データ
//==============================================================================
#include "cross_comm.cdat"

//------------------------------------------------------------------
//  処理時間AVERAGE
//------------------------------------------------------------------
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON_BEACON

static u8 DEBUG_MAIN_UPDATE_TYPE = 0;  ///<FIELDMAP TOP TAIL フレームチェック用


static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Now;    // 現在のチック

static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Start;    // 1フレーム開始
static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_End;    // 1フレーム開始
static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave;      // AVERAGE
static int DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count = 0; // カウンタ
static int DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount =0;    // 16000Over
static OSTick DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max =0;    // 16000Over
#define DEBUG_DEBUG_MAIN_TIME_AVERAGE_AVE_COUNT_TIME  ( 120 )

#define DEBUG_DEBUG_MAIN_TIME_AVERAGE_NOW_TIME_DRAW_KEY  ( PAD_BUTTON_R )

static void DEBUG_MAIN_TIME_AVERAGE_StartFunc( void );
static void DEBUG_MAIN_TIME_AVERAGE_EndFunc( void );

#ifndef DEBUG_TIME_Printf
#define DEBUG_TIME_Printf(...) OS_TFPrintf(3,__VA_ARGS__)
#endif

#endif

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
  GF_ASSERT_HEAVY(GameBeaconSys == NULL);
  
  GameBeaconSys = GFL_HEAP_AllocClearMemory(heap_id, sizeof(GAMEBEACON_SYSTEM));
  GameBeaconSys->end_log = -1;
  GameBeaconSys->beacon_type = GAMEBEACON_TYPE_COMMON;      //初期タイプはBW互換

  GameBeaconSys->sbuf_person_name = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, heap_id );

  GameBeaconSys->p_resort_beacon_adrs  = RESORT_BEACON_DATA_Init( heap_id );
//      GFL_HEAP_LOWID( GFL_HEAPID_APP ) );

  GAMEBEACON_Set_ResortDataEnable( TRUE );
}

//==================================================================
/**
 * ゲームビーコン管理システムの破棄
 */
//==================================================================
void GAMEBEACON_Exit(void)
{
  GF_ASSERT_HEAVY(GameBeaconSys != NULL);
 
  RESORT_BEACON_DATA_Exit( GameBeaconSys->p_resort_beacon_adrs );
  GFL_STR_DeleteBuffer( GameBeaconSys->sbuf_person_name );
  GFL_HEAP_FreeMemory(GameBeaconSys);
  GameBeaconSys = NULL;
}

//==================================================================
/**
 * ゲームビーコン管理
 */
//==================================================================
void GAMEBEACON_SetGameSystemWork( GAMESYS_WORK* gsys )
{
  GAMEBEACON_SYSTEM *bsys;
  GF_ASSERT_HEAVY(GameBeaconSys != NULL);
  bsys = GameBeaconSys;

  bsys->gsys = gsys;
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
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON_BEACON
//  DEBUG_MAIN_TIME_AVERAGE_StartFunc();
#endif

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

  //もし寿命が切れたらNONEにし
  //リゾートビーコンセット予約
  if( SendBeacon_Update(&bsys->send) )
  {
    bsys->resort_set_request = TRUE;
  }

  if( WB_NET_FIELDMOVE_SERVICEID == GFL_NET_GetGameServiceID() 
        && !GFL_NET_WLIsChildStateScan() )
  {
    if( SendBeacon_Update_ResortData(bsys) )
    {
      //リゾートビーコン再セットはサーチかNULLじゃないとやらない
      if(bsys->send.info.action.action_no == GAMEBEACON_ACTION_SEARCH
      || bsys->send.info.action.action_no == GAMEBEACON_ACTION_NULL )
      {
        bsys->resort_set_request = TRUE;
      }
    }
  }

  if( bsys->gamedata != NULL
      && GAMEDATA_GetIsSave( bsys->gamedata ) == FALSE
      && bsys->resort_enable 
      && WB_NET_FIELDMOVE_SERVICEID == GFL_NET_GetGameServiceID() 
    )
  {
    if( !GFL_NET_WLIsChildStateScan() )
    {
      //サーチかNULLかつリクエスト状態ならば、
      //ビーコンセットして、GAMEBEACON_ACTION_SEARCHにする
      //
      //これにより、拡張とアクションNOのずれを回避している。
      if(bsys->send.info.action.action_no == GAMEBEACON_ACTION_SEARCH
      || bsys->send.info.action.action_no == GAMEBEACON_ACTION_NULL )
      {
        //送信処理
        //内部でGAMEBEACON_ACTION_NULLの場合GAMEBEACON_ACTION_SEARCHに書き換えている
        if( bsys->resort_set_request == TRUE )
        {
          GAMEBEACON_Set_SendResortData( bsys );
          bsys->resort_set_request = FALSE;
        }
      }
    }
    else
    {
      //受信処理
      //
      //送信していないときの受信処理。受信リクエストは別な場所でやっている。
      if( bsys->resort_recv_request == TRUE )
      {
        if( BeaconInfo_RecvResortDataMain( bsys ) )
        {
          bsys->resort_recv_request = FALSE;
        }
      }
    }
  }
  
  for(i = 0; i < GAMEBEACON_SYSTEM_LOG_MAX; i++){
    if(bsys->log[i].info.version_bit != 0){
      if(bsys->log[i].time < 0xffff){
        bsys->log[i].time++;
      }
    }
  }
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON_BEACON
//  DEBUG_MAIN_TIME_AVERAGE_EndFunc();
#endif
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
  GAMEBEACON_SYSTEM *bsys = GameBeaconSys;
  
  GFL_STD_MemCopy(&bsys->send, info, sizeof(GAMEBEACON_INFO));

  //BW互換のビーコンとswan専用ビーコンを切り替えるためにコピーしたデータを改変
  BeaconInfo_BeaconTypeOverwrite( info, bsys->beacon_type );
}

//==================================================================
/**
 * ゲームビーコン送信タイプを切り替える
 *
 * 呼び出されるたびに、GAMEBEACON_TYPE_SWAN_ONLYとGAMEBEACON_TYPE_COMMONを切り替える
 */
//==================================================================
void GAMEBEACON_BeaconTypeSwap(void)
{
  GAMEBEACON_SYSTEM *bsys = GameBeaconSys;
 
  if( bsys == NULL ){
    return; //スルー
  }
#ifdef PM_DEBUG
  //ビーコン送信モード固定デバッグ機能
  if( DEBUG_FLG_GetFlg(DEBUG_FLG_BeaconSendModeFix) ){
    if( DEBUG_FLG_GetFlg(DEBUG_FLG_BeaconSendModeBW) ){
      bsys->beacon_type = GAMEBEACON_TYPE_COMMON;
    }else{
      bsys->beacon_type = GAMEBEACON_TYPE_SWAN_ONLY;
    }
    return;
  }
#endif

  if( bsys->beacon_type == GAMEBEACON_TYPE_COMMON ){
    bsys->beacon_type = GAMEBEACON_TYPE_SWAN_ONLY;
  }else{
    bsys->beacon_type = GAMEBEACON_TYPE_COMMON;
  }
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
#ifdef DEBUG_JA_EXTEND_BUFF_CHECK
      DEBUG_GAMEBEACON_CheckJAExtendCheck();
#endif
      NET_WHPIPE_BeaconSetInfo();
      GameBeaconSys->send.beacon_update = FALSE;
    }
  }
}

//==================================================================
/**
 * Cギアにビーコンの状態を通知するフラグをセットする
 */
//==================================================================
void GAMEBEACON_SetCGearInfoFlag( GAMEBEACON_CGEAR_INFO_FLAG flag )
{
  GAMEBEACON_SYSTEM *bsys = GameBeaconSys;
 
  if( bsys == NULL ){
    return; //スルー
  }
  bsys->cgear_info_flags |= (1<<flag);
}

//==================================================================
/**
 * Cギアにビーコンの状態を通知するフラグをリセットする
 */
//==================================================================
void GAMEBEACON_ResetCGearInfoFlag( GAMEBEACON_CGEAR_INFO_FLAG flag )
{
  GAMEBEACON_SYSTEM *bsys = GameBeaconSys;
 
  if( bsys == NULL ){
    return; //スルー
  }
  bsys->cgear_info_flags &= ( 0xff ^ (1<<flag) );
}

//==================================================================
/**
 * Cギアにビーコンの状態を通知するフラグをゲットする
 */
//==================================================================
BOOL GAMEBEACON_GetCGearInfoFlag( GAMEBEACON_CGEAR_INFO_FLAG flag )
{
  GAMEBEACON_SYSTEM *bsys = GameBeaconSys;
 
  if( bsys == NULL ){
    return FALSE; //スルー
  }
  return ( bsys->cgear_info_flags >> flag ) & 0x01;
}

//--------------------------------------------------------------
/**
 * 送信ビーコン更新処理
 *
 * @param   send		
 */
//--------------------------------------------------------------
static BOOL SendBeacon_Update(GAMEBEACON_SEND_MANAGER *send)
{
  if(send->info.action.action_no != GAMEBEACON_ACTION_SEARCH){
    send->life++;
    if(send->life > CrossCommData[send->info.action.action_no].life){
      send->life = 0;
      send->info.action.action_no = GAMEBEACON_ACTION_NULL;
      send->info.version_bit = GAMEBEACON_RECV_VERSION_FULL; //全バージョン指定に戻す
      send->info.g_power_id = GPOWER_ID_NULL; //発動中のデルパワーIDをNULL指定に戻す
      return TRUE;
    }
  }

  return FALSE;
}

//--------------------------------------------------------------
/**
 * ログからアンケート情報を収集する
 *
 * @param   bsys		
 * @param   info		ビーコン情報
 */
//--------------------------------------------------------------
static BOOL BeaconInfo_SetQuestionnaire(GAMEBEACON_SYSTEM *bsys, const GAMEBEACON_INFO *info)
{
  //アンケート反映
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(bsys->gamedata);
  QUESTIONNAIRE_SAVE_WORK *questsave = SaveData_GetQuestionnaire(sv_ctrl);
  MISC *misc = SaveData_GetMisc(sv_ctrl);
  int search_question_id, team_q_id, i;
  u32 answer, lower_version = FALSE;
    
  if( info->pm_version == VERSION_WHITE || info->pm_version == VERSION_BLACK ){
    lower_version = TRUE; //ホワイトorブラック
  }else{
    //ホワイト2 or ブラック2
    if( info->version_bit & GAMEBEACON_RECV_VERSION_SYACHI ){
      return FALSE; //SWANからBW互換ビーコンが来たらアンケート結果受け取りをupdate時まで遅延させる
    }
  }
    
  for(i = 0; i < QUESTION_NUM_PER_TOPIC; i++){
    int j;

    search_question_id = QuestionnaireWork_GetInvestigatingQuestion(questsave, i);
    if(search_question_id == INVESTIGATING_QUESTION_NULL){
      continue;
    }
    //送り主がBWの時、スワンで変わった質問の結果は受け取らない
    if( lower_version && QUESTIONNAIRE_INDEX_IsSwanNewQuestion( search_question_id ) ){
      continue;
    }
    if(search_question_id == QUESTION_ID_PLAY_TIME){
      u8 answer_num = QIDATA_Get_AnswerNum(search_question_id);
      answer = info->play_hour / 10;
      answer++; //0は無回答なので1インクリメント
      if(answer > answer_num){
        answer = answer_num;
      }
    }
    else{
      answer = QuestionnaireAnswer_ReadBit(&info->question_answer, search_question_id);
    }
    if(answer == 0){  //0は無回答
      continue;
    }
    //アンケート反映
    if(QuestionnaireWork_IsTodayAnswerNumFull(questsave, search_question_id, answer) == FALSE){
      QuestionnaireWork_AddTodayCount(questsave, search_question_id, 1);
      QuestionnaireWork_AddTodayAnswerNum(questsave, search_question_id, answer, 1);
      bsys->new_entry = TRUE;
      IWASAWA_Printf("QuestionAnswerSet id=%d, answer=%d\n", search_question_id, answer);
    }
    //調査隊依頼成果反映
    for(j = 0; j < MAX_QNUM_PER_RESEARCH_REQ; j++) {
      team_q_id = MISC_GetResearchQuestionID(misc, j);
      if(team_q_id == QUESTION_ID_DUMMY){ continue; }

      //受けている依頼と、調査中の質問が同じならカウントアップ
      if( team_q_id == search_question_id ){
        MISC_AddResearchCount(misc, j, 1);
      }
    }
  } //end of for()
  return TRUE;
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
#if 0
  {
    SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(bsys->gamedata);
    QUESTIONNAIRE_SAVE_WORK *questsave = SaveData_GetQuestionnaire(sv_ctrl);
    MISC *misc = SaveData_GetMisc(sv_ctrl);
    int search_question_id, team_q_id, i;
    u32 answer, lower_version = FALSE;
    
    if( info->pm_version == VERSION_WHITE || info->pm_version == VERSION_BLACK ){
      lower_version = TRUE; //ホワイトorブラック
    }
    
    for(i = 0; i < QUESTION_NUM_PER_TOPIC; i++){
      int j;

      search_question_id = QuestionnaireWork_GetInvestigatingQuestion(questsave, i);
      if(search_question_id == INVESTIGATING_QUESTION_NULL){
        continue;
      }
      //送り主がBWの時、スワンで変わった質問の結果は受け取らない
      if( lower_version && QUESTIONNAIRE_INDEX_IsSwanNewQuestion( search_question_id ) ){
        continue;
      }
      if(search_question_id == QUESTION_ID_PLAY_TIME){
        u8 answer_num = QIDATA_Get_AnswerNum(search_question_id);
        answer = info->play_hour / 10;
        answer++; //0は無回答なので1インクリメント
        if(answer > answer_num){
          answer = answer_num;
        }
      }
      else{
        answer = QuestionnaireAnswer_ReadBit(&info->question_answer, search_question_id);
      }
        
      if(answer == 0){  //0は無回答
        continue;
      }
      //アンケート反映
      if(QuestionnaireWork_IsTodayAnswerNumFull(questsave, search_question_id, answer) == FALSE){
        QuestionnaireWork_AddTodayCount(questsave, search_question_id, 1);
        QuestionnaireWork_AddTodayAnswerNum(questsave, search_question_id, answer, 1);
        bsys->new_entry = TRUE;
        MATSUDA_Printf("QuestionAnswerSet id=%d, answer=%d\n", search_question_id, answer);
      }
      //調査隊依頼成果反映
      for(j = 0; j < MAX_QNUM_PER_RESEARCH_REQ; j++) {
        team_q_id = MISC_GetResearchQuestionID(misc, j);
        if(team_q_id == QUESTION_ID_DUMMY){ continue; }

        //受けている依頼と、調査中の質問が同じならカウントアップ
        if( team_q_id == search_question_id ){
          MISC_AddResearchCount(misc, j, 1);
        }
      }
    } //end of for()
  } //end of アンケート反映
#endif

  //ログにセット
  {
    RTCTime recv_time;
    GFL_RTC_GetTime(&recv_time);
    GFL_STD_MemClear(setlog, sizeof(GAMEBEACON_LOG));
  
    setlog->info = *info;
    setlog->time = 0;
    setlog->clock.hour = recv_time.hour;
    setlog->clock.min = recv_time.minute;
    setlog->clock.research_f = BeaconInfo_SetQuestionnaire( bsys, info );
    
    //BTS1833対策 BWから送られてきた詳細情報Noがトレーナー戦関連だったら移動中に上書き
    BeaconInfo_BeaconDetailOverwrite( &(setlog->info), TRUE );
    
    bsys->update_log |= 1 << bsys->end_log;
    if(bsys->log_num < GAMEBEACON_SYSTEM_LOG_MAX){
      bsys->log_num++;
    }
  }
}

//--------------------------------------------------------------
/**
 * ビーコンの中身を見て、BWが受け取って意味のあるビーコンかチェックする
 *
 * @param   info		
 * @param   type  GAMEBEACON_TYPE(GAMEBEACON_TYPE_COMMONならBW互換、TYPE_SWAN_ONLYならSwan専用)
 *
 * ゾーンIDやアイテムNoがBWが知らないNoだった場合に、スワン専用で送る
 */
//--------------------------------------------------------------
static BOOL BeaconInfo_BeaconSwanOnlyCheck( GAMEBEACON_INFO *info, GAMEBEACON_TYPE type )
{
  //BW互換のビーコンとswan専用ビーコンを切り替えるためにデータを改変
  if( type == GAMEBEACON_TYPE_SWAN_ONLY || (info->version_bit==GAMEBEACON_RECV_VERSION_SWAN)){
    return TRUE;
  }

  //BWから存在するゾーンか？
  if( info->zone_id >= ZONE_ID_C12 || info->townmap_root_zone_id >= ZONE_ID_C12 ){
    return TRUE;  //C12以降はスワンで新規追加された
  }
  //BWが知っているデルパワーか？
  if( info->g_power_id != GPOWER_ID_NULL &&
      GPOWER_IsEnableID_BW( info->g_power_id ) == FALSE ){
    return TRUE;  //BWでは発動できないパワーだった
  }
  return FALSE; //BWのすれ違いログが受け取れる
}

//--------------------------------------------------------------
/**
 * 送信ビーコンのタイプを切り替えるために、ビーコン情報を上書きする
 *
 * @param   info		
 * @param   type  GAMEBEACON_TYPE(GAMEBEACON_TYPE_COMMONならBW互換、TYPE_SWAN_ONLYならSwan専用)
 *
 * ビーコンデータはSwanをメインターゲットに生成しておき、
 * BW互換にしたい時、BWが受け取れないパラメータをクリアする
 */
//--------------------------------------------------------------
static void BeaconInfo_BeaconTypeOverwrite( GAMEBEACON_INFO *info, GAMEBEACON_TYPE type )
{
  int i;

  //BW互換のビーコンとswan専用ビーコンを切り替えるためにデータを改変
  if( BeaconInfo_BeaconSwanOnlyCheck( info, type ) ){
    //スワン専用ビーコンなので、BW受信可フラグを強制Off
    info->version_bit = GAMEBEACON_RECV_VERSION_SWAN;
  }else{
    /*
     BWとの互換のためにデータを上書き(11.06.06現在、アンケートで互換性のない回答を無回答にしている)
    */

    //アンケートのバッファを上書き
    for( i = 0;i <= QUESTION_ID_MAX;i++ ){
      if( QUESTIONNAIRE_INDEX_IsSwanNewQuestion( i ) ){
        //スワンで変更があった質問の内容は無回答=0にする
        QuestionnaireAnswer_WriteBit( &info->question_answer, i, 0);
      }
    }
    //BTS1833対策 詳細情報Noがトレーナー戦関連だったら移動中に上書き
    BeaconInfo_BeaconDetailOverwrite( info, FALSE );
  }
}

//--------------------------------------------------------------
/**
 * Swan⇔BW間でビーコンを受信した際に、受け取ってはいけない詳細情報パラメータを上書きする
 *
 * @param   info		
 * 
 * BTS1833対策
 */
//--------------------------------------------------------------
static void BeaconInfo_BeaconDetailOverwrite( GAMEBEACON_INFO *info, BOOL recv_f )
{
  //受信モード時はBWから送られてきたビーコンかチェック。Swanからなら何もしない
  if( recv_f ){
    if( info->pm_version == VERSION_W2 || info->pm_version == VERSION_B2 ){
      return;
    }  
  }
  //詳細情報Noがトレーナー戦関連だったら移動中に上書き
  if( info->details.details_no >= GAMEBEACON_DETAILS_NO_BATTLE_TRAINER &&
      info->details.details_no <= GAMEBEACON_DETAILS_NO_BATTLE_SP_TRAINER ){
#if 0
    OS_TFPrintf(2,"DetailOverwrite act=%d detail=%d ver=%d, ver_bit=%08x\n",
      info->action.action_no, info->details.details_no, info->pm_version, info->version_bit );
#endif
    BEACONINFO_Set_Details_Walk( info );
    info->details.battle_trainer = 0;
  }
}

//--------------------------------------------------------------
/**
 * 受信ビーコンからフェスタミッションログを取得する
 *
 * @param   bsys		
 * @param   GAMEBEACON_INFO *
 */
//--------------------------------------------------------------
static void BeaconInfo_SetFestMissionLog(GAMEBEACON_SYSTEM *bsys, const GAMEBEACON_INFO *info)
{
  LINK_FESTIVAL*  fest_sys;
  LINKFEST_LOG_PARAM log;
  LINKFEST_SCORELOG_SET_CODE ret = LINKFEST_SCORELOG_SET_FAILED;
  BOOL is_entry;
  int log_score;

  if( bsys->gsys == NULL ){
    GF_ASSERT(0);
    return;
  }
  fest_sys = GAMESYSTEM_GetLinkFestivalWork( bsys->gsys );

  is_entry = LINK_FESTIVAL_IsMissionEntry( fest_sys );
  if( is_entry ){
    //Gパワーストック
    FMISSION_GPOWER_TBL_Set( 
      LINK_FESTIVAL_GetMissionLogGPowerTbl(fest_sys), info );
  }

  if( GAMEBEACON_Check_FestMission( info ) == FALSE ){
    return;
  }
  //エラーチェック
  if( FMISSION_BeaconErrorCheck( fest_sys, info ) ){
    return;
  }
  //エントリーデータストック
  FMISSION_ENTRY_TBL_Set( fest_sys, info );
  
  if( !is_entry || info->action.action_no == GAMEBEACON_ACTION_DISTRIBUTION_FMISSION ){
    //自分がエントリーしていない時、アクションが配布フェスミッションだった場合はログに積まない
    return;
  }
  //ログストアパラメータ生成
  LINKFEST_LOG_PARAM_Clear( &log );
  log.prm.score.tr_id = info->trainer_id;
  log.prm.score.tmap_root = info->townmap_root_zone_id;
  log.prm.score.value = info->action.fmission.score;

  FMISSION_GetBeaconParam( &log.m_data, info );
  
  ret = LINK_FESTIVAL_ScoreLogStore( fest_sys, &log, &log_score );

  if( ret != LINKFEST_SCORELOG_SET_FAILED ){
    //ミッションログ収集
    if( ret < LINKFEST_SCORELOG_SET_UPDATE ){
      if( ret == LINKFEST_SCORELOG_SET_NEW_ENTRY ){
        FMISSION_LOG_TBL_SetLogFromBeacon( fest_sys, info, log_score, FMISSION_LOG_SET_NEW, FALSE );
      }
      FMISSION_LOG_TBL_SetLogFromBeacon( fest_sys, info, log_score, FMISSION_LOG_SET_MAP, FALSE );
    }
    if( ret == LINKFEST_SCORELOG_SET_MAP_CHANGE_SUP ||
        ret == LINKFEST_SCORELOG_SET_UPDATE_SUP ){
      FMISSION_LOG_TBL_SetLogFromBeacon( fest_sys, info, log_score, FMISSION_LOG_SET_SCORE_UP, FALSE );
    }else{
      FMISSION_LOG_TBL_SetLogFromBeacon( fest_sys, info, log_score, FMISSION_LOG_SET_NORMAL, FALSE );
    }
    
    //トップスコア更新
    GFL_STR_SetStringCodeOrderLength( bsys->sbuf_person_name, info->name, PERSON_NAME_SIZE+EOM_SIZE );
    LINK_FESTIVAL_TopScoreUpdate( fest_sys, info->trainer_id, log_score, bsys->sbuf_person_name );
  }
}

//--------------------------------------------------------------
/**
 * 受信ビーコンをチェックして、Cギアに通知する情報があれば通知フラグをたてる
 *
 * @param   bsys		
 * @param   GAMEBEACON_INFO *
 */
//--------------------------------------------------------------
static void BeaconInfo_CheckCGearInfo(GAMEBEACON_SYSTEM *bsys, const GAMEBEACON_INFO *info, BOOL new_f )
{
  if( bsys->gsys == NULL ){
    GF_ASSERT(0);
    return;
  }

  if( RESEARCH_TEAM_CheckCurrentResearchFinishForCGear( bsys->gamedata ) ){
    //調査依頼を達成しているので通知する
    GAMEBEACON_SetCGearInfoFlag( GAMEBEACON_CGEAR_INFO_RADAR );
  }

  if( new_f ){
    //新着が来た
    GAMEBEACON_SetCGearInfoFlag( GAMEBEACON_CGEAR_INFO_HOT );
    return;
  }
  //デルパワービーコンが来たかどうか？(FMISSION_GPOWERは考えない)
  switch( info->action.action_no ){
  case GAMEBEACON_ACTION_GPOWER :
  case GAMEBEACON_ACTION_DISTRIBUTION_GPOWER:
    GAMEBEACON_SetCGearInfoFlag( GAMEBEACON_CGEAR_INFO_HOT );
    break;
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
  int i = 0;
  BOOL same_player = FALSE;
  BOOL same_data   = FALSE;
  BOOL is_search   = FALSE;

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
  //ビーコンデータの整合性チェック
  if( GAMEBEACON_Check_Error( info ) ){
    return FALSE;
  }
  {
    int ct;
    for( ct = 0, i = bsys->start_log; ct < bsys->log_num; ct++ ){
      if(bsys->log[i].info.trainer_id == info->trainer_id){
        if( bsys->log[i].info.send_counter == info->send_counter){
          same_data = TRUE; //ログに同じデータを受信済み
          break;
        }
        if(info->action.action_no == GAMEBEACON_ACTION_SEARCH ){
          is_search = TRUE; //同じプレイヤーから届いたサーチビーコン
          break;
        }
        same_player = TRUE; //1プレイヤーは対になった1つのログバッファしか持たない
        break;
      }
      if( ++i >= GAMEBEACON_SYSTEM_LOG_MAX ){
        i = 0;  //回り込み
      }
    }
  }
#if 0  
  if(bsys->start_log <= bsys->end_log){
    for(i = bsys->start_log; i <= bsys->end_log; i++){
      if(bsys->log[i].info.trainer_id == info->trainer_id){
        if( bsys->log[i].info.send_counter == info->send_counter){
          same_data = TRUE; //ログに同じデータを受信済み
          break;
        }
        if(info->action.action_no == GAMEBEACON_ACTION_SEARCH ){
          is_search = TRUE; //同じプレイヤーから届いたサーチビーコン
          break;
        }
        same_player = TRUE; //1プレイヤーは対になった1つのログバッファしか持たない
        break;
      }
    }
  }
  else{
    for(i = bsys->start_log; i < GAMEBEACON_SYSTEM_LOG_MAX; i++){
      if(bsys->log[i].info.trainer_id == info->trainer_id){
        if( bsys->log[i].info.send_counter == info->send_counter){
          same_data = TRUE; //ログに同じデータを受信済み
          break;
        }
        if(info->action.action_no == GAMEBEACON_ACTION_SEARCH ){
          is_search = TRUE; //同じプレイヤーから届いたサーチビーコン
          break;
        }
        same_player = TRUE; //1プレイヤーは対になった1つのログバッファしか持たない
        break;
      }
    }
    for(i = 0; i <= bsys->end_log; i++){
      if(bsys->log[i].info.trainer_id == info->trainer_id){
        if( bsys->log[i].info.send_counter == info->send_counter){
          same_data = TRUE; //ログに同じデータを受信済み
          break;
        }
        if(info->action.action_no == GAMEBEACON_ACTION_SEARCH ){
          is_search = TRUE; //同じプレイヤーから届いたサーチビーコン
          break;
        }
        same_player = TRUE; //1プレイヤーは対になった1つのログバッファしか持たない
        break;
      }
    }
  }
#endif

  if( same_data ){
    //前回受信したのと同じデータなので更新しない
    return FALSE;
  }
#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON_BEACON
  DEBUG_MAIN_TIME_AVERAGE_StartFunc();
#endif
  //リゾートビーコン受け取り
  {
    if( info->action.action_no == GAMEBEACON_ACTION_SEARCH )
    {
//DEBUG_TICK_PrintStart;
      if( !bsys->resort_recv_request )
      {
        if( BeaconInfo_RecvResortDataStart( bsys, info, RESORT_COMM_TYPE_BEACON ) )
        {
          bsys->resort_recv_request = TRUE;
        }
      }
//DEBUG_TICK_PrintEnd;
    }
    else if( GAMEBEACON_Check_FestMission(info) )
    {
      //フェスビーコンかつ、配信ミッションではないとき
      if( info->action.action_no != GAMEBEACON_ACTION_DISTRIBUTION_FMISSION )
      {
        if( !bsys->resort_recv_request )
        {
          if( BeaconInfo_RecvResortDataStart( bsys, info, RESORT_COMM_TYPE_HYLINKFES ) )
          {
            bsys->resort_recv_request = TRUE;
          }
        }
      }
    }
  } //リゾートビーコン受け取り終了

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON_BEACON
  DEBUG_MAIN_TIME_AVERAGE_EndFunc();
#endif

  //同じプレイヤーのサーチビーコンだったらアンケート回収だけ試みる
  if( is_search )
  {
    if( bsys->log[i].clock.research_f == FALSE ){
      //アンケートがまだ未収集
      bsys->log[i].clock.research_f = BeaconInfo_SetQuestionnaire( bsys, info );
    }
    return FALSE;
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

  BeaconInfo_SetFestMissionLog( bsys, info );
  BeaconInfo_CheckCGearInfo( bsys, info, !same_player );

#if 0
  OS_TFPrintf(2,"BeaconRecv act=%d detail=%d ver = %d, ver_bit=%08x\n",
      info->action.action_no, info->details.details_no, info->pm_version, info->version_bit );
#endif

  //受信データをログバッファにセット
  if(same_player == TRUE){
    RTCTime recv_time;
    GFL_RTC_GetTime(&recv_time);

    bsys->log[i].info = *info;
    bsys->log[i].time = 0;  //タイムカウントリセット
    bsys->log[i].clock.hour = recv_time.hour;
    bsys->log[i].clock.min = recv_time.minute;
    
    //BTS1833対策 BWから送られてきた詳細情報Noがトレーナー戦関連だったら移動中に上書き
    BeaconInfo_BeaconDetailOverwrite( &bsys->log[i].info, TRUE );

    if( bsys->log[i].clock.research_f == FALSE ){
      //アンケートがまだ未収集
      bsys->log[i].clock.research_f = BeaconInfo_SetQuestionnaire( bsys, info );
    }
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
static GAMEBEACON_INFO * BeaconInfo_GetDirect(GAMEBEACON_SYSTEM *bsys, int log_no, GAMEBEACON_RECV_CLOCK* clock )
{
  if(bsys->log_num == 0 || bsys->log_num - log_no <= 0){
    clock->hour = 0;
    clock->min = 0;
    return NULL;  //データ無し
  }
 
  *clock = bsys->log[log_no].clock;
  return &bsys->log[log_no].info;
}

//==================================================================
/**
 * ログからビーコン情報を取得する
 *
 * @param   log_no  ログのバッファ番号
 * @param   clock   GAMEBEACON_RECV_CLOCK* 受信日時を取得するワークポインタ
 * @retval  GAMEBEACON_INFO *		NULLの場合はデータ無し
 */
//==================================================================
const GAMEBEACON_INFO * GAMEBEACON_Get_BeaconLogDirect(int log_no, GAMEBEACON_RECV_CLOCK* clock )
{
  return BeaconInfo_GetDirect(GameBeaconSys, log_no, clock );
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
  
  info->version_bit = GAMEBEACON_RECV_VERSION_FULL; //全バージョン指定
  info->zone_id = PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork(gamedata));
  if(ZONEDATA_IsUnionRoom(info->zone_id) == TRUE || ZONEDATA_IsColosseum(info->zone_id) == TRUE){
    const LOCATION *loc = GAMEDATA_GetSpecialLocation(gamedata);
    info->zone_id = loc->zone_id;
  }
  info->townmap_root_zone_id = TOWNMAP_UTIL_GetRootZoneID(gamedata, info->zone_id);
  //「～を移動中です」はrootではなく下記zone_idを使っている。
  //BTS1875の対処のため、WFBCを受信側で識別できるようにする。
  if( ZONEDATA_IsWfbc( info->zone_id ) )
  {
    info->zone_id = info->townmap_root_zone_id;
  }
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
    MEDALBOX* sv_mdlbox = SaveData_GetMedalBox( sv );
    info->push_medal = MEDALBOX_GetPushMedalID( sv_mdlbox );
    info->medal_count = MEDALBOX_GetMedalCount( sv_mdlbox , FALSE );
  }
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
//----------------------------------------------------------------------------
/**
 *	@brief  リゾート用データを受け取る  開始処理
 *
 *	@param	GAMEBEACON_SYSTEM *bsys ビーコンシステム
 *	@param	GAMEBEACON_INFO *info   ビーコン情報
 *	@return TRUE開始した  FALSE開始しなかった
 */
//-----------------------------------------------------------------------------
static BOOL BeaconInfo_RecvResortDataStart( GAMEBEACON_SYSTEM *bsys, const GAMEBEACON_INFO *info, RESORT_COMM_TYPE comm_type )
{
  if( GAMEDATA_GetIsSave( bsys->gamedata ) == FALSE )
  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( bsys->gamedata );
    RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( p_sv );

    //ビーコン情報取得
    RESORT_BEACON_DATA_SetBeaconInfo( GameBeaconSys->p_resort_beacon_adrs, info, comm_type );
    RESONANCE_RESORT_SV_SetDataStart( p_resort );
    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  リゾート用データを受け取る  メイン処理
 *
 *	@param	GAMEBEACON_SYSTEM *bsys   ビーコンシステム
 *
 *	@return TRUE完了  FALSE処理中
 */
//-----------------------------------------------------------------------------
static BOOL BeaconInfo_RecvResortDataMain( GAMEBEACON_SYSTEM *bsys )
{
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( bsys->gamedata );
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( p_sv );
  RESORT_SETDATA ret;
  return RESONANCE_RESORT_SV_SetDataMain( p_resort, bsys->gamedata, GameBeaconSys->p_resort_beacon_adrs, RESORT_MERGE_TYPE_BEACON, &ret );
}

//--------------------------------------------------------------
/**
 * 送信ビーコンをセットした時の共通処理
 *
 * @param   send		
 * @param   info		
 */
//--------------------------------------------------------------
static void sub_SendBeaconSetCommonCore(GAMEBEACON_SEND_MANAGER *send, u32 version_bit )
{
  SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(GameBeaconSys->gamedata);
  PLAYTIME *sv_playtime = GAMEDATA_GetPlayTimeWork( GameBeaconSys->gamedata );
  const MISC *misc = SaveData_GetMisc( sv_ctrl );
  
  send->info.play_hour = PLAYTIME_GetHour( sv_playtime );
  send->info.play_min = PLAYTIME_GetMinute( sv_playtime );

  send->info.thanks_recv_count = MISC_CrossComm_GetThanksRecvCount(misc);
  send->info.suretigai_count = MISC_CrossComm_GetSuretigaiCount(misc);
  
  send->info.version_bit = version_bit;
  send->info.ex_param = 0;  //まずは0初期化。有効なパラメータを入れたい場合は各ビーコンのリクエスト側で設定すること
  send->info.g_power_id = GPOWER_ID_NULL; //発動中のデルパワーID領域をNULL指定に戻す

  send->info.send_counter++;
  send->life = 0;
  send->beacon_update = TRUE;
}

//--------------------------------------------------------------
/**
 * 送信ビーコンをセットした時の共通処理(基本全バージョン受け取り可)
 *
 * @param   send		
 * @param   info		
 */
//--------------------------------------------------------------
void GAMEBEACON_SUB_SendBeaconSetCommon(GAMEBEACON_SEND_MANAGER *send)
{
  sub_SendBeaconSetCommonCore( send, GAMEBEACON_RECV_VERSION_FULL );  //全バージョン受け取り可
}
//--------------------------------------------------------------
/**
 * 送信ビーコンをセットした時の共通処理(SWAN以降のみ受け取り可)
 *
 *以以降降 @param   send		
 * @param   info		
 */
//--------------------------------------------------------------
void GAMEBEACON_SUB_SendBeaconSetCommonSwan(GAMEBEACON_SEND_MANAGER *send)
{
  sub_SendBeaconSetCommonCore( send, GAMEBEACON_RECV_VERSION_SWAN );  //SWAN以降のみ受け取り可
}

//--------------------------------------------------------------
/**
 * @brief フェスミッションビーコン送信用　ビーコンデータ生成共通処理
 *
 * INFOパラメータの生成→プレイヤーログ生成→プライオリティチェック→ビーコン送信リクエスト
 * までを一気に行う
 *
 * @param   send		
 * @param   action
 * @param   param1
 * @param   param2
 */
//--------------------------------------------------------------
void GAMEBEACON_SUB_SendBeaconSetCommonForFMission(GAMEBEACON_SEND_MANAGER *send, u16 action, u32 param )
{
  GAMEBEACON_SUB_SendBeaconSetCommonForFMissionEx(send, action, param, 0 );
}

void GAMEBEACON_SUB_SendBeaconSetCommonForFMissionEx(GAMEBEACON_SEND_MANAGER *send, u16 action, u32 param1, u32 param2 )
{
  GAMEBEACON_SEND_MANAGER tmp;
  LINK_FESTIVAL* fest_sys;
  const FEST_MISSION* fmwk;
  GAMEBEACON_INFO* info = &tmp.info;

  if( GameBeaconSys->gsys  == NULL ) {
    return;
  }
  fest_sys = GAMESYSTEM_GetLinkFestivalWork( GameBeaconSys->gsys );
  fmwk = LINK_FESTIVAL_GetMissionWork( fest_sys );

  GF_ASSERT( fmwk->beacon.m_id != FMISSION_ID_NULL );

  tmp = *send;  //現時点での内容をコピー

  //パラメータ生成
  GAMEBEACON_SUB_SendBeaconSetCommonSwan( &tmp );
  
  LINK_FESTIVAL_GetBeaconSendParam( fest_sys, &info->action.fest_beacon, (u16)param1 );
  switch( action ){
  case GAMEBEACON_ACTION_FMISSION_FREEWORD:
    //フリーワードは一部領域を上書き
    GAMEBEACON_SUB_GetStringCodeForAction( (const STRBUF*)param1, info->action.fest_beacon.fword, FMISSION_FWORD_MESSAGE_LEN );
    info->action.fmission.fword_sid = param2; ///<NAMEIN_SHORTCUT_RET ショートカットID
    break;
  default:
    info->action.fmission.fword_sid = 0;
    break;
  }
  info->action.action_no = action;  //アクションno上書き
  info->action.fmission.score = fmwk->my_score;
  info->fmission_key = fmwk->key; //マジックキー登録

  BEACONINFO_Set_Details_Walk( info );

  //生成したパラメータで、自分用のミッションログを生成
  if( action != GAMEBEACON_ACTION_FMISSION ){
    FMISSION_LOG_TBL_SetLogFromBeacon( fest_sys, &tmp.info, LINK_FESTIVAL_GetMyScore( fest_sys ), FMISSION_LOG_SET_NORMAL, TRUE );
  }
  
  if(GAMEBEACON_SUB_CheckPriority( action ) == TRUE){
    //実際の送信バッファに生成したパラメータを反映して、送信リクエストをかける
    *send = tmp;
  }
}

//--------------------------------------------------------------
/**
 * フェスミッションビーコンかどうかチェック 
 * @param   info	ビーコン情報へのポインタ
 *
 * @retval  TRUE  フェスミッションビーコン
 * @retval  FALSE フェスミッションビーコンではない
 */
//--------------------------------------------------------------
BOOL GAMEBEACON_Check_FestMission(const GAMEBEACON_INFO *info)
{
  if( info->action.action_no < GAMEBEACON_ACTION_FEST_BEACON_START ||
      info->action.action_no > GAMEBEACON_ACTION_FEST_BEACON_END ){
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
/**
 * ミッションパラメータを持つフェスミッションビーコンかどうかチェック 
 * @param   info	ビーコン情報へのポインタ
 *
 * @retval  TRUE  パラメータ持ちビーコン
 * @retval  FALSE パラメータ持ちビーコンではない
 */
//--------------------------------------------------------------
BOOL GAMEBEACON_Check_FestMissionWithParam(const GAMEBEACON_INFO *info)
{
  if( info->action.action_no < GAMEBEACON_ACTION_FEST_BEACON_WP_START ||
      info->action.action_no > GAMEBEACON_ACTION_FEST_BEACON_WP_END ){
    return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief フェスミッションのエントリーチェックとスコア計算を行う
 *
 * @param   action  LINK_FESTIVAL_MissionScoreCalc()に引き渡す actionナンバー
 * @param   param LINK_FESTIVAL_MissionScoreCalc()に引き渡すパラメータ
 *
 * @retval  FM_CHECK_SCORE_RET型
 */
//--------------------------------------------------------------
FM_CHECK_SCORE_RET GAMEBEACON_SUB_FMissionCheckScore( FMISSION_LOG_ACTION action, u16 param )
{
  LINK_FESTIVAL* fest_sys;

  if( GameBeaconSys == NULL || GameBeaconSys->gsys == NULL ){
    return FM_CHECK_SCORE_FAILED;
  }
  fest_sys = GAMESYSTEM_GetLinkFestivalWork( GameBeaconSys->gsys );

  if( !LINK_FESTIVAL_IsMissionEntry( fest_sys ) ){
    return FM_CHECK_SCORE_FAILED;
  }
  return ( LINK_FESTIVAL_MissionScoreCalc( GameBeaconSys->gsys, action, param ) + FM_CHECK_SCORE_SUCCESS);
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
 * @brief 文字数がFullの場合はEOMを差し込まないビーコン用独自コピー処理
 * ※EOM込みのバッファ長を確保できるものは、GFL_STR_GetStringCode()を使うこと
 *
 * @param   src_buf		  コピー元STRBUF
 * @param   dest		    コピー先STRCODEバッファ
 * @param   len	  	    コピーしたいSTRCODEバッファ長(=EOM抜きの文字数 GAMEBEACON_BUFLEN_FOR_ACTIONまで)
 */
//--------------------------------------------------------------
void GAMEBEACON_SUB_GetStringCodeForAction(const STRBUF *src, STRCODE *dest, u32 len )
{
  STRCODE temp[GAMEBEACON_BUFLEN_FOR_ACTION+EOM_SIZE];
  STRCODE code_eom = GFL_STR_GetEOMCode();
  int i;

  GF_ASSERT_MSG( len<=GAMEBEACON_BUFLEN_FOR_ACTION, "%d>GAMEBEACON_BUFLEN_FOR_ACTION",len);

  //文字数がFullの場合はEOMを差し込まない独自コピー
  GFL_STD_MemFill16( dest, code_eom, len*sizeof(STRCODE) );
  
  //STRBUFに入っているSTRCODEをEOMも込みで展開できる一時バッファに取り出す
  GFL_STR_GetStringCode( src, temp, len+EOM_SIZE);
  for(i = 0; i < len; i++){
    dest[i] = temp[i];
    if(temp[i] == code_eom){
      break;
    }
  }
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
  GAMEBEACON_SUB_GetStringCodeForAction( nickname, dest_nickname, MONS_NAME_SIZE );
#if 0
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
#endif
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

//==================================================================
/**
 * 送信データ更新：メダル獲得数をバッファに反映する
 *
 * @param   pms		
 */
//==================================================================
void GAMEBEACON_SendDataUpdate_MedalCount( u8 medal_count )
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  GAMEBEACON_INFO *info = &send->info;

  info->medal_count = medal_count;
  send->beacon_update = TRUE;
}

//==================================================================
/**
 * 送信データ更新：イチオシメダルをバッファに反映する
 *
 * @param   pms		
 */
//==================================================================
void GAMEBEACON_SendDataUpdate_PushMedal( MedalID push_medal )
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  GAMEBEACON_INFO *info = &send->info;

  info->push_medal = push_medal;
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
 * @param   eff_enc_f エフェクトエンカウントならTRUE
 * @param   mushroom_enc_f  タマゲタケシンボルエンカウントならTRUE
 */
//==================================================================
void GAMEBEACON_Set_PokemonGet( u16 monsno, BOOL eff_enc_f, BOOL mushroom_enc_f )
{
  if( GAMEBEACON_SUB_PokeBeaconTypeIsSpecial( monsno )){
    GAMEBEACON_Set_SpecialPokemonGet(monsno);
  }else{
    if( GAMEBEACON_Set_FMissionWildPokemonGet( monsno, eff_enc_f, mushroom_enc_f ) ){
      return;
    }
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
static void GAMEBEACON_Set_WildPokemonGet(u16 monsno)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_POKE_GET) == FALSE){ return; };
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
  BEACONINFO_Set_WildPokemonGet(&GameBeaconSys->send.info, monsno);
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
static void GAMEBEACON_Set_SpecialPokemonGet(u16 monsno)
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_SP_POKE_GET) == FALSE){ return; };
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
  BEACONINFO_Set_SpecialPokemonGet(&GameBeaconSys->send.info, monsno);
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
 * 送信ビーコンセット：フェスミッション野生ポケモン捕獲
 *
 * @param   monsno    ポケモン番号
 * @param   eff_enc_f エフェクトエンカウントならTRUE
 * @param   mushroom_enc_f  タマゲタケシンボルエンカウントならTRUE
 */
//==================================================================
static BOOL GAMEBEACON_Set_FMissionWildPokemonGet( u16 monsno, BOOL eff_enc_f, BOOL mushroom_enc_f )
{
  u8 action;
  FMISSION_LOG_ACTION log_act;

  if( eff_enc_f ){
    action = GAMEBEACON_ACTION_FMISSION_EFF_ENC_CAPTURE;
    log_act = FMISSION_LOG_ACT_EFF_ENC_CAPTURE;
  }else if( mushroom_enc_f ){
    action = GAMEBEACON_ACTION_FMISSION_MUSHROOM_CAPTURE;
    log_act = FMISSION_LOG_ACT_MUSHROOM_CAPTURE;
  }else{
    action = GAMEBEACON_ACTION_FMISSION_POKE_CAPTURE;
    log_act = FMISSION_LOG_ACT_POKE_CAPTURE;
  }
  if( GAMEBEACON_SUB_FMissionCheckScore( log_act, monsno ) ){
    GAMEBEACON_SUB_SendBeaconSetCommonForFMission( &GameBeaconSys->send, action, monsno );
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * 送信ビーコンセット：フェスミッション野生ポケモン倒した
 *
 * @param   monsno    ポケモン番号
 * @param   eff_enc_f エフェクトエンカウントならTRUE
 * @param   mushroom_enc_f  タマゲタケシンボルエンカウントならTRUE
 */
//==================================================================
void GAMEBEACON_Set_FMissionWildPokemonKnockOut( u16 monsno, BOOL eff_enc_f, BOOL mushroom_enc_f )
{
  u8 action;
  FMISSION_LOG_ACTION  log_act;

  if( eff_enc_f ){
    action = GAMEBEACON_ACTION_FMISSION_EFF_ENC_WIN;
    log_act = FMISSION_LOG_ACT_EFF_ENC_WIN;
  }else if( mushroom_enc_f ){
    action = GAMEBEACON_ACTION_FMISSION_MUSHROOM_WIN;
    log_act = FMISSION_LOG_ACT_MUSHROOM_WIN;
  }else{
    action = GAMEBEACON_ACTION_FMISSION_POKE_WIN;
    log_act = FMISSION_LOG_ACT_POKE_WIN;
  }
  if( GAMEBEACON_SUB_FMissionCheckScore( log_act, monsno ) ){
    GAMEBEACON_SUB_SendBeaconSetCommonForFMission( &GameBeaconSys->send, action, monsno );
  }
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
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
  BEACONINFO_Set_PokemonLevelUp(&GameBeaconSys->send.info, nickname);
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
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
  BEACONINFO_Set_PokemonEvolution(&GameBeaconSys->send.info, monsno, nickname);
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
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
  BEACONINFO_Set_PlayTime(&GameBeaconSys->send.info, hour);
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
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
  BEACONINFO_Set_DistributionPoke(&GameBeaconSys->send.info, monsno);
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
  if( ITEM_IsSwanNewItem( item ) ){
    GAMEBEACON_SUB_SendBeaconSetCommonSwan(&GameBeaconSys->send);
  }else{
    GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
  }
  BEACONINFO_Set_DistributionItem(&GameBeaconSys->send.info, item);
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
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
  BEACONINFO_Set_DistributionEtc(&GameBeaconSys->send.info);
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
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
  BEACONINFO_Set_DistributionGPower(&GameBeaconSys->send.info, g_power_id);
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
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
  BEACONINFO_Set_CriticalHit(&GameBeaconSys->send.info, nickname);
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
  GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
  BEACONINFO_Set_CriticalDamage(&GameBeaconSys->send.info, nickname);
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
  if( GAMEBEACON_SUB_FMissionCheckScore( FMISSION_LOG_ACT_ITEM_USE, item_no ) ){
    GAMEBEACON_SUB_SendBeaconSetCommonForFMission( &GameBeaconSys->send, GAMEBEACON_ACTION_FMISSION_ITEM_USE, item_no );
    return;
  }
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_USE_ITEM) == FALSE){ return; };
  if( ITEM_IsSwanNewItem( item_no ) ){
    GAMEBEACON_SUB_SendBeaconSetCommonSwan(&GameBeaconSys->send);
  }else{
    GAMEBEACON_SUB_SendBeaconSetCommon(&GameBeaconSys->send);
  }
  BEACONINFO_Set_UseItem(&GameBeaconSys->send.info, item_no);
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
  if( ZONEDATA_IsPastMap( zone_id )){
    return; //思い出リンクマップではゾーンを飛ばしたくないのでゾーンチェンジを通知しない
  }
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
  LINK_FESTIVAL*  fest_sys;

  //「～を移動中です」はrootではなく下記zone_idを使っている。
  //BTS1875の対処のため、WFBCを受信側で識別できるようにする。
  if( ZONEDATA_IsWfbc( zone_id ) )
  {
    info->zone_id = TOWNMAP_UTIL_GetRootZoneID(cp_gamedata, zone_id);
  }
  else
  {
    info->zone_id = zone_id;
  }
  info->townmap_root_zone_id = TOWNMAP_UTIL_GetRootZoneID(cp_gamedata, zone_id);
  info->details.details_no = GAMEBEACON_DETAILS_NO_WALK;
  BEACONINFO_Set_Details_Walk(info);

  //フェスミッションログ生成
  fest_sys = GAMESYSTEM_GetLinkFestivalWork( GameBeaconSys->gsys );
  if( !LINK_FESTIVAL_IsMissionEntry( fest_sys ) ){
    return;
  }
  if( LINK_FESTIVAL_TMapRootUpdate( fest_sys, info->townmap_root_zone_id ))
  {
    //自分用のミッションログを生成
    FMISSION_LOG_TBL_SetLogFromBeacon( fest_sys, info, LINK_FESTIVAL_GetMyScore(fest_sys), FMISSION_LOG_SET_MAP, TRUE );
  }
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

//==================================================================
/**
 * 送信ビーコンセット：フェスミッション配布
 */
//==================================================================
void GAMEBEACON_Set_FestMission( void )
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_FMISSION) == FALSE){ return; };

  GAMEBEACON_SUB_SendBeaconSetCommonForFMission( &GameBeaconSys->send, GAMEBEACON_ACTION_FMISSION, 0);
}

//==================================================================
/**
 * 送信ビーコンセット：配信フェスミッション配布
 *
 * デバッグ版のデバッグモード、および、スポット用にしか使われない
 */
//==================================================================
void GAMEBEACON_Set_DistributionFestMission( void )
{
  if(GAMEBEACON_SUB_CheckPriority(GAMEBEACON_ACTION_DISTRIBUTION_FMISSION) == FALSE){ return; };

  GAMEBEACON_SUB_SendBeaconSetCommonForFMission( &GameBeaconSys->send, GAMEBEACON_ACTION_DISTRIBUTION_FMISSION, 0);
}


//==================================================================
/**
 * 送信ビーコンセット：レゾナンスリゾートの配布許可
 *  レゾナンスリゾートはライブ通信を送信しない場合は常に出される
 *  そのための許可フラグを設定
 *
 * @param   BOOL  TRUE配信許可  FALSE配信不許可
 */
//==================================================================
void GAMEBEACON_Set_ResortDataEnable( BOOL is_enable )
{
  if( GameBeaconSys )
  {
    GameBeaconSys->resort_enable  = is_enable;
  }
}
//==================================================================
/**
 * 送信ビーコンバッファをセット：レゾナンスリゾートのバッファ拡張設定
 *  レゾナンスリゾートは通常はセーブデータへデータを設定するが、
 *  特定の場所では特定のメモリ上にも確保する
 *
 * @param   void *p_buff RESORT_RECV_BUFFのポインタ 使用しない場合はNULL
 */
//==================================================================
void GAMEBEACON_Set_ResortRecvBuffer( void *p_buff )
{
  if( GameBeaconSys )
  {
    GameBeaconSys->p_resort_buff  = p_buff;
  }
}
#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *	@brief　デバッグ用ビーコンバッファ取得
 *
 *	@return ビーコンバッファ取得
 */
//-----------------------------------------------------------------------------
void * GAMEBEACON_DEBUG_GetResortRecvBuffer( void )
{
  if( GameBeaconSys )
  {
    return GameBeaconSys->p_resort_buff;
  }
  return NULL;
}
#endif

//==================================================================
/**
 * 送信ビーコンセット：レゾナンスリゾートのデータ
 *  基本的にレゾナンスリゾートのデータ送信は自動で行われる。
 *  そのため、この関数を呼ぶことは基本的にはないはず。
 *
 * @param   const RESORT_BEACON_DATA *cp_data
 */
//==================================================================
static void GAMEBEACON_Set_SendResortData( GAMEBEACON_SYSTEM *bsys )
{

  RESORT_BEACON_DATA *p_data  = RESORT_BEACON_DATA_Init( GFL_HEAP_LOWID( GFL_HEAPID_APP ) );

  //一旦ビーコンデータにデータ集積
  RESORT_BEACON_DATA_SetMyData( p_data, bsys->resort_extend_index, bsys->gamedata );

  //ビーコンデータからビーコンにデータ設定
  RESORT_BEACON_DATA_GetBeaconInfo( p_data, &bsys->send.info );

  GAMEBEACON_SUB_SendBeaconSetCommon(&bsys->send);

  RESORT_BEACON_DATA_Exit( p_data );

  if( bsys->send.info.action.action_no == GAMEBEACON_ACTION_NULL )
  {
    bsys->send.info.action.action_no = GAMEBEACON_ACTION_SEARCH;
  }

#ifdef DEBUG_JA_EXTEND_BUFF_CHECK
  s_ja_extend_check_flag    = TRUE;
  GFL_STD_MemCopy( &bsys->send.info.action.extend_buff, s_ja_extend_check_buff, 20 );
#endif

}
//==================================================================
/**
 * リゾート用ビーコンの更新
 *
 */
//==================================================================
static BOOL SendBeacon_Update_ResortData( GAMEBEACON_SYSTEM *bsys )
{
  //拡張切り替えカウント
  if( bsys->resort_extend_count++ > sc_extend_frame_max[bsys->resort_extend_index] )
  {
    bsys->resort_extend_count = 0;
    bsys->resort_extend_index++;
    bsys->resort_extend_index %= RESORT_EXTEND_TYPE_MAX;
    return TRUE;
  }
  return FALSE;
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
  GF_ASSERT_HEAVY(GameBeaconSys != NULL);
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
  GF_ASSERT_HEAVY( bsys != NULL);

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
  GF_ASSERT_HEAVY(GameBeaconSys != NULL);
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

#ifdef DEBUG_JA_EXTEND_BUFF_CHECK
void DEBUG_GAMEBEACON_CheckJAExtendCheck(void)
{
  if( GameBeaconSys->send.info.action.action_no == GAMEBEACON_ACTION_SEARCH )
  {
    if( GFL_STD_MemComp( s_ja_extend_check_buff, GameBeaconSys->send.info.action.extend_buff, 20 ) != 0 )
    {
      GF_ASSERT_MSG( 0, "ジョインアベニュービーコンチェック\n" );
    }
  }
}
#endif

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON_BEACON

static void DEBUG_MAIN_TIME_AVERAGE_StartFunc( void )
{
  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Start = OS_GetTick(); 
}

static void DEBUG_MAIN_TIME_AVERAGE_EndFunc( void )
{
  OSTick now_time;
  DEBUG_DEBUG_MAIN_TIME_AVERAGE_End = OS_GetTick(); 

  now_time = OS_TicksToMicroSeconds(DEBUG_DEBUG_MAIN_TIME_AVERAGE_End - DEBUG_DEBUG_MAIN_TIME_AVERAGE_Start);

  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Now = now_time;

  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave += now_time;
  DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count ++;
  if( now_time > 16000 ){
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount ++;
    DEBUG_TIME_Printf( " update %d", DEBUG_MAIN_UPDATE_TYPE );
    DEBUG_TIME_Printf( " over tick %ld", now_time );
    DEBUG_TIME_Printf( " ave tick %d\n", (DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave / DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count) );
  }
  if( DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max < now_time ){
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max = now_time;
  }
  if( DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count >= DEBUG_DEBUG_MAIN_TIME_AVERAGE_AVE_COUNT_TIME ){

    DEBUG_TIME_Printf( " ave tick %d", 
        DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave / DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count );
    DEBUG_TIME_Printf( " Over 120::%d",
        DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount );
    DEBUG_TIME_Printf( " Max %d\n",
        DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max );

    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave = 0;
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Ave_Count = 0;
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_OverCount = 0;
    DEBUG_DEBUG_MAIN_TIME_AVERAGE_Max       = 0;
  }

  if( 1 || DEBUG_DEBUG_MAIN_TIME_AVERAGE_NOW_TIME_DRAW_KEY & GFL_UI_KEY_GetCont() ){
    DEBUG_TIME_Printf( " update[%d] tick %ld\n", DEBUG_MAIN_UPDATE_TYPE, now_time );
  }

}

#endif // DEBUG_MAIN_TIME_AVERAGE_MASTER_ON_BEACON

#endif  //PM_DEBUG

