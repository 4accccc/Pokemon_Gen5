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

#include "arc/fieldmap/zone_id.h"
#include "arc/fieldmap/fldmmdl_objcode.h"
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_comm.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/natsuki.h"
#include "field/fieldmap.h"
#include "field/event_fieldmap_control.h"
#include "field/event_mapchange.h"
#include "field/script.h"
#include "field/script_local.h"
#include "field/field_sound.h"
#include "savedata/save_control.h"
#include "savedata/musical_save.h"
#include "savedata/record.h"
#include "savedata/etc_save.h"
#include "sound/pm_sndsys.h"
#include "musical/musical_system.h"
#include "musical/musical_local.h"
#include "musical/musical_program.h"
#include "musical/musical_dressup_sys.h"
#include "musical/musical_stage_sys.h"
#include "musical/musical_shot_sys.h"
#include "musical/comm/mus_comm_lobby.h"
#include "musical/comm/mus_comm_func.h"
#include "musical/stage/sta_acting.h"
#include "musical/stage/sta_local_def.h"
#include "musical/dressup/dup_local_def.h"
#include "test/ariizumi/ari_debug.h"

#include "message.naix"

#include "../resource/fldmapdata/script/c04r0202_def.h"
#include "../resource/fldmapdata/script/musical_scr_def.h"
#include "../resource/fldmapdata/script/musical_scr_local.h"

#include "musical/musical_event.h"

#include "debug/debug_flg.h"
#include "musical/musical_debug_menu.h"
#include "musical_reward_table.cdat" //景品テーブル
//======================================================================
//  define
//======================================================================
#pragma mark [> define

FS_EXTERN_OVERLAY(musical);

#define HEAPID_PROC_WRAPPER  (HEAPID_PROC)


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

  MES_INIT_MUSICAL_SHOT,
  MES_TERM_MUSICAL_SHOT,

  MES_CALL_HALL_EVENT,
  MES_FINIHS_EVENT,     //10

  MES_ENTER_WAITROOM_FIRST_BEF_COMM,
  MES_ENTER_WAITROOM_FIRST_BEF_COMM2,
  MES_ENTER_WAITROOM_FIRST,
  MES_WAITROOM_FIRST,
  MES_EXIT_WAITROOM_FIRST,

  MES_ENTER_WAITROOM_SECOND,
  MES_WAITROOM_SECOND,
  MES_EXIT_WAITROOM_SECOND,

  MES_ENTER_WAITROOM_THIRD,
  MES_WAITROOM_THIRD,   //20
  MES_EXIT_WAITROOM_THIRD,

  MES_ERROR_INIT_OPEN_FIELD,
  MES_ERROR_INIT,
  MES_ERROR_RETURN_HALL,
  MES_ERROR_REQ_DISP,
  MES_ERROR_RETURN_SCRIPT,
}MUSICAL_EVENT_STATE;



//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct
struct _MUSICAL_EVENT_WORK
{
  GAMESYS_WORK *gsys;
  GAMEDATA *gameData;
  MUSICAL_SCRIPT_WORK *scriptWork;

  MUSICAL_EVENT_STATE state;
  u8 subSeq;
  
  BOOL          isComm;
  POKEMON_PARAM *pokePara;
  SAVE_CONTROL_WORK *saveCtrl;    //消してGAMEDATA使う!
  GAME_COMM_SYS_PTR gameComm;
  
  MUSICAL_DISTRIBUTE_DATA *distData;
  MUS_COMM_WORK     *commWork;
  DRESSUP_INIT_WORK *dupInitWork;
  STAGE_INIT_WORK *actInitWork;
  MUS_SHOT_INIT_WORK *shotInitWork;
  
  MUSICAL_PROGRAM_WORK *progWork;
  MUSICAL_POKE_PARAM *musPoke;

  MUSICAL_SAVE *musSave;

  u8 musicalIndex[MUSICAL_POKE_MAX];  //キャラのIDXに対応した立ち位置
  u8 rankIndex[MUSICAL_POKE_MAX];     //参加番号が順位順で入っている
  u8 selfIdx;

  BOOL isNetErr;
};


//======================================================================
//  proto
//======================================================================
#pragma mark [> proto
static GMEVENT_RESULT MUSICAL_MainEvent( GMEVENT *event, int *seq, void *work );

static void MUSICAL_EVENT_InitMusical( MUSICAL_EVENT_WORK *evWork );
static void MUSICAL_EVENT_TermMusical( MUSICAL_EVENT_WORK *evWork );
static void MUSICAL_EVENT_InitDressUp( MUSICAL_EVENT_WORK *evWork );
static void MUSICAL_EVENT_TermDressUp( MUSICAL_EVENT_WORK *evWork );
static void MUSICAL_EVENT_InitActing( MUSICAL_EVENT_WORK *evWork );
static void MUSICAL_EVENT_TermActing( MUSICAL_EVENT_WORK *evWork );
static void MUSICAL_EVENT_InitMusicalShot( MUSICAL_EVENT_WORK *evWork );
static void MUSICAL_EVENT_SetSaveData( MUSICAL_EVENT_WORK *evWork );
static void MUSICAL_EVENT_CalcFanState( MUSICAL_EVENT_WORK *evWork );

static const BOOL MUSICAL_EVENT_InitField( GMEVENT *event, MUSICAL_EVENT_WORK *evWork );
static const BOOL MUSICAL_EVENT_ExitField( GMEVENT *event, MUSICAL_EVENT_WORK *evWork );
static const void MUSICAL_EVENT_JumpWaitingRoom( GMEVENT *event, MUSICAL_EVENT_WORK *evWork );
static const void MUSICAL_EVENT_JumpMusicalHall( GMEVENT *event, MUSICAL_EVENT_WORK *evWork );
static const void MUSICAL_EVENT_RunScript( GMEVENT *event, MUSICAL_EVENT_WORK *evWork , u16 scriptId );
static const u8 MUSICAL_EVENT_CalcNpcIdx( MUSICAL_EVENT_WORK *evWork , const u8 idx );

