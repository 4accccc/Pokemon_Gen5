//============================================================================================
/**
 * @file	func_pmcustom.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "map\dp3format.h"

#include "field_func_pmcustom_file.h"
//============================================================================================
/**
 *
 *
 *
 * @brief	ポケモンで使用されたデータを処理する関数郡
 *
 *
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	型宣言
 */
//------------------------------------------------------------------
//XZ頂点データ
typedef struct {
	fx32 x;
	fx32 z;
}XZ_VERTEX;

typedef struct {
	u16 vtx_idx0;
	u16 vtx_idx1;
	u16	nrm_idx;	//法線データ配列(正規化済み)へのインデックスNo
	u16 d_idx;		//Ｄ値配列へのインデックスNo
}POLYGON_DATA;

typedef struct {
	fx32	lineZ;
	u16		entryNum;
	u16		paramIndex;
}LINE_DATA;

typedef struct {
	XZ_VERTEX		*vertexArray;
	VecFx32			*normalArray;
	fx32			*valDArray;
	POLYGON_DATA	*polygonData;
	LINE_DATA		*lineDataTbl;
	u16				*polyIDList;

	int				lineNum;
}MAP_HEIGHT_INFO;

typedef struct {
	u32 signature;
	u16 vtxNum;
	u16 nrmNum;
	u16 valDNum;
	u16 polygonNum;
	u16 lineNum;
	u16 polyIDListNum;
}HEIGHT_DATA_HEADER;

typedef	struct	{
	int	id;			//データＩＤ

	VecFx32  global;	//グローバル座標
	VecFx32  rotate;	//回転
	VecFx32  scale;		//スケール

	int		dummy[2];	//
}MAP3D_OBJECT_ST;

//------------------------------------------------------------------
//ヘッダー情報
typedef struct { 
	u32		attrSize;
	u32		objSize;
	u32		mapSize;
	u32		heightSize;
}MAP_FILE_HEADER;

//サウンドヘッダー情報
typedef struct {
	u16		signature;
	u16		size;
}SOUND_DATA_HEADER;

//データ取得用情報
typedef struct { 
	u32				attrAdrs;	//アトリビュート取得用
	MAP_HEIGHT_INFO heightInfo;	//高さ取得用

}MAP_DATA_INFO;

#define GRID_SIZE (16)
#define GRID_DATASIZE (sizeof(u16))
#define	GRIDBLOCKW	(32)
//============================================================================================
/**
 *
 *
 *
 * @brief	マップデータ読み込み
 *				※シーケンス0番はIDLING 1番はスタートＩＤＸで固定
 *
 *
 */
//============================================================================================
enum {
	FILE_LOAD_START = FLD_G3D_MAP_LOAD_START,
	FILE_LOAD,
	FILE_HEADER_SET,
	RND_CREATE,
};

