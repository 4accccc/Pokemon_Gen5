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

//script
#include "../script.h"     //for SCRIPT_CallScript

//archive

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

//mine
#include "resort_field.h"


#ifdef PM_DEBUG
#define DEBUG_RESORT_FIELD_PRINT_ON
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
  void (*UpdateRecvCallback)( void *p_wk_adrs, RESORT_RECV_DATA *p_recv, RESORT_RECV_DATA_UPDATE update ); //データに更新があれば更新間隔ごとに呼ばれる
  void (*UpdateProxyCallback)( void *p_wk_adrs, RESORT_PROXY_DATA *p_proxy ); //データに更新があれば更新間隔ごとに呼ばれる
  u32 update_interval; //更新間隔
}RESORT_FIELD_BUFF_INIT_PARAM;

static RESORT_FIELD_BUFF * RESORT_FIELD_BUFF_Init( HEAPID heapID, const RESORT_FIELD_BUFF_INIT_PARAM *cp_param );
static void RESORT_FIELD_BUFF_Exit( RESORT_FIELD_BUFF *p_wk );
static void RESORT_FIELD_BUFF_Main( RESORT_FIELD_BUFF *p_wk );

//=============================================================================
/**
 *          群集管理
 */
//=============================================================================
//-------------------------------------
///	群集管理
//=====================================
typedef struct _RESORT_CROWD_PEOPLE RESORT_CROWD_PEOPLE;

static RESORT_CROWD_PEOPLE * RESORT_CROWD_PEOPLE_Init( HEAPID heapID, MMDLSYS* p_mmdlsys, const FIELD_PLAYER* cp_player );
static void RESORT_CROWD_PEOPLE_Exit( RESORT_CROWD_PEOPLE *p_wk );
static void RESORT_CROWD_PEOPLE_Main( RESORT_CROWD_PEOPLE *p_wk );
static void RESORT_CROWD_PEOPLE_SetUp( RESORT_CROWD_PEOPLE *p_wk );

static void RESORT_CROWD_PEOPLE_Add( RESORT_CROWD_PEOPLE *p_wk, RESORT_PEOPLE *p_people );

//=============================================================================
/**
 *          騒音管理
 */
//=============================================================================
//-------------------------------------
///	騒音管理
//=====================================
typedef struct _RESORT_CROWD_NOISE  RESORT_CROWD_NOISE;

static RESORT_CROWD_NOISE * RESORT_CROWD_NOISE_Init( HEAPID heapID, FLDMSGBG* p_msgbg, RESORT_ABSTRUCT_MANAGER *p_abstruct );
static void RESORT_CROWD_NOISE_Exit( RESORT_CROWD_NOISE *p_wk );
static void RESORT_CROWD_NOISE_Main( RESORT_CROWD_NOISE *p_wk, const FIELDMAP_WORK* cp_fieldmap );
static void RESORT_CROWD_NOISE_Clear( RESORT_CROWD_NOISE *p_wk );

//=============================================================================
/**
 *          人物管理
 */
//=============================================================================
//-------------------------------------
///	人物管理
//=====================================
typedef struct _RESORT_FIELD_PEOPLE RESORT_FIELD_PEOPLE;

