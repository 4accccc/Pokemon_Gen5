//============================================================================================
/**
 * @file	script.c
 * @brief	スクリプト制御メイン部分
 * @author	Satoshi Nohara
 * @date	05.08.04
 *
 * 01.11.07	Sousuke Tamada
 * 03.04.15	Satoshi Nohara
 * 05.04.25 Hiroyuki Nakamura
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

#include "print/wordset.h"    //WORDSET

#include "trainer_eye_data.h"   //EV_TRAINER_EYE_HITDATA
#include "field/eventdata_system.h" //EVENTDATA_GetSpecialScriptData

#include "fieldmap.h"   //FIELDMAP_GetFldMsgBG
#include "battle/battle.h"

#include "musical/musical_event.h"  //MUSICAL_EVENT_WORK

#include "system/main.h"  //HEAPID_PROC

#include "../../../resource/fldmapdata/script/init_scr_def.h" //SCRID_INIT_SCRIPT
#include "../../../resource/fldmapdata/script/common_scr_def.h" //SCRID_DUMMY

// scr_offset.cdat内で参照
#include "../../../resource/fldmapdata/script/scrid_offset/scr_offset_id.h"
#include "script_message.naix"
#include "arc/fieldmap/script_seq.naix"

//============================================================================================
//  define
//============================================================================================
#define TRAINER_EYE_HITMAX (2) ///<トレーナー視線データ最大数

enum
{
	WORDSET_SCRIPT_SETNUM = 32,		//デフォルトバッファ数
	WORDSET_SCRIPT_BUFLEN = 64,		//デフォルトバッファ長（文字数）
};

//#define SCR_MSG_BUF_SIZE	(1024)				//メッセージバッファサイズ
#define SCR_MSG_BUF_SIZE	(512)				//メッセージバッファサイズ

#define SCRIPT_MAGIC_NO		(0x3643f)

#define SCRIPT_MSG_NON (0xffffffff)

#define SCRIPT_NOT_EXIST_ID (0xffff)

enum {
  TEMP_WORK_START = SCWK_AREA_START,
  TEMP_WORK_END = USERWK_AREA_END,
  TEMP_WORK_SIZE = TEMP_WORK_END - TEMP_WORK_START,
};

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
typedef struct {

	u8 vm_machine_count;	//追加した仮想マシンの数
	VMHANDLE *vm[VMHANDLE_MAX];	//仮想マシンへのポインタ
	
	SCRIPT_EVENTFUNC next_func;		//スクリプト終了時に呼び出される関数
  void * next_func_params;      //スクリプト終了時に呼び出される関数に渡す引数

}SCRIPTSYS;

//--------------------------------------------------------------
//
//--------------------------------------------------------------
struct _TAG_SCRIPT_WORK
{
  //☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
  //script.c でPrivateなメンバ
  //☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
	u32 magic_no;			//イベントのワークがスクリプト制御ワークかを判別
	
  SCRIPTSYS * scr_sys;

  //☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
  //SCRIPT_WORK生成時に引数で初期化され、ReadOnlyで外部公開されるメンバ
  //☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
	u16 start_scr_id;			//メインのスクリプトID
  u16 start_zone_id;
	MMDL *target_obj;

	HEAPID main_heapID;
  HEAPID temp_heapID;

	GAMESYS_WORK *gsys;
  GMEVENT *gmevent; //スクリプトを実行しているGMEVENT*
	SCRIPT_FLDPARAM fld_param;

	u16 *ret_script_wk;			//スクリプト結果を代入するワークのポインタ

  //☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
  //script.c内で使用しない、外部公開用メンバ
  //  public R/W
  //☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆☆
  void *msgWin;           //メニュー系流用メッセージウィンドウ

	WORDSET* wordset;				//単語セット
	STRBUF* msg_buf;				//メッセージバッファポインタ
	STRBUF* tmp_buf;				//テンポラリバッファポインタ


	u8 MsgIndex;			//メッセージインデックス
	u8 anm_count;			//アニメーションしている数
	
	u8 win_open_flag;		//会話ウィンドウ表示フラグ(0=非表示、1=表示)

  u8 sound_se_flag;   ///< サウンドSE再生チェックフラグ

	BOOL win_flag;			///<戦闘結果保持用ワーク
	
  void *mw;				//ビットマップメニューワーク

	void * subproc_work; //サブプロセスとのやりとりに使用するワークへの*
	void * pWork;					//ワークへの汎用ポインタ

	MMDL *dummy_obj;

	//トレーナー視線情報
	EV_TRAINER_EYE_HITDATA eye_hitdata[TRAINER_EYE_HITMAX];
	
	u16 scrTempWork[TEMP_WORK_SIZE];		//ワーク(ANSWORK,TMPWORKなどの代わり)


  MUSICAL_EVENT_WORK *musicalEventWork;

  
  /*
   * 未使用・もしくはいらないワーク（前作から持ってきた
#ifndef SCRIPT_PL_NULL
	u8 common_scr_flag;		//ローカル、共通 切り替えフラグ(0=local、1=共通)
	int player_dir;					//イベント起動時の主人公の向き
	//イベントウィンドウ
	EV_WIN_WORK* ev_win;			//イベントウィンドウワークへのポインタ
	//会話ウィンドウ
	GF_BGL_BMPWIN MsgWinDat;		//ビットマップウィンドウデータ
	
	TCB_PTR player_tcb;				//自機形態レポートTCB
	
	GF_BGL_BMPWIN CoinWinDat;		//ビットマップウィンドウデータ
	GF_BGL_BMPWIN GoldWinDat;		//ビットマップウィンドウデータ
	
	REPORT_INFO * riw;				///<レポート情報用ウィンドウ制御ワーク
	EOA_PTR eoa;		//主にフィールドエフェクトのポインタとして使う
	void * waiticon;				///<待機アイコンのポインタ
#endif
   */
};

