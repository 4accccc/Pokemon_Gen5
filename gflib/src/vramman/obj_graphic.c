//==============================================================================
/**
 *
 *@file		obj_graphic.c
 *@brief	OBJ用グラフィックデータ転送、管理
 *@author	taya
 *@data		2006.11.28
 *
 */
//==============================================================================

#include "assert.h"
#include "heapsys.h"
#include "vman.h"

#include "obj_graphic.h"


//==============================================================
//	CGRデータ管理
//==============================================================
typedef struct {
	NNSG2dImageProxy			proxy;			///< 転送プロキシ
	GFL_VMAN_RESERVE_INFO		riMain;			///< 予約領域情報メイン
	GFL_VMAN_RESERVE_INFO		riSub;			///< 予約領域情報サブ
	BOOL						emptyFlag;		///< 未使用フラグ
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
	NNSG2dCellDataBank*			cellBankPtr;	///< セルデータポインタ
	NNSG2dAnimBankData*			animBankPtr;	///< アニメデータポインタ
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
static CGR_MAN* create_cgr_man( u32 heapID, u32 num );
static inline u32 search_empty_cgr_pos( void );
static BOOL register_cgr( u32 idx, void* dataPtr, GFL_VRAM_TYPE targetVram, NNSG2dCellDataBank* cellBankPtr );
static inline void trans_ncgr( NNSG2dCharacterData* charData, u32 byteOfs, NNS_G2D_VRAM_TYPE vramType, BOOL vramTransferFlag, NNSG2dImageProxy* proxy  );
static void delete_cgr_man( CGR_MAN* man );
static PLTT_MAN* create_pltt_man( u32 heapID, u32 num );
static inline u32 search_empty_pltt_pos( void );
static BOOL register_pltt( u32 idx, void* dataPtr, GFL_VRAM_TYPE vramType, u32 byteOffs );
static void delete_pltt_man( PLTT_MAN* man );
static CELLANIM_MAN* create_cellanim_man( u32 heapID, u32 num );
static inline u32 search_empty_cellanim_pos( void );
static BOOL register_cellanim( u32 idx, void* cellDataPtr, void* animDataPtr );
static void delete_cellanim_man( CELLANIM_MAN* man );


//------------------------------------------------------------------
/**
 * システム初期化（メイン起動時に１回だけ呼ぶ）
 */
//------------------------------------------------------------------
void GFL_OBJGRP_sysInit( void )
{
	SysWork.vmanMain = NULL;
	SysWork.vmanSub = NULL;
	SysWork.cgrMan = NULL;
	SysWork.plttMan = NULL;
	SysWork.cellAnimMan = NULL;
}

void GFL_OBJGRP_sysExit( void )
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
void GFL_OBJGRP_sysStart( u32 heapID, const GF_BGL_DISPVRAM* vramBank, const GFL_OBJGRP_INIT_PARAM* initParam )
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


//==============================================================================================
// データ登録および解放関数群
//==============================================================================================



//==============================================================================================
/**
 * CGRデータの登録（VRAM常駐型OBJ用）
 *
 * CGRデータのVRAM転送を行い、プロキシを作成して保持する。
 * CGRデータの実体は、この関数の後で破棄しても構わない。
 *
 * @param   dataPtr			[in] データポインタ
 * @param   targetVram		[in] 転送先VRAM指定
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//==============================================================================================
u32 GFL_OBJGRP_RegisterCGR( void* dataPtr, GFL_VRAM_TYPE targetVram )
{
	u32  idx = search_empty_cgr_pos();
	if( idx != GFL_OBJGRP_REGISTER_FAILED )
	{
		if( register_cgr( idx, dataPtr, targetVram, NULL ) )
		{
			return idx;
		}
	}
	GF_ASSERT(0);
	return GFL_OBJGRP_REGISTER_FAILED;
}
//==============================================================================================
/**
 * CGRデータの登録（VRAM転送型OBJ用）
 *
 * CGRデータのVRAM転送は行わず、プロキシを作成して保持する。
 * CGRデータの実体はユーザが保持する必要がある。
 * また、VRAM転送型OBJ用のCGR登録をする場合、関連付けられたセルアニメデータが先に登録されている必要がある。
 *
 * @param   dataPtr			[in] データポインタ
 * @param   targetVram		[in] 転送先VRAM指定
 * @param   cellIndex		[in] 既に登録されているセルデータの登録インデックス
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//==============================================================================================
u32 GFL_OBJGRP_RegisterCGR_VramTransfer( void* dataPtr, GFL_VRAM_TYPE targetVram, u32 cellIndex )
{
	GF_ASSERT( SysWork.cellAnimMan[cellIndex].emptyFlag == FALSE );
	GF_ASSERT( NNS_G2dCellDataBankHasVramTransferData( SysWork.cellAnimMan[cellIndex].cellBankPtr ) );

	{
		u32 idx = search_empty_cgr_pos();
		if( idx != GFL_OBJGRP_REGISTER_FAILED )
		{
			if( register_cgr( idx, dataPtr, targetVram, SysWork.cellAnimMan[cellIndex].cellBankPtr ) )
			{
				return idx;
			}
		}
		GF_ASSERT(0);
		return GFL_OBJGRP_REGISTER_FAILED;
	}
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

	SysWork.cgrMan[index].emptyFlag = TRUE;
}



//==============================================================================================
/**
 * セルアニメデータの登録
 *
 * セルデータ＆アニメデータをUnpackしたポインタを保持する。
 * データ実体はユーザが保持する必要がある。
 *
 * @param   cellDataPtr		[in] セルデータ
 * @param   animDataPtr		[in] アニメデータ
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//==============================================================================================
u32 GFL_OBJGRP_RegisterCellAnim( void* cellDataPtr, void* animDataPtr )
{
	u32 idx = search_empty_cellanim_pos();
	if( idx != GFL_OBJGRP_REGISTER_FAILED )
	{
		if( register_cellanim( idx, cellDataPtr, animDataPtr ) == FALSE )
		{
			GF_ASSERT(0);
			return GFL_OBJGRP_REGISTER_FAILED;
		}
	}
	return idx;
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
 * 登録されたセルアニメデータの解放
 *
 * @param   index		[in] 登録インデックス
 */
//==============================================================================================
void GFL_OBJGRP_ReleaseCellAnim( u32 index )
{
	GF_ASSERT( index < SysWork.initParam.CELL_RegisterMax );
	GF_ASSERT( SysWork.cellAnimMan[index].emptyFlag == FALSE );

	SysWork.cellAnimMan[index].emptyFlag = TRUE;
}



//==============================================================================================
/**
 * パレットデータの登録およびVRAMへの転送
 *
 * パレットデータをVRAM転送後、作成したプロキシを保持する。
 * データ実体は破棄しても構わない。
 *
 * @param   plttData		[in] パレットデータ
 * @param   vramType		[in] 転送先VRAM指定
 * @param   byteOffs		[in] 転送オフセット
 *
 * @retval  u32		登録インデックス（登録失敗の場合, GFL_OBJGRP_REGISTER_FAILED）
 */
//==============================================================================================
u32 GFL_OBJGRP_RegisterPltt( void* plttData, GFL_VRAM_TYPE vramType, u32 byteOffs )
{
	u32 idx = search_empty_pltt_pos();
	if( idx != GFL_OBJGRP_REGISTER_FAILED )
	{
		if( register_pltt( idx, plttData, vramType, byteOffs ) == FALSE )
		{
			GF_ASSERT(0);
			return GFL_OBJGRP_REGISTER_FAILED;
		}
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

//-------------------------------------
///	CLWK初期化基本データ
//=====================================
/*
typedef struct {
	s16	pos_x;				// ｘ座標
	s16 pos_y;				// ｙ座標
	u16 anmseq;				// アニメーションシーケンス
	u8	softpri;			// ソフト優先順位	0>0xff
	u8	bgpri;				// BG優先順位
} CLWK_DATA;
*/

//==============================================================================================
// 登録データによるセルアクター構築
//==============================================================================================


CLACT_WORK_PTR GFL_OBJGRP_CreateClAct( CLACT_SET_PTR actset, u32 cgrIndex, u32 plttIndex, u32 cellAnimIndex, const GFL_OBJGRP_CLWKDAT* param, u8 drawArea, u32 heapID )
{
	GF_ASSERT( cgrIndex < SysWork.initParam.CGR_RegisterMax );
	GF_ASSERT( plttIndex < SysWork.initParam.PLTT_RegisterMax );
	GF_ASSERT( cellAnimIndex < SysWork.initParam.CELL_RegisterMax );
	GF_ASSERT( SysWork.cgrMan[cgrIndex].emptyFlag == FALSE );
	GF_ASSERT_MSG( SysWork.plttMan[plttIndex].emptyFlag == FALSE, "plttIndex=%d", plttIndex );
	GF_ASSERT( SysWork.cellAnimMan[cellAnimIndex].emptyFlag == FALSE );

	{
		CLACT_HEADER       header;
		CLACT_ADD_SIMPLE   add;
		CLACT_WORK_PTR     act;

		header.pImageProxy   = &(SysWork.cgrMan[cgrIndex].proxy);
		header.pPaletteProxy = &(SysWork.plttMan[plttIndex].proxy);
		header.pCellBank     = SysWork.cellAnimMan[cellAnimIndex].cellBankPtr;
		header.pAnimBank     = SysWork.cellAnimMan[cellAnimIndex].animBankPtr;
		header.priority      = param->softpri;
		header.pCharData = NULL;
		header.pMCBank   = NULL;
		header.pMCABank  = NULL;
		header.flag      = FALSE;

		add.ClActSet    = actset;
		add.ClActHeader = &header;
		add.mat.x       = param->pos_x * FX32_ONE;
		add.mat.y       = param->pos_y * FX32_ONE;
		add.mat.z       = 0;
		add.pri         = param->softpri;
		add.DrawArea    = drawArea;
		add.heap        = heapID;

		act = CLACT_AddSimple( &add );

		if( act )
		{
			CLACT_SetAnmFlag( act, TRUE );
			CLACT_SetAnmFrame( act, param->anmseq );
		}
		else
		{
			GF_ASSERT(0);
		}
		return act;
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
static CGR_MAN* create_cgr_man( u32 heapID, u32 num )
{
	u32 i;
	CGR_MAN* man = sys_AllocMemory( heapID, sizeof(CGR_MAN)*num );
	for(i=0; i<num; i++)
	{
		man[i].emptyFlag = TRUE;
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
 *
 * @retval  BOOL			成功時TRUE
 */
//------------------------------------------------------------------
static BOOL register_cgr( u32 idx, void* dataPtr, GFL_VRAM_TYPE targetVram, NNSG2dCellDataBank* cellBankPtr )
{
	NNSG2dCharacterData* charData;

	if( NNS_G2dGetUnpackedBGCharacterData( dataPtr, &charData ) )
	{
		CGR_MAN* cgrMan = &SysWork.cgrMan[idx];
		u32 szByte;
		BOOL vramTransferFlag;

		if( cellBankPtr == NULL )
		{
			szByte = charData->szByte;
			vramTransferFlag = FALSE;
		}
		else
		{
			szByte = NNS_G2dGetSizeRequiredVramTransferCellDataBank( cellBankPtr );
			vramTransferFlag = TRUE;
		}

		NNS_G2dInitImageProxy( &cgrMan->proxy );

		if( targetVram & GFL_VRAM_2D_MAIN )
		{
			if( GFL_VMAN_Reserve( SysWork.vmanMain, szByte, &cgrMan->riMain ) )
			{
				u32 byteOfs = GFL_VMAN_GetByteOffset( SysWork.vmanMain, &cgrMan->riMain );
				trans_ncgr( charData, byteOfs, NNS_G2D_VRAM_TYPE_2DMAIN, vramTransferFlag, &cgrMan->proxy );
			}
		}
		if( targetVram & GFL_VRAM_2D_SUB )
		{
			if( GFL_VMAN_Reserve( SysWork.vmanSub, szByte, &cgrMan->riSub ) )
			{
				u32 byteOfs = GFL_VMAN_GetByteOffset( SysWork.vmanSub, &cgrMan->riSub );
				trans_ncgr( charData, byteOfs, NNS_G2D_VRAM_TYPE_2DSUB, vramTransferFlag, &cgrMan->proxy );
			}
		}

		SysWork.cgrMan[idx].emptyFlag = FALSE;

		return TRUE;
	}
	return FALSE;
}
//------------------------------------------------------------------
/**
 * CGR転送コア
 *
 * @param   charData			[in] NITROSystem CharData
 * @param   byteOfs				[in] VRAM先頭からの転送オフセット
 * @param   vramType			[in] 転送先VRAM 2D-Main, 2D-Sub
 * @param   vramTransferFlag	[in] VRAM転送型ならTRUEを指定
 * @param   proxy				[out] 構築するプロキシアドレス
 *
 */
//------------------------------------------------------------------
static inline void trans_ncgr( NNSG2dCharacterData* charData, u32 byteOfs, NNS_G2D_VRAM_TYPE vramType, BOOL vramTransferFlag, NNSG2dImageProxy* proxy  )
{
	if( vramTransferFlag )
	{
			OS_TPrintf("trans cgr VT  ofs:0x%08x, type:%d\n", byteOfs, vramType);
		NNS_G2dLoadImageVramTransfer( charData, byteOfs, vramType, proxy );
	}
	else
	{
		if( charData->mapingType != GX_OBJVRAMMODE_CHAR_2D )
		{
			OS_TPrintf("trans cgr 1d  ofs:0x%08x, type:%d\n", byteOfs, vramType);
			NNS_G2dLoadImage1DMapping( charData, byteOfs, vramType, proxy );
		}
		else
		{
			OS_TPrintf("trans cgr 2d  ofs:0x%08x, type:%d\n", byteOfs, vramType);
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
	sys_FreeMemoryEz( man );
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
static PLTT_MAN* create_pltt_man( u32 heapID, u32 num )
{
	u32 i;
	PLTT_MAN* man = sys_AllocMemory( heapID, sizeof(PLTT_MAN)*num );

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
 * @retval  BOOL			成功時TRUE
 */
//------------------------------------------------------------------
static BOOL register_pltt( u32 idx, void* dataPtr, GFL_VRAM_TYPE vramType, u32 byteOffs )
{
	NNSG2dPaletteData*  palData;
	NNSG2dPaletteCompressInfo*  cmpData;
	BOOL  cmpFlag;

	PLTT_MAN* man = &SysWork.plttMan[idx];

	cmpFlag = NNS_G2dGetUnpackedPaletteCompressInfo( dataPtr, &cmpData );

	if( NNS_G2dGetUnpackedPaletteData( dataPtr, &palData ) )
	{
		NNS_G2dInitImagePaletteProxy( &man->proxy );

		if( cmpFlag )
		{
			if( vramType & GFL_VRAM_2D_MAIN )
			{
				OS_TPrintf("CMP PLTT to MAIN idx:%d, ofs:0x%08x\n", idx, byteOffs);
				NNS_G2dLoadPaletteEx( palData, cmpData, byteOffs, NNS_G2D_VRAM_TYPE_2DMAIN, &man->proxy );
			}
			if( vramType & GFL_VRAM_2D_SUB )
			{
				OS_TPrintf("CMP PLTT to SUB  idx:%d, ofs:0x%08x\n", idx, byteOffs);
				NNS_G2dLoadPaletteEx( palData, cmpData, byteOffs, NNS_G2D_VRAM_TYPE_2DSUB, &man->proxy );
			}
		}
		else
		{
			if( vramType & GFL_VRAM_2D_MAIN )
			{
				OS_TPrintf("NML PLTT to MAIN idx:%d, ofs:0x%08x\n", idx, byteOffs);
				NNS_G2dLoadPalette( palData, byteOffs, NNS_G2D_VRAM_TYPE_2DMAIN, &man->proxy );
			}
			if( vramType & GFL_VRAM_2D_SUB )
			{
				OS_TPrintf("NML PLTT to SUB  idx:%d, ofs:0x%08x\n", idx, byteOffs);
				NNS_G2dLoadPalette( palData, byteOffs, NNS_G2D_VRAM_TYPE_2DSUB, &man->proxy );
			}
		}

		SysWork.plttMan[idx].emptyFlag = FALSE;

		return TRUE;
	}

	return FALSE;
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
	sys_FreeMemoryEz( man );
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
static CELLANIM_MAN* create_cellanim_man( u32 heapID, u32 num )
{
	u32 i;
	CELLANIM_MAN* man = sys_AllocMemory( heapID, sizeof(CELLANIM_MAN)*num );

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
	sys_FreeMemoryEz( man );
}

