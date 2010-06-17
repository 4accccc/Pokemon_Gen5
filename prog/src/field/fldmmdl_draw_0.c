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

#include "fieldmap.h" //FIELDMAP_GetBbdActYOffs
#include "field/zonedata.h"

//======================================================================
//  define
//======================================================================
//--------------------------------------------------------------
/// ビルボード共通オフセット表示座標
//--------------------------------------------------------------
//#define MMDL_BBD_OFFS_POS_Y (-FX32_ONE*2)  //(FX32_ONE*4)
#define MMDL_BBD_OFFS_POS_Z (FX32_ONE*4)  //(-FX32_ONE*8)

//--------------------------------------------------------------
/// 連れ歩きポケモン表示オフセット
//--------------------------------------------------------------

/* 
 * 連れ歩き自動上下アニメ差分。
 * 金銀では2.0fだったが1.5fなのはHGSS時の見た目にあわせて、金銀時の数値からWBカメラに合わせて調整した結果
 */
#define MMDL_POKE_OFS_UPDOWN_ANM  (FX32_CONST(1.5))

/*
 * ハイリンクの森で、X座標で2グリッド以内の位置にいるポケモンが重なった際に
 * Zバッファによるフリップを起こさないよう、力技だがオブジェの(グリッドX%3)で出した値で
 * Z軸方向に描画オフセットをかけている。
 *
 * 数値は森のカメラによって、ポケモン/自機とがフリップしないオフセット幅に調整した。
 * カメラが変わるとオフセットの見え方も変化し、フリップする可能性があるので、
 * カメラが変わった際は、数値も調整すること
 *
 * 配置イメージ
 * |0|1|2|0|1|2|
 * |~|-|_|~|-|_|
 *
 * DrawOffsetのみずらすと、Z+に対してオフセットがかかるグリッドに移動した際に
 * 足元が浮いて見えてしまうため、field/fldeff_shadow.c shadowTask_Draw()内の
 * 影描画処理で、連れ歩きポケモンはDrawOffsetのX/Z座標も加味した位置に影を出す調整をいれている。
 *
 * 計算で出した数値ではないです、ごめんなさい
 */
#define MMDL_POKE_OFS_SYMBOL_ALL_Z  (FX32_CONST(2.1))

/*
 * ハイリンクの森で、自機とポケモンがフリップを起こさないよう
 * 64x64ポケモンに一律かけているz軸描画オフセット。
 *
 * 32x32ポケモンは、そのままのオフセットで自機とフリップしないので
 * 64x64サイズのみに絞っている。
 *
 * カメラが変わるとオフセットの見え方も変化するので注意。
 * 10.06.14現在 MMDL_POKE_OFS_SYMBOL_ALL_Z と同じ値だが、同じ数値であることに意味はない。
 * ALL_Zとは目的が異なるので、別定義にしている。
 */
#define MMDL_POKE_OFS_SYMBOL_64x64_Z		(-FX32_CONST(2.1))

/*
 * 32x32ポケモンが左右向きの際にかけている描画オフセット値。
 * 連れ歩き絵は金銀ルールで描かれているので、そのまま描画すると左右にちょっとずれてみえる。
 * それをWBではできるだけ、見た目がグリッド中央に来るように描画オフセットをかけている。
 *
 * カメラが変わるとオフセットの見え方も変化するので注意
 */
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
  u8 fishing_hero_offs_type; //釣り自機専用
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

static void blact_SetCommonOffsPos( MMDL *mmdl, VecFx32 *pos );
static u16 blact_GetDrawDir( MMDL *mmdl );
static void blact_UpdatePauseVanish(
    MMDL *mmdl, GFL_BBDACT_SYS *actSys, u16 actID,
    BOOL start_anm, BOOL anm_pause_only );

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
//  DRAW_BLACT_WORK 共通
//======================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  DRAW_BLACT_WORK アニメーションフレームの取得
 *
 *	@param	mmdl        モデルワーク
 *	@parma  outIdx      アニメーションインデックス
 *	@parma  outFrmIdx   アニメーションフレームインデックス
 *
 */
