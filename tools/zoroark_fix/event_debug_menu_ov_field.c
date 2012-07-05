//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		src\field\event_debug_menu_ov_field.c
 *	@brief  デバッグメニューフィールド系オーバーレイ領域
 *	@author	Toru=Nagihashi
 *	@data		2010.10.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef  PM_DEBUG

//システム
#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/game_data.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"
#include "system/main.h" //for HEAPID_FIELDMAP
#include "font/font.naix"

//フィールド
#include "fieldmap.h"
#include "field/zonedata.h"
#include "arc/fieldmap/zone_id.h"
#include "event_fieldmap_control.h"
#include "event_mapchange.h"
#include "palace_warp_check.h"
#include "../../../resource/fld3d_ci/fldci_id_def.h"  // for FLDCIID_MAX
#include "message.naix"
#include "debug_message.naix"
#include "msg/debug/msg_d_field.h"
#include "field_camera_debug.h"
#include  "field/weather_no.h"
#include  "weather.h"
#include "gamesystem/pm_weather.h"
#include "event_debug_mvpoke.h"
#include "field_bbd_color.h"

//スクリプト
#include "script.h" //SCRIPT_ChangeScript
#include "../../../resource/fldmapdata/script/debug_scr_def.h"  //SCRID_DEBUG_COMMON
#include "../../../resource/fldmapdata/script/hiden_def.h"  //SCRID_HIDEN_DIVING
#include "../../../resource/fldmapdata/script/pasokon_def.h"  //SCRID_PC

//アプリ
#include "app/townmap.h"

//デバッグ
#include "field_debug.h" 
#include "event_debug_bbd_color.h"
#include "event_debug_menu_fskill.h"
#include "test/performance.h"
#include "event_debug_menu_mmdl_list.h"
#include  "msg/debug/msg_d_tomoya.h"
#include "include/debug/debug_flg.h"

//外部公開
#include "event_debug_menu_ov_field.h"

FS_EXTERN_OVERLAY( d_iwasawa );

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define LINER_CAM_KEY_WAIT    (30)
#define LINER_CAM_KEY_COUNT    (15)

//=============================================================================
/**
 *					グローバル
*/
//=============================================================================
int DbgCutinNo = 0;

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　平均負荷計測開始
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_FieldAveStress( DEBUG_MENU_EVENT_WORK *wk )
{
  //平均負荷通常
  DEBUG_PerformanceSetAveTest(FALSE);
  return( FALSE );
}
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　パレス平均負荷計測開始
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_FieldAveStressPalace( DEBUG_MENU_EVENT_WORK *wk )
{
  //平均負荷パレス
  DEBUG_PerformanceSetAveTest(TRUE);
  return( FALSE );
}
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　フィールド位置情報
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_FieldPosData( DEBUG_MENU_EVENT_WORK *wk )
{
  FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( wk->fieldWork );
  FIELD_DEBUG_SetPrint( debug, FIELD_DEBUG_PRINT_TYPE_POSITION );

  return( FALSE );
}


// 
//=============================================================================
/**
 *					フィールドあたり判定メニューイベント
*/
//=============================================================================
typedef struct {
  HEAPID heapID;
  FLDMENUFUNC *menuFunc;
  FIELDMAP_WORK *fieldWork;
  FIELD_DEBUG_WORK *debug;
} HIT_CHECK_WK;
static GMEVENT_RESULT debugMenuFieldHitchCheck( GMEVENT *event, int *seq, void *wk );

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　フィールド当たり判定情報
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_FieldHitchCheckData( DEBUG_MENU_EVENT_WORK *wk )
{
  HIT_CHECK_WK      *work;
  GMEVENT           *event      = wk->gmEvent;
  HEAPID            heapID      = wk->heapID;
  FIELDMAP_WORK     *fieldWork  = wk->fieldWork;
  
  // メニューイベント生成
  GMEVENT_Change( event, debugMenuFieldHitchCheck, sizeof(HIT_CHECK_WK) );
  work = GMEVENT_GetEventWork( event );

  // メニューイベント生成
  work->fieldWork = fieldWork;
  work->heapID    = wk->heapID;
  work->debug     = FIELDMAP_GetDebugWork( fieldWork );

  return TRUE;
}

//======================================================================
//  デバッグメニュー　カメラ操作
//======================================================================
//--------------------------------------------------------------
/// DEBUG_CTL_LINERCAMERA_WORK 線形カメラ操作ワーク1
//--------------------------------------------------------------
typedef struct
{
  int vanish;
  GAMESYS_WORK *gsys;
  GMEVENT *event;
  HEAPID heapID;
  FIELDMAP_WORK *fieldWork;
  FLDMSGBG *pMsgBG;
  FLDMSGWIN *pMsgWin;
  STRBUF *pStrBuf;
  u16 Wait;
  u16 Count;
}DEBUG_CTL_LINERCAMERA_WORK;

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlLinerCamera(
    GMEVENT *event, int *seq, void *wk );
static void DampCameraInfo(FIELD_CAMERA * cam);
static BOOL LinerCamKeyContCtl(DEBUG_CTL_LINERCAMERA_WORK *work, const fx32 inAddVal, fx32 *outVal);