BOOL FieldLoadMapData_PMcustomFile( FLD_G3D_MAP* g3Dmap, void * exWork )
{
	FLD_G3D_MAP_LOAD_STATUS* ldst;

	FLD_G3D_MAP_GetLoadStatusPointer( g3Dmap, &ldst );

	switch( ldst->seq ){

	case FILE_LOAD_START:
		FLD_G3D_MAP_ResetLoadStatus(g3Dmap);

		//メモリ先頭にはデータ取得用情報を配置するため、読み込みポインタをずらす
		ldst->mOffs += sizeof(MAP_DATA_INFO);

		//モデルデータロード開始
		{
			u32		datID;
			FLD_G3D_MAP_GetLoadDatID( g3Dmap, &datID );
			FLD_G3D_MAP_StartFileLoad( g3Dmap, datID );
		}
		ldst->seq = FILE_LOAD;
		break;

	case FILE_LOAD:
		if( FLD_G3D_MAP_ContinueFileLoad(g3Dmap) == FALSE ){
			ldst->seq = FILE_HEADER_SET;
		}
		break;

	case FILE_HEADER_SET:
		{
			void*				mem;
			MAP_FILE_HEADER*	header;
			MAP_DATA_INFO*		mapdataInfo;
			SOUND_DATA_HEADER*	soundHeader;
			HEIGHT_DATA_HEADER*	heightHeader;
			u32					dataOffset = 0;

			FLD_G3D_MAP_GetLoadMemoryPointer( g3Dmap, &mem );

			//データ取得用情報設定
			mapdataInfo = (MAP_DATA_INFO*)mem;
			dataOffset += sizeof(MAP_DATA_INFO);
			
			//ファイル内容ヘッダー設定
			header = (MAP_FILE_HEADER*)((u32)mem + dataOffset);
			dataOffset += sizeof(MAP_FILE_HEADER);

			//環境音データ設定
			soundHeader = (SOUND_DATA_HEADER*)((u32)mem + dataOffset);
			dataOffset += (sizeof(SOUND_DATA_HEADER) + soundHeader->size);

			//アトリビュートリソース設定
			mapdataInfo->attrAdrs = (u32)mem + dataOffset;
			dataOffset += header->attrSize;

			//配置オブジェクト設定
			{
				MAP3D_OBJECT_ST* objStatus = (MAP3D_OBJECT_ST*)((u32)mem + dataOffset);
				FLD_G3D_MAP_GLOBALOBJ_ST status;
				int i, count = header->objSize/sizeof(MAP3D_OBJECT_ST);
				u32 id;

				for( i=0; i<count; i++ ){
					if( FLD_G3D_MAP_GetGlobalObjectID( g3Dmap, objStatus[i].id, &id ) == TRUE ){
						status.id = id;
						status.trans = objStatus[i].global;
						status.rotate = 0;
						FLD_G3D_MAP_ResistGlobalObj( g3Dmap, &status, i );
					} else {
						OS_Printf("cannot exchange ID = %x\n", objStatus[i].id );
					}
				}
			}
			dataOffset += header->objSize;
			//モデルリソース設定
			FLD_G3D_MAP_CreateResourceMdl(g3Dmap, (void*)((u32)mem + dataOffset));
			dataOffset += header->mapSize;

			//高さリソース設定
			heightHeader = (HEIGHT_DATA_HEADER*)((u32)mem + dataOffset);
			{
				//高さデータ取得用情報設定
				MAP_HEIGHT_INFO* heightInfo = &mapdataInfo->heightInfo;
				u32 hdataOffset = 0;

				hdataOffset += sizeof(HEIGHT_DATA_HEADER);

				//ライン総数
				heightInfo->lineNum = heightHeader->lineNum;
				//頂点データポインタ算出
				heightInfo->vertexArray = (XZ_VERTEX*)((u32)heightHeader + hdataOffset);
				hdataOffset += sizeof(XZ_VERTEX) * heightHeader->vtxNum;
				//法線データポインタ算出
				heightInfo->normalArray = (VecFx32*)((u32)heightHeader + hdataOffset);
				hdataOffset += sizeof(VecFx32) * heightHeader->nrmNum;
				//Ｄ値データポインタ算出
				heightInfo->valDArray = (fx32*)((u32)heightHeader + hdataOffset);
				hdataOffset += sizeof(fx32) * heightHeader->valDNum;
				//ポリゴンデータポインタ算出
				heightInfo->polygonData = (POLYGON_DATA*)((u32)heightHeader + hdataOffset);
				hdataOffset += sizeof(POLYGON_DATA) * heightHeader->polygonNum;
				//ラインテーブルデータポインタ算出
				heightInfo->lineDataTbl = (LINE_DATA*)((u32)heightHeader + hdataOffset);
				hdataOffset += sizeof(LINE_DATA) * heightHeader->lineNum;
				//ライン内ポリゴンＩＤエントリデータポインタ算出
				heightInfo->polyIDList = (u16*)((u32)heightHeader + hdataOffset);
				hdataOffset += sizeof(u16) * heightHeader->polyIDListNum;
			}
			ldst->seq = RND_CREATE;
		}
		break;

	case RND_CREATE:
		//レンダー作成
		FLD_G3D_MAP_MakeRenderObj( g3Dmap );

		ldst->mdlLoaded = TRUE;
		ldst->attrLoaded = TRUE;

		ldst->seq = FLD_G3D_MAP_LOAD_IDLING;
		return FALSE;
		break;
	}
	return TRUE;
}

