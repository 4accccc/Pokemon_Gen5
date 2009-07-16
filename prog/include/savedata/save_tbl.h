//==============================================================================
/**
 * @file	save_tbl.h
 * @brief	WBセーブ
 * @author	matsuda
 * @date	2008.08.27(水)
 */
//==============================================================================
#ifndef __SAVE_H__
#define __SAVE_H__

#include <backup_system.h>


//---------------------------------------------------------------------------
///	セーブに使用しているセクタ数
//---------------------------------------------------------------------------
#define	SAVE_PAGE_MAX		(32)

#define	SAVE_SECTOR_SIZE	(0x1000)

//以下はデバッグツール用のセーブ領域です by soga
#define EDIT_ANM_SAVE_2		( 45 * SAVE_SECTOR_SIZE )	//ポケモンanimeデータ
#define CB_EDIT_SAVE		( 55 * SAVE_SECTOR_SIZE )	//カプセルボールセーブデータ
#define EDIT_ANM_SAVE		( 58 * SAVE_SECTOR_SIZE )	//ポケモンエディットデータ
#define	POKE_ANM_SAVE		(60*SAVE_SECTOR_SIZE)		//ポケモンアニメツール用セーブ領域（未使用のはず）
#define	DEBUG_FIGHT_SAVE	(62*SAVE_SECTOR_SIZE)		//デバッグファイト用セーブ領域


///セーブデータ識別ID
enum{
	GMDATA_ID_CONTEST,
	GMDATA_ID_MATSU_DEBUG,
	GMDATA_ID_MATSU_DEBUG_0,
	GMDATA_ID_MATSU_DEBUG_1,
	GMDATA_ID_MATSU_DEBUG_2,
	GMDATA_ID_MATSU_DEBUG_3,
	GMDATA_ID_MATSU_DEBUG_4,
	GMDATA_ID_MATSU_DEBUG_5,
	GMDATA_ID_MATSU_DEBUG_6,
	GMDATA_ID_MATSU_DEBUG_7,
	GMDATA_ID_MATSU_DEBUG_8,
	GMDATA_ID_MATSU_DEBUG_9,
	GMDATA_ID_MATSU_DEBUG_a,
	GMDATA_ID_MATSU_DEBUG_b,
	GMDATA_ID_MATSU_DEBUG_c,
	GMDATA_ID_MATSU_DEBUG_d,
	GMDATA_ID_MATSU_DEBUG_e,
	GMDATA_ID_BOXDATA,	//ボックスデータグループ
	GMDATA_ID_MYITEM,
	GMDATA_ID_MYPOKE,
	GMDATA_ID_PLAYER_DATA,
	GMDATA_ID_SITUATION,
	GMDATA_ID_WIFIHISTORY,
	GMDATA_ID_WIFILIST,
  GMDATA_ID_CGEAR,    //C-GEAR
	GMDATA_ID_TRCARD,		//トレーナーカード(サイン
	GMDATA_ID_MYSTERYDATA,	//不思議な贈り物
	GMDATA_ID_PERAPVOICE,	//ぺラップヴォイス
	GMDATA_ID_SYSTEM_DATA,	//システムデータ
	GMDATA_ID_RECORD,  		//レコード(スコア
	GMDATA_ID_PMS,  		//簡易文
	GMDATA_ID_MMDL,		//動作モデル
	GMDATA_ID_WFHIROBA,		//Wi-Fi広場
	GMDATA_ID_MUSICAL,		//ミュージカル
  GMDATA_ID_RANDOMMAP,  //ランダム生成マップ
	GMDATA_ID_IRCCOMPATIBLE,	//相性チェック
  GMDATA_ID_SP_RIBBON,	//配布リボン
  
#if 0
	GMDATA_ID_SYSTEM_DATA,
	GMDATA_ID_PLAYER_DATA,
	GMDATA_ID_TEMOTI_POKE,
	GMDATA_ID_TEMOTI_ITEM,
	GMDATA_ID_EVENT_WORK,
	GMDATA_ID_POKETCH_DATA,
	GMDATA_ID_SITUATION,
	GMDATA_ID_ZUKANWORK,
	GMDATA_ID_SODATEYA,
	GMDATA_ID_FRIEND,
	GMDATA_ID_MISC,
	GMDATA_ID_FIELDOBJSV,
	GMDATA_ID_UNDERGROUNDDATA,
    GMDATA_ID_REGULATION,
	GMDATA_ID_IMAGECLIPDATA,
	GMDATA_ID_MAILDATA,
	GMDATA_ID_PORUTODATA,
	GMDATA_ID_RANDOMGROUP,
	GMDATA_ID_FNOTE,
	GMDATA_ID_TRCARD,
	GMDATA_ID_RECORD,
	GMDATA_ID_CUSTOM_BALL,
	GMDATA_ID_PERAPVOICE,
	GMDATA_ID_FRONTIER,
	GMDATA_ID_SP_RIBBON,
	GMDATA_ID_ENCOUNT,
	GMDATA_ID_WORLDTRADEDATA,
	GMDATA_ID_TVWORK,
	GMDATA_ID_GUINNESS,
	GMDATA_ID_WIFILIST,
	GMDATA_ID_WIFIHISTORY,
	GMDATA_ID_FUSHIGIDATA,
	GMDATA_ID_POKEPARKDATA,
	GMDATA_ID_CONTEST,
	GMDATA_ID_PMS,
	GMDATA_ID_EMAIL,
	GMDATA_ID_WFHIROBA,
#endif

	GMDATA_ID_MAX,
};


//==============================================================================
//	外部データ宣言
//==============================================================================
extern const GFL_SVLD_PARAM SaveParam_Normal;


#endif	//__SAVE_H__

