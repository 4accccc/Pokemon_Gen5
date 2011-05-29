//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		gym_poison.c
 *	@brief  毒ジム
 *	@author		Katsuhiko=Ichiraku
 *	@date		2011.04.12
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "fieldmap.h"
#include "savedata/gimmickwork.h"
#include "system/ica_anime.h"
#include "system/main.h"    //for HEAPID_FIELDMAP

// field
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE
#include "field_player_core.h"  // for FIELD_PLAYER_CORE_GetMMdl

// zone
#include "../../../resource/fldmapdata/zonetable/zone_id.h"  // for ZONE_ID_

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/fieldmap/gym_poison.naix"

// sound
#include "sound/pm_sndsys.h"

// script
#include "script_def.h"
#include "../../../resource/fldmapdata/flagwork/work_define.h"

// mine
#include "gym_poison.h"
#include "gym_poison_se_def.h"


//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
#define GYM_POISON_DEBUG_MENU
#endif //PM_DEBUG

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define GYM_POISON_UNIT_IDX (0)
#define GYM_POISON_TMP_ASSIGN_ID  (1)

#define SPEAKER1_X    (29 << FX32_SHIFT)      // スピーカー1　X座標
#define SPEAKER1_Y    (0 << FX32_SHIFT)      // スピーカー1　Y座標
#define SPEAKER1_Z    (78 << FX32_SHIFT)     // スピーカー1　Z座標
#define SPEAKER2_X    (243 << FX32_SHIFT)     // スピーカー2  X座標
#define SPEAKER2_Y    (0 << FX32_SHIFT)      // スピーカー2  Y座標
#define SPEAKER2_Z    (78 << FX32_SHIFT)      // スピーカー2  Z座標
#define SPEAKER3_X    (227 << FX32_SHIFT)     // スピーカー3  X座標
#define SPEAKER3_Y    (18 << FX32_SHIFT)      // スピーカー3  Y座標
#define SPEAKER3_Z    (25 << FX32_SHIFT)      // スピーカー3  Z座標

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	マップ位置チェック
//=====================================
typedef struct {
  FIELDMAP_WORK   *p_fieldmap;        // フィールドマップ
  ISS_SWITCH_SYS  *p_iss_switch;      // スイッチiss
  u8              volume_idx;         // 音量インデックス
} MAP_WORK;


//-------------------------------------
///	ジム内部中の一時ワーク
//=====================================
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  
  // 各ワーク
  MAP_WORK    map_wk;     // マップワーク

} GYM_POISON_TMP;

//=============================================================================
// マップ情報
//=============================================================================
// マップデータ
#include "../../../resource/fldmapdata/gimmick/gym_poison/map_array.cdat"
// map_array.xlsの数字と同じ
enum {
  VOLUME_0 = 127,   // 100%
  VOLUME_1 = 78,    // 61%
  VOLUME_2 = 38,    // 30%
  VOLUME_3 = 0,     // 0%
};

#define VOLUME_FADE_FRAME (30)      // ISSのボリュームフェードフレーム
#define VOLUME_IDX_NULL   (0xffff)


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum {
  RESID_SPEAKER_MDL = 0,      // スピーカー
  RESID_SPEAKER_ICA,          // スピーカー
  RESID_SPEAKER_ITA,          // スピーカー
};

