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
#include <gflib.h>
#include <tcbl.h>

#include "print/printsys.h"

//==============================================================
// Consts
//==============================================================
enum {
	LETTER_CHAR_WIDTH = 2,			///< １文字あたり横キャラ数
	LETTER_CHAR_HEIGHT = 2,			///< １文字あたり縦キャラ数
	LINE_DOT_HEIGHT = 16,			///< １行あたりのドット数

	// 改行コード、終端コード
	EOM_CODE			= 0xffff,
	CR_CODE				= 0xfffe,

	// 汎用コントロールタイプ
	CTRL_GENERAL_COLOR			= (0x0000),	///< 色変更
	CTRL_GENERAL_RESET_COLOR	= (0x0001),	///< 色変更
	CTRL_GENERAL_X_RIGHTFIT		= (0x0002),	///< Ｘ座標右寄せ
	CTRL_GENERAL_X_CENTERING	= (0x0003),	///< Ｘ座標センタリング

	// 流れるメッセージ中のみ有効なコントロールタイプ
	CTRL_STREAM_LINE_FEED		= (0x0000),	///< 改ページ（行送り待ち）
	CTRL_STREAM_PAGE_CLEAR		= (0x0001),	///< 改ページ（描画クリア待ち）
	CTRL_STREAM_CHANGE_WAIT		= (0x0002),	///< 描画ウェイト変更（１回）
	CTRL_STREAM_SET_WAIT		= (0x0003),	///< 描画ウェイト変更（永続）
	CTRL_STREAM_RESET_WAIT		= (0x0004),	///< 描画ウェイトをデフォルトに戻す
	CTRL_STREAM_CHANGE_ARGV		= (0x0005),	///< コールバック引数を変更（１回）
	CTRL_STREAM_SET_ARGV		= (0x0006),	///< コールバック引数を変更（永続）
	CTRL_STREAM_RESET_ARGV		= (0x0007),	///< コールバック引数をデフォルトに戻す
	CTRL_STREAM_FORCE_CLEAR		= (0x0008),	///< 強制描画クリア

	// システムコントロール
	CTRL_SYSTEM_COLOR			= (0x0000),

	// タグ開始コード
	SPCODE_TAG_START_	= 0xf000,
	TAGTYPE_WORD = 0,				///< 文字列挿入
	TAGTYPE_NUMBER = 1,				///< 数値挿入
	TAGTYPE_GENERAL_CTRL = 0xbd,	///< 汎用コントロール処理
	TAGTYPE_STREAM_CTRL = 0xbe,		///< 流れるメッセージ用コントロール処理
	TAGTYPE_SYSTEM = 0xff,			///< エディタシステムタグ用コントロール処理


	// プリントキュー関連定数
	QUE_DEFAULT_BUFSIZE = 1024,
	QUE_DEFAULT_TICK = 2200,		///< 通信時、１度の描画処理に使う時間の目安（Tick）デフォルト値

};


//==============================================================
// Typedefs
//==============================================================

// 文字コード
typedef u16		STRCODE;


//--------------------------------------------------------------------------
/**
 *	Print Job
 *
 *  Bitmapへの描画処理ワーク
 */
//--------------------------------------------------------------------------
		typedef struct {
	u16   org_x;		///< 書き込み開始Ｘ座標
	u16   org_y;		///< 書き込み開始Ｙ座標
	u16   write_x;		///< 書き込み中のＸ座標
	u16   write_y;		///< 書き込み中のＹ座標

	u8    colorLetter;		///< 描画色（文字）
	u8    colorShadow;		///< 描画色（影）
	u8    colorBackGround;	///< 描画色（背景）

	GFL_FONT*		fontHandle;	///< フォントハンドル
	GFL_BMP_DATA*	dst;		///< 書き込み先 Bitmap

}PRINT_JOB;

//--------------------------------------------------------------------------
/**
 *	Print Queue
 *
 *	通信時、描画処理を小刻みに実行できるよう、処理内容を蓄積するための機構。
 */
//--------------------------------------------------------------------------
struct _PRINT_QUE {

	PRINT_JOB*		runningJob;
	const STRCODE*	sp;

	OSTick	limitPerFrame;

	u16		bufTopPos;
	u16		bufEndPos;
	u16		bufSize;
	u16		dmy;

	u8   buf[0];
};

//--------------------------------------------------------------------------
/**
 *	Print Stream
 *
 *	会話ウィンドウ等、１定間隔で１文字ずつ描画，表示を行うための機構。
 */
//--------------------------------------------------------------------------
struct _PRINT_STREAM {

	PRINTSTREAM_STATE		state;
	PRINTSTREAM_PAUSE_TYPE	pauseType;

	GFL_BMPWIN*			dstWin;
	GFL_BMP_DATA*		dstBmp;
	GFL_TCBL*			tcb;

	const STRCODE* sp;

	u16		org_wait;
	u16		current_wait;
	u16		wait;
	u8		pauseReleaseFlag;
	u8		pauseWait;
	u8		clearColor;

	pPrintCallBack	callback_func;
	u32				org_arg;
	u32				current_arg;
	u32				arg;

	PRINT_JOB	printJob;



};


//==============================================================================================
//--------------------------------------------------------------------------
/**
 *	SystemWork
 */
