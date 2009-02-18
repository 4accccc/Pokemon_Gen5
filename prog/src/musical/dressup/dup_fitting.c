//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ドレスアップ 試着メイン
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"

#include "arc_def.h"
#include "musical_item.naix"

#include "test/ariizumi/ari_debug.h"
#include "musical/mus_poke_draw.h"
#include "musical/mus_item_draw.h"
#include "dup_fitting.h"
#include "dup_fitting_item.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define FIT_FRAME_MAIN_3D	GFL_BG_FRAME0_M
#define FIT_FRAME_MAIN_CASE	GFL_BG_FRAME2_M
#define FIT_FRAME_MAIN_BG	GFL_BG_FRAME3_M
#define FIT_FRAME_SUB_BG	GFL_BG_FRAME3_S

#define DEG_TO_U16(val) ((val)*0x10000/360)
#define U16_TO_DEG(val) ((val)*360/0x10000)

//BBD用座標変換(カメラの幅から計算
#define FIT_POS_X(val)	FX32_CONST((val)/16.0f)
#define FIT_POS_Y(val)	FX32_CONST((192.0f-(val))/16.0f)
#define FIT_POS_X_FX(val)	((val)/16)
#define FIT_POS_Y_FX(val)	(FX32_CONST(192.0f)-(val))/16

//ポケモン表示位置
static const int FIT_POKE_POS_X = 128;
static const int FIT_POKE_POS_Y = 112;
static const fx32 FIT_POKE_POS_X_FX = FIT_POS_X( FIT_POKE_POS_X );
static const fx32 FIT_POKE_POS_Y_FX = FIT_POS_Y( FIT_POKE_POS_Y );
static const fx32 FIT_POKE_POS_Z_FX = FX32_CONST(-40000.0f);

//アイテム表示個数
static const u16 ITEM_LIST_NUM = 14;
//static const u16 ITEM_LIST_NUM = 48;
static const u16 ITEM_FIELD_NUM = 64;
static const u16 ITEM_EQUIP_NUM = MUS_POKE_EQUIP_MAX;//9
static const u16 ITEM_DISP_NUM = ITEM_LIST_NUM+ITEM_FIELD_NUM+ITEM_EQUIP_NUM+1;	//ペンで持つ分

//アイテムリスト系定義
//中心点
static const int LIST_CENTER_X = 128;
static const int LIST_CENTER_Y = 80;
static const int LIST_CENTER_X_FX = FX32_CONST(LIST_CENTER_X);
static const int LIST_CENTER_Y_FX = FX32_CONST(LIST_CENTER_Y);
//サイズ
static const int LIST_SIZE_X = 72;
static const int LIST_SIZE_Y = 48;
//FX32_CONST(LIST_SIZE_Y/LIST_SIZE_X)を手動計算
static const float LIST_SIZE_RATIO = 0.66f;
//サイズ
static const int LIST_TPHIT_MAX_X = LIST_SIZE_X+20;
static const int LIST_TPHIT_MIN_X = LIST_SIZE_X-20;
static const int LIST_TPHIT_MAX_Y = LIST_SIZE_Y+20;
static const int LIST_TPHIT_MIN_Y = LIST_SIZE_Y-20;
static const float LIST_TPHIT_RATIO_MAX = (float)LIST_TPHIT_MAX_Y/(float)LIST_TPHIT_MAX_X;
static const float LIST_TPHIT_RATIO_MIN = (float)LIST_TPHIT_MIN_Y/(float)LIST_TPHIT_MIN_X;
//縮小が始まる位置(90度)/消え始める位置(155度)/消える位置(170度)
static const u16 LIST_SIZE_DEPTH[3] = {0x4000,0x6E00,0x78E0};
//回転を始める角度(u16)
static const u16 LIST_ROTATE_ANGLE = DEG_TO_U16(2);

//アイテム1個の間隔
static const u16 LIST_ONE_ANGLE = 0x10000/ITEM_LIST_NUM;
static const u32 LIST_FULL_ANGLE = LIST_ONE_ANGLE*MUSICAL_ITEM_MAX;


//フィールド(置いてある)アイテム系
static const fx32 FIELD_ITEM_DEPTH = FX32_CONST(-10);	//リストは0～8

//持ってるアイテム系
static const fx32 HOLD_ITEM_DEPTH = FX32_CONST(10.0f);
static const fx32 RETURN_LIST_ITEM_DEPTH = FX32_CONST(9.0f);

//装備アイテム系
static const u16 HOLD_ITEM_SNAP_LENGTH = 12;
static const fx32 EQUIP_ITEM_DEPTH = FX32_CONST(8.5f);


//リストに戻るアニメーション
static const u16 ITEM_RETURN_ANIME_CNT = 30;


//表示順位メモ
//↑
//保持アイテム			10
//List戻り中			 9
//装備品				 8.5
//アイテムリスト		8～0
//フィールドアイテム	-10～-10-0.1*表示個数
//ポケモン				-40000	//たぶんmcssで座標変換が入ってる・・・これで一番下に出る
//↓

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
	IG_NONE,
	IG_LIST,
	IG_FIELD,
	IG_EQUIP,
	IG_ANIME,
}ITEM_GROUPE;


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

struct _FITTING_WORK
{
	HEAPID heapId;
	FITTING_INIT_WORK *initWork;
	MUS_POKE_DATA_WORK	*pokeData;
	GFL_TCB *vBlankTcb;
	
