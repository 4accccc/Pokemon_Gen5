//======================================================================
/**
 * @file	scrcmd_postman.c
 * @brief	スクリプトコマンド：ふしぎなおくりもの配達員
 * @date	2010.02.12
 * @author  tamada GAMEFREAK inc.
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

//#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_postman.h"
#include "field_mystery_gift.h"

#include "msg/script/msg_postman.h"
#include "savedata/mystery_data.h"
#include "pm_define.h"  //TEMOTI_POKEMAX
#include "poke_tool/poke_tool.h"  //POKEMON_PARAM
#include "savedata/myitem_savedata.h"  //MYITEM_
#include "net_app/mystery.h"  //MYSTERY_PokemonCreate
#include "print/wordset.h"   //WORDSET_

//======================================================================
//======================================================================
static BOOL PFuncCheckPokemon( SCRCMD_WORK * work, GAMEDATA * gamedata, GIFT_PACK_DATA * gpd );
static void PFuncAddPokemon( SCRCMD_WORK * work, GAMEDATA * gamedata, GIFT_PACK_DATA * gpd );
static u16  PFuncSetSuccessPokemonWords( WORDSET * wordset, GIFT_PACK_DATA * gpd, SCRCMD_WORK * work );  //msg_postman_06,
static u16  PFuncSetFailurePokemonWords( WORDSET * wordset, GIFT_PACK_DATA * gpd, SCRCMD_WORK * work );  //msg_postman_07,

static BOOL PFuncCheckItem( SCRCMD_WORK * work, GAMEDATA * gamedata, GIFT_PACK_DATA * gpd );
static void PFuncAddItem( SCRCMD_WORK * work, GAMEDATA * gamedata, GIFT_PACK_DATA * gpd );
static u16  PFuncSetSuccessItemWords( WORDSET * wordset, GIFT_PACK_DATA * gpd, SCRCMD_WORK * work ); //msg_postman_08,
static u16  PFuncSetFailureItemWords( WORDSET * wordset, GIFT_PACK_DATA * gpd, SCRCMD_WORK * work ); //msg_postman_09,

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  BOOL (*add_check_func)( SCRCMD_WORK * work, GAMEDATA * gamedata, GIFT_PACK_DATA * gpd );
  void (*add_func)( SCRCMD_WORK * work, GAMEDATA * gamedata, GIFT_PACK_DATA * gpd );
  u16 (*set_success_words)( WORDSET * wordset, GIFT_PACK_DATA * gpd, SCRCMD_WORK * work );
  u16 (*get_failure_words)( WORDSET * wordset, GIFT_PACK_DATA * gpd, SCRCMD_WORK * work );
}POSTMAN_FUNC_TABLE;

static const POSTMAN_FUNC_TABLE PostmanFuncTable[] = {
  { //MYSTERYGIFT_TYPE_NONE		
    NULL,
    NULL,
    NULL,
    NULL,
  },
  { //MYSTERYGIFT_TYPE_POKEMON	
    PFuncCheckPokemon,
    PFuncAddPokemon,
    PFuncSetSuccessPokemonWords,  //msg_postman_06,
    PFuncSetFailurePokemonWords,  //msg_postman_07,
  },
  { //MYSTERYGIFT_TYPE_ITEM		
    PFuncCheckItem,
    PFuncAddItem,
    PFuncSetSuccessItemWords, //msg_postman_08,
    PFuncSetFailureItemWords, //msg_postman_09,
  },
};

//--------------------------------------------------------------
/**
 * @brief 
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPostmanCommand( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK * work = wk;
  u16 req = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( wk );

  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET *wordset    = SCRIPT_GetWordSet( sc );

  MYSTERY_DATA * fd = SaveData_GetMysteryData( GAMEDATA_GetSaveControlWork( gamedata ) );
  int index;
  GIFT_PACK_DATA * gpd = FIELD_MYSTERYDATA_GetGiftData( fd, &index );
  u8 gift_type = FIELD_MYSTERYDATA_GetGiftType( fd );

  switch ( req )
  {
  case SCR_POSTMAN_REQ_EXISTS:
    *ret_wk =  ( gift_type != MYSTERYGIFT_TYPE_NONE );
    break;
  case SCR_POSTMAN_REQ_ENABLE:
    {
      BOOL result;
      result = PostmanFuncTable[gift_type].add_check_func( work, gamedata, gpd );
      *ret_wk = result;
    }
    break;
  case SCR_POSTMAN_REQ_OK_MSG:
    *ret_wk = msg_postman_06;
    break;
  case SCR_POSTMAN_REQ_NG_MSG:
    *ret_wk = msg_postman_07;
    break;
  case SCR_POSTMAN_REQ_RECEIVE:
    *ret_wk = 0;
    break;
  default:
    GF_ASSERT( 0 );
  }

  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static POKEMON_PARAM * createPokemon( SCRCMD_WORK * work, GIFT_PACK_DATA * gpd )
{
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  POKEMON_PARAM * pp;
  pp = MYSTERY_PokemonCreate( &(gpd->data.pokemon), heapID, gamedata );
  return pp;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL PFuncCheckPokemon( SCRCMD_WORK * work, GAMEDATA * gamedata, GIFT_PACK_DATA * gpd )
{
  POKEPARTY*   party = GAMEDATA_GetMyPokemon( gamedata );
  return ( PokeParty_GetPokeCount( party ) < TEMOTI_POKEMAX );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void PFuncAddPokemon( SCRCMD_WORK * work, GAMEDATA * gamedata, GIFT_PACK_DATA * gpd )
{
  POKEPARTY*   party = GAMEDATA_GetMyPokemon( gamedata );
  POKEMON_PARAM * pp = createPokemon( work, gpd );

  if ( PokeParty_GetPokeCount( party ) < TEMOTI_POKEMAX || pp == NULL )
  {
    return;
  }

  PokeParty_Add( party, pp );
   //一応図鑑登録
  ZUKANSAVE_SetPokeGet( GAMEDATA_GetZukanSave( gamedata ), pp );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static u16  PFuncSetSuccessPokemonWords( WORDSET * wordset, GIFT_PACK_DATA * gpd, SCRCMD_WORK * work )
{
  MYSTATUS * mystatus = GAMEDATA_GetMyStatus( SCRCMD_WORK_GetGameData( work ) );
  POKEMON_PARAM * pp = createPokemon( work, gpd );

  WORDSET_RegisterPlayerName( wordset, 0, mystatus );
  if ( pp == NULL )
  {
    GF_ASSERT( 0 );
  } else {
    WORDSET_RegisterPokeMonsName( wordset, 1, pp );
  }
  return msg_postman_06;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static u16  PFuncSetFailurePokemonWords( WORDSET * wordset, GIFT_PACK_DATA * gpd, SCRCMD_WORK * work )
{
  return msg_postman_07;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL PFuncCheckItem( SCRCMD_WORK * work, GAMEDATA * gamedata, GIFT_PACK_DATA * gpd )
{
  u16 item_no;
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gamedata );

  item_no = gpd->data.item.itemNo;

  return MYITEM_AddCheck( myitem, item_no, 1, heapID );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void PFuncAddItem( SCRCMD_WORK * work, GAMEDATA * gamedata, GIFT_PACK_DATA * gpd )
{
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gamedata );

  u16 item_no = gpd->data.item.itemNo;
  BOOL result;

  result = MYITEM_AddItem( myitem, item_no, 1, heapID );
  GF_ASSERT( result );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static u16  PFuncSetSuccessItemWords( WORDSET * wordset, GIFT_PACK_DATA * gpd, SCRCMD_WORK * work )
{
  MYSTATUS * mystatus = GAMEDATA_GetMyStatus( SCRCMD_WORK_GetGameData( work ) );
  u16 item_no = gpd->data.item.itemNo;
  WORDSET_RegisterPlayerName( wordset, 0, mystatus );
  WORDSET_RegisterItemName( wordset, 1, item_no );
  return msg_postman_08;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static u16  PFuncSetFailureItemWords( WORDSET * wordset, GIFT_PACK_DATA * gpd, SCRCMD_WORK * work )
{
  u16 item_no = gpd->data.item.itemNo;
  WORDSET_RegisterItemName( wordset, 0, item_no );

  return msg_postman_09;
}