//--------------------------------------------------------------------------
static struct {
	GFL_BMP_DATA*		charBuffer;
	PRINT_JOB			printJob;
}SystemWork;


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static inline BOOL IsNetConnecting( void );
static void setupPrintJob( PRINT_JOB* wk, GFL_FONT* font, GFL_BMP_DATA* dst, u16 org_x, u16 org_y );
static const STRCODE* print_next_char( PRINT_JOB* wk, const STRCODE* sp );
static void put_1char( GFL_BMP_DATA* dst, u16 xpos, u16 ypos, GFL_FONT* fontHandle, STRCODE charCode, u16* charWidth, u16* charHeight );
static const STRCODE* ctrlGeneralTag( PRINT_JOB* wk, const STRCODE* sp );
static const STRCODE* ctrlSystemTag( PRINT_JOB* wk, const STRCODE* sp );
static void print_stream_task( GFL_TCBL* tcb, void* wk_adrs );
static void ctrlStreamTag( PRINT_STREAM* wk );
static u32 get_line_width( const STRCODE* sp, GFL_FONT* font, u16 margin, const STRCODE** endPtr );
static inline u16 STR_TOOL_GetTagType( const STRCODE* sp );
static inline u16 STR_TOOL_GetTagParam( const STRCODE* sp, u16 paramIndex );
static inline const STRCODE* STR_TOOL_SkipTag( const STRCODE* sp );
static inline u16 STR_TOOL_GetTagNumber( const STRCODE* sp );
static u32 Que_CalcStringBufferSize( const STRBUF* str );
static u32 Que_CalcUseBufSize( const STRBUF* buf );
static u32 Que_GetFreeSize( const PRINT_QUE* que );
static void Que_StoreNewJob( PRINT_QUE* que, PRINT_JOB* printJob, const STRBUF* str );
static void* Que_WriteDataUnit( PRINT_QUE* que, const u8* src, u16 totalSize, u16 unitSize );
static PRINT_JOB* Que_ReadNextJobAdrs( const PRINT_QUE* que, const PRINT_JOB* job );
static inline u16 Que_WriteData( PRINT_QUE* que, const u8* src, u16 pos, u16 size );
static inline u16 Que_AdrsToBufPos( const PRINT_QUE* que, const void* bufAdrs );
static inline const void* Que_BufPosToAdrs( const PRINT_QUE* que, u16 bufPos );
static inline u16 Que_FwdBufPos( const PRINT_QUE* que, u16 pos, u16 fwdSize );






//==============================================================================================
/**
 * システム初期化（GFLIB初期化後に１度だけ呼ぶ）
 *
 * @param   heapID		初期化用ヒープID
 *
 */
//==============================================================================================
void PRINTSYS_Init( HEAPID heapID )
{
	GFL_STR_SetEOMCode( EOM_CODE );

	// １文字分のBitmap領域のみ確保
	SystemWork.charBuffer = GFL_BMP_Create( LETTER_CHAR_WIDTH, LETTER_CHAR_HEIGHT, GFL_BMP_16_COLOR, heapID );
}



//==============================================================================================
/**
 * プリントキューをデフォルトのバッファサイズで生成する
 *
 * @param   heapID			生成用ヒープID
 *
 * @retval  PRINT_QUE*		生成されたプリントキュー
 */
//==============================================================================================
PRINT_QUE* PRINTSYS_QUE_Create( HEAPID heapID )
{
	return PRINTSYS_QUE_CreateEx( QUE_DEFAULT_BUFSIZE, heapID );
}

//==============================================================================================
/**
 * プリントキューをサイズ指定して生成する
 *
 * @param   size			バッファサイズ
 * @param   heapID			生成用ヒープID
 *
 * @retval  PRINT_QUE*		
 */
//==============================================================================================
PRINT_QUE* PRINTSYS_QUE_CreateEx( u16 buf_size, HEAPID heapID )
{
	PRINT_QUE* que;
	u16 real_size;

	// リングバッファの終端で構造体が分断されないよう、
	// 確保サイズに余裕を持たせておく
	real_size = buf_size + sizeof(PRINT_JOB);
	while( real_size % 4 ){ real_size++; }

	que = GFL_HEAP_AllocMemory( heapID, sizeof(PRINT_QUE) + real_size );

	que->bufTopPos = 0;
	que->bufEndPos = buf_size;
	que->bufSize = buf_size;		// 終端判定は指定のサイズ値で行う
	que->limitPerFrame = QUE_DEFAULT_TICK;
	que->runningJob = NULL;
	que->sp = NULL;

//	GFL_STD_MemClear( que->buf, size );

	return que;
}

//==============================================================================================
/**
 * プリントキューを削除する
 *
 * @param   que		プリントキュー
 *
 */
//==============================================================================================
void PRINTSYS_QUE_Delete( PRINT_QUE* que )
{
	GF_ASSERT(que->runningJob==NULL);
	GFL_HEAP_FreeMemory( que );
}

//==============================================================================================
/**
 * プリントキューに蓄積された文字列描画作業を処理する。
 * 通信中なら一定時間毎に処理を返す。
 * 通信中でなければ一気に処理を終える（…のは未実装）
 *
 * @param   que		プリントキュー
 *
 * @retval  BOOL	処理が終了していればTRUE／それ以外はFALSE
 */
//==============================================================================================
BOOL PRINTSYS_QUE_Main( PRINT_QUE* que )
{
	if( que->runningJob )
	{
		OSTick start, diff;
		u8 endFlag = FALSE;

		start = OS_GetTick();

		while( que->runningJob )
		{
			while( *(que->sp) != EOM_CODE )
			{
				que->sp = print_next_char( que->runningJob, que->sp );
				diff = OS_GetTick() - start;
				if( diff > que->limitPerFrame )
				{
					endFlag = TRUE;
					break;
				}
			}
			//
			if( *(que->sp) == EOM_CODE )
			{
				u16 pos = Que_AdrsToBufPos( que, que->runningJob );

				que->runningJob = Que_ReadNextJobAdrs( que, que->runningJob );
				if( que->runningJob )
				{
					pos = Que_FwdBufPos( que, pos, sizeof(PRINT_JOB) );
					pos = Que_FwdBufPos( que, pos, sizeof(PRINT_JOB*) );
					que->sp = Que_BufPosToAdrs( que, pos );
				}
			}

			if( endFlag ){ break; }

		}

		if( que->runningJob )
		{
			que->bufEndPos = Que_AdrsToBufPos( que, que->sp );
			return FALSE;
		}

		que->bufTopPos = 0;
		que->bufEndPos = que->bufSize;

	}

	return TRUE;
}
//==============================================================================================
/**
 * プリントキューの処理が全て完了しているかチェック
 *
 * @param   que			プリントキュー
 *
 * @retval  BOOL		完了していればTRUE
 */
