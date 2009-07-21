//======================================================================
/**
 * @file	fieldmap.c
 * @brief	フィールドマップ処理
 * @author
 *
 * @todo  1/30で動作しているがキーは1/60で拾っているので取れないタイミングがある
 * @todo  外部からのフィールドマップ終了リクエストが同上のタイミングを考慮していない
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"					//GFL_HEAPID_APP
#include "arc_def.h"

#include "net/network_define.h"
#include "system/g3d_tool.h"

#include "print/gf_font.h"

#include "fieldmap.h"
#include "field_common.h"

#include "field_player.h"
#include "field_camera.h"
#include "field_data.h"
#include "field/field_msgbg.h"
#include "field_encount.h"

#include "weather.h"
#include "field_fog.h"
#include "field_light.h"
#include "field_buildmodel.h"
#include "field_rail.h"
#include "field_rail_loader.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_event.h"
#include "field/zonedata.h"
#include "field/areadata.h"

#include "field_comm_actor.h"
#include "field_comm/field_comm_main.h"
#include "field_comm/field_comm_event.h"

#include "fldmmdl.h"

#include "field_debug.h"

#include "map_matrix.h"
#include "field_event_check.h"  

#include "eventdata_local.h"
#include "field/eventdata_sxy.h"

#include "field_effect.h"

#include "field_player_grid.h"

#include "system/fld_wipe_3dobj.h"

#include "arc/fieldmap/zone_id.h"
#include "field/weather_no.h"
#include "sound/pm_sndsys.h"
//#include "sound/wb_sound_data.sadl"
#include "net_app/union/union_main.h"

#include "message.naix" //NARC_message_d_field_dat
#include "msg/msg_d_field.h"  //DEBUG_FIELD_STR00 DEBUG_FIELD_C_STR10

#include "debug/debugwin_sys.h" //デバッグメニュー

#if USE_DEBUGWIN_SYSTEM
#include "mapdatafunc/field_func_random_generate.h" //デバッグ登録のため
#endif //USE_DEBUGWIN_SYSTEM

#include "field_place_name.h"
#include "iss_unit.h"		
#include "sound/bgm_info.h"

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
///	GAMEMODE
//--------------------------------------------------------------
typedef enum{
	GAMEMODE_BOOT = 0,
	GAMEMODE_NORMAL,
	GAMEMODE_FINISH,
}GAMEMODE;

//--------------------------------------------------------------
///	FLDMAPSEQ
//--------------------------------------------------------------
typedef enum{
	FLDMAPSEQ_SETUP_SYSTEM = 0,
	FLDMAPSEQ_SETUP,
	FLDMAPSEQ_READY,
	FLDMAPSEQ_UPDATE,
	FLDMAPSEQ_FREE,
	FLDMAPSEQ_END,
	
	FLDMAPSEQ_MAX,
}FLDMAPSEQ;

//--------------------------------------------------------------
///	定数
//--------------------------------------------------------------
enum { 
  ///3D面の描画プライオリティー
  FIELD_3D_FRAME_PRI    =   1,

  ///テクスチャ使用サイズ指定
  FIELD_3D_VRAM_SIZE    =   GFL_G3D_TEX256K,

  ///パレット使用サイズ指定
  FIELD_3D_PLTT_SIZE    =   GFL_G3D_PLT64K,

  ///DTCMエリアのサイズ
  FIELD_3D_DTCM_SIZE    =   0x1000, 

///billboardActで使用するリソースの最大設定可能数
  FIELD_G3D_BBDACT_RESMAX	    =   64,
///billboardActで使用するオブジェクトの最大設定可能数
  FIELD_G3D_BBDACT_ACTMAX	    =   256,

  ///<セルアクターリソース最大
  FIELD_CLSYS_RESOUCE_MAX		  =   64,

  ///フォグ色
  FIELD_DEFAULT_FOG_COLOR		  =   (GX_RGB(31,31,31)),
  ///背景色
  FIELD_DEFAULT_BG_COLOR	    =   (GX_RGB(30,31,31)),
};

///g3Dutilで使用するリソースの最大設定可能数
#define G3D_UTIL_RESCOUNT	(512)
///g3Dutilで使用するオブジェクトの最大設定可能数
#define G3D_UTIL_OBJCOUNT	(128)

#define MMDL_LIST_MAX	(64) ///<動作モデルリスト最大

//--------------------------------------------------------------
/**
 * @brief フィールドマップシーケンス動作関数の戻り値
 */
//--------------------------------------------------------------
typedef enum {  
  MAINSEQ_RESULT_CONTINUE,      ///<動作継続中
  MAINSEQ_RESULT_NEXTSEQ,       ///<次のシーケンスへ切り替え
  MAINSEQ_RESULT_FINISH,        ///<終了

  MAINSEQ_RESULT_MAX
}MAINSEQ_RESULT;

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
/// フィールドマップシーケンス動作関数の型定義
//--------------------------------------------------------------
typedef MAINSEQ_RESULT (*FIELDMAP_MAIN_FUNC)(GAMESYS_WORK*,FIELDMAP_WORK*);

//--------------------------------------------------------------
///	FIELDMAP_WORK
//--------------------------------------------------------------
struct _FIELDMAP_WORK
{
	HEAPID heapID;
	GAMESYS_WORK *gsys;

  AREADATA * areadata;
	
	FIELD_CAMERA *camera_control;
	FIELD_LIGHT *light;
	FIELD_FOG_WORK *fog;
	FIELD_ZONEFOGLIGHT	 *zonefog;
	
	FIELD_WEATHER *weather_sys;
  FIELD_SUBSCREEN_WORK* fieldSubscreenWork;
	
	FIELD_COMM_MAIN *commSys;
	
	FLDMSGBG *fldMsgBG;

	FIELD_PLACE_NAME* placeNameSys;	// 地名表示ウィンドウ
	ISS_UNIT* issUnit;				// 街ISSユニット
	BGM_INFO_SYS* bgmInfoSys;		// BGM情報

	
	MMDLSYS *fldMMdlSys;

	FIELD_RAIL_MAN * railMan;
	FIELD_RAIL_LOADER * railLoader;

	FLD_SCENEAREA * sceneArea;
	FLD_SCENEAREA_LOADER * sceneAreaLoader;
	
	FLDMAPPER *g3Dmapper;
	FLD_WIPEOBJ *fldWipeObj;
	MAP_MATRIX *pMapMatrix;
	FLDMAPPER_RESISTDATA map_res;
	FIELD_PLAYER *field_player;
	FIELD_ENCOUNT *encount;
	FLDEFF_CTRL *fldeff_ctrl;

  GFL_G3D_CAMERA *g3Dcamera; //g3Dcamera Lib ハンドル
	GFL_G3D_LIGHTSET *g3Dlightset; //g3Dlight Lib ハンドル
	GFL_TCB *g3dVintr; //3D用vIntrTaskハンドル
	GFL_BBDACT_SYS *bbdActSys; //ビルボードアクトシステム設定ハンドル
	
	GAMEMODE gamemode;
	FLDMAPSEQ seq;
  u32 seq_switch;
	int timer;
	u16 map_id;
	VecFx32 now_pos;
	LOCATION location;
	
	const DEPEND_FUNCTIONS *func_tbl;
	void *mapCtrlWork;
	
	FIELD_DEBUG_WORK *debugWork;


  int firstConnectEventID;
};

fx32	fldWipeScale;
//--------------------------------------------------------------
///	MMDL_LIST
//--------------------------------------------------------------
typedef struct
{
	int count;
	u16 id_list[MMDL_LIST_MAX];
}MMDL_LIST;

//======================================================================
//	proto
//======================================================================
//フィールドメインシーケンス関数群
static MAINSEQ_RESULT mainSeqFunc_setup_system(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );
static MAINSEQ_RESULT mainSeqFunc_setup(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );
static MAINSEQ_RESULT mainSeqFunc_ready(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );
static MAINSEQ_RESULT mainSeqFunc_update(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );
static MAINSEQ_RESULT mainSeqFunc_free(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );
static MAINSEQ_RESULT mainSeqFunc_end(
    GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork );

//bg
static void	fldmap_BG_Init( FIELDMAP_WORK *fieldWork );
static void	fldmap_BG_Exit( FIELDMAP_WORK *fieldWork );

//g3d
static void fldmap_G3D_CallBackSetUp( void );
static void fldmap_G3D_Load( FIELDMAP_WORK * fieldWork );
static void fldmap_G3D_Control( FIELDMAP_WORK * fieldWork );
static void fldmap_G3D_Draw( FIELDMAP_WORK * fieldWork );
static void fldmap_G3D_Unload( FIELDMAP_WORK * fieldWork );
static void	fldmap_G3D_VBlank( GFL_TCB* tcb, void* work );
static void	fldmap_G3D_BBDTrans(
		GFL_BBDACT_TRANSTYPE type, u32 dst, u32 src, u32 siz );
static void FIELD_EDGEMARK_Setup(const AREADATA * areadata);

//fldmmdl
static void fldmapMain_MMDL_Init( FIELDMAP_WORK *fieldWork );
static void fldmapMain_MMDL_Finish( FIELDMAP_WORK *fieldWork );
static void fldmap_MMDL_InitList(
		MMDL_LIST *mlist, int list_id, HEAPID heapID );

