//==============================================================================
/**
 *
 *@file		obj_graphic_man.c
 *@brief	OBJ用グラフィックデータ転送、管理
 *@author	taya
 *@data		2006.11.28
 *
 */
//==============================================================================
#include "gflib.h"
#include "vman.h"

#include "obj_graphic_man.h"


//==============================================================
//	CGRデータ管理
//==============================================================
typedef struct {
	NNSG2dImageProxy			proxy;			///< 転送プロキシ
	void*						loadPtr;		///< データポインタ
	NNSG2dCharacterData*		g2dCharData;	///< NNSキャラデータポインタ
	GFL_VMAN_RESERVE_INFO		riMain;			///< 予約領域情報メイン
	GFL_VMAN_RESERVE_INFO		riSub;			///< 予約領域情報サブ
	u16							refFlag;		///< 他CGRのデータポインタを参照して使っている（VRAM転送型のみ）
	u16							emptyFlag;		///< 未使用フラグ
}CGR_MAN;

//==============================================================
//	パレットデータ管理
//==============================================================
typedef struct {
	NNSG2dImagePaletteProxy		proxy;			///< 転送プロキシ
	BOOL						emptyFlag;		///< 未使用フラグ
}PLTT_MAN;

//==============================================================
//	セルアニメデータ管理
//==============================================================
typedef struct {
	void*						cellLoadPtr;	///< セルデータポインタ
	void*						animLoadPtr;	///< アニメデータポインタ
	NNSG2dCellDataBank*			cellBankPtr;	///< セルバンクポインタ
	NNSG2dAnimBankData*			animBankPtr;	///< アニメバンクポインタ
	BOOL						emptyFlag;		///< 未使用フラグ
}CELLANIM_MAN;

//==============================================================
//	システムワーク
//==============================================================
static struct {
	GFL_VMAN*	vmanMain;
	GFL_VMAN*	vmanSub;

	CGR_MAN*		cgrMan;
	PLTT_MAN*		plttMan;
	CELLANIM_MAN*	cellAnimMan;

	GFL_OBJGRP_INIT_PARAM		initParam;

}SysWork;

//==============================================================
// Prototype
//==============================================================
static void* ReadDataWithUncompress( ARCHANDLE* arc, u32 dataID, BOOL compressFlag, HEAPID heapID );
static CGR_MAN* create_cgr_man( u16 heapID, u32 num );
static inline u32 search_empty_cgr_pos( void );
static void transCGRData( u32 idx, const NNSG2dCharacterData* charData, const NNSG2dCellDataBank* cellBankPtr, GFL_VRAM_TYPE targetVram );
static void delete_cgr_man( CGR_MAN* man );
static PLTT_MAN* create_pltt_man( u16 heapID, u32 num );
static inline u32 search_empty_pltt_pos( void );
static void register_pltt( u32 idx, void* dataPtr, GFL_VRAM_TYPE vramType, u32 byteOffs );
static void delete_pltt_man( PLTT_MAN* man );
static CELLANIM_MAN* create_cellanim_man( u16 heapID, u32 num );
static inline u32 search_empty_cellanim_pos( void );
static BOOL register_cellanim( u32 idx, void* cellDataPtr, void* animDataPtr );
static void delete_cellanim_man( CELLANIM_MAN* man );

static void registerCGR
	( u32 idx, void* dataPtr, GFL_VRAM_TYPE targetVram, const NNSG2dCellDataBank* cellBankPtr );

static inline void trans_cgr_core
	( const NNSG2dCharacterData* charData, u32 byteOfs, NNS_G2D_VRAM_TYPE vramType, 
	  BOOL vramTransferFlag, NNSG2dImageProxy* proxy  );


//------------------------------------------------------------------
/**
 * システム初期化（メイン起動時に１回だけ呼ぶ）
 */
//------------------------------------------------------------------
void GFL_OBJGRP_Init( void )
{
	SysWork.vmanMain = NULL;
	SysWork.vmanSub = NULL;
	SysWork.cgrMan = NULL;
	SysWork.plttMan = NULL;
	SysWork.cellAnimMan = NULL;
}

void GFL_OBJGRP_Exit( void )
{
	
}

//==============================================================================================
/**
 * プロセス毎の初期化処理
 *
 * @param   heapID			[in] 使用ヒープID
 * @param   vramBank		[in] VRAMバンク設定
 * @param   initParam		[in] 初期化構造体
 *
 */
//==============================================================================================
void GFL_OBJGRP_sysStart( u16 heapID, const GFL_BG_DISPVRAM* vramBank, const GFL_OBJGRP_INIT_PARAM* initParam )
{
	GF_ASSERT( SysWork.vmanMain == NULL );
	GF_ASSERT( SysWork.vmanSub == NULL );
	GF_ASSERT( SysWork.cgrMan == NULL );
	GF_ASSERT( SysWork.plttMan == NULL );
	GF_ASSERT( SysWork.cellAnimMan == NULL );

	SysWork.vmanMain = GFL_VMAN_Create( heapID, GFL_VMAN_TYPE_OBJ, vramBank->main_obj );
	SysWork.vmanSub = GFL_VMAN_Create( heapID, GFL_VMAN_TYPE_OBJ, vramBank->sub_obj );
	SysWork.cgrMan = create_cgr_man( heapID, initParam->CGR_RegisterMax );
	SysWork.plttMan = create_pltt_man( heapID, initParam->PLTT_RegisterMax );
	SysWork.cellAnimMan = create_cellanim_man( heapID, initParam->CELL_RegisterMax );

	SysWork.initParam = *initParam;
}

//==============================================================================================
/**
 * プロセス毎の終了処理
 */
