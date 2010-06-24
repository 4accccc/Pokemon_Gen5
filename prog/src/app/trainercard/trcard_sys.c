/**
 *  @file trcard_sys.c
 *  @brief  トレーナーカードシステム
 *  @author Miyuki Iwasawa
 *  @date 08.01.16
 *
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
#include "savedata/c_gear_data.h"
#include "gamesystem/game_beacon.h"
#include "gamesystem/g_power.h"
#include "field/monolith_tool_notwifi.h"

//#include "app/mysign.h"
#include "app/trainer_card.h"
#include "trcard_sys.h"

#include "test/ariizumi/ari_debug.h"


// デバッグ情報を表示する時は定義を有効に
#define TRAINERCARD_DEBUG_PRINT_ENABLE  ///< 初期化処理や調査用パラメータを表示したい時は有効にする

#ifdef  TRAINERCARD_DEBUG_PRINT_ENABLE
#define DEBUG_TR_PRINT( ... )  OS_Printf(__VA_ARGS__)
#else
#define DEBUG_TR_PRINT( ... )  ((void)0)
#endif


//================================================================
// 構造体定義
//================================================================
typedef struct _TR_CARD_SYS{
  int heapId;

  void              *app_wk;          ///< 簡易会話モジュールワークの保存
  GFL_PROCSYS       *procSys;         ///< サブプロセスワーク
  PMS_SELECT_PARAM  PmsParam;         ///< 簡易会話アプリ呼び出し用ワーク

  TRCARD_CALL_PARAM *tcp;
  int               TrainerView;      ///< トレーナーカード開始時のユニオン見た目

}TR_CARD_SYS;

//================================================================
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
  CARD_OR_BADGE=0,
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
static int _branch_card_app( TR_CARD_SYS* wk, TRCARD_CALL_PARAM *pp );


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

  DEBUG_TR_PRINT("pp->mode=%d\n", pp->mode);

  // ユニオン見た目を保存
  wk->TrainerView = wk->tcp->TrCardData->UnionTrNo;
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
  TR_CARD_SYS* wk       = (TR_CARD_SYS*)mywk;
  TRCARD_CALL_PARAM* pp = (TRCARD_CALL_PARAM*)pwk;
  
  switch(*seq){
  case CARD_OR_BADGE:
    // 未クリアならバッジへ、殿堂入りしていればカードへ
    // ショートカットからの指定がある場合はその画面へ
    *seq = _branch_card_app( wk, pp );
    break;
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
  TRCARD_CALL_PARAM* param = (TRCARD_CALL_PARAM*)pwk;

  // ユニオン見た目が変更されていたらすれ違い情報を更新する
  if(param->edit_possible)
  {
    MYSTATUS *my     = GAMEDATA_GetMyStatus(param->gameData);
    int trainer_view = MyStatus_GetTrainerView( my );
    if(wk->TrainerView!=trainer_view)
    {
      DEBUG_TR_PRINT("すれ違い情報[見た目]を更新した\n");
      GAMEBEACON_SendDataUpdate_TrainerView(trainer_view);
    }
  }
  
  // 終了条件を引き継ぐ
  param->next_proc = wk->tcp->next_proc;

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
 * @brief 起動時にどの面（表・裏・バッジ）に行くか判断
 *
 * @param   wk    
 * @param   pp    
 *
 * @retval  int   CARD_INIT or BADGE_INIT
 */
