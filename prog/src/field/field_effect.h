//======================================================================
/**
 * @file	field_effect.h
 * @brief	フィールドエフェクト
 * @authaor	kagaya
 * @date	2008.12.11
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "system/gfl_use.h"

#include "field/fieldmap_proc.h"

#include "arc/fieldmap/fldeff.naix"

//======================================================================
//  define
//======================================================================
#define FLDEFF_TASK_WORK_SIZE (160) ///<FLDEFF_TASKで使用するワークサイズ
#define FLDEFF_TASK_MAX (94) //64(最低限必要)+30(ギミック)

//--------------------------------------------------------------
/// FLDEFF_PROCID
//--------------------------------------------------------------
typedef enum
{
  FLDEFF_PROCID_SHADOW = 0, ///<影
  FLDEFF_PROCID_KEMURI, ///<土煙エフェクト
  FLDEFF_PROCID_GRASS, ///<草エフェクト
  FLDEFF_PROCID_NAMIPOKE, ///<波乗りポケモン
  FLDEFF_PROCID_GYOE, ///<びっくりマーク
  FLDEFF_PROCID_FOOTMARK, ///<足跡
  FLDEFF_PROCID_REFLECT, ///<映り込み
  FLDEFF_PROCID_RIPPLE, ///<水波紋
  FLDEFF_PROCID_SPLASH, ///<水飛沫
  FLDEFF_PROCID_NAMIPOKE_EFFECT, ///<波乗りポケモンエフェクト
  FLDEFF_PROCID_IWAKUDAKI, ///<岩砕き
  FLDEFF_PROCID_D06DENKI, ///<D06電気洞窟
  FLDEFF_PROCID_IAIGIRI, ///<居合い切り
  FLDEFF_PROCID_FISHING_LURE, ///<釣りイベントのルアー
  FLDEFF_PROCID_BTRAIN, ///<バトルトレイン
  
  //ここからエフェクトエンカウント用
  FLDEFF_PROCID_ENC_SGRASS, ///<エフェクトエンカウント用　短い草
  FLDEFF_PROCID_ENC_LGRASS, ///<エフェクトエンカウント用　長い草
  FLDEFF_PROCID_ENC_CAVE,   ///<エフェクトエンカウント用　洞窟
  FLDEFF_PROCID_ENC_WATER,  ///<エフェクトエンカウント用　淡水
  FLDEFF_PROCID_ENC_SEA,    ///<エフェクトエンカウント用　海
  FLDEFF_PROCID_ENC_BRIDGE, ///<エフェクトエンカウント用　橋
  
  FLDEFF_PROCID_MAX, ///<最大
}FLDEFF_PROCID;

//======================================================================
//  struct
//======================================================================
///FLDEFF_CTRL
typedef struct _TAG_FLDEFF_CTRL FLDEFF_CTRL;
///FLDEFF_TASK
typedef struct _TAG_FLDEFF_TASK FLDEFF_TASK;
///FLDEFF_TASKSYS
typedef struct _TAG_FLDEFF_TASKSYS FLDEFF_TASKSYS;

///FLDEFF_TASK 初期化関数
typedef void (*FLDEFF_TASK_INIT)( FLDEFF_TASK*, void *wk );
///FLDEFF_TASK 削除
typedef void (*FLDEFF_TASK_DELETE)( FLDEFF_TASK*, void *wk );
///FLDEFF_TASK 更新
typedef void (*FLDEFF_TASK_UPDATE)( FLDEFF_TASK*, void *wk );
///FLDEFF_TASK 描画
typedef void (*FLDEFF_TASK_DRAW)( FLDEFF_TASK*, void *wk );

//--------------------------------------------------------------
///	エフェクトサブプロセス初期化関数型定義。
/// 引数：FLDEFF_CTRL* 戻り値：関数で使用するワーク *
//--------------------------------------------------------------
typedef void * (*FLDEFF_PROCEFF_INIT)( FLDEFF_CTRL *, HEAPID heapID );

//--------------------------------------------------------------
///	エフェクトサブプロセス削除関数型定義。
/// 引数：void * = FLDEFF_PROCEFF_INITの戻り値
//--------------------------------------------------------------
typedef void (*FLDEFF_PROCEFF_DELETE)( FLDEFF_CTRL*, void * );

//--------------------------------------------------------------
/// FLDEFF_PROCEFF_DATA
//--------------------------------------------------------------
typedef struct
{
  u32 id; ///<proc_init,proc_delを呼び出す為の識別ID
  FLDEFF_PROCEFF_INIT proc_init; ///<idを指定した際に呼ばれる初期化関数
	FLDEFF_PROCEFF_DELETE proc_del; ///<idを指定した際に呼ばれる削除関数
}FLDEFF_PROCEFF_DATA;

//--------------------------------------------------------------
/// FLDEFF_TASK_HEADER
//--------------------------------------------------------------
typedef struct
{
  u32 work_size; ///<タスクで使用するワークのサイズ
  FLDEFF_TASK_INIT proc_init; ///<タスク追加時に呼び出される初期化関数
  FLDEFF_TASK_DELETE proc_delete; ///<タスク削除時に呼び出される削除関数
  FLDEFF_TASK_UPDATE proc_update; ///<更新時に呼び出される更新関数
  FLDEFF_TASK_DRAW proc_draw; ///<描画時に呼び出される描画関数
}FLDEFF_TASK_HEADER;

//======================================================================
//  extern
//======================================================================
//FLDEFF_CTRL
extern FLDEFF_CTRL * FLDEFF_CTRL_Create(
    FIELDMAP_WORK *fwork, u32 reg_max, HEAPID heapID );
extern void FLDEFF_CTRL_Delete( FLDEFF_CTRL *fectrl );
extern void FLDEFF_CTRL_Update( FLDEFF_CTRL *fectrl );
extern void FLDEFF_CTRL_Draw( FLDEFF_CTRL *fectrl );

//FLDEFF_CTRL 参照
extern FIELDMAP_WORK * FLDEFF_CTRL_GetFieldMapWork( FLDEFF_CTRL *fectrl );
extern ARCHANDLE * FLDEFF_CTRL_GetArcHandleEffect( FLDEFF_CTRL *fectrl );
extern u8 FLDEFF_CTRL_GetSeasonID( FLDEFF_CTRL *fectrl );
extern BOOL FLDEFF_CTRL_GetAreaInOutSwitch( FLDEFF_CTRL *fectrl );
extern BOOL FLDEFF_CTRL_GetHasSeasonDiff( FLDEFF_CTRL *fectrl );

//FLDEFF_CTRL effect process
extern void FLDEFF_CTRL_RegistEffect(
    FLDEFF_CTRL *fectrl, const FLDEFF_PROCID *id, u32 count );
extern void FLDEFF_CTRL_DeleteEffect(
     FLDEFF_CTRL *fectrl, FLDEFF_PROCID id );
extern BOOL FLDEFF_CTRL_CheckRegistEffect(
    const FLDEFF_CTRL *fectrl, FLDEFF_PROCID id );
extern void * FLDEFF_CTRL_GetEffectWork(
    FLDEFF_CTRL *fectrl, FLDEFF_PROCID id );

//FLDEFF_CTRL task
extern FLDEFF_TASK * FLDEFF_CTRL_AddTask(
    FLDEFF_CTRL *fectrl, const FLDEFF_TASK_HEADER *head,
    const VecFx32 *pos, int add_param, const void *add_ptr, int pri );
extern void FLDEFF_CTRL_SetTaskParam( FLDEFF_CTRL *fectrl, u32 max );

//FLDEFF_TASKSYS
extern FLDEFF_TASKSYS * FLDEFF_TASKSYS_Init(
    HEAPID heapID, GFL_TCBSYS *tcbsys, int max );
extern void FLDEFF_TASKSYS_Delete( FLDEFF_TASKSYS *tasksys );
extern void FLDEFF_TASKSYS_Draw( FLDEFF_TASKSYS *tasksys );
extern FLDEFF_TASK * FLDEFF_TASKSYS_AddTask(
    FLDEFF_TASKSYS *tasksys, const FLDEFF_TASK_HEADER *head,
    const VecFx32 *pos, int add_param, const void *add_ptr, int pri );

//FLDEFF_TASK
extern void FLDEFF_TASK_CallInit( FLDEFF_TASK *task );
extern void FLDEFF_TASK_CallDelete( FLDEFF_TASK *task );
extern void FLDEFF_TASK_CallUpdate( FLDEFF_TASK *task );
extern void FLDEFF_TASK_CallDraw( FLDEFF_TASK *task );
extern u32 FLDEFF_TASK_GetAddParam( const FLDEFF_TASK *task );
extern const void * FLDEFF_TASK_GetAddPointer( const FLDEFF_TASK *task );
extern void FLDEFF_TASK_GetPos( const FLDEFF_TASK *task, VecFx32 *pos );
extern void FLDEFF_TASK_SetPos( FLDEFF_TASK *task, const VecFx32 *pos );
extern void * FLDEFF_TASK_GetWork( FLDEFF_TASK *task );
extern HEAPID FLDEFF_TASK_GetHeapID( const FLDEFF_TASK *task );

//kari
extern FLDEFF_PROCEFF_DATA DATA_FLDEFF_ProcEffectDataTbl[FLDEFF_PROCID_MAX+1];

extern const FLDEFF_PROCID DATA_FLDEFF_RegistEffectGroundTbl[];
extern const u32 DATA_FLDEFF_RegistEffectGroundTblNum;
