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

//パッチールブチ作成
#include "../battle/btlv/data/patch.cdat"
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
static void PokeGra_GetFileOffset( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, u32 *p_mons_offset, u32 *p_dir_offset, u32 *p_sex_offset, u32 *p_rare_offset, u32 *pltt_only );

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

  PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, &sex_offset, NULL, NULL );
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

  PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, &sex_offset, NULL, NULL );
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
  u32 pltt_only = 0;
  PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, NULL, NULL, &rare_offset, &pltt_only );
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
ARCDATID POKEGRA_GetCelArcIndex( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg )
{	
  u32 mons_offset;
  u32 dir_offset;

  PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL, NULL );
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

	PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL, NULL );

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

	PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL, NULL );

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

	PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL, NULL );

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

	PokeGra_GetFileOffset( mons_no, form_no, sex, rare, dir, egg, &mons_offset, &dir_offset, NULL, NULL, NULL );

	return mons_offset + dir_offset + POKEGRA_NCEC;
}

//----------------------------------------------------------------------------
/**
 *	@brief  2D用キャラはセル分割のため、並び替えされているので、
 *          それをBGで使えるように並び直す関数
 *
 *	@param	NNSG2dCharacterData *p_chara キャラ
 *	@param  heapID  テンポラリ作成用ヒープ
 */
//-----------------------------------------------------------------------------
void POKEGRA_SortBGCharacter( NNSG2dCharacterData *p_chara, HEAPID heapID )
{ 
  //テンポラリにキャラをコピー
  u8 *p_buff  = p_chara->pRawData;
  u8 *p_temp  = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(heapID), POKEGRA_POKEMON_CHARA_SIZE );
  MI_CpuCopyFast( p_buff, p_temp, POKEGRA_POKEMON_CHARA_SIZE );

  //8x8,4*8,8*4,4*4の4つのOAMにあわせるように転送
  {
    int i;
    u32 dst_pos, src_pos;

    dst_pos = 0;
    src_pos = 0;

    //8*8を転送
    for( i = 0; i < POKEGRA_CELL01_Y; i++ )
    { 
      dst_pos = (i * POKEGRA_POKEMON_CHARA_WIDTH) * GFL_BG_1CHRDATASIZ;
      MI_CpuCopyFast( &p_temp[src_pos], &p_buff[dst_pos], POKEGRA_CELL01_X*GFL_BG_1CHRDATASIZ );
      src_pos += POKEGRA_CELL01_X*GFL_BG_1CHRDATASIZ;
    }
    //4*8を転送
    for( i = 0; i < POKEGRA_CELL02_Y; i++ )
    { 
      dst_pos = (POKEGRA_CELL01_X + i * POKEGRA_POKEMON_CHARA_WIDTH) * GFL_BG_1CHRDATASIZ;
      MI_CpuCopyFast( &p_temp[src_pos], &p_buff[dst_pos], POKEGRA_CELL02_X*GFL_BG_1CHRDATASIZ );
      src_pos += POKEGRA_CELL02_X*GFL_BG_1CHRDATASIZ;
    }
    //8*4を転送
    for( i = 0; i < POKEGRA_CELL03_Y; i++ )
    { 
      dst_pos = ((POKEGRA_CELL01_Y + i) * POKEGRA_POKEMON_CHARA_WIDTH) * GFL_BG_1CHRDATASIZ;
      MI_CpuCopyFast( &p_temp[src_pos], &p_buff[dst_pos], POKEGRA_CELL03_X*GFL_BG_1CHRDATASIZ );
      src_pos += POKEGRA_CELL03_X*GFL_BG_1CHRDATASIZ;
    }
    //4*4を転送
    for( i = 0; i < POKEGRA_CELL04_Y; i++ )
    { 
      dst_pos = (POKEGRA_CELL03_X + (POKEGRA_CELL02_Y + i) 
                * POKEGRA_POKEMON_CHARA_WIDTH) * GFL_BG_1CHRDATASIZ;
      MI_CpuCopyFast( &p_temp[src_pos], &p_buff[dst_pos], POKEGRA_CELL04_X*GFL_BG_1CHRDATASIZ );
      src_pos += POKEGRA_CELL04_X*GFL_BG_1CHRDATASIZ;
    }
  }

  //一時バッファ削除
  GFL_HEAP_FreeMemory( p_temp );
}
//----------------------------------------------------------------------------
/**
 *	@brief  上記で並び直したものをもとの並びに戻す関数
 *
 *	@param	NNSG2dCharacterData *p_chara キャラ
 *	@param  heapID  テンポラリ作成用ヒープ
 */
