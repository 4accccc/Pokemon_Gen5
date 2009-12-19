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

#include "field_player_grid.h"

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
	HEAPID heapID;
  GAMESYS_WORK *gsys;
	FIELDMAP_WORK *fieldWork;
  PLAYER_WORK *playerWork;
   
#if 0 //PLAYER_WORKへ移動
  PLAYER_MOVE_FORM move_form;
#endif
  
  PLAYER_MOVE_STATE move_state;
  PLAYER_MOVE_VALUE move_value;
	
  int sex; //性別
  
	u16 dir;
	u16 padding0;
	
  VecFx32 pos;
  
	MMDL *fldmmdl;
	FLDMAPPER_GRIDINFODATA gridInfoData;
};

//======================================================================
//	proto
//======================================================================
static void fldplayer_ChangeMoveForm(
    FIELD_PLAYER *fld_player, PLAYER_MOVE_FORM form );

static const MMDL_HEADER data_MMdlHeader;
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
	MMDLSYS *fmmdlsys;
	FIELD_PLAYER *fld_player;
	
	fld_player = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_PLAYER) );
  fld_player->playerWork = playerWork;
	fld_player->fieldWork = fieldWork;
  fld_player->gsys = FIELDMAP_GetGameSysWork( fieldWork );
	fld_player->pos = *pos;

	FLDMAPPER_GRIDINFODATA_Init( &fld_player->gridInfoData );
	
	//MMDLセットアップ
	fmmdlsys = FIELDMAP_GetMMdlSys( fieldWork );
	
	fld_player->fldmmdl =
		MMDLSYS_SearchOBJID( fmmdlsys, MMDL_ID_PLAYER );
  
  fld_player->sex = sex;
  
	if( fld_player->fldmmdl == NULL )	//新規
	{
		MMDL_HEADER head;
    MMDL_HEADER_GRIDPOS *gridpos;
		head = data_MMdlHeader;
    gridpos = (MMDL_HEADER_GRIDPOS *)head.pos_buf;
		gridpos->gx = SIZE_GRID_FX32( pos->x );
		gridpos->gz = SIZE_GRID_FX32( pos->z );
		gridpos->y = pos->y;
    head.obj_code = FIELD_PLAYER_GetMoveFormToOBJCode(
        sex, PLAYER_MOVE_FORM_NORMAL ); 
		fld_player->fldmmdl = MMDLSYS_AddMMdl( fmmdlsys, &head, 0 );
	}
	else //復帰
	{
		int gx = SIZE_GRID_FX32( pos->x );
		int gy = SIZE_GRID_FX32( pos->y );
		int gz = SIZE_GRID_FX32( pos->z );
		MMDL *fmmdl = fld_player->fldmmdl;
		
		MMDL_SetGridPosX( fmmdl, gx );
		MMDL_SetGridPosY( fmmdl, gy );
		MMDL_SetGridPosZ( fmmdl, gz );
		MMDL_SetVectorPos( fmmdl, pos );
    
    //レポート等のイベント用OBJの場合、動作フォームに合わせて復帰
    if( PLAYERWORK_GetFlagOBJCodeFix(fld_player->playerWork) == FALSE ){
      FIELD_PLAYER_ResetMoveForm( fld_player );
    }
	}
	
  { //OBJコードから動作フォームを設定
    u16 code = MMDL_GetOBJCode( fld_player->fldmmdl );
    PLAYER_MOVE_FORM form = FIELD_PLAYER_GetOBJCodeToMoveForm( sex, code );
#if 0
    fld_player->move_form = form;
#else
    PLAYERWORK_SetMoveForm( fld_player->playerWork, form );
#endif
  }
  
	MMDL_SetStatusBitNotZoneDelete( fld_player->fldmmdl, TRUE );
	return( fld_player );
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
  //プレイヤー座標をPLAYERWORKへ反映
  VecFx32 pos;
  FIELD_PLAYER_GetPos( fld_player, &pos );
	PLAYERWORK_setPosition( fld_player->playerWork, &pos );
  
  //方向をPLAYERWORKへ反映
  {
    u16 dir = FIELD_PLAYER_GetDir( fld_player );
    PLAYERWORK_setDirection_Type( fld_player->playerWork, dir );
  }
  
  // レール独自の更新処理
  if( FIELDMAP_GetBaseSystemType(fld_player->fieldWork) == FLDMAP_BASESYS_GRID )
  {
	  PLAYERWORK_setPosType( fld_player->playerWork, LOCATION_POS_TYPE_3D );
  }
  else
  {
    RAIL_LOCATION location;
    MMDL_GetRailLocation( fld_player->fldmmdl, &location );
	  PLAYERWORK_setRailPosition( fld_player->playerWork, &location );
	  PLAYERWORK_setPosType( fld_player->playerWork, LOCATION_POS_TYPE_RAIL );
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
        fld_player, key_trg, key_cont, evbit );
  }
  
  return( event );
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
  MMDL *fmmdl = fld_player->fldmmdl;
  PLAYER_MOVE_VALUE value = fld_player->move_value;
  PLAYER_MOVE_STATE state = fld_player->move_state;
  
  fld_player->move_state = PLAYER_MOVE_STATE_OFF;
  
  //グリッド限定　各種強制移動チェック
  if( FIELDMAP_GetBaseSystemType(
        fld_player->fieldWork) == FLDMAP_BASESYS_GRID )
  {
    //足元強制移動チェック
    if( FIELD_PLAYER_GRID_CheckUnderForceMove(fld_player) == TRUE )
    {
      fld_player->move_state = PLAYER_MOVE_STATE_ON;
      return;
    }
    
    //尾瀬落下チェック
    if( FIELD_PLAYER_GRID_CheckOzeFallOut(fld_player) == TRUE )
    {
      fld_player->move_state = PLAYER_MOVE_STATE_ON;
      return;
    }
  }

  if( MMDL_CheckPossibleAcmd(fmmdl) == FALSE ) //動作中
  {
    switch( value )
    {
    case PLAYER_MOVE_VALUE_STOP:
      break;
    case PLAYER_MOVE_VALUE_WALK:
      if( state == PLAYER_MOVE_STATE_OFF || state == PLAYER_MOVE_STATE_END )
      {
        fld_player->move_state = PLAYER_MOVE_STATE_START;
      }
      else
      {
        fld_player->move_state = PLAYER_MOVE_STATE_ON;
      }
      break;
    case PLAYER_MOVE_VALUE_TURN:
      fld_player->move_state = PLAYER_MOVE_STATE_ON;
      break;
    }
    return;
  }
  
  if( MMDL_CheckEndAcmd(fmmdl) == TRUE ) //動作終了
  {
    switch( value )
    {
    case PLAYER_MOVE_VALUE_STOP:
      break;
    case PLAYER_MOVE_VALUE_WALK:
      switch( state )
      {
      case PLAYER_MOVE_STATE_OFF:
        break;
      case PLAYER_MOVE_STATE_END:
        fld_player->move_state = PLAYER_MOVE_STATE_OFF;
        break;
      default:
        fld_player->move_state = PLAYER_MOVE_STATE_END;
      }
      break;
    case PLAYER_MOVE_VALUE_TURN:
      switch( state )
      {
      case PLAYER_MOVE_STATE_OFF:
        break;
      case PLAYER_MOVE_STATE_END:
        fld_player->move_state = PLAYER_MOVE_STATE_OFF;
        break;
      default:
        fld_player->move_state = PLAYER_MOVE_STATE_END;
      }
      break;
    }
    return;
  }
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
#if 0
	*pos = fld_player->pos;
