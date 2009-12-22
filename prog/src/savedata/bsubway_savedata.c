//======================================================================
/**
 * @file  bsubway_savedata.c
 * @brief  バトルサブウェイ　セーブデータ関連
 * @authaor  iwasawa
 * @date  2008.12.11
 * @note PLより移植 kagaya
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "libdpw/dpw_bt.h"

#include "savedata/bsubway_savedata.h"
#include "bsubway_savedata_local.h"
#include "../field/bsubway_scr_def.h"

#if 0 //wb null
#ifdef _NITRO
// 構造体が想定のサイズとなっているかチェック
SDK_COMPILER_ASSERT(sizeof(BSUBWAY_LEADER_DATA) == 34);
SDK_COMPILER_ASSERT(sizeof(BSUBWAY_POKEMON) == 56);
SDK_COMPILER_ASSERT(sizeof(BSUBWAY_WIFI_PLAYER) == 228);
#endif
#endif

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// バトルサブウェイプレイ進行データ
/// 挑戦開始時にクリアされる
/// size=20byte
//--------------------------------------------------------------
struct _BSUBWAY_PLAYDATA
{
  u8  itemfix_f  :1;  ///<固定アイテムかどうかのフラグ
  u8  saved_f    :1;  ///<セーブ済みかどうか
  u8  play_mode  :3;  ///<現在どこにチャレンジ中か?
  u8  partner    :3;  ///<現在誰と組んでいるか?
  u8  dummy; ///<4byte境界ダミー
  u8  tower_round;    ///<バトルサブウェイ今何人目？
  u8  wifi_rec_down;    ///<勝ち抜きまでに倒されたポケモン数
  u16  wifi_rec_turn;    ///<勝ち抜きにかかったターン数
  u16  wifi_rec_damage;  ///<勝ち抜きまでに受けたダメージ数
  
  u8  member_poke[BSUBWAY_STOCK_MEMBER_MAX];    ///<選択したポケモンの位置
  u16  trainer_no[BSUBWAY_STOCK_TRAINER_MAX];    ///<対戦トレーナーNo保存
  
  u32  play_rnd_seed;  ///<タワープレイ変化ランダムシード保存場所
  
  ///<AIマルチのペアのポケモン再生成に必要なパラメータ
  struct _BSUBWAY_PAREPOKE_PARAM  pare_poke;
};

//--------------------------------------------------------------
/// バトルサブウェイ　スコアデータ
/// size= 20+168+168=356byte
/// ゲームを通して保存&管理しておく、プレイヤー成績
//--------------------------------------------------------------
struct _BSUBWAY_SCOREDATA
{
  u16  btl_point;  ///<バトルポイント
  u8  wifi_lose;  ///<連続敗戦カウント
  u8  wifi_rank;  ///<WiFiランク
  
  u32  day_rnd_seed;  ///<タワー用日付変化ランダムシード保存場所
  
  union{
    struct{
    u16  silver_get:1;    ///<シルバートロフィーゲット
    u16  gold_get:1;      ///<ゴールドトロフィーゲット
    u16  silver_ready:1;    ///<シルバー貰えます
    u16  gold_ready:1;    ///<ゴールド貰えます
    u16  wifi_lose_f:1;    ///<wifi連続敗戦フラグ
    u16  wifi_update:1;    ///<wifi成績アップロードフラグ
    u16  wifi_poke_data:1;  ///<wifiポケモンデータストック有りナシフラグ
    u16  single_poke_data:1;  ///<singleポケモンデータストック有りナシフラグ
    u16  single_record:1;  ///<シングルレコード挑戦中フラグ
    u16  double_record:1;  ///<ダブルレコード挑戦中フラグ
    u16  multi_record:1;    ///<マルチレコード挑戦中フラグ
    u16  cmulti_record:1;  ///<通信マルチレコード挑戦中フラグ
    u16  wifi_record:1;    ///<Wifiレコード挑戦中フラグ
    u16  copper_get:1;    ///<カッパートロフィーゲット
    u16  copper_ready:1;    ///<カッパー貰えます
    u16  :1;  ///<境界ダミー
    };
    u16  flags;
  };
  
  ///<バトルサブウェイ周回数(paddingをWIFI_MULTI用に加えた)
  u16  tower_stage[6];
  
  //連勝記録
  u16 renshou[BSWAY_PLAYMODE_MAX];
  u16 renshou_max[BSWAY_PLAYMODE_MAX];
 
  //WiFiチャレンジデータ
  u16  wifi_score;  ///<WiFi成績
  
  //WiFiポケモンデータストック
  struct _BSUBWAY_POKEMON  wifi_poke[3];
  //トレーナーロード用シングルデータストック
  struct _BSUBWAY_POKEMON  single_poke[3];
};

//======================================================================
//  proto
//======================================================================

//======================================================================
//  バトルサブウェイ　プレイデータ
//======================================================================
//--------------------------------------------------------------
/**
 * プレイデータサイズ
 * @param nothing
 * @retval int ワークサイズ
 */
//--------------------------------------------------------------
int BSUBWAY_PLAYDATA_GetWorkSize( void )
{
  return( sizeof(BSUBWAY_PLAYDATA) );
}