//==============================================================================================
BOOL PRINTSYS_QUE_IsFinished( const PRINT_QUE* que )
{
	return (que->runningJob == NULL);
}

//==============================================================================================
/**
 * プリントキューに、特定Bitmapが出力対象の描画処理が残っているかチェック
 *
 * @param   que				プリントキュー
 * @param   targetBmp		描画対象Bitmap
 *
 * @retval  BOOL			残っていればTRUE
 */
//==============================================================================================
BOOL PRINTSYS_QUE_IsExistTarget( const PRINT_QUE* que, const GFL_BMP_DATA* targetBmp )
{
	if( que->runningJob )
	{
		const PRINT_JOB* job = que->runningJob;

		while( job )
		{
			if( job->dst == targetBmp )
			{
				return TRUE;
			}
			job = Que_ReadNextJobAdrs( que, job );
		}

	}
	return FALSE;
}

//--------------------------------------------------------------------------
/**
 * 通信中（描画処理を分割する必要がある）かどうか判定
 *
 * @retval  BOOL		通信中ならTRUE
 */
//--------------------------------------------------------------------------
static inline BOOL IsNetConnecting( void )
{
	return (GFL_NET_GetConnectNum() != 0);
}


//==============================================================================================
/**
 * プリントキューを介した文字列描画処理
 *
 * @param   que		[out] プリントキュー
 * @param   dst		[out] 描画先Bitmap
 * @param   xpos	[in]  描画開始Ｘ座標（ドット）
 * @param   ypos	[in]  描画開始Ｙ座標（ドット）
 * @param   str		[in]  文字列
 * @param   font	[in]  フォントタイプ
 *
 */
//==============================================================================================
void PRINTSYS_PrintQue( PRINT_QUE* que, GFL_BMP_DATA* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font )
{
	GF_ASSERT(que);
	GF_ASSERT(dst);
	GF_ASSERT(str);
	GF_ASSERT(font);

	{
		PRINT_JOB* job = &SystemWork.printJob;
		setupPrintJob( job, font, dst, xpos, ypos );

		if( !IsNetConnecting() )
		{
			const STRCODE* sp = GFL_STR_GetStringCodePointer( str );
			while( *sp != EOM_CODE )
			{
				sp = print_next_char( job, sp );
			}
		}
		else
		{
			u32  size = Que_CalcUseBufSize( str );

			if( size <= Que_GetFreeSize( que ) )
			{
				Que_StoreNewJob( que, job, str );
			}
			else
			{
				GF_ASSERT_MSG(0, "[PRINT ACM] not enough buffer ... strsize = %d\n", size);
			}
		}
	}
}

//==============================================================================================
/**
 * Bitmap へ直接の文字列描画
 *
 * @param   dst		[out] 描画先Bitmap
 * @param   xpos	[in]  描画開始Ｘ座標（ドット）
 * @param   ypos	[in]  描画開始Ｙ座標（ドット）
 * @param   str		[in]  文字列
 * @param   font	[in]  フォント
 *
 */
//==============================================================================================
void PRINTSYS_Print( GFL_BMP_DATA* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font )
{
	PRINT_JOB* job = &SystemWork.printJob;
	const STRCODE* sp = GFL_STR_GetStringCodePointer( str );

	setupPrintJob( job, font, dst, xpos, ypos );

	while( *sp != EOM_CODE )
	{
		sp = print_next_char( job, sp );
	}
}


//------------------------------------------------------------------
/**
 * 描画処理ワークの初期化
 *
 * @param   wk			ワークポインタ
 * @param   font_type	フォントタイプ
 * @param   dst			描画先Bitmap
 * @param   org_x		描画開始Ｘ座標
 * @param   org_y		描画開始Ｙ座標
 *
 */
//------------------------------------------------------------------
static void setupPrintJob( PRINT_JOB* wk, GFL_FONT* font, GFL_BMP_DATA* dst, u16 org_x, u16 org_y )
{
	wk->dst = dst;
	wk->fontHandle = font;
	wk->org_x = wk->write_x = org_x;
	wk->org_y = wk->write_y = org_y;
	GFL_FONTSYS_GetColor( &wk->colorLetter, &wk->colorShadow, &wk->colorBackGround );
}
//------------------------------------------------------------------
/**
 * 改行および汎用コントロールがあれば処理し、
 * 通常文字があれば１文字描画して、次の文字のポインタを返す
 *
 * @param   wk		描画ワークポインタ
 * @param   sp		
 *
 * @retval  const STRCODE*		
 */
//------------------------------------------------------------------
static const STRCODE* print_next_char( PRINT_JOB* wk, const STRCODE* sp )
{
	while( 1 )
	{
		switch( *sp ){
		case EOM_CODE:
			return sp;

		case CR_CODE:
			wk->write_x = wk->org_x;
			wk->write_y += LINE_DOT_HEIGHT;
			sp++;
			break;

		case SPCODE_TAG_START_:
			switch( STR_TOOL_GetTagType(sp) ){
			case TAGTYPE_GENERAL_CTRL:
				sp = ctrlGeneralTag( wk, sp );
				break;

			case TAGTYPE_SYSTEM:
				sp = ctrlSystemTag( wk, sp );
				break;

			default:
				sp = STR_TOOL_SkipTag( sp );
				break;
			}
			break;

		default:
			{
				u16 w, h;

				put_1char( wk->dst, wk->write_x, wk->write_y, wk->fontHandle, *sp, &w, &h );
				wk->write_x += (w+1);
				sp++;

				return sp;
			}
			break;
		}
	}
}
//------------------------------------------------------------------
/**
 * Bitmap１文字分描画
 *
 * @param   dst				描画先ビットマップ
 * @param   xpos			描画先Ｘ座標（ドット）
 * @param   ypos			描画先Ｙ座標（ドット）
 * @param   fontHandle		描画フォントハンドル
 * @param   charCode		文字コード
 * @param   charWidth		文字幅取得ワーク
 * @param   charHeight		文字高取得ワーク
 *
 */
