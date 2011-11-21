//======================================================================
/**
 * @file  palace_gimmick.c
 * @brief  パレスギミック
 * @author  Saito
 */
//======================================================================
#include "fieldmap.h"
#include "palace_gimmick.h"
#include "field_gimmick_def.h"
#include "savedata/gimmickwork.h"
#include "gmk_tmp_wk.h"
#include "p_tree.naix"
#include "palace_effect.naix"
#include "field/field_comm/intrude_field.h"
#include "field/field_comm/intrude_work.h"

// sound
#include "sound/pm_sndsys.h"

//==============================================================================
//  定数定義
//==============================================================================
#define EXPOBJ_UNIT_IDX (0)
#define GIMMICK_WORK_ASSIGN_ID (0)

// 座標関係
#define MAP_SIZE          (512 << FX32_SHIFT )
#define MAP_HALF_SIZE     (256 << FX32_SHIFT )

//==========================================================================================
// ■ギミックワーク
//==========================================================================================
typedef struct
{
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ

  u8    tree_palace_area;          ///<木を表示しているパレスエリア
  BOOL  bridge_flg;
}PALACE_GMK_WORK;


//==========================================================================================
// ■3Dリソース
//==========================================================================================
// リソースインデックス
typedef enum {
  RES_TREE_ALL,           // パレスの木全種

  // EFFECT
  RES_EFFECT_01_MDL,      // 波紋(常時)
  RES_EFFECT_01_ANM_IMA,  // 
  RES_EFFECT_01_ANM_ITA,  // 
  RES_EFFECT_02_MDL,      // 波紋(橋の上)
  RES_EFFECT_02_ANM_ITA,  // 
  RES_EFFECT_03_MDL,      // ミッション開始時
  RES_EFFECT_03_ANM_ICA,  //
  RES_EFFECT_03_ANM_IMA,  //
  RES_EFFECT_03_ANM_ITA,  //
  RES_EFFECT_04_MDL,      // ミッション開始時2
  RES_EFFECT_04_ANM_ICA,  // 

  RES_NUM
} RES_INDEX;
static const GFL_G3D_UTIL_RES res_table[ RES_NUM ] = 
{
  { ARCID_PALACE_TREE, NARC_p_tree_p_tree_all_nsbmd, GFL_G3D_UTIL_RESARC },     // パレスの木全種

  // EFFECT
  { ARCID_PALACE_EFFECT, NARC_palace_effect_hilink_e01_nsbmd, GFL_G3D_UTIL_RESARC },    // 波紋(常時)
  { ARCID_PALACE_EFFECT, NARC_palace_effect_hilink_e01_nsbma, GFL_G3D_UTIL_RESARC },    // 
  { ARCID_PALACE_EFFECT, NARC_palace_effect_hilink_e01_nsbta, GFL_G3D_UTIL_RESARC },    // 
  { ARCID_PALACE_EFFECT, NARC_palace_effect_hilink_e02_nsbmd, GFL_G3D_UTIL_RESARC },    // 波紋(橋の上)
  { ARCID_PALACE_EFFECT, NARC_palace_effect_hilink_e02_nsbta, GFL_G3D_UTIL_RESARC },    // 
  { ARCID_PALACE_EFFECT, NARC_palace_effect_hilink_e03_nsbmd, GFL_G3D_UTIL_RESARC },    // ミッション開始時
  { ARCID_PALACE_EFFECT, NARC_palace_effect_hilink_e03_s_nsbca, GFL_G3D_UTIL_RESARC },  // 
  { ARCID_PALACE_EFFECT, NARC_palace_effect_hilink_e03_s_nsbma, GFL_G3D_UTIL_RESARC },  // 
  { ARCID_PALACE_EFFECT, NARC_palace_effect_hilink_e03_nsbta, GFL_G3D_UTIL_RESARC },    // 
  { ARCID_PALACE_EFFECT, NARC_palace_effect_hilink_e04_nsbmd, GFL_G3D_UTIL_RESARC },    // ミッション開始時2
  { ARCID_PALACE_EFFECT, NARC_palace_effect_hilink_e04_nsbca, GFL_G3D_UTIL_RESARC },    // 
};

// ANIME
static const GFL_G3D_UTIL_ANM anm_effect_01[] =
{
  { RES_EFFECT_01_ANM_IMA, 0 },
  { RES_EFFECT_01_ANM_ITA, 0 },
};
#define ANM_EFFECT_01_NUM  ( NELEMS(anm_effect_01) )

