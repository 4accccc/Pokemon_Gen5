
//=============================================================================================
/**
 * @file	fade.h
 * @brief	画面フェードシステム
 * @author	Hisashi Sogabe
 * @date	2007/01/18
 */
//=============================================================================================

#ifndef _FADE_H_
#define _FADE_H_

#ifdef __cplusplus
extern "C" {
#endif

//=========================================================================
//	定数定義
//=========================================================================
//フェードモード指定
#define	GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN	(0x0001)		//マスター輝度ブラックアウト（メイン画面）
#define	GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB		(0x0002)		//マスター輝度ブラックアウト（サブ画面）
#define	GFL_FADE_MASTER_BRIGHT_BLACKOUT		    (0x0003)		//マスター輝度ブラックアウト（両画面）
#define	GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN	(0x0004)		//マスター輝度ホワイトアウト（メイン画面）
#define	GFL_FADE_MASTER_BRIGHT_WHITEOUT_SUB		(0x0008)		//マスター輝度ホワイトアウト（サブ画面）
#define	GFL_FADE_MASTER_BRIGHT_WHITEOUT		    (0x000C)		//マスター輝度ホワイトアウト（両画面）
#define	GFL_FADE_SOFT_CALC_FADE					(0x0010)		//プログラム計算によるカラー加減算（未実装）

#define	GFL_FADE_MASTER_BRIGHT	(GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN | \
								 GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB | \
								 GFL_FADE_MASTER_BRIGHT_WHITEOUT_MAIN | \
								 GFL_FADE_MASTER_BRIGHT_WHITEOUT_SUB)

//--------------------------------------------------------------------------------------------
/**
 * フェードシステム初期化
 *
 * @param	heapID	ヒープＩＤ
 *
 * @return	取得したメモリのアドレス
 */
//--------------------------------------------------------------------------------------------
extern	void	GFL_FADE_Init( u32 heapID );

//--------------------------------------------------------------------------------------------
/**
 * フェードシステムメイン
 */
//--------------------------------------------------------------------------------------------
extern	void	GFL_FADE_Main( void );

//--------------------------------------------------------------------------------------------
/**
 * フェードシステム終了
 */
//--------------------------------------------------------------------------------------------
extern	void	GFL_FADE_Exit( void );

//--------------------------------------------------------------------------------------------
/**
 * マスター輝度リクエスト
 *
 * @param	mode		フェードモード（GFL_FADE_MASTER_BRIGHT_BLACKOUT or GFL_FADE_MASTER_BRIGHT_WHITEOUT）
 * @param	start_evy	スタート輝度（0～16）
 * @param	end_evy		エンド輝度（0～16）
 * @param	wait		フェードスピード
 */
//--------------------------------------------------------------------------------------------
extern	void	GFL_FADE_SetMasterBrightReq( int mode, int start_evy, int end_evy, int wait );

//--------------------------------------------------------------------------------------------
/**
 * フェード実行中のチェック
 *
 * @reval	TRUE:実行中 FALSE:未実行
 */
//--------------------------------------------------------------------------------------------
extern	BOOL	GFL_FADE_CheckFade( void );

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif //_FADE_H_
