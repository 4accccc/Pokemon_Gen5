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

#include "arc_def.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#include "snd_strm.naix"

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
#include "test/ariizumi/ari_debug.h"
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
  MPS_INIT_LOBBY,
  MPS_TERM_LOBBY,
  MPS_INIT_DRESSUP_SEND_MUSICAL_IDX,
  MPS_INIT_DRESSUP_SEND_STRM,
  MPS_INIT_DRESSUP,
  MPS_TERM_DRESSUP,
  MPS_INIT_ACTING,
  MPS_INIT_ACTING_COMM,
  MPS_TERM_ACTING,
  
  MPS_WAIT_MEMBER_DRESSUP,
  MPS_SEND_MUS_POKE,
  MPS_POST_WAIT_MUS_POKE,
  MPS_POST_WAIT_ALL_MUS_POKE,
  MPS_POST_WAIT_START_ACTING,
  
  //開始前同期データ
  MPS_SEND_PROGRAM_SIZE,
  MPS_WAIT_SEND_PROGRAM_SIZE,
  MPS_WAIT_SEND_PROGRAM_DATA,
  MPS_WAIT_SEND_MESSAGE_SIZE,
  MPS_WAIT_SEND_MESSAGE_DATA,
  MPS_WAIT_SEND_SCRIPT_SIZE,
  MPS_WAIT_SEND_SCRIPT_DATA,
  
  MPS_FINISH,
}MUSICAL_PROC_STATE;

//ミュージカル配信データはnaixを持っていないし、コンバータで順番固定で出力しているので
//ココでenum定義
enum
{
  MUSICAL_ARCDATAID_PROGDATA = 0,
  MUSICAL_ARCDATAID_GMMDATA = 1,
  MUSICAL_ARCDATAID_SCRIPTDATA = 2,
  MUSICAL_ARCDATAID_SBNKDATA = 3,
  MUSICAL_ARCDATAID_SSEQDATA = 4,
  MUSICAL_ARCDATAID_SWAVDATA = 5,
};

//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  GFL_PROCSYS *procSys;
  
  MUSICAL_DISTRIBUTE_DATA *distData;
  
  MUSICAL_PROC_STATE state;
  MUS_COMM_WORK     *commWork;
  DRESSUP_INIT_WORK *dupInitWork;
  STAGE_INIT_WORK *actInitWork;
  
  MUSICAL_PROGRAM_WORK *progWork;
  MUSICAL_POKE_PARAM *musPoke;
}MUSICAL_PROC_WORK;
//======================================================================
//  proto
//======================================================================
#pragma mark [> proto
/*
static GFL_PROC_RESULT MusicalProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

GFL_PROC_DATA Musical_ProcData =
{
  MusicalProc_Init,
  MusicalProc_Main,
  MusicalProc_Term
};
*/

static const u16 musPokeArr[]=
{
  MONSNO_PIKATYUU,
  MONSNO_PIKUSII,
  MONSNO_WARUBIARU,
  MONSNO_ONOKKUSU,
  MONSNO_PURUNSU,
  MONSNO_MUNNA,  
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
  return TRUE;    //仮 FIXME
  return FALSE;
}

const BOOL  MUSICAL_SYSTEM_CheckEntryMusical( POKEMON_PARAM *pokePara )
{
  u16 mons_no = PP_Get( pokePara, ID_PARA_monsno, NULL );
  //FIXME タマゴとか
  return MUSICAL_SYSTEM_CheckEntryMusicalPokeNo( mons_no );
}

