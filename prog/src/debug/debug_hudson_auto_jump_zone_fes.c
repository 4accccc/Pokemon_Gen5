//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debug_hudson_auto_jump_zone_fes.c
 *	@brief  フェス起動してのマップジャンプチェック
 *	@author	Toru=Nagihashi
 *	@date		2011.01.11
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
#include "../../../resource/fldmapdata/zonetable/zone_id.h" // for ZONE_ID_xxxx
#include "debug/debug_hudson_auto_jump_zone_fes.h"


#ifdef DEBUG_ONLY_FOR_hudson
#ifdef PM_DEBUG

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
// デバッグ出力先
#define PRINT_TARGET (3)
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
  u32 cnt;

  ZONEID check_zone_id;
  ZONEID start_zone_id;

  HEAPID  heapID;

} RESORT_JUMP_FES_WK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void RESORT_JUMP_FES_Init( void *p_wk_adrs, GAMESYS_WORK* p_gamesys, HEAPID heapID );
static void RESORT_JUMP_FES_Update( GFL_TCBL *p_task, void *p_wk_adrs );
static u32 RESORT_JUMP_FES_GetWorkSize( void );

static u16 GetCurrentZoneID( const RESORT_JUMP_FES_WK *cp_wk );
static void UpdateCheckZoneID( RESORT_JUMP_FES_WK *p_wk );
static BOOL CheckValidZoneID( u16 zoneID );
static BOOL WaitCnt( RESORT_JUMP_FES_WK *p_wk, u32 max );
static void InitCheck( RESORT_JUMP_FES_WK *p_wk );
static BOOL IsEndCheck( const RESORT_JUMP_FES_WK *cp_wk );

