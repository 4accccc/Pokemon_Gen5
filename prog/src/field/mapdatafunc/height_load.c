//#include "common.h"
#include "height.h"
//#include "system/arc_tool.h"

#define GRID_ENTRY_HEADER_SIZE	(4)

#define DIV_HEIGHT_DATA_SIZE	(/*0x4000*/0xe000)

#define HEIGHT_HEADER_SIZE	(40)


typedef struct {
	int VtxNum;
	int NrmNum;
	int PolygonNum;
	int GridNum;
	int TotalPolyIDListSize;
	int TotalLineListSize;
	int LineEntryMax;
}READ_INFO;


#define	HEAPID_FIELD HEAPID_WATANABE_DEBUG
//////////////////////////////////////////////////////////////////////////////
// 前方宣言
//////////////////////////////////////////////////////////////////////////////
static void* ReadData(void *buf, int len, FSFile *p_file);
static void ReadHeaderData(FSFile *p_file, READ_INFO *outInfo);
static void SetDataMem(const READ_INFO *inInfo, MHI_PTR	outMapHeightInfo, void **mem);

static void ReadGridSpData(FSFile *p_file, MHI_PTR outMap3DInfo);
static void ReadVtxData(FSFile *p_file, MHI_PTR outMap3DInfo, const READ_INFO *inInfo);
static void ReadNrmData(FSFile *p_file, MHI_PTR outMap3DInfo, const READ_INFO *inInfo);
static void ReadPolygonData(FSFile *p_file, MHI_PTR outMap3DInfo, const READ_INFO *inInfo);
static void ReadGridDataTbl(FSFile *p_file, MHI_PTR outMap3DInfo, const READ_INFO *inInfo);
static void ReadLineDataTbl(FSFile *p_file, MHI_PTR outMap3DInfo, const READ_INFO *inInfo);
static void ReadGridData(FSFile *p_file, MHI_PTR outMap3DInfo, const READ_INFO *inInfo);



static void* ReadData(void *buf, int len, FSFile *p_file)
{
    int ret = FS_ReadFile(p_file, buf, len);
    if(ret < 0){
		return  NULL;
	}
    return  buf;
}

static void reader(void *buf, int len, const void **mem)
{
	const void * read = *mem;
	MI_CpuCopy8(read, buf, len);
	(u8*)read += len;
	*mem = read;
}

///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
	u32 id;
	u32 header_size;
	u16 vtx_size;
	u16 vtx_num;
	u16 nrm_size;
	u16 nrm_num;
	u16 poly_size;
	u16 poly_num;
	u16 grid_div_size;
	u16 grid_entry_size;
	u16 grid_num;
	u16 line_entry_size;
	u16 grid_line_max;
	u16 dummy;
	u32 line_num;
	u32 grid_entry_total;
}DP_HEIGHT_HEADER;
//ヘッダーデータ8+32バイトを読む
static void ReadHeaderData(FSFile *p_file, READ_INFO *outInfo)
{	
	u16 temp[4];

	//格納先のクリーン
	MI_CpuClear32(outInfo,sizeof(READ_INFO));

	//ID、ヘッダーサイズデータ計８バイトを読み飛ばす
	ReadData(temp, 8, p_file);
	//１つの頂点データサイズ（今は読み飛ばす）
	ReadData(temp, 2, p_file);
	//頂点数
	ReadData(&outInfo->VtxNum, 2, p_file);
	//１つの法線データサイズ（今は読み飛ばす）
	ReadData(temp, 2, p_file);
	//法線数
	ReadData(&outInfo->NrmNum, 2, p_file);
	//１つのポリゴンデータサイズ（今は読み飛ばす）
	ReadData(temp, 2, p_file);
	//三角ポリゴン数
	ReadData(&outInfo->PolygonNum, 2, p_file);
	//グリッド分割データサイズ（今は読み飛ばす）
	ReadData(temp, 2, p_file);
	//グリッドデータサイズ（今は読み飛ばす）
	ReadData(temp, 2, p_file);
	//グリッド数
	ReadData(&outInfo->GridNum, 2, p_file);
	//ラインエントリデータのヘッダサイズ（今は読み飛ばす）
	ReadData(temp, 2, p_file);
	//ラインエントリ最大数
	ReadData(&outInfo->LineEntryMax, 2, p_file);
	//ダミー
	ReadData(temp, 2, p_file);
	//ライン総数
	ReadData(&outInfo->TotalLineListSize, 4, p_file);
	//ポリゴンインデックス実データ
	ReadData(&outInfo->TotalPolyIDListSize, 4, p_file);

#if 0
OS_Printf("VtxNum				%d\n", outInfo->VtxNum );
OS_Printf("NrmNum				%d\n", outInfo->NrmNum);
OS_Printf("PolygonNum			%d\n", outInfo->PolygonNum);
OS_Printf("GridNum				%d\n", outInfo->GridNum);
OS_Printf("LineEntryMax			%d\n", outInfo->LineEntryMax );
OS_Printf("TotalLineListSize	%d\n", outInfo->TotalLineListSize);
OS_Printf("TotalPolyIDListSize	%d\n", outInfo->TotalPolyIDListSize);
#endif
}

