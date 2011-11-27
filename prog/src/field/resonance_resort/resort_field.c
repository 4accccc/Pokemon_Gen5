//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_field.c
 *	@brief  レゾナンスリゾートフィールドメインソース
 *	@author	Toru=Nagihashi
 *	@data		2011.02.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//system
#include <gflib.h>
#include "system/gfl_use.h"
#include "../fieldmap.h"
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "../field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "../gmk_tmp_wk.h"
#include "arc/fieldmap/zone_id.h" //for ZONE_ID_
#include "system/prof_word.h"
#include "savedata/medalbox.h"

//script
#include "../script.h"     //for SCRIPT_CallScript

//archive
#include "arc/arc_def.h"
#include "arc/fieldmap/wbt_gmk.naix"
#include "arc/resonance_resort_graphic.naix"
#include "arc/script_message.naix"
#include "msg/script/msg_c16.h"

//SE
#include "sound/pm_sndsys.h"

//resort
#include "savedata/resonance_resort_sv.h"
#include "field/resonance_resort_data.h"
#include "resort_people.h"
#include "resort_data_manager.h"
#include "resort_npc.h"
#include "resort_binary.h"
#include "scrcmd_resort_def.h"
#include "resort_sys.h"
#include "resort_util.h"
#include "../../../resource/fldmapdata/flagwork/flag_define.h"
#include "resort_debug.h"

//mine
#include "resort_field.h"


#ifdef PM_DEBUG
#define DEBUG_RESORT_FIELD_PRINT_ON
//#define DEBUG_EFFECT_MOVE
#endif

#if defined(DEBUG_RESORT_FIELD_PRINT_ON) && ( defined(DEBUG_ONLY_FOR_nagihashi_toru) || defined(DEBUG_ONLY_FOR_shimoyamada) )
#define RESORT_FIELD_Printf(...)  OS_TFPrintf(0,__VA_ARGS__)
#else
#define RESORT_FIELD_Printf(...)  /* */
#endif

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define RESORT_FIELD_NPC_MAX     (32)

//=============================================================================
/**
 *          バッファ管理
 */
//=============================================================================
//-------------------------------------
///	バッファ管理
//=====================================
typedef struct _RESORT_FIELD_BUFF RESORT_FIELD_BUFF;

//-------------------------------------
///	初期化構造体
//=====================================
typedef struct
{
  RESORT_RECV_BUFF *p_recv_buff;
  RESORT_RECV_BUFF *p_recv_tmp_buff;
  RESORT_PROXY_BUFF *p_proxy_buff;
  void *p_wk_adrs;      //コールバックに与える引数
  BOOL (*UpdateDataCallback)( void *p_wk_adrs, void *p_data_adrs, u32 people_type, RESORT_RECV_DATA_UPDATE update ); //データに更新があれば更新間隔ごとに呼ばれる
  u32 update_interval; //更新間隔
}RESORT_FIELD_BUFF_INIT_PARAM;

static RESORT_FIELD_BUFF * RESORT_FIELD_BUFF_Init( HEAPID heapID, const RESORT_FIELD_BUFF_INIT_PARAM *cp_param );
static void RESORT_FIELD_BUFF_Exit( RESORT_FIELD_BUFF *p_wk );
static void RESORT_FIELD_BUFF_Main( RESORT_FIELD_BUFF *p_wk );
static void RESORT_FIELD_BUFF_ChangeCallBack( RESORT_FIELD_BUFF *p_wk, BOOL (*UpdateDataCallback)( void *, void *, u32 , RESORT_RECV_DATA_UPDATE ) );

//=============================================================================
/**
 *          群集管理
 */
//=============================================================================
//-------------------------------------
///	群集管理
//=====================================
typedef enum
{
  RESORT_CROWD_MOVE_START,    //マップチェンジでここにきた
  RESORT_CROWD_MOVE_RESTART,  //現在のMMDLを動作再開  セーブ復帰、メニュー復帰等
}RESORT_CROWD_MOVE;

//-------------------------------------
///	群集管理
//=====================================
typedef struct _RESORT_CROWD_PEOPLE RESORT_CROWD_PEOPLE;

static RESORT_CROWD_PEOPLE * RESORT_CROWD_PEOPLE_Init( HEAPID heapID, MMDLSYS* p_mmdlsys, const FIELD_PLAYER* cp_player, const RESORT_BINARY_SHOP *cp_shop, const RESORT_BINARY_INSTITUTION *cp_institution, RESORT_PEOPLE_ALL_DATA *p_all_people );
static void RESORT_CROWD_PEOPLE_Exit( RESORT_CROWD_PEOPLE *p_wk );
static void RESORT_CROWD_PEOPLE_Main( RESORT_CROWD_PEOPLE *p_wk, const FIELDMAP_WORK* cp_fieldmap );
static void RESORT_CROWD_PEOPLE_SetUp( RESORT_CROWD_PEOPLE *p_wk, RESORT_CROWD_MOVE move );

static void RESORT_CROWD_PEOPLE_Add( RESORT_CROWD_PEOPLE *p_wk, RESORT_PEOPLE *p_people );
static void RESORT_CROWD_PEOPLE_Delete( RESORT_CROWD_PEOPLE *p_wk, RESORT_PEOPLE *p_people );
static void RESORT_CROWD_PEOPLE_ChangeOBJCodeReq( RESORT_CROWD_PEOPLE *p_wk, RESORT_PEOPLE *p_people );

//=============================================================================
/**
 *          騒音管理
 */
//=============================================================================
//-------------------------------------
///	騒音管理
//=====================================
typedef struct _RESORT_CROWD_NOISE  RESORT_CROWD_NOISE;

static RESORT_CROWD_NOISE * RESORT_CROWD_NOISE_Init( HEAPID heapID, FLDMSGBG* p_msgbg, RESORT_ABSTRUCT_MANAGER *p_abstruct, RESORT_CITY_DATA *p_city, RESORT_PEOPLE_ALL_DATA *p_people_all, BOOL enable );
static void RESORT_CROWD_NOISE_Exit( RESORT_CROWD_NOISE *p_wk );
static void RESORT_CROWD_NOISE_Main( RESORT_CROWD_NOISE *p_wk, const FIELDMAP_WORK* cp_fieldmap );
static void RESORT_CROWD_NOISE_Clear( RESORT_CROWD_NOISE *p_wk );
static void RESORT_CROWD_NOISE_Start( RESORT_CROWD_NOISE *p_wk );

static void RESORT_CROWD_NOISE_ForceRequest( RESORT_CROWD_NOISE *p_wk, BOOL is_visible );
static void RESORT_CROWD_NOISE_SetParadeMode( RESORT_CROWD_NOISE *p_wk );
//=============================================================================
/**
 *          人物管理
 */
//=============================================================================
//-------------------------------------
///	人物管理
//=====================================
typedef struct _RESORT_FIELD_PEOPLE RESORT_FIELD_PEOPLE;

static RESORT_FIELD_PEOPLE * RESORT_FIELD_PEOPLE_Init( HEAPID heapID, RESORT_SYSTEM *p_sys, MMDLSYS *p_mmdlsys, const FIELD_PLAYER* cp_player, BOOL is_tutorial, BOOL is_mapchange );
void RESORT_FIELD_PEOPLE_Setup( RESORT_FIELD_PEOPLE *p_wk, BOOL is_tutorial, BOOL is_mapchange );
static void RESORT_FIELD_PEOPLE_Exit( RESORT_FIELD_PEOPLE *p_wk );
static void RESORT_FIELD_PEOPLE_Main( RESORT_FIELD_PEOPLE *p_wk, FIELDMAP_WORK* p_fieldmap );
static void RESORT_FIELD_PEOPLE_StartAdd( RESORT_FIELD_PEOPLE *p_wk );
static RESORT_PEOPLE_SYS * RESORT_FIELD_PEOPLE_GetPeopleSys( RESORT_FIELD_PEOPLE *p_wk );
static void RESORT_FIELD_PEOPLE_DeleteCrowd( RESORT_FIELD_PEOPLE *p_wk, RESORT_PEOPLE *p_people );
static RESORT_CROWD_PEOPLE * RESORT_FIELD_PEOPLE_GetCrowdPeople( RESORT_FIELD_PEOPLE *p_wk );

static void RESORT_FIELD_PEOPLE_SetParadePeoplePos( RESORT_FIELD_PEOPLE *p_wk, RESORT_SYSTEM *p_sys, RESORT_NPC_SYS *p_npc_sys );

//=============================================================================
/**
 *          ギミック演出管理
 */
//=============================================================================
//-------------------------------------
///	ギミック演出ワーク
//=====================================
typedef struct _RESORT_FIELD_EFFECT RESORT_FIELD_EFFECT;

static RESORT_FIELD_EFFECT * RESORT_FIELD_EFFECT_Init( HEAPID heapID, FLD_EXP_OBJ_CNT_PTR p_exp_obj, FIELD_CAMERA *p_camera );
static void RESORT_FIELD_EFFECT_Exit( RESORT_FIELD_EFFECT *p_wk );
static void RESORT_FIELD_EFFECT_Main( RESORT_FIELD_EFFECT *p_wk );

typedef enum
{
  RESORT_FIELD_EFFECT_ID_CONFETTI,
  RESORT_FIELD_EFFECT_ID_PARADE,

  RESORT_FIELD_EFFECT_ID_MAX
}RESORT_FIELD_EFFECT_ID;
static void RESORT_FIELD_EFFECT_Start( RESORT_FIELD_EFFECT *p_wk, RESORT_FIELD_EFFECT_ID id );
static BOOL RESORT_FIELD_EFFECT_IsStop( const RESORT_FIELD_EFFECT *cp_wk, RESORT_FIELD_EFFECT_ID id );
static void RESORT_FIELD_EFFECT_Stop( RESORT_FIELD_EFFECT *p_wk, RESORT_FIELD_EFFECT_ID id );

//=============================================================================
/**
 *          その他
 */
//=============================================================================
static void RESORT_FIELD_AppearFun( RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata, HEAPID heapID );
static void RESORT_FIELD_AppearCeleb( RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata, HEAPID heapID );

//=============================================================================
/**
 *				構造体
*/
//=============================================================================
//-------------------------------------
///	レゾナンスリゾートメインワーク
//=====================================
typedef struct
{
  RESORT_FIELD_PEOPLE *p_people_sys;
  RESORT_NPC_SYS      *p_npc_sys;
  RESORT_SYSTEM       *p_sys;
  RESORT_CROWD_NOISE  *p_noise;
  RESORT_FIELD_EFFECT *p_effect;
} RESORT_FIELD_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミック初期化
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_FIELD_Setup( FIELDMAP_WORK *p_fieldmap )
{
  const HEAPID heapID = FIELDMAP_GetHeapID(p_fieldmap);
  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork(p_fieldmap) );
  RESORT_WORK *p_resort_wk = GAMEDATA_GetReosrtWork(p_gamedata);

  BOOL is_tutorial = TRUE;

  //汎用ワーク確保
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID, heapID, sizeof(RESORT_FIELD_WORK));

  //GFL_OVERLAY_Load( FS_OVERLAY_ID(prof_word) );


  //リゾートシステム作成
  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
    RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );
    void **pp_buff = GAMEDATA_GetResortBuffer( p_gamedata );

    RESORT_ALL_DATA_PARAM param;
    GFL_STD_MemClear( &param, sizeof(RESORT_ALL_DATA_PARAM) );
    param.p_all_people    = RESONANCE_RESORT_SV_GetPeopleAllData(p_resort_sv);
    param.p_recv_sv_buff  = RESONANCE_RESORT_SV_GetDataBuff(p_resort_sv);
    param.p_recv_tmp_buff = *pp_buff;
    param.p_proxy_buff    = RESONANCE_RESORT_SV_GetProxyBuff(p_resort_sv);
    param.p_city_data     = RESONANCE_RESORT_SV_GetCityData(p_resort_sv);

    p_wk->p_sys = RESORT_SYSTEM_Init( &param, p_resort_wk, heapID );
  }

  //人物追加フラグ取得
  {
    RESORT_CITY_DATA *p_city = RESORT_SYSTEM_GetCityData( p_wk->p_sys );
    is_tutorial = RESORT_CITY_DATA_GetData(p_city,RESORT_CITY_DATA_PARAM_IS_TUTORIAL,NULL);
  }
  
  //街へ来たとき１日１回だけセレブが出現することがある
  RESORT_FIELD_AppearCeleb( p_wk->p_sys, p_gamedata, heapID );

  //街へ来たとき１日１回だけファンが増加する
  RESORT_FIELD_AppearFun( p_wk->p_sys, p_gamedata, heapID );

  //人物システム
  {
    BOOL is_mapchange = RESORT_WORK_GetData(p_resort_wk, RESORT_WORK_GET_PARAM_IS_MAPCHANGE );
    p_wk->p_people_sys = RESORT_FIELD_PEOPLE_Init( heapID, p_wk->p_sys, GAMEDATA_GetMMdlSys(p_gamedata), FIELDMAP_GetFieldPlayer(p_fieldmap), is_tutorial, is_mapchange );
    RESORT_FIELD_PEOPLE_Setup( p_wk->p_people_sys, is_tutorial, is_mapchange );
  }

  //NPCシステム
  {
    RESORT_NPC_SYS_PARAM param;
    GFL_STD_MemClear( &param, sizeof(RESORT_NPC_SYS_PARAM) );
    param.p_mmdlsys = GAMEDATA_GetMMdlSys(p_gamedata);
    param.max   = RESORT_FIELD_NPC_MAX;
    param.p_city = RESORT_SYSTEM_GetCityData( p_wk->p_sys );
    param.placeID  = RESORT_MAP_ZoneIDToPlaceID( FIELDMAP_GetZoneID(p_fieldmap) );
    p_wk->p_npc_sys = RESORT_NPC_SYS_Init( &param, heapID );
  }

  //騒音
  {
    p_wk->p_noise = RESORT_CROWD_NOISE_Init( heapID, FIELDMAP_GetFldMsgBG(p_fieldmap), RESORT_SYSTEM_GetAbstructManager( p_wk->p_sys ), RESORT_SYSTEM_GetCityData( p_wk->p_sys ), RESORT_SYSTEM_GetPeopleAllData(p_wk->p_sys), !is_tutorial );
  }

  //演出
  {
    FLD_EXP_OBJ_CNT_PTR p_exp_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
    FIELD_CAMERA *p_camera  = FIELDMAP_GetFieldCamera( p_fieldmap );
    p_wk->p_effect = RESORT_FIELD_EFFECT_Init( heapID, p_exp_obj, p_camera );
  }

  RESORT_FIELD_Printf( "レゾナンスsetup\n" );
}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミック破棄
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_FIELD_End( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);

  RESORT_FIELD_EFFECT_Exit( p_wk->p_effect );
  RESORT_CROWD_NOISE_Exit( p_wk->p_noise );
  RESORT_NPC_SYS_Exit( p_wk->p_npc_sys );
  RESORT_FIELD_PEOPLE_Exit( p_wk->p_people_sys );
  RESORT_SYSTEM_Exit( p_wk->p_sys );

  //GFL_OVERLAY_Unload( FS_OVERLAY_ID(prof_word) );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミックメイン処理
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_FIELD_Move( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);

  RESORT_FIELD_PEOPLE_Main( p_wk->p_people_sys, p_fieldmap );
  RESORT_NPC_SYS_Main( p_wk->p_npc_sys, p_fieldmap );
  RESORT_CROWD_NOISE_Main( p_wk->p_noise, p_fieldmap );
  RESORT_FIELD_EFFECT_Main( p_wk->p_effect );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾート
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールド
 *
 *	@return 人物システム
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE_SYS * RESORT_FIELD_GetPeopleSys( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  return RESORT_FIELD_PEOPLE_GetPeopleSys( p_wk->p_people_sys );
}
//----------------------------------------------------------------------------
/**
 *	@brief  NPCシステムを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *
 *	@return NPCシステム
 */
//-----------------------------------------------------------------------------
RESORT_NPC_SYS * RESORT_FIELD_GetNpcSys( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  return p_wk->p_npc_sys;
}
//----------------------------------------------------------------------------
/**
 *	@brief  リゾートシステムを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールドマップ
 *
 *	@return リゾートシステム
 */
//-----------------------------------------------------------------------------
RESORT_SYSTEM * RESORT_FIELD_GetResortSys( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  return p_wk->p_sys;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フィールド上の群集の人を消す（p_peopleの人が群集にいなかったら何もしない）
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *	@param  void* p_people  (RESORT_PEOPLE*)
 */
//-----------------------------------------------------------------------------
void RESORT_FIELD_DeleteCrowdPeople( FIELDMAP_WORK *p_fieldmap, void* p_people )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  RESORT_FIELD_PEOPLE_DeleteCrowd( p_wk->p_people_sys, p_people );
}
//----------------------------------------------------------------------------
/**
 *	@brief  演出開始
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *	@param	id  演出ID
 */
//-----------------------------------------------------------------------------
void RESORT_FIELD_StartEffect( FIELDMAP_WORK *p_fieldmap, u32 id )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  RESORT_FIELD_EFFECT_Start( p_wk->p_effect, id );
}

//----------------------------------------------------------------------------
/**
 *	@brief  演出待ち
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *	@param	id  演出ID
 *
 *	@retval TRUE終了  FLASEまだ
 */
//-----------------------------------------------------------------------------
BOOL RESORT_FIELD_WaitEffect( FIELDMAP_WORK *p_fieldmap, u32 id )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  return RESORT_FIELD_EFFECT_IsStop( p_wk->p_effect, id );
}

//----------------------------------------------------------------------------
/**
 *	@brief  演出終了
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *	@param	id  演出ID
 */
//-----------------------------------------------------------------------------
void RESORT_FIELD_EndEffect( FIELDMAP_WORK *p_fieldmap, u32 id )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  RESORT_FIELD_EFFECT_Stop( p_wk->p_effect, id );
}
//----------------------------------------------------------------------------
/**
 *	@brief  止めていたのを消す
 *
 *	@param	FIELDMAP_WORK *p_fieldmap ワーク
 */
//-----------------------------------------------------------------------------
void RESORT_FIELD_StartPeople( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  RESORT_FIELD_PEOPLE_StartAdd( p_wk->p_people_sys );
  RESORT_CROWD_NOISE_Start( p_wk->p_noise );
}
//----------------------------------------------------------------------------
/**
 *	@brief  パレードモード設定
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 */
//-----------------------------------------------------------------------------
void RESORT_FIELD_SetParade( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  RESORT_CROWD_NOISE_SetParadeMode( p_wk->p_noise );
  RESORT_FIELD_PEOPLE_SetParadePeoplePos( p_wk->p_people_sys, p_wk->p_sys, p_wk->p_npc_sys );
}
//----------------------------------------------------------------------------
/** 
 *	@brief  騒音強制リクエスト
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *	@param	is_visible  TRUE表示  FALSE消す
 */
//-----------------------------------------------------------------------------
void RESORT_FIELD_SetNoiseReq( FIELDMAP_WORK *p_fieldmap, BOOL is_visible )
{
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);
  RESORT_CROWD_NOISE_ForceRequest( p_wk->p_noise, is_visible );
}

//=============================================================================
/**
 *          バッファ管理
 */
//=============================================================================
//-------------------------------------
///	定数
//=====================================

//-------------------------------------
///	バッファ管理
//=====================================
struct _RESORT_FIELD_BUFF
{
  RESORT_FIELD_BUFF_INIT_PARAM param;
  RESORT_RECV_BUFF  *p_sv_buff;     //セーブにあるバッファ
  RESORT_RECV_BUFF  *p_recep_buff;     //迎賓館にのみ存在するバッファ
  RESORT_PROXY_BUFF *p_proxy_buff;  //セーブにある代理人バッファ
  u32               update_frame;   //更新間隔カウンタ
  BOOL              is_proxy_update; //一度しか更新がかからないフラグ
};