//--------------------------------------------------------------
//  イベント作成
//--------------------------------------------------------------
GMEVENT* MUSICAL_CreateEvent( GAMESYS_WORK * gsys , GAMEDATA *gdata , const u8 pokeIdx , const BOOL isComm , MUSICAL_SCRIPT_WORK *scriptWork )
{
  GMEVENT *event;
  MUSICAL_EVENT_WORK *evWork;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, GFL_HEAP_LOWID(HEAPID_MUSICAL_STRM) , 0x42000 );

  MUS_TPrintf("CreateMusicalEvent[%d]\n",isComm);
  
  event = GMEVENT_Create(
      gsys, NULL, MUSICAL_MainEvent, sizeof(MUSICAL_EVENT_WORK) );
  evWork = GMEVENT_GetEventWork( event );
  
  evWork->gsys = gsys;
  evWork->gameData = gdata;
  evWork->isComm = isComm;

  evWork->saveCtrl = GAMEDATA_GetSaveControlWork( gdata );
  evWork->gameComm = GAMESYSTEM_GetGameCommSysPtr( gsys );
  evWork->scriptWork = scriptWork;
  evWork->scriptWork->eventWork = evWork;
  evWork->commWork = scriptWork->commWork;
  evWork->isNetErr = FALSE;
  evWork->progWork = NULL;
  
  {
    STRBUF *str = GFL_STR_CreateBuffer( SAVELEN_POKEMON_NAME+EOM_SIZE , HEAPID_PROC_WRAPPER );
    POKEPARTY *party = GAMEDATA_GetMyPokemon( evWork->gameData );
    evWork->pokePara = PokeParty_GetMemberPointer( party , pokeIdx );
    //ビーコン処理
    PP_Get( evWork->pokePara , ID_PARA_nickname , str );
    GAMEBEACON_Set_Musical( str );
    GFL_STR_DeleteBuffer( str );
  }

  evWork->dupInitWork = NULL;
  evWork->actInitWork = NULL;
  evWork->shotInitWork = NULL;
  evWork->musSave = GAMEDATA_GetMusicalSavePtr( gdata );
  
  evWork->distData = MUSICAL_SYSTEM_InitDistributeData( HEAPID_PROC_WRAPPER );
  if( evWork->isComm == FALSE || 
      MUS_COMM_GetMode( evWork->scriptWork->commWork ) == MCM_PARENT )
  {
    MUSICAL_SYSTEM_LoadDistributeData( evWork->distData , evWork->saveCtrl , evWork->gameData, MUSICAL_SAVE_GetProgramNumber(evWork->musSave) , HEAPID_MUSICAL_STRM );
  }
  
  if( evWork->isComm == TRUE )
  {
    evWork->state = MES_ENTER_WAITROOM_FIRST_BEF_COMM;
    evWork->subSeq = 0;
    MUS_COMM_InitMusical( evWork->commWork , 
                          GAMEDATA_GetMyStatus(evWork->gameData) , 
                          PP_GetPPPPointer( evWork->pokePara ) , 
                          evWork->gameComm , 
                          evWork->distData , 
                          HEAPID_PROC_WRAPPER );
  }
  else
  {
    //ミュージカルの配置入れ替え
    //通信時は別で行ったものを取得する
    {
      u8 i,j;
      const u16 sumPoint = MUSICAL_SAVE_GetSumPoint(evWork->musSave);
#if PM_DEBUG
      MUSICAL_DEBUG_MENU_WORK *debWork = MUSICAL_DEBUG_GetWork();
      if( debWork != NULL && 
          debWork->enableArr == TRUE )
      {
        for( i=0;i<MUSICAL_POKE_MAX;i++ )
        {
          evWork->musicalIndex[i] = debWork->arr[i];
        }
      }
      else
#endif
      {
        for( i=0;i<MUSICAL_POKE_MAX;i++ )
        {
          evWork->musicalIndex[i] = i;
        }
        for( j=0;j<10;j++ )
        {
          for( i=0;i<MUSICAL_POKE_MAX;i++ )
          {
            u8 swapIdx = GFUser_GetPublicRand0(MUSICAL_POKE_MAX);
            u8 temp = evWork->musicalIndex[i];
            evWork->musicalIndex[i] = evWork->musicalIndex[swapIdx];
            evWork->musicalIndex[swapIdx] = temp;
          }
        }
      }
      evWork->selfIdx = evWork->musicalIndex[0];
      
      //マップ遷移前に演目のデータだけ必要(NPCキャラを出すため
      evWork->progWork = MUSICAL_PROGRAM_InitProgramData( HEAPID_PROC_WRAPPER , evWork->distData , sumPoint );

    }
    evWork->state = MES_ENTER_WAITROOM_FIRST;
    evWork->subSeq = 0;
  }
  

  return event;
}

