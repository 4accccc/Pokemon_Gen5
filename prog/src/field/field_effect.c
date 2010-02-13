//======================================================================
/**
 * @file	field_effect.c
 * @brief	フィールドエフェクト
 * @authaor	kagaya
 * @date	2008.12.11
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "field/areadata.h"
#include "field/zonedata.h"

#include "fieldmap.h"
#include "field_effect.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
///FLDEFF_PROCEFF
typedef struct _TAG_FLDEFF_PROCEFF FLDEFF_PROCEFF;

//--------------------------------------------------------------
/// FLDEFF_CTRL
//--------------------------------------------------------------
struct _TAG_FLDEFF_CTRL
{
  int reg_max;
  HEAPID heapID;
  FIELDMAP_WORK *fieldMapWork;
  
  ARCHANDLE *arcHandle;

  FLDEFF_PROCEFF *proceff_tbl;
   
  u16 task_max;
  u8  season_id;
  u8  area_inout_switch:7;
  u8  has_season_diff:1;
  void *tcbsys_work;
  GFL_TCBSYS *tcbsys;
  FLDEFF_TASKSYS *tasksys;
};

//--------------------------------------------------------------
/// FLDEFF_PROCEFF
//--------------------------------------------------------------
struct _TAG_FLDEFF_PROCEFF
{
  u32 id;
  void *proc_work;
};

//--------------------------------------------------------------
/// FLDEFF_TASK
//--------------------------------------------------------------
struct _TAG_FLDEFF_TASK
{
  u32 flag;
  int add_param;
  const void *add_ptr;
  VecFx32 pos;
  u8 work[FLDEFF_TASK_WORK_SIZE];
  FLDEFF_TASK_HEADER header;
  
  FLDEFF_TASKSYS *tasksys;
  GFL_TCB *tcb;
};

//--------------------------------------------------------------
/// FLDEFF_TASKSYS
//--------------------------------------------------------------
struct _TAG_FLDEFF_TASKSYS
{
  u32 max;
  HEAPID heapID;
  FLDEFF_TASK *task_tbl;
  GFL_TCBSYS *tcbsys; //ユーザーから
};

//======================================================================
//  proto
//======================================================================
static void fectrl_InitProcEffect( FLDEFF_CTRL *fectrl );
static void fectrl_DeleteProcEffect( FLDEFF_CTRL *fectrl );
static FLDEFF_PROCEFF * fectrl_SearchProcEffect(
    FLDEFF_CTRL *fectrl, FLDEFF_PROCID id );
static const FLDEFF_PROCEFF_DATA * search_ProcEffectData(FLDEFF_PROCID);

static void fectrl_DeleteTaskParam( FLDEFF_CTRL *fectrl );
static void fectrl_UpdateTaskParam( FLDEFF_CTRL *fectrl );
static void fectrl_DrawTaskParam( FLDEFF_CTRL *fectrl );

static void fetask_ProcTCB( GFL_TCB *tcb, void *wk );

//======================================================================
//  フィールドエフェクト　コントロール
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドエフェクト コントロール　生成
 * @param fieldMapWork FIELDMAP_WORK
 * @param reg_max 登録するエフェクト総数
 * @param heapID エフェクトに使用するHEAPID
 * @retval FLDEFF_CTRL*
 */
