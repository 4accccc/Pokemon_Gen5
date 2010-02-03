//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		trgra.h
 *	@brief	トレーナーグラフィック素材の取得
 *	@author	HisashiSogabe
 *	@date		2010.01.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
//ライブラリ
#include <gflib.h>
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//-------------------------------------
///	リソース
//=====================================
#define TRGRA_TRAINER_CHARA_WIDTH		( 10 )	//ポケモン絵のキャラ幅
#define TRGRA_TRAINER_CHARA_HEIGHT	( 10 )	//ポケモン絵のキャラ高さ

//トレーナーのキャラサイズ
#define TRGRA_TRAINER_CHARA_SIZE		( TRGRA_TRAINER_CHARA_WIDTH * TRGRA_TRAINER_CHARA_HEIGHT * GFL_BG_1CHRDATASIZ )

#define TRGRA_TRAINER_PLT_NUM				( 1 )	                            //トレーナー一人分のパレット使用本数
#define TRGRA_TRAINER_PLT_SIZE			( TRGRA_TRAINER_PLT_NUM * 0x20 )	//トレーナー一人分のパレットサイズ


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern ARCID TRGRA_GetArcID( void );
//NCGR
extern ARCDATID TRGRA_GetCgrArcIndex( int tr_type );
//NCBR
extern ARCDATID TRGRA_GetCbrArcIndex( int tr_type );
//NCLR
extern ARCDATID TRGRA_GetPalArcIndex( int tr_type );
//NCER	-> マルチセル用のパーツです。
extern ARCDATID TRGRA_GetCelArcIndex( int tr_type );
//NANR
extern ARCDATID TRGRA_GetAnmArcIndex( int tr_type );
//NMCR
extern ARCDATID TRGRA_GetMCelArcIndex( int tr_type );
//NMAR
extern ARCDATID TRGRA_GetMAnmArcIndex( int tr_type );
//NCEC MCSS用ファイル
extern ARCDATID TRGRA_GetNcecArcIndex( int tr_type );
