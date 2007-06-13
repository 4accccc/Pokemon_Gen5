//============================================================================================
/**
 * @brief
 * @file
 * @author	tamada GAME FREAK inc.
 * @date	2007.06.12
 */
//============================================================================================

#include <nitro.h>
#include <nnsys.h>

#include "gflib.h"

#include "assert.h"
#include "savedata.h"
#include "savedata_local.h"
//============================================================================================
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ項目ごとの情報定義
 */
//---------------------------------------------------------------------------
typedef struct {
	DATA_ID gmdataID;	///<セーブデータ識別ID
	u32 size;			///<データサイズ格納
	u32 address;		///<データ開始位置
}SVPAGE_INFO;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct _SVDT {
	const SAVEDATA_TABLE * table;
	u32 table_max;
	u32 footer_size;
	u32 total_size;
	u32 savearea_size;
	SVPAGE_INFO * pageinfo;
};

//============================================================================================
//============================================================================================
static void SVDT_MakeIndex(SVDT * svdt);

static u32 GetWorkSize(const SVDT * svdt, int id);

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
SVDT * SVDT_Create(HEAPID heap_id, const SAVEDATA_TABLE * table, u32 table_max,
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
 * @param	pageinfo	セーブ項目保持ワーク
 */
//---------------------------------------------------------------------------
void SVDT_ClearWork(u8 * svwk, const SVDT * svdt)
{
	const SAVEDATA_TABLE * table = svdt->table;
	u32 table_max = svdt->table_max;
	int i;
	u32 size;
	void * page;
	u32 adrs;


	MI_CpuClearFast(svwk, svdt->savearea_size);

	for (i = 0; i <table_max; i++) {
		adrs = svdt->pageinfo[i].address;
		page = &svwk[adrs];
		size = svdt->pageinfo[i].size;
		MI_CpuClearFast(page, size);
		table[i].init_work(page);
	}

}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static u32 GetWorkSize(const SVDT * svdt, int id)
{
	u32 size;
	const SAVEDATA_TABLE * table = svdt->table;
	GF_ASSERT(id < svdt->table_max);
	size = table[id].get_size();
	size += 4 - (size % 4);
	return size;
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブ項目に関する辞書を作成する
 * @param	pageinfo	セーブ項目保持ワーク
 */
//---------------------------------------------------------------------------
static void SVDT_MakeIndex(SVDT * svdt)
{
	const SAVEDATA_TABLE * table = svdt->table;
	SVPAGE_INFO * pageinfo = svdt->pageinfo;
	int i;
	u32 table_max = svdt->table_max;
	u32 savedata_total_size = 0;


	for (i = 0; i < table_max; i++) {
		GF_ASSERT(table[i].gmdataID == i);
		pageinfo[i].gmdataID = table[i].gmdataID;
		pageinfo[i].size = GetWorkSize(svdt, i);
		pageinfo[i].address = savedata_total_size;
		savedata_total_size += pageinfo[i].size;
#ifdef	DEBUG_ONLY_FOR_tamada
		if (i % 4 == 0) {
			OS_TPrintf("SVDT:");
		}
		OS_TPrintf("%02d:%05x(%05x) ", pageinfo[i].gmdataID, 
				pageinfo[i].address, pageinfo[i].size);
		if (i % 4 == 3) {
			OS_PutString("\n");
		}
#endif
	}
	if (i % 4 != 0) {
		OS_PutString("\n");
	}
	svdt->total_size = savedata_total_size + svdt->footer_size;
	GF_ASSERT(svdt->total_size <= svdt->savearea_size);
}


//============================================================================================
//
//
//
//============================================================================================
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
u32 SVDT_GetPageOffset(SVDT * svdt, DATA_ID gmdataid)
{
	GF_ASSERT(gmdataid < svdt->table_max);
	return svdt->pageinfo[gmdataid].address;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
u32 SVDT_GetWorkSize(const SVDT * svdt)
{
	return svdt->total_size;
}

