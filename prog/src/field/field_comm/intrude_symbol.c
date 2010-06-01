//==============================================================================
/**
 * @file    intrude_symbol.c
 * @brief   侵入：シンボルポケモン
 * @author  matsuda
 * @date    2010.03.04(木)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "field/intrude_comm.h"
#include "intrude_comm_command.h"
#include "intrude_types.h"
#include "field/intrude_symbol.h"
#include "intrude_work.h"
#include "field/fieldmap.h"
#include "intrude_field.h"
#include "field/zonedata.h"


//==============================================================================
//  データ
//==============================================================================
///パレスと森の出入り口座標(パレス側)
static const VecFx32 PalaceForestDoorwayPos = {FX32_CONST(504), FX32_CONST(32), FX32_CONST(360)};



//==================================================================
/**
 * 自分の侵入先のNetIDを取得する
 *
 * @param   game_comm		
 * @param   gamedata		
 *
 * @retval  NetID		    侵入先のNetID
 *                      非通信 OR 自分のROMにいる場合は INTRUDE_NETID_NULL
 *
 * 通信エラーが起きていても侵入先でエラーになっている場合は引き続き侵入先のNetIDを返します
 */
//==================================================================
NetID IntrudeSymbol_CheckIntrudeNetID(GAME_COMM_SYS_PTR game_comm, GAMEDATA *gamedata)
{
  INTRUDE_COMM_SYS_PTR intcomm;
  NetID my_netid = GAMEDATA_GetIntrudeMyID(gamedata);
  
  intcomm = Intrude_Check_CommConnect(game_comm);
  if(NetErr_App_CheckError()){
    NetID invasion_netid = GAMEDATA_GetIntrudePalaceArea(gamedata);
    if(invasion_netid == my_netid){
      return INTRUDE_NETID_NULL;
    }
    return invasion_netid;
  }
  else if(intcomm != NULL){
    if(intcomm->intrude_status_mine.palace_area == my_netid){
      return INTRUDE_NETID_NULL;
    }
    return intcomm->intrude_status_mine.palace_area;
  }
  else{
    return INTRUDE_NETID_NULL;
  }
}

//==================================================================
/**
 * パレスと森の出入り口座標を取得(パレス側)
 *
 * @param   game_comm		
 * @param   gamedata		
 * @param   dest_pos		座標代入先
 */
//==================================================================
void IntrudeSymbol_GetPosPalaceForestDoorway(GAME_COMM_SYS_PTR game_comm, GAMEDATA *gamedata, VecFx32 *dest_pos)
{
  INTRUDE_COMM_SYS_PTR intcomm = Intrude_Check_CommConnect(game_comm);
  int map_offset;
  
  if(intcomm == NULL 
      || intcomm->intrude_status_mine.palace_area == GAMEDATA_GetIntrudeMyID(gamedata)){
    map_offset = 0;
  }
  else{
    map_offset = intcomm->intrude_status_mine.palace_area - GAMEDATA_GetIntrudeMyID(gamedata);
    if(map_offset < 0){
      map_offset = intcomm->member_num + map_offset;
    }
  }
  
  *dest_pos = PalaceForestDoorwayPos;
  dest_pos->x += PALACE_MAP_LEN * map_offset;
}

//==================================================================
/**
 * 侵入先の相手に対して、シンボルエンカウント用データを要求する
 *
 * @param   intcomm		  
 * @param   zone_type		相手へ要求するSYMBOL_ZONE_TYPE
 * @param   map_no		  相手へ要求するマップ番号(キープゾーンの場合は0固定)
 *
 * @retval  BOOL		    TRUE:送信成功　FALSE:送信失敗
 */
//==================================================================
BOOL IntrudeSymbol_ReqSymbolData(INTRUDE_COMM_SYS_PTR intcomm, SYMBOL_MAP_ID symbol_map_id)
{
  SYMBOL_DATA_REQ sdr;
  
  sdr.symbol_map_id = symbol_map_id;
  sdr.occ = TRUE;
  return IntrudeSend_SymbolDataReq(intcomm, intcomm->intrude_status_mine.palace_area, &sdr);
}

//==================================================================
/**
 * IntrudeSymbol_ReqSymbolDataで要求したデータを受信したかチェック
 *
 * @param   intcomm		
 *
 * @retval  BOOL		TRUE:受信した　FALSE:受信していない
 *
 * この関数でTRUE確認後、IntrudeSymbol_GetSymbolBufferで受信したデータが入っているバッファの
 * ポインタを取得できます
 */
//==================================================================
BOOL IntrudeSymbol_CheckRecvSymbolData(INTRUDE_COMM_SYS_PTR intcomm)
{
  return intcomm->recv_symbol_flag;
}

//==================================================================
/**
 * 侵入先のシンボルデータ受信バッファのポインタを取得
 *
 * @param   intcomm		
 *
 * @retval  INTRUDE_SYMBOL_WORK *		
 */
//==================================================================
INTRUDE_SYMBOL_WORK * IntrudeSymbol_GetSymbolBuffer(INTRUDE_COMM_SYS_PTR intcomm)
{
  GF_ASSERT(intcomm->recv_symbol_flag == TRUE);
  return &intcomm->intrude_symbol;
}

//==================================================================
/**
 * 自分のシンボルデータに変更が発生した為、みんなに通知する
 *
 * @param   intcomm		
 * @param   symbol_map_id		自分が変更したシンボルマップID
 *
 * ポケモンの移動：SymbolSave_Field_Move_FreeToKeep or SymbolSave_Field_Move_KeepToFree
 * ポケモンの捕獲：SymbolSave_DataShift
 * 上記のいずれかを行った場合、通信相手にもそれを通知する為に、この命令を使って送信してください
 * これで通信相手に知らせる事で相手側がIntrudeSymbol_CheckSymbolDataChangeで検知する事が出来ます
 */
//==================================================================
void IntrudeSymbol_SendSymbolDataChange(INTRUDE_COMM_SYS_PTR intcomm, SYMBOL_MAP_ID symbol_map_id)
{
  intcomm->send_symbol_change.symbol_map_id = symbol_map_id;
  intcomm->send_symbol_change.occ = TRUE;
}

//==================================================================
/**
 * 侵入先のシンボルデータに変更が無いか調べる
 *    ※自分が現在いるシンボルマップに変更が発生しない限りはTRUEにはなりません。
 *      自分がいる場所とは違うマップで変更が発生した場合はFALSEのままです
 *
 * @param   intcomm		
 *
 * @retval  TRUE:変更があった(IntrudeSymbol_ReqSymbolDataを使い、最新のデータを取得しなおしてください)
 * @retval  FALSE:変更は無い
 *
 * 一度この関数でTRUEを取得後、内部フラグをリセットする為、新たな通知が無い限りFALSEを返します
 */
//==================================================================
BOOL IntrudeSymbol_CheckSymbolDataChange(INTRUDE_COMM_SYS_PTR intcomm, u16 zone_id)
{
  BOOL ret = intcomm->recv_symbol_change_flag;
  intcomm->recv_symbol_change_flag = FALSE;
  if(ret == TRUE && ZONEDATA_IsBingo( zone_id ) == TRUE){
    return TRUE;
  }
  return FALSE;
}
