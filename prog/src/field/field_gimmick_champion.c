//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_champion.c
 *	@brief  チャンピオン部屋
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.07.22
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "fieldmap.h"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "savedata/gimmickwork.h"

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/fieldmap/champion.naix"
#include "../../../resource/fldmapdata/eventdata/tmp/c09r0501.h"

// sound
#include "sound/pm_sndsys.h"

// mine
#include "field_gimmick_champion.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define CHAMPION_UNIT_IDX        (0)
#define CHAMPION_TMP_ASSIGN_ID   (1)

// 座標関係
#define MAP_SIZE          (512 << FX32_SHIFT )
#define MAP_HALF_SIZE     (256 << FX32_SHIFT )

// 光アニメの最終フレームと仮定するフレーム
#define LIGHT_LAST_FRAME  ( 8 * FX32_ONE )

#define TOP_CAMERA_ID     ( 43 )      // 頂上のカメラID camera_dataのエクセルのカメラ指定値

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
// セーブワーク
typedef struct {
  BOOL      exchange_flag;     // 点灯後に切り替えているか
} CHAMPION_SV_WORK;

//-------------------------------------
///	ギミック内部中の一時ワーク
//=====================================
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ
  CHAMPION_SV_WORK    *p_sv_wk;       // セーブワーク

} CHAMPION_TMP;

//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum{
  RESID_ROOM_00_MDL,        // 照明点灯前マップモデル
  RESID_ROOM_01_MDL,        // 照明点灯後マップモデル
  RESID_ROOM_01_ANM_ICA,    // 照明点灯後マップアニメ
  RESID_ROOM_01_ANM_IMA,    // 照明点灯後マップアニメ
  RESID_RELIEF_00_MDL,      // 照明点灯前レリーフモデル
  RESID_RELIEF_01_MDL,      // 照明点灯後レリーフ
  RESID_RELIEF_01_ANM_ICA,  // 通常時レリーフ回転アニメ
  RESID_RELIEF_02_ANM_ICA,  // 戦闘後レリーフ回転アニメ
  RESID_RELIEF_02_ANM_IMA,  // 戦闘後レリーフ回転アニメ
  RESID_LIGHT_MDL,          // 差し込む光モデル
  RESID_LIGHT_ANM_ICA,      // 差し込む光アニメ
  RESID_LIGHT_ANM_IMA,      // 差し込む光アニメ
};

// OBJインデックス
enum{
  OBJ_ROOM_1,               // マップ(点灯前)
  OBJ_ROOM_2,               // マップ(点灯後)
  OBJ_RELIEF_1,             // レリーフ(点灯前)
  OBJ_RELIEF_2,             // レリーフ(点灯後)
  OBJ_LIGHT,                // 光
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_CHAMPION, NARC_champion_m_champ00_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_m_champ01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_m_champ01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_m_champ01_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_c_relief00_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_c_relief01_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_c_relief_01_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_c_relief_02_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_c_relief_02_nsbma, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_c_light_nsbmd, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_c_light_nsbca, GFL_G3D_UTIL_RESARC },
  { ARCID_CHAMPION, NARC_champion_c_light_nsbma, GFL_G3D_UTIL_RESARC },
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_room[] =
{
  { RESID_ROOM_01_ANM_ICA, 0 },
  { RESID_ROOM_01_ANM_IMA, 0 },
};
#define ANM_ROOM_NUM  ( NELEMS(anm_room) )

static const GFL_G3D_UTIL_ANM anm_relief[] =
{
  { RESID_RELIEF_01_ANM_ICA, 0 },
  { RESID_RELIEF_02_ANM_ICA, 0 },
  { RESID_RELIEF_02_ANM_IMA, 0 },
};
#define ANM_RELIEF_NUM  ( NELEMS(anm_relief) )

