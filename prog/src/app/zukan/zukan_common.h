//============================================================================================
/**
 * @file		zukan_common.h
 * @brief		図鑑画面 共通処理
 * @author	Hiroyuki Nakamura
 * @date		09.12.15
 *
 *	モジュール名：ZKNCOMM
 */
//============================================================================================
#pragma	once

#include "gamesystem/pm_season.h"
#include "savedata/zukan_savedata.h"
#include "search/zukan_search_engine.h"
#include "habitat_list/habitat_list.h"


//============================================================================================
//	定数定義
//============================================================================================

// 上下バーのスクロールスピード
#define ZKNCOMM_BAR_SPEED   (8)     // 毎フレームZKNCOMM_BAR_SPEEDだけ移動する
// フェードイン/アウトのスピード
#define ZKNCOMM_FADE_WAIT   (-14)     // PaletteFadeReqWriteのwaitに渡せる値

#define	FRAME_SCROLL_COUNT	( 48 )		// 背景スクロールカウント


// セルアクターデータ
typedef struct {
	GFL_CLWK_DATA	dat;

	u32	chrRes;
	u32	palRes;
	u32	celRes;

	u16	pal;
	u16	disp;

}ZKNCOMM_CLWK_DATA;

// リストデータ
enum {
	ZUKAN_LIST_MONS_NONE = 0,		// 未確認
	ZUKAN_LIST_MONS_SEE,				// 見た
	ZUKAN_LIST_MONS_GET,				// 捕獲した
};


// 地形タイプ
enum {
  GROUND_TYPE_LAND,
  GROUND_TYPE_WATERSIDE,
  GROUND_TYPE_FISHING,
  GROUND_TYPE_MAX,
};

// ポケモン生息データ
typedef struct {
  u16 mons_no;                          // モンスター番号
  u8  landtype_appear[GROUND_TYPE_MAX]; // 地形存在フラグ
  u8  season_appear[PMSEASON_TOTAL];    // 四季生息データ
  u8  pad[1];                           // パディング
}HABITAT_POKE_DATA;

// 生息地アーカイブデータ
typedef struct {
	u16	  zone_group_id;                      // ゾーングループID
  u8    version;                            // バージョン制限
  u8    landtype_appear[GROUND_TYPE_MAX];   // 地形存在フラグ
  u8    mons_num;                           // 登録ポケモン数
  HABITAT_POKE_DATA mons_data[1]; // モンスターデータ(可変長)
}HABITATZONEGROUP_DATA;


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		デフォルトリスト作成
 *
 * @param		sv			図鑑セーブデータ
 * @param		list		リスト作成場所
 * @param		heapID	ヒープＩＤ
 *
 * @return	リストに登録した数
 */
//--------------------------------------------------------------------------------------------
extern u16 ZKNCOMM_MakeDefaultList( ZUKAN_SAVEDATA * sv, u16 ** list, HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		デフォルトポケモンを登録されているものにする
 *
 * @param		sv		図鑑セーブデータ
 * @param		list	リストデータ
 * @param		max		リスト項目数
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNCOMM_InitDefaultMons( ZUKAN_SAVEDATA * sv, u16 * list, u16 max );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ビクティチェック
 *
 * @param		sv		図鑑セーブデータ
 *
 * @retval	"TRUE = 表示する"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL ZKNCOMM_CheckLocalListNumberZero( ZUKAN_SAVEDATA * sv );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ソートデータリセット
 *
 * @param		sv		図鑑セーブデータ
 * @param		sort	ソートデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNCOMM_ResetSortData( const ZUKAN_SAVEDATA * sv, ZKNCOMM_LIST_SORT * sort );

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードインセット
 *
 * @param		heapID		ヒープＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNCOMM_SetFadeIn( HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードアウトセット
 *
 * @param		heapID		ヒープＩＤ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNCOMM_SetFadeOut( HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * @brief		背景スクロール
 *
 * @param		mainBG		メイン画面のＢＧフレーム
 * @param		subBG			サブ画面のＢＧフレーム
 * @param		cnt				カウンタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNCOMM_ScrollBaseBG( u8 mainBG, u8 subBG, u32 * cnt );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター追加
 *
 * @param		unit		GFL_CLUNIT
 * @param		prm			セルアクターデータ
 * @param		heapID	ヒープＩＤ
 *
 * @return	GFL_CLWK
 */
//--------------------------------------------------------------------------------------------
extern GFL_CLWK * ZKNCOMM_CreateClact( GFL_CLUNIT * unit, const ZKNCOMM_CLWK_DATA * prm, HEAPID heapID );

//----------------------------------------------------------------------------
/**
 *	@brief  生息地リスト用リストデータを作成する
 *
 *	@param	* p_zone_info 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
extern void ZKNCOMM_MakeHabitatZoneListData( HABITATLIST_SAVE *p_sv,ZUKANHABITAT_ZONE_DATA * p_zone_info,BOOL is_set_single_zone );

//----------------------------------------------------------------------------
/**
 *	@brief  生息地リスト用リストデータ作成
 *
 *	@param	* zone_list 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
extern void ZKNCOMM_MakeHabitatListData( HABITATLIST_SAVE *p_sv,ZUKANHABITAT_LIST_DATA * p_zone_list );

extern u16 ZKNCOMM_MakeHabitatList( ZUKAN_SAVEDATA * p_zkn_sv,HABITATLIST_SAVE  *p_habitat_sv, u16 ** list, HEAPID heapID );

