//============================================================================================
/**
 * @file	fieldmap_resist.h
 * @date	2008.12.19
 */
//============================================================================================
#pragma once
//------------------------------------------------------------------
//------------------------------------------------------------------

#define FLDMAPPER_MAPDATA_NULL	(0xffffffff)
typedef struct {
	u32 datID;
}FLDMAPPER_MAPDATA;

typedef enum {
	FLDMAPPER_MODE_SCROLL_NONE = 0,
	FLDMAPPER_MODE_SCROLL_XZ,
	FLDMAPPER_MODE_SCROLL_Y,
}FLDMAPPER_MODE;

//------------------------------------------------------------------
//------------------------------------------------------------------
#define	NON_LOWQ	(0xffff)
#define	NON_TEX		(0xffff)
typedef struct {
	u16 highQ_ID;
	u16 lowQ_ID;
}FLDMAPPER_RESISTOBJDATA;

typedef struct {
	u32							arcID;	//アーカイブＩＤ
	const FLDMAPPER_RESISTOBJDATA*	data;	//実データ
	u32							count;		//モデル数

}FLDMAPPER_RESISTDATA_OBJ;

typedef struct {
	u32			arcID;	//アーカイブＩＤ
	const u16*	data;	//実データ
	u32			count;	//テクスチャ数

}FLDMAPPER_RESISTDATA_DDOBJ;

#define NON_GLOBAL_OBJ	(0xffffffff)
#define USE_GLOBAL_OBJSET_TBL	(0)
#define USE_GLOBAL_OBJSET_BIN	(1)
typedef struct {
	u32							objArcID;	//アーカイブＩＤ
	const FLDMAPPER_RESISTOBJDATA*	objData;	//実データ
	u32							objCount;	//モデル数
	u32							ddobjArcID;	//アーカイブＩＤ
	const u16*					ddobjData;	//実データ
	u32							ddobjCount;	//モデル数
}FLDMAPPER_RESISTDATA_OBJTBL;

typedef struct {
	u32							areaObjArcID;		//配置種類アーカイブＩＤ
	u32							areaObjAnmTblArcID;	//配置種類アニメ付与データアーカイブＩＤ
	u32							areaObjDatID;		//配置種類データＩＤ
	u32							objArcID;			//モデルアーカイブＩＤ
	u32							objtexArcID;		//テクスチャアーカイブＩＤ
	u32							objanmArcID;		//アニメアーカイブＩＤ
}FLDMAPPER_RESISTDATA_OBJBIN;

//------------------------------------------------------------------
//------------------------------------------------------------------
#define	NON_GLOBAL_TEX	(0xffffffff)
#define	USE_GLOBAL_TEX	(0)
typedef struct {
	u32 arcID;
	u32 datID;
}FLDMAPPER_RESIST_TEX;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef struct {
	FLDMAPPER_FILETYPE		g3DmapFileType;	//g3Dmapファイル識別タイプ（仮）
	fx32					width;			//ブロック１辺の幅
	fx32					height;			//ブロック高さ
	FLDMAPPER_MODE			mode;			//動作モード
	u32						arcID;			//グラフィックアーカイブＩＤ
	u32						gtexType;		//グローバルテクスチャタイプ
	FLDMAPPER_RESIST_TEX*	gtexData;		//グローバルテクスチャ
	u32						gobjType;		//グローバルオブジェクトタイプ
	void*					gobjData;		//グローバルオブジェクト

	u16						sizex;			//横ブロック数
	u16						sizez;			//縦ブロック数
	u32						totalSize;		//配列サイズ
	const FLDMAPPER_MAPDATA*	blocks;			//実マップデータ

}FLDMAPPER_RESISTDATA;

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	マップデータ登録
 */
//------------------------------------------------------------------
extern void	FLDMAPPER_ResistData( FLDMAPPER* g3Dmapper, const FLDMAPPER_RESISTDATA* resistData );


