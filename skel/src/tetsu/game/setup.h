//============================================================================================
/**
 * @file	
 * @brief	
 * @date	
 */
//============================================================================================
#include "map3d.h"
#include "act3d.h"

#define PLAYER_SETUP_MAX	(8)		//今のところこれ以上表示させるのは難しいかも、ライン
#define PLAYER_SETUP_NUM	(8)
#define STATUS_SETUP_NUM	(48)		//ステータスアイコン最大数

typedef struct _GAME_SYSTEM GAME_SYSTEM;

extern GAME_SYSTEM*	SetupGameSystem( HEAPID heapID );
extern void			RemoveGameSystem( GAME_SYSTEM* gs );
extern void			MainGameSystemPref( GAME_SYSTEM* gs );
extern void			MainGameSystemAfter( GAME_SYSTEM* gs );

extern GFL_G3D_UTIL*		Get_GS_G3Dutil( GAME_SYSTEM* gs );
extern GFL_G3D_SCENE*		Get_GS_G3Dscene( GAME_SYSTEM* gs );
extern GFL_G3D_CAMERA*		Get_GS_G3Dcamera( GAME_SYSTEM* gs, int cameraID );
extern GFL_G3D_LIGHTSET*	Get_GS_G3Dlight( GAME_SYSTEM* gs, int lightID );
extern GFL_BMPWIN*			Get_GS_BmpWin( GAME_SYSTEM* gs, int bmpwinID );
extern GFL_PTC_PTR			Get_GS_Perticle( GAME_SYSTEM* gs );
extern SCENE_MAP*			Get_GS_SceneMap( GAME_SYSTEM* gs );
extern SCENE_ACT*			Get_GS_SceneAct( GAME_SYSTEM* gs );
extern GFL_CLUNIT*			Get_GS_ClactUnit( GAME_SYSTEM* gs, u32 unitID );
extern u32					Get_GS_ClactResIdx( GAME_SYSTEM* gs, u32 resID );

#define PLT_2D_COL_WHITE	(15)
#define PLT_2D_COL_BLACK	(1)
#define PLT_2D_COL_NULL		(0)
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
	G3DRES_HUMAN_COMMON_BCA,
	G3DRES_HUMAN2_ATTACK_BCA,
	G3DRES_HUMAN2_SHOOT_BCA,
	G3DRES_HUMAN2_SPELL_BCA,
	G3DRES_ACCE_SWORD,
	G3DRES_ACCE_SHIELD,
	G3DRES_ACCE_BOW,
	G3DRES_ACCE_STAFF,
	G3DRES_EFFECT_WALL,
	G3DRES_EFFECT_ARROW,
};

//---------------------
//３Ｄオブジェクトアニメーション定義テーブルＩＮＤＥＸ
enum {
	HUMAN_ANM_COMMON = 0,
	HUMAN2_ANM_ATTACK,
	HUMAN2_ANM_SHOOT,
	HUMAN2_ANM_SPELL,
};

//---------------------
//３Ｄオブジェクト定義テーブルＩＮＤＥＸ
enum {
	G3DOBJ_MAP_FLOOR,
// プレーヤー数 (=PLAYER_SETUP_NUM)
	G3DOBJ_HUMAN1,
	G3DOBJ_HUMAN2,
	G3DOBJ_HUMAN3,
	G3DOBJ_HUMAN4,
	G3DOBJ_HUMAN5,
	G3DOBJ_HUMAN6,
	G3DOBJ_HUMAN7,
	G3DOBJ_HUMAN8,
//
	G3DOBJ_ACCE_SWORD,
	G3DOBJ_ACCE_SHIELD,
	G3DOBJ_ACCE_BOW,
	G3DOBJ_ACCE_STAFF,

	G3DOBJ_EFFECT_WALL,
	G3DOBJ_EFFECT_ARROW,
	G3DOBJ_NPC,
};

//---------------------
//２Ｄビットマップウインドウ定義テーブルＩＮＤＥＸ
enum {
	G2DBMPWIN_STATUS = 0,
	G2DBMPWIN_MSG,
	G2DBMPWIN_MAP,
	G2DBMPWIN_MASK,
};

//---------------------
//ＣＬＡＣＴリソース定義テーブルＩＮＤＥＸ
enum {
	CLACTRES_MAPOBJ_CGX = 0,
	CLACTRES_MAPOBJ_PLT_R,
	CLACTRES_MAPOBJ_PLT_B,
	CLACTRES_MAPOBJ_CEL,
	CLACTRES_STATUS_CEL,
	CLACTRES_STATUS_CGX,
	CLACTRES_STATUS_PLT = CLACTRES_STATUS_CGX + STATUS_SETUP_NUM,
};

//---------------------
//ＣＬＡＣＴユニット定義テーブルＩＮＤＥＸ
enum {
	CLACTUNIT_MAPOBJ = 0,
	CLACTUNIT_STATUS,
};

