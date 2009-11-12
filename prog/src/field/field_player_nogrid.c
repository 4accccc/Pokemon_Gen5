//======================================================================
/**
 * @file	field_player_nogrid.c
 * @brief	ノングリッド移動　フィールドプレイヤー制御
 */
//======================================================================
#include <gflib.h>

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

#include "field_sound.h"


#include "fieldmap.h"
#include "field_g3d_mapper.h"
#include "fldmmdl.h"
#include "field_player.h"
#include "field_camera.h"
#include "fldeff_shadow.h"
#include "field_rail.h"
#include "rail_attr.h"

#include "field_player_nogrid.h"

//======================================================================
//	define
//======================================================================
#define MV_SPEED (2*FX32_ONE) ///<移動速度


static const MMDL_HEADER data_MMdlHeader =
{
	MMDL_ID_PLAYER,	///<識別ID
	HERO,	///<表示するOBJコード
	MV_RAIL_DMY,	///<動作コード
	0,	///<イベントタイプ
	0,	///<イベントフラグ
	0,	///<イベントID
	0,	///<指定方向
	0,	///<指定パラメタ 0
	0,	///<指定パラメタ 1
	0,	///<指定パラメタ 2
	MOVE_LIMIT_NOT,	///<X方向移動制限
	MOVE_LIMIT_NOT,	///<Z方向移動制限
  MMDL_HEADER_POSTYPE_RAIL,
};

//--------------------------------------------------------------
///	PLAYER_MOVE
//--------------------------------------------------------------
typedef enum
{
	PLAYER_MOVE_STOP = 0,
	PLAYER_MOVE_WALK,
	PLAYER_MOVE_TURN,
	PLAYER_MOVE_HITCH,
} PLAYER_MOVE;

//--------------------------------------------------------------
///	PLAYER_SET
//--------------------------------------------------------------
typedef enum
{
	PLAYER_SET_NON = 0,
	PLAYER_SET_STOP,
	PLAYER_SET_WALK,
	PLAYER_SET_TURN,
	PLAYER_SET_HITCH,
//  PLAYER_SET_JUMP,
} PLAYER_SET;


//======================================================================
//	struct
//======================================================================
//-------------------------------------
///	レール動作用補助ワーク
//=====================================
struct _FIELD_PLAYER_NOGRID
{
  int move_state;
  
  FIELD_PLAYER* p_player;
	FIELDMAP_WORK* p_fieldwork;
  MMDL* p_mmdl;
  
  FIELD_RAIL_WORK* p_railwork;

  u16 form_change_req;
  u16 form_change;
} ;

//======================================================================
//	proto
//======================================================================
static void nogridPC_Move_SetValue( FIELD_PLAYER *fld_player,
		FIELDMAP_WORK *fieldWork, u16 key, VecFx32 *vec );
static BOOL nogridPC_Move_CalcSetGroundMove(
		const FLDMAPPER *g3Dmapper,
		FLDMAPPER_GRIDINFODATA* gridInfoData,
		VecFx32* pos, VecFx32* vecMove, fx32 speed );


static void updateFormChange( FIELD_PLAYER_NOGRID* p_player );


//======================================================================
//	proto　for nogrid_player
//======================================================================
//通常移動
static void jikiMove_Normal(
		FIELD_PLAYER_NOGRID *p_player, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag );

static PLAYER_SET player_CheckMoveStart_Stop(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET player_CheckMoveStart_Walk(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET player_CheckMoveStart_Turn(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET player_CheckMoveStart_Hitch(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void player_SetMove_Non(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void player_SetMove_Stop(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void player_SetMove_Walk(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void player_SetMove_Turn(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void player_SetMove_Hitch(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
/*static void player_SetMove_Jump(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );//*/

//自転車移動
static void jikiMove_Cycle(
		FIELD_PLAYER_NOGRID *p_player, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag );

static PLAYER_SET playerCycle_CheckMoveStart_Stop(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET playerCycle_CheckMoveStart_Walk(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET playerCycle_CheckMoveStart_Turn(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static PLAYER_SET playerCycle_CheckMoveStart_Hitch(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );

static void playerCycle_SetMove_Non(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void playerCycle_SetMove_Stop(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void playerCycle_SetMove_Walk(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void playerCycle_SetMove_Turn(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
static void playerCycle_SetMove_Hitch(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );
/*static void playerCycle_SetMove_Jump(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag );//*/

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドノーグリッドプレイヤーワークの生成
 *
 *	@param	p_player    フィールドプレイヤー
 *	@param	heapID      ヒープID
 *
 *	@return ノーグリッド用ワーク
 */
//-----------------------------------------------------------------------------
FIELD_PLAYER_NOGRID* FIELD_PLAYER_NOGRID_Create( FIELD_PLAYER* p_player, HEAPID heapID )
{
  FIELD_PLAYER_NOGRID* p_wk;

  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_PLAYER_NOGRID) );

  // ワークに保存
  p_wk->p_player      = p_player;
  p_wk->p_fieldwork   = FIELD_PLAYER_GetFieldMapWork( p_player );
  p_wk->p_mmdl = FIELD_PLAYER_GetMMdl( p_wk->p_player );

  // 動作コードをレール動作に変更
  if( MMDL_GetMoveCode( p_wk->p_mmdl ) != MV_RAIL_DMY )
  {
    MMDL_ChangeMoveParam( p_wk->p_mmdl, &data_MMdlHeader );
  }


  // レールワークの取得
  p_wk->p_railwork = MMDL_GetRailWork( p_wk->p_mmdl );

  // 位置を初期か
  {
    RAIL_LOCATION location = {0};
    MMDL_SetRailLocation( p_wk->p_mmdl, &location );
  }


  // 状態の復帰
  {
    /*
    PLAYER_MOVE_FORM form;
    form = FIELD_PLAYER_GetMoveForm( gjiki->fld_player );
    
    switch( form ){
    case PLAYER_MOVE_FORM_SWIM:
      FIELD_PLAYER_NOGRID_SetRequest(
          gjiki, FIELD_PLAYER_NOGRID_REQBIT_SWIM );
      FIELD_PLAYER_NOGRID_UpdateRequest( gjiki );
      break;
    }
    //*/
  }

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドノーグリッドプレイヤーワーク　破棄
 *
 *	@param	p_player  フィールドプレイヤー
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_NOGRID_Delete( FIELD_PLAYER_NOGRID* p_player )
{
  GFL_HEAP_FreeMemory( p_player );
}

//----------------------------------------------------------------------------
/**
 *	@brief  動作リセット　再構築
 *
 *	@param	p_player  プレイヤー
 *	@param	cp_pos    再始動座標
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_NOGRID_Restart( FIELD_PLAYER_NOGRID* p_player, const RAIL_LOCATION* cp_pos )
{
  // 動作コードをレール動作に変更
  if( MMDL_GetMoveCode( p_player->p_mmdl ) != MV_RAIL_DMY )
  {
    MMDL_ChangeMoveParam( p_player->p_mmdl, &data_MMdlHeader );
  }


  // レールワークの取得
  p_player->p_railwork = MMDL_GetRailWork( p_player->p_mmdl );

  // 位置を初期か
  MMDL_SetRailLocation( p_player->p_mmdl, cp_pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief  動作停止
 *
 *	@param	p_player  プレイヤー
 *	@param	cp_pos    再始動座標
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_NOGRID_Stop( FIELD_PLAYER_NOGRID* p_player )
{
  p_player->p_railwork = NULL;
}


//----------------------------------------------------------------------------
/**
 *	@brief  フィールドノーグリッドプレイヤーワーク  動作
 *
 *	@param	p_player      プレイヤー
 *	@param	key_trg       キートリガ
 *	@param	key_cont      キーコント
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_NOGRID_Move( FIELD_PLAYER_NOGRID* p_player, int key_trg, int key_cont )
{
	u16 dir;
	BOOL debug_flag;
  PLAYER_MOVE_FORM form;

  GF_ASSERT( p_player->p_railwork );

  // FORM変更処理
  updateFormChange( p_player );
  
	dir = DIR_NOT;
	if( (key_cont&PAD_KEY_UP) ){
		dir = DIR_UP;
	}else if( (key_cont&PAD_KEY_DOWN) ){
		dir = DIR_DOWN;
	}else if( (key_cont&PAD_KEY_LEFT) ){
		dir = DIR_LEFT;
	}else if( (key_cont&PAD_KEY_RIGHT) ){
		dir = DIR_RIGHT;
	}
	
	debug_flag = FALSE;
#ifdef PM_DEBUG
	if( key_cont & PAD_BUTTON_R ){
		debug_flag = TRUE;
	}
#endif

#if 0
  if( key_trg & PAD_BUTTON_SELECT ){
    VecFx32 pos;
    FLDEFF_CTRL *fectrl = FIELDMAP_GetFldEffCtrl( gjiki->fieldWork );
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( gjiki->fld_player );
    u16 dir = MMDL_GetDirDisp( mmdl );
    MMDL_GetVectorPos( mmdl, &pos );
    MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
    
    gjiki->fldeff_joint = FLDEFF_NAMIPOKE_SetMMdl(
        fectrl, dir, &pos, mmdl, FALSE );
  }
#endif
  
//  FIELD_PLAYER_NOGRID_UpdateRequest( gjiki );
  
  form = FIELD_PLAYER_GetMoveForm( p_player->p_player );
  switch( form ){
  case PLAYER_MOVE_FORM_NORMAL:
    jikiMove_Normal( p_player, key_trg, key_cont, dir, debug_flag );
	  break;
  case PLAYER_MOVE_FORM_CYCLE:
    jikiMove_Cycle( p_player, key_trg, key_cont, dir, debug_flag );
	  break;
  case PLAYER_MOVE_FORM_SWIM:
    GF_ASSERT( form == PLAYER_MOVE_FORM_SWIM );
    break;
  default:
    GF_ASSERT( 0 );
  }



#ifdef PM_DEBUG
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R )
  {
    FIELD_RAIL_WORK_DEBUG_PrintRailGrid( p_player->p_railwork );
  }
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  ヒット移動チェック
 *
 *	@param	cp_player   プレイヤー
 *
 *	@retval TRUE  ヒット中
 *	@retval FALSE ひっとしてない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_PLAYER_NOGRID_IsHitch( const FIELD_PLAYER_NOGRID* cp_player )
{
  if( cp_player->move_state == PLAYER_MOVE_HITCH )
  {
    return TRUE;
  }
  return FALSE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  ロケーションの設定
 *
 *	@param	p_player      プレイヤー
 *	@param	cp_location   ロケーション
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_NOGRID_SetLocation( FIELD_PLAYER_NOGRID* p_player, const RAIL_LOCATION* cp_location )
{
  MMDL_SetRailLocation( p_player->p_mmdl, cp_location );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ロケーションの取得
 *
 *	@param	cp_player     プレイヤー
 *	@param	p_location    ロケーション格納先
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_NOGRID_GetLocation( const FIELD_PLAYER_NOGRID* cp_player, RAIL_LOCATION* p_location )
{
  MMDL_GetRailLocation( cp_player->p_mmdl, p_location );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ノーグリッドマッププレイヤー　position取得
 *
 *	@param	cp_player     プレイヤーワーク
 *	@param	p_pos         position
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_NOGRID_GetPos( const FIELD_PLAYER_NOGRID* cp_player, VecFx32* p_pos )
{
  GF_ASSERT( cp_player->p_railwork );
  FIELD_RAIL_WORK_GetPos( cp_player->p_railwork, p_pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レールワークの取得
 *
 *	@param	cp_player   プレイヤー
 *
 *	@return レールワーク
 */
//-----------------------------------------------------------------------------
FIELD_RAIL_WORK* FIELD_PLAYER_NOGRID_GetRailWork( const FIELD_PLAYER_NOGRID* cp_player )
{
  GF_ASSERT( cp_player->p_railwork );
  return cp_player->p_railwork;
}

//----------------------------------------------------------------------------
/**
 *	@brief  自機  FORM　変更
 *
 *	@param	p_player
 *	@param	form 
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_NOGRID_ChangeForm( FIELD_PLAYER_NOGRID* p_player, PLAYER_MOVE_FORM form )
{
  GF_ASSERT( form < PLAYER_MOVE_FORM_MAX );
  
  p_player->form_change_req = TRUE;
  p_player->form_change     = form;
}

//----------------------------------------------------------------------------
/**
 *	@brief  自機　強制停止　ノーグリッドマップ用
 *
 *	@param	p_player  プレイヤーワーク
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_NOGRID_ForceStop( FIELD_PLAYER_NOGRID* p_player )
{
  // 長いアクションコマンドなどが出来てきたら必要
  if( p_player->move_state == PLAYER_MOVE_HITCH )
  {
    MMDL *mmdl = FIELD_PLAYER_GetMMdl( p_player->p_player );
    u16 dir = MMDL_GetDirDisp( mmdl );
    MMDL_FreeAcmd( mmdl );
    MMDL_SetDirDisp( mmdl, dir );
    MMDL_SetDrawStatus( mmdl, DRAW_STA_STOP );
    FIELD_PLAYER_SetMoveValue( p_player->p_player, PLAYER_MOVE_VALUE_STOP );
  }
}






//======================================================================
//	ノングリッド移動　フィールドプレイヤー制御
//======================================================================
//--------------------------------------------------------------
/**
 * 自機　ノングリッド移動
 * @param fld_player FIELD_PLAYER
 * @param key キー情報
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_NOGRID_Free_Move( FIELD_PLAYER *fld_player, int key, fx32 onedist )
{
	VecFx32 pos;
	FIELDMAP_WORK *fieldWork;
	VecFx32	vecMove = { 0, 0, 0 };
	
	FIELD_PLAYER_GetPos( fld_player, &pos );
	fieldWork = FIELD_PLAYER_GetFieldMapWork( fld_player );
	
	nogridPC_Move_SetValue( fld_player, fieldWork, key, &vecMove );
	vecMove.x = FX_Mul( vecMove.x, onedist );	// 移動距離を設定
	vecMove.z = FX_Mul( vecMove.z, onedist );
	
	if (key & PAD_BUTTON_B) {
		VEC_Add( &pos, &vecMove, &pos );
		FIELD_PLAYER_SetPos( fld_player, &pos );
	} else {
		fx32 diff;
		nogridPC_Move_CalcSetGroundMove(
				FIELDMAP_GetFieldG3Dmapper(fieldWork),
				FIELD_PLAYER_GetGridInfoData(fld_player),
				&pos, &vecMove, MV_SPEED );
		FIELD_PLAYER_SetPos( fld_player, &pos );
	}
}

//--------------------------------------------------------------
/**
 * 自機　ノングリッド移動 C3用
 * @param fld_player FIELD_PLAYER
 * @param key キー情報
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_C3_Move( FIELD_PLAYER *fld_player, int key, u16 angle )
{
	VecFx32	vecMove = { 0, 0, 0 };
	FIELDMAP_WORK *fieldWork = FIELD_PLAYER_GetFieldMapWork( fld_player );
	nogridPC_Move_SetValue( fld_player, fieldWork, key, &vecMove );
}


//======================================================================
//	ノングリッド自機　移動処理
//======================================================================
//--------------------------------------------------------------
/**
 * ノングリッド自機　移動制御　移動量、方向をセット
 * @param	fld_player FIELD_PLAYER
 * @param	fieldWork FIELDMAP_WORK
 * @param key キー情報
 * @param vec 移動量格納先
 * @retval nothing
 */
//--------------------------------------------------------------
static void nogridPC_Move_SetValue( FIELD_PLAYER *fld_player,
		FIELDMAP_WORK *fieldWork, u16 key, VecFx32 *vec )
{
	u16		dir;
	MMDL *fmmdl;
	BOOL	mvFlag = FALSE;
	
	fmmdl = FIELD_PLAYER_GetMMdl( fld_player );
	dir = FIELD_CAMERA_GetAngleYaw( FIELDMAP_GetFieldCamera(fieldWork) );
	
	if( key & PAD_KEY_UP )
	{
		mvFlag = TRUE;
		vec->x = FX_SinIdx( (u16)(dir + 0x8000) );
		vec->z = FX_CosIdx( (u16)(dir + 0x8000) );
		FIELD_PLAYER_SetDir( fld_player, dir );
		MMDL_SetDirDisp(fmmdl,DIR_UP);
	}

	if( key & PAD_KEY_DOWN )
	{
		mvFlag = TRUE;
		vec->x = FX_SinIdx( (u16)(dir + 0x0000) );
		vec->z = FX_CosIdx( (u16)(dir + 0x0000) );
		FIELD_PLAYER_SetDir( fld_player, dir + 0x8000 );
		MMDL_SetDirDisp(fmmdl, DIR_DOWN);
	}

	if( key & PAD_KEY_LEFT )
	{
		mvFlag = TRUE;
		vec->x = FX_SinIdx( (u16)(dir + 0xc000) );
		vec->z = FX_CosIdx( (u16)(dir + 0xc000) );
		FIELD_PLAYER_SetDir( fld_player, dir + 0x4000 );
		MMDL_SetDirDisp(fmmdl,DIR_LEFT);
	}

	if( key & PAD_KEY_RIGHT )
	{
		mvFlag = TRUE;
		vec->x = FX_SinIdx( (u16)(dir + 0x4000) );
		vec->z = FX_CosIdx( (u16)(dir + 0x4000) );
		FIELD_PLAYER_SetDir( fld_player, dir + 0xc000 );
		MMDL_SetDirDisp(fmmdl,DIR_RIGHT);
	}

	if (key & PAD_BUTTON_Y) {
		vec->y = -2 * FX32_ONE;
	}
	if (key & PAD_BUTTON_X) {
		vec->y = +2 * FX32_ONE;
	}
	
	if( mvFlag == TRUE ){
		if( key & PAD_BUTTON_B )
		{	
			//MMDL_SetDrawStatus(fmmdl,DRAW_STA_DASH_4F);
			MMDL_SetDrawStatus(fmmdl,DRAW_STA_WALK_8F);
		}
		else
		{
			MMDL_SetDrawStatus(fmmdl,DRAW_STA_WALK_8F);
		}
	} else {
		MMDL_SetDrawStatus(fmmdl,DRAW_STA_STOP);
	}
}

//--------------------------------------------------------------
/**
 * 移動方向の地形に沿ったベクトル取得
 * @param	vecN 地形ベクトル
 * @param	pos 座標
 * @param	vecMove 移動ベクトル
 * @param	result 結果格納先
 * @retval nothing
 */
//--------------------------------------------------------------
static void nogridPC_Move_GetGroundMoveVec(
	const VecFx16* vecN, const VecFx32* pos,
	const VecFx32* vecMove, VecFx32* result )
{
	VecFx32	vecN32, posNext;
	fx32	by, valD;
	
	VEC_Add( pos, vecMove, &posNext );

	VEC_Set( &vecN32, vecN->x, vecN->y, vecN->z );
	valD = -( FX_Mul(vecN32.x,pos->x) + FX_Mul(vecN32.y,pos->y) + FX_Mul(vecN32.z,pos->z) ); 
	by = -( FX_Mul( vecN32.x, posNext.x ) + FX_Mul( vecN32.z, posNext.z ) + valD );
	posNext.y = FX_Div( by, vecN32.y );

	VEC_Subtract( &posNext, pos, result );
	VEC_Normalize( result, result );
}

//--------------------------------------------------------------
/**
 * ノングリッド移動　移動計算
 * @param	g3Dmapper FLDMAPPER*
 * @param gridInfoData FLDMAPPER_GRIDINFODATA
 * @param pos 座標
 * @param vecMove 移動量
 * @param speed 移動速度
 * @retval BOOL FALSE=移動不可
 */
//--------------------------------------------------------------
static BOOL nogridPC_Move_CalcSetGroundMove(
		const FLDMAPPER *g3Dmapper,
		FLDMAPPER_GRIDINFODATA* gridInfoData,
		VecFx32* pos, VecFx32* vecMove, fx32 speed )
{
	FLDMAPPER_GRIDINFO gridInfo;
	VecFx32	posNext, vecGround;
	fx32	height = 0;
	BOOL	initSw = FALSE;

	//VEC_Normalize( &vecMove, &vecMove );
	//
	if( (gridInfoData->vecN.x|gridInfoData->vecN.y|gridInfoData->vecN.z) == 0 ){
		//vecN = {0,0,0}の場合は初期状態
		VecFx16	vecNinit = { 0, FX16_ONE, 0 };

		nogridPC_Move_GetGroundMoveVec( &vecNinit, pos, vecMove, &vecGround );
		initSw = TRUE;
	} else {
		nogridPC_Move_GetGroundMoveVec( &gridInfoData->vecN, pos, vecMove, &vecGround );
	}
	VEC_MultAdd( speed, &vecGround, pos, &posNext );
	if( posNext.y < 0 ){
		posNext.y = 0;	//ベースライン
	}
	if( FLDMAPPER_CheckOutRange( g3Dmapper, &posNext ) == TRUE ){
	//	OS_Printf("マップ範囲外で移動不可\n");
		return FALSE;
	}

	//プレーヤー用動作。この位置中心に高さデータが存在するため、すべて取得して設定
	if( FLDMAPPER_GetGridInfo( g3Dmapper, &posNext, &gridInfo ) == FALSE ){
		return FALSE;
	}

	if( gridInfo.count ){
		int		i = 0, p;
		fx32	h_tmp, diff1, diff2;

		height = gridInfo.gridData[0].height;
		p = 0;
		i++;
		for( ; i<gridInfo.count; i++ ){
			h_tmp = gridInfo.gridData[i].height;

			diff1 = height - pos->y;
			diff2 = h_tmp - pos->y;

			if( FX_Mul( diff2, diff2 ) < FX_Mul( diff1, diff1 ) ){
				height = h_tmp;
				p = i;
			}
		}
		*gridInfoData = gridInfo.gridData[p];	//グリッドデータ更新
		VEC_Set( pos, posNext.x, gridInfoData->height, posNext.z );		//位置情報更新
	}
	return TRUE;
}






//----------------------------------------------------------------------------
/**
 *	@brief  フォルムの変更
 *
 *	@param	p_player 
 */
//-----------------------------------------------------------------------------
static void updateFormChange( FIELD_PLAYER_NOGRID* p_player )
{
  if( p_player->form_change_req )
  {
    FIELD_PLAYER_ChangeMoveForm( p_player->p_player, p_player->form_change );

    p_player->form_change_req = FALSE;
  }
}





//======================================================================
//  グリッド移動　フィールドプレイヤー制御　通常移動
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド移動　フィールドプレイヤー制御　通常移動
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval nothing
 */
//--------------------------------------------------------------
static void jikiMove_Normal(
		FIELD_PLAYER_NOGRID *p_player, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag )
{
	PLAYER_SET set;
	MMDL *mmdl = FIELD_PLAYER_GetMMdl( p_player->p_player );
   
	set = PLAYER_SET_NON;
	switch( p_player->move_state ){
	case PLAYER_MOVE_STOP:
		set = player_CheckMoveStart_Stop(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_WALK:
		set = player_CheckMoveStart_Walk(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_TURN:
		set = player_CheckMoveStart_Turn(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_HITCH:
		set = player_CheckMoveStart_Hitch(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	default:
		GF_ASSERT( 0 );
	}
	
	switch( set ){
	case PLAYER_SET_NON:
		player_SetMove_Non(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_STOP:
		player_SetMove_Stop(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_WALK:
		player_SetMove_Walk(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_TURN:
		player_SetMove_Turn(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_HITCH:
		player_SetMove_Hitch(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
/*  case PLAYER_SET_JUMP:
		player_SetMove_Jump(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
    break;//*/
	}
}

//======================================================================
//	移動開始チェック　通常移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始チェック　停止中
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET player_CheckMoveStart_Stop(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
		if( dir != DIR_NOT ){
			u16 old_dir;
			old_dir = MMDL_GetDirDisp( mmdl );
			
			if( dir != old_dir && debug_flag == FALSE ){
				return( PLAYER_SET_TURN );
			}
			
			return( player_CheckMoveStart_Walk(
				p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
		}
		
		return( PLAYER_SET_STOP );
	}
	
	return( PLAYER_SET_NON );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　移動中
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET player_CheckMoveStart_Walk(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT )
  {
		return( player_CheckMoveStart_Stop(
			p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	{
		u32 hit = MMDL_HitCheckRailMoveDir( mmdl, dir );

		if( debug_flag == TRUE )
    {
      // アトリビュートムシ
			if( hit & MMDL_MOVEHITBIT_ATTR )
      {
				hit = MMDL_MOVEHITBIT_NON;
			}
		}
		
		if( hit == MMDL_MOVEHITBIT_NON )
    {
			return( PLAYER_SET_WALK );
		}

    
/*
    if( (hit & MMDL_MOVEHITBIT_ATTR) )
    {
      BOOL ret;
      u32 attr;
      VecFx32 pos;
      
      MMDL_GetVectorPos( mmdl, &pos );
      MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
      ret = MMDL_GetMapPosAttr( mmdl, &pos, &attr );
      
      if( ret == TRUE )
      {
        u16 attr_dir = DIR_NOT;
        MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
        MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( attr );
        
        if( MAPATTR_VALUE_CheckJumpUp(val) ){
          attr_dir = DIR_UP;
        }else if( MAPATTR_VALUE_CheckJumpDown(val) ){
          attr_dir = DIR_DOWN;
        }else if( MAPATTR_VALUE_CheckumpLeft(val) ){
          attr_dir = DIR_LEFT;
        }else if( MAPATTR_VALUE_CheckJumpRight(val) ){
          attr_dir = DIR_RIGHT;
        }
        
        if( attr_dir != DIR_NOT && attr_dir == dir ) //ジャンプ方向一致
        {
          return( PLAYER_SET_JUMP );
        }
      }
    }
//*/
	}
	
	return( PLAYER_SET_HITCH );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　振り向き中
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET player_CheckMoveStart_Turn(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( player_CheckMoveStart_Stop(
			p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( player_CheckMoveStart_Walk(
		p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　障害物ヒット中
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET player_CheckMoveStart_Hitch(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 dir_now = MMDL_GetDirDisp( mmdl );
   
  if( dir != DIR_NOT && dir != dir_now ){
    MMDL_FreeAcmd( mmdl );
	  return( player_CheckMoveStart_Walk(
		  p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( player_CheckMoveStart_Stop(
			p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( player_CheckMoveStart_Walk(
		p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//======================================================================
//	移動セット　通常移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始セット 特に無し
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void player_SetMove_Non(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
}

//--------------------------------------------------------------
/**
 * 移動セット 停止
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void player_SetMove_Stop(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	if( dir == DIR_NOT ){
		dir = MMDL_GetDirDisp( mmdl );
	}
	
	code = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
	MMDL_SetAcmd( mmdl, code );
	p_player->move_state = PLAYER_MOVE_STOP;
	
  FIELD_PLAYER_SetMoveValue( p_player->p_player, PLAYER_MOVE_VALUE_STOP );
}

//--------------------------------------------------------------
/**
 * 移動セット 移動
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void player_SetMove_Walk(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	
	if( debug_flag == TRUE ){
		code = AC_WALK_U_2F;
	}else if( key_cont & PAD_BUTTON_B ){
		code = AC_DASH_U_4F;
	}else{
		code = AC_WALK_U_8F;
	}
  
	code = MMDL_ChangeDirAcmdCode( dir, code );
	
	MMDL_SetAcmd( mmdl, code );
	p_player->move_state = PLAYER_MOVE_WALK;

  FIELD_PLAYER_SetMoveValue( p_player->p_player, PLAYER_MOVE_VALUE_WALK );
}

//--------------------------------------------------------------
/**
 * 移動セット 振り向き
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void player_SetMove_Turn(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_2F );
	
	MMDL_SetAcmd( mmdl, code );
	p_player->move_state = PLAYER_MOVE_TURN;
	
  FIELD_PLAYER_SetMoveValue( p_player->p_player, PLAYER_MOVE_VALUE_TURN );
}

//--------------------------------------------------------------
/**
 * 移動セット 障害物ヒット
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void player_SetMove_Hitch(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_16F );
	
	MMDL_SetAcmd( mmdl, code );
	p_player->move_state = PLAYER_MOVE_HITCH;
	
  FIELD_PLAYER_SetMoveValue( p_player->p_player, PLAYER_MOVE_VALUE_STOP );
  PMSND_PlaySE( SEQ_SE_WALL_HIT );
}

#if 0
//--------------------------------------------------------------
/**
 * 移動セット ジャンプ
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void player_SetMove_Jump(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	
	code = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_2G_16F );
	
	MMDL_SetAcmd( mmdl, code );
	p_player->move_state = PLAYER_MOVE_WALK;
  
  FIELD_PLAYER_SetMoveValue( p_player->p_player, PLAYER_MOVE_VALUE_WALK );
  PMSND_PlaySE( SEQ_SE_DANSA );

}
#endif




//======================================================================
//  グリッド移動　フィールドプレイヤー制御　自転車移動
//======================================================================
//--------------------------------------------------------------
/**
 * グリッド移動　フィールドプレイヤー制御　自転車移動
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @retval nothing
 */
//--------------------------------------------------------------
static void jikiMove_Cycle(
		FIELD_PLAYER_NOGRID *p_player, int key_trg, int key_cont,
    u16 dir, BOOL debug_flag )
{
	PLAYER_SET set;
	MMDL *mmdl = FIELD_PLAYER_GetMMdl( p_player->p_player );
   
	set = PLAYER_SET_NON;
	switch( p_player->move_state ){
	case PLAYER_MOVE_STOP:
		set = playerCycle_CheckMoveStart_Stop(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_WALK:
		set = playerCycle_CheckMoveStart_Walk(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_TURN:
		set = playerCycle_CheckMoveStart_Turn(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_MOVE_HITCH:
		set = playerCycle_CheckMoveStart_Hitch(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	default:
		GF_ASSERT( 0 );
	}
	
	switch( set ){
	case PLAYER_SET_NON:
		playerCycle_SetMove_Non(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_STOP:
		playerCycle_SetMove_Stop(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_WALK:
		playerCycle_SetMove_Walk(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_TURN:
		playerCycle_SetMove_Turn(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
	case PLAYER_SET_HITCH:
		playerCycle_SetMove_Hitch(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
		break;
/*  case PLAYER_SET_JUMP:
		playerCycle_SetMove_Jump(
			p_player, mmdl, key_trg, key_cont, dir, debug_flag );
    break;
//*/
	}
}

//======================================================================
//	移動開始チェック　自転車移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始チェック　停止中
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET playerCycle_CheckMoveStart_Stop(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
		if( dir != DIR_NOT ){
			u16 old_dir;
			old_dir = MMDL_GetDirDisp( mmdl );
			
			if( dir != old_dir && debug_flag == FALSE ){
				return( PLAYER_SET_TURN );
			}
			
			return( playerCycle_CheckMoveStart_Walk(
				p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
		}
		
		return( PLAYER_SET_STOP );
	}
	
	return( PLAYER_SET_NON );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　移動中
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET playerCycle_CheckMoveStart_Walk(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT )
  {
		return( playerCycle_CheckMoveStart_Stop(
			p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	{
		u32 hit = MMDL_HitCheckRailMoveDir( mmdl, dir );
		
		if( debug_flag == TRUE )
    {
      // アトリビュートムシ
			if( hit & MMDL_MOVEHITBIT_ATTR )
      {
				hit = MMDL_MOVEHITBIT_NON;
			}
		}
		
		if( hit == MMDL_MOVEHITBIT_NON )
    {
			return( PLAYER_SET_WALK );
		}
    
/*
    if( (hit & MMDL_MOVEHITBIT_ATTR) )
    {
      BOOL ret;
      u32 attr;
      VecFx32 pos;
      
      MMDL_GetVectorPos( mmdl, &pos );
      MMDL_TOOL_AddDirVector( dir, &pos, GRID_FX32 );
      ret = MMDL_GetMapPosAttr( mmdl, &pos, &attr );
      
      if( ret == TRUE )
      {
        u16 attr_dir = DIR_NOT;
        MAPATTR_VALUE val = MAPATTR_GetAttrValue( attr );
        MAPATTR_FLAG flag = MAPATTR_GetAttrFlag( attr );
        
        if( MAPATTR_VALUE_CheckJumpUp(val) ){
          attr_dir = DIR_UP;
        }else if( MAPATTR_VALUE_CheckJumpDown(val) ){
          attr_dir = DIR_DOWN;
        }else if( MAPATTR_VALUE_CheckumpLeft(val) ){
          attr_dir = DIR_LEFT;
        }else if( MAPATTR_VALUE_CheckJumpRight(val) ){
          attr_dir = DIR_RIGHT;
        }
        
        if( attr_dir != DIR_NOT && attr_dir == dir ) //ジャンプ方向一致
        {
          return( PLAYER_SET_JUMP );
        }
      }
    }
//*/
	}
	
	return( PLAYER_SET_HITCH );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　振り向き中
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET playerCycle_CheckMoveStart_Turn(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( playerCycle_CheckMoveStart_Stop(
			p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( playerCycle_CheckMoveStart_Walk(
		p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//--------------------------------------------------------------
/**
 * 移動開始チェック　障害物ヒット中
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param dir 移動方向
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval PLAYER_SET
 */
//--------------------------------------------------------------
static PLAYER_SET playerCycle_CheckMoveStart_Hitch(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
  u16 dir_now = MMDL_GetDirDisp( mmdl );
   
  if( dir != DIR_NOT && dir != dir_now ){
    MMDL_FreeAcmd( mmdl );
	  return( playerCycle_CheckMoveStart_Walk(
		  p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
  }
  
	if( MMDL_CheckPossibleAcmd(mmdl) == FALSE ){
		return( PLAYER_SET_NON );
	}
	
	if( dir == DIR_NOT ){
		return( playerCycle_CheckMoveStart_Stop(
			p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
	}
	
	return( playerCycle_CheckMoveStart_Walk(
		p_player,mmdl,key_trg,key_cont,dir,debug_flag) );
}

//======================================================================
//	移動セット　自転車移動
//======================================================================
//--------------------------------------------------------------
/**
 * 移動開始セット 特に無し
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void playerCycle_SetMove_Non(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
}

//--------------------------------------------------------------
/**
 * 移動セット 停止
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void playerCycle_SetMove_Stop(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	if( dir == DIR_NOT ){
		dir = MMDL_GetDirDisp( mmdl );
	}
	
	code = MMDL_ChangeDirAcmdCode( dir, AC_DIR_U );
	MMDL_SetAcmd( mmdl, code );
	p_player->move_state = PLAYER_MOVE_STOP;
	
  FIELD_PLAYER_SetMoveValue( p_player->p_player, PLAYER_MOVE_VALUE_STOP );
}

//--------------------------------------------------------------
/**
 * 移動セット 移動
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void playerCycle_SetMove_Walk(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	
	if( debug_flag == TRUE ){
		code = AC_WALK_U_2F;
	}else{
		code = AC_WALK_U_2F;
	}
  
	code = MMDL_ChangeDirAcmdCode( dir, code );
	
	MMDL_SetAcmd( mmdl, code );
	p_player->move_state = PLAYER_MOVE_WALK;

  FIELD_PLAYER_SetMoveValue( p_player->p_player, PLAYER_MOVE_VALUE_WALK );
}

//--------------------------------------------------------------
/**
 * 移動セット 振り向き
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void playerCycle_SetMove_Turn(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_2F );
	
	MMDL_SetAcmd( mmdl, code );
	p_player->move_state = PLAYER_MOVE_TURN;
	
  FIELD_PLAYER_SetMoveValue( p_player->p_player, PLAYER_MOVE_VALUE_TURN );
}

//--------------------------------------------------------------
/**
 * 移動セット 障害物ヒット
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void playerCycle_SetMove_Hitch(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	code = MMDL_ChangeDirAcmdCode( dir, AC_STAY_WALK_U_16F );
	
	MMDL_SetAcmd( mmdl, code );
	p_player->move_state = PLAYER_MOVE_HITCH;
	
  FIELD_PLAYER_SetMoveValue( p_player->p_player, PLAYER_MOVE_VALUE_STOP );
  PMSND_PlaySE( SEQ_SE_WALL_HIT );
}

#if 0
//--------------------------------------------------------------
/**
 * 移動セット ジャンプ
 * @param	p_player FIELD_PLAYER_NOGRID
 * @param mmdl MMDL*
 * @param key_trg キートリガ
 * @param key_cont キーコンティニュー
 * @param debug_flag デバッグ移動可能フラグ TRUE=可能
 * @retval nothing
 */
//--------------------------------------------------------------
static void playerCycle_SetMove_Jump(
	FIELD_PLAYER_NOGRID *p_player, MMDL *mmdl,
	u32 key_trg, u32 key_cont, u16 dir, BOOL debug_flag )
{
	u16 code;
	
	GF_ASSERT( dir != DIR_NOT );
	
	code = MMDL_ChangeDirAcmdCode( dir, AC_JUMP_U_2G_16F );
	
	MMDL_SetAcmd( mmdl, code );
	p_player->move_state = PLAYER_MOVE_WALK;
  
  FIELD_PLAYER_SetMoveValue( p_player->p_player, PLAYER_MOVE_VALUE_WALK );
  PMSND_PlaySE( SEQ_SE_DANSA );
}
#endif