	//TP系
	BOOL tpIsTrg;
	BOOL tpIsTouch;
	u32 tpx,tpy;
	u32 befTpx,befTpy;
	u16 listBefAngle;
	BOOL listAngleEnable;	//↑の変数の有効性
	BOOL isOpenList;		//リストの表示状態
	MUS_POKE_EQUIP_POS	snapPos;	//装備に吸着してる状態
	
	FIT_ITEM_WORK	*holdItem;	//保持しているアイテム
	ITEM_GROUPE		holdItemType;

	//アイテム系
	GFL_G3D_RES	*itemRes[MUSICAL_ITEM_MAX];
	FIT_ITEM_GROUP	*itemGroupList;
	FIT_ITEM_GROUP	*itemGroupField;
	FIT_ITEM_GROUP	*itemGroupEquip;
	//リストに戻るときなど操作不可でアニメ後消滅する物
	FIT_ITEM_GROUP	*itemGroupAnime;

	u16	listAngle;
	s16 listSpeed;
	u16	listHoldMove;	//持っているときの移動量
	s32	listTotalMove;	//リスト全体の移動量
	
	//3D描画に関わる物
	MUS_POKE_DRAW_SYSTEM	*drawSys;
	MUS_POKE_DRAW_WORK		*drawWork;
	MUS_ITEM_DRAW_SYSTEM	*itemDrawSys;
	GFL_G3D_CAMERA			*camera;
	GFL_BBD_SYS				*bbdSys;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static void DUP_FIT_VBlankFunc(GFL_TCB *, void * );

static void DUP_FIT_SetupGraphic( FITTING_WORK *work );
static void DUP_FIT_SetupPokemon( FITTING_WORK *work );
static void DUP_FIT_SetupItem( FITTING_WORK *work );
static void DUP_FIT_CalcItemListAngle( FITTING_WORK *work , u16 angle , s16 moveAngle );
static void DUP_FIT_OpenItemList( FITTING_WORK *work );
static void DUP_FIT_CloseItemList( FITTING_WORK *work );

static void DUP_FIT_UpdateTpMain( FITTING_WORK *work );
static void DUP_FIT_UpdateTpList( FITTING_WORK *work , s16 subX , s16 subY );
static void DUP_FIT_UpdateTpHoldItem( FITTING_WORK *work );
static void DUP_FIT_UpdateTpHoldingItem( FITTING_WORK *work );
static void DUP_FIT_UpdateTpDropItemToField( FITTING_WORK *work );
static void DUP_FIT_UpdateTpDropItemToList( FITTING_WORK *work );
static void DUP_FIT_UpdateTpDropItemToEquip(  FITTING_WORK *work );

static const BOOL DUP_FIT_CheckIsEquipItem( FITTING_WORK *work , const MUS_POKE_EQUIP_POS pos);
static void DUP_FIT_UpdateItemAnime( FITTING_WORK *work );
static void DUP_FIT_CreateItemListToField( FITTING_WORK *work );


static BOOL DUP_FIT_CheckPointInOval( s16 subX , s16 subY , s16 size , float ratioYX );


static const GFL_DISP_VRAM vramBank = {
	GX_VRAM_BG_128_D,				// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_NONE,				// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_NONE,			// サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_01_AB,				// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_01_FG,			// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_32K,
	GX_OBJVRAMMODE_CHAR_1D_32K
};
//	A テクスチャ
//	B テクスチャ
//	C SubBg
//	D MainBg
//	E テクスチャパレット
//	F テクスチャパレット
//	G None(OBJ?)
//	H None
//	I None


//--------------------------------------------------------------
//	試着メイン 初期化
//--------------------------------------------------------------
FITTING_WORK*	DUP_FIT_InitFitting( FITTING_INIT_WORK *initWork )
{
	FITTING_WORK *work = GFL_HEAP_AllocMemory( initWork->heapId , sizeof( FITTING_WORK ));

	work->heapId = initWork->heapId;
	work->initWork = initWork;
	work->tpIsTrg = FALSE;
	work->tpIsTouch = FALSE;
	work->listAngleEnable = FALSE;
	work->holdItem = NULL;
	work->holdItemType = IG_NONE;
	work->listTotalMove = LIST_FULL_ANGLE-0x8000;	//半回転の位置からスタート
	work->snapPos = MUS_POKE_EQU_INVALID;
	DUP_FIT_SetupGraphic( work );
	DUP_FIT_SetupPokemon( work );
	DUP_FIT_SetupItem( work );
	
	GFUser_VIntr_CreateTCB( DUP_FIT_VBlankFunc , work , 8 );
	
	GX_SetMasterBrightness(0);	
	GXS_SetMasterBrightness(0);
	return work;
}

//--------------------------------------------------------------
//	試着メイン 開放
//--------------------------------------------------------------
void	DUP_FIT_TermFitting( FITTING_WORK *work )
{
	GFL_TCB_DeleteTask( work->vBlankTcb );
	GFL_G3D_CAMERA_Delete( work->camera );
	GFL_G3D_Exit();
	GFL_BG_Exit();
	GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
//	試着メイン ループ
//--------------------------------------------------------------
FITTING_RETURN	DUP_FIT_LoopFitting( FITTING_WORK *work )
{

#if DEB_ARI
	VecFx32 scale,pos;
	GFL_BBD_GetScale( work->bbdSys , &scale );
	GFL_BBD_SetScale( work->bbdSys , &scale );
	MUS_POKE_DRAW_GetPosition( work->drawWork , &pos);
	MUS_POKE_DRAW_SetPosition( work->drawWork , &pos);
	if(GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
	{
		work->listAngle += 0x100;
		DUP_FIT_CalcItemListAngle( work , work->listAngle , -0x100 );
	}
	if(GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
	{
		work->listAngle -= 0x100;
		DUP_FIT_CalcItemListAngle( work , work->listAngle , +0x100 );
	}
	if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_R )
	{
		if( work->isOpenList == TRUE )
		{
			DUP_FIT_CloseItemList( work);
		}
		else
		{
			DUP_FIT_OpenItemList( work);
		}
	}
#endif

	DUP_FIT_UpdateTpMain( work );
	DUP_FIT_UpdateItemAnime( work );

	MUS_POKE_DRAW_UpdateSystem( work->drawSys ); 

	//3D描画	
	GFL_G3D_DRAW_Start();
	GFL_G3D_DRAW_SetLookAt();
	{
		MUS_POKE_DRAW_DrawSystem( work->drawSys ); 
		GFL_BBD_Draw( work->bbdSys , work->camera , NULL );
	}
	GFL_G3D_DRAW_End();

	return FIT_RET_CONTINUE;
}

static void DUP_FIT_VBlankFunc(GFL_TCB *tcb, void *wk )
{
	FITTING_WORK *work = wk;
	FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );
	while( item != NULL )
	{
		MUS_ITEM_DRAW_WORK *itemDrawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
		const u16 itemId = DUP_FIT_ITEM_GetItemIdx( item );
		const u16 newId = DUP_FIT_ITEM_GetNewItemIdx( item );
		
		if( itemId != newId )
		{
			MUS_ITEM_DRAW_ChengeGraphic(  work->itemDrawSys , itemDrawWork , newId , work->itemRes[MUS_ITEM_DRAW_GetArcIdx(newId)] );
			DUP_FIT_ITEM_SetItemIdx( item , newId );
		}
		
		
		item = DUP_FIT_ITEM_GetNextItem(item);
	}
}

//--------------------------------------------------------------
//	描画系初期化
//--------------------------------------------------------------
static void DUP_FIT_SetupGraphic( FITTING_WORK *work )
{
	GFL_DISP_GX_InitVisibleControl();
	GFL_DISP_GXS_InitVisibleControl();
	GX_SetVisiblePlane( 0 );
	GXS_SetVisiblePlane( 0 );
	WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
	WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
	WIPE_ResetWndMask(WIPE_DISP_MAIN);
	WIPE_ResetWndMask(WIPE_DISP_SUB);
	
	GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);
	GFL_DISP_SetBank( &vramBank );
	
	GFL_BG_Init( work->heapId );
	
	//Vram割り当ての設定
	{
		static const GFL_BG_SYS_HEADER sys_data = {
				GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
		};
		
		// BG2 MAIN (蓋
		static const GFL_BG_BGCNT_HEADER header_main2 = {
			0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x08000,0x6000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
		};
		// BG3 MAIN (背景
		static const GFL_BG_BGCNT_HEADER header_main3 = {
			0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000,0x6000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
		};

		// BG3 SUB (背景
		static const GFL_BG_BGCNT_HEADER header_sub3 = {
			0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000,0x6000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
		};

		GFL_BG_SetBGMode( &sys_data );
		GFL_BG_SetBGControl3D( 1 );
		GFL_BG_SetBGControl( FIT_FRAME_MAIN_CASE,  &header_main2, GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( FIT_FRAME_MAIN_BG,  &header_main3, GFL_BG_MODE_TEXT );

		GFL_BG_SetBGControl( FIT_FRAME_SUB_BG, &header_sub3, GFL_BG_MODE_TEXT );
		
		GFL_BG_SetVisible( FIT_FRAME_MAIN_3D , TRUE );
	}
	
	{	//3D系の設定
//		static const VecFx32 cam_pos = { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 7.8f ), FX_F32_TO_FX32( 21.0f ) };
//		static const VecFx32 cam_target = { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 2.6f ), FX_F32_TO_FX32( 0.0f ) };
		static const VecFx32 cam_pos = { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 101.0f ) };
		static const VecFx32 cam_target = { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 0.0f ) };
		static const VecFx32 cam_up = { 0, FX32_ONE, 0 };
		//エッジマーキングカラー
		static	const	GXRgb stage_edge_color_table[8]=
			{ GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 0, 0, 0, 0, 0, 0 };
		GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT80K,
						0, work->heapId, NULL );
		GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_W );
