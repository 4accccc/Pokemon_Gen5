//============================================================================================
/**
 * @file		dpc_main.h
 * @brief		殿堂入り確認画面 メイン処理
 * @author	Hiroyuki Nakamura
 * @date		10.03.29
 *
 *	モジュール名：DPCMAIN
 */
//============================================================================================
#pragma	once

#include "system/main.h"
#include "system/palanm.h"
#include "system/bmp_oam.h"
#include "savedata/dendou_save.h"
#include "print/printsys.h"
#include "print/wordset.h"

#include "app/dendou_pc.h"
#include "dpc_bmp_def.h"
#include "dpc_obj_def.h"


//============================================================================================
//	定数定義
//============================================================================================

// 後方確保用ヒープＩＤ
#define	HEAPID_DENDOU_PC_L		( GFL_HEAP_LOWID(HEAPID_DENDOU_PC) )

// ＢＧパレット
#define	DPCMAIN_MBG_PAL_FONT	( 15 )
#define	DPCMAIN_SBG_PAL_FONT	( 15 )


typedef struct {
	DENDOU_POKEMON_DATA	dat[6];
	RTCDate	date;
	u16	pokeMax;
	u16	recNo;
}DPC_PARTY_DATA;

// ワーク
typedef struct {
	DENDOUPC_PARAM * dat;

	DENDOU_RECORD * rec;
	DENDOU_SAVEDATA * ex_rec;

	DPC_PARTY_DATA	party[DENDOU_RECORD_MAX];
	s16	nowRad[6];
	s16	posRad[6];

	GFL_TCB * vtask;		// TCB ( VBLANK )
	GFL_TCB * htask;		// TCB ( HBLANK )

	PALETTE_FADE_PTR	pfd;		// パレットフェードデータ
	u8	posEvy[6];
	u8	nowEvy[6];
	u8	tmpEvy[6];

	PRINT_UTIL	win[DPCBMP_WINID_MAX];		// BMPWIN

	GFL_FONT * font;						// 通常フォント
	GFL_FONT * nfnt;						// 数字フォント
	GFL_MSGDATA * mman;					// メッセージデータマネージャ
	WORDSET * wset;							// 単語セット
	STRBUF * exp;								// メッセージ展開領域
	PRINT_QUE * que;						// プリントキュー

	// OBJ
	GFL_CLUNIT * clunit;
	GFL_CLWK * clwk[DPCOBJ_ID_MAX];
	u32	chrRes[DPCOBJ_CHRRES_MAX];
	u32	palRes[DPCOBJ_PALRES_MAX];
	u32	celRes[DPCOBJ_CELRES_MAX];

	// ＯＡＭフォント
	BMPOAM_SYS_PTR	fntoam;
	BMPOAM_ACT_PTR	foact;
	GFL_BMP_DATA * fobmp;

	s8	pokePos;
	s8	pokeChg;
	s8	page;
	u8	pageMax;
	u8	pokeSwap;

	s8	pokeMove;
	u8	pokeMoveCnt;

	u32	buttonID;		// ボタンＯＢＪＩＤ
	int	buttonSeq;	// ボタンアニメ用シーケンス

	int	mainSeq;		// メインシーケンス
	int	subSeq;			// サブシーケンス
	int	nextSeq;		// 次のシーケンス

}DPCMAIN_WORK;

typedef int (*pDENDOUPC_FUNC)(DPCMAIN_WORK*);


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数設定
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_InitVBlank( DPCMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK関数削除
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_ExitVBlank( DPCMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VRAM設定
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_InitVram(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		VRAM設定取得
 *
 * @param		none
 *
 * @return	VRAM設定データ
 */
//--------------------------------------------------------------------------------------------
extern const GFL_DISP_VRAM * DPCMAIN_GetVramBankData(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		BG初期化
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_InitBg(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		BG解放
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_ExitBg(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGグラフィック読み込み
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_LoadBgGraphic(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェード初期化
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_InitPaletteFade( DPCMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェード解放
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_ExitPaletteFade( DPCMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェードリクエスト
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_RequestPaletteFade( DPCMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		パレットフェードチェック
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL DPCMAIN_CheckPaletteFade( DPCMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		アルファブレンド設定
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_SetBlendAlpha(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連初期化
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_InitMsg( DPCMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief	  メッセージ関連解放
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_ExitMsg( DPCMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief	  ポケモンデータ作成
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_CreatePokeData( DPCMAIN_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief	  ポケモンデータ解放
 *
 * @param		wk		殿堂入りＰＣ画面ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void DPCMAIN_ExitPokeData( DPCMAIN_WORK * wk );