//--------------------------------------------------------------
/**
 * プレイデータ　初期化
 * @param bsw_play BSUBWAY_PLAYDATA
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_PLAYDATA_Init( BSUBWAY_PLAYDATA *bsw_play )
{
  MI_CpuClear8( bsw_play, sizeof(BSUBWAY_PLAYDATA) );
}

//--------------------------------------------------------------
/**
 * プレイデータ　セーブフラグセット
 * @param bsw_play BSUBWAY_PLAYDATA
 * @param flag TRUE=セーブあり FALSE=なし
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_PLAYDATA_SetSaveFlag( BSUBWAY_PLAYDATA *bsw_play, BOOL flag )
{
  bsw_play->saved_f = flag;
}

//--------------------------------------------------------------
/**
 *  プレイデータ　正しくセーブ済みかどうか？
 *  @retval  TRUE  正しくセーブされている
 *  @retval FALSE  セーブされていない
 */
//--------------------------------------------------------------
BOOL BSUBWAY_PLAYDATA_GetSaveFlag( BSUBWAY_PLAYDATA *bsw_play )
{
  return bsw_play->saved_f;
}

//--------------------------------------------------------------
/**
 * プレイデータ　取得
 * @param bsw_play BSUBWAY_PLAYDATA
 * @param id 取得するBSWAY_PLAYDATA_ID
 * @param buf データ取得ポインタ
 * @retval u32 IDから取得した結果
 */
//--------------------------------------------------------------
u32 BSUBWAY_PLAYDATA_GetData(
    const BSUBWAY_PLAYDATA *bsw_play, BSWAY_PLAYDATA_ID id, void *buf )
{
  switch( id )
  {
  case BSWAY_PLAYDATA_ID_playmode:
    return (u32)bsw_play->play_mode;
  case BSWAY_PLAYDATA_ID_round:
    return (u32)bsw_play->tower_round;
  case BSWAY_PLAYDATA_ID_rec_down:
    return (u32)bsw_play->wifi_rec_down;
  case BSWAY_PLAYDATA_ID_rec_turn:
    return bsw_play->wifi_rec_turn;
  case BSWAY_PLAYDATA_ID_rec_damage:
    return bsw_play->wifi_rec_damage;
  case BSWAY_PLAYDATA_ID_pokeno:
    GF_ASSERT( buf != NULL );
    MI_CpuCopy8( bsw_play->member_poke, buf, BSUBWAY_STOCK_MEMBER_MAX );
    return 0;
  case BSWAY_PLAYDATA_ID_pare_poke:
    GF_ASSERT( buf != NULL );
    MI_CpuCopy8( &bsw_play->pare_poke, buf, sizeof(BSUBWAY_PAREPOKE_PARAM) );
    return 0;
  case BSWAY_PLAYDATA_ID_pare_itemfix:
    return bsw_play->itemfix_f;
  case BSWAY_PLAYDATA_ID_trainer:
    GF_ASSERT( buf != NULL );
    MI_CpuCopy8(bsw_play->trainer_no,buf, 2*BSUBWAY_STOCK_TRAINER_MAX );
    return 0;
  case BSWAY_PLAYDATA_ID_partner:
    return bsw_play->partner;
  case BSWAY_PLAYDATA_ID_rnd_seed:
    return bsw_play->play_rnd_seed;
  default:
    GF_ASSERT( 0 );
  }
  
  return 0;
}

//--------------------------------------------------------------
/**
 * プレイデータ　設定
 * @param bsw_play BSUBWAY_PLAYDATA
 * @param id 取得するBSWAY_PLAYDATA_ID
 * @param buf 反映するデータポインタ
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_PLAYDATA_SetData(
    BSUBWAY_PLAYDATA *bsw_play, BSWAY_PLAYDATA_ID id, const void *buf )
{
  u32 *buf32 = (u32*)buf;
  u16 *buf16 = (u16*)buf;
  u8 *buf8 = (u8*)buf;
  
  GF_ASSERT( buf != NULL );
  
  switch(id){
  case BSWAY_PLAYDATA_ID_playmode:
    bsw_play->play_mode = buf8[0];
    break;
  case BSWAY_PLAYDATA_ID_round:
    bsw_play->tower_round = buf8[0];
    break;
  case BSWAY_PLAYDATA_ID_rec_down:
    bsw_play->wifi_rec_down = buf8[0];
    break;
  case BSWAY_PLAYDATA_ID_rec_turn:
    bsw_play->wifi_rec_turn = buf16[0];
    break;
  case BSWAY_PLAYDATA_ID_rec_damage:
    bsw_play->wifi_rec_damage = buf16[0];
    break;
  case BSWAY_PLAYDATA_ID_pokeno:
    MI_CpuCopy8( buf8, bsw_play->member_poke, 4 );
    break;
  case BSWAY_PLAYDATA_ID_pare_poke:
    MI_CpuCopy8( buf16,
        &bsw_play->pare_poke, sizeof(BSUBWAY_PAREPOKE_PARAM) );
    break;
  case BSWAY_PLAYDATA_ID_pare_itemfix:
    bsw_play->itemfix_f = buf8[0];
    break;
  case BSWAY_PLAYDATA_ID_trainer:
    MI_CpuCopy8( buf16, bsw_play->trainer_no, 2*BSUBWAY_STOCK_TRAINER_MAX );
    break;
  case BSWAY_PLAYDATA_ID_rnd_seed:
    bsw_play->play_rnd_seed = buf32[0];
    break;
  case BSWAY_PLAYDATA_ID_partner:
    bsw_play->partner = buf8[0];
    break;
  default:
    GF_ASSERT( 0 );
  }
  
  /* //wb
  #if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
  #endif //CRC_LOADCHECK
  */
}

