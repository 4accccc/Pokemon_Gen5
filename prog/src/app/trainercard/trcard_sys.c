/**
 *  @file trcard_sys.c
 *  @brief  トレーナーカードシステム
 *  @author Miyuki Iwasawa
 *  @date 08.01.16
 *
 *  @todo
 *    金銀ではトレーナーカードはサイン書きアプリを呼び出していたが、WBでは直接カード内で
 *    サインが書けるようになっている。mysignアプリが他で流用されないのであれば削除してよい
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
#include "savedata/musical_save.h"
#include "savedata/intrude_save.h"
#include "savedata/my_pms_data.h"
#include "savedata/zukan_savedata.h"

//#include "app/mysign.h"
#include "app/trainer_card.h"
#include "trcard_sys.h"

#include "test/ariizumi/ari_debug.h"


typedef struct _TR_CARD_SYS{
  int heapId;

  void              *app_wk;          ///< 簡易会話モジュールワークの保存
  GFL_PROCSYS       *procSys;         ///< サブプロセスワーク
  PMS_SELECT_PARAM  PmsParam;         ///< 簡易会話アプリ呼び出し用ワーク

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
  PMSINPUT_INIT,
  PMSINPUT_WAIT,
  BADGE_INIT,
  BADGE_WAIT,
  CARDSYS_END,
};

static int sub_CardInit(TR_CARD_SYS* wk);
static int sub_CardWait(TR_CARD_SYS* wk);
static int sub_PmsInputInit(TR_CARD_SYS* wk);
static int sub_PmsInputWait(TR_CARD_SYS* wk);
static int sub_BadgeInit(TR_CARD_SYS* wk);
static int sub_BadgeWait(TR_CARD_SYS* wk);


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
  TRAINERCARD_GetSelfData( wk->tcp->TrCardData , pp->gameData , FALSE, wk->tcp->edit_possible, wk->heapId);

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
  case PMSINPUT_INIT:
    *seq = sub_PmsInputInit(wk);
    break;
  case PMSINPUT_WAIT:
    *seq = sub_PmsInputWait(wk);
    break;
  case BADGE_INIT:
    *seq = sub_BadgeInit(wk);
    break;
  case BADGE_WAIT:
    *seq = sub_BadgeWait(wk);
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
  
  if(wk->tcp->value==CALL_PMSINPUT){
    return PMSINPUT_INIT;
  }else if(wk->tcp->value==CALL_BADGE){
    return BADGE_INIT;
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
static int sub_PmsInputInit(TR_CARD_SYS* wk)
{
  if( wk->procSys == NULL )
  { 
    wk->procSys = GFL_PROC_LOCAL_boot( wk->heapId );
  }
  wk->PmsParam.save_ctrl = GAMEDATA_GetSaveControlWork( wk->tcp->gameData );
  GFL_PROC_LOCAL_CallProc( wk->procSys, FS_OVERLAY_ID(pmsinput), &ProcData_PMSSelect, &wk->PmsParam);
  return PMSINPUT_WAIT;
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
static int sub_PmsInputWait(TR_CARD_SYS* wk)
{
  if(!TrCardSysProcCall(&wk->procSys)){
    return PMSINPUT_WAIT;
  }
  // 作成した簡易会話があれば書き換え
  if(wk->PmsParam.out_pms_data!=NULL){
    MYPMS_DATA *p_wk = SaveData_GetMyPmsData(GAMEDATA_GetSaveControlWork(wk->tcp->gameData));
    wk->tcp->TrCardData->Pms = *wk->PmsParam.out_pms_data;
//    TRCSave_SetPmsData( trc_ptr, wk->PmsParam.out_pms_data ); // セーブデータにも反映
    MYPMS_SetPms( p_wk, MYPMS_PMS_TYPE_INTRODUCTION, wk->PmsParam.out_pms_data );
  }
  
  return CARD_INIT;
}

extern const GFL_PROC_DATA BadgeViewProcData;

//----------------------------------------------------------------------------------
/**
 * @brief 簡易会話呼び出し
 *
 * @param   wk    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int sub_BadgeInit(TR_CARD_SYS* wk)
{
  if( wk->procSys == NULL )
  { 
    wk->procSys = GFL_PROC_LOCAL_boot( wk->heapId );
  }
  wk->PmsParam.save_ctrl = GAMEDATA_GetSaveControlWork( wk->tcp->gameData );
  GFL_PROC_LOCAL_CallProc( wk->procSys, FS_OVERLAY_ID(pmsinput), &BadgeViewProcData, wk->tcp );
  return BADGE_WAIT;
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
static int sub_BadgeWait(TR_CARD_SYS* wk)
{
  if(!TrCardSysProcCall(&wk->procSys)){
    return BADGE_WAIT;
  }
  

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
//=============================================================================================
/**
 * @brief 自分のトレーナーカードデータを収集する
 *
 * @param   cardData    格納するカードデータポインタ
 * @param   gameData    GAMEDATA
 * @param   isSendData  通信用データかどうか（TRUE:通信データ）
 * @param   edit        編集可能データかどうか（TRUE:編集可能　FALSE:不可能）
 * @param   heapId      ヒープＩＤ
 */
