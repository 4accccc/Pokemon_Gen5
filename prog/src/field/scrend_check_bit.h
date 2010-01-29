//======================================================================
/**
 * @file	scrend_check_bit.h
 * @brief	スクリプト終了時チェックビット定義
 * @author	NozomuSaito
 *
 * @note    SCREND_CHKの並びは、scrcmd.cのCheckEndFuncTblと1対1で対応するようにしてください
 *
 */
//======================================================================

#pragma once

#ifdef PM_DEBUG

#if defined(DEBUG_ONLY_FOR_masafumi_saitou) | defined(DEBUG_ONLY_FOR_naoto_murakami) \
  | defined(DEBUG_ONLY_FOR_suginaka_katsunori) | defined(DEBUG_ONLY_FOR_mai_ando)
#define SCR_ASSERT_ON
#endif

#endif //PM_DEBUG

typedef enum
{
  SCREND_CHK_CAMERA = 0,
  SCREND_CHK_WIN_OPEN,
  SCREND_CHK_BALLON_WIN_OPEN,
  SCREND_CHK_PLAINWIN_OPEN,
  SCREND_CHK_BGWIN_OPEN,
  SCREND_CHK_SUBWIN_OPEN,
  SCREND_CHK_SPWIN_OPEN,
  SCREND_CHK_FACEUP,
  /*ここに定義を追加してください*/
  SCREND_CHK_MAX
}SCREND_CHK;

extern void SCREND_CHK_ClearBits(void);
extern void SCREND_CHK_SetBitOn(const SCREND_CHK inCheckBit);
extern void SCREND_CHK_SetBitOff(const SCREND_CHK inCheckBit);
extern BOOL SCREND_CHK_CheckBit(const SCREND_CHK inCheckBit);

