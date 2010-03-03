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
#include "savedata/th_rank_def.h"
#include "savedata/save_tbl.h"    //for SAVESIZE_EXTRA_BATTLE_EXAMINATION

#include "net/delivery_beacon.h"

//ビーコンサーチワーク
typedef struct
{
  DELIVERY_BEACON_INIT BeaconInit;
  DELIVERY_BEACON_WORK *BeaconWork;
  GAMESYS_WORK *gsys;
  TRIAL_HOUSE_WORK_PTR Ptr;
  u16 *Ret;
  int Time;
}EVENT_WORK_BEACON_SEARCH;

static void MakeTrainer(TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount);
static void SetDownLoadData( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount);
static u16 GetTrainerOBJCode( TRIAL_HOUSE_WORK_PTR ptr );

static GMEVENT_RESULT BeaconSearchEvt( GMEVENT *event, int *seq, void *wk );

typedef enum
{
  BEACON_SEQ_START = 0,
  BEACON_SEQ_SEARCH,
  BEACON_SEQ_CHECK,
  BEACON_SEQ_MAIN,
  BEACON_SEQ_END
}BEACON_SEQ;

//--------------------------------------------------------------
/**
 * @brief	//トライアルハウスワーク確保関数
 * @param   gsys      ゲームシステムポインタ
 * @retval	ptr      TRIAL_HOUSE_WORK_PTR
*/
//--------------------------------------------------------------
TRIAL_HOUSE_WORK_PTR TRIAL_HOUSE_Start(GAMESYS_WORK * gsys)
{
  TRIAL_HOUSE_WORK_PTR ptr;
  int size = sizeof(TRIAL_HOUSE_WORK);
  int comm_size = BATTLE_EXAMINATION_SAVE_GetWorkSize();
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata);

  NOZOMU_Printf("TrialHouseWorkSize = 0x%x\n",size);
  NOZOMU_Printf("ExaminationSize = 0x%x\n",comm_size);
  ptr = GFL_HEAP_AllocClearMemory(HEAPID_APP_CONTROL, size);
  ptr->HeapID = HEAPID_APP_CONTROL;//HEAPID_PROC;
  ptr->CommBuf = GFL_HEAP_AllocClearMemory(HEAPID_APP_CONTROL, comm_size);
  ptr->ThSaveWork = GFL_HEAP_AllocClearMemory(GFL_HEAP_LOWID(HEAPID_PROC), SAVESIZE_EXTRA_BATTLE_EXAMINATION);

  if (sv != NULL)
  {
    //外部データをロード
    if ( LOAD_RESULT_OK == SaveControl_Extra_LoadWork(sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION, GFL_HEAP_LOWID(HEAPID_PROC),
                                                     ptr->ThSaveWork, SAVESIZE_EXTRA_BATTLE_EXAMINATION) )
    {
      OS_Printf("トライアルハウス外部データロード\n");
    }
    else GF_ASSERT_MSG(0,"extra_data_load fail");
  }
  //パーティ作成
  ptr->Party = PokeParty_AllocPartyWork( ptr->HeapID );
  return ptr;
}

//--------------------------------------------------------------
/**
 * @brief	トライアルハウスワーク解放関数
 * @param   gsys  ゲームシステムポインタ
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @retval		none
*/
//--------------------------------------------------------------
void TRIAL_HOUSE_End( GAMESYS_WORK * gsys, TRIAL_HOUSE_WORK_PTR *ptr )
{
  if ( *ptr != NULL )
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata);
    SaveControl_Extra_UnloadWork(sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION);
    GFL_HEAP_FreeMemory( (*ptr)->ThSaveWork );
    GFL_HEAP_FreeMemory( (*ptr)->CommBuf );
    GFL_HEAP_FreeMemory( (*ptr)->Party );
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
 * @param gsys      ゲームシステムポインタ
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @param inBtlCouont   対戦回数　0～4（最大五戦）
 * @retval	obj_id      OBJ見た目     
*/
//--------------------------------------------------------------
int TRIAL_HOUSE_MakeTrainer( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount )
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
 * @param	ptr      TRIAL_HOUSE_WORK_PTR
 * @param inBtlCouont   対戦回数　0～4（最大五戦）
 * @retval	none    