static const GFL_G3D_UTIL_ANM anm_light[] =
{
  { RESID_LIGHT_ANM_ICA, 0 },
  { RESID_LIGHT_ANM_IMA, 0 },
};
#define ANM_LIGHT_NUM  ( NELEMS(anm_light) )

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_ROOM_1
  {
    RESID_ROOM_00_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_ROOM_00_MDL,      // テクスチャリソースID
    NULL,                   // アニメテーブル(複数指定のため)
    0,                      // アニメリソース数
  },
  // OBJ_ROOM_2
  {
    RESID_ROOM_01_MDL,      // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_ROOM_01_MDL,      // テクスチャリソースID
    anm_room,               // アニメテーブル(複数指定のため)
    ANM_ROOM_NUM,           // アニメリソース数
  },
  // OBJ_RELIEF_1
  {
    RESID_RELIEF_00_MDL,    // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_RELIEF_00_MDL,    // テクスチャリソースID
    anm_relief,             // アニメテーブル(複数指定のため)
    ANM_RELIEF_NUM,         // アニメリソース数
  },
  // OBJ_RELIEF_2
  {
    RESID_RELIEF_01_MDL,    // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_RELIEF_01_MDL,    // テクスチャリソースID
    anm_relief,             // アニメテーブル(複数指定のため)
    ANM_RELIEF_NUM,         // アニメリソース数
  },
  // OBJ_LIGHT
  {
    RESID_LIGHT_MDL,        // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_LIGHT_MDL,        // テクスチャリソースID
    anm_light,              // アニメテーブル(複数指定のため)
    ANM_LIGHT_NUM,          // アニメリソース数
  },
};

// セットアップデータ
static const GFL_G3D_UTIL_SETUP g3Dutil_Setup = {
  res_3d_table,				    //リソーステーブル
	NELEMS(res_3d_table),		//リソース数
	obj_3d_table,				    //オブジェクト設定テーブル
	NELEMS(obj_3d_table),		//オブジェクト数
};

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// INIT
static void INIT_FieldRecover( CHAMPION_TMP *p_wk );

// EXPOBJ
static void EXPOBJ_ObjSet( CHAMPION_TMP *p_wk );
static void EXPOBJ_ObjDell( CHAMPION_TMP *p_wk );
static void EXPOBJ_ObjMain( CHAMPION_TMP *p_wk );
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num, BOOL vanish );

// アニメ管理
// 共通
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx, fx32 frame, BOOL loop );
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx );

// LIGHT
static void ANM_LIGHT_Play( CHAMPION_TMP *p_wk );
static BOOL ANM_LIGHT_CheckEnd( CHAMPION_TMP *p_wk );
static void ANM_LIGHT_SetLastFrame( CHAMPION_TMP *p_wk );

// 照明が点灯するイベント
static GMEVENT * LIGHTING_CreateEvent( GAMESYS_WORK* gsys );
static GMEVENT_RESULT LightingEvent( GMEVENT* event, int* seq, void* wk );

// 殿堂入り部屋出現イベント
static GMEVENT * HALL_ROOM_CreateEvent( GAMESYS_WORK* gsys );
static GMEVENT_RESULT HallRoomEvent( GMEVENT* event, int* seq, void* wk );

//-------------------------------------
///	セーブ情報
//=====================================
static void SAVE_SetExchangeFlag( CHAMPION_SV_WORK *p_wk, BOOL flag );
static BOOL SAVE_GetExchangeFlag( const CHAMPION_SV_WORK *p_wk );


//=============================================================================
/**
 *          カメラゆれ
 */
//=============================================================================
//-------------------------------------
///	カメラ振動パラメータ
//=====================================
typedef struct {

  s16 Height;             // ゆれ高さ
  u8  Time;               // ゆれ回数

  u32 Sync;               // 1周期するのにかかるシンク数
  u32 NowSync;            // 現在のシンク
  
  s16 WaitStartFrame;     // 開始まで何フレーム待つか

} CAMERA_SHAKE_PARAM;

// カメラ揺れるタスク作成
static FIELD_TASK* FIELD_TASK_CameraShake( FIELDMAP_WORK* p_fieldmap, const CAMERA_SHAKE_PARAM* shakeParam );

