//============================================================================================
/**
 * @file	func_pmcustom.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include "gflib.h"

#include "..\g3d_map.h"
#include "map\dp3format.h"

#include "func_pmcustom_file.h"
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
typedef struct XZ_VERTEX_tag
{
	fx32 X;
	fx32 Z;
}XZ_VERTEX;

typedef struct POLYGON_DATA_tag{
	u16 vtx_idx0;
	u16 vtx_idx1;
	u16	nrm_idx;	//法線データ配列(正規化済み)へのインデックスNo
	u16 d_idx;		//Ｄ値配列へのインデックスNo
}POLYGON_DATA;

typedef struct HEIGHT_ARRAY_tag
{
	fx32 Height;
	int Prev;
	int Next;
}HEIGHT_ARRAY;

typedef struct LINE_DATA_tag
{
	fx32 LineZ;
	u16 EntryNum;
	u16 ParamIndex;
}LINE_DATA;

typedef struct MAP_HEIGHT_INFO_tag{
	XZ_VERTEX		*VertexArray;
	VecFx32			*NormalArray;
	fx32			*DvalArray;
	POLYGON_DATA	*PolygonData;
	LINE_DATA		*LineDataTbl;
	u16				*PolyIDList;

	int				LineNum;
}MAP_HEIGHT_INFO;

typedef struct {
	u32 signature;
	u16 VtxNum;
	u16 NrmNum;
	u16 DNum;
	u16 PolygonNum;
	u16 LineNum;
	u16 PolyIDListNum;
}HEIGHT_DATA_HEADER;

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
	MAP_HEIGHT_INFO heightInfo;	//高さ取得用

}MAP_DATA_INFO;

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
	FILE_LOAD_START = LOAD_START,
	FILE_LOAD,
	FILE_HEADER_SET,
	RND_CREATE,
};

BOOL LoadMapData_PMcustomFile( GFL_G3D_MAP* g3Dmap )
{
	GFL_G3D_MAP_LOAD_STATUS* ldst;

	GFL_G3D_MAP_GetLoadStatusPointer( g3Dmap, &ldst );

	switch( ldst->seq ){

	case FILE_LOAD_START:
		GFL_G3D_MAP_ResetLoadStatus(g3Dmap);

		//メモリ先頭にはデータ取得用情報を配置するため、読み込みポインタをずらす
		ldst->mOffs += sizeof(MAP_DATA_INFO);

		//モデルデータロード開始
		{
			u32		datID;
			GFL_G3D_MAP_GetLoadDatID( g3Dmap, &datID );
			GFL_G3D_MAP_StartFileLoad( g3Dmap, datID );
		}
		ldst->seq = FILE_LOAD;
		break;

	case FILE_LOAD:
		if( GFL_G3D_MAP_ContinueFileLoad(g3Dmap) == FALSE ){
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

			GFL_G3D_MAP_GetLoadMemoryPointer( g3Dmap, &mem );

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
			heightHeader = (HEIGHT_DATA_HEADER*)((u32)mem + dataOffset);
			{
				//アトリビュートデータ取得用情報設定
				MAP_HEIGHT_INFO* heightInfo = &mapdataInfo->heightInfo;
				u32 hdataOffset = 0;

				hdataOffset += sizeof(HEIGHT_DATA_HEADER);

				//ライン総数
				heightInfo->LineNum = heightHeader->LineNum;
				//頂点データポインタ算出
				heightInfo->VertexArray = (XZ_VERTEX*)((u32)heightHeader + hdataOffset);
				hdataOffset += sizeof(XZ_VERTEX) * heightHeader->VtxNum;
				//法線データポインタ算出
				heightInfo->NormalArray = (VecFx32*)((u32)heightHeader + hdataOffset);
				hdataOffset += sizeof(VecFx32) * heightHeader->NrmNum;
				//Ｄ値データポインタ算出
				heightInfo->DvalArray = (fx32*)((u32)heightHeader + hdataOffset);
				hdataOffset += sizeof(fx32) * heightHeader->DNum;
				//ポリゴンデータポインタ算出
				heightInfo->PolygonData = (POLYGON_DATA*)((u32)heightHeader + hdataOffset);
				hdataOffset += sizeof(POLYGON_DATA) * heightHeader->PolygonNum;
				//ラインテーブルデータポインタ算出
				heightInfo->LineDataTbl = (LINE_DATA*)((u32)heightHeader + hdataOffset);
				hdataOffset += sizeof(LINE_DATA) * heightHeader->LineNum;
				//ライン内ポリゴンＩＤエントリデータポインタ算出
				heightInfo->PolyIDList = (u16*)((u32)heightHeader + hdataOffset);
				hdataOffset += sizeof(u16) * heightHeader->PolyIDListNum;
			}
			dataOffset += header->attrSize;

			//配置オブジェクト設定
			dataOffset += header->objSize;
			//モデルリソース設定
			GFL_G3D_MAP_CreateResourceMdl(g3Dmap, (void*)((u32)mem + dataOffset));
			dataOffset += header->mapSize;

			ldst->seq = RND_CREATE;
		}
		break;

	case RND_CREATE:
		//レンダー作成
		GFL_G3D_MAP_MakeRenderObj( g3Dmap );

		ldst->mdlLoaded = TRUE;
		ldst->attrLoaded = TRUE;

		ldst->seq = LOAD_IDLING;
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
BOOL GetHeightForBlock( const fx32 inX, const fx32 inZ, MAP_HEIGHT_INFO* inMap3DInfo );
//============================================================================================
void GetAttr_PMcustomFile( GFL_G3D_MAP_ATTRINFO* attrInfo, const void* mapodata,
					const VecFx32* posInBlock, const fx32 map_width, const fx32 map_height )
{
	VEC_Fx16Set( &attrInfo->mapAttr[0].vecN, 0, FX16_ONE, 0 );
	attrInfo->mapAttr[0].attr = 0;
	attrInfo->mapAttr[0].height = 0;

	attrInfo->mapAttrCount = 1;
}











#define LINE_ONE_DATA	(5)		//2バイトデータが5つで10バイト
#define Z_VAL_OFFSET_L(data_idx)	( data_idx*LINE_ONE_DATA+1 )
#define Z_VAL_OFFSET_H(data_idx)	( data_idx*LINE_ONE_DATA+2 )

#define LINE_Z_VAL(list,idx)	( (list[Z_VAL_OFFSET_H(data_idx)]<<16) | list[Z_VAL_OFFSET_L(data_idx)] )

//------------------------------------------------------------------
/**
 *	四角内外判定
*/
//------------------------------------------------------------------
static BOOL CheckRectIO
	( const XZ_VERTEX *inVtx1, const XZ_VERTEX *inVtx2, const XZ_VERTEX * inTarget )
{
	const fx32 *small_x, *big_x,*small_z,*big_z;
	if (inVtx1->X <= inVtx2->X){
		small_x = &inVtx1->X;
		big_x = &inVtx2->X;
	}else{
		small_x = &inVtx2->X;
		big_x = &inVtx1->X;
	}

	if (inVtx1->Z <= inVtx2->Z){
		small_z = &inVtx1->Z;
		big_z = &inVtx2->Z;
	}else{
		small_z = &inVtx2->Z;
		big_z = &inVtx1->Z;
	}
		
	if ( ( (*small_x <= inTarget->X)&&(inTarget->X <= *big_x) )&&
			( (*small_z <= inTarget->Z)&&(inTarget->Z <= *big_z) ) ){
		return TRUE;
	}
	return FALSE;
}

