//======================================================================
/**
 *
 * @file	fldmmdl_draw_0.c
 * @brief	フィールド動作モデル 描画処理その０
 * @author	kagaya
 * @date	05.07.13
 *
 */
//======================================================================
#include "fldmmdl.h"
#include "fldmmdl_procdraw.h"

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
/// ビルボード共通オフセット表示座標
//--------------------------------------------------------------
#define MMDL_BBD_OFFS_POS_Y (-FX32_ONE*2)  //(FX32_ONE*4)
#define MMDL_BBD_OFFS_POS_Z (FX32_ONE*4)  //(-FX32_ONE*8)

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
/// ANMCNT_WORK
//--------------------------------------------------------------
typedef struct
{
  u8 set_anm_dir;
  u8 set_anm_status;
  u8 next_walk_frmidx;
  u8 dmy;
}ANMCNT_WORK;

//--------------------------------------------------------------
///	DRAW_BLACT_WORK
//--------------------------------------------------------------
typedef struct
{
	u16 actID;
	u8 set_anm_dir;
	u8 set_anm_status;
  
  ANMCNT_WORK anmcnt_work;

#if 0
  fx32 TestScale;
  fx32 TestScaleAdd;
#endif
}DRAW_BLACT_WORK;

//======================================================================
//	proto
//======================================================================
static void blact_SetCommonOffsPos( VecFx32 *pos )
{
  pos->y += MMDL_BBD_OFFS_POS_Y;
#ifdef MMDL_BBD_DRAW_OFFS_Z_USE
  pos->z += MMDL_BBD_OFFS_POS_Z;
#endif
}

static u16 blact_GetDrawDir( MMDL *mmdl )
{
  const MMDLSYS *mmdlsys = MMDL_GetMMdlSys( mmdl );
  u16 angle = MMDLSYS_GetTargetCameraAngleYaw( mmdlsys );
  u16 dir = MMDL_GetDirDisp( mmdl );
  dir = MMDL_TOOL_GetAngleYawToDirFour( dir, angle );
  return( dir );
}

static void blactAnmControl_Init( ANMCNT_WORK *work )
{
  MI_CpuClear8( work, sizeof(ANMCNT_WORK) );
  work->set_anm_dir = DIR_MAX;
}

static void blactAnmControl_Update( ANMCNT_WORK *work,
    MMDL *mmdl, GFL_BBDACT_SYS *actSys, u16 actID )
{
  u16 init_flag = FALSE;
  u16 dir = blact_GetDrawDir( mmdl );
  u16 status = MMDL_GetDrawStatus( mmdl );
  u16 anm_idx = (status * DIR_MAX4) + dir;
  
  if( dir != work->set_anm_dir ) //方向更新
  {
    init_flag = TRUE;
    work->set_anm_dir = dir;
    work->set_anm_status = status;
    work->next_walk_frmidx = 0;
		GFL_BBDACT_SetAnimeIdx( actSys, actID, anm_idx );
  }
  else if( status != work->set_anm_status ) //ステータス更新
  {
    init_flag = TRUE;
    
    if( status == DRAW_STA_STOP ) //停止タイプ
    {
      work->next_walk_frmidx = GFL_BBDACT_GetAnimeFrmIdx( actSys, actID );
      
      {
        u8 tbl[4] = {0,2,2,0};
        
        if( work->next_walk_frmidx >= 4 ){
          work->next_walk_frmidx = 0;
        }
        
        work->next_walk_frmidx = tbl[work->next_walk_frmidx];
      }
      
		  GFL_BBDACT_SetAnimeIdx( actSys, actID, anm_idx );
    }
    else //歩きタイプ
    {
      if( work->set_anm_status != DRAW_STA_STOP ) //移動系アニメを行っていた
      {
        work->next_walk_frmidx =
          GFL_BBDACT_GetAnimeFrmIdx( actSys, actID );
        
        if( work->next_walk_frmidx >= 2 )
        {
          work->next_walk_frmidx = 0;
        }
        else
        {
          work->next_walk_frmidx = 2;
        }
      }
      
      GF_ASSERT( work->next_walk_frmidx < 4 );
		  GFL_BBDACT_SetAnimeIdx( actSys, actID, anm_idx );
      GFL_BBDACT_SetAnimeFrmIdx( actSys, actID, work->next_walk_frmidx );
    }
    
    work->set_anm_status = status;
  }
  
  {
		BOOL flag = TRUE;
    
		if( init_flag == FALSE && MMDL_CheckDrawPause(mmdl) == TRUE ){
			flag = FALSE;
		}
		GFL_BBDACT_SetAnimeEnable( actSys, actID, flag );
    
    flag = TRUE; 
    if( MMDL_CheckStatusBitVanish(mmdl) == TRUE ){
      flag = FALSE;
    }
    GFL_BBDACT_SetDrawEnable( actSys, actID, flag );
	}
}

