//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		palace_map_gimmick.c
 *	@brief  パレスマップ　ギミック
 *	@author	tomoya takahashi
 *	@date		2010.03.20
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "fieldmap.h"
#include "palace_map_gimmick.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"
#include "p_tree.naix"
#include "palace_effect.naix"
#include "field/field_comm/intrude_field.h"
#include "field/field_comm/intrude_work.h"

#include "eventdata_local.h"
#include "field/eventdata_sxy.h"
#include "field/eventdata_system.h"

#include "arc/fieldmap/zone_id.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------


// 拡張オブジェクト・ユニット番号
#define EXPOBJ_UNIT_IDX (0)
// ギミックワークのアサインID
#define GIMMICK_WORK_ASSIGN_ID (0)

//-------------------------------------
///	ギミックワーク
//=====================================
// リソースインデックス
enum {
  MAP_RES_CUBE,           // 
  MAP_RES_CUBE_ANIME,     // 

  MAP_RES_WARP,           // 
  MAP_RES_WARP_ANIME,     // 
  
  MAP_RES_NUM
} ;
static const GFL_G3D_UTIL_RES map_res_tbl[ MAP_RES_NUM ] = 
{
  { ARCID_PALACE_EFFECT, NARC_palace_effect_block_nsbmd, GFL_G3D_UTIL_RESARC },     // 
  { ARCID_PALACE_EFFECT, NARC_palace_effect_block_nsbta, GFL_G3D_UTIL_RESARC },     // 

  { ARCID_PALACE_EFFECT, NARC_palace_effect_warp01_nsbmd, GFL_G3D_UTIL_RESARC },     // 
  { ARCID_PALACE_EFFECT, NARC_palace_effect_warp01_nsbta, GFL_G3D_UTIL_RESARC },     // 
};

// アニメーションインデックス
enum{
  MAP_ANM_CUBE_ANIME,
  MAP_ANM_CUBE_NUM,

  MAP_ANM_WARP_ANIME = 0,
  MAP_ANM_WARP_NUM,
} ;
static const GFL_G3D_UTIL_ANM map_res_cube_anm_tbl[ MAP_ANM_CUBE_NUM ] = 
{
  { MAP_RES_CUBE_ANIME, 0 },
};

static const GFL_G3D_UTIL_ANM map_res_warp_anm_tbl[ MAP_ANM_WARP_NUM ] = 
{
  { MAP_RES_WARP_ANIME, 0 },
};

// オブジェクトインデックス
enum {
  // キューブ
  MAP_OBJ_CUBE,

  // ワープ
  MAP_OBJ_WARP00,
  MAP_OBJ_WARP01,
  MAP_OBJ_WARP02,
  MAP_OBJ_WARP03,
  MAP_OBJ_WARP04,
  MAP_OBJ_WARP05,
  MAP_OBJ_WARP06,
  MAP_OBJ_WARP07,

  MAP_OBJ_NUM,

    
  // ワープオブジェ数
  MAP_OBJ_WARP_MAX = 8,
} ;
static const GFL_G3D_UTIL_OBJ map_obj_table[ MAP_OBJ_NUM ] = 
{
  // モデルリソースID, 
  // モデルデータID(リソース内部INDEX), 
  // テクスチャリソースID,
  // アニメテーブル, 
  // アニメリソース数
  
  // キューブ
  { MAP_RES_CUBE, 0, MAP_RES_CUBE, map_res_cube_anm_tbl, MAP_ANM_CUBE_NUM },
  
  // ワープ
  { MAP_RES_WARP, 0, MAP_RES_WARP, map_res_warp_anm_tbl, MAP_ANM_WARP_NUM },
  { MAP_RES_WARP, 0, MAP_RES_WARP, map_res_warp_anm_tbl, MAP_ANM_WARP_NUM },
  { MAP_RES_WARP, 0, MAP_RES_WARP, map_res_warp_anm_tbl, MAP_ANM_WARP_NUM },
  { MAP_RES_WARP, 0, MAP_RES_WARP, map_res_warp_anm_tbl, MAP_ANM_WARP_NUM },
  { MAP_RES_WARP, 0, MAP_RES_WARP, map_res_warp_anm_tbl, MAP_ANM_WARP_NUM },
  { MAP_RES_WARP, 0, MAP_RES_WARP, map_res_warp_anm_tbl, MAP_ANM_WARP_NUM },
  { MAP_RES_WARP, 0, MAP_RES_WARP, map_res_warp_anm_tbl, MAP_ANM_WARP_NUM },
  { MAP_RES_WARP, 0, MAP_RES_WARP, map_res_warp_anm_tbl, MAP_ANM_WARP_NUM },
};

