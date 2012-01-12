//=============================================================================
/**
 *
 *	@file		debug_hudson.c
 *	@brief  hudson用ユーティリティ
 *	@author		hosaka genya
 *	@data		2009.11.25
 *
 */
//=============================================================================

//システム
#include <gflib.h>
#include "field/fieldmap.h"
#include "field/field_msgbg.h"
#include "field/field_subscreen.h"
#include "savedata/config.h"

//オートデバッグ用
#include "field/event_debug_menu.h" // for EVENT_DEBUG_AllMapCheck
#include "field/event_debug_all_connect_check.h" // for EVENT_DEBUG_AllConnectCheck
#include "field/event_debug_btl_all_waza_check.h" // for EVENT_DEBUG_BtlAllWazaCheck
#include "field/event_debug_btl_all_waza_check2.h" // for EVENT_DEBUG_BtlAllWazaCheck2
#include "field/event_debug_btl_all_poke_check.h" // for EVENT_DEBUG_BtlAllPokeCheck
#include "field/event_debug_btl_all_waza_cam_check.h" // for EVENT_DEBUG_BtlAllWazaCamCheck
#include "field/event_debug_btl_poke_change.h" // for EVENT_DEBUG_BtlPokeChange
#include "field/event_debug_menu_ov_jump.h" // for debugMenuCallProc_AllMapCheck
#include "debug/debug_hudson_auto_ja_check.h"
#include "debug/debug_hudson_auto_jump_zone_fes.h"
#include "debug/debug_hudson_auto_jump_zone_tick.h"


//外部公開
#include "debug/debug_hudson.h"

#ifdef PM_DEBUG
#ifdef DEBUG_ONLY_FOR_hudson

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
//タスク数
#define HUDSON_TCBLSYS_WK_NUM   (1)
#define HUDSON_TCBLSYS_WK_SIZE  (0x100)
//フラグ数
#define HUDSON_AUTOTEST_FLAG_NUM  (32)
#define HUDSON_AUTOTEST_FLAG_ARR_NUM ((HUDSON_AUTOTEST_FLAG_NUM+7)/8)

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================
//-------------------------------------
///	ハドソン用ワーク
//=====================================
typedef struct
{
  GFL_TCBLSYS *p_tcblsys;
  BOOL is_task_create;
  int seq;
  u8  flag[HUDSON_AUTOTEST_FLAG_ARR_NUM];
  FSOverlayID task_overlayID;
  GAMESYS_WORK* gsys;
  HEAPID heapID;
} HUDSON_WORK;

static HUDSON_WORK * sp_wk  = NULL;

//=============================================================================
/**
 *								prottype
 */
//=============================================================================
static void Hudson_CreateTask( HUDSON_WORK * p_wk, const HUDSON_AUTOTEST_WORK *autotest, FSOverlayID ov_id );

//=============================================================================
/**
 *								public
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ハドソン用処理メイン
 *
 *	@param	GAMESYS_WORK* gsys ゲームシステム
 */
//-----------------------------------------------------------------------------
void HUDSON_Init( GAMESYS_WORK* gsys, HEAPID heapID )
{
  GF_ASSERT( sp_wk == NULL );
  sp_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(HUDSON_WORK) );

  sp_wk->p_tcblsys  = GFL_TCBL_Init( heapID, heapID, HUDSON_TCBLSYS_WK_NUM, HUDSON_TCBLSYS_WK_SIZE );

  sp_wk->task_overlayID = GFL_OVERLAY_BLANK_ID;
  sp_wk->gsys = gsys;
  sp_wk->heapID = heapID;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハドソン用処理破棄
 *
 *	@param	GAMESYS_WORK* gsys ゲームシステム
 */
