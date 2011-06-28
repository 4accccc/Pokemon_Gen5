//======================================================================
/**
 * @file	scrcmd_fldmmdl.c
 * @brief	スクリプトコマンド：動作モデル関連
 * @date  2009.09.13
 * @author	tamada GAMEFREAK inc.
 *
 * 2009.09.13 scrcmd.cより分離させた
 */
//======================================================================

#include <gflib.h>

#include "system/vm_cmd.h"

#include "scrcmd.h"
#include "scrcmd_work.h"
#include "scrcmd_fldmmdl.h"

#include "script_def.h" //SCR_OBJID_
#include "script_local.h"
#include "field_player.h" //FIELD_PLAYER
#include "field_player_grid.h"  // FIELD_PLAYER_GRID
#include "fieldmap.h"     //FIELDMAP_
#include "field/field_const.h"  //GRID_TO_FX32
#include "eventdata_local.h"
#include "field/eventdata_sxy.h"

#include "fieldmap_ctrl_grid.h"

#include "field_task_manager.h"
#include "field_task_player_rot.h"
#include "field_task_player_drawoffset.h"
#include "field_task_player_fall.h"

#include "event_fldmmdl_control.h"
#include "event_rail_slipdown.h"

//======================================================================
//  ツール関数
//======================================================================
static MMDL * scmd_GetMMdlPlayer( SCRCMD_WORK *work );

//======================================================================
///  動作モデル監視に使用するインライン関数定義
//======================================================================
#define  PLAYER_BIT      (1<<0)
#define  PLAYER_PAIR_BIT    (1<<1)
#define  OTHER_BIT      (1<<2)
#define  OTHER_PAIR_BIT    (1<<3)

static const MMDL_ACMD_LIST anime_up_table[] = {
  AC_STAY_WALK_U_8F, 1,
  ACMD_END, 0
};
static const MMDL_ACMD_LIST anime_down_table[] = {
  AC_STAY_WALK_D_8F, 1,
  ACMD_END, 0
};
static const MMDL_ACMD_LIST anime_left_table[] = {
  AC_STAY_WALK_L_8F, 1,
  ACMD_END, 0
};
static const MMDL_ACMD_LIST anime_right_table[] = {
  AC_STAY_WALK_R_8F, 1,
  ACMD_END, 0
};


static u8 step_watch_bit;

static inline void InitStepWatchBit(void)
{
  step_watch_bit = 0;
}

static inline BOOL CheckStepWatchBit(int mask)
{
  return (step_watch_bit & mask) != 0;
}

static inline void SetStepWatchBit(int mask)
{
  step_watch_bit |= mask;
}

static inline void ResetStepWatchBit(int mask)
{
  step_watch_bit &= (0xff ^ mask);
}

//======================================================================
//  動作モデル  
//======================================================================
//--------------------------------------------------------------
/**
 * FIELD_OBJ_PTRを取得
 * @param  fsys  FIELDSYS_WORK型のポインタ
 * @param  obj_id  OBJID
 * @retval  "FIELD_OBJ_PTR"
 */
//--------------------------------------------------------------
static MMDL * FieldObjPtrGetByObjId( SCRCMD_WORK *work, u16 obj_id )
{
  //MMDL *dummy;
  MMDL *fmmdl;
  MMDLSYS *fmmdlsys;
  
  fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  
  //連れ歩きOBJ判別IDが渡された時
  if( obj_id == SCR_OBJID_MV_PAIR ){
    fmmdl = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
  //透明ダミーOBJ判別IDが渡された時
  }else if( obj_id == SCR_OBJID_DUMMY ){
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    //dummy = SCRIPT_GetMemberWork( sc, ID_EVSCR_DUMMY_OBJ );
  //対象のフィールドOBJのポインタ取得
  }else{
    fmmdl = MMDLSYS_SearchOBJID( fmmdlsys, obj_id );
  }
  
  return fmmdl;
}

//--------------------------------------------------------------
/**
 * 動作モデルアニメーション
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjAnime( VMHANDLE *core, void *wk )
{
  VM_CODE *p;
  GFL_TCB *anm_tcb;
  MMDL *fmmdl; //対象のフィールドOBJのポインタ
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 obj_id = SCRCMD_GetVMWorkValue(core,work); //obj ID
  u32 pos = VMGetU32(core); //list pos
  
  fmmdl = FieldObjPtrGetByObjId( work, obj_id );
  
  //エラーチェック
  if( fmmdl == NULL ){
    OS_Printf( "obj_id = %d\n", obj_id );
    GF_ASSERT( (0) && "対象のフィールドOBJのポインタ取得失敗！" );
    return VMCMD_RESULT_CONTINUE;        //08.06.12 プラチナで追加
  }
  
  //アニメーションコマンドリストセット
  p = (VM_CODE*)(core->adrs+pos);
  anm_tcb = MMDL_SetAcmdList( fmmdl, (MMDL_ACMD_LIST*)p );
  
  //TCBセット
  SCRCMD_WORK_SetMMdlAnmTCB( work, anm_tcb );
  return VMCMD_RESULT_CONTINUE;
}


// >> update ichiraku 2010/10/12

#define ACMD_LIST_MAX   ( 30 )      // アニメコマンドのMAX
#define GRID_AVOID_MAX  ( 10 )      // 障害物を避けるグリッド数のMAX

// 進む方向
typedef enum {
  DIR_U = DIR_UP,  // 上
  DIR_D,  // 下
  DIR_L,  // 左
  DIR_R,  // 右
}DIR_TYPE;

// ライン定義
typedef enum {
  LINE_X_RIGHT, // Xラインで右移動
  LINE_X_LEFT,  // Xラインで左移動
  LINE_Z_UP,    // Zラインで上移動
  LINE_Z_DOWN,  // Zラインで下移動
}LINE_TYPE;

//-------------------------------------
///	自動アニメパラメータ
//=====================================
typedef struct {
  SCRCMD_WORK     *p_scwk;
  FIELDMAP_WORK   *p_fieldMap_wk;

  // パラメータ関係
  // スクリプトコマンドの引数が追加されるならここ
  u16   target_x;     // 指定グリッドX座標
  u16   target_z;     // 指定グリッドZ座標
  u16   force_type;   // 障害物を強制的に無視するフラグ
  u16   ac_frame;     // 移動にかかるフレーム数
  u16   first_line;   // 先にXとZどちらに移動するか 0:X 1:Z
}ANIME_AUTO_PARAM;


//----------------------------------------------------------------------------
/**
 *	@brief  方向別アニメコマンド設定
 *
 *	@param	MMDL_ACMD_LIST *p_list  アニメーションコマンド格納するポインタ
 *	@param	dir         方向
 *	@param	*p_list_num アニメコマンド総数
 *  @param  num         総数
 */
