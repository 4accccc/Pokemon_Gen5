//============================================================================================
/**
 * @file		zknsearch_list.h
 * @brief		図鑑検索画面 条件選択リスト処理
 * @author	Hiroyuki Nakamura
 * @date		10.02.10
 */
//============================================================================================
#pragma	once



extern void ZKNSEARCHLIST_MakeRowList( ZKNSEARCHMAIN_WORK * wk );

extern void ZKNSEARCHLIST_MakeNameList( ZKNSEARCHMAIN_WORK * wk );

extern void ZKNSEARCHLIST_MakeTypeList( ZKNSEARCHMAIN_WORK * wk );

extern void ZKNSEARCHLIST_MakeColorList( ZKNSEARCHMAIN_WORK * wk );

extern void ZKNSEARCHLIST_MakeFormList( ZKNSEARCHMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief	  リスト削除
 *
 * @param		wk		図鑑検索画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void ZKNSEARCHLIST_FreeList( ZKNSEARCHMAIN_WORK * wk );
