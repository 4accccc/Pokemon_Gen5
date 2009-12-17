//=============================================================================================
/**
 * @file  printsys.c
 * @brief Bitmap利用文字描画　システム制御
 * @author  taya
 *
 * @date  2007.02.06  作成
 * @date  2008.09.11  通信対応キュー構造を作成
 */
//=============================================================================================
#include <gflib.h>
#include <tcbl.h>

#include "print/printsys.h"



//==============================================================
// Consts
//==============================================================
enum {
  LETTER_CHAR_WIDTH = 2,      ///< １文字あたり横キャラ数
  LETTER_CHAR_HEIGHT = 2,     ///< １文字あたり縦キャラ数
  LINE_DOT_HEIGHT = 16,       ///< １行あたりのドット数
  LINE_FEED_SPEED = 4,        ///< 改行時の行送り速度（dot/frame）

  // 改行コード、終端コード
  EOM_CODE      = 0xffff,
  CR_CODE       = 0xfffe,

  // システムコントロール
  CTRL_SYSTEM_COLOR     = (0x0000),

  // タグ開始コード
  SPCODE_TAG_START_ = 0xf000,   ///<
  TAGTYPE_WORD = 1,             ///< 文字列挿入
  TAGTYPE_NUMBER = 2,           ///< 数値挿入
  TAGTYPE_GENERAL_CTRL = 0xbd,  ///< 汎用コントロール処理
  TAGTYPE_STREAM_CTRL = 0xbe,   ///< 流れるメッセージ用コントロール処理
  TAGTYPE_SYSTEM = 0xff,        ///< エディタシステムタグ用コントロール処理


  // プリントキュー関連定数
  QUE_DEFAULT_BUFSIZE = 1024,
  QUE_DEFAULT_TICK = 4400,    ///< 通信時、１度の描画処理に使う時間の目安（Tick）デフォルト値

  //
  JOB_COLORSTATE_IGNORE = 0,
  JOB_COLORSTATE_CHANGE_REQ,
  JOB_COLORSTATE_CHANGE_DONE,

};


//==============================================================
// Typedefs
//==============================================================

// １文字書き込み関数フォーマット
typedef void (*pPut1CharFunc)( GFL_BMP_DATA* dst, s16 xpos, s16 ypos, GFL_FONT* fontHandle, STRCODE charCode, GFL_FONT_SIZE* size );

//--------------------------------------------------------------------------
/**
 *  Print Job
 *
 *  Bitmapへの描画処理ワーク
 */
//--------------------------------------------------------------------------
typedef struct {

  s16   write_x;
  s16   write_y;

  s32   org_x      : 10;
  s32   org_y      : 10;
  s32   colorState : 4;
  s32   pad1       : 18;

  PRINTSYS_LSB    defColor;
  PRINTSYS_LSB    jobColor;

  GFL_FONT*       fontHandle;   ///< フォントハンドル
  GFL_BMP_DATA*   dst;          ///< 書き込み先 Bitmap
  pPut1CharFunc   put1charFunc; ///< １文字書き込み関数

}PRINT_JOB;

//--------------------------------------------------------------------------
/**
 *  Print Queue
 *
 *  通信時、描画処理を小刻みに実行できるよう、処理内容を蓄積するための機構。
 */
//--------------------------------------------------------------------------
struct _PRINT_QUE {

  u16   bufTopPos;
  u16   bufEndPos;
  u16   bufSize;
  u8    debugCounter;
  u8    fColorChanged  : 4;
  u8    fForceCommMode : 4;   ///< 非通信時にも通信中と同じ挙動をする（デバッグ用）

  PRINTSYS_LSB  defColor;

  OSTick        limitPerFrame;

  PRINT_JOB*    runningJob;
  const STRCODE*  sp;

  u8   buf[0];
};

//--------------------------------------------------------------------------
/**
 *  Print Stream
 *
 *  会話ウィンドウ等、１定間隔で１文字ずつ描画，表示を行うための機構。
 */
//--------------------------------------------------------------------------
struct _PRINT_STREAM {

  PRINTSTREAM_STATE   state;
  PRINTSTREAM_PAUSE_TYPE  pauseType;

  GFL_BMPWIN*     dstWin;
  GFL_BMP_DATA*   dstBmp;
  GFL_TCBL*     tcb;

  const STRCODE* sp;

  u16   org_wait;
  u16   current_wait;
  u16   wait;
  u8    pauseReleaseFlag;
  u8    pauseWait;
  u8    clearColor;
  u8    putPerFrame;
  u8    stopFlag;
  u8    callbackResult;

  pPrintCallBack  callback_func;
  u32       org_arg;
  u32       current_arg;
  u32       arg;

  PRINT_JOB printJob;
};


//==============================================================================================
//--------------------------------------------------------------------------
/**
 *  SystemWork
 */
