//============================================================================================
/**
 * @file		cdemo_main.h
 * @brief		コマンドデモ画面 メイン処理
 * @author	Hiroyuki Nakamura
 * @date		09.01.21
 */
//============================================================================================
#pragma	once

#include "demo/command_demo.h"

//============================================================================================
//	定数定義
//============================================================================================

// [ HEAPID_COMMAND_DEMO ] 後方確保用定義
#define	HEAPID_COMMAND_DEMO_L		( GFL_HEAP_LOWID(HEAPID_COMMAND_DEMO) )

// パレットデータ
typedef struct {
	u16	buff[256*16];
	u32	size;
}CDEMO_PALETTE;

// ワーク
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
//	u16	frmMax;
	u16	bgsa_num;
	u8	bgsa_load;
	u8	bgsa_seq;
	OSTick	bfTick;
	OSTick	stTick;
//	u8	bgsa_wait;
//	u8	bgsa_cnt;
//	int	bgsa_chr;
//	int	bgsa_pal;
//	int	bgsa_scr;
//	OSTick	dfTick;
//	OSTick	afTick;
//	s64	stTick;

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

//--------------------------------------------------------------------------------------------
/**
 * @brief		VRAM設定
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void CDEMOMAIN_VramBankSet(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		BG設定
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void CDEMOMAIN_BgInit( CDEMO_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		BG削除
 *
 * @param		none
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void CDEMOMAIN_BgExit(void);

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンドデータ読み込み
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void CDEMOMAIN_CommDataLoad( CDEMO_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		コマンドデータ削除
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void CDEMOMAIN_CommDataDelete( CDEMO_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK設定
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void CDEMOMAIN_InitVBlank( CDEMO_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		VBLANK削除
 *
 * @param		wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void CDEMOMAIN_ExitVBlank( CDEMO_WORK * wk );

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードインセット
 *
 * @param		wk		ワーク
 * @param		div		分割数
 * @param		sync	分割されたフレームのウェイト数
 * @param		next	フェード後のシーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void CDEMOMAIN_FadeInSet( CDEMO_WORK * wk, int div, int sync, int next );

//--------------------------------------------------------------------------------------------
/**
 * @brief		フェードアウトセット
 *
 * @param		wk		ワーク
 * @param		div		分割数
 * @param		sync	分割されたフレームのウェイト数
 * @param		next	フェード後のシーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void CDEMOMAIN_FadeOutSet( CDEMO_WORK * wk, int div, int sync, int next );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ホワイトインセット
 *
 * @param		wk		ワーク
 * @param		div		分割数
 * @param		sync	分割されたフレームのウェイト数
 * @param		next	フェード後のシーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void CDEMOMAIN_WhiteInSet( CDEMO_WORK * wk, int div, int sync, int next );

//--------------------------------------------------------------------------------------------
/**
 * @brief		ホワイトアウトセット
 *
 * @param		wk		ワーク
 * @param		div		分割数
 * @param		sync	分割されたフレームのウェイト数
 * @param		next	フェード後のシーケンス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void CDEMOMAIN_WhiteOutSet( CDEMO_WORK * wk, int div, int sync, int next );

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGパレット転送リクエスト
 *
 * @param		wk		ワーク
 * @param		frm		BGフレーム
 * @param		id		アークインデックス
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void CDEMOMAIN_LoadPaletteRequest( CDEMO_WORK * wk, u32 frm, u32 id );

//--------------------------------------------------------------------------------------------
/**
 * @brief		BGグラフィック読み込み
 *
 * @param		wk		ワーク
 * @param		chr		キャラアークインデックス
 * @param		pal		パレットアークインデックス
 * @param		scrn	スクリーンアークインデックス
 * @param		frm		BGフレーム
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void CDEMOMAIN_LoadBgGraphic( CDEMO_WORK * wk, u32 chr, u32 pal, u32 scrn, u32 frm );
