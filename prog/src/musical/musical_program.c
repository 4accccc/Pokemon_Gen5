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
#include "arc_def.h"

#include "musical_program.h"
#include "musical/musical_stage_sys.h"
#include "musical/mus_item_data.h"
#include "musical/stage/sta_local_def.h"
#include "test/ariizumi/ari_debug.h"
#include "musical/musical_debug_menu.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define MUS_PROG_DATA_EQUIP_NUM (8)
#define MUS_PROG_DATA_POKE_NUM (6)

#define MUS_PROG_DATA_EQUIP_NONE (511)
#define MUS_PROG_DATA_EQUIP_RANDOM (510)
#define MUS_PROG_DATA_POKE_RANDOM (65535)
//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//出演ポケ装備データ
typedef struct
{
  u16 itemNo;
  u8  itemPos;
  u8 pad;
}MUSICAL_PROGRAM_EQUIP_DATA;

//出演ポケデータ
typedef struct
{
  u16 monsno;
  u8  trainerType;
  u8  trainerName;
  u8  trainerSex;
  u8  pokeSex;
  u16 appealTime;
  u16 openPoint;  //登場ポイント
  u16 padding;
  MUSICAL_PROGRAM_EQUIP_DATA equipData[MUS_PROG_DATA_EQUIP_NUM];
}MUSICAL_PROGRAM_POKE_DATA;

//演目のデータ
typedef struct 
{
  u8 bgNo;
  u8 condition[MCT_DATA_MAX];
  u8 rewardType;
  u8 pad;
  u16 rewardValue;
  u8  version;    //WB初期１(musical_shotとは別
  u8  langCode;   //pm_version
  MUSICAL_PROGRAM_POKE_DATA pokeData[MUS_PROG_DATA_POKE_NUM];
  
}MUSICAL_PROGRAM_DATA;

//演目のワーク
struct _MUSICAL_PROGRAM_WORK
{
  u8 condition[MCT_MAX];
  u8 conOnePoint[MCT_MAX];  //コンディションを総個数で割った値(アピールボーナスに必要
  u8 npcArr[3];             //抽選されたNPCの数
  