#if 0	//透視射影カメラ
		work->camera = 	GFL_G3D_CAMERA_Create( GFL_G3D_PRJPERS, 
											 FX32_SIN13,
											 FX32_COS13,
											 FX_F32_TO_FX32( 1.33f ),
											 NULL,
											 FX32_ONE,
											 FX32_ONE * 180,
											 NULL,
											 &cam_pos,
											 &cam_up,
											 &cam_target,
											 work->heapId );
#else
		//正射影カメラ
		work->camera = 	GFL_G3D_CAMERA_Create( GFL_G3D_PRJORTH, 
											 FX32_ONE*12.0f,
											 0,
											 0,
											 FX32_ONE*16.0f,
											 FX32_ONE,
											 FX32_ONE * 180,
											 NULL,
											 &cam_pos,
											 &cam_up,
											 &cam_target,
											 work->heapId );
#endif
		
		GFL_G3D_CAMERA_Switching( work->camera );
		//エッジマーキングカラーセット
		G3X_SetEdgeColorTable( &stage_edge_color_table[0] );
		G3X_EdgeMarking( TRUE );
		
		GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO , GX_BUFFERMODE_Z );
	}
	{
		GFL_BBD_SETUP bbdSetup = {
			128,128,
			{FX32_ONE,FX32_ONE,0},
			GX_RGB(0,0,0),
			GX_RGB(0,0,0),
			GX_RGB(0,0,0),
			GX_RGB(0,0,0),
			0
		};
		//ビルボードシステム構築
		work->bbdSys = GFL_BBD_CreateSys( &bbdSetup , work->heapId );
	}
}