//==============================================================================================
void GFL_OBJGRP_sysEnd( void )
{
	GF_ASSERT( SysWork.vmanMain != NULL );
	GF_ASSERT( SysWork.vmanSub != NULL );
	GF_ASSERT( SysWork.cgrMan != NULL );
	GF_ASSERT( SysWork.plttMan != NULL );
	GF_ASSERT( SysWork.cellAnimMan != NULL );

	{
		u32 i;

		for(i=0; i<SysWork.initParam.CGR_RegisterMax; i++)
		{
			if( SysWork.cgrMan[i].emptyFlag == FALSE )
			{
				GFL_OBJGRP_ReleaseCGR( i );
			}
		}

		for(i=0; i<SysWork.initParam.CELL_RegisterMax; i++)
		{
			if( SysWork.cellAnimMan[i].emptyFlag == FALSE )
			{
				GFL_OBJGRP_ReleaseCellAnim( i );
			}
		}

		for(i=0; i<SysWork.initParam.PLTT_RegisterMax; i++)
		{
			if( SysWork.plttMan[i].emptyFlag == FALSE )
			{
				GFL_OBJGRP_ReleasePltt( i );
			}
		}

		delete_cgr_man( SysWork.cgrMan );
		delete_pltt_man( SysWork.plttMan );
		delete_cellanim_man( SysWork.cellAnimMan );
		GFL_VMAN_Delete( SysWork.vmanSub );
		GFL_VMAN_Delete( SysWork.vmanMain );

		SysWork.vmanMain = NULL;
		SysWork.vmanSub = NULL;
		SysWork.cgrMan = NULL;
		SysWork.plttMan = NULL;
		SysWork.cellAnimMan = NULL;
	}
}
//==============================================================================================
// アーカイブからのデータ読み出し・解凍処理
//==============================================================================================
static void* ReadDataWithUncompress( ARCHANDLE* arc, u32 dataID, BOOL compressFlag, HEAPID heapID )
{
	void* retBuf;

	if( compressFlag )
	{
		void* tmpBuf;
		u32 size;

		tmpBuf = GFL_ARC_LoadDataAllocByHandle( arc, dataID, GetHeapLowID(heapID) );
		size = MI_GetUncompressedSize( tmpBuf );
		retBuf = GFL_HEAP_AllocMemory( heapID, size );
		MI_UncompressLZ8( tmpBuf, retBuf );
		GFL_HEAP_FreeMemory( tmpBuf );
	}
	else
	{
		retBuf = GFL_ARC_LoadDataAllocByHandle( arc, dataID, GetHeapLowID(heapID) );
	}

	return retBuf;
}

//==============================================================================================
// データ登録および解放関数群
//==============================================================================================

//==============================================================================================
/**
 * CGRデータの登録（VRAM常駐型OBJ用）
 *
 * アーカイブからCGRデータをロードしてVRAM転送を行い、プロキシを作成して保持する。
 * CGRデータの実体は破棄する。
 *
 * @param   arcHandle		[in] アーカイブハンドル
 * @param   cgrDataID		[in] アーカイブ内のCGRデータID
 * @param   compressedFlag	[in] データが圧縮されているか
 * @param   targetVram		[in] 転送先VRAM指定
 * @param   heapID			[in] データロード用ヒープ（テンポラリ）
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//==============================================================================================
u32 GFL_OBJGRP_RegisterCGR( ARCHANDLE* arcHandle, u32 cgrDataID, BOOL compressedFlag, GFL_VRAM_TYPE targetVram, HEAPID heapID )
{
	u32  idx = search_empty_cgr_pos();
	if( idx != GFL_OBJGRP_REGISTER_FAILED )
	{
		CGR_MAN* cgrMan = &SysWork.cgrMan[idx];
		void* loadPtr = ReadDataWithUncompress( arcHandle, 
												cgrDataID, 
												compressedFlag, 
												GetHeapLowID(heapID) );

		registerCGR( idx, loadPtr, targetVram, NULL );
		GFL_HEAP_FreeMemory( loadPtr );
		cgrMan->loadPtr = NULL;
		cgrMan->refFlag = FALSE;

		return idx;
	}
	GF_ASSERT(0);
	return GFL_OBJGRP_REGISTER_FAILED;
}

//==============================================================================================
/**
 * CGRデータの登録（VRAM転送型OBJ用）
 *
 * アーカイブからCGRデータをロードし、プロキシを作成して保持する。
 * CGRデータの実体も保持し続ける。
 * CGRデータに関連付けられたセルアニメデータが先に登録されている必要がある。
 *
 * @param   arcHandle		[in] アーカイブハンドル
 * @param   cgrDataID		[in] CGRデータのアーカイブ内ID
 * @param   targetVram		[in] 転送先VRAM指定
 * @param   cellIndex		[in] 関連付けられたセルデータの登録インデックス
 * @param   heapID			[in] データロード用ヒープ
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//==============================================================================================
u32 GFL_OBJGRP_RegisterCGR_VramTransfer( ARCHANDLE* arcHandle, u32 cgrDataID, BOOL compressedFlag, GFL_VRAM_TYPE targetVram, u32 cellIndex, HEAPID heapID )
{
	GF_ASSERT(SysWork.cellAnimMan[cellIndex].emptyFlag == FALSE);
	GF_ASSERT(NNS_G2dCellDataBankHasVramTransferData(SysWork.cellAnimMan[cellIndex].cellBankPtr));

	{
		u32 idx = search_empty_cgr_pos();
		if( idx != GFL_OBJGRP_REGISTER_FAILED )
		{
			CGR_MAN* cgrMan = &SysWork.cgrMan[idx];

			cgrMan->loadPtr = ReadDataWithUncompress( arcHandle, cgrDataID, compressedFlag, heapID );
			registerCGR(idx, cgrMan->loadPtr, targetVram, SysWork.cellAnimMan[cellIndex].cellBankPtr);
			cgrMan->refFlag = FALSE;

			return idx;
		}

		GF_ASSERT(0);
		return GFL_OBJGRP_REGISTER_FAILED;
	}
}

//==============================================================================================
/**
 * 登録されたVRAM転送型CGR参照のコピー
 *
 * すでに登録されているCGRデータを参照し、プロキシを作成・保持する。
 * この関数を使った登録したCGRデータを解放しても、実データの解放は行わない。
 * CGRデータに関連付けられたセルアニメデータが先に登録されている必要がある。
 *
 * @param   srcCgrIdx		[in] すでに登録されているCGRデータインデックス（VRAM転送型）
 * @param   cellAnimIdx		[in] 関連づけられたセルアニメデータの登録インデックス
 * @param   targetVram		[in] 転送先VRAM
 *
 * @retval  u32		登録インデックス
 */