  MUSICAL_PROGRAM_DATA *progData;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	ミュージカルデータの読み込み
//--------------------------------------------------------------
MUSICAL_PROGRAM_WORK* MUSICAL_PROGRAM_InitProgramData( HEAPID heapId , MUSICAL_DISTRIBUTE_DATA *distData , const u16 sumPoint )
{
  u8 i;
  MUSICAL_PROGRAM_WORK *progWork = GFL_HEAP_AllocMemory( heapId , sizeof( MUSICAL_PROGRAM_WORK ) );
#if PM_DEBUG
  MUSICAL_DEBUG_MENU_WORK *debWork = MUSICAL_DEBUG_GetWork();
#endif
//  MUSICAL_PROGRAM_DATA *progData = GFL_HEAP_AllocMemory( heapId , sizeof( MUSICAL_PROGRAM_DATA ) );
  progWork->progData = distData->programData;

  for( i=0;i<MCT_MAX;i++ )
  {
    progWork->condition[i] = 0;
    progWork->conOnePoint[i] = 0;
  }
  
  MUS_TPrintf("--Condition--\n");
  MUS_TPrintf("Cool   [%3d]\n",progWork->progData->condition[0]);
  MUS_TPrintf("Cute   [%3d]\n",progWork->progData->condition[1]);
  MUS_TPrintf("Elegant[%3d]\n",progWork->progData->condition[2]);
  MUS_TPrintf("Unique [%3d]\n",progWork->progData->condition[3]);
  MUS_TPrintf("Random [%3d]\n",progWork->progData->condition[4]);
  MUS_TPrintf("--Condition--\n");
  

  //データからワークへコピー
  for( i=0;i<MCT_MAX;i++ )
  {
    progWork->condition[i] = progWork->progData->condition[i];
  }
  //ランダム分の分配
  {
    u8 point = progWork->progData->condition[MCT_RANDOM]/5;
    while( point > 0 )
    {
      u8 val = GFL_STD_MtRand( 3 )+1;
      if( val > point )
      {
        val = point;
      }
      
      progWork->condition[GFL_STD_MtRand(MCT_MAX)] += val*5;
      point -= val;
    }
  }
  MUS_TPrintf("--FinalCondition--\n");
  MUS_TPrintf("Cool   [%3d]\n",progWork->condition[0]);
  MUS_TPrintf("Cute   [%3d]\n",progWork->condition[1]);
  MUS_TPrintf("Elegant[%3d]\n",progWork->condition[2]);
  MUS_TPrintf("Unique [%3d]\n",progWork->condition[3]);
  MUS_TPrintf("--FinalCondition--\n");
  
  
  //NPCの抽選
  #if PM_DEBUG
  if( debWork != NULL && 
      debWork->forceNpc == TRUE )
  {
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      progWork->npcArr[i] = debWork->npc[i];
    }
  }
  else
  #endif
  {
    u8 dataArr[6]={0,0,0,0,0,0};
    u8 dataNum = 0;
    //条件を満たすものを抽出
    for( i=0;i<MUS_PROG_DATA_POKE_NUM;i++ )
    {
      if( progWork->progData->pokeData[i].openPoint <= sumPoint )
      {
        dataArr[dataNum] = i;
        dataNum++;
      }
    }
    if( dataNum<3 )
    {
      GF_ASSERT_MSG(dataNum>=3,"Musical program npc is under 3!!\n");
      dataArr[0] = 0;
      dataArr[1] = 1;
      dataArr[2] = 2;
      dataNum = 3;
    }
    //混ぜる
    for( i=0;i<30;i++ )
    {
      const u8 trg1 = GFL_STD_MtRand(dataNum);
      const u8 trg2 = GFL_STD_MtRand(dataNum);
      const u8 temp = dataArr[trg1];
      dataArr[trg1] = dataArr[trg2];
      dataArr[trg2] = temp;
    }
    //コピー
    
    MUS_TPrintf("--NPC Arr--\n");
    for( i=0;i<3;i++ )
    {
      MUS_TPrintf("[%d][%d]\n",i,dataArr[i]);
      progWork->npcArr[i] = dataArr[i];
    }
    MUS_TPrintf("--NPC Arr--\n");
  }
  return progWork;
}

//--------------------------------------------------------------
//	ミュージカルデータの破棄
//--------------------------------------------------------------
void MUSICAL_PROGRAM_TermProgramData( MUSICAL_PROGRAM_WORK *progWork )
{
  GFL_HEAP_FreeMemory( progWork );
}

//--------------------------------------------------------------
//	得点配分の決定
//--------------------------------------------------------------
void MUSICAL_PROGRAM_CalcPokemonPoint( HEAPID heapId , MUSICAL_PROGRAM_WORK* progWork , STAGE_INIT_WORK *actInitWork )
{
  u8 conIdx;
  u8 pokeIdx;
  u8 pokeCondition[MUSICAL_POKE_MAX][MCT_MAX];
  u8 conditionMax[MCT_MAX];
  MUS_ITEM_DATA_SYS *itemDataSys = MUS_ITEM_DATA_InitSystem( heapId );
  
  for( conIdx=0;conIdx<MCT_MAX;conIdx++ )
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
    MUSICAL_POKE_PARAM *musPoke = actInitWork->musPoke[pokeIdx];
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
  
  MUS_TPrintf("OneCondition:");
  for( conIdx=0;conIdx<MCT_MAX;conIdx++ )
  {
    progWork->conOnePoint[conIdx] = progWork->condition[conIdx] / conditionMax[conIdx];
    MUS_TPrintf("[%d]",progWork->conOnePoint[conIdx]);
  }
  MUS_TPrintf("\n");
  
  for( pokeIdx = 0 ; pokeIdx < MUSICAL_POKE_MAX ; pokeIdx++ )
  {
    MUSICAL_POKE_PARAM *musPoke = actInitWork->musPoke[pokeIdx];
    MUS_TPrintf("Poke[%d]:",pokeIdx);
    for( conIdx=0;conIdx<MCT_MAX;conIdx++ )
    {
      const u8 addPoint = progWork->conOnePoint[conIdx] * pokeCondition[pokeIdx][conIdx] ;
      musPoke->point += addPoint;
      musPoke->conPoint[conIdx] = addPoint;
      MUS_TPrintf("[%3d]",addPoint);
    }
    MUS_TPrintf("Sum[%3d]\n",musPoke->point);
  }
  
  
  MUS_ITEM_DATA_ExitSystem( itemDataSys );
}

