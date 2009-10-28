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

typedef enum
{
  SCREND_CHK_CAMERA = 0,
  SCREND_CHK_WIN_OPEN,
  /*ここに定義を追加してください*/
  SCREND_CHK_MAX
}SCREND_CHK;

extern void SCREND_CHK_ClearBits(void);
extern void SCREND_CHK_SetBitOn(const SCREND_CHK inCheckBit);
extern void SCREND_CHK_SetBitOff(const SCREND_CHK inCheckBit);
extern BOOL SCREND_CHK_CheckBit(const SCREND_CHK inCheckBit);