//-----------------------------------------------------------------------------
void MMDL_DrawBlactWork_GetAnimeFrame(
    MMDL *mmdl, u16* outIdx, u16* outFrmIdx )
{
  DRAW_BLACT_WORK *work;
  GFL_BBDACT_SYS *actSys;
  const OBJCODE_PARAM *prm = MMDL_GetOBJCodeParam( mmdl );
  
  if( prm->draw_type == MMDL_DRAWTYPE_BLACT ){
    work = MMDL_GetDrawProcWork( mmdl );
    
    if( work->actID != MMDL_BLACTID_NULL ){
      actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
      *outIdx     = GFL_BBDACT_GetAnimeIdx( actSys,work->actID );
      *outFrmIdx  = GFL_BBDACT_GetAnimeFrmIdx( actSys, work->actID );
    }else{
      *outIdx     = 0;
      *outFrmIdx  = 0;
    }
  }else{
    GF_ASSERT( prm->draw_type == MMDL_DRAWTYPE_BLACT );
    *outIdx     = 0;
    *outFrmIdx  = 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  DRAW_BLACT_WORK アニメーションフレームの設定
 *
 *	@param	mmdl      モデルワーク
 *	@param  Idx       アニメーションインデックス
 *	@param	FrmIdx    フレームインデックス
 */
//-----------------------------------------------------------------------------
extern void MMDL_DrawBlactWork_SetAnimeFrame( MMDL *mmdl, u16 Idx, u16 FrmIdx )
{
  DRAW_BLACT_WORK *work;
  const OBJCODE_PARAM *prm = MMDL_GetOBJCodeParam( mmdl );
  GFL_BBDACT_SYS *actSys;

  if(prm->draw_type == MMDL_DRAWTYPE_BLACT){
    work = MMDL_GetDrawProcWork( mmdl );
    if( work->actID != MMDL_BLACTID_NULL ){
      actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
      GFL_BBDACT_SetAnimeIdx( actSys, work->actID, Idx );
      GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, FrmIdx );
    }
  }
}



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
  DRAW_BLACT_WORK *work;
  
  work = MMDL_InitDrawProcWork( mmdl, sizeof(DRAW_BLACT_WORK) );
  comManAnmCtrl_Init( &work->anmcnt );
  MMDL_BLACTCONT_AddActor( mmdl, &work->actID );
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
  blact_SetCommonOffsPos( mmdl, &pos );
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
  GF_ASSERT( status < DRAW_STA_HERO_MAX );

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
    case DRAW_STA_HERO_DASH_4F:
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
  blact_SetCommonOffsPos( mmdl, &pos );
  
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
  
  blact_UpdatePauseVanish( mmdl, actSys, work->actID, init_flag, FALSE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( mmdl, &pos );
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
  DRAW_BLACT_WORK *work;
  
  work = MMDL_InitDrawProcWork( mmdl, sizeof(DRAW_BLACT_WORK) );
  
  comManAnmCtrl_Init( &work->anmcnt );
  MMDL_BLACTCONT_AddActor( mmdl, &work->actID );
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
  
  actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
  comManAnmCtrl_Update( &work->anmcnt, mmdl, actSys, work->actID );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( mmdl, &pos );
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
  BOOL anm_pause_only = FALSE;
  u16 dir,anm_id,code;
  DRAW_BLACT_WORK *work;
  GFL_BBDACT_SYS *actSys;
  
  work = MMDL_GetDrawProcWork( mmdl );
  
  if( work->actID == MMDL_BLACTID_NULL ){
    return;
  }
  
  actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
  dir = blact_GetDrawDir( mmdl );
  code = MMDL_GetOBJCode( mmdl );
  
  if( code == FLAG ){
    anm_pause_only = TRUE;
  }
  
  {
    const OBJCODE_PARAM *prm = MMDL_GetOBJCodeParam( mmdl );
    
    if( prm->draw_proc_no == MMDL_DRAWPROCNO_BLACTALWAYSANIME_32 ){
      anm_id = DRAW_STA_WALK_32F * DIR_MAX4;
    }else if( prm->draw_proc_no == MMDL_DRAWPROCNO_BLACTALWAYSANIME_16 ){
      anm_id = DRAW_STA_WALK_16F * DIR_MAX4;
    }else{ //4frame
      anm_id = DRAW_STA_WALK_4F * DIR_MAX4;
    }
  }
  
  anm_id += dir;
  
  if( work->anmcnt.set_anm_dir != dir ){ //方向更新
    work->anmcnt.set_anm_dir = dir;
    GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
    init_flag = TRUE;
  }
  
  blact_UpdatePauseVanish(
      mmdl, actSys, work->actID, init_flag, anm_pause_only );

  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( mmdl, &pos );
  GFL_BBD_SetObjectTrans(
    GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　基本パターン+演技専用
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_DrawAct( MMDL *mmdl )
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
    
    if( status >= DRAW_STA_ACT0 ){
      u16 init_flag = FALSE;
      u16 dir = blact_GetDrawDir( mmdl );
      COMMAN_ANMCTRL_WORK *anmcnt = &work->anmcnt;
      
      if( status != anmcnt->set_anm_status ){
        u16 anm_idx = DRAW_STA_DIR4_MAX + (status - DRAW_STA_ACT0);
        
        init_flag = TRUE;
        anmcnt->set_anm_dir = dir;
        anmcnt->set_anm_status = status;
        anmcnt->next_walk_frmidx = 0;
        D_MMDL_DPrintf( "MMDL 演技アニメその0 IDX = %d\n", anm_idx );
        GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_idx );
      }
      
      blact_UpdatePauseVanish(
          mmdl, actSys, work->actID, init_flag, FALSE );
    }else{
      comManAnmCtrl_Update( &work->anmcnt, mmdl, actSys, work->actID );
    }
  }
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( mmdl, &pos );
  GFL_BBD_SetObjectTrans(
    GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　基本パターン+演技専用、演技中はポーズ無視
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawBlAct_DrawActNonePause( MMDL *mmdl )
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
    
    if( status >= DRAW_STA_ACT0 ){
      u16 init_flag = FALSE;
      u16 dir = blact_GetDrawDir( mmdl );
      COMMAN_ANMCTRL_WORK *anmcnt = &work->anmcnt;
      
      if( status != anmcnt->set_anm_status ){
        u16 anm_idx = DRAW_STA_DIR4_MAX + (status - DRAW_STA_ACT0);
        
        anmcnt->set_anm_dir = dir;
        anmcnt->set_anm_status = status;
        anmcnt->next_walk_frmidx = 0;
        D_MMDL_DPrintf( "MMDL 演技アニメその１ IDX = %d\n", anm_idx );
        GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_idx );
      }
      
      init_flag = TRUE; //演技中はポーズ無視
      blact_UpdatePauseVanish(
          mmdl, actSys, work->actID, init_flag, FALSE );
    }else{
      comManAnmCtrl_Update( &work->anmcnt, mmdl, actSys, work->actID );
    }
  }
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( mmdl, &pos );
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
//  描画処理　ビルボード　汎用+演技　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlActAct =
{
  DrawBlAct_Init,
  DrawBlAct_DrawAct,
  DrawBlAct_Delete,
  DrawBlAct_Delete,  //本当は退避
  DrawBlAct_Init,    //本当は復帰
  DrawBlAct_GetBlActID,
};

