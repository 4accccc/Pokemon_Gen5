//======================================================================
/**
 *
 * @file  event_debug_menu.c
 * @brief フィールドデバッグメニューの制御イベント
 * @author  kagaya
 * @date  2008.11.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"

#include "field/field_msgbg.h"

#include "message.naix"
#include "msg/msg_d_field.h"

#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "field/eventdata_sxy.h"

#include "event_debug_menu.h"
#include "event_mapchange.h"
#include "event_gamestart.h"

#include "net_app/irc_match/ircbattlemenu.h"
#include "event_ircbattle.h"
#include "event_wificlub.h"
#include "event_gtsnego.h"
#include "field_subscreen.h"
#include "sound/pm_sndsys.h"

#include "font/font.naix"

#include  "field/weather_no.h"
#include  "weather.h"
#include  "msg/msg_d_tomoya.h"

#include "field_debug.h"

#include "field_event_check.h"
#include "event_debug_item.h" //EVENT_DebugItemMake
#include "event_debug_numinput.h"
#include "savedata/box_savedata.h"  //デバッグアイテム生成用
#include  "item/itemsym.h"  //ITEM_DATA_MAX
#include  "item/item.h"  //ITEM_CheckEnable
#include "app/townmap.h"
#include "net_app/worldtrade.h"
#include "../ui/debug/ui_template.h"
#include "savedata/shortcut.h"
#include "event_debug_beacon.h"
#include "app/waza_oshie.h"

#include "field_buildmodel.h"

#include "field_sound.h"

#include "script.h" //SCRIPT_ChangeScript
#include "../../../resource/fldmapdata/script/debug_scr_def.h"  //SCRID_DEBUG_COMMON
#include "../../../resource/fldmapdata/script/hiden_def.h"  //SCRID_HIDEN_DIVING

#include "eventwork.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"

#include "event_wifibattlematch.h"

#include "field/field_comm/intrude_field.h" //PALACE_MAP_LEN

//CGEAR PICTURE
#include "c_gear.naix"
#include "./c_gear/c_gear.h"
#include "./c_gear/event_cgearget.h"
#include "savedata/save_tbl.h"
#include "savedata/c_gear_picture.h"

#include "event_debug_local.h"

#include "event_debug_season_display.h" // for DEBUG_EVENT_FLDMENU_SeasonDispSelect
#include "event_debug_demo3d.h" // for DEBUG_EVENT_FLDMENU_Demo3DSelect
#include "event_debug_menu_make_egg.h"  // for DEBUG_EVENT_FLDMENU_MakeEgg

#include "debug/debug_str_conv.h" // for DEB_STR_CONV_SJIStoStrcode

#include "event_debug_wifimatch.h"
#include "event_battlerecorder.h"
#include "event_debug_mvpoke.h"
#include "field_bbd_color.h"

#include "gamesystem/pm_weather.h"

#include "debug/debug_mystery_card.h"

#include "bsubway_scr.h"
#include "event_wifi_bsubway.h"
#include "event_bsubway.h"
#include "scrcmd_bsubway.h"

#include "savedata/battle_box_save.h"
#include "event_geonet.h"
#include "app/name_input.h"
#include "waza_tool\wazano_def.h"

#include "savedata/symbol_save.h"
#include "savedata/symbol_save_notwifi.h"
#include "savedata/symbol_save_field.h"

#include "event_debug_menu_symbol.h"


static BOOL debugMenuCallProc_SymbolPokeCreate( DEBUG_MENU_EVENT_WORK * p_wk );
//static BOOL debugMenuCallProc_SymbolPokeList( DEBUG_MENU_EVENT_WORK *wk );
static BOOL debugMenuCallProc_SymbolPokeKeepLargeFull( DEBUG_MENU_EVENT_WORK * wk );
static BOOL debugMenuCallProc_SymbolPokeKeepSmallFull( DEBUG_MENU_EVENT_WORK * wk );
static BOOL debugMenuCallProc_SymbolPokeFreeLargeFull( DEBUG_MENU_EVENT_WORK * wk );
static BOOL debugMenuCallProc_SymbolPokeFreeSmallFull( DEBUG_MENU_EVENT_WORK * wk );
static BOOL debugMenuCallProc_SymbolPokeCountup( DEBUG_MENU_EVENT_WORK * wk );


//======================================================================
//======================================================================
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_Default =
{
  1,    //リスト項目数
  9,    //表示最大項目数
  0,    //ラベル表示Ｘ座標
  13,   //項目表示Ｘ座標
  0,    //カーソル表示Ｘ座標
  0,    //表示Ｙ座標
  1,    //表示文字色
  15,   //表示背景色
  2,    //表示文字影色
  0,    //文字間隔Ｘ
  1,    //文字間隔Ｙ
  FLDMENUFUNC_SKIP_LRKEY, //ページスキップタイプ
  12,   //文字サイズX(ドット
  12,   //文字サイズY(ドット
  0,    //表示座標X キャラ単位
  0,    //表示座標Y キャラ単位
  0,    //表示サイズX キャラ単位
  0,    //表示サイズY キャラ単位
};
//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuSymbolPokeListEvent(GMEVENT *event, int *seq, void *wk );

static const FLDMENUFUNC_LIST DATA_SubSymbolPokeList[] =
{
  { DEBUG_FIELD_SYMBOL_11, debugMenuCallProc_SymbolPokeCreate },
  { DEBUG_FIELD_SYMBOL_05, debugMenuCallProc_SymbolPokeKeepLargeFull },
  { DEBUG_FIELD_SYMBOL_06, debugMenuCallProc_SymbolPokeKeepSmallFull },
  { DEBUG_FIELD_SYMBOL_07, debugMenuCallProc_SymbolPokeFreeLargeFull },
  { DEBUG_FIELD_SYMBOL_08, debugMenuCallProc_SymbolPokeFreeSmallFull },
  { DEBUG_FIELD_SYMBOL_09, debugMenuCallProc_SymbolPokeCountup },
};

static const DEBUG_MENU_INITIALIZER DebugSubSymbolPokeListSelectData = {
  NARC_message_d_field_dat,
  NELEMS(DATA_SubSymbolPokeList),
  DATA_SubSymbolPokeList,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};

//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * EVENT_DEBUG_SymbolPokeList( DEBUG_MENU_EVENT_WORK * wk )
{
  DEBUG_MENU_EVENT_WORK   *new_work;
  GMEVENT * new_event;
  new_event = GMEVENT_Create(
      wk->gmSys, NULL, debugMenuSymbolPokeListEvent, sizeof(DEBUG_MENU_EVENT_WORK) );
  new_work = GMEVENT_GetEventWork( new_event );
  GFL_STD_MemClear( new_work, sizeof(DEBUG_MENU_EVENT_WORK) );

  *new_work  = *wk;
  new_work->call_proc = NULL;

  new_work->menuFunc = DEBUGFLDMENU_Init(
      new_work->fieldWork, new_work->heapID,  &DebugSubSymbolPokeListSelectData );

  return new_event;
}



//--------------------------------------------------------------
/**
 * イベント：シンボルポケモン作成リスト
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuSymbolPokeListEvent(GMEVENT *event, int *seq, void *wk )
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
        work->call_proc = NULL;
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

#include "field/tpoke_data.h"
//--------------------------------------------------------------
/**
 * @brief シンボルポケモンを満タンにする
 */