//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　線形カメラ操作
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_ControlLinerCamera( DEBUG_MENU_EVENT_WORK *wk )
{
  DEBUG_CTL_LINERCAMERA_WORK *work;
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;

  GMEVENT_Change( event, debugMenuControlLinerCamera, sizeof(DEBUG_CTL_LINERCAMERA_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_CTL_LINERCAMERA_WORK) );

  work->gsys = gsys;
  work->event = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;

  {
    FIELD_SUBSCREEN_WORK * subscreen;


    // カメラ操作は下画面で行う
    subscreen = FIELDMAP_GetFieldSubscreenWork(work->fieldWork);
    FIELD_SUBSCREEN_ChangeForce(subscreen, FIELD_SUBSCREEN_DEBUG_TOUCHCAMERA);
    {
      void * inner_work;
      FIELD_CAMERA * cam = FIELDMAP_GetFieldCamera(work->fieldWork);
      inner_work = FIELD_SUBSCREEN_DEBUG_GetControl(subscreen);
      FIELD_CAMERA_DEBUG_BindSubScreen(cam, inner_work, FIELD_CAMERA_DEBUG_BIND_CAMERA_POS, heapID);

      //カメラのバインドを切る
      FIELD_CAMERA_FreeTarget(cam);
    }
  }
  // レールカメラ反映の停止
  {
    FLDNOGRID_MAPPER* mapper;
    FLD_SCENEAREA* scenearea;


    if( FIELDMAP_GetBaseSystemType( work->fieldWork ) == FLDMAP_BASESYS_RAIL )
    {
      mapper = FIELDMAP_GetFldNoGridMapper( work->fieldWork );
      FLDNOGRID_MAPPER_SetRailCameraActive( mapper, FALSE );
    }
    else
    {
      // カメラシーンの停止
      scenearea = FIELDMAP_GetFldSceneArea( work->fieldWork );
      FLD_SCENEAREA_SetActiveFlag( scenearea, FALSE );
    }
  }

  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：線形カメラ操作
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlLinerCamera(
    GMEVENT *event, int *seq, void *wk )
{
  VecFx32 add = {0,0,0};
  BOOL shift = FALSE;

  DEBUG_CTL_LINERCAMERA_WORK *work = wk;
  FIELD_CAMERA * cam = FIELDMAP_GetFieldCamera(work->fieldWork);

  if( DEBUG_FLG_GetFlg( DEBUG_FLG_DebugCameraStop ) )
  {
    return GMEVENT_RES_CONTINUE;
  }

  if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START )
  {
    //空を飛ぶときのカメラアングルにする
    VecFx32 add = {0,16384,0}; 
    FIELD_CAMERA_SetTargetOffset(cam, &add );
    FIELD_CAMERA_SetFovy(cam, 3640 );
    FIELD_CAMERA_SetAngleLen(cam, 970752 );
    FIELD_CAMERA_SetAnglePitch(cam, 9688 );
    FIELD_CAMERA_SetAngleYaw(cam,0);
    return GMEVENT_RES_CONTINUE;
  }


  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
    FIELD_CAMERA_DEBUG_ReleaseSubScreen( cam );

    // レールカメラ反映の再開
    {
      FLDNOGRID_MAPPER* mapper;
      FLD_SCENEAREA* scenearea;

      if( FIELDMAP_GetBaseSystemType( work->fieldWork ) == FLDMAP_BASESYS_RAIL )
      {
        mapper = FIELDMAP_GetFldNoGridMapper( work->fieldWork );
        FLDNOGRID_MAPPER_SetRailCameraActive( mapper, TRUE );
      }
      else
      {
        // カメラシーンの停止
        scenearea = FIELDMAP_GetFldSceneArea( work->fieldWork );
        FLD_SCENEAREA_SetActiveFlag( scenearea, TRUE );
      }
    }
    //バインド復帰
    FIELD_CAMERA_BindDefaultTarget(cam);
    return( GMEVENT_RES_FINISH );
  }else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DEBUG ){
    DampCameraInfo(cam);
  }else if( GFL_UI_KEY_GetCont() & PAD_KEY_UP ){             //奥
    shift = LinerCamKeyContCtl(work, -FX32_ONE, &add.z);
  }else if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN ){             //手前
    shift = LinerCamKeyContCtl(work, FX32_ONE, &add.z);
  }else if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT ){             //左
    shift = LinerCamKeyContCtl(work, -FX32_ONE, &add.x);
  }else if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT ){             //右
    shift = LinerCamKeyContCtl(work, FX32_ONE, &add.x);
  }else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ){             //上
    shift = LinerCamKeyContCtl(work, FX32_ONE, &add.y);
  }else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ){             //下
    shift = LinerCamKeyContCtl(work, -FX32_ONE, &add.y);
  }else{
    work->Wait = 0;
    work->Count = 0;
  }
  if(shift){
    VecFx32 pos;
    FIELD_CAMERA_GetTargetPos( cam, &pos );
    VEC_Add( &pos, &add, &pos );
    FIELD_CAMERA_SetTargetPos( cam, &pos );
  }

  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * イベント：線形カメラ操作 情報ダンプ
 * @param cam   フィールドカメラポインタ
 * @retval  none
 */
//--------------------------------------------------------------
static void DampCameraInfo(FIELD_CAMERA * cam)
{
  u16 pitch;
  u16 yaw;
  fx32 len;
  VecFx32 target;
  u16 fovy;
  VecFx32 shift;

  OS_Printf("--DUMP_FIELD_CAMERA_PARAM--\n");

  pitch = FIELD_CAMERA_GetAnglePitch(cam);
  yaw = FIELD_CAMERA_GetAngleYaw(cam);
  len = FIELD_CAMERA_GetAngleLen(cam);
  fovy = FIELD_CAMERA_GetFovy(cam);
  FIELD_CAMERA_GetTargetOffset( cam, &shift );
  FIELD_CAMERA_GetTargetPos( cam, &target );

  OS_Printf("%d,%d,%d,%d,%d,%d\n", pitch, yaw, len, target.x, target.y, target.z );
  OS_Printf("%d,%d,%d,%d\n", fovy, shift.x, shift.y, shift.z );
}

//--------------------------------------------------------------
/**
 * イベント：線形カメラ操作キー入力
 * @param work      カメラワーク
 * @param inAddval  気入力結果の際の加算値
 * @param outVal    格納バッファ
 * @retval  BOOL    加算したらTRUE
 */
//--------------------------------------------------------------
static BOOL LinerCamKeyContCtl(DEBUG_CTL_LINERCAMERA_WORK *work, const fx32 inAddVal, fx32 *outVal)
{
  if (work->Wait >= LINER_CAM_KEY_WAIT){
    *outVal = inAddVal;
    return TRUE;
  }else{
    work->Wait++;
    if (work->Count>0){
      work->Count--;
    }else{
      *outVal = inAddVal;
      work->Count = LINER_CAM_KEY_COUNT;
      return TRUE;
    }
  }
  return FALSE;
}

