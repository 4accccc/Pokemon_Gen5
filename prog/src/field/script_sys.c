//============================================================================================
/**
 * @file	script_sys.c
 * @brief	スクリプト制御メイン部分
 * @author	tamada GAMEFREAK inc.
 * @date	2010.01.25
 *
 */
//============================================================================================
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "system/vm.h"
#include "system/vm_cmd.h"

#include "script.h"
#include "script_local.h"

#include "script_def.h"
#include "scrcmd.h"     //ScriptCmdTbl  ScriptCmdMax
#include "scrcmd_work.h"
#include "eventwork_def.h"

#include "field/zonedata.h"

#include "field/eventdata_system.h" //EVENTDATA_GetSpecialScriptData

#include "system/main.h"  //HEAPID_PROC

#include "../../../resource/fldmapdata/script/init_scr_def.h" //SCRID_INIT_SCRIPT
#include "../../../resource/fldmapdata/script/common_scr_def.h" //SCRID_DUMMY

// scr_offset.cdat内で参照
#include "../../../resource/fldmapdata/script/scrid_offset/scr_offset_id.h"
#include "script_message.naix"  //NARC_script_message_
#include "arc/fieldmap/script_seq.naix" // NARC_script_seq_

#include "script_debugger.h"
//============================================================================================
//  define
//============================================================================================

#define SCRIPT_MSG_NON (0xffffffff)

#define SCRIPT_NOT_EXIST_ID (0xffff)

//============================================================================================
//	struct
//============================================================================================
//関数ポインタ型
typedef void (*SCRIPT_EVENTFUNC)(GMEVENT *event, void * work);

//--------------------------------------------------------------
/**
 * スクリプト制御ワーク構造体
 */
//--------------------------------------------------------------
struct _SCRIPTSYS{

  HEAPID heapID;
	u8 vm_machine_count;	//追加した仮想マシンの数
	VMHANDLE *vm[VMHANDLE_MAX];	//仮想マシンへのポインタ
	
	SCRIPT_EVENTFUNC next_func;		//スクリプト終了時に呼び出される関数
  void * next_func_params;      //スクリプト終了時に呼び出される関数に渡す引数

};

//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------
//--------------------------------------------------------------
static SCRIPTSYS * SCRIPTSYS_Create( HEAPID heapID );
static void SCRIPTSYS_Delete( SCRIPTSYS * scrsys );
static VMHANDLE * SCRIPTSYS_GetVM( SCRIPTSYS *scrsys, VMHANDLE_ID vm_id );
static void SCRIPTSYS_AddVM(SCRIPTSYS * scrsys, VMHANDLE * vm, VMHANDLE_ID vm_id);
static void SCRIPTSYS_RemoveVM( SCRIPTSYS *scrsys, VMHANDLE_ID vm_id );
static u32 SCRIPTSYS_GetVMCount( const SCRIPTSYS * scrsys );
static void SCRIPTSYS_EntryNextFunc( SCRIPTSYS *scrsys, GMEVENT * next_event );
static void SCRIPTSYS_CallNextFunc( SCRIPTSYS *scrsys, GMEVENT * event );
static BOOL SCRIPTSYS_HasNextFunc( const SCRIPTSYS *scrsys );

static SCRIPTSYS * SCRIPT_GetScriptSys( SCRIPT_WORK * sc );
//イベント
static GMEVENT * createScriptEvent(
    GAMESYS_WORK * gsys, u16 temp_heapID, u16 scr_id, MMDL * obj, void * ret_script_wk );

//--------------------------------------------------------------
//特殊スクリプト
//--------------------------------------------------------------
static BOOL searchMapInitScript( GAMESYS_WORK * gsys, HEAPID heapID, u8 key);
static u16 searchSpecialScript( const u8 * p, u8 key );
static u16 searchSceneScript( GAMEDATA * gamedata, const u8 * p, u8 key );

//--------------------------------------------------------------
//--------------------------------------------------------------
static VMHANDLE * SCRVM_Create( HEAPID main_heapID, SCRIPT_WORK *sc, u16 zone_id, u16 scr_id );
static void SCRVM_Delete( VMHANDLE* core );


