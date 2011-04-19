//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_reception.c
 *	@brief  レゾナンスリゾート迎賓館メインソース
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
#include "scrcmd_resort_def.h"

//mine
#include "resort_reception.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *          バッファ管理
 */
//=============================================================================
//-------------------------------------
///	バッファ管理
//=====================================
typedef struct _RECEPTION_BUFF RECEPTION_BUFF;

//-------------------------------------
///	初期化構造体
//=====================================
typedef struct
{
  GAMEDATA* p_gamedata; //ゲームデータ
  void *p_wk_adrs;      //コールバックに与える引数
  void (*UpdateRecvCallback)( void *p_wk_adrs, RESORT_RECV_DATA *p_recv, RESORT_RECV_DATA_UPDATE update ); //データに更新があれば更新間隔ごとに呼ばれる
  void (*UpdateProxyCallback)( void *p_wk_adrs, RESORT_PROXY_DATA *p_proxy ); //データに更新があれば更新間隔ごとに呼ばれる
  u32 update_interval; //更新間隔
}RECEPTION_BUFF_INIT_PARAM;

static RECEPTION_BUFF * RECEPTION_BUFF_Init( HEAPID heapID, const RECEPTION_BUFF_INIT_PARAM *cp_param );
static void RECEPTION_BUFF_Exit( RECEPTION_BUFF *p_wk );
static void RECEPTION_BUFF_Main( RECEPTION_BUFF *p_wk );

static RESORT_RECV_BUFF * RECEPTION_BUFF_GetRecvSvBuff( RECEPTION_BUFF *p_wk ); 
static RESORT_RECV_BUFF * RECEPTION_BUFF_GetRecvTmpBuff( RECEPTION_BUFF *p_wk ); 
static RESORT_PROXY_BUFF * RECEPTION_BUFF_GetProxyBuff( RECEPTION_BUFF *p_wk );

//=============================================================================
/**
 *          人物管理
 */
//=============================================================================
//-------------------------------------
///	人物管理
//=====================================
typedef struct _RECEPTION_PEOPLE RECEPTION_PEOPLE;

static RECEPTION_PEOPLE * RECEPTION_PEOPLE_Init( HEAPID heapID, FIELDMAP_WORK *p_fieldmap );
static void RECEPTION_PEOPLE_Exit( RECEPTION_PEOPLE *p_wk );
static void RECEPTION_PEOPLE_Main( RECEPTION_PEOPLE *p_wk, FIELD_PLAYER *p_player );
static RESORT_PEOPLE_SYS * RECEPTION_PEOPLE_GetPeopleSys( RECEPTION_PEOPLE *p_wk );
static RESORT_DATA_MANAGER * RECEPTION_PEOPLE_GetDataManager( RECEPTION_PEOPLE *p_wk );
static void RECEPTION_PEOPLE_Action( RECEPTION_PEOPLE *p_wk, RESORT_PEOPLE *p_people, u16 param );
//=============================================================================
/**
 *          迎賓館
 */
//=============================================================================
//-------------------------------------
///	迎賓館メインワーク
//=====================================
typedef struct
{
  RECEPTION_PEOPLE *p_people_sys;
  RESORT_BINARY_SHOP *p_binary_shop;
  RESORT_BINARY_SPOT *p_binary_spot;
  RESORT_BINARY_CITY *p_binary_city;
} RESORT_RECEPTION_WORK;

//-------------------------------------
///	PRIVATE
//=====================================

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミック初期化
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_RECEPTION_Setup( FIELDMAP_WORK *p_fieldmap )
{
  const HEAPID heapID = FIELDMAP_GetHeapID(p_fieldmap);

  //汎用ワーク確保
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID, heapID, sizeof(RESORT_RECEPTION_WORK));

  NAGI_Printf( "迎賓館setup\n" );

  {
    p_wk->p_people_sys = RECEPTION_PEOPLE_Init( heapID, p_fieldmap );
  }

  //バイナリデータ
  {
    p_wk->p_binary_shop = RESORT_BINARY_SHOP_Init( heapID );
    p_wk->p_binary_spot = RESORT_BINARY_SPOT_Init( heapID );
    p_wk->p_binary_city = RESORT_BINARY_CITY_Init( heapID );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミック破棄
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_RECEPTION_End( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);

  RESORT_BINARY_CITY_Exit( p_wk->p_binary_city );
  RESORT_BINARY_SPOT_Exit( p_wk->p_binary_spot );
  RESORT_BINARY_SHOP_Exit( p_wk->p_binary_shop );
  RECEPTION_PEOPLE_Exit( p_wk->p_people_sys );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
}
//----------------------------------------------------------------------------
/**
 *	@brief  レゾナンスリゾートギミックメイン処理
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 */
//-----------------------------------------------------------------------------
void RESORT_RECEPTION_Move( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);

  RECEPTION_PEOPLE_Main( p_wk->p_people_sys, FIELDMAP_GetFieldPlayer(p_fieldmap) );
}
//=============================================================================
/**
 *         スクリプトで使用 
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief 行動を設定
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *	@param	param　RESORT_ACTION_xxxx
 */