static const GFL_G3D_UTIL_ANM anm_effect_02[] =
{
  { RES_EFFECT_02_ANM_ITA, 0 },
};
#define ANM_EFFECT_02_NUM  ( NELEMS(anm_effect_02) )

static const GFL_G3D_UTIL_ANM anm_effect_03[] =
{
  { RES_EFFECT_03_ANM_ICA, 0 },
  { RES_EFFECT_03_ANM_IMA, 0 },
  { RES_EFFECT_03_ANM_ITA, 0 },
};
#define ANM_EFFECT_03_NUM  ( NELEMS(anm_effect_03) )

static const GFL_G3D_UTIL_ANM anm_effect_04[] =
{
  { RES_EFFECT_04_ANM_ICA, 0 },
};
#define ANM_EFFECT_04_NUM  ( NELEMS(anm_effect_04) )

// アニメ総数のテーブル
static const int anm_num_tbl[] =
{
  ANM_EFFECT_01_NUM,
  ANM_EFFECT_02_NUM,
  ANM_EFFECT_03_NUM,
  ANM_EFFECT_04_NUM,
};

// オブジェクトインデックス
typedef enum {
  OBJ_TREE_B_1,
  OBJ_TREE_B_2,
  OBJ_TREE_B_3,
  OBJ_TREE_B_4,
  OBJ_TREE_B_5,

  OBJ_TREE_W_1,
  OBJ_TREE_W_2,
  OBJ_TREE_W_3,
  OBJ_TREE_W_4,
  OBJ_TREE_W_5,

  OBJ_EFFECT_01,
  OBJ_EFFECT_02,
  OBJ_EFFECT_03,
  OBJ_EFFECT_04,

  OBJ_NUM,

  TREE_OBJ_NUM = OBJ_EFFECT_01,
} OBJ_INDEX;
static const GFL_G3D_UTIL_OBJ obj_table[ OBJ_NUM ] = 
{
  // モデルリソースID, 
  // モデルデータID(リソース内部INDEX), 
  // テクスチャリソースID,
  // アニメテーブル, 
  // アニメリソース数
  
  //白
  { RES_TREE_ALL, 0, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 1, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 2, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 3, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 4, RES_TREE_ALL, NULL, 0 },
  //黒
  { RES_TREE_ALL, 0+5, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 1+5, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 2+5, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 3+5, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 4+5, RES_TREE_ALL, NULL, 0 },


  //EFFECT
  { RES_EFFECT_01_MDL, 0, RES_EFFECT_01_MDL, anm_effect_01, ANM_EFFECT_01_NUM },
  { RES_EFFECT_02_MDL, 0, RES_EFFECT_02_MDL, anm_effect_02, ANM_EFFECT_02_NUM },
  { RES_EFFECT_03_MDL, 0, RES_EFFECT_03_MDL, anm_effect_03, ANM_EFFECT_03_NUM },
  { RES_EFFECT_04_MDL, 0, RES_EFFECT_04_MDL, anm_effect_04, ANM_EFFECT_04_NUM },
};

static const GFL_G3D_UTIL_SETUP setup = { res_table, RES_NUM, obj_table, OBJ_NUM };


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void InitWork( PALACE_GMK_WORK* work, FIELDMAP_WORK* fieldWork, GAMEDATA *gdata, INTRUDE_COMM_SYS_PTR intcomm );
static void _AllVanish(FLD_EXP_OBJ_CNT_PTR exobj_cnt);
static int _GetTreeLevel(int wb_level);
static u8 _TreeEnable(GAMEDATA *gamedata, FLD_EXP_OBJ_CNT_PTR exobj_cnt, INTRUDE_COMM_SYS_PTR intcomm);
static INTRUDE_COMM_SYS_PTR _GetIntrudeComm(GAME_COMM_SYS_PTR game_comm);

// EXPOBJ
static void EXPOBJ_ObjSet( PALACE_GMK_WORK *p_wk );
static void EXPOBJ_ObjDell( PALACE_GMK_WORK *p_wk );
static void EXPOBJ_ObjMain( PALACE_GMK_WORK *p_wk );

// アニメ管理
// 共通
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx, BOOL loop );
static void ANM_Stop( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx );

// エフェクト
static void ANM_EFFECT_01_Play( PALACE_GMK_WORK *p_wk );
static void ANM_EFFECT_02_Play( PALACE_GMK_WORK *p_wk );
static void ANM_EFFECT_03_Play( PALACE_GMK_WORK *p_wk );
static void ANM_EFFECT_04_Play( PALACE_GMK_WORK *p_wk );

// チェック
static BOOL CHECK_BridgeInPos( PALACE_GMK_WORK *p_wk );
static BOOL CHECK_BridgeOutPos( PALACE_GMK_WORK *p_wk );

//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * セットアップ関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void PALACE_GMK_Setup(FIELDMAP_WORK *fieldWork)
{
  PALACE_GMK_WORK* work;  // ギミック管理ワーク
  HEAPID                heapID = FIELDMAP_GetHeapID( fieldWork );
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldWork );
  GAMESYS_WORK*            gsys = FIELDMAP_GetGameSysWork( fieldWork );
  GAMEDATA*               gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*          gmkwork = GAMEDATA_GetGimmickWork(gdata);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  INTRUDE_COMM_SYS_PTR intcomm = _GetIntrudeComm(game_comm);

  //汎用ワーク確保
  work = GMK_TMP_WK_AllocWork
      (fieldWork, GIMMICK_WORK_ASSIGN_ID, heapID, sizeof(PALACE_GMK_WORK));
  work->exp_obj = FIELDMAP_GetExpObjCntPtr( fieldWork );
  work->p_fieldmap = fieldWork;

  // EXPOBJセット
  EXPOBJ_ObjSet( work );

  // ギミック管理ワークを初期化 
  InitWork( work, fieldWork, gdata, intcomm );
}

//--------------------------------------------------------------
/**
 * 解放関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void PALACE_GMK_End(FIELDMAP_WORK *fieldWork)
{
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldWork );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldWork );
  PALACE_GMK_WORK* work = GMK_TMP_WK_GetWork(fieldWork, GIMMICK_WORK_ASSIGN_ID);

  // EXPOBJ破棄
  EXPOBJ_ObjDell( work );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork(fieldWork, GIMMICK_WORK_ASSIGN_ID);
}


//--------------------------------------------------------------
/**
 * 動作関数
 * @param	      fieldWork     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void PALACE_GMK_Move(FIELDMAP_WORK *fieldWork)
{
  GAMESYS_WORK*    gsys = FIELDMAP_GetGameSysWork( fieldWork );
  GAMEDATA*       gdata = GAMESYSTEM_GetGameData( gsys );
  GIMMICKWORK*  gmkwork = GAMEDATA_GetGimmickWork(gdata);
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldWork );
  PALACE_GMK_WORK* work = GMK_TMP_WK_GetWork(fieldWork, GIMMICK_WORK_ASSIGN_ID);
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(gsys);
  INTRUDE_COMM_SYS_PTR intcomm = _GetIntrudeComm(game_comm);
  

#ifdef PM_DEBUG
  if (GFL_UI_KEY_GetCont() & PAD_BUTTON_L){
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      OS_Printf("デバッグ エフェクト01再生\n");

      ANM_EFFECT_01_Play( work );
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      OS_Printf("デバッグ エフェクト02再生\n");

      ANM_EFFECT_02_Play( work );
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
    {
      OS_Printf("デバッグ エフェクト03再生\n");

      ANM_EFFECT_03_Play( work );
    }
    if ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R )
    {
      OS_Printf("デバッグ エフェクト04再生\n");

      ANM_EFFECT_04_Play( work );
    }
  }
#endif


  if(intcomm != NULL && intcomm->intrude_status_mine.palace_area != work->tree_palace_area){
    _AllVanish(exobj_cnt);
    work->tree_palace_area = _TreeEnable(gdata, exobj_cnt, intcomm);
  }

  // エフェクト
  {

    if( work->bridge_flg == FALSE )
    {
      // 橋の上チェック
      if( CHECK_BridgeInPos( work ) == TRUE )
      {
        work->bridge_flg = TRUE;
        ANM_EFFECT_02_Play( work );

        // 波紋SE
        PMSND_PlaySE( SEQ_SE_SW_FES_14 );

      }
    }
    else
    {
      // 橋の外チェック
      if( CHECK_BridgeOutPos( work ) == TRUE )
      {
        work->bridge_flg = FALSE;
      }
    }

  }

  // EXPOBJアニメメイン
  EXPOBJ_ObjMain( work );

}

//----------------------------------------------------------------------------
/**
 *	@brief  ミッション開始のエフェクト開始
 *
 *	@param	GAMESYS_WORK *gsys 
 */
