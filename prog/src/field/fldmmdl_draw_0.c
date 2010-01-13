//======================================================================
/**
 * @file  fldmmdl_draw_0.c
 * @brief  フィールド動作モデル 描画処理その０　ビルボード関連
 * @author  kagaya
 * @date  05.07.13
 */
//======================================================================
#include "fldmmdl.h"
#include "fldmmdl_procdraw.h"

//======================================================================
//  define
//======================================================================
//--------------------------------------------------------------
/// ビルボード共通オフセット表示座標
//--------------------------------------------------------------
#define MMDL_BBD_OFFS_POS_Y (-FX32_ONE*2)  //(FX32_ONE*4)
#define MMDL_BBD_OFFS_POS_Z (FX32_ONE*4)  //(-FX32_ONE*8)

//--------------------------------------------------------------
/// 連れ歩きポケモン表示オフセット
//--------------------------------------------------------------
#define MMDL_POKE_OFS_UPDOWN		(FX32_CONST(1))
#define MMDL_POKE_OFS_RIGHTLEFT	(FX32_CONST(10))
#define MMDL_POKE_OFS_RIGHTLEFT_S	(FX32_CONST(2))

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
/// COMMAN_ANMCTRL_WORK
//--------------------------------------------------------------
typedef struct
{
  u8 set_anm_dir; //セットしている方向
  u8 set_anm_status; //セットしたアニメ
  u8 next_walk_frmidx; //移動系で次に出すアニメインデックス
  u8 next_set_wait; //次のステータスをセットするまでのウェイト
}COMMAN_ANMCTRL_WORK;

//--------------------------------------------------------------
///  DRAW_BLACT_WORK
//--------------------------------------------------------------
typedef struct
{
  u16 actID;
  u8 padding0;
  u8 padding1;
  COMMAN_ANMCTRL_WORK anmcnt;
}DRAW_BLACT_WORK;

//--------------------------------------------------------------
///  DRAW_BLACT_POKE_WORK
//--------------------------------------------------------------
typedef struct
{
  u16 actID;
  u8 set_anm_dir;
  u8 offs_frame;
  u8 dmy;
  fx32 offs_y;
}DRAW_BLACT_POKE_WORK;

//======================================================================
//  proto
//======================================================================
static void comManAnmCtrl_Init( COMMAN_ANMCTRL_WORK *work );
static void comManAnmCtrl_Update( COMMAN_ANMCTRL_WORK *work,
    MMDL *mmdl, GFL_BBDACT_SYS *actSys, u16 actID );

static void blact_SetCommonOffsPos( VecFx32 *pos );
static u16 blact_GetDrawDir( MMDL *mmdl );
static void blact_UpdatePauseVanish(
    MMDL *mmdl, GFL_BBDACT_SYS *actSys, u16 actID, BOOL force_anm );


static void TsurePoke_SetAnmAndOffset( MMDL* mmdl, DRAW_BLACT_POKE_WORK* work, u8 dir );
static void TsurePoke_GetDrawOffsetFromDir( MMDL* mmdl, u8 dir, const OBJCODE_PARAM* obj_prm, VecFx32* outVec);
static BOOL TsurePoke_CheckUpDown( DRAW_BLACT_POKE_WORK* work, u8 dir, const OBJCODE_PARAM* obj_prm );

//======================================================================
//  描画処理　描画無し
//======================================================================
//--------------------------------------------------------------
/**
 * 描画無し　初期化
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Init( MMDL * mmdl )
{
  MMDL_SetStatusBitVanish( mmdl, TRUE );
  MMDL_OnMoveBit( mmdl, MMDL_MOVEBIT_SHADOW_VANISH );
}

//--------------------------------------------------------------
/**
 * 描画無し　描画
 * @param  mmdl    MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Draw( MMDL * mmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画無し　削除
 * @param  mmdl  MMDL * 
 * @retval  nothing
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Delete( MMDL * mmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画無し　退避
 * @param  mmdl    MMDL * 
 * @retval  int      TRUE=初期化成功
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Push( MMDL * mmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画無し　復帰
 * 退避した情報を元に再描画。
 * @param  mmdl    MMDL * 
 * @retval  int      TRUE=初期化成功
 */
//--------------------------------------------------------------
static void MMdl_DrawNon_Pop( MMDL * mmdl )
{
}

