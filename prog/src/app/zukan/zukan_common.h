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

#include "savedata/zukan_savedata.h"


//============================================================================================
//	定数定義
//============================================================================================

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


extern void ZKNCOMM_MakeDefaultList( ZUKAN_SAVEDATA * sv, u16 * list );

extern void ZKNCOMM_SetFadeIn( HEAPID heapID );

extern void ZKNCOMM_SetFadeOut( HEAPID heapID );

extern void ZKNCOMM_ScrollBaseBG( u8 mainBG, u8 subBG, u32 * cnt );


extern GFL_CLWK * ZKNCOMM_CreateClact( GFL_CLUNIT * unit, const ZKNCOMM_CLWK_DATA * prm, HEAPID heapID );
