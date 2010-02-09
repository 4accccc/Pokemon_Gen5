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
#include "savedata/record.h"

#include "system/main.h"  //HEAPID_PROC

#include "poke_tool/poke_tool.h"
#include "app/pokelist.h"

#include "savedata/bsubway_savedata.h"
#include "savedata/bsubway_savedata_def.h"
#include "battle/bsubway_battle_data.h"
#include "bsubway_scr_def.h"
#include "bsubway_scr.h"
#include "bsubway_scrwork.h"
#include "bsubway_tr.h"

//======================================================================
//  define
//======================================================================
#ifdef DEBUG_ONLY_FOR_kagaya
#define DEBUG_BSW_CLEAR_1 //定義で1回戦闘のみでクリア
#endif

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================
static u8 get_MemberNum( u16 mode );
static BOOL is_ConflictTrainer( u16* trainer,u16 id,u16 num );

//======================================================================
//  バトルサブウェイ　スクリプトワーク関連
//======================================================================
//--------------------------------------------------------------
/**
 * BSUBWAY_SCRWORK ワークポインタをクリア
 * @param bsw_scr BSUBWAY_SCRWORK*
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_ClearWork( GAMESYS_WORK *gsys )
{
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  GAMEDATA_SetBSubwayScrWork( gdata, NULL );
}

//--------------------------------------------------------------
/**
 * BSUBWAY_SCRWORK ワークエリアを取得して初期化する
 * @param gsys GAMESYS_WORK*
 * @param init 初期化モード BSWAY_PLAY_NEW:始め、BSWAY_PLAY_CONTINE:続き
 * @param playmode プレイモード指定:BSWAY_MODE_～*
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
  
  bsw_scr->gdata = gdata;
  bsw_scr->my_sex = MyStatus_GetMySex( mystatus );
  
  bsw_scr->playData = SaveControl_DataPtrGet(
      save, GMDATA_ID_BSUBWAY_PLAYDATA );
  
  bsw_scr->scoreData = SaveControl_DataPtrGet(
      save, GMDATA_ID_BSUBWAY_SCOREDATA );

  BSUBWAY_PLAYDATA_SetSaveFlag( bsw_scr->playData, FALSE ); //セーブなしに
  
  GF_ASSERT( GAMEDATA_GetBSubwayScrWork(gdata) == NULL );
  GF_ASSERT( bsw_scr->playData != NULL );
  GF_ASSERT( bsw_scr->scoreData != NULL );
  
  GAMEDATA_SetBSubwayScrWork( gdata, bsw_scr );
  
  if( init == BSWAY_PLAY_NEW ){ //新規
    u8 buf8;
    
    bsw_scr->play_mode = playmode;
    bsw_scr->member_num = get_MemberNum( bsw_scr->play_mode );
    
    for(i = 0;i < BSUBWAY_STOCK_MEMBER_MAX;i++){
      bsw_scr->member[i] = BSWAY_NULL_POKE;
    }
    
    for(i = 0;i < BSUBWAY_STOCK_TRAINER_MAX;i++){
      bsw_scr->trainer[i] = BSWAY_NULL_TRAINER;
    }
    
    BSUBWAY_PLAYDATA_Init( bsw_scr->playData ); //プレイデータ初期化
    BSUBWAY_PLAYDATA_ResetRoundNo( bsw_scr->playData );
    
    buf8 = bsw_scr->play_mode; //プレイモード
    
    BSUBWAY_PLAYDATA_SetData(
        bsw_scr->playData, BSWAY_PLAYDATA_ID_playmode, &buf8 );
  }else{ //続き
    bsw_scr->play_mode = (u8)BSUBWAY_PLAYDATA_GetData(
        bsw_scr->playData, BSWAY_PLAYDATA_ID_playmode, NULL );
    bsw_scr->member_num = get_MemberNum( bsw_scr->play_mode );
    
    //選ばれているポケモンNo
    BSUBWAY_PLAYDATA_GetData( bsw_scr->playData,
        BSWAY_PLAYDATA_ID_pokeno, bsw_scr->member );
    
    //抽選済みのトレーナーNo
    BSUBWAY_PLAYDATA_GetData( bsw_scr->playData,
        BSWAY_PLAYDATA_ID_trainer, bsw_scr->trainer);
    
    if( bsw_scr->play_mode == BSWAY_MODE_MULTI ){
      bsw_scr->partner = (u8)BSUBWAY_PLAYDATA_GetData(
          bsw_scr->playData, BSWAY_PLAYDATA_ID_partner, NULL );
      
      //パートナートレーナーデータ再生成
      BSUBWAY_PLAYDATA_GetData( bsw_scr->playData,
          BSWAY_PLAYDATA_ID_pare_poke,
          &(bsw_scr->five_poke[bsw_scr->partner]) );
      
      BSUBWAY_SCRWORK_MakePartnerRomData(
          bsw_scr, &bsw_scr->five_data[bsw_scr->partner],
        BSUBWAY_FIVE_FIRST + bsw_scr->partner,
        BSUBWAY_PLAYDATA_GetData(bsw_scr->playData,
          BSWAY_PLAYDATA_ID_pare_itemfix, NULL ),
        &(bsw_scr->five_poke[bsw_scr->partner]),
        bsw_scr->heapID );
    }
  }
  
#if 0 //wb null
  //WIFI(32人データがあるので、周回数ワークが足りないため、
  //連勝数から周回数を算出してセット
  if(wk->play_mode == BSWAY_MODE_WIFI_MULTI){
    wk->stage = TowerScoreData_SetStageValue(
        wk->scoreSave, BSWAY_MODE_WIFI_MULTI, 
        (wk->renshou / 7) );
    OS_Printf( "** wk->renshou = %d\n", wk->renshou );
    OS_Printf( "** stage = %d\n", TowerScoreData_SetStage(wk->scoreSave,
                              BSWAY_MODE_WIFI_MULTI,BSWAY_DATA_get) );
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
void BSUBWAY_SCRWORK_ReleaseWork(
    GAMESYS_WORK *gsys, BSUBWAY_SCRWORK *bsw_scr )
{
  if( bsw_scr != NULL ){
    GF_ASSERT( bsw_scr->magicNo == BSUBWAY_SCRWORK_MAGIC );
    MI_CpuClear8( bsw_scr, sizeof(BSUBWAY_SCRWORK) );
    GFL_HEAP_FreeMemory( bsw_scr );
  }
  
  {
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
    GAMEDATA_SetBSubwayScrWork( gdata, NULL );
  }
}

//--------------------------------------------------------------
/**
 * BSUBWAY_SCRWORK　通信マルチモードへ変更する
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_ChangeCommMultiMode( BSUBWAY_SCRWORK *bsw_scr )
{
}

//--------------------------------------------------------------
/**
 * BSUBWAY_SCRWORK 休むときに現在のプレイ状況をセーブに書き出す
 * @param bsw_scr BSUBWAY_SCRWORK*
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_SaveRestPlayData( BSUBWAY_SCRWORK *bsw_scr )
{
  u16 i;
  u8 buf8[4];
  
  //プレイモード書き出し
  buf8[0] = bsw_scr->play_mode;
  BSUBWAY_PLAYDATA_SetData( bsw_scr->playData,
      BSWAY_PLAYDATA_ID_playmode, buf8 );
  
  //選んだポケモンNo
  BSUBWAY_PLAYDATA_SetData( bsw_scr->playData,
      BSWAY_PLAYDATA_ID_pokeno, bsw_scr->member );
  
  //バトル成績書き出し
  BSUBWAY_PLAYDATA_AddWifiRecord( bsw_scr->playData,
    bsw_scr->rec_down, bsw_scr->rec_turn, bsw_scr->rec_damage );
  
  //抽選されたトレーナーNo書き出し
  BSUBWAY_PLAYDATA_SetData( bsw_scr->playData,
      BSWAY_PLAYDATA_ID_trainer, bsw_scr->trainer );
  
  //セーブフラグを有効状態にリセット
  BSUBWAY_PLAYDATA_SetSaveFlag( bsw_scr->playData, TRUE );
  
  //AIマルチモードならパートナーを覚えておく
  if( bsw_scr->play_mode == BSWAY_MODE_COMM_MULTI ){
    buf8[0] = bsw_scr->partner;
    BSUBWAY_PLAYDATA_SetData(
        bsw_scr->playData, BSWAY_PLAYDATA_ID_partner, buf8 );
    BSUBWAY_PLAYDATA_SetData( bsw_scr->playData,
        BSWAY_PLAYDATA_ID_pare_poke,
        &(bsw_scr->five_poke[bsw_scr->partner]) );
    
    //アイテムが固定だったかどうか憶えておく
    #if 0 //wb null
    BSUBWAY_PLAYDATA_SetData( bsw_scr->playData,
        BSWAY_PLAYDATA_ID_pare_itemfix,
        &(bsw_scr->five_item[bsw_scr->partner]) );
    #endif
  }
}

//--------------------------------------------------------------
/**
 * BSUBWAY_SCRWORK ポケモンメンバーをロード
 * @param bsw_scr BSUBWAY_SCRWORK*
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_LoadPokemonMember(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys )
{
  int i;
  POKEPARTY *party;
  POKEMON_PARAM *pp;
  
  bsw_scr->member_num = get_MemberNum( bsw_scr->play_mode );
  
  BSUBWAY_PLAYDATA_GetData( bsw_scr->playData, //選んだポケモンNo
      BSWAY_PLAYDATA_ID_pokeno, bsw_scr->member );
  
  party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(gsys) );
  
  for( i = 0; i < bsw_scr->member_num; i++ ){ //選択ポケモン手持ちNo
    pp = PokeParty_GetMemberPointer( party, bsw_scr->member[i] );
    bsw_scr->mem_poke[i] = PP_Get( pp, ID_PARA_monsno, NULL );  
    bsw_scr->mem_item[i] = PP_Get( pp, ID_PARA_item, NULL );  
  }
}

//--------------------------------------------------------------
/**
 * BSUBWAY_SCRWORK ゲームクリア時に現在のプレイ状況をセーブに書き出す
 * @param bsw_scr BSUBWAY_SCRWORK*
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_SaveGameClearPlayData( BSUBWAY_SCRWORK *bsw_scr )
{
  u16  i;
  u8  buf8[4];
  
  //プレイモード書き出し
  buf8[0] = bsw_scr->play_mode;
  BSUBWAY_PLAYDATA_SetData( bsw_scr->playData,
      BSWAY_PLAYDATA_ID_playmode, buf8 );
  
  //ラウンド数リセット
  BSUBWAY_PLAYDATA_ResetRoundNo( bsw_scr->playData );
  
  //選んだポケモンNo
  BSUBWAY_PLAYDATA_SetData( bsw_scr->playData,
      BSWAY_PLAYDATA_ID_pokeno, bsw_scr->member );
  
  //バトル成績書き出し
  BSUBWAY_PLAYDATA_AddWifiRecord( bsw_scr->playData,
    bsw_scr->rec_down, bsw_scr->rec_turn, bsw_scr->rec_damage );
  
  //セーブフラグを有効状態にリセット
  BSUBWAY_PLAYDATA_SetSaveFlag( bsw_scr->playData, TRUE );
  
  if( bsw_scr->play_mode == BSWAY_MODE_MULTI ){
    //AIマルチモードならパートナーを覚えておく
    buf8[0] = bsw_scr->partner;
    BSUBWAY_PLAYDATA_SetData(
        bsw_scr->playData, BSWAY_PLAYDATA_ID_partner, buf8 );
  
    //パートナーのポケモンパラメータを憶えておく
    BSUBWAY_PLAYDATA_SetData( bsw_scr->playData,
        BSWAY_PLAYDATA_ID_pare_poke,
        &(bsw_scr->five_poke[bsw_scr->partner]) );
    
    //アイテムが固定だったかどうか憶えておく
    BSUBWAY_PLAYDATA_SetData( bsw_scr->playData,
        BSWAY_PLAYDATA_ID_pare_itemfix,
        &(bsw_scr->five_item[bsw_scr->partner]) );
  }
}

//--------------------------------------------------------------
/**
 * エラー時のワークセット
 * @param GAMESYS_WORK *gsys
 * @retval u16 プレイしていたモード
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCRWORK_SetNGScore( GAMESYS_WORK *gsys )
{
  int id;
  u8  play_mode;
  BSUBWAY_PLAYDATA *playData;
  BSUBWAY_SCOREDATA *scoreData;
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
  
  //セーブデータ取得
  playData = SaveControl_DataPtrGet( save, GMDATA_ID_BSUBWAY_PLAYDATA );
  scoreData = SaveControl_DataPtrGet( save, GMDATA_ID_BSUBWAY_SCOREDATA );

  //どのモードをプレイしていたか？
  play_mode = BSUBWAY_PLAYDATA_GetData(
      playData, BSWAY_PLAYDATA_ID_playmode, NULL );
  
  //現在の周回数リセット
  BSUBWAY_PLAYDATA_ResetRoundNo( playData );
  BSUBWAY_SCOREDATA_ResetStageNo( scoreData, play_mode );
  
  //レコード挑戦中フラグを落とす
  BSUBWAY_SCOREDATA_SetFlag( scoreData,
    BSWAY_SCOREDATA_FLAG_SINGLE_RECORD + play_mode,
    BSWAY_SETMODE_reset );
  
  //連勝記録クリア
  BSUBWAY_SCOREDATA_ResetRenshou( scoreData, play_mode );
  
  return play_mode;  
}

//--------------------------------------------------------------
/**
 * 敗戦時のワークセット
 * @param GAMESYS_WORK *gsys
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_SetLoseScore(
    GAMESYS_WORK *gsys, BSUBWAY_SCRWORK *bsw_scr )
{
  u16 play_mode = bsw_scr->play_mode;
  BSUBWAY_SCOREDATA *scoreData = bsw_scr->scoreData;
  
  //連勝記録更新
  BSUBWAY_SCOREDATA_UpdateRenshouMax( scoreData, play_mode,
      BSUBWAY_SCOREDATA_GetRenshou(scoreData,play_mode) );
  BSUBWAY_SCOREDATA_ResetRenshou( scoreData, play_mode );
  
  //周回数リセット
  BSUBWAY_PLAYDATA_ResetRoundNo( bsw_scr->playData );
  BSUBWAY_SCOREDATA_ResetStageNo( bsw_scr->scoreData, play_mode );
}

#if 0 //wb null
void BSUBWAY_SCRWORK_SetLoseScore(
    GAMESYS_WORK *gsys, BSUBWAY_SCRWORK *bsw_scr )
{
  u32  ret = 0;
  int  id;
  u16  before,after,chg_flg;
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK *save = GAMEDATA_GetSaveControlWork( gdata );
  RECORD *record = SaveControl_DataPtrGet( save, GMDATA_ID_RECORD );
  
  OS_Printf( "TowerLoseScoreSet -> mode = %d\n", bsw_scr->play_mode );
  
  id = bsw_scr->play_mode * 2 + RECID_RENSHOU_SINGLE;
  
  BSUBWAY_SCOREDATA_ResetRenshou( bsw_scr->scoreData, bsw_scr->play_mode );
  
  BSUBWAY_SCOREDATA_SetRenshou(
     bsw_scr->scoreData, BSWAY_SETMODE_reset );

  //現在の最大連勝数取得
  before = RECORD_Get( record, id );
  
  //最大連勝数更新
  after = RECORD_SetIfLarge(record,id,wk->renshou+wk->now_win);

  //更新している||(記録が7の倍数&&前後の値が同じ)なら番組作成
  if(after > 1){
    if(  (before < after) ||
      ((before == after) && (after%7==0)) ){
      towerscr_MakeTVRenshouMaxUpdate(wk,fsys,after);
    }
  }
  //成績モニタ用に現在の連勝数も書き出しておく
  chg_flg = TowerScoreData_SetFlags(wk->scoreSave,
      BSWAY_SFLAG_SINGLE_RECORD+wk->play_mode,BSWAY_DATA_get);
  if(chg_flg){  //連勝記録挑戦中なら加算
    ret = RECORD_Add(record,id+1,wk->now_win);
  }else{  //前回負けていればセット
    ret = RECORD_Set(record,id+1,wk->now_win);
  }

  //レコード挑戦中フラグを落とす
  TowerScoreData_SetFlags(
      wk->scoreSave,
      BSWAY_SFLAG_SINGLE_RECORD+wk->play_mode,BSWAY_DATA_reset);
  
  //延べ勝利数更新
  RECORD_Add(record,RECID_BTOWER_WIN,wk->now_win);

  //現在の周回数リセット
  BSUBWAY_SCOREDATA_ResetStageCount(
      bsw_scr->scoreData, bsw_scr->play_mode );

  #if 0  //wb null
  //バトルタワーへのチャレンジ数追加
  RECORD_Add(SaveData_GetRecord(savedata),RECID_BTOWER_CHALLENGE,1);
  
  //連勝リボンをもらえるかどうかのフラグをセット
  towerscr_IfRenshouPrizeGet(wk);
  #endif

  //勝ち負け共通データ処理
  ret+=1;
  
  if(ret > 9999){
    ret = 9999;  
  }

  towerscr_SetCommonScore(wk,savedata,FALSE,ret);
}
#endif

#if 0
static void towerscr_SetCommonScore(BTOWER_SCRWORK* wk,SAVEDATA* sv,u8 win_f,u16 now_renshou)
{
	u8	buf8;

	switch(wk->play_mode){
	case BSWAY_MODE_SINGLE:
		//ポケモンデータセット
		towerscr_SaveMemberPokeData(wk,sv,BTWR_SCORE_POKE_SINGLE);
	case BTWR_MODE_DOUBLE:
		if(now_renshou >= 7){
			//TVインタビューデータセット(シングルとダブルで実行)
			TVTOPIC_BTowerTemp_Set(SaveData_GetTvWork(sv),win_f,now_renshou);
		}
		break;
	case BTWR_MODE_WIFI:
		//ポケモンデータセット
		towerscr_SaveMemberPokeData(wk,sv,BTWR_SCORE_POKE_WIFI);
		//スコア押し出し
		TowerPlayData_WifiRecordAdd(wk->playSave,wk->rec_down,wk->rec_turn,wk->rec_damage);

		//プレイモード書き出し
		buf8 = wk->play_mode;
		TowerPlayData_Put(wk->playSave,BTWR_PSD_playmode,&buf8);
		//ラウンド数書き出し
		buf8 = wk->now_round;
		TowerPlayData_Put(wk->playSave,BTWR_PSD_round,&buf8);
		
		TowerScoreData_SetWifiScore(wk->scoreSave,wk->playSave);
		break;
	default:
		break;
	}
}
#endif

//--------------------------------------------------------------
/**
 * バトルポイント追加
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
u16  BSUBWAY_SCRWORK_AddBattlePoint( BSUBWAY_SCRWORK *bsw_scr )
{
  u16  point = 0;
  static const u8  bpoint_wifi[] = {0,
    BTLPOINT_VAL_BSUBWAY_WIFI1,BTLPOINT_VAL_BSUBWAY_WIFI2,
    BTLPOINT_VAL_BSUBWAY_WIFI3,BTLPOINT_VAL_BSUBWAY_WIFI4,
    BTLPOINT_VAL_BSUBWAY_WIFI5,BTLPOINT_VAL_BSUBWAY_WIFI6,
    BTLPOINT_VAL_BSUBWAY_WIFI7,BTLPOINT_VAL_BSUBWAY_WIFI8,
    BTLPOINT_VAL_BSUBWAY_WIFI9,BTLPOINT_VAL_BSUBWAY_WIFI10,
  };
  static const u8 bpoint_normal[] = {0,
    BTLPOINT_VAL_BSUBWAY_STAGE1,BTLPOINT_VAL_BSUBWAY_STAGE2,
    BTLPOINT_VAL_BSUBWAY_STAGE3,BTLPOINT_VAL_BSUBWAY_STAGE4,
    BTLPOINT_VAL_BSUBWAY_STAGE5,BTLPOINT_VAL_BSUBWAY_STAGE6,
    BTLPOINT_VAL_BSUBWAY_STAGE7,
  };
  
  if( bsw_scr->play_mode == BSWAY_MODE_WIFI ){ //ランクごと
    u8 rank = BSUBWAY_SCOREDATA_GetWifiRank( bsw_scr->scoreData );
    point = bpoint_wifi[rank];
  }else{ //周回数ごと
    u16 stage = BSUBWAY_SCOREDATA_GetStageNo(
        bsw_scr->scoreData, bsw_scr->play_mode );
    
    if( bsw_scr->boss_f ){
      point = BTLPOINT_VAL_BSUBWAY_LEADER;
    }else if( stage >= 7 ){ //７戦以上
      point = BTLPOINT_VAL_BSUBWAY_STAGE8;
    }else{
      point = bpoint_normal[stage];
    }
  }
  
  if( point == 0 ){
    point = 1;
  }
  
  BSUBWAY_SCOREDATA_AddBattlePoint( bsw_scr->scoreData, point );
  return point;
}

//--------------------------------------------------------------
/**
 * トレーナーナンバーをセット
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_SetBtlTrainerNo( BSUBWAY_SCRWORK *bsw_scr )
{
  int i;
  u8 play_mode;
  u16 no,stage,round;
  
  play_mode = bsw_scr->play_mode;
  stage = BSUBWAY_SCOREDATA_GetStageNo( bsw_scr->scoreData, play_mode );
  
  if( play_mode == BSWAY_MODE_MULTI ||
      play_mode == BSWAY_MODE_COMM_MULTI ||
      play_mode == BSWAY_MODE_S_MULTI ||
      play_mode == BSWAY_MODE_S_COMM_MULTI )
  {
    if( stage < bsw_scr->pare_stage_no )
    {
      stage = bsw_scr->pare_stage_no;  //通信時には周回数の多いほうで抽選
    }
    
    for( i = 0; i < BSUBWAY_STOCK_TRAINER_MAX; i++ )
    {
      do
      {
        no = BSUBWAY_SCRWORK_GetTrainerNo( bsw_scr, stage, i/2, play_mode );
      }while( is_ConflictTrainer(bsw_scr->trainer,no,i) );
      
      bsw_scr->trainer[i] = no;
    }
  }
  else
  {
    for( i = 0; i < (BSUBWAY_STOCK_TRAINER_MAX/2); i++ )
    {
      do
      {
        no = BSUBWAY_SCRWORK_GetTrainerNo(
            bsw_scr, stage, i, bsw_scr->play_mode );
      }while( is_ConflictTrainer(bsw_scr->trainer,no,i) );
      
      bsw_scr->trainer[i] = no;
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief  選択したポケモンを取得
 * @param wk BSUBWAY_SCRWORK
 * @param gsys GAMESYS_WORK
 * @retval  BOOL TRUE  選択した FALSE  選択せずにやめた
 */
