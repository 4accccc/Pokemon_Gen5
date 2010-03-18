//======================================================================
/**
 * @file    pdw_postman.c
 * @date    2010.03.17
 * @brief   PDW配達員関連
 * @author  tamada GAMEFREAK inc.
 *
 * @todo  keyWaitCursor_～をfield_msgbg.cからコピペで持ってきている
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "system/bmp_menu.h"
#include "system/bmp_menulist.h"
#include "system/bmp_winframe.h"

#include "arc_def.h"
#include "font/font.naix"

#include "message.naix"
#include "msg/msg_yesnomenu.h"

#include "field_bg_def.h"
#include "field/field_msgbg.h"

#include "sound/pm_sndsys.h"

//#include "gamesystem\msgspeed.h"

#include "winframe.naix"

#include "fieldmap.h"
#include "arc/script_message.naix"
#include "msg/script/msg_pdw_postman.h"

#include "pdw_postman.h"

#include "item/itemsym.h"
#include "field/field_msgbg.h"

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct _PIWW PDW_ITEM_WINDOW_WORK, PIWW;

//--------------------------------------------------------------
//--------------------------------------------------------------
extern PDW_ITEM_WINDOW_WORK * PDWPOSTMAN_ItemWindow_Create(
    FIELDMAP_WORK * fieldmap, const DREAMWORLD_ITEM_DATA * items, int item_count );

//--------------------------------------------------------------
//--------------------------------------------------------------
extern void PDWPOSTMAN_ItemWindow_Delete( PDW_ITEM_WINDOW_WORK * piww );

//--------------------------------------------------------------
//--------------------------------------------------------------
extern void PDWPOSTMAN_ItemWindow_Draw( PDW_ITEM_WINDOW_WORK * piww );

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
enum {
  //ウィンドウ開始位置指定：キャラ単位
  //
  //FLDMSGWINでは、この指定のさらに外側にウィンドウが描画されるので
  //その分を考慮し(0,0)ではなく(1,1)となっている
  PIWW_PX = 1,
  PIWW_PY = 1,

  PIWW_FONTX = 12,
  PIWW_FONTY = 12,

  PIWW_LINE_HEIGHT = PIWW_FONTY + 2,
  PIWW_ITEMNAME_LEN = 8,
  PIWW_MAXX = 1 + PIWW_ITEMNAME_LEN + 1 + 3 + 1,  //mgn(1) + itemname(8) + spc(1) + x??(3) + mgn(1)
  PIWW_MAX = 7,

  PIWW_SX = PIWW_FONTX * PIWW_MAXX, 
  PIWW_SY = PIWW_LINE_HEIGHT * PIWW_MAX,

  PIWW_DOTPOSX_ITEM = 0,
  PIWW_DOTPOSX_NUMBER = PIWW_DOTPOSX_ITEM + PIWW_FONTX * (PIWW_ITEMNAME_LEN+1),

  PIWW_STR_SIZE = 128,
};

//--------------------------------------------------------------
/**
 * @brief PDW配達員：受け取ったどうぐ一覧画面用ワーク
 */
//--------------------------------------------------------------
struct _PIWW{
  HEAPID heapID;
  FIELDMAP_WORK * fieldmap;
  GFL_MSGDATA * msgdata;
  FLDMSGWIN * fldmsgwin;
  FLDKEYWAITCURSOR * cursor_work;
  WORDSET * wordset;
  STRBUF * readBuf;
  STRBUF * expandBuf;