//--------------------------------------------------------------
///  描画処理　描画なし　まとめ
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
//  描画処理　ビルボード　自機専用
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機専用　初期化
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawHero_Init( MMDL *mmdl )
{
  u16 code;
  DRAW_BLACT_WORK *work;
  
  work = MMDL_InitDrawProcWork( mmdl, sizeof(DRAW_BLACT_WORK) );
  
  code = MMDL_GetOBJCode( mmdl );
  comManAnmCtrl_Init( &work->anmcnt );
  
  if( MMDL_BLACTCONT_AddActor(mmdl,code,&work->actID) == TRUE ){
    MMDL_CallDrawProc( mmdl );
  }
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機専用　削除
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawHero_Delete( MMDL *mmdl )
{
  DRAW_BLACT_WORK *work;
  work = MMDL_GetDrawProcWork( mmdl );
  MMDL_BLACTCONT_DeleteActor( mmdl, work->actID );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機専用　描画
 * @param  mmdl  MMDL
 * @retval  nothing
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
  comManAnmCtrl_Update( &work->anmcnt, mmdl, actSys, work->actID );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  GFL_BBD_SetObjectTrans(
      GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機専用　取得。
 * ビルボードアクターIDを返す。
 * @param  mmdl  MMDL
 * @param  state  特に無し
 * @retval  u32  GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static u32 DrawHero_GetBlActID( MMDL *mmdl, u32 state )
{
  DRAW_BLACT_WORK *work;
  work = MMDL_GetDrawProcWork( mmdl );
  return( work->actID );
}

//--------------------------------------------------------------
//  描画処理　ビルボード　自機　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_Hero =
{
  DrawHero_Init,
  DrawHero_Draw,
  DrawHero_Delete,
  DrawHero_Delete,  //退避
  DrawHero_Init,    //本当は復帰
  DrawHero_GetBlActID,
};

//======================================================================
//  描画処理　ビルボード　自転車自機専用
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自転車自機専用　描画
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
#if 0
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
#endif

//--------------------------------------------------------------
/// 描画処理　自転車自機　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_CycleHero =
{
  DrawHero_Init,
  DrawHero_Draw,
  DrawHero_Delete,
  DrawHero_Delete,  //退避
  DrawHero_Init,    //本当は復帰
  DrawHero_GetBlActID,
};

//======================================================================
//  描画処理　ビルボード　波乗り自機専用
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　波乗り自機専用　描画
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawSwimHero_Draw( MMDL *mmdl )
{
  VecFx32 pos;
  BOOL init_flag = FALSE;
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
  
  if( work->anmcnt.set_anm_dir != dir ){ //方向更新
    work->anmcnt.set_anm_dir = dir;
    work->anmcnt.set_anm_status = status;
    GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
    init_flag = TRUE;
  }else if( work->anmcnt.set_anm_status != status ){ //ステータス更新
    u16 frame = 0;
    GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
    GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, frame );
    work->anmcnt.set_anm_status = status;
  }
  
  blact_UpdatePauseVanish( mmdl, actSys, work->actID, init_flag );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  GFL_BBD_SetObjectTrans(
    GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/// 描画処理　波乗り自機　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_SwimHero =
{
  DrawHero_Init,
  DrawSwimHero_Draw,
  DrawHero_Delete,
  DrawHero_Delete,  //退避
  DrawHero_Init,    //本当は復帰
  DrawHero_GetBlActID,
};

//======================================================================
//  描画処理　ビルボード　汎用
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　汎用　初期化
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_Init( MMDL *mmdl )
{
  u16 code;
  DRAW_BLACT_WORK *work;
  
  work = MMDL_InitDrawProcWork( mmdl, sizeof(DRAW_BLACT_WORK) );
  
  code = MMDL_GetOBJCode( mmdl );
  comManAnmCtrl_Init( &work->anmcnt );

  if( MMDL_BLACTCONT_AddActor(mmdl,code,&work->actID) == TRUE ){
    MMDL_CallDrawProc( mmdl );
  }
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　汎用　削除
 * @param  mmdl  MMDL
 * @retval  nothing
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
 * @param  mmdl  MMDL
 * @retval  nothing
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
  comManAnmCtrl_Update( &work->anmcnt, mmdl, actSys, work->actID );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  GFL_BBD_SetObjectTrans(
    GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　汎用＋常にアニメ　描画
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_DrawAlwaysAnime( MMDL *mmdl )
{
  VecFx32 pos;
  BOOL init_flag = FALSE;
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
  
  if( work->anmcnt.set_anm_dir != dir ){ //方向更新
    work->anmcnt.set_anm_dir = dir;
    GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
    init_flag = TRUE;
  }
  
  blact_UpdatePauseVanish( mmdl, actSys, work->actID, init_flag );

  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  GFL_BBD_SetObjectTrans(
    GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　PCお姉さん
 * @param  mmdl  MMDL
 * @retval  nothing
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
      COMMAN_ANMCTRL_WORK *anmcnt = &work->anmcnt;
      
      if( status != anmcnt->set_anm_status ){
        init_flag = TRUE;
        anmcnt->set_anm_dir = dir;
        anmcnt->set_anm_status = status;
        anmcnt->next_walk_frmidx = 0;
        GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_idx );
      }

      blact_UpdatePauseVanish( mmdl, actSys, work->actID, init_flag );
    }else{
      comManAnmCtrl_Update( &work->anmcnt, mmdl, actSys, work->actID );
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
 * @param  mmdl  MMDL
 * @param  state  特に無し
 * @retval  u32  GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static u32 DrawBlAct_GetBlActID( MMDL *mmdl, u32 state )
{
  DRAW_BLACT_WORK *work;
  work = MMDL_GetDrawProcWork( mmdl );
  return( work->actID );
}

//--------------------------------------------------------------
//  描画処理　ビルボード　汎用　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlAct =
{
  DrawBlAct_Init,
  DrawBlAct_Draw,
  DrawBlAct_Delete,
  DrawBlAct_Delete,  //本当は退避
  DrawBlAct_Init,    //本当は復帰
  DrawBlAct_GetBlActID,
};

//--------------------------------------------------------------
//  描画処理　ビルボード　汎用+常にアニメ　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlActAlwaysAnime =
{
  DrawBlAct_Init,
  DrawBlAct_DrawAlwaysAnime,
  DrawBlAct_Delete,
  DrawBlAct_Delete,  //本当は退避
  DrawBlAct_Init,    //本当は復帰
  DrawBlAct_GetBlActID,
};

//--------------------------------------------------------------
//  描画処理　ビルボード　PCお姉さん　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlActPCWoman =
{
  DrawBlAct_Init,
  DrawBlAct_DrawPCWoman,
  DrawBlAct_Delete,
  DrawBlAct_Delete,  //本当は退避
  DrawBlAct_Init,    //本当は復帰
  DrawBlAct_GetBlActID,
};

//======================================================================
//  描画処理　ビルボード 自機アイテムゲット
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機アイテムゲット　描画
 * @param  mmdl  MMDL
 * @retval  nothing
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
  
  //エラーフォロー
  if( work->anmcnt.set_anm_status >= DRAW_STA_ITEMGET_MAX ){ 
    work->anmcnt.set_anm_status = DRAW_STA_ITEMGET_STOP;
    GFL_BBDACT_SetAnimeIdx( actSys,
        work->actID, work->anmcnt.set_anm_status );
    GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, 0 );
  }
 
  if( status < DRAW_STA_ITEMGET_MAX ){
    if( work->anmcnt.set_anm_status != status ){
      work->anmcnt.set_anm_status = status;
      GFL_BBDACT_SetAnimeIdx( actSys,
          work->actID, work->anmcnt.set_anm_status );
      GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, 0 );
    }
  }

  blact_UpdatePauseVanish( mmdl, actSys, work->actID, FALSE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  GFL_BBD_SetObjectTrans(
    GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/// 描画処理　自機アイテムゲット　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_ItemGetHero =
{
  DrawHero_Init,
  DrawItemGetHero_Draw,
  DrawHero_Delete,
  DrawHero_Delete,  //退避
  DrawHero_Init,    //本当は復帰
  DrawHero_GetBlActID,
};

//======================================================================
//  自機PC預け
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　自機PC預け　描画
 * @param  mmdl  MMDL
 * @retval  nothing
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
  
  //エラーフォロー
  if( work->anmcnt.set_anm_status >= DRAW_STA_PCAZUKE_MAX ){
    work->anmcnt.set_anm_status = DRAW_STA_ITEMGET_STOP;
    GFL_BBDACT_SetAnimeIdx( actSys,
        work->actID, work->anmcnt.set_anm_status );
    GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, 0 );
  }
 
  if( status < DRAW_STA_PCAZUKE_MAX ){
    if( work->anmcnt.set_anm_status != status ){
      work->anmcnt.set_anm_status = status;
      GFL_BBDACT_SetAnimeIdx( actSys,
          work->actID, work->anmcnt.set_anm_status );
      GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, 0 );
    }
  }
  
  blact_UpdatePauseVanish( mmdl, actSys, work->actID, FALSE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  GFL_BBD_SetObjectTrans(
    GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/// 描画処理　自機アイテムゲット　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_PCAzukeHero =
{
  DrawHero_Init,
  DrawPCAzukeHero_Draw,
  DrawHero_Delete,
  DrawHero_Delete,  //退避
  DrawHero_Init,    //本当は復帰
  DrawHero_GetBlActID,
};

//======================================================================
//  釣り自機
//======================================================================
//--------------------------------------------------------------
/**
 * 釣りアニメ　表示別にオフセットセット
 * @param
 * @retval
 */
//--------------------------------------------------------------
static void drawFishingHero_SetOffset(
    MMDL *mmdl, u16 anm_idx, u16 frm_idx, VecFx32 *offs )
{
  u16 code,cell;
  MMDLSYS *mmdlsys;
  const MMDL_BBDACT_ANMTBL *anmTbl;
  const GFL_BBDACT_ANM *anm;
  
  offs->x = 0;
  offs->y = 0;
  offs->z = 0;
#if 0
  code = MMDL_GetOBJCode( mmdl );
  mmdlsys = MMDL_GetMMdlSys( mmdl );
  anmTbl = MMDL_BLACTCONT_GetObjAnimeTable( mmdlsys, code );
  
  GF_ASSERT( anm_idx < anmTbl->anm_max );
  
  anm = anmTbl->pAnmTbl[anm_idx];
  cell = anm[frm_idx].anmData.celIdx;
  
  if( cell == 6 || cell == 7 ){
    offs->z = NUM_FX32( 8 );
  }else if( cell == 10 || cell == 11 ){
    offs->x = NUM_FX32( -5 );

    if( anm[frm_idx].anmData.flipS ){
      offs->x = -offs->x;
    }
  }
#endif
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　釣り自機　描画
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawFishingHero_Draw( MMDL *mmdl )
{
  u16 dir,status,anm_id;
  VecFx32 pos;
  DRAW_BLACT_WORK *work;
  GFL_BBDACT_SYS *actSys;

  work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
  actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
  
  dir = blact_GetDrawDir( mmdl );
  status = MMDL_GetDrawStatus( mmdl );
  anm_id = (status * DIR_MAX4) + dir;
  
  if( dir != work->anmcnt.set_anm_dir ||
    work->anmcnt.set_anm_status != status ){
    work->anmcnt.set_anm_dir = dir;
    work->anmcnt.set_anm_status = status;
    GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
    GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, 0 );
  }
  
  blact_UpdatePauseVanish( mmdl, actSys, work->actID, FALSE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );

  {
    VecFx32 offs;
    drawFishingHero_SetOffset( mmdl,
        GFL_BBDACT_GetAnimeIdx(actSys,work->actID),
        GFL_BBDACT_GetAnimeFrmIdx(actSys,work->actID), &offs );
    pos.x += offs.x;
    pos.y += offs.y;
    pos.z += offs.z;
  }
  
  { //64x64補正
    fx16 sizX = FX16_ONE*8-1;
    fx16 sizY = FX16_ONE*8-1;
    GFL_BBD_SetObjectSiz( GFL_BBDACT_GetBBDSystem(actSys),
        work->actID, &sizX, &sizY );
    pos.y += NUM_FX32( -12 );
    pos.z += NUM_FX32( 8 );
  }

  GFL_BBD_SetObjectTrans(
    GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/// 描画処理　釣り自機　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_FishingHero =
{
  DrawHero_Init,
  DrawFishingHero_Draw,
  DrawHero_Delete,
  DrawHero_Delete,  //退避
  DrawHero_Init,    //本当は復帰
  DrawHero_GetBlActID,
};

//======================================================================
//  ゆれ自機
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　ゆれ自機　描画
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawYureHero_Draw( MMDL *mmdl )
{
  u16 dir;
  VecFx32 pos;
  DRAW_BLACT_WORK *work;
  GFL_BBDACT_SYS *actSys;
  
  work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
  actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
  dir = blact_GetDrawDir( mmdl );
  
  if( work->anmcnt.set_anm_dir != dir ){
    u16 anm_id = dir;
    GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
    GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, 0 );
    work->anmcnt.set_anm_dir = dir;
  }
  
  blact_UpdatePauseVanish( mmdl, actSys, work->actID, FALSE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  GFL_BBD_SetObjectTrans(
    GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/**
 * ゆれ自機　アニメフレームセット
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL MMDL_DrawYureHero_SetAnimeFrame( MMDL *mmdl, u32 frame )
{
  u16 idx = 0;
  DRAW_BLACT_WORK *work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return( FALSE );
  }
  
  if( frame < 24 ){
    if( frame < 6 ){
      idx = 0;
    }else if( frame < 12 ){
      idx = 1;
    }else if( frame < 18 ){
      idx = 2;
    }else{
      idx = 3;
    }
  }else if( frame < 40 ){
    if( frame < 28 ){
      idx = 4;
    }else if( frame < 32 ){
      idx = 5;
    }else if( frame < 36 ){
      idx = 6;
    }else{
      idx = 7;
    }
  }else if( frame < 48 ){
    if( frame < 42 ){
      idx = 8;
    }else if( frame < 44 ){
      idx = 9;
    }else if( frame < 46 ){
      idx = 10;
    }else{
      idx = 11;
    }
  }else{
    idx = 12;
  }
  
  {
    u16 now;
    GFL_BBDACT_SYS *actSys;

    actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
    now = GFL_BBDACT_GetAnimeFrmIdx( actSys, work->actID );

    if( now != idx ){
      GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, 0 );
    }
  }
  
  return( TRUE );
}

//--------------------------------------------------------------
/// 描画処理　ゆれ自機　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_YureHero =
{
  DrawHero_Init,
  DrawYureHero_Draw,
  DrawHero_Delete,
  DrawHero_Delete,  //退避
  DrawHero_Init,    //本当は復帰
  DrawHero_GetBlActID,
};

//======================================================================
//  ビルボード　１パターンアニメループ
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　１パターンアニメ　描画
 * @param  mmdl  MMDL
 * @retval  nothing
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
  
  blact_UpdatePauseVanish( mmdl, actSys, work->actID, FALSE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  GFL_BBD_SetObjectTrans(
    GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/// 描画処理 ビルボード　１パターンアニメ　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlActOnePatternLoop =
{
  DrawBlAct_Init,
  DrawBlAct_DrawOnePatternLoop,
  DrawBlAct_Delete,
  DrawBlAct_Delete,  //本当は退避
  DrawBlAct_Init,    //本当は復帰
  DrawBlAct_GetBlActID,
};

//======================================================================
//  ビルボード　１パターンアニメ
//======================================================================
#if 0
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　１パターンアニメ　描画　ループなし
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_DrawOnePattern( MMDL *mmdl )
{
  VecFx32 pos;
  DRAW_BLACT_WORK *work;
  GFL_BBDACT_SYS *actSys;
  
  work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
  actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
  
#if 1  
  {
    u16 comm;

    if( GFL_BBDACT_GetAnimeLastCommand(actSys,work->actID,&comm) == TRUE ){
      int flag = TRUE;
      
      if( comm == GFL_BBDACT_ANMCOM_END ){
        GFL_BBDACT_SetAnimeEnable( actSys, work->actID, FALSE );
      }

      if( MMDL_CheckStatusBitVanish(mmdl) == TRUE ){
        flag = FALSE;
      }

      GFL_BBDACT_SetDrawEnable( actSys, work->actID, flag );
    }else{
      blact_UpdatePauseVanish( mmdl, actSys, work->actID, FALSE );
    }
  }
#endif

  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( &pos );
  GFL_BBD_SetObjectTrans(
    GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/// 描画処理 ビルボード　１パターンアニメ　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlActOnePattern =
{
  DrawBlAct_Init,
  DrawBlAct_DrawOnePattern,
  DrawBlAct_Delete,
  DrawBlAct_Delete,  //本当は退避
  DrawBlAct_Init,    //本当は復帰
  DrawBlAct_GetBlActID,
};
#endif

//======================================================================
//  ビルボード　連れ歩きポケモンアニメ
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　連れ歩きポケモン　初期化
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawTsurePoke_Init( MMDL *mmdl )
{
  u16 code;
  DRAW_BLACT_POKE_WORK *work;
  
  work = MMDL_InitDrawProcWork( mmdl, sizeof(DRAW_BLACT_WORK) );
  work->set_anm_dir = DIR_NOT;

  code = MMDL_GetOBJCode( mmdl );
  
  if( MMDL_BLACTCONT_AddActor(mmdl,code,&work->actID) == TRUE ){
    MMDL_CallDrawProc( mmdl );
  }
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　連れ歩きポケモン　削除
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawTsurePoke_Delete( MMDL *mmdl )
{
  DRAW_BLACT_POKE_WORK *work;
  work = MMDL_GetDrawProcWork( mmdl );
  MMDL_BLACTCONT_DeleteActor( mmdl, work->actID );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　連れ歩きポケモン　描画
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawTsurePoke_Draw( MMDL *mmdl )
{
  DRAW_BLACT_POKE_WORK *work;
  
  work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID != MMDL_BLACTID_NULL ){
    VecFx32 pos;
    u16 dir,init_flag;
    GFL_BBDACT_SYS *actSys;
  
    init_flag = FALSE;

    actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
    dir = MMDL_GetDirDisp( mmdl );
  
    if( dir != work->set_anm_dir ){
      init_flag = TRUE;
      work->set_anm_dir = dir;
      work->offs_frame = 0;
      GFL_BBDACT_SetAnimeIdx( actSys,work->actID, work->set_anm_dir );
    }
    TsurePoke_SetAnmAndOffset( mmdl, work, dir );
    
    MMDL_GetDrawVectorPos( mmdl, &pos );
    
    blact_SetCommonOffsPos( &pos );
    GFL_BBD_SetObjectTrans(
      GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
    
    blact_UpdatePauseVanish( mmdl, actSys, work->actID, init_flag );
  }
}

static void TsurePoke_SetAnmAndOffset( MMDL* mmdl, DRAW_BLACT_POKE_WORK* work, u8 dir )
{
  VecFx32 vec;
  BOOL pause_f, anmcmd_f;
  const OBJCODE_PARAM* obj_prm;
  u16 obj_code = MMDL_GetOBJCode( mmdl );
  obj_prm = MMDLSYS_GetOBJCodeParam( MMDL_GetMMdlSys(mmdl), obj_code );

  VEC_Set(&vec,0,0,0);

  pause_f = MMDL_CheckDrawPause( mmdl );
  anmcmd_f = MMDL_CheckMoveBitAcmd( mmdl );
  
  if( pause_f || anmcmd_f ){
    //Yオフセットのみ引き継ぐ
    MMDL_GetDrawVectorPos( mmdl, &vec );
    vec.x = vec.z = 0;
  }

  //向きから描画オフセットをセット
  TsurePoke_GetDrawOffsetFromDir( mmdl, dir, obj_prm, &vec );

  //ジャンプオフセットセット

  if( anmcmd_f ){
    work->offs_frame++; 
  }else if( !pause_f ){
    work->offs_frame++; 
    if( TsurePoke_CheckUpDown( work, dir, obj_prm )){
      vec.y -= FX32_CONST(2);
    }
  }
  MMDL_SetVectorDrawOffsetPos( mmdl, &vec );
}

static void TsurePoke_GetDrawOffsetFromDir( MMDL* mmdl, u8 dir, const OBJCODE_PARAM* obj_prm, VecFx32* outVec)
{
  //方向から描画オフセットをつける
	if ( obj_prm->mdl_size==MMDL_BLACT_MDLSIZE_64x64 ){
		switch(dir){
		case DIR_UP:
			outVec->z += MMDL_POKE_OFS_UPDOWN;
			break;
		case DIR_DOWN:
			outVec->z -= MMDL_POKE_OFS_UPDOWN;
			break;
		case DIR_LEFT:
			outVec->x += MMDL_POKE_OFS_RIGHTLEFT;
			break;
		case DIR_RIGHT:
			outVec->x -= MMDL_POKE_OFS_RIGHTLEFT;
			break;
    }
	}else{
		switch(dir){
		case DIR_LEFT:
			outVec->x += MMDL_POKE_OFS_RIGHTLEFT_S;
			break;
		case DIR_RIGHT:
			outVec->x -= MMDL_POKE_OFS_RIGHTLEFT_S;
			break;
		}
	}
}

static BOOL TsurePoke_CheckUpDown( DRAW_BLACT_POKE_WORK* work, u8 dir, const OBJCODE_PARAM* obj_prm )
{
  int frame;

  frame = work->offs_frame % 40;
  work->offs_frame = frame;
//  frame = GFL_BBDACT_GetAnimeFrmIdx( actSys, work->actID );
//  IWASAWA_Printf( "PairPokeAnmFrame = %d\n",frame );

  if( obj_prm->draw_proc_no == MMDL_DRAWPROCNO_TPOKE_FLY){
    //スピード半分
    if( dir == DIR_DOWN){
      if( (frame < 10) || ( 30 <= frame )){
        return TRUE;
      }
    }else if(frame < 20){
      return TRUE;
    }
  }else{
    if ( ((10<=frame)&&(frame<20)) || (30<=frame) ){
			return TRUE;
		}
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　連れ歩きポケモン　縦揺れ付き　描画
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawTsurePokeFly_Draw( MMDL *mmdl )
{
  DRAW_BLACT_POKE_WORK *work;
  
  work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID != MMDL_BLACTID_NULL ){
    VecFx32 pos;
    u16 dir,init_flag;
    GFL_BBDACT_SYS *actSys;
  
    init_flag = FALSE;

    actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
    dir = MMDL_GetDirDisp( mmdl );
  
    if( dir != work->set_anm_dir ){
      init_flag = TRUE;
      work->set_anm_dir = dir;
      work->offs_frame = 0;
      GFL_BBDACT_SetAnimeIdx( actSys, work->actID, work->set_anm_dir );
    }
    
    if( MMDL_CheckDrawPause(mmdl) == FALSE ){ //to GS
      work->offs_frame++;
      work->offs_frame %= 20;
      
      if( work->offs_frame >= 15 || //5-9 or 15-19
          (work->offs_frame >= 5 && work->offs_frame < 10) ){
        work->offs_y -= FX32_ONE * 2;
      }else{
        work->offs_y += FX32_ONE * 2;
      }
    }
    
    pos.x = 0;
    pos.y = work->offs_y;
    pos.z = 0;
	  MMDL_SetVectorDrawOffsetPos( mmdl, &pos );
    
    MMDL_GetDrawVectorPos( mmdl, &pos );
    blact_SetCommonOffsPos( &pos );
    GFL_BBD_SetObjectTrans(
      GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
    
    MMDL_GetDrawVectorPos( mmdl, &pos );
    blact_SetCommonOffsPos( &pos );
    blact_UpdatePauseVanish( mmdl, actSys, work->actID, init_flag );
  }
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　連れ歩きポケモン
 * ビルボードアクターIDを返す。
 * @param  mmdl  MMDL
 * @param  state  特に無し
 * @retval  u32  GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static u32 DrawTsurePoke_GetBlActID( MMDL *mmdl, u32 state )
{
  DRAW_BLACT_POKE_WORK *work;
  work = MMDL_GetDrawProcWork( mmdl );
  return( work->actID );
}

//--------------------------------------------------------------
// 描画処理　ビルボード　連れ歩きポケモン　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_TsurePoke =
{
  DrawTsurePoke_Init,
  DrawTsurePoke_Draw,
  DrawTsurePoke_Delete,
  DrawTsurePoke_Delete,  //退避
  DrawTsurePoke_Init,    //本当は復帰
  DrawTsurePoke_GetBlActID,
};

const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_TsurePokeFly =
{
  DrawTsurePoke_Init,
  DrawTsurePokeFly_Draw,
  DrawTsurePoke_Delete,
  DrawTsurePoke_Delete,  //退避
  DrawTsurePoke_Init,    //本当は復帰
  DrawTsurePoke_GetBlActID,
};

//======================================================================
//  人物系共通　ビルボードアクターアニメ処理
//======================================================================
//--------------------------------------------------------------
/**
 * 人物系共通　ビルボードアクターアニメ処理　初期化
 * @param work COMMAN_ANMCTRL_WORK*
 * @retval nothing
 */
//--------------------------------------------------------------
static void comManAnmCtrl_Init( COMMAN_ANMCTRL_WORK *work )
{
  MI_CpuClear8( work, sizeof(COMMAN_ANMCTRL_WORK) );
  work->set_anm_dir = DIR_MAX;
}

//--------------------------------------------------------------
/**
 * 人物系共通　ビルボードアクターアニメ処理　更新
 * @param work COMMAN_ANMCTRL_WORK*
 * @param mmdl MMDL*
 * @param actSys GFL_BBDACT_SYS
 * @param actID BBDACT ID
 * @retval nothing
 * @note 移動系アニメIndex数が4で、
 * Index0,1と2,3がそれぞれ別個のアニメを前提としている。
 * それ以外は対応していない。
 */
//--------------------------------------------------------------
static void comManAnmCtrl_Update( COMMAN_ANMCTRL_WORK *work,
    MMDL *mmdl, GFL_BBDACT_SYS *actSys, u16 actID )
{
  u16 init_flag = FALSE;
  u16 dir = blact_GetDrawDir( mmdl );
  u16 status = MMDL_GetDrawStatus( mmdl );
  u16 anm_idx = (status * DIR_MAX4) + dir;
  
  if( work->next_set_wait ) //セット待ち状態
  {
    if( status == work->set_anm_status ) //セット待ちでステータス同一化
    {
      work->next_set_wait = 0; //セット待ち解除
    }
  }

  if( dir != work->set_anm_dir ) //方向更新は無条件で更新
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
     
    if( status == DRAW_STA_STOP ) //セット停止タイプ
    {
      if( work->set_anm_status == DRAW_STA_STOP ) //停止アニメ中
      {
        GFL_BBDACT_SetAnimeIdx( actSys, actID, anm_idx );
        
        work->set_anm_status = status;
        work->next_set_wait = 0;
      }
      else if( work->next_set_wait == 0 ) //移動系 1frame後にセット
      {
        work->next_set_wait++;
        init_flag = FALSE;
      }
      else //移動系 1frame待ち後
      {
        u8 tbl[4] = {0,2,2,0}; //次回移動系セット時のアニメIndex
        
        work->next_walk_frmidx = GFL_BBDACT_GetAnimeFrmIdx( actSys, actID );
        
//        #ifdef DEBUG_ONLY_FOR_kagaya
        #if 0
        if( MMDL_GetOBJID(mmdl) == MMDL_ID_PLAYER ){
          OS_Printf( "自機アニメ切り替え　移動系->停止 それまでのIDX=%d\n",
              work->next_walk_frmidx ); 
        }       
        #endif
        
        if( work->next_walk_frmidx >= 4 ) //index over
        {
          work->next_walk_frmidx = 0;
        }
        
        work->next_walk_frmidx = tbl[work->next_walk_frmidx];
        
        GFL_BBDACT_SetAnimeIdx( actSys, actID, anm_idx );
        
        work->set_anm_status = status;
        work->next_set_wait = 0;
      }
    }
    else //セット歩きタイプ
    {
      if( work->set_anm_status != DRAW_STA_STOP ) //移動系アニメを行っていた
      { //次のアニメを表示
        work->next_walk_frmidx = GFL_BBDACT_GetAnimeFrmIdx( actSys, actID );

//        #ifdef DEBUG_ONLY_FOR_kagaya
        #if 0
        if( MMDL_GetOBJID(mmdl) == MMDL_ID_PLAYER ){
          OS_Printf( "自機アニメ切り替え　移動系継続 それまでのIDX=%d\n",
              work->next_walk_frmidx ); 
        }       
        #endif
        
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
      
      work->set_anm_status = status;
      work->next_set_wait = 0;
    }
  }
  
  #ifdef DEBUG_ONLY_FOR_kagaya
  {
  }
  #endif
  
  blact_UpdatePauseVanish( mmdl, actSys, actID, init_flag );
}

//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 * ビルボードOBJ 共通オフセット座標セット 32x32
 * @param pos VecFx32
 * @retval nothing
 */
//--------------------------------------------------------------
static void blact_SetCommonOffsPos( VecFx32 *pos )
{
  pos->y += MMDL_BBD_OFFS_POS_Y;
#ifdef MMDL_BBD_DRAW_OFFS_Z_USE
  pos->z += MMDL_BBD_OFFS_POS_Z;
#endif
}

//--------------------------------------------------------------
/**
 * カメラ左右の振れ角を考慮して方向取得
 * @param mmdl MMDL
 * @retval u16 方向
 */
//--------------------------------------------------------------
static u16 blact_GetDrawDir( MMDL *mmdl )
{
  const MMDLSYS *mmdlsys = MMDL_GetMMdlSys( mmdl );
  u16 angle = MMDLSYS_GetTargetCameraAngleYaw( mmdlsys );
  u16 dir = MMDL_GetDirDisp( mmdl );
  
  if( !MMDL_CheckStatusBit( mmdl, MMDL_STABIT_RAIL_MOVE ) )
  {
    dir = MMDL_TOOL_GetAngleYawToDirFour( dir, angle );
  }
  else
  {
    // レールマップでは、進行方向が関係してくるため。
    dir = MMDL_RAIL_GetAngleYawToDirFour( mmdl, angle );
  }
  
  return( dir );
}

//--------------------------------------------------------------
/**
 * ビルボードアクター共通　アニメ一時停止、非表示更新
 * @param mmdl MMDL*
 * @param actSys GFL_BBDACT_SYS
 * @param actID BBDACT ID
 * @param force_anm TRUE=アニメ強制セット
 * @retval nothing
 */
//--------------------------------------------------------------
static void blact_UpdatePauseVanish(
    MMDL *mmdl, GFL_BBDACT_SYS *actSys, u16 actID, BOOL force_anm )
{
  BOOL flag = TRUE;
  
  if( force_anm == FALSE && MMDL_CheckDrawPause(mmdl) == TRUE ){
    flag = FALSE;
  }
  
  GFL_BBDACT_SetAnimeEnable( actSys, actID, flag );
  
  flag = TRUE;

  if( MMDL_CheckStatusBitVanish(mmdl) == TRUE ){
    flag = FALSE;
  }
  
  GFL_BBDACT_SetDrawEnable( actSys, actID, flag );
}

