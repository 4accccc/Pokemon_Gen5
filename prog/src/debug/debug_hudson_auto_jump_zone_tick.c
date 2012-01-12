//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debug_hudson_auto_jump_zone_tick.c
 *	@brief  マップジャンプ処理負荷チェック
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
#include "debug/debug_hudson_auto_jump_zone_tick.h"
#include "field/fieldmap_resist.h"
#include "field/map_matrix.h"
#include "field/field_data.h"
#include "field/zonedata.h"

#ifdef DEBUG_ONLY_FOR_hudson
#ifdef PM_DEBUG

extern void DEBUG_MAIN_TIME_Start( int sync );
extern BOOL DEBUG_MAIN_TIME_End( int *p_ave, int *p_over_cnt );

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
// デバッグ出力先
#define PRINT_TARGET (3)

#define CHECK_SYNC  (3*120)


typedef enum
{
  TICK_CHECK_POS_CENTER,
  TICK_CHECK_POS_U,
  TICK_CHECK_POS_D,
  TICK_CHECK_POS_L,
  TICK_CHECK_POS_R,
  TICK_CHECK_POS_MAX,
}TICK_CHECK_POS;

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
  TICK_CHECK_POS  pos;

  HEAPID  heapID;

} RESORT_JUMP_TICK_WK;


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void RESORT_JUMP_TICK_Init( void *p_wk_adrs, GAMESYS_WORK* p_gamesys, HEAPID heapID );
static void RESORT_JUMP_TICK_Update( GFL_TCBL *p_task, void *p_wk_adrs );
static u32 RESORT_JUMP_TICK_GetWorkSize( void );

static u16 GetCurrentZoneID( const RESORT_JUMP_TICK_WK *cp_wk );
static void UpdateCheckZoneID( RESORT_JUMP_TICK_WK *p_wk );
static BOOL CheckValidZoneID( u16 zoneID );
static BOOL WaitCnt( RESORT_JUMP_TICK_WK *p_wk, u32 max );
static void InitCheck( RESORT_JUMP_TICK_WK *p_wk );
static BOOL IsEndCheck( const RESORT_JUMP_TICK_WK *cp_wk );