//--------------------------------------------------------------------------
static struct {
  GFL_BMP_DATA*   charBuffer;
  PRINT_JOB     printJob;
}SystemWork;


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static inline BOOL IsNetConnecting( void );
static void PrintQue_Core( PRINT_QUE* que, PRINT_JOB* job, const STRBUF* str );
static void printJob_setup( PRINT_JOB* wk, GFL_FONT* font, GFL_BMP_DATA* dst, s16 org_x, s16 org_y );
static void printJob_setColor( PRINT_JOB* wk, PRINTSYS_LSB color );
static void printJob_finish( PRINT_JOB* job, const STRBUF* str );
static const STRCODE* print_next_char( PRINT_JOB* wk, const STRCODE* sp );
static void put1char_normal( GFL_BMP_DATA* dst, s16 xpos, s16 ypos, GFL_FONT* fontHandle, STRCODE charCode, GFL_FONT_SIZE* size );
static void put1char_16to256( GFL_BMP_DATA* dst, s16 xpos, s16 ypos, GFL_FONT* fontHandle, STRCODE charCode, GFL_FONT_SIZE* size );
static const STRCODE* ctrlGeneralTag( PRINT_JOB* wk, const STRCODE* sp );
static const STRCODE* ctrlSystemTag( PRINT_JOB* wk, const STRCODE* sp );
static void print_stream_task( GFL_TCBL* tcb, void* wk_adrs );
static void ctrlStreamTag( PRINT_STREAM* wk );
static u32 get_line_width( const STRCODE* sp, GFL_FONT* font, u16 margin, const STRCODE** endPtr );
static inline u8 STR_TOOL_GetTagGroup( const STRCODE* sp );
static inline u8 STR_TOOL_GetTagIndex( const STRCODE* sp );
static inline STRCODE STR_TOOL_CreateTagCode( PrintSysTagGroup grp, u8 index );
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
 * @param   heapID    初期化用ヒープID
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
 * @param   heapID      生成用ヒープID
 *
 * @retval  PRINT_QUE*    生成されたプリントキュー
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
 * @param   size      バッファサイズ
 * @param   heapID      生成用ヒープID
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

  que->bufSize = buf_size;    // 終端判定は指定のサイズ値で行う
  que->limitPerFrame = QUE_DEFAULT_TICK;
  que->runningJob = NULL;
  que->sp = NULL;
  que->fColorChanged = FALSE;
  que->fForceCommMode = FALSE;


  que->debugCounter=0;

  que->bufTopPos = 0;
  que->bufEndPos = buf_size;

//  GFL_STD_MemClear( que->buf, size );

  return que;
}

//==============================================================================================
/**
 * プリントキューを削除する
 *
 * @param   que   プリントキュー
 *
 */
//==============================================================================================
void PRINTSYS_QUE_Delete( PRINT_QUE* que )
{
  GF_ASSERT(que->runningJob==NULL);
  GFL_HEAP_FreeMemory( que );
}

//=============================================================================================
/**
 * [デバッグ用] 強制的に通信時と同じ動作をするモードに変更
 *
 * @param   que
 * @param   flag  TRUEで有効／FALSEで無効化
 */
//=============================================================================================
void PRINTSYS_QUE_ForceCommMode( PRINT_QUE* que, BOOL flag )
{
  que->fForceCommMode = flag;
}

//=============================================================================================
/**
 * プリントキューが通信中、一度の処理に使う時間（Tick）を設定する
 *
 * @param   que
 * @param   tick
 *
 */
//=============================================================================================
void PRINTSYS_QUE_SetLimitTick( PRINT_QUE* que, OSTick tick )
{
  que->limitPerFrame = tick;
}

//==============================================================================================
/**
 * プリントキューに蓄積された文字列描画作業を処理する。
 * 通信中なら一定時間毎に処理を返す。
 * 通信中でなければ一気に処理を終える（…のは未実装）
 *
 * @param   que   プリントキュー
 *
 * @retval  BOOL  処理が終了していればTRUE／それ以外はFALSE
 */