*/
//--------------------------------------------------------------
static void SetDownLoadData( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount)
{
  BATTLE_EXAMINATION_SAVEDATA *exa;
  BSUBWAY_PARTNER_DATA *data;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata);
  exa = SaveControl_Extra_DataPtrGet( sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION, 0);
  
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
 * @param ptr         トライアルハウスワークポインタ
 * @param outRet      サーチ結果格納バッファ
 * @retval	event     イベントポインタ
*/
//--------------------------------------------------------------
GMEVENT *TRIAL_HOUSE_CreateBeaconSearchEvt( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr, u16* outRet )
{
  GMEVENT *event;
  EVENT_WORK_BEACON_SEARCH *evt_wk;
  event = GMEVENT_Create( gsys, NULL,
      BeaconSearchEvt, sizeof(EVENT_WORK_BEACON_SEARCH) );
  evt_wk = GMEVENT_GetEventWork( event );
  evt_wk->gsys = gsys;
  evt_wk->Ret = outRet;
  evt_wk->Ptr = ptr;
  evt_wk->Time = 0;
  //ビーコン構造体初期化
  {
    DELIVERY_BEACON_INIT *init;
    init = &evt_wk->BeaconInit;
    init->NetDevID = WB_NET_BATTLE_EXAMINATION;   // //通信種類
    init->datasize = BATTLE_EXAMINATION_SAVE_GetWorkSize();  //データ全体サイズ
    init->pData = ptr->CommBuf;     // 受信バッファデータ
    init->ConfusionID = 0;   //混線しないためのID
    init->heapID = HEAPID_APP_CONTROL;
  }

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
  case BEACON_SEQ_START:
    {
      BOOL rc;
      //ビーコンサーチ開始
      evt_wk->BeaconWork = DELIVERY_BEACON_Init(&evt_wk->BeaconInit);
      rc = DELIVERY_BEACON_RecvStart(evt_wk->BeaconWork);
      if (!rc)
      { //開始失敗
        GF_ASSERT_MSG(0,"beacon_start_error");
        //受け取りの結果をセット
        *(evt_wk->Ret) = FALSE;   //失敗
        (*seq) = BEACON_SEQ_END;
      }
      else (*seq) = BEACON_SEQ_SEARCH;
    }
    break;
  case BEACON_SEQ_SEARCH:
    //配信サービス検出のため、２秒間位配送メインをまわす
    {
      //配送メイン
      DELIVERY_BEACON_Main( evt_wk->BeaconWork );
    }
    evt_wk->Time++;
    if ( evt_wk->Time > 30*2 ) (*seq) = BEACON_SEQ_CHECK;
    break;
  case BEACON_SEQ_CHECK:
    //配送メイン
    DELIVERY_BEACON_Main( evt_wk->BeaconWork );
    {
      BOOL rc;
      //ビーコンが１つでもあるかどうか（ 配信サービスがやっているかどうか）
      rc = DELIVERY_BEACON_RecvSingleCheck( evt_wk->BeaconWork );
      if (rc) (*seq) = BEACON_SEQ_MAIN;
      else
      {   //配信していない
        //受け取りの結果をセット
        *(evt_wk->Ret) = FALSE;   //失敗
        (*seq) = BEACON_SEQ_END;
      }
    }
    break;
  case BEACON_SEQ_MAIN:
    //配送メイン
    DELIVERY_BEACON_Main( evt_wk->BeaconWork );
    //データ受け取り待ち
    if ( DELIVERY_BEACON_RecvCheck( evt_wk->BeaconWork )  )
    {
      BATTLE_EXAMINATION_SAVEDATA *exa;
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
      SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata); 
      exa = SaveControl_Extra_DataPtrGet( sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION, 0);
      //バッファからセーブへデータコピー
      GFL_STD_MemCopy( evt_wk->Ptr->CommBuf, exa, BATTLE_EXAMINATION_SAVE_GetWorkSize() );
      NOZOMU_Printf("データ受け取り成功\n");
      //受け取りの結果をセット
      *(evt_wk->Ret) = TRUE;    //成功
      (*seq) = BEACON_SEQ_END;
    }
    break;
  case BEACON_SEQ_END:
    //ビーコン終了
    DELIVERY_BEACON_End( evt_wk->BeaconWork );
    //終了
    return( GMEVENT_RES_FINISH );
  }
  
  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * ランク計算
 * @param   ptr      TRIAL_HOUSE_WORK_PTR
 * @param   outRank   算出ランク格納バッファ
 * @param   outPoint  得点格納バッファ
 * @retval  none
 */
