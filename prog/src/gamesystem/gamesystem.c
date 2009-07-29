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

#include "gamesystem/game_init.h"

#include "src/field/event_mapchange.h"

#include "poke_tool/monsno_def.h"
#ifdef PM_DEBUG
#include "item/item.h"  //デバッグアイテム生成用
#include "savedata/box_savedata.h"  //デバッグアイテム生成用
#endif

//============================================================================================
//============================================================================================
enum {
	HEAPID_GAMESYS = HEAPID_PROC,
	
	HEAPSIZE_GAMESYS = 0x6000,
	//x200000
};

///HEAPID_UNIONのヒープサイズ
#define HEAPSIZE_APP_CONTROL      (0x1000)

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
extern const GFL_PROC_DATA FieldProcData;
FS_EXTERN_OVERLAY(fieldmap);

static void GameSystem_Init(GAMESYS_WORK * gsys, HEAPID heapID, GAME_INIT_WORK * init_param);
static BOOL GameSystem_Main(GAMESYS_WORK * gsys);
static void GameSystem_End(GAMESYS_WORK * gsys);
static u32 GAMESYS_WORK_GetSize(void);
static void DEBUG_MyPokeAdd(GAMESYS_WORK * gsys);
static void DEBUG_MYITEM_MakeBag(MYITEM_PTR myitem, int heapID);


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
	
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GAMESYS, HEAPSIZE_GAMESYS );
	//パレスしながら戦闘もフィールドもするのでここで確保
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_APP_CONTROL, HEAPSIZE_APP_CONTROL );
	
	gsys = GFL_PROC_AllocWork(proc, work_size, HEAPID_GAMESYS);
	GFL_STD_MemClear(gsys, work_size);
	GameSysWork = gsys;
	GameSystem_Init(gsys, HEAPID_GAMESYS, game_init);
#if 1		/* 暫定的にプロセス登録 */
	switch(game_init->mode){
	case GAMEINIT_MODE_CONTINUE:
		{
			GMEVENT * event = DEBUG_EVENT_SetFirstMapIn(gsys, game_init);
			GAMESYSTEM_SetEvent(gsys, event);
		}
		break;
	case GAMEINIT_MODE_FIRST:
	case GAMEINIT_MODE_DEBUG:
		{
#ifdef PM_DEBUG
			GMEVENT * event = DEBUG_EVENT_SetFirstMapIn(gsys, game_init);
			GAMESYSTEM_SetEvent(gsys, event);
			//適当に手持ちポケモンをAdd
			DEBUG_MyPokeAdd(gsys);
			//デバッグアイテム追加
			DEBUG_MYITEM_MakeBag(GAMEDATA_GetMyItem(GAMESYSTEM_GetGameData(gsys)), GFL_HEAPID_APP);
#endif //PM_DEBUG
		}
		break;
	}
