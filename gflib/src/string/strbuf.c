//============================================================================================
/**
 * @file	strbuf.c
 * @brief	汎用文字列バッファ型オブジェクト
 */
//============================================================================================
#include "gflib.h"

//--------------------------------------------------------------
/**
 * 定数                                                      
 */
//--------------------------------------------------------------
#define STRBUF_MAGIC_NUMBER		(0xb6f8d2ec)
#define STRBUF_MAX_SIZE			(1024)

//--------------------------------------------------------------
/**
 * 文字列バッファオブジェクト型定義
 */
//--------------------------------------------------------------
typedef struct _STRBUF {
	u16  size;			///< 配列サイズ
	u16  strlen;		///< 文字列長（EOMを含まない）
	u32  magicNumber;	///< 正常に初期化済みであることをチェックするためのナンバー

	STRCODE  buffer[1];	///< 配列本体
}STRBUF;

/** ヘッダサイズ */
#define STRBUF_HEADER_SIZE	(sizeof(STRBUF) - sizeof(STRCODE))

//==============================================================================================
//------------------------------------------------------------------
/**
 * 指定されたポインタがSTRBUFとして有効なものであるかチェック
 *
 * @param   ptr		ポインタ
 *
 * @retval  BOOL	TRUEで有効である
 */
//------------------------------------------------------------------
BOOL
	GFL_STR_CheckBufferValid
		(const void* ptr)
{
	if( (ptr != NULL) && (((const STRBUF*)ptr)->magicNumber == STRBUF_MAGIC_NUMBER) ){
		return TRUE;
	} else {
		return FALSE;
	}
}


//------------------------------------------------------------------
/**
 * 文字列バッファオブジェクト作成
 *
 * @param   size		格納できる文字数（EOMを含む）
 * @param   heapID		作成先ヒープID
 *
 * @retval  STRBUF*		オブジェクトへのポインタ
 */
//------------------------------------------------------------------
STRBUF*
	GFL_STR_CreateBuffer
		( u32 size, HEAPID heapID )
{
	STRBUF* strbuf;

	strbuf = GFL_HEAP_AllocMemory( heapID, STRBUF_HEADER_SIZE + sizeof(STRCODE)*size );

	strbuf->magicNumber = STRBUF_MAGIC_NUMBER;
	strbuf->size = size;
	strbuf->strlen = 0;
	strbuf->buffer[0] = EOM_;

	return strbuf;
}


//------------------------------------------------------------------
/**
 * 文字列バッファオブジェクト削除
 *
 * @param   strbuf		オブジェクトへのポインタ
 *
 */
//------------------------------------------------------------------
void
	GFL_STR_DeleteBuffer
		( STRBUF* strbuf )
{
	GF_ASSERT( GFL_STR_CheckBufferValid( strbuf ) == TRUE );

	// Free 直後に同じアドレスで呼び出されるとMagicNumberがそのまま残ってるので
	strbuf->magicNumber = 0;

	GFL_HEAP_FreeMemory( strbuf );
}


//------------------------------------------------------------------
/**
 * 内容をクリアして空文字列にする
 * （Create 直後はクリアされているので呼ぶ必要は無い）
 *
 * @param   strbuf		オブジェクトへのポインタ
 *
 */
//------------------------------------------------------------------
void
	GFL_STR_ClearBuffer
		( STRBUF* strbuf )
{
	GF_ASSERT( GFL_STR_CheckBufferValid( strbuf ) == TRUE );

	strbuf->strlen = 0;
	strbuf->buffer[0] = EOM_;
}


//------------------------------------------------------------------
/**
 * バッファ間の文字列コピー
 *
 * @param   dst		コピー先バッファオブジェクト
 * @param   src		コピー元バッファオブジェクト
 *
 */
