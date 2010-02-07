//======================================================================
/**
 * @file bsubway_tr.c
 * @brief バトルサブウェイ　トレーナー関連
 * @author  Miyuki Iwasawa
 * @date  06.04.17
 * @note GSより移植 kaga
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "savedata/save_tbl.h"
#include "savedata/record.h"
#include "savedata/battle_rec.h"

#include "gamesystem/btl_setup.h"
#include "poke_tool/poke_tool.h"
#include "tr_tool/tr_tool.h"
#include "battle/battle.h"
#include "pm_define.h"
#include "buflen.h"
#include "sound/pm_sndsys.h"

#include "fldmmdl.h"

#include "message.naix"

#include "bsubway_scr_def.h"
#include "bsubway_scr.h"
#include "bsubway_scrwork.h"
#include "savedata/bsubway_savedata.h"

//#include "../mystery/fntequ.h"

#define __BSUBWAY_FLD_H_GLOBAL
#include "bsubway_tr.h"
#include "event_bsubway.h"

//正式データが出来るまでインクルード
#include "item/itemsym.h"
#include "poke_tool/monsno_def.h"
#include "waza_tool/wazano_def.h"
//#include "battle/battle_common.h"
#include "tr_tool/trtype_def.h"
#include "battle/bsubway_battle_data.h"
#include "tr_tool/trno_def.h"
//#include "b_tower_fld.dat"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================
static BOOL set_BSWayPokemonParam(
    BSUBWAY_SCRWORK* wk,BSUBWAY_TRAINER_ROM_DATA *trd,
    u16 tr_no,BSUBWAY_POKEMON *pwd,u8 cnt,
    u16 *set_poke_no,u16 *set_item_no,
    BSUBWAY_PAREPOKE_PARAM *poke, HEAPID heapID );

static void * get_TrainerRomData(u16 tr_no,HEAPID heapID);

static void get_PokemonRomData(
    BSUBWAY_POKEMON_ROM_DATA *prd,int index);

static STRCODE * PM_strcpy( STRCODE* to_str, const STRCODE* from_str );
static u16 get_Rand( BSUBWAY_SCRWORK *wk );

//======================================================================
//  トレーナータイプ
//======================================================================
//--------------------------------------------------------------
/// タワーに出現するトレーナータイプ←→OBJコード
//--------------------------------------------------------------
static const u16 btower_trtype2objcode[][2] =
{
 {TRTYPE_TANPAN,  BOY2},  ///<たんパンこぞう
 {TRTYPE_MINI,  GIRL1},  ///<ミニスカート
 {TRTYPE_SCHOOLB,  BOY1},  ///<じゅくがえり
// {TRTYPE_PL_SCHOOLG,  GIRL3},  ///<じゅくがえり
// {TRTYPE_PL_PRINCE,  GORGGEOUSM},  ///<おぼっちゃま
// {TRTYPE_PL_PRINCESS,  GORGGEOUSW},  ///<おじょうさま
// {TRTYPE_CAMPB,  CAMPBOY},  ///<キャンプボーイ
// {TRTYPE_PICNICG,  PICNICGIRL},  ///<ピクニックガール
// {TRTYPE_PL_UKIWAB,  BABYBOY1},  ///<うきわボーイ
// {TRTYPE_PL_UKIWAG,  BABYGIRL1},  ///<うきわガール
 {TRTYPE_DAISUKIM,  MIDDLEMAN1},  ///<だいすきクラブ
 {TRTYPE_DAISUKIW,  MIDDLEWOMAN1},  ///<だいすきクラブ
// {TRTYPE_PL_WAITER,  WAITER},  ///<ウエーター
// {TRTYPE_PL_WAITRESS,  WAITRESS},  ///<ウエートレス
// {TRTYPE_PL_BREEDERM,  MAN1},  ///<ポケモンブリーダー
// {TRTYPE_PL_BREEDERW,  WOMAN1},  ///<ポケモンブリーダー
// {TRTYPE_PL_CAMERAMAN,  CAMERAMAN},  ///<カメラマン
// {TRTYPE_PL_REPORTER,  REPORTER},  ///<レポーター
// {TRTYPE_PL_FARMER,  FARMER},  ///<ぼくじょうおじさん
// {TRTYPE_PL_COWGIRL,  COWGIRL},  ///<カウガール
// {TRTYPE_PL_CYCLINGM,  CYCLEM},  ///<サイクリング♂
// {TRTYPE_PL_CYCLINGW,  CYCLEW},  ///<サイクリング♀
 {TRTYPE_KARATE,  FIGHTERM},  ///<からておう
// {TRTYPE_PL_BATTLEG,  GIRL2},  ///<バトルガール
// {TRTYPE_PL_VETERAN,  OLDMAN1},  ///<ベテラントレーナー
// {TRTYPE_PL_MADAM,  LADY},  ///<マダム
 {TRTYPE_ESPM,  MAN1},  ///<サイキッカー
// {TRTYPE_PL_ESPW,  MYSTERY},  ///<サイキッカー
// {TRTYPE_PL_RANGERM,  MAN3},  ///<ポケモンレンジャー
// {TRTYPE_PL_RANGERW,  WOMAN3},  ///<ポケモンレンジャー
// {TRTYPE_ELITEM,  MAN3},  ///<エリートトレーナー
// {TRTYPE_ELITEW,  WOMAN3},  ///<エリートトレーナー
// {TRTYPE_PL_COLDELITEM,  MAN5},  ///<エリートトレーナー♂（厚着）
// {TRTYPE_PL_COLDELITEW,  WOMAN5},  ///<エリートトレーナー♀（厚着）
// {TRTYPE_PL_DRAGON,  MAN3},  ///<ドラゴンつかい
// {TRTYPE_MUSHI,  BOY3},  ///<むしとりしょうねん
// {TRTYPE_PL_SHINOBI,  BABYBOY1},  ///<にんじゃごっこ
// {TRTYPE_PL_JOGGER,  SPORTSMAN},  ///<ジョギング♂
 {TRTYPE_FISHING,  FISHING},  ///<つりびと
// {TRTYPE_SAILOR,  SEAMAN},  ///<ふなのり
 {TRTYPE_MOUNT,  MAN1},  ///<やまおとこ
// {TRTYPE_PL_ISEKI,  EXPLORE},  ///<いせきマニア
// {TRTYPE_GUITARIST,  MAN2},  ///<ギタリスト
// {TRTYPE_PL_COLLECTOR,  BIGMAN},  ///<ポケモンコレクター
// {TRTYPE_PL_HEADS,  BADMAN},  ///<スキンヘッズ
// {TRTYPE_SCIENTIST,  ASSISTANTM},  ///<けんきゅういん♂
 {TRTYPE_GENTLE,  GENTLEMAN},  ///<ジェントルマン
// {TRTYPE_PL_WORKER,  WORKMAN},  ///<さぎょういん
// {TRTYPE_PL_PIERROT,  CLOWN},  ///<ピエロ
// {TRTYPE_POLICE,  POLICEMAN},  ///<おまわりさん
// {TRTYPE_PL_GAMBLER,  GORGGEOUSM},  ///<ギャンブラー
// {TRTYPE_BIRD,  WOMAN3},  ///<とりつかい
// {TRTYPE_PL_PARASOL,  AMBRELLA},  ///<パラソルおねえさん
// {TRTYPE_SISTER,  WOMAN2},  ///<おとなのおねえさん
// {TRTYPE_PL_AROMA,  WOMAN1},  ///<アロマなおねえさん
// {TRTYPE_PL_IDOL,  IDOL},  ///<アイドル
// {TRTYPE_PL_ARTIST,  ARTIST},  ///<げいじゅつか
// {TRTYPE_PL_POKEGIRL,  PIKACHU},  ///<ポケモンごっこ♀
};

#define TRTYPE2OBJCODE_MAX  (NELEMS(btower_trtype2objcode))

//--------------------------------------------------------------
/**
 * @brief  トレーナータイプから人物OBJコードを返す
 * @param tr_type トレーナータイプ
 * @retval u16 OBJコード
 */
