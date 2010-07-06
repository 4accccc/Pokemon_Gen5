//======================================================================
/**
 *	GAME FREAK inc.
 *	@file		field_player_core.c
 *	@brief  フィールドプレイヤー制御　コア情報
 *	@author	gamefreak
 *	@date		2010.02.05
 *	モジュール名：FIELD_PLAYER_CORE
 */
//======================================================================
#include "gflib.h"

#include "field/field_const.h"

#include "fieldmap.h"
#include "field_g3d_mapper.h"
#include "fldmmdl.h"
#include "field_sound.h"
#include "fldeff_namipoke.h"
#include "fldeff_kemuri.h"
#include "fldeff_bubble.h"

#include "field_player_core.h"

//======================================================================
//  define
//======================================================================
#ifdef ADDFIX100703_REGU_TEX_HERO
#ifdef DEBUG_ONLY_FOR_kagaya
#define PRINTHEAP_TEX_HERO
#endif
#endif

#define REQ_CHG_DRAW_MIDDLE_WAIT (0)

//======================================================================
//  struct
//======================================================================
//--------------------------------------------------------------
///	FIELD_PLAYER_CORE
//--------------------------------------------------------------
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
  
  // 見た目更新ウエイト用　待ち時間
  u16 draw_form_wait;
  u16 chg_draw_form_middle_wait;
  
  // REQ SEQメンバ
  s16 req_seq_no;
  u16 req_seq_end_flag;
};

//--------------------------------------------------------------
/// REQPROC
//--------------------------------------------------------------
typedef int (*REQPROC)(FIELD_PLAYER_CORE *);

//======================================================================
//  proto
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

static void fldplayer_ChangeOBJCode(
    FIELD_PLAYER_CORE *player_core, u16 code );
static void fldplayer_ChangeMoveForm(
    FIELD_PLAYER_CORE *player_core, PLAYER_MOVE_FORM form );
static BOOL fldplayer_CheckOBJCodeHero( FIELD_PLAYER_CORE *player_core );
static BOOL fldplayer_CheckBBDAct( FIELD_PLAYER_CORE *player_core );

///	キー入力処理
static u16 gjiki_GetInputKeyDir(
    const FIELD_PLAYER_CORE *player_core, u16 key_prs );
static void gjiki_SetInputKeyDir( FIELD_PLAYER_CORE *player_core, u16 key_prs );
static u16 getKeyDirX( u16 key_prs );
static u16 getKeyDirZ( u16 key_prs );

///	Req系
static const REQPROC * const data_RequestProcTbl[FIELD_PLAYER_REQBIT_MAX];

//parts
static void gjiki_PlayBGM( FIELD_PLAYER_CORE *player_core );

//======================================================================
//  field player core
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドプレイヤー　作成
 * @param playerWork 使用するPLAYER_WOKR
 * @param *fieldWork 参照するFIELDMAP_WORK
 * @param pos 初期座標
 * @param sex 性別
 * @param heapID HEAPID
 * @retval FIELD_PLAYER_CORE*
 */
//---------------------------------------------------------------
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
      PLAYER_MOVE_FORM form = FIELD_PLAYER_CORE_GetMoveForm( player_core );
      head.obj_code = FIELD_PLAYER_GetMoveFormToOBJCode( sex, form );
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
	
#if 0 // MoveFormがセーブされるようになったため、不要なはず。2010.04.14
  //OBJコードから動作フォームを設定
  if( fixcode == OBJCODEMAX ){
    u16 code = MMDL_GetOBJCode( player_core->fldmmdl );
    PLAYER_MOVE_FORM form = FIELD_PLAYER_GetOBJCodeToMoveForm( sex, code );
    PLAYERWORK_SetMoveForm( player_core->playerWork, form );
  }
#endif
  
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
  fldplayer_ChangeOBJCode( player_core, code );
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

//--------------------------------------------------------------
/**
 * FIELD_PLAYER PLAYER_MOVE_STATEセット
 * @param player_core FIELD_PLAYER_CORE
 * @param val PLAYER_MOVE_STATE
 */
//---------------------------------------------------------------
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

//--------------------------------------------------------------
/**
 *	@brief  プレイヤーワークの取得
 *
 *	@param	player_core  ワーク
 *
 *	@return プレイヤーワーク
 */
//---------------------------------------------------------------
PLAYER_WORK * FIELD_PLAYER_CORE_GetPlayerWork( const FIELD_PLAYER_CORE * player_core )
{
  return (player_core->playerWork);
}