//--------------------------------------------------------------
static void addSymbolPokemons( GAMESYS_WORK * gsys, SYMBOL_ZONE_TYPE zone_type )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK * svctrl = GAMEDATA_GetSaveControlWork( gamedata );
  SYMBOL_SAVE_WORK * symbol_save = SymbolSave_GetSymbolData( svctrl );
  TPOKE_DATA * tpdata = TPOKE_DATA_Create( HEAPID_FIELDMAP );
  BOOL need_large;

  if ( zone_type == SYMBOL_ZONE_TYPE_KEEP_LARGE || zone_type == SYMBOL_ZONE_TYPE_FREE_LARGE ) {
    need_large = TRUE;
  } else {
    need_large = FALSE;
  }
  
  while ( SymbolSave_CheckSpace( symbol_save, zone_type ) != SYMBOL_SPACE_NONE )
  {
    u16 monsno;
    u8 sex;
    do {
      monsno = GFUser_GetPublicRand0( 492 ) + 1;
      sex = POKETOOL_GetSex( monsno, 0, 0 );
    } while ( need_large != TPOKE_DATA_IsSizeBig( gamedata, tpdata, monsno, sex, 0 ) );
    SymbolSave_Field_Set( symbol_save, monsno, 0, sex, 0, zone_type );
  }
  TPOKE_DATA_Delete( tpdata );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void countSymbolPoke( SYMBOL_SAVE_WORK * symbol_save, SYMBOL_ZONE_TYPE zone_type )
{
  u32 start, end, max, num;

  start = SYMBOLZONE_GetStartNo( zone_type );
  end = SYMBOLZONE_GetEndNo( zone_type );
  max = end - start;
  num = SymbolSave_CheckSpace( symbol_save, zone_type );
  if ( num == SYMBOL_SPACE_NONE ) num = end;
  switch ( zone_type ) {
  case SYMBOL_ZONE_TYPE_KEEP_LARGE: OS_PutString("KEEP LARGE"); break;
  case SYMBOL_ZONE_TYPE_KEEP_SMALL: OS_PutString("KEEP SMALL"); break;
  case SYMBOL_ZONE_TYPE_FREE_LARGE: OS_PutString("FREE LARGE"); break;
  case SYMBOL_ZONE_TYPE_FREE_SMALL: OS_PutString("FREE SMALL"); break;
  }
  OS_TPrintf(": %3d/%3d\n", num - start, max );
}

