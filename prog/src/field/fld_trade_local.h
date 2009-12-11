///////////////////////////////////////////////////////////////////////////////
/**
 *	@brief	  fld_tradeローカル構造体定義
 *	@author		tomoya takahasi, miyuki iwasawa	(obata)
 *	@data		  2006.05.15 (2009.12.09 HGSSより移植)
 */
///////////////////////////////////////////////////////////////////////////////
#ifndef __H_FIELD_TRADE_LOCAL_H__
#define __H_FIELD_TRADE_LOCAL_H__

//-------------------------------------
//	交換ポケモンデータ
//=====================================
typedef struct 
{
	u32 monsno;			    // モンスターナンバー
  u32 formno;         // フォームナンバー
  u32 level;          // レベル
	u32 hp_rnd;			    // HP乱数
	u32 at_rnd;			    // 攻撃乱数
	u32 df_rnd;			    // 防御乱数
	u32 sp_rnd;			    // 速さ乱数
	u32 sa_rnd;			    // 特攻乱数
	u32 sd_rnd;			    // 特防乱数
	u32 speabino;		    // 特殊能力
  u32 seikaku;        // 性格
  u32 sex;            // 性別
	u32	mons_id;		    // ID
	u32 style;			    // かっこよさ
	u32 beautiful;	    // うつくしさ
	u32 cute;			      // かわいさ
	u32 clever;			    // かしこさ
	u32 strong;			    // たくましさ
	u32 item;			      // アイテム
	u32 oya_sex;		    // 親性別
	u32 fur;			      // 毛艶
	u32 world_code;		  // 親の国コード
	u32 change_monsno;	// 交換するモンスターナンバー
	u32 change_monssex;	// 交換するポケモンの性別（0:オス,1:メス,2:性別問わず)

} FLD_TRADE_POKEDATA;

#include "include/msg/msg_fld_trade.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
#define FLD_TRADE_GET_POKE_GMM(x)	(x)
#define FLD_TRADE_GET_OYA_GMM(x)	(TRADE_OYA_00+x)

#endif //__H_FIELD_TRADE_LOCAL_H__