//--------------------------------------------------------------
/**
 * 動作モデルが生きているかチェック
 * @param player_core FIELD_PLAYER_CORE
 * @retval BOOL TRUE=生存
 */
//---------------------------------------------------------------
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
#if 0
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
#endif

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
    fldplayer_ChangeOBJCode( player_core, code );
  }
  
  player_core->draw_form_wait = 0;
}

//--------------------------------------------------------------
/**
 *	@brief  DrawForm変更待ち
 *
 *	@param	player_core 自機ワーク
 *
 *	@retval TRUE  完了
 *	@retval FALSE 変更中
 */
//---------------------------------------------------------------
BOOL FIELD_PLAYER_CORE_CheckDrawFormWait( FIELD_PLAYER_CORE *player_core )
{
  MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  u32 actID;
  actID = MMDL_CallDrawGetProc( mmdl, 0 );
  
  if(actID != MMDL_BLACTID_NULL){

    // 描画できる状態から1フレまつ
    if(player_core->draw_form_wait == 1){
      return TRUE;
    }

    player_core->draw_form_wait ++;
  }
  return FALSE;
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
#if 0
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
#endif

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
#if 0
BOOL FIELD_PLAYER_CORE_ChangeFormWait( FIELD_PLAYER_CORE *player_core )
{
  MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  BOOL result1, result2;
  result1 = FIELD_PLAYER_CORE_CheckDrawFormWait( player_core );
  result2 = MMDL_CheckEndAcmd( mmdl );
  
  // 見た目の更新と、アクションコマンドの終了を待つ
  if( (result1 == TRUE) && (result2 == result1) ){
    return TRUE;
  }
  return FALSE;
}
#endif

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

//--------------------------------------------------------------
/**
 *	@brief  自機のアニメーション完了待ち
 *
 *	@param	player_core   ワーク
 *  
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//---------------------------------------------------------------
BOOL FIELD_PLAYER_CORE_CheckAnimeEnd( const FIELD_PLAYER_CORE *player_core )
{
  MMDLSYS* p_mmdlsys = MMDL_GetMMdlSys( player_core->fldmmdl );
  MMDL_BLACTCONT * p_mmdl_bbdadct = MMDLSYS_GetBlActCont( p_mmdlsys );
  GFL_BBDACT_SYS* p_bbdact = MMDL_BLACTCONT_GetBbdActSys( p_mmdl_bbdadct );
  u32 actID;
  u16 comm;

  if( (MMDL_CheckDrawPause(player_core->fldmmdl) == TRUE) && //描画ポーズON
      (MMDL_BLACTCONT_CheckUpdateBBD(player_core->fldmmdl) == TRUE) ){ //アクター更新済み
    return TRUE;
  }
  
  actID = MMDL_CallDrawGetProc( player_core->fldmmdl, 0 );
  if( actID != MMDL_BLACTID_NULL ){

    if( GFL_BBDACT_GetAnimeLastCommand( p_bbdact, actID, &comm ) ){

      if( comm == GFL_BBDACT_ANMCOM_END ){
        return TRUE;
      }
    }
  }
  
  return FALSE;
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

//======================================================================
//  private関数
//======================================================================
//--------------------------------------------------------------
//  ADDFIX100703_REGU_TEX_HERO 
//--------------------------------------------------------------
#ifdef PRINTHEAP_TEX_HERO
#include "system/main.h"  //HEAPID_FIELDMAP
static void printHeap( int num, u16 code )
{
  switch( FIELD_PLAYER_CheckOBJCodeToDrawForm(code) ){
  case PLAYER_DRAW_FORM_NORMAL:
    OS_TPrintf( "HERO " ); break;
  case PLAYER_DRAW_FORM_CYCLE:
    OS_TPrintf( "CYCLE " ); break;
  case PLAYER_DRAW_FORM_SWIM:
    OS_TPrintf( "SWIM " ); break;
  case PLAYER_DRAW_FORM_DIVING:
    OS_TPrintf( "DIVING " ); break;
  case PLAYER_DRAW_FORM_ITEMGET:
    OS_TPrintf( "ITEMGET " ); break;
  case PLAYER_DRAW_FORM_SAVEHERO:
    OS_TPrintf( "SAVE " ); break;
  case PLAYER_DRAW_FORM_PCHERO:
    OS_TPrintf( "PC " ); break;
  case PLAYER_DRAW_FORM_YURE:
    OS_TPrintf( "YURE " ); break;
  case PLAYER_DRAW_FORM_FISHING:
    OS_TPrintf( "FISHING " ); break;
  case PLAYER_DRAW_FORM_CUTIN:
    OS_TPrintf( "CUTIN " ); break;
  }

  OS_TPrintf( "%d HEAP_FIELD %6x(%6x)\n",
      num,
      GFI_HEAP_GetHeapAllocatableSize( HEAPID_FIELDMAP ),
      GFL_HEAP_GetHeapFreeSize( HEAPID_FIELDMAP ) );
}
#endif

//--------------------------------------------------------------
/**
 * 自機、動作モデルのOBJコードを変更する
 * @param player_core FIELD_PLAYER＿CORE
 * @param code OBJコード
 * @retval nothing
 */
//--------------------------------------------------------------
//----
#ifdef ADDFIX100703_REGU_TEX_HERO
//----
static void fldplayer_ChangeOBJCode(
    FIELD_PLAYER_CORE *player_core, u16 code )
{
  u16 now_code;
  MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  MMDLSYS *mmdlsys = MMDL_GetMMdlSys( mmdl );
  
#if 0  
  OS_TPrintf( "*****HEAPID_FIELD****\n" );
  GFL_HEAP_DEBUG_PrintExistMemoryBlocks( HEAPID_FIELDMAP );
  OS_TPrintf( "****HEAPID_FIELD****\n" );
#endif
  
  now_code = MMDL_GetOBJCode( mmdl );

#ifdef PRINTHEAP_TEX_HERO  
  OS_TPrintf( "---- 自機変更開始\n" );
  printHeap( 0, now_code );
#endif
  
  //現在がHERO,HEROINE、もしくは変更がHERO,HEROINEであれば
  //１回の切り替えのみ
  if( code == HERO || code == HEROINE ||
      now_code == HERO || now_code == HEROINE )
  {
    MMDL_BLACTCONT_ChangeOBJCodeWithDummy( mmdl, code );
    MMDLSYS_ForceWaitVBlankProc( mmdlsys );
  }
  else //HERO,HEROINEを介さなくてはならない他の切り替え
  {
    int sex = FIELD_PLAYER_CORE_GetSex( player_core );
    u16 hero_code = FIELD_PLAYER_GetMoveFormToOBJCode(
        sex, PLAYER_DRAW_FORM_NORMAL );
    
    MMDL_ChangeOBJCode( mmdl, hero_code ); //HEROへの置き換えはダミー不要
    MMDLSYS_ForceWaitVBlankProc( mmdlsys );
    
#ifdef PRINTHEAP_TEX_HERO  
    OS_TPrintf( "一旦自機戻し後 " );
    printHeap( 1, hero_code );
#endif
    
    MMDL_BLACTCONT_ChangeOBJCodeWithDummy( mmdl, code );
    MMDLSYS_ForceWaitVBlankProc( mmdlsys );
  }
  
#ifdef PRINTHEAP_TEX_HERO  
  printHeap( 99, code );
  OS_TPrintf( "----自機変更完了\n" );
#endif
}
//----
#else //old
//----
static void fldplayer_ChangeOBJCode(
    FIELD_PLAYER_CORE *player_core, u16 code )
{
  MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
//  MMDL_ChangeOBJCode( mmdl, code );
  MMDL_BLACTCONT_ChangeOBJCodeWithDummy( mmdl, code );
}
//----
#endif
//----

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
    fldplayer_ChangeOBJCode( player_core, code );
  }
  FIELD_PLAYER_CORE_SetMoveForm( player_core, form );
}

