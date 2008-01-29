//===================================================================
/**
 * @file	filesystem_inter.h
 * @brief	ROMファイルシステム
 * @author	GAME FREAK Inc.
 * @date	06.11.09
 *
 * $LastChangedDate$
 * $Id$
 */
//===================================================================
#ifndef __FILESYSTEM_INTER_H__
#define __FILESYSTEM_INTER_H__

// ファイルシステムで利用するDMA番号の定義
// ※のちにDMA関連のヘッダーファイルへ移動予定
#define FS_DMA_NUMBER	1


//------------------------------------------------------------------
/**
 * @brief	ファイルシステム初期化
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
extern void InitFileSystem(void);



#endif	// __FILESYSTEM_INTER_H__
/*  */
