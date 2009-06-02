//============================================================================================
/**
 * @file	sample2.c
 * @brief	
 * @author	
 * @date	
 */
//============================================================================================
#include <wchar.h>
#include "gflib.h"
#include "system/gfl_use.h"
#include "arc_def.h"
#include "system/main.h"

#include "debug_common.h"
#include "system/el_scoreboard.h"

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	
 *
 *
 *
 *
 *
 */
//============================================================================================
//============================================================================================
/**
 *
 * @brief	構造体定義
 *
 */
//============================================================================================
typedef struct {
	HEAPID							heapID;
	DWS_SYS*						dws;

	int									seq;
	STRBUF*							strBuf;

	GFL_G3D_UTIL*				g3Dutil;
	u16									g3DutilUnitIdx;

	EL_SCOREBOARD_TEX*	elb_tex[2];

	u16									targetObjID;

	int									timer;
}SAMPLE2_WORK;

//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	呼び出し用PROC定義
 *
 *
 *
 *
 *
 */
//============================================================================================
static BOOL	sample2(SAMPLE2_WORK* sw);

//------------------------------------------------------------------
/**
 * @brief	proc関数
 */
//------------------------------------------------------------------
static GFL_PROC_RESULT Sample2Proc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SAMPLE2_WORK* sw = NULL;

	sw = GFL_PROC_AllocWork(proc, sizeof(SAMPLE2_WORK), GFL_HEAPID_APP);
  GFL_STD_MemClear(sw, sizeof(SAMPLE2_WORK));
  sw->heapID = GFL_HEAPID_APP;
	sw->seq = 0;

	sw->dws = DWS_SYS_Setup(sw->heapID);

	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT Sample2Proc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SAMPLE2_WORK*	sw;
	sw = mywk;

	DWS_SYS_Framework(sw->dws);

	sw->timer++;
	if(sample2(sw) == TRUE){
		return GFL_PROC_RES_CONTINUE;
	}
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
static GFL_PROC_RESULT Sample2Proc_End(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SAMPLE2_WORK*	sw;
	sw = mywk;

	DWS_SYS_Delete(sw->dws);

	GFL_PROC_FreeWork(proc);

	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------
/**
 * @brief	procテーブル
 */
//------------------------------------------------------------------
const GFL_PROC_DATA DebugWatanabeSample2ProcData = {
	Sample2Proc_Init,
	Sample2Proc_Main,
	Sample2Proc_End,
};





//============================================================================================
/**
 *
 *
 *
 *
 *
 * @brief	メイン
 *
 *
 *
 *
 *
 */
//============================================================================================
#define STRBUF_SIZE		(64*2)

#include "arc/elboard_test.naix"

enum {
	G3DRES_ELBOARD_BMD = 0,
	G3DRES_ELBOARD_BTX,
	G3DRES_ELBOARD_BCA,
	G3DRES_ELBOARD_BTA,
	G3DRES_ELBOARD2_BMD,
	G3DRES_ELBOARD2_BTX,
	G3DRES_ELBOARD2_BTA,
};

static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
	{	ARCID_ELBOARD_TEST, NARC_elboard_test_elboard_test_nsbmd, GFL_G3D_UTIL_RESARC },
	{	ARCID_ELBOARD_TEST, NARC_elboard_test_elboard_test_nsbtx, GFL_G3D_UTIL_RESARC },
	{	ARCID_ELBOARD_TEST, NARC_elboard_test_elboard_test_nsbca, GFL_G3D_UTIL_RESARC },
	{	ARCID_ELBOARD_TEST, NARC_elboard_test_elboard_test_nsbta, GFL_G3D_UTIL_RESARC },
	{	ARCID_ELBOARD_TEST, NARC_elboard_test_elboard2_test_nsbmd, GFL_G3D_UTIL_RESARC },
	{	ARCID_ELBOARD_TEST, NARC_elboard_test_elboard2_test_nsbtx, GFL_G3D_UTIL_RESARC },
	{	ARCID_ELBOARD_TEST, NARC_elboard_test_elboard2_test_nsbta, GFL_G3D_UTIL_RESARC },
};

static const GFL_G3D_UTIL_ANM g3Dutil_anm1Tbl[] = {
	{ G3DRES_ELBOARD_BCA, 0 },
	{ G3DRES_ELBOARD_BTA, 0 },
};

static const GFL_G3D_UTIL_ANM g3Dutil_anm2Tbl[] = {
	{ G3DRES_ELBOARD2_BTA, 0 },
};

enum {
	G3DOBJ_ELBOARD1 = 0,
	G3DOBJ_ELBOARD2,
};

static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
	{ G3DRES_ELBOARD_BMD, 0, G3DRES_ELBOARD_BTX, g3Dutil_anm1Tbl, NELEMS(g3Dutil_anm1Tbl) },
	{ G3DRES_ELBOARD2_BMD, 0, G3DRES_ELBOARD2_BTX, g3Dutil_anm2Tbl, NELEMS(g3Dutil_anm2Tbl) },
};