//------------------------------------------------------------------
void
	GFL_STR_CopyBuffer
		( STRBUF* dst, const STRBUF* src )
{
	GF_ASSERT( GFL_STR_CheckBufferValid( dst ) == TRUE );
	GF_ASSERT( GFL_STR_CheckBufferValid( src ) == TRUE );

	if( dst->size > src->strlen )
	{
		GFL_STD_MemCopy( ( void* )src->buffer, dst->buffer,
						( src->strlen + EOM_CODESIZE ) * sizeof(STRCODE) );
		dst->strlen = src->strlen;
		return;
	}
	GF_ASSERT_MSG(0, "STRBUF overflow : dstsize=%d,  srclen=%d", dst->size, src->strlen);
}


//------------------------------------------------------------------
/**
 * 同じ内容のバッファを複製する
 *
 * @param   strbuf		複製元のバッファオブジェクト
 *
 * @retval  STRBUF*		複製されたバッファオブジェクト
 */
//------------------------------------------------------------------
STRBUF*
	GFL_STR_CreateCopyBuffer
		( const STRBUF* origin, HEAPID heapID )
{
	STRBUF* copy;

	GF_ASSERT( GFL_STR_CheckBufferValid( origin ) == TRUE );

	copy = GFL_STR_CreateBuffer( origin->strlen + EOM_CODESIZE, heapID );
	GFL_STR_CopyBuffer( copy, origin );

	return copy;
}


//------------------------------------------------------------------
/**
 * 文字列を比較する
 *
 * @param   str1		文字列１
 * @param   str2		文字列２
 *
 * @retval  BOOL		TRUEで一致
 */
//------------------------------------------------------------------
BOOL
	GFL_STR_CompareBuffer
		( const STRBUF* str1, const STRBUF* str2 )
{
	int i;

	GF_ASSERT( GFL_STR_CheckBufferValid( str1 ) == TRUE );
	GF_ASSERT( GFL_STR_CheckBufferValid( str2 ) == TRUE );

	for(i=0; str1->buffer[i] == str2->buffer[i]; i++)
	{
		if( str1->buffer[i] == EOM_ )
		{
			return TRUE;
		}
	}
	return FALSE;
}


//------------------------------------------------------------------
/**
 * 格納されている文字数を返す
 *
 * @param   strbuf	文字列バッファオブジェクト
 *
 * @retval  u32		文字数
 */
//------------------------------------------------------------------
u32
	GFL_STR_GetBufferLength
		( const STRBUF* str )
{
	GF_ASSERT( GFL_STR_CheckBufferValid( str ) == TRUE );

	return str->strlen;
}


//==============================================================================================
// 生の文字配列を取り扱う関数群
// これらを利用できるモジュールは出来る限り限定する
//==============================================================================================
//------------------------------------------------------------------
/**
 * 生の文字列（EOM終端）をバッファオブジェクトにセットする
 *
 * @param   strbuf		[out] バッファオブジェクトへのポインタ
 * @param   sz			[in]  EOM で終わる文字配列
 *
 */
//------------------------------------------------------------------
void
	GFL_STR_SetStringCode
		( STRBUF* strbuf, const STRCODE* sz )
{
	GF_ASSERT( GFL_STR_CheckBufferValid( strbuf ) == TRUE );

	strbuf->strlen = 0;
	while( *sz != EOM_ )
	{
		if( strbuf->strlen >= (strbuf->size-1) )
		{
			GF_ASSERT_MSG( 0, "STRBUF overflow: busize=%d", strbuf->size);
			break;
		}
		strbuf->buffer[ strbuf->strlen++ ] = *sz++;
	}
	strbuf->buffer[strbuf->strlen] = EOM_;
}


//------------------------------------------------------------------
/**
 * 生の文字列（長さ指定）をバッファオブジェクトにセットする
 *
 * @param   strbuf		[out] バッファオブジェクトへのポインタ
 * @param   str			[in]  文字配列の先頭ポインタ
 * @param   len			[in]  セットする文字数（EOMを含む）
 *
 */