//======================================================================
//	描画処理　描画無し
//======================================================================
//--------------------------------------------------------------
/**
 * 描画無し　初期化
 * @param	mmdl		MMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Init( MMDL * mmdl )
{
	MMDL_SetStatusBitVanish( mmdl, TRUE );
	MMDL_OnStatusBit( mmdl, MMDL_STABIT_SHADOW_VANISH );
}

//--------------------------------------------------------------
/**
 * 描画無し　描画
 * @param	mmdl		MMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Draw( MMDL * mmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画無し　削除
 * @param	mmdl	MMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Delete( MMDL * mmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画無し　退避
 * @param	mmdl		MMDL * 
 * @retval	int			TRUE=初期化成功
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Push( MMDL * mmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画無し　復帰
 * 退避した情報を元に再描画。
 * @param	mmdl		MMDL * 
 * @retval	int			TRUE=初期化成功
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Pop( MMDL * mmdl )
{
}

//--------------------------------------------------------------
///	描画処理　描画なし　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_Non =
{
	MMdl_DrawNon_Init,
	MMdl_DrawNon_Draw,
	MMdl_DrawNon_Delete,
	MMdl_DrawNon_Push,
	MMdl_DrawNon_Pop,
	NULL,
};

//======================================================================
//	描画処理　ビルボード　自機専用
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機専用　初期化
 * @param	mmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawHero_Init( MMDL *mmdl )
{
  u16 code;
	DRAW_BLACT_WORK *work;
  
	work = MMDL_InitDrawProcWork( mmdl, sizeof(DRAW_BLACT_WORK) );
	work->set_anm_dir = DIR_NOT;
  
  code = MMDL_GetOBJCode( mmdl );
  blactAnmControl_Init( &work->anmcnt_work );

	if( MMDL_BLACTCONT_AddActor(mmdl,code,&work->actID) == TRUE ){
    MMDL_CallDrawProc( mmdl );
  }
#if 0  
  { //test
    work->TestScale = FX16_ONE*8;
    work->TestScaleAdd = 0x0100;
  }
#endif
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機専用　削除
 * @param	mmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawHero_Delete( MMDL *mmdl )
{
	DRAW_BLACT_WORK *work;
	work = MMDL_GetDrawProcWork( mmdl );
	MMDL_BLACTCONT_DeleteActor( mmdl, work->actID );
}

#if 0
void	GFL_BBDACT_SetAnimeIdx(
    GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 animeIdx );
void	GFL_BBDACT_SetAnimeFrmIdx(
    GFL_BBDACT_SYS* bbdActSys, u16 actIdx, u16 animeFrmIdx )
#endif

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機専用　描画
 * @param	mmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawHero_Draw( MMDL *mmdl )
{
	VecFx32 pos;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
	blactAnmControl_Update( &work->anmcnt_work, mmdl, actSys, work->actID );
  
	MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

#if 0
static void DrawHero_Draw( MMDL *mmdl )
{
	VecFx32 pos;
	BOOL chg_dir = FALSE;
	u16 dir,anm_id,status;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
	
	dir = MMDL_GetDirDisp( mmdl );
	status = MMDL_GetDrawStatus( mmdl );
	GF_ASSERT( status < DRAW_STA_MAX_HERO );
  
	anm_id = status * DIR_MAX4;
	anm_id += dir;
	
	if( work->set_anm_dir != dir ){ //方向更新
		work->set_anm_dir = dir;
		work->set_anm_status = status;
		GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
		chg_dir = TRUE;
	}else if( work->set_anm_status != status ){
		u16 frame = 0;
		
		switch( work->set_anm_status ){
		case DRAW_STA_WALK_32F:
		case DRAW_STA_WALK_16F:
		case DRAW_STA_WALK_8F:
		case DRAW_STA_WALK_4F:
		case DRAW_STA_WALK_2F:
		case DRAW_STA_DASH_4F:
			if( GFL_BBDACT_GetAnimeFrmIdx(actSys,work->actID) < 2 ){
				frame = 2;
			}
			break;
		}
		
		GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
		GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, frame );
		work->set_anm_status = status;
	}
	
	MMDL_GetDrawVectorPos( mmdl, &pos );

	pos.y += MMDL_BBD_OFFS_POS_Y;
  pos.z += MMDL_BBD_OFFS_POS_Z;
	
	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
	{
		BOOL flag = TRUE;
		if( chg_dir == FALSE && MMDL_CheckDrawPause(mmdl) == TRUE ){
			flag = FALSE;
		}
		GFL_BBDACT_SetAnimeEnable( actSys, work->actID, flag );
    
    flag = TRUE; 
    
    if( MMDL_CheckStatusBitVanish(mmdl) == TRUE ){
      flag = FALSE;
    }
    
    GFL_BBDACT_SetDrawEnable( actSys, work->actID, flag );
	}

#if 0 
  { //ScaleTest
    /*
    void	GFL_BBD_SetObjectSiz( GFL_BBD_SYS* billboardSys, int objIdx, const fx16* sizX, const fx16* sizY );
    */
    GFL_BBD_SYS *bbdSys = GFL_BBDACT_GetBBDSystem( actSys );
    int idx = GFL_BBDACT_GetBBDActIdxResIdx( actSys, work->actID );
    fx16 sx = FX16_ONE*4 - 1;
    fx16 sy = FX16_ONE*4 - 1;
    
    work->TestScale += work->TestScaleAdd;
    
    if( work->TestScale <= FX16_ONE ){
      work->TestScale = FX16_ONE;
      work->TestScaleAdd = -work->TestScaleAdd;
    }else if( work->TestScale >= (FX16_ONE*8-1) ){
      work->TestScale = (FX16_ONE*8-1);
      work->TestScaleAdd = -work->TestScaleAdd;
    }
    
    sx = work->TestScale;
    GFL_BBD_SetObjectSiz( bbdSys, idx, &sx, &sy );
    
    if( GFL_UI_KEY_GetCont() & PAD_BUTTON_A ){
      KAGAYA_Printf( "自機横サイズ 0x%x\n", work->TestScale );
    }else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_B ){
      VecFx32 scale;
      GFL_BBD_GetScale( bbdSys, &scale );
      KAGAYA_Printf( "ビルボードスケール 0x%x,0x%x,0x%x\n",
          scale.x, scale.y, scale.z );
    }
    
    {
      BOOL flip = TRUE;
      GFL_BBD_SetObjectFlipT( bbdSys, idx, &flip );
    }
  }
