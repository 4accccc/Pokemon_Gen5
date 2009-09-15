/*---------------------------------------------------------------------------*
  Project:  TwlSDK - snd - include
  File:     sndex_common.h

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_SND_COMMON_SNDEX_COMMON_H_
#define TWL_SND_COMMON_SNDEX_COMMON_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
/* PXI コマンドフォーマット */
#define     SNDEX_PXI_COMMAND_MASK              0x00ff0000
#define     SNDEX_PXI_COMMAND_SHIFT             16
#define     SNDEX_PXI_RESULT_MASK               0x0000ff00
#define     SNDEX_PXI_RESULT_SHIFT              8
#define     SNDEX_PXI_PARAMETER_MASK            0x000000ff
#define     SNDEX_PXI_PARAMETER_SHIFT           0
#define     SNDEX_PXI_PARAMETER_MASK_IIR        0x0000ffff
#define     SNDEX_PXI_PARAMETER_SHIFT_IIR       0

// 音量の設定・取得に関するパラメータのマスク一覧
#define     SNDEX_PXI_PARAMETER_MASK_VOL_VALUE  0x1f	// スピーカー音量
#define     SNDEX_PXI_PARAMETER_MASK_VOL_KEEP   0x80	// 変更予約されたスピーカー音量の保留値
#define     SNDEX_PXI_PARAMETER_SHIFT_VOL_KEEP  7
#define     SNDEX_PXI_PARAMETER_MASK_VOL_8LV    0x40	// スピーカー音量（8段階）
#define     SNDEX_PXI_PARAMETER_SHIFT_VOL_8LV   6

/* PXI コマンド種別 */
#define     SNDEX_PXI_COMMAND_GET_SMIX_MUTE           0x01    // 消音状態取得
#define     SNDEX_PXI_COMMAND_GET_SMIX_FREQ           0x02    // I2S 周波数取得
#define     SNDEX_PXI_COMMAND_GET_SMIX_DSP            0x03    // CPU/DSP 出力合成比取得
#define     SNDEX_PXI_COMMAND_GET_VOLUME              0x04    // 音量取得
#define     SNDEX_PXI_COMMAND_GET_SND_DEVICE          0x05    // 音声出力デバイス設定取得
#define     SNDEX_PXI_COMMAND_SET_SMIX_MUTE           0x81    // 消音状態変更
#define     SNDEX_PXI_COMMAND_SET_SMIX_FREQ           0x82    // I2S 周波数変更
#define     SNDEX_PXI_COMMAND_SET_SMIX_DSP            0x83    // CPU/DSP 出力合成比変更
#define     SNDEX_PXI_COMMAND_SET_VOLUME              0x84    // 音量変更
#define     SNDEX_PXI_COMMAND_SET_SND_DEVICE          0x85    // 音声出力デバイス設定変更
#define     SNDEX_PXI_COMMAND_SET_IIRFILTER_TARGET    0xc1    // IIRフィルタをかける対象の通知
#define     SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_N0  0xc2    // IIRフィルタパラメータの通知
#define     SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_N1  0xc3    // IIRフィルタパラメータの通知
#define     SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_N2  0xc4    // IIRフィルタパラメータの通知
#define     SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_D1  0xc5    // IIRフィルタパラメータの通知
#define     SNDEX_PXI_COMMAND_SET_IIRFILTER_PARAM_D2  0xc6    // IIRフィルタパラメータの通知
#define     SNDEX_PXI_COMMAND_SET_IIRFILTER           0xc7    // IIRフィルタの登録
#define     SNDEX_PXI_COMMAND_PRE_PROC_SHUTTER        0xfb    // シャッター音再生前処理
#define     SNDEX_PXI_COMMAND_POST_PROC_SHUTTER       0xfc    // シャッター音再生後処理
#define     SNDEX_PXI_COMMAND_PRESS_VOLSWITCH         0xfd    // 音量調整ボタン押下の通知
#define     SNDEX_PXI_COMMAND_HP_CONNECT              0xfe    // ヘッドフォン接続有無

/* PXI コマンド処理結果種別 */
#define     SNDEX_PXI_RESULT_SUCCESS            0x00    // 成功
#define     SNDEX_PXI_RESULT_INVALID_COMMAND    0x01    // 不明なコマンド
#define     SNDEX_PXI_RESULT_INVALID_PARAM      0x02    // 不明なパラメータ
#define     SNDEX_PXI_RESULT_EXCLUSIVE          0x03    // 排他制御中
#define     SNDEX_PXI_RESULT_ILLEGAL_STATE      0x04    // 処理できない状態 (排他制御以外)
#define     SNDEX_PXI_RESULT_DEVICE_ERROR       0x05    // デバイス操作に失敗
#define     SNDEX_PXI_RESULT_FATAL_ERROR        0xff    // 発生しえない致命的なエラー

/* 音声出力ミュート設定値一覧 */
#define     SNDEX_SMIX_MUTE_OFF                 0x00    // 発音
#define     SNDEX_SMIX_MUTE_ON                  0x01    // 消音

/* I2S 周波数設定値一覧 */
#define     SNDEX_SMIX_FREQ_32730               0x00    // 32.73 kHz
#define     SNDEX_SMIX_FREQ_47610               0x01    // 47.61 kHz

/* 音声出力デバイス設定値一覧 */
#define     SNDEX_MCU_DEVICE_AUTO               0x00    // 状況に応じて出力先を自動選択
#define     SNDEX_MCU_DEVICE_SPEAKER            0x01    // スピーカにのみ出力
#define     SNDEX_MCU_DEVICE_HEADPHONE          0x02    // ヘッドフォンにのみ出力
#define     SNDEX_MCU_DEVICE_BOTH               0x03    // スピーカ・ヘッドフォンの両方に出力

/* 音声強制スピーカ出力設定値一覧 */
#define     SNDEX_FORCEOUT_OFF                  0x00    // 通常状態
#define     SNDEX_FORCEOUT_CAMERA_SHUTTER_ON    0x01    // カメラ撮影用強制出力状態

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   // extern "C"
#endif
#endif  // TWL_SND_COMMON_SNDEX_COMMON_H_