//--------------------------------------------------------------
//	表示ポケモンの初期化
//--------------------------------------------------------------
static void DUP_FIT_SetupPokemon( FITTING_WORK *work )
{
	VecFx32 pos = {FIT_POKE_POS_X_FX,FIT_POKE_POS_Y_FX,FIT_POKE_POS_Z_FX};
//	VecFx32 pos = {FX32_ONE*10,FX32_ONE*10,FX32_ONE*-7};	//位置は適当
	work->drawSys = MUS_POKE_DRAW_InitSystem( work->heapId );
	work->drawWork = MUS_POKE_DRAW_Add( work->drawSys , work->initWork->musPoke );
	work->pokeData = MUS_POKE_DRAW_GetPokeData( work->drawWork);
	MUS_POKE_DRAW_SetPosition( work->drawWork , &pos);
}

//--------------------------------------------------------------
//アイテムの初期化
//--------------------------------------------------------------
static void DUP_FIT_SetupItem( FITTING_WORK *work )
{
	int i;
	work->itemDrawSys = MUS_ITEM_DRAW_InitSystem( work->bbdSys , ITEM_DISP_NUM , work->heapId );
	for( i=0;i<MUSICAL_ITEM_MAX;i++ )
	{
		work->itemRes[i] = MUS_ITEM_DRAW_LoadResource( i );
	}
	
	work->itemGroupList = DUP_FIT_ITEMGROUP_CreateGroup( work->heapId , ITEM_LIST_NUM );
	work->itemGroupField = DUP_FIT_ITEMGROUP_CreateGroup( work->heapId , ITEM_FIELD_NUM );
	work->itemGroupEquip = DUP_FIT_ITEMGROUP_CreateGroup( work->heapId , MUS_POKE_EQUIP_MAX );
	work->itemGroupAnime = DUP_FIT_ITEMGROUP_CreateGroup( work->heapId , 4 );
	for( i=0;i<ITEM_LIST_NUM;i++ )
	{
		FIT_ITEM_WORK* item;
		VecFx32 pos = {0,0,0};
		
		item = DUP_FIT_ITEM_CreateItem( work->heapId , work->itemDrawSys , i , work->itemRes[i] , &pos );
		DUP_FIT_ITEMGROUP_AddItem( work->itemGroupList,item );
	}
	DUP_FIT_CalcItemListAngle( work , 0 , 0 );
	work->isOpenList = TRUE;
}

//--------------------------------------------------------------
//アイテムリストの位置・サイズ計算
//--------------------------------------------------------------
static void DUP_FIT_CalcItemListAngle( FITTING_WORK *work , u16 angle , s16 moveAngle )
{
	//円は下が手前で0度として、depthは下で0x8000 上が0x0000
	int i = 0;
	FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );
	
	work->listTotalMove += moveAngle;
	if( work->listTotalMove < 0 )
	{
		work->listTotalMove += LIST_FULL_ANGLE;
	}
	if( work->listTotalMove >= LIST_FULL_ANGLE )
	{
		work->listTotalMove -= LIST_FULL_ANGLE;
	}
	

	while( item != NULL )
	{
		GFL_POINT dispPos;
		VecFx32 pos;
		u16 oneAngle = (i*0x10000/ITEM_LIST_NUM+angle)%0x10000;
		u16 depth;
		MUS_ITEM_DRAW_WORK *itemDrawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
		const fx32 sin = (fx32)FX_SinIdx( oneAngle );
		const fx32 cos = (fx32)FX_CosIdx( oneAngle );
		const fx32 posX = LIST_CENTER_X_FX+sin*LIST_SIZE_X;
		const fx32 posY = LIST_CENTER_Y_FX+cos*LIST_SIZE_Y;
		
		//絵の読み替え処理とか
		{
			const u16 nowId = DUP_FIT_ITEM_GetItemIdx( item );
			u16 newId;
			s32 subAngle;
			s32 itemTotalAngle;

			subAngle = (oneAngle + 0x8000 )%0x10000;
			itemTotalAngle = (work->listTotalMove+subAngle);
			if( itemTotalAngle < 0 )
			{
				itemTotalAngle += LIST_FULL_ANGLE;
			}
			if( itemTotalAngle >= LIST_FULL_ANGLE )
			{
				itemTotalAngle -= LIST_FULL_ANGLE;
			}
			
			newId = (itemTotalAngle+(LIST_ONE_ANGLE/2))/LIST_ONE_ANGLE;
			DUP_FIT_ITEM_SetNewItemIdx( item , newId );
		}
		
		//位置の計算
		if( oneAngle < 0x8000 )
		{
			depth = oneAngle;
		}
		else
		{
			depth = 0x8000-(oneAngle-0x8000);
		}
		dispPos.x = F32_CONST(posX);
		dispPos.y = F32_CONST(posY);
		pos.x = FIT_POS_X_FX(posX);
		pos.y = FIT_POS_Y_FX(posY);
		pos.z = 0x8000-depth;
	
		MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , itemDrawWork , &pos );
		DUP_FIT_ITEM_SetPosition( item , &dispPos );
		//サイズの計算
		{
			if( depth < LIST_SIZE_DEPTH[0] )
			{
				MUS_ITEM_DRAW_SetSize( work->itemDrawSys , itemDrawWork , FX16_ONE , FX16_ONE );
				MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , itemDrawWork , TRUE );
				DUP_FIT_ITEM_SetScale( item , FX32_ONE );
			}
			else if( depth < LIST_SIZE_DEPTH[1] )
			{
				//100%→50%
				const u16 subAngle = LIST_SIZE_DEPTH[1]-depth;
				const fx16 size = (FX16_CONST(0.5f)*subAngle/(LIST_SIZE_DEPTH[1]-LIST_SIZE_DEPTH[0]))+FX16_CONST(0.5f);
				MUS_ITEM_DRAW_SetSize( work->itemDrawSys , itemDrawWork , size,size );
				MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , itemDrawWork , TRUE );
				DUP_FIT_ITEM_SetScale( item , size );
			}
			else if( depth < LIST_SIZE_DEPTH[2] )
			{
				//50%→0%
				const u16 subAngle = LIST_SIZE_DEPTH[2]-depth;
				const fx16 size = FX16_CONST(0.5f)*subAngle/(LIST_SIZE_DEPTH[2]-LIST_SIZE_DEPTH[1]);
				MUS_ITEM_DRAW_SetSize( work->itemDrawSys , itemDrawWork , size,size );
				MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , itemDrawWork , TRUE );
				DUP_FIT_ITEM_SetScale( item , size );
			}
			else
			{
				MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , itemDrawWork , FALSE );
				DUP_FIT_ITEM_SetScale( item , 0 );
			}
		}
		
		item = DUP_FIT_ITEM_GetNextItem(item);
		i++;
	}
	item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );
	
}