//--------------------------------------------------------------
BOOL BSUBWAY_SCRWORK_GetEntryPoke( BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys )
{
  if( bsw_scr->pokelist_return_mode != PL_RET_NORMAL ||
      bsw_scr->pokelist_result_select == PL_SEL_POS_EXIT ||
      bsw_scr->pokelist_result_select == PL_SEL_POS_EXIT2 )
  {
    return FALSE;
  }
  else
  {
    u16  i = 0;
    POKEPARTY *party;
    POKEMON_PARAM *pp;

    party = GAMEDATA_GetMyPokemon( GAMESYSTEM_GetGameData(gsys) );
  
    for( i = 0; i < bsw_scr->member_num; i++ ){ //ポケモン選択の手持ちNo
      if( (bsw_scr->pokelist_select_num[i]-1) >= 6 ){
        GF_ASSERT( 0 );
        bsw_scr->pokelist_select_num[i] = 1;
      }
    
      bsw_scr->member[i] = bsw_scr->pokelist_select_num[i] - 1;
      pp = PokeParty_GetMemberPointer( party, bsw_scr->member[i] );
      bsw_scr->mem_poke[i] = PP_Get( pp, ID_PARA_monsno, NULL );  
      bsw_scr->mem_item[i] = PP_Get( pp, ID_PARA_item, NULL );  
    }
    
    return TRUE;
  }
}

