//============================================================================================
/**
 * @file	backup_data.c
 * @brief	バックアップデータの内部構造アクセス
 * @author	tamada GAME FREAK inc.
 * @date	2007.06.12
 */
//============================================================================================

#include <nitro.h>
#include <nnsys.h>

#include "gflib.h"

#include "assert.h"
#include "backup_system.h"
#include "savedata_local.h"


//============================================================================================
//
//			定義
//
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ項目ごとの情報定義
 */
//---------------------------------------------------------------------------
typedef struct {
	GFL_SVDT_ID gmdataID;	///<セーブデータ識別ID
	u32 size;				///<データサイズ格納
	u32 address;			///<データ開始位置
}SVPAGE_INFO;

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ項目管理用の構造体定義
 */
//---------------------------------------------------------------------------
struct _SVDT {
	const GFL_SAVEDATA_TABLE * table;	///<セーブデータ構造定義データへのポインタ
	u32 table_max;						///<セーブデータ構造定義データの要素数
	u32 footer_size;					///<セーブデータに付加するフッタのサイズ
	u32 total_size;						///<セーブデータの実際のサイズ
	u32 savearea_size;					///<割り当てられているバックアップ領域のサイズ
	SVPAGE_INFO * pageinfo;				///<セーブデータ項目毎のデータへのポインタ
};

//============================================================================================
//
//			関数
//
//============================================================================================
static void SVDT_MakeIndex(SVDT * svdt);

static u32 GetWorkSize(const SVDT * svdt, int id);

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ構造管理用データを生成する
 * @return	svdt	セーブデータ構造管理用データへのポインタ
 *
 * 与えられた初期化データに基づいてセーブデータ管理用のデータを生成する。
 */
//---------------------------------------------------------------------------
SVDT * SVDT_Create(HEAPID heap_id, const GFL_SAVEDATA_TABLE * table, u32 table_max,
		u32 savearea_size, u32 footer_size)
{
	SVDT * svdt;
	svdt = GFL_HEAP_AllocMemory(heap_id, sizeof(SVDT));
	GFL_STD_MemClear32(svdt, sizeof(SVDT));
	svdt->table = table;
	svdt->table_max = table_max;
	svdt->savearea_size = savearea_size;
	svdt->footer_size = footer_size;
	svdt->pageinfo = GFL_HEAP_AllocMemory(heap_id, sizeof(SVPAGE_INFO) * table_max);
	SVDT_MakeIndex(svdt);

	return svdt;
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ構造管理用データの解放処理
 * @param	svdt	セーブデータ構造管理用データへのポインタ
 */
//---------------------------------------------------------------------------
void SVDT_Delete(SVDT * svdt)
{
	GFL_HEAP_FreeMemory(svdt->pageinfo);
	GFL_HEAP_FreeMemory(svdt);
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータのクリア
 * @param	svwk	セーブワークへのポインタ
 * @param	svdt	セーブデータ構造管理用データへのポインタ
 */
//---------------------------------------------------------------------------
void SVDT_ClearWork(u8 * svwk, const SVDT * svdt)
{
	const GFL_SAVEDATA_TABLE * table = svdt->table;
	u32 table_max = svdt->table_max;
	int i;
	u32 size;
	void * page;
	u32 adrs;


	GFL_STD_MemClear32(svwk, svdt->savearea_size);

	for (i = 0; i <table_max; i++) {
		adrs = svdt->pageinfo[i].address;
		page = &svwk[adrs];
		size = svdt->pageinfo[i].size;
		GFL_STD_MemClear32(page, size);
		table[i].init_work(page);
	}
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static u32 GetWorkSize(const SVDT * svdt, int id)
{
	u32 size;
	const GFL_SAVEDATA_TABLE * table = svdt->table;
	GF_ASSERT(id < svdt->table_max);
	size = table[id].get_size();
	size += 4 - (size % 4);
	return size;
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブ項目に関する辞書を作成する
 * @param	svdt	セーブデータ構造管理用データへのポインタ
 */
//---------------------------------------------------------------------------
static void SVDT_MakeIndex(SVDT * svdt)
{
	const GFL_SAVEDATA_TABLE * table = svdt->table;
	SVPAGE_INFO * pageinfo = svdt->pageinfo;
	int i;
	u32 table_max = svdt->table_max;
	u32 savedata_total_size = 0;


	for (i = 0; i < table_max; i++) {
		//現時点ではセーブデータの並びとIDの並びが一致することを想定しているので
		//違った場合はASSERTで停止する
		GF_ASSERT(table[i].gmdataID == i);
		pageinfo[i].gmdataID = table[i].gmdataID;
		pageinfo[i].size = GetWorkSize(svdt, i);
		pageinfo[i].address = savedata_total_size;
		savedata_total_size += pageinfo[i].size;
	}
	svdt->total_size = savedata_total_size + svdt->footer_size;
	GF_ASSERT_MSG(svdt->total_size <= svdt->savearea_size,
		"SaveDataSize=%08xbytes, SaveAreaSize=%08xbytes", svdt->total_size, svdt->savearea_size );
}


//============================================================================================
//
//
//
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	各セーブ項目の相対開始位置を取得
 * @param	svdt	セーブデータ構造管理用データへのポインタ
 * @param	gmdataid
 *
 * セーブデータ内での各項目の開始アドレスを取得する
 */
//---------------------------------------------------------------------------
u32 SVDT_GetPageOffset(SVDT * svdt, GFL_SVDT_ID gmdataid)
{
	GF_ASSERT(gmdataid < svdt->table_max);
	return svdt->pageinfo[gmdataid].address;
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ全体の大きさを取得する
 * @param	svdt	セーブデータ構造管理用データへのポインタ
 *
 * 各項目が占有する領域＋フッタ領域（CRCなど）を含めた実セーブデータのサイズを返す
 */
//---------------------------------------------------------------------------
u32 SVDT_GetWorkSize(const SVDT * svdt)
{
	return svdt->total_size;
}