//------------------------------------------------------------------
static void put_1char( GFL_BMP_DATA* dst, u16 xpos, u16 ypos, GFL_FONT* fontHandle, STRCODE charCode, u16* charWidth, u16* charHeight )
{
	GFL_FONT_GetBitMap( fontHandle, charCode, GFL_BMP_GetCharacterAdrs(SystemWork.charBuffer), charWidth, charHeight );
	GFL_BMP_Print( SystemWork.charBuffer, dst, 0, 0, xpos, ypos, *charWidth, *charHeight, 0x0f );
}
//------------------------------------------------------------------
/**
 * 汎用コントロール処理
 *
 * @param   wk		描画処理ワーク
 * @param   sp		処理中の文字列ポインタ（=SPCODE_TAG_START_をポイントしている状態で呼ぶこと）
 *
 * @retval  const STRCODE*		処理後の文字列ポインタ
 */
//------------------------------------------------------------------
static const STRCODE* ctrlGeneralTag( PRINT_JOB* wk, const STRCODE* sp )
{
	switch( STR_TOOL_GetTagNumber(sp) ){
	case CTRL_GENERAL_COLOR:
		{
			u8 colL, colS, colB;

			colL = STR_TOOL_GetTagParam(sp, 0);
			colS = STR_TOOL_GetTagParam(sp, 1);
			colB = STR_TOOL_GetTagParam(sp, 2);

			GFL_FONTSYS_SetColor( colL, colS, colB );
		}
		break;

	case CTRL_GENERAL_RESET_COLOR:
		GFL_FONTSYS_SetColor( wk->colorLetter, wk->colorShadow, wk->colorBackGround );
		break;

	case CTRL_GENERAL_X_RIGHTFIT:
		{
			int bmpWidth, strWidth;

			bmpWidth = GFL_BMP_GetSizeX( wk->dst ) - wk->org_x;
			strWidth = get_line_width( sp, wk->fontHandle, 0, NULL );

			wk->write_x = wk->org_x + ((bmpWidth - strWidth) / 2);

			OS_TPrintf("[PRINTSYS] XfittingR ... bmpW=%d, strW=%d -> wrtX=%d\n",
						bmpWidth, strWidth, wk->write_x);
		}
		break;

	case CTRL_GENERAL_X_CENTERING:
		{
			int bmpWidth, strWidth;

			bmpWidth = GFL_BMP_GetSizeX( wk->dst ) - wk->org_x;
			strWidth = get_line_width( sp, wk->fontHandle, 0, NULL );

			if( strWidth < bmpWidth )
			{
				wk->write_x = wk->org_x + ((bmpWidth - strWidth) / 2);
			}
			else
			{
				wk->write_x = wk->org_x;
			}
		}
		break;

	default:
		GF_ASSERT_MSG(0, "illegal general-ctrl code ->%d\n", STR_TOOL_GetTagNumber(sp));
		break;
	}

	return STR_TOOL_SkipTag( sp );
}
//------------------------------------------------------------------
/**
 * システムコントロール処理
 *
 * @param   wk		描画処理ワーク
 * @param   sp		処理中の文字列ポインタ（=SPCODE_TAG_START_をポイントしている状態で呼ぶこと）
 *
 * @retval  const STRCODE*		処理後の文字列ポインタ
 */
//------------------------------------------------------------------
static const STRCODE* ctrlSystemTag( PRINT_JOB* wk, const STRCODE* sp )
{
	u16 ctrlCode = STR_TOOL_GetTagNumber(sp);

	switch( ctrlCode ){
	case CTRL_SYSTEM_COLOR:
		{
			u8  color = STR_TOOL_GetTagParam( sp, 0 );
			GFL_FONTSYS_SetColor( color*2+1, color*2+2, wk->colorBackGround );
		}
		break;

	default:
		GF_ASSERT_MSG(0, "illegal system-ctrl code ->%d\n", ctrlCode);
		break;
	}

	return STR_TOOL_SkipTag( sp );
}


//==============================================================================================================
// プリントストリーム関連処理
//==============================================================================================================

//==============================================================================================
/**
 * プリントストリーム作成（通常版 - コールバックなし）
 *
 * @param   dst		描画先Bitmap
 * @param   xpos	描画開始Ｘ座標（ドット）
 * @param   ypos	描画開始Ｙ座標（ドット）
 * @param   str		文字列
 * @param   font	フォントタイプ
 * @param   wait		１文字描画ごとのウェイトフレーム数
 * @param   tcbsys		使用するGFL_TCBLシステムポインタ
 * @param   tcbpri		使用するタスクプライオリティ
 * @param   heapID		使用するヒープID
 *
 * @retval	PRINT_STREAM*	ストリームハンドル
 *
 */
//==============================================================================================
PRINT_STREAM* PRINTSYS_PrintStream(
		GFL_BMPWIN* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font,
		u16 wait, GFL_TCBLSYS* tcbsys, u32 tcbpri, HEAPID heapID, u16 clearColor )
{
	return PRINTSYS_PrintStreamCallBack(
		dst, xpos, ypos, str, font, wait, tcbsys, tcbpri, heapID, clearColor, NULL
	);
}

