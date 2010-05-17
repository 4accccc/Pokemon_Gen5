//======================================================================
/**
 * @file	poke_memo.h
 * @brief	トレーナーメモ設定・表示関数
 * @author	ariizumi
 * @data	10/03/13
 *
 * モジュール名：POKE_MEMO
 */
//======================================================================
#include "poke_tool/poke_tool.h"

#include "msg/msg_place_name.h"
#include "msg/msg_place_name_out.h"
#include "msg/msg_place_name_per.h"
#include "msg/msg_place_name_spe.h"

//トレーナーメモ ファイル部分
#define POKE_PLACE_START_SPECIAL_PLACE   (30001)
#define POKE_PLACE_START_OUTER_PLACE     (40001)
#define POKE_PLACE_START_PERSON_NAME     (60001)

//特殊トレーナーメモ用場所定義
#define POKE_MEMO_PLACE_POKE_SHIFTER  (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_POKESIHUTAH)  //ポケシフター(過去作転送
#define POKE_MEMO_PLACE_GAME_TRADE    (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_NAIBUTUUSINKOUKAN)  //つうしんこうかん(ゲーム内)
#define POKE_MEMO_PLACE_HUMAN_TRADE   (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_TUUSINKOUKAN)  //つうしんこうかん(人と通信)
#define POKE_MEMO_PLACE_KANTOU        (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_KANTO)  //カントー
#define POKE_MEMO_PLACE_JYOUTO        (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_ZYOTO)  //ジョウト
#define POKE_MEMO_PLACE_HOUEN         (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_HOUEN)  //ホウエン
#define POKE_MEMO_PLACE_SHINOU        (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_SINOH)  //シンオウ
#define POKE_MEMO_PLACE_FAR_PLACE     (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_TOOKUHANARETATOTI)  //とおいとち(XD・コロシアム
#define POKE_MEMO_PLACE_UNKNOWN       (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_HUSEINAKOHDO)  //----------(不明
#define POKE_MEMO_PERSON_SODATEYA     (POKE_PLACE_START_PERSON_NAME+MAPNAME_SODATEYA)  //育て屋夫婦
#define POKE_MEMO_PERSON_TOREZYAHANTA (POKE_PLACE_START_PERSON_NAME+MAPNAME_TOREZYAHANTA)  //トレジャーハンター

#define POKE_MEMO_PLACE_SEREBIXI_BEFORE  (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_2010HAIHU_EV1_01)   //セレビィ転送時
#define POKE_MEMO_PLACE_SEREBIXI_AFTER   (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_2010HAIHU_EV1_02)   //セレビィイベント後
#define POKE_MEMO_PLACE_ENRAISUI_BEFORE  (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_2010HAIHU_EV2_01)   //エンテイ・ライコウ・スイクン転送時
#define POKE_MEMO_PLACE_ENRAISUI_AFTER   (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_2010HAIHU_EV2_02)   //エンテイ・ライコウ・スイクン後
#define POKE_MEMO_PLACE_PALACE           (POKE_PLACE_START_SPECIAL_PLACE+MAPNAME_PARESUKYOUTUU)   //パレス

//不正値用
#define POKE_MEMO_UNKNOWN_PLACE       (MAPNAME_TOOIBASYO)
#define POKE_MEMO_UNKNOWN_PERSON      (POKE_PLACE_START_PERSON_NAME+MAPNAME_TOOKUNIIRUHITO)

//DPPtHGSSでの特殊な地名(映画セレビィ・エンライスイ
#define POKE_MEMO_2010_MOVIE   (3008)

//ポケモン取得時トレーナーメモ数値操作種類
typedef enum
{
  POKE_MEMO_SET_CAPTURE,  //捕獲
  POKE_MEMO_GAME_TRADE,   //ゲーム内交換
  POKE_MEMO_INCUBATION,   //タマゴ孵化
  POKE_MEMO_POKE_SHIFTER, //ポケシフター(過去作転送
  POKE_MEMO_DISTRIBUTION, //配布 ※配布の親名・親IDは外でセットしてください。
                          //     ※タマゴは↓を使ってください。

  POKE_MEMO_EGG_FIRST,    //初めて手入れたタマゴ(育てや・ゲーム内イベント・外部配布
  POKE_MEMO_EGG_TRADE,    //もらったタマゴ(通信交換・ぐるぐる交換・ゲーム内交換
                          //placeにはPOKE_MEMO_PLACE_GAME_TRADEかPOKE_MEMO_PLACE_HUMAN_TRADEをセットしてください。
}POKE_MEMO_SET_TYPE;

//イベントポケチェック用種類
typedef enum
{
  POKE_MEMO_EVENT_2010MOVIE_SEREBIXI_BEF,   //2010映画セレビィ(イベント前
  POKE_MEMO_EVENT_2010MOVIE_SEREBIXI_AFT,   //2010映画セレビィ(イベント後
  POKE_MEMO_EVENT_2010MOVIE_ENRAISUI_BEF,   //2010映画エンテイ・ライコウ・スイクン(イベント前
  POKE_MEMO_EVENT_2010MOVIE_ENRAISUI_AFT,   //2010映画エンテイ・ライコウ・スイクン(イベント後
  POKE_MEMO_EVENT_DARUTANISU,           //配布ダルタニス
  POKE_MEMO_EVENT_MERODHIA,             //配布メロディア
  POKE_MEMO_EVENT_INSEKUTA,             //配布インセクタ
  POKE_MEMO_EVENT_SHEIMI,               //配布シェイミ

}POKE_MEMO_EVENT_TYPE;


//ポケモン取得時操作
//設定しているのは生まれた/捕まえた場所/時間・捕獲レベル・親情報・Romバージョンです。
extern void POKE_MEMO_SetTrainerMemoPP( POKEMON_PARAM *pp , const POKE_MEMO_SET_TYPE type ,
                                  const MYSTATUS* my, const u32 place , const HEAPID heapId );
extern void POKE_MEMO_SetTrainerMemoPPP( POKEMON_PASO_PARAM *ppp , const POKE_MEMO_SET_TYPE type ,
                                  const MYSTATUS* my, const u32 place , const HEAPID heapId );
//余計な関数呼ばないために独立(到着後に親機の時間で再設定
extern void POKE_MEMO_SetTrainerMemoPokeShifter( POKEMON_PASO_PARAM *ppp );
extern void POKE_MEMO_SetTrainerMemoPokeShifterAfterTrans( POKEMON_PASO_PARAM *ppp );
//日付設定のため独立
extern void POKE_MEMO_SetTrainerMemoPokeDistribution( POKEMON_PASO_PARAM *ppp , const u32 place , const u32 year , const u32 month , const u32 day );

//イベント用ポケモンチェック
extern const BOOL POKE_MEMO_CheckEventPokePP( POKEMON_PARAM *pp , const POKE_MEMO_EVENT_TYPE type );
extern const BOOL POKE_MEMO_CheckEventPokePPP( POKEMON_PASO_PARAM *ppp , const POKE_MEMO_EVENT_TYPE type );

//イベント後の処理を行う(2010映画配布用
extern void POKE_MEMO_SetEventPoke_AfterEventPP( POKEMON_PARAM *pp , const POKE_MEMO_EVENT_TYPE type );
extern void POKE_MEMO_SetEventPoke_AfterEventPPP( POKEMON_PASO_PARAM *ppp , const POKE_MEMO_EVENT_TYPE type );

//地名→メッセージのdatIdとmessageId
const ARCDATID POKE_PLACE_GetMessageDatId( const u32 place );
const u32 POKE_PLACE_GetMessageId( const u32 place );
