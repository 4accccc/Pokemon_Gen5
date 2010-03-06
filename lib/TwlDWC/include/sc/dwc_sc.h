/*--------------------------------------------------------------------------
  Project:  dwc_sc
  File:     dwci_sc.h
 
  Copyright 2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009/09/28#$
  $Rev:  $
  $Author: niwa_kazutomo $
  *-------------------------------------------------------------------------*/

#ifndef DWC_SC_H_
#define DWC_SC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <dwc.h>
#include <nonport/dwc_nonport.h>

/* -------------------------------------------------------------------------
        define
   ------------------------------------------------------------------------- */

#define DWC_SC_AUTHDATA_SIZE 16
#define DWC_SC_SESSION_GUID_SIZE 40
#define DWC_SC_CONNECTION_GUID_SIZE 40

/* -------------------------------------------------------------------------
        enum
   ------------------------------------------------------------------------- */

/**
 * エラーおよび失敗について示します。
 */
typedef enum
{
    DWC_SC_RESULT_NO_ERROR = 0,              ///< エラーは発生しませんでした
    DWC_SC_RESULT_NO_AVAILABILITY_CHECK,     ///< 初期化の前に、標準の GameSpy 可用性チェックが実行されませんでした
    DWC_SC_RESULT_INVALID_PARAMETERS,        ///< インターフェース関数に渡されたパラメータが無効です
    DWC_SC_RESULT_NOT_INITIALIZED,           ///< SDK が初期化されませんでした
    DWC_SC_RESULT_CORE_NOT_INITIALIZED,      ///< コアが初期化されませんでした
    DWC_SC_RESULT_OUT_OF_MEMORY,             ///< SDK はリソースにメモリを割り当てられませんでした
    DWC_SC_RESULT_CALLBACK_PENDING,          ///< アプリケーションに対し、操作が保留中であることを伝えます

    DWC_SC_RESULT_HTTP_ERROR,                ///< バックエンドが適正な HTTP によるレスポンスに失敗した場合のエラー発生
    DWC_SC_RESULT_UNKNOWN_RESPONSE,          ///< SDK が結果を理解できない場合のエラー発生
    DWC_SC_RESULT_RESPONSE_INVALID,          ///< SDK がバックエンドからのレスポンスを読み取れない場合のエラー発生
    DWC_SC_RESULT_INVALID_DATATYPE,          ///< 無効なデータタイプ

    DWC_SC_RESULT_REPORT_INCOMPLETE,         ///< レポートが不完全です
    DWC_SC_RESULT_REPORT_INVALID,            ///< レポートの一部またはすべてが無効です
    DWC_SC_RESULT_SUBMISSION_FAILED,         ///< レポートの送信に失敗しました

    DWC_SC_RESULT_UNKNOWN_ERROR,             ///< SDK が認識できないエラー

    DWC_SC_RESULT_MAX                        ///< 返される可能性のある結果コードの合計数
} DWCScResult;

/**
 * 対戦結果がどのように終了したかを表します。
 */
typedef enum
{
    DWC_SC_GAME_RESULT_WIN,                  ///< 勝利
    DWC_SC_GAME_RESULT_LOSS,                 ///< 敗北
    DWC_SC_GAME_RESULT_DRAW,                 ///< 引き分け
    DWC_SC_GAME_RESULT_DISCONNECT,           ///< 切断
    DWC_SC_GAME_RESULT_DESYNC,               ///< 同期ミス
    DWC_SC_GAME_RESULT_NONE,                 ///< 未定義

    DWC_SC_GAME_RESULT_MAX                   ///< ゲームの結果コードの合計数
} DWCScGameResult;

/**
 * ゲームセッションがどのように終了しているかを示します。
 */
typedef enum
{
    DWC_SC_GAME_STATUS_COMPLETE,             ///< ゲーム セッションが、中断（切断、非同期化など）されることなく予定通りに終了しました。 このステータスは、すべてのプレイヤーがゲーム結果を入手できることを示しています
    DWC_SC_GAME_STATUS_PARTIAL,              ///< ゲーム セッションは予定通りに終了しましたが、1 人以上のプレイヤーが予想外に中止または切断しました。 接続切断されたプレイヤーがゲーム結果で明確にレポートされ、正規化中、可能性のあるペナルティ メトリックに使用されます
    DWC_SC_GAME_STATUS_BROKEN,               ///< ゲーム セッションは予想通りのエンド ポイントに達せず、完了しませんでした。 これは、終了結果が確定できないイベントが検出された場合にレポートされる必要があります

    DWC_SC_GAME_STATUS_MAX                   ///< ゲーム ステータス コードの合計数
} DWCScGameStatus;

/* -------------------------------------------------------------------------
        struct
   ------------------------------------------------------------------------- */

/**
 * レポート情報格納用構造体
 */
typedef void* DWCScReportPtr;

/**
 * ログイン認証情報格納用構造体
 */
typedef void*	DWCScLoginCertificatePtr;

