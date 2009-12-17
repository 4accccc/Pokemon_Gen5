//=============================================================================================
/**
 * @file  printsys.c
 * @brief Bitmap利用文字描画　システム制御
 * @author  taya
 *
 * @date  2007.02.06  作成
 * @date  2008.09.11  通信対応キュー構造を作成
 * @date  2008.09.26  キュー構造を利用しない文字列描画処理も復活
 */
//=============================================================================================
#ifndef __PRINTSYS_H__
#define __PRINTSYS_H__

#include <tcbl.h>
#include <bmp_win.h>
#include <strbuf.h>

#include "print\gf_font.h"


//============================================================================================================
// Typedefs

//--------------------------------------------------------------
/**
 *  Print Queue
 *  通信時に描画処理を小刻みに実行できるよう、処理内容を蓄積するための機構。
 */
//--------------------------------------------------------------
typedef struct _PRINT_QUE PRINT_QUE;

//--------------------------------------------------------------------------
/**
 *  Print Stream
 *  会話ウィンドウ等、１定間隔で１文字ずつ描画，表示を行うための機構
 */
//--------------------------------------------------------------------------
typedef struct _PRINT_STREAM  PRINT_STREAM;

//--------------------------------------------------------------------------
/**
 *  Print Stream Callback
 *  Print Stream で１文字描画ごとに呼び出されるコールバック関数の型
 *
 *  引数：u32 gmmで設定した任意の値（未設定なら0）
 *
 *  戻り値：基本的には常時FALSEを返すようにしてください。
 *          TRUEを返している間は Stream タスクが空回りして次の処理へ遷移しなくなります。
 *          （SEの終了待ちなどに使用することを想定しています）
 */
//--------------------------------------------------------------------------
typedef BOOL (*pPrintCallBack)(u32);


//--------------------------------------------------------------------------
/**
 *  Print System で用いる描画色データ
 *  文字(Letter)、影(Shadow)、背景色(Background) の番号各5bitをパックしたもの
 */
//--------------------------------------------------------------------------
typedef u16   PRINTSYS_LSB;


#define PRINTSYS_MACRO_LSB(L, S, B)  (PRINTSYS_LSB)((((L)&0x1f)<<10) | (((S)&0x1f)<<5) | ((b)&0x1f))

static inline PRINTSYS_LSB PRINTSYS_LSB_Make( u8 l, u8 s, u8 b )
{
  return PRINTSYS_MACRO_LSB( l, s, b );
}
static inline u8 PRINTSYS_LSB_GetL( PRINTSYS_LSB color )
{
  return (color >> 10) & 0x1f;
}
static inline u8 PRINTSYS_LSB_GetS( PRINTSYS_LSB color )
{
  return (color >> 5) & 0x1f;
}
static inline u8 PRINTSYS_LSB_GetB( PRINTSYS_LSB color )
{
  return (color & 0x1f);
}
static inline void PRINTSYS_LSB_GetLSB( PRINTSYS_LSB color, u8* l, u8* s, u8* b )
{
  *l = PRINTSYS_LSB_GetL( color );
  *s = PRINTSYS_LSB_GetS( color );
  *b = PRINTSYS_LSB_GetB( color );
}

//============================================================================================================
// Consts

//--------------------------------------------------------------------------
/**
 *  Print Stream の状態を識別
 */
//--------------------------------------------------------------------------
typedef enum {
  PRINTSTREAM_STATE_RUNNING = 0,  ///< 処理実行中（文字列が流れている）
  PRINTSTREAM_STATE_PAUSE,    ///< 一時停止中（ページ切り替え待ち等）
  PRINTSTREAM_STATE_DONE,     ///< 文字列終端まで表示完了
}PRINTSTREAM_STATE;

//--------------------------------------------------------------------------
/**
 *  Print Stream の一時停止状態を識別
 */
