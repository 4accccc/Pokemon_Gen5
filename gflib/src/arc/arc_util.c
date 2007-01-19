//============================================================================================
/**
 * @file	arc_util.c
 * @bfief	アーカイブデータを便利に使うためのユーティリティ関数群
 * @author	taya
 * @date	05.08.30
 */
//============================================================================================
#include "gflib.h"

#include "arc_tool.h"
#include "arc_util.h"


//==============================================================
// Prototype
//==============================================================

vu32	*chr=NULL;
vu32	*scr=NULL;

//------------------------------------------------------------------
/**
 * BGｷｬﾗﾃﾞｰﾀの VRAM 転送
 *
 * @param   fileIdx			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   dataIndex		ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   frm				転送先ﾌﾚｰﾑﾅﾝﾊﾞ
 * @param   offs			転送ｵﾌｾｯﾄ（ｷｬﾗ単位）
 * @param	transSize		転送するｻｲｽﾞ（ﾊﾞｲﾄ単位 ==0で全転送）
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   heapID			ﾃﾞｰﾀ読み込み・解凍ﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 * @return  転送したデータサイズ（バイト）
 */
//------------------------------------------------------------------
u32 GFL_ARC_UtilBgCharSet(u32 fileIdx, u32 dataIdx, u32 frm, u32 offs, u32 transSize, BOOL compressedFlag, u32 heapID)
{
	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, compressedFlag, heapID );

	chr=arcData;

	{
		NNSG2dCharacterData* charData;

		if( NNS_G2dGetUnpackedBGCharacterData( arcData, &charData ) )
		{
			if( transSize == 0 )
			{
				transSize = charData->szByte;
			}
			GFL_BG_LoadCharacter(frm, charData->pRawData, transSize, offs);
		}

		GFL_HEAP_FreeMemory( arcData );
	}

    return transSize;
}

//------------------------------------------------------------------
/**
 * BGｷｬﾗﾃﾞｰﾀの VRAM 転送（エリアマネージャを使用して開いている領域に自動で転送）
 *
 * @param   fileIdx			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   dataIndex		ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   frm				転送先ﾌﾚｰﾑﾅﾝﾊﾞ
 * @param	transSize		転送するｻｲｽﾞ（ﾊﾞｲﾄ単位 ==0で全転送）
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   heapID			ﾃﾞｰﾀ読み込み・解凍ﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 * @return  転送した位置
 */
//------------------------------------------------------------------
u32 GFL_ARC_UtilBgCharSetAreaMan(u32 fileIdx, u32 dataIdx, u32 frm, u32 transSize, BOOL compressedFlag, u32 heapID)
{
	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, compressedFlag, heapID );
	u32	offs;

	chr=arcData;

	{
		NNSG2dCharacterData* charData;

		if( NNS_G2dGetUnpackedBGCharacterData( arcData, &charData ) )
		{
			if( transSize == 0 )
			{
				transSize = charData->szByte;
			}
			offs = GFL_BG_LoadCharAreaSet( frm, charData->pRawData, transSize );
		}

		GFL_HEAP_FreeMemory( arcData );
	}

	if(GFL_BG_ScreenColorModeGet( frm ) == GX_BG_COLORMODE_16 ){
		return offs;
	}
	else{
		return offs/2;
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ｽｸﾘｰﾝﾃﾞｰﾀの VRAM 転送
 * ※ BGL側に ｽｸﾘｰﾝﾊﾞｯﾌｧ が用意されていれば、ｽｸﾘｰﾝﾊﾞｯﾌｧ への転送も行う
 *
 * @param   fileIdx			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   arcIndex		ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   frm				転送先ﾌﾚｰﾑﾅﾝﾊﾞ
 * @param   offs			転送ｵﾌｾｯﾄ（ｷｬﾗ単位）
 * @param	transSize		転送するｻｲｽﾞ（ﾊﾞｲﾄ単位 ==0で全転送）
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   heapID			ﾃﾞｰﾀ読み込み・解凍ﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 */
//--------------------------------------------------------------------------------------------
void GFL_ARC_UtilScrnSet(u32 fileIdx, u32 dataIdx, u32 frm, u32 offs, u32 transSize, BOOL compressedFlag, u32 heapID)
{
	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, compressedFlag, heapID );

	scr=arcData;

	{
		NNSG2dScreenData* scrnData;

		if( NNS_G2dGetUnpackedScreenData( arcData, &scrnData ) )
		{
			if( transSize == 0 )
			{
				transSize = scrnData->szByte;
			}

			if( GFL_BG_ScreenAdrsGet( frm ) != NULL )
			{
				GFL_BG_ScreenBufSet( frm, scrnData->rawData, transSize );
			}
			GFL_BG_ScrAreaSet( frm, offs, transSize );
			GFL_BG_LoadScreen( frm, scrnData->rawData, transSize, offs );
		}
		GFL_HEAP_FreeMemory( arcData );
	}
}

