//============================================================================================
/**
 * @file		cdemo_main.h
 * @brief		コマンドデモ画面 メイン処理
 * @author	Hiroyuki Nakamura
 * @date		09.01.21
 */
//============================================================================================
/*
#ifndef LD_MAIN_H
#define	LD_MAIN_H

#include "demo/legend_demo.h"
//#include "../../field/motion_bl.h"
*/

#pragma	once

#include "demo/command_demo.h"

//============================================================================================
//	定数定義
//============================================================================================


/*
// アニメテーブルインデックス
enum {
	LD_SCENE_ANM_ICA = 0,
	LD_SCENE_ANM_IMA,
	LD_SCENE_ANM_ITA,
	LD_SCENE_ANM_ITP,
	LD_SCENE_ANM_MAX,
};

// シーンデータ
typedef struct {
	NNSG3dRenderObj	rendObj;
	NNSG3dResMdl * mdl;
	NNSG3dResFileHeader * rh;
	void * anmMem[LD_SCENE_ANM_MAX];
	NNSG3dAnmObj * anmObj[LD_SCENE_ANM_MAX];
}LD_SCENE_DATA;

// モーションブラー初期化パラメータ
typedef struct {
	// グラフィックモード
	GXDispMode dispMode;	// ディスプレイモード
	GXBGMode bgMode;		// BGモード	
	GXBG0As bg0_2d3d;		// BG0を3dに使用するか

	// キャプチャ
	GXCaptureSize sz;		// キャプチャサイズ
    GXCaptureMode mode;		// キャプチャモード
    GXCaptureSrcA a;		// キャプチャ先A
    GXCaptureSrcB b;		// キャプチャ先B
    GXCaptureDest dest;		// キャプチャデータ転送Vram
    int eva;				// ブレンド係数A
    int evb;				// ブレンド係数B

	int heap_id;			// 使用するヒープID
}LDMAIN_MBL_PARAM;

// モーションブラータスクワーク
typedef struct {
	GXVRamLCDC			lcdc;		// 元のLCDC
	LDMAIN_MBL_PARAM	data;		// モーションブラー初期化パラメータ
	BOOL				init_flg;
	TCB_PTR				tcb;	
}LDMAIN_MBL_WORK;

// 伝説デモワーク
typedef struct {
	LEGEND_DEMO_DATA * dat;

	LDMAIN_MBL_WORK * mb;

	NNSFndAllocator	allocator;
	GF_CAMERA_PTR	camera;		// カメラ
	LD_SCENE_DATA	scene[8];	// シーンデータ
	u32	scene_max;				// シーンデータ数

	VecFx32 target;

	int	main_seq;
	int	next_seq;

	u32	cnt;		// カウンタ

	void * work;	// 各アプリで使用するワーク（確保・解放は各アプリで）

}LEGEND_DEMO_WORK;

typedef int (*pLegendDemoFunc)(LEGEND_DEMO_WORK*);

// リソースデータテーブル
typedef struct {
	u32	imd;
	u8	anm[4];
	BOOL	glstFlag;
}LD_RES_TBL;

// カメラデータ
typedef struct {
	GF_CAMERA_PARAM	prm;
	VecFx32	target;
	fx32	near;
	fx32	far;
}LD_CAMERA_DATA;

#define	LD_RES_DUMMY	( 0xff )		// リソースダミーデータ
*/

typedef struct {
	u16	buff[256*16];
	u32	size;
}CDEMO_PALETTE;

typedef struct {
	COMMANDDEMO_DATA * dat;

	ARCHANDLE * gra_ah;

	CDEMO_PALETTE	pltt[3];

	GFL_TCB * vtask;					// TCB ( VBLANK )

	int * commSrc;
	int * commPos;

	u32	cnt;		// カウンタ

	// アルファブレンド
	int	alpha_plane1;		// 対象面１
	int	alpha_ev1;			// 対象面１のブレンド係数
	int	alpha_plane2;		// 対象面２
	int	alpha_ev2;			// 対象面２のブレンド係数
	int	alpha_end_ev;		// 終了ブレンド係数
	int	alpha_mv_frm;		// 終了までのフレーム数

	// ＢＧスクリーン切り替えアニメ
	int	bgsa_chr;
	int	bgsa_pal;
	int	bgsa_scr;
	u16	bgsa_max;
	u16	bgsa_num;
	u8	bgsa_wait;
	u8	bgsa_cnt;
	u16	bgsa_load;
	u16	bgsa_seq;

	int	main_seq;
	int	next_seq;

	BOOL	init_flg;			// 初期化済みフラグ

#ifdef PM_DEBUG
	u32	debug_count_frm;
	u32	debug_count;

	OSTick	stick;
	OSTick	etick;

#endif	// PM_DEBUG

}CDEMO_WORK;

typedef int (*pCommDemoFunc)(CDEMO_WORK*);


//============================================================================================
//	プロトタイプ宣言
//============================================================================================

extern void CDEMOMAIN_VramBankSet(void);
extern void CDEMOMAIN_BgInit( CDEMO_WORK * wk );
extern void CDEMOMAIN_BgExit(void);
extern void CDEMOMAIN_CommDataLoad( CDEMO_WORK * wk );
extern void CDEMOMAIN_CommDataDelete( CDEMO_WORK * wk );
extern void CDEMOMAIN_InitVBlank( CDEMO_WORK * wk );
extern void CDEMOMAIN_ExitVBlank( CDEMO_WORK * wk );

