//============================================================================================
/**
 * @file  cygnus_savedata.c
 * @brief   3DS連動：内部確保領域
 * @date    2011.08.24
 * @author  tamada
 *
 * 2012.02.04 セーブ失敗での領域破壊時に備えてCRCを導入
 */
//============================================================================================

#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/cygnus_savedata.h"
//#include "savedata/cygnus_local.h"

//============================================================================================
//============================================================================================

//============================================================================================
//============================================================================================
#define RAND_MUL ( (1566083941LL << 32) + 1812433253LL )
#define RAND_ADD ( 2531011 )
//----------------------------------------------------------
//----------------------------------------------------------
static u64 rand( u64 before )
{
  return (u64)RAND_MUL * before + (u64)RAND_ADD;
}

//----------------------------------------------------------
//----------------------------------------------------------
enum { CYGNUS_MAGIC_NUMBER = 'CRGF' };

///ctrl_crcで保護する領域のサイズ　＝　先頭からctrl_crcの直前まで
#define CYGSAVE_CTRLAREA_SIZE ( (u32)&(((CYGNUS_SAVEDATA *)0)->ctrl_crc) )

///data_crcで保護する領域のサイズ
#define CYGSAVE_DATAAREA_SIZE ( sizeof(u32) * CYGDATA_AREA_SIZE )

SDK_COMPILER_ASSERT( CYGSAVE_CTRLAREA_SIZE == 16 );
SDK_COMPILER_ASSERT( sizeof(CYGNUS_SAVEDATA) == 152 );

//============================================================================================
//============================================================================================
//----------------------------------------------------------
/**
 * @brief
 */
//----------------------------------------------------------
int CYGNUS_SAVEDATA_GetWorkSize( void )
{
  return sizeof(CYGNUS_SAVEDATA);
}

//----------------------------------------------------------
/**
 * @brief
 */
//----------------------------------------------------------
void CYGNUS_SAVEDATA_Init( CYGNUS_SAVEDATA * sv )
{
  sv->overwrite_enable_flag = FALSE;
  sv->onetime_key = 0;
  sv->magic_number = 0;
  sv->flags = 0;
  sv->ctrl_crc = 0;
}

//----------------------------------------------------------
/**
 * @brief   セーブデータの状態を取得する
 * @param sv            CYGNUS_SAVEDATAへのポインタ
 * @return  CYGSAVE_STATE
 */
//----------------------------------------------------------
CYGSAVE_STATE CYGNUS_SAVEDATA_GetStatus( const CYGNUS_SAVEDATA * sv )
{
  if ( sv->magic_number != CYGNUS_MAGIC_NUMBER ) return CYGSAVE_STATE_NON;
  if ( sv->ctrl_crc != GFL_STD_CrcCalc( sv, CYGSAVE_CTRLAREA_SIZE ) ) return CYGSAVE_STATE_NON;
  if ( sv->overwrite_enable_flag == TRUE ) return CYGSAVE_STATE_SEND;

  if ( sv->data_crc == GFL_STD_CrcCalc( sv->data_area, CYGSAVE_DATAAREA_SIZE ) )
  {
    return CYGSAVE_STATE_RECEIVE;
  }
  //データ領域のCRCが壊れている場合
  return CYGSAVE_STATE_BREAK;
}

//----------------------------------------------------------
/**
 * @brief CYGNUSデータを書き込み可能状態にする
 * @param sv            CYGNUS_SAVEDATAへのポインタ
 * @param onetime_key   暗号化に使用するワンタイムキー
 * @param flags         CYGNUSに伝える情報
 */
//----------------------------------------------------------
void CYGNUS_SAVEDATA_SetSend( CYGNUS_SAVEDATA * sv, u32 onetime_key, u32 flags )
{
  u32 xor_value = rand( onetime_key ) >> 32;

  if( sv->onetime_key == 0 )
  { //ゼロはCYGNUSから書き込んだときの値なので、SWANからは書き込まない 
    sv->onetime_key = CYGNUS_MAGIC_NUMBER;//ゼロ以外の値であればよい
  }

  sv->overwrite_enable_flag = TRUE;
  sv->onetime_key = onetime_key;
  sv->magic_number = CYGNUS_MAGIC_NUMBER;
  sv->flags = flags ^ xor_value;
  sv->ctrl_crc = GFL_STD_CrcCalc( sv, CYGSAVE_CTRLAREA_SIZE );
}