//==============================================================================================
u32 GFL_OBJGRP_CopyCGR_VramTransfer( u32 srcCgrIdx, u32 cellAnimIdx, GFL_VRAM_TYPE targetVram )
{
	GF_ASSERT(SysWork.cellAnimMan[cellAnimIdx].emptyFlag == FALSE);
	GF_ASSERT(NNS_G2dCellDataBankHasVramTransferData(SysWork.cellAnimMan[cellAnimIdx].cellBankPtr));
	GF_ASSERT(SysWork.cgrMan[srcCgrIdx].emptyFlag==FALSE);
	GF_ASSERT(SysWork.cgrMan[srcCgrIdx].loadPtr);

	{
		u32 idx = search_empty_cgr_pos();
		if( idx != GFL_OBJGRP_REGISTER_FAILED )
		{
			CGR_MAN* cgrMan = &SysWork.cgrMan[idx];

			cgrMan->loadPtr = SysWork.cgrMan[srcCgrIdx].loadPtr;
			cgrMan->g2dCharData = SysWork.cgrMan[srcCgrIdx].g2dCharData;
			transCGRData( idx, cgrMan->g2dCharData, SysWork.cellAnimMan[cellAnimIdx].cellBankPtr, targetVram );

			cgrMan->refFlag = TRUE;
			cgrMan->emptyFlag = FALSE;
			return idx;
		}

		GF_ASSERT(0);
		return GFL_OBJGRP_REGISTER_FAILED;
	}
}
//==============================================================================================
/**
 * 登録済みVRAM転送型CGRデータを別のデータで上書きする
 *
 * @param   index			登録済みCGRデータインデックス（VRAM転送型）
 * @param   arc				上書きするCGRデータを持つアーカイブのハンドル
 * @param   cgrDatIdx		上書きするCGRデータのアーカイブ内インデックス
 * @param   compressedFlag	CGRデータが圧縮されているか
 * @param	heapID			作業用ヒープID
 *
 */
//==============================================================================================
void GFL_OBJGRP_ReloadCGR_VramTransfer( u32 index, ARCHANDLE* arc, u32 cgrDatIdx, BOOL compressedFlag, HEAPID heapID )
{
	GF_ASSERT(SysWork.cgrMan[index].emptyFlag==FALSE);
	GF_ASSERT(SysWork.cgrMan[index].g2dCharData);

	{
		NNSG2dCharacterData	*srcData, *dstData;
		void *srcLoadPtr;

		dstData = SysWork.cgrMan[index].g2dCharData;

		if( compressedFlag )
		{
			void* tmp;
			u32 size;

			tmp = GFL_ARC_LoadDataAllocByHandle( arc, cgrDatIdx, GetHeapLowID(heapID) );
			size = MI_GetUncompressedSize( tmp );
			srcLoadPtr = GFL_HEAP_AllocMemory( GetHeapLowID(heapID), size );
			MI_UncompressLZ8( tmp, srcLoadPtr );
			GFL_HEAP_FreeMemory( tmp );
		}
		else
		{
			srcLoadPtr = GFL_ARC_LoadDataAllocByHandle( arc, cgrDatIdx, GetHeapLowID(heapID) );
		}

		NNS_G2dGetUnpackedCharacterData( srcLoadPtr, &srcData );

		if( srcData->szByte <= dstData->szByte )
		{
			MI_CpuCopyFast( srcData->pRawData, dstData->pRawData, srcData->szByte );
//			OS_TPrintf("srcAdrs:%08x, dstAdrs:%08x, size:%08x\n",
//				(u32)(srcData->pRawData), (u32)(dstData->pRawData), srcData->szByte );
			DC_FlushRange( dstData->pRawData, srcData->szByte );
		}
		else
		{
			GF_ASSERT(0);
		}

		GFL_HEAP_FreeMemory( srcLoadPtr );
	}


}


//==============================================================================================
/**
 * VRAM転送型OBJの転送元データポインタを取得（取り扱いは慎重に！）
 *
 * @param   index		登録インデックス
 *
 * @retval  void*		データポインタ
 */
//==============================================================================================
void* GFL_OBJGRP_GetVramTransCGRPointer( u32 index )
{
	GF_ASSERT(SysWork.cgrMan[index].emptyFlag == FALSE);
	GF_ASSERT(SysWork.cgrMan[index].loadPtr != NULL);

	return SysWork.cgrMan[index].loadPtr;
}

//==============================================================================================
/**
 * 登録されたCGRデータの解放
 *
 * @param   index		[in] 登録インデックス
 */
