//==============================================================================
/**
 * @file	pokeicon.h
 * @brief	ポケモンアイコンのヘッダ
 * @author	matsuda
 * @date	2008.11.26(水)
 */
//==============================================================================
#ifndef __POKEICON_H__
#define __POKEICON_H__

#include "poke_tool/poke_tool.h"
//マルチブート用きり分け
#ifndef MULTI_BOOT_MAKE  //通常時処理

#define GENDER_VER

#else                    //DL子機時処理

//#define GENDER_VER

#endif //MULTI_BOOT_MAKE


//==============================================================================
//	定数定義
//==============================================================================
// アニメ種類
enum {
	POKEICON_ANM_DEATH = 0,		// 瀕死
	POKEICON_ANM_HPMAX,			// HP MAX
	POKEICON_ANM_HPGREEN,		// HP 緑
	POKEICON_ANM_HPYERROW,		// HP 黄
	POKEICON_ANM_HPRED,			// HP 赤
	POKEICON_ANM_STCHG,			// 状態異常
};

///アイコンで使用するパレット数
#define	POKEICON_PAL_MAX	( 3 )

/// アイコンの横サイズ
#define POKEICON_SIZE_X		(32)
/// アイコンの縦サイズ
#define POKEICON_SIZE_Y		(32)

///ポケモンアイコンのCGXデータサイズ(1体分) ※アニメ込み
#define POKEICON_SIZE_CGX	(0x20*16 * 2)


//==============================================================================
//	外部関数宣言
//==============================================================================
extern u32 POKEICON_GetCgxArcIndex( const POKEMON_PASO_PARAM* ppp );
#ifdef GENDER_VER
extern u32 POKEICON_GetCgxArcIndexByMonsNumber( u32 mons, u32 form_no, u32 sex, BOOL egg );
extern const u8 POKEICON_GetPalNum( u32 mons, u32 form, u32 sex, BOOL egg );
#else
extern u32 POKEICON_GetCgxArcIndexByMonsNumber( u32 mons, u32 form_no, BOOL egg );
extern const u8 POKEICON_GetPalNum( u32 mons, u32 form, BOOL egg );
#endif
extern const u8 POKEICON_GetPalNumByPPP( const POKEMON_PASO_PARAM * ppp );
extern u8 POKEICON_GetPalNumGetByPPP( const POKEMON_PASO_PARAM * ppp );
extern u32 POKEICON_GetPalArcIndex(void);
extern u32 POKEICON_GetCellArcIndex(void);
extern u32 POKEICON_GetCellSubArcIndex(void);
extern u32 POKEICON_GetAnmArcIndex(void);
extern u32 POKEICON_GetAnmSubArcIndex(void);

#endif	//__POKEICON_H__
