//============================================================================================
/**
 * @file	fld_act.c
 * @brief	フィールドアクトシステム
 */
//============================================================================================
#include "gflib.h"

#include "setup.h"
#include "system\gfl_use.h"
#include "arc_def.h"

#include "fld_act.h"
//注）GFL_G3D_SYSTEM, GFL_BBD上での動作

//------------------------------------------------------------------
/**
 *
 * @brief	型宣言
 *
 */
//------------------------------------------------------------------
#define FLD_BBDACT_ACTMAX	(253)
#define WORK_SIZ			(8)

typedef struct {
	FLD_ACTSYS*	fldActSys;
	u16			work[WORK_SIZ];
}FLD_ACT_WORK;

struct _FLD_ACTSYS {
	HEAPID					heapID;
	GAME_SYSTEM*			gs;
	u16						cameraRotate;
	GFL_BBDACT_RESUNIT_ID	bbdActResUnitID;
	u16						bbdActResCount;
	GFL_BBDACT_ACTUNIT_ID	bbdActActUnitID;
	u16						bbdActActCount;
	FLD_ACT_WORK			actWork[FLD_BBDACT_ACTMAX];
};

#include "test_graphic/fld_act.naix"

static void	initActWork( FLD_ACTSYS* fldActSys, FLD_ACT_WORK* actWork );
static void	calcCameraRotate( FLD_ACTSYS* fldActSys );
//------------------------------------------------------------------
/**
 *
 * @brief	セットアップデータ
 *
 */
