//============================================================================================
/**
 * @file    pokewood_savedata.h
 * @brief   ポケウッドセーブデータ
 * @author  Akito Mori
 * @date    11.07.26
 */
//============================================================================================
#pragma once

#include "include/buflen.h"
#include "gamesystem/game_data.h"
#include "battle/pokewood.h"

#define POKEWOOD_BOXOFFICE_DATA_MAX   ( 10 )    // 記録として残す興行成績ランキングの数

/// ポケウッド興行成績データ構造体定義
typedef struct {
  int     scenario_no;    // シナリオNO   4
  int     profits;        // 収益         8
  STRCODE nickname[BUFLEN_POKEMON_NAME];  // ポケモン名 8+12*2=32
}POKEWOOD_BOXOFFICE_RECORD;


///<外部参照型定義
typedef struct _POKEWOOD_SAVEDATA POKEWOOD_SAVEDATA;


//---------------------------------------------------------------------------
/**
 * @brief	ワークのサイズ取得
 * @return	int		サイズ
 */
//---------------------------------------------------------------------------
extern int POKEWOOD_SAVEDATA_GetWorkSize(void);

//---------------------------------------------------------------------------
/**
 * @brief	ワークの初期化
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void POKEWOOD_SAVEDATA_Init( POKEWOOD_SAVEDATA * wk );

//---------------------------------------------------------------------------
/**
 * @brief	セーブポインタを返す
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern POKEWOOD_SAVEDATA* GAMEDATA_GetPokewoodSavedata( GAMEDATA *gamedata );



//=============================================================================================
//ポケウッドシナリオ解放・新着フラグ
//=============================================================================================
//=============================================================================================
//=============================================================================================
/**
 * @brief	ポケウッドシナリオ解放フラグ取得
 *
 * @param   sv  POKEWOOD_SAVEDATA
 * @param   no  シナリオNO
 *
 * @retval  BOOL		TRUE:解放すみ FALSE:未解放
 */
//=============================================================================================
extern BOOL POKEWOOD_SAVEDATA_GetPokewoodOpenFlag( const POKEWOOD_SAVEDATA *sv, int no );
//=============================================================================================
/**
 * @brief ポケウッドシナリオ解放フラグをセット
 *
 * @param   sv  POKEWOOD_SAVEDATA
 * @param   no  シナリオNO
 */
//=============================================================================================
extern void POKEWOOD_SAVEDATA_SetPokewoodOpenFlag( POKEWOOD_SAVEDATA *sv, int no );
//=============================================================================================
/**
 * @brief	ポケウッド上映済みかフラグを取得
 *
 * @param   sv  POKEWOOD_SAVEDATA
 * @param   no  シナリオNO
 *
 * @retval  BOOL		TRUE:上映済み FALSE:未上映
 */
//=============================================================================================
extern BOOL POKEWOOD_SAVEDATA_GetPokewoodPlayedFlag( const POKEWOOD_SAVEDATA *sv, int no );

//=============================================================================================
/**
 * @brief	ポケウッド上映済みかフラグをセット
 *
 * @param   sv  POKEWOOD_SAVEDATA
 * @param   no	シナリオNO
 */
//=============================================================================================
extern void POKEWOOD_SAVEDATA_SetPokewoodPlayedFlag( POKEWOOD_SAVEDATA *sv, int no );


//=============================================================================================
/**
 * @brief	興行成績を登録（TOP10に入っていれば記録に残ります）
 *
 * @param   sv	  POKEWOOD_SAVEDATR
 * @param   data  POKEWOOD_BOXOFFICE_RECORD
 */
//=============================================================================================
extern void POKEWOOD_SAVEDATA_SetBoxOfficeData( POKEWOOD_SAVEDATA *sv, POKEWOOD_BOXOFFICE_RECORD *data );

//=============================================================================================
/**
 * @brief	興行成績上位者のデータを取得する
 *
 * @param   sv		POKEWOOD_SAVEDATA
 * @param   no		no
 *
 * @retval  const POKEWOOD_BOXOFFICE_RECORD *		
 */
//=============================================================================================
extern const POKEWOOD_BOXOFFICE_RECORD *POKEWOOD_SAVEDATA_GetBoxOfficeData( POKEWOOD_SAVEDATA *sv, int no );


//=============================================================================================
/**
 * @brief	外部データの撮影データを上映したか？
 *
 * @param   sv		POKEWOOD_SAVEDATA
 * @param   no		撮影データスロット
 *
 * @retval  BOOL	TRUE:上映済み FALSE:上映してない
 */
//=============================================================================================
extern BOOL POKEWOOD_SAVEDATA_GetMovieShowed( POKEWOOD_SAVEDATA *sv, int no );

//=============================================================================================
/**
 * @brief	撮影データを上映フラグを設定する
 *
 * @param   sv    POKEWOOD_SAVEDATA
 * @param   no    撮影データスロット
 * @param   value TRUE or FALSE
 */
//=============================================================================================
extern void POKEWOOD_SAVEDATA_SetMovieShowed( POKEWOOD_SAVEDATA *sv, int no, BOOL value );

//=============================================================================================
/**
 * @brief	このシナリオは成功フラグを立てているか？
 *
 * @param   sv		POKEWOOD_SAVEDATA
 * @param   no		シナリオNO
 *
 * @retval  BOOL	TRUE:上映済み FALSE:上映してない
 */
//=============================================================================================
extern BOOL POKEWOOD_SAVEDATA_GetSuccessFlag( POKEWOOD_SAVEDATA *sv, int no );

//=============================================================================================
/**
 * @brief	シナリオ毎に成功フラグを立てる
 *
 * @param   sv    POKEWOOD_SAVEDATA
 * @param   no		シナリオNO
 * @param   value TRUE or FALSE
 */
//=============================================================================================
extern void POKEWOOD_SAVEDATA_SetSuccessFlag( POKEWOOD_SAVEDATA *sv, int no, BOOL value );

extern  int POKEWOOD_SAVEDATA_GetStarPoint( POKEWOOD_SAVEDATA *sv );
extern void POKEWOOD_SAVEDATA_AddStarPoint( POKEWOOD_SAVEDATA *sv, int value );
extern void POKEWOOD_SAVEDATA_DecStarPoint( POKEWOOD_SAVEDATA *sv, int value );
extern void POKEWOOD_SAVEDATA_SetStarPoint( POKEWOOD_SAVEDATA *sv, int value );
