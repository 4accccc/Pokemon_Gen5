//============================================================================
/**
 *
 *	@file		myitem_savedata_def.h
 *	@brief  手持ちアイテム スクリプトからも参照する定義
 *	@author		hosaka genya
 *	@data		2009.10.09
 *	@note   myitem_savededa.h から切り出し
 *	@note   アセンブラで読み込むためenum禁止
 *
 */
//============================================================================
#pragma once

#include "item\itempocket_def.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================
//--------------------------------------------------------------
//	この定義はitemconv_dp の datamake.cppと同じでなければいけない 2009.07.21
//	コンバータから吐き出された定数を受け渡すようにした 2009.10.10 by genya hosaka
//--------------------------------------------------------------
// ポケットID
#define	BAG_POKE_NORMAL		( ITEMPOCKET_NORMAL )		// 道具
#define	BAG_POKE_DRUG		( ITEMPOCKET_DRUG )		// 薬
#define	BAG_POKE_WAZA		( ITEMPOCKET_WAZA )		// 技マシン
#define	BAG_POKE_NUTS		( ITEMPOCKET_NUTS )		// 木の実
#define	BAG_POKE_EVENT		( ITEMPOCKET_EVENT )		// 大切な物

#define	BAG_POKE_MAX		( 5 )		// ポケット最大数

#define	BATTLE_BAG_POKE_MAX		( 4 )		// バトルバッグのポケット最大数

