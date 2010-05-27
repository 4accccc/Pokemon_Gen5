//=============================================================================================
/**
 * @file  btl_tables.c
 * @brief ポケモンWB てきとうなサイズのテーブルをVRAM_Hに置いてアクセスする
 * @author  taya
 *
 * @date  2010.05.27  作成
 */
//=============================================================================================

#include "poke_tool\poketype.h"
#include "poke_tool\monsno_def.h"
#include "waza_tool\wazano_def.h"
#include "item\itemsym.h"
#include "item\item.h"


#include "btl_common.h"
#include "btl_tables.h"


/**
 *  アンコール除外対象のワザか判定
 */
BOOL BTL_TABLES_IsMatchEncoreFail( WazaID waza )
{
  static const u16 table[] = {
    WAZANO_NULL,    WAZANO_ANKOORU,  WAZANO_OUMUGAESI,
    WAZANO_HENSIN,  WAZANO_MONOMANE, WAZANO_SUKETTI,
  };
  u32 i;

  for(i=0; i<NELEMS(table); ++i)
  {
    if( table[i] == waza ){
      return TRUE;
    }
  }
  return FALSE;
}

