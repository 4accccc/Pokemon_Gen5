//======================================================================
/**
 * @file	field_debug.c
 * @brief	フィールドデバッグ処理
 * @author	kagaya
 * @date	08.09.29
 */
//======================================================================
#ifdef  PM_DEBUG

#include "field_debug.h"
#include "fieldmap.h"
#include "field_bg_def.h"

#include "fldmmdl.h"
#include "field/map_matrix.h"
#include "field/zonedata.h"
#include "arc/others.naix"
#include "arc/debug/hitch_check.naix"

#include "system/main.h"  //HEAPID_FIELDMAP
//======================================================================
//	define
//======================================================================
#define DEBUG_PANO_FONT (15) //フォントで使用するパレットNo

enum
{ 
  UNIT_MAX = 5*5,   ///< ユニット最大数
};

#define	UNIT_NONE		( 0xffff )	///< ユニットなし
#define MAP_UNIT_W  (FX32_CONST(16))
#define MAP_UNIT_WH  (FX32_CONST(8))

//======================================================================
//	typedef struct
//======================================================================

typedef struct RENDER_INFO_tag
{
  u16 Vtx;
  u16 Ply;
  u16 Mat;
  u16 Shp;
  u16 Num;
  u16 DrawLandNum;
}RENDER_INFO;

typedef struct _TAG_HITCH_UNIT
{
  u16 idx;
  u8  draw_f:1;
  fx32  y;
}HITCH_UNIT;

//--------------------------------------------------------------
///	FIELD_DEBUG_WORK 
//--------------------------------------------------------------
struct _TAG_FIELD_DEBUG_WORK
{
	HEAPID heapID;	//デバッグで使用するHEAPID
	FIELDMAP_WORK *pFieldMainWork; //FIELDMAP_WORK*
	GAMESYS_WORK *gsys;
	PLAYER_WORK *player;
  FLDMAPPER * g3dMapper;
	
	u8 bgFrame;	//デバッグで使用するBG FRAME

	u8 flag_bgscr_load:1;	//デバッグBG面のスクリーンロード
	u8 flag_pos_print:1;	//座標表示切り替え
	u8 flag_pos_update:1;	//座標表示更新可能フラグ
	u8 flag_hitch_check:1;	//HitchチェックOn/Offフラグ
	u8 flag_hitch_y_org:1;	//Hitchポリゴンの高さをプレイヤーの高さにあわせる

  u8 print_type;

  GFL_G3D_UTIL*     g3d_util;
  GFL_G3D_OBJSTATUS obj_status;
  s16               unit_idx;
  HITCH_UNIT        unit[UNIT_MAX];
  fx32              unit_ox;
  fx32              unit_oz;
  fx32              unit_oy;
};

RENDER_INFO DbgRenderInfo = {0,0,0,0};


//RES
static const GFL_G3D_UTIL_RES res_unit_hitch_check[] = {
  { ARCID_DEBUG_HITCH_CHECK, NARC_hitch_check_hitch_chk_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_DEBUG_HITCH_CHECK, NARC_hitch_check_hitch_chk_nsbtp, GFL_G3D_UTIL_RESARC },
};
//ANM
static const GFL_G3D_UTIL_ANM anm_unit_hitch_check[] = {
  { 1, 0 },
};
//OBJ
static const GFL_G3D_UTIL_OBJ obj_unit_hitch_check[] = {
  0, 0, 0, anm_unit_hitch_check, 1,
};

#define UNIT_RES_MAX  (NELEMS(res_unit_hitch_check))
// セットアップ
static const GFL_G3D_UTIL_SETUP sc_g3d_setup = 
{ 
  res_unit_hitch_check, UNIT_RES_MAX, obj_unit_hitch_check, NELEMS(obj_unit_hitch_check)
};


//======================================================================
//	proto
//======================================================================
static void g3d_unit_Init( FIELD_DEBUG_WORK *work );
static void g3d_unit_Release( FIELD_DEBUG_WORK *work );
static void g3d_unit_Main( FIELD_DEBUG_WORK *work );
static void g3d_unit_Draw( FIELD_DEBUG_WORK *work );


