//======================================================================
/**
 * @file	fieldmap.c
 * @brief	フィールドマップ処理
 * @author
 *
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"					//GFL_HEAPID_APP
#include "system/rtc_tool.h"					//
#include "arc_def.h"

#include "net/network_define.h"
#include "system/g3d_tool.h"

#include "print/gf_font.h"

#include "field/field_const.h"
#include "fieldmap.h"
#include "field_bg_def.h"
#include "field_common.h"
#include "field_flagcontrol.h" //FIELD_FLAGCONT_INIT～

#include "field_player.h"
#include "field_camera.h"
#include "field_data.h"
#include "field/field_msgbg.h"
#include "field_encount.h"
#include "effect_encount.h"
#include "move_pokemon.h"

#include "weather.h"
#include "field_fog.h"
#include "field_light.h"
#include "field_buildmodel.h"
#include "field/field_nogrid_mapper.h"
#include "field/fld_scenearea.h"
#include "field/fld_scenearea_loader.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_event.h"
#include "gamesystem/pm_weather.h"
#include "field/zonedata.h"
#include "field/areadata.h"
#include "warpdata.h"   //ARRIVEDATA_SetArriveFlag

#include "field_comm_actor.h"
#include "field_comm/intrude_field.h"
#include "field_comm/intrude_work.h"

#include "fldmmdl.h"

#include "field_debug.h"

#include "field/map_matrix.h"
#include "field_event_check.h"  

#include "eventdata_local.h"
#include "field/eventdata_sxy.h"

#include "field_effect.h"


#include "fieldskill_mapeff.h"

#include "fieldmap_func.h"
#include "field/fieldmap_call.h"

#include "arc/fieldmap/zone_id.h"
#include "field/weather_no.h"
#include "sound/pm_sndsys.h"
//#include "sound/wb_sound_data.sadl"
#include "net_app/union/union_main.h"

#include "debug/debugwin_sys.h" //デバッグメニュー

#if USE_DEBUGWIN_SYSTEM
#include "mapdatafunc/field_func_random_generate.h" //デバッグ登録のため
#endif //USE_DEBUGWIN_SYSTEM

#include "field_place_name.h"
#include "field_palace_sys.h"

#include "field_sound.h"

#include "fieldmap_ctrl_grid.h"
#include "fieldmap_ctrl_nogrid_work.h"
#include "fieldmap_ctrl_hybrid.h"

#include "field_gimmick.h"
#include "gmk_tmp_wk.h"

#include "field_status_local.h" //FIELD_STATUS_～
#include "script.h"   //SCRIPT_CallZoneChangeScript

#include "fld_particle.h"

#include "field_wfbc.h"
#include "fld_vreq.h"

#include "savedata/sodateya_work.h"
#include "sodateya.h"
#include "system/net_err.h"

#include "field_task.h"  
#include "field_task_manager.h"
#include "ev_time.h"  //EVTIME_Update
#include "field_bbd_color.h"

#include "field_crowd_people.h"  

#include "net_app/union_eff.h"

#include "calender.h"

#include "savedata/record.h"

#include "pleasure_boat.h"    //for PL_BOAT_

//======================================================================
//	DEBUG定義
//======================================================================

#define DEBUG_SPEED_CHECK_ENABLE  //デバッグ速度計測を有効にする
#include "debug_speed_check.h"  //デバッグ速度計測本体
#include "debug/debug_flg.h"
#ifdef PM_DEBUG

#include "field_camera_debug.h"
#include "test/performance.h"

//#define DEBUG_FIELDMAP_SETUP_SPEED_CHECK  //setupでの処理負荷を表示
//#define DEBUG_FIELDMAP_INOUT_SPEED_CHECK  //FIELDMAP出入での処理負荷を表示
//#define DEBUG_FIELDMAP_DRAW_MICRO_SECOND_CHECK    // フィールドマップ描画にかかる処理時間を求める
//#define DEBUG_FIELDMAP_UPDATETOP_SPEED_CHECK  //update_topでの処理負荷を見る
//#define DEBUG_FIELDMAP_UPDATETAIL_SPEED_CHECK  //update_tailでの処理負荷を見る
//#define DEBUG_FIELDMAP_ZONE_CHANGE_SYNC    // ゾーンチェンジに必要なシンク数を監視


// フィールドマップ描画にかかる処理時間を求める
#ifdef DEBUG_FIELDMAP_DRAW_MICRO_SECOND_CHECK
#define DEBUG_FIELDMAP_DRAW_MICRO_SECOND_CHECK_DRAW_KEY ( PAD_BUTTON_L )
#endif //DEBUG_FIELDMAP_DRAW_MICRO_SECOND_CHECK

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
extern u8 DEBUG_MAIN_UPDATE_TYPE;  ///<FIELDMAP TOP TAIL フレームチェック用 実態は、main.c
#endif


static void DbgVramDumpCallBack(u32 addr, u32 szByte, void* pUserData);
#endif  //PM_DEBUG

//======================================================================
//	define
//======================================================================

#ifdef PM_DEBUG
extern BOOL DebugBGInitEnd;    //BG初期化監視フラグ
#endif

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
  ///テクスチャ使用サイズ指定
  FIELD_3D_VRAM_SIZE    =   GFL_G3D_TEX384K,  // 256Kから384Kに変更 tomoya

  ///パレット使用サイズ指定
  FIELD_3D_PLTT_SIZE    =   GFL_G3D_PLT16K,
  
  ///DTCMエリアのサイズ
  FIELD_3D_DTCM_SIZE    =   0x1000, 
  
  ///billboardActで使用するリソースの最大設定可能数
  FIELD_G3D_BBDACT_RESMAX	    =   64,
  ///billboardActで使用するオブジェクトの最大設定可能数
  FIELD_G3D_BBDACT_ACTMAX	    =   128,

  FIELD_G3D_SUBBBDACT_RESMAX  =   58,
  FIELD_G3D_SUBBBDACT_ACTMAX  =   80,
  
  FIELD_G3D_EFFBBDACT_RESMAX  =   30,
  FIELD_G3D_EFFBBDACT_ACTMAX  =   40,
  
  ///<セルアクターリソース最大
  FIELD_CLSYS_RESOUCE_MAX		  =   64,
  
  ///フォグ色
  FIELD_DEFAULT_FOG_COLOR		  =   (GX_RGB(31,31,31)),
  ///背景色
  FIELD_DEFAULT_BG_COLOR	    =   (GX_RGB(30,31,31)),
  
  ///FLD_G3DOBJ リソース最大
  FIELD_G3DOBJ_RES_MAX = 64,
  ///FLD_G3DOBJ オブジェクト最大
  FIELD_G3DOBJ_OBJ_MAX = 64,

  ///メインビルボード　移りこみTフリップ　テクスチャT座標　補正値
  FIELD_G3D_BBD_FLIP_TEX_OFFS_T = -(FX16_HALF+0x200),
};

///g3Dutilで使用するリソースの最大設定可能数
#define G3D_UTIL_RESCOUNT	(512)
///g3Dutilで使用するオブジェクトの最大設定可能数
#define G3D_UTIL_OBJCOUNT	(128)

#define MMDL_LIST_MAX	(64) ///<動作モデルリスト最大

/// FLDMAPFUNCシステムのタスク最大数
#define FLDMAPFUNC_TASK_MAX	( 32 )

//ギミックテンポラリワーク未アサインＩＤ
#define GMK_TMP_NO_ASSIGN_ID  (0xffffffff)
//拡張ＯＢＪリソース最大登録数
#define EXP_OBJ_RES_MAX   (60)
//拡張ＯＢＪ登録最大数
#define EXP_OBJ_MAX   (35)


///動作モデル描画時の高さ係数
enum {
  MMDL_BBD_OFFS_Y_VALUE = FX32_CONST(4),
  MMDL_BBD_OFFS_POS_Y   = FX32_CONST(-4),
};

//プロジェクションマトリクスを操作する際のＺオフセット
enum {
  ///通常の射影行列補正値
  PROJ_MAT_Z_OFS        =	(310+0xb8),

  ///砂漠地形での射影行列補正値
  PROJ_MAT_Z_OFS_DESERT = 310,
};

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

//フィールド用ギミックテンポラリワーク
typedef struct GMK_TMP_WORK_tag
{
  u32 AssignID;
  void * Work;
}GMK_TMP_WORK;

//--------------------------------------------------------------
///	FIELDMAP_WORK
//--------------------------------------------------------------
struct _FIELDMAP_WORK
{
	HEAPID heapID;
	GAMESYS_WORK *gsys;
  GAMEDATA * gamedata;

  AREADATA * areadata;
	
	FIELD_CAMERA *camera_control;
	FIELD_LIGHT *light;
	FIELD_FOG_WORK *fog;
	FIELD_ZONEFOGLIGHT	 *zonefog;
	
	FIELD_WEATHER *weather_sys;
  FIELD_SUBSCREEN_WORK* fieldSubscreenWork;
	
	FLDMSGBG *fldMsgBG;
  FLDMSGWIN *goldMsgWin;  // 所持金表示用ウィンドウ

	FIELD_PLACE_NAME* placeNameSys;	// 地名表示ウィンドウ
  FLD_EXP_OBJ_CNT_PTR ExpObjCntPtr;
	
	MMDLSYS *fldMMdlSys;

  FLDNOGRID_MAPPER* nogridMapper;
  FLD_SCENEAREA* sceneArea;
  FLD_SCENEAREA_LOADER* sceneAreaLoader;
	
	FLDMAPPER *g3Dmapper;
	MAP_MATRIX *pMapMatrix;
	FLDMAPPER_RESISTDATA map_res;
	FIELD_PLAYER *field_player;
	FIELD_ENCOUNT *encount;
	FLDEFF_CTRL *fldeff_ctrl;
	UNION_EFF_SYSTEM *union_eff;    //ユニオンエフェクト制御システム
  FIELD_PALACE_SYS *palace_sys;  //侵入系ワーク

	FIELDSKILL_MAPEFF * fieldskill_mapeff;
  

  GFL_G3D_CAMERA *g3Dcamera; //g3Dcamera Lib ハンドル
	GFL_G3D_LIGHTSET *g3Dlightset; //g3Dlight Lib ハンドル
	GFL_TCB *g3dVintr; //3D用vIntrTaskハンドル
	GFL_BBDACT_SYS *mainBbdActSys; //ビルボードアクトシステム設定ハンドル
  GFL_BBDACT_SYS *subBbdActSys;
  GFL_BBDACT_SYS *effBbdActSys;
  
  FLDMAPFUNC_SYS * fldmapFuncSys;
	
	GAMEMODE gamemode;
	FLDMAPSEQ seq;
  u8 subseq;
  u8 seq_switch;
	int timer;
	u16 map_id;
	LOCATION location;
	
  const VecFx32 *target_now_pos_p;
	VecFx32 now_pos;
  
  fx32 bbdact_y_offs;      ///<動作モデルBLACT描画時のYオフセット

  fx32 pro_mat_z_offs;    ///<描画時の射影に対する補正値

	const DEPEND_FUNCTIONS *func_tbl;
	void *mapCtrlWork;
	

  GFL_TCBSYS* fieldmapTCB;
  void* fieldmapTCBSysWork;
  GMK_TMP_WORK  GmkTmpWork;     //ギミックテンポラリワークポインタ

  FLD_PRTCL_SYS_PTR FldPrtclSys;
  FLD3D_CI_PTR Fld3dCiPtr;

  SODATEYA* sodateya;  // 育て屋

  DRAW3DMODE Draw3DMode;
  
  FLD_G3DOBJ_CTRL *fieldG3dObjCtrl;

  FIELD_TASK_MAN* taskManager;  // タスクマネージャ

  BOOL MapFadeReq;     //マップ遷移用フェードリクエスト

  ENCEFF_CNT_PTR EncEffCntPtr;

  BOOL MainHookFlg;

  BOOL seasonDispFlag; // 季節を表示中かどうか

  FACEUP_WK_PTR FaceUpWkPtr;

  FLD_VREQ* fldVReq;  ///<フィールドVBlank反映

  FIELD_CROWD_PEOPLE* crowdpeople; ///<群集ワーク/タスク生成時にポインタを設定 field_event_check.c内で、イベント発生時に騒音windowを消すため。

#ifdef  PM_DEBUG
	FIELD_DEBUG_WORK *debugWork;
#endif
};

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
static void fldmap_G3D_Draw_top( FIELDMAP_WORK * fieldWork );
static void fldmap_G3D_Draw_tail( FIELDMAP_WORK * fieldWork );
static void fldmap_G3D_Unload( FIELDMAP_WORK * fieldWork );
static void	fldmap_G3D_VBlank( GFL_TCB* tcb, void* work );
static void	fldmap_G3D_BBDTrans(
		GFL_BBDACT_TRANSTYPE type, u32 dst, u32 src, u32 siz );
static void FIELD_EDGEMARK_Setup(const AREADATA * areadata, const u16 zone_id);
static void fieldmap_G3D_BBDSetColorParam( FIELDMAP_WORK * fieldWork );

//fldmmdl
static void fldmapMain_MMDL_Init( FIELDMAP_WORK *fieldWork );
static void fldmapMain_MMDL_Finish( FIELDMAP_WORK *fieldWork );
static void fldmap_MMDL_InitList(
		MMDL_LIST *mlist, int list_id, HEAPID heapID );
static fx32 calcBbdActYOffset( const FIELDMAP_WORK * fieldmap );

//zonechange
static BOOL fldmapMain_UpdateMoveZone( FIELDMAP_WORK *fieldWork );
static BOOL fldmap_CheckPlayerPosUpdate( FIELDMAP_WORK *fieldWork );
static BOOL fldmap_CheckMoveZoneChange( FIELDMAP_WORK *fieldWork );
static void fldmap_ZoneChange( FIELDMAP_WORK *fieldWork );

static void zoneChange_SetMMdl( GAMEDATA *gdata,
		MMDLSYS *fmmdlsys, EVENTDATA_SYSTEM *evdata, u32 zone_id );
static void zoneChange_SetBGM( FIELDMAP_WORK* fieldWork, u32 next_zone_id );
static void zoneChange_SetWeather( FIELDMAP_WORK *fieldWork, u32 zone_id );
static void zoneChange_SetZoneFogLight( FIELDMAP_WORK *fieldWork, u32 zone_id );
static void zoneChange_UpdatePlayerWork( GAMEDATA *gdata, u32 zone_id );
static void zoneChange_SetCameraArea( FIELDMAP_WORK* fieldWork, u32 zone_id );

static void zoneChangeScene( FIELDMAP_WORK *fieldWork, u32 zone_id );

//etc
static void fldmap_ClearMapCtrlWork( FIELDMAP_WORK *fieldWork );
static void setupCameraArea( FIELDMAP_WORK *fieldWork, u32 zone_id, HEAPID heapID );
static void setupWfbc( GAMEDATA* gdata, FIELDMAP_WORK *fieldWork, u32 zone_id );

static fx32 fldmap_getProjMatZOffsValue( u16 zone_id );

static BOOL zonedata_GetShadowUse( u16 zone_id );

static BOOL zonedata_IsEnvSE( u16 zone_id );


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

static void InitGmkTmpWork(GMK_TMP_WORK *tmpWork);

//3Ｄ描画モード
static void Draw3DNormalMode_top( FIELDMAP_WORK * fieldWork );
static void Draw3DNormalMode_tail( FIELDMAP_WORK * fieldWork );
static void Draw3DCutinMode(FIELDMAP_WORK * fieldWork);
//static void Draw3DScrnTexMode(FIELDMAP_WORK * fieldWork);
static void DrawEncEff(FIELDMAP_WORK * fieldWork);

static void DrawTop(FIELDMAP_WORK *fieldWork);

typedef void (*DRAW3DMODE_FUNC)(FIELDMAP_WORK * fieldWork);

//--------------------------------------------------------------
//  オーバーレイID
//--------------------------------------------------------------
FS_EXTERN_OVERLAY(union_room);
FS_EXTERN_OVERLAY(field_intrude);


//======================================================================
//======================================================================
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
  fieldWork->gamedata = GAMESYSTEM_GetGameData(gsys);
	
	{
		PLAYER_WORK *pw = GAMESYSTEM_GetMyPlayerWork(gsys);
		fieldWork->map_id = PLAYERWORK_getZoneID( pw );
	}
  // 地面、見た目にかかわる情報には、進入の情報を関係付ける。
  {
    fieldWork->areadata = AREADATA_Create( heapID,
        ZONEDATA_GetAreaID(fieldWork->map_id), PMSEASON_GetConsiderCommSeason(gsys) );
  }
	
	//マップコントロール
	fieldWork->func_tbl = FIELDDATA_GetFieldFunctions( fieldWork->map_id );


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

  //ワーク開放
	GFL_HEAP_FreeMemory( fieldWork );
  
  //GAMESYSTEM内で保持しているワークをNULLに
  GAMESYSTEM_SetFieldMapWork( gsys, NULL );

  //通信アイコン強制破棄
  GFL_NET_WirelessIconEasyEnd();
  GFL_NET_WirelessSetForceXYPos(0,0); //座標固定リセット
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
	

	//VRAMクリア
	//GFL_DISP_ClearVRAM( GX_VRAM_D );
	//VRAM設定
	GFL_DISP_SetBank( &fldmapdata_dispVram );

	//BG初期化
	fldmap_BG_Init( fieldWork );

	// CLACT初期化
	GFL_CLACT_SYS_Create(
			&fldmapdata_CLSYS_Init, &fldmapdata_dispVram, HEAPID_FIELD_PRBUF );
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

	//３Ｄデータのロード
	fldmap_G3D_Load( fieldWork );
	
	//配置物設定
	fieldWork->g3Dmapper = FLDMAPPER_Create( fieldWork->heapID, GAMEDATA_GetSeasonID(fieldWork->gamedata) );

	//VBlankTCB
	fieldWork->g3dVintr =
		GFUser_VIntr_CreateTCB( fldmap_G3D_VBlank, (void*)fieldWork, 0 );

  // TCB
  {
    u32 task_max = 32;
    u32     size = GFL_TCB_CalcSystemWorkSize( task_max );
    fieldWork->fieldmapTCBSysWork = GFL_HEAP_AllocMemory( heapID, size );
    fieldWork->fieldmapTCB = GFL_TCB_Init( task_max, fieldWork->fieldmapTCBSysWork );
  }

  // タスクマネージャ
  fieldWork->taskManager = FIELD_TASK_MAN_Create( 10, fieldWork->heapID );

  // VBLANK反映管理
  fieldWork->fldVReq = FLD_VREQ_Create( fieldWork->heapID );

  //ユニオンと侵入で読み込むオーバーレイを分ける
  if( ZONEDATA_IsUnionRoom( fieldWork->map_id ) == TRUE
      || ZONEDATA_IsColosseum( fieldWork->map_id ) == TRUE){
    GFL_OVERLAY_Load( FS_OVERLAY_ID( union_room ) );
  }
  else{
    GFL_OVERLAY_Load( FS_OVERLAY_ID( field_intrude ) );
  }

	GFL_UI_StartFrameRateMode( GFL_UI_FRAMERATE_30 );

  return MAINSEQ_RESULT_NEXTSEQ;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static MAINSEQ_RESULT mainSeqFunc_setup(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  BOOL continue_flag = TRUE;

  INIT_CHECK();

  switch( fieldWork->subseq )
  {
  case 0:
    //フラグ操作：フィールドマップ生成タイミング
    FIELD_FLAGCONT_INIT_FieldIn( gdata, fieldWork->map_id );

    fieldWork->fldMsgBG = FLDMSGBG_Create( fieldWork->heapID, fieldWork->g3Dcamera );
    fieldWork->goldMsgWin = NULL;

    SET_CHECK("setup: place name");  //デバッグ：処理負荷計測
    // 地名表示システム作成
    fieldWork->placeNameSys = FIELD_PLACE_NAME_Create( gsys, HEAPID_PLACE_NAME, fieldWork->fldMsgBG );

    SET_CHECK("setup: camera & scene");  //デバッグ：処理負荷計測
    // フラッシュチェック
    {
      FIELD_STATUS * fldstatus = GAMEDATA_GetFieldStatus( fieldWork->gamedata );
      fieldWork->fieldskill_mapeff = FIELDSKILL_MAPEFF_Create( 
          FIELD_STATUS_GetFieldSkillMapEffectMsk(fldstatus), fieldWork->heapID ); 

      // 整合性チェック
#ifdef PM_DEBUG
      {
        u32 msk = FIELD_STATUS_GetFieldSkillMapEffectMsk(fldstatus);
        BOOL flash = FIELD_STATUS_IsFieldSkillFlash(fldstatus);
        msk = FIELDSKILL_MAPEFF_MSK_IS_ON( msk, FIELDSKILL_MAPEFF_MSK_FLASH_FAR );
        if( msk )
        {
          msk = TRUE;
        }
        GF_ASSERT( msk == flash );
      }
#endif
    }

    fieldWork->camera_control = FIELD_CAMERA_Create(
        ZONEDATA_GetCameraID(fieldWork->map_id),
        FIELD_CAMERA_MODE_CALC_CAMERA_POS,
        fieldWork->g3Dcamera,
        &fieldWork->now_pos,
        fieldWork->heapID );
    
    // シーンエリア
    fieldWork->sceneArea        = FLD_SCENEAREA_Create( fieldWork->heapID, fieldWork->camera_control, fieldWork );
    fieldWork->sceneAreaLoader  = FLD_SCENEAREA_LOADER_Create( fieldWork->heapID );

    break;

  case 1:
    SET_CHECK("setup: nogrid mapper");  //デバッグ：処理負荷計測
    // NOGRIDマッパー生成
    fieldWork->nogridMapper = FLDNOGRID_MAPPER_Create( fieldWork->heapID, fieldWork->camera_control, fieldWork->sceneArea, fieldWork->sceneAreaLoader );
    
    //進入用グレースケール管理作成
    fieldWork->palace_sys = FIELD_PALACE_Create( fieldWork->heapID, gsys , fieldWork , fieldWork->map_id );
    
    SET_CHECK("setup: fldmapper");  //デバッグ：処理負荷計測
    {
      FIELD_BMODEL_MAN * bmodel_man = FLDMAPPER_GetBuildModelManager( fieldWork->g3Dmapper );
      
      FIELDDATA_SetMapperData(fieldWork->map_id,
          fieldWork->areadata,
          &fieldWork->map_res,
          GAMEDATA_GetMapMatrix(fieldWork->gamedata) );

    SET_CHECK("setup: bmodel load");  //デバッグ：処理負荷計測
      //ここで配置モデルリストをセットする
      {
        u8 *gray_scale;
        GAME_COMM_SYS_PTR game_comm;
        game_comm = GAMESYSTEM_GetGameCommSysPtr( fieldWork->gsys );
        gray_scale  = FIELD_PALACE_GetShadeTable( fieldWork->palace_sys );
        FIELD_BMODEL_MAN_Load(bmodel_man, fieldWork->map_id, fieldWork->areadata, gray_scale);

        //G3DOBJにグレースケール適用
        FLD_G3DOBJ_CTRL_SetGrayScale( fieldWork->fieldG3dObjCtrl, gray_scale );
      }
    }

    // WFBC街情報を設定 影を操作するためにギミックのセットアップに位置を変更
    setupWfbc( gdata, fieldWork, fieldWork->map_id );
    break;

  case 2:
    SET_CHECK("setup: resistdata");  //デバッグ：処理負荷計測
    //フィールドマップ用ロケーション作成

    LOCATION_Set( &fieldWork->location, fieldWork->map_id, 0, 0, LOCATION_DEFAULT_EXIT_OFS,
        fieldWork->now_pos.x, fieldWork->now_pos.y, fieldWork->now_pos.z );

    
    //マップデータ登録
    {
      u8 *gray_scale;
      GAME_COMM_SYS_PTR game_comm;
      game_comm = GAMESYSTEM_GetGameCommSysPtr( fieldWork->gsys );
      gray_scale  = FIELD_PALACE_GetShadeTable( fieldWork->palace_sys );
      FLDMAPPER_ResistData( fieldWork->g3Dmapper, &fieldWork->map_res, gray_scale );
    }

    //NOGRIDマップデータ登録
    {
      u32 raildata = ZONEDATA_GetRailDataID(fieldWork->map_id);
      if ( raildata != ZONEDATA_NO_RAILDATA_ID ){
        FLDNOGRID_MAPPER_ResistDataArc( fieldWork->nogridMapper, raildata, fieldWork->heapID );  
      }
    }
    //描画時の射影に対する補正値
    fieldWork->pro_mat_z_offs = fldmap_getProjMatZOffsValue( fieldWork->map_id );

    SET_CHECK("setup: cameraArea");  //デバッグ：処理負荷計測

    //CAMERA_AREAの反映
    setupCameraArea( fieldWork, fieldWork->map_id, fieldWork->heapID );
    break;

  case 3:
    SET_CHECK("setup: mmdl init");  //デバッグ：処理負荷計測
    //動作モデル初期化
    fldmapMain_MMDL_Init(fieldWork);
    
    SET_CHECK("setup: fldeff sys");  //デバッグ：処理負荷計測
    //フィールドエフェクト初期化
    fieldWork->fldeff_ctrl = FLDEFF_CTRL_Create(
        fieldWork, FLDEFF_PROCID_MAX, fieldWork->heapID );
    
    //フィールドエフェクト　パラメタ設定　タスク
    FLDEFF_CTRL_SetTaskParam( fieldWork->fldeff_ctrl, FLDEFF_TASK_MAX );
    
    SET_CHECK("setup: fldeff grayscale");  //デバッグ：処理負荷計測
    //フィールドエフェクト　パラメタ設定 グレースケール
    {
      u8 *gray_scale = FIELD_PALACE_GetShadeTable( fieldWork->palace_sys );
      FLDEFF_CTRL_SetGrayScaleParam( fieldWork->fldeff_ctrl, gray_scale );
    }
    
    SET_CHECK("setup: fldeff regist");  //デバッグ：処理負荷計測
    //フィールドエフェクト　登録
    FLDEFF_CTRL_RegistEffect( fieldWork->fldeff_ctrl,
      DATA_FLDEFF_RegistEffectGroundTbl, DATA_FLDEFF_RegistEffectGroundTblNum );

    //ユニオンルームならユニオンエフェクト制御システムを作成
    if( ZONEDATA_IsUnionRoom( fieldWork->map_id ) == TRUE
        || ZONEDATA_IsColosseum( fieldWork->map_id ) == TRUE ){
      fieldWork->union_eff = UNION_EFF_SystemSetup(fieldWork->heapID);
    }
    
    break;

  case 4:
    SET_CHECK("setup: create_func");  //デバッグ：処理負荷計測
    {
      PLAYER_WORK *pw = GAMESYSTEM_GetMyPlayerWork(gsys);
      const u16 dir = PLAYERWORK_getDirection_Type( pw );
      const VecFx32 *pos = &pw->position;
      
      //自機作成
      {
        //自転車禁止な場所であれば事前に自転車形態を解除
        if( PLAYERWORK_GetMoveForm(pw) == PLAYER_MOVE_FORM_CYCLE ){
          if( ZONEDATA_BicycleEnable(fieldWork->map_id) == FALSE ){
            PLAYERWORK_SetMoveForm(pw,PLAYER_MOVE_FORM_NORMAL);
          }
        }
        
        {
          MYSTATUS *mystatus = GAMEDATA_GetMyStatus( gdata );
          int sex = MyStatus_GetMySex( mystatus );
          fieldWork->field_player = FIELD_PLAYER_Create(
              pw, fieldWork, pos, sex, fieldWork->heapID );
        }
      }
      
      //登録テーブルごとに個別の初期化処理を呼び出し
      fieldWork->now_pos = *pos;
      fieldWork->func_tbl->create_func(fieldWork, &fieldWork->now_pos, dir);

      FLDMAPPER_SetPos( fieldWork->g3Dmapper, &fieldWork->now_pos );
      
      TAMADA_Printf("start X,Y,Z=%d,%d,%d\n", FX_Whole(pos->x), FX_Whole(pos->y), FX_Whole(pos->z));
      TAMADA_Printf( "Start Dir = %04x\n", pw->direction );
    }
    SET_CHECK("setup: gimmick sys");  //デバッグ：処理負荷計測

    //ギミックテンポラリワーク初期化
    InitGmkTmpWork(&fieldWork->GmkTmpWork);

    //拡張3Ｄオブジェクトモジュール作成
    fieldWork->ExpObjCntPtr = FLD_EXP_OBJ_Create ( EXP_OBJ_RES_MAX, EXP_OBJ_MAX, fieldWork->heapID );
    
    //エッジマーキング設定セットアップ
    FIELD_EDGEMARK_Setup( fieldWork->areadata, FIELDMAP_GetZoneID( fieldWork ) );

    SET_CHECK("setup: fogs");  //デバッグ：処理負荷計測
    // フォグシステム生成
    fieldWork->fog	= FIELD_FOG_Create( fieldWork->heapID );
    
    // ゾーンフォグシステム生成
    fieldWork->zonefog = FIELD_ZONEFOGLIGHT_Create( fieldWork->heapID );
    FIELD_ZONEFOGLIGHT_LoadReq( fieldWork->zonefog, ZONEDATA_GetFog(fieldWork->map_id), ZONEDATA_GetLight(fieldWork->map_id), 1 );

    SET_CHECK("setup: light");  //デバッグ：処理負荷計測
    // ライトシステム生成
    {
      TOMOYA_Printf( "進入を含んだ　シーズン %d 通常のシーズン %d\n", PMSEASON_GetConsiderCommSeason(gsys), GAMEDATA_GetSeasonID( fieldWork->gamedata ) );
      fieldWork->light = FIELD_LIGHT_Create( AREADATA_GetLightType( fieldWork->areadata ), 
          GFL_RTC_GetTimeBySecond(), 
          PMSEASON_GetConsiderCommSeason(gsys),
          fieldWork->fog, fieldWork->g3Dlightset, fieldWork->heapID );
    }

    SET_CHECK("setup: weather");  //デバッグ：処理負荷計測
    // 天気システム生成
    {
      FIELD_SOUND* fsnd;
      fsnd = GAMEDATA_GetFieldSound( fieldWork->gamedata );
      
      fieldWork->weather_sys = FIELD_WEATHER_Init(
          fieldWork->camera_control,
          fieldWork->light,
          fieldWork->fog,
          fieldWork->zonefog, 
          fsnd,
          PMSEASON_GetConsiderCommSeason(gsys),
          HEAPID_WEATHER ); // プログラムヒープを使用する。
    }
    
    // 天気晴れ
    FIELD_WEATHER_Set(
        fieldWork->weather_sys, GAMEDATA_GetWeatherNo( fieldWork->gamedata ), HEAPID_WEATHER );
    
    SET_CHECK("setup: gimmick setup");  //デバッグ：処理負荷計測
    //フィールドギミックセットアップ
    FLDGMK_SetUpFieldGimmick(fieldWork);
    break;

  case 5:
    
    SET_CHECK("setup: subscreen");  //デバッグ：処理負荷計測
    //情報バーの初期化
    {
      fieldWork->fieldSubscreenWork = FIELD_SUBSCREEN_Init(fieldWork->heapID, fieldWork, GAMEDATA_GetSubScreenMode(gdata));
    }
    break;
  case 6:
    
    SET_CHECK("setup: 3dsystem");  //デバッグ：処理負荷計測
    //フィールドエンカウント初期化
    fieldWork->encount = FIELD_ENCOUNT_Create( fieldWork );
    
    //通信初期化コールバック呼び出し
    {
      GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
      GameCommSys_Callback_FieldCreate( game_comm, fieldWork );
    }

    // フィールドマップ用制御タスクシステム
    fieldWork->fldmapFuncSys = FLDMAPFUNC_Sys_Create( fieldWork, fieldWork->heapID, FLDMAPFUNC_TASK_MAX );

#ifdef  PM_DEBUG
    fieldWork->debugWork = NULL;
#endif

    //フィールドパーティクル
    fieldWork->FldPrtclSys = FLD_PRTCL_Init(HEAPID_FLD3DCUTIN);
    //フィールド3Dカットイン
    fieldWork->Fld3dCiPtr = FLD3D_CI_Init(HEAPID_FLD3DCUTIN, fieldWork->FldPrtclSys);
    //エンカウントエフェクト
    fieldWork->EncEffCntPtr = ENCEFF_CreateCntPtr(fieldWork->heapID, fieldWork);

    // 育て屋
    {
      SAVE_CONTROL_WORK* scw = GAMEDATA_GetSaveControlWork( fieldWork->gamedata );
      SODATEYA_WORK* work = SODATEYA_WORK_GetSodateyaWork( scw );
      fieldWork->sodateya = SODATEYA_Create( fieldWork->heapID, fieldWork, work );
    }
    
    // scenearea
    {
      zoneChangeScene( fieldWork, fieldWork->map_id );
    }
    continue_flag = FALSE;  //setup処理の最後！
    break;
  }
  fieldWork->subseq ++;

  SET_CHECK("setup: tail");  //デバッグ：処理負荷計測
#ifdef  DEBUG_FIELDMAP_SETUP_SPEED_CHECK
  {
    OSTick end_tick;
    TAIL_CHECK(&end_tick);
    OS_Printf("mainSeqFunc_setup:total %ld\n", OS_TicksToMicroSeconds( end_tick ) );
    PUT_CHECK();
  }
#endif
  
  //3Ｄ描画モードは通常でセットアップ
  fieldWork->Draw3DMode = DRAW3DMODE_NORMAL;


  if ( continue_flag ) {
    return MAINSEQ_RESULT_CONTINUE;
  } else {
    return MAINSEQ_RESULT_NEXTSEQ;
  }
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static MAINSEQ_RESULT mainSeqFunc_ready(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{ 
#ifdef  DEBUG_FIELDMAP_INOUT_SPEED_CHECK
  OS_TPrintf("mainSeqFunc_ready\n");
#endif

  switch ( fieldWork->subseq )
  {
  case 0:
    //FLDMAPPER 一括読み込み
    FLDMAPPER_AllSetUp( fieldWork->g3Dmapper );

    if( fieldWork->fldMMdlSys != NULL ){
      do {
        MMDLSYS_UpdateProc( fieldWork->fldMMdlSys );
        //VBlank期間用関数だが、暗転中にVRAM登録処理するだけなので副作用がない
        MMDL_BLACTCONT_ProcVBlank( fieldWork->fldMMdlSys );
      } while ( MMDL_BLACTCONT_IsThereReserve(fieldWork->fldMMdlSys) == TRUE );
    }

    // 天気待ち
    while( FIELD_WEATHER_IsLoading(fieldWork->weather_sys) ){
      FIELD_WEATHER_Main( fieldWork->weather_sys, HEAPID_WEATHER );
    }

    fldmap_G3D_Control( fieldWork );
    fldmap_G3D_Draw_top( fieldWork );
    fldmap_G3D_Draw_tail( fieldWork );
    GFL_CLACT_SYS_Main(); // CLSYSメイン
    
    if(fieldWork->fldMsgBG){ FLDMSGBG_PrintMain( fieldWork->fldMsgBG ); }
    
#ifdef  PM_DEBUG
    if(fieldWork->debugWork){ FIELD_DEBUG_UpdateProc( fieldWork->debugWork ); }
#endif

    FLDEFF_CTRL_Update( fieldWork->fldeff_ctrl );

    // フィールドマップ用制御タスクシステム
    FLDMAPFUNC_Sys_Main( fieldWork->fldmapFuncSys );


    fieldWork->subseq ++;
    /* FALL THROUGH */

  case 1:
    //2010.06.20  tamada
    //「つづきから」の場合、四天王部屋などで環境音的なSEの開始時に
    //読み込み中で再生できないという問題がある。
    //その対処のため、ここで分割ロードの完了を待つ
    if ( FIELD_STATUS_GetContinueFlag( GAMEDATA_GetFieldStatus( fieldWork->gamedata ) ) == TRUE )
    {
      if (zonedata_IsEnvSE( fieldWork->map_id ) == TRUE )
      { //環境音SEを必要とするマップだけ、ここで読み込み待ちを行う
        if ( PMSND_IsLoading() )
        {
          TAMADA_Printf( " BGM Load Wait!!\n" );
          return MAINSEQ_RESULT_CONTINUE;
        }
      }
    }
    
    { //フィールド初期化スクリプトの呼び出し
      FIELD_STATUS * fldstatus = GAMEDATA_GetFieldStatus( fieldWork->gamedata );
      if ( FIELD_STATUS_GetFieldInitFlag( fldstatus ) ) 
      {
        SCRIPT_CallFieldInitScript( fieldWork->gsys, fieldWork->heapID );
        FIELD_STATUS_SetFieldInitFlag( GAMEDATA_GetFieldStatus( fieldWork->gamedata ), FALSE );
      } else {
        SCRIPT_CallFieldRecoverScript( fieldWork->gsys, fieldWork->heapID );
      }
    }
    FIELD_STATUS_SetContinueFlag( GAMEDATA_GetFieldStatus( fieldWork->gamedata ), FALSE );
    FLDMAPPER_AllSetUp( fieldWork->g3Dmapper ); // 初期化スクリプトで追加されたマップブロックを読み込む


    //フィールドマップ用イベント起動チェックをセットする
    if( ZONEDATA_IsUnionRoom( fieldWork->map_id ) == TRUE
        || ZONEDATA_IsColosseum( fieldWork->map_id ) == TRUE){
      GAMESYSTEM_EVENT_EntryCheckFunc( gsys, FIELD_EVENT_CheckUnion, fieldWork );
    }
    else if( FIELDMAP_GetMapControlType( fieldWork ) == FLDMAP_CTRLTYPE_NOGRID ){
      GAMESYSTEM_EVENT_EntryCheckFunc( gsys, FIELD_EVENT_CheckNoGrid, fieldWork );
    }
    else if( FIELDMAP_GetMapControlType( fieldWork ) == FLDMAP_CTRLTYPE_HYBRID ){
      GAMESYSTEM_EVENT_EntryCheckFunc( gsys, FIELD_EVENT_CheckHybrid, fieldWork );
    }
    else{
      GAMESYSTEM_EVENT_EntryCheckFunc( gsys, FIELD_EVENT_CheckNormal_Wrap, fieldWork );
    }

    fieldWork->gamemode = GAMEMODE_NORMAL;
    GAMEDATA_SetFrameSpritEnable(GAMESYSTEM_GetGameData(gsys), TRUE);
  }
  

  return MAINSEQ_RESULT_NEXTSEQ;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static MAINSEQ_RESULT mainSeqFunc_update_top(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  // 描画にかかる micro second を表示
#ifdef DEBUG_FIELDMAP_DRAW_MICRO_SECOND_CHECK
  OSTick debug_fieldmap_start_tick = OS_GetTick(); 
  OSTick debug_fieldmap_end_tick;
#endif

  INIT_CHECK();


#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
  DEBUG_MAIN_UPDATE_TYPE = 0;
#endif

  if ( GAMESYSTEM_GetEvent( fieldWork->gsys ) == NULL )
  { //イベントがない状態の時==通常フィールド状態
    FIELD_STATUS * fldstatus = GAMEDATA_GetFieldStatus( fieldWork->gamedata );
    FIELD_STATUS_SetProcAction( fldstatus, PROC_ACTION_FIELD );
  }

  if (GAMEDATA_GetIsSave( fieldWork->gamedata )) return MAINSEQ_RESULT_CONTINUE;

  if (fieldWork->MainHookFlg)
  {
    //描画部分は行う
    DrawTop(fieldWork);
    GFL_CLACT_SYS_Main(); // CLSYSメイン
    //↑ 2010.03.05　CLSYSメインをテイルからトップへ移動。この移動で、2Ｄ描画と3Ｄ描画に１シンクのずれが生じます。saito
    return MAINSEQ_RESULT_CONTINUE;
  }

	//キーの分割取得カウンタをリセット
	GFL_UI_ResetFrameRate();
  SET_CHECK("update_top:zone change check");
	//ゾーン更新処理
  fldmapMain_UpdateMoveZone( fieldWork );
  SET_CHECK("update_top:map main func");
  //マップ別 登録処理
  if( GAMESYSTEM_IsEventExists(gsys) == FALSE ) {
    //登録テーブルごとに個別のメイン処理を呼び出し
    fieldWork->func_tbl->main_func( fieldWork, &fieldWork->now_pos ); 
    //時間イベントのアップデート処理を呼び出し
    EVTIME_Update( fieldWork->gamedata );
  }
  SET_CHECK("update_top:placename & skill eff");
  // 地名表示システム動作処理
  if(fieldWork->placeNameSys){ FIELD_PLACE_NAME_Process( fieldWork->placeNameSys ); }
  // フラッシュ処理
  FIELDSKILL_MAPEFF_Main( fieldWork->fieldskill_mapeff );
  SET_CHECK("update_top:player update");
  //自機更新
  FIELD_PLAYER_Update( fieldWork->field_player );
  SET_CHECK("update_top:comm update");
  //通信用処理(プレイヤーの座標の設定とか
  if( ZONEDATA_IsUnionRoom( fieldWork->map_id ) == TRUE
      || ZONEDATA_IsColosseum( fieldWork->map_id ) == TRUE){
    Union_Main(GAMESYSTEM_GetGameCommSysPtr(gsys), fieldWork);
  }
  else{
    IntrudeField_UpdateCommSystem( fieldWork, fieldWork->gsys, fieldWork->field_player );
  }
  SET_CHECK("update_top:subscreen");
  FIELD_SUBSCREEN_Main(fieldWork->fieldSubscreenWork);
#ifdef  PM_DEBUG
  if(fieldWork->debugWork){ FIELD_DEBUG_UpdateProc( fieldWork->debugWork ); }
#endif
  SET_CHECK("update_top:mmdlsys");
  if( fieldWork->fldMMdlSys != NULL ){
    MI_SetMainMemoryPriority(MI_PROCESSOR_ARM9); //500 micro sec 程度削減
    MMDLSYS_UpdateProc( fieldWork->fldMMdlSys );
    MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);
  }
  SET_CHECK("update_top:gimmick");
  //ギミック動作
  FLDGMK_MoveFieldGimmick(fieldWork);
  SET_CHECK("update_top:fldeff");
  FLDEFF_CTRL_Update( fieldWork->fldeff_ctrl );
  if(fieldWork->union_eff != NULL){
    UNION_EFF_SystemUpdate(fieldWork->union_eff);
  }
  SET_CHECK("update_top:FLDMAPFUNC_Sys_Main");
	// フィールドマップ用制御タスクシステム
	FLDMAPFUNC_Sys_Main( fieldWork->fldmapFuncSys );
  SET_CHECK("update_top:FLDMSGBG_PrintMain");
  if(fieldWork->fldMsgBG ){ FLDMSGBG_PrintMain( fieldWork->fldMsgBG ); }
  SET_CHECK("update_top:TASK");
  // TCB
  GFL_TCB_Main( fieldWork->fieldmapTCB );
  // タスクマネージャ
  FIELD_TASK_MAN_Main( fieldWork->taskManager );
  //now_pos更新
  if( fieldWork->target_now_pos_p != NULL ){
    fieldWork->now_pos = *fieldWork->target_now_pos_p;
  }

  SET_CHECK("update_top:g3d_control");
  //Mapシステムに位置を渡している。
  //これがないとマップ移動しないので注意
  FLDMAPPER_SetPos( fieldWork->g3Dmapper, &fieldWork->now_pos );
  // Visualにかかわる処理
	fldmap_G3D_Control( fieldWork );
  //遊覧船汽笛監視（遊覧船ではないときは処理をスルーする）
  {
    PL_BOAT_WORK_PTR *wk_ptr = GAMEDATA_GetPlBoatWorkPtr(fieldWork->gamedata);
    PL_BOAT_Main(*wk_ptr);
  }
  SET_CHECK("update_top:CLACT");

  // ----------------------top側3D描画処理---------------------------------------
  // これ以降にデータ更新処理を入れないこと。
  GFL_CLACT_SYS_Main(); // CLSYSメイン
  //↑ 2010.03.05　CLSYSメインをテイルからトップへ移動。この移動で、2Ｄ描画と3Ｄ描画に１シンクのずれが生じます。saito
  SET_CHECK("update_top:DrawTop");
  DrawTop(fieldWork);
  SET_CHECK("update_top:tail");
