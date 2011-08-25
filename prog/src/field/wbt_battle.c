//============================================================================================
/**
 * @file  wbt_battle.c
 * @brief
 * @author  tamada GAMEFREAK inc.
 * @date  2011.08.20
 *
 * wbt_system.cから戦闘呼び出しに関わる部分を切り離した。
 * WBT_GIMMICKのオーバーレイに配置されるため、会場内のみでしか呼べないので注意
 */
//============================================================================================

#include <gflib.h>

#include "wbt_system.h"

#include "system/main.h"

#include "wbt_system_local.h"
#include "wbt_tool.h"

//#include "poke_tool/poketype_def.h"

#include "tr_tool/trtype_def.h" //TRTYPE_HERO/TRTYPE_HEROINE

#include "savedata/bsubway_savedata.h"  //bsubway_battle_data.hの内部定義のため
#include "battle/bsubway_battle_data.h" //BSUBWAY_PARTNER_DATA
#include "fld_btl_inst_tool.h"
#include "message.naix"    //NARC_message_wbt_trname_dat
#include "tr_tool/ex_trainer_def.h" //TRID_OFFSET_WBT
#include "savedata/etc_save.h"    //ETC_SAVE_WORK

#include "savedata/save_tbl.h"
//#include "savedata/save_control.h"
//#include "savedata/wbt_ex_save.h"
#include "savedata/wbt_download.h"

#include "sound/swan_sound_data.sadl"


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  //（一行１８文字＋改行コード）*２（海外は倍)*8(8行分）
  WBT_TRAINER_MSG_LEN = ((18+1)*2*8),
};

enum {
  DUMMY_LEVEL_VALUE = 50, //後でレベル調整するので何でもよい
};

enum {
  ROM_POKE_MAX = 10,
};