static void DebugPrint_Start( const RESORT_JUMP_TICK_WK *cp_wk );
static void DebugPrint_Prog( const RESORT_JUMP_TICK_WK *cp_wk, int ave, int over );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
const HUDSON_AUTOTEST_WORK HUDSON_AUTOTEST_JumpTickCheck =
{
  RESORT_JUMP_TICK_Init,
  RESORT_JUMP_TICK_Update,
  RESORT_JUMP_TICK_GetWorkSize,
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
static void RESORT_JUMP_TICK_Init( void *p_wk_adrs, GAMESYS_WORK* p_gamesys, HEAPID heapID )
{
  RESORT_JUMP_TICK_WK  *p_wk = p_wk_adrs;
  GFL_STD_MemClear( p_wk, sizeof(RESORT_JUMP_TICK_WK) );
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
static void RESORT_JUMP_TICK_Update( GFL_TCBL *p_task, void *p_wk_adrs )
{
  enum
  {
    SEQ_INIT,
    SEQ_TICK_START,
    SEQ_TICK_WAIT,
    SEQ_NEXT_INIT,
    SEQ_JUMP_NEXT_START,
    SEQ_JUMP_NEXT_WAIT,
    SEQ_END,
  };

  RESORT_JUMP_TICK_WK  *p_wk = p_wk_adrs;
  GAMESYS_WORK  *p_gamesys  = p_wk->p_gamesys;
  GAMEDATA      *p_gamedata = GAMESYSTEM_GetGameData( p_gamesys );
  FIELDMAP_WORK *p_field    = GAMESYSTEM_GetFieldMapWork( p_gamesys );

  switch( p_wk->seq )
  {
  case SEQ_INIT:
    InitCheck( p_wk );
    DebugPrint_Start( p_wk );
    p_wk->seq = SEQ_TICK_START;
    break;
  case SEQ_TICK_START:
    DEBUG_MAIN_TIME_Start( CHECK_SYNC );
    p_wk->seq = SEQ_TICK_WAIT;
    break;
  case SEQ_TICK_WAIT:
    {
      int ave;
      int over;
      if( DEBUG_MAIN_TIME_End( &ave, &over ) )
      {
        DebugPrint_Prog(p_wk, ave, over);
        p_wk->seq = SEQ_NEXT_INIT;
      }
    }
    break;
  case SEQ_NEXT_INIT:
    UpdateCheckZoneID( p_wk );
    if( IsEndCheck( p_wk ) )
    {
      p_wk->seq = SEQ_END;
    }
    else
    {
      p_wk->seq = SEQ_JUMP_NEXT_START;
    }
    break;
  case SEQ_JUMP_NEXT_START:
    if( !GAMESYSTEM_IsEventExists(p_gamesys) )
    {
      MAP_MATRIX *p_mtx = MAP_MATRIX_Create( p_wk->heapID );
      u16 w, h, x, z;
      u16 block_x, block_z;
      VecFx32 appear_pos;
      GMEVENT* p_event;

      //飛び先座標のためにマトリックス作成
      MAP_MATRIX_Init( p_mtx, ZONEDATA_GetMatrixID(p_wk->check_zone_id), p_wk->check_zone_id, p_wk->heapID );
      MAP_MATRIX_CheckReplace( p_mtx, p_gamesys, p_wk->heapID );

      w = MAP_MATRIX_GetMapBlockSizeWidth( p_mtx );
      h = MAP_MATRIX_GetMapBlockSizeHeight( p_mtx );

      DEBUG_FIELDDATA_GetBrockSize( p_wk->check_zone_id, &block_x, &block_z );

      switch( p_wk->pos )
      {
      case TICK_CHECK_POS_CENTER:
        x = block_x*32/2;
        z = block_z*32/2;
        break;
      case TICK_CHECK_POS_U:
        x = block_x*32/2;
        z = 1;
        break;
      case TICK_CHECK_POS_D:
        x = block_x*32/2;
        z = block_z*32-1;
        break;
      case TICK_CHECK_POS_L:
        x = 1;
        z = block_z*32/2;
        break;
      case TICK_CHECK_POS_R:
        x = block_x*32-1;
        z = block_z*32/2;
        break;
      default:
        GF_ASSERT(0);
      }

      OS_TFPrintf( 1, "zoneID%d w%d h%d x%d z%d bx%d bz%d\n", p_wk->check_zone_id, w, h, x, z, block_x, block_z );

      VEC_Set(&appear_pos,
          GRID_TO_FX32(w*32+x),
          GRID_TO_FX32(0),
          GRID_TO_FX32(h*32+z)
          );

      MAP_MATRIX_Delete( p_mtx );
      
      p_event = EVENT_ChangeMapPos( 
        p_gamesys, p_field, p_wk->check_zone_id, &appear_pos, DIR_DOWN, FALSE );
      GAMESYSTEM_SetEvent( p_gamesys, p_event );
      
      p_wk->seq = SEQ_JUMP_NEXT_WAIT;
    }
    break;
  case SEQ_JUMP_NEXT_WAIT:
    if( !GAMESYSTEM_IsEventExists(p_gamesys) )
    {
      p_wk->seq = SEQ_TICK_START;
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
static u32 RESORT_JUMP_TICK_GetWorkSize( void )
{
  return sizeof( RESORT_JUMP_TICK_WK );
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在のマップのゾーンID取得
 *
 *	@param	const RESORT_JUMP_TICK_WK *cp_wk   ワーク
 *
 *	@return 現在のマップのゾーンID
 */
//-----------------------------------------------------------------------------
static u16 GetCurrentZoneID( const RESORT_JUMP_TICK_WK *cp_wk )
{
  FIELDMAP_WORK* fieldmap   = GAMESYSTEM_GetFieldMapWork( cp_wk->p_gamesys );
  return FIELDMAP_GetZoneID( fieldmap );
}
//----------------------------------------------------------------------------
/**
 *	@brief  次のチェックゾーンIDアップデート
 *
 *	@param	RESORT_JUMP_TICK_WK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void UpdateCheckZoneID( RESORT_JUMP_TICK_WK *p_wk )
{
  if( ++p_wk->pos >= TICK_CHECK_POS_MAX )
  {
    p_wk->pos = 0;

    p_wk->check_zone_id++;
    p_wk->check_zone_id %= ZONE_ID_MAX;

    //チェックしてだめなら次へ
    while( !CheckValidZoneID(p_wk->check_zone_id) )
    {
      p_wk->check_zone_id++;
      p_wk->check_zone_id %= ZONE_ID_MAX;
    }
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
 *	@param	RESORT_JUMP_TICK_WK *p_wk  ワーク
 *	@param	max   カウント
 *
 *	@return TRUEカウント終了  FALSEカウント中
 */
//-----------------------------------------------------------------------------
static BOOL WaitCnt( RESORT_JUMP_TICK_WK *p_wk, u32 max )
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
 *	@param	RESORT_JUMP_TICK_WK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void InitCheck( RESORT_JUMP_TICK_WK *p_wk )
{
  p_wk->check_zone_id = GetCurrentZoneID( p_wk );
  p_wk->start_zone_id = p_wk->check_zone_id;
}
//----------------------------------------------------------------------------
/**
 *	@brief  終了チェック
 *
 *	@param	const RESORT_JUMP_TICK_WK *cp_wk   ワーク
 *
 *	@return TRUE終了  FALSEまだ計測中
 */
//-----------------------------------------------------------------------------
static BOOL IsEndCheck( const RESORT_JUMP_TICK_WK *cp_wk )
{
  return cp_wk->start_zone_id == cp_wk->check_zone_id && cp_wk->pos == 0;
}
//------------------------------------------------------------------------
/**
 * @brief デバッグ出力  開始
 * @param cp_wk ワーク
 */
//------------------------------------------------------------------------
static void DebugPrint_Start( const RESORT_JUMP_TICK_WK *cp_wk )
{
  OS_TFPrintf( PRINT_TARGET, "==========================\n" );
  OS_TFPrintf( PRINT_TARGET, "TICK-JUMP-CHECK: Start!!\n" );
  OS_TFPrintf( PRINT_TARGET, "==========================\n" );
}
//------------------------------------------------------------------------
/**
 * @brief デバッグ出力  経過
 * @param cp_wk ワーク
 */
//------------------------------------------------------------------------
static void DebugPrint_Prog( const RESORT_JUMP_TICK_WK *cp_wk, int ave, int over )
{
  static const char *sc_str[]=
  {
    "Center",
    "Up",
    "Down",
    "Left",
    "Right",
  };

  if( ave > 16000 )
  {
    OS_TFPrintf( PRINT_TARGET, "× ZoneID[%d][%s] ave=%d over=%d\n", GetCurrentZoneID(cp_wk), sc_str[cp_wk->pos], ave, over );
  }
  else
  {
    OS_TFPrintf( PRINT_TARGET, "○ ZoneID[%d][%s] ave=%d over=%d\n", GetCurrentZoneID(cp_wk),  sc_str[cp_wk->pos], ave, over );
  }
}

#endif //PM_DEBUG
#endif // DEBUG_ONLY_FOR_hudson