/* -------------------------------------------------------------------------
        typedef - callback
   ------------------------------------------------------------------------- */

/**
 * セッション作成処理が完了したときに呼び出されるコールバック関数。
 *
 * セッション作成処理が完了したときに呼び出されるコールバック関数。
 *
 * Param:  theResult 処理結果
 * Param:  theUserData ユーザ定義データ
 */
typedef void (*DWCScCreateSessionCallback)(DWCScResult theResult, void* theUserData);

/**
 * レポートインテンション処理が完了したときに呼び出されるコールバック関数。
 *
 * レポートインテンション処理が完了したときに呼び出されるコールバック関数。
 *
 * Param:  theResult 処理結果
 * Param:  theUserData ユーザ定義データ
 */
typedef void (*DWCScSetReportIntentionCallback)(DWCScResult theResult, void* theUserData);

/**
 * レポート送信処理が完了したときに呼び出されるコールバック関数。
 *
 * レポート送信処理が完了したときに呼び出されるコールバック関数。
 *
 * Param:  theResult 処理結果
 * Param:  theUserData ユーザ定義データ
 */
typedef void (*DWCScSubmitReportCallback)(DWCScResult theResult, void* theUserData);

/* -------------------------------------------------------------------------
        function - external
   ------------------------------------------------------------------------- */

extern DWCScResult DWC_ScInitialize(int theGameId, DWCSSLType ssl_type);
extern DWCScResult DWC_ScShutdown();
extern DWCScResult DWC_ScProcess();
extern DWCScResult DWC_ScCreateSessionAsync(DWCScCreateSessionCallback theCallback, u32 theTimeoutMs, void* theUserData);
extern DWCScResult DWC_ScCreateMatchlessSessionAsync(DWCScCreateSessionCallback theCallback, u32 theTimeoutMs, void* theUserData);
extern DWCScResult DWC_ScSetReportIntentionAsync(BOOL isAuthoritative, DWCScSetReportIntentionCallback theCallback, u32 theTimeoutMs, void* theUserData);
extern DWCScResult DWC_ScSubmitReportAsync(const DWCScReportPtr theReport, BOOL isAuthoritative, DWCScSubmitReportCallback theCallback, u32 theTimeoutMs, void * theUserData);
extern DWCScResult DWC_ScSetSessionId(const u8 theSessionId[DWC_SC_SESSION_GUID_SIZE]);
const u8* DWC_ScGetSessionId();
const u8* DWC_ScGetConnectionId();
extern DWCScResult DWC_ScCreateReport(u32 theHeaderVersion, u32 thePlayerCount, u32 theTeamCount, DWCScReportPtr * theReportOut);
extern DWCScResult DWC_ScReportBeginGlobalData(DWCScReportPtr theReportData);
extern DWCScResult DWC_ScReportBeginPlayerData(DWCScReportPtr theReportData);
extern DWCScResult DWC_ScReportBeginTeamData(DWCScReportPtr theReportData);
extern DWCScResult DWC_ScReportBeginNewPlayer(DWCScReportPtr theReportData);
extern DWCScResult DWC_ScReportSetPlayerData(DWCScReportPtr theReport, u32 thePlayerIndex, const u8 thePlayerConnectionId[DWC_SC_CONNECTION_GUID_SIZE], u32 thePlayerTeamId, DWCScGameResult theResult, DWCScLoginCertificatePtr theCertificate, const u8 theAuthData[DWC_SC_AUTHDATA_SIZE]);
extern DWCScResult DWC_ScReportBeginNewTeam(DWCScReportPtr theReportData);
extern DWCScResult DWC_ScReportSetTeamData(DWCScReportPtr theReport, u32 theTeamId, DWCScGameResult theResult);
extern DWCScResult DWC_ScReportEnd(DWCScReportPtr theReport, BOOL isAuth, DWCScGameStatus theStatus);
extern DWCScResult DWC_ScReportSetAsMatchless(DWCScReportPtr theReport);
extern DWCScResult DWC_ScReportAddIntValue(DWCScReportPtr theReportData, u16 theKeyId, s32 theValue);
extern DWCScResult DWC_ScReportAddShortValue(DWCScReportPtr theReportData, u16 theKeyId, s16 theValue);
extern DWCScResult DWC_ScReportAddByteValue(DWCScReportPtr theReportData, u16 theKeyId, s8 theValue);
extern DWCScResult DWC_ScReportAddFloatValue(DWCScReportPtr theReportData, u16 theKeyId, f32 theValue);
extern DWCScResult DWC_ScReportAddStringValue(DWCScReportPtr theReportData, u16 theKeyId, const char* theValue);
extern DWCScResult DWC_ScDestroyReport(DWCScReportPtr theReport);

extern DWCScResult DWC_ScGetLoginCertificate( DWCScLoginCertificatePtr* theCertificate );

#ifdef __cplusplus
} /* extern "C" */
#endif

/* DWC_SC_H_ */
#endif