//-------------------------------------
///	カメラ振動ワーク
//=====================================
typedef struct {
  FIELDMAP_WORK*  p_fieldmap;
  FIELD_CAMERA*   camera;

  u32             seq;
  CAMERA_SHAKE_PARAM shakeParam;
  const VecFx32   *WatchTarget;
  const VecFx32   *DefaultWatchTarget;

} CAMERA_SHAKE_WORK;
// カメラ揺れるタスク
static FIELD_TASK_RETVAL CameraShake( void* wk );

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//--------------------------------------------------------------
/**
 * @bried       セットアップ関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void CHAMPION_Setup( FIELDMAP_WORK *p_fieldmap )
{
  GAMESYS_WORK* p_gsys = FIELDMAP_GetGameSysWork( p_fieldmap );
  GAMEDATA* p_gdata = GAMESYSTEM_GetGameData( p_gsys );

  // 汎用ワーク確保
  CHAMPION_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, CHAMPION_TMP_ASSIGN_ID, FIELDMAP_GetHeapID(p_fieldmap), sizeof(CHAMPION_TMP) );

  p_wk->exp_obj     = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_fieldmap  = p_fieldmap;
  p_wk->p_sv_wk     = GIMMICKWORK_Get( GAMEDATA_GetGimmickWork( p_gdata ), FLD_GIMMICK_C09R0501 );

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  // 初期化
  INIT_FieldRecover( p_wk );
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void CHAMPION_End( FIELDMAP_WORK *p_fieldmap )
{
  CHAMPION_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, CHAMPION_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, CHAMPION_TMP_ASSIGN_ID );
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void CHAMPION_Move( FIELDMAP_WORK *p_fieldmap )
{
  CHAMPION_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, CHAMPION_TMP_ASSIGN_ID );


  

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//=============================================================================
/**
 *        スクリプトコマンド
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  照明点灯イベント
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ 
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT* CHAMPION_Lighting( GAMESYS_WORK *gsys )
{
  GMEVENT *event;

  event = LIGHTING_CreateEvent( gsys );

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  殿堂入り部屋出現イベント
 *
 *	@param	GAMESYS_WORK *gsys  ゲームシステムポインタ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
GMEVENT* CHAMPION_HallRoom( GAMESYS_WORK *gsys )
{
  GMEVENT *event;

  event = HALL_ROOM_CreateEvent( gsys );

  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビル影OFF
 *
 *	@param	GAMESYS_WORK *gsys 
 *	@param  obj_id 対象のOBJ_ID
 */
//-----------------------------------------------------------------------------
void CHAMPION_LightAttrOFF( GAMESYS_WORK *gsys, const int obj_id )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

  {
    const MMDLSYS *cp_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
    MMDL* mmdl = MMDLSYS_SearchOBJID( cp_mmdlsys, obj_id );
    GF_ASSERT( mmdl );
    // アトリビュートライト無効
    MMDL_SetStatusLightAttrOFF( mmdl, TRUE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビル影ON
 *
 *	@param	GAMESYS_WORK *gsys 
 *	@param  obj_id 対象のOBJ_ID
 */
//-----------------------------------------------------------------------------
void CHAMPION_LightAttrON( GAMESYS_WORK *gsys, const int obj_id )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

  {
    const MMDLSYS *cp_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
    MMDL* mmdl = MMDLSYS_SearchOBJID( cp_mmdlsys, obj_id );
    GF_ASSERT( mmdl );
    // アトリビュートライト有効
    MMDL_SetStatusLightAttrOFF( mmdl, FALSE );
  }
}

//=============================================================================
/**
 *					static関数
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  セーブ状態による初期化
 *
 *	@param	CHAMPION_TMP *p_wk ギミックワーク
 */
//-----------------------------------------------------------------------------
static void INIT_FieldRecover( CHAMPION_TMP *p_wk )
{
  const BOOL exchange_flag = SAVE_GetExchangeFlag( p_wk->p_sv_wk );

  // 切り替え後ならモデルを点灯後にする
  // アニメは初期化
  if( exchange_flag == TRUE )
  {
    // ルーム表示切り替え
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_ROOM_1, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_ROOM_2, FALSE );

    // レリーフ表示切り替え
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_RELIEF_1, TRUE );
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_RELIEF_2, FALSE );
    {
      // 無効化解除
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_RELIEF_2, 1, TRUE );
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_RELIEF_2, 2, TRUE );
      // フレーム設定(0進めた状態にする)
      FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_RELIEF_2, 1, 0 );
      FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_RELIEF_2, 2, 0 );
    }

    // 光差し込み状態に
    ANM_LIGHT_SetLastFrame( p_wk );

    // デフォルトカメラも頂上用に変更
    {
      FIELD_CAMERA *p_camera = FIELDMAP_GetFieldCamera( p_wk->p_fieldmap );
      FIELD_CAMERA_SetCameraType( p_camera, TOP_CAMERA_ID );
    }
  }
}

