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
#include "savedata/save_control.h"
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
  DATA_CONVERT_GET_ID_MUSICAL_POINT,
  DATA_CONVERT_GET_ID_MUSICAL_ENTRY,
  DATA_CONVERT_GET_ID_DENDOU_POKE_00, //p_data_adrsからWB_SAVE_CONV_DENDOU_POKEMON_DATA構造体で受け取る
  DATA_CONVERT_GET_ID_DENDOU_POKE_01, //ゞ
  DATA_CONVERT_GET_ID_DENDOU_POKE_02, //ゞ
  DATA_CONVERT_GET_ID_DENDOU_POKE_03, //ゞ
  DATA_CONVERT_GET_ID_DENDOU_POKE_04, //ゞ
  DATA_CONVERT_GET_ID_DENDOU_POKE_05, //ゞ
  DATA_CONVERT_GET_ID_OWNER_SYS,  //p_data_adrsからWB_SAVE_CONV_OWNER構造体で受け取る
  DATA_CONVERT_GET_ID_OWNER_MUSICAL,  //p_data_adrsからWB_SAVE_CONV_OWNER構造体で受け取る
  DATA_CONVERT_GET_ID_OWNER_DENDOU,  //p_data_adrsからWB_SAVE_CONV_OWNER構造体で受け取る
  DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT,  //p_data_adrsからu8のWB_SAVE_CONV_MUSICAL_ITEM_BIT配列で受け取る
  DATA_CONVERT_GET_ID_MAX
} DATA_CONVERT_GET_ID;

//-------------------------------------
///	コンバート元ユーザー
//=====================================
typedef enum
{
  DATA_CONVERT_OWNER_SYS,     //フラグ、ワーク類
  DATA_CONVERT_OWNER_MUSICAL, //ミュージカル
  DATA_CONVERT_OWNER_DENDOU,  //でんどういり
  DATA_CONVERT_OWNER_MAX
} DATA_CONVERT_OWNER;

//サイズ
#define DATA_CONVERT_SAVE_WORK_SIZE (476)

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

extern BOOL DATA_CONVERT_SAVE_Convert( DATA_CONVERT_SAVE *p_wk, const WB_SAVE_CONV_ALL_DATA *cp_all_data, const SAVE_CONTROL_WORK *cp_sv );

extern BOOL DATA_CONVERT_SAVE_IsSameOwner( const DATA_CONVERT_SAVE *cp_wk, const WB_SAVE_CONV_ALL_DATA *cp_all_data, const SAVE_CONTROL_WORK *cp_sv );

extern u32 DATA_CONVERT_SAVE_GetData( const DATA_CONVERT_SAVE *cp_wk, DATA_CONVERT_GET_ID id, void *p_data_adrs );
extern BOOL DATA_CONVERT_SAVE_GetFlag( const DATA_CONVERT_SAVE *cp_wk, WB_SAVE_CONV_FLAG flag );
extern u16 DATA_CONVERT_SAVE_GetWork( const DATA_CONVERT_SAVE *cp_wk, WB_SAVE_CONV_WORK wk );
extern BOOL DATA_CONVERT_SAVE_IsExist( const DATA_CONVERT_SAVE *cp_wk, DATA_CONVERT_OWNER id );
extern u8 DATA_CONVERT_SAVE_GetSex( const DATA_CONVERT_SAVE *cp_wk, DATA_CONVERT_OWNER id );
extern const STRCODE * DATA_CONVERT_SAVE_GetName( const DATA_CONVERT_SAVE *cp_wk, DATA_CONVERT_OWNER id );

extern void DATA_CONVERT_SAVE_SetData( DATA_CONVERT_SAVE *p_wk, DATA_CONVERT_GET_ID id, u32 data );
extern void DATA_CONVERT_SAVE_SetFlag( DATA_CONVERT_SAVE *p_wk, WB_SAVE_CONV_FLAG flag, BOOL on_off );
extern void DATA_CONVERT_SAVE_SetWork( DATA_CONVERT_SAVE *p_wk, WB_SAVE_CONV_WORK wk, u16 value );
