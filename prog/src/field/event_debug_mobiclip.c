//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		event_debug_mobiclip.c
 *	@brief  デバッグメニュー：Mobiclip再生
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.05.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_comm.h"
#include "fieldmap.h"

#include "debug_message.naix"  // NARC_xxxx
#include "msg/debug/msg_d_field.h"  // for DEBUG_FIELD_STR_xxxx
#include "event_debug_local.h"  // for DEBUG_MENU_INITIALIZER

#include "event_fieldmap_control.h"  // EVENT_FieldFade

// mobiclip
#include "mobiclip/mobiclip.h"

// mine
#include "event_debug_mobiclip.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//-------------------------------------
///	
//=====================================
typedef struct {

  HEAPID            heapID;
  GAMESYS_WORK*     gsys;
  GAMEDATA*         gdata;
  GAME_COMM_SYS_PTR gcsp;
  FIELDMAP_WORK*    fieldmap;
  FLDMENUFUNC*      menuFunc;

  BOOL  net_off_flag;
  BOOL  always_net_flag;
  ARCID arcID;

  // mobiclip
  //MOBICLIP_PARAM    *p_param;
  MOBICLIP_PARAM    param;

} EVENT_WORK;


//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================
static GMEVENT_RESULT debugMenuMobiclipSelectEvent( GMEVENT *event, int *seq, void *wk );

//=============================================================================
/**
 *          データ
 */
//=============================================================================

static const DEBUG_MENU_LIST DATA_Mobiclip[] =
{
  /*
  { 表示する文字列ID,   選択時に返す値, 依存するオーバーレイID },
  */
  { DEBUG_FIELD_MOBICLIP_01, (void*)ARCID_OPENING_1, GFL_OVERLAY_BLANK_ID },
  { DEBUG_FIELD_MOBICLIP_02, (void*)ARCID_OPENING_2, GFL_OVERLAY_BLANK_ID },
};

// デバッグメニュー初期化
static const DEBUG_MENU_INITIALIZER DebugMobiclipJumpMenuData = {
  NARC_debug_message_d_field_dat, // 引用するメッセージアーカイブの指定
  NELEMS(DATA_Mobiclip),
  DATA_Mobiclip,
  &DATA_DebugMenuList_Default, //流用
  1, 1, 16, 17,
  NULL,
  NULL,
};

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================

GMEVENT* DEBUG_EVENT_FLDMENU_MobiclipSelect( GAMESYS_WORK * gsys, void* parent_wk )
{
  GMEVENT* event;
  EVENT_WORK* work;

  // 生成
  event = GMEVENT_Create( gsys, NULL, debugMenuMobiclipSelectEvent, sizeof(EVENT_WORK) );
   // 初期化
  work = GMEVENT_GetEventWork( event );
  work->gsys     = gsys;
  work->gdata    = GAMESYSTEM_GetGameData( gsys );
  work->gcsp     = GAMESYSTEM_GetGameCommSysPtr( gsys );
  work->fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  work->heapID   = FIELDMAP_GetHeapID( work->fieldmap );
  work->menuFunc = NULL;

  return event;
}

//=============================================================================
/**
 *					static関数
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  イベント関数
 *
 *	@param	GMEVENT *event
 *	@param	*seq
 *	@param	*wk 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static GMEVENT_RESULT debugMenuMobiclipSelectEvent( GMEVENT *event, int *seq, void *wk )
{
  EVENT_WORK *work = wk;
  GAME_COMM_SYS_PTR gcsp = GAMESYSTEM_GetGameCommSysPtr( work->gsys );
 
  switch( (*seq) ){
  case 0:
    work->menuFunc = DEBUGFLDMENU_Init( work->fieldmap, work->heapID,  
                                        &DebugMobiclipJumpMenuData  );
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

      if( ret == FLDMENUFUNC_CANCEL ){ //キャンセル
        return( GMEVENT_RES_FINISH );
      }

      work->arcID = ret;
      ICHI_Printf("arcID Select = %d\n",work->arcID);

      work->net_off_flag = GAMESYSTEM_GetNetOffEventFlag( work->gsys );
      GAMESYSTEM_SetNetOffEventFlag( work->gsys, TRUE );
      work->always_net_flag = GAMESYSTEM_GetAlwaysNetFlag( work->gsys );
      GAMESYSTEM_SetAlwaysNetFlag( work->gsys, FALSE );

      if( GameCommSys_BootCheck( work->gcsp ) == GAME_COMM_NO_NULL ){
        (*seq) += 2;
      }else{
        GameCommSys_ExitReq( work->gcsp );
        (*seq) += 1;
      }
    }
    break;
  case 2:
    if(GameCommSys_BootCheck( work->gcsp) == GAME_COMM_NO_NULL){
      (*seq)++;
    }
    break;
  case 3:
    {
      // BGM停止
      PMSND_FadeOutBGM(PMSND_FADE_FAST);
      (*seq)++;
    }
		break;
  case 4:
    {
      //work->p_param = MOBICLIP_PARAM_Alloc( work->heapID, work->arcID, MOBICLIP_MODE_DEFAULT, FALSE );

      // BGM退避
      PMSND_PauseBGM(TRUE);
      PMSND_PushBGM();

      work->param.arcID = work->arcID;
      GMEVENT_CallEvent( event, EVENT_FieldSubProc( work->gsys, work->fieldmap,
            FS_OVERLAY_ID(mobiclip), &Mobiclip_ProcData, &work->param ) );
      (*seq)++;
    }
		break;
	case 5:
    {
      GAMESYSTEM_SetNetOffEventFlag( work->gsys, work->net_off_flag );
      GAMESYSTEM_SetAlwaysNetFlag( work->gsys, work->always_net_flag );
      //ネットOffフラグを落とすタイミングがフィールド復帰に間に合わないので強制復帰させる
      GAMESYSTEM_CommBootAlways(work->gsys);

      // 強制Cギア画面
      {
        //FIELD_SUBSCREEN_WORK * subscreen = FIELDMAP_GetFieldSubscreenWork(work->fieldmap);
        //FIELD_SUBSCREEN_ChangeForce(subscreen, FIELD_SUBSCREEN_NORMAL);
      }

      // BGM復帰
      PMSND_PopBGM();
      PMSND_PauseBGM(FALSE);
      PMSND_FadeInBGM(PMSND_FADE_NORMAL);

      (*seq)++;
    }
    break;
  case 6:
    //MOBICLIP_PARAM_Free( work->p_param );
    (*seq) = 0;
    break;
  }
  return ( GMEVENT_RES_CONTINUE );
}

