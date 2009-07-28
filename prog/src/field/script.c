//======================================================================
/**
 * @file	script.c
 * @bfief	スクリプト制御メイン部分
 * @author	Satoshi Nohara
 * @date	05.08.04
 *
 * 01.11.07	Sousuke Tamada
 * 03.04.15	Satoshi Nohara
 * 05.04.25 Hiroyuki Nakamura
 */
//======================================================================
#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "system/vm.h"
#include "system/vm_cmd.h"

#include "field/zonedata.h"

#include "script_message.naix"
#include "print/wordset.h"

#include "arc/fieldmap/zone_id.h"

#include "script.h"
#include "script_offset_id.h" //スクリプトIDオフセット
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"
#include "eventwork_def.h"

#include "arc/fieldmap/script_seq.naix"

#include "../../../resource/fldmapdata/script/init_scr_def.h"

//======================================================================
//	デバック
//======================================================================
//VM_CODE* sp_script;

//======================================================================
//  define
//======================================================================
#define TRAINER_EYE_HITMAX (2) ///<トレーナー視線データ最大数

enum
{
	WORDSET_SCRIPT_SETNUM = 8,		//デフォルトバッファ数
	WORDSET_SCRIPT_BUFLEN = 64,		//デフォルトバッファ長（文字数）
};

#define SCR_MSG_BUF_SIZE	(1024)				//メッセージバッファサイズ

//======================================================================
//	struct
//======================================================================
//関数ポインタ型
typedef void (*SCRIPT_EVENTFUNC)(GMEVENT *fsys);

//--------------------------------------------------------------
//  トレーナー視線データ構造体
//--------------------------------------------------------------
typedef struct {
	int range;				//視線距離
	int dir;					//移動方向
	int scr_id;				//スクリプトID
	int tr_id;				//トレーナーID
	int tr_type;			//トレーナータイプ
	MMDL *mmdl;
  GMEVENT *ev_eye_move;
}EV_TRAINER_EYE_HITDATA;

//--------------------------------------------------------------
//	スクリプト制御ワーク構造体
//--------------------------------------------------------------
struct _TAG_SCRIPT_WORK
{
	u32 magic_no;			//イベントのワークがスクリプト制御ワークかを判別
	
	u8 seq;					//スクリプトシーケンス
	u8 MsgIndex;			//メッセージインデックス
	u8 anm_count;			//アニメーションしている数
	u8 common_scr_flag;		//ローカル、共通 切り替えフラグ(0=local、1=共通)
	
	u8 win_open_flag;		//会話ウィンドウ表示フラグ(0=非表示、1=表示)
	u8 vm_machine_count;	//追加した仮想マシンの数
	u16 script_id;			//メインのスクリプトID
	
	BOOL win_flag;			///<戦闘結果保持用ワーク
	
#ifndef SCRIPT_PL_NULL
	//イベントウィンドウ
	EV_WIN_WORK* ev_win;			//イベントウィンドウワークへのポインタ
	//会話ウィンドウ
	GF_BGL_BMPWIN MsgWinDat;		//ビットマップウィンドウデータ
#endif
	
#ifndef SCRIPT_PL_NULL
	BMPMENU_WORK* mw;				//ビットマップメニューワーク
#else
  void *mw;
#endif

#ifndef SCRIPT_PL_NULL
	int player_dir;					//イベント起動時の主人公の向き
	FIELD_OBJ_PTR target_obj;		//話しかけ対象のOBJのポインタ
	FIELD_OBJ_PTR dummy_obj;		//透明ダミーOBJのポインタ
#else
	int player_dir;					//イベント起動時の主人公の向き
	MMDL *target_obj;
	MMDL *dummy_obj;
#endif
	u16 *ret_script_wk;			//スクリプト結果を代入するワークのポインタ
	VMHANDLE *vm[VMHANDLE_MAX];	//仮想マシンへのポインタ

	WORDSET* wordset;				//単語セット
	STRBUF* msg_buf;				//メッセージバッファポインタ
	STRBUF* tmp_buf;				//テンポラリバッファポインタ
  
#ifndef SCRIPT_PL_NULL
	void * waiticon;				///<待機アイコンのポインタ
#endif
	
	//トレーナー視線情報
	EV_TRAINER_EYE_HITDATA eye_hitdata[TRAINER_EYE_HITMAX];
	
	u16 work[EVSCR_WORK_MAX];		//ワーク(ANSWORK,TMPWORKなどの代わり)
	
	SCRIPT_EVENTFUNC next_func;		//スクリプト終了時に呼び出される関数
	
	void * subproc_work; //サブプロセスとのやりとりに使用するワークへの*
	void * pWork;					//ワークへの汎用ポインタ

#ifndef SCRIPT_PL_NULL
	EOA_PTR eoa;		//主にフィールドエフェクトのポインタとして使う
#endif

#ifndef SCRIPT_PL_NULL
	TCB_PTR player_tcb;				//自機形態レポートTCB
	
	GF_BGL_BMPWIN CoinWinDat;		//ビットマップウィンドウデータ
	GF_BGL_BMPWIN GoldWinDat;		//ビットマップウィンドウデータ
	
	REPORT_INFO * riw;				///<レポート情報用ウィンドウ制御ワーク
#endif
	
	//WB
	GAMESYS_WORK *gsys;
	HEAPID heapID;
	FLDMSGBG *msgBG;
  u32 zone_id;

	SCRIPT_FLDPARAM fld_param;
  
  GMEVENT *gmevent; //スクリプトを実行しているGMEVENT*
};

//--------------------------------------------------------------
//	スクリプトイベント用ワーク
//--------------------------------------------------------------
typedef struct
{
	SCRIPT_WORK *sc; //スクリプト用ワーク
	SCRCMD_WORK *cmd_wk; //スクリプトコマンド用ワーク
}EVENT_SCRIPT_WORK;

//======================================================================
//	隠しアイテムデータ
//======================================================================
#ifndef SCRIPT_PL_NULL
typedef struct{
	u16 itemno;									//アイテムナンバー
	u8	num;									//個数
	u8	response;								//反応度
	u16	sp;										//特殊(未使用)
	u16	index;									//フラグインデック
}HIDE_ITEM_DATA;

//#include "../fielddata/script/hide_item.dat"	//隠しアイテムデータ
#endif

//======================================================================
//	プロトタイプ宣言
//======================================================================
static SCRIPT_WORK* EvScriptWork_Alloc( HEAPID heapID );
static void script_del( VMHANDLE* core );
static void EvScriptWork_Init( SCRIPT_WORK *sc, GAMESYS_WORK *gsys,
	u16 scr_id, MMDL *obj, void* ret_wk);

static void InitScript(
	SCRCMD_WORK *work, VMHANDLE* core, u32 zone_id, u16 id, u8 type, HEAPID heapID );
static u16 SetScriptDataSub( SCRCMD_WORK *work, VMHANDLE* core, u32 zone_id, u16 id, HEAPID heapID );
static void SetScriptData(
	SCRCMD_WORK *work, VMHANDLE* core, int index, u32 dat_id, HEAPID heapID );
static void SetZoneScriptData( FLDCOMMON_WORK* fsys, VMHANDLE* core );

//SCRIPTのメンバーアクセス
static void EventDataIDJump( VMHANDLE * core, u16 ev_id );

static void * LoadZoneScriptFile(int zone_id);
static u32 LoadZoneMsgNo(int zone_id);

//イベント
static GMEVENT * FldScript_CreateControlEvent( SCRIPT_WORK *sc );
static void FldScript_CallControlEvent( SCRIPT_WORK *sc, GMEVENT *event );
static GMEVENT * FldScript_ChangeControlEvent( SCRIPT_WORK *sc, GMEVENT *event );

//イベントワーク、スクリプトワークアクセス関数
#ifndef SCRIPT_PL_NULL
u16 * GetEventWorkAdrs( FLDCOMMON_WORK* fsys, u16 work_no );
u16 GetEventWorkValue( FLDCOMMON_WORK* fsys, u16 work_no );
BOOL SetEventWorkValue( FLDCOMMON_WORK* fsys, u16 work_no, u16 value );

u16 GetEvDefineObjCode( FLDCOMMON_WORK* fsys, u16 no );
BOOL SetEvDefineObjCode( FLDCOMMON_WORK* fsys, u16 no, u16 obj_code );
#endif

//イベントフラグ
#ifndef SCRIPT_PL_NULL
BOOL CheckEventFlag( FLDCOMMON_WORK* fsys, u16 flag_no);
void SetEventFlag( FLDCOMMON_WORK* fsys, u16 flag_no);
void ResetEventFlag( FLDCOMMON_WORK* fsys, u16 flag_no);

void LocalEventFlagClear( FLDCOMMON_WORK* fsys );
void TimeEventFlagClear( FLDCOMMON_WORK* fsys );
#endif

//トレーナー関連
#ifndef SCRIPT_PL_NULL
u16 GetTrainerIdByScriptId( u16 scr_id );
BOOL GetTrainerLRByScriptId( u16 scr_id );
BOOL CheckTrainer2vs2Type( u16 tr_id );
BOOL CheckEventFlagTrainer( FLDCOMMON_WORK* fsys, u16 tr_id );
void SetEventFlagTrainer( FLDCOMMON_WORK* fsys, u16 tr_id );
void ResetEventFlagTrainer( FLDCOMMON_WORK* fsys, u16 tr_id );
#endif

//隠しアイテム
#ifndef SCRIPT_PL_NULL
u16 GetHideItemFlagNoByScriptId( u16 scr_id );
u16 GetHideItemFlagIndexByScriptId( u16 scr_id );
u8 GetHideItemResponseByScriptId( u16 scr_id );
static BOOL HideItemParamSet( SCRIPT_WORK* sc, u16 scr_id );
void HideItemFlagOneDayClear( FLDCOMMON_WORK* fsys );
#endif

//特殊スクリプト
#ifndef SCRIPT_PL_NULL
void GameStartScriptInit( FLDCOMMON_WORK* fsys );
void SpScriptStart( FLDCOMMON_WORK* fsys, u16 scr_id );
BOOL SpScriptSearch( FLDCOMMON_WORK* fsys, u8 key );
static u16 SpScriptSearch_Sub( const u8 * p, u8 key );
#endif

