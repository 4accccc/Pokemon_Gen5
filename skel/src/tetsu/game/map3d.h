//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _SCENE_MAP		SCENE_MAP;

extern SCENE_MAP*	Create3Dmap( GFL_G3D_SCENE* g3Dscene, HEAPID heapID );
extern void			AddObject3Dmap( SCENE_MAP* sceneMap, int objID, VecFx32* trans );
extern void			Delete3Dmap( SCENE_MAP* sceneMap );
extern BOOL			Get3DmapAttr( SCENE_MAP* sceneMap, VecFx32* pos, u16* attr );

enum {
	EXMAPOBJ_WALL = 0,
	EXMAPOBJ_WALL1,
	EXMAPOBJ_WALL2,
	EXMAPOBJ_WALL3,
	EXMAPOBJ_WALL4,
};