//-----------------------------------------------------------------------------
/**
 *    グラフィッカー用
 *      カメラ　こまかく設定
 */
//-----------------------------------------------------------------------------
//-------------------------------------
/// デバッグカメラ　こまかく設定用ワーク
//=====================================
typedef struct
{
  FIELDMAP_WORK*  p_field;
  FIELD_CAMERA*   p_camera;
  HEAPID heapID;

  BOOL draw_help;
  BOOL param_change;

  GFL_BMPWIN* p_win;
} DEBUG_DELICATE_CAMERA_EVENT_WORK;
static GMEVENT_RESULT debugMenuDelicateCamera( GMEVENT *p_event, int *p_seq, void *p_wk_adrs );

//----------------------------------------------------------------------------
/**
 *  @brief  起動
 *
 *  @param  wk
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_ControlDelicateCamera( DEBUG_MENU_EVENT_WORK *wk )
{
  DEBUG_DELICATE_CAMERA_EVENT_WORK* p_work;
  GMEVENT* p_event = wk->gmEvent;
  FIELDMAP_WORK* p_fieldWork = wk->fieldWork;
  HEAPID heapID = wk->heapID;

  GMEVENT_Change( p_event, debugMenuDelicateCamera, sizeof(DEBUG_DELICATE_CAMERA_EVENT_WORK) );
  p_work = GMEVENT_GetEventWork( p_event );
  GFL_STD_MemClear( p_work, sizeof(DEBUG_DELICATE_CAMERA_EVENT_WORK) );

  p_work->p_field   = p_fieldWork;
  p_work->p_camera  = FIELDMAP_GetFieldCamera( p_fieldWork );
  p_work->heapID    = heapID;


  // DEBUG_ CAMERA
  GFL_OVERLAY_Load( FS_OVERLAY_ID(debug_camera) );

  FIELD_CAMERA_DEBUG_InitControl( p_work->p_camera, heapID );

  // レールカメラ反映の停止
  {
    FLDNOGRID_MAPPER* mapper;
    FLD_SCENEAREA* scenearea;

    if( FIELDMAP_GetBaseSystemType( p_work->p_field ) == FLDMAP_BASESYS_RAIL )
    {
      mapper = FIELDMAP_GetFldNoGridMapper( p_work->p_field );
      FLDNOGRID_MAPPER_SetRailCameraActive( mapper, FALSE );
    }
    else
    {
      // カメラシーンの停止
      scenearea = FIELDMAP_GetFldSceneArea( p_work->p_field );
      FLD_SCENEAREA_SetActiveFlag( scenearea, FALSE );
    }
  }


  // 表示面の作成
  {
    const FLDMAPPER* cp_mapper = FIELDMAP_GetFieldG3Dmapper( p_fieldWork );
    fx32 map_size_x, map_size_z;

    FLDMAPPER_GetSize( cp_mapper, &map_size_x, &map_size_z );

    // インフォーバーの非表示
    FIELD_SUBSCREEN_Exit(FIELDMAP_GetFieldSubscreenWork(p_fieldWork));
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
          PALTYPE_SUB_BG, FIELD_SUBSCREEN_PALLET*32, 32, heapID );

      // ビットマップウィンドウを作成
      p_work->p_win = GFL_BMPWIN_Create( FIELD_SUBSCREEN_BGPLANE,
          1, 1, 30, 22,
          FIELD_SUBSCREEN_PALLET, GFL_BMP_CHRAREA_GET_B );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_work->p_win ), 0xf );
      GFL_BMPWIN_MakeScreen( p_work->p_win );
      GFL_BMPWIN_TransVramCharacter( p_work->p_win );
      GFL_BG_LoadScreenReq( FIELD_SUBSCREEN_BGPLANE );

      // ウィンドウ
      BmpWinFrame_GraphicSet( FIELD_SUBSCREEN_BGPLANE, 1, 15, 0, heapID );
      BmpWinFrame_Write( p_work->p_win, TRUE, 1, 15 );

      FIELD_CAMERA_DEBUG_DrawInfo( p_work->p_camera, p_work->p_win, map_size_x, map_size_z );
    }
  }

  return( TRUE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  カメラ　詳細操作　メイン
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuDelicateCamera( GMEVENT *p_event, int *p_seq, void *p_wk_adrs )
{
  DEBUG_DELICATE_CAMERA_EVENT_WORK* p_work = p_wk_adrs;
  BOOL result;
  int trg = GFL_UI_KEY_GetTrg();
  int cont = GFL_UI_KEY_GetCont();
  int repeat = GFL_UI_KEY_GetRepeat();
  const FLDMAPPER* cp_mapper = FIELDMAP_GetFieldG3Dmapper( p_work->p_field );
  fx32 map_size_x, map_size_z;

  FLDMAPPER_GetSize( cp_mapper, &map_size_x, &map_size_z );

  // SELECT終了
  if( trg & PAD_BUTTON_SELECT )
  {
    // ビットマップウィンドウ破棄
    {
      BmpWinFrame_Clear( p_work->p_win, TRUE );
      GFL_BMPWIN_Delete( p_work->p_win );
      GFL_BG_FreeBGControl(FIELD_SUBSCREEN_BGPLANE);
    }

    FIELD_CAMERA_DEBUG_ExitControl( p_work->p_camera );

    // インフォーバーの表示
    FIELDMAP_SetFieldSubscreenWork(p_work->p_field,
        FIELD_SUBSCREEN_Init( p_work->heapID, p_work->p_field, FIELD_SUBSCREEN_NORMAL ));


    // DEBUG_ CAMERA
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(debug_camera) );

    return ( GMEVENT_RES_FINISH );
  }

  // DEBUGで、スクリプト操作用の表示を出す。
  // バッファリングモードも変わってしまいますが・・・。
  if( trg & PAD_BUTTON_DEBUG )
  {
    VecFx32 offset;
    VecFx32 target;
    u16 pitch;
    u16 yaw;
    fx32 len;

    // 線形カメラ用表示パラメータをカメラに設定
    FIELD_CAMERA_GetTargetOffset( p_work->p_camera, &offset );
    FIELD_CAMERA_GetTargetPos( p_work->p_camera, &target );
    pitch = FIELD_CAMERA_GetAnglePitch( p_work->p_camera );
    yaw   = FIELD_CAMERA_GetAngleYaw( p_work->p_camera );
    len   = FIELD_CAMERA_GetAngleLen( p_work->p_camera );
    FIELD_CAMERA_DEBUG_SetUpLinerCameraInfoData( p_work->p_camera );

    // 平行移動も含めた座標を返す。
    DampCameraInfo( p_work->p_camera );

    // 元に戻す
    FIELD_CAMERA_SetTargetPos( p_work->p_camera, &target );
    FIELD_CAMERA_SetTargetOffset( p_work->p_camera, &offset );
    FIELD_CAMERA_SetAnglePitch( p_work->p_camera, pitch );
    FIELD_CAMERA_SetAngleYaw( p_work->p_camera, yaw );
    FIELD_CAMERA_SetAngleLen( p_work->p_camera, len );
  }


  // カメラデバック入力
  result = FIELD_CAMERA_DEBUG_Control( p_work->p_camera, trg, cont, repeat );

  // タッチでヘルプ表示切替
  if( GFL_UI_TP_GetTrg() )
  {
    p_work->draw_help ^= 1;
    result = TRUE;
  }

  // ヘルプ表示
  if( result )
  {
    p_work->param_change = TRUE;
  }
  // 描画が重いので、操作が完了してから描画更新
  else if( p_work->param_change )
  {
    p_work->param_change = FALSE;
    if( p_work->draw_help )
    {
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_work->p_win ), 0xf );
      FIELD_CAMERA_DEBUG_DrawControlHelp( p_work->p_camera, p_work->p_win );
    }
    else
    {
      // カメラ情報を更新
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_work->p_win ), 0xf );
      FIELD_CAMERA_DEBUG_DrawInfo( p_work->p_camera, p_work->p_win, map_size_x, map_size_z );
    }
  }


  return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
BOOL debugMenuCallProc_EventFlagScript( DEBUG_MENU_EVENT_WORK *now_wk )
{
  SCRIPT_ChangeScript( now_wk->gmEvent, SCRID_DEBUG_COMMON, NULL, HEAPID_FIELDMAP );
  return TRUE;
}
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　デバッグスクリプト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_ScriptSelect( DEBUG_MENU_EVENT_WORK *now_wk )
{
  GMEVENT * new_event = DEBUG_EVENT_FLDMENU_DebugScript( now_wk );
  GMEVENT_ChangeEvent( now_wk->gmEvent, new_event );

  return TRUE;
}

//======================================================================
//  デバッグメニュー　動作モデル一覧
//======================================================================
//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　動作モデル一覧
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_MMdlList( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_menu_mmdl_list ), DEBUG_EVENT_DebugMenu_MmdlList, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return( TRUE ); 
}

BOOL debugMenuCallProc_DisguiseList( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_menu_mmdl_list ), DEBUG_EVENT_DebugMenu_DisguiseList, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return( TRUE ); 
}

//======================================================================
//  デバッグメニュー　ライト操作
//======================================================================

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
}DEBUG_CTLLIGHT_WORK;

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlLight(
    GMEVENT *event, int *seq, void *wk );

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　カメラ操作
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_ControlLight( DEBUG_MENU_EVENT_WORK *wk )
{
  DEBUG_CTLLIGHT_WORK *work;
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;

  GMEVENT_Change( event, debugMenuControlLight, sizeof(DEBUG_CTLLIGHT_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_CTLLIGHT_WORK) );

  work->gsys = gsys;
  work->event = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  return( TRUE );
}


//--------------------------------------------------------------
/**
 * イベント：カメラ操作
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlLight(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_CTLLIGHT_WORK *work = wk;
  FIELD_LIGHT* p_light;

  // ライト取得
  p_light = FIELDMAP_GetFieldLight( work->fieldWork );

  switch( (*seq) ){
  case 0:
    // ライト管理開始
    FIELD_LIGHT_DEBUG_Init( p_light, work->heapID );

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

    FIELD_LIGHT_DEBUG_PrintData( p_light, work->p_win );

    (*seq)++;
  case 1:
    // ライト管理メイン
    FIELD_LIGHT_DEBUG_Control( p_light );
    FIELD_LIGHT_DEBUG_PrintData( p_light, work->p_win );

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
      (*seq)++;
    }
    break;
  case 2:
    // ライト管理終了
    FIELD_LIGHT_DEBUG_Exit( p_light );

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

//======================================================================
//  デバッグメニュー　てんきリスト
//======================================================================
//--------------------------------------------------------------
/// DEBUG_WEATERLIST_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
}DEBUG_WEATERLIST_EVENT_WORK;


//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuWeatherListEvent(
    GMEVENT *event, int *seq, void *work );

///テストカメラリスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_WeatherList =
{
  1,    //リスト項目数
  6,    //表示最大項目数
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

///てんきメニューリスト
static const DEBUG_MENU_LIST DATA_WeatherMenuList[] =
{
  { D_TOMOYA_WEATEHR00, (void*)WEATHER_NO_SUNNY, GFL_OVERLAY_BLANK_ID },
  { D_TOMOYA_WEATEHR01, (void*)WEATHER_NO_SNOW, GFL_OVERLAY_BLANK_ID },
  { D_TOMOYA_WEATEHR02, (void*)WEATHER_NO_RAIN, GFL_OVERLAY_BLANK_ID },
  { D_TOMOYA_WEATEHR03, (void*)WEATHER_NO_STORMHIGH, GFL_OVERLAY_BLANK_ID },
  { D_TOMOYA_WEATEHR06, (void*)WEATHER_NO_SNOWSTORM, GFL_OVERLAY_BLANK_ID },
  { D_TOMOYA_WEATEHR07, (void*)WEATHER_NO_ARARE, GFL_OVERLAY_BLANK_ID },
  { D_TOMOYA_WEATEHR09, (void*)WEATHER_NO_RAIKAMI, GFL_OVERLAY_BLANK_ID },
  { D_TOMOYA_WEATEHR10, (void*)WEATHER_NO_KAZAKAMI, GFL_OVERLAY_BLANK_ID },
  { D_TOMOYA_WEATEHR11, (void*)WEATHER_NO_DIAMONDDUST, GFL_OVERLAY_BLANK_ID },
  { D_TOMOYA_WEATEHR12, (void*)WEATHER_NO_MIST, GFL_OVERLAY_BLANK_ID },

  /*  未使用
      { D_TOMOYA_WEATEHR13, (void*)WEATHER_NO_EVENING_RAIN, GFL_OVERLAY_BLANK_ID },
      { D_TOMOYA_WEATEHR05, (void*)WEATHER_NO_SPARK, GFL_OVERLAY_BLANK_ID },
      { D_TOMOYA_WEATEHR08, (void*)WEATHER_NO_MIRAGE, GFL_OVERLAY_BLANK_ID },
      */
};
#define DEBUG_WEATHERLIST_LIST_MAX  ( NELEMS(DATA_WeatherMenuList) )