//------------------------------------------------------------------
/**
 *	ポリゴンインデックスからポリゴン頂点データを取得
 *	@param	inMAp3DInfo		マップ高さ情報		
 *	@param	inIdx			ポリゴンデータインデックス
 *	@param	outVertex		頂点データ格納先ポインタ
 *	
 *	@retval	none
*/
//------------------------------------------------------------------
static void GetPolygonVertex(MAP_HEIGHT_INFO* inMap3DInfo, u16 inIdx, XZ_VERTEX *outVertex)
{
	outVertex[0] = inMap3DInfo->VertexArray[inMap3DInfo->PolygonData[inIdx].vtx_idx0];
	outVertex[1] = inMap3DInfo->VertexArray[inMap3DInfo->PolygonData[inIdx].vtx_idx1];
}

//------------------------------------------------------------------
/**
 *	ポリゴンインデックスからポリゴンの法線を取得
 *	@param	inMAp3DInfo		マップ高さ情報		
 *	@param	inIdx			ポリゴンデータインデックス
 *	@param	outVertex		法線データ格納先ポインタ
 *	
 *	@retval	none
*/
//------------------------------------------------------------------
static void GetPolygonNrm(MAP_HEIGHT_INFO* inMap3DInfo, u16 inIdx, VecFx32 *outVertex)
{
	*outVertex = inMap3DInfo->NormalArray[inMap3DInfo->PolygonData[inIdx].nrm_idx];
}

