//============================================================================================
/**
 * @brief	box_savedata.h
 * @brief	ボックスセーブデータ系
 * @date	2008.11.07
 * @author	ariizumi
 */
//============================================================================================

#pragma once

#include "poke_tool/poke_tool.h"
#include "savedata/save_control.h"

//============================================================================================
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	ボックスデータの不完全型定義
 */
//---------------------------------------------------------------------------
typedef struct _BOX_MANAGER BOX_MANAGER;
typedef struct _BOX_SAVEDATA BOX_SAVEDATA;
typedef struct _BOX_TRAY_DATA BOX_TRAY_DATA;

//------------------------------------------------------------
/**
 *   定数
 */
//------------------------------------------------------------
#define BOX_MAX_RAW				(5)
#define BOX_MAX_COLUMN			(6)
#define BOX_MIN_TRAY			(8)
#define BOX_MED_TRAY			(16)
#define BOX_MAX_TRAY			(24)
#define BOX_TRAYNAME_MAXLEN		(8)
#define BOX_TRAYNAME_BUFSIZE	(20)	// 日本語８文字＋EOM。海外版用の余裕も見てこの程度。
#define BOX_MAX_POS				(BOX_MAX_RAW*BOX_MAX_COLUMN)
#define BOX_POKESET_MAX		(BOX_MAX_POS*BOX_MAX_TRAY)		// ボックス全体に格納できるポケモン数


#define BOX_NORMAL_WALLPAPER_MAX	(16)
#define BOX_EX_WALLPAPER_MAX			(8)
#define BOX_TOTAL_WALLPAPER_MAX		(BOX_NORMAL_WALLPAPER_MAX+BOX_EX_WALLPAPER_MAX)
/*
#define BOX_PL_EX_WALLPAPER_MAX		(8)	//プラチナで追加された秘密壁紙
#define BOX_GS_EX_WALLPAPER_MAX		(8)	//金銀で追加された秘密壁紙
#define BOX_TOTAL_WALLPAPER_MAX_PL	(BOX_NORMAL_WALLPAPER_MAX+BOX_EX_WALLPAPER_MAX+BOX_PL_EX_WALLPAPER_MAX)
#define BOX_TOTAL_WALLPAPER_MAX_GS	(BOX_NORMAL_WALLPAPER_MAX+BOX_EX_WALLPAPER_MAX+BOX_PL_EX_WALLPAPER_MAX+BOX_GS_EX_WALLPAPER_MAX)
*/

#define BOXDAT_TRAYNUM_CURRENT		(0xffffffff)	///< トレイナンバーを引数に取る関数で指定すると、カレントを指定したことになる。
#define BOXDAT_TRAYNUM_ERROR		(BOX_MAX_TRAY)	///< 空きのあるトレイサーチで、見つからなかった時などの戻り値




//------------------------------------------------------------------
/**
 * ボックスデータの内容を初期化する
 *
 * @param   boxdat		ボックスデータのポインタ
 */
//------------------------------------------------------------------
extern void BOXDAT_Init( BOX_SAVEDATA* boxdat );

//------------------------------------------------------------------
/**
 * ボックスデータ領域の総サイズを返す（セーブデータ管理用）
 *
 * @retval  u32		ボックスデータ総サイズ
 */
//------------------------------------------------------------------
extern u32 BOXDAT_GetTotalSize( void );


//BOX_MANAGERはgamedata経由のアクセスになりました。
//------------------------------------------------------------------
/**
 * ボックス管理マネージャーの取得
 */
//------------------------------------------------------------------
extern BOX_MANAGER * BOX_DAT_InitManager( const HEAPID heapId , SAVE_CONTROL_WORK * sv);
//------------------------------------------------------------------
/**
 * ボックス管理マネージャーの開放
 */
//------------------------------------------------------------------
extern void BOX_DAT_ExitManager( BOX_MANAGER *box );

//------------------------------------------------------------------
/**
 * ボックス全体からから空き領域を探してポケモンデータを格納
 *
 * @param   box			ボックスデータポインタ
 * @param   poke		ポケモンデータ
 *
 * @retval  BOOL		TRUE=格納された／FALSE=空きが無い
 */
//------------------------------------------------------------------
extern BOOL BOXDAT_PutPokemon( BOX_MANAGER* box, const POKEMON_PASO_PARAM* poke );

//------------------------------------------------------------------
/**
 * ボックスを指定してポケモンデータ格納
 *
 * @param   box			ボックスデータポインタ
 * @param   boxNum		何番目のボックスに格納するか
 * @param   poke		ポケモンデータ
 *
 * @retval  BOOL		TRUE=格納された／FALSE=空きが無い
 */
//------------------------------------------------------------------
extern BOOL BOXDAT_PutPokemonBox( BOX_MANAGER* box, u32 boxNum, const POKEMON_PASO_PARAM* poke );

//------------------------------------------------------------------
/**
 * ボックス、位置を指定してポケモンデータ格納
 *
 * @param   box			ボックスデータポインタ
 * @param   boxNum		何番目のボックスに格納するか
 * @param   x			ボックス内の位置Ｘ
 * @param   y			ボックス内の位置Ｙ
 * @param   poke		ポケモンデータ
 *
 * @retval  BOOL		TRUE=格納された／FALSE=空きが無い
 */