static void DebugPrint_Start( const RESORT_JUMP_FES_WK *cp_wk );
static void DebugPrint_End( const RESORT_JUMP_FES_WK *cp_wk );
static void DebugPrint_Prog( const RESORT_JUMP_FES_WK *cp_wk );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
const HUDSON_AUTOTEST_WORK HUDSON_AUTOTEST_FesMapJump =
{
  RESORT_JUMP_FES_Init,
  RESORT_JUMP_FES_Update,
  RESORT_JUMP_FES_GetWorkSize,
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
static void RESORT_JUMP_FES_Init( void *p_wk_adrs, GAMESYS_WORK* p_gamesys, HEAPID heapID )
{
  RESORT_JUMP_FES_WK  *p_wk = p_wk_adrs;
  GFL_STD_MemClear( p_wk, sizeof(RESORT_JUMP_FES_WK) );
  p_wk->p_gamesys = p_gamesys;
  p_wk->heapID    = heapID;
}

//----------------------------------------------------------------------------
/**
 *	@brief  更新
 *
 *	@param	GFL_TCBL *p_task  タスク
 *	@param	*p_wk_adrs ワーク
 */
//-----------------------------------------------------------------------------
static void RESORT_JUMP_FES_Update( GFL_TCBL *p_task, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_FES_START,
    SEQ_FES_WAIT,
    SEQ_CHECK_WAIT,
    SEQ_NEXT_INIT,
    SEQ_EVENT_SKIP_START,
    SEQ_EVENT_SKIP_WAIT,
    SEQ_JUMP_NEXT_START,
    SEQ_JUMP_NEXT_WAIT,
    SEQ_END,
  };

  RESORT_JUMP_FES_WK  *p_wk = p_wk_adrs;
  GAMESYS_WORK  *p_gamesys  = p_wk->p_gamesys;
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  FIELDMAP_WORK *p_field    = GAMESYSTEM_GetFieldMapWork( p_gamesys );
  LINK_FESTIVAL *p_fes = GAMESYSTEM_GetLinkFestivalWork( p_gamesys );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    InitCheck( p_wk );
    DebugPrint_Start( p_wk );
    p_wk->seq = SEQ_FES_START;
    break;
  case SEQ_FES_START:
    {
      FEST_MISSION mission;
      FMISSION_DATA_GetMissionParamDirect( FMISSION_ID_NPC_PRESENT_GIFT, FMISSION_LEVEL_4, &mission, p_wk->heapID );
      LINK_FESTIVAL_MissionStart( p_fes, &mission, FALSE );
      LINK_FESTIVAL_MissionLiveStatusSet( p_fes, FALSE );
    }

    p_wk->seq = SEQ_FES_WAIT;
    break;
  case SEQ_FES_WAIT:
    p_wk->seq = SEQ_CHECK_WAIT;
    break;
  case SEQ_CHECK_WAIT:
    if( WaitCnt( p_wk, 60 ) )
    {
      DebugPrint_Prog(p_wk);
      p_wk->seq = SEQ_NEXT_INIT;
    }
    break;
  case SEQ_NEXT_INIT:
    UpdateCheckZoneID( p_wk );
    if( IsEndCheck( p_wk ) )
    {
      DebugPrint_End( p_wk );
      p_wk->seq = SEQ_END;
    }
    else
    {
      p_wk->seq = SEQ_EVENT_SKIP_START;
    }
    break;
  case SEQ_EVENT_SKIP_START:
    DEBUG_UI_SetUp( DEBUG_UI_AUTO_A, DEBUG_UI_PLAY_LOOP );
    p_wk->seq = SEQ_EVENT_SKIP_WAIT;
    break;
  case SEQ_EVENT_SKIP_WAIT:
    if( !GAMESYSTEM_IsEventExists(p_gamesys) )
    {
      DEBUG_UI_SetUp( DEBUG_UI_NONE, DEBUG_UI_PLAY_ONE );
      p_wk->seq = SEQ_JUMP_NEXT_START;
    }
    break;
  case SEQ_JUMP_NEXT_START:
    {
      GMEVENT* p_event = DEBUG_EVENT_ChangeMapDefaultPos( 
        p_gamesys, p_field, p_wk->check_zone_id  );
      GAMESYSTEM_SetEvent( p_gamesys, p_event );
      
      p_wk->seq = SEQ_JUMP_NEXT_WAIT;
    }
    break;
  case SEQ_JUMP_NEXT_WAIT:
    if( !GAMESYSTEM_IsEventExists(p_gamesys) )
    {
      p_wk->seq = SEQ_CHECK_WAIT;
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
static u32 RESORT_JUMP_FES_GetWorkSize( void )
{
  return sizeof( RESORT_JUMP_FES_WK );
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のマップのゾーンID取得
 *
 *	@param	const RESORT_JUMP_FES_WK *cp_wk   ワーク
 *
 *	@return 現在のマップのゾーンID
 */
//-----------------------------------------------------------------------------
static u16 GetCurrentZoneID( const RESORT_JUMP_FES_WK *cp_wk )
{
  FIELDMAP_WORK* fieldmap   = GAMESYSTEM_GetFieldMapWork( cp_wk->p_gamesys );
  return FIELDMAP_GetZoneID( fieldmap );
}
//----------------------------------------------------------------------------
/**
 *	@brief  次のチェックゾーンIDアップデート
 *
 *	@param	RESORT_JUMP_FES_WK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void UpdateCheckZoneID( RESORT_JUMP_FES_WK *p_wk )
{
  p_wk->check_zone_id++;
  p_wk->check_zone_id %= ZONE_ID_MAX;

  //チェックしてだめなら次へ
  while( !CheckValidZoneID(p_wk->check_zone_id) )
  {
    p_wk->check_zone_id++;
    p_wk->check_zone_id %= ZONE_ID_MAX;
  }
  
}
//----------------------------------------------------------------------------
/**
 *	@brief  チェックしてよいゾーンIDかチェック
 *
 *	@param	u16 zoneID  チェックするゾーンID
 *
 *	@return TRUEチェック有効  FALSEチェック不可
 */
//-----------------------------------------------------------------------------
static BOOL CheckValidZoneID( u16 zoneID )
{
  static const u16 sc_valid_zoneID[] = 
  {
    ZONE_ID_T06R0101,
    ZONE_ID_T01R0101,
    ZONE_ID_D12,  //イベントが発生
    ZONE_ID_C07R0201, //イベントが発生
    ZONE_ID_C04R0201, //イベントが発生
    ZONE_ID_UNION,
    ZONE_ID_CLOSSEUM,
    ZONE_ID_CLOSSEUM02,
    ZONE_ID_PLD10,
    ZONE_ID_C04R0202, // ミュージカル: 正しい手順を経て入室する必要がある
    ZONE_ID_C09P01,   // リーグフロント四天王分岐部屋 ( INIT_LABEL でスクリプトを予約するため )
  };

  int i;
  for( i = 0; i < NELEMS(sc_valid_zoneID); i++ )
  {
    if( sc_valid_zoneID[i] == zoneID )
    {
      return FALSE;
    }
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  カウント
 *
 *	@param	RESORT_JUMP_FES_WK *p_wk  ワーク
 *	@param	max   カウント
 *
 *	@return TRUEカウント終了  FALSEカウント中
 */
//-----------------------------------------------------------------------------
static BOOL WaitCnt( RESORT_JUMP_FES_WK *p_wk, u32 max )
{
  if( p_wk->cnt++ < max )
  {
    return FALSE;
  }
  else
  {
    p_wk->cnt = 0;
    return TRUE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  初期化
 *
 *	@param	RESORT_JUMP_FES_WK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void InitCheck( RESORT_JUMP_FES_WK *p_wk )
{
  p_wk->check_zone_id = GetCurrentZoneID( p_wk );
  p_wk->start_zone_id = p_wk->check_zone_id;
}
//----------------------------------------------------------------------------
/**
 *	@brief  終了チェック
 *
 *	@param	const RESORT_JUMP_FES_WK *cp_wk   ワーク
 *
 *	@return TRUE終了  FALSEまだ計測中
 */
//-----------------------------------------------------------------------------
static BOOL IsEndCheck( const RESORT_JUMP_FES_WK *cp_wk )
{
  return cp_wk->start_zone_id == cp_wk->check_zone_id;
}
//------------------------------------------------------------------------
/**
 * @brief デバッグ出力  開始
 * @param cp_wk ワーク
 */
//------------------------------------------------------------------------
static void DebugPrint_Start( const RESORT_JUMP_FES_WK *cp_wk )
{
  OS_TFPrintf( PRINT_TARGET, "==========================\n" );
  OS_TFPrintf( PRINT_TARGET, "FES-JUMP-CHECK: Start!!\n" );
  OS_TFPrintf( PRINT_TARGET, "==========================\n" );
}
//------------------------------------------------------------------------
/**
 * @brief デバッグ出力  終了
 * @param cp_wk ワーク
 */
//------------------------------------------------------------------------
static void DebugPrint_End( const RESORT_JUMP_FES_WK *cp_wk )
{
  OS_TFPrintf( PRINT_TARGET, "==========================\n" );
  OS_TFPrintf( PRINT_TARGET, "FES-JUMP-CHECK: End!!\n" );
  OS_TFPrintf( PRINT_TARGET, "==========================\n" );
}
//------------------------------------------------------------------------
/**
 * @brief デバッグ出力  経過
 * @param cp_wk ワーク
 */
//------------------------------------------------------------------------
static void DebugPrint_Prog( const RESORT_JUMP_FES_WK *cp_wk )
{
  OS_TFPrintf( PRINT_TARGET, "NOW = ZoneID[%d]\n", GetCurrentZoneID(cp_wk) );
}

#endif //PM_DEBUG
#endif // DEBUG_ONLY_FOR_hudson

