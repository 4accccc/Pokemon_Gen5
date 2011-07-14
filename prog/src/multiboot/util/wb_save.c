//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wb_save.c
 *	@brief
 *	@author		Toru=Nagihashi
 *	@data		2011.07.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include <backup_system.h>
#include "arc_def.h"
#include "wb_save.cdat"  //mb_data_wb.c 以外でinclude禁止
#include "wb_save.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
static MATHCRC16Table s_crc_table;	///<CRC算出用テーブル


//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
static WB_SAVE_BLOCK_FOOTER * Wb_Save_GetBlockFooter( void *p_data_adrs, const u16 id );

//=============================================================================
/**
 *          外部公開
 */
//=============================================================================
void WB_SAVE_Init( void )
{
	MATH_CRC16CCITTInitTable( &s_crc_table );
}


//----------------------------------------------------------------------------
/**
 *	@brief IDごとのセーブ開始アドレスからのオフセットを取得
 *
 *	@param	const u16 id    WB_GMDATA_ID
 *
 *	@return IDごとのセーブ開始アドレスからのオフセット
 */
//-----------------------------------------------------------------------------
u32 WB_SAVE_GetBlockOffset( const u16 id )
{
  u32 temp = 0;
  u8 i;
  for( i=0;i<id;i++ ){
    temp += WB_SAVE_GetBlockSize( i );
  }
  OS_TPrintf("[%d][%x]\n",id,temp);
  return temp;
}

//----------------------------------------------------------------------------
/**
 *	@brief  IDごとのセーブサイズ
 *
 *	@param	const u16 id　WB_GMDATA_ID
 *
 *	@return IDごとのセーブサイズ
 */
//-----------------------------------------------------------------------------
u32 WB_SAVE_GetBlockSize( const u16 id )
{
  return WB_BLOCK_SIZE_TABLE[id];
}


//----------------------------------------------------------------------------
/**
 *	@brief  IDごとのセーブアドレスを取得
 *
 *	@param	void *p_data_adrs セーブデータ先頭アドレス
 *	@param	u16 id  WB_GMDATA_ID
 *
 *	@return   セーブアドレス
 */
//-----------------------------------------------------------------------------
void * WB_SAVE_GetBlockAddress( void *p_data_adrs, const u16 id )
{
  return (u8*)p_data_adrs + WB_SAVE_GetBlockOffset( id );
}

//----------------------------------------------------------------------------
/**
 *	@brief  セーブデータの正当性をチェック  (全体チェック)
 *
 *	@param	const void *cp_data_adrs  セーブデータ先頭アドレス
 *
 *	@return TRUE正常  FALSE不正
 */
//-----------------------------------------------------------------------------
BOOL WB_SAVE_CheckIllegal( const void *cp_data_adrs )
{
  //各ブロックのチェック
  {
    int i;
    const u16 *cp_crc_table = WB_SAVE_GetBlockAddress( (void *)cp_data_adrs, WB_GMDATA_ID_CRC );

    //ブロックごとのチェック
    for( i = 0; i < WB_GMDATA_ID_MAX; i++ )
    {
      WB_SAVE_BLOCK_FOOTER *p_block_footer = Wb_Save_GetBlockFooter( (void*)cp_data_adrs, i );

      //各ブロックのCRCがCRCがテーブルと一致しているか
      if( p_block_footer->crc != cp_crc_table[i] )
      {
        OS_TPrintf( "GMID[%d] footer_crc[%d] != table_crc[%d]\n", i, p_block_footer->crc, cp_crc_table[i] );
        return FALSE;
      }
    }
  }

  //ラストフッタのチェック
  {
	  //ラストフッタのチェック
	  const WB_SAVE_FOOTER *cp_footer     = WB_SAVE_GetBlockAddress( (void *)cp_data_adrs, WB_GMDATA_ID_FOOTER );

	  if( cp_footer->size != WB_SAVE_SIZE )
	  {
	    OS_TPrintf( "size error! data[%d] != define[%d]\n", cp_footer->size, WB_SAVE_SIZE );
	    return FALSE;
	  }
	
	  if( cp_footer->magic_number != WB_MAGIC_NUMBER )
	  {
	    OS_TPrintf( "magicnum error! data[%d] != define[%d]\n", cp_footer->magic_number, WB_MAGIC_NUMBER );
	    return FALSE;
	  }
	
	  {
      const u16 *cp_crc_table = WB_SAVE_GetBlockAddress( (void *)cp_data_adrs, WB_GMDATA_ID_CRC );
	
	    u16 crc = MATH_CalcCRC16CCITT(&s_crc_table, 
	        cp_crc_table,
	        WB_SAVE_GetBlockSize(WB_GMDATA_ID_CRC));
	
	    if( cp_footer->save_block_crc_tbl_crc != crc )
	    {
	      OS_TPrintf( "crctable error! data[%d] != calc[%d]\n", cp_footer->save_block_crc_tbl_crc, crc);
	      return FALSE;
	    }
	  }
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ブロックごとのブロックフッターを取得
 *
 *	@param	void *p_data_adrs セーブデータ先頭アドレス
 *	@param	u16 id  WB_GMDATA_ID
 *
 *	@return ブロックフッターを取得
 */
//-----------------------------------------------------------------------------
static WB_SAVE_BLOCK_FOOTER * Wb_Save_GetBlockFooter( void *p_data_adrs, const u16 id )
{
  void *p_block = WB_SAVE_GetBlockAddress( p_data_adrs, id );
  u32 block_offset = WB_SAVE_GetBlockSize( id ) - sizeof(WB_SAVE_BLOCK_FOOTER);

  GF_ASSERT( id < WB_GMDATA_ID_MAX );

  //TODO もし生セーブ構造体が4byteアライメントされてないとずれるかも…

  return (void*)((u32)p_block+block_offset);
}
