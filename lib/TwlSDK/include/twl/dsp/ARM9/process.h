/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - dsp
  File:     dsp_process.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-21#$
  $Rev: 9018 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#ifndef TWL_DSP_PROCESS_H_
#define TWL_DSP_PROCESS_H_


/*---------------------------------------------------------------------------*
 * このモジュールはTWL限定。
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL


#include <twl/dsp.h>
#include <twl/dsp/common/byteaccess.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* constants */

// WRAM-B/Cスロットサイズ。
#define DSP_WRAM_SLOT_SIZE (32 * 1024)

// フック登録に使用するビット定義マクロ。
#define DSP_HOOK_SEMAPHORE_(id) (1 << (id))
#define DSP_HOOK_REPLY_(id)     (1 << (16 + (id)))
#define DSP_HOOK_MAX            (16 + 3)

// 起動時にSDK固有の同期処理を使用するかどうかのフラグ。
#define DSP_PROCESS_FLAG_USING_OS   0x0001

/*---------------------------------------------------------------------------*/
/* declarations */

// DSP割り込みフック関数プロトタイプ。
typedef void (*DSPHookFunction)(void *userdata);

// プロセスメモリ情報構造体。
// DSPプロセスのロード手順は以下の通り。
//   (1) プロセスイメージのセクションテーブルを列挙して
//       占有されるセグメント配置状況(セグメントマップ)を算出。
//   (2) DSPに使用を許されたWRAM-B/Cのスロット群のうち
//       実際にDSPセグメントとして使用するものを選定し、
//       各セグメントとの対応表(スロットマップ)を決定。
//   (3) 再びプロセスイメージのセクションテーブルを列挙して
//       今度は実際にプロセスイメージをWRAM-B/C上へ配置する。
typedef struct DSPProcessContext
{
    // マルチプロセス対応のための単方向リストとプロセス名。
    struct DSPProcessContext   *next;
    char                        name[15 + 1];
    // プロセスイメージの格納されたファイルハンドルと利用可能なWRAM。
    // ロード処理の間だけ一時的に使用する。
    FSFile                     *image;
    u16                         slotB;
    u16                         slotC;
    int                         flags;
    // プロセスが占有するDSP側のセグメントマップ。
    // DSP_MapProcessSegment関数が算出する。
    int                         segmentCode;
    int                         segmentData;
    // 各セグメントに割り当てられたスロットマップ。
    // DSP_StartupProcess関数でマッパーが配置する。
    int                         slotOfSegmentCode[8];
    int                         slotOfSegmentData[8];
    // DSPの各種割り込み要因に対して登録されたフック関数。
    // DSP_SetProcessHook関数で要因別に登録する。
    int                         hookFactors;
    DSPHookFunction             hookFunction[DSP_HOOK_MAX];
    void                       *hookUserdata[DSP_HOOK_MAX];
    int                         hookGroup[DSP_HOOK_MAX];
}
DSPProcessContext;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         DSP_StopDSPComponent

  Description:  DSPの終了準備を行う。
                現在はDSPのDMAを停止させているだけ。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_StopDSPComponent(void);

/*---------------------------------------------------------------------------*
  Name:         DSP_InitProcessContext

  Description:  プロセス情報構造体を初期化。

  Arguments:    context : DSPProcessContext構造体。
                name    : プロセス名またはNULL。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_InitProcessContext(DSPProcessContext *context, const char *name);

/*---------------------------------------------------------------------------*
  Name:         DSP_ExecuteProcess

  Description:  DSPプロセスイメージをロードして起動。

  Arguments:    context : 状態管理に使用するDSPProcessContext構造体。
                          プロセスを破棄するまでシステムによって参照される。
                image   : プロセスイメージを指すファイルハンドル。
                          この関数内でのみ参照される。
                slotB   : コードメモリのために使用を許可されたWRAM-B。
                slotC   : データメモリのために使用を許可されたWRAM-C。

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL DSP_ExecuteProcess(DSPProcessContext *context, FSFile *image, int slotB, int slotC);

/*---------------------------------------------------------------------------*
  Name:         DSP_QuitProcess

  Description:  DSPプロセスを終了してメモリを解放。

  Arguments:    context : 状態管理に使用するDSPProcessContext構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_QuitProcess(DSPProcessContext *context);

/*---------------------------------------------------------------------------*
  Name:         DSP_FindProcess

  Description:  実行中のプロセスを検索。

  Arguments:    name : プロセス名。
                       NULLを指定したら最後に登録したプロセスを検索。

  Returns:      指定した名前に合致するDSPProcessContext構造体。
 *---------------------------------------------------------------------------*/