//============================================================================================
/**
 *
 *
 *
 * @brief	３Ｄマップ情報取得
 *
 *
 *
 */
static BOOL CheckRectIO( const XZ_VERTEX* vtx0, const XZ_VERTEX *vtx1, const VecFx32* pos );
static BOOL	BinSearch( const LINE_DATA *list, const u16 size, const fx32 valZ, u16* idx );
//============================================================================================
void FieldGetAttr_PMcustomFile( FLD_G3D_MAP_ATTRINFO* attrInfo, const void* mapdata,
					const VecFx32* posInBlock, const fx32 map_width, const fx32 map_height )
{
	//データ取得用情報設定
	MAP_DATA_INFO*		mapdataInfo = (MAP_DATA_INFO*)mapdata;
	MAP_HEIGHT_INFO*	heightInfo = &mapdataInfo->heightInfo;
	XZ_VERTEX			*rectVtx0, *rectVtx1;
	VecFx32				vecN;
	fx32				valD, by;
	POLYGON_DATA*		polygon;
	u32					offsIdx;
	u16					lineIdx, polIdx, polNum;
	int					i;

	attrInfo->mapAttrCount = 0;
	
	//ラインデータを2分探索して、対象ポリゴンが存在するラインを検出
	if( BinSearch(heightInfo->lineDataTbl, heightInfo->lineNum, posInBlock->z, &lineIdx) == FALSE ){
		return;
	}
	polNum = heightInfo->lineDataTbl[lineIdx].entryNum;
	offsIdx = heightInfo->lineDataTbl[lineIdx].paramIndex;

	//高さ取得用グリッド内ラインデータに登録されているデータ分ループして、対象ポリゴンを検出
	for( i=0; i<polNum; i++ ){
		polIdx = heightInfo->polyIDList[offsIdx+i];
		polygon = &heightInfo->polygonData[polIdx];

		//頂点データ取得
		rectVtx0 = &heightInfo->vertexArray[polygon->vtx_idx0];
		rectVtx1 = &heightInfo->vertexArray[polygon->vtx_idx1];

		//四角形の内外判定
		if( CheckRectIO( rectVtx0, rectVtx1, posInBlock ) == TRUE ){
			//法線取得
			vecN = heightInfo->normalArray[polygon->nrm_idx];

			//平面の方程式に必要なデータ（原点を通る平面からのオフセット）を取得
			valD = heightInfo->valDArray[polygon->d_idx];

			//平面の方程式より高さ取得
			by = -( FX_Mul(vecN.x, posInBlock->x) + FX_Mul(vecN.z, posInBlock->z) + valD );
			attrInfo->mapAttr[attrInfo->mapAttrCount].height = FX_Div(by, vecN.y);
			VEC_Fx16Set( &attrInfo->mapAttr[attrInfo->mapAttrCount].vecN, vecN.x, vecN.y, vecN.z );
			
			attrInfo->mapAttrCount++;
			if (attrInfo->mapAttrCount >= FLD_G3D_MAP_ATTR_GETMAX){
				return;	//取得オーバーフロー
			}
		}
	}
	{
		int bx,bz;
		int attrmax;
		int x,z,num;
		const u16 *buf;
		const MAP_FILE_HEADER *header;
		
		buf = (u16*)mapdataInfo->attrAdrs;
		
		header = (MAP_FILE_HEADER*)((u32)mapdata + sizeof(MAP_DATA_INFO));
		attrmax = header->attrSize / GRID_DATASIZE;	//最大要素数
		//TAMADA_Printf("attrAdrs = %08x\n",mapdataInfo->attrAdrs);
		//TAMADA_Printf("attrSize = %04x\n",header->attrSize);
		//TAMADA_Printf("objSize = %04x\n",header->objSize);
		//TAMADA_Printf("mapSize = %04x\n",header->mapSize);
		//TAMADA_Printf("heightSize = %04x\n",header->heightSize);
		
		//ブロック中心の座標を左上からの座標に補正
		x = (posInBlock->x + map_width/2);
		z = (posInBlock->z + map_width/2);
		//グリッド単位に補正
		x /= FX32_ONE * GRID_SIZE;
		z /= FX32_ONE * GRID_SIZE;
		num = x + (z * GRIDBLOCKW);
		
		if( num >= attrmax ){
			//TAMADA_Printf( "GetAttrData GX=%d,GZ=%d, ERROR!!\n", x, z );
			attrInfo->mapAttr[0].attr = 0xffff;
		} else {
			//TAMADA_Printf( "GetAttrData GX=%d,GZ=%d, Attr=%xH\n", x, z,  buf[num] );
			attrInfo->mapAttr[0].attr = (buf[num] &0x8000) >> 15;
		}
	}
}

