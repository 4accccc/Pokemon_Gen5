//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _SCENE_MAP		SCENE_MAP;

extern SCENE_MAP*	Create3Dmap( GFL_G3D_SCENE* g3Dscene, HEAPID heapID );
extern void			Delete3Dmap( SCENE_MAP* sceneMap );
extern int			AddObject3Dmap( SCENE_MAP* sceneMap, int objID, VecFx32* trans );
extern void			RemoveObject3Dmap( SCENE_MAP* sceneMap, int mapobjID );
extern void			Set3DmapDrawSw( SCENE_MAP* sceneMap, int mapobjID, BOOL* sw );
extern void			Get3DmapDrawSw( SCENE_MAP* sceneMap, int mapobjID, BOOL* sw );
extern BOOL			Get3DmapAttr( SCENE_MAP* sceneMap, VecFx32* pos, u16* attr );

enum {
	EXMAPOBJ_WALL = 0,
	EXMAPOBJ_WALL1,
	EXMAPOBJ_WALL2,
	EXMAPOBJ_WALL3,
	EXMAPOBJ_WALL4,
};

//------------------------------------------------------------------
/**
 * @brief		地面との当たり判定（距離制限なし）
 */
//------------------------------------------------------------------
extern BOOL	CheckHitMapGround( VecFx32* posNow, VecFx32* vecMove, VecFx32* posResult );
//------------------------------------------------------------------
/**
 * @brief		地面との当たり判定（距離制限あり）
 */
//------------------------------------------------------------------
extern BOOL	CheckHitMapGroundLimit( VecFx32* posNow, VecFx32* posNext, VecFx32* posResult );
//------------------------------------------------------------------
/**
 * @brief		マップ上でのベクトル移動（重力あり）
 *
 *	gravityTimerはここでしか操作しないようにする
 */
//------------------------------------------------------------------
extern void	InitMoveMapGround( int* gravityTimer );
extern BOOL	MoveMapGround( VecFx32* posNow, VecFx32* vecMove, int* gravityTimer );


