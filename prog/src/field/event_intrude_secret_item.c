//==============================================================================
/**
 * @file  event_intrude_secret_item.c
 * @brief
 * @date  2010.02.16
 * @author  tamada GAMEFREAK inc.
 */
//==============================================================================

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "fieldmap.h"

#include "field/intrude_secret_item.h"
#include "savedata/intrude_save_field.h"
#include "savedata/intrude_save.h"

#include "system/main.h"
#include "script_local.h"

#include "../../../resource/fldmapdata/script/item_get_scr_def.h"

#include "event_intrude_secret_item.h"
//==============================================================================
//==============================================================================

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  INTRUDE_SECRET_ITEM_SAVE intrude_item;
}EVENT_INTRUDE_SECRET_ITEM_WORK, EISIW;

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT_RESULT intrudeSecretItemEvent( GMEVENT * event, int *seq, void * work )
{
  EISIW * ewk = (EISIW *)work;
  SCRIPT_WORK * sc;

  sc = SCRIPT_ChangeScript( event, SCRID_ITEM_EVENT_INTRUDE_SECRET, NULL, HEAPID_FIELDMAP );
  SCRIPT_SetScriptWorkParam( sc, ewk->intrude_item.item, 0, 0, 0 );
  {
    HEAPID heapID = HEAPID_FIELDMAP;
    WORDSET * wordset = SCRIPT_GetWordSet( sc );
    STRBUF * strbuf = GFL_STR_CreateBuffer( PERSON_NAME_SIZE + EOM_SIZE, heapID );
    GFL_STR_SetStringCodeOrderLength( strbuf, ewk->intrude_item.name, PERSON_NAME_SIZE + EOM_SIZE );
    WORDSET_RegisterWord( wordset, 3, strbuf, PM_FEMALE, TRUE, PM_LANG );
    GFL_STR_DeleteBuffer( strbuf );
  }
  return GMEVENT_RES_CONTINUE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * EVENT_IntrudeSecretItem( GAMESYS_WORK * gsys, HEAPID heapID, int index )
{
  GMEVENT * event;
  EISIW * ewk;
  INTRUDE_SAVE_WORK * intsave;
  event = GMEVENT_Create( gsys, NULL, intrudeSecretItemEvent, sizeof(EISIW) );
  ewk = GMEVENT_GetEventWork( event );
  intsave = SaveData_GetIntrude( GAMEDATA_GetSaveControlWork( GAMESYSTEM_GetGameData( gsys ) ) );
  ISC_SAVE_GetItem( intsave, index, &ewk->intrude_item );

  return event;
}
