//======================================================================
/**
 * @file	field_player.c
 * @date	2008.9.29
 * @brief	フィールドプレイヤー制御
 */
//======================================================================
#include <gflib.h>

#include "fieldmap.h"
#include "field_g3d_mapper.h"
#include "fldmmdl.h"
#include "field_sound.h"
#include "field_player.h"

#include "field_player_core.h"
#include "field_player_grid.h"
#include "field_player_nogrid.h"

#include "field/field_const.h"


//======================================================================
//	define
//======================================================================

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
/// OBJCODE_FORM
//--------------------------------------------------------------
typedef struct
{
  u16 code;
  u16 move_form;
}OBJCODE_FORM;

//--------------------------------------------------------------
///	FIELD_PLAYER
//--------------------------------------------------------------
struct _FIELD_PLAYER
{
  FIELDMAP_WORK * fieldWork;
  FIELD_PLAYER_CORE * corewk;   // field_player_core
  FIELD_PLAYER_GRID * gridwk;   // field_player_grid
  FIELD_PLAYER_NOGRID * nogridwk; // field_player_nogrid
};

//======================================================================
//	proto
//======================================================================
static const OBJCODE_FORM dataOBJCodeForm[2][PLAYER_DRAW_FORM_MAX];




//======================================================================
//	フィールドプレイヤー
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドプレイヤー　作成
 * @param playerWork 使用するPLAYER_WOKR
 * @param *fieldWork 参照するFIELDMAP_WORK
 * @param pos 初期座標
 * @param sex 性別
 * @param heapID HEAPID
 * @retval FIELD_PLAYER*
 */
//--------------------------------------------------------------
FIELD_PLAYER * FIELD_PLAYER_Create(
    PLAYER_WORK *playerWork, FIELDMAP_WORK *fieldWork,
		const VecFx32 *pos, int sex, HEAPID heapID )
{
  FIELD_PLAYER* fld_player;

  fld_player = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_PLAYER) );

  fld_player->fieldWork = fieldWork;

  // COREWK生成
  fld_player->corewk = FIELD_PLAYER_CORE_Create( playerWork, fieldWork, pos, sex, heapID );

  return fld_player;
}

//--------------------------------------------------------------
/**
 * フィールドプレイヤー　削除
 * @param fld_player FIELD_PLAYER
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_Delete( FIELD_PLAYER *fld_player )
{
  // GRID
  if( fld_player->gridwk ){
    FIELD_PLAYER_GRID_Delete( fld_player->gridwk );
  }
  

  // NOGRID
  if( fld_player->nogridwk ){
    FIELD_PLAYER_NOGRID_Delete( fld_player->nogridwk );
  }
  
  // CORE
  FIELD_PLAYER_CORE_Delete( fld_player->corewk );
  
	//動作モデルの削除はフィールドメイン側で
	GFL_HEAP_FreeMemory( fld_player );
}

//--------------------------------------------------------------
/**
 * フィールドプレイヤー　更新
 * @param fld_player FIELD_PLAYER
 * @param dir 方向
 * @param pos 座標
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_Update( FIELD_PLAYER *fld_player )
{
  
  // コアアップデート
  FIELD_PLAYER_CORE_Update( fld_player->corewk );
  { 
    //プレイヤー座標をPLAYERWORKへ反映
    VecFx32 pos;
    PLAYER_WORK* playerWork = FIELD_PLAYER_CORE_GetPlayerWork( fld_player->corewk );
    MMDL* fldmmdl = FIELD_PLAYER_CORE_GetMMdl( fld_player->corewk );
    
    MMDL_GetVectorPos( fldmmdl, &pos );
    PLAYERWORK_setPosition( playerWork, &pos );
    
    //方向をPLAYERWORKへ反映
    {
      u16 dir = MMDL_GetDirDisp(fldmmdl);
      PLAYERWORK_setDirection_Type( playerWork, dir );
    }
    
    // レール独自の更新処理
    if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_GRID )
    {
      PLAYERWORK_setPosType( playerWork, LOCATION_POS_TYPE_3D );
    }
    else
    {
      RAIL_LOCATION location;
      MMDL_GetRailLocation( fldmmdl, &location );
      PLAYERWORK_setRailPosition( playerWork, &location );
      PLAYERWORK_setPosType( playerWork, LOCATION_POS_TYPE_RAIL );
    }
  }
}

//--------------------------------------------------------------
/**
 * 自機イベント移動チェック
 * @param fld_player FIELD_PLAYER
 * @param key_trg 入力キートリガ情報
 * @param key_cont 入力キーコンテニュー情報
 * @param evbit PLAYER_EVENTBIT
 * @retval BOOL TRUE=自機イベント移動発生
 */
//--------------------------------------------------------------
GMEVENT * FIELD_PLAYER_CheckMoveEvent( FIELD_PLAYER *fld_player,
    int key_trg, int key_cont, PLAYER_EVENTBIT evbit )
{
  GMEVENT *event = NULL;
  
  if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_GRID )
  {
    event = FIELD_PLAYER_GRID_CheckMoveEvent(
        fld_player->gridwk, key_trg, key_cont, evbit );
  }
  else
  {
    OS_TPrintf( "Check Move Event rail not support\n" );
  }
  
  return( event );
}


