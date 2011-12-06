//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mus_pokegra.c
 *	@brief	ミュージカル専用ポケモングラフィック素材の取得
 *	@author	AkitoMori(BWから引越し）
 *	@data		2011.12.01
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
#include "mus_pokegra.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define POKEGRA_CELL01_X  8
#define POKEGRA_CELL01_Y  8
#define POKEGRA_CELL02_X  4
#define POKEGRA_CELL02_Y  8
#define POKEGRA_CELL03_X  8
#define POKEGRA_CELL03_Y  4
#define POKEGRA_CELL04_X  4
#define POKEGRA_CELL04_Y  4

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static void PokeGra_GetFileOffsetBW( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, u32 *p_mons_offset, u32 *p_dir_offset, u32 *p_sex_offset, u32 *p_rare_offset, u32 *pltt_only, BOOL ncgr_flag );

//=============================================================================
/**
 *		リソース取得関数
 */
//=============================================================================
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
ARCDATID MUS_POKEGRA_GetCgrArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
	u32 mons_offset;
	u32 dir_offset;
	u32 sex_offset;

  PokeGra_GetFileOffsetBW( arc_id , mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, &sex_offset, NULL, NULL, TRUE );
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
ARCDATID MUS_POKEGRA_GetCbrArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
	u32 mons_offset;
	u32 dir_offset;
	u32 sex_offset;

  PokeGra_GetFileOffsetBW( arc_id , mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, &sex_offset, NULL, NULL, FALSE );
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
ARCDATID MUS_POKEGRA_GetPalArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
  u32 mons_offset;
  u32 rare_offset;
  u32 pltt_only = 0;
  PokeGra_GetFileOffsetBW( arc_id , mons_no, form_no, sex, rare, dir, egg, &mons_offset, NULL, NULL, &rare_offset, &pltt_only, FALSE );
  if( pltt_only )
  { 
    return pltt_only;
  }
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
ARCDATID MUS_POKEGRA_GetCelArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
  u32 mons_offset;
  u32 dir_offset;

  PokeGra_GetFileOffsetBW( arc_id , mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL, NULL, FALSE );
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
ARCDATID MUS_POKEGRA_GetAnmArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
	u32 mons_offset;
	u32 dir_offset;

	PokeGra_GetFileOffsetBW( arc_id , mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL, NULL, FALSE );

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
ARCDATID MUS_POKEGRA_GetMCelArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
	u32 mons_offset;
	u32 dir_offset;

	PokeGra_GetFileOffsetBW( arc_id , mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL, NULL, FALSE );

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
ARCDATID MUS_POKEGRA_GetMAnmArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
	u32 mons_offset;
	u32 dir_offset;

	PokeGra_GetFileOffsetBW( arc_id , mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL, NULL, FALSE );

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
ARCDATID MUS_POKEGRA_GetNcecArcIndex( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
	u32 mons_offset;
	u32 dir_offset;

	PokeGra_GetFileOffsetBW( arc_id , mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL, NULL, FALSE );

	return mons_offset + dir_offset + POKEGRA_NCEC;
}



//=============================================================================
/**
 *	private
 */
//=============================================================================

#define MONSNO_BW_MAX     ( 651 ) /// BWの時のポケモンデータ定義最大数
#define OTHER_FORM_BW_MAX (  60 ) /// BWの時のフォルム違いポケモンデータ数

//----------------------------------------------------------------------------
/**
 *	@brief ポケモンBWの時のパーソナルのアーカイブインデックス取得を行う(ミュージカル専用)
 *         
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
 *	@param	*pltt_only_offset    別フォルムがパレットのみのときのオフセット
 *	@param	ncgr_flag           雌雄書き分け判断をncgrファイルで行うかどうか？
 */
//-----------------------------------------------------------------------------
static void PokeGra_GetFileOffsetBW( ARCID arc_id , int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, u32 *p_mons_offset, u32 *p_dir_offset, u32 *p_sex_offset, u32 *p_rare_offset, u32* pltt_only_offset, BOOL ncgr_flag )
{	
	u32 file_start;
	u32 file_offset;
	u32 mf_ratio;

  file_start	= POKEGRA_FILE_MAX * mons_no;
  file_offset	= (dir == POKEGRA_DIR_FRONT) ? POKEGRA_FRONT_M_NCGR: POKEGRA_BACK_M_NCGR;
  if( ncgr_flag )
  { 
    mf_ratio	= (dir == POKEGRA_DIR_FRONT) ? POKEGRA_FRONT_M_NCGR: POKEGRA_BACK_M_NCGR;
  }
  else
  { 
    mf_ratio	= (dir == POKEGRA_DIR_FRONT) ? POKEGRA_FRONT_M_NCBR: POKEGRA_BACK_M_NCBR;
  }

  //タマゴチェック
  if( egg )
  { 
    if( mons_no == MONSNO_MANAFI )
    { 
      form_no = FORMNO_TAMAGO_MANAFI;
    }
    else
    { 
      form_no = FORMNO_TAMAGO_NORMAL;
    }
    file_start = POKEGRA_FILE_MAX * ( MONSNO_TAMAGO + form_no );
  }
  //別フォルム処理
  else if( form_no )
  { 
    int gra_index = POKETOOL_GetPersonalParamBW( mons_no, 0, POKEPER_ID_form_gra_index );
    int pltt_only = POKETOOL_GetPersonalParamBW( mons_no, 0, POKEPER_ID_pltt_only );
    int form_max  = POKETOOL_GetPersonalParamBW( mons_no, 0, POKEPER_ID_form_max );
    if( form_no >= form_max )
    { 
      form_no = 0;
    }
    if( pltt_only )
    { 
      if( ( pltt_only_offset ) && ( form_no ) )
      { 
        *pltt_only_offset = POKEGRA_FILE_MAX * ( MONSNO_BW_MAX + OTHER_FORM_BW_MAX + 1 ) + 13 + POKEGRA_PLTT_ONLY_MAX * ( gra_index + form_no - 1 ) + rare;
      }
    }
    else
    { 
      if( form_no )
      { 
        file_start = POKEGRA_FILE_MAX * ( MONSNO_BW_MAX + 1 ) + POKEGRA_FILE_MAX * ( gra_index + form_no - 1 );
      }
    }
  }

  //性別のチェック
  switch( sex ){
  case PTL_SEX_MALE:
    break;
  case PTL_SEX_FEMALE:
    //オスメス書き分けしているかチェックする（サイズが０なら書き分けなし）
    sex = ( GFL_ARC_GetDataSize( arc_id, file_start + mf_ratio + 1 ) == 0 ) ? PTL_SEX_MALE : PTL_SEX_FEMALE;
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
