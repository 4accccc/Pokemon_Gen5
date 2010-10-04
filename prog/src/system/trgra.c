//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		trgra.c
 *	@brief	トレーナーグラフィック素材の取得
 *	@author	HisashiSogabe
 *	@data		2010.01.22
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
#include "system/trgra.h"

#include "trtype_gra.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//トレーナー一体を構成するMCSS用ファイルの構成
enum{
	TRGRA_NCGR,
	TRGRA_NCBR,
	TRGRA_NCER,
	TRGRA_NANR,
	TRGRA_NMCR,
	TRGRA_NMAR,
	TRGRA_NCEC,
	TRGRA_NCLR,

	TRGRA_FILE_MAX,			//トレーナー一体を構成するMCSS用ファイルの総数
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static  inline  const u8 get_gra_index( int tr_type, int dir );

//=============================================================================
/**
 *		リソース取得関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	POKEGRAのアーカイブID
 *
 *	@param[in]  dir 向き
 *		
 *	@return	アーカイブID
 */
//-----------------------------------------------------------------------------
ARCID TRGRA_GetArcID( int dir )
{	
  return 0;
/*  if( dir )
  { 
	  return ARCID_TRBGRA;
  }
  else
  { 
	  return ARCID_TRFGRA;
  }*/
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *  @param[in]	tr_type   トレーナータイプ
 *	@param[in]  dir       向き
 *
 *	@return	キャラアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID TRGRA_GetCgrArcIndex( int tr_type, int dir )
{	
	int	file_start = TRGRA_FILE_MAX * get_gra_index( tr_type, dir );	//トレーナータイプからファイルのオフセットを計算

	return file_start + TRGRA_NCGR;
}

//----------------------------------------------------------------------------
/**
 *	@brief	
 *
 *  @param[in]	tr_type   トレーナータイプ
 *	@param[in]  dir       向き
 *
 *	@return	キャラビットマップアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID TRGRA_GetCbrArcIndex( int tr_type, int dir )
{	
	int	file_start = TRGRA_FILE_MAX * get_gra_index( tr_type, dir );	//トレーナータイプからファイルのオフセットを計算

	return file_start + TRGRA_NCBR;
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *  @param[in]	tr_type   トレーナータイプ
 *	@param[in]  dir       向き
 *
 *	@return	ﾊﾟﾚｯﾄアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID TRGRA_GetPalArcIndex( int tr_type, int dir )
{	
	int	file_start = TRGRA_FILE_MAX * get_gra_index( tr_type, dir );	//トレーナータイプからファイルのオフセットを計算

	return file_start + TRGRA_NCLR;
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *  @param[in]	tr_type   トレーナータイプ
 *	@param[in]  dir       向き
 *
 *	@return	セルアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID TRGRA_GetCelArcIndex( int tr_type, int dir )
{	
	int	file_start = TRGRA_FILE_MAX * get_gra_index( tr_type, dir );	//トレーナータイプからファイルのオフセットを計算

	return file_start + TRGRA_NCER;
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *  @param[in]	tr_type   トレーナータイプ
 *	@param[in]  dir       向き
 *
 *	@return	セルアニメアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID TRGRA_GetAnmArcIndex( int tr_type, int dir )
{	
	int	file_start = TRGRA_FILE_MAX * get_gra_index( tr_type, dir );	//トレーナータイプからファイルのオフセットを計算

	return file_start + TRGRA_NANR;
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *  @param[in]	tr_type   トレーナータイプ
 *	@param[in]  dir       向き
 *
 *	@return	マルチセルアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID TRGRA_GetMCelArcIndex( int tr_type, int dir )
{	
	int	file_start = TRGRA_FILE_MAX * get_gra_index( tr_type, dir );	//トレーナータイプからファイルのオフセットを計算

	return file_start + TRGRA_NMCR;
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *  @param[in]	tr_type   トレーナータイプ
 *	@param[in]  dir       向き
 *
 *	@return	マルチセルアニメアーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID TRGRA_GetMAnmArcIndex( int tr_type, int dir )
{	
	int	file_start = TRGRA_FILE_MAX * get_gra_index( tr_type, dir );	//トレーナータイプからファイルのオフセットを計算

	return file_start + TRGRA_NMAR;
}

//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *  @param[in]	tr_type   トレーナータイプ
 *	@param[in]  dir       向き
 *
 *	@return	アーカイブインデックス
 */
//-----------------------------------------------------------------------------
ARCDATID TRGRA_GetNcecArcIndex( int tr_type, int dir )
{	
	int	file_start = TRGRA_FILE_MAX * get_gra_index( tr_type, dir );	//トレーナータイプからファイルのオフセットを計算

	return file_start + TRGRA_NCEC;
}

static  inline  const u8 get_gra_index( int tr_type, int dir )
{ 
  //背面は取得する必要なし
  if( dir & 1 )
  { 
    return tr_type;
  }
  GF_ASSERT( NELEMS( TrTypeGraTable ) > tr_type );

  if( NELEMS( TrTypeGraTable ) < tr_type )
  { 
    return 0;
  }

  return TrTypeGraTable[ tr_type ];
}
