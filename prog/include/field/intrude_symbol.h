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
  u32 num:6;                       ///<spoke_arrayの有効なデータが入っている数
  u32 map_level_small:4;           ///<SYMBOL_MAP_LEVEL_SMALL
  u32 map_level_large:4;           ///<SYMBOL_MAP_LEVEL_LARGE
  u32 net_id:4;                    ///<このシンボルデータを持つ相手のNetID
  u32 symbol_map_id:6;             ///<シンボルマップID
  u32 padding:8;
}INTRUDE_SYMBOL_WORK;

///シンボルデータリクエスト構造体
typedef struct{
  u8 symbol_map_id;
  u8 occ;             ///<TRUE:リクエスト有効
  u8 padding[2];
}SYMBOL_DATA_REQ;

///シンボルデータ変更通知構造体
typedef struct{
  u8 symbol_map_id;
  u8 occ;
  u8 padding[2];
}SYMBOL_DATA_CHANGE;


//==============================================================================
//  外部関数宣言
//==============================================================================
extern NetID IntrudeSymbol_CheckIntrudeNetID(GAME_COMM_SYS_PTR game_comm, GAMEDATA *gamedata);
extern BOOL IntrudeSymbol_ReqSymbolData(INTRUDE_COMM_SYS_PTR intcomm, SYMBOL_MAP_ID symbol_map_id);
extern BOOL IntrudeSymbol_CheckRecvSymbolData(INTRUDE_COMM_SYS_PTR intcomm);
extern INTRUDE_SYMBOL_WORK * IntrudeSymbol_GetSymbolBuffer(INTRUDE_COMM_SYS_PTR intcomm);
extern void IntrudeSymbol_SendSymbolDataChange(INTRUDE_COMM_SYS_PTR intcomm, SYMBOL_MAP_ID symbol_map_id);
extern BOOL IntrudeSymbol_CheckSymbolDataChange(INTRUDE_COMM_SYS_PTR intcomm);
