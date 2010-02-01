//==============================================================================
/**
 * @file    intrude_secret_item.c
 * @brief   侵入隠しアイテム
 * @author  matsuda
 * @date    2010.02.01(月)
 */
//==============================================================================
#include <gflib.h>
#include "field/intrude_secret_item.h"
#include "fieldmap/zone_id.h"
#include "gamesystem/game_comm.h"
#include "field\intrude_common.h"
#include "field_comm/intrude_main.h"


//==============================================================================
//  外部データ
//==============================================================================
#include "intrude_secret_item.cdat"



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * 隠しアイテム配置テーブルに載っている座標かチェック
 *
 * @param   zone_id		ゾーンID
 * @param   grid_x		グリッド座標X
 * @param   grid_y		グリッド座標Y
 * @param   grid_z		グリッド座標Z
 *
 * @retval  int		    ヒットした場合：テーブル位置
 * @retval  int		    ヒットしなかった場合：ISC_SEARCH_NONE
 */
//==================================================================
int ISC_TblSearch(u16 zone_id, u16 grid_x, u16 grid_y, u16 grid_z)
{
  int i;
  const INTRUDE_SECRET_ITEM_POSDATA *postbl = IntrudeSecretItemPosDataTbl;
  
  for(i = 0; i < INTRUDE_SECRET_ITEM_POSDATA_MAX; i++){
    if(postbl->zone_id == zone_id && postbl->grid_x == grid_x && postbl->grid_y == grid_y
        && postbl->grid_z == grid_z){
      return i;
    }
    postbl++;
  }
  return ISC_SEARCH_NONE;
}

//==================================================================
/**
 * 隠すアイテムを送信スタックへ載せる
 *
 * @param   gsys		ゲームシステムワークへのポインタ
 * @param   tblno		配置テーブル番号(ISC_TblSearchで取得した値)
 * @param   item		隠すアイテムの番号
 */
//==================================================================
void ISC_ItemSendStack(GAMESYS_WORK *gsys, int tblno, u16 item)
{
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  
  if(intcomm == NULL){
    return; //侵入中ではないため、無視
  }
  
#if 0 //※check　ここで送信スタックへセット
  名前などはGAMEDATAから取得
#endif
}