  int item_count;
  const DREAMWORLD_ITEM_DATA * items;
};

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
PDW_ITEM_WINDOW_WORK * PDWPOSTMAN_ItemWindow_Create(
    FIELDMAP_WORK * fieldmap, const DREAMWORLD_ITEM_DATA * items, int item_count )
{
  HEAPID heapID = FIELDMAP_GetHeapID( fieldmap );
  PIWW * piww = GFL_HEAP_AllocClearMemory( heapID, sizeof(PIWW) );

  piww->heapID = heapID;
  piww->fieldmap = fieldmap;

  piww->items = items;
  piww->item_count = item_count;

  piww->msgdata = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_pdw_postman_dat, heapID );

  piww->wordset = WORDSET_Create( heapID );
  piww->readBuf = GFL_STR_CreateBuffer( PIWW_STR_SIZE, heapID );
  piww->expandBuf = GFL_STR_CreateBuffer( PIWW_STR_SIZE, heapID );

  piww->fldmsgwin = FLDMSGWIN_Add( FIELDMAP_GetFldMsgBG( fieldmap ), 
      piww->msgdata, PIWW_PX, PIWW_PY,
      MATH_ROUNDUP( PIWW_SX, 8 ) / 8,
      MATH_ROUNDUP( PIWW_LINE_HEIGHT * item_count, 8) / 8 );


  return piww;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void PDWPOSTMAN_ItemWindow_Delete( PDW_ITEM_WINDOW_WORK * piww )
{
  FLDMSGWIN_ClearWindow( piww->fldmsgwin );
  FLDMSGWIN_Delete( piww->fldmsgwin );

  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );

  GFL_STR_DeleteBuffer( piww->readBuf );
  GFL_STR_DeleteBuffer( piww->expandBuf );
  WORDSET_Delete( piww->wordset );
  GFL_MSG_Delete( piww->msgdata );
  GFL_HEAP_FreeMemory( piww );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void PDWPOSTMAN_ItemWindow_Draw( PDW_ITEM_WINDOW_WORK * piww )
{
  int i;
  GFL_MSGDATA *item_man;

  item_man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
          NARC_message_itemname_dat, GFL_HEAP_LOWID(piww->heapID) );

  for ( i = 0; i < piww->item_count ; i ++ )
  {
    GFL_MSG_GetString( item_man, piww->items[i].itemindex, piww->readBuf );
    FLDMSGWIN_PrintStrBuf( piww->fldmsgwin,
        PIWW_DOTPOSX_ITEM, PIWW_LINE_HEIGHT * i, piww->readBuf );

    GFL_MSG_GetString( piww->msgdata, msg_pdw_item_01, piww->readBuf );
    WORDSET_RegisterNumber( piww->wordset, 0, piww->items[i].itemnum, 2,
        STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( piww->wordset, piww->expandBuf, piww->readBuf );
    FLDMSGWIN_PrintStrBuf( piww->fldmsgwin,
        PIWW_DOTPOSX_NUMBER, PIWW_LINE_HEIGHT * i, piww->expandBuf );
  }
  GFL_MSG_Delete( item_man );

  //@todo 本当はFLDMSGWIN_で転送関数があるべき
  //GFL_BG_LoadScreenReq( FLDMSGBG_BGFRAME );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
}


//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  FIELDMAP_WORK * fieldmap;
  PIWW * piww;

  //表示するアイテムデータ配列へのポインタ
  DREAMWORLD_ITEM_DATA * itemData;
  //表示するアイテムデータ配列数
  u32 itemCount;
  
  //表示するアイテムデータの現在先頭位置
  u16 itemPos;
}ITEM_WINDOW_EVENT_WORK;

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT_RESULT itemWindowEvent( GMEVENT * event, int *seq, void * work )
{
  ITEM_WINDOW_EVENT_WORK * iwew = work;
  u16 view_count;
  switch ( *seq )
  {
  case 0:
    //表示アイテム数を決定する
    view_count = iwew->itemCount - iwew->itemPos;
    if ( view_count > PIWW_MAX ) view_count = PIWW_MAX;

    //表示ウィンドウ：生成
    iwew->piww = PDWPOSTMAN_ItemWindow_Create(
        iwew->fieldmap, &iwew->itemData[iwew->itemPos], view_count );
    //表示ウィンドウ：描画
    PDWPOSTMAN_ItemWindow_Draw( iwew->piww );
    iwew->piww->cursor_work = FLDKEYWAITCURSOR_Create( FIELDMAP_GetHeapID( iwew->fieldmap ) );
    (*seq) ++;
    break;

  case 1:
    //表示ウィンドウ：描画まち
    if ( FLDMSGWIN_CheckPrintTrans( iwew->piww->fldmsgwin ) == TRUE )
    {
      (*seq) ++;
    }
    break;

  case 2:
    {
      GFL_BMPWIN * bmpwin = FLDMSGWIN_GetBmpWin( iwew->piww->fldmsgwin );
      GFL_BMP_DATA * bmp = GFL_BMPWIN_GetBmp( bmpwin );
      FLDKEYWAITCURSOR_Write( iwew->piww->cursor_work, bmp, 0x0f );
      GFL_BMPWIN_TransVramCharacter( bmpwin );
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      PMSND_PlaySE( SEQ_SE_MESSAGE );
      (*seq) ++;
    }
    break;

  case 3:
    //表示ウィンドウ：消去
    FLDKEYWAITCURSOR_Delete( iwew->piww->cursor_work );
    PDWPOSTMAN_ItemWindow_Delete( iwew->piww );
    (*seq) ++;
    break;

  case 4:
    if ( iwew->itemPos + PIWW_MAX >= iwew->itemCount )
    { //表示しきった場合
      GFL_HEAP_FreeMemory( (void*)iwew->itemData );
      return GMEVENT_RES_FINISH;
    }
    //まだ表示するアイテムが残っているので最初から
    iwew->itemPos += PIWW_MAX;
    *seq = 0;
    break;
  }
  return GMEVENT_RES_CONTINUE;
}