#ifdef DEBUG_FIELDMAP_DRAW_MICRO_SECOND_CHECK
  debug_fieldmap_end_tick = OS_GetTick();
  debug_fieldmap_end_tick -= debug_fieldmap_start_tick;

  if( GFL_UI_KEY_GetCont() & DEBUG_FIELDMAP_DRAW_MICRO_SECOND_CHECK_DRAW_KEY )
  {
    OS_Printf( "top_tick:%d micro second:%d\n",
        debug_fieldmap_end_tick, OS_TicksToMicroSeconds( debug_fieldmap_end_tick ) );
  }
#endif
#ifdef  DEBUG_FIELDMAP_UPDATETOP_SPEED_CHECK
  {
    OSTick end_tick;
    TAIL_CHECK( &end_tick );
    if ( (GFL_UI_KEY_GetTrg() & PAD_BUTTON_L) ||
        ( (GFL_UI_KEY_GetCont() & (PAD_BUTTON_L+PAD_BUTTON_R)) == (PAD_BUTTON_L+PAD_BUTTON_R)) )
    {
      PUT_CHECK();
      OS_TPrintf("top_tick:%ld", end_tick);
      OS_TPrintf(" micro sec:%ld\n", OS_TicksToMicroSeconds( end_tick ) );
    }
  }
