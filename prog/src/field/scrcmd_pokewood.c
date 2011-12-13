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
#include "app/pkwd_record_monitor.h"  //PKWD_RECORD_MONITOR

#include "field/pokewood.h"
#include "pokewood_system.h"
#include "savedata/pokewood_rec.h"
#include "savedata/pokewood_savedata.h"
#include "pkwd_scenario_open.h"

#include "msg/msg_pw_movietitle.h"

#include "../battle/pokewood/pokewood_senario.h"
#include "tr_tool/trtype_def.h"
#include "pkwd_scenario_open.h"

// ご褒美データ
#include "../../../resource/pokewood_etc/pokewood_bonus.cdat"
// TRTYPE→動作モデル変換テーブル
#include "../../../resource/pokewood_etc/pokewood_tr2mmdl.cdat"

//SDK_COMPILER_ASSERT( SCR_PKWD_SCENARIO_MAX == POKEWOOD_SENARIO_MAX );
SDK_COMPILER_ASSERT( SCR_PKWD_MOVIE_MAX == PKWD_REC_POS_MAX );

SDK_COMPILER_ASSERT(PWSV_TYPE_SENARIO_OPEN == SCR_PKWD_SCENARIO_OPEN_FLAG );
SDK_COMPILER_ASSERT(PWSV_TYPE_SENARIO_PLAYED == SCR_PKWD_SCENARIO_PLAYED_FLAG );
SDK_COMPILER_ASSERT(PWSV_TYPE_SENARIO_SUCCESS == SCR_PKWD_SCENARIO_SUCCESS_FLAG );
SDK_COMPILER_ASSERT(PWSV_TYPE_SENARIO_ANNOUNCED == SCR_PKWD_SCENARIO_ANNOUNCED_FLAG );

SDK_COMPILER_ASSERT( SCR_PKWD_SELECT_MODE_NORMAL == PKWD_SELECT_MODE_NORMAL );
SDK_COMPILER_ASSERT( SCR_PKWD_SELECT_MODE_DELETE == PKWD_SELECT_MODE_DEL );

SDK_COMPILER_ASSERT( SCR_PKWD_STAR_RANK_0 == POKEWOOD_STAR_RANK_0 );
SDK_COMPILER_ASSERT( SCR_PKWD_STAR_RANK_1 == POKEWOOD_STAR_RANK_1 );
SDK_COMPILER_ASSERT( SCR_PKWD_STAR_RANK_2 == POKEWOOD_STAR_RANK_2 );
SDK_COMPILER_ASSERT( SCR_PKWD_STAR_RANK_3 == POKEWOOD_STAR_RANK_3 );
SDK_COMPILER_ASSERT( SCR_PKWD_STAR_RANK_4 == POKEWOOD_STAR_RANK_4 );

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
/**
 * @brief ポケウッド用トレーナータイプ→動作モデル変換
 */
//--------------------------------------------------------------
static const u16 ActorTRType2MMdlID( u16 trtype )
{
  int i;
  for ( i = 0; i < NELEMS(TrType2MMdl); i ++ )
  {
    if ( TrType2MMdl[i][0] == trtype )
    {
      return TrType2MMdl[i][1];
    }
  }
  GF_ASSERT( 0 );
  //error対処：テーブルから見つけられなかった場合はBOY2にする
  return BOY2;
}

//--------------------------------------------------------------
/**
 * @brief 監督の指定→動作モデル変換
 */
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

  if ( director_id >= NELEMS(mmdl_table) )
  {
    GF_ASSERT( 0 );
    director_id = 0;  //error対処
  }
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
  PKWD_SC_PARAM_TRTYPE_FOR_M,
  PKWD_SC_PARAM_TRTYPE_FOR_F,
  PKWD_SC_PARAM_DIFFICULT,
}PKWD_SC_PARAM_ID;

