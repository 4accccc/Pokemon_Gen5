//======================================================================
/**
 * @file	mb_util_poke.c
 * @brief	マルチブート ユーティリティ
 * @author	ariizumi
 * @data	09/12/06
 *
 * モジュール名：MB_UTIL
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "poke_tool/monsno_def.h"
#include "waza_tool/wazano_def.h"

#include "multiboot/mb_util.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
const MB_UTIL_CHECK_PLAY_RET MB_UTIL_CheckPlay_PalGate( POKEMON_PASO_PARAM *ppp , const DLPLAY_CARD_TYPE cardType )
{
  MB_UTIL_CHECK_PLAY_RET ret = MUCPR_OK;
  u8 i;
  const BOOL isFastMode = PPP_FastModeOn( ppp );
  u32 wazaArr[4];
  
  PPP_FastModeOff( ppp , isFastMode );
  
  if( PPP_Get( ppp , ID_PARA_tamago_flag , NULL ) == TRUE )
  {
    ret = MUCPR_EGG;
  }

  if( ret == MUCPR_OK )
  {
    for( i=0;i<4;i++ )
    {
      wazaArr[i] = PPP_Get( ppp , ID_PARA_waza1+i , 0 );
    }
    for( i=0;i<4;i++ )
    {
      //暫定！(カードタイプで分岐かも？
      if( wazaArr[i] == WAZANO_IAIGIRI ||
          wazaArr[i] == WAZANO_SORAWOTOBU ||
          wazaArr[i] == WAZANO_KAIRIKI ||
          wazaArr[i] == WAZANO_NAMINORI ||
          wazaArr[i] == WAZANO_HURASSYU )
      {
        ret = MUCPR_HIDEN;
      }
    }
  }

  if( ret == MUCPR_OK )
  {
    if( PPP_Get( ppp , ID_PARA_fusei_tamago_flag , NULL ) ||
        PPP_Get( ppp , ID_PARA_monsno , NULL ) > MONSNO_ARUSEUSU )
    {
      ret = MUCPR_FUSEI;
    }
  }
  return ret;
}