//--------------------------------------------------------------------------------------------
/**
 * ｽｸﾘｰﾝﾃﾞｰﾀの VRAM 転送（キャラのオフセットも指定可）
 * ※ BGL側に ｽｸﾘｰﾝﾊﾞｯﾌｧ が用意されていれば、ｽｸﾘｰﾝﾊﾞｯﾌｧ への転送も行う
 *
 * @param   fileIdx			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   arcIndex		ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   frm				転送先ﾌﾚｰﾑﾅﾝﾊﾞ
 * @param   offs			転送ｵﾌｾｯﾄ（ｷｬﾗ単位）
 * @param	transSize		転送するｻｲｽﾞ（ﾊﾞｲﾄ単位 ==0で全転送）
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   heapID			ﾃﾞｰﾀ読み込み・解凍ﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 */
//--------------------------------------------------------------------------------------------
void GFL_ARC_UtilScrnSetCharOfs(u32 fileIdx, u32 dataIdx, u32 frm, u32 offs, u32 chr_ofs, u32 transSize, BOOL compressedFlag, u32 heapID)
{
	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, compressedFlag, heapID );

	scr=arcData;

	{
		NNSG2dScreenData* scrnData;

		if( NNS_G2dGetUnpackedScreenData( arcData, &scrnData ) )
		{
			if( transSize == 0 )
			{
				transSize = scrnData->szByte;
			}

			if( GFL_BG_ScreenAdrsGet( frm ) != NULL )
			{
				GFL_BG_ScreenBufSet( frm, scrnData->rawData, transSize );
			}
			if(chr_ofs){
				if(GFL_BG_ScreenColorModeGet(frm) == GX_BG_COLORMODE_16 ){
					{
						int	i;
						u16	*scr;

						scr=(u16 *)&scrnData->rawData;

						for(i=0;i<transSize/2;i++){
							scr[i]+=chr_ofs;
						}
					}
				}
				else{
					{
						int	i;
						u8	*scr;

						scr=(u8 *)&scrnData->rawData;

						for(i=0;i<transSize;i++){
							scr[i]+=chr_ofs;
						}
					}
				}
			}
			GFL_BG_ScrAreaSet( frm, offs, transSize );
			GFL_BG_LoadScreen( frm, scrnData->rawData, transSize, offs );
		}
		GFL_HEAP_FreeMemory( arcData );
	}
}

//------------------------------------------------------------------
/**
 * ﾊﾟﾚｯﾄﾃﾞｰﾀ の VRAM 転送
 *
 * @param   fileIdx		ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   dataIdx		ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   palType		ﾊﾟﾚｯﾄ転送先ﾀｲﾌﾟ
 * @param   offs		ﾊﾟﾚｯﾄ転送先ｵﾌｾｯﾄ
 * @param   transSize	ﾊﾟﾚｯﾄ転送ｻｲｽﾞ（0 で全転送）
 * @param   heapID		ﾃﾞｰﾀ読み込みﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 */
