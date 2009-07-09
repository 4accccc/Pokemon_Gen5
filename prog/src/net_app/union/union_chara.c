//==============================================================================
/**
 * @file    union_char.c
 * @brief   ユニオンルームでの人物関連
 * @author  matsuda
 * @date    2009.07.06(月)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gamesystem/game_comm.h"
#include "union_types.h"
#include "net_app/union/union_main.h"
#include "net_app/union/union_beacon_tool.h"
#include "fieldmap/zone_id.h"
#include "net_app/union/union_chara.h"
#include "union_local.h"


//==============================================================================
//  型定義
//==============================================================================
///ビーコンPCの動作関数型
typedef BOOL (*BPC_FUNC)(UNION_BEACON_PC *bpc, MMDL *mmdl, u8 *seq);

///ビーコンPCの動作テーブル型
typedef struct{
  BPC_FUNC func_init;
  BPC_FUNC func_update;
  BPC_FUNC func_exit;
  u8 event_pri;///<イベント優先順位(数値が低いイベントでは大きいイベントから切り替える事は出来ない)
  u8 life_dec;  ///<TRUE:寿命を減らしてもよい状態
  u8 padding[2];
}BEACON_PC_FUNC;


//==============================================================================
//  定数定義
//==============================================================================
///ビーコンPC動作サブプロセスNo
enum{
  BPC_SUBPROC_INIT,     ///<初期化処理
  BPC_SUBPROC_UPDATE,   ///<更新処理
  BPC_SUBPROC_EXIT,     ///<終了処理
};


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static BOOL BeaconPCFunc(UNION_BEACON_PC *bpc, MMDL *mmdl);


//==============================================================================
//  データ
//==============================================================================
///キャラクタの配置座標
static const struct{
  u16 x;    //グリッド座標：X
  u16 z;    //グリッド座標：Z
}UnionCharPosTbl[] = {
  {10 + 3, 14 - 4},   //index 0
  {10 - 6, 14 - 2},   //index 1
  {10 - 3, 14 - 4},   //index 2
  {10 + 3, 14 - 9},   //index 3
  {10 + 6, 14 - 2},   //index 4
  {10 - 6, 14 - 7},   //index 5
  {10 + 6, 14 - 7},   //index 6
  {10 + 0, 14 - 7},   //index 7
  {10 - 3, 14 - 9},   //index 8
  {10 + 0, 14 - 2},   //index 9
};

///ユニオンキャラクターのMMDLヘッダーデータ
static const MMDL_HEADER UnionChar_MMdlHeader = 
{
  0,
  0,
  MV_DIR_RND,
  0,	///<イベントタイプ
  0,	///<イベントフラグ
  0,	///<イベントID
  DIR_DOWN,	///<指定方向
  0,	///<指定パラメタ 0
  0,	///<指定パラメタ 1
  0,	///<指定パラメタ 2
  0,	///<X方向移動制限
  0,	///<Z方向移動制限
  0,	///<グリッドX
  0,	///<グリッドZ
  0,	///<Y値 fx32型
};

///ビーコンPCの動作テーブル
static const BEACON_PC_FUNC BeaconPcFuncTbl[] = {
  {//BPC_EVENT_STATUS_NULL
    NULL,
    NULL,
    NULL,
    0,
    TRUE,
  },
  {//BPC_EVENT_STATUS_ENTER
    NULL,
    NULL,
    NULL,
    0xff,
    FALSE,
  },
  {//BPC_EVENT_STATUS_LEAVE
    NULL,
    NULL,
    NULL,
    0xff,
    FALSE,
  },
};
SDK_COMPILER_ASSERT(NELEMS(BeaconPcFuncTbl) == BPC_EVENT_STATUS_MAX);



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * ユニオンキャラクター：登録
 *
 * @param   unisys		      
 * @param   gdata		        ゲームデータへのポインタ
 * @param   trainer_view		見た目
 * @param   sex		          性別
 * @param   char_index		  キャラクターIndex
 *
 * @retval  MMDL *		      
 */
//==================================================================
MMDL * UNION_CHAR_AddOBJ(UNION_SYSTEM_PTR unisys, GAMEDATA *gdata, u8 trainer_view, u16 char_index)
{
  MMDL *addmdl;
  MMDLSYS *mdlsys;
  MMDL_HEADER head;
  
  mdlsys = GAMEDATA_GetMMdlSys( gdata );
  
  head = UnionChar_MMdlHeader;
  head.id = char_index;
  head.obj_code = UnionView_GetObjCode(trainer_view);
  head.gx = UnionCharPosTbl[char_index].x;
  head.gz = UnionCharPosTbl[char_index].z;
  
  OS_TPrintf("obj_code = %d, id = %d\n", head.obj_code, char_index);
  addmdl = MMDLSYS_AddMMdl(mdlsys, &head, ZONE_ID_UNION);
  return addmdl;
}

