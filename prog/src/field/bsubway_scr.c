//======================================================================
/**
 * @file bsubway_scr.c
 * @brief バトルサブウェイ　スクリプト関連
 * @authaor kagaya
 * @date 2008.12.11
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "savedata/save_tbl.h"

#include "system/main.h"  //HEAPID_PROC

#include "savedata/bsubway_savedata.h"
#include "savedata/bsubway_savedata_def.h"
#include "battle/bsubway_battle_data.h"
#include "bsubway_scr_def.h"
#include "bsubway_scr.h"
#include "bsubway_scr_common.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================
static u8 bswScr_GetMemberNum( u16 mode );

//======================================================================
//  バトルサブウェイ　スクリプトワーク関連
//======================================================================
//--------------------------------------------------------------
/**
 * BSUBWAY_SCRWORK ワーク初期化
 * @param bsw_scr BSUBWAY_SCRWORK*
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_ClearWork( BSUBWAY_SCRWORK **bsw_scr )
{
  GF_ASSERT( *bsw_scr == NULL );
  *bsw_scr = NULL;
}

//--------------------------------------------------------------
/**
 * BSUBWAY_SCRWORK ワークエリアを取得して初期化する
 * @param gsys GAMESYS_WORK*
 * @param init 初期化モード BSWAY_PLAY_NEW:始め、BSWAY_PLAY_CONTINE:続き
 * @param playmode プレイモード指定:BTWR_MODE_～*
 * @retval BSUBWAY_SCRWORK*
 */
//--------------------------------------------------------------
BSUBWAY_SCRWORK * BSUBWAY_SCRWORK_CreateWork(
    GAMESYS_WORK *gsys, u16 init, u16 playmode )
{
  int i;
  BSUBWAY_SCRWORK *bsw_scr;
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  MYSTATUS *mystatus = GAMEDATA_GetMyStatus( gdata );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
  
  bsw_scr = GFL_HEAP_AllocClearMemory(
      HEAPID_PROC, sizeof(BSUBWAY_SCRWORK) );
  
  bsw_scr->heapID = HEAPID_PROC;
  bsw_scr->magicNo = BSUBWAY_SCRWORK_MAGIC;
  
  bsw_scr->playData = SaveControl_DataPtrGet(
      save, GMDATA_ID_BSUBWAY_PLAYDATA );
  bsw_scr->scoreData = SaveControl_DataPtrGet(
      save, GMDATA_ID_BSUBWAY_SCOREDATA );
  
  BSUBWAY_PLAYDATA_SetSaveFlag( bsw_scr->playData, FALSE ); //セーブなしに
  
  if( init == BSWAY_PLAY_NEW ){ //新規
    u8 buf8;
    
    bsw_scr->play_mode = playmode;
    bsw_scr->play_mode = playmode;
    bsw_scr->member_num = bswScr_GetMemberNum( bsw_scr->play_mode );
    bsw_scr->now_round = 1;
    bsw_scr->now_win = 0;
    
    for(i = 0;i < BSUBWAY_STOCK_MEMBER_MAX;i++){
      bsw_scr->member[i] = BSWAY_NULL_POKE;
    }
    
    for(i = 0;i < BSUBWAY_STOCK_TRAINER_MAX;i++){
      bsw_scr->trainer[i] = BSWAY_NULL_TRAINER;
    }
    
    //プレイデータ初期化
    BSUBWAY_PLAYDATA_Init( bsw_scr->playData );
    buf8 = bsw_scr->play_mode; //プレイモードだけはこの時点でセーブ反映
    BSUBWAY_PLAYDATA_SetData(
        bsw_scr->playData, BSWAY_PLAYDATA_ID_playmode, &buf8 );
    
    /* //wb null
    #ifdef BTOWER_AUTO_DEB
    if(wk->play_mode == BTWR_MODE_WIFI){
      OS_Printf(" #WifiTowerStage=%d\n",DebugBTowerAutoStage++);
    }
    #endif
    */
  }else{ //続き　データロード
    bsw_scr->play_mode = (u8)BSUBWAY_PLAYDATA_GetData(
        bsw_scr->playData, BSWAY_PLAYDATA_ID_playmode, NULL );
    bsw_scr->now_round = (u8)BSUBWAY_PLAYDATA_GetData(
        bsw_scr->playData, BSWAY_PLAYDATA_ID_round, NULL );
    bsw_scr->now_win = bsw_scr->now_round-1;
    
    bsw_scr->member_num = bswScr_GetMemberNum( bsw_scr->play_mode );
    
    //選ばれているポケモンNo
    BSUBWAY_PLAYDATA_GetData( bsw_scr->playData,
        BSWAY_PLAYDATA_ID_pokeno, bsw_scr->member );
    
    //抽選済みのトレーナーNo
    BSUBWAY_PLAYDATA_GetData( bsw_scr->playData,
        BSWAY_PLAYDATA_ID_trainer, bsw_scr->trainer);
    
    #if 0 //wb null
    //プレイランダムシード取得
    wk->play_rnd_seed = TowerPlayData_Get(
        wk->playSave,BTWR_PSD_rnd_seed,NULL);
    OS_Printf("TowerContinueRndSeed = %d\n",wk->play_rnd_seed);
    #endif
    
    #if 0 //wb null
    if( bsw_scr->play_mode == BSWAY_MODE_MULTI ){
      wk->partner = (u8)TowerPlayData_Get(
          wk->playSave,BTWR_PSD_partner,NULL);

      //パートナートレーナーデータ再生成
      TowerPlayData_Get(wk->playSave,
          BTWR_PSD_pare_poke,&(wk->five_poke[wk->partner]));
      RomBattleTowerPartnerDataMake(wk,&wk->five_data[wk->partner],
        TOWER_FIVE_FIRST+wk->partner,
        TowerPlayData_Get(wk->playSave,BTWR_PSD_pare_itemfix,NULL),
        &(wk->five_poke[wk->partner]),wk->heapID);
    }
    #endif
  }
  
  bsw_scr->my_sex = MyStatus_GetMySex( mystatus );
  
  //現在のレコードを取得
#if 0 //wb null
  if( bsw_scr->play_mode != BSWAY_MODE_RETRY ){
    frontier = SaveData_GetFrontier(savedata);
    record = SaveData_GetRecord(savedata);
    
    //現在の連勝数は挑戦中フラグがonのときだけ引き継ぐ
    if(wk->play_mode == BTWR_MODE_WIFI_MULTI){
#if 0
      chg_flg = TowerScoreData_SetFlags(wk->scoreSave,
            BTWR_SFLAG_WIFI_MULTI_RECORD,BTWR_DATA_get);
#else
      //chg_flg = FrontierRecord_Get(SaveData_GetFrontier(savedata), 
      //            FRID_TOWER_MULTI_WIFI_CLEAR_BIT,
      //            Frontier_GetFriendIndex(FRID_TOWER_MULTI_WIFI_CLEAR_BIT) );
      chg_flg = SysWork_WifiFrClearFlagGet( SaveData_GetEventWork(savedata) );
      OS_Printf( "WIFI chg_flg = %d\n", chg_flg );
#endif
    }else{
      chg_flg = TowerScoreData_SetFlags(wk->scoreSave,
            BTWR_SFLAG_SINGLE_RECORD+wk->play_mode,BTWR_DATA_get);
    }

    if(chg_flg){
      if(wk->play_mode == BTWR_MODE_WIFI_MULTI){
        wk->renshou = FrontierRecord_Get(
        frontier,FRID_TOWER_MULTI_WIFI_RENSHOU_CNT,
        Frontier_GetFriendIndex(FRID_TOWER_MULTI_WIFI_RENSHOU_CNT) );
      }else{
        wk->renshou = FrontierRecord_Get(
        frontier,FRID_TOWER_SINGLE_RENSHOU_CNT+wk->play_mode*2,
        FRONTIER_RECORD_NOT_FRIEND);
      }

      wk->stage = TowerScoreData_SetStage(
          wk->scoreSave,wk->play_mode,BTWR_DATA_get);
    }

    wk->win_cnt = RECORD_Get(record,RECID_BTOWER_WIN);

    //wk->stageがchg_flgに関係なく取得していたので変更(08.05.20)
    //wifiのために変更したがwifiでは連勝記録はDPとは別のものを使用しているので、
    //周回数の変数は見ていないから平気と思われるが一応。
    //wk->stage = TowerScoreData_SetStage(
        wk->scoreSave,wk->play_mode,BTWR_DATA_get);
  }
