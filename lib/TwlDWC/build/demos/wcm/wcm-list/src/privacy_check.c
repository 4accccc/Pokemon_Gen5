/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WCM - demos - wcm-list-2
  File:     privacy_check.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 1024 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

/*****************************************************************

///////////////////////////////////////////////
//                                           //
//     Infomation Element フォーマット詳細   //
//                                           //
///////////////////////////////////////////////
*
*WPA IE フォーマット
*    Element ID          ( 1 Byte) :     221(固定)
*    Length              ( 1 Byte) :     IE のバイト長
*    OUI                 ( 3 Byte) :     00:50:F2(固定)
*    SubType             ( 1 Byte) :     0x01(固定)
*    Version             ( 2 Byte) :     0x0001(固定)
*    GroupCipherType     ( 4 Byte) :     OUI(00：50：F2) + KeyType(*)
*    PairwiseSetCount    ( 2 Byte) :     サポートする PairwiseCipherType の数
*    PairwiseCipherType  (per 4 Byte) :  OUI(00：50：F2) + KeyType(*) のセットが
*                                        PairwiseSetCount の個数続く
*    AuthSetCount        ( 2 Byte) :     サポートする AuthenticationType の数
*    AuthenticationType  (per 4 Byte) :  OUI(00：50：F2) + AuthType(*) のセットが
*                                        AuthSetCount の個数続く
*    RSN Capability      ( 2 Byte) :     基本的に参照する必要なし
*                                         事前認証            ( 1 bit)
*                                         ペアなし            ( 1 bit)
*                                         PTK リプレイカウンタ( 2 bit)
*                                         GTK リプレイカウンタ( 2 bit)
*                                         Reserved            (10 bit)
*    PMK カウント        ( 2 Byte) :     詳細は仕様書参照。
*    PMK リスト          (per 16 Byte) : 詳細は仕様書参照。
*
*RSN(WPA2) IE フォーマット
*    Element ID          ( 1 Byte) :     48(固定)
*    Length              ( 1 Byte) :     IE のバイト長
*    Version             ( 2 Byte) :     0x0001(固定)
*    GroupCipherType     ( 4 Byte) :     OUI(00:0F:AC) + KeyType(*)
*    PairwiseSetCount    ( 2 Byte) :     サポートする PairwiseCipherType の数
*    PairwiseCipherType  (per 4 Byte) :  OUI(00:0F:AC) + KeyType(*) のセットが
*                                        PairwiseSetCount の個数続く
*    AuthSetCount        ( 2 Byte) :     サポートする AuthenticationType の数
*    AuthenticationType  (per 4 Byte) :  OUI(00:0F:AC) + AuthType(*) のセットが
*                                        AuthSetCount の個数続く
*    RSN Capability      ( 2 Byte) :     基本的に参照する必要なし
*                                         事前認証            ( 1 bit)
*                                         ペアなし            ( 1 bit)
*                                         PTK リプレイカウンタ( 2 bit)
*                                         GTK リプレイカウンタ( 2 bit)
*                                         Reserved            (10 bit)
*    PMK カウント        ( 2 Byte) :     詳細は仕様書参照。
*    PMK リスト          (per 16 Byte) : 詳細は仕様書参照。
*
*
* (*) KeyType :
*            0 ... GroupCipher に従う (PairwiseCipher のみ指定可能)
*            1 ... WEP40
*            2 ... TKIP
*            3 ... Reserved
*            4 ... CCMP(AES)
*            5 ... WEP104
*
* (*) AuthType :
*            1 ... 802.1X or PMK cache
*            2 ... PSK
*
*******************************************************************/

#include "privacy_check.h"



extern u8 gDeviceID; //無線デバイスID

/* 関数プロトタイプ */

u8 GetPrivacyMode(const WCMBssDesc *bssDesc);
u8 GetWPA2PrivacyMode(const WCMBssDesc *bssDesc);
u8 GetWPAPrivacyMode(const WCMBssDesc *bssDesc);

static int FindWifiProtectedAccess(u8** bufPtr, const WCMBssDesc* bssDesc);
static int FindInformationElement(u8** bufPtr, const WCMBssDesc* bssDesc, u8 elementID);
static int FindVendorSpecificIE(u8** bufPtr, const WCMBssDesc* bssDesc, u8 elementID, const u8* tag, u8 version);


