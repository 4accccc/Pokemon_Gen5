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
#include "savedata/pokewood_savedata.h"

#include "msg/msg_pw_movietitle.h"

#include "../battle/pokewood/pokewood_senario.h"
#include "tr_tool/trtype_def.h"

//SDK_COMPILER_ASSERT( SCR_PKWD_SCENARIO_MAX == POKEWOOD_SENARIO_MAX );
SDK_COMPILER_ASSERT( SCR_PKWD_MOVIE_MAX == PKWD_REC_POS_MAX );

SDK_COMPILER_ASSERT(PWSV_TYPE_SENARIO_OPEN == SCR_PKWD_SCENARIO_OPEN_FLAG );
SDK_COMPILER_ASSERT(PWSV_TYPE_SENARIO_PLAYED == SCR_PKWD_SCENARIO_PLAYED_FLAG );
SDK_COMPILER_ASSERT(PWSV_TYPE_SENARIO_SUCCESS == SCR_PKWD_SCENARIO_SUCCESS_FLAG );
SDK_COMPILER_ASSERT(PWSV_TYPE_SENARIO_ANNOUNCED == SCR_PKWD_SCENARIO_ANNOUNCED_FLAG );

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
GMEVENT * PKWD_EVENT_CopyMovie(
    GAMESYS_WORK * gsys, PKWD_REC_POS src_pos, PKWD_REC_POS dst_pos );
//============================================================================================
//============================================================================================

//============================================================================================
//--------------------------------------------------------------
/**
 * @todo  テーブルを完成させる
 */