static const DEBUG_MENU_INITIALIZER DebugWeatherMenuListData = {
  NARC_debug_message_d_tomoya_dat,
  NELEMS(DATA_WeatherMenuList),
  DATA_WeatherMenuList,
  &DATA_DebugMenuList_WeatherList,
  1, 1, 12, 11,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　天気リスト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_WeatherList( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_WEATERLIST_EVENT_WORK *work;

  GMEVENT_Change( event,
      debugMenuWeatherListEvent, sizeof(DEBUG_WEATERLIST_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_WEATERLIST_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：てんきリスト
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuWeatherListEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_WEATERLIST_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugWeatherMenuListData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( ret != FLDMENUFUNC_CANCEL ){  //決定
        GAMEDATA* gdata = GAMESYSTEM_GetGameData( work->gmSys );
        FIELD_WEATHER_Change( FIELDMAP_GetFieldWeather( work->fieldWork ), ret );
        GAMEDATA_SetWeatherNo( gdata, ret );
      }

      return( GMEVENT_RES_FINISH );
    }
    break;
  }

  return( GMEVENT_RES_CONTINUE );
}


//======================================================================
//  デバッグメニュー　フォグ操作
//======================================================================

//--------------------------------------------------------------
/// DEBUG_CTLFOG_WORK フォグ操作ワーク
//--------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK *gsys;
  GMEVENT *event;
  HEAPID heapID;
  FIELDMAP_WORK *fieldWork;

  GFL_BMPWIN* p_win;

}DEBUG_CTLFOG_WORK;

