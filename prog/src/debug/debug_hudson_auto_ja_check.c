//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debug_hudson_auto_ja_check.c
 *	@brief  JA店舗生成オートデバッグ
 *	@author	Toru=Nagihashi
 *	@date		2011.12.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "system/main.h"
#include "field/fieldmap_proc.h"
#include "field/fieldmap.h"
#include "field/field_const.h"
#include "field/event_mapchange.h"
#include "debug/debug_ui.h"
#include "field/eventwork_def.h"
#include "debug/debug_hudson_auto_ja_check.h"


#ifdef DEBUG_ONLY_FOR_hudson
#ifdef PM_DEBUG

//リゾート
#include "arc/fieldmap/zone_id.h"
#include "savedata/resonance_resort_sv.h"
#include "field/resonance_resort/resort_debug_inline.h"
#include "field/resonance_resort/resort_map.h"
#include "field/resonance_resort/resort_shop.h"
#include "field/resonance_resort/resort_binary.h"
#include "field/resonance_resort/resort_shop.h"
#include "field/resonance_resort/resort_debug.h"


extern void RESORT_RESIDENT_DATA_ConvertRecv( RESORT_RESIDENT_DATA *p_wk, const RESORT_RECV_DATA *cp_recv, const RESORT_BINARY_SHOP *cp_shop, u16 peopleID, u16 placeID );

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
typedef enum
{
  SHOP_VERSION_ID_B,
  SHOP_VERSION_ID_W,
  SHOP_VERSION_ID_B2,
  SHOP_VERSION_ID_W2,
  SHOP_VERSION_ID_MAX
}SHOP_VERSION_ID;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///チェックワーク	
//=====================================
typedef struct
{
  GAMESYS_WORK  *p_gamesys;

  u32 seq;
  SHOP_VERSION_ID version_id;
} RESORT_SHOP_CHECK_WK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void RESORT_SHOP_CHECK_Init( void *p_wk_adrs, GAMESYS_WORK* p_gamesys, HEAPID heapID );
static void RESORT_SHOP_CHECK_Update( GFL_TCBL *p_task, void *p_wk_adrs );
static u32 RESORT_SHOP_CHECK_GetWorkSize( void );

static void ResortShopCheck_InitCommon( GAMEDATA *p_gamedata, HEAPID heapID );
static void ResortShopCheck_InitResort( GAMEDATA *p_gamedata, HEAPID heapID );
static void ResortShopCheck_CreateShop( GAMEDATA *p_gamedata, SHOP_VERSION_ID version_id, HEAPID heapID );

static void ResortShopCheck_SetRecvAll( GAMEDATA *p_gamedata, HEAPID heapID );
static BOOL ResortShopCheck_CheckShopAllRankMax( GAMEDATA *p_gamedata );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
const HUDSON_AUTOTEST_WORK HUDSON_AUTOTEST_ResortShopCheck =
{
  RESORT_SHOP_CHECK_Init,
  RESORT_SHOP_CHECK_Update,
  RESORT_SHOP_CHECK_GetWorkSize,
};

//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *	@param	void *p_wk_adrs ワークアドレス
 *	@param	p_gamesys ゲームデータ
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void RESORT_SHOP_CHECK_Init( void *p_wk_adrs, GAMESYS_WORK* p_gamesys, HEAPID heapID )
{
  RESORT_SHOP_CHECK_WK  *p_wk = p_wk_adrs;
  GFL_STD_MemClear( p_wk, sizeof(RESORT_SHOP_CHECK_WK) );
  p_wk->p_gamesys = p_gamesys;
}

