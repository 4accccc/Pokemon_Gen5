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

#include "savedata/trialhouse_save_local.h"
#include "savedata/trialhouse_save.h"

#include "field/th_rank_calc.h"   //for TH_CALC_Rank  inline

#include "field_saveanime.h"      //for   FIELD_SAVEANIME_

#define DOWNLOAD_BIT_MAX  (128)

#define BEACON_SEARCH_TIME  (4)

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

//セーブシーケンスワーク
typedef struct
{
  GAMESYS_WORK *gsys;
  void* pWork;
  u32 Type;
  FIELD_SAVEANIME* bgAnime;
}EXA_SAVE_WORK;

static void SetDLDataType( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr );
static void MakeTrainer(TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount);
static void SetDownLoadData( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount);
static u16 GetTrainerOBJCode( TRIAL_HOUSE_WORK_PTR ptr );

static GMEVENT_RESULT BeaconSearchEvt( GMEVENT *event, int *seq, void *wk );

static u8 GetDownloadBit( THSV_WORK *sv_wk, const u32 inBitNo );
static void SetDownloadBit( THSV_WORK *sv_wk, const u32 inBitNo );

static GMEVENT_RESULT SaveUsedKeyEvt( GMEVENT *event, int *seq, void *wk );

typedef enum
{
  BEACON_SEQ_START = 0,
  BEACON_SEQ_SEARCH,
  BEACON_SEQ_CHECK,
  BEACON_SEQ_MAIN,
  BEACON_SEQ_END,
  BEACON_SEQ_NET_END,
  BEACON_SEQ_SET_DL_TYPE,
}BEACON_SEQ;

//--------------------------------------------------------------
/**
 * @brief //トライアルハウスワーク確保関数
 * @param   gsys      ゲームシステムポインタ
 * @retval  ptr      TRIAL_HOUSE_WORK_PTR
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

  SetDLDataType( gsys, ptr );

  //パーティ作成
  ptr->Party = PokeParty_AllocPartyWork( ptr->HeapID );
  return ptr;
}

//--------------------------------------------------------------
/**
 * @brief 外部セーブに入っているダウンロードデータの種類をワークにセットする
 * @param   gsys      ゲームシステムポインタ
 * @retval  ptr      TRIAL_HOUSE_WORK_PTR
*/
//--------------------------------------------------------------
static void SetDLDataType( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr )
{
  BOOL rc;
  u32 type;
  u32 monsno;
  BATTLE_EXAMINATION_SAVEDATA *exa;
  BSUBWAY_PARTNER_DATA *data;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata);

  void *tmp_wk = GFL_HEAP_AllocClearMemory(GFL_HEAP_LOWID(HEAPID_PROC), SAVESIZE_EXTRA_BATTLE_EXAMINATION);
  
  //外部データをロード
  if ( LOAD_RESULT_OK == SaveControl_Extra_LoadWork(sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION, GFL_HEAP_LOWID(HEAPID_PROC),
                                                   tmp_wk, SAVESIZE_EXTRA_BATTLE_EXAMINATION) )
  {
    OS_Printf("トライアルハウス外部データロード\n");
  }

  exa = SaveControl_Extra_DataPtrGet( sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION, 0);
  //データの有効性をチェック
  rc = BATTLE_EXAMINATION_SAVE_IsInData(exa);
  if (!rc) type = TH_DL_DATA_TYPE_NONE;       //データないよ
  else                                        //データあるよ
  {
    if ( BATTLE_EXAMINATION_SAVE_IsSingleBattle(exa) ) type = TH_DL_DATA_TYPE_SINGLE;
    else type = TH_DL_DATA_TYPE_DOUBLE;
#if 0    
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
#endif    
  }

  NOZOMU_Printf("DL_data_type = %d\n",type);

  SaveControl_Extra_UnloadWork(sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION);
  GFL_HEAP_FreeMemory( tmp_wk );
  ptr->DLDataType = type;
}


