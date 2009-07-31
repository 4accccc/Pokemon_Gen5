//============================================================================================
/**
 * @file	arc_util.h
 * @bfief	アーカイブデータを便利に使うためのユーティリティ関数群
 * @author	taya
 * @date	05.08.30
 */
//============================================================================================
#ifndef __ARC_UTIL_H__
#define __ARC_UTIL_H__

#include "bg_sys.h"
#include "arc_tool.h"

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------
/**
 *  パレット転送先タイプ指定
 */
//------------------------------------------------------------------
typedef enum {
	PALTYPE_MAIN_BG,			// メインBG
	PALTYPE_MAIN_OBJ,			// メインOBJ
	PALTYPE_MAIN_BG_EX,			// メイン拡張BG
	PALTYPE_MAIN_OBJ_EX,		// メイン拡張OBJ
	PALTYPE_SUB_BG,				// サブBG
	PALTYPE_SUB_OBJ,			// サブOBJ
	PALTYPE_SUB_BG_EX,			// サブ拡張BG
	PALTYPE_SUB_OBJ_EX,			// サブ拡張OBJ
}PALTYPE;

//------------------------------------------------------------------
/**
 *  OBJキャラデータ転送先タイプ指定
 */
//------------------------------------------------------------------
typedef enum {
	OBJTYPE_MAIN,
	OBJTYPE_SUB,
}OBJTYPE;

//------------------------------------------------------------------
/**
 *  キャラデータマッピングタイプ
 */
//------------------------------------------------------------------
typedef enum {
	CHAR_MAP_1D,		// 1D 
	CHAR_MAP_2D,		// 2D
}CHAR_MAPPING_TYPE;


//------------------------------------------------------------------
/**
 *  データ転送位置情報
 */
//------------------------------------------------------------------
typedef u32 GFL_ARCUTIL_TRANSINFO;

enum {
	GFL_ARCUTIL_TRANSINFO_FAIL = 0,	///< 転送失敗時に返る
};

/**
 *  転送位置情報：生成
 */
static inline GFL_ARCUTIL_TRANSINFO  GFL_ARCUTIL_TRANSINFO_Make( u16 pos, u16 size )
{
	return (size << 16) | pos;
}
/**
 *  転送位置情報：位置取り出し
 */
static inline u16 GFL_ARCUTIL_TRANSINFO_GetPos( GFL_ARCUTIL_TRANSINFO info )
{
	return info & 0xffff;
}
/**
 *  転送位置情報：サイズ取り出し
 */
static inline u16 GFL_ARCUTIL_TRANSINFO_GetSize( GFL_ARCUTIL_TRANSINFO info )
{
	return (info >> 16) & 0xffff;
}


//------------------------------------------------------------------
/**
 * BGｷｬﾗﾃﾞｰﾀの VRAM 転送
 *
 * @param   arcID			アーカイブファイルインデックス
 * @param   datID			アーカイブデータインデックス
 * @param   frm				転送先ﾌﾚｰﾑﾅﾝﾊﾞ
 * @param   offs			転送ｵﾌｾｯﾄ（ｷｬﾗ単位）
 * @param	transSize		転送するｻｲｽﾞ（ﾊﾞｲﾄ単位 ==0で全転送）
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   heapID			ﾃﾞｰﾀ読み込み・解凍ﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 * @return  転送したデータサイズ（バイト）
 */
//------------------------------------------------------------------
extern u32 GFL_ARC_UTIL_TransVramBgCharacter(ARCID arcID, ARCDATID datID, u32 frm, u32 offs, u32 transSize, BOOL compressedFlag, HEAPID heapID);
extern u32 GFL_ARCHDL_UTIL_TransVramBgCharacter(ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 offs, u32 transSize, BOOL compressedFlag, HEAPID heapID);

//------------------------------------------------------------------
/**
 * BGｷｬﾗﾃﾞｰﾀの VRAM 転送（エリアマネージャを使用して開いている領域に自動で転送）
 *
 * @param   arcID			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID			ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   frm				転送先ﾌﾚｰﾑﾅﾝﾊﾞ
 * @param	transSize		転送するｻｲｽﾞ（ﾊﾞｲﾄ単位 ==0で全転送）
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   heapID			ﾃﾞｰﾀ読み込み・解凍ﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 * @return  確保した領域情報（bg_sys）
 */