//==============================================================================================
//
//	スクリプト制御
//
//	●スクリプトイベントセット
//		EventSet_Script(...)
//
//	●処理の流れ
//		SCRIPT_SetEventScript(...)						イベント設定
//		  SCRIPTSYS_Create()
//		  SCRIPTWORK_Create(...)
//		  createScriptEvent(...)
//		    SCRIPT_AddVMachine(...)						VMマシーン追加
//				↓
//		scriptEvent(...)				イベント制御関数
//				↓
//		SCRIPTWORK_Delete(...)
//		SCRIPTSYS_Delete(...)
//
//
//	●仮想マシン(VMHANDLE)
//		・仮想マシン追加(SCRIPT_AddVMachine)
//		・仮想マシンの数(vm_machine_count)を追加
//
//	●スクリプト制御メイン(scriptEvent)
//		・仮想マシンの数(vm_machine_count)をチェックしてイベント終了
//
//==============================================================================================
//--------------------------------------------------------------
/**
 * スクリプトイベントセット
 * @param	scr_id		スクリプトID
 * @param	obj			話しかけ対象OBJのポインタ(ない時はNULL)
 * @retval	none
 */
//--------------------------------------------------------------
GMEVENT * SCRIPT_SetEventScript( GAMESYS_WORK *gsys, u16 scr_id, MMDL *obj,
		HEAPID temp_heapID )
{
	GMEVENT *event;

  event = createScriptEvent( gsys, temp_heapID, scr_id, obj, NULL );
	return event;
}

//--------------------------------------------------------------
/**
 * スクリプトイベントコール
 * @param	event		GMEVENT_CONTROL型のポインタ
 * @param	scr_id		スクリプトID
 * @param	obj			話しかけ対象OBJのポインタ(ない時はNULL)
 * @param	ret_script_wk	スクリプト結果を代入するワークのポインタ
 * @retval	none
 */
//--------------------------------------------------------------
SCRIPT_WORK * SCRIPT_CallScript( GMEVENT *event,
	u16 scr_id, MMDL *obj, void *ret_script_wk, HEAPID temp_heapID )
{
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork( event );
	GMEVENT *sc_event;
  EVENT_SCRIPT_WORK * ev_sc;

  sc_event = createScriptEvent( gsys, temp_heapID, scr_id, obj, ret_script_wk );
  ev_sc = GMEVENT_GetEventWork( sc_event );

	GMEVENT_CallEvent( event, sc_event );
  return ev_sc->sc;
}

//--------------------------------------------------------------
/**
 * スクリプトイベント切替
 *
 * @param	event		GMEVENT_CONTROL型のポインタ
 * @param	scr_id		スクリプトID
 * @param	obj			話しかけ対象OBJのポインタ(ない時はNULL)
 *
 * @retval	none
 *
 * 他のイベントからスクリプトへの切替を行う
 */
//--------------------------------------------------------------
SCRIPT_WORK * SCRIPT_ChangeScript( GMEVENT *event,
		u16 scr_id, MMDL *obj, HEAPID temp_heapID )
{
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork( event );
	GMEVENT *sc_event;
  EVENT_SCRIPT_WORK * ev_sc;

  sc_event = createScriptEvent( gsys, temp_heapID, scr_id, obj, NULL );
  ev_sc = GMEVENT_GetEventWork( sc_event );

  GMEVENT_ChangeEvent( event, sc_event );
  return ev_sc->sc;
}

//--------------------------------------------------------------
/**
 * @brief スクリプトが実行されているかどうかのチェック
 */
//--------------------------------------------------------------
BOOL SCRIPT_GetVMExists( SCRIPT_WORK *sc, VMHANDLE_ID vm_id )
{
  SCRIPTSYS * scrsys = SCRIPT_GetScriptSys( sc );
  if ( scrsys == NULL ) return FALSE;
  return ( SCRIPTSYS_GetVM( scrsys, vm_id ) != NULL);
}