//--------------------------------------------------------------
/**
 * 自機リクエスト 
 * @param fld_player FIELD_PLAYER
 * @param reqbit FIELD_PLAYER_REQBIT
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetRequest(
  FIELD_PLAYER * fld_player, FIELD_PLAYER_REQBIT req_bit )
{
  FIELD_PLAYER_CORE_SetRequest( fld_player->corewk, req_bit );
}

//--------------------------------------------------------------
/**
 * リクエストを更新
 * @param fld_player FIELD_PLAYER
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_UpdateRequest( FIELD_PLAYER * fld_player )
{
  FIELD_PLAYER_CORE_UpdateRequest( fld_player->corewk );
}


//======================================================================
//  移動チェック
//======================================================================
//--------------------------------------------------------------
/**
 * 自機オーダーチェック
 * @param gjiki FIELD_PLAYER_GRID
 * @param dir 移動方向。DIR_UP等
 * @retval BOOL TRUE=移動可能 FALSE=移動不可
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_CheckStartMove(
    FIELD_PLAYER * fld_player, u16 dir )
{
  if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_GRID ){
    return FIELD_PLAYER_GRID_CheckStartMove( fld_player->gridwk, dir );
  }
  return FIELD_PLAYER_CORE_CheckStartMove( fld_player->corewk, dir );
}

//----------------------------------------------------------------------------
/**
 *	@brief  自機を強制停止させる
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_ForceStop( FIELD_PLAYER * fld_player )
{
  if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_GRID ){
    FIELD_PLAYER_GRID_ForceStop( fld_player->gridwk );
  }else{
    FIELD_PLAYER_NOGRID_ForceStop( fld_player->nogridwk );
  }
}

//--------------------------------------------------------------
/**
 * 自機　動作停止
 * @param fld_player
 * @retval BOOL TRUE=停止完了。FALSE=移動中につき停止出来ない。
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetMoveStop( FIELD_PLAYER * fld_player )
{
  if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_GRID ){
    FIELD_PLAYER_GRID_SetMoveStop( fld_player->gridwk );
  }else{
    //FIELD_PLAYER_NOGRID_ForceStop( fld_player->nogridwk );
    OS_Printf( "rail not support\n" );
    GF_ASSERT( 0 );
  }
}


//--------------------------------------------------------------
/**
 * 自機に波乗りポケモンのタスクポインタをセット
 * @param gjiki FIELD_PLAYER
 * @param task セットするFLDEFF_TASK
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetEffectTaskWork(
    FIELD_PLAYER * fld_player, FLDEFF_TASK *task )
{
  FIELD_PLAYER_CORE_SetEffectTaskWork( fld_player->corewk, task );
}

//--------------------------------------------------------------
/**
 * 自機が持っているエフェクトタスクのポインタを取得
 * @param   gjiki FIELD_PLAYER
 * @retval FLDEFF_TASK*
 */
//--------------------------------------------------------------
FLDEFF_TASK * FIELD_PLAYER_GetEffectTaskWork(
    FIELD_PLAYER * fld_player )
{
  return FIELD_PLAYER_CORE_GetEffectTaskWork( fld_player->corewk );
}

//--------------------------------------------------------------
/**
 * 指定方向を自機に渡すとPLAYER_MOVE_VALUEはどうなるかチェック
 * @param fld_player FIELD_PLAYER
 * @param dir キー入力方向
 * @retval PLAYER_MOVE_VALUE
 */
//--------------------------------------------------------------
PLAYER_MOVE_VALUE FIELD_PLAYER_GetDirMoveValue(
    FIELD_PLAYER * fld_player, u16 dir )
{
  if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_GRID ){
    return FIELD_PLAYER_GRID_GetMoveValue( fld_player->gridwk, dir );
  }else{
    //FIELD_PLAYER_NOGRID_ForceStop( fld_player->nogridwk );
    OS_Printf( "rail not support\n" );
    GF_ASSERT( 0 );
  }
  return 0;
}

//--------------------------------------------------------------
/**
 * 自機　入力キーから移動する方向を返す
 * @param gjiki FIELD_PLAYER
 * @param key キー情報
 * @retval u16 移動方向 DIR_UP等
 */
//--------------------------------------------------------------
u16 FIELD_PLAYER_GetKeyDir( const FIELD_PLAYER* fld_player, int key )
{
  return FIELD_PLAYER_CORE_GetKeyDir( fld_player->corewk, key );
}


//--------------------------------------------------------------
/**
 * 自機を波乗り状態にする
 * @param gjiki FIELD_PLAYER
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetNaminori( FIELD_PLAYER * fld_player )
{
  FIELD_PLAYER_CORE_SetRequest( fld_player->corewk, FIELD_PLAYER_REQBIT_SWIM );
  FIELD_PLAYER_CORE_UpdateRequest( fld_player->corewk );
}

//--------------------------------------------------------------
/**
 * 自機波乗り状態を終了する
 * @param gjiki FIELD_PLAYER
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetNaminoriEnd( FIELD_PLAYER * fld_player )
{
  FIELD_PLAYER_CORE_SetRequest( fld_player->corewk, FIELD_PLAYER_REQBIT_NORMAL );
  FIELD_PLAYER_CORE_UpdateRequest( fld_player->corewk );
}






//======================================================================
//  FIELD_PLAYER 動作ステータス
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドプレイヤー　動作ステータス更新
 * @param fld_player
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_UpdateMoveStatus( FIELD_PLAYER *fld_player )
{
  
  //グリッド限定　各種強制移動チェック
  if( FIELDMAP_GetBaseSystemType(
        fld_player->fieldWork) == FLDMAP_BASESYS_GRID )
  {
    //足元強制移動チェック
    if( FIELD_PLAYER_GRID_CheckUnderForceMove(fld_player->gridwk) == TRUE )
    {
      FIELD_PLAYER_CORE_SetMoveState( fld_player->corewk, PLAYER_MOVE_STATE_ON );
      return;
    }
    
    //尾瀬落下チェック
    if( FIELD_PLAYER_GRID_CheckOzeFallOut(fld_player->gridwk) == TRUE )
    {
      FIELD_PLAYER_CORE_SetMoveState( fld_player->corewk, PLAYER_MOVE_STATE_ON );
      return;
    }
  }

  FIELD_PLAYER_CORE_UpdateMoveStatus( fld_player->corewk );
}

//======================================================================
//	FILED_PLAYER　参照、設定
//======================================================================
//--------------------------------------------------------------
/**
 * FILED_PLAYER　座標取得
 * @param fld_player FIELD_PLAYER
 * @param pos 座標格納先
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GetPos( const FIELD_PLAYER *fld_player, VecFx32 *pos )
{
  FIELD_PLAYER_CORE_GetPos( fld_player->corewk, pos );
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER　座標セット
 * @param fld_player FIELD_PLAYER
 * @param pos セットする座標
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetPos( FIELD_PLAYER *fld_player, const VecFx32 *pos )
{
  FIELD_PLAYER_CORE_SetPos( fld_player->corewk, pos );
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER　方向取得
 * @param fld_player FIELD_PLAYER
 * @retval u16 方向
 */