//============================================================================================
//
//
//  ダウンロード手持ちデータの設定処理
//
//
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL checkDLPokemon( const WBTDL_POKEMON * dl_poke )
{
  if ( dl_poke->monsno == 0 || dl_poke->level == 0 ) return FALSE;
  return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   DLデータからポケモンを生成
 * @param[out]  pp
 * @param[in]   dl_poke
 * @param[in]   tempHeapID
 */
//--------------------------------------------------------------
static void setDLPokemon( POKEMON_PARAM * pp, const WBTDL_POKEMON * dl_poke, HEAPID tempHeapID )
{
  int i;
  u64 personal_rnd;

  PP_Clear( pp );

  personal_rnd = ( dl_poke->rare_flag == TRUE ? PTL_SETUP_RND_RARE : PTL_SETUP_RND_AUTO );
  PP_SetupEx( pp, dl_poke->monsno, dl_poke->level, PTL_SETUP_ID_AUTO,
      (dl_poke->pow_rnd & 0x3FFFFFFF), personal_rnd );

  //form_no
  PP_Put( pp, ID_PARA_form_no, dl_poke->form_no );

  //装備アイテム設定
  PP_Put( pp, ID_PARA_item, dl_poke->item_no );
  //技設定
  for ( i = 0; i < PTL_WAZA_MAX; i ++ )
  {
    u8 point;
    u8 buf8;
    PP_Put( pp, ID_PARA_waza1 + i, dl_poke->waza_id[i] );
    buf8 = (dl_poke->pp_count >> (i*2))&0x03;
    PP_Put( pp, ID_PARA_pp_count1 + i, buf8);
    point = PP_Get( pp, ID_PARA_pp_max1 + i, NULL );
    PP_Put( pp, ID_PARA_pp1 + i, point );
  }
  //経験値設定
  PP_Put( pp, ID_PARA_hp_exp, dl_poke->hp_exp );
  PP_Put( pp, ID_PARA_pow_exp, dl_poke->pow_exp );
  PP_Put( pp, ID_PARA_def_exp, dl_poke->def_exp );
  PP_Put( pp, ID_PARA_agi_exp, dl_poke->agi_exp );
  PP_Put( pp, ID_PARA_spepow_exp, dl_poke->spepow_exp );
  PP_Put( pp, ID_PARA_spedef_exp, dl_poke->spedef_exp );

  //特性設定
  PP_Put( pp, ID_PARA_speabino, dl_poke->tokusei );
  //なつきど設定
  PP_Put( pp, ID_PARA_friend, dl_poke->friend_rate );

  //性格セット
  PP_SetSeikaku( pp, dl_poke->seikaku );

  { //ニックネームセット
    STRBUF * nickname;
    nickname = GFL_STR_CreateBuffer( MONS_NAME_SIZE + EOM_SIZE, tempHeapID );
    GFL_STR_SetStringCodeOrderLength( nickname, dl_poke->nickname, MONS_NAME_SIZE + EOM_SIZE );
    PP_Put( pp, ID_PARA_nickname, (u32)nickname );
    GFL_STR_DeleteBuffer( nickname );
  }
  //捕まえたボール
  PP_Put( pp, ID_PARA_get_ball, dl_poke->ball_id );

  PP_Renew( pp );
}

//--------------------------------------------------------------
/**
 * @brief DLデータから手持ちポケモンパーティを生成
 * @param[out]  party
 * @param[in]   wbtdl_data
 * @param[in]   dl_tr_id
 * @param[in]   num
 * @param[in]   tempHeapID
 * 
 * @todo  データ異常の場合のエラー処理
 */
//--------------------------------------------------------------
static void setDLPokeParty( POKEPARTY * party,
    const WBTDL_DATA * wbtdl_data, u32 dl_tr_id, u32 num, HEAPID tempHeapID )
{
  int i;
  POKEMON_PARAM * entry_pp;

  PokeParty_Init( party, TEMOTI_POKEMAX );

  entry_pp = GFL_HEAP_AllocMemoryLo( tempHeapID, POKETOOL_GetWorkSize() );

  for ( i = 0; i < num; i ++ )
  {
    const WBTDL_POKEMON * dl_poke = WBTDL_DATA_GetPokemon( wbtdl_data, dl_tr_id, i );
    if ( checkDLPokemon( dl_poke ) )
    {
      setDLPokemon( entry_pp, &dl_poke[i], tempHeapID );
      PokeParty_Add( party, entry_pp );
    }
    else
    {
      //万が一、データ異常の場合の救済処理：ロムから引っ張る？
    }
  }
  GFL_HEAP_FreeMemory( entry_pp );
}

//============================================================================================
//
//  ROM手持ちデータの設定処理
//
//============================================================================================
#include "field/eventwork.h"  //EVENTWORK_
#include "../../../resource/fldmapdata/flagwork/work_define.h"  //WK_SYS_FIRST_POKETYPE
#include "../../../resource/fldmapdata/script/user_define.h"    //FIRST_POKETYPE_

//--------------------------------------------------------------
/**
 * @brief 最初の3体選択結果でトレーナーIDを振り分ける
 */
//--------------------------------------------------------------
static u32 getWBTRivalTrainerID( GAMESYS_WORK * gsys )
{
  EVENTWORK * evwork = GAMEDATA_GetEventWork( GAMESYSTEM_GetGameData( gsys ) );
  u16 * first_choice = EVENTWORK_GetEventWorkAdrs( evwork, WK_SYS_FIRST_POKETYPE );
  switch ( *first_choice )
  {
  default:
    GF_ASSERT( 0 );
    /* FALL THROUGH */
  case FIRST_POKETYPE_KUSA:
    return TRID_SWRIVAL_07;
  case FIRST_POKETYPE_HONOO:
    return TRID_SWRIVAL_08;
  case FIRST_POKETYPE_MIZU:
    return TRID_SWRIVAL_09;
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const u8 pri2pow_table[5] = {
  0x10, 0x12, 0x14, 0x1a, 0x1f
};

//-----------------------------------------------------------------------------
/**
 * @param btpwd
 * @param wbt_tr
 * @param cnt
 * @param inHeapID
 * @todo  バトルサブウェイは４体しかポケモンを選択できない。
 */
//-----------------------------------------------------------------------------
static void makeRomTrainerData(
    BSUBWAY_POKEMON * btpwd,
    WBTCUP wbtcup, const WBTTRAINER * wbt_tr,
    u8 cnt, HEAPID inHeapID)
{
  BSUBWAY_PARTNER_DATA * dummy_tr_data;
  BSUBWAY_TRAINER_ROM_DATA  *trd;
  u8 pow_rnd;
  u8 tr_pri   = WBTTRAINER_GetPriority(wbt_tr);
  u8 tr_id    = WBTTRAINER_GetID(wbt_tr);
  u32 base_arc_id  = WBTTOOL_GetRomArcID( wbtcup );
  u32 id;
  GFL_STD_RandContext * rand_context = NULL;
  WBT_NGPMINFO * ng_info;

  ng_info = NGINFO_Create( inHeapID );
  
  // パワー乱数を決定する
  pow_rnd = pri2pow_table[ tr_pri - 1 ];
  //id=(GetRand( rand_context )|(GetRand( rand_context )<<16));

  //実際には使用しないがFBI_TOOL_AllocTrainerRomDataの呼び出しに必要
  dummy_tr_data = GFL_HEAP_AllocMemoryLo( inHeapID, sizeof(BSUBWAY_PARTNER_DATA) );

  if (WBTTRAINER_HasAce(wbt_tr) )
  { //切り札ポケモンセット処理
    u16 index_no;
    trd = FBI_TOOL_AllocTrainerRomData(dummy_tr_data,
        base_arc_id + WBTBTD_ARCOFS_TR_ACE, tr_id, NARC_message_wbt_trname_dat, inHeapID);
    //切り札ポケモンは１体だけなので０番決めうちで取得
    index_no = trd->use_poke_table[0];
    FBI_TOOL_MakePokemonParam( btpwd,
        base_arc_id + WBTBTD_ARCOFS_PM_ACE, index_no, tr_id, 0, pow_rnd, 0, FALSE, inHeapID );

    WBT_NGINFO_AddNGMons( ng_info, btpwd->mons_no );
    WBT_NGINFO_AddNGItem( ng_info, btpwd->item_no );
    btpwd ++;
    cnt --;
    GFL_HEAP_FreeMemory( trd );
  }

  { //ポケモンセット処理
    int i;
    u16 index_array[ TEMOTI_POKEMAX ];

    //トレーナーデータセット
    trd = FBI_TOOL_AllocTrainerRomData(dummy_tr_data,
        base_arc_id + WBTBTD_ARCOFS_TR_NORMAL, tr_id, NARC_message_wbt_trname_dat, inHeapID);

    TAMADA_Printf(" WBT TrData:arc_id=%d id=%d use_poke_cnt = %d\n", base_arc_id, tr_id, trd->use_poke_cnt );
    //ポケモンデータを選択
    GF_ASSERT( trd->use_poke_cnt < ROM_POKE_MAX );
    WBTSYS_ChooseBSubwayData( index_array, ng_info,
        inHeapID, trd->use_poke_cnt, trd->use_poke_table,
        base_arc_id + WBTBTD_ARCOFS_PM_NORMAL, cnt, NULL );

    for(i = 0; i < cnt; i++)
    {
      u32 index_no = index_array[ i ];
      FBI_TOOL_MakePokemonParam( &(btpwd[i]),
        base_arc_id + WBTBTD_ARCOFS_PM_NORMAL, index_no, tr_id, 0, pow_rnd, i, FALSE, inHeapID);
    }

    GFL_HEAP_FreeMemory( trd );
  }
  GFL_HEAP_FreeMemory( dummy_tr_data );

  WBT_NGINFO_Delete( ng_info );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void setRomPokeParty( POKEPARTY * party,
    const WBTTRAINER * wbt_tr, WBTCUP wbtcup, u8 num, HEAPID tempHeapID )
{
  BSUBWAY_POKEMON * btpwd;

  btpwd = GFL_HEAP_AllocMemoryLo( tempHeapID, sizeof(BSUBWAY_POKEMON) * TEMOTI_POKEMAX );
  makeRomTrainerData( btpwd, wbtcup, wbt_tr, num, tempHeapID );
  FBI_TOOL_WBT_SetPokemon( party, btpwd, DUMMY_LEVEL_VALUE, num, tempHeapID );
 
  GFL_HEAP_FreeMemory( btpwd );
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 対戦相手の手持ちポケモン設定
 * @param wbtsys
 * @param gsys      GAMESYS_WORKへのポインタ（ライバル手持ち判定に使用）
 * @param wbt_tr    対戦相手のWBTTRAINER
 */
//------------------------------------------------------------------
void WBTSYS_SetupEnemyParty( WBTSYS * wbtsys, GAMESYS_WORK * gsys, const WBTTRAINER * wbt_tr )
{

  HEAPID tempHeapID = GFL_HEAP_LOWID( wbtsys->heapID );
  int num = WBTSYS_GetPartyMax( wbtsys );
#ifdef PM_DEBUG
  u32 tempHeapSize = GFL_HEAP_GetHeapFreeSize( tempHeapID );
#endif

  switch ( wbt_tr->wbt_tr_type )
  {
  case WBT_TRTYPE_ROM:
    if ( wbt_tr->btl_tr_type == TRTYPE_SWRIVAL )
    {//ライバルの場合、ポケモンデータを上書き
      u32 normal_tr_id = getWBTRivalTrainerID( gsys );
      TT_EncountTrainerPokeDataMake( normal_tr_id, wbtsys->npc_party, tempHeapID );
    }
    else
    {
      setRomPokeParty( wbtsys->npc_party, wbt_tr, WBTSYS_GetWBTCup(wbtsys), num, tempHeapID );
    }
    break;

  case WBT_TRTYPE_DL:
    setDLPokeParty( wbtsys->npc_party, &wbtsys->wbtdl_data, wbt_tr->tr_id, num, tempHeapID );
    break;

  default:
  case WBT_TRTYPE_PLAYER:
    GF_ASSERT( 0 );
    break;
  }
#ifdef PM_DEBUG
  GF_ASSERT( tempHeapSize == GFL_HEAP_GetHeapFreeSize( tempHeapID ) );
#endif

}

//------------------------------------------------------------------
/**
 * @brief BATTLE_SETUP_PARAM用トレーナーデータの設定
 * @param[out]  tr_data
 * @param[in]   wbt_tr
 * @param[in]   aibit
 */
//------------------------------------------------------------------
static void setBSPTrainerData( BSP_TRAINER_DATA * tr_data, const WBTTRAINER * wbt_tr, u32 aibit )
{
  tr_data->tr_id    = wbt_tr->tr_id + TRID_OFFSET_WBT;
  tr_data->tr_type  = wbt_tr->btl_tr_type;
  tr_data->ai_bit   = aibit;

  WBTTRAINER_CopyName( wbt_tr, tr_data->name );
    
  GFL_STD_MemClear16( tr_data->use_item, sizeof(u16)*4 );
  PMSDAT_Clear( &tr_data->win_word );
  PMSDAT_Clear( &tr_data->lose_word );

}

//------------------------------------------------------------------
/**
 * @brief WBT用BATTLE_SETUP_PARAM生成
 */
//------------------------------------------------------------------
BATTLE_SETUP_PARAM * WBTSYS_CreateBattleSetupParam( WBTSYS * wbtsys, GAMESYS_WORK * gsys )
{
  BATTLE_SETUP_PARAM * bp;

  WBTRULE wbt_rule = WBTSYS_GetRule( wbtsys );
  BtlRule rule = WBTTOOL_WBTRuleToBtlRule( wbt_rule );
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );

  bp = FBI_TOOL_WBT_CreateBattleParam( gsys, wbtsys->my_party, rule );

  //自分手持ち：ホドモエイベント以外は、レギュレーションにあわせてレベル調整
  if ( WBTSYS_GetWBTCup(wbtsys) != WBTCUP_HODOMOE_EVENT )
  {
    PokeRegulation_ModifyLevelPokeParty( WBTSYS_GetRegulation(wbtsys), bp->party[BTL_CLIENT_PLAYER] );
  }

  //対戦トレーナーの情報をセットする
  setBSPTrainerData( bp->tr_data[BTL_CLIENT_ENEMY1], wbt_tr, 0x00000087 );
  PokeParty_Copy( wbtsys->npc_party, bp->party[BTL_CLIENT_ENEMY1] );

  //相手手持ち：レギュレーションにあわせてレベル調整
  PokeRegulation_ModifyLevelPokeParty( WBTSYS_GetRegulation(wbtsys), bp->party[BTL_CLIENT_ENEMY1] );

  //肩書STRBUFにポインタをつなぐ
  BATTLE_PARAM_SetTrainerTypeName( bp, BTL_CLIENT_ENEMY1, wbt_tr->title );

  //トレーナーがバトル内で話す台詞を事前生成、セットする
  wbtsys->win_msg   = GFL_STR_CreateBuffer( WBT_TRAINER_MSG_LEN, wbtsys->heapID );
  wbtsys->lose_msg  = GFL_STR_CreateBuffer( WBT_TRAINER_MSG_LEN, wbtsys->heapID );
  WBTSYS_GetTrainerMsg( wbtsys, WBT_TRMSG_FIGHT_LOSE, wbtsys->win_msg );
  WBTSYS_GetTrainerMsg( wbtsys, WBT_TRMSG_FIGHT_WIN, wbtsys->lose_msg );
  BATTLE_PARAM_SetWBTMsg( bp, wbtsys->win_msg, wbtsys->lose_msg );

  //対戦BGMのセット
  WBTSYS_GetOpponentBGM( wbtsys, &bp->musicDefault, &bp->musicWin );
  // 戦闘背景書き換え
  if( WBTSYS_GetWBTCup(wbtsys) == WBTCUP_DOWNLOAD
      &&  wbtsys->wbtdl_data.wbtdl_rule.bgm_flag == TRUE )
  {
    bp->fieldSituation.bgType = BATTLE_BG_TYPE_WCS;
    bp->fieldSituation.bgAttr = BATTLE_BG_ATTR_E_INDOOR;
  }
  else
  {
    //仮：定義の追加が必要
    //bp->fieldSituation.bgType = ;
    //bp->fieldSituation.bgAttr = ;
  }
  return bp;
}

//------------------------------------------------------------------
/**
 * @brief WBT用BATTLE_SETUP_PARAMを削除
 */
//------------------------------------------------------------------
void WBTSYS_DeleteBattleSetupParam( WBTSYS * wbtsys, BATTLE_SETUP_PARAM * bsp )
{
  if (wbtsys->win_msg)
  {
    GFL_STR_DeleteBuffer( wbtsys->win_msg );
  }
  if (wbtsys->lose_msg)
  {
    GFL_STR_DeleteBuffer( wbtsys->lose_msg );
  }
  BATTLE_PARAM_Delete( bsp );
}

//------------------------------------------------------------------
/**
 * @brief 戦闘結果から生き残ったポケモンの数を取得する
 * @param[in] party
 * @param[in] party_state
 * @return  u32
 */
//------------------------------------------------------------------
static u32 countAliveStatus( const POKEPARTY * party, const BtlPokeStatCode * party_state )
{
  int i;
  u32 count = 0;
  for ( i = 0; i < TEMOTI_POKEMAX; i ++ )
  {
    //OS_Printf("%d:%d\n", i, party_state[i] );
    if ( party_state[i] == BTL_POKESTATE_DEAD )
    {
      count ++;
    }
  }
  return PokeParty_GetPokeCount(party) - count;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static u8 getBattleRecordIndex( WBTSTAGE stage )
{
  int pos = 0;
  switch (stage)
  {
  default:
    GF_ASSERT( 0 );
    break;
  case WBTSTAGE_1ST:    pos = 2; break;
  case WBTSTAGE_2ND:    pos = 1; break;
  case WBTSTAGE_FINAL:  pos = 0; break;
  }
  return  pos;
}

//------------------------------------------------------------------
/**
 * @brief
 * @param[in] wbtsys
 * @param[in] stage
 * @retval    u8  その対戦での手持ちの残りの数
 */
//------------------------------------------------------------------
u8 WBTSYS_GetSurvivalCount( const WBTSYS * wbtsys, WBTSTAGE stage )
{
  u8 pos = getBattleRecordIndex( stage );
  return wbtsys->survival_count[ pos ];
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void WBTSYS_SetSurvivalCount( WBTSYS * wbtsys, WBTSTAGE stage, u8 count )
{
  u8 pos = getBattleRecordIndex( stage );
  wbtsys->survival_count[ pos ] = count;
}

//------------------------------------------------------------------
/**
 * @brief WBT戦闘結果の反映
 * @param[in,out] wbtsys
 * @param[in,out] gamedata
 */
//------------------------------------------------------------------
void WBTSYS_ReflectBattleResult( WBTSYS * wbtsys, GAMEDATA * gamedata, BATTLE_SETUP_PARAM * bsp )
{
  u16 * ret_wk = WBTSYS_GetScriptAnswerWork( wbtsys );
  u32 result = GAMEDATA_GetLastBattleResult( gamedata ); 
  u32 my_count, enemy_count;

  enemy_count = countAliveStatus( bsp->party[BTL_CLIENT_ENEMY1], bsp->party_state[BTL_CLIENT_ENEMY1] );
  my_count = countAliveStatus( bsp->party[BTL_CLIENT_PLAYER], bsp->party_state[BTL_CLIENT_PLAYER] );

  TAMADA_Printf("my_count = %d, enemy_count = %d\n", my_count, enemy_count );

  //生き残った手持ちの数をセットする
  WBTSYS_SetSurvivalCount( wbtsys, wbtsys->stage, my_count );

  switch ( result )
  {
  default:
    GF_ASSERT_MSG( 0, "想定外のBtlResult(%d)\n", result );
  case BTL_RESULT_RUN:  //自分が逃げた
    result = BTL_RESULT_LOSE;
    break;
  case BTL_RESULT_LOSE:
  case BTL_RESULT_WIN:
  case BTL_RESULT_DRAW:
    break;
  }

  {
    const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
    WBTSAVE * wbtsave = GAMEDATA_GetWBTSave( gamedata );
    if ( wbt_tr->tr_id < WBTSAVE_FLAG_MAX )
    { //戦ったかどうかのフラグを反映
      WBTSAVE_SetFlag( wbtsave, wbt_tr->tr_id );
    }
    if ( wbtsys->wbtcup != WBTCUP_DOWNLOAD )
    { //対戦相手のトレーナーIDを保存する
      u8 pos = getBattleRecordIndex( wbtsys->stage );
      WBTSAVE_SetTrainerID( wbtsave, pos, wbt_tr->tr_id );
    }
    if ( result == BTL_RESULT_LOSE )
    { //敗北した場合は無効なトレーナーIDをセットしておく
      u8 pos;
      for ( pos = 0; pos < WBT_BATTLE_NUM; pos ++ )
      {
        WBTSAVE_SetTrainerID( wbtsave, pos, WBTROM_TRID_INVALID );
      }
    }
  }
  *ret_wk = result;
}



//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 対戦相手のトレーナーデータを取得
 */
//------------------------------------------------------------------
const WBTTRAINER * WBTSYS_GetOpponentTrainer( const WBTSYS * wbtsys )
{
  u32 pos = WBTSYS_GetOpponentPos( wbtsys );
  const WBTTRAINER * wbt_tr = WBTSYS_GetTrainer( wbtsys, pos );
  return wbt_tr;
}

//------------------------------------------------------------------
/**
 * @brief 対戦相手の性別を取得する
 */
//------------------------------------------------------------------
u8 WBTSYS_GetOpponentSex( const WBTSYS * wbtsys )
{
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  return wbt_tr->sex;
}

//------------------------------------------------------------------
/**
 * @brief 対戦相手の動作モデル表示コードを取得する
 */
//------------------------------------------------------------------
u16 WBTSYS_GetOpponentMMdlObjCode( const WBTSYS * wbtsys )
{
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  return wbt_tr->mmdl_id;
}
//------------------------------------------------------------------
/**
 * @brief 対戦相手の名前を取得する
 */
//------------------------------------------------------------------
void WBTSYS_GetOpponentName( const WBTSYS * wbtsys, STRBUF * strbuf )
{
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  WBTTRAINER_CopyName( wbt_tr, strbuf );
}

//------------------------------------------------------------------
/**
 * @brief 対戦相手のトレーナータイプを取得する
 * @param wbtsys
 * @return  u16 tr_type(include/tr_tool/trtype_def.h参照）
 */
//------------------------------------------------------------------
u16 WBTSYS_GetOpponentTrainerType( const WBTSYS * wbtsys )
{
  const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
  return wbt_tr->btl_tr_type;
}

//------------------------------------------------------------------
/**
 * @brief 戦闘BGMの取得
 * @param[in]   wbtsys
 * @param[out]  btl_bgm
 * @param[out]  win_bgm
 */
//------------------------------------------------------------------
void WBTSYS_GetOpponentBGM( const WBTSYS * wbtsys, u16 *btl_bgm, u16 *win_bgm )
{
  switch ( wbtsys->wbtcup )
  {
  case WBTCUP_DOWNLOAD:
    if ( wbtsys->wbtdl_data.wbtdl_rule.bgm_flag )
    {
      *btl_bgm = SEQ_BGM_VS_WCS;
      *win_bgm = SEQ_BGM_WIN5;
    }
    else
    {
      *btl_bgm = SEQ_BGM_VS_WBT;
      *win_bgm = SEQ_BGM_WIN_WBT;
    }
    break;
  default:
    {
      const WBTTRAINER * wbt_tr = WBTSYS_GetOpponentTrainer( wbtsys );
      WBT_TRDATA tr_rom_data;
      GF_ASSERT( wbt_tr->wbt_tr_type == WBT_TRTYPE_ROM );
      WBTTOOL_GetRomTrainerData( wbtsys->rom_trdata, wbt_tr->tr_id, &tr_rom_data );
      *btl_bgm = tr_rom_data.btl_bgm_id;
      *win_bgm = tr_rom_data.win_bgm_id;
      //チャンピオン戦以外で決勝の場合はWBT専用曲にする
      if ( wbtsys->stage == WBTSTAGE_FINAL && wbtsys->wbtcup != WBTCUP_CHAMPION )
      {
        *btl_bgm = SEQ_BGM_VS_WBT;
        *win_bgm = SEQ_BGM_WIN_WBT;
      }
    }
    break;
  }
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief
 * @param[in]   wbtsys
 * @param[in]   type
 * @param[out]  strbuf
 */
//------------------------------------------------------------------
void WBTSYS_GetTrainerMsg( const WBTSYS * wbtsys, u32 type, STRBUF * strbuf )
{
  HEAPID tempHeapID = wbtsys->heapID;
  const WBTTRAINER * opponent = WBTSYS_GetOpponentTrainer( wbtsys );

  if ( type >= WBT_TRMSG_TYPE_NUM )
  {
    GF_ASSERT( 0 );
    return;
  }

  if ( WBTTRAINER_GetWBTTrainerType(opponent) != WBT_TRTYPE_DL )
  {
    GFL_MSGDATA * msgData;
    u16 msg_id;
    msg_id = WBTTRAINER_GetID( opponent ) * WBT_TRMSG_TYPE_NUM;
    msg_id += type;
    msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
        NARC_message_wbt_trmsg_dat, tempHeapID );
    GFL_MSG_GetString( msgData, msg_id, strbuf );
    GFL_MSG_Delete( msgData );
    return;
  }
  else
  {
    u16 dl_tr_id = WBTTRAINER_GetID( opponent );
    u16 preset_id = 1;
    const STRCODE * freeword = NULL;
    const WBTDL_CHARACTER * dl_char = WBTDL_DATA_GetCharacter( &wbtsys->wbtdl_data, dl_tr_id );
    switch ( type )
    {
    case WBT_TRMSG_1ST_FIGHT_START:
    case WBT_TRMSG_2ND_FIGHT_START:
    case WBT_TRMSG_3RD_FIGHT_START:
      preset_id = dl_char->msg_before_id;
      freeword  = dl_char->msg_before;
      break;
    case WBT_TRMSG_FIGHT_LOSE:
      preset_id = dl_char->msg_win_id;
      freeword  = NULL;
      break;
    case WBT_TRMSG_FIGHT_WIN:
      preset_id = dl_char->msg_lose_id;
      freeword  = NULL;
      break;
    case WBT_TRMSG_PLAYER_WIN:
      preset_id = dl_char->msg_after_win_id;
      freeword  = dl_char->msg_win;
      break;
    case WBT_TRMSG_PLAYER_LOSE:
      preset_id = dl_char->msg_after_lose_id;
      freeword  = dl_char->msg_lose;
      break;
    case WBT_TRMSG_ANNOUNCE:
      preset_id = dl_char->msg_announce_id;
      freeword  = dl_char->msg_announce;
      break;
    }

    if ( preset_id == 0 && freeword == NULL )
    {
      GF_ASSERT( 0 );
      preset_id = 1;  //エラー対処
    }
    if ( preset_id != 0 )
    {
      GFL_MSGDATA * msgData;
      u16 msg_id;
      msg_id = ( preset_id - 1 ) * WBT_TRMSG_TYPE_NUM + type;
      msgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
          NARC_message_wbt_trmsg_dl_dat, tempHeapID );
      GFL_MSG_GetString( msgData, msg_id, strbuf );
      GFL_MSG_Delete( msgData );
    }
    else
    {
      GF_ASSERT( freeword != NULL );
      GFL_STR_SetStringCodeOrderLength( strbuf, freeword, WBTDL_MSG_LEN );
    }
  }
}

