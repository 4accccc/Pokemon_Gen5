//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
typedef struct _SCENE_ACT		SCENE_ACT;

extern SCENE_ACT*	Create3Dact( GFL_G3D_SCENE* g3Dscene, HEAPID heapID );
extern void			Delete3Dact( SCENE_ACT* sceneAct );

extern GFL_G3D_SCENEOBJ*	Get3DactHandle( SCENE_ACT* sceneAct, u32 idx );
extern void*				Get3DactWork( SCENE_ACT* sceneAct, u32 idx );
extern void					Get3DactTrans( SCENE_ACT* sceneAct, u32 idx, VecFx32* trans );
extern void					Set3DactTrans( SCENE_ACT* sceneAct, u32 idx, VecFx32* trans );
extern void					Get3DactRotate( SCENE_ACT* sceneAct, u32 idx, VecFx32* rotate );
extern void					Set3DactRotate( SCENE_ACT* sceneAct, u32 idx, VecFx32* rotate );
extern void					Set3DactBlendAlpha( SCENE_ACT* sceneAct, u32 idx, u8* alpha );
extern void					Set3DactDrawSw( SCENE_ACT* sceneAct, u32 idx, BOOL sw );



enum {
	ACTANM_CMD_STAY = 0,
	ACTANM_CMD_WALK,
	ACTANM_CMD_RUN,
	ACTANM_CMD_ATTACK,
	ACTANM_CMD_SHOOT,
	ACTANM_CMD_SPELL,
	ACTANM_CMD_SITDOWN,
	ACTANM_CMD_STANDUP,
	ACTANM_CMD_TAKE,
	ACTANM_CMD_HIT,
	ACTANM_CMD_DEAD,
	ACTANM_CMD_JUMP_RDY,
	ACTANM_CMD_JUMPUP,
	ACTANM_CMD_JUMPDOWN,
	ACTANM_CMD_JUMP_END,
};

extern void	Set3DactPlayerChrAnimeCmd( SCENE_ACT* sceneAct, u32 idx, u8 animeCmd );
extern void	Set3DactPlayerChrAnimeForceCmd( SCENE_ACT* sceneAct, u32 idx, u8 animeCmd );
extern void	Set3DactPlayerChrAccesory( SCENE_ACT* sceneAct, u32 idx, u8 accesoryID );
extern BOOL	Check3DactPlayerBusy( SCENE_ACT* sceneAct, u32 idx );

//---------------------
//g3Dscene オブジェクト設定テーブルデータＩＮＤＥＸ
enum {
	G3DSCOBJ_PLAYER1 = 0,
	G3DSCOBJ_PLAYER2,
};