//--------------------------------------------------------------
//アイテムリストの表示(開始
//--------------------------------------------------------------
static void DUP_FIT_OpenItemList( FITTING_WORK *work )
{
	FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );
	GF_ASSERT( work->isOpenList == FALSE );
	//リストの角度計算内で表示非表示もやっているので、ここで全アイテムを対象にした処理はない
/*
	while( item != NULL )
	{
		MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
		
		MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , drawWork , FALSE );

		item = DUP_FIT_ITEM_GetNextItem(item);
	}
*/
	DUP_FIT_CalcItemListAngle( work , work->listAngle , 0 );
	
	work->isOpenList = TRUE;

}
//--------------------------------------------------------------
//アイテムリストの非表示(開始
//--------------------------------------------------------------
static void DUP_FIT_CloseItemList( FITTING_WORK *work )
{
	FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );
	GF_ASSERT( work->isOpenList == TRUE );
	while( item != NULL )
	{
		MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
		
		MUS_ITEM_DRAW_SetDrawEnable( work->itemDrawSys , drawWork , FALSE );

		item = DUP_FIT_ITEM_GetNextItem(item);
	}

	work->isOpenList = FALSE;
	
}

//--------------------------------------------------------------
//タッチペン操作更新メイン
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpMain( FITTING_WORK *work )
{
	BOOL isTouchList = FALSE;
	work->befTpx = work->tpx;
	work->befTpy = work->tpy;
	work->tpIsTrg = GFL_UI_TP_GetTrg();
	work->tpIsTouch = GFL_UI_TP_GetPointCont( &work->tpx,&work->tpy );
	if( work->tpIsTrg || work->tpIsTouch )
	{
		
		if( work->holdItemType == IG_FIELD && work->holdItem != NULL )
		{
			//アイテム保持中の処理
			DUP_FIT_UpdateTpHoldingItem( work );
		}
		else if( work->isOpenList == TRUE )
		{
			//アイテムリストの中か？
			//X座標には比をかけてYのサイズで計算する
			const s16 subX = work->tpx - LIST_CENTER_X;
			const s16 subY = work->tpy - LIST_CENTER_Y;
			const u32 centerLen = F32_CONST(FX_Sqrt(FX32_CONST(subX*subX+subY*subY)));
			//内側の円に居るか？
			BOOL hitMinOval,hitMaxOval;
			hitMinOval = DUP_FIT_CheckPointInOval( subX,subY,LIST_TPHIT_MIN_Y,LIST_TPHIT_RATIO_MIN );
			hitMaxOval = DUP_FIT_CheckPointInOval( subX,subY,LIST_TPHIT_MAX_Y,LIST_TPHIT_RATIO_MAX );
			if( hitMinOval == FALSE && hitMaxOval == TRUE )
			{
				isTouchList = TRUE;
				DUP_FIT_UpdateTpList( work , subX , subY );
			}
			else if( (hitMinOval == FALSE && hitMaxOval == FALSE )||
					 (hitMinOval == TRUE && hitMaxOval == TRUE ))
			{
				if( DUP_FIT_ITEMGROUP_IsItemMax(work->itemGroupField) == FALSE )
				{
					//円の内側も許可
					if( work->holdItemType == IG_LIST &&
						work->holdItem != NULL )
					{
						DUP_FIT_CreateItemListToField( work );
					}
				}
			}
		}
		if( isTouchList == FALSE && work->tpIsTrg == TRUE )
		{
			DUP_FIT_UpdateTpHoldItem(work);
		}
	}
	else
	{
		//アイテムリストの中か？
		//X座標には比をかけてYのサイズで計算する
		const s16 subX = work->befTpx - LIST_CENTER_X;
		const s16 subY = work->befTpy - LIST_CENTER_Y;
		const u32 centerLen = F32_CONST(FX_Sqrt(FX32_CONST(subX*subX+subY*subY)));
		//内側の円に居るか？
		BOOL hitMinOval,hitMaxOval;
		hitMinOval = DUP_FIT_CheckPointInOval( subX,subY,LIST_TPHIT_MIN_Y,LIST_TPHIT_RATIO_MIN );
		hitMaxOval = DUP_FIT_CheckPointInOval( subX,subY,LIST_TPHIT_MAX_Y,LIST_TPHIT_RATIO_MAX );
		if( work->holdItemType == IG_FIELD && work->holdItem != NULL )
		{
			if( work->snapPos != MUS_POKE_EQU_INVALID )
			{
				DUP_FIT_UpdateTpDropItemToEquip( work );
			}
			else
			if( hitMinOval == FALSE && hitMaxOval == TRUE &&
				work->isOpenList == TRUE )
			{
				DUP_FIT_UpdateTpDropItemToList( work );
			}
			else
			{
				DUP_FIT_UpdateTpDropItemToField( work );
			}
		}
		
		work->listAngleEnable = FALSE;
		work->holdItem = NULL;
		work->holdItemType = IG_NONE;
		work->snapPos = MUS_POKE_EQU_INVALID;
	}
	//慣性で回る～
	if( work->listSpeed != 0 && isTouchList == FALSE )
	{
		s32 tempAngle = (s32)work->listAngle+0x10000-work->listSpeed;
		if( tempAngle < 0 )
			tempAngle += 0x10000;
		if( tempAngle >= 0x10000 )
			tempAngle -= 0x10000;
		work->listAngle = tempAngle;
		DUP_FIT_CalcItemListAngle( work , work->listAngle , work->listSpeed );
		if( work->listSpeed > 0x80 || work->listSpeed < -0x80 )
		{
			work->listSpeed *= 0.75f;
		}
		else
		{ 
			work->listSpeed = 0;
		}
	}	
}
//--------------------------------------------------------------
//タッチペン操作更新リスト
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpList( FITTING_WORK *work , s16 subX , s16 subY )
{
	//回転は楕円なので角度で見ないほうが良いかも
	//でも角度じゃないと回せない・・・
	const u16 angle = FX_Atan2Idx(subX*FX32_ONE,subY*FX32_ONE);
	
	//つかみチェック
	if( work->tpIsTrg == TRUE )
	{
		FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupList );
		//ついでに初期化
		work->listSpeed = 0;
		work->listHoldMove = 0;
		work->holdItem = NULL;

		while( item != NULL )
		{
			//サイズを気にするか？
//			if( DUP_FIT_ITEM_GetScale( item ) == FX32_ONE &&
//				DUP_FIT_ITEM_CheckHit( item , work->tpx,work->tpy ) == TRUE )
			if( DUP_FIT_ITEM_CheckHit( item , work->tpx,work->tpy ) == TRUE )
			{
				work->holdItem = item;
				work->holdItemType = IG_LIST;
				//DUP_FIT_ITEM_CheckLengthSqrt( item , work->tpx,work->tpy );
				ARI_TPrintf("Item Hold\n");
				item = NULL;
			}
			else
			{
				item = DUP_FIT_ITEM_GetNextItem(item);
			}
		}
	}
	//回転処理
	if( work->listAngleEnable == TRUE )
	{
		u8 i;
		//前回とのリストに対する角度差
		const s32 subAngle = work->listBefAngle - angle;
		//前回の座標からどっちに動いたか？の角度
		//const u16 moveAngle = FX_Atan2Idx((work->tpx-work->befTpx)*FX32_ONE,(work->tpy-work->befTpy)*FX32_ONE);
		//回転チェック
		if( subAngle > LIST_ROTATE_ANGLE ||
			subAngle < -LIST_ROTATE_ANGLE )
		{
			s32 tempAngle = (s32)work->listAngle+0x10000-subAngle;
			if( tempAngle < 0 )
				tempAngle += 0x10000;
			if( tempAngle >= 0x10000 )
				tempAngle -= 0x10000;
			work->listAngle = tempAngle;
			DUP_FIT_CalcItemListAngle( work , work->listAngle , subAngle );
			
			if( ( work->listSpeed>0 && work->listSpeed<0) ||
				( work->listSpeed<0 && work->listSpeed>0) )
			{
				work->listSpeed = 0;
			}
			else
			{
				work->listSpeed = subAngle;
			}

			work->listHoldMove += MATH_ABS(subAngle);
			if( work->listHoldMove >= DEG_TO_U16(10) )
			{
				//移動しすぎたのでアイテム保持解除
				work->holdItem = NULL;
				work->holdItemType = IG_NONE;
			}
		}
		else
		{
			work->listSpeed = 0;
		}
	}
	work->listBefAngle = angle;
	work->listAngleEnable = TRUE;
}

