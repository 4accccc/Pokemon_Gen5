/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_ND_H_
#define DWC_ND_H_

#ifdef __cplusplus
extern "C"
{
#endif

    /* -------------------------------------------------------------------------
            define
       ------------------------------------------------------------------------- */

/// ファイル取得時に指定するファイル名の最大文字列長
#define DWC_ND_FILENAME_LEN                 32

/// ファイル取得前に指定する属性の最大文字列長
#define DWC_ND_FILEATTR_LEN                 10

/// ファイルに指定されているUTF16LEで表現された説明文の最大文字列長
#define DWC_ND_FILEEXPLSIN_LEN              50

/// ダウンロードライブラリ内部で使用するスレッドの優先度
#define DWC_ND_THREAD_PRIORITY              17

/// DLゲームコード(gamecd)のバイト長
#define DWC_ND_LENGTH_GAMECODE				4

/// 秘密鍵(passwd)のバイト長
#define DWC_ND_LENGTH_PASSWORD				16

    /* -------------------------------------------------------------------------
            enum
       ------------------------------------------------------------------------- */

    /**
     * ダウンロードライブラリステート
     *
     * ダウンロードライブラリの状態を示す列挙型です。
     */
    typedef enum
    {
        DWC_ND_STATE_NOTINITIALIZED,    ///< ダウンロードライブラリが未初期化
        DWC_ND_STATE_READY,             ///< 初期化が完了し、他のダウンロードライブラリ関数が呼べる状態
        DWC_ND_STATE_BUSY,              ///< 非同期処理を実行中
        DWC_ND_STATE_COMPLETE,          ///< 非同期処理が正常に完了した
        DWC_ND_STATE_ERROR,             ///< 非同期処理中にエラーが発生し、完了した
        DWC_ND_STATE_MAX
    } DWCNdState;


    /**
     * ダウンロードライブラリエラー列挙子
     *
     * ダウンロードライブラリで発生したエラーを示す列挙型です。
     */
    typedef enum
    {
        DWC_ND_ERROR_NONE,     ///< 実行中の処理が完了した
        DWC_ND_ERROR_ALLOC,    ///< メモリ確保に失敗した
        DWC_ND_ERROR_BUSY,     ///< 本来その関数を呼び出せないSTATEで呼び出した
        DWC_ND_ERROR_HTTP,     ///< HTTP通信にエラーが発生した
        DWC_ND_ERROR_BUFFULL,  ///< (内部エラー)ダウンロードバッファが足りない
        DWC_ND_ERROR_PARAM,	   ///< 不正なパラメータによるエラーが発生した
        DWC_ND_ERROR_CANCELED, ///< 処理のキャンセルが完了した
        DWC_ND_ERROR_DLSERVER, ///< サーバがエラーコードを返してきた
        DWC_ND_ERROR_FATAL,	   ///< 未定義の致命的なエラーが発生した
        DWC_ND_ERROR_MAX
    } DWCNdError;

    /**
     * ダウンロードライブラリ進行列挙子
     *
     * ダウンロードライブラリによって通知コールバック関数が呼び出された際、
     * どのような処理が実行中であったかを示す列挙型です。
     */
    typedef enum
    {
        DWC_ND_CBREASON_INITIALIZE,		///< ダウンロードライブラリ初期化中に通知コールバックが呼び出された
        DWC_ND_CBREASON_GETFILELISTNUM, ///< ダウンロード可能なファイル総数をダウンロード中に通知コールバックが呼び出された
        DWC_ND_CBREASON_GETFILELIST,    ///< ファイル情報一覧をダウンロード中に通知コールバックが呼び出された
        DWC_ND_CBREASON_GETFILE,        ///< ファイルをダウンロード中に通知コールバックが呼び出された
        DWC_ND_CBREASON_CLEANUP,        ///< クリーンアップ完了時に呼び出された
        DWC_ND_CBREASON_MAX
    } DWCNdCallbackReason;



    /* -------------------------------------------------------------------------
            struct
       ------------------------------------------------------------------------- */
    /**
     * 個々のファイル情報を格納する構造体です。
     * 
     * ダウンロードライブラリによって取得されるファイル情報を格納する構造体です。
     */
    typedef struct
    {
        char            name[DWC_ND_FILENAME_LEN+1];		    ///< ヌル文字終端した最長32文字のASCII文字列で表現されたファイル名
        short           explain[DWC_ND_FILEEXPLSIN_LEN+1];      ///< ヌル文字終端した最長50文字のUTF16BE文字列で表現されたファイルの説明文
        char            param1[DWC_ND_FILEATTR_LEN+1];		    ///< ヌル文字終端した最長10文字のASCII文字列で表現された属性文字列1
        char            param2[DWC_ND_FILEATTR_LEN+1];		    ///< ヌル文字終端した最長10文字のASCII文字列で表現された属性文字列2
        char            param3[DWC_ND_FILEATTR_LEN+1];		    ///< ヌル文字終端した最長10文字のASCII文字列で表現された属性文字列3
        unsigned int    size;						            ///< ファイルサイズ
    }
    DWCNdFileInfo;


    /* -------------------------------------------------------------------------
            function - external
       ------------------------------------------------------------------------- */

    /**
     * ダウンロードライブラリの各種非同期処理が完了したときに呼び出されるコールバック関数です。
     *
     * ダウンロードライブラリによって呼び出される通知コールバック関数型です。非同期処理の結果が通知されます。
     * 本コールバックのerrorパラメータが、DWC_ND_ERROR_NONE以外のとき、DWC_GetLastErrorEx関数でエラー情報を取得し、エラー処理タイプに応じてエラーメッセージとともにエラーコードを表示、及びライブラリの終了を行ってください。
     *
     * 本コールバック中もダウンロードライブラリの関数を呼ぶことができます。
     *
     * Param:  reason 通知コールバック関数が呼び出された際どのような処理が実行中であったかを示す値
     * Param:  error 通知コールバック関数が呼び出された際に発生したエラーを示す値
     * Param:  servererror サーバから通知されたエラーコード。 errorがDWC_ND_ERROR_DLSERVERではないときは-1となります。
     *
     * See also:  DWC_NdProcess
     * See also:  DWC_NdInitAsync
     * See also:  DWC_NdCleanupAsync
     * See also:  DWCNdCallbackReason
     * See also:  DWCNdError
    */
    typedef void (*DWCNdCallback)( DWCNdCallbackReason reason, DWCNdError error, int servererror );


    BOOL        DWC_NdInitAsync             (DWCNdCallback callback, const char *gamecd, const char *passwd);
    void        DWC_NdProcess               (void);
    BOOL        DWC_NdCleanupAsync          (void);
    BOOL        DWC_NdSetAttr               (const char* attr1, const char* attr2, const char* attr3);
    BOOL        DWC_NdGetFileListNumAsync   (int* entrynum);
    BOOL        DWC_NdGetFileListAsync      (DWCNdFileInfo* filelist, unsigned int offset, unsigned int num);
    BOOL        DWC_NdGetFileAsync          (DWCNdFileInfo* fileinfo, char* buf, unsigned int size);
    BOOL        DWC_NdGetFileExAsync          (DWCNdFileInfo* fileinfo, char* buf, unsigned int size, const char*opt);
    BOOL        DWC_NdCancelAsync           (void);
    BOOL        DWC_NdGetProgress           (u32* received, u32* contentlen);

#ifdef __cplusplus
}
#endif

#endif // DWC_ND_H_
