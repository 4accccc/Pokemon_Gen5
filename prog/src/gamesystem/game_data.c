//=============================================================================
/**
 * @file	game_data.c
 * @brief	ゲーム進行データ管理
 * @author	tamada
 * @date	2008.11.04
 *
 * @note
 * 基本的にゲーム進行データはセーブデータとして存在するが、
 * 通信時の複数プレイヤー情報やフィールドマップ情報などセーブデータに
 * のるとは限らない情報もある。
 * そのようなセーブデータ・非セーブデータへ並列にアクセスするインターフェイスを
 * 各パートごとに構成するとパートごとにアクセス手法が違ったり、また同じ機能の
 * ものが複数存在するなどプログラム全体の複雑さが増してしまう可能性が高い。
 * それを避けるため、共通インターフェイスを作ることによって簡略化する。
 */
//=============================================================================

#include <gflib.h>

#include "gamesystem/game_data.h"
#include "gamesystem/playerwork.h"
#include "field/eventdata_system.h"
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/myitem_savedata.h"
#include "savedata/config.h"
#include "poke_tool/pokeparty.h"
#include "savedata/mystatus.h"
#include "savedata/situation.h"
#include "savedata/player_data.h"
#include "gamesystem/pm_season.h"		//季節定義参照
#include "field/field_subscreen.h" //FIELD_SUBSCREEN_ACTION
#include "field/rail_location.h"    //RAIL_LOCATION
#include "sound/bgm_info.h"
#include "field/map_matrix.h"     //MAP_MATRIX
#include "field/field_status.h"   //FIELD_STATUS
#include "field/field_encount.h"
#include "field/field_dir.h"
#include "field/field_wfbc_data.h"  // WF・BC
#include "savedata/intrude_save.h"  //
#include "savedata/randommap_save.h"  //WF・BC
#include "savedata/shortcut.h"		//SHORTCUT_SetRegister
#include "savedata/wifilist.h"
#include "gamesystem/comm_player_support.h"
#include "net/wih_dwc.h"

#include "field/fldmmdl.h"      //MMDLSYS

#include "savedata/gimmickwork_local.h"
#include "savedata/gimmickwork.h"

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ゲームデータ内容定義
 */
//------------------------------------------------------------------
struct _GAMEDATA{
  SAVE_CONTROL_WORK *sv_control_ptr;		///<セーブデータ管理ワークへのポインタ
  PLAYER_WORK playerWork[PLAYER_MAX];
  EVENTDATA_SYSTEM * evdata;
  LOCATION *start_loc;
  LOCATION *entrance_loc;
  LOCATION *special_loc;    ///<特殊接続先位置
  LOCATION *escape_loc;     ///<脱出先位置

  BAG_CURSOR* bagcursor;  ///< バッグカーソルの管理構造体ポインタ
  MYITEM_PTR myitem;			///<手持ちアイテムセーブデータへのポインタ
  POKEPARTY *my_pokeparty;	///<手持ちポケモンセーブデータへのポインタ
	CONFIG		*config;				///<コンフィグセーブデータへのポインタ
  BOX_MANAGER *boxMng;      ///<ボックス管理構造体へのポインタ
  WIFI_LIST* WiFiList;
  BGM_INFO_SYS* bgm_info_sys;	// BGM情報取得システム

  MMDLSYS *mmdlsys;
  EVENTWORK *eventwork;
  MAP_MATRIX * mapMatrix;     ///<マップマトリックス管理
  
  FIELD_SOUND *field_sound; ///<フィールドサウンド管理

  FIELD_STATUS * fldstatus;  ///<フィールドマップ情報

  u32 last_battle_result;   ///<最新のバトルの結果

  int fieldmap_walk_count; ///<フィールドマップ歩数カウント
  u8 season_id;				///<季節指定ID
  u8 subscreen_mode; ///< フィールド下画面の状態
  u8 subscreen_type;
  u8 frameSpritcount;    ///< フレーム分割動作で動作する場合のカウント
  u8 frameSpritEnable;   ///< フレーム分割動作で動作する場合の許可

  u8 intrude_num;         ///<侵入している時の接続人数
  u8 intrude_my_id;       ///<侵入している自分のNetID
  OCCUPY_INFO occupy[OCCUPY_ID_MAX];    ///<占拠情報
  FIELD_WFBC_CORE wfbc[GAMEDATA_WFBC_ID_MAX];  ///<WhiteForest BlackCity
  
  FIELD_BEACON_MSG_DATA *fbmData; //フィールドビーコンメッセージデータ
  
  ENCOUNT_WORK* enc_work; ///<エンカウント関連データワーク
  
	SHORTCUT_CURSOR	shortcut_cursor;	///<ショートカット画面のカーソル
  
  GIMMICKWORK GimmickWork;      //ギミックワーク
  PL_BOAT_WORK_PTR PlBoatWorkPtr;   //遊覧船ワーク

  COMM_PLAYER_SUPPORT *comm_player_support;   ///<通信プレイヤーサポート
  
  BSUBWAY_SCRWORK *bsubway_scrwork; //バトルサブウェイワーク
};

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void GAMEDATA_SaveDataLoad(GAMEDATA *gamedata);
static void GAMEDATA_SaveDataUpdate(GAMEDATA *gamedata);


