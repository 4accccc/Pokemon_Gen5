//======================================================================
/**
 * @file	playerwork.h
 * @brief	ポケモンゲームシステム・データ定義
 * @author	tamada GAME FREAK Inc.
 * @date	08.10.22
 */
//======================================================================
#pragma once
#ifdef	__cplusplus
extern "C" {
#endif

#include "gamedata_def.h"

#include "savedata/mystatus.h"
#include "savedata/mystatus_local.h"
#include "field/location.h"

//======================================================================
//  define
//======================================================================
typedef u16 ZONEID; ///<ZONEID

enum {
	PLAYER_MAX = 5,
	
	PLAYER_ID_MINE = PLAYER_MAX - 1,    ///<自分自身のプレイヤーID
};

//--------------------------------------------------------------
/// PLAYER_MOVE_FORM
//--------------------------------------------------------------
typedef enum
{
  PLAYER_MOVE_FORM_NORMAL = 0,   ///<自機二足歩行
  PLAYER_MOVE_FORM_CYCLE,  ///<自機自転車移動
  PLAYER_MOVE_FORM_SWIM, ///<自機波乗り移動
  PLAYER_MOVE_FORM_MAX,
}PLAYER_MOVE_FORM;

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/**
 * @brief	PLAYER_WORK 自機データ定義
 * @note 最低限の「自機」をゲーム内に存在させるためのデータ。
 * 通信同期時などで他プレイヤーのデータなども保持する。
 */
//--------------------------------------------------------------
typedef struct {
	ZONEID zoneID;
	VecFx32 position;
  RAIL_LOCATION railposition;
	
  u16 direction;        ///<回転角度のRadian値　（0～0xffff）
  u8 palace_area;       ///<※check　パレステストの為、一時的にここに配置
  u8 pos_type;       ///<LOCATION_POS_TYPE

  u8 objcode_fix; ///<OBJコード変更不可
  u8 padding[3]; ///<4byte補正余り
  
	MYSTATUS mystatus;
  PLAYER_MOVE_FORM move_form;
}PLAYER_WORK;

//======================================================================
//  extern
//======================================================================
//------------------------------------------------------------------
/**
 * @brief	プレイヤーデータを取得する
 * @param	gamedata	GAMEDATAへのポインタ
 * @param	player_id	プレイヤー指定ID
 * @return	PLAYER_WORK	プレイヤーデータへのポインタ
 */
//------------------------------------------------------------------
extern PLAYER_WORK * GAMEDATA_GetPlayerWork(GAMEDATA * gamedata, u32 player_id);
//------------------------------------------------------------------
/**
 * @brief	自分のプレイヤーデータを取得する
 * @param	gamedata	GAMEDATAへのポインタ
 * @return	PLAYER_WORK	自分のプレイヤーデータへのポインタ
 */
//------------------------------------------------------------------
extern PLAYER_WORK * GAMEDATA_GetMyPlayerWork(GAMEDATA * gamedata);

//--------------------------------------------------------------
/**
 * @brief	プレイヤーワークの初期化
 * @param	player	PLAYER_WORKへのポインタ
 */
//--------------------------------------------------------------
extern void PLAYERWORK_init(PLAYER_WORK * player);

//--------------------------------------------------------------
/**
 * @brief	位置をセットする
 * @param	player	PLAYER_WORKへのポインタ
 * @param	pos		位置を指定する値
 */
//--------------------------------------------------------------
extern void PLAYERWORK_setPosition(PLAYER_WORK * player, const VecFx32 * pos);

//--------------------------------------------------------------
/**
 * @brief	位置を取得する
 * @param	player	PLAYER_WORKへのポインタ
 * @return	VecFx32* 位置を指定するVecFx32へのポインタ
 */
//--------------------------------------------------------------
extern const VecFx32 * PLAYERWORK_getPosition(const PLAYER_WORK * player);

//--------------------------------------------------------------
/**
 * @brief	レール座標をセットする
 * @param	player	PLAYER_WORKへのポインタ
 * @param	railpos		レール座標を指定する値
 */
//--------------------------------------------------------------
extern void PLAYERWORK_setRailPosition(PLAYER_WORK * player, const RAIL_LOCATION * railpos);

//--------------------------------------------------------------
/**
 * @brief	レール座標を取得する
 * @param	player	PLAYER_WORKへのポインタ
 * @return	LOCATION_RAILPOS レール座標を指定するポインタ
 */
//--------------------------------------------------------------
extern const RAIL_LOCATION * PLAYERWORK_getRailPosition(const PLAYER_WORK * player);

//--------------------------------------------------------------
/**
 * @brief	座標タイプを設定する
 * @param	player	PLAYER_WORKへのポインタ
 * @param	pos_type 座標タイプ LOCATION_POS_TYPE
 */
//--------------------------------------------------------------
extern void PLAYERWORK_setPosType(PLAYER_WORK * player, LOCATION_POS_TYPE pos_type);

//--------------------------------------------------------------
/**
 * @brief	座標タイプを取得する
 * @param	player	PLAYER_WORKへのポインタ
 * @return	LOCATION_POS_TYPE 座標タイプ
 */
//--------------------------------------------------------------
extern LOCATION_POS_TYPE PLAYERWORK_getPosType(const PLAYER_WORK * player);

//--------------------------------------------------------------
/**
 * @brief	位置を取得する
 * @param	player	PLAYER_WORKへのポインタ
 * @param	zoneid	マップを指定する値
 */
//--------------------------------------------------------------
extern void PLAYERWORK_setZoneID(PLAYER_WORK * player, ZONEID zoneid);

//--------------------------------------------------------------
/**
 * @brief	位置を取得する
 * @param	player	PLAYER_WORKへのポインタ
 * @param	zoneid	マップを指定する値
 */
//--------------------------------------------------------------
extern void PLAYERWORK_setZoneID(PLAYER_WORK * player, ZONEID zoneid);

//--------------------------------------------------------------
/**
 * @brief	位置を取得する
 * @param	player	PLAYER_WORKへのポインタ
 * @return	ZONEID	マップを指定する値
 */
//--------------------------------------------------------------
extern ZONEID PLAYERWORK_getZoneID(const PLAYER_WORK * player);

//--------------------------------------------------------------
/**
 * @brief	方向（角度）を設定する
 * @param	player		PLAYER_WORKへのポインタ
 * @param	direction	方向を指定する値(ラジアン角度0～0xffff)
 */
//--------------------------------------------------------------
extern void PLAYERWORK_setDirection(PLAYER_WORK * player, u16 direction);

//--------------------------------------------------------------
/**
 * @brief	方向（角度）を取得する
 * @param	player	PLAYER_WORKへのポインタ
 * @return	方向を指定する値(ラジアン角度0～0xffff)
 */
//--------------------------------------------------------------
extern u16 PLAYERWORK_getDirection(const PLAYER_WORK * player);

//--------------------------------------------------------------
/**
 * @brief	方向タイプで方向角度を設定する
 * @param	player		PLAYER_WORKへのポインタ
 * @param	dir_type	4方向を指定する値(DIR_UP DIR_DOWN DIR_LEFT DIR_RIGHT)
 */
//--------------------------------------------------------------
extern void PLAYERWORK_setDirection_Type(PLAYER_WORK * player, u16 dir_type);

//--------------------------------------------------------------
/**
 * @brief	方向角度の方向タイプを取得する
 * @param	player	PLAYER_WORKへのポインタ
 * @return	方向を指定する値(DIR_UP DIR_DOWN DIR_LEFT DIR_RIGHT)
 */
//--------------------------------------------------------------
extern u16 PLAYERWORK_getDirection_Type(const PLAYER_WORK * player);

//--------------------------------------------------------------
/**
 * @brief	パレスエリア番号を設定する
 * @param	player	      PLAYER_WORKへのポインタ
 * @param	palace_area   パレス番号
 * 
 * ※check　パレステストの為の暫定関数
 */
//--------------------------------------------------------------
extern void PLAYERWORK_setPalaceArea(PLAYER_WORK * player, u8 palace_area);

//--------------------------------------------------------------
/**
 * @brief	パレスエリア番号を取得する
 * @param	player	PLAYER_WORKへのポインタ
 * @return	パレスエリア番号
 * 
 * ※check　パレステストの為の暫定関数
 */
//--------------------------------------------------------------
extern u8 PLAYERWORK_getPalaceArea(const PLAYER_WORK * player);

//--------------------------------------------------------------
/**
 * PLAYER_MOVE_FORM取得
 * @param player PLAYER_WORKへのポインタ
 * @retval PLAYER_MOVE_FORM
 */
//--------------------------------------------------------------
extern PLAYER_MOVE_FORM PLAYERWORK_GetMoveForm( const PLAYER_WORK *player );

//--------------------------------------------------------------
/**
 * PLAYER_MOVE_FORMセット
 * @param player PLAYER_WORKへのポインタ
 * @param form PLAYER_MOVE_FORM
 * @retval none
 */
//--------------------------------------------------------------
extern void PLAYERWORK_SetMoveForm( PLAYER_WORK *player, PLAYER_MOVE_FORM form );

//--------------------------------------------------------------
/**
 * @brief 自機OBJコード固定フラグをセット
 * @param player PLAYER_WORKへのポインタ
 * @param flag TRUE=固定 FALSE=可変
 * @retval none
 */
//--------------------------------------------------------------
extern void PLAYERWORK_SetFlagOBJCodeFix( PLAYER_WORK *player, BOOL flag );

  //--------------------------------------------------------------
/**
 * @brief 自機OBJコード固定フラグをチェック
 * @param player PLAYER_WORKへのポインタ
 * @retval BOOL TRUE=固定 FALSE=可変
 */
//--------------------------------------------------------------
extern BOOL PLAYERWORK_GetFlagOBJCodeFix( const PLAYER_WORK *player );


#ifdef	__cplusplus
} /* extern "C" */
#endif
