/*---------------------------------------------------------------------------*
  Project:  TwlWiFi - NCFG - include
  File:     ncfg_info.h

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

#ifndef TWLWIFI_NCFG_INFO_H_
#define TWLWIFI_NCFG_INFO_H_

#ifdef __cplusplus

extern "C" {
#endif

/*===========================================================================*/
#include <nitro/types.h>
#include <nitro/wm.h>
#ifdef SDK_TWL
#include <twl/nwm.h>
#endif

/*---------------------------------------------------------------------------*
    定数 定義
 *---------------------------------------------------------------------------*/

// 設定方法
typedef enum
{
    NCFG_SETTYPE_MANUAL        = 0x00,  // 手動
    NCFG_SETTYPE_AOSS,                  // AOSS
    NCFG_SETTYPE_RAKURAKU,              // らくらく無線スタート

    // 以下、TWL における拡張設定 (DS 互換設定では使用不可)
    NCFG_SETTYPE_WPS_WEP       = 0x03,  // WPS (WEP 使用)

    NCFG_SETTYPE_MANUAL_WPA    = 0x10,  // 手動 (WPA 使用)
    NCFG_SETTYPE_AOSS_WPA,              // AOSS (WPA 使用)
    NCFG_SETTYPE_RAKURAKU_WPA,          // らくらく無線スタート (WPA 使用)
    NCFG_SETTYPE_WPS_WPA,               // WPS (WPA 使用)

    NCFG_SETTYPE_NOT           = 0xFF   // 未設定
} NCFGSetType;

#define NCFG_SETTYPE_WPS NCFG_SETTYPE_WPS_WPA


// WEP 暗号化モード種別
// WCM_WEPMODE_* よりコピー
typedef enum
{
    NCFG_WEPMODE_NONE = 0,              // 暗号化なし
    NCFG_WEPMODE_40   = 1,              // RC4 (  40 ビット ) 暗号化モード
    NCFG_WEPMODE_104  = 2,              // RC4 ( 104 ビット ) 暗号化モード
    NCFG_WEPMODE_128  = 3               // RC4 ( 128 ビット ) 暗号化モード
} NCFGWEPMode;

// WEP 表記
// util/setting.h よりコピー
typedef enum
{
    NCFG_WEP_NOTATION_HEX,              // 16進数表記
    NCFG_WEP_NOTATION_ASCII             // ASCII コード表記
} NCFGWEPNotation;

// Proxy モード
typedef enum
{
    NCFG_PROXY_MODE_NONE               =    0x00,  /* プロキシを使用しない */
    NCFG_PROXY_MODE_NORMAL             =    0x01   /* 通常のプロキシ */
} NCFGProxyMode;

// Proxy 認証形式
typedef enum
{
    NCFG_PROXY_AUTHTYPE_NONE           =    0x00,  /* 認証なし */
    NCFG_PROXY_AUTHTYPE_BASIC          =    0x01   /* BASIC 認証 */
} NCFGProxyAuthType;

// WPA モード種別
typedef enum
{
    NCFG_WPAMODE_WPA_PSK_TKIP   =   0x04,     /* WPA-PSK  ( TKIP ) 暗号 */
    NCFG_WPAMODE_WPA2_PSK_TKIP  =   0x05,     /* WPA2-PSK ( TKIP ) 暗号 */
    NCFG_WPAMODE_WPA_PSK_AES    =   0x06,     /* WPA-PSK  ( AES  ) 暗号 */
    NCFG_WPAMODE_WPA2_PSK_AES   =   0x07      /* WPA2-PSK ( AES  ) 暗号 */
} NCFGWPAMode;

// 各種フラグ
typedef enum
{
    NCFG_FLAG_NONE              =   0x00,
    NCFG_FLAG_ALWAYS_ACTIVE     =   0x01,     /* PowerSave を無効にする (未使用) */
    NCFG_FLAG_SAFE_MTU          =   0x02      /* MTU を 576 バイトにする (未使用/削除予定) */
} NCFGFlag;