//------------------------------------------------------------------
void GFL_ARC_UtilPalSet( u32 fileIdx, u32 dataIdx, PALTYPE palType, u32 offs, u32 transSize, u32 heapID )
{
	GFL_ARC_UtilPalSetEx( fileIdx, dataIdx, palType, 0, offs, transSize, heapID );
}

//------------------------------------------------------------------
/**
 * ﾊﾟﾚｯﾄﾃﾞｰﾀ の VRAM 転送（転送元の読み込み開始ｵﾌｾｯﾄ指定版）
 *
 * @param   fileIdx		ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   dataIdx		ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   palType		ﾊﾟﾚｯﾄ転送先ﾀｲﾌﾟ
 * @param   srcOfs		ﾊﾟﾚｯﾄ転送元読み込み開始ｵﾌｾｯﾄ
 * @param   dstOfs		ﾊﾟﾚｯﾄ転送先ｵﾌｾｯﾄ
 * @param   transSize	ﾊﾟﾚｯﾄ転送ｻｲｽﾞ（0 で全転送）
 * @param   heapID		ﾃﾞｰﾀ読み込みﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 */
//------------------------------------------------------------------
void GFL_ARC_UtilPalSetEx( u32 fileIdx, u32 dataIdx, PALTYPE palType, u32 srcOfs, u32 dstOfs, u32 transSize, u32 heapID )
{
		static void (* const load_func[])(const void*, u32, u32) = {
		GX_LoadBGPltt,
		GX_LoadOBJPltt,
		GX_LoadBGExtPltt,
		GX_LoadOBJExtPltt,
		GXS_LoadBGPltt,
		GXS_LoadOBJPltt,
		GXS_LoadBGExtPltt,
		GXS_LoadOBJExtPltt,
	};

	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, FALSE, heapID );

	if( arcData != NULL )
	{
		NNSG2dPaletteData* palData;

		if( NNS_G2dGetUnpackedPaletteData( arcData, &palData ) )
		{
			(u8*)(palData->pRawData) += srcOfs;
			if( transSize == 0 )
			{
				transSize = palData->szByte - srcOfs;
			}
			DC_FlushRange( palData->pRawData, transSize );

			switch( palType ){
			case PALTYPE_MAIN_BG_EX:
				GX_BeginLoadBGExtPltt();
				load_func[ palType ]( palData->pRawData, dstOfs, transSize );
				GX_EndLoadBGExtPltt();
				break;

			case PALTYPE_SUB_BG_EX:
				GXS_BeginLoadBGExtPltt();
				load_func[ palType ]( palData->pRawData, dstOfs, transSize );
				GXS_EndLoadBGExtPltt();
				break;

			case PALTYPE_MAIN_OBJ_EX:
				GX_BeginLoadOBJExtPltt();
				load_func[ palType ]( palData->pRawData, dstOfs, transSize );
				GX_EndLoadOBJExtPltt();
				break;

			case PALTYPE_SUB_OBJ_EX:
				GXS_BeginLoadOBJExtPltt();
				load_func[ palType ]( palData->pRawData, dstOfs, transSize );
				GXS_EndLoadOBJExtPltt();
				break;

			default:
				load_func[ palType ]( palData->pRawData, dstOfs, transSize );
				break;
			}
		}

		GFL_HEAP_FreeMemory( arcData );
	}
}

//------------------------------------------------------------------
/**
 * OBJ ｷｬﾗﾃﾞｰﾀ の VRAM 転送
 *
 * @param   fileIdx				ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   dataIdx				ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   objType				OBJﾀｲﾌﾟ(OBJTYPE_MAIN or OBJTYPE_SUB）
 * @param   offs				ｵﾌｾｯﾄ（ﾊﾞｲﾄ単位）
 * @param   transSize			転送ｻｲｽﾞ（ﾊﾞｲﾄ単位 : 0 で全転送）
 * @param   compressedFlag		圧縮されたﾃﾞｰﾀか？
 * @param   heapID				読み込み・解凍ﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 * @return  転送したデータサイズ（バイト）
 */
