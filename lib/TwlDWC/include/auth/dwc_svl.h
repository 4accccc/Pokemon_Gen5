/*---------------------------------------------------------------------------*
  Project:  TwlDWC

  Copyright 2005-2010 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#ifndef DWC_SVL_H
#define DWC_SVL_H

#ifdef __cplusplus
extern "C"
{
#endif


/* -------------------------------------------------------------------------
        define
   ------------------------------------------------------------------------- */

#define DWC_SVL_ERROR_BASENUM		(-24000)    ///<
#define DWC_SVL_ERROR_HTTPBASENUM	(-25000)    ///<

#define DWC_SVL_ERROR_FATAL			(-100)      ///<
#define DWC_SVL_ERROR_BADDATA		(-101)      ///<

#define DWC_SVL_HOST_LENGTH		    64      ///< ホスト名の最大文字数
#define DWC_SVL_TOKEN_LENGTH	    300     ///< トークンの最大文字数
#define DWC_SVL_STATUSDATA_LENGTH	1       ///< ステータスのサイズ


/* -------------------------------------------------------------------------
        enum
   ------------------------------------------------------------------------- */

///  サービスロケータ情報取得中のステートを示す列挙型
typedef enum
{
    DWC_SVL_STATE_DIRTY	= 0,///< 未初期化
    DWC_SVL_STATE_IDLE,		///< 処理実行中ではない
    DWC_SVL_STATE_HTTP,		///< HTTP通信中
    DWC_SVL_STATE_SUCCESS,	///< トークン取得成功
    DWC_SVL_STATE_ERROR,	///< トークン取得失敗
    DWC_SVL_STATE_CANCELED,	///< トークン取得キャンセル
    DWC_SVL_STATE_MAX
} DWCSvlState;


/* -------------------------------------------------------------------------
        struct
   ------------------------------------------------------------------------- */

/**
 * サービスロケータ情報取得結果を格納する構造体
 *
 * サービスロケータ情報を格納するための構造体です。 
 * 
 * See also:  DWC_SVLGetTokenAsync
 */
typedef struct
{
    BOOL	status;                             ///< 対応するサーバの稼動状況です。
    ///< TRUE : サーバは稼動しています。
    ///< FALSE : サーバは稼動していません。
    ///※この値は将来の拡張のために用意されており、現状は必ずTRUEが格納されます。この値は無視してください。
    
    char    svlhost[DWC_SVL_HOST_LENGTH+1];     ///< 対応するサーバのURLです。
    ///< ヌル文字終端した最大64文字のASCII文字列です。
    ///※この値は将来の拡張のために用意されています。この値は無視してください。

    char    svltoken[DWC_SVL_TOKEN_LENGTH+1];   ///< 対応するサーバ向けの認証トークンです。
    ///< ヌル文字終端した最大300文字のASCII文字列です。
}
DWCSvlResult;



/* -------------------------------------------------------------------------
        function - external
   ------------------------------------------------------------------------- */

BOOL        DWC_SVLGetTokenAsync(char *svl, DWCSvlResult *result);
DWCSvlState DWC_SVLProcess(void);
void        DWC_SVLAbort(void);




#ifdef __cplusplus
}
#endif

#endif