//------------------------------------------------------------------
/**
 *	ポリゴンインデックスからD値を取得
 *	@param	inMAp3DInfo		マップ高さ情報		
 *	@param	inIdx			ポリゴンデータインデックス
 *	@param	outD		Dデータ格納先ポインタ
 *	
 *	@retval	none
*/
//------------------------------------------------------------------
static void GetPolygonD(MAP_HEIGHT_INFO* inMap3DInfo, u16 inIdx, fx32 *outD)
{
	*outD = inMap3DInfo->DvalArray[inMap3DInfo->PolygonData[inIdx].d_idx];
}

//------------------------------------------------------------------
//------------------------------------------------------------------
/**
ソート用配列の初期化
*/
static void InitArray(HEIGHT_ARRAY *outArray)
{
	int i;
	for(i=0;i<GFL_G3D_MAP_ATTR_GETMAX;i++){
		outArray[i].Height = 0;
		outArray[i].Prev = -1;
		outArray[i].Next = -1;
	}
}

/**
 高さ配列データのソート(昇順)
*/
static void SortArray(int inCount, HEIGHT_ARRAY *ioArray)
{
	int i;
	int prev;
	for(i=0;i<=inCount-1;i++){
		if (ioArray[i].Height<ioArray[inCount].Height){
			prev = ioArray[i].Prev;
			ioArray[i].Prev = inCount;
			ioArray[inCount].Next = i;
			ioArray[inCount].Prev = prev;
			if (prev>=0){
				ioArray[prev].Next = inCount;
			}
			//FirstIdx = inCount;
			return;
		}else{
			ioArray[inCount].Prev = i;
			ioArray[i].Next = inCount;
		}
	}
	if (inCount>0){
		ioArray[inCount].Next = ioArray[i].Next;
		ioArray[i].Next = inCount;
		ioArray[inCount].Prev = i;
	}
	return;
}