//--------------------------------------------------------------
static const u16 ActorTRType2MMdlID( u16 trtype )
{
  static const u16 trtype2mmdl[][2] = {
    {TRTYPE_LEADER7,  LEADER7 },
  };

  int i;
  for ( i = 0; i < NELEMS(trtype2mmdl); i ++ )
  {
    if ( trtype2mmdl[i][0] == trtype )
    {
      return trtype2mmdl[i][1];
    }
  }
  return BOY2;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static const u16 DirectorIDtoMMdlID( u8 director_id )
{
  static const u16 mmdl_table[5] = {
    OLDMAN1,
    SUNGLASSES,
    MIDDLEMAN2,
    GENTLEMAN,
    ADULTW,
  };

  GF_ASSERT( director_id < 5 );
  return mmdl_table[director_id];
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static const POKEWOOD_SENARIO * allocScenarioData( HEAPID heapID, u32 scenario_no )
{
  POKEWOOD_SENARIO * scenario;
  scenario = (POKEWOOD_SENARIO*)GFL_ARC_LoadDataAlloc( ARCID_POKEWOOD, scenario_no, heapID);
  return scenario;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef enum {
  PKWD_SC_PARAM_DIRECTOR = 0,
  PKWD_SC_PARAM_TRTYPE,
}PKWD_SC_PARAM_ID;

//--------------------------------------------------------------
//--------------------------------------------------------------
static u16 readScenarioData( u32 scenario_no, PKWD_SC_PARAM_ID param_id )
{
  u16 value = 0;
  const POKEWOOD_SENARIO * scWork = allocScenarioData( HEAPID_PROC, scenario_no );
  switch ( param_id )
  {
  case PKWD_SC_PARAM_DIRECTOR:
    value = scWork->director;
    break;
  case PKWD_SC_PARAM_TRTYPE:
    value = scWork->trtype[0]; //仮
    break;
  default:
    GF_ASSERT(0);
  }

  GFL_HEAP_FreeMemory( (void*)scWork );
  return value;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static POKEWOOD_SAVEDATA * getPKWDSaveData( SCRCMD_WORK * work )
{
  GAMEDATA * gamedata = SCRCMD_WORK_GetGameData( work );
  return GAMEDATA_GetPokewoodSavedata( gamedata );
}

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
 * @brief スクリプトコマンド：ポケウッドシステム開始
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
 * @brief スクリプトコマンド：ポケウッドシステム終了
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
 *	@brief  スクリプトコマンド：ポケウッド撮影アプリ呼び出し
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodRec( VMHANDLE *core, void *wk )
{
  PKWDSYS      *pkwdsys = getPOKEWOODSys( wk );
  SCRCMD_WORK * work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK * scw  = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GMEVENT     * next_event;
  PKWD_REC_EVENT_PARAM pw_param;
  u16* ret_wk;
  pw_param.rec_pos      = SCRCMD_GetVMWorkValue( core, work );
  pw_param.scenario_no  = SCRCMD_GetVMWorkValue( core, work );
  ret_wk = SCRCMD_GetVMWork(core,work);  
  PKWDSYS_SetScriptAnswerWork( pkwdsys, ret_wk );

  next_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), POKEWOOD_EVENT_Rec, (void*)&pw_param );
  SCRIPT_CallEvent( scw, next_event );

  OS_Printf("EvCmdPokewoodRec( rec_pos = %d, scenario = %d\n", pw_param.rec_pos, pw_param.scenario_no );
  return VMCMD_RESULT_SUSPEND;
}


//--------------------------------------------------------------
/**
 * @brief スクリプトコマンド：参加ポケモンを選択する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPKWDSelectPokemon( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK  *work    = wk;
  SCRIPT_WORK  *sc      = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK *gsys    = SCRIPT_GetGameSysWork( sc );
  u16          *ret_wk  = SCRCMD_GetVMWork( core, wk );
  PKWDSYS      *pkwdsys = getPOKEWOODSys( wk );
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
 *	@brief  手持ちポケモンを参加させるかどうかをスクリプトからもらう
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPKWDSetJoinMypoke( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*    gdata = GAMESYSTEM_GetGameData( gsys );
  PKWDSYS   *pkwdsys = getPOKEWOODSys( wk );

  PKWDSYS_SetJoinMypoke( pkwdsys, SCRCMD_GetVMWorkValue( core, work ));

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
  PKWDSYS **pkwdsys = GAMEDATA_GetPKWDSysPointer( gdata );

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
 * @brief スクリプトコマンド：ポケウッド  台本状態の取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodGetScenarioStatus( VMHANDLE * core, void * wk )
{
  const POKEWOOD_SAVEDATA * pkwdsave = getPKWDSaveData( wk );
  SCRCMD_WORK*    work = wk;
  u16 type    = SCRCMD_GetVMWorkValue( core, work );
  u16 sc_no   = SCRCMD_GetVMWorkValue( core, work );
  u16 * ret_wk  = SCRCMD_GetVMWork( core, work );
  
  *ret_wk = POKEWOOD_SAVEDATA_GetPokewoodFlag( pkwdsave, type, sc_no );
  OS_Printf("EvCmdPokewoodGetScenarioStatus(%d, %d) = %d\n", type, sc_no, *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド  台本状態のセット
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSetScenarioStatus( VMHANDLE * core, void * wk )
{
  POKEWOOD_SAVEDATA * pkwdsave = getPKWDSaveData( wk );
  SCRCMD_WORK*    work = wk;
  u16 type    = SCRCMD_GetVMWorkValue( core, work );
  u16 sc_no   = SCRCMD_GetVMWorkValue( core, work );
  
  POKEWOOD_SAVEDATA_SetPokewoodFlag( pkwdsave, type, sc_no );
  OS_Printf("EvCmdPokewoodSetScenarioStatus(%d, %d) <- %d\n", type, sc_no );
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
  OS_Printf("EvCmdPokewoodGetMoviesCount=%d\n", *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド　スターランク更新チェック
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodCheckNewStarRank( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );
  POKEWOOD_SAVEDATA * pkwdsave = getPKWDSaveData( wk );
  u32 point = POKEWOOD_SAVEDATA_GetStarPoint( pkwdsave );
  u32 rank = POKEWOOD_SAVEDATA_CalcStarPoint2StarRank( point );
  *ret_wk = POKEWOOD_SAVEDATA_GetStarRankAnnounced( pkwdsave, rank );
  POKEWOOD_SAVEDATA_SetStarRankAnnounced( pkwdsave, rank, TRUE );

  OS_Printf("EvCmdPokewoodCheckNewStarRank(%d-->%d) == %d\n", point, rank, *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}
//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド　スタジオの動作モデルコードを取得する
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSetStudioMMdl( VMHANDLE * core, void * wk )
{
  u16 sc_no   = SCRCMD_GetVMWorkValue( core, wk );
  u16 * mmdl1 = SCRCMD_GetVMWork( core, wk );
  u16 * mmdl2 = SCRCMD_GetVMWork( core, wk );

  //シナリオから判別してセットする
  *mmdl1 = ActorTRType2MMdlID( readScenarioData( sc_no, PKWD_SC_PARAM_TRTYPE ) );
  *mmdl2 = DirectorIDtoMMdlID( readScenarioData( sc_no, PKWD_SC_PARAM_DIRECTOR ) );

  TAMADA_Printf("EvCmdPokewoodSetStudioMMdl(%d)-->%d,%d\n", sc_no, *mmdl1, *mmdl2 );
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

//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodCopyMovie( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK * scw  = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GMEVENT     * next_event;

  u16 rec_pos = SCRCMD_GetVMWorkValue( core, work );

  GF_ASSERT( rec_pos < PKWD_REC_POS_MAX );
  next_event = PKWD_EVENT_CopyMovie( gsys, PKWD_REC_POS_TMP, rec_pos );
  SCRIPT_CallEvent( scw, next_event );

  return VMCMD_RESULT_SUSPEND;
}

//============================================================================================
//  PKWD　イベント　映画をコピー
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
  GAMEDATA * gamedata;
  PKWD_REC_POS src_pos;
  PKWD_REC_POS dst_pos;
  u16 save_tmp;
}EVENT_POKEWOOD_COPY_MOVE_WORK, EPCMW;

//--------------------------------------------------------------
//--------------------------------------------------------------
static GMEVENT_RESULT eventPokewoodCopyMovie( GMEVENT * event, int *seq, void * wk )
{
  typedef enum {
    PKWD_COPY_SEQ_INIT = 0,
    PKWD_COPY_SEQ_LOAD,
    PKWD_COPY_SEQ_SAVE,
    PKWD_COPY_SEQ_END,
  }PKWD_COPY_SEQ;

  EPCMW * epcmw = (EPCMW *)wk;

  LOAD_RESULT load_res;
  SAVE_RESULT save_res;
  SAVE_CONTROL_WORK * sv = GAMEDATA_GetSaveControlWork( epcmw->gamedata );
  HEAPID heapID = HEAPID_FIELDMAP;


  switch ((PKWD_COPY_SEQ)*seq)
  {
  case PKWD_COPY_SEQ_INIT:
    OS_Printf("POKEWOOD Move Copy Start(%d -> %d)\n", epcmw->src_pos, epcmw->dst_pos );
    POKEWOOD_REC_Init( heapID );
    *seq = PKWD_COPY_SEQ_LOAD;
    /* FALL THROUGH */

  case PKWD_COPY_SEQ_LOAD:
    OS_Printf("POKEWOOD Move Copy Load\n");
    load_res = POKEWOOD_REC_Load( sv, heapID, epcmw->src_pos );
    if ( load_res == LOAD_RESULT_OK && POKEWOOD_REC_IsExist() == TRUE )
    {
      *seq = PKWD_COPY_SEQ_SAVE;
    }
    else
    {
      *seq = PKWD_COPY_SEQ_END;
    }
    break;

  case PKWD_COPY_SEQ_SAVE:
    OS_Printf("POKEWOOD Move Copy Save\n");
    save_res = POKEWOOD_REC_Save( epcmw->gamedata, heapID, epcmw->dst_pos, &epcmw->save_tmp );
    switch ( save_res )
    {
    case SAVE_RESULT_NG:
      (*seq) = PKWD_COPY_SEQ_END;
      break;
    case SAVE_RESULT_OK:
      (*seq) = PKWD_COPY_SEQ_END;
      break;
    }
    break;

  case PKWD_COPY_SEQ_END:
    POKEWOOD_REC_Exit();
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * PKWD_EVENT_CopyMovie(
    GAMESYS_WORK * gsys, PKWD_REC_POS src_pos, PKWD_REC_POS dst_pos )
{
  GMEVENT * event;
  EPCMW * epcmw;
  event = GMEVENT_Create( gsys, NULL, eventPokewoodCopyMovie, sizeof(EPCMW) );
  epcmw = GMEVENT_GetEventWork( event );
  epcmw->gamedata = GAMESYSTEM_GetGameData( gsys );
  epcmw->src_pos = src_pos;
  epcmw->dst_pos = dst_pos;
  epcmw->save_tmp = 0;
  return event;
}