/*-----------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         GetWPAPrivacyMode

  Description:  スキャンにより取得した BSSDesc から、AP のサポートする WPA 関連の暗号化方式を取得する。
                
  Arguments:    bssDesc       - 検索の対象となる WCMBssDesc 型。

  Returns:      u8            - WCM_PRIVACYMODE_* のいずれかを返す
 *---------------------------------------------------------------------------*/
u8 GetWPAPrivacyMode(const WCMBssDesc *bssDesc)
{
    u8* bufPtr;
    u8 multicastCipher = WCM_PRIVACYMODE_NONE;
    u8 unicastCipher = WCM_PRIVACYMODE_NONE;
    int length = FindWifiProtectedAccess(&bufPtr, bssDesc);
    
    if( length != -1 && (u32)length >  ( sizeof(u16) + sizeof(WPACipherType) +  sizeof(u16) ) ) //サイズチェックの前段として、PairwizeSetCount があるかをチェック
    {
        u8 oui[3] = {0x00, 0x50, 0xF2};	// WPA OUI
        u16 num;

        (void)MI_CpuCopy8(bufPtr + sizeof(u16) + sizeof(WPACipherType), &num, sizeof(16)); //Num の取得

        if( (u32)length <  sizeof(u16) + sizeof(WPACipherType) +  sizeof(u16) + (sizeof(WPACipherType)*num) )
        {
            return WCM_PRIVACYMODE_NONE; //Unicast Cipher が正しく受け取れない場合アリ
        }

        {
            WPACipherType group;

            (void)MI_CpuCopy8(bufPtr + sizeof(u16), &group, sizeof(WPACipherType));
            if( MI_CpuComp8( group.oui, oui, sizeof(u8) * 3) == 0)
            {
                switch( group.type )
                {
                    case WPA_KEYTYPE_WEP_40:
                        multicastCipher = WCM_PRIVACYMODE_WEP40;
                        break;
                    case WPA_KEYTYPE_TKIP:
                        multicastCipher = WCM_PRIVACYMODE_WPA_TKIP;
                        break;
                    case WPA_KEYTYPE_AES:
                        multicastCipher = WCM_PRIVACYMODE_WPA_AES;
                        break;
                    case WPA_KEYTYPE_WEP104:
                        multicastCipher = WCM_PRIVACYMODE_WEP104;
                        break;
                    default:
                        multicastCipher = WCM_PRIVACYMODE_NONE;
                        break;
                }
            }
        }

        if( multicastCipher == WCM_PRIVACYMODE_WEP104 || multicastCipher == WCM_PRIVACYMODE_WEP40 )
        {
            return multicastCipher;
        }
        
        /*
        基本的には Wii と同じ判定を用いるが、 MulticastCipherType と UnicastCipherType が異なる
        一部の無線 AP に対応するために、選択された方式が TKIP/AES であれば その方式が
        UnicastCipherType に定義されているか確認し、定義されていない場合は UnicastCipherType の方式を採用する。
        */
        {
            u16 num; // AP がサポートしている方式の数
            int i;

            (void)MI_CpuCopy8(bufPtr + sizeof(u16) + sizeof(WPACipherType), &num, sizeof(16)); //PairwiseSetCount の取得

            for(i=0 ;i < num; i++)
            {
                WPACipherType pairwise;

                (void)MI_CpuCopy8(bufPtr + sizeof(u16) + sizeof(WPACipherType) + sizeof(u16) + (sizeof(WPACipherType) * i), &pairwise, sizeof(WPACipherType));
                if( MI_CpuComp8( pairwise.oui, oui, sizeof(u8) * 3) == 0)
                {
                    u8 tmpCipher;
                    
                    switch( pairwise.type )
                    {
                    case WPA_KEYTYPE_WEP_40:
                        tmpCipher = WCM_PRIVACYMODE_WEP40;
                        break;
                    case WPA_KEYTYPE_TKIP:
                        tmpCipher = WCM_PRIVACYMODE_WPA_TKIP;
                        break;
                    case WPA_KEYTYPE_AES:
                        tmpCipher = WCM_PRIVACYMODE_WPA_AES;
                        break;
                    case WPA_KEYTYPE_WEP104:
                        tmpCipher = WCM_PRIVACYMODE_WEP104;
                        break;
                    case WPA_KEYTYPE_GPOUP_CIPHER:
                        tmpCipher = multicastCipher;
                        break;
                    default:
                        /* 未知の暗号化方式についてはノーケアとする */
                        tmpCipher = WCM_PRIVACYMODE_NONE;
                        break;
                    }
                    
                    // MulticastCipher と同じ方式があれば return
                    if( tmpCipher == multicastCipher )
                    {
                        return multicastCipher;
                    }
                    else
                    {
                        if( unicastCipher < tmpCipher )
                        {
                            unicastCipher = tmpCipher;
                        }
                    }
                }
            }
        return unicastCipher; //MulticastCipher と一致しなかったので、UnicastCipher を採用
        }
    }
    return WCM_PRIVACYMODE_NONE;
}