//------------------------------------------------------------------
/**
 *	XZ面上の四角内外判定
*/
//------------------------------------------------------------------
static BOOL CheckRectIO( const XZ_VERTEX* vtx0, const XZ_VERTEX *vtx1, const VecFx32* pos )
{
	fx32 x0, x1, z0, z1;

	if( vtx0->x <= vtx1->x ){
		x0 = vtx0->x;
		x1 = vtx1->x;
	}else{
		x0 = vtx1->x;
		x1 = vtx0->x;
	}
	if( vtx0->z <= vtx1->z ){
		z0 = vtx0->z;
		z1 = vtx1->z;
	}else{
		z0 = vtx1->z;
		z1 = vtx0->z;
	}
	if ( (x0 <= pos->x)&&(pos->x <= x1)&&(z0 <= pos->z)&&(pos->z <= z1) ){
		return TRUE;
	}
	return FALSE;
}

//------------------------------------------------------------------
/**
 *　Zソートされているデータを2分探索
 *	@param	list		データリスト		
 *	@param	size		データサイズ
 *	@param	valZ		Z値
 *	@param	idx			データインデックス
 *	
 *	@retval	BOOL	TRUE:データあり　FALSE:データ無し
*/
//------------------------------------------------------------------
static BOOL	BinSearch( const LINE_DATA *list, const u16 size, const fx32 valZ, u16* idx )
{
	int min,max; 
	u32 data_idx;
	fx32 lineZ;

	if( size == 0 ){
		return FALSE;//データなしなので、探索終了
	}else if ( size == 1 ){
		*idx = 0;//探索終了
		return TRUE;
	}
	min = 0;
	max = size - 1;
	data_idx = max/2;
	
	do{
		lineZ = list[data_idx].lineZ;

		if( lineZ > valZ ){
			if( max-1 > min ){
				max = data_idx;		//探索継続
				data_idx = (min + max)/2;
			}else{			
				*idx = data_idx;	//探索終了
				return TRUE;
			}
		} else {				
			if( min+1 < max ){
				min = data_idx;		//探索継続
				data_idx = (min + max)/2;
			}else{
				*idx = data_idx+1;	//探索終了
				return TRUE;
			}
		}
	}while(1);

	return FALSE;
}







#if 0
#define MAP_READ_OBJ_3D_MAX (32)	//1ブロック32個のＯＢＪ
//==============================================================================
/**
 * 3DOBJロード
 *
 * @param	inMapResource		マップリソースポインタ
 * @param	inFileName			ファイル名
 * @param	outMap3DObjList		3DOBJリストへのポインタ
 * @param	field_3d_anime_ptr	3Dアニメポインタ
 *
 * @return	none
 */
