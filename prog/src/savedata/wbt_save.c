//============================================================================================
/**
 * @file  wbt_save.c
 * @brief   WBT用セーブデータ関連
 * @author  tamada@gamefreak.co.jp
 * @date  2011.04.05
 */
//============================================================================================
#include "gflib.h"

#include "savedata/wbt_save.h"

#include "wbt_save_local.h"

#include "savedata/trialhouse_save.h"
#include "trialhouse_save_local.h"

#include "poke_tool/poketype_def.h"

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  CHAMPION_BATTLE_KEY = 'SUGI'
};

//優勝回数カウントとポケモンのタイプとを一致させるためのAssert
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_NORMAL == POKETYPE_NORMAL );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_KAKUTOU == POKETYPE_KAKUTOU );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_HIKOU == POKETYPE_HIKOU );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_DOKU == POKETYPE_DOKU );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_JIMEN == POKETYPE_JIMEN );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_IWA == POKETYPE_IWA );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_MUSHI == POKETYPE_MUSHI );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_GHOST == POKETYPE_GHOST );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_HAGANE == POKETYPE_HAGANE );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_HONOO == POKETYPE_HONOO );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_MIZU == POKETYPE_MIZU );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_KUSA == POKETYPE_KUSA );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_DENKI == POKETYPE_DENKI );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_ESPER == POKETYPE_ESPER );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_KOORI == POKETYPE_KOORI );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_DRAGON == POKETYPE_DRAGON );
SDK_COMPILER_ASSERT( WBTWIN_COUNT_POKETYPE_AKU == POKETYPE_AKU );

//SDK_COMPILER_ASSERT( WBTWIN_COUNT_ALLTOURNAMENT == POKETYPE_MAX );

//include/savedata/wbt_save.hとsrc/savedata/wbt_save_local.hとでの定義に食い違いがないように
SDK_COMPILER_ASSERT( WBTWIN_COUNT_MAX == LOCAL_WBTWIN_COUNT_MAX );
SDK_COMPILER_ASSERT( WBTSAVE_FLAG_MAX == LOCAL_WBTSAVE_FLAG_MAX );
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
u32 WBTSAVE_GetWorkSize(void)
{
  return sizeof(WBTSAVE);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSAVE_Init( WBTSAVE * wbtsave )
{
  GFL_STD_MemClear( wbtsave, sizeof(WBTSAVE) );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
WBTSAVE * SAVEDATA_GetWBTSave( SAVE_CONTROL_WORK * sv )
{
  THSV_WORK * thsv = THSV_GetSvPtr( sv );
  return &thsv->wbtsave;
}
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void WBTSAVE_UpdateChampionKey( WBTSAVE * wbtsave )
{
  wbtsave->ChampionTournamentFlag = CHAMPION_BATTLE_KEY;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
u16 WBTSAVE_GetWinCount( const WBTSAVE * wbtsave, WBTWIN_COUNT id )
{
  GF_ASSERT( id < WBTWIN_COUNT_MAX );
  return wbtsave->WinCount[ id ];
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void WBTSAVE_SetWinCount( WBTSAVE * wbtsave, WBTWIN_COUNT id, u16 value )
{
  GF_ASSERT( id < WBTWIN_COUNT_MAX );
  if ( value > WBTWIN_MAX )
  {
    value = WBTWIN_MAX;
  }
  wbtsave->WinCount[ id ] = value;
}

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
void WBTSAVE_IncWinCount( WBTSAVE * wbtsave, WBTWIN_COUNT id )
{
  GF_ASSERT( id < WBTWIN_COUNT_MAX );
  if ( wbtsave->WinCount[ id ] < WBTWIN_MAX )
  {
    wbtsave->WinCount[ id ] += 1;
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL WBTSAVE_GetFlag( const WBTSAVE * wbtsave, WBTSAVE_FLAG flag_id )
{
  GF_ASSERT( flag_id < WBTSAVE_FLAG_MAX );
  return ( wbtsave->flag[ flag_id / 8 ] & ( 1 << (flag_id % 8) ) != 0 );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSAVE_SetFlag( WBTSAVE * wbtsave, WBTSAVE_FLAG flag_id )
{
  GF_ASSERT( flag_id < WBTSAVE_FLAG_MAX );
  wbtsave->flag[ flag_id / 8 ] |= ( 1 << (flag_id % 8) );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void WBTSAVE_ResetFlag( WBTSAVE * wbtsave, WBTSAVE_FLAG flag_id )
{
  GF_ASSERT( flag_id < WBTSAVE_FLAG_MAX );
  wbtsave->flag[ flag_id / 8 ] &= ~((u8)( 1 << (flag_id % 8) ));
}

