//======================================================================
/**
 * @file	scrcmd_pokemon.c
 * @brief	スクリプトコマンド：ポケモン関連
 * @date  2009.09.15
 * @author	tamada GAMEFREAK inc.
 *
 * @todo  ポケルスチェックの確認が行われていない
 * @todo  status_rcv.cの命名規則修正ができたら回復処理を呼び出す
 */
//======================================================================

#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "script.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_pokemon.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/pokeparty.h"

#include "poke_tool/status_rcv.h" //PokeParty_RecoverAll

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================

//======================================================================
//  proto
//======================================================================

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

  PokeParty_RecoverAll( party );

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
    nowHP = PP_Get( pp, ID_PARA_hp, NULL );
    maxHP = PP_Get( pp, ID_PARA_hpmax, NULL );
    *ret_wk = FALSE;
    if (nowHP == maxHP)
    {
      *ret_wk = TRUE;
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
  u8              mode = VMGetU8( core );  // 設定モード(0:セット,1:足す,2:引く)

  POKEPARTY*    party = GAMEDATA_GetMyPokemon( SCRCMD_WORK_GetGameData(work) );
  u8            friend = 0;
  POKEMON_PARAM* param = NULL;

  // ポケモン指定に対する例外処理
  if( (pos < 0) || (PokeParty_GetPokeCountMax(party) <= pos) )
  {
    return VMCMD_RESULT_CONTINUE;
  }

  // なつき度取得
  param       = PokeParty_GetMemberPointer( party, pos );
  if(PP_Get( param, ID_PARA_tamago_flag, NULL ) == TRUE)  //タマゴならなにもしない
  {
    return VMCMD_RESULT_CONTINUE;
  }
  friend = PP_Get( param, ID_PARA_friend, NULL); 

  switch(mode)
  {
  case 0: //セット
    if(value > 256){
      friend = 255;
    }else if(value < 0){
      friend = 0;
    }else{
      friend = value;
    }
    break;
  case 1: //加算
    if((friend + value) > 256) {
      friend = 255;
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
  PP_Put( param, ID_PARA_friend, friend); 
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
  GAMEDATA*   gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*  party = GAMEDATA_GetMyPokemon( gdata );

  *ret_wk = (u16)PokeParty_GetPokeCount( party );
  return VMCMD_RESULT_CONTINUE;
} 


//--------------------------------------------------------------
/**
 * @brief 戦える手持ちポケモンの数を取得
 *     (手持ちからタマゴ, 瀕死のポケモンを除いた数を取得する)
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPartyBattlePokeCount( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK* work = (SCRCMD_WORK*)wk;
  u16*       ret_wk = SCRCMD_GetVMWork( core, work );
  GAMEDATA*   gdata = SCRCMD_WORK_GetGameData( work );
  POKEPARTY*  party = GAMEDATA_GetMyPokemon( gdata );

  *ret_wk = (u16)PokeParty_GetBattlePokeNum( party );
  return VMCMD_RESULT_CONTINUE;
} 


//--------------------------------------------------------------
/**
 * @brief 所持金を増やす
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdAddGold( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*   work = (SCRCMD_WORK*)wk;
  u16*         ret_wk = SCRCMD_GetVMWork( core, work );
  GAMEDATA*     gdata = SCRCMD_WORK_GetGameData( work );
  PLAYER_WORK* player = GAMEDATA_GetMyPlayerWork( gdata );
  MYSTATUS*  mystatus = &player->mystatus;
  u16             val = SCRCMD_GetVMWorkValue( core, wk );
  u32            gold = MyStatus_GetGold( mystatus );

  gold = gold + val;
  MyStatus_SetGold( mystatus, gold );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 所持金を減らす
 * @param	core		仮想マシン制御構造体へのポインタ
 * @param wk      SCRCMD_WORKへのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSubtractGold( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*   work = (SCRCMD_WORK*)wk;
  u16*         ret_wk = SCRCMD_GetVMWork( core, work );
  GAMEDATA*     gdata = SCRCMD_WORK_GetGameData( work );
  PLAYER_WORK* player = GAMEDATA_GetMyPlayerWork( gdata );
  MYSTATUS*  mystatus = &player->mystatus;
  u16             val = SCRCMD_GetVMWorkValue( core, wk );
  u32            gold = MyStatus_GetGold( mystatus );

  gold = gold - val;
  MyStatus_SetGold( mystatus, gold );
  return VMCMD_RESULT_CONTINUE;
}

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
  GAMEDATA *gamedata = SCRCMD_WORK_GetGameData( wk );
  POKEPARTY *party = GAMEDATA_GetMyPokemon( gamedata );
  u16 *ret_wk = SCRCMD_GetVMWork( core, wk );
  u16 waza = SCRCMD_GetVMWorkValue( core, wk );
  u16 tno = SCRCMD_GetVMWorkValue( core, wk );
  
  *ret_wk = 0;
  pp = PokeParty_GetMemberPointer( party, tno );
  
  //たまごチェック
  if( PP_Get(pp,ID_PARA_tamago_flag,NULL) != 0 ){
    return VMCMD_RESULT_CONTINUE;
  }
  
  //技リストからチェック
  if( PP_Get(pp,ID_PARA_waza1,NULL) == waza ||
      PP_Get(pp,ID_PARA_waza2,NULL) == waza ||
      PP_Get(pp,ID_PARA_waza3,NULL) == waza ||
      PP_Get(pp,ID_PARA_waza4,NULL) == waza ){
    *ret_wk = 1;
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
    
    //たまごチェック
    if( PP_Get(pp,ID_PARA_tamago_flag,NULL) != 0 ){
      continue;
    }
    
    //技リストからチェック
    if( PP_Get(pp,ID_PARA_waza1,NULL) == waza ||
        PP_Get(pp,ID_PARA_waza2,NULL) == waza ||
        PP_Get(pp,ID_PARA_waza3,NULL) == waza ||
        PP_Get(pp,ID_PARA_waza4,NULL) == waza ){
      *ret_wk = i;
      break;
    }
  }
  
  return VMCMD_RESULT_CONTINUE;
}