//==============================================================================
void M3DO_LoadArc3DObjData(	ARCHANDLE *ioHandle,
							const int inDataSize,
							const MAP_RESOURCE_PTR inMapResource,
							M3DOL_PTR outMap3DObjList,
							FLD_3D_ANM_MNG_PTR field_3d_anime_ptr)
{
	MAP3D_OBJECT_HEADER * obj_dat = NULL;
	u32	obj_max;
	int i;
	//アーカイブデータ読み込み
	if (inDataSize != 0){
		obj_dat = sys_AllocMemoryLo(HEAPID_FIELD, inDataSize);
		ArchiveDataLoadByHandleContinue( ioHandle, inDataSize, obj_dat );
		obj_max = inDataSize / sizeof(MAP3D_OBJECT_HEADER);
	}else{
		obj_max = 0;
	}

	for(i=0;i<MAP_READ_OBJ_3D_MAX;i++){
		M3DO_PTR ptr;
		ptr = &(outMap3DObjList->Map3DObjectData[i]);
		if(i < obj_max){
			ptr->id		    = obj_dat[i].id;		//ＩＤ
			ptr->valid		= TRUE;					//データ有効
			ptr->RotateFlg  = FALSE;				//回転無効
			ptr->global	    = obj_dat[i].global;	//座標
			ptr->rotate	    = obj_dat[i].rotate;	//回転
			ptr->scale	    = obj_dat[i].scale;		//スケール

			// 読み込んだテクスチャリソースをセット
			SetMap3DModel(
					ptr->id,
					inMapResource,
					&(ptr->objectdata),
					&(ptr->objectmodel) );
			if (CheckResourceEntry(inMapResource, ptr->id) == FALSE){
				OS_Printf("ダミー表示のため、配置モデルIDを書き換えます%d→0\n",ptr->id);
				ptr->id = 0;
			}
			
#ifdef FOG_MDL_SET_CHECK
			NNS_G3dMdlSetMdlFogEnableFlagAll(ptr->objectmodel, TRUE);
#endif
			F3DA_SetFld3DAnimeEasy(	ptr->id, &(ptr->objectdata), field_3d_anime_ptr);
		}else{
			VecFx32 init_data = {0,0,0};

			ptr->id		    = 0;		// =OFF
			ptr->valid		= FALSE;	//データ無効
			ptr->RotateFlg	= FALSE;
			ptr->global     = init_data;
			ptr->rotate     = init_data;
			ptr->scale	    = init_data;
		}
	}
	if( obj_dat != NULL ){
		sys_FreeMemoryEz( obj_dat );
	}

}
#endif


#if 0
//==============================================================================
/**
 * マップアトリビュート取得
 * @param	gridBlockW	グリッド単位でのブロック横幅
 * @return	u16	マップアトリビュート
 */
//==============================================================================
u16 FieldGetAttrData_PMcustomFile(
	FLD_G3D_MAP *g3Dmap, const VecFx32 *pos, int gridBlockW )
{
	void *mem;
	int bx,bz;
	int attrmax,linemax;
	int x,z,num;
	const u16 *buf;
	const MAP_FILE_HEADER *header;
	const MAP_DATA_INFO *mapdataInfo;
	
	FLD_G3D_MAP_GetLoadMemoryPointer( g3Dmap, &mem );
	mapdataInfo = (MAP_DATA_INFO*)mem;
	buf = (u16*)mapdataInfo->attrAdrs;
	
	header = (MAP_FILE_HEADER*)((u32)mem + sizeof(MAP_FILE_HEADER));
	attrmax = header->attrSize / GRID_DATASIZE;	//最大要素数
	
	x = (pos->x / GRID_SIZE) / FX32_ONE;				//整数グリッド単位
	z = (pos->z / GRID_SIZE) / FX32_ONE;
	bx = (x / gridBlockW) * gridBlockW;			//先頭ブロックNo
	bz = (z / gridBlockW) * gridBlockW;
	x = x - bx;									//相対位置
	z = z - bz;
	num = x + (z * gridBlockW);
	
	if( num >= attrmax ){
#if 0
		OS_Printf( "GetAttrData GX=%d,GZ=%d, gridBlockW = %d, ERROR!!\n",
			x, z, gridBlockW );
#endif
		return( 0xffff );
	}
#if 0
	OS_Printf( "GetAttrData GX=%d,GZ=%d, gridBlockW = %d, Attr=%xH\n",
			x, z, gridBlockW, buf[num] );
#endif
	return( buf[num] );
}
#endif
