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

#include "savedata/intrude_save_field.h"
#include "savedata/intrude_save.h"

#include "arc_def.h"  //ARCID_MESSAGE
#include "message.naix" //NARC_message_debugname_dat
#include "msg/msg_debugname.h"  //DEBUG_NAME_RAND_M_000
#include "system/gfl_use.h"   //GFUser_GetPublicRand

#include "app/research_radar/question_id.h"
#include "savedata/questionnaire_save.h"

#ifdef PM_DEBUG
//============================================================================================
// デバック用　手持ちポケモン　追加処理
//============================================================================================
//=====================================================================
// デバック用　手持ちポケモン　追加処理 用途別関数
//=====================================================================
static void DEBUG_MyPokeAdd_Field( POKEPARTY* party, POKEMON_PARAM* pp, ZUKAN_SAVEDATA * zukan, MYSTATUS* mystatus );
// ---その他自由に定義してください。
static void DEBUG_MyPokeAdd_Tamada( POKEPARTY* party, POKEMON_PARAM* pp, ZUKAN_SAVEDATA * zukan, MYSTATUS* mystatus );
static void DEBUG_MyPokeAdd_Matsuda( POKEPARTY* party, POKEMON_PARAM* pp, ZUKAN_SAVEDATA * zukan, MYSTATUS* mystatus );


//============================================================================================
// デバック用　手持ちポケモン　振り分け　
//===========================================================================================
//--------------------------------------------------------------
/**
 * @brief   デバッグ用に適当に手持ちポケモンをAdd
 * @param   gamedata  GAMEDATAへのポインタ		
 */
//--------------------------------------------------------------
static void DEBUG_MyPokeAdd(GAMEDATA * gamedata, HEAPID heapID)
{
  MYSTATUS *myStatus;
	POKEPARTY *party;
	POKEMON_PARAM *pp;
	
	party = GAMEDATA_GetMyPokemon(gamedata);
  myStatus = GAMEDATA_GetMyStatus(gamedata);
  
	pp = PP_Create(MONSNO_MOGURYUU, 100, MyStatus_GetID(myStatus), heapID);

  /*
   *  スタッフそれぞれの、作業に一番あったポケモン設定に
   *  振り分けてください。
   *
   */
#if defined(DEBUG_ONLY_for_tamada)
  DEBUG_MyPokeAdd_Tamada( party, pp, GAMEDATA_GetZukanSave(gamedata), myStatus );
#elif defined(DEBUG_ONLY_FOR_matsuda)
  DEBUG_MyPokeAdd_Matsuda( party, pp, GAMEDATA_GetZukanSave(gamedata), myStatus );
#else

  // デフォルトの設定。
  // 手持ちが全部ばらばらで、フィールド技を持っている状態
  DEBUG_MyPokeAdd_Field( party, pp, GAMEDATA_GetZukanSave(gamedata), myStatus );
#endif

	GFL_HEAP_FreeMemory(pp);
}


//--------------------------------------------------------------
/**
 * @brief   フィールド用（デフォ）
 */