//============================================================================================
//
//
//    イベントスクリプト関連
//
//
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static SCRIPTSYS * SCRIPTSYS_Create( HEAPID heapID )
{
  SCRIPTSYS * scrsys = GFL_HEAP_AllocClearMemory( heapID, sizeof(SCRIPTSYS) );
  scrsys->heapID = heapID;
  return scrsys;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void SCRIPTSYS_Delete( SCRIPTSYS * scrsys )
{
  GFL_HEAP_FreeMemory( scrsys );
}
//--------------------------------------------------------------
/**
 * @brief	仮想マシン追加
 * @param sc      SCRIPT_WORKへのポインタ
 * @param vm      追加するVMHANDLEへのポインタ
 * @param vm_id   VMHANDLE_ID (スクリプト用VM指定ID）
 */
//--------------------------------------------------------------
static void SCRIPTSYS_AddVM(SCRIPTSYS * scrsys, VMHANDLE * vm, VMHANDLE_ID vm_id)
{
  GF_ASSERT(vm_id < VMHANDLE_MAX);
  GF_ASSERT(scrsys->vm[vm_id] == NULL);
  scrsys->vm[vm_id] = vm;
  scrsys->vm_machine_count ++;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void SCRIPTSYS_RemoveVM( SCRIPTSYS *scrsys, VMHANDLE_ID vm_id )
{
  GF_ASSERT(vm_id < VMHANDLE_MAX);
  GF_ASSERT(scrsys->vm[vm_id] != NULL);
  if( scrsys->vm_machine_count == 0 ){
    GF_ASSERT_MSG(0, "仮想マシンの数が不正です！" );
    return;
  }
  scrsys->vm[vm_id] = NULL;
  scrsys->vm_machine_count--;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static VMHANDLE * SCRIPTSYS_GetVM( SCRIPTSYS *scrsys, VMHANDLE_ID vm_id )
{
  GF_ASSERT( vm_id < VMHANDLE_MAX );
  return scrsys->vm[ vm_id ];
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static u32 SCRIPTSYS_GetVMCount( const SCRIPTSYS * scrsys )
{
  return scrsys->vm_machine_count;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void changeEventFunc( GMEVENT * event, void * work)
{
  GMEVENT * next_event = work;
  GMEVENT_ChangeEvent( event, next_event );
}
//--------------------------------------------------------------
/**
 * @brief イベント呼び出し予約
 * スクリプト終了後、登録したイベントに遷移する
 */
//--------------------------------------------------------------
static void SCRIPTSYS_EntryNextFunc( SCRIPTSYS *scrsys, GMEVENT * next_event )
{
  GF_ASSERT(scrsys->next_func == NULL);
  GF_ASSERT(scrsys->next_func_params == NULL);
  scrsys->next_func = changeEventFunc;
  scrsys->next_func_params = (void*)next_event;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void SCRIPTSYS_CallNextFunc( SCRIPTSYS *scrsys, GMEVENT * event )
{
  scrsys->next_func( event, scrsys->next_func_params );
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static BOOL SCRIPTSYS_HasNextFunc( const SCRIPTSYS *scrsys )
{
  return (scrsys->next_func != NULL );
}


//============================================================================================
//	イベント
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static u16 getZoneID( GAMESYS_WORK * gsys )
{
  PLAYER_WORK *pw = GAMESYSTEM_GetMyPlayerWork( gsys );
  return PLAYERWORK_getZoneID( pw );
}
//--------------------------------------------------------------
/**
 * スクリプト制御イベント
 * @param	event	GMEVENT
 * @param	seq	event seq
 * @param	wk	event wk
 * @retval	GMEVENT_RESULT
 */
//--------------------------------------------------------------
static GMEVENT_RESULT scriptEvent(
	GMEVENT *event, int *seq, void *wk )
{
	int i;
	EVENT_SCRIPT_WORK *ev_wk = wk;
	SCRIPT_WORK *sc = ev_wk->sc;
  SCRIPTSYS * scrsys = ev_wk->scrsys;
	
  //仮想マシンコントロール
  for( i = 0; i < VMHANDLE_MAX; i++ )
  {
    VMHANDLE *vm = SCRIPTSYS_GetVM( scrsys, i );
    if( vm != NULL )
    {
      if( VM_Control(vm) == FALSE )
      {	//制御メイン
        SCRIPTSYS_RemoveVM( scrsys, i );
        SCRVM_Delete( vm );				//スクリプト削除
      }
    }
  }

  //仮想マシンの数をチェック
  if( SCRIPTSYS_GetVMCount( scrsys ) > 0 )
  {
    return GMEVENT_RES_CONTINUE;
  }

  SCRIPTWORK_Delete( sc );		//スクリプトワーク削除
  
  if( SCRIPTSYS_HasNextFunc( scrsys ) == TRUE )
  {
    //スクリプト終了時に呼び出される関数実行
    SCRIPTSYS_CallNextFunc( scrsys, event );
    SCRIPTSYS_Delete( scrsys );
	  return GMEVENT_RES_CONTINUE;
  }
  else
  { 
    SCRIPTSYS_Delete( scrsys );
    return GMEVENT_RES_FINISH;
  }
}

//--------------------------------------------------------------
/**
 * スクリプトイベントセット
 * @param	
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT * createScriptEvent(
    GAMESYS_WORK * gsys, u16 temp_heapID, u16 scr_id, MMDL * obj, void * ret_script_wk )
{
  GMEVENT * event;
	EVENT_SCRIPT_WORK *ev_sc;
  u16 zone_id;

	event = GMEVENT_Create( gsys, NULL, scriptEvent, sizeof(EVENT_SCRIPT_WORK) );
	ev_sc = GMEVENT_GetEventWork( event );
  ev_sc->scrsys = SCRIPTSYS_Create( HEAPID_PROC );
  ev_sc->sc = SCRIPTWORK_Create( HEAPID_PROC, gsys, event, scr_id, ret_script_wk);
  SCRIPT_SetTargetObj( ev_sc->sc, obj );
	
  zone_id = getZoneID( gsys );
  //仮想マシン追加
  SCRIPT_AddVMachine( ev_sc->sc, zone_id, scr_id, VMHANDLE_MAIN );
		
	return event;
}

//==============================================================================================
//
//
//  スクリプト・スクリプトコマンド内からよばれる制御関数
//
//
//==============================================================================================
//--------------------------------------------------------------
/**
 * @brief イベント呼び出し
 * イベント終了後、再びスクリプト実行に戻る
 */
//--------------------------------------------------------------
void SCRIPT_CallEvent( SCRIPT_WORK *sc, GMEVENT * call_event )
{
  GMEVENT_CallEvent( SCRIPT_GetEvent( sc ), call_event );
}

//--------------------------------------------------------------
/**
 * @brief イベント呼び出し予約
 * スクリプト終了後、登録したイベントに遷移する
 */
//--------------------------------------------------------------
void SCRIPT_EntryNextEvent( SCRIPT_WORK *sc, GMEVENT * next_event )
{
  SCRIPTSYS * scrsys = SCRIPT_GetScriptSys( sc );

  if ( scrsys )
  {
    SCRIPTSYS_EntryNextFunc( scrsys, next_event );
  }
  else
  {
    GF_ASSERT( 0 ); //SCRIPTSYSが存在しない時は動作させない
  }
}

//--------------------------------------------------------------
/**
 * @brief 仮想マシンの追加
 */
//--------------------------------------------------------------
void SCRIPT_AddVMachine( SCRIPT_WORK *sc, u16 zone_id, u16 scr_id, VMHANDLE_ID vm_id )
{
  SCRIPTSYS * scrsys = SCRIPT_GetScriptSys( sc );
  if ( scrsys )
  {
    VMHANDLE * vm;
    vm = SCRVM_Create( scrsys->heapID, sc, zone_id, scr_id );
    SCRIPTSYS_AddVM( scrsys, vm, vm_id );
  }
  else
  {
    GF_ASSERT( 0 ); //SCRIPTSYSが存在しない時は動作させない
  }
}

//--------------------------------------------------------------
/**
 * スクリプトイベントからスクリプトワークを取得
 * @param	GMEVENT *event
 * @retval  SCRIPT_WORK*
 */
//--------------------------------------------------------------
SCRIPT_WORK* SCRIPT_GetScriptWorkFromEvent( GMEVENT *event )
{
	EVENT_SCRIPT_WORK *ev_sc = GMEVENT_GetEventWork( event );
	return ev_sc->sc;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
SCRIPTSYS * SCRIPT_GetScriptSysFromEvent( GMEVENT *event )
{
	EVENT_SCRIPT_WORK *ev_sc = GMEVENT_GetEventWork( event );
	return ev_sc->scrsys;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static SCRIPTSYS * SCRIPT_GetScriptSys( SCRIPT_WORK * sc )
{
  GMEVENT * event;
  EVENT_SCRIPT_WORK * ev_sc;
  event = SCRIPT_GetEvent( sc );
  if( event == NULL )
  {
    GF_ASSERT( 0 );
    return NULL;
  }
  ev_sc = GMEVENT_GetEventWork( event );
  return ev_sc->scrsys;
}



//============================================================================================
//	フラグ関連
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	マップ内限定のセーブフラグをクリアする
 * @return	none
 */
//------------------------------------------------------------------
static void clearLocalEventFlags( EVENTWORK * evwork )
{
	//ローカルフラグのクリア
  EVENTWORK_ClearEventFlags( evwork, FH_AREA_START, FH_AREA_END );
	//ローカルワークのクリア
  EVENTWORK_ClearEventWorks( evwork, LOCALWORKAREA_START, LOCALWORKAREA_END );
}

//============================================================================================
//
//
//	特殊スクリプト関連
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ゲーム開始 スクリプト初期設定の実行
 * @param	gsys      GAMESYS_WORK型のポインタ
 * @param heapID    テンポラリに使用するヒープ指定
 * @return	none
 */
//------------------------------------------------------------------
void SCRIPT_CallGameStartInitScript( GAMESYS_WORK *gsys, HEAPID heapID )
{
  SCRIPT_CallSpecialScript( gsys, heapID, SCRID_INIT_SCRIPT );
}

#ifdef  PM_DEBUG
//------------------------------------------------------------------
/**
 * @brief	ゲーム開始 スクリプト初期設定の実行
 * @param	gsys      GAMESYS_WORK型のポインタ
 * @param heapID    テンポラリに使用するヒープ指定
 * @return	none
 */
//------------------------------------------------------------------
void SCRIPT_CallDebugGameStartInitScript( GAMESYS_WORK *gsys, HEAPID heapID )
{
  SCRIPT_CallSpecialScript( gsys, heapID, SCRID_INIT_DEBUG_SCRIPT );
}
#endif  //PM_DEBUG

//------------------------------------------------------------------
/**
 * @brief	ゲームクリア時処理のスクリプト実行
 * @param	gsys      GAMESYS_WORK型のポインタ
 * @param heapID    テンポラリに使用するヒープ指定
 */
//------------------------------------------------------------------
void SCRIPT_CallGameClearScript( GAMESYS_WORK *gsys, HEAPID heapID )
{
  SCRIPT_CallSpecialScript( gsys, heapID, SCRID_INIT_GAMECLEAR_SCRIPT );
}

//------------------------------------------------------------------
/**
 * @brief	EVENTではなく特殊スクリプト実行
 * @param	gsys      GAMESYS_WORK型のポインタ
 * @param	scr_id		スクリプトID
 * @return	none
 */
//------------------------------------------------------------------
void SCRIPT_CallSpecialScript( GAMESYS_WORK *gsys, HEAPID heapID, u16 script_id )
{
  VMHANDLE *core = NULL;
  SCRIPT_WORK *sc = SCRIPTWORK_Create( HEAPID_PROC, gsys, NULL, script_id, NULL);
  
  u16 zone_id = getZoneID( gsys );
  core = SCRVM_Create( HEAPID_PROC, sc, zone_id, script_id );
  while( VM_Control(core) == TRUE ){};
  SCRVM_Delete( core );

  SCRIPTWORK_Delete( sc );
}

//------------------------------------------------------------------
/**
 * @brief	特殊スクリプト検索して実行
 * @param	gsys      GAMESYS_WORK型のポインタ
 * @param	key			特殊スクリプトID
 * @return	"TRUE=特殊スクリプト実行、FALSE=何もしない"
 */
//------------------------------------------------------------------
static BOOL searchMapInitScript( GAMESYS_WORK * gsys, HEAPID heapID, u8 key)
{
  u16 scr_id;

  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  EVENTDATA_SYSTEM * evdata = GAMEDATA_GetEventData( gamedata );
  const u8 * p = EVENTDATA_GetSpecialScriptData(evdata);

  GF_ASSERT(key != SP_SCRID_SCENE_CHANGE);
  scr_id = searchSpecialScript( p, key );
	if( scr_id == SCRIPT_NOT_EXIST_ID )
  {
		return FALSE;
	}

  //特殊スクリプト実行
  SCRIPT_CallSpecialScript( gsys, heapID, scr_id );
	return TRUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL SCRIPT_CallFieldInitScript( GAMESYS_WORK * gsys, HEAPID heapID )
{
  return searchMapInitScript( gsys, heapID, SP_SCRID_FIELD_INIT );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL SCRIPT_CallFieldRecoverScript( GAMESYS_WORK * gsys, HEAPID heapID )
{
  return searchMapInitScript( gsys, heapID, SP_SCRID_FIELD_RECOVER );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL SCRIPT_CallZoneChangeScript( GAMESYS_WORK * gsys, HEAPID heapID)
{
  EVENTWORK * ev = GAMEDATA_GetEventWork( GAMESYSTEM_GetGameData(gsys) );
  clearLocalEventFlags(ev);
  return searchMapInitScript( gsys, heapID, SP_SCRID_ZONE_CHANGE );
}

//------------------------------------------------------------------
/**
 * @brief   シーン発動イベントの検索
 * @param gsys
 * @param heapID
 * @retval  GMEVENT シーン発動イベントへのポインタ
 * @retval  NULLのとき、イベントなし
 */
//------------------------------------------------------------------
GMEVENT * SCRIPT_SearchSceneScript( GAMESYS_WORK * gsys, HEAPID heapID )
{
  u16 scr_id;
  GMEVENT * event;

  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  EVENTDATA_SYSTEM * evdata = GAMEDATA_GetEventData( gamedata );
  const u8 * p = EVENTDATA_GetSpecialScriptData(evdata);

  scr_id = searchSceneScript( gamedata, p, SP_SCRID_SCENE_CHANGE );
	if( scr_id == SCRIPT_NOT_EXIST_ID )
  {
		return NULL;
  }
  event = SCRIPT_SetEventScript( gsys, scr_id, NULL, heapID );
  return event;
}



//------------------------------------------------------------------
//------------------------------------------------------------------
static inline u16 GETU16( const u8 * p )
{
  return p[0] + ( p[1] << 8 );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static inline u32 GETU32( const u8 * p )
{
  return p[0] + ( p[1] << 8 ) + ( p[2] << 16 ) + ( p[3] << 24 );
}
//------------------------------------------------------------------
/**
 * @brief	ヘッダーデータから指定タイプのものを検索
 * @param	key			特殊スクリプトID
 * @return	"スクリプトID"
 */
//------------------------------------------------------------------
static u16 searchSpecialScript( const u8 * p, u8 key )
{
  while (TRUE)
  {
    if ( GETU16(p) == SP_SCRID_NONE )
    {
      return SCRIPT_NOT_EXIST_ID;
    }
		//指定タイプの特殊スクリプト検索
    if ( GETU16(p) == key )
    {
      p+= sizeof(u16); //特殊スクリプトID分足す
      return GETU16( p );		//スクリプトIDを返す
    }
    //特殊スクリプトID+スクリプトID+ダミー分足す
    p += (sizeof(u16) + sizeof(u16) + sizeof(u16));
  }
  return SCRIPT_NOT_EXIST_ID;                //何も見つからなかった
}

//------------------------------------------------------------------
/**
 * @brief	ヘッダーデータから指定タイプのものを条件検索(シーンイベント)
 * @param	key			特殊スクリプトID
 * @return	"スクリプトID"
 */
//------------------------------------------------------------------
static u16 searchSceneScript( GAMEDATA * gamedata, const u8 * p, u8 key )
{
	u16 work1,work2;
	u32 pos;
	EVENTWORK *ev;

	pos = 0;

	while( TRUE )
  {
		//終了記述チェック(SP_EVENT_DATA_END)
		if( GETU16( p ) == SP_SCRID_NONE ){
			return SCRIPT_NOT_EXIST_ID;
		}

		//指定タイプの特殊スクリプト検索
		if( GETU16( p ) == key ){
			p += sizeof(u16);			//特殊スクリプトID分足す
			pos = GETU32( p );
			p += sizeof(u32);			//アドレス分足す
			break;
		}

		p += ( sizeof(u16) + sizeof(u32) );		//特殊スクリプトID+アドレス分足す
	}

	//見つからなかった時
	if( pos == 0 ){
		return SCRIPT_NOT_EXIST_ID;
	}

	//特殊スクリプトテーブルをセット
	p = (p + pos);

	ev = GAMEDATA_GetEventWork( gamedata );
	while ( TRUE ) {
		//終了記述チェック(SP_SCRIPT_END)
		if( GETU16( p ) == 0 ){
			return SCRIPT_NOT_EXIST_ID;
		}

		//比較するワーク取得
		work1 = GETU16( p );
		if( work1 == 0 ){
			return SCRIPT_NOT_EXIST_ID;
		};
		p += sizeof(u16);									//ワーク分足す
		//OS_Printf( "work1 = %04x\n", work1 );

		//比較する値(ワーク可)取得
		work2 = GETU16( p );
		p += sizeof(u16);									//比較する値分足す(ワーク可)
		//OS_Printf( "work2 = %d\n", work2 );

		//条件チェック
		//if( EVENTWORK_GetEventWorkAdrs(ev,work1) == EVENTWORK_GetEventWorkAdrs(ev,work2) ){
		if( *(EVENTWORK_GetEventWorkAdrs(ev,work1)) == work2 ){
			return GETU16( p );
		}

		p += sizeof(u16);									//スクリプトID分足す
	}
	return SCRIPT_NOT_EXIST_ID;								//何も見つからなかった
}



//============================================================================================
//
//
//      スクリプトコア：仮想マシン生成・削除
//
//
//============================================================================================
static u16 getScriptIndex( u32 zone_id, u16 scr_id, u16 * scr_idx, u32 * msg_idx );
static VM_CODE * loadScriptCodeData( u32 scr_id, HEAPID heapID );

//--------------------------------------------------------------
/**
 * @brief	仮想マシン生成
 * @param	id			スクリプトID
 * @param	start		命令テーブル開始アドレス
 * @param	end			命令テーブル終了アドレス
 * @retval	none
 */
//--------------------------------------------------------------
static VMHANDLE * SCRVM_Create( HEAPID main_heapID, SCRIPT_WORK *sc, u16 zone_id, u16 scr_id )
{
	VMHANDLE *core;
	SCRCMD_WORK *work;
	
  {
    SCRCMD_WORK_HEADER head;
    head.zone_id = zone_id;
    head.script_id = scr_id;
    head.gsys = SCRIPT_GetGameSysWork( sc );
    head.gdata = GAMESYSTEM_GetGameData( head.gsys );
    head.mmdlsys = GAMEDATA_GetMMdlSys( head.gdata );
    head.script_work = sc;

    work = SCRCMD_WORK_Create( &head, main_heapID );
  }
	
  {
    VM_INITIALIZER init;
    init.stack_size = 0x0100;
    init.reg_size = 0x040;
    init.command_table = ScriptCmdTbl;
    init.command_max  = ScriptCmdMax;
	
    core = VM_Create( main_heapID, &init );	//仮想マシン初期化
  }
	
	VM_Init( core, work );
  {
    u16 scr_idx;
    u32 msg_idx;
    u16 local_scr_id;
    //スクリプトデータ、メッセージデータ読み込み
    local_scr_id = getScriptIndex( zone_id, scr_id, &scr_idx, &msg_idx );
    core->pScript = loadScriptCodeData( scr_idx, main_heapID );
    if( msg_idx != SCRIPT_MSG_NON )
    {
      SCRCMD_WORK_CreateMsgData( work, msg_idx );
    }
    VM_Start( core, core->pScript ); //仮想マシンにコード設定
    //スクリプトの先頭部分はテーブルになっているので、
    //オフセットで実際の開始位置を設定する
    core->adrs += (local_scr_id * sizeof(u32));			//ID分進める(adrsがlongなので*4)
    core->adrs += VMGetU32( core );		//ラベルオフセット分進める
  }
	return core;
}
//--------------------------------------------------------------
/**
 * @brief	スクリプト共通削除処理
 *
 * @param	core		VMHANDLE型のポインタ
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SCRVM_Delete( VMHANDLE *core )
{
	SCRCMD_WORK *cmd = VM_GetContext( core );
	SCRCMD_WORK_Delete( cmd );
	GFL_HEAP_FreeMemory( core->pScript );
	VM_Delete( core );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
	u16 scr_id_start;
  u16 scr_id_end;
	u16 scr_arc_id;
	u16 msg_arc_id;
}SCRIPT_ARC_TABLE;

//ScriptArcTableを定義するファイル
#include "../../../resource/fldmapdata/script/scrid_offset/scr_offset.cdat"

//--------------------------------------------------------------
/**
 * @brief	スクリプトIDからスクリプトデータ、メッセージデータを読み込み
 * @param	work
 * @param	core		VMHANDLE型のポインタ
 * @param zone_id
 * @param	scr_id			スクリプトID
 * @retval	"スクリプトIDからオフセットを引いた値"
 */
//--------------------------------------------------------------
static u16 getScriptIndex( u32 zone_id, u16 scr_id, u16 * scr_idx, u32 * msg_idx )
{
  int i;
  u16 local_scr_id = scr_id;

#ifdef DEBUG_SCRIPT
  {
    char buf[ZONEDATA_NAME_LENGTH];
    ZONEDATA_DEBUG_GetZoneName(buf, zone_id);
    OS_Printf( "ZONE:%s SCRID:%d\n", buf, scr_id);
  }
#endif

  for (i = 0; i < NELEMS(ScriptArcTable); i++)
  {
    const SCRIPT_ARC_TABLE * tbl = ScriptArcTable;
    if (scr_id >= tbl[i].scr_id_start)
    {
      GF_ASSERT_MSG(scr_id <= tbl[i].scr_id_end,"id=%d end=%d",scr_id,tbl[i].scr_id_end);
      *scr_idx = tbl[i].scr_arc_id;
      *msg_idx = tbl[i].msg_arc_id;
      local_scr_id -= tbl[i].scr_id_start;
      OS_Printf( "共通スクリプト起動 scr_arc_idx = %d, msg_idx = %d, scr_id = %d\n",
        tbl[i].scr_arc_id, tbl[i].msg_arc_id, local_scr_id );
      return local_scr_id;
    }
  }

  //ローカルスクリプトのscr_IDが渡された時
  if( scr_id >= ID_START_SCR_OFFSET )
  {
    u16 idx_script = ZONEDATA_GetScriptArcID( zone_id );
    u16 idx_msg = ZONEDATA_GetMessageArcID( zone_id );
    *scr_idx = idx_script;
    *msg_idx = idx_msg;
    local_scr_id -= ID_START_SCR_OFFSET;
    OS_Printf( "ゾーンスクリプト起動 scr_idx = %d, msg_idx = %d\n",
        idx_script, idx_msg );
    return local_scr_id;
  }

  //SCRID_NULL(0)が渡された時
  *scr_idx = NARC_script_seq_common_scr_bin;
  *msg_idx = NARC_script_message_common_scr_dat;
  local_scr_id = 0;
  return local_scr_id;
}


//----------------------------------------------------------------
/**
 * スクリプトデータセット
 * @param	core		VMHANDLE型のポインタ
 * @param	scr_id		スクリプトデータのID
 */
//----------------------------------------------------------------
static VM_CODE * loadScriptCodeData( u32 scr_id, HEAPID heapID )
{
	//共通スクリプトデータロード
  VM_CODE * script = NULL;
#ifdef  PM_DEBUG
  script = SCRDEBUGGER_ReadScriptFile( heapID, scr_id );
#endif
  if ( script == NULL )
  {
    script = GFL_ARC_UTIL_Load( ARCID_SCRSEQ, scr_id, 0, heapID );
  }
  return (VM_CODE *) script;
}

//------------------------------------------------------------------
/**
 * @brief スクリプトIDのチェック
 * @param script_id
 * @return  BOOL  FALSEのとき、実行可能なスクリプト
 *
 * @todo
 * できたら厳密な範囲チェックを行って暴走しないように制限する
 */
//------------------------------------------------------------------
BOOL SCRIPT_IsValidScriptID( u16 script_id )
{
  if ( script_id == SCRID_DUMMY ) return FALSE;
  if ( script_id >= ID_DEBUG_SCR_OFFSET_END )
  {
    GF_ASSERT(0);
    return FALSE;
  }

  /* ここに範囲チェックを入れる？ */

  return TRUE;
}

