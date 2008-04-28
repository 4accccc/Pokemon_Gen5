//============================================================================================
/**
 * @file	fld_act.c
 * @brief	フィールドアクトシステム
 */
//============================================================================================
#include "gflib.h"

#include "setup.h"
#include "include\system\gfl_use.h"

//注）GFL_G3D_SYSTEM, GFL_BBD上での動作

//------------------------------------------------------------------
/**
 *
 * @brief	型宣言
 *
 */
//------------------------------------------------------------------
#define FLD_BBDACT_RESMAX	(64)
#define FLD_BBDACT_ACTMAX	(256)
#define WORK_SIZ			(8)

typedef struct {
	FLD_ACTSYS*	fldActSys;
	u16			work[WORK_SIZ];
}FLD_ACT_WORK;

struct _FLD_ACTSYS {
	HEAPID					heapID;
	GFL_G3D_CAMERA*			g3Dcamera;
	u16						cameraRotate;
	GFL_BBDACT_SYS*			bbdActSys;
	GFL_BBDACT_RESUNIT_ID	bbdActResUnitID;
	u16						bbdActResCount;
	GFL_BBDACT_ACTUNIT_ID	bbdActActUnitID;
	u16						bbdActActCount;
	FLD_ACT_WORK			actWork[FLD_BBDACT_ACTMAX];
};

#include "graphic_data/test_graphic/fld_act.naix"

static void	initActWork( FLD_ACTSYS* fldActSys, FLD_ACT_WORK* actWork );
static void	calcCameraRotate( FLD_ACTSYS* fldActSys );
//------------------------------------------------------------------
/**
 *
 * @brief	セットアップデータ
 *
 */
//------------------------------------------------------------------
static GFL_BBDACT_RESUNIT_ID testResUnitID;
static GFL_BBDACT_ACTUNIT_ID testActUnitID;

static const GFL_BBDACT_RESDATA testResTable[] = {
{ ARCID_FLDACT, NARC_fld_act_achamo_nsbtx, GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32 },
{ ARCID_FLDACT, NARC_fld_act_artist_nsbtx, GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32 },
{ ARCID_FLDACT, NARC_fld_act_badman_nsbtx, GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32 },
{ ARCID_FLDACT, NARC_fld_act_beachgirl_nsbtx, GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32 },
{ ARCID_FLDACT, NARC_fld_act_idol_nsbtx, GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32 },
{ ARCID_FLDACT, NARC_fld_act_lady_nsbtx, GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32 },
{ ARCID_FLDACT, NARC_fld_act_oldman1_nsbtx, GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32 },
{ ARCID_FLDACT, NARC_fld_act_policeman_nsbtx, GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32 },
{ ARCID_FLDACT, NARC_fld_act_rivel_nsbtx, GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32 },
{ ARCID_FLDACT, NARC_fld_act_waiter_nsbtx, GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32 },
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
	FLD_ACT_WORK*	actWork = (FLD_ACT_WORK*)work;

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
#if 0
		VEC_MultAdd( speed, &vecMove, &nowTrans, &nextTrans );
		GetGroundPlaneHeight( &nextTrans, &nextTrans.y );
		nextTrans.y += FX32_ONE*7;	//補正
#else
		GetGroundMovePos( &nowTrans, &vecMove, &nextTrans ); 
		nextTrans.y += FX32_ONE*7;	//補正
#endif
		if( CheckGroundOutRange( &nextTrans ) == TRUE ){
			GFL_BBD_SetObjectTrans( bbdSys, actIdx, &nextTrans );
		} else {
			actWork->work[0] = 0;
		}
	}
}

