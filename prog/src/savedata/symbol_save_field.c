//==============================================================================
/**
 * @file    symbol_save_field.c
 * @brief   シンボルエンカウント用セーブデータ：fieldmapオーバーレイに配置
 * @author  matsuda
 * @date    2010.03.03(水)
 *
 * フィールドでしか使用しないシンボルエンカウントセーブアクセス群
 */
//==============================================================================
#include <gflib.h>
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/symbol_save.h"
#include "savedata/symbol_save_notwifi.h"
#include "savedata/symbol_save_field.h"
#include "symbol_save_local.h"


//==================================================================
/**
 * シンボルポケモンを登録
 *
 * @param   symbol_save		シンボルセーブ領域へのポインタ
 * @param   monsno        ポケモン番号
 * @param   wazano		    技番号
 * @param   sex		        性別(PTL_SEX_MALE, PTL_SEX_FEMALE, PTL_SEX_UNKNOWN)
 * @param   form_no		    フォルム番号
 * @param   zone_type     SYMBOL_ZONE_TYPE
 * @return  BOOL          TRUEのとき、登録できた
 */
//==================================================================
BOOL SymbolSave_Field_Set( SYMBOL_SAVE_WORK *symbol_save,
    u16 monsno, u16 wazano, u8 sex, u8 form_no, u8 move_type, SYMBOL_ZONE_TYPE zone_type )
{
  if ( zone_type == SYMBOL_ZONE_TYPE_FREE_LARGE || zone_type == SYMBOL_ZONE_TYPE_FREE_SMALL )
  {
    //マップ拡張チェックなども含めてまかせてしまう
    SymbolSave_SetFreeZone( symbol_save, monsno, wazano, sex, form_no, move_type, zone_type );
  }
  else
  {
    u32 no, start, end;
    no = SymbolSave_CheckSpace( symbol_save, zone_type );
    if ( no == SYMBOL_SPACE_NONE ) return FALSE;
#if 0
    if ( no == SYMBOL_SPACE_NONE ) {
      SymbolSave_DataShift( symbol_save, start );
      no = end - 1;
    }
#endif
    SymbolSave_Local_Decode(symbol_save);  //暗号化解除
    symbol_save->symbol_poke[ no ].monsno = monsno;
    symbol_save->symbol_poke[ no ].wazano = wazano;
    symbol_save->symbol_poke[ no ].sex = sex;
    symbol_save->symbol_poke[ no ].form_no = form_no;
    symbol_save->symbol_poke[ no ].move_type = move_type;
    SymbolSave_Local_Encode(symbol_save);  //再び暗号化
  }
  return TRUE;
}

//==================================================================
/**
 * シンボルポケモンの移動
 *
 * @param   symbol_save
 * @param   now_no          移動対象ポケモンの配置No.
 *
 * 配置No.から現在のゾーンタイプを自動判別し、適切な移動を行う
 */
//==================================================================
BOOL SymbolSave_Field_MoveAuto( SYMBOL_SAVE_WORK *symbol_save, u32 now_no )
{
  SYMBOL_ZONE_TYPE now_type = SYMBOLZONE_GetZoneTypeFromNumber( now_no );
  SYMBOL_ZONE_TYPE new_type;
  u32 new_no;

  switch ( now_type )
  {
  case SYMBOL_ZONE_TYPE_KEEP_LARGE: //キープゾーン（大）→フリーゾーン（大）
    new_type = SYMBOL_ZONE_TYPE_FREE_LARGE;
    break;
  case SYMBOL_ZONE_TYPE_KEEP_SMALL: //キープゾーン（小）→フリーゾーン（小）
    new_type = SYMBOL_ZONE_TYPE_FREE_SMALL;
    break;
  case SYMBOL_ZONE_TYPE_FREE_LARGE: //フリーゾーン（大）→キープゾーン（大）
    new_type = SYMBOL_ZONE_TYPE_KEEP_LARGE;
    break;
  case SYMBOL_ZONE_TYPE_FREE_SMALL: //フリーゾーン（小）→キープゾーン（小）
    new_type = SYMBOL_ZONE_TYPE_KEEP_SMALL;
    break;
  default:
    GF_ASSERT( 0 );
    return FALSE;
  }

  new_no = SymbolSave_CheckSpace(symbol_save, new_type);
#ifdef  PM_DEBUG
  {
    static const char * const typenames[] = { "Keep Large", "Keep Small", "Free Large", "Free Small" };
    TAMADA_Printf("%s(%3d)-->%s(%3d)", typenames[now_type], now_no, typenames[new_type], new_no);
  }
#endif

  if(new_no == SYMBOL_SPACE_NONE){
    TAMADA_Printf(":Failure\n");
    return FALSE;

  } else {
    SymbolSave_Local_Decode(symbol_save);  //暗号化解除
    //新しい場所に代入する
    symbol_save->symbol_poke[new_no] = symbol_save->symbol_poke[now_no];
    if ( new_type == SYMBOL_ZONE_TYPE_FREE_SMALL )
    { //マップが拡張するか再計算
      SymbolSave_Local_CalcSmallLevel( symbol_save, new_no );
    }
    SymbolSave_Local_Encode(symbol_save);  //再び暗号化
    //元のゾーンを前詰め
    SymbolSave_DataShift(symbol_save, now_no);
    TAMADA_Printf(":Success\n");
    return TRUE;
  }
}