//==============================================================================================
/**
 * プリントストリーム作成（コールバックあり）
 *
 * @param   dst		描画先Bitmap
 * @param   xpos	描画開始Ｘ座標（ドット）
 * @param   ypos	描画開始Ｙ座標（ドット）
 * @param   str		文字列
 * @param   font	フォントタイプ
 * @param   wait		１文字描画ごとのウェイトフレーム数
 * @param   tcbsys		使用するGFL_TCBLシステムポインタ
 * @param   tcbpri		使用するタスクプライオリティ
 * @param   heapID		使用するヒープID
 * @param   callback	１文字描画ごとのコールバック関数アドレス
 *
 * @retval	PRINT_STREAM*	ストリームハンドル
 *
 */
//==============================================================================================
PRINT_STREAM* PRINTSYS_PrintStreamCallBack(
		GFL_BMPWIN* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font,
		u16 wait, GFL_TCBLSYS* tcbsys, u32 tcbpri, HEAPID heapID, u16 clearColor, pPrintCallBack callback )
{
	PRINT_STREAM* stwk;
	GFL_TCBL* tcb;
	GFL_BMP_DATA* dstBmp;

	dstBmp = GFL_BMPWIN_GetBmp( dst );;
	tcb = GFL_TCBL_Create( tcbsys, print_stream_task, sizeof(PRINT_STREAM), tcbpri );
	stwk = GFL_TCBL_GetWork( tcb );
	stwk->tcb = tcb;

	setupPrintJob( &stwk->printJob, font, dstBmp, xpos, ypos );

	stwk->sp = GFL_STR_GetStringCodePointer( str );
	stwk->org_wait = wait;
	stwk->current_wait = wait;
	stwk->wait = 0;
	stwk->callback_func = callback;
	stwk->org_arg = 0;
	stwk->current_arg = 0;
	stwk->arg = 0;
	stwk->dstWin = dst;
	stwk->dstBmp = dstBmp;
	stwk->state = PRINTSTREAM_STATE_RUNNING;
	stwk->clearColor = clearColor;
	stwk->pauseType = PRINTSTREAM_PAUSE_CLEAR;
	stwk->pauseWait = 0;
	stwk->pauseReleaseFlag = FALSE;

	return stwk;
}

//==============================================================================================
/**
 * プリントストリーム状態取得
 *
 * @param   handle		ストリームハンドラ
 *
 * @retval  PRINTSTREAM_STATE		状態
 */
//==============================================================================================
PRINTSTREAM_STATE PRINTSYS_PrintStreamGetState( PRINT_STREAM* handle )
{
	return handle->state;
}
//==============================================================================================
/**
 * プリントストリーム停止タイプ取得
 *
 * @param   handle		ストリームハンドラ
 *
 * @retval  PRINTSTREAM_PAUSE_TYPE		停止タイプ
 */
//==============================================================================================
PRINTSTREAM_PAUSE_TYPE PRINTSYS_PrintStreamGetPauseType( PRINT_STREAM* handle )
{
	return handle->pauseType;
}
//==============================================================================================
/**
 * プリントストリーム停止状態解除
 *
 * @param   handle		
 *
 */
//==============================================================================================
void PRINTSYS_PrintStreamReleasePause( PRINT_STREAM* handle )
{
	handle->pauseReleaseFlag = TRUE;
}
//==============================================================================================
/**
 * プリントストリーム削除
 *
 * @param   handle		
 *
 */
//==============================================================================================
void PRINTSYS_PrintStreamDelete( PRINT_STREAM* handle )
{
	if( handle->tcb )
	{
		GFL_TCBL_Delete( handle->tcb );
		handle->tcb = NULL;
	}
}
//==============================================================================================
/**
 * プリントストリームウェイト短縮（キー押し中など）
 *
 * @param   handle		ストリームハンドル
 * @param   wait		設定するウェイトフレーム数
 *
 */
//==============================================================================================
void PRINTSYS_PrintStreamShortWait( PRINT_STREAM* handle, u16 wait )
{
	if( wait < handle->wait )
	{
		handle->wait = wait;
	}
}

//------------------------------------------------------------------
/**
 * プリントストリーム描画タスク
 *
 * @param   tcb			
 * @param   wk_adrs		
 *
 */
//------------------------------------------------------------------
static void print_stream_task( GFL_TCBL* tcb, void* wk_adrs )
{
	PRINT_STREAM* wk = wk_adrs;

	switch( wk->state ){
	case PRINTSTREAM_STATE_RUNNING:
		if( wk->wait == 0 )
		{
			switch( *(wk->sp) ){
			case EOM_CODE:
				wk->state = PRINTSTREAM_STATE_DONE;
				break;

			case SPCODE_TAG_START_:
				if( STR_TOOL_GetTagType(wk->sp) == TAGTYPE_STREAM_CTRL )
				{
					ctrlStreamTag( wk );
					break;
				}
				/* fallthru */
			default:
				wk->sp = print_next_char( &wk->printJob, wk->sp );

				if( wk->callback_func )
				{
					wk->callback_func( wk->arg );
					wk->arg = wk->current_arg;
				}

				if( *(wk->sp) == EOM_CODE )
				{
					wk->state = PRINTSTREAM_STATE_DONE;
				}
				else
				{
					wk->wait = wk->current_wait;
				}

				GFL_BMPWIN_TransVramCharacter( wk->dstWin );
				break;
			}
		}
		else
		{
			wk->wait--;
		}
		break;

	case PRINTSTREAM_STATE_PAUSE:
		if( wk->pauseReleaseFlag )
		{
			switch( wk->pauseType ){
			case PRINTSTREAM_PAUSE_LINEFEED:	// @@@ 現在ラインフィード処理は未実装
				{
					u16 size_x, size_y;

					size_x = GFL_BMP_GetSizeX( wk->dstBmp );
					size_y = GFL_BMP_GetSizeY( wk->dstBmp );
					GFL_BMP_Print( wk->dstBmp, wk->dstBmp, 0, 1, 0, 0, size_x, size_y-1, 0xff );
					GFL_BMP_Fill( wk->dstBmp, 0, size_y-1, size_x, 1, wk->clearColor );
					GFL_BMPWIN_TransVramCharacter( wk->dstWin );
					if( wk->pauseWait == LINE_DOT_HEIGHT )
					{
						wk->printJob.write_x = 0;
						wk->printJob.write_y = LINE_DOT_HEIGHT;
						wk->state = PRINTSTREAM_STATE_RUNNING;
					}
					else
					{
						wk->pauseWait++;
					}
				}
				break;

			case PRINTSTREAM_PAUSE_CLEAR:
			default:
				GFL_BMP_Clear( wk->dstBmp, wk->clearColor );
				wk->state = PRINTSTREAM_STATE_RUNNING;
				break;
			}
		}
		break;
	}
}