// OBJインデックス
enum {
  OBJ_SPEAKER_1,
  OBJ_SPEAKER_2,
  OBJ_SPEAKER_3,

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_GYM_POISON, NARC_gym_poison_speaker_nsbmd, GFL_G3D_UTIL_RESARC }, // IMD
  { ARCID_GYM_POISON, NARC_gym_poison_speaker_nsbca, GFL_G3D_UTIL_RESARC }, // ICA
  { ARCID_GYM_POISON, NARC_gym_poison_speaker_nsbta, GFL_G3D_UTIL_RESARC }, // ITA
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_3d_table[] =
{
  { RESID_SPEAKER_ICA, 0 }, // nsbca
  { RESID_SPEAKER_ITA, 0 }, // nsbta
};

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] = 
{
  // スピーカー1
  {
    RESID_SPEAKER_MDL,    // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_SPEAKER_MDL,    // テクスチャリソースID
    anm_3d_table,         // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table), // アニメリソース数
  },
  // スピーカー2
  {
    RESID_SPEAKER_MDL,    // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_SPEAKER_MDL,    // テクスチャリソースID
    anm_3d_table,         // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table), // アニメリソース数
  },
  // スピーカー3
  {
    RESID_SPEAKER_MDL,    // モデルリソースID
    0,                    // モデルデータID(リソース内部INDEX)
    RESID_SPEAKER_MDL,    // テクスチャリソースID
    anm_3d_table,         // アニメテーブル(複数指定のため)
    NELEMS(anm_3d_table), // アニメリソース数
  },
};

// セットアップデータ
static const GFL_G3D_UTIL_SETUP g3Dutil_Setup = {
  res_3d_table,				//リソーステーブル
	NELEMS(res_3d_table),		//リソース数
	obj_3d_table,				//オブジェクト設定テーブル
	NELEMS(obj_3d_table),		//オブジェクト数
};

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// EXPOBJ
static void EXPOBJ_ObjSet( GYM_POISON_TMP *p_wk, const BOOL visible );
static void EXPOBJ_ObjDell( GYM_POISON_TMP *p_wk );
static void EXPOBJ_ObjMain( GYM_POISON_TMP *p_wk );

// MAP
static void MAP_Init( MAP_WORK *p_wk, FIELDMAP_WORK *p_fieldmap );
static BOOL MAP_CheckPos( MAP_WORK *p_wk );
static const int MAP_GetVolumeIdx( MAP_WORK *p_wk );

#ifdef GYM_POISON_DEBUG_MENU
//-------------------------------------
///	デバッグメニュー
//=====================================
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fieldmap, HEAPID heapID );
static void DEBUGMENU_Exit( void );

static int  debug_volume_1 = VOLUME_1;
static int  debug_volume_2 = VOLUME_2;
static int  debug_volume_fade = VOLUME_FADE_FRAME;

#endif // GYM_POISON_DEBUG_MENU

//=============================================================================
/**
 *          外部公開関数
 */
//=============================================================================
//--------------------------------------------------------------
/**
 * @brief     セットアップ関数
 * @param	    p_fieldmap     フィールドワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
void GYM_POISON_Setup(FIELDMAP_WORK *p_fieldmap)
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  //汎用ワーク確保
  GYM_POISON_TMP *p_wk = GMK_TMP_WK_AllocWork(p_fieldmap, GYM_POISON_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(GYM_POISON_TMP));

  p_wk->exp_obj = FIELDMAP_GetExpObjCntPtr( p_fieldmap );

  

  // EXPOBJセット
  {
    BOOL  visible;
    // ZONE_ID_C13GYM0102の時だけモデル表示させる
    visible = (FIELDMAP_GetZoneID( p_fieldmap ) == ZONE_ID_C13GYM0102) ? FALSE : TRUE;
    EXPOBJ_ObjSet( p_wk, visible );
  }

  // マップ初期化
  MAP_Init( &p_wk->map_wk, p_fieldmap );

#ifdef GYM_POISON_DEBUG_MENU
  DEBUGMENU_Init( p_wk, p_fieldmap, HEAPID_FIELDMAP );
#endif // GYM_POISON_DEBUG_MENU
}

//--------------------------------------------------------------
/**
 * @brief   解放関数
 * @param   p_fieldmap     フィールドワークポインタ
 * @return  none
 */
//--------------------------------------------------------------
void GYM_POISON_End(FIELDMAP_WORK *p_fieldmap)
{
  GYM_POISON_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_POISON_TMP_ASSIGN_ID );

#ifdef GYM_POISON_DEBUG_MENU
  DEBUGMENU_Exit();
