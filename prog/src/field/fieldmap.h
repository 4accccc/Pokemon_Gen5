//======================================================================
/**
 * @file	fieldmap.h
 * @brief フィールドマップ
 * @author
 * @date
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "system/gfl_use.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "field/field_msgbg.h"

#include "field_g3d_mapper.h"

#include "field/fieldmap_proc.h"

#include "field_bg_def.h"
#include "field_player.h"
#include "field_camera.h"
#include "field_light.h"
#include "field_fog.h"
#include "field_zonefog.h"
#include "weather.h"

#include "field_debug.h"
#include "field_subscreen.h"
#include "field_nogrid_mapper.h"
#include "field_effect.h"
#include "field_encount.h"
#include "field_place_name.h"
#include "fieldmap_func_def.h"
#include "fld_exp_obj.h"
#include "field_g3dobj.h"
#include "fieldskill_mapeff.h"

#include "savedata/sodateya_work.h"
#include "sodateya.h"
#include "field/areadata.h"

#include "fld3d_ci.h"

#include "field_task.h"
#include "field_task_manager.h"

//======================================================================
//	define
//======================================================================
///FIELDCOMM_ExitWait関数の戻り値
typedef enum{
  FIELDCOMM_EXIT_CONTINUE,    ///<通信終了待ち
  FIELDCOMM_EXIT_END,         ///<通信終了
  FIELDCOMM_EXIT_ERROR,       ///<エラー発生
}FIELDCOMM_EXIT;

typedef enum{
  DRAW3DMODE_NORMAL,          ///<通常3Ｄ描画
  DRAW3DMODE_CUTIN,           ///<3Ｄカットイン時描画
  DRAW3DMODE_SCRNTEX,         ///<スクリーンキャプチャテクスチャ描画
  DRAW3DMODE_MAX,
}DRAW3DMODE;

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	DEPEND_FUNCTIONS
//--------------------------------------------------------------
typedef struct
{
	FLDMAP_CTRLTYPE type;
	void (*create_func)(FIELDMAP_WORK*, VecFx32*, u16);
	void (*main_func)(FIELDMAP_WORK*, VecFx32*);
	void (*delete_func)(FIELDMAP_WORK*);
  const VecFx32 * (*get_camera_target_func)(FIELDMAP_WORK*);
}DEPEND_FUNCTIONS;

//======================================================================
//	extern
//======================================================================
//フィールドマップ　ツール
extern BOOL FIELDMAP_SetPlayerItemCycle( FIELDMAP_WORK *fieldWork );

//FIELDMAP_WORK 設定、参照
extern FLDMSGBG * FIELDMAP_GetFldMsgBG( FIELDMAP_WORK *fieldWork );
extern FIELD_CAMERA * FIELDMAP_GetFieldCamera( FIELDMAP_WORK *fieldWork );
extern FLDNOGRID_MAPPER* FIELDMAP_GetFldNoGridMapper( FIELDMAP_WORK *fieldWork );
extern int * FIELDMAP_GetFirstConnectID( FIELDMAP_WORK * fieldWork );
extern FIELD_LIGHT * FIELDMAP_GetFieldLight( FIELDMAP_WORK *fieldWork );
extern FIELD_FOG_WORK * FIELDMAP_GetFieldFog( FIELDMAP_WORK *fieldWork );
extern FIELD_ZONEFOGLIGHT * FIELDMAP_GetFieldZoneFog( FIELDMAP_WORK *fieldWork );
extern FIELD_WEATHER * FIELDMAP_GetFieldWeather( FIELDMAP_WORK *fieldWork );
extern u16 FIELDMAP_GetZoneWeatherID( FIELDMAP_WORK *fieldWork, u16 zone_id );
extern MMDLSYS * FIELDMAP_GetMMdlSys( FIELDMAP_WORK *fieldWork );
extern FIELD_DEBUG_WORK * FIELDMAP_GetDebugWork( FIELDMAP_WORK *fieldWork );
extern GAMESYS_WORK * FIELDMAP_GetGameSysWork( FIELDMAP_WORK *fieldWork );
extern HEAPID FIELDMAP_GetHeapID( FIELDMAP_WORK *fieldWork );
extern GFL_BBDACT_SYS * FIELDMAP_GetBbdActSys( FIELDMAP_WORK *fieldWork );
extern FLDMAPPER * FIELDMAP_GetFieldG3Dmapper( FIELDMAP_WORK *fieldWork );
extern const DEPEND_FUNCTIONS * FIELDMAP_GetDependFunctions( const FIELDMAP_WORK * fieldWork );
extern u16 FIELDMAP_GetZoneID( const FIELDMAP_WORK * fieldWork );
extern void * FIELDMAP_GetMapCtrlWork( FIELDMAP_WORK *fieldWork );
extern void FIELDMAP_SetMapCtrlWork(FIELDMAP_WORK *fieldWork,void *ctrlWork);
extern FLDMAP_CTRLTYPE FIELDMAP_GetMapControlType( FIELDMAP_WORK *fieldWork );
extern FIELD_PLAYER * FIELDMAP_GetFieldPlayer( FIELDMAP_WORK *fieldWork );
extern FIELD_SUBSCREEN_WORK* FIELDMAP_GetFieldSubscreenWork(
    FIELDMAP_WORK *fieldWork );
extern void FIELDMAP_SetFieldSubscreenWork(
    FIELDMAP_WORK *fieldWork,FIELD_SUBSCREEN_WORK* pWork );
extern FIELD_ENCOUNT * FIELDMAP_GetEncount( FIELDMAP_WORK * fieldWork );
extern FLDEFF_CTRL * FIELDMAP_GetFldEffCtrl( FIELDMAP_WORK *fieldWork );
extern FLDMAP_BASESYS_TYPE FIELDMAP_GetBaseSystemType( const FIELDMAP_WORK *fieldWork );
extern FIELD_PLACE_NAME * FIELDMAP_GetPlaceNameSys( FIELDMAP_WORK * fieldWork );
extern FLDMAPFUNC_SYS * FIELDMAP_GetFldmapFuncSys( FIELDMAP_WORK * fieldWork );
extern FLD_EXP_OBJ_CNT_PTR FIELDMAP_GetExpObjCntPtr( FIELDMAP_WORK *fieldWork );
extern GFL_TCBSYS* FIELDMAP_GetFieldmapTCBSys( FIELDMAP_WORK * fieldWork );
extern FIELD_TASK_MAN* FIELDMAP_GetTaskManager( FIELDMAP_WORK* fieldWork );
extern void FIELDMAP_SetNowPosTarget( FIELDMAP_WORK *fieldWork, const VecFx32 *pos );
extern FLDMSGWIN * FIELDMAP_GetGoldMsgWin( FIELDMAP_WORK *fieldWork );
extern void FIELDMAP_SetGoldMsgWin( FIELDMAP_WORK *fieldWork, FLDMSGWIN *msgWin );
extern SODATEYA* FIELDMAP_GetSodateya( FIELDMAP_WORK* fieldWork );
extern AREADATA* FIELDMAP_GetAreaData( FIELDMAP_WORK* fieldWork );
extern FIELDCOMM_EXIT FIELDCOMM_ExitWait(GAME_COMM_SYS_PTR game_comm);
extern GAME_COMM_NO FIELDCOMM_CheckCommNo(GAME_COMM_SYS_PTR game_comm);

extern DRAW3DMODE FIELDMAP_GetDraw3DMode(FIELDMAP_WORK *fieldWork);
extern void FIELDMAP_SetDraw3DMode(FIELDMAP_WORK *fieldWork, DRAW3DMODE mode);

extern FLD3D_CI_PTR FIELDMAP_GetFld3dCiPtr(FIELDMAP_WORK *fieldWork);
extern FIELDSKILL_MAPEFF * FIELDMAP_GetFieldSkillMapEffect( FIELDMAP_WORK * fieldWork );

extern void FIELDMAP_SetMapFadeReqFlg( FIELDMAP_WORK * fieldWork, const BOOL inReq );
extern BOOL FIELDMAP_CheckMapFadeReqFlg( FIELDMAP_WORK * fieldWork );

extern BOOL FIELDMAP_CheckCanSoundPlay( const FIELDMAP_WORK* fieldWork );

extern void FIELDMAP_InitBGMode( void );
extern void FIELDMAP_InitBG( FIELDMAP_WORK* fieldWork );