//--------------------------------------------------------------------------
typedef enum {
  PRINTSTREAM_PAUSE_LINEFEED = 0, ///< ページ切り替え待ち（改行）
  PRINTSTREAM_PAUSE_CLEAR,        /// <ページ切り替え待ち（表示クリア）
}PRINTSTREAM_PAUSE_TYPE;


//============================================================================================================
// Prototypes

//==============================================================================================
/**
 * システム初期化（プログラム起動時に１度だけ呼び出す）
 *
 * @param   heapID    初期化用ヒープID
 *
 */
//==============================================================================================
extern void PRINTSYS_Init( HEAPID heapID );

//==============================================================================================
/**
 * プリントキューをデフォルトのバッファサイズで生成する。
 *
 * @param   heapID      生成用ヒープID
 *
 * @retval  PRINT_QUE*    生成されたプリントキュー
 */
//==============================================================================================
extern PRINT_QUE* PRINTSYS_QUE_Create( HEAPID heapID );

//==============================================================================================
/**
 * プリントキューをサイズ指定して生成する
 *
 * @param   size      バッファサイズ
 * @param   heapID      生成用ヒープID
 *
 * @retval  PRINT_QUE*    生成されたプリントキュー
 */
//==============================================================================================
extern PRINT_QUE* PRINTSYS_QUE_CreateEx( u16 buf_size, HEAPID heapID );

//=============================================================================================
/**
 * プリントキューが通信中、一度の処理に使う時間（Tick）を設定する
 *
 * @param   que     プリントキュー
 * @param   tick    処理時間
 *
 */
//=============================================================================================
extern void PRINTSYS_QUE_SetLimitTick( PRINT_QUE* que, OSTick tick );

//==============================================================================================
/**
 * プリントキューメイン処理（メインループ中で１回呼び出す）
 *
 * @param   que   プリントキュー
 *
 * @retval  BOOL  キュー処理が残っていない場合はTRUE／にまだ処理が残っている場合はFALSE
 */
//==============================================================================================
extern BOOL PRINTSYS_QUE_Main( PRINT_QUE* que );

//==============================================================================================
/**
 * プリントキューに処理が残っているか判定
 *
 * @param   que   プリントキュー
 *
 * @retval  BOOL  キュー処理が残っていない場合はTRUE／にまだ処理が残っている場合はFALSE
 */
//==============================================================================================
extern BOOL PRINTSYS_QUE_IsFinished( const PRINT_QUE* que );

//==============================================================================================
/**
 * プリントキューに、特定Bitmapを出力対象とした処理が残っているか判定
 *
 * @param   que       プリントキュー
 * @param   targetBmp   出力対象のBitmap
 *
 * @retval  BOOL      targetBmpに対する処理が残っている場合はTRUE／それ以外はFALSE
 */
//==============================================================================================
extern BOOL PRINTSYS_QUE_IsExistTarget( const PRINT_QUE* que, const GFL_BMP_DATA* targetBmp );

//==============================================================================================
/**
 * プリントキューを介した文字列描画
 *
 * @param   que   [out] 描画処理内容を記録するためのプリントキュー
 * @param   dst   [out] 描画先Bitmap
 * @param   xpos  [in]  描画開始Ｘ座標（ドット）
 * @param   ypos  [in]  描画開始Ｙ座標（ドット）
 * @param   str   [in]  文字列
 * @param   font  [in]  フォント
 *
 */
//==============================================================================================
extern void PRINTSYS_PrintQue( PRINT_QUE* que, GFL_BMP_DATA* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font );

//==============================================================================================
/**
 * プリントキューを介した文字列描画（色変更に対応）
 *
 * @param   que   [out] 描画処理内容を記録するためのプリントキュー
 * @param   dst   [out] 描画先Bitmap
 * @param   xpos  [in]  描画開始Ｘ座標（ドット）
 * @param   ypos  [in]  描画開始Ｙ座標（ドット）
 * @param   str   [in]  文字列
 * @param   font  [in]  フォント
 * @param   color [in]  色番号
 *
 */
