//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _SCENE_OBJ		SCENE_OBJ;

extern SCENE_OBJ*	Create3Dobj( GFL_G3D_SCENE* g3Dscene, HEAPID heapID );
extern void			Delete3Dobj( SCENE_OBJ* sceneObj );

extern GFL_G3D_SCENEOBJ*	Get3DobjHandle( SCENE_OBJ* sceneObj, u32 idx );
extern void*				Get3DobjWork( SCENE_OBJ* sceneObj, u32 idx );
extern void					Get3DobjTrans( SCENE_OBJ* sceneObj, u32 idx, VecFx32* trans );
extern void					Set3DobjTrans( SCENE_OBJ* sceneObj, u32 idx, VecFx32* trans );
extern void					Get3DobjRotate( SCENE_OBJ* sceneObj, u32 idx, VecFx32* rotate );
extern void					Set3DobjRotate( SCENE_OBJ* sceneObj, u32 idx, VecFx32* rotate );

enum {
	OBJANM_CMD_STAY = 0,
	OBJANM_CMD_WALK,
	OBJANM_CMD_RUN,
	OBJANM_CMD_ATTACK,
	OBJANM_CMD_SHOOT,
	OBJANM_CMD_SPELL,
	OBJANM_CMD_SITDOWN,
	OBJANM_CMD_STANDUP,
	OBJANM_CMD_TAKE,
};

extern void	Set3DobjPlayerChrAnimeCmd( SCENE_OBJ* sceneObj, u32 idx, u8 animeCmd );
extern void	Set3DobjPlayerChrAccesory( SCENE_OBJ* sceneObj, u32 idx, u8 accesoryID );
extern BOOL	Check3DobjPlayerBusy( SCENE_OBJ* sceneObj, u32 idx );

