//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		pokegra.c
 *	@brief	ポケモングラフィック素材の取得
 *	@author	Toru=Nagihashi
 *	@data		2009.10.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//アーカイブ
#include "arc_def.h"
#include "pokegra/pokegra_wb.naix"

//ポケツール
#include "poke_tool/monsno_def.h"
#include "poke_tool/poke_tool.h"

//外部参照
#include "system/pokegra.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void PokeGra_GetFileOffset( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, u32 *p_mons_offset, u32 *p_dir_offset, u32 *p_sex_offset, u32 *p_rare_offset );

//=============================================================================
/**
 *		リソース取得関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	POKEGRAのアーカイブID
 *		
 *	@return	アーカイブID
 */
//-----------------------------------------------------------------------------
ARCID POKEGRA_GetArcID( void )
{	
	return ARCID_POKEGRA;
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 * @param	mons_no モンスターナンバー
 * @param	form_no フォルムナンバー
 * @param	sex		性別
 * @param	rare	レアかどうか
 * @param	dir		ポケモンの向き（POKEGRA_DIR_FRONT:正面、POKEGRA_DIR_BACK:背面）
 *
 *	@return	キャラアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID POKEGRA_GetCgrArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
	u32 mons_offset;
	u32 dir_offset;
	u32 sex_offset;

  PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, &sex_offset, NULL );
  return mons_offset + dir_offset + POKEGRA_M_NCGR + sex_offset;
}
//----------------------------------------------------------------------------
/**
 *	@brief	
 *
 * @param	mons_no モンスターナンバー
 * @param	form_no フォルムナンバー
 * @param	sex		性別
 * @param	rare	レアかどうか
 * @param	dir		ポケモンの向き（POKEGRA_DIR_FRONT:正面、POKEGRA_DIR_BACK:背面）
 *
 *	@return	キャラビットマップアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID POKEGRA_GetCbrArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
	u32 mons_offset;
	u32 dir_offset;
	u32 sex_offset;

  PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, &sex_offset, NULL );
  return mons_offset + dir_offset + POKEGRA_M_NCBR + sex_offset;
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 * @param	mons_no モンスターナンバー
 * @param	form_no フォルムナンバー
 * @param	sex		性別
 * @param	rare	レアかどうか
 * @param	dir		ポケモンの向き（POKEGRA_DIR_FRONT:正面、POKEGRA_DIR_BACK:背面）
 *
 *	@return	ﾊﾟﾚｯﾄアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID POKEGRA_GetPalArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
  u32 mons_offset;
  u32 rare_offset;
  PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, NULL, NULL, &rare_offset );
  return mons_offset + POKEGRA_NORMAL_NCLR + rare_offset;
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 * @param	mons_no モンスターナンバー
 * @param	form_no フォルムナンバー
 * @param	sex		性別
 * @param	rare	レアかどうか
 * @param	dir		ポケモンの向き（POKEGRA_DIR_FRONT:正面、POKEGRA_DIR_BACK:背面）
 *
 *	@return	セルアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID POKEGRA_GetCelArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
  u32 mons_offset;
  u32 dir_offset;

  PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL );
  return mons_offset + dir_offset + POKEGRA_NCER;
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 * @param	mons_no モンスターナンバー
 * @param	form_no フォルムナンバー
 * @param	sex		性別
 * @param	rare	レアかどうか
 * @param	dir		ポケモンの向き（POKEGRA_DIR_FRONT:正面、POKEGRA_DIR_BACK:背面）
 *
 *	@return	セルアニメアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID POKEGRA_GetAnmArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
	u32 mons_offset;
	u32 dir_offset;

	PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL );

	return mons_offset + dir_offset + POKEGRA_NANR;
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 * @param	mons_no モンスターナンバー
 * @param	form_no フォルムナンバー
 * @param	sex		性別
 * @param	rare	レアかどうか
 * @param	dir		ポケモンの向き（POKEGRA_DIR_FRONT:正面、POKEGRA_DIR_BACK:背面）
 *
 *	@return	マルチセルアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID POKEGRA_GetMCelArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
	u32 mons_offset;
	u32 dir_offset;

	PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL );

	return mons_offset + dir_offset + POKEGRA_NMCR;
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 * @param	mons_no モンスターナンバー
 * @param	form_no フォルムナンバー
 * @param	sex		性別
 * @param	rare	レアかどうか
 * @param	dir		ポケモンの向き（POKEGRA_DIR_FRONT:正面、POKEGRA_DIR_BACK:背面）
 *
 *	@return	マルチセルアニメアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID POKEGRA_GetMAnmArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
	u32 mons_offset;
	u32 dir_offset;

	PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL );

	return mons_offset + dir_offset + POKEGRA_NMAR;
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 * @param	mons_no モンスターナンバー
 * @param	form_no フォルムナンバー
 * @param	sex		性別
 * @param	rare	レアかどうか
 * @param	dir		ポケモンの向き（POKEGRA_DIR_FRONT:正面、POKEGRA_DIR_BACK:背面）
 *
 *	@return	アーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID POKEGRA_GetNcecArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
	u32 mons_offset;
	u32 dir_offset;

	PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL );

	return mons_offset + dir_offset + POKEGRA_NCEC;
}

//=============================================================================
/**
 *	private
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	 mons_no	モンスターナンバー
 *	@param	form_no		フォルムナンバー
 *	@param	sex				性別
 *	@param	rare			レアかどうか
 *	@param	dir		    ポケモンの向き（POKEGRA_DIR_FRONT:正面、POKEGRA_DIR_BACK:背面）
 *	@param  egg       タマゴかどうか
 *	@param	*p_mons_offset				ポケモン番号ファイルオフセット
 *	@param	*p_dir_offset			背面オフセット
 *	@param	*p_sex_offset				性別オフセット
 *	@param	u32 *p_rare_offset	レアオフセット
 */