//--------------------------------------------------------------
//フィールド・装備のアイテムを拾う
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpHoldItem( FITTING_WORK *work )
{
	BOOL isEquipList = TRUE;
	FIT_ITEM_WORK* item;
	item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupEquip );
	if( item == NULL || DUP_FIT_ITEMGROUP_IsItemMax(work->itemGroupField) == TRUE )
	{
		//フィールドのアイテムがいっぱいなら装備アイテムは拾えない
		item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupField );
		isEquipList = FALSE;
	}
	while( item != NULL )
	{
		if( DUP_FIT_ITEM_CheckHit( item , work->tpx,work->tpy ) == TRUE )
		{
			work->holdItem = item;
			work->holdItemType = IG_FIELD;
			//DUP_FIT_ITEM_CheckLengthSqrt( item , work->tpx,work->tpy );
			ARI_TPrintf("Item Hold[%d]\n",DUP_FIT_ITEM_GetItemIdx(item));
			item = NULL;
		}
		else
		{
			item = DUP_FIT_ITEM_GetNextItem(item);
			if( item == NULL && isEquipList == TRUE )
			{
				item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupField );
				isEquipList = FALSE;
			}
		}
		
	}
	
	if( isEquipList == TRUE && item != work->holdItem )
	{
		DUP_FIT_ITEMGROUP_RemoveItem( work->itemGroupEquip , work->holdItem );
		DUP_FIT_ITEMGROUP_AddItem( work->itemGroupField , work->holdItem );
		//座標あわせと深度設定
		DUP_FIT_UpdateTpHoldingItem( work );
	}

}