//------------------------------------------------------------------
u32 GFL_ARC_UtilObjCharSet( u32 fileIdx, u32 dataIdx, OBJTYPE objType, u32 offs, u32 transSize, BOOL compressedFlag, u32 heapID )
{
	static void (* const load_func[])(const void*, u32, u32) = {
		GX_LoadOBJ,
		GXS_LoadOBJ,
	};

	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, compressedFlag, heapID );

	if( arcData != NULL )
	{
		NNSG2dCharacterData* charData;

		if( NNS_G2dGetUnpackedCharacterData( arcData, &charData ) )
		{
			if( transSize == 0 )
			{
				transSize = charData->szByte;
			}

			DC_FlushRange( charData->pRawData, transSize );
			load_func[ objType ]( charData->pRawData, offs, transSize );
		}
		GFL_HEAP_FreeMemory( arcData );
	}
	return transSize;
}

//------------------------------------------------------------------
/**
 * ﾊﾟﾚｯﾄﾃﾞｰﾀ の VRAM 転送＆ NITRO System ﾊﾟﾚｯﾄﾌﾟﾛｷｼ を作成
 *（3D, OBJ 用にのみ対応。BG には使いません）
 *
 * @param   fileIdx		[in] ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   dataIdx		[in] ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   type		[in] 転送先ﾀｲﾌﾟ
 * @param   offs		[in] 転送ｵﾌｾｯﾄ
 * @param   heapID		[in] ﾋｰﾌﾟID
 * @param   proxy		[out]作成するﾌﾟﾛｷｼｱﾄﾞﾚｽ
 *
 *	[ type ]
 *		NNS_G2D_VRAM_TYPE_3DMAIN = 0,
 *		NNS_G2D_VRAM_TYPE_2DMAIN = 1,
 *		NNS_G2D_VRAM_TYPE_2DSUB  = 2,
 *
 */
//------------------------------------------------------------------
void GFL_ARC_UtilPalSysLoad( u32 fileIdx, u32 dataIdx, NNS_G2D_VRAM_TYPE type, u32 offs, u32 heapID, NNSG2dImagePaletteProxy* proxy )
{
	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, FALSE, heapID );

	if( arcData != NULL )
	{
		NNSG2dPaletteData*  palData;
		NNSG2dPaletteCompressInfo*  cmpData;
		BOOL  cmpFlag;

		cmpFlag = NNS_G2dGetUnpackedPaletteCompressInfo( arcData, &cmpData );

		if( NNS_G2dGetUnpackedPaletteData( arcData, &palData ) )
		{
			if( cmpFlag )
			{
				NNS_G2dLoadPaletteEx( palData, cmpData, offs, type, proxy );
			}
			else
			{
				NNS_G2dLoadPalette( palData, offs, type, proxy );
			}
		}

		GFL_HEAP_FreeMemory( arcData );
	}
}
//------------------------------------------------------------------
/**
 * ｷｬﾗﾃﾞｰﾀ の VRAM 転送＆ NITRO System ｲﾒｰｼﾞﾌﾟﾛｷｼ を作成
 *（3D, OBJ 用にのみ対応。BG には使いません）
 *
 *
 * @param   fileIdx			[in] ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   dataIdx			[in] ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	[in] 圧縮されているか
 * @param   mapType			[in] ﾏｯﾋﾟﾝｸﾞﾀｲﾌﾟ
 * @param   transSize		[in] 転送サイズ。０なら全転送。
 * @param   vramType		[in] 転送先ﾀｲﾌﾟ
 * @param   offs			[in] 転送ｵﾌｾｯﾄ
 * @param   heapID			[in] ﾃﾝﾎﾟﾗﾘに使うﾋｰﾌﾟID
 * @param   proxy			[out] 作成するﾌﾟﾛｷｼｱﾄﾞﾚｽ
 *
 *
 *	[ mapType ]
 *		CHAR_MAP_1D = 0,
 *		CHAR_MAP_2D = 1,
 *
 *	※ VRAM転送型は別関数 GFL_ARC_UtilTranCharSysLoad を使う
 *
 *	[ vramType ]
 *		NNS_G2D_VRAM_TYPE_3DMAIN = 0,
 *		NNS_G2D_VRAM_TYPE_2DMAIN = 1,
 *		NNS_G2D_VRAM_TYPE_2DSUB  = 2,
 *
 */
