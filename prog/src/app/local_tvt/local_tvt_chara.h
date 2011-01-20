//======================================================================
/**
 * @file	local_tvt_chara.h
 * @brief	非通信テレビトランシーバー キャラ
 * @author	ariizumi
 * @data	09/11/02
 *
 * モジュール名：LOCAL_TVT_CHARA
 */
//======================================================================
#pragma once

#include "local_tvt_message.h"
#include "local_tvt_util.h"
#include "local_tvt_type_def.h"
//======================================================================
//	define
//======================================================================
#pragma mark [> define

extern const u8 LOCAL_TVT_CHARA_BASE_POS[LOCAL_TVT_MEMBER_MAX][2];

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  LOCAL_TVT_CHARA_POS_LEFTUP,
  LOCAL_TVT_CHARA_POS_RIGHTUP,
  LOCAL_TVT_CHARA_POS_LEFTDOWN,
  LOCAL_TVT_CHARA_POS_RIGHTDOWN,
  LOCAL_TVT_CHARA_POS_MAX,
}LOCAL_TVT_CHARA_POS;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct _LOCAL_TVT_CHARA_SYS LOCAL_TVT_CHARA_SYS;
typedef struct _LOCAL_TVT_CHARA LOCAL_TVT_CHARA;

//======================================================================
//	system
//======================================================================
typedef struct
{
  u16 charaType;    //LOCAL_TVT_CHARA_TYPE
  u16 bgType;
  u16 weatherType;  //LOCAL_TVT_WEATHER
  u16 dummy;
}LOCAL_TVT_CHARA_PARAM;
typedef struct
{
  GFL_CLUNIT          *cellUnit;
  MYSTATUS            *mystatus;
  LOCAL_TVT_MESSAGE   *message;
  LOCAL_TVT_TIME_ZONE timeZone;
}LOCAL_TVT_CHARA_SYS_PARAM;

extern LOCAL_TVT_CHARA_SYS * LOCAL_TVT_CHARA_SYS_Init( const LOCAL_TVT_CHARA_SYS_PARAM *param , HEAPID heapID );
extern void LOCAL_TVT_CHARA_SYS_Term( LOCAL_TVT_CHARA_SYS *sys );
extern void LOCAL_TVT_CHARA_SYS_Main( LOCAL_TVT_CHARA_SYS *sys );

extern void LOCAL_TVT_CHARA_SYS_LoadChara( LOCAL_TVT_CHARA_SYS *sys, const LOCAL_TVT_CHARA_PARAM *cp_setup_tbl, LOCAL_TVT_MEMBER member );

extern LOCAL_TVT_CHARA* LOCAL_TVT_CHARA_SYS_GetWork( LOCAL_TVT_CHARA_SYS *sys, LOCAL_TVT_CHARA_POS pos );

extern void LOCAL_TVT_CHARA_SYS_RedrawName( LOCAL_TVT_CHARA_SYS *sys );

//デバッグ用
extern void LOCAL_TVT_CHARA_SYS_SetTimeZone( LOCAL_TVT_CHARA_SYS *sys, LOCAL_TVT_TIME_ZONE timeZone );
extern LOCAL_TVT_TIME_ZONE LOCAL_TVT_CHARA_SYS_GetTimeZone( const LOCAL_TVT_CHARA_SYS *sys );
extern void LOCAL_TVT_CHARA_SYS_GetLoadParam( const LOCAL_TVT_CHARA_SYS *sys, LOCAL_TVT_CHARA_PARAM *p_param, LOCAL_TVT_MEMBER *p_member );

//======================================================================
//	work
//======================================================================

extern void LOCAL_TVT_CHARA_StartTalk( LOCAL_TVT_CHARA *charaWork, const GFL_MSGDATA *cp_msg, u16 msgID );
extern BOOL LOCAL_TVT_CHARA_WaitTalk( LOCAL_TVT_CHARA *charaWork, LOCAL_TVT_CHARA_SYS *sys );
extern void LOCAL_TVT_CHARA_EndTalk( LOCAL_TVT_CHARA *charaWork, LOCAL_TVT_CHARA_SYS *sys );

