//=============================================================================================
/**
 * @file	printsys.c
 * @brief	Bitmap利用文字描画　システム制御
 * @author	taya
 * @date	2007.02.06
 */
//=============================================================================================
#include "gflib.h"
#include "tcbl.h"

#include "system/main.h"


#include "printsys.h"



enum {
	STR_EXPAND_MAXLEN = 500,		///< 文字データ展開用の一時バッファサイズ
	LINE_DOT_HEIGHT = 16,

	// 汎用コントロールタイプ
	CTRL_GENERAL_COLOR			= (0x0000),	///< 色変更
	CTRL_GENERAL_RESET_COLOR	= (0x0001),	///< 色変更
	CTRL_GENERAL_XPOS			= (0x0002),	///< 描画Ｘ座標
	CTRL_GENERAL_YPOS			= (0x0003),	///< 描画Ｙ座標
	CTRL_GENERAL_X_CENTERING	= (0x0004),	///< Ｘ座標センタリング

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

	EOM_CODE			= 0xffff,
	CR_CODE				= 0xfffe,
	SPCODE_TAG_START_	= 0xf000,

	TAGTYPE_WORD = 0,				///< 文字列挿入
	TAGTYPE_NUMBER = 1,				///< 数値挿入
	TAGTYPE_PARTY = 2,				///< パーティ用文字列
	TAGTYPE_GENERAL_CTRL = 0xbd,	///< 汎用コントロール処理
	TAGTYPE_STREAM_CTRL = 0xbe,		///< 流れるメッセージ用コントロール処理
	TAGTYPE_SYSTEM = 0xff,			///< エディタシステムタグ用コントロール処理

};

typedef u16		STRCODE;


typedef struct {
	u16   org_x;
	u16   org_y;
	u16   write_x;
	u16   write_y;

	u8    colorLetter;
	u8    colorShadow;
	u8    colorBackGround;

	GFL_FONT*	fontHandle;

	GFL_BMP_DATA*	dst;

}PRINT_WORK;



struct _PRINT_STREAM_WORK {

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

	PRINT_WORK	printWork;

	

};

typedef struct _PRINT_STREAM_WORK	PRINT_STREAM_WORK;





//======================================================================

static struct {
	GFL_BMP_DATA*		charBuffer;
	PRINT_WORK			printWork;
	STRCODE				expandBuffer[ STR_EXPAND_MAXLEN + 1 ];
}SystemWork;

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void initPrintWork( PRINT_WORK* wk, GFL_FONT* font, GFL_BMP_DATA* dst, u16 org_x, u16 org_y );
static const STRCODE* print_next_char( PRINT_WORK* wk, const STRCODE* sp, GFL_BMP_DATA* dst );
static void put_1char( GFL_BMP_DATA* dst, u16 xpos, u16 ypos, GFL_FONT* fontHandle, u32 charCode, u16* charWidth, u16* charHeight );
static const STRCODE* ctrlGeneralTag( PRINT_WORK* wk, const STRCODE* sp );
static const STRCODE* ctrlSystemTag( PRINT_WORK* wk, const STRCODE* sp );
static void print_stream_task( GFL_TCBL* tcb, void* wk_adrs );
static void ctrlStreamTag( PRINT_STREAM_WORK* wk );
static u32 get_line_width( const STRCODE* sp, GFL_FONT* font, u16 margin, const STRCODE** endPtr );
static inline u16 STR_TOOL_GetTagType( const STRCODE* sp );
static inline u16 STR_TOOL_GetTagParam( const STRCODE* sp, u16 paramIndex );
static inline const STRCODE* STR_TOOL_SkipTag( const STRCODE* sp );
static inline u16 STR_TOOL_GetTagNumber( const STRCODE* sp );





//==============================================================================================
/**
 * システム初期化（GFLIB初期化後に１度だけ呼ぶ）
 */
//==============================================================================================
void PRINTSYS_Init( HEAPID heapID )
{
	GFL_STR_SetEOMCode( EOM_CODE );
//	GFL_FONTSYS_Init();

	SystemWork.charBuffer = GFL_BMP_Create( 2, 2, GFL_BMP_16_COLOR, heapID );
}

//==============================================================================================
/**
 * BITMAPに対する文字列描画（一括）
 *
 * @param   dst		[out] 描画先Bitmap
 * @param   xpos	[in]  描画開始Ｘ座標（ドット）
 * @param   ypos	[in]  描画開始Ｙ座標（ドット）
 * @param   str		[in]  文字列
 * @param   font	[in]  フォントタイプ
 *
 */