//--------------------------------------------------------------
//  イベントメイン
//--------------------------------------------------------------
static GMEVENT_RESULT MUSICAL_MainEvent( GMEVENT *event, int *seq, void *work )
{
  MUSICAL_EVENT_WORK *evWork = work;
  
#if defined(DEBUG_ONLY_FOR_ariizumi_nobuhiko)
  {
    static u8 befState = 0xFF;
    if( befState != evWork->state )
    {
      MUS_TPrintf("MusEvState[%d]->[%d]\n",befState,evWork->state);
      befState = evWork->state;
    }
  }
#endif
  MUSICAL_EVENT_CheckNetErr( evWork );
  
  switch( evWork->state )
  {
  case MES_ENTER_WAITROOM_FIRST_BEF_COMM:
    if( MUS_COMM_IsPostAllMyStatus( evWork->scriptWork->commWork ) == TRUE )
    {
      //マップ遷移前に演目のデータだけ必要(NPCキャラを出すため
      u32 conPointArr = 0;
      u32 npcArr = 0;
      if( MUS_COMM_GetMode( evWork->scriptWork->commWork ) == MCM_PARENT )
      {
        const u16 sumPoint = MUSICAL_SAVE_GetSumPoint(evWork->musSave);
        evWork->progWork = MUSICAL_PROGRAM_InitProgramData( HEAPID_PROC_WRAPPER , evWork->distData , sumPoint );
        conPointArr = MUSICAL_PROGRAM_GetConditionPointArr( evWork->progWork );
        npcArr = MUSICAL_PROGRAM_GetNpcArr( evWork->progWork );
      }
      MUS_COMM_StartSendProgram_Data( evWork->scriptWork->commWork , conPointArr , npcArr );
      evWork->state = MES_ENTER_WAITROOM_FIRST_BEF_COMM2;
      
      //知り合いの登録
      {
        u8 i;
        ETC_SAVE_WORK *etcSave =  SaveData_GetEtc( evWork->saveCtrl );

        for( i=0;i<MUSICAL_POKE_MAX;i++ )
        {
          const u8 selfIdx = MUS_COMM_GetSelfMusicalIndex( evWork->scriptWork->commWork );
          const u8 checkIdx = MUS_COMM_GetMusicalIndex( evWork->scriptWork->commWork , i );
          if( selfIdx != checkIdx )
          {
            MYSTATUS *myStatus = MUS_COMM_GetPlayerMyStatus( evWork->scriptWork->commWork , i );
            if( myStatus != NULL )
            {
              const u32 id = MyStatus_GetID( myStatus );
              EtcSave_SetAcquaintance( etcSave , id );
              MUS_TPrintf("set person[%d][%8x(%5d)]\n",i,id,(id&0x0000FFFF));
            }
          }
        }
      }
    }
    else
    if( evWork->isNetErr == TRUE )
    {
      evWork->state = MES_ERROR_INIT;
    }
    break;
  case MES_ENTER_WAITROOM_FIRST_BEF_COMM2:
    if( MUS_COMM_IsPostProgramData(evWork->scriptWork->commWork) == TRUE )
    {
      u8 i;
      evWork->state = MES_ENTER_WAITROOM_FIRST;
      //ここでIDXの取得
      for( i=0;i<MUSICAL_POKE_MAX;i++ )
      {
        evWork->musicalIndex[i] = MUS_COMM_GetMusicalIndex( evWork->scriptWork->commWork , i );
      }
      evWork->selfIdx = MUS_COMM_GetSelfMusicalIndex( evWork->scriptWork->commWork );
      MUSICAL_EVENT_CalcProgramData( evWork );
    }
    else
    if( evWork->isNetErr == TRUE )
    {
      evWork->state = MES_ERROR_INIT;
    }
    break;
    
  case MES_ENTER_WAITROOM_FIRST:
    //GFL_HEAP_DEBUG_PrintExistMemoryBlocks( GFL_HEAPID_APP );
    //GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_PROC );
    MUSICAL_EVENT_JumpWaitingRoom( event , evWork );
    evWork->state = MES_WAITROOM_FIRST;
    break;

  case MES_WAITROOM_FIRST:
    MUSICAL_EVENT_InitMusical( evWork );
    MUSICAL_EVENT_RunScript( event , evWork , SCRID_C04R0202_MUS_1ST );
    evWork->state = MES_EXIT_FIELD;
    break;
    
  //フィールドから抜ける
  //------------------------------
  case MES_EXIT_FIELD:
    {
      if( evWork->isNetErr == TRUE  &&
          evWork->subSeq == 0 )
      {
        //フェード中に抜けると困るので
        evWork->state = MES_ERROR_INIT;
      }
      else
      {
        const BOOL isFinish = MUSICAL_EVENT_ExitField( event , evWork );
        if( isFinish == TRUE )
        {
          evWork->subSeq = 0;
          evWork->state = MES_INIT_MUSICAL;
        }
      }
    }
    break;

  //最初の初期化
  //------------------------------
  case MES_INIT_MUSICAL:
    {
      evWork->state = MES_INIT_DRESSUP;
    }
    break;

  case MES_INIT_DRESSUP:
    MUSICAL_EVENT_InitDressUp( evWork );
    GAMESYSTEM_CallProc( evWork->gsys , NO_OVERLAY_ID, &DressUp_ProcData, evWork->dupInitWork );
    evWork->state = MES_TERM_DRESSUP;
    break;
  
  case MES_TERM_DRESSUP:
    if( GAMESYSTEM_IsProcExists(evWork->gsys) == GFL_PROC_MAIN_NULL )
    {
      MUSICAL_EVENT_TermDressUp( evWork );
      evWork->state = MES_ENTER_WAITROOM_SECOND;
    }
    break;

  //ドレスアップ→ショーパート間の控え室
  //------------------------------
  case MES_ENTER_WAITROOM_SECOND:
    {
      const BOOL isFinish = MUSICAL_EVENT_InitField( event , evWork );
      if( isFinish == TRUE )
      {
        evWork->subSeq = 0;
        evWork->state = MES_WAITROOM_SECOND;
        if( evWork->isNetErr == TRUE )
        {
          evWork->state = MES_ERROR_INIT;
        }
      }
    }
    break;
    
  case MES_WAITROOM_SECOND:
    MUSICAL_EVENT_RunScript( event , evWork , SCRID_C04R0202_MUS_2ND );
    evWork->state = MES_EXIT_WAITROOM_SECOND;
    break;
    
  case MES_EXIT_WAITROOM_SECOND:
    {
      if( evWork->isNetErr == TRUE &&
          evWork->subSeq == 0 )
      {
        //フェード中に抜けると困るので
        evWork->state = MES_ERROR_INIT;
      }
      else
      {
        const BOOL isFinish = MUSICAL_EVENT_ExitField( event , evWork );
        if( isFinish == TRUE )
        {
          evWork->subSeq = 0;
          evWork->state = MES_INIT_ACTING;
        }
      }
    }
    break;
    
  //ショーパート
  //------------------------------
  case MES_INIT_ACTING:
    {
      MUSICAL_EVENT_InitActing( evWork );
      MUS_TPrintf("InitAct[%8x][%8x][%8x]\n",evWork->gsys,&MusicalStage_ProcData,evWork->actInitWork );
      GAMESYSTEM_CallProc( evWork->gsys , NO_OVERLAY_ID, &MusicalStage_ProcData, evWork->actInitWork );
      evWork->state = MES_TERM_ACTING;
    }
    break;
  
  case MES_TERM_ACTING:
    if( GAMESYSTEM_IsProcExists(evWork->gsys) == GFL_PROC_MAIN_NULL )
    {
      MUSICAL_EVENT_TermActing( evWork );
      evWork->state = MES_INIT_MUSICAL_SHOT;
      if( evWork->isNetErr == TRUE )
      {
        //フィールド開くため
        evWork->state = MES_ERROR_INIT_OPEN_FIELD;
      }
    }
    break;

  //ミュージカルショット
  //------------------------------
  case MES_INIT_MUSICAL_SHOT:
    MUSICAL_EVENT_InitMusicalShot( evWork );
    GAMESYSTEM_CallProc( evWork->gsys , NO_OVERLAY_ID, &MusicalShot_ProcData, evWork->shotInitWork );
    evWork->state = MES_TERM_MUSICAL_SHOT;
    break;
    
  case MES_TERM_MUSICAL_SHOT:
    if( GAMESYSTEM_IsProcExists(evWork->gsys) == GFL_PROC_MAIN_NULL )
    {
      evWork->state = MES_ENTER_WAITROOM_THIRD;
      if( evWork->isNetErr == TRUE )
      {
        //フィールド開くため
        evWork->state = MES_ERROR_INIT_OPEN_FIELD;
      }
    }
    break;
    
  //最後の控え室
  //------------------------------
  case MES_ENTER_WAITROOM_THIRD:
    {
      const BOOL isFinish = MUSICAL_EVENT_InitField( event , evWork );
      if( isFinish == TRUE )
      {
        evWork->subSeq = 0;
        evWork->state = MES_WAITROOM_THIRD;
        MUSICAL_EVENT_CalcFanState( evWork );
        MUSICAL_EVENT_SetSaveData( evWork );
      }
    }
    break;
    
  case MES_WAITROOM_THIRD:
    MUSICAL_EVENT_RunScript( event , evWork , SCRID_C04R0202_MUS_3RD );
    //evWork->state = MES_EXIT_WAITROOM_THIRD;
    evWork->state = MES_TERM_MUSICAL;
    break;
    
    /*
  case MES_EXIT_WAITROOM_THIRD:
    {
      if( evWork->isNetErr == TRUE )
      {
        evWork->state = MES_ERROR_INIT;
      }
      else
      {
        const BOOL isFinish = MUSICAL_EVENT_ExitField( event , evWork );
        if( isFinish == TRUE )
        {
          evWork->subSeq = 0;
          evWork->state = MES_TERM_MUSICAL;
        }
      }
    }
    break;
    */
  //諸々の開放
  //------------------------------
  case MES_TERM_MUSICAL:
    if( evWork->isNetErr == TRUE )
    {
      evWork->state = MES_ERROR_INIT;
    }
    else
    {
      MUSICAL_EVENT_TermMusical( evWork );
      MUSICAL_EVENT_JumpMusicalHall( event , evWork );
      evWork->state = MES_CALL_HALL_EVENT;
    }
    break;
  
  case MES_CALL_HALL_EVENT:
    MUSICAL_EVENT_RunScript( event , evWork , SCRID_MUSICAL_RETURN_HALL );
    evWork->state = MES_FINIHS_EVENT;
    break;
  
  case MES_FINIHS_EVENT:
    GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_PROC );
    GFL_HEAP_DeleteHeap( HEAPID_MUSICAL_STRM );
    return GMEVENT_RES_FINISH;
    break;

  case MES_ERROR_INIT_OPEN_FIELD:
    {
      const BOOL isFinish = MUSICAL_EVENT_InitField( event , evWork );
      if( isFinish == TRUE )
      {
        evWork->state = MES_ERROR_INIT;
      }
    }
    break;
    
  case MES_ERROR_INIT:
    NetErr_App_ReqErrorDisp();
    evWork->state = MES_ERROR_RETURN_HALL;
    break;

  //エラー処理
  //------------------------------
  case MES_ERROR_RETURN_HALL:
    MUSICAL_EVENT_TermMusical( evWork );
    MUSICAL_EVENT_JumpMusicalHall( event , evWork );
    evWork->state = MES_ERROR_REQ_DISP;
    break;

  case MES_ERROR_REQ_DISP:
    evWork->state = MES_ERROR_RETURN_SCRIPT;
    break;

  case MES_ERROR_RETURN_SCRIPT:
    MUSICAL_EVENT_RunScript( event , evWork , SCRID_MUSICAL_RETURN_HALL_ERR );
    evWork->state = MES_FINIHS_EVENT;
    break;

  }
  return GMEVENT_RES_CONTINUE;
}