//--------------------------------------------------------------
/**
 * @brief シンボルポケモンの数え上げ
 */
//--------------------------------------------------------------
static GMEVENT_RESULT symbolPokeCountupEvent( GMEVENT * event, int *seq, void *work )
{
  GAMESYS_WORK * gsys = ((DEBUG_MENU_EVENT_WORK *)work)->gmSys;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK * svctrl = GAMEDATA_GetSaveControlWork( gamedata );
  SYMBOL_SAVE_WORK * symbol_save = SymbolSave_GetSymbolData( svctrl );
  u32 num;
  int i;
  switch (*seq)
  {
  case 0:
    for ( i = 0; i < SYMBOL_POKE_MAX; i ++)
    {
      const SYMBOL_POKEMON * sympoke = SymbolSave_GetSymbolPokemon( symbol_save, i );
      OS_Printf("%03d: monsno=%03d sex(%d) form(%d) waza(%d)\n", i,
          sympoke->monsno, sympoke->sex, sympoke->form_no, sympoke->wazano );
    }
    countSymbolPoke( symbol_save, SYMBOL_ZONE_TYPE_KEEP_LARGE );
    countSymbolPoke( symbol_save, SYMBOL_ZONE_TYPE_KEEP_SMALL );
    countSymbolPoke( symbol_save, SYMBOL_ZONE_TYPE_FREE_LARGE );
    countSymbolPoke( symbol_save, SYMBOL_ZONE_TYPE_FREE_SMALL );

    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL debugMenuCallProc_SymbolPokeKeepLargeFull( DEBUG_MENU_EVENT_WORK * wk )
{
  addSymbolPokemons( wk->gmSys, SYMBOL_ZONE_TYPE_KEEP_LARGE );
  return FALSE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL debugMenuCallProc_SymbolPokeKeepSmallFull( DEBUG_MENU_EVENT_WORK * wk )
{
  addSymbolPokemons( wk->gmSys, SYMBOL_ZONE_TYPE_KEEP_SMALL );
  return FALSE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL debugMenuCallProc_SymbolPokeFreeLargeFull( DEBUG_MENU_EVENT_WORK * wk )
{
  addSymbolPokemons( wk->gmSys, SYMBOL_ZONE_TYPE_FREE_LARGE );
  return FALSE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL debugMenuCallProc_SymbolPokeFreeSmallFull( DEBUG_MENU_EVENT_WORK * wk )
{
  addSymbolPokemons( wk->gmSys, SYMBOL_ZONE_TYPE_FREE_SMALL );
  return FALSE;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL debugMenuCallProc_SymbolPokeCountup( DEBUG_MENU_EVENT_WORK * wk )
{
  GMEVENT * new_event;
  DEBUG_MENU_EVENT_WORK * new_wk;
  new_event = GMEVENT_Create( wk->gmSys, NULL, symbolPokeCountupEvent, sizeof(DEBUG_MENU_EVENT_WORK) );
  new_wk = GMEVENT_GetEventWork( new_event );
  *new_wk = *wk;
  GMEVENT_ChangeEvent( wk->gmEvent, new_event );
  return TRUE;
}


//--------------------------------------------------------------
//シンボルポケモン作成
//--------------------------------------------------------------
#include "debug/debug_wordsearch.h"
typedef struct
{
  u8 seq;
  u32 idx;
  HEAPID heapId;
  STRBUF *strBuf;
  STRBUF *tempBuf;
  STRBUF *compBuf;
  GFL_MSGDATA *msgHandle;

  GFL_SKB *skb;
  GFL_SKB_SETUP setup;
  
}DEBUG_SKB_WORK;
static DEBUG_SKB_WORK* debugMenuInitSkb( const HEAPID heapId , const u32 msgFileId );
static void debugMenuExitSkb( DEBUG_SKB_WORK* skb );
static const BOOL debugMenuUpdateSkb( DEBUG_SKB_WORK *work );

#define DEBUG_MENU_SKB_STR_LEN (64)

static DEBUG_SKB_WORK* debugMenuInitSkb( const HEAPID heapId , const u32 msgFileId )
{
  DEBUG_SKB_WORK* work = GFL_HEAP_AllocClearMemory( heapId , sizeof(DEBUG_SKB_WORK) );
  
  work->seq = 0;
  work->idx = 0;
  work->heapId = heapId;
  work->strBuf = GFL_STR_CreateBuffer(DEBUG_MENU_SKB_STR_LEN,heapId);
  work->tempBuf = GFL_STR_CreateBuffer(DEBUG_MENU_SKB_STR_LEN,heapId);
  work->compBuf = GFL_STR_CreateBuffer(DEBUG_MENU_SKB_STR_LEN,heapId);
  work->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , msgFileId , heapId );

  work->setup.strlen = 64;
  work->setup.strtype = GFL_SKB_STRTYPE_STRBUF;
  work->setup.mode = GFL_SKB_MODE_KATAKANA;
  work->setup.modeChange = TRUE;
  work->setup.cancelKey = 0;
  work->setup.decideKey = 0;
  work->setup.bgID = GFL_BG_FRAME0_S;
  work->setup.bgPalID = 1;
  work->setup.bgPalID_on = 2;

  work->skb = GFL_SKB_Create( (void*)(work->strBuf), &work->setup, work->heapId );
  GFL_BG_SetVisible(GFL_BG_FRAME1_S,VISIBLE_OFF);
  GFL_BG_SetVisible(GFL_BG_FRAME2_S,VISIBLE_OFF);
  GFL_BG_SetVisible(GFL_BG_FRAME3_S,VISIBLE_OFF);
  GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_OFF);
  
  return work;
}

static void debugMenuExitSkb( DEBUG_SKB_WORK* work )
{
  GFL_SKB_Delete(work->skb);

  GFL_BG_SetVisible(GFL_BG_FRAME0_S,VISIBLE_ON);
  GFL_BG_SetVisible(GFL_BG_FRAME1_S,VISIBLE_ON);
  GFL_BG_SetVisible(GFL_BG_FRAME2_S,VISIBLE_ON);
  GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);

  G2S_SetBlendAlpha( GFL_BG_FRAME2_S|GFL_BG_FRAME0_S, GFL_BG_FRAME1_S , 3, 16 );

  GFL_MSG_Delete( work->msgHandle );
  GFL_STR_DeleteBuffer( work->compBuf );
  GFL_STR_DeleteBuffer( work->tempBuf );
  GFL_STR_DeleteBuffer( work->strBuf );
  GFL_HEAP_FreeMemory( work );
}

static const BOOL debugMenuUpdateSkb( DEBUG_SKB_WORK *work )
{
  BOOL retVal = FALSE;
  GflSkbReaction ret = GFL_SKB_Main(work->skb);
  switch( ret )
  {
  case GFL_SKB_REACTION_QUIT:
    retVal = TRUE;
  case GFL_SKB_REACTION_INPUT:
    {
      GFL_STR_ClearBuffer(work->compBuf);
      work->idx = 0;
    }
    break;
  case GFL_SKB_REACTION_PAGE:
    break;
  }
  
  if( GFL_UI_KEY_GetTrg() && (PAD_BUTTON_R|PAD_BUTTON_L) ||
      (retVal == TRUE && work->idx == 0) )
  {
    int idx,dir;
    if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_R)
    {
      dir = 1;
    }
    if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_L)
    {
      dir = -1;
    }
    if( retVal == TRUE && work->idx == 0 )
    {
      dir = 1;
    }

    if(GFL_STR_GetBufferLength(work->compBuf) == 0)
    {
      GFL_SKB_PickStr(work->skb);
      GFL_STR_CopyBuffer(work->compBuf, work->strBuf);
      work->idx = 0;
    }
    idx = work->idx;
    if( DEBUG_WORDSEARCH_sub_search(work->msgHandle, work->tempBuf, work->compBuf, dir, &idx) == 1 )
    {
      GFL_MSG_GetString( work->msgHandle, idx, work->tempBuf );
      GFL_SKB_ReloadStr( work->skb, work->tempBuf );
      work->idx = idx;
    }
  }

  return retVal;
}

static GMEVENT_RESULT debugMenuSymbolpokeCreate( GMEVENT *event, int *seq, void *wk );
typedef struct
{
  HEAPID heapId;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  
  GFL_MSGDATA  *msgHandle;
  GFL_FONT *fontHandle;
  GFL_BMPWIN *bmpWin;
  
  u16 monsNo;
  u16 wazaNo;
  u16 sex;
  u16 place;
  
  DEBUG_SKB_WORK *skbWork;
  
}DEBUG_SYMBOL_POKE_CREATE;

static BOOL debugMenuCallProc_SymbolPokeCreate( DEBUG_MENU_EVENT_WORK * wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_SYMBOL_POKE_CREATE *work;

  GMEVENT_Change( event, debugMenuSymbolpokeCreate, sizeof(DEBUG_SYMBOL_POKE_CREATE) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_SYMBOL_POKE_CREATE) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->fieldWork = fieldWork;
  work->heapId = heapID;
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  work->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_d_field_dat , work->heapId );

  work->bmpWin = GFL_BMPWIN_Create( FLDBG_MFRM_MSG , 1,19,30,4 ,11,GFL_BMP_CHRAREA_GET_B);

  return TRUE;

}

