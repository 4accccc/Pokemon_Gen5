/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     ownerInfoEx.h

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

#ifndef TWL_OS_COMMON_OWNERINFO_EX_H_
#define TWL_OS_COMMON_OWNERINFO_EX_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include    <twl/types.h>
#include    <twl/spec.h>
#include    <nitro/std.h>
#ifndef SDK_TWL
#include    <nitro/hw/common/mmap_shared.h>
#else
#include    <twl/hw/common/mmap_shared.h>
#endif

#include <nitro/os/common/ownerInfo.h>


/*---------------------------------------------------------------------------*
    定数
 *---------------------------------------------------------------------------*/
// リージョンコード
typedef enum OSTWLRegionCode
{
    OS_TWL_REGION_JAPAN     = 0,
    OS_TWL_REGION_AMERICA   = 1,
    OS_TWL_REGION_EUROPE    = 2,
    OS_TWL_REGION_AUSTRALIA = 3,
    OS_TWL_REGION_CHINA     = 4,
    OS_TWL_REGION_KOREA     = 5,
    OS_TWL_REGION_MAX
} OSTWLRegion;

#define OS_TWL_HWINFO_SERIALNO_LEN_MAX        15                                // 本体シリアルNo.長Max(終端付きなので、14bytesまで拡張可)
#define OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN   16                                // 移行可能なユニークID
#define OS_TWL_NICKNAME_LENGTH                OS_OWNERINFO_NICKNAME_MAX         // ニックネーム長
#define OS_TWL_COMMENT_LENGTH                 OS_OWNERINFO_COMMENT_MAX          // コメント長
#define OS_TWL_PCTL_PASSWORD_LENGTH           4                                 // 暗証番号の桁数
#define OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX  64                                // 秘密の質問の回答、UTF-16で最大 MAX 文字


/*---------------------------------------------------------------------------*
    構造体
 *---------------------------------------------------------------------------*/

// TWLオーナ情報
typedef struct OSOwnerInfoEx
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
    // TWLのみで取得可能なデータ
    u8      country;                   // 国および地域コード
    u8      padding;

}
OSOwnerInfoEx;


// ペアレンタルコントロール
// 審査団体
typedef enum OSTWLRatingOgn
{
    OS_TWL_PCTL_OGN_CERO        =   0,  // CERO                         // 日本
    OS_TWL_PCTL_OGN_ESRB        =   1,  // ESRB                         // 北米
    OS_TWL_PCTL_OGN_RESERVED2   =   2,  // BBFC [obsolete]
    OS_TWL_PCTL_OGN_USK         =   3,  // USK                          // ドイツ
    OS_TWL_PCTL_OGN_PEGI_GEN    =   4,  // PEGI general                 // 欧州
    OS_TWL_PCTL_OGN_RESERVED5   =   5,  // PEGI Finland [obsolete]
    OS_TWL_PCTL_OGN_PEGI_PRT    =   6,  // PEGI Portugal                // ポルトガル
    OS_TWL_PCTL_OGN_PEGI_BBFC   =   7,  // PEGI and BBFC Great Briten   // イギリス
    OS_TWL_PCTL_OGN_AGCB        =   8,  // AGCB                         // オーストラリア、ニュージーランド
    OS_TWL_PCTL_OGN_GRB         =   9,  // GRB                          // 韓国

    OS_TWL_PCTL_OGN_MAX         =   16

} OSTWLRatingOgn;


// ペアレンタルコントロール情報
typedef struct OSTWLParentalControl
{
    struct {
        u32             isSetParentalControl : 1;   // ペアレンタルコントロール設定済みか？
        u32             pictoChat :1;               // ピクトチャットを起動制限するか？
        u32             dsDownload :1;              // DSダウンロードプレイを起動制限するか？
        u32             browser :1;                 // フルブラウザを起動制限するか？
        u32             prepaidPoint :1;            // ポイントを使用制限するか？
        u32             photoExchange : 1;          // 写真の交換を制限するか？
        u32             ugc : 1;                    // ユーザー作成コンテンツの制限をするか？
        u32             rsv :25;
    } flags;
    u8                  rsv1[ 3 ];
    u8                  ogn;                        // 審査団体
    u8                  ratingAge;                  // レーティング（年齢）値
    u8                  secretQuestionID;           // 秘密の質問文 ID
    u8                  secretAnswerLength;         // 秘密の質問への回答文字数
    u8                  rsv[ 2 ];
    char                password[ OS_TWL_PCTL_PASSWORD_LENGTH + 1 ];   // 暗証番号、終端コードあり
    u16                 secretAnswer[ OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1 ];  // UTF16,秘密の質問への回答、終端コードあり
} OSTWLParentalControl;   //  148bytes

// アプリケーション側のレーティング情報フォーマット
#define OS_TWL_PCTL_OGNINFO_ENABLE_MASK     0x80
#define OS_TWL_PCTL_OGNINFO_ALWAYS_MASK     0x40
#define OS_TWL_PCTL_OGNINFO_AGE_MASK        0x1f


