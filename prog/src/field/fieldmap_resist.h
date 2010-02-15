//============================================================================================
/**
 * @file	fieldmap_resist.h
 * @date	2008.12.19
 */
//============================================================================================
#pragma once

//------------------------------------------------------------------
//
//	マップデータ
//
//------------------------------------------------------------------

#define FLDMAPPER_MAPDATA_NULL	(0xffffffff)
typedef struct {
	u32 datID;
}FLDMAPPER_MAPDATA;

//------------------------------------------------------------------
///	動作タイプの指定
//------------------------------------------------------------------
typedef enum {
	FLDMAPPER_MODE_SCROLL_NONE = 0,
	FLDMAPPER_MODE_SCROLL_XZ,
	FLDMAPPER_MODE_SCROLL_XZ_LOOP,
	FLDMAPPER_MODE_SCROLL_Y,
}FLDMAPPER_MODE;

//------------------------------------------------------------------
///	読み込みブロックファイルの種類コード
///	field_g3d_mapper.cのmapFileFuncTblと同期する必要がある
//------------------------------------------------------------------
typedef enum {
	FLDMAPPER_FILETYPE_NORMAL = 0,
	FLDMAPPER_FILETYPE_PKGS = 1,
  FLDMAPPER_FILETYPE_RANDOM = 2,
}FLDMAPPER_FILETYPE;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef enum {
	FLDMAPPER_TEXTYPE_NONE = 0,
	FLDMAPPER_TEXTYPE_USE,

  FLDMAPPER_TEXTYPE_MAX,
}FLDMAPPER_TEXTYPE;

typedef struct {
	u32 arcID;
	u32 datID;
}FLDMAPPER_RESIST_TEX;

typedef struct {
	u32 ita_datID;				// 無い場合はFLDMAPPER_MAPDATA_NULLを指定
	u32 itp_anm_datID;		// 無い場合はFLDMAPPER_MAPDATA_NULLを指定
}FLDMAPPER_RESIST_GROUND_ANIME;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	FLDMAPPER_FILETYPE		g3DmapFileType;	//g3Dmapファイル識別タイプ（仮）
	fx32					blockWidth;			//ブロック１辺の幅
	fx32					blockHeight;		//ブロック高さ
  u16           blockXNum;      //メモリに一度に置く、X方向ブロック数
  u16           blockZNum;      //メモリに一度に置く、Z方法ブロック数
	FLDMAPPER_MODE			mode;			//動作モード
	u32						arcID;			//グラフィックアーカイブＩＤ
	u16						sizex;			//横ブロック数
	u16						sizez;			//縦ブロック数
	u32						totalSize;		//配列サイズ
	const FLDMAPPER_MAPDATA*	blocks;			//実マップデータ

	FLDMAPPER_TEXTYPE	gtexType;		//グローバルテクスチャタイプ
	FLDMAPPER_RESIST_TEX	gtexData;		//グローバルテクスチャ

	FLDMAPPER_RESIST_GROUND_ANIME ground_anime;	// 地面アニメーション

	u32						memsize;				// １ブロックメモリサイズ

  u8            topWriteBlockNum; // top（データ更新フレーム）で描画するブロック数
  u8            pad[3];
}FLDMAPPER_RESISTDATA;

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	マップデータ登録
 */
//------------------------------------------------------------------
extern void	FLDMAPPER_ResistData( FLDMAPPER* g3Dmapper, const FLDMAPPER_RESISTDATA* resistData );


