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
#include "msg/msg_beacon_status.h"
#include "savedata/encount_sv.h"


//==============================================================================
//  定数定義
//==============================================================================
///ビーコン寿命
#define LIFE_TIME     (15 * 60)

///行動番号
typedef enum{
  GAMEBEACON_ACTION_NULL,                 ///<データ無し
  GAMEBEACON_ACTION_APPEAL,               ///<「ちかくにいます」
  GAMEBEACON_ACTION_CONGRATULATIONS,      ///<「おめでとう！」
  GAMEBEACON_ACTION_POKE_EVOLUTION,       ///<ポケモン進化
  GAMEBEACON_ACTION_POKE_LVUP,            ///<ポケモンレベルアップ
  GAMEBEACON_ACTION_POKE_GET,             ///<ポケモン捕獲
  GAMEBEACON_ACTION_UNION_IN,             ///<ユニオンルーム入室
  GAMEBEACON_ACTION_UNION_OUT,            ///<ユニオンルーム退室
  GAMEBEACON_ACTION_ENCOUNT_DOWN,         ///<エンカウント率ダウン
  
  GAMEBEACON_ACTION_MAX,
}GAMEBEACON_ACTION;


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
  u16 padding;
}GAMEBEACON_SEND_MANAGER;

///ゲームビーコン管理システムのポインタ
typedef struct{
  GAMEDATA *gamedata;
  GAMEBEACON_SEND_MANAGER send;       ///<送信ビーコン管理
  GAMEBEACON_LOG log[GAMEBEACON_LOG_MAX]; ///<ログ(チェーンバッファ)
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

//--------------------------------------------------------------
/**
 * 送信ビーコン更新処理
 *
 * @param   send		
 */
//--------------------------------------------------------------
static void SendBeacon_Update(GAMEBEACON_SEND_MANAGER *send)
{
  if(send->info.action_no != GAMEBEACON_ACTION_APPEAL){
    send->life++;
    if(send->life > LIFE_TIME){
      send->life = 0;
      send->info.action_no = GAMEBEACON_ACTION_APPEAL;
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
  if(bsys->end_log >= GAMEBEACON_LOG_MAX){
    bsys->end_log = 0;
  }
  setlog = &bsys->log[bsys->end_log];

  if(bsys->log_num > 0 && bsys->end_log == bsys->start_log){
    bsys->start_log++;
    if(bsys->start_log >= GAMEBEACON_LOG_MAX){
      bsys->start_log = 0;
    }
  }
  
  GFL_STD_MemClear(setlog, sizeof(GAMEBEACON_LOG));
  setlog->info = *info;
  if(bsys->log_num < GAMEBEACON_LOG_MAX){
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
  
//  OS_TPrintf("SetRecv action_NO = %d\n", info->action_no);
  if(info->action_no == GAMEBEACON_ACTION_NULL || info->action_no >= GAMEBEACON_ACTION_MAX){
    return FALSE;
  }
  
  if(bsys->start_log <= bsys->end_log){
    for(i = bsys->start_log; i <= bsys->end_log; i++){
      if(bsys->log[i].info.trainer_id == info->trainer_id){
        if(info->action_no == GAMEBEACON_ACTION_APPEAL 
            || bsys->log[i].info.send_counter == info->send_counter){
//        OS_TPrintf("既に受信済み\n");
          return FALSE; //ログに同じデータを受信済み
        }
      }
    }
  }
  
  BeaconInfo_Set(bsys, info);
  bsys->log_count++;
  OS_TPrintf("セット完了 %d件目\n", bsys->log_count);
  
  //ログのパワー情報反映　※check ある程度パワーが決まってきたらフォーマット化する
  switch(info->action_no){
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
 * @param   log_no  最新のログから遡って何件目のログを取得したいか
 *                  (0だと最新：0～GAMEBEACON_LOG_MAX-1)
 *
 * @retval  GAMEBEACON_INFO *		NULLの場合はデータ無し
 */
//--------------------------------------------------------------
static GAMEBEACON_INFO * BeaconInfo_Get(GAMEBEACON_SYSTEM *bsys, int log_no)
{
  int log_pos;
  
  if(bsys->log_num == 0 || bsys->log_num - log_no <= 0){
    return NULL;  //データ無し
  }
  
  log_pos = bsys->end_log - log_no;
  if(log_pos < 0){
    log_pos = GAMEBEACON_LOG_MAX + log_pos;
  }
  return &bsys->log[log_pos].info;
}

//==================================================================
/**
 * ログからビーコン情報を取得する
 *
 * @param   log_no  最新のログから遡って何件目のログを取得したいか
 *                  (0だと最新：0～GAMEBEACON_LOG_MAX-1)
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
 * ログからビーコン情報のお気に入りの色を取得する
 *
 * @param   dest_buf		色の代入先へのポインタ
 * @param   log_no      最新のログから遡って何件目のログを取得したいか
 *                      (0だと最新：0～GAMEBEACON_LOG_MAX-1)
 *
 * @retval  BOOL		TRUE:ビーコン情報が存在している
 * @retval  BOOL		FALSE:ビーコン情報が無いため、色の取得が出来ない
 */
//==================================================================
BOOL GAMEBEACON_Get_FavoriteColor(GXRgb *dest_buf, int log_no)
{
  GAMEBEACON_INFO *info = BeaconInfo_Get(GameBeaconSys, log_no);
  if(info != NULL){
    *dest_buf = info->favorite_color;
    return TRUE;
  }
  return FALSE;
}


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

  info->action_no = GAMEBEACON_ACTION_APPEAL;
  MyStatus_CopyNameStrCode( myst, info->name, BUFLEN_PERSON_NAME );
  info->trainer_id = MyStatus_GetID(myst);
  info->favorite_color = *(OS_GetFavoriteColorTable());
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

  switch(info->action_no){
  case GAMEBEACON_ACTION_POKE_EVOLUTION:       ///<ポケモン進化
  case GAMEBEACON_ACTION_POKE_LVUP:            ///<ポケモンレベルアップ
  case GAMEBEACON_ACTION_POKE_GET:             ///<ポケモン捕獲
  	GFL_STR_SetStringCodeOrderLength(name_strbuf, info->nickname, BUFLEN_POKEMON_NAME);
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
  if(info->action_no == GAMEBEACON_ACTION_NULL || info->action_no >= GAMEBEACON_ACTION_MAX){
    GF_ASSERT_MSG(0, "action_no = %d\n", info->action_no);
    return msg_beacon_001;
  }
  return msg_beacon_001 + info->action_no - GAMEBEACON_ACTION_APPEAL;
}

//==================================================================
/**
 * ビーコンセット：「おめでとう！」
 */
//==================================================================
void GAMEBEACON_Set_Congratulations(void)
{
  GAMEBEACON_SEND_MANAGER *send = &GameBeaconSys->send;
  
  send->info.action_no = GAMEBEACON_ACTION_CONGRATULATIONS;
  SendBeacon_SetCommon(send);
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
  
  send->info.action_no = GAMEBEACON_ACTION_POKE_EVOLUTION;
  GFL_STR_GetStringCode(nickname, send->info.nickname, BUFLEN_POKEMON_NAME);
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
  
  send->info.action_no = GAMEBEACON_ACTION_POKE_LVUP;
  GFL_STR_GetStringCode(nickname, send->info.nickname, BUFLEN_POKEMON_NAME);
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
  
  send->info.action_no = GAMEBEACON_ACTION_POKE_GET;
  GFL_STR_GetStringCode(nickname, send->info.nickname, BUFLEN_POKEMON_NAME);
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
  
  send->info.action_no = GAMEBEACON_ACTION_UNION_IN;
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
  
  send->info.action_no = GAMEBEACON_ACTION_UNION_OUT;
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
  
  send->info.action_no = GAMEBEACON_ACTION_ENCOUNT_DOWN;
  SendBeacon_SetCommon(send);
}