//------------------------------------------------------------------
static const GFL_BBDACT_RESDATA testResTable[] = {
	{ ARCID_FLDACT, NARC_fld_act_hero_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_achamo_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_artist_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_badman_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_beachgirl_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_idol_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_lady_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_oldman1_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_policeman_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_rivel_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDACT, NARC_fld_act_waiter_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
};

 

static const GFL_BBDACT_ANM walkLAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 1, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM walkRAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 1, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM walkUAnm[] = {
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM walkDAnm[] = {
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM testAnm1[] = {
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 1, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 4, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 5, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 6, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM testAnm2[] = {
	{ 1, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 16 },
	{ 3, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 24 },
	{ 4, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 32 },
	{ 5, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 48 },
	{ 6, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 7, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 8, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 9, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 10, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ GFL_BBDACT_ANMCOM_LOOP, 0, 0, GFL_BBDACT_ANMLOOPPARAM( 4, 5 ) },
	{ 11, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 64 },
	{ GFL_BBDACT_ANMCOM_CHG, 0, 0, 0 },
};
static const GFL_BBDACT_ANM testAnm3[] = {
	{ 0, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 16, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 32, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ 64, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 8 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 1 },
};

enum {
	ACTWALK_UP = 0,
	ACTWALK_DOWN,
	ACTWALK_LEFT,
	ACTWALK_RIGHT,
};

static const GFL_BBDACT_ANM* testAnmTable[] = { walkUAnm, walkDAnm, walkLAnm, walkRAnm };

static void testFunc( GFL_BBDACT_SYS* bbdActSys, int actIdx, void* work )
{
	GFL_BBD_SYS*	bbdSys = GFL_BBDACT_GetBBDSystem( bbdActSys );
	FLD_ACTSYS*		fldActSys = (FLD_ACTSYS*)work;
	FLD_ACT_WORK*	actWork = &fldActSys->actWork[actIdx];

	if( actWork->work[0] == 0 ){
		u32	timer = GFUser_GetPublicRand(8);

		actWork->work[0] = (timer+2) * 30;
		actWork->work[1] = GFUser_GetPublicRand( 0x10000 );
	} else {
		actWork->work[0]--;
	}
	{
		VecFx32 nowTrans, nextTrans, vecMove, vecGround, rotVec;
		u16	 theta = actWork->work[1];
		fx32 speed = FX32_ONE;
		BOOL mvf;

		{
			u16	 dir = actWork->work[1] - actWork->fldActSys->cameraRotate;
			if( (dir > 0x2000)&&(dir < 0x6000)){
				//OS_Printf("右　rotate = %x\n", dir );
				GFL_BBDACT_SetAnimeIdxContinue( bbdActSys, actIdx, ACTWALK_RIGHT );
			} else if( (dir >= 0x6000)&&(dir <= 0xa000)){
				//OS_Printf("上　rotate = %x\n", dir );
				GFL_BBDACT_SetAnimeIdxContinue( bbdActSys, actIdx, ACTWALK_UP );
			} else if( (dir > 0xa000)&&(dir < 0xe000)){
				//OS_Printf("左　rotate = %x\n", dir );
				GFL_BBDACT_SetAnimeIdxContinue( bbdActSys, actIdx, ACTWALK_LEFT );
			} else {
				//OS_Printf("下　rotate = %x\n", dir );
				GFL_BBDACT_SetAnimeIdxContinue( bbdActSys, actIdx, ACTWALK_DOWN );
			}
		}
		GFL_BBD_GetObjectTrans( bbdSys, actIdx, &nowTrans );

		vecMove.x = FX_SinIdx( theta );
		vecMove.y = 0;
		vecMove.z = FX_CosIdx( theta );

		GetGroundMovePos( Get_GS_SceneMap( fldActSys->gs ), &nowTrans, &vecMove, &nextTrans ); 
		//範囲外コントロール
		mvf = CheckGroundOutRange( Get_GS_SceneMap( fldActSys->gs ), &nextTrans );
		if( mvf == TRUE ){ 
			nextTrans.y += FX32_ONE*7;	//補正
			GFL_BBD_SetObjectTrans( bbdSys, actIdx, &nextTrans );
		} else {
			actWork->work[0] = 0;
		}
	}
}

#define TEST_NPC_SETNUM	(250)
static void testSetUp( FLD_ACTSYS* fldActSys )
{
	GFL_BBDACT_SYS* bbdActSys = Get_GS_BillboardActSys( fldActSys->gs );	
	GFL_BBDACT_ACTDATA* actData;
	GFL_BBDACT_ACTUNIT_ID actUnitID;
	int		i, objIdx;
	VecFx32	trans;
	u8		alpha;
	BOOL	drawEnable;
	u16		setActNum;

	//リソースセットアップ
	fldActSys->bbdActResCount = NELEMS(testResTable);
	fldActSys->bbdActResUnitID = GFL_BBDACT_AddResourceUnit(	bbdActSys, 
																testResTable, 
																fldActSys->bbdActResCount );
	fldActSys->bbdActActCount = FLD_BBDACT_ACTMAX;

	//ＮＰＣアクターセットアップ
	{
		u16	setActNum = TEST_NPC_SETNUM;
		GFL_BBDACT_ACTDATA* actData = GFL_HEAP_AllocClearMemory( fldActSys->heapID,
													setActNum*sizeof(GFL_BBDACT_ACTDATA) );
		for( i=0; i<setActNum; i++ ){
			actData[i].resID = GFUser_GetPublicRand( 10 )+1;
			actData[i].sizX = FX16_ONE*8-1;
			actData[i].sizY = FX16_ONE*8-1;
	
			do{
				actData[i].trans.x = (GFUser_GetPublicRand( 32 ) - ( 32/2 )) * FX32_ONE*16;
				actData[i].trans.y = 0;
				actData[i].trans.z = (GFUser_GetPublicRand( 32 ) - ( 32/2 )) * FX32_ONE*16;
			}while
			( CheckGroundOutRange(Get_GS_SceneMap(fldActSys->gs), &actData[i].trans) == FALSE );
		
			
			actData[i].alpha = 31;
			actData[i].drawEnable = TRUE;
			actData[i].lightMask = GFL_BBD_LIGHTMASK_01;
			actData[i].func = testFunc;
			actData[i].work = fldActSys;
		}
		fldActSys->bbdActActUnitID = GFL_BBDACT_AddAct(	bbdActSys, fldActSys->bbdActResUnitID,
														actData, setActNum );
		for( i=0; i<setActNum; i++ ){
			GFL_BBDACT_SetAnimeTable( bbdActSys, fldActSys->bbdActActUnitID+i, 
										testAnmTable, NELEMS(testAnmTable) );
			GFL_BBDACT_SetAnimeIdxOn( bbdActSys, fldActSys->bbdActActUnitID+i, 0 );
		}
		GFL_HEAP_FreeMemory( actData );
	}
}

static void testRelease( FLD_ACTSYS* fldActSys )
{
	GFL_BBDACT_SYS* bbdActSys = Get_GS_BillboardActSys( fldActSys->gs );	
	u16	setActNum = FLD_BBDACT_ACTMAX;

	GFL_BBDACT_RemoveAct( bbdActSys, fldActSys->bbdActActUnitID, TEST_NPC_SETNUM );
		
	GFL_BBDACT_RemoveResourceUnit(	bbdActSys, 
									fldActSys->bbdActResUnitID, fldActSys->bbdActResCount );
}

//------------------------------------------------------------------
/**
 * @brief	フィールドアクトシステム作成
 */
//------------------------------------------------------------------
FLD_ACTSYS*	CreateFieldActSys( GAME_SYSTEM* gs, HEAPID heapID )
{
	FLD_ACTSYS* fldActSys = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLD_ACTSYS) );
	int	i;

	fldActSys->heapID = heapID;
	fldActSys->gs = gs;

	for( i=0; i<FLD_BBDACT_ACTMAX; i++ ){ initActWork( fldActSys, &fldActSys->actWork[i] ); }

	testSetUp( fldActSys );	//テスト

	return fldActSys;
}

//------------------------------------------------------------------
/**
 * @brief	フィールドアクトシステム破棄
 */
//------------------------------------------------------------------
void	DeleteFieldActSys( FLD_ACTSYS* fldActSys )
{
	testRelease( fldActSys );	//テスト
	GFL_HEAP_FreeMemory( fldActSys ); 
}

//------------------------------------------------------------------
/**
 * @brief	フィールドアクト動作関数
 */
//------------------------------------------------------------------
void	MainFieldActSys( FLD_ACTSYS* fldActSys )
{
	//カメラ回転算出(ビルボードそのものには関係ない。アニメ向きの変更をするのに参照)
	VecFx32 vec, camPos, target;

	GFL_G3D_CAMERA_GetPos( Get_GS_G3Dcamera( fldActSys->gs, MAINCAMERA_ID ), &camPos );
	GFL_G3D_CAMERA_GetTarget( Get_GS_G3Dcamera( fldActSys->gs, MAINCAMERA_ID ), &target );

	VEC_Subtract( &target, &camPos, &vec );
	
	fldActSys->cameraRotate = FX_Atan2Idx( -vec.z, vec.x ) - 0x4000;
}

//------------------------------------------------------------------
/**
 * @brief	フィールドアクトワーク初期化
 */
//------------------------------------------------------------------
static void	initActWork( FLD_ACTSYS* fldActSys, FLD_ACT_WORK* actWork )
{
	int i;

	actWork->fldActSys = fldActSys;

	for( i=0; i<WORK_SIZ; i++ ){
		actWork->work[i] = 0;
	}
}


