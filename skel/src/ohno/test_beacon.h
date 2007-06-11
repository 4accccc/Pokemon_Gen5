//============================================================================================
/**
 * @file	test_beacon.h
 * @brief	デバッグ用関数定義
 * @author	ohno
 * @date	2006.12.18
 */
//============================================================================================


#ifndef __TEST_BEACON_H__
#define __TEST_BEACON_H__

typedef struct _SKEL_TEST_BEACON_WORK SKEL_TEST_BEACON_WORK;

extern void TEST_BEACON_End(SKEL_TEST_BEACON_WORK* pWork);
extern void TEST_BEACON_Main(SKEL_TEST_BEACON_WORK* pWork);
extern SKEL_TEST_BEACON_WORK* TEST_BEACON_Init(HEAPID heapID);


#endif //__TEST_BEACON_H__




