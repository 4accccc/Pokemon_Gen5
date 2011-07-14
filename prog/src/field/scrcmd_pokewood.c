//============================================================================================
/**
 *	@file		scrcmd_pokewood.c
 *	@brief  ポケウッドスクリプトコマンド
 *	@author	tamada  GAME FREAK inc.
 *  @date   2011.06.30
 *
 */
//============================================================================================

#include "gflib.h"
#include "system/gfl_use.h"
#include "system/vm_cmd.h"
#include "system/main.h"


#include "script.h"
#include "script_def.h"
#include "script_local.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "fieldmap.h"

#include "fieldmap_func.h"

#include "scrcmd_pokewood.h"
#include "event_pokewood.h"

#include "arc/message.naix"

#include "app/pkwd_book.h"  //PKWD_BOOK_PARAM
#include "pokewood_setup.h" //PKWD_BOOK_PARAM_Create

#include "field/pokewood.h"
#include "pokewood_system.h"
#include "savedata/pokewood_rec.h"

#include "msg/msg_pw_movietitle.h"


//SDK_COMPILER_ASSERT( SCR_PKWD_SCENARIO_MAX == POKEWOOD_SENARIO_MAX );
SDK_COMPILER_ASSERT( SCR_PKWD_MOVIE_MAX == PKWD_REC_POS_MAX );

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//============================================================================================
//============================================================================================

//============================================================================================

//--------------------------------------------------------------
//--------------------------------------------------------------
static PKWDSYS * getPOKEWOODSys( SCRCMD_WORK * work )
{
  GAMEDATA* gamedata    = SCRCMD_WORK_GetGameData( work );
  PKWDSYS ** pkwdsys_work = GAMEDATA_GetPKWDSysPointer( gamedata );
  if ( *pkwdsys_work == NULL )
  {
    OS_Printf("POKEWOODシステムが起動していません！！\n");
    GF_ASSERT(0);
  }
  return *pkwdsys_work;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTシステム開始
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSystemStart( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK* work     = (SCRCMD_WORK*)wk;
  GAMEDATA   * gamedata = SCRCMD_WORK_GetGameData( work );
  PKWDSYS   ** pkwdsys_work;

  MORI_Printf("EvCmdPKWDSystemStart\n");
  pkwdsys_work = GAMEDATA_GetPKWDSysPointer( gamedata );
  GF_ASSERT( *pkwdsys_work == NULL );
  *pkwdsys_work = PKWDSYS_Create( HEAPID_PROC );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：WBTシステム終了
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSystemEnd( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK* work     = (SCRCMD_WORK*)wk;
  GAMEDATA   * gamedata = SCRCMD_WORK_GetGameData( work );
  PKWDSYS   ** pkwdsys_work;

  MORI_Printf("EvCmdPKWDSystemEnd\n");
  pkwdsys_work = GAMEDATA_GetPKWDSysPointer( gamedata );
  GF_ASSERT( *pkwdsys_work != NULL );
  PKWDSYS_Delete( *pkwdsys_work );
  *pkwdsys_work = NULL;
  return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド：ポケウッド録画アプリ呼び出し
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodRec( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK * work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK * scw  = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GMEVENT     * next_event;
  POKEWOOD_EVENT_PARAM pw_param;
  u16* ret_wk;
  pw_param.type       = SCRCMD_GetVMWorkValue( core, work );
  pw_param.senario_no = SCRCMD_GetVMWorkValue( core, work );
  ret_wk = SCRCMD_GetVMWork(core,work);  

  next_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), POKEWOOD_EVENT_Rec, (void*)&pw_param );
  SCRIPT_CallEvent( scw, next_event );

  OS_Printf("EvCmdPokewoodRec( type = %d, scenario = %d\n", pw_param.type, pw_param.senario_no );
  return VMCMD_RESULT_SUSPEND;
}


//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：参加ポケモンを選択する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPKWDSelectPokemon( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK  *work   = wk;
  SCRIPT_WORK  *sc     = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys   = SCRIPT_GetGameSysWork( sc );
  u16          *ret_wk = SCRCMD_GetVMWork( core, wk );
  PKWDSYS       *pkwdsys = getPOKEWOODSys( wk );
  GMEVENT      *event;

  event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_EVENT_SetSelectPokeList, (void *)pkwdsys );
  TAMADA_Printf("EvCmdPKWDSelectPokemon\n");
  SCRIPT_CallEvent( sc, event );
  PKWDSYS_SetScriptAnswerWork( pkwdsys, ret_wk );

  return VMCMD_RESULT_SUSPEND;
}



//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド：ポケウッド上映アプリ呼び出し
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodShow( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  int rec_id =  SCRCMD_GetVMWorkValue( core, work );
  GMEVENT *next_event;
  next_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), POKEWOOD_EVENT_Show, (void*)&rec_id );
  SCRIPT_CallEvent( scw, next_event );

  OS_Printf("EvCmdPokewoodShow( rec_id = %d\n", rec_id );
  return VMCMD_RESULT_SUSPEND;
}

// ----------------------------------------
// ----------------------------------------
static GMEVENT * createDummyEvent( void * wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );

	GMEVENT *new_event;
	PKWD_BOOK_PARAM 	*pkwd_param = PKWD_BOOK_PARAM_Create( heapID, gsys, NULL );

	new_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_BOOK_EVENT_Show, (void *)pkwd_param );

  return new_event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド：ポケウッド台本画面呼び出し
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSelectScenario( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  

	GMEVENT *new_event;
	PKWD_BOOK_PARAM 	*pkwd_param = PKWD_BOOK_PARAM_Create( heapID, gsys, ret_wk );

	new_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_BOOK_EVENT_Show, (void *)pkwd_param );

  PKWD_BOOK_PARAM_Delete( pkwd_param );
	SCRIPT_CallEvent( scw, new_event );

  return VMCMD_RESULT_SUSPEND;
}

