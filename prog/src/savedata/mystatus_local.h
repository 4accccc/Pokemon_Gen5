//============================================================================================
/**
 * @file	mystatus_local.h
 * @author	tamada
 * @date	2006.01.27
 * @brief	自分状態データ型
 *
 * セーブデータ関連の共有ヘッダ。外部公開はしない。
 */
//============================================================================================
#ifndef	__MYSTATUS_LOCAL_H__
#define	__MYSTATUS_LOCAL_H__

#include "buflen.h"

//----------------------------------------------------------
/**
 * @brief	自分状態データ型定義
 */
//----------------------------------------------------------
struct _MYSTATUS {
	STRCODE name[PERSON_NAME_SIZE + EOM_SIZE];		// 16
	u32 id;								// 20
	u32 profileID;				// 24
  u8 nation;            ///< 国  25
  u8 area;              ///< 地域 26
	u8 region_code;				//PM_LANG   27
	u8 rom_code;	       	// PM_VERSION   28
  u8 sex:2;	   //29
  u8 trainer_view:4;	// ユニオンルーム内での見た目フラグ //29
	u8 dummy1:2;									// 29
	u8 dummy2;										// 30
	u8 dummy3;										// 31
	u8 dummy4;										// 32
};

#endif	/* __MYSTATUS_LOCAL_H__ */