//--------------------------------------------------------------
static void DEBUG_MyPokeAdd_Field( POKEPARTY* party, POKEMON_PARAM* pp, ZUKAN_SAVEDATA * zukan, MYSTATUS* mystatus )
{
	PP_Setup(pp, MONSNO_MOGURYUU, 100, MyStatus_GetID( mystatus ));
  PP_Put( pp , ID_PARA_id_no, (u32)MyStatus_GetID( mystatus ) );
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)MyStatus_GetMyName( mystatus ) );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( mystatus ) );
	PokeParty_Add(party, pp);
	ZUKANSAVE_SetPokeGet( zukan, pp );

	PP_Setup(pp, MONSNO_MUSYAANA, 100, MyStatus_GetID( mystatus ));
  PP_Put( pp , ID_PARA_id_no, (u32)MyStatus_GetID( mystatus ) );
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)MyStatus_GetMyName( mystatus ) );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( mystatus ) );
	PokeParty_Add(party, pp);
	ZUKANSAVE_SetPokeGet( zukan, pp );

	PP_Setup(pp, MONSNO_TIRAAMHI, 100, MyStatus_GetID( mystatus ));
  PP_Put( pp , ID_PARA_id_no, (u32)MyStatus_GetID( mystatus ) );
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)MyStatus_GetMyName( mystatus ) );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( mystatus ) );
  PP_SetWazaPos( pp , WAZANO_TEREPOOTO , 0 );
  PP_SetWazaPos( pp , WAZANO_HURASSYU, 1 );
	PokeParty_Add(party, pp);
	ZUKANSAVE_SetPokeGet( zukan, pp );

  PP_Setup(pp, MONSNO_GOBITTO, 100, MyStatus_GetID( mystatus ));
  PP_Put( pp , ID_PARA_id_no, (u32)MyStatus_GetID( mystatus ) );
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)MyStatus_GetMyName( mystatus ) );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( mystatus ) );
  PP_SetWazaPos( pp , WAZANO_NAMINORI , 0 );
  PP_SetWazaPos( pp , WAZANO_TAKINOBORI , 1 );
  PP_SetWazaPos( pp , WAZANO_KAIRIKI , 2 );
  PP_SetWazaPos( pp , WAZANO_IAIGIRI , 3 );
	PokeParty_Add(party, pp);
	ZUKANSAVE_SetPokeGet( zukan, pp );
}

// ---その他自由に定義してください。
static void DEBUG_MyPokeAdd_Tamada( POKEPARTY* party, POKEMON_PARAM* pp, ZUKAN_SAVEDATA * zukan, MYSTATUS* mystatus )
{
	PP_Setup(pp, MONSNO_MUSYAANA, 2, MyStatus_GetID( mystatus ));
  PP_Put( pp , ID_PARA_id_no, (u32)MyStatus_GetID( mystatus ) );
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)MyStatus_GetMyName( mystatus ) );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( mystatus ) );
	PokeParty_Add(party, pp);
	ZUKANSAVE_SetPokeGet( zukan, pp );
}

static void DEBUG_MyPokeAdd_Matsuda( POKEPARTY* party, POKEMON_PARAM* pp, ZUKAN_SAVEDATA * zukan, MYSTATUS* mystatus )
{
	PP_Setup(pp, MONSNO_MOGURYUU, 100, MyStatus_GetID( mystatus ));
  PP_Put( pp , ID_PARA_id_no, (u32)MyStatus_GetID( mystatus ) );
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)MyStatus_GetMyName( mystatus ) );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( mystatus ) );
	PokeParty_Add(party, pp);
	ZUKANSAVE_SetPokeGet( zukan, pp );

	PP_Setup(pp, MONSNO_MUSYAANA, 100, 123456);
  PP_Put( pp , ID_PARA_id_no, (u32)MyStatus_GetID( mystatus ) );
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)MyStatus_GetMyName( mystatus ) );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( mystatus ) );
	PokeParty_Add(party, pp);
	ZUKANSAVE_SetPokeGet( zukan, pp );

	PP_Setup(pp, MONSNO_TIRAAMHI, 100, 123456);
  PP_Put( pp , ID_PARA_id_no, (u32)MyStatus_GetID( mystatus ) );
  PP_Put( pp , ID_PARA_oyaname_raw , (u32)MyStatus_GetMyName( mystatus ) );
  PP_Put( pp , ID_PARA_oyasex , MyStatus_GetMySex( mystatus ) );
  PP_SetWazaPos( pp , WAZANO_TEREPOOTO , 0 );
  PP_SetWazaPos( pp , WAZANO_HURASSYU, 1 );
	PokeParty_Add(party, pp);
	ZUKANSAVE_SetPokeGet( zukan, pp );
}





//============================================================================================
//============================================================================================
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
	{ ITEM_MOKOSINOMI,	50},	//モコシのみ
	{ ITEM_GOSUNOMI,	50},	//ゴスのみ
	{ ITEM_RABUTANOMI,	50},	//ラブタのみ
  { ITEM_PARESUDAMA, 99}, // パレスだま
	
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
static void DEBUG_MYITEM_MakeBag(GAMEDATA * gamedata, HEAPID heapID)
{
	u32	i;
  MYITEM_PTR myitem = GAMEDATA_GetMyItem( gamedata );

	MYITEM_Init( myitem );
	for( i=0; i<NELEMS(DebugItem); i++ ){
		MYITEM_AddItem( myitem, DebugItem[i].id, DebugItem[i].no, heapID );
	}
}