static const GFL_G3D_UTIL_SETUP map_setup = { map_res_tbl, MAP_RES_NUM, map_obj_table, MAP_OBJ_NUM };





//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	ギミックワーク
//=====================================
typedef struct {
  BOOL on;

  GFL_G3D_OBJSTATUS* objstatus;

  u32 last_zoneid;
  
} PALACE_MAP_GMK_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static BOOL IsHit( PALACE_MAP_GMK_WORK* wk, const FIELD_PLAYER* player );

static void WARP_OBJ_SetUp( PALACE_MAP_GMK_WORK* wk, FIELDMAP_WORK* fieldWork );



//----------------------------------------------------------------------------
/**
 *	@brief  パレス　進入先の街でのギミックをセットアップ
 *
 *	@param	fieldWork   フィールドワーク
 */
//-----------------------------------------------------------------------------
void PALACE_MAP_GMK_Setup(FIELDMAP_WORK *fieldWork)
{
  PALACE_MAP_GMK_WORK* wk;  //
  HEAPID                heapID = FIELDMAP_GetHeapID( fieldWork );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldWork );
  int i;

  //汎用ワーク確保
  wk = GMK_TMP_WK_AllocWork
      (fieldWork, GIMMICK_WORK_ASSIGN_ID, heapID, sizeof(PALACE_MAP_GMK_WORK));
  // 拡張オブジェクトのユニットを追加
  FLD_EXP_OBJ_AddUnitByHandle( exobj_cnt, &map_setup, EXPOBJ_UNIT_IDX );

  // 全オブジェの表示をOFF
  for( i=0; i<MAP_OBJ_NUM; i++ ){
    // 表示OFF
    FLD_EXP_OBJ_SetVanish( exobj_cnt, EXPOBJ_UNIT_IDX, i, TRUE );
  }

  // キューブ
  {
    // OBJSTATUS取得
    wk->objstatus = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_IDX, MAP_OBJ_CUBE );

    // アニメOFF
    {
      EXP_OBJ_ANM_CNT_PTR anime;
      
      for( i=0; i<MAP_ANM_CUBE_NUM; i++ ){
        anime = FLD_EXP_OBJ_GetAnmCnt( exobj_cnt, 
            EXPOBJ_UNIT_IDX, MAP_OBJ_CUBE, i );

        FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, EXPOBJ_UNIT_IDX, MAP_OBJ_CUBE, i, TRUE );
        FLD_EXP_OBJ_ChgAnmStopFlg( anime, TRUE );
        FLD_EXP_OBJ_ChgAnmLoopFlg( anime, FALSE );
      }
    }
  }


  // ワープ
  WARP_OBJ_SetUp( wk, fieldWork );
  wk->last_zoneid = FIELDMAP_GetZoneID( fieldWork );
}

//----------------------------------------------------------------------------
/**
 *	@brief  パレス　進入先の街でのギミックを終了
 *
 *	@param	fieldWork   フィールドワーク
 */
//-----------------------------------------------------------------------------
void PALACE_MAP_GMK_End(FIELDMAP_WORK *fieldWork)
{
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldWork );

  //ユニット破棄
  FLD_EXP_OBJ_DelUnit( exobj_cnt, EXPOBJ_UNIT_IDX );  
  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GIMMICK_WORK_ASSIGN_ID);
}