//--------------------------------------------------------------
u16 BSUBWAY_GetTrainerOBJCode( u8 tr_type )
{
  int i;

  for( i = 0; i < TRTYPE2OBJCODE_MAX; i++ ){
    if( btower_trtype2objcode[i][0] == tr_type ){
      return btower_trtype2objcode[i][1];
    }
  }
  return BOY1;
}

//--------------------------------------------------------------
/**
 * @brief  トレーナータイプが対応しているか
 * @param tr_type トレーナータイプ
 * @retval BOOL TRUE=対応
 */
//--------------------------------------------------------------
static BOOL check_TrainerType(u8 tr_type)
{
  int i;

  for( i = 0;i < TRTYPE2OBJCODE_MAX; i++ ){
    if( btower_trtype2objcode[i][0] == tr_type ){
      return( TRUE );
    }
  }
  return( FALSE );
}

//======================================================================
//  バトルサブウェイ関連
//======================================================================
//--------------------------------------------------------------
/**
 *  @brief バトルサブウェイ用ポケモンデータからPOKEMON_PARAMを生成
 *  @param src BSUBWAY_POKEMON
 *  @param dest POKEMON_PARAM
 *  @retval nothing
 */
//--------------------------------------------------------------
static void make_PokePara(
    const BSUBWAY_POKEMON *src, POKEMON_PARAM *dest )
{
  int i;
  u8 buf8,waza_pp;
  u16 buf16;
  u32 buf32;

  PP_Clear(dest);

  //monsno,level,pow_rnd,idno
  PP_SetupEx( dest, src->mons_no, 50, PTL_SETUP_ID_AUTO,
      (src->power_rnd & 0x3FFFFFFF), src->personal_rnd);

  //form_no
  buf8 = src->form_no;
  PP_Put( dest, ID_PARA_form_no, buf8 );

  //装備アイテム設定
  PP_Put( dest,ID_PARA_item, src->item_no );

  //技設定
  for(i = 0;i < 4;i++){
    buf16 = src->waza[i];
    PP_Put(dest,ID_PARA_waza1+i,buf16);
    buf8 = (src->pp_count >> (i*2))&0x03;
    PP_Put(dest,ID_PARA_pp_count1+i, buf8);

    //pp再設定
    waza_pp = (u8)PP_Get(dest,ID_PARA_pp_max1+i,NULL);
    PP_Put(dest,ID_PARA_pp1+i,waza_pp);
  }

  //ID設定
  buf32 = src->id_no;
  PP_Put(dest,ID_PARA_id_no,buf32);

  //経験値設定
  buf8 = src->hp_exp;
  PP_Put(dest,ID_PARA_hp_exp,buf8);
  buf8 = src->pow_exp;
  PP_Put(dest,ID_PARA_pow_exp,buf8);
  buf8 = src->def_exp;
  PP_Put(dest,ID_PARA_def_exp,buf8);
  buf8 = src->agi_exp;
  PP_Put(dest,ID_PARA_agi_exp,buf8);
  buf8 = src->spepow_exp;
  PP_Put(dest,ID_PARA_spepow_exp,buf8);
  buf8 = src->spedef_exp;
  PP_Put(dest,ID_PARA_spedef_exp,buf8);

  //特性設定
  PP_Put(dest,ID_PARA_speabino, src->tokusei);

  //なつきど設定
  PP_Put(dest,ID_PARA_friend,src->natuki);

  //NGネームフラグをチェック
  if( 1 ){ //デフォルトネームを展開する
#if 0
    GFL_MSGDATA *msgdata;
    STRBUF *def_name;

    msgdata= GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
        ARCID_MESSAGE, NARC_message_monsname_dat, HEAPID_PROC );
    def_name = GFL_STR_CreateBuffer( HEAPID_PROC, MONS_NAME_SIZE+EOM_SIZE );

    GFL_MSG_GetString( msgdata, src->mons_no, def_name );
    PP_Put( dest, ID_PARA_nickname, (u32)def_name );

    GFL_STR_DeleteBuffer( def_name );
    GFL_MSG_Delete( msgdata );
#endif
  }else{ //ニックネーム
    STRBUF *nick_name;
    nick_name = GFL_STR_CreateBuffer(
        HEAPID_PROC, MONS_NAME_SIZE+EOM_SIZE );
    GFL_STR_SetStringCode( nick_name, src->nickname );
    PP_Put( dest, ID_PARA_nickname, (u32)nick_name );
    GFL_STR_DeleteBuffer( nick_name );
  }

  PP_Put(dest,ID_PARA_country_code,src->country_code); //カントリーコード

  //パラメータ再計算
  {
    u32 lv = PP_CalcLevel( dest );
    PP_Put( dest, ID_PARA_level,lv );
  }
}

//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　プレイモードからFIGHT_TYPEを返す
 * @param play_mode プレイモード
 * @retval u32 FIGHT_TYPE
 */
//--------------------------------------------------------------
static u32 get_FightType(u8 play_mode)
{
  switch(play_mode){
  case BSWAY_MODE_SINGLE:
  case BSWAY_MODE_WIFI:
//    return FIGHT_TYPE_BATTLE_TOWER_1vs1;
    return BTL_RULE_SINGLE;
  case BSWAY_MODE_DOUBLE:
//  return FIGHT_TYPE_BATTLE_TOWER_2vs2;
    return BTL_RULE_DOUBLE;
#if 0 //wb null
  case BSWAY_MODE_MULTI:
    return FIGHT_TYPE_BATTLE_TOWER_AI_MULTI;
  case BSWAY_MODE_COMM_MULTI:
  case BSWAY_MODE_WIFI_MULTI:
    return FIGHT_TYPE_BATTLE_TOWER_SIO_MULTI;
#endif
  }
  return BTL_RULE_SINGLE;
}

//--------------------------------------------------------------
/**
 * @brief  バトルサブウェイ　トレーナーデータ生成
 * @param heapID HEAPID
 * @retval BSP_TRAINER_DATA*
 */
//--------------------------------------------------------------
static BSP_TRAINER_DATA * create_BSP_TRAINER_DATA( HEAPID heapID )
{
  BSP_TRAINER_DATA* tr_data = GFL_HEAP_AllocClearMemory( heapID, sizeof( BSP_TRAINER_DATA ) );
  tr_data->name =   GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, heapID );
  return tr_data;
}

