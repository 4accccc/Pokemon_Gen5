//=============================================================================
/**
 * @file	プロセス管理ヘッダ
 * @brief
 * @date	2005.07.25
 */
//=============================================================================

#ifndef	__PROCSYS_H__
#define	__PROCSYS_H__

//===========================================================================
//===========================================================================
//------------------------------------------------------------------
/**
 * @brief	プロセス動作関数の結果
 */
//------------------------------------------------------------------
typedef enum {
	GFL_PROC_RES_CONTINUE = 0,		///<動作継続中
	GFL_PROC_RES_FINISH				///<動作終了
}GFL_PROC_RESULT;

//------------------------------------------------------------------
/**
 * @brief	プロセス構造体への不完全型
 *
 * 構造体の実際の定義はprocsys.cに封じられている
 */
//------------------------------------------------------------------
typedef struct _GFL_PROC GFL_PROC;

//------------------------------------------------------------------
/**
 */
//------------------------------------------------------------------
typedef struct _GFL_PROCSYS GFL_PROCSYS;

//------------------------------------------------------------------
/**
 * @brief	プロセス動作関数の型定義
 *
 * フィールド、戦闘、メニューなどの動作関数は下記の形式で作成する
 */
//------------------------------------------------------------------
typedef GFL_PROC_RESULT (*GFL_PROC_FUNC)(GFL_PROC *, int *, void *, void *);

//------------------------------------------------------------------
/**
 * @brief	プロセス定義データ
 *
 * プロセスの初期化・メイン・終了関数を登録したデータ
 * オーバーレイID指定は今のところダミー
 */
//------------------------------------------------------------------
typedef struct {
	GFL_PROC_FUNC init_func;
	GFL_PROC_FUNC main_func;
	GFL_PROC_FUNC end_func;
}GFL_PROC_DATA;

#define	NO_OVERLAY_ID	(0xffffffff)
//===========================================================================
//===========================================================================
extern GFL_PROCSYS * GFL_PROC_SysInit(u32 heap_id);
extern void GFL_PROC_SysMain(GFL_PROCSYS * psys);
extern void GFL_PROC_SysExit(GFL_PROCSYS * psys);

extern void GFL_PROC_SysCallNextProc(GFL_PROCSYS * psys, GFL_PROC * proc);
extern void GFL_PROC_SysCallProc(GFL_PROCSYS * psys, const GFL_PROC_DATA * procdata, void * param);
extern void GFL_PROC_SysSetNextProc(GFL_PROCSYS * psys, const GFL_PROC_DATA * procdata, void * param);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern GFL_PROC * GFL_PROC_Create(const GFL_PROC_DATA * data, void * parent_work, const u32 heap_id);
extern GFL_PROC * GFL_PROC_CreateChild(	GFL_PROC * proc,
								const GFL_PROC_DATA * data,
								void * parent_work,
								const u32 heap_id);

extern void GFL_PROC_Delete(GFL_PROC * proc);

extern BOOL GFL_PROC_Main(GFL_PROC * proc);

//------------------------------------------------------------------
//------------------------------------------------------------------
extern void * GFL_PROC_AllocWork(GFL_PROC * proc, unsigned int size, u32 heap_id);
extern void * GFL_PROC_GetWork(GFL_PROC * proc);
extern void GFL_PROC_FreeWork(GFL_PROC * proc);
//extern void PROC_SetPause(GFL_PROC * proc, BOOL pause_flag);



#endif /* __PROCSYS_H__ */