//==============================================================================================
extern void PRINTSYS_PrintQueColor( PRINT_QUE* que, GFL_BMP_DATA* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font, PRINTSYS_LSB color );

//=============================================================================================
/**
 * プリントキューに貯まっている処理を全てクリアする
 *
 * @param   que   プリントキュー
 *
 */
//=============================================================================================
extern void PRINTSYS_QUE_Clear( PRINT_QUE* que );

//==============================================================================================
/**
 * プリントキューを削除する
 *
 * @param   que   プリントキュー
 *
 */
//==============================================================================================
extern void PRINTSYS_QUE_Delete( PRINT_QUE* que );

//=============================================================================================
/**
 * [デバッグ用] 非通信時にも通信時と同様の挙動を行わせる
 *
 * @param   que
 * @param   flag    TRUEで有効／FALSEで無効に戻す
 *
 */
//=============================================================================================
extern void PRINTSYS_QUE_ForceCommMode( PRINT_QUE* que, BOOL flag );


//==============================================================================================
/**
 * Bitmap へ直接の文字列描画
 *
 * @param   dst   [out] 描画先Bitmap
 * @param   xpos  [in]  描画開始Ｘ座標（ドット）
 * @param   ypos  [in]  描画開始Ｙ座標（ドット）
 * @param   str   [in]  文字列
 * @param   font  [in]  フォント
 *
 */
//==============================================================================================
extern void PRINTSYS_Print( GFL_BMP_DATA* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font );

//=============================================================================================
/**
 * Bitmap へ直接の文字列描画（カラー指定版）
 *
 * @param   dst   [out] 描画先Bitmap
 * @param   xpos  [in]  描画開始Ｘ座標（ドット）
 * @param   ypos  [in]  描画開始Ｙ座標（ドット）
 * @param   str   [in]  文字列
 * @param   font  [in]  フォント
 * @param   color [in]  描画カラー
 *
 */
//=============================================================================================
extern void PRINTSYS_PrintColor( GFL_BMP_DATA* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font, PRINTSYS_LSB color );

//==============================================================================================
/**
 * プリントストリームを利用した文字列描画（通常版 - コールバックなし）
 *
 * @param   dst     描画先Bitmap
 * @param   xpos    描画開始Ｘ座標（ドット）
 * @param   ypos    描画開始Ｙ座標（ドット）
 * @param   str     文字列
 * @param   font    フォントハンドラ
 * @param   wait    １文字描画ごとのウェイトフレーム数（※）
 * @param   tcbsys    使用するGFL_TCBLシステムポインタ
 * @param   tcbpri    使用するタスクプライオリティ
 * @param   heapID    使用するヒープID
 *
 * @retval  PRINT_STREAM* ストリームハンドル
 *
 * ※ wait に-2 以下の値を設定することで、毎フレーム２文字以上の描画を行う。-2（２文字）, -3（３文字）...
 */
//==============================================================================================
extern PRINT_STREAM* PRINTSYS_PrintStream(
    GFL_BMPWIN* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font,
    int wait, GFL_TCBLSYS* tcbsys, u32 tcbpri, HEAPID heapID, u16 clearColor );

//==============================================================================================
/**
 * プリントストリームを利用した文字列描画（コールバックあり）
 *
 * @param   dst     描画先Bitmap
 * @param   xpos    描画開始Ｘ座標（ドット）
 * @param   ypos    描画開始Ｙ座標（ドット）
 * @param   str     文字列
 * @param   font    フォントハンドラ
 * @param   wait    １文字描画ごとのウェイトフレーム数（※ 通常版と同じ）
 * @param   tcbsys    使用するGFL_TCBLシステムポインタ
 * @param   tcbpri    使用するタスクプライオリティ
 * @param   heapID    使用するヒープID
 * @param   callback  １文字描画ごとのコールバック関数アドレス
 *
 * @retval  PRINT_STREAM* ストリームハンドラ
 *
 */
