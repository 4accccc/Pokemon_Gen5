//======================================================================
/**
 * @file  scrcmd_word.c
 * @brief  スクリプトコマンド：単語関連
 * @author  Satoshi Nohara
 * @date  06.06.26
 */
//======================================================================
#include <nitro/code16.h> 
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "item/item.h"
#include "savedata/myitem_savedata.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"

#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_word.h"

#include "msgdata.h"
#include "print/wordset.h"
#include "print/global_msg.h" //GlobalMsg_PokeName

#include "savedata/sodateya_work.h"
#include "sodateya.h"
#include "fieldmap.h"

#include "print/str_tool.h"

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
//  スクリプトコマンド：単語関連
//======================================================================
//--------------------------------------------------------------
/**
 * プレイヤー名を指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerName( VMHANDLE * core, void *wk ) 
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  MYSTATUS *mystatus = GAMEDATA_GetMyStatus( gdata );
  WORDSET *wordset    = SCRIPT_GetWordSet( sc );
  u8 idx = VMGetU8(core);
  
  WORDSET_RegisterPlayerName( wordset, idx, mystatus );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * アイテム名を指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdItemName( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET *wordset    = SCRIPT_GetWordSet( sc );
 	u8 idx = VMGetU8( core );
	u16 itemno = SCRCMD_GetVMWorkValue( core, work );

  WORDSET_RegisterItemName( wordset, idx, itemno );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 技マシンから技名を指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdItemWazaName( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET *wordset    = SCRIPT_GetWordSet( sc );
	u8 idx = VMGetU8(core);
	u16 itemno = SCRCMD_GetVMWorkValue( core, work );
  u16 wazano = ITEM_GetWazaNo( itemno );

  WORDSET_RegisterWazaName( wordset, idx, wazano );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 技名を指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdWazaName( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET *wordset    = SCRIPT_GetWordSet( sc );
	u8 idx = VMGetU8(core);
	u16 wazano = SCRCMD_GetVMWorkValue( core, work );
  
  WORDSET_RegisterWazaName( wordset, idx, wazano );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * タイプ名を指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPokeTypeName( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET *wordset    = SCRIPT_GetWordSet( sc );
	u8 idx = VMGetU8(core);
	u16 type_no = SCRCMD_GetVMWorkValue( core, work );
  
  WORDSET_RegisterPokeTypeName( wordset, idx, type_no );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * タイプ名を指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMonsName( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET *wordset    = SCRIPT_GetWordSet( sc );
	u8 idx = VMGetU8(core);
	u16 monsno = SCRCMD_GetVMWorkValue( core, work );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  STRBUF * tmpBuf;

  tmpBuf = GFL_STR_CreateBuffer( WORDSET_DEFAULT_BUFLEN, heapID );
  GFL_MSG_GetString( GlobalMsg_PokeName, monsno, tmpBuf );
  //WORDSET, タグ位置、バッファ、性別、単複、言語コード
  WORDSET_RegisterWord( wordset, idx, tmpBuf, 0, TRUE, PM_LANG );
  
  GFL_STR_DeleteBuffer( tmpBuf );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * ポケット名を指定バッファに登録
 *
 * @param	core		仮想マシン制御構造体へのポインタ
 *
 * @return	VMCMD_RESULT_CONTINUE
 */