static RESORT_FIELD_PEOPLE * RESORT_FIELD_PEOPLE_Init( HEAPID heapID, RESORT_SYSTEM *p_sys, MMDLSYS *p_mmdlsys, const FIELD_PLAYER* cp_player );
static void RESORT_FIELD_PEOPLE_Exit( RESORT_FIELD_PEOPLE *p_wk );
static void RESORT_FIELD_PEOPLE_Main( RESORT_FIELD_PEOPLE *p_wk, FIELD_PLAYER *p_player );
static RESORT_PEOPLE_SYS * RESORT_FIELD_PEOPLE_GetPeopleSys( RESORT_FIELD_PEOPLE *p_wk );

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

  //汎用ワーク確保
  RESORT_FIELD_WORK *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, RESORT_FIELD_ASSIGN_ID, heapID, sizeof(RESORT_FIELD_WORK));

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

    p_wk->p_sys = RESORT_SYSTEM_Init( &param, heapID );
  }

  //人物システム
  p_wk->p_people_sys = RESORT_FIELD_PEOPLE_Init( heapID, p_wk->p_sys, GAMEDATA_GetMMdlSys(p_gamedata), FIELDMAP_GetFieldPlayer(p_fieldmap) );

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
    p_wk->p_noise = RESORT_CROWD_NOISE_Init( heapID, FIELDMAP_GetFldMsgBG(p_fieldmap), RESORT_SYSTEM_GetAbstructManager( p_wk->p_sys ) );
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

  RESORT_CROWD_NOISE_Exit( p_wk->p_noise );
  RESORT_NPC_SYS_Exit( p_wk->p_npc_sys );
  RESORT_FIELD_PEOPLE_Exit( p_wk->p_people_sys );
  RESORT_SYSTEM_Exit( p_wk->p_sys );

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

  RESORT_FIELD_PEOPLE_Main( p_wk->p_people_sys, FIELDMAP_GetFieldPlayer(p_fieldmap) );
  RESORT_NPC_SYS_Main( p_wk->p_npc_sys );
  RESORT_CROWD_NOISE_Main( p_wk->p_noise, p_fieldmap );
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
      p_wk->param.UpdateRecvCallback( p_wk->param.p_wk_adrs, p_recv, update );
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

      p_wk->param.UpdateRecvCallback( p_wk->param.p_wk_adrs, p_recv, update );
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
      p_wk->param.UpdateProxyCallback( p_wk->param.p_wk_adrs, p_proxy );
    }
  }

  p_wk->is_proxy_update = TRUE;
}

//=============================================================================
/**
 *          群集管理
 */
//=============================================================================
#define RESORT_CROWD_PEOPLE_BUFFER_MAX      (32)
#define RESORT_CROWD_MOVE_LINE_MAX          (6)
#define RESORT_CROWD_MOVE_LINE_PEOPLE_MAX   (4)
#define RESORT_CROWD_MOVE_LIMIT_BACK        (9)
#define RESORT_CROWD_MOVE_LIMIT_FRONT       (77)

//-------------------------------------
///	１人ずつのワーク
//=====================================
typedef struct
{
  BOOL  is_walk;
  RESORT_PEOPLE *p_people;
  u16 move_acmd;
  u16 move_dir;
  u16 seq;
  s16 move_grid;
} RESORT_CROWD_PEOPLE_WK;

static void RESORT_CROWD_PEOPLE_WK_Init( RESORT_CROWD_PEOPLE_WK *p_wk, HEAPID heapID );
static void RESORT_CROWD_PEOPLE_WK_Exit( RESORT_CROWD_PEOPLE_WK *p_wk );
static void RESORT_CROWD_PEOPLE_WK_Main( RESORT_CROWD_PEOPLE_WK *p_wk, MMDLSYS* p_fos, const FIELD_PLAYER* cp_player );
static void RESORT_CROWD_PEOPLE_WK_SetUp( RESORT_CROWD_PEOPLE_WK *p_wk, RESORT_PEOPLE *p_people );
static BOOL RESORT_CROWD_PEOPLE_WK_IsExist( const RESORT_CROWD_PEOPLE_WK *cp_wk );
static void RESORT_CROWD_PEOPLE_WK_StartMove( RESORT_CROWD_PEOPLE_WK *p_wk, u16 acmd, u16 move_dir, u16 x );
static void RESORT_CROWD_PEOPLE_WK_StartMovePos( RESORT_CROWD_PEOPLE_WK *p_wk, u16 acmd, u16 move_dir, u16 x, u16 z );
static BOOL RESORT_CROWD_PEOPLE_WK_IsEndMove( const RESORT_CROWD_PEOPLE_WK *cp_wk );
static BOOL RESORT_CLOWD_PEOPLE_WK_IsNewData( const RESORT_CROWD_PEOPLE_WK *cp_wk );
static u16 RESORT_CLOWD_PEOPLE_WK_GetDir( const RESORT_CROWD_PEOPLE_WK *cp_wk );