//==============================================================================================
void GFL_OBJGRP_ReleaseCGR( u32 index )
{
	GF_ASSERT( index < SysWork.initParam.CGR_RegisterMax );
	GF_ASSERT( SysWork.cgrMan[index].emptyFlag == FALSE );

	if( GFL_VMAN_IsReserveInfoInitialized(&SysWork.cgrMan[index].riMain) == FALSE )
	{
		GFL_VMAN_Release( SysWork.vmanMain, &SysWork.cgrMan[index].riMain );
	}

	if( GFL_VMAN_IsReserveInfoInitialized(&SysWork.cgrMan[index].riSub) == FALSE )
	{
		GFL_VMAN_Release( SysWork.vmanSub, &SysWork.cgrMan[index].riSub );
	}

	if( SysWork.cgrMan[index].loadPtr )
	{
		if( SysWork.cgrMan[index].refFlag == FALSE )
		{
			GFL_HEAP_FreeMemory( SysWork.cgrMan[index].loadPtr );
		}
		else
		{
			SysWork.cgrMan[index].refFlag = FALSE;
		}
		SysWork.cgrMan[index].loadPtr = NULL;
	}

	SysWork.cgrMan[index].emptyFlag = TRUE;

}



//==============================================================================================
/**
 * セルアニメデータ登録
 *
 * アーカイブからセル＆アニメデータをロードし、Unpackして実体ごと保持する。
 *
 * @param   arcHandle		アーカイブハンドル
 * @param   cellDataID		セルデータID
 * @param   animDataID		アニメデータID
 * @param   heapID			ヒープID
 *
 * @retval  登録インデックス
 *
 */
//==============================================================================================
u32 GFL_OBJGRP_RegisterCellAnim( ARCHANDLE* arcHandle, u32 cellDataID, u32 animDataID, u16 heapID )
{
	u32 idx = search_empty_cellanim_pos();

	{
		CELLANIM_MAN* man = &SysWork.cellAnimMan[idx];

		man->cellLoadPtr = GFL_ARC_LoadDataAllocByHandle( arcHandle, cellDataID, heapID );
		man->animLoadPtr = GFL_ARC_LoadDataAllocByHandle( arcHandle, animDataID, heapID );

		if( register_cellanim( idx, man->cellLoadPtr, man->animLoadPtr ) == FALSE )
		{
			GF_ASSERT(0);
		}

		return idx;
	}
}
//==============================================================================================
/**
 * 登録されたセルアニメデータの解放
 *
 * @param   index		[in] 登録インデックス
 */
//==============================================================================================
void GFL_OBJGRP_ReleaseCellAnim( u32 index )
{
	GF_ASSERT( index < SysWork.initParam.CELL_RegisterMax );
	GF_ASSERT( SysWork.cellAnimMan[index].emptyFlag == FALSE );

	{
		CELLANIM_MAN* man = &SysWork.cellAnimMan[index];

		if( man->cellLoadPtr )
		{
			GFL_HEAP_FreeMemory( man->cellLoadPtr );
			man->cellLoadPtr = NULL;
		}
		if( man->animLoadPtr )
		{
			GFL_HEAP_FreeMemory( man->animLoadPtr );
			man->animLoadPtr = NULL;
		}

		man->emptyFlag = TRUE;
	}
}

//==============================================================================================
/**
 * 指定されたセルデータが、VRAM転送型かどうかをチェック
 *
 * @param   index		[in] 登録インデックス
 *
 * @retval  BOOL		TRUEならVRAM転送型データ
 */
//==============================================================================================
BOOL GFL_OBJGRP_CellBankHasVramTransferData( u32 index )
{
	GF_ASSERT( index < SysWork.initParam.CELL_RegisterMax );
	GF_ASSERT( SysWork.cellAnimMan[index].emptyFlag == FALSE );

	return NNS_G2dCellDataBankHasVramTransferData( SysWork.cellAnimMan[index].cellBankPtr );
}



//==============================================================================================
/**
 * パレットデータの登録およびVRAMへの転送
 *
 * パレットデータをアーカイブからロードしてVRAM転送を行い、プロキシを作成して保持する。
 * ロードしたデータ実体は破棄する。
 *
 * @param   arcHandle		[in] アーカイブハンドル
 * @param   plttDataID		[in] パレットデータのアーカイブ内データID
 * @param   vramType		[in] 転送先VRAM指定
 * @param   byteOffs		[in] 転送オフセット（バイト）
 * @param   heapID			[in] データロード用ヒープ（テンポラリ）
 *
 * OBJ拡張パレットを指定したい場合、 byteOffsの値に GFL_OBJGRP_EXPLTT_OFFSET + オフセットバイト数を指定する
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 *
 */
//==============================================================================================
u32 GFL_OBJGRP_RegisterPltt( ARCHANDLE* arcHandle, u32 plttDataID, GFL_VRAM_TYPE vramType, u32 byteOffs, u16 heapID )
{
	u32 idx = search_empty_pltt_pos();
	if( idx != GFL_OBJGRP_REGISTER_FAILED )
	{
		void* loadPtr = GFL_ARC_LoadDataAllocByHandle(	arcHandle, 
														plttDataID, 
														GetHeapLowID(heapID) );
		register_pltt( idx, loadPtr, vramType, byteOffs );
		GFL_HEAP_FreeMemory( loadPtr );

		return idx;
	}

	return idx;
}

//==============================================================================================
/**
 * 登録されたパレットデータの解放
 *
 * @param   index		[in] 登録インデックス
 */
//==============================================================================================
void GFL_OBJGRP_ReleasePltt( u32 index )
{
	GF_ASSERT( index < SysWork.initParam.PLTT_RegisterMax );
	GF_ASSERT( SysWork.plttMan[index].emptyFlag == FALSE );

	SysWork.plttMan[index].emptyFlag = TRUE;
}


//==============================================================================================
// 登録データのProxy取得
//==============================================================================================


//==============================================================================================
/**
 * CGRプロキシ取得
 *
 * @param   index		[in]  登録インデックス
 * @param   proxy		[out] プロキシデータ取得のための構造体アドレス
 *
 */
//==============================================================================================
void GFL_OBJGRP_GetCGRProxy( u32 index, NNSG2dImageProxy* proxy )
{
	GF_ASSERT(index < SysWork.initParam.CGR_RegisterMax);
	GF_ASSERT(SysWork.cgrMan[index].emptyFlag == FALSE);

	*proxy = SysWork.cgrMan[index].proxy;
}






