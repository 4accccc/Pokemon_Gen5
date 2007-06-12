//============================================================================================
/**
 * @file	savedata_local.h
 * @brief
 * @date	2006.04.17
 * @author	tamada GAME FREAK inc.
 */
//============================================================================================

#ifndef	__SAVEDATA_LOCAL_H__
#define	__SAVEDATA_LOCAL_H__

#include "heap.h"
#include "savedata.h"

#ifdef	PM_DEBUG
//この定義を有効にするとフラッシュのエラーチェックが無効になる
//デバッグ期間用なので製品版では繁栄させないこと！
//#define	DISABLE_FLASH_CHECK
#endif

//=============================================================================
//=============================================================================
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define	SECTOR_SIZE		(SAVE_SECTOR_SIZE)
#define SECTOR_MAX		(SAVE_PAGE_MAX)

#define	SAVEAREA_SIZE	(SECTOR_SIZE * SECTOR_MAX)
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
typedef u32 (*FUNC_GET_SIZE)(void);

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
typedef void (*FUNC_INIT_WORK)(void *);

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ内容データ用の構造体定義
 */
//---------------------------------------------------------------------------
typedef struct {
	GMDATA_ID gmdataID;				///<セーブデータ識別ID
	SVBLK_ID	blockID;
	FUNC_GET_SIZE get_size;			///<セーブデータサイズ取得関数
	FUNC_INIT_WORK	init_work;		///<セーブデータ初期化関数
}SAVEDATA_TABLE;

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ項目ごとの情報定義
 */
//---------------------------------------------------------------------------
typedef struct {
	GMDATA_ID gmdataID;	///<セーブデータ識別ID
	u32 size;			///<データサイズ格納
	u32 address;		///<データ開始位置
	u16 crc;			///<エラー検出用CRCコード格納
	u16 blockID;		///<所属ブロックのID
}SVPAGE_INFO;

//---------------------------------------------------------------------------
/**
 * @brief	セーブブロックデータ情報
 */
//---------------------------------------------------------------------------
typedef struct {
	u32 id;				///<ブロック定義ID
	u32 start_sec;		///<開始セクタ位置
	u32 use_sec;			///<占有セクタ数
	u32 start_ofs;		///<セーブデータでの開始オフセット
	u32 size;			///<セーブデータの占有サイズ
}SVBLK_INFO;


typedef struct _SVDT SVDT;

//=============================================================================
//=============================================================================

extern const SAVEDATA_TABLE SaveDataTable[];
extern const int SaveDataTableMax;


extern SVDT * SVDT_Create(HEAPID heap_id, const SAVEDATA_TABLE * table, u32 table_max, u32 footer_size);

//extern void SVDT_ClearWork(u8 * svwk, const SVPAGE_INFO * pageinfo, u32 savearea_size);
extern void SVDT_ClearWork(u8 * svwk, const SVDT * svdt, u32 savearea_size);
extern void SVDT_MakeBlockIndex(SVBLK_INFO * blkinfo, const SVPAGE_INFO * pageinfo, u32 footer_size);
extern u32 SVDT_GetPageOffset(SVDT * svdt, GMDATA_ID gmdataid);


#endif	/* __SAVEDATA_LOCAL_H__ */
