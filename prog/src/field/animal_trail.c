//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		animal_trail.c
 *	@brief  けものみち抽選
 *	@author	Masayuki Onoue
 *	@date		2011.05.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "animal_trail.h"
#include <gflib.h>
#include "system/gfl_use.h"
#include "../../../resource/animal_trail/animal_trail_data.cdat"  // テーブルデータ

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

//============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
void ANIMAL_TRAIL_SelectRandomTable( ETC_SAVE_WORK *p_save ,u8 trail_id )
{
  int i,n;
  AT_OBJ_TYPE obj_type;
  u8 main_table_index = TABLE_TABLE_CNT-1;
  u8 data_table_index = GFUser_GetPublicRand( DATA_TABLE_LEN );
  for( i = 0; i < (TABLE_TABLE_CNT-1) ;i++ )
  {
    u8 table_probability = sc_table_probability[trail_id][i];
    if( GFUser_GetPublicRand( 100 ) < table_probability )
    {
      main_table_index = i;
    }
  }

  // セーブデータに格納
  {
    ANIMAL_TRAIL_SAVE_SetMainTableIndex( p_save ,trail_id ,main_table_index );        // メインテーブルインデックス
    ANIMAL_TRAIL_SAVE_SetDataTableIndex( p_save ,trail_id ,data_table_index );        // データテーブルインデックス
    ANIMAL_TRAIL_SAVE_SetFemaleNum( p_save ,trail_id ,GFUser_GetPublicRand( 100 ));   // メスになるかどうか 0~99
    ANIMAL_TRAIL_SAVE_SetObjectSetFlag( p_save ,trail_id ,TRUE );                     // 設置フラグ
  }
  OS_Printf("けものみち%u番にオブジェクトを配置しました(テーブルID：%u)\n",trail_id,main_table_index);

}