//======================================================================
//	グローバル変数
//======================================================================
#define SCRIPT_MAGIC_NO		(0x3643f)

//========================================================================
//
//	スクリプト制御
//
//	●スクリプトイベントセット
//		EventSet_Script(...)
//
//	●処理の流れ
//		EvScriptWork_Alloc(...)					ワーク確保
//		EvScriptWork_Init(...)					初期設定
//		FieldEvent_Set(...)						イベント設定
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
//========================================================================
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
		HEAPID heapID, const SCRIPT_FLDPARAM *fparam )
{
	GMEVENT *event;
	
	SCRIPT_WORK *sc = EvScriptWork_Alloc( heapID );		//ワーク確保
	sc->fld_param = *fparam;
	EvScriptWork_Init( sc, gsys, scr_id, obj, NULL );	//初期設定
	event = FldScript_CreateControlEvent( sc );
	return( event );
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
  EVENT_SCRIPT_WORK *ev_sc = GMEVENT_GetEventWork( event );
  SCRIPT_WORK *sc = ev_sc->sc;
	EV_TRAINER_EYE_HITDATA *eye = &sc->eye_hitdata[tr_no];
  
  eye->range = range;
	eye->dir = dir;
	eye->scr_id = scr_id;
	eye->tr_id = tr_id;
	eye->tr_type = tr_type;
	eye->mmdl = mmdl;
  eye->ev_eye_move = NULL;
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
void SCRIPT_CallScript( GMEVENT *event,
	u16 scr_id, MMDL *obj, void *ret_script_wk, HEAPID heapID )
{
	SCRIPT_WORK *sc = EvScriptWork_Alloc( heapID );	//ワーク確保
	EvScriptWork_Init( sc, GMEVENT_GetGameSysWork(event), scr_id, obj, ret_script_wk );	//初期設定
	FldScript_CallControlEvent( sc, event );
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
void SCRIPT_ChangeScript( GMEVENT *event,
		u16 scr_id, MMDL *obj, HEAPID heapID )
{
	SCRIPT_WORK *sc = EvScriptWork_Alloc( heapID );	//ワーク確保
	EvScriptWork_Init( sc, GMEVENT_GetGameSysWork(event), scr_id, obj, NULL );	//初期設定
	FldScript_ChangeControlEvent( sc, event );
}


//--------------------------------------------------------------
/**
 * @brief	スクリプト制御メイン
 *
 * @param	event		GMEVENT_CONTROL型
 *
 * @retval	"FALSE = スクリプト実行中"
 * @retval	"TRUE = スクリプト実行終了"
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
static BOOL GMEVENT_ControlScript(GMEVENT_CONTROL * event)
{
	int i;
	fsysFunc func;
	VMHANDLE* core = NULL;
	SCRIPT_WORK * sc = FieldEvent_GetSpecialWork( event );
	FLDCOMMON_WORK* fsys = FieldEvent_GetFieldSysWork( event );

	switch(sc->seq){
	case 0:
		//仮想マシン追加
		sc->vm[VMHANDLE_MAIN] = VMMachineAdd( fsys, sc->script_id );
		//VMMachineAdd( fsys, id, &ScriptCmdTbl[0], &ScriptCmdTbl[EVCMD_MAX] );
		sc->vm_machine_count = 1;

		//MSGMAN_Createの後に処理
		//sc->wordset = WORDSET_Create( HEAPID_WORLD );
		sc->wordset = WORDSET_CreateEx(WORDSET_SCRIPT_SETNUM, WORDSET_SCRIPT_BUFLEN, HEAPID_WORLD);
		sc->msg_buf = STRBUF_Create( SCR_MSG_BUF_SIZE, HEAPID_WORLD );
		sc->tmp_buf = STRBUF_Create( SCR_MSG_BUF_SIZE, HEAPID_WORLD );
    
		sc->seq++;
		/* FALL THROUGH */
		
	case 1:
		//仮想マシンコントロール
		for( i=0; i < VMHANDLE_MAX; i++ ){

			core = sc->vm[i];

			if( core != NULL ){

				if( VM_Control(core) == FALSE ){				//制御メイン

					script_del(core);							//スクリプト削除

					if( sc->vm_machine_count == 0 ){
						GF_ASSERT( (0) && "仮想マシンの数が不正です！" );
					}

					sc->vm[i] = NULL;
					sc->vm_machine_count--;
				}
			}
		}

		//仮想マシンの数をチェック
		if( sc->vm_machine_count <= 0 ){

			func = sc->next_func;								//退避

			WORDSET_Delete( sc->wordset );
			STRBUF_Delete( sc->msg_buf );
			STRBUF_Delete( sc->tmp_buf );

			//デバック処理
			//debug_script_flag = 0;
			sc->magic_no = 0;

			sys_FreeMemoryEz( sc );								//スクリプトワーク

			//スクリプト終了時に呼び出される関数実行
			if( func != NULL ){
				func(fsys);
				return FALSE;				//FieldEventを終了しないようにFALSEを返す！
			}

			return TRUE;										//FieldEvent_Set終了！
		}
		break;
	};
	return FALSE;
}
#endif

//--------------------------------------------------------------
/**
 * @brief	スクリプト制御ワーク確保
 *
 * @param	fsys		FLDCOMMON_WORK型のポインタ
 *
 * @retval	"スクリプト制御ワークのアドレス"
 */
