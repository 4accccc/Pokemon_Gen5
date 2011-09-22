//============================================================================
/**
 *  @file   pokefoot_ex.h
 *  @brief  ポケモン足跡グラフィックデータ取得関数
 *  @author Koji Kawada
 *  @data   2010.06.25
 *  @note   
 *
 *  モジュール名：POKEFOOT_EX
 */
//============================================================================
// インクルード
#include <gflib.h>

#include "poke_tool/monsno_def.h"

#include "poke_tool/pokefoot_ex.h"

// アーカイブ
#include "arc_def.h"
#include "pokefoot_ex.naix"


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define  POKEFOOT_EX_ARC_CHAR_GIRATHINA_FORM_01  NARC_pokefoot_ex_poke_foot_487_001_NCGR    // 487 ギラティナ フォルム01
#define  POKEFOOT_EX_ARC_CHAR_SHEIMI_FORM_01     NARC_pokefoot_ex_poke_foot_492_001_NCGR    // 492 シェイミ   フォルム01
#define  POKEFOOT_EX_ARC_CHAR_KYUREMU_FORM_01     NARC_pokefoot_ex_poke_foot_646_001_NCGR    // 646 キュレム   フォルム01
#define  POKEFOOT_EX_ARC_CHAR_KYUREMU_FORM_02     NARC_pokefoot_ex_poke_foot_646_002_NCGR    // 646 キュレム   フォルム02
#define  POKEFOOT_EX_ARC_CHAR_KERUDHIO_FORM_01     NARC_pokefoot_ex_poke_foot_647_001_NCGR    // 647 ケルディオ   フォルム01
#define  POKEFOOT_EX_ARC_CHAR_TORUNEROSU_FORM_01     NARC_pokefoot_ex_poke_foot_641_001_NCGR    // 641 トルネロス   フォルム01
#define  POKEFOOT_EX_ARC_CHAR_BORUTOROSU_FORM_01     NARC_pokefoot_ex_poke_foot_642_001_NCGR    // 642 ボルトロス   フォルム01
#define  POKEFOOT_EX_ARC_CHAR_RANDOROSU_FORM_01     NARC_pokefoot_ex_poke_foot_645_001_NCGR    // 645 ランドロス   フォルム01


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief           アーカイブファイルIDX取得
 *
 *  @param[in]       none
 *
 *  @retval          アーカイブファイルIDX
 */
//------------------------------------------------------------------
int POKEFOOT_EX_ArcFileGet(void)
{
  return  ARCID_POKEFOOT_EX_GRA;
}

//------------------------------------------------------------------
/**
 *  @brief           キャラクタデータIDX取得
 *
 *  @param[in]       monsno      monsno
 *  @param[in]       formno      formno
 *
 *  @retval          キャラクタデータIDX
 */
//------------------------------------------------------------------
int POKEFOOT_EX_CharDataIdxGet(int monsno, int formno)
{
  if( monsno == MONSNO_GIRATHINA && formno == FORMNO_GIRATHINA_ORIGIN )
  {
    return  POKEFOOT_EX_ARC_CHAR_GIRATHINA_FORM_01;
  }
  else if( monsno == MONSNO_SHEIMI && formno == FORMNO_SHEIMI_SKY )
  {
    return  POKEFOOT_EX_ARC_CHAR_SHEIMI_FORM_01;
  }
  else if( monsno == MONSNO_KYUREMU && formno == FORMNO_KYUREMU_WHITE )
  {
    return  POKEFOOT_EX_ARC_CHAR_KYUREMU_FORM_01;
  }
  else if( monsno == MONSNO_KYUREMU && formno == FORMNO_KYUREMU_BLACK )
  {
    return  POKEFOOT_EX_ARC_CHAR_KYUREMU_FORM_02;
  }
  else if( monsno == MONSNO_KERUDHIO && formno == FORMNO_KERUDHIO_SWORD )
  {
    return  POKEFOOT_EX_ARC_CHAR_KERUDHIO_FORM_01;
  }
  else if( monsno == MONSNO_TORUNEROSU && formno == FORMNO_TORUNEROSU_AR )
  {
    return  POKEFOOT_EX_ARC_CHAR_TORUNEROSU_FORM_01;
  }
  else if( monsno == MONSNO_BORUTOROSU && formno == FORMNO_BORUTOROSU_AR )
  {
    return  POKEFOOT_EX_ARC_CHAR_BORUTOROSU_FORM_01;
  }
  else if( monsno == MONSNO_RANDOROSU && formno == FORMNO_RANDOROSU_AR )
  {
    return  POKEFOOT_EX_ARC_CHAR_RANDOROSU_FORM_01;
  }
  else
  {
    return  POKEFOOT_EX_CHAR_DATA_IDX_NULL;
  }
}