//--------------------------------------------------------------
/**
 * @brief トライアルハウスワーク解放関数
 * @param   gsys  ゲームシステムポインタ
 * @param ptr      TRIAL_HOUSE_WORK_PTR
 * @retval    none
*/
//--------------------------------------------------------------
void TRIAL_HOUSE_End( GAMESYS_WORK * gsys, TRIAL_HOUSE_WORK_PTR *ptr )
{
  if ( *ptr != NULL )
  {
    GFL_HEAP_FreeMemory( (*ptr)->CommBuf );
    GFL_HEAP_FreeMemory( (*ptr)->Party );
    GFL_HEAP_FreeMemory( *ptr );
    *ptr = NULL;
  }
  else GF_ASSERT(0);
}

//--------------------------------------------------------------
/**
 * @brief トライアルハウス プレイモードのセット
 * @param ptr      TRIAL_HOUSE_WORK_PTR
 * @param inPlayMode     プレイモード　 TH_PLAYMODE_SINGLE or TH_PLAYMODE_DOUBLE
 * @note  trial_house_scr_def.h参照
 * @retval    none
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
 * @brief トライアルハウス プレイモードのセット
 * @param ptr      TRIAL_HOUSE_WORK_PTR
 * @param inDLFlg   ダウンロードデータで遊ぶか？　　TRUE:ダウンロードデータ　FALSE:ROMデータ
 * @retval    none
*/
//--------------------------------------------------------------
void TRIAL_HOUSE_SetDLFlg( TRIAL_HOUSE_WORK_PTR ptr, const BOOL inDLFlg )
{
  ptr->DownLoad = inDLFlg;
}

//--------------------------------------------------------------
/**
 * @brief 対戦相手の抽選
 * @param gsys      ゲームシステムポインタ
 * @param ptr      TRIAL_HOUSE_WORK_PTR
 * @param inBtlCouont   対戦回数　0～4（最大五戦）
 * @retval  obj_id      OBJ見た目     
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
 * @brief 対戦相手の抽選(ＲＯＭデータ)
 * @param ptr      TRIAL_HOUSE_WORK_PTR
 * @param inBtlCouont   対戦回数　0～4（最大五戦）
 * @retval  none  
*/
//--------------------------------------------------------------
static void MakeTrainer(TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount)
{
  int obj_id;
  int tr_no;
  int base, band;

  //※ここで抽選するトレーナーナンバーは０オリジン。この後アーカイブで+1される
  switch(inBtlCount){
  case 0:
    base = 50;
    band = 20;
    break;
  case 1:
    base = 110;
    band = 50;
    break;
  case 2:
    base = 160;
    band = 20;
    break;
  case 3:
    base = 240;
    band = 30;
    break;
  case 4:
    base = 270;
    band = 30;
    break;
  default:
    GF_ASSERT_MSG(0,"count error %d",inBtlCount);
    base = 50;
    band = 20;
  }
  //トレーナー抽選
  tr_no = base + GFUser_GetPublicRand(band);
  NOZOMU_Printf("ZERO_ORG entry_tr_no = %d\n",tr_no);

  //データをトライアルハウスワークにセット
  FBI_TOOL_MakeRomTrainerData(
      &ptr->TrData,
      tr_no, ptr->MemberNum,
      NULL,NULL,NULL, GFL_HEAP_LOWID(ptr->HeapID));
}

//--------------------------------------------------------------
/**
 * @brief 対戦相手のセット(ダウンロードデータ)
 * @param ptr      TRIAL_HOUSE_WORK_PTR
 * @param inBtlCouont   対戦回数　0～4（最大五戦）
 * @retval  none    
*/
//--------------------------------------------------------------
static void SetDownLoadData( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr, const int inBtlCount)
{
  BATTLE_EXAMINATION_SAVEDATA *exa;
  BSUBWAY_PARTNER_DATA *data;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata);

  void *tmp_wk = GFL_HEAP_AllocClearMemory(GFL_HEAP_LOWID(HEAPID_PROC), SAVESIZE_EXTRA_BATTLE_EXAMINATION);

  //外部データをロード
  if ( LOAD_RESULT_OK == SaveControl_Extra_LoadWork(sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION, GFL_HEAP_LOWID(HEAPID_PROC),
                                                   tmp_wk, SAVESIZE_EXTRA_BATTLE_EXAMINATION) )
  {
    OS_Printf("トライアルハウス外部データロード\n");
    exa = SaveControl_Extra_DataPtrGet( sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION, 0);
  
    //データ取得
    data = BATTLE_EXAMINATION_SAVE_GetData(exa, inBtlCount);
    //トライアルハウスワークにデータをセット
    ptr->TrData = *data;
  }
  else
  {
    GF_ASSERT(0);
  }
  SaveControl_Extra_UnloadWork(sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION);
  GFL_HEAP_FreeMemory( tmp_wk );
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
 * @brief 対戦相手の先頭前メッセージ表示
 * @param ptr      TRIAL_HOUSE_WORK_PTR
 * @retval    none
