//======================================================================
/**
 * @file	fldmmdl_blact.c
 * @brief	フィールド動作モデル ビルボードアクター管理
 * @author	kagaya
 * @data	05.07.13
 */
//======================================================================
#include "fldmmdl.h"
#include "test_graphic/fld_act.naix"

#ifdef FLDMMDL_BLACT_HEAD3_TEST
#include "test_graphic/fldmmdl_btx.naix"
#endif

//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	FLDMMDL_BLACTCONT
//--------------------------------------------------------------
struct _TAG_FLDMMDL_BLACTCONT{
	GFL_BBDACT_SYS *pBbdActSys;
	GFL_BBDACT_RESUNIT_ID bbdActResUnitID;
	GFL_BBDACT_ACTUNIT_ID bbdActActUnitID;
	u16 bbdActResCount;
	u16 bbdActActCount;
	
	FLDMMDLSYS *fmmdlsys;
};

//======================================================================
//	proto
//======================================================================
static void BlActFunc( GFL_BBDACT_SYS *bbdActSys, int actIdx, void *work );

//test func
#if 0
static void testFunc( GFL_BBDACT_SYS* bbdActSys, int actIdx, void *work );
static void testJikiFunc(
	GFL_BBDACT_SYS* bbdActSys, int actIdx, void *work );
#endif
//test data
const GFL_BBDACT_RESDATA testResTable[];
const GFL_BBDACT_ANM *testAnmTable[];
const GFL_BBDACT_ANM* playerBBDactAnmTable[];
const u32 testResTableMax;
const u32 testAnmTableMax;
const u32 playerBBDactAnmTableMax;

//======================================================================
//	フィールド動作モデル　ビルボードアクター管理
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDL_BLACTCONT ビルボードアクター管理　セットアップ
 * @param	fmmdlsys	構築済みのFLDMMDLSYS
 * @param	pBbdActSys	構築済みのGFL_BBDACT_SYS
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_BLACTCONT_Setup(
	FLDMMDLSYS *fmmdlsys, GFL_BBDACT_SYS *pBbdActSys )
{
	HEAPID heapID;
	GFL_BBDACT_ACTDATA *actData;
	GFL_BBDACT_ACTUNIT_ID actUnitID;
	FLDMMDL_BLACTCONT *pBlActCont;
	
	heapID = FLDMMDLSYS_GetHeapID( fmmdlsys );
	pBlActCont = GFL_HEAP_AllocClearMemory(
			heapID, sizeof(FLDMMDL_BLACTCONT) );
	
	pBlActCont->fmmdlsys = fmmdlsys;
	pBlActCont->pBbdActSys = pBbdActSys;
	
	pBlActCont->bbdActResCount = testResTableMax;
	pBlActCont->bbdActResUnitID = GFL_BBDACT_AddResourceUnit(
			pBbdActSys, testResTable, pBlActCont->bbdActResCount );
	
	FLDMMDLSYS_SetBlActCont( fmmdlsys, pBlActCont );
}