static void DebugFont_Init( FIELD_DEBUG_WORK *work );
static void DebugFont_InitResource( FIELD_DEBUG_WORK *work );
static void DebugFont_Put( u16 *screen, char c, u16 x, u16 y );
static void DebugFont_Print(
	FIELD_DEBUG_WORK *work, u16 x, u16 y, const char *msgBuf );
static void DebugFont_ClearLine( FIELD_DEBUG_WORK *work, u16 y );

static void DebugFieldPosPrint_Proc( FIELD_DEBUG_WORK *work );
static void DebugFieldMemPrint_Proc( FIELD_DEBUG_WORK *work );

static void DbgDrawCallBackFunc( NNSG3dRenderObj *renderobj, void *work );

//======================================================================
//	フィールドデバッグシステム
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドデバッグシステム 初期化
 * @param 	pFieldMainWork FIELDMAP_WORK
 * @retval	FIELD_DEBUG_WORK
 */
//--------------------------------------------------------------
FIELD_DEBUG_WORK * FIELD_DEBUG_Init(
	FIELDMAP_WORK *pFieldMainWork, u8 bg_frame, HEAPID heapID )
{
	FIELD_DEBUG_WORK *work;
	work = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_DEBUG_WORK) );
	
	work->heapID = heapID;
	work->bgFrame = bg_frame;
	work->pFieldMainWork = pFieldMainWork;
	work->gsys = FIELDMAP_GetGameSysWork( work->pFieldMainWork );
	work->player = GAMESYSTEM_GetMyPlayerWork( work->gsys );
  work->g3dMapper = FIELDMAP_GetFieldG3Dmapper( work->pFieldMainWork );
	
	{	//デバッグ用フォント初期化
		DebugFont_Init( work );
	}
	
	{	//各デバッグ機能初期化
    g3d_unit_Init( work );
	}
	
	return( work );
}

