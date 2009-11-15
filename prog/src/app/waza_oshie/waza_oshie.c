//============================================================================================
/**
 * @file  waza_oshie.c
 * @brief 技教え/思い出し処理
 * @author  Hiroyuki Nakamura / Akito Mori(WBに引越し)
 * @date  09.10.13
 */
//============================================================================================
#include <gflib.h>
#include "savedata/mystatus.h"
#include "savedata/config.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_personal.h"

#define WAZA_OSHIE_H_GLOBAL
#include "app/waza_oshie.h"

// --------poke_tool.hになかったので一旦こっちに持ってきた----------------
// 技覚えテーブルに関する定義
#define LEVELUPWAZA_OBOE_NUM  ( 20 )              ///<技20個
#define LEVELUPWAZA_OBOE_MAX  ( LEVELUPWAZA_OBOE_NUM+2 )    ///<技20個＋終端コード:1
#define LEVELUPWAZA_OBOE_BUFLEN ( (LEVELUPWAZA_OBOE_MAX+1)*2) ///<（技20個＋終端コード:1+4バイト境界補正:1）* 2バイト
#define LEVELUPWAZA_OBOE_END  ( 0xffff )
#define LEVELUPWAZA_LEVEL_MASK  ( 0xfe00 )
#define LEVELUPWAZA_WAZA_MASK   ( 0x01ff )
#define LEVELUPWAZA_LEVEL_SHIFT (      9 )
// ------------------------------------------------------------------------


#define LV_WAZA_OBOE_MAX  ( LEVELUPWAZA_OBOE_MAX )  // レベルアップで覚える技数 + 終端コード
#define LV_WAZA_OBOE_END  ( LEVELUPWAZA_OBOE_END )  // 終端コード
#define LV_WAZA_LV_MASK   ( LEVELUPWAZA_LEVEL_MASK )  // 覚えるレベル部分
#define LV_WAZA_WAZA_MASK   ( LEVELUPWAZA_WAZA_MASK ) // 覚える技部分
#define LV_WAZA_LV_SHIFT  ( LEVELUPWAZA_LEVEL_SHIFT ) // レベル取得シフト


//--------------------------------------------------------------------------------------------
/**
 * 技教えデータ領域取得
 *
 * @param heap  ヒープID
 *
 * @return  技教えデータ領域
 */
//--------------------------------------------------------------------------------------------
WAZAOSHIE_DATA * WAZAOSHIE_DataAlloc( HEAPID heap )
{
  WAZAOSHIE_DATA * dat = GFL_HEAP_AllocMemory( heap, sizeof(WAZAOSHIE_DATA) );

  GFL_STD_MemFill( dat, 0, sizeof(WAZAOSHIE_DATA) );

  return dat;
}

//--------------------------------------------------------------------------------------------
/**
 * 技教えデータ領域解放
 *
 * @param heap  ヒープID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
void WAZAOSHIE_DataFree( WAZAOSHIE_DATA * dat )
{
  GFL_HEAP_FreeMemory( dat );
}

extern void POKE_PERSONAL_LoadWazaOboeTable( u16 mons_no, u16 form_no, u16* dst );


//--------------------------------------------------------------------------------------------
/**
 * 思い出し可能な技を取得
 *
 * @param pp    ポケモンデータ
 * @param heap  ヒープID
 *
 * @return  取得した技データ
 *
 *  解放は各自で
 */
//--------------------------------------------------------------------------------------------
u16 * WAZAOSHIE_GetRemaindWaza( POKEMON_PARAM * pp, HEAPID heap )
{
  u16 * get;
  u16 * ret;
  u16 waza[4];
  u16 mons;
  u8  lv;
  u8  form;
  u8  h, i, j;

  mons = (u16)PP_Get( pp, ID_PARA_monsno, NULL );
  form = (u8)PP_Get( pp, ID_PARA_form_no, NULL );
  lv   = (u8)PP_Get( pp, ID_PARA_level, NULL );
  for( i=0; i<4; i++ ){
    waza[i] = (u16)PP_Get( pp, ID_PARA_waza1+i, NULL );
  }

  get = GFL_HEAP_AllocMemory( heap, LV_WAZA_OBOE_MAX * 2 );
  ret = GFL_HEAP_AllocMemory( heap, LV_WAZA_OBOE_MAX * 2 );

//  PokeFormNoWazaOboeDataGet( mons, form, get );
  POKE_PERSONAL_LoadWazaOboeTable( mons, form, get );  // allocではなくLoadに変更

  j = 0;
  for( i=0; i<LV_WAZA_OBOE_MAX; i++ ){
    // 終端コードチェック
    if( get[i] == LV_WAZA_OBOE_END ){
      ret[j] = WAZAOSHIE_TBL_MAX;
      break;
    // 現在のレベルで覚えられる技か
    }else if( ((get[i]&LV_WAZA_LV_MASK)>>LV_WAZA_LV_SHIFT) > lv ){
      continue;
    }else{
      get[i] &= LV_WAZA_WAZA_MASK;
      // すでに覚えているか
      for( h=0; h<4; h++ ){
        if( get[i] == waza[h] ){ break; }
      }
      if( h == 4 ){
        // テーブルに同じ技がないか
        for( h=0; h<j; h++ ){
          if( ret[h] == get[i] ){ break; }
        }
        if( h == j ){
          ret[j] = get[i];
          j++;
        }
      }
    }
  }

  GFL_HEAP_FreeMemory( get );

  return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * 教えられる/思い出せる技があるかチェック
 *
 * @param tbl   技テーブル
 *
 * @retval  "TRUE = あり"
 * @retval  "FALSE = なし"
 */
//--------------------------------------------------------------------------------------------
BOOL WAZAOSHIE_WazaTableChack( u16 * tbl )
{
  if( tbl[0] == WAZAOSHIE_TBL_MAX ){
    return FALSE;
  }
  return TRUE;
}
