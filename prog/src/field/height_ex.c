//============================================================================================
/**
 * @file	height_ex.c
 * @brief	拡張高さデータ
 * @author	Nozomu Saito
 * @date	2005.12.05
 */
//============================================================================================
#include <gflib.h>
#include "height_ex.h"

typedef struct EX_HEIGHT_DATA_tag
{
	int StartGridX;
	int StartGridZ;
	int SizeGridX;
	int SizeGridZ;
	fx32 Height;
	BOOL Valid;
}EX_HEIGHT_DATA;

typedef struct EX_HEIGHT_LIST_tag
{
	int Num;
	EX_HEIGHT_DATA *ExHeightData;
}EX_HEIGHT_LIST;

static BOOL HitCheckExHeightFromData(	const int inGridX,
										const int inGridZ,
										const EX_HEIGHT_DATA *inExHeightData );

//--------------------------------------------------------------------------------------------
/**
 * 拡張高さデータのヒットチェック
 *
 * @param	inGridX			チェックするグリッドX座標
 * @param	inGridZ			チェックするグリッドZ座標
 * @param	inExHeightData	拡張高さデータへのポインタ
 *
 * @return	BOOL	TRUE:ヒット	FALSE:ノーヒット
 */
//--------------------------------------------------------------------------------------------
static BOOL HitCheckExHeightFromData(	const int inGridX,
										const int inGridZ,
										const EX_HEIGHT_DATA *inExHeightData )
{
	//データが無効ならヒットしなかったとみなす
	if (inExHeightData->Valid == FALSE){
		return FALSE;
	}
#if 0	
#ifdef DEBUG_ONLY_FOR_saitou	
OS_Printf("local_printt search%d,%d\n",inGridX,inGridZ);	
#endif
#endif
	{
		int st_x = inExHeightData->StartGridX;
		int st_z = inExHeightData->StartGridZ;
		int end_x = st_x+inExHeightData->SizeGridX-1;
		int end_z = st_z+inExHeightData->SizeGridZ-1;

		if ( (st_x<=inGridX)&&(inGridX<=end_x)&&
				(st_z<=inGridZ)&&(inGridZ<=end_z) ){
#if 0			
#ifdef DEBUG_ONLY_FOR_saitou			
OS_Printf("local_print HIT!!\n");
#endif
#endif
			return TRUE;
		}
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 拡張高さデータのメモリアロケート
 *
 * @param	inNum		確保するデータ個数
 * @param	inHeapID	確保するヒープID
 *
 * @return	EHL_PTR	確保した領域の先頭ポインタ
 */
//--------------------------------------------------------------------------------------------
EHL_PTR EXH_AllocHeightList(const u8 inNum, const u8 inHeapID)
{
	u8 i;
	EHL_PTR ehl_p;
	ehl_p = GFL_HEAP_AllocMemory( inHeapID, sizeof(EX_HEIGHT_LIST) );
	ehl_p->ExHeightData = GFL_HEAP_AllocMemory( inHeapID, sizeof(EX_HEIGHT_DATA)*inNum );
	ehl_p->Num = inNum;
	//データを無効化
	for(i=0;i<inNum;i++){
		ehl_p->ExHeightData[i].Valid = FALSE;
	}
	
	return ehl_p;
}

//--------------------------------------------------------------------------------------------
/**
 * 拡張高さデータのセットアップ
 *
 * @param	inIndex		取得する高さデータのインデックス
 * @param	inStX			領域開始グリッドX座標
 * @param	instZ			領域開始グリッドZ座標
 * @param	inSizX			領域グリッド横幅
 * @param	inSizZ			領域グリッド縦幅
 * @param	inHeight		領域内高さ
 * @param	outExHeightList	拡張高さデータへのポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void EXH_SetUpHeightData(	const u8 inIndex,
							const int inStX,
							const int inStZ,
							const int inSizX,
							const int inSizZ,
							const fx32 inHeight,
							EHL_PTR outExHeightList )
{
  if ( inIndex >= outExHeightList->Num )
  {
    GF_ASSERT(0);
    return;
  }

  if ( outExHeightList->ExHeightData[inIndex].Valid )
  {
    GF_ASSERT(0);
    return;
  }

	outExHeightList->ExHeightData[inIndex].StartGridX = inStX;
	outExHeightList->ExHeightData[inIndex].StartGridZ = inStZ;
	outExHeightList->ExHeightData[inIndex].SizeGridX = inSizX;
	outExHeightList->ExHeightData[inIndex].SizeGridZ = inSizZ;
	outExHeightList->ExHeightData[inIndex].Height = inHeight;
	outExHeightList->ExHeightData[inIndex].Valid = TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * 拡張高さデータ解放
 *
 * @param	outExHeightData	拡張高さデータの先頭ポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void EXH_FreeHeightList(EHL_PTR outExHeightList)
{
	GFL_HEAP_FreeMemory(outExHeightList->ExHeightData);
	GFL_HEAP_FreeMemory(outExHeightList);
}

//--------------------------------------------------------------------------------------------
/**
 * 拡張高さデータ数取得
 *
 * @param	inExHeightList	拡張高さデータへのポインタ
 *
 * @return	int       データ数
 */
//--------------------------------------------------------------------------------------------
int EXH_GetDatNum(EHL_PTR inExHeightList)
{
  return inExHeightList->Num;
}

//--------------------------------------------------------------------------------------------
/**
 * 拡張高さデータのヒットチェック
 *
 * @param	inGridX			チェックするグリッドX座標
 * @param	inGridZ			チェックするグリッドZ座標
 * @param	inExHeightList	拡張高さデータへのポインタ
 * @param inIdx           データインデックス
 *
 * @return	BOOL	TRUE:ヒット	FALSE:ノーヒット
 */
//--------------------------------------------------------------------------------------------
BOOL EXH_HitCheckHeight(	const int inGridX,
							            const int inGridZ,
							            EHL_CONST_PTR inExHeightList,
							            const u8 inIdx)
{
	u8 i;
	BOOL rc;
	EX_HEIGHT_DATA *ehd_p;

	ehd_p = &inExHeightList->ExHeightData[inIdx];
  if ( HitCheckExHeightFromData( inGridX, inGridZ, ehd_p ) ){
    return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------------------------
/**
 * 拡張高さデータから高さを取得(リスト)
 *
 * @param	inIndex	拡張高さデータインデックス
 * @param	inExHeightList	拡張高さデータへのポインタ
 *
 * @return	fx32	高さ
 */
//--------------------------------------------------------------------------------------------
fx32 EXH_GetHeight(const u8 inIndex, EHL_CONST_PTR inExHeightList)
{
	GF_ASSERT((inIndex<inExHeightList->Num)&&"拡張高さデータのインデックスオーバー");
	GF_ASSERT((inExHeightList->ExHeightData[inIndex].Valid)&&"データは無効です");
	return inExHeightList->ExHeightData[inIndex].Height;
}

//--------------------------------------------------------------------------------------------
/**
 * 拡張高さデータの高さセット(リスト)
 *
 * @param	inIndex	拡張高さデータインデックス
 * @param	inHeight		領域内高さ
 * @param	outExHeightList	拡張高さデータへのポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void EXH_SetHeight(	const u8 inIndex,
						const fx32 inHeight,
						EHL_PTR outExHeightList )
{
	GF_ASSERT((inIndex<outExHeightList->Num)&&"拡張高さデータのインデックスオーバー");
	outExHeightList->ExHeightData[inIndex].Height = inHeight;
}

//--------------------------------------------------------------------------------------------
/**
 * 拡張高さデータの有効・無効化(リスト)
 *
 * @param	inIndex	拡張高さデータインデックス
 * @param	inValid			TRUE:有効	FALSE:無効
 * @param	outExHeightData	拡張高さデータへのポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void EXH_SetHeightValid(	const u8 inIndex,
							const BOOL inValid,
							EHL_PTR outExHeightList )
{
	GF_ASSERT((inIndex<outExHeightList->Num)&&"拡張高さデータのインデックスオーバー");
	outExHeightList->ExHeightData[inIndex].Valid = inValid;
}


