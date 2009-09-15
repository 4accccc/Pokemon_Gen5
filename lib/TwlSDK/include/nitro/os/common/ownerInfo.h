/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     ownerInfo.h

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

#ifndef NITRO_OS_COMMON_OWNERINFO_H_
#define NITRO_OS_COMMON_OWNERINFO_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include    <nitro/types.h>
#include    <nitro/spec.h>
#include    <nitro/spi/common/config.h>
#ifndef SDK_TWL
#include    <nitro/hw/common/mmap_shared.h>
#else
#include    <twl/hw/common/mmap_shared.h>
#endif
#include    <nitro/mi/memory.h>

#include <nitro/gx/gxcommon.h>



/* 本体情報 : 「お気に入りの色」定義 (0-15) */
#define	OS_FAVORITE_COLOR_GRAY         0x00
#define	OS_FAVORITE_COLOR_BROWN        0x01
#define	OS_FAVORITE_COLOR_RED          0x02
#define	OS_FAVORITE_COLOR_PINK         0x03
#define	OS_FAVORITE_COLOR_ORANGE       0x04
#define	OS_FAVORITE_COLOR_YELLOW       0x05
#define	OS_FAVORITE_COLOR_LIME_GREEN   0x06
#define	OS_FAVORITE_COLOR_GREEN        0x07
#define	OS_FAVORITE_COLOR_DARK_GREEN   0x08
#define	OS_FAVORITE_COLOR_SEA_GREEN    0x09
#define	OS_FAVORITE_COLOR_TURQUOISE    0x0A
#define	OS_FAVORITE_COLOR_BLUE         0x0B
#define	OS_FAVORITE_COLOR_DARK_BLUE    0x0C
#define	OS_FAVORITE_COLOR_PURPLE       0x0D
#define	OS_FAVORITE_COLOR_VIOLET       0x0E
#define	OS_FAVORITE_COLOR_MAGENTA      0x0F
#define	OS_FAVORITE_COLOR_BITMASK      0x0F
#define	OS_FAVORITE_COLOR_MAX          0x10

/* 本体情報 : 「お気に入りの色」定数 (GXRgb) */
#define	OS_FAVORITE_COLOR_VALUE_GRAY         GX_RGB(12,16,19)
#define	OS_FAVORITE_COLOR_VALUE_BROWN        GX_RGB(23, 9, 0)
#define	OS_FAVORITE_COLOR_VALUE_RED          GX_RGB(31, 0, 3)
#define	OS_FAVORITE_COLOR_VALUE_PINK         GX_RGB(31,17,31)
#define	OS_FAVORITE_COLOR_VALUE_ORANGE       GX_RGB(31,18, 0)
#define	OS_FAVORITE_COLOR_VALUE_YELLOW       GX_RGB(30,28, 0)
#define	OS_FAVORITE_COLOR_VALUE_LIME_GREEN   GX_RGB(21,31, 0)
#define	OS_FAVORITE_COLOR_VALUE_GREEN        GX_RGB( 0,31, 0)
#define	OS_FAVORITE_COLOR_VALUE_DARK_GREEN   GX_RGB( 0,20, 7)
#define	OS_FAVORITE_COLOR_VALUE_SEA_GREEN    GX_RGB( 9,27,17)
#define	OS_FAVORITE_COLOR_VALUE_TURQUOISE    GX_RGB( 6,23,30)
#define	OS_FAVORITE_COLOR_VALUE_BLUE         GX_RGB( 0,11,30)
#define	OS_FAVORITE_COLOR_VALUE_DARK_BLUE    GX_RGB( 0, 0,18)
#define	OS_FAVORITE_COLOR_VALUE_PURPLE       GX_RGB(17, 0,26)
#define	OS_FAVORITE_COLOR_VALUE_VIOLET       GX_RGB(26, 0,29)
#define	OS_FAVORITE_COLOR_VALUE_MAGENTA      GX_RGB(31, 0,18)