//--------------------------------------------------------------------------
/**
 * プリントストリーム用コントロールタグ処理
 *
 * @param   wk		
 *
 */
//--------------------------------------------------------------------------
static void ctrlStreamTag( PRINT_STREAM* wk )
{
	BOOL skipCR = FALSE;	// 直後の改行を無視する

	switch( STR_TOOL_GetTagNumber(wk->sp) ){
	case CTRL_STREAM_LINE_FEED:
		wk->state = PRINTSTREAM_STATE_PAUSE;
		wk->pauseType = PRINTSTREAM_PAUSE_LINEFEED;
		wk->pauseWait = 0;
		wk->pauseReleaseFlag = FALSE;
		wk->printJob.write_x = 0;
		wk->printJob.write_y = 0;
		skipCR = TRUE;
		break;
	case CTRL_STREAM_PAGE_CLEAR:
		wk->state = PRINTSTREAM_STATE_PAUSE;
		wk->pauseType = PRINTSTREAM_PAUSE_CLEAR;
		wk->pauseWait = 0;
		wk->pauseReleaseFlag = FALSE;
		wk->printJob.write_x = 0;
		wk->printJob.write_y = 0;
		skipCR = TRUE;
		break;
	case CTRL_STREAM_CHANGE_WAIT:
		wk->wait = STR_TOOL_GetTagParam( wk->sp, 0 );
		break;
	case CTRL_STREAM_SET_WAIT:
		wk->current_wait = STR_TOOL_GetTagParam( wk->sp, 0 );;
		break;
	case CTRL_STREAM_RESET_WAIT:	///< 描画ウェイトをデフォルトに戻す
		wk->current_wait = wk->org_wait;
		break;
	case CTRL_STREAM_CHANGE_ARGV:	///< コールバック引数を変更（１回）
		wk->arg = STR_TOOL_GetTagParam( wk->sp, 0 );;
		break;
	case CTRL_STREAM_SET_ARGV:		///< コールバック引数を変更（永続）
		wk->current_arg = STR_TOOL_GetTagParam( wk->sp, 0 );
		wk->arg = STR_TOOL_GetTagParam( wk->sp, 0 );
		break;
	case CTRL_STREAM_RESET_ARGV:	///< コールバック引数をデフォルトに戻す
		wk->current_arg = wk->org_arg;
		wk->arg = wk->org_arg;
		break;
	case CTRL_STREAM_FORCE_CLEAR:
		GFL_BMP_Clear( wk->dstBmp, wk->clearColor );
		break;
	}

	wk->sp = STR_TOOL_SkipTag( wk->sp );

	if( skipCR && *(wk->sp) == CR_CODE )
	{
		wk->sp++;
	}

}

//--------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------
/**
 * 文字列１行分のドット数計算
 *
 * @param   sp			文字列ポインタ
 * @param   font		フォントタイプ
 * @param   margin		字間スペース（ドット）
 * @param   endPtr		[out] 次行の先頭ポインタを格納するポインタアドレス（NULLなら無視する）
 *
 * @retval  u32		ドット数
 */
//--------------------------------------------------------------------------
static u32 get_line_width( const STRCODE* sp, GFL_FONT* font, u16 margin, const STRCODE** endPtr )
{
	u32 width = 0;

	while( *sp != EOM_CODE && *sp != CR_CODE )
	{
		if( *sp != SPCODE_TAG_START_ )
		{
			width += ( GFL_FONT_GetWidth( font, *sp ) + margin );
			sp++;
		}
		else
		{
			sp = STR_TOOL_SkipTag( sp );
		}
	}

	if( endPtr )
	{
		if( *sp == CR_CODE ){ sp++; }
		*endPtr = sp;
	}

	return width;
}


//==============================================================================================
/**
 * 文字列をBitmap表示する際の幅（ドット数）を計算
 * ※文字列が複数行ある場合、その最長行のドット数
 *
 * @param   str			文字列
 * @param   font		フォントタイプ
 * @param   margin		字間スペース（ドット）
 *
 * @retval  u32			ドット数
 */
//==============================================================================================
u32 PRINTSYS_GetStrWidth( const STRBUF* str, GFL_FONT* font, u16 margin )
{
	u32 width, max;
	const STRCODE* sp;

	width = max = 0;
	sp = GFL_STR_GetStringCodePointer( str );

	while( *sp != EOM_CODE )
	{
		width = get_line_width( sp, font, margin, &sp );
		if( width > max )
		{
			max = width;
		}
	}

	return max;
}


//=============================================================================================
/**
 * タグ開始コードを取得
 *
 * @retval  STRCODE		
 */
//=============================================================================================
STRCODE PRINTSYS_GetTagStartCode( void )
{
	return SPCODE_TAG_START_;
}
//=============================================================================================
/**
 * タグコードが単語セット用タグコードかどうか判定
 *
 * @param   sp		文字列ポインタ（タグ開始コードをポイントしていること）
 *
 * @retval  BOOL		単語セット用タグコードならTRUE
 */