// ワークメモリサイズ
#define NCFG_CHECKCONFIG_WORK_SIZE      0x700
#define NCFG_READCONFIG_WORK_SIZE       0x700
#define NCFG_WRITECONFIG_WORK_SIZE      0x700

#define NCFG_CHECKCONFIGEX_WORK_SIZE    0x1000
#define NCFG_READCONFIGEX_WORK_SIZE     0x1000
#define NCFG_WRITECONFIGEX_WORK_SIZE    0x1000


/*---------------------------------------------------------------------------*
    マクロ 定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    構造体 定義
 *---------------------------------------------------------------------------*/

// アクセスポイント接続情報
typedef struct NCFGApInfo
{
    u8  ispId    [0x20];       // ISP 用 ID
    u8  ispPass  [0x20];       // ISP 用パスワード
    u8  ssid  [2][0x20];       // SSID (ssid[1] : AOSS WEP40 用)
    u8  wep   [4][0x10];       // WEP キー
    u8  ip       [0x04];       // IP アドレス
    u8  gateway  [0x04];       // デフォルトゲートウェイ
    u8  dns   [2][0x04];       // DNS
    u8  netmask;               // サブネットマスク
    u8  wep2  [4][0x05];       // WEP キー (AOSS 用)
    u8  authType;              // 認証方法
    u8  wepMode      : 2;      // WEP モード
    u8  wepNotation  : 6;      // WEP 表記
    u8  setType;               // 設定方法
    u8  ssidLength[2];         // SSID 長 (0 ならば ssid に NULL が来るまで)
    u16 mtu;                   // MTU 長 (0 ならばデフォルト値で解釈)
    u8  rsv      [0x02];       // 予備
    u8  flags;                 // フラグ (NCFG_FLAG_*)
    u8  state;                 // 使用状況
}  NCFGApInfo;

// 拡張アクセスポイント接続情報
typedef struct NCFGApInfoEx
{
    union {
        struct {
            u8  wpaKey[0x20];      // WPA PMK
            u8  wpaPassphrase[0x40];   // WPA パスフレーズ (NULL 終端文字列 or 64桁 HEX)
            u8  rsv[0x21];
            u8  wpaMode;           // WPA モード
        } wpa;
        struct {
            u8  wpaKey[2][0x20];   // WPA PMK (AOSS 用 [0]:TKIP, [1]:AES)
            u8  ssid[2][0x20];     // SSID (AOSS 用 [0]:TKIP, [1]:AES)
            u8  ssidLength[2];     // SSID 長 (AOSS 用 [0]:TKIP, [1]:AES)
        } aossEx;
    };
    u8  proxyMode;             // Proxy mode
    u8  proxyAuthType;         // Proxy 認証形式
    u8  proxyHost[0x64];       // Proxy hostname
    u16 proxyPort;             // Proxy port
    u8  rsvEx    [0x06];       // 予備
}  NCFGApInfoEx;

// 内蔵バックアップメモリ 1 スロット分 (1 ページ)
typedef struct NCFGSlotInfo
{
    NCFGApInfo ap;             // アクセスポイント接続情報
    u8         wifi[0x0E];     // WiFi 情報
    u16        crc;            // CRC
} NCFGSlotInfo;

// 拡張設定の内蔵バックアップメモリ 1 スロット分 (2ページ)
typedef struct NCFGSlotInfoEx
{
    NCFGApInfo ap;             // アクセスポイント接続情報
    u8         wifi[0x0E];     // WiFi 情報
    u16        crc;            // CRC
    NCFGApInfoEx apEx;         // 拡張アクセスポイント接続情報
    u8         rsvEx[0x0E];    // 予備
    u16        crcEx;          // CRC
} NCFGSlotInfoEx;

// 内蔵バックアップメモリの全体マップ
typedef struct NCFGConfig
{
    NCFGSlotInfo   slot[3];
    NCFGSlotInfo   rsv;
} NCFGConfig;

// 拡張設定を含んだ内蔵バックアップメモリの全体マップ
typedef struct NCFGConfigEx
{
    NCFGSlotInfoEx slotEx[3];
    union {
        NCFGConfig compat;
        struct {
            NCFGSlotInfo slot[3];
            NCFGSlotInfo rsv;
        };
    };
} NCFGConfigEx;