/*---------------------------------------------------------------------------*
  Name:         GetWPA2PrivacyMode

  Description:  スキャンにより取得した BSSDesc から、AP のサポートする WPA2 関連の暗号化方式を取得する。
                
  Arguments:    bssDesc       - 検索の対象となる WCMBssDesc 型。

  Returns:      u8            - WCM_PRIVACYMODE_* のいずれかを返す
 *---------------------------------------------------------------------------*/
u8 GetWPA2PrivacyMode(const WCMBssDesc *bssDesc)
{
    u8* bufPtr;
    u8 multicastCipher = WCM_PRIVACYMODE_NONE;
    u8 unicastCipher = WCM_PRIVACYMODE_NONE;
    int length = FindInformationElement(&bufPtr, bssDesc, ELEMENTID_RSN);
    
    // id = 48 (RSN)
    if( length != -1 && (u32)length > ( sizeof(u16) + sizeof(WPACipherType) +  sizeof(u16) ) ) //サイズチェックの前段として、PairwizeSetCount があるかをチェック
    {
        u8 oui[3] = {0x00, 0x0F, 0xAC};	// 802.11i
        u16 num;

        (void)MI_CpuCopy8(bufPtr + sizeof(u16) + sizeof(WPACipherType), &num, sizeof(16)); //Num の取得

        if( (u32)length <  sizeof(u16) + sizeof(WPACipherType) +  sizeof(u16) + (sizeof(WPACipherType)*num) )
        {
            return WCM_PRIVACYMODE_NONE; //Unicast Cipher が正しく受け取れない場合アリ
        }
        
        {
            WPACipherType group;

            (void)MI_CpuCopy8(bufPtr + sizeof(u16), &group, sizeof(WPACipherType));
            if( MI_CpuComp8( group.oui, oui, sizeof(u8) * 3) == 0)
            {
                switch( group.type )
                {
                    case WPA_KEYTYPE_WEP_40:
                        multicastCipher = WCM_PRIVACYMODE_WEP40;
                        break;
                    case WPA_KEYTYPE_TKIP:
                        multicastCipher = WCM_PRIVACYMODE_WPA2_TKIP;
                        break;
                    case WPA_KEYTYPE_AES:
                        multicastCipher = WCM_PRIVACYMODE_WPA2_AES;
                        break;
                    case WPA_KEYTYPE_WEP104:
                        multicastCipher = WCM_PRIVACYMODE_WEP104;
                        break;
                    default:
                        multicastCipher = WCM_PRIVACYMODE_NONE;
                        break;
                }
            }
        }

        if( multicastCipher == WCM_PRIVACYMODE_WEP104 || multicastCipher == WCM_PRIVACYMODE_WEP40 )
        {
            return multicastCipher;
        }
        
        /*
        基本的には Wii と同じ判定を用いるが、 MulticastCipherType と UnicastCipherType が異なる
        一部の無線 AP に対応するために、選択された方式が TKIP/AES であれば その方式が
        UnicastCipherType に定義されているか確認し、定義されていない場合は UnicastCipherType の方式を採用する。
        */
        {
            u16 num; // AP がサポートしている方式の数
            int i;

            //(void)MI_CpuCopy8(bufPtr + sizeof(u8) + sizeof(WPACipherType), &num, sizeof(16));
            (void)MI_CpuCopy8(bufPtr + + sizeof(u16) + sizeof(WPACipherType), &num, sizeof(16)); // miz

            for(i=0 ;i < num; i++)
            {
                WPACipherType pairwise;

                //(void)MI_CpuCopy8(bufPtr + sizeof(u8) + sizeof(WPACipherType) + sizeof(u16) + (sizeof(WPACipherType) * i), &pairwise, sizeof(WPACipherType));
                (void)MI_CpuCopy8(bufPtr + sizeof(u16) + sizeof(WPACipherType) + sizeof(u16) + (sizeof(WPACipherType) * i), &pairwise, sizeof(WPACipherType)); // miz
                if( MI_CpuComp8( pairwise.oui, oui, sizeof(u8) * 3) == 0)
                {
                    u8 tmpCipher;
                    
                    switch( pairwise.type )
                    {
                    case WPA_KEYTYPE_WEP_40:
                        tmpCipher = WCM_PRIVACYMODE_WEP40;
                        break;
                    case WPA_KEYTYPE_TKIP:
                        tmpCipher = WCM_PRIVACYMODE_WPA2_TKIP;
                        break;
                    case WPA_KEYTYPE_AES:
                        tmpCipher = WCM_PRIVACYMODE_WPA2_AES;
                        break;
                    case WPA_KEYTYPE_WEP104:
                        tmpCipher = WCM_PRIVACYMODE_WEP104;
                        break;
                    case WPA_KEYTYPE_GPOUP_CIPHER:
                        tmpCipher = multicastCipher;
                        break;
                    default:
                        /* 未知の暗号化方式についてはノーケアとする */
                        tmpCipher = WCM_PRIVACYMODE_NONE;
                        break;
                    }
                    
                    // MulticastCipher と同じ方式があれば return
                    if( tmpCipher == multicastCipher )
                    {
                        return multicastCipher;
                    }
                    else
                    {
                        if( unicastCipher < tmpCipher )
                        {
                            unicastCipher = tmpCipher;
                        }
                    }
                }
            }
        return unicastCipher; //MulticastCipher と一致しなかったので、UnicastCipher を採用
        }
    }
    return WCM_PRIVACYMODE_NONE;
}

