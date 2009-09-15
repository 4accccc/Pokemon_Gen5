/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     types.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-27#$
  $Rev: 9427 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_CARD_TYPES_H_
#define NITRO_CARD_TYPES_H_


#include <nitro/misc.h>
#include <nitro/types.h>


#ifdef __cplusplus
extern "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* declarations */

// ROM領域情報構造体
typedef struct CARDRomRegion
{
    u32     offset;
    u32     length;
}
CARDRomRegion;

// ROMヘッダ構造体
typedef struct CARDRomHeader
{

    // 0x000-0x020 [システム予約領域]
    char    game_name[12];          // ソフトタイトル名
    u32     game_code;              // イニシャルコード
    u16     maker_code;             // メーカーコード
    u8      product_id;             // 本体コード
    u8      device_type;            // デバイスタイプ
    u8      device_size;            // デバイス容量
    u8      reserved_A[9];          // システム予約 A
    u8      game_version;           // ソフトバージョン
    u8      property;               // 内部使用フラグ
    // 0x020-0x040 [常駐モジュール用パラメータ]
    void   *main_rom_offset;        // ARM9 転送元 ROM オフセット
    void   *main_entry_address;     // ARM9 実行開始アドレス(未実装)
    void   *main_ram_address;       // ARM9 転送先 RAM オフセット
    u32     main_size;              // ARM9 転送サイズ
    void   *sub_rom_offset;         // ARM7 転送元 ROM オフセット
    void   *sub_entry_address;      // ARM7 実行開始アドレス(未実装)
    void   *sub_ram_address;        // ARM7 転送先 RAM オフセット
    u32     sub_size;               // ARM7 転送サイズ

    // 0x040-0x050 [ファイルテーブル用パラメータ]
    CARDRomRegion fnt;              // ファイルネームテーブル
    CARDRomRegion fat;              // ファイルアロケーションテーブル

    // 0x050-0x060 [オーバーレイヘッダテーブル用パラメータ]
    CARDRomRegion main_ovt;         // ARM9 オーバーレイヘッダテーブル
    CARDRomRegion sub_ovt;          // ARM7 オーバーレイヘッダテーブル

    // 0x060-0x070 [システム予約領域]
    u8      rom_param_A[8];         // マスク ROM コントロールパラメータ A
    u32     banner_offset;          // バナーファイル ROM オフセット
    u16     secure_crc;             // セキュア領域 CRC
    u8      rom_param_B[2];         // マスク ROM コントロールパラメータ B

    // 0x070-0x078 [オートロードパラメータ]
    void   *main_autoload_done;     // ARM9 オートロードフックアドレス
    void   *sub_autoload_done;      // ARM7 オートロードフックアドレス

    // 0x078-0x0C0 [システム予約領域]
    u8      rom_param_C[8];         // マスク ROM コントロールパラメータ C
    u32     rom_size;               // アプリケーション最終 ROM オフセット
    u32     header_size;            // ROM ヘッダサイズ
    u32     main_module_param_offset;  // Offset for table of ARM9 module parameters
    u32     sub_module_param_offset;   // Offset for table of ARM7 module parameters

    u16     normal_area_rom_offset; // undeveloped
    u16     twl_ltd_area_rom_offset;// undeveloped
    u8      reserved_B[0x2C];       // システム予約 B

    // 0x0C0-0x160 [システム予約領域]
    u8      logo_data[0x9C];        // NINTENDO ロゴイメージデータ
    u16     logo_crc;               // NINTENDO ロゴ CRC
    u16     header_crc;             // ROM 内登録データ CRC

}
CARDRomHeader;

typedef CARDRomHeader CARDRomHeaderNTR;

// TWL拡張ROMヘッダ情報
typedef struct CARDRomHeaderTWL
{
    CARDRomHeaderNTR    ntr;
    u8                  debugger_reserved[0x20];    // デバッガ予約
    u8                  config1[0x34];              // 内部使用フラグ群
    // 0x1B4 - accessControl
    struct {
        u32     :5;
        u32     game_card_on :1;            // NANDアプリでゲームカード電源ON（ノーマルモード）
		u32     :2;
        u32     game_card_nitro_mode :1;    // NANDアプリでゲームカードNTR互換領域へアクセス
        u32     :0;
    }access_control;
    u8                  reserved_0x1B8[8];          // 予約 (all 0)
    u32                 main_ltd_rom_offset;
    u8                  reserved_0x1C4[4];          // 予約 (all 0)
    void               *main_ltd_ram_address;
    u32                 main_ltd_size;
    u32                 sub_ltd_rom_offset;
    u8                  reserved_0x1D4[4];          // 予約 (all 0)
    void               *sub_ltd_ram_address;
    u32                 sub_ltd_size;
    CARDRomRegion       digest_area_ntr;
    CARDRomRegion       digest_area_ltd;
    CARDRomRegion       digest_tabel1;
    CARDRomRegion       digest_tabel2;
    u32                 digest_table1_size;
    u32                 digest_table2_sectors;
    u8                  config2[0xF8];              // 内部使用フラグ群
    u8                  main_static_digest[0x14];
    u8                  sub_static_digest[0x14];
    u8                  digest_tabel2_digest[0x14];
    u8                  banner_digest[0x14];
    u8                  main_ltd_static_digest[0x14];
    u8                  sub_ltd_static_digest[0x14];
}
CARDRomHeaderTWL;


SDK_COMPILER_ASSERT(sizeof(CARDRomHeader) == 0x160);
SDK_COMPILER_ASSERT(sizeof(CARDRomHeaderTWL) == 0x378);


/*---------------------------------------------------------------------------*/
/* constants */

// ROM エリアサイズ
#define CARD_ROM_PAGE_SIZE	512

// 場合によってNTR領域終端(CARDRomHeader.rom_size)に付加されている
// DSダウンロードプレイ署名データのサイズ。
#define CARD_ROM_DOWNLOAD_SIGNATURE_SIZE 136

// CARDライブラリ関数処理結果
typedef enum CARDResult
{
    CARD_RESULT_SUCCESS = 0,
    CARD_RESULT_FAILURE,
    CARD_RESULT_INVALID_PARAM,
    CARD_RESULT_UNSUPPORTED,
    CARD_RESULT_TIMEOUT,
    CARD_RESULT_ERROR,
    CARD_RESULT_NO_RESPONSE,
    CARD_RESULT_CANCELED
}
CARDResult;

#define CARD_ROM_HEADER_EXE_NTR_OFF 0x01
#define CARD_ROM_HEADER_EXE_TWL_ON  0x02


/*---------------------------------------------------------------------------*/
/* types */

/*---------------------------------------------------------------------------*
  Name:         CARD_IsExecutableOnNTR

  Description:  プログラムの「DSモード起動可能」フラグを判定する。

  Arguments:    header : 判定すべきプログラムのROMヘッダ情報。

  Returns:      DSモードで起動可能なプログラムのROMヘッダであればTRUE。
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_IsExecutableOnNTR(const CARDRomHeader *header)
{
    return ((header->product_id & CARD_ROM_HEADER_EXE_NTR_OFF) == 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_IsExecutableOnTWL

  Description:  プログラムの「TWLモード起動可能」フラグを判定する。

  Arguments:    header : 判定すべきプログラムのROMヘッダ情報。

  Returns:      TWLモードで起動可能なプログラムのROMヘッダであればTRUE。
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL CARD_IsExecutableOnTWL(const CARDRomHeader *header)
{
    return ((header->product_id & CARD_ROM_HEADER_EXE_TWL_ON) != 0);
}


#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_CARD_TYPES_H_
