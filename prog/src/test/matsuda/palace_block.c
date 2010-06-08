//==============================================================================
/**
 * @file	palace_block.c
 * @brief	パレス：ブロック制御
 * @author	matsuda
 * @date	2009.01.14(水)
 */
//==============================================================================
#include <gflib.h>
#if PM_DEBUG
#include <procsys.h>
#include "system\main.h"
#include "arc_def.h"
#include "palace.naix"
#include "test/performance.h"


//==============================================================================
//	定数定義
//==============================================================================
///アクター最大数
#define ACT_MAX			(64)

#define ONE_GRID		(FX32_ONE * 8)

//--------------------------------------------------------------
//	3D
//--------------------------------------------------------------
#define DTCM_SIZE		(0x1000)

static const GFL_G3D_OBJSTATUS status0 = {
	{ 0, 0, 0 },								//座標
	{ FX32_ONE, FX32_ONE, FX32_ONE },		//スケール
	{ FX32_ONE, 0, 0, 0, FX32_ONE, 0, 0, 0, FX32_ONE },	//回転
};


//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
	VecFx32				trans;
	fx32				distance;
	fx32				cameraHeight;
	u16					cameraLength;
	u16					direction;
	VecFx32				transOffset;
	struct{
		u16 x;
		u16 y;
		u16 z;
		u16 padding;
	}angle;
}PB_CAMERA;

typedef struct {
	u16		seq;
	int debug_mode;
	int mode;			///<次のメニュー画面へ引き渡すモード

	GFL_G3D_UTIL*			g3Dutil;
	u16						g3DutilUnitIdx;
	GFL_G3D_OBJSTATUS		gira_status;
	
	GFL_G3D_CAMERA *g3d_camera;
	PB_CAMERA camera;
	
	int block_max;
	int base_size;		//8x8 12x12 16x16
}TITLE_WORK;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
GFL_PROC_RESULT PalaceBlockProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
GFL_PROC_RESULT PalaceBlockProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
GFL_PROC_RESULT PalaceBlockProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static void Local_VramSetting(TITLE_WORK *tw);
static void Local_BGFrameSetting(TITLE_WORK *tw);
static void Local_3DSetting(TITLE_WORK *tw);
static void Local_GirathinaLoad(TITLE_WORK *tw);
static void Local_Draw3D(TITLE_WORK *tw);
static void Local_GirathinaFree(TITLE_WORK *tw);
static void _PB_CameraMove(TITLE_WORK *tw);
static VecFx32 _BlockPosGet(TITLE_WORK *tw, int block_no);


//==============================================================================
//	データ
//==============================================================================
///タイトル画面呼び出しようのPROCデータ
const GFL_PROC_DATA PalaceBlockProcData = {
	PalaceBlockProcInit,
	PalaceBlockProcMain,
	PalaceBlockProcEnd,
};

//--------------------------------------------------------------
//	3D
//--------------------------------------------------------------
enum{
	G3DRES_GIRA_BMD = 0,
};

//読み込む3Dリソース
static const GFL_G3D_UTIL_RES g3Dutil_resTbl[] = {
#if 0 //2010.04.17(土)  不要なグラフィックデータを消した為、使用していたここも削除
	{ ARCID_PALACE, NARC_palace_cube_nsbmd, GFL_G3D_UTIL_RESARC },
#else
  0
#endif
};

//3Dオブジェクト設定テーブル
static const GFL_G3D_UTIL_OBJ g3Dutil_objTbl[] = {
	{
		G3DRES_GIRA_BMD, 			//モデルリソースID
		0, 							//モデルデータID(リソース内部INDEX)
		G3DRES_GIRA_BMD, 			//テクスチャリソースID
		NULL, 			//アニメテーブル(複数指定のため)
		0,	//アニメリソース数
	},
};

static const GFL_G3D_UTIL_SETUP g3Dutil_setup = {
	g3Dutil_resTbl,				//リソーステーブル
	NELEMS(g3Dutil_resTbl),		//リソース数
	g3Dutil_objTbl,				//オブジェクト設定テーブル
	NELEMS(g3Dutil_objTbl),		//オブジェクト数
};

