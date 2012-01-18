//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		cygnus_data.c
 *	@brief  3DS連動：cygnus_savedataの実装部分
 *	@author	Toru=Nagihashi
 *	@date		2012.01.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>

#include "savedata/cygnus_data.h"
#include "poke_tool/monsno_def.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
SDK_COMPILER_ASSERT( CYGDATA_MAX < CYGDATA_AREA_SIZE );

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	SPポケモンのデータ
//=====================================
static const struct
{
  CYGNUSDATA_MAGICNUMBER magic_num;
  CYGNUSDATA_FLAG flag;
  u16 mons_no;
  u8  form_no;
  u8  sex;
} sc_cygnus_sp_poke[]  =
{
  {
    MAGICNUMBER_SP_POKE01,
    CYGFLAG_SP_POKE01,
    MONSNO_TORUNEROSU,
    1,
    PM_MALE,
  },
  {
    MAGICNUMBER_SP_POKE02,
    CYGFLAG_SP_POKE02,
    MONSNO_BORUTOROSU,
    1,
    PM_MALE,
  },
  {
    MAGICNUMBER_SP_POKE03,
    CYGFLAG_SP_POKE03,
    MONSNO_RANDOROSU,
    1,
    PM_MALE,
  },
  {
    MAGICNUMBER_SP_POKE04,
    CYGFLAG_SP_POKE04,
    MONSNO_DHIARUGA,  //TODO
    0,
    PM_MALE,
  },
  {
    MAGICNUMBER_SP_POKE05,
    CYGFLAG_SP_POKE05,
    MONSNO_PARUKIA,  //TODO
    0,
    PM_MALE,
  },
  {
    MAGICNUMBER_SP_POKE06,
    CYGFLAG_SP_POKE06,
    MONSNO_GIRATHINA,  //TODO
    0,
    PM_MALE,
  },
  {
    MAGICNUMBER_SP_POKE07,
    CYGFLAG_SP_POKE07,
    MONSNO_HOUOU,  //TODO
    0,
    PM_MALE,
  },
  {
    MAGICNUMBER_SP_POKE08,
    CYGFLAG_SP_POKE08,
    MONSNO_RUGIA,  //TODO
    0,
    PM_MALE,
  },
};


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  有効なマジックナンバー
 *
 *	@param	CYGNUSDATA_MAGICNUMBER magic_num  マジックナンバー
 *
 *	@return TRUE有効  FALSE無効
 */
//-----------------------------------------------------------------------------
BOOL CUGNUS_DATA_IsValidMagicNumber( CYGNUSDATA_MAGICNUMBER magic_num )
{
  int i;
  for( i = 0; i < NELEMS(sc_cygnus_sp_poke); i++ )
  {
    if( sc_cygnus_sp_poke[i].magic_num == magic_num )
    {
      return TRUE;
    }
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  マジックナンバーからポケモン情報の取得
 *
 *	@param	CYGNUSDATA_MAGICNUMBER magic_num  マジックナンバー
 *	@param	*p_mons_no  モンスター番号受け取り  NULLで受け取らない
 *	@param	*p_form_no  フォルム番号受け取り  NULLで受け取らない
 *	@param	*p_sex  性別受け取り  NULLで受け取らない
 */
//-----------------------------------------------------------------------------
void CYGNUS_DATA_GetMonsNoByMagicNum( CYGNUSDATA_MAGICNUMBER magic_num, u16 *p_mons_no, u8 *p_form_no, u8 *p_sex )
{
  int i;
  for( i = 0; i < NELEMS(sc_cygnus_sp_poke); i++ )
  {
    if( sc_cygnus_sp_poke[i].magic_num == magic_num )
    {
      if( p_mons_no )
      {
        *p_mons_no  = sc_cygnus_sp_poke[i].mons_no;
      }
      if( p_form_no )
      {
        *p_form_no = sc_cygnus_sp_poke[i].form_no;
      }
      if( p_sex )
      {
        *p_sex = sc_cygnus_sp_poke[i].sex;
      }
      
      return;
    }
  }

  GF_ASSERT(0);
}
//----------------------------------------------------------------------------
/**
 *	@brief  マジックナンバーからフラグを受け取り
 *
 *	@param	CYGNUSDATA_MAGICNUMBER magic_num  マジックナンバー
 *
 *	@return フラグ
 */
//-----------------------------------------------------------------------------
CYGNUSDATA_FLAG CYGNUS_DATA_GetFlagByMagicNum( CYGNUSDATA_MAGICNUMBER magic_num )
{
  int i;
  for( i = 0; i < NELEMS(sc_cygnus_sp_poke); i++ )
  {
    if( sc_cygnus_sp_poke[i].magic_num == magic_num )
    {
      return sc_cygnus_sp_poke[i].flag;
    }
  }

  GF_ASSERT(0);

  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フラグからマジックナンバーを取得
 *
 *	@param	CYGNUSDATA_FLAG flag  フラグ
 *
 *	@return マジックナンバー
 */
//-----------------------------------------------------------------------------
CYGNUSDATA_MAGICNUMBER CYGNUS_DATA_GetMagicNumByFlag( CYGNUSDATA_FLAG flag )
{
  int i;
  for( i = 0; i < NELEMS(sc_cygnus_sp_poke); i++ )
  {
    if( sc_cygnus_sp_poke[i].flag == flag )
    {
      return sc_cygnus_sp_poke[i].magic_num;
    }
  }

  GF_ASSERT(0);

  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  フラグからポケモン情報受け取り
 *
 *	@param	CYGNUSDATA_FLAG flag  フラグ
 *	@param	*p_mons_no  モンスター番号受け取り  NULLで受け取らない
 *	@param	*p_form_no  フォルム番号受け取り  NULLで受け取らない
 *	@param	*p_sex  性別受け取り  NULLで受け取らない
 *
 */
//-----------------------------------------------------------------------------
void CYGNUS_DATA_GetMonsNoByFlag( CYGNUSDATA_FLAG flag, u16 *p_mons_no, u8 *p_form_no, u8 *p_sex )
{
  int i;
  for( i = 0; i < NELEMS(sc_cygnus_sp_poke); i++ )
  {
    if( sc_cygnus_sp_poke[i].flag == flag )
    {
      if( p_mons_no )
      {
        *p_mons_no  = sc_cygnus_sp_poke[i].mons_no;
      }
      if( p_form_no )
      {
        *p_form_no = sc_cygnus_sp_poke[i].form_no;
      }
      if( p_sex )
      {
        *p_sex = sc_cygnus_sp_poke[i].sex;
      }
      
      return;
    }
  }

  GF_ASSERT(0);
}