//-----------------------------------------------------------------------------
void HUDSON_Exit( GAMESYS_WORK* gsys )
{
  GF_ASSERT( sp_wk != NULL );

  GFL_TCBL_DeleteAll( sp_wk->p_tcblsys );

  if( sp_wk->task_overlayID != GFL_OVERLAY_BLANK_ID )
  {
    GFL_OVERLAY_Unload( sp_wk->task_overlayID );
  }

  GFL_TCBL_Exit( sp_wk->p_tcblsys );

  GFL_HEAP_FreeMemory( sp_wk );
  sp_wk = NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハドソン用処理メイン
 *
 *	@param	GAMESYS_WORK* gsys ゲームシステム
 */
//-----------------------------------------------------------------------------
void HUDSON_Main( GAMESYS_WORK* gsys )
{
  GF_ASSERT( sp_wk != NULL );
  
  GFL_TCBL_Main( sp_wk->p_tcblsys );

#ifdef DEBUG_HUDSON_INPUT_AUTO_TEST
  //JAチェックのため、任意で発動できるようにしている。
  if( GFL_UI_KEY_GetTrg( ) & PAD_BUTTON_B
      && GFL_UI_KEY_GetCont( ) & PAD_BUTTON_L )
  {
    if( !sp_wk->is_task_create )
    {
      Hudson_CreateTask( sp_wk, &HUDSON_AUTOTEST_JumpTickCheck, FS_OVERLAY_ID( debug_jump_tick_check ) );
    }
  }
#else

  if( OS_GetArgc() < 2 )
  {
    return;
  }

  switch( sp_wk->seq )
  {
  default:
    sp_wk->seq++;
    break;
  case 180:
    {
      // すれ違い画面に切り替え
      FIELDMAP_WORK * fwork = GAMESYSTEM_GetFieldMapWork(gsys);
      FIELD_SUBSCREEN_WORK* subscreen = FIELDMAP_GetFieldSubscreenWork(fwork);
      FIELD_SUBSCREEN_ChangeForce( subscreen, FIELD_SUBSCREEN_BEACON_VIEW );
      OS_Printf("デバッグ：すれ違いログ画面\n");
      sp_wk->seq++;
    }
    break;
  case 781:
    break;
  case 780:
    {
      GMEVENT * new_event;

      sp_wk->seq++;

      //>>引数からテストモード切り替え

      // 全マップチェック
#if 1
      /*    デバグメニューをオーバーレイ化したので、
       *    ひとまずコメントアウトしています。
       *    下記関数を使うためにはオーバーレイを読み込む必要があります
       */
      if ( HUDSON_IsTestCode( HUDSON_TESTCODE_MAP_JUMP ) )
      {
        new_event = GMEVENT_CreateOverlayEventCall( gsys, 
            FS_OVERLAY_ID( debugmenu_jump ), EVENT_DEBUG_AllMapCheck, NULL );
        GAMESYSTEM_SetEvent( gsys, new_event );
        OS_Printf("デバッグ：全マップチェック開始\n");
      }
      // 全接続チェック
      else
#endif
        if ( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_CONNECT ) )
        {
          new_event = GMEVENT_CreateOverlayEventCall( gsys, 
              FS_OVERLAY_ID( debug_connect_check ), EVENT_DEBUG_AllConnectCheck, NULL );
          GAMESYSTEM_SetEvent( gsys, new_event );
        }
      // 全技チェック
        else if( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA ) )
        {
#if 0      
          {
            // 技エフェクトOFF
            GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
            SAVE_CONTROL_WORK* sv = GAMEDATA_GetSaveControlWork( gdata );
            CONFIG* cfg = SaveData_GetConfig( sv ); 
            CONFIG_SetWazaEffectMode( cfg, WAZAEFF_MODE_OFF );
          }
#endif
          new_event = GMEVENT_CreateOverlayEventCall( gsys, 
              FS_OVERLAY_ID( debug_all_waza_check ), EVENT_DEBUG_BtlAllWazaCheck, NULL );
          GAMESYSTEM_SetEvent( gsys, new_event );
        }
      // 全技チェック2(トリプルバトル)
        else if( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA2 ) )
        {
          new_event = GMEVENT_CreateOverlayEventCall( gsys, 
              FS_OVERLAY_ID( debug_all_waza_check2 ), EVENT_DEBUG_BtlAllWazaCheck2, NULL );
          GAMESYSTEM_SetEvent( gsys, new_event );
        }
      // 全ポケモンチェック
        else if( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_POKE ) )
        {
          new_event = GMEVENT_CreateOverlayEventCall( gsys, 
              FS_OVERLAY_ID( debug_all_poke_check ), EVENT_DEBUG_BtlAllPokeCheck, NULL );
          GAMESYSTEM_SetEvent( gsys, new_event );
        }
      // 全技カメラチェック
        else if( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA_CAM ) )
        {
          new_event = GMEVENT_CreateOverlayEventCall( gsys, 
              FS_OVERLAY_ID( debug_all_waza_cam_check ), EVENT_DEBUG_BtlAllWazaCamCheck, NULL );
          GAMESYSTEM_SetEvent( gsys, new_event );
        }
        else if( HUDSON_IsTestCode( HUDSON_TESTCODE_POKE_CHANGE ) )
        {
          new_event = GMEVENT_CreateOverlayEventCall( gsys, 
              FS_OVERLAY_ID( debug_poke_change ), EVENT_DEBUG_BtlPokeChange, NULL );
          GAMESYSTEM_SetEvent( gsys, new_event );
        }
        else if( HUDSON_IsTestCode( HUDSON_TESTCODE_JA_SHOP_CHECK ) )
        {
          Hudson_CreateTask( sp_wk, &HUDSON_AUTOTEST_ResortShopCheck, FS_OVERLAY_ID( debug_resort_check ) );
        }
    }
    break;
  }