DSPProcessContext* DSP_FindProcess(const char *name);

/*---------------------------------------------------------------------------*
  Name:         DSP_GetProcessSlotFromSegment

  Description:  指定したセグメント番号に該当するWRAMスロット番号を取得。

  Arguments:    context : DSPProcessContext構造体。
                wram    : MI_WRAM_B/MI_WRAM_C。
                segment : セグメント番号。

  Returns:      指定したセグメント番号に該当するWRAMスロット番号。
 *---------------------------------------------------------------------------*/
SDK_INLINE int DSP_GetProcessSlotFromSegment(const DSPProcessContext *context, MIWramPos wram, int segment)
{
    return (wram == MI_WRAM_B) ? context->slotOfSegmentCode[segment] : context->slotOfSegmentData[segment];
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ConvertProcessAddressFromDSP

  Description:  DSPメモリ空間のアドレスからWRAMのアドレスへ変換。

  Arguments:    context : DSPProcessContext構造体。
                wram    : MI_WRAM_B/MI_WRAM_C。
                address : DSPメモリ空間のアドレス。(DSPメモリ空間のワード単位)

  Returns:      指定したDSPアドレスに該当するWRAMアドレス。
 *---------------------------------------------------------------------------*/
SDK_INLINE void* DSP_ConvertProcessAddressFromDSP(const DSPProcessContext *context, MIWramPos wram, int address)
{
    int     segment = (address / (DSP_WRAM_SLOT_SIZE/2));
    int     mod = (address - segment * (DSP_WRAM_SLOT_SIZE/2));
    int     slot = DSP_GetProcessSlotFromSegment(context, wram, segment);
    SDK_ASSERT((slot >= 0) && (slot < MI_WRAM_B_MAX_NUM));
    return (u8*)MI_GetWramMapStart(wram) + slot * DSP_WRAM_SLOT_SIZE + mod * 2;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_AttachProcessMemory

  Description:  プロセスの未使用領域に連続したメモリ空間を割り当てる。

  Arguments:    context : DSPProcessContext構造体。
                wram    : 割り当てるメモリ空間。 (MI_WRAM_B/MI_WRAM_C)
                slots   : 割り当てるWRAMスロット。

  Returns:      割り当てられたDSPメモリ空間の先頭アドレスまたは0。
 *---------------------------------------------------------------------------*/
u32 DSP_AttachProcessMemory(DSPProcessContext *context, MIWramPos wram, int slots);

/*---------------------------------------------------------------------------*
  Name:         DSP_DetachProcessMemory

  Description:  プロセスの使用領域に割り当てられたWRAMスロットを解放する。

  Arguments:    context : DSPProcessContext構造体。
                slots   : 割り当て済みのWRAMスロット。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_DetachProcessMemory(DSPProcessContext *context, MIWramPos wram, int slots);

/*---------------------------------------------------------------------------*
  Name:         DSP_SwitchProcessMemory

  Description:  指定のプロセスが使用中のDSPアドレス管理を切り替える。

  Arguments:    context : DSPProcessContext構造体。
                wram    : 切り替えるメモリ空間。 (MI_WRAM_B/MI_WRAM_C)
                address : 切り替える先頭アドレス。(DSPメモリ空間のワード単位)
                length  : 切り替えるメモリサイズ。(DSPメモリ空間のワード単位)
                to      : 新しいマスタープロセッサ。

  Returns:      全てのスロットが正しく切り替えられればTRUE。
 *---------------------------------------------------------------------------*/
BOOL DSP_SwitchProcessMemory(DSPProcessContext *context, MIWramPos wram, u32 address, u32 length, MIWramProc to);

/*---------------------------------------------------------------------------*
  Name:         DSP_SetProcessHook

  Description:  プロセスへのDSP割り込み要因に対してフックを設定。

  Arguments:    context  : DSPProcessContext構造体。
                factors  : 指定する割り込み要因のビット集合。
                function : 呼び出すべきフック関数。
                userdata : フック関数に与える任意のユーザ定義引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SetProcessHook(DSPProcessContext *context, int factors, DSPHookFunction function, void *userdata);

/*---------------------------------------------------------------------------*
  Name:         DSPi_CreateMemoryFile

  Description:  静的なDSPプロセスイメージをメモリファイルへ変換。

  Arguments:    memfile : メモリファイルとなるFSFile構造体。
                image   : DSPプロセスイメージを保持するバッファ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL DSPi_CreateMemoryFile(FSFile *memfile, const void *image)
{
    if (!FS_IsAvailable())
    {
        OS_TWarning("FS is not initialized yet.\n");
        FS_Init(FS_DMA_NOT_USE);
    }
    FS_InitFile(memfile);
    return FS_CreateFileFromMemory(memfile, (void *)image, (u32)(16 * 1024 * 1024));
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ReadProcessPipe

  Description:  プロセスの所定ポートに関連付けられたパイプから受信。

  Arguments:    context : DSPProcessContext構造体。
                port    : 受信元のポート。
                buffer  : 受信データ。
                length  : 受信サイズ。(バイト単位)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ReadProcessPipe(DSPProcessContext *context, int port, void *buffer, u32 length);

/*---------------------------------------------------------------------------*
  Name:         DSP_WriteProcessPipe

  Description:  プロセスの所定ポートに関連付けられたパイプへ送信。

  Arguments:    context : DSPProcessContext構造体。
                port    : 送信先のポート。
                buffer  : 送信データ。
                length  : 送信サイズ。(バイト単位)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_WriteProcessPipe(DSPProcessContext *context, int port, const void *buffer, u32 length);

#if defined(DSP_SUPPORT_OBSOLETE_LOADER)
/*---------------------------------------------------------------------------*
 * 以下は現在使用されていないと思われる廃止候補インタフェース。
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         DSP_MapProcessSegment

  Description:  プロセスが占有するセグメントマップを算出。

  Arguments:    context : DSPProcessContext構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_MapProcessSegment(DSPProcessContext *context);

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadProcessImage

  Description:  指定されたプロセスイメージをロードする。
                セグメントマップはすでに算出済みでなければならない。

  Arguments:    context  : DSPProcessContext構造体。

  Returns:      全てのイメージが正しくロードされればTRUE。
 *---------------------------------------------------------------------------*/
BOOL DSP_LoadProcessImage(DSPProcessContext *context);

/*---------------------------------------------------------------------------*
  Name:         DSP_StartupProcess

  Description:  プロセスイメージのセグメントマップを算出してWRAMへロードする。
                DSP_MapProcessSegment関数とDSP_LoadProcessImage関数の複合版。

  Arguments:    context    : DSPProcessContext構造体。
                image      : プロセスイメージを指すファイルハンドル。
                             この関数内でのみ参照される。
                slotB      : コードメモリのために使用を許可されたWRAM-B。
                slotC      : データメモリのために使用を許可されたWRAM-C。
                slotMapper : セグメントにWRAMスロットを割り当てるアルゴリズム。
                             NULLを指定すればデフォルトの最適な処理が選択される。

  Returns:      全てのイメージが正しくロードされればTRUE。
 *---------------------------------------------------------------------------*/
BOOL DSP_StartupProcess(DSPProcessContext *context, FSFile *image,
                        int slotB, int slotC, BOOL (*slotMapper)(DSPProcessContext *, int, int));


#endif


#ifdef __cplusplus
} // extern "C"
#endif


#endif // SDK_TWL


/*---------------------------------------------------------------------------*/
#endif // TWL_DSP_PROCESS_H_