static BOOL Resort_Crowd_People_Wk_IsFrontPlayer( const RESORT_CROWD_PEOPLE_WK *cp_wk, const MMDL* cp_mmdl, u16 grid, u16 min_grid );
static BOOL Resort_Crowd_People_Wk_IsFrontAllMMdl( const RESORT_CROWD_PEOPLE_WK* cp_wk, const MMDLSYS* cp_fos, MMDL** pp_mmdl, u16 grid, u16 min_grid );
static BOOL Resort_Crowd_People_Wk_IsGridHit( s16 mygx, s16 mygz, s16 pl_gx, s16 pl_gz, u16 grid, u16 min_grid, u16 move_dir );

//-------------------------------------
///	隊列ワーク
//=====================================
typedef struct
{
  RESORT_CROWD_PEOPLE_WK *p_move[RESORT_CROWD_MOVE_LINE_PEOPLE_MAX];
  u16 start_index;
  s16 wait;
  u16 register_cnt;
  u16 move_dir;
  u16 x;
  u16 dummy;
} RESORT_CROWD_LINE;

#define RESORT_CROWD_LINE_WAIT_MIN  (60)
#define RESORT_CROWD_LINE_WAIT_MAX  (180)
#define RESORT_CROWD_LINE_WAIT_DIF  (RESORT_CROWD_LINE_WAIT_MAX-RESORT_CROWD_LINE_WAIT_MIN)
#define RESORT_CROWD_LINE_WAIT_RND  (RESORT_CROWD_LINE_WAIT_MIN+GFL_STD_MtRand( RESORT_CROWD_LINE_WAIT_DIF ))

static void RESORT_CROWD_LINE_Init( RESORT_CROWD_LINE *p_wk, u16 x, HEAPID heapID );
static void RESORT_CROWD_LINE_Exit( RESORT_CROWD_LINE *p_wk );
static void RESORT_CROWD_LINE_Main( RESORT_CROWD_LINE *p_wk );
static void RESORT_CROWD_LINE_SetUp( RESORT_CROWD_LINE *p_wk );
static void RESORT_CLOWD_LINE_Register( RESORT_CROWD_LINE *p_wk, u16 index, RESORT_CROWD_PEOPLE_WK *p_people );
static BOOL RESORT_CLOWD_LINE_GetRegisterData( RESORT_PEOPLE *p_people, u16 *p_line, u16 *p_index );

//-------------------------------------
///	群集管理
//=====================================
struct _RESORT_CROWD_PEOPLE
{
  RESORT_CROWD_LINE       line[RESORT_CROWD_MOVE_LINE_MAX];
  RESORT_CROWD_PEOPLE_WK  wk[RESORT_CROWD_PEOPLE_BUFFER_MAX];
  const FIELD_PLAYER      *cp_player;
  MMDLSYS                 *p_mmdlsys;
};