//zonechange
static BOOL fldmapMain_UpdateMoveZone( FIELDMAP_WORK *fieldWork );
static BOOL fldmap_CheckPlayerPosUpdate( FIELDMAP_WORK *fieldWork );
static BOOL fldmap_CheckMoveZoneChange( FIELDMAP_WORK *fieldWork );
static void fldmap_ZoneChange( FIELDMAP_WORK *fieldWork );

static void zoneChange_SetMMdl(
		MMDLSYS *fmmdlsys, EVENTDATA_SYSTEM *evdata, u32 zone_id );
static void zoneChange_SetBGM( GAMEDATA *gdata, u32 zone_id );
static void zoneChange_SetWeather( FIELDMAP_WORK *fieldWork, u32 zone_id );
static void zoneChange_SetZoneFogLight( FIELDMAP_WORK *fieldWork, u32 zone_id );
static void zoneChange_UpdatePlayerWork( GAMEDATA *gdata, u32 zone_id );

//etc
static void fldmap_ClearMapCtrlWork( FIELDMAP_WORK *fieldWork );

//data
static const GFL_DISP_VRAM fldmapdata_dispVram;
static const GFL_BG_SYS_HEADER fldmapdata_bgsysHeader;
static const GFL_CLSYS_INIT fldmapdata_CLSYS_Init;
static const VecFx32 fldmapdata_cameraTarget;
static const VecFx32 fldmapdata_cameraPos;
static const GFL_G3D_LIGHTSET_SETUP fldmapdata_light0Setup;
static const GXRgb fldmapdata_edgeColorTable[8];
static const u32 fldmapdata_fogColorTable[8];

static const u16 fldmapdata_bgColorTable[16];

static void FIELDMAP_CommBoot(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, HEAPID heapID);

//======================================================================
//	フィールドマップ　生成　削除
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドマップ　生成
 * @param	gsys	GAMESYS_WORK
 * @param	heapID HEAPID
 * @retval	FIELDMAP_WORK
 */
//--------------------------------------------------------------
FIELDMAP_WORK * FIELDMAP_Create( GAMESYS_WORK *gsys, HEAPID heapID )
{
	FIELDMAP_WORK *fieldWork;
	
	fieldWork = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELDMAP_WORK) );
	fieldWork->heapID = heapID;
	fieldWork->gamemode = GAMEMODE_BOOT;
	fieldWork->gsys = gsys;
	
	{
		PLAYER_WORK *pw = GAMESYSTEM_GetMyPlayerWork(gsys);
		fieldWork->map_id = PLAYERWORK_getZoneID( pw );
	}
  {
    GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    fieldWork->areadata = AREADATA_Create( heapID,
        ZONEDATA_GetAreaID(fieldWork->map_id), GAMEDATA_GetSeasonID(gamedata) );
  }
	
	//マップコントロール
	fieldWork->func_tbl = FIELDDATA_GetFieldFunctions( fieldWork->map_id );

	//マップマトリクス
	fieldWork->pMapMatrix = MAP_MATRIX_Create( heapID );
	
	//通信用処理 
  FIELDMAP_CommBoot(gsys, fieldWork, heapID);
	
	return fieldWork;
}