#endif

#if 0 //wb null
  //WIFI(32人データがあるので、周回数ワークが足りないため、
  //連勝数から周回数を算出してセット
  if(wk->play_mode == BTWR_MODE_WIFI_MULTI){
    wk->stage = TowerScoreData_SetStageValue(
        wk->scoreSave, BTWR_MODE_WIFI_MULTI, 
        (wk->renshou / 7) );
    OS_Printf( "** wk->renshou = %d\n", wk->renshou );
    OS_Printf( "** stage = %d\n", TowerScoreData_SetStage(wk->scoreSave,
                              BTWR_MODE_WIFI_MULTI,BTWR_DATA_get) );
  }

  OS_Printf( "stage = %d\n", wk->stage );
  return wk;
#endif
  return( bsw_scr );
}

//--------------------------------------------------------------
/**
 * BSUBWAY_SCRWORK ワークエリアを開放する
 * @param bsw_scr BSUBWAY_SCRWORK*
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_ReleaseWork( BSUBWAY_SCRWORK *bsw_scr )
{
  GF_ASSERT( bsw_scr != NULL );
  
  if( bsw_scr != NULL ){
    GF_ASSERT( bsw_scr->magicNo == BSUBWAY_SCRWORK_MAGIC );
    MI_CpuClear8( bsw_scr, sizeof(BSUBWAY_SCRWORK) );
    GFL_HEAP_FreeMemory( bsw_scr );
  }
}

//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 * プレイモード別メンバー数
 * @param play_mode 
 * @retval  
 */
//--------------------------------------------------------------
static u8 bswScr_GetMemberNum( u16 mode )
{
  switch(mode){
  case BSWAY_MODE_SINGLE:
  case BSWAY_MODE_WIFI:
  case BSWAY_MODE_RETRY:
    return 3;
  case BSWAY_MODE_DOUBLE:
    return 4;
  case BSWAY_MODE_MULTI:
  case BSWAY_MODE_COMM_MULTI:
  case BSWAY_MODE_WIFI_MULTI:
    return 2;
  }
  return 0;
}

//======================================================================
//  バトルサブウェイ　スクリプトコマンド関連
//======================================================================
//--------------------------------------------------------------
/**
 *  @brief  バトルタワー用ポケモン選択画面呼び出し呼出し
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_SelectPoke(
    BSUBWAY_SCRWORK *wk, GMEVENT *ev, void **proc_wk )
{
#if 0 //wb
  EventCmd_BTowerPokeSelectCall( ev_work,proc_wk,
    PL_MODE_BATTLE_TOWER,PST_MODE_NORMAL,
    wk->member_num,wk->member_num,100,0);
#endif
}

//--------------------------------------------------------------
/**
 *  @brief  選択したポケモンを取得
 *  @retval  TRUE  選択した
 *  @retval  FALSE  選択せずにやめた
 */
//--------------------------------------------------------------
BOOL BSUBWAY_SCRWORK_GetEntryPoke(
    BSUBWAY_SCRWORK *wk, void **app_work, GAMESYS_WORK *gsys ) //SAVEDATA *sv )
{
#if 0 //wb
  u16  i = 0;
  PLIST_DATA *pld = *app_work;
  POKEPARTY *party;
  POKEMON_PARAM *pp;

#ifdef BSUBWAY_AUTO_DEB
  if( pld->ret_mode != 0){
    sys_FreeMemoryEz(*app_work );
    *app_work = NULL;
    return FALSE;
  }
#else  //BSUBWAY_AUTO_DEB
  //データ取得
  if( pld->ret_mode != 0 || pld->ret_sel == PL_SEL_POS_EXIT ){
    sys_FreeMemoryEz(*app_work );
    *app_work = NULL;
    return FALSE;
  }
#endif  //BSUBWAY_AUTO_DEB
  party = SaveData_GetTemotiPokemon( sv );
  
  for( i = 0;i < wk->member_num;i++){
    wk->member[i] = pld->in_num[i]-1;
    pp = PokeParty_GetMemberPointer( party,wk->member[i]);
    wk->mem_poke[i] = PokeParaGet( pp,ID_PARA_monsno,NULL );  
    wk->mem_item[i] = PokeParaGet( pp,ID_PARA_item,NULL );  
  }
  //ワーク領域解放
  sys_FreeMemoryEz(*app_work );
  *app_work = NULL;
#endif
  return TRUE;
}

//--------------------------------------------------------------
/**
 *  @brief  参加指定した手持ちポケモンの条件チェック
 *  @retval  0  参加OK
 *  @retval  1  同じポケモンがいる
 *  @retval 2  同じアイテムを持っている
 */
