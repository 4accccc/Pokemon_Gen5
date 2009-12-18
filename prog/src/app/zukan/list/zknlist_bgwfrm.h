//============================================================================================
/**
 * @file		zknlist_bgwfrm.h
 * @brief		図鑑リスト画面 ＢＧウィンドウフレーム関連
 * @author	Hiroyuki Nakamura
 * @date		09.12.16
 *
 *	モジュール名：ZKNLISTBGWFRM
 */
//============================================================================================
#pragma	once


//============================================================================================
//	定数定義
//============================================================================================

// リストフレーム番号
enum {
	ZKNLISTBGWFRM_POKE_GET = 0,		// 捕獲済み
	ZKNLISTBGWFRM_POKE_NONE,			// 捕獲していない
};

enum {
	ZKNLISTBGWFRM_LISTPUT_UP = 0,
	ZKNLISTBGWFRM_LISTPUT_DOWN
};


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

extern void ZKNLISTBGWFRM_Init( ZKNLISTMAIN_WORK * wk );

extern void ZKNLISTBGWFRM_Exit( ZKNLISTMAIN_WORK * wk );

extern void ZKNLISTBGWFRM_SetNameFrame( ZKNLISTMAIN_WORK * wk, u32 idx, u32 num );

extern void ZKNLISTBGWFRM_PutPokeList( ZKNLISTMAIN_WORK * wk, u32 idx, s32 listPos );

extern void ZKNLISTBGWFRM_PutScrollList( ZKNLISTMAIN_WORK * wk, u32 idx, u32 mv );

extern void ZKNLISTBGWFRM_PutScrollListSub( ZKNLISTMAIN_WORK * wk, u32 idx, u32 mv );