//--------------------------------------------------------------
/**
 * フィールドマップ　削除
 * @param	fieldWork	FIELDMAP_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
void FIELDMAP_Delete( FIELDMAP_WORK *fieldWork )
{
  GAMESYS_WORK *gsys = fieldWork->gsys;
  
  //エリアデータ
  AREADATA_Delete( fieldWork->areadata );

	//マップマトリクス
	MAP_MATRIX_Delete( fieldWork->pMapMatrix );
	
  //ワーク開放
	GFL_HEAP_FreeMemory( fieldWork );
  
  //GAMESYSTEM内で保持しているワークをNULLに
  GAMESYSTEM_SetFieldMapWork( gsys, NULL );
}

//======================================================================
//	フィールドマップ　メイン
//======================================================================
//  フィールドメインシーケンス関数群
//--------------------------------------------------------------
//--------------------------------------------------------------
static MAINSEQ_RESULT mainSeqFunc_setup_system(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{ 
	HEAPID heapID = fieldWork->heapID;
	
	//乱数初期化
	GFL_STD_MtRandInit(0);

	//VRAMクリア
	GFL_DISP_ClearVRAM( GX_VRAM_D );
	//VRAM設定
	GFL_DISP_SetBank( &fldmapdata_dispVram );

	//BG初期化
	fldmap_BG_Init( fieldWork );
	
	// CLACT初期化
	GFL_CLACT_SYS_Create(
			&fldmapdata_CLSYS_Init, &fldmapdata_dispVram, heapID );
	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

	//BMP初期化
	GFL_BMPWIN_Init( heapID );
	
	//FONT初期化
	GFL_FONTSYS_Init();
	
	//３Ｄシステム起動
	GFL_G3D_Init(
			GFL_G3D_VMANLNK, FIELD_3D_VRAM_SIZE,
      GFL_G3D_VMANLNK, FIELD_3D_PLTT_SIZE,
			FIELD_3D_DTCM_SIZE, fieldWork->heapID, fldmap_G3D_CallBackSetUp );
	DEBUG_GFL_G3D_SetVManSize( FIELD_3D_VRAM_SIZE, FIELD_3D_PLTT_SIZE );

	//３Ｄデータのロード
	fldmap_G3D_Load( fieldWork );
	
	//配置物設定
	fieldWork->g3Dmapper = FLDMAPPER_Create( fieldWork->heapID );
	fieldWork->fldWipeObj = FLD_WIPEOBJ_Create( fieldWork->heapID );
	fldWipeScale = 0;

	TAMADA_Printf("TEX:%06x PLT:%04x\n",
			DEBUG_GFL_G3D_GetBlankTextureSize(),
			DEBUG_GFL_G3D_GetBlankPaletteSize());

	//VBlankTCB
	fieldWork->g3dVintr =
		GFUser_VIntr_CreateTCB( fldmap_G3D_VBlank, (void*)fieldWork, 0 );

	GFL_UI_StartFrameRateMode( GFL_UI_FRAMERATE_30 );
	
  return MAINSEQ_RESULT_NEXTSEQ;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static MAINSEQ_RESULT mainSeqFunc_setup(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );

  fieldWork->fldMsgBG = FLDMSGBG_Setup(
      fieldWork->heapID, fieldWork->g3Dcamera );

  // 地名表示システム作成
  fieldWork->placeNameSys = FIELD_PLACE_NAME_Create( fieldWork->heapID, fieldWork->fldMsgBG );

  // 街ISSユニットの作成
  fieldWork->issUnit = ISS_UNIT_Create( fieldWork->map_id, fieldWork->heapID );

  // BGM情報管理システムを作成
  fieldWork->bgmInfoSys = BGM_INFO_CreateSystem( fieldWork->heapID );

  fieldWork->camera_control = FIELD_CAMERA_Create(
      fieldWork,
      ZONEDATA_GetCameraID(fieldWork->map_id),
			FIELD_CAMERA_MODE_CALC_CAMERA_POS,
      fieldWork->g3Dcamera,
      &fieldWork->now_pos,
      fieldWork->heapID );

	// railデータ読み込みシステム
	fieldWork->railLoader = FIELD_RAIL_LOADER_Create( fieldWork->heapID );

  // railシステム初期化
  fieldWork->railMan = FIELD_RAIL_MAN_Create( fieldWork->heapID, fieldWork->camera_control );

	// sceneareaデータ読み込みシステム
	fieldWork->sceneAreaLoader = FLD_SCENEAREA_LOADER_Create( fieldWork->heapID );
	
	// sceneareaシステム
	fieldWork->sceneArea = FLD_SCENEAREA_Create( fieldWork->heapID, fieldWork->camera_control );

  {
    FIELD_BMODEL_MAN * bmodel_man = FLDMAPPER_GetBuildModelManager( fieldWork->g3Dmapper );
    
    FIELDDATA_SetMapperData(fieldWork->map_id,
        fieldWork->areadata,
        &fieldWork->map_res,
        fieldWork->pMapMatrix );
    { //とりあえず、電光掲示板用文字列を登録
      FIELD_BMODEL_MAN_EntryELStringID(bmodel_man,
          FIELD_BMODEL_ELBOARD_ID1, NARC_message_d_field_dat, DEBUG_FIELD_C_STR10);
      FIELD_BMODEL_MAN_EntryELStringID(bmodel_man,
          FIELD_BMODEL_ELBOARD_ID2, NARC_message_d_field_dat, DEBUG_FIELD_STR00);
    }
    //とりあえずここで配置モデルリストをセットする
    FIELD_BMODEL_MAN_Load(bmodel_man, fieldWork->map_id, fieldWork->areadata);
  }
  
  //フィールドマップ用ロケーション作成
  fieldWork->location.zone_id = fieldWork->map_id;
  fieldWork->location.pos = fieldWork->now_pos;
  
  //マップデータ登録
  FLDMAPPER_ResistData( fieldWork->g3Dmapper, &fieldWork->map_res );
  
  //動作モデル初期化
  fldmapMain_MMDL_Init(fieldWork);
  
  //フィールドエフェクト初期化
  fieldWork->fldeff_ctrl = FLDEFF_CTRL_Create(
      fieldWork, FLDEFF_PROCID_MAX, fieldWork->heapID );
  
  //フィールドエフェクト　パラメタ初期化
  FLDEFF_CTRL_SetTaskParam( fieldWork->fldeff_ctrl, FLDEFF_TASK_MAX );
  
  //フィールドエフェクト　登録
  FLDEFF_CTRL_RegistEffect( fieldWork->fldeff_ctrl,
    DATA_FLDEFF_RegistEffectGroundTbl, DATA_FLDEFF_RegistEffectGroundTblNum );
  
  {
    PLAYER_WORK *pw = GAMESYSTEM_GetMyPlayerWork(gsys);
    const u16 dir = pw->direction;
    const VecFx32 *pos = &pw->position;

    //自機作成
    {
      MYSTATUS *mystatus = GAMEDATA_GetMyStatus( gdata );
      int sex = MyStatus_GetMySex( mystatus );
      fieldWork->field_player =
        FIELD_PLAYER_Create( pw, fieldWork, pos, sex, fieldWork->heapID );
    }

    //登録テーブルごとに個別の初期化処理を呼び出し
    fieldWork->now_pos = *pos;
    fieldWork->func_tbl->create_func(fieldWork, &fieldWork->now_pos, dir);

    //レールマップの場合は、↑上記でレールに関する初期化が終わった後に
    //保存した位置を反映する
    if (ZONEDATA_DEBUG_IsRailMap(fieldWork->map_id) == TRUE)
    { 
      const RAIL_LOCATION * railLoc = GAMEDATA_GetRailLocation( gdata );
      FIELD_RAIL_MAN_SetLocation(fieldWork->railMan, railLoc);
      FIELD_RAIL_MAN_GetPos( fieldWork->railMan, &fieldWork->now_pos );
    }

    FLDMAPPER_SetPos( fieldWork->g3Dmapper, &fieldWork->now_pos );
    
    TAMADA_Printf("start X,Y,Z=%d,%d,%d\n", FX_Whole(pos->x), FX_Whole(pos->y), FX_Whole(pos->z));
    TAMADA_Printf( "Start Dir = %04x\n", pw->direction );
  }
  
  //エッジマーキング設定セットアップ
  FIELD_EDGEMARK_Setup( fieldWork->areadata );

  // フォグシステム生成
  fieldWork->fog	= FIELD_FOG_Create( fieldWork->heapID );

	// ゾーンフォグシステム生成
	fieldWork->zonefog = FIELD_ZONEFOGLIGHT_Create( fieldWork->heapID );
	FIELD_ZONEFOGLIGHT_Load( fieldWork->zonefog, FIELD_ZONEFOGLIGHT_DATA_NONE, FIELD_ZONEFOGLIGHT_DATA_NONE, fieldWork->heapID );

  // ライトシステム生成
  {
    fieldWork->light = FIELD_LIGHT_Create( AREADATA_GetLightType( fieldWork->areadata ), 
        GFL_RTC_GetTimeBySecond(), 
        fieldWork->fog, fieldWork->g3Dlightset, fieldWork->heapID );
  }

  // 天気システム生成
  fieldWork->weather_sys = FIELD_WEATHER_Init(
      fieldWork->camera_control,
      fieldWork->light,
      fieldWork->fog,
			fieldWork->zonefog, 
      fieldWork->heapID );
  
  // 天気晴れ
  FIELD_WEATHER_Set(
			fieldWork->weather_sys, ZONEDATA_GetWeatherID(fieldWork->map_id), fieldWork->heapID );
  
  //情報バーの初期化
	{
		fieldWork->fieldSubscreenWork = FIELD_SUBSCREEN_Init(fieldWork->heapID, fieldWork, GAMEDATA_GetSubScreenMode(gdata));
	}
  
  //フィールドエンカウント初期化
  fieldWork->encount = FIELD_ENCOUNT_Create( fieldWork );
  
  //フィールドデバッグ初期化
  fieldWork->debugWork = FIELD_DEBUG_Init( fieldWork, fieldWork->heapID );

#if USE_DEBUGWIN_SYSTEM
  DEBUGWIN_InitProc( GFL_BG_FRAME1_M , FLDMSGBG_GetFontHandle(fieldWork->fldMsgBG) );
  DEBUGWIN_ChangeLetterColor( 31,31,31 );
  FIELD_FUNC_RANDOM_GENERATE_InitDebug( fieldWork->heapID );
#endif  //USE_DEBUGWIN_SYSTEM


  return MAINSEQ_RESULT_NEXTSEQ;
}


//--------------------------------------------------------------
//--------------------------------------------------------------
static MAINSEQ_RESULT mainSeqFunc_ready(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{ 
	fldmap_G3D_Control( fieldWork );
	fldmap_G3D_Draw( fieldWork );
	GFL_CLACT_SYS_Main(); // CLSYSメイン
  
  FLDMSGBG_PrintMain( fieldWork->fldMsgBG );
  
  FIELD_DEBUG_UpdateProc( fieldWork->debugWork );
  
  if( fieldWork->fldMMdlSys != NULL ){
    MMDLSYS_UpdateProc( fieldWork->fldMMdlSys );
  }
  
  if( FLDMAPPER_CheckTrans(fieldWork->g3Dmapper) == FALSE ){
    return MAINSEQ_RESULT_CONTINUE;
  }
  
  FLDEFF_CTRL_Update( fieldWork->fldeff_ctrl );

  if (ZONEDATA_DEBUG_IsRailMap(fieldWork->map_id) == TRUE)
  {
    GAMEDATA * gamedata = GAMESYSTEM_GetGameData(gsys);
    EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData(gamedata);
    fieldWork->firstConnectEventID = 
      EVENTDATA_SearchConnectIDBySphere(evdata, &fieldWork->now_pos);
  }
  //フィールドマップ用イベント起動チェックをセットする
  if(fieldWork->map_id == ZONE_ID_UNION || fieldWork->map_id == ZONE_ID_CLOSSEUM){
    GAMESYSTEM_EVENT_EntryCheckFunc( gsys, FIELD_EVENT_CheckUnion, fieldWork );
  }
  else{
    GAMESYSTEM_EVENT_EntryCheckFunc( gsys, FIELD_EVENT_CheckNormal_Wrap, fieldWork );
  }
  
  fieldWork->gamemode = GAMEMODE_NORMAL;
	GAMEDATA_SetFrameSpritEnable(GAMESYSTEM_GetGameData(gsys), TRUE);

  return MAINSEQ_RESULT_NEXTSEQ;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static MAINSEQ_RESULT mainSeqFunc_update_top(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
	//キーの分割取得カウンタをリセット
	GFL_UI_ResetFrameRate();
	//ゾーン更新処理
  fldmapMain_UpdateMoveZone( fieldWork );

  //マップ別 登録処理

  if( GAMESYSTEM_GetEvent(gsys) == NULL) {

		VecFx32 rail_pos;

    // レールシステムメイン
    FIELD_RAIL_MAN_Update(fieldWork->railMan, GFL_UI_KEY_GetCont() );
    FIELD_RAIL_MAN_UpdateCamera(fieldWork->railMan);
		FIELD_RAIL_MAN_GetPos( fieldWork->railMan, &rail_pos );
		FLD_SCENEAREA_Update( fieldWork->sceneArea, &rail_pos );
    
    //登録テーブルごとに個別のメイン処理を呼び出し
    fieldWork->func_tbl->main_func( fieldWork, &fieldWork->now_pos );
    
    //Mapシステムに位置を渡している。
    //これがないとマップ移動しないので注意
    FLDMAPPER_SetPos( fieldWork->g3Dmapper, &fieldWork->now_pos );
  }

  // 地名表示システム動作処理
  FIELD_PLACE_NAME_Process( fieldWork->placeNameSys );

  //自機更新
  FIELD_PLAYER_Update( fieldWork->field_player );

  //通信用処理(プレイヤーの座標の設定とか
  FIELD_COMM_MAIN_UpdateCommSystem( fieldWork,
      fieldWork->gsys, fieldWork->field_player, fieldWork->commSys );
  Union_Main(GAMESYSTEM_GetGameCommSysPtr(gsys), fieldWork);
  
  FIELD_SUBSCREEN_Main(fieldWork->fieldSubscreenWork);
  FIELD_DEBUG_UpdateProc( fieldWork->debugWork );

	fldmap_G3D_Control( fieldWork );
  
  if( fieldWork->fldMMdlSys != NULL ){
    MMDLSYS_UpdateProc( fieldWork->fldMMdlSys );
    
    if( fieldWork->func_tbl->type == FLDMAP_CTRLTYPE_GRID ){ //仮対処
      FIELD_PLAYER_GetPos( fieldWork->field_player, &fieldWork->now_pos );
    }
  }
  
  FLDEFF_CTRL_Update( fieldWork->fldeff_ctrl );

  // TEMP: BGMの一部トラックの音量調整
  {
	  int gx, gy, gz;
      VecFx32 player_pos;

	  // グリッド座標を算
      FIELD_PLAYER_GetPos(fieldWork->field_player, &player_pos);
	  gx = (int)( ( FX_Div( player_pos.x, 16*FX32_ONE) & FX32_INT_MASK ) >> FX32_SHIFT );
	  gy = (int)( ( FX_Div( player_pos.y, 16*FX32_ONE) & FX32_INT_MASK ) >> FX32_SHIFT );
	  gz = (int)( ( FX_Div( player_pos.z, 16*FX32_ONE) & FX32_INT_MASK ) >> FX32_SHIFT );

	  // ボリュームを更新
	  ISS_UNIT_Update( fieldWork->issUnit, gx, gy, gz );
  }

  return MAINSEQ_RESULT_CONTINUE;
}

static MAINSEQ_RESULT mainSeqFunc_update_tail(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{ 
  FIELD_SUBSCREEN_Draw(fieldWork->fieldSubscreenWork);
  FIELD_CAMERA_Main( fieldWork->camera_control, GFL_UI_KEY_GetCont() );
  
  FLDMSGBG_PrintMain( fieldWork->fldMsgBG );

  FIELD_PLACE_NAME_Draw( fieldWork->placeNameSys );
  
	fldmap_G3D_Draw( fieldWork );
	GFL_CLACT_SYS_Main(); // CLSYSメイン
  
	// ゲームデータのフレーム分割用カウンタをリセット
	GAMEDATA_ResetFrameSpritCount(GAMESYSTEM_GetGameData(gsys));
  return MAINSEQ_RESULT_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static MAINSEQ_RESULT mainSeqFunc_free(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{ 
  //イベント起動チェックを停止する
  GAMESYSTEM_EVENT_EntryCheckFunc(gsys, NULL, NULL);
  
  { //アクターが持つプレイヤー現在位置をPLAYER_WORKに反映する
    VecFx32 player_pos;
    PLAYER_WORK * pw = GAMESYSTEM_GetMyPlayerWork(gsys);
    FIELD_PLAYER_GetPos(fieldWork->field_player, &player_pos);
    PLAYERWORK_setPosition(pw, &player_pos);
  }
  if (ZONEDATA_DEBUG_IsRailMap(fieldWork->map_id) == TRUE)
  {
		GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
    RAIL_LOCATION railLoc;
    FIELD_RAIL_MAN_GetLocation(fieldWork->railMan, &railLoc);
    GAMEDATA_SetRailLocation(gamedata, &railLoc);
  }
  
  // 地名表示システム破棄
  FIELD_PLACE_NAME_Delete( fieldWork->placeNameSys );

  // 街ISSユニットの破棄
  ISS_UNIT_Delete( fieldWork->issUnit );

  // BGM情報管理システムの破棄
  BGM_INFO_DeleteSystem( fieldWork->bgmInfoSys );

  //フィールドエンカウント破棄
  FIELD_ENCOUNT_Delete( fieldWork->encount );

  //情報バーの開放
	{
		GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
		GAMEDATA_SetSubScreenMode(gamedata, FIELD_SUBSCREEN_Exit(fieldWork->fieldSubscreenWork ));
	}
  
  // フィールドエフェクト破棄
  FLDEFF_CTRL_Delete( fieldWork->fldeff_ctrl );
  
  // 天気システム破棄
  FIELD_WEATHER_Exit( fieldWork->weather_sys );
  fieldWork->weather_sys = NULL;

  // ライトシステム破棄
  FIELD_LIGHT_Delete( fieldWork->light );

	// ゾーンフォグシステム破棄
	FIELD_ZONEFOGLIGHT_Delete( fieldWork->zonefog );
  
  // フォグシステム破棄
  FIELD_FOG_Delete( fieldWork->fog );

	// sceneareaシステム
	FLD_SCENEAREA_Delete( fieldWork->sceneArea );

	// sceneareaデータ読み込みシステム
	FLD_SCENEAREA_LOADER_Delete( fieldWork->sceneAreaLoader );

  // レール制御破棄
  FIELD_RAIL_MAN_Delete( fieldWork->railMan );

	// railデータ読み込みシステム
	FIELD_RAIL_LOADER_Delete( fieldWork->railLoader );
  
  FIELD_CAMERA_Delete( fieldWork->camera_control );

	//フィールド通信削除
	FIELD_COMM_MAIN_TermSystem( fieldWork, fieldWork->commSys );
  
  //登録テーブルごとに個別の終了処理を呼び出し
  fieldWork->func_tbl->delete_func(fieldWork);
  fldmap_ClearMapCtrlWork( fieldWork );
  
  fldmapMain_MMDL_Finish(fieldWork);
  
  //自機破棄
  FIELD_PLAYER_Delete( fieldWork->field_player );
  
  FLDMAPPER_ReleaseData( fieldWork->g3Dmapper );
  
  FLDMSGBG_Delete( fieldWork->fldMsgBG );

  FIELD_DEBUG_Delete( fieldWork->debugWork );

	GAMEDATA_SetFrameSpritEnable(GAMESYSTEM_GetGameData(gsys), FALSE);
	GFL_UI_StartFrameRateMode( GFL_UI_FRAMERATE_60 );

#if USE_DEBUGWIN_SYSTEM
  FIELD_FUNC_RANDOM_GENERATE_TermDebug( );
  DEBUGWIN_ExitProc();
#endif  //USE_DEBUGWIN_SYSTEM

  return MAINSEQ_RESULT_NEXTSEQ;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static MAINSEQ_RESULT mainSeqFunc_end(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{ 

	GFL_TCB_DeleteTask( fieldWork->g3dVintr );

	FLD_WIPEOBJ_Delete( fieldWork->fldWipeObj );
	FLDMAPPER_Delete( fieldWork->g3Dmapper );

	fldmap_G3D_Unload( fieldWork );	//３Ｄデータ破棄
	
	GFL_CLACT_SYS_Delete(); // CLACT破棄
	
	GFL_BMPWIN_Exit();
	
	fldmap_BG_Exit( fieldWork );
  return MAINSEQ_RESULT_FINISH;
}

//======================================================================
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドマップ　メイン
 * @param	gsys	GAMESYS_WORK
 * @param	fieldWork FIELDMAP_WORK
 * @retval	TRUE    フィールドマップ終了
 * @retval  FALSE   動作継続中
 */