*/
//--------------------------------------------------------------
GMEVENT *TRIAL_HOUSE_CreateBeforeMsgEvt( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr, int inTrIdx, int inObjID )
{
  GMEVENT *event;

  event = FBI_TOOL_CreateTrainerBeforeMsgEvt(gsys, &ptr->TrData, inTrIdx, inObjID );

  return event;
}

//--------------------------------------------------------------
/**
 * @brief ビーコンサーチ
 * @param gsys        ゲームシステムポインタ
 * @param ptr         トライアルハウスワークポインタ
 * @param outRet      サーチ結果格納バッファ
 * @retval  event     イベントポインタ
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
    init->data[0].datasize = BATTLE_EXAMINATION_SAVE_GetWorkSize();  //データ全体サイズ
    init->data[0].pData = ptr->CommBuf;     // 受信バッファデータ
    init->data[0].LangCode  = CasetteLanguage;     // 受け取る言語コード
    init->data[0].version   = 1<<GET_VERSION();     // 受け取るバージョンのビット
    init->dataNum = 1;  //受け取り側は１
    init->ConfusionID = 0;   //混線しないためのID
    init->heapID = GFL_HEAP_LOWID(HEAPID_FIELDMAP);
  }

  return event;
}

//--------------------------------------------------------------
/**
 * ビーコンサーチ
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
        *(evt_wk->Ret) = TH_DL_RSLT_FAIL;   //失敗
        (*seq) = BEACON_SEQ_END;
      }
      else (*seq) = BEACON_SEQ_SEARCH;
    }
    break;
  case BEACON_SEQ_SEARCH:
    //ボタンキャンセル処理
    if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_CANCEL ){
      *(evt_wk->Ret) = TH_DL_RSLT_CANCEL;   //キャンセル
      (*seq) = BEACON_SEQ_END;
      break;
    }

    //配信サービス検出のため、4秒間位配送メインをまわす
    {
      //配送メイン
      DELIVERY_BEACON_Main( evt_wk->BeaconWork );
    }
    evt_wk->Time++;
    if ( evt_wk->Time > 30*BEACON_SEARCH_TIME ) (*seq) = BEACON_SEQ_CHECK;
    break;
  case BEACON_SEQ_CHECK:
    //ボタンキャンセル処理
    if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_CANCEL ){
      *(evt_wk->Ret) = TH_DL_RSLT_CANCEL;   //キャンセル
      (*seq) = BEACON_SEQ_END;
      break;
    }

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
        *(evt_wk->Ret) = TH_DL_RSLT_FAIL;   //失敗
        (*seq) = BEACON_SEQ_END;
      }
    }
    break;
  case BEACON_SEQ_MAIN:
    //ボタンキャンセル処理
    if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_CANCEL ){
      *(evt_wk->Ret) = TH_DL_RSLT_CANCEL;   //キャンセル
      (*seq) = BEACON_SEQ_END;
      break;
    }

    //配送メイン
    DELIVERY_BEACON_Main( evt_wk->BeaconWork );
    //データ受け取り待ち
    if ( DELIVERY_BEACON_RecvCheck( evt_wk->BeaconWork )  )
    {
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
      THSV_WORK *sv_wk = THSV_GetSvPtr( GAMEDATA_GetSaveControlWork(gamedata) );
      BATTLE_EXAMINATION_SAVEDATA* pBE = evt_wk->Ptr->CommBuf;
      int no = BATTLE_EXAMINATION_SAVE_GetDataNo(pBE);
      BOOL bActive = BATTLE_EXAMINATION_SAVE_IsInData(pBE);
      BOOL bGet = GetDownloadBit(sv_wk, no);
      if(bActive && (!bGet || (no == 0))){
        NOZOMU_Printf("外部セーブにデータを保存\n");
        SetDownloadBit(sv_wk, no);
        BATTLE_EXAMINATION_SAVE_Write(gamedata, evt_wk->Ptr->CommBuf, GFL_HEAP_LOWID(HEAPID_PROC));
        NOZOMU_Printf("データ受け取り成功\n");
        //受け取りの結果をセット
        *(evt_wk->Ret) = TH_DL_RSLT_SUCCESS;    //成功
      }
      else if(bGet){
        OHNO_Printf("うけ取りずみ %d %d %d\n", no,bActive,bGet);
        *(evt_wk->Ret) = TH_DL_RSLT_SETTLED;
      }
      else{
        OHNO_Printf("しっぱい %d %d %d\n", no,bActive,bGet);
        *(evt_wk->Ret) = TH_DL_RSLT_FAIL;
      }
      (*seq) = BEACON_SEQ_END;
    }
    break;
  case BEACON_SEQ_END:
    //ビーコン終了
    DELIVERY_BEACON_End( evt_wk->BeaconWork );
    (*seq) = BEACON_SEQ_NET_END;
    break;
  case BEACON_SEQ_NET_END:
    //通信完全終了待ち
    if ( GFL_NET_IsResetEnable() )
    {
      (*seq) = BEACON_SEQ_SET_DL_TYPE;
    }
    break;
  case BEACON_SEQ_SET_DL_TYPE:
    SetDLDataType( gsys, evt_wk->Ptr );
    //終了
    return( GMEVENT_RES_FINISH );
  }
  
  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * ランク計算
 * @param   gsys    ゲームシステムポインタ
 * @param   ptr      TRIAL_HOUSE_WORK_PTR
 * @param   outRank   算出ランク格納バッファ
 * @param   outPoint  得点格納バッファ
 * @retval  none
 */