//------------------------------------------------------------------
/**
 *　Zソートされているデータを2分探索
 *	@param	inDataList		データリスト		
 *	@param	inDataSize		データサイズ
 *	@param	inZ_Val			Z値
 *	@param	outIndex		データインデックス
 *	
 *	@retval	BOOL	TRUE:データあり　FALSE:データ無し
*/
//------------------------------------------------------------------
static BOOL	BinSearch
		( const LINE_DATA *inDataList, const u16 inDataSize, const fx32 inZ_Val, u16 *outIndex)
{
	int min,max; 
	u32 data_idx;
	fx32 z_val;
	if (inDataSize == 0){
		OS_Printf("Zソートデータがありません\n");
		return FALSE;//データなしなので、探索終了
	}else if (inDataSize == 1){
		*outIndex = 0;//探索終了
		return TRUE;
	}

	min = 0;
	max = inDataSize-1;
	data_idx = max/2;
	
	do{
///OS_Printf("min_max:%d,%d\n",min,max);
		z_val = inDataList[data_idx].LineZ;
///OS_Printf("%d z_val:%x\n",data_idx,z_val);
		if (z_val>inZ_Val){	//探索継続
			if (max-1 > min){
				max = data_idx;
				data_idx = (min+max)/2;
			}else{				//探索終了
				*outIndex = data_idx;
				return TRUE;
			}
		}else{					//探索継続
			if (min+1 < max){
				min = data_idx;
				data_idx = (min+max)/2;
			}else{				//探索終了
				*outIndex = data_idx+1;
				return TRUE;
			}
		}
	}while(1);

	return FALSE;
}

//------------------------------------------------------------------
/**
 *	高さ取得
 *
 *	@param	mode			高さ取得モード
 *	@param	inX				求めたい高さのX座標
 *	@param	inZ				求めたい高さのZ座標
 *	@param	inMap3DInfo		高さ情報
 *	
 *	@retval	BOOL			TRUE:高さを取得できた	FALSE:高さを取得できなかった
*/
//------------------------------------------------------------------
BOOL GetHeightForBlock( const fx32 inX, const fx32 inZ, MAP_HEIGHT_INFO* inMap3DInfo )
{
	XZ_VERTEX vertex[2];
	XZ_VERTEX target;
	VecFx32 nrm;
	BOOL result;
	u16 i,pol_index;
	fx32 d,y;
	int pol_count;
	u32 line_num,tbl_idx,tbl_x_idx,tbl_z_idx;
	HEIGHT_ARRAY height_array[GFL_G3D_MAP_ATTR_GETMAX];
	fx32 tbl_x,tbl_z;

	u32 offset_idx;

	u16 line_data_idx;
	u16 line_index;
	u16 polygon_num;

	const LINE_DATA *line_list;

	result = FALSE;
	//ZX平面に射影
	target.X = inX;
	target.Z = inZ;
	pol_count = 0;
	InitArray(height_array);
	
	line_num = inMap3DInfo->LineNum;
	
	line_list = inMap3DInfo->LineDataTbl;
	//ラインデータを2分探索して、自分が立っているポリゴンが存在するラインを検出
	if ( BinSearch(line_list,line_num,target.Z,&line_index)==FALSE){
		return FALSE;
	}

	polygon_num = line_list[line_index].EntryNum;
	offset_idx = line_list[line_index].ParamIndex;

	//高さ取得用グリッド内ラインデータに登録されているデータ分ループして、
	//自分が立っているポリゴンを検出
	for(i=0;i<polygon_num;i++){
		pol_index = inMap3DInfo->PolyIDList[offset_idx+i];

		//頂点データ取得
		GetPolygonVertex(inMap3DInfo,pol_index, vertex);
		//四角形の内外判定
		result = CheckRectIO( &vertex[0], &vertex[1], &target);
		if (result == TRUE){//四角形内
			//法線取得
			GetPolygonNrm(inMap3DInfo,pol_index, &nrm);
			//平面の方程式に必要なデータ（原点を通る平面からのオフセット）を取得
			GetPolygonD(inMap3DInfo,pol_index, &d);
			//平面の方程式より高さ取得
			y = -(FX_Mul(nrm.x, target.X)+FX_Mul(nrm.z, target.Z)+d);
			y = FX_Div(y, nrm.y);
			height_array[pol_count].Height = y;
			//配列のソート（現行では機能してません。対応予定）<<未使用にしました。20060801
			///SortArray(pol_count,height_array);
			
			pol_count++;
			if (pol_count >= GFL_G3D_MAP_ATTR_GETMAX){
				break;
			}
		}
	}
	return FALSE;
}


