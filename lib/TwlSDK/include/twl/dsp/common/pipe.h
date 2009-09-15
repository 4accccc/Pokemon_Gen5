/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - dsp - common
  File:     pipe.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-21#$
  $Rev: 9018 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#ifndef TWL_DSP_PIPE_H_
#define TWL_DSP_PIPE_H_


#include <twl/dsp/common/byteaccess.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* constants */

// IN/OUT定義用の定数。(原則的に 0 : DSP->ARM, 1 : ARM->DSP と定めておく)
#ifdef SDK_TWL
#define DSP_PIPE_INPUT      0
#define DSP_PIPE_OUTPUT     1
#else
#define DSP_PIPE_INPUT      1
#define DSP_PIPE_OUTPUT     0
#endif
#define DSP_PIPE_PEER_MAX   2

// システムリソース
#define DSP_PIPE_PORT_MAX               8   // 使用可能なパイプの最大ポート数
#define DSP_PIPE_DEFAULT_BUFFER_LENGTH  256 // デフォルトのリングバッファサイズ

// 定義済みポート
#define DSP_PIPE_CONSOLE    0   // DSP->ARM: デバッグコンソール
#define DSP_PIPE_DMA        1   // DSP<->ARM: 擬似DMA
#define DSP_PIPE_AUDIO      2   // DSP<->ARM: オーディオ汎用通信
#define DSP_PIPE_BINARY     3   // DSP<->ARM: 汎用バイナリ
#define DSP_PIPE_EPHEMERAL  4   // DSP_CreatePipe()で確保可能な空き領域

#define DSP_PIPE_FLAG_INPUT     0x0000  // 入力側
#define DSP_PIPE_FLAG_OUTPUT    0x0001  // 出力側
#define DSP_PIPE_FLAG_PORTMASK  0x00FF  // ポート番号負フィールド
#define DSP_PIPE_FLAG_BOUND     0x0100  // Openされている
#define DSP_PIPE_FLAG_EOF       0x0200  // EOF

#define DSP_PIPE_FLAG_EXIT_OS   0x8000  // DSPのAHBマスタ終了処理

#define DSP_PIPE_COMMAND_REGISTER   2

// DSPファイルIO用のコマンド構造体。
#define DSP_PIPE_IO_COMMAND_OPEN    0
#define DSP_PIPE_IO_COMMAND_CLOSE   1
#define DSP_PIPE_IO_COMMAND_SEEK    2
#define DSP_PIPE_IO_COMMAND_READ    3
#define DSP_PIPE_IO_COMMAND_WRITE   4
#define DSP_PIPE_IO_COMMAND_MEMMAP  5

#define DSP_PIPE_IO_MODE_R          0x0001
#define DSP_PIPE_IO_MODE_W          0x0002
#define DSP_PIPE_IO_MODE_RW         0x0004
#define DSP_PIPE_IO_MODE_TRUNC      0x0008
#define DSP_PIPE_IO_MODE_CREATE     0x0010

#define DSP_PIPE_IO_SEEK_SET        0
#define DSP_PIPE_IO_SEEK_CUR        1
#define DSP_PIPE_IO_SEEK_END        2


/*---------------------------------------------------------------------------*/
/* declarations */

// パイプ構造体。
// DSPからARMへの直接アクセス手段が存在しないため
// 基本的にARMからAPBP-FIFOでDSP内のバッファを制御する。
// 特に指定が無ければリングバッファとして使用する。
typedef struct DSPPipe
{
    DSPAddr address;    // バッファの先頭アドレス
    DSPByte length;     // バッファサイズ
    DSPByte rpos;       // 未読先頭領域
    DSPByte wpos;       // 追記終端領域
    u16     flags;      // 属性フラグ
}
DSPPipe;

// DSPが保持してARMが参照するパイプ情報。
typedef struct DSPPipeMonitor
{
    DSPPipe pipe[DSP_PIPE_PORT_MAX][DSP_PIPE_PEER_MAX];
}
DSPPipeMonitor;

// DSPパイプ通信ハンドラ。
typedef void (*DSPPipeCallback)(void *userdata, int port, int peer);


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         DSP_InitPipe

  Description:  DSPパイプ通信の初期化。
                DSPコマンドリプライレジスタ2を占有する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_InitPipe(void);

/*---------------------------------------------------------------------------*
  Name:         DSP_SetPipeCallback

  Description:  DSPパイプ通信のコールバックを設定。

  Arguments:    port     : パイプのポート番号。
                callback : Readable/Writableイベント時のコールバック。
                userdata : 任意のユーザ定義引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SetPipeCallback(int port, void (*callback)(void *, int, int), void *userdata);

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadPipe

  Description:  DSPパイプ情報をロード。

  Arguments:    pipe : パイプ情報の格納先 (DSP側ではNULLでよい)
                port : パイプのポート番号
                peer : DSP_PIPE_INPUT または DSP_PIPE_OUTPUT

  Returns:      ロードしたパイプ情報のポインタ
 *---------------------------------------------------------------------------*/