static void readerHeader(const void **mem, READ_INFO *outInfo)
{	
	u16 temp[4];

	//格納先のクリーン
	MI_CpuClear32(outInfo,sizeof(READ_INFO));

	//ID、ヘッダーサイズデータ計８バイトを読み飛ばす
	reader(temp, 8, mem);
	//１つの頂点データサイズ（今は読み飛ばす）
	reader(temp, 2, mem);
	//頂点数
	reader(&outInfo->VtxNum, 2, mem);
	//１つの法線データサイズ（今は読み飛ばす）
	reader(temp, 2, mem);
	//法線数
	reader(&outInfo->NrmNum, 2, mem);
	//１つのポリゴンデータサイズ（今は読み飛ばす）
	reader(temp, 2, mem);
	//三角ポリゴン数
	reader(&outInfo->PolygonNum, 2, mem);
	//グリッド分割データサイズ（今は読み飛ばす）
	reader(temp, 2, mem);
	//グリッドデータサイズ（今は読み飛ばす）
	reader(temp, 2, mem);
	//グリッド数
	reader(&outInfo->GridNum, 2, mem);
	//ラインエントリデータのヘッダサイズ（今は読み飛ばす）
	reader(temp, 2, mem);
	//ラインエントリ最大数
	reader(&outInfo->LineEntryMax, 2, mem);
	//ダミー
	reader(temp, 2, mem);
	//ライン総数
	reader(&outInfo->TotalLineListSize, 4, mem);
	//ポリゴンインデックス実データ
	reader(&outInfo->TotalPolyIDListSize, 4, mem);

#if 0
OS_Printf("VtxNum				%d\n", outInfo->VtxNum );
OS_Printf("NrmNum				%d\n", outInfo->NrmNum);
OS_Printf("PolygonNum			%d\n", outInfo->PolygonNum);
OS_Printf("GridNum				%d\n", outInfo->GridNum);
OS_Printf("LineEntryMax			%d\n", outInfo->LineEntryMax );
OS_Printf("TotalLineListSize	%d\n", outInfo->TotalLineListSize);
OS_Printf("TotalPolyIDListSize	%d\n", outInfo->TotalPolyIDListSize);
#endif
}