//-----------------------------------------------------------------------------
void PALACE_GMK_EffectStart( GAMESYS_WORK *gsys )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  PALACE_GMK_WORK *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_WORK_ASSIGN_ID);

  ANM_EFFECT_03_Play( p_wk );

  // 柱SE
  PMSND_PlaySE( SEQ_SE_SW_FES_15 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ミッション開始のエフェクト開始 2つ目
 *
 *	@param	GAMESYS_WORK *gsys 
 */
//-----------------------------------------------------------------------------
void PALACE_GMK_EffectStart2( GAMESYS_WORK *gsys )
{
  FIELDMAP_WORK *p_fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  PALACE_GMK_WORK *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_WORK_ASSIGN_ID);

  ANM_EFFECT_04_Play( p_wk );
}

//==============================================================================
//  				static関数
//==============================================================================
//=============================================================================
/**
 *          EXPOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	PALACE_GMK_WORK *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( PALACE_GMK_WORK *p_wk )
{
  GFL_G3D_OBJSTATUS* status;
  int i;

  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnit( p_wk->exp_obj, &setup, EXPOBJ_UNIT_IDX);

  //=====================================
  // ハイルツリー
  for(i = 0; i < TREE_OBJ_NUM; i++){
    status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX, i );
    status->trans.x = PALACE_MAP_TREE_X;
    status->trans.y = PALACE_MAP_TREE_Y;
    status->trans.z = PALACE_MAP_TREE_Z;
    //カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX, i, TRUE);
    //非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, i, TRUE);
  } 

  //=====================================
  // エフェクト
  for( i=OBJ_EFFECT_01; i<=OBJ_EFFECT_04; i++ )
  {
    int anm_idx;
    int anm_num = anm_num_tbl[ i - OBJ_EFFECT_01 ];
    status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, EXPOBJ_UNIT_IDX, i );
    status->trans.x = MAP_SIZE;
    status->trans.y = 0;
    status->trans.z = MAP_SIZE;
    //カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, EXPOBJ_UNIT_IDX, i, TRUE);
    //非表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, i, TRUE);

    // アニメ再生
    for( anm_idx=0; anm_idx<anm_num; anm_idx++ )
    {
      EXP_OBJ_ANM_CNT_PTR exp_anm;
      exp_anm = FLD_EXP_OBJ_GetAnmCnt( p_wk->exp_obj, EXPOBJ_UNIT_IDX, i, anm_idx );

      // 1回再生設定
      FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, FALSE );  // ループ再生設定
      FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );   // アニメ再生設定

      // 無効化
      FLD_EXP_OBJ_ValidCntAnm( p_wk->exp_obj, EXPOBJ_UNIT_IDX, i, anm_idx, FALSE );
    }
  }
  
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	PALACE_GMK_WORK *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( PALACE_GMK_WORK *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, EXPOBJ_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	PALACE_GMK_WORK *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( PALACE_GMK_WORK *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
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
 *	@param	loop      ループ再生フラグ(TRUEでループ
 */
//-----------------------------------------------------------------------------
static void ANM_Play( FLD_EXP_OBJ_CNT_PTR exp_obj, u16 obj_idx, u16 anm_idx, BOOL loop )
{
  EXP_OBJ_ANM_CNT_PTR exp_anm;
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, anm_idx );

  // 再生
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, FALSE );
  FLD_EXP_OBJ_ChgAnmLoopFlg( exp_anm, loop );

  // 無効化解除
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, anm_idx, TRUE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, anm_idx, 0 );
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
  exp_anm = FLD_EXP_OBJ_GetAnmCnt( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, anm_idx );
  // 停止
  FLD_EXP_OBJ_ChgAnmStopFlg( exp_anm, TRUE );
  // 無効化
  FLD_EXP_OBJ_ValidCntAnm( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, anm_idx, FALSE );
  // 頭だし
  FLD_EXP_OBJ_SetObjAnmFrm( exp_obj, EXPOBJ_UNIT_IDX, obj_idx, anm_idx, 0 );
}