//-------------------------------------
///	GFL_CLWK初期化基本データ
//=====================================
/*
typedef struct {
	s16	pos_x;				// ｘ座標
	s16 pos_y;				// ｙ座標
	u16 anmseq;				// アニメーションシーケンス
	u8	softpri;			// ソフト優先順位	0>0xff
	u8	bgpri;				// BG優先順位
} GFL_CLWK_DATA;
*/

//==============================================================================================
// 登録データによるセルアクター構築
//==============================================================================================
//==============================================================================================
/**
 * 登録データによるセルアクター構築（VRAM常駐タイプ：ノーマル）
 *
 * @param   actUnit			[in] アクターユニットポインタ
 * @param   cgrIndex		[in] 使用するCGRデータの登録インデックス
 * @param   plttIndex		[in] 使用するパレットデータの登録インデックス
 * @param   cellAnimIndex	[in] 使用するセルアニメデータの登録インデックス
 * @param   param			[in] アクター登録パラメータ
 * @param   setSerface		[in] アクターをセットするサーフェイス指定（clact.h参照）
 * @param   heapID			[in] アクター作成用ヒープID
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//==============================================================================================
GFL_CLWK* GFL_OBJGRP_CreateClAct( GFL_CLUNIT* actUnit, u32 cgrIndex, u32 plttIndex, u32 cellAnimIndex, 
	  const GFL_CLWK_DATA* param, u16 setSerface, HEAPID heapID )
{
	GF_ASSERT( cgrIndex < SysWork.initParam.CGR_RegisterMax );
	GF_ASSERT( plttIndex < SysWork.initParam.PLTT_RegisterMax );
	GF_ASSERT( cellAnimIndex < SysWork.initParam.CELL_RegisterMax );
	GF_ASSERT( SysWork.cgrMan[cgrIndex].emptyFlag == FALSE );
	GF_ASSERT_MSG( SysWork.plttMan[plttIndex].emptyFlag == FALSE, "plttIndex=%d", plttIndex );
	GF_ASSERT( SysWork.cellAnimMan[cellAnimIndex].emptyFlag == FALSE );


	{
		GFL_CLWK_RES     clactRes;

		GFL_CLACT_WK_SetCellResData( &clactRes,
					&(SysWork.cgrMan[cgrIndex].proxy),
					&(SysWork.plttMan[plttIndex].proxy),
					SysWork.cellAnimMan[cellAnimIndex].cellBankPtr,
					SysWork.cellAnimMan[cellAnimIndex].animBankPtr
		);

		return GFL_CLACT_WK_Add( actUnit, param, &clactRes, setSerface, heapID );
	}
}
//==============================================================================================
/**
 * 登録データによるセルアクター構築（VRAM常駐タイプ：アフィン）
 *
 * @param   actUnit			[in] アクターユニットポインタ
 * @param   cgrIndex		[in] 使用するCGRデータの登録インデックス
 * @param   plttIndex		[in] 使用するパレットデータの登録インデックス
 * @param   cellAnimIndex	[in] 使用するセルアニメデータの登録インデックス
 * @param   param			[in] アクター登録パラメータ
 * @param   setSerface		[in] アクターをセットするサーフェイス指定（clact.h参照）
 * @param   heapID			[in] アクター作成用ヒープID
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//==============================================================================================
GFL_CLWK* GFL_OBJGRP_CreateClActAffine( GFL_CLUNIT* actUnit, u32 cgrIndex, u32 plttIndex, u32 cellAnimIndex,
	const GFL_CLWK_AFFINEDATA* param, u16 setSerface, HEAPID heapID )
{
	GF_ASSERT( cgrIndex < SysWork.initParam.CGR_RegisterMax );
	GF_ASSERT( plttIndex < SysWork.initParam.PLTT_RegisterMax );
	GF_ASSERT( cellAnimIndex < SysWork.initParam.CELL_RegisterMax );
	GF_ASSERT( SysWork.cgrMan[cgrIndex].emptyFlag == FALSE );
	GF_ASSERT_MSG( SysWork.plttMan[plttIndex].emptyFlag == FALSE, "plttIndex=%d", plttIndex );
	GF_ASSERT( SysWork.cellAnimMan[cellAnimIndex].emptyFlag == FALSE );


	{
		GFL_CLWK_RES     clactRes;

		GFL_CLACT_WK_SetCellResData( &clactRes,
					&(SysWork.cgrMan[cgrIndex].proxy),
					&(SysWork.plttMan[plttIndex].proxy),
					SysWork.cellAnimMan[cellAnimIndex].cellBankPtr,
					SysWork.cellAnimMan[cellAnimIndex].animBankPtr
		);

		return GFL_CLACT_WK_AddAffine( actUnit, param, &clactRes, setSerface, heapID );
	}
}
//==============================================================================================
/**
 * 登録データによるセルアクター構築（VRAM転送タイプ：ノーマル）
 *
 * @param   actUnit			[in] アクターユニットポインタ
 * @param   cgrIndex		[in] 使用するCGRデータの登録インデックス
 * @param   plttIndex		[in] 使用するパレットデータの登録インデックス
 * @param   cellAnimIndex	[in] 使用するセルアニメデータの登録インデックス
 * @param   param			[in] アクター登録パラメータ
 * @param   setSerface		[in] アクターをセットするサーフェイス指定（clact.h参照）
 * @param   heapID			[in] アクター作成用ヒープID
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//==============================================================================================
GFL_CLWK* GFL_OBJGRP_CreateClActVT( GFL_CLUNIT* actUnit, u32 cgrIndex, u32 plttIndex, u32 cellAnimIndex, 
	  const GFL_CLWK_DATA* param, u16 setSerface, HEAPID heapID )
{
	GF_ASSERT( cgrIndex < SysWork.initParam.CGR_RegisterMax );
	GF_ASSERT( plttIndex < SysWork.initParam.PLTT_RegisterMax );
	GF_ASSERT( cellAnimIndex < SysWork.initParam.CELL_RegisterMax );
	GF_ASSERT( SysWork.cgrMan[cgrIndex].emptyFlag == FALSE );
	GF_ASSERT_MSG( SysWork.plttMan[plttIndex].emptyFlag == FALSE, "plttIndex=%d", plttIndex );
	GF_ASSERT( SysWork.cellAnimMan[cellAnimIndex].emptyFlag == FALSE );


	{
		GFL_CLWK_RES     clactRes;

		GFL_CLACT_WK_SetTrCellResData( &clactRes,
					&(SysWork.cgrMan[cgrIndex].proxy),
					&(SysWork.plttMan[plttIndex].proxy),
					SysWork.cellAnimMan[cellAnimIndex].cellBankPtr,
					SysWork.cellAnimMan[cellAnimIndex].animBankPtr,
					SysWork.cgrMan[cgrIndex].g2dCharData
		);

		return GFL_CLACT_WK_Add( actUnit, param, &clactRes, setSerface, heapID );
	}
}
//==============================================================================================
/**
 * 登録データによるセルアクター構築（VRAM転送タイプ：アフィン）
 *
 * @param   actUnit			[in] アクターユニットポインタ
 * @param   cgrIndex		[in] 使用するCGRデータの登録インデックス
 * @param   plttIndex		[in] 使用するパレットデータの登録インデックス
 * @param   cellAnimIndex	[in] 使用するセルアニメデータの登録インデックス
 * @param   param			[in] アクター登録パラメータ
 * @param   setSerface		[in] アクターをセットするサーフェイス指定（clact.h参照）
 * @param   heapID			[in] アクター作成用ヒープID
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//==============================================================================================
extern GFL_CLWK* GFL_OBJGRP_CreateClActVTAffine( GFL_CLUNIT* actUnit, u32 cgrIndex, u32 plttIndex, u32 cellAnimIndex, 
	  const GFL_CLWK_AFFINEDATA* param, u16 setSerface, HEAPID heapID )
{
	GF_ASSERT( cgrIndex < SysWork.initParam.CGR_RegisterMax );
	GF_ASSERT( plttIndex < SysWork.initParam.PLTT_RegisterMax );
	GF_ASSERT( cellAnimIndex < SysWork.initParam.CELL_RegisterMax );
	GF_ASSERT( SysWork.cgrMan[cgrIndex].emptyFlag == FALSE );
	GF_ASSERT_MSG( SysWork.plttMan[plttIndex].emptyFlag == FALSE, "plttIndex=%d", plttIndex );
	GF_ASSERT( SysWork.cellAnimMan[cellAnimIndex].emptyFlag == FALSE );


	{
		GFL_CLWK_RES     clactRes;

		GFL_CLACT_WK_SetTrCellResData( &clactRes,
					&(SysWork.cgrMan[cgrIndex].proxy),
					&(SysWork.plttMan[plttIndex].proxy),
					SysWork.cellAnimMan[cellAnimIndex].cellBankPtr,
					SysWork.cellAnimMan[cellAnimIndex].animBankPtr,
					SysWork.cgrMan[cgrIndex].g2dCharData
		);

		return GFL_CLACT_WK_AddAffine( actUnit, param, &clactRes, setSerface, heapID );
	}
}


//==============================================================================================
//==============================================================================================
//------------------------------------------------------------------
/**
 * CGR管理領域の構築、初期化
 *
 * @param   heapID		ヒープID
 * @param   num			管理するCGRデータ数
 *
 * @retval  CGR_MAN*	管理領域ポインタ
 */