#endif
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
	GFL_HEAP_DeleteHeap( HEAPID_GAMESYS );
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
GAME_INIT_WORK * DEBUG_GetGameInitWork(GAMEINIT_MODE mode, u16 mapid, VecFx32 *pos, s16 dir, BOOL always_net)
{
  GFL_STD_MemClear(&TestGameInitWork, sizeof(GAME_INIT_WORK));
	TestGameInitWork.mode = mode;
	TestGameInitWork.mapid = mapid;
	TestGameInitWork.pos = *pos;
	TestGameInitWork.dir = dir;
	TestGameInitWork.always_net = always_net;
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

	BOOL proc_result;

	EVCHECK_FUNC evcheck_func;
	void * evcheck_context;
	GMEVENT * event;

	GAMEDATA * gamedata;
	void * fieldmap;
	GAME_COMM_SYS_PTR game_comm;    ///<ゲーム通信管理ワークへのポインタ
	void * comm_infowin;		///<INFOWIN通信ワーク
	
	u8 always_net;          ///<TRUE:常時通信
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
	gsys->proc_result = FALSE;
	gsys->evcheck_func = NULL;
	gsys->evcheck_context = NULL;
	gsys->event = NULL;

	gsys->gamedata = GAMEDATA_Create(gsys->heapID);
	gsys->fieldmap = NULL;
	gsys->game_comm = GameCommSys_Alloc(gsys->heapID, gsys->gamedata);
	gsys->comm_infowin = NULL;
	gsys->always_net = init_param->always_net;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
static void GAMESYS_WORK_Delete(GAMESYS_WORK * gsys)
{
	GAMEDATA_Delete(gsys->gamedata);
	GameCommSys_Free(gsys->game_comm);
}


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static void GameSystem_Init(GAMESYS_WORK * gsys, HEAPID heapID, GAME_INIT_WORK * init_param)
{
	GAMESYS_WORK_Init(gsys, heapID, init_param);
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

	if(GAMEDATA_IsFrameSpritMode(gsys->gamedata)) //フレーム分割状態にいる場合
	{
		//ここから６０フレーム用のキーを返すようにします
		GFL_UI_ChangeFrameRate(GFL_UI_FRAMERATE_60);
	}

	if (gsys->proc_result == FALSE && gsys->event == NULL)
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
BOOL GAMESYSTEM_IsProcExists(const GAMESYS_WORK * gsys)
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
void * GAMESYSTEM_GetFieldMapWork(GAMESYS_WORK * gsys)
{
	GF_ASSERT(gsys->fieldmap != NULL);
	return gsys->fieldmap;
}

//------------------------------------------------------------------
//	フィールドマップワークのポインタセット
//------------------------------------------------------------------
void GAMESYSTEM_SetFieldMapWork(GAMESYS_WORK * gsys, void * fieldmap)
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
BOOL GAMESYSTEM_GetAlwaysNetFlag(GAMESYS_WORK * gsys)
{
	return gsys->always_net;
}


//--------------------------------------------------------------
/**
 * @brief   デバッグ用に適当に手持ちポケモンをAdd
 * @param   gsys		
 */
//--------------------------------------------------------------
static void DEBUG_MyPokeAdd(GAMESYS_WORK * gsys)
{
  MYSTATUS *myStatus;
	POKEPARTY *party;
	POKEMON_PARAM *pp;
	const STRCODE *name;
	
	party = GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(gsys));
  myStatus = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(gsys));
  name = MyStatus_GetMyName( myStatus );
  
	pp = PP_Create(MONSNO_WANIBAAN, 100, 123456, GFL_HEAPID_APP);
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );
	PokeParty_Add(party, pp);

	PP_Setup(pp, MONSNO_ONOKKUSU, 100, 123456);
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );
	PokeParty_Add(party, pp);

	PP_Setup(pp, MONSNO_BANBIINA, 100, 123456);
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );
	PokeParty_Add(party, pp);

	{
		int i,j;
		BOX_DATA* pBox = SaveData_GetBoxData(GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(gsys)));

		for(i=0;i<18;i++){
			for(j=0;j<5;j++){
				POKEMON_PERSONAL_DATA* ppd = POKE_PERSONAL_OpenHandle(MONSNO_ZENIGAME+i+j, 0, GFL_HEAPID_APP);
				u32 ret = POKE_PERSONAL_GetParam(ppd,POKEPER_ID_sex);

				PP_SetupEx(pp, MONSNO_ZENIGAME+i+j, i+j, 123456,PTL_SETUP_POW_AUTO, ret);
				PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
				PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );

				BOXDAT_PutPokemonBox(pBox, i, (POKEMON_PASO_PARAM*)PP_GetPPPPointerConst(pp));

				POKE_PERSONAL_CloseHandle(ppd);
			}
		}
	}
	
	GFL_HEAP_FreeMemory(pp);
}

#ifdef PM_DEBUG
//------------------------------------------------------------------
/**
 * @brief	デバッグ用：適当に手持ちを生成する
 * @param	myitem	手持ちアイテム構造体へのポインタ
 */
