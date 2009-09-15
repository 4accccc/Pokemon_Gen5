/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_rom.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_LIBRARIES_CARD_ROM_H__
#define NITRO_LIBRARIES_CARD_ROM_H__


#include <nitro.h>
#include "../include/card_common.h"


// #define SDK_ARM7_READROM_SUPPORT


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* declarations */

typedef struct CARDTransferInfo
{
    u32     command;
    void  (*callback)(void *userdata);
    void   *userdata;
    u32     src;
    u32     dst;
    u32     len;
    u32     work;
}
CARDTransferInfo;

typedef void (*CARDTransferCallbackFunction)(void *userdata);


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomWithCPU

  Description:  CPUを使用してROM転送。
                キャッシュやページ単位の制限を考慮する必要は無いが
                転送完了まで関数がブロッキングする点に注意。

  Arguments:    userdata          (他のコールバックとして使用するためのダミー)
                buffer            転送先バッファ
                offset            転送元ROMオフセット
                length            転送サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
int CARDi_ReadRomWithCPU(void *userdata, void *buffer, u32 offset, u32 length);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomWithDMA

  Description:  DMAを使用してROM転送。
                最初のページの転送を開始したら関数はただちに制御を返す。

  Arguments:    info              転送情報

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ReadRomWithDMA(CARDTransferInfo *info);

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitRom

  Description:  ROMアクセス管理情報を初期化。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_InitRom(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_CheckPulledOutCore

  Description:  カード抜け検出関数のメイン処理。
                カードバスはロックされている必要がある。

  Arguments:    id            カードから読み出された ROM-ID

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_CheckPulledOutCore(u32 id);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomIDCore

  Description:  カード ID の読み出し

  Arguments:    None.

  Returns:      カード ID
 *---------------------------------------------------------------------------*/
u32     CARDi_ReadRomIDCore(void);

#ifdef SDK_ARM7_READROM_SUPPORT

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomCore

  Description:  ARM7 からのカードアクセス

  Arguments:    src        転送元オフセット
                src        転送元メモリアドレス
                src        転送サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_ReadRomCore(const void *src, void *dst, u32 len);

#endif // SDK_ARM7_READROM_SUPPORT

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomStatusCore

  Description:  カードステータスの読み出し。
                リフレッシュ対応ROM検出時のみ発行。
                対応ROMを搭載したNITROアプリケーションでも必要になる。

  Arguments:    None.

  Returns:      カードステータス
 *---------------------------------------------------------------------------*/
u32 CARDi_ReadRomStatusCore(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_RefreshRomCore

  Description:  カードROMバッドブロックのリフレッシュ。
                該当ROMを搭載したNITROアプリケーションでも必要になる。
                カードへコマンドを一方的に発行するだけなのでレイテンシ設定は無効。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_RefreshRomCore(void);


#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_LIBRARIES_CARD_ROM_H__