//--------------------------------------------------------------
static SCRIPT_WORK * EvScriptWork_Alloc( HEAPID heapID )
{
	SCRIPT_WORK *sc;
	
	sc = GFL_HEAP_AllocClearMemory( heapID, sizeof(SCRIPT_WORK) );
	
	//デバック処理
	//debug_script_flag = 1;
	
	sc->magic_no = SCRIPT_MAGIC_NO;
	sc->heapID = heapID;
	return sc;
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
static void script_del( VMHANDLE *core )
{
	#ifndef SCRIPT_PL_NULL
	MSGMAN_Delete( core->msgman );
	sys_FreeMemoryEz( core->pScript );	//スクリプトデータ
	sys_FreeMemoryEz( core );
	return;
	#else
	SCRCMD_WORK *cmd = VM_GetContext( core );
	SCRCMD_WORK_Delete( cmd );
	GFL_HEAP_FreeMemory( core->pScript );
	VM_Delete( core );
	#endif
}

//-----------------ON=---------------------------------------------
/**
 * @brief	スクリプト制御ワーク初期設定
 *
 * @param	fsys		FLDCOMMON_WORK型のポインタ
 * @param	sc			SCRIPT型のポインタ
 * @param	scr_id		スクリプトID
 * @param	obj			話しかけ対象OBJのポインタ
 * @param	ret_wk		スクリプト結果を代入するワークのポインタ
 *
 * @retval	"スクリプト制御ワークのアドレス"
 */
//--------------------------------------------------------------
static void EvScriptWork_Init( SCRIPT_WORK *sc,
	GAMESYS_WORK *gsys,	u16 scr_id, MMDL *obj, void *ret_wk )
{
	u16 *objid;
	
	sc->gsys = gsys;
	objid = SCRIPT_GetSubMemberWork( sc, ID_EVSCR_WK_TARGET_OBJID );
	
	//DIR_NOT = -1
	#ifndef SCRIPT_PL_NULL
	sc->player_dir = Player_DirGet( fsys->player ); //起動時の自機向きセット
	#endif
	sc->target_obj = obj;		//話しかけ対象OBJのポインタセット
	sc->script_id  = scr_id;	//メインのスクリプトID
	sc->ret_script_wk = ret_wk;	//スクリプト結果を代入するワーク
  
  { //wb
    PLAYER_WORK *pw = GAMESYSTEM_GetMyPlayerWork( gsys );
  	sc->zone_id = PLAYERWORK_getZoneID( pw );
  }
  
	if( obj != NULL ){
		*objid = MMDL_GetOBJID( obj ); //話しかけ対象OBJIDのセット
	}

#ifndef SCRIPT_PL_NULL
	//隠しアイテムのスクリプトIDだったら(あとで移動する予定)
	if( (scr_id >= ID_HIDE_ITEM_OFFSET) &&
		(scr_id <= ID_HIDE_ITEM_OFFSET_END) ){
		HideItemParamSet( sc, scr_id );
	}
	
	#ifdef DEBUG_SCRIPT
	OS_Printf( "ZONE:%s SCRID:%d\n",
		ZoneData_GetZoneName(fsys->location->zone_id), scr_id);
	#endif
#endif
}

//--------------------------------------------------------------
/**
 * @brief	仮想マシン追加
 * @param	fsys		FLDCOMMON_WORK型のポインタ
 * @param	id			スクリプトID
 * @param	start		命令テーブル開始アドレス
 * @param	end			命令テーブル終了アドレス
 * @retval	none
 */
//--------------------------------------------------------------
VMHANDLE * SCRIPT_AddVMachine(
		GAMESYS_WORK *gsys,
		SCRIPT_WORK *sc,
		HEAPID heapID,
		u16 scr_id )
{
	SCRCMD_WORK_HEADER head;
	VMHANDLE *core;
	VM_INITIALIZER init;
	SCRCMD_WORK *work;
	
	head.gsys = gsys;
	head.gdata = GAMESYSTEM_GetGameData( gsys );
	head.mmdlsys = GAMEDATA_GetMMdlSys( head.gdata );
	head.script = sc;
	
	work = SCRCMD_WORK_Create( &head, heapID );
	
	init.stack_size = 0x0100;
	init.reg_size = 0x0100;
	init.command_table = ScriptCmdTbl;
	
	#if 0	//command_maxがconstの為初期化出来ない
	init.command_max  = ScriptCmdMax;
	#else
	{
		u32 *max = (u32*)(&init.command_max);
		*max = ScriptCmdMax;
	}
	#endif
	
	core = VM_Create( heapID, &init );	//仮想マシン初期化
	
	VM_Init( core, work );
	InitScript( work, core, sc->zone_id, scr_id, 0, heapID );//ローカルスクリプト初期化
	return core;
}

//--------------------------------------------------------------
/**
 * @brief	スクリプト設定初期化
 * @param	fsys		FLDCOMMON_WORK型のポインタ
 * @param	core		VMHANDLE型のポインタ
 * @param	id			スクリプトID
 * @param	type		"未使用"
 * @retval	none
 */
//--------------------------------------------------------------
static void InitScript(
	SCRCMD_WORK *work, VMHANDLE* core, u32 zone_id, u16 id, u8 type, HEAPID heapID )
{
	u16 scr_id;
	scr_id = SetScriptDataSub( work, core, zone_id, id, heapID ); //スクリプトデータ、メッセージデータ読み込み
	VM_Start( core, core->pScript ); //仮想マシンにコード設定
	EventDataIDJump( core, scr_id );
//	VM_SetWork(core,(void *)fsys->event); //コマンドなどで参照するワークセット
	return;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef struct {
	u16 scr_id;
	u16 scr_arc_id;
	u16 msg_arc_id;
}SCRIPT_ARC_TABLE;

//※スクリプトIDが大きい順に定義すること！！！
static const SCRIPT_ARC_TABLE ScriptArcTable[] = {
  //初期化スクリプトのIDが渡されたとき
  { ID_INIT_SCR_OFFSET, NARC_script_seq_init_scr_bin, NARC_script_message_common_scr_dat },
	//トレーナースクリプトのIDが渡された時
  { ID_TRAINER_OFFSET ,NARC_script_seq_trainer_bin, NARC_script_message_common_scr_dat },
	//BGスクリプトのIDが渡された時
  { ID_BG_ATTR_OFFSET ,NARC_script_seq_bg_attr_bin, NARC_script_message_bg_attr_dat },
	//共通スクリプトのIDが渡された時
  { ID_COMMON_SCR_OFFSET ,NARC_script_seq_common_scr_bin, NARC_script_message_common_scr_dat },
};

//--------------------------------------------------------------
/**
 * @brief	スクリプトIDからスクリプトデータ、メッセージデータを読み込み
 * @param	work
 * @param	core		VMHANDLE型のポインタ
 * @param zone_id
 * @param	id			スクリプトID
 * @retval	"スクリプトIDからオフセットを引いた値"
 */
//--------------------------------------------------------------
static u16 SetScriptDataSub( SCRCMD_WORK *work, VMHANDLE* core, u32 zone_id, u16 id, HEAPID heapID )
{
  int i;
  u16 scr_id = id;
  const SCRIPT_ARC_TABLE * tbl = ScriptArcTable;

  for (i = 0; i < NELEMS(ScriptArcTable); i++) {
    if (scr_id >= tbl[i].scr_id) {
      SetScriptData(work, core, tbl[i].scr_arc_id, tbl[i].msg_arc_id, heapID);
      scr_id -= tbl[i].scr_id;
      return scr_id;
    }
  }
  //ローカルスクリプトのIDが渡された時
  if( scr_id >= ID_START_SCR_OFFSET ){
    u16 idx_script = ZONEDATA_GetScriptArcID( zone_id );
    u16 idx_msg = ZONEDATA_GetMessageArcID( zone_id );
    SetScriptData( work, core, idx_script, idx_msg, heapID );
    scr_id -= ID_START_SCR_OFFSET;
    OS_Printf( "ゾーンスクリプト起動 scr_idx = %d, msg_idx = %d\n",
        idx_script, idx_msg );
    return scr_id;

  //SCRID_NULL(0)が渡された時
  }
  SetScriptData( work, core,
      NARC_script_seq_dummy_scr_bin,
      NARC_script_message_common_scr_dat, heapID );
  scr_id = 0;
  return scr_id;
}


#if 0 //SCRIPT_PL_NULL
static u16 SetScriptDataSub( SCRCMD_WORK *work, VMHANDLE* core, u16 id )
{
	u16 scr_id = id;
	

	//スクラッチスクリプトのIDが渡された時
	if( scr_id >= ID_SCRATCH_OFFSET ){
		SetScriptData(
			fsys, core,
			NARC_scr_seq_scratch_bin,
			NARC_msg_scratch_counter_dat );
		scr_id -= ID_SCRATCH_OFFSET;
	//フロンティア成績スクリプトのIDが渡された時
	}else if( scr_id >= ID_SEISEKI_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_seiseki_bin, NARC_msg_bf_seiseki_dat );
		scr_id -= ID_SEISEKI_OFFSET;

	//ドサ周りスクリプトのIDが渡された時
	}else if( scr_id >= ID_DOSA_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_dosa_bin, NARC_msg_circuit_trainer_dat );
		scr_id -= ID_DOSA_OFFSET;

	//サポートスクリプトのIDが渡された時
	}else  if( scr_id >= ID_SUPPORT_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_support_bin, NARC_msg_support_dat );
		scr_id -= ID_SUPPORT_OFFSET;

	//配達員スクリプトのIDが渡された時
	}else if( scr_id >= ID_HAITATU_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_haitatu_bin, NARC_msg_haitatu_dat );
		scr_id -= ID_HAITATU_OFFSET;

	//TVインタビュースクリプトのIDが渡された時
	}else if( scr_id >= ID_TV_INTERVIEW_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_tv_interview_bin, NARC_msg_tv_interview_dat );
		scr_id -= ID_TV_INTERVIEW_OFFSET;

	//TVスクリプトのIDが渡された時
	}else if( scr_id >= ID_TV_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_tv_bin, NARC_msg_tv_program_dat );
		scr_id -= ID_TV_OFFSET;

	//秘伝スクリプトのIDが渡された時
	}else if( scr_id >= ID_HIDEN_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_hiden_bin, NARC_msg_hiden_dat );
		scr_id -= ID_HIDEN_OFFSET;

	//評価スクリプトのIDが渡された時
	}else if( scr_id >= ID_HYOUKA_SCR_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_hyouka_scr_bin, NARC_msg_hyouka_dat );
		scr_id -= ID_HYOUKA_SCR_OFFSET;

	//デバックスクリプトのIDが渡された時
	}else if( scr_id >= ID_DEBUG_SCR_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_debug_scr_bin, NARC_msg_common_scr_dat );
		scr_id -= ID_DEBUG_SCR_OFFSET;

	//コンテスト受付スクリプトのIDが渡された時
	}else if( scr_id >= ID_CON_RECEPTION_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_con_reception_bin, NARC_msg_con_reception_dat );
		scr_id -= ID_CON_RECEPTION_OFFSET;

	//連れ歩きスクリプトのIDが渡された時
	}else if( scr_id >= ID_PAIR_SCR_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_pair_scr_bin, NARC_msg_pair_scr_dat );
		scr_id -= ID_PAIR_SCR_OFFSET;

	//ゲーム開始スクリプトのIDが渡された時
	}else if( scr_id >= ID_INIT_SCR_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_init_scr_bin, NARC_msg_common_scr_dat );
		scr_id -= ID_INIT_SCR_OFFSET;

	//育て屋スクリプトのIDが渡された時
	}else if( scr_id >= ID_SODATEYA_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_sodateya_bin, NARC_msg_sodateya_dat );
		scr_id -= ID_SODATEYA_OFFSET;

	//ポルトミニゲームスクリプトのIDが渡された時
	}else if( scr_id >= ID_PORUTO_SCR_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_poruto_scr_bin, NARC_msg_pgamestart_dat );
		scr_id -= ID_PORUTO_SCR_OFFSET;

	//グループスクリプトのIDが渡された時
	}else if( scr_id >= ID_GROUP_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_group_bin, NARC_msg_group_dat );
		scr_id -= ID_GROUP_OFFSET;

	//ポケセン地下スクリプトのIDが渡された時
	}else if( scr_id >= ID_PC_UG_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_pc_ug_bin, NARC_msg_pc_ug_dat );
		scr_id -= ID_PC_UG_OFFSET;

	//コロシアムスクリプトのIDが渡された時
	}else if( scr_id >= ID_BATTLE_ROOM_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_battle_room_bin, NARC_msg_battle_room_dat );
		scr_id -= ID_BATTLE_ROOM_OFFSET;

	//通信スクリプトのIDが渡された時
	}else if( scr_id >= ID_CONNECT_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_connect_bin, NARC_msg_connect_dat );
		scr_id -= ID_CONNECT_OFFSET;

	//ポケサーチャースクリプトのIDが渡された時
	}else if( scr_id >= ID_POKESEARCHER_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_pokesearcher_bin, NARC_msg_bag_dat );
		scr_id -= ID_POKESEARCHER_OFFSET;

	//再戦スクリプトのIDが渡された時
	}else if( scr_id >= ID_SAISEN_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_saisen_bin, NARC_msg_saisen_dat );
		scr_id -= ID_SAISEN_OFFSET;

	//ペラップスクリプトのIDが渡された時
	}else if( scr_id >= ID_PERAP_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_perap_bin, NARC_msg_perap_dat );
		scr_id -= ID_PERAP_OFFSET;

	//サファリスクリプトのIDが渡された時
	}else if( scr_id >= ID_SAFARI_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_safari_bin, NARC_msg_safari_dat );
		scr_id -= ID_SAFARI_OFFSET;

	//隠しアイテムスクリプトのIDが渡された時
	}else if( scr_id >= ID_HIDE_ITEM_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_hide_item_bin, NARC_msg_hide_item_dat );
		scr_id -= ID_HIDE_ITEM_OFFSET;

	//フィールドアイテムスクリプトのIDが渡された時
	}else if( scr_id >= ID_FLD_ITEM_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_fld_item_bin, NARC_msg_fld_item_dat );
		scr_id -= ID_FLD_ITEM_OFFSET;

	//2vs2トレーナースクリプトのIDが渡された時
	}else if( scr_id >= ID_TRAINER_2VS2_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_trainer_bin, NARC_msg_common_scr_dat );
		scr_id -= ID_TRAINER_2VS2_OFFSET;

	//トレーナースクリプトのIDが渡された時
	}else if( scr_id >= ID_TRAINER_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_trainer_bin, NARC_msg_common_scr_dat );
		scr_id -= ID_TRAINER_OFFSET;

	//きのみスクリプトのIDが渡された時
	}else if( scr_id >= ID_KINOMI_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_kinomi_bin, NARC_msg_kinomi_dat );
		scr_id -= ID_KINOMI_OFFSET;

	//BGスクリプトのIDが渡された時
	}else if( scr_id >= ID_BG_ATTR_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_bg_attr_bin, NARC_msg_bg_attr_dat );
		scr_id -= ID_BG_ATTR_OFFSET;

	//共通スクリプトのIDが渡された時
	}else if( scr_id >= ID_COMMON_SCR_OFFSET ){
		SetScriptData( fsys, core, NARC_scr_seq_common_scr_bin, NARC_msg_common_scr_dat );
		scr_id -= ID_COMMON_SCR_OFFSET;

	//ローカルスクリプトのIDが渡された時
	}else if( scr_id >= ID_START_SCR_OFFSET ){
		SetZoneScriptData( fsys, core );						//ゾーンスクリプトデータセット
		scr_id -= ID_START_SCR_OFFSET;

	//SCRID_NULL(0)が渡された時
	}else{
		SetScriptData( fsys, core, NARC_scr_seq_dummy_scr_bin, NARC_msg_dummy_scr_dat );
		scr_id = 0;
	}

	return scr_id;
}
#endif