#endif
}
#endif

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機専用　取得。
 * ビルボードアクターIDを返す。
 * @param	mmdl	MMDL
 * @param	state	特に無し
 * @retval	u32	GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static u32 DrawHero_GetBlActID( MMDL *mmdl, u32 state )
{
	DRAW_BLACT_WORK *work;
	work = MMDL_GetDrawProcWork( mmdl );
	return( work->actID );
}

//--------------------------------------------------------------
//	描画処理　ビルボード　自機　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_Hero =
{
	DrawHero_Init,
	DrawHero_Draw,
	DrawHero_Delete,
	DrawHero_Delete,	//退避
	DrawHero_Init,    //本当は復帰
	DrawHero_GetBlActID,
};

//======================================================================
//  描画処理　ビルボード　自転車自機専用
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自転車自機専用　描画
 * @param	mmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawCycleHero_Draw( MMDL *mmdl )
{
	VecFx32 pos;
	BOOL chg_dir = FALSE;
	u16 dir,anm_id,status;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
	
  dir = blact_GetDrawDir( mmdl );
	status = MMDL_GetDrawStatus( mmdl );
	GF_ASSERT( status < DRAW_STA_MAX_HERO );

#if 0  //無理
  if( status == DRAW_STA_WALK_4F ){ //090731 速度は4Fだが8Fで見せたいと要望
    status = DRAW_STA_WALK_8F;
  }
#endif

	anm_id = status * DIR_MAX4;
	anm_id += dir;
  
	if( work->set_anm_dir != dir ){ //方向更新
		work->set_anm_dir = dir;
		work->set_anm_status = status;
		GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
		chg_dir = TRUE;
	}else if( work->set_anm_status != status ){
		u16 frame = 0;
		
		switch( work->set_anm_status ){
		case DRAW_STA_WALK_32F:
		case DRAW_STA_WALK_16F:
		case DRAW_STA_WALK_8F:
		case DRAW_STA_WALK_4F:
		case DRAW_STA_WALK_2F:
		case DRAW_STA_DASH_4F:
			if( GFL_BBDACT_GetAnimeFrmIdx(actSys,work->actID) < 2 ){
				frame = 2;
			}
			break;
		}
		
		GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
		GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, frame );
		work->set_anm_status = status;
	}
	
	MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
	
	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
	{
		BOOL flag = TRUE;
		if( chg_dir == FALSE && MMDL_CheckDrawPause(mmdl) == TRUE ){
			flag = FALSE;
		}
		GFL_BBDACT_SetAnimeEnable( actSys, work->actID, flag );
    
    flag = TRUE; 
    
    if( MMDL_CheckStatusBitVanish(mmdl) == TRUE ){
      flag = FALSE;
    }
    
    GFL_BBDACT_SetDrawEnable( actSys, work->actID, flag );
	}
}