static void SetDataMem(const READ_INFO *inInfo, MHI_PTR	outMapHeightInfo, void **mem)
{
	void *p;
	int offset;
	u8 mod;
	offset = 0;

	//頂点データ
	p = &((u8*)(*mem))[0];
	outMapHeightInfo->VertexArray = p;//(void *)&data[0];
	offset += (sizeof(VecFx32)*inInfo->VtxNum);
	mod = offset%4;
	if (mod != 0){
		offset += mod;
	}
	//法線データ
	p = &((u8*)(*mem))[offset];
	outMapHeightInfo->NormalArray = p;//(void *)&data[offset];
	offset += (sizeof(VecFx32)*inInfo->NrmNum);
	mod = offset%4;
	if (mod != 0){
		offset += mod;
	}
	//ポリゴンデータ
	p = &((u8*)(*mem))[offset];
	outMapHeightInfo->PolygonData = p;//(void *)&data[offset];
	offset += (sizeof(POLYGON_DATA)*inInfo->PolygonNum);
	mod = offset%4;
	if (mod != 0){
		offset += mod;
	}
	//グリッド分割データ
	p = &((u8*)(*mem))[offset];
	outMapHeightInfo->SplitGridData = p;//(void *)&data[offset];
	offset += (sizeof(SPLIT_GRID_DATA));
	mod = offset%4;
	if (mod != 0){
		offset += mod;
	}
	//グリッドテーブルデータ
	p = &((u8*)(*mem))[offset];
	outMapHeightInfo->GridDataTbl = p;//(void *)&data[offset];
	offset += (GRID_ENTRY_HEADER_SIZE*inInfo->GridNum);
	mod = offset%4;
	if (mod != 0){
		offset += mod;
	}
	//ラインテーブルデータ
	p = &((u8*)(*mem))[offset];
	outMapHeightInfo->LineDataTbl = p;//(void *)&data[offset];
	offset += (inInfo->TotalLineListSize);
	mod = offset%4;
	if (mod != 0){
		offset += mod;
	}
	//ライン内ポリゴンIDエントリデータ
	p = &((u8*)(*mem))[offset];
	outMapHeightInfo->PolyIDList = p;//(void *)&data[offset];
	offset += (inInfo->TotalPolyIDListSize);
	mod = offset%4;
	if (mod != 0){
		offset += mod;
	}

#ifdef DEBUG_ONLY_FOR_saitou
	OS_Printf("local_print height_data_toal:%x\n",
			(sizeof(VecFx32)*inInfo->VtxNum)+(sizeof(VecFx32)*inInfo->NrmNum)+ \
			sizeof(POLYGON_DATA)*inInfo->PolygonNum+sizeof(SPLIT_GRID_DATA)+ \
			(GRID_ENTRY_HEADER_SIZE*inInfo->GridNum)+inInfo->TotalLineListSize+ \
			inInfo->TotalPolyIDListSize);
#endif
	
	GF_ASSERT(offset<=0x9000&&"高さデータが大きすぎです");
}

//グリッド分割データをリード
static void ReadGridSpData(FSFile *p_file, MHI_PTR outMap3DInfo)
{
	ReadData(outMap3DInfo->SplitGridData, sizeof(SPLIT_GRID_DATA), p_file);
}
//ヘッダから取得した、頂点数のデータを読む
static void ReadVtxData(FSFile *p_file, MHI_PTR outMap3DInfo, const READ_INFO *inInfo)
{
	ReadData(outMap3DInfo->VertexArray, sizeof(VecFx32)*inInfo->VtxNum, p_file);
}

static void ReadNrmData(FSFile *p_file, MHI_PTR outMap3DInfo, const READ_INFO *inInfo)
{
	ReadData(outMap3DInfo->NormalArray, sizeof(VecFx32)*inInfo->NrmNum, p_file);
}

static void ReadPolygonData(FSFile *p_file, MHI_PTR outMap3DInfo, const READ_INFO *inInfo)
{
	ReadData(outMap3DInfo->PolygonData, sizeof(POLYGON_DATA)*inInfo->PolygonNum, p_file);
}

static void ReadGridDataTbl(FSFile *p_file, MHI_PTR outMap3DInfo, const READ_INFO *inInfo)
{
	ReadData(outMap3DInfo->GridDataTbl, GRID_ENTRY_HEADER_SIZE*inInfo->GridNum, p_file);
}

static void ReadLineDataTbl(FSFile *p_file, MHI_PTR outMap3DInfo, const READ_INFO *inInfo)
{
	ReadData(outMap3DInfo->LineDataTbl, inInfo->TotalLineListSize, p_file);
}

static void ReadGridData(FSFile *p_file, MHI_PTR outMap3DInfo, const READ_INFO *inInfo)
{
	ReadData(outMap3DInfo->PolyIDList, inInfo->TotalPolyIDListSize, p_file);
}



////////////////////////////////////////////////////////////////////////////////////////
#if 0
MHI_PTR	AllocMapHeightInfo(void)
{
	MHI_PTR ptr;
	ptr = GFL_HEAP_AllocMemory(HEAPID_FIELD,sizeof(MAP_HEIGHT_INFO));
	ptr->VertexArray = NULL;
	ptr->NormalArray = NULL;
	ptr->PolygonData = NULL;
	ptr->SplitGridData = NULL;	
	ptr->GridDataTbl = NULL;
	ptr->LineDataTbl = NULL;
	ptr->PolyIDList = NULL;
	return ptr;
}
#endif