//--------------------------------------------------------------
u16 FIELD_PLAYER_GetDir( const FIELD_PLAYER *fld_player )
{
  return FIELD_PLAYER_CORE_GetDir( fld_player->corewk );
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER　方向セット
 * @param fld_player FIELD_PLAYER
 * @param dir 方向
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetDir( FIELD_PLAYER *fld_player, u16 dir )
{
  FIELD_PLAYER_CORE_SetDir( fld_player->corewk, dir );
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER　FIELDMAP_WORK取得
 * @param fld_player FIELD_PLAYER
 * @retval FIELDMAP_WORK*
 */
//--------------------------------------------------------------
FIELDMAP_WORK * FIELD_PLAYER_GetFieldMapWork( FIELD_PLAYER *fld_player )
{
	return FIELD_PLAYER_CORE_GetFieldMapWork( fld_player->corewk );
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER　MMDL取得
 * @param fld_player FIELD_PLAYER
 * @retval MMDL*
 */
//--------------------------------------------------------------
MMDL * FIELD_PLAYER_GetMMdl( const FIELD_PLAYER *fld_player )
{
	return FIELD_PLAYER_CORE_GetMMdl( fld_player->corewk );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER PLAYER_MOVE_VALUEセット
 * @param fld_player FIELD_PLAYER
 * @retval PLAYER_MOVE_STATE
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetMoveValue(
    FIELD_PLAYER *fld_player, PLAYER_MOVE_VALUE val )
{
  FIELD_PLAYER_CORE_SetMoveValue( fld_player->corewk, val );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER PLAYER_MOVE_VALUE取得
 * @param fld_player FIELD_PLAYER
 * @retval PLAYER_MOVE_STATE
 */
//--------------------------------------------------------------
PLAYER_MOVE_VALUE FIELD_PLAYER_GetMoveValue(
    const FIELD_PLAYER *fld_player )
{
  return FIELD_PLAYER_CORE_GetMoveValue( fld_player->corewk );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER PLAYER_MOVE_STATE取得
 * @param fld_player FIELD_PLAYER
 * @retval PLAYER_MOVE_STATE
 */
//--------------------------------------------------------------
PLAYER_MOVE_STATE FIELD_PLAYER_GetMoveState(
    const FIELD_PLAYER *fld_player )
{
  return FIELD_PLAYER_CORE_GetMoveState( fld_player->corewk );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER PLAYER_MOVE_FORM取得
 * @param fld_player FIELD_PLAYER
 * @retval PLAYER_MOVE_FORM
 */
//--------------------------------------------------------------
PLAYER_MOVE_FORM FIELD_PLAYER_GetMoveForm(
    const FIELD_PLAYER *fld_player )
{
  return FIELD_PLAYER_CORE_GetMoveForm( fld_player->corewk );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER PLAYER_MOVE_FORMセット
 * @param fld_player FIELD_PLAYER
 * @retval PLAYER_MOVE_FORM
 */
//--------------------------------------------------------------
void FIELD_PLAYER_SetMoveForm(
    FIELD_PLAYER *fld_player, PLAYER_MOVE_FORM form )
{
  FIELD_PLAYER_CORE_SetMoveForm( fld_player->corewk, form );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER FIELD_PLAYERにセットされている性別を取得
 * @param fld_player FIELD_PLAYER
 * @retval int PM_MALE,PM_FEMALE
 */
//--------------------------------------------------------------
int FIELD_PLAYER_GetSex( const FIELD_PLAYER *fld_player )
{
  return FIELD_PLAYER_CORE_GetSex( fld_player->corewk );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER GAMESYS_WORK取得
 * @param fld_player FIELD_PLAYER
 * @retval GAMESYS_WORK*
 */
//--------------------------------------------------------------
GAMESYS_WORK * FIELD_PLAYER_GetGameSysWork( FIELD_PLAYER *fld_player )
{
  return FIELD_PLAYER_CORE_GetGameSysWork( fld_player->corewk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレイヤー位置のアトリビュートの取得
 *
 *	@param	fld_player  FIELD_PLAYER
 *
 *	@return マップアトリビュート
 */
//-----------------------------------------------------------------------------
MAPATTR FIELD_PLAYER_GetMapAttr( const FIELD_PLAYER *fld_player )
{
  MAPATTR attr;
  
  if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_GRID )
  {
    VecFx32 pos;  
    FLDMAPPER *mapper = FIELDMAP_GetFieldG3Dmapper( fld_player->fieldWork );

    FIELD_PLAYER_GetPos( fld_player, &pos );
    attr = MAPATTR_GetAttribute( mapper, &pos );
  }
  else if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_RAIL )
  {
    RAIL_LOCATION location;
    const MMDL * mmdl = FIELD_PLAYER_GetMMdl( (FIELD_PLAYER*)fld_player );
    const FLDNOGRID_MAPPER* mapper = FIELDMAP_GetFldNoGridMapper( fld_player->fieldWork );

    MMDL_GetRailLocation( mmdl, &location );
    attr = FLDNOGRID_MAPPER_GetAttr( mapper, &location );
  }
  
  return( attr );
}

//----------------------------------------------------------------------------
/**
 *	@brief  dir方向のアトリビュートの取得
 *
 *	@param	fld_player  FIELD_PLAYER
 *	@param	dir         方向
 *
 *	@return マップアトリビュート
 */
//-----------------------------------------------------------------------------
MAPATTR FIELD_PLAYER_GetDirMapAttr( const FIELD_PLAYER *fld_player, u16 dir )
{
  MAPATTR attr;
  
  if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_GRID )
  {
    VecFx32 pos;  
    FLDMAPPER *mapper = FIELDMAP_GetFieldG3Dmapper( fld_player->fieldWork );

    FIELD_PLAYER_GetDirPos( fld_player, dir, &pos );
    attr = MAPATTR_GetAttribute( mapper, &pos );
  }
  else if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_RAIL )
  {
    RAIL_LOCATION location;
    MMDL * mmdl = FIELD_PLAYER_GetMMdl( fld_player );
    FLDNOGRID_MAPPER* mapper = FIELDMAP_GetFldNoGridMapper( fld_player->fieldWork );

    MMDL_GetRailDirLocation( mmdl, dir, &location );
    attr = FLDNOGRID_MAPPER_GetAttr( mapper, &location );
  }
  
  return( attr );
}


//======================================================================
/// 性別、OBJコード、各フォーム
//======================================================================
//--------------------------------------------------------------
/**
 * 性別とPLAYER_DRAW_FORMからOBJコード取得
 * @param sex PM_MALE,PM_FEMALE
 * @param form PLAYER_DRAW_FORM
 * @retval OBJコード
 */
//--------------------------------------------------------------
u16 FIELD_PLAYER_GetDrawFormToOBJCode( int sex, PLAYER_DRAW_FORM form )
{
  const OBJCODE_FORM *tbl = &dataOBJCodeForm[sex][form];
  GF_ASSERT( sex < 2 ); //PM_FEMALE
  GF_ASSERT( form < PLAYER_DRAW_FORM_MAX );
  return( tbl->code );
}

//--------------------------------------------------------------
/**
 * 性別とOBJコードからからPLAYER_MOVE_FORM取得
 * @param sex PM_MALE,PM_FEMALE
 * @param code OBJコード
 * @retval PLAYER_MOVE_FORM
 * @note PLAYER_MOVE_FORM_MAX時はアサート
 */
//--------------------------------------------------------------
PLAYER_MOVE_FORM FIELD_PLAYER_GetOBJCodeToMoveForm( int sex, u16 code )
{
  int i = 0;
  const OBJCODE_FORM *tbl = dataOBJCodeForm[sex];
  GF_ASSERT( sex < 2 ); //PM_FEMALE
  for( ; i < PLAYER_DRAW_FORM_MAX; i++,tbl++ ){
    if( tbl->code == code ){
      if( tbl->move_form != PLAYER_MOVE_FORM_MAX ){
        return(tbl->move_form);
      }
      //PLAYER_MOVE_FORMが当てはまらないコードに対して
      //この関数が呼ばれている(本来ありえない
       GF_ASSERT( tbl->move_form != PLAYER_MOVE_FORM_MAX );
       break;
    }
  }
  GF_ASSERT( 0 );
  return( PLAYER_MOVE_FORM_NORMAL );
}

//--------------------------------------------------------------
/**
 * 性別とPLAYER_MOVE_FORMからOBJコード取得
 * @param sex PM_MALE,PM_FEMALE
 * @param form PLAYER_MOVE_FORM
 * @retval u16 OBJコード
 */
//--------------------------------------------------------------
u16 FIELD_PLAYER_GetMoveFormToOBJCode( int sex, PLAYER_MOVE_FORM form )
{
  int i = 0;
  const OBJCODE_FORM *tbl = dataOBJCodeForm[sex];
  GF_ASSERT( sex < 2 ); //PM_FEMALE
  GF_ASSERT( form < PLAYER_MOVE_FORM_MAX );
  for( ; i < PLAYER_DRAW_FORM_MAX; i++,tbl++ ){
    if( tbl->move_form == form ){
      return(tbl->code);
    }
  }
  GF_ASSERT( 0 );
  return( HERO );
}

//--------------------------------------------------------------
/**
 * BJコードがからPLAYER_DRAW_FORM取得
 * @param sex PM_MALE,PM_FEMALE
 * @param code OBJコード
 * @retval PLAYER_DRAW_FORM
 */
//--------------------------------------------------------------
PLAYER_DRAW_FORM FIELD_PLAYER_GetOBJCodeToDrawForm( int sex, u16 code )
{
  int i = 0;
  const OBJCODE_FORM *tbl = dataOBJCodeForm[sex];
  GF_ASSERT( sex < 2 ); //PM_FEMALE
  for( ; i < PLAYER_DRAW_FORM_MAX; i++,tbl++ ){
    if( tbl->code == code ){
      return( i );
    }
  }
  GF_ASSERT( 0 );
  return( PLAYER_DRAW_FORM_NORMAL );
}


//--------------------------------------------------------------
/**
 * OBJコードからPLAYER_DRAW_FORM取得　チェック用
 * @param sex PM_MALE,PM_FEMALE
 * @param code OBJコード
 * @retval PLAYER_DRAW_FORM
 */
//--------------------------------------------------------------
PLAYER_DRAW_FORM FIELD_PLAYER_CheckOBJCodeToDrawForm( u16 code )
{
  int sex,i;

  for( sex = 0; sex < 2; sex++ )
  {
    const OBJCODE_FORM *tbl = dataOBJCodeForm[sex];
    
    for( i = 0; i < PLAYER_DRAW_FORM_MAX; i++, tbl++ )
    {
      if( tbl->code == code )
      {
        return( i );
      }
    }
  }
  
  return( PLAYER_DRAW_FORM_MAX );
}


//--------------------------------------------------------------
/**
 * 自機をレポートやアイテムゲットなどイベント用表示に変えても問題ないか
 * @param fld_player FIELD_PLAYER
 * @retval  BOOL TRUE=OK
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_CheckChangeEventDrawForm( FIELD_PLAYER *fld_player )
{
  PLAYER_MOVE_FORM form = FIELD_PLAYER_GetMoveForm( fld_player );
  
  switch( form ){
  case PLAYER_MOVE_FORM_SWIM:
    return( FALSE );
  }
  
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 自機の表示コードを直接変更
 * @param fld_player FIELD_PLAYER
 * @param code 表示コード HERO等
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_ChangeOBJCode( FIELD_PLAYER *fld_player, u16 code )
{
  FIELD_PLAYER_CORE_ChangeOBJCode( fld_player->corewk, code );
}

//--------------------------------------------------------------
/**
 * 自機の表示コード固定を解除
 * @param fld_player FIELD_PLAYER
 * @param code 表示コード HERO等
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_ClearOBJCodeFix( FIELD_PLAYER *fld_player )
{
  FIELD_PLAYER_CORE_ClearOBJCodeFix( fld_player->corewk );
}

//--------------------------------------------------------------
/**
 * 自機の表示コードが自機専用のものかどうか
 * @param fld_player FIELD_PLAYER
 * @retval BOOL TRUE=自機専用 FALSE=自機以外のコード
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_CheckIllegalOBJCode( FIELD_PLAYER *fld_player )
{
  return FIELD_PLAYER_CORE_CheckIllegalOBJCode( fld_player->corewk );
}


//----------------------------------------------------------------------------
/**
 *	@brief  目の前にいるモデルを検索
 *
 *	@param	fld_player  フィールドプレイヤー
 *
 *	@return 目の前にいるモデル
 */
//-----------------------------------------------------------------------------
MMDL * FIELD_PLAYER_GetFrontMMdl( const FIELD_PLAYER *fld_player )
{
  MMDL* mmdl = NULL;
  MMDLSYS* mmdlsys = FIELDMAP_GetMMdlSys( fld_player->fieldWork );
  
  if( FIELDMAP_GetBaseSystemType( fld_player->fieldWork ) == FLDMAP_BASESYS_GRID )
  {
    s16 gx,gy,gz;
    
    FIELD_PLAYER_GetFrontGridPos( fld_player, &gx, &gy, &gz );
    mmdl = MMDLSYS_SearchGridPos( mmdlsys, gx, gz, FALSE );
  }
  else
  {
    MMDL* mymmdl = FIELD_PLAYER_CORE_GetMMdl( fld_player->corewk );
    RAIL_LOCATION location;
    
    MMDL_GetRailFrontLocation( mymmdl, &location );
    mmdl = MMDLSYS_SearchRailLocation( mmdlsys, &location, TRUE );
  }
  
  return mmdl;
}

//======================================================================
//	FILED_PLAYER　ツール
//======================================================================
//--------------------------------------------------------------
/**
 * 自機の指定方向先の位置をグリッド単位で取得
 * @param fld_player FIELD_PLAYER
 * @param gx X座標格納先
 * @param gy Y座標格納先
 * @param gz Z座標格納先
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GetDirGridPos(
		const FIELD_PLAYER *fld_player, u16 dir, s16 *gx, s16 *gy, s16 *gz )
{
	const MMDL *fmmdl = FIELD_PLAYER_GetMMdl( fld_player );
	
  if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_GRID )
  {
    *gx = MMDL_GetGridPosX( fmmdl );
    *gy = MMDL_GetGridPosY( fmmdl );
    *gz = MMDL_GetGridPosZ( fmmdl );
    *gx += MMDL_TOOL_GetDirAddValueGridX( dir );
    *gz += MMDL_TOOL_GetDirAddValueGridZ( dir );
  }
  else
  {
    GF_ASSERT_MSG( 0, "FIELD_PLAYER_GetDirGridPos BaseSystem Rail\n" );
  }
}

//--------------------------------------------------------------
/**
 * 自機の指定方向先の座標を取得
 * @param fld_player FIELD_PLAYER
 * @param pos 座標格納先
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GetDirPos(
		const FIELD_PLAYER *fld_player, u16 dir, VecFx32 *pos )
{
  MMDL* fldmmdl = FIELD_PLAYER_CORE_GetMMdl( fld_player->corewk );

  if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_GRID )
  {
    s16 gx,gy,gz;
    FIELD_PLAYER_GetDirGridPos( fld_player, dir, &gx, &gy, &gz );
    MMDL_TOOL_GetCenterGridPos( gx, gz, pos );
    pos->y = GRID_SIZE_FX32( gy );
  }
  else
  {
    fx32 grid_size;
    VecFx16 way;
    RAIL_LOCATION location;
    VecFx32 now_pos;
    FLDNOGRID_MAPPER* p_mapper = FIELDMAP_GetFldNoGridMapper( fld_player->fieldWork );
    FIELD_RAIL_MAN* p_railman = FLDNOGRID_MAPPER_DEBUG_GetRailMan( p_mapper );

    // 今の位置と、DIRキーの３D方向取得
    MMDL_GetRailLocation( fldmmdl, &location );
    FIELD_RAIL_MAN_GetLocationPosition( p_railman, &location, &now_pos );
    MMDL_Rail_GetDirLineWay( fldmmdl, dir, &way );

    // now_posからwayに１グリッド分進んだ先が次のポジション
    grid_size = FIELD_RAIL_MAN_GetRailGridSize( p_railman );
    pos->x = now_pos.x + FX_Mul( way.x, grid_size );
    pos->y = now_pos.y + FX_Mul( way.x, grid_size );
    pos->z = now_pos.z + FX_Mul( way.x, grid_size );
  }
}

//--------------------------------------------------------------
/**
 * 自機の前方位置をグリッド単位で取得
 * @param fld_player FIELD_PLAYER
 * @param gx X座標格納先
 * @param gy Y座標格納先
 * @param gz Z座標格納先
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_GetFrontGridPos(
		const FIELD_PLAYER *fld_player, s16 *gx, s16 *gy, s16 *gz )
{
	const MMDL *fmmdl = FIELD_PLAYER_GetMMdl( fld_player );
	u16 dir = MMDL_GetDirDisp( fmmdl );
  FIELD_PLAYER_GetDirGridPos( fld_player, dir, gx, gy, gz );
}

//----------------------------------------------------------------------------
/**
 *	@brief  自機のDIRの移動方向を取得
 *
 *	@param	fld_player    FIELD_PLAYER
 *	@param	dir           DIR_～
 *	@param	way           ３D移動方向
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_GetDirWay( 
    const FIELD_PLAYER *fld_player, u16 dir, VecFx32* way )
{
  MMDL* fldmmdl = FIELD_PLAYER_CORE_GetMMdl( fld_player->corewk );
  if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_GRID )
  {
    way->y = 0;
    way->x = MMDL_TOOL_GetDirAddValueGridX( dir );
    way->z = MMDL_TOOL_GetDirAddValueGridZ( dir );

    way->x = GRID_TO_FX32( way->x );
    way->z = GRID_TO_FX32( way->z );
    VEC_Normalize( way, way );
  }
  else
  {
    VecFx16 way16;

    MMDL_Rail_GetDirLineWay( fldmmdl, dir, &way16 );
    VEC_Set( way, way16.x, way16.y, way16.z );
  }
}

//--------------------------------------------------------------
/**
 * 動作モデルが生きているかチェック
 * @param fld_player FIELD_PLAYER
 * @retval BOOL TRUE=生存
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_CheckLiveMMdl( const FIELD_PLAYER *fld_player )
{
  return FIELD_PLAYER_CORE_CheckLiveMMdl( fld_player->corewk );
}
//--------------------------------------------------------------
/**
 * 自機の動作形態を変更 BGM変更あり
 * @param fld_player FIELD_PLAYER
 * @param form PLAYER_MOVE_FORM
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_ChangeMoveForm(
    FIELD_PLAYER *fld_player, PLAYER_MOVE_FORM form )
{
  FIELD_PLAYER_CORE_ChangeMoveForm( fld_player->corewk, form );
}

//--------------------------------------------------------------
/**
 * 自機の現在の動作状態で再セット
 * @param fld_player FIELD_PLAYER
 * @param form PLAYER_MOVE_FORM
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_ResetMoveForm( FIELD_PLAYER *fld_player )
{
  FIELD_PLAYER_CORE_ResetMoveForm( fld_player->corewk );
}

//--------------------------------------------------------------
/**
 * PLAYER_DRAW_FORMから自機のOBJコードのみ変更
 * @param fld_player FIELD_PLAYER
 * @param form PLAYER_MOVE_FORM
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_ChangeDrawForm(
    FIELD_PLAYER *fld_player, PLAYER_DRAW_FORM form )
{
  FIELD_PLAYER_CORE_ChangeDrawForm( fld_player->corewk, form );
}

//--------------------------------------------------------------
/**
 * 自機のDrawFormを取得
 * @param fld_player FIELD_PLAYER
 * @retval PLAYER_DRAW_FORM
 */
//--------------------------------------------------------------
PLAYER_DRAW_FORM FIELD_PLAYER_GetDrawForm( FIELD_PLAYER *fld_player )
{
  return FIELD_PLAYER_CORE_GetDrawForm( fld_player->corewk );
}

//--------------------------------------------------------------
/**
 * 自機を指定フォームに変更リクエスト(アニメコール込み)
 * @param fld_player FIELD_PLAYER
 * @param form PLAYER_MOVE_FORM
 * @retval nothing
 *
 * FIELD_PLAYER_ChangeFormWait()で待つこと
 */
//--------------------------------------------------------------
void FIELD_PLAYER_ChangeFormRequest( FIELD_PLAYER *fld_player, PLAYER_DRAW_FORM form )
{
  FIELD_PLAYER_CORE_ChangeFormRequest( fld_player->corewk, form );
}

//--------------------------------------------------------------
/**
 * 自機の指定フォーム変更を待つ
 * @param fld_player FIELD_PLAYER
 * @param form PLAYER_MOVE_FORM
 * @retval nothing
 *
 * FIELD_PLAYER_ChangeFormRequest()とセット
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_ChangeFormWait( FIELD_PLAYER *fld_player )
{
  return FIELD_PLAYER_CORE_ChangeFormWait( fld_player->corewk );
}

//--------------------------------------------------------------
/**
 * 自機波乗りアトリビュートチェック
 * @param fld_player
 * @param nattr
 * @param fattr
 * @retval BOOL TRUE=波乗り可能アトリビュートである。
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_CheckAttrNaminori(
    FIELD_PLAYER *fld_player, MAPATTR nattr, MAPATTR fattr )
{
  MAPATTR_FLAG attr_flag = MAPATTR_GetAttrFlag( fattr );
  MAPATTR_VALUE f_val = MAPATTR_GetAttrValue( fattr );
  
  if( ((attr_flag&MAPATTR_FLAGBIT_WATER) &&
      MAPATTR_GetHitchFlag(fattr) == FALSE) ||
    MAPATTR_VALUE_CheckShore(f_val) == TRUE ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 自機が特殊表示の場合は元に戻す。
 * @param fld_player
 * @retval nothing
 * @note 今の所、該当箇所は尾瀬ゆれ状態のみ
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CheckSpecialDrawForm(
    FIELD_PLAYER *fld_player, BOOL menu_open_flag )
{
  if( FIELDMAP_GetBaseSystemType(
        fld_player->fieldWork) == FLDMAP_BASESYS_GRID ){
    FIELD_PLAYER_GRID_CheckSpecialDrawForm( fld_player->gridwk, menu_open_flag );
  }
  else
  {
    OS_Printf( "rail not support\n" );
    GF_ASSERT(0);
  }
}


//======================================================================
//	Grid 専用処理
//======================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  GRIDワークをセットアップする
 *
 *	@param	fld_player  フィールドワーク
 *	@param	heapID      ヒープID
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_SetUpGrid( FIELD_PLAYER *fld_player, HEAPID heapID )
{
  GF_ASSERT( fld_player->gridwk == NULL );
  fld_player->gridwk = FIELD_PLAYER_GRID_Init( fld_player->corewk, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  GRIDワークの更新処理
 *
 *	@param	fld_player  フィールドワーク
 *	@param	key_trg     トリガ
 *	@param	key_cont    コント
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_MoveGrid( FIELD_PLAYER *fld_player, int key_trg, int key_cont )
{
  GF_ASSERT( fld_player->gridwk );
  FIELD_PLAYER_GRID_Move( fld_player->gridwk, key_trg, key_cont );
}


//======================================================================
//	NOGrid 専用処理
//======================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  NOGRIDワークをセットアップする
 *
 *	@param	fld_player  フィールドワーク
 *	@param	heapID      ヒープID
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_SetUpNoGrid( FIELD_PLAYER *fld_player, HEAPID heapID )
{
  GF_ASSERT( fld_player->nogridwk == NULL );
  fld_player->nogridwk = FIELD_PLAYER_NOGRID_Create( fld_player->corewk, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  NOGRIDワークの更新処理
 *
 *	@param	fld_player  フィールドワーク
 *	@param	key_trg     トリガ
 *	@param	key_cont    コント
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_MoveNoGrid( FIELD_PLAYER *fld_player, int key_trg, int key_cont )
{
  GF_ASSERT( fld_player->nogridwk );
  FIELD_PLAYER_NOGRID_Move( fld_player->nogridwk, key_trg, key_cont );
}

//----------------------------------------------------------------------------
/**
 *	@brief  NOGRIDワーク動作再始動
 *
 *	@param	fld_player  フィールドワーク
 *	@param	heapID      ヒープID
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_RestartNoGrid( FIELD_PLAYER *fld_player, const RAIL_LOCATION* cp_pos )
{
  GF_ASSERT( fld_player->nogridwk );
  FIELD_PLAYER_NOGRID_Restart( fld_player->nogridwk, cp_pos );
}

//----------------------------------------------------------------------------
/**
 *	@brief  NOGRIDワーク動作停止
 *
 *	@param	fld_player    フィールドワーク
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_StopNoGrid( FIELD_PLAYER *fld_player )
{
  GF_ASSERT( fld_player->nogridwk );
  FIELD_PLAYER_NOGRID_Stop( fld_player->nogridwk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  NOGRIDワーク　レールワークの取得
 *
 *	@param	fld_player  フィールドプレイヤー
 *
 *	@return レールワーク
 */
//-----------------------------------------------------------------------------
FIELD_RAIL_WORK* FIELD_PLAYER_GetNoGridRailWork( const FIELD_PLAYER *fld_player )
{
  GF_ASSERT( fld_player->nogridwk );
  return FIELD_PLAYER_NOGRID_GetRailWork( fld_player->nogridwk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  NOGRID　レールロケーションを設定
 *
 *	@param	p_player
 *	@param	cp_location 
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_SetNoGridLocation( FIELD_PLAYER* p_player, const RAIL_LOCATION* cp_location )
{
  GF_ASSERT( p_player->nogridwk );
  FIELD_PLAYER_NOGRID_SetLocation( p_player->nogridwk, cp_location );
}

//----------------------------------------------------------------------------
/**
 *	@brief  NOGRID　レールロケーションを取得
 *
 *	@param	cp_player     プレイヤー
 *	@param	p_location    ロケーション格納先
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_GetNoGridLocation( const FIELD_PLAYER* cp_player, RAIL_LOCATION* p_location )
{
  GF_ASSERT( cp_player->nogridwk );
  FIELD_PLAYER_NOGRID_GetLocation( cp_player->nogridwk, p_location );
}

//----------------------------------------------------------------------------
/**
 *	@brief  NOGRIDワーク  レールワークの座標を取得
 *
 *	@param	cp_player プレイヤーワーク
 *	@param	p_pos     座標格納先
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_GetNoGridPos( const FIELD_PLAYER* cp_player, VecFx32* p_pos )
{
  GF_ASSERT( cp_player->nogridwk );
  FIELD_PLAYER_NOGRID_GetPos( cp_player->nogridwk, p_pos );
}

//-----------------------------------------------------------------------------
// コアワークの取得
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  フィールドプレイヤーからコアワークを取得
 *
 *	@param	p_player    フィールドプレイヤー
 *
 *	@return コアワーク
 */
//-----------------------------------------------------------------------------
FIELD_PLAYER_CORE * FIELD_PLAYER_GetCoreWk( FIELD_PLAYER* p_player )
{
  return p_player->corewk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドプレイヤーからグリッドプレイヤーワーク
 *
 *	@param	fld_player  フィールドプレイヤー 
 *
 *	@return グリッドプレイヤーワーク
 */
//-----------------------------------------------------------------------------
FIELD_PLAYER_GRID* FIELD_PLAYER_GetGridWk( FIELD_PLAYER* fld_player )
{
  return fld_player->gridwk;
}






//======================================================================
//	data
//======================================================================

//--------------------------------------------------------------
/// 性別、OBJコード、各フォーム
//--------------------------------------------------------------
static const OBJCODE_FORM dataOBJCodeForm[2][PLAYER_DRAW_FORM_MAX] =
{
  { //男主人公
    {HERO,PLAYER_MOVE_FORM_NORMAL},
    {CYCLEHERO,PLAYER_MOVE_FORM_CYCLE},
    {SWIMHERO,PLAYER_MOVE_FORM_SWIM},
    {GETHERO,PLAYER_MOVE_FORM_MAX},
    {REPORTHERO,PLAYER_MOVE_FORM_MAX},
    {PCHERO,PLAYER_MOVE_FORM_MAX},
    {HURAHERO,PLAYER_MOVE_FORM_MAX},
    {FISHHERO,PLAYER_MOVE_FORM_MAX},
    {CUTINHERO,PLAYER_MOVE_FORM_MAX},
  },
  { //女主人公
    {HEROINE,PLAYER_MOVE_FORM_NORMAL},
    {CYCLEHEROINE,PLAYER_MOVE_FORM_CYCLE},
    {SWIMHEROINE,PLAYER_MOVE_FORM_SWIM},
    {GETHEROINE,PLAYER_MOVE_FORM_MAX},
    {REPORTHEROINE,PLAYER_MOVE_FORM_MAX},
    {PCHEROINE,PLAYER_MOVE_FORM_MAX},
    {HURAHEROINE,PLAYER_MOVE_FORM_MAX},
    {FISHHEROINE,PLAYER_MOVE_FORM_MAX},
    {CUTINHEROINE,PLAYER_MOVE_FORM_MAX},
  },
};

//======================================================================
//	以下消します
//======================================================================
#if 0
GFL_BBDACT_RESUNIT_ID GetPlayerBBdActResUnitID( FIELD_PLAYER *fld_player )
{
	return( fld_player->bbdActResUnitID );
}
#endif

#if 0
void PlayerActGrid_Update(
	FIELD_PLAYER *fld_player, u16 dir, const VecFx32 *pos )
{
	VecFx32 trans;
	fld_player->pos = *pos;
	
	SetGridPlayerActTrans( fld_player, pos );
	MMDL_SetForceDirDisp( fld_player->fldmmdl, dir );
}
#endif

#if 0
void SetGridPlayerActTrans( FIELD_PLAYER* fld_player, const VecFx32* trans )
{
	int gx = SIZE_GRID_FX32( trans->x );
	int gy = SIZE_GRID_FX32( trans->y );
	int gz = SIZE_GRID_FX32( trans->z );
	
	MMDL_SetOldGridPosX( fld_player->fldmmdl,
		MMDL_GetGridPosX(fld_player->fldmmdl) );
	MMDL_SetOldGridPosY( fld_player->fldmmdl,
		MMDL_GetGridPosY(fld_player->fldmmdl) );
	MMDL_SetOldGridPosZ( fld_player->fldmmdl,
		MMDL_GetGridPosZ(fld_player->fldmmdl) );
	
	MMDL_SetGridPosX( fld_player->fldmmdl, gx );
	MMDL_SetGridPosY( fld_player->fldmmdl, gy );
	MMDL_SetGridPosZ( fld_player->fldmmdl, gz );
	MMDL_SetVectorPos( fld_player->fldmmdl, trans );
	
	VEC_Set( &fld_player->pos, trans->x, trans->y, trans->z );
}
#endif

#if 0
void PlayerActGrid_AnimeSet(
	FIELD_PLAYER *fld_player, int dir, PLAYER_ANIME_FLAG flag )
{
	switch( flag ){
	case PLAYER_ANIME_FLAG_STOP:
		MMDL_SetDrawStatus( fld_player->fldmmdl, DRAW_STA_STOP );
		break;
	case PLAYER_ANIME_FLAG_WALK:
		MMDL_SetDrawStatus( fld_player->fldmmdl, DRAW_STA_WALK_8F );
		break;
	default:
		MMDL_SetDrawStatus( fld_player->fldmmdl, DRAW_STA_WALK_4F );
		break;
	}
}
#endif

#if 0
FLDMAPPER_GRIDINFODATA * PlayerActGrid_GridInfoGet( FIELD_PLAYER *fld_player )
{
	return( &fld_player->gridInfoData );
}
#endif

#if 0
void PlayerActGrid_ScaleSizeSet(
	FIELD_PLAYER *fld_player, fx16 sizeX, fx16 sizeY )
{
	GFL_BBDACT_SYS *bbdActSys = FIELDMAP_GetBbdActSys( fld_player->fieldWork );
	int idx = GFL_BBDACT_GetBBDActIdxResIdx(
			bbdActSys, fld_player->bbdActActUnitID );
	GFL_BBD_SetObjectSiz(
		GFL_BBDACT_GetBBDSystem(bbdActSys),
		idx, &sizeX, &sizeY );
}
#endif

#if 0
void PLAYER_GRID_GetFrontGridPos(
	FIELD_PLAYER *fld_player, int *gx, int *gy, int *gz )
{
	int dir;
	MMDL *fmmdl = FIELD_PLAYER_GetMMdl( fld_player );
	
	*gx = MMDL_GetGridPosX( fmmdl );
	*gy = MMDL_GetGridPosY( fmmdl );
	*gz = MMDL_GetGridPosZ( fmmdl );
	dir = MMDL_GetDirDisp( fmmdl );
	
	*gx += MMDL_TOOL_GetDirAddValueGridX( dir );
	*gz += MMDL_TOOL_GetDirAddValueGridZ( dir );
}
#endif