//--------------------------------------------------------------
//フィールドのアイテムを持っている
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpHoldingItem( FITTING_WORK *work )
{
	u16 snapLen = HOLD_ITEM_SNAP_LENGTH;
	GFL_POINT dispPos;
	GFL_POINT pokePosSub;
	VecFx32 pos = {0,0,HOLD_ITEM_DEPTH};
	fx16 scaleX,scaleY;
	MUS_ITEM_DRAW_WORK *itemDrawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
	
	pokePosSub.x = work->tpx - FIT_POKE_POS_X;
	pokePosSub.y = work->tpy - FIT_POKE_POS_Y;
	work->snapPos = MUS_POKE_DATA_SearchEquipPosition( work->pokeData , &pokePosSub , &snapLen );
	if( work->snapPos != MUS_POKE_EQU_INVALID &&
		DUP_FIT_CheckIsEquipItem( work , work->snapPos ) == FALSE )
	{
		
		dispPos = *MUS_POKE_DATA_GetEquipPosition( work->pokeData , work->snapPos );
		dispPos.x += FIT_POKE_POS_X;
		dispPos.y += FIT_POKE_POS_Y;
	}
	else
	{
		work->snapPos = MUS_POKE_EQU_INVALID;
		dispPos.x = work->tpx;
		dispPos.y = work->tpy;
	}

	pos.x = FIT_POS_X(dispPos.x);
	pos.y = FIT_POS_Y(dispPos.y);
	MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , itemDrawWork , &pos );
	DUP_FIT_ITEM_SetPosition( work->holdItem , &dispPos );
	
	MUS_ITEM_DRAW_GetSize( work->itemDrawSys , itemDrawWork ,&scaleX,&scaleY );
	if( scaleX < FX16_ONE || scaleY < FX16_ONE )
	{
		scaleX += FX16_CONST( 0.1f );
		scaleY += FX16_CONST( 0.1f );
		if( scaleX > FX16_ONE )
		{
			scaleX = FX16_ONE;
		}
		if( scaleY > FX16_ONE )
		{
			scaleY = FX16_ONE;
		}
		MUS_ITEM_DRAW_SetSize( work->itemDrawSys , itemDrawWork ,scaleX,scaleY );
	}
}


//--------------------------------------------------------------
//リストからフィールドへアイテムを置く(そのまま保持状態へ
//--------------------------------------------------------------
static void DUP_FIT_CreateItemListToField( FITTING_WORK *work )
{
	FIT_ITEM_WORK* item;
	VecFx32 pos = {0,0,HOLD_ITEM_DEPTH};
	u16 itemIdx = DUP_FIT_ITEM_GetItemIdx( work->holdItem );
	MUS_ITEM_DRAW_WORK *itemDrawWork,*holdDrawWork;
	GFL_POINT dispPos;
	fx16 scaleX,scaleY;

	dispPos.x = work->tpx;
	dispPos.y = work->tpy;
	pos.x = FIT_POS_X(work->tpx);
	pos.y = FIT_POS_Y(work->tpy);

	item = DUP_FIT_ITEM_CreateItem( work->heapId , work->itemDrawSys , itemIdx , work->itemRes[itemIdx] , &pos );
	DUP_FIT_ITEMGROUP_AddItem( work->itemGroupField,item );

	DUP_FIT_ITEM_SetPosition( item , &dispPos );
	//サイズの引継ぎ
	holdDrawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
	itemDrawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
	MUS_ITEM_DRAW_GetSize( work->itemDrawSys , holdDrawWork ,&scaleX,&scaleY );
	MUS_ITEM_DRAW_SetSize( work->itemDrawSys , itemDrawWork ,scaleX,scaleY );

	//保持アイテムを今生成したものに変える
	work->holdItem = item;
	work->holdItemType = IG_FIELD;
}

//--------------------------------------------------------------
//アイテムを離す フィールドへ
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpDropItemToField( FITTING_WORK *work )
{
	fx32 depth = FIELD_ITEM_DEPTH;
	FIT_ITEM_WORK *item;
	MUS_ITEM_DRAW_WORK *holdDrawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
	//一度アイテムをリストから取り、頭につなぎ直す
	DUP_FIT_ITEMGROUP_RemoveItem( work->itemGroupField , work->holdItem );
	DUP_FIT_ITEMGROUP_AddItemTop( work->itemGroupField , work->holdItem );
	
	MUS_ITEM_DRAW_SetSize( work->itemDrawSys , holdDrawWork ,FX16_ONE,FX16_ONE );
	//深度の再設定
	item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupField );
	while( item != NULL )
	{
		VecFx32 pos;
		MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
		MUS_ITEM_DRAW_GetPosition( work->itemDrawSys , drawWork , &pos );
		pos.z = depth;
		MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , drawWork , &pos );
		
		depth -= FX32_CONST(0.1f);
		item = DUP_FIT_ITEM_GetNextItem(item);
	}
	
