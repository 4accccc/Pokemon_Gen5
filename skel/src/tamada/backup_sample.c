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
#include "backup_system.h"

#include "backup_sample.h"

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
GFL_SAVEDATA * SavePointer;

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
GFL_SAVEDATA * BackupSample_Create(void)
{
	GFL_SAVEDATA * sv;

	sv = GFL_SAVEDATA_Create(&init_param);
	SavePointer = sv;

	BackupSample_DumpData(sv);
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
static DUMMY_DATA1 * GetDummyData1(GFL_SAVEDATA * sv)
{
	return GFL_SAVEDATA_Get(sv, GMDATA_ID_DUMMY_DATA1);
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
static DUMMY_DATA2 * GetDummyData2(GFL_SAVEDATA * sv)
{
	return GFL_SAVEDATA_Get(sv, GMDATA_ID_DUMMY_DATA2);
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
static DUMMY_DATA3 * GetDummyData3(GFL_SAVEDATA * sv)
{
	return GFL_SAVEDATA_Get(sv, GMDATA_ID_DUMMY_DATA3);
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
static DUMMY_DATA4 * GetDummyData4(GFL_SAVEDATA * sv)
{
	return GFL_SAVEDATA_Get(sv, GMDATA_ID_DUMMY_DATA4);
}

//------------------------------------------------------------------
//------------------------------------------------------------------

const GFL_SAVEDATA_TABLE SaveDataTable[] = {
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
	GFL_BACKUP_NORMAL_FLASH
};


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void BackupSample_DumpData(GFL_SAVEDATA * sv)
{
	{
		DUMMY_DATA1 * d;
		d = GetDummyData1(sv);
		OS_TPrintf("DUMMY1::hoge = %d\n", d->hoge);
		OS_TPrintf("DUMMY1::fuga = %d\n", d->fuga);
	}
	{
		DUMMY_DATA2 * d;
		d = GetDummyData2(sv);
		OS_TPrintf("DUMMY2::hoge = %d\n", d->hoge);
		OS_TPrintf("DUMMY2::fuga = %d\n", d->fuga);
	}
	{
		DUMMY_DATA3 * d;
		d = GetDummyData3(sv);
		OS_TPrintf("DUMMY3::hoge = %d\n", d->hoge);
		OS_TPrintf("DUMMY3::fuga = %d\n", d->fuga);
	}
	{
		DUMMY_DATA4 * d;
		d = GetDummyData4(sv);
		OS_TPrintf("DUMMY4::hoge = %d\n", d->hoge);
		OS_TPrintf("DUMMY4::fuga = %d\n", d->fuga);
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void BackupSample_SetData(GFL_SAVEDATA * sv, const u16 * data)
{
	{
		DUMMY_DATA1 * d;
		d = GetDummyData1(sv);
		 d->hoge = data[0];
		 d->fuga = data[1];
	}
	{
		DUMMY_DATA2 * d;
		d = GetDummyData2(sv);
		 d->hoge = data[2];
		 d->fuga = data[3];
	}
	{
		DUMMY_DATA3 * d;
		d = GetDummyData3(sv);
		 d->hoge = data[4];
		 d->fuga = data[5];
	}
	{
		DUMMY_DATA4 * d;
		d = GetDummyData4(sv);
		 d->hoge = data[6];
		 d->fuga = data[7];
	}
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
BOOL BackupSample_Control(void)
{
	int key = GFL_UI_KEY_GetTrg();
	GFL_SAVEDATA * sv = SavePointer;
	if (key & PAD_KEY_UP) {
		const u16 dt[] = { 0, 1, 2, 3, 4, 5, 6, 7};
		BackupSample_SetData(sv,  dt);
		BackupSample_DumpData(sv);
	} else if (key & PAD_KEY_DOWN) {
		const u16 dt[] = { 4, 4, 4, 4, 8, 8, 8, 8};
		BackupSample_SetData(sv,  dt);
		BackupSample_DumpData(sv);
	} else if (key & PAD_KEY_LEFT) {
		const u16 dt[] = { 100, 0, 100, 0, 201, 10, 202, 11 };
		BackupSample_SetData(sv,  dt);
		BackupSample_DumpData(sv);
	} else if (key & PAD_KEY_RIGHT) {
		const u16 dt[] = { 1, 4, 9, 16, 25, 36, 49, 64 };
		BackupSample_SetData(sv,  dt);
		BackupSample_DumpData(sv);
	} else if (key & PAD_BUTTON_A) {
		SAVE_RESULT res = GFL_BACKUP_Save(sv);
		OS_TPrintf("SAVE_RESULT %d\n", res);
		BackupSample_DumpData(sv);
	} else if (key & PAD_BUTTON_B) {
		LOAD_RESULT res = GFL_BACKUP_Load(sv);
		OS_TPrintf("LOAD_RESULT %d\n", res);
		BackupSample_DumpData(sv);
	}
	return FALSE;
}




