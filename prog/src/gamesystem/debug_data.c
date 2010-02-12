//============================================================================================
/**
 * @file	debug_data.c
 * @brief	ゲーム開始時のデバッグデータ追加処理
 * @author	tamada GAME FREAK Inc.
 * @date	09.10.24
 *
 */
//============================================================================================

#include <gflib.h>

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"

#include "debug_data.h"


#include "poke_tool/monsno_def.h"
#include "item/item.h"  //デバッグアイテム生成用
#include "waza_tool/wazano_def.h"  //デバッグポケ生成用
#include "savedata/zukan_savedata.h"		// 図鑑捕獲フラグ設定

#ifdef PM_DEBUG
//============================================================================================
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief   デバッグ用に適当に手持ちポケモンをAdd
 * @param   gamedata  GAMEDATAへのポインタ		
 */
//--------------------------------------------------------------
void DEBUG_MyPokeAdd(GAMEDATA * gamedata, HEAPID heapID)
{
  MYSTATUS *myStatus;
	POKEPARTY *party;
	POKEMON_PARAM *pp;
	const STRCODE *name;
	
	party = GAMEDATA_GetMyPokemon(gamedata);
  myStatus = GAMEDATA_GetMyStatus(gamedata);
  name = MyStatus_GetMyName( myStatus );
  
	pp = PP_Create(MONSNO_MOGURYUU, 100, 123456, heapID);

#ifdef DEBUG_ONLY_for_tamada
	PP_Setup(pp, MONSNO_MUSYAANA, 2, 123456);
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );
	PokeParty_Add(party, pp);
	ZUKANSAVE_SetPokeGet( GAMEDATA_GetZukanSave(gamedata), pp );
#else
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );
	PokeParty_Add(party, pp);
	ZUKANSAVE_SetPokeGet( GAMEDATA_GetZukanSave(gamedata), pp );

	PP_Setup(pp, MONSNO_MUSYAANA, 100, 123456);
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );
	PokeParty_Add(party, pp);
	ZUKANSAVE_SetPokeGet( GAMEDATA_GetZukanSave(gamedata), pp );

	PP_Setup(pp, MONSNO_TIRAAMHI, 100, 123456);
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );
	PokeParty_Add(party, pp);
	ZUKANSAVE_SetPokeGet( GAMEDATA_GetZukanSave(gamedata), pp );

  PP_Setup(pp, MONSNO_TIRAAMHI, 100, 123456);
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)name );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( myStatus ) );
  PP_SetWazaPos( pp , WAZANO_NAMINORI , 0 );
  PP_SetWazaPos( pp , WAZANO_TAKINOBORI , 1 );
  PP_SetWazaPos( pp , WAZANO_KAIRIKI , 2 );
  PP_SetWazaPos( pp , WAZANO_IAIGIRI , 3 );
	PokeParty_Add(party, pp);
	ZUKANSAVE_SetPokeGet( GAMEDATA_GetZukanSave(gamedata), pp );
#endif

	GFL_HEAP_FreeMemory(pp);
}

//------------------------------------------------------------------
/**
 * @brief	デバッグ用：適当に手持ちを生成する
 * @param	myitem	手持ちアイテム構造体へのポインタ
 */