//----------------------------------------------------------------------------
/**
 *	@brief  パレス　進入先の街でのギミック動作
 *
 *	@param	fieldWork   フィールドワーク
 */
//-----------------------------------------------------------------------------
void PALACE_MAP_GMK_Move(FIELDMAP_WORK *fieldWork)
{
  PALACE_MAP_GMK_WORK* wk;  //
  EXP_OBJ_ANM_CNT_PTR anime;
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldWork );
  FIELD_PLAYER* player = FIELDMAP_GetFieldPlayer( fieldWork );
  VecFx32 pos;
  GAMESYS_WORK* gsys = FIELDMAP_GetGameSysWork( fieldWork );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  EVENTDATA_SYSTEM* evsys = GAMEDATA_GetEventData( gdata );
  int i;

  // ワーク取得
  wk = GMK_TMP_WK_GetWork( fieldWork, GIMMICK_WORK_ASSIGN_ID );

  // ワープの再配置
  {
    u16 now_zoneid = FIELDMAP_GetZoneID( fieldWork );
    if( wk->last_zoneid != now_zoneid ){
      WARP_OBJ_SetUp( wk, fieldWork );
      wk->last_zoneid = now_zoneid;
    }
  }

  // アニメーションコントローラー取得
  anime = FLD_EXP_OBJ_GetAnmCnt( exobj_cnt, 
      EXPOBJ_UNIT_IDX, MAP_OBJ_CUBE, MAP_ANM_CUBE_ANIME );

  // アニメメイン
  FLD_EXP_OBJ_PlayAnime( exobj_cnt );
  
  // 表示キューブOFF
  if( wk->on ){

    if( FLD_EXP_OBJ_ChkAnmEnd(anime) ){
      // OFF
      FLD_EXP_OBJ_SetVanish( exobj_cnt, EXPOBJ_UNIT_IDX, MAP_OBJ_CUBE, TRUE );
      wk->on = FALSE;
      //TOMOYA_Printf( "cube off\n" );
    }
  }

  // 位置チェック
  // 自機がヒット状態の場合、
  // 目の前にダミーイベントがあったら、
  // キューブを表示する。
  if(  IsHit(wk, player) ){
    // 位置チェック
    FIELD_PLAYER_GetDirPos( player, FIELD_PLAYER_GetDir(player), &pos );
    pos.x -= MMDL_VEC_X_GRID_OFFS_FX32;
    pos.z -= MMDL_VEC_Z_GRID_OFFS_FX32;

    if( EVENTDATA_SYS_CheckPosDummyEvent( evsys, &pos ) ){
      wk->on        = TRUE;

      // 表示ON
      FLD_EXP_OBJ_SetVanish( exobj_cnt, EXPOBJ_UNIT_IDX, MAP_OBJ_CUBE, FALSE );

      for( i=0; i<MAP_ANM_CUBE_NUM; i++ ){

        anime = FLD_EXP_OBJ_GetAnmCnt( exobj_cnt, 
            EXPOBJ_UNIT_IDX, MAP_OBJ_CUBE, i );

        FLD_EXP_OBJ_SetObjAnmFrm( exobj_cnt, 
          EXPOBJ_UNIT_IDX, MAP_OBJ_CUBE, i, 0 );
        FLD_EXP_OBJ_ChgAnmStopFlg( anime, FALSE );
      }

      wk->objstatus->trans = pos;

      //TOMOYA_Printf( "cube on x[%d] y[%d] z[%d]\n", FX_Whole(pos.x), FX_Whole(pos.y), FX_Whole(pos.z) );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ゾーンチェンジ　コールバック
 *
 *	@param	fieldWork フィールドワーク
 */
//-----------------------------------------------------------------------------
void PALACE_MAP_GMK_ZoneChange(FIELDMAP_WORK *fieldWork)
{
  PALACE_MAP_GMK_WORK* wk;  //

  // ワーク取得
  wk = GMK_TMP_WK_GetWork( fieldWork, GIMMICK_WORK_ASSIGN_ID );

  // ワープ
  WARP_OBJ_SetUp( wk, fieldWork );
}




//-----------------------------------------------------------------------------
/**
 *      private関数
 */
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief  壁ヒット中のアクションコマンドかチェック
 *
 *	@param	acmd  アクションコマンド
 *
 *	@retval TRUE    ヒット中
 *	@retval FALSE   そのた動作中
 */
//-----------------------------------------------------------------------------
static BOOL IsHit( PALACE_MAP_GMK_WORK* wk, const FIELD_PLAYER* player )
{
  int i;
  u32 move_value = FIELD_PLAYER_GetMoveValue( player );
  u32 move_status = FIELD_PLAYER_GetMoveState( player );

  //TOMOYA_Printf( "move_value [%d] move_status [%d]\n", move_value, move_status );
  
  if( (move_status == PLAYER_MOVE_STATE_END) && (move_value == PLAYER_MOVE_VALUE_STOP) ){

    return TRUE;
  }
  return FALSE;
}


//----------------------------------------------------------------------------
/**
 *	@brief  ワープオブジェを設定
 *
 *	@param	wk
 *	@param	fieldWork 
 */
//-----------------------------------------------------------------------------
static void WARP_OBJ_SetUp( PALACE_MAP_GMK_WORK* wk, FIELDMAP_WORK* fieldWork )
{
  GAMESYS_WORK* gsys = FIELDMAP_GetGameSysWork( fieldWork );
  GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
  EVENTDATA_SYSTEM* evdata = GAMEDATA_GetEventData( gdata );
  int connect_num = EVENTDATA_GetConnectEventNum( evdata );
  int i;
  const CONNECT_DATA * cp_connect;
  EXIT_TYPE exit_type;
  VecFx32 pos;
  int warp_num = 0;
  GFL_G3D_OBJSTATUS* p_trans;
  HEAPID                heapID = FIELDMAP_GetHeapID( fieldWork );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldWork );

  // 全オブジェの表示をOFF
  for( i=0; i<MAP_OBJ_WARP_MAX; i++ ){
    // 表示OFF
    FLD_EXP_OBJ_SetVanish( exobj_cnt, EXPOBJ_UNIT_IDX, MAP_OBJ_WARP00+i, TRUE );
  }
  
  for( i=0; i<connect_num; i++ ){
    
    cp_connect = EVENTDATA_GetConnectByID( evdata, i );
    if( cp_connect ){

      //　進入用で入り口の場所にワープを表示
      exit_type = CONNECTDATA_GetExitType( cp_connect );
      if( exit_type == EXIT_TYPE_INTRUDE ){
        // 位置を生成
        EVENTDATA_GetConnectCenterPos( cp_connect, &pos );
        //シリンダーブリッジの場合だけ、ワープの表示位置をちょっと下にする
        if ( FIELDMAP_GetZoneID( fieldWork ) == ZONE_ID_PLH03 )
        {
          pos.y -= FX32_CONST(2.0);
        }

        GF_ASSERT( warp_num < MAP_OBJ_WARP_MAX );

        // ギミックを表示ON
        {
          // 表示ON
          FLD_EXP_OBJ_SetVanish( exobj_cnt, EXPOBJ_UNIT_IDX, MAP_OBJ_WARP00+i, FALSE );

          // OBJSTATUSに座標を設定
          p_trans = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_IDX, MAP_OBJ_WARP00+i );
          p_trans->trans = pos;

          // アニメON
          {
            EXP_OBJ_ANM_CNT_PTR anime = FLD_EXP_OBJ_GetAnmCnt( exobj_cnt, 
                EXPOBJ_UNIT_IDX, MAP_OBJ_WARP00+i, MAP_ANM_CUBE_ANIME );
            FLD_EXP_OBJ_ValidCntAnm( exobj_cnt, EXPOBJ_UNIT_IDX, MAP_OBJ_WARP00+i, MAP_ANM_WARP_ANIME, TRUE );
          }
        }

      }
    }
  }
}