//----------------------------------------------------------------
/**
 * スクリプトデータセット
 * @param	fsys		FLDCOMMON_WORK型のポインタ
 * @param	core		VMHANDLE型のポインタ
 * @param	index		スクリプトデータのID
 * @param	dat_id		メッセージデータのID
 */
//----------------------------------------------------------------
#define SCRIPT_MSG_NON (0xffffffff)

static void SetScriptData( SCRCMD_WORK *work,
	VMHANDLE *core, int index, u32 dat_id, HEAPID heapID )
{
	//共通スクリプトデータロード
	VM_CODE *script = GFL_ARC_UTIL_Load( ARCID_SCRSEQ, index, 0, heapID );
	core->pScript = (VM_CODE*)script;
	
#ifndef SCRIPT_PL_NULL
	//メッセージデータマネージャー作成
	script->msgman = GFL_MSG_Create(
		GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, dat_id, heapID );
	core->msgman = MSGMAN_Create(
		MSGMAN_TYPE_DIRECT, ARC_MSG, dat_id, HEAPID_WORLD );
#else
	if( dat_id != SCRIPT_MSG_NON ){
		SCRCMD_WORK_CreateMsgData( work, dat_id );
	}
#endif
}

//----------------------------------------------------------------
/**
 * ゾーンスクリプトデータセット
 * @param	ID		スクリプトデータID
 */
//----------------------------------------------------------------
static void SetZoneScriptData( FLDCOMMON_WORK* fsys, VMHANDLE* core )
{
#ifndef SCRIPT_PL_NULL
	//ゾーンIDからロードするスクリプトバイナリを取得
	VM_CODE* script = LoadZoneScriptFile(fsys->location->zone_id);
	core->pScript	= (VM_CODE*)script;

	//メッセージデータマネージャー作成
	core->msgman = MSGMAN_Create( MSGMAN_TYPE_DIRECT, ARC_MSG, 
									LoadZoneMsgNo(fsys->location->zone_id), HEAPID_WORLD );
	
	return;
#else
	GF_ASSERT( 0 );
#endif
}


//======================================================================
//	スクリプト制御ワークのメンバーアクセス
//======================================================================
//--------------------------------------------------------------
/**
 * スクリプト制御ワークのメンバーアドレス取得
 * @param	sc		SCRIPT型のポインタ
 * @param	id		取得するメンバID(script.h参照)
 * @return	"アドレス"
 */
//--------------------------------------------------------------
void * SCRIPT_GetSubMemberWork( SCRIPT_WORK *sc, u32 id )
{
	EV_TRAINER_EYE_HITDATA *eye;
	
	switch( id ){
	//会話ウィンドウメッセージインデックスのポインタ
	case ID_EVSCR_MSGINDEX:
		return &sc->MsgIndex;
	//アニメーションの数のポインタ
	case ID_EVSCR_ANMCOUNT:
		return &sc->anm_count;
	//共通スクリプト切り替えフラグのポインタ
	case ID_EVSCR_COMMON_SCR_FLAG:
		return &sc->common_scr_flag;
	//会話ウィンドウを開いたかフラグのポインタ
	case ID_EVSCR_WIN_OPEN_FLAG:
		return &sc->win_open_flag;
	//追加した仮想マシンの数のポインタ
	case ID_EVSCR_VMHANDLE_COUNT:
		return &sc->vm_machine_count;
	//メインのスクリプトID
	case ID_EVSCR_SCRIPT_ID:
		return &sc->script_id;
	//イベント起動時の主人公の向き
	case ID_EVSCR_PLAYER_DIR:
		return &sc->player_dir;
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
	case ID_EVSCR_VM_MAIN:
		return &sc->vm[VMHANDLE_MAIN];
	//仮想マシン(サブ)のポインタ
	case ID_EVSCR_VM_SUB1:
		return &sc->vm[VMHANDLE_SUB1];
	//サブプロセスとのやりとりワークへのポインタ
	case ID_EVSCR_SUBPROC_WORK:
		return &sc->subproc_work;
	//ワークへの汎用ポインタ
	case ID_EVSCR_PWORK:
		return &sc->pWork;
	//戦闘結果フラグ
	case ID_EVSCR_WIN_FLAG:
		return &sc->win_flag;
	//ワーク(ANSWORK,TMPWORKなどの代わり)のポインタ
	//スクリプトとプログラムのデータ交換用
	case ID_EVSCR_WK_PARAM0:				
	case ID_EVSCR_WK_PARAM1:
	case ID_EVSCR_WK_PARAM2:
	case ID_EVSCR_WK_PARAM3:
	//スクリプトでのテンポラリ
	case ID_EVSCR_WK_TEMP0:					
	case ID_EVSCR_WK_TEMP1:
	case ID_EVSCR_WK_TEMP2:
	case ID_EVSCR_WK_TEMP3:
	//スクリプト内部での処理用
	case ID_EVSCR_WK_REG0:					
	case ID_EVSCR_WK_REG1:
	case ID_EVSCR_WK_REG2:
	case ID_EVSCR_WK_REG3:
	//スクリプトに答えを返す汎用ワーク
	case ID_EVSCR_WK_ANSWER:
	//話しかけ対象OBJID
	case ID_EVSCR_WK_TARGET_OBJID:
		return &sc->work[ id - ID_EVSCR_WK_START ];
	//GAMESYS_WORK wb
	case ID_EVSCR_WK_GAMESYS_WORK:
		return &sc->gsys;
	//HEAPID wb
	case ID_EVSCR_WK_HEAPID:
		return &sc->heapID;
	//SCRIPT_FLDPARAM wb
	case ID_EVSCR_WK_FLDPARAM:
		return &sc->fld_param;
  //GMEVENT   
  case ID_EVSCR_WK_GMEVENT:
    return &sc->gmevent;
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
	//視線(0)：視線距離
	case ID_EVSCR_TR0_RANGE:
		eye = &sc->eye_hitdata[0];
		return &eye->range;
	//視線(0)：方向
	case ID_EVSCR_TR0_DIR:
		eye = &sc->eye_hitdata[0];
		return &eye->dir;
		GF_ASSERT( 0 );
		break;
	//視線(0)：スクリプトID
	case ID_EVSCR_TR0_SCR_ID:
		eye = &sc->eye_hitdata[0];
		return &eye->scr_id;
	//視線(0)：トレーナーID
	case ID_EVSCR_TR0_ID:
		eye = &sc->eye_hitdata[0];
		return &eye->tr_id;
	//視線(0)：トレーナータイプ
	case ID_EVSCR_TR0_TYPE:
		eye = &sc->eye_hitdata[0];
		return &eye->tr_type;
	//視線(0)：トレーナーOBJ
	case ID_EVSCR_TR0_FLDOBJ:
		eye = &sc->eye_hitdata[0];
		return &eye->mmdl;
	//視線(0)：TCB
	case ID_EVSCR_TR0_TCB:
		eye = &sc->eye_hitdata[0];
		return &eye->ev_eye_move;
	//視線(1)：視線距離
	case ID_EVSCR_TR1_RANGE:
		eye = &sc->eye_hitdata[1];
		return &eye->range;
	//視線(1)：方向
	case ID_EVSCR_TR1_DIR:
		eye = &sc->eye_hitdata[1];
		return &eye->dir;
	//視線(1)：スクリプトID
	case ID_EVSCR_TR1_SCR_ID:
		eye = &sc->eye_hitdata[1];
		return &eye->scr_id;
	//視線(1)：トレーナーID
	case ID_EVSCR_TR1_ID:
		eye = &sc->eye_hitdata[1];
		return &eye->tr_id;
	//視線(1)：トレーナータイプ
	case ID_EVSCR_TR1_TYPE:
		eye = &sc->eye_hitdata[1];
		return &eye->tr_type;
	//視線(1)：トレーナーOBJ
	case ID_EVSCR_TR1_FLDOBJ:
		eye = &sc->eye_hitdata[1];
		return &eye->mmdl;
	//視線(1)：TCB
	case ID_EVSCR_TR1_TCB:
		eye = &sc->eye_hitdata[1];
		return &eye->ev_eye_move;

#ifndef SCRIPT_PL_NULL
	//イベントウィンドウワークのポインタ
	case ID_EVSCR_EVWIN:
		return &sc->ev_win;
	//会話ウィンドウビットマップデータのポインタ
	case ID_EVSCR_MSGWINDAT:
		return &sc->MsgWinDat;
	//待機アイコンのポインタ
	case ID_EVSCR_WAITICON:
		return &sc->waiticon;
	//フィールドエフェクトへのポインタ
	case ID_EVSCR_EOA:
		return &sc->eoa;
	//自機形態レポートTCBのポインタ
	case ID_EVSCR_PLAYER_TCB:
		return &sc->player_tcb;
	//コインウィンドウビットマップデータのポインタ
	case ID_EVSCR_COINWINDAT:
		return &sc->CoinWinDat;
	//お金ウィンドウビットマップデータのポインタ
	case ID_EVSCR_GOLDWINDAT:
		return &sc->GoldWinDat;
	//レポート情報表示ウィンドウ制御ワークへのポインタ
	case ID_EVSCR_REPORTWIN:
		return &sc->riw;
#endif
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
		GF_ASSERT( (0) &&
		"起動(確保)していないスクリプトのワークにアクセスしています！" );
	}
	
	return SCRIPT_GetSubMemberWork( sc, id );
}

