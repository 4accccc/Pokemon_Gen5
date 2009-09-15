/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     hash.h

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
#ifndef NITRO_CARD_HASH_H_
#define NITRO_CARD_HASH_H_


#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/card/types.h>
#include <nitro/mi/device.h>


#ifdef __cplusplus
extern "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* constants */

#define CARD_ROM_HASH_SIZE      20


/*---------------------------------------------------------------------------*/
/* declarations */

struct CARDRomHashSector;
struct CARDRomHashBlock;
struct CARDRomHashContext;

// 各セクタのイメージキャッシュを管理する構造体。
typedef struct CARDRomHashSector
{
    struct CARDRomHashSector   *next;
    u32                         index;
    u32                         offset;
    void                       *image;
}
CARDRomHashSector;

// 各セクタのイメージキャッシュとハッシュを管理するブロック構造体。
// NTR領域の最後のブロックサイズが端数だとLTD領域の先頭が連結されるため
// 両者の混在するブロックが最大1個だけ存在しうる点に留意しておくこと。
typedef struct CARDRomHashBlock
{
    struct CARDRomHashBlock    *next;
    u32                         index;
    u32                         offset;
    u8                         *hash;
    u8                         *hash_aligned;
}
CARDRomHashBlock;

// SRLファイルのハッシュ管理構造体。
// 所要サイズはプログラムごとに異なり静的に算出できないため
// 初期化時にアリーナから適量だけ動的に確保する予定。
// 必要となるメモリは以下の通り。
//   - マスターハッシュテーブル:
//       ROMヘッダとハッシュ比較して正当性を判定する必要上、
//       初期化時にROMから一括ロードして常駐させておく。
//       (ROMサイズ/セクタサイズ/セクタ単位)*20(SHA1)バイト必要で、
//       1Gbits:1024バイト:32セクタならば約80kBとなる。
//   - ブロックキャッシュ:
//       ブロック単位でROMイメージキャッシュとそのハッシュを管理する。
//       ブロック境界をまたいだ離散的なアクセスを考慮して
//       常に複数を保持できるリスト構造にしておく必要がある。
//       (20 * セクタ単位 + α) の構造体をリスト総数の分だけ必要。
//   - セクタキャッシュ:
//       実際のイメージキャッシュを保持する。
//       ブロックが全セクタをあまねく参照するとは限らないため
//       セクタも別途リスト構造として管理する必要がある。
typedef struct CARDRomHashContext
{
    // ROMヘッダから取得する基本設定
    CARDRomRegion       area_ntr;
    CARDRomRegion       area_ltd;
    CARDRomRegion       sector_hash;
    CARDRomRegion       block_hash;
    u32                 bytes_per_sector;
    u32                 sectors_per_block;
    u32                 block_max;
    u32                 sector_max;

    // データとハッシュをロードするデバイスインタフェース
    void                   *userdata;
    MIDeviceReadFunction    ReadSync;
    MIDeviceReadFunction    ReadAsync;

    // ロード処理中のスレッド。
    OSThread           *loader;
    void               *recent_load;

    // セクタとブロックのキャッシュ
    CARDRomHashSector  *loading_sector; // メディアロード待ちセクタ
    CARDRomHashSector  *loaded_sector;  // ハッシュ検証待ちセクタ
    CARDRomHashSector  *valid_sector;   // 正当性検証済みセクタ
    CARDRomHashBlock   *loading_block;  // メディアロード待ちブロック
    CARDRomHashBlock   *loaded_block;   // ハッシュ検証待ちブロック
    CARDRomHashBlock   *valid_block;    // 正当性検証済みブロック
    // アリーナから確保した配列
    u8                 *master_hash;    // ブロックのハッシュ配列
    u8                 *images;         // セクタイメージ
    u8                 *hashes;         // ブロック内のハッシュ配列
    CARDRomHashSector  *sectors;        // セクタ情報
    CARDRomHashBlock   *blocks;         // ブロック情報
}
CARDRomHashContext;


/*---------------------------------------------------------------------------*/
/* functions */

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
                             void *userdata);

/*---------------------------------------------------------------------------*
  Name:         CARD_CalcRomHashBufferLength

  Description:  ROMハッシュコンテキストに必要なバッファサイズを取得。

  Arguments:    header   : 管理すべきROM情報を保持したROMヘッダバッファ

  Returns:      ROMハッシュコンテキストに必要なバッファサイズ。
 *---------------------------------------------------------------------------*/
u32 CARD_CalcRomHashBufferLength(const CARDRomHeaderTWL *header);

/*---------------------------------------------------------------------------*
  Name:         CARDi_NotifyRomHashReadAsync

  Description:  デバイス非同期リード処理の完了をハッシュコンテキストへ通知。

  Arguments:    context : CARDRomHashContext構造体。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_NotifyRomHashReadAsync(CARDRomHashContext *context);

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadRomHashImage

  Description:  指定のオフセットからハッシュ検証済みのROMイメージを読み出す。

  Arguments:    context : CARDRomHashContext構造体。
                offset  : アクセスするROMオフセット。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_ReadRomHashImage(CARDRomHashContext *context, void *buffer, u32 offset, u32 length);


#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_CARD_HASH_H_