//--------------------------------------------------------------
/**
 * シナリオデータから必要な情報の読み込み
 */
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
  case PKWD_SC_PARAM_TRTYPE_FOR_M:
    value = scWork->trtype[POKEWOOD_TRTYPE_REC_MAN];
    break;
  case PKWD_SC_PARAM_TRTYPE_FOR_F:
    value = scWork->trtype[POKEWOOD_TRTYPE_SHOW_MAN];
    break;
  case PKWD_SC_PARAM_DIFFICULT:
    value = scWork->difficult;
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
  POKEWOOD_SAVEDATA * pkwdsave = GAMEDATA_GetPokewoodSavedata( gamedata );
  
  MORI_Printf("EvCmdPKWDSystemStart\n");
  pkwdsys_work = GAMEDATA_GetPKWDSysPointer( gamedata );
  GF_ASSERT( *pkwdsys_work == NULL );
  *pkwdsys_work = PKWDSYS_Create( HEAPID_PROC );

  // シナリオオープン条件チェック(ゲームクリア後などオープンに使う）
  GFL_OVERLAY_Load( FS_OVERLAY_ID(notwifi_event_pokewood) );  
  PKWD_OpenCheckScenario( pkwdsave, gamedata, 0, 0, 0 );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(notwifi_event_pokewood) );


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
  u16          *ret_wk  = SCRCMD_GetVMWork( core, wk );
  PKWDSYS      *pkwdsys = getPOKEWOODSys( wk );
  GMEVENT *next_event;

  next_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), POKEWOOD_EVENT_Show, (void*)&rec_id );

  SCRIPT_CallEvent( scw, next_event );
  PKWDSYS_SetScriptAnswerWork( pkwdsys, ret_wk );

  OS_Printf("EvCmdPokewoodShow( rec_id = %d\n", rec_id );
  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリプトコマンド：ポケウッド上映アプリ呼び出し（チュートリアル用）
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodShowTutorial( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  PKWDSYS      *pkwdsys = getPOKEWOODSys( wk );
  GMEVENT *next_event;

//  こっちはチュートリアル再生
  next_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), POKEWOOD_EVENT_TutorialShow, (void*)NULL );

  SCRIPT_CallEvent( scw, next_event );
  PKWDSYS_SetScriptAnswerWork( pkwdsys, NULL );

  OS_PutString("EvCmdPokewoodShowTutorial()\n");
  return VMCMD_RESULT_SUSPEND;
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
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  PKWD_SELECT_SCREEN_PARAM * screen_param;
  GMEVENT * new_event;

  u16 mode    = SCRCMD_GetVMWorkValue(core,work);
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  

  screen_param = PKWD_SELECT_SCREEN_PARAM_Create( heapID, gsys, mode, ret_wk );

  new_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_SELECT_SCREEN_EVENT_Call, (void*)screen_param );

  PKWD_SELECT_SCREEN_PARAM_Delete( screen_param );
	SCRIPT_CallEvent( scw, new_event );

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
 *
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodGetMovieValue( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  u16   rec_id = SCRCMD_GetVMWorkValue( core, work );
  u16 * ret_wk = SCRCMD_GetVMWork( core, work );

  switch ( getRecordInfo( wk, rec_id, POKEWOOD_REC_PARAM_SUCCESS, NULL ) )
  {
  default:
    GF_ASSERT( 0 );
    /* FALL THROUGH */
  case BTL_POKEWOOD_RESULT_BAD:
    *ret_wk = SCR_PKWD_RESULT_BAD_END;
    break;
  case BTL_POKEWOOD_RESULT_SUCCESS:
    *ret_wk = SCR_PKWD_RESULT_GOOD_END;
    break;
  case BTL_POKEWOOD_RESULT_STRANGE:
    *ret_wk = SCR_PKWD_RESULT_STRANGE_END;
    break;
  }
  OS_Printf("EvCmdPokewoodGetMovieValue( %d ) --> %d\n", rec_id, *ret_wk );
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
  u16           sc_no = SCRCMD_GetVMWorkValue( core, work );
  HEAPID        heapID = SCRCMD_WORK_GetHeapID( work );

  u32           msg_id;
  
  msg_id = pw_title_00 + sc_no;
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
  OS_Printf("EvCmdPokewoodSetScenarioName( scenario = %d, msg_id = %d\n", sc_no, msg_id );
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
 * @brief スクリプトコマンド：ポケウッド  進行フラグの総数の取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodGetFlagTotal( VMHANDLE * core, void * wk )
{
  POKEWOOD_SAVEDATA * pkwdsave = getPKWDSaveData( wk );
  SCRCMD_WORK*    work = wk;
  u16 type    = SCRCMD_GetVMWorkValue( core, work );
  u16 * ret_wk  = SCRCMD_GetVMWork( core, work );
  
  *ret_wk =  POKEWOOD_SAVEDATA_GetFlagTotal( pkwdsave, type );
  OS_Printf("EvCmdPokewoodGetFlagTotal(%d) = %d\n", type, *ret_wk );
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
 * @brief スクリプトコマンド：ポケウッド　新着台本のチェック
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodCheckNewScenario( VMHANDLE * core, void * wk )
{
  POKEWOOD_SAVEDATA * pkwdsave = getPKWDSaveData( wk );
  SCRCMD_WORK*    work = wk;
  u16 sc_no       = SCRCMD_GetVMWorkValue( core, work );
  u16 * msg_type  = SCRCMD_GetVMWork( core, work );
  u16 * ret_wk    = SCRCMD_GetVMWork( core, work );

  *ret_wk = FALSE;
  if ( POKEWOOD_SAVEDATA_GetPokewoodFlag( pkwdsave, PWSV_TYPE_SENARIO_OPEN, sc_no ) == TRUE
      && POKEWOOD_SAVEDATA_GetPokewoodFlag( pkwdsave, PWSV_TYPE_SENARIO_ANNOUNCED, sc_no ) == FALSE )
  {
    //告知済み状態にする
    POKEWOOD_SAVEDATA_SetPokewoodFlag( pkwdsave, PWSV_TYPE_SENARIO_ANNOUNCED, sc_no );

    GFL_OVERLAY_Load( FS_OVERLAY_ID(notwifi_event_pokewood) );
    switch ( PKWD_GetScenarioOpenProvision( sc_no ) )
    {
    default:
      GF_ASSERT( 0 );
      /* FALL THROUGH */
    case PW_OPEN_PROVISION_SUCCESS:
      *msg_type = SCR_PKWD_ADDMSG_GOOD_END;
      break;
    case PW_OPEN_PROVISION_GAMECLEAR:
      *msg_type = SCR_PKWD_ADDMSG_GAMECLEAR;
      break;
    case PW_OPEN_PROVISION_STARRANK:
      *msg_type = SCR_PKWD_ADDMSG_STARRANK;
      break;
    case PW_OPEN_PROVISION_PROFIT:
      *msg_type = SCR_PKWD_ADDMSG_PROFITS;
      break;
    case PW_OPEN_PROVISION_ALLGOOD:
      *msg_type = SCR_PKWD_ADDMSG_ALL_GOOD_END;
      break;
    case PW_OPEN_PROVISION_TUTORIAL:
      *msg_type = SCR_PKWD_ADDMSG_TUTORIAL;
      break;
    case PW_OPEN_PROVISION_DUAL:
      *msg_type = SCR_PKWD_ADDMSG_DUAL;
      break;
    }
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(notwifi_event_pokewood) );
    *ret_wk = TRUE;
  }
  OS_Printf("EvCmdPokewoodCheckNewScenario(%d) --> %d, %d\n", sc_no, *msg_type, *ret_wk );
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
 * @brief スクリプトコマンド：ポケウッド 上映済みかどうかの取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodIsShowedMovie( VMHANDLE * core, void * wk )
{
  POKEWOOD_SAVEDATA * pkwdsave = getPKWDSaveData( wk );
  u16 play_id = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  *ret_wk = POKEWOOD_SAVEDATA_GetMovieShowed( pkwdsave, play_id );
  TAMADA_Printf("EvCmdPokewoodIsShowedMovie(%d)-->%d\n", play_id, *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}
//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド 上映済みフラグのセット
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSetShowedMovie( VMHANDLE * core, void * wk )
{
  POKEWOOD_SAVEDATA * pkwdsave = getPKWDSaveData( wk );
  u16 play_id = SCRCMD_GetVMWorkValue( core, wk );

  POKEWOOD_SAVEDATA_SetMovieShowed( pkwdsave, play_id, TRUE );
  TAMADA_Printf("EvCmdPokewoodSetShowedMovie(%d)\n", play_id );
  return VMCMD_RESULT_CONTINUE;
}
//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド　出演ポケモンがレンタルかどうかチェック
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodIsRentalPokeMovie( VMHANDLE * core, void * wk )
{
  POKEWOOD_SAVEDATA * pkwdsave = getPKWDSaveData( wk );
  u16 play_id = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  *ret_wk = getRecordInfo( wk, play_id, POKEWOOD_REC_PARAM_RENTAL_FLAG, NULL );
  OS_Printf("EvCmdPokewoodIsRentalPokeMovie(%d)-->%d\n", play_id, *ret_wk );
  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド　スタジオの動作モデルコードを取得する
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSetStudioMMdl( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK * work = wk;
  GAMEDATA   * gamedata = SCRCMD_WORK_GetGameData( work );
  const MYSTATUS * mystat = GAMEDATA_GetMyStatus( gamedata );
  u8 sex = MyStatus_GetMySex( mystat );

  u16 sc_no   = SCRCMD_GetVMWorkValue( core, wk );
  u16 * mmdl1 = SCRCMD_GetVMWork( core, wk );
  u16 * mmdl2 = SCRCMD_GetVMWork( core, wk );
  PKWD_SC_PARAM_ID param_id;

  //シナリオから判別してセットする
  param_id = ( sex == PM_MALE ? PKWD_SC_PARAM_TRTYPE_FOR_M : PKWD_SC_PARAM_TRTYPE_FOR_F );
  *mmdl1 = ActorTRType2MMdlID( readScenarioData( sc_no, param_id ) );
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
 * @brief スクリプトコマンド：ポケウッド  映画をセーブ
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodSaveMovie( VMHANDLE * core, void * wk )
{
  POKEWOOD_SAVEDATA * pkwdsave = getPKWDSaveData( wk );

  SCRCMD_WORK * work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK * scw  = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA   * gamedata = SCRCMD_WORK_GetGameData( work );
  GMEVENT     *next_event;
  PKWDSYS     *pkwdsys = getPOKEWOODSys( wk );

  u16 rec_pos = SCRCMD_GetVMWorkValue( core, work );

  GF_ASSERT( rec_pos < PKWD_REC_POS_MAX );
  // 録画データ位置をPKWDSYSに通知
  PKWDSYS_SetPlayNo( pkwdsys, rec_pos );

  //updatePokewoodSaveParams( gamedata, pkwdsave, rec_pos );
  //next_event = PKWD_EVENT_Save( gsys, NULL );
  //SCRIPT_CallEvent( scw, next_event );

  return VMCMD_RESULT_SUSPEND;
}

//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド 監督の種類を取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodGetDirectorType( VMHANDLE *core, void * wk )
{
  u16 sc_no     = SCRCMD_GetVMWorkValue( core, wk );
  u16 * ret_wk  = SCRCMD_GetVMWork( core, wk );
  *ret_wk = readScenarioData( sc_no, PKWD_SC_PARAM_DIRECTOR );
  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief スクリプトコマンド：ポケウッド 全シナリオGOODEND達成したかどうか
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodCheckAllSuccess( VMHANDLE *core, void * wk )
{
  POKEWOOD_SAVEDATA * pkwdsave = getPKWDSaveData( wk );

  u16 * ret_wk  = SCRCMD_GetVMWork( core, wk );

  // 全ての映画で成功ENDにしたかチェック
  if(POKEWOOD_SAVEDATA_GetFlagTotal(pkwdsave, PWSV_TYPE_SENARIO_SUCCESS)==POKEWOOD_BOXOFFICE_DATA_MAX)
  {
    *ret_wk = TRUE;  //
  }
  else
  {
    *ret_wk = FALSE;  //
  }

  return VMCMD_RESULT_CONTINUE;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPokewoodRecordMonitor( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*  work = (SCRCMD_WORK*)wk;
  SCRIPT_WORK*   scw = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  HEAPID heapID = SCRCMD_WORK_GetHeapID( work );
  u16* ret_wk = SCRCMD_GetVMWork(core,work);  
  PKWDSYS **pkwdsys = GAMEDATA_GetPKWDSysPointer( gdata );

	GMEVENT *new_event;
	PKWD_RECORD_MONITOR_PARAM	*pkwd_param = PKWD_RECORD_MONITOR_PARAM_Create( heapID, gsys );

	new_event = GMEVENT_CreateOverlayEventCall( gsys,
      FS_OVERLAY_ID(notwifi_event_pokewood), PKWD_RECORD_MONITOR_EVENT_Call, (void *)pkwd_param );

  PKWD_RECORD_MONITOR_PARAM_Delete( pkwd_param );
	SCRIPT_CallEvent( scw, new_event );

  return VMCMD_RESULT_SUSPEND;
}

//=============================================================================
//  ご褒美アイテム
//=============================================================================
VMCMD_RESULT EvCmdPokewoodGetBonusItem( VMHANDLE * core, void * wk )
{
  SCRCMD_WORK*    work = wk;
  u16 rec_id          = SCRCMD_GetVMWorkValue( core, work );
  u16 fan_no         = SCRCMD_GetVMWorkValue( core, work );
  u16 * ret_item_no   = SCRCMD_GetVMWork( core, work );
  u16 * ret_item_num  = SCRCMD_GetVMWork( core, work );

  // エンディング種類
  u16 rec_result = getRecordInfo( wk, rec_id, POKEWOOD_REC_PARAM_SUCCESS, NULL );
  // 台本種類
  u16 senario_no = getRecordInfo( wk, rec_id, POKEWOOD_REC_PARAM_SENARIO, NULL );
  u16 difficult  = readScenarioData( senario_no, PKWD_SC_PARAM_DIFFICULT );

  u16 bonus_rank;  // ご褒美のランク
  u16 fan_type;    // どうぐを上げるタイプ

  difficult = difficult / 2 - 1;  // 取得したものが 2,4,6,8のため

  GF_ASSERT( rec_result < SCR_PKWD_RESULT_MAX );
  GF_ASSERT( difficult < PKWD_DIFFICULT_MAX );
  GF_ASSERT( fan_no < PKWD_FAN_MAX )

  // コンバータ出力データを参照
  bonus_rank = pkwd_bonus_rank[ difficult ][ rec_result ];
  GF_ASSERT( bonus_rank < PKWD_RANK_MAX );

  fan_type = pkwd_bonus_fan[ fan_no ];

  ICHI_Printf("録画no %d ", rec_id );
  ICHI_Printf("シナリオno %d ", senario_no );
  ICHI_Printf("エンディングno %d ", rec_result );
  ICHI_Printf("台本難易度 %d \n", difficult );
  ICHI_Printf("ファンno %d ", fan_no );
  ICHI_Printf("ご褒美ランク %d ", bonus_rank );
  ICHI_Printf("道具を上げるタイプ %d \n", fan_type );

  // もらえるアイテム取得
  if( fan_type != NOT_GIVE )
  {
    // ご褒美アイテム番号
    *ret_item_no = pkwd_bonus_item[ fan_type ][ bonus_rank ];
    // ご褒美アイテムの個数
    *ret_item_num = pkwd_bonus_item_num[ fan_type ][ bonus_rank ];
  }
  else
  {
    // もらえない
    *ret_item_no = 0;
    *ret_item_num = 0;
  }


  OS_Printf("EvCmdPokewoodGetBonusItem=%d %d\n", *ret_item_no, *ret_item_num);
  return VMCMD_RESULT_CONTINUE;
}