//--------------------------------------------------------------
/**
 * スクリプト制御ワークの"next_func"にメニュー呼び出しをセット
 * @param	fsys	FLDCOMMON_WORK型のポインタ
 * @return	none
 * フィールドメニュー呼び出し限定！
 * 流れが把握しずらくなるので、汎用的に使えないようにしている！
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
void SCRIPT_SetNextScript( FLDCOMMON_WORK *fsys )
{
	SCRIPT_WORK *sc = FieldEvent_GetSpecialWork( fsys->event );
	
	if( FieldMenuCallCheck( fsys ) == TRUE ){	//「なぞのばしょ」チェック 2006/10/24 by nakahiro
		sc->next_func = FieldMenuEvChg;
	}
	return;
}
#endif

//--------------------------------------------------------------
/**
 * IDジャンプ
 * @param	core	VMHANDLE型のポインタ
 * @param	id		スクリプトID
 * @return	none
 */
//--------------------------------------------------------------
static void EventDataIDJump( VMHANDLE * core, u16 ev_id )
{
	core->adrs += (ev_id * 4);			//ID分進める(adrsがlongなので*4)
	core->adrs += VMGetU32( core );		//ラベルオフセット分進める
	return;
}

//--------------------------------------------------------------
/**
 *
 * @param	zone_id		ゾーンID
 * @retval	""
 */
//--------------------------------------------------------------
static void * LoadZoneScriptFile(int zone_id)
{
#ifndef SCRIPT_PL_NULL
	return ArchiveDataLoadMalloc(ARC_SCRIPT, ZoneData_GetScriptArchiveID(zone_id), HEAPID_WORLD);
#else
	return NULL;
#endif
}

#if 0	//pl..
void * LoadZoneMsgFile(int zone_id);
void * LoadZoneMsgFile(int zone_id)
{
	return ArchiveDataLoadMalloc(ARC_SCRIPT_MSG, ZoneData_GetMsgArchiveID(zone_id), HEAPID_WORLD);
}
#endif

//--------------------------------------------------------------
/**
 * マップ管理表に登録されている、メッセージファイルのアーカイブIDを取得
 *
 * @param	zone_id		ゾーンID
 *
 * @retval	"アーカイブID"
 */
//--------------------------------------------------------------
static u32 LoadZoneMsgNo(int zone_id)
{
#ifndef SCRIPT_PL_NULL
	return ZoneData_GetMsgArchiveID(zone_id);
#else
	return 0;
#endif
}

