//======================================================================
/**
 * @file  trial_house.c
 * @brief トライアルハウス
 * @author  Saito
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"   //for GFUser_GetPublicRand
#include "trial_house.h"
#include "trial_house_def.h"
#include "system/main.h"  //for HEAPID_APP_CONTROL
#include "fld_btl_inst_tool.h"

#include  "trial_house_scr_def.h"
#include "savedata/battle_examination.h"

//ビーコンサーチワーク
typedef struct
{
  GAMESYS_WORK *gsys;
  u16 *Ret;
}EVENT_WORK_BEACON_SEARCH;

static void MakeTrainer(TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount);
static void SetDownLoadData(GAMESYS_WORK * gsys, TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount);
static u16 GetTrainerOBJCode( TRIAL_HOUSE_WORK_PTR ptr );

static GMEVENT_RESULT BeaconSearchEvt( GMEVENT *event, int *seq, void *wk );

//--------------------------------------------------------------
/**
 * @brief	//トライアルハウスワーク確保関数
 * @retval	ptr      TRIAL_HOUSE_WORK_PTR
*/
//--------------------------------------------------------------
TRIAL_HOUSE_WORK_PTR TRIAL_HOUSE_Start(void)
{
  TRIAL_HOUSE_WORK_PTR ptr;
  int size = sizeof(TRIAL_HOUSE_WORK);
  NOZOMU_Printf("TrialHouseWorkSize = 0x%x\n",size);
  ptr = GFL_HEAP_AllocClearMemory(HEAPID_APP_CONTROL, size);
  ptr->HeapID = HEAPID_APP_CONTROL;//HEAPID_PROC;
  //パーティ作成
  ptr->Party = PokeParty_AllocPartyWork( ptr->HeapID );
  return ptr;
}

//--------------------------------------------------------------
/**
 * @brief	トライアルハウスワーク解放関数
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @retval		none
*/
//--------------------------------------------------------------
void TRIAL_HOUSE_End( TRIAL_HOUSE_WORK_PTR *ptr )
{
  if ( *ptr != NULL )
  {
    GFL_HEAP_FreeMemory((*ptr)->Party);
    GFL_HEAP_FreeMemory( *ptr );
    *ptr = NULL;
  }
  else GF_ASSERT(0);
}

//--------------------------------------------------------------
/**
 * @brief	トライアルハウス プレイモードのセット
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @param inPlayMode     プレイモード　 TH_PLAYMODE_SINGLE or TH_PLAYMODE_DOUBLE
 * @note  trial_house_scr_def.h参照
 * @retval		none
*/
//--------------------------------------------------------------
void TRIAL_HOUSE_SetPlayMode( TRIAL_HOUSE_WORK_PTR ptr, const u32 inPlayMode )
{
  BSWAY_PLAYMODE mode;
  if ( ptr == NULL )
  {
    GF_ASSERT(0);
    return;
  }

  if ( (inPlayMode == TH_PLAYMODE_SINGLE)||(inPlayMode == TH_PLAYMODE_DOUBLE) )
  {
    //シングル3　それ以外（ダブル）4
    if ( inPlayMode == TH_PLAYMODE_SINGLE )
    {
      mode = BSWAY_PLAYMODE_SINGLE; //バトルサブウェイのシングルモードをセット
      ptr->MemberNum = 3;
    }
    else
    {
      mode = BSWAY_PLAYMODE_DOUBLE; //バトルサブウェイのダブルモードをセット
      ptr->MemberNum = 4;
    }

    ptr->PlayMode = mode;
  }
  else
  {
    GF_ASSERT_MSG(0,"mode error %d",inPlayMode);
    ptr->PlayMode = BSWAY_PLAYMODE_SINGLE;
    ptr->MemberNum = 3;
  }

  //パーティの最大匹数をセットして初期化
  PokeParty_Init( ptr->Party, ptr->MemberNum );
}

//--------------------------------------------------------------
/**
 * @brief	トライアルハウス プレイモードのセット
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @param inDLFlg   ダウンロードデータで遊ぶか？　　TRUE:ダウンロードデータ　FALSE:ROMデータ
 * @retval		none
*/
//--------------------------------------------------------------
void TRIAL_HOUSE_SetDLFlg( TRIAL_HOUSE_WORK_PTR ptr, const BOOL inDLFlg )
{
  ptr->DownLoad = inDLFlg;
}

//--------------------------------------------------------------
/**
 * @brief	対戦相手の抽選
 * @param gsys        ゲームシステムポインタ
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @param inBtlCouont   対戦回数　0～4（最大五戦）
 * @retval	obj_id      OBJ見た目     
*/
//--------------------------------------------------------------
int TRIAL_HOUSE_MakeTrainer( GAMESYS_WORK * gsys, TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount )
{
  int obj_id;

  if ( ptr->DownLoad ) SetDownLoadData(gsys, ptr, inBtlCount);
  else MakeTrainer(ptr, inBtlCount);

  //見た目を取得して返す
  obj_id = GetTrainerOBJCode( ptr );
  NOZOMU_Printf("obj_id = %d\n",obj_id); 
  return obj_id;
}

