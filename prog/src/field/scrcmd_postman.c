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

#include "fldmmdl.h"
#include "fieldmap.h" //FIELDMAP_GetMMdlSys
#include "field/eventdata_system.h"
#include "field/eventdata_sxy.h"

//======================================================================
//======================================================================

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  BOOL (*add_check_func)( SCRCMD_WORK * work, GAMEDATA * gamedata, GIFT_PACK_DATA * gpd );
  void (*add_func)( SCRCMD_WORK * work, GAMEDATA * gamedata, GIFT_PACK_DATA * gpd );
  u16 (*set_success_words)( WORDSET * wordset, GIFT_PACK_DATA * gpd, SCRCMD_WORK * work );
  u16 (*set_failure_words)( WORDSET * wordset, GIFT_PACK_DATA * gpd, SCRCMD_WORK * work );
}POSTMAN_FUNC_TABLE;

//--------------------------------------------------------------
//--------------------------------------------------------------
static MMDL * searchPOSTMANOBJ( FIELDMAP_WORK * fieldWork );
static int searchEventDataPostmanObj( GAMEDATA * gamedata );

static const POSTMAN_FUNC_TABLE PostmanFuncTable[ MYSTERYGIFT_TYPE_MAX ];


//======================================================================
//
//    スクリプトコマンド実装
//
//======================================================================
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
    if ( gift_type != MYSTERYGIFT_TYPE_NONE )
    { 
      *ret_wk = TRUE;
    }
    else
    {
      *ret_wk = FALSE;
    }
    break;
  case SCR_POSTMAN_REQ_ENABLE:
    {
      BOOL result;
      result = PostmanFuncTable[gift_type].add_check_func( work, gamedata, gpd );
      *ret_wk = result;
    }
    break;
  case SCR_POSTMAN_REQ_OK_MSG:
    {
      u16 msg_id;
      msg_id = PostmanFuncTable[gift_type].set_success_words( wordset, gpd, work );
      *ret_wk = msg_id;
    }
    break;
  case SCR_POSTMAN_REQ_NG_MSG:
    {
      u16 msg_id;
      msg_id = PostmanFuncTable[gift_type].set_failure_words( wordset, gpd, work );
      *ret_wk = msg_id;
    }
    break;
  case SCR_POSTMAN_REQ_RECEIVE:
    //手に入れる処理
    PostmanFuncTable[gift_type].add_func( work, gamedata, gpd );
    FIELD_MYSTERYDATA_SetReceived( fd, index );
    *ret_wk = 0;
    break;
  case SCR_POSTMAN_REQ_OBJID:
    {
      FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( SCRCMD_WORK_GetGameSysWork( work ) );
      MMDL * mmdl = searchPOSTMANOBJ( fieldmap );
      if ( mmdl != NULL )
      {
        *ret_wk = MMDL_GetOBJID( mmdl );
      }
      else
      {
        int obj_id = searchEventDataPostmanObj( gamedata );
        if ( obj_id >= 0 )
        {
          *ret_wk = obj_id;
        }
      }
    }
    break;
  case SCR_POSTMAN_REQ_OBJSTAT:
    {
      FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( SCRCMD_WORK_GetGameSysWork( work ) );
      MMDL * mmdl = searchPOSTMANOBJ( fieldmap );
      if ( mmdl == NULL )
      {
        *ret_wk = SCR_POSTMAN_OBJ_NONE;
      }
      else 
      {
        *ret_wk = SCR_POSTMAN_OBJ_EXISTS;
      }
    }
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
static MMDL * searchPOSTMANOBJ( FIELDMAP_WORK * fieldWork )
{
  u32 no = 0;
	MMDL *mmdl;
  MMDLSYS * mmdlsys = FIELDMAP_GetMMdlSys( fieldWork );
	while( MMDLSYS_SearchUseMMdl(mmdlsys,&mmdl,&no) == TRUE ){
		if( MMDL_GetOBJCode(mmdl) == DELIVERY ){
			return( mmdl );
		}
	}
  return NULL;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static int searchEventDataPostmanObj( GAMEDATA * gamedata )
{
  EVENTDATA_SYSTEM * evdata = GAMEDATA_GetEventData( gamedata );
  const MMDL_HEADER * tbl = EVENTDATA_GetNpcData( evdata );
  int max = EVENTDATA_GetNpcCount( evdata );
  int i;

  if ( tbl == NULL || max == 0 ) return -1;
  
  for ( i = 0; i < max; i ++ )
  {
    if ( tbl[i].obj_code == DELIVERY ) return tbl[i].id;
  }
  return -1;
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
  pp = MYSTERY_CreatePokemon( gpd, heapID, gamedata );
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

  if ( PokeParty_GetPokeCount( party ) >= TEMOTI_POKEMAX || pp == NULL )
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

  item_no = MYSTERY_CreateItem(gpd);

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

//--------------------------------------------------------------
//--------------------------------------------------------------
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
static const POSTMAN_FUNC_TABLE PostmanFuncTable[ MYSTERYGIFT_TYPE_MAX ] = {
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