//----------------------------------------------------------------------------
/**
 *	@brief  更新
 *
 *	@param	GFL_TCBL *p_task  タスク
 *	@param	*p_wk_adrs ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_SHOP_CHECK_Update( GFL_TCBL *p_task, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_INIT_JA_SHOP,
    SEQ_JUMP_JA_START,
    SEQ_JUMP_JA_WAIT,

    SEQ_BEACON_FULL,
    SEQ_OBJ_CHECK,
    SEQ_GUIDE_START,
    SEQ_GUIDE_WAIT,
    SEQ_GUIDE_EVENT_WAIT,
    SEQ_CHECK_SHOP,

    SEQ_END,
  };

  RESORT_SHOP_CHECK_WK  *p_wk = p_wk_adrs;
  GAMESYS_WORK  *p_gamesys  = p_wk->p_gamesys;
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  FIELDMAP_WORK *p_field    = GAMESYSTEM_GetFieldMapWork( p_gamesys );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    ResortShopCheck_InitCommon( p_gamedata, HEAPID_PROC );
    ResortShopCheck_InitResort( p_gamedata, HEAPID_PROC );
    p_wk->seq = SEQ_INIT_JA_SHOP;
    break;

  case SEQ_INIT_JA_SHOP:
    ResortShopCheck_CreateShop( p_gamedata, p_wk->version_id, HEAPID_PROC );
    p_wk->seq = SEQ_JUMP_JA_START;
    break;

  case SEQ_JUMP_JA_START:
    if( !GAMESYSTEM_IsEventExists(p_gamesys) )
    {
      VecFx32 appear_pos;
      GMEVENT* p_event;

      VEC_Set(&appear_pos,
          GRID_TO_FX32(14),
          GRID_TO_FX32(0),
          GRID_TO_FX32(73)
          );     
      
      p_event = EVENT_ChangeMapPos( 
        p_gamesys, p_field, ZONE_ID_C16, &appear_pos, DIR_DOWN, FALSE );
      GAMESYSTEM_SetEvent( p_gamesys, p_event );
      
      p_wk->seq = SEQ_JUMP_JA_WAIT;
    }
    break;

  case SEQ_JUMP_JA_WAIT:
    if( !GAMESYSTEM_IsEventExists(p_gamesys) )
    {
      p_wk->seq = SEQ_BEACON_FULL;
    }
    break;

  case SEQ_BEACON_FULL:
    ResortShopCheck_SetRecvAll( p_gamedata, HEAPID_PROC );
    p_wk->seq = SEQ_OBJ_CHECK;
    break;

  case SEQ_OBJ_CHECK:
    if( FIELD_PLAYER_GetFrontMMdl( FIELDMAP_GetFieldPlayer(p_field) ) != NULL )
    {
      p_wk->seq = SEQ_GUIDE_START;
    }
    break;

  case SEQ_GUIDE_START:
    DEBUG_UI_SetUp( DEBUG_UI_AUTO_RESORT_GUIDE, DEBUG_UI_PLAY_ONE );
    p_wk->seq = SEQ_GUIDE_WAIT;
    break;

  case SEQ_GUIDE_WAIT:
    if( !DEBUG_UI_IsUpdate() && DEBUG_UI_AUTO_RESORT_GUIDE == DEBUG_UI_GetType() )
    {
      DEBUG_UI_SetUp( DEBUG_UI_AUTO_A, DEBUG_UI_PLAY_LOOP );
      p_wk->seq = SEQ_GUIDE_EVENT_WAIT;
    }
    break;
    
  case SEQ_GUIDE_EVENT_WAIT:
    if( !GAMESYSTEM_IsEventExists(p_gamesys) )
    {
      DEBUG_UI_SetUp( DEBUG_UI_NONE, DEBUG_UI_PLAY_ONE );
      p_wk->seq = SEQ_CHECK_SHOP;
    }
    break;

  case SEQ_CHECK_SHOP:
    //全お店１０ならば
    if(ResortShopCheck_CheckShopAllRankMax(p_gamedata) )
    {
      p_wk->version_id++;
      //バージョン全部やった？
      if( p_wk->version_id < SHOP_VERSION_ID_MAX )
      {
        p_wk->seq = SEQ_INIT_JA_SHOP;
      }
      else
      {
        p_wk->seq = SEQ_END;
      }
    }
    else
    {
      //違うならばまだ案内する
      p_wk->seq = SEQ_BEACON_FULL;
    }
    break;

  case SEQ_END:
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  タスクワークサイズ取得関数
 *	@return タスクワークサイズ
 */
//-----------------------------------------------------------------------------
static u32 RESORT_SHOP_CHECK_GetWorkSize( void )
{
  return sizeof( RESORT_SHOP_CHECK_WK );
}

