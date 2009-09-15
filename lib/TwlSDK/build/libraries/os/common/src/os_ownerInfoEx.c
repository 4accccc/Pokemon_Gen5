/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     ownerInfoEx.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-13#$
  $Rev: 9302 $
  $Author: hirose_kazuki $
 *---------------------------------------------------------------------------*/

#include    <nitro/os/common/ownerInfo.h>
#include    <nitro/os/common/systemWork.h>
#include    <nitro/os/common/emulator.h>
#include    <twl/os/common/ownerInfoEx.h>
#include    <twl/os/common/ownerInfoEx_private.h>
#include    <twl/hw/common/mmap_parameter.h>

#ifdef SDK_ARM9
#include    <twl/hw/ARM9/mmap_main.h>
#else //SDK_ARM7
#include    <twl/hw/ARM7/mmap_main.h>
#endif

/*---------------------------------------------------------------------------*
    定数
 *---------------------------------------------------------------------------*/

//
// オーナー情報の各値はメインメモリの固定アドレスにランチャによって読み出されている
#define  OS_ADDR_TWL_SETTINGSDATA   ( (OSTWLSettingsData *)HW_PARAM_TWL_SETTINGS_DATA )
#define  OS_ADDR_TWL_HWNORMALINFO   ( (OSTWLHWNormalInfo *)HW_PARAM_TWL_HW_NORMAL_INFO )


/*---------------------------------------------------------------------------*
    関数定義
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
void OS_GetOwnerInfoEx(OSOwnerInfoEx *info)
{
    NVRAMConfig *src;
    OSTWLSettingsData  *twlSettings;

    // NITROと同じ情報はNITROの領域から取得する(よってコピペ)
    src = (NVRAMConfig *)(OS_GetSystemWork()->nvramUserInfo);
    //info->language = (u8)(src->ncd.option.language);          // 言語コードはTWLモードとNITROモードで場合分け
    info->favoriteColor = (u8)(src->ncd.owner.favoriteColor);
    info->birthday.month = (u8)(src->ncd.owner.birthday.month);
    info->birthday.day = (u8)(src->ncd.owner.birthday.day);
    info->nickNameLength = (u16)(src->ncd.owner.nickname.length);
    info->commentLength = (u16)(src->ncd.owner.comment.length);
    MI_CpuCopy16(src->ncd.owner.nickname.str,
                 info->nickName, OS_OWNERINFO_NICKNAME_MAX * sizeof(u16));
    MI_CpuCopy16(src->ncd.owner.comment.str, info->comment, OS_OWNERINFO_COMMENT_MAX * sizeof(u16));
    info->nickName[OS_OWNERINFO_NICKNAME_MAX] = 0;
    info->comment[OS_OWNERINFO_COMMENT_MAX] = 0;

    // TWL情報の取得
    if( OS_IsRunOnTwl() )
    {
        twlSettings   = (OSTWLSettingsData*) OS_ADDR_TWL_SETTINGSDATA ;
        info->country = twlSettings->country;
        info->language = twlSettings->language;
    }
    else
    {
        info->country = 0;
        info->language = (u8)(src->ncd.option.language);
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsAvailableWireless

  Description:  無線モジュールのRFユニットの有効化／無効化情報を取得する。

  Arguments:    None.

  Returns:      有効のときTRUE/無効のときFALSE。
                TWL上の動作でないとき常にTRUE。
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static BOOL OS_IsAvailableWireless_ltdmain(void);
static BOOL OS_IsAvailableWireless_ltdmain(void)
{
    OSTWLSettingsData *p;
    p = (OSTWLSettingsData*)OS_ADDR_TWL_SETTINGSDATA;
    return (p->flags.isAvailableWireless == 0x1);
}
#include <twl/ltdmain_end.h>
#endif

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
BOOL OS_IsAvailableWireless(void)
{
    BOOL result;
    if( OS_IsRunOnTwl() )
    {
#ifdef SDK_TWL
        result = OS_IsAvailableWireless_ltdmain();
#endif
    }
    else
    {
        result = TRUE;
    }
    return (result);
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsAgreeEULA

  Description:  EULA同意済みかどうかを取得する。

  Arguments:    None.

  Returns:      BOOL - EULA同意済みの時TRUE、非同意の時FALSE。
                TWL上の動作でないとき常にFALSE。
 *---------------------------------------------------------------------------*/
