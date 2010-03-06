/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_CONNECTINET_H_
#define DWC_CONNECTINET_H_

#ifdef __cplusplus
extern "C"
{
#endif


    /* -------------------------------------------------------------------------
            define
       ------------------------------------------------------------------------- */

// WEP 暗号化モード種別 (互換性のため残されています)
#define DWC_WEPMODE_NONE        DWC_APINFO_WSEC_NONE                  // 暗号化なし
#define DWC_WEPMODE_40          DWC_APINFO_WSEC_WEPMODE_40            // RC4 (  40 ビット ) 暗号化モード
#define DWC_WEPMODE_104         DWC_APINFO_WSEC_WEPMODE_104           // RC4 ( 104 ビット ) 暗号化モード
#define DWC_WEPMODE_128         DWC_APINFO_WSEC_WEPMODE_128           // RC4 ( 128 ビット ) 暗号化モード

// ニンテンドーWi-Fiステーションの場所情報の長さ
#define DWC_SIZE_ZONEINFO				9
#define DWC_SIZE_SPOTINFO 				DWC_SIZE_ZONEINFO				// nintendo spot互換用

#define	DWC_APINFO_TYPE_NINTENDOSPOT	DWC_APINFO_TYPE_NINTENDOZONE	// nintendo spot互換用

// WDS用バッファサイズ
#define	DWC_WDS_SSID_BUF_SIZE				32
#define DWC_WDS_WEPKEY_BUF_SIZE             32



    /* -------------------------------------------------------------------------
            enum
       ------------------------------------------------------------------------- */

    /**
     * インターネット接続状態
     *
     */
    typedef enum
    {
        DWC_CONNECTINET_STATE_NOT_INITIALIZED = 0, ///< 初期化されていない
        DWC_CONNECTINET_STATE_IDLE,         ///< 接続開始されていない
        DWC_CONNECTINET_STATE_OPERATING,    ///< 処理中
        DWC_CONNECTINET_STATE_OPERATED,     ///< 処理完了
        DWC_CONNECTINET_STATE_CONNECTED,    ///< インターネットへ接続済み
        DWC_CONNECTINET_STATE_DISCONNECTING,///< インターネットから切断中
        DWC_CONNECTINET_STATE_DISCONNECTED, ///< インターネットから切断された
        DWC_CONNECTINET_STATE_ERROR,        ///< エラーが発生した
        DWC_CONNECTINET_STATE_FATAL_ERROR,  ///< 致命的なエラーが発生した
        DWC_CONNECTINET_STATE_LAST
    } DWCInetResult;


    /**
     * 認証サーバの種類
     * 
     */
    typedef enum
    {
        DWC_CONNECTINET_AUTH_TEST,    ///< 開発向け認証サーバ
        DWC_CONNECTINET_AUTH_DEVELOP, ///< ライブラリ開発用認証サーバ(使用しないで下さい)
        DWC_CONNECTINET_AUTH_RELEASE, ///< 製品向け認証サーバ
        DWC_CONNECTINET_AUTH_LAST
    } DWCInetAuthType;


    /**
     * ニンテンドーWi-Fiステーション/Nintendo Zoneの設置地域を示すエリアコード
     *
     */
    typedef enum
    {
        DWC_APINFO_AREA_JPN = 0, ///< 日本
        DWC_APINFO_AREA_USA,     ///< 北米
        DWC_APINFO_AREA_EUR,     ///< 欧州
        DWC_APINFO_AREA_AUS,     ///< 豪州
        DWC_APINFO_AREA_KOR,     ///< 韓国
        DWC_APINFO_AREA_UNKNOWN = 0xff ///< 不明 (ニンテンドーWi-Fiステーション/Nintendo Zoneではない)
    } DWCApInfoArea;


    /**
     * APタイプ
     *
     */
    typedef enum
    {
        DWC_APINFO_TYPE_USER0 = 0, ///< 接続先1のアクセスポイント
        DWC_APINFO_TYPE_USER1,     ///< 接続先2のアクセスポイント
        DWC_APINFO_TYPE_USER2,     ///< 接続先3のアクセスポイント
        DWC_APINFO_TYPE_USB,       ///< ニンテンドーWi-Fi USBコネクタ
        DWC_APINFO_TYPE_SHOP,      ///< ニンテンドーWi-Fiステーション
        DWC_APINFO_TYPE_FREESPOT,  ///< FREESPOT（フリースポット）
        DWC_APINFO_TYPE_WAYPORT,   ///< Wayport(北米ホットスポット)※現在は使用できません
        DWC_APINFO_TYPE_OTHER,     ///<     (旧定義)
        DWC_APINFO_TYPE_NINTENDOWFC = DWC_APINFO_TYPE_OTHER, ///<    (予約)
        DWC_APINFO_TYPE_NINTENDOZONE, ///< Nintendo Zone
#ifdef SDK_TWL
        DWC_APINFO_TYPE_USER3 = 0x0a, ///< 接続先4のアクセスポイント　 ※NITRO ROMではこの定義はありません
        DWC_APINFO_TYPE_USER4,        ///< 接続先5のアクセスポイント　 ※NITRO ROMではこの定義はありません
        DWC_APINFO_TYPE_USER5,        ///< 接続先6のアクセスポイント　 ※NITRO ROMではこの定義はありません
#endif // SDK_TWL
        DWC_APINFO_TYPE_UNKNOWN = 99  ///< 不明
    } DWCApInfoType;


    /**
     * 無線セキュリティタイプ
     *
     */
    typedef enum
    {
        DWC_APINFO_WSEC_NONE  = 0x00,  ///< 暗号化なし
        DWC_APINFO_WSEC_WEPMODE_40,    ///< RC4 (  40 ビット ) 暗号化モード
        DWC_APINFO_WSEC_WEPMODE_104,   ///< RC4 ( 104 ビット ) 暗号化モード
        DWC_APINFO_WSEC_WEPMODE_128	   ///< RC4 ( 128 ビット ) 暗号化モード
#ifdef SDK_TWL
        ,DWC_APINFO_WSEC_WPA_PSK_TKIP, ///< WPA-PSK  ( TKIP ) 暗号　 ※NITRO ROMではこの定義はありません
        DWC_APINFO_WSEC_WPA2_PSK_TKIP, ///< WPA2-PSK ( TKIP ) 暗号　 ※NITRO ROMではこの定義はありません
        DWC_APINFO_WSEC_WPA_PSK_AES,   ///< WPA-PSK  ( AES  ) 暗号　 ※NITRO ROMではこの定義はありません
        DWC_APINFO_WSEC_WPA2_PSK_AES   ///< WPA2-PSK ( AES  ) 暗号　 ※NITRO ROMではこの定義はありません
#endif
    } DWCApInfoSecurity;


    /**
     * インターネットとの接続テストの通信処理結果
     *
     */
    typedef enum
    {
        DWC_TESTINET_NOT_INITIALIZED = 0, ///< DWC_TestInetAsync関数が呼び出されていない
        DWC_TESTINET_OPERATING,    ///< 通信処理を実行中
        DWC_TESTINET_CONNECTED,    ///< インターネットに接続されている
        DWC_TESTINET_DISCONNECTED  ///< インターネットに接続されていない
    } DWCTestInetResult;


    /**
     * Proxy 認証形式
     *
     */
    typedef enum
    {
        DWC_PROXY_AUTHTYPE_NONE  = 0x00,	///< 認証なし
        DWC_PROXY_AUTHTYPE_BASIC 			///< BASIC 認証
    } DWCProxyAuthType;


    // WDS用ステート
    typedef enum
    {
        DWC_WDS_STATE_IDLE      = 0,
        DWC_WDS_STATE_PROCESS   = 1,
        DWC_WDS_STATE_COMPLETED = 2, ///< APのデータを取得できた。
        DWC_WDS_STATE_FAILED    = 3, ///< 取得できなかった。
        DWC_WDS_STATE_ERROR     = 4,
        DWC_WDS_STATE_CANCEL    = 5  ///< WDS処理中にDWC_CleanupInetAsync()が呼ばれたので終了処理中
    } DWCWDSState;



    /* -------------------------------------------------------------------------
            struct
       ------------------------------------------------------------------------- */


    /**
     * 接続中のアクセスポイントに関する情報を格納するための構造体です。
     *
     * アクセスポイントの情報を格納するための構造体です。
     *
     * See also:  DWC_GetApInfo
     *
     */
    typedef struct
    {
        DWCApInfoType		aptype; ///< アクセスポイントの種類
        ///<
        ///< - DWC_APINFO_TYPE_USER0        :  接続先1のアクセスポイント
        ///< - DWC_APINFO_TYPE_USER1        :  接続先2のアクセスポイント
        ///< - DWC_APINFO_TYPE_USER2        :  接続先3のアクセスポイント
        ///< - DWC_APINFO_TYPE_USER3        :  接続先4のアクセスポイント
        ///< - DWC_APINFO_TYPE_USER4        :  接続先5のアクセスポイント
        ///< - DWC_APINFO_TYPE_USER5        :  接続先6のアクセスポイント
        ///< - DWC_APINFO_TYPE_USB          :  ニンテンドーWi-Fi USBコネクタ
        ///< - DWC_APINFO_TYPE_SHOP         :  ニンテンドーWi-Fiステーション
        ///< - DWC_APINFO_TYPE_FREESPOT     :  FREESPOT（フリースポット）
        ///< - DWC_APINFO_TYPE_WAYPORT      :  Wayport(北米ホットスポット)※現在は使用できません
        ///< - DWC_APINFO_TYPE_NINTENDOZONE : Nintendo Zone

        DWCApInfoArea		area; ///< ニンテンドーWi-Fiステーション/nintendo zoneの設置地域
        ///< ※aptypeがDWC_APINFO_TYPE_SHOP/DWC_APINFO_TYPE_NINTENDOZONEの時のみ有効です。
        ///< ※下記の全ての地域に設置されているとは限りません。
        ///< - DWC_APINFO_AREA_JPN      :  日本
        ///< - DWC_APINFO_AREA_USA      :  北米
        ///< - DWC_APINFO_AREA_EUR      :  欧州
        ///< - DWC_APINFO_AREA_AUS      :  豪州
        ///< - DWC_APINFO_AREA_KOR      :  韓国
        ///< - DWC_APINFO_AREA_UNKNOWN  : (ニンテンドーWi-Fiステーション/nintendo zoneではありません)

        DWCApInfoSecurity   security; ///< APとの通信に用いる無線暗号化方式
        ///<
        ///< - DWC_APINFO_WSEC_NONE            :  暗号化なし
        ///< - DWC_APINFO_WSEC_WEPMODE_40      :  RC4 (  40 ビット ) 暗号化モード
        ///< - DWC_APINFO_WSEC_WEPMODE_104     :  RC4 ( 104 ビット ) 暗号化モード
        ///< - DWC_APINFO_WSEC_WEPMODE_128     :  RC4 ( 128 ビット ) 暗号化モード
        ///< - DWC_APINFO_WSEC_WPA_PSK_TKIP    :  WPA-PSK  ( TKIP ) 暗号
        ///< - DWC_APINFO_WSEC_WPA2_PSK_TKIP   :  WPA2-PSK ( TKIP ) 暗号
        ///< - DWC_APINFO_WSEC_WPA_PSK_AES     :  WPA-PSK  ( AES  ) 暗号
        ///< - DWC_APINFO_WSEC_WPA2_PSK_AES    :  WPA2-PSK ( AES  ) 暗号

        char				spotinfo[DWC_SIZE_ZONEINFO+1]; ///< ニンテンドーWi-Fiステーション/nintendo zoneの場所情報
        ///< ※aptypeがDWC_APINFO_TYPE_SHOP/DWC_APINFO_TYPE_NINTENDOZONEの時のみ有効です。
        ///< 各ニンテンドーWi-Fiステーション/nintendo zoneでユニークな9桁の文字列です。

        char				essid[WM_SIZE_SSID+1]; ///< アクセスポイントのSSID ※aptypeがDWC_APINFO_TYPE_USER*の時のみ有効です。
        u8 					bssid[WM_SIZE_BSSID]; ///< アクセスポイントのBSSID
    } DWCApInfo;


    /**
     * 接続中のアクセスポイントに関するproxy設定情報を取得する関数です。
     *
     * proxy設定情報を格納するための構造体です。
     *
     * See also:  DWC_GetProxySetting
     */
    typedef struct
    {
        u16         authType;					///< proxyの認証タイプ
        ///<
        ///< - DWC_PROXY_AUTHTYPE_NONE ： 認証なし
        ///< - DWC_PROXY_AUTHTYPE_BASIC： BASIC 認証
        
        u16         port;						///< proxyが使用するポート
        u8          hostName      [ 0x64 ];		///< proxyサーバのURL
        u8          authId        [ 0x20 ];		///< basic認証時のID
        u8          authPass      [ 0x20 ];		///< basic認証時のパスワード
    } DWCProxySetting;


    // WDS用の構造体
    typedef struct tagDWCWDSData
    {
        u8          ssid[ DWC_WDS_SSID_BUF_SIZE ];
        u8          wep[ DWC_WDS_WEPKEY_BUF_SIZE ];
        int         wepMode;
        u16		    mtu;
        char        apnum[10];
    } DWCWDSData;


    typedef struct DWCstInetControl
    {
        volatile int ac_state;
        u16  state;
        u16  online;
        u16  dmaNo;
        u16  powerMode;
        s32  rwin; ///< TCP rwinサイズ
        u8   wontExecuteEULACheck;    ///< EULAチェックをしない場合TRUE
        u8            wontExecuteWDS; ///< nintendo zoneへの接続を試みるフェーズを中止するかどうかのフラグ
        DWCWDSState   processWDSstate;///< WDSの処理ステート
        DWCWDSData    nspotInfo;      ///< WDSで取得したssid,wep,wepmode,apnumを格納するための変数
#ifndef SDK_FINALROM
        int  connectType;
#endif
    } DWCInetControl;



    /* -------------------------------------------------------------------------
            function - external
       ------------------------------------------------------------------------- */

    typedef void* (* DWCACAlloc )( u32 name, s32 size );				// alloc関数へのポインタ
    typedef void  (* DWCACFree  )( u32 name, void *ptr, s32 size );		// free関数へのポインタ

    void  DWC_InitInet( DWCInetControl* inetCntl );
    void  DWC_InitInetEx( DWCInetControl* inetCntl, u16 dmaNo, u16 powerMode, u16 ssl_priority );
    void  DWC_SetRwinSize( s32 size );

#ifndef SDK_FINALROM
    void         DWC_SetConnectApType( DWCApInfoType type );
#else
#define      DWC_SetConnectApType( type )       ((void)0)
#endif

    void  DWC_ConnectInetAsync( void );
    void  DWC_DebugConnectInetAsync( const void *ssid, const void *wep, int wepMode );
    void  DWC_DebugConnectInetExAsync( const void *ssid, const void *wep, int wepMode, u16 mtu, const char *apSpotInfo, int overrideType );

    BOOL  DWC_CheckInet( void );
    void  DWC_ProcessInet( void );
    DWCInetResult DWC_GetInetStatus( void );

    BOOL DWC_UpdateConnection( void );

    BOOL DWC_GetApInfo(DWCApInfo *apinfo);
    BOOL DWC_GetProxySetting( DWCProxySetting *proxy );
    WMLinkLevel DWC_GetLinkLevel( void );
    int   DWC_GetUdpPacketDropNum( void );

    void  DWC_CleanupInet( void );
    BOOL  DWC_CleanupInetAsync( void );

    BOOL DWC_TestInetAsync(int timeout);
    DWCTestInetResult DWC_TestInetProcess(void);


    void DWC_EnableHotspot(void); // [nakata] ブラウザ対応の拡張
    BOOL DWC_CheckWiFiStation( const void *ssid, u16 len, DWCApInfo* apinfo );
    size_t DWC_GetCheckNintendoZoneWorkSize( void );
    BOOL DWC_CheckNintendoZone( void* work, const WMBssDesc* bssdesc, DWCApInfo* apinfo );
    size_t DWC_GetParseWMBssDescWorkSize( void );
    BOOL DWC_ParseWMBssDesc( void* work, const WMBssDesc* bssdesc, DWCApInfo* apinfo );
    void DWC_SetDisableEulaCheck( void );


#ifdef __cplusplus
}
#endif


#endif // DWC_CONNECTINET_H_
