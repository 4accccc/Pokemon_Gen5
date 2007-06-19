//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
#include "map3d.h"
#include "obj3d.h"

typedef struct _GAME_SYSTEM GAME_SYSTEM;

GAME_SYSTEM*	SetupGameSystem( HEAPID heapID );
void			RemoveGameSystem( GAME_SYSTEM* gs );
void			MainGameSystemPref( GAME_SYSTEM* gs );
void			MainGameSystemAfter( GAME_SYSTEM* gs );

GFL_G3D_UTIL*		Get_GS_G3Dutil( GAME_SYSTEM* gs );
GFL_G3D_SCENE*		Get_GS_G3Dscene( GAME_SYSTEM* gs );
GFL_G3D_CAMERA*		Get_GS_G3Dcamera( GAME_SYSTEM* gs, int cameraID );
GFL_G3D_LIGHTSET*	Get_GS_G3Dlight( GAME_SYSTEM* gs, int lightID );
GFL_PTC_PTR			Get_GS_Perticle( GAME_SYSTEM* gs );
SCENE_MAP*			Get_GS_SceneMap( GAME_SYSTEM* gs );
SCENE_OBJ*			Get_GS_SceneObj( GAME_SYSTEM* gs );

//------------------------------------------------------------------
/**
 * @brief	データヘッダ
 */
//------------------------------------------------------------------
//３ＤカメラＩＮＤＥＸ
enum {
	MAINCAMERA_ID = 0,
	SUBCAMERA_ID,
};

//３ＤカメラＩＮＤＥＸ
enum {
	MAINLIGHT_ID = 0,
	SUBLIGHT_ID,
};

//３ＤグラフィックリソーステーブルＩＮＤＥＸ
enum {
	G3DRES_MAP_FLOOR = 0,
	G3DRES_HUMAN2_BMD,
	G3DRES_HUMAN2_STAY_BCA,
	G3DRES_HUMAN2_WALK_BCA,
	G3DRES_HUMAN2_RUN_BCA,
	G3DRES_HUMAN2_ATTACK_BCA,
	G3DRES_HUMAN2_SHOOT_BCA,
	G3DRES_HUMAN2_SPELL_BCA,
	G3DRES_HUMAN2_SIT_BCA,
	G3DRES_ACCE_SWORD,
	G3DRES_ACCE_SHIELD,
	G3DRES_ACCE_BOW,
	G3DRES_ACCE_STAFF,
};

//---------------------
//３Ｄオブジェクトアニメーション定義テーブルＩＮＤＥＸ
enum {
	HUMAN2_ANM_STAY = 0,
	HUMAN2_ANM_WALK,
	HUMAN2_ANM_RUN,
	HUMAN2_ANM_ATTACK,
	HUMAN2_ANM_SHOOT,
	HUMAN2_ANM_SPELL,
	HUMAN2_ANM_SIT,
};

//---------------------
//３Ｄオブジェクト定義テーブルＩＮＤＥＸ
enum {
	G3DOBJ_MAP_FLOOR,
	G3DOBJ_HUMAN2,
	G3DOBJ_HUMAN2_STOP,
	G3DOBJ_ACCE_SWORD,
	G3DOBJ_ACCE_SHIELD,
	G3DOBJ_ACCE_BOW,
	G3DOBJ_ACCE_STAFF,
};

//---------------------
//g3Dscene オブジェクト設定テーブルデータＩＮＤＥＸ
enum {
	G3DSCOBJ_PLAYER = 0,
};

