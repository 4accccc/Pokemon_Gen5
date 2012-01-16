//============================================================================================
/**
 * @file  cygnus_savedata.c
 * @brief   3DS連動：内部確保領域
 * @date    2011.08.24
 * @author  tamada
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
}

//----------------------------------------------------------
//----------------------------------------------------------
CYGSAVE_STATE CYGNUS_SAVEDATA_GetStatus( const CYGNUS_SAVEDATA * sv )
{
  if ( sv->magic_number != CYGNUS_MAGIC_NUMBER ) return CYGSAVE_STATE_NON;
  if ( sv->overwrite_enable_flag == TRUE ) return CYGSAVE_STATE_SEND;
}

//----------------------------------------------------------
/**
 * @brief CYGNUSデータを書き込み可能状態にする
 */
//----------------------------------------------------------
void CYGNUS_SAVEDATA_SetSend( CYGNUS_SAVEDATA * sv, u32 onetime_key, u32 flags )
{
  u32 xor_value = rand( onetime_key ) >> 32;

  sv->overwrite_enable_flag = TRUE;
  sv->onetime_key = onetime_key;
  sv->magic_number = CYGNUS_MAGIC_NUMBER;
  sv->flags = flags ^ xor_value;
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
 * @brief デバッグ用：CYGNUSからの書き込み
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
  sv->overwrite_enable_flag = FALSE;
  sv->onetime_key = 0;
  return state;
}
#endif