void CheckHeightData(const void *mem, MHI_PTR outMHI)
{

	void * hdata_adrs = (void*)((u8*)mem + sizeof(DP_HEIGHT_HEADER));
	READ_INFO read_info;
	DP_HEIGHT_HEADER * dhh = (DP_HEIGHT_HEADER *)mem;
	OS_Printf("address:%08x\n",mem);
	readerHeader(&mem, &read_info);
	//SetDataMem(&read_info, outMHI, (void **)&hdata_adrs);
	outMHI->SplitGridData = hdata_adrs; (u8*)hdata_adrs += sizeof(SPLIT_GRID_DATA);
	outMHI->VertexArray = hdata_adrs; (u8*)hdata_adrs += sizeof(VecFx32)*read_info.VtxNum;
	outMHI->NormalArray = hdata_adrs; (u8*)hdata_adrs += sizeof(VecFx32)*read_info.NrmNum;
	outMHI->PolygonData = hdata_adrs; (u8*)hdata_adrs += sizeof(POLYGON_DATA)*read_info.PolygonNum;
	outMHI->GridDataTbl = hdata_adrs; (u8*)hdata_adrs += GRID_ENTRY_HEADER_SIZE*read_info.GridNum;
	outMHI->LineDataTbl = hdata_adrs; (u8*)hdata_adrs += read_info.TotalLineListSize;
	outMHI->PolyIDList = hdata_adrs; (u8*)hdata_adrs += read_info.TotalPolyIDListSize;
	outMHI->DataValid = TRUE;		//データ有効
#if 0
	OS_Printf("id=%08x\n",dhh->id);
	OS_Printf("header_size=%08x\n",dhh->header_size);
	OS_Printf("vtx_size=%d\n",dhh->vtx_size);
	OS_Printf("vtx_num=%d\n",dhh->vtx_num);
	OS_Printf("nrm_size=%d\n",dhh->nrm_size);
	OS_Printf("nrm_num=%d\n",dhh->nrm_num);
	OS_Printf("poly_size=%d\n",dhh->poly_size);
	OS_Printf("poly_num=%d\n",dhh->poly_num);
	OS_Printf("grid_div_size=%d\n",dhh->grid_div_size);
	OS_Printf("grid_entry_size=%d\n",dhh->grid_entry_size);
	OS_Printf("grid_num=%d\n",dhh->grid_num);
	OS_Printf("line_entry_size=%d\n",dhh->line_entry_size);
	OS_Printf("grid_line_max=%d\n",dhh->grid_line_max);
	OS_Printf("dummy=%d\n",dhh->dummy);
	OS_Printf("line_num=%d\n",dhh->line_num);
	OS_Printf("grid_entry_total=%d\n",dhh->grid_entry_total);
#endif
#if 0
	SPLIT_GRID_DATA	*SplitGridData;
	POLYGON_DATA	*PolygonData;
	u16				*GridDataTbl;
	u16				*LineDataTbl;
	VecFx32			*VertexArray;
	VecFx32			*NormalArray;
	u16				*PolyIDList;
//	BOOL			LoadOK;
	BOOL			DataValid;		//データ有効有無
#endif
#if 0
	//頂点データ
	OS_Printf("VertexArray=%08x\n", outMHI->VertexArray );
	//法線データ
	OS_Printf("NormalArray=%08x\n", outMHI->NormalArray );
	//ポリゴンデータ
	OS_Printf("PolygonData=%08x\n", outMHI->PolygonData );
	//グリッド分割データ
	OS_Printf("SplitGridData=%08x\n", outMHI->SplitGridData );	
	//グリッドテーブルデータ
	OS_Printf("GridDataTbl=%08x\n", outMHI->GridDataTbl );
	//ラインテーブルデータ
	OS_Printf("LineDataTbl=%08x\n", outMHI->LineDataTbl );
	//グリッド実データ
	OS_Printf("PolyIDList=%08x\n", outMHI->PolyIDList );
	OS_Printf("DataValid=%08x\n", outMHI->DataValid );		//データ無効
#endif
}