//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *	@param	GAMEDATA *p_gamedata  ゲームデータ
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void ResortShopCheck_InitCommon( GAMEDATA *p_gamedata, HEAPID heapID )
{
  SAVE_CONTROL_WORK* p_save = GAMEDATA_GetSaveControlWork(p_gamedata);
  {
    CONFIG *p_config = SaveData_GetConfig( p_save );
    CONFIG_SetMsgSpeed( p_config, MSGSPEED_FAST );
    CONFIG_SetMojiMode( p_config, MOJIMODE_KANJI );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  リゾートの初期化を行う
 *
 *	@param	GAMEDATA *p_gamedata  ゲームデータ
 *	@param	heapID ヒープID
 */
//-----------------------------------------------------------------------------
static void ResortShopCheck_InitResort( GAMEDATA *p_gamedata, HEAPID heapID )
{
  SAVE_CONTROL_WORK* p_save = GAMEDATA_GetSaveControlWork(p_gamedata);
  EVENTWORK *p_ev = SaveData_GetEventWork( p_save );
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( p_save );
  RESORT_CITY_DATA *p_city =  RESONANCE_RESORT_SV_GetCityData(p_resort);
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort );


  //チュートリアル終了
  {
    u16 *p_01_wk = EVENTWORK_GetEventWorkAdrs( p_ev, WK_SCENE_C16_TUTORIAL_01 );
    u16 *p_seq_wk = EVENTWORK_GetEventWorkAdrs( p_ev, WK_SCENE_C16_TUTORIAL_SEQ );

    *p_01_wk = 1;
    *p_seq_wk = 6;

    RESORT_CITY_DATA_SetData( p_city, RESORT_CITY_DATA_PARAM_IS_TUTORIAL, FALSE );

    //秘書２人追加
    {
      int i;

      RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(heapID) );

      for( i = 0; i < 2; i++ )
      {
        RESORT_SECRETARY_DATA *p_dst = RESORT_PEOPLE_ALL_DATA_GetSecretary( p_all_people, i );
        if( RESORT_SECRETARY_DATA_IsEmpty( p_dst ) )
        {
          Debug_Resort_SetRecv( p_recv, p_gamedata );
          RESORT_SECRETARY_DATA_ConvertRecv( p_dst, p_recv, 8+i, RESORT_MAP_GetPeoplePlace( RESORT_PEOPLE_TYPE_SECRETARY ), i );
        }
      }
      RESORT_RECV_DATA_Exit( p_recv );
    }
  }

  //デバッグフラグ
  {
    RESORT_DEBUG_FLAG_Set( RESORT_DEBUG_FLAG_FORCE_EXCELLENT, TRUE );
    //RESORT_DEBUG_FLAG_Set( RESORT_DEBUG_FLAG_EXP_9999, TRUE );
    RESORT_DEBUG_FLAG_Set( RESORT_DEBUG_FLAG_FORCE_INIT_COMBO, TRUE );
    RESORT_DEBUG_FLAG_Set( RESORT_DEBUG_FLAG_FORCE_COMBO_MAX, TRUE );
    RESORT_DEBUG_FLAG_Set( RESORT_DEBUG_FLAG_PRESENT_NONE, TRUE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  リゾートのお店レベル１を作成する
 *
 *	@param	GAMEDATA *p_gamedata  ゲームデータ
 *	@param	version バージョン
 *	@param	heapID  テンポラリ用ヒープID
 */
//-----------------------------------------------------------------------------
static void ResortShopCheck_CreateShop( GAMEDATA *p_gamedata, SHOP_VERSION_ID version_id, HEAPID heapID )
{
  static const int sc_shop_tbl[][RESORT_RESIDENT_DATA_NUM] =
  {
    //B
    {
      RESORT_SHOP_ID_LOT_00_B,
      RESORT_SHOP_ID_PARLOR_00_B,
      RESORT_SHOP_ID_MARKET_00_B,
      RESORT_SHOP_ID_FLORIST_00_B,
      RESORT_SHOP_ID_EXERCICE_00_B,
      RESORT_SHOP_ID_NURSERY_00_B,
      RESORT_SHOP_ID_ANTIQUE_00_B,
      RESORT_SHOP_ID_CAFE_00_B,
    },
    //W
    {
      RESORT_SHOP_ID_LOT_00_W,
      RESORT_SHOP_ID_PARLOR_00_W,
      RESORT_SHOP_ID_MARKET_00_W,
      RESORT_SHOP_ID_FLORIST_00_W,
      RESORT_SHOP_ID_EXERCICE_00_W,
      RESORT_SHOP_ID_NURSERY_00_W,
      RESORT_SHOP_ID_ANTIQUE_00_W,
      RESORT_SHOP_ID_CAFE_00_W,
    },
    //B2
    {
      RESORT_SHOP_ID_LOT_00_B2,
      RESORT_SHOP_ID_PARLOR_00_B2,
      RESORT_SHOP_ID_MARKET_00_B2,
      RESORT_SHOP_ID_FLORIST_00_B2,
      RESORT_SHOP_ID_EXERCICE_00_B2,
      RESORT_SHOP_ID_NURSERY_00_B2,
      RESORT_SHOP_ID_ANTIQUE_00_B2,
      RESORT_SHOP_ID_CAFE_00_B2,
    },
    //W2
    {
      RESORT_SHOP_ID_LOT_00_W2,
      RESORT_SHOP_ID_PARLOR_00_W2,
      RESORT_SHOP_ID_MARKET_00_W2,
      RESORT_SHOP_ID_FLORIST_00_W2,
      RESORT_SHOP_ID_EXERCICE_00_W2,
      RESORT_SHOP_ID_NURSERY_00_W2,
      RESORT_SHOP_ID_ANTIQUE_00_W2,
      RESORT_SHOP_ID_CAFE_00_W2,
    },
  };

  SAVE_CONTROL_WORK* p_save = GAMEDATA_GetSaveControlWork(p_gamedata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( p_save );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort );
  int i;

  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(heapID) );
  RESORT_BINARY_SHOP *p_shop =  RESORT_BINARY_SHOP_Init( GFL_HEAP_LOWID(heapID) );

  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_dst = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people, i );
    {
      Debug_Resort_SetRecv( p_recv, p_gamedata );
  
      RESORT_RECV_DATA_SetData( p_recv, RESORT_DATA_PARAM_WANT_JOB, sc_shop_tbl[version_id][i] );

      RESORT_RESIDENT_DATA_ConvertRecv( p_dst, p_recv, p_shop, 63-i, RESORT_PLACEID_FIELD );
    }
  }
  RESORT_BINARY_SHOP_Exit(p_shop);

  RESORT_RECV_DATA_Exit( p_recv );
}