//------------------------------------------------------------------
u32 GFL_ARC_UtilCharSysLoad( u32 fileIdx, u32 dataIdx, BOOL compressedFlag, CHAR_MAPPING_TYPE mapType, u32 transSize,
	NNS_G2D_VRAM_TYPE vramType, u32 offs, u32 heapID, NNSG2dImageProxy* proxy )
{
	static void (* const load_func[])(const NNSG2dCharacterData*, u32, NNS_G2D_VRAM_TYPE, NNSG2dImageProxy*) = {
		NNS_G2dLoadImage1DMapping,
		NNS_G2dLoadImage2DMapping,
	};

	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, compressedFlag, heapID );
	u32  transed_size = 0;

	if( arcData != NULL )
	{
		NNSG2dCharacterData* charData;

		if( NNS_G2dGetUnpackedCharacterData( arcData, &charData ) )
		{
			if( transSize )
			{
				charData->szByte = transSize;
			}
			load_func[ mapType ]( charData, offs, vramType, proxy );
			transed_size = charData->szByte;
		}
		GFL_HEAP_FreeMemory( arcData );
	}

	return transed_size;
}
//------------------------------------------------------------------
/**
 * ｷｬﾗﾃﾞｰﾀ の VRAM 転送＆ NITRO System ｲﾒｰｼﾞﾌﾟﾛｷｼ を作成
 *（3D, OBJ 用にのみ対応。BG には使いません）
 *
 * ｷｬﾗﾃﾞｰﾀのﾏｯﾋﾟﾝｸﾞﾓｰﾄﾞ値を、現在のﾚｼﾞｽﾀ設定に合わせて書き換えます
 *
 * @param   fileIdx			[in] ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   dataIdx			[in] ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	[in] 圧縮されているか
 * @param   mapType			[in] ﾏｯﾋﾟﾝｸﾞﾀｲﾌﾟ
 * @param   transSize		[in] 転送サイズ。０なら全転送。
 * @param   vramType		[in] 転送先ﾀｲﾌﾟ
 * @param   offs			[in] 転送ｵﾌｾｯﾄ
 * @param   heapID			[in] ﾃﾝﾎﾟﾗﾘに使うﾋｰﾌﾟID
 * @param   proxy			[out] 作成するﾌﾟﾛｷｼｱﾄﾞﾚｽ
 *
 *
 *	[ mapType ]
 *		CHAR_MAP_1D = 0,
 *		CHAR_MAP_2D = 1,
 *
 *	※ VRAM転送型は別関数 GFL_ARC_UtilTranCharSysLoad を使う
 *
 *	[ vramType ]
 *		NNS_G2D_VRAM_TYPE_3DMAIN = 0,
 *		NNS_G2D_VRAM_TYPE_2DMAIN = 1,
 *		NNS_G2D_VRAM_TYPE_2DSUB  = 2,
 *
 */
//------------------------------------------------------------------
void GFL_ARC_UtilCharSysLoadSyncroMappingMode( u32 fileIdx, u32 dataIdx, BOOL compressedFlag, CHAR_MAPPING_TYPE mapType, u32 transSize,
	NNS_G2D_VRAM_TYPE vramType, u32 offs, u32 heapID, NNSG2dImageProxy* proxy )
{
	static void (* const load_func[])(const NNSG2dCharacterData*, u32, NNS_G2D_VRAM_TYPE, NNSG2dImageProxy*) = {
		NNS_G2dLoadImage1DMapping,
		NNS_G2dLoadImage2DMapping,
	};

	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, compressedFlag, heapID );

	if( arcData != NULL )
	{
		NNSG2dCharacterData* charData;

		if( NNS_G2dGetUnpackedCharacterData( arcData, &charData ) )
		{
			if( transSize )
			{
				charData->szByte = transSize;
			}

			switch( vramType ){
			case NNS_G2D_VRAM_TYPE_2DMAIN:
				charData->mapingType = GX_GetOBJVRamModeChar();
				break;
			case NNS_G2D_VRAM_TYPE_2DSUB:
				charData->mapingType = GXS_GetOBJVRamModeChar();
				break;
			}
			load_func[ mapType ]( charData, offs, vramType, proxy );
		}
		GFL_HEAP_FreeMemory( arcData );
	}
}