//----------------------------------------------------------------------------------
static int _branch_card_app( TR_CARD_SYS* wk, TRCARD_CALL_PARAM *pp )
{
  // ショートカット指定があった場合はその画面へ
  switch(pp->mode){
  case TRCARD_SHORTCUT_FRONT:
    return CARD_INIT;
    break;
  case TRCARD_SHORTCUT_BACK:
    return CARD_INIT;
    break;
  case TRCARD_SHORTCUT_BADGE:
    return BADGE_INIT;
    break;
  case TRCARD_SHORTCUT_NONE:
    break;
  }

  // クリアするまでは、トレーナカードの最初をバッジ画面にする
  if(wk->tcp->TrCardData->Clear_m==0 ){
    // 自分のデータを参照しているモード
    if(wk->tcp->TrCardData->PlayTime!=NULL){
      return BADGE_INIT;
    }
  }
  
  return CARD_INIT;

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
  // プロセス定義データ
  static const GFL_PROC_DATA TrCardProcData = {
    TrCardProc_Init,
    TrCardProc_Main,
    TrCardProc_End,
  };

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
    GAMEBEACON_SendDataUpdate_TrCardIntroduction( wk->PmsParam.out_pms_data );
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
  
  // カードに戻るならカード初期化へ
  if(wk->tcp->value==CALL_CARD){
  return CARD_INIT;
  }

  return CARDSYS_END;
}


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
  SAVE_CONTROL_WORK *sv = GAMEDATA_GetSaveControlWork(gameData);
  TR_CARD_SV_PTR trc_ptr = TRCSave_GetSaveDataPtr(sv);
  MYSTATUS *mystatus = GAMEDATA_GetMyStatus( gameData );
  MISC *misc = GAMEDATA_GetMiscWork( gameData );
  GMTIME *gameTime = SaveData_GetGameTime( sv );
  RECORD *rec = SaveData_GetRecord( sv );
  const MYPMS_DATA *p_wk = SaveData_GetMyPmsDataConst(sv);
  ZUKAN_SAVEDATA *zukan = GAMEDATA_GetZukanSave( gameData );

  //名前
  if( MyStatus_CheckNameClear( mystatus ) == FALSE )
  {
    const STRCODE *baseName = MyStatus_GetMyName( mystatus );
    GFL_STD_MemCopy16( baseName , cardData->TrainerName , sizeof(STRCODE)*(PERSON_NAME_SIZE+EOM_SIZE) );
  }
  else
  {
    // 念のため名前が入ってないときに落ちないようにしておく
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

  //バッヂフラグ取得
  flag = 1;
  for( i=0; i<TRAINERCARD_BADGE_NUM; i++ )
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
    PLAYTIME *playTime    = GAMEDATA_GetPlayTimeWork( gameData );
    cardData->UnionTrNo   = MyStatus_GetTrainerView( mystatus );
    cardData->TimeUpdate  = FALSE;
    cardData->PlayTime    = NULL;
    cardData->PlayTime_h  = PLAYTIME_GetHour( playTime );
    cardData->PlayTime_m  = PLAYTIME_GetMinute( playTime );
    cardData->Personality = TRCSave_GetPersonarity(  trc_ptr );
    cardData->SignAnimeOn = TRCSave_GetSignAnime(  trc_ptr );
  }
  else
  {
    cardData->UnionTrNo   = MyStatus_GetTrainerView( mystatus );
    cardData->TimeUpdate  = TRUE;
    cardData->PlayTime    = GAMEDATA_GetPlayTimeWork( gameData );
    cardData->Personality = TRCSave_GetPersonarity(  trc_ptr );
    cardData->SignAnimeOn = TRCSave_GetSignAnime(  trc_ptr );
  }

  // 簡易会話取得
  MYPMS_GetPms( p_wk, MYPMS_PMS_TYPE_INTRODUCTION, &cardData->Pms );

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
  cardData->CommNum = RECORD_Get(rec, RECID_COMM_TRADE)  + RECORD_Get(rec, RECID_COMM_BATTLE) +
                      RECORD_Get(rec, RECID_WIFI_TRADE)  + RECORD_Get(rec, RECID_WIFI_BATTLE) +
                      RECORD_Get(rec, RECID_IRC_TRADE)   + RECORD_Get(rec, RECID_IRC_BATTLE)  +
                      RECORD_Get(rec, RECID_GURUGURU_COUNT );
  // 通信対戦回数
  cardData->CommBattleNum = RECORD_Get(rec, RECID_COMM_BATTLE) + 
                            RECORD_Get(rec, RECID_WIFI_BATTLE) +
                            RECORD_Get(rec, RECID_IRC_BATTLE);
  //勝ち数  ワイヤレス+WiFi
  cardData->CommBattleWin = RECORD_Get(rec, RECID_COMM_BTL_WIN) + 
                            RECORD_Get(rec, RECID_WIFI_BTL_WIN) +
                            RECORD_Get(rec, RECID_IRC_BTL_WIN);
  //負け数  ワイヤレス+WiFi
  cardData->CommBattleLose = RECORD_Get(rec, RECID_COMM_BTL_LOSE) + 
                             RECORD_Get(rec, RECID_WIFI_BTL_LOSE) +
                             RECORD_Get(rec, RECID_IRC_BTL_LOSE);
  //通信交換  ワイヤレス+WiFi
  cardData->CommTrade = RECORD_Get(rec, RECID_COMM_TRADE) + 
                        RECORD_Get(rec, RECID_WIFI_TRADE) +
                        RECORD_Get(rec, RECID_IRC_TRADE)  +
                        RECORD_Get(rec, RECID_GURUGURU_COUNT);

  // 野生エンカウント回数
  cardData->EncountNum        = RECORD_Get(rec, RECID_BTL_ENCOUNT);        
  // トレーナー戦をした回数
  cardData->TrainerEncountNum = RECORD_Get(rec, RECID_BTL_TRAINER);

  // すれ違い通信をした回数
  cardData->SuretigaiNum      = MISC_CrossComm_GetSuretigaiCount(misc);
  // フィーリングチェックをした回数
  cardData->FeelingCheckNum = RECORD_Get(rec, RECID_AFFINITY_CHECK_NUM);   
  // 使えるGパワーの数取得
  {
    POWER_CONV_DATA *powerdata;
    INTRUDE_SAVE_WORK *intsave = SaveData_GetIntrude( sv );
    u8 distribution_bit[INTRUDE_SAVE_DISTRIBUTION_BIT_WORK_MAX];
    
    powerdata = GPOWER_PowerData_LoadAlloc(heapId);
    ISC_SAVE_GetDistributionGPower_Array(
      intsave, distribution_bit, INTRUDE_SAVE_DISTRIBUTION_BIT_WORK_MAX);
    cardData->gPowerNum = MonolithToolEx_CountUsePower(
      powerdata, GAMEDATA_GetMyOccupyInfo(gameData), distribution_bit);
    GPOWER_PowerData_Unload(powerdata);
  }
  // ミュージカルをした回数
  cardData->MusicalNum        = RECORD_Get( rec, RECID_MUSICAL_PLAY_NUM )     
                               +RECORD_Get( rec, RECID_MUSICAL_COMM_NUM );     
  // ポケシフターのハイスコア
  {
    cardData->PokeshifterHigh = MISC_GetPalparkHighscore(misc);
  }
  // トライアウハウスの最高得点
  cardData->TrialHouseHigh = RECORD_GetThScore(rec);    
  // トライアルハウスのランク
  cardData->TrialHouseRank = RECORD_GetThRank(rec);

  // Cギアを持っているか
  {
    CGEAR_SAVEDATA *cgear = GAMEDATA_GetCGearSaveData( gameData );
    cardData->CgearGetFlag  = CGEAR_SV_GetCGearONOFF(cgear);    
  }
  
  // パレスに行った事があるか
  {
    INTRUDE_SAVE_WORK *intrude_sv = SaveData_GetIntrude( sv );
    if(ISC_SAVE_GetPalaceSojournTime(intrude_sv)){
      cardData->PaleceFlag      = 1;    
    } 
  }
  // ミュージカルをした事があるか
  if(cardData->MusicalNum){
    cardData->MusicalFlag   = 1;    
  }
  // ポケシフターをした事があるか
  if(RECORD_Get( rec, RECID_POKESHIFTER_COUNT)){
    cardData->PokeshifterFlag = 1;    
  }
  // トライアルハウスに参加した事があるか
  if(RECORD_Get( rec, RECID_TRIALHOUSE_COUNT)){
    cardData->TrianHouseFlag  = 1;    
  }
  
  //図鑑処理
  {
    EVENTWORK * ev = GAMEDATA_GetEventWork( gameData );
    if( EVENTWORK_CheckEventFlag( ev, SYS_FLAG_ZUKAN_GET ) == TRUE ){
      cardData->PokeBookFlg = 1;
    }else{
      cardData->PokeBookFlg = 0;
    }
    cardData->PokeBook    = ZUKANSAVE_GetZukanPokeSeeCount( zukan, heapId );
  }

  
  
  

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
    DEBUG_TR_PRINT( "ストーリークリア\n");
    rank++;
  }
  //全国図鑑完成（イベント系ポケモンを除くポケモンをゲットしているか）
  if ( ZUKANSAVE_CheckZenkokuComp(GAMEDATA_GetZukanSave(gameData)) ){
    DEBUG_TR_PRINT( "全国図鑑完成\n");
    rank++;
  }

  // バトルサブウェイ、スーパーシングル、スーパーダブルでボス撃破
  if(BSUBWAY_SCOREDATA_SetFlag( bs_score,  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_S_SINGLE, BSWAY_SETMODE_get) 
  && BSUBWAY_SCOREDATA_SetFlag( bs_score,  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_S_DOUBLE, BSWAY_SETMODE_get) 
//  && BSUBWAY_SCOREDATA_SetFlag( bs_score,  BSWAY_SCOREDATA_FLAG_BOSS_CLEAR_S_MULTI, BSWAY_SETMODE_get)    // 必要なら
  ){
    DEBUG_TR_PRINT( "バトルサブウェイボス撃破\n");
    rank++;
  }

  // ミュージカルグッズをコンプリートしているか
  if( MUSICAL_SAVE_IsCompleteItem( musical_sv ) ){
    DEBUG_TR_PRINT( "ミュージカルグッズコンプ\n");
    rank++;
  }
  
  // ブラックシティ・ホワイトフォレストのバランス数値が３０以上
  if(OccupyInfo_GetWhiteLevel(occupy_sv)>=OCCUPY_QUALIFY
  && OccupyInfo_GetBlackLevel(occupy_sv)>=OCCUPY_QUALIFY
  ){
    DEBUG_TR_PRINT( "BC/WFのバランス数値が両方共３０以上\n");
    rank++;
  }

  return rank;
}