//----------------------------------------------------------------------------
/**
 *	@brief  受信データを最大にする
 *
 *	@param	GAMEDATA *p_gamedata  ゲームデータ
 *	@param	heapID テンポラリ用ヒープID
 */
//-----------------------------------------------------------------------------
static void ResortShopCheck_SetRecvAll( GAMEDATA *p_gamedata, HEAPID heapID )
{
  SAVE_CONTROL_WORK* p_save = GAMEDATA_GetSaveControlWork(p_gamedata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( p_save );
  RESORT_RECV_BUFF *p_recv_buff = RESONANCE_RESORT_SV_GetDataBuff( p_resort );
  RESORT_RECV_BUFF *p_recv_tmp_buff = GAMEBEACON_DEBUG_GetResortRecvBuffer();

  RESORT_RECV_DATA *p_recv = RESORT_RECV_DATA_Init( GFL_HEAP_LOWID(heapID) );
  while( TRUE )
  {
    Debug_Resort_SetRecv( p_recv, p_gamedata );
    RESONANCE_RESORT_SV_SetData( p_resort, p_gamedata, p_recv, RESORT_MERGE_TYPE_RECV );

    if( p_recv_tmp_buff )
    {
      if( RESORT_RECV_BUFF_GetCnt(p_recv_buff) >= RESONANCE_RESORT_SV_RECV_BUFF_NUM
        && RESORT_RECV_BUFF_GetCnt(p_recv_tmp_buff) >= RESONANCE_RESORT_BUFF_RECV_EX_NUM )
      {
        break;
      }
    }
    else
    {
      if( RESORT_RECV_BUFF_GetCnt(p_recv_buff) >= RESONANCE_RESORT_SV_RECV_BUFF_NUM )
      {
        break;
      }
    }
  }
  RESORT_RECV_DATA_Exit( p_recv );
}
//----------------------------------------------------------------------------
/**
 *	@brief  お店が全部ランク最大かチェック
 *
 *	@param	GAMEDATA *p_gamedata  ゲームデータ
 *
 *	@return TRUEお店が全部ランク最大  FALSE違う
 */
//-----------------------------------------------------------------------------
static BOOL ResortShopCheck_CheckShopAllRankMax( GAMEDATA *p_gamedata )
{
  SAVE_CONTROL_WORK* p_save = GAMEDATA_GetSaveControlWork(p_gamedata);
  RESONANCE_RESORT_SV *p_resort = RESONANCE_RESORT_SV_GetWork( p_save );
  RESORT_PEOPLE_ALL_DATA *p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort );
  int i;

  for( i = 0; i < RESORT_RESIDENT_DATA_NUM; i++ )
  {
    RESORT_RESIDENT_DATA *p_dst = RESORT_PEOPLE_ALL_DATA_GetResident( p_all_people, i );
    RESORT_INSTITUTION_DATA *p_inst = RESORT_RESIDENT_DATA_GetInstitution(p_dst);
    if( RESORT_INSTITUTION_DATA_GetData(p_inst, RESORT_INSTITUTION_DATA_PARAM_LV) != 9 )
    {
      return FALSE;
    }
  }

  return TRUE;
}

#endif //PM_DEBUG
#endif // DEBUG_ONLY_FOR_hudson

