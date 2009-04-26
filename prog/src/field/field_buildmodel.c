//============================================================================================
/**
 * @file	field_buildmodel.c
 * @brief		配置モデルの制御
 * @author	tamada GAMEFREAK inc.
 * @date	2009.04.20
 */
//============================================================================================

#include <gflib.h>

#include "arc_def.h"
#include "field/zonedata.h"
#include "field/areadata.h"

#include "field_buildmodel.h"

#include "field_g3d_mapper.h"
#include "fieldmap_resist.h"

//============================================================================================
//============================================================================================

//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
	BMODEL_ID_MAX = 400,	//とりあえず
	BMODEL_ENTRY_MAX = 128,	//とりあえず

	BMODEL_ID_NG	= BMODEL_ID_MAX,
	BMODEL_ENTRY_NG = BMODEL_ENTRY_MAX,
};

//------------------------------------------------------------------
//------------------------------------------------------------------
struct _FIELD_BMODEL_MAN
{
	HEAPID heapID;
	u8 tbl[BMODEL_ID_MAX];
	BMODEL_ID entry_index[BMODEL_ENTRY_MAX];
	u16 entry_count;
	ARCID use_arcid;

	FLDMAPPER_RESISTDATA_OBJTBL	gobjData_Tbl;
	FLDMAPPER_RESISTOBJDATA resistObjTbl[BMODEL_ENTRY_MAX * 2];
};
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static FIELD_BMODEL_MAN * GlobalManager;

//============================================================================================
//============================================================================================
static void makeTblFromIndex(FIELD_BMODEL_MAN * man);
static void makeResistObjTable(FIELD_BMODEL_MAN * man);

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
FIELD_BMODEL_MAN * FIELD_BMODEL_MAN_Create(HEAPID heapID)
{	
	FIELD_BMODEL_MAN * man = GFL_HEAP_AllocMemory(heapID, sizeof(FIELD_BMODEL_MAN));
	man->heapID = heapID;
	GlobalManager = man;
	return man;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_Delete(FIELD_BMODEL_MAN * man)
{	
	GFL_HEAP_FreeMemory(man);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_BMODEL_MAN_Load(FIELD_BMODEL_MAN * man, u16 zoneid)
{	
	ARCHANDLE * handle;
	u16 area_id = ZONEDATA_GetAreaID(zoneid);

	GFL_STD_MemClear(man->tbl, sizeof(man->tbl));
	GFL_STD_MemFill16(man->entry_index, BMODEL_ENTRY_NG, sizeof(man->entry_index));

	{	
		ARCID index_arc;
		if (AREADATA_GetInnerOuterSwitch(area_id) != 0) 
		{	
			man->use_arcid = ARCID_BMODEL_OUTDOOR;
			index_arc = ARCID_BMODEL_IDX_OUTDOOR;
		}
		else
		{	
			man->use_arcid = ARCID_BMODEL_INDOOR;
			index_arc = ARCID_BMODEL_IDX_INDOOR;
		}
		handle = GFL_ARC_OpenDataHandle(index_arc, man->heapID);
	}
	{	
		u16 data_count = GFL_ARC_GetDataFileCntByHandle(handle);
		TAMADA_Printf("bmodel list id = %d\n",data_count);
		if (data_count < area_id)
		{	
			GF_ASSERT_MSG(0, "配置モデルリストデータがありません(%d<%d)\n", data_count, area_id);
			area_id = 0;		//とりあえずハングアップ回避
		}
	}
	//読み込み
	{	
		u16 size = GFL_ARC_GetDataSizeByHandle(handle, area_id);
		man->entry_count = size / sizeof(BMODEL_ID);
		if(size > sizeof(man->entry_index))
		{	
			GF_ASSERT_MSG(0, "配置モデルリストデータが大きすぎます（AREA=%d)\n", area_id);
			man->entry_count = BMODEL_ENTRY_MAX;	//とりあえずハングアップ回避
			size = sizeof(man->entry_index);
		}
		TAMADA_Printf("entry_count=%d\n", man->entry_count);
		GFL_ARC_LoadDataOfsByHandle(handle, area_id, 0, size, man->entry_index);
	}
	GFL_ARC_CloseDataHandle(handle);

	//データテーブル生成
	makeTblFromIndex(man);
	makeResistObjTable(man);

}
//------------------------------------------------------------------
//------------------------------------------------------------------
const FLDMAPPER_RESISTDATA_OBJTBL * FIELD_BMODEL_MAN_GetOBJTBL(const FIELD_BMODEL_MAN * man)
{	
	return &man->gobjData_Tbl;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
u16 FIELD_BMODEL_MAN_GetNarcIndex(const FIELD_BMODEL_MAN * man, BMODEL_ID id)
{
	int index;
	GF_ASSERT(id < BMODEL_ID_MAX);
	index = man->tbl[id];
	GF_ASSERT(index < BMODEL_ENTRY_MAX);
	return man->entry_index[index];
}

//------------------------------------------------------------------
//------------------------------------------------------------------
u16 FIELD_BMODEL_MAN_GetEntryIndex(BMODEL_ID id)
{	
	return FIELD_BMODEL_MAN_GetNarcIndex(GlobalManager, id);
}
//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//BMODEL_IDで引くtblにはentry_indexへのオフセットを格納
//------------------------------------------------------------------
static void makeTblFromIndex(FIELD_BMODEL_MAN * man)
{
	int i;
	for (i = 0; i < man->entry_count; i++)
	{	
		int pos = man->entry_index[i];
		man->tbl[pos] = i;
	}
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void makeResistObjTable(FIELD_BMODEL_MAN * man)
{
	static const FLDMAPPER_RESISTDATA_OBJTBL init = {	
		0, NULL, 0, 0, NULL, 0
	};
	int i;
	FLDMAPPER_RESISTDATA_OBJTBL * resist = &man->gobjData_Tbl;
	FLDMAPPER_RESISTOBJDATA * mapperObj = man->resistObjTbl;
	GFL_STD_MemFill16(mapperObj, NON_LOWQ, sizeof(man->resistObjTbl));
	for (i = 0; i < man->entry_count; i++)
	{	
		mapperObj[i].highQ_ID = man->entry_index[i];
	}
	*resist = init;
	resist->objArcID = man->use_arcid;
	resist->objData = mapperObj;
	resist->objCount = man->entry_count;
}