//------------------------------------------------------------------
/**
 * ｷｬﾗﾃﾞｰﾀ の VRAM 転送＆ NITRO System ｲﾒｰｼﾞﾌﾟﾛｷｼ を作成。VRAM転送型の画像素材用。
 *
 * ※ この関数を使っても、VRAMに画像は転送されません
 *    この関数でﾛｰﾄﾞしたﾃﾞｰﾀは解放されません。戻り値のconst void*を管理して、
 *    不要になったら解放処理を行ってください。
 *
 * @param   fileIdx			[in] ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   dataIdx			[in] ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	[in] 圧縮されているか
 * @param   vramType		[in] 転送先ﾀｲﾌﾟ
 * @param   offs			[in] 転送ｵﾌｾｯﾄ
 * @param   heapID			[in] ﾃﾝﾎﾟﾗﾘに使うﾋｰﾌﾟID
 * @param   proxy			[out] 作成するﾌﾟﾛｷｼのｱﾄﾞﾚｽ
 * @param   charData		[out] ｷｬﾗﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 *
 * @retval  const void*		ﾛｰﾄﾞしたﾃﾞｰﾀのｱﾄﾞﾚｽ
 *
 *	[ vramType ]
 *		NNS_G2D_VRAM_TYPE_3DMAIN = 0,
 *		NNS_G2D_VRAM_TYPE_2DMAIN = 1,
 *		NNS_G2D_VRAM_TYPE_2DSUB  = 2,
 *
 */
//------------------------------------------------------------------
const void* GFL_ARC_UtilTransTypeCharSysLoad( u32 fileIdx, u32 dataIdx, BOOL compressedFlag, 
	NNS_G2D_VRAM_TYPE vramType, u32 offs, u32 heapID, NNSG2dImageProxy* proxy, NNSG2dCharacterData** charData )
{
	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, compressedFlag, heapID );

	if( arcData != NULL )
	{
		if( NNS_G2dGetUnpackedCharacterData( arcData, charData ) )
		{
			NNS_G2dLoadImageVramTransfer( *charData, offs, vramType, proxy );
		}
	}
	return arcData;
}



//------------------------------------------------------------------
/**
 * ｷｬﾗﾃﾞｰﾀを ﾛｰﾄﾞして Unpack するだけです。解放は各自で。
 *
 * @param   fileIdx				[in] ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   dataIdx				[in] ﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag		[in] 圧縮されているか
 * @param   charData			[out] ｷｬﾗﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 * @param   heapID				[in] ﾋｰﾌﾟID
 *
 * @retval  void*		ﾛｰﾄﾞしたﾃﾞｰﾀの先頭ﾎﾟｲﾝﾀ
 */
//------------------------------------------------------------------
void* GFL_ARC_UtilCharDataGet( u32 fileIdx, u32 dataIdx, BOOL compressedFlag, NNSG2dCharacterData** charData, u32 heapID )
{
	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, compressedFlag, heapID );

	if( arcData != NULL )
	{
		if( NNS_G2dGetUnpackedBGCharacterData( arcData, charData ) == FALSE)
		{
			// 失敗したらNULL
			GFL_HEAP_FreeMemory( arcData );
			return NULL;
		}
	}
	return arcData;
}
//--------------------------------------------------------------------------------------------
/**
 * ｽｸﾘｰﾝﾃﾞｰﾀを ﾛｰﾄﾞして Unpack するだけです。解放は各自で。
 *
 * @param   fileIdx			[in] ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   arcIndex		[in] ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	[in] 圧縮されているﾃﾞｰﾀか？
 * @param   scrnData		[out] ｽｸﾘｰﾝﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 * @param   heapID			[in] ﾋｰﾌﾟID
 *
 * @retval  void*		ﾛｰﾄﾞしたﾃﾞｰﾀの先頭ﾎﾟｲﾝﾀ
 */