/*---------------------------------------------------------------------------*
  Name:         GetPrivacyMode

  Description:  スキャンにより取得した BSSDesc から、AP のサポートする暗号化方式を取得する。
                
  Arguments:    bssDesc       - 検索の対象となる WCMBssDesc 型。

  Returns:      u8            - WCM_PRIVACYMODE_* のいずれかを返す
 *---------------------------------------------------------------------------*/
u8 GetPrivacyMode(const WCMBssDesc *bssDesc)
{
    u8 wpa2Key, wpaKey;

    wpa2Key = GetWPA2PrivacyMode(bssDesc);

    if ( wpa2Key != WCM_PRIVACYMODE_NONE )
    {
        return wpa2Key;
    }

    wpaKey = GetWPAPrivacyMode(bssDesc);

    if ( wpaKey != WCM_PRIVACYMODE_NONE )
    {
        return wpaKey;
    }

    /* WPA IE と RSN(WPA) IE が無かった場合 */
    if( wpa2Key ==  WCM_PRIVACYMODE_NONE && wpaKey == WCM_PRIVACYMODE_NONE)
    {
        /* WPA or WPA2 の暗号化が使われていない かつ 暗号化はされている場合は WEP */
        if ( bssDesc && ( ( bssDesc->capaInfo & 0x0010 ) == 0x0010 ) )
        {
            return WCM_PRIVACYMODE_WEP; /* この場合 WEP の鍵長は不明な事に留意 */
        }
        else /* 暗号化されていない */
        {
            return WCM_PRIVACYMODE_NONE;
        }
    }

    return WCM_PRIVACYMODE_NONE;
}

/*---------------------------------------------------------------------------*
  Name:         FindInformationElement

  Description:  特定の Information Element を取得する。
                
  Arguments:    bufPtr        - 見つかった情報エレメントのVersion以下のデータの先頭のポインタが格納される。
                bssDesc       - 検索の対象となる WCMBssDesc 型。
                elementID     - 検索の対象となる情報エレメントID

  Returns:      int 存在した場合は Lengh フィールドを、存在しない場合は -1 を返す。
 *---------------------------------------------------------------------------*/
