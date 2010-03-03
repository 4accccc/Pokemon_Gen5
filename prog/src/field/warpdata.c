//============================================================================================
/**
 * @file	warpdata.c
 * @brief	ワープ用データ関連ソース
 * @since	2005.12.19
 * 
 * 2009.09.20 HGSSから移植開始
 */
//============================================================================================

#include <gflib.h>

#include "gamesystem/game_data.h"
#include "warpdata.h"

#include "field/location.h"

#include "arc/fieldmap/zone_id.h" //ZONE_ID_～

#include "field/field_const.h"  //GRID_TO_FX32

#include "field/field_dir.h"    //DIR_～

//#include "fieldobj_code.h"		//DIR_UP DIR_DOWN
//#include "ev_mapchange.h"		//DOOR_ID_JUMP_CODE

//SYS_FLAG_ARRIVE_～
#include "../../../resource/fldmapdata/flagwork/flag_define.h"


//自動生成されるはずのシンボル定義
#include "../../../resource/fldmapdata/warpdata/warpdata.h"

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	u16	arrive_flag;	//到着フラグ

	u16	IsTeleportPos:1;
	u16	AutoSetArriveFlag:1;
	u16	:14;

	u16	room_id;	//全滅時の戻り先ZoneID
	u16	room_gx:8;	//全滅時の戻り先X
	u16	room_gz:8;	//全滅時の戻り先Z
	
	u16	fld_id;		//ワープ座標ZoneID
	u16 fld_gx;		//ワープ座標X
	u16	fld_gz;		//ワープ座標Z

	u16	escape_id;	//出口座標ZoneID
	u16 escape_gx;	//出口座標X
	u16	escape_gz;	//出口座標Z
}WARPDATA;

SDK_COMPILER_ASSERT(sizeof(WARPDATA) == 20);

//自動生成されるはずのデータ定義
#include "../../../resource/fldmapdata/warpdata/warpdata.cdat"


static void set_location(LOCATION * loc, u16 zone_id, u16 dir, u16 gx, u16 gz);
static int regulate_warp_id(int warp_id);
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ワープIDの初期値を取得する
 */
//------------------------------------------------------------------
int WARPDATA_GetInitializeID(void)
{
	return 1;
}

//------------------------------------------------------------------
/**
 * @brief	ワープ場所の取得
 * @param	warp_id		ワープID
 * @param	loc			場所を受け取るLOCATIONへのポインタ
 */
//------------------------------------------------------------------
void WARPDATA_GetWarpLocation(int warp_id, LOCATION * loc)
{
	warp_id = regulate_warp_id(warp_id);

  set_location( loc,
      WarpData[warp_id].fld_id,
      DIR_DOWN, 
      WarpData[warp_id].fld_gx,
      WarpData[warp_id].fld_gz );
}

//------------------------------------------------------------------
/**
 * @brief	復活場所の取得
 * @param	warp_id		ワープID
 * @param	loc			場所を受け取るLOCATIONへのポインタ
 */
//------------------------------------------------------------------
void WARPDATA_GetRevivalLocation(int warp_id, LOCATION * loc)
{
	warp_id = regulate_warp_id(warp_id);

  set_location( loc,
      WarpData[warp_id].room_id,
      DIR_UP, 
      WarpData[warp_id].room_gx,
      WarpData[warp_id].room_gz );
}

//------------------------------------------------------------------
/**
 * @brief	出口座標の取得(Escapeフラグのないポイントなら空飛ぶ座標を返す)
 * @param	warp_id		ワープID
 * @param	loc			場所を受け取るLOCATIONへのポインタ
 */
//------------------------------------------------------------------
void WARPDATA_GetEscapeLocation(int warp_id, LOCATION * loc)
{
	int id = regulate_warp_id(warp_id);

  set_location( loc,
      WarpData[warp_id].escape_id,
      DIR_DOWN, 
      WarpData[warp_id].escape_gx,
      WarpData[warp_id].escape_gz );
}

//------------------------------------------------------------------
/**
 * @brief	ワープIDの取得
 * @param	zone_id		検索する屋内のゾーンID
 * @retval	int			ワープID（１オリジン）
 * @retval	0			見つからなかった
 */
//------------------------------------------------------------------
int WARPDATA_SearchByRoomID(int zone_id)
{
	int i;
	for (i = 0; i < NELEMS(WarpData); i++) {
		if (WarpData[i].room_id == zone_id
				&& WarpData[i].IsTeleportPos) {
			return i + 1;
		}
	}
	return 0;
}