//--------------------------------------------------------------
/**
 * BATTLE_SETUP_PARAM作成
 * @param bsw_scr BSUBWAY_SCRWORK
 * @param gsys GAMESYS_WORK
 * @retval BATTLE_SETUP_PARAM
 */
//--------------------------------------------------------------
BATTLE_SETUP_PARAM * BSUBWAY_SCRWORK_CreateBattleParam(
    BSUBWAY_SCRWORK *wk, GAMESYS_WORK *gsys )
{
  BATTLE_SETUP_PARAM *dst;
  BTL_FIELD_SITUATION sit;
  GAMEDATA *gameData = GAMESYSTEM_GetGameData( gsys );

  dst = BATTLE_PARAM_Create( HEAPID_PROC );

  {
    BTL_FIELD_SITUATION_Init( &sit );

    dst->netHandle = NULL;
    dst->commMode = BTL_COMM_NONE;
    dst->commPos = 0;
//  dst->netID = 0;
    dst->multiMode = 0;
    dst->recBuffer = NULL;
    dst->fRecordPlay = FALSE;

    dst->party[BTL_CLIENT_PLAYER] = NULL;
    dst->party[BTL_CLIENT_ENEMY1] = NULL;
    dst->party[BTL_CLIENT_PARTNER] = NULL;
    dst->party[BTL_CLIENT_ENEMY2] = NULL;

    dst->competitor = BTL_COMPETITOR_TRAINER;

    dst->playerStatus[BTL_CLIENT_PLAYER] = GAMEDATA_GetMyStatus( gameData );
    dst->playerStatus[BTL_CLIENT_ENEMY1] = NULL;
    dst->playerStatus[BTL_CLIENT_PARTNER] = NULL;
    dst->playerStatus[BTL_CLIENT_ENEMY2] = NULL;

    dst->itemData     = GAMEDATA_GetMyItem( gameData );
    dst->bagCursor    = GAMEDATA_GetBagCursor( gameData );
    dst->zukanData    = GAMEDATA_GetZukanSave( gameData );
//  dst->commSupport  = GAMEDATA_GetCommPlayerSupportPtr( gameData );
    dst->commSupport  = NULL;

    {
      SAVE_CONTROL_WORK *saveCtrl = GAMEDATA_GetSaveControlWork( gameData );
      dst->configData = SaveData_GetConfig( saveCtrl );
    }

    MI_CpuCopy8( &sit, &dst->fieldSituation, sizeof(BTL_FIELD_SITUATION) );

    dst->musicDefault = SEQ_BGM_VS_NORAPOKE;
    dst->musicPinch = SEQ_BGM_BATTLEPINCH;
    dst->result = BTL_RESULT_WIN;
  }

  BTL_SETUP_SetSubwayMode( dst );

  switch( wk->play_mode )
  {
  case BSWAY_MODE_SINGLE:
  case BSWAY_MODE_WIFI:
    dst->rule = BTL_RULE_SINGLE;
    break;
  case BSWAY_MODE_DOUBLE:
  case BSWAY_MODE_MULTI:
  case BSWAY_MODE_COMM_MULTI:
    dst->rule = BTL_RULE_DOUBLE;
    break;
  default:
    GF_ASSERT( 0 );
  }
  
  { //トレーナーデータ確保
    dst->tr_data[BTL_CLIENT_PLAYER] = create_BSP_TRAINER_DATA( HEAPID_PROC );
    dst->tr_data[BTL_CLIENT_ENEMY1] = create_BSP_TRAINER_DATA( HEAPID_PROC );
  }

  { //敵トレーナーセット
    PMS_DATA *pd;
    BTL_CLIENT_ID client = BTL_CLIENT_ENEMY1;
    POKEPARTY **party = &dst->party[client];
    BSP_TRAINER_DATA *tr_data = dst->tr_data[client];

    BSUBWAY_PARTNER_DATA *bsw_partner = &wk->tr_data[0];
    BSUBWAY_TRAINER *bsw_trainer = &bsw_partner->bt_trd;

    tr_data->tr_id = bsw_trainer->player_id;
    tr_data->tr_type = bsw_trainer->tr_type;
    tr_data->ai_bit = 0xFFFFFFFF;  //最強

    //name
    GFL_STR_SetStringCode( tr_data->name, bsw_trainer->name );

    //win word
    pd = (PMS_DATA*)bsw_trainer->win_word;
    tr_data->win_word = *pd;

    pd = (PMS_DATA*)bsw_trainer->lose_word;
    tr_data->lose_word = *pd;

    //敵ポケモンセット
    {
      int i;
      POKEMON_PARAM*  pp;

      pp = GFL_HEAP_AllocMemoryLo( HEAPID_PROC, POKETOOL_GetWorkSize() );
      *party = PokeParty_AllocPartyWork( HEAPID_PROC );
      PokeParty_Init( *party, TEMOTI_POKEMAX );

      for( i = 0; i < wk->member_num; i++ ){
        make_PokePara( &(bsw_partner->btpwd[i]), pp );
        PokeParty_Add( *party, pp );
      }
      GFL_HEAP_FreeMemory( pp );
    }
  }

  { //プレイヤーセット
    BTL_CLIENT_ID client = BTL_CLIENT_PLAYER;
    POKEPARTY **party = &dst->party[client];
    BSP_TRAINER_DATA *data = dst->tr_data[client];
    PLAYER_WORK * player = GAMEDATA_GetMyPlayerWork( gameData );

    MyStatus_CopyNameString(
        (const MYSTATUS*)&player->mystatus, data->name );

    data->tr_type = TRTYPE_HERO +
      MyStatus_GetMySex((const MYSTATUS*)&player->mystatus );

    //ポケモンセット
    {
      int i;
      POKEMON_PARAM *entry_pp;
      const POKEMON_PARAM *my_pp;
      const POKEPARTY *myparty = GAMEDATA_GetMyPokemon(gameData);

      entry_pp = GFL_HEAP_AllocMemoryLo(
          HEAPID_PROC, POKETOOL_GetWorkSize() );
      PP_Clear( entry_pp );

      *party = PokeParty_AllocPartyWork( HEAPID_PROC );
      PokeParty_Init( *party, TEMOTI_POKEMAX );

      for( i = 0; i < wk->member_num; i++ ){
        my_pp = PokeParty_GetMemberPointer( myparty,
            wk->member[i] );

        POKETOOL_CopyPPtoPP( (POKEMON_PARAM*)my_pp, entry_pp );

        if( PP_Get(my_pp,ID_PARA_level,NULL) != 50 ){
          u32 exp = POKETOOL_GetMinExp(
              PP_Get(my_pp,ID_PARA_monsno,NULL),
              PP_Get(my_pp,ID_PARA_form_no,NULL),
              50 );

          PP_Put( entry_pp, ID_PARA_exp, exp );
          PP_Renew( entry_pp );
        }

#ifdef DEBUG_ONLY_FOR_kagaya
        PP_Put( entry_pp, ID_PARA_hp, 1 );
#endif
        PokeParty_Add( *party, entry_pp );
      }

      #ifdef DEBUG_ONLY_FOR_kagaya
      {
        int count = PokeParty_GetPokeCount( *party );
        KAGAYA_Printf( "ポケモンセット　カウント=%d, max=%d\n", count, i );
      }
      #endif

      GFL_HEAP_FreeMemory( entry_pp );
    }
  }

  return dst;
}

