//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		tpoke_data.c
 *	@brief  連れ歩きポケモン情報  OBJCODE 大きさ
 *	@author	tomoya takahashi
 *	@date		2010.03.23
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "arc_def.h"
#include "fieldmap/tpoke_data.naix"

#include "field/tpoke_data.h"

#include "fldmmdl.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	連れ歩きポケモン１データ
//=====================================
typedef struct {
  u16 monsno;     // モンスターナンバー
  u16 sex;        // 性別
  u16 formno;     // フォルムナンバー
  u16 objcode;    // オブジェコード
} TPOKE_DAT;


//-------------------------------------
///	連れ歩きポケモン情報
//=====================================
struct _TPOKE_DATA {
  TPOKE_DAT* p_data;
  u32 num;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static int TPokeData_GetIndex( const TPOKE_DATA* cp_wk, u16 monsno, u16 sex, u16 formno );


//----------------------------------------------------------------------------
/**
 *	@brief  データ読み込み
 *
 *	@param	heapID  ヒープID
 *
 *	@return データ
 */
//-----------------------------------------------------------------------------
TPOKE_DATA* TPOKE_DATA_Create( HEAPID heapID )
{
  TPOKE_DATA* p_wk;
  u32 size;

  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(TPOKE_DATA) );

  p_wk->p_data = GFL_ARC_UTIL_LoadEx( ARCID_TPOKE_DATA, NARC_tpoke_data_tpoke_dat, FALSE, heapID, &size );
  GF_ASSERT( (size % sizeof(TPOKE_DAT)) == 0 );
  p_wk->num = size / sizeof(TPOKE_DAT);

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ破棄
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void TPOKE_DATA_Delete( TPOKE_DATA* p_wk )
{
  GFL_HEAP_FreeMemory( p_wk->p_data );
  GFL_HEAP_FreeMemory( p_wk );
}


//----------------------------------------------------------------------------
/**
 *	@brief  オブジェクトコードを取得する
 *
 *	@param	cp_wk   ワーク
 *	@param	monsno  モンスターナンバー
 *	@param	sex     性別
 *	@param	formno  フォームNO
 *
 *	@return オブジェコード
 */
//-----------------------------------------------------------------------------
u16 TPOKE_DATA_GetObjCode( const TPOKE_DATA* cp_wk, u16 monsno, u16 sex, u16 formno )
{
  int index;
  index = TPokeData_GetIndex( cp_wk, monsno, sex, formno );
  return cp_wk->p_data[ index ].objcode;
}

//----------------------------------------------------------------------------
/**
 *	@brief  モデル大きさチェック  大きいサイズか？
 *
 *  @param  cp_gdataゲームデータ
 *	@param	cp_wk   ワーク
 *	@param	monsno  モンスターナンバー
 *	@param	sex     性別
 *	@param	formno  フォームNO
 *
 *	@retval TRUE    大きい
 *	@retval FALSE   通常の大きさ 
 */
//-----------------------------------------------------------------------------
BOOL TPOKE_DATA_IsSizeBig( const GAMEDATA* cp_gdata, const TPOKE_DATA* cp_wk, u16 monsno, u16 sex, u16 formno )
{
  int index;
  const MMDLSYS* cp_mmdlsys = GAMEDATA_GetMMdlSys( (GAMEDATA*)cp_gdata );
  const OBJCODE_PARAM * cp_objcode_para;
  
  
  index = TPokeData_GetIndex( cp_wk, monsno, sex, formno );

  cp_objcode_para = MMDLSYS_GetOBJCodeParam( cp_mmdlsys, cp_wk->p_data[ index ].objcode );
  
  if( cp_objcode_para->mdl_size == MMDL_BLACT_MDLSIZE_64x64 ){
    return TRUE;
  }

#ifdef PM_DEBUG
  // CHECK 16x16
  if( cp_objcode_para->mdl_size == MMDL_BLACT_MDLSIZE_16x16 ){
    GF_ASSERT( cp_objcode_para->mdl_size != MMDL_BLACT_MDLSIZE_16x16 );
    return FALSE;
  }
#endif
  
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  モデル大きさチェック  通常サイズか？
 *
 *	@param	cp_wk   ワーク
 *	@param	monsno  モンスターナンバー
 *	@param	sex     性別
 *	@param	formno  フォームNO
 *
 *	@retval TRUE    通常の大きさ
 *	@retval FALSE   大きい
 */
//-----------------------------------------------------------------------------
BOOL TPOKE_DATA_IsSizeNormal( const GAMEDATA* cp_gdata, const TPOKE_DATA* cp_wk, u16 monsno, u16 sex, u16 formno )
{
  int index;
  const MMDLSYS* cp_mmdlsys = GAMEDATA_GetMMdlSys( (GAMEDATA*)cp_gdata );
  const OBJCODE_PARAM * cp_objcode_para;
  
  
  index = TPokeData_GetIndex( cp_wk, monsno, sex, formno );

  cp_objcode_para = MMDLSYS_GetOBJCodeParam( cp_mmdlsys, cp_wk->p_data[ index ].objcode );
  
  if( cp_objcode_para->mdl_size == MMDL_BLACT_MDLSIZE_32x32 ){
    return TRUE;
  }

#ifdef PM_DEBUG
  // CHECK 16x16
  if( cp_objcode_para->mdl_size == MMDL_BLACT_MDLSIZE_16x16 ){
    GF_ASSERT( cp_objcode_para->mdl_size != MMDL_BLACT_MDLSIZE_16x16 );
    return FALSE;
  }
#endif
  
  return FALSE;
}





//-----------------------------------------------------------------------------
/**
 *      private関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief  データインデックスを取得
 *
 *	@param	cp_wk   ワーク
 *	@param	monsno  モンスターナンバー
 *	@param	sex     性別
 *	@param	formno  フォーム
 *
 *	@return データインデックス
 */
//-----------------------------------------------------------------------------
static int TPokeData_GetIndex( const TPOKE_DATA* cp_wk, u16 monsno, u16 sex, u16 formno )
{
  int i;

  for( i=0; i<cp_wk->num; i++ ){

    // モンスターナンバー
    if( cp_wk->p_data[i].monsno != monsno ){
      continue;
    }

    // 性別
    if( (cp_wk->p_data[i].sex != PM_NEUTRAL) ){ // NEUTRAL以外ならチェックする
      if( cp_wk->p_data[i].sex != sex ){
        continue;
      }
    }

    // フォルム
    if( cp_wk->p_data[i].formno != formno ){ 
      continue;
    }
    
    return i;
  }

  // みつからない・・・
  GF_ASSERT( 0 );
  return 0; // フリーズ回避
}