//--------------------------------------------------------------
void TRIAL_HOUSE_CalcBtlResult( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr, u16 *outRank, u16 *outPoint )
{
  SAVE_CONTROL_WORK *sv;
  int val;
  u16 rank;
  TH_POINT_WORK *point;
  point = &ptr->PointWork;
  val = 0;
  val += (point->WinTrainerNum * 1000);     //倒したトレーナー数 x 1000
  val += (point->WinPokeNum * 80);         //倒したポケモン数 x 80
  val += (point->PokeChgNum * 5);           //交代回数 x 5
  val += (point->WeakAtcNum * 1);           //ばつぐんの技を出した回数 x 1
  val += (point->VoidNum * 5);              //効果がない技を受けた回数 x 5
  val += (point->ResistNum * 2);            //いまひとつの技を受けた回数 x 2
  val += (point->UseWazaNum * 15);           //使用した技の数 x 15

  val -= (point->TurnNum * 10);              //かかったターン数 x 10
  val -= (point->VoidAtcNum * 10);          //効果がない技を出した回数 x 10
  val -= (point->ResistAtcNum * 2);         //いまひとつの技を出した回数 x 2
  val -= (point->LosePokeNum * 80);        //倒されたポケモン数 x 80
  val -= ( ((500 - point->RestHpPer) * 100) / 500 );     //残りＨＰ割合(最大-100)

  if (val < 0) val = 0;
  else if (val > TH_SCORE_MAX) val = TH_SCORE_MAX;

  rank = TH_CALC_Rank( val );

  *outRank = rank;
  *outPoint = val;

  //ランキングセーブデータに書き込み
  {
    int i;
    TH_SV_COMMON_WK *cm_dat;
    THSV_WORK *sv_wk;
    u8 idx;
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    sv = GAMEDATA_GetSaveControlWork(gamedata);
    sv_wk = THSV_GetSvPtr( sv );

    if ( ptr->DLDataType == TH_DL_DATA_TYPE_NONE ) idx = 0; //ＲＯＭデータ
    else idx = 1;
    
    cm_dat = &sv_wk->CommonData[idx];
    cm_dat->Valid = 1;
    
    if ( ptr->MemberNum == 3 ) cm_dat->IsDouble = 0;
    else if ( ptr->MemberNum == 4 ) cm_dat->IsDouble = 1;
    else
    {
      GF_ASSERT_MSG(0,"member error %d", ptr->MemberNum);
      cm_dat->IsDouble = 0;
    }
    
    cm_dat->Point = val;
    //モンスターデータクリア
    MI_CpuClear8( cm_dat->MonsData, sizeof(TH_MONS_DATA)*TH_MONS_DATA_MAX );
    for(i=0;i<ptr->MemberNum;i++)
    {
      TH_MONS_DATA *mons_dat = &cm_dat->MonsData[i];
      POKEMON_PARAM *pp = PokeParty_GetMemberPointer( ptr->Party, i );
      mons_dat->MonsNo = PP_Get(pp, ID_PARA_monsno, NULL);
      mons_dat->FormNo = PP_Get(pp, ID_PARA_form_no, NULL);
      mons_dat->Sex = PP_Get(pp, ID_PARA_sex, NULL);;
    }

    //ダウンロードデータの場合はタイトルを取得
    if ( ptr->DLDataType != TH_DL_DATA_TYPE_NONE )
    {
      BATTLE_EXAMINATION_SAVEDATA *exa;
      GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
      SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata);
      void *tmp_wk = GFL_HEAP_AllocClearMemory(GFL_HEAP_LOWID(HEAPID_PROC), SAVESIZE_EXTRA_BATTLE_EXAMINATION);
      //外部データをロード
      if ( LOAD_RESULT_OK == SaveControl_Extra_LoadWork(sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION, GFL_HEAP_LOWID(HEAPID_PROC),
                                                   tmp_wk, SAVESIZE_EXTRA_BATTLE_EXAMINATION) )
      {
        OS_Printf("トライアルハウス外部データロード\n");
        exa = SaveControl_Extra_DataPtrGet( sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION, 0);
        //タイトル取得
        GFL_STD_MemCopy( exa->titleName, sv_wk->Name,
            BATTLE_EXAMINATION_TITLE_MSG_MAX * sizeof( STRCODE ) );
      }
      else GF_ASSERT(0);

      GFL_HEAP_FreeMemory( tmp_wk );
      SaveControl_Extra_UnloadWork(sv, SAVE_EXTRA_ID_BATTLE_EXAMINATION);
    }
  }

  //最高得点、最高ランク更新処理(シングル、ダブルの区別無し) 
  {
    RECORD * rec;
    rec = SaveData_GetRecord(sv);
    //内部で上書きチェックしているので、条件無しで処理してＯＫ
    RECORD_SetThScore(rec, val);
    RECORD_SetThRank(rec, rank);
  }
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
  return ptr->DLDataType;
}

