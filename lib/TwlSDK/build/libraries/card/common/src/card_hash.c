/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_hash.c

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "../include/card_rom.h"


// サイズが大きいMI_CpuCopyをNTR環境でリンクしないよう、LIMITED時だけ使用。
#if defined(SDK_TWLLTD)
#define CARDi_CopyMemory MI_CpuCopy
#else
#define CARDi_CopyMemory MI_CpuCopy8
#endif


// HMAC-SHA1の計算を高速化する場合はこのフラグを有効に。
#if defined(SDK_TWL)
#define CARD_SUPPORT_SHA1OPTIMIZE
#endif

#include <twl/ltdmain_begin.h>

/*---------------------------------------------------------------------------*/
/* constants */

// 現時点でCARDライブラリが妥当と判断した定数。
// パフォーマンス計測の結果によって適宜変更してもよい。
static const u32    CARD_ROM_HASH_BLOCK_MAX = 4;
static const u32    CARD_ROM_HASH_SECTOR_MAX = 32;

#if defined(CARD_SUPPORT_SHA1OPTIMIZE)
static SVCSHA1Context   sha1_ipad_def;
static SVCSHA1Context   sha1_opad_def;
#endif

static u8 CARDiHmacKey[] ATTRIBUTE_ALIGN(4) =
{
    0x21, 0x06, 0xc0, 0xde, 0xba, 0x98, 0xce, 0x3f,
    0xa6, 0x92, 0xe3, 0x9d, 0x46, 0xf2, 0xed, 0x01,
    0x76, 0xe3, 0xcc, 0x08, 0x56, 0x23, 0x63, 0xfa,
    0xca, 0xd4, 0xec, 0xdf, 0x9a, 0x62, 0x78, 0x34,
    0x8f, 0x6d, 0x63, 0x3c, 0xfe, 0x22, 0xca, 0x92,
    0x20, 0x88, 0x97, 0x23, 0xd2, 0xcf, 0xae, 0xc2,
    0x32, 0x67, 0x8d, 0xfe, 0xca, 0x83, 0x64, 0x98,
    0xac, 0xfd, 0x3e, 0x37, 0x87, 0x46, 0x58, 0x24,
};

/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_CalcRomHashBufferLength

  Description:  ROMハッシュコンテキストに必要なバッファサイズを取得。

  Arguments:    header   : 管理すべきROM情報を保持したROMヘッダバッファ

  Returns:      ROMハッシュコンテキストに必要なバッファサイズ。
 *---------------------------------------------------------------------------*/