//--------------------------------------------------------------------------------------------
VMCMD_RESULT EvCmdPocketName(VMHANDLE * core, void *wk ) 
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
	WORDSET* wordset		= SCRIPT_GetWordSet( sc );
	u8 idx					= VMGetU8(core);
	u16 pocket				= SCRCMD_GetVMWorkValue( core, work );

  WORDSET_RegisterItemPocketName( wordset, idx, pocket );		//アイコンなし
	//WORDSET_RegisterItemPocketWithIcon( *wordset, idx, pocket );	//アイコンあり

	return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 手持ちポケモンの種族名を指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPartyPokeMonsName(VMHANDLE * core, void *wk ) 
{
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*      gdata = SCRCMD_WORK_GetGameData( work );
  WORDSET*     wordset = SCRIPT_GetWordSet( sc );
 	u8               idx = VMGetU8( core );
  u16              pos = SCRCMD_GetVMWorkValue( core, work );
  POKEPARTY*     party = GAMEDATA_GetMyPokemon( gdata );
  POKEMON_PARAM* param = PokeParty_GetMemberPointer( party, pos );

  WORDSET_RegisterPokeMonsName( wordset, idx, param );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 手持ちポケモンのニックネームを指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPartyPokeNickName(VMHANDLE * core, void *wk ) 
{
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*      gdata = SCRCMD_WORK_GetGameData( work );
  WORDSET*     wordset = SCRIPT_GetWordSet( sc );
 	u8               idx = VMGetU8( core );
  u16              pos = SCRCMD_GetVMWorkValue( core, work );
  POKEPARTY*     party = GAMEDATA_GetMyPokemon( gdata );
  POKEMON_PARAM* param = PokeParty_GetMemberPointer( party, pos );

  WORDSET_RegisterPokeNickName( wordset, idx, param );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 育て屋のポケモンの種族名を指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSodateyaPokeMonsName(VMHANDLE * core, void *wk ) 
{
  SCRCMD_WORK*       work = wk;
  SCRIPT_WORK*         sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*        wordset = SCRIPT_GetWordSet( sc );
 	u8                  idx = VMGetU8( core );
  u16                 pos = SCRCMD_GetVMWorkValue( core, work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );
  const POKEMON_PARAM* pp = SODATEYA_GetPokemonParam( sodateya, pos );

  WORDSET_RegisterPokeMonsName( wordset, idx, pp );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 育て屋のポケモンのニックネームを指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSodateyaPokeNickName(VMHANDLE * core, void *wk ) 
{
  SCRCMD_WORK*       work = wk;
  SCRIPT_WORK*         sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*        wordset = SCRIPT_GetWordSet( sc );
 	u8                  idx = VMGetU8( core );
  u16                 pos = SCRCMD_GetVMWorkValue( core, work );
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SODATEYA*      sodateya = FIELDMAP_GetSodateya( fieldmap );
  const POKEMON_PARAM* pp = SODATEYA_GetPokemonParam( sodateya, pos );

  WORDSET_RegisterPokeNickName( wordset, idx, pp );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 数字を指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdNumber(VMHANDLE * core, void *wk )
{
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*     wordset = SCRIPT_GetWordSet( sc );
 	u8               idx = VMGetU8( core );
  u16              num = SCRCMD_GetVMWorkValue( core, work );
  u16             keta = SCRCMD_GetVMWorkValue( core, work );

  WORDSET_RegisterNumber( wordset, idx, num, keta, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
/**
 * 手持ちポケモンのニックネームを指定バッファに登録
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdNickName( VMHANDLE *core, void *wk )
{
  POKEMON_PARAM *pp;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA *gamedata = SCRCMD_WORK_GetGameData( wk );
  POKEPARTY *party = GAMEDATA_GetMyPokemon( gamedata );
	WORDSET *wordset = SCRIPT_GetWordSet( sc );
	u8 idx = VMGetU8( core );
	u16 pos = SCRCMD_GetVMWorkValue( core, work );
  
	//ポケモンへのポインタ取得
  pp = PokeParty_GetMemberPointer( party, pos );
    
	//POKEMON_PARAMからPOKEMON_PASO_PARAMに渡しているので注意！(田谷さんに確認済み)
  WORDSET_RegisterPokeNickName( wordset, idx, pp );
	return VMCMD_RESULT_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 地名を表示する
 * @param  core  仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//------------------------------------------------------------------------------------------
VMCMD_RESULT EvCmdPlaceName( VMHANDLE *core, void *wk )
{ 
  SCRCMD_WORK* work = wk;
  SCRIPT_WORK*   sc = SCRCMD_WORK_GetScriptWork( work );
	WORDSET*  wordset = SCRIPT_GetWordSet( sc );
	u16           idx = SCRCMD_GetVMWorkValue( core, work );  // 第一引数
	u16       zone_id = SCRCMD_GetVMWorkValue( core, work );  // 第二引数

  WORDSET_RegisterPlaceName( wordset, idx, zone_id );
	return VMCMD_RESULT_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 簡易会話単語をバッファに登録
 * @param  core  仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//------------------------------------------------------------------------------------------
VMCMD_RESULT EvCmdEasyTalkWordName( VMHANDLE *core, void *wk )
{ 
  SCRCMD_WORK* work = wk;
  SCRIPT_WORK*   sc = SCRCMD_WORK_GetScriptWork( work );
	WORDSET*  wordset = SCRIPT_GetWordSet( sc );
	u8           idx = VMGetU8( core );  // 第一引数
  PMS_WORD      word = SCRCMD_GetVMWorkValue( core, work );  // 第二引数

  WORDSET_RegisterPMSWord( wordset, idx, word );

	return VMCMD_RESULT_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 国名をバッファに登録
 * @param  core  仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//------------------------------------------------------------------------------------------
VMCMD_RESULT EvCmdCountryName( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK* work = wk;
  SCRIPT_WORK*   sc = SCRCMD_WORK_GetScriptWork( work );
	WORDSET*  wordset = SCRIPT_GetWordSet( sc );
  u8 idx;
  u16 country;

  idx = VMGetU8( core );  // 第一引数
  country = SCRCMD_GetVMWorkValue( core, work );  // 第二引数

  WORDSET_RegisterCountryName( wordset, idx, country );

  return VMCMD_RESULT_CONTINUE;
}

//------------------------------------------------------------------------------------------
/**
 * @brief 趣味名をバッファに登録
 * @param  core  仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//------------------------------------------------------------------------------------------
VMCMD_RESULT EvCmdHobbyName( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK* work    = wk;
  SCRIPT_WORK* sc      = SCRCMD_WORK_GetScriptWork( work );
	WORDSET*     wordset = SCRIPT_GetWordSet( sc );
  u8           bufIdx  = VMGetU8( core );                      // 第一引数
  u16          hobbyID = SCRCMD_GetVMWorkValue( core, work );  // 第二引数

  WORDSET_RegisterCountryName( wordset, bufIdx, hobbyID ); 
  return VMCMD_RESULT_CONTINUE;
}