//--------------------------------------------------------------
//	スクリプトイベント用ワーク
//--------------------------------------------------------------
typedef struct
{
	SCRIPT_WORK *sc; //スクリプト用ワーク
}EVENT_SCRIPT_WORK;

//============================================================================================
//	プロトタイプ宣言
//============================================================================================
static SCRIPT_WORK * SCRIPTWORK_Create( HEAPID main_heapID, HEAPID temp_heapID,
    GAMESYS_WORK * gsys, u16 scr_id, MMDL *obj, void* ret_wk);
static void SCRIPTWORK_Delete( SCRIPT_WORK * sc );
static void SCRIPTSYS_AddVM(SCRIPTSYS * scr_sys, VMHANDLE * vm, VMHANDLE_ID vm_id);
static void SCRIPTSYS_RemoveVM( SCRIPTSYS *scr_sys, VMHANDLE_ID vm_id );

static VMHANDLE * SCRVM_Create( SCRIPT_WORK *sc, u16 zone_id, u16 scr_id );
static void SCRVM_Delete( VMHANDLE* core );
static u16 loadScriptData( SCRCMD_WORK *work, VMHANDLE* core, u32 zone_id, u16 id, HEAPID heapID );
static void loadScriptDataDirect(
	SCRCMD_WORK *work, VMHANDLE* core, u32 scr_id, u32 msg_id, HEAPID heapID );


//イベント
static GMEVENT * FldScript_CreateControlEvent( SCRIPT_WORK *sc );

//イベントワーク、スクリプトワークアクセス関数
static void * SCRIPT_GetSubMemberWork( SCRIPT_WORK * sc, u32 id );
static u16 * getTempWork( SCRIPT_WORK * sc, u16 work_no );

static void initFldParam(SCRIPT_FLDPARAM * fparam, GAMESYS_WORK * gsys);


//特殊スクリプト
static BOOL searchMapInitScript( GAMESYS_WORK * gsys, HEAPID heapID, u8 key);
static u16 searchSpecialScript( const u8 * p, u8 key );
static u16 searchSceneScript( GAMEDATA * gamedata, const u8 * p, u8 key );

//==============================================================================================
//
//	スクリプト制御
//
//	●スクリプトイベントセット
//		EventSet_Script(...)
//
//	●処理の流れ
//		SCRIPTWORK_Create(...)
//		SCRIPT_SetEventScript(...)						イベント設定
//		GMEVENT_ControlScript(...)				イベント制御関数
//				↓
//		VMMachineAdd(...)						VMマシーン追加
//
//	●仮想マシン(VMHANDLE)
//		・仮想マシン追加(VMMachineAdd)
//		・仮想マシンの数(vm_machine_count)を追加
//
//	●スクリプト制御メイン(GMEVENT_ControlScript)
//		・仮想マシンの数(vm_machine_count)をチェックしてイベント終了
//
//==============================================================================================
//--------------------------------------------------------------
/**
 * スクリプトイベントセット
 * @param	fsys		FLDCOMMON_WORK型のポインタ
 * @param	scr_id		スクリプトID
 * @param	obj			話しかけ対象OBJのポインタ(ない時はNULL)
 * @retval	none
 */