//==============================================================================================
BOOL PRINTSYS_QUE_Main( PRINT_QUE* que )
{
  que->debugCounter++;
  if( que->runningJob )
  {
    OSTick start, diff;
    u8 endFlag = FALSE;

    start = OS_GetTick();

    while( que->runningJob )
    {
      if( que->runningJob->colorState == JOB_COLORSTATE_CHANGE_REQ )
      {
        u8 colL, colS, colB;

        PRINTSYS_LSB_GetLSB( que->runningJob->jobColor, &colL, &colS, &colB );
        if( GFL_FONTSYS_IsDifferentColor(colL, colS, colB) )
        {
          // 最初の色変更時、デフォルトの色情報を覚えておく
          if( que->fColorChanged == FALSE )
          {
            u8 defL, defS, defB;
            GFL_FONTSYS_GetColor( &defL, &defS, &defB );
            que->defColor = PRINTSYS_LSB_Make( defL, defS, defB );
            que->fColorChanged = TRUE;
          }
          GFL_FONTSYS_SetColor( colL, colS, colB );
          que->runningJob->colorState = JOB_COLORSTATE_CHANGE_DONE;
        }
      }

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
        u16 pos;

        // job 終了ごとにデフォルト色に戻す
        if( que->runningJob->colorState == JOB_COLORSTATE_CHANGE_DONE )
        {
          u8 defL, defS, defB;
          PRINTSYS_LSB_GetLSB( que->defColor, &defL, &defS, &defB );
          if( GFL_FONTSYS_IsDifferentColor( defL, defS, defB ) ){
            GFL_FONTSYS_SetColor( defL, defS, defB );
          }
        }
        pos = Que_AdrsToBufPos( que, que->runningJob );

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

  if( que->fColorChanged )
  {
    u8 defL, defS, defB;
    PRINTSYS_LSB_GetLSB( que->defColor, &defL, &defS, &defB );
    if( GFL_FONTSYS_IsDifferentColor( defL, defS, defB ) ){
      GFL_FONTSYS_SetColor( defL, defS, defB );
    }
  }

  return TRUE;
}
//==============================================================================================
/**
 * プリントキューの処理が全て完了しているかチェック
 *
 * @param   que     プリントキュー
 *
 * @retval  BOOL    完了していればTRUE
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
 * @param   que       プリントキュー
 * @param   targetBmp   描画対象Bitmap
 *
 * @retval  BOOL      残っていればTRUE
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

//=============================================================================================
/**
 * プリントキューに貯まっている処理を全てクリアする
 *
 * @param   que   プリントキュー
 *
 */
//=============================================================================================
void PRINTSYS_QUE_Clear( PRINT_QUE* que )
{
  que->bufTopPos = 0;
  que->bufEndPos = que->bufSize;
  que->sp = NULL;
  que->runningJob = NULL;
}

//--------------------------------------------------------------------------
/**
 * 通信中（描画処理を分割する必要がある）かどうか判定
 *
 * @retval  BOOL    通信中ならTRUE
 */
//--------------------------------------------------------------------------
static inline BOOL IsNetConnecting( void )
{
  return (GFL_NET_GetConnectNum() != 0);
}


//==============================================================================================
/**
 * プリントキューを介した文字列描画処理リクエスト
 *
 * @param   que   [out] プリントキュー
 * @param   dst   [out] 描画先Bitmap
 * @param   xpos  [in]  描画開始Ｘ座標（ドット）
 * @param   ypos  [in]  描画開始Ｙ座標（ドット）
 * @param   str   [in]  文字列
 * @param   font  [in]  フォントタイプ
 *
 */
//==============================================================================================
void PRINTSYS_PrintQue( PRINT_QUE* que, GFL_BMP_DATA* dst, s16 xpos, s16 ypos, const STRBUF* str, GFL_FONT* font )
{
  GF_ASSERT(que);
  GF_ASSERT(dst);
  GF_ASSERT(str);
  GF_ASSERT(font);

  {
    PRINT_JOB* job = &SystemWork.printJob;
    printJob_setup( job, font, dst, xpos, ypos );

    PrintQue_Core( que, job, str );
  }
}
//=============================================================================================
/**
 * プリントキューを介した文字列描画処理リクエスト（色変更に対応）
 *
 * @param   que
 * @param   dst
 * @param   xpos
 * @param   ypos
 * @param   str
 * @param   font
 * @param   col
 *
 */
//=============================================================================================
void PRINTSYS_PrintQueColor( PRINT_QUE* que, GFL_BMP_DATA* dst, s16 xpos, s16 ypos, const STRBUF* str, GFL_FONT* font, PRINTSYS_LSB color )
{
  GF_ASSERT(que);
  GF_ASSERT(dst);
  GF_ASSERT(str);
  GF_ASSERT(font);

  {
    PRINT_JOB* job = &SystemWork.printJob;

    printJob_setup( job, font, dst, xpos, ypos );
    printJob_setColor( job, color );

    PrintQue_Core( que, job, str );
  }
}
//--------------------------------------------------------------------------
/**
 * プリントキューを介した文字列描画処理リクエスト共通処理部分
 *
 * @param   job
 *
 */
//--------------------------------------------------------------------------
static void PrintQue_Core( PRINT_QUE* que, PRINT_JOB* job, const STRBUF* str )
{
  if( !IsNetConnecting() && !que->fForceCommMode )
  {
    printJob_finish( job, str );
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
      GF_ASSERT_MSG(0, "[PRINT_QUE] buffer over ... strsize = %d\n", size);
    }
  }
}

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
void PRINTSYS_Print( GFL_BMP_DATA* dst, s16 xpos, s16 ypos, const STRBUF* str, GFL_FONT* font )
{
  PRINT_JOB* job = &SystemWork.printJob;
  const STRCODE* sp = GFL_STR_GetStringCodePointer( str );

  printJob_setup( job, font, dst, xpos, ypos );
  printJob_finish( job, str );
}
//=============================================================================================
/**
 * Bitmap へ直接の文字列描画（カラー指定版）
 *
 * @param   dst
 * @param   xpos
 * @param   ypos
 * @param   str
 * @param   font
 * @param   color
 */
//=============================================================================================
void PRINTSYS_PrintColor( GFL_BMP_DATA* dst, s16 xpos, s16 ypos, const STRBUF* str, GFL_FONT* font, PRINTSYS_LSB color )
{
  PRINT_JOB* job = &SystemWork.printJob;
  const STRCODE* sp = GFL_STR_GetStringCodePointer( str );

  printJob_setup( job, font, dst, xpos, ypos );
  printJob_setColor( job, color );
  printJob_finish( job, str );
}


//------------------------------------------------------------------
/**
 * 描画処理ワークの初期化
 *
 * @param   wk      ワークポインタ
 * @param   font_type フォントタイプ
 * @param   dst     描画先Bitmap
 * @param   org_x   描画開始Ｘ座標
 * @param   org_y   描画開始Ｙ座標
 *
 */
//------------------------------------------------------------------
static void printJob_setup( PRINT_JOB* wk, GFL_FONT* font, GFL_BMP_DATA* dst, s16 org_x, s16 org_y )
{
  wk->dst = dst;
  wk->put1charFunc = (GFL_BMP_GetColorFormat(dst) == GFL_BMP_16_COLOR)?
                put1char_normal : put1char_16to256;
  wk->fontHandle = font;
  wk->org_x = wk->write_x = org_x;
  wk->org_y = wk->write_y = org_y;
  wk->colorState = JOB_COLORSTATE_IGNORE;

  {
    u8 letter, shadow, back;
    GFL_FONTSYS_GetColor( &letter, &shadow, &back );
    wk->defColor = PRINTSYS_LSB_Make( letter, shadow, back );
  }
}
//--------------------------------------------------------------------------
/**
 * 処理ワーク単位で使用色を変更するリクエスト
 *
 * @param   wk
 *
 */
//--------------------------------------------------------------------------
static void printJob_setColor( PRINT_JOB* wk, PRINTSYS_LSB color )
{
  wk->jobColor = color;
  wk->colorState = JOB_COLORSTATE_CHANGE_REQ;
}
//--------------------------------------------------------------------------
/**
 * 描画処理ワークの処理内容を１回で実行する
 *
 * @param   job   描画処理ワーク
 * @param   str   処理文字列
 *
 */
//--------------------------------------------------------------------------
static void printJob_finish( PRINT_JOB* job, const STRBUF* str )
{
  const STRCODE* sp = GFL_STR_GetStringCodePointer( str );

  u8 defColorL, defColorS, defColorB, colorChanged;

  defColorL = defColorS = defColorB = 0;
  colorChanged = 0;

  if( job->colorState == JOB_COLORSTATE_CHANGE_REQ )
  {
    u8 colL, colS, colB;

    PRINTSYS_LSB_GetLSB( job->jobColor, &colL, &colS, &colB );
    if( GFL_FONTSYS_IsDifferentColor(colL, colS, colB) )
    {
      GFL_FONTSYS_GetColor( &defColorL, &defColorS, &defColorB );
      GFL_FONTSYS_SetColor( colL, colS, colB );
      colorChanged = TRUE;
    }
    job->colorState = JOB_COLORSTATE_CHANGE_DONE;
  }

  while( *sp != EOM_CODE )
  {
    sp = print_next_char( job, sp );
  }

  if( colorChanged )
  {
    GFL_FONTSYS_SetColor( defColorL, defColorS, defColorB );
  }
}


//------------------------------------------------------------------
/**
 * 改行および汎用コントロールがあれば処理し、
 * 通常文字があれば１文字描画して、次の文字のポインタを返す
 *
 * @param   wk    描画ワークポインタ
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
      switch( STR_TOOL_GetTagGroup(sp) ){
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
        GFL_FONT_SIZE size;
        u16 w, h;

        wk->put1charFunc( wk->dst, wk->write_x, wk->write_y, wk->fontHandle, *sp, &size );
        wk->write_x += size.width;
        sp++;

        return sp;
      }
      break;
    }
  }
}
//------------------------------------------------------------------
/**
 * Bitmap１文字分描画（通常版=16色->16色フォーマット）
 *
 * @param[out]  dst       描画先ビットマップ
 * @param[in] xpos        描画先Ｘ座標（ドット）
 * @param[in] ypos        描画先Ｙ座標（ドット）
 * @param[in] fontHandle  描画フォントハンドル
 * @param[in] charCode    文字コード
 * @param[out]  size      文字サイズ取得ワーク
 *
 */
//------------------------------------------------------------------
static void put1char_normal( GFL_BMP_DATA* dst, s16 xpos, s16 ypos, GFL_FONT* fontHandle, STRCODE charCode, GFL_FONT_SIZE* size )
{
  GFL_FONT_GetBitMap( fontHandle, charCode, GFL_BMP_GetCharacterAdrs(SystemWork.charBuffer), size );
  GFL_BMP_Print( SystemWork.charBuffer, dst, 0, 0, xpos+size->left_width, ypos, size->glyph_width, size->height, 0x00 );
}
//------------------------------------------------------------------
/**
 * Bitmap１文字分描画（16色->256色フォーマット）
 *
 * @param[out]  dst       描画先ビットマップ
 * @param[in] xpos        描画先Ｘ座標（ドット）
 * @param[in] ypos        描画先Ｙ座標（ドット）
 * @param[in] fontHandle  描画フォントハンドル
 * @param[in] charCode    文字コード
 * @param[out]  size      文字サイズ取得ワーク
 *
 */
//------------------------------------------------------------------
static void put1char_16to256( GFL_BMP_DATA* dst, s16 xpos, s16 ypos, GFL_FONT* fontHandle, STRCODE charCode, GFL_FONT_SIZE* size )
{
  GFL_FONT_GetBitMap( fontHandle, charCode, GFL_BMP_GetCharacterAdrs(SystemWork.charBuffer), size );
  GFL_BMP_Print16to256( SystemWork.charBuffer, dst, 0, 0, xpos+size->left_width, ypos, size->glyph_width, size->height, 0x0f, 0 );
}


//------------------------------------------------------------------
/**
 * 汎用コントロール処理
 *
 * @param   wk    描画処理ワーク
 * @param   sp    処理中の文字列ポインタ（=SPCODE_TAG_START_をポイントしている状態で呼ぶこと）
 *
 * @retval  const STRCODE*    処理後の文字列ポインタ
 */
//------------------------------------------------------------------
static const STRCODE* ctrlGeneralTag( PRINT_JOB* wk, const STRCODE* sp )
{
  switch( STR_TOOL_GetTagNumber(sp) ){
  case PRINTSYS_CTRL_GENERAL_COLOR:
    {
      u8 colL, colS, colB;

      colL = STR_TOOL_GetTagParam(sp, 0);
      colS = STR_TOOL_GetTagParam(sp, 1);
      colB = STR_TOOL_GetTagParam(sp, 2);

      GFL_FONTSYS_SetColor( colL, colS, colB );
    }
    break;

  case PRINTSYS_CTRL_GENERAL_RESET_COLOR:
    {
      u8 colL, colS, colB;
      PRINTSYS_LSB_GetLSB( wk->defColor, &colL, &colS, &colB );
      GFL_FONTSYS_SetColor( colL, colS, colB );
    }
    break;

  case PRINTSYS_CTRL_GENERAL_X_RIGHTFIT:
    {
      int areaWidth, strWidth;

      areaWidth = GFL_BMP_GetSizeX( wk->dst ) - wk->org_x;
      strWidth = get_line_width( sp, wk->fontHandle, 0, NULL );
      wk->write_x = wk->org_x + (areaWidth - strWidth) - 1;
    }
    break;

  case PRINTSYS_CTRL_GENERAL_X_CENTERING:
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

  case PRINTSYS_CTRL_GENERAL_X_ADD:
    wk->write_x += STR_TOOL_GetTagParam( sp, 0 );
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
 * @param   wk    描画処理ワーク
 * @param   sp    処理中の文字列ポインタ（=SPCODE_TAG_START_をポイントしている状態で呼ぶこと）
 *
 * @retval  const STRCODE*    処理後の文字列ポインタ
 */
//------------------------------------------------------------------
static const STRCODE* ctrlSystemTag( PRINT_JOB* wk, const STRCODE* sp )
{
  u16 ctrlCode = STR_TOOL_GetTagNumber(sp);

  switch( ctrlCode ){
  case CTRL_SYSTEM_COLOR:
    {
      u8  colorIdx;
      u8  bgcolor;

      colorIdx = STR_TOOL_GetTagParam( sp, 0 );
      bgcolor = PRINTSYS_LSB_GetB( wk->defColor );
      GFL_FONTSYS_SetColor( colorIdx*2+1, colorIdx*2+2, bgcolor );
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
 * @param   dst             描画先Bitmap
 * @param   xpos            描画開始Ｘ座標（ドット）
 * @param   ypos            描画開始Ｙ座標（ドット）
 * @param   str             文字列
 * @param   font            フォントタイプ
 * @param   wait            １文字描画ごとのウェイトフレーム数
 * @param   tcbsys          使用するGFL_TCBLシステムポインタ
 * @param   tcbpri          使用するタスクプライオリティ
 * @param   clearColorIdx   描画クリア用の色インデックス
 * @param   heapID          使用するヒープID
 *
 * @retval  PRINT_STREAM* ストリームハンドル
 *
 */
//==============================================================================================
PRINT_STREAM* PRINTSYS_PrintStream(
    GFL_BMPWIN* dst, s16 xpos, s16 ypos, const STRBUF* str, GFL_FONT* font,
    int wait, GFL_TCBLSYS* tcbsys, u32 tcbpri, HEAPID heapID, u16 clearColorIdx )
{
  return PRINTSYS_PrintStreamCallBack(
    dst, xpos, ypos, str, font, wait, tcbsys, tcbpri, heapID, clearColorIdx, NULL
  );
}

//==============================================================================================
/**
 * プリントストリーム作成（コールバックあり）
 *
 * @param   dst             描画先Bitmap
 * @param   xpos            描画開始Ｘ座標（ドット）
 * @param   ypos            描画開始Ｙ座標（ドット）
 * @param   str             文字列
 * @param   font            フォントタイプ
 * @param   wait            １文字描画ごとのウェイトフレーム数
 * @param   tcbsys          使用するGFL_TCBLシステムポインタ
 * @param   tcbpri          使用するタスクプライオリティ
 * @param   heapID          使用するヒープID
 * @param   clearColorIdx   描画クリア用の色インデックス
 * @param   callback        １文字描画ごとのコールバック関数アドレス
 *
 * @retval  PRINT_STREAM* ストリームハンドル
 *
 */
//==============================================================================================
PRINT_STREAM* PRINTSYS_PrintStreamCallBack(
    GFL_BMPWIN* dst, s16 xpos, s16 ypos, const STRBUF* str, GFL_FONT* font,
    int wait, GFL_TCBLSYS* tcbsys, u32 tcbpri, HEAPID heapID, u16 clearColorIdx, pPrintCallBack callback )
{
  PRINT_STREAM* stwk;
  GFL_TCBL* tcb;
  GFL_BMP_DATA* dstBmp;

  dstBmp = GFL_BMPWIN_GetBmp( dst );;
  tcb = GFL_TCBL_Create( tcbsys, print_stream_task, sizeof(PRINT_STREAM), tcbpri );
  stwk = GFL_TCBL_GetWork( tcb );
  stwk->tcb = tcb;

  printJob_setup( &stwk->printJob, font, dstBmp, xpos, ypos );

  if( wait >= 0 )
  {
    stwk->putPerFrame = 1;
  }
  else
  {
    stwk->putPerFrame = -wait;
    wait = 0;
  }

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
  stwk->clearColor = clearColorIdx;
  stwk->pauseType = PRINTSTREAM_PAUSE_CLEAR;
  stwk->pauseWait = 0;
  stwk->pauseReleaseFlag = FALSE;
  stwk->stopFlag = FALSE;
  stwk->callbackResult = FALSE;

  return stwk;
}
//=============================================================================================
/**
 * プリントストリームをユーザプログラムが強制的に一時停止する
 *
 * @param   handle
 */
//=============================================================================================
void PRINTSYS_PrintStreamStop( PRINT_STREAM* handle )
{
  handle->stopFlag = TRUE;
}
//=============================================================================================
/**
 * プリントストリームのユーザ強制一時停止を解除する
 *
 * @param   handle
 */
//=============================================================================================
void PRINTSYS_PrintStreamRun( PRINT_STREAM* handle )
{
  handle->stopFlag = FALSE;
}
//==============================================================================================
/**
 * プリントストリーム状態取得
 *
 * @param   handle    ストリームハンドラ
 *
 * @retval  PRINTSTREAM_STATE   状態
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
 * @param   handle    ストリームハンドラ
 *
 * @retval  PRINTSTREAM_PAUSE_TYPE    停止タイプ
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
 * @param   handle    ストリームハンドル
 * @param   wait    設定するウェイトフレーム数
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

  if( wk->callback_func != NULL && wk->callbackResult ){
    wk->callbackResult = wk->callback_func( wk->arg );
    if( wk->callbackResult ){
      return;
    }
  }

  if( wk->stopFlag ){
    return;
  }

  switch( wk->state ){
  case PRINTSTREAM_STATE_RUNNING:
    if( wk->wait == 0 )
    {
      int i;
      for(i=0; i<wk->putPerFrame; ++i)
      {
        switch( *(wk->sp) ){
        case EOM_CODE:
          wk->state = PRINTSTREAM_STATE_DONE;
          break;

        case SPCODE_TAG_START_:
          if( STR_TOOL_GetTagGroup(wk->sp) == TAGTYPE_STREAM_CTRL )
          {
            ctrlStreamTag( wk );
            if( wk->state != PRINTSTREAM_STATE_RUNNING ){
              i = wk->putPerFrame;  // for loop out
            }
            break;
          }
          /* fallthru */
        default:
          wk->sp = print_next_char( &wk->printJob, wk->sp );
          if( *(wk->sp) == EOM_CODE )
          {
            wk->state = PRINTSTREAM_STATE_DONE;
          }
          else
          {
            wk->wait = wk->current_wait;
          }

          GFL_BMPWIN_TransVramCharacter( wk->dstWin );
          if( wk->callback_func )
          {
            wk->callbackResult = wk->callback_func( wk->arg );
            wk->arg = wk->current_arg;
          }
          break;
        }
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
      case PRINTSTREAM_PAUSE_LINEFEED:
        if( wk->pauseWait < LINE_DOT_HEIGHT )
        {
          u16 size_x, size_y, feed_speed;

          feed_speed = LINE_FEED_SPEED;
          if( (wk->pauseWait+feed_speed) > LINE_DOT_HEIGHT ){
            feed_speed = (LINE_DOT_HEIGHT - wk->pauseWait);
          }
          wk->pauseWait += feed_speed;

          size_x = GFL_BMP_GetSizeX( wk->dstBmp );
          size_y = GFL_BMP_GetSizeY( wk->dstBmp );
          GFL_BMP_Print( wk->dstBmp, wk->dstBmp, 0, feed_speed, 0, 0, size_x, size_y-feed_speed, GF_BMPPRT_NOTNUKI );
          GFL_BMP_Fill( wk->dstBmp, 0, size_y-feed_speed, size_x, feed_speed, wk->clearColor );
          GFL_BMPWIN_TransVramCharacter( wk->dstWin );
        }else{
          wk->printJob.write_x = wk->printJob.org_x;
          wk->printJob.write_y = LINE_DOT_HEIGHT;
          wk->state = PRINTSTREAM_STATE_RUNNING;
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
  BOOL skipCR = FALSE;  // 直後の改行を無視する

  switch( STR_TOOL_GetTagNumber(wk->sp) ){
  case PRINTSYS_CTRL_STREAM_LINE_FEED:
    wk->state = PRINTSTREAM_STATE_PAUSE;
    wk->pauseType = PRINTSTREAM_PAUSE_LINEFEED;
    wk->pauseWait = 0;
    wk->pauseReleaseFlag = FALSE;
    wk->printJob.write_x = wk->printJob.org_x;
    wk->printJob.write_y = 0;
    skipCR = TRUE;
    break;
  case PRINTSYS_CTRL_STREAM_PAGE_CLEAR:
    wk->state = PRINTSTREAM_STATE_PAUSE;
    wk->pauseType = PRINTSTREAM_PAUSE_CLEAR;
    wk->pauseWait = 0;
    wk->pauseReleaseFlag = FALSE;
    wk->printJob.write_x = wk->printJob.org_x;
    wk->printJob.write_y = 0;
    skipCR = TRUE;
    break;
  case PRINTSYS_CTRL_STREAM_CHANGE_WAIT:
    wk->wait = STR_TOOL_GetTagParam( wk->sp, 0 );
    break;
  case PRINTSYS_CTRL_STREAM_SET_WAIT:
    wk->current_wait = STR_TOOL_GetTagParam( wk->sp, 0 );;
    break;
  case PRINTSYS_CTRL_STREAM_RESET_WAIT:  ///< 描画ウェイトをデフォルトに戻す
    wk->current_wait = wk->org_wait;
    break;
  case PRINTSYS_CTRL_STREAM_CHANGE_ARGV: ///< コールバック引数を変更（１回）
    wk->arg = STR_TOOL_GetTagParam( wk->sp, 0 );;
    break;
  case PRINTSYS_CTRL_STREAM_SET_ARGV:    ///< コールバック引数を変更（永続）
    wk->current_arg = STR_TOOL_GetTagParam( wk->sp, 0 );
    wk->arg = STR_TOOL_GetTagParam( wk->sp, 0 );
    break;
  case PRINTSYS_CTRL_STREAM_RESET_ARGV:  ///< コールバック引数をデフォルトに戻す
    wk->current_arg = wk->org_arg;
    wk->arg = wk->org_arg;
    break;
  case PRINTSYS_CTRL_STREAM_FORCE_CLEAR:
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
 * @param   sp      文字列ポインタ
 * @param   font    フォントタイプ
 * @param   margin    字間スペース（ドット）
 * @param   endPtr    [out] 次行の先頭ポインタを格納するポインタアドレス（NULLなら無視する）
 *
 * @retval  u32   ドット数
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

//=============================================================================================
/**
 * 文字列の行数を返す
 *
 * @param   str
 *
 * @retval  u32
 */
//=============================================================================================
u32 PRINTSYS_GetLineCount( const STRBUF* str )
{
  const STRCODE* sp = GFL_STR_GetStringCodePointer( str );
  u32 count = 1;

  while( *sp != EOM_CODE )
  {
    if( *sp != SPCODE_TAG_START_ )
    {
      if( *sp == CR_CODE ){
        ++count;
      }
      ++sp;
    }
    else
    {
      sp = STR_TOOL_SkipTag( sp );
    }
  }

  return count;
}

//==============================================================================================
/**
 * 文字列をBitmap表示する際の幅（ドット数）を計算
 * ※文字列が複数行ある場合、その最長行のドット数
 *
 * @param   str     文字列
 * @param   font    フォントタイプ
 * @param   margin    字間スペース（ドット）
 *
 * @retval  u32     ドット数
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
u32 PRINTSYS_GetStrLineWidth( const STRBUF* str, GFL_FONT* font, u16 margin, u32* dst, u32 dstElems )
{
  const STRCODE* sp;
  u32 cnt;

  sp = GFL_STR_GetStringCodePointer( str );
  cnt = 0;
  while( cnt < dstElems )
  {
    if( *sp != EOM_CODE )
    {
      dst[cnt++] = get_line_width( sp, font, margin, &sp );
    }
    else
    {
      break;
    }
  }
  return cnt;
}

//=============================================================================================
/**
 * 複数行ある文字列から指定行の文字列のみを取り出す
 * ※ローカライズ注意！　使用は許可制です。※
 *
 * @param   src     読み取り元の文字列
 * @param   dst     読み取り先バッファ
 * @param   line    行番号（0～）
 *
 * @retval  BOOL    指定 line が大きすぎて読み取れない場合FALSE／それ以外はTRUE
 */
//=============================================================================================
BOOL PRINTSYS_CopyLineStr( const STRBUF* src, STRBUF* dst, u32 line )
{
  const STRCODE* sp = GFL_STR_GetStringCodePointer( src );

  GFL_STR_ClearBuffer( dst );

  while( line )
  {
    while( *sp != EOM_CODE && *sp != CR_CODE )
    {
      if( *sp != SPCODE_TAG_START_ ){
        ++sp;
      }else{
        sp = STR_TOOL_SkipTag( sp );
      }
    }
    --line;
    if( *sp == EOM_CODE ){
      break;
    }
    else{
      ++sp;
    }
  }

  if( (line == 0) && (*sp != EOM_CODE) )
  {
    while( *sp != EOM_CODE && *sp != CR_CODE )
    {
      if( *sp != SPCODE_TAG_START_ )
      {
        GFL_STR_AddCode( dst, *sp++ );
      }
      else
      {
        const STRCODE* next = STR_TOOL_SkipTag( sp );
        while( sp < next ){
          GFL_STR_AddCode( dst, *sp++ );
        }
      }
    }
    GFL_STR_AddCode( dst, EOM_CODE );
    return TRUE;
  }

  return FALSE;
}

//=============================================================================================
/**
 * 文字列をBitmap表示する際の高さ（ドット数）を計算
 *
 * @param   str
 * @param   font
 *
 * @retval  u32     ドット数
 */
//=============================================================================================
u32 PRINTSYS_GetStrHeight( const STRBUF* str, GFL_FONT* font )
{
  const STRCODE* sp;
  u16 numCR;

  sp = GFL_STR_GetStringCodePointer( str );
  numCR = 0;

  while( *sp != EOM_CODE )
  {
    if( *sp == CR_CODE )
    {
      ++numCR;
    }
    if( *sp == SPCODE_TAG_START_ )
    {
      sp = STR_TOOL_SkipTag( sp );
    }
    else
    {
      ++sp;
    }
  }

  return GFL_FONT_GetLineHeight( font ) * (numCR + 1);
}


//=============================================================================================
/**
 * 文字列の中に含まれる、単語タグの数をカウントして返す
 *
 * @param   str   文字列
 *
 * @retval  u16   単語タグの数
 */
//=============================================================================================
u16 PRINTSYS_GetTagCount( const STRBUF* str )
{
  const STRCODE* sp;
  u16 cnt = 0;

  sp = GFL_STR_GetStringCodePointer( str );

  while( *sp != EOM_CODE )
  {
    if( *sp == SPCODE_TAG_START_ )
    {
      if( PRINTSYS_IsWordSetTagGroup(sp) )
      {
        cnt++;
      }
      sp = PRINTSYS_SkipTag( sp );
    }
    sp++;
  }

  return cnt;
}

//=============================================================================================
/**
 * 指定したタグ（展開前）が文字列中何行目にあるかを返す
 *
 * @param   str     文字列
 * @param   tag_id  タグインデックス（バッファ番号）
 *
 * @retval  u8      行数
 *
 * @note  add by genya hosaka
 */
//=============================================================================================
u8 PRINTSYS_GetTagLine( const STRBUF* str, u8 tag_id )
{
  const STRCODE* sp;
  u16 line = 0;

  sp = GFL_STR_GetStringCodePointer( str );

#if 0
  HOSAKA_Printf("================\n");
  // 一端全てプリント
  while( *sp != EOM_CODE )
  {
    HOSAKA_Printf("tag_id=%d sp=%x \n", tag_id, *sp);
    sp++;
  }
  HOSAKA_Printf("================\n");
#endif

  sp = GFL_STR_GetStringCodePointer( str );

  while( *sp != EOM_CODE )
  {
//  HOSAKA_Printf("tag_id=%d sp=%x \n", tag_id, *sp);

    // 改行カウント
    if( *sp == CR_CODE )
    {
      line++;
    }

    // タグ検出
    if( *sp == SPCODE_TAG_START_ )
    {
      if( PRINTSYS_IsWordSetTagGroup(sp) )
      {
        if( STR_TOOL_GetTagParam(sp, 0) == tag_id )
        {
          return line;
        }
      }
      // タグ直後の文字を頭だし
      sp = PRINTSYS_SkipTag( sp );
    }
    else
    {
      // 次の文字を参照
      sp++;
    }

  }

  GF_ASSERT( 0 ); ///< 指定タグは見つからなかった

  return 0;
}

//=============================================================================================
/**
 * 指定したタグ（展開前）が何ドット目にあるかを返す
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
u8 PRINTSYS_GetTagWidth( const STRBUF* str, u8 tag_id, GFL_FONT* font, u16 margin )
{
  const STRCODE* sp;
  u32 width = 0;

  sp = GFL_STR_GetStringCodePointer( str );

  while( *sp != EOM_CODE )
  {
    // 改行コードがきたら一端リセット
    if( *sp == CR_CODE )
    {
      width = 0;
      sp++;
    }
    // タグ以外ならドット数計上
    else if( *sp != SPCODE_TAG_START_ )
    {
      width += ( GFL_FONT_GetWidth( font, *sp ) + margin );
//    HOSAKA_Printf("[%d] sp=%x width=%d \n",tag_id, *sp, width );
      sp++;
    }
    else
    {
      if( PRINTSYS_IsWordSetTagGroup(sp) )
      {
        if( STR_TOOL_GetTagParam(sp, 0) == tag_id )
        {
          return width;
        }
        sp = STR_TOOL_SkipTag( sp );
      }
    }
  }

  GF_ASSERT(0);

  return 0;
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
 * @param   sp    文字列ポインタ（タグ開始コードをポイントしていること）
 *
 * @retval  BOOL    単語セット用タグコードならTRUE
 */
//=============================================================================================
BOOL PRINTSYS_IsWordSetTagGroup( const STRCODE* sp )
{
  u16 type = STR_TOOL_GetTagGroup( sp );
  return (type == TAGTYPE_WORD) || (type==TAGTYPE_NUMBER);
}


//=============================================================================================
/**
 * タググループ取得
 *
 * @param   sp    文字列ポインタ（タグ開始コードをポイントしていること）
 *
 * @retval  PrintSysTagGroup   タググループ（gmm の ポケモンWB（単語），ポケモンWB（数値）等）
 */
//=============================================================================================
PrintSysTagGroup PRINTSYS_GetTagGroup( const STRCODE* sp )
{
  return STR_TOOL_GetTagGroup( sp );
}

//=============================================================================================
/**
 * タグコードを文字列化してバッファに格納
 *
 * @param   str         [out] 格納先バッファ
 * @param   tagGrp
 * @param   tagIdx
 * @param   numParams
 * @param   params
 *
 */
//=============================================================================================
void PRINTSYS_CreateTagCode( STRBUF* str, PrintSysTagGroup tagGrp, u8 tagIdx, u8 numParams, const u16* params )
{
  GFL_STR_AddCode( str, SPCODE_TAG_START_ );
  GFL_STR_AddCode( str, STR_TOOL_CreateTagCode(tagGrp, tagIdx) );
  GFL_STR_AddCode( str, numParams );
  {
    u8 i;
    for(i=0; i<numParams; ++i){
      GFL_STR_AddCode( str, params[i] );
    }
  }
}

//=============================================================================================
/**
 * タグインデックス取得
 *
 * @param   sp    文字列ポインタ（タグ開始コードをポイントしていること）
 *
 * @retval  u16   タグインデックス
 */
//=============================================================================================
u8 PRINTSYS_GetTagIndex( const STRCODE* sp )
{
  return STR_TOOL_GetTagIndex( sp );
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






//----------------------------------------------------------------------------------
/**
 * タグ開始コードを指している文字列から、そのタグのグループを取得
 *
 * @param   sp    文字列ポインタ（タグ開始コードを指している必要がある）
 *
 * @retval  u8    タググループ
 */
//----------------------------------------------------------------------------------
static inline u8 STR_TOOL_GetTagGroup( const STRCODE* sp )
{
  GF_ASSERT( *sp == SPCODE_TAG_START_ );

  if( *sp == SPCODE_TAG_START_ )
  {
    sp++;
    return (((*sp)>>8)&0xff);
  }

  return 0xffff;
}
//----------------------------------------------------------------------------------
/**
 * タグ開始コードを指している文字列から、そのタグのグループ内インデックスを返す
 *
 * @param   sp    文字列ポインタ（タグ開始コードを指している必要がある）
 *
 * @retval  u8    グループ内インデックス
 */
//----------------------------------------------------------------------------------
static inline u8 STR_TOOL_GetTagIndex( const STRCODE* sp )
{
  GF_ASSERT( *sp == SPCODE_TAG_START_ );

  if( *sp == SPCODE_TAG_START_ )
  {
    sp++;
    return ((*sp)&0xff);
  }

  return 0xffff;
}
//----------------------------------------------------------------------------------
/**
 * タググループ、グループ内インデックスからタグコードを生成
 *
 * @param   grp
 * @param   index
 *
 * @retval  inline STRCODE
 */
//----------------------------------------------------------------------------------
static inline STRCODE STR_TOOL_CreateTagCode( PrintSysTagGroup grp, u8 index )
{
  return (grp & 0xff) << 8 | index;
}

//----------------------------------------------------------------------------------
/**
 * タグ開始コードを指している文字列から、そのタグのタイプを取得
 *
 * @param   sp            文字列ポインタ（タグ開始コードを指している必要がある）
 * @param   paramIndex    パラメータインデックス
 *
 * @retval  u16   タグタイプ
 */
//----------------------------------------------------------------------------------
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
 * @param   sp    文字列ポインタ（タグコード開始値を指していること）
 *
 * @retval  const STRCODE*    スキップ後のポインタ
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
 * @param   sp    文字列ポインタ（タグ開始コードを指している必要がある）
 *
 * @retval  u16   タグ種類
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
 * @param   str   [in] 文字列格納バッファ
 *
 * @retval  u32   バッファサイズ（byte）
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
 * @param   buf   [in] 書き込む文字列が格納されている文字列バッファ
 *
 * @retval  u32   プリントキューが必要とするバッファサイズ（byte）
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
 * @param   que   [in] プリントキュー
 *
 * @retval  u32   空きサイズ（byte）
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
 * @param   que     [out] プリントキュー
 * @param   printJob  [in] 内容設定済みの描画処理ワーク
 * @param   str     [in] 描画文字列バッファ
 *
 */
//--------------------------------------------------------------------------
static void Que_StoreNewJob( PRINT_QUE* que, PRINT_JOB* printJob, const STRBUF* str )
{
  PRINT_JOB* store_adrs;
  const STRCODE* sp;

  sp = Que_WriteDataUnit( que,
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
 * @param   que       [io] プリントキュー （内部書き込みポインタが移動する）
 * @param   src       [in] 書き込むデータポインタ
 * @param   totalSize   [in] 書き込みデータ総サイズ
 * @param   unitSize    [in] アドレス分断せずに書き込む単位
 *
 * @retval  void*     格納先頭アドレス
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
 * @param   que   [in] プリントキュー
 * @param   job   [in] プリントキューのバッファに格納されている Job アドレス
 *
 * @retval  PRINT_JOB   次のJobアドレス（無ければNULL）
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
 * @param   que   [out] プリントキュー
 * @param   src   [in]  書き込みデータポインタ
 * @param   pos   [in]  バッファ書き込みポインタ
 * @param   size  [in]  書き込みデータサイズ
 *
 * @retval  u16   次に書き込むときのバッファ書き込みポインタ
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
 * @param   que       [in] プリントキュー
 * @param   bufAdrs     [in] リングバッファ中の任意のアドレス
 *
 * @retval  u16       書き込みポインタ
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
 * @param   que       [in] プリントキュー
 * @param   bufPos      [in] リングバッファ書き込みポインタ
 *
 * @retval  void*     アドレス
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
 * @param   que     プリントキュー
 * @param   pos     書き込みポインタ初期値
 * @param   fwdSize   進めるサイズ（バイト単位）
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