#if 0
//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static const DREAMWORLD_ITEM_DATA testData[] = {
  { ITEM_KIZUGUSURI, 10 },
  { ITEM_MASUTAABOORU, 9 },
  { ITEM_SIRUBAASUPUREE, 8},
  { ITEM_PIPPININGYOU, 20 },
  { ITEM_HAATONOUROKO, 19 },
  { ITEM_ZISYAKU, 18},
  { ITEM_GOORUDOSUPUREE, 11 },

  { ITEM_MAGATTASUPUUN, 8},
  { ITEM_OKKANOMI, 19 },
  { ITEM_MOKUTAN, 18},
  { ITEM_KODAINODOUKA, 11 },
  { ITEM_RINDONOMI, 11 },
  { ITEM_MUSIYOKESUPUREE, 12},
  { ITEM_PARESUDAMA, 4 },

  { ITEM_MONSUTAABOORU, 11 },
  { ITEM_DENKIDAMA, 12},
};

//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_PDW_PostmanWindow( GAMESYS_WORK * gsys )
{
  GMEVENT * event;
  ITEM_WINDOW_EVENT_WORK * iwew;
  event = GMEVENT_Create( gsys, NULL, itemWindowEvent, sizeof(ITEM_WINDOW_EVENT_WORK) );
  iwew = GMEVENT_GetEventWork( event );
  iwew->fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  iwew->itemPos = 0;
  iwew->itemData = testData;
  iwew->itemCount = NELEMS(testData);
  return event;
}
#endif



//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * @param gamedata
 * @param heapID
 * @param dws
 * @param size
 * @return  DREAMWORLD_ITEM_DATA  取得可能アイテムのテーブル
 *
 * アロケーションしたメモリを返すので、呼び出し側で
 * 責任持って開放すること
 */
//--------------------------------------------------------------
static DREAMWORLD_ITEM_DATA * getRestItems(
    GAMEDATA * gamedata, HEAPID heapID, DREAMWORLD_SAVEDATA * dws, u32 * size )
{
  int i, num;
  DREAMWORLD_ITEM_DATA * did;
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gamedata );

  did = GFL_HEAP_AllocClearMemory(
      heapID, sizeof(DREAMWORLD_ITEM_DATA) * DREAM_WORLD_DATA_MAX_ITEMBOX );

  for ( i = 0, num = 0; i < DREAM_WORLD_DATA_MAX_ITEMBOX; i++ )
  {
    u16 item_id = DREAMWORLD_SV_GetItem( dws, i );
    u16 item_num = DREAMWORLD_SV_GetItemNum( dws, i );
    if ( item_id != 0 )
    {
      if ( MYITEM_AddCheck( myitem, item_id, item_num, heapID ) == TRUE )
      {
        did[num].itemindex = item_id;
        did[num].itemnum = item_num;
        num ++;
      }
    }
  }
  *size = num;
  return did;
}