//--------------------------------------------------------------
GMEVENT * SCRIPT_SetEventScript( GAMESYS_WORK *gsys, u16 scr_id, MMDL *obj,
		HEAPID temp_heapID )
{
	GMEVENT *event;
	SCRIPT_WORK *sc;

  sc = SCRIPTWORK_Create( HEAPID_PROC, temp_heapID, gsys, scr_id, obj, NULL );
	event = FldScript_CreateControlEvent( sc );
	return( event );
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
	GMEVENT *sc_event;
	SCRIPT_WORK *sc;
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork( event );

  sc = SCRIPTWORK_Create( HEAPID_PROC, temp_heapID, gsys, scr_id, obj, ret_script_wk);
	sc_event = FldScript_CreateControlEvent( sc );
	GMEVENT_CallEvent( event, sc_event );

  return sc;
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
	GMEVENT *sc_event;
  SCRIPT_WORK *sc;
  GAMESYS_WORK * gsys = GMEVENT_GetGameSysWork( event );

  sc = SCRIPTWORK_Create( HEAPID_PROC, temp_heapID, gsys, scr_id, obj, NULL);
  sc_event = FldScript_CreateControlEvent( sc );
  GMEVENT_ChangeEvent( event, sc_event );
  return sc;
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
  GMEVENT_CallEvent( sc->gmevent, call_event );
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
void SCRIPT_EntryNextEvent( SCRIPT_WORK *sc, GMEVENT * next_event )
{
  SCRIPTSYS * scr_sys = sc->scr_sys;

  GF_ASSERT(scr_sys->next_func == NULL);
  GF_ASSERT(scr_sys->next_func_params == NULL);
  scr_sys->next_func = changeEventFunc;
  scr_sys->next_func_params = (void*)next_event;
}

//--------------------------------------------------------------
/**
 * @brief 仮想マシンの追加
 */
//--------------------------------------------------------------
void SCRIPT_AddVMachine( SCRIPT_WORK *sc, u16 zone_id, u16 scr_id, VMHANDLE_ID vm_id )
{
  VMHANDLE * vm = SCRVM_Create( sc, zone_id, scr_id );
  SCRIPTSYS_AddVM( sc->scr_sys, vm, vm_id );
}

//--------------------------------------------------------------
/**
 * @brief スクリプトが実行されているかどうかのチェック
 */
//--------------------------------------------------------------
BOOL SCRIPT_GetVMExists( SCRIPT_WORK *sc, VMHANDLE_ID vm_id )
{
  GF_ASSERT(vm_id < VMHANDLE_MAX);
  return (sc->scr_sys->vm[vm_id] != NULL);
}


//============================================================================================
//
//  スクリプトワーク
//
//============================================================================================
//--------------------------------------------------------------
/**
 * @param	scr_id		スクリプトID
 * @param	obj			話しかけ対象OBJのポインタ
 * @param	ret_wk		スクリプト結果を代入するワークのポインタ
 * @return	SCRIPT_WORK			SCRIPT型のポインタ
 */
//--------------------------------------------------------------
static SCRIPT_WORK * SCRIPTWORK_Create( HEAPID main_heapID, HEAPID temp_heapID,
    GAMESYS_WORK * gsys, u16 scr_id, MMDL *obj, void* ret_wk)
{
  SCRIPT_WORK * sc;

	sc = GFL_HEAP_AllocClearMemory( main_heapID, sizeof(SCRIPT_WORK) );
	sc->magic_no = SCRIPT_MAGIC_NO;
	sc->main_heapID = main_heapID;
  sc->temp_heapID = temp_heapID;

	sc->gsys = gsys;
	sc->start_scr_id  = scr_id;	//メインのスクリプトID
	sc->target_obj = obj;		//話しかけ対象OBJのポインタセット
	sc->ret_script_wk = ret_wk;	//スクリプト結果を代入するワーク
  
  { //wb
    PLAYER_WORK *pw = GAMESYSTEM_GetMyPlayerWork( gsys );
  	sc->start_zone_id = PLAYERWORK_getZoneID( pw );
  }
  
	if( obj != NULL ){
    u16 *objid;
    objid = getTempWork( sc, SCWK_TARGET_OBJID );
		*objid = MMDL_GetOBJID( obj ); //話しかけ対象OBJIDのセット
	}
  sc->scr_sys = GFL_HEAP_AllocClearMemory( main_heapID, sizeof(SCRIPTSYS) );

  //メッセージ関連
  sc->wordset = WORDSET_CreateEx(
    WORDSET_SCRIPT_SETNUM, WORDSET_SCRIPT_BUFLEN, main_heapID );
  sc->msg_buf = GFL_STR_CreateBuffer( SCR_MSG_BUF_SIZE, main_heapID );
  sc->tmp_buf = GFL_STR_CreateBuffer( SCR_MSG_BUF_SIZE, main_heapID );

  //フィールドパラメータ生成
	initFldParam( &sc->fld_param, gsys );
  //ミュージカルの制御イベントで渡される
  sc->musicalEventWork = NULL;

  //チェックビット処理化
  SCREND_CHK_ClearBits();

  return sc;
}

//--------------------------------------------------------------
/**
 */
//--------------------------------------------------------------
static void SCRIPTWORK_Delete( SCRIPT_WORK * sc )
{
  sc->magic_no = 0;
  WORDSET_Delete( sc->wordset );
  GFL_STR_DeleteBuffer( sc->msg_buf );
  GFL_STR_DeleteBuffer( sc->tmp_buf );

  GFL_HEAP_FreeMemory( sc->scr_sys );

  GFL_HEAP_FreeMemory( sc );
}

//============================================================================================
//
//
//  スクリプト仮想マシン関連
//
//
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief	仮想マシン追加
 * @param sc      SCRIPT_WORKへのポインタ
 * @param vm      追加するVMHANDLEへのポインタ
 * @param vm_id   VMHANDLE_ID (スクリプト用VM指定ID）
 */
//--------------------------------------------------------------
static void SCRIPTSYS_AddVM(SCRIPTSYS * scr_sys, VMHANDLE * vm, VMHANDLE_ID vm_id)
{
  GF_ASSERT(vm_id < VMHANDLE_MAX);
  GF_ASSERT(scr_sys->vm[vm_id] == NULL);
  scr_sys->vm[vm_id] = vm;
  scr_sys->vm_machine_count ++;
}
//--------------------------------------------------------------
//--------------------------------------------------------------
static void SCRIPTSYS_RemoveVM( SCRIPTSYS *scr_sys, VMHANDLE_ID vm_id )
{
  GF_ASSERT(vm_id < VMHANDLE_MAX);
  GF_ASSERT(scr_sys->vm[vm_id] != NULL);
  if( scr_sys->vm_machine_count == 0 ){
    GF_ASSERT_MSG(0, "仮想マシンの数が不正です！" );
    return;
  }
  scr_sys->vm[vm_id] = NULL;
  scr_sys->vm_machine_count--;
}

//--------------------------------------------------------------
/**
 * @brief	仮想マシン生成
 * @param	fsys		FLDCOMMON_WORK型のポインタ
 * @param	id			スクリプトID
 * @param	start		命令テーブル開始アドレス
 * @param	end			命令テーブル終了アドレス
 * @retval	none
 */
//--------------------------------------------------------------
static VMHANDLE * SCRVM_Create( SCRIPT_WORK *sc, u16 zone_id, u16 scr_id )
{
	SCRCMD_WORK_HEADER head;
	VMHANDLE *core;
	VM_INITIALIZER init;
	SCRCMD_WORK *work;
	
  head.zone_id = zone_id;
  head.script_id = scr_id;
	head.gsys = sc->gsys;
	head.gdata = GAMESYSTEM_GetGameData( sc->gsys );
	head.mmdlsys = GAMEDATA_GetMMdlSys( head.gdata );
	head.script = sc;
	head.fldMsgBG = sc->fld_param.msgBG;

	work = SCRCMD_WORK_Create( &head, sc->main_heapID, sc->temp_heapID );
	
	init.stack_size = 0x0100;
	init.reg_size = 0x040;
	init.command_table = ScriptCmdTbl;
	init.command_max  = ScriptCmdMax;
	
	core = VM_Create( sc->main_heapID, &init );	//仮想マシン初期化
	
	VM_Init( core, work );
  {
    u16 local_scr_id;
    //スクリプトデータ、メッセージデータ読み込み
    local_scr_id = loadScriptData( work, core, zone_id, scr_id, sc->main_heapID );
    VM_Start( core, core->pScript ); //仮想マシンにコード設定
    //スクリプトの先頭部分はテーブルになっているので、
    //オフセットで実際の開始位置を設定する
    core->adrs += (local_scr_id * sizeof(u32));			//ID分進める(adrsがlongなので*4)
    core->adrs += VMGetU32( core );		//ラベルオフセット分進める
  }
#ifdef DEBUG_SCRIPT
{
  char buf[ZONEDATA_NAME_LENGTH];
  ZONEDATA_DEBUG_GetZoneName(buf, zone_id);
  OS_Printf( "ZONE:%s SCRID:%d\n", buf, scr_id);
}
#endif
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
static u16 loadScriptData( SCRCMD_WORK *work, VMHANDLE* core, u32 zone_id, u16 scr_id, HEAPID heapID )
{
  int i;
  u16 local_scr_id = scr_id;
  const SCRIPT_ARC_TABLE * tbl = ScriptArcTable;

  for (i = 0; i < NELEMS(ScriptArcTable); i++) {
    if (scr_id >= tbl[i].scr_id_start) {
      GF_ASSERT_MSG(scr_id <= tbl[i].scr_id_end,"id=%d end=%d",scr_id,tbl[i].scr_id_end);
      loadScriptDataDirect(work, core, tbl[i].scr_arc_id, tbl[i].msg_arc_id, heapID);
      local_scr_id -= tbl[i].scr_id_start;
      OS_Printf( "共通スクリプト起動 scr_arc_idx = %d, msg_idx = %d, scr_id = %d\n",
        tbl[i].scr_arc_id, tbl[i].msg_arc_id, local_scr_id );
      return local_scr_id;
    }
  }
  //ローカルスクリプトのscr_IDが渡された時
  if( scr_id >= ID_START_SCR_OFFSET ){
    u16 idx_script = ZONEDATA_GetScriptArcID( zone_id );
    u16 idx_msg = ZONEDATA_GetMessageArcID( zone_id );
    loadScriptDataDirect( work, core, idx_script, idx_msg, heapID );
    local_scr_id -= ID_START_SCR_OFFSET;
    OS_Printf( "ゾーンスクリプト起動 scr_idx = %d, msg_idx = %d\n",
        idx_script, idx_msg );
    return local_scr_id;

  //SCRID_NULL(0)が渡された時
  }
  loadScriptDataDirect( work, core,
      NARC_script_seq_common_scr_bin,
      NARC_script_message_common_scr_dat, heapID );
  local_scr_id = 0;
  return local_scr_id;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void initFldParam(SCRIPT_FLDPARAM * fparam, GAMESYS_WORK * gsys)
{
  FIELDMAP_WORK * fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  MI_CpuClear32( fparam, sizeof(SCRIPT_FLDPARAM) );
  fparam->fieldMap = fieldmap;
  if (fieldmap != NULL)
  {
    fparam->msgBG = FIELDMAP_GetFldMsgBG(fieldmap);
  }
}

//----------------------------------------------------------------
/**
 * スクリプトデータセット
 * @param	fsys		FLDCOMMON_WORK型のポインタ
 * @param	core		VMHANDLE型のポインタ
 * @param	scr_id		スクリプトデータのID
 * @param	msg_id		メッセージデータのID
 */
//----------------------------------------------------------------
static void loadScriptDataDirect( SCRCMD_WORK *work,
	VMHANDLE *core, u32 scr_id, u32 msg_id, HEAPID heapID )
{
	//共通スクリプトデータロード
	VM_CODE *script = GFL_ARC_UTIL_Load( ARCID_SCRSEQ, scr_id, 0, heapID );
	core->pScript = (VM_CODE*)script;
	
	if( msg_id != SCRIPT_MSG_NON ){
		SCRCMD_WORK_CreateMsgData( work, msg_id );
	}
}


//============================================================================================
//	スクリプト制御ワークのメンバーアクセス
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
GMEVENT * SCRIPT_GetEvent( SCRIPT_WORK * sc )
{
  GF_ASSERT(sc->magic_no == SCRIPT_MAGIC_NO);
  return sc->gmevent;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
GAMESYS_WORK * SCRIPT_GetGameSysWork( SCRIPT_WORK * sc )
{
  GF_ASSERT(sc->magic_no == SCRIPT_MAGIC_NO);
  return sc->gsys;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
SCRIPT_FLDPARAM * SCRIPT_GetFieldParam( SCRIPT_WORK * sc )
{
  FIELDMAP_WORK * fieldmap;
  GF_ASSERT( sc->magic_no == SCRIPT_MAGIC_NO );
  //スクリプトコマンド実行中にフィールドへの出入りがあると、
  //条件が変わってポインタが正しい値を指していない可能性がある。
  //なので、アクセスタイミングでの初期化処理を行う。
  initFldParam( &sc->fld_param, sc->gsys );
  GF_ASSERT( sc->fld_param.fieldMap != NULL );
  return &sc->fld_param;
}

//--------------------------------------------------------------
/**
 * スクリプト制御ワークのメッセージウィンドウポインタ取得
 * @param	sc		    SCRIPT型のポインタ
 * @param	msgWin		取得するメンバID(script.h参照)
 * @return	"アドレス"
 */
//--------------------------------------------------------------
void * SCRIPT_GetMsgWinPointer( SCRIPT_WORK *sc )
{
  return sc->msgWin;
}

//--------------------------------------------------------------
/**
 * スクリプト制御ワークのメッセージウィンドウポインタセット
 * @param	sc		    SCRIPT型のポインタ
 * @param	msgWin		取得するメンバID(script.h参照)
 * @return	"アドレス"
 */
//--------------------------------------------------------------
void SCRIPT_SetMsgWinPointer( SCRIPT_WORK *sc, void* msgWin )
{
  sc->msgWin = msgWin;
}

//--------------------------------------------------------------
/**
 * @brief   開始時のスクリプトID取得
 * @param	sc		    SCRIPT_WORKのポインタ
 * @return  u16   スクリプトID
 */
//--------------------------------------------------------------
u16 SCRIPT_GetStartScriptID( const SCRIPT_WORK * sc )
{
  return sc->start_scr_id;
}

//--------------------------------------------------------------
/**
 * スクリプト制御ワークのメンバーアドレス取得
 * @param	sc		SCRIPT型のポインタ
 * @param	id		取得するメンバID(script.h参照)
 * @return	"アドレス"
 */
//--------------------------------------------------------------
static void * SCRIPT_GetSubMemberWork( SCRIPT_WORK *sc, u32 id )
{
	switch( id ){
	//会話ウィンドウメッセージインデックスのポインタ
	case ID_EVSCR_MSGINDEX:
		return &sc->MsgIndex;
	//アニメーションの数のポインタ
	case ID_EVSCR_ANMCOUNT:
		return &sc->anm_count;
	//会話ウィンドウを開いたかフラグのポインタ
	case ID_EVSCR_WIN_OPEN_FLAG:
		return &sc->win_open_flag;
	//話しかけ対象のOBJのポインタ
	case ID_EVSCR_TARGET_OBJ:
		return &sc->target_obj;
	//透明ダミーOBJのポインタ
	case ID_EVSCR_DUMMY_OBJ:
		return &sc->dummy_obj;
	//結果を代入するワークのポインタ
	case ID_EVSCR_RETURN_SCRIPT_WK:
		return &sc->ret_script_wk;
	//仮想マシン(メイン)のポインタ
	//サブプロセスとのやりとりワークへのポインタ
	case ID_EVSCR_SUBPROC_WORK:
		return &sc->subproc_work;
	//ワークへの汎用ポインタ
	case ID_EVSCR_PWORK:
		return &sc->pWork;
	//戦闘結果フラグ
	case ID_EVSCR_WIN_FLAG:
		return &sc->win_flag;


	//HEAPID wb
	case ID_EVSCR_WK_HEAPID:
		return &sc->main_heapID;
  //TEMP_HEAPID wb
  case ID_EVSCR_WK_TEMP_HEAPID:
    return &sc->temp_heapID;
	//ビットマップメニューワークのポインタ
	case ID_EVSCR_MENUWORK:
		return &sc->mw;
	//単語セット
	case ID_EVSCR_WORDSET:
		return &sc->wordset;
	//メッセージバッファのポインタ
	case ID_EVSCR_MSGBUF:
		return &sc->msg_buf;
	//テンポラリバッファのポインタ
	case ID_EVSCR_TMPBUF:
		return &sc->tmp_buf;


  case ID_EVSCR_TRAINER0:
		return &sc->eye_hitdata[0];
  case ID_EVSCR_TRAINER1:
		return &sc->eye_hitdata[1];
  
  case ID_EVSCR_MUSICAL_EVENT_WORK:
  	GF_ASSERT_MSG( sc->musicalEventWork != NULL , "ミュージカルワークがNULL！" );
    return sc->musicalEventWork;

  case ID_EVSCR_SOUND_SE_FLAG:
    return &sc->sound_se_flag;
      

	};
	
	//エラー
	GF_ASSERT( (0) && "メンバーIDが不正です！" );
	return NULL;
}

//--------------------------------------------------------------
/**
 * スクリプト制御ワークのメンバーアドレス取得
 * @param	fsys	FLDCOMMON_WORK型のポインタ
 * @param	id		取得するメンバID(script.h参照)
 * @return	"アドレス"
 */
//--------------------------------------------------------------
void * SCRIPT_GetMemberWork( SCRIPT_WORK *sc, u32 id )
{
	if( sc->magic_no != SCRIPT_MAGIC_NO ){
		GF_ASSERT_MSG(0, "起動(確保)していないスクリプトのワークにアクセスしています！" );
	}
	return SCRIPT_GetSubMemberWork( sc, id );
}


//============================================================================================
//	イベント
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static void FldScriptEvent_InitWork( EVENT_SCRIPT_WORK *ev_wk, SCRIPT_WORK * sc )
{
	MI_CpuClear8( ev_wk, sizeof(EVENT_SCRIPT_WORK) );
	ev_wk->sc = sc;
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
static GMEVENT_RESULT FldScriptEvent_ControlScript(
	GMEVENT *event, int *seq, void *wk )
{
	int i;
	EVENT_SCRIPT_WORK *ev_wk = wk;
	SCRIPT_WORK *sc = ev_wk->sc;
	
	switch( *seq ){
	case 0:
    //ワークセット
    sc->gmevent = event; //イベントセット時に初期化してるが念のため
		
    //仮想マシン追加
    SCRIPT_AddVMachine( sc, sc->start_zone_id, sc->start_scr_id, VMHANDLE_MAIN );
		
		(*seq)++;
		/* FALL THROUGH */

	case 1:
		//仮想マシンコントロール
		for( i = 0; i < VMHANDLE_MAX; i++ ){
	    VMHANDLE *vm = sc->scr_sys->vm[i];
			if( vm != NULL ){
				if( VM_Control(vm) == FALSE ){	//制御メイン
          SCRIPTSYS_RemoveVM( sc->scr_sys, i );
					SCRVM_Delete( vm );				//スクリプト削除
				}
			}
		}

		//仮想マシンの数をチェック
		if( sc->scr_sys->vm_machine_count <= 0 ){
			SCRIPT_EVENTFUNC func = sc->scr_sys->next_func;		//退避
      void * func_params = sc->scr_sys->next_func_params;//退避
			
			//デバック処理
			//debug_script_flag = 0;
			
      SCRIPTWORK_Delete( sc );		//スクリプトワーク
			
			//スクリプト終了時に呼び出される関数実行
			if( func != NULL ){
				func( event, func_params );
			}else{ //FieldEventを終了しないようにFALSEを返す！
				return( GMEVENT_RES_FINISH );		//FieldEvent_Set終了！
			}
		}

		break;
	}

	return( GMEVENT_RES_CONTINUE );
}

//--------------------------------------------------------------
/**
 * スクリプトイベントセット
 * @param	sc	SCRIPT_WORK*
 * @param	
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT * FldScript_CreateControlEvent( SCRIPT_WORK *sc )
{
	GMEVENT *event;
	EVENT_SCRIPT_WORK *ev_sc;
	
	event = GMEVENT_Create( sc->gsys, NULL,
		FldScriptEvent_ControlScript, sizeof(EVENT_SCRIPT_WORK) );
	ev_sc = GMEVENT_GetEventWork( event );
  FldScriptEvent_InitWork( ev_sc, sc );
	
  ev_sc->sc->gmevent = event;
	return event;
}

//--------------------------------------------------------------
/**
 * スクリプトイベントからスクリプトワークを取得
 * @param	GMEVENT *event
 * @retval  SCRIPT_WORK*
 */
//--------------------------------------------------------------
SCRIPT_WORK* SCRIPT_GetEventWorkToScriptWork( GMEVENT *event )
{
	EVENT_SCRIPT_WORK *ev_sc = GMEVENT_GetEventWork( event );
	return ev_sc->sc;
}

//============================================================================================
//	イベントワーク
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static u16 * getTempWork( SCRIPT_WORK * sc, u16 work_no )
{
  GF_ASSERT( work_no >= TEMP_WORK_START );
  GF_ASSERT( work_no < TEMP_WORK_END );
  return &(sc->scrTempWork[ work_no - TEMP_WORK_START ]);
}
//------------------------------------------------------------------
/**
 * @brief	イベントワークアドレスを取得
 * @param	ev			イベントワークへのポインタ
 * @param	work_no		ワークナンバー
 *
 * @return	"ワークのアドレス"
 *
 * @li	work_no < 0x8000	通常のセーブワーク
 * @li	work_no >= 0x8000	スクリプト制御ワークの中に確保しているワーク
 */
//------------------------------------------------------------------
u16 * SCRIPT_GetEventWork( SCRIPT_WORK *sc, GAMEDATA *gdata, u16 work_no )
{
	EVENTWORK *ev;
	ev = GAMEDATA_GetEventWork( gdata );
	
	if( work_no < SVWK_START ){
		return NULL;
	}
	
	if( work_no < SCWK_START ){
		return EVENTWORK_GetEventWorkAdrs(ev,work_no);
	}
	
	//スクリプト制御ワークの中で、ANSWORKなどのワークを確保しています
  return getTempWork( sc, work_no );
}

//------------------------------------------------------------------
/**
 * @brief	イベントワークの値を取得
 *
 * @param	work_no		ワークナンバー
 *
 * @return	"ワークの値"
 */
//------------------------------------------------------------------
u16 SCRIPT_GetEventWorkValue( SCRIPT_WORK *sc, GAMEDATA *gdata, u16 work_no )
{
	u16 *res = SCRIPT_GetEventWork( sc, gdata, work_no );
	if( res == NULL ){ return work_no; }
	return *res;
}

//------------------------------------------------------------------
/**
 * @brief	イベントワークの値をセット
 *
 * @param	work_no		ワークナンバー
 * @param	value		セットする値
 *
 * @return	"TRUE=セット出来た、FALSE=セット出来なかった"
 */
//------------------------------------------------------------------
BOOL SCRIPT_SetEventWorkValue(
	SCRIPT_WORK *sc, u16 work_no, u16 value )
{
  GAMEDATA * gdata;
	u16* res;
  GF_ASSERT(sc->magic_no == SCRIPT_MAGIC_NO);
  gdata = GAMESYSTEM_GetGameData( sc->gsys );
	res = SCRIPT_GetEventWork( sc, gdata, work_no );
	if( res == NULL ){ return FALSE; }
	*res = value;
	return TRUE;
}
//--------------------------------------------------------------
/**
 * スクリプト制御ワークのメンバーアドレス設定(ミュージカル
 * @param	sc	  SCRIPT_WORK型のポインタ
 */
//--------------------------------------------------------------
void SCRIPT_SetMemberWork_Musical( SCRIPT_WORK *sc, void *musEveWork )
{
	if( sc->magic_no != SCRIPT_MAGIC_NO ){
		GF_ASSERT_MSG(0, "起動(確保)していないスクリプトのワークにアクセスしています！" );
	}
	sc->musicalEventWork = musEveWork;
}


//------------------------------------------------------------------
/**
 * @brief	スクリプトから指定するOBJコードを取得
 * @param	fsys		FLDCOMMON_WORKへのポインタ
 * @param	no			0-15
 * @return	"OBJキャラコード"
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
u16 GetEvDefineObjCode( FLDCOMMON_WORK* fsys, u16 no )
{
	GF_ASSERT( (no < OBJCHR_WORK_MAX) && "引数が不正です！" );
	return GetEventWorkValue( fsys, (OBJCHR_WORK_START+no) );
}
#endif

//------------------------------------------------------------------
/**
 * @brief	スクリプトから指定するOBJコードをセット
 *
 * @param	fsys		FLDCOMMON_WORKへのポインタ
 * @param	no			0-15
 * @param	obj_code	OBJコード
 *
 * @return	"TRUE=セット出来た、FALSE=セット出来なかった"
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
BOOL SetEvDefineObjCode( FLDCOMMON_WORK* fsys, u16 no, u16 obj_code )
{
	GF_ASSERT( (no < OBJCHR_WORK_MAX) && "引数が不正です！" );
	return SetEventWorkValue( fsys, (OBJCHR_WORK_START+no), obj_code );
}
#endif

//--------------------------------------------------------------
/**
 * プログラムからスクリプトへの引数となるパラメータをセット
 * @param	sc SCRIPT_WORK
 * @param	prm0	パラメータ０（SCWK_PARAM0）
 * @param	prm1	パラメータ１（SCWK_PARAM1）
 * @param	prm2	パラメータ２（SCWK_PARAM2）
 * @param	prm3	パラメータ３（SCWK_PARAM3）
 * @retval none
 */
//--------------------------------------------------------------
void SCRIPT_SetScriptWorkParam( SCRIPT_WORK *sc, u16 prm0, u16 prm1, u16 prm2, u16 prm3 )
{
  GF_ASSERT(sc->magic_no == SCRIPT_MAGIC_NO);
  SCRIPT_SetEventWorkValue( sc, SCWK_PARAM0, prm0 );
  SCRIPT_SetEventWorkValue( sc, SCWK_PARAM1, prm1 );
  SCRIPT_SetEventWorkValue( sc, SCWK_PARAM2, prm2 );
  SCRIPT_SetEventWorkValue( sc, SCWK_PARAM3, prm3 );
}

//--------------------------------------------------------------
/**
 * サブプロセス用ワークのポインターアドレスを取得
 * @param	sc SCRIPT_WORK
 * @retval none
 *
 * 使い終わったら必ずNULLクリアすること！
 */
//--------------------------------------------------------------
void** SCRIPT_SetSubProcWorkPointerAdrs( SCRIPT_WORK *sc )
{
  return &sc->subproc_work;
}

//--------------------------------------------------------------
/**
 * サブプロセス用ワークのポインターを取得
 * @param	sc SCRIPT_WORK
 * @retval none
 *
 * 使い終わったら必ずNULLクリアすること！
 */
//--------------------------------------------------------------
void * SCRIPT_SetSubProcWorkPointer( SCRIPT_WORK *sc )
{
  return sc->subproc_work;
}

//--------------------------------------------------------------
/**
 * サブプロセス用ワーク領域の解放(ポインタがNULLでなければFree)
 * @param	sc SCRIPT_WORK
 * @retval none
 */
//--------------------------------------------------------------
void SCRIPT_FreeSubProcWorkPointer( SCRIPT_WORK *sc )
{
  if(sc->subproc_work != NULL){
    GFL_HEAP_FreeMemory(sc->subproc_work);
    sc->subproc_work = NULL;
  }
}

//============================================================================================
//	フラグ関連
//============================================================================================

//------------------------------------------------------------------
/**
 * @brief	マップ内限定のセーブフラグをクリアする
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @return	none
 */
//------------------------------------------------------------------
void LocalEventFlagClear( EVENTWORK * evwork )
{
	//ローカルフラグのクリア
  EVENTWORK_ClearEventFlags( evwork, FH_AREA_START, FH_AREA_END );
	//ローカルワークのクリア
  EVENTWORK_ClearEventWorks( evwork, LOCALWORKAREA_START, LOCALWORKAREA_END );
}

//------------------------------------------------------------------
/**
 * @brief	1日経過ごとにクリアされるフラグをクリアする
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @return	none
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
void TimeEventFlagClear( EVENTWORK * eventwork )
{
	//イベントワークのポインタ取得
	EVENTWORK* evwork = SaveData_GetEventWork(fsys->savedata);
	MI_CpuClear8( EventWork_GetEventFlagAdrs(evwork,TIMEFLAG_START), TIMEFLAG_AREA_MAX );
}
#endif

//============================================================================================
//	トレーナーフラグ関連
//	・スクリプトIDから、トレーナーIDを取得して、フラグチェック
//============================================================================================
#define GET_TRAINER_FLAG(id)	( TR_FLAG_START+(id) )

//--------------------------------------------------------------
/**
 * スクリプトIDから、トレーナーIDを取得
 *
 * @param   scr_id		スクリプトID
 *
 * @retval  "トレーナーID = フラグインデックス"
 */
//--------------------------------------------------------------
u16 SCRIPT_GetTrainerID_ByScriptID( u16 scr_id )
{
#if 0 //pl
	if( scr_id < ID_TRAINER_2VS2_OFFSET ){
		return (scr_id - ID_TRAINER_OFFSET + 1);		//1オリジン
	}else{
		return (scr_id - ID_TRAINER_2VS2_OFFSET + 1);		//1オリジン
	}
#else
	if( scr_id < ID_TRAINER_2VS2_OFFSET ){
		return (scr_id - ID_TRAINER_OFFSET);		//0オリジン
	}else{
		return (scr_id - ID_TRAINER_2VS2_OFFSET);		//0オリジン
	}
#endif
}

//--------------------------------------------------------------
/**
 * スクリプトIDから、左右どちらのトレーナーか取得
 * @param   scr_id		スクリプトID
 * @retval  "0=左、1=右"
 */
//--------------------------------------------------------------
BOOL SCRIPT_GetTrainerLR_ByScriptID( u16 scr_id )
{
	if( scr_id < ID_TRAINER_2VS2_OFFSET ){
		return 0;
	}else{
		return 1;
	}
}

//--------------------------------------------------------------
/**
 * トレーナーIDから、ダブルバトルタイプか取得
 * @param   tr_id		トレーナーID
 * @retval  "0=シングルバトル、1=ダブルバトル"
 */
//--------------------------------------------------------------
BOOL SCRIPT_CheckTrainer2vs2Type( u16 tr_id )
{
  u32 rule = TT_TrainerDataParaGet( tr_id, ID_TD_fight_type );
  
  OS_Printf( "check 2v2 TRID=%d, type = %d\n", tr_id, rule );

  switch( rule ){
  case BTL_RULE_SINGLE:
    return( 0 );
  case BTL_RULE_DOUBLE:
  case BTL_RULE_TRIPLE: //kari
    return( 1 );
  }
  
//  GF_ASSERT( 0 );
  return( 1 );
}

//------------------------------------------------------------------
/**
 * @brief	トレーナーイベントフラグをチェックする
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @param	tr_id		トレーナーID
 * @retval	"1 = フラグON"
 * @retval	"0 = フラグOFF"
 */
//------------------------------------------------------------------
BOOL SCRIPT_CheckEventFlagTrainer( EVENTWORK *ev, u16 tr_id )
{
  return EVENTWORK_CheckEventFlag(ev,GET_TRAINER_FLAG(tr_id) );
}

//------------------------------------------------------------------
/**
 * @brief	トレーナーイベントフラグをセットする
 *
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @param	tr_id		トレーナーID
 *
 * @return	none
 */
//------------------------------------------------------------------
void SCRIPT_SetEventFlagTrainer( EVENTWORK *ev, u16 tr_id )
{
  EVENTWORK_SetEventFlag( ev, GET_TRAINER_FLAG(tr_id) );
}

//------------------------------------------------------------------
/**
 * @brief	トレーナーイベントフラグをリセットする
 *
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @param	tr_id		トレーナーID
 *
 * @return	none
 */
//------------------------------------------------------------------
void SCRIPT_ResetEventFlagTrainer( EVENTWORK *ev, u16 tr_id )
{
	EVENTWORK_ResetEventFlag( ev, GET_TRAINER_FLAG(tr_id) );
}

//--------------------------------------------------------------
/**
 * トレーナー視線情報を格納 事前にSCRIPT_SetEventScript()を起動しておく事
 * @param	event SCRIPT_SetEventScript()戻り値。
 * @param	mmdl 視線がヒットしたFIELD_OBJ_PTR
 * @param	range		グリッド単位の視線距離
 * @param	dir			移動方向
 * @param	scr_id		視線ヒットしたスクリプトID
 * @param	tr_id		視線ヒットしたトレーナーID
 * @param	tr_type		トレーナータイプ　シングル、ダブル、タッグ識別
 * @param	tr_no		何番目にヒットしたトレーナーなのか
 */
//--------------------------------------------------------------
void SCRIPT_SetTrainerEyeData( GMEVENT *event, MMDL *mmdl,
    s16 range, u16 dir, u16 scr_id, u16 tr_id, int tr_type, int tr_no )
{
  SCRIPT_WORK *sc =  SCRIPT_GetEventWorkToScriptWork( event );
	EV_TRAINER_EYE_HITDATA *eye = &sc->eye_hitdata[tr_no];
  TRAINER_EYE_HITDATA_Set(eye, mmdl, range, dir, scr_id, tr_id, tr_type, tr_no );
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
  SCRIPT_WORK *sc = SCRIPTWORK_Create( HEAPID_PROC, heapID, gsys, script_id, NULL, NULL);
  
  core = SCRVM_Create( sc, sc->start_zone_id, script_id );
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
	if( scr_id == SCRIPT_NOT_EXIST_ID ){
		return FALSE;
	} else {
    //特殊スクリプト実行
    SCRIPT_CallSpecialScript( gsys, heapID, scr_id );
  }
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
  LocalEventFlagClear(ev);
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
	if( scr_id == SCRIPT_NOT_EXIST_ID ){
		return NULL;
  }
  {
    event = SCRIPT_SetEventScript( gsys, scr_id, NULL, heapID );
  }
  return event;
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
    if (*p == SP_SCRID_NONE)
    {
      return SCRIPT_NOT_EXIST_ID;
    }
		//指定タイプの特殊スクリプト検索
    if (*p == key)
    {
      p+= 2; //特殊スクリプトID分足す
      return ( *p + ( *(p+1)<<8) );		//スクリプトIDを返す
    }
    p += (2 + 2 + 2);						//特殊スクリプトID+スクリプトID+ダミー分足す
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

	while( TRUE ){
		//終了記述チェック(SP_EVENT_DATA_END)
		if( *p == SP_SCRID_NONE ){
			return SCRIPT_NOT_EXIST_ID;
		}

		//指定タイプの特殊スクリプト検索
		if( (*p) == key ){
			p += 2;			//特殊スクリプトID分足す
			pos = ( *p + ( *(p+1)<<8 ) + ( *(p+2)<<16 ) + ( *(p+3)<<24 ) );
			p += 4;			//アドレス分足す
			break;
		}

		p += (2 + 4);		//特殊スクリプトID+アドレス分足す
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
		if( *p == 0 ){
			return SCRIPT_NOT_EXIST_ID;
		}

		//比較するワーク取得
		work1 = ( *p + ( *(p+1)<<8 ) );
		if( work1 == 0 ){
			return SCRIPT_NOT_EXIST_ID;
		};
		p += 2;									//ワーク分足す
		//OS_Printf( "work1 = %d\n", work1 );

		//比較する値(ワーク可)取得
		work2 = ( *p + ( *(p+1)<<8 ) );
		p += 2;									//比較する値分足す(ワーク可)
		//OS_Printf( "work2 = %d\n", work2 );

		//条件チェック
		//if( EVENTWORK_GetEventWorkAdrs(ev,work1) == EVENTWORK_GetEventWorkAdrs(ev,work2) ){
		if( *(EVENTWORK_GetEventWorkAdrs(ev,work1)) == work2 ){
			return ( *p + ( *(p+1)<<8 ) );
		}

		p += 2;									//スクリプトID分足す
	}
	return SCRIPT_NOT_EXIST_ID;								//何も見つからなかった
}
