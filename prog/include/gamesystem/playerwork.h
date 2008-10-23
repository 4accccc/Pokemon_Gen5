//============================================================================================
/**
 * @file	playerwork.h
 * @brief	ポケモンゲームシステム・データ定義
 * @author	tamada GAME FREAK Inc.
 * @date	08.10.22
 *
 */
//============================================================================================

#pragma once
#ifdef	__cplusplus
extern "C" {
#endif

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef u16 ZONEID;

//------------------------------------------------------------------
/**
 * @brief	自分データ定義
 *
 * 最低限のIDとなるデータ。
 * （ビーコンに埋め込まれる等）
 * 現在はこのヘッダで定義しているが将来的には分離されるはず
 *
 */
//------------------------------------------------------------------
typedef struct {
	u32 id;
	u8 sex;
	u8 region_code;
}MYSTATUS;

//------------------------------------------------------------------
/**
 * @brief	自機データ定義
 *
 * 最低限の「自機」をゲーム内に存在させるためのデータ。
 * 通信同期時などで他プレイヤーのデータなども保持する。
 */
//------------------------------------------------------------------
typedef struct {
	ZONEID zoneID;
	VecFx32 position;
	u16 direction;

	MYSTATUS mystatus;
}PLAYER_WORK;

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void PLAYERWORK_init(PLAYER_WORK * player);

extern void PLAYERWORK_setPosition(PLAYER_WORK * player, const VecFx32 * pos);
extern const VecFx32 * PLAYERWORK_getPosition(const PLAYER_WORK * player);
extern void PLAYERWORK_setZoneID(PLAYER_WORK * player, ZONEID zoneid);
extern ZONEID PLAYERWORK_getZoneID(const PLAYER_WORK * player);
extern void PLAYERWORK_setDirection(PLAYER_WORK * player, u16 direction);
extern u16 PLAYERWORK_getDirection(const PLAYER_WORK * player);


#ifdef	__cplusplus
} /* extern "C" */
#endif