//--------------------------------------------------------------
/**
 * @brief	対戦相手の抽選(ＲＯＭデータ)
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @param inBtlCouont   対戦回数　0～4（最大五戦）
 * @retval	none  
*/
//--------------------------------------------------------------
static void MakeTrainer(TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount)
{
  int obj_id;
  int tr_no;
  int base, band;

  switch(inBtlCount){
  case 0:
    base = 0;
    band = 100;
    break;
  case 1:
    base = 140;
    band = 20;
    break;
  case 2:
    base = 180;
    band = 20;
    break;
  case 3:
    base = 220;
    band = 80;
    break;
  case 4:
    base = 220;
    band = 80;
    break;
  default:
    GF_ASSERT_MSG(0,"count error %d",inBtlCount);
    base = 0;
    band = 100;
  }
  //トレーナー抽選
  tr_no = base + GFUser_GetPublicRand(band);
  NOZOMU_Printf("entry_tr_no = %d\n",tr_no);

  //データをトライアルハウスワークにセット
  FBI_TOOL_MakeRomTrainerData(
      &ptr->TrData,
      tr_no, ptr->MemberNum,
      NULL,NULL,NULL, GFL_HEAP_LOWID(ptr->HeapID));
}

//--------------------------------------------------------------
/**
 * @brief	対戦相手のセット(ダウンロードデータ)
 * @param gsys    ゲームシステムポインタ
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @param inBtlCouont   対戦回数　0～4（最大五戦）
 * @retval	none    
*/
//--------------------------------------------------------------
static void SetDownLoadData(GAMESYS_WORK * gsys, TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount)
{
  SAVE_CONTROL_WORK * sv;
  BATTLE_EXAMINATION_SAVEDATA *exa;
  BSUBWAY_PARTNER_DATA *data;
  {
    GAMEDATA *gdata = GAMESYSTEM_GetGameData(gsys);
    sv = GAMEDATA_GetSaveControlWork( gdata );
  }
  //セーブデータにアクセス
  exa = BATTLE_EXAMINATION_SAVE_GetSvPtr(sv);
  //データ取得
  data = BATTLE_EXAMINATION_SAVE_GetData(exa, inBtlCount);
  //トライアルハウスワークにデータをセット
  ptr->TrData = *data;
}

//--------------------------------------------------------------
/**
 * トレーナータイプからOBJコードを取得してくる
 * @param ptr   トライアルハウスポインタ
 * @param idx トレーナーデータインデックス
 * @retval u16 OBJコード
 */
//--------------------------------------------------------------
static u16 GetTrainerOBJCode( TRIAL_HOUSE_WORK_PTR ptr )
{
  return FBI_TOOL_GetTrainerOBJCode( ptr->TrData.bt_trd.tr_type );
}


//--------------------------------------------------------------
/**
 * @brief	対戦相手の先頭前メッセージ表示
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @retval		none
*/
//--------------------------------------------------------------
GMEVENT *TRIAL_HOUSE_CreateBeforeMsgEvt( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr )
{
  GMEVENT *event;

  event = FBI_TOOL_CreateTrainerBeforeMsgEvt(gsys, &ptr->TrData );

  return event;
}

//--------------------------------------------------------------
/**
 * @brief	ビーコンサーチ
 * @param	gsys        ゲームシステムポインタ
 * @param outRet      サーチ結果格納バッファ
 * @retval	event     イベントポインタ
*/
//--------------------------------------------------------------
GMEVENT *TRIAL_HOUSE_CreateBeaconSearchEvt( GAMESYS_WORK *gsys, u16* outRet )
{
  GMEVENT *event;
  EVENT_WORK_BEACON_SEARCH *evt_wk;
  event = GMEVENT_Create( gsys, NULL,
      BeaconSearchEvt, sizeof(EVENT_WORK_BEACON_SEARCH) );
  evt_wk = GMEVENT_GetEventWork( event );
  evt_wk->gsys = gsys;
  evt_wk->Ret = outRet;

  return event;
}

//--------------------------------------------------------------
/**
 * ビーコンサーチ  @todoまだ未完成
 * @param event GMEVENT
 * @param seq event sequence
 * @param wk event work
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT BeaconSearchEvt( GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK_BEACON_SEARCH *evt_wk = wk;
  GAMESYS_WORK *gsys = evt_wk->gsys;
  switch( *seq ){
  case 0:
    //ビーコンサーチ開始
    ;
    (*seq)++;
    break;
  case 1:
    //データ受け取り待ち
    if (1)
    {
      //受け取りの結果をセット（現行は失敗）@todo
      *(evt_wk->Ret) = FALSE;
      (*seq)++;
    }
    break;
  case 2:
    //終了
    return( GMEVENT_RES_FINISH );
  }
  
  return( GMEVENT_RES_CONTINUE );
}