//----------------------------------------------------------------------------
/**
 *	@brief  群集管理初期化
 *
 *	@param	HEAPID heapID ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static RESORT_CROWD_PEOPLE * RESORT_CROWD_PEOPLE_Init( HEAPID heapID, MMDLSYS* p_mmdlsys, const FIELD_PLAYER* cp_player )
{
  RESORT_CROWD_PEOPLE *p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(RESORT_CROWD_PEOPLE) );
  p_wk->cp_player = cp_player;
  p_wk->p_mmdlsys = p_mmdlsys;

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

    int i;
    for( i = 0; i < RESORT_CROWD_MOVE_LINE_MAX; i++ )
    {
      RESORT_CROWD_LINE_Init( &p_wk->line[i], sc_line_x[i], heapID );
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
static void RESORT_CROWD_PEOPLE_Main( RESORT_CROWD_PEOPLE *p_wk )
{
  //各ライン動作
  {
    int i;
    for( i = 0; i < RESORT_CROWD_MOVE_LINE_MAX; i++ )
    {
      RESORT_CROWD_LINE_Main( &p_wk->line[i] );
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
static void RESORT_CROWD_PEOPLE_SetUp( RESORT_CROWD_PEOPLE *p_wk )
{
  //各ライン動作
  {
    int i;
    for( i = 0; i < RESORT_CROWD_MOVE_LINE_MAX; i++ )
    {
      RESORT_CROWD_LINE_SetUp( &p_wk->line[i] );
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
  {
    int i;
    for( i = 0; i < RESORT_CROWD_PEOPLE_BUFFER_MAX; i++ )
    {
      if( !RESORT_CROWD_PEOPLE_WK_IsExist(&p_wk->wk[i]) )
      {
        u16 line;
        u16 index;
        if( RESORT_CLOWD_LINE_GetRegisterData( p_people, &line, &index ) )
        {
          //ワークへの設定
          RESORT_CROWD_PEOPLE_WK_SetUp( &p_wk->wk[i], p_people );

          //隊列への登録
          RESORT_CLOWD_LINE_Register( &p_wk->line[line], index, &p_wk->wk[i] );
        }
        break;
      }
    }
  }
  
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
  GFL_STD_MemClear( p_wk, sizeof(RESORT_CROWD_PEOPLE_WK) );
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
  GFL_STD_MemClear( p_wk, sizeof(RESORT_CROWD_PEOPLE_WK) );
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
  enum
  {
    SEQ_WAIT,
    SEQ_WALKWAIT,
    SEQ_NORMAL_WALK,
  };

  if( RESORT_CROWD_PEOPLE_WK_IsExist(p_wk) )
  {
    switch( p_wk->seq )
    {
    case SEQ_WAIT:
      if( p_wk->is_walk )
      {
        p_wk->seq++;
      }
      break;
    case SEQ_WALKWAIT:
      {
        MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk->p_people );
        if( MMDL_CheckEndAcmd( p_mmdl ) )
        {
          p_wk->seq++;
        }
        else
        {
          break;
        }
      }
      /* fall throght */
    case SEQ_NORMAL_WALK:
      //歩き終了
      {
        if( p_wk->move_grid <= 0 )
        {
          p_wk->is_walk = FALSE;
          p_wk->seq = SEQ_WAIT;
          RESORT_PEOPLE_SetVisible( p_wk->p_people, FALSE );
        }
        else
        {

          MMDL* p_hit_mmdl;
          if( Resort_Crowd_People_Wk_IsFrontAllMMdl( p_wk, p_mmdlsys,
                &p_hit_mmdl, 1, 1 ) )
          {
            /*  */
          }
          else
          {
            MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( p_wk->p_people );
            MMDL_SetAcmd( p_mmdl, p_wk->move_acmd + p_wk->move_dir );
            p_wk->move_grid--;
            p_wk->seq = SEQ_WALKWAIT;
          }
        }
      }
      break;
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
static void RESORT_CROWD_PEOPLE_WK_SetUp( RESORT_CROWD_PEOPLE_WK *p_wk, RESORT_PEOPLE *p_people )
{
  GF_ASSERT( p_wk->p_people == NULL );
  p_wk->p_people = p_people;

  RESORT_PEOPLE_SetVisible( p_wk->p_people, FALSE );
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

    if( RESORT_CLOWD_PEOPLE_WK_IsNewData(p_wk) )
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
          MMDL_SetVectorPos( p_mmdl, &vec );
        }
        else
        {
          p_wk->move_grid = now_z - RESORT_CROWD_MOVE_LIMIT_BACK+1;
        }
      }
      else if( p_wk->move_dir == DIR_DOWN )
      {
        if( now_z > RESORT_CROWD_MOVE_LIMIT_FRONT )
        {
          MMDL_SetGridPosZ( p_mmdl, RESORT_CROWD_MOVE_LIMIT_BACK );
          vec.z = GRID_TO_FX32(RESORT_CROWD_MOVE_LIMIT_BACK) + MMDL_VEC_Z_GRID_OFFS_FX32;
          p_wk->move_grid = RESORT_CROWD_MOVE_LIMIT_FRONT-RESORT_CROWD_MOVE_LIMIT_BACK+1;
          MMDL_SetVectorPos( p_mmdl, &vec );
        }
        else
        {
          p_wk->move_grid = RESORT_CROWD_MOVE_LIMIT_FRONT - now_z+1;
        }
      }
    }
  }

  RESORT_PEOPLE_SetVisible( p_wk->p_people, TRUE );
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
 *	@return TRUE新規データ  FALSE読み込みデータ
 */