//-----------------------------------------------------------------------------
static void EvAcmdSet( MMDL_ACMD_LIST *p_list, DIR_TYPE dir, u16 *p_list_num, u16 num, ANIME_AUTO_PARAM *p_param )
{
  u16 ac_walk;    // アクションコマンドコード

  // フレームからアクションコマンドコードの上移動を取得する
  switch( p_param->ac_frame ){
  case 32:  ac_walk = AC_WALK_U_32F; break;
  case 16:  ac_walk = AC_WALK_U_16F; break;
  case 12:  ac_walk = AC_WALK_U_12F; break;
  case 8:   ac_walk = AC_WALK_U_8F; break;
  case 7:   ac_walk = AC_WALK_U_7F; break;
  case 6:   ac_walk = AC_WALK_U_6F; break;
  case 4:   ac_walk = AC_WALK_U_4F; break;
  case 3:   ac_walk = AC_WALK_U_3F; break;
  case 2:   ac_walk = AC_WALK_U_2F; break;
  default:  ac_walk = AC_WALK_U_16F;break;  // デフォルト
  }

  switch( dir ){
  case DIR_U:  // 上
    p_list[ *p_list_num ].code = ac_walk;
    p_list[ *p_list_num ].num  = num;
    break;
  case DIR_D:  // 下
    p_list[ *p_list_num ].code = ac_walk + 1;
    p_list[ *p_list_num ].num  = num;
    break;
  case DIR_L:  // 左
    p_list[ *p_list_num ].code = ac_walk + 2;
    p_list[ *p_list_num ].num  = num;
    break;
  case DIR_R:  // 右
    p_list[ *p_list_num ].code = ac_walk + 3;
    p_list[ *p_list_num ].num  = num;
    break;
  default:
    OS_Printf("ここには通らないはず\n");
    GF_ASSERT(0);
  }

  *p_list_num+=1;

  // エラーチェック
  if( *p_list_num > ACMD_LIST_MAX )
  {
    GF_ASSERT_MSG( 0, "アニメコマンドがMAX値を超えました。");
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  前方の位置セット
 *
 *	@param	const DIR_TYPE dir  方向
 *	@param	*p_pos  設定される位置
 *	@param  len     距離(基本は1)
 */
//-----------------------------------------------------------------------------
static void EvSetFrontPos( const DIR_TYPE dir, VecFx32 *p_pos, u16 len )
{
  switch( dir ){
  case DIR_U: p_pos->z -= FIELD_CONST_GRID_FX32_SIZE * len; break;
  case DIR_D: p_pos->z += FIELD_CONST_GRID_FX32_SIZE * len; break;
  case DIR_L: p_pos->x -= FIELD_CONST_GRID_FX32_SIZE * len; break;
  case DIR_R: p_pos->x += FIELD_CONST_GRID_FX32_SIZE * len; break;
  default: GF_ASSERT(0);
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  指定位置に障害物があるかチェック
 *
 *	@param	FIELDMAP_WORK* p_fieldMap_wk
 *	@param	pos   調べたい位置
 *
 *	@return TRUE:障害物あり FALSE:障害物なし
 */
//-----------------------------------------------------------------------------
static BOOL EvCheckAttr( ANIME_AUTO_PARAM *p_param, VecFx32 pos )
{
  // 強制フラグが立っていると何もチェックしない
  if( p_param->force_type & FORCE_TYPE_ALL )
  {
    // マップの範囲外はチェックする
    FLDMAPPER     *g3Dmapper= FIELDMAP_GetFieldG3Dmapper( p_param->p_fieldMap_wk );
    MAPATTR       attr      = MAPATTR_GetAttribute( g3Dmapper, &pos );
    MAPATTR_FLAG  attr_flag = MAPATTR_GetAttrFlag( attr );
    if( FLDMAPPER_CheckOutRange( g3Dmapper, &pos ) == TRUE )
    {
      ICHI_Printf("マップ範囲外\n");
      return TRUE;
    }
    
    return FALSE;
  }

  {
    FLDMAPPER     *g3Dmapper= FIELDMAP_GetFieldG3Dmapper( p_param->p_fieldMap_wk );

    // 地形チェック
    if( !(p_param->force_type & FORCE_TYPE_MAP) )  // マップ無視フラグ
    {
      MAPATTR       attr      = MAPATTR_GetAttribute( g3Dmapper, &pos );
      MAPATTR_FLAG  attr_flag = MAPATTR_GetAttrFlag( attr );
      // アトリビュートフラグチェック
      if( attr_flag & MAPATTR_FLAGBIT_HITCH ||
          attr_flag & MAPATTR_FLAGBIT_WATER)
      {
        // 障害物あり
        ICHI_Printf("地形侵入不可\n");
        return TRUE;
      }
    }

    // マップの範囲外チェック
    // 0 < pos < マップ範囲 をチェック
    if( FLDMAPPER_CheckOutRange( g3Dmapper, &pos ) == TRUE )
    {
      ICHI_Printf("マップ範囲外\n");
      return TRUE;
    }
  }

  // NPCがいるかチェック
  if( !(p_param->force_type & FORCE_TYPE_NPC) )  // OBJ無視フラグがOFFのときのみ
  {
    MMDL *p_mmdl;
    // 動作モデルを検索
    p_mmdl = MMDLSYS_SearchGridPos(
        FIELDMAP_GetMMdlSys(p_param->p_fieldMap_wk),
        FX32_TO_GRID(pos.x),
        FX32_TO_GRID(pos.z),
        TRUE );

    // 動作モデルを発見
    if( p_mmdl ) { return TRUE; }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  障害物を避けて一歩進む
 *	        障害物の大きさも考慮しているが限界がある(GRID_AVOID_MAX)
 *
 *	@param	const DIR_TYPE dir  進む方向
 *	@param	*p_fieldMap_wk      マップアトリビュート取得用
 *	@param	*p_pos              現在位置
 *	@param	*p_list             アニメコマンドリストへのポインタ
 *	@param	*p_list_num         アニメコマンドリストの数へのポインタ
 *
 *	@return TRUE：避けれた FALSE:避けれない
 */
//-----------------------------------------------------------------------------
static BOOL EvAttrHit( const DIR_TYPE dir, VecFx32 *p_pos, MMDL_ACMD_LIST *p_list, u16 *p_list_num, ANIME_AUTO_PARAM *p_param )
{
  VecFx32 save_pos;

  // 位置保存
  VEC_Set( &save_pos, p_pos->x, p_pos->y, p_pos->z );

  switch( dir ){
  case DIR_R:   // 右に行きたいのにいけないとき
  case DIR_L:   // 左に行きたいのにいけないとき
    {
      u16  len = 1;

      // まず上から調べて次に下から調べる
      while(1)
      {
        VecFx32 tmp_pos = save_pos;

        // 上いって右いってみる
        EvSetFrontPos( DIR_U, &tmp_pos, len );
        EvSetFrontPos( dir, &tmp_pos, 1 );

        // 障害物チェック
        if( !( EvCheckAttr( p_param, tmp_pos ) ) )
        {
          // 方向別アニメコマンドセット
          EvAcmdSet( p_list, DIR_U, p_list_num, len, p_param );
          EvAcmdSet( p_list, dir, p_list_num, 1, p_param );
          // 座標も同時に移動させる
          VEC_Set( p_pos, tmp_pos.x, tmp_pos.y, tmp_pos.z );
          break;
        }else
        {
          tmp_pos = save_pos;
          // 上がだめだったので下から調べてみる
          EvSetFrontPos( DIR_D, &tmp_pos, len );
          EvSetFrontPos( dir, &tmp_pos, 1 );

          if( !(EvCheckAttr( p_param, tmp_pos )) )
          {
            // 方向別アニメコマンドセット
            EvAcmdSet( p_list, DIR_D, p_list_num, len, p_param );
            EvAcmdSet( p_list, dir, p_list_num, 1, p_param );
            // 座標も同時に移動させる
            VEC_Set( p_pos, tmp_pos.x, tmp_pos.y, tmp_pos.z );
            break;
          }else
          {
            // エラーチェック
            if( len > GRID_AVOID_MAX ){
              return FALSE;
            }

            len++;
            continue;
          }
        }
      }// while(1)
    }
    break;
  case DIR_U:   // 上に行きたいのに行けないとき
  case DIR_D:   // 下に行きたいのに行けないとき
    {
      u16  len = 1;

      // まず上から調べて次に下から調べる
      while(1)
      {
        VecFx32 tmp_pos = save_pos;

        // 左いって上下みてみる
        EvSetFrontPos( DIR_L, &tmp_pos, len );
        EvSetFrontPos( dir, &tmp_pos, 1 );

        // 障害物チェック
        if( !(EvCheckAttr( p_param, tmp_pos )) )
        {
          // 方向別アニメコマンドセット
          EvAcmdSet( p_list, DIR_L, p_list_num, len, p_param );
          EvAcmdSet( p_list, dir, p_list_num, 1, p_param );
          // 座標も同時に移動させる
          VEC_Set( p_pos, tmp_pos.x, tmp_pos.y, tmp_pos.z );
          break;
        }else
        {
          tmp_pos = save_pos;
          // 左がだめだったので右から調べてみる
          EvSetFrontPos( DIR_R, &tmp_pos, len );
          EvSetFrontPos( dir, &tmp_pos, 1 );

          if( !(EvCheckAttr( p_param, tmp_pos )) )
          {
            // 方向別アニメコマンドセット
            EvAcmdSet( p_list, DIR_R, p_list_num, len, p_param );
            EvAcmdSet( p_list, dir, p_list_num, 1, p_param );
            // 座標も同時に移動させる
            VEC_Set( p_pos, tmp_pos.x, tmp_pos.y, tmp_pos.z );
            break;
          }else
          {
            // エラーチェック
            if( len > GRID_AVOID_MAX ){
              return FALSE;
            }

            len++;
            continue;
          }
        }
      }// while(1)
    }
    break;
  default:
    GF_ASSERT(0);
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ラインにそって一歩進む
 *	        障害物があってもよけて一歩進む
 *
 *	@param	LINE_TYPE line    進んでいるライン  LINE_X_LEFTとか
 *	@param	*p_fieldMap_wk    フィールドマップワーク
 *	@param	*p_pos            キャラの実座標
 *	@param	*p_list           アニメコマンドリストへのポインタ
 *	@param	*p_list_num       アニメコマンドリストの数へのポインタ
 */
//-----------------------------------------------------------------------------
static void EvLineStep( LINE_TYPE line, VecFx32 *p_pos, MMDL_ACMD_LIST *p_list, u16 *p_list_num, ANIME_AUTO_PARAM *p_param )
{
  DIR_TYPE  dir;
  VecFx32   tmp_pos;

  // 位置保存
  VEC_Set( &tmp_pos, p_pos->x, p_pos->y, p_pos->z );

  // 進む方向決める
  switch( line ){
  case LINE_X_LEFT:   dir = DIR_L;  break;
  case LINE_X_RIGHT:  dir = DIR_R;  break;
  case LINE_Z_UP:     dir = DIR_U;  break;
  case LINE_Z_DOWN:   dir = DIR_D;  break;
  default:
                      GF_ASSERT(0);
  }

  // 一歩先を調べて、進めるかチェック
  {
    EvSetFrontPos( dir, &tmp_pos, 1 );

    // 障害物チェック
    if( EvCheckAttr( p_param, tmp_pos ) )
    {
      BOOL ret;
      ret = EvAttrHit( dir, p_pos, p_list, p_list_num, p_param );
      GF_ASSERT_MSG(ret, "障害物避けれない\n");
      ICHI_Printf("進入禁止\t");
    }else
    {
      // 方向別アニメコマンドセット
      EvAcmdSet( p_list, dir, p_list_num, 1, p_param );
      // 座標も同時に移動させる
      EvSetFrontPos( dir, p_pos, 1 );

      ICHI_Printf("進入OK\t");
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ライン合わせチェック
 *	        ラインを合わせる処理もする
 *
 *	@param	LINE_TYPE line  進んでいるライン  LINE_X_LEFTとか
 *	@param	*p_fieldMap_wk  フィールドマップワーク
 *	@param	old_line        過去(正式)なライン
 *	@param	now_line        現在いるライン
 *	@param	*p_list         アニメコマンドリストへのポインタ
 *	@param	*p_list_num     アニメコマンドリストの数へのポインタ
 *
 *	@return TRUE:ラインが合わせれた FALSE:合わせれない
 */
//-----------------------------------------------------------------------------
static BOOL EvLineCheck( LINE_TYPE line, fx32 old_line, fx32 now_line, VecFx32 *p_pos, MMDL_ACMD_LIST *p_list, u16 *p_list_num, ANIME_AUTO_PARAM *p_param )
{
  DIR_TYPE  dir;
  VecFx32   tmp_pos;
  s16       diff;   // どれだけ離れいるか

  diff = FX32_TO_GRID(now_line) - FX32_TO_GRID(old_line);

  ICHI_Printf("ラインの差は%d\n", diff);

  VEC_Set( &tmp_pos, p_pos->x, p_pos->y, p_pos->z );

  // 進む方向みて、ずれている方向も調べる
  switch( line ){
  case LINE_X_LEFT:   // 上か下にずれている
  case LINE_X_RIGHT:
    {
      if( diff > 0 ){dir = DIR_U;}
      else          {dir = DIR_D;}
    }
    break;
  case LINE_Z_UP:
  case LINE_Z_DOWN:
    {
      if( diff > 0 ){ dir = DIR_L; }
      else          { dir = DIR_R; }
    }
    break;
  default:
    GF_ASSERT(0);
  }

  // ラインに戻れるかチェック
  // 戻れるなら、離れている距離分進む
  {
    u16 len = GFL_STD_Abs( diff );

    EvSetFrontPos( dir, &tmp_pos, len );

    // 障害物チェック
    if( EvCheckAttr( p_param, tmp_pos ) )
    {
      return FALSE;
    }else
    {
      // 方向別アニメコマンドセット
      EvAcmdSet( p_list, dir, p_list_num, len, p_param );
      // 座標も同時に移動させる
      EvSetFrontPos( dir, p_pos, len );
      return TRUE;
    }
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  X軸かZ軸の移動 first_lineで変わる
 *
 *	@param	*p_list         アニメコマンドリストへのポインタ
 *	@param	*p_list_num     アニメコマンドリストの数へのポインタ
 *	@param	*p_pos          現在位置へのポインタ
 *	@param	*p_tpos         指定位置へのポインタ
 *	@param	*p_param        パラメータへのポインタ
 */
//-----------------------------------------------------------------------------
static void EvLineCalc( MMDL_ACMD_LIST *p_list, u16 *p_list_num, VecFx32 *p_pos, VecFx32 *p_tpos, ANIME_AUTO_PARAM *p_param )
{
  LINE_TYPE line;
  fx32      line_val; // ラインの値
  fx32      *p_line_now;
  fx32      *p_pos_tmp;
  fx32      *p_tpos_tmp;

  // Xライン
  if( p_param->first_line == DIR_LR )
  {
    line_val = p_pos->z; // ラインの値
    p_line_now = &p_pos->z;

    // 左右どちらに移動か？
    line = (p_tpos->x > p_pos->x)?(LINE_X_RIGHT):(LINE_X_LEFT); 

    // アドレス保存
    p_pos_tmp = &p_pos->x;
    p_tpos_tmp= &p_tpos->x;
  }
  // Zライン
  else
  {
    line_val = p_pos->x;   // ラインの値
    p_line_now = &p_pos->x;

    // 左右どちらに移動か？
    line = (p_tpos->z > p_pos->z)?(LINE_Z_DOWN):(LINE_Z_UP); 

    // アドレス保存
    p_pos_tmp = &p_pos->z;
    p_tpos_tmp= &p_tpos->z;
  }


  while( FX32_TO_GRID(*p_pos_tmp) != FX32_TO_GRID(*p_tpos_tmp) )
  {
    // 一歩進む
    EvLineStep( line, p_pos, p_list, p_list_num, p_param );

    // ラインが合っているかチェック
    // ラインが違う、かつまだ到達座標きていない
    if( (*p_line_now != line_val) && (FX32_TO_GRID(*p_pos_tmp) != FX32_TO_GRID(*p_tpos_tmp)) )
    {
      BOOL ret;

      ICHI_Printf("==========ラインチェック=========\n");
      ret = EvLineCheck( line, line_val, *p_line_now, p_pos, p_list, p_list_num, p_param );

      ICHI_Printf("ラインが合っていない\n");
      if( ret == TRUE ){
        line_val = *p_line_now;
        ICHI_Printf("ラインを合わすが出来た\n");
      }
    }

    // エラーチェック
    // 指定座標を過ぎていたらエラーにする→回り込みに対応していない為
    if( line == LINE_X_RIGHT )
    {
      GF_ASSERT_MSG( (FX32_TO_GRID(*p_tpos_tmp) >= FX32_TO_GRID(*p_pos_tmp)), "指定X座標を通り過ぎた");
    }else if( line == LINE_X_LEFT )
    {
      GF_ASSERT_MSG( (FX32_TO_GRID(*p_tpos_tmp) <= FX32_TO_GRID(*p_pos_tmp)), "指定X座標を通り過ぎた");
    }else if( line == LINE_Z_DOWN )
    {
      GF_ASSERT_MSG( (FX32_TO_GRID(*p_tpos_tmp) >= FX32_TO_GRID(*p_pos_tmp)), "指定Z座標を通り過ぎた");
    }else if( line == LINE_Z_UP )
    {
      GF_ASSERT_MSG( (FX32_TO_GRID(*p_tpos_tmp) <= FX32_TO_GRID(*p_pos_tmp)), "指定Z座標を通り過ぎた");
    }
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  歩行経路を算出
 *	        
 *	        STEP1.指定位置とラインを合わせる(XかZ)
 *	        STEP2.指定位置に向かう
 *	        STEP3.障害物があったら、避ける方向を判定して避ける方に進む
 *	        STEP4.STEP2と同じ方向に進めるようになったら進む
 *	        STEP5.元のラインに戻れるかチェックして、戻れるようになったら戻る
 *	        STEP6. STEP2に戻る
 *
 *	@param	SCRCMD_WORK *p_wk       スクリプトコマンドのワーク
 *	@param	MMDL_ACMD_LIST *p_list  アニメーションコマンドのリストのポインタ
 *	@param	p_mmdl  キャラクターモデル
 *	@param	tx      指定グリッドX座標
 *	@param	tz      指定グリッドZ座標
 *  
 *  @return アニメーションコマンドの総数
 *  
 *  算出された経路はアニメーションコマンドの配列としてp_listに入る
 */
//-----------------------------------------------------------------------------
static u32 EvRouteCalc( MMDL_ACMD_LIST *p_list, MMDL *p_mmdl, ANIME_AUTO_PARAM *p_param )
{
  u16     list_num = 0;   // アニメーションの数
  u8      dir = 0;        // 進む方向
  u8      line;           // 合っているライン
  VecFx32 pos;            // モデルの実座標
  VecFx32 tpos;           // 指定位置

  // モデルの実座標取得
  MMDL_GetVectorPos( p_mmdl, &pos);

  // 指定位置の座標をグリッド座標から実座標に
  tpos.x = GRID_TO_FX32( p_param->target_x );
  tpos.z = GRID_TO_FX32( p_param->target_z );

  // 移動する前に指定した位置が障害物かチェック
  if( EvCheckAttr( p_param, tpos ) )
  {
    GF_ASSERT_MSG(0, "指定位置%d:%d Error\n", p_param->target_x, p_param->target_z);
  }

  // 指定位置移動までループ
  // ラインごと
  EvLineCalc( p_list, &list_num, &pos, &tpos, p_param );
  // ライン変更
  p_param->first_line = (p_param->first_line == DIR_LR) ? DIR_UD : DIR_LR;
  EvLineCalc( p_list, &list_num, &pos, &tpos, p_param );

  // 最後は絶対にアニメーション終了コマンド入れる
  p_list[ list_num ].code = ACMD_AUTO_END;
  p_list[ list_num ].num  = 0;

  ICHI_Printf("探索終了 list_num:%d\n", list_num);

  return list_num;
}


// NPC歩行経路の自動生成
//----------------------------------------------------------------------------
/**
 * @brief  動作モデル自動アニメーション
 *
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdObjAnimeAuto( VMHANDLE *core, void *wk )
{
  GFL_TCB *anm_tcb;   // 登録用タスク
  MMDL *p_mmdl;       // 対象のフィールドOBJのポインタ
  SCRCMD_WORK *work = wk;
  u16 obj_id = SCRCMD_GetVMWorkValue(core,work); //obj ID

  // フィールドマップ
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  HEAPID  heap_id = FIELDMAP_GetHeapID( fparam->fieldMap );

  // OBJIDからモデル取得
  p_mmdl = FieldObjPtrGetByObjId( work, obj_id );

  // エラーチェック
  if( p_mmdl == NULL ){
    OS_Printf( "obj_id = %d\n", obj_id );
    GF_ASSERT( (0) && "対象のフィールドOBJのポインタ取得失敗！" );
    return VMCMD_RESULT_CONTINUE;        //08.06.12 プラチナで追加
  }

  //アニメーションコマンドリストを探索し、タスクにセットする
  {
    MMDL_ACMD_LIST    *p_list;
    ANIME_AUTO_PARAM  param;

    // メモリ作成
    p_list = GFL_HEAP_AllocClearMemory( heap_id, (MMDL_ACMD_LIST_SIZE * ACMD_LIST_MAX) );

    // パラメータセット
    param.p_scwk  = work;
    param.p_fieldMap_wk = fparam->fieldMap;
    param.target_x  = SCRCMD_GetVMWorkValue( core, work );;
    param.target_z  = SCRCMD_GetVMWorkValue( core, work );;
    param.force_type= SCRCMD_GetVMWorkValue( core, work );
    param.ac_frame  = SCRCMD_GetVMWorkValue( core, work );;
    param.first_line= SCRCMD_GetVMWorkValue( core, work );;

    // 経路算出
    EvRouteCalc( p_list, p_mmdl, &param );

    // 算出結果セット
    anm_tcb = MMDL_SetAcmdList( p_mmdl, p_list );
  }

  //TCBセット
  SCRCMD_WORK_SetMMdlAnmTCB( work, anm_tcb );

  return VMCMD_RESULT_CONTINUE;
}
// <<

//--------------------------------------------------------------
/**
 * 動作モデルアニメーション終了待ち　ウェイと部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvObjAnimeWait(VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;

  if( SCRCMD_WORK_CheckMMdlAnmTCB(work) == FALSE ){
    return TRUE;
  }

  return FALSE;
}

//--------------------------------------------------------------
/**
 * アニメーション終了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjAnimeWait( VMHANDLE * core, void *wk )
{
  VMCMD_SetWait( core, EvObjAnimeWait );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 動作コード取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMoveCodeGet( VMHANDLE *core, void *wk )
{
  MMDL *mmdl;
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  u16 *ret_wk  = SCRCMD_GetVMWork( core, work );

  *ret_wk = 0;
  mmdl = MMDLSYS_SearchOBJID( mmdlsys, SCRCMD_GetVMWorkValue(core,work) );
  GF_ASSERT( mmdl != NULL && "SCRCMD GET MOVECODE NON OBJ" );
  *ret_wk = MMDL_GetMoveCode( mmdl );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 動作コード変更
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMoveCodeChange( VMHANDLE *core, void *wk )
{
  MMDL *mmdl;
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  u16 id = SCRCMD_GetVMWorkValue( core, work );
  u16 code = SCRCMD_GetVMWorkValue( core, work );

  mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
  GF_ASSERT( mmdl != NULL && "SCRCMD GET MOVECODE NON OBJ" );
  MMDL_ChangeMoveCode( mmdl, code );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * OBJの位置情報取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjPosGet( VMHANDLE *core, void *wk )
{
  u16 *x,*z;
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *mmdl = MMDLSYS_SearchOBJID(
      mmdlsys, SCRCMD_GetVMWorkValue(core,work) );

  x = SCRCMD_GetVMWork( core, work );
  z = SCRCMD_GetVMWork( core, work );
  *x = MMDL_GetGridPosX( mmdl );
  *z = MMDL_GetGridPosZ( mmdl );
#ifdef PM_DEBUG
  //レール上では使用不可(正確な値が取れない)
  {
    SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( SCRCMD_WORK_GetScriptWork( work ) );
    if( FIELDMAP_GetBaseSystemType( fparam->fieldMap ) == FLDMAP_BASESYS_RAIL ){
      GF_ASSERT_MSG(0,"_OBJ_POS_GET:Can't use on rail\n");
    }
  }
#endif
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 自機の位置情報取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerPosGet( VMHANDLE *core, void *wk )
{
  u16 *x,*z;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fparam->fieldMap );
  MMDL *mmdl = FIELD_PLAYER_GetMMdl( fld_player );

  x = SCRCMD_GetVMWork( core, work );
  z = SCRCMD_GetVMWork( core, work );
  *x = MMDL_GetGridPosX( mmdl );
  *z = MMDL_GetGridPosZ( mmdl );

#ifdef PM_DEBUG
  //レール上では使用不可(正確な値が取れない)
  if( FIELDMAP_GetBaseSystemType( fparam->fieldMap ) == FLDMAP_BASESYS_RAIL ){
    GF_ASSERT_MSG(0,"_PLAYER_POS_GET:Can't use on rail\n");
  }
#endif
  IWASAWA_Printf("PlayerPosGet (%d,%d)\n",*x,*z);
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief 自機の向き取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerDirGet( VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDL *player = scmd_GetMMdlPlayer( work );

  u16 * ret_wk = SCRCMD_GetVMWork( core, wk );

  *ret_wk = MMDL_GetDirDisp( player );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * OBJを追加
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjAdd( VMHANDLE *core, void *wk )
{
  MMDL *mmdl;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );
  u16 gx = SCRCMD_GetVMWorkValue( core, work );
  u16 gz = SCRCMD_GetVMWorkValue( core, work );
  u16 dir = SCRCMD_GetVMWorkValue( core, work );
  u16 id = SCRCMD_GetVMWorkValue( core, work );
  u16 code = SCRCMD_GetVMWorkValue( core, work );
  u16 move = SCRCMD_GetVMWorkValue( core, work );
  int zone_id = FIELDMAP_GetZoneID( fparam->fieldMap );
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *evwork =  GAMEDATA_GetEventWork( gdata );

  code = MMDL_TOOL_GetWorkOBJCode( evwork, code );
  mmdl = MMDLSYS_AddMMdlParam(
      mmdlsys, gx, gz, dir, id, code, move, zone_id );

  if( mmdl != NULL ){ //追加されたOBJは即ポーズさせる
    MMDL_OnMoveBitMoveProcPause( mmdl );
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * OBJを削除
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjDel( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  u16 id = SCRCMD_GetVMWorkValue( core, work );
  MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
  if ( mmdl == NULL ) {
    GF_ASSERT_MSG( 0, "OBJ DEL 対象のOBJ(%d)が居ません\n", id );
  } else {
    MMDL_Delete( mmdl );
  }
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * OBJを追加 配置データから
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjAddEvent( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );
  u16 count = EVENTDATA_GetNpcCount( evdata );
  PLAYER_WORK *player = GAMEDATA_GetMyPlayerWork( gdata );
  int zone_id = PLAYERWORK_getZoneID( player );
  u16 id = SCRCMD_GetVMWorkValue( core, work );

  if( count ){
    EVENTWORK *evwork =  GAMEDATA_GetEventWork( gdata );
    const MMDL_HEADER *header = EVENTDATA_GetNpcData( evdata );
    MMDL *mmdl = MMDLSYS_AddMMdlHeaderID(
        mmdlsys, header, zone_id, count, evwork, id );
    if( mmdl != NULL ){ //追加されたOBJは即ポーズさせる
      MMDL_OnMoveBitMoveProcPause( mmdl );
    }
    GF_ASSERT( mmdl != NULL );
  }else{
    GF_ASSERT( 0 );
  }

  return( VMCMD_RESULT_CONTINUE );
}

//--------------------------------------------------------------
/**
 * OBJを削除　OBJ指定イベントフラグをONに
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
#if 0
VMCMD_RESULT EvCmdObjDelEvent( VMHANDLE *core, void *wk )
{
  MMDL *mmdl;
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTWORK *evwork =  GAMEDATA_GetEventWork( gdata );
  u16 id = SCRCMD_GetVMWorkValue( core, work );

  mmdl = MMDLSYS_SearchOBJID( mmdlsys, id );
  GF_ASSERT( mmdl != NULL );

  MMDL_DeleteEvent( mmdl, evwork );
  return( VMCMD_RESULT_CONTINUE );
}
#endif

//--------------------------------------------------------------
/**
 * OBJデータの座標情報を変更
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjPosChange( VMHANDLE *core, void *wk )
{
  MMDL *mmdl;
  SCRCMD_WORK *work = wk;
  u16 obj_id	= SCRCMD_GetVMWorkValue( core, work );
  s16 x		= SCRCMD_GetVMWorkValue(core,work);
  s16 y		= SCRCMD_GetVMWorkValue(core,work);
  s16 z		= SCRCMD_GetVMWorkValue(core,work);
  u16 dir		= SCRCMD_GetVMWorkValue(core,work);
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );

  //対象のフィールドOBJのポインタ取得
  mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );
  if ( mmdl == NULL ) {
    GF_ASSERT_MSG( 0, "ERROR OBJ ID(%d)", obj_id);
  } else {
    MMDL_InitGridPosition( mmdl, x, y, z, dir );
  }
  return VMCMD_RESULT_CONTINUE;
}

#if 0
//--------------------------------------------------------------
/**
 * SXYデータの座標情報を変更
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
static BOOL EvCmdSxyPosChange( VM_MACHINE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 id = SCRCMD_GetVMWorkValue(core,work);
  u16 gx = SCRCMD_GetVMWorkValue(core,work);
  u16 gz = SCRCMD_GetVMWorkValue(core,work);
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );

  EventData_NpcDataPosChange( core->fsys, id, gx, gz );
  return 0;
}
#endif

//--------------------------------------------------------------
/**
 * OBJに指定されているイベントフラグを取得
 * @param	core		仮想マシン制御構造体へのポインタ
 * @return	"0"
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjEventFlagGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 obj_id	= SCRCMD_GetVMWorkValue( core, work );
  u16 *ret_wk = SCRCMD_GetVMWork( core, work );
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );
  GF_ASSERT( mmdl != NULL && "OBJ GET EVENT FLAG 対象のOBJが居ません\n" );

  if( mmdl != NULL ){
    *ret_wk = MMDL_GetEventFlag( mmdl );
  }

  return( VMCMD_RESULT_CONTINUE );
}

//--------------------------------------------------------------
/**
 * OBJに指定されているパラメータを取得
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjParamGet( VMHANDLE *core, void *wk )
{
  SDK_COMPILER_ASSERT( SCR_OBJPARAM0 == MMDL_PARAM_0 );
  SDK_COMPILER_ASSERT( SCR_OBJPARAM1 == MMDL_PARAM_1 );
  SDK_COMPILER_ASSERT( SCR_OBJPARAM2 == MMDL_PARAM_2 );
  SCRCMD_WORK *work = wk;
  u16 obj_id	= SCRCMD_GetVMWorkValue( core, work );
  u16 prm_no  = SCRCMD_GetVMWorkValue( core, work );
  u16 *ret_wk = SCRCMD_GetVMWork( core, work );
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );

  if( prm_no > MMDL_PARAM_2 ) {
    GF_ASSERT_MSG(0, "EvCmdObjParamGet:param no(%d)が大きすぎます\n", prm_no );
  } else if( mmdl == NULL ){
    GF_ASSERT_MSG(0, "EvCmdObjParamGet:OBJ ID(%d)が居ません\n", obj_id );
  } else {
    *ret_wk = MMDL_GetParam( mmdl, prm_no );
  }
  return( VMCMD_RESULT_CONTINUE );
}

//--------------------------------------------------------------
/**
 * OBJをゾーン切り替えでも削除されないOBJに指定
 * @param
 * @retval
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjSetNotDelZoneChange( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 obj_id	= SCRCMD_GetVMWorkValue( core, work );
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );

  if( mmdl != NULL ){
    MMDL_SetStatusBitNotZoneDelete( mmdl, TRUE );
  }else{
    GF_ASSERT( 0 );
  }

  return( VMCMD_RESULT_CONTINUE );
}

//======================================================================
//  動作モデル　イベント関連
//======================================================================
//--------------------------------------------------------------
/**
 * @brief  OBJ動作終了待ち　ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitTalkObj( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDL *fmmdl = SCRIPT_GetTargetObj( sc );
  MMDL *player = scmd_GetMMdlPlayer( work );
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );

  //自機動作停止チェック
  if( CheckStepWatchBit(PLAYER_BIT) &&
      MMDL_CheckEndAcmd(player) == TRUE ){
    MMDL_OnMoveBitMoveProcPause( player );
    ResetStepWatchBit(PLAYER_BIT);
  }

  //話しかけ対象動作停止チェック
  if( CheckStepWatchBit(OTHER_BIT) &&
      MMDL_CheckMoveBitMove(fmmdl) == FALSE ){
    MMDL_OnMoveBitMoveProcPause( fmmdl );
    ResetStepWatchBit(OTHER_BIT);
  }

  //自機の連れ歩き動作停止チェック
  if( CheckStepWatchBit(PLAYER_PAIR_BIT) ){
    MMDL *player_pair = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );

    if( MMDL_CheckMoveBitMove(player_pair) == FALSE ){
      MMDL_OnMoveBitMoveProcPause( player_pair );
      ResetStepWatchBit(PLAYER_PAIR_BIT);
    }
  }

  //話しかけ対象の連れ歩き動作停止チェック
  if( CheckStepWatchBit(OTHER_PAIR_BIT) ){
    MMDL *other_pair = MMDL_SearchMovePair( fmmdl );
    if( other_pair == NULL ){
      GF_ASSERT( 0 );
      ResetStepWatchBit(OTHER_PAIR_BIT);
    }else{
      if( MMDL_CheckMoveBitMove(other_pair) == FALSE ) {
        MMDL_OnMoveBitMoveProcPause( other_pair );
        ResetStepWatchBit(OTHER_PAIR_BIT);
      }
    }
  }

  if( step_watch_bit == 0 ){
    return TRUE;
  }

  return FALSE;
}

//--------------------------------------------------------------
/**
 * 会話イベント用全OBJ動作停止
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdTalkObjPauseAll( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *fmmdl = SCRIPT_GetTargetObj( sc );
  MMDL *player = scmd_GetMMdlPlayer( work );
  MMDL *player_pair = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );
  MMDL *other_pair = MMDL_SearchMovePair( fmmdl );

  InitStepWatchBit();
  MMDLSYS_PauseMoveProc( fmmdlsys );

  if( MMDL_CheckEndAcmd(player) == FALSE ){
    SetStepWatchBit(PLAYER_BIT);
    MMDL_OffMoveBitMoveProcPause( player );
  }

  if( fmmdl != NULL ){
    if( MMDL_CheckMoveBitMove(fmmdl) == TRUE ){
      SetStepWatchBit(OTHER_BIT);
      MMDL_OffMoveBitMoveProcPause( fmmdl );
    }
  }

  if( player_pair ){
    if( MMDL_CheckMoveBitMove(player_pair) == TRUE ){
      SetStepWatchBit(PLAYER_PAIR_BIT);
      MMDL_OffMoveBitMoveProcPause( player_pair );
      IWASAWA_Printf("連れ歩き動作待ち\n");
    }
  }

  if( other_pair ){
    if( MMDL_CheckMoveBitMove(other_pair) == TRUE ){
      SetStepWatchBit(OTHER_PAIR_BIT);
      MMDL_OffMoveBitMoveProcPause( other_pair );
    }
  }

  VMCMD_SetWait( core, EvWaitTalkObj );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 全OBJ動作停止
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
#if 0
static VMCMD_RESULT EvCmdObjPauseAll( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc;
  MMDL *fmmdl;

  sc = SCRCMD_WORK_GetScriptWork( work );
  fmmdl = SCRIPT_GetTargetObj( sc );

  if( (fmmdl) == NULL ){
    MMDLSYS *fmmdlsys;
    fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
    MMDLSYS_PauseMoveProc( fmmdlsys );

    //08.06.18
    //話しかけの対象がいないBGやPOSの時
    //連れ歩きOBJの移動動作中かのチェックをしていない
    //
    //ふれあい広場などで、連れ歩きOBJに対して、
    //スクリプトでアニメを発行すると、
    //アニメが行われず終了待ちにいかないでループしてしまう

    {
      FIELD_OBJ_PTR player_pair =
        FieldOBJSys_MoveCodeSearch( fsys->fldobjsys, MV_PAIR );
      //ペアが存在している
      if (player_pair) {
        //連れ歩きフラグが立っていて、移動動作中なら
        if( SysFlag_PairCheck(
              SaveData_GetEventWork(fsys->savedata)) == 1
            && FieldOBJ_StatusBitCheck_Move(player_pair) != 0) {

          //ペアの動作ポーズ解除
          FieldOBJ_MovePauseClear( player_pair );

          //移動動作の終了待ちをセット
          VM_SetWait( core, EvWaitPairObj );
          return VMCMD_RESULT_SUSPEND;
        }
      }
    }
  }else{
    EvCmdTalkObjPauseAll( core, wk );
  }

  return VMCMD_RESULT_SUSPEND;
}
#endif

//--------------------------------------------------------------
/**
 * @brief	ペアOBJ動作終了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitPairObj( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *player_pair = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );

	//自機の連れ歩き動作停止チェック
  if( MMDL_CheckMoveBitMove(player_pair) == FALSE ){
    MMDL_OnMoveBitMoveProcPause( player_pair );
		return TRUE;  //移動動作が終了した
	}

	return FALSE;
}

VMCMD_RESULT SCRCMD_SUB_ObjPauseAll( VMHANDLE *core, SCRCMD_WORK *work )
{
  SCRIPT_WORK *sc;
  MMDL *fmmdl;

  sc = SCRCMD_WORK_GetScriptWork( work );
  fmmdl = SCRIPT_GetTargetObj( sc );

  if( (fmmdl) == NULL ){
    MMDLSYS *fmmdlsys;
    fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
    MMDLSYS_PauseMoveProc( fmmdlsys );

#if 1	//10.11.10 by iwasawa
		//話しかけの対象がいないBGやPOSの時
		//連れ歩きOBJの移動動作中かのチェックをしていない
		//
		//連れ歩きOBJに対して、
		//スクリプトでアニメを発行すると、
		//アニメが行われず終了待ちにいかないでループしてしまう

		{
      MMDL *player_pair = MMDLSYS_SearchMoveCode( fmmdlsys, MV_PAIR );

			//連れ歩きがいて、移動動作中なら
			if( player_pair != NULL && MMDL_CheckMoveBitMove(player_pair) == TRUE ) {	
				//ペアの動作ポーズ解除
        MMDL_OffMoveBitMoveProcPause( player_pair );

				//移動動作の終了待ちをセット
        VMCMD_SetWait( core, EvWaitPairObj );
        return VMCMD_RESULT_SUSPEND;
			}
		}
#endif
    //プラチナ→金銀からの引継ぎ。スワンでも有効にしておく。
    //これにより、ポーズがかかった場合、連れ歩きがグリッド中心にいることが保障される反面、
    //連れ歩き動作終了まで、次のスクリプトが実行されないため、若干の待ち状態が発生する。
    //仕様としてこの動作を嫌う場合は、
    //グリッド中心まで移動しなくても、イベントを進行させてしまうようにするため
		//ここの#if部分を削除する。ならびに、終了待ち関数EvWaitPairObjも必要なくなる

  }else{
    return( EvCmdTalkObjPauseAll(core,work) );
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 全OBJ動作再開
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
#if 0
static VMCMD_RESULT EvCmdObjPauseClearAll( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDLSYS_ClearPauseMoveProc( fmmdlsys );
  return VMCMD_RESULT_SUSPEND;
}
#endif
void SCRCMD_SUB_PauseClearAll( SCRCMD_WORK *work )
{
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDLSYS_ClearPauseMoveProc( fmmdlsys );
}

//--------------------------------------------------------------
/**
 * 話しかけたOBJ自機方向への振り向き
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdObjTurn( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDL *jiki,*fmmdl;
  u16 dir;

  jiki = scmd_GetMMdlPlayer( work );

  dir = MMDL_GetDirDisp( jiki );

  dir = MMDL_TOOL_FlipDir( dir );
  fmmdl = SCRIPT_GetTargetObj( sc );

  if( fmmdl == NULL ){
    GF_ASSERT( 0 );
  } else {
    MMDL_SetDirDisp( fmmdl, dir );
    MMDL_SetDrawStatus( fmmdl, DRAW_STA_STOP );
  }
  return VMCMD_RESULT_CONTINUE;
}
//--------------------------------------------------------------
/**
 * @brief 指定位置のOBJを検索する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetObjID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fldparam = SCRIPT_GetFieldParam( sc );
  FIELDMAP_WORK *fieldmap = fldparam->fieldMap;
  u16 gx, gy, gz;
  u16 *ret_id     = SCRCMD_GetVMWork( core, work );
  u16 *ret_valid  = SCRCMD_GetVMWork( core, work );
  MMDL *mmdl;
  u16 dir;
  VecFx32 pos;
  fx32 fy;

  gx = SCRCMD_GetVMWorkValue( core, work );
  gy = SCRCMD_GetVMWorkValue( core, work );
  gz = SCRCMD_GetVMWorkValue( core, work );
  fy = GRID_TO_FX32( gy );
  mmdl = MMDLSYS_SearchGridPosEx(
      FIELDMAP_GetMMdlSys(fieldmap), gx, gz, fy, GRID_HALF_FX32, FALSE );
  if ( mmdl )
  {
    *ret_valid = TRUE;
    *ret_id    = MMDL_GetOBJID( mmdl );
  }
  else
  {
    *ret_valid = FALSE;
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief プレイヤーの前方1グリッドにあるOBJを検索する
 * @param core 仮想マシン制御構造体へのポインタ
 * @param wk
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetFrontObjID( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_PLAYER*    player = FIELDMAP_GetFieldPlayer( fieldmap );
  u16*             ret_id = SCRCMD_GetVMWork( core, work );  // コマンド第一引数(オブジェIDの格納先)
  u16*          ret_valid = SCRCMD_GetVMWork( core, work );  // コマンド第二引数(オブジェの有無)
  MMDL* mmdl;

  // 前方にある動作モデルを取得
  mmdl = FIELD_PLAYER_GetFrontMMdlEx( player, GRID_HALF_FX32 );
  // 結果を格納
  if( mmdl )
  {
    *ret_valid = TRUE;
    *ret_id    = MMDL_GetOBJID( mmdl );
  }
  else
  {
    *ret_valid = FALSE;
  }
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  自機
//======================================================================
//--------------------------------------------------------------
/**
 * 自機リクエスト消化待ち　ウェイと部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitPlayerRequest(VMHANDLE * core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fldparam = SCRIPT_GetFieldParam( sc );
  FIELDMAP_WORK *fieldmap = fldparam->fieldMap;
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );

  if( FIELD_PLAYER_UpdateRequest(fld_player) == TRUE ){
    return( TRUE );
  }

  return( FALSE );
}

//--------------------------------------------------------------
/**
 * 自機リクエスト
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerRequest( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fldparam = SCRIPT_GetFieldParam( sc );
  u16 req = VMGetU16( core );
  FIELDMAP_WORK *fieldmap = fldparam->fieldMap;
  FIELD_PLAYER* fld_player = FIELDMAP_GetFieldPlayer( fieldmap );

  FIELD_PLAYER_SetRequest( fld_player, req );

  if( FIELD_PLAYER_UpdateRequest(fld_player) == TRUE ){
    return VMCMD_RESULT_CONTINUE;
  }

  VMCMD_SetWait( core, EvWaitPlayerRequest );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 自機の上下動演出リクエスト
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerUpDownEffect( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fldparam = SCRIPT_GetFieldParam( sc );
  FIELDMAP_WORK *fieldmap = fldparam->fieldMap;
  FIELD_TASK* move;
  FIELD_TASK_MAN* man = FIELDMAP_GetTaskManager( fieldmap );
  VecFx32 vec;
  u16 type = SCRCMD_GetVMWorkValue( core, work );
  u16 frame_param = SCRCMD_GetVMWorkValue( core, work );
  u16 length = SCRCMD_GetVMWorkValue( core, work );
  u16 way = SCRCMD_GetVMWorkValue( core, work );
  int frame;
  if (type == 0) {
    frame = frame_param;
  } else {
    frame = - frame_param;
  }

  if(way == 0){
    VEC_Set( &vec, 0, (- length) <<FX32_SHIFT, 0 );
  }else{
    VEC_Set( &vec, 0, (length) <<FX32_SHIFT, 0 );
  }
  //move     = FIELD_TASK_TransDrawOffset( fieldmap, frame, &vec );
  {
    FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer( fieldmap );
    move     = FIELD_TASK_TransPos( fieldmap, frame, &vec, FIELD_PLAYER_GetMMdl( player ) );
  }
  FIELD_TASK_MAN_AddTask( man, move, NULL );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 自機の上下動リクエスト
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerUpDownPos( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fldparam = SCRIPT_GetFieldParam( sc );
  FIELDMAP_WORK *fieldmap = fldparam->fieldMap;
  FIELD_TASK* move;
  FIELD_TASK_MAN* man = FIELDMAP_GetTaskManager( fieldmap );
  VecFx32 vec;
  u16 type = SCRCMD_GetVMWorkValue( core, work );
  u16 frame_param = SCRCMD_GetVMWorkValue( core, work );
  u16 length = SCRCMD_GetVMWorkValue( core, work );
  u16 way = SCRCMD_GetVMWorkValue( core, work );
  int frame;
  if (type == 0) {
    frame = frame_param;
  } else {
    frame = - frame_param;
  }

  if(way == 0){
    VEC_Set( &vec, 0, (- length) <<FX32_SHIFT, 0 );
  }else{
    VEC_Set( &vec, 0, (length) <<FX32_SHIFT, 0 );
  }
  //move     = FIELD_TASK_TransDrawOffset( fieldmap, frame, &vec );
  {
    FIELD_PLAYER * player = FIELDMAP_GetFieldPlayer( fieldmap );
    move     = FIELD_TASK_TransPos( fieldmap, frame, &vec, FIELD_PLAYER_GetMMdl( player ) );
  }
  FIELD_TASK_MAN_AddTask( man, move, NULL );

  return VMCMD_RESULT_CONTINUE;
}



//======================================================================
//
//
//    ツール関数
//
//
//======================================================================
//--------------------------------------------------------------
/**
 * 自機MMDL取得
 * @param work SCRCMD_WORK
 * @retval
 */
//--------------------------------------------------------------
static MMDL * scmd_GetMMdlPlayer( SCRCMD_WORK *work )
{
  MMDLSYS *fmmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *fmmdl = MMDLSYS_SearchMMdlPlayer( fmmdlsys );
  return( fmmdl );
}

//--------------------------------------------------------------
/**
 * 方向起動ＰＯＳ用向き直り処理
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPosPlayerTurn( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fldparam = SCRIPT_GetFieldParam( sc );
  FIELDMAP_WORK *fieldmap = fldparam->fieldMap;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );

  u16 dir;
  {
    int i;
    VecFx32 pos;
    FIELD_PLAYER *fld_player;
    u16 player_dir;

    u16 dir_base[4] = {DIR_UP,DIR_DOWN,DIR_LEFT,DIR_RIGHT};

    EVENTDATA_SYSTEM *evdata = GAMEDATA_GetEventData( gdata );
    EVENTWORK *evwork = GAMEDATA_GetEventWork( gdata );

    fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
    FIELD_PLAYER_GetPos( fld_player, &pos );
    for (i=0;i<4;i++)
    {
      const POS_EVENT_DATA *evt_data = EVENTDATA_GetPosEvent(
          evdata, evwork, &pos, dir_base[i] );
      if ( evt_data != NULL )
      {
        dir = dir_base[i];
        break;
      }
    }

    //自機の見た目の向きを取得
    player_dir = FIELD_PLAYER_GetDir( fld_player );
    //向きの比較をする
    if (player_dir != dir)
    {
      GMEVENT * child;
      void *tbl;
      if ( dir == DIR_UP ){
        tbl = (void*)anime_up_table;
      }else if (dir == DIR_DOWN){
        tbl = (void*)anime_down_table;
      }else if (dir == DIR_LEFT){
        tbl = (void*)anime_left_table;
      }else if (dir == DIR_RIGHT){
        tbl = (void*)anime_right_table;
      }else{
        GF_ASSERT(0);
        return VMCMD_RESULT_CONTINUE;
      }
      child = EVENT_ObjAnime( gsys, fieldmap, MMDL_ID_PLAYER, tbl );
      SCRIPT_CallEvent( sc, child );
      return VMCMD_RESULT_SUSPEND;
    }
  }

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * 自機の動作形態を取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk
 * @retval  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdGetPlayerFrom( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fldparam = SCRIPT_GetFieldParam( sc );
  FIELDMAP_WORK *fieldmap = fldparam->fieldMap;
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
  u16 *ret_wk = SCRCMD_GetVMWork( core, work );
  PLAYER_MOVE_FORM form = FIELD_PLAYER_GetMoveForm( fld_player );

  switch( form ){
  case PLAYER_MOVE_FORM_NORMAL:
    *ret_wk = SCR_PLAYER_FORM_NORMAL;
    break;
  case PLAYER_MOVE_FORM_CYCLE:
    *ret_wk = SCR_PLAYER_FORM_CYCLE;
    break;
  case PLAYER_MOVE_FORM_SWIM:
    *ret_wk = SCR_PLAYER_FORM_SWIM;
    break;
  case PLAYER_MOVE_FORM_DIVING:
    *ret_wk = SCR_PLAYER_FORM_DIVING;
    break;
  default:
    GF_ASSERT( 0 );
    *ret_wk = SCR_PLAYER_FORM_NORMAL;
  }

  return( VMCMD_RESULT_CONTINUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ハイジャンプ
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdObjHighJump( VMHANDLE *core, void *wk )
{
  MMDL *mmdl;
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  u16 obj_id	= SCRCMD_GetVMWorkValue( core, work );
  u16 gx	= SCRCMD_GetVMWorkValue( core, work );
  u16 gy	= SCRCMD_GetVMWorkValue( core, work );
  u16 gz	= SCRCMD_GetVMWorkValue( core, work );
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  GAMESYS_WORK *gsys = SCRCMD_WORK_GetGameSysWork( work );
  VecFx32 start, end;

  //対象のフィールドOBJのポインタ取得
  mmdl = MMDLSYS_SearchOBJID( mmdlsys, obj_id );
  GF_ASSERT( mmdl != NULL && "ERROR OBJ ID" );

  // 今の座標と終了座標を求める
  MMDL_GetVectorPos( mmdl, &start );
  end.x = GRID_TO_FX32( gx ) + MMDL_VEC_X_GRID_OFFS_FX32;
  end.y = GRID_TO_FX32( gy );
  end.z = GRID_TO_FX32( gz ) + MMDL_VEC_Z_GRID_OFFS_FX32;

  SCRIPT_CallEvent( sc, EVENT_HighJump(gsys, mmdl, &start, &end) );

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 自機の表示コードが自機専用のものかどうか。
 * @param  core    仮想マシン制御構造体へのポインタ
 * @param wk
 * @retval  VMCMD_RESULT
 * @note 変装中かどうかのチェック
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCheckPlayerIllegalOBJCode( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  SCRIPT_FLDPARAM *fldparam = SCRIPT_GetFieldParam( sc );
  FIELDMAP_WORK *fieldmap = fldparam->fieldMap;
  FIELD_PLAYER *fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
  u16 *ret_wk = SCRCMD_GetVMWork( core, work );

  *ret_wk = FIELD_PLAYER_CheckIllegalOBJCode( fld_player );
  return VMCMD_RESULT_CONTINUE;
}

//======================================================================
//  railマップ専用
//======================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  プレイヤーのレール位置情報を取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerRailLocationGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDL* mmdl = scmd_GetMMdlPlayer( work );
  RAIL_LOCATION rail_location;
  FIELD_RAIL_WORK* p_railWork;
  u16 *index, *ofs, *w_ofs;


  // レールのオブジェか？
  GF_ASSERT( MMDL_CheckStatusBit( mmdl, MMDL_STABIT_RAIL_MOVE ) );

  // ロケーションを返す
  index = SCRCMD_GetVMWork( core, work );
  ofs = SCRCMD_GetVMWork( core, work );
  w_ofs = SCRCMD_GetVMWork( core, work );

  p_railWork = MMDL_GetRailWork( mmdl );

  FIELD_RAIL_WORK_GetNotMinusRailParam( p_railWork, index, ofs, w_ofs );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  動作モデルのレール位置情報を取得
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdObjRailLocationGet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *mmdl = MMDLSYS_SearchOBJID(
      mmdlsys, SCRCMD_GetVMWorkValue(core,work) );
  RAIL_LOCATION rail_location;
  FIELD_RAIL_WORK* p_railWork;
  u16 *index, *ofs, *w_ofs;


  // レールのオブジェか？
  GF_ASSERT( MMDL_CheckStatusBit( mmdl, MMDL_STABIT_RAIL_MOVE ) );

  // ロケーションを返す
  index = SCRCMD_GetVMWork( core, work );
  ofs = SCRCMD_GetVMWork( core, work );
  w_ofs = SCRCMD_GetVMWork( core, work );

  p_railWork = MMDL_GetRailWork( mmdl );

  FIELD_RAIL_WORK_GetNotMinusRailParam( p_railWork, index, ofs, w_ofs );

  return VMCMD_RESULT_CONTINUE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  動作モデル　滑り降り
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdObjRailSlipDown( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *mmdl = MMDLSYS_SearchOBJID(
      mmdlsys, SCRCMD_GetVMWorkValue(core,work) );
  SCRIPT_FLDPARAM *fldparam = SCRIPT_GetFieldParam( sc );
  FIELDMAP_WORK *fieldmap = fldparam->fieldMap;

  SCRIPT_CallEvent( sc, EVENT_RailSlipDownObj(gsys, fieldmap, mmdl) );
  return VMCMD_RESULT_SUSPEND;
}


//----------------------------------------------------------------------------
/**
 *	@brief  レールロケーションの設定
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerRailLocationSet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDL* mmdl = scmd_GetMMdlPlayer( work );
  RAIL_LOCATION rail_location;
  FIELD_RAIL_WORK* p_railWork;
  u16 index, ofs, w_ofs;
  RAIL_LOCATION location;


  // レールのオブジェか？
  GF_ASSERT( MMDL_CheckStatusBit( mmdl, MMDL_STABIT_RAIL_MOVE ) );

  // ロケーションを返す
  index = SCRCMD_GetVMWorkValue( core, work );
  ofs = SCRCMD_GetVMWorkValue( core, work );
  w_ofs = SCRCMD_GetVMWorkValue( core, work );

  p_railWork = MMDL_GetRailWork( mmdl );

  FIELD_RAIL_WORK_SetNotMinusRailParam( p_railWork, index, ofs, w_ofs );
  FIELD_RAIL_WORK_GetLocation( p_railWork, &location );
  MMDL_SetRailLocation( mmdl, &location );

  return VMCMD_RESULT_CONTINUE;
}
VMCMD_RESULT EvCmdObjRailLocationSet( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  MMDLSYS *mmdlsys = SCRCMD_WORK_GetMMdlSys( work );
  MMDL *mmdl = MMDLSYS_SearchOBJID(
      mmdlsys, SCRCMD_GetVMWorkValue(core,work) );
  RAIL_LOCATION rail_location;
  FIELD_RAIL_WORK* p_railWork;
  u16 index, ofs, w_ofs;
  RAIL_LOCATION location;


  // レールのオブジェか？
  GF_ASSERT( MMDL_CheckStatusBit( mmdl, MMDL_STABIT_RAIL_MOVE ) );

  // ロケーションを返す
  index = SCRCMD_GetVMWorkValue( core, work );
  ofs = SCRCMD_GetVMWorkValue( core, work );
  w_ofs = SCRCMD_GetVMWorkValue( core, work );

  p_railWork = MMDL_GetRailWork( mmdl );

  FIELD_RAIL_WORK_SetNotMinusRailParam( p_railWork, index, ofs, w_ofs );
  FIELD_RAIL_WORK_GetLocation( p_railWork, &location );
  MMDL_SetRailLocation( mmdl, &location );

  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief ワープによる消去
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdObjWarpOut( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*     work     = wk;
  SCRIPT_WORK*     script   = SCRCMD_WORK_GetScriptWork( work );
  u16              obj_id   = SCRCMD_GetVMWorkValue( core, work ); // コマンド第一引数: OBJID
  MMDLSYS*         mmdlsys  = SCRCMD_WORK_GetMMdlSys( work );
  MMDL*            mmdl     = MMDLSYS_SearchOBJID( mmdlsys, obj_id );
  SCRIPT_FLDPARAM* fldparam = SCRIPT_GetFieldParam( script );
  FIELDMAP_WORK*   fieldmap = fldparam->fieldMap;
  FIELD_TASK_MAN*  taskMan  = FIELDMAP_GetTaskManager( fieldmap );

  VecFx32 vec;
  FIELD_TASK* moveTask;
  FIELD_TASK* rotTask; 

  GF_ASSERT( mmdl ); // 引数エラー: 指定されたOBJIDは存在しない

  PMSND_PlaySE( SEQ_SE_FLD_05 ); // ワープ音

  // タスクを生成
  VEC_Set( &vec, 0, 150<<FX32_SHIFT, 0 );
  moveTask = FIELD_TASK_TransDrawOffsetEX( fieldmap, 24, &vec, mmdl );
  rotTask  = FIELD_TASK_PlayerRotateEX( fieldmap, 24, 3, mmdl );

  // タスクを登録
  FIELD_TASK_MAN_AddTask( taskMan, rotTask, NULL );
  FIELD_TASK_MAN_AddTask( taskMan, moveTask, NULL ); 

  return VMCMD_RESULT_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * @brief 動作モデルの落下による出現
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdObjFallIn( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*     work     = wk;
  SCRIPT_WORK*     script   = SCRCMD_WORK_GetScriptWork( work );
  u16              obj_id   = SCRCMD_GetVMWorkValue( core, work ); // コマンド第一引数: OBJID
  u16              grid_x   = SCRCMD_GetVMWorkValue( core, work ); // コマンド第二引数: x 座標 ( グリッド単位 )
  u16              grid_z   = SCRCMD_GetVMWorkValue( core, work ); // コマンド第三引数: z 座標 ( グリッド単位 )
  u16              dir      = SCRCMD_GetVMWorkValue( core, work ); // コマンド第四引数: 向き
  MMDLSYS*         mmdlsys  = SCRCMD_WORK_GetMMdlSys( work );
  MMDL*            mmdl     = MMDLSYS_SearchOBJID( mmdlsys, obj_id );
  SCRIPT_FLDPARAM* fldparam = SCRIPT_GetFieldParam( script );
  FIELDMAP_WORK*   fieldmap = fldparam->fieldMap;
  FIELD_TASK_MAN*  taskMan  = FIELDMAP_GetTaskManager( fieldmap );

  // 引数エラー: 指定されたOBJIDは存在しない
  if( mmdl == NULL ) {
    GF_ASSERT(0); 
    return VMCMD_RESULT_CONTINUE;
  }

  // 動作モデルを落下地点に移動
  {
    VecFx32 pos;
    fx32 y = 0;
    s16 grid_y;

    pos.x = GRID_TO_FX32( grid_x ) + GRID_HALF_FX32; // グリッドの中心
    pos.y = 250 << FX32_SHIFT;
    pos.z = GRID_TO_FX32( grid_z ) + GRID_HALF_FX32; // グリッドの中心
    MMDL_GetMapPosHeight( mmdl, &pos, &y );
    pos.y = y;
    grid_y = SIZE_GRID_FX32( pos.y );
    MMDL_InitGridPosition( mmdl, grid_x, grid_y, grid_z, dir );
    MMDL_SetVectorPos( mmdl, &pos );
  }

  // 動作モデルの描画オフセットを初期設定 ( 画面外にいるようにする )
  {
    VecFx32 offset = { 0, 250<<FX32_SHIFT, 0 };
    MMDL_SetVectorDrawOffsetPos( mmdl, &offset );
  }

  // タスクを登録
  {
    FIELD_TASK* task;

    task = FIELD_TASK_PlayerFall( fieldmap, mmdl, 40, 250 ); 
    FIELD_TASK_MAN_AddTask( taskMan, task, NULL );
  }

  return VMCMD_RESULT_CONTINUE;
}