//==================================================================
/**
 * ユニオンキャラクター：更新処理
 *
 * @param   unisys		
 */
//==================================================================
void UNION_CHAR_Update(UNION_SYSTEM_PTR unisys, GAMEDATA *gdata)
{
  int i;
  UNION_BEACON_PC *bpc;
  BOOL ret;
  MMDL *mmdl;
  MMDLSYS *mdlsys;
  
  mdlsys = GAMEDATA_GetMMdlSys( gdata );

  for(i = 0; i < UNION_RECEIVE_BEACON_MAX; i++){
    bpc = &unisys->receive_beacon[i];
    if(bpc->beacon.data_valid == UNION_BEACON_VALID){
      mmdl = MMDLSYS_SearchOBJID( mdlsys, i );
      if(mmdl == NULL){ //ありえないはずだけど一応。無い場合は削除扱いにする
        GFL_STD_MemClear(bpc, sizeof(UNION_BEACON_PC));
        GF_ASSERT_MSG(0, "mmdl == NULL! %d", i);
        continue;
      }
      
      //寿命チェック
      if(BeaconPcFuncTbl[bpc->event_status].life_dec == TRUE){
        if(bpc->life <= 0){
          UNION_CHAR_EventReq(bpc, BPC_EVENT_STATUS_LEAVE);
        }
        else{
          bpc->life--;
          OS_TPrintf("life = %d\n", bpc->life);
        }
      }
      
      //動作実行
      ret = BeaconPCFunc(bpc, mmdl);
      if(ret == FALSE){
        MMDL_Delete(mmdl);
        GFL_STD_MemClear(bpc, sizeof(UNION_BEACON_PC));
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * ビーコンPC：イベントリクエスト
 *
 * @param   bpc		          ビーコンPCへのポインタ
 * @param   event_status		BPC_EVENT_STATUS_???
 *
 * @retval  TRUE:リクエスト正常受付。　FALSE:リクエスト拒否
 */
//--------------------------------------------------------------
BOOL UNION_CHAR_EventReq(UNION_BEACON_PC *bpc, int event_status)
{
  //今と次のリクエスト、両方のプライオリティよりも高い場合はリクエスト受付
  if(BeaconPcFuncTbl[bpc->event_status].event_pri < BeaconPcFuncTbl[event_status].event_pri
      && BeaconPcFuncTbl[bpc->next_event_status].event_pri < BeaconPcFuncTbl[event_status].event_pri){
    bpc->next_event_status = event_status;
    if(bpc->func_proc != BPC_SUBPROC_EXIT){
      bpc->func_proc = BPC_SUBPROC_EXIT;
      bpc->func_seq = 0;
    }
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * ビーコンPC動作アップデート
 *
 * @param   bpc		  ビーコンPCへのポインタ
 *
 * @retval  BOOL		TRUE:動作継続。　FALSE:動作終了(モデルを削除してください)
 */
//--------------------------------------------------------------
static BOOL BeaconPCFunc(UNION_BEACON_PC *bpc, MMDL *mmdl)
{
  BOOL next_seq = TRUE;
  BPC_FUNC func;
  
  switch(bpc->func_proc){
  case BPC_SUBPROC_INIT:
    func = BeaconPcFuncTbl[bpc->event_status].func_init;
    break;
  case BPC_SUBPROC_UPDATE:
    func = BeaconPcFuncTbl[bpc->event_status].func_update;
    break;
  case BPC_SUBPROC_EXIT:
    func = BeaconPcFuncTbl[bpc->event_status].func_exit;
    break;
  }

  if(func != NULL){
    next_seq = func(bpc, mmdl, &bpc->func_seq);
  }
  
  if(next_seq == TRUE){
    bpc->func_seq = 0;
    bpc->func_proc++;
    if(bpc->func_proc > BPC_SUBPROC_EXIT){
      if(bpc->event_status == BPC_EVENT_STATUS_LEAVE){
        return FALSE; //退出完了
      }
      bpc->func_proc = 0;
      bpc->event_status = bpc->next_event_status;
      bpc->next_event_status = BPC_EVENT_STATUS_NULL;
    }
  }
  
  return TRUE;
}
