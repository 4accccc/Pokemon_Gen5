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
#include "resonance_resort_data.naix"

//=============================================================================
/**
 *  バイナリ操作
 *    ->データ構造としては、u16の配列構造体がいくつか繋がっているもの
 */
//=============================================================================
//-------------------------------------
///	ワーク
//=====================================
typedef struct _RESORT_BINARY RESORT_BINARY;

//-------------------------------------
///	PUBLIC
//=====================================
extern RESORT_BINARY * RESORT_BINARY_Init( ARCDATID datID, u32 param_max, HEAPID heapID );
extern void RESORT_BINARY_Exit( RESORT_BINARY *p_wk );
extern u16 RESORT_BINARY_GetData( const RESORT_BINARY *cp_wk, u32 index, u32 param );
extern u32 RESORT_BINARY_GetMax( const RESORT_BINARY *cp_wk );

//=============================================================================
/**
 *					レゾナンスリゾート人物バイナリデータ
*/
//=============================================================================
//-------------------------------------
///	データ
//=====================================
typedef enum
{
  RESORT_BINARY_PEOPLE_PARAM_PEOPLE_TYPE,
  RESORT_BINARY_PEOPLE_PARAM_KIND,
  RESORT_BINARY_PEOPLE_PARAM_PERSONALITY,
  RESORT_BINARY_PEOPLE_PARAM_VIEW,
  RESORT_BINARY_PEOPLE_PARAM_SEX,
  RESORT_BINARY_PEOPLE_PARAM_SCRID,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_00,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_01,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_02,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_03,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_04,
  RESORT_BINARY_PEOPLE_PARAM_MSGID_05,
  RESORT_BINARY_PEOPLE_PARAM_MAX,
}RESORT_BINARY_PEOPLE_PARAM;

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
typedef RESORT_BINARY RESORT_BINARY_PEOPLE;

//-------------------------------------
///	人物バイナリデータ 各人物分のワーク(直接アクセス禁止)
//=====================================
typedef struct _RESORT_BINARY_PEOPLE_DATA
{
  u16 param[RESORT_BINARY_PEOPLE_PARAM_MAX];
} RESORT_BINARY_PEOPLE_DATA;

//-------------------------------------
/// PUBLIC	
//=====================================
//バイナリ読み込み、破棄
#define RESORT_BINARY_PEOPLE_Init(x) RESORT_BINARY_Init(NARC_resonance_resort_data_resort_people_dat,RESORT_BINARY_PEOPLE_PARAM_MAX,x)
#define RESORT_BINARY_PEOPLE_Exit(x) RESORT_BINARY_Exit(x)
#define RESORT_BINARY_PEOPLE_GetMax(x) RESORT_BINARY_GetMax(x)

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
#define RESORT_BINARY_SPOT_ANSWER_NUM (4)
//-------------------------------------
///	列挙型
//=====================================
typedef enum
{
  RESORT_BINARY_SPOT_PARAM_MSG_00,
  RESORT_BINARY_SPOT_PARAM_GROUP,
  RESORT_BINARY_SPOT_PARAM_BAESEXP,
  RESORT_BINARY_SPOT_PARAM_DUMMY,

  RESORT_BINARY_SPOT_DATA_START,

  RESORT_BINARY_SPOT_PARAM_TYPE = RESORT_BINARY_SPOT_DATA_START,
  RESORT_BINARY_SPOT_PARAM_DATA,
  RESORT_BINARY_SPOT_PARAM_GRADE,
  RESORT_BINARY_SPOT_PARAM_EFF_MSG,

  RESORT_BINARY_SPOT_DATA_END = RESORT_BINARY_SPOT_DATA_START+RESORT_BINARY_SPOT_ANSWER_NUM*RESORT_BINARY_SPOT_ANSWER_MAX-1,

  RESORT_BINARY_SPOT_PARAM_MAX,
}RESORT_BINARY_SPOT_PARAM;

//-------------------------------------
///	アピールデータ
//=====================================
typedef RESORT_BINARY RESORT_BINARY_SPOT;

//-------------------------------------
/// PUBLIC	
//=====================================
#define RESORT_BINARY_SPOT_Init(x) RESORT_BINARY_Init( NARC_resonance_resort_data_resort_spot_dat,RESORT_BINARY_SPOT_PARAM_MAX,x)
#define RESORT_BINARY_SPOT_Exit(x) RESORT_BINARY_Exit(x)
#define RESORT_BINARY_SPOT_GetData(x,y,z) RESORT_BINARY_GetData(x,y,z)
extern BOOL RESORT_BINARY_SPOT_SearchIndex( const RESORT_BINARY_SPOT *cp_wk, u32 msg, u32 *p_index );
#define RESORT_BINARY_SPOT_GetMax(x) RESORT_BINARY_GetMax(x)

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
typedef RESORT_BINARY RESORT_BINARY_NPC;