//-----------------------------------------------------------------------------
void RESORT_RECEPTION_Action( FIELDMAP_WORK *p_fieldmap, u16 param )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  RESORT_PEOPLE_SYS *p_people_sys = RECEPTION_PEOPLE_GetPeopleSys( p_wk->p_people_sys );
  RESORT_PEOPLE *p_people = NULL;

  s16 gx, gy, gz;
  FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( p_fieldmap );
  const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
  MMDL* p_frontmmdl;

  // 目の前のグリッド取得
  FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );

  // 目の前にいる人物を検索
  p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

  p_people = RESORT_PEOPLE_SYS_GetPeopleByMmdl( p_people_sys, p_frontmmdl );

  RECEPTION_PEOPLE_Action( p_wk->p_people_sys, p_people, param );
}
//----------------------------------------------------------------------------
/**
 *	@brief  目の前の人物を取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールドマップ
 *
 *	@return 人物
 */
//-----------------------------------------------------------------------------
RESORT_PEOPLE_SYS * RESORT_RECEPTION_GetPeopleSys( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  return RECEPTION_PEOPLE_GetPeopleSys( p_wk->p_people_sys );
}
//----------------------------------------------------------------------------
/**
 *	@brief  データマネージャーを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールドマップ
 *
 *	@return データマネージャーを取得
 */
