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

extern SCENE_ACT*	Create3Dact( GFL_G3D_SCENE* g3Dscene, HEAPID heapID );
extern void			Delete3Dact( SCENE_ACT* sceneAct );

extern GFL_G3D_SCENEOBJ*	Get3DactHandle( SCENE_ACT* sceneAct, u32 idx );
extern void*				Get3DactWork( SCENE_ACT* sceneAct, u32 idx );

extern void					Set3DactTrans( SCENE_ACT* sceneAct, VecFx32* trans );
extern void					Set3DactRotate( SCENE_ACT* sceneAct, VecFx32* rotate );
extern void					Set3DactBlendAlpha( SCENE_ACT* sceneAct, u8* alpha );
extern void					Set3DactDrawSw( SCENE_ACT* sceneAct, BOOL sw );

enum {
	ACTANM_CMD_STAY = 0,
	ACTANM_CMD_WALK,
	ACTANM_CMD_RUN,
	ACTANM_CMD_SITDOWN,
	ACTANM_CMD_STANDUP,
	ACTANM_CMD_TAKE,
	ACTANM_CMD_HIT,
	ACTANM_CMD_DEAD,
	ACTANM_CMD_JUMP_RDY,
	ACTANM_CMD_JUMPUP,
	ACTANM_CMD_JUMPDOWN,
	ACTANM_CMD_JUMP_END,

	ACTANM_CMD_SWORD_ATTACK1,
	ACTANM_CMD_SWORD_ATTACK2,
	ACTANM_CMD_SWORD_ATTACK3,
	ACTANM_CMD_SWORD_ATTACK4,
	ACTANM_CMD_SWORD_ATTACK5,
	ACTANM_CMD_SWORD_ATTACK6,
	ACTANM_CMD_SHOOT,
	ACTANM_CMD_SPELL,
};

extern BOOL	Set3DactPlayerChrAnimeCmd( SCENE_ACT* sceneAct, u8 animeCmd );
extern void	Set3DactPlayerChrAnimeForceCmd( SCENE_ACT* sceneAct, u8 animeCmd );
extern void	Set3DactPlayerChrAccesory( SCENE_ACT* sceneAct, u8 accesoryID );
extern BOOL	Check3DactPlayerBusy( SCENE_ACT* sceneAct );

//---------------------
//g3Dscene オブジェクト設定テーブルデータＩＮＤＥＸ
enum {
	G3DSCOBJ_PLAYER1 = 0,
	G3DSCOBJ_PLAYER2,
};

//-------------------------------------------------------------------------------------
//
// テスト
//
//-------------------------------------------------------------------------------------
extern void	ChangeEquipNum( SCENE_ACT* sceneAct, u16 equipID );