//------------------------------------------------------------------
static CGR_MAN* create_cgr_man( u16 heapID, u32 num )
{
	u32 i;
	CGR_MAN* man = GFL_HEAP_AllocMemory( heapID, sizeof(CGR_MAN)*num );
	for(i=0; i<num; i++)
	{
		man[i].emptyFlag = TRUE;
		GFL_VMAN_InitReserveInfo( &man[i].riMain );
		GFL_VMAN_InitReserveInfo( &man[i].riSub );
	}
	return man;
}

//------------------------------------------------------------------
/**
 * CGR管理領域から空きを探す
 *
 * @retval  u32		空きindex
 */
//------------------------------------------------------------------
static inline u32 search_empty_cgr_pos( void )
{
	u32 i;
	for(i=0; i<SysWork.initParam.CGR_RegisterMax; i++)
	{
		if( SysWork.cgrMan[i].emptyFlag )
		{
			return i;
		}
	}
	return GFL_OBJGRP_REGISTER_FAILED;
}

//------------------------------------------------------------------
/**
 * CGR登録、転送
 *
 * @param   dataPtr			
 * @param   targetVram		
 * @param   cellDataPtr		VRAM転送型の時、最大領域サイズを取得するのに使用。
 *							VRAM常駐型なら、NULLを指定すること。
 *
 * @retval  BOOL			成功時TRUE
 */
//------------------------------------------------------------------
static void registerCGR
( u32 idx, void* dataPtr, GFL_VRAM_TYPE targetVram, const NNSG2dCellDataBank* cellBankPtr )
{
	NNSG2dCharacterData* charData;

	if( NNS_G2dGetUnpackedCharacterData( dataPtr, &charData ) )
	{
		transCGRData( idx, charData, cellBankPtr, targetVram );
		SysWork.cgrMan[idx].emptyFlag = FALSE;
	}
	else
	{
		GF_ASSERT(0);
	}
}