//-------------------------------------
///	PUBLIC
//=====================================
#define RESORT_BINARY_NPC_Init(x) RESORT_BINARY_Init(NARC_resonance_resort_data_resort_npc_dat,RESORT_BINARY_NPC_PARAM_MAX,x)
#define RESORT_BINARY_NPC_Exit(x) RESORT_BINARY_Exit(x)
#define RESORT_BINARY_NPC_GetData(x,y,z) RESORT_BINARY_GetData(x,y,z)
#define RESORT_BINARY_NPC_GetMax(x) RESORT_BINARY_GetMax(x)

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
  RESORT_BINARY_MOUTHCOMM_PARAM_MSG_00,
  RESORT_BINARY_MOUTHCOMM_PARAM_MSG_01,
  RESORT_BINARY_MOUTHCOMM_PARAM_MSG_02,
  RESORT_BINARY_MOUTHCOMM_PARAM_MSG_03,
  RESORT_BINARY_MOUTHCOMM_PARAM_MSG_04,
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
typedef RESORT_BINARY RESORT_BINARY_MOUTHCOMM;

//-------------------------------------
///	PUBLIC
//=====================================
#define RESORT_BINARY_MOUTHCOMM_Init(x) RESORT_BINARY_Init(NARC_resonance_resort_data_resort_kutikomi_dat,RESORT_BINARY_MOUTHCOMM_PARAM_MAX,x)
#define RESORT_BINARY_MOUTHCOMM_Exit(x) RESORT_BINARY_Exit(x)
#define RESORT_BINARY_MOUTHCOMM_GetData(x,y,z) RESORT_BINARY_GetData(x,y,z)


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
typedef RESORT_BINARY RESORT_BINARY_OFFRECO;

//-------------------------------------
///	PUBLIC
//=====================================
#define RESORT_BINARY_OFFRECO_Init(x) RESORT_BINARY_Init(NARC_resonance_resort_data_resort_offreco_dat,RESORT_BINARY_OFFRECO_PARAM_MAX,x)
#define RESORT_BINARY_OFFRECO_Exit(x) RESORT_BINARY_Exit(x) 
#define RESORT_BINARY_OFFRECO_GetData(x,y,z) RESORT_BINARY_GetData(x,y,z)
#define RESORT_BINARY_OFFRECO_GetMax(x) RESORT_BINARY_GetMax(x)

//=============================================================================
/**
 *      レゾナンスリゾートCITYデータ
 */
//=============================================================================
//-------------------------------------
///	列挙型
//=====================================
typedef enum
{
  RESORT_BINARY_CITY_PARAM_LV_LOW,
  RESORT_BINARY_CITY_PARAM_LV_HIGH,
  RESORT_BINARY_CITY_PARAM_GROUP_00,
  RESORT_BINARY_CITY_PARAM_GROUP_01,
  RESORT_BINARY_CITY_PARAM_GROUP_02,
  RESORT_BINARY_CITY_PARAM_EXP,
  RESORT_BINARY_CITY_PARAM_RANK,
  RESORT_BINARY_CITY_PARAM_PRESENT_LIST,
  RESORT_BINARY_CITY_PARAM_PRESENT_PER,
  RESORT_BINARY_CITY_PARAM_MAX
} RESORT_BINARY_CITY_PARAM;

//-------------------------------------
///	CITYデータ
//=====================================
typedef RESORT_BINARY RESORT_BINARY_CITY;

//-------------------------------------
///	PUBLIC
//=====================================
#define RESORT_BINARY_CITY_Init( x ) RESORT_BINARY_Init(NARC_resonance_resort_data_resort_city_dat,RESORT_BINARY_CITY_PARAM_MAX,x)
#define RESORT_BINARY_CITY_Exit( x ) RESORT_BINARY_Exit( x )
#define RESORT_BINARY_CITY_GetData( x, y, z ) RESORT_BINARY_GetData(x,y,z)
#define RESORT_BINARY_CITY_GetMax( x ) RESORT_BINARY_GetMax( x )
extern u32 RESORT_BINARY_CITY_GetIndexByCityLv( const RESORT_BINARY_CITY *cp_wk, u32 lv );
