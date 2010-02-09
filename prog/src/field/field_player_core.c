//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_player_core.c
 *	@brief  フィールドプレイヤー制御　コア情報
 *	@author	gamefreak
 *	@date		2010.02.05
 *
 *	モジュール名：FIELD_PLAYER_CORE
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"


#include "field/field_const.h"

#include "fieldmap.h"
#include "field_g3d_mapper.h"
#include "fldmmdl.h"
#include "field_sound.h"
#include "fldeff_namipoke.h"
#include "fldeff_kemuri.h"

#include "field_player_core.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	FIELD_PLAYER_CORE
//=====================================
struct _FIELD_PLAYER_CORE 
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
	MMDL *fldmmdl;


  // キー入力処理
  u16 input_key_dir_x; //キー入力横方向
  u16 input_key_dir_z; //キー入力縦方向

  // REQ系メンバ
  FIELD_PLAYER_REQBIT req_bit;

  // EffectTask
  FLDEFF_TASK *fldeff_joint;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static void fldplayer_ChangeMoveForm(
    FIELD_PLAYER_CORE *player_core, PLAYER_MOVE_FORM form );

//-------------------------------------
///	キー入力処理
//=====================================
static u16 gjiki_GetInputKeyDir(
    const FIELD_PLAYER_CORE *player_core, u16 key_prs );
static void gjiki_SetInputKeyDir( FIELD_PLAYER_CORE *player_core, u16 key_prs );
static u16 getKeyDirX( u16 key_prs );
static u16 getKeyDirZ( u16 key_prs );



//-------------------------------------
///	Req系
//=====================================
static void gjikiReq_SetNormal( FIELD_PLAYER_CORE *player_core );
static void gjikiReq_SetCycle( FIELD_PLAYER_CORE *player_core );
static void gjikiReq_SetSwim( FIELD_PLAYER_CORE *player_core );
static void gjikiReq_SetMoveFormToDrawForm( FIELD_PLAYER_CORE *player_core );
static void gjikiReq_SetItemGetHero( FIELD_PLAYER_CORE *player_core );
static void gjikiReq_SetReportHero( FIELD_PLAYER_CORE *player_core );
static void gjikiReq_SetPCAzukeHero( FIELD_PLAYER_CORE *player_core );
static void gjikiReq_SetCutInHero( FIELD_PLAYER_CORE *player_core );
static void gjiki_PlayBGM( FIELD_PLAYER_CORE *player_core );


//----------------------------------------------------------------------------
/**
 * フィールドプレイヤー　作成
 * @param playerWork 使用するPLAYER_WOKR
 * @param *fieldWork 参照するFIELDMAP_WORK
 * @param pos 初期座標
 * @param sex 性別
 * @param heapID HEAPID
 * @retval FIELD_PLAYER_CORE*
 */
