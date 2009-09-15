/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     ownerInfoEx_private.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef TWL_OS_COMMON_OWNERINFO_PRIVATE_H_
#define TWL_OS_COMMON_OWNERINFO_PRIVATE_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
    一般の開発者が意識しなくてよい宣言をownerInfoEx.hから分離
    ここに定義されているものはアプリから直接使用しないでください
 *---------------------------------------------------------------------------*/

/*===========================================================================*/

#include    <twl/types.h>
#include    <twl/spec.h>
#ifndef SDK_TWL
#include    <nitro/hw/common/mmap_shared.h>
#else
#include    <twl/hw/common/mmap_shared.h>
#endif

#include <twl/os/common/ownerInfoEx.h>


/*---------------------------------------------------------------------------*
    メインメモリに格納される形式
 *---------------------------------------------------------------------------*/


// TWL日付データと互換性のある構造体
typedef struct OSTWLDate{
    u8              month;                      // 月:01～12
    u8              day;                        // 日:01～31
} OSTWLDate;          // 2byte

// TWLオーナー情報と互換性のある構造体
typedef struct OSTWLOwnerInfo
{
    u8              userColor : 4;              // 好きな色
    u8              rsv : 4;                    // 予約。
    u8              pad;                        // パディング
    OSTWLDate       birthday;                   // 生年月日
    u16             nickname[ OS_TWL_NICKNAME_LENGTH + 1 ];   // ニックネーム（終端あり）
    u16             comment[ OS_TWL_COMMENT_LENGTH + 1 ];     // コメント（終端あり）
} OSTWLOwnerInfo;     // 80  bytes


// TWL設定データと互換性のある構造体
typedef struct OSTWLSettingsData
{
    union {
        struct {
            u32     isFinishedInitialSetting : 1;       // 初回設定終了？
            u32     isFinishedInitialSetting_Launcher : 1;  // ランチャーの初回設定終了？
            u32     isSetLanguage : 1;                  // 言語コード設定済み？
            u32     isAvailableWireless : 1;            // 無線モジュールのRFユニットの有効化／無効化
            u32     rsv : 20;                           // 予約
            u32     isAgreeEULAFlagList : 8;            // EULA同意フラグリスト
            // WiFi設定は別データなので、ここに設定済みフラグは用意しない。
        };
        u32         raw;
    } flags;
    u8                      rsv[ 1 ];               // 予約
    u8                      country;                // 国コード
    u8                      language;               // 言語(NTRとの違いは、データサイズ8bit)
    u8                      rtcLastSetYear;         // RTCの前回設定年
    s64                     rtcOffset;              // RTC設定時のオフセット値（ユーザーがRTC設定を変更する度にその値に応じて増減します。）
                                                    //   16bytes ここまでのパラメータサイズ
    u8                      agreeEulaVersion[ 8 ];  //    8bytes 同意したEULAのバージョン
    u8                      pad1[2];
    u8                      pad2[6];                //    6bytes
    u8                      pad3[16];               //   16bytes
    u8                      pad4[20];               //   20bytes
    OSTWLOwnerInfo          owner;                  //   80bytes オーナー情報
    OSTWLParentalControl    parentalControl;        //  148bytes ペアレンタルコントロール情報
} OSTWLSettingsData;  // 296bytes


// TWL_HWノーマル情報設定データと互換性のある構造体
typedef struct OSTWLHWNormalInfo
{
    u8              rtcAdjust;                                  // RTC調整値
    u8              rsv[ 3 ];
    u8              movableUniqueID[ OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN ]; // 移行可能なユニークID
} OSTWLHWNormalInfo;  // 20byte


// TWL_HWセキュア情報設定データと互換性のある構造体
typedef struct OSTWLHWSecureInfo
{
    u32             validLanguageBitmap;                            // 本体で有効な言語コードをビット列で表現
    struct {
        u8          forceDisableWireless :1;
        u8          :7;
    }flags;
    u8              pad[ 3 ];
    u8              region;                                         // リージョン
    u8              serialNo[ OS_TWL_HWINFO_SERIALNO_LEN_MAX ];   // シリアルNo.（終端付きASCII文字列）
} OSTWLHWSecureInfo;  // 24bytes


typedef struct OSTWLWirelessFirmwareData
{
    u8              data;
    u8              rsv[ 3 ];
} OSTWLWirelessFirmwareData;

/*---------------------------------------------------------------------------*
    関数宣言
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         OS_GetWirelessWirmwareData

  Description:  無線ファームウェア用データへのポインタを取得する。

  Arguments:    None.
  Returns:      無線ファームウェア用データへのポインタを返す。
                TWL上の動作でないときはNULLを返す。
 *---------------------------------------------------------------------------*/
OSTWLWirelessFirmwareData *OS_GetWirelessFirmwareData(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetValidLanguageBitmap

  Description:  対応言語ビットマップを取得する。

  Arguments:    None.
  Returns:      対応言語ビットマップを返す。
                TWL上の動作でないときは"0"を返す。
 *---------------------------------------------------------------------------*/
u32 OS_GetValidLanguageBitmap(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetSerialNo

  Description:  シリアルナンバを取得する。

  Arguments:    serialNo - 格納先。
                           終端文字も含めてOS_TWL_HWINFO_SERIALNO_LEN_MAXバイト格納される。
                           TWL上の動作でないとき常に0で埋められる。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_GetSerialNo(u8 *serialNo);


/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* TWL_OS_COMMON_OWNERINFO_EX_PRIVATE_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