//ミュージカルでの番号に変換
const u16 MUSICAL_SYSTEM_ChangeMusicalPokeNumber( const u16 mons_no )
{
  u16 i=0;

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

//ランダムでミュージカル参加可能なポケモンを返す
const u16 MUSICAL_SYSTEM_GetMusicalPokemonRandom( void )
{
  u16 i=0;

  while( 0xFFFF != musPokeArr[i] )
  {
    i++;
  }
  return musPokeArr[ GFUser_GetPublicRand0(i) ];
}

//ミュージカル用パラメータの初期化
MUSICAL_POKE_PARAM* MUSICAL_SYSTEM_InitMusPoke( POKEMON_PARAM *pokePara , HEAPID heapId )
{
  int i;
  MUSICAL_POKE_PARAM *musPara;
  musPara = GFL_HEAP_AllocClearMemory( heapId , sizeof(MUSICAL_POKE_PARAM) );
  musPara->charaType = MUS_CHARA_INVALID;
  musPara->pokePara = pokePara;
  musPara->point = 0;
  
  for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
  {
    musPara->equip[i].itemNo = MUSICAL_ITEM_INVALID;
    musPara->equip[i].angle = 0;
    musPara->isApeerBonus[i] = FALSE;
  }
  
  musPara->mcssParam.monsno = PP_Get( pokePara, ID_PARA_monsno,	NULL );
  musPara->mcssParam.sex    = PP_Get( pokePara, ID_PARA_form_no,	NULL );
  musPara->mcssParam.form = PP_Get( pokePara, ID_PARA_sex,	NULL );
  musPara->mcssParam.rare   = PP_CheckRare( pokePara );
  
  return musPara;
}
MUSICAL_POKE_PARAM* MUSICAL_SYSTEM_InitMusPokeParam( u16 monsno , u8 sex , u8 form , u8 rare , HEAPID heapId )
{
  int i;
  MUSICAL_POKE_PARAM *musPara;
  musPara = GFL_HEAP_AllocClearMemory( heapId , sizeof(MUSICAL_POKE_PARAM) );
  musPara->charaType = MUS_CHARA_INVALID;
  musPara->pokePara = NULL;
  musPara->point = 0;
  
  for( i=0;i<MUS_POKE_EQUIP_MAX;i++ )
  {
    musPara->equip[i].itemNo = MUSICAL_ITEM_INVALID;
    musPara->equip[i].angle = 0;
    musPara->isApeerBonus[i] = FALSE;
  }
  
  musPara->mcssParam.monsno = monsno;
  musPara->mcssParam.sex    = sex;
  musPara->mcssParam.form = form;
  musPara->mcssParam.rare   = rare;
  
  return musPara;
}

#pragma mark [>distribute data
MUSICAL_DISTRIBUTE_DATA* MUSICAL_SYSTEM_InitDistributeData( HEAPID workHeapId )
{
  MUSICAL_DISTRIBUTE_DATA *distData = GFL_HEAP_AllocMemory( workHeapId , sizeof( MUSICAL_DISTRIBUTE_DATA ) );

  distData->programData = NULL;
  distData->messageData = NULL;
  distData->scriptData = NULL;
  distData->midiData = NULL;
  distData->midiSeqData = NULL;
  distData->midiBnkData = NULL;
  distData->midiWaveData = NULL;

  return distData;
}

void MUSICAL_SYSTEM_TermDistributeData( MUSICAL_DISTRIBUTE_DATA *distData )
{
  if( distData->programData != NULL )
  {
    GFL_HEAP_FreeMemory( distData->programData );
  }
  if( distData->messageData != NULL )
  {
    GFL_HEAP_FreeMemory( distData->messageData );
  }
  if( distData->scriptData != NULL )
  {
    GFL_HEAP_FreeMemory( distData->scriptData );
  }
  //Seq・Bnk・Waveはパックしてある
  if( distData->midiData != NULL )
  {
    GFL_HEAP_FreeMemory( distData->midiData );
  }

  GFL_HEAP_FreeMemory( distData );
}

void MUSICAL_SYSTEM_LoadDistributeData_Data( MUSICAL_DISTRIBUTE_DATA *distData , const u8 programNo , HEAPID heapId )
{
  distData->programNo = programNo;
  if( programNo >= MUS_PROGRAM_LOCAL_NUM )
  {
    //FIXME:セーブデータからの取得
    distData->programData = GFL_ARC_UTIL_LoadEx( ARCID_MUSICAL_PROGRAM_01 , MUSICAL_ARCDATAID_PROGDATA , FALSE , heapId , &distData->programDataSize );
  }
  else
  {
    distData->programData = GFL_ARC_UTIL_LoadEx( ARCID_MUSICAL_PROGRAM_01+programNo , MUSICAL_ARCDATAID_PROGDATA , FALSE , heapId , &distData->programDataSize );
  }
}
void MUSICAL_SYSTEM_LoadDistributeData_Script( MUSICAL_DISTRIBUTE_DATA *distData , const u8 programNo , HEAPID heapId )
{
  distData->programNo = programNo;
  if( programNo >= MUS_PROGRAM_LOCAL_NUM )
  {
    //FIXME:セーブデータからの取得
    distData->messageData = GFL_ARC_UTIL_LoadEx( ARCID_MUSICAL_PROGRAM_01 , MUSICAL_ARCDATAID_GMMDATA , FALSE , heapId , &distData->messageDataSize );
    distData->scriptData = GFL_ARC_UTIL_LoadEx( ARCID_MUSICAL_PROGRAM_01 , MUSICAL_ARCDATAID_SCRIPTDATA , FALSE , heapId , &distData->scriptDataSize );
  }
  else
  {
    distData->messageData = GFL_ARC_UTIL_LoadEx( ARCID_MUSICAL_PROGRAM_01+programNo , MUSICAL_ARCDATAID_GMMDATA , FALSE , heapId , &distData->messageDataSize );
    distData->scriptData = GFL_ARC_UTIL_LoadEx( ARCID_MUSICAL_PROGRAM_01+programNo , MUSICAL_ARCDATAID_SCRIPTDATA , FALSE , heapId , &distData->scriptDataSize );
  }
}
void MUSICAL_SYSTEM_LoadDistributeData_Strm( MUSICAL_DISTRIBUTE_DATA *distData , const u8 programNo , HEAPID heapId )
{
  ARCHANDLE *arcHandle;
  MUS_DIST_MIDI_HEADER midiSizeHeader;
  MUS_DIST_MIDI_HEADER *pMidiSizeHeader;

  distData->programNo = programNo;
  if( programNo >= MUS_PROGRAM_LOCAL_NUM )
  {
    //FIXME:セーブデータからの取得
    arcHandle = GFL_ARC_OpenDataHandle( ARCID_MUSICAL_PROGRAM_01 , GFL_HEAP_LOWID(heapId) );
  }
  else
  {
    arcHandle = GFL_ARC_OpenDataHandle( ARCID_MUSICAL_PROGRAM_01+programNo , GFL_HEAP_LOWID(heapId) );
  }

  midiSizeHeader.seqSize  = GFL_ARC_GetDataSizeByHandle( arcHandle , MUSICAL_ARCDATAID_SSEQDATA );
  midiSizeHeader.bankSize = GFL_ARC_GetDataSizeByHandle( arcHandle , MUSICAL_ARCDATAID_SBNKDATA );
  midiSizeHeader.waveSize = GFL_ARC_GetDataSizeByHandle( arcHandle , MUSICAL_ARCDATAID_SWAVDATA );
  
  distData->midiDataSize = sizeof(MUS_DIST_MIDI_HEADER) + midiSizeHeader.seqSize + midiSizeHeader.bankSize + midiSizeHeader.waveSize;
  
  distData->midiData = GFL_HEAP_AllocClearMemory( heapId , distData->midiDataSize );
  
  pMidiSizeHeader = distData->midiData;
  distData->midiSeqData  = (void*)((u32)distData->midiData + sizeof(MUS_DIST_MIDI_HEADER));
  distData->midiBnkData  = (void*)((u32)distData->midiData + sizeof(MUS_DIST_MIDI_HEADER) + midiSizeHeader.seqSize );
  distData->midiWaveData = (void*)((u32)distData->midiData + sizeof(MUS_DIST_MIDI_HEADER) + midiSizeHeader.seqSize + midiSizeHeader.bankSize );
  
  pMidiSizeHeader->seqSize  = midiSizeHeader.seqSize;
  pMidiSizeHeader->bankSize = midiSizeHeader.bankSize;
  pMidiSizeHeader->waveSize = midiSizeHeader.waveSize;
  
  GFL_ARC_LoadDataByHandle( arcHandle , MUSICAL_ARCDATAID_SSEQDATA , distData->midiSeqData );
  GFL_ARC_LoadDataByHandle( arcHandle , MUSICAL_ARCDATAID_SBNKDATA , distData->midiBnkData );
  GFL_ARC_LoadDataByHandle( arcHandle , MUSICAL_ARCDATAID_SWAVDATA , distData->midiWaveData );
  
  GFL_ARC_CloseDataHandle( arcHandle );

  ARI_TPrintf("MusicalSystem LoadMidiData[%d][%d][%d].\n",pMidiSizeHeader->seqSize,pMidiSizeHeader->bankSize,pMidiSizeHeader->waveSize);
}

#pragma mark [>proc 
/*
static GFL_PROC_RESULT MusicalProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUSICAL_INIT_WORK *initWork = pwk;
  MUSICAL_PROC_WORK *work;
  
  GFL_OVERLAY_Load(FS_OVERLAY_ID(musical));
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MUSICAL_STRM|HEAPDIR_MASK, 0x80000 );
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MUSICAL_PROC|HEAPDIR_MASK, 0x8000 );

  work = GFL_PROC_AllocWork( proc, sizeof(MUSICAL_PROC_WORK), HEAPID_MUSICAL_PROC );
  work->musPoke = MUSICAL_SYSTEM_InitMusPoke( initWork->pokePara , HEAPID_MUSICAL_PROC );
  work->dupInitWork = NULL;
  work->actInitWork = NULL;
  work->distData = MUSICAL_SYSTEM_InitDistributeData( HEAPID_MUSICAL_PROC );

  work->procSys = GFL_PROC_LOCAL_boot( HEAPID_MUSICAL_PROC );
  
  if( initWork->isComm == TRUE )
  {
    //distDataは親子確定後に初期化する
    work->state = MPS_INIT_LOBBY;
    work->commWork = MUS_COMM_CreateWork( HEAPID_MUSICAL_PROC , initWork->gameComm , initWork->saveCtrl , work->distData );
  }
  else
  {
    MUSICAL_SYSTEM_LoadDistributeData( work->distData , 0 , HEAPID_MUSICAL_STRM );
    work->progWork = MUSICAL_PROGRAM_InitProgramData( HEAPID_MUSICAL_PROC , work->distData );
    work->state = MPS_INIT_DRESSUP;
    work->commWork = NULL;
  }
  return GFL_PROC_RES_FINISH;
  
}

static GFL_PROC_RESULT MusicalProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUSICAL_INIT_WORK *initWork = pwk;
  MUSICAL_PROC_WORK *work = mywk;
  
  if( work->commWork != NULL )
  {
    MUS_COMM_DeleteWork( work->commWork );
    work->commWork = NULL;
  }
  
  if( GFL_NET_IsExit() == FALSE )
  {
    return GFL_PROC_RES_CONTINUE;
  }
  
  if( work->dupInitWork != NULL )
  {
    MUSICAL_DRESSUP_DeleteInitWork( work->dupInitWork );
  }
  if( work->actInitWork != NULL )
  {
    MUSICAL_STAGE_DeleteStageWork( work->actInitWork );
  }
  GFL_PROC_LOCAL_Exit( work->procSys );

  MUSICAL_SYSTEM_TermDistributeData( work->distData );
  GFL_HEAP_FreeMemory( work->musPoke );
  
  MUSICAL_PROGRAM_TermProgramData( work->progWork );
  
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_MUSICAL_STRM );
  GFL_HEAP_DeleteHeap( HEAPID_MUSICAL_PROC );
  
  if( initWork->isDebug == TRUE )
  {
    GFL_HEAP_FreeMemory( initWork->pokePara );
    GFL_HEAP_FreeMemory( initWork );
  }


  GFL_OVERLAY_Unload(FS_OVERLAY_ID(musical));
  
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MusicalProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  MUSICAL_INIT_WORK *initWork = pwk;
  MUSICAL_PROC_WORK *work = mywk;
  
  const GFL_PROC_MAIN_STATUS isActiveProc = GFL_PROC_LOCAL_Main( work->procSys );
  
  switch( work->state )
  {
  case MPS_INIT_LOBBY:
    GFL_PROC_LOCAL_CallProc( work->procSys , NO_OVERLAY_ID, &MUS_LOBBY_ProcData, work->commWork );
    work->state = MPS_TERM_LOBBY;
    break;
    
  case MPS_TERM_LOBBY:
    if( isActiveProc == GFL_PROC_MAIN_NULL )
    {
      switch( MUS_COMM_GetMode( work->commWork ) )
      {
      case MCM_NONE:
        work->state = MPS_FINISH;
        break;
      
      case MCM_PARENT:  //仮
        MUSICAL_SYSTEM_LoadDistributeData( work->distData , 0 , HEAPID_MUSICAL_STRM );
        work->state = MPS_INIT_DRESSUP_SEND_MUSICAL_IDX;
        break;

      case MCM_CHILD:  //仮
        work->state = MPS_WAIT_SEND_PROGRAM_DATA;
        break;
      }
    }
    break;
    
  case MPS_INIT_DRESSUP_SEND_MUSICAL_IDX:
    if( MUS_COMM_Send_MusicalIndex( work->commWork ) == TRUE )
    {
      work->state = MPS_SEND_PROGRAM_SIZE;
    }
    break;
    
  //開始前同期データ
  case MPS_SEND_PROGRAM_SIZE:
    if( MUS_COMM_Send_ProgramSize( work->commWork ) == TRUE )
    {
      work->state = MPS_WAIT_SEND_PROGRAM_SIZE;
    }
    break;

  case MPS_WAIT_SEND_PROGRAM_SIZE:
    if( MUS_COMM_IsPostProgramSize( work->commWork ) == TRUE )
    {
      if( MUS_COMM_Send_ProgramData( work->commWork ) == TRUE )
      {
        work->state = MPS_WAIT_SEND_PROGRAM_DATA;
      }
    }
    break;

  case MPS_WAIT_SEND_PROGRAM_DATA:
    if( MUS_COMM_IsPostProgramData( work->commWork ) == TRUE )
    {
      if( MUS_COMM_GetMode( work->commWork ) == MCM_PARENT )
      {
        if( MUS_COMM_Send_MessageSize( work->commWork ) == TRUE )
        {
          work->state = MPS_WAIT_SEND_MESSAGE_SIZE;
        }
      }
      else
      {
        work->state = MPS_WAIT_SEND_MESSAGE_DATA;
      }
    }
    break;

  case MPS_WAIT_SEND_MESSAGE_SIZE:
    if( MUS_COMM_IsPostMessageSize( work->commWork ) == TRUE )
    {
      if( MUS_COMM_Send_MessageData( work->commWork ) == TRUE )
      {
        work->state = MPS_WAIT_SEND_MESSAGE_DATA;
      }
    }
    break;

  case MPS_WAIT_SEND_MESSAGE_DATA:
    if( MUS_COMM_IsPostMessageData( work->commWork ) == TRUE )
    {
      if( MUS_COMM_GetMode( work->commWork ) == MCM_PARENT )
      {
        if( MUS_COMM_Send_ScriptSize( work->commWork ) == TRUE )
        {
          work->state = MPS_WAIT_SEND_SCRIPT_SIZE;
        }
      }
      else
      {
        work->state = MPS_WAIT_SEND_SCRIPT_DATA;
      }
    }
    break;

  case MPS_WAIT_SEND_SCRIPT_SIZE:
    if( MUS_COMM_IsPostScriptSize( work->commWork ) == TRUE )
    {
      if( MUS_COMM_Send_ScriptData( work->commWork ) == TRUE )
      {
        work->state = MPS_WAIT_SEND_SCRIPT_DATA;
      }
    }
    break;

  case MPS_WAIT_SEND_SCRIPT_DATA:
    if( MUS_COMM_IsPostScriptData( work->commWork ) == TRUE )
    {
      work->progWork = MUSICAL_PROGRAM_InitProgramData( HEAPID_MUSICAL_PROC , work->distData );
      if( MUS_COMM_GetMode( work->commWork ) == MCM_PARENT )
      {
        work->state = MPS_INIT_DRESSUP_SEND_STRM;
      }
      else
      {
        work->state = MPS_INIT_DRESSUP;
      }
    }
    break;

  case MPS_INIT_DRESSUP_SEND_STRM:
    MUS_COMM_Start_SendStrmData( work->commWork );
    work->state = MPS_INIT_DRESSUP;
    break;

  case MPS_INIT_DRESSUP:
    work->dupInitWork = MUSICAL_DRESSUP_CreateInitWork( HEAPID_MUSICAL_PROC , work->musPoke , initWork->saveCtrl );
    work->dupInitWork->commWork = work->commWork;
    GFL_PROC_LOCAL_CallProc( work->procSys , NO_OVERLAY_ID, &DressUp_ProcData, work->dupInitWork );
    work->state = MPS_TERM_DRESSUP;
    break;

  case MPS_TERM_DRESSUP:
    if( isActiveProc == GFL_PROC_MAIN_NULL )
    {
      if( initWork->isComm == TRUE )
      {
        if( MUS_COMM_SetCommGameState( work->commWork , MCGS_WAIT_DRESSUP ) == TRUE )
        {
          work->state = MPS_WAIT_MEMBER_DRESSUP;
          MUS_COMM_SendTimingCommand( work->commWork , MUS_COMM_TIMMING_DRESSUP_WAIT );
        }
      }
      else
      {
        work->state = MPS_INIT_ACTING;
      }
    }
    break;
  
    //メンバー待ち
  case MPS_WAIT_MEMBER_DRESSUP:
    if( MUS_COMM_CheckTimingCommand( work->commWork , MUS_COMM_TIMMING_DRESSUP_WAIT ) == TRUE )
    {
      work->state = MPS_SEND_MUS_POKE;
    }
    break;
    
    //MusPokeを送る
  case MPS_SEND_MUS_POKE:
    if( MUS_COMM_Send_MusPokeData( work->commWork , work->musPoke ) == TRUE )
    {
      if( MUS_COMM_GetMode( work->commWork ) == MCM_PARENT )
      {
        work->state = MPS_POST_WAIT_MUS_POKE;
      }
      else
      {
        work->state = MPS_POST_WAIT_ALL_MUS_POKE;
      }
    }
    break;

    //全員分もらうのを待つ
  case MPS_POST_WAIT_MUS_POKE:
    if( MUS_COMM_CheckAllPostPokeData( work->commWork ) == TRUE )
    {
      const BOOL ret = MUS_COMM_Send_AllMusPokeData( work->commWork );
      if( ret == TRUE )
      {
        work->state = MPS_POST_WAIT_ALL_MUS_POKE;
      }
    }
    break;
  
    //親が全員へ送るのを待つ
  case MPS_POST_WAIT_ALL_MUS_POKE:
    if( MUS_COMM_CheckAllPostPokeData( work->commWork ) == TRUE )
    {
      MUS_COMM_SendTimingCommand( work->commWork , MUS_COMM_TIMMING_START_ACTING );
      work->state = MPS_POST_WAIT_START_ACTING;
    }
    break;
  
    //全員そろうのを待つ
  case MPS_POST_WAIT_START_ACTING:
    if( MUS_COMM_CheckTimingCommand( work->commWork , MUS_COMM_TIMMING_START_ACTING ) == TRUE )
    {
      work->state = MPS_INIT_ACTING_COMM;
    }
    break;

  case MPS_INIT_ACTING:
    {
      u8 ePos,i;
      ARI_TPrintf("FreeHeap:[%x][%x]\n", 
          GFL_HEAP_GetHeapFreeSize( GFL_HEAPID_APP ) ,
          GFI_HEAP_GetHeapAllocatableSize( GFL_HEAPID_APP ) );
      work->actInitWork = MUSICAL_STAGE_CreateStageWork( HEAPID_MUSICAL_PROC , work->commWork );
      MUSICAL_STAGE_SetData_Player( work->actInitWork , 1 , work->musPoke );
      MUSICAL_STAGE_SetData_NPC( work->actInitWork , 0 , MONSNO_ONOKKUSU , HEAPID_MUSICAL_PROC );
      MUSICAL_STAGE_SetData_NPC( work->actInitWork , 2 , MONSNO_PIKATYUU  , HEAPID_MUSICAL_PROC );
      MUSICAL_STAGE_SetData_NPC( work->actInitWork , 3 , MONSNO_WARUBIARU , HEAPID_MUSICAL_PROC );

      MUSICAL_STAGE_SetEquip( work->actInitWork , 0 , MUS_POKE_EQU_EAR_L , 7 , 0 , 0 );
      MUSICAL_STAGE_SetEquip( work->actInitWork , 0 , MUS_POKE_EQU_HEAD  ,15 , 0 , 1 );
      MUSICAL_STAGE_SetEquip( work->actInitWork , 0 , MUS_POKE_EQU_HAND_L,13 , 0 , 2 );

      MUSICAL_STAGE_SetEquip( work->actInitWork , 2 , MUS_POKE_EQU_EAR_R  ,  2 , 0 , 0 );
      MUSICAL_STAGE_SetEquip( work->actInitWork , 2 , MUS_POKE_EQU_EYE    , 11 , 0 , 1 );
      MUSICAL_STAGE_SetEquip( work->actInitWork , 2 , MUS_POKE_EQU_BODY   , 27 , 0 , 2 );

      MUSICAL_STAGE_SetEquip( work->actInitWork , 3 , MUS_POKE_EQU_FACE   , 21 , 0 , 0 );
      MUSICAL_STAGE_SetEquip( work->actInitWork , 3 , MUS_POKE_EQU_HAND_R , 30 , 0 , 1 );
    }
    work->actInitWork->distData = work->distData;
    work->actInitWork->progWork = work->progWork;
    MUSICAL_PROGRAM_CalcPokemonPoint( HEAPID_MUSICAL_PROC , work->progWork , work->actInitWork );
    GFL_PROC_LOCAL_CallProc( work->procSys , NO_OVERLAY_ID, &MusicalStage_ProcData, work->actInitWork );
    work->state = MPS_TERM_ACTING;
    break;

  case MPS_INIT_ACTING_COMM:
    {
      u8 i;
      work->actInitWork = MUSICAL_STAGE_CreateStageWork( HEAPID_MUSICAL_PROC , work->commWork );
      for( i=0;i<MUSICAL_POKE_MAX;i++ )
      {
        MUSICAL_POKE_PARAM *musPoke = MUS_COMM_GetMusPokeParam( work->commWork , i );
        if( musPoke != NULL )
        {
          MUSICAL_STAGE_SetData_Comm( work->actInitWork , i , musPoke );
        }
        else
        {
          MUSICAL_STAGE_SetData_NPC( work->actInitWork , i , MONSNO_PIKATYUU  , HEAPID_MUSICAL_PROC );
        }
      }
      work->actInitWork->distData = work->distData;
      work->actInitWork->progWork = work->progWork;
      MUSICAL_PROGRAM_CalcPokemonPoint( HEAPID_MUSICAL_PROC , work->progWork , work->actInitWork );
      GFL_PROC_LOCAL_CallProc( work->procSys , NO_OVERLAY_ID, &MusicalStage_ProcData, work->actInitWork );
      work->state = MPS_TERM_ACTING;
    }
    
    break;

  case MPS_TERM_ACTING:
    if( isActiveProc == GFL_PROC_MAIN_NULL )
    {
      work->state = MPS_FINISH;
    }
    break;
    
  case MPS_FINISH:
    return GFL_PROC_RES_FINISH;
    break;
  }
  
  if( work->commWork != NULL )
  {
    MUS_COMM_UpdateComm( work->commWork );
  }
  
  return GFL_PROC_RES_CONTINUE;
}

*/