//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_people.c
 *	@brief  レゾナンスリゾート人物管理
 *	@author	Toru=Nagihashi
 *	@date		2011.02.16
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *				人物ワーク
*/
//=============================================================================
//-------------------------------------
///	人物の性格
//=====================================
typedef enum
{
  RESORT_PEOPLE_SEIKAKU_TODO,
  RESORT_PEOPLE_SEIKAKU_MAX,
}RESORT_PEOPLE_SEIKAKU;

//-------------------------------------
///	人物のタイプ
//=====================================
typedef enum
{
  RESORT_PEOPLE_TYPE_RESIDENT,  //住民
  RESORT_PEOPLE_TYPE_PROXY,     //代理人
  RESORT_PEOPLE_TYPE_TOURIST,   //観光客
  RESORT_PEOPLE_TYPE_VISTOR,    //滞在者
  RESORT_PEOPLE_TYPE_SECRETARY, //秘書
  RESORT_PEOPLE_TYPE_NPC,       //NPC

  RESORT_PEOPLE_TYPE_MAX,
}RESORT_PEOPLE_TYPE;

//-------------------------------------
///	人物の種類
//=====================================
typedef enum
{
  RESORT_PEOPLE_KIND_START = 0,

  //住民の種類
  RESORT_RESIDENT_KIND_00 = RESORT_PEOPLE_KIND_START,
  RESORT_RESIDENT_KIND_MAX,

  //代理人の種類
  RESORT_PROXY_KIND_00 = RESORT_PEOPLE_KIND_START,
  RESORT_PROXY_KIND_MAX,

  //観光客の種類
  RESORT_TOURIST_KIND_00 = RESORT_PEOPLE_KIND_START,
  RESORT_TOURIST_KIND_MAX,

  //滞在者の種類
  RESORT_VISITOR_KIND_00 = RESORT_PEOPLE_KIND_START,
  RESORT_VISITOR_KIND_MAX,

  //秘書の種類
  RESORT_SECRETARY_KIND_00 = RESORT_PEOPLE_KIND_START,
  RESORT_SECRETARY_KIND_MAX,

  //NPCの種類
  RESORT_NPC_KIND_00 = RESORT_PEOPLE_KIND_START,
  RESORT_NPC_KIND_MAX,

} RESORT_PEOPLE_KIND;


//-------------------------------------
///	人物ワーク
//=====================================
typedef struct _RESORT_PEOPLE RESORT_PEOPLE;

//=============================================================================
/**
 *					人物管理ワーク
*/
//=============================================================================
//-------------------------------------
///	人物管理ワーク
//=====================================
typedef struct _RESORT_PEOPLE_SYS RESORT_PEOPLE_SYS;

//-------------------------------------
///	人物管理ワーク生成パラメータ
//=====================================
typedef struct
{
  MMDLSYS *p_mmdlsys;           //動作モデルシステム
  RESORT_CITY_DATA *p_city_data;//街用データ
} RESORT_PEOPLE_SYS_PARAM;

//-------------------------------------
///	PUBLIC
//=====================================
extern RESORT_PEOPLE_SYS * RESORT_PEOPLE_SYS_Init( const RESORT_PEOPLE_SYS_PARAM *cp_param, HEAPID heapID );
extern void RESORT_PEOPLE_SYS_Exit( RESORT_PEOPLE_SYS *p_wk );
extern void RESORT_PEOPLE_SYS_Main( RESORT_PEOPLE_SYS *p_wk );

extern const RESORT_PEOPLE * RESORT_PEOPLE_SYS_GetPeopleByMmdl( const RESORT_PEOPLE_SYS *cp_wk, const MMDL* cp_mmdl );
