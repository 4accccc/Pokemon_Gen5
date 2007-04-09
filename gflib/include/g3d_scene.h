//=============================================================================================
/**
 * @file	g3d_scene.h
 * @brief	３Ｄシーン管理プログラム
 */
//=============================================================================================
#ifndef _G3D_SCENE_H_
#define _G3D_SCENE_H_

//=============================================================================================
//	型宣言
//=============================================================================================
typedef struct _GFL_G3D_SCENE		GFL_G3D_SCENE;
typedef struct _GFL_G3D_SCENEOBJ	GFL_G3D_SCENEOBJ;	
typedef void						(GFL_G3D_SCENEOBJFUNC)( GFL_G3D_SCENEOBJ*, void* );

typedef struct {
	u32						objID;
	u8						movePriority;				
	u8						drawPriority;				
	u8						blendAlpha;
	BOOL					drawSW;
	GFL_G3D_OBJSTATUS		status; 
	GFL_G3D_SCENEOBJFUNC*	func;
}GFL_G3D_SCENEOBJ_DATA;

//=============================================================================================
/**
 * 管理システム
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * システム作成
 *
 * @param	g3Dutil			依存するg3Dutil
 * @param	SceneObjMax		配置可能なオブジェクト数
 * @param	SceneObjWkSiz	１オブジェクトに割り当てるワークのサイズ
 * @param	heapID			ヒープＩＤ
 */
