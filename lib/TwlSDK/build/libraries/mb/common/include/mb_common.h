 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_common.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef __MB_COMMON_H__
#define __MB_COMMON_H__

/* macro definition -------------------------------------------------------- */

// for debug
#ifdef	PRINT_DEBUG
#define MB_COMM_TYPE_OUTPUT             MBi_comm_type_output
#define MB_COMM_WMEVENT_OUTPUT          MBi_comm_wmevent_output
#else
#define MB_COMM_TYPE_OUTPUT( ... )      ((void)0)
#define MB_COMM_WMEVENT_OUTPUT( ... )   ((void)0)
#endif

/* 送受信サイズ定義 */
#define MB_COMM_P_SENDLEN_DEFAULT       (256)
#define MB_COMM_P_RECVLEN_DEFAULT       (8)

#define MB_COMM_P_SENDLEN_MAX           MB_COMM_PARENT_SEND_MAX
#define MB_COMM_P_RECVLEN_MAX           MB_COMM_PARENT_RECV_MAX

#define MB_COMM_P_SENDLEN_MIN           (sizeof(MBDownloadFileInfo))    // min size
#define MB_COMM_P_RECVLEN_MIN           MB_COMM_PARENT_RECV_MIN

#define MB_COMM_REQ_DATA_SIZE           (29)

/* 断片化リクエストデータに関する計算マクロ */
#define MB_COMM_CALC_REQ_DATA_PIECE_SIZE( __P_RECVLEN__ )       (( __P_RECVLEN__ ) - 2)
#define MB_COMM_CALC_REQ_DATA_PIECE_NUM( __P_RECVLEN__ )        \
                                ( (MB_COMM_REQ_DATA_SIZE + 1) / (MB_COMM_CALC_REQ_DATA_PIECE_SIZE( __P_RECVLEN__ )) )
#define MB_COMM_CALC_REQ_DATA_BUF_SIZE( __P_RECVLEN__ )     (MB_COMM_REQ_DATA_SIZE + 1)
/*
 * ↑論理的に間違いがありますが、IPLがこの仕様で動作してしまっているので、
 *   修正はできません。 本来ならば
 * #define MB_COMM_CALC_REQ_DATA_PICE_NUM( __P_RECV_LEN__)                                                            \
 *                                  ( (MB_COMM_REQ_DATA_SIZE + MB_COMM_CALC_REQ_DATA_PIECE_SIZE( __P_RECVLEN__ ) - 1) \
 *                                    / MB_COMM_CALC_REQ_DATA_PIECE_SIZE( __P_RECVLEN__ ) )
 * #define MB_COMM_CALC_REQ_DATA_BUF_SIZE( __P_RECV_LEN__ )                                 \
 *                                  ( MB_COMM_CALC_REQ_DATA_PIECE_SIZE( __P_RECVLEN__ )     \
 *                                    * MB_COMM_CALC_REQ_DATA_PIECE_NUM( __P_RECVLEN__ ) )
 */


/* ブロックヘッダのサイズ */
#define MB_COMM_PARENT_HEADER_SIZE      (6)     // MBCommParentBlockHeaderのサイズ(padding抜き)
#define MB_COMM_CHILD_HEADER_SIZE       (8)     // MBCommChildBlockHeaderのサイズ(padding抜き)

#define MB_COMM_CALC_BLOCK_SIZE( __P_SENDLEN__ )                (( __P_SENDLEN__ ) - MB_COMM_PARENT_HEADER_SIZE)

/* データ送信関数が返すエラー定義
   WMのERRCODEと被らない値で定義しています。 */
#define MB_SENDFUNC_STATE_ERR           (WM_ERRCODE_MAX + 1)

/* ブロック転送データタイプ */
typedef enum MBCommType
{
    MB_COMM_TYPE_DUMMY = 0,            //  0

    MB_COMM_TYPE_PARENT_SENDSTART,     //  1
    MB_COMM_TYPE_PARENT_KICKREQ,       //  2
    MB_COMM_TYPE_PARENT_DL_FILEINFO,   //  3
    MB_COMM_TYPE_PARENT_DATA,          //  4
    MB_COMM_TYPE_PARENT_BOOTREQ,       //  5
    MB_COMM_TYPE_PARENT_MEMBER_FULL,   //  6

    MB_COMM_TYPE_CHILD_FILEREQ,        //  7
    MB_COMM_TYPE_CHILD_ACCEPT_FILEINFO, //  8
    MB_COMM_TYPE_CHILD_CONTINUE,       //  9
    MB_COMM_TYPE_CHILD_STOPREQ,        //  10
    MB_COMM_TYPE_CHILD_BOOTREQ_ACCEPTED //  11
}
MBCommType;

/* ユーザーからのリクエスト型 */
typedef enum MBCommUserReq
{
    MB_COMM_USER_REQ_NONE = 0,         //  0
    MB_COMM_USER_REQ_DL_START,         //  1
    MB_COMM_USER_REQ_SEND_START,       //  2
    MB_COMM_USER_REQ_ACCEPT,           //  3
    MB_COMM_USER_REQ_KICK,             //  4
    MB_COMM_USER_REQ_BOOT              //  5
}
MBCommUserReq;

/* structure definition ---------------------------------------------------- */

/*
 * エラー通知コールバック
 */
typedef struct
{
    u16     apiid;                     // APIコード
    u16     errcode;                   // エラーコード
}
MBErrorCallback;


/* functions --------------------------------------------------------------- */

/*
 * ブロックヘッダを初期化し, タイプだけ設定する.
 * このあと MBi_BlockHeaderEnd() で送信するまでの間に
 * こまごまとしたフィールドを埋めていく.
 * 引数がなければそのままで良い.
 */
void    MBi_BlockHeaderBegin(u8 type, u32 *sendbuf);

/*
 * ブロックヘッダ設定を完了し, チェックサムを計算する.
 * その後, pollbmp で指定した相手へ実際に送信する.
 */
int     MBi_BlockHeaderEnd(int body_len, u16 pollbmp, u32 *sendbuf);


/*
 * チェックサム計算
 */
u16     MBi_calc_cksum(const u16 *buf, int length);

// --- for debug
void    MBi_DebugPrint(const char *file, int line, const char *func, const char *fmt, ...);
void    MBi_comm_type_output(u16 type);
void    MBi_comm_wmevent_output(u16 type, void *arg);

#endif /* __MB_COMMON_H__ */