//-------------------------------------
///	PRIVATE
//=====================================
static void Reception_Buff_UpdateRecv( RESORT_FIELD_BUFF *p_wk );
static void Reception_Buff_UpdateProxy( RESORT_FIELD_BUFF *p_wk );

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  バッファ管理作成
 *
 *	@param	HEAPID heapID ヒープID
 *	@param	RESORT_FIELD_BUFF_INIT_PARAM *cp_param   引数
 *
 *	@return　バッファ管理
 */
//-----------------------------------------------------------------------------
static RESORT_FIELD_BUFF * RESORT_FIELD_BUFF_Init( HEAPID heapID, const RESORT_FIELD_BUFF_INIT_PARAM *cp_param )
{
  RESORT_FIELD_BUFF *p_wk = GFL_HEAP_AllocClearMemory(heapID, sizeof(RESORT_FIELD_BUFF));
  p_wk->param = *cp_param;

  //セーブからバッファを取得
  {
    p_wk->p_sv_buff     = cp_param->p_recv_buff;
    p_wk->p_proxy_buff  = cp_param->p_proxy_buff;
  }

  //迎賓館のみ存在する拡張バッファを登録
  {
    p_wk->p_recep_buff  = cp_param->p_recv_tmp_buff;
    GAMEBEACON_Set_ResortRecvBuffer( p_wk->p_recep_buff );
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バッファ管理破棄
 *
 *	@param	RESORT_FIELD_BUFF *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_BUFF_Exit( RESORT_FIELD_BUFF *p_wk )
{
  GAMEBEACON_Set_ResortRecvBuffer( NULL );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バッファ管理メイン
 *
 *	@param	RESORT_FIELD_BUFF *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_BUFF_Main( RESORT_FIELD_BUFF *p_wk )
{
  if( p_wk->update_frame++ > p_wk->param.update_interval )
  {
    Reception_Buff_UpdateRecv( p_wk );
    Reception_Buff_UpdateProxy( p_wk );
    p_wk->update_frame = 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  コールバック切り替え
 *
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_BUFF_ChangeCallBack( RESORT_FIELD_BUFF *p_wk, BOOL (*UpdateDataCallback)( void *, void *, u32 , RESORT_RECV_DATA_UPDATE ) )
{
  p_wk->param.UpdateDataCallback  = UpdateDataCallback;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信バッファのアップデート
 *
 *	@param	RESORT_FIELD_BUFF *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Reception_Buff_UpdateRecv( RESORT_FIELD_BUFF *p_wk )
{
  int i;

  //セーブ用バッファをチェック
  for( i = 0; i < RESORT_RECV_BUFF_GetMax(p_wk->p_sv_buff); i++ )
  {
    RESORT_RECV_DATA *p_recv = RESORT_RECV_BUFF_GetData( p_wk->p_sv_buff, i );
    RESORT_RECV_DATA_UPDATE update = RESORT_RECV_DATA_GetData(p_recv,RESORT_DATA_PARAM_UPDATE, NULL);
    if( !RESORT_RECV_DATA_IsEmpty( p_recv ) 
        && ( update != RESORT_RECV_DATA_UPDATE_NONE ) )
    {
      RESORT_RECV_DATA_SetData(p_recv,RESORT_DATA_PARAM_UPDATE, RESORT_RECV_DATA_UPDATE_NONE );
      if( !p_wk->param.UpdateDataCallback( p_wk->param.p_wk_adrs, p_recv, RESORT_PEOPLE_TYPE_TOURIST, update ) )
      {
        RESORT_RECV_DATA_SetData(p_recv,RESORT_DATA_PARAM_UPDATE, update );
      }
    }
  }

  //迎賓館用バッファをチェック
  for( i = 0; i < RESORT_RECV_BUFF_GetMax(p_wk->p_recep_buff); i++ )
  {
    RESORT_RECV_DATA *p_recv = RESORT_RECV_BUFF_GetData( p_wk->p_recep_buff, i );
    RESORT_RECV_DATA_UPDATE update = RESORT_RECV_DATA_GetData(p_recv,RESORT_DATA_PARAM_UPDATE, NULL );
    if( !RESORT_RECV_DATA_IsEmpty( p_recv ) 
        && ( update != RESORT_RECV_DATA_UPDATE_NONE ) )
    {
      RESORT_RECV_DATA_SetData(p_recv,RESORT_DATA_PARAM_UPDATE, RESORT_RECV_DATA_UPDATE_NONE );

      if( !p_wk->param.UpdateDataCallback( p_wk->param.p_wk_adrs, p_recv, RESORT_PEOPLE_TYPE_TOURIST, update ) )
      {
        RESORT_RECV_DATA_SetData(p_recv,RESORT_DATA_PARAM_UPDATE, update );
      }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人バッファのアップデート
 *
 *	@param	RESORT_FIELD_BUFF *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Reception_Buff_UpdateProxy( RESORT_FIELD_BUFF *p_wk )
{
  int i;

  if( p_wk->is_proxy_update )
  {
    return;
  }

  for( i = 0; i < RESORT_PROXY_BUFF_GetMax(p_wk->p_proxy_buff ); i++ )
  {
    RESORT_PROXY_DATA * p_proxy = RESORT_PROXY_BUFF_GetData( p_wk->p_proxy_buff, i );
    if( !RESORT_PROXY_DATA_IsEmpty( p_proxy ) )
    {
      p_wk->param.UpdateDataCallback( p_wk->param.p_wk_adrs, p_proxy, RESORT_PEOPLE_TYPE_PROXY, RESORT_RECV_DATA_UPDATE_FIRST );
    }
  }

  //p_wk->is_proxy_update = TRUE;
}

//=============================================================================
/**
 *          群集管理
 */
//=============================================================================
#define RESORT_CROWD_MOVE_LINE_MAX          (6)
#define RESORT_CROWD_MOVE_LINE_PEOPLE_MAX   (5)
#define RESORT_CROWD_PEOPLE_BUFFER_MAX      (RESORT_CROWD_MOVE_LINE_PEOPLE_MAX*RESORT_CROWD_MOVE_LINE_MAX)
#define RESORT_CROWD_MOVE_LIMIT_BACK        (9)
#define RESORT_CROWD_MOVE_LIMIT_FRONT       (77)

#define RESORT_CROWD_PEOPLE_WK_MAGIC_NUMBER (0x765F573)

//-------------------------------------
///	１人ずつのワーク
//=====================================
typedef struct _RESORT_CROWD_PEOPLE_WK
{
  u32 magic_number;
  BOOL  is_walk;
  RESORT_PEOPLE *p_people;
  u16 move_acmd;
  u16 move_dir;
  u16 seq;
  s16 move_grid;
  u8  is_acmd_req;
  u8  is_acmd_end;
  u16 changeObjCode;
  void (*InitMove)(struct _RESORT_CROWD_PEOPLE_WK* );
  void (*UpdateMove)(struct _RESORT_CROWD_PEOPLE_WK*, MMDLSYS*, const FIELD_PLAYER* );

  struct
  {
    s16 cnt;
    s16 sync;
    s16 max;
    s16 start_grid;
    s16 grid;
    u8  seq;
    u8  dir;
  }kyoro;

  u16 interest_grid_z[RESORT_RESIDENT_DATA_NUM];
  u16 interest_dir[RESORT_RESIDENT_DATA_NUM];
  u16 interest_seq;
  u16 interest_cnt;
  u16 interest_index;
  u32 acmd_seq;

} RESORT_CROWD_PEOPLE_WK;

static void RESORT_CROWD_PEOPLE_WK_Init( RESORT_CROWD_PEOPLE_WK *p_wk, HEAPID heapID );
static void RESORT_CROWD_PEOPLE_WK_Exit( RESORT_CROWD_PEOPLE_WK *p_wk );
static void RESORT_CROWD_PEOPLE_WK_Main( RESORT_CROWD_PEOPLE_WK *p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player );
static void RESORT_CROWD_PEOPLE_WK_SetUp( RESORT_CROWD_PEOPLE_WK *p_wk, RESORT_PEOPLE *p_people, const RESORT_BINARY_SHOP *cp_shop, const RESORT_BINARY_INSTITUTION *cp_institution, RESORT_PEOPLE_ALL_DATA *p_all_people );
static void RESORT_CROWD_PEOPLE_WK_Clear( RESORT_CROWD_PEOPLE_WK *p_wk );
static BOOL RESORT_CROWD_PEOPLE_WK_IsCompPeople( const RESORT_CROWD_PEOPLE_WK *cp_wk, const RESORT_PEOPLE *cp_people );
static BOOL RESORT_CROWD_PEOPLE_WK_IsExist( const RESORT_CROWD_PEOPLE_WK *cp_wk );
static void RESORT_CROWD_PEOPLE_WK_StartMove( RESORT_CROWD_PEOPLE_WK *p_wk, u16 acmd, u16 move_dir, u16 x );
static void RESORT_CROWD_PEOPLE_WK_StartMovePos( RESORT_CROWD_PEOPLE_WK *p_wk, u16 acmd, u16 move_dir, u16 x, u16 z );
static BOOL RESORT_CROWD_PEOPLE_WK_IsEndMove( const RESORT_CROWD_PEOPLE_WK *cp_wk );
static BOOL RESORT_CROWD_PEOPLE_WK_IsNewData( const RESORT_CROWD_PEOPLE_WK *cp_wk );
static u16 RESORT_CROWD_PEOPLE_WK_GetDir( const RESORT_CROWD_PEOPLE_WK *cp_wk );
static void RESORT_CROWD_PEOPLE_WK_ChangeOBJCodeReq( RESORT_CROWD_PEOPLE_WK *p_wk );
static BOOL RESORT_CROWD_PEOPLE_WK_IsTutorial( const RESORT_CROWD_PEOPLE_WK *cp_wk );

static BOOL Resort_Crowd_People_Wk_IsFrontPlayer( const RESORT_CROWD_PEOPLE_WK *cp_wk, const MMDL* cp_mmdl, s16 grid, s16 min_grid );
static BOOL Resort_Crowd_People_Wk_IsFrontAllMMdl( const RESORT_CROWD_PEOPLE_WK* cp_wk, const MMDLSYS* cp_fos, const FIELD_PLAYER* cp_player, s16 grid, s16 min_grid );
static BOOL Resort_Crowd_People_Wk_IsGridHit( s16 mygx, s16 mygz, s16 pl_gx, s16 pl_gz, s16 grid, u16 min_grid, s16 move_dir );

static void Resort_Crowd_People_Wk_InitKyoroKyoro( RESORT_CROWD_PEOPLE_WK *p_wk );
static void Resort_Crowd_People_Wk_UpdateKyoroKyoro( RESORT_CROWD_PEOPLE_WK *p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player );

static void Resort_Crowd_People_Wk_InitInterest( RESORT_CROWD_PEOPLE_WK *p_wk, const RESORT_BINARY_SHOP *cp_shop, const RESORT_BINARY_INSTITUTION *cp_institution, RESORT_PEOPLE_ALL_DATA *p_all_people );
static BOOL Resort_Crowd_People_Wk_CheckInterest( RESORT_CROWD_PEOPLE_WK *p_wk, const FIELD_PLAYER* cp_player );

static void Resort_Crowd_People_Mmdl_Init( MMDL * mmdl );
static void Resort_Crowd_People_Mmdl_Move( MMDL * mmdl );
static void Resort_Crowd_People_Mmdl_Delete( MMDL * mmdl );
static void Resort_Crowd_People_Mmdl_Recover( MMDL * mmdl );

static const MMDL_MOVE_PROC_LIST sc_Resort_Crowd_People_Proc  =
{
  MV_DMY,
  Resort_Crowd_People_Mmdl_Init,
  Resort_Crowd_People_Mmdl_Move,
  Resort_Crowd_People_Mmdl_Delete,
  Resort_Crowd_People_Mmdl_Recover,
};

//-------------------------------------
///	隊列ワーク
//=====================================
typedef struct
{
  RESORT_CROWD_PEOPLE_WK *p_move[RESORT_CROWD_MOVE_LINE_PEOPLE_MAX];
  u16 want_shop_type;
  u16 start_index;
  s16 wait;
  u16 register_cnt;
  u16 move_dir;
  u16 x;
} RESORT_CROWD_LINE;

#define RESORT_CROWD_LINE_WAIT_MIN  (1)
#define RESORT_CROWD_LINE_WAIT_MAX  (240)
#define RESORT_CROWD_LINE_WAIT_DIF  (RESORT_CROWD_LINE_WAIT_MAX-RESORT_CROWD_LINE_WAIT_MIN)
#define RESORT_CROWD_LINE_WAIT_RND  (RESORT_CROWD_LINE_WAIT_MIN+GFL_STD_MtRand( RESORT_CROWD_LINE_WAIT_DIF ))

static void RESORT_CROWD_LINE_Init( RESORT_CROWD_LINE *p_wk, u16 x, u16 dir, HEAPID heapID );
static void RESORT_CROWD_LINE_Exit( RESORT_CROWD_LINE *p_wk );
static void RESORT_CROWD_LINE_Main( RESORT_CROWD_LINE *p_wk, const FIELD_PLAYER* cp_player );
static void RESORT_CROWD_LINE_SetUp( RESORT_CROWD_LINE *p_wk, RESORT_CROWD_MOVE move );
static void RESORT_CROWD_LINE_Register( RESORT_CROWD_LINE *p_wk, u16 index, RESORT_CROWD_PEOPLE_WK *p_people );
static void RESORT_CROWD_LINE_UnRegister( RESORT_CROWD_LINE *p_wk, u16 index );
static BOOL RESORT_CROWD_LINE_GetRegisterData( const RESORT_PEOPLE *cp_people, u16 *p_line, u16 *p_index );
static void RESORT_CROWD_LINE_SetWait( RESORT_CROWD_LINE *p_wk, s16 wait );

//-------------------------------------
///	群集管理
//=====================================
struct _RESORT_CROWD_PEOPLE
{
  RESORT_CROWD_LINE         line[RESORT_CROWD_MOVE_LINE_MAX];
  RESORT_CROWD_PEOPLE_WK    wk[RESORT_CROWD_PEOPLE_BUFFER_MAX];
  const FIELD_PLAYER        *cp_player;
  MMDLSYS                   *p_mmdlsys;
  const RESORT_BINARY_SHOP  *cp_shop;
  const RESORT_BINARY_INSTITUTION *cp_institution;
  RESORT_PEOPLE_ALL_DATA    *p_all_people;
};

static RESORT_CROWD_PEOPLE_WK * Resort_Crowd_People_GetWkByPeople( RESORT_CROWD_PEOPLE *p_wk, RESORT_PEOPLE *p_people );


static void RESORT_CROWD_PEOPLE_ChangeOBJCodeReq( RESORT_CROWD_PEOPLE *p_wk, RESORT_PEOPLE *p_people );

//----------------------------------------------------------------------------
/**
 *	@brief  群集管理初期化
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static RESORT_CROWD_PEOPLE * RESORT_CROWD_PEOPLE_Init( HEAPID heapID, MMDLSYS* p_mmdlsys, const FIELD_PLAYER* cp_player, const RESORT_BINARY_SHOP *cp_shop, const RESORT_BINARY_INSTITUTION *cp_institution, RESORT_PEOPLE_ALL_DATA *p_all_people )
{
  RESORT_CROWD_PEOPLE *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_CROWD_PEOPLE) );
  p_wk->cp_player = cp_player;
  p_wk->p_mmdlsys = p_mmdlsys;
  p_wk->cp_shop   = cp_shop;
  p_wk->cp_institution   = cp_institution;
  p_wk->p_all_people  = p_all_people;

  {
    int i;
    for( i = 0; i < RESORT_CROWD_PEOPLE_BUFFER_MAX; i++ )
    {
      RESORT_CROWD_PEOPLE_WK_Init( &p_wk->wk[i], heapID );
    }
  }

  {
    static const u16 sc_line_x[]  =
    {
      12,13,14,16,17,18
    };
    static const u16 sc_dir[] =
    {
      DIR_UP,DIR_DOWN,
      DIR_UP,DIR_DOWN,
      DIR_UP,DIR_DOWN,
    };

    int i;
    for( i = 0; i < RESORT_CROWD_MOVE_LINE_MAX; i++ )
    {
      RESORT_CROWD_LINE_Init( &p_wk->line[i], sc_line_x[i], sc_dir[i], heapID );
    }
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  群集管理破棄
 *
 *	@param	RESORT_CROWD_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_Exit( RESORT_CROWD_PEOPLE *p_wk )
{

  {
    int i;
    for( i = 0; i < RESORT_CROWD_MOVE_LINE_MAX; i++ )
    {
      RESORT_CROWD_LINE_Exit( &p_wk->line[i] );
    }
  }

  {
    int i;
    for( i = 0; i < RESORT_CROWD_PEOPLE_BUFFER_MAX; i++ )
    {
      RESORT_CROWD_PEOPLE_WK_Exit( &p_wk->wk[i] );
    }
  }

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  群集管理メイン処理
 *
 *	@param	RESORT_CROWD_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_Main( RESORT_CROWD_PEOPLE *p_wk, const FIELDMAP_WORK* cp_fieldmap )
{
  if( FIELDMAP_CheckDoEvent( cp_fieldmap ) == FALSE )
  {
    //各ライン動作
    {
      int i;
      for( i = 0; i < RESORT_CROWD_MOVE_LINE_MAX; i++ )
      {
        RESORT_CROWD_LINE_Main( &p_wk->line[i], p_wk->cp_player );
      }
    }
  }

  //各ワーク動作
  {
    int i;
    for( i = 0; i < RESORT_CROWD_PEOPLE_BUFFER_MAX; i++ )
    {
      RESORT_CROWD_PEOPLE_WK_Main( &p_wk->wk[i], p_wk->p_mmdlsys, p_wk->cp_player );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  群集管理  セットアップ（初期人物登録後に使用）
 *
 *	@param	RESORT_CROWD_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_SetUp( RESORT_CROWD_PEOPLE *p_wk, RESORT_CROWD_MOVE move )
{
  //各ライン動作
  {
    int i;
    for( i = 0; i < RESORT_CROWD_MOVE_LINE_MAX; i++ )
    {
      RESORT_CROWD_LINE_SetUp( &p_wk->line[i], move );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  群集管理  人追加
 *
 *	@param	RESORT_CROWD_PEOPLE *p_wkワーク
 *	@param	*p_data   追加する人
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_Add( RESORT_CROWD_PEOPLE *p_wk, RESORT_PEOPLE *p_people )
{
  u16 line;
  u16 index;

  if( RESORT_CROWD_LINE_GetRegisterData( p_people, &line, &index ) )
  {
    int i;
    for( i = 0; i < RESORT_CROWD_PEOPLE_BUFFER_MAX; i++ )
    {
      if( !RESORT_CROWD_PEOPLE_WK_IsExist(&p_wk->wk[i]) )
      {
        //ワークへの設定
        RESORT_CROWD_PEOPLE_WK_SetUp( &p_wk->wk[i], p_people, p_wk->cp_shop, p_wk->cp_institution, p_wk->p_all_people );

        //隊列への登録
        RESORT_CROWD_LINE_Register( &p_wk->line[line], index, &p_wk->wk[i] );
        break;
      }
    }
  }
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  群集から外す
 *
 *	@param	RESORT_CROWD_PEOPLE *p_wk ワーク
 *	@param	*p_people                 人物
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_Delete( RESORT_CROWD_PEOPLE *p_wk, RESORT_PEOPLE *p_people )
{
  u16 line;
  u16 index;
  if( RESORT_CROWD_LINE_GetRegisterData( p_people, &line, &index ) )
  {
    RESORT_CROWD_PEOPLE_WK *p_crowd = Resort_Crowd_People_GetWkByPeople( p_wk, p_people );

    //登録されていた
    if( p_crowd )
    {
      //まずは隊列から外す
      RESORT_CROWD_LINE_UnRegister( &p_wk->line[line], index );

      //群集人物から外す
      RESORT_CROWD_PEOPLE_WK_Exit( p_crowd );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  群集の人物にOBJCode変更リクエスト
 *
 *	@param	RESORT_CROWD_PEOPLE *p_wk ワーク
 *	@param	*p_people 人物
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_ChangeOBJCodeReq( RESORT_CROWD_PEOPLE *p_wk, RESORT_PEOPLE *p_people )
{
  RESORT_CROWD_PEOPLE_WK *p_crowd = Resort_Crowd_People_GetWkByPeople( p_wk, p_people );

  //登録されていた
  if( p_crowd )
  {
    RESORT_CROWD_PEOPLE_WK_ChangeOBJCodeReq( p_crowd );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  群集人物ワーク取得
 *
 *	@param	RESORT_CROWD_PEOPLE *p_wk ワーク
 *	@param	*p_people   人物
 *
 *	@return 群集人物ワーク
 */
//-----------------------------------------------------------------------------
static RESORT_CROWD_PEOPLE_WK * Resort_Crowd_People_GetWkByPeople( RESORT_CROWD_PEOPLE *p_wk, RESORT_PEOPLE *p_people )
{
  u16 line;
  u16 index;
  if( RESORT_CROWD_LINE_GetRegisterData( p_people, &line, &index ) )
  {
    int i;
    //登録されているかチェックする
    for( i = 0; i < RESORT_CROWD_PEOPLE_BUFFER_MAX; i++ )
    {
      if( RESORT_CROWD_PEOPLE_WK_IsCompPeople(&p_wk->wk[i], p_people) )
      {
        return &p_wk->wk[i];
      }
    }
  }
  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ワークを初期化
 *
 *	@param	RESORT_CROWD_PEOPLE_WK *p_wk  ワーク
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_WK_Init( RESORT_CROWD_PEOPLE_WK *p_wk, HEAPID heapID )
{
  RESORT_CROWD_PEOPLE_WK_Clear( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワークを破棄
 *
 *	@param	RESORT_CROWD_PEOPLE_WK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_WK_Exit( RESORT_CROWD_PEOPLE_WK *p_wk )
{
  if( RESORT_CROWD_PEOPLE_WK_IsExist(p_wk) )
  {
    MMDL_ChangeMoveCode( RESORT_PEOPLE_GetMmdl(p_wk->p_people), MV_DMY );
  }
  RESORT_CROWD_PEOPLE_WK_Clear( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物動作メイン処理
 *
 *	@param	RESORT_CROWD_PEOPLE_WK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_WK_Main( RESORT_CROWD_PEOPLE_WK *p_wk, MMDLSYS* p_mmdlsys, const FIELD_PLAYER* cp_player )
{
  if( RESORT_CROWD_PEOPLE_WK_IsExist(p_wk) )
  {
    enum
    {
      SEQ_MARK_START,
      SEQ_MARK_END,
    };
    MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk->p_people );

    //音符処理
    if( p_wk->is_acmd_req && !p_wk->is_acmd_end )
    {
      switch( p_wk->acmd_seq )
      {
      case SEQ_MARK_START:
        //SetAcmdでやらないのは、話しかけた時に止まって欲しくないため
        MMDL_SetLocalAcmd( p_mmdl, AC_MARK_ONPU_NO_SE );
        p_wk->acmd_seq++;
        /* fall */
      case SEQ_MARK_END:
        if( MMDL_ActionLocalAcmd( p_mmdl ) )
        {
          p_wk->acmd_seq  = 0;
          p_wk->is_acmd_req = FALSE;
          p_wk->is_acmd_end = TRUE;
        }
        break;
      }
    }

    //見た目変更処理
  
    if( p_wk->changeObjCode )
    {
      if( RESORT_CROWD_PEOPLE_WK_IsEndMove(p_wk) )
      {
        RESORT_PEOPLE_ChangeOBJCode( p_wk->p_people );
        p_wk->changeObjCode = FALSE;
      }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物を設定
 *
 *	@param	RESORT_CROWD_PEOPLE_WK *p_wk  ワーク
 *	@param	*p_people 登録する人物
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_WK_SetUp( RESORT_CROWD_PEOPLE_WK *p_wk, RESORT_PEOPLE *p_people, const RESORT_BINARY_SHOP *cp_shop, const RESORT_BINARY_INSTITUTION *cp_institution, RESORT_PEOPLE_ALL_DATA *p_all_people )
{
  GF_ASSERT( p_wk->p_people == NULL );
  RESORT_CROWD_PEOPLE_WK_Clear( p_wk );
  p_wk->p_people = p_people;

  MMDL_SetMoveProcList( RESORT_PEOPLE_GetMmdl(p_people), &sc_Resort_Crowd_People_Proc );

  RESORT_PEOPLE_SetVisible( p_wk->p_people, FALSE );

  Resort_Crowd_People_Wk_InitInterest( p_wk, cp_shop, cp_institution, p_all_people );

  switch( RESORT_PEOPLE_GetPeopleData( p_wk->p_people, RESORT_PEOPLE_DATA_PARAM_TYPE ) )
  {
  case RESORT_PEOPLE_TYPE_PROXY:
  case RESORT_PEOPLE_TYPE_TOURIST:
    p_wk->InitMove    = Resort_Crowd_People_Wk_InitKyoroKyoro;
    p_wk->UpdateMove  = Resort_Crowd_People_Wk_UpdateKyoroKyoro;
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ワーククリア
 *
 *	@param	RESORT_CROWD_PEOPLE_WK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_WK_Clear( RESORT_CROWD_PEOPLE_WK *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_CROWD_PEOPLE_WK) );
  p_wk->magic_number = RESORT_CROWD_PEOPLE_WK_MAGIC_NUMBER;
}
//----------------------------------------------------------------------------
/**
 *	@brief  同じ人物かチェック
 *
 *	@param	const RESORT_CROWD_PEOPLE_WK *cp_wkワーク
 *	@param	RESORT_PEOPLE *cp_people  人物
 *
 *	@return TRUE  人物が同じ  FALSE人物が別
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_CROWD_PEOPLE_WK_IsCompPeople( const RESORT_CROWD_PEOPLE_WK *cp_wk, const RESORT_PEOPLE *cp_people )
{
  return cp_people == cp_wk->p_people;
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物が登録されているかチェック
 *
 *	@param	const RESORT_CROWD_PEOPLE_WK *cp_wk   ワーク
 *
 *	@return TRUEで存在  FALSEで存在していない
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_CROWD_PEOPLE_WK_IsExist( const RESORT_CROWD_PEOPLE_WK *cp_wk )
{
  return cp_wk->p_people != NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  動作開始
 *
 *	@param	RESORT_CROWD_PEOPLE_WK *p_wk  ワーク
 *	@param	x どの軸を歩くか
 *	@param	front_or_back TRUEならば前面から開始  FLASEならば背面から開始
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_WK_StartMove( RESORT_CROWD_PEOPLE_WK *p_wk, u16 acmd, u16 move_dir, u16 x )
{
  RESORT_CROWD_PEOPLE_WK_StartMovePos( p_wk, acmd, move_dir, x, 0 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  動作開始位置指定版
 *
 *	@param	RESORT_CROWD_PEOPLE_WK *p_wk  ワーク
 *	@param	acmd  アクションコマンド
 *	@param	move_dir  向き
 *	@param	x         X座標
 *	@param	z         Y座標
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_WK_StartMovePos( RESORT_CROWD_PEOPLE_WK *p_wk, u16 acmd, u16 move_dir, u16 x, u16 z )
{
  p_wk->move_acmd = acmd;
  p_wk->seq       = 0;
  p_wk->is_walk   = TRUE;
  p_wk->move_dir  = move_dir;

  {
    MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk->p_people );

    VecFx32 vec;

    MMDL_GetVectorPos( p_mmdl, &vec );

    if( RESORT_CROWD_PEOPLE_WK_IsNewData(p_wk) )
    {
      vec.x = GRID_TO_FX32(x) + MMDL_VEC_X_GRID_OFFS_FX32;
      MMDL_SetGridPosX( p_mmdl, x );
      if( p_wk->move_dir == DIR_UP )
      {
        if( z == 0 )
        {
          z = RESORT_CROWD_MOVE_LIMIT_FRONT;
        }

        MMDL_SetGridPosZ( p_mmdl, z );
        vec.z = GRID_TO_FX32(z) + MMDL_VEC_Z_GRID_OFFS_FX32;
        p_wk->move_grid = z - RESORT_CROWD_MOVE_LIMIT_BACK+1;
        if( p_wk->move_grid <= 0 )
        {
          p_wk->move_grid = 1;
        }
      }
      else
      {
        if( z == 0 )
        {
          z = RESORT_CROWD_MOVE_LIMIT_BACK;
        }

        MMDL_SetGridPosZ( p_mmdl, z );
        vec.z = GRID_TO_FX32(z) + MMDL_VEC_Z_GRID_OFFS_FX32;
        p_wk->move_grid = RESORT_CROWD_MOVE_LIMIT_FRONT-z+1;
        if( p_wk->move_grid <= 0 )
        {
          p_wk->move_grid = 1;
        }
      }
      MMDL_SetVectorPos( p_mmdl, &vec );
    }
    else
    {
      s16 now_z = MMDL_GetGridPosZ( p_mmdl );
      if( p_wk->move_dir == DIR_UP )
      {
        if( now_z < RESORT_CROWD_MOVE_LIMIT_BACK )
        {
          MMDL_SetGridPosZ( p_mmdl, RESORT_CROWD_MOVE_LIMIT_FRONT );
          vec.z = GRID_TO_FX32(RESORT_CROWD_MOVE_LIMIT_FRONT) + MMDL_VEC_Z_GRID_OFFS_FX32;
          p_wk->move_grid = RESORT_CROWD_MOVE_LIMIT_FRONT-RESORT_CROWD_MOVE_LIMIT_BACK+1;
          if( p_wk->move_grid <= 0 )
          {
            p_wk->move_grid = 1;
          }
          MMDL_SetVectorPos( p_mmdl, &vec );
        }
        else
        {
          p_wk->move_grid = now_z - RESORT_CROWD_MOVE_LIMIT_BACK+1;
          if( p_wk->move_grid <= 0 )
          {
            p_wk->move_grid = 1;
          }
        }
      }
      else if( p_wk->move_dir == DIR_DOWN )
      {
        if( now_z > RESORT_CROWD_MOVE_LIMIT_FRONT )
        {
          MMDL_SetGridPosZ( p_mmdl, RESORT_CROWD_MOVE_LIMIT_BACK );
          vec.z = GRID_TO_FX32(RESORT_CROWD_MOVE_LIMIT_BACK) + MMDL_VEC_Z_GRID_OFFS_FX32;
          p_wk->move_grid = RESORT_CROWD_MOVE_LIMIT_FRONT-RESORT_CROWD_MOVE_LIMIT_BACK+1;
          if( p_wk->move_grid <= 0 )
          {
            p_wk->move_grid = 1;
          }
          MMDL_SetVectorPos( p_mmdl, &vec );
        }
        else
        {
          p_wk->move_grid = RESORT_CROWD_MOVE_LIMIT_FRONT - now_z+1;
          if( p_wk->move_grid <= 0 )
          {
            p_wk->move_grid = 1;
          }
        }
      }
    }
  }

  RESORT_PEOPLE_SetVisible( p_wk->p_people, TRUE );
  RESORT_PEOPLE_SetData( p_wk->p_people, RESORT_DATA_PARAM_REPLACE_LOCK, TRUE );

  if( p_wk->InitMove )
  {
    p_wk->InitMove( p_wk );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  動作終了チェック
 *
 *	@param	const RESORT_CROWD_PEOPLE_WK *cp_wk ワーク
 *
 *	@return TRUE歩いていない  FALSE歩いている
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_CROWD_PEOPLE_WK_IsEndMove( const RESORT_CROWD_PEOPLE_WK *cp_wk )
{
  return cp_wk->is_walk == FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  新規データかどうかチェック
 *
 *	@param	const RESORT_CROWD_PEOPLE_WK *cp_wk ワーク 
 *
 *	@return TRUE落ちてきたデータ新規データ  FALSE読み込みデータ
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_CROWD_PEOPLE_WK_IsNewData( const RESORT_CROWD_PEOPLE_WK *cp_wk )
{
  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( cp_wk->p_people );

  s16 now_x = MMDL_GetGridPosX( p_mmdl );
  s16 now_z = MMDL_GetGridPosZ( p_mmdl );
  //もし座標０，０だったら新規データ
  if( now_x == 0 && now_z == 0 )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  方向取得
 *
 *	@param	const RESORT_CROWD_PEOPLE_WK *cp_wk ワーク
 *
 *	@return 方向
 */
//-----------------------------------------------------------------------------
static u16 RESORT_CROWD_PEOPLE_WK_GetDir( const RESORT_CROWD_PEOPLE_WK *cp_wk )
{
  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( cp_wk->p_people );
  return MMDL_GetDirDisp(p_mmdl);
}
//----------------------------------------------------------------------------
/**
 *	@brief  見た目変更リクエスト
 *
 *	@param	RESORT_CROWD_PEOPLE_WK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_PEOPLE_WK_ChangeOBJCodeReq( RESORT_CROWD_PEOPLE_WK *p_wk )
{
  p_wk->changeObjCode = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  チュートリアル用かチェック
 *
 *	@param	const RESORT_CROWD_PEOPLE_WK *cp_wk ワーク
 *
 *	@return TRUEチュートリアル用  FLASEその他
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_CROWD_PEOPLE_WK_IsTutorial( const RESORT_CROWD_PEOPLE_WK *cp_wk )
{
  u32 people_type = RESORT_PEOPLE_GetPeopleData( cp_wk->p_people, RESORT_PEOPLE_DATA_PARAM_TYPE );
  u32 type  = RESORT_PEOPLE_GetData( cp_wk->p_people, RESORT_DATA_PARAM_TYPE, NULL);

  if( people_type == RESORT_PEOPLE_TYPE_PROXY )
  {
    if( type == RESORT_PROXY_TYPE_TUTORIAL_01
        || type == RESORT_PROXY_TYPE_TUTORIAL_02 )
    {
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  grid歩前までに自機がいないかチェック
 *
 *	@param	cp_wk         ワーク
 *	@param	cp_player     プレイヤー
 *	@param  grid          グリッド
 *
 *	@retval TRUE  いる
 *	@retval FALSE いない
 */
//-----------------------------------------------------------------------------
static BOOL Resort_Crowd_People_Wk_IsFrontPlayer( const RESORT_CROWD_PEOPLE_WK *cp_wk, const MMDL* cp_mmdl, s16 grid, s16 min_grid )
{
  s16 player_gx, player_gz;
  s16 my_gx, my_gz;

  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( cp_wk->p_people );
  const MMDL* cp_playermmdl = cp_mmdl;
  
  player_gx = MMDL_GetGridPosX( cp_playermmdl );
  player_gz = MMDL_GetGridPosZ( cp_playermmdl );
  my_gx = MMDL_GetGridPosX( p_mmdl );
  my_gz = MMDL_GetGridPosZ( p_mmdl );
  
  if( Resort_Crowd_People_Wk_IsGridHit( my_gx, my_gz, player_gx, player_gz, grid, min_grid, cp_wk->move_dir ) )
  {
    return TRUE;
  }

  player_gx = MMDL_GetOldGridPosX( cp_playermmdl );
  player_gz = MMDL_GetOldGridPosZ( cp_playermmdl );
  
  return Resort_Crowd_People_Wk_IsGridHit( my_gx, my_gz, player_gx, player_gz, grid, min_grid, cp_wk->move_dir );
}
//----------------------------------------------------------------------------
/**
 *	@brief  あたらないか、全モデルでチェック
 *
 *	@param	cp_wk
 *	@param	cp_fos
 *	@param  pp_mmdl 当たったモデル
 *	@param	grid 
 */
//-----------------------------------------------------------------------------
static BOOL Resort_Crowd_People_Wk_IsFrontAllMMdl( const RESORT_CROWD_PEOPLE_WK* cp_wk, const MMDLSYS* cp_fos, const FIELD_PLAYER* cp_player, s16 grid, s16 min_grid )
{
	u32 no = 0;
	MMDL *mmdl;

  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( cp_wk->p_people );

#if 1
  {
    u32 ret = MMDL_HitCheckMoveDir( p_mmdl, cp_wk->move_dir );
    if( ret & MMDL_MOVEHITBIT_OBJ )
    {
      return TRUE;
    }
    return FALSE;
  }
#else
	while( MMDLSYS_SearchUseMMdl(cp_fos,&mmdl,&no) == TRUE ){
    if( p_mmdl != mmdl ){
      if( MMDL_CheckStatusBit(mmdl,MMDL_STABIT_FELLOW_HIT_NON) == 0 )
      {
        // 表示されている？ 
        if( MMDL_CheckStatusBitVanish( mmdl ) == FALSE ){
          if( Resort_Crowd_People_Wk_IsFrontPlayer( cp_wk, mmdl, grid, min_grid ) ){
            return TRUE;
          }
        }
      }
    }
  }

  //プレイヤーとの当たり判定チェック
  //プレイヤーが動作しているときはプレイヤーとMMDLの間１マスグリッドまで歩かない
  {
    if( FIELD_PLAYER_GetMoveValue(cp_player) != PLAYER_MOVE_VALUE_STOP )
    {
      {
        MMDL *p_plyaer_mmdl = FIELD_PLAYER_GetMMdl(cp_player);
        s16 px, py, pz;
        s16 mydir = MMDL_GetDirDisp( p_mmdl );
        s16 my_gx = MMDL_GetGridPosX( p_mmdl );
        s16 my_gz = MMDL_GetGridPosZ( p_mmdl );
        FIELD_PLAYER_GetGridPos( cp_player, &px, &py, &pz );

        if( Resort_Crowd_People_Wk_IsGridHit( my_gx, my_gz, px, pz, grid, grid, cp_wk->move_dir ) )
        {
          return TRUE;
        }
      }
    }
  }
#endif

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  grid歩前にいるのかのグリッド単位のチェック
 *
 *	@param	mygx
 *	@param	mygz
 *	@param	pl_gx
 *	@param	pl_gz
 *	@param	grid 
 *
 *	@retval TRUE  いる
 *	@retval FALSE いない
 */
//-----------------------------------------------------------------------------
static BOOL Resort_Crowd_People_Wk_IsGridHit( s16 mygx, s16 mygz, s16 pl_gx, s16 pl_gz, s16 grid, u16 min_grid, s16 move_dir )
{
  s32 grid_dif;
  s32 grid_dif_2;

  switch( move_dir )
  {
  case DIR_UP:
    grid_dif = mygz - pl_gz;
    grid_dif_2 = mygx - pl_gx;
    break;
  case DIR_DOWN:
    grid_dif = pl_gz - mygz;
    grid_dif_2 = mygx - pl_gx;
    break;
  case DIR_LEFT:
    grid_dif = mygx - pl_gx;
    grid_dif_2 = mygz - pl_gz;
    break;
  case DIR_RIGHT:
    grid_dif = pl_gx - mygx;
    grid_dif_2 = mygz - pl_gz;
    break;

  default:
    GF_ASSERT(0);
    break;
  }

  if( ((grid_dif <= grid) && (grid_dif >= min_grid)) && (grid_dif_2 == 0) )
  {
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物動作処理初期化  キョロキョロ
 *
 *	@param	RESORT_CROWD_PEOPLE_WK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Resort_Crowd_People_Wk_InitKyoroKyoro( RESORT_CROWD_PEOPLE_WK *p_wk )
{
  u16 bit_num = RESORT_PEOPLE_GetData( p_wk->p_people, RESORT_DATA_PARAM_EXTEND_NUM, NULL );

  //チュートリアル用の人物はキョロキョロしまくる
  //→追いつきやすくする対応
  if( RESORT_PEOPLE_GetPeopleData( p_wk->p_people, RESORT_PEOPLE_DATA_PARAM_TYPE ) == RESORT_PEOPLE_TYPE_PROXY )
  {
    u32 type = RESORT_PEOPLE_GetData(p_wk->p_people,RESORT_DATA_PARAM_TYPE,NULL);
    if( type == RESORT_PROXY_TYPE_TUTORIAL_01
        || type == RESORT_PROXY_TYPE_TUTORIAL_02 )
    {
      bit_num = RESORT_EXTEND_TYPE_MAX;
    }
  }

  GF_ASSERT(p_wk->move_grid > 0 );

  //距離に応じた振り向き回数を出す
  bit_num = p_wk->move_grid * bit_num / (RESORT_CROWD_MOVE_LIMIT_FRONT-RESORT_CROWD_MOVE_LIMIT_BACK+1);

  p_wk->kyoro.cnt   = 0;
  p_wk->kyoro.seq   = 0;
  p_wk->kyoro.sync  = 15;
  p_wk->kyoro.max   = bit_num;
  p_wk->kyoro.grid  = (p_wk->move_grid / p_wk->kyoro.max) + 5/2 - GFL_STD_MtRand(5);
  if( p_wk->kyoro.grid <= 0 )
  {
    p_wk->kyoro.grid = 1;
  }
  p_wk->kyoro.start_grid  = p_wk->move_grid - p_wk->kyoro.grid;
}

enum
{
  CROWD_PEOPLE_WALK_SEQ_CHEK_END,
  CROWD_PEOPLE_MOVE_SEQ_CHECK,
  CROWD_PEOPLE_WALK_SEQ_WAIT,

  CROWD_PEOPLE_KYORO_SEQ_WAIT,

  CROWD_PEOPLE_INTEREST_SEQ_WAIT,
};

//----------------------------------------------------------------------------
/**
 *	@brief  終了チェック
 *
 *	@param	*p_wk ワーク
 *	@param	p_mmdlsys 動作モデルシステム
 *	@param	FIELD_PLAYER* cp_player フィールド
 *
 *	@return TRUE続行  FALSE続行していない
 */
//-----------------------------------------------------------------------------
static BOOL Resort_Crowd_People_Walk_CheckEnd(RESORT_CROWD_PEOPLE_WK *p_wk, MMDLSYS* p_mmdlsys, const FIELD_PLAYER* cp_player)
{
  //歩き終了
  if( p_wk->move_grid <= 0 )
  {
    p_wk->is_walk = FALSE;
    p_wk->seq = 0;
    RESORT_PEOPLE_SetVisible( p_wk->p_people, FALSE );
    RESORT_PEOPLE_SetData( p_wk->p_people, RESORT_DATA_PARAM_REPLACE_LOCK, FALSE );
    return FALSE;
  }
  else
  {
    p_wk->seq = CROWD_PEOPLE_MOVE_SEQ_CHECK;
    return TRUE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  動作開始チェック
 *
 *	@param	*p_wk ワーク
 *	@param	p_mmdlsys 動作モデルシステム
 *	@param	FIELD_PLAYER* cp_player フィールド
 *
 *	@return TRUE続行  FALSE続行していない
 */
//-----------------------------------------------------------------------------
static BOOL Resort_Crowd_People_Move_Check(RESORT_CROWD_PEOPLE_WK *p_wk, MMDLSYS* p_mmdlsys, const FIELD_PLAYER* cp_player)
{
  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk->p_people );
  if( Resort_Crowd_People_Wk_IsFrontAllMMdl( p_wk, p_mmdlsys, cp_player, 1, 0 ) )
  {
    //当たり判定にあたった
    //u16 ac = MMDL_ChangeDirAcmdCode( p_wk->move_dir, AC_WAIT_8F );
    MMDL_SetLocalAcmd( p_mmdl, AC_WAIT_8F );
    MMDL_OnMoveBitMove( p_mmdl );
    p_wk->seq = CROWD_PEOPLE_WALK_SEQ_WAIT;
  }
  else if( Resort_Crowd_People_Wk_CheckInterest(p_wk, cp_player) )
  {
    //興味がある場所
    p_wk->seq = CROWD_PEOPLE_INTEREST_SEQ_WAIT;
  }
  else if( p_wk->kyoro.start_grid == p_wk->move_grid )
  {
    //キョロキョロポイント
    p_wk->kyoro.seq = 0;
    p_wk->seq = CROWD_PEOPLE_KYORO_SEQ_WAIT;
  }
  else
  {
    //通常歩行
    u16 ac = MMDL_ChangeDirAcmdCode( p_wk->move_dir, p_wk->move_acmd );
    MMDL_SetLocalAcmd( p_mmdl, ac );
    MMDL_SetDirMove( p_mmdl, p_wk->move_dir );
    MMDL_OnMoveBitMove( p_mmdl );
    p_wk->move_grid--;
    p_wk->seq = CROWD_PEOPLE_WALK_SEQ_WAIT;
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  歩行待機
 *
 *	@param	*p_wk ワーク
 *	@param	p_mmdlsys 動作モデルシステム
 *	@param	FIELD_PLAYER* cp_player フィールド
 *
 *	@return TRUE続行  FALSE続行していない
 */
//-----------------------------------------------------------------------------
static BOOL Resort_Crowd_People_Walk_Wait(RESORT_CROWD_PEOPLE_WK *p_wk, MMDLSYS* p_mmdlsys, const FIELD_PLAYER* cp_player)
{
  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk->p_people );
  if( MMDL_ActionLocalAcmd( p_mmdl ) )
  {
    MMDL_OffMoveBitMove( p_mmdl );
    p_wk->seq = CROWD_PEOPLE_WALK_SEQ_CHEK_END;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  きょろきょろ待機
 *
 *	@param	*p_wk ワーク
 *	@param	p_mmdlsys 動作モデルシステム
 *	@param	FIELD_PLAYER* cp_player フィールド
 *
 *	@return TRUE続行  FALSE続行していない
 */
//-----------------------------------------------------------------------------
static BOOL Resort_Crowd_People_Kyoro_Wait(RESORT_CROWD_PEOPLE_WK *p_wk, MMDLSYS* p_mmdlsys, const FIELD_PLAYER* cp_player)
{
  enum
  {
    KYORO_SEQ_START,
    KYORO_SEQ_WAIT,
    KYORO_SEQ_END,
    KYORO_SEQ_START2,
    KYORO_SEQ_WAIT2,
    KYORO_SEQ_END2,
  };
  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk->p_people );

  switch( p_wk->kyoro.seq )
  {
  case KYORO_SEQ_START:
    p_wk->kyoro.dir = GFL_STD_MtRand(2) ? AC_DIR_L: AC_DIR_R;
    MMDL_SetLocalAcmd( p_mmdl, p_wk->kyoro.dir);
    p_wk->kyoro.seq++;
    /*  fall  */
  case KYORO_SEQ_WAIT:
    if( MMDL_ActionLocalAcmd( p_mmdl ) )
    {
      p_wk->kyoro.seq++;
      /* fall */
    }
    else
    {
      return FALSE;
    }
  case KYORO_SEQ_END:
    if( p_wk->kyoro.cnt++ >= p_wk->kyoro.sync )
    {
      p_wk->kyoro.cnt = 0;
      p_wk->kyoro.seq++;
      /*  fall  */
    }
    else
    {
      return FALSE;
    }
  case KYORO_SEQ_START2:
    p_wk->kyoro.dir = p_wk->kyoro.dir == AC_DIR_R ? AC_DIR_L: AC_DIR_R;
    MMDL_SetLocalAcmd( p_mmdl, p_wk->kyoro.dir);
    p_wk->kyoro.seq++;
    /* fall */
  case KYORO_SEQ_WAIT2:
    if( MMDL_ActionLocalAcmd( p_mmdl ) )
    {
      p_wk->kyoro.seq++;
      /* fall */
    }
    else
    {
      return FALSE;
    }
  case KYORO_SEQ_END2:
    if( p_wk->kyoro.cnt++ >= p_wk->kyoro.sync )
    {
      p_wk->kyoro.cnt = 0;
      p_wk->kyoro.seq = 0;
      p_wk->kyoro.start_grid  = p_wk->move_grid - p_wk->kyoro.grid;

      p_wk->seq = CROWD_PEOPLE_WALK_SEQ_CHEK_END;
    }
    else
    {
      return FALSE;
    }

  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  興味待機
 *
 *	@param	*p_wk ワーク
 *	@param	p_mmdlsys 動作モデルシステム
 *	@param	FIELD_PLAYER* cp_player フィールド
 *
 *	@return TRUE続行  FALSE続行していない
 */
//-----------------------------------------------------------------------------
static BOOL Resort_Crowd_People_Interest_Wait(RESORT_CROWD_PEOPLE_WK *p_wk, MMDLSYS* p_mmdlsys, const FIELD_PLAYER* cp_player)
{
  enum
  {
    SEQ_DIR_START,
    SEQ_DIR_END,
    SEQ_MARK_START,
    SEQ_MARK_END,
  };

  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk->p_people );

  switch( p_wk->interest_seq )
  {
  case SEQ_DIR_START:
    MMDL_OffMoveBitMove( p_mmdl );
    MMDL_OffMoveBitMoveEnd( p_mmdl );

    MMDL_SetLocalAcmd( p_mmdl, p_wk->interest_dir[p_wk->interest_index] );
    p_wk->interest_seq++;
    /* fall */
  case SEQ_DIR_END:
    if( MMDL_ActionLocalAcmd( p_mmdl ) )
    {
      p_wk->interest_seq++;
      /* fall */
    }
    else
    {
      return FALSE;
    }
  case SEQ_MARK_START:  
    //動作関数の外で呼び出す
    p_wk->acmd_seq  = 0;
    p_wk->is_acmd_req = TRUE;
    p_wk->is_acmd_end = FALSE;
    p_wk->interest_seq++;
    /* fall */
  case SEQ_MARK_END:
    if( p_wk->is_acmd_end )
    {
      p_wk->seq = CROWD_PEOPLE_WALK_SEQ_WAIT;
    }
    else
    {
      return FALSE;
    }
  }
  return TRUE;
}


static BOOL (* const Resort_Crowd_People_Walk[])(RESORT_CROWD_PEOPLE_WK *, MMDLSYS* , const FIELD_PLAYER* )  =
{
  Resort_Crowd_People_Walk_CheckEnd,
  Resort_Crowd_People_Move_Check,
  Resort_Crowd_People_Walk_Wait,

  Resort_Crowd_People_Kyoro_Wait,

  Resort_Crowd_People_Interest_Wait,
};

//----------------------------------------------------------------------------
/**
 *	@brief  人物動作メイン処理  キョロキョロ
 *
 *	@param	RESORT_CROWD_PEOPLE_WK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Resort_Crowd_People_Wk_UpdateKyoroKyoro( RESORT_CROWD_PEOPLE_WK *p_wk, MMDLSYS* p_mmdlsys, const FIELD_PLAYER* cp_player )
{
  while( Resort_Crowd_People_Walk[p_wk->seq](p_wk,p_mmdlsys,cp_player) == TRUE ){}
}
//----------------------------------------------------------------------------
/**
 *	@brief  興味ある場所を初期化
 *
 *	@param	RESORT_CROWD_PEOPLE_WK *p_wk  ワーク
 *	@param	RESORT_BINARY_SHOP *cp_shop   お店バイナリ
 *	@param	RESORT_BINARY_INSTITUTION *cp_institution 施設バイナリ
 *	@param	*p_all_people   人物情報
 *
 */
//-----------------------------------------------------------------------------
static void Resort_Crowd_People_Wk_InitInterest( RESORT_CROWD_PEOPLE_WK *p_wk, const RESORT_BINARY_SHOP *cp_shop, const RESORT_BINARY_INSTITUTION *cp_institution, RESORT_PEOPLE_ALL_DATA *p_all_people )
{
  //自分のなりたいお店と、現在あるお店から
  //興味があるグリッドを取得
  {
    u16 want_shop_type;
    {
      u16 want_job  = RESORT_PEOPLE_GetData( p_wk->p_people, RESORT_DATA_PARAM_WANT_JOB, NULL );
      const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_GetData( cp_shop, want_job );
      GF_ASSERT( want_job < RESORT_BINARY_SHOP_DATA_GetMax(cp_shop) );
      want_shop_type  = RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP );
    }

    {
      int i;

      MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk->p_people );
      s16 now_z = MMDL_GetGridPosZ( p_mmdl );

      for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
      {
        RESORT_RESIDENT_DATA *p_resi = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people, i );
        if( !RESORT_RESIDENT_DATA_IsEmpty( p_resi ) )
        {

          const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_SearchDataByData( cp_shop, p_resi );
          if( want_shop_type == RESORT_BINARY_SHOP_DATA_GetData( cp_shop_data, RESORT_BINARY_SHOP_PARAM_SHOP )
)
          {
            u16 z = RESORT_BINARY_INSTITUTION_GetData( cp_institution, i, RESORT_BINARY_INSTITUTION_PARAM_GRID_Z );
            p_wk->interest_grid_z[p_wk->interest_cnt] = z + GFL_STD_MtRand(7);
            p_wk->interest_dir[p_wk->interest_cnt] = (i % 2 == 0 ? AC_DIR_R: AC_DIR_L);
            p_wk->interest_cnt++;
          }
        }
      }

      p_wk->interest_index  = p_wk->interest_cnt;
    }
  }


}
//----------------------------------------------------------------------------
/**
 *	@brief  興味の場所からチェック
 *
 *	@param	RESORT_CROWD_PEOPLE_WK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static BOOL Resort_Crowd_People_Wk_UpdateInterest( RESORT_CROWD_PEOPLE_WK *p_wk )
{
  enum
  {
    SEQ_DIR_START,
    SEQ_DIR_END,
    SEQ_MARK_START,
    SEQ_MARK_END,
  };

  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk->p_people );

  switch( p_wk->interest_seq )
  {
  case SEQ_DIR_START:
    MMDL_SetLocalAcmd( p_mmdl, p_wk->interest_dir[p_wk->interest_index] );
    p_wk->interest_seq++;
    /* fall */
  case SEQ_DIR_END:
    if( MMDL_ActionLocalAcmd( p_mmdl ) )
    {
      p_wk->interest_seq++;
      /* fall */
    }
    else
    {
      break;
    }
  case SEQ_MARK_START:
    MMDL_SetAcmd( p_mmdl, AC_MARK_ONPU_NO_SE );
    p_wk->interest_seq++;
    /* fall */
  case SEQ_MARK_END:
    if( MMDL_EndAcmd( p_mmdl ) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  興味の場所かチェック
 *
 *	@param	RESORT_CROWD_PEOPLE_WK *p_wk  ワーク
 *
 *	@return TRUE興味の場所だった  FALSE通常
 */
//-----------------------------------------------------------------------------
static BOOL Resort_Crowd_People_Wk_CheckInterest( RESORT_CROWD_PEOPLE_WK *p_wk, const FIELD_PLAYER* cp_player )
{
  enum
  {
    CROUWD_INTEREST_RANGE_FRONT   = 20,
    CROUWD_INTEREST_RANGE_BACK    = 7,
  };

  int i;
  s16 x, y, player_z;
  s16 mmdl_z;

  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk->p_people );
  FIELD_PLAYER_GetGridPos( cp_player, &x, &y, &player_z );
  mmdl_z = MMDL_GetGridPosZ(p_mmdl);

  if( !(player_z - CROUWD_INTEREST_RANGE_FRONT < mmdl_z 
    && mmdl_z < player_z + CROUWD_INTEREST_RANGE_BACK) )
  {
    return FALSE;
  }

  for( i = 0; i < p_wk->interest_cnt; i++ )
  {
    if( p_wk->interest_grid_z[i] == MMDL_GetGridPosZ(p_mmdl) 
        && p_wk->interest_index != i )
    {
      p_wk->interest_seq  = 0;
      p_wk->interest_index  = i;
      return TRUE;
    }
  }
  return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *	@param	MMDL * mmdl 動作モデル
 */
//-----------------------------------------------------------------------------
static void Resort_Crowd_People_Mmdl_Init( MMDL * p_mmdl )
{
}

//----------------------------------------------------------------------------
/**
 *	@brief  動作
 *
 *	@param	MMDL * mmdl 動作モデル
 */
//-----------------------------------------------------------------------------
static void Resort_Crowd_People_Mmdl_Move( MMDL * p_mmdl )
{
  MMDLSYS   *p_mmdlsys = MMDL_GetMMdlSys( p_mmdl );
  FIELDMAP_WORK *p_fieldmap = MMDLSYS_GetFieldMapWork( p_mmdlsys );
  RESORT_FIELD_WORK *p_resort_field = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID);

  RESORT_CROWD_PEOPLE_WK *p_wk = Resort_Crowd_People_GetWkByPeople( RESORT_FIELD_PEOPLE_GetCrowdPeople(p_resort_field->p_people_sys), RESORT_PEOPLE_SYS_GetPeopleByMmdl(RESORT_FIELD_PEOPLE_GetPeopleSys(p_resort_field->p_people_sys), p_mmdl) );

  GF_ASSERT( p_wk );
  GF_ASSERT( p_wk->magic_number == RESORT_CROWD_PEOPLE_WK_MAGIC_NUMBER );

  if( p_wk->UpdateMove && p_wk->is_walk )
  {
    p_wk->UpdateMove( p_wk, p_mmdlsys, FIELDMAP_GetFieldPlayer(p_fieldmap) );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  削除
 *
 *	@param	MMDL * mmdl モデル
 */
//-----------------------------------------------------------------------------
static void Resort_Crowd_People_Mmdl_Delete( MMDL * mmdl )
{
}

//----------------------------------------------------------------------------
/**
 *	@brief  リカバー
 *
 *	@param	MMDL * mmdl 動作モデル
 */
//-----------------------------------------------------------------------------
static void Resort_Crowd_People_Mmdl_Recover( MMDL * mmdl )
{
}

//----------------------------------------------------------------------------
/**
 *	@brief  隊列ワーク初期化
 *
 *	@param	RESORT_CROWD_LINE *p_wk ワーク
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_LINE_Init( RESORT_CROWD_LINE *p_wk, u16 x, u16 dir, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_CROWD_LINE) );
  p_wk->x         = x;
  p_wk->move_dir  = dir;
  p_wk->wait  = RESORT_CROWD_LINE_WAIT_RND;
}
//----------------------------------------------------------------------------
/**
 *	@brief  隊列ワーク破棄
 *
 *	@param	RESORT_CROWD_LINE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_LINE_Exit( RESORT_CROWD_LINE *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_CROWD_LINE) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  隊列ワークメイン処理
 *
 *	@param	RESORT_CROWD_LINE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_LINE_Main( RESORT_CROWD_LINE *p_wk, const FIELD_PLAYER* cp_player )
{
  enum
  {
    RESORT_CROWD_LINE_WALK_NUM  = 1,  //１ラインに歩く人の数
  };

  u32 move_cnt  = 0;

  //もし主人公が出口にいたら何もしない
  {
    MMDL *p_mmdl  = FIELD_PLAYER_GetMMdl(cp_player);
    s16 z = MMDL_GetGridPosZ(p_mmdl);
    s16 x = MMDL_GetGridPosX(p_mmdl);
    if( x == p_wk->x )
    {
      if( p_wk->move_dir == DIR_UP )
      {
        if( z == 78 || z == 77 )
        {
          return;
        }
      }
      else if( p_wk->move_dir == DIR_DOWN )
      {
        if( z == 9 || z == 10 )
        {
          return;
        }
      }
    }
  }

  //一定以下のとき移動開始
  {
    int i;
    for( i = 0; i < RESORT_CROWD_MOVE_LINE_PEOPLE_MAX; i++ )
    {
      if( p_wk->p_move[i] && !RESORT_CROWD_PEOPLE_WK_IsEndMove(p_wk->p_move[i]) )
      {
        move_cnt++;
      }
    }
  }

  //移動開始処理
  if( p_wk->register_cnt > 0 && move_cnt < RESORT_CROWD_LINE_WALK_NUM )
  {
    //空だったら飛ばす処理
    while( p_wk->p_move[ p_wk->start_index ] == NULL )
    {
      p_wk->start_index++;
      p_wk->start_index %= RESORT_CROWD_MOVE_LINE_PEOPLE_MAX;
    }

    //もしチュートリアル用ならばカウントを待たない
    if( RESORT_CROWD_PEOPLE_WK_IsTutorial( p_wk->p_move[ p_wk->start_index ] ) )
    {
      p_wk->wait = 0;
    }

    //カウント後に発射
    if( p_wk->wait-- <= 0 )
    {

      if( RESORT_CROWD_PEOPLE_WK_IsEndMove( p_wk->p_move[ p_wk->start_index ] ) )
      {
        RESORT_CROWD_PEOPLE_WK_StartMove( p_wk->p_move[ p_wk->start_index ], AC_WALK_U_8F, p_wk->move_dir, p_wk->x );
        p_wk->wait  = RESORT_CROWD_LINE_WAIT_RND;
      }
      p_wk->start_index++;
      p_wk->start_index %= RESORT_CROWD_MOVE_LINE_PEOPLE_MAX;
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  隊列セットアップ  （初期人物登録後に使用）
 *
 *	@param	RESORT_CROWD_LINE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_LINE_SetUp( RESORT_CROWD_LINE *p_wk, RESORT_CROWD_MOVE move )
{
  //存在データはさっさと動作開始させる
  {
    int i;
    for( i = 0; i < RESORT_CROWD_MOVE_LINE_PEOPLE_MAX; i++ )
    {
      if( p_wk->p_move[i] 
          && RESORT_CROWD_PEOPLE_WK_IsExist(p_wk->p_move[i]) )
      {
        if( move == RESORT_CROWD_MOVE_START )
        {
          if( RESORT_CROWD_PEOPLE_WK_IsNewData(p_wk->p_move[i]) )
          {
            u16 init_z;

            if( p_wk->move_dir == DIR_UP )
            {
              init_z  = i * (8 + GFL_STD_MtRand(8));
            }
            else
            {
              init_z  = (RESORT_CROWD_MOVE_LINE_PEOPLE_MAX-1-i) * (8 + GFL_STD_MtRand(8));
            }

            RESORT_CROWD_PEOPLE_WK_StartMovePos( p_wk->p_move[i], AC_WALK_U_8F, p_wk->move_dir, p_wk->x, init_z );
            p_wk->start_index = i + 1;
            p_wk->start_index %= RESORT_CROWD_MOVE_LINE_PEOPLE_MAX;
          }
        }
        else if( move == RESORT_CROWD_MOVE_RESTART )
        {
          if( !RESORT_CROWD_PEOPLE_WK_IsNewData(p_wk->p_move[i]) )
          {
            RESORT_CROWD_PEOPLE_WK_StartMove( p_wk->p_move[i], AC_WALK_U_8F, p_wk->move_dir, p_wk->x );
          }
        }
      }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  隊列に登録
 *
 *	@param	RESORT_CROWD_LINE *p_wk ワーク
 *	@param	index                   隊列番号
 *	@param	*p_people               人物
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_LINE_Register( RESORT_CROWD_LINE *p_wk, u16 index, RESORT_CROWD_PEOPLE_WK *p_people )
{
  GF_ASSERT( index < RESORT_CROWD_MOVE_LINE_PEOPLE_MAX );
  GF_ASSERT( p_wk->p_move[ index ] == NULL );
  p_wk->p_move[ index ] = p_people;
  p_wk->register_cnt++;
}
//----------------------------------------------------------------------------
/**
 *	@brief  隊列から群集人物をクリア
 *
 *	@param	RESORT_CROWD_LINE *p_wk ワーク
 *	@param  消すインデックス
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_LINE_UnRegister( RESORT_CROWD_LINE *p_wk, u16 index )
{
  GF_ASSERT( index < RESORT_CROWD_MOVE_LINE_PEOPLE_MAX );
  GF_ASSERT( p_wk->p_move[index] );

  p_wk->register_cnt--;
  p_wk->p_move[ index ] = NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  隊列に登録するデータを取得
 *
 *	@param	const RESORT_CROWD_PEOPLE_WK *cp_people ワーク
 *	@param	*p_line   何番目の列か
 *	@param	*p_index  列の中で何番目か
 *
 *	@retval TRUE登録する  FALSE取得登録しない
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_CROWD_LINE_GetRegisterData( const RESORT_PEOPLE *cp_people, u16 *p_line, u16 *p_index )
{ 
  static const struct
  {
    RESORT_DATA_BUFFER_TYPE buffer_type;
    u16                     buffer_index;
    u16                     line_num;
    u32                     line_index;
  } sc_register_data[]  =
  {
    { RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,  0, 0, 0, },
    { RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,  1, 1, 0, },
    { RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,  2, 2, 0, },
    { RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,  3, 3, 0, },
    { RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,  4, 4, 0, },
    { RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,  5, 5, 0, },
    { RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,  6, 0, 1, },
    { RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,  7, 1, 1, },
    { RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF, 0, 2, 1, },
    { RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF, 1, 3, 1, },
    { RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF, 2, 4, 1, },
    { RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF, 3, 5, 1, },
    { RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF, 4, 0, 2, },
    { RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF, 5, 1, 2, },
    { RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF, 6, 2, 2, },
    { RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF, 7, 3, 2, },
    { RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,    0, 4, 2, },
    { RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,    1, 5, 2, },
    { RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,    2, 0, 3, },
    { RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,    3, 1, 3, },
    { RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,    4, 2, 3, },
    { RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,    5, 3, 3, },
    { RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,    6, 4, 3, },
    { RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,    7, 5, 3, },
    { RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,    8, 0, 4, },
    { RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,    9, 1, 4, },
    { RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,   10, 2, 4, },
    { RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,   11, 3, 4, },
  };

  int i;

  RESORT_ABSTRUCT_DATA *p_abstruct = RESORT_PEOPLE_GetAbstructData( (RESORT_PEOPLE *)cp_people );
  RESORT_DATA_BUFFER_TYPE buffer_type   = RESORT_ABSTRUCT_DATA_GetBufferType(p_abstruct);
  u16                     buffer_index  = RESORT_ABSTRUCT_DATA_GetBufferIndex(p_abstruct);
  for( i = 0; i < NELEMS(sc_register_data); i++ )
  {
    if( buffer_type == sc_register_data[i].buffer_type
    && buffer_index == sc_register_data[i].buffer_index )
    {
      *p_line   = sc_register_data[i].line_num;
      *p_index  = sc_register_data[i].line_index;

      GF_ASSERT( (*p_line) < RESORT_CROWD_MOVE_LINE_MAX );
      GF_ASSERT( (*p_index) < RESORT_CROWD_MOVE_LINE_PEOPLE_MAX );
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  人が出てくる待機時間を設定
 *
 *	@param	RESORT_CROWD_LINE *p_wk ワーク
 *	@param	wait 待機時間
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_LINE_SetWait( RESORT_CROWD_LINE *p_wk, s16 wait )
{
  p_wk->wait  = wait;
}

//=============================================================================
/**
 *          騒音管理
 */
//=============================================================================

#define RESORT_CROWD_NOISE_MAX  (8)

//-------------------------------------
///	騒音ワーク
//=====================================
typedef struct 
{
  STRBUF  *p_str;
  u16 flag;
  u8 x;
  u8 y;
  u16 id;
  s16 wait;
} RESORT_CROWD_NOISE_WK;

static void RESORT_CROWD_NOISE_WK_Init( RESORT_CROWD_NOISE_WK *p_wk, u16 id, HEAPID heapID );
static void RESORT_CROWD_NOISE_WK_Exit( RESORT_CROWD_NOISE_WK *p_wk );
static void RESORT_CROWD_NOISE_WK_Main( RESORT_CROWD_NOISE_WK *p_wk, FLDMSGBG* p_msgbg );

typedef enum
{
  RESORT_CROWD_NOISE_MSG_NORMAL_START,
  RESORT_CROWD_NOISE_MSG_INTRODUCTION = RESORT_CROWD_NOISE_MSG_NORMAL_START,
  RESORT_CROWD_NOISE_MSG_PHRASE,
  RESORT_CROWD_NOISE_MSG_THANKS,
  RESORT_CROWD_NOISE_MSG_NORMAL_END,
  RESORT_CROWD_NOISE_MSG_PARADE_START,
  RESORT_CROWD_NOISE_MSG_PLAYER_PHRASE  = RESORT_CROWD_NOISE_MSG_PARADE_START,
  RESORT_CROWD_NOISE_MSG_PLAYER_THANKS,
  RESORT_CROWD_NOISE_MSG_OWNER_NAME,
  RESORT_CROWD_NOISE_MSG_BANZAI,
  RESORT_CROWD_NOISE_MSG_PARADE_END,

  RESORT_CROWD_NOISE_MSG_NORMAL_NUM = RESORT_CROWD_NOISE_MSG_NORMAL_END-RESORT_CROWD_NOISE_MSG_NORMAL_START,
  RESORT_CROWD_NOISE_MSG_PARADE_NUM = RESORT_CROWD_NOISE_MSG_PARADE_END-RESORT_CROWD_NOISE_MSG_PARADE_START,
}RESORT_CROWD_NOISE_MSG;
static void RESORT_CROWD_NOISE_WK_Start( RESORT_CROWD_NOISE_WK *p_wk, FLDMSGBG* p_msgbg, const RESORT_ABSTRUCT_DATA *cp_data, RESORT_CITY_DATA *p_city, RESORT_PEOPLE_ALL_DATA *p_people_all, GFL_MSGDATA *p_msg, RESORT_CROWD_NOISE_MSG msg );
static void RESORT_CROWD_NOISE_WK_Off( RESORT_CROWD_NOISE_WK *p_wk, FLDMSGBG* p_msgbg );
static void RESORT_CROWD_NOISE_WK_Clear( RESORT_CROWD_NOISE_WK *p_wk );
static BOOL RESORT_CROWD_NOISE_WK_IsEnd( const RESORT_CROWD_NOISE_WK *cp_wk );
static BOOL Resort_Crowd_Noise_Wk_IsOverlap( u16 src, u16 dst );

typedef enum
{
  RESORT_CROWD_NOISE_MODE_NORMAL,
  RESORT_CROWD_NOISE_MODE_PARADE,
}RESORT_CROWD_NOISE_MODE;

//-------------------------------------
///	騒音管理
//=====================================
#define RESORT_CROWD_NOISE_WND_DRAW_MAX (3)
struct _RESORT_CROWD_NOISE
{
  BOOL  force_req;
  BOOL  enable;
  s16 wait;
  s16 wait_max;

  s16 wnd_wait;
  s16 wnd_wait_max;
  RESORT_CROWD_NOISE_MSG msg;

  u16 wnd_idx;
  u16 wnd_num;
  u16 add_idx[RESORT_CROWD_NOISE_WND_DRAW_MAX];
  u16  seq;

  FLDMSGBG                  *p_msgbg;
  RESORT_ABSTRUCT_MANAGER   *p_abstruct;
  RESORT_CITY_DATA          *p_city;
  RESORT_PEOPLE_ALL_DATA    *p_people_all;

  RESORT_ABSTRUCT_DATA      *p_src_data;
  RESORT_CROWD_NOISE_WK wk[RESORT_CROWD_NOISE_MAX];

  RESORT_CROWD_NOISE_MODE mode;

  GFL_MSGDATA *p_msg;
};

static void Resort_Crowd_Noise_CheckBufferNum( RESORT_CROWD_NOISE *p_wk );
static RESORT_ABSTRUCT_DATA * Resort_Crowd_Noise_GetDrawLotsData( RESORT_CROWD_NOISE *p_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  騒音初期化
 *
 *	@param	HEAPID heapID ヒープID
 *	@param	p_msgbg   フィールドメッセージBG
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static RESORT_CROWD_NOISE * RESORT_CROWD_NOISE_Init( HEAPID heapID, FLDMSGBG* p_msgbg, RESORT_ABSTRUCT_MANAGER *p_abstruct, RESORT_CITY_DATA *p_city, RESORT_PEOPLE_ALL_DATA *p_people_all, BOOL enable )
{
  RESORT_CROWD_NOISE *p_wk = GFL_HEAP_AllocClearMemory( heapID,sizeof(RESORT_CROWD_NOISE) );
  p_wk->p_msgbg     = p_msgbg;
  p_wk->p_abstruct  = p_abstruct;
  p_wk->p_city      = p_city;
  p_wk->p_people_all  = p_people_all;
  p_wk->enable      = enable;
  p_wk->p_msg       = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_c16_dat, heapID );
  {
    int i;
    for( i = 0; i < RESORT_CROWD_NOISE_MAX; i++ )
    {
      RESORT_CROWD_NOISE_WK_Init( &p_wk->wk[i], i, heapID );
    }
  }

  Resort_Crowd_Noise_CheckBufferNum( p_wk );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  騒音破棄
 *
 *	@param	RESORT_CROWD_NOISE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_NOISE_Exit( RESORT_CROWD_NOISE *p_wk )
{
  {
    int i;
    for( i = 0; i < RESORT_CROWD_NOISE_MAX; i++ )
    {
      RESORT_CROWD_NOISE_WK_Off( &p_wk->wk[i], p_wk->p_msgbg );
      RESORT_CROWD_NOISE_WK_Exit( &p_wk->wk[i] );
    }
  }

  GFL_MSG_Delete( p_wk->p_msg );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  騒音メイン処理
 *
 *	@param	RESORT_CROWD_NOISE *p_wk    ワーク
 *	@param	FIELDMAP_WORK* cp_fieldmap フィールドマップ
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_NOISE_Main( RESORT_CROWD_NOISE *p_wk, const FIELDMAP_WORK* cp_fieldmap )
{
  BOOL is_main  = FALSE;
  BOOL is_clear = FALSE;

  if( !p_wk->enable )
  {
    return;
  }

  if( p_wk->mode == RESORT_CROWD_NOISE_MODE_NORMAL )
  {
    if( FIELDMAP_CheckDoEvent( cp_fieldmap ) == FALSE )
    {
      is_main   = TRUE;
    }
    else
    {
      is_clear  = TRUE;
    }
  }
  else
  {
    if( p_wk->force_req )
    {
      is_main = TRUE;
    }
  }

  if( is_main )
  {
    enum
    {
      SEQ_CHECK,
      SEQ_START,
      SEQ_END,
    };

    switch( p_wk->seq )
    {
    case SEQ_CHECK:
      if( p_wk->wait++ >= p_wk->wait_max )
      {
        Resort_Crowd_Noise_CheckBufferNum( p_wk );

        p_wk->wnd_idx   = 0;
        p_wk->wnd_num   = 1 + GFL_STD_MtRand( RESORT_CROWD_NOISE_WND_DRAW_MAX-1);
        p_wk->wnd_wait  = 0;
        if( p_wk->mode == RESORT_CROWD_NOISE_MODE_NORMAL )
        {
          p_wk->wnd_wait_max  = 30;
          p_wk->msg       = RESORT_CROWD_NOISE_MSG_NORMAL_START+GFL_STD_MtRand( RESORT_CROWD_NOISE_MSG_NORMAL_NUM );
        }
        else
        {
          p_wk->wnd_wait_max  = 10;
        }
        p_wk->wait      = 0;
        p_wk->seq++;
      }
      break;

    case SEQ_START:
      if( p_wk->wnd_wait++ >=p_wk->wnd_wait_max )
      {
        RESORT_ABSTRUCT_DATA *p_data = Resort_Crowd_Noise_GetDrawLotsData( p_wk );
        p_wk->wnd_wait = 0;

        if( p_data )
        {
          u16 index;


          switch( p_wk->wnd_idx )
          {
          default:
            GF_ASSERT(0);
          case 0:
            index = GFL_STD_MtRand(RESORT_CROWD_NOISE_MAX);
            break;
          case 1:
            do
            {
              index = GFL_STD_MtRand(RESORT_CROWD_NOISE_MAX);
            }
            while( Resort_Crowd_Noise_Wk_IsOverlap(index, p_wk->add_idx[0]) );
            break;
          case 2:
            do
            {
              index = GFL_STD_MtRand(RESORT_CROWD_NOISE_MAX);
            }
            while( Resort_Crowd_Noise_Wk_IsOverlap(index, p_wk->add_idx[0])
                || Resort_Crowd_Noise_Wk_IsOverlap(index, p_wk->add_idx[1]) );
            break;
          }

          p_wk->add_idx[ p_wk->wnd_idx ]  = index;


          if( p_wk->mode == RESORT_CROWD_NOISE_MODE_PARADE )
          {
            p_wk->msg       = RESORT_CROWD_NOISE_MSG_PARADE_START+GFL_STD_MtRand( RESORT_CROWD_NOISE_MSG_PARADE_NUM );
          } 

          RESORT_CROWD_NOISE_WK_Start( &p_wk->wk[ index ], p_wk->p_msgbg, p_data, p_wk->p_city, p_wk->p_people_all, p_wk->p_msg, p_wk->msg );
          if( p_wk->wnd_idx++ >= p_wk->wnd_num )
          {
            p_wk->seq = SEQ_END;
          }
        }
      }
      break;

    case SEQ_END:
      {
        int i;
        BOOL is_end = TRUE;
        for( i = 0; i < p_wk->wnd_num; i++ )
        {
          u16 index = p_wk->add_idx[ i ];
          is_end &= RESORT_CROWD_NOISE_WK_IsEnd( &p_wk->wk[ index ] );
        }
        if( is_end )
        {
          p_wk->seq = 0;
          p_wk->force_req = FALSE;
        }
      }
      break;
    default:
      GF_ASSERT(0);
    }

    // メッセージ表示
    {
      int i;
      for( i = 0; i < RESORT_CROWD_NOISE_MAX; i++ )
      {
        RESORT_CROWD_NOISE_WK_Main( &p_wk->wk[i], p_wk->p_msgbg );
      }
    }
  }
  else if( is_clear )
  {
    RESORT_CROWD_NOISE_Clear( p_wk );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  騒音をクリア
 *
 *	@param	RESORT_CROWD_NOISE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_NOISE_Clear( RESORT_CROWD_NOISE *p_wk )
{
  {
    int i;
    for( i = 0; i < RESORT_CROWD_NOISE_MAX; i++ )
    {
      RESORT_CROWD_NOISE_WK_Off( &p_wk->wk[i], p_wk->p_msgbg );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  騒音開始
 *
 *	@param	RESORT_CROWD_NOISE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_NOISE_Start( RESORT_CROWD_NOISE *p_wk )
{
  p_wk->enable = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  パレードモード
 *
 *	@param	RESORT_CROWD_NOISE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_NOISE_SetParadeMode( RESORT_CROWD_NOISE *p_wk )
{
  p_wk->mode = RESORT_CROWD_NOISE_MODE_PARADE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  表示
 *
 *	@param	RESORT_CROWD_NOISE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_NOISE_ForceRequest( RESORT_CROWD_NOISE *p_wk, BOOL is_visible )
{
  if( p_wk->mode == RESORT_CROWD_NOISE_MODE_PARADE )
  {
    if( is_visible )
    {
      p_wk->seq = 0;
      p_wk->wait  = p_wk->wait_max;
      p_wk->force_req = TRUE;
    }
    else
    {
      RESORT_CROWD_NOISE_Clear( p_wk );
      p_wk->force_req = FALSE;
    }
  }
  else
  {
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  バッファの数をチェックしてデータ変更する
 *
 *	@param	RESORT_CROWD_NOISE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Resort_Crowd_Noise_CheckBufferNum( RESORT_CROWD_NOISE *p_wk )
{
  static const RESORT_DATA_BUFFER_TYPE  sc_type[] =
  {
    RESORT_DATA_BUFFER_TYPE_RESIDENT,
    RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,
    RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF,
    RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,
  };
  u32 cnt = 0;

  RESORT_ABSTRUCT_ITERATOR ite = RESORT_ABSTRUCT_MANAGER_BeginIterator( p_wk->p_abstruct, sc_type, NELEMS(sc_type) );

  while(TRUE)
  {
    RESORT_ABSTRUCT_DATA *p_data = RESORT_ABSTRUCT_MANAGER_NextIterator( p_wk->p_abstruct, &ite, sc_type, NELEMS(sc_type) );
    if( p_data == NULL )
    {
      break;
    }
    if( !RESORT_ABSTRUCT_DATA_IsEmpty(p_data) )
    {
      cnt++;
    }
  }

  if( cnt >= 22 )
  {
    p_wk->wait_max  = 1*60;
  }
  else if( cnt >= 16 )
  {
    p_wk->wait_max  = 2*60;
  }
  else if( cnt >= 10 )
  {
    p_wk->wait_max  = 3*60;
  }
  else if( cnt >= 4 )
  {
    p_wk->wait_max  = 4*60;
  }
  else if( cnt >=1 )
  {
    p_wk->wait_max  = 6*60;
  }
  else
  {
    p_wk->wait_max  = 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  データを抽選
 *
 *	@param	RESORT_CROWD_NOISE *p_wk  ワーク
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
static RESORT_ABSTRUCT_DATA * Resort_Crowd_Noise_GetDrawLotsData( RESORT_CROWD_NOISE *p_wk )
{
  enum
  {
    DRAWLOTS_BUFFER_MAX =  RESORT_RESIDENT_DATA_NUM +
    RESONANCE_RESORT_SV_RECV_BUFF_NUM +
    RESONANCE_RESORT_BUFF_RECV_EX_NUM +
    RESONANCE_RESORT_SV_PROXY_BUFF_NUM,
  };

  static const RESORT_DATA_BUFFER_TYPE  sc_type[] =
  {
    RESORT_DATA_BUFFER_TYPE_RESIDENT,
    RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,
    RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF,
    RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,
  };

  static RESORT_DATA_BUFFER_TYPE s_type[DRAWLOTS_BUFFER_MAX];
  static u16 s_index[DRAWLOTS_BUFFER_MAX];
  int cnt = 0;

  RESORT_ABSTRUCT_ITERATOR ite = RESORT_ABSTRUCT_MANAGER_BeginIterator( p_wk->p_abstruct, sc_type, NELEMS(sc_type) );

  while(TRUE)
  {
    RESORT_ABSTRUCT_DATA *p_data = RESORT_ABSTRUCT_MANAGER_NextIterator( p_wk->p_abstruct, &ite, sc_type, NELEMS(sc_type) );
    if( p_data == NULL )
    {
      break;
    } 
    if( !RESORT_ABSTRUCT_DATA_IsEmpty(p_data) )
    {
      s_type[cnt]   = RESORT_ABSTRUCT_DATA_GetBufferType(p_data);
      s_index[cnt]  = RESORT_ABSTRUCT_DATA_GetBufferIndex(p_data);

      cnt++;
    }
  }

  if( cnt == 0 )
  {
    return NULL;
  }
  else if( cnt == 1 )
  {
    return RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_wk->p_abstruct, s_type[0], s_index[0] );
  }
  else
  {
    RESORT_ABSTRUCT_DATA *p_data  = NULL;
    int all_num  = 0;
    int all_max   = GFL_STD_MtRand( cnt );
    int i;
    for( i = 0; i < cnt; i++ )
    {
      if( all_num++ == all_max )
      {
        p_data = RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_wk->p_abstruct, s_type[i], s_index[i] );
        return p_data;
      }
    }
  }
  GF_ASSERT(0);
  return NULL;
}

#if 0
//以前のに戻してと言われたときのため残してある
//----------------------------------------------------------------------------
/**
 *	@brief  データを抽選
 *
 *	@param	RESORT_CROWD_NOISE *p_wk  ワーク
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
static RESORT_ABSTRUCT_DATA * Resort_Crowd_Noise_GetDrawLotsData( RESORT_CROWD_NOISE *p_wk )
{
  enum
  {
    DRAWLOTS_BUFFER_MAX =  RESORT_RESIDENT_DATA_NUM +
    RESONANCE_RESORT_SV_RECV_BUFF_NUM +
    RESONANCE_RESORT_BUFF_RECV_EX_NUM +
    RESONANCE_RESORT_SV_PROXY_BUFF_NUM,
  };

  static const RESORT_DATA_BUFFER_TYPE  sc_type[] =
  {
    RESORT_DATA_BUFFER_TYPE_RESIDENT,
    RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,
    RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF,
    RESORT_DATA_BUFFER_TYPE_PROXY_BUFF,
  };

  static RESORT_DATA_BUFFER_TYPE s_type[DRAWLOTS_BUFFER_MAX];
  static u16 s_index[DRAWLOTS_BUFFER_MAX];
  int cnt = 0;
  int resident_cnt  = 0;
  int tourist_cnt   = 0;

  RESORT_ABSTRUCT_MANAGER_BeginIterator( p_wk->p_abstruct, sc_type, NELEMS(sc_type) );

  while(TRUE)
  {
    RESORT_ABSTRUCT_DATA *p_data = RESORT_ABSTRUCT_MANAGER_NextIterator( p_wk->p_abstruct, sc_type, NELEMS(sc_type) );
    if( p_data == NULL )
    {
      break;
    } 
    if( !RESORT_ABSTRUCT_DATA_IsEmpty(p_data) )
    {
      s_type[cnt]   = RESORT_ABSTRUCT_DATA_GetBufferType(p_data);
      s_index[cnt]  = RESORT_ABSTRUCT_DATA_GetBufferIndex(p_data);

      if( s_type[cnt] == RESORT_DATA_BUFFER_TYPE_RESIDENT )
      {
        resident_cnt++;
      }
      else
      {
        tourist_cnt++;
      }

      cnt++;
    }
  }

  RESORT_FIELD_Printf( "cnt[%d], res[%d] tou[%d]\n", cnt, resident_cnt, tourist_cnt );

  if( cnt == 0 )
  {
    return NULL;
  }
  else if( cnt == 1 )
  {
    return RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_wk->p_abstruct, s_type[0], s_index[0] );
  }
  else
  {
    BOOL is_resident;
    if( resident_cnt > 0 && tourist_cnt > 0 )
    {
      u16 rnd = GFL_STD_MtRand( 100 );
      if( rnd < 25 )
      {
        is_resident = TRUE;
      }
      else
      {
        is_resident = FALSE;
      }
    }
    else if( resident_cnt == 0 )
    {
      is_resident = FALSE;
    }
    else
    {
      is_resident = TRUE;
    }

    if( is_resident == TRUE && resident_cnt == 1 )
    {
      RESORT_ABSTRUCT_DATA *p_data;
      int i;
      for( i = 0; i < cnt; i++ )
      {
        if( s_type[i] == RESORT_DATA_BUFFER_TYPE_RESIDENT )
        {
          break;
        }
      }
      GF_ASSERT( i < cnt );

      p_data = RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_wk->p_abstruct, s_type[i], s_index[i] );

      //住人データがひとつだけのなので、以前のとかぶってた場合は住人以外にする
      if( p_data == p_wk->p_src_data )
      {
        int tourist_num  = 0;
        int tourist_max = GFL_STD_MtRand( tourist_cnt );
        int i;
        for( i = 0; i < cnt; i++ )
        {
          if( s_type[i] != RESORT_DATA_BUFFER_TYPE_RESIDENT )
          {
            if( tourist_num++ == tourist_max )
            {
              return RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_wk->p_abstruct, s_type[i], s_index[i] );

            }
          }
        }
        GF_ASSERT(0);
      }
      else
      {
        return p_data;
      }
    }
    else if( is_resident == FALSE && tourist_cnt == 1 )
    {
      RESORT_ABSTRUCT_DATA *p_data;
      int i;
      for( i = 0; i < cnt; i++ )
      {
        if( s_type[i] != RESORT_DATA_BUFFER_TYPE_RESIDENT )
        {
          break;
        }
      }
      GF_ASSERT( i < cnt );

      p_data = RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_wk->p_abstruct, s_type[i], s_index[i] );

      //住人データがひとつだけのなので、以前のとかぶってた場合は住人以外にする
      if( p_data == p_wk->p_src_data )
      {
        int resident_num  = 0;
        int resident_max = GFL_STD_MtRand( resident_cnt );
        int i;
        for( i = 0; i < cnt; i++ )
        {
          if( s_type[i] == RESORT_DATA_BUFFER_TYPE_RESIDENT )
          {
            if( resident_num++ == resident_max )
            {
              return RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_wk->p_abstruct, s_type[i], s_index[i] );

            }
          }
        }
        GF_ASSERT(0);
      }
      else
      {
        return p_data;
      }
    }
    else
    {
      if( is_resident )
      {
        RESORT_ABSTRUCT_DATA *p_data  = NULL;
        do
        {
          int resident_num  = 0;
          int resident_max = GFL_STD_MtRand( resident_cnt );
          int i;
          for( i = 0; i < cnt; i++ )
          {
            if( s_type[i] == RESORT_DATA_BUFFER_TYPE_RESIDENT )
            {
              if( resident_num++ == resident_max )
              {
                 p_data = RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_wk->p_abstruct, s_type[i], s_index[i] );
                 break;
              }
            }
          }
        }
        while( p_data == p_wk->p_src_data );

        GF_ASSERT( p_data != NULL );

        return p_data;
      }
      else
      {
        RESORT_ABSTRUCT_DATA *p_data  = NULL;
        do
        {
          int tourist_num  = 0;
          int tourist_max = GFL_STD_MtRand( tourist_cnt );
          int i;
          for( i = 0; i < cnt; i++ )
          {
            if( s_type[i] != RESORT_DATA_BUFFER_TYPE_RESIDENT )
            {
              if( tourist_num++ == tourist_max )
              {
                 p_data = RESORT_ABSTRUCT_MANAGER_GetDataByIndex( p_wk->p_abstruct, s_type[i], s_index[i] );
                 break;
              }
            }
          }
        }
        while( p_data == p_wk->p_src_data );

        GF_ASSERT( p_data != NULL );

        return p_data;
      }
    }
  }
  GF_ASSERT(0);
  return NULL;
}
#endif

//----------------------------------------------------------------------------
/**
 *	@brief  騒音ワーク初期化
 *
 *	@param	RESORT_CROWD_NOISE_WK *p_wk ワーク
 *	@param	id ID
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_NOISE_WK_Init( RESORT_CROWD_NOISE_WK *p_wk, u16 id, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_CROWD_NOISE_WK));
  p_wk->p_str = GFL_STR_CreateBuffer( 32+EOM_SIZE, heapID );
  p_wk->id    = id;
}
//----------------------------------------------------------------------------
/**
 *	@brief  騒音ワーク破棄
 *
 *	@param	RESORT_CROWD_NOISE_WK *p_wkワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_NOISE_WK_Exit( RESORT_CROWD_NOISE_WK *p_wk )
{
  GFL_STR_DeleteBuffer( p_wk->p_str );
  GFL_STD_MemClear( p_wk, sizeof(RESORT_CROWD_NOISE_WK));
}
//----------------------------------------------------------------------------
/**
 *	@brief  騒音ワークメイン
 *
 *	@param	RESORT_CROWD_NOISE_WK *p_wk ワーク
 *	@param	p_msgbg フィールドメッセージ
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_NOISE_WK_Main( RESORT_CROWD_NOISE_WK *p_wk, FLDMSGBG* p_msgbg )
{
  if( p_wk->flag == FALSE )
  {
    return ;
  }
  
  // waitまって消える
  p_wk->wait++;
  if( p_wk->wait >= 60 )
  {
    RESORT_CROWD_NOISE_WK_Off(p_wk, p_msgbg);
  }
}

static const u8 sc_NOISE_X[] = 
{
  1,
  22,
  2,
  20,
  4,
  21,
  3,
  19,
  1,
  19,
  2,
  22,
  3,
  20,
  4,
  21,
};
static const u8 sc_NOISE_Y[] = 
{
  6,
  2,
  17,
  14,
  17,
  10,
  14,
  2,
  10,
  17,
  10,
  6,
  2,
  6,
  14,
  6,
};

//----------------------------------------------------------------------------
/**
 *	@brief  範囲がかぶっているかチェック
 *
 *	@param	u16 src
 *	@param	dst 
 *
 *	@return TRUEかぶってる  FALSEかぶってない
 */
//-----------------------------------------------------------------------------
static BOOL Resort_Crowd_Noise_Wk_IsOverlap( u16 src, u16 dst )
{
  enum
  {
    NOISE_WND_WIDTH   = 10/2,
    NOISE_WND_HEIGHT  = 4/2,
  };

	s16 left1, right1, top1, bottom1;
	s16 left2, right2, top2, bottom2;

	left1	  = sc_NOISE_X[src] - NOISE_WND_WIDTH;
	right1	= sc_NOISE_X[src] + NOISE_WND_WIDTH;
	top1	  = sc_NOISE_Y[src] - NOISE_WND_HEIGHT;
	bottom1	= sc_NOISE_Y[src] + NOISE_WND_HEIGHT;

	left2	  = sc_NOISE_X[dst] - NOISE_WND_WIDTH;
	right2	= sc_NOISE_X[dst] + NOISE_WND_WIDTH;
	top2	  = sc_NOISE_Y[dst] - NOISE_WND_HEIGHT;
	bottom2	= sc_NOISE_Y[dst] + NOISE_WND_HEIGHT;
	
	return left1   <= right2
		& right1  >= left2
		& top1    <= bottom2
		& bottom1 >= top2;
}

//----------------------------------------------------------------------------
/**
 *	@brief  騒音ワーク描画開始
 *
 *	@param	RESORT_CROWD_NOISE_WK *p_wk ワーク
 *	@param	p_msgbg                     フィールドメッセージ
 *	@param	p_msgdata                   メッセージ
 *	@param	noise_msg_start       
 *	@param	noise_msg_num 
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_NOISE_WK_Start( RESORT_CROWD_NOISE_WK *p_wk, FLDMSGBG* p_msgbg, const RESORT_ABSTRUCT_DATA *cp_data, RESORT_CITY_DATA *p_city, RESORT_PEOPLE_ALL_DATA *p_people_all, GFL_MSGDATA *p_msg, RESORT_CROWD_NOISE_MSG msg )
{
  TALKMSGWIN_TYPE type;

  GF_ASSERT( p_wk->id < NELEMS(sc_NOISE_X) );
  GF_ASSERT( p_wk->id < NELEMS(sc_NOISE_Y) );

  p_wk->flag = TRUE;
  p_wk->wait = 0;

  switch( msg )
  {
  case RESORT_CROWD_NOISE_MSG_INTRODUCTION:
    {
      STRCODE code[GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN+EOM_SIZE];
      RESORT_ABSTRUCT_DATA_GetData( cp_data, RESORT_DATA_PARAM_INTRODUCTION, code );
      GFL_STR_SetStringCode( p_wk->p_str, code );
    }
    type = TALKMSGWIN_TYPE_NORMAL;
    break;
  default:
    GF_ASSERT(0);
  case RESORT_CROWD_NOISE_MSG_PHRASE:
    {
      STRCODE code[RESORT_PHRASE_LEN+EOM_SIZE];
      RESORT_ABSTRUCT_DATA_GetData( cp_data, RESORT_DATA_PARAM_EXT6_PHRASE, code );
      GFL_STR_SetStringCode( p_wk->p_str, code );
    }
    type = TALKMSGWIN_TYPE_GIZA;
    break;
  case RESORT_CROWD_NOISE_MSG_THANKS:
    {
      STRCODE code[RESORT_THANKS_LEN+EOM_SIZE];
      RESORT_ABSTRUCT_DATA_GetData( cp_data, RESORT_DATA_PARAM_EXT7_THANKS, code );
      GFL_STR_SetStringCode( p_wk->p_str, code );
    }
    type = TALKMSGWIN_TYPE_MOYA;
    break;

  case RESORT_CROWD_NOISE_MSG_PLAYER_PHRASE:
    {
      RESORT_RESIDENT_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData(p_people_all);
      STRCODE code[RESORT_PHRASE_LEN+EOM_SIZE];
      RESORT_RESIDENT_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT6_PHRASE, code );
      GFL_STR_SetStringCode( p_wk->p_str, code );
    }
    type = TALKMSGWIN_TYPE_GIZA;
    break;
  case RESORT_CROWD_NOISE_MSG_PLAYER_THANKS:
    {
      RESORT_RESIDENT_DATA *p_owner = RESORT_PEOPLE_ALL_DATA_GetOwnerData(p_people_all);
      STRCODE code[RESORT_THANKS_LEN+EOM_SIZE];
      RESORT_RESIDENT_DATA_GetData( p_owner, RESORT_DATA_PARAM_EXT7_THANKS, code );
      GFL_STR_SetStringCode( p_wk->p_str, code );
    }
    type = TALKMSGWIN_TYPE_GIZA;
    break;
  case RESORT_CROWD_NOISE_MSG_OWNER_NAME:
    {
      STRCODE code[RESORT_MY_NAME_LEN+EOM_SIZE];
      RESORT_CITY_DATA_GetData( p_city, RESORT_CITY_DATA_PARAM_AUTHER_NAME, code );
      GFL_STR_SetStringCode( p_wk->p_str, code );
    }
    type = TALKMSGWIN_TYPE_GIZA;
    break;
  case RESORT_CROWD_NOISE_MSG_BANZAI:
    GFL_MSG_GetString( p_msg, msg_c16_parade_00, p_wk->p_str );
    break;
  }
  
  {
    u8 cx;
    u8 cy;
    FLDSPWIN_GetNeedWindowCharaSize( p_msgbg, p_wk->p_str, &cx, &cy );
    FLDSUBMSGWIN_AddStrBufEx( p_msgbg,
        p_wk->p_str, p_wk->id, sc_NOISE_X[p_wk->id], sc_NOISE_Y[p_wk->id], cx, 2, type );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  描画OFF
 *
 *	@param	RESORT_CROWD_NOISE_WK *p_wk ワーク
 *	@param	p_msgbg フィールドメッセージ
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_NOISE_WK_Off( RESORT_CROWD_NOISE_WK *p_wk, FLDMSGBG* p_msgbg )
{
  if( p_wk->flag == FALSE )
  {
    return ;
  }
  
  FLDSUBMSGWIN_Delete( p_msgbg, p_wk->id );
  RESORT_CROWD_NOISE_WK_Clear( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  騒音ワーククリア
 *
 *	@param	RESORT_CROWD_NOISE_WK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_NOISE_WK_Clear( RESORT_CROWD_NOISE_WK *p_wk )
{
  p_wk->wait = 0;
  p_wk->flag = FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  騒音ワークが終わっているか
 *
 *	@param	const RESORT_CROWD_NOISE_WK *cp_wk ワーク
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_CROWD_NOISE_WK_IsEnd( const RESORT_CROWD_NOISE_WK *cp_wk )
{
  return cp_wk->flag == FALSE;
}

//=============================================================================
/**
 *          人物管理
 */
//=============================================================================
//-------------------------------------
///	定数
//=====================================
//動作上限数
#define RESORT_FIELD_PEOPLE_MAX (64)
//更新間隔
#define RESORT_FIELD_PEOPLE_UPDATE_INTERVAL (0)

//-------------------------------------
///	人物管理
//=====================================
struct _RESORT_FIELD_PEOPLE
{
  RESORT_PEOPLE_SYS   *p_people_sys;
  RESORT_CROWD_PEOPLE *p_crowd;
  RESORT_FIELD_BUFF   *p_buff;
  RESORT_SYSTEM       *p_sys;
} ;

//-------------------------------------
///	PRIVATE
//=====================================
static BOOL Field_People_UpdateDataCallback( void *p_wk_adrs, void *p_data_adrs, u32 people_type, RESORT_RECV_DATA_UPDATE update );
static BOOL Field_People_UpdateDataCallbackTutorial( void *p_wk_adrs, void *p_data_adrs, u32 people_type, RESORT_RECV_DATA_UPDATE update );
//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  人物管理ワーク作成
 *
 *	@param	HEAPID heapID ヒープID
 *	@param  fieldmap      フィールド	
 *	@param  sys           システム
 *
 *	@return 人物管理ワーク
 */
//-----------------------------------------------------------------------------
static RESORT_FIELD_PEOPLE * RESORT_FIELD_PEOPLE_Init( HEAPID heapID, RESORT_SYSTEM *p_sys, MMDLSYS *p_mmdlsys, const FIELD_PLAYER* cp_player, BOOL is_tutorial, BOOL is_mapchange )
{
  RESORT_FIELD_PEOPLE *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(RESORT_FIELD_PEOPLE));
  p_wk->p_sys = p_sys;
  //バッファ管理
  {
    RESORT_FIELD_BUFF_INIT_PARAM param;
    GFL_STD_MemClear( &param, sizeof( RESORT_FIELD_BUFF_INIT_PARAM ) );
    param.p_recv_buff     = RESORT_SYSTEM_GetRecvBuff( p_sys );
    param.p_recv_tmp_buff = RESORT_SYSTEN_GetRecvTmpBuff( p_sys );
    param.p_proxy_buff    = RESORT_SYSTEM_GetProxyBuff( p_sys );
    param.p_wk_adrs = p_wk;
    if( is_tutorial )
    {
      param.UpdateDataCallback = Field_People_UpdateDataCallbackTutorial;
    }
    else
    {
      param.UpdateDataCallback = Field_People_UpdateDataCallback;
    }
    param.update_interval = RESORT_FIELD_PEOPLE_UPDATE_INTERVAL;


    GF_ASSERT( param.p_recv_tmp_buff != NULL );
    p_wk->p_buff = RESORT_FIELD_BUFF_Init( heapID, &param );
  }

  //人物システム
  {

    RESORT_PEOPLE_SYS_PARAM param;
    GFL_STD_MemClear( &param, sizeof(RESORT_PEOPLE_SYS_PARAM) );
    param.p_mmdlsys = p_mmdlsys;
    param.p_data_manager = RESORT_SYSTEM_GetDataManager( p_sys );
    param.p_abstruct  = RESORT_SYSTEM_GetAbstructManager( p_sys );
    param.max = RESORT_FIELD_PEOPLE_MAX;
    param.placeID = RESORT_PLACEID_FIELD;
    param.cp_shop = RESORT_SYSTEM_GetBinaryShop(p_sys);
    param.cp_institution = RESORT_SYSTEM_GetBinaryInstitution(p_sys);
    p_wk->p_people_sys = RESORT_PEOPLE_SYS_Init( &param, heapID );

  }

  //群集システム
  {
    p_wk->p_crowd = RESORT_CROWD_PEOPLE_Init( heapID, p_mmdlsys, cp_player, 
        RESORT_SYSTEM_GetBinaryShop(p_sys),
        RESORT_SYSTEM_GetBinaryInstitution(p_sys), 
        RESORT_SYSTEM_GetPeopleAllData(p_sys) );
  }

  return p_wk;
}


//----------------------------------------------------------------------------
/**
 *	@brief  セットアップ
 *
 *	@param	RESORT_FIELD_PEOPLE *p_wk
 *	@param	is_tutorial
 *	@param	is_mapchange 
 */
//-----------------------------------------------------------------------------
void RESORT_FIELD_PEOPLE_Setup( RESORT_FIELD_PEOPLE *p_wk, BOOL is_tutorial, BOOL is_mapchange )
{
  //人物追加
  {
    if( is_tutorial )
    {
      static const RESORT_DATA_BUFFER_TYPE type = RESORT_DATA_BUFFER_TYPE_RESIDENT;
      RESORT_PEOPLE_SYS_AddPeopleBufferType( p_wk->p_people_sys, &type, 1 );
    }
    else
    {
      RESORT_PEOPLE_SYS_AddAll( p_wk->p_people_sys );
    }
  }

  //群集登録
  {
    int i;
    RESORT_CROWD_MOVE move;

    for( i = 0; i < RESORT_FIELD_PEOPLE_MAX; i++ )
    {
      RESORT_PEOPLE *p_people =  RESORT_PEOPLE_SYS_GetPeopleByIndex( p_wk->p_people_sys, i );
      if( p_people )
      {
        RESORT_CROWD_PEOPLE_Add( p_wk->p_crowd, p_people );
      }
    }

    //if( is_mapchange )
    if( 0 )
    {
      move = RESORT_CROWD_MOVE_START;
    }
    else
    {
      move = RESORT_CROWD_MOVE_RESTART;
    }

    RESORT_CROWD_PEOPLE_SetUp( p_wk->p_crowd, move );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  人物管理ワーク破棄
 *
 *	@param	RESORT_FIELD_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_PEOPLE_Exit( RESORT_FIELD_PEOPLE *p_wk )
{
  RESORT_CROWD_PEOPLE_Exit( p_wk->p_crowd );
  RESORT_PEOPLE_SYS_Exit( p_wk->p_people_sys );
  RESORT_FIELD_BUFF_Exit( p_wk->p_buff );
  GFL_HEAP_FreeMemory(p_wk);
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物管理ワーク破棄
 *
 *	@param	RESORT_FIELD_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_PEOPLE_Main( RESORT_FIELD_PEOPLE *p_wk, FIELDMAP_WORK* p_fieldmap )
{
  RESORT_FIELD_BUFF_Main( p_wk->p_buff );
  RESORT_PEOPLE_SYS_Main( p_wk->p_people_sys, FIELDMAP_GetFieldPlayer(p_fieldmap) );
  RESORT_CROWD_PEOPLE_Main( p_wk->p_crowd, p_fieldmap );
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物追加を開始
 *
 *	@param	RESORT_FIELD_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_PEOPLE_StartAdd( RESORT_FIELD_PEOPLE *p_wk )
{
  static const RESORT_DATA_BUFFER_TYPE sc_type[]  =
  {
    RESORT_DATA_BUFFER_TYPE_RECV_SV_BUFF,
    RESORT_DATA_BUFFER_TYPE_RECV_TMP_BUFF,
  };
  //RESORT_PEOPLE_SYS_AddPeopleBufferType( p_wk->p_people_sys, sc_type, NELEMS(sc_type) );

  RESORT_FIELD_BUFF_ChangeCallBack( p_wk->p_buff, Field_People_UpdateDataCallback );
}
//----------------------------------------------------------------------------
/**
 *	@brief　人物システムを取得
 *
 *	@param	RESORT_FIELD_PEOPLE *p_wk ワーク
 *
 *	@return 人物システム
 */
//-----------------------------------------------------------------------------
static RESORT_PEOPLE_SYS * RESORT_FIELD_PEOPLE_GetPeopleSys( RESORT_FIELD_PEOPLE *p_wk )
{
  return p_wk->p_people_sys;
}
//----------------------------------------------------------------------------
/**
 *	@brief  群集人物を消す
 *
 *	@param	RESORT_FIELD_PEOPLE *p_wk ワーク
 *	@param	*p_people 人物
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_PEOPLE_DeleteCrowd( RESORT_FIELD_PEOPLE *p_wk, RESORT_PEOPLE *p_people )
{
  RESORT_CROWD_PEOPLE_Delete( p_wk->p_crowd, p_people );
}
//----------------------------------------------------------------------------
/**
 *	@brief  群集システムを取得
 *
 *	@param	RESORT_FIELD_PEOPLE *p_wk   ワーク
 *
 *	@return 群集システム
 */
//-----------------------------------------------------------------------------
static RESORT_CROWD_PEOPLE * RESORT_FIELD_PEOPLE_GetCrowdPeople( RESORT_FIELD_PEOPLE *p_wk )
{
  return p_wk->p_crowd;
}
//----------------------------------------------------------------------------
/**
 *	@brief  パレード用に位置を設定
 *
 *	@param	RESORT_FIELD_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_PEOPLE_SetParadePeoplePos( RESORT_FIELD_PEOPLE *p_wk, RESORT_SYSTEM *p_sys, RESORT_NPC_SYS *p_npc_sys )
{
  static const RESORT_DATA_BUFFER_TYPE  sc_type[] =
  {
    RESORT_DATA_BUFFER_TYPE_RESIDENT
  };

  RESORT_ABSTRUCT_MANAGER *p_man = RESORT_SYSTEM_GetAbstructManager( p_wk->p_sys );
  RESORT_ABSTRUCT_ITERATOR ite = RESORT_ABSTRUCT_MANAGER_BeginIterator( p_man, sc_type, NELEMS(sc_type) );

  //住民の位置を店の前に設定
  while(TRUE)
  {
    RESORT_ABSTRUCT_DATA *p_data = RESORT_ABSTRUCT_MANAGER_NextIterator( p_man, &ite, sc_type, NELEMS(sc_type) );
    if( p_data == NULL )
    {
      break;
    }
    if( !RESORT_ABSTRUCT_DATA_IsEmpty(p_data) )
    {
      RESORT_PEOPLE *p_people = RESORT_PEOPLE_SYS_GetPeopleByAbstruct(p_wk->p_people_sys, p_data);
      MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl(p_people);
      s16 x = MMDL_GetGridPosX(p_mmdl);
      u16 dir;

      if( x < 15 )
      {
        x   = 11;
        dir = DIR_RIGHT;
      }
      else
      {
        x   = 19;
        dir = DIR_LEFT;
      }
      MMDL_InitGridPosition( p_mmdl, x, 0, MMDL_GetGridPosZ(p_mmdl), dir );
    }
  }

  //NPCの視線を固定

  {
    int i;
    for( i = 0; i < RESORT_NPS_SYS_GetMax(p_npc_sys); i++ )
    {
      MMDL *p_mmdl = RESORT_NPC_SYS_GetMmdl( p_npc_sys, i );
      s16 x = MMDL_GetGridPosX(p_mmdl);
      u16 dir;
      if( x < 15 )
      {
        dir = DIR_RIGHT;
      }
      else
      {
        dir = DIR_LEFT;
      }
      MMDL_ChangeMoveCode( p_mmdl, MV_UP + dir );
    }
  }
 

}
//----------------------------------------------------------------------------
/**
 *	@brief 受信データ更新コールバック
 *
 *	@param	void *p_wk_adrs ワーク
 *	@param	*p_recv 更新があったデータ
 *	@param update
 */
//-----------------------------------------------------------------------------
static BOOL Field_People_UpdateDataCallback( void *p_wk_adrs, void *p_data_adrs, u32 people_type, RESORT_RECV_DATA_UPDATE update )
{
  RESORT_FIELD_PEOPLE *p_wk = p_wk_adrs;
  RESORT_PEOPLE *p_people;

  switch( update )
  {
  default:
    GF_ASSERT(0);
    return TRUE;
  case RESORT_RECV_DATA_UPDATE_NONE:         //更新していない
    GF_ASSERT(0);
    return TRUE;
  case RESORT_RECV_DATA_UPDATE_FIRST:        //初回更新        優先度：高
    //何もせず新規人物作成へ
    break;
  case RESORT_RECV_DATA_UPDATE_MERGE_VIEW:   //見た目更新
    //見た目変更
    p_people = RESORT_PEOPLE_SYS_GetPeopleByData( p_wk->p_people_sys, p_data_adrs );
    GF_ASSERT(p_people);
    RESORT_CROWD_PEOPLE_ChangeOBJCodeReq( p_wk->p_crowd, p_people );
    return TRUE;
  case RESORT_RECV_DATA_UPDATE_MERGE_DATA:   //データ更新      優先度：低
    //データの更新のみなら何も行わない
    return TRUE;
  case RESORT_RECV_DATA_UPDATE_OVERWRITE:    //新規データで上書きした
    //上書きした場合は、
    //一旦このデータを消してから新規人物登録
    //アドレスの位置は同じなのでこの人を一旦消す
    p_people = RESORT_PEOPLE_SYS_GetPeopleByData( p_wk->p_people_sys, p_data_adrs );
    if( p_people )
    {
      //群集から外し
      RESORT_FIELD_PEOPLE_DeleteCrowd( p_wk, p_people );
      //完全に人を消す
      RESORT_PEOPLE_Delete( p_wk->p_people_sys, p_people );

      RESORT_FIELD_Printf( "新規データで上書き\n" );
    }
    break;
  }

  //新規人物作成処理
  p_people = RESORT_PEOPLE_SYS_GetPeopleByData( p_wk->p_people_sys, p_data_adrs );
  if( p_people == NULL )
  {
    RESORT_PEOPLE_SYS_ADD_PARAM param;
    GFL_STD_MemClear(&param,sizeof(RESORT_PEOPLE_SYS_ADD_PARAM));

    param.p_data  = RESORT_ABSTRUCT_MANAGER_GetDataByAdrs( RESORT_SYSTEM_GetAbstructManager(p_wk->p_sys), p_data_adrs );
    param.placeID = RESORT_PLACEID_FIELD;

    p_people = RESORT_PEOPLE_SYS_AddNew( p_wk->p_people_sys, &param );
    RESORT_CROWD_PEOPLE_Add( p_wk->p_crowd, p_people );
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief 受信データ更新コールバック チュートリアル番
 *
 *	@param	void *p_wk_adrs ワーク
 *	@param	*p_recv 更新があったデータ
 *	@param update
 */
//-----------------------------------------------------------------------------
static BOOL Field_People_UpdateDataCallbackTutorial( void *p_wk_adrs, void *p_data_adrs, u32 people_type, RESORT_RECV_DATA_UPDATE update )
{
  BOOL is_update = FALSE;

  if( people_type == RESORT_PEOPLE_TYPE_PROXY )
  {
    u32 type = RESORT_PROXY_DATA_GetData(p_data_adrs,RESORT_DATA_PARAM_TYPE,NULL);
    if( type == RESORT_PROXY_TYPE_TUTORIAL_01
        || type == RESORT_PROXY_TYPE_TUTORIAL_02 )
    {
      is_update = TRUE;
    }
  }

  if( is_update )
  {
    return Field_People_UpdateDataCallback( p_wk_adrs, p_data_adrs, people_type, update );
  }

  return FALSE;
}

//=============================================================================
/**
 *          ギミック演出管理
 */
//=============================================================================
enum
{
  RESORT_FIELD_EFFECT_UNIT,
};
//-------------------------------------
///	ギミック演出関数
//=====================================
typedef struct
{
  void (*Start)( RESORT_FIELD_EFFECT *p_wk );
  BOOL (*Update)( RESORT_FIELD_EFFECT *p_wk );
  void (*Stop)( RESORT_FIELD_EFFECT *p_wk );
}RESORT_FIELD_EFFECT_FUNC;

//-------------------------------------
///	ギミック演出ワーク
//=====================================
struct _RESORT_FIELD_EFFECT
{
  FLD_EXP_OBJ_CNT_PTR p_exp_obj;
  FIELD_CAMERA  *p_camera;
  const RESORT_FIELD_EFFECT_FUNC  *cp_func[RESORT_FIELD_EFFECT_ID_MAX];
};


enum
{
  MODEL_RESID_WIN_A_IMD,
  MODEL_RESID_WIN_A_ICA,
  MODEL_RESID_WIN_A_ITA,
  MODEL_RESID_WIN_B_IMD,
  MODEL_RESID_WIN_B_ICA,
  MODEL_RESID_WIN_B_ITA,
};

static const GFL_G3D_UTIL_RES sc_res_table[] = 
{
  // 優勝 エフェクト
  { ARCID_RESONANCE_RESORT_GRAPHIC, NARC_resonance_resort_graphic_ja_ps_a_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_RESONANCE_RESORT_GRAPHIC, NARC_resonance_resort_graphic_ja_ps_a_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_RESONANCE_RESORT_GRAPHIC, NARC_resonance_resort_graphic_ja_ps_a_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
  { ARCID_RESONANCE_RESORT_GRAPHIC, NARC_resonance_resort_graphic_ja_ps_b_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_RESONANCE_RESORT_GRAPHIC, NARC_resonance_resort_graphic_ja_ps_b_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_RESONANCE_RESORT_GRAPHIC, NARC_resonance_resort_graphic_ja_ps_b_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
};
static const GFL_G3D_UTIL_ANM sc_win_a_anm_table[] =
{
  { MODEL_RESID_WIN_A_ICA, 0 }, // ICA
  { MODEL_RESID_WIN_A_ITA, 0 }, // ITA
};
static const GFL_G3D_UTIL_ANM sc_win_b_anm_table[] =
{
  { MODEL_RESID_WIN_B_ICA, 0 }, // ICA
  { MODEL_RESID_WIN_B_ITA, 0 }, // ITA
};

enum
{
  MODEL_OBJ_CONFETTI_START,
  MODEL_OBJ_CONFETTI_LOOP,
};
static const GFL_G3D_UTIL_OBJ sc_obj_table[] =
{
  //MODEL_OBJ_CONFETTI_START
  {
    MODEL_RESID_WIN_A_IMD,         // モデルリソースID
    0,                            // モデルデータID(リソース内部INDEX)
    MODEL_RESID_WIN_A_IMD,         // テクスチャリソースID
    sc_win_a_anm_table,           // アニメテーブル(複数指定のため)
    NELEMS(sc_win_a_anm_table),    // アニメリソース数
  },
  //MODEL_OBJ_CONFETTI_LOOP
  {
    MODEL_RESID_WIN_B_IMD,         // モデルリソースID
    0,                            // モデルデータID(リソース内部INDEX)
    MODEL_RESID_WIN_B_IMD,         // テクスチャリソースID
    sc_win_b_anm_table,           // アニメテーブル(複数指定のため)
    NELEMS(sc_win_b_anm_table),   // アニメリソース数
  },
};

static const GFL_G3D_UTIL_SETUP sc_util_setup = 
{
  sc_res_table,				    //リソーステーブル
	NELEMS(sc_res_table),		//リソース数
	sc_obj_table,				    //オブジェクト設定テーブル
	NELEMS(sc_obj_table),		//オブジェクト数
};

static void Resort_Field_Effect_StartConfetti( RESORT_FIELD_EFFECT *p_wk );
static BOOL Resort_Field_Effect_UpdateConfetti( RESORT_FIELD_EFFECT *p_wk );
static void Resort_Field_Effect_StopConfetti( RESORT_FIELD_EFFECT *p_wk );

static void Resort_Field_Effect_StartParade( RESORT_FIELD_EFFECT *p_wk );
static BOOL Resort_Field_Effect_UpdateParade( RESORT_FIELD_EFFECT *p_wk );
static void Resort_Field_Effect_StopParade( RESORT_FIELD_EFFECT *p_wk );

static const RESORT_FIELD_EFFECT_FUNC sc_effect_func[]  =
{
  //RESORT_FIELD_EFFECT_ID_CONFETTI
  {
    Resort_Field_Effect_StartConfetti,
    Resort_Field_Effect_UpdateConfetti,
    Resort_Field_Effect_StopConfetti,
  },
  //RESORT_FIELD_EFFECT_ID_PARADE
  {
    Resort_Field_Effect_StartParade,
    Resort_Field_Effect_UpdateParade,
    Resort_Field_Effect_StopParade,
  },
};

SDK_COMPILER_ASSERT( NELEMS(sc_effect_func) == RESORT_FIELD_EFFECT_ID_MAX );

//----------------------------------------------------------------------------
/**
 *	@brief  演出ワーク作成
 *
 *	@param	HEAPID heapID ヒープID
 *	@param	p_exp_obj     拡張OBJ
 *
 *	@return 演出ワーク
 */
//-----------------------------------------------------------------------------
static RESORT_FIELD_EFFECT * RESORT_FIELD_EFFECT_Init( HEAPID heapID, FLD_EXP_OBJ_CNT_PTR p_exp_obj, FIELD_CAMERA *p_camera )
{
  RESORT_FIELD_EFFECT *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_FIELD_EFFECT) );
  p_wk->p_exp_obj = p_exp_obj;
  p_wk->p_camera  = p_camera;

  FLD_EXP_OBJ_AddUnit( p_wk->p_exp_obj, &sc_util_setup, RESORT_FIELD_EFFECT_UNIT );

#ifdef DEBUG_EFFECT_MOVE
  RESORT_FIELD_EFFECT_Start( p_wk, RESORT_FIELD_EFFECT_ID_PARADE );
#endif

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  演出ワーク破棄
 *
 *	@param	RESORT_FIELD_EFFECT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_EFFECT_Exit( RESORT_FIELD_EFFECT *p_wk )
{
#ifdef DEBUG_EFFECT_MOVE
  RESORT_FIELD_EFFECT_Stop( p_wk, RESORT_FIELD_EFFECT_ID_CONFETTI );
#endif

  FLD_EXP_OBJ_DelUnit( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  演出ワークメイン処理
 *
 *	@param	RESORT_FIELD_EFFECT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_EFFECT_Main( RESORT_FIELD_EFFECT *p_wk )
{
  FLD_EXP_OBJ_PlayAnime( p_wk->p_exp_obj );

  {
    int i;

    for( i = 0; i < RESORT_FIELD_EFFECT_ID_MAX; i++ )
    {
      if( p_wk->cp_func[i] )
      {
        if( p_wk->cp_func[i]->Update( p_wk ) )
        {
          p_wk->cp_func[i] = NULL;
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  演出開始
 *
 *	@param	RESORT_FIELD_EFFECT *p_wk ワーク
 *	@param	id  開始する演出ID
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_EFFECT_Start( RESORT_FIELD_EFFECT *p_wk, RESORT_FIELD_EFFECT_ID id )
{
  p_wk->cp_func[ id ] = &sc_effect_func[ id ];
  p_wk->cp_func[ id ]->Start( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  演出終了チェック
 *
 *	@param	const RESORT_FIELD_EFFECT *cp_wk  ワーク
 *	@param	id  チェックする演出ID
 *
 *	@return TRUE終了  FALSEまだ
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_FIELD_EFFECT_IsStop( const RESORT_FIELD_EFFECT *cp_wk, RESORT_FIELD_EFFECT_ID id )
{
  return cp_wk->cp_func[id] == NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  演出強制終了
 *
 *	@param	RESORT_FIELD_EFFECT *p_wk ワーク
 *	@param	id  終了する演出ID
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_EFFECT_Stop( RESORT_FIELD_EFFECT *p_wk, RESORT_FIELD_EFFECT_ID id )
{
  if( p_wk->cp_func[id] )
  {
    p_wk->cp_func[id]->Stop( p_wk );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  紙吹雪開始
 *
 *	@param	RESORT_FIELD_EFFECT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Resort_Field_Effect_StartConfetti( RESORT_FIELD_EFFECT *p_wk )
{
  {
    GFL_G3D_OBJSTATUS *p_start_st =FLD_EXP_OBJ_GetUnitObjStatus( p_wk->p_exp_obj,
                            RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START );
    GFL_G3D_OBJSTATUS *p_loop_st =FLD_EXP_OBJ_GetUnitObjStatus( p_wk->p_exp_obj,
                            RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_LOOP );

    p_start_st->trans.x = FX32_CONST(127);
    p_start_st->trans.z = FX32_CONST(500);
    p_loop_st->trans.x = FX32_CONST(127);
    p_loop_st->trans.z = FX32_CONST(500);
  }

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START, FALSE );
  FLD_EXP_OBJ_SetVanish( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_LOOP, TRUE );

  //アニメ再生
  {
    int i;
    for( i=0; i<NELEMS(sc_win_a_anm_table); i++ )
    {
      EXP_OBJ_ANM_CNT_PTR p_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START, i );
      FLD_EXP_OBJ_SetObjAnmFrm( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START, i, 0 );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START, i, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( p_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( p_anm, FALSE );  // アニメ再生設定
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  紙吹雪メイン
 *
 *	@param	RESORT_FIELD_EFFECT *p_wk   ワーク
 *
 *	@return TRUE  終了  FALSEまだ
 */
//-----------------------------------------------------------------------------
static BOOL Resort_Field_Effect_UpdateConfetti( RESORT_FIELD_EFFECT *p_wk )
{
  EXP_OBJ_ANM_CNT_PTR p_start_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START, 0 );

  //開始アニメが表示中だったらチェック
  if( FLD_EXP_OBJ_GetVanish( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START ) == FALSE )
  {
    // アニメ終了したら、ループ用に切り替え
    if( FLD_EXP_OBJ_ChkAnmEnd( p_start_anm ) == TRUE )
    {
      int i;
      // 開始エフェクトは消す
      FLD_EXP_OBJ_SetVanish( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START, TRUE );

      // ループエフェクトは表示
      FLD_EXP_OBJ_SetVanish( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_LOOP, FALSE );
      // ループエフェクトアニメ設定
      for( i=0; i<NELEMS(sc_win_b_anm_table); i++ )
      {
        EXP_OBJ_ANM_CNT_PTR p_loop_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_LOOP, i );
        FLD_EXP_OBJ_SetObjAnmFrm( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_LOOP, i, 0 );
        FLD_EXP_OBJ_ValidCntAnm( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_LOOP, i, TRUE );

        FLD_EXP_OBJ_ChgAnmLoopFlg( p_loop_anm, TRUE );  // ループ再生設定
        FLD_EXP_OBJ_ChgAnmStopFlg( p_loop_anm, FALSE );  // アニメ再生設定
      }
    }
  }

  {
    VecFx32 cam_pos;
    GFL_G3D_OBJSTATUS *p_start_st =FLD_EXP_OBJ_GetUnitObjStatus( p_wk->p_exp_obj,
                            RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START );
    GFL_G3D_OBJSTATUS *p_loop_st =FLD_EXP_OBJ_GetUnitObjStatus( p_wk->p_exp_obj,
                            RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_LOOP );

    GFL_G3D_CAMERA_GetPos( FIELD_CAMERA_GetCameraPtr(p_wk->p_camera), &cam_pos );

    p_start_st->trans.x = cam_pos.x-FX32_CONST(121);
    p_start_st->trans.z = cam_pos.z-FX32_CONST(297);
    p_loop_st->trans  = p_start_st->trans;
  }

#ifdef DEBUG_EFFECT_MOVE
  if( GFL_UI_KEY_GetCont( ) & PAD_BUTTON_L )
  {
    static u16 s_rot = 0xdd00;

    GFL_G3D_OBJSTATUS *p_start_st =FLD_EXP_OBJ_GetUnitObjStatus( p_wk->p_exp_obj,
                            RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START );
    GFL_G3D_OBJSTATUS *p_loop_st =FLD_EXP_OBJ_GetUnitObjStatus( p_wk->p_exp_obj,
                            RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_LOOP );

#if 1
    //回転
    if( GFL_UI_KEY_GetCont( ) & PAD_BUTTON_X )
    {
      s_rot+=0x100;
      MTX_RotX33(&p_start_st->rotate, FX_SinIdx(s_rot), FX_CosIdx(s_rot) );
      MTX_RotX33(&p_loop_st->rotate, FX_SinIdx(s_rot), FX_CosIdx(s_rot) );
    }
    else if( GFL_UI_KEY_GetCont( ) & PAD_BUTTON_Y )
    {
      s_rot-=0x100;
      MTX_RotX33(&p_start_st->rotate, FX_SinIdx(s_rot), FX_CosIdx(s_rot) );
      MTX_RotX33(&p_loop_st->rotate, FX_SinIdx(s_rot), FX_CosIdx(s_rot) );
    }
    s_rot %= 0xFFFF;
    OS_TPrintf( "rot0x%x\n", s_rot );
#endif

#if 1
    //位置
    if( GFL_UI_KEY_GetCont( ) & PAD_KEY_UP )
    {
      p_start_st->trans.z -= FX32_ONE;
      p_loop_st->trans.z -= FX32_ONE;
    }
    else if( GFL_UI_KEY_GetCont( ) & PAD_KEY_DOWN )
    {
      p_start_st->trans.z += FX32_ONE;
      p_loop_st->trans.z += FX32_ONE;
    }
    else if( GFL_UI_KEY_GetCont( ) & PAD_KEY_LEFT )
    {
      p_start_st->trans.x -= FX32_ONE;
      p_loop_st->trans.x -= FX32_ONE;
    }
    else if( GFL_UI_KEY_GetCont( ) & PAD_KEY_RIGHT )
    {
      p_start_st->trans.x += FX32_ONE;
      p_loop_st->trans.x += FX32_ONE;
    }
    OS_TPrintf( "start X%d Z%d\n", FX_Whole(p_start_st->trans.x), FX_Whole( p_start_st->trans.z ) );
    OS_TPrintf( "loop X%d Z%d\n", FX_Whole( p_loop_st->trans.x ), FX_Whole( p_loop_st->trans.z ) );
#endif

  }
#endif

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  紙吹雪終了
 *
 *	@param	RESORT_FIELD_EFFECT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Resort_Field_Effect_StopConfetti( RESORT_FIELD_EFFECT *p_wk )
{
  int i;
  FLD_EXP_OBJ_SetVanish( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START, TRUE );
  FLD_EXP_OBJ_SetVanish( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_LOOP, TRUE );

  for( i=0; i<NELEMS(sc_win_a_anm_table); i++ )
  {
    EXP_OBJ_ANM_CNT_PTR p_loop_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START, i );
    FLD_EXP_OBJ_ValidCntAnm( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START, i, FALSE );
    FLD_EXP_OBJ_ChgAnmStopFlg( p_loop_anm, TRUE );
  }
  for( i=0; i<NELEMS(sc_win_b_anm_table); i++ )
  {
    EXP_OBJ_ANM_CNT_PTR p_loop_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_LOOP, i );
    FLD_EXP_OBJ_ValidCntAnm( p_wk->p_exp_obj, RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_LOOP, i, FALSE );
    FLD_EXP_OBJ_ChgAnmStopFlg( p_loop_anm, TRUE );
  }

  p_wk->cp_func[RESORT_FIELD_EFFECT_ID_CONFETTI] = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  パレード開始
 *
 *	@param	RESORT_FIELD_EFFECT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Resort_Field_Effect_StartParade( RESORT_FIELD_EFFECT *p_wk )
{
  //素材は紙吹雪
  Resort_Field_Effect_StartConfetti( p_wk );

  //位置変更
  {
    GFL_G3D_OBJSTATUS *p_start_st =FLD_EXP_OBJ_GetUnitObjStatus( p_wk->p_exp_obj,
                            RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START );
    GFL_G3D_OBJSTATUS *p_loop_st =FLD_EXP_OBJ_GetUnitObjStatus( p_wk->p_exp_obj,
                            RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_LOOP );

    static const u16 sc_rot = 0xdd00;
    MTX_RotX33(&p_start_st->rotate, FX_SinIdx(sc_rot), FX_CosIdx(sc_rot));
    MTX_RotX33(&p_loop_st->rotate, FX_SinIdx(sc_rot), FX_CosIdx(sc_rot));
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  パレードメイン
 *
 *	@param	RESORT_FIELD_EFFECT *p_wk   ワーク
 *
 *	@return TRUE  終了  FALSEまだ
 */
//-----------------------------------------------------------------------------
static BOOL Resort_Field_Effect_UpdateParade( RESORT_FIELD_EFFECT *p_wk )
{
  //素材は紙吹雪
  BOOL ret = Resort_Field_Effect_UpdateConfetti( p_wk );

  //位置変更処理
  {
    VecFx32 cam_pos;
    GFL_G3D_OBJSTATUS *p_start_st =FLD_EXP_OBJ_GetUnitObjStatus( p_wk->p_exp_obj,
                            RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_START );
    GFL_G3D_OBJSTATUS *p_loop_st =FLD_EXP_OBJ_GetUnitObjStatus( p_wk->p_exp_obj,
                            RESORT_FIELD_EFFECT_UNIT, MODEL_OBJ_CONFETTI_LOOP );

    GFL_G3D_CAMERA_GetPos( FIELD_CAMERA_GetCameraPtr(p_wk->p_camera), &cam_pos );

    p_start_st->trans.x = cam_pos.x-FX32_CONST(121);
    p_start_st->trans.z = cam_pos.z-FX32_CONST(200);
    p_start_st->trans.y = cam_pos.y-FX32_CONST(200);
    p_loop_st->trans  = p_start_st->trans;
  }

  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  パレード終了
 *
 *	@param	RESORT_FIELD_EFFECT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Resort_Field_Effect_StopParade( RESORT_FIELD_EFFECT *p_wk )
{
  //素材は紙吹雪
  Resort_Field_Effect_StopConfetti(p_wk);
}

//=============================================================================
/**
 *          その他
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ファン出現処理
 *
 *	@param	RESORT_SYSTEM *p_sys システム
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_AppearFun( RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata, HEAPID heapID )
{
  RESORT_CITY_DATA *p_city = RESORT_SYSTEM_GetCityData( p_sys );

  if( !RESORT_CITY_DATA_GetData(p_city,RESORT_CITY_DATA_PARAM_IS_TUTORIAL,NULL ) 
      && RESORT_CITY_DATA_GetData(p_city,RESORT_CITY_DATA_FUN_DAY_FLAG, NULL) == FALSE )
  {
    int i;
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork(p_gamedata);
    RESONANCE_RESORT_SV *p_reso_sv = RESONANCE_RESORT_SV_GetWork(p_sv);
    RESORT_PROXY_BUFF *p_buff = RESORT_SYSTEM_GetProxyBuff( p_sys );
    RESORT_PROXY_DATA *p_proxy = RESORT_PROXY_DATA_Init( GFL_HEAP_LOWID(heapID) );
    
    u32 fun_appear_num = 0;

    {
      static const u8 sc_card_rank_add[]  =
      {
        1,2,3,4,5,5
      };
      static const u8 sc_badge_add[]  =
      {
        1,1,1,1,2,2,2,2,
      };
      static const u8 sc_medal_add[]  =
      {
        1,2,3,4,5
      };

      u32 rank  = TRAINERCARD_GetCardRank( p_gamedata );
      u8  badge = MISC_GetBadgeCount( SaveData_GetMisc(p_sv) );
      u16 medal = MEDALBOX_GetMedalCount( SaveData_GetMedalBox(p_sv) , FALSE );

      if( rank < NELEMS(sc_card_rank_add) )
      {
        fun_appear_num += sc_card_rank_add[ rank ];
      }
      else
      {
        fun_appear_num += sc_card_rank_add[ NELEMS(sc_card_rank_add)-1 ];
      }
      if( rank < NELEMS(sc_badge_add) )
      {
        fun_appear_num += sc_badge_add[ rank ];
      }
      else
      {
        fun_appear_num += sc_badge_add[ NELEMS(sc_badge_add)-1 ];
      }
      if( rank < NELEMS(sc_medal_add) )
      {
        fun_appear_num += sc_medal_add[ rank ];
      }
      else
      {
        fun_appear_num += sc_medal_add[ NELEMS(sc_medal_add)-1 ];
      }
    }

    for( i = 0; i < fun_appear_num; i++ )
    {
      RESORT_UTIL_SetProxyRandom( p_sys, p_proxy, RESORT_PROXY_TYPE_FUN, heapID );
      if( RESONANCE_RESORT_SV_SetData( p_reso_sv, p_gamedata, p_proxy, RESORT_MERGE_TYPE_PROXY ) != RESORT_SETDATA_SET )
      {
        continue;
      }
    }

    RESORT_PROXY_DATA_Exit( p_proxy );

    RESORT_CITY_DATA_SetData(p_city,RESORT_CITY_DATA_FUN_DAY_FLAG, TRUE);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  セレブ出現処理
 *
 *	@param	RESORT_SYSTEM *p_sys システム
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_AppearCeleb( RESORT_SYSTEM *p_sys, GAMEDATA *p_gamedata, HEAPID heapID )
{
  SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork(p_gamedata);
  RESORT_BINARY_CITY *p_city_data = RESORT_SYSTEM_GetBinaryCity(p_sys);
  RESORT_CITY_DATA *p_city = RESORT_SYSTEM_GetCityData( p_sys );
  EVENTWORK *p_evwork = GAMEDATA_GetEventWork( p_gamedata );
  RESONANCE_RESORT_SV *p_reso_sv = RESONANCE_RESORT_SV_GetWork(p_sv);

  BOOL is_ok = FALSE;

  //殿堂入りしているかつ１日１回フラグ
  if( EVENTWORK_CheckEventFlag( p_evwork, SYS_FLAG_GAME_CLEAR )
    && !RESORT_CITY_DATA_GetData(p_city,RESORT_CITY_DATA_CELEB_DAY_FLAG,NULL) )
  {
    RESORT_PROXY_BUFF *p_proxy_buff = RESORT_SYSTEM_GetProxyBuff(p_sys);
    int i;
    is_ok = TRUE;
    
    //セレブがいないとき
    for( i = 0; i < RESORT_PROXY_BUFF_GetMax(p_proxy_buff); i++ )
    {
      RESORT_PROXY_DATA *p_data = RESORT_PROXY_BUFF_GetData( p_proxy_buff, i );
      if( !RESORT_PROXY_DATA_IsEmpty(p_data) )
      {
        if( RESORT_PROXY_DATA_GetData(p_data,RESORT_DATA_PARAM_TYPE,NULL) == RESORT_PROXY_TYPE_CELEB )
        {
          is_ok = FALSE;
          break;
        }
      }
    }

    //１日フラグを立てる
    RESORT_CITY_DATA_SetData(p_city,RESORT_CITY_DATA_CELEB_DAY_FLAG,TRUE);
  }

#ifdef PM_DEBUG
  if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_FORCE_CELEB) )
  {
    is_ok = TRUE;
  }
#endif

  if( is_ok )
  {

    u32 lv = RESORT_CITY_DATA_GetData(p_city,RESORT_CITY_DATA_PARAM_LV,NULL);
    u32 city_idx = RESORT_BINARY_CITY_GetIndexByCityLv( p_city_data, lv );

    u32 celeb_per =  RESORT_BINARY_CITY_GetData( p_city_data, city_idx, RESORT_BINARY_CITY_PARAM_CELEB_PER );

    u32 rnd = RESORT_CITY_DATA_GetRnd( p_city, RESORT_RND_ID_CITY_CELEV, 100 );

#ifdef PM_DEBUG
  if( RESORT_DEBUG_FLAG_Get(RESORT_DEBUG_FLAG_FORCE_CELEB) )
  {
    celeb_per = 100;
    rnd = 0;
  }
#endif

    if( rnd < celeb_per )
    {
      //セレブいれる
      RESORT_PROXY_DATA *p_proxy = RESORT_PROXY_DATA_Init( GFL_HEAP_LOWID(heapID) );

      RESORT_UTIL_SetProxyRandom( p_sys, p_proxy, RESORT_PROXY_TYPE_CELEB, heapID );

      if( RESONANCE_RESORT_SV_SetData( p_reso_sv, p_gamedata, p_proxy, RESORT_MERGE_TYPE_PROXY ) == RESORT_SETDATA_SET )
      {
        //交換フラグOFF
        RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_CELEB_TRADE_FLAG, FALSE );
      }
      RESORT_PROXY_DATA_Exit( p_proxy );

    }
  }
}