#endif // GYM_POISON_DEBUG_MENU

  //反映されず残っている予約設定を全削除
  ISS_SWITCH_SYS_ClearReserve( p_wk->map_wk.p_iss_switch );
  ISS_SWITCH_SYS_ClearReserveVolume( p_wk->map_wk.p_iss_switch );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(p_fieldmap, GYM_POISON_TMP_ASSIGN_ID);
}

//--------------------------------------------------------------
/**
 * @brief       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void GYM_POISON_Move(FIELDMAP_WORK *p_fieldmap)
{
  GYM_POISON_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GYM_POISON_TMP_ASSIGN_ID );


#ifdef PM_DEBUG
  // デバッグ
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
      ICHI_Printf("スイッチ1 ON\n");
      ISS_SWITCH_SYS_SwitchOn( p_wk->map_wk.p_iss_switch, 1 );
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
      ICHI_Printf("スイッチ2 ON\n");
      ISS_SWITCH_SYS_SwitchOn( p_wk->map_wk.p_iss_switch, 2 );
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
      ICHI_Printf("スイッチ3 ON\n");
      ISS_SWITCH_SYS_SwitchOn( p_wk->map_wk.p_iss_switch, 3 );
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT ){
      ICHI_Printf("スイッチ1 OFF\n");
      ISS_SWITCH_SYS_SwitchOff( p_wk->map_wk.p_iss_switch, 1 );
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
      ICHI_Printf("スイッチ2 OFF\n");
      ISS_SWITCH_SYS_SwitchOff( p_wk->map_wk.p_iss_switch, 2 );
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ){
      ICHI_Printf("スイッチ3 OFF\n");
      ISS_SWITCH_SYS_SwitchOff( p_wk->map_wk.p_iss_switch, 3 );
    }
  }
#endif

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );

  // マップ更新
  MAP_CheckPos( &p_wk->map_wk );
}


//----------------------------------------------------------------------------
/**
 *	@brief  スピーカーのアニメストップ
 *
 *	@param	*gsys
 *	@param	int idx   0-2
 */
//-----------------------------------------------------------------------------
void GYM_POISON_AnmStop(GAMESYS_WORK *gsys, const int idx)
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  GYM_POISON_TMP *p_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_POISON_TMP_ASSIGN_ID);

  {
    u16 obj_idx = idx;    // OBJインデックス

    // アニメ再生OFF
    EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_POISON_UNIT_IDX, obj_idx, 0 );

    // ICA
    FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_POISON_UNIT_IDX, obj_idx, 0, FALSE );
    // ITA
    FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_POISON_UNIT_IDX, obj_idx, 1, FALSE );

    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメSTOP
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  ISSOBJのアニメストップ
 *
 *	@param	*gsys
 *	@param	int obj_id
 */
//-----------------------------------------------------------------------------
void GYM_POISON_IssObjStop(GAMESYS_WORK *gsys, const int obj_id)
{
  FIELDMAP_WORK     *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  const MMDLSYS     *cp_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );

  MMDL *mmdl = MMDLSYS_SearchOBJID( cp_mmdlsys, obj_id );  // 
  MMDL_OnStatusBit( mmdl, MMDL_STABIT_PAUSE_ANM );
  ICHI_Printf("ISS OBJ %d ANM STOP \n", obj_id);
}

//=============================================================================
/**
 *      static関数
 */