//------------------------------------------------------------------
extern	GFL_ARCUTIL_TRANSINFO  GFL_ARC_UTIL_TransVramBgCharacterAreaMan(ARCID arcID, ARCDATID datID, u32 frm, u32 transSize, BOOL compressedFlag, HEAPID heapID);
extern	GFL_ARCUTIL_TRANSINFO  GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan(ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 transSize, BOOL compressedFlag, HEAPID heapID);

//------------------------------------------------------------------
/**
 * OBJ ｷｬﾗﾃﾞｰﾀ の VRAM 転送
 *
 * @param   arcID				アーカイブファイルインデックス
 * @param   datID				アーカイブデータインデックス
 * @param   objType				OBJﾀｲﾌﾟ
 * @param   offs				ｵﾌｾｯﾄ（ﾊﾞｲﾄ単位）
 * @param   transSize			転送ｻｲｽﾞ（ﾊﾞｲﾄ単位 : 0 で全転送）
 * @param   compressedFlag		圧縮されたﾃﾞｰﾀか？
 * @param   heapID				読み込み・解凍ﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 * @return  転送したデータサイズ（バイト）
 */
//------------------------------------------------------------------
extern u32 GFL_ARC_UTIL_TransVramObjCharacter( ARCID arcID, ARCDATID datID, OBJTYPE objType, u32 offs, u32 transSize, BOOL compressedFlag, HEAPID heapID );
extern u32 GFL_ARCHDL_UTIL_TransVramObjCharacter( ARCHANDLE *handle, ARCDATID datID, OBJTYPE objType, u32 offs, u32 transSize, BOOL compressedFlag, HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * ｽｸﾘｰﾝﾃﾞｰﾀの VRAM 転送
 * ※ BGL側に ｽｸﾘｰﾝﾊﾞｯﾌｧ が用意されていれば、ｽｸﾘｰﾝﾊﾞｯﾌｧ への転送も行う
 *
 * @param   arcID			アーカイブファイルインデックス
 * @param   datID			アーカイブデータインデックス
 * @param   frm				転送先ﾌﾚｰﾑﾅﾝﾊﾞ
 * @param   offs			転送ｵﾌｾｯﾄ（ｷｬﾗ単位）
 * @param	transSize		転送するｻｲｽﾞ（ﾊﾞｲﾄ単位 ==0で全転送）
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   heapID			ﾃﾞｰﾀ読み込み・解凍ﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 */
//--------------------------------------------------------------------------------------------
extern void GFL_ARC_UTIL_TransVramScreen(u32 arcFile, u32 dataIdx, u32 frm, u32 offs, u32 transSize, BOOL compressedFlag, HEAPID heapID);
extern void GFL_ARCHDL_UTIL_TransVramScreen(ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 offs, u32 transSize, BOOL compressedFlag, HEAPID heapID);

//--------------------------------------------------------------------------------------------
/**
 * ｽｸﾘｰﾝﾃﾞｰﾀの VRAM 転送（キャラのオフセットも指定可）
 * ※ BGL側に ｽｸﾘｰﾝﾊﾞｯﾌｧ が用意されていれば、ｽｸﾘｰﾝﾊﾞｯﾌｧ への転送も行う
 *
 * @param   arcID			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID			ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   frm				転送先ﾌﾚｰﾑﾅﾝﾊﾞ
 * @param   offs			転送ｵﾌｾｯﾄ（ｷｬﾗ単位）
 * @param   chr_ofs			スクリーンデータのキャラNoへのｵﾌｾｯﾄ
 * @param	transSize		転送するｻｲｽﾞ（ﾊﾞｲﾄ単位 ==0で全転送）
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   heapID			ﾃﾞｰﾀ読み込み・解凍ﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 */
//--------------------------------------------------------------------------------------------
extern	void GFL_ARC_UTIL_TransVramScreenCharOfs(ARCID arcID, ARCDATID datID, u32 frm, u32 offs, u32 chr_ofs, u32 transSize, BOOL compressedFlag, HEAPID heapID);
extern void GFL_ARCHDL_UTIL_TransVramScreenCharOfs(ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 offs, u32 chr_ofs, u32 transSize, BOOL compressedFlag, HEAPID heapID);
extern void GFL_ARCHDL_UTIL_TransVramScreenCharOfsVBlank(ARCHANDLE *handle, ARCDATID datID, u32 frm, u32 offs, u32 chr_ofs, u32 transSize, BOOL compressedFlag, HEAPID heapID);

//------------------------------------------------------------------
/**
 * ﾊﾟﾚｯﾄﾃﾞｰﾀ の VRAM 転送
 *
 * @param   arcID		アーカイブファイルインデックス
 * @param   datID		アーカイブデータインデックス
 * @param   palType		ﾊﾟﾚｯﾄ転送先ﾀｲﾌﾟ
 * @param   offs		ﾊﾟﾚｯﾄ転送先ｵﾌｾｯﾄ
 * @param   transSize	ﾊﾟﾚｯﾄ転送ｻｲｽﾞ（0 で全転送）
 * @param   heapID		ﾃﾞｰﾀ読み込みﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 */
//------------------------------------------------------------------
extern void GFL_ARC_UTIL_TransVramPalette( ARCID arcID, ARCDATID datID, PALTYPE palType, u32 offs, u32 transSize, HEAPID heapID );
extern void GFL_ARCHDL_UTIL_TransVramPalette( ARCHANDLE *handle, ARCDATID datID, PALTYPE palType, u32 offs, u32 transSize, HEAPID heapID );


//------------------------------------------------------------------
/**
 * ﾊﾟﾚｯﾄﾃﾞｰﾀ の VRAM 転送（転送元の読み込み開始ｵﾌｾｯﾄ指定版）
 *
 * @param   arcID		ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID		ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   palType		ﾊﾟﾚｯﾄ転送先ﾀｲﾌﾟ
 * @param   srcOfs		ﾊﾟﾚｯﾄ転送元読み込み開始ｵﾌｾｯﾄ
 * @param   dstOfs		ﾊﾟﾚｯﾄ転送先ｵﾌｾｯﾄ
 * @param   transSize	ﾊﾟﾚｯﾄ転送ｻｲｽﾞ（0 で全転送）
 * @param   heapID		ﾃﾞｰﾀ読み込みﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 */
//------------------------------------------------------------------
extern void GFL_ARC_UTIL_TransVramPaletteEx( ARCID arcID, ARCDATID datID, PALTYPE palType, u32 srcOfs, u32 dstOfs, u32 transSize, HEAPID heapID );
extern void GFL_ARCHDL_UTIL_TransVramPaletteEx( ARCHANDLE *handle, ARCDATID datID, PALTYPE palType, u32 srcOfs, u32 dstOfs, u32 transSize, HEAPID heapID );

//------------------------------------------------------------------
/**
 * ﾊﾟﾚｯﾄﾃﾞｰﾀ の VRAM 転送＆ NITRO System ﾊﾟﾚｯﾄﾌﾟﾛｷｼ を作成
 *（3D, OBJ 用にのみ対応。BG には使いません）
 *
 * @param   arcID		ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID		ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   type		転送先ﾀｲﾌﾟ
 * @param   offs		転送ｵﾌｾｯﾄ
 * @param   heapID		ﾋｰﾌﾟID
 * @param   proxy		作成するﾌﾟﾛｷｼのｱﾄﾞﾚｽ
 *
 *	[ type ]
 *		NNS_G2D_VRAM_TYPE_3DMAIN = 0,
 *		NNS_G2D_VRAM_TYPE_2DMAIN = 1,
 *		NNS_G2D_VRAM_TYPE_2DSUB  = 2,
 *
 */
//------------------------------------------------------------------
extern void GFL_ARC_UTIL_TransVramPaletteMakeProxy( ARCID arcID, ARCDATID datID, NNS_G2D_VRAM_TYPE type, u32 offs, HEAPID heapID, NNSG2dImagePaletteProxy* proxy );
extern void GFL_ARCHDL_UTIL_TransVramPaletteMakeProxy( ARCHANDLE *handle, ARCDATID datID, NNS_G2D_VRAM_TYPE type, u32 offs, HEAPID heapID, NNSG2dImagePaletteProxy* proxy );

//------------------------------------------------------------------
/**
 * ｷｬﾗﾃﾞｰﾀ の VRAM 転送＆ NITRO System ｲﾒｰｼﾞﾌﾟﾛｷｼ を作成
 *（3D, OBJ 用にのみ対応。BG には使いません）
 *
 *
 * @param   arcID			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID			ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	圧縮されているか
 * @param   mapType			ﾏｯﾋﾟﾝｸﾞﾀｲﾌﾟ
 * @param   transSize		転送ｻｲｽﾞ（0なら全転送）
 * @param   vramType		転送先ﾀｲﾌﾟ
 * @param   offs			転送ｵﾌｾｯﾄ
 * @param   heapID			ﾃﾝﾎﾟﾗﾘに使うﾋｰﾌﾟID
 * @param   proxy			作成するﾌﾟﾛｷｼのｱﾄﾞﾚｽ
 *
 * @retval  転送されたﾃﾞｰﾀｻｲｽﾞ（ﾊﾞｲﾄ単位）
 *
 *	[ mapType ]
 *		MAP_TYPE_1D = 0,
 *		MAP_TYPE_2D = 1,
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
extern u32 GFL_ARC_UTIL_TransVramCharacterMakeProxy( ARCID arcID, ARCDATID datID, BOOL compressedFlag, CHAR_MAPPING_TYPE mapType,
	u32 transSize, NNS_G2D_VRAM_TYPE vramType, u32 offs, HEAPID heapID, NNSG2dImageProxy* proxy );
extern u32 GFL_ARCHDL_UTIL_TransVramCharacterMakeProxy(ARCHANDLE *handle, ARCDATID datID, BOOL compressedFlag, CHAR_MAPPING_TYPE mapType, u32 transSize, NNS_G2D_VRAM_TYPE vramType, u32 offs, HEAPID heapID, NNSG2dImageProxy* proxy );


//------------------------------------------------------------------
/**
 * ｷｬﾗﾃﾞｰﾀ の VRAM 転送＆ NITRO System ｲﾒｰｼﾞﾌﾟﾛｷｼ を作成
 *（3D, OBJ 用にのみ対応。BG には使いません）
 *
 * ｷｬﾗﾃﾞｰﾀのﾏｯﾋﾟﾝｸﾞﾓｰﾄﾞ値を、現在のﾚｼﾞｽﾀ設定に合わせて書き換えます
 *
 * @param   arcID			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID			ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	圧縮されているか
 * @param   mapType			ﾏｯﾋﾟﾝｸﾞﾀｲﾌﾟ
 * @param   transSize		転送サイズ。０なら全転送。
 * @param   vramType		転送先ﾀｲﾌﾟ
 * @param   offs			転送ｵﾌｾｯﾄ
 * @param   heapID			ﾃﾝﾎﾟﾗﾘに使うﾋｰﾌﾟID
 * @param   proxy			作成するﾌﾟﾛｷｼｱﾄﾞﾚｽ
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
extern void GFL_ARC_UTIL_TransVramCharacterMakeProxySyncroMappingMode( ARCID arcID, ARCDATID datID, BOOL compressedFlag, CHAR_MAPPING_TYPE mapType, u32 transSize,
	NNS_G2D_VRAM_TYPE vramType, u32 offs, HEAPID heapID, NNSG2dImageProxy* proxy );
extern void GFL_ARCHDL_UTIL_TransVramCharacterMakeProxySyncroMappingMode( ARCHANDLE *handle, ARCDATID datID, BOOL compressedFlag, CHAR_MAPPING_TYPE mapType, u32 transSize,
	NNS_G2D_VRAM_TYPE vramType, u32 offs, HEAPID heapID, NNSG2dImageProxy* proxy );

//------------------------------------------------------------------
/**
 * ｷｬﾗﾃﾞｰﾀ の VRAM 転送＆ NITRO System ｲﾒｰｼﾞﾌﾟﾛｷｼ を作成。VRAM転送型の画像素材用。
 *
 * ※ この関数を使っても、VRAMに画像は転送されません
 *    この関数でﾛｰﾄﾞしたﾃﾞｰﾀは解放されません。戻り値のconst void*を管理して、
 *    不要になったら解放処理を行ってください。
 *
 * @param   arcID			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID			ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	圧縮されているか
 * @param   vramType		転送先ﾀｲﾌﾟ
 * @param   offs			転送ｵﾌｾｯﾄ
 * @param   heapID			ﾃﾝﾎﾟﾗﾘに使うﾋｰﾌﾟID
 * @param   proxy			作成するﾌﾟﾛｷｼのｱﾄﾞﾚｽ
 * @param   charData		ｷｬﾗﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
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
extern const void* GFL_ARC_UTIL_LoadTransTypeCharacterMakeProxy( ARCID arcID, ARCDATID datID, BOOL compressedFlag, 
	NNS_G2D_VRAM_TYPE vramType, u32 offs, HEAPID heapID, NNSG2dImageProxy* proxy, NNSG2dCharacterData** charData );
extern const void* GFL_ARCHDL_UTIL_LoadTransTypeCharacterMakeProxy( ARCHANDLE *handle, ARCDATID datID, BOOL compressedFlag, 
	NNS_G2D_VRAM_TYPE vramType, u32 offs, HEAPID heapID, NNSG2dImageProxy* proxy, NNSG2dCharacterData** charData );

//------------------------------------------------------------------
/**
 * ｷｬﾗﾃﾞｰﾀを ﾛｰﾄﾞして Unpack するだけです。解放は各自で。（BG用）
 *
 * @param   arcID				[in] ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID				[in] ﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag		[in] 圧縮されているか
 * @param   charData			[out] ｷｬﾗﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 * @param   heapID				[in] ﾋｰﾌﾟID
 *
 * @retval  void*		ﾛｰﾄﾞしたﾃﾞｰﾀの先頭ﾎﾟｲﾝﾀ
 */
//------------------------------------------------------------------
extern void* GFL_ARC_UTIL_LoadBGCharacter( ARCID arcID, ARCDATID datID, BOOL compressedFlag, NNSG2dCharacterData** charData, HEAPID heapID );
extern void* GFL_ARCHDL_UTIL_LoadBGCharacter( ARCHANDLE *handle, ARCDATID datID, BOOL compressedFlag, NNSG2dCharacterData** charData, HEAPID heapID );

//------------------------------------------------------------------
/**
 * ｷｬﾗﾃﾞｰﾀを ﾛｰﾄﾞして Unpack するだけです。解放は各自で。（OBJ用）
 *
 * @param   arcID				[in] ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID				[in] ﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag		[in] 圧縮されているか
 * @param   charData			[out] ｷｬﾗﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 * @param   heapID				[in] ﾋｰﾌﾟID
 *
 * @retval  void*		ﾛｰﾄﾞしたﾃﾞｰﾀの先頭ﾎﾟｲﾝﾀ
 */
//------------------------------------------------------------------
extern void* GFL_ARC_UTIL_LoadOBJCharacter( ARCID arcID, ARCDATID datID, BOOL compressedFlag, NNSG2dCharacterData** charData, HEAPID heapID );
extern void* GFL_ARCHDL_UTIL_LoadOBJCharacter( ARCHANDLE *handle, ARCDATID datID, BOOL compressedFlag, NNSG2dCharacterData** charData, HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * ｽｸﾘｰﾝﾃﾞｰﾀを ﾛｰﾄﾞして Unpack するだけです。解放は各自で。
 *
 * @param   arcID			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID			ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   scrnData		ｽｸﾘｰﾝﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 * @param   heapID			ﾃﾞｰﾀ読み込み・解凍ﾃﾝﾎﾟﾗﾘとして使うﾋｰﾌﾟID
 *
 * @retval  void*		ﾛｰﾄﾞしたﾃﾞｰﾀの先頭ﾎﾟｲﾝﾀ
 */
//--------------------------------------------------------------------------------------------
extern void* GFL_ARC_UTIL_LoadScreen(ARCID arcID, ARCDATID datID, BOOL compressedFlag, NNSG2dScreenData** scrnData, HEAPID heapID);
extern void* GFL_ARCHDL_UTIL_LoadScreen(ARCHANDLE *handle, ARCDATID datID, BOOL compressedFlag, NNSG2dScreenData** scrnData, HEAPID heapID);

//------------------------------------------------------------------
/**
 * ﾊﾟﾚｯﾄﾃﾞｰﾀをﾛｰﾄﾞして Unpack するだけです。解放は各自で。
 *
 * @param   arcID			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID			ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   palData			ﾊﾟﾚｯﾄﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 * @param   heapID			ﾋｰﾌﾟID
 *
 * @retval  void*		ﾛｰﾄﾞしたﾃﾞｰﾀの先頭ﾎﾟｲﾝﾀ
 */
//------------------------------------------------------------------
extern void* GFL_ARC_UTIL_LoadPalette( ARCID arcID, ARCDATID datID, NNSG2dPaletteData** palData, HEAPID heapID );
extern void* GFL_ARCHDL_UTIL_LoadPalette(ARCHANDLE *handle, ARCDATID datID, NNSG2dPaletteData** palData, HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * ｾﾙﾊﾞﾝｸﾃﾞｰﾀを ﾛｰﾄﾞして Unpack するだけです。解放は各自で。
 *
 * @param   arcID			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID			ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   cellBank		ｾﾙﾊﾞﾝｸﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 * @param   heapID			ﾋｰﾌﾟID
 *
 * @retval  void*		ﾛｰﾄﾞしたﾃﾞｰﾀの先頭ﾎﾟｲﾝﾀ
 */
//--------------------------------------------------------------------------------------------
extern void* GFL_ARC_UTIL_LoadCellBank(ARCID arcID, ARCDATID datID, BOOL compressedFlag, NNSG2dCellDataBank** cellBank, HEAPID heapID );
extern void* GFL_ARCHDL_UTIL_LoadCellBank(ARCHANDLE *handle, ARCDATID datID, BOOL compressedFlag, NNSG2dCellDataBank** cellBank, HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * ｱﾆﾒﾊﾞﾝｸﾃﾞｰﾀを ﾛｰﾄﾞして Unpack するだけです。解放は各自で。
 *
 * @param   arcID			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID			ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   anmBank			ｱﾆﾒﾊﾞﾝｸﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 * @param   heapID			ﾋｰﾌﾟID
 *
 * @retval  void*		ﾛｰﾄﾞしたﾃﾞｰﾀの先頭ﾎﾟｲﾝﾀ
 */
//--------------------------------------------------------------------------------------------
extern void* GFL_ARC_UTIL_LoadAnimeBank(ARCID arcID, ARCDATID datID, BOOL compressedFlag, NNSG2dAnimBankData** anmBank, HEAPID heapID);
extern void* GFL_ARCHDL_UTIL_LoadAnimeBank(ARCHANDLE *handle, ARCDATID datID, BOOL compressedFlag, NNSG2dAnimBankData** anmBank, HEAPID heapID);

//--------------------------------------------------------------------------------------------
/**
 * ﾏﾙﾁｾﾙﾊﾞﾝｸﾃﾞｰﾀを ﾛｰﾄﾞして Unpack するだけです。解放は各自で。
 *
 * @param   arcID			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID			ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   cellBank		ﾏﾙﾁｾﾙﾊﾞﾝｸﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 * @param   heapID			ﾋｰﾌﾟID
 * @param   pSize			ｾﾙﾊﾞﾝｸのﾊﾞｲﾄｻｲｽﾞを受け取る変数ﾎﾟｲﾝﾀ（要らなければNULLで）
 *
 * @retval  void*		ﾛｰﾄﾞしたﾃﾞｰﾀの先頭ﾎﾟｲﾝﾀ
 */
//--------------------------------------------------------------------------------------------
extern	void* GFL_ARC_UTIL_LoadMultiCellBank(ARCID arcID, ARCDATID datID, BOOL compressedFlag, NNSG2dMultiCellDataBank** cellBank, HEAPID heapID );
extern void* GFL_ARCHDL_UTIL_LoadMultiCellBank(ARCHANDLE *handle, ARCDATID datID, BOOL compressedFlag, NNSG2dMultiCellDataBank** cellBank, HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * ｱﾆﾒﾊﾞﾝｸﾃﾞｰﾀを ﾛｰﾄﾞして Unpack するだけです。解放は各自で。
 *
 * @param   arcID			ｱｰｶｲﾌﾞﾌｧｲﾙｲﾝﾃﾞｯｸｽ
 * @param   datID			ｱｰｶｲﾌﾞﾃﾞｰﾀｲﾝﾃﾞｯｸｽ
 * @param   compressedFlag	圧縮されているﾃﾞｰﾀか？
 * @param   anmBank			ﾏﾙﾁｾﾙｱﾆﾒﾊﾞﾝｸﾃﾞｰﾀｱﾄﾞﾚｽを保持するﾎﾟｲﾝﾀのｱﾄﾞﾚｽ
 * @param   heapID			ﾋｰﾌﾟID
 *
 * @retval  void*		ﾛｰﾄﾞしたﾃﾞｰﾀの先頭ﾎﾟｲﾝﾀ
 */
//--------------------------------------------------------------------------------------------
extern	void* GFL_ARC_UTIL_LoadMultiAnimeBank(ARCID arcID, ARCDATID datID, BOOL compressedFlag, NNSG2dMultiCellAnimBankData** anmBank, HEAPID heapID);
extern void* GFL_ARCHDL_UTIL_LoadMultiAnimeBank(ARCHANDLE *handle, ARCDATID datID, BOOL compressedFlag, NNSG2dMultiCellAnimBankData** anmBank, HEAPID heapID);

//------------------------------------------------------------------
/**
 * LZ圧縮後アーカイブされているデータを読み出し、解凍して返す
 *
 * @param   arcID		アーカイブファイルインデックス
 * @param   datID		アーカイブデータインデックス
 * @param   heapID		読み出し・解凍に使うヒープＩＤ
 *
 * @retval  void*		解凍後のデータ保存先アドレス
 */
//------------------------------------------------------------------
extern void* GFL_ARC_UTIL_LoadUnCompress(ARCID arcID, ARCDATID datID, HEAPID heapID);
extern void* GFL_ARCHDL_UTIL_LoadUnCompress(ARCHANDLE *handle, ARCDATID datID, HEAPID heapID);


//------------------------------------------------------------------
/**
 * アーカイブデータの読み出し
 *
 * @param   arcID			アーカイブファイルインデックス
 * @param   datID			アーカイブデータインデックス
 * @param   compressedFlag	圧縮されているか？
 * @param   heapID			メモリ確保に使うヒープＩＤ
 * @param   allocType		ヒープのどの位置からメモリ確保するか
 *
 * @retval  void*			読み出し領域ポインタ
 */
//------------------------------------------------------------------
extern void* GFL_ARC_UTIL_Load(ARCID arcID, ARCDATID datID, BOOL compressedFlag, HEAPID heapID );
extern void* GFL_ARCHDL_UTIL_Load(ARCHANDLE *handle, ARCDATID datID, BOOL compressedFlag, HEAPID heapID);

//------------------------------------------------------------------
/**
 * アーカイブデータの読み出し＆データサイズ取得（圧縮されていたら解凍後のサイズを取得する）
 *
 * @param   arcID			アーカイブファイルインデックス
 * @param   datID			アーカイブデータインデックス
 * @param   compressedFlag	圧縮されているか？
 * @param   heapID			メモリ確保に使うヒープＩＤ
 * @param   allocType		ヒープのどの位置からメモリ確保するか
 * @param   pSize			実データのバイトサイズを受け取る変数のポインタ
 *
 * @retval  void*			読み出し領域ポインタ
 */
//------------------------------------------------------------------
extern void* GFL_ARC_UTIL_LoadEx(ARCID arcID, ARCDATID datID, BOOL compressedFlag, HEAPID heapID, u32* pSize);
extern void* GFL_ARCHDL_UTIL_LoadEx(ARCHANDLE* handle, ARCDATID datID, BOOL compressedFlag, u32 heapID, u32* pSize);





#ifdef __cplusplus
}/* extern "C" */
#endif

#endif
