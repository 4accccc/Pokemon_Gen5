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

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define RESORT_FIELD_PEOPLE_MAX   (32)
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
 *          人物管理
 */
//=============================================================================
//-------------------------------------
///	人物管理
//=====================================
typedef struct _RESORT_FIELD_PEOPLE RESORT_FIELD_PEOPLE;

static RESORT_FIELD_PEOPLE * RESORT_FIELD_PEOPLE_Init( HEAPID heapID, RESORT_SYSTEM *p_sys, MMDLSYS *p_mmdlsys );
static void RESORT_FIELD_PEOPLE_Exit( RESORT_FIELD_PEOPLE *p_wk );
static void RESORT_FIELD_PEOPLE_Main( RESORT_FIELD_PEOPLE *p_wk, FIELD_PLAYER *p_player );
static RESORT_PEOPLE_SYS * RESORT_FIELD_PEOPLE_GetPeopleSys( RESORT_FIELD_PEOPLE *p_wk );
static void RESORT_FIELD_PEOPLE_Action( RESORT_FIELD_PEOPLE *p_wk, RESORT_PEOPLE *p_people, u16 param );

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
  p_wk->p_people_sys = RESORT_FIELD_PEOPLE_Init( heapID, p_wk->p_sys, GAMEDATA_GetMMdlSys(p_gamedata) );

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

  NAGI_Printf( "レゾナンスsetup\n" );
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
 *          人物管理
 */
//=============================================================================
//-------------------------------------
///	定数
//=====================================
//動作上限数
#define RESORT_FIELD_PEOPLE_MAX (32)
//更新間隔
#define RESORT_FIELD_PEOPLE_UPDATE_INTERVAL (0)

//-------------------------------------
///	人物が動作するときのワーク
//=====================================
typedef struct
{
  RESORT_PEOPLE *p_people;
  u32 seq;
} RESORT_FIELD_PEOPLE_ACT;

static void Reception_People_CreateEnterTask( GFL_TCBLSYS * p_tcblsys, RESORT_PEOPLE *p_people );
static void Reception_People_CreateLeaveTask( GFL_TCBLSYS * p_tcblsys, RESORT_PEOPLE *p_people );
static void Reception_People_CreateReplaceTask( GFL_TCBLSYS * p_tcblsys, RESORT_PEOPLE *p_people );
static void Reception_People_Act_Enter( GFL_TCBL *p_tcbl, void *p_wk_adrs );
static void Reception_People_Act_Leave( GFL_TCBL *p_tcbl, void *p_wk_adrs );
static void Reception_People_Act_Replace( GFL_TCBL *p_tcbl, void *p_wk_adrs );

//-------------------------------------
///	人物管理
//=====================================
struct _RESORT_FIELD_PEOPLE
{
  RESORT_PEOPLE_SYS *p_people_sys;
  RESORT_FIELD_BUFF    *p_buff;
  GFL_TCBLSYS       *p_tcblsys;
  RESORT_SYSTEM     *p_sys;
} ;

