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
BOOL IntrudeSymbol_ReqSymbolData(INTRUDE_COMM_SYS_PTR intcomm, SYMBOL_ZONE_TYPE zone_type, u8 map_no)
{
  SYMBOL_DATA_REQ sdr;
  
  sdr.zone_type = zone_type;
  sdr.map_no = map_no;
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
 * @param   zone_type		自分が変更したSYMBOL_ZONE_TYPE
 * @param   map_no		  自分が変更したマップ番号(キープゾーンの場合は0固定)
 *
 * ポケモンの移動：SymbolSave_Field_Move_FreeToKeep or SymbolSave_Field_Move_KeepToFree
 * ポケモンの捕獲：SymbolSave_DataShift
 * 上記のいずれかを行った場合、通信相手にもそれを通知する為に、この命令を使って送信してください
 * これで通信相手に知らせる事で相手側がIntrudeSymbol_CheckSymbolDataChangeで検知する事が出来ます
 */
//==================================================================
void IntrudeSymbol_SendSymbolDataChange(INTRUDE_COMM_SYS_PTR intcomm, SYMBOL_ZONE_TYPE zone_type, u8 map_no)
{
  intcomm->send_symbol_change.zone_type = zone_type;
  intcomm->send_symbol_change.map_no = map_no;
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
BOOL IntrudeSymbol_CheckSymbolDataChange(INTRUDE_COMM_SYS_PTR intcomm)
{
  BOOL ret = intcomm->recv_symbol_change_flag;
  intcomm->recv_symbol_change_flag = FALSE;
  return ret;
}
