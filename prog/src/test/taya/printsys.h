//=============================================================================================
/**
 * @file	printsys.c
 * @brief	Bitmap利用文字描画　システム制御
 * @author	taya
 *
 * @date	2007.02.06	作成
 * @date	2008.09.11	通信対応キュー構造を作成
 */
//=============================================================================================
#ifndef __PRINTSYS_H__
#define __PRINTSYS_H__

#include "gf_font.h"



//======================================================================================
// Typedefs
//======================================================================================

//--------------------------------------------------------------
/**
 *	Print Queue
 *	通信時に描画処理を小刻みに実行できるよう、処理内容を蓄積するための機構。
 */
//--------------------------------------------------------------
typedef struct _PRINT_QUE	PRINT_QUE;

//--------------------------------------------------------------------------
/**
 *	Print Stream
 *	会話ウィンドウ等、１定間隔で１文字ずつ描画，表示を行うための機構
 */
//--------------------------------------------------------------------------
typedef struct _PRINT_STREAM	PRINT_STREAM;

//--------------------------------------------------------------------------
/**
 *	Print Stream Callback
 *	Print Stream で１文字描画ごとに呼び出されるコールバック関数の型
 */
//--------------------------------------------------------------------------
typedef void (*pPrintCallBack)(u32);


//======================================================================================
// Consts
//======================================================================================

//--------------------------------------------------------------------------
/**
 *	Print Stream の状態を識別
 */
//--------------------------------------------------------------------------
typedef enum {
	PRINTSTREAM_STATE_RUNNING = 0,	///< 処理実行中（文字列が流れている）
	PRINTSTREAM_STATE_PAUSE,		///< 停止中（ページ切り替え待ち等）
	PRINTSTREAM_STATE_DONE,			///< 文字列終端まで表示完了
}PRINTSTREAM_STATE;

//--------------------------------------------------------------------------
/**
 *	Print Stream の一時停止状態を識別
 */
//--------------------------------------------------------------------------
typedef enum {
	PRINTSTREAM_PAUSE_LINEFEED = 0,	///< ページ切り替え待ち（改行）
	PRINTSTREAM_PAUSE_CLEAR,		/// <ページ切り替え待ち（表示クリア）
}PRINTSTREAM_PAUSE_TYPE;



//==============================================================================================
/**
 * システム初期化（プログラム起動時に１度だけ呼び出す）
 *
 * @param   heapID		
 *
 */
//==============================================================================================
extern void PRINTSYS_Init( HEAPID heapID );

//==============================================================================================
/**
 * プリントキューをデフォルトのバッファサイズで生成する。
 *
 * @param   heapID			生成用ヒープID
 *
 * @retval  PRINT_QUE*		生成されたプリントキュー
 */
//==============================================================================================
extern PRINT_QUE* PRINTSYS_QUE_Create( HEAPID heapID );

//==============================================================================================
/**
 * プリントキューをサイズ指定して生成する
 *
 * @param   size			バッファサイズ
 * @param   heapID			生成用ヒープID
 *
 * @retval  PRINT_QUE*		生成されたプリントキュー
 */
//==============================================================================================
extern PRINT_QUE* PRINTSYS_QUE_CreateEx( u16 buf_size, HEAPID heapID );

//==============================================================================================
/**
 * プリントキューメイン処理（メインループ中で１回呼び出す）
 *
 * @param   que		プリントキュー
 *
 * @retval  BOOL	キュー処理が残っていない場合はTRUE／にまだ処理が残っている場合はFALSE
 */
//==============================================================================================
extern BOOL PRINTSYS_QUE_Main( PRINT_QUE* que );

//==============================================================================================
/**
 * プリントキューに処理が残っているか判定
 *
 * @param   que		プリントキュー
 *
 * @retval  BOOL	キュー処理が残っていない場合はTRUE／にまだ処理が残っている場合はFALSE
 */
//==============================================================================================
extern BOOL PRINTSYS_QUE_IsFinished( const PRINT_QUE* que );

//==============================================================================================
/**
 * プリントキューに、特定Bitmapを出力対象とした処理が残っているか判定
 *
 * @param   que				プリントキュー
 * @param   targetBmp		出力対象のBitmap
 *
 * @retval  BOOL			targetBmpに対する処理が残っている場合はTRUE／それ以外はFALSE
 */
//==============================================================================================
extern BOOL PRINTSYS_QUE_IsExistTarget( const PRINT_QUE* que, const GFL_BMP_DATA* targetBmp );


//==============================================================================================
/**
 * BITMAPに対する文字列描画
 *
 * @param   que		[out] 描画処理内容を記録するためのプリントキュー
 * @param   dst		[out] 描画先Bitmap
 * @param   xpos	[in]  描画開始Ｘ座標（ドット）
 * @param   ypos	[in]  描画開始Ｙ座標（ドット）
 * @param   str		[in]  文字列
 * @param   font	[in]  フォント
 *
 */
//==============================================================================================
extern void PRINTSYS_Print( PRINT_QUE* que, GFL_BMP_DATA* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font );




extern PRINT_STREAM_HANDLE PRINTSYS_PrintStream(
		GFL_BMPWIN* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font,
		u16 wait, GFL_TCBLSYS* tcbsys, u32 tcbpri, u16 heapID, u16 clearColor, pPrintCallBack callback );

extern PRINTSTREAM_STATE PRINTSYS_PrintStreamGetState( PRINT_STREAM_HANDLE handle );

extern PRINTSTREAM_PAUSE_TYPE PRINTSYS_PrintStreamGetPauseType( PRINT_STREAM_HANDLE handle );
extern void PRINTSYS_PrintStreamReleasePause( PRINT_STREAM_HANDLE handle );
extern void PRINTSYS_PrintStreamDelete( PRINT_STREAM_HANDLE handle );
extern void PRINTSYS_PrintStreamShortWait( PRINT_STREAM_HANDLE handle, u16 wait );
extern u32 PRINTSYS_GetStrWidth( const STRBUF* str, GFL_FONT* font, u16 margin );



//--------------------------------------------------------------
/**
 * プリントキュー用ユーティリティ
 */
//--------------------------------------------------------------

typedef struct {
	GFL_BMPWIN*		win;
	u8				transReq;
}PRINT_UTIL;


inline void PRINT_UTIL_Setup( PRINT_UTIL* wk, GFL_BMPWIN* win )
{
	wk->win = win;
	wk->transReq = FALSE;
}

inline void PRINT_UTIL_Print( PRINT_UTIL* wk, PRINT_QUE* que, u16 xpos, u16 ypos, const STRBUF* buf, GFL_FONT* font )
{
	PRINTSYS_Print( que, GFL_BMPWIN_GetBmp(wk->win), xpos, ypos, buf, font );
	wk->transReq = TRUE;
}


inline BOOL PRINT_UTIL_Trans( PRINT_UTIL* wk, PRINT_QUE* que )
{
	if( wk->transReq )
	{
		if( !PRINTSYS_QUE_IsExistTarget(que, GFL_BMPWIN_GetBmp(wk->win)) )
		{
			GFL_BMPWIN_TransVramCharacter( wk->win );
			wk->transReq = FALSE;
		}
	}
	return wk->transReq;
}






#endif