//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド上映選択画面呼び出し
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSelectMovie( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  

  GMEVENT * new_event = createDummyEvent( wk );
	SCRIPT_CallEvent( scw, new_event );

  *ret_wk = 0;  /* 仮：本当はアプリから結果を受け取る */

  return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
//============================================================================================
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static BOOL isEnableRecord( void * wk, PKWD_REC_POS rec_id )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gamedata );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );

  BOOL result = FALSE;
  LOAD_RESULT load_res;

  OS_Printf("load id=%d\n", rec_id );
  POKEWOOD_REC_Init( heapID );

  load_res = POKEWOOD_REC_Load( sv, heapID, rec_id );
  if ( load_res == LOAD_RESULT_OK && POKEWOOD_REC_IsExist() == TRUE )
  {
    result = TRUE;
  }
  POKEWOOD_REC_Exit();
  return result;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static u32 getEnableRecordCount( void * wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gamedata );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );

  u32 rec_id, count = 0;
  LOAD_RESULT load_res;

  POKEWOOD_REC_Init( heapID );
  for ( rec_id = 0; rec_id < PKWD_REC_POS_MAX; rec_id ++ )
  {
    OS_Printf("load id=%d\n", rec_id );

    load_res = POKEWOOD_REC_Load( sv, heapID, rec_id );
    if ( load_res == LOAD_RESULT_OK && POKEWOOD_REC_IsExist() == TRUE )
    {
      count ++;
    }
  }
  POKEWOOD_REC_Exit();
  return count;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static u32 getRecordInfo(
    void * wk, PKWD_REC_POS rec_id, POKEWOOD_REC_PARAM prm_id, void * ptr  )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gamedata = GAMESYSTEM_GetGameData( gsys );
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( gamedata );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );

  u32 result;
  LOAD_RESULT load_res;

  OS_Printf("load id=%d\n", rec_id );
  POKEWOOD_REC_Init( heapID );

  load_res = POKEWOOD_REC_Load( sv, heapID, rec_id );
  if ( load_res == LOAD_RESULT_OK )
  {
    result = POKEWOOD_REC_GetData( prm_id, ptr );
  }
  else
  {
    GF_ASSERT_MSG( 0, "存在しない録画データを参照\n" );
    result = 0;
  }
  POKEWOOD_REC_Exit();
  return result;
}

//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド録画データの存在チェック
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodCheckMovieEnable( VMHANDLE * core, void * wk )
{

  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  u16   rec_id = SCRCMD_GetVMWorkValue( core, work );
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );

  *ret_wk = isEnableRecord( wk, rec_id );
  OS_Printf("EvCmdPokewoodCheckMovieEnable( rec_id = %d, result = %d\n", rec_id, *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド録画データの評価を取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodGetMovieValue( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  u16   rec_id = SCRCMD_GetVMWorkValue( core, work );
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );

  *ret_wk = getRecordInfo( wk, rec_id, POKEWOOD_REC_PARAM_SUCCESS, NULL );
  OS_Printf("EvCmdPokewoodGetMovieValue( rec_id = %d, result = %d\n", rec_id, *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド：ポケウッドシナリオ名をバッファにセット
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSetScenarioName( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  SCRIPT_WORK*      sc = SCRCMD_WORK_GetScriptWork( work );
  WORDSET*     wordset = SCRIPT_GetWordSet( sc );
  u8               idx = VMGetU8(core);
  u16           rec_id = SCRCMD_GetVMWorkValue( core, work );
  HEAPID        heapID = SCRCMD_WORK_GetHeapID( work );

  u32           msg_id;
  
  msg_id = pw_title_00 + getRecordInfo( wk, rec_id, POKEWOOD_REC_PARAM_SENARIO, NULL );
  {
    GFL_MSGDATA     *man;
    STRBUF * strbuf = SCRIPT_GetMsgTempBuffer( sc );
    man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_pw_movietitle_dat, heapID );
    if( man )
    {
      GFL_MSG_GetString( man, msg_id, strbuf );
      GFL_MSG_Delete( man );
    }
    WORDSET_RegisterWord( wordset, idx, strbuf, PM_NEUTRAL, TRUE, PM_LANG );
  }
  OS_Printf("EvCmdPokewoodSetScenarioName( rec_id = %d, scenario = %d\n", rec_id, msg_id );
  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド  台本追加条件チェック
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodGetScenarioStatus( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );
  /* 仮：シナリオ追加状態を入れる */
  *ret_wk = 0;
  return VMCMD_RESULT_CONTINUE;
}
//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド　上映データ数取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodGetMoviesCount( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );

  *ret_wk = getEnableRecordCount( wk );
  return VMCMD_RESULT_CONTINUE;
}
//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド  上映データ削除
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodDeleteMovie( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  u16 delete_id = SCRCMD_GetVMWorkValue( core, work );
  /* 仮：削除処理をここに追加　*/

  return VMCMD_RESULT_CONTINUE;
}
//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド  初回視聴フラグ取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodGetFirstTimeSeeingFlag( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );
  /* 仮： */
  *ret_wk = 0;
  return VMCMD_RESULT_CONTINUE;
}

#if 0
//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSetScenarioName( VMHANDLE * core, void * wk )
{
  return VMCMD_RESULT_CONTINUE;
}
#endif

