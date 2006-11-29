//===================================================================
/**
 * @file	main.h
 * @brief	アプリケーション　サンプルスケルトン
 * @author	GAME FREAK Inc.
 * @date	06.11.28
 *
 * $LastChangedDate$
 * $Id$
 */
//===================================================================

//ヒープ用宣言
enum{
//	HEAPID_BASE_SYSTEM=0,
//	HEAPID_BASE_APP,
	HEAPID_BASE_APP = HEAPID_BASE_SYSTEM + 1,

	HEAPID_BASE_MAX
};

#define	HEAPSIZE_SYSTEM	(0x8000)
#define	HEAPSIZE_APP	(0x8000)