//--------------------------------------------------------------------------------------------
void* GFL_ARC_UtilScrnDataGet(u32 fileIdx, u32 dataIdx, BOOL compressedFlag, NNSG2dScreenData** scrnData, u32 heapID)
{
	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, compressedFlag, heapID );

	if( arcData != NULL )
	{
		if( NNS_G2dGetUnpackedScreenData( arcData, scrnData ) == FALSE )
		{
			GFL_HEAP_FreeMemory( arcData );
			return NULL;
		}
	}
	return arcData;
}
//------------------------------------------------------------------
/**
 * ﾊﾟﾚｯﾄﾃﾞｰﾀをﾛｰﾄﾞして Unpack するだけです。解放は各自で。
 *
 * @param   fileIdx			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   arcIndex		ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   palData			ﾊﾟﾚｯﾄﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 * @param   heapID			ﾋｰﾌﾟID
 *
 * @retval  void*		ﾛｰﾄﾞしたﾃﾞｰﾀの先頭ﾎﾟｲﾝﾀ
 */
//------------------------------------------------------------------
void* GFL_ARC_UtilPalDataGet( u32 fileIdx, u32 dataIdx, NNSG2dPaletteData** palData, u32 heapID )
{
	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, FALSE, heapID );

	if( arcData != NULL )
	{
		if( NNS_G2dGetUnpackedPaletteData( arcData, palData ) == FALSE )
		{
			GFL_HEAP_FreeMemory( arcData );
			return NULL;
		}
	}
	return arcData;
}
//--------------------------------------------------------------------------------------------
/**
 * ｾﾙﾊﾞﾝｸﾃﾞｰﾀを ﾛｰﾄﾞして Unpack するだけです。解放は各自で。
 *
 * @param   fileIdx			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   arcIndex		ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   cellBank		ｾﾙﾊﾞﾝｸﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 * @param   heapID			ﾋｰﾌﾟID
 * @param   pSize			ｾﾙﾊﾞﾝｸのﾊﾞｲﾄｻｲｽﾞを受け取る変数ﾎﾟｲﾝﾀ（要らなければNULLで）
 *
 * @retval  void*		ﾛｰﾄﾞしたﾃﾞｰﾀの先頭ﾎﾟｲﾝﾀ
 */
//--------------------------------------------------------------------------------------------
void* GFL_ARC_UtilCellBankDataGet(u32 fileIdx, u32 dataIdx, BOOL compressedFlag, NNSG2dCellDataBank** cellBank, u32 heapID )
{
	void* arcData;

	arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, compressedFlag, heapID );

	if( arcData != NULL )
	{
		if( NNS_G2dGetUnpackedCellBank( arcData, cellBank ) == FALSE )
		{
			GFL_HEAP_FreeMemory( arcData );
			return NULL;
		}
	}
	return arcData;
}
//--------------------------------------------------------------------------------------------
/**
 * ｱﾆﾒﾊﾞﾝｸﾃﾞｰﾀを ﾛｰﾄﾞして Unpack するだけです。解放は各自で。
 *
 * @param   fileIdx			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   arcIndex		ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   anmBank			ｱﾆﾒﾊﾞﾝｸﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 * @param   heapID			ﾋｰﾌﾟID
 *
 * @retval  void*		ﾛｰﾄﾞしたﾃﾞｰﾀの先頭ﾎﾟｲﾝﾀ
 */
