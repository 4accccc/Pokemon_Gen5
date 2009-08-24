//======================================================================
/**
 * @file  musical_program.c
 * @brief ミュージカル用 演目
 * @author  ariizumi
 * @data  09/08/21
 *
 * モジュール名：MUSICAL_PROGRAM
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "musical_program.h"
#include "musical/musical_stage_sys.h"
#include "musical/mus_item_data.h"
#include "musical/stage/sta_local_def.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//演目のデータ
struct _MUSICAL_PROGRAM_DATA
{
  u8 condition[MPC_DATA_MAX];
  //デフォルトの参加ポケとか
};

//演目のワーク
struct _MUSICAL_PROGRAM_WORK
{
  u8 condition[MPC_MAX];
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	ミュージカルデータの読み込み
//--------------------------------------------------------------
MUSICAL_PROGRAM_WORK* MUSICAL_PROGRAM_GetProgramData( HEAPID heapId )
{
  u8 i;
  MUSICAL_PROGRAM_WORK *progWork = GFL_HEAP_AllocMemory( heapId , sizeof( MUSICAL_PROGRAM_WORK ) );
  MUSICAL_PROGRAM_DATA *progData = GFL_HEAP_AllocMemory( heapId , sizeof( MUSICAL_PROGRAM_DATA ) );
  for( i=0;i<MPC_DATA_MAX;i++ )
  {
    progData->condition[i] = 0;
  }
  for( i=0;i<MPC_MAX;i++ )
  {
    progWork->condition[i] = 0;
  }
  
  //ランダム生成(デバッグ
  {
    u8 point = MUSICAL_PROGRAM_CONDITION_SUM/5;
    while( point > 0 )
    {
      u8 val = GFL_STD_MtRand( 3 )+GFL_STD_MtRand( 3 )+2;
      if( val > point )
      {
        val = point;
      }
      
      progData->condition[GFL_STD_MtRand(MPC_DATA_MAX)] += val*5;
      point -= val;
    }
  }
  
  OS_TPrintf("--Condition--\n");
  OS_TPrintf("Cool   [%3d]\n",progData->condition[0]);
  OS_TPrintf("Cute   [%3d]\n",progData->condition[1]);
  OS_TPrintf("Elegant[%3d]\n",progData->condition[2]);
  OS_TPrintf("Unique [%3d]\n",progData->condition[3]);
  OS_TPrintf("Random [%3d]\n",progData->condition[4]);
  OS_TPrintf("--Condition--\n");
  

  //データからワークへコピー
  for( i=0;i<MPC_MAX;i++ )
  {
    progWork->condition[i] = progData->condition[i];
  }
  //ランダム分の分配
  {
    u8 point = progData->condition[MPC_RANDOM]/5;
    while( point > 0 )
    {
      u8 val = GFL_STD_MtRand( 3 )+1;
      if( val > point )
      {
        val = point;
      }
      
      progWork->condition[GFL_STD_MtRand(MPC_MAX)] += val*5;
      point -= val;
    }
  }
  OS_TPrintf("--FinalCondition--\n");
  OS_TPrintf("Cool   [%3d]\n",progWork->condition[0]);
  OS_TPrintf("Cute   [%3d]\n",progWork->condition[1]);
  OS_TPrintf("Elegant[%3d]\n",progWork->condition[2]);
  OS_TPrintf("Unique [%3d]\n",progWork->condition[3]);
  OS_TPrintf("--FinalCondition--\n");

  GFL_HEAP_FreeMemory( progData );

  return progWork;
}


//--------------------------------------------------------------
//	得点配分の決定
//--------------------------------------------------------------
void MUSICAL_PROGRAM_CalcPokemonPoint( HEAPID heapId , MUSICAL_PROGRAM_WORK* progWork , STAGE_INIT_WORK *actInitWork )
{
  u8 conIdx;
  u8 pokeIdx;
  u8 pokeCondition[MUSICAL_POKE_MAX][MPC_MAX];
  u8 conditionMax[MPC_MAX];
  MUS_ITEM_DATA_SYS *itemDataSys = MUS_ITEM_DATA_InitSystem( heapId );
  
  for( conIdx=0;conIdx<MPC_MAX;conIdx++ )
  {
    for( pokeIdx = 0 ; pokeIdx < MUSICAL_POKE_MAX ; pokeIdx++ )
    {
      pokeCondition[pokeIdx][conIdx] = 0;
    }
    conditionMax[conIdx] = 0;
  }

  for( pokeIdx = 0 ; pokeIdx < MUSICAL_POKE_MAX ; pokeIdx++ )
  {
    u8 ePos;
    MUSICAL_POKE_PARAM *musPoke = &actInitWork->musPoke[pokeIdx];
    for( ePos = 0 ; ePos < MUS_POKE_EQUIP_MAX ; ePos++ )
    {
      if( musPoke->equip[ePos].itemNo != MUSICAL_ITEM_INVALID )
      {
        const u8 conType = MUS_ITEM_DATA_GetItemConditionType( itemDataSys , musPoke->equip[ePos].itemNo );
        
        pokeCondition[pokeIdx][conType]++;
        conditionMax[conType]++;
      }
    }
  }
  
  for( pokeIdx = 0 ; pokeIdx < MUSICAL_POKE_MAX ; pokeIdx++ )
  {
    MUSICAL_POKE_PARAM *musPoke = &actInitWork->musPoke[pokeIdx];
    OS_TPrintf("Poke[%d]:",pokeIdx);
    for( conIdx=0;conIdx<MPC_MAX;conIdx++ )
    {
      const u8 addPoint = progWork->condition[conIdx] * pokeCondition[pokeIdx][conIdx] / conditionMax[conIdx];
      musPoke->point += addPoint;
      OS_TPrintf("[%3d]",addPoint);
    }
    OS_TPrintf("Sum[%3d]\n",musPoke->point);
  }
  
  
  MUS_ITEM_DATA_ExitSystem( itemDataSys );
}