//=============================================================================
/**
 *          EXPOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	CHAMPION_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( CHAMPION_TMP *p_wk )
{
  int i;

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, CHAMPION_UNIT_IDX);

  //=====================================
  // マップ
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_ROOM_1, 0, FALSE );
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_ROOM_2, ANM_ROOM_NUM, TRUE );

  //=====================================
  // レリーフ
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_RELIEF_1, ANM_RELIEF_NUM, FALSE );
  // ループアニメは再生しておく
  ANM_Play( p_wk->exp_obj, OBJ_RELIEF_1, 0, 0, TRUE );

  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_RELIEF_2, ANM_RELIEF_NUM, TRUE );

  //=====================================
  // 差し込む光
  EXPOBJ_ObjSetOne( p_wk->exp_obj, OBJ_LIGHT, ANM_LIGHT_NUM, TRUE );
  // カリングしない
  FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_LIGHT, FALSE );

}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	CHAMPION_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( CHAMPION_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, CHAMPION_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	CHAMPION_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( CHAMPION_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ初期設定
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj 3D拡張OBJポインタ
 *	@param  u16 obj_idx OBJインデックス
 *	@param	int anm_num アニメ数
 *	@param  BOOL vanish 表示設定
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSetOne( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, int anm_num, BOOL vanish )
{
  int i;
  GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( exp_obj, CHAMPION_UNIT_IDX, obj_idx );

  // 座標
  VEC_Set( &status->trans, (MAP_HALF_SIZE), 0, (MAP_SIZE+MAP_HALF_SIZE) );

  // カリングする
  FLD_EXP_OBJ_SetCulling( exp_obj, CHAMPION_UNIT_IDX, obj_idx, TRUE );
  // 表示設定
  FLD_EXP_OBJ_SetVanish( exp_obj, CHAMPION_UNIT_IDX, obj_idx, vanish );

  // 
  if( anm_num == 0 )
  {
    // アニメがないなら設定しない
    return;
  }

  // アニメ再生
  //  1回再生 無効化
  for( i=0; i<anm_num; i++ )
  {
    EXP_OBJ_ANM_CNT_PTR exp_anm;
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, CHAMPION_UNIT_IDX, obj_idx, i );

    // 1回再生設定
    FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
    FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

    // 無効化
    FLD_EXP_OBJ_ValidCntAnm( exp_obj, CHAMPION_UNIT_IDX, obj_idx, i, FALSE );
  }
}

//=============================================================================
/**
 *          アニメ管理
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  アニメ再生
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx   OBJインデックス
 *	@param	anm_idx   アニメインデックス
 *	@param  frame     現在のフレーム
 *	@param	loop      ループ再生フラグ(TRUEでループ
 */
//-----------------------------------------------------------------------------
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx, fx32 frame, BOOL loop )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, CHAMPION_UNIT_IDX, obj_idx, anm_idx );

  // 再生
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );
  FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, loop );

  // 無効化解除
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, CHAMPION_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // フレーム設定
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, CHAMPION_UNIT_IDX, obj_idx, anm_idx, frame );
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメ停止
 *
 *	@param	FLD_EXP_OBJ_CNT_PTR exp_obj
 *	@param	obj_idx   OBJインデックス
 *	@param	anm_idx   アニメインデックス
 */
//-----------------------------------------------------------------------------
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, CHAMPION_UNIT_IDX, obj_idx, anm_idx );
  // 停止
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );
  // 無効化
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, CHAMPION_UNIT_IDX, obj_idx, anm_idx, FALSE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, CHAMPION_UNIT_IDX, obj_idx, anm_idx, 0 );
}

//=============================================================================
/**
 *          LIGHT
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  差し込む光アニメ再生
 *
 *	@param	CHAMPION_TMP *p_wk  ギミックワーク
 */