#if 0
  dst->musicDefault = SEQ_BGM_VS_NORAPOKE;
  dst->musicPinch = SEQ_BGM_BATTLEPINCH;

  dst->result = BTL_RESULT_WIN;

  sv = fsys->savedata;
  party = SaveData_GetTemotiPokemon(sv);

  BattleParam_SetParamByGameDataCore(bp,fsys);
  bp->bg_id = BG_ID_ROOM_A;      //基本背景指定
  bp->ground_id = GROUND_ID_FLOOR;  //基本地面指定

  //ポケモンデータセット
  pp = PokemonParam_AllocWork(wk->heapID);

  //選んだ手持ちポケモンをセット
  val8 = 50;
  PokeParty_Init(bp->poke_party[POKEPARTY_MINE],wk->member_num);
  for(i = 0;i < wk->member_num;i++){
    PokeCopyPPtoPP(PokeParty_GetMemberPointer(party,wk->member[i]),pp);

    //レベル調整
    if(PokeParaGet(pp,ID_PARA_level,NULL) > val8){
      val32 = PokeLevelExpGet(PokeParaGet(pp,ID_PARA_monsno,NULL),val8);

      PokeParaPut(pp,ID_PARA_exp,&val32);
      PokeParaCalc(pp);
    }
    BattleParam_AddPokemon(bp,pp,POKEPARTY_MINE);
  }
  sys_FreeMemoryEz(pp);

  //トレーナーデータ生成（自分側）
  BattleParam_TrainerDataMake(bp);

  //トレーナーデータ(enemy1)をセット
  BattleTowerTrainerDataMake(bp,&(wk->tr_data[0]),wk->member_num,CLIENT_NO_ENEMY,wk->heapID);

  switch(wk->play_mode){
  case BTWR_MODE_MULTI:
    //ペアデータをセット
    BattleTowerTrainerDataMake(bp,&(wk->five_data[wk->partner]),wk->member_num,CLIENT_NO_MINE2,wk->heapID);
    //↓ここは共通処理で流れていい
  case BTWR_MODE_COMM_MULTI:
    //トレーナーデータ(enemy2)をセット
    BattleTowerTrainerDataMake(bp,&(wk->tr_data[1]),wk->member_num,CLIENT_NO_ENEMY2,wk->heapID);
    break;
  default:
    break;
  }
#endif

//======================================================================
//  通信関連コマンド
//======================================================================
#if 0
/**
 *  @brief  バトルサブウェイ　送られてきたプレイヤーデータを受け取る
 */
u16 BTowerComm_RecvPlayerData(FIELDSYS_WORK* fsys,const u16* recv_buf)
{
  u16  ret = 0;
  BSUBWAY_SCRWORK* wk = fsys->btower_wk;

  wk->pare_sex = (u8)recv_buf[0];
  wk->pare_poke[0] = recv_buf[1];
  wk->pare_poke[1] = recv_buf[2];
  wk->pare_stage = recv_buf[3];

  wk->partner = 5+wk->pare_sex;

  OS_Printf("sio multi mem = %d,%d:%d,%d\n",wk->mem_poke[0],wk->mem_poke[1],wk->pare_poke[0],wk->pare_poke[1]);
  if(  wk->mem_poke[0] == wk->pare_poke[0] ||
    wk->mem_poke[0] == wk->pare_poke[1]){
    ret += 1;
  }
  if(  wk->mem_poke[1] == wk->pare_poke[0] ||
    wk->mem_poke[1] == wk->pare_poke[1]){
    ret += 2;
  }
  return ret;
}

/**
 *  @brief  バトルサブウェイ　送られてきたトレーナーデータを受け取る
 */
u16  BTowerComm_RecvTrainerData(FIELDSYS_WORK* fsys,const u16* recv_buf)
{
  int i;
  BSUBWAY_SCRWORK* wk = fsys->btower_wk;

  if(CommGetCurrentID() == COMM_PARENT_ID){
    return 0;  //親は送信するだけなので受け取らない
  }

  MI_CpuCopy8(recv_buf,wk->trainer,BSUBWAY_STOCK_TRAINER_MAX*2);
  OS_Printf("sio multi trainer01 = %d,%d:%d,%d\n",
      wk->trainer[0],wk->trainer[1],wk->trainer[2],wk->trainer[3]);
  OS_Printf("sio multi trainer02 = %d,%d:%d,%d\n",
      wk->trainer[4],wk->trainer[5],wk->trainer[6],wk->trainer[7]);
  OS_Printf("sio multi trainer03 = %d,%d:%d,%d\n",
      wk->trainer[8],wk->trainer[9],wk->trainer[10],wk->trainer[11]);
  OS_Printf("sio multi trainer04 = %d,%d\n",
      wk->trainer[12],wk->trainer[13]);
  return 1;
}

/**
 *  @brief  バトルサブウェイ　送られてきたリタイアするかどうかの結果を受け取る
 *
 *  @retval  0  リタイアしない
 *  @retval  1  リタイアする
 */
u16  BTowerComm_RecvRetireSelect(FIELDSYS_WORK* fsys,const u16* recv_buf)
{
  int i;
  BSUBWAY_SCRWORK* wk = fsys->btower_wk;

  OS_Printf("sio multi retire = %d,%d\n",wk->retire_f,recv_buf[0]);
  if(wk->retire_f || recv_buf[0]){
    return 1;
  }
  return 0;
}

/**
 *  @brief  バトルサブウェイ　自機性別とモンスターNoを送信
 */
void BTowerComm_SendPlayerData(BSUBWAY_SCRWORK* wk,SAVEDATA *sv)
{
  int i;
  POKEPARTY* party;
  POKEMON_PARAM *pp;
  MYSTATUS  *my = SaveData_GetMyStatus(sv);

  wk->send_buf[0] = MyStatus_GetMySex(my);
  party = SaveData_GetTemotiPokemon(sv);
  for(i = 0;i < 2;i++){
    wk->send_buf[1+i] =
      PP_Get(PokeParty_GetMemberPointer(party,wk->member[i]),
            ID_PARA_monsno,NULL);
  }

  //ここはBSWAY_MODE_COMM_MULTI専用
  OS_Printf( "wk->play_mode = %d\n", wk->play_mode );
  wk->send_buf[3] =
    TowerScoreData_SetStage(wk->scoreSave,BSWAY_MODE_COMM_MULTI,BSWAY_DATA_get);
}

/**
 *  @brief  バトルサブウェイ　通信マルチ　抽選したトレーナーNoを子機に送信
 */
void BTowerComm_SendTrainerData(BSUBWAY_SCRWORK* wk)
{
  MI_CpuCopy8(wk->trainer,wk->send_buf,BSUBWAY_STOCK_TRAINER_MAX*2);
}

/**
 *  @brief  バトルサブウェイ　通信マルチ　リタイアするかどうかを互いに送信
 *
 *  @param  retire  TRUEならリタイア
 */
