/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     rom.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-05#$
  $Rev: 9518 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_CARD_ROM_H_
#define NITRO_CARD_ROM_H_


#include <nitro/card/types.h>

#include <nitro/mi/dma.h>
#include <nitro/mi/exMemory.h>


#ifdef __cplusplus
extern "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_GetRomHeader

  Description:  現在挿入されているカードのROMヘッダ情報を取得

  Arguments:    None.

  Returns:      CARDRomHeader構造体へのポインタ
 *---------------------------------------------------------------------------*/
const u8 *CARD_GetRomHeader(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetOwnRomHeader

  Description:  現在実行しているプログラムのROMヘッダ情報を取得。

  Arguments:    None.

  Returns:      CARDRomHeader構造体へのポインタ
 *---------------------------------------------------------------------------*/
const CARDRomHeader *CARD_GetOwnRomHeader(void);

#ifdef SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         CARD_GetOwnRomHeaderTWL

  Description:  現在実行しているプログラムのTWL-ROMヘッダ情報を取得。

  Arguments:    None.

  Returns:      CARDRomHeaderTWL構造体へのポインタ
 *---------------------------------------------------------------------------*/
const CARDRomHeaderTWL *CARD_GetOwnRomHeaderTWL(void);

#endif // SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         CARD_GetRomRegionFNT

  Description:  ROM ヘッダの FNT 領域情報を取得する

  Arguments:    None.

  Returns:      ROM ヘッダの FNT 領域情報 へのポインタ
 *---------------------------------------------------------------------------*/
SDK_INLINE const CARDRomRegion *CARD_GetRomRegionFNT(void)
{
    const CARDRomHeader *header = CARD_GetOwnRomHeader();
    return &header->fnt;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetRomRegionFAT

  Description:  ROM ヘッダの FAT 領域情報を取得する

  Arguments:    None.

  Returns:      ROM ヘッダの FAT 領域情報 へのポインタ
 *---------------------------------------------------------------------------*/
SDK_INLINE const CARDRomRegion *CARD_GetRomRegionFAT(void)
{
    const CARDRomHeader *header = CARD_GetOwnRomHeader();
    return &header->fat;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetRomRegionOVT

  Description:  ROM ヘッダの OVT 領域情報を取得する

  Arguments:    None.

  Returns:      ROM ヘッダの OVT 領域情報 へのポインタ
 *---------------------------------------------------------------------------*/
SDK_INLINE const CARDRomRegion *CARD_GetRomRegionOVT(MIProcessor target)
{
    const CARDRomHeader *header = CARD_GetOwnRomHeader();
    return (target == MI_PROCESSOR_ARM9) ? &header->main_ovt : &header->sub_ovt;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_LockRom

  Description:  ROMアクセスのためにカードバスをロックする

  Arguments:    lock id.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_LockRom(u16 lock_id);

/*---------------------------------------------------------------------------*
  Name:         CARD_UnlockRom

  Description:  ロックしたカードバスを解放する

  Arguments:    lock id which is used by CARD_LockRom().

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_UnlockRom(u16 lock_id);

/*---------------------------------------------------------------------------*
  Name:         CARD_TryWaitRomAsync

  Description:  ROMアクセス関数が完了したか判定

  Arguments:    None.

  Returns:      ROMアクセス関数が完了していればTRUE
 *---------------------------------------------------------------------------*/
BOOL    CARD_TryWaitRomAsync(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_WaitRomAsync

  Description:  ROMアクセス関数が完了するまで待機

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_WaitRomAsync(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRom

  Description:  ROM リードの基本関数

  Arguments:    dma        使用する DMA チャンネル
                src        転送元オフセット
                dst        転送先メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)
                is_async   非同期モードを指定するなら TRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_ReadRom(u32 dma, const void *src, void *dst, u32 len,
                      MIDmaCallback callback, void *arg, BOOL is_async);

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadRomAsync

  Description:  非同期 ROM リード

  Arguments:    dma        使用する DMA チャンネル
                src        転送元オフセット
                dst        転送先メモリアドレス
                len        転送サイズ
                callback   完了コールバック (不使用なら NULL)
                arg        完了コールバックの引数 (不使用なら無視される)

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ReadRomAsync(u32 dma, const void *src, void *dst, u32 len,
                                  MIDmaCallback callback, void *arg)
{
    CARDi_ReadRom(dma, src, dst, len, callback, arg, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadRom

  Description:  同期 ROM リード

  Arguments:    dma        使用する DMA チャンネル
                src        転送元オフセット
                dst        転送先メモリアドレス
                len        転送サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARD_ReadRom(u32 dma, const void *src, void *dst, u32 len)
{
    CARDi_ReadRom(dma, src, dst, len, NULL, NULL, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetCacheFlushThreshold

  Description:  キャッシュ無効化を部分的に行うか全体へ行うかの閾値を取得

  Arguments:    icache            命令キャッシュの無効化閾値を格納するポインタ
                                  NULLであれば無視される
                dcache            データキャッシュの無効化閾値を格納するポインタ
                                  NULLであれば無視される

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_GetCacheFlushThreshold(u32 *icache, u32 *dcache);

/*---------------------------------------------------------------------------*
  Name:         CARD_SetCacheFlushThreshold

  Description:  キャッシュ無効化を部分的に行うか全体へ行うかの閾値を設定

  Arguments:    icache            命令キャッシュの無効化閾値
                dcache            データキャッシュの無効化閾値

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_SetCacheFlushThreshold(u32 icache, u32 dcache);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetCacheFlushFlag

  Description:  キャッシュ無効化を自動的に行うかどうかの設定フラグを取得。

  Arguments:    icache            命令キャッシュの自動無効化フラグを格納するポインタ
                                  NULLであれば無視される
                dcache            データキャッシュの自動無効化フラグを格納するポインタ
                                  NULLであれば無視される

  Returns:      None.
 *---------------------------------------------------------------------------*/
void     CARD_GetCacheFlushFlag(BOOL *icache, BOOL *dcache);

/*---------------------------------------------------------------------------*
  Name:         CARD_SetCacheFlushFlag

  Description:  キャッシュ無効化を自動的に行うかどうかを設定。
                デフォルトでは命令キャッシュがFALSEでデータキャッシュがTRUE。

  Arguments:    icache            命令キャッシュの自動無効化を有効にするならTRUE
                dcache            データキャッシュの自動無効化を有効にするならTRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void     CARD_SetCacheFlushFlag(BOOL icache, BOOL dcache);


/*---------------------------------------------------------------------------*
 * internal functions
 *---------------------------------------------------------------------------*/

u32     CARDi_ReadRomID(void);
void    CARDi_RefreshRom(u32 warn_mask);
BOOL    CARDi_IsTwlRom(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetOwnSignature

  Description:  自分自身のDSダウンロードプレイ署名データを取得。

  Arguments:    None.

  Returns:      自分自身のDSダウンロードプレイ署名データ。
 *---------------------------------------------------------------------------*/
const u8* CARDi_GetOwnSignature(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetOwnSignature

  Description:  自分自身のDSダウンロードプレイ署名データを設定。
                非カードブート時に上位ライブラリから呼び出す。

  Arguments:    DSダウンロードプレイ署名データ

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_SetOwnSignature(const void *signature);


#ifdef __cplusplus
} /* extern "C" */
#endif


/* NITRO_CARD_ROM_H_ */
#endif