//--------------------------------------------------------------
/// 描画処理　自転車自機　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_CycleHero =
{
  DrawHero_Init,
	DrawCycleHero_Draw,
	DrawHero_Delete,
	DrawHero_Delete,	//退避
	DrawHero_Init,    //本当は復帰
	DrawHero_GetBlActID,
};

//======================================================================
//  描画処理　ビルボード　波乗り自機専用
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　波乗り自機専用　描画
 * @param	mmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawSwimHero_Draw( MMDL *mmdl )
{
	VecFx32 pos;
	BOOL chg_dir = FALSE;
	u16 dir,anm_id,status;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
	
  dir = blact_GetDrawDir( mmdl );
	status = MMDL_GetDrawStatus( mmdl );
  GF_ASSERT( dir < DIR_MAX4 );
  anm_id = dir;

#if 0
  {
    u16 idx = GFL_BBDACT_GetAnimeFrmIdx(actSys,work->actID);
    KAGAYA_Printf("波乗りアニメ %d\n", idx );
  }
#endif

	if( work->set_anm_dir != dir ){ //方向更新
		work->set_anm_dir = dir;
		work->set_anm_status = status;
		GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
		chg_dir = TRUE;
	}else if( work->set_anm_status != status ){
		u16 frame = 0;
		GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
		GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, frame );
		work->set_anm_status = status;
	}
	
	MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );

	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );

	{
		BOOL flag = TRUE;
		if( chg_dir == FALSE && MMDL_CheckDrawPause(mmdl) == TRUE ){
			flag = FALSE;
		}
		GFL_BBDACT_SetAnimeEnable( actSys, work->actID, flag );
    
    flag = TRUE; 
    
    if( MMDL_CheckStatusBitVanish(mmdl) == TRUE ){
      flag = FALSE;
    }
    
    GFL_BBDACT_SetDrawEnable( actSys, work->actID, flag );
	}
}

//--------------------------------------------------------------
/// 描画処理　波乗り自機　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_SwimHero =
{
  DrawHero_Init,
	DrawSwimHero_Draw,
	DrawHero_Delete,
	DrawHero_Delete,	//退避
	DrawHero_Init,    //本当は復帰
	DrawHero_GetBlActID,
};