#pragma mark [>MusicalFunc
//--------------------------------------------------------------
//  ミュージカルの初期化
//--------------------------------------------------------------
static void MUSICAL_EVENT_InitMusical( MUSICAL_EVENT_WORK *evWork )
{
  evWork->musPoke = MUSICAL_SYSTEM_InitMusPoke( evWork->pokePara , HEAPID_PROC_WRAPPER );
  
  
  if( evWork->isComm == FALSE )
  {
  }
  else
  {
    if( MUS_COMM_GetMode( evWork->scriptWork->commWork ) == MCM_PARENT )
    {
    }
    MUS_COMM_StartSendProgram_Script( evWork->scriptWork->commWork );
  }
}

//--------------------------------------------------------------
//  ミュージカルの開放
//--------------------------------------------------------------
static void MUSICAL_EVENT_TermMusical( MUSICAL_EVENT_WORK *evWork )
{
  if( evWork->dupInitWork != NULL )
  {
    MUSICAL_DRESSUP_DeleteInitWork( evWork->dupInitWork );
  }
  if( evWork->actInitWork != NULL )
  {
    MUSICAL_STAGE_DeleteStageWork( evWork->actInitWork );
  }
  if( evWork->shotInitWork != NULL )
  {
    GFL_HEAP_FreeMemory( evWork->shotInitWork->musShotData );
    GFL_HEAP_FreeMemory( evWork->shotInitWork );
  }
  if( evWork->distData != NULL )
  {
    MUSICAL_SYSTEM_TermDistributeData( evWork->distData );
  }
  if( evWork->musPoke != NULL )
  {
    GFL_HEAP_FreeMemory( evWork->musPoke );
  }
  
  if( evWork->progWork != NULL )
  {
    MUSICAL_PROGRAM_TermProgramData( evWork->progWork );
  }
  evWork->scriptWork->eventWork = NULL;

}

//--------------------------------------------------------------
//  ドレスアップの初期化
//--------------------------------------------------------------
static void MUSICAL_EVENT_InitDressUp( MUSICAL_EVENT_WORK *evWork )
{
  evWork->dupInitWork = MUSICAL_DRESSUP_CreateInitWork( HEAPID_PROC_WRAPPER , evWork->musPoke , evWork->saveCtrl );
  evWork->dupInitWork->commWork = evWork->commWork;
}

//--------------------------------------------------------------
//  ドレスアップの開放
//--------------------------------------------------------------
static void MUSICAL_EVENT_TermDressUp( MUSICAL_EVENT_WORK *evWork )
{
  if( evWork->isComm == TRUE )
  {
    MUS_COMM_StartSendPoke( evWork->commWork , evWork->musPoke );
  }
}

//--------------------------------------------------------------
//  ショーパートの初期化
//--------------------------------------------------------------
static void MUSICAL_EVENT_InitActing( MUSICAL_EVENT_WORK *evWork )
{
  u8 i;
  evWork->actInitWork = MUSICAL_STAGE_CreateStageWork( HEAPID_PROC_WRAPPER , evWork->commWork );
  if( evWork->isComm == FALSE )
  {
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      if( i == 0 )
      {
        //プレイヤー
        MUSICAL_STAGE_SetData_Player( evWork->actInitWork , evWork->musicalIndex[i] , evWork->musPoke );
      }
      else
      {
        //NPC
        MUSICAL_PROGRAM_SetData_NPC( evWork->progWork , evWork->actInitWork , evWork->musicalIndex[i] , i-1 , HEAPID_PROC_WRAPPER );
      }
    }
  }
  else
  {
    u8 npcIdx = 0;
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      MUSICAL_POKE_PARAM *musPoke = MUS_COMM_GetMusPokeParam( evWork->commWork , evWork->musicalIndex[i] );
      if( musPoke != NULL )
      {
        MUSICAL_STAGE_SetData_Comm( evWork->actInitWork , evWork->musicalIndex[i] , musPoke );
      }
      else
      {
        MUSICAL_PROGRAM_SetData_NPC( evWork->progWork , evWork->actInitWork , evWork->musicalIndex[i] , npcIdx , HEAPID_PROC_WRAPPER );
        npcIdx++;
      }
    }
  }

  evWork->actInitWork->distData = evWork->distData;
  evWork->actInitWork->progWork = evWork->progWork;
  MUSICAL_PROGRAM_CalcPokemonPoint( HEAPID_PROC_WRAPPER , evWork->progWork , evWork->actInitWork );
}

//--------------------------------------------------------------
//  ショーパートの開放
//--------------------------------------------------------------
static void MUSICAL_EVENT_TermActing( MUSICAL_EVENT_WORK *evWork )
{
  if( evWork->isComm == TRUE )
  {
    //通信ワークからアピールボーナスの取得
    u8 i,j;
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      MUSICAL_POKE_PARAM *musPoke = evWork->actInitWork->musPoke[i];
      for( j=0;j<MUS_COMM_APPEALBONUS_NUM;j++ )
      {
        const u8 val = MUS_COMM_GetAppealBonus( evWork->commWork , i , j );
        if( val != MUS_COMM_APPEALBONUS_INVALID )
        {
          musPoke->isApeerBonus[val] = TRUE;
        }
      }
    }
  }
  //アピールボーナスの反映
  {
    MUS_ITEM_DATA_SYS *itemDataSys = MUS_ITEM_DATA_InitSystem( HEAPID_PROC );
    u8 i,j;
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      MUSICAL_POKE_PARAM *musPoke = evWork->actInitWork->musPoke[i];
      for( j=0;j<MUS_POKE_EQUIP_MAX;j++ )
      {
        if( musPoke->isApeerBonus[j] == TRUE )
        {
          const u8 conType = MUS_ITEM_DATA_GetItemConditionType( itemDataSys , musPoke->equip[j].itemNo );
          const u8 bonus = MUSICAL_PROGRAM_GetConOnePoint( evWork->progWork , conType );
          musPoke->point += bonus;
          MUS_TPrintf("AddAppealBonus[%d:%d(%d)]\n",i,bonus,musPoke->point);
        }
      }
    }
    
    MUS_ITEM_DATA_ExitSystem( itemDataSys );
  }
}