//------------------------------------------------------------------
static const ITEM_ST DebugItem[] = {
	{ ITEM_MASUTAABOORU,	111 },
	{ ITEM_MONSUTAABOORU,	222 },
	{ ITEM_SUUPAABOORU,		333 },
	{ ITEM_HAIPAABOORU,		444 },
	{ ITEM_PUREMIABOORU,	555 },
	{ ITEM_DAIBUBOORU,		666 },
	{ ITEM_TAIMAABOORU,		777 },
	{ ITEM_RIPIITOBOORU,	888 },
	{ ITEM_NESUTOBOORU,		999 },
	{ ITEM_GOOZYASUBOORU,	100 },
	{ ITEM_KIZUGUSURI,		123 },
	{ ITEM_NEMUKEZAMASI,	456 },
	{ ITEM_BATORUREKOODAA,  1},  // バトルレコーダー
	{ ITEM_TAUNMAPPU,		1 },
	{ ITEM_ZITENSYA,		1 },
	{ ITEM_NANDEMONAOSI,	18 },
	{ ITEM_PIIPIIRIKABAA,	18 },
	{ ITEM_PIIPIIMAKKUSU,	18 },
	{ ITEM_ANANUKENOHIMO, 50 },
	{ ITEM_GOORUDOSUPUREE, 50 },
	{ ITEM_DOKUKESI,		18 },		// どくけし
	{ ITEM_YAKEDONAOSI,		19 },		// やけどなおし
	{ ITEM_KOORINAOSI,		20 },		// こおりなおし
	{ ITEM_MAHINAOSI,		22 },		// まひなおし
	{ ITEM_EFEKUTOGAADO,	54 },		// エフェクトガード
	{ ITEM_KURITHIKATTAA,	55 },		// クリティカッター
	{ ITEM_PURASUPAWAA,		56 },		// プラスパワー
	{ ITEM_DHIFENDAA,		57 },		// ディフェンダー
	{ ITEM_SUPIIDAA,		58 },		// スピーダー
	{ ITEM_YOKUATAARU,		59 },		// ヨクアタール
	{ ITEM_SUPESYARUAPPU,	60 },		// スペシャルアップ
	{ ITEM_SUPESYARUGAADO,	61 },		// スペシャルガード
	{ ITEM_PIPPININGYOU,	62 },		// ピッピにんぎょう
	{ ITEM_ENEKONOSIPPO,	63 },		// エネコのシッポ
	{ ITEM_GENKINOKAKERA,	28 },		// げんきのかけら
	{ ITEM_KAIHUKUNOKUSURI,	28 },		// げんきのかけら
	{ ITEM_PIIPIIEIDO,	28 },
	{ ITEM_PIIPIIEIDAA,	28 },
	{ ITEM_DAAKUBOORU,	13 },		// ダークボール
	{ ITEM_HIIRUBOORU,  14 },		// ヒールボール
	{ ITEM_KUIKKUBOORU,	15 },		// クイックボール
	{ ITEM_PURESYASUBOORU,	16 },	// プレシアボール
	{ ITEM_TOMODATITETYOU,  1},  // ともだち手帳
	{ ITEM_POFINKEESU,  1},  // ポルトケース
	{ ITEM_MOKOSINOMI,	50},	//モコシのみ
	{ ITEM_GOSUNOMI,	50},	//ゴスのみ
	{ ITEM_RABUTANOMI,	50},	//ラブタのみ

  { ITEM_WAZAMASIN01,  328},		// わざマシン０１
  {  ITEM_WAZAMASIN02	, 329 },		// わざマシン０２
  {  ITEM_WAZAMASIN03	, 330 },		// わざマシン０３
  {  ITEM_WAZAMASIN04	, 331 },		// わざマシン０４
  {  ITEM_WAZAMASIN05	, 332 },		// わざマシン０５
  
};

static void DEBUG_MYITEM_MakeBag(MYITEM_PTR myitem, int heapID)
{
	u32	i;

	MYITEM_Init( myitem );
	for( i=0; i<NELEMS(DebugItem); i++ ){
		MYITEM_AddItem( myitem, DebugItem[i].id, DebugItem[i].no, heapID );
	}
}

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