//--------------------------------------------------------------
/**
 * フィールドデバッグシステム 削除
 * @param 	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
void FIELD_DEBUG_Delete( FIELD_DEBUG_WORK *work )
{
  g3d_unit_Release( work );

	GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
/**
 * フィールドデバッグシステム 常駐処理
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
void FIELD_DEBUG_UpdateProc( FIELD_DEBUG_WORK *work )
{
	if( (work->flag_pos_print == TRUE) && (work->flag_pos_update == TRUE) ){ //座標表示
    switch ( work->print_type )
    {
    case FIELD_DEBUG_PRINT_TYPE_POSITION:
      DebugFieldPosPrint_Proc( work );
      break;
    case FIELD_DEBUG_PRINT_TYPE_MEMORY:
      DebugFieldMemPrint_Proc( work );
      break;
    }
	}
  if( work->flag_hitch_check ){
    g3d_unit_Main( work );
  }
	
	if( work->flag_bgscr_load == TRUE ){ //デバッグ用BGスクリーン反映
    GFL_BG_LoadScreenReq( work->bgFrame );
    work->flag_bgscr_load = FALSE;
	}
}

//--------------------------------------------------------------
/**
 * フィールドデバッグシステム 3D描画常駐処理
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
void FIELD_DEBUG_Draw( FIELD_DEBUG_WORK *work )
{
	if( work->flag_hitch_check && work->flag_pos_update ){ //当たり判定表示
    g3d_unit_Draw( work );
	}
}

//--------------------------------------------------------------
/**
 * フィールドデバッグシステム　ＢＧ設定復帰
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
void FIELD_DEBUG_RecoverBgCont( FIELD_DEBUG_WORK *work )
{
#if 0
  //セットアップしなおし
  G2_SetBG2ControlText(
      GX_BG_SCRSIZE_TEXT_256x256,
      FLDBG_MFRM_EFF1_COLORMODE,
      FLDBG_MFRM_EFF1_SCRBASE,
      FLDBG_MFRM_EFF1_CHARBASE);
#else
  {
    FLDMSGBG *fmb = FIELDMAP_GetFldMsgBG( work->pFieldMainWork );
    FLDMSGBG_ReqResetBG2( fmb );
    work->flag_pos_print = FALSE;
  }
#endif
}

//======================================================================
//	システムフォント表示
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドデバッグシステム　BG面セットアップしなおし
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void resetBgCont( FIELD_DEBUG_WORK *work )
{
  G2_SetBG2ControlText(
      GX_BG_SCRSIZE_TEXT_256x256,
      FLDBG_MFRM_EFF1_COLORMODE,
      FLDBG_MFRM_EFF1_SCRBASE,
      FLDBG_MFRM_EFF1_CHARBASE);
  
  {
    FLDMSGBG *fmb = FIELDMAP_GetFldMsgBG( work->pFieldMainWork );
    FLDMSGBG_ReqResetBG2( fmb );
  }
}

//--------------------------------------------------------------
/**
 * g3dリソース　初期化
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void g3d_unit_Init( FIELD_DEBUG_WORK *work )
{
  int i;

  // 3D管理ユーティリティーの生成
  work->g3d_util = GFL_G3D_UTIL_Create( UNIT_RES_MAX, UNIT_MAX, work->heapID );
  VEC_Set( &work->obj_status.trans, 0, 0, 0 );
  VEC_Set( &work->obj_status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
  MTX_Identity33( &work->obj_status.rotate );
  work->unit_oy = FX32_CONST(0);

  work->unit_idx = -1;

  for(i = 0;i < UNIT_MAX;i++){
    work->unit[i].draw_f = TRUE;
  }
}

//--------------------------------------------------------------
/**
 * g3dリソース  解放　
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void g3d_unit_Release( FIELD_DEBUG_WORK *work )
{
  int i;

  if( work->unit_idx >= 0 ){
    GFL_G3D_UTIL_DelUnit( work->g3d_util, work->unit_idx );
  }
  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( work->g3d_util );
}

//--------------------------------------------------------------
/**
 * g3dリソース  メイン　
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void g3d_unit_Main( FIELD_DEBUG_WORK *work )
{
  int i,j;
  VecFx32 vec;
  fx32  sx,sz; 
  HITCH_UNIT* ph;
  FLDMAPPER_GRIDINFODATA gridData;
  int trg = GFL_UI_KEY_GetTrg();
	const VecFx32 *pos = PLAYERWORK_getPosition( work->player );

  if( work->g3dMapper == NULL ){
		return;
	}
  if( trg & PAD_BUTTON_START ){
    resetBgCont( work );
  }

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ){
    if( trg & PAD_KEY_UP ){
      work->unit_oy += FX32_CONST(8);
		  work->flag_pos_update = TRUE;
    }else if( trg & PAD_KEY_DOWN ){
      work->unit_oy -= FX32_CONST(8);
		  work->flag_pos_update = TRUE;
    }else if( trg & PAD_KEY_LEFT ){
      work->unit_oy -= (FX32_CONST(1));
		  work->flag_pos_update = TRUE;
    }else if( trg & PAD_KEY_RIGHT ){
      work->unit_oy += (FX32_CONST(1));
		  work->flag_pos_update = TRUE;
    }else if( trg & PAD_BUTTON_B ){
      work->unit_oy = 0;
		  work->flag_pos_update = TRUE;
    }else if( trg & PAD_BUTTON_START ){
      work->flag_hitch_y_org ^= 1;
		  work->flag_pos_update = TRUE;
    }
  }
  sx = (pos->x-FX_Mod( pos->x, MAP_UNIT_W))+FX32_CONST(-32+8);
  sz = (pos->z-FX_Mod( pos->z, MAP_UNIT_W))+FX32_CONST(-32+8);
  work->unit_ox = sx;
  work->unit_oz = sz;

  vec.y = pos->y;

  for(i = 0;i < UNIT_MAX;i++){
    work->unit[i].draw_f = TRUE;
    work->unit[i].y = 0;
  }
  for( i = 0; i < 5; i++ )
  {
    vec.z = sz+FX32_CONST(16*i);
    if( vec.z < 0 ){// || vec.z >= FX32_CONST(512)){
      continue;
    }

    for(j = 0; j < 5; j++ ){
      vec.x = sx+FX32_CONST(16*j);
      if( vec.x < 0 ){// || vec.x >= FX32_CONST(512)){
        continue;
      }
      ph = &work->unit[i*5+j];

      if( FLDMAPPER_GetGridData( work->g3dMapper,&vec,&gridData) == FALSE){
        continue;
      }
      ph->y = gridData.height;
      if( MAPATTR_GetHitchFlag( gridData.attr )){
        continue;
      }
      ph->draw_f = FALSE; 
    }
  }
}

//--------------------------------------------------------------
/**
 * g3dリソース  描画
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void g3d_unit_Draw( FIELD_DEBUG_WORK *work )
{
  int i,j,frame;
  GFL_G3D_OBJ* obj;
  HITCH_UNIT* ph;

  if( work->unit_idx < 0 ){
    return;
  }
  for( i = 0; i < 5; i++ )
  {
    work->obj_status.trans.z = work->unit_oz+FX32_CONST(16*i);

    for(j = 0; j < 5; j++ ){
      ph = &work->unit[i*5+j];
      work->obj_status.trans.x = work->unit_ox+FX32_CONST(16*j);
      
      if( work->flag_hitch_y_org ){
        work->obj_status.trans.y = work->unit_oy+ph->y;
      }else{
        work->obj_status.trans.y = work->unit_oy+work->unit[2*5+2].y;
      }

      obj = GFL_G3D_UTIL_GetObjHandle( work->g3d_util, work->unit_idx );
      if( i == 2 && j == 2){
        frame = FX32_CONST(2+ph->draw_f);
      }else{
        frame = FX32_CONST(ph->draw_f);
      }
      GFL_G3D_OBJECT_SetAnimeFrame( obj, 0, &frame );
      GFL_G3D_DRAW_DrawObject( obj, &work->obj_status );
    }
  }
}

//--------------------------------------------------------------
/**
 * システムフォント表示　初期化
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DebugFont_Init( FIELD_DEBUG_WORK *work )
{
  { //BGリソースをクリア
    FLDMSGBG *fmb = FIELDMAP_GetFldMsgBG( work->pFieldMainWork );
    FLDMSGBG_ReleaseBG2Resource( fmb );
    FLDMSGBG_ReqResetBG2( fmb );
  }
  
	{	//BG Frame
		GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, FLDBG_MFRM_EFF1_SCRSIZE, 0,
			GFL_BG_SCRSIZ_256x256, FLDBG_MFRM_EFF1_COLORMODE,
			FLDBG_MFRM_EFF1_SCRBASE, FLDBG_MFRM_EFF1_CHARBASE, FLDBG_MFRM_EFF1_CHARSIZE,
			GX_BG_EXTPLTT_01, FLDBG_MFRM_EFF1_PRI, 0, 0, FALSE
		};
    
		GFL_BG_SetBGControl( work->bgFrame, &bgcntText, GFL_BG_MODE_TEXT );
		GFL_BG_SetVisible( work->bgFrame, VISIBLE_ON );
	}
	
  { //クリアキャラ
  	GFL_BG_FillCharacter( work->bgFrame, 0x00, 1, 0 );
  }

  DebugFont_InitResource( work );
}

//--------------------------------------------------------------
/**
 * システムフォント初期化　リソース初期化
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DebugFont_InitResource( FIELD_DEBUG_WORK *work )
{
	void *buf;
  
  { //クリアキャラ
		GFL_BG_FillScreen( work->bgFrame,
			0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( work->bgFrame );
  }
  
	{	//パレット
		NNSG2dPaletteData *pal;
		buf = GFL_ARC_LoadDataAlloc(
			ARCID_OTHERS, NARC_others_nfont_NCLR, work->heapID );
		GF_ASSERT( buf != NULL );
		
		if( NNS_G2dGetUnpackedPaletteData(buf,&pal) == FALSE ){
			GF_ASSERT( 0 );
		}
		
		GFL_BG_LoadPalette(
        work->bgFrame, pal->pRawData, 32, DEBUG_PANO_FONT*32 );
		GFL_HEAP_FreeMemory( buf );
	}
	
	{	//キャラ
		NNSG2dCharacterData *chr;
    
    buf = GFL_ARC_LoadDataAlloc( ARCID_OTHERS, NARC_others_nfont_NCGR, work->heapID );
		GF_ASSERT( buf != NULL );
		
		if( NNS_G2dGetUnpackedBGCharacterData(buf,&chr) == FALSE ){
			GF_ASSERT( 0 );
		}
		GFL_BG_LoadCharacter( work->bgFrame, chr->pRawData, chr->szByte, 0 );
		GFL_HEAP_FreeMemory( buf );
	}
}

//--------------------------------------------------------------
/**
 * システムフォント表示 一文字表示
 * @param	work	FIELD_DEBUG_WORK
 * @param	x		表示するX座標 キャラ単位
 * @param	y		表示するY座標 キャラ単位
 * @param	msgBuf	表示する文字列
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DebugFont_Put( u16 *screen, char c, u16 x, u16 y )
{
	if(c>=0x61&&c<=(0x61+6)){ //sprintf()16進英字コードを置換
		c = (c-0x61) + 0x41; //0x41 = 'A'
	}
	if(c==0x20){
		c = 0x29;
	}
	if(c=='='){
		c = 0x30+10;
	}
	if(c=='-'){   // マイナス表示が無いのでMに置換
		c = 'M';
	}
	screen[x+y*32] = (DEBUG_PANO_FONT<<12)+(c-0x2F);
}

//--------------------------------------------------------------
/**
 * システムフォント表示 表示
 * @param	work	FIELD_DEBUG_WORK
 * @param	x		表示するX座標 キャラ単位
 * @param	y		表示するY座標 キャラ単位
 * @param	msgBuf	表示する文字列
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DebugFont_Print(
	FIELD_DEBUG_WORK *work, u16 x, u16 y, const char *msgBuf )
{
	u16 n = 0;
	u16 *screen;
	screen = GFL_BG_GetScreenBufferAdrs( work->bgFrame );
	while( msgBuf[n] != 0 ){
		DebugFont_Put(screen,msgBuf[n],x+n,y);
		n++;
	}
	
	work->flag_bgscr_load = TRUE;
}

//--------------------------------------------------------------
/**
 * システムフォント表示　行クリア
 * @param	y
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DebugFont_ClearLine( FIELD_DEBUG_WORK *work, u16 y )
{
	char buf[33];
	MI_CpuFill8( buf, 0x20, 32 );
	buf[32] = 0;
	DebugFont_Print( work, 0, y, buf );
}

//======================================================================
//	フィールドデバッグ　フィールド座標表示
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド座標表示
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
void FIELD_DEBUG_SetPrint( FIELD_DEBUG_WORK *work, FIELD_DEBUG_PRINT_TYPE type )
{
  work->print_type = type;
	if( work->flag_pos_print == FALSE ){
		GFL_BG_SetVisible( work->bgFrame, VISIBLE_OFF );
		work->flag_pos_print = TRUE;
		work->flag_pos_update = TRUE;
    resetBgCont( work );
    DebugFont_InitResource( work );
		GFL_BG_SetVisible( work->bgFrame, VISIBLE_ON );
	}else{
		work->flag_pos_print = FALSE;
    
    // スクリーンのクリア
    GFL_BG_ClearScreen( work->bgFrame );
		GFL_BG_LoadScreenReq( work->bgFrame );
	}
  //描画コールバックのセット
  FIELD_DEBUG_SetDrawCallBackFunc(work->flag_pos_print);
}

//--------------------------------------------------------------
/**
 * フィールド当たり判定表示
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
void FIELD_DEBUG_SetHitchCheckPrint( FIELD_DEBUG_WORK *work )
{
  work->flag_pos_update = TRUE; //初回描画リクエスト
	work->flag_hitch_check ^= 1;

  if( work->flag_hitch_check && work->unit_idx < 0 ){
    GFL_G3D_OBJ* obj;
    work->unit_idx = GFL_G3D_UTIL_AddUnit( work->g3d_util, &sc_g3d_setup );
    obj = GFL_G3D_UTIL_GetObjHandle( work->g3d_util, work->unit_idx );
    GFL_G3D_OBJECT_EnableAnime( obj, 0 );
  }
  work->unit_oy = FX32_CONST(0);
}

//----------------------------------------------------------------------------
/**
 *	@brief  表示更新フラグ　設定
 *
 *	@param	work
 *	@param	flag 
 */
