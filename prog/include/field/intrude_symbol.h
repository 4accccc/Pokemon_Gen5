//==============================================================================
/**
 * @file    intrude_symbol.h
 * @brief   侵入：シンボルポケモン
 * @author  matsuda
 * @date    2010.03.04(木)
 */
//==============================================================================
#pragma once

#include "savedata/symbol_save.h"



//==============================================================================
//  構造体定義
//==============================================================================
///通信でやり取りする相手のシンボルマップパラメータ(1マップ分)
typedef struct{
  SYMBOL_POKEMON spoke_array[SYMBOL_MAP_STOCK_MAX];   ///<1マップ分のシンボルポケモンデータの配列ポインタ
  u8 num:6;                       ///<spoke_arrayの有効なデータが入っている数
  u8 zone_type:2;                 ///<SYMBOL_ZONE_TYPE
  u8 map_level_small:4;           ///<SYMBOL_MAP_LEVEL_SMALL
  u8 map_level_large:4;           ///<SYMBOL_MAP_LEVEL_LARGE
  u8 net_id:4;                    ///<このシンボルデータを持つ相手のNetID
  u8 map_no:4;                    ///<マップ番号
  u8 padding;
}INTRUDE_SYMBOL_WORK;

///シンボルデータリクエスト構造体
typedef struct{
  u8 zone_type;
  u8 map_no;
  u8 occ;             ///<TRUE:リクエスト有効
  u8 padding;
}SYMBOL_DATA_REQ;

///シンボルデータ変更通知構造体
typedef struct{
  u8 zone_type;
  u8 map_no;
  u8 occ;
  u8 padding;
}SYMBOL_DATA_CHANGE;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern BOOL IntrudeSymbol_ReqSymbolData(INTRUDE_COMM_SYS_PTR intcomm, SYMBOL_ZONE_TYPE zone_type, u8 map_no);
extern BOOL IntrudeSymbol_CheckRecvSymbolData(INTRUDE_COMM_SYS_PTR intcomm);
extern INTRUDE_SYMBOL_WORK * IntrudeSymbol_GetSymbolBuffer(INTRUDE_COMM_SYS_PTR intcomm);
extern void IntrudeSymbol_SendSymbolDataChange(INTRUDE_COMM_SYS_PTR intcomm, SYMBOL_ZONE_TYPE zone_type, u8 map_no);
extern BOOL IntrudeSymbol_CheckSymbolDataChange(INTRUDE_COMM_SYS_PTR intcomm);