static int FindInformationElement(u8** bufPtr, const WCMBssDesc* bssDesc, u8 elementID)
{
    int i;
#ifdef SDK_TWL
    if(gDeviceID == WCM_DEVICEID_WM) /* 旧無線を使った場合 */
#endif
    {
        WMOtherElements elem = WM_GetOtherElements((WMBssDesc*)bssDesc);

        for(i = 0; i < elem.count; i++)
        {
            if(elem.element[i].id == elementID)
            {
                *bufPtr = elem.element[i].body;
                return (int)(elem.element[i].length);
            }
        }
    }
#ifdef SDK_TWL
    else
    {
        if(OS_IsRunOnTwl())
        {
            if(gDeviceID == WCM_DEVICEID_NWM) /* 新無線を使った場合 */
            {
                NWMInfoElements *elem = NWM_GetInfoElements((NWMBssDesc*)bssDesc, elementID);

                if( elem != NULL)
                {
                    *bufPtr = (u8*)(elem->element);
                    
                    return (int)(elem->length);
                }
            }
        }
    }
#endif
    return -1;
}

/*---------------------------------------------------------------------------*
  Name:         FindWifiProtectedAccess

  Description:  WPA Information Element を取得する。
                
  Arguments:    bufPtr        - 見つかった情報エレメントのVersion以下のデータの先頭のポインタが格納される。
                bssDesc       - 検索の対象となる WCMBssDesc 型。

  Returns:      存在した場合はTRUE
 *---------------------------------------------------------------------------*/

static int FindWifiProtectedAccess(u8** bufPtr, const WCMBssDesc* bssDesc)
{
    const u8 tag[4] = { 0x00, 0x50, 0xF2, 0x01 };   // WPA 固有のタグ
    return FindVendorSpecificIE(bufPtr, bssDesc, ELEMENTID_VENDOR_SPECIFIC, tag, 0x01);
}

/*---------------------------------------------------------------------------*
  Name:         FindVendorSpecificIE

  Description:  Vendor Specific Information Element を取得する。
                ただし、ID, Length, tag を省く。
                
  Arguments:    bufPtr        - 見つかった情報エレメントのVersion以下のデータの先頭のポインタが格納される。
                bssDesc       - 検索の対象となる WCMBssDesc 型。
                elementID     - 検索の対象となる情報エレメントID (Vendor Specific IE は 221)
                tag           - WPA 固有のタグとして version フィールドの前に付加される 4バイト
                version       - 検索を行う情報エレメントの version 値。

  Returns:      int 存在した場合は Lengh フィールドを、存在しない場合は -1 を返す。
 *---------------------------------------------------------------------------*/
static int FindVendorSpecificIE(u8** bufPtr, const WCMBssDesc* bssDesc, u8 elementID, const u8* tag, u8 version)
{
    int result = -1;

#ifdef SDK_TWL
    if( OS_IsRunOnTwl() )
    {
        if(gDeviceID == WCM_DEVICEID_NWM) /* 新無線を使った場合 */
        {
            // WM と NWM の BssDesc における IE の扱いが少し異なるため処理を分ける
            NWMInfoElements *elem = NWM_GetVenderInfoElements((NWMBssDesc*)bssDesc, elementID, (u8*)tag);
    
            if( elem != NULL)
            {
                *bufPtr = (u8*)(elem->element) + 4;
                result = (int)(elem->length);
            }
        }
    }
#endif
    if(gDeviceID == WCM_DEVICEID_WM ) /* 旧無線を使った場合 */
    {
        int i;
        WMOtherElements elem = WM_GetOtherElements((WMBssDesc*)bssDesc);

        if( bssDesc != NULL)
        {
            for( i = 0; i < elem.count; i++)
            {
                if( elementID == elem.element[i].id)
                {
                    u8 elem_version = *(elem.element[i].body + 4);
                    // tag と version のチェック
                    if ( !memcmp( elem.element[i].body, tag, 4) && elem_version == version)
                    {
                        *bufPtr = elem.element[i].body + 4;
                        result = (int)(elem.element[i].length);
                        break;
                    }
                }
            }
        }
    }
    
    return result;
}