//--------------------------------------------------------------
//  ミュージカルショットの初期化
//--------------------------------------------------------------
static void MUSICAL_EVENT_InitMusicalShot( MUSICAL_EVENT_WORK *evWork )
{
  evWork->shotInitWork = GFL_HEAP_AllocMemory( HEAPID_PROC_WRAPPER , sizeof( MUS_SHOT_INIT_WORK ));
  evWork->shotInitWork->musShotData = GFL_HEAP_AllocClearMemory( HEAPID_PROC_WRAPPER , sizeof( MUSICAL_SHOT_DATA ));
  evWork->shotInitWork->isLoadOverlay = FALSE;
  evWork->shotInitWork->commWork = evWork->commWork;
  {
    u8 i;
    RTCDate date;
    MUSICAL_SHOT_DATA *shotData = evWork->shotInitWork->musShotData;
    GFL_RTC_GetDate( &date );
    shotData->bgNo = MUSICAL_PROGRAM_GetBgNo( evWork->progWork );
    shotData->year = date.year;
    shotData->month = date.month;
    shotData->day = date.day;
    {
      STRBUF * tmpBuf = MUSICAL_EVENT_CreateStr_ProgramTitle( evWork , HEAPID_PROC_WRAPPER );
      GFL_STR_GetStringCode( tmpBuf , shotData->title , MUSICAL_PROGRAM_NAME_MAX );
      GFL_STR_DeleteBuffer( tmpBuf );
    }
    
    shotData->musVer = MUSICAL_VERSION;
    shotData->pmVersion = VERSION_BLACK;
    shotData->pmLang = LANG_JAPAN;
    shotData->player = evWork->selfIdx;
    
    //スポットライトの計算
    {
      u8 maxPoint = 0;
      for( i=0;i<MUSICAL_POKE_MAX;i++ )
      {
        const u8 point = evWork->actInitWork->musPoke[i]->point;
        if( maxPoint < point )
        {
          shotData->spotBit = 0;
          maxPoint = point;
        }
        if( maxPoint == point )
        {
          shotData->spotBit += 1<<i;
        }
      }
    }
    
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      MYSTATUS *myStatus = NULL;
      u8 j;
      MUSICAL_POKE_PARAM *musPoke = evWork->actInitWork->musPoke[i];
      shotData->shotPoke[i].monsno = musPoke->mcssParam.monsno;
      shotData->shotPoke[i].sex = musPoke->mcssParam.sex;
      shotData->shotPoke[i].rare = musPoke->mcssParam.rare;
      shotData->shotPoke[i].form = musPoke->mcssParam.form;
      shotData->shotPoke[i].perRand = musPoke->mcssParam.perRand;

      if( i == evWork->selfIdx )
      {
        myStatus = GAMEDATA_GetMyStatus( evWork->gameData );
      }
      else
      if( evWork->isComm == TRUE )
      {
        u8 idx = MUSICAL_EVENT_GetPosIndex( evWork , i );
        myStatus = MUS_COMM_GetPlayerMyStatus( evWork->commWork , idx );
      }
      
      if( myStatus != NULL )
      {
        GFL_STD_MemCopy( myStatus->name , shotData->shotPoke[i].trainerName , (SAVELEN_PLAYER_NAME+EOM_SIZE)*2 );
      }
      else
      {
        u8 idx = MUSICAL_EVENT_GetPosIndex( evWork , i );
        const u8 npcIdx = MUSICAL_EVENT_CalcNpcIdx( evWork , idx );
        const u8 nameIdx = MUSICAL_PROGRAM_GetNpcNameIdx( evWork->progWork , npcIdx );
        GFL_MSGDATA *msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_musical_extra_dat , HEAPID_PROC );
        STRBUF * tmpBuf = GFL_MSG_CreateString( msgHandle , nameIdx );

        GFL_STR_GetStringCode( tmpBuf , shotData->shotPoke[i].trainerName , SAVELEN_PLAYER_NAME+EOM_SIZE );
        
        GFL_STR_DeleteBuffer( tmpBuf );
        GFL_MSG_Delete( msgHandle );
      }

      //装備箇所の初期化
      for( j=0;j<MUSICAL_ITEM_EQUIP_MAX;j++ )
      {
        shotData->shotPoke[i].equip[j].itemNo = MUSICAL_ITEM_INVALID;
        shotData->shotPoke[i].equip[j].angle = 0;
        shotData->shotPoke[i].equip[j].equipPos = MUS_POKE_EQU_INVALID;
      }

      for( j=0;j<MUS_POKE_EQUIP_MAX;j++ )
      {
        if( musPoke->equip[j].itemNo != MUSICAL_ITEM_INVALID )
        {
          const u8 idx = musPoke->equip[j].priority;
          shotData->shotPoke[i].equip[idx].itemNo = musPoke->equip[j].itemNo;
          shotData->shotPoke[i].equip[idx].angle = musPoke->equip[j].angle;
          shotData->shotPoke[i].equip[idx].equipPos = j;
        }
      }
    }
  }
  evWork->shotInitWork->isCheckMode = TRUE;
  {
    GAMEDATA *gameData = GAMESYSTEM_GetGameData(evWork->gsys);
    SAVE_CONTROL_WORK *saveWork = GAMEDATA_GetSaveControlWork(gameData);
    evWork->shotInitWork->musicalSave = MUSICAL_SAVE_GetMusicalSave(saveWork);
  }
  
  //今後のための処理
  //順位の確定
  {
    u8 i,j;
    u8 pointArr[MUSICAL_POKE_MAX];
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      pointArr[i] = evWork->actInitWork->musPoke[i]->point;
      evWork->rankIndex[i] = i;
    }
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      for( j=0;j<MUSICAL_POKE_MAX-(i+1);j++ )
      {
        if( pointArr[j+1] > pointArr[j] )
        {
          const u8 tp = pointArr[j+1];
          const u8 ti = evWork->rankIndex[j+1];
          pointArr[j+1] = pointArr[j];
          evWork->rankIndex[j+1] = evWork->rankIndex[j];
          pointArr[j] = tp;
          evWork->rankIndex[j] = ti;
        }
      }
    }
    MUS_TPrintf("RANK:");
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      MUS_TPrintf("[%d:%d]",pointArr[i],evWork->rankIndex[i]);
    }
    MUS_TPrintf("\n");
  }
}

