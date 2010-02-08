/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  調査レーダー 共通ヘッダ
 * @file   research_common.h
 * @author obata
 * @date   2010.02.06
 */
///////////////////////////////////////////////////////////////////////////////// 
#pragma once


//=============================================================================== 
// ■定数
//=============================================================================== 
#define PRINT_TARGET (2)  // デバッグ出力先

#define ONE_PALETTE_SIZE (0x20)  // 1パレット(16色分)のサイズ [Byte]

// 上画面
#define SUB_BG_BACK_GROUND (GFL_BG_FRAME0_S)  // 背景BG面
#define SUB_BG_BACK_GROUND_PRIORITY      (3)  // 背景BG面の表示優先順位
#define SUB_BG_BACK_GROUND_FIRST_PLT_IDX (0)  // 背景BG面の先頭パレット番号
#define SUB_BG_BACK_GROUND_PLT_NUM       (6)  // 背景BG面のパレット数