//-----------------------------------------------------------------------------
RESORT_DATA_MANAGER * RESORT_RECEPTION_GetDataManager( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  return RECEPTION_PEOPLE_GetDataManager( p_wk->p_people_sys );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ショップバイナリデータを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールドマップ
 *
 *	@return ショップバイナリデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_SHOP * RESORT_RECEPTION_GetBinaryShop( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  return p_wk->p_binary_shop;
}
//----------------------------------------------------------------------------
/**
 *	@brief  アピールバイナリデータを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールドマップ
 *
 *	@return アピールバイナリデータ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_SPOT * RESORT_RECEPTION_GetBinarySpot( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  return p_wk->p_binary_spot;
}

//----------------------------------------------------------------------------
/**
 *	@brief  街情報データ取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールドマップ
 *
 *	@return 街情報データ
 */
//-----------------------------------------------------------------------------
RESORT_BINARY_CITY * RESORT_RECEPTION_GetBinaryCity( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_RECEPTION_WORK *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, RESORT_RECEPTION_ASSIGN_ID);
  return p_wk->p_binary_city;
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
struct _RECEPTION_BUFF
{
  RECEPTION_BUFF_INIT_PARAM param;
  RESORT_RECV_BUFF  *p_sv_buff;     //セーブにあるバッファ
  RESORT_RECV_BUFF  *p_recep_buff;     //迎賓館にのみ存在するバッファ
  RESORT_PROXY_BUFF *p_proxy_buff;  //セーブにある代理人バッファ
  u32               update_frame;   //更新間隔カウンタ
  BOOL              is_proxy_update; //一度しか更新がかからないフラグ
};

//-------------------------------------
///	PRIVATE
//=====================================
static void Reception_Buff_UpdateRecv( RECEPTION_BUFF *p_wk );
static void Reception_Buff_UpdateProxy( RECEPTION_BUFF *p_wk );

//-------------------------------------
///	PUBLIC
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  バッファ管理作成
 *
 *	@param	HEAPID heapID ヒープID
 *	@param	RECEPTION_BUFF_INIT_PARAM *cp_param   引数
 *
 *	@return　バッファ管理
 */
//-----------------------------------------------------------------------------
static RECEPTION_BUFF * RECEPTION_BUFF_Init( HEAPID heapID, const RECEPTION_BUFF_INIT_PARAM *cp_param )
{
  RECEPTION_BUFF *p_wk = GFL_HEAP_AllocClearMemory(heapID, sizeof(RECEPTION_BUFF));
  p_wk->param = *cp_param;

  //セーブからバッファを取得
  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( cp_param->p_gamedata );
    RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );
    p_wk->p_sv_buff     = RESONANCE_RESORT_SV_GetDataBuff( p_resort_sv );
    p_wk->p_proxy_buff  = RESONANCE_RESORT_SV_GetProxyBuff( p_resort_sv );
  }

  //迎賓館のみ存在する拡張バッファを登録
  {
    void **pp_buff = GAMEDATA_GetResortBuffer( cp_param->p_gamedata );
    GF_ASSERT( *pp_buff != NULL );
    p_wk->p_recep_buff  = *pp_buff;
    GAMEBEACON_Set_ResortRecvBuffer( p_wk->p_recep_buff );
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バッファ管理破棄
 *
 *	@param	RECEPTION_BUFF *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RECEPTION_BUFF_Exit( RECEPTION_BUFF *p_wk )
{
  GAMEBEACON_Set_ResortRecvBuffer( NULL );
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バッファ管理メイン
 *
 *	@param	RECEPTION_BUFF *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RECEPTION_BUFF_Main( RECEPTION_BUFF *p_wk )
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
 *	@brief  バッファ管理　バッファ取得
 *
 *	@param	RECEPTION_BUFF *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static RESORT_RECV_BUFF * RECEPTION_BUFF_GetRecvSvBuff( RECEPTION_BUFF *p_wk )
{
  return p_wk->p_sv_buff;
}
static RESORT_RECV_BUFF * RECEPTION_BUFF_GetRecvTmpBuff( RECEPTION_BUFF *p_wk )
{
  return p_wk->p_recep_buff;
}
static RESORT_PROXY_BUFF * RECEPTION_BUFF_GetProxyBuff( RECEPTION_BUFF *p_wk )
{
  return p_wk->p_proxy_buff;
}
//----------------------------------------------------------------------------
/**
 *	@brief  受信バッファのアップデート
 *
 *	@param	RECEPTION_BUFF *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Reception_Buff_UpdateRecv( RECEPTION_BUFF *p_wk )
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
 *	@param	RECEPTION_BUFF *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Reception_Buff_UpdateProxy( RECEPTION_BUFF *p_wk )
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
#define RECEPTION_PEOPLE_MAX (24)
//更新間隔
#define RECEPTION_PEOPLE_UPDATE_INTERVAL (0)

//-------------------------------------
///	人物が動作するときのワーク
//=====================================
typedef struct
{
  RESORT_PEOPLE *p_people;
  u32 seq;
} RECEPTION_PEOPLE_ACT;

static void Reception_People_CreateEnterTask( GFL_TCBLSYS * p_tcblsys, RESORT_PEOPLE *p_people );
static void Reception_People_CreateLeaveTask( GFL_TCBLSYS * p_tcblsys, RESORT_PEOPLE *p_people );
static void Reception_People_CreateReplaceTask( GFL_TCBLSYS * p_tcblsys, RESORT_PEOPLE *p_people );
static void Reception_People_Act_Enter( GFL_TCBL *p_tcbl, void *p_wk_adrs );
static void Reception_People_Act_Leave( GFL_TCBL *p_tcbl, void *p_wk_adrs );
static void Reception_People_Act_Replace( GFL_TCBL *p_tcbl, void *p_wk_adrs );

//-------------------------------------
///	人物管理
//=====================================
struct _RECEPTION_PEOPLE
{
  RESORT_PEOPLE_SYS *p_people_sys;
  RECEPTION_BUFF    *p_buff;
  GFL_TCBLSYS       *p_tcblsys;
  RESORT_DATA_MANAGER *p_data_manager;
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
 *	@param  fieldmap     フィールド	
 *
 *	@return 人物管理ワーク
 */
//-----------------------------------------------------------------------------
static RECEPTION_PEOPLE * RECEPTION_PEOPLE_Init( HEAPID heapID, FIELDMAP_WORK *p_fieldmap )
{
  RECEPTION_PEOPLE *p_wk = GFL_HEAP_AllocClearMemory(heapID,sizeof(RECEPTION_PEOPLE));

  GAMEDATA *p_gamedata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork(p_fieldmap) );

  //TCBL
  p_wk->p_tcblsys = GFL_TCBL_Init( heapID, heapID, RECEPTION_PEOPLE_MAX, sizeof(RECEPTION_PEOPLE_ACT) );

  //バッファ管理
  {
    RECEPTION_BUFF_INIT_PARAM param;
    GFL_STD_MemClear( &param, sizeof( RECEPTION_BUFF_INIT_PARAM ) );
    param.p_gamedata = p_gamedata;
    param.p_wk_adrs = p_wk;
    param.UpdateRecvCallback = Reception_People_UpdateRecvCallback;
    param.UpdateProxyCallback = Reception_People_UpdateProxyCallback;
    param.update_interval = RECEPTION_PEOPLE_UPDATE_INTERVAL;
    p_wk->p_buff = RECEPTION_BUFF_Init( heapID, &param );
  }

  //データマネージャー
  {
    SAVE_CONTROL_WORK *p_sv = GAMEDATA_GetSaveControlWork( p_gamedata );
    RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( p_sv );

    RESORT_DATA_MANAGER_INIT_PARAM param;
    GFL_STD_MemClear( &param, sizeof(RESORT_DATA_MANAGER_INIT_PARAM) );
    param.p_all_people    = RESONANCE_RESORT_SV_GetPeopleAllData(p_resort_sv);
    param.p_recv_sv_buff  = RECEPTION_BUFF_GetRecvSvBuff( p_wk->p_buff );
    param.p_recv_tmp_buff = RECEPTION_BUFF_GetRecvTmpBuff( p_wk->p_buff );
    param.p_proxy_buff    = RECEPTION_BUFF_GetProxyBuff( p_wk->p_buff );
    param.p_city_data     = RESONANCE_RESORT_SV_GetCityData( p_resort_sv );
    
    p_wk->p_data_manager = RESORT_DATA_MANAGER_Init( &param, heapID );
  }

  //人物システム
  {

    RESORT_PEOPLE_SYS_PARAM param;
    GFL_STD_MemClear( &param, sizeof(RESORT_PEOPLE_SYS_PARAM) );
    param.p_mmdlsys = GAMEDATA_GetMMdlSys(p_gamedata);
    param.p_data_manager = p_wk->p_data_manager;
    param.max = RECEPTION_PEOPLE_MAX;
    param.placeID = RESORT_PLACEID_RECEPTION;
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
 *	@param	RECEPTION_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RECEPTION_PEOPLE_Exit( RECEPTION_PEOPLE *p_wk )
{
  RESORT_PEOPLE_SYS_Exit( p_wk->p_people_sys );
  RESORT_DATA_MANAGER_Exit( p_wk->p_data_manager );
  RECEPTION_BUFF_Exit( p_wk->p_buff );
  GFL_TCBL_DeleteAll( p_wk->p_tcblsys );
  GFL_TCBL_Exit( p_wk->p_tcblsys );
  GFL_HEAP_FreeMemory(p_wk);
}
//----------------------------------------------------------------------------
/**
 *	@brief  人物管理ワーク破棄
 *
 *	@param	RECEPTION_PEOPLE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void RECEPTION_PEOPLE_Main( RECEPTION_PEOPLE *p_wk, FIELD_PLAYER *p_player )
{
  RECEPTION_BUFF_Main( p_wk->p_buff );
  GFL_TCBL_Main( p_wk->p_tcblsys );
  RESORT_PEOPLE_SYS_Main( p_wk->p_people_sys, p_player );
}
//----------------------------------------------------------------------------
/**
 *	@brief　人物システムを取得
 *
 *	@param	RECEPTION_PEOPLE *p_wk ワーク
 *
 *	@return 人物システム
 */
//-----------------------------------------------------------------------------
static RESORT_PEOPLE_SYS * RECEPTION_PEOPLE_GetPeopleSys( RECEPTION_PEOPLE *p_wk )
{
  return p_wk->p_people_sys;
}
//----------------------------------------------------------------------------
/**
 *	@brief  データマネージャーを取得
 *
 *	@param	RECEPTION_PEOPLE *p_wk  ワーク
 *
 *	@return データマネージャー
 */
//-----------------------------------------------------------------------------
static RESORT_DATA_MANAGER * RECEPTION_PEOPLE_GetDataManager( RECEPTION_PEOPLE *p_wk )
{
  return p_wk->p_data_manager;
}
//----------------------------------------------------------------------------
/**
 *	@brief 　人物の行動
 *
 *	@param	RECEPTION_PEOPLE *p_wk　ワーク
 *	@param	*p_people 行動する人物
 *	@param	param     行動状態
 */
//-----------------------------------------------------------------------------
static void RECEPTION_PEOPLE_Action( RECEPTION_PEOPLE *p_wk, RESORT_PEOPLE *p_people, u16 param )
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
  RECEPTION_PEOPLE *p_wk = p_wk_adrs;
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
    Reception_People_CreateReplaceTask( p_wk->p_tcblsys, p_people );
  }
  else
  {
    //いないなら人物作成
    RESORT_PEOPLE_SYS_ADD_PARAM param;
    GFL_STD_MemClear(&param,sizeof(RESORT_PEOPLE_SYS_ADD_PARAM));

    param.people_type = RESORT_PEOPLE_TYPE_TOURIST;
    param.p_data_adrs = p_recv;
    param.placeID      = RESORT_PLACEID_RECEPTION;

    p_people = RESORT_PEOPLE_SYS_AddNew( p_wk->p_people_sys, &param );
    RESORT_PEOPLE_SetVisible( p_people, FALSE );

    //人物作成したら、上から落下してくるタスクを登録
    Reception_People_CreateEnterTask( p_wk->p_tcblsys, p_people );
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
  RECEPTION_PEOPLE *p_wk = p_wk_adrs;
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
    param.people_type = RESORT_PEOPLE_TYPE_PROXY;
    param.p_data_adrs = p_proxy;
    param.placeID      = RESORT_PLACEID_RECEPTION;
    p_people = RESORT_PEOPLE_SYS_AddNew( p_wk->p_people_sys, &param );

    //人物作成したら、上から落下してくるタスクを登録
    Reception_People_CreateEnterTask( p_wk->p_tcblsys, p_people );
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
  GFL_TCBL * p_tcbl = GFL_TCBL_Create(p_tcblsys, Reception_People_Act_Enter, sizeof(RECEPTION_PEOPLE_ACT), 0);
  RECEPTION_PEOPLE_ACT *p_act = GFL_TCBL_GetWork(p_tcbl);
  GFL_STD_MemClear( p_act, sizeof(RECEPTION_PEOPLE_ACT) );

  p_act->p_people = p_people;
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

  RECEPTION_PEOPLE_ACT *p_wk = p_wk_adrs;

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
  GFL_TCBL * p_tcbl = GFL_TCBL_Create(p_tcblsys, Reception_People_Act_Leave, sizeof(RECEPTION_PEOPLE_ACT), 0);
  RECEPTION_PEOPLE_ACT *p_act = GFL_TCBL_GetWork(p_tcbl);
  GFL_STD_MemClear( p_act, sizeof(RECEPTION_PEOPLE_ACT) );

  p_act->p_people = p_people;
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

  RECEPTION_PEOPLE_ACT *p_wk = p_wk_adrs;

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
  GFL_TCBL * p_tcbl = GFL_TCBL_Create(p_tcblsys, Reception_People_Act_Replace, sizeof(RECEPTION_PEOPLE_ACT), 0);
  RECEPTION_PEOPLE_ACT *p_act = GFL_TCBL_GetWork(p_tcbl);
  GFL_STD_MemClear( p_act, sizeof(RECEPTION_PEOPLE_ACT) );

  p_act->p_people = p_people;
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

  RECEPTION_PEOPLE_ACT *p_wk = p_wk_adrs;

  switch( p_wk->seq )
  {
  case SEQ_REAVE_START:
    RESORT_PEOPLE_ActionReq( p_wk->p_people, RESORT_PEOPLE_ACTION_WARPUP );
    p_wk->seq++;
    break;
  case SEQ_REAVE_WAIT:
    if( RESORT_PEOPLE_IsActionEnd( p_wk->p_people ) )
    {
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
    GFL_TCBL_Delete(p_tcbl);
    break;
  default:
    GF_ASSERT(0);
  }
}