//==============================================================================================
void PRINTSYS_Print( GFL_BMP_DATA* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font )
{
	const STRCODE* sp;
	PRINT_WORK* wk = &SystemWork.printWork;

	initPrintWork( wk, font, dst, xpos, ypos );
	sp = GFL_STR_GetStringCodePointer( str );

	while( *sp != EOM_CODE )
	{
		sp = print_next_char( wk, sp, dst );
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
static void initPrintWork( PRINT_WORK* wk, GFL_FONT* font, GFL_BMP_DATA* dst, u16 org_x, u16 org_y )
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
 * @param   dst		
 * @param   font		
 *
 * @retval  const STRCODE*		
 */
//------------------------------------------------------------------
static const STRCODE* print_next_char( PRINT_WORK* wk, const STRCODE* sp, GFL_BMP_DATA* dst )
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

				put_1char( dst, wk->write_x, wk->write_y, wk->fontHandle, *sp, &w, &h );
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
static void put_1char( GFL_BMP_DATA* dst, u16 xpos, u16 ypos, GFL_FONT* fontHandle, u32 charCode, u16* charWidth, u16* charHeight )
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
static const STRCODE* ctrlGeneralTag( PRINT_WORK* wk, const STRCODE* sp )
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

	case CTRL_GENERAL_XPOS:
		wk->write_x = STR_TOOL_GetTagParam( sp, 0 );
		break;

	case CTRL_GENERAL_YPOS:
		wk->write_y = STR_TOOL_GetTagParam( sp, 0 );;
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
static const STRCODE* ctrlSystemTag( PRINT_WORK* wk, const STRCODE* sp )
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




//==============================================================================================
/**
 * プリントストリーム（BITMAP流れる文字列描画）開始
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
 * @param   callback	１文字描画ごとのコールバック関数アドレス（不要ならNULL）
 *
 * @retval	PRINT_STREAM_HANDLE	ストリームハンドル
 *
 */
//==============================================================================================
PRINT_STREAM_HANDLE PRINTSYS_PrintStream(
		GFL_BMPWIN* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font,
		u16 wait, GFL_TCBLSYS* tcbsys, u32 tcbpri, u16 heapID, u16 clearColor, pPrintCallBack callback )
{
	PRINT_STREAM_WORK* stwk;
	GFL_TCBL* tcb;
	GFL_BMP_DATA* dstBmp;

	dstBmp = GFL_BMPWIN_GetBmp( dst );;
	tcb = GFL_TCBL_Create( tcbsys, print_stream_task, sizeof(PRINT_STREAM_WORK), tcbpri );
	stwk = GFL_TCBL_GetWork( tcb );
	stwk->tcb = tcb;

	initPrintWork( &stwk->printWork, font, dstBmp, xpos, ypos );

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
 * @param   handle		ストリームハンドル
 *
 * @retval  PRINTSTREAM_STATE		状態
 */
//==============================================================================================
PRINTSTREAM_STATE PRINTSYS_PrintStreamGetState( PRINT_STREAM_HANDLE handle )
{
	return handle->state;
}

//==============================================================================================
/**
 * プリントストリーム停止タイプ取得
 *
 * @param   handle		ストリームハンドル
 *
 * @retval  PRINTSTREAM_PAUSE_TYPE		停止タイプ
 */
//==============================================================================================
PRINTSTREAM_PAUSE_TYPE PRINTSYS_PrintStreamGetPauseType( PRINT_STREAM_HANDLE handle )
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
void PRINTSYS_PrintStreamReleasePause( PRINT_STREAM_HANDLE handle )
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
void PRINTSYS_PrintStreamDelete( PRINT_STREAM_HANDLE handle )
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
void PRINTSYS_PrintStreamShortWait( PRINT_STREAM_HANDLE handle, u16 wait )
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
	PRINT_STREAM_WORK* wk = wk_adrs;

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
				wk->sp = print_next_char( &wk->printWork, wk->sp, wk->dstBmp );

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
						wk->printWork.write_x = 0;
						wk->printWork.write_y = LINE_DOT_HEIGHT;
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
static void ctrlStreamTag( PRINT_STREAM_WORK* wk )
{
	BOOL skipCR = FALSE;	// 直後の改行を無視する

	switch( STR_TOOL_GetTagNumber(wk->sp) ){
	case CTRL_STREAM_LINE_FEED:
		wk->state = PRINTSTREAM_STATE_PAUSE;
		wk->pauseType = PRINTSTREAM_PAUSE_LINEFEED;
		wk->pauseWait = 0;
		wk->pauseReleaseFlag = FALSE;
		wk->printWork.write_x = 0;
		wk->printWork.write_y = 0;
		skipCR = TRUE;
		break;
	case CTRL_STREAM_PAGE_CLEAR:
		wk->state = PRINTSTREAM_STATE_PAUSE;
		wk->pauseType = PRINTSTREAM_PAUSE_CLEAR;
		wk->pauseWait = 0;
		wk->pauseReleaseFlag = FALSE;
		wk->printWork.write_x = 0;
		wk->printWork.write_y = 0;
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