//-----------------------------------------------------------------------------
static void ANM_LIGHT_Play( CHAMPION_TMP *p_wk )
{
  int i;
  u16 obj_idx = OBJ_LIGHT;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<ANM_LIGHT_NUM; i++ )
  {
    ANM_Play( p_wk->exp_obj, obj_idx, i, 0, FALSE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  差し込む光アニメ終了チェック
 *
 *	@param	CHAMPION_TMP *p_wk  ギミックワーク
 *
 *	@return TRUEで終了
 */
//-----------------------------------------------------------------------------
static BOOL ANM_LIGHT_CheckEnd( CHAMPION_TMP *p_wk )
{
  u16 obj_idx = OBJ_LIGHT;
  u16 anm_idx = 0;
  fx32  now_frame;

  now_frame = FLD_EXP_OBJ_GetObjAnmFrm( p_wk->exp_obj, CHAMPION_UNIT_IDX, obj_idx, anm_idx );

  if( now_frame >= LIGHT_LAST_FRAME )
  {
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  差し込む光アニメラストフレーム状態に設定
 *
 *	@param	CHAMPION_TMP *p_wk  ギミックワーク
 */
//----------------------------------------------------------------------------
static void ANM_LIGHT_SetLastFrame( CHAMPION_TMP *p_wk )
{
  int i;
  fx32 last_frame;
  u16 obj_idx = OBJ_LIGHT;
  u16 anm_idx = 0;
  EXP_OBJ_ANM_CNT_PTR exp_anm;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<ANM_LIGHT_NUM; i++ )
  {
    exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, CHAMPION_UNIT_IDX, obj_idx, i );
    FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, CHAMPION_UNIT_IDX, obj_idx, i, TRUE );

    // 最終フレーム取得
    last_frame = FLD_EXP_OBJ_GetAnimeLastFrame( exp_anm );

    // ラストフレームにする
    FLD_EXP_OBJ_SetObjAnmFrm( p_wk->exp_obj, CHAMPION_UNIT_IDX, obj_idx, i, last_frame );
  }
}

//=============================================================================
/**
 *          イベント
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  照明が点灯するイベント作成
 *
 *	@param	GAMESYS_WORK* gsys ゲームシステムポインタ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * LIGHTING_CreateEvent( GAMESYS_WORK* gsys )
{
  GMEVENT*  event;
  // イベント生成
  event = GMEVENT_Create( gsys, NULL, LightingEvent, 0 );

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  照明が点灯するイベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//----------------------------------------------------------------------------
static GMEVENT_RESULT LightingEvent( GMEVENT* event, int* seq, void* wk )
{
  GAMESYS_WORK  *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  CHAMPION_TMP  *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, CHAMPION_TMP_ASSIGN_ID );

  // 処理シーケンス
  enum{
    SEQ_START,          // イベント開始
    SEQ_LIGHT_ANIME,    // 光アニメ
    SEQ_LIGHT_WAIT,     // 光アニメ待ち
    SEQ_EXCHANGE,      // ルームモデル切り替え
    SEQ_END,            // イベント終了
  };

  switch( *seq ){
  case SEQ_START:
    *seq = SEQ_LIGHT_ANIME;
    break;

  // 四隅ライト点灯
  case SEQ_LIGHT_ANIME:
    ANM_LIGHT_Play( p_wk );
    *seq = SEQ_LIGHT_WAIT;
    break;

  // 光アニメ待ち
  case SEQ_LIGHT_WAIT:
    if( ANM_LIGHT_CheckEnd( p_wk ) == TRUE )
    {
      *seq = SEQ_EXCHANGE;
    }
    break;

  // ルームモデル切り替え
  case SEQ_EXCHANGE:
    {
      // ルーム表示切り替え
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_ROOM_1, TRUE );
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_ROOM_2, FALSE );

      // レリーフ表示切り替え
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_RELIEF_1, TRUE );
      FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_RELIEF_2, FALSE );
      // アニメはフレーム経過を引き継ぐ
      {
        int   anm_idx = 0;    // レリーフ回転ica
        fx32  tmp_frame;
        EXP_OBJ_ANM_CNT_PTR exp_anm;
        exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_RELIEF_1, anm_idx );

        // 点灯前の経過フレーム取得
        tmp_frame = FLD_EXP_OBJ_GetObjAnmFrm( p_wk->exp_obj, CHAMPION_UNIT_IDX, OBJ_RELIEF_1, anm_idx );

        // 点灯後再生
        ANM_Play( p_wk->exp_obj, OBJ_RELIEF_2, anm_idx, tmp_frame, TRUE );

        // 点灯前停止
        ANM_Stop( p_wk->exp_obj, OBJ_RELIEF_1, anm_idx );
      }

      // 切り替え後になったことを保存
      SAVE_SetExchangeFlag( p_wk->p_sv_wk, TRUE );

      // デフォルトカメラも頂上用に変更
      {
        FIELD_CAMERA *p_camera = FIELDMAP_GetFieldCamera( p_fieldmap );
        FIELD_CAMERA_SetCameraType( p_camera, TOP_CAMERA_ID );
      }

      *seq = SEQ_END;
    }
    break;

  case SEQ_END:
    return GMEVENT_RES_FINISH;

  default:
    GF_ASSERT(0);
  }

  return GMEVENT_RES_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  殿堂入り部屋出現イベント作成
 *
 *	@param	GAMESYS_WORK* gsys ゲームシステムポインタ
 *
 *	@return イベント
 */
//-----------------------------------------------------------------------------
static GMEVENT * HALL_ROOM_CreateEvent( GAMESYS_WORK* gsys )
{
  GMEVENT*  event;
  // イベント生成
  event = GMEVENT_Create( gsys, NULL, HallRoomEvent, 0 );

  return event;
}
//----------------------------------------------------------------------------
/**
 *	@brief  殿堂入り部屋出現イベント関数
 *
 *	@param	event イベントポインタ
 *	@param	seq   シーケンス
 *	@param	wk    ワーク
 *
 *	@return GMEVENT_RESULT
 */
//----------------------------------------------------------------------------
static GMEVENT_RESULT HallRoomEvent( GMEVENT* event, int* seq, void* wk )
{
  GAMESYS_WORK  *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  CHAMPION_TMP  *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, CHAMPION_TMP_ASSIGN_ID );

  // 処理シーケンス
  enum{
    SEQ_ANM_START,      // イベント開始
    SEQ_END,            // イベント終了
  };

  switch( *seq ){
  case SEQ_ANM_START:
    // 戦闘後アニメを再生
    {
      // レリーフのアニメ
      ANM_Stop( p_wk->exp_obj, OBJ_RELIEF_2, 0 );
      ANM_Play( p_wk->exp_obj, OBJ_RELIEF_2, 1, 0, FALSE );
      ANM_Play( p_wk->exp_obj, OBJ_RELIEF_2, 2, 0, FALSE );

      // マップのアニメ
      ANM_Play( p_wk->exp_obj, OBJ_ROOM_2, 0, 0, FALSE );
      ANM_Play( p_wk->exp_obj, OBJ_ROOM_2, 1, 0, FALSE );

      // カメラ振動タスクを登録
      {
        FIELD_TASK* cam;
        FIELD_TASK_MAN* taskMan;

        CAMERA_SHAKE_PARAM shakeParam;
        GFL_STD_MemFill(&shakeParam, 0, sizeof(shakeParam)); // パラメータの初期化
        shakeParam.Height          = 1;
        shakeParam.Sync            = 3;
        shakeParam.Time            = 22;
        shakeParam.WaitStartFrame  = 149;

        cam = FIELD_TASK_CameraShake( p_fieldmap, &shakeParam );
        taskMan = FIELDMAP_GetTaskManager( p_fieldmap );
        FIELD_TASK_MAN_AddTask( taskMan, cam, NULL );
      }
    }
    *seq = SEQ_END;
    break;

  case SEQ_END:
    return GMEVENT_RES_FINISH;

  default:
    GF_ASSERT(0);
  }

  return GMEVENT_RES_CONTINUE;
}