//=============================================================================
//=============================================================================
/**
 *      EXPOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	GYM_POISON_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( GYM_POISON_TMP *p_wk, const BOOL visible )
{
  u8 i;

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GYM_POISON_UNIT_IDX);

  //=====================================
  // スピーカーの初期設定
  for( i=OBJ_SPEAKER_1; i<OBJ_NUM; i++ )
  {
    // 座標セット
    VecFx32 pos[ OBJ_NUM ] = {
      { SPEAKER1_X,  SPEAKER1_Y,  SPEAKER1_Z },
      { SPEAKER2_X,  SPEAKER2_Y,  SPEAKER2_Z },
      { SPEAKER3_X,  SPEAKER3_Y,  SPEAKER3_Z },
    };

    u16 obj_idx = i;    // OBJインデックス

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GYM_POISON_UNIT_IDX, obj_idx );

    // 座標
    status->trans.x = pos[i].x;
    status->trans.y = pos[i].y;
    status->trans.z = pos[i].z;

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GYM_POISON_UNIT_IDX, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GYM_POISON_UNIT_IDX, obj_idx, visible );
    // アニメ再生
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, GYM_POISON_UNIT_IDX, obj_idx, 0 );
      // ICA
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_POISON_UNIT_IDX, obj_idx, 0, TRUE );
      // ITA
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, GYM_POISON_UNIT_IDX, obj_idx, 1, TRUE );
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, TRUE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );  // アニメ再生設定
    }
  }

}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	GYM_POISON_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( GYM_POISON_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GYM_POISON_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	GYM_POISON_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( GYM_POISON_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}

//=============================================================================
/**
 *      MAP
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  マップワーク初期化
 *
 *	@param	MAP_WORK *p_wk  ワーク
 *	@param	*p_fieldmap     
 */