//======================================================================
//	イベント
//======================================================================
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
	VMHANDLE *core = NULL;
	EVENT_SCRIPT_WORK *ev_wk = wk;
	SCRIPT_WORK *sc = ev_wk->sc;
	
	switch( *seq ){
	case 0:
    //ワークセット
    sc->gmevent = event; //イベントセット時に初期化してるが念のため

		//仮想マシン追加
		sc->vm[VMHANDLE_MAIN] = SCRIPT_AddVMachine(
				sc->gsys, sc, sc->heapID, sc->script_id );
		sc->vm_machine_count = 1;
		
		//メッセージ関連
		sc->wordset = WORDSET_CreateEx(
			WORDSET_SCRIPT_SETNUM, WORDSET_SCRIPT_BUFLEN, sc->heapID );
		sc->msg_buf = GFL_STR_CreateBuffer( SCR_MSG_BUF_SIZE, sc->heapID );
		sc->tmp_buf = GFL_STR_CreateBuffer( SCR_MSG_BUF_SIZE, sc->heapID );
		
		(*seq)++;
	case 1:
		//仮想マシンコントロール
		for( i = 0; i < VMHANDLE_MAX; i++ ){
			core = sc->vm[i];
			if( core != NULL ){
				if( VM_Control(core) == FALSE ){	//制御メイン
					script_del( core );				//スクリプト削除
					
					if( sc->vm_machine_count == 0 ){
						GF_ASSERT( (0) && "仮想マシンの数が不正です！" );
					}
					
					sc->vm[i] = NULL;
					sc->vm_machine_count--;
				}
			}
		}

		//仮想マシンの数をチェック
		if( sc->vm_machine_count <= 0 ){
			SCRIPT_EVENTFUNC func = sc->next_func;		//退避
			
			WORDSET_Delete( sc->wordset );
			GFL_STR_DeleteBuffer( sc->msg_buf );
			GFL_STR_DeleteBuffer( sc->tmp_buf );

			//デバック処理
			//debug_script_flag = 0;
			
			sc->magic_no = 0;
			GFL_HEAP_FreeMemory( sc );		//スクリプトワーク
			
			//スクリプト終了時に呼び出される関数実行
			if( func != NULL ){
				func( event );
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
	MI_CpuClear8( ev_sc, sizeof(EVENT_SCRIPT_WORK) );
	
	ev_sc->sc = sc;
  ev_sc->sc->gmevent = event;
	return event;
}

//--------------------------------------------------------------
/**
 * スクリプトイベントコール
 * @param	sc	SCRIPT_WORK*
 * @param	
 * @retval
 */
//--------------------------------------------------------------
static void FldScript_CallControlEvent( SCRIPT_WORK *sc, GMEVENT *event )
{
	GMEVENT *sc_event;
	sc_event = FldScript_CreateControlEvent( sc );
	GMEVENT_CallEvent( event, sc_event );
}

//--------------------------------------------------------------
/**
 * スクリプトイベントチェンジ
 * @param	sc	SCRIPT_WORK*
 * @param	
 * @retval
 */
//--------------------------------------------------------------
static GMEVENT * FldScript_ChangeControlEvent( SCRIPT_WORK *sc, GMEVENT *event )
{
	EVENT_SCRIPT_WORK *ev_sc;
	
	GMEVENT_Change( event,
		FldScriptEvent_ControlScript, sizeof(EVENT_SCRIPT_WORK) );
	ev_sc = GMEVENT_GetEventWork( event );
	MI_CpuClear8( ev_sc, sizeof(EVENT_SCRIPT_WORK) );

	ev_sc->sc = sc;
	return event;
}

//======================================================================
//	イベントワーク
//======================================================================
//------------------------------------------------------------------
/**
 * @brief	イベントワークアドレスを取得
 * @param	ev			イベントワークへのポインタ
 * @param	fsys		FLDCOMMON_WORKへのポインタ
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
	return SCRIPT_GetMemberWork(
		sc, (ID_EVSCR_WK_START + work_no - SCWK_START) );
}

//------------------------------------------------------------------
/**
 * @brief	イベントワークの値を取得
 *
 * @param	fsys		FLDCOMMON_WORKへのポインタ
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
 * @param	fsys		FLDCOMMON_WORKへのポインタ
 * @param	work_no		ワークナンバー
 * @param	value		セットする値
 *
 * @return	"TRUE=セット出来た、FALSE=セット出来なかった"
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
BOOL SCRIPT_SetEventWorkValue(
	SCRIPT_WORK *sc, u16 work_no, u16 value )
{
	u16* res = GetEventWorkAdrs( fsys, work_no );
	if( res == NULL ){ return FALSE; }
	*res = value;
	return TRUE;
}
#endif

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

//======================================================================
//	フラグ関連
//======================================================================
//------------------------------------------------------------------
/**
 * @brief	イベントフラグをチェックする
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @param	flag_no		フラグナンバー
 * @retval	"1 = フラグON"
 * @retval	"0 = フラグOFF"
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
BOOL CheckEventFlag( FLDCOMMON_WORK* fsys, u16 flag_no)
{
	return EventWork_CheckEventFlag( SaveData_GetEventWork(fsys->savedata),	flag_no );
}
#endif

//------------------------------------------------------------------
/**
 * @brief	イベントフラグをセットする
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @param	flag_no		フラグナンバー
 * @return	none
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
void SetEventFlag( FLDCOMMON_WORK* fsys, u16 flag_no)
{
	EventWork_SetEventFlag( SaveData_GetEventWork(fsys->savedata),flag_no );
	return;
}
#endif

//------------------------------------------------------------------
/**
 * @brief	イベントフラグをリセットする
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @param	flag_no		フラグナンバー
 * @return	none
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
void ResetEventFlag( FLDCOMMON_WORK* fsys, u16 flag_no)
{
	EventWork_ResetEventFlag( SaveData_GetEventWork(fsys->savedata), flag_no );
	return;
}
#endif

//------------------------------------------------------------------
/**
 * @brief	マップ内限定のセーブフラグをクリアする
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @return	none
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
void LocalEventFlagClear( FLDCOMMON_WORK* fsys )
{
	int i;
	EVENTWORK* event;

#if 0
	//クリア前
	for( i=0; i < 80 ;i++ ){
		OS_Printf( "flag %d= %d\n", i, CheckEventFlag(fsys,i) );
	}
	for( i=0x4000; i < 0x4020 ;i++ ){
		OS_Printf( "work %d= %d\n", i, GetEventWorkValue(fsys,i) );
	}
#endif

	//イベントワークのポインタ取得
	event = SaveData_GetEventWork(fsys->savedata);

	//ローカルフラグのクリア"8bit * 8 = 64個分"
	//"0"は無効なナンバーなので1を渡して取得している
	memset( EventWork_GetEventFlagAdrs(event,1), 0, LOCAL_FLAG_AREA_MAX );

	//ローカルワークのクリア"32個分"
	memset( EventWork_GetEventWorkAdrs(event,LOCAL_WORK_START), 0, 2*LOCAL_WORK_MAX );

#if 0
	//クリア後
	for( i=0; i < 80 ;i++ ){
		OS_Printf( "flag %d= %d\n", i, CheckEventFlag(fsys,i) );
	}
	for( i=0x4000; i < 0x4020 ;i++ ){
		OS_Printf( "work %d= %d\n", i, GetEventWorkValue(fsys,i) );
	}
#endif

	return;
}
#endif

//------------------------------------------------------------------
/**
 * @brief	1日経過ごとにクリアされるフラグをクリアする
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @return	none
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
void TimeEventFlagClear( FLDCOMMON_WORK* fsys )
{
	EVENTWORK* event;

	//イベントワークのポインタ取得
	event = SaveData_GetEventWork(fsys->savedata);

	memset( EventWork_GetEventFlagAdrs(event,TIMEFLAG_START), 0, TIMEFLAG_AREA_MAX );

	return;
}
#endif

//======================================================================
//		プログラム→スクリプト汎用
//======================================================================
//--------------------------------------------------------------
/**
 * @brief	プログラムからスクリプトへパラメータを渡す
 * @param	event	スクリプトを起動しているイベントへのポインタ
 * @param	prm0	パラメータ０（SCWK_PARAM0）
 * @param	prm1	パラメータ１（SCWK_PARAM1）
 * @param	prm2	パラメータ２（SCWK_PARAM2）
 * @param	prm3	パラメータ３（SCWK_PARAM3）
 */
//--------------------------------------------------------------
void SCRIPT_SetParam(
	SCRIPT_WORK *sc, u16 prm0, u16 prm1, u16 prm2, u16 prm3 )
{
	*(u16*)SCRIPT_GetMemberWork( sc, ID_EVSCR_WK_PARAM0 ) = prm0;
	*(u16*)SCRIPT_GetMemberWork( sc, ID_EVSCR_WK_PARAM1 ) = prm1;
	*(u16*)SCRIPT_GetMemberWork( sc, ID_EVSCR_WK_PARAM2 ) = prm2;
	*(u16*)SCRIPT_GetMemberWork( sc, ID_EVSCR_WK_PARAM3 ) = prm3;
}

//======================================================================
//	トレーナーフラグ関連
//	・スクリプトIDから、トレーナーIDを取得して、フラグチェック
//	BOOL CheckEventFlagTrainer( fsys, GetTrainerIdByScriptId(scr_id) );
//======================================================================
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
#ifndef SCRIPT_PL_NULL
u16 GetTrainerIdByScriptId( u16 scr_id )
{
	if( scr_id < ID_TRAINER_2VS2_OFFSET ){
		return (scr_id - ID_TRAINER_OFFSET + 1);		//1オリジン
	}else{
		return (scr_id - ID_TRAINER_2VS2_OFFSET + 1);		//1オリジン
	}
}
#else
u16 SCRIPT_GetTrainerID_ByScriptID( u16 scr_id )
{
	if( scr_id < ID_TRAINER_2VS2_OFFSET ){
		return (scr_id - ID_TRAINER_OFFSET + 1);		//1オリジン
	}else{
		return (scr_id - ID_TRAINER_2VS2_OFFSET + 1);		//1オリジン
	}
}
#endif

//--------------------------------------------------------------
/**
 * スクリプトIDから、左右どちらのトレーナーか取得
 * @param   scr_id		スクリプトID
 * @retval  "0=左、1=右"
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
BOOL GetTrainerLRByScriptId( u16 scr_id )
{
	if( scr_id < ID_TRAINER_2VS2_OFFSET ){
		return 0;
	}else{
		return 1;
	}
}
#else
BOOL SCRIPT_GetTrainerLR_ByScriptID( u16 scr_id )
{
	if( scr_id < ID_TRAINER_2VS2_OFFSET ){
		return 0;
	}else{
		return 1;
	}
}
#endif

//--------------------------------------------------------------
/**
 * トレーナーIDから、ダブルバトルタイプか取得
 * @param   tr_id		トレーナーID
 * @retval  "0=シングルバトル、1=ダブルバトル"
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
BOOL CheckTrainer2vs2Type( u16 tr_id )
{
	if( TT_TrainerDataParaGet(tr_id, ID_TD_fight_type) == FIGHT_TYPE_1vs1 ){	//1vs1
		OS_Printf( "trainer_type = 1vs1 " );
		return 0;
		//return SCR_EYE_TR_TYPE_SINGLE;
	}

	OS_Printf( "trainer_type = 2vs2 " );
	return 1;
	//return SCR_EYE_TR_TYPE_DOUBLE;
}
#else
BOOL SCRIPT_CheckTrainer2vs2Type( u16 tr_id )
{
  #if 0 //wb 現状無効
	if( TT_TrainerDataParaGet(tr_id, ID_TD_fight_type) == FIGHT_TYPE_1vs1 ){	//1vs1
		OS_Printf( "trainer_type = 1vs1 " );
		return 0;
		//return SCR_EYE_TR_TYPE_SINGLE;
	}

	OS_Printf( "trainer_type = 2vs2 " );
	return 1;
	//return SCR_EYE_TR_TYPE_DOUBLE;
  #else
  return 0; //仮でシングル固定
  #endif
}
#endif

//------------------------------------------------------------------
/**
 * @brief	トレーナーイベントフラグをチェックする
 * @param	fsys		FLDCOMMON_WORKのポインタ
 * @param	tr_id		トレーナーID
 * @retval	"1 = フラグON"
 * @retval	"0 = フラグOFF"
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
BOOL CheckEventFlagTrainer( FLDCOMMON_WORK* fsys, u16 tr_id )
{
	return EventWork_CheckEventFlag( 
		SaveData_GetEventWork(fsys->savedata), GET_TRAINER_FLAG(tr_id) );
}
#else
BOOL SCRIPT_CheckEventFlagTrainer( EVENTWORK *ev, u16 tr_id )
{
  return EVENTWORK_CheckEventFlag(ev,GET_TRAINER_FLAG(tr_id) );
}
#endif

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
#ifndef SCRIPT_PL_NULL
void SetEventFlagTrainer( FLDCOMMON_WORK* fsys, u16 tr_id )
{
	EventWork_SetEventFlag( SaveData_GetEventWork(fsys->savedata), GET_TRAINER_FLAG(tr_id) );
	return;
}
#else
void SCRIPT_SetEventFlagTrainer( EVENTWORK *ev, u16 tr_id )
{
  EVENTWORK_SetEventFlag( ev, GET_TRAINER_FLAG(tr_id) );
}
#endif

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
#ifndef SCRIPT_PL_NULL
void ResetEventFlagTrainer( FLDCOMMON_WORK* fsys, u16 tr_id )
{
	EventWork_ResetEventFlag( SaveData_GetEventWork(fsys->savedata), GET_TRAINER_FLAG(tr_id) );
	return;
}
#else
void SCRIPT_ResetEventFlagTrainer( EVENTWORK *ev, u16 tr_id )
{
	EVENTWORK_ResetEventFlag( ev, GET_TRAINER_FLAG(tr_id) );
}
#endif

//======================================================================
//	隠しアイテム関連
//	・スクリプトIDから、隠しアイテムフラグを取得して、フラグチェック
//	BOOL CheckEventFlag( fsys, GetHideItemFlagNoByScriptId(scr_id) );
//======================================================================
//--------------------------------------------------------------
/**
 * スクリプトIDから、隠しアイテムフラグナンバーを取得
 * @param   scr_id		スクリプトID
 * @retval  "フラグナンバー"
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
u16 GetHideItemFlagNoByScriptId( u16 scr_id )
{
	return (scr_id - ID_HIDE_ITEM_OFFSET + FH_FLAG_START);
}
#else //wb
u16 SCRIPT_GetHideItemFlagNoByScriptID( u16 scr_id )
{
	return (scr_id - ID_HIDE_ITEM_OFFSET + FH_FLAG_START);
}
#endif

//--------------------------------------------------------------
/**
 * スクリプトIDから、隠しアイテムフラグインデックスを取得
 * @param   scr_id		スクリプトID
 * @retval  "フラグインデックス"
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
u16 GetHideItemFlagIndexByScriptId( u16 scr_id )
{
	return (scr_id - ID_HIDE_ITEM_OFFSET);
}
#endif

//--------------------------------------------------------------
/**
 * 0時で隠しフラグが復活する
 * @param   fsys	FLDCOMMON_WORK型のポインタ
 * @retval  none
 * 殿堂入り後のチェックが抜けていましたが、
 * 現状の形でOKだそうです。(08.06.25)
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
static u16 oneday_hide_item1[][2] = {		//鋼鉄島
	{ ZONE_ID_D24R0103, 52 },
	{ ZONE_ID_D24R0104, 53 },
	{ ZONE_ID_D24R0105, 54 },
	{ ZONE_ID_D24R0105, 55 },
};

#define ONEDAY_HIDE_ITEM1_MAX	( NELEMS(oneday_hide_item1) )

static u16 oneday_hide_item2[] = {			//ソノオの花園
	58,59,219,220,221,222,
};

#define ONEDAY_HIDE_ITEM2_MAX	( NELEMS(oneday_hide_item2) )
#endif

#ifndef SCRIPT_PL_NULL
void HideItemFlagOneDayClear( FLDCOMMON_WORK* fsys )
{
	u8 index;

	//鋼鉄島で2箇所の隠しアイテムが復活することがある
	index = ( gf_rand() % ONEDAY_HIDE_ITEM1_MAX );
	OS_Printf( "0_0 index = %d\n", index );
	if( fsys->location->zone_id != oneday_hide_item1[index][0] ){
		ResetEventFlag( fsys, (FH_FLAG_START + oneday_hide_item1[index][1]) );
	}

	index = ( gf_rand() % ONEDAY_HIDE_ITEM1_MAX );
	OS_Printf( "0_1 index = %d\n", index );
	if( fsys->location->zone_id != oneday_hide_item1[index][0] ){
		ResetEventFlag( fsys, (FH_FLAG_START + oneday_hide_item1[index][1]) );
	}

	//ソノオの花園で2箇所の隠しアイテムが復活することがある
	if( fsys->location->zone_id != ZONE_ID_D13R0101 ){
		index = ( gf_rand() % ONEDAY_HIDE_ITEM2_MAX );
		OS_Printf( "1_0 index = %d\n", index );
		ResetEventFlag( fsys, (FH_FLAG_START + oneday_hide_item2[index]) );

		index = ( gf_rand() % ONEDAY_HIDE_ITEM2_MAX );
		OS_Printf( "1_1 index = %d\n", index );
		ResetEventFlag( fsys, (FH_FLAG_START + oneday_hide_item2[index]) );
	}
	return;
}
#endif

//--------------------------------------------------------------
/**
 * スクリプトIDから、隠しアイテムの反応を取得
 * @param   scr_id		スクリプトID
 * @retval  "反応"
 * 隠しアイテムのスクリプトを見つけたら呼ぶようにする！
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
u8 GetHideItemResponseByScriptId( u16 scr_id )
{
	int i;
	u16	index;
	const HIDE_ITEM_DATA* data;

	data	= &hide_item_data[0];
	index	= GetHideItemFlagIndexByScriptId(scr_id);			//フラグインデックス取得

	//サーチする
	for( i=0; i < HIDE_ITEM_DATA_MAX ;i++ ){
		if( data[i].index == index ){
			break;
		}
	}

	//見つからなかった
	if( i >= HIDE_ITEM_DATA_MAX ){
		GF_ASSERT( (0) && "隠しアイテムデータに該当するデータがありません！" );
		return 0;
	}

	return data[i].response;
}
#endif

//--------------------------------------------------------------
/**
 * 隠しアイテムパラメータをワークにセット
 *
 * @param   sc			SCRIPT型のポインタ
 * @param   scr_id		スクリプトID
 *
 * @retval  "0=セット失敗、1=セット成功"
 *
 * 使用している！
 * SCWK_PARAM0
 * SCWK_PARAM1
 * SCWK_PARAM2
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
static BOOL HideItemParamSet( SCRIPT_WORK* sc, u16 scr_id )
{
	int i;
	u16	index;
	const HIDE_ITEM_DATA* data;
	u16* param0 = SCRIPT_GetSubMemberWork( sc, ID_EVSCR_WK_PARAM0 );
	u16* param1 = SCRIPT_GetSubMemberWork( sc, ID_EVSCR_WK_PARAM1 );
	u16* param2 = SCRIPT_GetSubMemberWork( sc, ID_EVSCR_WK_PARAM2 );

	data	= &hide_item_data[0];
	index	= GetHideItemFlagIndexByScriptId(scr_id);		//フラグインデックス取得

	//サーチする
	for( i=0; i < HIDE_ITEM_DATA_MAX ;i++ ){
		if( data[i].index == index ){
			break;
		}
	}

	//見つからなかった
	if( i >= HIDE_ITEM_DATA_MAX ){
		GF_ASSERT( (0) && "隠しアイテムデータに該当するデータがありません！" );
		return 0;
	}

	*param0 = data[i].itemno;						//アイテムナンバー
	*param1 = data[i].num;							//個数
	*param2 = GetHideItemFlagNoByScriptId(scr_id);	//フラグナンバー

	return 1;
}
#endif

#define DEBUG_HIDE_ITEM_LIST	//デバック有効
//--------------------------------------------------------------
/**
 * 画面内にある隠しアイテムを検索して確保したリストに登録
 *
 * @param   fsys		FLDCOMMON_WORK型のポインタ
 * @param   heapid		ヒープID
 *
 * @retval  "リストのアドレス"
 *
 * 解放処理を忘れずに！
 */
//--------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
HIDE_ITEM_LIST * HideItem_CreateList( FLDCOMMON_WORK * fsys, int heapid )
{
	HIDE_ITEM_LIST* list;
	const BG_TALK_DATA* bg;
	int	hero_gx,hero_gz,i,max,count,tmp;
	int l,r,u,d;

	count = 0;

	//BGデータ数取得
	max = EventData_GetNowBgTalkDataSize( fsys );
	max++;		//終了コードを入れるので+1

	//メモリ確保
	list = sys_AllocMemory( heapid, sizeof(HIDE_ITEM_LIST) * max );

	//BGデータが存在していなかった時
	if( max == 1 ){
		//終了コードセット
		list[0].response= HIDE_LIST_RESPONSE_NONE;
		list[0].gx		= 0xffff;
		list[0].gz		= 0xffff;
		return list;
	}

	//BGデータ取得
	bg	= EventData_GetNowBgTalkData( fsys );
	if( bg == NULL ){
		//終了コードセット
		list[0].response= HIDE_LIST_RESPONSE_NONE;
		list[0].gx		= 0xffff;
		list[0].gz		= 0xffff;
		return list;
	}

	//主人公の位置取得
	hero_gx = Player_NowGPosXGet( fsys->player );
	hero_gz = Player_NowGPosZGet( fsys->player );

	//検索範囲をセット(3Dは意識せずの簡易版)
	l = hero_gx - HIDE_LIST_SX;
	r = hero_gx + HIDE_LIST_SX;
	u = hero_gz - HIDE_LIST_TOP;
	d = hero_gz + HIDE_LIST_BOTTOM;

	//補正
	if( l < 0 ){ l = 0; }
	//if( r < 0 ){ 0 };
	if( u < 0 ){ u = 0; }
	//if( d < 0 ){ 0 };

#ifdef DEBUG_HIDE_ITEM_LIST
	OS_Printf( "\n＜検索範囲＞\n" );
	OS_Printf( "l = %d\n", l );
	OS_Printf( "r = %d\n", r );
	OS_Printf( "u = %d\n", u );
	OS_Printf( "d = %d\n", d );
#endif

	//BGデータ分サーチをかける
	for( i=0; i < max ;i++ ){

		//隠しアイテムタイプで、まだ入手していなかったら
		if( (bg[i].type == BG_TALK_TYPE_HIDE) &&
			(CheckEventFlag(fsys, GetHideItemFlagNoByScriptId(bg[i].id)) == 0) ){

			//検索範囲内にあるかチェック
			if( (bg[i].gx >= l) &&
				(bg[i].gx <= r) &&
				(bg[i].gz >= u) &&
				(bg[i].gz <= d) ){

				//スクリプトIDから、隠しアイテムの反応を取得
				list[count].response= GetHideItemResponseByScriptId( bg[i].id );

#if 0
				//見つけた座標をそのまま代入
				list[count].gx		= bg[i].gx;
				list[count].gz		= bg[i].gz;

				//ローカル座標
				list[count].gx		= (bg[i].gx % 32);
				list[count].gz		= (bg[i].gz % 32);
#endif	

				//左上を0,0として取得した座標
				//(主人公の位置からの差分を求める)
				tmp = ( hero_gx - bg[i].gx );
				list[count].gx = abs(HIDE_LIST_SX - tmp);
				tmp = ( hero_gz - bg[i].gz );
				list[count].gz = abs(HIDE_LIST_TOP - tmp);

#ifdef DEBUG_HIDE_ITEM_LIST
				OS_Printf( "\n＜主人公の位置＞\n" );
				OS_Printf( "hero_gx = %d\n", hero_gx );
				OS_Printf( "hero_gz = %d\n", hero_gz );

				OS_Printf( "\n＜見つけた隠しアイテムの位置＞\n" );
				OS_Printf( "bg[i].gx = %d\n", bg[i].gx );
				OS_Printf( "bg[i].gz = %d\n", bg[i].gz );

				OS_Printf( "\n＜左上を0,0として取得した座標＞\n" );
				OS_Printf( "list[count].gx = %d\n", list[count].gx );
				OS_Printf( "list[count].gz = %d\n", list[count].gz );
#endif

				count++;
			}
		}
	}

	//終了コードセット
	list[count].response= HIDE_LIST_RESPONSE_NONE;
	list[count].gx		= 0xffff;
	list[count].gz		= 0xffff;

	return list;
}
#endif


//======================================================================
//	特殊スクリプト関連
//======================================================================
//------------------------------------------------------------------
/**
 * @brief	ゲーム開始 スクリプト初期設定の実行
 * @param	fsys		FLDCOMMON_WORK型のポインタ
 * @return	none
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
void GameStartScriptInit( FLDCOMMON_WORK* fsys )
{
	SpScriptStart( fsys, SCRID_INIT_SCRIPT );

#ifdef PM_DEBUG
	//「デバックかいし」を選択していたら
	if( DebugFlagData.debug_mode == TRUE ){

		//ポケセン地下ストッパー削除、ともだち手帳イベント無効
		SpScriptStart( fsys, SCRID_DEBUG_PC_UG );
	}
#endif
	return;
}
#else //wb
void SCRIPT_CallGameStartInitScript( GAMESYS_WORK *gsys, HEAPID heapID )
{
  SCRIPT_WORK *sc = EvScriptWork_Alloc( heapID );
//  sc->fld_param = NULL;
  EvScriptWork_Init( sc, gsys, SCRID_INIT_SCRIPT, NULL, NULL );
  
  {
    VMHANDLE *core = NULL;
    core = SCRIPT_AddVMachine( sc->gsys, sc, sc->heapID, sc->script_id );
    while( VM_Control(core) == TRUE ){};
    script_del( core );
  }
}
#endif

//------------------------------------------------------------------
/**
 * @brief	EVENTではなく特殊スクリプト実行
 * @param	fsys		FLDCOMMON_WORK型のポインタ
 * @param	scr_id		スクリプトID
 * @return	none
 *
 * 注意！
 * SCRIPTを確保していないので、
 * SCWK_ANSWERなどのワークは使用することが出来ない！
 * LOCALWORK0などを使用するようにする！
 *
 * 共通スクリプトについては現状使用不可！
 * 対応予定だが、ゾーンをまたいだ時に処理落ちするかも？
 * あまりつかわないかも？
 *
 * フラグチェンジラベルで共通スクリプトを使いたいときは、
 * 二つフラグチェンジラベルを書くことになるかも？
 * 片方は、ゾーンでことたりるもの、片方は、共通スクリプトのIDを指定？。。。
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
void SpScriptStart( FLDCOMMON_WORK* fsys, u16 scr_id )
{
	VMHANDLE*core = VMMachineAdd( fsys, scr_id );
	while( VM_Control( core ) == TRUE );
	script_del(core);	//スクリプト削除
	return;
}
#endif

//------------------------------------------------------------------
/**
 * @brief	特殊スクリプト検索して実行
 * @param	fsys		FLDCOMMON_WORK型のポインタ
 * @param	key			特殊スクリプトID
 * @return	"TRUE=特殊スクリプト実行、FALSE=何もしない"
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
BOOL SpScriptSearch( FLDCOMMON_WORK* fsys, u8 key )
{
	u16 scr_id;
	const u8 * p;

	p = EventData_GetSpecialScript(fsys);
	if (p == NULL) {
		return FALSE;
	}
	//特殊スクリプト検索
	if( key == SP_SCRID_SCENE_CHANGE ){
		scr_id = SpScriptSearch_Sub2( fsys, p, key );	//条件チェック有り
	}else{
		scr_id = SpScriptSearch_Sub( p, key );			//条件チェック無し
	}

	//OS_Printf( "scr_id = %d\n", scr_id );
	if( scr_id == 0xffff ){
		return FALSE;
	}

	//特殊スクリプト実行
	if( key == SP_SCRID_SCENE_CHANGE ){
		EventSet_Script( fsys, scr_id, NULL );
	}else{
		SpScriptStart( fsys, scr_id );
	}
	return TRUE;
}
#endif

//------------------------------------------------------------------
/**
 * @brief	ヘッダーデータから指定タイプのものを検索
 * @param	key			特殊スクリプトID
 * @return	"スクリプトID"
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
static u16 SpScriptSearch_Sub(const u8 * p, u8 key )
{
	while( 1 ){
		//終了記述チェック(SP_EVENT_DATA_END)
		if( *p == SP_SCRID_NONE ){
			return 0xffff;
		}

		//指定タイプの特殊スクリプト検索
		if( *p == key ){
			p++;								//特殊スクリプトID分足す
			return ( *p + ( *(p+1)<<8 ) );		//スクリプトIDを返す
		}

		p += (1 + 2 + 2);						//特殊スクリプトID+スクリプトID+ダミー分足す
	}
	return 0xffff;								//何も見つからなかった
}
#endif

//------------------------------------------------------------------
/**
 * @brief	ヘッダーデータから指定タイプのものを条件検索(シーンイベント)
 * @param	key			特殊スクリプトID
 * @return	"スクリプトID"
 * 08/04/30
 * LOCALWORK0をシーンチェンジで指定しても特殊スクリプトが動かない。
 * 原因を調べてみたら、SP_SCRIPT_END(=0)をチェックしている処理が、
 * ワークをu8で処理しているので、
 * LOCALWORK0(=0x4000)なので1byteを見ると、
 * 0のため、SP_SCRIPT_ENDとして処理されてしまっていた。
 */
//------------------------------------------------------------------
#ifndef SCRIPT_PL_NULL
static u16 SpScriptSearch_Sub2( FLDCOMMON_WORK* fsys, const u8 * p, u8 key )
{
	u16 work1,work2;
	u32 pos;

	pos = 0;

	while( 1 ){
		//終了記述チェック(SP_EVENT_DATA_END)
		if( *p == SP_SCRID_NONE ){
			return 0xffff;
		}

		//指定タイプの特殊スクリプト検索
		if( (*p) == key ){
			p++;			//特殊スクリプトID分足す
			pos = ( *p + ( *(p+1)<<8 ) + ( *(p+2)<<16 ) + ( *(p+3)<<24 ) );
			p += 4;			//アドレス分足す
			break;
		}

		p += (1 + 4);		//特殊スクリプトID+アドレス分足す
	}

	//見つからなかった時
	if( pos == 0 ){
		return 0xffff;
	}

	//特殊スクリプトテーブルをセット
	p = (p + pos);

	while ( TRUE ) {
		//終了記述チェック(SP_SCRIPT_END)
		if( *p == 0 ){
			return 0xffff;
		}

		//比較すワーク取得
		work1 = ( *p + ( *(p+1)<<8 ) );
		if( work1 == 0 ){
			return 0xffff;
		};
		p += 2;									//ワーク分足す
		//OS_Printf( "work1 = %d\n", work1 );

		//比較す値(ワーク可)取得
		work2 = ( *p + ( *(p+1)<<8 ) );
		p += 2;									//比較する値分足す(ワーク可)
		//OS_Printf( "work2 = %d\n", work2 );

		//条件チェック
		if( GetEventWorkValue(fsys,work1) == GetEventWorkValue(fsys,work2) ){
			return ( *p + ( *(p+1)<<8 ) );
		}

		p += 2;									//スクリプトID分足す
	}
	return 0xffff;								//何も見つからなかった
}
#endif


//======================================================================
//
//	未使用
//
//======================================================================
#if 0
static void FieldScriptLoad( u16 map, u16 id );
static void FieldScriptFileNameMake( char * path, u16 map, u16 id );

static VMHANDLE field_script;			// 仮想マシン用制御構造体
static u8 field_script_status;			// スクリプト状態変数
//static u8 ForceEventFlag;				// 強制イベント制御フラグ
static VM_CODE * pScript = NULL;		// 通常スクリプト
//static u16 * ScriptWork = NULL;		// スクリプトワーク

//	スクリプト状態定義
enum{
	SCRIPT_ON,
	SCRIPT_WAIT,
	SCRIPT_OFF,
};

static void FieldScriptFileNameMake( char * path, u16 map, u16 id )
{
	strcpy( path, "/data/script/script_map" );

	StrNumSet( &path[23], map, 3 );		// "/data/script/script_map"
	strcat( path, "_" );
	StrNumSet( &path[27], id, 2 );		// "/data/script/script_map000_"
	strcat( path, ".dat" );
}

static void FieldScriptLoad( u16 map, u16 id )
{
	char * path;

	path = (char *)sys_AllocMemory( APP_W_HEAP_AREA, 48 );
	FieldScriptFileNameMake( path, map, id );
	pScript = (VM_CODE *)sys_LoadFile( APP_W_HEAP_AREA, path );
	sys_FreeMemory( APP_W_HEAP_AREA, path );
}

//--------------------------------------------------------------
/**
 * スクリプト起動チェック
 *
 * @param	none
 *
 * @retval	"TRUE = 起動している"
 * @retval	"FALSE = 起動していない"
 */
//--------------------------------------------------------------
u8 CheckScriptStatus(void)
{
	if( field_script_status == SCRIPT_ON ){
		return TRUE;
	}
	return FALSE;
}

#endif


//======================================================================
//
//	VMMachineのTCBを追加
//
//======================================================================

#if 0
//--------------------------------------------------------------
/**
 * @brief	仮想マシン追加
 *
 * @param	fsys		FLDCOMMON_WORK型のポインタ
 * @param	id			スクリプトID
 * @param	start		命令テーブル開始アドレス
 * @param	end			命令テーブル終了アドレス
 *
 * @retval	none
 */
//--------------------------------------------------------------
void VMMachineAddTCB(FLDCOMMON_WORK* fsys, u16 id, const VM_CMD* start, const VM_CMD* end)
{
	SCRIPT_WORK * sc;
	VMHANDLE* core = NULL;

	sc = FieldEvent_GetSpecialWork( fsys->event );

	//core = sys_AllocMemory( HEAPID_FIELD, sizeof(VMHANDLE) );
	core = sys_AllocMemory( HEAPID_WORLD, sizeof(VMHANDLE) );
	if( core == NULL ){
		GF_ASSERT( (0) && "メモリ確保に失敗しました！" );
	}

	//仮想マシン初期化
	//VM_Init( core, &ScriptCmdTbl[0], &ScriptCmdTbl[EVCMD_MAX] );
	VM_Init( core, start, end );

	InitScript( fsys, core, id, 0 );							//ローカルスクリプト初期化
	
	TCB_Add( VMMachineControlTCB, core, 0 );
	sc->vm_machine_count++;
	return;
}

//--------------------------------------------------------------
/**
 * @brief	TCBメイン
 *
 * @param	tcb		TCB_PTR
 * @param	wk		ワークのアドレス
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void VMMachineControlTCB( TCB_PTR tcb, void* wk )
{
	VMHANDLE *core;
	FLDCOMMON_WORK* fsys;
	SCRIPT_WORK * sc;

	core = (VMHANDLE *)wk;
	fsys = FieldEvent_GetFieldSysWork( core->event_work );
	sc = FieldEvent_GetSpecialWork( fsys->event );

	if( VM_Control( core ) == FALSE ){

		MSGMAN_Delete( core->msgman );

		sys_FreeMemoryEz( core->pScript );			//スクリプトデータ

		TCB_Delete( tcb );
		sys_FreeMemoryEz( core );

		if( sc->vm_machine_count == 0 ){
			GF_ASSERT( (0) && "仮想マシンの数が不正です！" );
		}

		sc->vm_machine_count--;
	};

	return;
}
#endif

#if 0
//--------------------------------------------------------------
/**
 * @brief	スクリプト制御ワークのVMポインタテーブルの空きを検索
 *
 * @param	event		GMEVENT_CONTROL型
 *
 * @retval	"0xff = 空きなし、それ以外は管理ID"
 */
//--------------------------------------------------------------
u8 EvScriptWork_VMEmptySearch(GMEVENT_CONTROL * event);
u8 EvScriptWork_VMEmptySearch(GMEVENT_CONTROL * event)
{
	VMHANDLE* core = NULL;
	SCRIPT_WORK * sc = FieldEvent_GetSpecialWork( event );

	for( i=0; i < VMHANDLE_MAX; i++ ){

		core = sc->vm[i];

		if( core == NULL ){
			return i;
		}
	}

	return 0xff;
}
#endif

