//======================================================================
/**
 * @file  musical_system.h
 * @brief ミュージカル用 汎用定義
 * @author  ariizumi
 * @data  09/02/10
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#include "savedata/musical_save.h"
#include "musical/musical_system.h"
#include "musical/musical_dressup_sys.h"
#include "musical/musical_stage_sys.h"
#include "musical/stage/sta_acting.h"
#include "musical/dressup/dup_local_def.h"
#include "poke_tool/monsno_def.h"

//======================================================================
//  define
//======================================================================
#pragma mark [> define
FS_EXTERN_OVERLAY(musical);

//======================================================================
//  enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  MPS_INIT_DRESSUP,
  MPS_LOOP_DRESSUP,
  MPS_TERM_DRESSUP,
  MPS_INIT_ACTING,
  MPS_LOOP_ACTING,
  MPS_TERM_ACTING,
}MUSICAL_PROC_STATE;

//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  MUSICAL_PROC_STATE state;
  DRESSUP_INIT_WORK *dupInitWork;
  STAGE_INIT_WORK *actInitWork;
  
  MUSICAL_POKE_PARAM *musPoke;
}MUSICAL_PROC_WORK;
//======================================================================
//  proto
//======================================================================
#pragma mark [> proto
static GFL_PROC_RESULT MusicalProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk );


GFL_PROC_DATA Musical_ProcData =
{
  MusicalProc_Init,
  MusicalProc_Main,
  MusicalProc_Term
};



static const u16 musPokeArr[]=
{
  MONSNO_PIKATYUU,
  MONSNO_RAITYUU,
  MONSNO_PIPPI,
  MONSNO_PIKUSII,
  MONSNO_PURIN,
  MONSNO_EREBUU,
  494,
  495,
  0xFFFF,
};
//ミュージカルの参加資格があるか調べる(仮)
const BOOL  MUSICAL_SYSTEM_CheckEntryMusicalPokeNo( const u16 mons_no )
{
  u16 i=0;
  while( 0xFFFF != musPokeArr[i] )
  {
    if( mons_no == musPokeArr[i] )
    {
      return TRUE;
    }
    i++;
  }
  return FALSE;
}

const BOOL  MUSICAL_SYSTEM_CheckEntryMusical( POKEMON_PARAM *pokePara )
{
  u16 mons_no = PP_Get( pokePara, ID_PARA_monsno, NULL );
  return MUSICAL_SYSTEM_CheckEntryMusicalPokeNo( mons_no );
}

//ミュージカルでの番号に変換
const u16 MUSICAL_SYSTEM_ChangeMusicalPokeNumber( POKEMON_PARAM *pokePara )
{
  u16 i=0;
  int mons_no = PP_Get( pokePara, ID_PARA_monsno, NULL );

  while( 0xFFFF != musPokeArr[i] )
  {
    if( mons_no == musPokeArr[i] )
    {
      return i;
    }
    i++;
  }
  //一応最終番号に変換
  return i-1;
}

//ミュージカル用パラメータの初期化
MUSICAL_POKE_PARAM* MUSICAL_SYSTEM_InitMusPoke( POKEMON_PARAM *pokePara , HEAPID heapId )
{
  int i;
  MUSICAL_POKE_PARAM *musPara;
  musPara = GFL_HEAP_AllocMemory( heapId , sizeof(MUSICAL_POKE_PARAM) );
  musPara->charaType = MUS_CHARA_INVALID;
  musPara->pokePara = pokePara;
  
  for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
  {
    musPara->equip[i].itemNo = MUSICAL_ITEM_INVALID;
    musPara->equip[i].angle = 0;
  }
  
  return musPara;
}



#pragma mark [>proc 
static GFL_PROC_RESULT MusicalProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUSICAL_INIT_WORK *initWork = pwk;
  MUSICAL_PROC_WORK *work;
  *seq = MPS_INIT_DRESSUP;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MUSICAL_PROC|HEAPDIR_MASK, 0x4000 );

  work = GFL_PROC_AllocWork( proc, sizeof(MUSICAL_PROC_WORK), HEAPID_MUSICAL_PROC );
  work->musPoke = MUSICAL_SYSTEM_InitMusPoke( initWork->pokePara , HEAPID_MUSICAL_PROC );
  work->dupInitWork = NULL;
  work->actInitWork = NULL;
  return GFL_PROC_RES_FINISH;
  
}

static GFL_PROC_RESULT MusicalProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUSICAL_INIT_WORK *initWork = pwk;
  MUSICAL_PROC_WORK *work = mywk;
  if( work->dupInitWork != NULL )
  {
    MUSICAL_DRESSUP_DeleteInitWork( work->dupInitWork );
  }
  if( work->actInitWork != NULL )
  {
    MUSICAL_STAGE_DeleteStageWork( work->actInitWork );
  }
  GFL_HEAP_FreeMemory( work->musPoke );
  
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_MUSICAL_PROC );

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MusicalProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUSICAL_INIT_WORK *initWork = pwk;
  MUSICAL_PROC_WORK *work = mywk;
  switch( *seq )
  {
  case MPS_INIT_DRESSUP:
    work->dupInitWork = MUSICAL_DRESSUP_CreateInitWork( HEAPID_MUSICAL_PROC , work->musPoke , initWork->saveCtrl );
    GFL_PROC_SysCallProc( FS_OVERLAY_ID(musical), &DressUp_ProcData, work->dupInitWork );
    *seq = MPS_TERM_DRESSUP;
    break;

  case MPS_LOOP_DRESSUP:
    break;

  case MPS_TERM_DRESSUP:
    *seq = MPS_INIT_ACTING;
    break;

  case MPS_INIT_ACTING:
    {
      u8 ePos,i;
      work->actInitWork = MUSICAL_STAGE_CreateStageWork( HEAPID_MUSICAL_PROC );
      MUSICAL_STAGE_SetData_Player( work->actInitWork , 0 , work->musPoke );
      MUSICAL_STAGE_SetData_NPC( work->actInitWork , 1 , MONSNO_RAITYUU , HEAPID_MUSICAL_PROC );
      MUSICAL_STAGE_SetData_NPC( work->actInitWork , 2 , MONSNO_EREBUU  , HEAPID_MUSICAL_PROC );
      MUSICAL_STAGE_SetData_NPC( work->actInitWork , 3 , MONSNO_RUKARIO , HEAPID_MUSICAL_PROC );
      /*
      for( i=0;i<MUSICAL_ITEM_EQUIP_MAX;i++ )
      {
        MUSICAL_EQUIP_SAVE *equipSave = MUSICAL_SAVE_GetBefEquipData( work->dupInitWork->mus_save );
        MUSICAL_EQUIP_ONE_SAVE *equip = &equipSave->equipData[i];
        //MUSICAL_POKE_PARAM *pokePara = &work->actInitWork->musPoke[0];
        if( equip->pos != MUS_POKE_EQU_INVALID )
        {
          MUSICAL_STAGE_SetEquip( work->actInitWork , 0 , equip->pos , equip->data.itemNo , equip->data.angle );
        }
      }
      */
      MUSICAL_STAGE_SetEquip( work->actInitWork , 1 , MUS_POKE_EQU_EAR_L , 7 , 0 );
      MUSICAL_STAGE_SetEquip( work->actInitWork , 1 , MUS_POKE_EQU_BODY  , 9 , 0 );

      MUSICAL_STAGE_SetEquip( work->actInitWork , 2 , MUS_POKE_EQU_HAND_R , 31 , 0 );
      MUSICAL_STAGE_SetEquip( work->actInitWork , 2 , MUS_POKE_EQU_HEAD   , 15 , 0 );

      MUSICAL_STAGE_SetEquip( work->actInitWork , 3 , MUS_POKE_EQU_HAND_R , 30 , 0 );
      MUSICAL_STAGE_SetEquip( work->actInitWork , 3 , MUS_POKE_EQU_HEAD   , 21 , 0 );
    }
    GFL_PROC_SysCallProc( FS_OVERLAY_ID(musical), &MusicalStage_ProcData, work->actInitWork );
    *seq = MPS_TERM_ACTING;
    break;

  case MPS_LOOP_ACTING:
    break;

  case MPS_TERM_ACTING:
    return GFL_PROC_RES_FINISH;
    break;
  }
  return GFL_PROC_RES_CONTINUE;
}