//--------------------------------------------------------------
/**
 * 検定データを無効状態にしてセーブする
 * @param   gsys      ゲームシステムポインタ
 * @param   ptr       トライアルハウスワークポインタ
 * @param   clear_type    TH_DL_DATA_CLEAR_TYPE_CLEAR or TH_DL_DATA_CLEAR_TYPE_USED
 * @retval  none
 */
//--------------------------------------------------------------
GMEVENT *TRIAL_HOUSE_InvalidDLData( GAMESYS_WORK *gsys, TRIAL_HOUSE_WORK_PTR ptr, const u32 inClearType )
{
  GMEVENT *event;
  EXA_SAVE_WORK *evt_wk;
  event = GMEVENT_Create( gsys, NULL,
      SaveUsedKeyEvt, sizeof(EXA_SAVE_WORK) );
  evt_wk = GMEVENT_GetEventWork( event );
  evt_wk->gsys = gsys;
  evt_wk->pWork = NULL;
  evt_wk->Type = inClearType;

  {
    FIELDMAP_WORK *fieldWork;
    fieldWork = GAMESYSTEM_GetFieldMapWork(gsys);
    evt_wk->bgAnime = FIELD_SAVEANIME_Create( GFL_HEAP_LOWID(HEAPID_FIELDMAP), fieldWork );
    FIELD_SAVEANIME_Start( evt_wk->bgAnime );
  }

  return event;
  
/**  
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );

  if ( inClearType == TH_DL_DATA_CLEAR_TYPE_USED )
  {
    BATTLE_EXAMINATION_SAVEDATA *exa;
    SAVE_CONTROL_WORK *pSave = GAMEDATA_GetSaveControlWork(gamedata);
    void* pWork = GFL_HEAP_AllocMemory(GFL_HEAP_LOWID(HEAPID_PROC),SAVESIZE_EXTRA_BATTLE_EXAMINATION);
    SaveControl_Extra_LoadWork(pSave, SAVE_EXTRA_ID_BATTLE_EXAMINATION, GFL_HEAP_LOWID(HEAPID_PROC),
                             pWork, SAVESIZE_EXTRA_BATTLE_EXAMINATION);
    exa = SaveControl_Extra_DataPtrGet( pSave, SAVE_EXTRA_ID_BATTLE_EXAMINATION, 0);
    BATTLE_EXAMINATION_SAVE_SetDataUsedKey(exa);
    GAMEDATA_ExtraSaveAsyncStart(gamedata,SAVE_EXTRA_ID_BATTLE_EXAMINATION);
    while(1){
      if(SAVE_RESULT_OK==GAMEDATA_ExtraSaveAsyncMain(gamedata,SAVE_EXTRA_ID_BATTLE_EXAMINATION)){
        break;
      }
      OS_WaitIrq(TRUE, OS_IE_V_BLANK);
    }
    SaveControl_Extra_UnloadWork(pSave, SAVE_EXTRA_ID_BATTLE_EXAMINATION);
    GFL_HEAP_FreeMemory(pWork);
    NOZOMU_Printf("検定データ使用済みキーセット\n");
  }
  else
  {
    BATTLE_EXAMINATION_SAVEDATA exa_data;
    int size = BATTLE_EXAMINATION_SAVE_GetWorkSize();
    NOZOMU_Printf("検定データ　%dbyte をクリア\n",size);
    GFL_STD_MemClear( &exa_data, size );
    BATTLE_EXAMINATION_SAVE_Write(gamedata, &exa_data, GFL_HEAP_LOWID(HEAPID_PROC));
  }
*/

}

