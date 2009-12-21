/**
 *  @file trcard_sys.c
 *  @brief  トレーナーカードシステム
 *  @author Miyuki Iwasawa
 *  @date 08.01.16
 */

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "gamesystem/game_data.h"
#include "savedata/config.h"
#include "savedata/trainercard_data.h"
#include "savedata/mystatus.h"
#include "savedata/gametime.h"
#include "system/pms_data.h"
#include "app/pms_select.h"
#include "savedata/record.h"
#include "savedata/bsubway_savedata.h"
#include "savedata/zukan_savedata.h"
#include "field/eventwork.h"
#include "field/eventwork_def.h"


#include "app/mysign.h"
#include "app/trainer_card.h"
#include "trcard_sys.h"

#include "test/ariizumi/ari_debug.h"


typedef struct _TR_CARD_SYS{
  int heapId;

  void              *app_wk;    ///<簡易会話モジュールワークの保存
  GFL_PROCSYS       *procSys;   ///<サブプロセスワーク
  PMS_SELECT_PARAM  PmsParam;   ///<簡易会話アプリ呼び出し用ワーク

  TRCARD_CALL_PARAM *tcp;

}TR_CARD_SYS;

//プロトタイプ宣言　ローカル
//================================================================
///オーバーレイプロセス
GFL_PROC_RESULT TrCardSysProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
GFL_PROC_RESULT TrCardSysProc_InitComm( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
GFL_PROC_RESULT TrCardSysProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
GFL_PROC_RESULT TrCardSysProc_End( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
GFL_PROC_RESULT TrCardSysProc_EndComm( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
const GFL_PROC_DATA TrCardSysProcData = {
  TrCardSysProc_Init,
  TrCardSysProc_Main,
  TrCardSysProc_End,
};
//初期化・開放が違う通信用Procデータ
//トレーナーカードデータを外部から受け取るので開放しない
const GFL_PROC_DATA TrCardSysCommProcData = {
  TrCardSysProc_InitComm,
  TrCardSysProc_Main,
  TrCardSysProc_EndComm,
};

//================================================================
///データ定義エリア
//================================================================
///オーバーレイプロセス定義データ
enum{
  CARD_INIT,
  CARD_WAIT,
  SIGN_INIT,
  SIGN_WAIT,
  CARDSYS_END,
};

static int sub_CardInit(TR_CARD_SYS* wk);
static int sub_CardWait(TR_CARD_SYS* wk);
static int sub_SignInit(TR_CARD_SYS* wk);
static int sub_SignWait(TR_CARD_SYS* wk);

//------------------------------------------------------------------
/**
 * @brief サブプロセス呼び出し処理
 * @param proc  サブプロセスポインタを保持するワークへのポインタ
 */
//------------------------------------------------------------------
static BOOL TrCardSysProcCall(GFL_PROCSYS ** procSys)
{
  if (*procSys) {
    if (GFL_PROC_LOCAL_Main(*procSys) == GFL_PROC_MAIN_NULL ) {
      GFL_PROC_LOCAL_Exit(*procSys);
      *procSys = NULL;
      return TRUE;
    }
  }
  return FALSE;
}


//=============================================================================================
/**
 * @brief  トレーナーカードシステム呼び出し　初期化
 *
 * @param   proc    
 * @param   seq   
 * @param   pwk   
 * @param   mywk    
 *
 * @retval  GFL_PROC_RESULT   
 */
//=============================================================================================
GFL_PROC_RESULT TrCardSysProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  TR_CARD_SYS* wk = NULL;
  TRCARD_CALL_PARAM* pp = (TRCARD_CALL_PARAM*)pwk;
  
  //ヒープ作成
  GFL_HEAP_CreateHeap(GFL_HEAPID_APP,HEAPID_TRCARD_SYS,0x18000);
  wk = GFL_PROC_AllocWork(proc,sizeof(TR_CARD_SYS),HEAPID_TRCARD_SYS);
  MI_CpuClear8(wk,sizeof(TR_CARD_SYS));

  //ヒープID保存
  wk->heapId = HEAPID_TRCARD_SYS;

  //データテンポラリ作成
  wk->tcp = GFL_HEAP_AllocClearMemory( wk->heapId , sizeof( TRCARD_CALL_PARAM ) );
  *wk->tcp = *pp;
  wk->tcp->TrCardData = GFL_HEAP_AllocClearMemory( wk->heapId , sizeof( TR_CARD_DATA ) );
  TRAINERCARD_GetSelfData( wk->tcp->TrCardData , pp->gameData , FALSE);

  return GFL_PROC_RES_FINISH;
}

GFL_PROC_RESULT TrCardSysProc_InitComm( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  TR_CARD_SYS* wk = NULL;
  TRCARD_CALL_PARAM* callParam = (TRCARD_CALL_PARAM*)pwk;
  
  //ヒープ作成
  GFL_HEAP_CreateHeap(GFL_HEAPID_APP,HEAPID_TRCARD_SYS,0x18000);
  wk = GFL_PROC_AllocWork(proc,sizeof(TR_CARD_SYS),HEAPID_TRCARD_SYS);
  MI_CpuClear8(wk,sizeof(TR_CARD_SYS));

  //ヒープID保存
  wk->heapId = HEAPID_TRCARD_SYS;

  //データテンポラリ作成
  wk->tcp = callParam;

  return GFL_PROC_RES_FINISH;
}


//=============================================================================================
/**
 * @brief  トレーナーカード　システムメイン
 *
 * @param   proc    
 * @param   seq   
 * @param   pwk   
 * @param   mywk    
 *
 * @retval  GFL_PROC_RESULT   
 */
//=============================================================================================
GFL_PROC_RESULT TrCardSysProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  TR_CARD_SYS* wk = (TR_CARD_SYS*)mywk;
  switch(*seq){
  case CARD_INIT:
    *seq = sub_CardInit(wk);
    break;
  case CARD_WAIT:
    *seq = sub_CardWait(wk);
    break;
  case SIGN_INIT:
    *seq = sub_SignInit(wk);
    break;
  case SIGN_WAIT:
    *seq = sub_SignWait(wk);
    break;
  case CARDSYS_END:
  default:
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}


//=============================================================================================
/**
 * @brief トレーナーカード　システム終了
 *
 * @param   proc    
 * @param   seq   
 * @param   pwk   
 * @param   mywk    
 *
 * @retval  GFL_PROC_RESULT   
 */
//=============================================================================================
GFL_PROC_RESULT TrCardSysProc_End( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  TR_CARD_SYS* wk = (TR_CARD_SYS*)mywk;

  GFL_HEAP_FreeMemory( wk->tcp->TrCardData );
  GFL_HEAP_FreeMemory( wk->tcp );

  //ワークエリア解放
  GFL_PROC_FreeWork(proc);
  
  GFL_HEAP_DeleteHeap(HEAPID_TRCARD_SYS);
  return GFL_PROC_RES_FINISH;
}


//=============================================================================================
/**
 * @brief 通信トレーナーカード閲覧終了時
 *
 * @param   proc    
 * @param   seq   
 * @param   pwk   
 * @param   mywk    
 *
 * @retval  GFL_PROC_RESULT   
 */
//=============================================================================================
GFL_PROC_RESULT TrCardSysProc_EndComm( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  TR_CARD_SYS* wk = (TR_CARD_SYS*)mywk;

  GFL_HEAP_FreeMemory( wk->tcp );

  //ワークエリア解放
  GFL_PROC_FreeWork(proc);
  
  GFL_HEAP_DeleteHeap(HEAPID_TRCARD_SYS);
  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------------
/**
 * @brief トレーナーカード呼び出し
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int sub_CardInit(TR_CARD_SYS* wk)
{
  // オーバーレイID宣言
//  FS_EXTERN_OVERLAY(trainer_card);

  // プロセス定義データ
  static const GFL_PROC_DATA TrCardProcData = {
    TrCardProc_Init,
    TrCardProc_Main,
    TrCardProc_End,
  };
//  wk->proc = PROC_Create(&TrCardProcData,wk->tcp,wk->heapId);
  if( wk->procSys == NULL )
  { 
    wk->procSys = GFL_PROC_LOCAL_boot( wk->heapId );
  }
  GFL_PROC_LOCAL_CallProc( wk->procSys, NO_OVERLAY_ID, &TrCardProcData,(void*)wk->tcp);
  return CARD_WAIT;
}


//----------------------------------------------------------------------------------
/**
 * @brief トレーナーカード終了待ち
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int sub_CardWait(TR_CARD_SYS* wk)
{
  if(!TrCardSysProcCall(&wk->procSys)){
    return CARD_WAIT;
  }
  
  if(wk->tcp->value){
    return SIGN_INIT;
  }
  return CARDSYS_END;
}

FS_EXTERN_OVERLAY(pmsinput);
//----------------------------------------------------------------------------------
/**
 * @brief 簡易会話呼び出し
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int sub_SignInit(TR_CARD_SYS* wk)
{
//  FS_EXTERN_OVERLAY(mysign);
  // プロセス定義データ
  static const GFL_PROC_DATA MySignProcData = {
    MySignProc_Init,
    MySignProc_Main,
    MySignProc_End,
  };
    
//  wk->proc = PROC_Create(&MySignProcData,(void*)wk->tcp->savedata,wk->heapId);
  if( wk->procSys == NULL )
  { 
    wk->procSys = GFL_PROC_LOCAL_boot( wk->heapId );
  }
//  GFL_PROC_LOCAL_CallProc( wk->procSys, NO_OVERLAY_ID, &MySignProcData,wk->tcp);
  wk->PmsParam.save_ctrl = GAMEDATA_GetSaveControlWork( wk->tcp->gameData );
  GFL_PROC_LOCAL_CallProc( wk->procSys, FS_OVERLAY_ID(pmsinput), &ProcData_PMSSelect, &wk->PmsParam );
  return SIGN_WAIT;
}

//----------------------------------------------------------------------------------
/**
 * @brief 簡易会話終了待ち
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int sub_SignWait(TR_CARD_SYS* wk)
{
  if(!TrCardSysProcCall(&wk->procSys)){
    return SIGN_WAIT;
  }
  
  // 作成した簡易会話があれば書き換え
  if(wk->PmsParam.out_pms_data!=NULL){
    wk->tcp->TrCardData->Pms = *wk->PmsParam.out_pms_data;
  }
/*
  //サインデータを呼び出しテンポラリに書き戻し
  {
    TR_CARD_SV_PTR trc_ptr = TRCSave_GetSaveDataPtr(SaveControl_GetPointer());
    //サインデータの有効/無効フラグを取得(金銀ローカルでのみ有効)
    wk->tcp->TrCardData->MySignValid = TRCSave_GetSigned(trc_ptr);
    //サインデータをセーブデータからコピー
    MI_CpuCopy8(TRCSave_GetSignDataPtr(trc_ptr),
        wk->tcp->TrCardData->SignRawData, SIGN_SIZE_X*SIGN_SIZE_Y*8 );
  }
*/
  return CARD_INIT;
}

//=============================================================================================
/**
 * @brief 自分のトレーナーカードデータを収集する
 *
 * @param   cardData    
 * @param   gameData    
 * @param   isSendData    
 */
//=============================================================================================
void TRAINERCARD_GetSelfData( TR_CARD_DATA *cardData , GAMEDATA *gameData , const BOOL isSendData )
{
  u8 i,flag;
  TR_CARD_SV_PTR trc_ptr = TRCSave_GetSaveDataPtr(GAMEDATA_GetSaveControlWork(gameData));
  MYSTATUS *mystatus = GAMEDATA_GetMyStatus( gameData );
  GMTIME *gameTime = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork(gameData) );
  RECORD *rec = SaveData_GetRecord( GAMEDATA_GetSaveControlWork(gameData) );

  //名前
  if( MyStatus_CheckNameClear( mystatus ) == FALSE )
  {
    const STRCODE *baseName = MyStatus_GetMyName( mystatus );
    GFL_STD_MemCopy16( baseName , cardData->TrainerName , sizeof(STRCODE)*(PERSON_NAME_SIZE+EOM_SIZE) );
  }
  else
  {
    //TODO 念のため名前が入ってないときに落ちないようにしておく
    cardData->TrainerName[0] = L'N';
    cardData->TrainerName[1] = L'o';
    cardData->TrainerName[2] = L'N';
    cardData->TrainerName[3] = L'a';
    cardData->TrainerName[4] = L'm';
    cardData->TrainerName[5] = L'e';
    cardData->TrainerName[6] = GFL_STR_GetEOMCode();
  }
  
  // カードランク取得
  cardData->CardRank = TRAINERCARD_GetCardRank( gameData );

  cardData->TrSex = MyStatus_GetMySex( mystatus );
  cardData->TrainerID = MyStatus_GetID( mystatus );
  cardData->Money = MyStatus_GetGold( mystatus );
  cardData->BadgeFlag = 0;
  cardData->Version   = PM_VERSION;

  //FIXME 正しいバッヂ個数の定義に(8個)
  flag = 1;
  for( i=0; i<8; i++ )
  {
    if( MyStatus_GetBadgeFlag( mystatus , i ) == TRUE )
    {
      cardData->BadgeFlag &= flag;
    }
    flag <<= 1;
  }
  //通信用かで分岐
  if( isSendData == TRUE )
  {
    PLAYTIME *playTime = SaveData_GetPlayTime( SaveControl_GetPointer() );
    cardData->UnionTrNo = MyStatus_GetTrainerView( mystatus );
    cardData->TimeUpdate = FALSE;
    cardData->PlayTime = NULL;
    cardData->PlayTime_h = PLAYTIME_GetHour( playTime );
    cardData->PlayTime_m = PLAYTIME_GetMinute( playTime );
  }
  else
  {
    cardData->UnionTrNo = UNION_TR_NONE;
    cardData->TimeUpdate = TRUE;
    cardData->PlayTime = SaveData_GetPlayTime( SaveControl_GetPointer() );
  }
  //クリア日時とプレイ開始日時
  {
    RTCDate workDate;
    RTCTime workTime;
    
    RTC_ConvertSecondToDateTime( &workDate , &workTime , gameTime->start_sec );
    cardData->Start_y = workDate.year;
    cardData->Start_m = workDate.month;
    cardData->Start_d = workDate.day;

    if( gameTime->clear_sec == 0 )
    {
      //未クリア
      cardData->Clear_y = 0;
      cardData->Clear_m = 0;
      cardData->Clear_d = 0;
      cardData->ClearTime_h = 0;
      cardData->ClearTime_m = 0;
    }
    else
    {
      RTC_ConvertSecondToDateTime( &workDate , &workTime , gameTime->clear_sec );
      cardData->Clear_y = workDate.year;
      cardData->Clear_m = workDate.month;
      cardData->Clear_d = workDate.day;
      cardData->ClearTime_h = workTime.hour;
      cardData->ClearTime_m = workTime.minute;
    }
  }
  //通信回数  ワイヤレスコンテスト+ワイヤレス交換+WiFi交換+ワイヤレス対戦+WiFi対戦+ワイヤレスポルト
  cardData->CommNum = RECORD_Get(rec, RECID_CONTEST_COMM_ENTRY)+
            RECORD_Get(rec, RECID_COMM_TRADE)+RECORD_Get(rec, RECID_WIFI_TRADE)+
            RECORD_Get(rec, RECID_COMM_BATTLE)+RECORD_Get(rec, RECID_WIFI_BATTLE)+
            RECORD_Get(rec, RECID_PORUTO_COMM);
  // 通信対戦回数
  cardData->CommBattleNum = RECORD_Get(rec, RECID_BATTLE_COUNT);
  //勝ち数  ワイヤレス+WiFi
  cardData->CommBattleWin = RECORD_Get(rec, RECID_COMM_BTL_WIN)+RECORD_Get(rec, RECID_WIFI_BTL_WIN);
  //負け数  ワイヤレス+WiFi
  cardData->CommBattleLose = RECORD_Get(rec, RECID_COMM_BTL_LOSE)+RECORD_Get(rec, RECID_WIFI_BTL_LOSE);
  //通信交換  ワイヤレス+WiFi
  cardData->CommTrade = RECORD_Get(rec, RECID_COMM_TRADE)+RECORD_Get(rec, RECID_WIFI_TRADE);
  
  //スコア
  cardData->Score = RECORD_Score_Get( rec );
  
  //FIXME 図鑑処理
  cardData->PokeBookFlg = TRUE;

  //サインデータの取得
  //サインデータの有効/無効フラグを取得(金銀ローカルでのみ有効)
  cardData->MySignValid = TRCSave_GetSigned(trc_ptr);
  //サインデータをセーブデータからコピー
  MI_CpuCopy8(TRCSave_GetSignDataPtr(trc_ptr),
      cardData->SignRawData, SIGN_SIZE_X*SIGN_SIZE_Y*8 );
}

//=============================================================================================
/**
 * @brief CALL_PARAMの作成
 *
 * @param   gameData    
 * @param   heapId    
 *
 * @retval  TRCARD_CALL_PARAM*    
 */
//=============================================================================================
TRCARD_CALL_PARAM* TRAINERCASR_CreateCallParam_SelfData( GAMEDATA *gameData , HEAPID heapId )
{
  TRCARD_CALL_PARAM* callParam;
  callParam = GFL_HEAP_AllocMemory( heapId , sizeof( TRCARD_CALL_PARAM ) );
  GFL_STD_MemClear( callParam, sizeof(TRCARD_CALL_PARAM) );
  callParam->TrCardData = NULL;
  callParam->gameData = gameData;
  callParam->value = 0;
  
  return callParam;
}

//=============================================================================================
/**
 * @brief 通信時のCALL_PARAMの作成
 *
 * @param   gameData    
 * @param   pCardData   
 * @param   heapId    
 *
 * @retval  TRCARD_CALL_PARAM*    
 */
//=============================================================================================
TRCARD_CALL_PARAM*  TRAINERCASR_CreateCallParam_CommData( GAMEDATA *gameData , void* pCardData , HEAPID heapId )
{
  TRCARD_CALL_PARAM* callParam;
  
  callParam = GFL_HEAP_AllocMemory( heapId , sizeof( TRCARD_CALL_PARAM ) );
  callParam->TrCardData = pCardData;
  callParam->gameData = gameData;
  callParam->value = 0;

  return callParam;
}


// バトルサブウェイでランクアップになる条件(シングル49連勝）
#define BSUBWAY_BADGE_TERMS ( 49 )

//=============================================================================================
/**
 * @brief トレーナーカードのランクを取得する
 *
 * @param   gameData    
 *
 * @retval  int   
 */
//=============================================================================================
int TRAINERCARD_GetCardRank( GAMEDATA *gameData )
{
  int rank=0;
  //殿堂入り（クリアフラグ）
  if (EVENTWORK_CheckEventFlag(GAMEDATA_GetEventWork( gameData ),SYS_FLAG_GAME_CLEAR)){
    rank++;
  }
  //全国図鑑完成（イベント系ポケモンを除くポケモンをゲットしているか）
  if ( ZUKANSAVE_CheckZenkokuComp(GAMEDATA_GetZukanSave(gameData)) ){
       
    rank++;
  }

#if 0
  // バトルサブウェイで49連勝
  bs_save = SaveControl_DataPtrGet( save, GMDATA_ID_BSUBWAY_SCOREDATA );    /// @@TODO まだバトルサブウェイの連勝未実装
  if(BSUBWAY_SCOREDATA_GetMaxRenshouCount( bs_save, BSWAY_PLAYMODE_SINGLE) > BSUBWAY_BADGE_TERMS){
    rank++;
  }
#endif

  return rank;
}