//--------------------------------------------------------------
//  セーブに保存するもの設定
//--------------------------------------------------------------
static void MUSICAL_EVENT_SetSaveData( MUSICAL_EVENT_WORK *evWork )
{
  const u8 selfPoint = MUSICAL_EVENT_GetPoint( evWork , evWork->selfIdx );
  const u8 topPoint = MUSICAL_EVENT_GetPoint( evWork , evWork->rankIndex[0] );

  //参加回数・トップ回数
  MUSICAL_SAVE_AddEntryNum( evWork->musSave );

  if( selfPoint == topPoint &&
      selfPoint > 0 )
  {
    MUSICAL_SAVE_AddTopNum( evWork->musSave );
  }
  
  //最高点・累計点
  {
    MUSICAL_SAVE_SetBefPoint( evWork->musSave , selfPoint );
    MUSICAL_SAVE_AddSumPoint( evWork->musSave , selfPoint );
  }

  //コンディションの保存
  {
    u8 i;
    u8 conTemp[MCT_MAX] = {0};
    MUS_ITEM_DATA_SYS *itemDataSys = MUS_ITEM_DATA_InitSystem( HEAPID_PROC );
    MUSICAL_POKE_PARAM *musPoke = evWork->actInitWork->musPoke[evWork->selfIdx];
    for( i = 0 ; i < MUS_POKE_EQUIP_MAX ; i++ )
    {
      if( musPoke->equip[i].itemNo != MUSICAL_ITEM_INVALID )
      {
        const u8 conType = MUS_ITEM_DATA_GetItemConditionType( itemDataSys , musPoke->equip[i].itemNo );
        conTemp[conType]++;
      }
    }
    for( i = 0 ; i < MCT_MAX ; i++ )
    {
      MUSICAL_SAVE_SetBefCondition( evWork->musSave , i , conTemp[i] );
    }
    MUS_ITEM_DATA_ExitSystem( itemDataSys );
  }
  
  //なつき度上昇
  {
    const ZONEID zoneId = GAMEDATA_GetMyPlayerWork(evWork->gameData)->zoneID;
    NATSUKI_Calc( evWork->pokePara , CALC_NATSUKI_MUSICAL , zoneId , HEAPID_PROC );
    
  }
  
  //レコード系処理
  {
    RECORD *record = GAMEDATA_GetRecordPtr(evWork->gameData);
    if( evWork->isComm == FALSE )
    {
      //非通信の参加回数・勝利回数
      RECORD_Inc( record , RECID_MUSICAL_PLAY_NUM );
      if( selfPoint == topPoint &&
          selfPoint > 0 )
      {
        RECORD_Inc( record , RECID_MUSICAL_WINNER_NUM );
      }
    }
    else
    {
      //通信の参加回数・勝利回数
      RECORD_Inc( record , RECID_MUSICAL_COMM_NUM );
      if( selfPoint == topPoint &&
          selfPoint > 0 )
      {
        RECORD_Inc( record , RECID_MUSICAL_COMM_WINNER_NUM );
      }
    }
    
    //累計点
    RECORD_Add( record , RECID_MUSICAL_TOTAL_POINT , selfPoint );
  }
}

//--------------------------------------------------------------
//  ファンの数値計算
//--------------------------------------------------------------
static void MUSICAL_EVENT_CalcFanState( MUSICAL_EVENT_WORK *evWork )
{
  u8 i,j;
  u8 num;
  u8 rewardNum;
  u8 *rewardArr = GFL_HEAP_AllocClearMemory( HEAPID_PROC_WRAPPER , MUS_REWARD_NUM );
  u32 checkPoint = 0;
  static const u16 totalPointArr[10] = {12000,8000,5500,3500,2000,1000,500,250,100,0};
  //初期化
  for( i=0;i<MUS_SAVE_FAN_NUM;i++ )
  {
    MUSICAL_FAN_STATE* fanState = MUSICAL_SAVE_GetFanState( evWork->musSave , i );
    
    fanState->type = MCT_MAX;
    fanState->giftType = MUSICAL_GIFT_TYPE_NONE;
    fanState->giftValue = 0;
  }
  
  //人数計算
  if( evWork->isComm == FALSE )
  {
    //非通信なので1人
    checkPoint = MUSICAL_SAVE_GetSumPoint( evWork->musSave );
    checkPoint += MUSICAL_EVENT_GetPoint( evWork , evWork->selfIdx );
  }
  else
  {
    MUS_TPrintf( "Comm point check!\n");
    
    for( i=0;i<MUSICAL_POKE_MAX;i++ )
    {
      u8 idx = MUSICAL_EVENT_GetPosIndex( evWork , i );
      const MUS_COMM_MISC_DATA *miscData = MUS_COMM_GetUserMiscData( evWork->commWork , idx );
      MUSICAL_POKE_PARAM *musPoke = MUS_COMM_GetMusPokeParam( evWork->commWork , i );
      if( musPoke != NULL )
      {
        checkPoint += miscData->sumPoint;
        checkPoint += MUSICAL_EVENT_GetPoint( evWork , i );
        MUS_TPrintf( "[%d:%d+%d]\n",i,MUSICAL_EVENT_GetPoint( evWork , i ),miscData->sumPoint );
      }
    }
  }

  for( i=0;i<MUS_SAVE_FAN_NUM;i++ )
  {
    if( totalPointArr[i] <= checkPoint )
    {
      num = 10-i;
      break;
    }
  }
  MUS_TPrintf( "Total point[%d] rewardNum[%d]\n",checkPoint,num );
  //景品テーブルの作成
  {
    
    const u8 type = MUSICAL_PROGRAM_GetRewardType( evWork->progWork );
    MUS_TPrintf( "CheckReward type[%d]\n",type );
    if( type < MUS_REWARD_TYPE )
    {
      //一応チェック
      rewardNum = 0;
      for( j=0;j<MUS_REWARD_NUM;j++ )
      {
        const u16 itemNo = MUSICAL_REWARD_TABLE[type][j];
        if( MUSICAL_SAVE_ChackHaveItem( evWork->musSave , itemNo ) == FALSE )
        {
          rewardArr[rewardNum] = itemNo;
          rewardNum++;
          MUS_TPrintf( "[%2d]",itemNo );
        }
      }
    }
    MUS_TPrintf( "\nTotal[%d]\n",rewardNum );
  }
  {
    const MUSICAL_CONDITION_TYPE conType = MUSICAL_PROGRAM_GetMaxConditionType( evWork->progWork );
    
    //ファンの順番のシャッフル
    u8 fanTempArr[MUS_SAVE_FAN_NUM];
    for( i=0;i<MUS_SAVE_FAN_NUM;i++ )
    {
      fanTempArr[i] = i;
    }
    for( j=0;j<10;j++ )
    {
      for( i=0;i<MUS_SAVE_FAN_NUM;i++ )
      {
        u8 swapIdx = GFUser_GetPublicRand0(MUS_SAVE_FAN_NUM);
        u8 temp = fanTempArr[i];
        fanTempArr[i] = fanTempArr[swapIdx];
        fanTempArr[swapIdx] = temp;
      }
    }
    
    for( i=0;i<num;i++ )
    {
      u8 rand;
      const u8 idx = fanTempArr[i];
      MUSICAL_FAN_STATE* fanState = MUSICAL_SAVE_GetFanState( evWork->musSave , idx );
      const u8 point = MUSICAL_EVENT_GetPoint( evWork , evWork->selfIdx );
      MUS_TPrintf("Fan[%d]",idx+1);
      //見た目の決定
      fanState->type = conType; //タイプを渡す
      MUS_TPrintf("[%d]",fanState->type);
      //贈り物の抽選
      rand = GFUser_GetPublicRand0(100);
      MUS_TPrintf("[%3d/100 (%d):",point,rand);
      if( rand < point )
      {
        if( rewardNum > 0 )
        {
          const u8 idx = GFUser_GetPublicRand0(rewardNum);
          fanState->giftType = MUSICAL_GIFT_TYPE_GOODS;
          fanState->giftValue = rewardArr[idx];
          MUS_TPrintf("%d:%d",fanState->giftType,fanState->giftValue);
          for( j=idx;j<rewardNum-1;j++ )
          {
            rewardArr[j] = rewardArr[j+1];
          }
          rewardNum--;
        }
        else
        {
          //グッズがない！
          //現状アイテムは渡さない
          MUS_TPrintf("goods none!");
        }
      }
      MUS_TPrintf("]\n");
    }
  }
  GFL_HEAP_FreeMemory(rewardArr);
}