//--------------------------------------------------------------
/**
 *  @brief プレイデータ　WifiレコードデータAdd  
 *  @param  down  倒されたポケモン追加数
 *  @param  turn  かかったターン追加数
 *  @param  damage  受けたダメージ追加値
 */
//--------------------------------------------------------------
void BSUBWAY_PLAYDATA_AddWifiRecord(
    BSUBWAY_PLAYDATA *bsw_play, u8 down,u16 turn,u16 damage )
{
  if(bsw_play->wifi_rec_down + down < 255){
    bsw_play->wifi_rec_down += down;
  }
  if(bsw_play->wifi_rec_turn + turn < 65535){
    bsw_play->wifi_rec_turn += turn;
  }
  if(bsw_play->wifi_rec_damage + damage < 65535){
    bsw_play->wifi_rec_damage += damage;
  }

#if  PL_T0855_080710_FIX
  /*
  OS_Printf( "********************\n" );
  OS_Printf( "dat->wifi_rec_turn = %d\n", dat->wifi_rec_turn );
  OS_Printf( "dat->wifi_rec_down = %d\n", dat->wifi_rec_down );
  OS_Printf( "dat->wifi_rec_damage = %d\n", dat->wifi_rec_damage );
  */
#endif

/*
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
*/
}

//======================================================================
//  バトルサブウェイ　スコアデータ
//======================================================================
//--------------------------------------------------------------
/**
 * スコアデータサイズ
 * @param none
 * @retval int ワークサイズ
 */
//--------------------------------------------------------------
int BSUBWAY_SCOREDATA_GetWorkSize( void )
{
  return( sizeof(BSUBWAY_SCOREDATA) );
}

//--------------------------------------------------------------
/**
 * スコアデータ初期化
 * @param bsw_score BSUBWAY_SCOREDATA
 * @retval nothing
 */
//--------------------------------------------------------------
void BSUBWAY_SCOREDATA_Init( BSUBWAY_SCOREDATA *bsw_score )
{
  MI_CpuClear8( bsw_score, sizeof(BSUBWAY_SCOREDATA) );
  bsw_score->wifi_rank = 1;
}

//--------------------------------------------------------------
/**
 * スコアデータ　バトルポイント操作
 * @param bsw_score BSUBWAY_SCOREDATA
 * @param num セットする値
 * @param mode BSWAY_SETMODE
 * @retval u16 操作後のバトルポイント
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCOREDATA_SetBattlePoint(
    BSUBWAY_SCOREDATA *bsw_score, u16 num, BSWAY_SETMODE mode )
{
  switch( mode ){
  case BSWAY_SETMODE_set:
    if( num > 9999 ){
      bsw_score->btl_point = 9999;
    }else{
      bsw_score->btl_point = num;
    }
    break;
  case BSWAY_SETMODE_add:
    if( bsw_score->btl_point+num > 9999 ){
      bsw_score->btl_point = 9999;
    }else{
      bsw_score->btl_point += num;
    }
    break;
  case BSWAY_SETMODE_sub:
    if( bsw_score->btl_point < num ){
      bsw_score->btl_point = 0;
    }else{
      bsw_score->btl_point -= num;
    }
  case BSWAY_SETMODE_get:
    break;
  default:
    GF_ASSERT( 0 );
  }

  /* //wb
  #if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
  #endif //CRC_LOADCHECK
  */
  return bsw_score->btl_point;
}

//--------------------------------------------------------------
/**
 * スコアデータ　連勝数操作
 * @param
 * @retval
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCOREDATA_CalcRenshou(
    BSUBWAY_SCOREDATA *bsw_score, BSWAY_SETMODE mode )
{
  switch(mode){
  case BSWAY_SETMODE_reset:
    bsw_score->renshou[mode] = 0;
    break;
  case BSWAY_SETMODE_inc:
    if( bsw_score->renshou[mode] < 65534 ){
      bsw_score->renshou[mode]++;
    }
    break;
  case BSWAY_SETMODE_get:
    break;
  default:
    GF_ASSERT( 0 );
  }
  
  return( bsw_score->renshou[mode] );
}

//--------------------------------------------------------------
/**
 * スコアデータ　周回数操作
 * @param bsw_score BSUBWAY_SCOREDATA
 * @param id スコアID
 * @param value セットする値
 * @param mode BSWAY_SETMODE
 * @retval u16 操作後の周回数
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCOREDATA_SetStage(
    BSUBWAY_SCOREDATA *bsw_score, u16 id, BSWAY_SETMODE mode )
{
  u16 id2;
  
  if( id == BSWAY_MODE_RETRY ){
    return 0;  //リトライモードではカウントしない
  }
  
  //プラチナ追加　wifiマルチ
  if( id == BSWAY_MODE_WIFI_MULTI ){
    id2 = 5; //tower_stage[5]
  }else{
    id2 = id;
  }

  switch(mode){
  case BSWAY_SETMODE_reset:
    bsw_score->tower_stage[id2] = 0;
    break;
  case BSWAY_SETMODE_inc:
    if( bsw_score->tower_stage[id2] < 65534 ){
      bsw_score->tower_stage[id2] += 1;
    }
    break;
  }
  
  /* //wb null
  #if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
  #endif //CRC_LOADCHECK
  */
  return bsw_score->tower_stage[id2];
}
  
