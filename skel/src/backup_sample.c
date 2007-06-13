//=============================================================================================
/**
 * @file	backup_sample.c
 * @date	2007.06.14
 * @author	GAME FREAK inc.
 * @brief	バックアップ使用サンプル
 *
 */
//=============================================================================================

#include <nitro.h>
#include "gflib.h"
//#include "gflib_inter.h"

#include "main.h"

#include "gfl_use.h"
#include "procsys.h"
#include "gf_overlay.h"
#include "tcb.h"
#include "savedata.h"

//---------------------------------------------------------------------------
///	セーブに使用しているセクタ数
//---------------------------------------------------------------------------
#define	SAVE_PAGE_MAX		(32)

#define	SAVE_SECTOR_SIZE	(0x1000)


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define	SECTOR_SIZE		(SAVE_SECTOR_SIZE)

#define MAGIC_NUMBER	(0x20070612)

#define	FIRST_MIRROR_START	(0 * SECTOR_SIZE)
#define	SECOND_MIRROR_START	(64 * SECTOR_SIZE)


#define	DEFAULT_SAVEAREA_SIZE	(SECTOR_SIZE * SAVE_PAGE_MAX)




static const GFL_SVLD_PARAM init_param;

//=============================================================================================
//=============================================================================================
enum {
	GMDATA_ID_DUMMY_DATA1,
	GMDATA_ID_DUMMY_DATA2,
	GMDATA_ID_DUMMY_DATA3,
	GMDATA_ID_DUMMY_DATA4,

	GMDATA_ID_DUMMY_MAX,
};
//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ構造の初期化
 */
//---------------------------------------------------------------------------
SAVEDATA * SaveData_System_Init(u32 heap_save_id, u32 heap_temp_id)
{
	SAVEDATA * sv;

	GFL_BACKUP_Init(heap_save_id, heap_temp_id);

	sv = GFL_SVLD_Create(&init_param);

	return sv;
}


//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	int hoge;
	u32 fuga;
}DUMMY_DATA1;

static u32 GetDummyData1Size(void)
{
	return sizeof(DUMMY_DATA1);
}
static void InitDummyData1(DUMMY_DATA1 * data)
{
	GFL_STD_MemClear32(data, sizeof(DUMMY_DATA1));
}

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	int hoge;
	u32 fuga;
}DUMMY_DATA2;

static u32 GetDummyData2Size(void)
{
	return sizeof(DUMMY_DATA2);
}
static void InitDummyData2(DUMMY_DATA2 * data)
{
	GFL_STD_MemClear32(data, sizeof(DUMMY_DATA2));
}

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	int hoge;
	u32 fuga;
}DUMMY_DATA3;

static u32 GetDummyData3Size(void)
{
	return sizeof(DUMMY_DATA3);
}
static void InitDummyData3(DUMMY_DATA3 * data)
{
	GFL_STD_MemClear32(data, sizeof(DUMMY_DATA3));
}

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	int hoge;
	u32 fuga;
}DUMMY_DATA4;

static u32 GetDummyData4Size(void)
{
	return sizeof(DUMMY_DATA4);
}
static void InitDummyData4(DUMMY_DATA4 * data)
{
	GFL_STD_MemClear32(data, sizeof(DUMMY_DATA4));
}

//------------------------------------------------------------------
//------------------------------------------------------------------

const SAVEDATA_TABLE SaveDataTable[] = {
	{
		GMDATA_ID_DUMMY_DATA1,
		(FUNC_GET_SIZE)GetDummyData1Size,
		(FUNC_INIT_WORK)InitDummyData1,
	},
	{
		GMDATA_ID_DUMMY_DATA2,
		(FUNC_GET_SIZE)GetDummyData2Size,
		(FUNC_INIT_WORK)InitDummyData2,
	},
	{
		GMDATA_ID_DUMMY_DATA3,
		(FUNC_GET_SIZE)GetDummyData3Size,
		(FUNC_INIT_WORK)InitDummyData3,
	},
	{
		GMDATA_ID_DUMMY_DATA4,
		(FUNC_GET_SIZE)GetDummyData4Size,
		(FUNC_INIT_WORK)InitDummyData4,
	},
};

//------------------------------------------------------------------
//------------------------------------------------------------------
static const GFL_SVLD_PARAM init_param = {
	SaveDataTable,
	NELEMS(SaveDataTable),
	FIRST_MIRROR_START,
	DEFAULT_SAVEAREA_SIZE,
	MAGIC_NUMBER,
};