//----------------------------------------------------------
/**
 * @brief CYGNUSデータの受け取り
 */
//----------------------------------------------------------
CYGSAVE_STATE CYGNUS_SAVEDATA_Receive( const CYGNUS_SAVEDATA * sv, u32 onetime_key, u32 * rec_buf )
{
  u32 xor_value = rand( onetime_key ) >> 32;
  u32 i;
  CYGSAVE_STATE state = CYGNUS_SAVEDATA_GetStatus( sv );
  for ( i = 0; i < CYGDATA_AREA_SIZE; i ++ )
  {
    rec_buf[i] = sv->data_area[i] ^ xor_value;
  }
  return state;
}

#ifdef  PM_DEBUG
//============================================================================================
//============================================================================================
//----------------------------------------------------------
/**
 * @brief デバッグ用：CYGNUSから：書き込み
 */
//----------------------------------------------------------
CYGSAVE_STATE CYGNUS_SAVEDATA_DEBUG_Write( CYGNUS_SAVEDATA * sv, u32 * rec_buf )
{
  u32 xor_value = rand( sv->onetime_key ) >> 32;
  u32 i;
  CYGSAVE_STATE state = CYGNUS_SAVEDATA_GetStatus( sv );

  if ( state != CYGSAVE_STATE_SEND ) return state;

  for ( i = 0; i < CYGDATA_AREA_SIZE; i ++ )
  {
    sv->data_area[i] = rec_buf[i] ^ xor_value;
  }
  sv->data_crc = GFL_STD_CrcCalc( sv->data_area, CYGSAVE_DATAAREA_SIZE );

  sv->overwrite_enable_flag = FALSE;
  sv->onetime_key = 0;
  sv->flags = 0;
  sv->ctrl_crc = GFL_STD_CrcCalc( sv, CYGSAVE_CTRLAREA_SIZE );
  return state;
}

//----------------------------------------------------------
/**
 * @brief デバッグ用：CYGNUSから：フラグ取得
 */
//----------------------------------------------------------
u32 CYGNUS_SAVEDATA_DEBUG_GetFlags( const CYGNUS_SAVEDATA * sv )
{
  u32 xor_value = rand( sv->onetime_key ) >> 32;
  return sv->flags ^ xor_value;
}

//----------------------------------------------------------
/**
 * @brief デバッグ用：管理領域用CRCを破壊する
 */
//----------------------------------------------------------
void CYGNUS_SAVEDATA_DEBUG_BreakCtrlCRC( CYGNUS_SAVEDATA * sv )
{
  sv->ctrl_crc = GFL_STD_CrcCalc( sv, CYGSAVE_CTRLAREA_SIZE ) ^ 0xffff;
}

//----------------------------------------------------------
/**
 * @brief デバッグ用：データ領域用CRCを破壊する
 */
//----------------------------------------------------------
void CYGNUS_SAVEDATA_DEBUG_BreakDataCRC( CYGNUS_SAVEDATA * sv )
{
  sv->data_crc = GFL_STD_CrcCalc( sv->data_area, CYGSAVE_DATAAREA_SIZE ) ^ 0xffff;
}

//----------------------------------------------------------
/**
 */
//----------------------------------------------------------
BOOL CYGNUS_SAVEDATA_DEBUG_CheckCtrlCRC( CYGNUS_SAVEDATA * sv )
{
  return sv->ctrl_crc == GFL_STD_CrcCalc( sv, CYGSAVE_CTRLAREA_SIZE );
}

//----------------------------------------------------------
/**
 */
//----------------------------------------------------------
BOOL CYGNUS_SAVEDATA_DEBUG_CheckDataCRC( CYGNUS_SAVEDATA * sv )
{
  return sv->data_crc == GFL_STD_CrcCalc( sv->data_area, CYGSAVE_DATAAREA_SIZE);
}
#endif