//--------------------------------------------------------------
/**
 *  スコアデータ フラグエリアセット
 *  @param bsw_score BSUBWAY_SCOREDATA*
 *  @param id BSWAY_SCOREDATA_FLAG
 *  @param mode BSWAY_SETMODE
 *  @retval BOOL フラグの状態
 */
//--------------------------------------------------------------
BOOL BSUBWAY_SCOREDATA_SetFlag( BSUBWAY_SCOREDATA *bsw_score,
    BSWAY_SCOREDATA_FLAG id, BSWAY_SETMODE mode )
{
  u16 i;
  u16 flag = 1;
  
  //エラーチェック
  if( id >= BSWAY_SCOREDATA_FLAG_MAX ){
    GF_ASSERT( 0 );
    return 0;
  }

  //フラグID生成
  for( i = 0; i < id; i++ ){
    flag <<= 1;
  }
  
  switch( mode ){
  case BSWAY_SETMODE_reset:
    flag = (flag^0xFFFF);
    bsw_score->flags &= flag;
    break;
  case BSWAY_SETMODE_set:
    bsw_score->flags |= flag;
    break;
  case BSWAY_SETMODE_get:
    return (BOOL)((bsw_score->flags>>id)&0x0001);
  default:
    GF_ASSERT( 0 );
  }
  
  /* //wb null
  #if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
  #endif //CRC_LOADCHECK
  */
  return 0;
}

//--------------------------------------------------------------
/**
 *  スコアデータ　Wifiランク操作
 *  有効コマンド get/reset/inc/dec
 *  @param bsw_score BSUBWAY_SCOREDATA*
 *  @param mode BSWAY_SETMODE
 *  @retval u8 Wifiランキング
 */
//--------------------------------------------------------------
u8 BSUBWAY_SCOREDATA_SetWifiRank(
    BSUBWAY_SCOREDATA *bsw_score, BSWAY_SETMODE mode )
{
  switch( mode ){
  case BSWAY_SETMODE_reset:
    bsw_score->wifi_rank = 1;
    break;
  case BSWAY_SETMODE_inc:
    if( bsw_score->wifi_rank < 10 ){
      bsw_score->wifi_rank += 1;
    }
    break;
  case BSWAY_SETMODE_dec:
    if( bsw_score->wifi_rank > 1 ){
      bsw_score->wifi_rank -= 1;
    }
    break;
  }
/*  //wb null  
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
*/
  return bsw_score->wifi_rank;
}

//--------------------------------------------------------------
/**
 * スコアデータ、前回の連勝記録を取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCOREDATA_GetRenshouCount( const BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode )
{
  return bsw_score->renshou[mode];
}

//--------------------------------------------------------------
/**
 * スコアデータ、前回の連勝記録をセット
 * @param
 * @retval
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCOREDATA_SetRenshouCount(
    BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode, u16 count )
{
  bsw_score->renshou[mode] = count;
}

//--------------------------------------------------------------
/**
 * スコアデータ、最大連勝記録を取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCOREDATA_GetMaxRenshouCount(
    const BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode )
{
  return bsw_score->renshou_max[mode];
}

//--------------------------------------------------------------
/**
 * スコアデータ、最大連勝記録をセット
 * @param
 * @retval
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCOREDATA_SetMaxRenshouCount(
    BSUBWAY_SCOREDATA *bsw_score, BSWAY_PLAYMODE mode, u16 count )
{
  bsw_score->renshou_max[mode] = count;
}

//======================================================================
//  wb null
//======================================================================
//----
#if 0 //wb null
//----
//--------------------------------------------------------------
/**
 *  @brief  サブウェイ　ポケモンデータサイズ
 */
//--------------------------------------------------------------
int BSUBWAY_WifiPoke_GetWorkSize(void)
{
  return sizeof(BSUBWAY_POKEMON);
}

//--------------------------------------------------------------
/**
 *  @brief  サブウェイ　Wifiデータサイズ
 */
//--------------------------------------------------------------
int  BSUBWAY_WifiData_GetWorkSize(void)
{
  return sizeof(BSUBWAY_WIFI_DATA);
}

//--------------------------------------------------------------
/**
 *  @brief　サブウェイ Wifi/トレーナーロードプレイヤーデータ構造体データサイズ
 */
//--------------------------------------------------------------
int BSUBWAY_DpwBtPlayer_GetWorkSize(void)
{
  return sizeof(Dpw_Bt_Player);
}

//--------------------------------------------------------------
/**
 *  @brief  サブウェイ　プレイデータクリア
 */
//--------------------------------------------------------------
void BSUBWAY_PlayData_Clear(BSUBWAY_PLAYWORK* dat)
{
  MI_CpuClear8(dat,sizeof(BSUBWAY_PLAYWORK));
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}

//--------------------------------------------------------------
/**
 *  @brief  サブウェイ  スコアデータクリア
 */
//--------------------------------------------------------------
void BSUBWAY_ScoreData_Clear(BSUBWAY_SCOREWORK* dat)
{
  MI_CpuClear8(dat,sizeof(BSUBWAY_SCOREWORK));
  dat->wifi_rank = 1;
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}

//--------------------------------------------------------------
/**
 *  @brief  サブウェイ　プレイヤーメッセージデータクリア
 */