//--------------------------------------------------------------------------------------------
void* GFL_ARC_UtilAnimBankDataGet(u32 fileIdx, u32 dataIdx, BOOL compressedFlag, NNSG2dAnimBankData** anmBank, u32 heapID)
{
	void* arcData = GFL_ARC_UtilLoad( fileIdx, dataIdx, compressedFlag, heapID );

	if( arcData != NULL )
	{
		if( NNS_G2dGetUnpackedAnimBank( arcData, anmBank ) == FALSE )
		{
			GFL_HEAP_FreeMemory( arcData );
			return NULL;
		}
	}
	return arcData;
}











//------------------------------------------------------------------
/**
 * LZ圧縮後アーカイブされているデータを読み出し、解凍して返す
 *
 * @param   fileIdx		アーカイブファイルインデックス
 * @param   dataIdx		アーカイブデータインデックス
 * @param   heapID		読み出し・解凍に使うヒープＩＤ
 *
 * @retval  void*		解凍後のデータ保存先アドレス
 */
//------------------------------------------------------------------
void* GFL_ARC_UtilUnCompress(u32 fileIdx, u32 dataIdx, u32 heapID)
{
	return GFL_ARC_UtilLoad(fileIdx, dataIdx, TRUE, heapID );
}

//------------------------------------------------------------------
/**
 * アーカイブデータの読み出し（メモリ確保する）
 *
 * @param   fileIdx			アーカイブファイルインデックス
 * @param   dataIdx			アーカイブデータインデックス
 * @param   compressedFlag	圧縮されているか？
 * @param   heapID			メモリ確保に使うヒープＩＤ
 *
 * @retval  void*			読み出し領域ポインタ
 */
//------------------------------------------------------------------
void* GFL_ARC_UtilLoad(u32 fileIdx, u32 dataIdx, BOOL compressedFlag, u32 heapID )
{
	void* arcData;

	if( compressedFlag )
	{
		arcData = GFL_HEAP_AllocMemoryLow( heapID, GFL_ARC_DataSizeGet(fileIdx, dataIdx) );
	}
	else
	{
		arcData = GFL_HEAP_AllocMemory( heapID, GFL_ARC_DataSizeGet(fileIdx, dataIdx) );
	}

	GFL_ARC_DataLoad(arcData, fileIdx, dataIdx);
	if( compressedFlag )
	{
		void* data;

		data = GFL_HEAP_AllocMemory( heapID, MI_GetUncompressedSize( arcData ) );

		MI_UncompressLZ8( arcData, data );
		GFL_HEAP_FreeMemory( arcData );

		arcData = data;
	}
	return arcData;
}
//------------------------------------------------------------------
/**
 * アーカイブデータの読み出し（読み出したサイズも取得できる）
 *
 * @param   fileIdx			アーカイブファイルインデックス
 * @param   dataIdx			アーカイブデータインデックス
 * @param   compressedFlag	圧縮されているか？
 * @param   heapID			メモリ確保に使うヒープＩＤ
 * @param   pSize			実データのバイトサイズ（圧縮されている場合は展開後）
 *
 * @retval  void*			読み出し領域ポインタ
 */
//------------------------------------------------------------------
void* GFL_ARC_UtilLoadEx(u32 fileIdx, u32 dataIdx, BOOL compressedFlag, u32 heapID, u32* pSize)
{
	void* arcData;

	*pSize = GFL_ARC_DataSizeGet(fileIdx, dataIdx);

	if( compressedFlag )
	{
		arcData = GFL_HEAP_AllocMemoryLow( heapID, *pSize );
	}
	else
	{
		arcData = GFL_HEAP_AllocMemory( heapID, *pSize );
	}

	GFL_ARC_DataLoad(arcData, fileIdx, dataIdx);
	if( compressedFlag )
	{
		void* data;

		*pSize = MI_GetUncompressedSize( arcData );

		data = GFL_HEAP_AllocMemory( heapID, *pSize );

		MI_UncompressLZ8( arcData, data );
		GFL_HEAP_FreeMemory( arcData );

		arcData = data;
	}
	return arcData;
}
