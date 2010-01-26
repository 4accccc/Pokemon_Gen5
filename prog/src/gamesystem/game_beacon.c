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


//==============================================================================
//  定数定義
//==============================================================================
///ビーコン寿命
#define LIFE_TIME     (15 * 60)

//ここのASSERTにひっかかったらupdate_logをbit管理ではなく配列管理に変更する
SDK_COMPILER_ASSERT(GAMEBEACON_SYSTEM_LOG_MAX < 32);


//==============================================================================
//  構造体定義
//==============================================================================
///1件のログデータ
typedef struct{
  GAMEBEACON_INFO info;
}GAMEBEACON_LOG;

///送信ビーコン管理
typedef struct{
  GAMEBEACON_INFO info;               ///<送信ビーコン
  s16 life;                           ///<送信寿命
  u8 beacon_update;                   ///<TRUE:送信ビーコン更新
  u8 padding;
}GAMEBEACON_SEND_MANAGER;

///ゲームビーコン管理システムのポインタ
typedef struct{
  GAMEDATA *gamedata;
  GAMEBEACON_SEND_MANAGER send;       ///<送信ビーコン管理
  GAMEBEACON_LOG log[GAMEBEACON_SYSTEM_LOG_MAX]; ///<ログ(チェーンバッファ)
  u32 update_log;                     ///<更新のあったログ番号をbitで管理(32を超えたら変える)
  u32 log_count;                      ///<ログ全体の受信件数をカウント
  s8 start_log;                       ///<ログのチェーン開始位置
  s8 end_log;                         ///<ログのチェーン終端位置
  s8 log_num;                         ///<ログ件数
  u8 padding;
}GAMEBEACON_SYSTEM;


//==============================================================================
//  ローカル変数
//==============================================================================
///ゲームビーコン管理システムのポインタ
static GAMEBEACON_SYSTEM * GameBeaconSys = NULL;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void SendBeacon_Update(GAMEBEACON_SEND_MANAGER *send);
static void BeaconInfo_Set(GAMEBEACON_SYSTEM *bsys, const GAMEBEACON_INFO *info);
static GAMEBEACON_INFO * BeaconInfo_Get(GAMEBEACON_SYSTEM *bsys, int log_no);
static void SendBeacon_Init(GAMEBEACON_SEND_MANAGER *send, GAMEDATA * gamedata);
static void SendBeacon_SetCommon(GAMEBEACON_SEND_MANAGER *send);


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
  
  if(GameBeaconSys == NULL){
    return;
  }

  SendBeacon_Update(&bsys->send);
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
    GFI_NET_BeaconSetInfo();
    GameBeaconSys->send.beacon_update = FALSE;
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
  if(send->info.action.action_no != GAMEBEACON_ACTION_APPEAL){
    send->life++;
    if(send->life > LIFE_TIME){
      send->life = 0;
      send->info.action.action_no = GAMEBEACON_ACTION_APPEAL;
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
  
  GFL_STD_MemClear(setlog, sizeof(GAMEBEACON_LOG));
  setlog->info = *info;
  bsys->update_log |= 1 < bsys->end_log;
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
  
//  OS_TPrintf("SetRecv action_NO = %d\n", info->action.action_no);
  if(info->action.action_no == GAMEBEACON_ACTION_NULL || info->action.action_no >= GAMEBEACON_ACTION_MAX){
    return FALSE;
  }
  
  if(bsys->start_log <= bsys->end_log){
    for(i = bsys->start_log; i <= bsys->end_log; i++){
      if(bsys->log[i].info.trainer_id == info->trainer_id){
        if(info->action.action_no == GAMEBEACON_ACTION_APPEAL 
            || bsys->log[i].info.send_counter == info->send_counter){
//        OS_TPrintf("既に受信済み\n");
          return FALSE; //ログに同じデータを受信済み
        }
        same_player = TRUE; //1プレイヤーは対になった1つのログバッファしか持たない
        break;
      }
    }
  }
  
  if(same_player == TRUE){
    bsys->log[i].info = *info;
    bsys->update_log |= 1 << i;
  }
  else{
    BeaconInfo_Set(bsys, info);
    bsys->log_count++;
    OS_TPrintf("セット完了 %d件目\n", bsys->log_count);
  }
  
  //ログのパワー情報反映　※check ある程度パワーが決まってきたらフォーマット化する
  switch(info->action.action_no){
  case GAMEBEACON_ACTION_ENCOUNT_DOWN:
    {
      SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(bsys->gamedata);
      ENC_SV_PTR evc_sv = EncDataSave_GetSaveDataPtr(sv_ctrl);
      EncDataSave_SetSprayCnt(evc_sv, 100);
    }
    break;
  }
  
  return TRUE;
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
//  int log_pos;
  
  if(bsys->log_num == 0 || bsys->log_num - log_no <= 0){
    return NULL;  //データ無し
  }
  
#if 0 //直接ログ位置を見るように変更 2010.01.18(月)
  log_pos = bsys->end_log - log_no;
  if(log_pos < 0){
    log_pos = GAMEBEACON_SYSTEM_LOG_MAX + log_pos;
  }
  return &bsys->log[log_pos].info;
#else
  return &bsys->log[log_no].info;
#endif
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
      return TRUE;
    }
  }
  return FALSE;
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
  const MISC *misc = SaveData_GetMisc( GAMEDATA_GetSaveControlWork(gamedata) );
  
  info->version_bit = 0xffff; //全バージョン指定
  info->zone_id = PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork(gamedata));
  info->g_power_id = GPOWER_ID_NULL;
  info->trainer_id = MyStatus_GetID(myst);
  MyStatus_CopyNameStrCode( myst, info->name, BUFLEN_PERSON_NAME );
  info->favorite_color = *(OS_GetFavoriteColorTable());
  info->trainer_view = MyStatus_GetTrainerView(myst);
  info->sex = MyStatus_GetMySex( myst );
  info->pm_version = PM_VERSION;
  info->language = PM_LANG;
  info->nation = MyStatus_GetMyNation(myst);
  info->area = MyStatus_GetMyArea(myst);
  
  info->thanks_recv_count = MISC_CrossComm_GetThanksRecvCount(misc);
  info->suretigai_count = MISC_CrossComm_GetSuretigaiCount(misc);
  { //自己紹介文コピー　文字数がFullの場合はEOMを差し込まないので
    //STRTOOL_Copyは使用せずに独自コピー
    STRCODE code_eom = GFL_STR_GetEOMCode();
    const STRCODE *src_code = MISC_CrossComm_GetSelfIntroduction(misc);
    int i;
    for(i = 0; i < GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN; i++){
      info->self_introduction[i] = src_code[i];
      if(src_code[i] == code_eom){
        break;
      }
    }
  }
  
  //詳細情報
  info->details.details_no = GAMEBEACON_DETAILS_NO_ROAD;
  
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
static void SendBeacon_SetCommon(GAMEBEACON_SEND_MANAGER *send)
{
  send->info.send_counter++;
  send->life = 0;
  send->beacon_update = TRUE;
}