//--------------------------------------------------------------
/**
 *  @brief  7連勝しているかどうかチェック
 *  @param bsw_scr BSUBWAY_SCRWORK
 *  @retval BOOL TRUE=７連勝している
 */
//--------------------------------------------------------------
BOOL BSUBWAY_SCRWORK_IsClear( BSUBWAY_SCRWORK *bsw_scr )
{
  if( bsw_scr->clear_f == FALSE ){
    u32 round = BSUBWAY_PLAYDATA_GetRoundNo( bsw_scr->playData );

#ifdef DEBUG_BSW_CLEAR_1
    if( round < 1 ){
      return( FALSE );
    }
#else
    if( round < BSWAY_CLEAR_WINCNT ){
      return( FALSE );
    }
#endif
    
    bsw_scr->clear_f = 1; //クリアフラグon
  }
  
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * @brief  クリア処理
 * @param bsw_scr BSUBWAY_SCRWORK
 * @param gsys GAMESYS_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_SetClearScore(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys )
{
  u32 ret = 0;
  u16 before,after,chg_flg,play_mode;
  play_mode = bsw_scr->play_mode;

  //最大連勝数更新
  BSUBWAY_SCOREDATA_UpdateRenshouMax(
      bsw_scr->scoreData, play_mode,
      BSUBWAY_SCOREDATA_GetRenshou(bsw_scr->scoreData,play_mode) );
  
  //周回数プラス
  BSUBWAY_SCOREDATA_IncStageNo( bsw_scr->scoreData, play_mode );
}

//--------------------------------------------------------------
/**
 * 対戦トレーナー抽選
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_ChoiceBattlePartner( BSUBWAY_SCRWORK *bsw_scr )
{
  int i;
  u16 round;
  u16 monsno[2];
  u16 itemno[2];
   
  round = BSUBWAY_PLAYDATA_GetRoundNo( bsw_scr->playData );
  
  switch( bsw_scr->play_mode ){
  case BSWAY_MODE_WIFI:
    #if 0 //wb null
    btltower_BtlPartnerSelectWifi(sv,wk->tr_data,wk->now_round-1);
    #else
    GF_ASSERT( 0 );
    #endif
    break;
  case BSWAY_MODE_MULTI:
  case BSWAY_MODE_S_MULTI:
  case BSWAY_MODE_COMM_MULTI:
  case BSWAY_MODE_S_COMM_MULTI:
    BSUBWAY_SCRWORK_MakeRomTrainerData(
        bsw_scr, &(bsw_scr->tr_data[0]),
        bsw_scr->trainer[round*2+0],
        bsw_scr->member_num, NULL, NULL, NULL, bsw_scr->heapID );
    
    for( i = 0; i < bsw_scr->member_num; i++ ){ //モンスターNo重複チェック
      monsno[i] = bsw_scr->tr_data[0].btpwd[i].mons_no;
      itemno[i] = bsw_scr->tr_data[0].btpwd[i].item_no;
    }
    
    BSUBWAY_SCRWORK_MakeRomTrainerData(
        bsw_scr, &(bsw_scr->tr_data[1]),
        bsw_scr->trainer[round*2+1],
        bsw_scr->member_num, monsno, itemno, NULL, bsw_scr->heapID );
    break;
  case BSWAY_MODE_SINGLE:
  case BSWAY_MODE_DOUBLE:
  case BSWAY_MODE_S_SINGLE:
  case BSWAY_MODE_S_DOUBLE:
  default:
    BSUBWAY_SCRWORK_MakeRomTrainerData(
      bsw_scr, &(bsw_scr->tr_data[0]),
      bsw_scr->trainer[round],
      bsw_scr->member_num, NULL, NULL, NULL, bsw_scr->heapID );
    break;
  }
}

//--------------------------------------------------------------
/**
 * トレーナータイプからOBJコードを取得してくる
 * @param bsw_scr BSUBWAY_SCR
 * @param idx トレーナーデータインデックス
 * @retval u16 OBJコード
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCRWORK_GetTrainerOBJCode( BSUBWAY_SCRWORK *bsw_scr, u16 idx )
{
  return BSUBWAY_GetTrainerOBJCode( bsw_scr->tr_data[idx].bt_trd.tr_type );
}

//--------------------------------------------------------------
/**
 * @brief  現在のWIFIランクを操作して返す
 * @param bsw_scr BSUBWAY_SCRWORK*
 * @param gsys GAMESYS_WORK
 * @param mode BSWAY_SETMODE
 * @retval u16 操作後のランク
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCRWORK_SetWifiRank(
    BSUBWAY_SCRWORK *bsw_scr, GAMESYS_WORK *gsys, BSWAY_SETMODE mode )
{
  u8 ct,rank;
  BSUBWAY_SCOREDATA *score = bsw_scr->scoreData;
  static const u8 btower_wifi_rankdown[] = {
   0,5,4,4,3,3,2,2,1,1,
  };

  switch( mode ){
  case BSWAY_SETMODE_get:
    return (u16)BSUBWAY_SCOREDATA_GetWifiRank( score );
  case BSWAY_SETMODE_inc:  //Inc
    BSUBWAY_SCOREDATA_SetFlag( score,  //連続敗戦フラグを落とす
      BSWAY_SCOREDATA_FLAG_WIFI_LOSE_F, BSWAY_SETMODE_reset);
    rank = BSUBWAY_SCOREDATA_GetWifiRank( score );
    
    if( rank == 10 ){  
      bsw_scr->prize_f = 1; //ランク10ならリボンを貰える条件はクリア(08.06.01)
      return 0;  //もう上がらない
    }
    
    //ランクアップ処理
    BSUBWAY_SCOREDATA_IncWifiRank( score );
    
    //ランク5以上にアップしてたらリボンがもらえる
    if( rank+1 >= 5 ){
      bsw_scr->prize_f = 1;
    }
    return 1;
  case BSWAY_SETMODE_dec:  //dec
    //現在の連続敗戦数をカウント
    ct = BSUBWAY_SCOREDATA_SetWifiLoseCount( score, BSWAY_SETMODE_inc );
    rank = BSUBWAY_SCOREDATA_GetWifiRank( score );
    
    if( rank == 1 ){
      return 0;
    }
    
    //ランク別敗戦カウントチェック
    if( ct >= btower_wifi_rankdown[rank-1] ){
      BSUBWAY_SCOREDATA_DecWifiRank( score ); //ランクダウン
      //連続敗戦数と連続敗戦フラグをリセット
      BSUBWAY_SCOREDATA_SetWifiLoseCount( score, BSWAY_SETMODE_reset );
      BSUBWAY_SCOREDATA_SetFlag( score,
        BSWAY_SCOREDATA_FLAG_WIFI_LOSE_F, BSWAY_SETMODE_reset );
      return 1;
    }
    return 0;
  }
  return 0;
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
static u8 get_MemberNum( u16 mode )
{
  switch(mode){
  case BSWAY_MODE_SINGLE:
  case BSWAY_MODE_WIFI:
    return 3;
  case BSWAY_MODE_DOUBLE:
    return 4;
  case BSWAY_MODE_MULTI:
  case BSWAY_MODE_COMM_MULTI:
//  case BSWAY_MODE_WIFI_MULTI:
    return 2;
  }
  GF_ASSERT( 0 );
  return 0;
}

//--------------------------------------------------------------
/**
 * トレーナーID重複チェック
 * @param trainer トレーナーIDが格納されたテーブル
 * @param id チェックするID
 * @param num trainer要素数
 * @retval BOOL TRUE=重複している
 */
//--------------------------------------------------------------
static BOOL is_ConflictTrainer( u16 * trainer, u16 id, u16 num )
{
  u16 i;
  for( i = 0;i < num;i++){
    if( trainer[i] == id ){
      return TRUE;
    }
  }
  return FALSE;
}

//======================================================================
//  wb null
//======================================================================
//--------------------------------------------------------------
/**
 * 特殊接続先に現在のロケーションをセット
 * @param
 * @retval
 */
//--------------------------------------------------------------
#if 0 //wb null
void BSUBWAY_SCRWORK_PushNowLocation( GAMEDATA *gdata )
{
  GAMEDATA_SetSpecialLocation( gdata, &loc );
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  バトルタワー用ポケモン選択画面呼び出し呼出し
 */
//--------------------------------------------------------------
#if 0 //wb null
void BSUBWAY_SCRWORK_SelectPoke(
    BSUBWAY_SCRWORK *wk, GMEVENT *ev, void **proc_wk )
{
  EventCmd_BTowerPokeSelectCall( ev_work,proc_wk,
    PL_MODE_BATTLE_TOWER,PST_MODE_NORMAL,
    wk->member_num,wk->member_num,100,0);
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  参加指定した手持ちポケモンの条件チェック
 *  @retval  0  参加OK
 *  @retval  1  同じポケモンがいる
 *  @retval 2  同じアイテムを持っている
 */
//--------------------------------------------------------------
#if 0 //wb null
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
#endif

//--------------------------------------------------------------
/**
 *  @brief　対戦トレーナーNo抽選
 */
//--------------------------------------------------------------
#if 0 //wb null
void BSUBWAY_SCRWORK_BtlTrainerNoSet( BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys )
{
#if 0 //wb
  int i;
  u16  no,stage;
  
  if(  wk->play_mode == BSWAY_MODE_MULTI ||
    wk->play_mode == BSWAY_MODE_WIFI_MULTI ||
    wk->play_mode == BSWAY_MODE_COMM_MULTI ){
    //if( wk->play_mode == BSWAY_MODE_COMM_MULTI && wk->pare_round > wk->stage ){
    if( ( wk->play_mode == BSWAY_MODE_COMM_MULTI && wk->pare_round > wk->stage ) ||
      ( wk->play_mode == BSWAY_MODE_WIFI_MULTI && wk->pare_round > wk->stage ) ){
      stage = wk->pare_round;  //通信時には周回数の多いほうで抽選
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
#endif

//--------------------------------------------------------------
/**
 *  @brief  最大連勝記録更新番組作成関数
 */
//--------------------------------------------------------------
#if 0 //wb null
static void bswayscr_MakeTVRenshouMaxUpdate(
    BSUBWAY_SCRWORK *wk,SAVEDATA *savedata,u16 renshou )
{
  POKEPARTY *party;

  if(  wk->play_mode != BSWAY_MODE_SINGLE &&
    wk->play_mode != BSWAY_MODE_DOUBLE ){
    return;
  }
  party = SaveData_GetTemotiPokemon( savedata );

  if( wk->play_mode == BSWAY_MODE_SINGLE ){
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
  case BSWAY_MODE_SINGLE:
    //ポケモンデータセット
    bswayscr_SaveMemberPokeData( wk,sv,BSWAY_SCORE_POKE_SINGLE );
  case BSWAY_MODE_DOUBLE:
    if( now_renshou >= 7){
      //TVインタビューデータセット(シングルとダブルで実行)
      TVTOPIC_BTowerTemp_Set( SaveData_GetTvWork( sv ),win_f,now_renshou );
    }
    break;
  case BSWAY_MODE_WIFI:
    //ポケモンデータセット
    bswayscr_SaveMemberPokeData( wk,sv,BSWAY_SCORE_POKE_WIFI );
    //スコア押し出し
    TowerPlayData_WifiRecordAdd( wk->playSave,wk->rec_down,wk->rec_turn,wk->rec_damage );

    //プレイモード書き出し
    buf8 = wk->play_mode;
    TowerPlayData_Put( wk->playSave,BSWAY_PSD_playmode,&buf8);
    //ラウンド数書き出し
    buf8 = wk->now_round;
    TowerPlayData_Put( wk->playSave,BSWAY_PSD_round,&buf8);
    
    TowerScoreData_SetWifiScore( wk->scoreSave,wk->playSave );
    break;
  default:
    break;
  }
}
#endif

#if 0 //wb null
void BSUBWAY_SCRWORK_SetClearScore( BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys )
{
/*
  u32  ret = 0;
  int  id;
//  void *note;
  u16  before,after,chg_flg;
//  RECORD * record;
//  FRONTIER_SAVEWORK *frontier;
    
  if( wk->play_mode == BSWAY_MODE_RETRY ){
    return;
  }
  
//  OS_Printf("TowerClearScoreSet -> mode = %d\n",wk->play_mode );
//
#if 0  
  record = SaveData_GetRecord( savedata );
  frontier = SaveData_GetFrontier( savedata );
#endif

#if 0
  if( wk->play_mode == BSWAY_MODE_WIFI_MULTI ){
    id = FRID_TOWER_MULTI_WIFI_RENSHOU;
  }else{
    id = wk->play_mode*2+FRID_TOWER_SINGLE_RENSHOU;
  }
#endif

  //現在の連勝数を書き出し
  if( wk->play_mode == BSWAY_MODE_WIFI_MULTI ){
    #if 0 //wb null
    chg_flg = FrontierRecord_Get( SaveData_GetFrontier( savedata ), 
          FRID_TOWER_MULTI_WIFI_CLEAR_BIT,
          Frontier_GetFriendIndex( FRID_TOWER_MULTI_WIFI_CLEAR_BIT ) );
    #else
    ch_flg = 0;
    #endif
  }else{
    chg_flg = BSUBWAY_SCOREDATA_SetFlag( wk->scoreData,
        BSWAY_SCOREDATA_FLAG_SINGLE_RECORD + wk->play_mode,
        BSWAY_DATA_get );
  }

  if( chg_flg ){  //現在も連勝記録挑戦中なら加算
    #if 0 //wb null
    ret = FrontierRecord_Get(  SaveData_GetFrontier( savedata ), 
                  ( id+1),
                  Frontier_GetFriendIndex( id+1) );

    FrontierRecord_Set(  SaveData_GetFrontier( savedata ), 
              ( id+1),
              Frontier_GetFriendIndex( id+1), ( ret+BSWAY_CLEAR_WINCNT ) );

    ret = FrontierRecord_Get(  SaveData_GetFrontier( savedata ), 
                  ( id+1),
                  Frontier_GetFriendIndex( id+1) );
    #endif
  }else{  //前回負けてた場合は一旦リセット
    #if 0 //wb null
    ret = FrontierRecord_Set(
        frontier,id+1,Frontier_GetFriendIndex( id+1),BSWAY_CLEAR_WINCNT );
    #endif
  }

#if 0
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
#if 0
  if( wk->play_mode == BSWAY_MODE_WIFI_MULTI ){
#if 0
    TowerScoreData_SetFlags( wk->scoreSave,BSWAY_SFLAG_WIFI_MULTI_RECORD,BSWAY_DATA_set );
#else
    FrontierRecord_Set(  SaveData_GetFrontier( savedata ), 
              FRID_TOWER_MULTI_WIFI_CLEAR_BIT,
              Frontier_GetFriendIndex( FRID_TOWER_MULTI_WIFI_CLEAR_BIT ), 1 );
#endif
  }else{
    TowerScoreData_SetFlags( wk->scoreSave,BSWAY_SFLAG_SINGLE_RECORD+wk->play_mode,BSWAY_DATA_set );
  }
#endif

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

#if 0
  //延べ勝利数更新
  RECORD_Add( record,RECID_BSUBWAY_WIN,BSWAY_CLEAR_WINCNT );
  //周回数プラス
  TowerScoreData_SetStage( wk->scoreSave,wk->play_mode,BSWAY_DATA_inc );
#endif

#if 0
  //バトルタワーへのチャレンジ数追加
  if( wk->play_mode != BSWAY_MODE_WIFI_MULTI ){
    RECORD_Add( record,RECID_BSUBWAY_CHALLENGE,1);
  }

  //スコア加算
  RECORD_Score_Add( record,SCORE_ID_BSUBWAY_7WIN );
  
  //連勝リボンをもらえるかどうかのフラグをセット
  bswayscr_IfRenshouPrizeGet( wk );

  //勝ち負け共通データ処理
  bswayscr_SetCommonScore( wk,savedata,TRUE,ret );
#endif

#if 0
  //冒険ノート(通信マルチ&wifiのみ)
  if(  wk->play_mode == BSWAY_MODE_COMM_MULTI ||
    wk->play_mode == BSWAY_MODE_WIFI_MULTI ||
    wk->play_mode == BSWAY_MODE_WIFI ){
#else
  //冒険ノート( WIFI_DLのみ)
  if( wk->play_mode == BSWAY_MODE_WIFI ){
#endif
    note = FNOTE_SioBattleTowerDataMake( wk->heapID );
    FNOTE_DataSave( fnote, note, FNOTE_TYPE_SIO );
  }
*/
  u8 buf8;
  
  //プレイモード書き出し
  buf8 = wk->play_mode;
  BSUBWAY_PLAYDATA_SetData( wk->playData,
      BSWAY_PLAYDATA_ID_playmode, &buf8 );

  //ラウンド数書き出し
  buf8 = wk->now_round;
	BSUBWAY_PLAYDATA_SetData( wk->playData,
      BSWAY_PLAYDATA_ID_round, &buf8 );
  
  //連勝記録書き出し
  {
    u32 renshou = BSUBWAY_SCOREDATA_GetRenshou(
        wk->scoreData, wk->play_mode );
    
    if( renshou + wk->now_win > 0xffff ){
      renshou = 0xffff;
    }else{
      renshou += wk->now_win;
    }
#if 0   
    BSUBWAY_SCOREDATA_SetRenshouCount(
          wk->scoreData, wk->play_mode, renshou );
#endif
  }
}
#endif

#if 0 //wb null
void TowerScr_ChoiceBtlSeven(BTOWER_SCRWORK* wk)
{
  int i;

  for(i = 0;i < BSWAY_FIVE_NUM;i++){
    wk->five_item[i] = (u8)RomBattleTowerTrainerDataMake(wk,&(wk->five_data[i]),
      TOWER_FIVE_FIRST+i,wk->member_num,wk->mem_poke,wk->mem_item,&(wk->five_poke[i]),wk->heapID);
  }
}
#endif

// @brief  AIマルチペアポケモン抽選
#if 0 //wb
void BSUBWAY_SCRWORK_ChoiceBtlSeven( BSUBWAY_SCRWORK *wk )
{
  int i;
  
  for( i = 0;i < BSWAY_FIVE_NUM;i++){
    wk->five_item[i] = ( u8)RomBattleTowerTrainerDataMake( wk,&( wk->five_data[i]),
      TOWER_FIVE_FIRST+i,wk->member_num,wk->mem_poke,wk->mem_item,&( wk->five_poke[i]),wk->heapID );
  }
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
#if 0 // wb
u16 BSUBWAY_SCRWORK_GetPlayMode( BSUBWAY_SCRWORK *wk )
{
  return wk->play_mode;
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  リーダークリアフラグを取得する
 */
//--------------------------------------------------------------
#if 0 // wb
u16 BSUBWAY_SCRWORK_GetLeaderClearFlag( BSUBWAY_SCRWORK *wk )
{
  return (u16)wk->leader_f;
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  タワークリア時にバトルポイントを加算する
 *  @return  新たに取得したバトルポイント
 */
//--------------------------------------------------------------
#if 0 //wb
u16 BSUBWAY_SCRWORK_AddBtlPoint( BSUBWAY_SCRWORK *wk )
{
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

  if( wk->play_mode == BSWAY_MODE_RETRY ){
    return 0;
  }
  if( wk->play_mode == BSWAY_MODE_WIFI ){
    //ランクごと
    point = bpoint_wifi[TowerScoreData_SetWifiRank( wk->scoreSave,BSWAY_DATA_get )];
  }else{

    //ワイアレスマルチ、WIFI
    //if( wk->play_mode == BSWAY_MODE_COMM_MULTI ){
    if( ( wk->play_mode == BSWAY_MODE_COMM_MULTI ) || ( wk->play_mode == BSWAY_MODE_WIFI_MULTI ) ){

      //周回数ごと
      stage = TowerScoreData_SetStage( wk->scoreSave,wk->play_mode,BSWAY_DATA_get );
      if( stage >= 7){                //この比較文はDPと合わせる
        point = BTLPOINT_VAL_TOWER_COMM8;
      }else{
        point = bpoint_comm[stage];
      }

#if 0
    //WIFI(32人データがあるので、周回数ワークが足りないため、連勝数から周回数を算出する
    }else if( wk->play_mode == BSWAY_MODE_WIFI_MULTI ){

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
      stage = TowerScoreData_SetStage( wk->scoreSave,wk->play_mode,BSWAY_DATA_get );
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
  TowerScoreData_SetBattlePoint( wk->scoreSave,point,BSWAY_DATA_add );
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

  if( record < BSWAY_50_RENSHOU_CNT ){
    return 0;
  }
  if( record >= BSWAY_100_RENSHOU_CNT ){
    if( TowerScoreData_SetFlags( wk->scoreSave,BSWAY_SFLAG_GOLD_GET,BSWAY_DATA_get )){
      return 0;
    }
  }else{
    if( TowerScoreData_SetFlags( wk->scoreSave,BSWAY_SFLAG_SILVER_GET,BSWAY_DATA_get )){
      return 0;
    }
  }
  return 1;
}
#endif

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
    return ( u16)TowerScoreData_SetWifiRank( score,BSWAY_DATA_get );
  case 1:  //Inc
    //連続敗戦フラグを落とす
    TowerScoreData_SetFlags( score,BSWAY_SFLAG_WIFI_LOSE_F,BSWAY_DATA_reset );
    //現在のランクを取得
    rank = TowerScoreData_SetWifiRank( score,BSWAY_DATA_get );

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
    TowerScoreData_SetWifiRank( score,BSWAY_DATA_inc );

    //ランク5以上にアップしてたらリボンがもらえる
    if( rank+1 >= 5){
      wk->prize_f = 1;
    }
    return 1;
  case 2:  //dec
    //現在の連続敗戦数をカウント
    ct = TowerScoreData_SetWifiLoseCount( score,BSWAY_DATA_inc );
    rank = TowerScoreData_SetWifiRank( score,BSWAY_DATA_get );

    if( rank == 1){
      return 0;
    }
    //ランク別敗戦カウントチェック
    if( ct >= btower_wifi_rankdown[rank-1] ){
      //ランクダウン
      TowerScoreData_SetWifiRank( score,BSWAY_DATA_dec );
      //連続敗戦数と連続敗戦フラグをリセット
      TowerScoreData_SetWifiLoseCount( score,BSWAY_DATA_reset );
      TowerScoreData_SetFlags( score,BSWAY_SFLAG_WIFI_LOSE_F,BSWAY_DATA_reset );
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
  if( wk->play_mode != BSWAY_MODE_SINGLE ){
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

  if( wk->play_mode == BSWAY_MODE_RETRY ){
    return 0;
  }

  /////////////////////////////////////////
  //とりあえず仕様が決まっていないので保留
  /////////////////////////////////////////
  if( wk->play_mode == BSWAY_MODE_WIFI_MULTI ){
    return 0;
  }

  if(!wk->prize_f ){
    return 0;
  }
  switch( wk->play_mode ){
  case BSWAY_MODE_DOUBLE:
    id = ID_PARA_sinou_battle_tower_2vs2_win50;
    break;
  case BSWAY_MODE_MULTI:
    id = ID_PARA_sinou_battle_tower_aimulti_win50;
    break;
  case BSWAY_MODE_COMM_MULTI:
    id = ID_PARA_sinou_battle_tower_siomulti_win50;
    break;
  case BSWAY_MODE_WIFI:
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
 *  case BSWAY_SUB_UPDATE_RANDOM で呼ばれる
 *  ここは、WIFIマルチは通過しないので、このままでよい
 */
//--------------------------------------------------------------
#if 0 //wb
u16 BSUBWAY_SCRWORK_PlayRandUpdate( BSUBWAY_SCRWORK *wk,SAVEDATA *sv )
{
  u8  chg_flg;

  //現在チャレンジ継続中かどうか？
  if( wk->play_mode == BSWAY_MODE_WIFI_MULTI ){
#if 0
    chg_flg = TowerScoreData_SetFlags( wk->scoreSave,
        BSWAY_SFLAG_WIFI_MULTI_RECORD,BSWAY_DATA_get );
#else
    chg_flg = FrontierRecord_Get( SaveData_GetFrontier( sv ), 
                  FRID_TOWER_MULTI_WIFI_CLEAR_BIT,
                  Frontier_GetFriendIndex( FRID_TOWER_MULTI_WIFI_CLEAR_BIT ) );
#endif
  }else{
    chg_flg = TowerScoreData_SetFlags( wk->scoreSave,
        BSWAY_SFLAG_SINGLE_RECORD+wk->play_mode,BSWAY_DATA_get );
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
  if(  wk->play_mode == BSWAY_MODE_RETRY ||
    wk->play_mode == BSWAY_MODE_SINGLE ||
    wk->play_mode == BSWAY_MODE_WIFI_MULTI ||
    wk->play_mode == BSWAY_MODE_WIFI ){
    return 0;
  }
  win = wk->renshou+wk->now_win;

  //50連勝以上でもらえる
  if( win < BSWAY_50_RENSHOU_CNT ){
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
static void bswayscr_SaveMemberPokeData( BSUBWAY_SCRWORK *wk,SAVEDATA *sv,BSWAY_SCORE_POKE_DATA mode )
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
  if( wk->play_mode == BSWAY_MODE_WIFI_MULTI ){
    return ( gf_rand() );
  }

  wk->play_rnd_seed = BtlTower_PlayFixRand( wk->play_rnd_seed );

  OS_Printf("btower_rand = %d\n",wk->play_rnd_seed );
  return ( wk->play_rnd_seed/65535);
}
#endif