//-----------------------------------------------------------------------------
void POKEGRA_SortOBJCharacter( NNSG2dCharacterData *p_chara, HEAPID heapID )
{ 
  //テンポラリにキャラをコピー
  u8 *p_buff  = p_chara->pRawData;
  u8 *p_temp  = GFL_HEAP_AllocClearMemory( GFL_HEAP_LOWID(heapID), POKEGRA_POKEMON_CHARA_SIZE );
  MI_CpuCopyFast( p_buff, p_temp, POKEGRA_POKEMON_CHARA_SIZE );

  //8x8,4*8,8*4,4*4の4つのOAMにあわせるように転送
  {
    int i;
    u32 dst_pos, src_pos;

    dst_pos = 0;
    src_pos = 0;

    //8*8を転送
    for( i = 0; i < POKEGRA_CELL01_Y; i++ )
    { 
      src_pos = (i * POKEGRA_POKEMON_CHARA_WIDTH) * GFL_BG_1CHRDATASIZ;
      MI_CpuCopyFast( &p_temp[src_pos], &p_buff[dst_pos], POKEGRA_CELL01_X*GFL_BG_1CHRDATASIZ );
      dst_pos += POKEGRA_CELL01_X*GFL_BG_1CHRDATASIZ;
    }
    //4*8を転送
    for( i = 0; i < POKEGRA_CELL02_Y; i++ )
    { 
      src_pos = (POKEGRA_CELL01_X + i * POKEGRA_POKEMON_CHARA_WIDTH) * GFL_BG_1CHRDATASIZ;
      MI_CpuCopyFast( &p_temp[src_pos], &p_buff[dst_pos], POKEGRA_CELL02_X*GFL_BG_1CHRDATASIZ );
      dst_pos += POKEGRA_CELL02_X*GFL_BG_1CHRDATASIZ;
    }
    //8*4を転送
    for( i = 0; i < POKEGRA_CELL03_Y; i++ )
    { 
      src_pos = ((POKEGRA_CELL01_Y + i) * POKEGRA_POKEMON_CHARA_WIDTH) * GFL_BG_1CHRDATASIZ;
      MI_CpuCopyFast( &p_temp[src_pos], &p_buff[dst_pos], POKEGRA_CELL03_X*GFL_BG_1CHRDATASIZ );
      dst_pos += POKEGRA_CELL03_X*GFL_BG_1CHRDATASIZ;
    }
    //4*4を転送
    for( i = 0; i < POKEGRA_CELL04_Y; i++ )
    { 
      src_pos = (POKEGRA_CELL03_X + (POKEGRA_CELL02_Y + i) 
                * POKEGRA_POKEMON_CHARA_WIDTH) * GFL_BG_1CHRDATASIZ;
      MI_CpuCopyFast( &p_temp[src_pos], &p_buff[dst_pos], POKEGRA_CELL04_X*GFL_BG_1CHRDATASIZ );
      dst_pos += POKEGRA_CELL04_X*GFL_BG_1CHRDATASIZ;
    }
  }

  //一時バッファ削除
  GFL_HEAP_FreeMemory( p_temp );
}

//----------------------------------------------------------------------------
/**
 *	@brief  パッチールのためのブチをキャラに書き込み(BGキャラの並びのときを想定)
 *
 *	@param	NNSG2dCharacterData *p_chara  キャラデータ
 */
//-----------------------------------------------------------------------------
void POKEGRA_MakePattiiruBuchi( NNSG2dCharacterData *p_chara, u32 personal_rnd )
{ 
  const	PATTIIRU_BUCHI_DATA	*pbd;
  int i, j;
  int	setx, sety, cnt;
  int	pos;
  u32 rnd = personal_rnd;
  u8  *buf = p_chara->pRawData;

  for( i = 0 ; i < 4 ; i++ )
  {
    pbd = pbd_table[ i ];
    cnt=0;
    while( pbd[ cnt ].posx != 0xff )
    {
      setx = pbd[ cnt ].posx - OFS_X + 8 - 2 + ( ( (rnd) & 0x0f ) - 8 );
      sety = pbd[ cnt ].posy - OFS_Y + 8     + ( ( ( (rnd) & 0xf0 ) >> 4 ) - 8 );

      pos = (setx / 8 + (sety / 8) * 12 ) * 64  + ((setx) % 8 + (sety % 8) * 8) ;
      pos /= 2;

      //NAGI_Printf( "[%d]=x[%d]y[%d]\n", i, setx, sety );

      if( setx & 1 )
      {
        if( ( ( buf[ pos ] & 0xf0 ) >= 0x10 ) && ( ( buf[ pos ] & 0xf0 ) <= 0x30) )
        {
          buf[ pos ] += 0x50;
        }
      }
      else
      {
        if( ( ( buf[ pos ] & 0x0f ) >= 0x01 ) && ( ( buf[ pos ] & 0x0f ) <= 0x03 ) )
        {
          buf[ pos ] += 0x05;
        }
      }
      cnt++;
    }
    (rnd) = (rnd) >> 8;
  }
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
 *	@param	*pltt_only_offset    別フォルムがパレットのみのときのオフセット
 */
//-----------------------------------------------------------------------------
static void PokeGra_GetFileOffset( int mons_no, int form_no, int sex, int rare, int dir, BOOL egg, u32 *p_mons_offset, u32 *p_dir_offset, u32 *p_sex_offset, u32 *p_rare_offset, u32* pltt_only_offset )
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
    if( mons_no == MONSNO_MANAFI )
    { 
      form_no = 1;
    }
    else
    { 
      form_no = 0;
    }
    file_start = POKEGRA_FILE_MAX * ( MONSNO_TAMAGO + form_no );
  }
  //別フォルム処理
  else if( form_no )
  { 
    int gra_index = POKETOOL_GetPersonalParam( mons_no, 0, POKEPER_ID_form_gra_index );
    int pltt_only = POKETOOL_GetPersonalParam( mons_no, 0, POKEPER_ID_pltt_only );
    int form_max = POKETOOL_GetPersonalParam( mons_no, 0, POKEPER_ID_form_max );
    if( form_no >= form_max )
    { 
      form_no = 0;
    }
    if( pltt_only )
    { 
      if( pltt_only_offset )
      { 
        *pltt_only_offset = POKEGRA_FILE_MAX * ( MONSNO_MAX + OTHER_FORM_MAX + 1 ) + 13 + POKEGRA_PLTT_ONLY_MAX * ( gra_index + form_no - 1 ) + rare;
      }
    }
    else
    { 
      file_start = POKEGRA_FILE_MAX * ( MONSNO_MAX + 1 ) + POKEGRA_FILE_MAX * ( gra_index + form_no - 1 );
    }
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
