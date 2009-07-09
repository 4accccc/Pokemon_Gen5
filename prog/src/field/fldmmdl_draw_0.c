//======================================================================
/**
 *
 * @file	fldmmdl_draw_0.c
 * @brief	フィールド動作モデル 描画処理その０
 * @author	kagaya
 * @data	05.07.13
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
///	DRAW_BLACT_WORK
//--------------------------------------------------------------
typedef struct
{
	GFL_BBDACT_ACTUNIT_ID actID;
	u8 set_anm_dir;
	u8 set_anm_status;
}DRAW_BLACT_WORK;

//======================================================================
//	proto
//======================================================================

//======================================================================
//	描画処理　描画無し
//======================================================================
//--------------------------------------------------------------
/**
 * 描画無し　初期化
 * @param	fmmdl		MMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Init( MMDL * fmmdl )
{
	MMDL_SetStatusBitVanish( fmmdl, TRUE );
	MMDL_OnStatusBit( fmmdl, MMDL_STABIT_SHADOW_VANISH );
}

//--------------------------------------------------------------
/**
 * 描画無し　描画
 * @param	fmmdl		MMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Draw( MMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画無し　削除
 * @param	fmmdl	MMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Delete( MMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画無し　退避
 * @param	fmmdl		MMDL * 
 * @retval	int			TRUE=初期化成功
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Push( MMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画無し　復帰
 * 退避した情報を元に再描画。
 * @param	fmmdl		MMDL * 
 * @retval	int			TRUE=初期化成功
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Pop( MMDL * fmmdl )
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
 * @param	fmmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawHero_Init( MMDL *fmmdl )
{
  u16 code;
	DRAW_BLACT_WORK *work;
  
	work = MMDL_InitDrawProcWork( fmmdl, sizeof(DRAW_BLACT_WORK) );
	work->set_anm_dir = DIR_NOT;
  
  code = MMDL_GetOBJCode( fmmdl );
  
	if( MMDL_BLACTCONT_AddActor(fmmdl,code,&work->actID) == TRUE ){
    MMDL_CallDrawProc( fmmdl );
  }
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機専用　削除
 * @param	fmmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawHero_Delete( MMDL *fmmdl )
{
	DRAW_BLACT_WORK *work;
	work = MMDL_GetDrawProcWork( fmmdl );
	MMDL_BLACTCONT_DeleteActor( fmmdl, work->actID );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機専用　描画
 * @param	fmmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawHero_Draw( MMDL *fmmdl )
{
	VecFx32 pos;
	BOOL chg_dir = FALSE;
	u16 dir,anm_id,status;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( fmmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(fmmdl) );
	
	dir = MMDL_GetDirDisp( fmmdl );
	status = MMDL_GetDrawStatus( fmmdl );
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
	
	MMDL_GetDrawVectorPos( fmmdl, &pos );

	pos.y += MMDL_BBD_OFFS_POS_Y;
  pos.z += MMDL_BBD_OFFS_POS_Z;
	
	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
	{
		BOOL flag = TRUE;
		if( chg_dir == FALSE && MMDL_CheckDrawPause(fmmdl) == TRUE ){
			flag = FALSE;
		}
		GFL_BBDACT_SetAnimeEnable( actSys, work->actID, flag );
    
    flag = TRUE; 
    
    if( MMDL_CheckStatusBitVanish(fmmdl) == TRUE ){
      flag = FALSE;
    }
    
    GFL_BBDACT_SetDrawEnable( actSys, work->actID, flag );
	}
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機専用　取得。
 * ビルボードアクターIDを返す。
 * @param	fmmdl	MMDL
 * @param	state	特に無し
 * @retval	u32	GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static u32 DrawHero_GetBlActID( MMDL *fmmdl, u32 state )
{
	DRAW_BLACT_WORK *work;
	work = MMDL_GetDrawProcWork( fmmdl );
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
 * @param	fmmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawCycleHero_Draw( MMDL *fmmdl )
{
	VecFx32 pos;
	BOOL chg_dir = FALSE;
	u16 dir,anm_id,status;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( fmmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(fmmdl) );
	
	dir = MMDL_GetDirDisp( fmmdl );
	status = MMDL_GetDrawStatus( fmmdl );
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
	
	MMDL_GetDrawVectorPos( fmmdl, &pos );
  
	pos.y += MMDL_BBD_OFFS_POS_Y;
  pos.z += MMDL_BBD_OFFS_POS_Z;
	
	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
	{
		BOOL flag = TRUE;
		if( chg_dir == FALSE && MMDL_CheckDrawPause(fmmdl) == TRUE ){
			flag = FALSE;
		}
		GFL_BBDACT_SetAnimeEnable( actSys, work->actID, flag );
    
    flag = TRUE; 
    
    if( MMDL_CheckStatusBitVanish(fmmdl) == TRUE ){
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
 * @param	fmmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawSwimHero_Draw( MMDL *fmmdl )
{
	VecFx32 pos;
	BOOL chg_dir = FALSE;
	u16 dir,anm_id,status;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( fmmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(fmmdl) );
	
	dir = MMDL_GetDirDisp( fmmdl );
	status = MMDL_GetDrawStatus( fmmdl );
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
	
	MMDL_GetDrawVectorPos( fmmdl, &pos );
  
	pos.y += MMDL_BBD_OFFS_POS_Y;
  pos.z += MMDL_BBD_OFFS_POS_Z;
	
	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );

	{
		BOOL flag = TRUE;
		if( chg_dir == FALSE && MMDL_CheckDrawPause(fmmdl) == TRUE ){
			flag = FALSE;
		}
		GFL_BBDACT_SetAnimeEnable( actSys, work->actID, flag );
    
    flag = TRUE; 
    
    if( MMDL_CheckStatusBitVanish(fmmdl) == TRUE ){
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
 * @param	fmmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_Init( MMDL *fmmdl )
{
  u16 code;
	DRAW_BLACT_WORK *work;
  
	work = MMDL_InitDrawProcWork( fmmdl, sizeof(DRAW_BLACT_WORK) );
	work->set_anm_dir = DIR_NOT;
  
  code = MMDL_GetOBJCode( fmmdl );

	if( MMDL_BLACTCONT_AddActor(fmmdl,code,&work->actID) == TRUE ){
    MMDL_CallDrawProc( fmmdl );
  }
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　汎用　削除
 * @param	fmmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_Delete( MMDL *fmmdl )
{
	DRAW_BLACT_WORK *work;
	work = MMDL_GetDrawProcWork( fmmdl );
	MMDL_BLACTCONT_DeleteActor( fmmdl, work->actID );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　汎用　描画
 * @param	fmmdl	MMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_Draw( MMDL *fmmdl )
{
	VecFx32 pos;
	BOOL chg_dir = FALSE;
	u16 dir,anm_id,status;
	DRAW_BLACT_WORK *work;
	GFL_BBDACT_SYS *actSys;
	
	work = MMDL_GetDrawProcWork( fmmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){
    return;
  }

	actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(fmmdl) );
	
	dir = MMDL_GetDirDisp( fmmdl );
	status = MMDL_GetDrawStatus( fmmdl );
	GF_ASSERT( status < DRAW_STA_MAX );
	anm_id = status * DIR_MAX4;
	anm_id += dir;
	
	if( work->set_anm_dir != dir ){ //方向更新
		work->set_anm_dir = dir;
		work->set_anm_status = status;
		GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
		chg_dir = TRUE;
	}else if( work->set_anm_status != status ){ //ステータス更新
		u16 frame = 0;
		switch( work->set_anm_status ){
		case DRAW_STA_WALK_32F:
		case DRAW_STA_WALK_16F:
		case DRAW_STA_WALK_8F:
		case DRAW_STA_WALK_4F:
		case DRAW_STA_WALK_2F:
			if( GFL_BBDACT_GetAnimeFrmIdx(actSys,work->actID) <= 2 ){
				frame = 2;
			}
			break;
		}
		
		GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
		GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, frame );
	}
	
	MMDL_GetDrawVectorPos( fmmdl, &pos );
	
  pos.y += MMDL_BBD_OFFS_POS_Y;
  pos.z += MMDL_BBD_OFFS_POS_Z;
	
	GFL_BBD_SetObjectTrans(
		GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );

	{
		BOOL flag = TRUE;
		if( chg_dir == FALSE && MMDL_CheckDrawPause(fmmdl) == TRUE ){
			flag = FALSE;
		}
		GFL_BBDACT_SetAnimeEnable( actSys, work->actID, flag );
    
    flag = TRUE; 
    
    if( MMDL_CheckStatusBitVanish(fmmdl) == TRUE ){
      flag = FALSE;
    }
    
    GFL_BBDACT_SetDrawEnable( actSys, work->actID, flag );
	}
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　汎用　取得。
 * ビルボードアクターIDを返す。
 * @param	fmmdl	MMDL
 * @param	state	特に無し
 * @retval	u32	GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static u32 DrawBlAct_GetBlActID( MMDL *fmmdl, u32 state )
{
	DRAW_BLACT_WORK *work;
	work = MMDL_GetDrawProcWork( fmmdl );
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