//======================================================================
//	描画処理　ビルボード　汎用
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　汎用　初期化
 * @param	mmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_Init( MMDL *mmdl )
{
  u16 code;
	DRAW_BLACT_WORK *work;
  
	work = MMDL_InitDrawProcWork( mmdl, sizeof(DRAW_BLACT_WORK) );
	work->set_anm_dir = DIR_NOT;
  
  code = MMDL_GetOBJCode( mmdl );
  blactAnmControl_Init( &work->anmcnt_work );

	if( MMDL_BLACTCONT_AddActor(mmdl,code,&work->actID) == TRUE ){
    MMDL_CallDrawProc( mmdl );
  }
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　汎用　削除
 * @param	mmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_Delete( MMDL *mmdl )
{
	DRAW_BLACT_WORK *work;
	work = MMDL_GetDrawProcWork( mmdl );
	MMDL_BLACTCONT_DeleteActor( mmdl, work->actID );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　汎用　描画
 * @param	mmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_Draw( MMDL *mmdl )
{
	VecFx32 pos;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
#if 0
  if( MMDL_GetOBJID(mmdl) == 1 && MMDL_GetOBJCode(mmdl) == WOMAN2 ){
    KAGAYA_Printf( "きました\n" );
  }
#endif
  
	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
	blactAnmControl_Update( &work->anmcnt_work, mmdl, actSys, work->actID );
  
	MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　汎用＋常にアニメ　描画
 * @param	mmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_DrawAlwaysAnime( MMDL *mmdl )
{
	VecFx32 pos;
	BOOL chg_dir = FALSE;
	u16 dir,anm_id;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){
    return;
  }
  
	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
  dir = blact_GetDrawDir( mmdl );
  
	anm_id = DRAW_STA_WALK_16F * DIR_MAX4;
	anm_id += dir;
	
	if( work->set_anm_dir != dir ){ //方向更新
		work->set_anm_dir = dir;
		GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
		chg_dir = TRUE;
	}
	
	MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
	
	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );

	{
		BOOL flag = TRUE;
		
    if( chg_dir == FALSE && MMDL_CheckDrawPause(mmdl) == TRUE ){
			flag = FALSE;
		}
    
		GFL_BBDACT_SetAnimeEnable( actSys, work->actID, flag );
    
    flag = TRUE; 
    
    if( MMDL_CheckStatusBitVanish(mmdl) == TRUE ){
      flag = FALSE;
    }
    
    GFL_BBDACT_SetDrawEnable( actSys, work->actID, flag );
	}
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　PCお姉さん
 * @param	mmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_DrawPCWoman( MMDL *mmdl )
{
	VecFx32 pos;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
  
  {
    u16 status = MMDL_GetDrawStatus( mmdl );
    
    if( status == DRAW_STA_PC_BOW ){
      u16 init_flag = FALSE;
      u16 dir = blact_GetDrawDir( mmdl );
      u16 anm_idx = (status * DIR_MAX4);
      ANMCNT_WORK *anmcnt = &work->anmcnt_work;
      
      if( status != anmcnt->set_anm_status ){
        init_flag = TRUE;
        anmcnt->set_anm_dir = dir;
        anmcnt->set_anm_status = status;
        anmcnt->next_walk_frmidx = 0;
		    GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_idx );
      }
      
      {
    		BOOL flag = TRUE;
        
    		if( init_flag == FALSE && MMDL_CheckDrawPause(mmdl) == TRUE ){
    			flag = FALSE;
    		}
    		GFL_BBDACT_SetAnimeEnable( actSys, work->actID, flag );
        
        flag = TRUE; 
        if( MMDL_CheckStatusBitVanish(mmdl) == TRUE ){
          flag = FALSE;
        }
        GFL_BBDACT_SetDrawEnable( actSys, work->actID, flag );
    	}
    }else{
	    blactAnmControl_Update( &work->anmcnt_work, mmdl, actSys, work->actID );
    }
  }

	MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　汎用　取得。
 * ビルボードアクターIDを返す。
 * @param	mmdl	MMDL
 * @param	state	特に無し
 * @retval	u32	GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static u32 DrawBlAct_GetBlActID( MMDL *mmdl, u32 state )
{
	DRAW_BLACT_WORK *work;
	work = MMDL_GetDrawProcWork( mmdl );
	return( work->actID );
}

//--------------------------------------------------------------
//	描画処理　ビルボード　汎用　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlAct =
{
	DrawBlAct_Init,
	DrawBlAct_Draw,
	DrawBlAct_Delete,
	DrawBlAct_Delete,	//本当は退避
	DrawBlAct_Init,		//本当は復帰
	DrawBlAct_GetBlActID,
};

//--------------------------------------------------------------
//	描画処理　ビルボード　汎用+常にアニメ　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlActAlwaysAnime =
{
	DrawBlAct_Init,
	DrawBlAct_DrawAlwaysAnime,
	DrawBlAct_Delete,
	DrawBlAct_Delete,	//本当は退避
	DrawBlAct_Init,		//本当は復帰
	DrawBlAct_GetBlActID,
};

//--------------------------------------------------------------
//	描画処理　ビルボード　PCお姉さん　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlActPCWoman =
{
	DrawBlAct_Init,
  DrawBlAct_DrawPCWoman,
	DrawBlAct_Delete,
	DrawBlAct_Delete,	//本当は退避
	DrawBlAct_Init,		//本当は復帰
	DrawBlAct_GetBlActID,
};

//======================================================================
//  描画処理　ビルボード 自機アイテムゲット
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機アイテムゲット　描画
 * @param	mmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawItemGetHero_Draw( MMDL *mmdl )
{
	VecFx32 pos;
	u16 status;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
	status = MMDL_GetDrawStatus( mmdl );
  
  if( work->set_anm_status >= DRAW_STA_ITEMGET_MAX ){ //エラーフォロー
		work->set_anm_status = DRAW_STA_ITEMGET_STOP;
		GFL_BBDACT_SetAnimeIdx( actSys, work->actID, work->set_anm_status );
		GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, 0 );
  }
 
  if( status < DRAW_STA_ITEMGET_MAX ){
    if( work->set_anm_status != status ){
		  work->set_anm_status = status;
		  GFL_BBDACT_SetAnimeIdx( actSys, work->actID, work->set_anm_status );
		  GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, 0 );
	  }
  }
  
	MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  
	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
  
	{
		BOOL flag = TRUE;
		if( MMDL_CheckDrawPause(mmdl) == TRUE ){
			flag = FALSE;
		}
		GFL_BBDACT_SetAnimeEnable( actSys, work->actID, flag );
    
    flag = TRUE; 
    if( MMDL_CheckStatusBitVanish(mmdl) == TRUE ){
      flag = FALSE;
    }
    GFL_BBDACT_SetDrawEnable( actSys, work->actID, flag );
	}
}