#endif

  return MAINSEQ_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * フィールドマップ　トップ部分描画
 * @param	fieldWork	FIELDMAP_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawTop(FIELDMAP_WORK *fieldWork)
{
#ifdef PM_DEBUG
  FIELD_DEBUG_ClearDrawCallBackWork();
#endif  
  GFL_G3D_ClearG3dInfo();
  FIELD_CAMERA_Main( fieldWork->camera_control, GFL_UI_KEY_GetCont() );

  fieldWork->bbdact_y_offs = calcBbdActYOffset( fieldWork );

  MI_SetMainMemoryPriority(MI_PROCESSOR_ARM9);
  fldmap_G3D_Draw_top( fieldWork );
  MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static MAINSEQ_RESULT mainSeqFunc_update_tail(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  
  // 描画にかかる micro second を表示
#ifdef DEBUG_FIELDMAP_DRAW_MICRO_SECOND_CHECK
  OSTick debug_fieldmap_start_tick = OS_GetTick(); 
  OSTick debug_fieldmap_end_tick;
#endif
  INIT_CHECK();

#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
  DEBUG_MAIN_UPDATE_TYPE = 1;
#endif
  
  if(GAMEDATA_GetIsSave( fieldWork->gamedata )){
     GAMEDATA_ResetFrameSpritCount(GAMESYSTEM_GetGameData(gsys));
     return MAINSEQ_RESULT_CONTINUE;
  }

  //ロード後半
  MI_SetMainMemoryPriority(MI_PROCESSOR_ARM9); //500 micro sec 程度削減
  FLDMAPPER_MainTail( fieldWork->g3Dmapper );
  MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);
  
  SET_CHECK("update_tail:subscreen");
  FIELD_SUBSCREEN_Draw(fieldWork->fieldSubscreenWork);
  
  SET_CHECK("update_tail:fldmsgbg");
  if(fieldWork->fldMsgBG ){ FLDMSGBG_PrintMain( fieldWork->fldMsgBG ); }

  SET_CHECK("update_tail:place name");
  if(fieldWork->placeNameSys){ FIELD_PLACE_NAME_Draw( fieldWork->placeNameSys ); }
  
	MI_SetMainMemoryPriority(MI_PROCESSOR_ARM9);
	fldmap_G3D_Draw_tail( fieldWork );
	MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);

	// ゲームデータのフレーム分割用カウンタをリセット
	GAMEDATA_ResetFrameSpritCount(GAMESYSTEM_GetGameData(gsys));


  // 描画にかかる micro second を表示
  // 1シンクに描画以外の処理もしているので、
  // 10000 micro second以内になっているのが理想です。
