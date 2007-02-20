//=============================================================================================
/**
 * @file	g3d_util.h                                                  
 * @brief	３Ｄ管理システムプログラム
 */
//=============================================================================================
typedef struct _GFL_G3D_UTIL_SCENE	GFL_G3D_UTIL_SCENE;

typedef enum {
	GFL_G3D_UTIL_RESARC = 0,		//アーカイブＩＤを指定
	GFL_G3D_UTIL_RESPATH,			//アーカイブパスを指定

}GFL_G3D_UTIL_RESTYPE;	//指定アーカイブタイプ

//	リソース配列設定用テーブル構造体
typedef struct {
	const u32						arcive;		//指定アーカイブＩＤもしくはアーカイブパスポインタ
	const u16						datID;		//アーカイブ内データＩＤ
	const GFL_G3D_UTIL_RESTYPE		arcType;	//指定アーカイブタイプ
}GFL_G3D_UTIL_SCENE_RES;

typedef struct {
	const u16						anmresID;	//アニメリソースＩＤ
	const u16						anmdatID;	//アニメデータＩＤ(リソース内部ＩＮＤＥＸ)
}GFL_G3D_UTIL_SCENE_ANM;

typedef struct {
	const u16						mdlresID;	//モデルリソースＩＤ
	const u16						mdldatID;	//モデルデータＩＤ(リソース内部ＩＮＤＥＸ)
	const u16						texresID;	//テクスチャリソースＩＤ
	const GFL_G3D_UTIL_SCENE_ANM*	anmTbl;		//アニメテーブル（複数設定のため）
	const u16						anmCount;	//アニメリソース数
}GFL_G3D_UTIL_SCENE_OBJ;

typedef struct {
	const GFL_G3D_UTIL_SCENE_RES*	resTbl;		//リソーステーブル
	const u16						resCount;	//リソース数

	const GFL_G3D_UTIL_SCENE_OBJ*	objTbl;		//オブジェクト設定テーブル
	const u16						objCount;	//オブジェクト数

}GFL_G3D_UTIL_SCENE_SETUP;

//=============================================================================================
/**
 *
 *
 * ３Ｄ関連ハンドルの管理ユーティリティー
 *
 *
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * セットアップ
 *
 * @param	scene					設定データ
 * @param	heapID					ヒープＩＤ
 *
 * @return	GFL_G3D_UTIL_SCENE*		シーンハンドル
 */
//--------------------------------------------------------------------------------------------
extern GFL_G3D_UTIL_SCENE*
	GFL_G3D_UtilsysCreate
		( const GFL_G3D_UTIL_SCENE_SETUP* scene, HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * 破棄
 *
 * @param	GFL_G3D_UTIL_SCENE*		シーンハンドル
 */
//--------------------------------------------------------------------------------------------
extern void
	GFL_G3D_UtilsysDelete
		( GFL_G3D_UTIL_SCENE* g3DutilScene );

//--------------------------------------------------------------------------------------------
/**
 * オブジェクトハンドル取得
 */
//--------------------------------------------------------------------------------------------
extern GFL_G3D_OBJ*
	GFL_G3D_UtilsysObjHandleGet
		( GFL_G3D_UTIL_SCENE* g3DutilScene, u16 idx );

//--------------------------------------------------------------------------------------------
/**
 * オブジェクト数取得
 */
//--------------------------------------------------------------------------------------------
extern u16
	GFL_G3D_UtilsysObjCountGet
		( GFL_G3D_UTIL_SCENE* g3DutilScene );

