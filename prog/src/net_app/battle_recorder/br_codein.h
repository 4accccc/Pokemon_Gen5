//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_codein.h
 *	@brief  コード入力
 *	@author	Toru=Nagihashi
 *	@date		2009.11.23
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "br_res.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define BR_CODE_BLOCK_MAX		( 3 )		///< 入力ブロック最大数

//-------------------------------------
///	
//=====================================
typedef enum
{
  BR_CODEIN_SELECT_NONE,
  BR_CODEIN_SELECT_CANCEL,
  BR_CODEIN_SELECT_DECIDE,
} BR_CODEIN_SELECT;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	コードイン引数
//=====================================
typedef struct 
{
	
	int		  	word_len;					    ///< 入力文字数	
	int		  	block[ BR_CODE_BLOCK_MAX + 1 ];///< 入力ブロック　xx-xxxx-xxx とか
	
	int			  end_state;					///< 終了時の状態
	STRBUF*		strbuf;						///< 空欄ならバッファそうでなければデフォルト値

  GFL_CLUNIT  *p_unit;
  BR_RES_WORK *p_res;
} BR_CODEIN_PARAM;

//-------------------------------------
///	コードインワーク不完全型
//=====================================
typedef struct _BR_CODEIN_WORK BR_CODEIN_WORK;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//システム
extern BR_CODEIN_WORK * BR_CODEIN_Init( const BR_CODEIN_PARAM *cp_param, HEAPID heapID );
extern void BR_CODEIN_Exit( BR_CODEIN_WORK *wk );
extern void BR_CODEIN_Main( BR_CODEIN_WORK *wk );
extern BR_CODEIN_SELECT BR_CODEIN_GetSelect( const BR_CODEIN_WORK *wk );

//引数作成
extern BR_CODEIN_PARAM*	CodeInput_ParamCreate( int heap_id, int word_len, GFL_CLUNIT *p_unit, BR_RES_WORK *p_res, int block[] );
extern void	 CodeInput_ParamDelete( BR_CODEIN_PARAM* codein_param );


//--------------------------------------------------------------
/**
 * @brief	xxxx-xxxx-xxxx のブロック定義を作る
 *
 * @param	block[]	
 *
 * @retval	static inline	
 *
 */
//--------------------------------------------------------------
static inline void Br_CodeIn_BlockDataMake_4_4_4( int block[] )
{
#if 1
	block[ 0 ] = 4;
	block[ 1 ] = 4;
	block[ 2 ] = 4;
#else
	block[ 0 ] = 2;
	block[ 1 ] = 5;
	block[ 2 ] = 5;
#endif
}


//--------------------------------------------------------------
/**
 * @brief	xx-xxxxx-xxxxx のブロック定義を作る
 *
 * @param	block[]	
 *
 * @retval	static inline	
 *
 */
//--------------------------------------------------------------
static inline void Br_CodeIn_BlockDataMake_2_5_5( int block[] )
{
	block[ 0 ] = 2;
	block[ 1 ] = 5;
	block[ 2 ] = 5;
}