//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlFog(
    GMEVENT *event, int *seq, void *wk );

BOOL debugMenuCallProc_ControlFog( DEBUG_MENU_EVENT_WORK *wk )
{
  DEBUG_CTLFOG_WORK *work;
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;

  GMEVENT_Change( event, debugMenuControlFog, sizeof(DEBUG_CTLFOG_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_CTLFOG_WORK) );

  work->gsys = gsys;
  work->event = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  return( TRUE );
}


//--------------------------------------------------------------
/**
 * イベント：ふぉぐ操作
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuControlFog(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_CTLFOG_WORK *work = wk;
  FIELD_FOG_WORK* p_fog;

  // フォグ取得
  p_fog = FIELDMAP_GetFieldFog( work->fieldWork );

  switch( (*seq) ){
  case 0:

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

    FIELD_FOG_DEBUG_Init( p_fog, work->heapID );


    (*seq)++;
  case 1:
    // フォグ管理メイン
    FIELD_FOG_DEBUG_Control( p_fog );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->p_win ), 0xf );
    FIELD_FOG_DEBUG_PrintData( p_fog, work->p_win );
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
      (*seq)++;
    }
    break;
  case 2:

    // ビットマップウィンドウ破棄
    {
      GFL_BG_FreeBGControl(FIELD_SUBSCREEN_BGPLANE);
      GFL_BMPWIN_Delete( work->p_win );
    }

    FIELD_FOG_DEBUG_Exit( p_fog );


    // インフォーバーの表示
    FIELDMAP_SetFieldSubscreenWork(work->fieldWork,
        FIELD_SUBSCREEN_Init( work->heapID, work->fieldWork, FIELD_SUBSCREEN_NORMAL ));

    return( GMEVENT_RES_FINISH );
  }

  return( GMEVENT_RES_CONTINUE );
}
//--------------------------------------------------------------
/**
 *  サブスクリーン選択
*/
//--------------------------------------------------------------
typedef struct {
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  FLDMENUFUNC *menuFunc;
  FIELD_SUBSCREEN_WORK * subscreen;
}DEBUG_MENU_EVENT_SUBSCRN_SELECT_WORK, DMESSWORK;

//--------------------------------------------------------------
//  proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuSubscreenSelectEvent(
    GMEVENT *event, int *seq, void *wk );
static void setupSoundViewerSubscreen(DMESSWORK * dmess);
static void setupNormalSubscreen(DMESSWORK * dmess);
static void setupTopMenuSubscreen(DMESSWORK * dmess);
static void setupUnionSubscreen(DMESSWORK * dmess);
static void setupDebugLightSubscreen(DMESSWORK * dmess);
static void setupCGearFirstSubscreen(DMESSWORK * dmess);
static void setupNoneSubscreen(DMESSWORK * dmess);
static void setupFMissionLogSubscreen(DMESSWORK * dmess);


