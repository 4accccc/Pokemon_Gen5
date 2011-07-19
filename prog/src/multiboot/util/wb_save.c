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
#include "multiboot/wb_save.h"
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

#ifdef PM_DEBUG
  {
    int i;
    u32 total_size = 0;
    for( i = 0; i < WB_GMDATA_ID_MAX; i++ )
    {
      total_size += WB_BLOCK_SIZE_TABLE[i];
    }
    OS_TPrintf( "TotalSize =%d(0x%x)\n", total_size, total_size );
  }
#endif

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
    if( WB_GMDATA_ID_MAX <= i )
    {
      temp += WB_SAVE_GetSaveSize( i );
    }
    else
    {
      temp += WB_SAVE_GetBlockSize( i );
    }
  }
  OS_TPrintf("[%d][%x]\n",id,temp);
  return temp;
}

//----------------------------------------------------------------------------
/**
 *	@brief  IDごとのブロックサイズ  ２５６アライメントされている
 *
 *	@param	const u16 id　WB_GMDATA_ID
 *
 *	@return IDごとのブロックサイズ
 */
//-----------------------------------------------------------------------------
u32 WB_SAVE_GetBlockSize( const u16 id )
{
  return ALIGN256( WB_BLOCK_SIZE_TABLE[id] );
}

//----------------------------------------------------------------------------
/**
 *	@brief  IDごとのごとのセーブサイズ
 *
 *	@param	const u16 id  WB_GMDATA_ID
 *
 *	@return IDごとのセーブサイズ
 */
//-----------------------------------------------------------------------------
u32 WB_SAVE_GetSaveSize( const u16 id )
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
#if 0
  {
    int i;
    const u16 *cp_crc_table = WB_SAVE_GetBlockAddress( (void *)cp_data_adrs, WB_GMDATA_ID_CRC );

#ifdef PM_DEBUG
    for( i = 0; i < WB_GMDATA_ID_MAX; i++ )
    {
      WB_SAVE_BLOCK_FOOTER *p_block_footer = Wb_Save_GetBlockFooter( (void*)cp_data_adrs, i );
      OS_TPrintf( "GMID[%d] footer_crc[%d]  table_crc[%d]\n", i, p_block_footer->crc, cp_crc_table[i] );
    }
#endif

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
#endif

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
	        WB_SAVE_GetSaveSize(WB_GMDATA_ID_CRC));
	
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
  u32 save_size = WB_SAVE_GetSaveSize( id );

  GF_ASSERT( id < WB_GMDATA_ID_MAX );
  return (void*)((u32)p_block+save_size);
}
#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *	@brief  ダンプ
 *
 *	@param	const void *cp_data_adrs  セーブデータ先頭アドレス
 *
 *	@return TRUEダンプ終了  FALSEダンプ中
 */
//-----------------------------------------------------------------------------
BOOL DEBUG_WB_SAVE_Dump( const void *cp_data_adrs, u32 *p_seq )
{
  enum
  {
    PRINT_DIV_NUM = 120,
  };

  if( (*p_seq) >= PRINT_DIV_NUM )
  {
    return TRUE;
  }
  else
  {
    int i;
    int cnt = 0;

    int start = (*p_seq) * (WB_SAVE_SIZE / PRINT_DIV_NUM);
    int end   = start + (WB_SAVE_SIZE / PRINT_DIV_NUM);
    const u8 *cp_data = (const u8*)cp_data_adrs;

    for( i = start; i <= end; i++ )
    {
      OS_TPrintf( "%x ", cp_data[i] );

      if( cnt++ >= 0x10 )
      {
        OS_TPrintf( "\n" );
        cnt = 0;
      }
    }

    (*p_seq)++;
    return FALSE;
  }
}
#endif
