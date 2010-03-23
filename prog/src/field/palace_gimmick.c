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
#include "field/field_comm/intrude_field.h"
#include "field/field_comm/intrude_work.h"


//==============================================================================
//  定数定義
//==============================================================================
// 拡張オブジェクト・ユニット番号
#define EXPOBJ_UNIT_ELBOARD (1)

#define EXPOBJ_UNIT_IDX (0)
// ギミックワークのアサインID
#define GIMMICK_WORK_ASSIGN_ID (0)


//==========================================================================================
// ■ギミックワーク
//==========================================================================================
typedef struct
{
  u8 tree_palace_area;          ///<木を表示しているパレスエリア
  u8 padding[3];
}PALACE_GMK_WORK;


//==========================================================================================
// ■3Dリソース
//==========================================================================================
// リソースインデックス
typedef enum {
  RES_TREE_ALL,           // パレスの木全種
  
  RES_NUM
} RES_INDEX;
static const GFL_G3D_UTIL_RES res_table[ RES_NUM ] = 
{
  { ARCID_PALACE_TREE, NARC_p_tree_p_tree_all_nsbmd, GFL_G3D_UTIL_RESARC },     // パレスの木全種
};

// オブジェクトインデックス
typedef enum {
  OBJ_TREE_W_1,
  OBJ_TREE_W_2,
  OBJ_TREE_W_3,
  OBJ_TREE_W_4,
  OBJ_TREE_W_5,
  OBJ_TREE_W_6,
  OBJ_TREE_W_7,
  OBJ_TREE_W_8,
  OBJ_TREE_W_9,
  OBJ_TREE_W_10,

  OBJ_TREE_B_1,
  OBJ_TREE_B_2,
  OBJ_TREE_B_3,
  OBJ_TREE_B_4,
  OBJ_TREE_B_5,
  OBJ_TREE_B_6,
  OBJ_TREE_B_7,
  OBJ_TREE_B_8,
  OBJ_TREE_B_9,
  OBJ_TREE_B_10,

  OBJ_NUM
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
  { RES_TREE_ALL, 5, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 6, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 7, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 8, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 9, RES_TREE_ALL, NULL, 0 },
  //黒
  { RES_TREE_ALL, 0+10, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 1+10, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 2+10, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 3+10, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 4+10, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 5+10, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 6+10, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 7+10, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 8+10, RES_TREE_ALL, NULL, 0 },
  { RES_TREE_ALL, 9+10, RES_TREE_ALL, NULL, 0 },
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
  // 拡張オブジェクトのユニットを追加
  FLD_EXP_OBJ_AddUnit( exobj_cnt, &setup, EXPOBJ_UNIT_IDX );

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

/**
  u32*         gmk_save = (u32*)GIMMICKWORK_Get( gmkwork, FLD_GIMMICK_H01 );
  PALACE_GMK_WORK*         work = (PALACE_GMK_WORK*)gmk_save[0]; // gmk_save[0]はギミック管理ワークのアドレス
  // セーブ
  SaveGimmick( work, fieldWork );

*/
  //ユニット破棄
  FLD_EXP_OBJ_DelUnit( exobj_cnt, EXPOBJ_UNIT_IDX );  
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
  
  if(intcomm != NULL && intcomm->intrude_status_mine.palace_area != work->tree_palace_area){
    _AllVanish(exobj_cnt);
    work->tree_palace_area = _TreeEnable(gdata, exobj_cnt, intcomm);
  }
}


//==============================================================================
//  
//==============================================================================
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
  GFL_G3D_OBJSTATUS* status;
  int i;
  
  // オブジェクトを作成
  for(i = 0; i < OBJ_NUM; i++){
    status = FLD_EXP_OBJ_GetUnitObjStatus( exobj_cnt, EXPOBJ_UNIT_IDX, i );
    status->trans.x = PALACE_MAP_TREE_X;
    status->trans.y = PALACE_MAP_TREE_Y;
    status->trans.z = PALACE_MAP_TREE_Z;
    //カリングする
    FLD_EXP_OBJ_SetCulling(exobj_cnt, EXPOBJ_UNIT_IDX, i, TRUE);
    //非表示
    FLD_EXP_OBJ_SetVanish(exobj_cnt, EXPOBJ_UNIT_IDX, i, TRUE);
  }

  work->tree_palace_area = _TreeEnable(gdata, exobj_cnt, intcomm);
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

  for(i = 0; i < OBJ_NUM; i++){
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
  
  tree_level = wb_level % 3;
  if(tree_level > 9){
    tree_level = 9;
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
  int pm_version;
  int tree_w_lv, tree_b_lv;
  const OCCUPY_INFO *occupy;
  int palace_area, map_offset;
  
  if(intcomm == NULL){
    palace_area = 0;
    map_offset = 0;
  }
  else{
    palace_area = intcomm->intrude_status_mine.palace_area;
    //子機の場合、palace_area == 0 が左端、ではなく自分のNetIDのパレスが左端の為。
    map_offset = palace_area - GAMEDATA_GetIntrudeMyID(gamedata);
    if(map_offset < 0){
      map_offset = intcomm->member_num + map_offset;
    }
  }
  
  if(intcomm == NULL || palace_area == GAMEDATA_GetIntrudeMyID(gamedata)){
    pm_version = PM_VERSION;
    occupy = GAMEDATA_GetMyOccupyInfo(gamedata);
  }
  else{
    pm_version = MyStatus_GetRomCode( Intrude_GetMyStatus(intcomm, palace_area) );
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