void BTowerComm_SendRetireSelect(BSUBWAY_SCRWORK* wk,u16 retire)
{
  //自分の選択結果をワークに保存
  wk->retire_f = retire;
  wk->send_buf[0] = retire;
}
#endif

//======================================================================
//  トレーナーデータ
//======================================================================
//--------------------------------------------------------------
//  トレーナー番号テーブル　その１
//--------------------------------------------------------------
static const u16 TrainerNoRangeTable[][2]={
  {  1-1,100-1},
  { 81-1,120-1},
  {101-1,140-1},
  {121-1,160-1},
  {141-1,180-1},
  {161-1,200-1},
  {181-1,220-1},
  {201-1,300-1},
};

//--------------------------------------------------------------
//  トレーナー番号テーブル　その１２
//--------------------------------------------------------------
static const u16 TrainerNoRangeTable2[][2]={
  {101-1,120-1},
  {121-1,140-1},
  {141-1,160-1},
  {161-1,180-1},
  {181-1,200-1},
  {201-1,220-1},
  {221-1,240-1},
  {201-1,300-1},
};

#define TOWER_MASTER_FIRST  (305)    //0オリジン
#define TOWER_MASTER_SECOND  (306)

//--------------------------------------------------------------
/**
 * @brief  トレーナーナンバー抽選
 * @param bsw_scr BSUBWAY_SCRWORK
 * @param stage ステージ数
 * @param round 何人目か
 * @param play_mode プレイモード
 * @retval u16 トレーナーナンバー
 */
//--------------------------------------------------------------
u16 BSUBWAY_SCRWORK_GetTrainerNo(
    BSUBWAY_SCRWORK* wk, u16 stage, u8 round, int play_mode )
{
  u16  no;

  OS_Printf( "stage = %d\n", stage );
  OS_Printf( "round = %d\n", round );

  //タワータイクーンはシングルのみ
  if( play_mode == BSWAY_MODE_SINGLE ){
    //タワータイクーン1回目
    if( (stage==2) && (round==6) ){
      return TOWER_MASTER_FIRST;
    }
    //タワータイクーン2回目
    if((stage==6)&&(round==6)){
      return TOWER_MASTER_SECOND;
    }
  }
  
  if(stage<7){
    if(round==(6)){
      no=(TrainerNoRangeTable2[stage][1]-TrainerNoRangeTable2[stage][0])+1;
//      no=TrainerNoRangeTable2[stage][0]+(gf_rand()%no);
      no= TrainerNoRangeTable2[stage][0]+(get_Rand(wk)%no);
    }
    else{
      no=(TrainerNoRangeTable[stage][1]-TrainerNoRangeTable[stage][0])+1;
//      no=TrainerNoRangeTable[stage][0]+(gf_rand()%no);
      no=TrainerNoRangeTable[stage][0]+(get_Rand(wk)%no);
    }
  }
  else{
    no=(TrainerNoRangeTable[7][1]-TrainerNoRangeTable[7][0])+1;
//    no=TrainerNoRangeTable[7][0]+(gf_rand()%no);
    no=TrainerNoRangeTable[7][0]+(get_Rand(wk)%no);
  }
  return no;
}

//--------------------------------------------------------------
/**
 *  バトルサブウェイ　romトレーナーデータ展開
 *  BSUBWAY_TRAINER_ROM_DATA型をメモリ確保して返すので、
 *  呼び出し側が明示的に解放すること
 *  @param tr_data BSUBWAY_PARTER_DATA
 *  @param tr_no トレーナーナンバー
 *  @param heapID HEAPID
 *  @retval BSUBWAY_TRAINER_ROM_DATA*
 *  TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 *  TOWER_AI_MULTI_ONLY 似た処理frontier_tool.c Frontier_TrainerDataGet
 */
//--------------------------------------------------------------
static BSUBWAY_TRAINER_ROM_DATA * alloc_TrainerRomData(
    BSUBWAY_PARTNER_DATA *tr_data, u16 tr_no, HEAPID heapID )
{
  BSUBWAY_TRAINER_ROM_DATA  *trd;
  GFL_MSGDATA *msgdata;
  STRBUF *name;

  msgdata =  GFL_MSG_Create(
      GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
      NARC_message_btdtrname_dat, heapID );

  MI_CpuClear8(tr_data, sizeof(BSUBWAY_PARTNER_DATA));
  trd=get_TrainerRomData(tr_no,heapID);

  //トレーナーIDをセット
  tr_data->bt_trd.player_id=tr_no;

  //トレーナー出現メッセージ
  tr_data->bt_trd.appear_word[0] = 0xFFFF;
  tr_data->bt_trd.appear_word[1] = tr_no*3;

  //トレーナーデータをセット
  tr_data->bt_trd.tr_type=trd->tr_type;

  //GSデータからの移植による処理
  //wbでは存在していないタイプを書き換え
  #if 1
  if( check_TrainerType(trd->tr_type) == FALSE ){
    tr_data->bt_trd.tr_type = TRTYPE_TANPAN;
  }
  #endif

  name = GFL_MSG_CreateString( msgdata, tr_no );
  GFL_STR_GetStringCode( name,
      &tr_data->bt_trd.name[0], BUFLEN_PERSON_NAME );
  GFL_STR_DeleteBuffer(name);
  GFL_MSG_Delete( msgdata );
  return trd;
}

//--------------------------------------------------------------
/// アイテムテーブル
//  持ちポケモン決定はランダムでしているが無限ループ防止のため、
//  ある程度まわしたら、ポケモンの不一致のみをチェックして、
//  アイテムを固定で持たせるためのアイテムテーブル
//--------------------------------------------------------------
static const u16 BattleTowerPokemonItem[]={
  ITEM_HIKARINOKONA,
  ITEM_RAMUNOMI,
  ITEM_TABENOKOSI,
  ITEM_SENSEINOTUME,
};

//--------------------------------------------------------------
/**
 * ポケモンの性格を取得
 * 性格は、個性乱数を25で割った余りから算出される
 * @param[in]  rnd  取得したい個性乱数
 * @return  取得した性格
 */
//--------------------------------------------------------------
static u8  get_PokeSeikaku(u32 rnd)
{
  return (u8)(rnd % 25);
}

//--------------------------------------------------------------
/**
 *  @brief  バトルサブウェイのポケモンパラメータ生成
 *  @param pwd  [in/out]ポケモンパラメータの展開場所
 *  @param poke_no  [in]タワーromデータポケモンナンバー
 *  @param poke_id  [in]ポケモンにセットするid
 *  @param poke_rnd [in]セットする個性乱数(0が引き渡されたら関数内で生成)
 *  @param pow_rnd  [in]ポケモンにセットするpow_rnd値
 *  @param mem_idx  [in]メンバーindex。一体目or二体目
 *  @param itemfix  [in]TRUEなら固定アイテム。FALSEならromデータのアイテム
 *  @param heapID [in]テンポラリメモリを確保するヒープID
 *  @retval u32 personal_rnd:生成されたポケモンの個性乱数値
 * TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 * TOWER_AI_MULTI_ONLY 似た処理frontier_tool.c Frontier_PokemonParamMake
 */