//-----------------------------------------------------------------------------
static void PokeGra_GetFileOffset( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, u32 *p_mons_offset, u32 *p_dir_offset, u32 *p_sex_offset, u32 *p_rare_offset )
{	
	u32 file_start;
	u32 file_offset;
	u32 mf_ratio;

  file_start	= POKEGRA_FILE_MAX * mons_no;
  file_offset	= (dir == POKEGRA_DIR_FRONT) ? POKEGRA_FRONT_M_NCGR: POKEGRA_BACK_M_NCGR;
  mf_ratio	= (dir == POKEGRA_DIR_FRONT) ? POKEGRA_FRONT_M_NCBR: POKEGRA_BACK_M_NCBR;

  //タマゴチェック
  if( egg )
  { 
    if( form_no > 2 )
    { 
      form_no = 0;
    }
    file_start = POKEGRA_FILE_MAX * ( MONSNO_TAMAGO + form_no );
  }
  //別フォルム処理
  else if( form_no )
  { 
    int gra_index = POKETOOL_GetPersonalParam( mons_no, 0, POKEPER_ID_form_gra_index );
    int form_max = POKETOOL_GetPersonalParam( mons_no, 0, POKEPER_ID_form_max );
    if( form_no >= form_max )
    { 
      form_no = 0;
    }
    file_start = POKEGRA_FILE_MAX * ( MONSNO_MAX + 1 ) + POKEGRA_FILE_MAX * ( gra_index + form_no - 1 );
  }

  //性別のチェック
  switch( sex ){
  case PTL_SEX_MALE:
    break;
  case PTL_SEX_FEMALE:
    //オスメス書き分けしているかチェックする（サイズが０なら書き分けなし）
    sex = ( GFL_ARC_GetDataSize( ARCID_POKEGRA, file_start + mf_ratio + 1 ) == 0 ) ? PTL_SEX_MALE : PTL_SEX_FEMALE;
    break;
  case PTL_SEX_UNKNOWN:
    //性別なしは、オス扱いにする
    sex = PTL_SEX_MALE;
    break;
  default:
    //ありえない性別
    GF_ASSERT(0);
    break;
  }

  //受け取り
  if( p_mons_offset )
  {	
    *p_mons_offset	= file_start;
  }
  if( p_dir_offset )
	{	
		*p_dir_offset		= file_offset;
	}
	if( p_sex_offset )
	{	
		*p_sex_offset		= sex;
	}
	if( p_rare_offset )
	{	
		*p_rare_offset	= rare;
	}
}
