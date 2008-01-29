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
extern void			MainGameSystem( GAME_SYSTEM* gs );

extern GFL_G3D_UTIL*		Get_GS_G3Dutil( GAME_SYSTEM* gs );
extern GFL_G3D_SCENE*		Get_GS_G3Dscene( GAME_SYSTEM* gs );
extern GFL_G3D_CAMERA*		Get_GS_G3Dcamera( GAME_SYSTEM* gs, int cameraID );
extern GFL_G3D_LIGHTSET*	Get_GS_G3Dlight( GAME_SYSTEM* gs, int lightID );
extern GFL_BMPWIN*			Get_GS_BmpWin( GAME_SYSTEM* gs, int bmpwinID );
extern GFL_PTC_PTR			Get_GS_Perticle( GAME_SYSTEM* gs );
extern SCENE_ACTSYS*		Get_GS_SceneActSys( GAME_SYSTEM* gs );
extern SCENE_MAP*			Get_GS_SceneMap( GAME_SYSTEM* gs );
extern GFL_CLUNIT*			Get_GS_ClactUnit( GAME_SYSTEM* gs, u32 unitID );
extern u32					Get_GS_ClactResIdx( GAME_SYSTEM* gs, u32 resID );

#define PLT_2D_COL_WHITE	(15)
#define PLT_2D_COL_BLACK	(1)
#define PLT_2D_COL_NULL		(0)

#define PLAYICON_FRM		(GFL_BG_FRAME3_M)
#define TEXT_FRM			(GFL_BG_FRAME3_S)
#define MASK_FRM			(GFL_BG_FRAME2_S)
#define PLAYICON_FRM_PRI	(0)
#define TEXT_FRM_PRI		(1)
#define MASK_FRM_PRI		(0)
#define PLAYICON_PLTT		(0)
#define TEXT_PLTT			(15)
#define MASK_PLTT			(1)
#define MAP_PLTT			(1)
#define G2D_FONT_COL		(0x7fff)
#define G2D_FONTSELECT_COL	(0x001f)
#define PLTT_DATSIZ			(16*2)
#define CLACT_WKSIZ_MAPOBJ	(64)
#define CLACT_WKSIZ_STATUS	(64)

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