//-----------------------------------------------------------------------------
void FIELD_DEBUG_SetPosUpdateFlag( FIELD_DEBUG_WORK *work, BOOL flag )
{
  work->flag_pos_update = flag;
}

//==============================================================================
/**
 * リンクドVRAMマネージャの内部情報を取得するためのコールバック
 *
 *
 * @retval  none
 */
//==============================================================================
static void cb_DumpLnkVramManager(u32 addr, u32 szByte, void* pUserData )
{
	// 合計サイズを計算
    (*((u32*)pUserData)) += szByte;
}

//--------------------------------------------------------------
/**
 * フィールド座標表示
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DebugFieldPosPrint_Proc( FIELD_DEBUG_WORK *work )
{
	char str[256];
	GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( work->pFieldMainWork );
	PLAYER_WORK *player = GAMESYSTEM_GetMyPlayerWork( gsys );
	const VecFx32 *pos = PLAYERWORK_getPosition( player );
  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
    resetBgCont( work );
  }
	
	{	//座標表示
    u32 gx, gz;

		DebugFont_ClearLine( work, 0 );
		sprintf( str, "X %d %xH GRID %d",
			FX_Whole(pos->x), pos->x, SIZE_GRID_FX32(pos->x) );
		DebugFont_Print( work, 0, 0, str );
		
		DebugFont_ClearLine( work, 1 );
		sprintf( str, "Y %d %xH GRID %d",
			FX_Whole(pos->y), pos->y, SIZE_GRID_FX32(pos->y) );
		DebugFont_Print( work, 0, 1, str );
		
		DebugFont_ClearLine( work, 2 );
		sprintf( str, "Z %d %xH GRID %d",
			FX_Whole(pos->z), pos->z, SIZE_GRID_FX32(pos->z) );
		DebugFont_Print( work, 0, 2, str );

		DebugFont_ClearLine( work, 3 );
    FLDMAPPER_GetBlockXZPos( work->g3dMapper, &gx, &gz );
    sprintf( str, "BLOCK (%d,%d) LOCAL (%d,%d)", gx, gz, SIZE_GRID_FX32(pos->x)%32, SIZE_GRID_FX32(pos->z)%32 );
    DebugFont_Print( work, 0, 3, str );
	}
	
	{	//マップアトリビュート表示
		u32 attr;
		int x,y,z;
		VecFx32 a_pos;
		const FLDMAPPER *pG3DMapper;
		FLDMAPPER_GRIDINFO gridInfo;
		
		pG3DMapper = FIELDMAP_GetFieldG3Dmapper( work->pFieldMainWork );
		
		if( pG3DMapper == NULL ){
			return;
		}
		
		sprintf( str, "ATTRIBUTE" );
		DebugFont_ClearLine( work, 4 );
		DebugFont_Print( work, 0, 4, str );
		
		a_pos = *pos;
		a_pos.z -= GRID_SIZE_FX32( 1 );
		
		for( z = 0, y = 5; z < 3; z++, y++, a_pos.z += GRID_SIZE_FX32(1) ){
			DebugFont_ClearLine( work, y );
			for( x = 0, a_pos.x = pos->x - GRID_SIZE_FX32(1);
					x < 3*10; x += 10, a_pos.x += GRID_SIZE_FX32(1) ){
#if 0
				if( FLDMAPPER_GetGridInfo(pG3DMapper,&a_pos,&gridInfo) ){
					attr = gridInfo.gridData[0].attr;
					sprintf( str, "%08xH", attr );
				}else{
					sprintf( str, "GET ERROR", attr );
				}
#else
        {
          FLDMAPPER_GRIDINFODATA gridData;
          if( FLDMAPPER_GetGridData(pG3DMapper,&a_pos,&gridData)){
					  sprintf( str, "%08xH", gridData.attr );
          }else{
  					sprintf( str, "GET ERROR", gridData.attr );
          }
        }
#endif
				DebugFont_Print( work, x, y, str );
			}
		}
	}
	
	{	//マップブロック　ZONE_ID表示
		u32 zone_id = MAP_MATRIX_ZONE_ID_NON;
    GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
		MAP_MATRIX *pMatrix = GAMEDATA_GetMapMatrix( gamedata );
		
		if( MAP_MATRIX_CheckVectorPosRange(pMatrix,pos->x,pos->z) == TRUE ){
			zone_id = MAP_MATRIX_GetVectorPosZoneID(
					pMatrix, pos->x, pos->z );
		}
		
		DebugFont_ClearLine( work, 8 );
		sprintf( str, "ZONE ID " );
		DebugFont_Print( work, 0, 8, str );
		
		if( zone_id == MAP_MATRIX_ZONE_ID_NON ){
			sprintf( str, "ERROR" );
		}else{
			ZONEDATA_DEBUG_GetZoneName( str, zone_id );
		}
		
		DebugFont_Print( work, 8, 8, str );
	}

  { // レール座標の表示
    FIELD_PLAYER* fld_player = FIELDMAP_GetFieldPlayer( work->pFieldMainWork );

    
    if( PLAYERWORK_getPosType(player) == LOCATION_POS_TYPE_RAIL )
    {
      FIELD_RAIL_WORK* railWork = FIELD_PLAYER_GetNoGridRailWork( fld_player );
      u16 index, ofs, w_ofs;

      FIELD_RAIL_WORK_GetNotMinusRailParam( railWork, &index, &ofs, &w_ofs );

      sprintf( str, "RAIL INDEX %d FRONT %d SIDE %d",
        index, ofs, w_ofs );
    }
    else
    {
      sprintf( str, "RAIL DATA NONE" );
    }
		DebugFont_ClearLine( work, 9 );
		DebugFont_Print( work, 0, 9, str );
  }

  //3D描画情報
  {
    DebugFont_ClearLine( work, 10 );
    sprintf( str, "VTX:%d",DbgRenderInfo.Vtx);
		DebugFont_Print( work, 0, 10, str );
    sprintf( str, "PLY:%d",DbgRenderInfo.Ply);
		DebugFont_Print( work, 16, 10, str );

    DebugFont_ClearLine( work, 11 );
    sprintf( str, "MAT:%d",DbgRenderInfo.Mat);
		DebugFont_Print( work, 0, 11, str );
    sprintf( str, "SHP:%d",DbgRenderInfo.Shp);
		DebugFont_Print( work, 10, 11, str );
    sprintf( str, "NUM:%d",DbgRenderInfo.Num);
		DebugFont_Print( work, 20, 11, str );

    DebugFont_ClearLine( work, 12 );
    sprintf( str, "LAND:%d",DbgRenderInfo.DrawLandNum);
		DebugFont_Print( work, 0, 12, str );
  }

  {
    u32 size = 0;

    NNS_GfdDumpLnkTexVramManagerEx( cb_DumpLnkVramManager,NULL,&size );
    DebugFont_ClearLine( work, 14 );
    sprintf( str, "TEXV REM:%d USE:%d",size, (128*3*1024)-size);
		DebugFont_Print( work, 0, 14, str );
	
    size = 0;
    NNS_GfdDumpLnkPlttVramManagerEx( cb_DumpLnkVramManager,&size );
    DebugFont_ClearLine( work, 15 );
    sprintf( str, "PLTV REM:%d USE:%d",size, (16*1024)-size);
		DebugFont_Print( work, 0, 15, str );
  }
}

//--------------------------------------------------------------
/**
 * フィールドメモリ状況表示
 * @param	work	FIELD_DEBUG_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DebugFieldMemPrint_Proc( FIELD_DEBUG_WORK *work )
{
	char str[256];
	GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( work->pFieldMainWork );
	PLAYER_WORK *player = GAMESYSTEM_GetMyPlayerWork( gsys );
	const VecFx32 *pos = PLAYERWORK_getPosition( player );
  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT ){
    resetBgCont( work );
  }

  {
    DebugFont_ClearLine( work, 1 );
    sprintf( str, "ALLOCATABLE   FREE" );
    DebugFont_Print( work, 7, 0, str );

    DebugFont_ClearLine( work, 1 );
    sprintf( str, "HEAP_FIELD  %6x(%6x)", GFI_HEAP_GetHeapAllocatableSize( HEAPID_FIELDMAP ),
        GFL_HEAP_GetHeapFreeSize( HEAPID_FIELDMAP ) );
		DebugFont_Print( work, 0,  1, str );

    DebugFont_ClearLine( work, 2 );
    sprintf( str, "HEAP_SUBSCRN%6x(%6x)", GFI_HEAP_GetHeapAllocatableSize( HEAPID_FIELD_SUBSCREEN ),
        GFL_HEAP_GetHeapFreeSize( HEAPID_FIELD_SUBSCREEN ) );
		DebugFont_Print( work, 0,  2, str );

    DebugFont_ClearLine( work, 4 );
    sprintf( str, "BUILDMODEL USE=%6x", FIELD_BMODEL_MAN_GetUseResourceMemorySize() );
		DebugFont_Print( work, 0,  4, str );
#if 0
    DebugFont_ClearLine( work, 5 );
    sprintf( str, "MOVEMODEL USE=%6x", DEBUG_MMDL_GetUseResourceMemorySize() );
		DebugFont_Print( work, 0,  5, str );
#endif
  }

  {
    u32 size = 0;

    NNS_GfdDumpLnkTexVramManagerEx( cb_DumpLnkVramManager,NULL,&size );
    DebugFont_ClearLine( work, 6 );
    sprintf( str, "TEXV REM:%d USE:%d",size, (128*3*1024)-size);
		DebugFont_Print( work, 0,  6, str );
	
    size = 0;
    NNS_GfdDumpLnkPlttVramManagerEx( cb_DumpLnkVramManager,&size );
    DebugFont_ClearLine( work, 7 );
    sprintf( str, "PLTV REM:%d USE:%d",size, (16*1024)-size);
		DebugFont_Print( work, 0,  7, str );
  }
}
	
//--------------------------------------------------------------
/**
 * 描画コールバック関数セット
 * @param	sw    TRUE:セット　FALSE:リセット
 * @retval	nothing
 */