//==============================================================================================
extern PRINT_STREAM* PRINTSYS_PrintStreamCallBack(
    GFL_BMPWIN* dst, u16 xpos, u16 ypos, const STRBUF* str, GFL_FONT* font,
    int wait, GFL_TCBLSYS* tcbsys, u32 tcbpri, HEAPID heapID, u16 clearColor, pPrintCallBack callback );

//==============================================================================================
/**
 * プリントストリーム状態取得
 *
 * @param   handle    ストリームハンドラ
 *
 * @retval  PRINTSTREAM_STATE   状態
 */
//==============================================================================================
extern PRINTSTREAM_STATE PRINTSYS_PrintStreamGetState( PRINT_STREAM* handle );

//==============================================================================================
/**
 * プリントストリーム一時停止タイプ取得
 *
 * @param   handle    ストリームハンドラ
 *
 * @retval  PRINTSTREAM_PAUSE_TYPE    停止タイプ
 */
//==============================================================================================
extern PRINTSTREAM_PAUSE_TYPE PRINTSYS_PrintStreamGetPauseType( PRINT_STREAM* handle );

//==============================================================================================
/**
 * プリントストリーム一時停止を解除
 *
 * @param   handle    ストリームハンドラ
 *
 */
//==============================================================================================
extern void PRINTSYS_PrintStreamReleasePause( PRINT_STREAM* handle );

//==============================================================================================
/**
 * プリントストリーム削除
 *
 * @param   handle    ストリームハンドラ
 *
 */
//==============================================================================================
extern void PRINTSYS_PrintStreamDelete( PRINT_STREAM* handle );

//=============================================================================================
/**
 * プリントストリームをユーザプログラムが強制的に一時停止する
 *
 * @param   handle
 */
//=============================================================================================
extern void PRINTSYS_PrintStreamStop( PRINT_STREAM* handle );

//=============================================================================================
/**
 * プリントストリームのユーザ強制一時停止を解除する
 *
 * @param   handle
 */
//=============================================================================================
extern void PRINTSYS_PrintStreamRun( PRINT_STREAM* handle );

//==============================================================================================
/**
 * プリントストリームウェイト短縮（キー押し中などに毎フレーム呼び出すことで強制的にウェイトを短縮）
 *
 * @param   handle    ストリームハンドラ
 * @param   wait    設定するウェイトフレーム数
 *
 */
//==============================================================================================
extern void PRINTSYS_PrintStreamShortWait( PRINT_STREAM* handle, u16 wait );

//==============================================================================================
/**
 * 文字列をBitmap表示する際の幅（ドット数）を計算
 * ※文字列が複数行ある場合、その最長行のドット数
 *
 * @param   str     文字列
 * @param   font    フォントハンドラ
 * @param   margin    字間スペース（ドット）
 *
 * @retval  u32     文字列幅（ドット）
 */
//==============================================================================================
extern u32 PRINTSYS_GetStrWidth( const STRBUF* str, GFL_FONT* font, u16 margin );

//=============================================================================================
/**
 * ※ローカライズ注意！　使用は許可制です。※
 *
 * 文字列の行数を返す
 *
 * @param   str
 *
 * @retval  u32
 */
//=============================================================================================
extern u32 PRINTSYS_GetLineCount( const STRBUF* str );

//=============================================================================================
/**
 * ※ローカライズ注意！　使用は許可制です。※
 *
 * 文字列をBitmap表示する際の幅（ドット数）を計算
 * ※文字列が複数行ある場合、行ごとの幅を取得できる
 *

 * @param   str       文字列
 * @param   font      フォントハンドル
 * @param   margin    字間スペース（ドット）
 * @param   dst       行ごとの幅を取得するための配列
 * @param   dstElems  dst の配列要素数（これを越えないように書き込みます）
 *
 * @retval  u32       dst に格納した件数（最大でも dstElemsを越えない）
 */
//=============================================================================================
extern u32 PRINTSYS_GetStrLineWidth( const STRBUF* str, GFL_FONT* font, u16 margin, u32* dst, u32 dstElems );