BOOL OS_IsAgreeEULA(void)
{
    OSTWLSettingsData *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLSettingsData*)OS_ADDR_TWL_SETTINGSDATA;
        return (p->flags.isAgreeEULAFlagList & 0x01) ? TRUE : FALSE;
    }
    else
    {
        return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetAgreeEULAVersion

  Description:  同意したEULAのバージョンを取得する。

  Arguments:    None

  Returns:      agreedEulaVersion - 同意したEULAバージョン（0-255の数字）
 *---------------------------------------------------------------------------*/
u8 OS_GetAgreedEULAVersion( void )
{
    OSTWLSettingsData *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLSettingsData*)OS_ADDR_TWL_SETTINGSDATA;
        return p->agreeEulaVersion[ 0 ];
    }
    else
    {
        return 0;
    }
}


/*---------------------------------------------------------------------------*
  Name:         OS_GetROMHeaderEULAVersion

  Description:  アプリケーションのROMヘッダに登録されているEULAのバージョンを取得する。

  Arguments:    None

  Returns:      cardEulaVersion - アプリケーションのROMヘッダに登録されているEULAバージョン（0-255の数字）
 *---------------------------------------------------------------------------*/
u8 OS_GetROMHeaderEULAVersion( void )
{
    if( OS_IsRunOnTwl() )
    {
        // ROMヘッダの agree_EULA_version をハードコーディング
        return *(u8 *)( HW_TWL_ROM_HEADER_BUF + 0x020e );
    }
    else
    {
        return 0;
    }
}


/*---------------------------------------------------------------------------*
  Name:         OS_GetParentalControlInfoPtr

  Description:  ペアレンタルコントロール情報へのポインタを取得する。

  Arguments:    None.

  Returns:      ポインタを返す。
                TWL上の動作でないときはNULLを返す。
 *---------------------------------------------------------------------------*/
const OSTWLParentalControl* OS_GetParentalControlInfoPtr(void)
{
    OSTWLSettingsData *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLSettingsData*)OS_ADDR_TWL_SETTINGSDATA;
        return &(p->parentalControl);
    }
    else
    {
        return NULL;
    }
}

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
/* 以下のコードは TWL 拡張メモリ領域に配置 */
#ifdef    SDK_TWL
#include  <twl/ltdmain_begin.h>
#endif
BOOL OSi_IsParentalControledAppCore(u8* appRatingInfo);

BOOL
OSi_IsParentalControledAppCore(u8* appRatingInfo)
{
    OSTWLParentalControl*   p   =   &(((OSTWLSettingsData*)OS_ADDR_TWL_SETTINGSDATA)->parentalControl);
    
    if (p->flags.isSetParentalControl)
    {
        if (p->ogn < OS_TWL_PCTL_OGN_MAX)
        {
            if (appRatingInfo == NULL)
            {
                /* 調査するアプリケーションのレーティング情報が不明 */
                OS_TWarning("Invalid pointer to Application rating information.\n");
                return TRUE;
            }
            else
            {
                if ((appRatingInfo[p->ogn] & OS_TWL_PCTL_OGNINFO_ENABLE_MASK) == 0)
                {
                    /* アプリケーション側に該当する審査団体でのレーティング情報が定義されていない */
                    OS_TWarning("Application doesn't have rating information for the organization.\n");
                    return TRUE;
                }
                else
                {
                    if ((appRatingInfo[p->ogn] & OS_TWL_PCTL_OGNINFO_ALWAYS_MASK) != 0)
                    {
                        return TRUE;
                    }
                    else
                    {
                        if ((appRatingInfo[p->ogn] & OS_TWL_PCTL_OGNINFO_AGE_MASK) > p->ratingAge)
                        {
                            return TRUE;
                        }
                    }
                }
            }
        }
        else
        {
            /* 本体設定内の審査団体指定が想定外 */
            OS_TWarning("Invalid rating organization index (%d) in LCFG.\n", p->ogn);
        }
    }
    return FALSE;
}
/* 以上のコードは TWL 拡張メモリ領域に配置 */
#ifdef    SDK_TWL
#include  <twl/ltdmain_end.h>
#endif

