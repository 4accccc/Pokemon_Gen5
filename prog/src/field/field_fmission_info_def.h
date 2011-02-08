//===============================================================
/*
 *  @file   field_fmission_info_def.h
 *  @brief  フェスミッション　フィールド情報表示システム リテラル定義
 *  @author Miyuki Iwasawa
 *  @date   11.02.08
 */
//===============================================================

#pragma once

#define AP_TIMEP_SX (10*8)
#define AP_TIMEP_X  (256-AP_TIMEP_SX)
#define AP_TIMEP_Y  (0)
#define AP_TIMEN_OX (8)
#define AP_TIMEN_X  (3*8+4+AP_TIMEP_X)
#define AP_TIMEN_Y  (4+AP_TIMEP_Y)
#define AP_TIMENC_OX (5)
#define AP_TIMENC_X (5*8+AP_TIMEP_X)
#define AP_TIMENC_Y (AP_TIMEN_Y)

#define AP_TSP_SX   (12*8)
#define AP_TSP_SY   (3*8)
#define AP_TSP_X    (0)
#define AP_TSP_Y    (192-AP_TSP_SY)
#define AP_TSN_OX   (8)
#define AP_TSN_X    (5*8+4+AP_TSP_X)
#define AP_TSN_Y    (4+AP_TSP_Y)

#define AP_MSP_SX   (11*8)
#define AP_MSP_SY   (3*8)
#define AP_MSP_X    (AP_TSP_X)
#define AP_MSP_Y    (AP_TSP_Y-AP_MSP_SY)
#define AP_MSN_OX   (8)
#define AP_MSN_X    (4*8+4+AP_MSP_X)
#define AP_MSN_Y    (8+AP_MSP_Y)