//--------------------------------------------------------------
/**
 * 検定データを無効状態にしてセーブする
 * @param   gsys      ゲームシステムポインタ
 * @retval  データステート trial_house_scr_def.h参照
 * @note 戻り値はTH_RDAT_ST_～
 */
//--------------------------------------------------------------
u32 TRIAL_HOUSE_GetRankDataState( GAMESYS_WORK *gsys )
{
  u32 state;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gamedata);
  THSV_WORK *sv_wk = THSV_GetSvPtr( sv );

  if ( sv_wk->CommonData[0].Valid )
  {
    if ( sv_wk->CommonData[1].Valid ) state = TH_RDAT_ST_BOTH;    //両方データある
    else  state = TH_RDAT_ST_ROM;   //ＲＯＭデータのみ
  }
  else
  {
    if ( sv_wk->CommonData[1].Valid ) state = TH_RDAT_ST_DL;  //ダウンロードデータのみ
    else state = TH_RDAT_ST_NONE; //データなし
  }

  return state;
}

//--------------------------------------------------------------
/**
 * ダウンロードビット取得
 * @param   sv_wk トライアルハウスセーブデータポインタ
 * @param   inBitNo   ビットナンバー 0～127
 * @retval  bit  0 or 1
 */
//--------------------------------------------------------------
static u8 GetDownloadBit( THSV_WORK *sv_wk, const u32 inBitNo )
{
  if (inBitNo < DOWNLOAD_BIT_MAX)
  {
    u8 bit;
    u8 idx;
    u8 data;
    u8 shift;
    idx = inBitNo / 8;
    shift = inBitNo % 8;
    data = sv_wk->DownloadBits[idx];
    bit = (data >> shift) & 0x1;    //１ビットマスク

    OS_Printf("BIT GET ( BitNo=%d, idx=%d, shift=%d )\n",inBitNo, idx, shift );
    return bit;
  }
  else
  {
    GF_ASSERT_MSG(0,"error %d",inBitNo);
    return 1;   //立っていることにする
  }
}

