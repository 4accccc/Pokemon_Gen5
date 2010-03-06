/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_H_
#define DWC_H_

// 使用しているDWC
#define DWC_TWL

// バージョン情報
#include <dwc_version.h>

// プラットフォーム依存コード
#include <nonport/dwc_nonport.h>

// 共通モジュール
#include <common/dwc_base64.h>
#include <common/dwc_error.h>
#include <common/dwc_init.h>
#include <common/dwc_memfunc.h>
#include <common/dwc_report.h>
#include <common/dwc_ssl.h>

// 認証サーバ関連
#include <auth/dwc_nas.h>

// サービスロケータ
#include <auth/dwc_svl.h>

// 不正文字列チェック
#include <auth/dwc_prof.h>

// 自動接続関連
#include <ac/dwc_connectinet.h>

// WiFi設定関連
#include <util/dwc_utility.h>

// ダウンロード関連
#include <nd/dwc_nd.h>

#ifndef TWLDWC_NOGS
// アカウント関連
#include <account/dwc_account.h>

// マッチメイク関連
#include <match/dwc_core.h>

// ランキング関連
#include <ranking/dwc_ranking.h>
#endif

#endif // DWC_H_