//------------------------------------------------------------------------------------------
/**
 * @brief ギミック管理ワークを初期化する
 *
 * @param work     初期化対象ワーク
 * @param fieldWork 依存するフィールドマップ
 */
//------------------------------------------------------------------------------------------
static void InitWork( PALACE_GMK_WORK* work, FIELDMAP_WORK* fieldWork, GAMEDATA *gdata, INTRUDE_COMM_SYS_PTR intcomm )
{
  FLD_EXP_OBJ_CNT_PTR exobj_cnt = FIELDMAP_GetExpObjCntPtr( fieldWork );
  int i;

  work->tree_palace_area = _TreeEnable(gdata, exobj_cnt, intcomm);
  if( CHECK_BridgeInPos( work ) == TRUE )
  {
    work->bridge_flg = TRUE;
  }
  else
  {
    work->bridge_flg = FALSE;
  }

  // エフェクト再生
  ANM_EFFECT_01_Play( work );
}

//--------------------------------------------------------------
/**
 * 全ての木を表示OFFする
 *
 * @param   exobj_cnt		
 */
//--------------------------------------------------------------
static void _AllVanish(FLD_EXP_OBJ_CNT_PTR exobj_cnt)
{
  int i;

  for(i = 0; i < TREE_OBJ_NUM; i++){
    FLD_EXP_OBJ_SetVanish(exobj_cnt, EXPOBJ_UNIT_IDX, i, TRUE);
  }
}

//--------------------------------------------------------------
/**
 * 白黒レベルから木のレベルを取得する
 *
 * @param   wb_level		白 or 黒 のレベル
 *
 * @retval  int		木のレベル
 */
//--------------------------------------------------------------
static int _GetTreeLevel(int wb_level)
{
  int tree_level;
  
  tree_level = wb_level / 6;
  if(tree_level > 4){
    tree_level = 4;
  }
  return tree_level;
}

//--------------------------------------------------------------
/**
 * 現在のパレスエリアの木をレベルに合わせたものにし、表示ONする
 *
 * @param   exobj_cnt		
 * @param   intcomm		
 *
 * @retval  現在のパレスエリア
 */
//--------------------------------------------------------------
static u8 _TreeEnable(GAMEDATA *gamedata, FLD_EXP_OBJ_CNT_PTR exobj_cnt, INTRUDE_COMM_SYS_PTR intcomm)
{
  GFL_G3D_OBJSTATUS* status;
  OBJ_INDEX obj_id;
  int tree_w_lv, tree_b_lv;
  const OCCUPY_INFO *occupy;
  int palace_area, map_offset;
  
  palace_area = GAMEDATA_GetIntrudePalaceArea(gamedata);
  //子機の場合、palace_area == 0 が左端、ではなく自分のNetIDのパレスが左端の為。
  map_offset = palace_area - GAMEDATA_GetIntrudeMyID(gamedata);
  if(map_offset < 0){
    map_offset = GAMEDATA_GetIntrudeNum(gamedata) + map_offset;
  }
  
  if(palace_area == GAMEDATA_GetIntrudeMyID(gamedata)){
    occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
  }
  else{
    occupy = GAMEDATA_GetOccupyInfo(gamedata, palace_area);
  }
  tree_w_lv = _GetTreeLevel( OccupyInfo_GetWhiteLevel(occupy) );
  tree_b_lv = _GetTreeLevel( OccupyInfo_GetBlackLevel(occupy) );
  
  //白
  status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TREE_W_1 + tree_w_lv );
  status->trans.x = PALACE_MAP_LEN * map_offset + PALACE_MAP_TREE_X;
  status->trans.y = PALACE_MAP_TREE_Y;
  status->trans.z = PALACE_MAP_TREE_Z;
  //黒
  status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TREE_B_1 + tree_b_lv );
  status->trans.x = PALACE_MAP_LEN * map_offset + PALACE_MAP_TREE_X;
  status->trans.y = PALACE_MAP_TREE_Y;
  status->trans.z = PALACE_MAP_TREE_Z;
  //表示
  FLD_EXP_OBJ_SetVanish(exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TREE_W_1 + tree_w_lv, FALSE);
  FLD_EXP_OBJ_SetVanish(exobj_cnt, EXPOBJ_UNIT_IDX, OBJ_TREE_B_1 + tree_b_lv, FALSE);
  
  return palace_area;
}