//--------------------------------------------------------------
/**
 * FLDMMDL_BLACTCONT ビルボードアクター管理　削除
 * @param	fmmdlsys	FLDMMDLSYS
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_BLACTCONT_Release( FLDMMDLSYS *fmmdlsys )
{
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( fmmdlsys );
	
	GFL_BBDACT_RemoveResourceUnit( pBlActCont->pBbdActSys,
		pBlActCont->bbdActResUnitID, pBlActCont->bbdActResCount );
	GFL_HEAP_FreeMemory( pBlActCont );
	
	FLDMMDLSYS_SetBlActCont( fmmdlsys, NULL );
}

//======================================================================
//	フィールド動作モデル ビルボードアクター追加
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードアクター 追加
 * @param	fmmdl	FLDMMDL
 * @param	resID	追加するアクターで使用するリソースID
 * @retval	GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
GFL_BBDACT_ACTUNIT_ID FLDMMDL_BLACTCONT_AddActor( FLDMMDL *fmmdl, u32 resID )
{
	VecFx32 pos;
	GFL_BBDACT_ACTDATA actData;
	GFL_BBDACT_ACTUNIT_ID actID;
	FLDMMDLSYS *fmmdlsys = (FLDMMDLSYS*)FLDMMDL_GetFldMMdlSys( fmmdl );
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( fmmdlsys );
	
	FLDMMDL_GetDrawVectorPos( fmmdl, &pos );
	
	actData.resID = resID;
	actData.sizX = FX16_ONE*8-1;
	actData.sizY = FX16_ONE*8-1;
	actData.trans = pos;
	actData.alpha = 31;
	actData.drawEnable = TRUE;
	actData.lightMask = GFL_BBD_LIGHTMASK_01;
	actData.work = fmmdl;
	actData.func = BlActFunc;
	
	actID = GFL_BBDACT_AddAct(
		pBlActCont->pBbdActSys, pBlActCont->bbdActResUnitID, &actData, 1 );
	
	if( resID == 0 ){
		GFL_BBDACT_SetAnimeTable( pBlActCont->pBbdActSys,
			actID, playerBBDactAnmTable, playerBBDactAnmTableMax );
	}else{
		GFL_BBDACT_SetAnimeTable( pBlActCont->pBbdActSys,
			actID, testAnmTable, testAnmTableMax );
	}
	
	GFL_BBDACT_SetAnimeIdxOn(
			pBlActCont->pBbdActSys, actID, 0 );
	return( actID );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター 削除
 * @param	fmmdl	FLDMMDL*
 * @param	actID GFL_BBDACT_ACTUNIT_ID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_BLACTCONT_DeleteActor( FLDMMDL *fmmdl, u32 actID )
{
	FLDMMDLSYS *pFMMdlSys = (FLDMMDLSYS*)FLDMMDL_GetFldMMdlSys( fmmdl );
	FLDMMDL_BLACTCONT *pBlActCont = FLDMMDLSYS_GetBlActCont( pFMMdlSys );
	GFL_BBDACT_RemoveAct( pBlActCont->pBbdActSys, actID, 1 );
}

//======================================================================
//	フィールド動作モデル　ビルボードアクター動作
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードアクター動作関数
 * @param	bbdActSys	GFL_BBDACT_SYS
 * @param	actIdx		int
 * @param	work		void*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void BlActFunc( GFL_BBDACT_SYS *bbdActSys, int actIdx, void *work )
{
}

//======================================================================
//	フィールド動作モデル　ビルボードアクター管理　参照
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードアクター管理　GFL_BBDACT_SYS取得
 * @param	pBlActCont FLDMMDL_BLACTCONT
 * @retval	GFL_BBDACT_SYS
 */
//--------------------------------------------------------------
GFL_BBDACT_SYS * FLDMMDL_BLACTCONT_GetBbdActSys(
		FLDMMDL_BLACTCONT *pBlActCont )
{
	return( pBlActCont->pBbdActSys );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター管理 GFL_BBDACT_RESUNIT_ID取得
 * @param	pBlActCont FLDMMDL_BLACTCONT
 * @retval	GFL_BBDACT_RESUNIT_ID
 */
//--------------------------------------------------------------
GFL_BBDACT_RESUNIT_ID FLDMMDL_BLACTCONT_GetResUnitID(
		FLDMMDL_BLACTCONT *pBlActCont )
{
	return( pBlActCont->bbdActResUnitID );
}

//======================================================================
//	test func
//======================================================================
#if 0
typedef struct
{
	u8 init;
	u8 anm_dir;
	u8 anm_status;
	u8 anm_walk_next_frmidx;
	s8 anm_walk_stop_frame;
}TEST_DRAW_WORK;

static void testFunc( GFL_BBDACT_SYS* bbdActSys, int actIdx, void *work )
{
	u16 dir,anm_id,status;
	VecFx32 pos;
	int tbl[] = {0,4,8,12,16,20};
	FLDMMDL *fmmdl = work;
	TEST_DRAW_WORK *draw = FLDMMDL_GetDrawProcWork( fmmdl );
	
	dir = FLDMMDL_GetDirDisp( fmmdl );
	status = FLDMMDL_GetDrawStatus( fmmdl );
	anm_id = tbl[status];
	anm_id += dir;
	
	if( draw->init == FALSE ){
		draw->init = TRUE;
		draw->anm_dir = dir;
		draw->anm_status = status;
		draw->anm_walk_next_frmidx = 0;
		GFL_BBDACT_SetAnimeIdx( bbdActSys, actIdx, anm_id );
	}else if( draw->anm_dir != dir ){
		draw->anm_dir = dir;
		draw->anm_status = status;
		draw->anm_walk_next_frmidx = 0;
		GFL_BBDACT_SetAnimeIdx( bbdActSys, actIdx, anm_id );
	}else if( draw->anm_status != status ){
		GFL_BBDACT_SetAnimeIdx( bbdActSys, actIdx, anm_id );
		if( status >= DRAW_STA_WALK && status <= DRAW_STA_WALK_2F ){
			GFL_BBDACT_SetAnimeFrmIdx(
				bbdActSys, actIdx, draw->anm_walk_next_frmidx );
		}
		draw->anm_status = status;
	}
	
	if( draw->anm_status >= DRAW_STA_WALK &&
		draw->anm_status <= DRAW_STA_WALK_2F ){
		u8 frm = GFL_BBDACT_GetAnimeFrmIdx( bbdActSys, actIdx );
		if( frm < 2 ){ frm = 2; }
		else{ frm = 0; }
		draw->anm_walk_next_frmidx = frm;
	}
	
	FLDMMDL_GetDrawVectorPos( fmmdl, &pos );
	pos.y += FX32_ONE * 12;
//	pos.y += FX32_ONE * 7; //3
	
	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(bbdActSys), actIdx, &pos );
	
	if( FLDMMDL_CheckStatusBitMoveProcPause(fmmdl) == FALSE ||
		FLDMMDL_CheckStatusBitAcmd(fmmdl) == TRUE ){
		GFL_BBDACT_SetAnimeEnable( bbdActSys, actIdx, TRUE );
	}else{
		GFL_BBDACT_SetAnimeEnable( bbdActSys, actIdx, FALSE );
	}
}

