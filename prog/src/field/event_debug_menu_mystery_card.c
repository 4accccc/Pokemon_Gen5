#ifdef PM_DEBUG 
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  デバッグメニュー『ふしぎデータ作成』
 * @file   event_debug_menu_fskill.c
 * @author obata
 * @date   2010.05.06
 *
 * ※event_debug_menu.c より移動
 */
///////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "system/main.h"
#include "field/fieldmap_proc.h"
#include "field/field_msgbg.h"
#include "debug/debug_str_conv.h"
#include "event_debug_local.h"
#include "fieldmap.h"

#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"

#include "item/itemsym.h"
#include "debug/debug_mystery_card.h"
#include "event_debug_menu_mystery_card.h"



//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuMysteryCardListEvent(GMEVENT *event, int *seq, void *wk );
static BOOL debugMenuCallProc_MakeMysteryCardPoke( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MakeMysteryCardItem( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MakeMysteryCardGPower( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MakeMysteryCardGLiberty( DEBUG_MENU_EVENT_WORK *p_wk );
static BOOL debugMenuCallProc_MakeMysteryCardEgg( DEBUG_MENU_EVENT_WORK *p_wk );
//#define MYSTERY_DLDATA_DEBUG_PRINT
static const FLDMENUFUNC_LIST DATA_SubMysteryCardMakeList[] =
{
  { DEBUG_FIELD_MYSTERY_01, debugMenuCallProc_MakeMysteryCardPoke },              //ポケモン作成
  { DEBUG_FIELD_MYSTERY_02, debugMenuCallProc_MakeMysteryCardItem },               //道具作成
  { DEBUG_FIELD_MYSTERY_03, debugMenuCallProc_MakeMysteryCardGPower },              //Gパワー作成
  { DEBUG_FIELD_MYSTERY_04, debugMenuCallProc_MakeMysteryCardGLiberty },              //リバティ作成
  { DEBUG_FIELD_MYSTERY_05, debugMenuCallProc_MakeMysteryCardEgg },              //タマゴ作成
};

static const DEBUG_MENU_INITIALIZER DebugSubMysteryCardMakeData = {
  NARC_debug_message_d_field_dat,
  NELEMS(DATA_SubMysteryCardMakeList),
  DATA_SubMysteryCardMakeList,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};


//--------------------------------------------------------------
/**
 * イベント：ふしぎデータ作成
 * @param gsys
 * @param wk DEBUG_MENU_EVENT_WORK*
 * @return GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_DebugMenu_MakeMysteryCard( GAMESYS_WORK *gsys, void* wk )
{
  DEBUG_MENU_EVENT_WORK* debug_work = wk;
  DEBUG_MENU_EVENT_WORK   *work;
  GMEVENT *event;

  event = GMEVENT_Create( gsys, debug_work->gmEvent,
    debugMenuMysteryCardListEvent, sizeof(DEBUG_MENU_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_MENU_EVENT_WORK) );

  *work  = *debug_work;
  work->gmEvent = event;
  work->call_proc = NULL;

  work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugSubMysteryCardMakeData );

  return event;
}


//--------------------------------------------------------------
/**
 * イベント：フィールド技リスト
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuMysteryCardListEvent(GMEVENT *event, int *seq, void *wk )
{
  DEBUG_MENU_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }else if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        (*seq)++;
      }else if( ret != FLDMENUFUNC_CANCEL ){  //決定
        work->call_proc = (DEBUG_MENU_CALLPROC)ret;
        (*seq)++;
      }
    }
    break;

  case 2:
    {
      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( work->call_proc != NULL ){
        if( work->call_proc(work) == TRUE ){
          return( GMEVENT_RES_CONTINUE );
        }
      }

      return( GMEVENT_RES_FINISH );
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}


//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL debugMenuCallProc_MakeMysteryCardPoke( DEBUG_MENU_EVENT_WORK *p_wk )
{
  DOWNLOAD_GIFT_DATA  dl_data;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  MYSTERY_DATA *p_mystery_sv  = MYSTERY_DATA_Load( pSave, MYSTERYDATA_LOADTYPE_NORMAL,GFL_HEAPID_APP );
  GIFT_PACK_DATA  data;
  int i;

  GFL_STD_MemClear( &dl_data, sizeof(DOWNLOAD_GIFT_DATA) );

  for( i = 1; i < 2048; i++ )
  {
    if( !MYSTERYDATA_IsEventRecvFlag(p_mystery_sv, i) )
    {
      DEBUG_MYSTERY_SetGiftPokeData( &data );
      DEBUG_MYSTERY_SetGiftCommonData( &data, i, FALSE );
      MYSTERYDATA_SetCardData( p_mystery_sv, &data );

      DEBUG_MYSTERY_SetDownLoadData( &dl_data, 0xFFFFFFFF, LANG_JAPAN );
      dl_data.data  = data;
#ifdef MYSTERY_DLDATA_DEBUG_PRINT
      {
        int j;
        const u8 * cp_data  = (const u8*)&dl_data;
        for( j = 0; j < sizeof(DOWNLOAD_GIFT_DATA); j++ )
        {
          OS_TFPrintf( 3, "0x%x ", cp_data[j] );
          if( j % 0x10 == 0xF )
          {
            OS_TFPrintf( 3, "\n" );
          }
        }
      }
#endif
      OS_TPrintf( "ふしぎなカードをセットしました イベントID=[%d]\n", i );
      break;
    }
  }

  MYSTERY_DATA_UnLoad( p_mystery_sv );

  return FALSE;
}


//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL debugMenuCallProc_MakeMysteryCardItem( DEBUG_MENU_EVENT_WORK *p_wk )
{
  DOWNLOAD_GIFT_DATA  dl_data;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  MYSTERY_DATA *p_mystery_sv  = MYSTERY_DATA_Load( pSave, MYSTERYDATA_LOADTYPE_NORMAL,GFL_HEAPID_APP );
  GIFT_PACK_DATA  data;
  int i;

  GFL_STD_MemClear( &dl_data, sizeof(DOWNLOAD_GIFT_DATA) );
  for( i = 1; i < 2048; i++ )
  {
    if( !MYSTERYDATA_IsEventRecvFlag(p_mystery_sv, i) )
    {
      DEBUG_MYSTERY_SetGiftItemData( &data, ITEM_MONSUTAABOORU );
      DEBUG_MYSTERY_SetGiftCommonData( &data, i, FALSE );
      MYSTERYDATA_SetCardData( p_mystery_sv, &data );

      DEBUG_MYSTERY_SetDownLoadData( &dl_data, 0xFFFFFFFF, LANG_JAPAN );
      dl_data.data  = data;
#ifdef MYSTERY_DLDATA_DEBUG_PRINT
      {
        int j;
        const u8 * cp_data  = (const u8*)&dl_data;
        for( j = 0; j < sizeof(DOWNLOAD_GIFT_DATA); j++ )
        {
          OS_TFPrintf( 3, "0x%x ", cp_data[j] );
          if( j % 0x10 == 0xF )
          {
            OS_TFPrintf( 3, "\n" );
          }
        }
      }
#endif
      OS_TPrintf( "ふしぎなカードをセットしました イベントID=[%d]\n", i );
      break;
    }
  }
  MYSTERY_DATA_UnLoad( p_mystery_sv );

  return FALSE;
}



//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL debugMenuCallProc_MakeMysteryCardGPower( DEBUG_MENU_EVENT_WORK *p_wk )
{
  DOWNLOAD_GIFT_DATA  dl_data;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  MYSTERY_DATA *p_mystery_sv  = MYSTERY_DATA_Load( pSave, MYSTERYDATA_LOADTYPE_NORMAL, GFL_HEAPID_APP );
  GIFT_PACK_DATA  data;
  int i;

  GFL_STD_MemClear( &dl_data, sizeof(DOWNLOAD_GIFT_DATA) );
  for( i = 1; i < 2048; i++ )
  {
    if( !MYSTERYDATA_IsEventRecvFlag(p_mystery_sv, i) )
    {
      DEBUG_MYSTERY_SetGiftGPowerData( &data );
      DEBUG_MYSTERY_SetGiftCommonData( &data, i, FALSE );
      MYSTERYDATA_SetCardData( p_mystery_sv, &data );

      DEBUG_MYSTERY_SetDownLoadData( &dl_data, 0xFFFFFFFF, LANG_JAPAN );
      dl_data.data  = data;
#ifdef MYSTERY_DLDATA_DEBUG_PRINT
      {
        int j;
        const u8 * cp_data  = (const u8*)&dl_data;
        for( j = 0; j < sizeof(DOWNLOAD_GIFT_DATA); j++ )
        {
          OS_TFPrintf( 3, "0x%x ", cp_data[j] );
          if( j % 0x10 == 0xF )
          {
            OS_TFPrintf( 3, "\n" );
          }
        }
      }
#endif
      OS_TPrintf( "ふしぎなカードをセットしました イベントID=[%d]\n", i );
      break;
    }
  }
  MYSTERY_DATA_UnLoad( p_mystery_sv );

  return FALSE;
}


//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL debugMenuCallProc_MakeMysteryCardGLiberty( DEBUG_MENU_EVENT_WORK *p_wk )
{
  DOWNLOAD_GIFT_DATA  dl_data;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  MYSTERY_DATA *p_mystery_sv  = MYSTERY_DATA_Load( pSave, MYSTERYDATA_LOADTYPE_NORMAL, GFL_HEAPID_APP );
  GIFT_PACK_DATA  data;
  int i;

  GFL_STD_MemClear( &dl_data, sizeof(DOWNLOAD_GIFT_DATA) );
  DEBUG_MYSTERY_SetGiftItemData( &data, ITEM_RIBATHITIKETTO );
  DEBUG_MYSTERY_SetGiftCommonData( &data, MYSTERY_DATA_EVENT_LIBERTY, FALSE );
  MYSTERYDATA_SetCardData( p_mystery_sv, &data );

  DEBUG_MYSTERY_SetDownLoadData( &dl_data, 0xFFFFFFFF, LANG_JAPAN );
  dl_data.data  = data;


  MYSTERY_DATA_UnLoad( p_mystery_sv );

  return FALSE;
}


//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL debugMenuCallProc_MakeMysteryCardEgg( DEBUG_MENU_EVENT_WORK *p_wk )
{
  DOWNLOAD_GIFT_DATA  dl_data;
  SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(p_wk->gdata);
  MYSTERY_DATA *p_mystery_sv  = MYSTERY_DATA_Load( pSave, MYSTERYDATA_LOADTYPE_NORMAL,GFL_HEAPID_APP );
  GIFT_PACK_DATA  data;
  int i;

  GFL_STD_MemClear( &dl_data, sizeof(DOWNLOAD_GIFT_DATA) );

  for( i = 1; i < 2048; i++ )
  {
    if( !MYSTERYDATA_IsEventRecvFlag(p_mystery_sv, i) )
    {
      DEBUG_MYSTERY_SetGiftPokeData( &data );
      data.data.pokemon.egg = 1;
      DEBUG_MYSTERY_SetGiftCommonData( &data, i, FALSE );
      MYSTERYDATA_SetCardData( p_mystery_sv, &data );

      DEBUG_MYSTERY_SetDownLoadData( &dl_data, 0xFFFFFFFF, LANG_JAPAN );
      dl_data.data  = data;
#ifdef MYSTERY_DLDATA_DEBUG_PRINT
      {
        int j;
        const u8 * cp_data  = (const u8*)&dl_data;
        for( j = 0; j < sizeof(DOWNLOAD_GIFT_DATA); j++ )
        {
          OS_TFPrintf( 3, "0x%x ", cp_data[j] );
          if( j % 0x10 == 0xF )
          {
            OS_TFPrintf( 3, "\n" );
          }
        }
      }
#endif
      OS_TPrintf( "ふしぎなカードをセットしました イベントID=[%d]\n", i );
      break;
    }
  }

  MYSTERY_DATA_UnLoad( p_mystery_sv );

  return FALSE;
}


#endif // PM_DEBUG 