BOOL
OS_IsParentalControledApp(u8* appRatingInfo)
{
    if (OS_IsRunOnTwl() == TRUE)
    {
        return OSi_IsParentalControledAppCore(appRatingInfo);
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetMovableUniqueID

  Description:  本体間で移行可能なユニークID(HW情報に含まれる)を取得する。

  Arguments:    pUniqueID - 格納先(OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LENバイト格納される)。
                         TWL上の動作でないとき常に0で埋められる

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_GetMovableUniqueID(u8 *pUniqueID)
{
    OSTWLHWNormalInfo *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLHWNormalInfo*)OS_ADDR_TWL_HWNORMALINFO;
        MI_CpuCopy8( p->movableUniqueID, pUniqueID, OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*sizeof(u8) );
    }
    else
    {
        MI_CpuFill8( pUniqueID, 0, OS_TWL_HWINFO_MOVABLE_UNIQUE_ID_LEN*sizeof(u8) );
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetValidLanguageBitmap

  Description:  対応言語ビットマップを取得する。

  Arguments:    None.
  Returns:      対応言語ビットマップを返す。
                TWL上の動作でないときは"0"を返す。
 *---------------------------------------------------------------------------*/
u32 OS_GetValidLanguageBitmap( void )
{
    OSTWLHWSecureInfo *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLHWSecureInfo*)HW_HW_SECURE_INFO;
        return p->validLanguageBitmap;
    }
    else
    {
        return 0;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetUniqueIDPtr

  Description:  本体間で移行可能なユニークID(HW情報に含まれる)へのポインタを取得する。

  Arguments:    None.

  Returns:      ポインタを返す。
                TWL上の動作でないときはNULLを返す。
 *---------------------------------------------------------------------------*/
const u8* OS_GetMovableUniqueIDPtr(void)
{
    OSTWLHWNormalInfo *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLHWNormalInfo*)OS_ADDR_TWL_HWNORMALINFO;
        return (p->movableUniqueID);
    }
    else
    {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsForceDisableWireless

  Description:  無線を強制的に無効化するかどうかの情報。
                (HWセキュア情報に含まれる)を取得する。

  Arguments:    None.

  Returns:      強制的に無効化するときTRUE/無効化されないときFALSE。
                TWL上の動作でないとき常にFALSE。
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static BOOL OS_IsForceDisableWireless_ltdmain(void);
static BOOL OS_IsForceDisableWireless_ltdmain(void)
{
    OSTWLHWSecureInfo *p;
    p = (OSTWLHWSecureInfo*)HW_HW_SECURE_INFO;
    return (p->flags.forceDisableWireless == 0x1);
}
#include <twl/ltdmain_end.h>
#endif

BOOL OS_IsForceDisableWireless(void)
{
    BOOL  result;
    if( OS_IsRunOnTwl() )
    {
#ifdef SDK_TWL
        result = OS_IsForceDisableWireless_ltdmain();
#endif
    }
    else
    {
        result = FALSE;
    }
    return (result);
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetRegion

  Description:  リージョン情報を取得する。

  Arguments:    None.

  Returns:      リージョン番号を返す。
                TWL上の動作でないとき常に0を返す。
 *---------------------------------------------------------------------------*/
OSTWLRegion OS_GetRegion(void)
{
    OSTWLRegion result;
    OSTWLHWSecureInfo *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLHWSecureInfo*)HW_HW_SECURE_INFO;
        result = (OSTWLRegion)(p->region);
    }
    else
    {
        result = (OSTWLRegion)0;
    }
    return (result);
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetSerialNo

  Description:  シリアルナンバを取得する。

  Arguments:    serialNo - 格納先。
                           終端文字も含めてOS_TWL_HWINFO_SERIALNO_LEN_MAXバイト格納される。
                           TWL上の動作でないとき常に0で埋められる。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_GetSerialNo(u8 *serialNo)
{
    OSTWLHWSecureInfo *p;
    if( OS_IsRunOnTwl() )
    {
        p = (OSTWLHWSecureInfo*)HW_HW_SECURE_INFO;
        MI_CpuCopy8( p->serialNo, serialNo, OS_TWL_HWINFO_SERIALNO_LEN_MAX*sizeof(u8) );
    }
    else
    {
        MI_CpuFill8( serialNo, 0, OS_TWL_HWINFO_SERIALNO_LEN_MAX*sizeof(u8) );
    }
}


/*---------------------------------------------------------------------------*
  Name:         OS_GetWirelessFirmwareData

  Description:  無線ファームウェア用データへのポインタを取得する。

  Arguments:    None.
  Returns:      無線ファームウェア用データへのポインタ
 *---------------------------------------------------------------------------*/
OSTWLWirelessFirmwareData *OS_GetWirelessFirmwareData(void)
{
    if( OS_IsRunOnTwl() )
    {
        return (OSTWLWirelessFirmwareData*)HW_PARAM_WIRELESS_FIRMWARE_DATA;
    }
    else
    {
        return NULL;
    }
}


/*---------------------------------------------------------------------------*
  Name:         OS_GetRegionCodeA3

  Description:  EC / NUP で使うリージョンコードに対応する文字列を取得する。

  Arguments:    region -   リージョンコード

  Returns:      リージョンコードに対応する文字列へのポインタを返す。
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl/ltdmain_begin.h>
static const char* OSi_GetRegionCodeA3_ltdmain(OSTWLRegion region);
static const char* OSi_GetRegionCodeA3_ltdmain(OSTWLRegion region)
{
    const char* regionList[] =
    {
        "JPN",  // OS_TWL_REGION_JAPAN     = 0,  // NCL
        "USA",  // OS_TWL_REGION_AMERICA   = 1,  // NOA
        "EUR",  // OS_TWL_REGION_EUROPE    = 2,  // NOE
        "AUS",  // OS_TWL_REGION_AUSTRALIA = 3,  // NAL
        "CHN",  // OS_TWL_REGION_CHINA     = 4,  // IQue
        "KOR"   // OS_TWL_REGION_KOREA     = 5,  // NOK
    };
    
    if (region >= sizeof(regionList)/sizeof(regionList[0]))
    {
        OS_TWarning("Invalide region code.(%d)", region);
        return NULL;
    }
    return regionList[region];
}
#include <twl/ltdmain_end.h>
#endif /* SDK_TWL */

const char* OS_GetRegionCodeA3(OSTWLRegion region)
{
    if( OS_IsRunOnTwl() )
    {
#ifdef SDK_TWL
        return OSi_GetRegionCodeA3_ltdmain(region);
#else /* SDK_TWL */
        return NULL;
#endif /* SDK_TWL */
    }
    else
    {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetISOCountryCodeA2

  Description:  TWL独自の国および地域コード番号を ISO 3166-1 alpha-2 に変換する。

  Arguments:    twlCountry -   国および地域コード

  Returns:      国および地域コードに対応する文字列へのポインタを返す。
                TWL上の動作でないときNULLを返す。
 *---------------------------------------------------------------------------*/
extern const int    OSi_CountryCodeListNumEntries;
extern const char*  OSi_CountryCodeList[];

const char* OS_GetISOCountryCodeA2(u8 twlCountry)
{
    const char* cca2 = NULL;
    if( OS_IsRunOnTwl() )
    {
#ifdef SDK_TWL
        if (twlCountry < OSi_CountryCodeListNumEntries)
        {
            cca2 = OSi_CountryCodeList[twlCountry];
        }
#ifndef SDK_FINALROM
        if (!cca2)
        {
            OS_TWarning("Invalid country code(%d)\n", twlCountry);
        }
#endif /* SDK_FINALROM */
#endif /* SDK_TWL */
        return cca2;
    }
    else  // TWL上の動作でないとき
    {
        return NULL;
    }
}

#undef OS_TWL_COUNTRY_NAME_MAX

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
