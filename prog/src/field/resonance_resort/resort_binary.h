//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_binary.h
 *	@brief  レゾナンスリゾートバイナリデータ読み込み
 *	@author	Toru=Nagihashi
 *	@date		2011.02.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "field/resonance_resort_data.h"
#include "resort_binary_st.h"
//=============================================================================
/**
 *					レゾナンスリゾート人物バイナリデータ
*/
//=============================================================================
//-------------------------------------
///	人物要素定義  値はコンバータから吐き出されるので型を付けてあげる
//=====================================
typedef s32 RESORT_PEOPLE_TYPE;
typedef s32 RESORT_PERSONALITY_TYPE;

typedef s32 RESORT_KIND_TYPE; //以下のタイプすべてが格納される
typedef s32 RESORT_RESIDENT_TYPE;
typedef s32 RESORT_PROXY_TYPE;
typedef s32 RESORT_TURIST_TYPE;
typedef s32 RESORT_SECRETARY_TYPE;
typedef s32 RESORT_VISITOR_TYPE;
typedef s32 RESORT_NPC_TYPE;

//-------------------------------------
///	人物バイナリデータワーク
//=====================================
typedef struct _RESORT_BINARY_PEOPLE RESORT_BINARY_PEOPLE;

//-------------------------------------
///	人物バイナリデータ 各人物分のワーク
//=====================================
typedef struct _RESORT_BINARY_PEOPLE_DATA RESORT_BINARY_PEOPLE_DATA;

//-------------------------------------
/// PUBLIC	
//=====================================
//バイナリ読み込み、破棄
extern RESORT_BINARY_PEOPLE * RESORT_BINARY_PEOPLE_Init( HEAPID heapID );
extern void RESORT_BINARY_PEOPLE_Exit( RESORT_BINARY_PEOPLE *p_wk );


extern const RESORT_BINARY_PEOPLE_DATA * RESORT_BINARY_SearchData( const RESORT_BINARY_PEOPLE *cp_wk, RESORT_PEOPLE_TYPE people_type, RESORT_KIND_TYPE kind );

//バイナリからデータ取得
extern u16 RESORT_BINARY_PEOPLE_DATA_GetScrID( const RESORT_BINARY_PEOPLE_DATA *cp_wk );
extern u16 RESORT_BINARY_PEOPLE_DATA_GetMsgID( const RESORT_BINARY_PEOPLE_DATA *cp_wk, u16 param );


//=============================================================================
/**
 *					レゾナンスリゾートアピール情報バイナリデータ
*/
//=============================================================================
#define RESORT_BINARY_SPOT_ANSWER_MAX (6)
#define RESORT_BINARY_SPOT_ANSWER_NUM (3)
//-------------------------------------
///	列挙型
//=====================================
typedef enum
{
  RESORT_BINARY_SPOT_PARAM_MSG_00,
  RESORT_BINARY_SPOT_PARAM_MSG_01,
  RESORT_BINARY_SPOT_PARAM_MSG_02,

  RESORT_BINARY_SPOT_PARAM_BAESEXP,

  RESORT_BINARY_SPOT_DATA_START,

  RESORT_BINARY_SPOT_PARAM_TYPE = RESORT_BINARY_SPOT_DATA_START,
  RESORT_BINARY_SPOT_PARAM_DATA,
  RESORT_BINARY_SPOT_PARAM_GRADE,

  RESORT_BINARY_SPOT_DATA_END = RESORT_BINARY_SPOT_DATA_START+RESORT_BINARY_SPOT_ANSWER_NUM*RESORT_BINARY_SPOT_ANSWER_MAX-1,

  RESORT_BINARY_SPOT_PARAM_MAX,
}RESORT_BINARY_SPOT_PARAM;

//-------------------------------------
///	アピールデータ
//=====================================
typedef struct _RESORT_BINARY_SPOT RESORT_BINARY_SPOT;

//-------------------------------------
/// PUBLIC	
//=====================================
extern RESORT_BINARY_SPOT * RESORT_BINARY_SPOT_Init( HEAPID heapID );
extern void RESORT_BINARY_SPOT_Exit( RESORT_BINARY_SPOT *p_wk );
extern u16 RESORT_BINARY_SPOT_GetData( const RESORT_BINARY_SPOT *cp_wk, u32 index, RESORT_BINARY_SPOT_PARAM param );
extern BOOL RESORT_BINARY_SPOT_SearchIndex( const RESORT_BINARY_SPOT *cp_wk, u32 msg, u32 *p_index );