//--------------------------------------------------------------
/**
 * 自機の表示コードがHERO,HEROINEかどうか
 * @param player_core FIELD_PLAYER_CORE
 * @retval BOOL TRUE=HERO,HEROINEである
 */
//--------------------------------------------------------------
static BOOL fldplayer_CheckOBJCodeHero( FIELD_PLAYER_CORE *player_core )
{
  MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  u16 code = MMDL_GetOBJCode( mmdl );

  if( code == HERO || code == HEROINE ){
    return( TRUE );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 自機のビルボードアクターチェック
 * @param player_core FIELD_PLAYER_CORE
 * @retval BOOL TRUE=アクター存在している
 */
//--------------------------------------------------------------
static BOOL fldplayer_CheckBBDAct( FIELD_PLAYER_CORE *player_core )
{
  MMDL *mmdl = player_core->fldmmdl;
  u16 actID = MMDL_CallDrawGetProc( mmdl, 0 );
  
  if( actID != MMDL_BLACTID_NULL ){
    return( TRUE );
  }
  
  return( FALSE );
}

//======================================================================
///	キー入力処理
//======================================================================
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

//======================================================================
//  自機リクエスト
//======================================================================
//--------------------------------------------------------------
/**
 * 指定リクエストを実行
 */
//--------------------------------------------------------------
static BOOL updateRequest( FIELD_PLAYER_CORE *player_core, int no )
{
  if( player_core->req_seq_end_flag == 0 ){
    int ret;
    
    do{
      ret = data_RequestProcTbl[no][player_core->req_seq_no]( player_core );
    }while( ret == TRUE );
  }
  
  return( player_core->req_seq_end_flag );
}

//--------------------------------------------------------------
/**
 * FIELD_PLAYER_REQBIT -> 番号
 */
//--------------------------------------------------------------
static int reqBit_to_Number( u32 bit )
{
  int i = 0;
  
  do{
    if( (bit&0x01) ){
      break;
    }
    bit >>= 1;
    i++;
  }while( i < FIELD_PLAYER_REQBIT_MAX );
  
  return( i );
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
  player_core->req_seq_no = 0;
  player_core->req_seq_end_flag = 0;
  player_core->draw_form_wait = 0;
  player_core->chg_draw_form_middle_wait = 0;
}

//--------------------------------------------------------------
/**
 * リクエストを更新
 * @param player_core FIELD_PLAYER_CORE
 * @retval BOOL TRUE=リクエスト消化完了
 */
//--------------------------------------------------------------
BOOL FIELD_PLAYER_CORE_UpdateRequest( FIELD_PLAYER_CORE *player_core )
{
  FIELD_PLAYER_REQBIT bit = player_core->req_bit;
  int no = reqBit_to_Number( bit );
  
  if( no >= FIELD_PLAYER_REQBIT_MAX ){
    return( TRUE );
  }
  
  if( updateRequest(player_core,no) == TRUE ){
    return( TRUE );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * リクエストを強制更新
 * @param player_core FIELD_PLAYER_CORE
 * @retval nothing
 * @attention リクエストが消化し終わるまで強制実行します。
 */
//--------------------------------------------------------------
void FIELD_PLAYER_CORE_ForceUpdateRequest( FIELD_PLAYER_CORE *player_core )
{
  FIELD_PLAYER_REQBIT bit = player_core->req_bit;
  int no = reqBit_to_Number( bit );
  
  if( no >= FIELD_PLAYER_REQBIT_MAX ){
    GF_ASSERT( 0 );
    return;
  }
  
  while( updateRequest(player_core,no) == FALSE ){};
}

#if 0 //old ビット単位で複数処理していた
void FIELD_PLAYER_CORE_SetRequest(
  FIELD_PLAYER_CORE * player_core, FIELD_PLAYER_REQBIT req_bit )
{
  player_core->req_bit = req_bit;
}

void FIELD_PLAYER_CORE_UpdateRequest( FIELD_PLAYER_CORE *player_core )
{
  int i = 0;
  FIELD_PLAYER_REQBIT req_bit  = player_core->req_bit;
  
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
    gjikiReq_SetDiving, //FIELD_PLAYER_REQBIT_SWIM
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
#endif

//======================================================================
//  自機リクエスト　汎用
//======================================================================
//--------------------------------------------------------------
//  リクエスト req_seq_no増加
//--------------------------------------------------------------
#define req_NextSeqNo( player_core ) player_core->req_seq_no++;

//--------------------------------------------------------------
//  リクエスト req_seq_end_flagを立てる
//--------------------------------------------------------------
#define req_SetEndFlag( player_core ) player_core->req_seq_end_flag = TRUE;

//--------------------------------------------------------------
/**
 * リクエスト　自機のOBJコードを変更する
 * @param
 * @retval BOOL TRUE=変更もしくは同一。FALSE=HERO,HEROINEに変更した
 */
//--------------------------------------------------------------
static BOOL reqCommon_SetOBJCode(
    FIELD_PLAYER_CORE *player_core, u16 code )
{
  MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  
  if( MMDL_GetOBJCode(mmdl) == code ){
    req_SetEndFlag( player_core );
    return( TRUE );
  }
  
  if( fldplayer_CheckOBJCodeHero(player_core) == TRUE ){
    fldplayer_ChangeOBJCode( player_core, code );
    req_SetEndFlag( player_core );
    return( TRUE );
  }
  
  {
    int sex = FIELD_PLAYER_CORE_GetSex( player_core );
    code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_NORMAL );
    fldplayer_ChangeOBJCode( player_core, code );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * リクエスト　PLAYER_DRAW_FORMセット
 * @param
 * @retval
 */
//--------------------------------------------------------------
static BOOL reqCommon_SetDrawForm(
    FIELD_PLAYER_CORE *player_core, PLAYER_DRAW_FORM form )
{
  int sex = FIELD_PLAYER_CORE_GetSex( player_core );
  u16 code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, form );
  return( reqCommon_SetOBJCode(player_core,code) );
}

//--------------------------------------------------------------
/**
 * リクエスト　自機の表示コード変更を待ち、指定PLAYER_DRAW_FORMに変更
 * @param 
 * @retval
 */
//--------------------------------------------------------------
static BOOL reqCommon_WaitSetDrawForm(
    FIELD_PLAYER_CORE *player_core, PLAYER_DRAW_FORM form )
{
  if( fldplayer_CheckBBDAct(player_core) == TRUE ){
    player_core->chg_draw_form_middle_wait++;
    
    if( player_core->chg_draw_form_middle_wait >= REQ_CHG_DRAW_MIDDLE_WAIT ){
      int sex = FIELD_PLAYER_CORE_GetSex( player_core );
      u16 code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, form );
      fldplayer_ChangeOBJCode( player_core, code );
      req_SetEndFlag( player_core );
      return( TRUE );
    }
  }
  
  return( FALSE );
}

//======================================================================
//  自機リクエスト　通常移動に変更
//======================================================================
//--------------------------------------------------------------
/**
 * 自機リクエスト処理　通常移動に変更 0
 * @param player_core FIELD_PLAYER_CORE
 * @retval nothing
 */
//---------------------------------------------------------------
static int req_SetNormal0( FIELD_PLAYER_CORE *player_core )
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
    fldplayer_ChangeOBJCode( player_core, code );
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
  
  req_SetEndFlag( player_core );
  return( FALSE );
}

//--------------------------------------------------------------
//  自機リクエスト　通常移動に変更　まとめ
//--------------------------------------------------------------
static const REQPROC data_req_SetNormalTbl[] =
{
  req_SetNormal0,
};

//======================================================================
//  自機リクエスト　自転車移動に変更
//======================================================================
//--------------------------------------------------------------
/**
 * 自機リクエスト処理　自転車移動に変更 0
 * @param player_core FIELD_PLAYER_CORE
 * @retval int TRUE=再帰呼び出し
 */
//--------------------------------------------------------------
static int req_SetCycle0( FIELD_PLAYER_CORE *player_core )
{
  int sex;
  u16 code;
  MMDL *mmdl;
  
  sex = FIELD_PLAYER_CORE_GetSex( player_core );
  mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  
  FIELD_PLAYER_CORE_SetMoveForm( player_core, PLAYER_MOVE_FORM_CYCLE );
  gjiki_PlayBGM( player_core );
  
  if( reqCommon_SetDrawForm(player_core,PLAYER_DRAW_FORM_CYCLE) != TRUE ){
    req_NextSeqNo( player_core );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　自転車移動に変更 1
 * @param player_core FIELD_PLAYER_CORE
 * @retval int TRUE=再帰呼び出し
 */
//--------------------------------------------------------------
static int req_SetCycle1( FIELD_PLAYER_CORE *player_core )
{
  reqCommon_WaitSetDrawForm( player_core, PLAYER_DRAW_FORM_CYCLE );
  return( FALSE );
}

//--------------------------------------------------------------
//  自機リクエスト　自転車移動に変更　まとめ
//--------------------------------------------------------------
static const REQPROC data_req_SetCycleTbl[] =
{
  req_SetCycle0,
  req_SetCycle1,
};

//======================================================================
//  自機リクエスト　波乗り移動に変更
//======================================================================
//--------------------------------------------------------------
/**
 * 自機リクエスト処理　波乗り移動に変更 0
 * @param player_core FIELD_PLAYER_CORE
 * @retval nothing
 */
//--------------------------------------------------------------
static int req_SetSwim0( FIELD_PLAYER_CORE *player_core )
{
  int sex;
  u16 code;
  MMDL *mmdl;
  
  sex = FIELD_PLAYER_CORE_GetSex( player_core );
  mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
  code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_SWIM );
  
  if( MMDL_GetOBJCode(mmdl) != code ){
    fldplayer_ChangeOBJCode( player_core, code );
  }
  
  if(FIELD_PLAYER_CORE_GetMoveForm(player_core) != PLAYER_MOVE_FORM_SWIM ){
    FIELD_PLAYER_CORE_SetMoveForm( player_core, PLAYER_MOVE_FORM_SWIM );
  }
  
  gjiki_PlayBGM( player_core );
  
  //波乗りポケモン
  if( FIELD_PLAYER_CORE_GetEffectTaskWork(player_core) == NULL ){
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
  
  //強制で波乗りに置き換えるリクエストなので
  //表示変更待ちは要らない
  req_SetEndFlag( player_core );
  return( FALSE );
}

//--------------------------------------------------------------
//  自機リクエスト　波乗り移動に変更　まとめ
//--------------------------------------------------------------
static const REQPROC data_req_SetNaminoriTbl[] =
{
  req_SetSwim0,
};

//======================================================================
//  自機リクエスト　動作形態にあわせた表示にする
//======================================================================
//--------------------------------------------------------------
/**
 * 自機リクエスト処理　動作形態にあわせた表示にする 0
 * @param player_core FIELD_PLAYER_CORE
 * @retval nothing
 */
//--------------------------------------------------------------
static int req_SetMoveFormToDrawForm0( FIELD_PLAYER_CORE *player_core )
{
  PLAYER_MOVE_FORM form = FIELD_PLAYER_CORE_GetMoveForm( player_core );
  int sex = FIELD_PLAYER_CORE_GetSex( player_core );
  u16 code = FIELD_PLAYER_GetMoveFormToOBJCode( sex, form );
  
  if( reqCommon_SetOBJCode(player_core,code) != TRUE ){
    req_NextSeqNo( player_core );
  }
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　動作形態にあわせた表示にする 1
 * @param player_core FIELD_PLAYER_CORE
 * @retval nothing
 */
//--------------------------------------------------------------
static int req_SetMoveFormToDrawForm1( FIELD_PLAYER_CORE *player_core )
{
  if( fldplayer_CheckBBDAct(player_core) == TRUE ){
    player_core->chg_draw_form_middle_wait++;
    
    if( player_core->chg_draw_form_middle_wait >= REQ_CHG_DRAW_MIDDLE_WAIT ){
      PLAYER_MOVE_FORM form = FIELD_PLAYER_CORE_GetMoveForm( player_core );
      int sex = FIELD_PLAYER_CORE_GetSex( player_core );
      u16 code = FIELD_PLAYER_GetMoveFormToOBJCode( sex, form );
      fldplayer_ChangeOBJCode( player_core, code );
      req_SetEndFlag( player_core );
    }
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
// 自機リクエスト処理　動作形態にあわせた表示にする　まとめ
//--------------------------------------------------------------
static const REQPROC data_req_SetMoveFormToDrawFormTbl[] =
{
  req_SetMoveFormToDrawForm0,
  req_SetMoveFormToDrawForm1,
};

//======================================================================
//  自機リクエスト  アイテムゲット自機に変更
//======================================================================
//--------------------------------------------------------------
/**
 * 自機リクエスト処理　アイテムゲット自機に変更 0
 * @param player_core FIELD_PLYER
 * @retval int TRUE=再帰呼び出し
 */
//--------------------------------------------------------------
static int req_SetItemGetHero0( FIELD_PLAYER_CORE *player_core )
{
  if( reqCommon_SetDrawForm(player_core,PLAYER_DRAW_FORM_ITEMGET) != TRUE ){
    req_NextSeqNo( player_core );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　アイテムゲット自機に変更 1
 * @param player_core FIELD_PLAYER_CORE
 * @retval int TRUE=再帰呼び出し
 */
//--------------------------------------------------------------
static int req_SetItemGetHero1( FIELD_PLAYER_CORE *player_core )
{
  reqCommon_WaitSetDrawForm( player_core, PLAYER_DRAW_FORM_ITEMGET );
  return( FALSE );
}

//--------------------------------------------------------------
//  自機リクエスト　アイテムゲット自機に変更　まとめ
//--------------------------------------------------------------
static const REQPROC data_req_SetItemGetHeroTbl[] =
{
  req_SetItemGetHero0,
  req_SetItemGetHero1,
};

//======================================================================
//  自機リクエスト  レポート自機に変更
//======================================================================
//--------------------------------------------------------------
/**
 * 自機リクエスト処理　レポート自機に変更 0
 * @param player_core FIELD_PLYER
 * @retval int TRUE=再帰呼び出し
 */
//--------------------------------------------------------------
static int req_SetReportHero0( FIELD_PLAYER_CORE *player_core )
{
  if( reqCommon_SetDrawForm(player_core,PLAYER_DRAW_FORM_SAVEHERO) != TRUE ){
    req_NextSeqNo( player_core );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　レポート自機に変更 1
 * @param player_core FIELD_PLAYER_CORE
 * @retval int TRUE=再帰呼び出し
 */
//--------------------------------------------------------------
static int req_SetReportHero1( FIELD_PLAYER_CORE *player_core )
{
  reqCommon_WaitSetDrawForm( player_core, PLAYER_DRAW_FORM_SAVEHERO );
  return( FALSE );
}

//--------------------------------------------------------------
//  自機リクエスト　レポート自機に変更　まとめ
//--------------------------------------------------------------
static const REQPROC data_req_SetReportHeroTbl[] =
{
  req_SetReportHero0,
  req_SetReportHero1,
};

//======================================================================
//  自機リクエスト  PC預け自機に変更
//======================================================================
//--------------------------------------------------------------
/**
 * 自機リクエスト処理　PC預け自機に変更 0
 * @param player_core FIELD_PLYER
 * @retval int TRUE=再帰呼び出し
 */
//--------------------------------------------------------------
static int req_SetPcAzukeHero0( FIELD_PLAYER_CORE *player_core )
{
  if( reqCommon_SetDrawForm(player_core,PLAYER_DRAW_FORM_PCHERO) != TRUE ){
    req_NextSeqNo( player_core );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　PC預け自機に変更 1
 * @param player_core FIELD_PLAYER_CORE
 * @retval int TRUE=再帰呼び出し
 */
//--------------------------------------------------------------
static int req_SetPcAzukeHero1( FIELD_PLAYER_CORE *player_core )
{
  reqCommon_WaitSetDrawForm( player_core, PLAYER_DRAW_FORM_PCHERO );
  return( FALSE );
}

//--------------------------------------------------------------
//  自機リクエスト　PC預け自機に変更　まとめ
//--------------------------------------------------------------
static const REQPROC data_req_SetPcAzukeHeroTbl[] =
{
  req_SetPcAzukeHero0,
  req_SetPcAzukeHero1,
};

//======================================================================
//  自機リクエスト  カットイン自機に変更
//======================================================================
//--------------------------------------------------------------
/**
 * 自機リクエスト処理　カットイン自機に変更 0
 * @param player_core FIELD_PLYER
 * @retval int TRUE=再帰呼び出し
 */
//--------------------------------------------------------------
static int req_SetCutinHero0( FIELD_PLAYER_CORE *player_core )
{
  if( reqCommon_SetDrawForm(player_core,PLAYER_DRAW_FORM_CUTIN) != TRUE ){
    req_NextSeqNo( player_core );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　カットイン自機に変更 1
 * @param player_core FIELD_PLAYER_CORE
 * @retval int TRUE=再帰呼び出し
 */
//--------------------------------------------------------------
static int req_SetCutinHero1( FIELD_PLAYER_CORE *player_core )
{
  reqCommon_WaitSetDrawForm( player_core, PLAYER_DRAW_FORM_CUTIN);
  return( FALSE );
}

//--------------------------------------------------------------
//  自機リクエスト　カットイン自機に変更　まとめ
//--------------------------------------------------------------
static const REQPROC data_req_SetCutinHeroTbl[] =
{
  req_SetCutinHero0,
  req_SetCutinHero1,
};

//======================================================================
//  自機リクエスト  ダイビング自機に変更
//======================================================================
//--------------------------------------------------------------
/**
 *	@brief  DIVING状態を設定
 */
//--------------------------------------------------------------
static int req_SetDiving0( FIELD_PLAYER_CORE *player_core )
{
  FLDEFF_CTRL *fectrl = FIELDMAP_GetFldEffCtrl( player_core->fieldWork );
  
  // まず波乗り状態にして
  {
    int sex;
    u16 code;
    MMDL *mmdl;
    
    sex = FIELD_PLAYER_CORE_GetSex( player_core );
    mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
    code = FIELD_PLAYER_GetDrawFormToOBJCode( sex, PLAYER_DRAW_FORM_DIVING );
    
    if( MMDL_GetOBJCode(mmdl) != code ){
      fldplayer_ChangeOBJCode( player_core, code );
    }
    
    if( FIELD_PLAYER_CORE_GetMoveForm(player_core) != PLAYER_MOVE_FORM_DIVING ){
      FIELD_PLAYER_CORE_SetMoveForm( player_core, PLAYER_MOVE_FORM_DIVING );
    }
      
    // ダイビングは、常に画面遷移とともに発動される。
    // さらに、BGMは通常曲でよいので、
    // BGM操作は行わない。
    //gjiki_PlayBGM( player_core );
    
    //波乗りポケモン
    if( FIELD_PLAYER_CORE_GetEffectTaskWork(player_core) == NULL ){
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
  
  //気泡を設定
  FLDEFF_BUBBLE_SetMMdl( player_core->fldmmdl, fectrl );

  // 水飛沫エフェクトOFF
  if( player_core->fldeff_joint ){
    FLDEFF_NAMIPOKE_SetRippleEffect( player_core->fldeff_joint, FALSE );
  }

  //強制でダイビングに置き換えるリクエストなので
  //表示変更待ちは要らない
  req_SetEndFlag( player_core );
  return( FALSE );
}

//--------------------------------------------------------------
//  自機リクエスト　ダイビングに変更　まとめ
//--------------------------------------------------------------
static const REQPROC data_req_SetDivingTbl[] =
{
  req_SetDiving0,
};

//======================================================================
//  自機リクエスト  釣り自機に変更
//======================================================================
//--------------------------------------------------------------
/**
 * 自機リクエスト処理　釣り自機に変更 0
 * @param player_core FIELD_PLYER
 * @retval int TRUE=再帰呼び出し
 */
//--------------------------------------------------------------
static int req_SetFishing0( FIELD_PLAYER_CORE *player_core )
{
  if( reqCommon_SetDrawForm(player_core,PLAYER_DRAW_FORM_FISHING) != TRUE ){
    req_NextSeqNo( player_core );
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト処理　釣り自機に変更 1
 * @param player_core FIELD_PLAYER_CORE
 * @retval int TRUE=再帰呼び出し
 */
//--------------------------------------------------------------
static int req_SetFishing1( FIELD_PLAYER_CORE *player_core )
{
  reqCommon_WaitSetDrawForm( player_core, PLAYER_DRAW_FORM_FISHING);
  
  /*
   * add 100617 BUGFIX BTS 社内バグNo.1568
   * 波乗り中で下にOBJが居るならばオフセットを変える
  */
  if( player_core->req_seq_end_flag == TRUE ){
    if( FIELD_PLAYER_CORE_GetMoveForm(player_core) == PLAYER_MOVE_FORM_SWIM ){
      int ret = FALSE;
      MMDL *mmdl = FIELD_PLAYER_CORE_GetMMdl( player_core );
      MMDLSYS *mmdlsys = MMDL_GetMMdlSys( mmdl );
      s16 gx = MMDL_GetGridPosX( mmdl );
      s16 gz = MMDL_GetGridPosZ( mmdl );
      MMDL_TOOL_AddDirGrid( DIR_DOWN, &gx, &gz, 1 );
      
      if( MMDLSYS_SearchGridPos(mmdlsys,gx,gz,TRUE) != NULL ){
        ret = TRUE; //従来の表示に
      }

      MMDL_DrawFishingHero_SetOffsetType( mmdl, ret );
    }  
  }
  
  return( FALSE );
}

//--------------------------------------------------------------
//  自機リクエスト　釣り自機に変更　まとめ
//--------------------------------------------------------------
static const REQPROC data_req_SetFishingTbl[] =
{
  req_SetFishing0,
  req_SetFishing1,
};

//======================================================================
//  自機リクエスト　まとめ
//======================================================================
//--------------------------------------------------------------
/// リクエストテーブル
//--------------------------------------------------------------
static const REQPROC * const data_RequestProcTbl[FIELD_PLAYER_REQBIT_MAX] =
{
  data_req_SetNormalTbl, //FIELD_PLAYER_REQBIT_NORMAL 
  data_req_SetCycleTbl, //FIELD_PLAYER_REQBIT_CYCLE
  data_req_SetNaminoriTbl, //FIELD_PLAYER_REQBIT_SWIM
  data_req_SetMoveFormToDrawFormTbl, //FIELD_PLAYER_REQBIT_MOVE_FORM_TO_DRAW_FORM
  data_req_SetItemGetHeroTbl, //FIELD_PLAYER_REQBIT_ITEMGET
  data_req_SetReportHeroTbl, //FIELD_PLAYER_REQBIT_REPORT
  data_req_SetPcAzukeHeroTbl, //FIELD_PLAYER_REQBIT_PC_AZUKE
  data_req_SetCutinHeroTbl, //FIELD_PLAYER_REQBIT_CUTIN
  data_req_SetDivingTbl, //FIELD_PLAYER_REQBIT_DIVING
  data_req_SetFishingTbl, //FIELD_PLAYER_REQBIT_MAX
};

//======================================================================
//  parts
//======================================================================
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