//--------------------------------------------------------------
BOOL FIELDMAP_Main( GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  static const FIELDMAP_MAIN_FUNC mainfuncTable[FLDMAPSEQ_MAX][2] = { 
    //FLDMAPSEQ_SETUP_SYSTEM: 基本システムセットアップ
    { 
      mainSeqFunc_setup_system,
      mainSeqFunc_setup_system,
    },
    //FLDMAPSEQ_SETUP: セットアップ
    { 
      mainSeqFunc_setup,
      mainSeqFunc_setup,
    },
    //FLDMAPSEQ_READY: セットアップ完了
    { 
      mainSeqFunc_ready,
      mainSeqFunc_ready,
    },
    //FLDMAPSEQ_UPDATE: フィールドマップ更新
    { 
      mainSeqFunc_update_top,
      mainSeqFunc_update_tail,
    },
    //FLDMAPSEQ_FREE: フィールドマップ開放
    { 
      mainSeqFunc_free,
      mainSeqFunc_free,
    },
    //FLDMAPSEQ_END: 終了
    { 
      mainSeqFunc_end,
      mainSeqFunc_end,
    },
  };

  MAINSEQ_RESULT result;
  FIELDMAP_MAIN_FUNC func;


  GF_ASSERT(fieldWork->seq < NELEMS(mainfuncTable));
	fieldWork->timer++;
	
  result = MAINSEQ_RESULT_CONTINUE;
  func = mainfuncTable[fieldWork->seq][fieldWork->seq_switch];
  if (func != NULL) 
  { 
    result = func(gsys, fieldWork);
  }

  switch (result) 
  { 
  case MAINSEQ_RESULT_CONTINUE:
    fieldWork->seq_switch = !fieldWork->seq_switch;
    break;
  case MAINSEQ_RESULT_NEXTSEQ:
    fieldWork->seq ++;
    fieldWork->seq_switch = 0;
    break;
  case MAINSEQ_RESULT_FINISH:
    return TRUE;
  }
  return FALSE;
}


//======================================================================
//	フィールドマップ　ツール
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドマップ　終了リクエストセット
 * @param fieldWork FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELDMAP_Close( FIELDMAP_WORK *fieldWork )
{
	fieldWork->gamemode = GAMEMODE_FINISH;
	fieldWork->seq = FLDMAPSEQ_FREE;
}

//--------------------------------------------------------------
/**
 * フィールドマップ　更新可能状態チェック
 * @param	fieldWork FIELDMAP_WORK
 * @retval BOOL TRUE=更新可能
 */
//--------------------------------------------------------------
const BOOL FIELDMAP_IsReady( const FIELDMAP_WORK *fieldWork )
{
	return (fieldWork->gamemode == GAMEMODE_NORMAL);
}

//--------------------------------------------------------------
/**
 * フィールドマップ　フィールド強制更新
 * @param fieldWork FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELDMAP_ForceUpdate( FIELDMAP_WORK *fieldWork )
{
	VecFx32 rail_pos;

  // レールシステムメイン
  FIELD_RAIL_MAN_Update(fieldWork->railMan, GFL_UI_KEY_GetCont() );
  FIELD_RAIL_MAN_UpdateCamera(fieldWork->railMan);
	FIELD_RAIL_MAN_GetPos( fieldWork->railMan, &rail_pos );
	FLD_SCENEAREA_Update( fieldWork->sceneArea, &rail_pos );
  
	//登録テーブルごとに個別のメイン処理を呼び出し
	fieldWork->func_tbl->main_func( fieldWork, &fieldWork->now_pos );
	
	//Mapシステムに位置を渡している。
	//これがないとマップ移動しないので注意
	FLDMAPPER_SetPos( fieldWork->g3Dmapper, &fieldWork->now_pos );
}

//======================================================================
//	フィールドマップ　参照、設定
//======================================================================
//--------------------------------------------------------------
/**
 * FIELDMAP_WORK FIELD_COMM_MAIN取得
 * @param	fieldWork	FIELDMAP_WORK
 * @retval	void*
 */