static GMEVENT_RESULT debugMenuSymbolpokeCreate( GMEVENT *event, int *seq, void *wk )
{
  DEBUG_SYMBOL_POKE_CREATE *work = wk;

  switch( *seq )
  {
  case 0:
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(work->bmpWin), 0x0f );
    {
      STRBUF *str = GFL_MSG_CreateString( work->msgHandle , DEBUG_FIELD_SYMBOL_01 );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(work->bmpWin), 0, 0, str, work->fontHandle);
      GFL_STR_DeleteBuffer( str );
    }
    GFL_BMPWIN_MakeTransWindow_VBlank( work->bmpWin );
    work->skbWork = debugMenuInitSkb( work->heapId , NARC_message_monsname_dat );
    *seq += 1;
    break;

  case 1:
    if( debugMenuUpdateSkb( work->skbWork ) == TRUE )
    {
      work->monsNo = work->skbWork->idx;
      debugMenuExitSkb( work->skbWork );
      *seq += 1;
    }
    break;
  
  case 2:
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(work->bmpWin), 0x0f );
    {
      STRBUF *str = GFL_MSG_CreateString( work->msgHandle , DEBUG_FIELD_SYMBOL_02 );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(work->bmpWin), 0, 0, str, work->fontHandle);
      GFL_STR_DeleteBuffer( str );
    }
    GFL_BMPWIN_MakeTransWindow_VBlank( work->bmpWin );
    work->skbWork = debugMenuInitSkb( work->heapId , NARC_message_wazaname_dat );
    *seq += 1;
    break;

  case 3:
    if( debugMenuUpdateSkb( work->skbWork ) == TRUE )
    {
      work->wazaNo = work->skbWork->idx;
      debugMenuExitSkb( work->skbWork );
      *seq += 1;
    }
    break;
  
  case 4:
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(work->bmpWin), 0x0f );
    {
      STRBUF *str = GFL_MSG_CreateString( work->msgHandle , DEBUG_FIELD_SYMBOL_03 );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(work->bmpWin), 0, 0, str, work->fontHandle);
      GFL_STR_DeleteBuffer( str );
    }
    GFL_BMPWIN_MakeTransWindow_VBlank( work->bmpWin );
    *seq += 1;
    break;
  
  case 5:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      work->sex = PTL_SEX_FEMALE;
      *seq += 1;
    }
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
    {
      work->sex = PTL_SEX_UNKNOWN;
      *seq += 1;
    }
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
    {
      work->sex = PTL_SEX_MALE;
      *seq += 1;
    }
    break;
  
  case 6:
    GFL_BMP_Clear(GFL_BMPWIN_GetBmp(work->bmpWin), 0x0f );
    {
      STRBUF *str = GFL_MSG_CreateString( work->msgHandle , DEBUG_FIELD_SYMBOL_04 );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp(work->bmpWin), 0, 0, str, work->fontHandle);
      GFL_STR_DeleteBuffer( str );
    }
    GFL_BMPWIN_MakeTransWindow_VBlank( work->bmpWin );
    *seq += 1;
    break;
  
  case 7:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
    {
      work->place = SYMBOL_ZONE_TYPE_KEEP_LARGE;
      *seq += 1;
    }
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
    {
      work->place = SYMBOL_ZONE_TYPE_KEEP_SMALL;
      *seq += 1;
    }
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      work->place = SYMBOL_ZONE_TYPE_FREE_LARGE;
      *seq += 1;
    }
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      work->place = SYMBOL_ZONE_TYPE_FREE_SMALL;
      *seq += 1;
    }
    break;

  case 8:
    {
      SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(work->gmSys));
      SYMBOL_SAVE_WORK *symbolSave = SymbolSave_GetSymbolData(pSave);
      SymbolSave_Field_Set( symbolSave , work->monsNo , work->wazaNo , work->sex , 0 , work->place );
    }
    *seq += 1;
    break;
  
  case 9:
    GFL_BMPWIN_ClearTransWindow_VBlank( work->bmpWin );
    GFL_BMPWIN_Delete( work->bmpWin );
    GFL_MSG_Delete( work->msgHandle );
    GFL_FONT_Delete( work->fontHandle );
    return GMEVENT_RES_FINISH;
    break;

  }
  
  return GMEVENT_RES_CONTINUE;
}