/*---------------------------------------------------------------------------*
    関数 定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         NCFG_CheckConfig

  Description:  ネットワーク設定の一貫性を確認し、修復する。
                ※ 32 バイトアラインされているバッファを指定してください

  Arguments:    work   - ワークメモリ

  Returns:      s32   -      0 : 正常終了
                        -10002 : 接続先設定が消失したが正常終了
                        -10003 : ユーザ ID が消失したが正常終了
                        -10000 : 内蔵バックアップメモリの書き込みエラー終了
                        -10001 : 内蔵バックアップメモリの読み込みエラー終了
 *---------------------------------------------------------------------------*/
s32 NCFG_CheckConfig(void* work);

/*---------------------------------------------------------------------------*
  Name:         NCFG_CheckConfigEx

  Description:  拡張ネットワーク設定の一貫性を確認し、修復する。
                ※ 32 バイトアラインされているバッファを指定してください

  Arguments:    work   - ワークメモリ

  Returns:      s32   -      0 : 正常終了
                        -10002 : 接続先設定が消失したが正常終了
                        -10003 : ユーザ ID が消失したが正常終了
                        -10000 : 内蔵バックアップメモリの書き込みエラー終了
                        -10001 : 内蔵バックアップメモリの読み込みエラー終了
 *---------------------------------------------------------------------------*/
s32 NCFG_CheckConfigEx(void* work);

/*---------------------------------------------------------------------------*
  Name:         NCFG_ReadConfig

  Description:  ネットワーク設定を取得する。
                ※ 32 バイトアラインされているバッファを指定してください

  Arguments:    config - データ格納バッファ
                work   - ワークメモリ

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            NCFG_RESULT_SUCCESS, NCFG_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
s32 NCFG_ReadConfig(NCFGConfig* config, void* work);

/*---------------------------------------------------------------------------*
  Name:         NCFG_WriteConfig

  Description:  ネットワーク設定を設定する。
                ※ 32 バイトアラインされているバッファを指定してください

  Arguments:    config - データ格納バッファ
                work   - ワークメモリ

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            NCFG_RESULT_SUCCESS, NCFG_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
s32 NCFG_WriteConfig(const NCFGConfig* config, void* work);

/*---------------------------------------------------------------------------*
  Name:         NCFG_ReadConfigEx

  Description:  拡張ネットワーク設定を取得する。
                ※ 32 バイトアラインされているバッファを指定してください

  Arguments:    configEx - データ格納バッファ
                work   - ワークメモリ

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            NCFG_RESULT_SUCCESS, NCFG_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
s32 NCFG_ReadConfigEx(NCFGConfigEx* configEx, void* work);

/*---------------------------------------------------------------------------*
  Name:         NCFG_WriteConfigEx

  Description:  拡張ネットワーク設定を設定する。
                ※ 32 バイトアラインされているバッファを指定してください

  Arguments:    configEx - データ格納バッファ
                work   - ワークメモリ

  Returns:      s32     -   以下の内のいずれかの処理結果が返される。
                            NCFG_RESULT_SUCCESS, NCFG_RESULT_FAILURE
 *---------------------------------------------------------------------------*/
s32 NCFG_WriteConfigEx(const NCFGConfigEx* configEx, void* work);


BOOL NCFGi_CheckApInfo(const NCFGApInfo* info);
BOOL NCFGi_CheckApInfoEx(const NCFGApInfo* info, const NCFGApInfoEx* infoEx);
BOOL NCFGi_CheckIpInfo(const NCFGApInfo* info);
u8  NCFGi_ConvMaskCidr(const u8* mask);
void NCFGi_ConvMaskAddr(int mask, u8* buf);
BOOL NCFGi_CheckSsid(const u8* ssid);
BOOL NCFGi_CheckIp(const u8* ip, const u8* mask);
BOOL NCFGi_CheckAddress(const u8* address);

/*===========================================================================*/
#ifdef __cplusplus

}       /* extern "C" */
#endif

#endif /* TWLWIFI_NCFG_INFO_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
