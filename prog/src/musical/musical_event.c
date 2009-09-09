//======================================================================
/**
 * @file  musical_event.c
 * @brief ミュージカル管理イベント
 * @author  ariizumi
 * @data  09/09/08
 *
 * モジュール名：MUSICAL_
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_comm.h"
#include "poke_tool/monsno_def.h"
#include "field/fieldmap.h"
#include "field/event_fieldmap_control.h"
#include "savedata/save_control.h"
#include "savedata/musical_save.h"
#include "musical/musical_system.h"
#include "musical/musical_local.h"
#include "musical/musical_program.h"
#include "musical/musical_dressup_sys.h"
#include "musical/musical_stage_sys.h"
#include "musical/comm/mus_comm_lobby.h"
#include "musical/comm/mus_comm_func.h"
#include "musical/stage/sta_acting.h"
#include "musical/stage/sta_local_def.h"
#include "musical/dressup/dup_local_def.h"

#include "musical_event.h"

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
  MES_EXIT_FIELD,
  
  MES_INIT_MUSICAL,
  MES_TERM_MUSICAL,
  
  MES_INIT_DRESSUP,
  MES_TERM_DRESSUP,

  MES_INIT_ACTING,
  MES_TERM_ACTING,

  MES_RETURN_FIELD,
  MES_FINIHS_EVENT,
}MUSICAL_EVENT_STATE;



//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  GAMESYS_WORK *gsys;
  FIELD_MAIN_WORK *fieldWork;

  MUSICAL_EVENT_STATE state;
  u8 subSeq;

  
  BOOL          isComm;
  POKEMON_PARAM *pokePara;
  SAVE_CONTROL_WORK *saveCtrl;
  GAME_COMM_SYS_PTR gameComm;
  
  MUSICAL_DISTRIBUTE_DATA *distData;
  MUS_COMM_WORK     *commWork;
  DRESSUP_INIT_WORK *dupInitWork;
  STAGE_INIT_WORK *actInitWork;
  
  MUSICAL_PROGRAM_WORK *progWork;
  MUSICAL_POKE_PARAM *musPoke;

}MUSICAL_EVENT_WORK;


//======================================================================
//  proto
//======================================================================
#pragma mark [> proto
static GMEVENT_RESULT MUSICAL_MainEvent( GMEVENT *event, int *seq, void *work );
static const BOOL MUSICAL_EVENT_InitField( GMEVENT *event, MUSICAL_EVENT_WORK *evWork );
static const BOOL MUSICAL_EVENT_ExitField( GMEVENT *event, MUSICAL_EVENT_WORK *evWork );

static void MUSICAL_EVENT_InitMusical( MUSICAL_EVENT_WORK *evWork );
static void MUSICAL_EVENT_TermMusical( MUSICAL_EVENT_WORK *evWork );

static void MUSICAL_EVENT_InitDressUp( MUSICAL_EVENT_WORK *evWork );
static void MUSICAL_EVENT_TermDressUp( MUSICAL_EVENT_WORK *evWork );

static void MUSICAL_EVENT_InitActing( MUSICAL_EVENT_WORK *evWork );
static void MUSICAL_EVENT_TermActing( MUSICAL_EVENT_WORK *evWork );
//--------------------------------------------------------------
//  イベント作成
//--------------------------------------------------------------
GMEVENT* MUSICAL_CreateEvent( GAMESYS_WORK * gsys , GAMEDATA *gdata , FIELD_MAIN_WORK *fieldWork , const BOOL isComm )
{
  GMEVENT *event;
  MUSICAL_EVENT_WORK *evWork;
  
  event = GMEVENT_Create(
      gsys, NULL, MUSICAL_MainEvent, sizeof(MUSICAL_EVENT_WORK) );
  evWork = GMEVENT_GetEventWork( event );
  
  evWork->gsys = gsys;
  evWork->fieldWork = fieldWork;
  evWork->isComm = isComm;

  evWork->saveCtrl = GAMEDATA_GetSaveControlWork( gdata );
  evWork->gameComm = GAMESYSTEM_GetGameCommSysPtr( gsys );
  //FIXME 仮生成処理
  evWork->pokePara = PP_Create(
      MONSNO_PIKUSII , 20, PTL_SETUP_POW_AUTO , HEAPID_PROC );

  evWork->dupInitWork = NULL;
  evWork->actInitWork = NULL;

  evWork->state = MES_EXIT_FIELD;
  evWork->subSeq = 0;

  return event;
}

//--------------------------------------------------------------
//  イベントメイン
//--------------------------------------------------------------
static GMEVENT_RESULT MUSICAL_MainEvent( GMEVENT *event, int *seq, void *work )
{
  MUSICAL_EVENT_WORK *evWork = work;
  switch( evWork->state )
  {
  //会場から抜ける
  case MES_EXIT_FIELD:
    {
      const BOOL isFinish = MUSICAL_EVENT_ExitField( event , evWork );
      if( isFinish == TRUE )
      {
        evWork->subSeq = 0;
        evWork->state = MES_INIT_MUSICAL;
      }
    }
    break;

  //最初の初期化
  case MES_INIT_MUSICAL:
    {
      MUSICAL_EVENT_InitMusical( evWork );
      //本当は待合室へ
      evWork->state = MES_INIT_DRESSUP;
    }
    break;

  case MES_INIT_DRESSUP:
    MUSICAL_EVENT_InitDressUp( evWork );
    GAMESYSTEM_CallProc( evWork->gsys , NO_OVERLAY_ID, &DressUp_ProcData, evWork->dupInitWork );
    evWork->state = MES_TERM_DRESSUP;
    break;
  
  case MES_TERM_DRESSUP:
    if( GAMESYSTEM_IsProcExists(evWork->gsys) == FALSE )
    {
      MUSICAL_EVENT_TermDressUp( evWork );
      //本当は待合室へ
      evWork->state = MES_INIT_ACTING;
    }
    break;
    
  case MES_INIT_ACTING:
    MUSICAL_EVENT_InitActing( evWork );
    GAMESYSTEM_CallProc( evWork->gsys , NO_OVERLAY_ID, &MusicalStage_ProcData, evWork->actInitWork );
    evWork->state = MES_TERM_ACTING;
    break;
  
  case MES_TERM_ACTING:
    if( GAMESYSTEM_IsProcExists(evWork->gsys) == FALSE )
    {
      MUSICAL_EVENT_TermActing( evWork );
      //本当は待合室へ
      evWork->state = MES_TERM_MUSICAL;
    }
    break;
  
  case MES_TERM_MUSICAL:
    MUSICAL_EVENT_TermMusical( evWork );
    evWork->state = MES_RETURN_FIELD;
    break;
  
  case MES_RETURN_FIELD:
    {
      const BOOL isFinish = MUSICAL_EVENT_InitField( event , evWork );
      if( isFinish == TRUE )
      {
        evWork->subSeq = 0;
        evWork->state = MES_FINIHS_EVENT;
      }
    }
    break;
  
  case MES_FINIHS_EVENT:
    //FIXME 仮生成処理
    GFL_HEAP_FreeMemory( evWork->pokePara );
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}


#pragma mark [>MusicalFunc
//--------------------------------------------------------------
//  ミュージカルの初期化
//--------------------------------------------------------------
static void MUSICAL_EVENT_InitMusical( MUSICAL_EVENT_WORK *evWork )
{
  GFL_OVERLAY_Load(FS_OVERLAY_ID(musical));
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MUSICAL_STRM|HEAPDIR_MASK, 0x80000 );
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MUSICAL_PROC|HEAPDIR_MASK, 0x8000 );

  evWork->musPoke = MUSICAL_SYSTEM_InitMusPoke( evWork->pokePara , HEAPID_MUSICAL_PROC );
  evWork->progWork = MUSICAL_PROGRAM_GetProgramData( HEAPID_MUSICAL_PROC );
  evWork->distData = MUSICAL_SYSTEM_InitDistributeData( HEAPID_MUSICAL_PROC );
  evWork->commWork = NULL;
  MUSICAL_SYSTEM_LoadStrmData( evWork->distData , HEAPID_MUSICAL_STRM );
}

//--------------------------------------------------------------
//  ミュージカルの開放
//--------------------------------------------------------------
static void MUSICAL_EVENT_TermMusical( MUSICAL_EVENT_WORK *evWork )
{
  if( evWork->commWork != NULL )
  {
    MUS_COMM_DeleteWork( evWork->commWork );
    evWork->commWork = NULL;
  }
  if( evWork->dupInitWork != NULL )
  {
    MUSICAL_DRESSUP_DeleteInitWork( evWork->dupInitWork );
  }
  if( evWork->actInitWork != NULL )
  {
    MUSICAL_STAGE_DeleteStageWork( evWork->actInitWork );
  }
  MUSICAL_SYSTEM_TermDistributeData( evWork->distData );
  GFL_HEAP_FreeMemory( evWork->musPoke );
  
  GFL_HEAP_FreeMemory( evWork->progWork );
  GFL_HEAP_DeleteHeap( HEAPID_MUSICAL_STRM );
  GFL_HEAP_DeleteHeap( HEAPID_MUSICAL_PROC );

  GFL_OVERLAY_Unload(FS_OVERLAY_ID(musical));
  
    
}

//--------------------------------------------------------------
//  ドレスアップの初期化
//--------------------------------------------------------------
static void MUSICAL_EVENT_InitDressUp( MUSICAL_EVENT_WORK *evWork )
{
  evWork->dupInitWork = MUSICAL_DRESSUP_CreateInitWork( HEAPID_MUSICAL_PROC , evWork->musPoke , evWork->saveCtrl );
  evWork->dupInitWork->commWork = evWork->commWork;
}

//--------------------------------------------------------------
//  ドレスアップの開放
//--------------------------------------------------------------
static void MUSICAL_EVENT_TermDressUp( MUSICAL_EVENT_WORK *evWork )
{
  //現在処理無し
}

//--------------------------------------------------------------
//  ショーパートの初期化
//--------------------------------------------------------------
static void MUSICAL_EVENT_InitActing( MUSICAL_EVENT_WORK *evWork )
{
  {
    u8 ePos,i;
    OS_TPrintf("FreeHeap:[%x][%x]\n", 
        GFL_HEAP_GetHeapFreeSize( GFL_HEAPID_APP ) ,
        GFI_HEAP_GetHeapAllocatableSize( GFL_HEAPID_APP ) );
    evWork->actInitWork = MUSICAL_STAGE_CreateStageWork( HEAPID_MUSICAL_PROC , evWork->commWork );
    MUSICAL_STAGE_SetData_Player( evWork->actInitWork , 1 , evWork->musPoke );
    MUSICAL_STAGE_SetData_NPC( evWork->actInitWork , 0 , MONSNO_ONOKKUSU , HEAPID_MUSICAL_PROC );
    MUSICAL_STAGE_SetData_NPC( evWork->actInitWork , 2 , MONSNO_PIKATYUU  , HEAPID_MUSICAL_PROC );
    MUSICAL_STAGE_SetData_NPC( evWork->actInitWork , 3 , MONSNO_REIBAAN , HEAPID_MUSICAL_PROC );

    MUSICAL_STAGE_SetEquip( evWork->actInitWork , 0 , MUS_POKE_EQU_EAR_L , 7 , 0 );
    MUSICAL_STAGE_SetEquip( evWork->actInitWork , 0 , MUS_POKE_EQU_HEAD  ,15 , 0 );
    MUSICAL_STAGE_SetEquip( evWork->actInitWork , 0 , MUS_POKE_EQU_HAND_L,13 , 0 );

    MUSICAL_STAGE_SetEquip( evWork->actInitWork , 2 , MUS_POKE_EQU_EAR_R  ,  2 , 0 );
    MUSICAL_STAGE_SetEquip( evWork->actInitWork , 2 , MUS_POKE_EQU_EYE    , 11 , 0 );
    MUSICAL_STAGE_SetEquip( evWork->actInitWork , 2 , MUS_POKE_EQU_BODY   , 27 , 0 );

    MUSICAL_STAGE_SetEquip( evWork->actInitWork , 3 , MUS_POKE_EQU_FACE   , 21 , 0 );
    MUSICAL_STAGE_SetEquip( evWork->actInitWork , 3 , MUS_POKE_EQU_HAND_R , 30 , 0 );
  }
  evWork->actInitWork->distData = evWork->distData;
  evWork->actInitWork->progWork = evWork->progWork;
  MUSICAL_PROGRAM_CalcPokemonPoint( HEAPID_MUSICAL_PROC , evWork->progWork , evWork->actInitWork );
}

//--------------------------------------------------------------
//  ショーパートの開放
//--------------------------------------------------------------
static void MUSICAL_EVENT_TermActing( MUSICAL_EVENT_WORK *evWork )
{
  //現在処理無し
}

#pragma mark [>FieldFunc
//--------------------------------------------------------------
//  フィールドを呼ぶ
//--------------------------------------------------------------
static const BOOL MUSICAL_EVENT_InitField( GMEVENT *event, MUSICAL_EVENT_WORK *evWork )
{
  switch( evWork->subSeq )
  {
  case 0:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(evWork->gsys));
    evWork->subSeq++;
    break;
  case 1:
    GMEVENT_CallEvent(event, EVENT_FieldFadeIn(evWork->gsys, evWork->fieldWork, 0));
    evWork->subSeq++;
    break;
  case 2:
    return TRUE;
    break;
  }
  return FALSE;
}

//--------------------------------------------------------------
//  フィールドを抜ける
//--------------------------------------------------------------
static const BOOL MUSICAL_EVENT_ExitField( GMEVENT *event, MUSICAL_EVENT_WORK *evWork )
{
  switch( evWork->subSeq )
  {
  case 0:
    GMEVENT_CallEvent(event, EVENT_FieldFadeOut(evWork->gsys, evWork->fieldWork, 0));
    evWork->subSeq++;
    break;
  case 1:
    GMEVENT_CallEvent(event, EVENT_FieldClose(evWork->gsys, evWork->fieldWork));
    evWork->subSeq++;
    break;
  case 2:
    return TRUE;
    break;
  }
  return FALSE;
}


