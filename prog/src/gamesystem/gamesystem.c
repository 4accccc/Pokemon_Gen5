//============================================================================================
/**
 * @file	gamemsystem.c
 * @brief	ポケモンゲームシステム（ここが通常ゲームのトップレベル）
 * @author	tamada GAME FREAK Inc.
 * @date	08.10.20
 *
 */
//============================================================================================
#include <gflib.h>

#include "system/main.h"		//HEAPIDの参照用

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_event.h"
#include "gamesystem/iss_sys.h"
#include "gamesystem/game_beacon.h"

#include "gamesystem/game_init.h"

#include "src/field/event_mapchange.h"
#include "src/field/field_sound.h"
#include "system/net_err.h"

#include "field/zonedata.h"




//============================================================================================
//============================================================================================
enum {
  /**
   * @brief 永続情報用HEAP領域
   */
  HEAPSIZE_WORLD = 0x10000,

  /**
   * @brief イベント用HEAP領域
   *
   * 個別のイベントごとに確保し、イベント終了で解放されるメモリは
   * このHEAPから確保する
   */
	HEAPSIZE_PROC = 0x10000,

  /**
   * @brief イベントコントローラー用HEAP領域
   * 常にではないが、イベントとイベントをまたぐような形で
   * 存在するメモリ（ユニオンルームやバトルタワーのコントロール）は
   * このHEAPから取得する
   */
  HEAPSIZE_APP_CONTROL  =      (0x1000),

};

//------------------------------------------------------------------
/**
 * @brief	ゲーム制御システム用ワーク保持変数
 *
 * ※たぶんデバッグ用か限定時のグローバル参照用にしか使わないはず
 */
//------------------------------------------------------------------
static GAMESYS_WORK * GameSysWork;

//------------------------------------------------------------------
/**
 * @brief	ゲーム初期化パラメータ保持変数
 *
 */
//------------------------------------------------------------------
static GAME_INIT_WORK TestGameInitWork;

//============================================================================================
//============================================================================================

static void GameSystem_Init(GAMESYS_WORK * gsys, HEAPID heapID, GAME_INIT_WORK * init_param);
static BOOL GameSystem_Main(GAMESYS_WORK * gsys);
static void GameSystem_End(GAMESYS_WORK * gsys);
static u32 GAMESYS_WORK_GetSize(void);


//============================================================================================
/**
 * @brief	ゲームシステム用プロセス
 *
 * NitroMain->GFLUser_Main->GFL_PROC_Main()に登録される。
 * 基本的にはゲーム本編開始後はこのプロセスが動作し続けるはず
 */
//============================================================================================
static GFL_PROC_RESULT GameMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT GameMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk);
static GFL_PROC_RESULT GameMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk);

