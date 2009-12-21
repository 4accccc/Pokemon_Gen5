//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		namein_data.c
 *	@brief	名前入力用データ
 *	@author	Toru=Nagihashi
 *	@data		2009.10.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "print/str_tool.h"

//アーカイブ
#include "arc_def.h"
#include "namein_data.naix"

//外部
#include "namein_data.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	変換用データ
//=====================================
//存在しない場合入っている値（namein_change_conv.pl内でスペースをいれている）
#define NAMEIN_STRCHANGE_EMPTY	(L'　')

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	名前入力データ
//=====================================
struct _NAMEIN_KEYMAP
{
	u16			w;
	u16			h;
	STRCODE	str[0];
} ;

//-------------------------------------
///	変換用データ
//=====================================
typedef struct
{	
	STRCODE	input[NAMEIN_STRCHANGE_CODE_LEN];
	STRCODE	chage[NAMEIN_STRCHANGE_CODE_LEN];
	STRCODE	shift[NAMEIN_STRCHANGE_CODE_LEN];
	STRCODE	rest[NAMEIN_STRCHANGE_CODE_LEN];
}CHANGECODE;
struct _NAMEIN_STRCHANGE
{
	u16					num;
	CHANGECODE	word[0];
} ;

//-------------------------------------
///	ハンドル
//=====================================
struct _NAMEIN_KEYMAP_HANDLE
{ 
  NAMEIN_KEYMAP *p_data[ NAMEIN_KEYMAPTYPE_MAX ];
};


//=============================================================================
/**
 *	プロトタイプ宣言
 */
//=============================================================================
static BOOL STRCODE_Search( const STRCODE* data, const STRCODE* code );

//=============================================================================
/**
 *				キー配列
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	データ読みこみ
 *
 *	@param	NAMEIN_KEYMAP type	タイプ
 *	@param	heapID						ヒープID
 *
 *	@return	データハンドル
 */
