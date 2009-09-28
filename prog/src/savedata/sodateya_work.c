/////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  育て屋ワーク
 * @file   sodateya_work.c
 * @author obata
 * @date   2009.09.24
 *
 */
/////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_tool_def.h"
#include "poke_tool/pokeparty.h"
#include "savedata/save_tbl.h"
#include "savedata/save_control.h"
#include "savedata/sodateya_work.h"


//=======================================================================
/**
 * @brief 預けられているポケモン一体分のデータ
 */
//=======================================================================
typedef struct 
{
  POKEMON_PASO_PARAM  pasoParam;  // パーソナルデータ
  u32                 walkCount;  // 歩数カウンタ
} SODATEYA_POKE_DATA;

//=======================================================================
/**
 * @brief 育て屋ワーク
 */
//=======================================================================
struct _SODATEYA_WORK
{
  SODATEYA_POKE_DATA pokemon[ SODATE_POKE_MAX ];  // 預けポケモン
  BOOL                                  haveEgg;  // タマゴの有無
  u8                                eggLayCount;  // 産卵カウンタ
};


//-----------------------------------------------------------------------
/**
 * @brief	育て屋ワークのサイズ取得
 *
 * @return 育て屋ワークのサイズ
 */
//-----------------------------------------------------------------------
u32 SODATEYA_WORK_GetWorkSize(void)
{
	return sizeof(SODATEYA_WORK);
}

//-----------------------------------------------------------------------
/**
 * @brief	育て屋ワークの初期化
 *
 * @param	sodateya 初期化するワーク
 */
//-----------------------------------------------------------------------
void SODATEYA_WORK_InitWork( void* work )
{
  SODATEYA_WORK* sodateya = (SODATEYA_WORK*)work;

  // 初期化
	memset( sodateya, 0, sizeof(SODATEYA_WORK) ); 
	sodateya->haveEgg     = FALSE;
	sodateya->eggLayCount = 0;
}

//-----------------------------------------------------------------------
/**
 * @brief ワークへのポインタを取得する
 *
 * @param scw セーブデータ保持ワークへのポインタ
 *
 * @return 育て屋ワークへのポインタ
 */
//-----------------------------------------------------------------------
SODATEYA_WORK* SODATEYA_WORK_GetSodateyaWork( SAVE_CONTROL_WORK* scw )
{
  return SaveControl_DataPtrGet( scw, GMDATA_ID_SODATEYA_WORK );
}

//-----------------------------------------------------------------------
/**
 * @brief タマゴの有無を取得する
 *
 * @param sodateya 取得対象ワーク
 * 
 * @return タマゴがある場合 TRUE
 */
//-----------------------------------------------------------------------
BOOL SODATEYA_WORK_HaveEgg( SODATEYA_WORK* sodateya )
{
  return sodateya->haveEgg;
}

//-----------------------------------------------------------------------
/**
 * @brief タマゴを削除する
 *
 * @param sodateya 操作対象ワーク
 */
//-----------------------------------------------------------------------
void SODATEYA_WORK_DeleteEgg( SODATEYA_WORK* sodateya )
{
  // タマゴを削除し, 産卵カウンタを初期化
  sodateya->haveEgg     = FALSE;
  sodateya->eggLayCount = 0;
}

//-----------------------------------------------------------------------
/**
 * @brief タマゴを受け取る
 *
 * @param sodateya タマゴを所持する育て屋さん
 * @param party    タマゴを追加する手持ちポケパーティー
 */
//-----------------------------------------------------------------------
void SODATEYA_WORK_AddEggToParty( SODATEYA_WORK* sodateya, POKEPARTY* party )
{
  /*
  POKEMON_PARAM param;

  // TEMP:タマゴのパラメータ計算
  PP_Setup( &param, 1, 1, 0 );
  PP_Put( &param, ID_PARA_tamago_flag, 1 );

  // パーティーに追加
  PokeParty_Add( party, &param );
  SODATEYA_WORK_DeleteEgg( sodateya );
  */

  // TEMP: 先頭ポケモンを追加
  POKEMON_PARAM* param;
  param = PokeParty_GetMemberPointer( party, 0 );
  PokeParty_Add( party, param );
  SODATEYA_WORK_DeleteEgg( sodateya );
}

//-----------------------------------------------------------------------
/**
 * @brief 預けポケモンの数を取得する
 *
 * @param sodateya 取得対象ワーク
 * 
 * @return 育て屋が預かっているポケモンの数
 */
//-----------------------------------------------------------------------
u8 SODATEYA_WORK_GetPokemonNum( SODATEYA_WORK* sodateya )
{
  return 0;
}

//-----------------------------------------------------------------------
/**
 * @brief 預けポケモンのパーソナルパラメータを取得する
 *
 * @param sodateya 取得対象ワーク
 * @param poke_id  パラメータ取得対象ポケモンの番号(0 or 1)
 *
 * @return 指定ポケモンのパーソナルパラメータ
 */
//-----------------------------------------------------------------------
POKEMON_PASO_PARAM* SODATEYA_WORK_GetPokemonPasoParam( 
    SODATEYA_WORK* sodateya, SODATE_POKE_ID poke_id )
{
  // パラメータ取得対象ポケモン番号が範囲外の場合
  if( (poke_id < 0) || (SODATE_POKE_MAX <= poke_id) )
  {
    OBATA_Printf( "--------------------------------------------------\n" );
    OBATA_Printf( "error: パラメータ取得対象ポケモン番号が範囲外です。\n" );
    OBATA_Printf( "--------------------------------------------------\n" );
    return 0;
  }

  // 指定パラメータを取得する
  return &sodateya->pokemon[ poke_id ].pasoParam; }