//=============================================================================================
void TRAINERCARD_GetSelfData( TR_CARD_DATA *cardData , GAMEDATA *gameData , const BOOL isSendData, BOOL edit, HEAPID heapId )
{
  u8 i,flag;
  TR_CARD_SV_PTR trc_ptr = TRCSave_GetSaveDataPtr(GAMEDATA_GetSaveControlWork(gameData));
  MYSTATUS *mystatus = GAMEDATA_GetMyStatus( gameData );
  MISC *misc = GAMEDATA_GetMiscWork( gameData );
  GMTIME *gameTime = SaveData_GetGameTime( GAMEDATA_GetSaveControlWork(gameData) );
  RECORD *rec = SaveData_GetRecord( GAMEDATA_GetSaveControlWork(gameData) );
  const MYPMS_DATA *p_wk = SaveData_GetMyPmsDataConst(GAMEDATA_GetSaveControlWorkConst(gameData));
  ZUKAN_SAVEDATA *zukan = GAMEDATA_GetZukanSave( gameData );

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

  cardData->TrSex     = MyStatus_GetMySex( mystatus );
  cardData->TrainerID = MyStatus_GetID( mystatus );
  cardData->Money     = MISC_GetGold( misc );
  cardData->BadgeFlag = 0;
  cardData->Version   = PM_VERSION;

  //FIXME 正しいバッヂ個数の定義に(8個)
  flag = 1;
  for( i=0; i<8; i++ )
  {
    if( MISC_GetBadgeFlag( misc , i ) == TRUE )
    {
      cardData->BadgeFlag |= flag;
    }
    flag <<= 1;
  }
  //通信用かで分岐
  if( isSendData == TRUE )
  {
    PLAYTIME *playTime = SaveData_GetPlayTime( GAMEDATA_GetSaveControlWork(gameData) );
    cardData->UnionTrNo = MyStatus_GetTrainerView( mystatus );
    cardData->TimeUpdate = FALSE;
    cardData->PlayTime = NULL;
    cardData->PlayTime_h = PLAYTIME_GetHour( playTime );
    cardData->PlayTime_m = PLAYTIME_GetMinute( playTime );
  }
  else
  {
    cardData->UnionTrNo   = MyStatus_GetTrainerView( mystatus );
    cardData->TimeUpdate  = TRUE;
    cardData->PlayTime    = SaveData_GetPlayTime( GAMEDATA_GetSaveControlWork(gameData) );
    cardData->Personality = TRCSave_GetPersonarity(  trc_ptr );
//    TRCSave_GetPmsData( trc_ptr, &cardData->Pms );
    MYPMS_GetPms( p_wk, MYPMS_PMS_TYPE_INTRODUCTION, &cardData->Pms );

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
  cardData->CommNum = RECORD_Get(rec, RECID_COMM_TRADE)+RECORD_Get(rec, RECID_WIFI_TRADE)+
                      RECORD_Get(rec, RECID_COMM_BATTLE)+RECORD_Get(rec, RECID_WIFI_BATTLE);
                      
  // 通信対戦回数
  cardData->CommBattleNum = RECORD_Get(rec, RECID_BATTLE_COUNT);
  //勝ち数  ワイヤレス+WiFi
  cardData->CommBattleWin = RECORD_Get(rec, RECID_COMM_BTL_WIN)+RECORD_Get(rec, RECID_WIFI_BTL_WIN);
  //負け数  ワイヤレス+WiFi
  cardData->CommBattleLose = RECORD_Get(rec, RECID_COMM_BTL_LOSE)+RECORD_Get(rec, RECID_WIFI_BTL_LOSE);
  //通信交換  ワイヤレス+WiFi
  cardData->CommTrade = RECORD_Get(rec, RECID_COMM_TRADE)+RECORD_Get(rec, RECID_WIFI_TRADE);
  
  //図鑑処理
  cardData->PokeBookFlg = TRUE;
  cardData->PokeBook    = ZUKANSAVE_GetZukanPokeGetCount( zukan, heapId );

  //サインデータの取得
  //サインデータの有効/無効フラグを取得(金銀ローカルでのみ有効)
  cardData->MySignValid = TRCSave_GetSigned(trc_ptr);

  //サインデータをセーブデータからコピー
  MI_CpuCopy8(TRCSave_GetSignDataPtr(trc_ptr),
      cardData->SignRawData, SIGN_SIZE_X*SIGN_SIZE_Y*8 );

  // 編集可能フラグの格納
  cardData->EditPossible = edit;

}

//=============================================================================================
/**
 * @brief CALL_PARAMの作成
 *
 * @param   gameData    
 * @param   heapId    
 * @param   edit      編集可能か(TRUE:OK FALSE:NG)
 *
 * @retval  TRCARD_CALL_PARAM*    
 */
//=============================================================================================
TRCARD_CALL_PARAM* TRAINERCASR_CreateCallParam_SelfData( GAMEDATA *gameData , HEAPID heapId, BOOL edit )
{
  TRCARD_CALL_PARAM* callParam;
  callParam = GFL_HEAP_AllocClearMemory( heapId , sizeof( TRCARD_CALL_PARAM ) );
  GFL_STD_MemClear( callParam, sizeof(TRCARD_CALL_PARAM) );
  callParam->TrCardData    = NULL;
  callParam->gameData      = gameData;
  callParam->value         = 0;
  callParam->edit_possible = edit;
  
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
  
  callParam = GFL_HEAP_AllocClearMemory( heapId , sizeof( TRCARD_CALL_PARAM ) );
  callParam->TrCardData = pCardData;
  callParam->gameData = gameData;
  callParam->value = 0;
  callParam->TrCardData->EditPossible = FALSE;   // 必ず編集不可能
  callParam->TrCardData->OtherTrCard  = TRUE;

  return callParam;
}


// ブラックシティ・ホワイトフォレストのトレーナーカードランクアップ条件
#define OCCUPY_QUALIFY  ( 30 )

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
  BSUBWAY_SCOREDATA *bs_score   = GAMEDATA_GetBSubwayScoreData( gameData );
  MUSICAL_SAVE      *musical_sv = GAMEDATA_GetMusicalSavePtr( gameData );
  OCCUPY_INFO       *occupy_sv  = GAMEDATA_GetMyOccupyInfo( gameData );

  //ストーリークリア（殿堂入りとは違う）
  if (EVENTWORK_CheckEventFlag(GAMEDATA_GetEventWork( gameData ),SYS_FLAG_GAME_CLEAR)){
    OS_Printf( "ストーリークリア\n");
    rank++;
  }
  //全国図鑑完成（イベント系ポケモンを除くポケモンをゲットしているか）
  if ( ZUKANSAVE_CheckZenkokuComp(GAMEDATA_GetZukanSave(gameData)) ){
    OS_Printf( "全国図鑑完成\n");
    rank++;
  }

  // バトルサブウェイ、スーパーシングル、スーパーダブルでボス撃破
  if(BSUBWAY_SCOREDATA_SetFlag( bs_score,  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_S_SINGLE, BSWAY_SETMODE_get) 
  && BSUBWAY_SCOREDATA_SetFlag( bs_score,  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_S_DOUBLE, BSWAY_SETMODE_get) 
//  && BSUBWAY_SCOREDATA_SetFlag( bs_score,  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_S_MULTI, BSWAY_SETMODE_get)    // 必要なら
  ){
    OS_Printf( "バトルサブウェイボス撃破\n");
    rank++;
  }

  // ミュージカルグッズをコンプリートしているか
  if( MUSICAL_SAVE_IsCompleteItem( musical_sv ) ){
    OS_Printf( "ミュージカルグッズコンプ\n");
    rank++;
  }
  
  // ブラックシティ・ホワイトフォレストのバランス数値がうにゃうにゃ
  if(OccupyInfo_GetWhiteLevel(occupy_sv)>OCCUPY_QUALIFY
  && OccupyInfo_GetBlackLevel(occupy_sv)>OCCUPY_QUALIFY
  ){
    OS_Printf( "BC/WFのバランス数値が両方共３０以上\n");
    rank++;
  }

  return rank;
}