//-----------------------------------------------------------------------------
static BOOL RESORT_CLOWD_PEOPLE_WK_IsNewData( const RESORT_CROWD_PEOPLE_WK *cp_wk )
{
  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( cp_wk->p_people );

  s16 now_x = MMDL_GetGridPosX( p_mmdl );
  s16 now_z = MMDL_GetGridPosZ( p_mmdl );
  //もし座標０，０だったら新規データ
  return ( now_x == 0 && now_z == 0 );
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
static u16 RESORT_CLOWD_PEOPLE_WK_GetDir( const RESORT_CROWD_PEOPLE_WK *cp_wk )
{
  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( cp_wk->p_people );
  return MMDL_GetDirDisp(p_mmdl);
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
static BOOL Resort_Crowd_People_Wk_IsFrontPlayer( const RESORT_CROWD_PEOPLE_WK *cp_wk, const MMDL* cp_mmdl, u16 grid, u16 min_grid )
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
static BOOL Resort_Crowd_People_Wk_IsFrontAllMMdl( const RESORT_CROWD_PEOPLE_WK* cp_wk, const MMDLSYS* cp_fos, MMDL** pp_mmdl, u16 grid, u16 min_grid )
{
	u32 no = 0;
	MMDL *mmdl;
  BOOL result = FALSE;

  MMDL *p_mmdl = RESORT_PEOPLE_GetMmdl( cp_wk->p_people );

	while( MMDLSYS_SearchUseMMdl(cp_fos,&mmdl,&no) == TRUE ){
    if( p_mmdl != mmdl ){
      if( MMDL_CheckStatusBit(mmdl,MMDL_STABIT_FELLOW_HIT_NON) == 0 )
      {
        // 表示されている？ 
        if( MMDL_CheckStatusBitVanish( mmdl ) == FALSE ){
          result = Resort_Crowd_People_Wk_IsFrontPlayer( cp_wk, mmdl, grid, min_grid );
          if( result ){
            *pp_mmdl = mmdl;
            return TRUE;
          }
        }
      }
    }
  }
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
static BOOL Resort_Crowd_People_Wk_IsGridHit( s16 mygx, s16 mygz, s16 pl_gx, s16 pl_gz, u16 grid, u16 min_grid, u16 move_dir )
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
 *	@brief  隊列ワーク初期化
 *
 *	@param	RESORT_CROWD_LINE *p_wk ワーク
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_LINE_Init( RESORT_CROWD_LINE *p_wk, u16 x, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(RESORT_CROWD_LINE) );
  p_wk->x = x;
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
static void RESORT_CROWD_LINE_Main( RESORT_CROWD_LINE *p_wk )
{
  //移動開始処理
  if( p_wk->register_cnt > 0 )
  {
    if( p_wk->wait-- <= 0 )
    {
      //空だったら飛ばす処理
      while( p_wk->p_move[ p_wk->start_index ] == NULL )
      {
        p_wk->start_index++;
        if( p_wk->start_index >= RESORT_CROWD_MOVE_LINE_PEOPLE_MAX )
        {
          p_wk->start_index = 0;
        }
      }

      if( RESORT_CROWD_PEOPLE_WK_IsEndMove( p_wk->p_move[ p_wk->start_index ] ) )
      {
        RESORT_CROWD_PEOPLE_WK_StartMove( p_wk->p_move[ p_wk->start_index ], AC_WALK_U_8F, p_wk->move_dir, p_wk->x );
        p_wk->wait  = RESORT_CROWD_LINE_WAIT_RND;
      }
      p_wk->start_index++;
    }
  }
#if 0
  //移動終了検知
  {
    int i;
    BOOL is_end_move = TRUE;

    //すべての人物が歩いていない
    for( i = 0; i < RESORT_CROWD_MOVE_LINE_PEOPLE_MAX; i++ )
    {
      if( p_wk->p_move[i] )
      {
        is_end_move &= RESORT_CROWD_PEOPLE_WK_IsEndMove( p_wk->p_move[i] );
      }
    }

    //すべての人物に開始をだしている
    is_end_move &= ( p_wk->start_index >= RESORT_CROWD_MOVE_LINE_PEOPLE_MAX );

    if( is_end_move )
    {
      p_wk->start_index = 0;
      p_wk->wait  = RESORT_CROWD_LINE_WAIT_RND;
    }
  }
#endif
}
//----------------------------------------------------------------------------
/**
 *	@brief  隊列セットアップ  （初期人物登録後に使用）
 *
 *	@param	RESORT_CROWD_LINE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_CROWD_LINE_SetUp( RESORT_CROWD_LINE *p_wk )
{
  //存在データから方向を取得する
  {
    int i;
    u16 dir_up    = 0;
    u16 dir_down  = 0;
    for( i = 0; i < RESORT_CROWD_MOVE_LINE_PEOPLE_MAX; i++ )
    {
      if( p_wk->p_move[i] 
          && RESORT_CROWD_PEOPLE_WK_IsExist(p_wk->p_move[i])
          && !RESORT_CLOWD_PEOPLE_WK_IsNewData(p_wk->p_move[i]))
      {
        u16 dir = RESORT_CLOWD_PEOPLE_WK_GetDir( p_wk->p_move[i] );
        if( dir == DIR_UP )
        {
          dir_up++;
        }
        else if( dir == DIR_DOWN )
        {
          dir_down++;
        }
      }
    }

    if( dir_up < dir_down )
    {
      p_wk->move_dir  = DIR_DOWN;
    }
    else if( dir_up > dir_down )
    {
      p_wk->move_dir  = DIR_UP;
    }
    else
    {
      p_wk->move_dir  = GFL_STD_MtRand( 2 ) ? DIR_UP: DIR_DOWN;
    }
  }

  //存在データはさっさと動作開始させる
  {
    int i;
    for( i = 0; i < RESORT_CROWD_MOVE_LINE_PEOPLE_MAX; i++ )
    {
      if( p_wk->p_move[i] 
          && RESORT_CROWD_PEOPLE_WK_IsExist(p_wk->p_move[i])
          && RESORT_CLOWD_PEOPLE_WK_IsNewData(p_wk->p_move[i]) )
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
static void RESORT_CLOWD_LINE_Register( RESORT_CROWD_LINE *p_wk, u16 index, RESORT_CROWD_PEOPLE_WK *p_people )
{
  GF_ASSERT( index < RESORT_CROWD_MOVE_LINE_PEOPLE_MAX );
  GF_ASSERT( p_wk->p_move[ index ] == NULL );
  p_wk->p_move[ index ] = p_people;
  p_wk->register_cnt++;
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
static BOOL RESORT_CLOWD_LINE_GetRegisterData( RESORT_PEOPLE *p_people, u16 *p_line, u16 *p_index )
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
  };

  int i;

  RESORT_ABSTRUCT_DATA *p_abstruct = RESORT_PEOPLE_GetAbstructData( p_people );
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
static void RESORT_CROWD_NOISE_WK_Start( RESORT_CROWD_NOISE_WK *p_wk, FLDMSGBG* p_msgbg, const RESORT_ABSTRUCT_DATA *cp_data );
static void RESORT_CROWD_NOISE_WK_Off( RESORT_CROWD_NOISE_WK *p_wk, FLDMSGBG* p_msgbg );
static void RESORT_CROWD_NOISE_WK_Clear( RESORT_CROWD_NOISE_WK *p_wk );

//-------------------------------------
///	騒音管理
//=====================================
struct _RESORT_CROWD_NOISE
{
  s16 wait;
  s16 wait_max;
  u32 add_idx;
  FLDMSGBG                  *p_msgbg;
  RESORT_ABSTRUCT_MANAGER   *p_abstruct;
  RESORT_ABSTRUCT_DATA      *p_src_data;
  RESORT_CROWD_NOISE_WK wk[RESORT_CROWD_NOISE_MAX];
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
static RESORT_CROWD_NOISE * RESORT_CROWD_NOISE_Init( HEAPID heapID, FLDMSGBG* p_msgbg, RESORT_ABSTRUCT_MANAGER *p_abstruct )
{
  RESORT_CROWD_NOISE *p_wk = GFL_HEAP_AllocClearMemory( heapID,sizeof(RESORT_CROWD_NOISE) );
  p_wk->p_msgbg     = p_msgbg;
  p_wk->p_abstruct  = p_abstruct;


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
  if( FIELDMAP_CheckDoEvent( cp_fieldmap ) == FALSE )
  {
    p_wk->wait++;

    // ウエイトたったら、ランダムメッセージ表示
    if( p_wk->wait >= p_wk->wait_max )
    {
      // 表示
      u32 src = p_wk->add_idx;

      do
      {
        p_wk->add_idx = GFL_STD_MtRand(RESORT_CROWD_NOISE_MAX);
      }while( src == p_wk->add_idx );

      p_wk->p_src_data = Resort_Crowd_Noise_GetDrawLotsData( p_wk );

      if( p_wk->p_src_data != NULL )
      {
        RESORT_CROWD_NOISE_WK_Start( &p_wk->wk[ p_wk->add_idx ], p_wk->p_msgbg, p_wk->p_src_data );

        // ウエイト初期化
        p_wk->wait = 0;
      }
      Resort_Crowd_Noise_CheckBufferNum( p_wk );
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
  else
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
      cnt++;
    }
  }

  if( cnt >= 16 )
  {
    p_wk->wait_max  = 2*60;
  }
  else if( cnt >= 10 )
  {
    p_wk->wait_max  = 4*60;
  }
  else if( cnt >= 4 )
  {
    p_wk->wait_max  = 8*60;
  }
  else if( cnt >=1 )
  {
    p_wk->wait_max  = 16*60;
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
  p_wk->p_str = GFL_STR_CreateBuffer( RESORT_PHRASE_LEN+EOM_SIZE, heapID );
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
static void RESORT_CROWD_NOISE_WK_Start( RESORT_CROWD_NOISE_WK *p_wk, FLDMSGBG* p_msgbg, const RESORT_ABSTRUCT_DATA *cp_data )
{
  static const GFL_POINT sc_pos[] =
  {
    { 2, 2},
    {18, 2},
    { 2, 6},
    {18, 6},
    { 2,10},
    {18,10},
    { 2,14},
    {18,14},
  };

  GF_ASSERT( p_wk->id < NELEMS(sc_pos) );

  p_wk->flag = TRUE;
  p_wk->wait = 0;

  {
    STRCODE code[RESORT_PHRASE_LEN+EOM_SIZE];
    RESORT_ABSTRUCT_DATA_GetData( cp_data, RESORT_DATA_PARAM_EXT6_PHRASE, code );
    GFL_STR_SetStringCode( p_wk->p_str, code );
  }
  
  FLDSUBMSGWIN_AddStrBuf( p_msgbg,
    p_wk->p_str, p_wk->id, sc_pos[p_wk->id].x, sc_pos[p_wk->id].y, 12, 2 );
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
static void Field_People_UpdateRecvCallback( void *p_wk_adrs, RESORT_RECV_DATA *p_recv, RESORT_RECV_DATA_UPDATE update );
static void Field_People_UpdateProxyCallback( void *p_wk_adrs, RESORT_PROXY_DATA *p_proxy );
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
static RESORT_FIELD_PEOPLE * RESORT_FIELD_PEOPLE_Init( HEAPID heapID, RESORT_SYSTEM *p_sys, MMDLSYS *p_mmdlsys, const FIELD_PLAYER* cp_player )
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
    param.UpdateRecvCallback = Field_People_UpdateRecvCallback;
    param.UpdateProxyCallback = Field_People_UpdateProxyCallback;
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
    p_wk->p_people_sys = RESORT_PEOPLE_SYS_Init( &param, heapID );
    RESORT_PEOPLE_SYS_AddAll( p_wk->p_people_sys );
  }

  //群集システム
  {
    int i;
    p_wk->p_crowd = RESORT_CROWD_PEOPLE_Init( heapID, p_mmdlsys, cp_player );

    for( i = 0; i < RESORT_FIELD_PEOPLE_MAX; i++ )
    {
      RESORT_PEOPLE *p_people =  RESORT_PEOPLE_SYS_GetPeopleByIndex( p_wk->p_people_sys, i );
      if( p_people )
      {
        RESORT_CROWD_PEOPLE_Add( p_wk->p_crowd, p_people );
      }
    }

    RESORT_CROWD_PEOPLE_SetUp( p_wk->p_crowd );
  }

  return p_wk;
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
static void RESORT_FIELD_PEOPLE_Main( RESORT_FIELD_PEOPLE *p_wk, FIELD_PLAYER *p_player )
{
  RESORT_FIELD_BUFF_Main( p_wk->p_buff );
  RESORT_PEOPLE_SYS_Main( p_wk->p_people_sys, p_player );
  RESORT_CROWD_PEOPLE_Main( p_wk->p_crowd );
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
 *	@brief 受信データ更新コールバック
 *
 *	@param	void *p_wk_adrs ワーク
 *	@param	*p_recv 更新があったデータ
 *	@param update
 */
//-----------------------------------------------------------------------------
static void Field_People_UpdateRecvCallback( void *p_wk_adrs, RESORT_RECV_DATA *p_recv, RESORT_RECV_DATA_UPDATE update )
{
  RESORT_FIELD_PEOPLE *p_wk = p_wk_adrs;
  RESORT_PEOPLE *p_people;

  if( update == RESORT_RECV_DATA_UPDATE_MERGE_DATA )
  {
    //データの更新のみなら何も行わない
    return;
  }

  //受信したデータ番地をもっている人がいるか検索
  p_people = RESORT_PEOPLE_SYS_GetPeopleByData( p_wk->p_people_sys, p_recv );

  if( p_people )
  {
    //いるならなら置き換え動作
  }
  else
  {
    //いないなら人物作成
    RESORT_PEOPLE_SYS_ADD_PARAM param;
    GFL_STD_MemClear(&param,sizeof(RESORT_PEOPLE_SYS_ADD_PARAM));

    param.p_data  = RESORT_ABSTRUCT_MANAGER_GetDataByAdrs( RESORT_SYSTEM_GetAbstructManager(p_wk->p_sys), p_recv );
    param.placeID = RESORT_PLACEID_FIELD;

    p_people = RESORT_PEOPLE_SYS_AddNew( p_wk->p_people_sys, &param );

    RESORT_CROWD_PEOPLE_Add( p_wk->p_crowd, p_people );

  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  代理人更新コールバック
 *　
 *	@param	void *p_wk_adrs　ワーク
 *	@param	*p_proxy  更新があったデータ
 */
//-----------------------------------------------------------------------------
static void Field_People_UpdateProxyCallback( void *p_wk_adrs, RESORT_PROXY_DATA *p_proxy )
{
  RESORT_FIELD_PEOPLE *p_wk = p_wk_adrs;
  RESORT_PEOPLE *p_people;

  //受信したデータ番地をもっている人がいるか検索
  p_people = RESORT_PEOPLE_SYS_GetPeopleByData( p_wk->p_people_sys, p_proxy );

  if( p_people )
  {
    //ここに来た場合、一旦代理人を生成して、
    //セーブや復帰に来た場合なので、MMDLが残っているため作成しない
  }
  else
  {
    RESORT_PEOPLE_SYS_ADD_PARAM param;
    GFL_STD_MemClear(&param,sizeof(RESORT_PEOPLE_SYS_ADD_PARAM));
    param.p_data  = RESORT_ABSTRUCT_MANAGER_GetDataByAdrs( RESORT_SYSTEM_GetAbstructManager(p_wk->p_sys), p_proxy );
    param.placeID = RESORT_PLACEID_FIELD;
    p_people = RESORT_PEOPLE_SYS_AddNew( p_wk->p_people_sys, &param );

    RESORT_CROWD_PEOPLE_Add( p_wk->p_crowd, p_people );
  }
}