#else //表示座標となるアクターから直に取得
  MMDL_GetVectorPos( fld_player->fldmmdl, pos );
#endif
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
	int gx = SIZE_GRID_FX32( pos->x );
	int gy = SIZE_GRID_FX32( pos->y );
	int gz = SIZE_GRID_FX32( pos->z );
	MMDL *fmmdl = fld_player->fldmmdl;
	
	MMDL_SetOldGridPosX( fmmdl, MMDL_GetGridPosX(fmmdl) );
	MMDL_SetOldGridPosY( fmmdl, MMDL_GetGridPosY(fmmdl) );
	MMDL_SetOldGridPosZ( fmmdl, MMDL_GetGridPosZ(fmmdl) );
	MMDL_SetGridPosX( fmmdl, gx );
	MMDL_SetGridPosY( fmmdl, gy );
	MMDL_SetGridPosZ( fmmdl, gz );
	MMDL_SetVectorPos( fmmdl, pos );
	
	fld_player->pos = *pos;
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
#if 0
	return( fld_player->dir );
#else //表示するアクターから直に取得
  return( MMDL_GetDirDisp(fld_player->fldmmdl) );
#endif
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
#if 0
	fld_player->dir = dir;
#else //表示するアクターへ直にセット
  MMDL_SetDirDisp( fld_player->fldmmdl, dir );
#endif
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
	return( fld_player->fieldWork );
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
	return( fld_player->fldmmdl );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER FLDMAPPER_GRIDINFODATA取得
 * @param fld_player* FIELD_PLAYER
 * @retval FLDMAPPER_GRIDINFODATA*
 */
