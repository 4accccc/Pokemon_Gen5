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


//==============================================================================
//  定数定義
//==============================================================================
///ビーコン寿命
#define LIFE_TIME     (15 * 60)

///配信用マジックキー：ポケモン
#define MAGIC_KEY_DISTRIBUTION_POKE    (0x2932013a)
///配信用マジックキー：アイテム
#define MAGIC_KEY_DISTRIBUTION_ITEM    (0x48841dc1)
///配信用マジックキー：その他
#define MAGIC_KEY_DISTRIBUTION_ETC     (0x701ddc92)

///ビーコン内容wordset用テンポラリバッファ長定義
#define BUFLEN_BEACON_WORDSET_TMP (16*2+EOM_SIZE)

enum{
 BEACON_WSET_DEFAULT,   //デフォルト(トレーナー名)  
 BEACON_WSET_TRNAME,    //対戦相手名
 BEACON_WSET_MONSNAME,  //ポケモン種族名
 BEACON_WSET_NICKNAME,  //ポケモンニックネーム
 BEACON_WSET_POKE_W,  //ポケモンニックネーム
 BEACON_WSET_ITEM,      //アイテム名
 BEACON_WSET_PTIME,     //プレイタイム
 BEACON_WSET_THANKS,    //御礼回数
 BEACON_WSET_HAIHU_MONS, //配布モンスター名
 BEACON_WSET_HAIHU_ITEM, //配布アイテム名
};

//ここのASSERTにひっかかったらupdate_logをbit管理ではなく配列管理に変更する
SDK_COMPILER_ASSERT(GAMEBEACON_SYSTEM_LOG_MAX < 32);

/*
 *  @brief  GAMEBEACON_ACTION型の並びと同一である必要があります
 */
