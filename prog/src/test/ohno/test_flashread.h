//=============================================================================
/**
 * @file	test_flashread.h
 * @brief	フラッシュ逐次読み込みテスト
 * @author	Katsumi Ohno
 * @date    2005.07.08
 */
//=============================================================================

#ifndef _TEST_FLASHREAD_H_
#define _TEST_FLASHREAD_H_

#if 0
extern void TEST_ReadFlashStart(GFL_TCBSYS* pTCB,HEAPID id,u32 size);
extern void TEST_ReadFlashEnd(void);

#else
extern void TEST_ReadFlashStart(u8* pData, int size);
extern void TEST_ReadFlashRead(u8* pData, int offset, int size);
#endif

#endif //_TEST_FLASHREAD_H_