//--------------------------------------------------------------
/**
 * ダウンロードビットセット
 * @param   sv_wk トライアルハウスセーブデータポインタ
 * @param   inBitNo   ビットナンバー 0～127
 * @retval  none
 */
//--------------------------------------------------------------
static void SetDownloadBit( THSV_WORK *sv_wk, const u32 inBitNo )
{
  if (inBitNo < DOWNLOAD_BIT_MAX)
  {
    u8 idx;
    u8 data;
    u8 shift;
    idx = inBitNo / 8;
    shift = inBitNo % 8;
    data = sv_wk->DownloadBits[idx];
    data |= (1<<shift);
    sv_wk->DownloadBits[idx] = data;
    OS_Printf("BIT SET ( BitNo=%d, idx=%d, shift=%d )\n",inBitNo, idx, shift );
  }
  else
  {
    GF_ASSERT_MSG(0,"error %d",inBitNo);
  }
}

//--------------------------------------------------------------
/**
 * セーブシーケンスイベント
 * @param event GMEVENT
 * @param seq event sequence
 * @param wk event work
 * @retval GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT SaveUsedKeyEvt( GMEVENT *event, int *seq, void *wk )
{
  EXA_SAVE_WORK *evt_wk = wk;
  GAMESYS_WORK *gsys = evt_wk->gsys;
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *pSave = GAMEDATA_GetSaveControlWork(gamedata);
  
  switch(*seq){
  case 0:
    {
      BATTLE_EXAMINATION_SAVEDATA *exa;
      evt_wk->pWork = GFL_HEAP_AllocMemory(GFL_HEAP_LOWID(HEAPID_PROC),SAVESIZE_EXTRA_BATTLE_EXAMINATION);
      SaveControl_Extra_LoadWork( pSave, SAVE_EXTRA_ID_BATTLE_EXAMINATION, GFL_HEAP_LOWID(HEAPID_PROC),
                                  evt_wk->pWork, SAVESIZE_EXTRA_BATTLE_EXAMINATION  );
      exa = SaveControl_Extra_DataPtrGet( pSave, SAVE_EXTRA_ID_BATTLE_EXAMINATION, 0);
      
      if ( evt_wk->Type == TH_DL_DATA_CLEAR_TYPE_USED )
      {
        BATTLE_EXAMINATION_SAVE_SetDataUsedKey(exa);
        NOZOMU_Printf("検定データ使用済みキーセット\n");
      }
      else
      {
        int size = BATTLE_EXAMINATION_SAVE_GetWorkSize();
        NOZOMU_Printf("検定データ　%dbyte をクリア\n",size);
        GFL_STD_MemClear( exa, size );
      }

      GAMEDATA_ExtraSaveAsyncStart(gamedata,SAVE_EXTRA_ID_BATTLE_EXAMINATION);
    }
    (*seq)++;
    break;
  case 1:
    if( SAVE_RESULT_OK == GAMEDATA_ExtraSaveAsyncMain(gamedata, SAVE_EXTRA_ID_BATTLE_EXAMINATION) )
    {
      SaveControl_Extra_UnloadWork(pSave, SAVE_EXTRA_ID_BATTLE_EXAMINATION);
      GFL_HEAP_FreeMemory(evt_wk->pWork);
      FIELD_SAVEANIME_End( evt_wk->bgAnime );
      FIELD_SAVEANIME_Delete( evt_wk->bgAnime );
      //終了
      return( GMEVENT_RES_FINISH );
    }
  }
  return( GMEVENT_RES_CONTINUE );
}