//--------------------------------------------------------------
void BSUBWAY_PlayerMsg_Clear(BSUBWAY_PLAYER_MSG* dat)
{
#if 0 //wb
  PMSDAT_SetupDefaultBattleTowerMessage(&dat->msg[0], BSWAY_MSG_PLAYER_READY);
  PMSDAT_SetupDefaultBattleTowerMessage(&dat->msg[1], BSWAY_MSG_PLAYER_WIN);
  PMSDAT_SetupDefaultBattleTowerMessage(&dat->msg[2], BSWAY_MSG_PLAYER_LOSE);
  PMSDAT_SetupDefaultBattleTowerMessage(&dat->msg[3], BSWAY_MSG_LEADER);
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
#endif
}

//--------------------------------------------------------------
/**
 *  @brief  サブウェイ　Wifiデータクリア
 */
//--------------------------------------------------------------
void BSUBWAY_WifiData_Clear(BSUBWAY_WIFI_DATA* dat)
{
  MI_CpuClear8(dat,sizeof(BSUBWAY_WIFI_DATA));
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}

//======================================================================
//　サブウェイ　プレイデータアクセス系
//======================================================================

//======================================================================
//　サブウェイ　スコアデータアクセス系
//======================================================================
//--------------------------------------------------------------
/**
 *  @brief  スコアデータ　Wifi連続敗戦カウント操作
 *  有効コマンド get/reset/inc
 *  @return  操作後のカウント
 */
//--------------------------------------------------------------
u8 BSUBWAY_ScoreData_SetWifiLoseCount(BSUBWAY_SCOREWORK* dat,BSWAY_DATA_SETID mode)
{
  switch(mode){
  case BSWAY_DATA_reset:
    dat->wifi_lose = 0;
    dat->wifi_lose_f = 0;
    break;
  case BSWAY_DATA_inc:
    if(dat->wifi_lose_f){
      //連続敗戦中
      dat->wifi_lose += 1;
    }else{
      dat->wifi_lose = 1;
      dat->wifi_lose_f = 1;
    }
    break;
  }
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
  return dat->wifi_lose;
}

//--------------------------------------------------------------
/**
 *  @brief  使用ポケモンデータ保存
 */
//--------------------------------------------------------------
void BSUBWAY_ScoreData_SetUsePokeData(BSUBWAY_SCOREWORK* dat,
        BSWAY_SCORE_POKE_DATA mode,BSUBWAY_POKEMON* poke)
{
  if(mode == BSWAY_SCORE_POKE_SINGLE){
    MI_CpuCopy8(poke,dat->single_poke,sizeof(BSUBWAY_POKEMON)*3);
  }else{
    MI_CpuCopy8(poke,dat->wifi_poke,sizeof(BSUBWAY_POKEMON)*3);
  }
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}

//--------------------------------------------------------------
/**
 *  @brief  使用ポケモンデータをバッファにコピーして取得
 */
//--------------------------------------------------------------
void BSUBWAY_ScoreData_GetUsePokeData(BSUBWAY_SCOREWORK* dat,
      BSWAY_SCORE_POKE_DATA mode,BSUBWAY_POKEMON* poke)
{
  if(mode == BSWAY_SCORE_POKE_SINGLE){
    MI_CpuCopy8(dat->single_poke,poke,sizeof(BSUBWAY_POKEMON)*3);
  }else{
    MI_CpuCopy8(dat->wifi_poke,poke,sizeof(BSUBWAY_POKEMON)*3);
  }
}

//--------------------------------------------------------------
/**
 *  @brief  使用ポケモンデータをバッファにコピーして取得(WiFi構造体データ型)
 */
//--------------------------------------------------------------
void BSUBWAY_ScoreData_GetUsePokeDataDpw(BSUBWAY_SCOREWORK* dat,
      BSWAY_SCORE_POKE_DATA mode,DPW_BT_POKEMON_DATA* poke)
{
  if(mode == BSWAY_SCORE_POKE_SINGLE){
    MI_CpuCopy8(dat->single_poke,poke,sizeof(BSUBWAY_POKEMON)*3);
  }else{
    MI_CpuCopy8(dat->wifi_poke,poke,sizeof(BSUBWAY_POKEMON)*3);
  }
}

//--------------------------------------------------------------
/**
 *  @brief  スコアデータ Wifi成績操作
 */
//--------------------------------------------------------------
u16  BSUBWAY_ScoreData_SetWifiScore(BSUBWAY_SCOREWORK* dat,BSUBWAY_PLAYWORK *playdata)
{
  u16  sa,sb,sc,sd,st;
  u16  score = 0;

  //ラウンド数は勝ち抜き数+1になっているのでマイナス１して計算する
  sa = (playdata->tower_round-1)*1000;
  sb = playdata->wifi_rec_turn*10;
  sc = playdata->wifi_rec_down*20;
  if(sb+sc > 950){
    st = 0;
  }else{
    st = 950-(sb+sc);
  }
  if(playdata->wifi_rec_damage>(1000-30)){
    sd = 0;
  }else{
    sd = (1000-playdata->wifi_rec_damage)/30;
  }
  score = sa+st+sd;
  dat->wifi_score = score;
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
  return score;
}

//--------------------------------------------------------------
/**
 *  @brief  スコアデータ　Wifi成績0クリア
 */
