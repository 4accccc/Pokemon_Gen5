//======================================================================
/**
 * @file	scrcmd_pokemon.c
 * @brief	スクリプトコマンド：ポケモン関連
 * @date  2009.09.15
 * @author	tamada GAMEFREAK inc.
 *
 */
//======================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_pokemon.h"
#include "poke_tool/tokusyu_def.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/pokeparty.h"

#include "poke_tool/status_rcv.h" //PokeParty_RecoverAll

#include "savedata/box_savedata.h"

#include "event_name_input.h" // for EVENT_PartyPokeNameInput
#include "event_egg_birth.h"  // for EVENT_EggBirth

#include "savedata/sodateya_work.h"
#include "sodateya.h" // for POKEMON_EGG_Birth

#include "event_poke_status.h" // for EvCmdPartyPokeSelect

#include "fieldmap.h" 

#include "fld_trade.h"  // for EVENT_FieldPokeTrade

#include "app/waza_oshie.h"  // for WAZAOSHIE_xxxx

#include "msg/msg_place_name.h"  // for MAPNAME_xxxx

#include "waza_tool/wazano_def.h" // for WAZANO_xxxx


//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================
static GMEVENT_RESULT EVENT_FUNC_PokeSelect(GMEVENT * event, int * seq, void * work);

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @brief ツール関数：指定位置の手持ちポケモンを取得する
 * @param work
 * @param pos
 * @param poke_para
 * @return  BOOL  取得に成功したとき=TRUE、失敗した時=FALSE（位置が大きすぎるなど）
 */
//--------------------------------------------------------------
BOOL SCRCMD_GetTemotiPP( SCRCMD_WORK * work, u16 pos, POKEMON_PARAM ** poke_para )
{
  GAMEDATA *gamedata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY * party = GAMEDATA_GetMyPokemon( gamedata );
  u32 max = PokeParty_GetPokeCount( party );
  BOOL result;

  if (pos < max)
  {
    result = TRUE;
  }
  else
  {
    GF_ASSERT_MSG(0, "Temoti Pos %d over max(%d)!!\n", pos, max);
    result = FALSE;
    pos = 0;
  }
  *poke_para = PokeParty_GetMemberPointer( party, pos );
  return result;
}

//--------------------------------------------------------------
/**
 * @brief ツール関数：指定したわざを持っているか？のチェック
 * @param pp
 * @param wazano
 * @return  BOOL
 */
//--------------------------------------------------------------
BOOL SCRCMD_CheckTemotiWaza( const POKEMON_PARAM * pp, u16 wazano )
{
  //たまごチェック
  if( PP_Get(pp,ID_PARA_tamago_flag,NULL) != 0 ){
    return FALSE;
  }
  //技リストからチェック
  if( PP_Get(pp,ID_PARA_waza1,NULL) == wazano ||
      PP_Get(pp,ID_PARA_waza2,NULL) == wazano ||
      PP_Get(pp,ID_PARA_waza3,NULL) == wazano ||
      PP_Get(pp,ID_PARA_waza4,NULL) == wazano ){
    return TRUE;
  }
  return FALSE;
}

