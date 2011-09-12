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

#define POKEWOOD_BOXOFFICE_DATA_MAX   ( POKEWOOD_SENARIO_MAX )  ///< 記録として残す興行成績ランキングの数
#define POKEWOOD_MYPOKE_NUM           (  6 )                    ///< ポケモンアイコンで保存する自分の手持ちポケモンアイコンの数
#define POKEWOOD_SHOWING_MAX          ( POKEWOOD_SAVEDATA_MAX ) ///< 同時に上映する最大数

/// ポケウッド情報取得関数用定義
enum{
  PWSV_TYPE_SENARIO_OPEN=0,     ///< シナリオオープンフラグ
  PWSV_TYPE_SENARIO_PLAYED,     ///< シナリオ撮影済みフラグ
  PWSV_TYPE_SENARIO_SUCCESS,    ///< シナリオ成功終了済みフラグ
  PWSV_TYPE_SENARIO_ANNOUNCED,  ///< シナリオオープン告知済みフラグ
};


/// ポケウッド興行成績データ構造体定義
typedef struct {
  int     profits;                          // 収益             4
  u16     pokeicon[POKEWOOD_MYPOKE_NUM];    // ポケモンアイコン 2x6
                                            // 計 12+4 = 16
//  STRCODE nickname[BUFLEN_POKEMON_NAME];  // ポケモン名 8+12*2=32
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

extern BOOL POKEWOOD_SAVEDATA_GetPokewoodFlag( const POKEWOOD_SAVEDATA *sv, int type, int no );
extern void POKEWOOD_SAVEDATA_SetPokewoodFlag( POKEWOOD_SAVEDATA *sv, int type, int no );

//=============================================================================================
/**
 * @brief	ポケウッドシナリオ解放フラグ
 */
//=============================================================================================
extern BOOL POKEWOOD_SAVEDATA_GetPokewoodOpenFlag( const POKEWOOD_SAVEDATA *sv, int no );
extern void POKEWOOD_SAVEDATA_SetPokewoodOpenFlag( POKEWOOD_SAVEDATA *sv, int no );
extern  int POKEWOOD_SAVEDATA_GetSenarioOpenNum( POKEWOOD_SAVEDATA *sv );


//=============================================================================================
/**
 * @brief	ポケウッド上映済みフラグ
 * @retval  BOOL		TRUE:上映済み FALSE:未上映
 */
//=============================================================================================
extern BOOL POKEWOOD_SAVEDATA_GetPokewoodPlayedFlag( const POKEWOOD_SAVEDATA *sv, int no );
extern void POKEWOOD_SAVEDATA_SetPokewoodPlayedFlag( POKEWOOD_SAVEDATA *sv, int no );


//=============================================================================================
/**
 * @brief	興行成績上位者のデータを取得する
 * @retval  const POKEWOOD_BOXOFFICE_RECORD *		
 */
//=============================================================================================
extern const POKEWOOD_BOXOFFICE_RECORD *POKEWOOD_SAVEDATA_GetBoxOfficeData( POKEWOOD_SAVEDATA *sv, int no );
extern void  POKEWOOD_SAVEDATA_SetBoxOfficeData( POKEWOOD_SAVEDATA *sv, POKEWOOD_BOXOFFICE_RECORD *data );



//=============================================================================================
/**
 * @brief	外部データの撮影データを上映したか？
 * @retval  BOOL	TRUE:上映済み FALSE:上映してない
 */
//=============================================================================================
extern BOOL POKEWOOD_SAVEDATA_GetMovieShowed( POKEWOOD_SAVEDATA *sv, int no );
extern void POKEWOOD_SAVEDATA_SetMovieShowed( POKEWOOD_SAVEDATA *sv, int no, BOOL value );

//=============================================================================================
/**
 * @brief	このシナリオは成功フラグを立てているか？
 * @retval  BOOL	TRUE:成功フラグ取得済み FALSE:成功フラグ取得してない
 */
//=============================================================================================
extern BOOL POKEWOOD_SAVEDATA_GetSuccessFlag( POKEWOOD_SAVEDATA *sv, int no );
extern void POKEWOOD_SAVEDATA_SetSuccessFlag( POKEWOOD_SAVEDATA *sv, int no, BOOL value );

//=============================================================================================
/**
 * @brief プレイヤーのスターポイント関連
 */
//=============================================================================================
extern  int POKEWOOD_SAVEDATA_GetStarPoint( POKEWOOD_SAVEDATA *sv );
extern void POKEWOOD_SAVEDATA_AddStarPoint( POKEWOOD_SAVEDATA *sv, int value );
extern void POKEWOOD_SAVEDATA_DecStarPoint( POKEWOOD_SAVEDATA *sv, int value );
extern void POKEWOOD_SAVEDATA_SetStarPoint( POKEWOOD_SAVEDATA *sv, int value );
extern  int POKEWOOD_SAVEDATA_CalcStarPoint2StarRank( int point );

//=============================================================================================
/**
 * @brief	指定のスターランクを告知済みか？
 */
//=============================================================================================
extern BOOL POKEWOOD_SAVEDATA_GetStarRankAnnounced( POKEWOOD_SAVEDATA *sv, int rank );
extern void POKEWOOD_SAVEDATA_SetStarRankAnnounced( POKEWOOD_SAVEDATA *sv, int rank, BOOL value );


//=============================================================================================
/**
 * @brief	上映中の映画の看板NO関連
 * @retval  int	上映中の映画の看板NO
 */
//=============================================================================================
int  POKEWOOD_SAVEDATA_GetKanbanNo( POKEWOOD_SAVEDATA *sv, u32 index );
void POKEWOOD_SAVEDATA_SetKanbanNo( POKEWOOD_SAVEDATA *sv, u32 index, int title_no );
