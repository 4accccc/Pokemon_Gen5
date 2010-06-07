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

#define	SRMAIN_CAMERA_REQ_MAX		( 16 )		// ３Ｄカメラ動作リクエスト最大数
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

// カメラ動作データ
typedef struct {
	VecFx32	pos;
	VecFx32	target;
}SR3DCAMERA_PARAM;

// カメラリクエストデータ
typedef struct {
	u16	no;
	u16	cnt;
}SR3DCAMERA_REQ_PARAM;

typedef struct {
	const SR3DCAMERA_PARAM * tbl;		// 座標テーブル
	SR3DCAMERA_PARAM	param;				// 移動先
	SR3DCAMERA_PARAM	val;					// 移動値

	SR3DCAMERA_REQ_PARAM	req[SRMAIN_CAMERA_REQ_MAX];	// リクエスト

	u16	tblMax;
	u16	pos;

	u16	cnt;
	u16	cntMax;

	BOOL	flg;
}SR3DCAMERA_MOVE;

typedef struct {
	GFL_BMP_DATA * bmp;
	u32	idx;
}SRVBMP_DATA;

typedef struct {
	STAFFROLL_DATA * dat;			// 外部設定データ

	GFL_TCB * vtask;		// TCB ( VBLANK )

	GFL_G3D_UTIL * g3d_util;
	GFL_G3D_CAMERA * g3d_camera;
	GFL_G3D_LIGHTSET * g3d_light;
	u32	g3d_obj_id;
	u16	g3d_unit;
	fx32	g3d_briCount;

	GFL_G3D_OBJ * g3d_obj;
	int	g3d_anm;

	SR3DCAMERA_MOVE	cameraMove;

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