#ifdef DEBUG_FIELDMAP_DRAW_MICRO_SECOND_CHECK
  debug_fieldmap_end_tick = OS_GetTick();
  debug_fieldmap_end_tick -= debug_fieldmap_start_tick;

  if( GFL_UI_KEY_GetCont() & DEBUG_FIELDMAP_DRAW_MICRO_SECOND_CHECK_DRAW_KEY )
  {
    const GFL_D3D_INFO *info = GFL_G3D_GetG3dInfoPtr();
    OS_TPrintf( "draw_tick:%d ", debug_fieldmap_end_tick );
    OS_TPrintf( "micro second:%d\n", OS_TicksToMicroSeconds( debug_fieldmap_end_tick ) );
    OS_TPrintf( "poly_vtx = %d, %d\n",
        G3X_GetPolygonListRamCount(),
        G3X_GetVtxListRamCount()
        );
    OS_TPrintf( "total poly_vtx = %d, %d\n",
        info->TotalPolygonNum,
        info->TotalVertexNum
        );
  }
#endif
#ifdef  DEBUG_FIELDMAP_UPDATETAIL_SPEED_CHECK
  {
    OSTick end_tick;
    TAIL_CHECK( &end_tick );
    if ( (GFL_UI_KEY_GetTrg() & PAD_BUTTON_L) ||
        ( (GFL_UI_KEY_GetCont() & (PAD_BUTTON_L+PAD_BUTTON_R)) == (PAD_BUTTON_L+PAD_BUTTON_R)) )
    {
      PUT_CHECK();
      OS_TPrintf("tail_tick:%ld", end_tick);
      OS_TPrintf(" micro sec:%ld\n", OS_TicksToMicroSeconds( end_tick ) );
    }
  }