//--------------------------------------------------------------
void * FIELDMAP_GetCommSys( const FIELDMAP_WORK *fieldWork )
{
	return (void*)fieldWork->commSys;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK FLDMSGBG取得
 * @param	fieldWork	FIELDMAP_WORK
 * @retval FLDMSGBG*
 */
//--------------------------------------------------------------
FLDMSGBG * FIELDMAP_GetFldMsgBG( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->fldMsgBG;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK FIELD_CAMERA取得
 * @param	fieldWork	FIELDMAP_WORK
 * @retval FIELD_CAMERA*
 */
//--------------------------------------------------------------
FIELD_CAMERA * FIELDMAP_GetFieldCamera( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->camera_control;
}

//--------------------------------------------------------------
/**
 *	@brief  レール制御システムの取得
 */
//--------------------------------------------------------------
FIELD_RAIL_MAN * FIELDMAP_GetFieldRailMan( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->railMan;
}
FIELD_RAIL_LOADER * FIELDMAP_GetFieldRailLoader( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->railLoader;
}

//----------------------------------------------------------------------------
/**
 *	@brief	シーンエリア制御システムの取得
 */
//-----------------------------------------------------------------------------
FLD_SCENEAREA * FIELDMAP_GetFldSceneArea( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->sceneArea;
}
FLD_SCENEAREA_LOADER * FIELDMAP_GetFldSceneAreaLoader( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->sceneAreaLoader;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
int * FIELDMAP_GetFirstConnectID( FIELDMAP_WORK * fieldWork )
{
  return &fieldWork->firstConnectEventID;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK FIELD_LIGHT取得
 * @param	fieldWork	FIELDMAP_WORK
 * @retval FIELD_LIGHT*
 */
//--------------------------------------------------------------
FIELD_LIGHT * FIELDMAP_GetFieldLight( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->light;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK FIELD_FOG_WORK取得
 * @param	fieldWork	FIELDMAP_WORK
 * @retval FIELD_FOG_WORK
 */
//--------------------------------------------------------------
FIELD_FOG_WORK * FIELDMAP_GetFieldFog( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->fog;
}

//----------------------------------------------------------------------------
/**
 * FIELDMAP_WORK FIELD_ZONEFOGLIGHT取得
 * @param	fieldWork	FIELDMAP_WORK
 * @retval FIELD_ZONEFOGLIGHT
 */
//-----------------------------------------------------------------------------
FIELD_ZONEFOGLIGHT * FIELDMAP_GetFieldZoneFog( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->zonefog;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK FIELD_WEATHER*取得
 * @param	fieldWork	FIELDMAP_WORK
 * @retval FIELD_WEATHER*
 */
//--------------------------------------------------------------
FIELD_WEATHER * FIELDMAP_GetFieldWeather( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->weather_sys;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK MMDLSYS取得
 * @param	fieldWork	FIELDMAP_WORK
 * @retval MMDLSYS*
 */
//--------------------------------------------------------------
MMDLSYS * FIELDMAP_GetMMdlSys( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->fldMMdlSys;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK FIELD_DEBUG_WORK取得
 * @param	fieldWork FIELDMAP_WORK
 * @retval	FIELD_DEBUG_WORK*
 */
//--------------------------------------------------------------
FIELD_DEBUG_WORK * FIELDMAP_GetDebugWork( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->debugWork;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK GAMESYS_WORK取得
 * @param	fieldWork	FIELDMAP_WORK
 * @retval GAMESYS_WORK*
 */
//--------------------------------------------------------------
GAMESYS_WORK * FIELDMAP_GetGameSysWork( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->gsys;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK MAP_MATRIX取得
 * @param fieldWork	FIELDMAP_WORK
 * @retval MAP_MATRIX*
 */
//--------------------------------------------------------------
MAP_MATRIX * FIELDMAP_GetMapMatrix( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->pMapMatrix;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK HEAPID取得
 * @param fieldWork	FIELDMAP_WORK
 * @retval HEAPID
 */
//--------------------------------------------------------------
HEAPID FIELDMAP_GetHeapID( FIELDMAP_WORK *fieldWork )
{
	return( fieldWork->heapID );
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK GFL_BBDACT_SYS取得
 * @param fieldWork FIELDMAP_WORK
 * @retval GFL_BBDACT_SYS
 */
//--------------------------------------------------------------
GFL_BBDACT_SYS * FIELDMAP_GetBbdActSys( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->bbdActSys;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK FLDMAPPER*取得
 * @param fieldWork FIELDMAP_WORK
 * @retval FLDMAPPER*
 */
//--------------------------------------------------------------
FLDMAPPER * FIELDMAP_GetFieldG3Dmapper( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->g3Dmapper;
}

//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
const DEPEND_FUNCTIONS * FIELDMAP_GetDependFunctions( const FIELDMAP_WORK * fieldWork )
{
  return fieldWork->func_tbl;
}

//--------------------------------------------------------------
/**
 * @brief 現在のマップ指定IDを取得
 * @param fieldWork
 * @return  u16   map指定ID
 */
//--------------------------------------------------------------
u16 FIELDMAP_GetZoneID( const FIELDMAP_WORK * fieldWork )
{
  return fieldWork->map_id;
}


//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
const VecFx32 * FIELDMAP_GetNowPos( const FIELDMAP_WORK * fieldWork )
{
  return &fieldWork->now_pos;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK マップコントロール用ワーク取得
 * @param fieldWork FIELDMAP_WORK
 * @retval void* マップコントロール用ワーク
 */
//--------------------------------------------------------------
void * FIELDMAP_GetMapCtrlWork( FIELDMAP_WORK *fieldWork )
{
	GF_ASSERT( fieldWork->mapCtrlWork != NULL ); //未登録参照は禁止
	return fieldWork->mapCtrlWork;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK マップコントロール用ワークセット
 * @param fieldWork FIELDMAP_WORK
 * @param	ctrlWork マップコントロール用ワーク
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELDMAP_SetMapCtrlWork( FIELDMAP_WORK *fieldWork, void *ctrlWork )
{
	GF_ASSERT( fieldWork->mapCtrlWork == NULL ); //二重登録禁止
	fieldWork->mapCtrlWork = ctrlWork;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK FIELD_PLAYER取得
 * @param fieldWork FIELDMAP_WORK
 * @retval FIELD_PLAYER*
 */
//--------------------------------------------------------------
FIELD_PLAYER * FIELDMAP_GetFieldPlayer( FIELDMAP_WORK *fieldWork )
{
	GF_ASSERT( fieldWork->field_player != NULL ); //未登録参照は禁止
	return fieldWork->field_player;
}

//--------------------------------------------------------------
/**
 * @brief  FIELD_SUBSCREEN_WORKを得る
 * @param fieldWork FIELDMAP_WORK
 * @retval FIELD_SUBSCREEN_WORK*
 */
//--------------------------------------------------------------
FIELD_SUBSCREEN_WORK * FIELDMAP_GetFieldSubscreenWork(
    FIELDMAP_WORK *fieldWork )
{
	return fieldWork->fieldSubscreenWork;
}

//--------------------------------------------------------------
/**
 * @brief  FIELD_SUBSCREEN_WORKを設定する
 * @param  fieldWork FIELDMAP_WORK
 * @retval FIELD_SUBSCREEN_WORK*
 */
//--------------------------------------------------------------
void FIELDMAP_SetFieldSubscreenWork(
    FIELDMAP_WORK *fieldWork, FIELD_SUBSCREEN_WORK *pWork )
{
  fieldWork->fieldSubscreenWork = pWork;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK FLDEFF_CTRL取得
 * @param fieldWork FIELDMAP_WORK
 * @retval FLDEFF_CTRL*
 */
//--------------------------------------------------------------
FLDEFF_CTRL * FIELDMAP_GetFldEffCtrl( FIELDMAP_WORK *fieldWork )
{
  return fieldWork->fldeff_ctrl;
}

//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
FIELD_ENCOUNT * FIELDMAP_GetEncount( FIELDMAP_WORK * fieldWork )
{
  return fieldWork->encount;
}

//--------------------------------------------------------------
/**
 * 現在のマップコントロールタイプを取得 
 * @param fieldWork FIELDMAP_WORK
 * @retval FLDMAP_CTRLTYPE
 */
//--------------------------------------------------------------
FLDMAP_CTRLTYPE FIELDMAP_GetMapControlType( FIELDMAP_WORK *fieldWork )
{
  GF_ASSERT( fieldWork->func_tbl != NULL );
  return fieldWork->func_tbl->type;
}

//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
FIELD_PLACE_NAME * FIELDMAP_GetPlaceNameSys( FIELDMAP_WORK * fieldWork )
{
  return fieldWork->placeNameSys;
}

//======================================================================
//	フィールドマップ　サブ　BG関連
//======================================================================
//--------------------------------------------------------------
/**
 * BG 初期化
 * @param fieldWork FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void	fldmap_BG_Init( FIELDMAP_WORK *fieldWork )
{
	//ＢＧシステム起動
	GFL_BG_Init( fieldWork->heapID );
	
	//背景色パレット転送
  GFL_BG_LoadPalette( GFL_BG_FRAME0_M, (void*)fldmapdata_bgColorTable, 16*2, 0 ); //メイン画面の背景色
  GFL_BG_LoadPalette( GFL_BG_FRAME0_S, (void*)fldmapdata_bgColorTable, 16*2, 0 ); //サブ画面の背景色

	//ＢＧモード設定
	GFL_BG_SetBGMode( &fldmapdata_bgsysHeader );

	//ＢＧコントロール設定
	G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2, GX_BLEND_PLANEMASK_BG3, 16, 8 );
	
	GFL_BG_SetBGControl3D( FIELD_3D_FRAME_PRI );

	//ディスプレイ面の選択
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
	GFL_DISP_SetDispOn();
  
  //使用するウィンドウ
  GX_SetVisibleWnd( GX_WNDMASK_NONE );
}

//--------------------------------------------------------------
/**
 * BG 削除
 * @param fieldWork FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void	fldmap_BG_Exit( FIELDMAP_WORK *fieldWork )
{
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
	GFL_G3D_Exit();
	GFL_BG_Exit();
}

//======================================================================
//	フィールドマップ　サブ　G3D関連
//======================================================================
//--------------------------------------------------------------
/**
 * G3D セットアップコールバック
 * @param nothing
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldmap_G3D_CallBackSetUp( void )
{
	//各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
	G3X_SetShading( GX_SHADING_TOON );
	G3X_AntiAlias( TRUE );
	G3X_AlphaTest( FALSE, 0 );	// アルファテスト　　オフ
	G3X_AlphaBlend( TRUE );

	// フォグセットアップ
  G3X_SetFog(FALSE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x0400, 0x00e00 );
  G3X_SetFogColor(FIELD_DEFAULT_FOG_COLOR, 0);
  G3X_SetFogTable(fldmapdata_fogColorTable);
	
	// クリアカラーの設定
	//color,alpha,depth,polygonID,fog
	G3X_SetClearColor(GX_RGB(0,0,0),31,0x7fff,63,FALSE);
	
	G3X_SetEdgeColorTable( fldmapdata_edgeColorTable ); 
	G3X_EdgeMarking( FALSE );
	
	// ビューポートの設定
	G3_ViewPort(0, 0, 255, 191);
	GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z );
}

//--------------------------------------------------------------
/**
 * G3D ロード
 * @param fieldWork FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldmap_G3D_Load( FIELDMAP_WORK *fieldWork )
{
	fieldWork->bbdActSys = GFL_BBDACT_CreateSys(
			FIELD_G3D_BBDACT_RESMAX, FIELD_G3D_BBDACT_ACTMAX,
			fldmap_G3D_BBDTrans, fieldWork->heapID );
  {
    GFL_BBD_SYS * bbdsys = GFL_BBDACT_GetBBDSystem(fieldWork->bbdActSys);
    GFL_BBD_SetOrigin(bbdsys, GFL_BBD_ORIGIN_BOTTOM);
  }
	//カメラ作成
	fieldWork->g3Dcamera = GFL_G3D_CAMERA_CreateDefault(
			&fldmapdata_cameraPos, &fldmapdata_cameraTarget, fieldWork->heapID );
	
	//ライト作成
	fieldWork->g3Dlightset = GFL_G3D_LIGHT_Create(
			&fldmapdata_light0Setup, fieldWork->heapID );

	//カメラライト0反映
	GFL_G3D_CAMERA_Switching( fieldWork->g3Dcamera );
	GFL_G3D_LIGHT_Switching( fieldWork->g3Dlightset );
	
}

//--------------------------------------------------------------
/**
 * G3D 動作
 * @param fieldWork FIELMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldmap_G3D_Control( FIELDMAP_WORK * fieldWork )
{
	FLDMAPPER_Main( fieldWork->g3Dmapper );
	GFL_BBDACT_Main( fieldWork->bbdActSys );
	
	FIELD_WEATHER_Main( fieldWork->weather_sys, fieldWork->heapID );
	FIELD_FOG_Main( fieldWork->fog );
	FIELD_LIGHT_Main( fieldWork->light, GFL_RTC_GetTimeBySecond() );
}

//--------------------------------------------------------------
/**
 * G3D 描画
 * @param fieldWork FIELMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
//プロジェクションマトリクスを操作する際のＺオフセット
#define	PRO_MAT_Z_OFS	(310)
static void fldmap_G3D_Draw( FIELDMAP_WORK * fieldWork )
{

	FIELD_FOG_Reflect( fieldWork->fog );
	FIELD_LIGHT_Reflect( fieldWork->light );
	
	GFL_G3D_CAMERA_Switching( fieldWork->g3Dcamera );
	GFL_G3D_LIGHT_Switching( fieldWork->g3Dlightset );
  
  FLDMSGBG_PrintG3D( fieldWork->fldMsgBG );
  

	FLDMAPPER_Draw( fieldWork->g3Dmapper, fieldWork->g3Dcamera );
	FLD_WIPEOBJ_Main( fieldWork->fldWipeObj, fldWipeScale ); 
  {
	  MtxFx44 org_pm,pm;
		const MtxFx44 *m;
		m = NNS_G3dGlbGetProjectionMtx();
/**		
		OS_Printf("%x,%x,%x,%x\n%x,%x,%x,%x\n%x,%x,%x,%x\n%x,%x,%x,%x\n",
				m->_00,m->_01,m->_02,m->_03,
				m->_10,m->_11,m->_12,m->_13,
				m->_20,m->_21,m->_22,m->_23,
				m->_30,m->_31,m->_32,m->_33);
//*/				
    
		org_pm = *m;
		pm = org_pm;
		pm._32 += FX_Mul( pm._22, PRO_MAT_Z_OFS );
		NNS_G3dGlbSetProjectionMtx(&pm);
		NNS_G3dGlbFlush();		  //　ジオメトリコマンドを転送
    NNS_G3dGeFlushBuffer(); // 転送まち

    FLDEFF_CTRL_Draw( fieldWork->fldeff_ctrl );
  
    GFL_BBDACT_Draw(
        fieldWork->bbdActSys, fieldWork->g3Dcamera, fieldWork->g3Dlightset );

		NNS_G3dGlbSetProjectionMtx(&org_pm);
		NNS_G3dGlbFlush();		//　ジオメトリコマンドを転送
  }

	
  FIELD_WEATHER_3DWrite( fieldWork->weather_sys );	// 天気描画処理
	
  GFL_G3D_DRAW_End(); //描画終了（バッファスワップ）
}

//--------------------------------------------------------------
/**
 * G3D 破棄
 * @param fieldWork FIELMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldmap_G3D_Unload( FIELDMAP_WORK * fieldWork )
{
	GFL_G3D_LIGHT_Delete( fieldWork->g3Dlightset );
	GFL_G3D_CAMERA_Delete( fieldWork->g3Dcamera );
	GFL_BBDACT_DeleteSys( fieldWork->bbdActSys );
}

//--------------------------------------------------------------
/**
 * G3D VBlank処理
 * @param TCB GFL_TCB
 * @param work tcb work
 * @retval nothing
 */
//--------------------------------------------------------------
static void	fldmap_G3D_VBlank( GFL_TCB *tcb, void *work )
{
	FIELDMAP_WORK * fieldWork = (FIELDMAP_WORK*)work;
  
  if( fieldWork->fldMMdlSys != NULL ){
    MMDLSYS_VBlankProc( fieldWork->fldMMdlSys );
  }

	GFL_CLACT_SYS_VBlankFunc();	//セルアクターVBlank
}

//--------------------------------------------------------------
/**
 * G3D ビルボードアクター用VRAM転送処理
 * @param type GFL_BBDACT_TRANSTYPE
 * @param dst テクスチャデータ転送先アドレス
 * @param src テクスチャデータソースアドレス
 * @param	siz テクスチャデータサイズ
 * @retval nothing
 */
//--------------------------------------------------------------
static void	fldmap_G3D_BBDTrans(
		GFL_BBDACT_TRANSTYPE type, u32 dst, u32 src, u32 siz )
{
	NNS_GFD_DST_TYPE transType;
	
	if( type == GFL_BBDACT_TRANSTYPE_DATA ){
		transType = NNS_GFD_DST_3D_TEX_VRAM;
	}else{
		transType = NNS_GFD_DST_3D_TEX_PLTT;
	}
	
	NNS_GfdRegisterNewVramTransferTask( transType, dst, (void*)src, siz );
}

//--------------------------------------------------------------
/**
 * @brief エッジマーキング設定反映
 * @param areadata
 */
//--------------------------------------------------------------
static void FIELD_EDGEMARK_Setup(const AREADATA * areadata)
{
  static GXRgb edgeTable[8];
  int edgemark_type = AREADATA_GetEdgeMarkingType(areadata);

  if (edgemark_type == 0xff)
  {
    G3X_EdgeMarking(FALSE);
    return;
  }
  G3X_EdgeMarking(TRUE);
  GFL_ARC_LoadData((void *)edgeTable, ARCID_FIELD_EDGEMARK, edgemark_type);
  G3X_SetEdgeColorTable( edgeTable );
}

//======================================================================
//	フィールドマップ　サブ　動作モデル関連
//======================================================================
//--------------------------------------------------------------
/**
 * 動作モデル　初期化
 * @param fieldWork FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldmapMain_MMDL_Init( FIELDMAP_WORK *fieldWork )
{
	GAMEDATA *gdata = GAMESYSTEM_GetGameData( fieldWork->gsys );
	MMDLSYS *fmmdlsys = GAMEDATA_GetMMdlSys( gdata );
		
	fieldWork->fldMMdlSys = fmmdlsys;
	MMDLSYS_SetFieldMapWork( fmmdlsys, fieldWork );

	MMDLSYS_SetupProc( fmmdlsys,	//動作モデルシステム　セットアップ
		fieldWork->heapID, fieldWork->g3Dmapper );
		
	MMDL_BLACTCONT_Setup(		//動作モデルビルボード　セットアップ
		fieldWork->fldMMdlSys, fieldWork->bbdActSys, 32 );
	
	{ //ビルボードリソース登録
	  MMDL_LIST mlist;
	  int list_area_id = 0; //仮
	  fldmap_MMDL_InitList( &mlist, list_area_id, fieldWork->heapID );
	  MMDL_BLACTCONT_AddResourceTex(
	    fieldWork->fldMMdlSys, mlist.id_list, mlist.count );
	}
  
	//動作モデル描画　セットアップ
	MMDLSYS_SetupDrawProc( fieldWork->fldMMdlSys );
	
	//動作モデル　復帰
	MMDLSYS_Pop( fieldWork->fldMMdlSys );
  
  if( fieldWork->func_tbl->type == FLDMAP_CTRLTYPE_GRID ){
    MMDLSYS_SetJoinShadow( fmmdlsys, TRUE );
  }else{
    MMDLSYS_SetJoinShadow( fmmdlsys, FALSE );
  }
  
#ifdef DEBUG_ONLY_FOR_kagaya
  #define OBJID_TEST_TRAINER (128)
  #include "../../../resource/fldmapdata/script/trainer_def.h"
  
  if( fieldWork->location.zone_id == ZONE_ID_T01 )
  {
    MMDLSYS *mmdlsys = fieldWork->fldMMdlSys;
    
    if( MMDLSYS_SearchOBJID(mmdlsys,OBJID_TEST_TRAINER) == NULL )
    {
      MMDL_HEADER head =
      {
        OBJID_TEST_TRAINER,
        BOY1,
        MV_DOWN,
        EV_TYPE_TRAINER, 0, SCRID_TANPAN_01,
        DIR_DOWN,
        4, 0, 0,
        0, 0,
        753,814,0,
      };
      
      MMDLSYS_AddMMdl( mmdlsys, &head, ZONE_ID_T01 );
    }
  }
#endif
}

//--------------------------------------------------------------
/**
 * 動作モデル　終了
 * @param fieldWork FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldmapMain_MMDL_Finish( FIELDMAP_WORK *fieldWork )
{
	MMDLSYS_Push( fieldWork->fldMMdlSys );
	MMDLSYS_DeleteProc( fieldWork->fldMMdlSys );
	fieldWork->fldMMdlSys = NULL;
}

//--------------------------------------------------------------
/**
 * 動作モデル　リスト作成
 * @param mlist MMDL_LIST
 * @param list_id 作成するリストID
 * @param	heapID テンポラリ用HEAPID
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldmap_MMDL_InitList(
		MMDL_LIST *mlist, int list_id, HEAPID heapID )
{
	int i = 0;
	u16 *pList;
	pList = GFL_ARC_LoadDataAlloc( ARCID_MMDL_LIST, list_id, heapID );
	mlist->count = 0;
	
	while( pList[i] != OBJCODEMAX ){
		mlist->id_list[i] = pList[i];
		i++;
		GF_ASSERT( i < MMDL_LIST_MAX );
	}
	
	OS_Printf( "モデルリスト総数 %d\n", i );
	
	mlist->count = i;
	mlist->id_list[i] = OBJCODEMAX;
	GFL_HEAP_FreeMemory( pList );
}

//======================================================================
//	フィールドマップ　サブ　ゾーン更新
//======================================================================
//--------------------------------------------------------------
/**
 * 自機移動によるゾーン更新
 * @param fieldWork FIELDMAP_WORK
 * @retval BOOL TRUE=ゾーン更新発生
 */
//--------------------------------------------------------------
static BOOL fldmapMain_UpdateMoveZone( FIELDMAP_WORK *fieldWork )
{
	if( fldmap_CheckPlayerPosUpdate(fieldWork) == TRUE ){
		if( fldmap_CheckMoveZoneChange(fieldWork) == TRUE ){
			fldmap_ZoneChange( fieldWork );
			return( TRUE );
		}
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * 自機座標更新チェック
 * @param	fieldWork	FIELDMAP_WORK
 * @retval	BOOL	TRUE=座標更新
 */
//--------------------------------------------------------------
static BOOL fldmap_CheckPlayerPosUpdate( FIELDMAP_WORK *fieldWork )
{
	LOCATION *lc = &fieldWork->location;
	GAMEDATA *gdata = GAMESYSTEM_GetGameData( fieldWork->gsys );
	PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
	const VecFx32 *pos = PLAYERWORK_getPosition( player );
	
	if( pos->x != lc->pos.x || pos->z != lc->pos.z ){
		lc->pos = *pos;
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * ゾーン切り替えチェック 
 * @param	fieldWork	FIELDMAP_WORK
 * @retval	BOOL TRUE=ゾーン切り替え発生
 */
//--------------------------------------------------------------
static BOOL fldmap_CheckMoveZoneChange( FIELDMAP_WORK *fieldWork )
{
	LOCATION *lc = &fieldWork->location;
	MAP_MATRIX *mat = fieldWork->pMapMatrix;
	
	if( MAP_MATRIX_CheckVectorPosRange(mat,lc->pos.x,lc->pos.z) == TRUE ){
		u32 zone_id =
			MAP_MATRIX_GetVectorPosZoneID( mat, lc->pos.x, lc->pos.z );
		
		//現状 IDが無い場合は更新しない
		if( zone_id != MAP_MATRIX_ZONE_ID_NON ){
			if( zone_id != (u32)lc->zone_id ){
				return( TRUE );
			}
		}
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * ゾーン切り替え時の処理
 * @param	fieldWork	FIELDMAP_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fldmap_ZoneChange( FIELDMAP_WORK *fieldWork )
{
	LOCATION *lc = &fieldWork->location;
	GAMEDATA *gdata = GAMESYSTEM_GetGameData( fieldWork->gsys );
	EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );
	MMDLSYS *fmmdlsys = fieldWork->fldMMdlSys;
	
	MAP_MATRIX *mat = fieldWork->pMapMatrix;
	u32 new_zone_id = MAP_MATRIX_GetVectorPosZoneID(
			mat, lc->pos.x, lc->pos.z );
	u32 old_zone_id = lc->zone_id;
	
	GF_ASSERT( new_zone_id != MAP_MATRIX_ZONE_ID_NON );
	
	//旧ゾーン配置動作モデル削除
	MMDLSYS_DeleteZoneUpdateMMdl( fmmdlsys );
	
	//次のイベントデータをロード
	EVENTDATA_SYS_Load( evdata, new_zone_id );
	
	//新規ゾーンに配置する動作モデルセット
	zoneChange_SetMMdl( fmmdlsys, evdata, new_zone_id );
	
	//BGM切り替え
	zoneChange_SetBGM( gdata, new_zone_id );

	// ZONEフォグライト設定
	zoneChange_SetZoneFogLight( fieldWork, new_zone_id );
	
	//天候リクエスト
	zoneChange_SetWeather( fieldWork, new_zone_id );
	
	//PLAYER_WORK更新
	zoneChange_UpdatePlayerWork( gdata, new_zone_id );

	// 地名表示システムに, ゾーンの切り替えを通達
	FIELD_PLACE_NAME_ZoneChange( fieldWork->placeNameSys, new_zone_id );

	// ISSユニットにゾーンの切り替えを通達
	ISS_UNIT_ZoneChange( fieldWork->issUnit, new_zone_id );
	
	//ゾーンID更新
	lc->zone_id = new_zone_id;
	
	KAGAYA_Printf( "ゾーン更新完了 %d -> %d\n", lc->zone_id, new_zone_id );
}

//--------------------------------------------------------------
/**
 * ゾーン切り替え時の処理　新規ゾーン動作モデルセット
 * @param	fmmdlsys	MMDLSYS
 * @param	evdata	EVENTDATA_SYSTEM
 * @param	zone_id	次のゾーンID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void zoneChange_SetMMdl(
		MMDLSYS *fmmdlsys, EVENTDATA_SYSTEM *evdata, u32 zone_id )
{
	u16 count = EVENTDATA_GetNpcCount( evdata );
	
	if( count ){
		const MMDL_HEADER *header = EVENTDATA_GetNpcData( evdata );
		MMDLSYS_SetMMdl( fmmdlsys, header, zone_id, count );
	}
}

//--------------------------------------------------------------
/**
 * ゾーン切り替え時の処理　BGM切り替え
 * @param	gdata	GAMEDATA
 * @param	zone_id	次のゾーンID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void zoneChange_SetBGM( GAMEDATA *gdata, u32 zone_id )
{
	u16 trackBit = 0xfcff;	// track 9,10 OFF

	u16 nextBGM = ZONEDATA_GetBGMID(
			zone_id, GAMEDATA_GetSeasonID(gdata) );
	
	if( nextBGM != 0 ){
		if( PMSND_GetNextBGMsoundNo() != nextBGM ){
			PMSND_PlayNextBGM_EX( nextBGM, trackBit, 60, 0 );
		}
	}
}

//--------------------------------------------------------------
/**
 * ゾーン切り替え時の処理　天候リクエスト
 * @param	fieldWork	FIELDMAP_WORK*
 * @param	zone_id	次のゾーンID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void zoneChange_SetWeather( FIELDMAP_WORK *fieldWork, u32 zone_id )
{
	u32 w_no = ZONEDATA_GetWeatherID( zone_id );
	FIELD_WEATHER *we = FIELDMAP_GetFieldWeather( fieldWork );
	
	
	if( w_no != WEATHER_NO_NUM && w_no != FIELD_WEATHER_GetWeatherNo(we) ){
		FIELD_WEATHER_Change( we, w_no );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ゾーン切り替え時の処理	ゾーン用上書きFOG、ライト情報
 *
 *	@param	fieldWork	
 *	@param	zone_id 
 */
//-----------------------------------------------------------------------------
static void zoneChange_SetZoneFogLight( FIELDMAP_WORK *fieldWork, u32 zone_id )
{
	FIELD_ZONEFOGLIGHT* p_zonefog = FIELDMAP_GetFieldZoneFog( fieldWork );

	FIELD_ZONEFOGLIGHT_Clear( p_zonefog );
	
	FIELD_ZONEFOGLIGHT_Load( p_zonefog, FIELD_ZONEFOGLIGHT_DATA_NONE, FIELD_ZONEFOGLIGHT_DATA_NONE, fieldWork->heapID );
//	FIELD_ZONEFOGLIGHT_Load( p_zonefog, 0, 0, fieldWork->heapID );
}

//--------------------------------------------------------------
/**
 * ゾーン切り替え時の処理　PLAYER_WORK更新
 * @param	gdata	GAMEDATA
 * @param	zone_id	次のゾーンID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void zoneChange_UpdatePlayerWork( GAMEDATA *gdata, u32 zone_id )
{
	PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
	PLAYERWORK_setZoneID( player, zone_id );
}

//======================================================================
//	フィールドマップ　その他
//======================================================================
//--------------------------------------------------------------
/**
 * FIELDMAP_WORK マップコントロール用ワーククリア
 * @param fieldWork FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldmap_ClearMapCtrlWork( FIELDMAP_WORK *fieldWork )
{
	fieldWork->mapCtrlWork = NULL;
}

//==================================================================
/**
 * フィールドマップ作成と同時に起動する通信プログラム
 *
 * @param   gsys		
 * @param   fieldWork		
 */
//==================================================================
static void FIELDMAP_CommBoot(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork, HEAPID heapID)
{
  GAME_COMM_SYS_PTR game_comm;
  GAME_COMM_NO comm_no;
  GAMEDATA *gdata;
  
  game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  comm_no = GameCommSys_BootCheck(game_comm);
  gdata = GAMESYSTEM_GetGameData( gsys );
  
	switch(fieldWork->map_id){    //※check　どこかのタイミングで一度整理
	case ZONE_ID_UNION:
	case ZONE_ID_CLOSSEUM:
	  {
      UNION_PARENT_WORK *upw;
      
      if(comm_no != GAME_COMM_NO_UNION){
        upw = GFL_HEAP_AllocClearMemory(GFL_HEAP_LOWID(GFL_HEAPID_APP), sizeof(UNION_PARENT_WORK));
        upw->mystatus = GAMEDATA_GetMyStatus(gdata);
        upw->game_comm = game_comm;
        upw->game_data = gdata;
        if(comm_no == GAME_COMM_NO_NULL){
    	    GameCommSys_Boot(game_comm, GAME_COMM_NO_UNION, upw);
    	  }
    	  else{
          GameCommSys_ChangeReq(game_comm, GAME_COMM_NO_UNION, upw);
        }
      }
  	}
    break;
  default:  //パレス
  fieldWork->commSys = FIELD_COMM_MAIN_InitSystem( heapID, GFL_HEAPID_APP, GAMESYSTEM_GetGameCommSysPtr(gsys) );
  FIELD_COMM_MAIN_CommFieldMapInit(FIELD_COMM_MAIN_GetCommFieldSysPtr(fieldWork->commSys));
	FIELD_COMM_MAIN_SetCommActor(fieldWork->commSys,
      GAMEDATA_GetMMdlSys(GAMESYSTEM_GetGameData(gsys)));
//    fieldWork->commSys = FIELD_COMM_MAIN_InitSystem( heapID, GFL_HEAPID_APP, game_comm );
  //  FIELD_COMM_MAIN_CommFieldMapInit(FIELD_COMM_MAIN_GetCommFieldSysPtr(fieldWork->commSys));
  	//FIELD_COMM_MAIN_SetCommActor(fieldWork->commSys, GAMEDATA_GetMMdlSys(gdata));
    break;
  }
}

//======================================================================
//	data
//======================================================================
//--------------------------------------------------------------
/// ディスプレイ環境データ
//--------------------------------------------------------------
static const GFL_DISP_VRAM fldmapdata_dispVram =
{
	GX_VRAM_BG_128_D, //メイン2DエンジンのBGに割り当て 
	GX_VRAM_BGEXTPLTT_NONE, //メイン2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_SUB_BG_32_H, //サブ2DエンジンのBGに割り当て
	GX_VRAM_SUB_BGEXTPLTT_NONE, //サブ2DエンジンのBG拡張パレットに割り当て
	GX_VRAM_OBJ_64_E, //メイン2DエンジンのOBJに割り当て
	GX_VRAM_OBJEXTPLTT_NONE, //メイン2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_SUB_OBJ_16_I, //サブ2DエンジンのOBJに割り当て
	GX_VRAM_SUB_OBJEXTPLTT_NONE, //サブ2DエンジンのOBJ拡張パレットにに割り当て
	GX_VRAM_TEX_012_ABC, //テクスチャイメージスロットに割り当て
	GX_VRAM_TEXPLTT_0_G, //テクスチャパレットスロットに割り当て
	GX_OBJVRAMMODE_CHAR_1D_64K, // メインOBJマッピングモード
	GX_OBJVRAMMODE_CHAR_1D_32K, // サブOBJマッピングモード
};

//--------------------------------------------------------------
///	BGヘッダー
//--------------------------------------------------------------
static const GFL_BG_SYS_HEADER fldmapdata_bgsysHeader = {
	GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
};

//--------------------------------------------------------------
///	セルアクター　初期化データ
//--------------------------------------------------------------
static const GFL_CLSYS_INIT fldmapdata_CLSYS_Init =
{
	0, 0,
	0, 512,
	GFL_CLSYS_OAMMAN_INTERVAL, 128-GFL_CLSYS_OAMMAN_INTERVAL,
	GFL_CLSYS_OAMMAN_INTERVAL, 128-GFL_CLSYS_OAMMAN_INTERVAL, //通信アイコン部分
	0,
	FIELD_CLSYS_RESOUCE_MAX,
	FIELD_CLSYS_RESOUCE_MAX,
	FIELD_CLSYS_RESOUCE_MAX,
	FIELD_CLSYS_RESOUCE_MAX,
  16, 16,
};

//--------------------------------------------------------------
///	カメラ初期設定データ
//--------------------------------------------------------------
static const VecFx32 fldmapdata_cameraTarget	= { 0, 0, 0 };

static const VecFx32 fldmapdata_cameraPos	= {
	0, (FX32_ONE * 64), (FX32_ONE * 128)
};

//--------------------------------------------------------------
///	ライト初期設定データ
//--------------------------------------------------------------
static const GFL_G3D_LIGHT_DATA light0Tbl[] =
{
	{ 0, {{-(FX16_ONE-1),-(FX16_ONE-1),-(FX16_ONE-1)},GX_RGB(31,31,31)} },
	{ 1, {{(FX16_ONE-1),-(FX16_ONE-1),-(FX16_ONE-1)},GX_RGB(31,31,31)} },
	{ 2, {{-(FX16_ONE-1),-(FX16_ONE-1),-(FX16_ONE-1)},GX_RGB(31,31,31)} },
	{ 3, {{-(FX16_ONE-1),-(FX16_ONE-1),-(FX16_ONE-1)},GX_RGB(31,31,31)} },
};

static const GFL_G3D_LIGHTSET_SETUP fldmapdata_light0Setup = {
	light0Tbl, NELEMS(light0Tbl)
};

//--------------------------------------------------------------
///	3D エッジカラー
//--------------------------------------------------------------
static const GXRgb fldmapdata_edgeColorTable[8] = {
	GX_RGB(10,10,10),GX_RGB(10,10,10),GX_RGB(10,10,10),GX_RGB(10,10,10),
	GX_RGB(10,10,10),GX_RGB(10,10,10),GX_RGB(10,10,10),GX_RGB(10,10,10),
};

//--------------------------------------------------------------
//  3D  フォグカラー
//--------------------------------------------------------------
#define FOG_VALUE(v) (u32)(((v*16)<<0) | ((v*16+4)<<8) | ((v*16+8)<<16) | ((v*16+12)<<24))
static const u32 fldmapdata_fogColorTable[8] = {  
  FOG_VALUE(0),
  FOG_VALUE(1),
  FOG_VALUE(2),
  FOG_VALUE(3),
  FOG_VALUE(4),
  FOG_VALUE(5),
  FOG_VALUE(6),
  FOG_VALUE(7),
};
//--------------------------------------------------------------
//  BG  背景カラー
//--------------------------------------------------------------
static const u16 fldmapdata_bgColorTable[16] = { 
  FIELD_DEFAULT_BG_COLOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

