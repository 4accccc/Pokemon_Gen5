//======================================================================
/**
 * @file	mb_util_poke.h
 * @brief	マルチブート ユーティリティ
 * @author	ariizumi
 * @data	09/12/06
 *
 * モジュール名：MB_UTIL
 */
//======================================================================
#pragma once

#include "multiboot/mb_data_def.h"
#include "poke_tool/poke_tool.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  MUCPR_OK,
  MUCPR_EGG,    //タマゴはダメ
  MUCPR_HIDEN,  //秘伝持ちもダメ
  MUCPR_FUSEI,  //不正！
  MUCPR_GIZAMIMI,  //ぎざみみピチュー
}MB_UTIL_CHECK_PLAY_RET;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
extern const MB_UTIL_CHECK_PLAY_RET MB_UTIL_CheckPlay_PalGate( POKEMON_PASO_PARAM *ppp , const DLPLAY_CARD_TYPE cardType );
extern void MB_UTIL_ConvertPPP( const void *src , POKEMON_PASO_PARAM *dst , const DLPLAY_CARD_TYPE cardType );
extern void MB_UTIL_ConvertStr( const u16 *src , STRCODE *dst , const u8 dstLen , const DLPLAY_CARD_TYPE cardType );