#endif
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タイトルをスキップするか判定。引数が指定されていた場合、スキップ。
 *
 *	@param	void 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL HUDSON_IsSkipTitle( void )
{
  if( OS_GetArgc() < 2 )
  {
    return FALSE;
  }

  return TRUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  テストコードをチェック
 *
 *	@param	const char* str 
 *
 *	@retval TRUE:指定したテストコードと一致
 */
//-----------------------------------------------------------------------------
BOOL HUDSON_IsTestCode( const char* str )
{ 
  return ( OS_GetArgc() >= 2 && STD_StrCmp( OS_GetArgv(1), str ) == 0 );
}


//----------------------------------------------------------------------------
/**
 *	@brief  フラグをセット
 *
 *	@param	int flag  フラグ
 *	@param	is_on     TRUEフラグをON  FALSEフラグをOFF
 */
//-----------------------------------------------------------------------------
void HUDSON_AUTOTEST_SetFlag( int flag, BOOL is_on )
{
  const u8 bit_idx = flag%8;
  const u8 arr_idx = flag/8;
  GF_ASSERT( flag < HUDSON_AUTOTEST_FLAG_NUM );
  if( is_on )
  {
    sp_wk->flag[arr_idx] |= (1<<bit_idx);
  }
  else
  {
    sp_wk->flag[arr_idx] &= ~(1<<bit_idx);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フラグを取得
 *
 *	@param	int flag  フラグ
 *
 *	@return TRUEフラグはON, FALSEフラグはOFF
 */
//-----------------------------------------------------------------------------
BOOL HUDSON_AUTOTEST_GetFlag( int flag )
{
  const u8 bit_idx = flag%8;
  const u8 arr_idx = flag/8;
  GF_ASSERT( flag < HUDSON_AUTOTEST_FLAG_NUM );
  
  if( sp_wk->flag[arr_idx] & (1<<bit_idx) )
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

//=============================================================================
/**
 *								plivate
 */
//=============================================================================

//----------------------------------------------------------------------------
/**
 *	@brief  タスク生成
 *
 *	@param	GFL_TCBLFUNC * func タスク関数
 *	@param	work_size   ワークサイズ
 */
//-----------------------------------------------------------------------------
static void Hudson_CreateTask( HUDSON_WORK * p_wk, const HUDSON_AUTOTEST_WORK *autotest, FSOverlayID ov_id )
{
  if( p_wk->task_overlayID != GFL_OVERLAY_BLANK_ID )
  {
    GFL_OVERLAY_Unload( p_wk->task_overlayID );
  }
  p_wk->task_overlayID = ov_id;

  GFL_OVERLAY_Load( p_wk->task_overlayID );

  {
    GFL_TCBL *p_tcbl = GFL_TCBL_Create(p_wk->p_tcblsys, autotest->Update, autotest->GetSize(), 0);
    GF_ASSERT( p_tcbl );
    {
      void *p_wk_adrs = GFL_TCBL_GetWork(p_tcbl);
      autotest->Init( p_wk_adrs, p_wk->gsys, p_wk->heapID );
    }

    p_wk->is_task_create  = TRUE;
  }
}

#endif // DEBUG_ONLY_FOR_hudson
#endif // PM_DEBUG

