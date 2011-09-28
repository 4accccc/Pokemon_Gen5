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
#include "gamesystem/g_power.h"
#include "../../../resource/animal_trail/animal_trail_data.cdat"  // テーブルデータ

#ifdef PM_DEBUG 
//#define ANIMAL_TRAIL_WALK_TEST // 定義時獣道抽選テスト
#endif

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

  u32 count = GPOWER_Calc_AnimalTrailPokeTryCount( DEFAULT_LOTTERY_COUNT );
  for( n = 0; n < count ;n++ )
  {
    for( i = 0; i < (TABLE_TABLE_CNT-1) ;i++ )
    {
      u8 table_probability = sc_table_probability[trail_id][i];
      if( GFUser_GetPublicRand( 100 ) < table_probability )
      {
        main_table_index = i;
        break;
      }
    }
    // ポケモンテーブルが選ばれていたときはすぐに終了（１回の時はそのまま終了）
    if( main_table_index <= ANIMAL_TRAIL_DATA_TABLE_2 )
    {
      break;
    }
  }

  // セーブデータに格納
  {
    ANIMAL_TRAIL_SAVE_SetMainTableIndex( p_save ,trail_id ,main_table_index );        // メインテーブルインデックス
    ANIMAL_TRAIL_SAVE_SetDataTableIndex( p_save ,trail_id ,data_table_index );        // データテーブルインデックス
    ANIMAL_TRAIL_SAVE_SetFemaleNum( p_save ,trail_id ,GFUser_GetPublicRand( 100 ));   // メスになるかどうか 0~99
    ANIMAL_TRAIL_SAVE_SetObjectSetFlag( p_save ,trail_id ,TRUE );                     // 設置フラグ
  }

#ifdef ANIMAL_TRAIL_WALK_TEST // 定義時獣道抽選テスト
  {
    s32 ti,mi,di;
    static s32 lottery_count_all;
    static s32 lottery_count[ANIMAL_TRAIL_NUM];
    static s32 table_cnt[ANIMAL_TRAIL_NUM][TABLE_TABLE_CNT][DATA_TABLE_LEN];
    // 設置しなかったことにする
    ANIMAL_TRAIL_SAVE_SetObjectSetFlag( p_save ,trail_id ,FALSE );
    table_cnt[trail_id][main_table_index][data_table_index]++;
    lottery_count[trail_id]++;
    lottery_count_all++;


    if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_L) || (lottery_count_all%1000 == 0) )
    {
      OS_Printf("全抽選回数:%u\n",lottery_count_all);
      for( ti = 0; ti < ANIMAL_TRAIL_NUM; ti ++ )
      {
        if( lottery_count[ti] > 0 )
        {
          f32 p = ((f32)lottery_count[ti]/lottery_count_all)*100;
          OS_Printf("獣道%u: %3.3f%%\n",ti,p);
          for( mi = 0; mi < TABLE_TABLE_CNT; mi ++ )
          {
            u32 total = 0;
            f32 p;
            for( di = 0; di < DATA_TABLE_LEN; di ++ )
            {
              total += table_cnt[ti][mi][di];
            }
            p = ((f32)total/lottery_count[ti])*100;
            OS_Printf("メインテーブル%u (%3.3f%%):",mi,p);
            for( di = 0; di < DATA_TABLE_LEN; di ++ )
            {
              p = ((f32)table_cnt[ti][mi][di]/lottery_count[ti])*100;
              OS_Printf("%3.3f%% ",p);
            }
            OS_Printf("\n");
          }
          OS_Printf("\n\n");
        }
      }
    }
  }
#endif

}

