/*---------------------------------------------------------------------------*
  Project:  TwlSDK - camera - include
  File:     fifo.h

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_CAMERA_FIFO_H_
#define TWL_CAMERA_FIFO_H_

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
// プロトコル関連定義
#define CAMERA_PXI_CONTINUOUS_PACKET_MAX       4           // 連続パケットの最大連続回数
#define CAMERA_PXI_DATA_SIZE_MAX               ((CAMERA_PXI_CONTINUOUS_PACKET_MAX-1)*3+1) // 最大データ数

#define CAMERA_PXI_START_BIT                   0x02000000  // 先頭パケットを意味する
#define CAMERA_PXI_RESULT_BIT                  0x00008000  // PXIの応答を示す

/* 先頭パケットのみの規則 */
#define CAMERA_PXI_DATA_NUMS_MASK              0x00ff0000  // データ数領域
#define CAMERA_PXI_DATA_NUMS_SHIFT             16          // データ数位置
#define CAMERA_PXI_COMMAND_MASK                0x00007f00  // コマンド格納部分のマスク
#define CAMERA_PXI_COMMAND_SHIFT               8           // コマンド格納部分の位置
#define CAMERA_PXI_1ST_DATA_MASK               0x000000ff  // 先頭パケットのデータ領域
#define CAMERA_PXI_1ST_DATA_SHIFT              0           // 先頭パケットのデータ位置

/* 後続パケットのみの規則 */
#define CAMERA_PXI_DATA_MASK                   0x00ffffff  // データ領域
#define CAMERA_PXI_DATA_SHIFT                  0           // データ位置

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

// PXIコマンド定義
typedef enum CAMERAPxiCommand
{
    CAMERA_PXI_COMMAND_INIT                         = 0x00, // 初期化

    CAMERA_PXI_COMMAND_ACTIVATE                     = 0x10, // アクティブ選択
#if 0   /* 有用性が低いので封印 */
    CAMERA_PXI_COMMAND_OUTPUT_WITH_DUAL_ACTIVATION  = 0x11, // 出力選択(両方アクティブ)
#endif
    CAMERA_PXI_COMMAND_CONTEXT_SWITCH               = 0x12, // コンテキストスイッチ

    CAMERA_PXI_COMMAND_SIZE                         = 0x30, // サイズ設定
    CAMERA_PXI_COMMAND_FRAME_RATE                   = 0x31, // フレームレート設定
    CAMERA_PXI_COMMAND_EFFECT                       = 0x32, // エフェクト設定
    CAMERA_PXI_COMMAND_FLIP                         = 0x33, // 反転設定
    CAMERA_PXI_COMMAND_PHOTO_MODE                   = 0x34, // 撮影モード
    CAMERA_PXI_COMMAND_WHITE_BALANCE                = 0x35, // ホワイトバランス
    CAMERA_PXI_COMMAND_EXPOSURE                     = 0x36, // 露光
    CAMERA_PXI_COMMAND_SHARPNESS                    = 0x37, // シャープネス
    CAMERA_PXI_COMMAND_TEST_PATTERN                 = 0x38, // テストパターン
    CAMERA_PXI_COMMAND_AUTO_EXPOSURE                = 0x39, // AE On/Off
    CAMERA_PXI_COMMAND_AUTO_WHITE_BALANCE           = 0x3A, // AWB On/Off

    CAMERA_PXI_COMMAND_SET_LED                      = 0x40, // カメラLED

    CAMERA_PXI_COMMAND_UNKNOWN
}
CAMERAPxiCommand;

// PXIコマンドサイズ定義
typedef enum CAMERAPxiSize
{
    CAMERA_PXI_SIZE_INIT                            = 1,    // camera

    CAMERA_PXI_SIZE_ACTIVATE                        = 1,    // camera
#if 0   /* 有用性が低いので封印 */
    CAMERA_PXI_SIZE_OUTPUT_WITH_DUAL_ACTIVATION     = 1,    // camera
#endif
    CAMERA_PXI_SIZE_CONTEXT_SWITCH                  = 2,    // camera, context

    CAMERA_PXI_SIZE_SIZE                            = 3,    // camera, context, size
    CAMERA_PXI_SIZE_FRAME_RATE                      = 2,    // camera, rate
    CAMERA_PXI_SIZE_EFFECT                          = 3,    // camera, context, effect
    CAMERA_PXI_SIZE_FLIP                            = 3,    // camera, context, flip
    CAMERA_PXI_SIZE_PHOTO_MODE                      = 2,    // camera, mode
    CAMERA_PXI_SIZE_WHITE_BALANCE                   = 2,    // camera, wb
    CAMERA_PXI_SIZE_EXPOSURE                        = 2,    // camera, exposure
    CAMERA_PXI_SIZE_SHARPNESS                       = 2,    // camera, sharpness
    CAMERA_PXI_SIZE_TEST_PATTERN                    = 2,    // camera, pattern
    CAMERA_PXI_SIZE_AUTO_EXPOSURE                   = 2,    // camera, on
    CAMERA_PXI_SIZE_AUTO_WHITE_BALANCE              = 2,    // camera, on

    CAMERA_PXI_SIZE_SET_LED                         = 1,    // isBlink

    CAMERA_PXI_SIZE_UNKNOWN
}
CAMERAPxiSize;

// 応答定義
typedef enum CAMERAPxiResult
{
    CAMERA_PXI_RESULT_SUCCESS = 0,        // 処理成功 (void/void*型) // 場合により後続パケットあり
    CAMERA_PXI_RESULT_SUCCESS_TRUE = 0,   // 処理成功 (BOOL型)
    CAMERA_PXI_RESULT_SUCCESS_FALSE,      // 処理失敗 (BOOL型)
    CAMERA_PXI_RESULT_INVALID_COMMAND,    // 不正なPXIコマンド
    CAMERA_PXI_RESULT_INVALID_PARAMETER,  // 不正なパラメータ
    CAMERA_PXI_RESULT_ILLEGAL_STATUS,     // CAMERAの状態により処理を実行不可
    CAMERA_PXI_RESULT_BUSY,               // 他のリクエストを実行中
    CAMERA_PXI_RESULT_FATAL_ERROR,        // その他何らかの原因で処理に失敗
    CAMERA_PXI_RESULT_MAX
}
CAMERAPxiResult;


/*===========================================================================*/

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_CAMERA_FIFO_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
