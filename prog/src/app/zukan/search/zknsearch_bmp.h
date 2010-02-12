//============================================================================================
/**
 * @file		zknsearch_bmp.h
 * @brief		図鑑検索画面 ＢＭＰ関連
 * @author	Hiroyuki Nakamura
 * @date		10.02.09
 *
 *	モジュール名：ZKNSEARCHBMP
 */
//============================================================================================
#pragma	once


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ関連初期化
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHBMP_Init( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ＢＭＰ関連削除
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHBMP_Exit( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		プリントメイン
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHBMP_PrintUtilTrans( ZKNSEARCHMAIN_WORK * wk );


extern void ZKNSEARCHBMP_PutMainPage( ZKNSEARCHMAIN_WORK * wk );
extern void ZKNSEARCHBMP_PutRowPage( ZKNSEARCHMAIN_WORK * wk );
extern void ZKNSEARCHBMP_PutNamePage( ZKNSEARCHMAIN_WORK * wk );
extern void ZKNSEARCHBMP_PutTypePage( ZKNSEARCHMAIN_WORK * wk );
extern void ZKNSEARCHBMP_PutColorPage( ZKNSEARCHMAIN_WORK * wk );
extern void ZKNSEARCHBMP_PutFormPage( ZKNSEARCHMAIN_WORK * wk );

extern void ZKNSEARCHBMP_PutListItem( ZKNSEARCHMAIN_WORK * wk, PRINT_UTIL * util, STRBUF * str );

extern void ZKNSEARCHBMP_PutFormListItem( ZKNSEARCHMAIN_WORK * wk, PRINT_UTIL * util );