//=============================================================================================
/**
 * ※ローカライズ注意！　使用は許可制です。※
 *
 * 複数行ある文字列から指定行の文字列のみを取り出す
 *
 *
 * @param   src     読み取り元の文字列
 * @param   dst     読み取り先バッファ
 * @param   line    行番号（0～）
 *
 * @retval  BOOL    指定 line が大きすぎて読み取れない場合FALSE／それ以外はTRUE
 */
//=============================================================================================
extern BOOL PRINTSYS_CopyLineStr( const STRBUF* src, STRBUF* dst, u32 line );



//=============================================================================================
/**
 * 文字列をBitmap表示する際の幅（ドット数）を計算
 *
 * @param   str     文字列
 * @param   font    フォントハンドラ
 *
 * @retval  u32     文字列高さ（ドット）
 */
//=============================================================================================
extern u32 PRINTSYS_GetStrHeight( const STRBUF* str, GFL_FONT* font );

//=============================================================================================
/**
 * 文字列の中に含まれる、単語タグの数をカウントして返す
 *
 * @param   str   文字列
 *
 * @retval  u16   単語タグの数
 */
//=============================================================================================
extern u16 PRINTSYS_GetTagCount( const STRBUF* str );

//=============================================================================================
/**
 * 指定したタグが文字列中何行目にあるかを返す
 *
 * @param   str     文字列
 * @param   tag_id  タグインデックス（バッファ番号）
 *
 * @retval  u8      行数
 *
 * @note  add by genya hosaka
 */
//=============================================================================================
extern u8 PRINTSYS_GetTagLine( const STRBUF* str, u8 tag_id );

//=============================================================================================
/**
 * 指定したタグが何ドット目にあるかを返す
 *
 * @param   str     文字列
 * @param   tag_id  タグインデックス（バッファ番号）
 * @param   font    フォントタイプ
 * @param   margin  字間スペース（ドット）
 *
 * @retval  u8      X座標（ドット）
 *
 * @note  add by genya hosaka
 */
//=============================================================================================
extern u8 PRINTSYS_GetTagWidth( const STRBUF* str, u8 tag_id, GFL_FONT* font, u16 margin );


//--------------------------------------------------------------
/**
 *  ユーティリティ管理構造体
 *
 *  中身を公開してるのでAlloc不要。
 *  各自ワーク領域に必要なだけ作ってください
 */
//--------------------------------------------------------------
typedef struct {
  GFL_BMPWIN*   win;
  u8        transReq;
}PRINT_UTIL;


//--------------------------------------------------------------------------------------
/**
 * ユーティリティ初期化（BitmapWindowと１対１で関連付けを行う。１回だけ呼び出し）
 *
 * @param   wk
 * @param   win
 */
//--------------------------------------------------------------------------------------
inline void PRINT_UTIL_Setup( PRINT_UTIL* wk, GFL_BMPWIN* win )
{
  wk->win = win;
  wk->transReq = FALSE;
}

//--------------------------------------------------------------------------------------
/**
 * ユーティリティを介してBitmap文字列描画
 *
 * @param   wk
 * @param   que
 * @param   xpos
 * @param   ypos
 * @param   buf
 * @param   font
 *
 */
//--------------------------------------------------------------------------------------
inline void PRINT_UTIL_Print( PRINT_UTIL* wk, PRINT_QUE* que, u16 xpos, u16 ypos, const STRBUF* buf, GFL_FONT* font )
{
  PRINTSYS_PrintQue( que, GFL_BMPWIN_GetBmp(wk->win), xpos, ypos, buf, font );
  wk->transReq = TRUE;
}
//--------------------------------------------------------------------------------------
/**
 * ユーティリティを介してBitmap文字列描画（色変更に対応）
 *
 * @param   wk
 * @param   que
 * @param   xpos
 * @param   ypos
 * @param   buf
 * @param   font
 * @param   color
 *
 */