#endif

  return MAINSEQ_RESULT_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static MAINSEQ_RESULT mainSeqFunc_free(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{
  //エンカウントエフェクトコントローラ破棄
  ENCEFF_DeleteCntPtr(fieldWork->EncEffCntPtr);
  //フィールド3Ｄカットインコントローラ破棄
  FLD3D_CI_End(fieldWork->Fld3dCiPtr);

  //フィールドパーティクル破棄
  FLD_PRTCL_End(fieldWork->FldPrtclSys);

  //イベント起動チェックを停止する
  GAMESYSTEM_EVENT_EntryCheckFunc(gsys, NULL, NULL);
  
  //アクターが持つプレイヤー現在位置をPLAYER_WORKに反映する
  FIELD_PLAYER_Update( fieldWork->field_player );
#if 0
  { 
    VecFx32 player_pos;
    PLAYER_WORK * pw = GAMESYSTEM_GetMyPlayerWork(gsys);
    FIELD_PLAYER_GetPos(fieldWork->field_player, &player_pos);
    PLAYERWORK_setPosition(pw, &player_pos);
  }
  if (FIELDMAP_GetMapControlType(fieldWork) == FLDMAP_CTRLTYPE_NOGRID)
  {
    PLAYER_WORK * pw = GAMESYSTEM_GetMyPlayerWork(gsys);
    FIELDMAP_CTRL_NOGRID_WORK* p_mapctrl_work = FIELDMAP_GetMapCtrlWork( fieldWork );
    FIELD_PLAYER_NOGRID* p_nogrid_player = FIELDMAP_CTRL_NOGRID_WORK_GetNogridPlayerWork( p_mapctrl_work );
    RAIL_LOCATION railLoc;


    FIELD_PLAYER_NOGRID_GetLocation( p_nogrid_player, &railLoc );
    PLAYERWORK_setRailPosition(pw, &railLoc);
  }
#endif

	// フィールドマップ用制御タスクシステム
	FLDMAPFUNC_Sys_Delete( fieldWork->fldmapFuncSys );
  
  //通信削除コールバック呼び出し
  {
    GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
    GameCommSys_Callback_FieldDelete( game_comm, fieldWork );
  }
  
  // 地名表示システム破棄
  if(fieldWork->placeNameSys){ FIELD_PLACE_NAME_Delete( fieldWork->placeNameSys ); }

  //ギミック終了
  FLDGMK_EndFieldGimmick(fieldWork);

  //拡張3Ｄオブジェクトモジュール破棄
  FLD_EXP_OBJ_Delete(fieldWork->ExpObjCntPtr);

  //フィールドエンカウント破棄
  FIELD_ENCOUNT_Delete( fieldWork->encount );

  //情報バーの開放
	{
		GAMEDATA *gamedata = GAMESYSTEM_GetGameData( gsys );
		GAMEDATA_SetSubScreenMode(gamedata, FIELD_SUBSCREEN_Exit(fieldWork->fieldSubscreenWork ));
	}
  
  // フィールドエフェクト破棄
  FLDEFF_CTRL_Delete( fieldWork->fldeff_ctrl );

  if(fieldWork->union_eff != NULL){
    UNION_EFF_SystemExit(fieldWork->union_eff);
    fieldWork->union_eff = NULL;
  }
  
  // 天気システム破棄
  FIELD_WEATHER_Exit( fieldWork->weather_sys );
  fieldWork->weather_sys = NULL;

  // ライトシステム破棄
  FIELD_LIGHT_Delete( fieldWork->light );

	// ゾーンフォグシステム破棄
	FIELD_ZONEFOGLIGHT_Delete( fieldWork->zonefog );
  
  // フォグシステム破棄
  FIELD_FOG_Delete( fieldWork->fog );

  //侵入系開放
  FIELD_PALACE_Delete( fieldWork->palace_sys );
  
  // NOGRIDマッパー破棄
  FLDNOGRID_MAPPER_Delete( fieldWork->nogridMapper );

  // シーンエリア破棄
  FLD_SCENEAREA_LOADER_Delete( fieldWork->sceneAreaLoader );
  FLD_SCENEAREA_Delete( fieldWork->sceneArea );


  FIELD_CAMERA_Delete( fieldWork->camera_control );

  //登録テーブルごとに個別の終了処理を呼び出し
  fieldWork->func_tbl->delete_func(fieldWork);
  fldmap_ClearMapCtrlWork( fieldWork );
  
  fldmapMain_MMDL_Finish(fieldWork);
  
  //自機破棄
  FIELD_PLAYER_Delete( fieldWork->field_player );

  FLDMAPPER_ReleaseData( fieldWork->g3Dmapper );

  // 所持金表示ウィンドウ破棄
  if( fieldWork->goldMsgWin )
  {
    FLDMSGWIN_Delete( fieldWork->goldMsgWin );
  }
  
  if(fieldWork->fldMsgBG ){ FLDMSGBG_Delete( fieldWork->fldMsgBG ); }

#ifdef  PM_DEBUG
  if(fieldWork->debugWork){ FIELD_DEBUG_Delete( fieldWork->debugWork ); }
#endif

	GAMEDATA_SetFrameSpritEnable(GAMESYSTEM_GetGameData(gsys), FALSE);
	GFL_UI_StartFrameRateMode( GFL_UI_FRAMERATE_60 );

  // 育て屋
  SODATEYA_Delete( fieldWork->sodateya );

	FIELDSKILL_MAPEFF_Delete( fieldWork->fieldskill_mapeff );

#if USE_DEBUGWIN_SYSTEM
  FIELD_FUNC_RANDOM_GENERATE_TermDebug();
  DEBUGWIN_ExitProc();
#endif  //USE_DEBUGWIN_SYSTEM


#ifdef PM_DEBUG
  //3Ｄ描画コールバックリセット
  FIELD_DEBUG_SetDrawCallBackFunc(FALSE);
#endif  


#ifdef DEBUG_MAIN_TIME_AVERAGE_MASTER_ON
  DEBUG_MAIN_UPDATE_TYPE = 0;
#endif


  return MAINSEQ_RESULT_NEXTSEQ;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static MAINSEQ_RESULT mainSeqFunc_end(GAMESYS_WORK *gsys, FIELDMAP_WORK *fieldWork )
{ 
  // TCB
  GFL_TCB_Exit( fieldWork->fieldmapTCB );
  GFL_HEAP_FreeMemory( fieldWork->fieldmapTCBSysWork );

  // VBLANK反映管理
  FLD_VREQ_Delete( fieldWork->fldVReq );
  fieldWork->fldVReq = NULL;

  // タスクマネージャ
  FIELD_TASK_MAN_Delete( fieldWork->taskManager );

	GFL_TCB_DeleteTask( fieldWork->g3dVintr );

  FLDMAPPER_Delete( fieldWork->g3Dmapper );

	fldmap_G3D_Unload( fieldWork );	//３Ｄデータ破棄
	
	GFL_CLACT_SYS_Delete(); // CLACT破棄
	
	GFL_BMPWIN_Exit();
	
	fldmap_BG_Exit( fieldWork );

  //ユニオンと侵入で読み込むオーバーレイを分ける
  if( ZONEDATA_IsUnionRoom( fieldWork->map_id ) == TRUE
      || ZONEDATA_IsColosseum( fieldWork->map_id ) == TRUE){
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( union_room ) );
  }
  else{
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( field_intrude ) );
  }

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
#ifdef  DEBUG_FIELDMAP_INOUT_SPEED_CHECK
  OSTick debug_tick = OS_GetTick(); 
#endif

  GF_ASSERT(fieldWork->seq < NELEMS(mainfuncTable));
	fieldWork->timer++;

  result = MAINSEQ_RESULT_CONTINUE;
  func = mainfuncTable[fieldWork->seq][fieldWork->seq_switch];
  if (func != NULL) 
  {
    result = func(gsys, fieldWork);
  }
#ifdef  DEBUG_FIELDMAP_INOUT_SPEED_CHECK
  if ( fieldWork->seq != FLDMAPSEQ_UPDATE )
  {
    debug_tick = OS_GetTick() - debug_tick;
    OS_Printf("FIELDMAP seq:%d tick:%ld\n", fieldWork->seq,  debug_tick );
    OS_Printf("FIELDMAP seq:%d micro:%ld\n", fieldWork->seq, OS_TicksToMicroSeconds( debug_tick ) );
  }
#endif

  switch (result) 
  { 
  case MAINSEQ_RESULT_CONTINUE:
#ifdef PM_DEBUG    
    DEBUG_PerformanceSetTopFlg(fieldWork->seq_switch);
#endif    
    fieldWork->seq_switch = !fieldWork->seq_switch;
    break;
  case MAINSEQ_RESULT_NEXTSEQ:
    fieldWork->seq ++;
    fieldWork->subseq = 0;
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
#ifdef PM_DEBUG
  DebugBGInitEnd = FALSE;    //BG初期化監視フラグクリア
#endif
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
 * フィールドマップ　自機にアイテム、自転車を使用。
 * 歩行形態の場合は自転車に。自転車乗りの場合は歩行形態に。
 * @param fieldWork FIELDMAP_WORK
 * @retval BOOL TRUE=使用した FALSE=使用不可
 */
//--------------------------------------------------------------
BOOL FIELDMAP_SetPlayerItemCycle( FIELDMAP_WORK *fieldWork )
{
  BOOL ret = FALSE;
  PLAYER_MOVE_FORM form;
    
  form = FIELD_PLAYER_GetMoveForm( fieldWork->field_player );

  if( form == PLAYER_MOVE_FORM_CYCLE )
  {
    ret = TRUE;
    FIELD_PLAYER_SetRequest( fieldWork->field_player, FIELD_PLAYER_REQBIT_NORMAL );
  }
  else if( form == PLAYER_MOVE_FORM_NORMAL )
  {
    ret = TRUE;
    PMSND_PlaySE( SEQ_SE_BICYCLE );
    FIELD_PLAYER_SetRequest( fieldWork->field_player, FIELD_PLAYER_REQBIT_CYCLE );

		{	// レコードデータ＋
			RECORD * rec = GAMEDATA_GetRecordPtr( fieldWork->gamedata );
			RECORD_Inc( rec, RECID_RIDE_CYCLE );
		}
  }
  
  if( ret == TRUE ){
    FIELD_PLAYER_UpdateRequest( fieldWork->field_player );
  }
  
  return( ret );
}

//======================================================================
//	フィールドマップ　参照、設定
//======================================================================
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
 *	@brief  ノーグリッド動作　マッパーの取得
 */
//--------------------------------------------------------------
FLDNOGRID_MAPPER* FIELDMAP_GetFldNoGridMapper( FIELDMAP_WORK *fieldWork )
{
  return fieldWork->nogridMapper;
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
 * FIELDMAP_WORK 指定ゾーンの現在の天候を返す
 * @param	fieldWork	FIELDMAP_WORK
 * @retval FIELD_WEATHER*
 */
//--------------------------------------------------------------
u16 FIELDMAP_GetZoneWeatherID( FIELDMAP_WORK *fieldWork, u16 zone_id )
{
	return PM_WEATHER_GetZoneWeatherNo( fieldWork->gsys, zone_id );
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

#ifdef  PM_DEBUG
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
#endif

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
 * FIELDMAP_WORK メインGFL_BBDACT_SYS取得
 * @param fieldWork FIELDMAP_WORK
 * @retval GFL_BBDACT_SYS
 */
//--------------------------------------------------------------
GFL_BBDACT_SYS * FIELDMAP_GetBbdActSys( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->mainBbdActSys;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK サブGFL_BBDACT_SYS取得
 * @param fieldWork FIELDMAP_WORK
 * @retval GFL_BBDACT_SYS
 */
//--------------------------------------------------------------
GFL_BBDACT_SYS * FIELDMAP_GetSubBbdActSys( FIELDMAP_WORK *fieldWork )
{
  return fieldWork->subBbdActSys;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK エフェクトGFL_BBDACT_SYS取得
 * @param fieldWork FIELDMAP_WORK
 * @retval GFL_BBDACT_SYS
 */
//--------------------------------------------------------------
GFL_BBDACT_SYS * FIELDMAP_GetEffBbdActSys( FIELDMAP_WORK *fieldWork )
{
  return fieldWork->effBbdActSys;
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
#if 0
  return fieldWork->map_id;
#else
  return fieldWork->location.zone_id;
#endif
}

#if 0
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
#endif

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
 * @brief  FLD_G3DOBJ_CTRL取得
 * @param  fieldWork FIELDMAP_WORK
 * @retval FLD_G3DOBJ_CTRL*
 */
//--------------------------------------------------------------
FLD_G3DOBJ_CTRL * FIELDMAP_GetFldG3dOBJCtrl( FIELDMAP_WORK *fieldWork )
{
  GF_ASSERT( fieldWork->fieldG3dObjCtrl != NULL );
  return( fieldWork->fieldG3dObjCtrl );
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK UNION_EFF_SYSTEM取得
 * @param fieldWork FIELDMAP_WORK
 * @retval UNION_EFF_SYSTEM*
 */
//--------------------------------------------------------------
UNION_EFF_SYSTEM * FIELDMAP_GetUnionEffSystem( FIELDMAP_WORK *fieldWork )
{
  GF_ASSERT(fieldWork->union_eff != NULL);
  return fieldWork->union_eff;
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
  GF_ASSERT( fieldWork );
  GF_ASSERT( fieldWork->func_tbl );
  return fieldWork->func_tbl->type;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のベースシステムタイプを取得
 *
 *	@param	fieldWork   フィールドワーク
 *
 *	@retval FLDMAP_BASESYS_GRID = 0,  ///<ベースシステム　グリッド移動
 *	@retval FLDMAP_BASESYS_RAIL,      ///<ベースシステム　レール移動
 */
//-----------------------------------------------------------------------------
FLDMAP_BASESYS_TYPE FIELDMAP_GetBaseSystemType( const FIELDMAP_WORK *fieldWork )
{
  static const u8 MAPCTRL_BASESYS_TYPE[] = 
  {
    FLDMAP_BASESYS_GRID,
    FLDMAP_BASESYS_RAIL,
    FLDMAP_BASESYS_MAX,   ///<HYBRIDは、ワークから取得する
  };

  GF_ASSERT( fieldWork );
  GF_ASSERT( fieldWork->func_tbl );
  
  if( MAPCTRL_BASESYS_TYPE[ fieldWork->func_tbl->type ] == FLDMAP_BASESYS_MAX )
  {
    return FIELDMAP_CTRL_HYBRID_GetBaseSystemType( fieldWork->mapCtrlWork );
  }
  return MAPCTRL_BASESYS_TYPE[ fieldWork->func_tbl->type ];
}

//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
FIELD_PLACE_NAME * FIELDMAP_GetPlaceNameSys( FIELDMAP_WORK * fieldWork )
{
  return fieldWork->placeNameSys;
}

//--------------------------------------------------------------
/**
 *	@brief	フィールドマップ上で動作する制御タスクシステム	取得
 */
//--------------------------------------------------------------
FLDMAPFUNC_SYS * FIELDMAP_GetFldmapFuncSys( FIELDMAP_WORK * fieldWork )
{
	return fieldWork->fldmapFuncSys;
}

//--------------------------------------------------------------
/**
 *  @brief	拡張3Ｄオブジェクトコントローラ取得
 * @param	fieldWork	FIELDMAP_WORK
 * @retval FLD_EXP_OBJ_CNT_PTR
 */
//--------------------------------------------------------------
FLD_EXP_OBJ_CNT_PTR FIELDMAP_GetExpObjCntPtr( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->ExpObjCntPtr;
}

//--------------------------------------------------------------
/**
 *	@brief	TCBシステム　取得
 */
//--------------------------------------------------------------
extern GFL_TCBSYS* FIELDMAP_GetFieldmapTCBSys( FIELDMAP_WORK * fieldWork )
{
  return fieldWork->fieldmapTCB;
}

//--------------------------------------------------------------
/**
 * @brief フィールドタスクマネージャ 取得
 */
//--------------------------------------------------------------
FIELD_TASK_MAN* FIELDMAP_GetTaskManager( FIELDMAP_WORK* fieldWork )
{
  return fieldWork->taskManager;
}

//--------------------------------------------------------------
/**
 * フィールドカメラの目標座標となるポインタをセット
 * @param fieldWork FIELDMAP_WORK
 * @retval  nothing
 */
//--------------------------------------------------------------
void FIELDMAP_SetNowPosTarget( FIELDMAP_WORK *fieldWork, const VecFx32 *pos )
{
  fieldWork->target_now_pos_p = pos;
}

//--------------------------------------------------------------
/**
 * @brief 所持金表示用ウィンドウを取得
 * @param fieldWork FIELDMAP_WORK
 * @retval 所持金表示用ウィンドウ
 */
//--------------------------------------------------------------
FLDMSGWIN * FIELDMAP_GetGoldMsgWin( FIELDMAP_WORK *fieldWork )
{
  return fieldWork->goldMsgWin;
}

//--------------------------------------------------------------
/**
 * @brief 所持金表示用ウィンドウをセット
 * @param fieldWork FIELDMAP_WORK
 * @param msgWin    FLDMSGWIN
 * @retval 所持金表示用ウィンドウ
 */
//--------------------------------------------------------------
void FIELDMAP_SetGoldMsgWin( FIELDMAP_WORK *fieldWork, FLDMSGWIN *msgWin )
{
  fieldWork->goldMsgWin = msgWin;
}

//--------------------------------------------------------------
/**
 * @brief 育て屋を取得
 * @param fieldWork FIELDMAP_WORK
 * @retval 育て屋
 */
//--------------------------------------------------------------
SODATEYA* FIELDMAP_GetSodateya( FIELDMAP_WORK* fieldWork )
{
  return fieldWork->sodateya;
}

//--------------------------------------------------------------
/**
 * FIELDMAP_WORK AREADATA*取得
 * @param fieldWork FIELDMAP_WORK
 * @retval AREADATA*
 */
//--------------------------------------------------------------
AREADATA* FIELDMAP_GetAreaData( FIELDMAP_WORK *fieldWork )
{
	return fieldWork->areadata;
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
	//メイン画面の背景色
  GFL_BG_LoadPalette( GFL_BG_FRAME0_M, (void*)fldmapdata_bgColorTable, 16*2, 0 );
	//サブ画面の背景色
  GFL_BG_LoadPalette( GFL_BG_FRAME0_S, (void*)fldmapdata_bgColorTable, 16*2, 0 );
	//ディスプレイ面の選択
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
	GFL_DISP_SetDispOn();
  
  //使用するウィンドウ
  GX_SetVisibleWnd( GX_WNDMASK_NONE );
}

/*
 *  @brief  フィールドBGモードデフォルト初期化
 */
void FIELDMAP_InitBGMode( void )
{
	//ＢＧモード設定
	GFL_BG_SetBGMode( &fldmapdata_bgsysHeader );
}


/*
 *  @brief  フィールドBG Vramデフォルト初期化
 */
void FIELDMAP_InitBG( FIELDMAP_WORK* fieldWork )
{
  /**
  //アフィン初期化
  //@note 
  //アプリケーション等で、ＢＧ２面でアフィンを使用した場合フィールドに戻ってきてもパラメータが残っており、
  //引き続きＢＧ２面を使用する、カットインやクロスフェード処理で表示の不具合が発生するためここで初期化をします。
  */
  {
    MtxFx22 mtx;
    GFL_CALC2D_GetAffineMtx22( &mtx, 0, FX32_ONE, FX32_ONE, GFL_CALC2D_AFFINE_MAX_NORMAL );
    G2_SetBG2Affine( &mtx, 0, 0, 0, 0 );
  }
	//ＢＧコントロール設定
	GFL_BG_SetBGControl3D( FLDBG_MFRM_3D_PRI );

  // 会話ウインドウリソースセットアップ
	FLDMSGBG_SetupResource( fieldWork->fldMsgBG );
#if 0
  {
    GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, FLDBG_MFRM_EFF1_SCRSIZE, 0,
			GFL_BG_SCRSIZ_256x256, FLDBG_MFRM_EFF1_COLORMODE,
			FLDBG_MFRM_EFF1_SCRBASE, FLDBG_MFRM_EFF1_CHARBASE, FLDBG_MFRM_EFF1_CHARSIZE,
			GX_BG_EXTPLTT_01, FLDBG_MFRM_EFF1_PRI, 0, 0, FALSE
		};
		
		GFL_BG_SetBGControl( FLDBG_MFRM_EFF1, &bgcntText, GFL_BG_MODE_TEXT );
		GFL_BG_SetVisible( FLDBG_MFRM_EFF1, VISIBLE_ON );
		GFL_BG_FillCharacter( FLDBG_MFRM_EFF1, 0x00, 1, 0 );
		GFL_BG_FillScreen( FLDBG_MFRM_EFF1,0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( FLDBG_MFRM_EFF1 );
	}
#endif
#ifdef  PM_DEBUG
  //フィールドデバッグ初期化
  fieldWork->debugWork = FIELD_DEBUG_Init( fieldWork, FLDBG_MFRM_EFF1, fieldWork->heapID );
#endif

#if USE_DEBUGWIN_SYSTEM
	DEBUGWIN_InitProc( FLDBG_MFRM_MSG , FLDMSGBG_GetFontHandle(fieldWork->fldMsgBG) );
	DEBUGWIN_ChangeLetterColor( 31,31,31 );
	FIELD_FUNC_RANDOM_GENERATE_InitDebug
		( fieldWork->heapID, fieldWork->gamedata );
#endif  //USE_DEBUGWIN_SYSTEM

#ifdef PM_DEBUG
  //ＢＧ初期化終了
  DebugBGInitEnd = TRUE;
#endif
}
/*
 *  @brief  フィールドBG Vramデフォルト初期化
 */
void FIELDMAP_InitBgNoTrans( FIELDMAP_WORK* fieldWork )
{
  /**
  //アフィン初期化
  //@note 
  //アプリケーション等で、ＢＧ２面でアフィンを使用した場合フィールドに戻ってきてもパラメータが残っており、
  //引き続きＢＧ２面を使用する、カットインやクロスフェード処理で表示の不具合が発生するためここで初期化をします。
  */
  {
    MtxFx22 mtx;
    GFL_CALC2D_GetAffineMtx22( &mtx, 0, FX32_ONE, FX32_ONE, GFL_CALC2D_AFFINE_MAX_NORMAL );
    G2_SetBG2Affine( &mtx, 0, 0, 0, 0 );
  }
	//ＢＧコントロール設定
	GFL_BG_SetBGControl3D( FLDBG_MFRM_3D_PRI );

  // 会話ウインドウリソースセットアップ
	FLDMSGBG_SetupResourceNoTrans( fieldWork->fldMsgBG );
#if USE_DEBUGWIN_SYSTEM
	DEBUGWIN_InitProc( FLDBG_MFRM_MSG , FLDMSGBG_GetFontHandle(fieldWork->fldMsgBG) );
	DEBUGWIN_ChangeLetterColor( 31,31,31 );
	FIELD_FUNC_RANDOM_GENERATE_InitDebug
		( fieldWork->heapID, fieldWork->gamedata );
#endif  //USE_DEBUGWIN_SYSTEM

#ifdef PM_DEBUG
  //ＢＧ初期化終了
  DebugBGInitEnd = TRUE;
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  VBLANK期間レジスタ反映管理システムの取得
 */
//-----------------------------------------------------------------------------
FLD_VREQ *FIELDMAP_GetFldVReq(FIELDMAP_WORK *fieldWork)
{
  return fieldWork->fldVReq;
}

#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * デバッグ関数デバッグワーク初期化
 * @param fieldWork FIELDMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELDMAP_InitDebugWork( FIELDMAP_WORK* fieldWork )
{
  //フィールドデバッグ初期化
  fieldWork->debugWork = FIELD_DEBUG_Init( fieldWork, FLDBG_MFRM_EFF1, fieldWork->heapID );
}
#endif

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

  // tailフレームを実行せずにFreeSeqに遷移した場合、
  // 描画途中のコマンドがNNSにたまっている可能性がるので、
  // 念のためFlushBuffer
  NNS_G3dGeFlushBuffer();

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
	G3X_SetClearColor(GX_RGB(0,0,0),31,0x7fff,0,FALSE);
	
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
	fieldWork->mainBbdActSys = GFL_BBDACT_CreateSys(
			FIELD_G3D_BBDACT_RESMAX, FIELD_G3D_BBDACT_ACTMAX,
			fldmap_G3D_BBDTrans, fieldWork->heapID );
  {
    GFL_BBD_SYS * bbdsys = GFL_BBDACT_GetBBDSystem(fieldWork->mainBbdActSys);
    GFL_BBD_SetOrigin(bbdsys, GFL_BBD_ORIGIN_BOTTOM);
    GFL_BBD_SetFlipTexOffs( bbdsys, 0, FIELD_G3D_BBD_FLIP_TEX_OFFS_T );
  }
  
  fieldWork->subBbdActSys = GFL_BBDACT_CreateSys(
      FIELD_G3D_SUBBBDACT_RESMAX, FIELD_G3D_SUBBBDACT_ACTMAX,
      fldmap_G3D_BBDTrans, fieldWork->heapID );
  
  fieldWork->effBbdActSys = GFL_BBDACT_CreateSys(
      FIELD_G3D_EFFBBDACT_RESMAX, FIELD_G3D_EFFBBDACT_ACTMAX,
      fldmap_G3D_BBDTrans, fieldWork->heapID );
  {
    #define BBD_EFF_SCALE ((FX32_ONE+0xc00)*4)
    GFL_BBD_SYS *bbdSys = GFL_BBDACT_GetBBDSystem( fieldWork->effBbdActSys );
    VecFx32 scale = {BBD_EFF_SCALE,BBD_EFF_SCALE,BBD_EFF_SCALE};
    GFL_BBD_SetScale( bbdSys, &scale );
    { //Poligon ID
      u8 id = 0;
      GFL_BBD_SetPolID( bbdSys, &id );
    }
  }
  
  fieldmap_G3D_BBDSetColorParam( fieldWork ); // 色設定
  
  //G3dObj作成
  fieldWork->fieldG3dObjCtrl = FLD_G3DOBJ_CTRL_Create(
      fieldWork->heapID, FIELD_G3DOBJ_RES_MAX, FIELD_G3DOBJ_OBJ_MAX );
  
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
  BOOL map_load_start;

#ifdef PM_DEBUG
  //負荷セット
  DEBUG_PerformanceSetStress();
#endif

	GFL_BBDACT_Main( fieldWork->mainBbdActSys );
  GFL_BBDACT_Main( fieldWork->subBbdActSys );
  GFL_BBDACT_Main( fieldWork->effBbdActSys );

  // NOGRID動作制御
  FLDNOGRID_MAPPER_Main( fieldWork->nogridMapper );

  // scenearea
  FLD_SCENEAREA_Update( fieldWork->sceneArea, &fieldWork->now_pos );

  //ZONEFOGLIGHT
  FIELD_ZONEFOGLIGHT_Update( fieldWork->zonefog, fieldWork->heapID ); 
	
	FIELD_WEATHER_Main( fieldWork->weather_sys, HEAPID_WEATHER );
	FIELD_FOG_Main( fieldWork->fog );
	FIELD_LIGHT_Main( fieldWork->light, GFL_RTC_GetTimeBySecond() );

  MI_SetMainMemoryPriority(MI_PROCESSOR_ARM9); //500 micro sec 程度削減
	map_load_start = FLDMAPPER_Main( fieldWork->g3Dmapper );
  MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);
  if (map_load_start)
  {
    GFL_NET_ChangeoverChangeSpeed(GFL_NET_CROSS_SPEED_PAUSE);  //すれ違い一時停止
    NOZOMU_Printf("マップロード開始\n");
  }
#ifdef PM_DEBUG
  if( FLDMAPPER_CheckTrans( fieldWork->g3Dmapper ) == FALSE )
  {
    //負荷をオフ
    DEBUG_PerformanceStressON(FALSE, STRESS_ID_MAP);
  }
  else{
    //負荷をオン
    DEBUG_PerformanceStressON(TRUE, STRESS_ID_MAP);
  }
#endif
}

//--------------------------------------------------------------
/**
 * G3D 描画
 * @param fieldWork FIELMAP_WORK
 * @retval nothing
 */
//--------------------------------------------------------------

// topフレームでの描画
// ３D描画環境の初期化
// field_g3d_mapperのtopフレーム描画
static void fldmap_G3D_Draw_top( FIELDMAP_WORK * fieldWork )
{
#ifdef PM_DEBUG
  if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG) &&
        (GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT) ){
    return;
  }
#endif
  
  GFL_G3D_DRAW_Start();
  GFL_G3D_DRAW_SetLookAt();	//カメラグローバルステート設定

#ifdef PM_DEBUG  
  {
    s32 check;
    //ラインズオーバーチェック
    check = G3X_IsLineBufferUnderflow();
//    GF_ASSERT_MSG( check == 0, "LINES OVER" );
    if (check) OS_Printf("LINES OVER\n");
    //ポリゴンリストRAM, 頂点RAMのオーバーフローチェック
    check = G3X_IsListRamOverflow();
//    GF_ASSERT_MSG( check == 0, "POLY_VTX_OVER" );
    if (check) OS_Printf("POLY_VTX_OVER\n");
  }
#endif  //PM_DEBUG


  {
    static const DRAW3DMODE_FUNC func[DRAW3DMODE_MAX] = {
      Draw3DNormalMode_top,
      NULL,
      NULL,
    };

    if(func[ fieldWork->Draw3DMode ]){
      func[ fieldWork->Draw3DMode ](fieldWork);
    }
  }
}

// tailフレームでの描画
// 3D・2Dの描画処理
// ３D描画終了（スワップバッファ）
static void fldmap_G3D_Draw_tail( FIELDMAP_WORK * fieldWork )
{
#ifdef PM_DEBUG
  if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG) &&
        (GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT) ){
    return;
  }
#endif
  
  {
    static const DRAW3DMODE_FUNC func[DRAW3DMODE_MAX] = {
      Draw3DNormalMode_tail,
      Draw3DCutinMode,
//      Draw3DScrnTexMode,
      DrawEncEff,
    };

    func[ fieldWork->Draw3DMode ](fieldWork);
  }

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
	GFL_BBDACT_DeleteSys( fieldWork->mainBbdActSys );
  GFL_BBDACT_DeleteSys( fieldWork->subBbdActSys );
  GFL_BBDACT_DeleteSys( fieldWork->effBbdActSys );
  FLD_G3DOBJ_CTRL_Delete( fieldWork->fieldG3dObjCtrl );
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
  
  // 
  // Snd読み込みをスレッド動作にすることで、
  // Sndリソース読み込み中に、mainスレッドが動作している。
  //
  // SND分割読み込み　と並列にVBlank処理が動作している場合には、
  // 処理があふれる可能性があるため３Dテクスチャの転送を行わない。
  // 
  if( PMSND_IsAccessCARD() == FALSE ){
    if( fieldWork->fldMMdlSys != NULL ){
      MMDLSYS_VBlankProc( fieldWork->fldMMdlSys );
    }
  
    if( fieldWork->fieldG3dObjCtrl != NULL ){
      FLD_G3DOBJ_CTRL_Trans( fieldWork->fieldG3dObjCtrl );
    }
  }

	GFL_CLACT_SYS_VBlankFunc();	//セルアクターVBlank

  //VREQ
  if(fieldWork->fldVReq){
    FLD_VREQ_VBlank( fieldWork->fldVReq );
  }
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
 * @param zone_id
 */
//--------------------------------------------------------------
static void FIELD_EDGEMARK_Setup(const AREADATA * areadata, const u16 zone_id)
{
  static GXRgb edgeTable[8];
  int edgemark_type = AREADATA_GetEdgeMarkingType(areadata);

  if (edgemark_type == 0xff)
  {
    G3X_EdgeMarking(FALSE);
    return;
  }
  
  //ビンゴマップを除く裏フィールドのときはエッジマーキングをつけない
  if ( ZONEDATA_IsPalaceFieldNoBingo( zone_id) ) G3X_EdgeMarking(FALSE);
  else G3X_EdgeMarking(TRUE);

  GFL_ARC_LoadData((void *)edgeTable, ARCID_FIELD_EDGEMARK, edgemark_type);
  //プログラム用エッジカラー設定（インデックス7番を使用します）
  edgeTable[7] = GX_RGB(16,16,16);
  G3X_SetEdgeColorTable( edgeTable );
}

//----------------------------------------------------------------------------
/**
 *	@brief  BBDにカラーパラメータを設定する。
 *
 *	@param	fieldWork   フィールドワーク
 *	@param	heapID      ヒープID
 */
//-----------------------------------------------------------------------------
static void fieldmap_G3D_BBDSetColorParam( FIELDMAP_WORK * fieldWork )
{
  FLD_BBD_COLOR* p_color;
  HEAPID heapID = GFL_HEAP_LOWID(fieldWork->heapID);
  u32 bbd_color_idx = AREADATA_GetBBDColor( fieldWork->areadata );

  // ビルボードカラーの設定
  p_color = FLD_BBD_COLOR_Create( heapID );
  FLD_BBD_COLOR_Load( p_color, bbd_color_idx );
  FLD_BBD_COLOR_SetData(
      p_color, GFL_BBDACT_GetBBDSystem(fieldWork->mainBbdActSys) );
  FLD_BBD_COLOR_Delete( p_color );
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
  
	MMDLSYS_SetupProc( fmmdlsys,	//動作モデルシステム　セットアップ
		fieldWork->heapID, gdata, fieldWork,
    fieldWork->g3Dmapper, fieldWork->nogridMapper );
	
	MMDL_BLACTCONT_Setup(		//動作モデルビルボード　セットアップ
		fieldWork->fldMMdlSys, fieldWork->mainBbdActSys, 32 );

	//ビルボードリソース登録
  MMDL_BLACTCONT_AddResourceTex( fieldWork->fldMMdlSys );
  
#ifdef ADDFIX100703_REGU_TEX_HERO
  { //自機リソース登録
    int sex = 0;
    u16 hero_code;
    u16 zone_id = fieldWork->location.zone_id;
    
    if( ZONEDATA_IsUnionRoom(zone_id) == FALSE &&
        ZONEDATA_IsColosseum(zone_id) == FALSE ){
      GAME_COMM_SYS_PTR game_comm =
        GAMESYSTEM_GetGameCommSysPtr( fieldWork->gsys );
      sex = IntrudeField_GetPlayerSettingSex( game_comm );
    }else{
      const MYSTATUS *myst = GAMEDATA_GetMyStatus( gdata );
      sex = MyStatus_GetMySex( myst );
    }
    
    hero_code = FIELD_PLAYER_GetMoveFormToOBJCode(
        sex, PLAYER_DRAW_FORM_NORMAL );
    MMDL_BLACTCONT_AddResourceTexCode( fmmdlsys, hero_code );
    KAGAYA_Printf( "FIELD INIT ADD HERO TEX sex=%d\n", sex );
  }
#endif
  
  MMDL_G3DOBJCONT_Setup( //動作モデルオブジェクト　セットアップ
      fieldWork->fldMMdlSys, fieldWork->fieldG3dObjCtrl );

	//動作モデル描画　セットアップ
  {
    const u16 *pAngleYaw =
      FIELD_CAMERA_GetAngleYawAddress( fieldWork->camera_control );
	  MMDLSYS_SetupDrawProc( fieldWork->fldMMdlSys, pAngleYaw );
	}

	//動作モデル　復帰
	MMDLSYS_Pop( fieldWork->fldMMdlSys );
  
  //影の生成制御フラグ
  MMDLSYS_SetJoinShadow( fmmdlsys, zonedata_GetShadowUse( fieldWork->map_id ) );
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
	u16 i = 1;
  u16 num = 0;
  u16 count;
	u16 *pList;
	pList = GFL_ARC_LoadDataAlloc( ARCID_MMDL_LIST, list_id, heapID );
	mlist->count = 0;

  //ヘッダ部分２バイトとばす(i=1から)
  count = pList[0];
	while( pList[i] != OBJCODEMAX ){
		mlist->id_list[num] = pList[i];
		i++;
    num++;
		GF_ASSERT( num < MMDL_LIST_MAX );
	}
	
	NOZOMU_Printf( "モデルリスト総数 %d\n", num );
	mlist->count = num;
	mlist->id_list[num] = OBJCODEMAX;

  GF_ASSERT_MSG(count == mlist->count,"ERROR:%d_%d",count,mlist->count);
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
  { //エンカウントデータはゾーンチェンジ後に遅延読み込みを行う
    EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( fieldWork->gamedata );
    if ( EVENTDATA_SYS_IsEncountLoaded( evdata ) == FALSE )
    {
      MI_SetMainMemoryPriority(MI_PROCESSOR_ARM9);
      EVENTDATA_SYS_LoadEncountData(
          evdata, fieldWork->location.zone_id, GAMEDATA_GetSeasonID(fieldWork->gamedata) );
      MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);
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
  VecFx32 loc_pos;

  LOCATION_Get3DPos( lc, &loc_pos ); 
	
	if( pos->x != loc_pos.x || pos->z != loc_pos.z ){
    LOCATION_Set3DPos( lc, pos );
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
  VecFx32 pos;
	MAP_MATRIX *mat = GAMEDATA_GetMapMatrix(fieldWork->gamedata);

  LOCATION_Get3DPos( lc, &pos );
	
	if( MAP_MATRIX_CheckVectorPosRange(mat,pos.x,pos.z) == TRUE ){
		u32 zone_id =
			MAP_MATRIX_GetVectorPosZoneID( mat, pos.x, pos.z );
		
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
  VecFx32 lc_pos;
	GAMEDATA *gdata = GAMESYSTEM_GetGameData( fieldWork->gsys );
	EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );
	MMDLSYS *fmmdlsys = fieldWork->fldMMdlSys;
	
	MAP_MATRIX *mat = GAMEDATA_GetMapMatrix(fieldWork->gamedata);
	u32 new_zone_id;
	u32 old_zone_id = lc->zone_id;

  GFL_NET_ChangeoverChangeSpeed(GFL_NET_CROSS_SPEED_PAUSE);  //すれ違い一時停止

  INIT_CHECK();   //デバッグ：処理負荷計測用

  //エフェクトエンカウント破棄
  EFFECT_ENC_EffectDelete( fieldWork->encount );

  LOCATION_Get3DPos( lc, &lc_pos );

  new_zone_id = MAP_MATRIX_GetVectorPosZoneID(
			mat, lc_pos.x, lc_pos.z );

	
	GF_ASSERT( new_zone_id != MAP_MATRIX_ZONE_ID_NON );

  TOMOYA_Printf( "zone change start %d\n", new_zone_id );

  SET_CHECK("zonechange:delete mmdl");
	
  MI_SetMainMemoryPriority(MI_PROCESSOR_ARM9); // 1000 micro sec程度削減
	//旧ゾーン配置動作モデル削除 
	MMDLSYS_DeleteZoneUpdateMMdl( fmmdlsys );

  SET_CHECK("zonechange:event load");
	
	//次のイベントデータをロード
  EVENTDATA_SYS_Clear( evdata );
	EVENTDATA_SYS_LoadEventData( evdata, new_zone_id, GAMEDATA_GetSeasonID(gdata) );
  MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);

  //歩いてゾーンが変更した場合のフラグ初期化
  FIELD_FLAGCONT_INIT_WalkStepOver( gdata, fieldWork );
	
  SET_CHECK("zonechange:bgm");
	//BGM切り替え
	zoneChange_SetBGM( fieldWork, new_zone_id );

  SET_CHECK("zonechange: weather");
	// ZONEフォグライト設定
	zoneChange_SetZoneFogLight( fieldWork, new_zone_id );
	
	//天候リクエスト
	zoneChange_SetWeather( fieldWork, new_zone_id );

  SET_CHECK("zonechange: camera area");
  //カメラエリアの設定
  zoneChange_SetCameraArea( fieldWork, new_zone_id );
	
  SET_CHECK("zonechange: scene area");
	//PLAYER_WORK更新
	zoneChange_UpdatePlayerWork( gdata, new_zone_id );

  //SCENEAREA更新 
  zoneChangeScene( fieldWork, new_zone_id );

	// 地名表示システムに, ゾーンの切り替えを通達
  if( fieldWork->placeNameSys &&
      (GAMESYSTEM_IsEventExists(fieldWork->gsys) == FALSE) ){ 
    FIELD_PLACE_NAME_DisplayOnStanderdRule( fieldWork->placeNameSys, new_zone_id ); 
  }

  SET_CHECK("zonechange: script");
	//ゾーンID更新
	lc->zone_id = new_zone_id;
	GAMEBEACON_Set_ZoneChange(new_zone_id, gdata);
	
  //マップ到着フラグセット
  ARRIVEDATA_SetArriveFlag( gdata, new_zone_id );

  //特殊スクリプト呼び出し：ゾーン切り替え
  SCRIPT_CallZoneChangeScript( fieldWork->gsys, HEAPID_PROC );

  SET_CHECK("zonechange:set mmdl");
	//新規ゾーンに配置する動作モデルセット 1200 micro sec程度削減
  MI_SetMainMemoryPriority(MI_PROCESSOR_ARM9);
	zoneChange_SetMMdl( gdata, fmmdlsys, evdata, new_zone_id );
  MI_SetMainMemoryPriority(MI_PROCESSOR_ARM7);

  //侵入：通信プレイヤー更新停止フラグリセット
  Intrude_ResetStopCommPlayerUpdateMapFlag(GAMESYSTEM_GetGameCommSysPtr(fieldWork->gsys));
	
  SET_CHECK("zonechange:tail");

#ifdef DEBUG_FIELDMAP_ZONE_CHANGE_SYNC
  {
  OSTick debug_fieldmap_end_tick;
  TAIL_CHECK( &debug_fieldmap_end_tick );
  debug_fieldmap_end_tick = OS_TicksToMicroSeconds (debug_fieldmap_end_tick);
  if( debug_fieldmap_end_tick > 10000 )
  {
    OS_TPrintf( "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" );
    OS_TPrintf( "!!!!!!zone_change TickOver  [%d] micro second !!!!!!\n", debug_fieldmap_end_tick );
    OS_TPrintf( "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" );
    PUT_CHECK();
  }
  }
#endif

  
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
static void zoneChange_SetMMdl( GAMEDATA *gdata,
		MMDLSYS *fmmdlsys, EVENTDATA_SYSTEM *evdata, u32 zone_id )
{
	u16 count = EVENTDATA_GetNpcCount( evdata );
	NOZOMU_Printf("COUNT : %d\n",count);
	if( count ){
    EVENTWORK *evwork =  GAMEDATA_GetEventWork( gdata );
		const MMDL_HEADER *header = EVENTDATA_GetNpcData( evdata );
		MMDLSYS_SetMMdl( fmmdlsys, header, zone_id, count, evwork );
	}
}

//--------------------------------------------------------------
/**
 * ゾーン切り替え時の処理　BGM切り替え
 * @param	fieldWork FIELDMAP_WORK*
 * @param	next_zone_id 次のゾーンID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void zoneChange_SetBGM( FIELDMAP_WORK* fieldWork, u32 next_zone_id )
{
  GAMEDATA*        gdata  = GAMESYSTEM_GetGameData( fieldWork->gsys );
  ISS_SYS*         iss    = GAMESYSTEM_GetIssSystem( fieldWork->gsys );
  FIELD_SOUND*     fsnd   = GAMEDATA_GetFieldSound( gdata );
  PLAYER_WORK*     player = GAMEDATA_GetMyPlayerWork( gdata );
  PLAYER_MOVE_FORM form   = PLAYERWORK_GetMoveForm( player );

  // ISSシステムにゾーンの切り替わりを通知
  ISS_SYS_ZoneChange( iss, next_zone_id );

  // BGM を変更
  if( form != PLAYER_MOVE_FORM_SWIM ) { // なみのり時は変更しない
    FSND_ChangeBGM_byZoneChange( fsnd, gdata, next_zone_id );
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
	u16 w_no;
	FIELD_WEATHER *we = FIELDMAP_GetFieldWeather( fieldWork );

  PM_WEATHER_UpdateZoneChangeWeatherNo( fieldWork->gsys, zone_id );
  
  w_no = GAMEDATA_GetWeatherNo( fieldWork->gamedata );

	if( w_no != WEATHER_NO_NUM ){
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
	
	FIELD_ZONEFOGLIGHT_LoadReq( fieldWork->zonefog, ZONEDATA_GetFog(zone_id), ZONEDATA_GetLight(zone_id), 1 );  // 実際の読み込みはメインフレーム
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

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーン切り替え時の処理　カメラエリア更新
 *
 *	@param	fieldWork   FIELDMAP_WORK
 *	@param	zone_id     次のゾーン
 */
//-----------------------------------------------------------------------------
static void zoneChange_SetCameraArea( FIELDMAP_WORK* fieldWork, u32 zone_id )
{
  setupCameraArea( fieldWork, zone_id, fieldWork->heapID );
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

//----------------------------------------------------------------------------
/**
 *	@brief  カメラエリアの設定
 *
 *	@param	fieldWork
 *	@param	heapID 
 */
//-----------------------------------------------------------------------------
static void setupCameraArea( FIELDMAP_WORK *fieldWork, u32 zone_id, HEAPID heapID )
{
  u32 cameraarea = ZONEDATA_GetCameraAreaID(zone_id);

  if ( cameraarea != ZONEDATA_NO_CAMERA_AREA_ID )
  {
    FIELD_CAMERA_LoadCameraArea( fieldWork->camera_control, cameraarea, heapID );
  }
  else
  {
    FIELD_CAMERA_ClearCameraArea( fieldWork->camera_control );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  wfbcのセットアップ
 *
 *  @param  gdata
 *	@param	fieldWork
 *	@param	zone_id 
 */
//-----------------------------------------------------------------------------
static void setupWfbc( GAMEDATA* gdata, FIELDMAP_WORK *fieldWork, u32 zone_id )
{
  FIELD_WFBC_CORE* p_core;
  FIELD_WFBC* p_wfbc;
  EVENTDATA_SYSTEM* p_evdata;
  FIELD_STATUS* p_fs =  GAMEDATA_GetFieldStatus( gdata );
  MAPMODE mapmode = 0;
  
  if( ZONEDATA_IsWfbc( fieldWork->map_id ) )
  {
    
    // WFBC街情報を設定
    mapmode = FIELD_STATUS_GetMapMode( p_fs );
    if( mapmode == MAPMODE_NORMAL )
    {
      p_core = GAMEDATA_GetMyWFBCCoreData( fieldWork->gamedata );
    }
    else
    {
      // 通信相手の情報を取得
      p_core = GAMEDATA_GetWFBCCoreData( fieldWork->gamedata, GAMEDATA_WFBC_ID_COMM );
    }
    FLDMAPPER_SetWfbcData( fieldWork->g3Dmapper, p_core, mapmode );

    // 
    p_wfbc = FLDMAPPER_GetWfbcWork( fieldWork->g3Dmapper );
    p_evdata = GAMEDATA_GetEventData( gdata );

    // イベントの設定
    FILED_WFBC_EventDataOverwrite( p_wfbc, p_evdata, mapmode, fieldWork->heapID );
    
    // カメラのセットアップ
    FIELD_WFBC_SetUpCamera( p_wfbc, fieldWork->camera_control, fieldWork->heapID );

    // MMDLSYSのセットアップ
    FIELD_WFBC_SetUpMmdlSys( p_wfbc, GAMEDATA_GetMMdlSys( fieldWork->gamedata ) );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  シーンエリアのセットアップ
 *
 *	@param	fieldWork ワーク
 *	@param	zone_id   ゾーンID
 */
//-----------------------------------------------------------------------------
static void zoneChangeScene( FIELDMAP_WORK *fieldWork, u32 zone_id )
{
  u32 id = ZONEDATA_GetSceneAreaID( zone_id );
  TOMOYA_Printf( "zone_id %d   id %d\n", zone_id, id );
  if( id != ZONEDATA_NO_SCENEAREA_ID ) 
  {
    TOMOYA_Printf( "seanarea load\n" );
    // まずクリア
    FLD_SCENEAREA_Release( fieldWork->sceneArea );
    FLD_SCENEAREA_LOADER_Clear( fieldWork->sceneAreaLoader );
    
    // シーンエリアを読み込んで設定
    FLD_SCENEAREA_LOADER_Load( fieldWork->sceneAreaLoader, FLD_SCENEAREA_LOAD_GRID, id, HEAPID_FIELD_SCENE );


    FLD_SCENEAREA_Load( fieldWork->sceneArea, 
        FLD_SCENEAREA_LOADER_GetData(fieldWork->sceneAreaLoader),
        FLD_SCENEAREA_LOADER_GetDataNum(fieldWork->sceneAreaLoader),
        FLD_SCENEAREA_LOADER_GetFunc(fieldWork->sceneAreaLoader) );
  }
  else if( !ZONEDATA_IsRailMap( zone_id ) )
  {
    TOMOYA_Printf( "seanarea clear\n" );
    FLD_SCENEAREA_Release( fieldWork->sceneArea );
    FLD_SCENEAREA_LOADER_Clear( fieldWork->sceneAreaLoader );
  }
}


//==================================================================
/**
 * フィールドで実行されている通信Noを取得する
 *
 * @param   game_comm		    
 *
 * @retval  GAME_COMM_NO		実行している通信ゲーム番号
 */
//==================================================================
GAME_COMM_NO FIELDCOMM_CheckCommNo(GAME_COMM_SYS_PTR game_comm)
{
  GAME_COMM_NO comm_no = GameCommSys_BootCheck(game_comm);
  switch(comm_no){
  case GAME_COMM_NO_NULL:                  //何も起動していない状態
    if(NetErr_App_CheckError()){  //エラーが発生している場合は最後に起動していた通信番号を返す
      return GameCommSys_GetLastCommNo(game_comm);
    }
    return comm_no;
  case GAME_COMM_NO_FIELD_BEACON_SEARCH:   //フィールド上でビーコンサーチ
  case GAME_COMM_NO_INVASION:              //侵入
  case GAME_COMM_NO_DEBUG_SCANONLY:        //デバッグ用スキャンのみ
    return comm_no;
  }
  GF_ASSERT_MSG(0, "comm_no = %d\n", comm_no); //フィールドで上記通信以外はありえない
  return GAME_COMM_NO_INVASION;   //通信中という意味で侵入Noを返すようにする
}

//==================================================================
/**
 * フィールド通信終了処理
 *
 * @param   game_comm		
 *
 * @retval  FIELDCOMM_EXIT		通信終了状態
 *
 * FIELDCOMM_EXIT_END     ：通信が正常に終了
 * FIELDCOMM_EXIT_CONTINUE：通信終了待ちです。先のシーケンスへは進まずにENDになるまで待ってください
 * FIELDCOMM_EXIT_ERROR   ：エラーが発生しています。エラー画面を表示してください。
 *                          エラー画面表示後はこの関数を呼ぶ必要はありません
 */
//==================================================================
FIELDCOMM_EXIT FIELDCOMM_ExitWait(GAME_COMM_SYS_PTR game_comm)
{
  if(GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_NULL){
    return FIELDCOMM_EXIT_END;
  }
  else if(GameCommSys_CheckSystemWaiting(game_comm) == FALSE){
    GameCommSys_ExitReq(game_comm);
  }
  return FIELDCOMM_EXIT_CONTINUE;
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
	FIELD_CLSYS_RESOUCE_MAX+8,  //2010.06.18 足りない状況に対応するためCGRのみ追加
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

//--フィールド用ギミックテンポラリワーク操作関数群--//
//==================================================================
/**
 * ギミックテンポラリワーク初期化関数
 *
 * @param   tmpWork     テンポラリワークポインタ
 */
//==================================================================
static void InitGmkTmpWork(GMK_TMP_WORK *tmpWork)
{
  tmpWork->AssignID = GMK_TMP_NO_ASSIGN_ID;
  tmpWork->Work = NULL;
}

//==================================================================
/**
 * ギミックテンポラリワークメモリアロック関数
 *
 * @note  使用の際はgmk_tmp_wk.hを参照のこと
 *
 * @param   fieldWork   フィールドワークポインタ
 * @param   inAssinID   アサインＩＤ
 * @param   inHeapID    ヒープＩＤ
 * @param   inSize      確保メモリ
 *
 * @return アロケーションしたワークポインタ
 */
//==================================================================
void *GMK_TMP_WK_AllocWork
      (FIELDMAP_WORK *fieldWork, const u32 inAssignID, const HEAPID inHeapID, const u32 inSize)
{
  GMK_TMP_WORK *tmpWork;
  tmpWork = &fieldWork->GmkTmpWork;
  if (inAssignID == GMK_TMP_NO_ASSIGN_ID){
    GF_ASSERT(0);
    return NULL;
  }
  if (tmpWork->AssignID != GMK_TMP_NO_ASSIGN_ID){
    GF_ASSERT_MSG(0,"Alrady Assigned %d\n",tmpWork->AssignID);
    return NULL;
  }
  
  tmpWork->AssignID = inAssignID;
  tmpWork->Work = GFL_HEAP_AllocClearMemory( inHeapID, inSize );
  return tmpWork->Work;
}

//==================================================================
/**
 * ギミックテンポラリワークメモリ解放関数
 *
 * @note  使用の際はgmk_tmp_wk.hを参照のこと
 *
 * @param   fieldWork   フィールドワークポインタ
 * @param   inAssinID   アサインＩＤ
 * @param   inHeapID    ヒープＩＤ
 * @param   inSize      確保メモリ
 */
//==================================================================
void GMK_TMP_WK_FreeWork
      (FIELDMAP_WORK *fieldWork, const u32 inAssignID)
{
  GMK_TMP_WORK *tmpWork;
  tmpWork = &fieldWork->GmkTmpWork;
  if (inAssignID == GMK_TMP_NO_ASSIGN_ID){
    GF_ASSERT(0);
    return;
  }
  if (tmpWork->AssignID != inAssignID){
    GF_ASSERT_MSG(0,"AssignID Diff %d_%d\n",tmpWork->AssignID, inAssignID);
  }

  GFL_HEAP_FreeMemory( tmpWork->Work );
  tmpWork->AssignID = GMK_TMP_NO_ASSIGN_ID;
}


//==================================================================
/**
 * ギミックテンポラリワーク取得関数
 *
 * @note  使用の際はgmk_tmp_wk.hを参照のこと
 *
 * @param   fieldWork   フィールドワークポインタ
 * @param   inAssinID   アサインＩＤ
 *
 * @return void*    メモリポインタ
 */
//==================================================================
void *GMK_TMP_WK_GetWork(FIELDMAP_WORK *fieldWork, const u32 inAssignID)
{
  GMK_TMP_WORK *tmpWork;
  tmpWork = &fieldWork->GmkTmpWork;
  if (inAssignID == GMK_TMP_NO_ASSIGN_ID){
    GF_ASSERT(0);
    return NULL;
  }
  if (inAssignID != tmpWork->AssignID){
    GF_ASSERT(0);
    return NULL;
  }
  return tmpWork->Work;
}

//==================================================================
/**
 * 通常描画
 *
 * @param   fieldWork   フィールドワークポインタ
 *
 * @return none
 */
//==================================================================
//　topフレームでの描画処理
static void Draw3DNormalMode_top( FIELDMAP_WORK * fieldWork )
{
  FIELD_FOG_Reflect( fieldWork->fog );
	FIELD_LIGHT_Reflect( fieldWork->light, FALSE );

	GFL_G3D_CAMERA_Switching( fieldWork->g3Dcamera );
	GFL_G3D_LIGHT_Switching( fieldWork->g3Dlightset );

	FLDMAPPER_Draw(
      fieldWork->g3Dmapper, fieldWork->g3Dcamera, FLDMAPPER_DRAW_TOP );
}

//　tailフレームでの描画処理
static void Draw3DNormalMode_tail( FIELDMAP_WORK * fieldWork )
{
  if( fieldWork->fldMsgBG ){
    FLDMSGBG_PrintG3D( fieldWork->fldMsgBG );
  }
  
  SET_CHECK("update_tail:mapper draw");
  FLDMAPPER_Draw(
      fieldWork->g3Dmapper, fieldWork->g3Dcamera, FLDMAPPER_DRAW_TAIL );
  
  SET_CHECK("update_tail:wipe draw");
	FIELDSKILL_MAPEFF_Draw( fieldWork->fieldskill_mapeff );
  
#ifdef PM_DEBUG
  // デバックカメラ表示 (WIPE用)
  if( FIELD_CAMERA_DEBUG_IsWipeDraw( fieldWork->camera_control ) ){
    FIELD_CAMERA_DEBUG_Draw( fieldWork->camera_control );
  }
#endif
  
  SET_CHECK("update_tail:proj calc");

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
*/				
		org_pm = *m;
		pm = org_pm;

		pm._32 += FX_Mul( pm._22, fieldWork->pro_mat_z_offs );
    
		NNS_G3dGlbSetProjectionMtx(&pm);
		NNS_G3dGlbFlush();		  //　ジオメトリコマンドを転送
    NNS_G3dGeFlushBuffer(); // 転送まち

#ifdef PM_DEBUG
    if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG) &&
        (GFL_UI_KEY_GetCont() & PAD_BUTTON_A) )
    {
    }
    else
#endif
    
    {
      SET_CHECK("update_tail:fldeff draw");
      FLDEFF_CTRL_Draw( fieldWork->fldeff_ctrl );
    
      SET_CHECK("update_tail:g3dobj draw");
      FLD_G3DOBJ_CTRL_Draw( fieldWork->fieldG3dObjCtrl );
      
      SET_CHECK("update_tail:bbd draw");
      GFL_BBDACT_Draw( fieldWork->mainBbdActSys,
          fieldWork->g3Dcamera, fieldWork->g3Dlightset );
      
      SET_CHECK("update_tail:eff bbd draw");
      GFL_BBDACT_Draw( fieldWork->effBbdActSys,
          fieldWork->g3Dcamera, fieldWork->g3Dlightset );
    }
    
#ifdef  PM_DEBUG
  if(fieldWork->debugWork){ FIELD_DEBUG_Draw( fieldWork->debugWork ); }
#endif
    
    if(fieldWork->union_eff != NULL){
      UNION_EFF_SystemDraw( fieldWork->union_eff);
    }
    
    SET_CHECK("update_tail:bbd draw 2");

		pm = org_pm;
		pm._32 += FX_Mul( pm._22, fieldWork->pro_mat_z_offs / 2 );
		NNS_G3dGlbSetProjectionMtx(&pm);
		NNS_G3dGlbFlush();		  //　ジオメトリコマンドを転送
    NNS_G3dGeFlushBuffer(); // 転送まち

    GFL_BBDACT_Draw( fieldWork->subBbdActSys,
        fieldWork->g3Dcamera, fieldWork->g3Dlightset );
    
		NNS_G3dGlbSetProjectionMtx(&org_pm);
		NNS_G3dGlbFlush();		//　ジオメトリコマンドを転送
  }
  
  // フィールドマップ用制御タスクシステム
  SET_CHECK("update_tail:FLDMAPFUNC_Sys_Draw3D");
	FLDMAPFUNC_Sys_Draw3D( fieldWork->fldmapFuncSys );
	
  SET_CHECK("update_tail:weather 3d");
  FIELD_WEATHER_3DWrite( fieldWork->weather_sys );	// 天気描画処理

  SET_CHECK("update_tail: exp obj draw");
  //フィールド拡張3ＤＯＢＪ描画
  FLD_EXP_OBJ_Draw( fieldWork->ExpObjCntPtr );
  
#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG){
#if 0    
    if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_L){
      u32 rest;
      rest = 0;
      NNS_GfdDumpLnkTexVramManagerEx( DbgVramDumpCallBack, DbgVramDumpCallBack, &rest );
      NOZOMU_Printf( "3DVram rest = 0x%x\n",rest);

//      DEBUG_CreateCamShakeEvt(fieldWork->gsys, 0, 10, 3, 10, 0,1,0,5);
//      FLD3D_CI_CallCutIn(fieldWork->gsys, fieldWork->Fld3dCiPtr, 0);
    }
/**    
    else if (GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT){
      FLD3D_CI_CallEncCutIn(fieldWork->gsys, fieldWork->Fld3dCiPtr);
    }else if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_R){
      FLD3D_CI_FlySkyCameraDebug(
          fieldWork->gsys, fieldWork->Fld3dCiPtr, fieldWork->camera_control,
          FIELDMAP_GetFieldPlayer(fieldWork),
          FIELDMAP_GetFldNoGridMapper( fieldWork ) );
    }
*/    
#endif    
  }
#endif  //PM_DEBUG  
  
  SET_CHECK("update_tail:prt & ci draw");
  FLD_PRTCL_Main();
  FLD3D_CI_Draw( fieldWork->Fld3dCiPtr );
}

//==================================================================
/**
 * フィールド3Ｄカットイン時描画
 *
 * @param   fieldWork   フィールドワークポインタ
 *
 * @return none
 */
//==================================================================
static void Draw3DCutinMode(FIELDMAP_WORK * fieldWork)
{
  G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,0,FALSE);

  FLD_PRTCL_Main();
  FLD3D_CI_Draw( fieldWork->Fld3dCiPtr );
}
#if 0
//==================================================================
/**
 * フィールドスクリーンキャプチャ描画
 *
 * @param   fieldWork   フィールドワークポインタ
 *
 * @return none
 */
extern void ENCEFF_Draw3D(void);
//==================================================================
static void Draw3DScrnTexMode(FIELDMAP_WORK * fieldWork)
{
  G3X_SetClearColor(GX_RGB(31,31,31),31,0x7fff,0,FALSE);

	ENCEFF_Draw3D();
}
#endif
//==================================================================
/**
 * エンカウントエフェクト描画
 *
 * @param   fieldWork   フィールドワークポインタ
 *
 * @return none
 */
//==================================================================
static void DrawEncEff(FIELDMAP_WORK * fieldWork)
{
  G3X_SetClearColor(GX_RGB(16,16,16),31,0x7fff,0,FALSE);

	ENCEFF_Draw(fieldWork->EncEffCntPtr);
}


//==================================================================
/**
 * 3D描画モード取得
 *
 * @param   fieldWork   フィールドワークポインタ
 *
 * @return DRAW3DMODE   描画モード
 */
//==================================================================
DRAW3DMODE FIELDMAP_GetDraw3DMode(FIELDMAP_WORK *fieldWork)
{
  return fieldWork->Draw3DMode;
}

//==================================================================
/**
 * 3D描画モードセット
 *
 * @param   fieldWork   フィールドワークポインタ
 * @param   DRAW3DMODE  描画モード
 *
 * @return  none
 */
//==================================================================
void FIELDMAP_SetDraw3DMode(FIELDMAP_WORK *fieldWork, DRAW3DMODE mode)
{
  fieldWork->Draw3DMode = mode;
}

//==================================================================
/**
 * カットイン管理ポインタ取得
 *
 * @param   fieldWork   フィールドワークポインタ
 *
 * @return  FLD3D_CI_PTR    カットイン管理ポインタ
 */
//==================================================================
FLD3D_CI_PTR FIELDMAP_GetFld3dCiPtr(FIELDMAP_WORK *fieldWork)
{
  return fieldWork->Fld3dCiPtr;
}

//==================================================================
/**
 * エンカウントエフェクト管理ポインタ取得
 *
 * @param   fieldWork   フィールドワークポインタ
 *
 * @return  ENCEFF_CNT_PTR    エンカウントエフェクト管理ポインタ
 */
//==================================================================
ENCEFF_CNT_PTR FIELDMAP_GetEncEffCntPtr(FIELDMAP_WORK *fieldWork)
{
  return fieldWork->EncEffCntPtr;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールド技　マップ効果ワーク取得
 *
 *	@param	fieldWork   ワーク
 */
//-----------------------------------------------------------------------------
FIELDSKILL_MAPEFF * FIELDMAP_GetFieldSkillMapEffect( FIELDMAP_WORK * fieldWork )
{
  return fieldWork->fieldskill_mapeff;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドscenearea取得
 *
 *	@param	fieldWork   ワーク
 */
//-----------------------------------------------------------------------------
FLD_SCENEAREA* FIELDMAP_GetFldSceneArea( FIELDMAP_WORK * fieldWork )
{
  return fieldWork->sceneArea;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップフェードリクエストセット
 *
 *	@param	fieldWork   ワーク
 *	@param  BOOL
 */
//-----------------------------------------------------------------------------
void FIELDMAP_SetMapFadeReqFlg( FIELDMAP_WORK * fieldWork, const BOOL inReq )
{
  fieldWork->MapFadeReq = inReq;
}

//----------------------------------------------------------------------------
/**
 *	@brief  マップフェードリクエストチェック
 *
 *	@param	fieldWork   ワーク
 *	@return  BOOL     TRUE：リクエスト中
 */
//-----------------------------------------------------------------------------
BOOL FIELDMAP_CheckMapFadeReqFlg( FIELDMAP_WORK * fieldWork  )
{
  return fieldWork->MapFadeReq;
}



//----------------------------------------------------------------------------
/**
 *	@brief  SOUND、BGMの読み込み中チェック   BGMの再生やSEの再生が出来ません。
 *
 *	@param	fieldWork 
 *
 *	@retval TRUE    読み込み中
 *	@retval FALSE   読み込んでない
 */
//-----------------------------------------------------------------------------
BOOL FIELDMAP_CheckCanSoundPlay( const FIELDMAP_WORK* fieldWork )
{
  return PMSND_IsLoading();
}

//----------------------------------------------------------------------------
/**
 *	@brief  イベント動作中かチェック
 *
 *	@param	fieldWork   ワーク
 *
 *	@retval TRUE    動作中
 *	@retval FALSE   動作してない（fieldmap_ctrlの更新が行われている）
 */
//-----------------------------------------------------------------------------
BOOL FIELDMAP_CheckDoEvent( const FIELDMAP_WORK* fieldWork )
{
  return GAMESYSTEM_IsEventExists(fieldWork->gsys);
}

//----------------------------------------------------------------------------
/**
 *	@brief  季節ごとの時間帯を取得
 *
 *	@param	fieldWork     フィールドワーク
 *
 *	@return 時間帯
 */
//-----------------------------------------------------------------------------
u32 FIELDMAP_GetSeasonTimeZone( const FIELDMAP_WORK * fieldWork )
{
  return PM_RTC_GetTimeZone( GAMEDATA_GetSeasonID(fieldWork->gamedata) );
}

//-----------------------------------------------------------------------------
/**
 * @brief 季節を表示中かどうかを取得する
 *
 * @param fieldWork
 *
 * @return 季節を表示中なら TRUE
 *         そうでなければ FALSE
 */
//-----------------------------------------------------------------------------
BOOL FIELDMAP_CheckSeasonDispFlag( const FIELDMAP_WORK * fieldWork )
{
  return fieldWork->seasonDispFlag;
}

//-----------------------------------------------------------------------------
/**
 * @brief 季節表示中フラグを設定する
 *
 * @param fieldWork
 * @param flag
 */
//-----------------------------------------------------------------------------
void FIELDMAP_SetSeasonDispFlag( FIELDMAP_WORK * fieldWork, BOOL flag )
{
  fieldWork->seasonDispFlag = flag;
}


//==================================================================
/**
 * 更新処理フックフラグセット関数
 *
 * @param   fieldWork   フィールドワークポインタ
 * @param   inFlg       フックフラグ　TRUEで処理をフック
 *
 * @return none
 */
//==================================================================
void FIELDMAP_SetMainFuncHookFlg(FIELDMAP_WORK * fieldWork, const BOOL inFlg)
{
  fieldWork->MainHookFlg = inFlg;
}

//==================================================================
/**
 * 顔アップワークポインタへのポインタ取得
 *
 * @param   fieldWork   フィールドワークポインタ
 *
 * @return  ENCEFF_CNT_PTR    エンカウントエフェクト管理ポインタ
 */
//==================================================================
FACEUP_WK_PTR *FIELDMAP_GetFaceupWkPtrAdr(FIELDMAP_WORK *fieldWork)
{
  return &fieldWork->FaceUpWkPtr;
}

//----------------------------------------------------------------------------
/**
 *	@brief  群集ワークの設定
 *
 *	@param	fieldWork   フィールドワーク
 *	@param	crowdpeople 群集ワーク
 */
//-----------------------------------------------------------------------------
void FIELDMAP_SetCrowdPeopleSys( FIELDMAP_WORK * fieldWork, FIELD_CROWD_PEOPLE * crowdpeople )
{
  fieldWork->crowdpeople = crowdpeople;
}

//----------------------------------------------------------------------------
/**
 *	@brief  群集ワークの取得
 *
 *	@param	fieldWork   フィールドワーク
 *
 *	@return 群集ワーク
 */
//-----------------------------------------------------------------------------
FIELD_CROWD_PEOPLE * FIELDMAP_GetCrowdPeopleSys( const FIELDMAP_WORK * fieldWork )
{
  return fieldWork->crowdpeople;
}


//==================================================================
/**
 * @brief 動作モデル描画時のY方向オフセット取得
 */
//==================================================================
fx32 FIELDMAP_GetBbdActYOffs( const FIELDMAP_WORK * fieldmap )
{
  return fieldmap->bbdact_y_offs;
}

//==================================================================
/**
 * @brief 動作モデル描画時のY方向オフセット計算
 */
//==================================================================
static fx32 calcBbdActYOffset( const FIELDMAP_WORK * fieldmap )
{
  u16 angle;
  fx32 offs;
  angle = *FIELD_CAMERA_GetAnglePitchAddress( fieldmap->camera_control );
  offs = MMDL_BBD_OFFS_POS_Y + FX_Mul( MMDL_BBD_OFFS_Y_VALUE, MATH_ABS(FX_CosIdx(angle)) );
  if ( offs < FX32_CONST(-2) ) {
    offs = FX32_CONST(-2);
  }
  return offs;
}

//==================================================================
//==================================================================
#ifdef PM_DEBUG
static void DbgVramDumpCallBack( u32 addr, u32 szByte, void* pUserData )
{
    // 合計サイズを計算。
    (*((u32*)pUserData)) += szByte;
}

#endif


//==================================================================
/**
 * @brief ゾーンごとの射影補正値を取り出す
 * @date  2010.05.28
 * @author  tamada GAMEFREAK inc.
 *
 * 自転車めり込み問題への対処として、人物ビルボードなどの射影調整を
 * おこなった結果、砂漠の表現に問題が出たため、今まで一律に適用していた
 * 射影の値を場所ごとに可変とした。
 * 本来は外部化した値を取得する仕組みを作るべきだが、時期が時期なので
 * 直接ここに作成する。
 */
//==================================================================
static fx32 fldmap_getProjMatZOffsValue( u16 zone_id )
{
  if ( ZONEDATA_GetProjectionType( zone_id ) == TRUE )
  { //通常の射影オフセット（2010.05.28新規）
    return PROJ_MAT_Z_OFS;
  }
  else
  { //指定したゾーンによっては以前の射影オフセット
    return PROJ_MAT_Z_OFS_DESERT;
  }
}

//==================================================================
/**
 * @brief 影をだすかどうかの判定処理
 * @date  2010.06.15
 * @author  tamada
 * 本来はzonedata.cに外部化した値を取得する仕組みを作るべきだが、
 * 時期が時期であること、常駐メモリをなるべく消費したくないこと、
 * の２点を考慮して直接ここに作成する。
 */
//==================================================================
static BOOL zonedata_GetShadowUse( u16 zone_id )
{
  if ( zone_id == ZONE_ID_C04GYM0101 ) return FALSE;
  if ( ZONEDATA_IsColosseum(zone_id) == TRUE) return FALSE;
  return TRUE;
}

//==================================================================
/**
 * @brief 環境音SEを使用しているかどうかの判定処理
 * @date  2010.06.20
 * @author  tamada
 * @return  BOOL FIELD_INITの中でループSEを鳴らしている箇所の場合、TRUEとなる
 */
//==================================================================
static BOOL zonedata_IsEnvSE( u16 zone_id )
{
  if ( zone_id == ZONE_ID_C04GYM0101 ) return TRUE;
  if ( zone_id == ZONE_ID_C09R0101 ) return TRUE;
  if ( zone_id == ZONE_ID_C09R0201 ) return TRUE;
  if ( zone_id == ZONE_ID_C09R0301 ) return TRUE;
  return FALSE;
}