//============================================================================================
//============================================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
static void addIntrudeSecretItem(
    GFL_MSGDATA * msgman, INTRUDE_SAVE_WORK * intsave, u16 item_no, u16 tbl_no)
{
  INTRUDE_SECRET_ITEM_SAVE intrude_item = {
    { 0 },
    ITEM_OMAMORIKOBAN,
    0,
    0
  };
  STRBUF * namebuf;

  namebuf = GFL_MSG_CreateString( msgman, DEBUG_NAME_RAND_M_000 + GFUser_GetPublicRand(8) );
  GFL_STR_GetStringCode( namebuf, intrude_item.name, PERSON_NAME_SIZE + EOM_SIZE );
  intrude_item.item = item_no;
  intrude_item.tbl_no = tbl_no;
  ISC_SAVE_SetItem( intsave, &intrude_item );

  GFL_STR_DeleteBuffer( namebuf );
}
//--------------------------------------------------------------
/**
 * @brief   デバッグ用に隠されアイテムを加える
 * @param   gamedata  GAMEDATAへのポインタ		
 * @param   heapID    利用するヒープIDの指定
 */
//--------------------------------------------------------------
static void DEBUG_INTRUDE_MakeSecretItem( GAMEDATA * gamedata, HEAPID heapID )
{
  GFL_MSGDATA * msgman;
  INTRUDE_SAVE_WORK * intsave;
    
  msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_debugname_dat, heapID );

  intsave = SaveData_GetIntrude( GAMEDATA_GetSaveControlWork( gamedata ) );
  addIntrudeSecretItem( msgman, intsave, ITEM_OMAMORIKOBAN, 0 );
  addIntrudeSecretItem( msgman, intsave, ITEM_WAZAMASIN05 ,1 );
  addIntrudeSecretItem( msgman, intsave, ITEM_KODAWARIHATIMAKI ,0 );

  GFL_MSG_Delete( msgman );
}

//--------------------------------------------------------------
/**
 * デバッグ用に設定する侵入パラメータ
 *
 * @param   gamedata		
 * @param   heapID		
 */
//--------------------------------------------------------------
static void DEBUG_IntrudeParam( GAMEDATA *gamedata, HEAPID heapID )
{
  INTRUDE_SAVE_WORK * intsave;
  OCCUPY_INFO *occupy;
  
  occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
  occupy->white_level = 25;
  occupy->black_level = 25;
  
  intsave = SaveData_GetIntrude( GAMEDATA_GetSaveControlWork( gamedata ) );
  ISC_SAVE_SetGPowerID( intsave, GFUser_GetPublicRand( GPOWER_ID_MAX - 1) + 1 );
}

//--------------------------------------------------------------
/**
 * デバッグ用に設定するアンケートデータ
 *
 * @param   gamedata		
 * @param   heapID		
 */
//--------------------------------------------------------------
static void DEBUG_QuestionnaireParam( GAMEDATA *gamedata )
{
  QUESTIONNAIRE_SAVE_WORK *qsw;
  QUESTIONNAIRE_ANSWER_WORK *answork;
  int question_id;
  
  qsw = SaveData_GetQuestionnaire( GAMEDATA_GetSaveControlWork( gamedata ) );
  answork = Questionnaire_GetAnswerWork(qsw);
  
  for(question_id = 0; question_id < QUESTION_ID_NUM; question_id++){
    QuestionnaireAnswer_WriteBit(answork, question_id, GFUser_GetPublicRand(2) + 1);
  }
}

//--------------------------------------------------------------
//--------------------------------------------------------------
void DEBUG_SetStartData( GAMEDATA * gamedata, HEAPID heapID )
{
  DEBUG_INTRUDE_MakeSecretItem( gamedata, heapID );
  DEBUG_MYITEM_MakeBag( gamedata, heapID );
  DEBUG_MyPokeAdd( gamedata, heapID );
  DEBUG_IntrudeParam( gamedata, heapID );
  DEBUG_QuestionnaireParam( gamedata );
}



#endif  // PM_DEBUG