//-------------------------------------
///	PRIVATE
//=====================================
static void Reception_People_UpdateRecvCallback( void *p_wk_adrs, RESORT_RECV_DATA *p_recv, RESORT_RECV_DATA_UPDATE update );
static void Reception_People_UpdateProxyCallback( void *p_wk_adrs, RESORT_PROXY_DATA *p_proxy );
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
static RESORT_FIELD_PEOPLE * RESORT_FIELD_PEOPLE_Init( HEAPID heapID, RESORT_SYSTEM *p_sys, MMDLSYS *p_mmdlsys )
{
  RESORT_FIELD_PEOPLE *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(RESORT_FIELD_PEOPLE));
  p_wk->p_sys = p_sys;
  //TCBL
  p_wk->p_tcblsys = GFL_TCBL_Init( heapID, heapID, RESORT_FIELD_PEOPLE_MAX, sizeof(RESORT_FIELD_PEOPLE_ACT) );

  //バッファ管理
  {
    RESORT_FIELD_BUFF_INIT_PARAM param;
    GFL_STD_MemClear( &param, sizeof( RESORT_FIELD_BUFF_INIT_PARAM ) );
    param.p_recv_buff     = RESORT_SYSTEM_GetRecvBuff( p_sys );
    param.p_recv_tmp_buff = RESORT_SYSTEN_GetRecvTmpBuff( p_sys );
    param.p_proxy_buff    = RESORT_SYSTEM_GetProxyBuff( p_sys );
    param.p_wk_adrs = p_wk;
    param.UpdateRecvCallback = Reception_People_UpdateRecvCallback;
    param.UpdateProxyCallback = Reception_People_UpdateProxyCallback;
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
    RESORT_PEOPLE_SYS_EraseMMdl( p_wk->p_people_sys );  //拡張バッファで戻ってくることを考えて一旦消す
    RESORT_PEOPLE_SYS_AddAll( p_wk->p_people_sys );
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
  RESORT_PEOPLE_SYS_Exit( p_wk->p_people_sys );
  RESORT_FIELD_BUFF_Exit( p_wk->p_buff );
  GFL_TCBL_DeleteAll( p_wk->p_tcblsys );
  GFL_TCBL_Exit( p_wk->p_tcblsys );
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
  GFL_TCBL_Main( p_wk->p_tcblsys );
  RESORT_PEOPLE_SYS_Main( p_wk->p_people_sys, p_player );
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
 *	@brief 　人物の行動
 *
 *	@param	RESORT_FIELD_PEOPLE *p_wk　ワーク
 *	@param	*p_people 行動する人物
 *	@param	param     行動状態
 */
//-----------------------------------------------------------------------------
static void RESORT_FIELD_PEOPLE_Action( RESORT_FIELD_PEOPLE *p_wk, RESORT_PEOPLE *p_people, u16 param )
{
  switch( param )
  {
  case RESORT_ACTION_LEAVE:
    Reception_People_CreateLeaveTask( p_wk->p_tcblsys, p_people );
    break;
  default:
    GF_ASSERT(0);
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
static void Reception_People_UpdateRecvCallback( void *p_wk_adrs, RESORT_RECV_DATA *p_recv, RESORT_RECV_DATA_UPDATE update )
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
    //Reception_People_CreateReplaceTask( p_wk->p_tcblsys, p_people );
  }
  else
  {
    //いないなら人物作成
    RESORT_PEOPLE_SYS_ADD_PARAM param;
    GFL_STD_MemClear(&param,sizeof(RESORT_PEOPLE_SYS_ADD_PARAM));

    param.p_data  = RESORT_ABSTRUCT_MANAGER_GetDataByAdrs( RESORT_SYSTEM_GetAbstructManager(p_wk->p_sys), p_recv );
    param.placeID = RESORT_PLACEID_FIELD;

    p_people = RESORT_PEOPLE_SYS_AddNew( p_wk->p_people_sys, &param );
    RESORT_PEOPLE_SetVisible( p_people, TRUE );

    //人物作成したら、上から落下してくるタスクを登録
    //Reception_People_CreateEnterTask( p_wk->p_tcblsys, p_people );
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
static void Reception_People_UpdateProxyCallback( void *p_wk_adrs, RESORT_PROXY_DATA *p_proxy )
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

    //人物作成したら、上から落下してくるタスクを登録
    //Reception_People_CreateEnterTask( p_wk->p_tcblsys, p_people );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief 入ってくるタスク作成
 *
 *	@param	GFL_TCBL *p_tcbl　タスクシステム
 *	@param	*p_people         入ってくる人物
 */
//-----------------------------------------------------------------------------
static void Reception_People_CreateEnterTask( GFL_TCBLSYS * p_tcblsys, RESORT_PEOPLE *p_people )
{
  GFL_TCBL * p_tcbl = GFL_TCBL_Create(p_tcblsys, Reception_People_Act_Enter, sizeof(RESORT_FIELD_PEOPLE_ACT), 0);
  RESORT_FIELD_PEOPLE_ACT *p_act = GFL_TCBL_GetWork(p_tcbl);
  GFL_STD_MemClear( p_act, sizeof(RESORT_FIELD_PEOPLE_ACT) );

  p_act->p_people = p_people;

  RESORT_PEOPLE_SetTalkFlag( p_act->p_people, FALSE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  入ってくる処理
 *
 *	@param	GFL_TCBL *p_tcbl  TCBL
 *	@param	*p_wk_adrs        ワーク
 */
//-----------------------------------------------------------------------------
static void Reception_People_Act_Enter( GFL_TCBL *p_tcbl, void *p_wk_adrs )
{
  enum
  {
    SEQ_START,
    SEQ_WAIT,
    SEQ_END,
  };

  RESORT_FIELD_PEOPLE_ACT *p_wk = p_wk_adrs;

  switch( p_wk->seq )
  {
  case SEQ_START:
    RESORT_PEOPLE_ActionReq( p_wk->p_people, RESORT_PEOPLE_ACTION_WARPDOWN );
    p_wk->seq++;
    break;
  case SEQ_WAIT:
    if( RESORT_PEOPLE_IsActionEnd( p_wk->p_people ) )
    {
      p_wk->seq++;
    }
    break;
  case SEQ_END:
    RESORT_PEOPLE_SetTalkFlag( p_wk->p_people, TRUE );
    GFL_TCBL_Delete(p_tcbl);
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief　降りてくる動作
 *
 *	@param	GFL_TCBLSYS * p_tcblsys タスクシステム
 *	@param	*p_people　 人物
 */
//-----------------------------------------------------------------------------
static void Reception_People_CreateLeaveTask( GFL_TCBLSYS * p_tcblsys, RESORT_PEOPLE *p_people )
{
  GFL_TCBL * p_tcbl = GFL_TCBL_Create(p_tcblsys, Reception_People_Act_Leave, sizeof(RESORT_FIELD_PEOPLE_ACT), 0);
  RESORT_FIELD_PEOPLE_ACT *p_act = GFL_TCBL_GetWork(p_tcbl);
  GFL_STD_MemClear( p_act, sizeof(RESORT_FIELD_PEOPLE_ACT) );

  p_act->p_people = p_people;

  RESORT_PEOPLE_SetTalkFlag( p_act->p_people, FALSE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  出て行く処理
 *
 *	@param	GFL_TCBL *p_tcbl  TCBL
 *	@param	*p_wk_adrs        ワーク
 */
//-----------------------------------------------------------------------------
static void Reception_People_Act_Leave( GFL_TCBL *p_tcbl, void *p_wk_adrs )
{
  enum
  {
    SEQ_START,
    SEQ_WAIT,
    SEQ_END,
  };

  RESORT_FIELD_PEOPLE_ACT *p_wk = p_wk_adrs;

  switch( p_wk->seq )
  {
  case SEQ_START:
    RESORT_PEOPLE_ActionReq( p_wk->p_people, RESORT_PEOPLE_ACTION_WARPUP );
    p_wk->seq++;
    break;
  case SEQ_WAIT:
    if( RESORT_PEOPLE_IsActionEnd( p_wk->p_people ) )
    {
      p_wk->seq++;
    }
    break;
  case SEQ_END:
    RESORT_PEOPLE_SetTalkFlag( p_wk->p_people, TRUE );
    GFL_TCBL_Delete(p_tcbl);
    break;
  default:
    GF_ASSERT(0);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  一旦上がって降りてくる動作
 *
 *	@param	GFL_TCBLSYS * p_tcblsys タスクシステム
 *	@param	*p_people     人物
 */
//-----------------------------------------------------------------------------
static void Reception_People_CreateReplaceTask( GFL_TCBLSYS * p_tcblsys, RESORT_PEOPLE *p_people )
{
  GFL_TCBL * p_tcbl = GFL_TCBL_Create(p_tcblsys, Reception_People_Act_Replace, sizeof(RESORT_FIELD_PEOPLE_ACT), 0);
  RESORT_FIELD_PEOPLE_ACT *p_act = GFL_TCBL_GetWork(p_tcbl);
  GFL_STD_MemClear( p_act, sizeof(RESORT_FIELD_PEOPLE_ACT) );
  p_act->p_people = p_people;

  //話しかけできなくする
  RESORT_PEOPLE_SetTalkFlag( p_act->p_people, FALSE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  入れ替える処理
 *
 *	@param	GFL_TCBL *p_tcbl  TCBL
 *	@param	*p_wk_adrs        ワーク
 */
//-----------------------------------------------------------------------------
static void Reception_People_Act_Replace( GFL_TCBL *p_tcbl, void *p_wk_adrs )
{
  enum
  {
    SEQ_REAVE_START,
    SEQ_REAVE_WAIT,
    SEQ_ENTER_START,
    SEQ_ENTER_WAIT,
    SEQ_END,
  };

  RESORT_FIELD_PEOPLE_ACT *p_wk = p_wk_adrs;

  switch( p_wk->seq )
  {
  case SEQ_REAVE_START:
    RESORT_PEOPLE_ActionReq( p_wk->p_people, RESORT_PEOPLE_ACTION_WARPUP );
    p_wk->seq++;
    break;
  case SEQ_REAVE_WAIT:
    if( RESORT_PEOPLE_IsActionEnd( p_wk->p_people ) )
    {
      RESORT_PEOPLE_ChangeOBJCode( p_wk->p_people );
      p_wk->seq++;
    }
    break;
  case SEQ_ENTER_START:
    RESORT_PEOPLE_ActionReq( p_wk->p_people, RESORT_PEOPLE_ACTION_WARPDOWN );
    p_wk->seq++;
    break;
  case SEQ_ENTER_WAIT:
    if( RESORT_PEOPLE_IsActionEnd( p_wk->p_people ) )
    {
      p_wk->seq++;
    }
    break;
  case SEQ_END:
    RESORT_PEOPLE_SetTalkFlag( p_wk->p_people, TRUE );
    GFL_TCBL_Delete(p_tcbl);
    break;
  default:
    GF_ASSERT(0);
  }
}