//--------------------------------------------------------------
/**
 * @brief PDW配達員：アイテムを受け取る
 * @param gamedata
 * @param heapID
 * @param dws
 */
//--------------------------------------------------------------
void PDW_POSTMAN_ReceiveItems( GAMEDATA * gamedata, HEAPID heapID, DREAMWORLD_SAVEDATA * dws )
{
  u32 i;
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gamedata );

  for ( i = 0; i < DREAM_WORLD_DATA_MAX_ITEMBOX; i++ )
  {
    u16 item_id = DREAMWORLD_SV_GetItem( dws, i );
    u16 item_num = DREAMWORLD_SV_GetItemNum( dws, i );
    if ( item_id != 0 )
    {
      if (MYITEM_AddItem( myitem, item_id, item_num, heapID ) == TRUE )
      {
        DREAMWORLD_SV_DeleteItem( dws, i );
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * @brief PDW配達員：受け取れないアイテムの数を取得
 * @param gamedata
 * @param heapID
 * @param dws
 * @return  u32 受け取れないアイテムの数
 */
//--------------------------------------------------------------
u32 PDW_POSTMAN_GetNGCount( GAMEDATA * gamedata, HEAPID heapID, DREAMWORLD_SAVEDATA * dws )
{
  u32 i, count;
  u32 size;
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gamedata );

  for ( i = 0, count = 0; i < DREAM_WORLD_DATA_MAX_ITEMBOX; i++ )
  {
    u16 item_id = DREAMWORLD_SV_GetItem( dws, i );
    u16 item_num = DREAMWORLD_SV_GetItemNum( dws, i );
    if ( item_id != 0 )
    {
      if ( MYITEM_AddCheck( myitem, item_id, item_num, heapID ) == FALSE )
      {
        count ++;
      }
    }
  }
  TAMADA_Printf("PDW_POSTMAN: NG COUNT=%d\n", count );
  return count;
}

//--------------------------------------------------------------
/**
 * @brief PDW配達員：受け取れないアイテムのアイテムナンバー取得
 * @param gamedata
 * @param heapID
 * @param dws
 * @param ng_count  何番目のNGアイテムかの指定
 * @return  u16 アイテムナンバー（０のとき、受け取れないアイテムは存在しない）
 */
//--------------------------------------------------------------
u16 PDW_POSTMAN_searchNGItem( GAMEDATA * gamedata, HEAPID heapID, DREAMWORLD_SAVEDATA * dws, u32 ng_count )
{
  int i;
  u32 num;
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gamedata );

  for ( i = 0, num = 0; i < DREAM_WORLD_DATA_MAX_ITEMBOX; i++ )
  {
    u16 item_id = DREAMWORLD_SV_GetItem( dws, i );
    u16 item_num = DREAMWORLD_SV_GetItemNum( dws, i );
    if ( item_id != 0 )
    {
      if ( MYITEM_AddCheck( myitem, item_id, item_num, heapID ) == FALSE )
      {
        if ( num == ng_count ) return item_id;
        num ++;
      }
    }
  }
  return 0;
}


//--------------------------------------------------------------
/**
 * @brief PDW配達員：受け取れるアイテム表示イベント
 * @param gsys
 * @return  GMEVENT 生成したイベント
 */
//--------------------------------------------------------------
GMEVENT * PDW_POSTMAN_CreateInfoEvent( GAMESYS_WORK * gsys )
{
  GMEVENT * event;
  ITEM_WINDOW_EVENT_WORK * iwew;
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  DREAMWORLD_SAVEDATA *  dws =
    DREAMWORLD_SV_GetDreamWorldSaveData( GAMEDATA_GetSaveControlWork( gamedata ) );
  event = GMEVENT_Create( gsys, NULL, itemWindowEvent, sizeof(ITEM_WINDOW_EVENT_WORK) );
  iwew = GMEVENT_GetEventWork( event );
  iwew->fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  iwew->itemPos = 0;
  iwew->itemData = getRestItems(
      gamedata, FIELDMAP_GetHeapID( iwew->fieldmap ), dws, &(iwew->itemCount) );
  TAMADA_Printf("PDW_POSTMAN: GET ITEM =%d\n", iwew->itemCount );
  return event;
}


