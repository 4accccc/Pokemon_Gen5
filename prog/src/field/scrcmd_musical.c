//======================================================================
/**
 * @file scr_musical.c
 * @brief スクリプトコマンド　ミュージカル関連
 * @author	Satoshi Nohara
 * @date	06.06.26
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "fieldmap.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "musical/musical_system.h"
#include "musical/musical_event.h"

#include "poke_tool/poke_tool.h"  //ドレスアップ仮データ用
#include "poke_tool/monsno_def.h" //ドレスアップ仮データ用
#include "event_fieldmap_control.h"

#include "field_sound.h"
#include "message.naix"
#include "msg/msg_musical_common.h"
#include "scrcmd_musical.h"

#include "../../../resource/fldmapdata/script/musical_scr_local.h"

#include "test/ariizumi/ari_debug.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// EVENT_MUSICAL_WORK
//--------------------------------------------------------------
typedef struct
{
  MUSICAL_INIT_WORK init;
  GMEVENT *event;
}EVENT_MUSICAL_WORK;

//======================================================================
//  proto
//======================================================================
static GMEVENT_RESULT event_Musical( GMEVENT *event, int *seq, void *work );

//======================================================================
//  スクリプトコマンド　ミュージカル
//======================================================================
//--------------------------------------------------------------
/**
 * ミュージカル：ミュージカル呼び出し
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMusicalCall( VMHANDLE *core, void *wk )
{
  GMEVENT *call_event;
  MUSICAL_INIT_WORK *init;
  EVENT_MUSICAL_WORK *ev_musical_work;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  
  u8  mode = VMGetU8(core);
  u16 pokeIdx = SCRCMD_GetVMWorkValue( core, work );

  call_event = GMEVENT_Create(
      gsys, NULL, event_Musical, sizeof(EVENT_MUSICAL_WORK) );
  ev_musical_work = GMEVENT_GetEventWork( call_event );
	MI_CpuClear8( ev_musical_work, sizeof(EVENT_MUSICAL_WORK) );
  
  init = &ev_musical_work->init;
  init->saveCtrl = GAMEDATA_GetSaveControlWork( gdata );
  init->pokePara = PP_Create(
      MONSNO_PIKUSII , 20, PTL_SETUP_POW_AUTO , HEAPID_PROC );
  init->isComm = FALSE;
  init->isDebug = FALSE;
  init->gameComm = GAMESYSTEM_GetGameCommSysPtr( gsys );
  
//  ev_musical_work->event = EVENT_FieldSubProc( gsys, fparam->fieldMap,
//    NO_OVERLAY_ID, &Musical_ProcData, init );
  ev_musical_work->event = MUSICAL_CreateEvent( gsys, gdata , FALSE );
   
  //スクリプト終了後、指定したイベントに遷移する
  SCRIPT_EntryNextEvent( sc, call_event );
  
  {
    FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
    PMSND_StopBGM();
  //  FIELD_SOUND_PushBGM( fsnd );
  //  PMSND_FadeOutBGM( 30 );
  }
  
  VM_End( core );
  //return VMCMD_RESULT_CONTINUE;;
  return VMCMD_RESULT_SUSPEND;;
}

//--------------------------------------------------------------
/**
 * ミュージカル：ミュージカル系ワードセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMusicalFittingCall( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;

  u16 pokeIdx = SCRCMD_GetVMWorkValue( core, work );

  return VMCMD_RESULT_CONTINUE;
}

VMCMD_RESULT EvCmdGetMusicalValue( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;

  u8   type = VMGetU8(core);
  u16  val = SCRCMD_GetVMWorkValue( core, work );
  u16* ret_wk = SCRCMD_GetVMWork( core, work );

  return VMCMD_RESULT_CONTINUE;
}

VMCMD_RESULT EvCmdGetMusicalFanValue( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;

  u16  pos  = SCRCMD_GetVMWorkValue( core, work );
  u8   type = VMGetU8(core);
  u16* ret_wk = SCRCMD_GetVMWork( core, work );

  return VMCMD_RESULT_CONTINUE;
}

VMCMD_RESULT EvCmdGetMusicalWaitRoomValue( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );

  u8   type = VMGetU8(core);
  u16  val   = SCRCMD_GetVMWorkValue( core, work );
  u16* ret_wk = SCRCMD_GetVMWork( core, work );

  MUSICAL_EVENT_WORK *evWork = SCRIPT_GetMemberWork( sc , ID_EVSCR_MUSICAL_EVENT_WORK );

  switch( type )
  {
  case MUSICAL_VALUE_WR_SELF_IDX:  //自分の参加番号
    *ret_wk = MUSICAL_EVENT_GetSelfIndex( evWork );
    ARI_TPrintf("MusValWR:SelfIdx[%d]\n",*ret_wk);
    break;
  case MUSICAL_VALUE_WR_MAX_POINT: //最高評価点
    *ret_wk = MUSICAL_EVENT_GetMaxPoint( evWork );
    ARI_TPrintf("MusValWR:MaxPoint[%d]\n",*ret_wk);
    break;
  case MUSICAL_VALUE_WR_MIN_POINT: //最低評価点
    *ret_wk = MUSICAL_EVENT_GetMinPoint( evWork );
    ARI_TPrintf("MusValWR:MinPoint[%d]\n",*ret_wk);
    break;
  case MUSICAL_VALUE_WR_POINT:     //個人得点
    *ret_wk = MUSICAL_EVENT_GetPoint( evWork , val );
    ARI_TPrintf("MusValWR:Point[%d:%d]\n",val,*ret_wk);
    break;
  case MUSICAL_VALUE_WR_GRADE_MSG: //評価メッセージの取得
    break;
  case MUSICAL_VALUE_WR_POS_TO_IDX: //立ち位置に対応した参加番号
    *ret_wk = MUSICAL_EVENT_GetPosIndex( evWork , val );
    ARI_TPrintf("MusValWR:PosIdx[%d:%d]\n",val,*ret_wk);
    break;
  case MUSICAL_VALUE_WR_MAX_CONDITION: //演目の高いコンディション
    *ret_wk = MUSICAL_EVENT_GetMaxCondition( evWork  );
    ARI_TPrintf("MusValWR:MaxCondition[%d]\n",*ret_wk);
    break;
  case MUSICAL_VALUE_WR_POS_TO_RANK://順位に対応した参加番号
    *ret_wk = MUSICAL_EVENT_GetPosToRank( evWork , val );
    ARI_TPrintf("MusValWR:PosRank[%d:%d]\n",val,*ret_wk);
    break;
  case MUSICAL_VALUE_WR_POKE_MAX_POINT_CON: //最も点が高かったコンディション
    *ret_wk = MUSICAL_EVENT_GetMaxPointCondition( evWork , val );
    ARI_TPrintf("MusValWR:MaxPointCon[%d:%d]\n",val,*ret_wk);
    break;
  default:
    GF_ASSERT_MSG( 0 , "タイプ指定が間違ってる[%d]\n",type );
    break;
  }

  return VMCMD_RESULT_CONTINUE;
}

VMCMD_RESULT EvCmdAddMusicalGoods( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16  val   = SCRCMD_GetVMWorkValue( core, work );

  return VMCMD_RESULT_CONTINUE;
}

VMCMD_RESULT EvCmdMusicalWord( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  HEAPID heapId = SCRCMD_WORK_GetHeapID( work );
  WORDSET **wordset    = SCRIPT_GetMemberWork( sc, ID_EVSCR_WORDSET );

  u8   type = VMGetU8(core);
  u8   idx  = VMGetU8(core);
  u16  val  = SCRCMD_GetVMWorkValue( core, work );

  MUSICAL_EVENT_WORK *evWork = SCRIPT_GetMemberWork( sc , ID_EVSCR_MUSICAL_EVENT_WORK );

  switch( type )
  {
  case MUSICAL_WORD_TITLE:        //セーブにある演目
    break;
  case MUSICAL_WORD_GOODS:        //グッズ名
    break;
  case MUSICAL_WORD_TITLE_LOCAL:  //※現在演目
    {
      STRBUF * tmpBuf = MUSICAL_EVENT_CreateStr_ProgramTitle( evWork , heapId );
      WORDSET_RegisterWord( *wordset, idx, tmpBuf, 0, TRUE, PM_LANG );
      GFL_STR_DeleteBuffer( tmpBuf );
    }
    break;
  case MUSICAL_WORD_AUDI_TYPE:    //人気客層
    {
      STRBUF * tmpBuf;
      GFL_MSGDATA *msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_musical_common_dat , heapId );

      tmpBuf = GFL_MSG_CreateString( msgHandle , MUSICAL_AUDIENCE_TYPE_01+val );
      WORDSET_RegisterWord( *wordset, idx, tmpBuf, 0, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer( tmpBuf );
      GFL_MSG_Delete( msgHandle );
    }
    break;
  case MUSICAL_WORD_CONDITION:  //コンディション名
    {
      STRBUF * tmpBuf;
      GFL_MSGDATA *msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_musical_common_dat , heapId );

      tmpBuf = GFL_MSG_CreateString( msgHandle , MUSICAL_AUDIENCE_CON_01+val );
      WORDSET_RegisterWord( *wordset, idx, tmpBuf, 0, TRUE, PM_LANG );

      GFL_STR_DeleteBuffer( tmpBuf );
      GFL_MSG_Delete( msgHandle );
    }
    break;
  }

  return VMCMD_RESULT_CONTINUE;
}


//======================================================================
//  ミュージカル　イベント部分
//======================================================================
//--------------------------------------------------------------
/**
 * @param
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT_RESULT event_Musical(
    GMEVENT *event, int *seq, void *work )
{
  EVENT_MUSICAL_WORK *ev_musical_work = work;
  
  switch( (*seq) ){
  case 0:
    GMEVENT_CallEvent( event, ev_musical_work->event );
    (*seq)++;
    break;
  case 1:
    {
      MUSICAL_INIT_WORK *init = &ev_musical_work->init;
      GFL_HEAP_FreeMemory( init->pokePara );
    }

    {
      GAMESYS_WORK *gsys =  GMEVENT_GetGameSysWork( event );
      GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
      FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
      //FIELD_SOUND_PopBGM( fsnd );
      PMSND_FadeInBGM( 30 );
    }
    (*seq)++;
    return( GMEVENT_RES_FINISH );
  }
  return( GMEVENT_RES_CONTINUE );
}