//=============================================================================================
BOOL PRINTSYS_IsWordSetTagType( const STRCODE* sp )
{
	u16 type = STR_TOOL_GetTagType( sp );
	return (type == TAGTYPE_WORD) || (type==TAGTYPE_NUMBER);
}

//=============================================================================================
/**
 * タグパラメータ取得
 *
 * @param   sp		
 * @param   paramIdx		
 *
 * @retval  u16		
 */
//=============================================================================================
u16 PRINTSYS_GetTagParam( const STRCODE* sp, u16 paramIdx )
{
	return STR_TOOL_GetTagParam( sp, paramIdx );
}

//=============================================================================================
/**
 * タグ部分をスキップ
 *
 * @param   sp		
 *
 * @retval  const STRCODE*		
 */
//=============================================================================================
const STRCODE* PRINTSYS_SkipTag( const STRCODE* sp )
{
	return STR_TOOL_SkipTag( sp );
}






//============================================================================================
/**
 * 文字列ポインタの指しているタグコード・パラメータ部から、タグタイプを取得
 *
 * @param   sp		文字列ポインタ（タグ開始コードを指している必要がある）
 *
 * @retval  u16		タグ種類
 */
//============================================================================================
static inline u16 STR_TOOL_GetTagType( const STRCODE* sp )
{
	GF_ASSERT( *sp == SPCODE_TAG_START_ );

	if( *sp == SPCODE_TAG_START_ )
	{
		sp++;
		return (((*sp)>>8)&0xff);
	}

	return 0xffff;
}
//============================================================================================
/**
 * 文字列ポインタのさしているタグコード・パラメータ部から、パラメータ値を取得
 *
 * @param   sp				文字列ポインタ（タグ開始コードを指している必要がある）
 * @param   paramIndex		パラメータインデックス
 *
 * @retval  u16				パラメータ値
 */
//============================================================================================
static inline u16 STR_TOOL_GetTagParam( const STRCODE* sp, u16 paramIndex )
{
	GF_ASSERT( *sp == SPCODE_TAG_START_ );

	if( *sp == SPCODE_TAG_START_ )
	{
		u32 params;
		sp += 2;
		params = *sp++;
		GF_ASSERT(paramIndex<params);
		return sp[paramIndex];
	}
	return 0;
}
//============================================================================================
/**
 * 文字列ポインタのタグコード・パラメータ部分をスキップしたポインタを返す
 *
 * @param   sp		文字列ポインタ（タグコード開始値を指していること）
 *
 * @retval  const STRCODE*		スキップ後のポインタ
 */
//============================================================================================
static inline const STRCODE* STR_TOOL_SkipTag( const STRCODE* sp )
{
	GF_ASSERT( *sp == SPCODE_TAG_START_ );

	if( *sp == SPCODE_TAG_START_ )
	{
		u32 params;
		sp += 2;
		params = *sp++;
		sp += params;
		return sp;
	}
	return sp;
}
//============================================================================================
/**
 * 文字列ポインタの指しているタグコード・パラメータ部から、タグ番号を取得
 *
 * @param   sp		文字列ポインタ（タグ開始コードを指している必要がある）
 *
 * @retval  u16		タグ種類
 */
//============================================================================================
static inline u16 STR_TOOL_GetTagNumber( const STRCODE* sp )
{
	GF_ASSERT( *sp == SPCODE_TAG_START_ );

	if( *sp == SPCODE_TAG_START_ )
	{
		sp++;
		return ((*sp)&0xff);
	}

	return 0xffff;
}


//==============================================================================================================
// プリントキュー関連処理
//==============================================================================================================

//--------------------------------------------------------------------------
/**
 * 文字列格納に必要なバッファサイズ(byte)を計算。※4の倍数に揃える
 *
 * @param   str		[in] 文字列格納バッファ
 *
 * @retval  u32		バッファサイズ（byte）
 */
//--------------------------------------------------------------------------
static u32 Que_CalcStringBufferSize( const STRBUF* str )
{
	u32 size = sizeof(STRCODE) * (GFL_STR_GetBufferLength(str) + 1);
	while( size % 4 ){ size++; }
	return size;
}
//--------------------------------------------------------------------------
/**
 * １件の書き込み情報を プリントキューに保存するために必要なバッファサイズ（byte）を計算
 *
 * @param   buf		[in] 書き込む文字列が格納されている文字列バッファ
 *
 * @retval  u32		プリントキューが必要とするバッファサイズ（byte）
 */
//--------------------------------------------------------------------------
static u32 Que_CalcUseBufSize( const STRBUF* buf )
{
	return sizeof(PRINT_JOB) + sizeof(PRINT_QUE*) + Que_CalcStringBufferSize(buf);
}

//--------------------------------------------------------------------------
/**
 * プリントキュー のバッファ空きサイズを取得
 *
 * @param   que		[in] プリントキュー
 *
 * @retval  u32		空きサイズ（byte）
 */
//--------------------------------------------------------------------------
static u32 Que_GetFreeSize( const PRINT_QUE* que )
{
	if( que->bufTopPos < que->bufEndPos )
	{
		return que->bufEndPos - que->bufTopPos;
	}
	else
	{
		return (que->bufSize - que->bufTopPos) + que->bufEndPos;
	}
}

//--------------------------------------------------------------------------
/**
 * プリントキューに新規描画処理を追加
 *
 * @param   que			[out] プリントキュー
 * @param   printJob	[in] 内容設定済みの描画処理ワーク
 * @param   str			[in] 描画文字列バッファ
 *
 */
