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
#include "buflen.h"
#include "system/main.h"
#include "system/gfl_use.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/pokepara_conv.h"
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
  if( PPP_Get( ppp , ID_PARA_monsno , NULL ) == MONSNO_PITYUU &&
      PPP_Get( ppp , ID_PARA_form_no , NULL ) == 1 )
  {
    ret = MUCPR_GIZAMIMI;
  }

  if( ret == MUCPR_OK )
  {
    for( i=0;i<4;i++ )
    {
      wazaArr[i] = PPP_Get( ppp , ID_PARA_waza1+i , 0 );
    }
    for( i=0;i<4;i++ )
    {
      if( cardType == CARD_TYPE_GS )
      {
        if( wazaArr[i] == WAZANO_IAIGIRI ||
            wazaArr[i] == WAZANO_SORAWOTOBU ||
            wazaArr[i] == WAZANO_KAIRIKI ||
            wazaArr[i] == WAZANO_NAMINORI ||
            wazaArr[i] == WAZANO_UZUSIO ||      //ここが違う
            wazaArr[i] == WAZANO_IWAKUDAKI ||
            wazaArr[i] == WAZANO_TAKINOBORI ||
            wazaArr[i] == WAZANO_ROKKUKURAIMU  )
        {
          ret = MUCPR_HIDEN;
        }
      }
      else
      {
        if( wazaArr[i] == WAZANO_IAIGIRI ||
            wazaArr[i] == WAZANO_SORAWOTOBU ||
            wazaArr[i] == WAZANO_KAIRIKI ||
            wazaArr[i] == WAZANO_NAMINORI ||
            wazaArr[i] == WAZANO_KIRIBARAI ||      //ここが違う
            wazaArr[i] == WAZANO_IWAKUDAKI ||
            wazaArr[i] == WAZANO_TAKINOBORI ||
            wazaArr[i] == WAZANO_ROKKUKURAIMU  )
        {
          ret = MUCPR_HIDEN;
        }
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

//--------------------------------------------------------------
//	PPPの変換
//--------------------------------------------------------------
void MB_UTIL_ConvertPPP( const void *src , POKEMON_PASO_PARAM *dst , const DLPLAY_CARD_TYPE cardType )
{
  GFL_STD_MemCopy( src , dst , POKETOOL_GetPPPWorkSize() );

  //共通処理はこっち
  PPCONV_ConvertPPPFromPokeShifter( dst );

  //PokeShifter専用はこっち
  //フォルム処理(ギラティナ・アルセウス)
  {
    const u16 monsno = PPP_Get( dst , ID_PARA_monsno , NULL );
    if( monsno == MONSNO_ARUSEUSU ||
        monsno == MONSNO_GIRATHINA )
    {
      PPP_ChangeFormNo( dst , 0 );
    }
  }
}