//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　四季マップ間移動
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_SubscreenSelect( DEBUG_MENU_EVENT_WORK *wk )
{
  HEAPID heapID = wk->heapID;
  GMEVENT *event = wk->gmEvent;
  GAMESYS_WORK *gsys = wk->gmSys;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DMESSWORK *work;

  GMEVENT_Change( event,
      debugMenuSubscreenSelectEvent, sizeof(DMESSWORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DMESSWORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;
  work->subscreen = FIELDMAP_GetFieldSubscreenWork(fieldWork);
  return( TRUE );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static const DEBUG_MENU_LIST DATA_SubcreenMenuList[] =
{
  { DEBUG_FIELD_STR_SUBSCRN02, (void*)setupSoundViewerSubscreen, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_STR_SUBSCRN03, (void*)setupNormalSubscreen, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_STR_SUBSCRN04, (void*)setupTopMenuSubscreen, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_STR_SUBSCRN01, (void*)setupDebugLightSubscreen, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_STR_SUBSCRN05, (void*)setupUnionSubscreen, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_STR_SUBSCRN06, (void*)setupCGearFirstSubscreen, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_STR_SUBSCRN07, (void*)setupNoneSubscreen, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_STR_SUBSCRN08, (void*)setupFMissionLogSubscreen, GFL_OVERLAY_BLANK_ID },
};

static const DEBUG_MENU_INITIALIZER DebugSubscreenSelectData = {
  NARC_debug_message_d_field_dat,
  NELEMS(DATA_SubcreenMenuList),
  DATA_SubcreenMenuList,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL
};

//--------------------------------------------------------------
/**
 * イベント：した画面ジャンプ
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuSubscreenSelectEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DMESSWORK *work = wk;

  switch( *seq )
  {
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugSubscreenSelectData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_CANCEL ){  //キャンセル
        return( GMEVENT_RES_FINISH );
      }else{
        typedef void (* CHANGE_FUNC)(DMESSWORK*);
        CHANGE_FUNC func = (CHANGE_FUNC)ret;
        func(work);
        //FIELD_SUBSCREEN_ChangeForce(FIELDMAP_GetFieldSubscreenWork(work->fieldWork), ret);
        return( GMEVENT_RES_FINISH );
      }
    }
    break;
  }

  return GMEVENT_RES_CONTINUE ;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupSoundViewerSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_DEBUG_SOUNDVIEWER);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupNormalSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_NORMAL);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupTopMenuSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_TOPMENU);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupUnionSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_UNION);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupDebugLightSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_DEBUG_LIGHT);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupCGearFirstSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_CGEARFIRST);
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void setupNoneSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_NOGEAR);
}
static void setupFMissionLogSubscreen(DMESSWORK * dmess)
{
  FIELD_SUBSCREEN_ChangeForce(dmess->subscreen, FIELD_SUBSCREEN_FMISSION_VIEW);
}


//======================================================================
//  デバッグメニュー　フィールドわざ
//======================================================================

//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　フィールド技リスト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_FieldSkillList( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT* event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_fskill ), DEBUG_EVENT_DebugMenu_Fskill, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return TRUE; 
}


//--------------------------------------------------------------
/**
 * @brief   移動ポケモンデバッグ
 * @param   wk DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_DebugMvPokemon( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT *event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( d_iwasawa ), EVENT_DebugMovePoke, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return( TRUE );
}

//-----------------------------------------------------------------------------
// ビルボードカラー　の調整
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *  @brief  ビルボードカラーを指定
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_BBDColor( DEBUG_MENU_EVENT_WORK *wk )
{
  GMEVENT* event;

  event = GMEVENT_CreateOverlayEventCall( wk->gmSys,
      FS_OVERLAY_ID( debug_bbd_color ), DEBUG_EVENT_DebugMenu_BBD_Color, wk );

  GMEVENT_ChangeEvent( wk->gmEvent, event );

  return TRUE; 
}

//======================================================================
//  デバッグメニュー　エンカウントエフェクトリスト
//======================================================================
//--------------------------------------------------------------
/// DEBUG_ENCEFF_LIST_EVENT_WORK
//--------------------------------------------------------------
typedef struct
{
  int seq_no;
  HEAPID heapID;
  GAMESYS_WORK *gmSys;
  GMEVENT *gmEvent;
  FIELDMAP_WORK *fieldWork;
  GFL_MSGDATA *msgData;
  FLDMENUFUNC *menuFunc;
}DEBUG_ENCEFF_LIST_EVENT_WORK;

///リスト最大
#define ENCEFFLISTMAX (38)

///リスト メニューヘッダー
static const FLDMENUFUNC_HEADER DATA_DebugMenuList_EncEffList =
{
  1,    //リスト項目数
  6,    //表示最大項目数
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

///メニューリスト
static const DEBUG_MENU_LIST DATA_EncEffMenuList[ENCEFFLISTMAX] =
{
  { DEBUG_FIELD_ENCEFF01, (void*)0, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF02, (void*)1, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF03, (void*)2, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF04, (void*)3, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF05, (void*)4, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF06, (void*)5, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF07, (void*)6, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF08, (void*)7, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF09, (void*)8, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF10, (void*)9, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF11, (void*)10, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF12, (void*)11, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF13, (void*)12, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF14, (void*)13, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF15, (void*)14, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF16, (void*)15, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF17, (void*)16, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF18, (void*)17, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF19, (void*)18, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF20, (void*)19, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF21, (void*)20, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF22, (void*)21, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF23, (void*)22, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF24, (void*)23, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF25, (void*)24, GFL_OVERLAY_BLANK_ID }, // ENCEFFID_SWBOSS
  { DEBUG_FIELD_ENCEFF26, (void*)25, GFL_OVERLAY_BLANK_ID }, // ENCEFFID_N
  { DEBUG_FIELD_ENCEFF27, (void*)26, GFL_OVERLAY_BLANK_ID }, // ENCEFFID_NEO_PLASMA
  { DEBUG_FIELD_ENCEFF28, (void*)27, GFL_OVERLAY_BLANK_ID }, // ENCEFFID_SAGE
  { DEBUG_FIELD_ENCEFF29, (void*)28, GFL_OVERLAY_BLANK_ID }, // ENCEFFID_WBT
  { DEBUG_FIELD_ENCEFF30, (void*)29, GFL_OVERLAY_BLANK_ID }, // ENCEFFID_POKEWOOD
  { DEBUG_FIELD_ENCEFF31, (void*)30, GFL_OVERLAY_BLANK_ID }, // ENCEFFID_MUGEN_BOSS
  { DEBUG_FIELD_ENCEFF37, (void*)31, GFL_OVERLAY_BLANK_ID }, // ENCEFFID_SWBOSS2
  { DEBUG_FIELD_ENCEFF32, (void*)32, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF33, (void*)33, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF34, (void*)34, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF35, (void*)35, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_ENCEFF36, (void*)36, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_EFF_PALACE, (void*)37, GFL_OVERLAY_BLANK_ID },
};

static const DEBUG_MENU_INITIALIZER DebugEncEffMenuListData = {
  NARC_debug_message_d_field_dat,
  NELEMS(DATA_EncEffMenuList),
  DATA_EncEffMenuList,
  &DATA_DebugMenuList_EncEffList, //流用
  1, 1, 16, 11,
  NULL,
  NULL
};


//--------------------------------------------------------------
/// proto
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuEncEffListEvent( GMEVENT *event, int *seq, void *wk );


//--------------------------------------------------------------
/**
 * デバッグメニュー呼び出し　エンカウントエフェクト
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_EncEffList( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_ENCEFF_LIST_EVENT_WORK *work;

  GMEVENT_Change( event,
      debugMenuEncEffListEvent, sizeof(DEBUG_ENCEFF_LIST_EVENT_WORK) );

  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_ENCEFF_LIST_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;

  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：エンカウントエフェクトリスト
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugMenuEncEffListEvent(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_ENCEFF_LIST_EVENT_WORK *work = wk;

  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID,  &DebugEncEffMenuListData );
    (*seq)++;
    break;
  case 1:
    {
      u32 ret;
      ret = FLDMENUFUNC_ProcMenu( work->menuFunc );

      if( ret == FLDMENUFUNC_NULL ){  //操作無し
        break;
      }

      FLDMENUFUNC_DeleteMenu( work->menuFunc );

      if( ret != FLDMENUFUNC_CANCEL ) //決定
      {
        //エンカウントエフェクトコール
        OS_Printf("enceff= %d\n",ret);
        ENCEFF_SetEncEff(FIELDMAP_GetEncEffCntPtr(work->fieldWork), event, ret);
        (*seq)++;
        return( GMEVENT_RES_CONTINUE );
      }
      else return( GMEVENT_RES_FINISH );
    }
    break;
  case 2:
    {
      ENCEFF_CNT_PTR cnt_ptr = FIELDMAP_GetEncEffCntPtr(work->fieldWork);
      ENCEFF_FreeUserWork(cnt_ptr);
      //オーバーレイアンロード
      ENCEFF_UnloadEffOverlay(cnt_ptr);
    }
    GFL_FADE_SetMasterBrightReq(
        GFL_FADE_MASTER_BRIGHT_WHITEOUT, 16, 0, -1 );
    (*seq)++;
    break;
  case 3:
    if( GFL_FADE_CheckFade() == FALSE ){
      (*seq) = 0;
      //return( GMEVENT_RES_FINISH );
    }
  }

  return( GMEVENT_RES_CONTINUE );
}


static GMEVENT_RESULT debugCutin(
    GMEVENT *event, int *seq, void *wk );
//--------------------------------------------------------------
/**
 * カットインデバッグ開始
 * @param wk  DEBUG_MENU_EVENT_WORK*
 * @retval  BOOL  TRUE=イベント継続
 */
//--------------------------------------------------------------
BOOL debugMenuCallProc_Cutin( DEBUG_MENU_EVENT_WORK *wk )
{
  GAMESYS_WORK *gsys = wk->gmSys;
  GMEVENT *event = wk->gmEvent;
  HEAPID heapID = wk->heapID;
  FIELDMAP_WORK *fieldWork = wk->fieldWork;
  DEBUG_ENCEFF_LIST_EVENT_WORK *work;

  GMEVENT_Change( event,
      debugCutin, sizeof(DEBUG_ENCEFF_LIST_EVENT_WORK) );
  work = GMEVENT_GetEventWork( event );
  GFL_STD_MemClear( work, sizeof(DEBUG_ENCEFF_LIST_EVENT_WORK) );

  work->gmSys = gsys;
  work->gmEvent = event;
  work->heapID = heapID;
  work->fieldWork = fieldWork;

  DbgCutinNo = 0;
  OS_Printf("----------DebugCutinStart----------\n");
  OS_Printf("DebugCutinNo %d\n",DbgCutinNo);
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * イベント：カットイン
 * @param event GMEVENT
 * @param seq   シーケンス
 * @param wk    event work
 * @retval  GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT debugCutin(
    GMEVENT *event, int *seq, void *wk )
{
  DEBUG_ENCEFF_LIST_EVENT_WORK *work = wk;

  if ( GFL_UI_KEY_GetTrg() == PAD_BUTTON_B )
  {
    OS_Printf("----------DebugCutinEnd----------\n");
    return( GMEVENT_RES_FINISH );
  }
  else if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_A )
  {
    GMEVENT *call_evt;
    FLD3D_CI_PTR ptr = FIELDMAP_GetFld3dCiPtr(work->fieldWork);
    call_evt = FLD3D_CI_CreateCutInEvt(work->gmSys, ptr, DbgCutinNo);
    GMEVENT_CallEvent( event, call_evt );
  }

  if ( GFL_UI_KEY_GetTrg() == PAD_KEY_UP )
  {
    DbgCutinNo++;
    if ( DbgCutinNo >= FLDCIID_MAX ) DbgCutinNo = 0;
    OS_Printf("DebugCutinNo %d\n",DbgCutinNo);
  }else if( GFL_UI_KEY_GetTrg() == PAD_KEY_DOWN )
  {
    DbgCutinNo--;
    if ( DbgCutinNo < 0  ) DbgCutinNo = FLDCIID_MAX-1;
    OS_Printf("DebugCutinNo %d\n",DbgCutinNo);
  }

  return( GMEVENT_RES_CONTINUE );
}


///メニューリスト用項目
static const DEBUG_MENU_LIST MenuList[] =
{
  { DEBUG_FIELD_HITCHECK_MENU_01 , (void*)DEBUG_FIELD_HITCHECK_MENU_01 , GFL_OVERLAY_BLANK_ID },  // 進入不可
  { DEBUG_FIELD_HITCHECK_MENU_02 , (void*)DEBUG_FIELD_HITCHECK_MENU_02 , GFL_OVERLAY_BLANK_ID },  // 水アトリビュート
  { DEBUG_FIELD_HITCHECK_MENU_03 , (void*)DEBUG_FIELD_HITCHECK_MENU_03 , GFL_OVERLAY_BLANK_ID },  // 敵出現有り
  { DEBUG_FIELD_HITCHECK_MENU_04 , (void*)DEBUG_FIELD_HITCHECK_MENU_04 , GFL_OVERLAY_BLANK_ID },  // 足跡有り
  { DEBUG_FIELD_HITCHECK_MENU_05 , (void*)DEBUG_FIELD_HITCHECK_MENU_05 , GFL_OVERLAY_BLANK_ID },  // 水飛沫有り
  { DEBUG_FIELD_HITCHECK_MENU_06 , (void*)DEBUG_FIELD_HITCHECK_MENU_06 , GFL_OVERLAY_BLANK_ID },  // 草揺れ有り
  { DEBUG_FIELD_HITCHECK_MENU_07 , (void*)DEBUG_FIELD_HITCHECK_MENU_07 , GFL_OVERLAY_BLANK_ID },  // 映り込み有り
  { DEBUG_FIELD_HITCHECK_MENU_08 , (void*)DEBUG_FIELD_HITCHECK_MENU_08 , GFL_OVERLAY_BLANK_ID },  // 影表示有り
  { DEBUG_FIELD_HITCHECK_MENU_09 , (void*)DEBUG_FIELD_HITCHECK_MENU_09 , GFL_OVERLAY_BLANK_ID },  // ライト有り
};

///リスト メニューヘッダー
static const FLDMENUFUNC_HEADER MenuHeader =
{
  7,    //リスト項目数
  8,    //表示最大項目数
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

//-------------------------------------
///   メニュー初期化変数
//=====================================
static const DEBUG_MENU_INITIALIZER DebugMenuHitCheckInitializer = {
  NARC_debug_message_d_field_dat,
  NELEMS(MenuList),
  MenuList,
  &MenuHeader,
  1, 1, 30, 16,
  NULL,
  NULL
};

//----------------------------------------------------------------------------
/**
 *	@brief  当たり判定メニュー
 *
 *	@param	*event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuFieldHitchCheck( GMEVENT *event, int *seq, void *wk )
{
  HIT_CHECK_WK *work = wk;

  switch( *seq ){
  case 0:
    // 起動するか、終了させるか判断
    if( FIELD_DEBUG_GetHitcheckEnable(work->debug) )
    {
      // チェックを終了させる->メニューを表示せず終了
      FIELD_DEBUG_SetHitchCheckPrint( work->debug );
      return GMEVENT_RES_FINISH;
    }
    else
    {
      // 選択肢表示->イベント継続
      work->menuFunc = DEBUGFLDMENU_Init( work->fieldWork, work->heapID, &DebugMenuHitCheckInitializer );
      (*seq)++;
    }
    break;
  case 1:
    
    switch( FLDMENUFUNC_ProcMenu(work->menuFunc) ){
    // 操作なし
    case FLDMENUFUNC_NULL:
      // イベント継続
      return( GMEVENT_RES_CONTINUE );
    // キャンセル
    case FLDMENUFUNC_CANCEL:
      {
        (*seq)++;
        return( GMEVENT_RES_CONTINUE );
      }
      break;
    case DEBUG_FIELD_HITCHECK_MENU_01:
      FIELD_DEBUG_SetHitcheckBit(work->debug,MAPATTR_FLAGBIT_HITCH);
      break;
    case DEBUG_FIELD_HITCHECK_MENU_02:
      FIELD_DEBUG_SetHitcheckBit(work->debug,MAPATTR_FLAGBIT_WATER);
      break;
    case DEBUG_FIELD_HITCHECK_MENU_03:
      FIELD_DEBUG_SetHitcheckBit(work->debug,MAPATTR_FLAGBIT_ENCOUNT);
      break;
    case DEBUG_FIELD_HITCHECK_MENU_04:
      FIELD_DEBUG_SetHitcheckBit(work->debug,MAPATTR_FLAGBIT_FOOTMARK);
      break;
    case DEBUG_FIELD_HITCHECK_MENU_05:
      FIELD_DEBUG_SetHitcheckBit(work->debug,MAPATTR_FLAGBIT_SPLASH);
      break;
    case DEBUG_FIELD_HITCHECK_MENU_06:
      FIELD_DEBUG_SetHitcheckBit(work->debug,MAPATTR_FLAGBIT_GRASS);
      break;
    case DEBUG_FIELD_HITCHECK_MENU_07:
      FIELD_DEBUG_SetHitcheckBit(work->debug,MAPATTR_FLAGBIT_REFLECT);
      break;
    case DEBUG_FIELD_HITCHECK_MENU_08:
      FIELD_DEBUG_SetHitcheckBit(work->debug,MAPATTR_FLAGBIT_SHADOW);
      break;
    case DEBUG_FIELD_HITCHECK_MENU_09:
      FIELD_DEBUG_SetHitcheckBit(work->debug,MAPATTR_FLAGBIT_LIGHT);
      break;
    }

    // 選択内容設定
    FIELD_DEBUG_SetHitchCheckPrint( work->debug );
    (*seq)++;
    break;
  case 2:
    // メニュー終了
    FLDMENUFUNC_DeleteMenu( work->menuFunc );
    return GMEVENT_RES_FINISH;
  }
  return( GMEVENT_RES_CONTINUE );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ゾロアーク不具合修正スクリプト実行
 */
//-----------------------------------------------------------------------------
BOOL debugMenuCallProc_ZoroaFlagClear( DEBUG_MENU_EVENT_WORK *wk )
{
  SCRIPT_ChangeScript( wk->gmEvent, SCRID_DEBUG_ZOROFUGUAI, NULL, HEAPID_FIELDMAP );
  return TRUE;
}


#endif  //  PM_DEBUG