//-----------------------------------------------------------------------------
static void MAP_Init( MAP_WORK *p_wk, FIELDMAP_WORK *p_fieldmap )
{
  GYM_POISON_TMP *p_gym_wk = GMK_TMP_WK_GetWork(p_fieldmap, GYM_POISON_TMP_ASSIGN_ID);
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA   *gdata = GAMESYSTEM_GetGameData( gsys );
  EVENTWORK* p_evwk = GAMEDATA_GetEventWork( gdata );

  u16 *local_wk0 = EVENTWORK_GetEventWorkAdrs( p_evwk, LOCALWORK0 );
  u16 *local_wk1 = EVENTWORK_GetEventWorkAdrs( p_evwk, LOCALWORK1 );
  u16 *local_wk2 = EVENTWORK_GetEventWorkAdrs( p_evwk, LOCALWORK2 );

  p_wk->p_fieldmap = p_fieldmap;

  //ISSを保存
  {
    ISS_SYS *p_iss  = GAMESYSTEM_GetIssSystem( FIELDMAP_GetGameSysWork(p_fieldmap) );
    p_wk->p_iss_switch  = ISS_SYS_GetIssSwitchSystem( p_iss );
    p_wk->volume_idx = VOLUME_IDX_NULL;
  }

  // ISS設定
#if 1
  {
    // トレーナー戦闘フラグがONの時はISSスイッチをOFFにする
    BOOL  iss_flg1 = (*local_wk0 == 0 ) ? TRUE : FALSE;
    BOOL  iss_flg2 = (*local_wk1 == 0 ) ? TRUE : FALSE;
    BOOL  iss_flg3 = (*local_wk2 == 0 ) ? TRUE : FALSE;

    ICHI_Printf("ISS スイッチ1 %d \n", iss_flg1);
    ICHI_Printf("ISS スイッチ2 %d \n", iss_flg2);
    ICHI_Printf("ISS スイッチ3 %d \n", iss_flg3);

    // ISS予約
    // スイッチ3 → ホミカ
    // スイッチ2 → ギター
    // スイッチ1 → ドラム
    //
    ISS_SWITCH_SYS_ReserveSwitch( p_wk->p_iss_switch, SWITCH_03, iss_flg3, GYM_POISON_BGM );
    ISS_SWITCH_SYS_ReserveSwitch( p_wk->p_iss_switch, SWITCH_02, iss_flg2, GYM_POISON_BGM );
    ISS_SWITCH_SYS_ReserveSwitch( p_wk->p_iss_switch, SWITCH_01, iss_flg1, GYM_POISON_BGM );

  }
#endif

  // スピーカー設定
  // ジムリーダー話したフラグがたっているとアニメOFFにする
  if( *local_wk2 == 1 )
  {
    u8 i;

    for( i=OBJ_SPEAKER_1; i<OBJ_NUM; i++ )
    {
      u16 obj_idx = i;    // OBJインデックス

      // アニメ再生
      {
        // アニメ再生OFF
        EXP_OBJ_ANM_CNT_PTR exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_gym_wk->exp_obj, GYM_POISON_UNIT_IDX, obj_idx, 0 );

        // ICA
        FLD_EXP_OBJ_ValidCntAnm( p_gym_wk->exp_obj, GYM_POISON_UNIT_IDX, obj_idx, 0, FALSE );
        // ITA
        FLD_EXP_OBJ_ValidCntAnm( p_gym_wk->exp_obj, GYM_POISON_UNIT_IDX, obj_idx, 1, FALSE );

        FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );  // アニメSTOP
        FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      }
    }
  }


  // 初期音量を設定
  {
    // インデックス別音量テーブル
    int volume_tbl[] = {
      VOLUME_0,
      VOLUME_1,
      VOLUME_2,
      VOLUME_3,
    };

    int volume_idx = 0;

    // 現在位置のマップ音量データ取得
    volume_idx = MAP_GetVolumeIdx( p_wk );

    ISS_SWITCH_SYS_ReserveVolume( p_wk->p_iss_switch, volume_tbl[ volume_idx ], VOLUME_FADE_FRAME, GYM_POISON_BGM );
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  マップデータチェック
 *
 *	@param	MAP_WORK *p_wk 
 *
 *	@retval TRUE:マップデータ変化有り
 *	@retval FALSE:マップデータ変化なし
 */
//-----------------------------------------------------------------------------
static BOOL MAP_CheckPos( MAP_WORK *p_wk )
{
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
  s16 gx, gy, gz;
  int tmp_map_data = 0;

  // 現在位置のマップ音量データ取得
  tmp_map_data = MAP_GetVolumeIdx( p_wk );

  // 現在位置のデータと比較
  if( tmp_map_data != p_wk->volume_idx )
  {
    // インデックス別音量テーブル
    int volume_tbl[] = {
      VOLUME_0,
      VOLUME_1,
      VOLUME_2,
      VOLUME_3,
    };

    // 調整用デバッグ
#ifdef GYM_POISON_DEBUG_MENU
      volume_tbl[1] = debug_volume_1,
      volume_tbl[2] = debug_volume_2,
#endif

    p_wk->volume_idx = tmp_map_data;

    ISS_SWITCH_SYS_SwitchSetVolume( p_wk->p_iss_switch, 
        volume_tbl[ p_wk->volume_idx ],
#ifdef GYM_POISON_DEBUG_MENU
        debug_volume_fade );
#else
        VOLUME_FADE_FRAME );
#endif

    ICHI_Printf("音量変更 %d\n", volume_tbl[ p_wk->volume_idx ]);
    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  現在位置の音量インデックス取得
 *
 *	@param	MAP_WORK *p_wk 
 *
 *	@return どの音量なのかのIDX
 */
//-----------------------------------------------------------------------------
static const int MAP_GetVolumeIdx( MAP_WORK *p_wk )
{
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
  s16 gx, gy, gz;
  int map_volume_idx = 0;

  // 自機グリッド位置
  FIELD_PLAYER_GetGridPos( cp_player, &gx, &gy, &gz );

  // ゾーンIDでマップ配列変える
  switch( FIELDMAP_GetZoneID( p_wk->p_fieldmap ) ){
  case ZONE_ID_C13GYM0101:    // ロビー
    if( map_component_max[0][0] <= gz ||
        map_component_max[0][1] <= gx )
    {
      return VOLUME_IDX_NULL;
    }
    map_volume_idx = map_array_1[gz][gx];
    break;

  case ZONE_ID_C13GYM0102:    // ライブ会場
    if( map_component_max[1][0] <= gz ||
        map_component_max[1][1] <= gx )
    {
      return VOLUME_IDX_NULL;
    }
    map_volume_idx = map_array_2[gz][gx];
    break;

  default:
    GF_ASSERT(0);
  }

  return map_volume_idx;
}


#ifdef GYM_POISON_DEBUG_MENU
//=============================================================================
/**
 *    デバッグメニュー
 */
//=============================================================================
//-------------------------------------
///	マクロ
//=====================================
#define DEBUGMENU_GYM_POISON (2)

//-------------------------------------
///	static
//=====================================
// キー入力でデータ変更
static BOOL DebugWin_ChangeData( int *p_param, int min, int max );
// 音量の変更
static void DebugWin_Update_ChangeVolume1( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeVolume1( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeVolume2( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeVolume2( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Update_ChangeVolumeFade( void *userWork, DEBUGWIN_ITEM *item );
static void DebugWin_Draw_ChangeVolumeFade( void *userWork, DEBUGWIN_ITEM *item );

//----------------------------------------------------------------------------
/**
 *	@brief  R+Yにデバッグメニュー追加
 *
 *	@param	*p_wk       ギミックワーク
 *	@param	*p_fieldmap フィールドマップワーク
 *	@param	heapID      ヒープID
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Init( void *p_wk, FIELDMAP_WORK *p_fieldmap, HEAPID heapID )
{

  // デバッグメニューグループ追加
  DEBUGWIN_AddGroupToTop( DEBUGMENU_GYM_POISON, "どくジム", heapID );

  // 風力変更の時間
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeVolume1,
                              DebugWin_Draw_ChangeVolume1,
                              p_fieldmap,
                              DEBUGMENU_GYM_POISON,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeVolume2,
                              DebugWin_Draw_ChangeVolume2,
                              p_fieldmap,
                              DEBUGMENU_GYM_POISON,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_Update_ChangeVolumeFade,
                              DebugWin_Draw_ChangeVolumeFade,
                              p_fieldmap,
                              DEBUGMENU_GYM_POISON,
                              heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief  R+Yからデバッグメニュー破棄
 */
//-----------------------------------------------------------------------------
static void DEBUGMENU_Exit( void )
{
  DEBUGWIN_RemoveGroup( DEBUGMENU_GYM_POISON );
}

//----------------------------------------------------------------------------
/**
 *	@brief  デバッグ数値入力
 *
 *	@param	*p_param  変更する値のポインタ
 *	@param	min       入力する時の最小値
 *	@param	max       入力する時の最大値
 *
 *	@retval TRUE 入力あり
 *	@retval FALSE 入力なし
 */
//-----------------------------------------------------------------------------
static BOOL DebugWin_ChangeData( int *p_param, int min, int max )
{
  int value = 0;

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  { 
    value = -1;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  { 
    value = +1;
  }

  (*p_param) += value;
  (*p_param)  = MATH_CLAMP( (*p_param), min, max );

  if( value == 0 )
  {
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  音量1変更 更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeVolume1( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  int value = debug_volume_1;

  if( DebugWin_ChangeData( &value, 0, 127 ) )
  {
    // 更新あり
    debug_volume_1 = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  音量1変更 描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeVolume1( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "おんりょう1[%d]", debug_volume_1 );
}
//----------------------------------------------------------------------------
/**
 *	@brief  音量2変更 更新
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeVolume2( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  int value = debug_volume_2;

  if( DebugWin_ChangeData( &value, 0, 127 ) )
  {
    // 更新あり
    debug_volume_2 = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  音量2変更 描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeVolume2( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "おんりょう2[%d]", debug_volume_2 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  音量フェードフレーム変更 描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Update_ChangeVolumeFade( void *userWork, DEBUGWIN_ITEM *item )
{
  FIELDMAP_WORK *p_fieldmap = userWork;
  int value = debug_volume_fade;

  if( DebugWin_ChangeData( &value, 0, 100 ) )
  {
    // 更新あり
    debug_volume_fade = value;
    DEBUGWIN_RefreshScreen();
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  音量フェードフレーム変更 描画
 *
 *	@param	*userWork ワーク
 *	@param	*item     アイテム
 */
//-----------------------------------------------------------------------------
static void DebugWin_Draw_ChangeVolumeFade( void *userWork, DEBUGWIN_ITEM *item )
{
  DEBUGWIN_ITEM_SetNameV( item, "フェードフレーム[%d]", debug_volume_fade );
}

#endif // GYM_POISON_DEBUG_MENU