#pragma mark [>FieldFunc
//--------------------------------------------------------------
//  フィールドを呼ぶ
//--------------------------------------------------------------
static const BOOL MUSICAL_EVENT_InitField( GMEVENT *event, MUSICAL_EVENT_WORK *evWork )
{
  GAMEDATA *gameData = GAMESYSTEM_GetGameData(evWork->gsys);
  FIELD_SOUND *fieldSound = GAMEDATA_GetFieldSound( gameData );
  switch( evWork->subSeq )
  {
  case 0:
    GMEVENT_CallEvent(event, EVENT_FSND_PopBGMNoWait(evWork->gsys, FSND_FADE_NONE, FSND_FADE_NORMAL));
    evWork->subSeq++; 
    break; 
  case 1:
    GMEVENT_CallEvent(event, EVENT_FieldOpen(evWork->gsys));
    evWork->subSeq++;
    break;
  case 2:
    {
      //FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( evWork->gsys );
      //GMEVENT_CallEvent(event, EVENT_FieldFadeIn(evWork->gsys, fieldWork, 0));
      evWork->subSeq++;
    }
    break;
  case 3:
    GMEVENT_CallEvent(event, EVENT_FSND_WaitBGMPop(evWork->gsys));
    evWork->subSeq++;
    break;
  case 4:
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//  フィールドを抜ける
//--------------------------------------------------------------
static const BOOL MUSICAL_EVENT_ExitField( GMEVENT *event, MUSICAL_EVENT_WORK *evWork )
{
  GAMEDATA *gameData = GAMESYSTEM_GetGameData(evWork->gsys);
  FIELD_SOUND *fieldSound = GAMEDATA_GetFieldSound( gameData );
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( evWork->gsys );
  switch( evWork->subSeq )
  {
  case 0:
    //GMEVENT_CallEvent(event, EVENT_FieldFadeOut(evWork->gsys, fieldWork, 0));
    evWork->subSeq++;
    break;
  case 1:
    GMEVENT_CallEvent(event, EVENT_FSND_PushBGM(evWork->gsys, 30));
    evWork->subSeq++;
    break; 
  case 2:
    GMEVENT_CallEvent(event, EVENT_FieldClose(evWork->gsys, fieldWork));
    evWork->subSeq++;
    break;
  case 3:
    GMEVENT_CallEvent(event, EVENT_FSND_WaitBGMFade(evWork->gsys));
    evWork->subSeq++;
    break;
  case 4:
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//  待合室へ移動
//--------------------------------------------------------------
static const void MUSICAL_EVENT_JumpWaitingRoom( GMEVENT *event, MUSICAL_EVENT_WORK *evWork )
{
  GMEVENT *newEvent;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( evWork->gsys );
  VecFx32 pos = { FX32_CONST(136.0f) , FX32_CONST(0.0f) , FX32_CONST(200.0f) };
  
  pos.x += evWork->selfIdx * (FX32_ONE*32);
  
  newEvent = EVENT_ChangeMapPosNoFade( evWork->gsys, fieldWork ,
                ZONE_ID_C04R0202 , &pos , DIR_UP );
  GMEVENT_CallEvent(event, newEvent);
}

//--------------------------------------------------------------
//  会場へ移動
//--------------------------------------------------------------
static const void MUSICAL_EVENT_JumpMusicalHall( GMEVENT *event, MUSICAL_EVENT_WORK *evWork )
{
  GMEVENT *newEvent;
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork( evWork->gsys );


  const VecFx32 pos     = { FX32_CONST(232.0f) , FX32_CONST(0.0f) , FX32_CONST(200.0f) };
  const VecFx32 posComm = { FX32_CONST(168.0f) , FX32_CONST(0.0f) , FX32_CONST(200.0f) };
  
  if( evWork->isComm == TRUE )
  {
    newEvent = EVENT_ChangeMapPosNoFade( evWork->gsys, fieldWork , ZONE_ID_C04R0201 , &posComm , DIR_DOWN );
  }
  else
  {
    newEvent = EVENT_ChangeMapPosNoFade( evWork->gsys, fieldWork , ZONE_ID_C04R0201 , &pos , DIR_DOWN);
  }
  GMEVENT_CallEvent(event, newEvent);
}

//--------------------------------------------------------------
//  スクリプトを呼ぶ
//--------------------------------------------------------------
static const void MUSICAL_EVENT_RunScript( GMEVENT *event, MUSICAL_EVENT_WORK *evWork , u16 scriptId )
{
  {
    GMEVENT *newEvent;
    SCRIPT_FLDPARAM fparam;
    SCRIPT_WORK *scWork;

    newEvent = SCRIPT_SetEventScript( evWork->gsys, scriptId , NULL , HEAPID_FIELDMAP );
    scWork = SCRIPT_GetScriptWorkFromEvent( newEvent );
    
    GMEVENT_CallEvent(event, newEvent);
  }
}

static const u8 MUSICAL_EVENT_CalcNpcIdx( MUSICAL_EVENT_WORK *evWork , const u8 idx )
{
  u8 i;
  u8 npcIdx = 0;
  for( i=0;i<idx;i++ )
  {
    
    if( evWork->commWork == NULL )
    {
      if( i != 0 )
      {
        npcIdx++;
      }
    }
    else
    {
      if( MUS_COMM_GetPlayerMyStatus( evWork->commWork , i ) == NULL )
      {
        npcIdx++;
      }
    }
  }
  OS_TPrintf("Pos[%d]->NPC[%d]\n",idx,npcIdx);
  return npcIdx;
}

#pragma mark [>outer func
//自分の参加番号取得
const u8 MUSICAL_EVENT_GetSelfIndex( MUSICAL_EVENT_WORK *evWork )
{
  return evWork->selfIdx;
}

//位置に対応した参加番号取得
const u8 MUSICAL_EVENT_GetPosIndex( MUSICAL_EVENT_WORK *evWork , const u8 pos )
{
  u8 i;
  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    if( evWork->musicalIndex[i] == pos )
    {
      return i;
    }
  }
  return 0;
}

//最高コンディションの取得
const u8 MUSICAL_EVENT_GetMaxCondition( MUSICAL_EVENT_WORK *evWork )
{
  return  MUSICAL_PROGRAM_GetMaxConditionType( evWork->progWork );
}

//現在演目名取得
STRBUF* MUSICAL_EVENT_CreateStr_ProgramTitle( MUSICAL_EVENT_WORK *evWork , HEAPID heapId )
{
  STRBUF *strBuf;
  GFL_MSGDATA *msgHandle = GFL_MSG_Construct( evWork->distData->messageData , heapId );
  strBuf = GFL_MSG_CreateString( msgHandle , 0 );
  GFL_MSG_Delete( msgHandle );
  return strBuf;
}

//各種評価点取得
const u8 MUSICAL_EVENT_GetPoint( MUSICAL_EVENT_WORK *evWork , const u8 idx )
{
  const MUSICAL_POKE_PARAM *musPoke = evWork->actInitWork->musPoke[idx];
  if( musPoke->point < 256 )
  {
    return musPoke->point;
  }
  else
  {
    return 255;
  }
}

const u8 MUSICAL_EVENT_GetMaxPoint( MUSICAL_EVENT_WORK *evWork )
{
  u8 i;
  u8 maxPoint = 0;
  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    const MUSICAL_POKE_PARAM *musPoke = evWork->actInitWork->musPoke[i];
    if( maxPoint < musPoke->point )
    {
      maxPoint = musPoke->point;
    }
  }
  return maxPoint;
}

const u8 MUSICAL_EVENT_GetMinPoint( MUSICAL_EVENT_WORK *evWork )
{
  u8 i;
  u8 minPoint = 0xFF;
  for( i=0;i<MUSICAL_POKE_MAX;i++ )
  {
    const MUSICAL_POKE_PARAM *musPoke = evWork->actInitWork->musPoke[i];
    if( minPoint > musPoke->point )
    {
      minPoint = musPoke->point;
    }
  }
  return minPoint;
}

//指定順位のキャラの位置
const u8 MUSICAL_EVENT_GetPosToRank( MUSICAL_EVENT_WORK *evWork , const u8 idx )
{
  return evWork->rankIndex[idx];
}

//キャラ別のコンディション評価点の高い数値を取得
const u8 MUSICAL_EVENT_GetMaxPointCondition( MUSICAL_EVENT_WORK *evWork , const u8 idx )
{
  u8 i;
  u8 maxPoint = 0;
  u8 maxType = 0;
  BOOL isSame = FALSE;  //トップが同点が
  for( i=0;i<MCT_MAX;i++ )
  {
    const MUSICAL_POKE_PARAM *musPoke = evWork->actInitWork->musPoke[idx];
    if( maxPoint < musPoke->conPoint[i] )
    {
      isSame = FALSE;
      maxType = i;
      maxPoint = musPoke->conPoint[i];
    }
    else
    if( maxPoint == musPoke->conPoint[i] )
    {
      isSame = TRUE;
    }
  }
  
  if( isSame == TRUE )
  {
    return MUSICAL_CONDITION_NONE;
  }
  
  return maxType;
}

const u8 MUSICAL_EVENT_GetPosObjView( MUSICAL_EVENT_WORK *evWork , const u8 pos )
{
  u8 i;
  u8 idx = MUSICAL_EVENT_GetPosIndex( evWork , pos );
  
  if( pos == evWork->selfIdx )
  {
    return NONDRAW;
  }
  else
  if( evWork->isComm == TRUE )
  {
    MYSTATUS *commMyStatus = MUS_COMM_GetPlayerMyStatus( evWork->commWork , idx );
    if( commMyStatus != NULL )
    {
      const u32 sex = MyStatus_GetMySex( commMyStatus );
      if( sex == PTL_SEX_MALE )
      {
        return HERO;
      }
      else
      {
        return HEROINE;
      }
    }
  }
  
  {
    const u8 npcIdx = MUSICAL_EVENT_CalcNpcIdx( evWork , idx );
    return MUSICAL_PROGRAM_GetNpcObjId( evWork->progWork , npcIdx );
  }
}

void MUSICAL_EVENT_SetPosCharaName_Wordset( MUSICAL_EVENT_WORK *evWork , const u8 pos , WORDSET *wordSet , const u8 wordIdx )
{
  u8 i;
  BOOL isSet = FALSE;
  u8 idx = MUSICAL_EVENT_GetPosIndex( evWork , pos );

  if( pos == evWork->selfIdx )
  {
    MYSTATUS *myStatus = GAMEDATA_GetMyStatus( evWork->gameData );
    WORDSET_RegisterPlayerName( wordSet , wordIdx , myStatus );
    isSet = TRUE;
  }
  else
  if( evWork->isComm == TRUE )
  {
    MYSTATUS *commMyStatus = MUS_COMM_GetPlayerMyStatus( evWork->commWork , idx );
    if( commMyStatus != NULL )
    {
      WORDSET_RegisterPlayerName( wordSet , wordIdx , commMyStatus );
      isSet = TRUE;
    }
  }
  
  if( isSet == FALSE )
  {
    const u8 npcIdx = MUSICAL_EVENT_CalcNpcIdx( evWork , idx );
    const u8 nameIdx = MUSICAL_PROGRAM_GetNpcNameIdx( evWork->progWork , npcIdx );
    GFL_MSGDATA *msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_musical_extra_dat , HEAPID_PROC );
    STRBUF * tmpBuf = GFL_MSG_CreateString( msgHandle , nameIdx );
    
    WORDSET_RegisterWord( wordSet, wordIdx, tmpBuf, 0, TRUE, PM_LANG );

    GFL_STR_DeleteBuffer( tmpBuf );
    GFL_MSG_Delete( msgHandle );
  }
}


void MUSICAL_EVENT_SetPosPokeName_Wordset( MUSICAL_EVENT_WORK *evWork , const u8 pos , WORDSET *wordSet , const u8 wordIdx )
{
  u8 i;
  BOOL isSet = FALSE;
  u8 idx = MUSICAL_EVENT_GetPosIndex( evWork , pos );

  if( pos == evWork->selfIdx )
  {
    MYSTATUS *myStatus = GAMEDATA_GetMyStatus( evWork->gameData );
    WORDSET_RegisterPokeNickName( wordSet , wordIdx , evWork->pokePara );
    isSet = TRUE;
  }
  else
  if( evWork->isComm == TRUE )
  {
    POKEMON_PASO_PARAM *ppp = MUS_COMM_GetPlayerPPP( evWork->commWork , idx );
    if( ppp != NULL )
    {
      WORDSET_RegisterPokeNickNamePPP( wordSet , wordIdx , ppp );
      isSet = TRUE;
    }
  }
  
  if( isSet == FALSE )
  {
    const u8 npcIdx = MUSICAL_EVENT_CalcNpcIdx( evWork , idx );
    const u8 nameIdx = MUSICAL_PROGRAM_GetNpcNameIdx( evWork->progWork , npcIdx );
    GFL_MSGDATA *msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_musical_extrapoke_dat , HEAPID_PROC );
    STRBUF * tmpBuf = GFL_MSG_CreateString( msgHandle , nameIdx );
    
    WORDSET_RegisterWord( wordSet, wordIdx, tmpBuf, 0, TRUE, PM_LANG );

    GFL_STR_DeleteBuffer( tmpBuf );
    GFL_MSG_Delete( msgHandle );
    
    //const u16 monsno = MUSICAL_PROGRAM_GetNpcMonsno( evWork->progWork , idx-1 );
    //WORDSET_RegisterPokeMonsNameNo( wordSet, wordIdx, monsno );
  }
}