//------------------------------------------------------------------
/**
 * CGR用VRAM予約＆データ転送
 *
 * @param   idx				CGR登録インデックス
 * @param   charData		NitroSystemキャラデータポインタ
 * @param   cellBankPtr		VRAM転送型なら関連付けるセルバンクデータポインタ／非転送型ならNULLを指定すること
 * @param   targetVram		転送先VRAM指定
 *
 */
//------------------------------------------------------------------
static void transCGRData( u32 idx, const NNSG2dCharacterData* charData, const NNSG2dCellDataBank* cellBankPtr, GFL_VRAM_TYPE targetVram )
{
	CGR_MAN* cgrMan = &SysWork.cgrMan[idx];
	u32  transByteSize;
	BOOL vramTransFlag;


	if( cellBankPtr == NULL )
	{
		transByteSize = charData->szByte;
		vramTransFlag = FALSE;
	}
	else
	{
		transByteSize = NNS_G2dGetSizeRequiredVramTransferCellDataBank( cellBankPtr );
		vramTransFlag = TRUE;
	}

	NNS_G2dInitImageProxy( &cgrMan->proxy );

	if( targetVram & GFL_VRAM_2D_MAIN )
	{
		if( GFL_VMAN_Reserve( SysWork.vmanMain, transByteSize, &cgrMan->riMain ) )
		{
			u32 byteOfs = GFL_VMAN_GetByteOffset( SysWork.vmanMain, &cgrMan->riMain );
			trans_cgr_core( charData, byteOfs, NNS_G2D_VRAM_TYPE_2DMAIN, 
						vramTransFlag, &cgrMan->proxy );
		}
	}
	if( targetVram & GFL_VRAM_2D_SUB )
	{
		if( GFL_VMAN_Reserve( SysWork.vmanSub, transByteSize, &cgrMan->riSub ) )
		{
			u32 byteOfs = GFL_VMAN_GetByteOffset( SysWork.vmanSub, &cgrMan->riSub );
			trans_cgr_core( charData, byteOfs, NNS_G2D_VRAM_TYPE_2DSUB, 
						vramTransFlag, &cgrMan->proxy );
		}
	}

	cgrMan->g2dCharData = ( vramTransFlag )? (NNSG2dCharacterData*)charData : NULL;
}


//------------------------------------------------------------------
/**
 * CGRデータ転送コア
 *
 * @param   charData			[in] NITROSystem CharData
 * @param   byteOfs				[in] VRAM先頭からの転送オフセット
 * @param   vramType			[in] 転送先VRAM 2D-Main, 2D-Sub
 * @param   vramTransferFlag	[in] VRAM転送型ならTRUEを指定
 * @param   proxy				[out] 構築するプロキシアドレス
 *
 */
//------------------------------------------------------------------
static inline void trans_cgr_core
	( const NNSG2dCharacterData* charData, u32 byteOfs, NNS_G2D_VRAM_TYPE vramType, 
	  BOOL vramTransferFlag, NNSG2dImageProxy* proxy  )
{
	if( vramTransferFlag )
	{
//		OS_TPrintf("trans cgr VT  ofs:0x%08x, type:%d\n", byteOfs, vramType);
		NNS_G2dLoadImageVramTransfer( charData, byteOfs, vramType, proxy );
	}
	else
	{
		if( charData->mapingType != GX_OBJVRAMMODE_CHAR_2D )
		{
//			OS_TPrintf("trans cgr 1d  ofs:0x%08x, type:%d\n", byteOfs, vramType);
			NNS_G2dLoadImage1DMapping( charData, byteOfs, vramType, proxy );
		}
		else
		{
//			OS_TPrintf("trans cgr 2d  ofs:0x%08x, type:%d\n", byteOfs, vramType);
			NNS_G2dLoadImage2DMapping( charData, byteOfs, vramType, proxy );
		}
	}
}

//------------------------------------------------------------------
/**
 * CGR管理領域の破棄
 *
 * @param   man			管理領域ポインタ
 */
//------------------------------------------------------------------
static void delete_cgr_man( CGR_MAN* man )
{
	GFL_HEAP_FreeMemory( man );
}

//------------------------------------------------------------------
/**
 * パレット管理領域の構築、初期化
 *
 * @param   heapID		ヒープID
 * @param   num			管理するCGRデータ数
 *
 * @retval  CGR_MAN*	管理領域ポインタ
 */
//------------------------------------------------------------------
static PLTT_MAN* create_pltt_man( u16 heapID, u32 num )
{
	u32 i;
	PLTT_MAN* man = GFL_HEAP_AllocMemory( heapID, sizeof(PLTT_MAN)*num );

	for(i=0; i<num; i++)
	{
		man[i].emptyFlag = TRUE;
	}
	return man;
}

//------------------------------------------------------------------
/**
 * パレット管理領域から空きを探す
 *
 * @retval  u32		空きindex
 */
//------------------------------------------------------------------
static inline u32 search_empty_pltt_pos( void )
{
	u32 i;
	for(i=0; i<SysWork.initParam.PLTT_RegisterMax; i++)
	{
		if( SysWork.plttMan[i].emptyFlag )
		{
			return i;
		}
	}
	return GFL_OBJGRP_REGISTER_FAILED;
}

//------------------------------------------------------------------
/**
 * パレットデータの登録＆転送
 *
 * @param   plttData		[in] パレットデータ
 * @param   vramType		[in] 転送先VRAM指定
 * @param   byteOffs		[in] 転送開始オフセット
 *
 */
