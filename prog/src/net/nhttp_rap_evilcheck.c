//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		nhttp_rap_evilcheck.c
 * @brief	  nhttpで不正チェックするときのレスポンスコードなど
 *	@author	Toru=Nagihashi
 *	@data		2010.02.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
#include <nitro/crypto.h>
#include <nitro/crypto/sign.h>


//外部公開
#include "net/nhttp_rap_evilcheck.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define NHTTP_RAP_HEAPID_NONE   (0xFFFF)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//CRYPTOライブラリ計算で必要なAlloc関数のためヒープID
static HEAPID s_heapID  = NHTTP_RAP_HEAPID_NONE;

//=============================================================================
/**
 *					PRIVATE
*/
//=============================================================================
static void * Nhttp_Rap_EvilCheck_Alloc( u32 size );
static void Nhttp_Rap_EvilCheck_Free( void *p_adrs );

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//------------------------------------------------------------------------------
/**
 * @brief   ポケモン不正検査 ステータスコードを取得
 * @param   cp_data  NHTTP_RAP_GetRecvBufferの中身
 *
 * @retval  ステータスコード  ０ならば成功  １ならば失敗
 */
//------------------------------------------------------------------------------
u8 NHTTP_RAP_EVILCHECK_GetStatusCode( const void *cp_data )
{ 
  //バイト位置０　サイズ１byte
  return *((const u8*)cp_data);
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン不正検査 ポケモン不正検査の結果
 * @param   cp_data     NHTTP_RAP_GetRecvBufferの中身
 * @param   poke_index  不正チェックのときに送ったポケモンの位置
 *
 * @retval  不正検査の結果  （NHTTP_RAP_EVILCHECK_RESULTの項目参照）
 */
//------------------------------------------------------------------------------
NHTTP_RAP_EVILCHECK_RESULT NHTTP_RAP_EVILCHECK_GetPokeResult( const void *cp_data, int poke_index )
{ 
  //バイト位置 1+poke_index*4 サイズ4
  const u8 *p_result  = (((const u8*)cp_data)+1+poke_index*4);

  //ポケモンチェックの値はネットワークバイトオーダーで帰ってくるので
  //リトルエンディアンに直す
  return (u32)( (p_result[0] << 24) | (p_result[1] << 16) | (p_result[2] << 8) | ( p_result[3] << 0 ) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン不正検査  署名を取得
 *
 *  @param    cp_data     NHTTP_RAP_GetRecvBufferの中身
 *  @param    poke_max    不正チェックのときに送ったポケモンの最大数
 *
 *	@return 署名へのアドレス  コピーする場合、ここからNHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN分、memcpyしてください
 */
//-----------------------------------------------------------------------------
const s8 * NHTTP_RAP_EVILCHECK_GetSign( const void *cp_data, int poke_max )
{ 
  //バイト位置 1+4*(poke_max) サイズ128byte
  return (const s8*)(((const u8*)cp_data)+1+poke_max*4);
}


//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン不正検査  署名とポケモンが一致していることを証明する
 *	        内部で計算のため最高で4kbyte程度のメモリを使用する
 *
 *	@param	const void *p_poke_buff ポケモンデータが格納されているバッファ
 *	@param	poke_size               不正チェックの時に送った１体あたりのサイズ
 *	@param	poke_max                不正チェックの時に送ったポケモンの最大数
 *	@param	s8 *cp_sign             不正チェックで受け取ったサイン
 *	@param  HEAPID                  計算のために使用するヒープ
 *
 *	@return TRUE一致している  FALSE一致していない
 */
//-----------------------------------------------------------------------------
BOOL NHTTP_RAP_EVILCHECK_VerifySign( const void *p_poke_buff, int poke_size, int poke_max, const s8 *cp_sign, HEAPID heapID )
{ 
  // 公開鍵
  static unsigned char publicKey[ NHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN ] = 
  {
    0xD9, 0x87, 0xD4, 0x65, 0xE4, 0xEE, 0xAE, 0x58,
    0x2D, 0x01, 0x73, 0x15, 0xF0, 0x0E, 0xA3, 0x40,
    0x0C, 0x51, 0x0B, 0x2E, 0x51, 0xE1, 0x5D, 0x77,
    0xD0, 0x3A, 0xDC, 0xB2, 0x5C, 0x83, 0x01, 0x71,
    0xF5, 0x69, 0xFB, 0xD2, 0x6A, 0x78, 0xDC, 0x69,
    0x69, 0x4D, 0xDD, 0x2C, 0xEF, 0xA4, 0xA9, 0xAA,
    0xD1, 0xA0, 0xD9, 0xAA, 0x99, 0x70, 0x5B, 0xF0,
    0x80, 0x38, 0xF5, 0x77, 0x64, 0xEE, 0xA5, 0xAB,
    0x7D, 0x6A, 0x38, 0x38, 0x67, 0x8A, 0xEC, 0x26,
    0x2E, 0x95, 0x2A, 0x1C, 0xDB, 0xB8, 0xE2, 0xFF,
    0x68, 0xDC, 0x93, 0x2E, 0x7F, 0x8E, 0x3A, 0xEC,
    0xD1, 0xFE, 0x52, 0x82, 0xEA, 0xCA, 0x41, 0x61,
    0xC2, 0x20, 0x3F, 0xF0, 0x98, 0xF7, 0x9D, 0x67,
    0x35, 0xE6, 0x44, 0x14, 0xE1, 0x85, 0xFB, 0xB3,
    0xEC, 0x04, 0x3D, 0x83, 0x8D, 0x9B, 0x4B, 0x19,
    0x07, 0x23, 0x31, 0xC3, 0xF7, 0x98, 0x57, 0xE5
  };

  BOOL ret;

  s_heapID  = heapID;

  CRYPTO_SetMemAllocator(
      Nhttp_Rap_EvilCheck_Alloc,
      Nhttp_Rap_EvilCheck_Free,
      NULL  //ReAllocはRSA暗号化機能のときしか使わない(電子署名では使わない)
      );


  ret = CRYPTO_VerifySignature( p_poke_buff, poke_size * poke_max, cp_sign, publicKey );

  s_heapID  = NHTTP_RAP_HEAPID_NONE;

  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン証明のためのバッファを作成する
 *	        NHTTP_RAP_PokemonEvilCheckCreateは送信バッファの作成ですが、
 *	        こちらは、証明のためにPOKEPARTYからNHTTP_RAP_EVILCHECK_VerifySignで使用するバッファを
 *	        作成するために使います
 *
 *	@param	poke_size               不正チェックの時に送った１体あたりのサイズ
 *	@param	poke_max                不正チェックの時に送ったポケモンの最大数
 *	@param	HEAPID heapID バッファ作成のためのヒープID
 *
 *	@return バッファ
 */
//-----------------------------------------------------------------------------
void * NHTTP_RAP_EVILCHECK_CreateVerifyPokeBuffer( int poke_size, int poke_max, HEAPID heapID )
{
  const u32 size  = poke_max * poke_size;
  void *p_buff;
  p_buff  = GFL_HEAP_AllocMemory( heapID, size );
  GFL_STD_MemClear( p_buff, size );
  return p_buff;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン証明のためのバッファを破棄する
 *
 *	@param	void *p_buff バッファ
 */
//-----------------------------------------------------------------------------
void NHTTP_RAP_EVILCHECK_DeleteVerifyPokeBuffer( void *p_buff )
{ 
  GFL_HEAP_FreeMemory( p_buff );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ポケモン証明バッファへポケモンデータを追加する
 *
 *	@param	void *p_buff            バッファ
 *	@param	void *cp_data           ポケモンデータ
 *	@param	poke_size               不正チェックの時に送った１体あたりのサイズ
 *	@param	poke_index              格納する順番インデックス
 */
//-----------------------------------------------------------------------------
void NHTTP_RAP_EVILCHECK_AddPokeVerifyPokeBuffer( void *p_buff, const void *cp_data, int poke_size, int poke_index )
{ 
  const u32 offset  = poke_size * poke_index;
  GFL_STD_MemCopy( cp_data, (((u8*)p_buff) + offset ), poke_size );
}

//----------------------------------------------------------------------------
/**
 *	@brief  CRYPT計算のためのアロケート関数
 *
 *	@param	u32 size  サイズ
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static void * Nhttp_Rap_EvilCheck_Alloc( u32 size )
{ 
  void *p_adrs;

  GF_ASSERT_MSG( s_heapID != NHTTP_RAP_HEAPID_NONE, "heapIDを設定してください\n" );
    
  p_adrs  = GFL_HEAP_AllocMemory( s_heapID, size );

  GF_ASSERT_MSG( p_adrs, "CRYPT alloc failed!! size=%d =restHeap%d \n", size, GFL_HEAP_GetHeapFreeSize(s_heapID) );

  return p_adrs;
}
//----------------------------------------------------------------------------
/**
 *	@brief  CRYPT計算のためのフリー関数
 *
 *	@param	void *p_adrs  ワーク
 */
//-----------------------------------------------------------------------------
static void Nhttp_Rap_EvilCheck_Free( void *p_adrs )
{ 
  GFL_HEAP_FreeMemory( p_adrs );
}