//============================================================================================
//
//
//		GAMEDATA全体
//
//
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
GAMEDATA * GAMEDATA_Create(HEAPID heapID)
{
  int i;
  GAMEDATA * gd;
  SITUATION *st;

  gd = GFL_HEAP_AllocClearMemory(heapID, sizeof(GAMEDATA));

  gd->sv_control_ptr = SaveControl_GetPointer();

  //季節設定：ゲーム内では1ヶ月＝１シーズン
  {
    RTCDate date;
    GFL_RTC_GetDate(&date);
    gd->season_id = PMSEASON_SPRING;
    //gd->season_id = date.month % PMSEASON_TOTAL;
  }

  //
  gd->subscreen_mode = FIELD_SUBSCREEN_NORMAL;

  //状況データ
  st = SaveData_GetSituation(gd->sv_control_ptr);
  gd->start_loc = Situation_GetStartLocation(st);
  gd->entrance_loc = Situation_GetStartLocation(st);
  gd->special_loc = Situation_GetSpecialLocation(st);
  gd->escape_loc = Situation_GetEscapeLocation(st);

  // BGM情報取得システムを作成(BGM情報を読み込む)
  gd->bgm_info_sys = BGM_INFO_CreateSystem( heapID );

  //プレイヤーワーク
  for (i = 0; i < PLAYER_MAX; i++) {
    PLAYERWORK_init(&gd->playerWork[i]);
  }
  //自分自身のプレイヤーワークのマイステータスにセーブデータのマイステータスをコピーしてくる
  //「最初から」で始めたときでもここに来るまでに名前入力がされているので
  //セーブの有無に関係なくコピー
  GFL_STD_MemCopy(SaveData_GetMyStatus(gd->sv_control_ptr),
                  GAMEDATA_GetMyStatus(gd), MyStatus_GetWorkSize());

  //イベントデータ
  gd->evdata = EVENTDATA_SYS_Create(heapID);
  EVENTDATA_SYS_Clear(gd->evdata);
  
  //イベントワーク
  gd->eventwork = SaveControl_DataPtrGet( gd->sv_control_ptr, GMDATA_ID_EVENT_WORK );
  
  //マップマトリックス
  gd->mapMatrix = MAP_MATRIX_Create( heapID );

  //動作モデル
  gd->mmdlsys = MMDLSYS_CreateSystem( heapID, MMDL_MDL_MAX, SaveControl_DataPtrGet(gd->sv_control_ptr,GMDATA_ID_ROCKPOS) );
  
  //フィールドサウンド管理
  gd->field_sound = FIELD_SOUND_Create( heapID );

  gd->fldstatus = FIELD_STATUS_Create( heapID );

  gd->bagcursor = MYITEM_BagCursorAlloc( heapID );
  gd->myitem = SaveControl_DataPtrGet(gd->sv_control_ptr, GMDATA_ID_MYITEM);
  gd->my_pokeparty = SaveControl_DataPtrGet(gd->sv_control_ptr, GMDATA_ID_MYPOKE);
	gd->config	= SaveData_GetConfig( gd->sv_control_ptr );
  gd->boxMng	= BOX_DAT_InitManager( heapID , gd->sv_control_ptr );
  //WifiList
  gd->WiFiList = WifiList_AllocWork(heapID);

  //占拠情報
  for(i = 0; i < OCCUPY_ID_MAX; i++){
    OccupyInfo_WorkInit(&gd->occupy[i]);
  }

  //街拠情報
  for(i = 0; i < GAMEDATA_WFBC_ID_MAX; i++){
    FIELD_WFBC_CORE_Clear(&gd->wfbc[i]);
  }
  // 街自分の場所初期化
  FIELD_WFBC_CORE_SetUp( &gd->wfbc[GAMEDATA_WFBC_ID_MINE], GAMEDATA_GetMyStatus(gd), heapID );
  FIELD_WFBC_CORE_SetUpZoneData( &gd->wfbc[GAMEDATA_WFBC_ID_MINE] );
  
  //歩数カウント
  gd->fieldmap_walk_count = 0;

  //上にココの時点で名前が入っていると書いてあったので、初期化に使用。
  gd->fbmData = FIELD_BEACON_MSG_CreateData( heapID , GAMEDATA_GetMyStatus(gd) );

  if(SaveControl_NewDataFlagGet(gd->sv_control_ptr) == FALSE){
  }
  
  //エンカウントワーク初期化
  gd->enc_work = ENCOUNT_WORK_Create( heapID );

	//ショートカット用カーソル初期化
	SHORTCUT_CURSOR_Init( &gd->shortcut_cursor );

  GAMEDATA_SaveDataLoad(gd);	//セーブデータから必要なものをロードしてくる

  //遊覧船ポインタをＮＵＬＬ初期化
  gd->PlBoatWorkPtr = NULL;
  
  //バトルサブウェイ NULL初期化
  gd->bsubway_scrwork = NULL;
  
  //通信相手からのサポートデータ
  gd->comm_player_support = COMM_PLAYER_SUPPORT_Alloc(heapID);

  return gd;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_Delete(GAMEDATA * gamedata)
{
  COMM_PLAYER_SUPPORT_Free(gamedata->comm_player_support);
  ENCOUNT_WORK_Delete( gamedata->enc_work );
  FIELD_BEACON_MSG_DeleteData( gamedata->fbmData );
	BGM_INFO_DeleteSystem(gamedata->bgm_info_sys);
  GFL_HEAP_FreeMemory( gamedata->WiFiList );
  BOX_DAT_ExitManager( gamedata->boxMng );
  GFL_HEAP_FreeMemory(gamedata->bagcursor);
  MMDLSYS_FreeSystem(gamedata->mmdlsys);
  MAP_MATRIX_Delete( gamedata->mapMatrix );
  EVENTDATA_SYS_Delete(gamedata->evdata);
  FIELD_STATUS_Delete( gamedata->fldstatus );
  FIELD_SOUND_Delete( gamedata->field_sound );
  GFL_HEAP_FreeMemory(gamedata);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
PLAYER_WORK * GAMEDATA_GetPlayerWork(GAMEDATA * gamedata, u32 player_id)
{
  GF_ASSERT(player_id < PLAYER_MAX);
  return &gamedata->playerWork[player_id];
}

//------------------------------------------------------------------
//------------------------------------------------------------------
PLAYER_WORK * GAMEDATA_GetMyPlayerWork(GAMEDATA * gamedata)
{
  return &gamedata->playerWork[PLAYER_ID_MINE];
}

//--------------------------------------------------------------
/**
 * @brief   OCCUPY_INFOへのポインタ取得(プレイヤーID指定)
 * @param   gamedata		GAMEDATAへのポインタ
 * @param   player_id   プレイヤーID
 * @retval  OCCUPY_INFOへのポインタ
 */
//--------------------------------------------------------------
OCCUPY_INFO * GAMEDATA_GetOccupyInfo(GAMEDATA * gamedata, u32 player_id)
{
  GF_ASSERT_MSG(player_id < OCCUPY_ID_MAX, "player_id = %d\n", player_id);
  return &gamedata->occupy[player_id];
}

//--------------------------------------------------------------
/**
 * @brief   OCCUPY_INFOへのポインタ取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @retval  OCCUPY_INFOへのポインタ
 */
//--------------------------------------------------------------
OCCUPY_INFO * GAMEDATA_GetMyOccupyInfo(GAMEDATA * gamedata)
{
  return GAMEDATA_GetOccupyInfo(gamedata, OCCUPY_ID_MINE);
}


//----------------------------------------------------------------------------
/**
 *	@brief  自分のWFBC街情報の取得
 *
 *	@param	gamedata  ゲームデータ
 *
 *	@return WFBC街情報のポインタ
 */
//-----------------------------------------------------------------------------
FIELD_WFBC_CORE* GAMEDATA_GetMyWFBCCoreData( GAMEDATA * gamedata )
{
  return GAMEDATA_GetWFBCCoreData( gamedata, GAMEDATA_WFBC_ID_MINE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレイヤーのWFBC街情報の取得
 *
 *	@param	gamedata      ゲームデータ
 *	@param	player_id     プレイヤーID
 *
 *	@return WFBC街情報のポインタ
 */
//-----------------------------------------------------------------------------
FIELD_WFBC_CORE* GAMEDATA_GetWFBCCoreData( GAMEDATA * gamedata, GAMEDATA_WFBC_ID id )
{
  GF_ASSERT_MSG(id < GAMEDATA_WFBC_ID_MAX, "wfbc data data_id = %d\n", id);
  return &gamedata->wfbc[id];
}

//----------------------------------------------------------------------------
/**
 *	@brief  Palaceのコア情報設定
 *
 *	@param	gamedata  ゲームデータ
 *	@param	cp_core   Palaceのコア情報
 */ 
//-----------------------------------------------------------------------------
void GAMEDATA_SetUpPalaceWFBCCoreData( GAMEDATA * gamedata, const FIELD_WFBC_CORE* cp_core )
{
  GF_ASSERT( gamedata );
  GF_ASSERT( cp_core );
  GFL_STD_MemCopy( cp_core, &gamedata->wfbc[ GAMEDATA_WFBC_ID_COMM ], sizeof(FIELD_WFBC_CORE) );

  // PalaceマップのZONE変更処理
  FIELD_WFBC_CORE_SetUpZoneData( &gamedata->wfbc[ GAMEDATA_WFBC_ID_COMM ] );
}

//----------------------------------------------------------------------------
/**
 *	@brief  Palaceのコア情報を破棄
 *
 *	@param	gamedata ゲームデータ
 */
//-----------------------------------------------------------------------------
void GAMEDATA_ClearPalaceWFBCCoreData( GAMEDATA * gamedata )
{
  GF_ASSERT( gamedata );
  GFL_STD_MemClear( &gamedata->wfbc[ GAMEDATA_WFBC_ID_COMM ], sizeof(FIELD_WFBC_CORE) );

  // 自分のマップのZONE変更処理
  FIELD_WFBC_CORE_SetUpZoneData( &gamedata->wfbc[ GAMEDATA_WFBC_ID_MINE ] );
}


//----------------------------------------------------------------------------
/**
 *	@brief  GAMEDATAからのWIFIListの取得
 *	@param	gamedata      ゲームデータ
 *	@return WIFIListのポインタ
 */
//-----------------------------------------------------------------------------
WIFI_LIST * GAMEDATA_GetWiFiList(GAMEDATA * gamedata)
{
  return gamedata->WiFiList;
}



//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	イベント起動データシステムへのポインタを取得する
 * @param	gamedata			GAMEDATAへのポインタ
 * @return	EVENTDATA_SYSTEM	イベント起動データシステムへのポインタ
 */
//------------------------------------------------------------------
EVENTDATA_SYSTEM * GAMEDATA_GetEventData(GAMEDATA * gamedata)
{
  return gamedata->evdata;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
const LOCATION * GAMEDATA_GetStartLocation(const GAMEDATA * gamedata)
{
  return gamedata->start_loc;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_SetStartLocation(GAMEDATA * gamedata, const LOCATION * loc)
{
  *(gamedata->start_loc) = *loc;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const LOCATION * GAMEDATA_GetEntranceLocation(const GAMEDATA * gamedata)
{
  return gamedata->entrance_loc;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_SetEntranceLocation(GAMEDATA * gamedata, const LOCATION * loc)
{
  *(gamedata->entrance_loc) = *loc;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const LOCATION * GAMEDATA_GetSpecialLocation(const GAMEDATA * gamedata)
{
  return gamedata->special_loc;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_SetSpecialLocation(GAMEDATA * gamedata, const LOCATION * loc)
{
  *(gamedata->special_loc) = *loc;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
const LOCATION * GAMEDATA_GetEscapeLocation(const GAMEDATA * gamedata)
{
  return gamedata->escape_loc;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_SetEscapeLocation(GAMEDATA * gamedata, const LOCATION * loc)
{
  *(gamedata->escape_loc) = *loc;
}

//--------------------------------------------------------------
/**
 * @brief ワープ飛び先IDのセット
 * @param	gamedata	GAMEDATAへのポインタ
 * @param warp_id   ワープ飛び先ID指定
 */
//--------------------------------------------------------------
void GAMEDATA_SetWarpID(GAMEDATA * gamedata, u16 warp_id)
{
  u16 * warp = Situation_GetWarpID( SaveData_GetSituation(gamedata->sv_control_ptr) );
  *warp = warp_id;
}

//--------------------------------------------------------------
/**
 * @brief  ワープ飛び先IDの取得
 * @param	gamedata	GAMEDATAへのポインタ
 * @return  u16 ワープ飛び先ID
 */
//--------------------------------------------------------------
u16 GAMEDATA_GetWarpID(GAMEDATA * gamedata)
{
  u16 * warp = Situation_GetWarpID( SaveData_GetSituation(gamedata->sv_control_ptr) );
  return *warp;
}

//--------------------------------------------------------------
/**
 * @brief   バッグカーソルのポインタ取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @retval  バッグカーソルポインタ
 */
//--------------------------------------------------------------
BAG_CURSOR* GAMEDATA_GetBagCursor(const GAMEDATA * gamedata)
{
  return gamedata->bagcursor;
}

//--------------------------------------------------------------
/**
 * @brief   手持ちアイテムへのポインタ取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @retval  手持ちアイテムへのポインタ
 */
//--------------------------------------------------------------
MYITEM_PTR GAMEDATA_GetMyItem(const GAMEDATA * gamedata)
{
  return gamedata->myitem;
}

//--------------------------------------------------------------
/**
 * @brief   手持ちポケモンへのポインタ取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @retval  手持ちポケモンへのポインタ
 */
//--------------------------------------------------------------
POKEPARTY * GAMEDATA_GetMyPokemon(const GAMEDATA * gamedata)
{
  return gamedata->my_pokeparty;
}

//--------------------------------------------------------------
/**
 * @brief   ボックス管理構造体のポインタ取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @retval  ボックス管理構造体へのポインタ
 */
//--------------------------------------------------------------
BOX_MANAGER * GAMEDATA_GetBoxManager(const GAMEDATA * gamedata)
{
  return gamedata->boxMng;
}

//--------------------------------------------------------------
/**
 * @brief   MYSTATUSへのポインタ取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @retval  MYSTATUSへのポインタ
 */
//--------------------------------------------------------------
MYSTATUS * GAMEDATA_GetMyStatus(GAMEDATA * gamedata)
{
  return &((GAMEDATA_GetMyPlayerWork(gamedata))->mystatus);
}

//--------------------------------------------------------------
/**
 * @brief   MYSTATUSへのポインタ取得(プレイヤーID指定)
 * @param   gamedata		GAMEDATAへのポインタ
 * @param   player_id   プレイヤーID
 * @retval  MYSTATUSへのポインタ
 */
//--------------------------------------------------------------
MYSTATUS * GAMEDATA_GetMyStatusPlayer(GAMEDATA * gamedata, u32 player_id)
{
  GF_ASSERT(player_id < PLAYER_MAX);
  return &gamedata->playerWork[player_id].mystatus;
}

//--------------------------------------------------------------
/**
 * @brief BGM_INFO_SYSへのポインタ取得
 * @param gamedata    GAMEDATAへのポインタ
 * preturn BGM_INFO_SYS BGM情報取得システムへのポインタ
 */
//--------------------------------------------------------------
extern BGM_INFO_SYS * GAMEDATA_GetBGMInfoSys(GAMEDATA * gamedata)
{
	return gamedata->bgm_info_sys;
}

//--------------------------------------------------------------
/**
 * @brief	MMDLSYSへのポインタ取得
 * @param	gamedata	GAMEDATAへのポインタ
 * @retval	MMDLSYSへのポインタ
 */
//--------------------------------------------------------------
MMDLSYS * GAMEDATA_GetMMdlSys(GAMEDATA *gamedata)
{
  return gamedata->mmdlsys;
}

//--------------------------------------------------------------
/**
 * @brief	EVENTWORKへのポインタ取得
 * @param	gamedata	GAMEDATAへのポインタ
 * @retval	EVENTWORKへのポインタ
 */
//--------------------------------------------------------------
EVENTWORK * GAMEDATA_GetEventWork(GAMEDATA *gamedata)
{
  return gamedata->eventwork;
}

//--------------------------------------------------------------
/**
 * @brief   MAP_MATRIXへのポインタ取得
 * @param	  gamedata	GAMEDATAへのポインタ
 * @retval  MAP_MATRIXへのポインタ
 */
//--------------------------------------------------------------
MAP_MATRIX * GAMEDATA_GetMapMatrix(GAMEDATA * gamedata)
{
  return gamedata->mapMatrix;
}

//--------------------------------------------------------------
/**
 * @brief	季節の取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @return	u8	季節ID（gamesystem/pm_season.h参照）
 */
//--------------------------------------------------------------
u8 GAMEDATA_GetSeasonID(const GAMEDATA *gamedata)
{
  return gamedata->season_id;
}

//--------------------------------------------------------------
/**
 * @brief	季節のセット
 * @param   gamedata	GAMEDATAへのポインタ
 * @param	season_id	季節ID（gamesystem/pm_season.h参照）
 *
 * 実際の機能として必要かどうかは疑問だがデバッグ用として追加
 */
//--------------------------------------------------------------
void GAMEDATA_SetSeasonID(GAMEDATA *gamedata, u8 season_id)
{
  GF_ASSERT(season_id < PMSEASON_TOTAL);
  gamedata->season_id = season_id;
}

//--------------------------------------------------------------
/**
 * @brief	  フィールド下画面の取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @return	subscreen_mode
 */
//--------------------------------------------------------------
u8 GAMEDATA_GetSubScreenMode(const GAMEDATA *gamedata)
{
  return gamedata->subscreen_mode;
}

//--------------------------------------------------------------
/**
 * @brief	  フィールド下画面の設定
 * @param   gamedata	GAMEDATAへのポインタ
 * @param	  subscreen_mode
 */
//--------------------------------------------------------------
void GAMEDATA_SetSubScreenMode(GAMEDATA *gamedata, u8 subscreen_mode)
{
  GF_ASSERT(subscreen_mode < FIELD_SUBSCREEN_MODE_MAX);
  gamedata->subscreen_mode = subscreen_mode;
}
//--------------------------------------------------------------
/**
 * @brief	  フィールド下画面タイプの取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @return	subscreen_type
 */
//--------------------------------------------------------------
const u8 GAMEDATA_GetSubScreenType(const GAMEDATA *gamedata)
{
  return gamedata->subscreen_type;
}

//--------------------------------------------------------------
/**
 * @brief	  フィールド下画面種類の設定
 * @param   gamedata	GAMEDATAへのポインタ
 * @param	  subscreen_type
 */
//--------------------------------------------------------------
void GAMEDATA_SetSubScreenType(GAMEDATA *gamedata, const u8 subscreen_type)
{
  gamedata->subscreen_type = subscreen_type;
}

//------------------------------------------------------------------
/**
 * @brief	  フレーム分割動作で動作する場合の許可フラグ設定
 * @param   gamedata	GAMEDATAへのポインタ
 * @param	  bEnable 分割する場合TRUE
 */
//------------------------------------------------------------------
void GAMEDATA_SetFrameSpritEnable(GAMEDATA *gamedata,BOOL bEnable)
{
  gamedata->frameSpritEnable = bEnable;
  if(bEnable){
    gamedata->frameSpritcount = 0;    ///< フレーム分割動作で動作する場合のカウント
  }
}

//------------------------------------------------------------------
/**
 * @brief	  フレーム分割動作で動作する場合のカウンタリセット
 * @param   gamedata	GAMEDATAへのポインタ
 */
//------------------------------------------------------------------
void GAMEDATA_ResetFrameSpritCount(GAMEDATA *gamedata)
{
  gamedata->frameSpritcount = 0;    ///< フレーム分割動作で動作する場合のカウント
}

//------------------------------------------------------------------
/**
 * @brief	  フレーム分割動作で動作する場合のフレーム
 * @param   gamedata	GAMEDATAへのポインタ
 */
//------------------------------------------------------------------
u8 GAMEDATA_GetAndAddFrameSpritCount(GAMEDATA *gamedata)
{
  if(gamedata->frameSpritEnable){
    u8 ret = gamedata->frameSpritcount;
    gamedata->frameSpritcount++;
    return ret;
  }
  return 0;
}

//------------------------------------------------------------------
/**
 * @brief	  フレーム分割動作で動作しているかどうか
 * @param   gamedata	GAMEDATAへのポインタ
 * @return  動作している場合TRUE
 */
//------------------------------------------------------------------
BOOL GAMEDATA_IsFrameSpritMode(GAMEDATA *gamedata)
{
  return gamedata->frameSpritEnable;
}

//------------------------------------------------------------------
/**
 * @brief	  フィールドマップ歩数カウント取得
 * @param   gamedata	GAMEDATAへのポインタ
 * @return  int 歩数カウント
 */
//------------------------------------------------------------------
int GAMEDATA_GetFieldMapWalkCount(GAMEDATA *gamedata)
{
  return gamedata->fieldmap_walk_count;
}

//------------------------------------------------------------------
/**
 * @brief	  フィールドマップ歩数カウントセット
 * @param   gamedata	GAMEDATAへのポインタ
 * @param   count セットするカウント
 * @return  nothing
 */
//------------------------------------------------------------------
void GAMEDATA_SetFieldMapWalkCount(GAMEDATA *gamedata,int count)
{
  gamedata->fieldmap_walk_count = count;
}

//--------------------------------------------------------------
/**
 * @brief FIELD_SOUND取得
 * @param   gamedata	GAMEDATAへのポインタ
 * @return  FIELD_SOUND*
 */
//--------------------------------------------------------------
FIELD_SOUND * GAMEDATA_GetFieldSound( GAMEDATA *gamedata )
{
  return( gamedata->field_sound );
}

//--------------------------------------------------------------
/**
 * @brief   ENCOUNT_WORK取得
 * @param   gamedata	GAMEDATAへのポインタ
 * @return  ENCOUNT_WORK*
 */
//--------------------------------------------------------------
ENCOUNT_WORK* GAMEDATA_GetEncountWork( GAMEDATA *gamedata )
{
  return( gamedata->enc_work );
}
//--------------------------------------------------------------
/**
 * @brief 最新の戦闘結果を取得
 * @param   gamedata	GAMEDATAへのポインタ
 * @return  u32 最新の戦闘結果（include/battle/battle.hのBtlResult）
 */
//--------------------------------------------------------------
u32 GAMEDATA_GetLastBattleResult( const GAMEDATA * gamedata )
{
  return gamedata->last_battle_result;
}

//--------------------------------------------------------------
/**
 * @brief 最新の戦闘結果をセット
 * @param   gamedata	GAMEDATAへのポインタ
 * @param btl_result  戦闘結果（include/battle/battle.hのBtlResult）
 */
//--------------------------------------------------------------
void GAMEDATA_SetLastBattleResult( GAMEDATA * gamedata, u32 btl_result )
{
  gamedata->last_battle_result = btl_result;
}

//============================================================================================
//
//
//	PLAYER_WORK用関数
//	※player_work.cを作成して移動させるかも
//
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	PLAYER_WORK初期化
 */
//------------------------------------------------------------------
void PLAYERWORK_init(PLAYER_WORK * player)
{
  player->zoneID = 0;
  player->position.x = 0;
  player->position.y = 0;
  player->position.z = 0;
  player->direction = 0;
  player->pos_type = 0;
  player->mystatus.id = 0;
  player->mystatus.sex = 0;
  player->mystatus.region_code = 0;
  player->railposition.type       = 0;
  player->railposition.key        = 0;
  player->railposition.rail_index = 0;
  player->railposition.line_grid  = 0;
  player->railposition.width_grid = 0;
  player->objcode_fix = OBJCODEMAX;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PLAYERWORK_setPosition(PLAYER_WORK * player, const VecFx32 * pos)
{
  player->position = *pos;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
const VecFx32 * PLAYERWORK_getPosition(const PLAYER_WORK * player)
{
  return &player->position;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void PLAYERWORK_setRailPosition(PLAYER_WORK * player, const RAIL_LOCATION * railpos)
{
  player->railposition = *railpos;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
const RAIL_LOCATION * PLAYERWORK_getRailPosition(const PLAYER_WORK * player)
{
  return &player->railposition;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void PLAYERWORK_setPosType(PLAYER_WORK * player, LOCATION_POS_TYPE pos_type)
{
  player->pos_type = pos_type;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
LOCATION_POS_TYPE PLAYERWORK_getPosType(const PLAYER_WORK * player)
{
  return player->pos_type;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PLAYERWORK_setZoneID(PLAYER_WORK * player, ZONEID zoneid)
{
  player->zoneID = zoneid;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
ZONEID PLAYERWORK_getZoneID(const PLAYER_WORK * player)
{
  return player->zoneID;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void PLAYERWORK_setDirection(PLAYER_WORK * player, u16 direction)
{
  player->direction = direction;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u16 PLAYERWORK_getDirection(const PLAYER_WORK * player)
{
  return player->direction;
}

//--------------------------------------------------------------
/**
 * @brief	方向タイプで方向角度を設定する
 * @param	player		PLAYER_WORKへのポインタ
 * @param	dir_type	方向を指定する値(DIR_～)
 */
//--------------------------------------------------------------
void PLAYERWORK_setDirection_Type(PLAYER_WORK * player, u16 dir_type)
{
  static const u16 DIR_ROT[DIR_MAX4] = 
  {
    0,
    0x8000,
    0x4000,
    0xC000,
  };
  
  GF_ASSERT( dir_type < DIR_MAX4 );

  player->direction = DIR_ROT[ dir_type ];
}

//--------------------------------------------------------------
/**
 * @brief	方向角度の方向タイプを取得する
 * @param	player	PLAYER_WORKへのポインタ
 * @return	方向を指定する値(DIR_～)
 */
//--------------------------------------------------------------
u16 PLAYERWORK_getDirection_Type(const PLAYER_WORK * player)
{
  u16 dir = DIR_UP;
  
	if( (player->direction>0x2000) && (player->direction<0x6000) ){
		dir = DIR_LEFT;
	}else if( (player->direction >= 0x6000) && (player->direction <= 0xa000) ){
		dir = DIR_DOWN;
	}else if( (player->direction > 0xa000)&&(player->direction < 0xe000) ){
		dir = DIR_RIGHT;
	}
	return( dir );
}

//--------------------------------------------------------------
//--------------------------------------------------------------
//※check　パレステストの為の暫定関数
void PLAYERWORK_setPalaceArea(PLAYER_WORK * player, u8 palace_area)
{
  player->palace_area = palace_area;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
//※check　パレステストの為の暫定関数
u8 PLAYERWORK_getPalaceArea(const PLAYER_WORK * player)
{
  return player->palace_area;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
PLAYER_MOVE_FORM PLAYERWORK_GetMoveForm( const PLAYER_WORK *player )
{
  return player->move_form;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void PLAYERWORK_SetMoveForm( PLAYER_WORK *player, PLAYER_MOVE_FORM form )
{
  player->move_form = form;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void PLAYERWORK_SetOBJCodeFix( PLAYER_WORK *player, u16 code )
{
  player->objcode_fix = code;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
u16 PLAYERWORK_GetOBJCodeFix( const PLAYER_WORK *player )
{
  return player->objcode_fix;
}

//==================================================================
/**
 * @brief   FIELD_STATUSの取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @return  FIELD_STATUS
 */
//==================================================================
FIELD_STATUS * GAMEDATA_GetFieldStatus(GAMEDATA * gamedata)
{
  return gamedata->fldstatus;
}

//==================================================================
/**
 * 侵入接続人数取得
 *
 * @param   gamedata		GAMEDATAへのポインタ
 *
 * @retval  int		接続人数
 */
//==================================================================
int GAMEDATA_GetIntrudeNum(const GAMEDATA *gamedata)
{
  return gamedata->intrude_num;
}

//==================================================================
/**
 * 侵入時の自分のNetIDを取得
 *
 * @param   gamedata		GAMEDATAへのポインタ
 *
 * @retval  int		自分のNetID
 */
//==================================================================
int GAMEDATA_GetIntrudeMyID(const GAMEDATA *gamedata)
{
  return gamedata->intrude_my_id;
}

//==================================================================
/**
 * 侵入時の自分のNetIDをセット
 *
 * @param   gamedata		GAMEDATAへのポインタ
 *
 * @retval  int		自分のNetID
 */
//==================================================================
void GAMEDATA_SetIntrudeMyID(GAMEDATA *gamedata, int intrude_my_id)
{
  gamedata->intrude_my_id = intrude_my_id;
  OS_TPrintf("自分の侵入MyNetID=%d\n", intrude_my_id);
}

//----------------------------------------------------------------------------
/**
 *	@brief	常時通信モードフラグ取得
 *
 *	@param	gamedata		GAMEDATAへのポインタ
 *	
 *	@return	TRUE常時通信モードON FALSE常時通信モードOFF
 */
//-----------------------------------------------------------------------------
BOOL GAMEDATA_GetAlwaysNetFlag( const GAMEDATA *gamedata )
{	
	return CONFIG_GetNetworkSearchMode( gamedata->config ) == NETWORK_SEARCH_ON;
}
//----------------------------------------------------------------------------
/**
 *	@brief	常時通信モードフラグ設定
 *
 *	@param	gamedata		GAMEDATAへのポインタ
 *	@param	is_on				TRUEならば常時通信モードON FALSEならば常時通信モードOFF
 */
//-----------------------------------------------------------------------------
void GAMEDATA_SetAlwaysNetFlag( GAMEDATA *gamedata, BOOL is_on )
{	
	NETWORK_SEARCH_MODE mode	= is_on? NETWORK_SEARCH_ON: NETWORK_SEARCH_OFF;
	CONFIG_SetNetworkSearchMode( gamedata->config, mode );
}

//==================================================================
/**
 * フィールドビーコンメッセージデータ取得
 *
 * @param   gamedata		GAMEDATAへのポインタ
 *
 * @retval  
 */
//==================================================================
FIELD_BEACON_MSG_DATA* GAMEDATA_GetFieldBeaconMessageData( GAMEDATA *gamedata )
{
  return gamedata->fbmData;
}

//----------------------------------------------------------------------------
/**
 *	@brief	Yボタン登録設定
 *
 *  @param  gamedata		GAMEDATAへのポインタ
 *  @param	shortcutID	Yボタン登録するID
 *  @param	is_on				TRUEならばYボタン登録	FALSEならば解除
 */
//----------------------------------------------------------------------------
void GAMEDATA_SetShortCut( GAMEDATA *gamedata, SHORTCUT_ID shortcutID, BOOL is_on )
{	
	SAVE_CONTROL_WORK*	p_sv	= GAMEDATA_GetSaveControlWork(gamedata);

	SHORTCUT *p_shortcut	=  SaveData_GetShortCut( p_sv );

	SHORTCUT_SetRegister( p_shortcut, shortcutID, is_on );	
}

//----------------------------------------------------------------------------
/**
 *	@brief	Yボタン登録設定
 *
 *  @param  gamedata		GAMEDATAへのポインタ
 *  @param	shortcutID	Yボタン登録するID
 *  @retval	TRUEならばYBTNに登録してある	FALSEならば登録していない
 */
//----------------------------------------------------------------------------
BOOL GAMEDATA_GetShortCut( const GAMEDATA *gamedata, SHORTCUT_ID shortcutID )
{	
	const SAVE_CONTROL_WORK*	cp_sv	= GAMEDATA_GetSaveControlWorkConst(gamedata);

	const SHORTCUT *cp_shortcut	=  SaveData_GetShortCutConst( cp_sv );

	return SHORTCUT_GetRegister( cp_shortcut, shortcutID );	
}


//----------------------------------------------------------------------------
/**
 *	@brief	Yボタン用メニューのカーソルセット
 *
 *	@param	gamedata	ゲームデータ
 *	@param	cursor		カーソル
 */
//-----------------------------------------------------------------------------
void GAMEDATA_SetShortCutCursor( GAMEDATA *gamedata, SHORTCUT_CURSOR *cursor )
{	
	gamedata->shortcut_cursor	= *cursor;
}

//----------------------------------------------------------------------------
/**
 *	@brief	Yボタン用メニューのカーソル取得
 *
 *	@param	gamedata	ゲームデータ
 *	@retval	cursor		カーソル
 */
//-----------------------------------------------------------------------------
SHORTCUT_CURSOR * GAMEDATA_GetShortCutCursor( GAMEDATA *gamedata )
{	
	return &gamedata->shortcut_cursor;
}

//==============================================================================
//	ゲームデータが持つ情報を元にセーブ
//==============================================================================
///この関数はここでしか使用してはいけないので直接定義
extern WIFI_LIST* SaveData_GetWifiListData(SAVE_CONTROL_WORK* pSave);

//--------------------------------------------------------------
/**
 * @brief   セーブデータワークからゲームデータが持つ情報をロードする
 *
 * @param   gamedata		ゲームデータへのポインタ
 */
//--------------------------------------------------------------
static void GAMEDATA_SaveDataLoad(GAMEDATA *gamedata)
{
  if(SaveControl_NewDataFlagGet(gamedata->sv_control_ptr) == TRUE){
    return;	//セーブデータが無いので何も読み込まない
  }

  {	//PLAYER_WORK
    PLAYER_WORK *pw;
    pw = GAMEDATA_GetMyPlayerWork(gamedata);
    SaveData_PlayerDataLoad(gamedata->sv_control_ptr, pw);
    SaveData_SituationDataLoad(gamedata->sv_control_ptr, pw);
  }

  {	//MMDL
    MMDLSYS *mmdlsys = GAMEDATA_GetMMdlSys(gamedata);
    MMDL_SAVEDATA *pw = SaveControl_DataPtrGet(gamedata->sv_control_ptr,GMDATA_ID_MMDL);
    MMDL_SAVEDATA_Load( mmdlsys, pw );
  }
  
  { //OCCUPY_INFO
    OCCUPY_INFO *occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
    SaveData_OccupyInfoLoad(gamedata->sv_control_ptr, occupy);
  }

  { //WFBC_CORE
    FIELD_WFBC_CORE *wfbc = GAMEDATA_GetMyWFBCCoreData(gamedata);
    RANDOMMAP_SAVE* save = RANDOMMAP_SAVE_GetRandomMapSave( gamedata->sv_control_ptr );
    RANDOMMAP_SAVE_GetCoreWork(save, wfbc);
  }

  { //GIMMICK_WORK
    SaveData_LoadGimmickWork(gamedata->sv_control_ptr, &gamedata->GimmickWork);
  }

  { //FIELD_STATUS
    SITUATION * situation = SaveData_GetSituation( gamedata->sv_control_ptr );
    SaveData_SituationDataLoadStatus( situation, gamedata->fldstatus );
    SaveData_SituationLoadSeasonID( situation, &gamedata->season_id );
  }
  { //WIFILIST
    WIFI_LIST* wifilist = GAMEDATA_GetWiFiList(gamedata);
    const WIFI_LIST* swifilist = SaveData_GetWifiListData( gamedata->sv_control_ptr );
    WifiList_Copy(swifilist, wifilist);
  }
}

//--------------------------------------------------------------
/**
 * @brief   ゲームデータが持つ情報を元にセーブデータのワークを更新する
 *
 * @param   gamedata		ゲームデータへのポインタ
 *
 * セーブするデータを、セーブワークから直接使用せずに、別ワークにコピーして使用しているものが対象
 */
//--------------------------------------------------------------
static void GAMEDATA_SaveDataUpdate(GAMEDATA *gamedata)
{
  {	//PLAYER_WORK
    PLAYER_WORK *pw = GAMEDATA_GetMyPlayerWork(gamedata);
    SaveData_PlayerDataUpdate(gamedata->sv_control_ptr, pw);
    SaveData_SituationDataUpdate(gamedata->sv_control_ptr, pw);
  }

  {	//MMDL
    MMDLSYS *mmdlsys = GAMEDATA_GetMMdlSys(gamedata);
    MMDL_SAVEDATA *pw = SaveControl_DataPtrGet(gamedata->sv_control_ptr,GMDATA_ID_MMDL);
    MMDL_SAVEDATA_Save( mmdlsys, pw );
  }
  
  { //OCCUPY_INFO
    OCCUPY_INFO *occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
    SaveData_OccupyInfoUpdate(gamedata->sv_control_ptr, occupy);
  }

  { //WFBC_CORE
    FIELD_WFBC_CORE *wfbc = GAMEDATA_GetMyWFBCCoreData(gamedata);
    RANDOMMAP_SAVE* save = RANDOMMAP_SAVE_GetRandomMapSave( gamedata->sv_control_ptr );
    RANDOMMAP_SAVE_SetCoreWork(save, wfbc);
  }

  { //GIMMICK_WORK
    SaveData_SaveGimmickWork(&gamedata->GimmickWork, gamedata->sv_control_ptr);
  }

  { //FIELD_STATUS
    SITUATION * situation = SaveData_GetSituation( gamedata->sv_control_ptr );
    SaveData_SituationDataUpdateStatus( situation, gamedata->fldstatus );
    SaveData_SituationUpdateSeasonID( situation, gamedata->season_id );
  }
  { //WIFILIST
    const WIFI_LIST* wifilist = GAMEDATA_GetWiFiList(gamedata);
    WIFI_LIST* swifilist = SaveData_GetWifiListData( gamedata->sv_control_ptr );
    WifiList_Copy(wifilist, swifilist);
  }
}

//--------------------------------------------------------------
/**
 * @brief   ゲームデータが持つ情報を元にセーブを実行
 *
 * @param   gamedata		ゲームデータへのポインタ
 *
 * @retval  セーブ結果
 */
//--------------------------------------------------------------
SAVE_RESULT GAMEDATA_Save(GAMEDATA *gamedata)
{
  SAVE_RESULT sr;
  
  //セーブワークの情報を更新
  GAMEDATA_SaveDataUpdate(gamedata);

  //ビーコンのスキャンを即時停止
  WIH_DWC_Stop();

  //セーブ実行
  sr = SaveControl_Save(gamedata->sv_control_ptr);

  //ビーコンのスキャンを再開
  WIH_DWC_Restart();

  return sr;
}

//--------------------------------------------------------------
/**
 * @brief   ゲームデータが持つ情報を元に分割セーブを開始
 *
 * @param   gamedata		ゲームデータへのポインタ
 *
 * @retval  none
 */
//--------------------------------------------------------------
void GAMEDATA_SaveAsyncStart(GAMEDATA *gamedata)
{
  //セーブワークの情報を更新
  GAMEDATA_SaveDataUpdate(gamedata);

  //ビーコンのスキャンを即時停止
  WIH_DWC_Stop();

  //セーブ開始
  SaveControl_SaveAsyncInit(gamedata->sv_control_ptr);
}


//--------------------------------------------------------------
/**
 * @brief   ゲームデータが持つ情報を元に分割セーブを実行
 *
 * @param   gamedata		ゲームデータへのポインタ
 *
 * @retval  セーブ結果
 */
//--------------------------------------------------------------
SAVE_RESULT GAMEDATA_SaveAsyncMain(GAMEDATA *gamedata)
{
  SAVE_RESULT sr;

  sr = SaveControl_SaveAsyncMain(gamedata->sv_control_ptr);

  if(sr==SAVE_RESULT_OK){
    //ビーコンのスキャンを再開
    WIH_DWC_Restart();
  }

  return sr;
}

//--------------------------------------------------------------
/**
 * @brief	  セーブコントロールワークを取得する
 * @param	  gamedata			GAMEDATAへのポインタ
 * @return	SAVE_CONTROL_WORK	のポインタ
 */
//--------------------------------------------------------------

SAVE_CONTROL_WORK* GAMEDATA_GetSaveControlWork(GAMEDATA * gamedata)
{
  return gamedata->sv_control_ptr;
}

//--------------------------------------------------------------
/**
 * @brief	  セーブコントロールワークを取得する
 * @param	  gamedata			GAMEDATAへのポインタ
 * @return	SAVE_CONTROL_WORK	のポインタ
 */
//--------------------------------------------------------------
const SAVE_CONTROL_WORK* GAMEDATA_GetSaveControlWorkConst(const GAMEDATA * gamedata)
{
  return gamedata->sv_control_ptr;
}

//----------------------------------------------------------
/**
 * @brief   ギミックデータへのポインタ取得
 * @param	  gamedata			GAMEDATAへのポインタ
 * @return	ギミックデータへのポインタ
 */
//----------------------------------------------------------
GIMMICKWORK * GAMEDATA_GetGimmickWork(GAMEDATA * gamedata)
{
	return &gamedata->GimmickWork;
}

//----------------------------------------------------------
/**
 * @brief   遊覧船へのポインタ取得
 * @param	  gamedata			GAMEDATAへのポインタ
 * @return	遊覧船ワークへのポインタ
 */
//----------------------------------------------------------
PL_BOAT_WORK_PTR *GAMEDATA_GetPlBoatWorkPtr(GAMEDATA * gamedata)
{
	return &gamedata->PlBoatWorkPtr;
}

//----------------------------------------------------------
/**
 * @brief   バトルサブウェイワーク取得
 * @param	  gamedata			GAMEDATAへのポインタ
 * @return	BSUBWAY_SCRWORK*
 */
//----------------------------------------------------------
BSUBWAY_SCRWORK * GAMEDATA_GetBSubwayScrWork(GAMEDATA * gamedata)
{
	return gamedata->bsubway_scrwork;
}

//----------------------------------------------------------
/**
 * @brief   バトルサブウェイワークセット
 * @param	  gamedata			GAMEDATAへのポインタ
 * @return	nothing
 */
//----------------------------------------------------------
BSUBWAY_SCRWORK * GAMEDATA_SetBSubwayScrWork(GAMEDATA * gamedata, BSUBWAY_SCRWORK *bsw_scr )
{
	gamedata->bsubway_scrwork = bsw_scr;
}

//==================================================================
/**
 * サポートデータへのポインタ取得
 *
 * @param   gamedata		
 *
 * @retval  COMM_PLAYER_SUPPORT *		
 */
//==================================================================
COMM_PLAYER_SUPPORT * GAMEDATA_GetCommPlayerSupportPtr(GAMEDATA * gamedata)
{
  return gamedata->comm_player_support;
}