//--------------------------------------------------------------
void BSUBWAY_ScoreData_ClearWifiScore(BSUBWAY_SCOREWORK* dat)
{
  dat->wifi_score = 0;
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}

//--------------------------------------------------------------
/**
 *  @brief  スコアデータ　Wifi成績取得
 */
//--------------------------------------------------------------
u16  BSUBWAY_ScoreData_GetWifiScore(BSUBWAY_SCOREWORK* dat)
{
  return dat->wifi_score;
}

//--------------------------------------------------------------
/**
 *  @brief  スコアデータから勝ち抜いた数を取得
 */
//--------------------------------------------------------------
u8  BSUBWAY_ScoreData_GetWifiWinNum(BSUBWAY_SCOREWORK* dat)
{
  u8 ret = 0;
  ret = (dat->wifi_score)/1000;

  return ret;
}

//--------------------------------------------------------------
/**
 *  @brief  スコアデータ　周回数操作
 *  有効コマンド reset/inc/get
 */
//--------------------------------------------------------------

//--------------------------------------------------------------
/**
 *  @brief  現在のステージ値をセットする
 */
//--------------------------------------------------------------
u16 BSUBWAY_ScoreData_SetStageValue(BSUBWAY_SCOREWORK* dat,u16 id,u16 value)
{
  u16 id2;

  //if(id >= BSWAY_MODE_RETRY){
  if(id == BSWAY_MODE_RETRY){
    return 0;  //リトライモードではカウントしない
  }

  //プラチナ追加
  if(id == BSWAY_MODE_WIFI_MULTI){
    id2 = 5;            //tower_stage[5]
  }else{
    id2 = id;
  }

  dat->tower_stage[id2] = value;
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
  return dat->tower_stage[id2];
}

//--------------------------------------------------------------
/**
 *  @brief  デバッグ限定　現在のステージ値をセットする
 */
//--------------------------------------------------------------
#ifdef PM_DEBUG
u16 BSUBWAY_ScoreData_DebugSetStageValue(BSUBWAY_SCOREWORK* dat,u16 id,u16 value)
{
  return BSUBWAY_ScoreData_SetStageValue( dat, id, value );
}
#endif  //PM_DEBUG
  
//--------------------------------------------------------------
/**
 *  @brief  スコアデータ フラグエリアセット
 */
//--------------------------------------------------------------
BOOL  BSUBWAY_ScoreData_SetFlags(
    BSUBWAY_SCOREWORK* dat,u16 id,BSWAY_DATA_SETID mode)
{
  u16  i;
  u16  flag = 1;

  //エラーチェック
  if( id >= BSWAY_SFLAG_MAX ){
    GF_ASSERT( (0) && "TowerScoreData_SetFlags IDが不正です！" );
    return 0;
  }

  //フラグID生成
  for(i = 0;i < id;i++){
    flag <<= 1;
  }
  switch(mode){
  case BSWAY_DATA_reset:
    flag = (flag^0xFFFF);
    dat->flags &= flag;
    break;
  case BSWAY_DATA_set:
    dat->flags |= flag;
    break;
  case BSWAY_DATA_get:
    return (BOOL)((dat->flags>>id)&0x0001);
    
  }
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
  return 0;
}

//--------------------------------------------------------------
/**
 *  @brief  サブウェイ用日付変化ランダムシード保存
 */
//--------------------------------------------------------------
void BSUBWAY_ScoreData_SetDayRndSeed(BSUBWAY_SCOREWORK* dat,u32 rnd_seed)
{
  dat->day_rnd_seed = rnd_seed;
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}

//--------------------------------------------------------------
/**
 *  @brief  サブウェイ用日付変化ランダムシード取得
 */
//--------------------------------------------------------------
u32 BSUBWAY_ScoreData_GetDayRndSeed(BSUBWAY_SCOREWORK* dat)
{
  return dat->day_rnd_seed;
}

//======================================================================
//　サブウェイ　プレイヤーメッセージデータアクセス系
//======================================================================
//--------------------------------------------------------------
/**
 *  @brief  簡易会話データをセット
 */
//--------------------------------------------------------------
#if 0 //wb
void BSUBWAY_PlayerMsg_Set(SAVEDATA* sv,BSWAY_PLAYER_MSG_ID id,PMS_DATA* src)
{
  FRONTIER_SAVEWORK* data = SaveData_Get(sv,GMDATA_ID_FRONTIER);
  
  PMSDAT_Copy(&(data->tower.player_msg.msg[id]),src);
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}
#endif

//--------------------------------------------------------------
/**
 *  @brief  簡易会話データを取得
 */
//--------------------------------------------------------------
#if 0 //wb
PMS_DATA* BSUBWAY_PlayerMsg_Get(SAVEDATA* sv,BSWAY_PLAYER_MSG_ID id)
{
  FRONTIER_SAVEWORK* data = SaveData_Get(sv,GMDATA_ID_FRONTIER);
  
  return &(data->tower.player_msg.msg[id]);
}
#endif

//======================================================================
//　サブウェイ　Wifiデータアクセス系
//======================================================================
#if 0 //wb
//--------------------------------------------------------------
/**
 *  @brief  指定したルームデータの取得フラグを立てる
 *
 *  @param  rank  1オリジンなので注意
 *  @param  roomno  1オリジンなので注意
 */