//--------------------------------------------------------------
static u32 make_PokemonParam(
    BSUBWAY_SCRWORK *bsw_scr, BSUBWAY_POKEMON *pwd,
    u16 poke_no, u32 poke_id, u32 poke_rnd, u8 pow_rnd,
    u8 mem_idx, BOOL itemfix, HEAPID heapID )
{
  int i;
  int  exp;
  u32  personal_rnd;
  u8  friend;
  BSUBWAY_POKEMON_ROM_DATA  prd_s;

  MI_CpuClear8(pwd,sizeof(BSUBWAY_POKEMON));

  //ロムデータロード
  get_PokemonRomData(&prd_s,poke_no);

  //モンスターナンバー
  pwd->mons_no=prd_s.mons_no;

  //フォルムナンバー
  pwd->form_no=prd_s.form_no;

  //装備道具
  if(itemfix){
    //50回以上まわしていたフラグがTUREなら、
    //装備アイテムは固定のものを持たせる
    pwd->item_no=BattleTowerPokemonItem[mem_idx];
  }else{
    //romデータに登録されたアイテムを持たせる
    pwd->item_no=prd_s.item_no;
  }

  //なつき度は255がデフォルト
  friend=255;
  for(i=0;i<WAZA_TEMOTI_MAX;i++){
    pwd->waza[i]=prd_s.waza[i];
    //やつあたりを持っているときは、なつき度を0にする
    if(prd_s.waza[i]==WAZANO_YATUATARI){
      friend=0;
    }
  }

  //IDNo
  pwd->id_no=poke_id;

  if(poke_rnd == 0){
    //個性乱数
    do{
//    personal_rnd=(gf_rand()|gf_rand()<<16);
      personal_rnd=(get_Rand(bsw_scr)|get_Rand(bsw_scr)<<16);
#if 0
    }while((prd_s.chr!=get_PokeSeikaku(personal_rnd))&&(PokeRareGetPara(poke_id,personal_rnd)==TRUE));
#else
    //プラチナはタワーも修正する(08.03.17)(似た処理がfrontier_tool.cにもあるので注意！)
    //データの性格と一致していない"もしくは"レアの時は、ループを回す
    }while((prd_s.chr!=get_PokeSeikaku(personal_rnd))||(
          POKETOOL_CheckRare(poke_id,personal_rnd)==TRUE));
#endif
    //OS_Printf( "決定したpersonal_rnd = %d\n", personal_rnd );
    //OS_Printf( "get_PokeSeikaku = %d\n",get_PokeSeikaku(personal_rnd) );
    //OS_Printf( "レアじゃないか = %d\n", PokeRareGetPara(poke_id,personal_rnd) );
    pwd->personal_rnd=personal_rnd;
  }else{
    pwd->personal_rnd = poke_rnd;  //0でなければ引数の値を使用
    personal_rnd = poke_rnd;
  }


  //パワー乱数
  pwd->hp_rnd=pow_rnd;
  pwd->pow_rnd=pow_rnd;
  pwd->def_rnd=pow_rnd;
  pwd->agi_rnd=pow_rnd;
  pwd->spepow_rnd=pow_rnd;
  pwd->spedef_rnd=pow_rnd;

  //努力値
#if 0 //wb null
  exp=0;
  for(i=0;i<6;i++){
    if(prd_s.exp_bit&No2Bit(i)){
      exp++;
    }
  }
  if((PARA_EXP_TOTAL_MAX/exp)>255){
    exp=255;
  }else{
    exp=PARA_EXP_TOTAL_MAX/exp;
  }
  for(i = 0;i < 6;i++){
    if(prd_s.exp_bit&No2Bit(i)){
      pwd->exp[i]=exp;
    }
  }
#endif

  //技ポイント
  pwd->pp_count=0;

  //国コード
  pwd->country_code = 0;

  //特性
  i= POKETOOL_GetPersonalParam( pwd->mons_no,0,POKEPER_ID_speabi2);
  if(i){
    if(pwd->personal_rnd&1){
      pwd->tokusei=i;
    }else{
      pwd->tokusei=POKETOOL_GetPersonalParam(pwd->mons_no,0,POKEPER_ID_speabi1);
    }
  }else{
    pwd->tokusei=POKETOOL_GetPersonalParam( pwd->mons_no,0,POKEPER_ID_speabi1);
  }

  //なつき度
  pwd->natuki=friend;

  //ニックネーム
  {
   GFL_MSGDATA *msgdata = GFL_MSG_Create(
       GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_monsname_dat,
       heapID );
    GFL_MSG_GetStringRaw( msgdata,
        pwd->mons_no, pwd->nickname, MONS_NAME_SIZE+EOM_SIZE );
    GFL_MSG_Delete( msgdata );
  }
  return personal_rnd;
}

//--------------------------------------------------------------
/**
 * バトルサブウェイトレーナーデータ生成
 * （ロムデータをBSUBWAY_PARTNER_DATA構造体に展開）
 * @param tr_data  [in/out]生成するBSUBWAY_PARTNAER_DATA構造体
 * @param tr_no [in]生成元になるトレーナーID
 * @param cnt [in]トレーナーに持たせるポケモンの数
 * @param set_poke_no [in]ペアトレーナーのポケモン（NULL チェックなし）
 * @param set_item_no [in]ペアのポケモンの装備アイテム（NULL チェックなし）
 * @param poke [in/out] 抽選ポケモン二体のパラメータ格納先(NULLだとなし）
 * @param heapID  [in]ヒープID
 * @retval TRUE 抽選ループが50回超えた FALSE 抽選ループが50回以内で終わった
 * TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 * TOWER_AI_MULTI_ONLY 似た処理tower_tool.c FSRomBattleTowerTrainerDataMake
 * TOWER_AI_MULTI_ONLY サロンで、5人衆の手持ちポケモンを生成している
 */
//--------------------------------------------------------------
BOOL BSUBWAY_SCRWORK_MakeRomTrainerData(
    BSUBWAY_SCRWORK *bsw_scr, BSUBWAY_PARTNER_DATA *tr_data,
    u16 tr_no, int cnt,
    u16 *set_poke_no,u16 *set_item_no,
    BSUBWAY_PAREPOKE_PARAM* poke,HEAPID heapID)
{
  BOOL      ret = 0;
  BSUBWAY_TRAINER_ROM_DATA  *trd;

  //トレーナーデータセット
  trd = alloc_TrainerRomData(tr_data,tr_no,heapID);

  //ポケモンデータをセット
  ret = set_BSWayPokemonParam(
      bsw_scr,trd,tr_no,&tr_data->btpwd[0],cnt,
      set_poke_no,set_item_no,poke,heapID);
  GFL_HEAP_FreeMemory( trd );
  return ret;
}

//--------------------------------------------------------------
/**
 * バトルサブウェイトレーナーの持ちポケモンのパワー乱数を決定する
 * @param  tr_no  トレーナーナンバー
 * @return  パワー乱数
 * b_tower_fld.c→b_tower_evに移動
 */