//==================================================================
/**
 * ビーコン情報の内容をWORDSETする
 *
 * @param   info		        対象のビーコン情報へのポインタ
 * @param   wordset		
 * @param   temp_heap_id		内部でテンポラリとして使用するヒープID
 */
//==================================================================
void GAMEBEACON_Wordset(const GAMEBEACON_INFO *info, WORDSET *wordset, HEAPID temp_heap_id)
{
  STRBUF *name_strbuf = GFL_STR_CreateBuffer(BUFLEN_PERSON_NAME, temp_heap_id);
  STRBUF *nickname_strbuf = GFL_STR_CreateBuffer(BUFLEN_POKEMON_NAME, temp_heap_id);

	GFL_STR_SetStringCodeOrderLength(name_strbuf, info->name, BUFLEN_PERSON_NAME);
  WORDSET_RegisterWord(wordset, 0, name_strbuf, 0, TRUE, PM_LANG);

  switch(info->action.action_no){
  case GAMEBEACON_ACTION_POKE_EVOLUTION:       ///<ポケモン進化
  case GAMEBEACON_ACTION_POKE_LVUP:            ///<ポケモンレベルアップ
  case GAMEBEACON_ACTION_POKE_GET:             ///<ポケモン捕獲
  	GFL_STR_SetStringCodeOrderLength(name_strbuf, info->action.nickname, BUFLEN_POKEMON_NAME);
    WORDSET_RegisterWord(wordset, 1, name_strbuf, 0, TRUE, PM_LANG);
    break;
  }

  GFL_STR_DeleteBuffer(name_strbuf);
  GFL_STR_DeleteBuffer(nickname_strbuf);
}

//==================================================================
/**
 * ビーコン情報から表示するメッセージIDを取得する
 *
 * @param   info		対象のビーコン情報へのポインタ
 */
//==================================================================
u32 GAMEBEACON_GetMsgID(const GAMEBEACON_INFO *info)
{
  if(info->action.action_no == GAMEBEACON_ACTION_NULL || info->action.action_no >= GAMEBEACON_ACTION_MAX){
    GF_ASSERT_MSG(0, "action_no = %d\n", info->action.action_no);
    return msg_beacon_001;
  }
  return msg_beacon_001 + info->action.action_no - GAMEBEACON_ACTION_APPEAL;
}



//==============================================================================
//
//  ビーコンセット
//
//==============================================================================
//==================================================================
/**
 * ビーコンセット：「おめでとう！」
 */