//------------------------------------------------------------------
/**
 * @brief	ワープIDの取得
 * @param	zone_id		検索するフィールドのゾーンID
 * @retval	int			ワープID（１オリジン）
 * @retval	0			見つからなかった
 */
//------------------------------------------------------------------
int WARPDATA_SearchByFieldID(int zone_id)
{
	int i;
	for (i = 0; i < NELEMS(WarpData); i++) {
		if (WarpData[i].fld_id == zone_id
				&& WarpData[i].IsTeleportPos) {
			return i + 1;
		}
	}
	return 0;
}

//------------------------------------------------------------------
/**
 * @brief	ワープIDの取得（「そらをとぶ」用）
 * @param	zone_id		検索するフィールドのゾーンID
 * @retval	int			ワープID（１オリジン）
 * @retval	0			見つからなかった
 */
//------------------------------------------------------------------
int WARPDATA_SearchForFly(int zone_id)
{
	int i;
	int id = 0;
	for (i = 0; i < NELEMS(WarpData); i++) {
		if (WarpData[i].fld_id == zone_id) {
			return (i+1);
		}
	}
	return id;
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	到着フラグのセット
 * @param gamedata  ゲームデータへのポインタ
 * @param	zone_id		セットする場所のゾーンID
 *
 * @note
 * 過去作ではArriveFlagの扱いは特殊だったが、
 * 今回からは単なるシステムフラグとして扱うようにする
 */
//------------------------------------------------------------------
void ARRIVEDATA_SetArriveFlag( GAMEDATA * gamedata, int zone_id)
{
	int i;
	for (i = 0; i < NELEMS(WarpData); i++) {
		//OS_Printf("zone = %d, WarpZone = %d\n",zone_id,WarpData[i].fld_id);
		if (WarpData[i].room_id == zone_id && WarpData[i].AutoSetArriveFlag) {
			EVENTWORK_SetEventFlag(GAMEDATA_GetEventWork( gamedata ), WarpData[i].arrive_flag);
      TAMADA_Printf("Arrive Flag Set: %x\n", WarpData[i].arrive_flag );
			return;
		}
	}
}

//------------------------------------------------------------------
/**
 * @brief	到着フラグを参照する
 * @param gamedata  ゲームデータへのポインタ
 * @param	warp_id		対象となるマップのワープID
 * @retval	BOOL		TRUEのとき、来たことがある
 *
 * @note
 * 過去作ではArriveFlagの扱いは特殊だったが、
 * 今回からは単なるシステムフラグとして扱うようにする
 */
//------------------------------------------------------------------
BOOL ARRIVEDATA_GetArriveFlag(GAMEDATA * gamedata, int warp_id)
{
	int reg_id = regulate_warp_id(warp_id);
	return EVENTWORK_CheckEventFlag(GAMEDATA_GetEventWork( gamedata ), WarpData[reg_id].arrive_flag);
}


//============================================================================================
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief LOCATIONのセットアップ
 * @param loc       セットするLOCATIONへのポインタ
 * @param zone_id   出現先のゾーン指定ID
 * @param dir       向き
 * @param gx        出現するX位置（グリッド単位）
 * @param gz        出現するZ位置（グリッド単位）
 *
 * @todo  dirの値の定義に取り決めがない
 * @todo  Y方向データも設定するべきかどうか？
 */
//------------------------------------------------------------------
static void set_location(LOCATION * loc, u16 zone_id, u16 dir, u16 gx, u16 gz)
{
  fx32 x, y, z;
  x = GRID_TO_FX32( gx );
  y = 0;
  z = GRID_TO_FX32( gz );

  LOCATION_SetDirect(loc, zone_id, DIR_DOWN, x, y, z);
}

//------------------------------------------------------------------
/**
 * @brief	ワープIDの正規化
 * @param	warp_id		ワープID
 * @return	int			正規化したワープID
 *
 * 不正な値は変換する。1オリジンを0オリジンに変換する。
 */
//------------------------------------------------------------------
static int regulate_warp_id(int warp_id)
{
	if(warp_id <= 0 || warp_id > NELEMS(WarpData)) {
		GF_ASSERT_MSG(0, "不正なワープID（%d）です。\n", warp_id);
		warp_id = 1;		//製品版では不正な値でも動作するように補正する
	}
	warp_id --;		//1 origin --> 0 origin
	return warp_id;
}


