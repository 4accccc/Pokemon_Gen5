//=============================================================================
/**
 * @file	union_beacon_tool.H
 * @brief	ユニオンルーム見た目設定処理
 * @author	Akito Mori
 * @date    	2006.03.16
 */
//=============================================================================
#pragma once

#include "print/wordset.h"


enum{
  UNION_VIEW_INDEX_MAN_START = 0,     ///<見た目INDEX：男開始
  UNION_VIEW_INDEX_MAN_END = 7,       ///<見た目INDEX：男終了
  UNION_VIEW_INDEX_WOMAN_START = 8,   ///<見た目INDEX：女開始
  UNION_VIEW_INDEX_WOMAN_END = 15,    ///<見た目INDEX：女終了
  
  UNION_VIEW_INDEX_MAX = UNION_VIEW_INDEX_WOMAN_END,  ///<見た目INDEX最大数
};


extern int UnionView_GetObjCode(int view_index);
extern int UnionView_GetTrType(int view_index);
extern int UnionView_GetMsgType(int view_index);
extern u16 *UnionView_GetPalNo( u16 * table, int sex, int view_type );
extern u16 *UnionView_PalleteTableAlloc( int heapID );
extern int UnionView_Objcode_to_Index(u16 objcode);