//--------------------------------------------------------------
FLDEFF_CTRL * FLDEFF_CTRL_Create(
    FIELDMAP_WORK *fieldMapWork, u32 reg_max, HEAPID heapID )
{
  int i;
  FLDEFF_CTRL *fectrl;
  
  fectrl = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_CTRL) );
  fectrl->reg_max = reg_max;
  fectrl->heapID = heapID;
  fectrl->fieldMapWork = fieldMapWork;
  fectrl->arcHandle = GFL_ARC_OpenDataHandle( ARCID_FLDEFF, heapID );

  { //季節変化用データ取得
    u16 area_id = ZONEDATA_GetAreaID( FIELDMAP_GetZoneID( fieldMapWork ));
    GAMEDATA* gdata = GAMESYSTEM_GetGameData( FIELDMAP_GetGameSysWork( fieldMapWork ) );
    fectrl->season_id = GAMEDATA_GetSeasonID( gdata );
    fectrl->area_inout_switch= AREADATA_GetInnerOuterSwitch( FIELDMAP_GetAreaData( fieldMapWork ) );
    fectrl->has_season_diff = AREADATA_HasSeasonDiff( area_id );
  }
  fectrl_InitProcEffect( fectrl );
  return( fectrl );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　コントロール　削除
 * @param fectrl FLDEFF_CTRL
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_CTRL_Delete( FLDEFF_CTRL *fectrl )
{
  fectrl_DeleteTaskParam( fectrl );
  fectrl_DeleteProcEffect( fectrl );
  GFL_ARC_CloseDataHandle( fectrl->arcHandle );
  GFL_HEAP_FreeMemory( fectrl );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　コントロール　更新
 * @param fectrl FLDEFF_CTRL
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_CTRL_Update( FLDEFF_CTRL *fectrl )
{
  fectrl_UpdateTaskParam( fectrl );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　コントロール　描画
 * @param fectrl FLDEFF_CTRL
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_CTRL_Draw( FLDEFF_CTRL *fectrl )
{
  fectrl_DrawTaskParam( fectrl );
}

//======================================================================
//  フィールドエフェクト　コントロール　参照
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドエフェクト　コントロール　FIELDMAP_WORK取得
 * @param fectrl
 * @retval FIELDMAP_WORK
 */
//--------------------------------------------------------------
FIELDMAP_WORK * FLDEFF_CTRL_GetFieldMapWork( FLDEFF_CTRL *fectrl )
{
  return( fectrl->fieldMapWork );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　コントロール　FLD_G3DOBJ_CTRL取得
 * @param
 * @retval
 */
//--------------------------------------------------------------
FLD_G3DOBJ_CTRL * FLDEFF_CTRL_GetFldG3dOBJCtrl( FLDEFF_CTRL *fectrl )
{
  return( FIELDMAP_GetFldG3dOBJCtrl(fectrl->fieldMapWork) );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト コントロール アーカイブID ARCID_FLDEFFハンドル取得
 * @param fectrl
 * @retval ARCHANDLE ARCID_FLDEFFハンドル
 */
//--------------------------------------------------------------
ARCHANDLE * FLDEFF_CTRL_GetArcHandleEffect( FLDEFF_CTRL *fectrl )
{
  return( fectrl->arcHandle );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト コントロール 季節ID取得
 * @param fectrl
 * @retval 季節ID
 */
//--------------------------------------------------------------
u8 FLDEFF_CTRL_GetSeasonID( FLDEFF_CTRL *fectrl )
{
  return( fectrl->season_id );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト コントロール エリアIn/Outタイプ取得
 * @param fectrl
 * @retval 0  Inner 
 * @retval 1  Outer 
 */
//--------------------------------------------------------------
BOOL FLDEFF_CTRL_GetAreaInOutSwitch( FLDEFF_CTRL *fectrl )
{
  return( fectrl->area_inout_switch );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト コントロール 季節変化タイプ取得
 * @param fectrl
 * @retval  0 変化がない 
 * @retval  1 変化がある 
 */
//--------------------------------------------------------------
BOOL FLDEFF_CTRL_GetHasSeasonDiff( FLDEFF_CTRL *fectrl )
{
  return( fectrl->has_season_diff );
}

//======================================================================
//  フィールドエフェクト　エフェクトプロセス
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドエフェクト　エフェクトプロセス登録
 * @param fectrl FLDEFF_CTRL
 * @param id 登録するFLDEFF_PROCIDを纏めた配列
 * @param count 登録する数
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_CTRL_RegistEffect(
    FLDEFF_CTRL *fectrl, const FLDEFF_PROCID *id, u32 count )
{
  FLDEFF_PROCEFF *proc;
  const FLDEFF_PROCEFF_DATA *proc_data;
  
  GF_ASSERT( count );
  
  while( count ){
    #ifdef PM_DEBUG //二重登録チェック
    proc = fectrl_SearchProcEffect( fectrl, *id );
    GF_ASSERT( proc == NULL && "effect has registered" );
    #endif
    proc = fectrl_SearchProcEffect( fectrl, FLDEFF_PROCID_MAX );
    GF_ASSERT( proc != NULL && "effect max" ); //空き無し
    proc_data = search_ProcEffectData( *id );
    proc->id = *id;
    proc->proc_work = proc_data->proc_init( fectrl, fectrl->heapID );
    id++;
    count--;
  }
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　エフェクトプロセス削除
 * @param fectrl FLDEFF_CTRL
 * @param id 削除するFLDEFF_PROCID
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_CTRL_DeleteEffect( FLDEFF_CTRL *fectrl, FLDEFF_PROCID id )
{
  FLDEFF_PROCEFF *proc;
  const FLDEFF_PROCEFF_DATA *proc_data;
  
  proc = fectrl_SearchProcEffect( fectrl, id );
  
  if( proc == NULL ){ //未登録
    GF_ASSERT( 0 );
    return;
  }
  
  proc_data = search_ProcEffectData( id );
  proc_data->proc_del( fectrl, proc->proc_work );
  proc->id = FLDEFF_PROCID_MAX;
  proc->proc_work = NULL;
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　エフェクトプロセス登録チェック
 * @param fectrl FLDEFF_CTRL
 * @param id チェックするFLDEFF_PROCID
 * @retval BOOL TRUE=登録済み
 */
//--------------------------------------------------------------
BOOL FLDEFF_CTRL_CheckRegistEffect(
    const FLDEFF_CTRL *fectrl, FLDEFF_PROCID id )
{
  FLDEFF_PROCEFF *proc;
  //const cast変更問題無し
  proc = fectrl_SearchProcEffect( (FLDEFF_CTRL*)fectrl, id );
  if( proc == NULL ){
    return( FALSE );
  }
  return( TRUE );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　エフェクトプロセスが使用しているワークを取得
 * @param fectrl FLDEFF_CTRL
 * @param id FLDEFF_PROCID
 * @retval
 */
//--------------------------------------------------------------
void * FLDEFF_CTRL_GetEffectWork( FLDEFF_CTRL *fectrl, FLDEFF_PROCID id )
{
  FLDEFF_PROCEFF *proc;
  proc = fectrl_SearchProcEffect( (FLDEFF_CTRL*)fectrl, id );
  GF_ASSERT( proc != NULL && "effect not regist\n" );
  return( proc->proc_work );
}

//--------------------------------------------------------------
/**
 * エフェクトプロセス　初期化
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void fectrl_InitProcEffect( FLDEFF_CTRL *fectrl )
{
  int i;
  FLDEFF_PROCEFF *tbl;
  
  tbl = GFL_HEAP_AllocClearMemory(
      fectrl->heapID, sizeof(FLDEFF_PROCEFF)*fectrl->reg_max );
  fectrl->proceff_tbl = tbl;
  
  for( i = 0; i < fectrl->reg_max; i++, tbl++ ){
    tbl->id = FLDEFF_PROCID_MAX;
  }
}

//--------------------------------------------------------------
/**
 * エフェクトプロセス　削除
 * @param fectrl FLDEFF_CTRL*
 * @retval nothing
 */
//--------------------------------------------------------------
static void fectrl_DeleteProcEffect( FLDEFF_CTRL *fectrl )
{
  int i;
  const FLDEFF_PROCEFF_DATA *proc_data;
  FLDEFF_PROCEFF *tbl = fectrl->proceff_tbl;
  
  for( i = 0; i < fectrl->reg_max; i++, tbl++ ){
    if( tbl->id != FLDEFF_PROCID_MAX ){
      proc_data = search_ProcEffectData( tbl->id );
      proc_data->proc_del( fectrl, tbl->proc_work );
      tbl->id = FLDEFF_PROCID_MAX;
      tbl->proc_work = NULL;
    }
  }
  
  GFL_HEAP_FreeMemory( fectrl->proceff_tbl );
  fectrl->proceff_tbl = NULL;
}

//--------------------------------------------------------------
/**
 * エフェクトプロセス　検索
 * @param  fectrl FLDEFF_CTRL*
 * @param  id 検索するID
 * @retval FLDEFF_PROCEFF NULL=一致なし
 */
//--------------------------------------------------------------
static FLDEFF_PROCEFF * fectrl_SearchProcEffect(
    FLDEFF_CTRL *fectrl, FLDEFF_PROCID id )
{
  u32 max = fectrl->reg_max;
  FLDEFF_PROCEFF *proc = fectrl->proceff_tbl;
  
  while( max ){
    if( proc->id == id ){
      return( proc );
    }
    max--;
    proc++;
  }
  
  return( NULL );
}

//--------------------------------------------------------------
/**
 * DATA_FLDEFF_ProcEffectDataTbl検索
 * @param id FLDEFF_PROCID
 * @retval FLDEFF_PROCEFF_DATA
 */
//--------------------------------------------------------------
static const FLDEFF_PROCEFF_DATA * search_ProcEffectData( FLDEFF_PROCID id )
{
  int i = 0;
  const FLDEFF_PROCEFF_DATA *data;

  data = DATA_FLDEFF_ProcEffectDataTbl;
  
  do{
    if( data->id == id ){
      return( data );
    }
    data++;
    i++;
  }while( i < FLDEFF_PROCID_MAX );
  
  GF_ASSERT( 0 && "unknown id" );
  return( data );
}

//======================================================================
//  フィールドエフェクト　コントロール　タスク設定
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドエフェクト　コントロール　タスク設定
 * @param fectrl FLDEFF_CTRL
 * @param max タスク総数
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_CTRL_SetTaskParam( FLDEFF_CTRL *fectrl, u32 max )
{
  GF_ASSERT( max );
  GF_ASSERT( fectrl->tasksys == NULL );
  
  fectrl->task_max = max;
  fectrl->tcbsys_work = GFL_HEAP_AllocMemory(
      fectrl->heapID, GFL_TCB_CalcSystemWorkSize(max) );
  fectrl->tcbsys = GFL_TCB_Init( max, fectrl->tcbsys_work );
  fectrl->tasksys = FLDEFF_TASKSYS_Init(
      fectrl->heapID, fectrl->tcbsys, max );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　コントロール　タスク追加
 * @param fectrl FLDEFF_CTRL
 * @param head FLDEFF_TASK_HEADER
 * @param pos タスクに指定する座標 NULL=指定無し
 * @param add_param タスクに渡す任意のパラメタ
 * @param add_ptr タスクに渡す任意のポインタ
 * @param pri タスク動作時のTCBプライオリティ
 * @retval FLDEFF_TASK*
 * @note 追加時にヘッダー指定の初期化関数が呼ばれる。
 * add_paramはFLDEFF_TASK_GetAddParam()で取得。
 * add_ptrはFLDEFF_TASK_GetAddPointer()で取得。
 */
//--------------------------------------------------------------
FLDEFF_TASK * FLDEFF_CTRL_AddTask(
    FLDEFF_CTRL *fectrl, const FLDEFF_TASK_HEADER *head,
    const VecFx32 *pos, int add_param, const void *add_ptr, int pri )
{
  FLDEFF_TASK *task;
  GF_ASSERT( fectrl->tasksys );
  task = FLDEFF_TASKSYS_AddTask(
      fectrl->tasksys, head, pos, add_param, add_ptr, pri );
  return( task );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　コントロール　タスク設定　削除
 * @param fectrl  FLDEFF_CTRL
 * @retval nothing
 */
//--------------------------------------------------------------
static void fectrl_DeleteTaskParam( FLDEFF_CTRL *fectrl )
{
  if( fectrl->tasksys ){
    FLDEFF_TASKSYS_Delete( fectrl->tasksys );
    GFL_TCB_Exit( fectrl->tcbsys );
    GFL_HEAP_FreeMemory( fectrl->tcbsys_work );
  }
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　コントロール　タスク設定　更新
 * @param fectrl  FLDEFF_CTRL
 * @retval  nothing
 */
//--------------------------------------------------------------
static void fectrl_UpdateTaskParam( FLDEFF_CTRL *fectrl )
{
  if( fectrl->tcbsys ){
    GFL_TCB_Main( fectrl->tcbsys );
  }
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　コントロール　タスク設定　描画
 * @param fectrl  FLDEFF_CTRL
 * @retval nothing
 */
//--------------------------------------------------------------
static void fectrl_DrawTaskParam( FLDEFF_CTRL *fectrl )
{
  if( fectrl->tasksys ){
    FLDEFF_TASKSYS_Draw( fectrl->tasksys );
  }
}

//======================================================================
//  フィールドエフェクト　タスクシステム
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスクシステム初期化
 * @param heapID HEAPID
 * @param max タスク最大数
 * @retval FLDEFF_TASKSYS
 */
//--------------------------------------------------------------
FLDEFF_TASKSYS * FLDEFF_TASKSYS_Init(
    HEAPID heapID, GFL_TCBSYS *tcbsys, int max )
{
  FLDEFF_TASKSYS *tasksys;
   
  tasksys = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_TASKSYS) );
  tasksys->max = max;
  tasksys->heapID = heapID;
  tasksys->task_tbl = GFL_HEAP_AllocClearMemory(
      heapID, sizeof(FLDEFF_TASK)*max );
  tasksys->tcbsys = tcbsys;
  return( tasksys );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスクシステム削除
 * @param FLDEFF_TASKSYS
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_TASKSYS_Delete( FLDEFF_TASKSYS *tasksys )
{
  int i;
  
  for( i = 0; i < tasksys->max; i++ ){
    if( tasksys->task_tbl[i].flag ){
      FLDEFF_TASK_CallDelete( &tasksys->task_tbl[i] );
    }
  }
  
  GFL_HEAP_FreeMemory( tasksys->task_tbl );
  GFL_HEAP_FreeMemory( tasksys );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスクシステム　描画
 * @param FLDEFF_TASKSYS
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_TASKSYS_Draw( FLDEFF_TASKSYS *tasksys )
{
  int i;
  for( i = 0; i < tasksys->max; i++ ){
    if( tasksys->task_tbl[i].flag ){
      FLDEFF_TASK_CallDraw( &tasksys->task_tbl[i] );
    }
  }
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスクシステム　追加
 * @param FLDEFF_TASKSYS
 * @param head FLDEFF_TASK_HEADER
 * @param pos タスクに指定する座標 NULL=指定無し
 * @param add_param タスクに渡す任意のパラメタ
 * @param add_ptr タスクに渡す任意のポインタ
 * @param pri タスク動作時のTCBプライオリティ
 * @retval FLDEFF_TASK*
 * @note 追加時にヘッダー指定の初期化関数が呼ばれる。
 * add_paramはFLDEFF_TASK_GetAddParam()で取得。
 * add_ptrはFLDEFF_TASK_GetAddPointer()で取得。
 */
//--------------------------------------------------------------
FLDEFF_TASK * FLDEFF_TASKSYS_AddTask(
    FLDEFF_TASKSYS *tasksys, const FLDEFF_TASK_HEADER *head,
    const VecFx32 *pos, int add_param, const void *add_ptr, int pri )
{
  int i = 0;
  FLDEFF_TASK *task = tasksys->task_tbl;
  
  GF_ASSERT( head->work_size < FLDEFF_TASK_WORK_SIZE );
  GF_ASSERT( tasksys->max );

  do{
    if( task->flag == 0 ){
      MI_CpuClear8( task, sizeof(FLDEFF_TASK) );
      task->flag = TRUE;
      task->header = *head;
      task->add_param = add_param;
      task->add_ptr = add_ptr;

      if( pos != NULL ){
        task->pos = *pos;
      }

      task->tasksys = tasksys;
      task->tcb = GFL_TCB_AddTask(
          tasksys->tcbsys, fetask_ProcTCB, task, pri );
      break;
    }
    i++;
    task++;
  }while( i < tasksys->max );
  
  GF_ASSERT( i < tasksys->max && "ERROR FLDEFF_TASKSYS MAX\n" );
  FLDEFF_TASK_CallInit( task );
  return( task );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスクシステム　TCB動作関数
 * @param tcb GFL_TCB
 * @param wk tcb work
 * @retval nothing
 */
//--------------------------------------------------------------
static void fetask_ProcTCB( GFL_TCB *tcb, void *wk )
{
  FLDEFF_TASK *task = wk;
  FLDEFF_TASK_CallUpdate( task );
}

//======================================================================
//  フィールドエフェクト　タスク
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスク　初期化呼び出し
 * @param task FLDEFF_TASK 
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_TASK_CallInit( FLDEFF_TASK *task )
{
  GF_ASSERT( task->flag );
  GF_ASSERT( task != NULL );
  task->header.proc_init( task, task->work );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスク　削除呼び出し
 * @param task FLDEFF_TASK
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_TASK_CallDelete( FLDEFF_TASK *task )
{
  GF_ASSERT( task->flag );
  GF_ASSERT( task != NULL );
  task->header.proc_delete( task, task->work );
  GFL_TCB_DeleteTask( task->tcb );
  task->flag = 0;
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスク　更新呼び出し
 * @param task FLDEFF_TASK
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_TASK_CallUpdate( FLDEFF_TASK *task )
{
  GF_ASSERT( task->flag );
  GF_ASSERT( task != NULL );
  task->header.proc_update( task, task->work );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスク　描画呼び出し
 * @param task FLDEFF_TASK
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_TASK_CallDraw( FLDEFF_TASK *task )
{
  GF_ASSERT( task->flag );
  GF_ASSERT( task != NULL );
  GF_ASSERT( task->work );
  task->header.proc_draw( task, task->work );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスク　追加時の引数パラメタ取得
 * @param task FLDEFF_TASK
 * @retval u32 FLDEFF_TASKSYS_AddTask()引数add_param
 */
//--------------------------------------------------------------
u32 FLDEFF_TASK_GetAddParam( const FLDEFF_TASK *task )
{
  return( task->add_param );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスク　追加時の引数ポインタ取得
 * @param task  FLDEFF_TASK
 * @retval void* FLDEFF_TASKSYS_AddTask()引数add_ptr
 */
//--------------------------------------------------------------
const void * FLDEFF_TASK_GetAddPointer( const FLDEFF_TASK *task )
{
  return( task->add_ptr );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスク　座標取得
 * @param task FLDEFF_TASK
 * @param pos 座標格納先
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_TASK_GetPos( const FLDEFF_TASK *task, VecFx32 *pos )
{
  *pos = task->pos;
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスク　座標セット
 * @param task FLDEFF_TASK
 * @param pos 設定座標
 * @retval nothing
 */
//--------------------------------------------------------------
void FLDEFF_TASK_SetPos( FLDEFF_TASK *task, const VecFx32 *pos )
{
  task->pos = *pos;
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスク　ワーク取得
 * @param task FLDEFF_TASK
 * @retval void* taskが使用しているワーク*
 */
//--------------------------------------------------------------
void * FLDEFF_TASK_GetWork( FLDEFF_TASK *task )
{
  return( task->work );
}

//--------------------------------------------------------------
/**
 * フィールドエフェクト　タスク　HEAPID取得
 * @param task FLDEFF_TASK
 * @retval HEAPID
 */
//--------------------------------------------------------------
HEAPID FLDEFF_TASK_GetHeapID( const FLDEFF_TASK *task )
{
  return( task->tasksys->heapID );
}

//======================================================================
//  data 仮 旧field_effect_data.c
//======================================================================
#include "fldeff_shadow.h"
#include "fldeff_kemuri.h"
#include "fldeff_grass.h"
#include "fldeff_namipoke.h"
#include "fldeff_gyoe.h"
#include "fldeff_footmark.h"
#include "fldeff_reflect.h"
#include "fldeff_ripple.h"
#include "fldeff_splash.h"
#include "fldeff_encount.h"
#include "fldeff_iwakudaki.h"
#include "fldeff_d06denki.h"
#include "fldeff_iaigiri.h"
#include "fldeff_fishing.h"
#include "fldeff_btrain.h"

FLDEFF_PROCEFF_DATA DATA_FLDEFF_ProcEffectDataTbl[FLDEFF_PROCID_MAX+1] =
{
  {FLDEFF_PROCID_SHADOW,FLDEFF_SHADOW_Init,FLDEFF_SHADOW_Delete},
  {FLDEFF_PROCID_KEMURI,FLDEFF_KEMURI_Init,FLDEFF_KEMURI_Delete},
  {FLDEFF_PROCID_GRASS,FLDEFF_GRASS_Init,FLDEFF_GRASS_Delete},
  {FLDEFF_PROCID_NAMIPOKE,FLDEFF_NAMIPOKE_Init,FLDEFF_NAMIPOKE_Delete},
  {FLDEFF_PROCID_GYOE,FLDEFF_GYOE_Init,FLDEFF_GYOE_Delete},
  {FLDEFF_PROCID_FOOTMARK,FLDEFF_FOOTMARK_Init,FLDEFF_FOOTMARK_Delete},
  {FLDEFF_PROCID_REFLECT,FLDEFF_REFLECT_Init,FLDEFF_REFLECT_Delete},
  {FLDEFF_PROCID_RIPPLE,FLDEFF_RIPPLE_Init,FLDEFF_RIPPLE_Delete},
  {FLDEFF_PROCID_SPLASH,FLDEFF_SPLASH_Init,FLDEFF_SPLASH_Delete},
  {FLDEFF_PROCID_NAMIPOKE_EFFECT,FLDEFF_NAMIPOKE_EFFECT_Init,FLDEFF_NAMIPOKE_EFFECT_Delete},
  {FLDEFF_PROCID_IWAKUDAKI,FLDEFF_IWAKUDAKI_Init,FLDEFF_IWAKUDAKI_Delete},
  {FLDEFF_PROCID_D06DENKI,FLDEFF_D06DENKI_Init,FLDEFF_D06DENKI_Delete},
  {FLDEFF_PROCID_IAIGIRI,FLDEFF_IAIGIRI_Init,FLDEFF_IAIGIRI_Delete},
  {FLDEFF_PROCID_FISHING_LURE,FLDEFF_FISHING_LURE_Init,FLDEFF_FISHING_LURE_Delete},
  {FLDEFF_PROCID_BTRAIN,FLDEFF_BTRAIN_Init,FLDEFF_BTRAIN_Delete},
  
  //ここからエフェクトエンカウント用
  {FLDEFF_PROCID_ENC_SGRASS,FLDEFF_ENCOUNT_SGrassInit,FLDEFF_ENCOUNT_Delete},
  {FLDEFF_PROCID_ENC_LGRASS,FLDEFF_ENCOUNT_LGrassInit,FLDEFF_ENCOUNT_Delete},
  {FLDEFF_PROCID_ENC_CAVE,FLDEFF_ENCOUNT_CaveInit,FLDEFF_ENCOUNT_Delete},
  {FLDEFF_PROCID_ENC_WATER,FLDEFF_ENCOUNT_WaterInit,FLDEFF_ENCOUNT_Delete},
  {FLDEFF_PROCID_ENC_SEA,FLDEFF_ENCOUNT_SeaInit,FLDEFF_ENCOUNT_Delete},
  {FLDEFF_PROCID_ENC_BRIDGE,FLDEFF_ENCOUNT_BridgeInit,FLDEFF_ENCOUNT_Delete},
  {FLDEFF_PROCID_MAX,NULL,NULL}, ///<終端
};

//--------------------------------------------------------------
/// フィールドエフェクト　地上用エフェクト登録テーブル
//--------------------------------------------------------------
const FLDEFF_PROCID DATA_FLDEFF_RegistEffectGroundTbl[] =
{
  FLDEFF_PROCID_SHADOW,
  FLDEFF_PROCID_KEMURI,
  FLDEFF_PROCID_GRASS,
  FLDEFF_PROCID_NAMIPOKE,
  FLDEFF_PROCID_GYOE,
  FLDEFF_PROCID_FOOTMARK,
  FLDEFF_PROCID_REFLECT,
  FLDEFF_PROCID_RIPPLE,
  FLDEFF_PROCID_SPLASH,
  
  //ここからエフェクトエンカウント用
  FLDEFF_PROCID_ENC_SGRASS, ///<エフェクトエンカウント用　短い草
  FLDEFF_PROCID_ENC_LGRASS, ///<エフェクトエンカウント用　長い草
  FLDEFF_PROCID_ENC_CAVE,   ///<エフェクトエンカウント用　洞窟
  FLDEFF_PROCID_ENC_WATER,  ///<エフェクトエンカウント用　淡水
  FLDEFF_PROCID_ENC_SEA,    ///<エフェクトエンカウント用　海
  FLDEFF_PROCID_ENC_BRIDGE, ///<エフェクトエンカウント用　橋
};

const u32 DATA_FLDEFF_RegistEffectGroundTblNum = 
  NELEMS(DATA_FLDEFF_RegistEffectGroundTbl);