u32 CARD_CalcRomHashBufferLength(const CARDRomHeaderTWL *header)
{
    u32     length = 0;
    length += MATH_ROUNDUP(header->digest_tabel2.length, 32);
    length += MATH_ROUNDUP(header->digest_table1_size * CARD_ROM_HASH_SECTOR_MAX, 32);
    length += MATH_ROUNDUP(MATH_ROUNDUP(CARD_ROM_HASH_SIZE * header->digest_table2_sectors, CARD_ROM_PAGE_SIZE * 3) * CARD_ROM_HASH_BLOCK_MAX, 32);
    length += MATH_ROUNDUP(sizeof(CARDRomHashSector) * CARD_ROM_HASH_SECTOR_MAX, 32);
    length += MATH_ROUNDUP(sizeof(CARDRomHashBlock) * CARD_ROM_HASH_BLOCK_MAX, 32);
    return length;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CompareHash

  Description:  ハッシュによる正当性チェック。(HMAC-SHA1)

  Arguments:    hash : 比較基準となるハッシュ値
                src  : チェック対象のイメージ
                len　: チェック対象のサイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_CompareHash(const void *hash, void *buffer, u32 length)
{
#if defined(CARD_SUPPORT_SHA1OPTIMIZE)
    // 前もって計算済みのipad/opadを構造体ごとコピー。
    u8      tmphash[CARD_ROM_HASH_SIZE];
    const u32  *h1 = (const u32 *)hash;
    const u32  *h2 = (const u32 *)tmphash;
    u32         dif = 0;
    SVCSHA1Context  tmpcxt;
    CARDi_CopyMemory(&sha1_ipad_def, &tmpcxt, sizeof(tmpcxt));
    SVC_SHA1Update(&tmpcxt, buffer, length);
    SVC_SHA1GetHash(&tmpcxt, tmphash);
    CARDi_CopyMemory(&sha1_opad_def, &tmpcxt, sizeof(tmpcxt));
    SVC_SHA1Update(&tmpcxt, tmphash, sizeof(tmphash));
    SVC_SHA1GetHash(&tmpcxt, tmphash);
    dif |= (h1[0] ^ h2[0]);
    dif |= (h1[1] ^ h2[1]);
    dif |= (h1[2] ^ h2[2]);
    dif |= (h1[3] ^ h2[3]);
    dif |= (h1[4] ^ h2[4]);
    if (dif != 0)
#else
    u8      tmphash[CARD_ROM_HASH_SIZE];
#ifdef SDK_TWL
    if (OS_IsRunOnTwl())
    {
        SVC_CalcHMACSHA1(tmphash, buffer, length, CARDiHmacKey, sizeof(CARDiHmacKey));
    }
    else
#endif
    {
        MATH_CalcHMACSHA1(tmphash, buffer, length, CARDiHmacKey, sizeof(CARDiHmacKey));
    }
    if (MI_CpuComp8(hash, tmphash, sizeof(tmphash)) != 0)
#endif
    {
        // カードが抜けつつ終了処理が遅延されている場合は
        // この場で強制停止せず警告出力&データクリアにとどめる。
        if ((OS_GetBootType() == OS_BOOTTYPE_ROM) && CARD_IsPulledOut())
        {
            OS_TWarning("ROM-hash comparation error. (CARD removal)\n");
            MI_CpuFill(buffer, 0, length);
        }
        else
        {
            OS_TPanic("ROM-hash comparation error!\n");
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetHashSectorIndex

  Description:  指定のROMオフセットが属するセクタ番号を取得。

  Arguments:    context : CARDRomHashContext構造体
                offset  : ROMオフセット

  Returns:      指定のROMオフセットが属するセクタ番号。
 *---------------------------------------------------------------------------*/
SDK_INLINE u32 CARDi_GetHashSectorIndex(const CARDRomHashContext *context, u32 offset)
{
    offset -= context->area_ntr.offset;
    if (offset >= context->area_ntr.length)
    {
        offset += (context->area_ntr.offset - context->area_ltd.offset);
        if (offset < context->area_ltd.length)
        {
            offset += context->area_ntr.length;
        }
        else
        {
            OS_TPanic("specified ROM address is outof-range.(unsafe without secure hash)\n");
        }
    }
    return offset / context->bytes_per_sector;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_StartLoading

  Description:  メディアロード待ちセクタをひとつだけ転送開始。

  Arguments:    context : CARDRomHashContext構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_StartLoading(CARDRomHashContext *context)
{
    // 現在非同期処理を実行していなければここで開始。
    void   *buffer = NULL;
    u32     offset = 0;
    u32     length = 0;
    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
    if (context->recent_load == NULL)
    {
        if (context->loading_block)
        {
            // 非同期DMA転送ができるように実際より多くバッファを取ってある。
            // 参照先は実際に必要とするテーブル位置へオフセットを合わせる。
            CARDRomHashBlock   *block = context->loading_block;
            u32                 pos = block->offset;
            u32                 mod = (pos & (CARD_ROM_PAGE_SIZE - 1UL));
            block->hash = &block->hash_aligned[mod];
            context->recent_load = block;
            buffer = block->hash_aligned;
            offset = pos - mod;
            length = MATH_ROUNDUP(mod + CARD_ROM_HASH_SIZE * context->sectors_per_block, CARD_ROM_PAGE_SIZE);
        }
        else if (context->loading_sector)
        {
            CARDRomHashSector  *sector = context->loading_sector;
            context->recent_load = sector;
            buffer = sector->image;
            offset = sector->offset;
            length = context->bytes_per_sector;
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
    // 非同期転送が非サポートか現在利用不可能ならばここで同期転送。
    if (buffer != NULL)
    {
        if ((*context->ReadAsync)(context->userdata, buffer, offset, length) == 0)
        {
            (void)(*context->ReadSync)(context->userdata, buffer, offset, length);
            CARD_NotifyRomHashReadAsync(context);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_NotifyRomHashReadAsync

  Description:  デバイス非同期リードの完了を通知。

  Arguments:    context : CARDRomHashContext構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_NotifyRomHashReadAsync(CARDRomHashContext *context)
{
    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
    // ロード待ちから検証待ちへリスト移動。
    if (context->recent_load == context->loading_sector)
    {
        CARDRomHashSector  *sector = context->loading_sector;
        context->loading_sector = sector->next;
        sector->next = context->loaded_sector;
        context->loaded_sector = sector;
    }
    else
    {
        CARDRomHashBlock   *block = context->loading_block;
        context->loading_block = block->next;
        block->next = context->loaded_block;
        context->loaded_block = block;
    }
    context->recent_load = NULL;
    (void)OS_RestoreInterrupts(bak_cpsr);
    // 検証待ちリストが追加されたことを待ち受けスレッドへ通知。
    OS_WakeupThreadDirect(context->loader);
    // あるなら次のロード待ちリストを転送開始。
    CARDi_StartLoading(context);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_TouchRomHashBlock

  Description:  指定のオフセットが含まれるハッシュブロック情報をロード。
                すでにロード済みなら何もしない。

  Arguments:    context : CARDRomHashContext構造体。
                sector  : アクセスするセクタ。

  Returns:      該当するハッシュブロック情報。
 *---------------------------------------------------------------------------*/
static CARDRomHashBlock* CARDi_TouchRomHashBlock(CARDRomHashContext *context, u32 sector)
{
    // ブロック領域はNTR領域とLTD領域が連結して扱われる点に注意。
    // (余計なパディングやアライメントを挿入してはならない)
    u32     index = sector / context->sectors_per_block;
    // 該当するブロック情報がすでにロード済みか判定。
    CARDRomHashBlock  **pp = &context->valid_block;
    CARDRomHashBlock   *block = NULL;
    for (;; pp = &(*pp)->next)
    {
        block = *pp;
        if (block->index == index)
        {
            // タッチされたブロック情報はリストの先頭に移動しておく。
            *pp = block->next;
            block->next = context->valid_block;
            context->valid_block = block;
            break;
        }
        // ロードされていなければ最も古いブロック情報を破棄して再ロード。
        else if (block->next == NULL)
        {
            // ロード待ちリストの終端へ移動する。
            OSIntrMode  bak_cpsr = OS_DisableInterrupts();
            CARDRomHashBlock  **tail = pp;
            CARDRomHashBlock   *loading = block;
            // すでに同一ブロックがロード待ちか検証待ちなら何もしない。
            for (pp = &context->loaded_block; *pp; pp = &(*pp)->next)
            {
                if ((*pp)->index == index)
                {
                    block = (*pp);
                    loading = NULL;
                    break;
                }
            }
            if (loading)
            {
                for (pp = &context->loading_block; *pp; pp = &(*pp)->next)
                {
                    if ((*pp)->index == index)
                    {
                        block = (*pp);
                        loading = NULL;
                        break;
                    }
                }
                if (loading)
                {
                    *tail = NULL;
                    *pp = loading;
                    loading->index = index;
                    loading->offset = context->sector_hash.offset + index * (CARD_ROM_HASH_SIZE * context->sectors_per_block);
                }
            }
            (void)OS_RestoreInterrupts(bak_cpsr);
            if (loading)
            {
                // 非同期ロードの開始タイミングを確認。
                CARDi_StartLoading(context);
            }
            break;
        }
    }
    return block;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_TouchRomHashSector

  Description:  指定のセクタをブロック上にロード。
                すでにロード済みなら何もしない。

  Arguments:    context : CARDRomHashContext構造体。
                block  : CARDRomHashBlock構造体。
                offset : アクセスするROMオフセット。

  Returns:      該当するセクタイメージまたはNULL。
 *---------------------------------------------------------------------------*/
static void* CARDi_TouchRomHashSector(CARDRomHashContext *context, u32 offset)
{
    void               *image = NULL;
    CARDRomHashSector **pp = &context->valid_sector;
    CARDRomHashSector  *sector = NULL;
    u32                 index = CARDi_GetHashSectorIndex(context, offset);
    for (pp = &context->valid_sector; ; pp = &(*pp)->next)
    {
        // 該当するセクタ情報がすでにロード済みならそのイメージを返す。
        if ((*pp)->index == index)
        {
            // タッチされたセクタ情報はリストの先頭に移動しておく。
            sector = *pp;
            *pp = (*pp)->next;
            sector->next = context->valid_sector;
            context->valid_sector = sector;
            image = sector->image;
            break;
        }
        // ロードされていなければ最も古いセクタ情報を破棄して再ロード。
        else if ((*pp)->next == NULL)
        {
            // ロード待ちリストの終端へ移動する。
            OSIntrMode  bak_cpsr = OS_DisableInterrupts();
            CARDRomHashSector  *loading = *pp;
            *pp = NULL;
            for (pp = &context->loading_sector; *pp; pp = &(*pp)->next)
            {
            }
            *pp = loading;
            loading->index = index;
            loading->offset = MATH_ROUNDDOWN(offset, context->bytes_per_sector);
            (void)OS_RestoreInterrupts(bak_cpsr);
            // 正当性検証のために必要となるブロックを先に検索しておく。
            (void)CARDi_TouchRomHashBlock(context, index);
            // 非同期ロードの開始タイミングを確認。
            CARDi_StartLoading(context);
            break;
        }
    }
    return image;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_InitRomHashContext

  Description:  ROMハッシュコンテキストを初期化。

  Arguments:    context  : 初期化すべきCARDRomHashContext構造体
                header   : 管理すべきROM情報を保持したROMヘッダバッファ
                           (この関数内でのみ参照されるため保持する必要はない)
                buffer   : コンテキスト内部で使用する管理情報用バッファ
                length   : 管理情報用バッファのサイズ
                           (CARD_CalcRomHashBufferLength関数で計算可能)
                sync     : デバイス同期リードコールバック関数
                async    : デバイス非同期リードコールバック関数(非対応ならNULL)
                userdata : デバイスリードコールバックに与える任意のユーザ定義引数

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_InitRomHashContext(CARDRomHashContext *context, const CARDRomHeaderTWL *header,
                             void *buffer, u32 length,
                             MIDeviceReadFunction sync, MIDeviceReadFunction async,
                             void *userdata)
{
    // ビルドごとに異なる定数。
    const u32   bytes_per_sector = header->digest_table1_size;
    const u32   sectors_per_block = header->digest_table2_sectors;
    const u32   master_hash_size = header->digest_tabel2.length;

    // 必要な可変長メモリを確保できているか計算。
    u8     *lo = (u8 *)MATH_ROUNDUP((u32)buffer, 32);
    u8     *hi = (u8 *)MATH_ROUNDDOWN((u32)&lo[length], 32);
    u8     *cur = lo;
    context->master_hash = (u8 *)cur;
    cur += MATH_ROUNDUP(master_hash_size, 32);
    context->images = (u8 *)cur;
    cur += MATH_ROUNDUP(bytes_per_sector * CARD_ROM_HASH_SECTOR_MAX, 32);
    context->hashes = (u8 *)cur;
    cur += MATH_ROUNDUP(MATH_ROUNDUP(CARD_ROM_HASH_SIZE * sectors_per_block, CARD_ROM_PAGE_SIZE * 3) * CARD_ROM_HASH_BLOCK_MAX, 32);
    context->sectors = (CARDRomHashSector *)cur;
    cur += MATH_ROUNDUP(sizeof(*context->sectors) * CARD_ROM_HASH_SECTOR_MAX, 32);
    context->blocks = (CARDRomHashBlock *)cur;
    cur += MATH_ROUNDUP(sizeof(*context->blocks) * CARD_ROM_HASH_BLOCK_MAX, 32);
    if (cur > hi)
    {
        OS_TPanic("cannot allocate memory for ROM-hash from ARENA");
    }
    else
    {
        // 確保可能なら基本設定を初期化。
        MI_CpuClear32(lo, (u32)(cur - lo));
        context->area_ntr = header->digest_area_ntr;
        context->area_ltd = header->digest_area_ltd;
        context->sector_hash = header->digest_tabel1;
        context->block_hash = header->digest_tabel2;
        context->bytes_per_sector = bytes_per_sector;
        context->sectors_per_block = sectors_per_block;
        context->block_max = CARD_ROM_HASH_BLOCK_MAX;
        context->sector_max = CARD_ROM_HASH_SECTOR_MAX;
        // デバイスリーダを初期化。
        context->ReadSync = sync;
        context->ReadAsync = async;
        context->userdata = userdata;
#if defined(CARD_SUPPORT_SHA1OPTIMIZE)
        // ハッシュ計算用のipad/opadを事前計算してしまう。
        {
            u8      ipad[MATH_HASH_BLOCK_SIZE];
            u8      opad[MATH_HASH_BLOCK_SIZE];
            int     i;
            for (i = 0; i < MATH_HASH_BLOCK_SIZE; ++i)
            {
                ipad[i] = (u8)(CARDiHmacKey[i] ^ 0x36);
                opad[i] = (u8)(CARDiHmacKey[i] ^ 0x5c);
            }
            SVC_SHA1Init(&sha1_ipad_def);
            SVC_SHA1Init(&sha1_opad_def);
            SVC_SHA1Update(&sha1_ipad_def, ipad, sizeof(ipad));
            SVC_SHA1Update(&sha1_opad_def, opad, sizeof(opad));
        }
#endif
        // さっそくマスターハッシュをロードして正当性判定。
        (void)(*context->ReadSync)(context->userdata, context->master_hash, context->block_hash.offset, context->block_hash.length); 
        CARDi_CompareHash(header->digest_tabel2_digest, context->master_hash, context->block_hash.length);
        // セクタ情報リストを初期化。
        {
            CARDRomHashSector  *sectors = context->sectors;
            int     sector_index = 0;
            int     i;
            for (i = 0; i < context->sector_max; ++i)
            {
                sectors[i].next = &sectors[i - 1];
                sectors[i].image = &context->images[i * context->bytes_per_sector];
                sectors[i].index = 0xFFFFFFFF;
            }
            context->sectors[0].next = NULL;
            context->valid_sector = &context->sectors[context->sector_max - 1];
            context->loading_sector = NULL;
            context->loaded_sector = NULL;
        }
        // ブロック情報リストを初期化。
        {
            CARDRomHashBlock   *blocks = context->blocks;
            const u32           unit = MATH_ROUNDUP(CARD_ROM_HASH_SIZE * sectors_per_block, CARD_ROM_PAGE_SIZE * 3);
            int     i;
            for (i = 0; i < context->block_max; ++i)
            {
                blocks[i].next = &blocks[i + 1];
                blocks[i].index = 0xFFFFFFFF;
                blocks[i].hash_aligned = &context->hashes[i * unit];
            }
            context->valid_block = &context->blocks[0];
            context->blocks[context->block_max - 1].next = NULL;
            context->loading_block = NULL;
            context->loaded_block = NULL;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomHashImageDirect

  Description:  ハッシュコンテキストへキャッシュせずに転送先へ直接コピー。

  Arguments:    context : CARDRomHashContext構造体。
                buffer  : 転送先バッファ。(4バイト整合されている必要がある)
                offset  : アクセスするROMオフセット。
                length  : 転送サイズ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_ReadRomHashImageDirect(CARDRomHashContext *context, void *buffer, u32 offset, u32 length)
{
    const u32   sectunit = context->bytes_per_sector;
    const u32   blckunit = context->sectors_per_block;
    u32         position = offset;
    u32         end = length + offset;
    u32         sector = CARDi_GetHashSectorIndex(context, position);
    while (position < end)
    {
        // 今回取得可能な最小単位のイメージを同期読み込み。
        u32     available = (u32)(*context->ReadSync)(context->userdata, buffer, position, end - position);
        // 今回の検証中に必要となりそうなブロックを前もってアクセス。
        (void)CARDi_TouchRomHashBlock(context, sector);
        // 次回に必要となる分の読み込み準備を要求。
        if (context->ReadAsync && (position + available < end))
        {
            (void)(*context->ReadAsync)(context->userdata, NULL, position + available, end - (position + available));
        }
        // 取得したイメージの正当性を検証。
        while (available >= sectunit)
        {
            CARDRomHashBlock   *block = CARDi_TouchRomHashBlock(context, sector);
            u32                 slot = sector - block->index * blckunit;
            while ((slot < blckunit) && (available >= sectunit))
            {
                // 必要ならここでブロック単位のハッシュテーブルを検証。
                if (block != context->valid_block)
                {
                    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
                    while (context->loading_block)
                    {
                        OS_SleepThread(NULL);
                    }
                    if (block == context->loaded_block)
                    {
                        context->loaded_block = block->next;
                    }
                    (void)OS_RestoreInterrupts(bak_cpsr);
                    CARDi_CompareHash(&context->master_hash[block->index * CARD_ROM_HASH_SIZE],
                                      block->hash, CARD_ROM_HASH_SIZE * blckunit);
                    block->next = context->valid_block;
                    context->valid_block = block;
                }
                // イメージのハッシュを計算。
                CARDi_CompareHash(&block->hash[slot * CARD_ROM_HASH_SIZE], buffer, sectunit);
                position += sectunit;
                available -= sectunit;
                buffer = ((u8 *)buffer) + sectunit;
                slot += 1;
                sector += 1;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomHashImageCaching

  Description:  ハッシュコンテキストへキャッシュしつつ転送先へ直接コピー。

  Arguments:    context : CARDRomHashContext構造体。
                buffer  : 転送先バッファ。
                offset  : アクセスするROMオフセット。
                length  : 転送サイズ。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_ReadRomHashImageCaching(CARDRomHashContext *context, void *buffer, u32 offset, u32 length)
{
    while (length > 0)
    {
        // 次のセクタが正当性検証済みか判定。
        void   *image = CARDi_TouchRomHashSector(context, offset);
        if (image)
        {
            // 検証済みならそのままメモリコピーして次へ。
            u32     max = context->bytes_per_sector;
            u32     mod = offset - MATH_ROUNDDOWN(offset, max);
            u32     len = (u32)MATH_MIN(length, (max - mod));
            CARDi_CopyMemory((u8*)image + mod, buffer, len);
            buffer = (u8 *)buffer + len;
            length -= len;
            offset += len;
        }
        else
        {
            // ヒットしなければ、全ての未検証リストをまとめて処理するため
            // 容量の許す限りここでセクタを先読み要求しておく。
            // 関連するブロックのハッシュも自動的に先読みされ、
            // 可能ならすでに非同期転送が開始されている。
            u32     hit = MATH_ROUNDDOWN(offset, context->bytes_per_sector);
            while (context->valid_sector && context->valid_block)
            {
                hit += context->bytes_per_sector;
                if (hit >= offset + length)
                {
                    break;
                }
                (void)CARDi_TouchRomHashSector(context, hit);
            }
            for (;;)
            {
                // ロード完了状態の検証待ちリストが発生するまで待機。
                OSIntrMode          bak_cpsr = OS_DisableInterrupts();
                CARDRomHashBlock   *block = NULL;
                CARDRomHashSector  *sector = NULL;
                while ((context->loading_sector && !context->loaded_sector) ||
                        (context->loading_block && !context->loaded_block))
                {
                    OS_SleepThread(NULL);
                }
                // 依存関係を考慮してセクタよりブロックを先に処理する。
                block = context->loaded_block;
                if (block)
                {
                    context->loaded_block = block->next;
                }
                else
                {
                    sector = context->loaded_sector;
                    if (sector)
                    {
                        context->loaded_sector = sector->next;
                    }
                }
                (void)OS_RestoreInterrupts(bak_cpsr);
                // ブロックまたはセクタのハッシュを計算してより上位のテーブルと比較検証。
                if (block)
                {
                    u32     len = CARD_ROM_HASH_SIZE * context->sectors_per_block;
                    CARDi_CompareHash(&context->master_hash[block->index * CARD_ROM_HASH_SIZE], block->hash, len);
                    // タッチされたブロック情報はリストの先頭に移動しておく。
                    block->next = context->valid_block;
                    context->valid_block = block;
                }
                else if (sector)
                {
                    CARDRomHashBlock   *block = CARDi_TouchRomHashBlock(context, sector->index);
                    u32                 slot = sector->index - block->index * context->sectors_per_block;
                    CARDi_CompareHash(&block->hash[slot * CARD_ROM_HASH_SIZE], sector->image, context->bytes_per_sector);
                    // タッチされたセクタ情報はリストの先頭に移動しておく。
                    sector->next = context->valid_sector;
                    context->valid_sector = sector;
                }
                // 非同期に検証すべきものがが無くなれば今回のループは終了。
                else
                {
                    break;
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadRomHashImage

  Description:  指定のオフセットからハッシュ検証済みのROMイメージを読み出す。

  Arguments:    context : CARDRomHashContext構造体。
                offset  : アクセスするROMオフセット。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_ReadRomHashImage(CARDRomHashContext *context, void *buffer, u32 offset, u32 length)
{
    // DMAからの通知先を初期化。
    context->loader = OS_GetCurrentThread();
    context->recent_load = NULL;
    // セクタキャッシュの半分以上を一気に失うような大規模なリードなら
    // ダイレクト転送モードを採用して高速化しつつキャッシュも保護する。
    if ((length >= context->bytes_per_sector * (CARD_ROM_HASH_SECTOR_MAX / 2))
        // TODO:
        //   現状ではひとまずカードブートでダイレクト転送モードを使用しない。
        //   (転送元と転送先のワード境界整合は特に考慮しなくともよいが
        //    ROM転送の場合は同期版になるとかえって性能低下するかもしれないので
        //    必要に応じて条件を追加すべき)
        && (OS_GetBootType() != OS_BOOTTYPE_ROM))
    {
        // セクタ境界に整合していない先端部分と終端部分を分割。
        const u32   sectmask = (u32)(context->bytes_per_sector - 1UL);
        const u32   headlen = (u32)((context->bytes_per_sector - offset) & sectmask);
        const u32   bodylen = (u32)((length - headlen) & ~sectmask);
        // 先端部分。
        if (headlen > 0)
        {
            CARDi_ReadRomHashImageCaching(context, buffer, offset, headlen);
            offset += headlen;
            length -= headlen;
            buffer = ((u8 *)buffer) + headlen;
        }
        // 中間部分。
        if (bodylen > 0)
        {
            CARDi_ReadRomHashImageDirect(context, buffer, offset, bodylen);
            offset += bodylen;
            length -= bodylen;
            buffer = ((u8 *)buffer) + bodylen;
        }
    }
    // ダイレクト転送モードで使用できなかった端数を処理。
    CARDi_ReadRomHashImageCaching(context, buffer, offset, length);
}
#include <twl/ltdmain_end.h>