//------------------------------------------------------------------
static const ITEM_ST DebugItem[] = {
	{ ITEM_MASUTAABOORU,	111 },
	{ ITEM_MONSUTAABOORU,	222 },
	{ ITEM_SUUPAABOORU,		333 },
	{ ITEM_HAIPAABOORU,		444 },
	{ ITEM_PUREMIABOORU,	555 },
	{ ITEM_DAIBUBOORU,		666 },
	{ ITEM_TAIMAABOORU,		777 },
	{ ITEM_RIPIITOBOORU,	888 },
	{ ITEM_NESUTOBOORU,		999 },
	{ ITEM_GOOZYASUBOORU,	100 },
	{ ITEM_KIZUGUSURI,		123 },
	{ ITEM_NEMUKEZAMASI,	456 },
	{ ITEM_BATORUREKOODAA,  1},  // バトルレコーダー
	{ ITEM_TAUNMAPPU,		1 },
	{ ITEM_ZITENSYA,		1 },
	{ ITEM_NANDEMONAOSI,	18 },
	{ ITEM_PIIPIIRIKABAA,	18 },
	{ ITEM_PIIPIIMAKKUSU,	18 },
	{ ITEM_ANANUKENOHIMO, 50 },
	{ ITEM_GOORUDOSUPUREE, 50 },
	{ ITEM_DOKUKESI,		18 },		// どくけし
	{ ITEM_YAKEDONAOSI,		19 },		// やけどなおし
	{ ITEM_KOORINAOSI,		20 },		// こおりなおし
	{ ITEM_MAHINAOSI,		22 },		// まひなおし
	{ ITEM_EFEKUTOGAADO,	54 },		// エフェクトガード
	{ ITEM_KURITHIKATTAA,	55 },		// クリティカッター
	{ ITEM_PURASUPAWAA,		56 },		// プラスパワー
	{ ITEM_DHIFENDAA,		57 },		// ディフェンダー
	{ ITEM_SUPIIDAA,		58 },		// スピーダー
	{ ITEM_YOKUATAARU,		59 },		// ヨクアタール
	{ ITEM_SUPESYARUAPPU,	60 },		// スペシャルアップ
	{ ITEM_SUPESYARUGAADO,	61 },		// スペシャルガード
	{ ITEM_PIPPININGYOU,	62 },		// ピッピにんぎょう
	{ ITEM_ENEKONOSIPPO,	63 },		// エネコのシッポ
	{ ITEM_GENKINOKAKERA,	28 },		// げんきのかけら
	{ ITEM_KAIHUKUNOKUSURI,	28 },		// げんきのかけら
	{ ITEM_PIIPIIEIDO,	28 },
	{ ITEM_PIIPIIEIDAA,	28 },
	{ ITEM_DAAKUBOORU,	13 },		// ダークボール
	{ ITEM_HIIRUBOORU,  14 },		// ヒールボール
	{ ITEM_KUIKKUBOORU,	15 },		// クイックボール
	{ ITEM_PURESYASUBOORU,	16 },	// プレシアボール
	{ ITEM_TOMODATITETYOU,  1},  // ともだち手帳
  {ITEM_PARESUHEGOO, 1}, // パレスへゴー
	{ ITEM_MOKOSINOMI,	50},	//モコシのみ
	{ ITEM_GOSUNOMI,	50},	//ゴスのみ
	{ ITEM_RABUTANOMI,	50},	//ラブタのみ
	
  { ITEM_GAKUSYUUSOUTI,  5},  // がくしゅうそうち
  {ITEM_SENSEINOTUME, 5}, // せんせいのツメ
  { ITEM_KODAWARIHATIMAKI,  5},  // こだわりハチマキ
  {ITEM_OMAMORIKOBAN, 5}, // おまもりこばん

  { ITEM_WAZAMASIN01,  328},		// わざマシン０１
  {  ITEM_WAZAMASIN02	, 329 },		// わざマシン０２
  {  ITEM_WAZAMASIN03	, 330 },		// わざマシン０３
  {  ITEM_WAZAMASIN04	, 331 },		// わざマシン０４
  {  ITEM_WAZAMASIN05	, 332 },		// わざマシン０５
  
};

//--------------------------------------------------------------
/**
 * @brief   デバッグ用に手持ちどうぐを加える
 * @param   gamedata  GAMEDATAへのポインタ		
 * @param   heapID    利用するヒープIDの指定
 */
//--------------------------------------------------------------
void DEBUG_MYITEM_MakeBag(GAMEDATA * gamedata, int heapID)
{
	u32	i;
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gamedata );

	MYITEM_Init( myitem );
	for( i=0; i<NELEMS(DebugItem); i++ ){
		MYITEM_AddItem( myitem, DebugItem[i].id, DebugItem[i].no, heapID );
	}
}

#endif  // PM_DEBUG