//------------------------------------------------------------------
//------------------------------------------------------------------
static GFL_PROC_RESULT GameMainProcInit(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GAMESYS_WORK * gsys;
	u32 work_size = GAMESYS_WORK_GetSize();
	GAME_INIT_WORK *game_init = pwk;
	
  //ゲーム中、永続的に保持する必要のあるデータはこのHEAPで
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WORLD, HEAPSIZE_WORLD );
  //ゲームイベント中に生成するべきメモリ（イベント終了で解放）の場合はこのHEAPで
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PROC, HEAPSIZE_PROC );
	//パレスしながら戦闘もフィールドもするのでここで確保
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_APP_CONTROL, HEAPSIZE_APP_CONTROL );
	
	gsys = GFL_PROC_AllocWork(proc, work_size, HEAPID_WORLD);
	GFL_STD_MemClear(gsys, work_size);
	GameSysWork = gsys;
	GameSystem_Init(gsys, HEAPID_WORLD, game_init);

  { //ゲーム開始イベントの生成
    GMEVENT * event = EVENT_GameStart(gsys, game_init);
    GAMESYSTEM_SetEvent(gsys, event);
  }

	return GFL_PROC_RES_FINISH;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static GFL_PROC_RESULT GameMainProcMain(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GAMESYS_WORK * gsys = mywk;

	if (GameSystem_Main(gsys)) {
		return GFL_PROC_RES_FINISH;
	} else {
		return GFL_PROC_RES_CONTINUE;
	}
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static GFL_PROC_RESULT GameMainProcEnd(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GAMESYS_WORK * gsys = mywk;
	GameSystem_End(gsys);
	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap( HEAPID_WORLD );
  GFL_HEAP_DeleteHeap( HEAPID_PROC );
  GFL_HEAP_DeleteHeap( HEAPID_APP_CONTROL );
  OS_ResetSystem( 0 );
	return GFL_PROC_RES_FINISH;
}
//------------------------------------------------------------------
/**
 * @brief		プロセス関数テーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA GameMainProcData = {
	GameMainProcInit,
	GameMainProcMain,
	GameMainProcEnd,
};


//------------------------------------------------------------------
/**
 * @file	ゲーム開始初期化用ワーク取得
 */
//------------------------------------------------------------------
GAME_INIT_WORK * DEBUG_GetGameInitWork(GAMEINIT_MODE mode, u16 mapid, VecFx32 *pos, s16 dir )
{
  GFL_STD_MemClear(&TestGameInitWork, sizeof(GAME_INIT_WORK));
	TestGameInitWork.mode = mode;
	TestGameInitWork.mapid = mapid;
	TestGameInitWork.pos = *pos;
	TestGameInitWork.dir = dir;
	return &TestGameInitWork;
}


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ゲーム制御システム用ワーク定義
 */
//------------------------------------------------------------------
struct _GAMESYS_WORK {
	HEAPID heapID;			///<使用するヒープ指定ID
	GAME_INIT_WORK * init_param;		///<親（呼び出し元）から引き継いだワークへのポインタ

	GFL_PROCSYS * procsys;	///<使用しているPROCシステムへのポインタ

	GFL_PROC_MAIN_STATUS proc_result;

	EVCHECK_FUNC evcheck_func;
	void * evcheck_context;
	GMEVENT * event;

	GAMEDATA * gamedata;
	FIELDMAP_WORK * fieldmap;
	GAME_COMM_SYS_PTR game_comm;    ///<ゲーム通信管理ワークへのポインタ
	void * comm_infowin;		///<INFOWIN通信ワーク

	ISS_SYS * iss_sys;		// ISSシステム
	
	u8 field_comm_error_req;      ///<TRUE:フィールドでの通信エラー画面表示リクエスト
	u8 padding[3];
};

//------------------------------------------------------------------
//------------------------------------------------------------------
static u32 GAMESYS_WORK_GetSize(void)
{
	return sizeof(GAMESYS_WORK);
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void GAMESYS_WORK_Init(GAMESYS_WORK * gsys, HEAPID heapID, GAME_INIT_WORK * init_param)
{
	gsys->heapID = heapID;
	gsys->init_param = init_param;
	gsys->procsys = GFL_PROC_LOCAL_boot(gsys->heapID);
	gsys->proc_result = GFL_PROC_MAIN_NULL;
	gsys->evcheck_func = NULL;
	gsys->evcheck_context = NULL;
	gsys->event = NULL;

  ZONEDATA_Open( heapID ); // ゾーンデータのアーカイブハンドルを開く

	gsys->gamedata = GAMEDATA_Create(gsys->heapID);
	gsys->fieldmap = NULL;
	gsys->game_comm = GameCommSys_Alloc(gsys->heapID, gsys->gamedata);
	gsys->comm_infowin = NULL;
	gsys->iss_sys = ISS_SYS_Create( gsys->gamedata, heapID );

}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void GAMESYS_WORK_Delete(GAMESYS_WORK * gsys)
{
	ISS_SYS_Delete(gsys->iss_sys);
	GAMEDATA_Delete(gsys->gamedata);
	GameCommSys_Free(gsys->game_comm);
  ZONEDATA_Close();        // ゾーンデータのアーカイブハンドルを閉じる
}


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void GameSystem_Init(GAMESYS_WORK * gsys, HEAPID heapID, GAME_INIT_WORK * init_param)
{
	GAMESYS_WORK_Init(gsys, heapID, init_param);
	GAMEBEACON_Setting(gsys->gamedata);
}

//------------------------------------------------------------------
/**
 * @return BOOL	TRUEのとき、ゲーム終了
 */
//------------------------------------------------------------------
static BOOL GameSystem_Main(GAMESYS_WORK * gsys)
{
	//Game Server Proccess

	if(GAMEDATA_IsFrameSpritMode(gsys->gamedata)) //フレーム分割状態にいる場合
	{
		//ここから３０フレーム用のキーを返すようにします
		GFL_UI_ChangeFrameRate(GFL_UI_FRAMERATE_30);
	}

	// このフレームが０以外の場合、処理分割が行われている
	// 処理分割時には処理分割の最初のフレームだけ、動く必要がある
	// そうでない場合、継続のフラグだけ返す
	if(!GAMEDATA_GetAndAddFrameSpritCount(gsys->gamedata))
	{
		//PlayerController/Event Trigger
		//イベント起動チェック処理（シチュエーションにより分岐）
		GAMESYSTEM_EVENT_CheckSet(gsys, gsys->evcheck_func, gsys->evcheck_context);
		//イベント実行処理
		GAMESYSTEM_EVENT_Main(gsys);
		//通信イベント実行処理
		GameCommSys_Main(gsys->game_comm);
	}
	//メインプロセス
	gsys->proc_result = GFL_PROC_LOCAL_Main(gsys->procsys);

  // FIELD_SOUNDメイン
  {
    FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gsys->gamedata );
    FIELD_SOUND_UpdateBGM( fsnd );
  }
	// ISSシステムメイン
	ISS_SYS_Update( gsys->iss_sys );

	if(GAMEDATA_IsFrameSpritMode(gsys->gamedata)) //フレーム分割状態にいる場合
	{
		//ここから６０フレーム用のキーを返すようにします
		GFL_UI_ChangeFrameRate(GFL_UI_FRAMERATE_60);
	}

  //通信エラー画面呼び出しチェック
  if(gsys->proc_result == GFL_PROC_MAIN_CHANGE || gsys->proc_result == GFL_PROC_MAIN_NULL){
    NetErr_DispCall();
  }
  
	if (gsys->proc_result == GFL_PROC_MAIN_NULL && gsys->event == NULL)
	{
		//プロセスもイベントも存在しないとき、ゲーム終了
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void GameSystem_End(GAMESYS_WORK * gsys)
{
	GFL_PROC_LOCAL_Exit(gsys->procsys);
	GAMESYS_WORK_Delete(gsys);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMESYSTEM_EVENT_EntryCheckFunc(GAMESYS_WORK * gsys, EVCHECK_FUNC evcheck_func, void * context)
{
	gsys->evcheck_func = evcheck_func;
	gsys->evcheck_context = context;
}

//============================================================================================
//
//		外部インターフェイス関数
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMESYSTEM_SetNextProc(GAMESYS_WORK * gsys,
		FSOverlayID ov_id, const GFL_PROC_DATA *procdata, void * pwk)
{
	GFL_PROC_LOCAL_SetNextProc(gsys->procsys, ov_id, procdata, pwk);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMESYSTEM_CallProc(GAMESYS_WORK * gsys,
		FSOverlayID ov_id, const GFL_PROC_DATA *procdata, void * pwk)
{
	GFL_PROC_LOCAL_CallProc(gsys->procsys, ov_id, procdata, pwk);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMESYSTEM_CallFieldProc(GAMESYS_WORK * gsys)
{
	GAMESYSTEM_CallProc(gsys, FS_OVERLAY_ID(fieldmap), &FieldProcData, gsys);
}

//------------------------------------------------------------------
//	プロセス存在チェック
//------------------------------------------------------------------
GFL_PROC_MAIN_STATUS GAMESYSTEM_IsProcExists(const GAMESYS_WORK * gsys)
{
	return gsys->proc_result;
}
//------------------------------------------------------------------
//	ゲームデータ取得
//------------------------------------------------------------------
GAMEDATA * GAMESYSTEM_GetGameData(GAMESYS_WORK * gsys)
{
	return gsys->gamedata;
}

//------------------------------------------------------------------
//	自分プレイヤーワーク取得
//------------------------------------------------------------------
PLAYER_WORK * GAMESYSTEM_GetMyPlayerWork(GAMESYS_WORK * gsys)
{
	return GAMEDATA_GetMyPlayerWork(gsys->gamedata);
}
//------------------------------------------------------------------
//	ヒープID取得
//------------------------------------------------------------------
HEAPID GAMESYSTEM_GetHeapID(GAMESYS_WORK * gsys)
{
	return gsys->heapID;
}

//------------------------------------------------------------------
//	イベント取得
//------------------------------------------------------------------
GMEVENT * GAMESYSTEM_GetEvent(GAMESYS_WORK * gsys)
{
	return gsys->event;
}

//------------------------------------------------------------------
//	イベント設定
//------------------------------------------------------------------
void GAMESYSTEM_SetEvent(GAMESYS_WORK * gsys, GMEVENT * event)
{
	gsys->event = event;
}

//------------------------------------------------------------------
//	フィールドマップワークへのポインタ取得
//------------------------------------------------------------------
FIELDMAP_WORK * GAMESYSTEM_GetFieldMapWork(GAMESYS_WORK * gsys)
{
	//GF_ASSERT(gsys->fieldmap != NULL);
	return gsys->fieldmap;
}

//------------------------------------------------------------------
//	フィールドマップワークのポインタセット
//------------------------------------------------------------------
void GAMESYSTEM_SetFieldMapWork(GAMESYS_WORK * gsys, FIELDMAP_WORK * fieldmap)
{
	gsys->fieldmap = fieldmap;
}

//--------------------------------------------------------------
//  フィールドマップワークポインタ有効チェック
//--------------------------------------------------------------
BOOL GAMESYSTEM_CheckFieldMapWork( const GAMESYS_WORK *gsys )
{
  if( gsys->fieldmap != NULL ){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * フィールドでの通信エラー画面表示リクエスト設定
 *
 * @param   fieldWork		FIELDMAP_WORK
 * @param   error_req		TRUE:エラー画面表示。　FALSE:リクエストをクリア
 */
//==================================================================
void GAMESYSTEM_SetFieldCommErrorReq( GAMESYS_WORK *gsys, BOOL error_req )
{
  gsys->field_comm_error_req = error_req;
}

//==================================================================
/**
 * フィールドでの通信エラー画面表示リクエスト取得
 *
 * @param   fieldWork		FIELDMAP_WORK
 * @retval  TRUE:エラー画面表示。　FALSE:リクエストをクリア
 */
//==================================================================
BOOL GAMESYSTEM_GetFieldCommErrorReq( const GAMESYS_WORK *gsys )
{
  return gsys->field_comm_error_req;
}

//--------------------------------------------------------------
//	ISSシステム取得
//--------------------------------------------------------------
ISS_SYS * GAMESYSTEM_GetIssSystem( GAMESYS_WORK * gsys )
{
	return gsys->iss_sys;
}

//==================================================================
/**
 * ゲーム通信管理ワークへのポインタを取得する
 * @param   gsys		ゲーム制御システムへのポインタ
 * @retval  GAME_COMM_SYS_PTR		ゲーム通信管理ワークへのポインタ
 */
//==================================================================
GAME_COMM_SYS_PTR GAMESYSTEM_GetGameCommSysPtr(GAMESYS_WORK *gsys)
{
  return gsys->game_comm;
}

//--------------------------------------------------------------
/**
 * @brief   常時通信フラグの取得
 * @param   gsys		ゲーム制御システムへのポインタ
 * @retval  TRUE:常時通信ON、　FALSE:常時通信OFF
 */
//--------------------------------------------------------------
BOOL GAMESYSTEM_GetAlwaysNetFlag(const GAMESYS_WORK * gsys)
{
	return GAMEDATA_GetAlwaysNetFlag(gsys->gamedata);
}

//--------------------------------------------------------------
/**
 * @brief   常時通信フラグの設定
 * @param   gsys		ゲーム制御システムへのポインタ
 * @param	is_on			TRUEならば常時通信モードON FALSEならば常時通信モードOFF
 */
//--------------------------------------------------------------
void GAMESYSTEM_SetAlwaysNetFlag( GAMESYS_WORK * gsys, BOOL is_on )
{	
	GAMEDATA_SetAlwaysNetFlag(gsys->gamedata, is_on);
}

#ifdef PM_DEBUG
//==================================================================
/**
 * デバッグ：ゲーム制御システム用ワーク取得
 *
 * @retval  GAMESYS_WORK *		
 */
//==================================================================
GAMESYS_WORK * DEBUG_GameSysWorkPtrGet(void)
{
  return GameSysWork;
}

#endif //PM_DEBUG