//-----------------------------------------------------------------------------
FIELD_PLAYER_CORE * FIELD_PLAYER_CORE_Create(
    PLAYER_WORK *playerWork, FIELDMAP_WORK *fieldWork,
		const VecFx32 *pos, int sex, HEAPID heapID )
{
  u16 fixcode;
	MMDLSYS *fmmdlsys;
	FIELD_PLAYER_CORE *player_core;
	
	player_core = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_PLAYER_CORE) );
  player_core->playerWork = playerWork;
	player_core->fieldWork = fieldWork;
  player_core->gsys = FIELDMAP_GetGameSysWork( fieldWork );

  player_core->input_key_dir_x = DIR_NOT;
  player_core->input_key_dir_z = DIR_NOT;

	//MMDLセットアップ
	fmmdlsys = FIELDMAP_GetMMdlSys( fieldWork );
	
	player_core->fldmmdl =
		MMDLSYS_SearchOBJID( fmmdlsys, MMDL_ID_PLAYER );
  
  player_core->sex = sex;
  
  fixcode = PLAYERWORK_GetOBJCodeFix( player_core->playerWork );

	if( player_core->fldmmdl == NULL )	//新規
	{
		MMDL_HEADER head;
    MMDL_HEADER_GRIDPOS *gridpos;
		head = data_MMdlHeader;
    gridpos = (MMDL_HEADER_GRIDPOS *)head.pos_buf;
		gridpos->gx = SIZE_GRID_FX32( pos->x );
		gridpos->gz = SIZE_GRID_FX32( pos->z );
		gridpos->y = pos->y;
    
    if( fixcode == OBJCODEMAX )
    {
      head.obj_code = FIELD_PLAYER_GetMoveFormToOBJCode(
          sex, PLAYER_MOVE_FORM_NORMAL ); 
    }
    else
    {
      head.obj_code = fixcode;
    }

		player_core->fldmmdl = MMDLSYS_AddMMdl( fmmdlsys, &head, 0 );
	}
	else //復帰
	{
		int gx = SIZE_GRID_FX32( pos->x );
		int gy = SIZE_GRID_FX32( pos->y );
		int gz = SIZE_GRID_FX32( pos->z );
		MMDL *fmmdl = player_core->fldmmdl;
		
		MMDL_SetGridPosX( fmmdl, gx );
		MMDL_SetGridPosY( fmmdl, gy );
		MMDL_SetGridPosZ( fmmdl, gz );
		MMDL_SetVectorPos( fmmdl, pos );
    
    //レポート等のイベント用OBJの場合、動作フォームに合わせて復帰
    if( fixcode == OBJCODEMAX ){
      FIELD_PLAYER_CORE_ResetMoveForm( player_core );
    }
	}
	
  //OBJコードから動作フォームを設定
  if( fixcode == OBJCODEMAX ){
    u16 code = MMDL_GetOBJCode( player_core->fldmmdl );
    PLAYER_MOVE_FORM form = FIELD_PLAYER_GetOBJCodeToMoveForm( sex, code );
#if 0
    player_core->move_form = form;
#else
    PLAYERWORK_SetMoveForm( player_core->playerWork, form );
#endif
  }
  
	MMDL_SetStatusBitNotZoneDelete( player_core->fldmmdl, TRUE );
	return( player_core );
}

//--------------------------------------------------------------
/**
 * フィールドプレイヤー　削除
 * @param player_core FIELD_PLAYER_CORE
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_Delete( FIELD_PLAYER_CORE *player_core )
{
	//動作モデルの削除はフィールドメイン側で
	GFL_HEAP_FreeMemory( player_core );
}

//--------------------------------------------------------------
/**
 * フィールドプレイヤー　動作ステータス更新
 * @param player_core
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_UpdateMoveStatus( FIELD_PLAYER_CORE *player_core )
{
  MMDL *fmmdl = player_core->fldmmdl;
  PLAYER_MOVE_VALUE value = player_core->move_value;
  PLAYER_MOVE_STATE state = player_core->move_state;
  
  player_core->move_state = PLAYER_MOVE_STATE_OFF;
  
  if( MMDL_CheckPossibleAcmd(fmmdl) == FALSE ) //動作中
  {
    switch( value )
    {
    case PLAYER_MOVE_VALUE_STOP:
      break;
    case PLAYER_MOVE_VALUE_WALK:
      if( state == PLAYER_MOVE_STATE_OFF || state == PLAYER_MOVE_STATE_END )
      {
        player_core->move_state = PLAYER_MOVE_STATE_START;
      }
      else
      {
        player_core->move_state = PLAYER_MOVE_STATE_ON;
      }
      break;
    case PLAYER_MOVE_VALUE_TURN:
      player_core->move_state = PLAYER_MOVE_STATE_ON;
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
        player_core->move_state = PLAYER_MOVE_STATE_OFF;
        break;
      default:
        player_core->move_state = PLAYER_MOVE_STATE_END;
      }
      break;
    case PLAYER_MOVE_VALUE_TURN:
      switch( state )
      {
      case PLAYER_MOVE_STATE_OFF:
        break;
      case PLAYER_MOVE_STATE_END:
        player_core->move_state = PLAYER_MOVE_STATE_OFF;
        break;
      default:
        player_core->move_state = PLAYER_MOVE_STATE_END;
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
 * @param player_core FIELD_PLAYER_CORE
 * @param pos 座標格納先
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_GetPos(
		const FIELD_PLAYER_CORE *player_core, VecFx32 *pos )
{
  MMDL_GetVectorPos( player_core->fldmmdl, pos );
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER　座標セット
 * @param player_core FIELD_PLAYER_CORE
 * @param pos セットする座標
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_SetPos(
		FIELD_PLAYER_CORE *player_core, const VecFx32 *pos )
{
	int gx = SIZE_GRID_FX32( pos->x );
	int gy = SIZE_GRID_FX32( pos->y );
	int gz = SIZE_GRID_FX32( pos->z );
  MMDL* fmmdl = player_core->fldmmdl;
	
	MMDL_SetOldGridPosX( fmmdl, MMDL_GetGridPosX(fmmdl) );
	MMDL_SetOldGridPosY( fmmdl, MMDL_GetGridPosY(fmmdl) );
	MMDL_SetOldGridPosZ( fmmdl, MMDL_GetGridPosZ(fmmdl) );
	MMDL_SetGridPosX( fmmdl, gx );
	MMDL_SetGridPosY( fmmdl, gy );
	MMDL_SetGridPosZ( fmmdl, gz );
	MMDL_SetVectorPos( fmmdl, pos );
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER　方向取得
 * @param player_core FIELD_PLAYER_CORE
 * @retval u16 方向
 */