//--------------------------------------------------------------------------
static void Que_StoreNewJob( PRINT_QUE* que, PRINT_JOB* printJob, const STRBUF* str )
{
	PRINT_JOB* store_adrs;
	const STRCODE* sp;

	sp = Que_WriteDataUnit(	que,
						(const u8*)(GFL_STR_GetStringCodePointer(str)),
						Que_CalcStringBufferSize(str),
						sizeof(STRCODE) );

	store_adrs = Que_WriteDataUnit( que, (const u8*)(printJob), sizeof(PRINT_JOB), sizeof(PRINT_JOB) );


	// 次に処理する作業内容へのポインタ（新規追加なら次は無いのでNULLにしておく
	{
		PRINT_JOB* p = NULL;
		Que_WriteDataUnit( que, (const u8*)(&p), sizeof(PRINT_JOB*), sizeof(PRINT_JOB*) );
	}

	if( que->runningJob != NULL )
	{
		PRINT_JOB *job, *next;
		u16 pos;

		job = que->runningJob;
		while(1)
		{
			next = Que_ReadNextJobAdrs( que, job );
			if( next == NULL )
			{
				break;
			}
			job = next;
		}
		pos = Que_AdrsToBufPos( que, job );
		pos = Que_FwdBufPos( que, pos, sizeof(PRINT_JOB) );
		Que_WriteData( que, (const u8*)(&store_adrs), pos, sizeof(store_adrs) );
	}
	else
	{
		que->runningJob = store_adrs;
		que->sp = sp;
	}
}
//--------------------------------------------------------------------------
/**
 * プリントキューのリングバッファにデータを書き込み、
 * それに応じて内部の書き込みポインタを移動させる。
 *
 * @param   que				[io] プリントキュー （内部書き込みポインタが移動する）
 * @param   src				[in] 書き込むデータポインタ
 * @param   totalSize		[in] 書き込みデータ総サイズ
 * @param   unitSize		[in] アドレス分断せずに書き込む単位
 *
 * @retval  void*			格納先頭アドレス
 */
//--------------------------------------------------------------------------
static void* Que_WriteDataUnit( PRINT_QUE* que, const u8* src, u16 totalSize, u16 unitSize )
{
	u16 wroteSize = 0;
	void* topAdrs = &(que->buf[ que->bufTopPos ]);

	GF_ASSERT((que->bufTopPos%4)==0);

	while( wroteSize < totalSize )
	{
		que->bufTopPos = Que_WriteData( que, src, que->bufTopPos, unitSize );
		src += unitSize;
		wroteSize += unitSize;
	}

	GF_ASSERT((que->bufTopPos%4)==0);

	return topAdrs;
}
//--------------------------------------------------------------------------
/**
 * 指定されたJobの次のJobアドレスを返す
 *
 * @param   que		[in] プリントキュー
 * @param   job		[in] プリントキューのバッファに格納されている Job アドレス
 *
 * @retval  PRINT_JOB		次のJobアドレス（無ければNULL）
 */
//--------------------------------------------------------------------------
static PRINT_JOB* Que_ReadNextJobAdrs( const PRINT_QUE* que, const PRINT_JOB* job )
{
	u32 bufPos = Que_AdrsToBufPos( que, job );

	bufPos = Que_FwdBufPos( que, bufPos, sizeof(PRINT_JOB) );

	{
		PRINT_JOB** ppjob = (PRINT_JOB**)Que_BufPosToAdrs( que, bufPos );
		return *ppjob;
	}
}


//--------------------------------------------------------------------------
/**
 * プリントキューのリングバッファにデータを書き込む
 *
 * @param   que		[out] プリントキュー
 * @param   src		[in]  書き込みデータポインタ
 * @param   pos		[in]  バッファ書き込みポインタ
 * @param   size	[in]  書き込みデータサイズ
 *
 * @retval  u16		次に書き込むときのバッファ書き込みポインタ
 */
//--------------------------------------------------------------------------
static inline u16 Que_WriteData( PRINT_QUE* que, const u8* src, u16 pos, u16 size )
{
	GFL_STD_MemCopy( src, &(que->buf[pos]), size );
	pos += size;
	if( pos >= que->bufSize )
	{
		pos = 0;
	}
	return pos;
}
//--------------------------------------------------------------------------
/**
 * リングバッファ中の任意のアドレスを、書き込みポインタに変換
 *
 * @param   que				[in] プリントキュー
 * @param   bufAdrs			[in] リングバッファ中の任意のアドレス
 *
 * @retval  u16				書き込みポインタ
 */
//--------------------------------------------------------------------------
static inline u16 Que_AdrsToBufPos( const PRINT_QUE* que, const void* bufAdrs )
{
	u32 pos = ((u32)bufAdrs) - ((u32)que->buf);
	GF_ASSERT(pos < que->bufSize);
	return pos;
}
//--------------------------------------------------------------------------
/**
 * リングバッファの書き込みポインタをアドレスに変換
 *
 * @param   que				[in] プリントキュー
 * @param   bufPos			[in] リングバッファ書き込みポインタ
 *
 * @retval  void*			アドレス
 */
//--------------------------------------------------------------------------
static inline const void* Que_BufPosToAdrs( const PRINT_QUE* que, u16 bufPos )
{
	return &(que->buf[bufPos]);
}
//--------------------------------------------------------------------------
/**
 * リングバッファ書き込みポインタを、指定サイズ分進めた値に変換
 *
 * @param   que			プリントキュー
 * @param   pos			書き込みポインタ初期値
 * @param   fwdSize	 	進めるサイズ（バイト単位）
 *
 * @retval  pos を fwdSize 分だけ進めた値
 */
//--------------------------------------------------------------------------
static inline u16 Que_FwdBufPos( const PRINT_QUE* que, u16 pos, u16 fwdSize )
{
	pos += fwdSize;
	if( pos >= que->bufSize )
	{
		pos = 0;
	}
	return pos;
}

