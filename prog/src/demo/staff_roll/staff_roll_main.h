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
#define	SRMAIN_DRAW_3D		// 定義を有効にすると３Ｄが有効になります

#define	SRMAIN_CAMERA_REQ_MAX		( 16 )

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

/*
typedef struct {
	const SR3DCAMERA_PARAM * tbl;		// 座標テーブル
	SR3DCAMERA_PARAM	param;				// 移動先
	SR3DCAMERA_PARAM	val;					// 移動値

	u16	tblMax;		// テーブル数
	u16	pos;

	u16	cnt;
	u16	cntMax;

	BOOL	flg;
}SR3DCAMERA_MOVE;
*/

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

#ifdef	SRMAIN_DRAW_3D
	GFL_G3D_UTIL * g3d_util;
//	GFL_G3D_SCENE * g3d_scene;
	GFL_G3D_CAMERA * g3d_camera;
	GFL_G3D_LIGHTSET * g3d_light;
	u32	g3d_obj_id;
	u16	g3d_unit;

	SR3DCAMERA_MOVE	cameraMove;

#endif	// SRMAIN_DRAW_3D

	GFL_FONT * font[SRMAIN_FONT_MAX];		// フォント
	GFL_MSGDATA * mman;									// メッセージデータマネージャ
//	WORDSET * wset;											// 単語セット
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