/*---------------------------------------------------------------------------*
    関数宣言
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         OS_GetOwnerInfoEx

  Description:  オーナー情報を取得する。TWL版。
                NITROにも存在する情報はNITRO版と同じ場所から取得する。
                言語コード(laungage)はTWLとNITROで異なる可能性があるため
                TWL側の情報から取得する。

  Arguments:    info    -   オーナー情報を取得するバッファへのポインタ。
                            このバッファにデータが書き込まれる。
                            (*)TWLの情報はTWLモードでしか取得できない。
                               (TWL上の動作でないとき常に0となる。)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_GetOwnerInfoEx(OSOwnerInfoEx *info);

/*---------------------------------------------------------------------------*
  Name:         OS_IsAvailableWireless

  Description:  無線モジュールのRFユニットの有効化／無効化情報を取得する。

  Arguments:    None.

  Returns:      有効のときTRUE/無効のときFALSE。
                TWL上の動作でないとき常にTRUE。
 *---------------------------------------------------------------------------*/
BOOL    OS_IsAvailableWireless(void);

/*---------------------------------------------------------------------------*
  Name:         OS_IsAgreeEULA

  Description:  この本体でEULA同意済みかどうかを取得する。

  Arguments:    None.

  Returns:      BOOL - EULA同意済みの時TRUE、非同意の時FALSE。
                TWL上の動作でないとき常にFALSE。
 *---------------------------------------------------------------------------*/