#define G3DUTIL_RESCOUNT	(NELEMS(g3Dutil_resTbl))
#define G3DUTIL_OBJCOUNT	(NELEMS(g3Dutil_objTbl))


//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   
 *
 * @param   proc		
 * @param   seq		
 * @param   pwk		
 * @param   mywk		
 *
 * @retval  
 */
//--------------------------------------------------------------
GFL_PROC_RESULT PalaceBlockProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	TITLE_WORK *tw;
	
	DEBUG_PerformanceSetActive(TRUE);	//デバッグ：パフォーマンスメーター有効

	// 各種効果レジスタ初期化
	GX_SetMasterBrightness(0);
	GXS_SetMasterBrightness(0);
	GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
	GFL_DISP_GXS_SetVisibleControlDirect(0);
	G2_BlendNone();
	G2S_BlendNone();
	GX_SetVisibleWnd(GX_WNDMASK_NONE);
	GXS_SetVisibleWnd(GX_WNDMASK_NONE);

	//ヒープ作成
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PALACE, 0x70000 );
	tw = GFL_PROC_AllocWork( proc, sizeof(TITLE_WORK), HEAPID_PALACE );
	GFL_STD_MemClear(tw, sizeof(TITLE_WORK));
	tw->block_max = 1;
	tw->base_size = 16;
	
	// 上下画面設定
	GFL_DISP_SetDispSelect( GFL_DISP_3D_TO_MAIN );
	GFL_DISP_SetDispOn();

	//VRAM設定 & BGフレーム設定
	Local_VramSetting(tw);
	Local_BGFrameSetting(tw);
	
	//3D設定
	Local_3DSetting(tw);
	Local_GirathinaLoad(tw);

	//パフォーマンスメーターの為OBJ表示ON
	GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ, VISIBLE_ON);
	
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
GFL_PROC_RESULT PalaceBlockProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	TITLE_WORK* tw = mywk;
	enum{
		SEQ_MAIN,
	};
	
	if(tw->seq == SEQ_MAIN){
		int trg = GFL_UI_KEY_GetTrg();
		if(trg & PAD_BUTTON_START){
			tw->mode = trg;
//			return GFL_PROC_RES_FINISH;
		}
	}
	
	switch(tw->seq){
	case SEQ_MAIN:
		if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_DEBUG){
			switch(tw->base_size){
			case 16:
				tw->base_size = 8;
				break;
			case 8:
				tw->base_size = 12;
				break;
			default:
				tw->base_size = 16;
				break;
			}
		}
		break;
	}
	
	if(GFL_UI_KEY_GetCont() & PAD_BUTTON_X){
		tw->block_max++;
		OS_TPrintf("ブロックの数 = %d\n", tw->block_max);
	}
	else if(GFL_UI_KEY_GetCont() & PAD_BUTTON_Y){
		tw->block_max--;
		OS_TPrintf("ブロックの数 = %d\n", tw->block_max);
	}
	
	_PB_CameraMove(tw);
	Local_Draw3D(tw);
	
	return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
GFL_PROC_RESULT PalaceBlockProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	TITLE_WORK* tw = mywk;
	int i, mode;
	
	mode = tw->mode;
	
	Local_GirathinaFree(tw);
	GFL_G3D_Exit();

	GFL_BG_Exit();

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_PALACE);
	
	return GFL_PROC_RES_FINISH;
}


//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   VRAMバンク＆モード設定
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_VramSetting(TITLE_WORK *tw)
{
	static const GFL_DISP_VRAM vramBank = {
		GX_VRAM_BG_128_B,				// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_0_A,				// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_01_FG,			// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_64K,		// メインOBJマッピングモード
		GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
	};

	static const GFL_BG_SYS_HEADER sysHeader = {
		GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
	};
	
	GFL_DISP_SetBank( &vramBank );
	GFL_BG_Init( HEAPID_PALACE );
	GFL_BG_SetBGMode( &sysHeader );
}