static const u8 DATA_BeaconWordsetType[GAMEBEACON_ACTION_MAX] = {
  BEACON_WSET_DEFAULT,	///<GAMEBEACON_ACTION_NULL,                     ///<データ無し
  
  BEACON_WSET_DEFAULT,	///<GAMEBEACON_ACTION_SEARCH,                   ///<「ｘｘｘさんをサーチしました！」      1
  BEACON_WSET_MONSNAME,	///<GAMEBEACON_ACTION_BATTLE_WILD_POKE_START,   ///<野生のポケモンと対戦を開始しました！  2
  BEACON_WSET_MONSNAME,	///<GAMEBEACON_ACTION_BATTLE_WILD_POKE_VICTORY, ///<野生のポケモンに勝利しました！        3
  BEACON_WSET_MONSNAME,	///<GAMEBEACON_ACTION_BATTLE_SP_POKE_START,     ///<特別なポケモンと対戦を開始しました！  4
  BEACON_WSET_MONSNAME,	///<GAMEBEACON_ACTION_BATTLE_SP_POKE_VICTORY,   ///<特別なポケモンに勝利しました！        5
  BEACON_WSET_TRNAME,	///<GAMEBEACON_ACTION_BATTLE_TRAINER_START,     ///<トレーナーと対戦を開始しました！      6
  BEACON_WSET_TRNAME,	///<GAMEBEACON_ACTION_BATTLE_TRAINER_VICTORY,   ///<トレーナーに勝利しました！            7
  BEACON_WSET_TRNAME,	///<GAMEBEACON_ACTION_BATTLE_LEADER_START,      ///<ジムリーダーと対戦を開始しました！    8
  BEACON_WSET_TRNAME,	///<GAMEBEACON_ACTION_BATTLE_LEADER_VICTORY,    ///<ジムリーダーに勝利しました！          9
  BEACON_WSET_TRNAME,	///<GAMEBEACON_ACTION_BATTLE_BIGFOUR_START,     ///<四天王と対戦を開始しました！          10
  BEACON_WSET_TRNAME,	///<GAMEBEACON_ACTION_BATTLE_BIGFOUR_VICTORY,   ///<四天王に勝利しました！                11
  BEACON_WSET_TRNAME,	///<GAMEBEACON_ACTION_BATTLE_CHAMPION_START,    ///<チャンピオンと対戦を開始しました！    12
  BEACON_WSET_TRNAME,	///<GAMEBEACON_ACTION_BATTLE_CHAMPION_VICTORY,  ///<チャンピオンに勝利しました！          13
  BEACON_WSET_MONSNAME,	///<GAMEBEACON_ACTION_POKE_GET,                 ///<ポケモン捕獲                          14
  BEACON_WSET_MONSNAME,	///<GAMEBEACON_ACTION_SP_POKE_GET,              ///<特別なポケモン捕獲                    15
  BEACON_WSET_NICKNAME,	///<GAMEBEACON_ACTION_POKE_LVUP,                ///<ポケモンレベルアップ                  16
  BEACON_WSET_POKE_W,	///<GAMEBEACON_ACTION_POKE_EVOLUTION,           ///<ポケモン進化                          17
  BEACON_WSET_DEFAULT,	///<GAMEBEACON_ACTION_GPOWER,                   ///<Gパワー発動                           18
  BEACON_WSET_ITEM,	  ///<GAMEBEACON_ACTION_SP_ITEM_GET,              ///<貴重品ゲット                          19
  BEACON_WSET_PTIME,	///<GAMEBEACON_ACTION_PLAYTIME,                 ///<一定のプレイ時間を越えた              20
  BEACON_WSET_DEFAULT,	///<GAMEBEACON_ACTION_ZUKAN_COMPLETE,           ///<図鑑完成                              21
  BEACON_WSET_THANKS,	///<GAMEBEACON_ACTION_THANKYOU_OVER,            ///<お礼を受けた回数が規定数を超えた      22
  BEACON_WSET_DEFAULT,	///<GAMEBEACON_ACTION_UNION_IN,                 ///<ユニオンルームに入った                23
  BEACON_WSET_DEFAULT,	///<GAMEBEACON_ACTION_THANKYOU,                 ///<「ありがとう！」                      24
  BEACON_WSET_HAIHU_MONS,	///<GAMEBEACON_ACTION_DISTRIBUTION_POKE,        ///<ポケモン配布中                        25
  BEACON_WSET_HAIHU_ITEM,	///<GAMEBEACON_ACTION_DISTRIBUTION_ITEM,        ///<アイテム配布中                        26
  BEACON_WSET_DEFAULT,	///<GAMEBEACON_ACTION_DISTRIBUTION_ETC,         ///<その他配布中                          27
};

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
  u8 new_entry;                       ///<TRUE:新しい人物とすれ違った
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
static void GAMEBEACON_Set_ThanksRecvCount(u32 count);
static void GAMEBEACON_Set_SuretigaiCount(u32 count);

