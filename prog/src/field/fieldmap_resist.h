//============================================================================================
/**
 * @file	fieldmap_resist.h
 * @date	2008.12.19
 */
//============================================================================================
#pragma once
//------------------------------------------------------------------
//------------------------------------------------------------------

#define FLD_G3D_MAPPER_NOMAP	(0xffffffff)
typedef struct {
	u32 datID;
}FLD_G3D_MAPPER_DATA;

typedef enum {
	FLD_G3D_MAPPER_MODE_SCROLL_NONE = 0,
	FLD_G3D_MAPPER_MODE_SCROLL_XZ,
	FLD_G3D_MAPPER_MODE_SCROLL_Y,
}FLD_G3D_MAPPER_MODE;

#define	NON_LOWQ	(0xffff)
#define	NON_TEX		(0xffff)
typedef struct {
	u16 highQ_ID;
	u16 lowQ_ID;
}FLD_G3D_MAPPEROBJ_DATA;

typedef struct {
	u32							arcID;	//アーカイブＩＤ
	const FLD_G3D_MAPPEROBJ_DATA*	data;	//実マップデータ
	u32							count;		//モデル数

}FLD_G3D_MAPPER_RESIST_OBJ;

typedef struct {
	u32			arcID;	//アーカイブＩＤ
	const u16*	data;	//実マップデータ
	u32			count;	//テクスチャ数

}FLD_G3D_MAPPER_RESIST_DDOBJ;

#define NON_GLOBAL_OBJ	(0xffffffff)
#define USE_GLOBAL_OBJSET_TBL	(0)
#define USE_GLOBAL_OBJSET_BIN	(1)
typedef struct {
	u32							objArcID;	//アーカイブＩＤ
	const FLD_G3D_MAPPEROBJ_DATA*	objData;	//実マップデータ
	u32							objCount;	//モデル数
	u32							ddobjArcID;	//アーカイブＩＤ
	const u16*					ddobjData;	//実マップデータ
	u32							ddobjCount;	//モデル数
}FLD_G3D_MAPPER_GLOBAL_OBJSET_TBL;

typedef struct {
	u32							areaObjArcID;		//配置種類アーカイブＩＤ
	u32							areaObjAnmTblArcID;	//配置種類アニメ付与データアーカイブＩＤ
	u32							areaObjDatID;		//配置種類データＩＤ
	u32							objArcID;			//モデルアーカイブＩＤ
	u32							objtexArcID;		//テクスチャアーカイブＩＤ
	u32							objanmArcID;		//アニメアーカイブＩＤ
}FLD_G3D_MAPPER_GLOBAL_OBJSET_BIN;

#define	NON_GLOBAL_TEX	(0xffffffff)
#define	USE_GLOBAL_TEX	(0)
typedef struct {
	u32 arcID;
	u32 datID;
}FLD_G3D_MAPPER_GLOBAL_TEXTURE;

//============================================================================================
//============================================================================================

typedef struct {
	u32						g3DmapFileType;	//g3Dmapファイル識別タイプ（仮）
	fx32					width;			//ブロック１辺の幅
	fx32					height;			//ブロック高さ
	FLD_G3D_MAPPER_MODE			mode;			//動作モード
	u32						arcID;			//グラフィックアーカイブＩＤ
	u32						gtexType;		//グローバルテクスチャタイプ
	FLD_G3D_MAPPER_GLOBAL_TEXTURE*	gtexData;		//グローバルテクスチャ
	u32						gobjType;		//グローバルオブジェクトタイプ
	void*					gobjData;		//グローバルオブジェクト

	u16						sizex;			//横ブロック数
	u16						sizez;			//縦ブロック数
	u32						totalSize;		//配列サイズ
	const FLD_G3D_MAPPER_DATA*	data;			//実マップデータ

}FLD_G3D_MAPPER_RESIST;

//------------------------------------------------------------------
/**
 * @brief	マップデータ登録
 */
//------------------------------------------------------------------
extern void	ResistDataFieldG3Dmapper( FLD_G3D_MAPPER* g3Dmapper, const FLD_G3D_MAPPER_RESIST* resistData );