//--------------------------------------------------------------
void TRIAL_HOUSE_CalcBtlResult( TRIAL_HOUSE_WORK_PTR ptr, u16 *outRank, u16 *outPoint )
{
  int val;
  u16 rank;
  TH_POINT_WORK *point;
  point = &ptr->PointWork;
  val = 0;
  val += (point->WinTrainerNum * 1000);     //倒したトレーナー数 x 1000
  val += (point->WinPokeNum * 100);         //倒したポケモン数 x 100
  val += (point->PokeChgNum * 5);           //交代回数 x 5
  val += (point->WeakAtcNum * 1);           //ばつぐんの技を出した回数 x 1
  val += (point->VoidNum * 5);              //効果がない技を受けた回数 x 5
  val += (point->ResistNum * 2);            //いまひとつの技を受けた回数 x 2
  val += (point->UseWazaNum * 1);           //使用した技の数 x 1

  val -= (point->TurnNum * 10);              //かかったターン数 x 10
  val -= (point->VoidAtcNum * 10);          //効果がない技を出した回数 x 10
  val -= (point->ResistAtcNum * 2);         //いまひとつの技を出した回数 x 2
  val -= (point->LosePokeNum * 100);        //倒されたポケモン数 x 100
  val -= ( ((500 - point->RestHpPer) * 100) / 500 );     //残りＨＰ割合(最大-100)

  if (val < 0) val = 0;

  if (val >= 6000) rank = TH_RANK_MASTER;
  else if (val >= 5000) rank = TH_RANK_ELITE;
  else if (val >= 4000) rank = TH_RANK_HYPER;
  else if (val >= 3000) rank = TH_RANK_SUPER;
  else if (val >= 2000) rank = TH_RANK_NORMAL;
  else if (val >= 1000) rank = TH_RANK_NOVICE;
  else rank = TH_RANK_BEGINNER;

  *outRank = rank;
  *outPoint = val;
}

//--------------------------------------------------------------
/**
 * ダウンロードデータの種類を取得
 * @param   gsys      ゲームシステムポインタ
 * @param   ptr       トライアルハウスワークポインタ
 * @retval  u32       データタイプ
 */
//--------------------------------------------------------------
u32 TRIAL_HOUSE_GetDLDataType( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr )
{
  BOOL rc;
  u32 type;
  u32 monsno;
  BATTLE_EXAMINATION_SAVEDATA *exa;
  BSUBWAY_PARTNER_DATA *data;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata);
  exa = SaveControl_Extra_DataPtrGet( sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION, 0);
  //データの有効性をチェック
  rc = BATTLE_EXAMINATION_SAVE_IsInData(exa);
  if (!rc) type = TH_DL_DATA_TYPE_NONE;
  else
  {
    //一人目のトレーナーの手持ちポケモンの４番目を取得
    data = BATTLE_EXAMINATION_SAVE_GetData(exa, 0);
    //3匹目取得
    monsno = data->btpwd[2].mons_no;
    //3匹目いなければ、データ無し
    if ( (0<monsno)&&(monsno<=MONSNO_END) )
    {
      //4匹目取得
      monsno = data->btpwd[3].mons_no;
      //４匹目いなければシングル、いればダブル
      if ( (0<monsno)&&(monsno<=MONSNO_END) ) type = TH_DL_DATA_TYPE_DOUBLE;
      else type = TH_DL_DATA_TYPE_SINGLE;
    }  
    else type = TH_DL_DATA_TYPE_NONE;
    
  }

  NOZOMU_Printf("DL_data_type = %d\n",type);
  return type;
}

//--------------------------------------------------------------
/**
 * 検定データを無効状態にしてセーブする
 * @param   gsys      ゲームシステムポインタ
 * @param   ptr       トライアルハウスワークポインタ
 * @retval  none
 */
//--------------------------------------------------------------
void TRIAL_HOUSE_InvalidDLData( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr )
{
  BATTLE_EXAMINATION_SAVEDATA exa_data;
  BSUBWAY_PARTNER_DATA *data;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata);
  int size = BATTLE_EXAMINATION_SAVE_GetWorkSize();

  NOZOMU_Printf("検定データ　%dbyte をクリア\n",size);

  GFL_STD_MemClear( &exa_data, size );
  BATTLE_EXAMINATION_SAVE_Write(sv, &exa_data, ptr->HeapID);
}