//--------------------------------------------------------------
//	コンディション配分の取得
//--------------------------------------------------------------
const u8 MUSICAL_PROGRAM_GetConditionPoint( MUSICAL_PROGRAM_WORK* progWork , MUSICAL_CONDITION_TYPE conType )
{
  return progWork->condition[conType];
}
u32 MUSICAL_PROGRAM_GetConditionPointArr( MUSICAL_PROGRAM_WORK* progWork )
{
  u32 pointArr = progWork->condition[0] + 
                 (progWork->condition[1]<<8) + 
                 (progWork->condition[2]<<16) + 
                 (progWork->condition[3]<<24);
  return pointArr;
}
void MUSICAL_PROGRAM_SetConditionPointArr( MUSICAL_PROGRAM_WORK* progWork , const u32 pointArr )
{
  progWork->condition[0] = (pointArr&0x000000FF);
  progWork->condition[1] = (pointArr&0x0000FF00)>>8;
  progWork->condition[2] = (pointArr&0x00FF0000)>>16;
  progWork->condition[3] = (pointArr&0xFF000000)>>24;

  MUS_TPrintf("--FinalCondition(CommSync!)--\n");
  MUS_TPrintf("Cool   [%3d]\n",progWork->condition[0]);
  MUS_TPrintf("Cute   [%3d]\n",progWork->condition[1]);
  MUS_TPrintf("Elegant[%3d]\n",progWork->condition[2]);
  MUS_TPrintf("Unique [%3d]\n",progWork->condition[3]);
  MUS_TPrintf("--FinalCondition(CommSync!)--\n");
}

u32 MUSICAL_PROGRAM_GetNpcArr( MUSICAL_PROGRAM_WORK* progWork )
{
  u32 npcArr = (progWork->npcArr[0])+ 
               (progWork->npcArr[1]<<8)+ 
               (progWork->npcArr[2]<<16);
  return npcArr;
}
void MUSICAL_PROGRAM_SetNpcArr( MUSICAL_PROGRAM_WORK* progWork , const u32 npcArr )
{
  progWork->npcArr[0] = (npcArr&0x000000FF);
  progWork->npcArr[1] = (npcArr&0x0000FF00)>>8;
  progWork->npcArr[2] = (npcArr&0x00FF0000)>>16;

  MUS_TPrintf("--NPC Arr--\n");
  MUS_TPrintf("[%d][%d]\n",0,progWork->npcArr[0]);
  MUS_TPrintf("[%d][%d]\n",1,progWork->npcArr[1]);
  MUS_TPrintf("[%d][%d]\n",2,progWork->npcArr[2]);
  MUS_TPrintf("--NPC Arr--\n");
}

//--------------------------------------------------------------
//	最高コンディションの取得
//--------------------------------------------------------------
const u8 MUSICAL_PROGRAM_GetMaxConditionType( MUSICAL_PROGRAM_WORK* progWork )
{
  u8 i;
  u8 maxPoint = 0;
  u8 maxIdx = 0;
  for( i=0;i<MCT_MAX;i++ )
  {
    if( maxPoint < progWork->condition[i] )
    {
      maxPoint = progWork->condition[i];
      maxIdx = i;
    }
  }
  return maxIdx;
}