//=============================================================================
/**
 *          セーブ情報
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  モデル切り替え後フラグを設定
 *
 *	@param	CHAMPION_SV_WORK *p_wk  セーブワーク
 *	@param	flag  TRUEで切り替え後
 */
//-----------------------------------------------------------------------------
static void SAVE_SetExchangeFlag( CHAMPION_SV_WORK *p_wk, BOOL flag )
{
  p_wk->exchange_flag = flag;
}
//----------------------------------------------------------------------------
/**
 *	@brief  モデル切り替え後フラグを取得
 *
 *	@param	CHAMPION_SV_WORK *p_wk  セーブワーク
 *
 *	@return	flag  TRUEで切り替え後
 */
//-----------------------------------------------------------------------------
static BOOL SAVE_GetExchangeFlag( const CHAMPION_SV_WORK *p_wk )
{
  return p_wk->exchange_flag;
}

//=============================================================================
/**
 *          カメラゆれ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  カメラ揺れるタスクを作成する
 *
 *	@param	FIELDMAP_WORK* fieldmap   フィールドマップ
 *	@param	CAMERA_SHAKE_PARAM* shakeParam  ゆれパラメータ
 *
 *	@return 作成したタスク
 */
//-----------------------------------------------------------------------------
static FIELD_TASK* FIELD_TASK_CameraShake( FIELDMAP_WORK* p_fieldmap, const CAMERA_SHAKE_PARAM* shakeParam )
{
  FIELD_TASK* task;
  CAMERA_SHAKE_WORK* work;
  HEAPID heap_id = FIELDMAP_GetHeapID( p_fieldmap );

  // 生成
  task = FIELD_TASK_Create( heap_id, sizeof(CAMERA_SHAKE_WORK), CameraShake );
  // 初期化
  work              = FIELD_TASK_GetWork( task );
  work->p_fieldmap  = p_fieldmap;
  work->camera      = FIELDMAP_GetFieldCamera( p_fieldmap );
  work->shakeParam  = *shakeParam;
  work->seq         = 0;

  return task;
}
//----------------------------------------------------------------------------
/**
 *	@brief  カメラ揺れるタスク
 */