//--------------------------------------------------------------
void FIELD_DEBUG_SetDrawCallBackFunc(const BOOL sw)
{
  if (sw){
    GFL_G3D_SetDrawCallBack( DbgDrawCallBackFunc, &DbgRenderInfo );
  }else {
    GFL_G3D_SetDrawCallBack( NULL, NULL );
  }
}

//--------------------------------------------------------------
/**
 * 描画コールバックワーククリア
 * @param	none
 * @retval	nothing
 */
//--------------------------------------------------------------
void FIELD_DEBUG_ClearDrawCallBackWork(void)
{
  DbgRenderInfo.Vtx = 0;
  DbgRenderInfo.Ply = 0;
  DbgRenderInfo.Mat = 0;
  DbgRenderInfo.Shp = 0;
  DbgRenderInfo.Num = 0;

  DbgRenderInfo.DrawLandNum = 0;
}

//--------------------------------------------------------------
/**
 * 描画コールバック関数
 * @param	renderobj     レンダーオブジェポインタ
 * @pamra work          コールバックワークポインタ
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DbgDrawCallBackFunc( NNSG3dRenderObj *renderobj, void *work )
{
  RENDER_INFO * info = (RENDER_INFO*)(work);
  info->Vtx += renderobj->resMdl->info.numVertex;
  info->Ply += renderobj->resMdl->info.numPolygon;
  info->Mat += renderobj->resMdl->info.numMat;
  info->Shp += renderobj->resMdl->info.numShp;
  info->Num++;
}

//--------------------------------------------------------------
/**
 * 地面描画回数カウント
 * @param	none
 * @retval	nothing
 */
//--------------------------------------------------------------
void FIELD_DEBUG_AddDrawLandNum(void)
{
  DbgRenderInfo.DrawLandNum++;
}

#endif  //PM_DEBUG