BOOL    OS_IsAgreeEULA(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetAgreeEULAVersion

  Description:  この本体で同意したEULAのバージョンを取得する。

  Arguments:    None

  Returns:      agreedEulaVersion - 同意したEULAバージョン（0-255の数字）
 *---------------------------------------------------------------------------*/
u8    OS_GetAgreedEULAVersion( void );

/*---------------------------------------------------------------------------*
  Name:         OS_GetROMHeaderEULAVersion

  Description:  アプリケーションのROMヘッダに登録されているEULAのバージョンを取得する。

  Arguments:    None

  Returns:      cardEulaVersion - アプリケーションのROMヘッダに登録されているEULAバージョン（0-255の数字）
 *---------------------------------------------------------------------------*/
u8    OS_GetROMHeaderEULAVersion( void );

/*---------------------------------------------------------------------------*
  Name:         OS_GetParentalControlPtr

  Description:  ペアレンタルコントロール情報へのポインタを取得する。

  Arguments:    None.

  Returns:      ポインタを返す。
                TWL上の動作でないときはNULLを返す。
 *---------------------------------------------------------------------------*/
const OSTWLParentalControl* OS_GetParentalControlInfoPtr(void);

/*---------------------------------------------------------------------------*
  Name:         OS_IsParentalControledApp

  Description:  アプリケーションの起動にペアレンタルコントロール制限をかけるべ
                きであるかどうか判定する。

  Arguments:    appRatingInfo   -   アプリケーションの ROM ヘッダ内のオフセット
                                    0x2f0 位置に埋め込まれている 16 バイトのレ
                                    ーティング情報へのポインタを指定する。

  Returns:      ペアレンタルコントロール制限をかけてパスワード入力を促すべき場
                合に TRUE を返す。制限せずに起動してよいアプリケーションである
                場合は FALSE を返す。
 *---------------------------------------------------------------------------*/
BOOL    OS_IsParentalControledApp(u8* appRatingInfo);

/*---------------------------------------------------------------------------*
  Name:         OS_GetMovableUniqueID

  Description:  本体間で移行可能なユニークID(HW情報に含まれる)を取得する。

  Arguments:    pUniqueID - 格納先(OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LENバイト格納される)。
                         TWL上の動作でないとき常に0で埋められる

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_GetMovableUniqueID(u8 *pUniqueID);

/*---------------------------------------------------------------------------*
  Name:         OS_GetUniqueIDPtr

  Description:  本体間で移行可能なユニークID(HW情報に含まれる)へのポインタを取得する。

  Arguments:    None.

  Returns:      ポインタを返す。
                TWL上の動作でないときはNULLを返す。
 *---------------------------------------------------------------------------*/
const u8* OS_GetMovableUniqueIDPtr(void);

/*---------------------------------------------------------------------------*
  Name:         OS_IsForceDisableWireless

  Description:  無線を強制的に無効化するかどうかの情報。
                (HWセキュア情報に含まれる)を取得する。

  Arguments:    None.

  Returns:      強制的に無効化するときTRUE/無効化されないときFALSE。
                TWL上の動作でないとき常にFALSE。
 *---------------------------------------------------------------------------*/
BOOL    OS_IsForceDisableWireless(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetRegion

  Description:  リージョン情報を取得する。

  Arguments:    None.

  Returns:      リージョン番号を返す。
                TWL上の動作でないとき常に0を返す。
 *---------------------------------------------------------------------------*/
OSTWLRegion  OS_GetRegion(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetRegionCodeA3

  Description:  リージョンコードに対応する文字列を取得する。

  Arguments:    region -   リージョンコード

  Returns:      リージョンコードに対応する文字列へのポインタを返す。
 *---------------------------------------------------------------------------*/
const char* OS_GetRegionCodeA3(OSTWLRegion region);

/*---------------------------------------------------------------------------*
  Name:         OS_GetISOCountryCodeA2

  Description:  TWL独自の国および地域コード番号を ISO 3166-1 alpha-2 に変換する。

  Arguments:    twlCountry -   国および地域コード

  Returns:      国および地域コードに対応する文字列へのポインタを返す。
 *---------------------------------------------------------------------------*/
const char* OS_GetISOCountryCodeA2(u8 twlCountry);

/*---------------------------------------------------------------------------*
  Name:         OS_IsRestrictPictoChatBoot

  Description:  ペアレンタルコントロールの設定のうち、
                ピクトチャットの起動制限がかかっているかどうかを判定する。

  Arguments:    None.

  Returns:      制限がかけられている場合は TRUE, かけられていない場合は FALSE。
 *---------------------------------------------------------------------------*/
static inline BOOL OS_IsRestrictPictoChatBoot( void )
{
    return (BOOL)OS_GetParentalControlInfoPtr()->flags.isSetParentalControl && (BOOL)OS_GetParentalControlInfoPtr()->flags.pictoChat;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsRestrictDSDownloadBoot

  Description:  ペアレンタルコントロールの設定のうち、
                DS ダウンロードプレイの起動制限がかかっているかどうかを判定する。

  Arguments:    None.

  Returns:      制限がかけられている場合は TRUE、かけられていない場合は FALSE。
 *---------------------------------------------------------------------------*/
static inline BOOL OS_IsRestrictDSDownloadBoot( void )
{
    return (BOOL)OS_GetParentalControlInfoPtr()->flags.isSetParentalControl && (BOOL)OS_GetParentalControlInfoPtr()->flags.dsDownload;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsRestrictBrowserBoot

  Description:  ペアレンタルコントロールの設定のうち、
                DSi ブラウザーの起動制限がかかっているかどうかを判定する。

  Arguments:    None.

  Returns:      制限がかけられている場合は TRUE, かけられていない場合は FALSE。
 *---------------------------------------------------------------------------*/
static inline BOOL OS_IsRestrictBrowserBoot( void )
{
    return (BOOL)OS_GetParentalControlInfoPtr()->flags.isSetParentalControl && (BOOL)OS_GetParentalControlInfoPtr()->flags.browser;
}


/*---------------------------------------------------------------------------*
  Name:         OS_IsRestrictPrepaidPoint

  Description:  ペアレンタルコントロールの設定のうち、
                ニンテンドー DSi ポイントの仕様に関する制限がかかっているかどうかを判定する。

  Arguments:    None.

  Returns:      制限がかけられている場合は TRUE, かけられていない場合は FALSE。
 *---------------------------------------------------------------------------*/
static inline BOOL OS_IsRestrictPrepaidPoint( void )
{
    return (BOOL)OS_GetParentalControlInfoPtr()->flags.isSetParentalControl && (BOOL)OS_GetParentalControlInfoPtr()->flags.prepaidPoint;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsRestrictPhotoExchange

  Description:  ペアレンタルコントロールの設定のうち、
                写真データの交換に関する制限がかかっているかどうかを判定する。

  Arguments:    None.

  Returns:      制限がかけられている場合は TRUE, かけられていない場合は FALSE。
 *---------------------------------------------------------------------------*/
static inline BOOL OS_IsRestrictPhotoExchange( void )
{
    return (BOOL)OS_GetParentalControlInfoPtr()->flags.isSetParentalControl && (BOOL)OS_GetParentalControlInfoPtr()->flags.photoExchange;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsRestrictUGC

  Description:  ペアレンタルコントロールの設定のうち、一部のユーザー作成コンテンツの
                送受信に関する制限がかかっているかどうかを判定する。

  Arguments:    None.

  Returns:      制限がかけられている場合は TRUE, かけられていない場合は FALSE。
 *---------------------------------------------------------------------------*/
static inline BOOL OS_IsRestrictUGC( void )
{
    return (BOOL)OS_GetParentalControlInfoPtr()->flags.isSetParentalControl && (BOOL)OS_GetParentalControlInfoPtr()->flags.ugc;
}

/*---------------------------------------------------------------------------*
  Name:         OS_CheckParentalControlPassword

  Description:  与えられた文字列が、ペアレンタルコントロールの解除パスワードと
                一致するかどうかを判定する。

  Arguments:    password - 判定される文字列.

  Returns:      一致していた場合は TRUE, 一致していない場合は FALSE。
 *---------------------------------------------------------------------------*/
static inline BOOL OS_CheckParentalControlPassword( const char *password )
{
    return password && (STD_CompareLString(password, OS_GetParentalControlInfoPtr()->password, OS_TWL_PCTL_PASSWORD_LENGTH) == 0);
}

/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* TWL_OS_COMMON_OWNERINFO_EX_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