//==================================================================
void GAMEBEACON_Set_Congratulations(void)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  
  send->info.action.action_no = GAMEBEACON_ACTION_CONGRATULATIONS;
  SendBeacon_SetCommon(send);
}

//==================================================================
/**
 * ビーコンセット：「ありがとう！」
 *
 * @param   gamedata		
 */
//==================================================================
void GAMEBEACON_Set_Thankyou(GAMEDATA *gamedata)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  const MISC *misc = SaveData_GetMisc( GAMEDATA_GetSaveControlWork(gamedata) );

  STRTOOL_Copy(MISC_CrossComm_GetSelfIntroduction(misc), 
    send->info.action.thankyou_message, GAMEBEACON_THANKYOU_MESSAGE_LEN);
  send->info.action.action_no = GAMEBEACON_ACTION_THANKYOU;
}

//==================================================================
/**
 * ビーコンセット：ポケモン進化
 *
 * @param   nickname		対象ポケモンのニックネーム
 */
//==================================================================
void GAMEBEACON_Set_PokemonEvolution(const STRBUF *nickname)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  
  send->info.action.action_no = GAMEBEACON_ACTION_POKE_EVOLUTION;
  GFL_STR_GetStringCode(nickname, send->info.action.nickname, BUFLEN_POKEMON_NAME);
  SendBeacon_SetCommon(send);
}

//==================================================================
/**
 * ビーコンセット：ポケモンレベルアップ
 *
 * @param   nickname		対象ポケモンのニックネーム
 */
//==================================================================
void GAMEBEACON_Set_PokemonLevelUp(const STRBUF *nickname)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  
  send->info.action.action_no = GAMEBEACON_ACTION_POKE_LVUP;
  GFL_STR_GetStringCode(nickname, send->info.action.nickname, BUFLEN_POKEMON_NAME);
  SendBeacon_SetCommon(send);
}

//==================================================================
/**
 * ビーコンセット：ポケモン捕獲
 *
 * @param   nickname		対象ポケモンのニックネーム
 */
//==================================================================
void GAMEBEACON_Set_PokemonGet(const STRBUF *nickname)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  
  send->info.action.action_no = GAMEBEACON_ACTION_POKE_GET;
  GFL_STR_GetStringCode(nickname, send->info.action.nickname, BUFLEN_POKEMON_NAME);
  SendBeacon_SetCommon(send);
}

//==================================================================
/**
 * ビーコンセット：ユニオンルーム入室
 */
//==================================================================
void GAMEBEACON_Set_UnionIn(void)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  
  send->info.action.action_no = GAMEBEACON_ACTION_UNION_IN;
  SendBeacon_SetCommon(send);
}

//==================================================================
/**
 * ビーコンセット：ユニオンルーム退室
 */
//==================================================================
void GAMEBEACON_Set_UnionOut(void)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  
  send->info.action.action_no = GAMEBEACON_ACTION_UNION_OUT;
  SendBeacon_SetCommon(send);
}

//==================================================================
/**
 * ビーコンセット：エンカウント率ダウン
 */
//==================================================================
void GAMEBEACON_Set_EncountDown(void)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  
  send->info.action.action_no = GAMEBEACON_ACTION_ENCOUNT_DOWN;
  SendBeacon_SetCommon(send);
}

//==================================================================
/**
 * ビーコンセット：ゾーン切り替え
 *
 * @param   zone_id   現在地
 */
//==================================================================
void GAMEBEACON_Set_ZoneChange(ZONEID zone_id)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  
  send->info.zone_id = zone_id;
  send->info.details.details_no = GAMEBEACON_DETAILS_NO_ROAD;
  SendBeacon_SetCommon(send);
}

void GAMEBEACON_Set_GPower(GPOWER_ID g_power_id)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  
  send->info.g_power_id = g_power_id;
  SendBeacon_SetCommon(send);
}

void GAMEBEACON_Set_ThanksRecvCount(u16 count)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;

  send->info.thanks_recv_count = count;
  SendBeacon_SetCommon(send);
}

void GAMEBEACON_Set_SuretigaiCount(u16 count)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;

  send->info.suretigai_count = count;
  SendBeacon_SetCommon(send);
}

void GAMEBEACON_Set_BattleTrainer(u16 trainer_code)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;

  send->info.details.details_no = GAMEBEACON_DETAILS_NO_BATTLE_TRAINER;
  send->info.details.battle_trainer = trainer_code;
  SendBeacon_SetCommon(send);
}

void GAMEBEACON_Set_BattlePokemon(u16 monsno)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;

  send->info.details.details_no = GAMEBEACON_DETAILS_NO_BATTLE_POKEMON;
  send->info.details.battle_monsno = monsno;
  SendBeacon_SetCommon(send);
}


