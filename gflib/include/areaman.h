//==============================================================================
/**
 *
 *@file		areaman.h
 *@brief	VRAMキャラデータ領域マネージャ
 *@author	taya
 *@data		2006.10.27
 *
 */
//==============================================================================
#ifndef __AREAMAN_H__
#define __AREAMAN_H__

#include "heapsys.h"

//--------------------------------------------
// 不完全型定義
//--------------------------------------------
typedef struct _GFL_AREAMAN		GFL_AREAMAN;


//--------------------------------------------
// 
//--------------------------------------------
typedef	u32		GFL_AREAMAN_POS;

#define AREAMAN_POS_NOTFOUND	(0xffffffff)

//------------------------------------------------------------------
/**
 * 領域マネージャ作成
 *
 * @param   maxBlock		[in] このマネージャーが管理するブロック数	
 * @param   heapID			[in] 使用ヒープＩＤ
 *
 * @retval  GFL_AREAMAN*	領域マネージャポインタ
 */
//------------------------------------------------------------------
GFL_AREAMAN*
	GFL_AREAMAN_Create
		( u32 maxBlock, u32 heapID );
//------------------------------------------------------------------
/**
 * 領域マネージャ破棄
 *
 * @param   man			[in] 領域マネージャポインタ
 */
//------------------------------------------------------------------
void
	GFL_AREAMAN_Delete
		( GFL_AREAMAN* man );
//------------------------------------------------------------------
/**
 * 領域先頭から末尾まで、空いている所を探して確保
 *
 * @param   man				[in] マネージャ
 * @param   blockNum		[in] 確保したいブロック数
 *
 * @retval  GFL_AREAMAN_POS		確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//------------------------------------------------------------------
u32
	GFL_AREAMAN_ReserveAuto
		( GFL_AREAMAN* man, u32 blockNum );
//------------------------------------------------------------------
/**
 * 領域の指定範囲内から空いている所を探して確保
 *
 * @param   man				[in] マネージャ
 * @param   startBlock		[in] 探索開始ブロック
 * @param   numBlockArea	[in] 探索ブロック範囲
 * @param   numBlockReserve	[in] 確保したいブロック数
 *
 * @retval  GFL_AREAMAN_POS		確保できた位置（できなければ AREAMAN_POS_NOTFOUND）
 */
//------------------------------------------------------------------
u32
	GFL_AREAMAN_ReserveAssignArea
		( GFL_AREAMAN* man, u32 startBlock, u32 numBlockArea, u32 numBlockReserve );
//------------------------------------------------------------------
/**
 * 領域の指定位置から確保
 *
 * @param   man				[in] マネージャ
 * @param   blockNum		[in] 確保したいブロック数
 *
 * @retval  BOOL			TRUEで成功
 */
//------------------------------------------------------------------
BOOL
	AREAMAN_ReserveAssignPos
		( GFL_AREAMAN* man, GFL_AREAMAN_POS pos, u32 blockNum );
//------------------------------------------------------------------
/**
 * 領域を開放
 *
 * @param   man			[in] マネージャ
 * @param   pos			[in] 確保している位置
 * @param   blockNum	[in] 確保しているブロック数
 */
//------------------------------------------------------------------
void 
	AREAMAN_Release
		( GFL_AREAMAN* man, GFL_AREAMAN_POS pos, u32 blockNum );

#endif	// __AREAMAN_H__