extern void CDEMOMAIN_FadeInSet( CDEMO_WORK * wk, int div, int sync, int next );
extern void CDEMOMAIN_FadeOutSet( CDEMO_WORK * wk, int div, int sync, int next );
extern void CDEMOMAIN_WhiteInSet( CDEMO_WORK * wk, int div, int sync, int next );
extern void CDEMOMAIN_WhiteOutSet( CDEMO_WORK * wk, int div, int sync, int next );

extern void CDEMOMAIN_LoadPaletteRequest( CDEMO_WORK * wk, u32 frm, u32 id );

extern void CDEMOMAIN_LoadBgGraphic( CDEMO_WORK * wk, u32 chr, u32 pal, u32 scrn, u32 frm );
























#if 0
//--------------------------------------------------------------------------------------------
/**
 * 共通初期化
 *
 * @param	wk		伝説デモワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_CommInit( LEGEND_DEMO_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * 共通解放処理
 *
 * @param	wk		伝説デモワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_CommRelease( LEGEND_DEMO_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * フェードインセット
 *
 * @param	wk		伝説デモワーク
 * @param	div		分割数
 * @param	sync	分割されたフレームのウェイト数
 * @param	next	フェード後のシーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_FadeInSet( LEGEND_DEMO_WORK * wk, int div, int sync, int next );

//--------------------------------------------------------------------------------------------
/**
 * フェードアウトセット
 *
 * @param	wk		伝説デモワーク
 * @param	div		分割数
 * @param	sync	分割されたフレームのウェイト数
 * @param	next	フェード後のシーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_FadeOutSet( LEGEND_DEMO_WORK * wk, int div, int sync, int next );

//--------------------------------------------------------------------------------------------
/**
 * ホワイトインセット
 *
 * @param	wk		伝説デモワーク
 * @param	div		分割数
 * @param	sync	分割されたフレームのウェイト数
 * @param	next	フェード後のシーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_WhiteInSet( LEGEND_DEMO_WORK * wk, int div, int sync, int next );

//--------------------------------------------------------------------------------------------
/**
 * ホワイトアウトセット
 *
 * @param	wk		伝説デモワーク
 * @param	div		分割数
 * @param	sync	分割されたフレームのウェイト数
 * @param	next	フェード後のシーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_WhiteOutSet( LEGEND_DEMO_WORK * wk, int div, int sync, int next );

//--------------------------------------------------------------------------------------------
/**
 * カメラ設定
 *
 * @param	wk		伝説デモワーク
 * @param	dat		カメラデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_InitCamera( LEGEND_DEMO_WORK * wk, const LD_CAMERA_DATA * dat );

//--------------------------------------------------------------------------------------------
/**
 * ライト初期化
 *
 * @param	none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_InitLight(void);

//--------------------------------------------------------------------------------------------
/**
 * ライト設定
 *
 * @param	gst		ライトデータ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_SetLight( GLST_DATA * gst );

//--------------------------------------------------------------------------------------------
/**
 * エッジマーキング設定
 *
 * @param	rgb		カラーテーブル
 *
 * @return	none
 *
 * @li	rgb = NULL でエッジマーキング無効
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_EdgeMarkingSet( const GXRgb * rgb );

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄモデルデータ読み込み
 *
 * @param	wk			伝説デモワーク
 * @param	arcIndex	アークインデックス
 * @param	tbl			シーンリソースデータテーブル
 * @param	max			シーンリソースデータ数
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_Load3DData( LEGEND_DEMO_WORK * wk, u32 arcIndex, const LD_RES_TBL * tbl, u32 max );

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄモデルデータ削除
 *
 * @param	wk			伝説デモワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_Delete3DData( LEGEND_DEMO_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄモデル表示
 *
 * @param	wk			伝説デモワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_Draw3DMain( LEGEND_DEMO_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄモデルアニメチェック（個別）
 *
 * @param	wk		伝説デモワーク
 * @param	id		モデルＩＤ
 *
 * @retval	"TRUE = アニメ中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL LDMAIN_ObjAnmCheck( LEGEND_DEMO_WORK * wk, u32 id );

//--------------------------------------------------------------------------------------------
/**
 * ３Ｄモデルアニメチェック（全体）
 *
 * @param	wk		伝説デモワーク
 *
 * @retval	"TRUE = アニメ中"
 * @retval	"FALSE = それ以外"
 */
//--------------------------------------------------------------------------------------------
extern BOOL LDMAIN_AllObjAnmCheck( LEGEND_DEMO_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * モーションブラー初期化
 *
 * @param	eva		ブレンド係数A
 * @param	evb		ブレンド係数B
 *
 * @return	FLD_MOTION_BL_PTR	モーションブラーポインタ
 */
//--------------------------------------------------------------------------------------------
extern LDMAIN_MBL_WORK * LDMAIN_MotionBlInit( int eva, int evb );

//--------------------------------------------------------------------------------------------
/**
 * モーションブラー削除
 *
 * @param	mbl		モーションブラーポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void LDMAIN_MotionBlExit( LDMAIN_MBL_WORK * mb );

#endif