static const GFL_G3D_UTIL_SETUP g3Dutil_setup = {
	g3Dutil_resTbl, NELEMS(g3Dutil_resTbl),
	g3Dutil_objTbl, NELEMS(g3Dutil_objTbl),
};

static const GFL_G3D_OBJSTATUS g3DobjStatus1 = {
	{ 0, 0, 0 },																				//座標
	{ FX32_ONE, FX32_ONE, FX32_ONE },										//スケール
	{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },	//回転
};

//============================================================================================
static BOOL	sample2(SAMPLE2_WORK* sw)
{
	switch(sw->seq){

	case 0:
		sw->strBuf = GFL_STR_CreateBuffer(STRBUF_SIZE, sw->heapID);
		GFL_MSG_GetString(DWS_GetMsgManager(sw->dws), DEBUG_TETSU_STR0_5, sw->strBuf);
		sw->timer = 0;

		//３Ｄオブジェクト作成
		{
			u16						objIdx, elboard1Idx;
			GFL_G3D_OBJ*	g3Dobj;
			GFL_G3D_RES*	g3Dtex;
			int i, anmCount;

			//リソース作成
			sw->g3Dutil = GFL_G3D_UTIL_Create
											(NELEMS(g3Dutil_resTbl), NELEMS(g3Dutil_objTbl), sw->heapID );
			sw->g3DutilUnitIdx = GFL_G3D_UTIL_AddUnit( sw->g3Dutil, &g3Dutil_setup );
	
			//アニメーションを有効にする
			objIdx = GFL_G3D_UTIL_GetUnitObjIdx(sw->g3Dutil, sw->g3DutilUnitIdx);
			{
				elboard1Idx = objIdx + G3DOBJ_ELBOARD1;
				g3Dobj = GFL_G3D_UTIL_GetObjHandle(sw->g3Dutil, elboard1Idx);
	
				anmCount = GFL_G3D_OBJECT_GetAnimeCount(g3Dobj);
				for( i=0; i<anmCount; i++ ){ GFL_G3D_OBJECT_EnableAnime(g3Dobj, i); } 
	
				g3Dtex =	GFL_G3D_RENDER_GetG3DresTex(GFL_G3D_OBJECT_GetG3Drnd(g3Dobj));
				sw->elb_tex[0] = ELBOARD_TEX_Add(g3Dtex, sw->strBuf, sw->heapID);
			} 
			{
				elboard1Idx = objIdx + G3DOBJ_ELBOARD2;
				g3Dobj = GFL_G3D_UTIL_GetObjHandle(sw->g3Dutil, elboard1Idx);
	
				anmCount = GFL_G3D_OBJECT_GetAnimeCount(g3Dobj);
				for( i=0; i<anmCount; i++ ){ GFL_G3D_OBJECT_EnableAnime(g3Dobj, i); } 
	
				g3Dtex =	GFL_G3D_RENDER_GetG3DresTex(GFL_G3D_OBJECT_GetG3Drnd(g3Dobj));
				sw->elb_tex[1] = ELBOARD_TEX_Add(g3Dtex, sw->strBuf, sw->heapID);
			}
			sw->targetObjID = G3DOBJ_ELBOARD1;
		}
		sw->seq++;
		break;

	case 1:
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){ sw->seq++; }
		if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
			if(sw->targetObjID == G3DOBJ_ELBOARD2)	{ sw->targetObjID = G3DOBJ_ELBOARD1; }
			else																		{ sw->targetObjID = G3DOBJ_ELBOARD2; }
		}
		ELBOARD_TEX_Main(sw->elb_tex[0]);
		ELBOARD_TEX_Main(sw->elb_tex[1]);

		//３Ｄ描画
		GFL_G3D_DRAW_Start();			//描画開始
		GFL_G3D_DRAW_SetLookAt();	//カメラグローバルステート設定		
		{
			u16						objIdx, elboard1Idx;
			GFL_G3D_OBJ*	g3Dobj;
			int i, anmCount;

			objIdx = GFL_G3D_UTIL_GetUnitObjIdx(sw->g3Dutil, sw->g3DutilUnitIdx );
			elboard1Idx = objIdx + sw->targetObjID;
			g3Dobj = GFL_G3D_UTIL_GetObjHandle(sw->g3Dutil, elboard1Idx);

			GFL_G3D_DRAW_DrawObject(g3Dobj, &g3DobjStatus1);

			anmCount = GFL_G3D_OBJECT_GetAnimeCount(g3Dobj);
			for( i=0; i<anmCount; i++ ){ GFL_G3D_OBJECT_LoopAnimeFrame(g3Dobj, i, FX32_ONE ); } 
		}
		GFL_G3D_DRAW_End();				//描画終了（バッファスワップ）					
		break;

	case 2:
		ELBOARD_TEX_Delete(sw->elb_tex[1]);
		ELBOARD_TEX_Delete(sw->elb_tex[0]);

		GFL_G3D_UTIL_DelUnit(sw->g3Dutil, sw->g3DutilUnitIdx);
		GFL_G3D_UTIL_Delete(sw->g3Dutil);

		GFL_STR_DeleteBuffer(sw->strBuf);

		return FALSE;
	}
	return TRUE;
}