static void testJikiFunc( GFL_BBDACT_SYS* bbdActSys, int actIdx, void *work )
{
	u16 dir,anm_id,status;
	VecFx32 pos;
#ifdef FLDMMDL_BLACT_HEAD3_TEST
	int tbl[] = {
		0,4,4,4,
		4,4,4,4,
		4,4,4,4,
	};
#else
	int tbl[] = {
		 0, 4, 8,12,
		16,20,24,28,
		32,36,40,44
	};
#endif
	
	FLDMMDL *fmmdl = work;
	TEST_DRAW_WORK *draw = FLDMMDL_GetDrawProcWork( fmmdl );
	
	dir = FLDMMDL_GetDirDisp( fmmdl );
	status = FLDMMDL_GetDrawStatus( fmmdl );
	anm_id = tbl[status];
	anm_id += dir;
	
	if( draw->init == FALSE ){
		draw->init = TRUE;
		draw->anm_dir = dir;
		draw->anm_status = status;
		draw->anm_walk_next_frmidx = 0;
		GFL_BBDACT_SetAnimeIdx( bbdActSys, actIdx, anm_id );
	}else if( draw->anm_dir != dir ){
		draw->anm_dir = dir;
		draw->anm_status = status;
		draw->anm_walk_next_frmidx = 0;
		GFL_BBDACT_SetAnimeIdx( bbdActSys, actIdx, anm_id );
	}else if( draw->anm_status != status ){
		if( draw->anm_status == DRAW_STA_DASH_4F ){
			draw->anm_walk_stop_frame++;
		}else{
			draw->anm_walk_stop_frame = 2;
		}
		
		if( draw->anm_walk_stop_frame > 1 ){
			draw->anm_walk_stop_frame = 0;
			
			switch( draw->anm_status ){
			case DRAW_STA_WALK_32F:
			case DRAW_STA_WALK_16F:
			case DRAW_STA_WALK_8F:
			case DRAW_STA_WALK_4F:
			case DRAW_STA_WALK_2F:
			case DRAW_STA_DASH_4F:
				{
					u8 frm = GFL_BBDACT_GetAnimeFrmIdx( bbdActSys, actIdx );
					if( frm <= 2 ){
						draw->anm_walk_next_frmidx = 2;
					}else{
						draw->anm_walk_next_frmidx = 0;
					}
				}
				break;
			}
			
			if( draw->anm_status == DRAW_STA_DASH_4F &&
				status == DRAW_STA_STOP ){
			}
			
			GFL_BBDACT_SetAnimeIdx( bbdActSys, actIdx, anm_id );
			
			switch( status ){
			case DRAW_STA_WALK_32F:
			case DRAW_STA_WALK_16F:
			case DRAW_STA_WALK_8F:
			case DRAW_STA_WALK_4F:
			case DRAW_STA_WALK_2F:
			case DRAW_STA_DASH_4F:
				GFL_BBDACT_SetAnimeFrmIdx(
					bbdActSys, actIdx, draw->anm_walk_next_frmidx );
				break;
			}
			
			draw->anm_status = status;
		}
	}
	
	FLDMMDL_GetDrawVectorPos( fmmdl, &pos );
	pos.y += FX32_ONE * 12;
//	pos.y += FX32_ONE * 7; //3
	
	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(bbdActSys), actIdx, &pos );
	
	if( FLDMMDL_CheckStatusBitMoveProcPause(fmmdl) == FALSE ||
		FLDMMDL_CheckStatusBitAcmd(fmmdl) == TRUE ){
		GFL_BBDACT_SetAnimeEnable( bbdActSys, actIdx, TRUE );
	}else{
		GFL_BBDACT_SetAnimeEnable( bbdActSys, actIdx, FALSE );
	}
}
#endif