//--------------------------------------------------------------
/// 描画処理　自機アイテムゲット　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_ItemGetHero =
{
  DrawHero_Init,
	DrawItemGetHero_Draw,
	DrawHero_Delete,
	DrawHero_Delete,	//退避
	DrawHero_Init,    //本当は復帰
	DrawHero_GetBlActID,
};

//======================================================================
//  自機PC預け
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機PC預け　描画
 * @param	mmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawPCAzukeHero_Draw( MMDL *mmdl )
{
	VecFx32 pos;
	u16 status;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
	status = MMDL_GetDrawStatus( mmdl );
  
  if( work->set_anm_status >= DRAW_STA_PCAZUKE_MAX ){ //エラーフォロー
		work->set_anm_status = DRAW_STA_ITEMGET_STOP;
		GFL_BBDACT_SetAnimeIdx( actSys, work->actID, work->set_anm_status );
		GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, 0 );
  }
 
  if( status < DRAW_STA_PCAZUKE_MAX ){
    if( work->set_anm_status != status ){
		  work->set_anm_status = status;
		  GFL_BBDACT_SetAnimeIdx( actSys, work->actID, work->set_anm_status );
		  GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, 0 );
	  }
  }
  
	MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  
	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
  
	{
		BOOL flag = TRUE;
		if( MMDL_CheckDrawPause(mmdl) == TRUE ){
			flag = FALSE;
		}
		GFL_BBDACT_SetAnimeEnable( actSys, work->actID, flag );
    
    flag = TRUE; 
    if( MMDL_CheckStatusBitVanish(mmdl) == TRUE ){
      flag = FALSE;
    }
    GFL_BBDACT_SetDrawEnable( actSys, work->actID, flag );
	}
}

//--------------------------------------------------------------
/// 描画処理　自機アイテムゲット　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_PCAzukeHero =
{
  DrawHero_Init,
	DrawPCAzukeHero_Draw,
	DrawHero_Delete,
	DrawHero_Delete,	//退避
	DrawHero_Init,    //本当は復帰
	DrawHero_GetBlActID,
};

//======================================================================
//  ビルボード　１パターンアニメループ
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　１パターンアニメ　描画
 * @param	mmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_DrawOnePatternLoop( MMDL *mmdl )
{
	VecFx32 pos;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
  
	MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  
	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
  
	{
		BOOL flag = TRUE;
		if( MMDL_CheckDrawPause(mmdl) == TRUE ){
			flag = FALSE;
		}
		GFL_BBDACT_SetAnimeEnable( actSys, work->actID, flag );
    
    flag = TRUE; 
    if( MMDL_CheckStatusBitVanish(mmdl) == TRUE ){
      flag = FALSE;
    }
    GFL_BBDACT_SetDrawEnable( actSys, work->actID, flag );
	}
}

//--------------------------------------------------------------
/// 描画処理 ビルボード　１パターンアニメ　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlActOnePatternLoop =
{
	DrawBlAct_Init,
	DrawBlAct_DrawOnePatternLoop,
	DrawBlAct_Delete,
	DrawBlAct_Delete,	//本当は退避
	DrawBlAct_Init,		//本当は復帰
	DrawBlAct_GetBlActID,
};