void BEACONINFO_Set_BattleWildPokeStart(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_BattleWildPokeVictory(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_BattleSpPokeStart(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_BattleSpPokeVictory(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_BattleTrainerStart(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleTrainerVictory(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleLeaderStart(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleLeaderVictory(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleBigFourStart(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleBigFourVictory(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleChampionStart(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_BattleChampionVictory(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_PokemonGet(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_SpecialPokemonGet(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_PokemonLevelUp(GAMEBEACON_INFO *info, const STRBUF *nickname);
void BEACONINFO_Set_PokemonEvolution(GAMEBEACON_INFO *info, u16 monsno, const STRBUF *nickname);
void BEACONINFO_Set_GPower(GAMEBEACON_INFO *info, GPOWER_ID g_power_id);
void BEACONINFO_Set_SpItemGet(GAMEBEACON_INFO *info, u16 item);
void BEACONINFO_Set_PlayTime(GAMEBEACON_INFO *info, u32 hour);
void BEACONINFO_Set_ZukanComplete(GAMEBEACON_INFO *info);
void BEACONINFO_Set_ThankyouOver(GAMEBEACON_INFO *info, u32 thankyou_count);
void BEACONINFO_Set_UnionIn(GAMEBEACON_INFO *info);
#ifdef PM_DEBUG
void BEACONINFO_Set_DistributionPoke(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_DistributionItem(GAMEBEACON_INFO *info, u16 item);
void BEACONINFO_Set_DistributionEtc(GAMEBEACON_INFO *info);
#endif
void BEACONINFO_Set_Thankyou(GAMEBEACON_INFO *info, GAMEDATA *gamedata, u32 target_trainer_id);
void BEACONINFO_Set_ZoneChange(GAMEBEACON_INFO *info, ZONEID zone_id, const GAMEDATA *cp_gamedata);
void BEACONINFO_Set_ThanksRecvCount(GAMEBEACON_INFO *info, u32 count);
void BEACONINFO_Set_SuretigaiCount(GAMEBEACON_INFO *info, u32 count);

void BEACONINFO_Set_Details_BattleWildPoke(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_Details_BattleSpPoke(GAMEBEACON_INFO *info, u16 monsno);
void BEACONINFO_Set_Details_BattleTrainer(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_Details_BattleLeader(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_Details_BattleBigFour(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_Details_BattleChampion(GAMEBEACON_INFO *info, u16 tr_no);
void BEACONINFO_Set_Details_Walk(GAMEBEACON_INFO *info);
void BEACONINFO_Set_Details_IntroductionPms(GAMEBEACON_INFO *info, const PMS_DATA *pms);

#ifdef PM_DEBUG
GAMEBEACON_INFO * DEBUG_SendBeaconInfo_GetPtr(void);
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
  if(send->info.action.action_no != GAMEBEACON_ACTION_SEARCH){
    send->life++;
    if(send->life > LIFE_TIME){
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
        if(info->action.action_no == GAMEBEACON_ACTION_SEARCH 
            || bsys->log[i].info.send_counter == info->send_counter){
//        OS_TPrintf("既に受信済み\n");
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
  
  if(same_player == TRUE){
    bsys->log[i].info = *info;
    bsys->update_log |= 1 << i;
  }
  else{
    BeaconInfo_Set(bsys, info);
    bsys->new_entry = TRUE;
    bsys->log_count++;
    OS_TPrintf("セット完了 %d件目\n", bsys->log_count);
  }
  
#if 0
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
#endif

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
  const MYPMS_DATA *mypms = SaveData_GetMyPmsDataConst( GAMEDATA_GetSaveControlWork(gamedata) );
  OSOwnerInfo owner_info;
  PMS_DATA pms;
  
  OS_GetOwnerInfo(&owner_info);
  
  info->version_bit = 0xffff; //全バージョン指定
  info->zone_id = PLAYERWORK_getZoneID(GAMEDATA_GetMyPlayerWork(gamedata));
  info->townmap_root_zone_id = TOWNMAP_UTIL_GetRootZoneID(gamedata, info->zone_id);
  info->g_power_id = GPOWER_ID_NULL;
  info->trainer_id = MyStatus_GetID_Low(myst);
  info->favorite_color_index = owner_info.favoriteColor;
  info->trainer_view = MyStatus_GetTrainerView(myst);
  info->sex = MyStatus_GetMySex( myst );
  info->pm_version = PM_VERSION;
  info->language = PM_LANG;
  info->nation = MyStatus_GetMyNation(myst);
  info->area = MyStatus_GetMyArea(myst);
  info->research_team_rank = MISC_CrossComm_GetResearchTeamRank(misc);
  
  info->thanks_recv_count = MISC_CrossComm_GetThanksRecvCount(misc);
  info->suretigai_count = MISC_CrossComm_GetSuretigaiCount(misc);
  
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
  u8 type;
  STRBUF *strbuf = GFL_STR_CreateBuffer( BUFLEN_BEACON_WORDSET_TMP , temp_heap_id);

  //トレーナー名展開(デフォルト)
	GFL_STR_SetStringCodeOrderLength(strbuf, info->name, PERSON_NAME_SIZE+EOM_SIZE);
  WORDSET_RegisterWord( wordset, 0, strbuf, 0, TRUE, PM_LANG);

  type = DATA_BeaconWordsetType[ info->action.action_no ];

  switch( type ){
  case BEACON_WSET_TRNAME:
    WORDSET_RegisterTrainerName( wordset, 1, info->action.tr_no );
    break;
  case BEACON_WSET_MONSNAME:
  case BEACON_WSET_POKE_W:
    WORDSET_RegisterPokeMonsNameNo( wordset, 1, info->action.monsno );
    if( type == BEACON_WSET_MONSNAME ){
      break;
    }
    //ブレイクスルー
  case BEACON_WSET_NICKNAME:
  	GFL_STR_SetStringCodeOrderLength(strbuf, info->action.normal.nickname, MONS_NAME_SIZE+EOM_SIZE);
    WORDSET_RegisterWord(wordset, 2, strbuf, 0, TRUE, PM_LANG);
    break;
  case BEACON_WSET_ITEM:
    WORDSET_RegisterItemName( wordset, 1, info->action.itemno );
    break;
  case BEACON_WSET_PTIME:
    WORDSET_RegisterNumber( wordset, 1, info->action.hour, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    break;
  case BEACON_WSET_THANKS:
    WORDSET_RegisterNumber( wordset, 1, info->action.thankyou_count, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    break;
  case BEACON_WSET_HAIHU_MONS:
    WORDSET_RegisterItemName( wordset, 1, info->action.distribution.monsno );
    break;
  case BEACON_WSET_HAIHU_ITEM:
    WORDSET_RegisterItemName( wordset, 1, info->action.distribution.itemno );
    break;
  }
  GFL_STR_DeleteBuffer(strbuf);
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
  return msg_beacon_001 + info->action.action_no - GAMEBEACON_ACTION_SEARCH;
}


//==============================================================================
//
//  アンケート
//
//==============================================================================
//==================================================================
/**
 * 新しい人物とすれ違ったかを取得する
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

//==================================================================
/**
 * デバッグ用：強制で新しい人物とすれ違ったフラグを立てる
 */
//==================================================================
#ifdef PM_DEBUG
void DEBUG_GAMEBEACON_Set_NewEntry(void)
{
  GameBeaconSys->new_entry = TRUE;
}
#endif


//==============================================================================
//
//  ビーコンセット
//
//==============================================================================
//==================================================================
/**
 * 送信ビーコンセット：野生のポケモンと対戦を開始しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_BattleWildPokeStart(u16 monsno)
{
  BEACONINFO_Set_BattleWildPokeStart(&GameBeaconSys->send.info, monsno);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：野生のポケモンと対戦を開始しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void BEACONINFO_Set_BattleWildPokeStart(GAMEBEACON_INFO *info, u16 monsno)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_WILD_POKE_START;
  info->action.monsno = monsno;
  
  BEACONINFO_Set_Details_BattleWildPoke(info, monsno);
}

//==================================================================
/**
 * 送信ビーコンセット：野生のポケモンとの対戦に勝利しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_BattleWildPokeVictory(u16 monsno)
{
  BEACONINFO_Set_BattleWildPokeVictory(&GameBeaconSys->send.info, monsno);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：野生のポケモンとの対戦に勝利しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void BEACONINFO_Set_BattleWildPokeVictory(GAMEBEACON_INFO *info, u16 monsno)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_WILD_POKE_VICTORY;
  info->action.monsno = monsno;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：特別なポケモンと対戦を開始しました
 * @param   tr_no   ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_BattleSpPokeStart(u16 monsno)
{
  BEACONINFO_Set_BattleSpPokeStart(&GameBeaconSys->send.info, monsno);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：特別なポケモンと対戦を開始しました
 * @param   tr_no   ポケモン番号
 */
//==================================================================
void BEACONINFO_Set_BattleSpPokeStart(GAMEBEACON_INFO *info, u16 monsno)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_SP_POKE_START;
  info->action.monsno = monsno;

  BEACONINFO_Set_Details_BattleSpPoke(info, monsno);
}

//==================================================================
/**
 * 送信ビーコンセット：特別なポケモンとの対戦に勝利しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_BattleSpPokeVictory(u16 monsno)
{
  BEACONINFO_Set_BattleSpPokeVictory(&GameBeaconSys->send.info, monsno);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：特別なポケモンとの対戦に勝利しました
 * @param   monsno   ポケモン番号
 */
//==================================================================
void BEACONINFO_Set_BattleSpPokeVictory(GAMEBEACON_INFO *info, u16 monsno)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_SP_POKE_VICTORY;
  info->action.monsno = monsno;
  
  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：トレーナーと対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleTrainerStart(u16 tr_no)
{
  BEACONINFO_Set_BattleTrainerStart(&GameBeaconSys->send.info, tr_no);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：トレーナーと対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleTrainerStart(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_TRAINER_START;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_BattleTrainer(info, tr_no);
}

//==================================================================
/**
 * 送信ビーコンセット：トレーナーとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleTrainerVictory(u16 tr_no)
{
  BEACONINFO_Set_BattleTrainerVictory(&GameBeaconSys->send.info, tr_no);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：トレーナーとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleTrainerVictory(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_TRAINER_VICTORY;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：ジムリーダーと対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleLeaderStart(u16 tr_no)
{
  BEACONINFO_Set_BattleLeaderStart(&GameBeaconSys->send.info, tr_no);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：トレーナーとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleLeaderStart(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_LEADER_START;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_BattleLeader(info, tr_no);
}

//==================================================================
/**
 * 送信ビーコンセット：ジムリーダーとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleLeaderVictory(u16 tr_no)
{
  BEACONINFO_Set_BattleLeaderVictory(&GameBeaconSys->send.info, tr_no);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ジムリーダーとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleLeaderVictory(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_LEADER_VICTORY;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：四天王と対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleBigFourStart(u16 tr_no)
{
  BEACONINFO_Set_BattleBigFourStart(&GameBeaconSys->send.info, tr_no);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：四天王と対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleBigFourStart(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_BIGFOUR_START;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_BattleBigFour(info, tr_no);
}

//==================================================================
/**
 * 送信ビーコンセット：四天王との対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleBigFourVictory(u16 tr_no)
{
  BEACONINFO_Set_BattleBigFourVictory(&GameBeaconSys->send.info, tr_no);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：四天王との対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleBigFourVictory(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_BIGFOUR_VICTORY;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：チャンピオンと対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleChampionStart(u16 tr_no)
{
  BEACONINFO_Set_BattleChampionStart(&GameBeaconSys->send.info, tr_no);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：チャンピオンと対戦を開始しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleChampionStart(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_CHAMPION_START;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_BattleChampion(info, tr_no);
}

//==================================================================
/**
 * 送信ビーコンセット：チャンピオンとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void GAMEBEACON_Set_BattleChampionVictory(u16 tr_no)
{
  BEACONINFO_Set_BattleChampionVictory(&GameBeaconSys->send.info, tr_no);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：チャンピオンとの対戦に勝利しました
 * @param   tr_no   トレーナー番号
 */
//==================================================================
void BEACONINFO_Set_BattleChampionVictory(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->action.action_no = GAMEBEACON_ACTION_BATTLE_CHAMPION_VICTORY;
  info->action.tr_no = tr_no;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：ポケモン捕獲
 *
 * @param   monsno    ポケモン番号
 */
//==================================================================
void GAMEBEACON_Set_PokemonGet(u16 monsno)
{
  BEACONINFO_Set_PokemonGet(&GameBeaconSys->send.info, monsno);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ポケモン捕獲
 *
 * @param   monsno    ポケモン番号
 */
//==================================================================
void BEACONINFO_Set_PokemonGet(GAMEBEACON_INFO *info, u16 monsno)
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
  BEACONINFO_Set_SpecialPokemonGet(&GameBeaconSys->send.info, monsno);
  SendBeacon_SetCommon(&GameBeaconSys->send);
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
  info->action.action_no = GAMEBEACON_ACTION_POKE_GET;
  info->action.monsno = monsno;

  BEACONINFO_Set_Details_Walk(info);
}

//--------------------------------------------------------------
/**
 * ニックネームのコピー　※文字数がFullの場合はEOMを差し込まないので独自コピー処理
 *
 * @param   nickname		    コピー元ニックネーム
 * @param   dest_nickname		ニックネームコピー先
 */
//--------------------------------------------------------------
static void _StrbufNicknameCopy(const STRBUF *nickname, STRCODE *dest_nickname)
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

//==================================================================
/**
 * 送信ビーコンセット：ポケモンレベルアップ
 *
 * @param   nickname		対象ポケモンのニックネーム
 */
//==================================================================
void GAMEBEACON_Set_PokemonLevelUp(const STRBUF *nickname)
{
  BEACONINFO_Set_PokemonLevelUp(&GameBeaconSys->send.info, nickname);
  SendBeacon_SetCommon(&GameBeaconSys->send);
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
  _StrbufNicknameCopy(nickname, info->action.normal.nickname);

  BEACONINFO_Set_Details_Walk(info);
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
  BEACONINFO_Set_PokemonEvolution(&GameBeaconSys->send.info, monsno, nickname);
  SendBeacon_SetCommon(&GameBeaconSys->send);
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
  _StrbufNicknameCopy(nickname, info->action.normal.nickname);

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信Gパワー発動
 *
 * @param   g_power_id		GパワーID
 */
//==================================================================
void GAMEBEACON_Set_GPower(GPOWER_ID g_power_id)
{
  BEACONINFO_Set_GPower(&GameBeaconSys->send.info, g_power_id);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * Gパワー発動
 *
 * @param   g_power_id		GパワーID
 */
//==================================================================
void BEACONINFO_Set_GPower(GAMEBEACON_INFO *info, GPOWER_ID g_power_id)
{
  info->action.action_no = GAMEBEACON_ACTION_GPOWER;
  info->g_power_id = g_power_id;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：貴重品をプレイヤーが入手
 *
 * @param   item		アイテム番号
 */
//==================================================================
void GAMEBEACON_Set_SpItemGet(u16 item)
{
  BEACONINFO_Set_SpItemGet(&GameBeaconSys->send.info, item);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：貴重品をプレイヤーが入手
 *
 * @param   item		アイテム番号
 */
//==================================================================
void BEACONINFO_Set_SpItemGet(GAMEBEACON_INFO *info, u16 item)
{
  info->action.action_no = GAMEBEACON_ACTION_SP_ITEM_GET;
  info->action.itemno = item;

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
  BEACONINFO_Set_PlayTime(&GameBeaconSys->send.info, hour);
  SendBeacon_SetCommon(&GameBeaconSys->send);
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

//==================================================================
/**
 * 送信ビーコンセット：ポケモン図鑑が完成
 */
//==================================================================
void GAMEBEACON_Set_ZukanComplete(void)
{
  BEACONINFO_Set_ZukanComplete(&GameBeaconSys->send.info);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ポケモン図鑑が完成
 */
//==================================================================
void BEACONINFO_Set_ZukanComplete(GAMEBEACON_INFO *info)
{
  info->action.action_no = GAMEBEACON_ACTION_ZUKAN_COMPLETE;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：お礼を受けた回数が規定数を超えた
 * @param   thankyou_count	お礼回数
 */
//==================================================================
void GAMEBEACON_Set_ThankyouOver(u32 thankyou_count)
{
  BEACONINFO_Set_ThankyouOver(&GameBeaconSys->send.info, thankyou_count);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：お礼を受けた回数が規定数を超えた
 * @param   thankyou_count	お礼回数
 */
//==================================================================
void BEACONINFO_Set_ThankyouOver(GAMEBEACON_INFO *info, u32 thankyou_count)
{
  info->action.action_no = GAMEBEACON_ACTION_THANKYOU_OVER;
  info->action.thankyou_count = thankyou_count;

  BEACONINFO_Set_Details_Walk(info);
}

//==================================================================
/**
 * 送信ビーコンセット：ユニオンルーム入室
 */
//==================================================================
void GAMEBEACON_Set_UnionIn(void)
{
  BEACONINFO_Set_UnionIn(&GameBeaconSys->send.info);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：ユニオンルーム入室
 */
//==================================================================
void BEACONINFO_Set_UnionIn(GAMEBEACON_INFO *info)
{
  info->action.action_no = GAMEBEACON_ACTION_UNION_IN;

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
  BEACONINFO_Set_DistributionPoke(&GameBeaconSys->send.info, monsno);
  SendBeacon_SetCommon(&GameBeaconSys->send);
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
  BEACONINFO_Set_DistributionItem(&GameBeaconSys->send.info, item);
  SendBeacon_SetCommon(&GameBeaconSys->send);
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
  BEACONINFO_Set_DistributionEtc(&GameBeaconSys->send.info);
  SendBeacon_SetCommon(&GameBeaconSys->send);
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
#endif  //PM_DEBUG

//==================================================================
/**
 * 送信ビーコンセット：「ありがとう！」
 *
 * @param   gamedata		
 * @param   target_trainer_id   お礼をする相手のトレーナーID
 */
//==================================================================
void GAMEBEACON_Set_Thankyou(GAMEDATA *gamedata, u32 target_trainer_id)
{
  BEACONINFO_Set_Thankyou(&GameBeaconSys->send.info, gamedata, target_trainer_id);
  SendBeacon_SetCommon(&GameBeaconSys->send);
}

//==================================================================
/**
 * ビーコンセット：「ありがとう！」
 *
 * @param   gamedata		
 * @param   target_trainer_id   お礼をする相手のトレーナーID
 */
//==================================================================
void BEACONINFO_Set_Thankyou(GAMEBEACON_INFO *info, GAMEDATA *gamedata, u32 target_trainer_id)
{
  const MISC *misc = SaveData_GetMisc( GAMEDATA_GetSaveControlWork(gamedata) );

  STRTOOL_Copy(MISC_CrossComm_GetSelfIntroduction(misc), 
    info->action.thankyou_message, GAMEBEACON_THANKYOU_MESSAGE_LEN);
  info->action.action_no = GAMEBEACON_ACTION_THANKYOU;
  info->action.target_trainer_id = target_trainer_id;
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

//==================================================================
/**
 * 送信その他セット：お礼を受けた回数
 *
 * @param   count		
 */
//==================================================================
static void GAMEBEACON_Set_ThanksRecvCount(u32 count)
{
  BEACONINFO_Set_ThanksRecvCount(&GameBeaconSys->send.info, count);
}

//==================================================================
/**
 * その他セット：お礼を受けた回数
 *
 * @param   count		
 */
//==================================================================
void BEACONINFO_Set_ThanksRecvCount(GAMEBEACON_INFO *info, u32 count)
{
  info->thanks_recv_count = count;
}

//==================================================================
/**
 * 送信その他セット：すれ違った人数
 *
 * @param   count		
 */
//==================================================================
static void GAMEBEACON_Set_SuretigaiCount(u32 count)
{
  BEACONINFO_Set_SuretigaiCount(&GameBeaconSys->send.info, count);
}

//==================================================================
/**
 * その他セット：すれ違った人数
 *
 * @param   count		
 */
//==================================================================
void BEACONINFO_Set_SuretigaiCount(GAMEBEACON_INFO *info, u32 count)
{
  info->suretigai_count = count;
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
  info->details.details_no = GAMEBEACON_DETAILS_NO_BATTLE_JIMLEADER;
  info->details.battle_trainer = tr_no;
}

//--------------------------------------------------------------
/**
 * 四天王と対戦中
 * @param   tr_no		トレーナー番号
 */
//--------------------------------------------------------------
void BEACONINFO_Set_Details_BattleBigFour(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->details.details_no = GAMEBEACON_DETAILS_NO_BATTLE_BIGFOUR;
  info->details.battle_trainer = tr_no;
}

//--------------------------------------------------------------
/**
 * チャンピオンと対戦中
 * @param   tr_no		トレーナー番号
 */
//--------------------------------------------------------------
void BEACONINFO_Set_Details_BattleChampion(GAMEBEACON_INFO *info, u16 tr_no)
{
  info->details.details_no = GAMEBEACON_DETAILS_NO_BATTLE_CHAMPION;
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

#ifdef PM_DEBUG
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

#endif  //PM_DEBUG