//--------------------------------------------------------------
static u8 get_PowerRnd(u16 tr_no)
{
  u8  pow_rnd;

  if(tr_no<100){
    pow_rnd=(0x1f/8)*1;
  }
  else if(tr_no<120){
    pow_rnd=(0x1f/8)*2;
  }
  else if(tr_no<140){
    pow_rnd=(0x1f/8)*3;
  }
  else if(tr_no<160){
    pow_rnd=(0x1f/8)*4;
  }
  else if(tr_no<180){
    pow_rnd=(0x1f/8)*5;
  }
  else if(tr_no<200){
    pow_rnd=(0x1f/8)*6;
  }
  else if(tr_no<220){
    pow_rnd=(0x1f/8)*7;
  }
  else{
    pow_rnd=0x1f;
  }
  return pow_rnd;
}

//--------------------------------------------------------------
/**
 * バトルサブウェイ ペアトレーナーデータ再生成
 * （セーブされたAIマルチパートナーのデータを
 * BSUBWAY_PARTNER_DATA構造体に展開）
 * @param tr_data [in/out] 生成するBSUBWAY_PARTNAER_DATA構造体
 * @param tr_no [in]  生成元になるトレーナーID
 * @param fixitem [in] TUREなら固定アイテム、FALSEならromアイテムをセット
 * @param poke  [in] ポケモンデータ再生成に必要な構造体型データへのポインタ
 * @param heapID [in] ヒープID
 * @retval nothing
 * TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 * TOWER_AI_MULTI_ONLY AIマルチで休むの後、再開した時に呼ばれる
 */
//--------------------------------------------------------------
void BSUBWAY_SCRWORK_MakePartnerRomData(
    BSUBWAY_SCRWORK *wk, BSUBWAY_PARTNER_DATA *tr_data,
    u16 tr_no,BOOL itemfix,
    const BSUBWAY_PAREPOKE_PARAM* poke,HEAPID heapID)
{
  int  i;
  u8  pow_rnd = 0;
  BSUBWAY_TRAINER_ROM_DATA  *trd;

  //トレーナーデータセット
  trd = alloc_TrainerRomData(tr_data,tr_no,heapID);

  //ポケモンデータをセット
  pow_rnd=get_PowerRnd(tr_no);

  for(i = 0;i < BSUBWAY_STOCK_PAREPOKE_MAX;i++){
    make_PokemonParam(
      wk,&(tr_data->btpwd[i]),
      poke->poke_no[i],poke->poke_id,
      poke->poke_rnd[i],pow_rnd,i,itemfix,heapID);
  }
  GFL_HEAP_FreeMemory( trd );
}

//--------------------------------------------------------------
/**
 * バトルサブウェイのポケモンを決める
 * @param trd [in]トレーナーデータ
 * @param tr_no [in]トレーナーナンバー
 * @param pwd [in/out]BSUBWAY_POKEMON構造体
 * @param cnt [in]トレーナーに持たせるポケモンの数
 * @param set_poke_no [in]ペアトレーナーポケモン (NULLだとチェックなし
 * @param set_item_no [in]ペアトレーナーポケモンアイテム (NULL チェックなし
 * @param poke [in/out]抽選ポケモン二体のパラメータ格納先(NULL チェックなし
 * @param heapID [in]ヒープID
 * @retval  FALSE  抽選ループが50回以内で終わった
 * @retval  TRUE  抽選ループが50回以内で終わらなかった
 * TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 */
//--------------------------------------------------------------
static BOOL set_BSWayPokemonParam(
    BSUBWAY_SCRWORK *wk, BSUBWAY_TRAINER_ROM_DATA *trd,
    u16 tr_no, BSUBWAY_POKEMON *pwd, u8 cnt,
    u16 *set_poke_no, u16 *set_item_no,
    BSUBWAY_PAREPOKE_PARAM *poke, HEAPID heapID )
{
  int  i,j;
  u8  pow_rnd;
  u8  poke_index;
  u32  id;
  int  set_index;
  int  set_index_no[4];
  u32  set_rnd_no[4];
  int  set_count;
  int  loop_count;
  BOOL  ret = 0;
  BSUBWAY_POKEMON_ROM_DATA  prd_s;
  BSUBWAY_POKEMON_ROM_DATA  prd_d;

  //手持ちポケモンのMAXは4体まで
  GF_ASSERT(cnt<=4);

  set_count=0;
  loop_count=0;
  while(set_count!=cnt){
//    poke_index = gf_rand()%trd->use_poke_cnt;
    poke_index = get_Rand(wk)%trd->use_poke_cnt;
    set_index=trd->use_poke_table[poke_index];
    get_PokemonRomData(&prd_d,set_index);

    //モンスターナンバーのチェック（同一のポケモンは持たない）
    for(i=0;i<set_count;i++){
      get_PokemonRomData(&prd_s,set_index_no[i]);
      if(prd_s.mons_no==prd_d.mons_no){
        break;
      }
    }
    if(i!=set_count){
      continue;
    }

    //ペアを組んでいるトレーナーの持ちポケモンとのチェック
    if(set_poke_no!=NULL){
      //モンスターナンバーのチェック（同一のポケモンは持たない）
      for(i=0;i<cnt;i++){
        if(set_poke_no[i]==prd_d.mons_no){
          break;
        }
      }
      if(i!=cnt){
        continue;
      }
    }

    //50回まわして、決まらないようなら、同一アイテムチェックはしない
    if(loop_count<50){
      //装備アイテムのチェック（同一のアイテムは持たない）
      for(i=0;i<set_count;i++){
        get_PokemonRomData(&prd_s,set_index_no[i]);
        if((prd_s.item_no)&&(prd_s.item_no==prd_d.item_no)){
          break;
        }
      }
      if(i!=set_count){
        loop_count++;
        continue;
      }
      //ペアを組んでいるトレーナーの持ちポケモンの装備アイテムとのチェック
      if(set_item_no!=NULL){
        //装備アイテムのチェック（同一のアイテムは持たない）
        for(i=0;i<cnt;i++){
          if((set_item_no[i]==prd_d.item_no) && (set_item_no[i]!=0)){
            break;
          }
        }
        if(i!=cnt){
          loop_count++;
          continue;
        }
      }
    }

    set_index_no[set_count]=set_index;
    set_count++;
  }

  pow_rnd=get_PowerRnd(tr_no);
//id=(gf_rand()|(gf_rand()<<16));
  id=(get_Rand(wk)|(get_Rand(wk)<<16));

  if(loop_count >= 50){
    ret = TRUE;
  }
  for(i=0;i<set_count;i++){
    set_rnd_no[i] = make_PokemonParam(wk,&(pwd[i]),
      set_index_no[i],id,0,pow_rnd,i,ret,heapID);
  }
  if(poke == NULL){
    return ret;
  }
  //ポインタがNULLでなければ、抽選されたポケモンの必要なパラメータを返す
  poke->poke_id = id;
  for(i = 0;i< BSUBWAY_STOCK_PAREPOKE_MAX;i++){
    poke->poke_no[i] = set_index_no[i];
    poke->poke_rnd[i] = set_rnd_no[i];
  }
  return ret;
}