//======================================================================
//  ポケモン関連
//======================================================================
//--------------------------------------------------------------
/**
 * 手持ちにポケルス状態のポケモンがいるか？のチェック
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCheckTemotiPokerus( VMHANDLE * core, void *wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  *ret_wk = FALSE;

  {
    GAMEDATA *gamedata = SCRCMD_WORK_GetGameData( wk );
    POKEPARTY * party = GAMEDATA_GetMyPokemon( gamedata );
    u32 max = PokeParty_GetPokeCount( party );
    int idx;

    for (idx = 0; idx < max; idx ++)
    {
      u32 result;
      POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, idx );
      result = PP_Get( pp, ID_PARA_pokerus, NULL );
      if (result != 0)
      {
        *ret_wk = TRUE;
        break;
      }
    }
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 「めざめるパワー」のタイプを取得する
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 *
 * @todo  わざマシン１０＝＝１０を指定するなのか？10-1なのか？確認
 *
 * めざめるパワー＝わざマシン１０
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetMezameruPowerType( VMHANDLE * core, void *wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );  //結果格納ワーク
  u16 pos = SCRCMD_GetVMWorkValue( core, wk );  //ポケモンの位置
  POKEMON_PARAM * pp;
  if ( SCRCMD_GetTemotiPP( wk, pos, &pp ) == FALSE )
  { //エラー対処
    *ret_wk = 0xffff;
  }
  else if (PP_CheckWazaMachine( pp, 10 - 1 ) == FALSE)
  {
    *ret_wk = 0xffff;
  }
  else
  {
    *ret_wk = POKETOOL_GetMezaPa_Type( pp );
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief てもちポケモンのタイプ取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPartyPokeType( VMHANDLE * core, void *wk )
{
  u16 * ret_wk1 = SCRCMD_GetVMWork( core, wk );  //結果格納ワーク
  u16 * ret_wk2 = SCRCMD_GetVMWork( core, wk );  //結果格納ワーク
  u16 pos = SCRCMD_GetVMWorkValue( core, wk );  //ポケモンの位置
  POKEMON_PARAM * pp;
  if ( SCRCMD_GetTemotiPP( wk, pos, &pp ) == TRUE )
  {
    *ret_wk1 = PP_Get( pp, ID_PARA_type1, NULL );
    *ret_wk2 = PP_Get( pp, ID_PARA_type2, NULL );
  }
  else
  {
    *ret_wk1 =  POKETYPE_NORMAL;
    *ret_wk2 =  POKETYPE_NORMAL;
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 手持ちポケモン回復
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPokemonRecover( VMHANDLE * core, void *wk )
{
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( wk );
  POKEPARTY * party = GAMEDATA_GetMyPokemon( gamedata );

  STATUS_RCV_PokeParty_RecoverAll( party );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 手持ちポケモンの努力値チェック
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetParamEXP( VMHANDLE *core, void *wk )
{
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );  //結果格納ワーク
  u16 pos = SCRCMD_GetVMWorkValue( core, wk );  //ポケモンの位置
  POKEMON_PARAM * pp;
  u16 exp = 0;

  if ( SCRCMD_GetTemotiPP( wk, pos, &pp ) == TRUE )
  {
    exp += PP_Get( pp, ID_PARA_hp_exp, NULL );
    exp += PP_Get( pp, ID_PARA_pow_exp, NULL );
    exp += PP_Get( pp, ID_PARA_def_exp, NULL );
    exp += PP_Get( pp, ID_PARA_agi_exp, NULL );
    exp += PP_Get( pp, ID_PARA_spepow_exp, NULL );
    exp += PP_Get( pp, ID_PARA_spedef_exp, NULL );
  }
  *ret_wk = exp;

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 手持ちポケモンのHPチェック
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCheckPokemonHP( VMHANDLE * core, void *wk )
{

  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );  //結果格納ワーク
  u16 pos = SCRCMD_GetVMWorkValue( core, wk );  //ポケモンの位置

  {
#if 0
    GAMEDATA *gamedata = SCRCMD_WORK_GetGameData( wk );
    POKEPARTY * party = GAMEDATA_GetMyPokemon( gamedata );
    u32 max = PokeParty_GetPokeCount( party );
    POKEMON_PARAM * pp;
    u16 nowHP, maxHP;

    if (pos >= max)
    {
      GF_ASSERT_MSG(0, "Temoti Pos %d over max(%d)!!\n", pos, max);
      pos = 0;
    }
    pp = PokeParty_GetMemberPointer( party, pos );
#endif
    POKEMON_PARAM * pp;
    *ret_wk = FALSE;
    if ( SCRCMD_GetTemotiPP( wk, pos, &pp ) == TRUE )
    {
      u16 nowHP, maxHP;
      nowHP = PP_Get( pp, ID_PARA_hp, NULL );
      maxHP = PP_Get( pp, ID_PARA_hpmax, NULL );
      if (nowHP == maxHP)
      {
        *ret_wk = TRUE;
      }
    }
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 手持ちポケモンのタマゴかどうかの判定
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCheckPokemonEgg( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK*   work = (SCRCMD_WORK*)wk;
  u16*         ret_wk = SCRCMD_GetVMWork( core, wk );       // 結果格納先ワーク
  u16             pos = SCRCMD_GetVMWorkValue( core, wk );  // 判定ポケモン指定
  POKEMON_PARAM * pp;

  *ret_wk = FALSE;
  if ( SCRCMD_GetTemotiPP( work, pos, &pp ) == TRUE )
  {
    if ( PP_Get( pp, ID_PARA_tamago_flag, NULL ) == 0 )
    {
      *ret_wk = FALSE;
    }
    else
    {
      *ret_wk = TRUE;
    }
  }
#if 0
  GAMEDATA*     gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*    party = GAMEDATA_GetMyPokemon( gdata );
  int             max = PokeParty_GetPokeCountMax( party );
  u32     tamago_flag = 0;
  POKEMON_PARAM* param = NULL;

  // ポケモン指定に対する例外処理
  if( (pos < 0) || (max <= pos) )
  {
    *ret_wk = FALSE;
    return VMCMD_RESULT_CONTINUE;
  }

  // タマゴフラグ取得
  param       = PokeParty_GetMemberPointer( party, pos );
  tamago_flag = PP_Get( param, ID_PARA_tamago_flag, NULL );

  // 結果を格納
  if( tamago_flag == 0 ) *ret_wk = FALSE;
  else                   *ret_wk = TRUE;
#endif
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 手持ちポケモンのなつき度を取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPokemonFriendValue( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK*    work = (SCRCMD_WORK*)wk;
  u16*            ret_wk = SCRCMD_GetVMWork( core, wk );       // 結果格納先ワーク
  u16             pos = SCRCMD_GetVMWorkValue( core, wk );  // 判定ポケモン指定

  POKEMON_PARAM * pp;
  *ret_wk = 0;
  if ( SCRCMD_GetTemotiPP( work, pos, &pp ) == TRUE )
  {
    if ( PP_Get( pp, ID_PARA_tamago_flag, NULL ) == FALSE )
    {
      *ret_wk = PP_Get( pp, ID_PARA_friend, NULL );
    }
  }
#if 0
  POKEPARTY*    party = GAMEDATA_GetMyPokemon( SCRCMD_WORK_GetGameData(work) );
  int             max = PokeParty_GetPokeCountMax( party );
  u8            friend = 0;
  POKEMON_PARAM* param = NULL;

  // ポケモン指定に対する例外処理
  if( (pos < 0) || (PokeParty_GetPokeCountMax(party) <= pos) )
  {
    *ret_wk = 0;
    return VMCMD_RESULT_CONTINUE;
  }

  // なつき度取得
  param       = PokeParty_GetMemberPointer( party, pos );
  if(PP_Get( param, ID_PARA_tamago_flag, NULL ) == TRUE){
    friend = PP_Get( param, ID_PARA_friend, NULL); 
  }

  // 結果を格納
  *ret_wk = friend;
#endif
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 手持ちポケモンのなつき度をセット
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSetPokemonFriendValue( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK*    work = (SCRCMD_WORK*)wk;
  u16             pos = SCRCMD_GetVMWorkValue( core, wk );  // 判定ポケモン指定
  u16             value = SCRCMD_GetVMWorkValue( core, wk );  // 設定値
  u16             mode = VMGetU16( core );  // 設定モード(0:セット,1:足す,2:引く)

  u8            friend = 0;
  POKEMON_PARAM* pp = NULL;

  if ( SCRCMD_GetTemotiPP( work, pos, &pp ) == FALSE )
  { // ポケモン指定に対する例外処理
    return VMCMD_RESULT_CONTINUE;
  }

  // なつき度取得
  if(PP_Get( pp, ID_PARA_tamago_flag, NULL ) == TRUE)  //タマゴならなにもしない
  {
    return VMCMD_RESULT_CONTINUE;
  }
  friend = PP_Get( pp, ID_PARA_friend, NULL); 

  switch(mode)
  {
  case 0: //セット
    if(value > PTL_FRIEND_MAX){
      friend = PTL_FRIEND_MAX;
    }else if(value < 0){
      friend = 0;
    }else{
      friend = value;
    }
    break;
  case 1: //加算
    if((friend + value) > PTL_FRIEND_MAX) {
      friend = PTL_FRIEND_MAX;
    }else{
      friend += value;
    }
    break;
  case 2: //減算
    if(friend < value){
      friend = 0;
    }else{
      friend -= value;
    }
  }
  PP_Put( pp, ID_PARA_friend, friend); 
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 手持ちポケモンのモンスターナンバーを取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPartyPokeMonsNo( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK*    work = (SCRCMD_WORK*)wk;
  u16*          ret_wk = SCRCMD_GetVMWork( core, wk );       // 結果格納先ワーク
  u16              pos = SCRCMD_GetVMWorkValue( core, wk );  // 判定ポケモン指定
  GAMEDATA*      gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*     party = GAMEDATA_GetMyPokemon( gdata );
  POKEMON_PARAM* param = PokeParty_GetMemberPointer( party, pos );

  *ret_wk = (u16)PP_Get( param, ID_PARA_monsno, NULL );
  OBATA_Printf( "EvCmdGetPartyPokeMonsNo : %d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 手持ちポケモンの形状ナンバーを取得する
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPartyPokeFormNo( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK*    work = (SCRCMD_WORK*)wk;
  u16*          ret_wk = SCRCMD_GetVMWork( core, wk );       // 結果格納先ワーク
  u16              pos = SCRCMD_GetVMWorkValue( core, wk );  // 判定ポケモン指定
  GAMEDATA*      gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*     party = GAMEDATA_GetMyPokemon( gdata );
  POKEMON_PARAM* param = PokeParty_GetMemberPointer( party, pos );

  *ret_wk = (u16)PP_Get( param, ID_PARA_form_no, NULL );
  OBATA_Printf( "EvCmdGetPartyPokeFormNo : %d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPartyFrontPoke( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK* work = (SCRCMD_WORK*)wk;
  u16*       ret_wk = SCRCMD_GetVMWork( core, work );     // コマンド第1引数
  u16          mode = SCRCMD_GetVMWorkValue( core, wk );  // コマンド第2引数
  GAMEDATA*   gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*  party = GAMEDATA_GetMyPokemon( gdata );
  u16 num;

  switch( mode )
  {
  case POKECOUNT_MODE_BATTLE_ENABLE:  // 戦える = タマゴ以外
    num = PokeParty_GetMemberTopIdxBattleEnable( party );
    break;
  case POKECOUNT_MODE_NOT_EGG:  // タマゴを除く
    num = PokeParty_GetMemberTopIdxNotEgg( party );
    break;
  default:
    num = 0;
    GF_ASSERT_MSG(0, "Get Poke Index MODE:%d is not enable!!\n", mode );
    break;
  }

  *ret_wk = (u16)num;
  return VMCMD_RESULT_CONTINUE;
}


//--------------------------------------------------------------
/**
 * @brief 手持ちポケモンの数を取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPartyPokeCount( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK* work = (SCRCMD_WORK*)wk;
  u16*       ret_wk = SCRCMD_GetVMWork( core, work );
  u16          mode = SCRCMD_GetVMWorkValue( core, wk );
  GAMEDATA*   gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*  party = GAMEDATA_GetMyPokemon( gdata );
  int           num = 0;

  // 指定されたモードに応じたポケモン数をカウント
  switch( mode )
  {
  case POKECOUNT_MODE_TOTAL:
    num = PokeParty_GetPokeCount( party );
    break;
  case POKECOUNT_MODE_NOT_EGG:
    num = PokeParty_GetPokeCountNotEgg( party );
    break;
  case POKECOUNT_MODE_BATTLE_ENABLE:
    num = PokeParty_GetPokeCountBattleEnable( party );
    break;
  case POKECOUNT_MODE_ONLY_EGG:
    num = PokeParty_GetPokeCountOnlyEgg( party );
    break;
  case POKECOUNT_MODE_ONLY_DAME_EGG:
    num = PokeParty_GetPokeCountOnlyDameEgg( party );
    break;
  case POKECOUNT_MODE_EMPTY:
    {
      int max = PokeParty_GetPokeCountMax( party );
      int now = PokeParty_GetPokeCount( party );
      if ( max<now )
      {
        GF_ASSERT_MSG(0,"max over max=%d now=%d",max, now);
        num = 0;
      }
      else num = max - now;
    }
    break;
  default:
    num = 0;
    break;
  }

  *ret_wk = (u16)num;
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 手持ちポケモンの数を取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPartyPokeCountByMonsNo( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK* work = (SCRCMD_WORK*)wk;
  u16      target_monsno = SCRCMD_GetVMWorkValue( core, work );
  u16*      ret_wk = SCRCMD_GetVMWork( core, work );
  GAMEDATA*   gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*  party = GAMEDATA_GetMyPokemon( gdata );
  
  u16 num,total;

  total = PokeParty_GetPokeCount( party );
  num = 0;

  if ( (0<target_monsno)&&(target_monsno<=MONSNO_END) )
  {
    int i;
    for (i=0;i<total;i++)
    {
      POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, i );
      if (pp != NULL)
      {
        u32 tamago_flag = PP_Get( pp, ID_PARA_tamago_flag, NULL );
        if (!tamago_flag)    //タマゴチェック
        {
          u16 monsno;
          //モンスターナンバーを取得
          monsno = PP_Get(pp,ID_PARA_monsno,NULL);
          if (monsno == target_monsno) num++;
        }
      }
    }

    *ret_wk = num;
  }
  else
  {
    GF_ASSERT(0);
    *ret_wk = 0;
  }
  return VMCMD_RESULT_CONTINUE;
} 

//--------------------------------------------------------------
/**
 * @brief ボックス内のポケモン数を取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdGetBoxPokeCount( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK* work = (SCRCMD_WORK*)wk;
  u16*       ret_wk = SCRCMD_GetVMWork( core, work );     // コマンド第1引数
  u16          mode = SCRCMD_GetVMWorkValue( core, wk );  // コマンド第2引数
  GAMEDATA*   gdata = SCRCMD_WORK_GetGameData( work );
  BOX_MANAGER*  box = GAMEDATA_GetBoxManager( gdata );
  POKEPARTY*  party = GAMEDATA_GetMyPokemon( gdata );
  int           num = 0;

  // 指定されたモードに応じたポケモン数をカウント
  switch( mode )
  {
  case POKECOUNT_MODE_TOTAL:  // タマゴ込み
    num = BOXDAT_GetPokeExistCountTotal( box );
    break;
  case POKECOUNT_MODE_NOT_EGG:  // タマゴを除く
  case POKECOUNT_MODE_BATTLE_ENABLE:  // 戦える = タマゴ以外
    num = BOXDAT_GetPokeExistCount2Total( box );
    break;
  case POKECOUNT_MODE_ONLY_EGG: // タマゴのみ
    num  = BOXDAT_GetPokeExistCountTotal( box );
    num -= BOXDAT_GetPokeExistCount2Total( box );
    break;
  case POKECOUNT_MODE_EMPTY:   //空き数
    {
      int max = BOX_POKESET_MAX;
      int now = BOXDAT_GetPokeExistCountTotal( box );
      if ( max<now )
      {
        GF_ASSERT_MSG(0,"max over max=%d now=%d",max, now);
        num = 0;
      }
      else num = max - now;
    }
    break;
  default:
    num = 0;
    break;
  }

  *ret_wk = (u16)num;
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 指定モンスターナンバーのポケモンが手持ちのどこにいるかを取得（先頭から数えて始めに見つかった位置を返す）
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPartyPosByMonsNo( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK* work = (SCRCMD_WORK*)wk;
  u16      target_monsno = SCRCMD_GetVMWorkValue( core, work );
  u16*      ret_wk = SCRCMD_GetVMWork( core, work );
  u16*      pos = SCRCMD_GetVMWork( core, work );
  GAMEDATA*   gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*  party = GAMEDATA_GetMyPokemon( gdata );
  
  u16 num,total;

  total = PokeParty_GetPokeCount( party );
  num = 0;

  if ( (0<target_monsno)&&(target_monsno<=MONSNO_END) )
  {
    int i;
    for (i=0;i<total;i++)
    {
      POKEMON_PARAM * pp = PokeParty_GetMemberPointer( party, i );
      if (pp != NULL)
      {
        u32 tamago_flag = PP_Get( pp, ID_PARA_tamago_flag, NULL );
        if (!tamago_flag)    //タマゴチェック
        {
          u16 monsno;
          //モンスターナンバーを取得
          monsno = PP_Get(pp,ID_PARA_monsno,NULL);
          if (monsno == target_monsno) break;
        }
      }
    }

    if (i < total)
    {
      *ret_wk = TRUE;
      *pos = i;
    }else *ret_wk = FALSE;
  }
  else
  {
    GF_ASSERT(0);
    *ret_wk = FALSE;
  }
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * 手持ちポケモンが指定された技を覚えているかチェック
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdChkPokeWaza( VMHANDLE *core, void *wk )
{
  POKEMON_PARAM *pp;
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );
  u16 waza = SCRCMD_GetVMWorkValue( core, wk );
  u16 tno = SCRCMD_GetVMWorkValue( core, wk );
  
  *ret_wk = 0;
  if ( SCRCMD_GetTemotiPP( wk, tno, &pp ) == TRUE)
  {
    if ( SCRCMD_CheckTemotiWaza( pp, waza ) == TRUE )
    {
      *ret_wk = 1;
    }
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 手持ちポケモンが指定された技を覚えているかチェック　手持ちポケモン全体
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdChkPokeWazaGroup( VMHANDLE *core, void *wk )
{
  int i,max;
  POKEMON_PARAM *pp;
  GAMEDATA *gamedata = SCRCMD_WORK_GetGameData( wk );
  POKEPARTY *party = GAMEDATA_GetMyPokemon( gamedata );
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );
  u16 waza = SCRCMD_GetVMWorkValue( core, wk );
  
  max = PokeParty_GetPokeCount( party );

#ifdef DEBUG_ONLY_FOR_kagaya  //test
  *ret_wk = 0;
  return VMCMD_RESULT_CONTINUE;
#endif
  
  for( i = 0, *ret_wk = 6; i < max; i++ ){
    pp = PokeParty_GetMemberPointer( party, i );
    
    if ( SCRCMD_CheckTemotiWaza( pp, waza ) == TRUE )
    {
      *ret_wk = i;
      break;
    }
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ポケモンを手持ちに追加
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdAddPokemonToParty( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*    gdata = SCRCMD_WORK_GetGameData( work );
  HEAPID     heap_id = SCRCMD_WORK_GetHeapID( work );
  POKEPARTY*   party = GAMEDATA_GetMyPokemon( gdata );
  MYSTATUS*   status = GAMEDATA_GetMyStatus( gdata );
  u16*        ret_wk = SCRCMD_GetVMWork( core, work );       // コマンド第1引数
  u16         monsno = SCRCMD_GetVMWorkValue( core, work );  // コマンド第2引数
  u16         formno = SCRCMD_GetVMWorkValue( core, work );  // コマンド第3引数
  u16        tokusei = SCRCMD_GetVMWorkValue( core, work );  // コマンド第4引数
  u16          level = SCRCMD_GetVMWorkValue( core, work );  // コマンド第5引数
  u16         itemno = SCRCMD_GetVMWorkValue( core, work );  // コマンド第6引数
  POKEMON_PARAM*  pp = NULL;

  // 手持ちがいっぱいなら追加しない
  if( PokeParty_GetPokeCountMax(party) <= PokeParty_GetPokeCount(party) )
  {
    *ret_wk = FALSE;
    return VMCMD_RESULT_CONTINUE;
  }

  // 追加するポケモンを作成
  pp = PP_Create( monsno, level, PTL_SETUP_ID_AUTO, heap_id );
  PP_Put( pp, ID_PARA_form_no, formno );    // フォーム
  PP_Put( pp, ID_PARA_item, itemno );       // 所持アイテム

  if( tokusei != TOKUSYU_NULL ){
    PP_Put( pp, ID_PARA_speabino, tokusei );  // 特性
  }
  // 親の名前と性別
  PP_Put( pp, ID_PARA_id_no, (u32)MyStatus_GetID(status) );
  PP_Put( pp, ID_PARA_oyasex, MyStatus_GetMySex(status) );
  PP_Put( pp, ID_PARA_oyaname_raw, (u32)MyStatus_GetMyName(status) );

  PP_Renew( pp );

  // 手持ちに追加
  PokeParty_Add( party, pp );

  //一応図鑑登録
  ZUKANSAVE_SetPokeGet(GAMEDATA_GetZukanSave( gdata ), pp);

  GFL_HEAP_FreeMemory( pp );
  *ret_wk = TRUE;
  return VMCMD_RESULT_CONTINUE;
}


//--------------------------------------------------------------
/**
 * 手持ちポケモンの名前入力画面を呼び出す
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdPartyPokeNameInput( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16*        ret_wk = SCRCMD_GetVMWork( core, work );       // コマンド第1引数
  u16          index = SCRCMD_GetVMWorkValue( core, work );  // コマンド第2引数

  // イベントを呼び出す
  SCRIPT_CallEvent( scw, EVENT_NameInput_PartyPoke(gsys, ret_wk, index) );
  return VMCMD_RESULT_SUSPEND;
}


//--------------------------------------------------------------
/**
 * 手持ちのタマゴを孵化させる
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdPartyPokeEggBirth( VMHANDLE *core, void *wk )
{
  int i;
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );       // コマンド第1引数
  SCRIPT_WORK*        scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HEAPID          heap_id = FIELDMAP_GetHeapID( fieldmap );
  GAMEDATA*         gdata = GAMESYSTEM_GetGameData( gsys );
  MYSTATUS*            my = GAMEDATA_GetMyStatus( gdata );
  POKEPARTY*        party = GAMEDATA_GetMyPokemon( gdata );
  int          poke_count = PokeParty_GetPokeCount( party );


  for( i=0; i<poke_count; i++ )
  {
    POKEMON_PARAM* param = PokeParty_GetMemberPointer( party, i );
    u32      tamago_flag = PP_Get( param, ID_PARA_tamago_flag, NULL );

    if( tamago_flag )
    {
      // 孵化イベント
      SCRIPT_CallEvent( scw, EVENT_EggBirth( gsys, fieldmap, param ) );
      *ret_wk = i;
      return VMCMD_RESULT_SUSPEND;
    }
  }
  return VMCMD_RESULT_CONTINUE; 
}


//--------------------------------------------------------------
/**
 * ポケモンを選択する
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPartyPokeSelect( VMHANDLE *core, void *wk )
{
  int i;
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*         ret_decide = SCRCMD_GetVMWork( core, work );       // コマンド第1引数
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );       // コマンド第2引数
  u16               value = SCRCMD_GetVMWorkValue( core, work );  // コマンド第3引数(予備値)
  SCRIPT_WORK*        scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  
  {
    GMEVENT *event = EVENT_CreatePokeSelect( gsys , ret_decide , ret_wk );
    SCRIPT_CallEvent( scw, event );
  }
  
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 手持ちポケモンの技個数取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPokemonWazaNum( VMHANDLE *core, void *wk )
{
  int i;
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );       // コマンド第1引数
  u16                 pos = SCRCMD_GetVMWorkValue( core, work );       // コマンド第2引数

  SCRIPT_WORK*        scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GAMEDATA*         gdata = GAMESYSTEM_GetGameData( gsys );
  POKEPARTY*        party = GAMEDATA_GetMyPokemon( gdata );
  int          waza_count = 0;
  
  POKEMON_PARAM *pp = PokeParty_GetMemberPointer( party , pos );
  for( i=0; i<PTL_WAZA_MAX; i++ )
  {
    const u32 wazaNo = PP_Get( pp , ID_PARA_waza1+i , NULL );
    if( wazaNo != 0 )
    {
      waza_count++;
    }
  }
  *ret_wk = waza_count;
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 技を選択する
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPartyPokeWazaSelect( VMHANDLE *core, void *wk )
{
  int i;
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*         ret_decide = SCRCMD_GetVMWork( core, work );       // コマンド第1引数
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );       // コマンド第2引数
  u16             pokePos = SCRCMD_GetVMWorkValue( core, work );  // コマンド第3引数
  u16               value = SCRCMD_GetVMWorkValue( core, work );  // コマンド第4引数(予備値)
  SCRIPT_WORK*        scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  
  {
    GMEVENT *event = EVENT_CreateWazaSelect( gsys , fieldmap , pokePos , ret_decide , ret_wk );
    SCRIPT_CallEvent( scw, event );
  }
  
  return VMCMD_RESULT_SUSPEND;
}


//--------------------------------------------------------------
/**
 * 技IDの取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPokemonWazaID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );       // コマンド第1引数
  u16            poke_pos = SCRCMD_GetVMWorkValue( core, work );       // コマンド第2引数
  u16            waza_pos = SCRCMD_GetVMWorkValue( core, work );       // コマンド第3引数
  
  POKEMON_PARAM *pp;
  if ( SCRCMD_GetTemotiPP( work, poke_pos, &pp ) == TRUE )
  {
    const u32 wazaNo = PP_Get( pp , ID_PARA_waza1+waza_pos , NULL );
    *ret_wk = wazaNo;
  }
  else
  {
    *ret_wk = 0;  //エラー対処
  }
  return VMCMD_RESULT_CONTINUE;
}


//--------------------------------------------------------------
/**
 * 技の追加
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSetPokemonWaza( VMHANDLE *core, void *wk )
{
  int i;
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16            poke_pos = SCRCMD_GetVMWorkValue( core, work );       // コマンド第2引数
  u16            waza_pos = SCRCMD_GetVMWorkValue( core, work );       // コマンド第3引数
  u16             waza_id = SCRCMD_GetVMWorkValue( core, work );       // コマンド第3引数

  POKEMON_PARAM      *pp;
  BOOL          fastMode;

  if ( SCRCMD_GetTemotiPP( work, poke_pos, &pp ) == FALSE )
  { //エラー対処用
    return VMCMD_RESULT_CONTINUE;
  }

  fastMode = PP_FastModeOn( pp );

  if( waza_pos > PTL_WAZA_MAX )
  {
    //押し出しモード
    PP_SetWazaPush( pp , waza_id );
  }
  else
  if( waza_id == 0 )
  {
    //忘れモード
    u8 i;
    u8 forgetPos = waza_pos;
    if( waza_pos < PTL_WAZA_MAX-1 )
    {
      //詰める
      for( i=waza_pos ; i<PTL_WAZA_MAX-1 ; i++ )
      {
        const u32 temp_waza_no = PP_Get( pp , ID_PARA_waza1+i+1 , NULL );
        forgetPos = i+1;
        if( temp_waza_no != 0 )
        {
          const u32 temp_waza_cnt = PP_Get( pp , ID_PARA_pp_count1+i+1 , NULL );
          const u32 temp_waza_pp = PP_Get( pp , ID_PARA_pp1+i+1 , NULL );
          PP_Put( pp , ID_PARA_waza1+i , temp_waza_no );
          PP_Put( pp , ID_PARA_pp_count1+i , temp_waza_cnt );
          PP_Put( pp , ID_PARA_pp1+i , temp_waza_pp );
        }
        else
        {
          break;
        }
      }
    }
    PP_Put( pp , ID_PARA_waza1+forgetPos , 0 );
    PP_Put( pp , ID_PARA_pp_count1+forgetPos , 0 );
    PP_Put( pp , ID_PARA_pp1+forgetPos , 0 );
  }
  else
  {
    //置き換えモード
    PP_SetWazaPos( pp , waza_id , waza_pos );
    
  }
  PP_FastModeOff( pp , fastMode );
  return VMCMD_RESULT_CONTINUE;
  
}


//--------------------------------------------------------------
/**
 * @brief ゲーム内交換が可能かどうかをチェックする
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFieldPokeTradeCheck( VMHANDLE *core, void *wk )
{
  int i;
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*        scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*         gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  HEAPID          heap_id = FIELDMAP_GetHeapID( fieldmap );
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );       // コマンド第一引数
  u16            trade_no = SCRCMD_GetVMWorkValue( core, work );  // コマンド第二引数
  u16           party_pos = SCRCMD_GetVMWorkValue( core, work );  // コマンド第三引数
  POKEPARTY*        party = GAMEDATA_GetMyPokemon( gdata );
  POKEMON_PARAM*     poke = PokeParty_GetMemberPointer( party, party_pos );
  FLD_TRADE_WORK*   trade = FLD_TRADE_WORK_Create( heap_id, trade_no );
 
  // 全チェックをパスすれば交換可能
  *ret_wk = FLD_TRADE_ENABLE;

  // タマゴチェック
  if( *ret_wk == FLD_TRADE_ENABLE )
  {
    u32 tamago_flag = PP_Get( poke, ID_PARA_tamago_flag, NULL );
    if( tamago_flag == TRUE )
    {
      *ret_wk = FLD_TRADE_DISABLE_EGG;
    }
  }
  // モンスターNo.チェック
  if( *ret_wk == FLD_TRADE_ENABLE )
  {
    u32 monsno = PP_Get( poke, ID_PARA_monsno, NULL );
    if( monsno != FLD_TRADE_WORK_GetChangeMonsno(trade) )
    {
      *ret_wk = FLD_TRADE_DISABLE_MONSNO;
    }
  }
  // 性別チェック
  if( *ret_wk == FLD_TRADE_ENABLE )
  {
    if( FLD_TRADE_WORK_GetChangeMonsSex(trade) != PM_NEUTRAL )
    {
      u32 sex = PP_Get( poke, ID_PARA_sex, NULL );
      if( sex != FLD_TRADE_WORK_GetChangeMonsSex(trade) )
      {
        *ret_wk = FLD_TRADE_DISABLE_SEX;
      }
    } 
  } 

  FLD_TRADE_WORK_Delete( trade );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief ゲーム内交換イベントを呼ぶ
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdFieldPokeTrade( VMHANDLE *core, void *wk )
{
  int i;
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  u16       trade_no = SCRCMD_GetVMWorkValue( core, work );  // コマンド第1引数
  u16      party_pos = SCRCMD_GetVMWorkValue( core, work );  // コマンド第2引数
  
  {
    GMEVENT *event = EVENT_FieldPokeTrade( gsys, trade_no, party_pos );
    SCRIPT_CallEvent( scw, event );
  } 
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief 指定の手持ちポケモンの親が自分かどうかをチェックする
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCheckPokeOwner( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*   work = (SCRCMD_WORK*)wk;
  u16*         ret_wk = SCRCMD_GetVMWork( core, wk );       // 結果格納先ワーク
  u16             pos = SCRCMD_GetVMWorkValue( core, wk );  // 判定ポケモン指定
  GAMEDATA*     gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*    party = GAMEDATA_GetMyPokemon( gdata );
  int             max = PokeParty_GetPokeCountMax( party );
  MYSTATUS*   status = GAMEDATA_GetMyStatus( gdata );
  u32     id = 0;
  POKEMON_PARAM* param = NULL;

  // ポケモン指定に対する例外処理
  if( (pos < 0) || (max <= pos) )
  {
    *ret_wk = FALSE;
    return VMCMD_RESULT_CONTINUE;
  }

  // ID取得
  param       = PokeParty_GetMemberPointer( party, pos );
  id = PP_Get( param, ID_PARA_id_no, NULL );

  NOZOMU_Printf("poke_id = %d\n",id);
  NOZOMU_Printf("my_id = %d\n",MyStatus_GetID(status) );

  // 結果を格納
  if ( id != MyStatus_GetID(status) ) *ret_wk = FALSE;    //親は他人
  else  *ret_wk = TRUE;     //自分が親

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * フォルムチェンジ
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdChgFormNo( VMHANDLE *core, void *wk )
{
  int i;
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16            poke_pos = SCRCMD_GetVMWorkValue( core, work );       //手持ち位置
  u16            formno = SCRCMD_GetVMWorkValue( core, work );       //フォルムナンバー

  SCRIPT_WORK*        scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GAMEDATA*         gdata = GAMESYSTEM_GetGameData( gsys );
  POKEPARTY*        party = GAMEDATA_GetMyPokemon( gdata );
  
  POKEMON_PARAM *pp = PokeParty_GetMemberPointer( party , poke_pos );
  BOOL rc;

  rc = PP_ChangeFormNo( pp, formno );
  GF_ASSERT(rc);
  
  //図鑑登録「見た」
  {
    ZUKAN_SAVEDATA *zw = GAMEDATA_GetZukanSave( gdata );
    ZUKANSAVE_SetPokeSee(zw, pp);
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ロトムフォルムチェンジ
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdChgRotomFormNo( VMHANDLE *core, void *wk )
{
  int i;
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  u16            poke_pos = SCRCMD_GetVMWorkValue( core, work );       //手持ち位置
  u16            waza_pos = SCRCMD_GetVMWorkValue( core, work );       //技位置
  u16            formno = SCRCMD_GetVMWorkValue( core, work );       //フォルムナンバー

  SCRIPT_WORK*        scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GAMEDATA*         gdata = GAMESYSTEM_GetGameData( gsys );
  POKEPARTY*        party = GAMEDATA_GetMyPokemon( gdata );
  
  POKEMON_PARAM *pp = PokeParty_GetMemberPointer( party , poke_pos );
  BOOL rc;

  rc = PP_ChangeRotomFormNo( pp, formno, waza_pos );
  GF_ASSERT(rc);
  
  //図鑑登録「見た」
  {
    ZUKAN_SAVEDATA *zw = GAMEDATA_GetZukanSave( gdata );
    ZUKANSAVE_SetPokeSee(zw, pp);
  }

  return VMCMD_RESULT_CONTINUE;
}


//--------------------------------------------------------------
/**
 * @brief 思い出し技の有無を取得する
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCheckRemaindWaza( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GAMEDATA*         gdata = GAMESYSTEM_GetGameData( gsys );
  POKEPARTY*        party = GAMEDATA_GetMyPokemon( gdata );
  u16*             ret_wk = SCRCMD_GetVMWork( core, work );     // コマンド第一引数(結果を受け取るワーク)
  u16            poke_pos = SCRCMD_GetVMWorkValue( core, work );// コマンド第二引数(手持ち位置)
  POKEMON_PARAM*     poke = PokeParty_GetMemberPointer( party, poke_pos );
  HEAPID          heap_id = SCRCMD_WORK_GetHeapID( work );
  u16*               waza = NULL;

  // 思い出し技の有無を返す
  waza = WAZAOSHIE_GetRemaindWaza( poke, heap_id );
  *ret_wk = WAZAOSHIE_WazaTableChack( waza );
  GFL_HEAP_FreeMemory( waza );
  return VMCMD_RESULT_CONTINUE;
}


//--------------------------------------------------------------
/**
 * @brief 捕獲場所IDの取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCheckPartyPokeGetPlace( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*    work = (SCRCMD_WORK*)wk;
  u16*          ret_wk = SCRCMD_GetVMWork( core, wk );       // 結果格納先ワーク
  u16              pos = SCRCMD_GetVMWorkValue( core, wk );  // 判定ポケモン指定
  u16              type = SCRCMD_GetVMWorkValue( core, wk );  // POKE_GET_PLACE_CHECK_～
  GAMEDATA*      gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*     party = GAMEDATA_GetMyPokemon( gdata );
  POKEMON_PARAM* param = PokeParty_GetMemberPointer( party, pos );
  u16 place;
  static const u16 POKE_GET_PLACE_CHECK_PLACE[POKE_GET_PLACE_CHECK_MAX] = 
  {
    MAPNAME_WC10,
  };

  GF_ASSERT( type < POKE_GET_PLACE_CHECK_MAX );
  
  place = (u16)PP_Get( param, ID_PARA_get_place, NULL );
  
  if( place == POKE_GET_PLACE_CHECK_PLACE[ type ] ){
    *ret_wk = TRUE;
  }else{
    *ret_wk = FALSE;
  }
  
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 捕獲日時の取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPartyPokeGetDate( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*    work = (SCRCMD_WORK*)wk;
  u16*          ret_year = SCRCMD_GetVMWork( core, wk );       // 結果格納先ワーク
  u16*          ret_month = SCRCMD_GetVMWork( core, wk );       // 結果格納先ワーク
  u16*          ret_day = SCRCMD_GetVMWork( core, wk );       // 結果格納先ワーク
  u16              pos = SCRCMD_GetVMWorkValue( core, wk );  // 判定ポケモン指定
  GAMEDATA*      gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*     party = GAMEDATA_GetMyPokemon( gdata );
  POKEMON_PARAM* param = PokeParty_GetMemberPointer( party, pos );

  *ret_year = (u16)PP_Get( param, ID_PARA_get_year, NULL );
  *ret_month = (u16)PP_Get( param, ID_PARA_get_month, NULL );
  *ret_day = (u16)PP_Get( param, ID_PARA_get_day, NULL );
  return VMCMD_RESULT_CONTINUE;
}


