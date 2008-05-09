//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _SCENE_ACTSYS	SCENE_ACTSYS;
typedef struct _SCENE_ACT		SCENE_ACT;

extern SCENE_ACTSYS*	Create3DactSys( GFL_G3D_SCENE* g3Dscene, HEAPID heapID );
extern void				Delete3DactSys( SCENE_ACTSYS* sceneActSys );

extern GFL_G3D_SCENEOBJ*	Get3DactHandle( SCENE_ACT* sceneAct, u32 idx );
extern void*				Get3DactWork( SCENE_ACT* sceneAct, u32 idx );

extern void		Set3DactTrans( SCENE_ACT* sceneAct, VecFx32* trans );
extern void		Set3DactRotate( SCENE_ACT* sceneAct, VecFx32* rotate );
extern void		Set3DactBlendAlpha( SCENE_ACT* sceneAct, u8* alpha );
extern void		Set3DactDrawSw( SCENE_ACT* sceneAct, BOOL sw );

extern BOOL		Set3DactChrAnimeCmd( SCENE_ACT* sceneAct, u8 animeCmd );
extern void		Set3DactChrAnimeForceCmd( SCENE_ACT* sceneAct, u8 animeCmd );
extern void		Set3DactChrAccesory( SCENE_ACT* sceneAct, u8 accesoryID );
extern BOOL		Check3DactBusy( SCENE_ACT* sceneAct );

//--------------------------------------------------------------------------------------
//‚m‚o‚b
extern SCENE_ACT*		Create3DactNPC( GFL_G3D_SCENE* g3Dscene, HEAPID heapID );
extern void				Delete3DactNPC( SCENE_ACT* sceneAct );

