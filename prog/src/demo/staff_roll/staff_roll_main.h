//============================================================================================
/**
 * @file		staff_roll_main.h
 * @brief		エンディング・スタッフロール画面
 * @author	Hiroyuki Nakamura
 * @date		10.04.19
 *
 *	モジュール名：SRMAIN
 */
//============================================================================================
#pragma	once

#include "print/printsys.h"
#include "print/wordset.h"
#include "demo/staff_roll.h"


//============================================================================================
//============================================================================================

// フォントタイプ
enum {
	SRMAIN_FONT_NORMAL = 0,			// 通常
	SRMAIN_FONT_SMALL,					// 小さいフォント
	SRMAIN_FONT_MAX							// フォント最大数
};

// リストデータ
typedef struct {
	u32	msgIdx;

	u16	wait;
	u8	label;
	u8	labelType;

	u8	color;
	u8	font;
	u16	putMode;

	s16	px;
	s16	offs_x;
}ITEMLIST_DATA;

typedef struct {
	STAFFROLL_DATA * dat;			// 外部設定データ

	GFL_TCB * vtask;		// TCB ( VBLANK )

/*
	// OBJ
	GFL_CLUNIT * clunit;
	GFL_CLWK * clwk;
	u32	chrRes;
	u32	palRes;
	u32	celRes;
*/

	GFL_FONT * font[SRMAIN_FONT_MAX];		// フォント
	GFL_MSGDATA * mman;									// メッセージデータマネージャ
	WORDSET * wset;											// 単語セット
	STRBUF * exp;												// メッセージ展開領域
//	PRINT_QUE * que;										// プリントキュー

	PRINT_UTIL	util[2];			// BMPWIN
//	u8	vBmp[0x20*32*32*2];		// 仮想ＢＭＰ
	u8	bmpTransFlag;
//	u8	bmpShitfFlag;

	u8	labelType;
	u8	labelSeq;

	u8	putFrame;

	s8	britness;

	u8	skipCount;
	u8	skipFlag;

	ITEMLIST_DATA * list;
	u16	listPos;
	s16	listWait;
	fx32	listScrollCnt;
	BOOL	listScrollFlg;


	int	mainSeq;
	int	subSeq;
	int	nextSeq;

#ifdef	PM_DEBUG
	BOOL	debugStopFlg;
#endif

}SRMAIN_WORK;

typedef int (*pSRMAIN_FUNC)(SRMAIN_WORK*);


//============================================================================================
//============================================================================================

extern BOOL SRMAIN_Main( SRMAIN_WORK * wk );
