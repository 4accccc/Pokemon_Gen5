//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		data_convert_save.h
 *	@brief  データコンバートセーブ
 *	@author	Toru=Nagihashi
 *	@date		2011.07.15
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "multiboot/wb_save_convert.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	取得ID
//=====================================
typedef enum
{
  DATA_CONVERT_GET_ID_,
  DATA_CONVERT_GET_ID_MAX
} DATA_CONVERT_GET_ID;

//サイズ
#define DATA_CONVERT_SAVE_WORK_SIZE (424)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	データコンバートセーブデータ
//=====================================
typedef struct _DATA_CONVERT_SAVE DATA_CONVERT_SAVE;

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
extern u32  DATA_CONVERT_SAVE_GetWorkSize( void );
extern void DATA_CONVERT_SAVE_Init( DATA_CONVERT_SAVE *p_wk );

extern void DATA_CONVERT_SAVE_Convert( DATA_CONVERT_SAVE *p_wk, const WB_SAVE_CONV_ALL_DATA *cp_all_data );

extern BOOL DATA_CONVERT_SAVE_IsSameOwner( const DATA_CONVERT_SAVE *cp_wk, const WB_SAVE_CONV_OWNER *cp_owner );

extern u32 DATA_CONVERT_SAVE_GetData( const DATA_CONVERT_SAVE *cp_wk, DATA_CONVERT_GET_ID id, void *p_data_adrs );
extern BOOL DATA_CONVERT_SAVE_GetFlag( const DATA_CONVERT_SAVE *cp_wk, WB_SAVE_CONV_FLAG flag );
extern u16 DATA_CONVERT_SAVE_GetWork( const DATA_CONVERT_SAVE *cp_wk, WB_SAVE_CONV_WORK wk );