//-----------------------------------------------------------------------------
static FIELD_TASK_RETVAL CameraShake( void* wk )
{
  FIELD_CAMERA * camera;
  BOOL end = FALSE;
  CAMERA_SHAKE_WORK   *work = wk;
  CAMERA_SHAKE_PARAM  *param = &(work->shakeParam);

  camera = FIELDMAP_GetFieldCamera( work->p_fieldmap );

  // 開始フレーム経過まで待つ
  if( param->WaitStartFrame > 0 )
  {
    param->WaitStartFrame--;
    return FIELD_TASK_RETVAL_CONTINUE;
  }

  switch(work->seq){
  case 0:
    //トレース終了
    FIELD_CAMERA_StopTraceRequest(camera);
    work->seq++;
    break;
    /* break through */
  case 1:
    if ( FIELD_CAMERA_CheckTrace(work->camera) )
    {
      break;
    }
    {
      //復帰パラメータをセットする
      FIELD_CAMERA_SetRecvCamParam(work->camera);

      //カメラのターゲットを取得
      work->WatchTarget = FIELD_CAMERA_GetWatchTarget(camera);
      /*
      // レールカメラは
      // 毎フレームwatch_targetがdefault_targetにバインドされているので
      // 注視点を変えようと思っても上書きされてしまう。
      // その処理を打ち消すために、一時的にNULLにして、
      // targetが上書きされないようにする
      //
      // field_rail_func.c
      // field_camera.c
      */
      {
        FLDNOGRID_MAPPER* mapper = FIELDMAP_GetFldNoGridMapper( work->p_fieldmap );
        BOOL is_rail = FLDNOGRID_MAPPER_IsResistData( mapper );

        if( is_rail )
        {
          work->DefaultWatchTarget = FIELD_CAMERA_GetDefaultTarget(camera);
          FIELD_CAMERA_SetDefaultTarget( camera, NULL );
        }
      }

      //バインド解除
      FIELD_CAMERA_FreeTarget(camera);
      
      work->seq++;
    }
    /* break through */
  case 2:
    {
      //ターゲットを動かす
      int tmp;
      u16 rad;
      param->NowSync++;
      tmp = (param->NowSync * 0x10000) / param->Sync;
      rad = tmp;

      if (param->NowSync >= param->Sync)
      {
        param->Time--;
        param->NowSync = 0;
      }
      if (param->Time == 0)
      {
        param->NowSync = 0;
        end = TRUE;
      }

      if (end)
      {
        //カメラオフセットクリア
        {
          VecFx32 cam_ofs = {0,0,0};
          FIELD_CAMERA_SetCamPosOffset( camera, &cam_ofs );
        }

        //復帰パラメータをクリアする
        FIELD_CAMERA_ClearRecvCamParam(camera);
        
        // レールの時
        // default_targetを戻す
        {
          FLDNOGRID_MAPPER* mapper = FIELDMAP_GetFldNoGridMapper( work->p_fieldmap );
          BOOL is_rail = FLDNOGRID_MAPPER_IsResistData( mapper );

          if( is_rail )
          {
            FIELD_CAMERA_SetDefaultTarget( camera, work->DefaultWatchTarget );
          }
        }

        //際バインド
        FIELD_CAMERA_BindTarget(camera, work->WatchTarget);

        //トレース再開
        FIELD_CAMERA_RestartTrace(camera);
        work->seq++;
      }
      else
      {
        fx32 h;
        VecFx32 target;
        h = param->Height * FX_SinIdx(rad);
        target = *work->WatchTarget;
        target.y += h;

        //座標セット
        {
          FIELD_CAMERA_SetTargetPos( camera, &target );
        }
      }
    }
    break;
  case 3:
    return FIELD_TASK_RETVAL_FINISH;
  }

  return FIELD_TASK_RETVAL_CONTINUE;
}

