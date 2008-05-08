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
	EXMAPOBJ_TOWER_RED,
	EXMAPOBJ_TOWER_BLUE,
	EXMAPOBJ_TOWER_GREEN,
	EXMAPOBJ_TOWER_YELLOW,
};

extern BOOL	CheckGroundOutRange
	( SCENE_MAP* sceneMap, const VecFx32* pos );
extern BOOL	GetGroundPlaneData
	( SCENE_MAP* sceneMap, const VecFx32* pos, VecFx32* vecN, fx32* valD );
extern void	GetGroundPlaneVecN
	( SCENE_MAP* sceneMap, const VecFx32* pos, VecFx32* vecN );
extern void	GetGroundPlaneHeight
	( SCENE_MAP* sceneMap, const VecFx32* pos, fx32* height );
extern void	GetGroundMoveVec
	( SCENE_MAP* sceneMap, const VecFx32* pos, const VecFx32* vecDir, VecFx32* vecMove );
extern void	GetGroundMovePos
	( SCENE_MAP* sceneMap, const VecFx32* posNow, const VecFx32* vecMoveXZ, VecFx32* posNext );

extern BOOL GetRayPosOnMap
	( SCENE_MAP* sceneMap, const VecFx32* posRay, const VecFx32* vecRay, VecFx32* dst );

//‰¼
extern BOOL	DEBUG_CheckGroundMove
	( const VecFx32* posNow, const VecFx32* vecMove, VecFx32* posNext );
extern void	GLOBAL_GetGroundPlaneVecN
	( const VecFx32* pos, VecFx32* vecN );
extern void	GLOBAL_GetGroundPlaneHeight
	( const VecFx32* pos, fx32* height );