//------------------------------------------------------------------
void
	GFL_STR_SetStringCodeOrderLength
		( STRBUF* strbuf, const STRCODE* str, u32 len )
{
	GF_ASSERT( GFL_STR_CheckBufferValid( strbuf ) == TRUE );

	if( len <= strbuf->size )
	{
		u32 i;

		GFL_STD_MemCopy( ( void* )str, strbuf->buffer, len*sizeof(STRCODE) );

		for(i=0; i<len; i++)
		{
			if(strbuf->buffer[i] == EOM_)
			{
				break;
			}
		}
		strbuf->strlen = i;

		// EOM付きじゃない文字列だったら付けておく
		if( i==len )
		{
			strbuf->buffer[len-1] = EOM_;
		}
		return;
	}
	GF_ASSERT_MSG(0, "STRBUF overflow: bufsize=%d, strlen=%d", strbuf->size, len);
}


//------------------------------------------------------------------
/**
 * バッファから生の文字配列をコピーする
 *
 * @param   strbuf		[in]  バッファオブジェクトへのポインタ
 * @param   ary			[out] コピー先配列
 * @param   arysize		[in]  コピー先配列の要素数
 *
 */
//------------------------------------------------------------------
void
	GFL_STR_GetStringCode
		( const STRBUF* strbuf, STRCODE* ary, u32 arysize )
{
	GF_ASSERT( GFL_STR_CheckBufferValid( strbuf ) == TRUE );

	if( ( strbuf->strlen + EOM_CODESIZE ) <= arysize )
	{
		GFL_STD_MemCopy( ( void* )strbuf->buffer, ary,
						( strbuf->strlen + EOM_CODESIZE ) * sizeof( STRCODE ) );
		return;
	}

	GF_ASSERT_MSG(0, "STRBUF overflow: strlen=%d, arysize=%d", strbuf->strlen, arysize);
}


//------------------------------------------------------------------
/**
 * バッファオブジェクトが内包する文字配列のアドレスを返す
 * ※この関数を呼び出す箇所は超限定。たぶん文字出力系のみ。
 *
 * @param   strbuf				[in] バッファオブジェクトへのポインタ
 *
 * @retval  const STRCODE*		文字配列のアドレス
 */
//------------------------------------------------------------------
const STRCODE* 
	GFL_STR_GetStringCodePointer
		( const STRBUF* strbuf )
{
	GF_ASSERT( GFL_STR_CheckBufferValid( strbuf ) == TRUE );

	return strbuf->buffer;
}


//==============================================================================================
// 文字列の連結操作を行う関数。【【 原則使用禁止 】】
// 使って良いのはローカライズを考慮した文字列処理モジュールのみ。
//==============================================================================================
//------------------------------------------------------------------
/**
 * 文字列連結
 *
 * @param   dst		連結されるバッファオブジェクト
 * @param   src		連結する文字列を持つバッファオブジェクト
 *
 */
//------------------------------------------------------------------
void
	GFL_STR_AddString
		( STRBUF* dst, const STRBUF* src )
{
	GF_ASSERT( GFL_STR_CheckBufferValid( dst ) == TRUE );
	GF_ASSERT( GFL_STR_CheckBufferValid( src ) == TRUE );

	if( ( dst->strlen + src->strlen + EOM_CODESIZE ) <= dst->size )
		
	{
		GFL_STD_MemCopy( ( void* )src->buffer, &( dst->buffer[ dst->strlen ] ),
							( src->strlen + EOM_CODESIZE ) * sizeof( STRCODE ) );
		dst->strlen += src->strlen;
		return;
	}
	GF_ASSERT_MSG(0, "STRBUF overflow: bufsize=%d, src_len=%d, dst_len=%d",
					dst->size, src->strlen, dst->strlen);
}


//------------------------------------------------------------------
/**
 * １文字連結
 *
 * @param   dst			連結されるバッファオブジェクト
 * @param   code		連結する文字コード
 *
 */
//------------------------------------------------------------------
void
	GFL_STR_AddCode
		( STRBUF* dst, STRCODE code )
{
	GF_ASSERT( GFL_STR_CheckBufferValid( dst ) == TRUE );

	if( (dst->strlen + EOM_CODESIZE) < dst->size )
	{
		dst->buffer[dst->strlen++] = code;
		dst->buffer[dst->strlen] = EOM_;
		return;
	}
	GF_ASSERT_MSG(0, "STRBUF overflow: busize=%d", dst->size);
}


