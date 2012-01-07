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
#include "pokeicon/pokeicon.h"

#define POKEWOOD_BOXOFFICE_DATA_MAX   ( 40 )  ///< 記録として残す興行成績ランキングの数
#define POKEWOOD_MYPOKE_NUM           (  6 )                    ///< ポケモンアイコンで保存する自分の手持ちポケモンアイコンの数
#define POKEWOOD_SHOWING_MAX          ( POKEWOOD_SAVEDATA_MAX ) ///< 同時に上映する最大数

/// ポケウッド情報取得関数用定義
typedef enum{
  PWSV_TYPE_SENARIO_OPEN=0,     ///< シナリオオープンフラグ
  PWSV_TYPE_SENARIO_PLAYED,     ///< シナリオ撮影済みフラグ
  PWSV_TYPE_SENARIO_SUCCESS,    ///< シナリオ成功終了済みフラグ
  PWSV_TYPE_SENARIO_ANNOUNCED,  ///< シナリオオープン告知済みフラグ
  PWSV_TYPE_SENARIO_STRANGE,    ///< ストレンジEND取得済みフラグ
  PWSV_TYPE_SENARIO_THEATERGO,  ///< 指定のシナリオを一度でも上映して観た事がある
  PWSV_TYPE_SENARIO_BAD,        ///< バッドEND取得済みフラグ
} PokewoodFlagType;

// プレイヤーのポケウッドスターランク定義
enum{
  POKEWOOD_STAR_RANK_0=0,
  POKEWOOD_STAR_RANK_1,
  POKEWOOD_STAR_RANK_2,
  POKEWOOD_STAR_RANK_3,
  POKEWOOD_STAR_RANK_4,
  POKEWOOD_STAR_RANK_MAX,   // 使いません(0-4までしかない)
};

/// ポケウッド興行成績データ構造体定義
typedef struct {
  int     profits;                        ///< 収益             4
  u16     pokeicon[POKEWOOD_MYPOKE_NUM];  ///< ポケモンアイコン 2x6
  u8      new_flag;                       ///< 新着フラグ(0:閲覧してない, 1:閲覧済み）
  u8      update_flag;                    ///< 更新フラグ(0:更新無し,1:更新アリ）
  u8      pokeicon_star_bit;              ///< ポケモンアイコンにスターをつけるかどうか(0-5bitが対応）
  u8      etc;                            ///< 余り
                                          ///< 計 12+4+2 = 18
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

extern BOOL POKEWOOD_SAVEDATA_GetPokewoodFlag( const POKEWOOD_SAVEDATA *sv, PokewoodFlagType type, int no );
extern void POKEWOOD_SAVEDATA_SetPokewoodFlag( POKEWOOD_SAVEDATA *sv, PokewoodFlagType type, int no );

//=============================================================================================
/**
 * @brief	ポケウッドシナリオ解放フラグ
 */
//=============================================================================================
extern BOOL POKEWOOD_SAVEDATA_GetPokewoodOpenFlag( const POKEWOOD_SAVEDATA *sv, int no );
extern void POKEWOOD_SAVEDATA_SetPokewoodOpenFlag( POKEWOOD_SAVEDATA *sv, int no );
extern  int POKEWOOD_SAVEDATA_GetFlagTotal( POKEWOOD_SAVEDATA *sv, PokewoodFlagType type );


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
extern void  POKEWOOD_SAVEDATA_SetBoxOfficeData( POKEWOOD_SAVEDATA *sv, int no, POKEWOOD_BOXOFFICE_RECORD *data );
extern void  POKEWOOD_SAVEDATA_ClearUpdate( POKEWOOD_SAVEDATA *sv, int no );

#ifdef PM_DEBUG
extern void POKEWOOD_SAVEDATA_SetForceBoxOfficeData(POKEWOOD_SAVEDATA *sv, int no, POKEWOOD_BOXOFFICE_RECORD *data );
#endif

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
extern int  POKEWOOD_SAVEDATA_GetKanbanNo( POKEWOOD_SAVEDATA *sv, u32 index );
extern void POKEWOOD_SAVEDATA_SetKanbanNo( POKEWOOD_SAVEDATA *sv, u32 index, int title_no );
extern void POKEWOOD_SAVEDATA_SetKanbanTemporary( POKEWOOD_SAVEDATA *sv, u32 index, int title_no );


#ifdef PM_DEBUG
BOOL POKEWOOD_SAVEDATA_GetBattleDebugFlag( int index );
void POKEWOOD_SAVEDATA_SetBattleDebugFlag( int index, int value );
#endif