#if DEB_ARI
	DUP_FIT_ITEM_DumpList( work->itemGroupField ,2 );
#endif
}
//--------------------------------------------------------------
//アイテムを離す リストへ
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpDropItemToList( FITTING_WORK *work )
{
	VecFx32 pos;
	MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
	MUS_ITEM_DRAW_WORK *holdDrawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );

	//リストを付け替えて座標を再設定
	DUP_FIT_ITEMGROUP_RemoveItem( work->itemGroupField , work->holdItem );
	DUP_FIT_ITEMGROUP_AddItemTop( work->itemGroupAnime , work->holdItem );
	
	MUS_ITEM_DRAW_GetPosition( work->itemDrawSys , drawWork , &pos );
	pos.z = RETURN_LIST_ITEM_DEPTH;
	MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , drawWork , &pos );

	MUS_ITEM_DRAW_SetSize( work->itemDrawSys , holdDrawWork ,FX16_ONE,FX16_ONE );

	DUP_FIT_ITEM_SetCount( work->holdItem , 0 );
	
#if DEB_ARI
	DUP_FIT_ITEM_DumpList( work->itemGroupField ,2 );
#endif
}

//--------------------------------------------------------------
//アイテムを離す 装備へ
//--------------------------------------------------------------
static void DUP_FIT_UpdateTpDropItemToEquip(  FITTING_WORK *work )
{
	VecFx32 pos;
	fx32 depth = EQUIP_ITEM_DEPTH;
	FIT_ITEM_WORK *item;
	MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
	MUS_ITEM_DRAW_WORK *holdDrawWork = DUP_FIT_ITEM_GetItemDrawWork( work->holdItem );
	//リストを付け替えて座標を再設定
	DUP_FIT_ITEMGROUP_RemoveItem( work->itemGroupField , work->holdItem );
	DUP_FIT_ITEMGROUP_AddItemTop( work->itemGroupEquip , work->holdItem );

	MUS_ITEM_DRAW_SetSize( work->itemDrawSys , holdDrawWork ,FX16_ONE,FX16_ONE );

	DUP_FIT_ITEM_SetCount( work->holdItem , work->snapPos );
	
	//深度の再設定
	item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupEquip );
	while( item != NULL )
	{
		VecFx32 pos;
		MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
		MUS_ITEM_DRAW_GetPosition( work->itemDrawSys , drawWork , &pos );
		pos.z = depth;
		MUS_ITEM_DRAW_SetPosition( work->itemDrawSys , drawWork , &pos );
		
		depth -= FX32_CONST(0.1f);
		item = DUP_FIT_ITEM_GetNextItem(item);
	}
	
#if DEB_ARI
	DUP_FIT_ITEM_DumpList( work->itemGroupField ,2 );
#endif
	
}

//--------------------------------------------------------------
//指定箇所に装備があるか？
//--------------------------------------------------------------
static const BOOL DUP_FIT_CheckIsEquipItem( FITTING_WORK *work , const MUS_POKE_EQUIP_POS pos)
{
	FIT_ITEM_WORK *item = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupEquip );
	while( item != NULL )
	{
		if( pos == DUP_FIT_ITEM_GetCount( item ) )
		{
			return TRUE;
		}
		item = DUP_FIT_ITEM_GetNextItem(item);
	}
	return FALSE;
}

//--------------------------------------------------------------
//アイテムのアニメーション(itemGroupAnimeに登録されている物の処理
//--------------------------------------------------------------
static void DUP_FIT_UpdateItemAnime( FITTING_WORK *work )
{
	FIT_ITEM_WORK *nextItem = DUP_FIT_ITEMGROUP_GetStartItem( work->itemGroupAnime );
	while( nextItem != NULL )
	{
		FIT_ITEM_WORK *item = nextItem;
		MUS_ITEM_DRAW_WORK *drawWork = DUP_FIT_ITEM_GetItemDrawWork( item );
		u16 cnt = DUP_FIT_ITEM_GetCount( item );

		//removeする可能性があるので、先に次をとっておく
		nextItem = DUP_FIT_ITEM_GetNextItem(item);

		cnt++;
		if( ITEM_RETURN_ANIME_CNT > cnt )
		{
			fx16 size = FX16_ONE * (ITEM_RETURN_ANIME_CNT-cnt) / ITEM_RETURN_ANIME_CNT;
			MUS_ITEM_DRAW_SetSize( work->itemDrawSys , drawWork , size , size );
			DUP_FIT_ITEM_SetCount( item , cnt );
		}
		else
		{
			DUP_FIT_ITEMGROUP_RemoveItem( work->itemGroupAnime , item );
			DUP_FIT_ITEM_DeleteItem( item , work->itemDrawSys );
		}
	}
}



//--------------------------------------------------------------
//楕円の当たり判定
//--------------------------------------------------------------
static BOOL DUP_FIT_CheckPointInOval( s16 subX , s16 subY , s16 size , float ratioYX )
{
	const s16 newSubX = (s16)(subX*ratioYX);
	if( newSubX * newSubX + subY * subY < size*size )
		return TRUE;
	else
		return FALSE;
}