//------------------------------------------------------------------
static void register_pltt( u32 idx, void* dataPtr, GFL_VRAM_TYPE vramType, u32 byteOffs )
{
	NNSG2dPaletteData*  palData;
	NNSG2dPaletteCompressInfo*  cmpData;
	BOOL  cmpFlag, exFlag;

	PLTT_MAN* man = &SysWork.plttMan[idx];

	// 拡張パレット判定
	if( byteOffs < GFL_OBJGRP_EXPLTT_OFFSET )
	{
		exFlag = FALSE;
	}
	else
	{
		exFlag = TRUE;
		byteOffs -= GFL_OBJGRP_EXPLTT_OFFSET;
	}

	cmpFlag = NNS_G2dGetUnpackedPaletteCompressInfo( dataPtr, &cmpData );

	if( NNS_G2dGetUnpackedPaletteData( dataPtr, &palData ) )
	{
		NNS_G2dInitImagePaletteProxy( &man->proxy );

		if( cmpFlag )
		{
			if( vramType & GFL_VRAM_2D_MAIN )
			{
				/*
					拡張パレット転送処理に関する覚え書き

					NitroSDKのマニュアル（GX_SetBankForOBJExtPltt とか GX_SetBankForLCDC のあたり）を読む限り、
					拡張パレットへのバンク割り当てを解除→LCDCにバンク割り当て→
					パレットデータ転送→拡張パレットへのバンク割り当て復帰
					としなければならないように思えるが、
					拡張パレットへのバンク割り当てを解除処理（→GX_SetBankForOBJExtPltt(0);)
					を行うと正しく動作しない。以下のように最低限の処理のみ行うと動作するようである。
					サブ2Dエンジンも同様。

					taya 07/02/21
				*/
				int bank = GX_GetBankForOBJExtPltt();
				if( exFlag ){ GX_SetBankForLCDC( bank ); }
				NNS_G2dLoadPaletteEx(	palData, cmpData, byteOffs, 
										NNS_G2D_VRAM_TYPE_2DMAIN, &man->proxy );
				if( exFlag ){ GX_SetBankForOBJExtPltt( bank ); }
			}
			if( vramType & GFL_VRAM_2D_SUB )
			{
				int bank = GX_GetBankForSubOBJExtPltt();
				if( exFlag ){ GX_SetBankForLCDC( bank ); }
				NNS_G2dLoadPaletteEx(	palData, cmpData, byteOffs, 
										NNS_G2D_VRAM_TYPE_2DSUB, &man->proxy );
				if( exFlag ){ GX_SetBankForSubOBJExtPltt( bank ); }
			}
		}
		else
		{
			if( vramType & GFL_VRAM_2D_MAIN )
			{
				int bank = GX_GetBankForOBJExtPltt();
				if( exFlag ){ GX_SetBankForLCDC( bank ); }
				NNS_G2dLoadPalette( palData, byteOffs, NNS_G2D_VRAM_TYPE_2DMAIN, &man->proxy );
				if( exFlag ){ GX_SetBankForOBJExtPltt( bank ); }
			}
			if( vramType & GFL_VRAM_2D_SUB )
			{
				int bank = GX_GetBankForSubOBJExtPltt();
				if( exFlag ){ GX_SetBankForLCDC( bank ); }
				NNS_G2dLoadPalette( palData, byteOffs, NNS_G2D_VRAM_TYPE_2DSUB, &man->proxy );
				if( exFlag ){ GX_SetBankForSubOBJExtPltt( bank ); }
			}
		}

		SysWork.plttMan[idx].emptyFlag = FALSE;
	}
	else
	{
		GF_ASSERT(0);
	}
}

//------------------------------------------------------------------
/**
 * パレット管理領域の破棄
 *
 * @param   man			管理領域ポインタ
 */
//------------------------------------------------------------------
static void delete_pltt_man( PLTT_MAN* man )
{
	GFL_HEAP_FreeMemory( man );
}

//------------------------------------------------------------------
/**
 * セルアニメ管理領域の構築、初期化
 *
 * @param   heapID		ヒープID
 * @param   num			管理するCGRデータ数
 *
 * @retval  CGR_MAN*	管理領域ポインタ
 */
//------------------------------------------------------------------
static CELLANIM_MAN* create_cellanim_man( u16 heapID, u32 num )
{
	u32 i;
	CELLANIM_MAN* man = GFL_HEAP_AllocMemory( heapID, sizeof(CELLANIM_MAN)*num );

	for(i=0; i<num; i++)
	{
		man[i].emptyFlag = TRUE;
	}
	return man;
}

//------------------------------------------------------------------
/**
 * セルアニメ管理領域から空きを探す
 *
 * @retval  u32		空きindex
 */
//------------------------------------------------------------------
static inline u32 search_empty_cellanim_pos( void )
{
	u32 i;
	for(i=0; i<SysWork.initParam.CELL_RegisterMax; i++)
	{
		if( SysWork.cellAnimMan[i].emptyFlag )
		{
			return i;
		}
	}
	return GFL_OBJGRP_REGISTER_FAILED;
}

//------------------------------------------------------------------
/**
 * セルアニメデータの登録
 *
 * @param   idx				[in] 登録インデックス
 * @param   cellDataPtr		[in] セルデータポインタ
 * @param   animDataPtr		[in] アニメデータポインタ
 *
 */
//------------------------------------------------------------------
static BOOL register_cellanim( u32 idx, void* cellDataPtr, void* animDataPtr )
{
	CELLANIM_MAN* man = &SysWork.cellAnimMan[idx];

	if(	NNS_G2dGetUnpackedCellBank( cellDataPtr, &(man->cellBankPtr) )
	&&	NNS_G2dGetUnpackedAnimBank( animDataPtr, &(man->animBankPtr) )
	){
		man->emptyFlag = FALSE;
		return TRUE;
	}

	return FALSE;
}

//------------------------------------------------------------------
/**
 * セルアニメ管理領域の破棄
 *
 * @param   man			管理領域ポインタ
 */
//------------------------------------------------------------------
static void delete_cellanim_man( CELLANIM_MAN* man )
{
	GFL_HEAP_FreeMemory( man );
}











