//=============================================================================================
/**
 * @file  btl_wazarec.c
 * @brief 出たワザ記録機構
 * @author  taya
 *
 * @date  2009.11.21  作成
 */
//=============================================================================================

#include "waza_tool\wazano_def.h"
#include "btl_wazarec.h"



//=============================================================================================
/**
 * 構造体初期化
 *
 * @param   rec
 */
//=============================================================================================
void BTL_WAZAREC_Init( BTL_WAZAREC* rec )
{
  u32 i;

  GFL_STD_MemClear( rec, sizeof(BTL_WAZAREC) );

  rec->ptr = 0;
  for(i=0; i<NELEMS(rec->record); ++i){
    rec->record[i].wazaID = WAZANO_NULL;
  }
}
//=============================================================================================
/**
 * 出たワザ１件記録
 *
 * @param   rec
 * @param   waza
 * @param   turn
 * @param   pokeID
 */
//=============================================================================================
void BTL_WAZAREC_Add( BTL_WAZAREC* rec, WazaID waza, u32 turn, u8 pokeID )
{
  rec->record[ rec->ptr ].wazaID = waza;
  rec->record[ rec->ptr ].turn = turn;
  rec->record[ rec->ptr ].pokeID = pokeID;
  rec->record[ rec->ptr ].fEffective = FALSE;
  rec->ptr++;
  if( rec->ptr >= NELEMS(rec->record) ){
    rec->ptr = 0;
  }
}
//=============================================================================================
/**
 * 最後に記録したワザが効果があったことを記録
 *
 * @param   rec
 */
//=============================================================================================
void BTL_WAZAREC_SetEffectiveLast( BTL_WAZAREC* rec )
{
  int ptr = rec->ptr - 1;
  if( ptr < 0 ){
    ptr = NELEMS(rec->record) - 1;
  }
  rec->record[ ptr ].fEffective = TRUE;
}




//=============================================================================================
/**
 * 指定ターンにワザが使われていたかチェック
 *
 * @param   rec
 * @param   waza
 * @param   turn
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTL_WAZAREC_IsUsedWaza( const BTL_WAZAREC* rec, WazaID waza, u32 turn )
{
  int p = rec->ptr;

  while( 1 )
  {
    if( --p < 0 ){
      p = NELEMS(rec->record) - 1;
    }
    if( (p == rec->ptr) || (rec->record[p].wazaID == WAZANO_NULL) ){
      break;
    }
    if( (rec->record[p].wazaID == waza) && (rec->record[p].turn == turn) ){
      return TRUE;
    }
  }
  return FALSE;
}