static void testSetUp( FLD_ACTSYS* fldActSys )
{
	GFL_BBDACT_ACTDATA* actData;
	GFL_BBDACT_ACTUNIT_ID actUnitID;
	int		i, objIdx;
	VecFx32	trans;
	u8		alpha;
	BOOL	drawEnable;
	u16		setActNum = FLD_BBDACT_ACTMAX;

	//リソースセットアップ
	fldActSys->bbdActResCount = NELEMS(testResTable);
	fldActSys->bbdActResUnitID = GFL_BBDACT_AddResourceUnit(	fldActSys->bbdActSys, 
																testResTable, 
																fldActSys->bbdActResCount );
	//アクターセットアップ
	fldActSys->bbdActActCount = FLD_BBDACT_ACTMAX;
	actData = GFL_HEAP_AllocClearMemory( fldActSys->heapID, 
										fldActSys->bbdActActCount*sizeof(GFL_BBDACT_ACTDATA) );

	for( i=0; i<setActNum; i++ ){
		actData[i].resID = GFUser_GetPublicRand( 10 );
		actData[i].sizX = FX16_ONE*8-1;
		actData[i].sizY = FX16_ONE*8-1;

		do{
			actData[i].trans.x = (GFUser_GetPublicRand( 32 ) - ( 32/2 )) * FX32_ONE*16;
			actData[i].trans.y = 0;
			actData[i].trans.z = (GFUser_GetPublicRand( 32 ) - ( 32/2 )) * FX32_ONE*16;
		}while( CheckGroundOutRange( &actData[i].trans ) == FALSE );
		
		actData[i].alpha = 31;
		actData[i].drawEnable = TRUE;
		actData[i].func = testFunc;
		actData[i].work = &fldActSys->actWork[i];
	}
	fldActSys->bbdActActUnitID = GFL_BBDACT_AddAct( fldActSys->bbdActSys, 
													testResUnitID, 
													actData,
													fldActSys->bbdActActCount );
	for( i=0; i<setActNum; i++ ){
		GFL_BBDACT_SetAnimeTable( fldActSys->bbdActSys, fldActSys->bbdActResUnitID+i, 
									testAnmTable, NELEMS(testAnmTable) );
		GFL_BBDACT_SetAnimeIdxOn( fldActSys->bbdActSys, fldActSys->bbdActActUnitID+i, 0 );
	}

	GFL_HEAP_FreeMemory( actData );
}

static void testRelease( FLD_ACTSYS* fldActSys )
{
	u16	setActNum = FLD_BBDACT_ACTMAX;

	GFL_BBDACT_RemoveAct( fldActSys->bbdActSys, testActUnitID, setActNum );
	GFL_BBDACT_RemoveResourceUnit( fldActSys->bbdActSys, testResUnitID, NELEMS(testResTable) );
}

//------------------------------------------------------------------
/**
 * @brief	フィールドアクトシステム作成
 */
//------------------------------------------------------------------
FLD_ACTSYS*	CreateFieldActSys( GFL_G3D_CAMERA* g3Dcamera, HEAPID heapID )
{
	FLD_ACTSYS* fldActSys = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLD_ACTSYS) );
	int	i;

	fldActSys->heapID = heapID;
	fldActSys->g3Dcamera = g3Dcamera;
	fldActSys->bbdActSys = GFL_BBDACT_CreateSys( FLD_BBDACT_RESMAX, FLD_BBDACT_ACTMAX, heapID );

	calcCameraRotate( fldActSys );
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
	GFL_BBDACT_DeleteSys( fldActSys->bbdActSys );
	GFL_HEAP_FreeMemory( fldActSys ); 
}

//------------------------------------------------------------------
/**
 * @brief	フィールドアクト動作関数
 */
//------------------------------------------------------------------
void	MainFieldActSys( FLD_ACTSYS* fldActSys )
{
	calcCameraRotate( fldActSys );
	GFL_BBDACT_Main( fldActSys->bbdActSys );
}

//------------------------------------------------------------------
/**
 * @brief	フィールドアクト描画関数
 */
//------------------------------------------------------------------
void	DrawFieldActSys( FLD_ACTSYS* fldActSys )
{
	GFL_BBDACT_Draw( fldActSys->bbdActSys, fldActSys->g3Dcamera );
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

//------------------------------------------------------------------
/**
 * @brief	カメラ回転算出(ビルボードそのものには関係ない。アニメ向きの変更をするのに参照)
 */
//------------------------------------------------------------------
static void	calcCameraRotate( FLD_ACTSYS* fldActSys )
{
	VecFx32 vec, camPos, target;

	GFL_G3D_CAMERA_GetPos( fldActSys->g3Dcamera, &camPos );
	GFL_G3D_CAMERA_GetTarget( fldActSys->g3Dcamera, &target );

	VEC_Subtract( &target, &camPos, &vec );

	fldActSys->cameraRotate = FX_Atan2Idx( -vec.z, vec.x ) - 0x4000;
}

