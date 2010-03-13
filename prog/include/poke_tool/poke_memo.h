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

//特殊トレーナーメモ用場所定義
#define POKE_MEMO_PLACE_POKE_SHIFTER (30001)  //ポケシフター(過去作転送
#define POKE_MEMO_PLACE_GAME_TRADE   (30002)  //つうしんこうかん(ゲーム内)
#define POKE_MEMO_PLACE_KANTOU       (30004)  //カントー
#define POKE_MEMO_PLACE_JYOUTO       (30005)  //ジョウト
#define POKE_MEMO_PLACE_HOUEN        (30006)  //ホウエン
#define POKE_MEMO_PLACE_SHINOU       (30007)  //シンオウ
#define POKE_MEMO_PLACE_FAR_PLACE    (30008)  //とおいとち(XD・コロシアム
#define POKE_MEMO_PLACE_UNKNOWN      (30009)  //----------(不明

#define POKE_MEMO_PLACE_SEREBIXI_BEFORE  (30010)   //セレビィ転送時
#define POKE_MEMO_PLACE_SEREBIXI_AFTER   (30011)   //セレビィイベント後
#define POKE_MEMO_PLACE_ENRAISUI_BEFORE  (30012)   //エンテイ・ライコウ・スイクン転送時
#define POKE_MEMO_PLACE_ENRAISUI_AFTER   (30013)   //エンテイ・ライコウ・スイクン後

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

}POKE_MEMO_EVENT_TYPE;


//ポケモン取得時操作
//設定しているのは生まれた/捕まえた場所/時間・捕獲レベル・親情報・Romバージョンです。
void POKE_MEMO_SetTrainerMemoPP( POKEMON_PARAM *pp , const POKE_MEMO_SET_TYPE type ,
                                  const MYSTATUS* my, const u32 place , const HEAPID heapId );
void POKE_MEMO_SetTrainerMemoPPP( POKEMON_PASO_PARAM *ppp , const POKE_MEMO_SET_TYPE type ,
                                  const MYSTATUS* my, const u32 place , const HEAPID heapId );

//イベント用ポケモンチェック
const BOOL POKE_MEMO_CheckEventPokePP( POKEMON_PARAM *pp , const POKE_MEMO_EVENT_TYPE type );
const BOOL POKE_MEMO_CheckEventPokePPP( POKEMON_PASO_PARAM *ppp , const POKE_MEMO_EVENT_TYPE type );

//イベント後の処理を行う(2010映画配布用
void POKE_MEMO_SetEventPoke_AfterEventPP( POKEMON_PARAM *pp , const POKE_MEMO_EVENT_TYPE type );
void POKE_MEMO_SetEventPoke_AfterEventPPP( POKEMON_PASO_PARAM *ppp , const POKE_MEMO_EVENT_TYPE type );