//--------------------------------------------------------------------------------------
inline void PRINT_UTIL_PrintColor( PRINT_UTIL* wk, PRINT_QUE* que, u16 xpos, u16 ypos, const STRBUF* buf, GFL_FONT* font, PRINTSYS_LSB color )
{
  PRINTSYS_PrintQueColor( que, GFL_BMPWIN_GetBmp(wk->win), xpos, ypos, buf, font, color );
  wk->transReq = TRUE;
}


//--------------------------------------------------------------------------------------
/**
 * 必要なタイミングを判断して、CGXデータのVRAM転送を行います
 *
 * @param   wk
 * @param   que
 *
 * @retval  BOOL  転送が終わっている場合はTRUE／終わっていない場合はFALSE
 */
//--------------------------------------------------------------------------------------
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
  return !(wk->transReq);
}



//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------

// タグ大分類
typedef enum {
  PRINTSYS_TAGGROUP_WORD = 1,           ///< 通常の単語
  PRINTSYS_TAGGROUP_NUM  = 2,           ///< 数値
  PRINTSYS_TAGGROUP_CTRL_GEN = 189,     ///< 汎用コントロール
  PRINTSYS_TAGGROUP_CTRL_STREAM = 190,  ///< ストリームコントロール
}PrintSysTagGroup;

// 汎用コントロールタイプ
typedef enum {
  PRINTSYS_CTRL_GENERAL_COLOR        = (0x0000), ///< 色変更
  PRINTSYS_CTRL_GENERAL_RESET_COLOR  = (0x0001), ///< 色変更
  PRINTSYS_CTRL_GENERAL_X_CENTERING  = (0x0002), ///< Ｘ座標右寄せ
  PRINTSYS_CTRL_GENERAL_X_RIGHTFIT   = (0x0003), ///< Ｘ座標センタリング
  PRINTSYS_CTRL_GENERAL_X_ADD        = (0x0004), ///< Ｘ座標を加算
}PrintSysGenCtrlCode;

// ストリームコントロールタイプ
typedef enum {
  PRINTSYS_CTRL_STREAM_LINE_FEED     = (0x0000), ///< 改ページ（行送り待ち）
  PRINTSYS_CTRL_STREAM_PAGE_CLEAR    = (0x0001), ///< 改ページ（描画クリア待ち）
  PRINTSYS_CTRL_STREAM_CHANGE_WAIT   = (0x0002), ///< 描画ウェイト変更（１回）
  PRINTSYS_CTRL_STREAM_SET_WAIT      = (0x0003), ///< 描画ウェイト変更（永続）
  PRINTSYS_CTRL_STREAM_RESET_WAIT    = (0x0004), ///< 描画ウェイトをデフォルトに戻す
  PRINTSYS_CTRL_STREAM_CHANGE_ARGV   = (0x0005), ///< コールバック引数を変更（１回）
  PRINTSYS_CTRL_STREAM_SET_ARGV      = (0x0006), ///< コールバック引数を変更（永続）
  PRINTSYS_CTRL_STREAM_RESET_ARGV    = (0x0007), ///< コールバック引数をデフォルトに戻す
  PRINTSYS_CTRL_STREAM_FORCE_CLEAR   = (0x0008), ///< 強制描画クリア
}PrintSysStreamCtrlCode;


extern STRCODE PRINTSYS_GetTagStartCode( void );
extern BOOL PRINTSYS_IsWordSetTagGroup( const STRCODE* sp );
extern PrintSysTagGroup  PRINTSYS_GetTagGroup( const STRCODE* sp );
extern u8  PRINTSYS_GetTagIndex( const STRCODE* sp );
extern u16 PRINTSYS_GetTagParam( const STRCODE* sp, u16 paramIdx );
extern const STRCODE* PRINTSYS_SkipTag( const STRCODE* sp );
extern void PRINTSYS_CreateTagCode( STRBUF* str, PrintSysTagGroup tagGrp, u8 tagIdx, u8 numParams, const u16* params );

#endif