//--------------------------------------------------------------
/**
 * intcommを取得
 *
 * @param   game_comm		
 *
 * @retval  INTRUDE_COMM_SYS_PTR		
 */
//--------------------------------------------------------------
static INTRUDE_COMM_SYS_PTR _GetIntrudeComm(GAME_COMM_SYS_PTR game_comm)
{
  if(GameCommSys_BootCheck(game_comm) == GAME_COMM_NO_INVASION){
    return GameCommSys_GetAppWork(game_comm);
  }
  return NULL;
}

//=============================================================================
/**
 *        エフェクト
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  波紋(常時)アニメ再生	
 *
 *	@param	PALACE_GMK_WORK *p_wk  ギミックワーク
 */
//-----------------------------------------------------------------------------
static void ANM_EFFECT_01_Play( PALACE_GMK_WORK *p_wk )
{
  int i;
  u16 obj_idx = OBJ_EFFECT_01;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<ANM_EFFECT_01_NUM; i++ )
  {
    ANM_Play( p_wk->exp_obj, obj_idx, i, TRUE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  波紋(橋上時)アニメ再生	
 *
 *	@param	PALACE_GMK_WORK *p_wk  ギミックワーク
 */
//-----------------------------------------------------------------------------
static void ANM_EFFECT_02_Play( PALACE_GMK_WORK *p_wk )
{
  int i;
  u16 obj_idx = OBJ_EFFECT_02;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, FALSE );

  for( i=0; i<ANM_EFFECT_02_NUM; i++ )
  {
    ANM_Play( p_wk->exp_obj, obj_idx, i, FALSE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ミッション開始時エフェクト アニメ再生	
 *
 *	@param	PALACE_GMK_WORK *p_wk  ギミックワーク
 */
//-----------------------------------------------------------------------------
static void ANM_EFFECT_03_Play( PALACE_GMK_WORK *p_wk )
{
  int i;
  u16 obj_idx = OBJ_EFFECT_03;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, FALSE );

  ANM_Play( p_wk->exp_obj, obj_idx, 0, FALSE ); // ICA
  ANM_Play( p_wk->exp_obj, obj_idx, 1, FALSE ); // IMA
  ANM_Play( p_wk->exp_obj, obj_idx, 2, TRUE ); // ITA
}

//----------------------------------------------------------------------------
/**
 *	@brief  ミッション開始時エフェクト2 アニメ再生	
 *
 *	@param	PALACE_GMK_WORK *p_wk  ギミックワーク
 */
//-----------------------------------------------------------------------------
static void ANM_EFFECT_04_Play( PALACE_GMK_WORK *p_wk )
{
  int i;
  u16 obj_idx = OBJ_EFFECT_04;

  // 表示
  FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, EXPOBJ_UNIT_IDX, obj_idx, FALSE );

  ANM_Play( p_wk->exp_obj, obj_idx, 0, FALSE ); // ICA
}

//----------------------------------------------------------------------------
/**
 *	@brief  主人公が橋の上にいるかチェック
 *
 *	@param	PALACE_GMK_WORK *p_wk 
 *
 *	@return TRUEで橋の上
 */
//-----------------------------------------------------------------------------
static BOOL CHECK_BridgeInPos( PALACE_GMK_WORK *p_wk )
{
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
  VecFx32 pos;

  // 主人公位置取得
  FIELD_PLAYER_GetPos( cp_player, &pos );

  if( pos.x == PALACE_MAP_BRIDGE_X &&
      ( pos.z >= PALACE_MAP_BRIDGE_01_Z && pos.z <= PALACE_MAP_BRIDGE_02_Z )
    )
  {
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  主人公が橋の外にいるかチェック
 *
 *	@param	PALACE_GMK_WORK *p_wk 
 *
 *	@return TRUEで橋の上
 */
//-----------------------------------------------------------------------------
static BOOL CHECK_BridgeOutPos( PALACE_GMK_WORK *p_wk )
{
  const FIELD_PLAYER *cp_player  = FIELDMAP_GetFieldPlayer( p_wk->p_fieldmap );
  VecFx32 pos;

  // 主人公位置取得
  FIELD_PLAYER_GetPos( cp_player, &pos );

  if( pos.x == PALACE_MAP_BRIDGE_X && pos.z == PALACE_MAP_BRIDGE_OUT_Z )
  {
    return TRUE;
  }

  return FALSE;
}