//--------------------------------------------------------------
/**
 * @brief   BGフレーム設定
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_BGFrameSetting(TITLE_WORK *tw)
{
#if 0
	static const GFL_BG_BGCNT_HEADER bgcnt_frame[] = {	//メイン画面BGフレーム
		{//FRAME_LOGO_M
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x8000,
			GX_BG_EXTPLTT_01, BGPRI_TITLE_LOGO, 0, 0, FALSE
		},
		{//FRAME_MIST_M
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, 0x4000,
			GX_BG_EXTPLTT_01, BGPRI_MIST, 0, 0, FALSE
		},
	};

	static const GFL_BG_BGCNT_HEADER sub_bgcnt_frame[] = {	//サブ画面BGフレーム
		{//FRAME_MSG_S
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, 0x04000,
			GX_BG_EXTPLTT_01, BGPRI_MSG, 0, 0, FALSE
		},
		{//FRAME_LOGO_S
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, 0x8000,
			GX_BG_EXTPLTT_01, BGPRI_TITLE_LOGO, 0, 0, FALSE
		},
	};

	GFL_BG_SetBGControl( FRAME_LOGO_M,   &bgcnt_frame[0],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRAME_MIST_M,   &bgcnt_frame[1],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRAME_MSG_S,   &sub_bgcnt_frame[0],   GFL_BG_MODE_TEXT );
	GFL_BG_SetBGControl( FRAME_LOGO_S,   &sub_bgcnt_frame[1],   GFL_BG_MODE_TEXT );

	GFL_BG_FillCharacter( FRAME_LOGO_M, 0x00, 1, 0 );
	GFL_BG_FillCharacter( FRAME_MIST_M, 0x00, 1, 0 );
	GFL_BG_FillCharacter( FRAME_MSG_S, 0x00, 1, 0 );
	GFL_BG_FillCharacter( FRAME_LOGO_S, 0x00, 1, 0 );

	GFL_BG_FillScreen( FRAME_LOGO_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( FRAME_MIST_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( FRAME_MSG_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
	GFL_BG_FillScreen( FRAME_LOGO_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

	GFL_BG_LoadScreenReq( FRAME_LOGO_M );
	GFL_BG_LoadScreenReq( FRAME_MIST_M );
	GFL_BG_LoadScreenReq( FRAME_MSG_S );
	GFL_BG_LoadScreenReq( FRAME_LOGO_S );
#endif

	GFL_STD_MemFill16((void*)HW_BG_PLTT, 0x7fff, 0x20);
	GFL_STD_MemFill16((void*)HW_DB_BG_PLTT, 0x7fff, 0x20);
}

//--------------------------------------------------------------
/**
 * @brief   3D初期設定
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_3DSetting(TITLE_WORK *tw)
{
	GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K, GFL_G3D_VMANLNK, GFL_G3D_PLT16K,
						DTCM_SIZE, HEAPID_PALACE, NULL );
	GFL_BG_SetBGControl3D(0);
}

//--------------------------------------------------------------
/**
 * @brief   ギラティナをロード
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_GirathinaLoad(TITLE_WORK *tw)
{
	u16 objIdx;
	
	tw->g3Dutil = GFL_G3D_UTIL_Create(G3DUTIL_RESCOUNT, G3DUTIL_OBJCOUNT, HEAPID_PALACE);
	tw->g3DutilUnitIdx = GFL_G3D_UTIL_AddUnit( tw->g3Dutil, &g3Dutil_setup );

	//アニメーションを有効にする
#if 0
	objIdx = GFL_G3D_UTIL_GetUnitObjIdx( tw->g3Dutil, tw->g3DutilUnitIdx );
	GFL_G3D_OBJECT_EnableAnime( GFL_G3D_UTIL_GetObjHandle(tw->g3Dutil, objIdx), 0); 
	GFL_G3D_OBJECT_EnableAnime( GFL_G3D_UTIL_GetObjHandle(tw->g3Dutil, objIdx), 1); 
	OS_TPrintf("3d objIdx = %d\n", objIdx);
#endif

	tw->gira_status = status0;

	//カメラセット
	{
		const VecFx32 cam_pos = {0, FX32_ONE * 50, FX32_ONE * 100};
		const VecFx32 target = {0,0,0};
		
		tw->g3d_camera = GFL_G3D_CAMERA_CreateDefault(&cam_pos, &target, HEAPID_PALACE);
		tw->camera.cameraLength = 16;
	}
}

//--------------------------------------------------------------
/**
 * @brief   3D描画
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_Draw3D(TITLE_WORK *tw)
{
	GFL_G3D_OBJ* g3Dobj;
	u16				objIdx;
	int i;
	
	GFL_G3D_CAMERA_Switching(tw->g3d_camera);
	
	objIdx = GFL_G3D_UTIL_GetUnitObjIdx( tw->g3Dutil, tw->g3DutilUnitIdx );
	g3Dobj = GFL_G3D_UTIL_GetObjHandle( tw->g3Dutil, objIdx + 0 );

	GFL_G3D_DRAW_Start();
	GFL_G3D_DRAW_SetLookAt();
	{
		GFL_G3D_OBJSTATUS status = status0;
		int send_mat = TRUE;
		for(i = tw->block_max-1; i > -1; i--){
			status.trans = _BlockPosGet(tw, i);
//			GFL_G3D_DRAW_DrawObject( g3Dobj, &status );
			GFL_G3D_DRAW_DrawObject1mat1shpCullingON(g3Dobj, 0, 0, TRUE, &status);
//			GFL_G3D_DRAW_DrawObject1mat1shp(g3Dobj, 0, 0, TRUE, &status);
		}
	}
	GFL_G3D_DRAW_End();

#if 0
	GFL_G3D_OBJECT_LoopAnimeFrame( g3Dobj, 0, FX32_ONE/2 );	//BTA	/2=プラチナの1/30アニメの為
	GFL_G3D_OBJECT_LoopAnimeFrame( g3Dobj, 1, FX32_ONE/2 );	//BCA
#endif
}

//--------------------------------------------------------------
/**
 * @brief   ギラティナをアンロード
 * @param   tw		タイトルワークへのポインタ
 */