//--------------------------------------------------------------
//  描画処理　ビルボード　汎用+演技　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlActActNonePause =
{
  DrawBlAct_Init,
  DrawBlAct_DrawActNonePause,
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

  blact_UpdatePauseVanish( mmdl, actSys, work->actID, FALSE, FALSE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( mmdl, &pos );
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
  
  blact_UpdatePauseVanish( mmdl, actSys, work->actID, FALSE, FALSE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( mmdl, &pos );
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
 * 釣り自機のオフセットタイプを指定
 * @param mmdl MMDL*
 * @param flag TRUE=従来の定数オフセット　FALSE=カメラから補正
 * @retval nothing
 *
 * BTS 社内バグNo.1568
 * なみのり中にNPCの１つ上で釣りをすると、
 * 自機の足がNPCの頭よりも上に表示されます。
 * 対処の為の処理。
 */
//--------------------------------------------------------------
void MMDL_DrawFishingHero_SetOffsetType( MMDL *mmdl, BOOL flag )
{
  DRAW_BLACT_WORK *work = MMDL_GetDrawProcWork( mmdl );
  work->fishing_hero_offs_type = flag;
}

//--------------------------------------------------------------
//動作モデル位置からのオフセットベクトルに、
//カメラの逆行列をかけて、ビルボードに対して正確な位置オフセットを算出する
//--------------------------------------------------------------
static void getBillboardOffset(
    const GFL_G3D_CAMERA * g3Dcamera, const VecFx32 * org_ofs, VecFx32 * bbd_ofs )
{
  VecFx32		camPos, camUp, target;
  MtxFx43		mtx43;
  MtxFx33 mtx33; 

  GFL_G3D_CAMERA_GetPos( g3Dcamera, &camPos );
  GFL_G3D_CAMERA_GetCamUp( g3Dcamera, &camUp );
  GFL_G3D_CAMERA_GetTarget( g3Dcamera, &target );

  MTX_LookAt( &camPos, &camUp, &target, &mtx43 ); //カメラ行列取得
  MTX_Copy43To33( &mtx43, &mtx33 );               //並行移動成分はいらないので3x3行列にコピー
  MTX_Inverse33( &mtx33, &mtx33 );			          //カメラ逆行列生成

  //カメラ逆行列で回転させる＝＝カメラ行列で回転させても正位置になるはずのベクトル
  MTX_MultVec33( org_ofs, &mtx33, bbd_ofs );
}

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
  BOOL init_flag = FALSE;
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
    init_flag = TRUE;
    work->anmcnt.set_anm_dir = dir;
    work->anmcnt.set_anm_status = status;
    GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_id );
    GFL_BBDACT_SetAnimeFrmIdx( actSys, work->actID, 0 );
  }
  
  blact_UpdatePauseVanish( mmdl, actSys, work->actID, init_flag, TRUE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( mmdl, &pos );
  
  {
    VecFx32 offs;
    drawFishingHero_SetOffset( mmdl,
        GFL_BBDACT_GetAnimeIdx(actSys,work->actID),
        GFL_BBDACT_GetAnimeFrmIdx(actSys,work->actID), &offs );
    pos.x += offs.x;
    pos.y += offs.y;
    pos.z += offs.z;
  }

  /* BTS 社内バグNo.1568
   * なみのり中にNPCの１つ上で釣りをすると、
   * 自機の足がNPCの頭よりも上に表示されます。
   * 対処の為の処理。
   */
  //64x64補正：カメラ位置からオフセットを計算する
  if( work->fishing_hero_offs_type == FALSE )
  {
    VecFx32 offs = { 0, -FX32_CONST(13), 0 };
    MMDLSYS * mmdlsys = MMDL_GetMMdlSys( mmdl );
    FIELDMAP_WORK * fieldmap = MMDLSYS_GetFieldMapWork( mmdlsys );
    FIELD_CAMERA * fld_cam = FIELDMAP_GetFieldCamera( fieldmap );
    const GFL_G3D_CAMERA * g3Dcamera = FIELD_CAMERA_GetCameraPtr( fld_cam );
    getBillboardOffset( g3Dcamera, &offs, &offs );
    VEC_Add( &pos, &offs, &pos );
  }
  else //従来補正
  {
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
  
  blact_UpdatePauseVanish( mmdl, actSys, work->actID, FALSE, FALSE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( mmdl, &pos );
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
  
  blact_UpdatePauseVanish( mmdl, actSys, work->actID, FALSE, FALSE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( mmdl, &pos );
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
  blact_SetCommonOffsPos( mmdl, &pos );
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
  DRAW_BLACT_POKE_WORK *work;
  
  work = MMDL_InitDrawProcWork( mmdl, sizeof(DRAW_BLACT_WORK) );
  work->set_anm_dir = DIR_NOT;
  MMDL_BLACTCONT_AddActor( mmdl, &work->actID );
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
      work->offs_y = 0;
      GFL_BBDACT_SetAnimeIdx(
          actSys, work->actID, work->set_anm_dir );
      { //描画オフセットもクリア
        VecFx32 offs = {0,0,0};
        MMDL_GetVectorDrawOffsetPos( mmdl, &offs );
      }
    }
    
    TsurePoke_SetAnmAndOffset( mmdl, work, dir );
    
    MMDL_GetDrawVectorPos( mmdl, &pos );
    blact_SetCommonOffsPos( mmdl, &pos );

    GFL_BBD_SetObjectTrans(
      GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
    
    blact_UpdatePauseVanish( mmdl, actSys, work->actID, init_flag, FALSE );
  }
}

static void TsurePoke_SetAnmAndOffset( MMDL* mmdl, DRAW_BLACT_POKE_WORK* work, u8 dir )
{
  VecFx32 vec,ofs;
  BOOL pause_f;
  u16 zone_id;
  s16 gx;
  const OBJCODE_PARAM* obj_prm;
#ifdef DEBUG_ONLY_FOR_iwasawa
#if 0
  static float test_z = 2.1, test_diff = 0.1;
  static BOOL updown_f = 0;
  static BOOL cont_f = 0;
#endif
#endif

  obj_prm = MMDL_GetOBJCodeParam( mmdl );
  
  VEC_Set(&vec,0,0,0);

  pause_f = MMDL_CheckDrawPause( mmdl );
  MMDL_GetVectorDrawOffsetPos( mmdl, &ofs );
  gx = MMDL_GetGridPosX( mmdl );
  zone_id = MMDL_GetZoneID( mmdl );

#ifdef DEBUG_ONLY_FOR_iwasawa
#if 0
  if(work->actID == 1){
    int key = GFL_UI_KEY_GetTrg();
    int cont = GFL_UI_KEY_GetCont();
    int flag = 0;
    if( cont_f ){
      key = cont;
    }
    if( key & PAD_BUTTON_R){
      test_z += test_diff;
      flag = 1;
    }else if( key & PAD_BUTTON_L){
      test_z -= test_diff;
      flag = 1;
    }
    key = GFL_UI_KEY_GetTrg();
    if( key & PAD_BUTTON_START ){
      test_z = 0;
      flag = 1;
    }else if( key & PAD_BUTTON_SELECT){
      test_z = 2.0;
      flag = 1;
    }
    if( cont & PAD_BUTTON_B ){
      if(key & PAD_KEY_UP){
        test_diff *= 10;
        flag = 1;
      }else if(key & PAD_KEY_DOWN){
        test_diff /= 10;
        flag = 1;
      }
    }
    if( key & PAD_BUTTON_B ){
      updown_f ^= 1;
    }
    if( key & PAD_BUTTON_Y){
      cont_f ^= 1;
    }
    if(flag){
      OS_Printf("test_z = %f Act = %d diff = %f\n",test_z,work->actID,test_diff);
    }
  }
#endif
#endif
  if( pause_f ||
      (ofs.y > MMDL_POKE_OFS_UPDOWN_ANM && obj_prm->draw_proc_no != MMDL_DRAWPROCNO_TPOKE_FLY)){
    /*
     * ・ポーズ状態の時はYは維持
     * ・また、ジャンプアニメに対応するため
     * 　YオフセットがMMDL_POKE_OFS_UPDOWN_ANM以上で飛んでいるポケモンでない場合もYを維持する
     */
    vec.y = ofs.y;
  }else if( TsurePoke_CheckUpDown( work, dir, obj_prm ) == FALSE ){
    //上下アニメ用オフセット
    vec.y += MMDL_POKE_OFS_UPDOWN_ANM;
  }

  /*
   * ハイリンクの森のシンボルポケに対してのみ有効なZ軸描画オフセット処理
   * フリップを避けるため、(gridX%3)の値により、Zの描画座標をずらしている
   *
   * 処理の意図・詳細については、MMDL_POKE_OFS_SYMBOL_ALL_Z定義のコメントを参照のこと
   */
  if( ZONEDATA_IsBingo( zone_id)){
    u8 idx = gx%3;

    //シンボルポケ同士のフリップ対策
    if( idx == 0 ){
      vec.z -= MMDL_POKE_OFS_SYMBOL_ALL_Z;
    }else if( idx == 2){
      vec.z += MMDL_POKE_OFS_SYMBOL_ALL_Z;
    }
  
    //自機とのフリップ対策。でっかいポケモンはさらに一律オフセットをかける
	  if ( obj_prm->mdl_size==MMDL_BLACT_MDLSIZE_64x64 ){
      vec.z += MMDL_POKE_OFS_SYMBOL_64x64_Z;
	  }
  }
  //向きからX/Z描画オフセットをセット
  TsurePoke_GetDrawOffsetFromDir( mmdl, dir, obj_prm, &vec );

#if 0
	if ( obj_prm->mdl_size==MMDL_BLACT_MDLSIZE_64x64 ){
    vec.z -= FX32_CONST(2.1);//FX32_CONST(test_z);
  }
  if((work->actID/4)%2 == 0){
   if(updown_f){
      vec.z += FX32_CONST(test_z);
      vec.y -= FX32_CONST(1.0);
    }
  }else{
   if(!updown_f){
      vec.z += FX32_CONST(test_z);
      vec.y -= FX32_CONST(1.0);
    }
  }
#endif
  
  MMDL_SetVectorDrawOffsetPos( mmdl, &vec );
  
  work->offs_frame++; 
}

static void TsurePoke_GetDrawOffsetFromDir( MMDL* mmdl, u8 dir, const OBJCODE_PARAM* obj_prm, VecFx32* outVec)
{
  /*
   * もともと連れ歩き絵なので、そのままだとマップグリッド中心からずれて見えるのを
   * 補正するために、方向から描画オフセットをつけている。
   *
   * 大きいポケモンについては気にならなかったので、小さいポケモンだけ
   */
	if ( obj_prm->mdl_size==MMDL_BLACT_MDLSIZE_32x32 ){
		switch(dir){
		case DIR_LEFT:
			outVec->x -= MMDL_POKE_OFS_RIGHTLEFT_S;
			break;
		case DIR_RIGHT:
			outVec->x += MMDL_POKE_OFS_RIGHTLEFT_S;
			break;
		}
	}
}

static BOOL TsurePoke_CheckUpDown( DRAW_BLACT_POKE_WORK* work, u8 dir, const OBJCODE_PARAM* obj_prm )
{
  int frame;

  frame = work->offs_frame % 20;
  work->offs_frame = frame;
  
  if( obj_prm->draw_proc_no == MMDL_DRAWPROCNO_TPOKE_FLY){
    //スピード半分
    if( dir == DIR_DOWN){
      if( (frame < 5) || ( 15 <= frame )){
        return TRUE;
      }
    }else if(frame < 10){
      return TRUE;
    }
  }else{
    if ( ((5<=frame)&&(frame<10)) || (15<=frame) ){
			return TRUE;
		}
  }
  return FALSE;
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

//======================================================================
//  ビルボード　ポケモン シン・ム
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　ポケモン　シン・ム　初期化
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawBlActShinMu_Init( MMDL *mmdl )
{
  DRAW_BLACT_WORK *work;
  
  work = MMDL_InitDrawProcWork( mmdl, sizeof(DRAW_BLACT_WORK) );
  comManAnmCtrl_Init( &work->anmcnt );
  MMDL_BLACTCONT_AddActor( mmdl, &work->actID );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　ポケモン シン・ム　削除
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawBlActShinMu_Delete( MMDL *mmdl )
{
  DRAW_BLACT_WORK *work;
  work = MMDL_GetDrawProcWork( mmdl );
  MMDL_BLACTCONT_DeleteActor( mmdl, work->actID );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　ポケモン シン・ム 描画
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawBlActShinMu_Draw( MMDL *mmdl )
{
  u16 status,code;
  VecFx32 pos;
  u16 init_flag = FALSE;
  DRAW_BLACT_WORK *work;
  GFL_BBDACT_SYS *actSys;
  
  work = MMDL_GetDrawProcWork( mmdl );

  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
  actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
  
  code = MMDL_GetOBJCode( mmdl );
  status = MMDL_GetDrawStatus( mmdl );
  
  if( code == SHIN_A || code == MU_A || code == POKE10 ){
    u16 anm_idx = DRAW_STA_SHINMU_A_ANMNO_STOP;
    
    if( status != work->anmcnt.set_anm_status ){
      switch( status ){
      case DRAW_STA_SHINMU_A_GUTARI:
        anm_idx = DRAW_STA_SHINMU_A_ANMNO_GUTARI;
        break;
      case DRAW_STA_SHINMU_A_FLY_UP:
        anm_idx = DRAW_STA_SHINMU_A_ANMNO_FLY_UP;
        break;
      case DRAW_STA_SHINMU_A_FLY:
        anm_idx = DRAW_STA_SHINMU_A_ANMNO_FLY;
        break;
      }
      
      work->anmcnt.set_anm_status = status;
      GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_idx );
      init_flag = TRUE;
    }
  }else if( code == SHIN_B || code == MU_B ){ //B
    u16 dir = blact_GetDrawDir( mmdl );
    u16 anm_idx = DRAW_STA_SHINMU_B_ANMNO_STOP_U;
    
    if( status < DRAW_STA_MAX ){ //基本形
      if( dir != work->anmcnt.set_anm_dir ||
          status != work->anmcnt.set_anm_status ){
        if( dir == DIR_UP ){
          anm_idx = DRAW_STA_SHINMU_B_ANMNO_STOP_U;
        }else if( dir == DIR_DOWN ){
          anm_idx = DRAW_STA_SHINMU_B_ANMNO_STOP_D;
        }
        
        work->anmcnt.set_anm_dir = dir;
        work->anmcnt.set_anm_status = status;
        GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_idx );
        init_flag = TRUE;
      }
    }else{
      if( status != work->anmcnt.set_anm_status ){
        switch( status ){
        case DRAW_STA_SHINMU_B_HOERU:
          anm_idx = DRAW_STA_SHINMU_B_ANMNO_HOERU;
          break;
        case DRAW_STA_SHINMU_B_TURN:
          anm_idx = DRAW_STA_SHINMU_B_ANMNO_TURN;
          break;
        }
        
        work->anmcnt.set_anm_status = status;
        GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_idx );
        init_flag = TRUE;
      }
    }
  }else{ //C
    if( status != work->anmcnt.set_anm_status ){
      u16 anm_idx = DRAW_STA_SHINMU_C_ANMNO_FLY;
      
      work->anmcnt.set_anm_status = status;
      GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_idx );
      init_flag = TRUE;
    }
  }
  
  blact_UpdatePauseVanish( //シンムは動作ポーズを無視する
      mmdl, actSys, work->actID, init_flag, TRUE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( mmdl, &pos );
  GFL_BBD_SetObjectTrans(
  GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　ポケモン シン・ム　取得。
 * ビルボードアクターIDを返す。
 * @param  mmdl  MMDL
 * @param  state  特に無し
 * @retval  u32  GFL_BBDACT_ACTUNIT_ID
 */
//--------------------------------------------------------------
static u32 DrawBlActShinMu_GetBlActID( MMDL *mmdl, u32 state )
{
  DRAW_BLACT_WORK *work;
  work = MMDL_GetDrawProcWork( mmdl );
  return( work->actID );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　ポケモン　シン・ム専用　アニメフレームNo取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL MMDL_ShinMuA_GetAnimeFrame( MMDL *mmdl, u16 *outIdx, u16 *outFrmIdx )
{
  u16 code = MMDL_GetOBJCode( mmdl );
  
  if( code == SHIN_A || code == MU_A ){
    DRAW_BLACT_WORK *work = MMDL_GetDrawProcWork( mmdl );
    
    if( work->actID != MMDL_BLACTID_NULL ){
      GFL_BBDACT_SYS *actSys =
        MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
      *outIdx = GFL_BBDACT_GetAnimeIdx( actSys, work->actID );
      *outFrmIdx = GFL_BBDACT_GetAnimeFrmIdx( actSys, work->actID );
      return( TRUE );
    }
    return( FALSE );
  }
  
  GF_ASSERT( 0 );
  *outIdx = 0;
  *outFrmIdx = 0;
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　ポケモン　シン・ム専用　アニメフレームNo取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
BOOL MMDL_ShinMuC_GetAnimeFrame( MMDL *mmdl, u16 *outIdx, u16 *outFrmIdx )
{
  u16 code = MMDL_GetOBJCode( mmdl );
  
  if( code == SHIN_C || code == MU_C ){
    DRAW_BLACT_WORK *work = MMDL_GetDrawProcWork( mmdl );
    
    if( work->actID != MMDL_BLACTID_NULL ){
      GFL_BBDACT_SYS *actSys =
        MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
      
      *outIdx = GFL_BBDACT_GetAnimeIdx( actSys, work->actID );
      *outFrmIdx = GFL_BBDACT_GetAnimeFrmIdx( actSys, work->actID );
      return( TRUE );
    }
    
    return( FALSE );
  }

  GF_ASSERT( 0 );
  *outIdx = 0;
  *outFrmIdx = 0;
  return( TRUE );
}

//--------------------------------------------------------------
//  描画処理　ビルボード　ポケモン シン・ム　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlActShinMu =
{
  DrawBlActShinMu_Init,
  DrawBlActShinMu_Draw,
  DrawBlActShinMu_Delete,
  DrawBlActShinMu_Delete,  //本当は退避
  DrawBlActShinMu_Init,    //本当は復帰
  DrawBlActShinMu_GetBlActID,
};

//======================================================================
//  ビルボード　くもの巣
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　くもの巣　描画
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawBlActSpider_Draw( MMDL *mmdl )
{
  u16 status;
  VecFx32 pos;
  u16 init_flag = FALSE;
  DRAW_BLACT_WORK *work;
  GFL_BBDACT_SYS *actSys;
  
  work = MMDL_GetDrawProcWork( mmdl );

  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
  actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
  
  status = MMDL_GetDrawStatus( mmdl );
  
  if( status != work->anmcnt.set_anm_status ){
    if( status < DRAW_STA_SPIDER_MAX ){
      u16 anm_idx = status;
      work->anmcnt.set_anm_status = status;
      GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_idx );
      
      init_flag = TRUE;
    }
  }
  
  blact_UpdatePauseVanish( mmdl, actSys, work->actID, init_flag, FALSE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( mmdl, &pos );
  GFL_BBD_SetObjectTrans(
    GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
//  描画処理　ビルボード　くもの巣　まとめ
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlActSpider =
{
  DrawBlAct_Init,
  DrawBlActSpider_Draw,
  DrawBlAct_Delete,
  DrawBlAct_Delete,  //本当は退避
  DrawBlAct_Init,    //本当は復帰
  DrawBlAct_GetBlActID,
};

//======================================================================
//  ビルボード　ポケモン メロディア
//======================================================================
//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　ポケモン メロディア　描画
 * @param  mmdl  MMDL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void DrawBlActMelodyer_Draw( MMDL *mmdl )
{
  u16 status;
  VecFx32 pos;
  u16 init_flag = FALSE;
  DRAW_BLACT_WORK *work;
  GFL_BBDACT_SYS *actSys;
  
  work = MMDL_GetDrawProcWork( mmdl );

  if( work->actID == MMDL_BLACTID_NULL ){ //未登録
    return;
  }
  
  actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
  
  status = MMDL_GetDrawStatus( mmdl );
  
  if( status != work->anmcnt.set_anm_status ){
    u16 anm_idx;
    
    switch( status ){
    case DRAW_STA_MELODYER_SPIN:
      anm_idx = DRAW_STA_MELODYER_ANMNO_SPIN;
      break;
    case DRAW_STA_MELODYER_SPIN_POSE:
      anm_idx = DRAW_STA_MELODYER_ANMNO_SPIN_POSE;
      break;
    case DRAW_STA_MELODYER_SHAKE:
      anm_idx = DRAW_STA_MELODYER_ANMNO_SHAKE;
      break;
    default:
      anm_idx = DRAW_STA_MELODYER_ANMNO_STOP;
    }
    
    work->anmcnt.set_anm_status = status;
    GFL_BBDACT_SetAnimeIdx( actSys, work->actID, anm_idx );
    init_flag = TRUE;
  }
  
  blact_UpdatePauseVanish( mmdl, actSys, work->actID, init_flag, FALSE );
  
  MMDL_GetDrawVectorPos( mmdl, &pos );
  blact_SetCommonOffsPos( mmdl, &pos );
  GFL_BBD_SetObjectTrans(
    GFL_BBDACT_GetBBDSystem(actSys), work->actID, &pos );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　ポケモン メロディア アニメ終了チェック
 * @param  mmdl  MMDL
 * @retval  BOOL TRUE=終了
 */
//--------------------------------------------------------------
BOOL MMDL_CheckDrawMelodyerAnime( MMDL *mmdl )
{
  if( MMDL_GetOBJCode(mmdl) != POKE12 ){
    GF_ASSERT( 0 );
    return( TRUE );
  }else{
    u16 actID = MMDL_CallDrawGetProc( mmdl, 0 );
    
    if( actID != MMDL_BLACTID_NULL ){
      u16 comm;
      GFL_BBDACT_SYS *actSys;
      
      actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );

      if( GFL_BBDACT_GetAnimeLastCommand(actSys,actID,&comm) == TRUE ){
        return( TRUE );
      }
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 描画処理　ビルボード　ポケモン メロディア アニメフレーム初期化
 * @param  mmdl  MMDL
 * @retval  BOOL TRUE=終了
 */
//--------------------------------------------------------------
void MMDL_SetDrawMelodyerStartAnimeFrame( MMDL *mmdl )
{
  if( MMDL_GetOBJCode(mmdl) == POKE12 ){
    u16 actID = MMDL_CallDrawGetProc( mmdl, 0 );
    
    if( actID != MMDL_BLACTID_NULL ){
      GFL_BBDACT_SYS *actSys;
      actSys = MMDL_BLACTCONT_GetBbdActSys( MMDL_GetBlActCont(mmdl) );
      GFL_BBDACT_SetAnimeFrmIdx( actSys, actID, 0 );
    }
  }
}

//--------------------------------------------------------------
/// 描画処理　ビルボード　ポケモン メロディア　描画
//--------------------------------------------------------------
const MMDL_DRAW_PROC_LIST DATA_MMDL_DRAWPROCLIST_BlActMelodyer =
{
  DrawBlAct_Init,
  DrawBlActMelodyer_Draw,
  DrawBlAct_Delete,
  DrawBlAct_Delete,  //本当は退避
  DrawBlAct_Init,    //本当は復帰
  DrawBlAct_GetBlActID,
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
/*        
        #ifdef DEBUG_ONLY_FOR_kagaya
        if( MMDL_GetOBJID(mmdl) == MMDL_ID_PLAYER ){
          OS_Printf( "自機アニメ切り替え　移動系->停止 それまでのIDX=%d\n",
              work->next_walk_frmidx ); 
        }       
        #endif
*/        
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

#if 0
  //方向初期化後、アニメが繁栄されるまでにアニメ停止バグが現状ある。
  //停止時は無理やり反映させる事で緊急対処
  //
  //100331 原因判明
  //動作モデル更新とビルボード更新が一致しない場合がある為だった。
  if( work->set_anm_status == DRAW_STA_STOP ){
    init_flag = TRUE;
  }
#endif
  
  blact_UpdatePauseVanish( mmdl, actSys, actID, init_flag, FALSE );
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
static void blact_SetCommonOffsPos( MMDL * mmdl, VecFx32 *pos )
{
  MMDLSYS *mmdlsys = MMDL_GetMMdlSys( mmdl );
  FIELDMAP_WORK * fieldmap = MMDLSYS_GetFieldMapWork( mmdlsys );
  fx32 y_offs = FIELDMAP_GetBbdActYOffs( fieldmap );
  pos->y += y_offs;
  //pos->y += MMDL_BBD_OFFS_POS_Y;

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
 * @param start_anm TRUE=初回アニメである
 * @param anm_pause_only TRUE=動作ポーズを無視する(アニメポーズのみ判定
 * @retval nothing
 */
//--------------------------------------------------------------
static void blact_UpdatePauseVanish(
    MMDL *mmdl, GFL_BBDACT_SYS *actSys, u16 actID,
    BOOL start_anm, BOOL anm_pause_only )
{
  BOOL flag = TRUE; //アニメフラグ
  BOOL update = MMDL_BLACTCONT_CheckUpdateBBD( mmdl );
  
  if( start_anm == FALSE )    //初回アニメではない
  {
    if( anm_pause_only == TRUE )  //アニメポーズのみ判定する
    {
      if( update == TRUE )    //アクター更新済みで
      {                       //アニメポーズ指定があるなら
	      if( MMDL_CheckStatusBit(mmdl,MMDL_STABIT_PAUSE_ANM) )
        {
          flag = FALSE;       //アニメ停止
        }
      }
    }
    else                      //動作ポーズも加味する
    {
      if( update == TRUE )    //アクター更新済みで
      {                       //描画ポーズ指定があるならば
        if( MMDL_CheckDrawPause(mmdl) == TRUE )
        {
          flag = FALSE;           //アニメ停止
        }
      }
    }
  }
  
  GFL_BBDACT_SetAnimeEnable( actSys, actID, flag ); //アニメ有効フラグ
  
  flag = TRUE; //表示フラグ

  if( MMDL_CheckStatusBitVanish(mmdl) == TRUE ){
    flag = FALSE; //非表示
  }
  
  GFL_BBDACT_SetDrawEnable( actSys, actID, flag ); //表示有効フラグ
}