//======================================================================
//	test data
//======================================================================
static const GFL_BBDACT_RESDATA testResTable[] = {
#ifdef FLDMMDL_BLACT_HEAD3_TEST
	{ ARCID_FLDMMDL_BTX, NARC_fldmmdl_btx_obj_kabi32_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
#else
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_hero_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x1024, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
#endif
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_achamo_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_badman_nsbtx, 
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_beachgirl_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_idol_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_lady_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_oldman1_nsbtx,
		GFL_BBD_TEXFMT_PAL16, GFL_BBD_TEXSIZ_32x512, 32, 32, GFL_BBDACT_RESTYPE_DATACUT },
	{ ARCID_FLDMAP_ACTOR, NARC_fld_act_policeman_nsbtx,
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

static const GFL_BBDACT_ANM* testAnmTable[] = {
	walkUAnm, walkDAnm, walkLAnm, walkRAnm,	//停止
	walkUAnm, walkDAnm, walkLAnm, walkRAnm,	//移動 32
	walkUAnm, walkDAnm, walkLAnm, walkRAnm,	//移動 16
	walkUAnm, walkDAnm, walkLAnm, walkRAnm,	//移動 8
	walkUAnm, walkDAnm, walkLAnm, walkRAnm,	//移動 4
	walkUAnm, walkDAnm, walkLAnm, walkRAnm,	//移動 2
};

//-----HERO
#ifdef FLDMMDL_BLACT_HEAD3_TEST	//三頭身テスト
static const GFL_BBDACT_ANM PCstopLAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopRAnm[] = {
	{ 4, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopUAnm[] = {
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopDAnm[] = {
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCwalkLAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkRAnm[] = {
	{ 5, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 4, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 6, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 4, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkUAnm[] = {
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkDAnm[] = {
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunLAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 16, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunRAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 16, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunUAnm[] = {
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunDAnm[] = {
	{ 12, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 13, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM* playerBBDactAnmTable[] = { 
	PCstopUAnm, PCstopDAnm, PCstopLAnm, PCstopRAnm,
	PCwalkUAnm, PCwalkDAnm, PCwalkLAnm, PCwalkRAnm,
	PCwalkUAnm, PCwalkDAnm, PCwalkLAnm, PCwalkRAnm,
	PCwalkUAnm, PCwalkDAnm, PCwalkLAnm, PCwalkRAnm,
};
#else	//二頭身
static const GFL_BBDACT_ANM PCstopLAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopRAnm[] = {
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopUAnm[] = {
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCstopDAnm[] = {
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCwalkLAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkRAnm[] = {
	{ 1, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 3, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 2, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkUAnm[] = {
	{ 9, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 9, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 0, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCwalkDAnm[] = {
	{ 22, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 23, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 21, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCrunLAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 16, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunRAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 16, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 14, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunUAnm[] = {
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 10, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 7, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCrunDAnm[] = {
	{ 12, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 13, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ 11, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_JMP, 0, 0, 0 },
};

static const GFL_BBDACT_ANM PCjumpLAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCjumpRAnm[] = {
	{ 15, GFL_BBDACT_ANMFLIP_ON, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCjumpUAnm[] = {
	{ 8, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};
static const GFL_BBDACT_ANM PCjumpDAnm[] = {
	{ 12, GFL_BBDACT_ANMFLIP_OFF, GFL_BBDACT_ANMFLIP_OFF, 4 },
	{ GFL_BBDACT_ANMCOM_END, 0, 0, 0 },
};

static const GFL_BBDACT_ANM* playerBBDactAnmTable[] = { 
	PCstopUAnm, PCstopDAnm, PCstopLAnm, PCstopRAnm,	//停止
	PCwalkUAnm, PCwalkDAnm, PCwalkLAnm, PCwalkRAnm,	//移動32
	PCwalkUAnm, PCwalkDAnm, PCwalkLAnm, PCwalkRAnm,	//移動16
	PCwalkUAnm, PCwalkDAnm, PCwalkLAnm, PCwalkRAnm,	//移動8
	PCrunUAnm, PCrunDAnm, PCrunLAnm, PCrunRAnm,		//移動4
	PCrunUAnm, PCrunDAnm, PCrunLAnm, PCrunRAnm,		//移動2
	PCrunUAnm, PCrunDAnm, PCrunLAnm, PCrunRAnm,		//移動6
	PCrunUAnm, PCrunDAnm, PCrunLAnm, PCrunRAnm,		//移動3
	PCrunUAnm, PCrunDAnm, PCrunLAnm, PCrunRAnm,		//移動7

	PCrunUAnm, PCrunDAnm, PCrunLAnm, PCrunRAnm,		//ダッシュ4

	PCjumpUAnm, PCjumpDAnm, PCjumpLAnm, PCjumpRAnm,	//
};
#endif

const u32 testResTableMax = NELEMS( testResTable );
const u32 testAnmTableMax = NELEMS( testAnmTable );
const u32 playerBBDactAnmTableMax = NELEMS( playerBBDactAnmTable );
