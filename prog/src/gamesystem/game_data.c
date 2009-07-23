//=============================================================================
/**
 * @file	game_data.c
 * @brief	ゲーム進行データ管理
 * @author	tamada
 * @date	2008.11.04
 *
 * 基本的にゲーム進行データはセーブデータとして存在するが、
 * 通信時の複数プレイヤー情報やフィールドマップ情報などセーブデータに
 * のるとは限らない情報もある。
 * そのようなセーブデータ・非セーブデータへ並列にアクセスするインターフェイスを
 * 各パートごとに構成するとパートごとにアクセス手法が違ったり、また同じ機能の
 * ものが複数存在するなどプログラム全体の複雑さが増してしまう可能性が高い。
 * それを避けるため、共通インターフェイスを作ることによって簡略化する試み
 * …試みなので途中で方針を変えるかも。
 */
//=============================================================================

#include <gflib.h>

#include "gamesystem/game_data.h"
#include "gamesystem/playerwork.h"
#include "field/eventdata_system.h"
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/myitem_savedata.h"
#include "poke_tool/pokeparty.h"
#include "savedata/mystatus.h"
#include "savedata/situation.h"
#include "savedata/player_data.h"
#include "gamesystem/pm_season.h"		//季節定義参照
#include "field/field_subscreen.h" //FIELD_SUBSCREEN_ACTION
#include "field/rail_location.h"    //RAIL_LOCATION

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
  LOCATION *special_loc;
  RAIL_LOCATION railLoc;    ///<レール構造時の自分位置

  BAG_CURSOR* bagcursor;  ///< バッグカーソルの管理構造体ポインタ
  MYITEM_PTR myitem;			///<手持ちアイテムセーブデータへのポインタ
  POKEPARTY *my_pokeparty;	///<手持ちポケモンセーブデータへのポインタ
  MMDLSYS *mmdlsys;
  EVENTWORK *eventwork;
  int fieldmap_walk_count; ///<フィールドマップ歩数カウント
  u8 season_id;				///<季節指定ID
  u8 subscreen_mode; ///< フィールド下画面の状態
  u8 subscreen_type;
  u8 frameSpritcount;    ///< フレーム分割動作で動作する場合のカウント
  u8 frameSpritEnable;   ///< フレーム分割動作で動作する場合の許可
  u8 dummy[3];
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
  gd->special_loc = Situation_GetStartLocation(st);

  //レール状況データのクリア
  RAIL_LOCATION_Init(&gd->railLoc);

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
  gd->eventwork = EVENTWORK_AllocWork( heapID );
  EVENTWORK_Init( gd->eventwork );

  //動作モデル
  gd->mmdlsys = MMDLSYS_CreateSystem( heapID, MMDL_MDL_MAX );

  gd->bagcursor = MYITEM_BagCursorAlloc( heapID );
  gd->myitem = SaveControl_DataPtrGet(gd->sv_control_ptr, GMDATA_ID_MYITEM);
  gd->my_pokeparty = SaveControl_DataPtrGet(gd->sv_control_ptr, GMDATA_ID_MYPOKE);

  //歩数カウント
  gd->fieldmap_walk_count = 0;

  if(SaveControl_NewDataFlagGet(gd->sv_control_ptr) == FALSE){
  }

  GAMEDATA_SaveDataLoad(gd);	//セーブデータから必要なものをロードしてくる
  return gd;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void GAMEDATA_Delete(GAMEDATA * gamedata)
{
  GFL_HEAP_FreeMemory(gamedata->bagcursor);
  MMDLSYS_FreeSystem(gamedata->mmdlsys);
  EVENTWORK_FreeWork(gamedata->eventwork);
  EVENTDATA_SYS_Delete(gamedata->evdata);
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

//--------------------------------------------------------------
//--------------------------------------------------------------
const RAIL_LOCATION * GAMEDATA_GetRailLocation(const GAMEDATA * gamedata)
{
  return &gamedata->railLoc;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void GAMEDATA_SetRailLocation(GAMEDATA * gamedata, const RAIL_LOCATION * railLoc)
{
  gamedata->railLoc = * railLoc;
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
  player->mystatus.id = 0;
  player->mystatus.sex = 0;
  player->mystatus.region_code = 0;
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

//==============================================================================
//	ゲームデータが持つ情報を元にセーブ
//==============================================================================
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
  //セーブワークの情報を更新
  GAMEDATA_SaveDataUpdate(gamedata);

  //セーブ実行
  return SaveControl_Save(gamedata->sv_control_ptr);
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

