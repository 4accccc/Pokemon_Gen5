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
//	定数定義
//============================================================================================

#define	SRMAIN_VBMP_MAX					( 64 )		// 仮想ＢＭＰ最大数

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
	GFL_BMP_DATA * bmp;
	u32	idx;
}SRVBMP_DATA;

//-------------------------------------
///	2Dカットの構造体
//=====================================
typedef struct {

  u8      seq;        // シーケンス
  u8      disp_type;  // 画面選択

  u8      cut_no;     // 何枚目のカットか

  int     alpha_cnt;  // アルファカウント
  int     alpha_fade_sync;  // アルファ
  int     wait;       // 表示待ち

} CUT_WORK;

typedef struct {
	STAFFROLL_DATA * dat;			// 外部設定データ

	GFL_TCB * vtask;		// TCB ( VBLANK )

	GFL_FONT * font[SRMAIN_FONT_MAX];		// フォント
	GFL_MSGDATA * mman;									// メッセージデータマネージャ
	STRBUF * exp;												// メッセージ展開領域

	SRVBMP_DATA	vBmp[SRMAIN_VBMP_MAX];
	u16	vBmpMax;
	u8	vBmpCnt;
	u8	vBmpPut;

	PRINT_UTIL	util[2];			// BMPWIN
	u8	bmpTransFlag;

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

	u32	wait;

	int	mainSeq;
	int	subSeq;
	int	nextSeq;

  // 2Dカットのワーク
  CUT_WORK    cut;

#ifdef	PM_DEBUG
	BOOL	debugStopFlg;

	int	testStartInitWait;
	int	testStartEndWait;
	int	testLogoWait;

#endif

}SRMAIN_WORK;

typedef int (*pSRMAIN_FUNC)(SRMAIN_WORK*);


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

//--------------------------------------------------------------------------------------------
/**
 * @brief		メインシーケンス
 *
 * @param		wk		ワーク
 *
 * @retval	"TRUE = 処理中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL SRMAIN_Main( SRMAIN_WORK * wk );
