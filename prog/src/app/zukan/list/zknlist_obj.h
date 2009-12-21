//============================================================================================
/**
 * @file		zknlist_obj.h
 * @brief		図鑑リスト画面 ＯＢＪ関連
 * @author	Hiroyuki Nakamura
 * @date		09.12.14
 *
 *	モジュール名：ZKNLISTOBJ
 */
//============================================================================================
#pragma	once


//============================================================================================
//	定数定義
//============================================================================================

// 図鑑用ＯＢＪアニメ定義
enum {
	ZKNLISTOBJ_ANM_START = 0,
	ZKNLISTOBJ_ANM_SELECT,
	ZKNLISTOBJ_ANM_BAR,
	ZKNLISTOBJ_ANM_RAIL,
	ZKNLISTOBJ_ANM_START_ANM,
	ZKNLISTOBJ_ANM_SELECT_ANM,
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター初期化
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTOBJ_Init( ZKNLISTMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター削除
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTOBJ_Exit( ZKNLISTMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメメイン
 *
 * @param		wk		図鑑リストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTOBJ_AnmMain( ZKNLISTMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメ変更
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 * @param		anm		アニメ番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTOBJ_SetAnm( ZKNLISTMAIN_WORK * wk, u32 id, u32 anm );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターオートアニメ設定
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 * @param		anm		アニメ番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTOBJ_SetAutoAnm( ZKNLISTMAIN_WORK * wk, u32 id, u32 anm );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメ状態取得
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 *
 * @retval	"TRUE = アニメ中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL ZKNLISTOBJ_CheckAnm( ZKNLISTMAIN_WORK * wk, u32 id );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター表示切り替え
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 * @param		flg		表示フラグ
 *
 * @return	none
 *
 * @li	flg = TRUE : 表示
 * @li	flg = FALSE : 非表示
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTOBJ_SetVanish( ZKNLISTMAIN_WORK * wk, u32 id, BOOL flg );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター表示チェック
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 *
 * @retval	"TRUE = 表示"
 * @retval	"FALSE = 非表示"
 */
//--------------------------------------------------------------------------------------------
extern BOOL ZKNLISTOBJ_CheckVanish( ZKNLISTMAIN_WORK * wk, u32 id );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター半透明設定
 *
 * @param		wk		図鑑リストワーク
 * @param		id		OBJ ID
 * @param		flg		ON/OFFフラグ
 *
 * @return	none
 *
 * @li	flg : TRUE = ON, FALSE = OFF
 */
//--------------------------------------------------------------------------------------------
extern void BOX2OBJ_SetBlendMode( ZKNLISTMAIN_WORK * wk, u32 id, BOOL flg );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター座標設定
 *
 * @param		wk			図鑑リストワーク
 * @param		id			OBJ ID
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 * @param		setsf		表示画面
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTOBJ_SetPos( ZKNLISTMAIN_WORK * wk, u32 id, s16 x, s16 y, u16 setsf );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター座標取得
 *
 * @param		wk			図鑑リストワーク
 * @param		id			OBJ ID
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 * @param		setsf		表示画面
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTOBJ_GetPos( ZKNLISTMAIN_WORK * wk, u32 id, s16 * x, s16 * y, u16 setsf );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターパレット変更
 *
 * @param		wk			図鑑リストワーク
 * @param		id			OBJ ID
 * @param		pal			パレット番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTOBJ_ChgPltt( ZKNLISTMAIN_WORK * wk, u32 id, u32 pal );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモン正面絵セルアクターセット
 *
 * @param		wk			図鑑リストワーク
 * @param		mons		ポケモン番号
 * @param		form		フォルム
 * @param		sex			性別
 * @param		rare		レアかどうか
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTOBJ_SetPokeGra( ZKNLISTMAIN_WORK * wk, u16 mons, u8 form, u8 sex, u8 rare );

extern void ZKNLISTOBJ_PutListPosPokeGra( ZKNLISTMAIN_WORK * wk, s16 pos );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ポケモンアイコンキャラ書き換え
 *
 * @param		wk			図鑑リストワーク
 * @param		idx			OBJ Index
 * @param		form		フォルム
 * @param		disp		表示画面
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNLISTOBJ_ChgPokeIcon( ZKNLISTMAIN_WORK * wk, u32 idx, u16 mons, u16 form, BOOL disp );

extern void ZKNLISTOBJ_ScrollPokeIcon( ZKNLISTMAIN_WORK * wk, s16 mv );

extern void ZKNLISTOBJ_PutPokeList( ZKNLISTMAIN_WORK * wk, u32 objIdx, s32 listPos, BOOL disp );

extern void ZKNLISTOBJ_PutScrollList( ZKNLISTMAIN_WORK * wk, u32 idx, u32 mv );

extern void ZKNLISTOBJ_InitScrollList( ZKNLISTMAIN_WORK * wk );

extern void ZKNLISTOBJ_ChgListPosAnm( ZKNLISTMAIN_WORK * wk, u32 pos, BOOL flg );

extern void ZKNLISTOBJ_SetScrollBar( ZKNLISTMAIN_WORK * wk, u32 py );
