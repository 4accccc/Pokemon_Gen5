//============================================================================================
/**
 * @file		zknsearch_obj.h
 * @brief		図鑑検索画面 ＯＢＪ関連
 * @author	Hiroyuki Nakamura
 * @date		10.02.09
 *
 *	モジュール名：ZKNSEARCHOBJ
 */
//============================================================================================
#pragma	once


//============================================================================================
//	定数定義
//============================================================================================

// 図鑑用ＯＢＪアニメ定義
enum {
	ZKNSEARCHOBJ_ANM_BAR = 0,
	ZKNSEARCHOBJ_ANM_RAIL,
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター初期化
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHOBJ_Init( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター削除
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHOBJ_Exit( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメメイン
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHOBJ_AnmMain( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメ変更
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 * @param		anm		アニメ番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHOBJ_SetAnm( ZKNSEARCHMAIN_WORK * wk, u32 id, u32 anm );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターオートアニメ設定
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 * @param		anm		アニメ番号
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHOBJ_SetAutoAnm( ZKNSEARCHMAIN_WORK * wk, u32 id, u32 anm );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメ取得
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 *
 * @return	アニメ番号
 */
//--------------------------------------------------------------------------------------------
extern u32 ZKNSEARCHOBJ_GetAnm( ZKNSEARCHMAIN_WORK * wk, u32 id );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクターアニメ状態取得
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 *
 * @retval	"TRUE = アニメ中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL ZKNSEARCHOBJ_CheckAnm( ZKNSEARCHMAIN_WORK * wk, u32 id );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター表示切り替え
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 * @param		flg		表示フラグ
 *
 * @return	none
 *
 * @li	flg = TRUE : 表示
 * @li	flg = FALSE : 非表示
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHOBJ_SetVanish( ZKNSEARCHMAIN_WORK * wk, u32 id, BOOL flg );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター表示チェック
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 *
 * @retval	"TRUE = 表示"
 * @retval	"FALSE = 非表示"
 */
//--------------------------------------------------------------------------------------------
extern BOOL ZKNSEARCHOBJ_CheckVanish( ZKNSEARCHMAIN_WORK * wk, u32 id );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター半透明設定
 *
 * @param		wk		図鑑検索画面ワーク
 * @param		id		OBJ ID
 * @param		flg		ON/OFFフラグ
 *
 * @return	none
 *
 * @li	flg : TRUE = ON, FALSE = OFF
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHOBJ_SetBlendMode( ZKNSEARCHMAIN_WORK * wk, u32 id, BOOL flg );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター座標設定
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		id			OBJ ID
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 * @param		setsf		表示画面
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHOBJ_SetPos( ZKNSEARCHMAIN_WORK * wk, u32 id, s16 x, s16 y, u16 setsf );

//--------------------------------------------------------------------------------------------
/**
 * @brief		セルアクター座標取得
 *
 * @param		wk			図鑑検索画面ワーク
 * @param		id			OBJ ID
 * @param		x				Ｘ座標
 * @param		y				Ｙ座標
 * @param		setsf		表示画面
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHOBJ_GetPos( ZKNSEARCHMAIN_WORK * wk, u32 id, s16 * x, s16 * y, u16 setsf );

//extern void ZKNSEARCHOBJ_VanishAll( ZKNSEARCHMAIN_WORK * wk );

extern void ZKNSEARCHOBJ_VanishList( ZKNSEARCHMAIN_WORK * wk );



extern void ZKNSEARCHOBJ_PutMainPage( ZKNSEARCHMAIN_WORK * wk );
extern void ZKNSEARCHOBJ_PutRowPage( ZKNSEARCHMAIN_WORK * wk );
extern void ZKNSEARCHOBJ_PutNamePage( ZKNSEARCHMAIN_WORK * wk );
extern void ZKNSEARCHOBJ_PutTypePage( ZKNSEARCHMAIN_WORK * wk );
extern void ZKNSEARCHOBJ_PutColorPage( ZKNSEARCHMAIN_WORK * wk );
extern void ZKNSEARCHOBJ_PutFormPage( ZKNSEARCHMAIN_WORK * wk );

extern void ZKNSEARCHOBJ_PutFormListNow( ZKNSEARCHMAIN_WORK * wk );


extern void ZKNSEARCHOBJ_SetScrollBar( ZKNSEARCHMAIN_WORK * wk, u32 py );

extern void ZKNSEARCHOBJ_SetListPageArrowAnime( ZKNSEARCHMAIN_WORK * wk, BOOL anm );

extern void ZKNSEARCHOBJ_PutMark( ZKNSEARCHMAIN_WORK * wk, u16 num, s16 py, BOOL disp );
extern void ZKNSEARCHOBJ_ChangeMark( ZKNSEARCHMAIN_WORK * wk, u16 pos, BOOL flg );
extern void ZKNSEARCHOBJ_ChangeTypeMark( ZKNSEARCHMAIN_WORK * wk, u8	pos1, u8 pos2 );
extern void ZKNSEARCHOBJ_PutFormMark( ZKNSEARCHMAIN_WORK * wk, s16 py, BOOL disp );
extern void ZKNSEARCHOBJ_ChangeFormMark( ZKNSEARCHMAIN_WORK * wk, u16 pos, BOOL flg );
extern void ZKNSEARCHOBJ_PutFormList( ZKNSEARCHMAIN_WORK * wk, u16 num, s16 py, BOOL disp );

extern void ZKNSEARCHOBJ_ScrollList( ZKNSEARCHMAIN_WORK * wk, s8 mv );
extern void ZKNSEARCHOBJ_ScrollFormList( ZKNSEARCHMAIN_WORK * wk, s8 mv );

extern void ZKNSEARCHOBJ_MoveLoadingBar( ZKNSEARCHMAIN_WORK * wk, u32 cnt );