//--------------------------------------------------------------
u16 FIELD_PLAYER_CORE_GetDir( const FIELD_PLAYER_CORE *player_core )
{
  return MMDL_GetDirDisp(player_core->fldmmdl);
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER　方向セット
 * @param player_core FIELD_PLAYER_CORE
 * @param dir 方向
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_SetDir( FIELD_PLAYER_CORE *player_core, u16 dir )
{
  MMDL_SetDirDisp( player_core->fldmmdl, dir );
}

//--------------------------------------------------------------
/**
 * 自機の表示コードを直接変更
 * @param player_core FIELD_PLAYER_CORE
 * @param code 表示コード HERO等
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_ChangeOBJCode( FIELD_PLAYER_CORE *player_core, u16 code )
{
  MMDL_ChangeOBJCode( player_core->fldmmdl, code );
  PLAYERWORK_SetOBJCodeFix( player_core->playerWork, code );
}


//--------------------------------------------------------------
/**
 * 自機の表示コード固定を解除
 * @param player_core FIELD_PLAYER_CORE
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_ClearOBJCodeFix( FIELD_PLAYER_CORE *player_core )
{
  PLAYERWORK_SetOBJCodeFix( player_core->playerWork, OBJCODEMAX );
}





//--------------------------------------------------------------
/**
 * 自機リクエスト 
 * @param player_core FIELD_PLAYER_CORE
 * @param reqbit FIELD_PLAYER_REQBIT
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_SetRequest(
  FIELD_PLAYER_CORE * player_core, FIELD_PLAYER_REQBIT req_bit )
{
  player_core->req_bit = req_bit;
}

//--------------------------------------------------------------
/**
 * リクエストを更新
 * @param player_core FIELD_PLAYER_CORE
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_UpdateRequest( FIELD_PLAYER_CORE * player_core )
{
  int i = 0;
  FIELD_PLAYER_REQBIT req_bit = player_core->req_bit;
  static void (* const data_RequestProcTbl[FIELD_PLAYER_REQBIT_MAX])( FIELD_PLAYER_CORE *player_core ) =
  {
    gjikiReq_SetNormal, //FIELD_PLAYER_REQBIT_NORMAL
    gjikiReq_SetCycle, //FIELD_PLAYER_REQBIT_CYCLE
    gjikiReq_SetSwim, //FIELD_PLAYER_REQBIT_SWIM
    gjikiReq_SetMoveFormToDrawForm,
    gjikiReq_SetItemGetHero,
    gjikiReq_SetReportHero,
    gjikiReq_SetPCAzukeHero,
    gjikiReq_SetCutInHero,
  };

  
  while( i < FIELD_PLAYER_REQBIT_MAX ){
    if( (req_bit&0x01) ){
      data_RequestProcTbl[i]( player_core );
    }
    req_bit >>= 1;
    i++;
  }
   
  player_core->req_bit = 0;
}



//--------------------------------------------------------------
/**
 * FILED_PLAYER_CORE　FIELDMAP_WORK取得
 * @param player_core FIELD_PLAYER_CORE
 * @retval FIELDMAP_WORK*
 */
//--------------------------------------------------------------
FIELDMAP_WORK * FIELD_PLAYER_CORE_GetFieldMapWork(
		FIELD_PLAYER_CORE *player_core )
{
	return( player_core->fieldWork );
}

//--------------------------------------------------------------
/**
 * FILED_PLAYER_CORE　MMDL取得
 * @param player_core FIELD_PLAYER_CORE
 * @retval MMDL*
 */
//--------------------------------------------------------------
MMDL * FIELD_PLAYER_CORE_GetMMdl( const FIELD_PLAYER_CORE *player_core )
{
	return( player_core->fldmmdl );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER PLAYER_MOVE_VALUEセット
 * @param player_core FIELD_PLAYER_CORE
 * @retval PLAYER_MOVE_STATE
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_SetMoveValue(
    FIELD_PLAYER_CORE *player_core, PLAYER_MOVE_VALUE val )
{
  player_core->move_value = val;
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER_CORE PLAYER_MOVE_VALUE取得
 * @param player_core FIELD_PLAYER_CORE
 * @retval PLAYER_MOVE_VALUE
 */
//--------------------------------------------------------------
PLAYER_MOVE_VALUE FIELD_PLAYER_CORE_GetMoveValue(
    const FIELD_PLAYER_CORE *player_core )
{
  return( player_core->move_value );
}

//----------------------------------------------------------------------------
/**
 * FIELD_PLAYER PLAYER_MOVE_STATEセット
 * @param player_core FIELD_PLAYER_CORE
 * @param val PLAYER_MOVE_STATE
 */
//-----------------------------------------------------------------------------
void FIELD_PLAYER_CORE_SetMoveState(
    FIELD_PLAYER_CORE *player_core, PLAYER_MOVE_STATE val )
{
  player_core->move_state = val;
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER_CORE PLAYER_MOVE_STATE取得
 * @param player_core FIELD_PLAYER_CORE
 * @retval PLAYER_MOVE_STATE_CORE
 */
//--------------------------------------------------------------
PLAYER_MOVE_STATE FIELD_PLAYER_CORE_GetMoveState(
    const FIELD_PLAYER_CORE *player_core )
{
  return( player_core->move_state );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER_CORE PLAYER_MOVE_FORM取得
 * @param player_core FIELD_PLAYER_CORE
 * @retval PLAYER_MOVE_FORM
 */
//--------------------------------------------------------------
PLAYER_MOVE_FORM FIELD_PLAYER_CORE_GetMoveForm(
    const FIELD_PLAYER_CORE *player_core )
{
  PLAYER_MOVE_FORM form = PLAYERWORK_GetMoveForm( player_core->playerWork );
  return( form );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER_CORE PLAYER_MOVE_FORMセット
 * @param player_core FIELD_PLAYER_CORE
 * @retval PLAYER_MOVE_FORM
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_SetMoveForm(
    FIELD_PLAYER_CORE *player_core, PLAYER_MOVE_FORM form )
{
  PLAYERWORK_SetMoveForm( player_core->playerWork, form );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER_CORE FIELD_PLAYERにセットされている性別を取得
 * @param player_core FIELD_PLAYER_CORE
 * @retval int PM_MALE,PM_FEMALE
 */
//--------------------------------------------------------------
int FIELD_PLAYER_CORE_GetSex( const FIELD_PLAYER_CORE *player_core )
{
  return( player_core->sex );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER_CORE GAMESYS_WORK取得
 * @param player_core FIELD_PLAYER_CORE
 * @retval GAMESYS_WORK*
 */
//--------------------------------------------------------------
GAMESYS_WORK * FIELD_PLAYER_CORE_GetGameSysWork( FIELD_PLAYER_CORE *player_core )
{
  return( player_core->gsys );
}

//----------------------------------------------------------------------------
/**
 *	@brief  プレイヤーワークの取得
 *
 *	@param	player_core  ワーク
 *
 *	@return プレイヤーワーク
 */
//-----------------------------------------------------------------------------
PLAYER_WORK * FIELD_PLAYER_CORE_GetPlayerWork( const FIELD_PLAYER_CORE * player_core )
{
  return (player_core->playerWork);
}

//----------------------------------------------------------------------------
/**
 * 動作モデルが生きているかチェック
 * @param player_core FIELD_PLAYER_CORE
 * @retval BOOL TRUE=生存
 */
//-----------------------------------------------------------------------------
BOOL FIELD_PLAYER_CORE_CheckLiveMMdl( const FIELD_PLAYER_CORE *player_core )
{
  const MMDL *fmmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  
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
 * 自機の動作形態を変更 BGM変更あり
 * @param player_core FIELD_PLAYER_CORE
 * @param form PLAYER_MOVE_FORM
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_ChangeMoveForm(
    FIELD_PLAYER_CORE *player_core, PLAYER_MOVE_FORM form )
{
  fldplayer_ChangeMoveForm( player_core, form );
  
  {
    FIELDMAP_WORK *fieldWork = player_core->fieldWork;
    GAMEDATA *gdata = GAMESYSTEM_GetGameData( player_core->gsys );
    FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gdata );
    u32 zone_id = FIELDMAP_GetZoneID( fieldWork );
    FSND_ChangeBGM_byPlayerFormChange( fsnd, gdata, zone_id );
  }
}

//--------------------------------------------------------------
/**
 * 自機の現在の動作状態で再セット
 * @param player_core FIELD_PLAYER_CORE
 * @param form PLAYER_MOVE_FORM
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_ResetMoveForm( FIELD_PLAYER_CORE *player_core )
{
  PLAYER_MOVE_FORM form = FIELD_PLAYER_CORE_GetMoveForm( player_core );
  fldplayer_ChangeMoveForm( player_core, form );
}

//--------------------------------------------------------------
/**
 * PLAYER_DRAW_FORMから自機のOBJコードのみ変更
 * @param player_core FIELD_PLAYER_CORE
 * @param form PLAYER_MOVE_FORM
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_ChangeDrawForm(
    FIELD_PLAYER_CORE *player_core, PLAYER_DRAW_FORM form )
{
  int sex = FIELD_PLAYER_CORE_GetSex( player_core );
  MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  u16 code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, form );
  
  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
}

//--------------------------------------------------------------
/**
 * 自機のDrawFormを取得
 * @param player_core FIELD_PLAYER_CORE
 * @retval PLAYER_DRAW_FORM
 */
//--------------------------------------------------------------
PLAYER_DRAW_FORM FIELD_PLAYER_CORE_GetDrawForm( FIELD_PLAYER_CORE *player_core )
{
  MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  return FIELD_PLAYER_CheckOBJCodeToDrawForm( MMDL_GetOBJCode( mmdl ));
}

//--------------------------------------------------------------
/**
 * 自機を指定フォームに変更リクエスト(アニメコール込み)
 * @param player_core FIELD_PLAYER_CORE
 * @param form PLAYER_MOVE_FORM
 * @retval nothing
 *
 * FIELD_PLAYER_ChangeFormWait()で待つこと
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_ChangeFormRequest( FIELD_PLAYER_CORE *player_core, PLAYER_DRAW_FORM form )
{
  u16 code;
  MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );

  FIELD_PLAYER_CORE_ChangeDrawForm( player_core, form );

  switch(form){
  case PLAYER_DRAW_FORM_CUTIN:
    code = AC_HERO_CUTIN;
    break;
  default:
    return;
  }
  MMDL_SetAcmd( mmdl, code );
}

//--------------------------------------------------------------
/**
 * 自機の指定フォーム変更を待つ
 * @param player_core FIELD_PLAYER_CORE
 * @param form PLAYER_MOVE_FORM
 * @retval nothing
 *
 * FIELD_PLAYER_ChangeFormRequest()とセット
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_CORE_ChangeFormWait( FIELD_PLAYER_CORE *player_core )
{
  MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  return MMDL_CheckEndAcmd( mmdl );
}

//--------------------------------------------------------------
/**
 * 自機の表示コードが自機専用のものかどうか
 * @param player_core FIELD_PLAYER
 * @retval BOOL TRUE=自機専用 FALSE=自機以外のコード
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_CORE_CheckIllegalOBJCode( FIELD_PLAYER_CORE *player_core )
{
  MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  u16 code = MMDL_GetOBJCode( mmdl );
  if( FIELD_PLAYER_CheckOBJCodeToDrawForm(code) != PLAYER_DRAW_FORM_MAX ){
    return( TRUE );
  }
  return( FALSE );
}


//======================================================================
//  移動チェック
//======================================================================
//--------------------------------------------------------------
/**
 * 自機オーダーチェック
 * @param player_core FIELD_PLAYER_GRID
 * @param dir 移動方向。DIR_UP等
 * @retval BOOL TRUE=移動可能 FALSE=移動不可
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_CORE_CheckStartMove(
    const FIELD_PLAYER_CORE * player_core, u16 dir )
{
  MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  
  if( MMDL_CheckPossibleAcmd(mmdl) == TRUE ){
    return( TRUE );
  }
  
  if( dir == DIR_NOT ){
    return( FALSE );
  }

#if 0  
  if( Jiki_WallHitAcmdCodeCheck(code) == TRUE ){ //壁ヒットはキャンセル可能
  }
#endif

  return( FALSE );
}


//--------------------------------------------------------------
/**
 * 自機　入力キーから移動する方向を返す
 * @param player_core FIELD_PLAYER_CORE
 * @param key キー情報
 * @retval u16 移動方向 DIR_UP等
 */
//--------------------------------------------------------------
u16 FIELD_PLAYER_CORE_GetKeyDir( const FIELD_PLAYER_CORE* player_core, int key )
{
  u16 dir = gjiki_GetInputKeyDir( player_core, key );
  return dir;
}

//--------------------------------------------------------------
/**
 * 移動開始時のキー入力情報セット
 * @param player_core FIELD_PLAYER_GRID
 * @param key_prs キー入力情報
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_SetMoveStartKeyDir( FIELD_PLAYER_CORE* player_core, int key )
{
  gjiki_SetInputKeyDir( player_core, key );
}



//--------------------------------------------------------------
/**
 * 自機に波乗りポケモンのタスクポインタをセット
 * @param player_core FIELD_PLAYER_CORE
 * @param task セットするFLDEFF_TASK
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_SetEffectTaskWork(
    FIELD_PLAYER_CORE *player_core, FLDEFF_TASK *task )
{
  player_core->fldeff_joint = task;
}

//--------------------------------------------------------------
/**
 * 自機が持っているエフェクトタスクのポインタを取得
 * @param   player_core FIELD_PLAYER_CORE
 * @retval FLDEFF_TASK*
 */
//--------------------------------------------------------------
FLDEFF_TASK * FIELD_PLAYER_CORE_GetEffectTaskWork(
    FIELD_PLAYER_CORE *player_core )
{
  return( player_core->fldeff_joint );
}




//-----------------------------------------------------------------------------
/**
 *      private関数
 */
//-----------------------------------------------------------------------------

//--------------------------------------------------------------
/**
 * 自機の動作形態を変更
 * @param player_core FIELD_PLAYER＿CORE
 * @param form PLAYER_MOVE_FORM
 * @retval nothing
 */
//--------------------------------------------------------------
static void fldplayer_ChangeMoveForm(
    FIELD_PLAYER_CORE *player_core, PLAYER_MOVE_FORM form )
{
  int sex = FIELD_PLAYER_CORE_GetSex( player_core );
  MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  u16 code = FIELD_PLAYER_GetMoveFormToOBJCode( sex, form );
  
  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
  FIELD_PLAYER_CORE_SetMoveForm( player_core, form );
}





//-------------------------------------
///	キー入力処理
//=====================================
//--------------------------------------------------------------
/**
 * キー入力方向取得
 * @param player_core
 * @param key_prs キー入力情報
 * @retval u16 キー入力方向
 */
//--------------------------------------------------------------
static u16 gjiki_GetInputKeyDir(
    const FIELD_PLAYER_CORE *player_core, u16 key_prs )
{
  int key_dir_x = getKeyDirX( key_prs );
  int key_dir_z = getKeyDirZ( key_prs );
  
  if( key_dir_x == DIR_NOT ){   //一方向押しの場合はZ優先
    return( key_dir_z );
  }
  
  if( key_dir_z == DIR_NOT ){   //Zキー無し Xキー返し
    return( key_dir_x );
  }
  
  { //斜め押し
    MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
    u16 move_dir = MMDL_GetDirMove( mmdl );
    u16 input_x = player_core->input_key_dir_x;
    u16 input_z = player_core->input_key_dir_z;
    
    //移動方向と一致しない方向をZ優先で返す
    if( move_dir != DIR_NOT ){
      //過去に押した方向を継続
      if( key_dir_x == input_x && key_dir_z == input_z ){
        return( move_dir ); //移動中と一致する方向を返す
      }
      
      if( key_dir_z != input_z ){  //新規斜め押しはZ優先で返す
        return( key_dir_z );
      }
      
      return( key_dir_x );
    }
    
    return( key_dir_z ); //Z優先で返す
  }
  
  return( DIR_NOT );
}

//--------------------------------------------------------------
/**
 * キー入力情報セット
 * @param player_core FIELD_PLAYER_CORE
 * @param key_prs キー入力情報
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_SetInputKeyDir( FIELD_PLAYER_CORE *player_core, u16 key_prs )
{
  player_core->input_key_dir_x = getKeyDirX( key_prs );
  player_core->input_key_dir_z = getKeyDirZ( key_prs );
}

//--------------------------------------------------------------
/**
 * キープレスから押されているX方向を取得
 * @param key_prs キー入力情報
 * @retval u16 押されている方向 DIR_UP等
 */
//--------------------------------------------------------------
static u16 getKeyDirX( u16 key_prs )
{
  if( (key_prs & PAD_KEY_LEFT) ){
    return( DIR_LEFT );
  }
  
  if( (key_prs & PAD_KEY_RIGHT) ){
    return( DIR_RIGHT );
  }
  
  return( DIR_NOT );
}

//--------------------------------------------------------------
/**
 * キープレスから押されているZ方向を取得
 * @param key_prs キー入力情報
 * @retval u16 押されている方向 DIR_UP等
 */
//--------------------------------------------------------------
static u16 getKeyDirZ( u16 key_prs )
{
  if( (key_prs & PAD_KEY_UP) ){
    return( DIR_UP );
  }
  
  if( (key_prs & PAD_KEY_DOWN) ){
    return( DIR_DOWN );
  }
  
  return( DIR_NOT );
}


//-------------------------------------
///	Req系
//=====================================
//----------------------------------------------------------------------------
/**
 * 自機リクエスト処理　通常移動に変更
 * @param player_core FIELD_PLAYER_CORE
 * @retval nothing
 */
//-----------------------------------------------------------------------------
static void gjikiReq_SetNormal( FIELD_PLAYER_CORE *player_core )
{
  VecFx32 offs = {0,0,0};
  int sex;
  u16 code;
  MMDL *mmdl;
  FLDEFF_TASK * task;
  
  sex = FIELD_PLAYER_CORE_GetSex( player_core );
  mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_NORMAL );
  
  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
  
  FIELD_PLAYER_CORE_SetMoveForm( player_core, PLAYER_MOVE_FORM_NORMAL );
  gjiki_PlayBGM( player_core );
  
  // JoinEffectがあれば削除
  task = FIELD_PLAYER_CORE_GetEffectTaskWork( player_core );
  if( task != NULL ){
    FLDEFF_TASK_CallDelete( task );
    FIELD_PLAYER_CORE_SetEffectTaskWork( player_core, NULL );
  }
  
  MMDL_SetVectorOuterDrawOffsetPos( mmdl, &offs );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　自転車移動に変更
 * @param player_core FIELD_PLAYER_CORE
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetCycle( FIELD_PLAYER_CORE *player_core )
{
  int sex;
  u16 code;
  MMDL *mmdl;
  
  sex = FIELD_PLAYER_CORE_GetSex( player_core );
  mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_CYCLE );
  
  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
   
  FIELD_PLAYER_CORE_SetMoveForm( player_core, PLAYER_MOVE_FORM_CYCLE );
  gjiki_PlayBGM( player_core );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　波乗り移動に変更
 * @param player_core FIELD_PLAYER_CORE
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetSwim( FIELD_PLAYER_CORE *player_core )
{
  int sex;
  u16 code;
  MMDL *mmdl;
  
  sex = FIELD_PLAYER_CORE_GetSex( player_core );
  mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_SWIM );

  if( MMDL_GetOBJCode(mmdl) != code ){
    MMDL_ChangeOBJCode( mmdl, code );
  }
  
  FIELD_PLAYER_CORE_SetMoveForm( player_core, PLAYER_MOVE_FORM_SWIM );
  gjiki_PlayBGM( player_core );
  
  if( FIELD_PLAYER_CORE_GetEffectTaskWork( player_core ) == NULL ){ //波乗りポケモン
    u16 dir;
    VecFx32 pos;
    FLDEFF_CTRL *fectrl;
    FLDEFF_TASK* task;
    
    fectrl = FIELDMAP_GetFldEffCtrl( player_core->fieldWork );
    dir = MMDL_GetDirDisp( mmdl );
    MMDL_GetVectorPos( mmdl, &pos );
    
    task = FLDEFF_NAMIPOKE_SetMMdl(
        fectrl, dir, &pos, mmdl, NAMIPOKE_JOINT_ON );

    FIELD_PLAYER_CORE_SetEffectTaskWork( player_core, task );
  }
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　動作形態にあわせた表示にする
 * @param player_core FIELD_PLAYER_CORE
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetMoveFormToDrawForm( FIELD_PLAYER_CORE *player_core )
{
  FIELD_PLAYER_CORE_ResetMoveForm( player_core );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　アイテムゲット自機に変更
 * @param player_core FIELD_PLYER
 * @retval  nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetItemGetHero( FIELD_PLAYER_CORE *player_core )
{
  FIELD_PLAYER_CORE_ChangeDrawForm( player_core, PLAYER_DRAW_FORM_ITEMGET );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　PC預け自機に変更
 * @param player_core FIELD_PLYER
 * @retval  nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetReportHero( FIELD_PLAYER_CORE *player_core )
{
  FIELD_PLAYER_CORE_ChangeDrawForm( player_core, PLAYER_DRAW_FORM_SAVEHERO );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　PC預け自機に変更
 * @param player_core FIELD_PLYER
 * @retval  nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetPCAzukeHero( FIELD_PLAYER_CORE *player_core )
{
  FIELD_PLAYER_CORE_ChangeDrawForm( player_core, PLAYER_DRAW_FORM_PCHERO );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　カットイン自機に変更
 * @param player_core FIELD_PLYER
 * @retval  nothing
 */
//--------------------------------------------------------------
static void gjikiReq_SetCutInHero( FIELD_PLAYER_CORE *player_core )
{
  FIELD_PLAYER_CORE_ChangeDrawForm( player_core, PLAYER_DRAW_FORM_CUTIN );
}

//--------------------------------------------------------------
/**
 * 自機の形態に合わせてBGMを再生
 * @param player_core FIELD_PLAYER_CORE
 * @retval nothing
 */
//--------------------------------------------------------------
static void gjiki_PlayBGM( FIELD_PLAYER_CORE *player_core )
{
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( player_core->fieldWork );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( gdata );
  u32 zone_id = FIELDMAP_GetZoneID( player_core->fieldWork );
  FSND_ChangeBGM_byPlayerFormChange( fsnd, gdata, zone_id );
}