/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
// 言語コード
typedef enum OSLanguage
{
    OS_LANGUAGE_JAPANESE = NVRAM_CONFIG_LANG_JAPANESE,  // 0: 日本語
    OS_LANGUAGE_ENGLISH = NVRAM_CONFIG_LANG_ENGLISH,    // 1: 英語
    OS_LANGUAGE_FRENCH = NVRAM_CONFIG_LANG_FRENCH,      // 2: フランス語
    OS_LANGUAGE_GERMAN = NVRAM_CONFIG_LANG_GERMAN,      // 3: ドイツ語
    OS_LANGUAGE_ITALIAN = NVRAM_CONFIG_LANG_ITALIAN,    // 4: イタリア語
    OS_LANGUAGE_SPANISH = NVRAM_CONFIG_LANG_SPANISH,    // 5: スペイン語
//#if ( SDK_NVRAM_FORMAT >= 300 )
    OS_LANGUAGE_CHINESE = NVRAM_CONFIG_LANG_CHINESE,    // 6: 中国語
    OS_LANGUAGE_HANGUL = NVRAM_CONFIG_LANG_HANGUL,      // 7: 韓国語
//#endif
    OS_LANGUAGE_CODE_MAX = NVRAM_CONFIG_LANG_CODE_MAX
}
OSLanguage;


// 文字列最大長
#define     OS_OWNERINFO_NICKNAME_MAX       NVRAM_CONFIG_NICKNAME_LENGTH
#define     OS_OWNERINFO_COMMENT_MAX        NVRAM_CONFIG_COMMENT_LENGTH


/*---------------------------------------------------------------------------*
    構造体定義
 *---------------------------------------------------------------------------*/
// 誕生日情報
typedef struct OSBirthday
{
    u8      month;                     // 月( 1 ～ 12 )
    u8      day;                       // 日( 1 ～ 31 )

}
OSBirthday;

// オーナー情報
typedef struct OSOwnerInfo
{
    u8      language;                  // 言語コード
    u8      favoriteColor;             // お気に入りの色 ( 0 ～ 15 )
    OSBirthday birthday;               // 誕生日
    u16     nickName[OS_OWNERINFO_NICKNAME_MAX + 1];
    // ニックネーム (Unicode(UTF16)で最大10文字、終端コードなし)
    u16     nickNameLength;            // ニックネーム文字数( 0 ～ 10 )
    u16     comment[OS_OWNERINFO_COMMENT_MAX + 1];
    // コメント (Unicode(UTF16)で最大26文字、終端コードなし)
    u16     commentLength;             // コメント文字数( 0 ～ 26 )

}
OSOwnerInfo;


/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         OS_GetOwnerInfo

  Description:  オーナー情報を取得する。

  Arguments:    info    -   オーナー情報を取得するバッファへのポインタ。
                            このバッファにデータが書き込まれる。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_GetOwnerInfo(OSOwnerInfo *info);

/*---------------------------------------------------------------------------*
  Name:         OS_GetOwnerRtcOffset

  Description:  オーナーがRTC設定を変更する度にその値に応じて増減するオフセット
                値(単位は秒)を取得する。
                再設定直前のRTC時刻から未来に進められた場合は進めた時間が加算
                される。過去に戻された場合は戻した時間が減算される。

  Arguments:    None.

  Returns:      s64 -   RTC設定オフセット値を返す。
 *---------------------------------------------------------------------------*/
s64     OS_GetOwnerRtcOffset(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetMacAddress

  Description:  無線ハードウェアのMACアドレスを取得する。

  Arguments:    macAddress  -   MACアドレスを取得するバッファへのポインタ。
                                このバッファに6バイトのデータが書き込まれる。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_GetMacAddress(u8 *macAddress);

/*---------------------------------------------------------------------------*
  Name:         OS_GetFavoriteColorTable

  Description:  OSOwnerInfo 構造体のメンバ favoriteColor の番号に対応する
                「お気に入りの色」の配列へのポインタを取得します。

  Arguments:    None.

  Returns:      対応する色値を格納した GXRgb 型の定数配列へのポインタ.
 *---------------------------------------------------------------------------*/
const GXRgb *OS_GetFavoriteColorTable(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetFavoriteColor

  Description:  指定の番号に対応する「お気に入りの色」を取得します。

  Arguments:    index  -   指定する「お気に入りの色」の番号.
                           このインデックスは「お気に入りの色」総数である
                           16 色(4bit)でマスクされます。

  Returns:      指定した番号に対応する GXRgb 型の色値.
 *---------------------------------------------------------------------------*/
static inline GXRgb OS_GetFavoriteColor(int index)
{
    return OS_GetFavoriteColorTable()[index & OS_FAVORITE_COLOR_BITMASK];
}

/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_OS_COMMON_OWNERINFO_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