//------------------------------------------------------------------
extern BOOL BOXDAT_PutPokemonPos( BOX_MANAGER* box, u32 boxNum, u32 pos, const POKEMON_PASO_PARAM* poke );


//------------------------------------------------------------------
/**
 * １つでも空きのあるトレイナンバーを返す（カレントから検索開始する）
 *
 * @param   box		ボックスデータポインタ
 *
 * @retval  u32		空きのあるトレイナンバー／見つからなければ BOXDAT_TRAYNUM_ERROR が返る
 */
//------------------------------------------------------------------
extern u32 BOXDAT_GetEmptyTrayNumber( const BOX_MANAGER* box );

//------------------------------------------------------------------
/**
 * 指定位置のポケモンを入れ替える
 *
 * @param   box			ボックスデータポインタ
 * @param   trayNum		何番目のボックスか
 * @param   pos1		ボックス内の位置
 * @param   pos2		ボックス内の位置
 *
 */
//------------------------------------------------------------------
extern void BOXDAT_ChangePokeData( BOX_MANAGER * box, u32 tray1, u32 pos1, u32 tray2, u32 pos2 );

//------------------------------------------------------------------
/**
 * 位置を指定してカレントボックスのポケモンデータクリア
 *
 * @param   box			ボックスデータポインタ
 * @param   trayNumber	何番目のボックスか
 * @param   pos			ボックス内の位置
 *
 */
//------------------------------------------------------------------
extern void BOXDAT_ClearPokemon( BOX_MANAGER* box, u32 trayNum, u32 pos );

//------------------------------------------------------------------
/**
 * ボックス全体に含まれるポケモン数を返す
 *
 * @param   box		ボックスデータポインタ
 *
 * @retval  u32		
 */
//------------------------------------------------------------------
extern u32 BOXDAT_GetPokeExistCountTotal( const BOX_MANAGER* box );
extern u32 BOXDAT_GetPokeExistCount2Total( const BOX_MANAGER* box );

extern u32 BOXDAT_GetTrayMax( const BOX_MANAGER * box );
extern u32 BOXDAT_AddTrayMax( BOX_MANAGER * box );

extern u32 BOXDAT_GetEmptySpaceTotal( const BOX_MANAGER* box );
extern u32 BOXDAT_GetEmptySpaceTray( const BOX_MANAGER* box, u32 trayNum );

extern u32 BOXDAT_GetCureentTrayNumber( const BOX_MANAGER* box );
extern void BOXDAT_SetCureentTrayNumber( BOX_MANAGER* box, u32 num );
extern u32 BOXDAT_GetWallPaperNumber( const BOX_MANAGER* box, u32 trayNumber );
extern void BOXDAT_SetWallPaperNumber( BOX_MANAGER* box, u32 trayNum, u32 wallPaperNumber );
extern void BOXDAT_GetBoxName( const BOX_MANAGER* box, u32 trayNumber, STRBUF* buf );
extern void BOXDAT_SetBoxName( BOX_MANAGER* box, u32 trayNumber, const STRBUF* src );
extern u32 BOXDAT_GetPokeExistCount( const BOX_MANAGER* box, u32 trayNumber );
extern u32 BOXDAT_GetPokeExistCount2( const BOX_MANAGER* box, u32 trayNumber );
extern POKEMON_PASO_PARAM* BOXDAT_GetPokeDataAddress( const BOX_MANAGER* box, u32 boxNum, u32 pos );
extern BOOL BOXDAT_GetEmptyTrayNumberAndPos( const BOX_MANAGER* box, int* trayNum, int* pos );
extern u32 BOXDAT_PokeParaGet( const BOX_MANAGER* box, u32 trayNum, u32 pos, int param, void* buf );
extern void BOXDAT_PokeParaPut( BOX_MANAGER* box, u32 trayNum, u32 pos, int param, u32 arg );


//==============================================================================================
// だいすきクラブ壁紙
//==============================================================================================
extern void BOXDAT_SetDaisukiKabegamiFlag( BOX_MANAGER* box, u32 number );
extern BOOL BOXDAT_GetDaisukiKabegamiFlag( const BOX_MANAGER* box, u32 number );
extern u32  BOXDAT_GetDaiukiKabegamiCount( const BOX_MANAGER* box );


//extern void BOXDAT_SetTrayUseBit(BOX_MANAGER* box, const u8 inTrayIdx);
//extern void BOXDAT_SetTrayUseBitAll(BOX_MANAGER* box);
//extern void BOXDAT_ClearTrayUseBits(BOX_MANAGER* box);
//extern u32 BOXDAT_GetTrayUseBits(const BOX_MANAGER* box);
extern u32 BOXDAT_GetOneBoxDataSize(void);
extern void BOXDAT_CheckBoxDummyData(BOX_MANAGER* box);


extern void BOXDAT_SetPPPData_Tray( u8 trayIdx , void *dataPtr , BOX_MANAGER *boxData );

//==============================================================================================
// ボックスデータ分割処理
//==============================================================================================
extern void BOXTRAYDAT_Init( BOX_TRAY_DATA* traydat );
extern u32 BOXTRAYDAT_GetTotalSize( void );
