/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_REPORT_H_
#define DWC_REPORT_H_

#ifdef  __cplusplus
extern "C"
{
#endif

    /**
     *  dwcのデバッグ情報出力用フラグ
     *
     */
    typedef enum 
    {
        DWC_REPORTFLAG_NONE            = 0x00000000, ///< 出力なし
        DWC_REPORTFLAG_INFO            = 0x00000001, ///< 情報
        DWC_REPORTFLAG_ERROR           = 0x00000002, ///< エラー
        DWC_REPORTFLAG_DEBUG           = 0x00000004, ///< デバッグ情報
        DWC_REPORTFLAG_WARNING         = 0x00000008, ///< 警告
        DWC_REPORTFLAG_ACHECK          = 0x00000010, ///< GameSpyサーバーの利用可能チェック
        DWC_REPORTFLAG_LOGIN           = 0x00000020, ///< ログイン関係
        DWC_REPORTFLAG_MATCH_NN        = 0x00000040, ///< マッチング - NN関係
        DWC_REPORTFLAG_MATCH_GT2       = 0x00000080, ///< マッチング - GT2関係
        DWC_REPORTFLAG_TRANSPORT       = 0x00000100, ///< 送受信関係
        DWC_REPORTFLAG_QR2_REQ         = 0x00000200, ///< QR2関係
        DWC_REPORTFLAG_SB_UPDATE       = 0x00000400, ///< SB関係
        DWC_REPORTFLAG_SEND_INFO       = 0x00008000, ///< 送信情報
        DWC_REPORTFLAG_RECV_INFO       = 0x00010000, ///< 受信情報
        DWC_REPORTFLAG_UPDATE_SV       = 0x00020000, ///< サーバ更新関係
        DWC_REPORTFLAG_CONNECTINET     = 0x00040000, ///< インターネット接続関係
        DWC_REPORTFLAG_AUTH            = 0x01000000, ///< 認証関係
        DWC_REPORTFLAG_AC              = 0x02000000, ///< アカウント関係
        DWC_REPORTFLAG_BM              = 0x04000000, ///< WiFi設定のセーブ・ロード
        DWC_REPORTFLAG_UTIL            = 0x08000000, ///< UTIL関係
        DWC_REPORTFLAG_OPTION_CF       = 0x10000000, ///< 未使用
        DWC_REPORTFLAG_OPTION_CONNTEST = 0x20000000, ///< 未使用
        DWC_REPORTFLAG_GAMESPY         = 0x80000000, ///< Gamespy関係
        DWC_REPORTFLAG_TEST            = 0x00100000,
        DWC_REPORTFLAG_ALL             = 0xffffffff  ///< すべて

    }DWCReportFlag;

#ifndef SDK_FINALROM
    void        DWC_SetReportLevel      ( u32 level );
    void        DWC_Printf              ( DWCReportFlag level, const char* fmt, ... );
#else
#define     DWC_SetReportLevel( level )         ((void)0)
#define     DWC_Printf( level, ... )        ((void)0)
#endif


#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif // DWC_REPORT_H_