//--------------------------------------------------------------------------------------------
extern GFL_G3D_SCENE*
	GFL_G3D_SCENE_Create
		( GFL_G3D_UTIL* g3Dutil, const u16 sceneObjMax, const u32 sceneObjWkSiz, 
			const HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * システムメイン（動作）
 *
 * @param	g3Dscene		システムポインタ
 */
//--------------------------------------------------------------------------------------------
extern void
	GFL_G3D_SCENE_Main
		( GFL_G3D_SCENE* g3Dscene );  

//--------------------------------------------------------------------------------------------
/**
 * システムメイン（描画）
 *
 * @param	g3Dscene		システムポインタ
 */
//--------------------------------------------------------------------------------------------
extern void
	GFL_G3D_SCENE_Draw
		( GFL_G3D_SCENE* g3Dscene );  

//--------------------------------------------------------------------------------------------
/**
 * システム破棄
 *
 * @param	g3Dscene		システムポインタ
 */
//--------------------------------------------------------------------------------------------
extern void
	GFL_G3D_SCENE_Delete
		( GFL_G3D_SCENE* g3Dscene );  

//--------------------------------------------------------------------------------------------
/**
 * 配置オブジェクトポインタをＩＮＤＥＸより取得
 *
 * @param	g3Dscene		システムポインタ
 * @param	idx				アクター配置ＩＮＤＥＸ
 */
//--------------------------------------------------------------------------------------------
GFL_G3D_SCENEOBJ*
	GFL_G3D_SCENEOBJ_Get
		( GFL_G3D_SCENE* g3Dscene, u32 idx );

//=============================================================================================
/**
 * 配置オブジェクト
 */
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * 作成
 *
 * @param	g3Dscene		システムポインタ
 * @param	sceneObjTbl		配置オブジェクト設定データ
 * @param	sceneObjCount	配置オブジェクト数
 *
 * @return	idx				配置オブジェクト先頭ＩＮＤＥＸ
 */
//--------------------------------------------------------------------------------------------
extern u32
	GFL_G3D_SCENEOBJ_Add
		( GFL_G3D_SCENE* g3Dscene, const GFL_G3D_SCENEOBJ_DATA* sceneObjTbl, 
			const u16 sceneObjCount );

//--------------------------------------------------------------------------------------------
/**
 * 破棄
 *
 * @param	g3Dscene		システムポインタ
 * @param	idx				配置オブジェクト先頭ＩＮＤＥＸ
 * @param	sceneObjCount	配置オブジェクト数
 */
//--------------------------------------------------------------------------------------------
extern void
	GFL_G3D_SCENEOBJ_Remove
		( GFL_G3D_SCENE* g3Dscene, u32 idx, const u16 sceneObjCount );

//--------------------------------------------------------------------------------------------
/**
 * オブジェクトハンドル(G3D_OBJ)の取得
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 */
//--------------------------------------------------------------------------------------------
extern GFL_G3D_OBJ*
	GFL_G3D_SCENEOBJ_GetG3DobjHandle
		( GFL_G3D_SCENEOBJ* g3DsceneObj );

//--------------------------------------------------------------------------------------------
/**
 * オブジェクトＩＤの取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	objID			オブジェクトＩＤの格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
extern void GFL_G3D_SCENEOBJ_GetObjID( GFL_G3D_SCENEOBJ* g3DsceneObj, u16* objID );
extern void GFL_G3D_SCENEOBJ_SetObjID( GFL_G3D_SCENEOBJ* g3DsceneObj, u16* objID );

//--------------------------------------------------------------------------------------------
/**
 * 描画プライオリティーの取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	drawPri			描画プライオリティーの格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
extern void GFL_G3D_SCENEOBJ_GetDrawPri( GFL_G3D_SCENEOBJ* g3DsceneObj, u8* drawPri );
extern void GFL_G3D_SCENEOBJ_SetDrawPri( GFL_G3D_SCENEOBJ* g3DsceneObj, u8* drawPri );

//--------------------------------------------------------------------------------------------
/**
 * αブレンド値の取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	blendAlpha		αブレンドの格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
extern void GFL_G3D_SCENEOBJ_GetBlendAlpha( GFL_G3D_SCENEOBJ* g3DsceneObj, u8* blendAlpha );
extern void GFL_G3D_SCENEOBJ_SetBlendAlpha( GFL_G3D_SCENEOBJ* g3DsceneObj, u8* blendAlpha );

//--------------------------------------------------------------------------------------------
/**
 * 描画スイッチの取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	drawSW			描画スイッチの格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
extern void GFL_G3D_SCENEOBJ_GetDrawSW( GFL_G3D_SCENEOBJ* g3DsceneObj, BOOL* drawSW );
extern void GFL_G3D_SCENEOBJ_SetDrawSW( GFL_G3D_SCENEOBJ* g3DsceneObj, BOOL* drawSW );

//--------------------------------------------------------------------------------------------
/**
 * ステータス（位置情報）の取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	trans			位置情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
extern void GFL_G3D_SCENEOBJ_GetPos( GFL_G3D_SCENEOBJ* g3DsceneObj, VecFx32* trans );
extern void GFL_G3D_SCENEOBJ_SetPos( GFL_G3D_SCENEOBJ* g3DsceneObj, VecFx32* trans );

//--------------------------------------------------------------------------------------------
/**
 * ステータス（スケール情報）の取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	scale			スケール情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
extern void GFL_G3D_SCENEOBJ_GetScale( GFL_G3D_SCENEOBJ* g3DsceneObj, VecFx32* scale );
extern void GFL_G3D_SCENEOBJ_SetScale( GFL_G3D_SCENEOBJ* g3DsceneObj, VecFx32* scale );

//--------------------------------------------------------------------------------------------
/**
 * ステータス（回転情報）の取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	rotate			回転情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
extern void GFL_G3D_SCENEOBJ_GetRotate( GFL_G3D_SCENEOBJ* g3DsceneObj, MtxFx33* rotate );
extern void GFL_G3D_SCENEOBJ_SetRotate( GFL_G3D_SCENEOBJ* g3DsceneObj, MtxFx33* rotate );

//--------------------------------------------------------------------------------------------
/**
 * 動作関数の取得と変更
 *
 * @param	g3DsceneObj		配置オブジェクトポインタ
 * @param	scale			スケール情報の格納もしくは参照ワークポインタ
 */
//--------------------------------------------------------------------------------------------
extern void GFL_G3D_SCENEOBJ_GetFunc( GFL_G3D_SCENEOBJ* g3DsceneObj, GFL_G3D_SCENEOBJFUNC** func );
extern void GFL_G3D_SCENEOBJ_SetFunc( GFL_G3D_SCENEOBJ* g3DsceneObj, GFL_G3D_SCENEOBJFUNC** func );



#endif