DSPPipe* DSP_LoadPipe(DSPPipe *pipe, int port, int peer);

/*---------------------------------------------------------------------------*
  Name:         DSP_SyncPipe

  Description:  DSPパイプ情報を最新の内容へ更新。

  Arguments:    pipe : パイプ情報

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SyncPipe(DSPPipe *pipe);

/*---------------------------------------------------------------------------*
  Name:         DSP_FlushPipe

  Description:  DSPパイプのストリームをフラッシュ。

  Arguments:    pipe : パイプ情報

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_FlushPipe(DSPPipe *pipe);

/*---------------------------------------------------------------------------*
  Name:         DSP_GetPipeReadableSize

  Description:  指定のDSPパイプから現在読み出し可能な最大サイズを取得。

  Arguments:    pipe : パイプ情報

  Returns:      現在読み出し可能な最大サイズ。
 *---------------------------------------------------------------------------*/
u16 DSP_GetPipeReadableSize(const DSPPipe *pipe);

/*---------------------------------------------------------------------------*
  Name:         DSP_GetPipeWritableSize

  Description:  指定のDSPパイプへ現在書き込み可能な最大サイズを取得。

  Arguments:    pipe : パイプ情報

  Returns:      現在書き込み可能な最大サイズ。
 *---------------------------------------------------------------------------*/
u16 DSP_GetPipeWritableSize(const DSPPipe *pipe);

/*---------------------------------------------------------------------------*
  Name:         DSP_ReadPipe

  Description:  DSPパイプ通信ポートからデータを読み込み。

  Arguments:    pipe   : パイプ情報
                buffer : 転送先バッファ
                length : 転送サイズ。(ただし単位はその環境のワードサイズ)
                         ARM側は1バイト単位、DSP側は2バイト単位となる点に注意。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ReadPipe(DSPPipe *pipe, void *buffer, u16 length);

/*---------------------------------------------------------------------------*
  Name:         DSP_WritePipe

  Description:  DSPパイプ通信ポートへデータを書き込み。

  Arguments:    pipe   : パイプ情報
                buffer : 転送元バッファ
                length : 転送サイズ。(ただし単位はその環境のワードサイズ)
                         ARM側は1バイト単位、DSP側は2バイト単位となる点に注意。
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_WritePipe(DSPPipe *pipe, const void *buffer, u16 length);

/*---------------------------------------------------------------------------*
  Name:         DSP_HookPipeNotification

  Description:  DSP割り込み内で呼び出すべきパイプ通知フック。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_HookPipeNotification(void);

#ifdef SDK_TWL

#else

/*---------------------------------------------------------------------------*
  Name:         DSP_Printf

  Description:  DSPパイプのコンソールポートへ文字列を出力。

  Arguments:    format : 書式文字列

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_Printf(const char *format, ...);

int DSP_OpenFile(const char *path, int mode);
int DSP_OpenMemoryFile(DSPAddrInARM address, DSPWord32 length);
void DSP_CloseFile(int port);
s32 DSP_GetFilePosition(int port);
s32 DSP_GetFileLength(int port);
s32 DSP_SeekFile(int port, s32 offset, int whence);
s32 DSP_ReadFile(int port, void *buffer, DSPWord length);
s32 DSP_WriteFile(int port, const void *buffer, DSPWord length);

// C標準関数の置き換え例。
#if 0
typedef void FILE;
#define fopen(path, mode)           (FILE*)DSP_OpenFile(path, mode)
#define fclose(f)                   DSP_CloseFile((int)f)
#define fseek(f, ofs, whence)       DSP_SeekFile((int)f, ofs, whence)
#define fread(buf, len, unit, f)    DSP_ReadFile((int)f, buf, (len) * (unit))
#define fwrite(buf, len, unit, f)   DSP_WriteFile((int)f, buf, (len) * (unit))
#define rewind(f)                   (void)DSP_SeekFile((int)f, 0, DSP_PIPE_IO_SEEK_SET)
#define ftell(f)                    (void)DSP_SeekFile((int)f, 0, DSP_PIPE_IO_SEEK_CUR)
#define fgetpos(f, ppos)            (((*(ppos) = ftell((int)f)) != -1) ? 0 : -1)
#define fsetpos(f, ppos)            fseek((int)f, *(ppos), DSP_PIPE_IO_SEEK_SET)
#endif

#endif


/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_DSP_PIPE_H_ */