//--------------------------------------------------------------
static void Local_GirathinaFree(TITLE_WORK *tw)
{
	GFL_G3D_UTIL_DelUnit( tw->g3Dutil, tw->g3DutilUnitIdx );
	GFL_G3D_UTIL_Delete( tw->g3Dutil );
	
	//カメラ削除
	GFL_G3D_CAMERA_Delete(tw->g3d_camera);
}

//--------------------------------------------------------------
/**
 * @brief   カメラ制御
 *
 * @param   tw		
 */
//--------------------------------------------------------------
#define ROTATE_SPEED		(FX32_ONE/8)
#define MOVE_SPEED		(FX32_ONE*2)
static void _PB_CameraMove(TITLE_WORK *tw)
{
	GFL_G3D_CAMERA * g3Dcamera = tw->g3d_camera;
	PB_CAMERA *camera = &tw->camera;
	VecFx32	pos, target, trans;
	VecFx32	vecMove = { 0, 0, 0 };
	VecFx32	vecUD = { 0, 0, 0 };
	BOOL	mvFlag = FALSE;
	int key = GFL_UI_KEY_GetCont();
	s32 length = 1;
	
#if 0
	if( key & PAD_BUTTON_R ){
		camera->direction -= ROTATE_SPEED;
	}
	if( key & PAD_BUTTON_L ){
		camera->direction += ROTATE_SPEED;
	}
	if( key & PAD_BUTTON_B ){
		if( camera->cameraLength > 8 ){
			camera->cameraLength -= 8;
		}
		//vecMove.y = -MOVE_SPEED;
	}
	if( key & PAD_BUTTON_A ){
		if( camera->cameraLength < 4096 ){
			camera->cameraLength += 8;
		}
		//vecMove.y = MOVE_SPEED;
	}
	if( key & PAD_BUTTON_Y ){
		camera->cameraHeight -= MOVE_SPEED;
	}
	if( key & PAD_BUTTON_X ){
		camera->cameraHeight += MOVE_SPEED;
	}
	
	trans = camera->trans;
	trans.x += camera->transOffset.x;
	trans.y += camera->transOffset.y;
	trans.z += camera->transOffset.z;

	VEC_Set( &target,
			trans.x,
			trans.y,// + CAMERA_TARGET_HEIGHT*FX32_ONE,
			trans.z);

	pos.x = trans.x + camera->cameraLength * FX_SinIdx(camera->direction);
	pos.y = trans.y + camera->cameraHeight;
	pos.z = trans.z + camera->cameraLength * FX_CosIdx(camera->direction);

	GFL_G3D_CAMERA_SetTarget( g3Dcamera, &target );
	GFL_G3D_CAMERA_SetPos( g3Dcamera, &pos );
#else
	GFL_G3D_CAMERA_GetTarget(g3Dcamera, &target);
	GFL_G3D_CAMERA_GetPos(g3Dcamera, &pos);

	//回転
	if(key & (PAD_BUTTON_R|PAD_BUTTON_L|PAD_BUTTON_A|PAD_BUTTON_B|PAD_BUTTON_START|PAD_BUTTON_SELECT)){
		if( key & PAD_BUTTON_R ){
			camera->angle.y -= ROTATE_SPEED;
			camera->direction -= ROTATE_SPEED;
			length = 1;
		}
		if( key & PAD_BUTTON_L ){
			camera->angle.y += ROTATE_SPEED;
			camera->direction += ROTATE_SPEED;
			length = 1;
		}
		if(key & PAD_BUTTON_START){
			camera->angle.x -= ROTATE_SPEED;
			length = 1;
		}
		if(key & PAD_BUTTON_SELECT){
			camera->angle.x += ROTATE_SPEED;
			length = 1;
		}

		//距離
		if( key & PAD_BUTTON_A ){
			camera->distance += FX32_ONE;
			length = 8;
			if( camera->cameraLength < 4096 ){
				camera->cameraLength += 8;
			}
		}
		if( key & PAD_BUTTON_B ){
			camera->distance -= FX32_ONE;
			length = -8;
			if( camera->cameraLength > 8 ){
				camera->cameraLength -= 8;
			}
		}

		pos.x = FX_Mul( FX_Mul( FX_SinIdx( camera->angle.y ), camera->distance ), FX_CosIdx( camera->angle.x ) );
		pos.y = FX_Mul( FX_SinIdx( camera->angle.x ), camera->distance );
		pos.z = FX_Mul(FX_Mul(FX_CosIdx(camera->angle.y), camera->distance), FX_CosIdx(camera->angle.x));
		VEC_Add(&pos, &camera->transOffset, &pos);
		OS_TPrintf("pos.x = %x(%d), pos.y = %x(%d), pos.z = %x(%d)\n", pos.x, pos.x, pos.y, pos.y, pos.z, pos.z);
	}
	else{
		//平行移動
		if(key & PAD_KEY_UP){
			vecMove.y = FX32_ONE;
		}
		if(key & PAD_KEY_DOWN){
			vecMove.y = -FX32_ONE;
		}
		if(key & PAD_KEY_LEFT){
			vecMove.x = -FX32_ONE;
		}
		if(key & PAD_KEY_RIGHT){
			vecMove.x = FX32_ONE;
		}
		VEC_Add(&camera->transOffset, &vecMove, &camera->transOffset);
		VEC_Add(&pos, &vecMove, &pos);
		VEC_Add(&target, &vecMove, &target);
	}

	GFL_G3D_CAMERA_SetTarget(g3Dcamera, &target);
	GFL_G3D_CAMERA_SetPos(g3Dcamera, &pos);
#endif
}

//--------------------------------------------------------------
/**
 * @brief   ブロックの配置座標を取得する
 *
 * @param   block_no		ブロック番号
 *
 * @retval  座標
 */
//--------------------------------------------------------------
static VecFx32 _BlockPosGet(TITLE_WORK *tw, int block_no)
{
	VecFx32 pos;
	int base_size = tw->base_size;
	
	pos.x = (block_no % base_size) * ONE_GRID;
	pos.y = (block_no / (base_size*base_size)) * ONE_GRID;
	pos.z = ((block_no / base_size) % base_size) * ONE_GRID;
	return pos;
}
#endif //PM_DEBUG