//--------------------------------------------------------------
/**
 *  バトルサブウェイトレーナーロムデータの読み出し
 * @param tr_no  [in]トレーナーナンバー
 * @param heapID  [in]メモリ確保するためのヒープID
 * TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 * TOWER_AI_MULTI_ONLY 似た処理 frontier_tool.c Frontier_TrainerRomDataGet
 */
//--------------------------------------------------------------
static void * get_TrainerRomData( u16 tr_no, HEAPID heapID )
{
  //AIマルチ限定なのでプラチナ！
  return GFL_ARC_UTIL_Load( ARCID_PL_BTD_TR, tr_no, 0, heapID );
}

//--------------------------------------------------------------
/**
 *  バトルサブウェイポケモンロムデータの読み出し
 * @param[in]  prd    読み出したポケモンデータの格納先
 * @param[in]  index  読み出すポケモンデータのインデックス
 * TOWER_AI_MULTI_ONLY フィールド上で呼ばれる処理
 * TOWER_AI_MULTI_ONLY 似た処理 frontier_tool.c Frontier_PokemonRomDataGet
 */
//--------------------------------------------------------------
static void get_PokemonRomData(
    BSUBWAY_POKEMON_ROM_DATA *prd,int index)
{
  //ここは通信はありえないのでプラチナ限定！(AIマルチ)
  GFL_ARC_LoadData( (void*)prd, ARCID_PL_BTD_PM, index );
}

//======================================================================
//  パーツ
//======================================================================
//--------------------------------------------------------------
/**
 * 文字列コピー
 * @param  to_str    コピー先バッファ
 * @param  from_str  文字列データ
 * @return  バッファにコピーしたデータ最後のEOM_へのポインタ
 *  データがない(文字列データ先頭がEOM_だった)場合、
 *  EOM_をバッファ先頭に書きこみ、そこへのポインタを返す
 */
//--------------------------------------------------------------
static STRCODE * PM_strcpy( STRCODE* to_str, const STRCODE* from_str )
{
  while( *from_str != GFL_STR_GetEOMCode() ){
    *to_str = *from_str;
    to_str++;
    from_str++;
  }
  *to_str = GFL_STR_GetEOMCode();
  return to_str;
}

/**
 *  @brief  参加できないポケモン名をタグ展開
 */
#if 0
WORDSET * BtlTower_SetNgPokeName(SAVEDATA* sv,u16 pokenum,u16 sex,u8 flag,u8* num)
{
  u8  i;
  u16  mons;
  STRBUF *buf,*nbuf;
  ZUKAN_WORK* zukan;
  WORDSET*  wset;
  MSGDATA_MANAGER* man;

  buf = STRBUF_Create( BUFLEN_POKEMON_NAME+BUFLEN_EOM_SIZE, HEAPID_FIELD );
  nbuf = STRBUF_Create( 2, HEAPID_FIELD );
  zukan = SaveData_GetZukanWork(sv);

  man = MSGMAN_Create( MSGMAN_TYPE_DIRECT, ARC_MSG, NARC_msg_monsname_dat, HEAPID_FIELD );
  wset = WORDSET_CreateEx(BSUBWAY_EXPOKE_NUM+1,BUFLEN_POKEMON_NAME+BUFLEN_EOM_SIZE,HEAPID_FIELD);

  WORDSET_RegisterNumber( wset,0,pokenum,1,NUMBER_DISPTYPE_LEFT,NUMBER_CODETYPE_DEFAULT);

  for(i = 0;i < BSUBWAY_EXPOKE_NUM;i++){
    mons = BattleTowerExPoke_MonsNoGet(i);
    if(ZukanWork_GetPokeSeeFlag(zukan,mons)){
      MSGMAN_GetString(man,mons,buf);
      //単語セット
      //バッファID
      //文字列
      //性別コード
      //単／複（TRUEで単数）
      //言語コード
      WORDSET_RegisterWord( wset,(*num)+1,buf,sex,flag,PM_LANG );
      (*num)++;
    }
  }

  MSGMAN_Delete(man);
  STRBUF_Delete( nbuf );
  STRBUF_Delete( buf );

  return wset;
}
#endif

//--------------------------------------------------------------
/**
 * 乱数取得
 * @param bsw_scr BSUBWAY_SCRWORK
 * @retval u16 乱数
 * @note 乱数固定の名残
 */
//--------------------------------------------------------------
static u16 get_Rand( BSUBWAY_SCRWORK *wk )
{
  return( GFUser_GetPublicRand(0xffffffff)/65535 );
}

//======================================================================
//  wb null
//======================================================================
#if 0
static void btwoer_TrainerDataMake(
    u8 play_mode,BSUBWAY_TRAINER* src,BSP_TRAINER_DATA * dest, HEAPID heapID )
{
  MI_CpuClear8(dest,sizeof(BSP_TRAINER_DATA));

  dest->tr_type = src->tr_type;  //トレーナータイプ
//dest->tr_gra = 0;//src->tr_type;  //トレーナータイプ
  dest->ai_bit = 0xFFFFFFFF;  //最強

  //トレーナー名
  dest->name = GFL_STR_CreateBuffer( PERSON_NAME_SIZE+EOM_SIZE,

//  GFL_STR_CopyBuffer(
  PM_strcpy(dest->name,src->name);

  //勝ち負けメッセージ
  MI_CpuCopy8(src->win_word,&dest->win_word,sizeof(PMS_DATA));
  MI_CpuCopy8(src->lose_word,&dest->lose_word,sizeof(PMS_DATA));

  switch(play_mode){
  case BSWAY_MODE_SINGLE:
  case BSWAY_MODE_WIFI:
    dest->fight_type = BTL_RULE_SINGLE;
    break;
  case BSWAY_MODE_DOUBLE:
  case BSWAY_MODE_MULTI:
  case BSWAY_MODE_COMM_MULTI:
  case BSWAY_MODE_WIFI_MULTI:
    dest->fight_type = BTL_RULE_DOUBLE;
    break;
  }
}
#endif

/**
 *  @brief  トレーナー対戦データ生成
 */
#if 0
static void btltower_SetTrainerData(BSUBWAY_SCRWORK* wk,BATTLE_PARAM* bp,
    BSUBWAY_PARTNER_DATA* tr,u8 tr_id,u8 tr_client,u8 poke_client )
{
  int i;
  POKEMON_PARAM *pp;

  //トレーナーデータをセット
  btower_TrainerDataMake(
      wk->play_mode,&(tr->bt_trd),&(bp->trainer_data[tr_client]));

  //トレーナーID入力
  bp->trainer_id[tr_client] = tr_id;//wk->tr_data[0].bt_trd.tr_type;

  //対戦相手のポケモンデータをセット
  pp = GFL_HEAP_AllocClearMemory( wk->heapID, POKETOOL_GetWorkSize() );

  PokeParty_Init(bp->poke_party[poke_client],wk->member_num);

  for(i = 0;i < wk->member_num;i++){
    make_PokePara(&(tr->btpwd[i]),pp);
    #if 0
    BattleParam_AddPokemon(bp,pp,poke_client);
    #else
    {
      int result;
      result = PokeParty_Add(bp->poke_party[client_no], pp);
    }
    #endif
  }
  GFL_HEAP_FreeMemory( pp );
}
#endif