void SetupHeightData(const char  *path, MHI_PTR outMapHeightInfo, u8 * inHeightMem)
{
	FSFile  file;
    BOOL    open_is_ok;
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, path);
    if(open_is_ok) {
        char    buf[256];
		READ_INFO read_info;
		//一時メモリ確保
		//read_info = GFL_HEAP_AllocMemory(HEAPID_FIELD,sizeof(READ_INFO));
		ReadHeaderData(&file,&read_info);
		SetDataMem(&read_info, outMapHeightInfo, (void **)&inHeightMem);
		
		//ReadGridSpData(&file,outMapHeightInfo);
		ReadData(outMapHeightInfo->SplitGridData, sizeof(SPLIT_GRID_DATA), &file);
		//ReadVtxData(&file,outMapHeightInfo, &read_info);
		ReadData(outMapHeightInfo->VertexArray, sizeof(VecFx32)*read_info.VtxNum, &file);
		//ReadNrmData(&file,outMapHeightInfo, &read_info);
		ReadData(outMapHeightInfo->NormalArray, sizeof(VecFx32)*read_info.NrmNum, &file);
		//ReadPolygonData(&file,outMapHeightInfo, &read_info);
		ReadData(outMapHeightInfo->PolygonData, sizeof(POLYGON_DATA)*read_info.PolygonNum, &file);
		//ReadGridDataTbl(&file,outMapHeightInfo, &read_info);
		ReadData(outMapHeightInfo->GridDataTbl, GRID_ENTRY_HEADER_SIZE*read_info.GridNum, &file);
		//ReadLineDataTbl(&file,outMapHeightInfo, &read_info);
		ReadData(outMapHeightInfo->LineDataTbl, read_info.TotalLineListSize, &file);
		//ReadGridData(&file,outMapHeightInfo, &read_info);
		ReadData(outMapHeightInfo->PolyIDList, read_info.TotalPolyIDListSize, &file);

		//一時メモリ解放
		//GFL_HEAP_FreeMemory(read_info);

		(void)FS_CloseFile( &file );

//		outMapHeightInfo->LoadOK = TRUE;
		outMapHeightInfo->DataValid = TRUE;		//データ有効
	}else{
		OS_Printf("FS_OpenFile(\"%s\") ... ERROR!\n", path);
	}
}



#if 0
void FreeMapHeightInfo(MHI_PTR outMap3DInfo)
{
	if (outMap3DInfo == NULL)
	{
		return;
	}
	GFL_HEAP_FreeMemory(outMap3DInfo);
	outMap3DInfo = NULL;
}

void SetInvalidHeightData(MHI_PTR outMap3DInfo)
{
	if (outMap3DInfo == NULL){
		return;
	}
	outMap3DInfo->DataValid = FALSE;		//データ無効
	//頂点データ
	outMap3DInfo->VertexArray = NULL;
	//法線データ
	outMap3DInfo->NormalArray = NULL;
	//ポリゴンデータ
	outMap3DInfo->PolygonData = NULL;
	//グリッド分割データ
	outMap3DInfo->SplitGridData = NULL;	
	//グリッドテーブルデータ
	outMap3DInfo->GridDataTbl = NULL;
	//ラインテーブルデータ
	outMap3DInfo->LineDataTbl = NULL;
	//グリッド実データ
	outMap3DInfo->PolyIDList = NULL;
}


void InitHeightData(MHI_PTR outData)
{
//	outData->LoadOK = FALSE;
	outData->DataValid = FALSE;
}

BOOL GetInvalidHeightDataFlg(MHI_CONST_PTR inMap3DInfo)
{
	return inMap3DInfo->DataValid;
}
#endif
void SetInvalidHeightData(MHI_PTR outMap3DInfo)
{
	if (outMap3DInfo == NULL){
		return;
	}
	outMap3DInfo->DataValid = FALSE;		//データ無効
	//頂点データ
	outMap3DInfo->VertexArray = NULL;
	//法線データ
	outMap3DInfo->NormalArray = NULL;
	//ポリゴンデータ
	outMap3DInfo->PolygonData = NULL;
	//グリッド分割データ
	outMap3DInfo->SplitGridData = NULL;	
	//グリッドテーブルデータ
	outMap3DInfo->GridDataTbl = NULL;
	//ラインテーブルデータ
	outMap3DInfo->LineDataTbl = NULL;
	//グリッド実データ
	outMap3DInfo->PolyIDList = NULL;
}

