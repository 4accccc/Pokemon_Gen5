#ifdef PM_DEBUG 
///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  デバッグメニュー『ビルボード色』
 * @file   event_debug_bbd_color.c
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

#include "message.naix"
#include "font/font.naix"
#include "msg/debug/msg_d_field.h"

#include "system/bmp_winframe.h"
#include "field_bbd_color.h"
#include "event_debug_bbd_color.h"


//-----------------------------------------------------------------------------
// ビルボードカラー　の調整
//-----------------------------------------------------------------------------

//--------------------------------------------------------------
/// DEBUG_CTLIGHT_WORK ライト操作ワーク
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  GMEVENT *event;
  HEAPID heapID;
  FIELDMAP_WORK *fieldWork;

  GFL_BMPWIN* p_win;
}DEBUG_BBDCOLOR_WORK;

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlBbdColor( GMEVENT *event, int *seq, void *wk );


//--------------------------------------------------------------
/**
 * イベント：ビルボード色
 * @param gsys
 * @param wk
 * @return GMEVENT*
 */
//--------------------------------------------------------------
GMEVENT * DEBUG_EVENT_DebugMenu_BBD_Color( GAMESYS_WORK *gsys, void* wk )
{
  DEBUG_MENU_EVENT_WORK* debug_work = wk;
  HEAPID                 heapID    = debug_work->heapID;
  FIELDMAP_WORK*         fieldWork = debug_work->fieldWork;

  GMEVENT* event;
  DEBUG_BBDCOLOR_WORK* work;

  event = GMEVENT_Create( gsys, debug_work->gmEvent, 
      debugMenuControlBbdColor, sizeof(DEBUG_BBDCOLOR_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_BBDCOLOR_WORK) );

  work->gsys = gsys;
  work->event = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;

  return event;
}



//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlBbdColor(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_BBDCOLOR_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    {
      HEAPID heapID = FIELDMAP_GetHeapID( work->fieldWork );
      FLD_BBD_COLOR* p_color = FLD_BBD_COLOR_Create( heapID );
      GFL_BBDACT_SYS* p_bbd_act = FIELDMAP_GetBbdActSys( work->fieldWork );
      GFL_BBD_SYS* p_bbd_sys = GFL_BBDACT_GetBBDSystem( p_bbd_act );
      AREADATA* p_areadata = FIELDMAP_GetAreaData( work->fieldWork );

      FLD_BBD_COLOR_Load( p_color, AREADATA_GetBBDColor(p_areadata) );

      // モデル管理開始
      FLD_BBD_COLOR_DEBUG_Init( p_bbd_sys, p_color, work->heapID );

      FLD_BBD_COLOR_Delete( p_color );
    }

    // インフォーバーの非表示
    FIELD_SUBSCREEN_Exit(FIELDMAP_GetFieldSubscreenWork(work->fieldWork));
    GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_OFF );

    // ビットマップウィンドウ初期化
    {
      static const GFL_BG_BGCNT_HEADER header_sub3 = {
        0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
        GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x7000,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
      };

      GFL_BG_SetBGControl( FIELD_SUBSCREEN_BGPLANE, &header_sub3, GFL_BG_MODE_TEXT );
      GFL_BG_ClearFrame( FIELD_SUBSCREEN_BGPLANE );
      GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_ON );

      // パレット情報を転送
      GFL_ARC_UTIL_TransVramPalette(
        ARCID_FONT, NARC_font_default_nclr,
        PALTYPE_SUB_BG, FIELD_SUBSCREEN_PALLET*32, 32, work->heapID );

      // ビットマップウィンドウを作成
      work->p_win = GFL_BMPWIN_Create( FIELD_SUBSCREEN_BGPLANE,
        1, 1, 30, 22,
        FIELD_SUBSCREEN_PALLET, GFL_BMP_CHRAREA_GET_B );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->p_win ), 0xf );
      GFL_BMPWIN_MakeScreen( work->p_win );
      GFL_BMPWIN_TransVramCharacter( work->p_win );
      GFL_BG_LoadScreenReq( FIELD_SUBSCREEN_BGPLANE );

      // ウィンドウ
      BmpWinFrame_GraphicSet( FIELD_SUBSCREEN_BGPLANE, 1, 15, 0, work->heapID );
      BmpWinFrame_Write( work->p_win, TRUE, 1, 15 );
    }

    FLD_BBD_COLOR_DEBUG_PrintData( work->p_win );

    (*seq)++;
  case 1:
    // ライト管理メイン
    FLD_BBD_COLOR_DEBUG_Control();
    FLD_BBD_COLOR_DEBUG_PrintData( work->p_win );

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
      (*seq)++;
    }
    break;
  case 2:
    FLD_BBD_COLOR_DEBUG_Exit();

    // ビットマップウィンドウ破棄
    {
      GFL_BG_FreeBGControl(FIELD_SUBSCREEN_BGPLANE);
      GFL_BMPWIN_Delete( work->p_win );
    }

    // インフォーバーの表示
    FIELDMAP_SetFieldSubscreenWork(work->fieldWork,
        FIELD_SUBSCREEN_Init( work->heapID, work->fieldWork, FIELD_SUBSCREEN_NORMAL ));

    return( GMEVENT_RES_FINISH );
  }

  return( GMEVENT_RES_CONTINUE );
}

#endif // PM_DEBUG 