//--------------------------------------------------------------
int BSUBWAY_SCRWORK_CheckEntryPoke(
    BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys ) //SAVEDATA *savedata )
{
#if 0 //wb
#if 1
  u16  i = 0,j = 0;
  u16  monsno[BSUBWAY_STOCK_MEMBER_MAX],itemno[BSUBWAY_STOCK_MEMBER_MAX];
  POKEPARTY *party;
  POKEMON_PARAM *poke;

  party = SaveData_GetTemotiPokemon( savedata );

  for( i = 0;i < wk->member_num;i++){
    poke = PokeParty_GetMemberPointer( party,wk->member[i]);
    monsno[i] = PokeParaGet( poke,ID_PARA_monsno,NULL );
    itemno[i] = PokeParaGet( poke,ID_PARA_item,NULL );

    if( i == 0){
      continue;
    }
    for( j = 0;j < i;j++){
      if( monsno[i] == monsno[j]){
        //同じポケモンがいる
        return 1;
      }
      if(( itemno[i] != 0) && ( itemno[i] == itemno[j])){
        //同じアイテムをもっている
        return 2;
      }
    }
  }
  return 0;
#else
  return BSUBWAY_SCRWORK_CheckEntryPokeSub( wk->member, wk->member_num, savedata );
#endif
#else
  return 0;
#endif
}

#if 0 //wb
static BOOL is_trainer_conflict( u16* trainer,u16 id,u16 num )
{
  u16 i;
  for( i = 0;i < num;i++){
    if( trainer[i] == id ){
      return TRUE;
    }
  }
  return FALSE;
}
#endif

//--------------------------------------------------------------
/**
 *  @brief　対戦トレーナーNo抽選
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_BtlTrainerNoSet( BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys )
{
#if 0 //wb
  int i;
  u16  no,stage;
  
  if(  wk->play_mode == BTWR_MODE_MULTI ||
    wk->play_mode == BTWR_MODE_WIFI_MULTI ||
    wk->play_mode == BTWR_MODE_COMM_MULTI ){
    //if( wk->play_mode == BTWR_MODE_COMM_MULTI && wk->pare_stage > wk->stage ){
    if( ( wk->play_mode == BTWR_MODE_COMM_MULTI && wk->pare_stage > wk->stage ) ||
      ( wk->play_mode == BTWR_MODE_WIFI_MULTI && wk->pare_stage > wk->stage ) ){
      stage = wk->pare_stage;  //通信時には周回数の多いほうで抽選
    }else{
      stage = wk->stage;
    }
    for( i = 0;i < BSUBWAY_STOCK_TRAINER_MAX;i++){
      do{
        no = BattleTowerTrainerNoGet( wk,stage,i/2,wk->play_mode );
      }while( is_trainer_conflict( wk->trainer,no,i ));
#ifdef BSUBWAY_AUTO_DEB
#if 0
      wk->trainer[i] = DebugBTowerAutoTrainer;
      OS_Printf(" #TowerStage = %d,%d\n",DebugBTowerAutoStage,DebugBTowerAutoTrainer );
      if( DebugBTowerAutoTrainer++ >= 299){
        DebugBTowerAutoTrainer = 0;
        DebugBTowerAutoStage++;
      }
#endif
      wk->trainer[i] = no;
      if( DebugBTowerAutoTrainer++ >= 299){
        DebugBTowerAutoStage++;
      }
      OS_Printf(" #TowerStage = %d,%d\n",DebugBTowerAutoStage,DebugBTowerAutoTrainer );
#else    
      wk->trainer[i] = no;
#endif
    }
  }else{
    for( i = 0;i < ( BSUBWAY_STOCK_TRAINER_MAX/2);i++){
      do{
        no = BattleTowerTrainerNoGet( wk,wk->stage,i,wk->play_mode );
      }while( is_trainer_conflict( wk->trainer,no,i ));
#ifdef BSUBWAY_AUTO_DEB
#if 0
      wk->trainer[i] = DebugBTowerAutoTrainer;
      OS_Printf(" #TowerStage = %d,%d\n",DebugBTowerAutoStage,DebugBTowerAutoTrainer );
      if( DebugBTowerAutoTrainer++ >= 299){
        DebugBTowerAutoTrainer = 0;
        DebugBTowerAutoStage++;
      }
#endif
      if( DebugBTowerAutoTrainer++ >= 299){
        DebugBTowerAutoStage++;
      }
      OS_Printf(" #TowerStage = %d,%d\n",DebugBTowerAutoStage,DebugBTowerAutoTrainer );
      wk->trainer[i] = 25;//no;
#else    
      wk->trainer[i] = no;
#endif
    }
  }
#endif
}

//--------------------------------------------------------------
/**
 *  @brief  現在のラウンド数をスクリプトワークに取得する
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCRWORK_GetNowRound( BSUBWAY_SCRWORK *wk )
{
  return wk->now_round;
}

//--------------------------------------------------------------
/**
 *  @brief  7連勝しているかどうかチェック
 */