//--------------------------------------------------------------
FLDMAPPER_GRIDINFODATA * FIELD_PLAYER_GetGridInfoData(
		FIELD_PLAYER *fld_player )
{
	return( &fld_player->gridInfoData );
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
  fld_player->move_value = val;
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
  return( fld_player->move_value );
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
  return( fld_player->move_state );
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
  PLAYER_MOVE_FORM form = PLAYERWORK_GetMoveForm( fld_player->playerWork );
  return( form );
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
  PLAYERWORK_SetMoveForm( fld_player->playerWork, form );
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
  return( fld_player->sex );
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
  return( fld_player->gsys );
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
 * OBJコードからPLAYER_DRAW_FORM取得　チェック用
 * @param sex PM_MALE,PM_FEMALE
 * @param code OBJコード
 * @retval PLAYER_DRAW_FORM
 */
//--------------------------------------------------------------
static PLAYER_DRAW_FORM fld_player_CheckOBJCodeToDrawForm( u16 code )
{
  int sex,i;
  const OBJCODE_FORM *tbl;
  
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
 * 自機をレポートやアイテムゲットなどイベント用表示に変えても問題ないか
 * @param fld_player FIELD_PLAYER
 * @retval  BOOL TRUE=OK
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_CheckChangeEventDrawForm( FIELD_PLAYER *fld_player )
{
  PLAYER_MOVE_FORM form = FIELD_PLAYER_GetMoveForm( fld_player );
  
  if( FLDMAP_BASESYS_RAIL ==
      FIELDMAP_GetBaseSystemType(fld_player->fieldWork) ){
    return( FALSE );
  }
  
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
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
  MMDL_ChangeOBJCode( mmdl, code );
  PLAYERWORK_SetFlagOBJCodeFix( fld_player->playerWork, TRUE );
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
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
  u16 code = MMDL_GetOBJCode( mmdl );
  if( fld_player_CheckOBJCodeToDrawForm(code) != PLAYER_DRAW_FORM_MAX ){
    return( TRUE );
  }
  return( FALSE );
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
    MMDL_GetRailLocation( fld_player->fldmmdl, &location );
    FIELD_RAIL_MAN_GetLocationPosition( p_railman, &location, &now_pos );
    MMDL_Rail_GetDirLineWay( fld_player->fldmmdl, dir, &way );

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

    MMDL_Rail_GetDirLineWay( fld_player->fldmmdl, dir, &way16 );
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
  const MMDL *fmmdl = FIELD_PLAYER_GetMMdl( fld_player );
  
  if( fmmdl == NULL ){
    return( FALSE );
  }
   
  if( MMDL_CheckStatusBitUse(fmmdl) == FALSE ){
    return( FALSE );
  }
  
  if( MMDL_GetOBJID(fmmdl) != MMDL_ID_PLAYER ){
    return( FALSE );
  }
  
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * 自機の動作形態を変更
 * @param fld_player FIELD_PLAYER
 * @param form PLAYER_MOVE_FORM
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldplayer_ChangeMoveForm(
    FIELD_PLAYER *fld_player, PLAYER_MOVE_FORM form )
{
  int sex = FIELD_PLAYER_GetSex( fld_player );
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
  u16 code = FIELD_PLAYER_GetMoveFormToOBJCode( sex, form );
  
  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
  
  FIELD_PLAYER_SetMoveForm( fld_player, form );
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
  fldplayer_ChangeMoveForm( fld_player, form );
  
  {
    FIELDMAP_WORK *fieldWork = FIELD_PLAYER_GetFieldMapWork( fld_player );
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( fld_player->gsys );
    FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gdata );
    u32 zone_id = FIELDMAP_GetZoneID( fieldWork );
    FIELD_SOUND_ChangePlayZoneBGM( fsnd, gdata, form, zone_id );
  }
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
  PLAYER_MOVE_FORM form = FIELD_PLAYER_GetMoveForm( fld_player );
  fldplayer_ChangeMoveForm( fld_player, form );
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
  int sex = FIELD_PLAYER_GetSex( fld_player );
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
  u16 code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, form );
  
  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
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
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );
  return fld_player_CheckOBJCodeToDrawForm( MMDL_GetOBJCode( mmdl ));
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
    FIELD_PLAYER_GRID_CheckSpecialDrawForm( fld_player, menu_open_flag );
  }
}

//======================================================================
//	data
//======================================================================
//--------------------------------------------------------------
/// 自機動作モデルヘッダー
//--------------------------------------------------------------
static const MMDL_HEADER data_MMdlHeader =
{
	MMDL_ID_PLAYER,	///<識別ID
	HERO,	///<表示するOBJコード
	MV_DMY,	///<動作コード
	0,	///<イベントタイプ
	0,	///<イベントフラグ
	0,	///<イベントID
	0,	///<指定方向
	0,	///<指定パラメタ 0
	0,	///<指定パラメタ 1
	0,	///<指定パラメタ 2
	MOVE_LIMIT_NOT,	///<X方向移動制限
	MOVE_LIMIT_NOT,	///<Z方向移動制限
  MMDL_HEADER_POSTYPE_GRID,
};

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