//演目データ受信後に数値を計算する
void MUSICAL_EVENT_CalcProgramData( MUSICAL_EVENT_WORK *evWork )
{
  u32 conPointArr = MUS_COMM_GetConditionPointArr( evWork->commWork );
  u32 npcArr = MUS_COMM_GetNpcArr( evWork->commWork );
  if( evWork->progWork == NULL )
  {
    evWork->progWork = MUSICAL_PROGRAM_InitProgramData( HEAPID_PROC_WRAPPER , evWork->distData , 0 );
  }
  MUSICAL_PROGRAM_SetConditionPointArr( evWork->progWork , conPointArr );
  MUSICAL_PROGRAM_SetNpcArr( evWork->progWork , npcArr );
}

void MUSICAL_EVENT_CheckNetErr( MUSICAL_EVENT_WORK *evWork )
{
  if( evWork->isComm == TRUE )
  {
    if( NetErr_App_CheckError() != NET_ERR_CHECK_NONE &&
        evWork->isNetErr == FALSE )
    {
      MUS_TPrintf("Musical Error!!!\n");
      //NetErr_App_ReqErrorDisp();
      evWork->isNetErr = TRUE;
    }
  }
}

const BOOL MUSICAL_EVENT_IsNetErr( MUSICAL_EVENT_WORK *evWork )
{
  return evWork->isNetErr;
}