//-----------------------------------------------------------------------------
NAMEIN_KEYMAP *NAMEIN_KEYMAP_Alloc( NAMEIN_KEYMAPTYPE type, HEAPID heapID )
{	
	NAMEIN_KEYMAP	*p_wk;

	GF_ASSERT( type < NAMEIN_KEYMAPTYPE_MAX );

	//narcのインデックスがタイプ順になっているので、
	//そのまま読み込む
	p_wk	= GFL_ARC_UTIL_Load( ARCID_NAMEIN_DATA, NARC_namein_data_keymap_kana_dat + type, FALSE, heapID );

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	データ破棄
 *
 *	@param	NAMEIN_KEYMAP *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void NAMEIN_KEYMAP_Free( NAMEIN_KEYMAP *p_wk )
{	
	GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  メモリ上に全データをおくためのハンドル作成
 *
 *	@param	HEAPID heapID   ヒープID
 *
 *	@return KEYMAPをすべてメモリ上におくためのワーク
 */
//-----------------------------------------------------------------------------
NAMEIN_KEYMAP_HANDLE * NAMEIN_KEYMAP_HANDLE_Alloc( HEAPID heapID )
{ 
  NAMEIN_KEYMAP_HANDLE *p_wk;
  p_wk= GFL_HEAP_AllocMemory( heapID, sizeof(NAMEIN_KEYMAP_HANDLE) );
  GFL_STD_MemClear( p_wk, sizeof(NAMEIN_KEYMAP_HANDLE) );

  {
    int i;
    for( i = 0; i < NAMEIN_KEYMAPTYPE_MAX; i++ )
    { 
      p_wk->p_data[i] = NAMEIN_KEYMAP_Alloc( i, heapID );
    }
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ハンドル破棄
 *
 *	@param	NAMEIN_KEYMAP_HANDLE *p_handleハンドル
 */
//-----------------------------------------------------------------------------
void NAMEIN_KEYMAP_HANDLE_Free( NAMEIN_KEYMAP_HANDLE *p_handle )
{ 
  {
    int i;
    for( i = 0; i < NAMEIN_KEYMAPTYPE_MAX; i++ )
    { 
      NAMEIN_KEYMAP_Free( p_handle->p_data[i] );
    }
  }

  GFL_HEAP_FreeMemory( p_handle );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ハンドルからデータを取得
 *
 *	@param	NAMEIN_KEYMAP_HANDLE *p_handle ハンドル
 */
//-----------------------------------------------------------------------------
NAMEIN_KEYMAP *NAMEIN_KEYMAP_HANDLE_GetData( NAMEIN_KEYMAP_HANDLE *p_handle, NAMEIN_KEYMAPTYPE type )
{ 
  return p_handle->p_data[ type ];
}


//----------------------------------------------------------------------------
/**
 *	@brief	データ幅を取得
 *
 *	@param	const NAMEIN_KEYMAP *cp_wk	ワーク
 *
 *	@return	幅
 */
//-----------------------------------------------------------------------------
u16 NAMEIN_KEYMAP_GetWidth( const NAMEIN_KEYMAP *cp_wk )
{	
	return cp_wk->w;
}
//----------------------------------------------------------------------------
/**
 *	@brief	データ高さを取得
 *
 *	@param	const NAMEIN_KEYMAP *cp_wk	ワーク
 *
 *	@return	高さ
 */
//-----------------------------------------------------------------------------
u16 NAMEIN_KEYMAP_GetHeight( const NAMEIN_KEYMAP *cp_wk )
{	
	return cp_wk->h;
}
//----------------------------------------------------------------------------
/**
 *	@brief	指定位置の文字を取得
 *
 *	@param	const NAMEIN_KEYMAP *cp_wk	ワーク
 *	@param	x	X位置
 *	@param	y	Y位置
 *
 *	@return	文字を取得
 */
//-----------------------------------------------------------------------------
STRCODE NAMEIN_KEYMAP_GetStr( const NAMEIN_KEYMAP *cp_wk, u16 x, u16 y )
{	
	GF_ASSERT( x < NAMEIN_KEYMAP_GetWidth(cp_wk) );
	GF_ASSERT( y < NAMEIN_KEYMAP_GetHeight(cp_wk) );

	return cp_wk->str[  y * NAMEIN_KEYMAP_GetWidth( cp_wk ) + x ];
}
//=============================================================================
/**
 *				変換
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	データ読みこみ
 *
 *	@param	NAMEIN_STRCHANGE type	タイプ
 *	@param	heapID						ヒープID
 *
 *	@return	データハンドル
 */
//-----------------------------------------------------------------------------
NAMEIN_STRCHANGE *NAMEIN_STRCHANGE_Alloc( NAMEIN_STRCHANGETYPE type, HEAPID heapID )
{	
	NAMEIN_STRCHANGE	*p_wk;

	GF_ASSERT( type < NAMEIN_STRCHANGETYPE_MAX );

	//narcのインデックスがタイプ順になっているので、
	//そのまま読み込む
	p_wk	= GFL_ARC_UTIL_Load( ARCID_NAMEIN_DATA, NARC_namein_data_change_dakuten_dat + type, FALSE, heapID );

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	データ破棄
 *
 *	@param	NAMEIN_STRCHANGE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void NAMEIN_STRCHANGE_Free( NAMEIN_STRCHANGE *p_wk )
{	
	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	データ幅を取得
 *
 *	@param	const NAMEIN_STRCHANGE *cp_wk	ワーク
 *
 *	@return	幅
 */
//-----------------------------------------------------------------------------
u16 NAMEIN_STRCHANGE_GetNum( const NAMEIN_STRCHANGE *cp_wk )
{	
	return cp_wk->num;
}
//----------------------------------------------------------------------------
/**
 *	@brief	変換前文字列取得
 *
 *	@param	const NAMEIN_STRCHANGE *cp_wk	ワーク
 *	@param	idx				インデックス
 *	@param	*p_code		受け取り配列
 *	@param	code_len	受け取り配列要素数
 *	@param	*p_len		何文字あるか
 */
//-----------------------------------------------------------------------------
void NAMEIN_STRCHANGE_GetInputStr( const NAMEIN_STRCHANGE *cp_wk, u16 idx, STRCODE *p_code, u8 code_len, u8 *p_len )
{	
	GF_ASSERT( idx < NAMEIN_STRCHANGE_GetNum(cp_wk) );
	STRTOOL_Copy( cp_wk->word[ idx ].input, p_code, code_len );	
	if( p_len )
	{	
		for( *p_len = 0; *p_len < NAMEIN_STRCHANGE_CODE_LEN && p_code[*p_len] != NAMEIN_STRCHANGE_EMPTY ; (*p_len)++ )
		{	};
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief	変換後文字列取得
 *
 *	@param	const NAMEIN_STRCHANGE *cp_wk	ワーク
 *	@param	idx				インデックス
 *	@param	*p_code		受け取り配列
 *	@param	code_len	受け取り配列要素数
 *	@param	*p_len		何文字あるか
 *	@return	TRUEならば存在、FALSEならば存在しない
 */
//-----------------------------------------------------------------------------
BOOL NAMEIN_STRCHANGE_GetChangeStr( const NAMEIN_STRCHANGE *cp_wk, u16 idx, STRCODE *p_code, u8 code_len, u8 *p_len )
{	
	u8	len;

	GF_ASSERT( idx < NAMEIN_STRCHANGE_GetNum(cp_wk) );
	STRTOOL_Copy( cp_wk->word[ idx ].chage, p_code, code_len );	
	for( len = 0; len < NAMEIN_STRCHANGE_CODE_LEN && p_code[len] != NAMEIN_STRCHANGE_EMPTY ; (len)++ )
	{	};
	if( p_len )
	{	
		*p_len = len;
	}
	return len != 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	SHIFT変換後文字列取得
 *
 *	@param	const NAMEIN_STRCHANGE *cp_wk	ワーク
 *	@param	idx				インデックス
 *	@param	*p_code		受け取り配列
 *	@param	code_len	受け取り配列要素数
 *	@param	*p_len		何文字あるか
 *	@return	TRUEならば存在、FALSEならば存在しない
 */
//-----------------------------------------------------------------------------
BOOL NAMEIN_STRCHANGE_GetChangeShiftStr( const NAMEIN_STRCHANGE *cp_wk, u16 idx, STRCODE *p_code, u8 code_len, u8 *p_len )
{	
	u8	len;

	GF_ASSERT( idx < NAMEIN_STRCHANGE_GetNum(cp_wk) );
	STRTOOL_Copy( cp_wk->word[ idx ].shift, p_code, code_len );	
	for( len = 0; len < NAMEIN_STRCHANGE_CODE_LEN && p_code[len] != NAMEIN_STRCHANGE_EMPTY ; (len)++ )
	{	};
	if( p_len )
	{	
		*p_len = len;
	}
	return len != 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	変換後に残る文字列取得
 *
 *	@param	const NAMEIN_STRCHANGE *cp_wk	ワーク
 *	@param	idx				インデックス
 *	@param	*p_code		受け取り配列
 *	@param	code_len	受け取り配列要素数
 *	@param	*p_len		何文字あるか
 *	@return	TRUEならば存在、FALSEならば存在しない
 */
//-----------------------------------------------------------------------------
BOOL NAMEIN_STRCHANGE_GetRestStr( const NAMEIN_STRCHANGE *cp_wk, u16 idx, STRCODE *p_code, u8 code_len, u8 *p_len )
{	
	u8	len;

	GF_ASSERT( idx < NAMEIN_STRCHANGE_GetNum(cp_wk) );
	STRTOOL_Copy( cp_wk->word[ idx ].rest, p_code, code_len );
	for( len = 0; len < NAMEIN_STRCHANGE_CODE_LEN && p_code[len] != NAMEIN_STRCHANGE_EMPTY ; (len)++ )
	{	};
	if( p_len )
	{	
		*p_len = len;
	}
	return len != 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief	変換前文字列から、インデックスを得る
 *
 *	@param	const NAMEIN_STRCHANGE *cp_wk	ワーク
 *	@param	*p_code												変換前文字列
 *
 *	@return	インデックス	なかった場合はNAMEIN_STRCHANGE_NONE
 */
//-----------------------------------------------------------------------------
u16 NAMEIN_STRCHANGE_GetIndexByInputStr( const NAMEIN_STRCHANGE *cp_wk, const STRCODE *cp_code )
{	
	int i;
	u16 max;

	max	= NAMEIN_STRCHANGE_GetNum(cp_wk);
	for( i = 0; i < max; i++ )
	{	
		if( STRCODE_Search( cp_wk->word[ i ].input, cp_code) )
		{	
			return i;
		}
	}

	return NAMEIN_STRCHANGE_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	検索文字列をサーチ
 *
 *	@param	const STRCODE* data	データ
 *	@param	STRCODE* code				検索文字列
 *
 *	@return	TRUEで成功
 */
//-----------------------------------------------------------------------------
static BOOL STRCODE_Search( const STRCODE* data, const STRCODE* code )
{
	STRCODE EOMCODE = GFL_STR_GetEOMCode();
	int i;

	for( i = 0; i < NAMEIN_STRCHANGE_CODE_LEN; i++ )
	{
		if( *data == NAMEIN_STRCHANGE_EMPTY
				&& *code == EOMCODE )
		{	
			return TRUE;
		}
		if( *data != *code )
		{ 
			return FALSE; 
		}
		if( *code == EOMCODE )
		{	
			return FALSE;
		}
		data++;
		code++;
	}

	//ここにくる条件は
	//3文字あった

	return TRUE;
}
