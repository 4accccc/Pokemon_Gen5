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

//=============================================================================
/**
 *      レゾナンスリゾートNPCデータ
 */
//=============================================================================
//-------------------------------------
///	列挙型
//=====================================
typedef enum
{
  RESORT_BINARY_NPC_PARAM_VIEW,
  RESORT_BINARY_NPC_PARAM_PLACE,
  RESORT_BINARY_NPC_PARAM_GRID_X,
  RESORT_BINARY_NPC_PARAM_GRID_Z,
  RESORT_BINARY_NPC_PARAM_CITY_LV,
  RESORT_BINARY_NPC_PARAM_MOVE_CODE,
  RESORT_BINARY_NPC_PARAM_LIMIT_X,
  RESORT_BINARY_NPC_PARAM_LIMIT_Z,
  RESORT_BINARY_NPC_PARAM_TYPE,
  RESORT_BINARY_NPC_PARAM_KIND,
  RESORT_BINARY_NPC_PARAM_DUMMY,
  RESORT_BINARY_NPC_PARAM_MAX
} RESORT_BINARY_NPC_PARAM;

//-------------------------------------
///	NPCデータ
//=====================================
typedef struct _RESORT_BINARY_NPC RESORT_BINARY_NPC;

//-------------------------------------
///	PUBLIC
//=====================================
extern RESORT_BINARY_NPC * RESORT_BINARY_NPC_Init( HEAPID heapID );
extern void RESORT_BINARY_NPC_Exit( RESORT_BINARY_NPC *p_wk );
extern u16 RESORT_BINARY_NPC_GetData( const RESORT_BINARY_NPC *cp_wk, u32 index, RESORT_BINARY_NPC_PARAM param );
extern u32 RESORT_BINARY_NPC_GetMax( const RESORT_BINARY_NPC *cp_wk );

//=============================================================================
/**
 *      レゾナンスリゾートMOUTHCOMMデータ
 */
//=============================================================================
//-------------------------------------
///	列挙型
//=====================================
typedef enum
{
  RESORT_BINARY_MOUTHCOMM_PARAM_QUESTION,
  RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_NUM,
  RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_00,
  RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_01,
  RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_02,
  RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_03,
  RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_04,
  RESORT_BINARY_MOUTHCOMM_PARAM_ANSWER_05,

  RESORT_BINARY_MOUTHCOMM_PARAM_MAX
} RESORT_BINARY_MOUTHCOMM_PARAM;

//-------------------------------------
///	MOUTHCOMMデータ
//=====================================
typedef struct _RESORT_BINARY_MOUTHCOMM RESORT_BINARY_MOUTHCOMM;

//-------------------------------------
///	PUBLIC
//=====================================
extern RESORT_BINARY_MOUTHCOMM * RESORT_BINARY_MOUTHCOMM_Init( HEAPID heapID );
extern void RESORT_BINARY_MOUTHCOMM_Exit( RESORT_BINARY_MOUTHCOMM *p_wk );
extern u16 RESORT_BINARY_MOUTHCOMM_GetData( const RESORT_BINARY_MOUTHCOMM *cp_wk, u32 index, RESORT_BINARY_MOUTHCOMM_PARAM param );


//=============================================================================
/**
 *      レゾナンスリゾートOFFRECOデータ
 */
//=============================================================================
//-------------------------------------
///	列挙型
//=====================================
typedef enum
{
  RESORT_BINARY_OFFRECO_PARAM_BIT,
  RESORT_BINARY_OFFRECO_PARAM_GROUP,
  RESORT_BINARY_OFFRECO_PARAM_TYPE,
  RESORT_BINARY_OFFRECO_PARAM_MSG_00,
  RESORT_BINARY_OFFRECO_PARAM_MSG_01,
  RESORT_BINARY_OFFRECO_PARAM_MSG_02,
  RESORT_BINARY_OFFRECO_PARAM_MSG_03,
  RESORT_BINARY_OFFRECO_PARAM_MSG_04,
  RESORT_BINARY_OFFRECO_PARAM_MSG_05,
  RESORT_BINARY_OFFRECO_PARAM_MSG_06,
  RESORT_BINARY_OFFRECO_PARAM_MSG_07,
  RESORT_BINARY_OFFRECO_PARAM_MAX
} RESORT_BINARY_OFFRECO_PARAM;

//-------------------------------------
///	OFFRECOデータ
//=====================================
typedef struct _RESORT_BINARY_OFFRECO RESORT_BINARY_OFFRECO;

//-------------------------------------
///	PUBLIC
//=====================================
extern RESORT_BINARY_OFFRECO * RESORT_BINARY_OFFRECO_Init( HEAPID heapID );
extern void RESORT_BINARY_OFFRECO_Exit( RESORT_BINARY_OFFRECO *p_wk );
extern u16 RESORT_BINARY_OFFRECO_GetData( const RESORT_BINARY_OFFRECO *cp_wk, u32 index, RESORT_BINARY_OFFRECO_PARAM param );
extern u32 RESORT_BINARY_OFFRECO_GetMax( const RESORT_BINARY_OFFRECO *cp_wk );