//--------------------------------------------------------------
void BSUBWAY_WifiData_SetRoomDataFlag(BSUBWAY_WIFI_DATA* dat,
    u8 rank,u8 roomno,RTCDate *day)
{
  u8  idx,ofs;
  u8  flag = 1;
  u16  roomid;
  
  if(roomno == 0 || roomno > 200){
    return;
  }
  if(rank == 0 || rank > 10){
    return;
  }
  //両方1オリジンなので-1して計算
  roomid = (rank-1)*200+(roomno-1);
  
  idx = roomid/8;
  ofs = roomid%8;
  flag <<= ofs;

  dat->flags[idx] |= flag;

  dat->day = RTCDate2GFDate(day);
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}

//--------------------------------------------------------------
/**
 *  @brief  ルームデータ取得フラグをクリアする
 */
//--------------------------------------------------------------
void BSUBWAY_WifiData_ClearRoomDataFlag(BSUBWAY_WIFI_DATA* dat)
{
  MI_CpuClear8(dat->flags,BSUBWAY_ROOM_DATA_FLAGS_LEN);
  MI_CpuClear8(&dat->day,sizeof(GF_DATE));
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}

//--------------------------------------------------------------
/**
 *  @brief  日付が変わっているかどうかチェック
 */
//--------------------------------------------------------------
static BOOL check_day(RTCDate* new,RTCDate* old)
{
  if(new->year > old->year){
    return TRUE;
  }
  if(new->month > old->month){
    return TRUE;
  }
  if(new->day > old->day){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 *  @brief  指定したルームのDLフラグが立っているかチェック
 *
 *  @param  rank  1オリジンなので注意
 *  @param  roomno  1オリジンなので注意
 */
//--------------------------------------------------------------
BOOL BSUBWAY_WifiData_CheckRoomDataFlag(BSUBWAY_WIFI_DATA* dat,u8 rank,u8 roomno,RTCDate* day)
{
  u8  idx,ofs;
  u8  flag = 1;
  u16  roomid;
  RTCDate old_day;
  
  if(roomno > 200 || rank > 10){
    return FALSE;
  }

  //最後にDLした日付から、日が変わっているかどうかチェック
  GFDate2RTCDate(dat->day,&old_day);
  if(check_day(day,&old_day)){
    //日が変わっているので、フラグ群をオールクリア
    BSUBWAY_WifiData_ClearRoomDataFlag(dat);
    return FALSE;
  }
  //両方1オリジンなので-1して計算
  roomid = (rank-1)*200+(roomno-1);
  
  idx = roomid/8;
  ofs = roomid%8;
  flag <<= ofs;

  if(dat->flags[idx] & flag){
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 *  @brief  プレイヤーデータが存在しているかチェック
 */
//--------------------------------------------------------------
BOOL BSUBWAY_WifiData_IsPlayerDataEnable(BSUBWAY_WIFI_DATA* dat)
{
  return (BOOL)dat->player_data_f;
}

//--------------------------------------------------------------
/**
 *  @brief  リーダーデータがあるかどうかチェック
 */
//--------------------------------------------------------------
BOOL BSUBWAY_WifiData_IsLeaderDataEnable(BSUBWAY_WIFI_DATA* dat)
{
  return (BOOL)dat->leader_data_f;
}

//--------------------------------------------------------------
/**
 *  @brief  プレイヤーデータをセーブ
 *  
 *  @param  rank  1オリジンなので注意 
 *  @param  roomno  1オリジンなので注意
 */
//--------------------------------------------------------------
void BSUBWAY_WifiData_SetPlayerData(BSUBWAY_WIFI_DATA* dat,DPW_BT_PLAYER* src,u8 rank,u8 roomno)
{
  MI_CpuCopy8(src,dat->player,
    sizeof(BSUBWAY_WIFI_PLAYER)*BSUBWAY_STOCK_WIFI_PLAYER_MAX);
  
  //roomnoとrankを保存
  dat->player_rank = rank;
  dat->player_room = roomno;
  dat->player_data_f = 1;
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}

//--------------------------------------------------------------
/**
 *  @brief  プレイヤーデータをクリア
 */
//--------------------------------------------------------------
void BSUBWAY_WifiData_ClearPlayerData(BSUBWAY_WIFI_DATA* dat)
{
  MI_CpuClear8(dat->player,
    sizeof(BSUBWAY_WIFI_PLAYER)*BSUBWAY_STOCK_WIFI_PLAYER_MAX);
  dat->player_data_f = 0;
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}

//--------------------------------------------------------------
/**
 *  @brief  プレイヤーデータのRoomID(ランクとroomNo)を取得
 *
 *  @param  roomid  BSUBWAY_ROOMID型(b_tower.hで公開)
 */
//--------------------------------------------------------------
void BSUBWAY_WifiData_GetPlayerDataRoomID(BSUBWAY_WIFI_DATA* dat,BSUBWAY_ROOMID *roomid)
{
  roomid->rank = dat->player_rank;
  roomid->no = dat->player_room;
}

//--------------------------------------------------------------
/**
 *  @brief  プレイヤーデータをサブウェイ戦闘用に解凍
 */
//--------------------------------------------------------------
void BSUBWAY_WifiData_GetBtlPlayerData(BSUBWAY_WIFI_DATA* dat,
    BSUBWAY_PARTNER_DATA* player,const u8 round)
{
  BSUBWAY_TRAINER  *tr;      //トレーナーデータ
  BSUBWAY_POKEMON  *poke;    //持ちポケモンデータ
  BSUBWAY_WIFI_PLAYER* src;
  MSGDATA_MANAGER* pMan;

  tr = &(player->bt_trd);
  poke = player->btpwd;
  src = &(dat->player[round]);

  //トレーナーパラメータ取得
  tr->player_id = BSUBWAY_TRAINER_ID;//src->id_no;  //サブウェイ用IDは固定値
  tr->tr_type = src->tr_type;
  //NGネームフラグチェック
  if(src->ngname_f){
    pMan = MSGMAN_Create(MSGMAN_TYPE_NORMAL,ARC_MSG,
        NARC_msg_btower_app_dat,HEAPID_WORLD);

    MSGMAN_GetStr(pMan,msg_def_player_name01+src->gender,tr->name);
    MSGMAN_Delete(pMan);
  }else{
    MI_CpuCopy8(src->name,tr->name,16);
  }
  MI_CpuCopy8(src->appear_word,tr->appear_word,8);
  MI_CpuCopy8(src->win_word,tr->win_word,8);
  MI_CpuCopy8(src->lose_word,tr->lose_word,8);

  //ポケモンデータ取得
  MI_CpuCopy8(src->poke,poke,sizeof(BSUBWAY_POKEMON)*3);
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}

/**
 *  @brief  リーダーデータをセーブ
 *  
 *  @param  rank  1オリジンなので注意
 *  @param  roomno  1オリジンなので注意
 */
void BSUBWAY_WifiData_SetLeaderData(BSUBWAY_WIFI_DATA* dat,DPW_BT_LEADER* src,u8 rank,u8 roomno)
{
  MI_CpuCopy8(src,&dat->leader,
    sizeof(BSUBWAY_LEADER_DATA)*BSUBWAY_STOCK_WIFI_LEADER_MAX);
  
  //roomnoとrankを保存
  dat->leader_rank = rank;
  dat->leader_room = roomno;
  dat->leader_data_f = 1;
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}

//--------------------------------------------------------------
/**
 *  @brief  リーダーデータをクリア
 */
//--------------------------------------------------------------
void BSUBWAY_WifiData_ClearLeaderData(BSUBWAY_WIFI_DATA* dat)
{
  MI_CpuClear8(&dat->leader,
    sizeof(BSUBWAY_LEADER_DATA)*BSUBWAY_STOCK_WIFI_LEADER_MAX);
  dat->leader_data_f = 0;
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
}

//--------------------------------------------------------------
/**
 *  @brief  リーダーデータのRoomID(ランクとroomNo)を取得
 *
 *  @param  roomid  BSUBWAY_ROOMID型(b_tower.hで公開)
 */
//--------------------------------------------------------------
void BSUBWAY_WifiData_GetLeaderDataRoomID(BSUBWAY_WIFI_DATA* dat,BSUBWAY_ROOMID *roomid)
{
  roomid->rank = dat->leader_rank;
  roomid->no = dat->leader_room;
}

//--------------------------------------------------------------
/**
 *  @brief  リーダーデータをAllocしたメモリにコピーして取得する
 *
 *  ＊内部でAllocしたメモリ領域を返すので、呼び出し側が明示的に解放すること！
 */
//--------------------------------------------------------------
BSUBWAY_LEADER_DATA* BSUBWAY_WifiData_GetLeaderDataAlloc(BSUBWAY_WIFI_DATA* dat,int heapID) 
{
  BSUBWAY_LEADER_DATA* bp;

  bp = sys_AllocMemory(heapID,sizeof(BSUBWAY_LEADER_DATA)*BSUBWAY_STOCK_WIFI_LEADER_MAX);
  MI_CpuCopy8(dat->leader,bp,sizeof(BSUBWAY_LEADER_DATA)*BSUBWAY_STOCK_WIFI_LEADER_MAX);

  return bp;
}
#endif

//======================================================================
//  サブウェイ　セーブデータブロック関連
//======================================================================
#if 0 //wb
//--------------------------------------------------------------
/**
 *  @brief  サブウェイ　プレイデータへのポインタを取得
 */
//--------------------------------------------------------------
BSUBWAY_PLAYWORK* SaveData_GetTowerPlayData(SAVEDATA* sv)
{
  FRONTIER_SAVEWORK* data = SaveData_Get(sv,GMDATA_ID_FRONTIER);
  
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_CheckCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
  return &data->play_tower;
}

//--------------------------------------------------------------
/**
 *  @brief  サブウェイ　スコアデータへのポインタを取得
 */
//--------------------------------------------------------------
BSUBWAY_SCOREWORK* SaveData_GetTowerScoreData(SAVEDATA* sv)
{
  FRONTIER_SAVEWORK* data = SaveData_Get(sv,GMDATA_ID_FRONTIER);
  
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_CheckCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
  return &data->tower.score;
}

//--------------------------------------------------------------
/**
 *  @brief  サブウェイ Wifiデータへのポインタを取得  
 */
//--------------------------------------------------------------
BSUBWAY_WIFI_DATA*  SaveData_GetTowerWifiData(SAVEDATA* sv)
{
  FRONTIER_SAVEWORK* data = SaveData_Get(sv,GMDATA_ID_FRONTIER);
  
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
  SVLD_CheckCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
  return &data->tower.wifi;
}
#endif

//----
#endif //wb null
//----