//--------------------------------------------------------------
BOOL BSUBWAY_SCRWORK_IsClear( BSUBWAY_SCRWORK *wk )
{
  if( wk->clear_f ){
    return TRUE;
  }
  
  if( wk->now_round > BSWAY_CLEAR_WINCNT ){
    //クリアフラグon
    wk->clear_f = 1;
    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 *  @brief  最大連勝記録更新番組作成関数
 */
//--------------------------------------------------------------
#if 0 //wb
static void bswayscr_MakeTVRenshouMaxUpdate(
    BSUBWAY_SCRWORK *wk,SAVEDATA *savedata,u16 renshou )
{
  POKEPARTY *party;

  if(  wk->play_mode != BTWR_MODE_SINGLE &&
    wk->play_mode != BTWR_MODE_DOUBLE ){
    return;
  }
  party = SaveData_GetTemotiPokemon( savedata );

  if( wk->play_mode == BTWR_MODE_SINGLE ){
    //TVTOPIC_Entry_Record_BTower( fsys,
    TVTOPIC_Entry_Record_BTowerEx( savedata,
      renshou,PokeParty_GetMemberPointer( party,wk->member[0]), TRUE );
  }else{
    //TVTOPIC_Entry_Record_BTower( fsys,
    TVTOPIC_Entry_Record_BTowerEx( savedata,
      renshou,PokeParty_GetMemberPointer( party,wk->member[0]), FALSE );
  }
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  7人抜き成功または負けた時の共通データセーブ処理
 *  @param  now_renshou  現在の連勝数
 */
//--------------------------------------------------------------
#if 0 //wb
static void bswayscr_SetCommonScore(
    BSUBWAY_SCRWORK *wk, SAVEDATA *sv, u8 win_f, u16 now_renshou )
{
  u8  buf8;

  switch( wk->play_mode ){
  case BTWR_MODE_SINGLE:
    //ポケモンデータセット
    bswayscr_SaveMemberPokeData( wk,sv,BTWR_SCORE_POKE_SINGLE );
  case BTWR_MODE_DOUBLE:
    if( now_renshou >= 7){
      //TVインタビューデータセット(シングルとダブルで実行)
      TVTOPIC_BTowerTemp_Set( SaveData_GetTvWork( sv ),win_f,now_renshou );
    }
    break;
  case BTWR_MODE_WIFI:
    //ポケモンデータセット
    bswayscr_SaveMemberPokeData( wk,sv,BTWR_SCORE_POKE_WIFI );
    //スコア押し出し
    TowerPlayData_WifiRecordAdd( wk->playSave,wk->rec_down,wk->rec_turn,wk->rec_damage );

    //プレイモード書き出し
    buf8 = wk->play_mode;
    TowerPlayData_Put( wk->playSave,BTWR_PSD_playmode,&buf8);
    //ラウンド数書き出し
    buf8 = wk->now_round;
    TowerPlayData_Put( wk->playSave,BTWR_PSD_round,&buf8);
    
    TowerScoreData_SetWifiScore( wk->scoreSave,wk->playSave );
    break;
  default:
    break;
  }
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  敗戦処理  
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_SetLoseScore( BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys )
{
#if 0 //wb
  u32  ret = 0;
  int  id;
  u16  before,after,chg_flg;
  RECORD *record = SaveData_GetRecord( savedata );
  FRONTIER_SAVEWORK *frontier = SaveData_GetFrontier( savedata );
  
  if( wk->play_mode == BTWR_MODE_RETRY ){
    return;
  }
  
  OS_Printf("TowerLoseScoreSet -> mode = %d\n",wk->play_mode );

  if( wk->play_mode == BTWR_MODE_WIFI_MULTI ){
    id = FRID_TOWER_MULTI_WIFI_RENSHOU;        //最大
  }else{
    id = wk->play_mode*2+FRID_TOWER_SINGLE_RENSHOU;  //最大
  }

  //現在の最大連勝数取得
  before = FrontierRecord_Get( frontier,id,Frontier_GetFriendIndex( id ));

  OS_Printf( "before = %d\n", before );
  OS_Printf( "wk->renshou = %d\n", wk->renshou );
  OS_Printf( "wk->now_win = %d\n", wk->now_win );

  //最大連勝数更新
  after = FrontierRecord_SetIfLarge( frontier, id, Frontier_GetFriendIndex( id ), 
                    wk->renshou+wk->now_win );

  //更新している||(記録が7の倍数&&前後の値が同じ)なら番組作成
  if( after > 1){
    if(  ( before < after ) ||
      (( before == after ) && ( after%7==0)) ){
      bswayscr_MakeTVRenshouMaxUpdate( wk,savedata,after );
    }
  }

  //成績モニタ用に現在の連勝数も書き出しておく
  if( wk->play_mode == BTWR_MODE_WIFI_MULTI ){
#if 0
    chg_flg = TowerScoreData_SetFlags( wk->scoreSave,
        BTWR_SFLAG_WIFI_MULTI_RECORD,BTWR_DATA_get );
#else
    chg_flg = FrontierRecord_Get( SaveData_GetFrontier( savedata ), 
                  FRID_TOWER_MULTI_WIFI_CLEAR_BIT,
                  Frontier_GetFriendIndex( FRID_TOWER_MULTI_WIFI_CLEAR_BIT ) );
#endif
  }else{
    chg_flg = TowerScoreData_SetFlags( wk->scoreSave,
        BTWR_SFLAG_SINGLE_RECORD+wk->play_mode,BTWR_DATA_get );
  }

  OS_Printf( "chg_flg = %d\n", chg_flg );
  OS_Printf( "now win = %d\n", FrontierRecord_Get( frontier,id+1,Frontier_GetFriendIndex( id+1)) );

#if 0
  if( chg_flg ){  //連勝記録挑戦中なら加算
    ret = FrontierRecord_Add( frontier, id+1, Frontier_GetFriendIndex( id+1), wk->now_win );
  }else{  //前回負けていればセット
    ret = FrontierRecord_Set( frontier, id+1, Frontier_GetFriendIndex( id+1), wk->now_win );
  }
#else
  //連勝してから、負けた時に、wk->renshouが必ず0になっていれば、
  //( renshou + now_win )をセットでいいはず(08.05.25)
  //7+7=14勝利(勝ち終わり)
  //7+3=10勝利(負け終わり)
  //0+3=3勝利(負け終わり)
  ret = FrontierRecord_Set( frontier,id+1,Frontier_GetFriendIndex( id+1),( wk->renshou+wk->now_win ));
#endif

  OS_Printf( "now win = %d\n", FrontierRecord_Get( frontier,id+1,Frontier_GetFriendIndex( id+1)) );

  //レコード挑戦中フラグを落とす
  if( wk->play_mode == BTWR_MODE_WIFI_MULTI ){
#if 0
    TowerScoreData_SetFlags( wk->scoreSave,BTWR_SFLAG_WIFI_MULTI_RECORD,
                BTWR_DATA_reset );
#else
    FrontierRecord_Set(  SaveData_GetFrontier( savedata ), 
              FRID_TOWER_MULTI_WIFI_CLEAR_BIT,
              Frontier_GetFriendIndex( FRID_TOWER_MULTI_WIFI_CLEAR_BIT ), 0 );
#endif
  }else{
    TowerScoreData_SetFlags( wk->scoreSave,BTWR_SFLAG_SINGLE_RECORD+wk->play_mode,
                BTWR_DATA_reset );
  }

  //延べ勝利数更新
  RECORD_Add( record,RECID_BSUBWAY_WIN,wk->now_win );
  //現在の周回数リセット
  TowerScoreData_SetStage( wk->scoreSave,wk->play_mode,BTWR_DATA_reset );

  //バトルタワーへのチャレンジ数追加
  if( wk->play_mode != BTWR_MODE_WIFI_MULTI ){
    RECORD_Add( SaveData_GetRecord( savedata ),RECID_BSUBWAY_CHALLENGE,1);
  }
  
  //連勝リボンをもらえるかどうかのフラグをセット
  bswayscr_IfRenshouPrizeGet( wk );

  //勝ち負け共通データ処理
  ret+=1;
  if( ret > 9999){
    ret = 9999;  
  }
  bswayscr_SetCommonScore( wk,savedata,FALSE,ret );
#endif
}

//--------------------------------------------------------------
/**
 *  @brief  クリア処理
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_SetClearScore( BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys )
{
#if 0 //wb
  u32  ret = 0;
  int  id;
  void *note;
  u16  before,after,chg_flg;
  RECORD * record;
  FRONTIER_SAVEWORK *frontier;
    
  if( wk->play_mode == BTWR_MODE_RETRY ){
    return;
  }
  OS_Printf("TowerClearScoreSet -> mode = %d\n",wk->play_mode );
  
  record = SaveData_GetRecord( savedata );
  frontier = SaveData_GetFrontier( savedata );

  if( wk->play_mode == BTWR_MODE_WIFI_MULTI ){
    id = FRID_TOWER_MULTI_WIFI_RENSHOU;
  }else{
    id = wk->play_mode*2+FRID_TOWER_SINGLE_RENSHOU;
  }
  
  //現在の連勝数を書き出し
  if( wk->play_mode == BTWR_MODE_WIFI_MULTI ){
#if 0
    chg_flg = TowerScoreData_SetFlags( wk->scoreSave,
        BTWR_SFLAG_WIFI_MULTI_RECORD,BTWR_DATA_get );
#else
    chg_flg = FrontierRecord_Get( SaveData_GetFrontier( savedata ), 
                  FRID_TOWER_MULTI_WIFI_CLEAR_BIT,
                  Frontier_GetFriendIndex( FRID_TOWER_MULTI_WIFI_CLEAR_BIT ) );
#endif
  }else{
    chg_flg = TowerScoreData_SetFlags( wk->scoreSave,
        BTWR_SFLAG_SINGLE_RECORD+wk->play_mode,BTWR_DATA_get );
  }

#if 0
  if( chg_flg ){  //現在も連勝記録挑戦中なら加算
    //ret = FrontierRecord_Add( frontier,id+1,Frontier_GetFriendIndex( id+1),BTWR_CLEAR_WINCNT );

    ret = FrontierRecord_Get(  SaveData_GetFrontier( savedata ), 
                  ( id+1),
                  Frontier_GetFriendIndex( id+1) );

    FrontierRecord_Set(  SaveData_GetFrontier( savedata ), 
              ( id+1),
              Frontier_GetFriendIndex( id+1), ( ret+BTWR_CLEAR_WINCNT ) );

    ret = FrontierRecord_Get(  SaveData_GetFrontier( savedata ), 
                  ( id+1),
                  Frontier_GetFriendIndex( id+1) );

  }else{  //前回負けてた場合は一旦リセット
    ret = FrontierRecord_Set( frontier,id+1,Frontier_GetFriendIndex( id+1),BTWR_CLEAR_WINCNT );
  }
#else
  //連勝してから、負けた時に、wk->renshouが必ず0になっていれば、
  //( renshou + now_win )をセットでいいはず(08.05.25)
  //7+7=14勝利(勝ち終わり)
  //7+3=10勝利(負け終わり)
  //0+3=3勝利(負け終わり)
  OS_Printf( "wk->renshou = %d\n", wk->renshou );
  OS_Printf( "wk->now_win = %d\n", wk->now_win );
  ret = FrontierRecord_Set( frontier,id+1,Frontier_GetFriendIndex( id+1),( wk->renshou+wk->now_win ));
#endif

  //レコード挑戦中フラグを立てる
  if( wk->play_mode == BTWR_MODE_WIFI_MULTI ){
#if 0
    TowerScoreData_SetFlags( wk->scoreSave,BTWR_SFLAG_WIFI_MULTI_RECORD,BTWR_DATA_set );
#else
    FrontierRecord_Set(  SaveData_GetFrontier( savedata ), 
              FRID_TOWER_MULTI_WIFI_CLEAR_BIT,
              Frontier_GetFriendIndex( FRID_TOWER_MULTI_WIFI_CLEAR_BIT ), 1 );
#endif
  }else{
    TowerScoreData_SetFlags( wk->scoreSave,BTWR_SFLAG_SINGLE_RECORD+wk->play_mode,BTWR_DATA_set );
  }

  //現在の最大連勝数取得
  before = FrontierRecord_Get( frontier,id,Frontier_GetFriendIndex( id ));
  //最大連勝数更新
  after = FrontierRecord_SetIfLarge( frontier,id,Frontier_GetFriendIndex( id ),ret );

  //更新してたら番組作成
#if 0
  if( before < after ){
    bswayscr_MakeTVRenshouMaxUpdate( wk,fsys->savedata,after );
  }
#endif
  //延べ勝利数更新
  RECORD_Add( record,RECID_BSUBWAY_WIN,BTWR_CLEAR_WINCNT );
  //周回数プラス
  TowerScoreData_SetStage( wk->scoreSave,wk->play_mode,BTWR_DATA_inc );

  //バトルタワーへのチャレンジ数追加
  if( wk->play_mode != BTWR_MODE_WIFI_MULTI ){
    RECORD_Add( record,RECID_BSUBWAY_CHALLENGE,1);
  }

  //スコア加算
  RECORD_Score_Add( record,SCORE_ID_BSUBWAY_7WIN );
  
  //連勝リボンをもらえるかどうかのフラグをセット
  bswayscr_IfRenshouPrizeGet( wk );

  //勝ち負け共通データ処理
  bswayscr_SetCommonScore( wk,savedata,TRUE,ret );

#if 0
  //冒険ノート(通信マルチ&wifiのみ)
  if(  wk->play_mode == BTWR_MODE_COMM_MULTI ||
    wk->play_mode == BTWR_MODE_WIFI_MULTI ||
    wk->play_mode == BTWR_MODE_WIFI ){
#else
  //冒険ノート( WIFI_DLのみ)
  if( wk->play_mode == BTWR_MODE_WIFI ){
#endif
    note = FNOTE_SioBattleTowerDataMake( wk->heapID );
    FNOTE_DataSave( fnote, note, FNOTE_TYPE_SIO );
  }
#endif
}

//--------------------------------------------------------------
/**
 *  @brief  休むときに現在のプレイ状況をセーブに書き出す
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_SaveRestPlayData( BSUBWAY_SCRWORK *wk )
{
#if 0 //wb
  u16  i;
  u8  buf8[4];

  //プレイモード書き出し
  buf8[0] = wk->play_mode;
  TowerPlayData_Put( wk->playSave,BTWR_PSD_playmode,buf8);
  
  //ラウンド数書き出し
  buf8[0] = wk->now_round;
  TowerPlayData_Put( wk->playSave,BTWR_PSD_round,buf8);

  //選んだポケモンNo
  TowerPlayData_Put( wk->playSave,BTWR_PSD_pokeno,wk->member );
  
  //バトル成績書き出し
  TowerPlayData_WifiRecordAdd( wk->playSave,wk->rec_down,wk->rec_turn,wk->rec_damage );

  //抽選されたトレーナーNo書き出し
  TowerPlayData_Put( wk->playSave,BTWR_PSD_trainer,wk->trainer );
  
  //プレイランダムシード保存
  TowerPlayData_Put( wk->playSave,BTWR_PSD_rnd_seed,&( wk->play_rnd_seed ));
  OS_Printf("TowerRestRndSeed = %d\n",wk->play_rnd_seed );
  
  //セーブフラグを有効状態にリセット
  TowerPlayData_SetSaveFlag( wk->playSave,TRUE );

  if( wk->play_mode != BTWR_MODE_MULTI ){
    return;
  }
  //AIマルチモードならパートナーを覚えておく
  buf8[0] = wk->partner;
  TowerPlayData_Put( wk->playSave,BTWR_PSD_partner,buf8);

  //パートナーのポケモンパラメータを憶えておく
  TowerPlayData_Put( wk->playSave,BTWR_PSD_pare_poke,&( wk->five_poke[wk->partner]));
  //アイテムが固定だったかどうか憶えておく
  TowerPlayData_Put( wk->playSave,BTWR_PSD_pare_itemfix,&( wk->five_item[wk->partner]));
  //プレイランダムシードを憶えておく
#endif
}

//--------------------------------------------------------------
/**
 * @brief  AIマルチペアポケモン抽選
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_ChoiceBtlSeven( BSUBWAY_SCRWORK *wk )
{
#if 0 //wb
  int i;
  
  for( i = 0;i < BTWR_FIVE_NUM;i++){
    wk->five_item[i] = ( u8)RomBattleTowerTrainerDataMake( wk,&( wk->five_data[i]),
      TOWER_FIVE_FIRST+i,wk->member_num,wk->mem_poke,wk->mem_item,&( wk->five_poke[i]),wk->heapID );
  }
#endif
}

//--------------------------------------------------------------
/*
 * @brief  対戦トレーナーOBJコード取得
 */
//--------------------------------------------------------------
#if 0 //wb
u16 BSUBWAY_SCRWORK_GetEnemyObj( BSUBWAY_SCRWORK *wk, u16 idx )
{
  //トレーナータイプからOBJコードを取得してくる
  return BtlTower_TrType2ObjCode( wk->tr_data[idx].bt_trd.tr_type );
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  戦闘呼び出し
 */
//--------------------------------------------------------------
#if 0 //wb
void BSUBWAY_SCRWORK_LocalBattleCall(
    GMEVENT_CONTROL *event,BSUBWAY_SCRWORK *wk,BOOL *win_flag )
{
  EventCmd_TowerLocalBattle( event,wk,win_flag );
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  現在のプレイモードを返す
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCRWORK_GetPlayMode( BSUBWAY_SCRWORK *wk )
{
  return wk->play_mode;
}

//--------------------------------------------------------------
/**
 *  @brief  リーダークリアフラグを取得する
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCRWORK_GetLeaderClearFlag( BSUBWAY_SCRWORK *wk )
{
  return (u16)wk->leader_f;
}

//--------------------------------------------------------------
/**
 *  @brief  タワークリア時にバトルポイントを加算する
 *  @return  新たに取得したバトルポイント
 */
//--------------------------------------------------------------
u16  BSUBWAY_SCRWORK_AddBtlPoint( BSUBWAY_SCRWORK *wk )
{
#if 0 //wb
  u16  stage;
  u16  point = 0;
  static const u8  bpoint_wifi[] = {0,
    BTLPOINT_VAL_TOWER_WIFI1,BTLPOINT_VAL_TOWER_WIFI2,BTLPOINT_VAL_TOWER_WIFI3,
    BTLPOINT_VAL_TOWER_WIFI4,BTLPOINT_VAL_TOWER_WIFI5,BTLPOINT_VAL_TOWER_WIFI6,
    BTLPOINT_VAL_TOWER_WIFI7,BTLPOINT_VAL_TOWER_WIFI8,BTLPOINT_VAL_TOWER_WIFI9,
    BTLPOINT_VAL_TOWER_WIFI10,
  };
  static const u8 bpoint_normal[] = {0,
    BTLPOINT_VAL_TOWER_STAGE1,BTLPOINT_VAL_TOWER_STAGE2,BTLPOINT_VAL_TOWER_STAGE3,
    BTLPOINT_VAL_TOWER_STAGE4,BTLPOINT_VAL_TOWER_STAGE5,BTLPOINT_VAL_TOWER_STAGE6,
    BTLPOINT_VAL_TOWER_STAGE7,
  };
  static const u8 bpoint_comm[] = {0,
    BTLPOINT_VAL_TOWER_COMM1,BTLPOINT_VAL_TOWER_COMM2,BTLPOINT_VAL_TOWER_COMM3,
    BTLPOINT_VAL_TOWER_COMM4,BTLPOINT_VAL_TOWER_COMM5,BTLPOINT_VAL_TOWER_COMM6,
    BTLPOINT_VAL_TOWER_COMM7,
  };

  if( wk->play_mode == BTWR_MODE_RETRY ){
    return 0;
  }
  if( wk->play_mode == BTWR_MODE_WIFI ){
    //ランクごと
    point = bpoint_wifi[TowerScoreData_SetWifiRank( wk->scoreSave,BTWR_DATA_get )];
  }else{

    //ワイアレスマルチ、WIFI
    //if( wk->play_mode == BTWR_MODE_COMM_MULTI ){
    if( ( wk->play_mode == BTWR_MODE_COMM_MULTI ) || ( wk->play_mode == BTWR_MODE_WIFI_MULTI ) ){

      //周回数ごと
      stage = TowerScoreData_SetStage( wk->scoreSave,wk->play_mode,BTWR_DATA_get );
      if( stage >= 7){                //この比較文はDPと合わせる
        point = BTLPOINT_VAL_TOWER_COMM8;
      }else{
        point = bpoint_comm[stage];
      }

#if 0
    //WIFI(32人データがあるので、周回数ワークが足りないため、連勝数から周回数を算出する
    }else if( wk->play_mode == BTWR_MODE_WIFI_MULTI ){

      OS_Printf( "wk->renshou = %d\n", wk->renshou );
      OS_Printf( "wk->now_win = %d\n", wk->now_win );
      stage = ( wk->renshou / 7 );
      if( stage >= 7){                //この比較文はDPと合わせる
        point = BTLPOINT_VAL_TOWER_COMM8;
      }else{
        point = bpoint_comm[stage];
      }
#endif

    //シングル、ダブル、AIマルチ
    }else{

      //周回数ごと
      stage = TowerScoreData_SetStage( wk->scoreSave,wk->play_mode,BTWR_DATA_get );
      if( wk->leader_f ){
        point = BTLPOINT_VAL_TOWER_LEADER;
      }else if( stage >= 7){
        point = BTLPOINT_VAL_TOWER_STAGE8;
      }else{
        point = bpoint_normal[stage];
      }
    }
  }
  //バトルポイントを加算する
  TowerScoreData_SetBattlePoint( wk->scoreSave,point,BTWR_DATA_add );
  return point;
}

/**
 *  @brief  シングルで連勝した時のご褒美のトロフィーをあげるフラグを立てておく
 */
u16  BSUBWAY_SCRWORK_GoodsFlagSet( BSUBWAY_SCRWORK *wk,SAVEDATA *sv )
{
  u8  id,fid;
  u16  record,goods,ret;

  record = TowerScrTools_GetRenshouRecord( sv,wk->play_mode );

  if( record < BTWR_50_RENSHOU_CNT ){
    return 0;
  }
  if( record >= BTWR_100_RENSHOU_CNT ){
    if( TowerScoreData_SetFlags( wk->scoreSave,BTWR_SFLAG_GOLD_GET,BTWR_DATA_get )){
      return 0;
    }
  }else{
    if( TowerScoreData_SetFlags( wk->scoreSave,BTWR_SFLAG_SILVER_GET,BTWR_DATA_get )){
      return 0;
    }
  }
#endif
  return 1;
}

//--------------------------------------------------------------
/**
 *  @brief  現在のWIFIランクを操作して返す
 */
//--------------------------------------------------------------
#if 0 //wb
u16 BSUBWAY_SCRWORK_SetWifiRank(
    BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys, u8 mode )
{
  u8  ct,rank;
  BTLTOWER_SCOREWORK *score = SaveData_GetTowerScoreData( sv );
  
  static const u8  btower_wifi_rankdown[] = {
   0,5,4,4,3,3,2,2,1,1,
//   0,2,1,1,1,1,1,1,1,1,
  };

  switch( mode ){
  //Get
  case 0:
    return ( u16)TowerScoreData_SetWifiRank( score,BTWR_DATA_get );
  case 1:  //Inc
    //連続敗戦フラグを落とす
    TowerScoreData_SetFlags( score,BTWR_SFLAG_WIFI_LOSE_F,BTWR_DATA_reset );
    //現在のランクを取得
    rank = TowerScoreData_SetWifiRank( score,BTWR_DATA_get );

    if( rank == 10){  
      wk->prize_f = 1;        //ランク10なら、リボンを貰える条件はクリア(08.06.01)
      return 0;  //もう上がらない
    }
#if 0
    //rank10の時も、リボンをもらえるフラグを立てる必要がある
    //ランクアップ処理は呼んではいけない
    //戻り値は、ランクアップしたか、ランクアップしていないかを返している
    //ランクアップした時しか、リボンを渡す流れに行っていないのは正しいのか？＞てつじさん
    //
    //msg_tower_56
    //「そして●さんは　ランク●に　なりました！」
    //
    //続いて、リボンを渡す流れへ
    //リボンをすでに持っているとか、全員持っているとかは、コメントを見る限りチェックされている
#endif

    //ランクアップ処理
    TowerScoreData_SetWifiRank( score,BTWR_DATA_inc );

    //ランク5以上にアップしてたらリボンがもらえる
    if( rank+1 >= 5){
      wk->prize_f = 1;
    }
    return 1;
  case 2:  //dec
    //現在の連続敗戦数をカウント
    ct = TowerScoreData_SetWifiLoseCount( score,BTWR_DATA_inc );
    rank = TowerScoreData_SetWifiRank( score,BTWR_DATA_get );

    if( rank == 1){
      return 0;
    }
    //ランク別敗戦カウントチェック
    if( ct >= btower_wifi_rankdown[rank-1] ){
      //ランクダウン
      TowerScoreData_SetWifiRank( score,BTWR_DATA_dec );
      //連続敗戦数と連続敗戦フラグをリセット
      TowerScoreData_SetWifiLoseCount( score,BTWR_DATA_reset );
      TowerScoreData_SetFlags( score,BTWR_SFLAG_WIFI_LOSE_F,BTWR_DATA_reset );
      return 1;
    }
    return 0;
  }
  return 0;
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  リーダーを倒したご褒美リボンをあげる
 */
//--------------------------------------------------------------
#if 0 //wb
u16 BSUBWAY_SCRWORK_LeaderRibbonSet( BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys )
{
  if( wk->play_mode != BTWR_MODE_SINGLE ){
    return 0;
  }
  
  //メンバーにリボンセット
  switch( wk->leader_f ){
  case 1:
    return bswayscr_PokeRibbonSet( sv,ID_PARA_sinou_battle_tower_ttwin_first,wk );
  case 2:
    return bswayscr_PokeRibbonSet( sv,ID_PARA_sinou_battle_tower_ttwin_second,wk );
  }
  return 0;
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  フラグが立っていたら連勝のご褒美リボンをあげる  
 */
//--------------------------------------------------------------
#if 0 //wb
u16 BSUBWAY_SCRWORK_RenshouRibbonSet( BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys )
{
  u8  id,fid;
  u16  record,goods,ret;
  UNDERGROUNDDATA *gSave;

  if( wk->play_mode == BTWR_MODE_RETRY ){
    return 0;
  }

  /////////////////////////////////////////
  //とりあえず仕様が決まっていないので保留
  /////////////////////////////////////////
  if( wk->play_mode == BTWR_MODE_WIFI_MULTI ){
    return 0;
  }

  if(!wk->prize_f ){
    return 0;
  }
  switch( wk->play_mode ){
  case BTWR_MODE_DOUBLE:
    id = ID_PARA_sinou_battle_tower_2vs2_win50;
    break;
  case BTWR_MODE_MULTI:
    id = ID_PARA_sinou_battle_tower_aimulti_win50;
    break;
  case BTWR_MODE_COMM_MULTI:
    id = ID_PARA_sinou_battle_tower_siomulti_win50;
    break;
  case BTWR_MODE_WIFI:
    id = ID_PARA_sinou_battle_tower_wifi_rank5;
    break;
  }
  //メンバーにリボンセット
  return bswayscr_PokeRibbonSet( sv,id,wk );
}
#endif

//--------------------------------------------------------------
/**
 *  @biref  プレイランダムシードを更新する
 *  d31r0201.ev
 *  case BTWR_SUB_UPDATE_RANDOM で呼ばれる
 *  ここは、WIFIマルチは通過しないので、このままでよい
 */
//--------------------------------------------------------------
#if 0 //wb
u16 BSUBWAY_SCRWORK_PlayRandUpdate( BSUBWAY_SCRWORK *wk,SAVEDATA *sv )
{
  u8  chg_flg;

  //現在チャレンジ継続中かどうか？
  if( wk->play_mode == BTWR_MODE_WIFI_MULTI ){
#if 0
    chg_flg = TowerScoreData_SetFlags( wk->scoreSave,
        BTWR_SFLAG_WIFI_MULTI_RECORD,BTWR_DATA_get );
#else
    chg_flg = FrontierRecord_Get( SaveData_GetFrontier( sv ), 
                  FRID_TOWER_MULTI_WIFI_CLEAR_BIT,
                  Frontier_GetFriendIndex( FRID_TOWER_MULTI_WIFI_CLEAR_BIT ) );
#endif
  }else{
    chg_flg = TowerScoreData_SetFlags( wk->scoreSave,
        BTWR_SFLAG_SINGLE_RECORD+wk->play_mode,BTWR_DATA_get );
  }
  
  if(!chg_flg ){
    //プレイ更新ランダムシード初期値取得&日付更新シード更新
    wk->play_rnd_seed = BtlTower_UpdatePlayRndSeed( sv );
  }else{
    //記録挑戦開始時のdayシードを基点に、現在のplayシードを取得する
    wk->play_rnd_seed = BtlTower_GetContinuePlayRndSeed( sv );
  }
  return ( wk->play_rnd_seed/65535);
}
#endif

//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 *  @brief  ポケモンにリボンをつける　サブ
 */
//--------------------------------------------------------------
#if 0 //wb
static u16 bswayscr_PokeRibbonSet( SAVEDATA *sv,u8 ribbon,BSUBWAY_SCRWORK *wk )
{
  u8  flag = 1;
  u8 ct;
  int i;
  POKEPARTY  *party;
  POKEMON_PARAM *pp;
  
  party = SaveData_GetTemotiPokemon( sv );

  ct = 0;
  for( i = 0;i < wk->member_num;i++){
    pp = PokeParty_GetMemberPointer( party,wk->member[i]);
    if( PokeParaGet( pp,ribbon,NULL )){
      continue;  //既に持ってる
    }

    PokeParaPut( pp,ribbon,&flag );

    //TVトピック作成：リボンコレクター
    TVTOPIC_Entry_Record_Ribbon( sv, pp, ribbon );

    ++ct;
  }
  if( ct == 0){
    //みんなもう持ってた
    return FALSE;
  }
  return TRUE;
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  連勝のご褒美リボンをもらえるかどうかのフラグを立てておく
 */
//--------------------------------------------------------------
#if 0
static u16 bswayscr_IfRenshouPrizeGet( BSUBWAY_SCRWORK *wk )
{
  u16 win;
  if(  wk->play_mode == BTWR_MODE_RETRY ||
    wk->play_mode == BTWR_MODE_SINGLE ||
    wk->play_mode == BTWR_MODE_WIFI_MULTI ||
    wk->play_mode == BTWR_MODE_WIFI ){
    return 0;
  }
  win = wk->renshou+wk->now_win;

  //50連勝以上でもらえる
  if( win < BTWR_50_RENSHOU_CNT ){
    return 0;
  }
  //prize getフラグを立てておく
  wk->prize_f = 1;
  return 1;
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  参加したポケモンのパラメータをB_TOWER_POKEMON型にパックする
 */
//--------------------------------------------------------------
#if 0
static void bswayscr_PokeDataPack( B_TOWER_POKEMON *dat,POKEMON_PARAM *pp )
{
  int i;
  
  //mons_no/form_no/item_no
  dat->mons_no = PokeParaGet( pp,ID_PARA_monsno,NULL );
  dat->form_no = PokeParaGet( pp,ID_PARA_form_no,NULL );
  dat->item_no = PokeParaGet( pp,ID_PARA_item,NULL );

  //waza/pp_count
  for( i = 0;i < WAZA_TEMOTI_MAX;i++){
    dat->waza[i] = PokeParaGet( pp,ID_PARA_waza1+i,NULL );
    dat->pp_count |= (( PokeParaGet( pp,ID_PARA_pp_count1+i,NULL )) << ( i*2));
  }
  //country,id,personal
  dat->country_code = PokeParaGet( pp,ID_PARA_country_code,NULL );
  dat->id_no = PokeParaGet( pp,ID_PARA_id_no,NULL );
  dat->personal_rnd = PokeParaGet( pp,ID_PARA_personal_rnd,NULL );

  //power_rnd
  dat->power_rnd = PokeParaGet( pp,ID_PARA_power_rnd,NULL );

  //exp
  for( i = 0;i < 6;i++){
    dat->exp[i] = PokeParaGet( pp,ID_PARA_hp_exp+i,NULL );
  }
  //tokusei,natukido
  dat->tokusei = PokeParaGet( pp,ID_PARA_speabino,NULL );
  dat->natuki = PokeParaGet( pp,ID_PARA_friend,NULL );
  
  //ニックネーム
  PokeParaGet( pp,ID_PARA_nickname,dat->nickname );
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  参加したポケモンのパラメータを保存する
 */
//--------------------------------------------------------------
#if 0
static void bswayscr_SaveMemberPokeData( BSUBWAY_SCRWORK *wk,SAVEDATA *sv,BTWR_SCORE_POKE_DATA mode )
{
  int i = 0;
  B_TOWER_POKEMON *dat;
  POKEPARTY  *party;
  POKEMON_PARAM *pp;
  
  dat = sys_AllocMemoryLo( wk->heapID,sizeof( B_TOWER_POKEMON )*3);
  MI_CpuClear8( dat,sizeof( B_TOWER_POKEMON )*3);

  party = SaveData_GetTemotiPokemon( sv );
  for( i = 0;i < 3;i++){
    pp = PokeParty_GetMemberPointer( party,wk->member[i]);
    bswayscr_PokeDataPack(&( dat[i]),pp );  
  }
  TowerScoreData_SetUsePokeData( wk->scoreSave,mode,dat );

  MI_CpuClear8( dat,sizeof( B_TOWER_POKEMON )*3);
  sys_FreeMemoryEz( dat );
}
#endif

//  フロンティアとフィールドで共通で使用するものを移動
//--------------------------------------------------------------
/**
 * バトルタワートレーナーの持ちポケモンのパワー乱数を決定する
 * @param  tr_no  トレーナーナンバー
 * @return  パワー乱数
 * b_tower_fld.c→b_tower_evに移動
 */
//--------------------------------------------------------------
#if 0
u8  BattleTowerPowRndGet( u16 tr_no )
{
  u8  pow_rnd;

  if( tr_no<100){
    pow_rnd=(0x1f/8)*1;
  }
  else if( tr_no<120){
    pow_rnd=(0x1f/8)*2;
  }
  else if( tr_no<140){
    pow_rnd=(0x1f/8)*3;
  }
  else if( tr_no<160){
    pow_rnd=(0x1f/8)*4;
  }
  else if( tr_no<180){
    pow_rnd=(0x1f/8)*5;
  }
  else if( tr_no<200){
    pow_rnd=(0x1f/8)*6;
  }
  else if( tr_no<220){
    pow_rnd=(0x1f/8)*7;
  }
  else{
    pow_rnd=0x1f;
  }
  return pow_rnd;
}
#endif

//--------------------------------------------------------------
/**
 * @brief  タワープレイ更新ランダムシード取得 ラウンド更新時に呼ぶ
 * b_tower_fld.c→b_tower_evに移動
 */
//--------------------------------------------------------------
#if 0
u16  btower_rand( BSUBWAY_SCRWORK *wk )
{
  //プラチナで追加されたWIFIマルチは通常のランダムを使用
  if( wk->play_mode == BTWR_MODE_WIFI_MULTI ){
    return ( gf_rand() );
  }

  wk->play_rnd_seed = BtlTower_PlayFixRand( wk->play_rnd_seed );

  OS_Printf("btower_rand = %d\n",wk->play_rnd_seed );
  return ( wk->play_rnd_seed/65535);
}
#endif