//--------------------------------------------------------------
//	ショーパート用NPC作成
//--------------------------------------------------------------
void MUSICAL_PROGRAM_SetData_NPC( MUSICAL_PROGRAM_WORK* progWork , STAGE_INIT_WORK *actInitWork , const u8 musicalIdx , const u8 NPCIdx , HEAPID heapId )
{
  u8 i;
  const MUSICAL_PROGRAM_POKE_DATA *pokeData = &progWork->progData->pokeData[progWork->npcArr[NPCIdx]];
  //適当な数値をでっち上げる
  u32 perRand = pokeData->trainerType + (pokeData->trainerName<<8) + (pokeData->trainerSex<<16)  + (pokeData->pokeSex<<24);
  MUSICAL_STAGE_SetData_NPC( actInitWork , musicalIdx , pokeData->monsno , pokeData->pokeSex , perRand , pokeData->appealTime , heapId );
#if PM_DEBUG
  {
    MUSICAL_DEBUG_MENU_WORK *debWork = MUSICAL_DEBUG_GetWork();
    if( debWork != NULL && 
        (debWork->memMaxMode == TRUE ||
         debWork->npcNoEquip == TRUE ))
    {
      return;
    }
  }
#endif
  
  for( i=0;i<MUS_PROG_DATA_EQUIP_NUM;i++ )
  {
    if( pokeData->equipData[i].itemNo != MUS_PROG_DATA_EQUIP_RANDOM && 
        pokeData->equipData[i].itemNo != MUS_PROG_DATA_EQUIP_NONE )
    {
      MUSICAL_STAGE_SetEquip( actInitWork , musicalIdx , pokeData->equipData[i].itemPos , pokeData->equipData[i].itemNo , 0 , i );
    }
  }
}

//--------------------------------------------------------------
//	BG番号取得
//--------------------------------------------------------------
const u8 MUSICAL_PROGRAM_GetBgNo( MUSICAL_PROGRAM_WORK* progWork )
{
  return progWork->progData->bgNo;
}

//--------------------------------------------------------------
//	ご褒美種類
//--------------------------------------------------------------
const u8 MUSICAL_PROGRAM_GetRewardType( MUSICAL_PROGRAM_WORK* progWork )
{
  return progWork->progData->rewardType;
}

//--------------------------------------------------------------
//	NPC見た目取得
//--------------------------------------------------------------
const u8 MUSICAL_PROGRAM_GetNpcObjId( MUSICAL_PROGRAM_WORK* progWork , const u8 NPCIdx )
{
  return progWork->progData->pokeData[ progWork->npcArr[NPCIdx] ].trainerType;
}

//--------------------------------------------------------------
//	NPC名前
//--------------------------------------------------------------
const u8 MUSICAL_PROGRAM_GetNpcNameIdx( MUSICAL_PROGRAM_WORK* progWork , const u8 NPCIdx )
{
  return progWork->progData->pokeData[ progWork->npcArr[NPCIdx] ].trainerName;
}
//--------------------------------------------------------------
//	NPCMonsNo
//--------------------------------------------------------------
const u16 MUSICAL_PROGRAM_GetNpcMonsno( MUSICAL_PROGRAM_WORK* progWork , const u8 NPCIdx )
{
  return progWork->progData->pokeData[ progWork->npcArr[NPCIdx] ].monsno;
}

//--------------------------------------------------------------
//	コンディションの単体ポイントを取得(ポイント／全体装備個数
//--------------------------------------------------------------
const u8 MUSICAL_PROGRAM_GetConOnePoint( MUSICAL_PROGRAM_WORK* progWork , const u8 condition )
{
  return progWork->conOnePoint[condition];
}
