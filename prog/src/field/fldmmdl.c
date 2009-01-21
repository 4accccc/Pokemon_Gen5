//******************************************************************************
/**
 * 
 * @file	fldmmdl.c
 * @brief	フィールド動作モデル
 * @author	kagaya
 * @data	05.07.13
 *
 */
//******************************************************************************
#include "fldmmdl.h"

//==============================================================================
//	define
//==============================================================================
//--------------------------------------------------------------
//	debug
//--------------------------------------------------------------
#if 0
#ifdef PM_DEBUG
#define DEBUG_OBJID_SAM_CHECK		//定義でOBJ ID同一チェック
#define DEBUG_PRINT_FLDMMDL_COUNT	//定義でOBJ存在数出力

#ifdef DEBUG_ONLY_FOR_kagaya		//kagaya専用
//#define DEBUG_CHECK_NEWOBJ		//新規OBJチェック
#define DEBUG_FLDMMDL_PRINTF			//OS_Printf()有効
#endif

#ifdef DEBUG_CHECK_NEWOBJ
#define DEBUG_NEWOBJCODE_0 ROTOMWALL
#define DEBUG_NEWOBJCODE_1 ROTOMWALL
#endif

#endif //PM_DEBUG
#endif

//--------------------------------------------------------------
///	エイリアスシンボル
//--------------------------------------------------------------
enum
{
	RET_ALIES_NOT = 0,	//エイリアスではない
	RET_ALIES_EXIST,	//エイリアスとして既に存在している
	RET_ALIES_CHANGE,	//エイリアス変更が必要である
};

//==============================================================================
//	struct
//==============================================================================
//--------------------------------------------------------------
///	FLDMMDLSYS構造体
//--------------------------------------------------------------
typedef struct _TAG_FLDMMDLSYS
{
	u32 status_bit;					///<ステータスビット
	int fmmdl_max;					///<FLDMMDL最大数
	int fmmdl_count;				///<フィールド動作モデル現在数
	int tcb_pri;					///<TCBプライオリティ
	int blact_idx;					///<ビルボードアクターインデックス
	ARCHANDLE *archandle;			///<アーカイブハンドル
	FLDMMDL *fmmdl_work;			///<FLDMMDLワーク *
	
#if 0
	FLDMMDL_BLACT_CONT blact_cont;			///<FLDMMDL_BLACT_CONT
	FLDMMDL_RENDER_CONT_PTR render_cont;	///<FLDMMDL_RENDER_CONT
	FIELDSYS_WORK *fieldsys;				///<FIELDSYS_WORK *
#endif
	
	HEAPID heapID;
	void *pTCBSysWork;
	GFL_TCBSYS *pTCBSys;
	FIELD_MAIN_WORK *pFldMainWork;
	
	const FLDMAPPER *pG3DMapper;

	FLDMMDL_BLACTCONT *pBlActCont;
}FLDMMDLSYS;

#define FLDMMDLSYS_SIZE (sizeof(FLDMMDLSYS)) ///<FLDMMDLSYSサイズ

//--------------------------------------------------------------
///	FLDMMDL構造体
//--------------------------------------------------------------
typedef struct _TAG_FLDMMDL
{
	u32 status_bit;				///<ステータスビット
	u32 move_bit;				///<動作ビット
	u32 obj_id;					///<OBJ ID
	u32 zone_id;				///<ゾーン ID
	u32 obj_code;				///<OBJコード
	u32 move_code;				///<動作コード
	u32 event_type;				///<イベントタイプ
	u32 event_flag;				///<イベントフラグ
	u32 event_id;				///<イベントID
	int dir_head;				///<FLDMMDL_H指定方向
	int dir_disp;				///<現在向いている方向
	int dir_move;				///<現在動いている方向
	int dir_disp_old;			///<過去の動いていた方向
	int dir_move_old;			///<過去の動いていた方向
	int param0;					///<ヘッダ指定パラメタ
	int param1;					///<ヘッダ指定パラメタ
	int param2;					///<ヘッダ指定パラメタ
	int move_limit_x;			///<X方向移動制限
	int move_limit_z;			///<Z方向移動制限
	int gx_init;				///<初期グリッドX
	int gy_init;				///<初期グリッドY
	int gz_init;				///<初期グリッドZ
	int gx_old;					///<過去グリッドX
	int gy_old;					///<過去グリッドY
	int gz_old;					///<過去グリッドZ
	int gx_now;					///<現在グリッドX
	int gy_now;					///<現在グリッドY
	int gz_now;					///<現在グリッドZ
	VecFx32 vec_pos_now;		///<現在実数座標
	VecFx32 vec_draw_offs;		///<表示座標オフセット
	VecFx32 vec_draw_offs_outside;		///<外部指定表示座標オフセット
	VecFx32 vec_attr_offs;				///<アトリビュートによる座標オフセット
	u32 draw_status;					///<描画ステータス
	int acmd_code;						///<アニメーションコマンドコード
	int acmd_seq;						///<アニメーションコマンドシーケンス
	u16 now_attr;						///<現在のマップアトリビュート
	u16 old_attr;						///<過去のマップアトリビュート
	
	GFL_TCB * tcb;						///<動作関数TCB*
	const FLDMMDLSYS *fmmdlsys;			///<FLDMMDLSYSへの *
	
	FLDMMDL_MOVE_PROC_INIT move_init_proc;	///<初期化関数
	FLDMMDL_MOVE_PROC move_proc;			///<動作関数
	FLDMMDL_MOVE_PROC_DEL move_delete_proc;	///<削除関数
	FLDMMDL_DRAW_PROC_INIT draw_init_proc;	///<描画初期化関数
	FLDMMDL_DRAW_PROC draw_proc;			///<描画関数
	FLDMMDL_DRAW_PROC_DEL draw_delete_proc;	///<描画削除関数
	FLDMMDL_DRAW_PROC_PUSH draw_push_proc;	///<描画退避関数
	FLDMMDL_DRAW_PROC_POP draw_pop_proc;	///<描画復帰関数
	
	u8 move_proc_work[FLDMMDL_MOVE_WORK_SIZE];///<動作関数用ワーク
	u8 move_sub_proc_work[FLDMMDL_MOVE_SUB_WORK_SIZE];///<動作サブ関数用ワーク
	u8 move_cmd_proc_work[FLDMMDL_MOVE_CMD_WORK_SIZE];///<動作コマンド用ワーク
	u8 draw_proc_work[FLDMMDL_DRAW_WORK_SIZE];///<描画関数用ワーク
	
	GFL_BBDACT_ACTUNIT_ID blActID;
}FLDMMDL;

#define FLDMMDL_SIZE (sizeof(FLDMMDL)) ///<FLDMMDLサイズ

HEAPID FLDMMDLSYS_GetHeapID( const FLDMMDLSYS *fos )
{
	return( fos->heapID );
}

HEAPID FLDMMDL_GetHeapID( const FLDMMDL *fmmdl )
{
	return( FLDMMDLSYS_GetHeapID(fmmdl->fmmdlsys) );
}

GFL_TCBSYS * FLDMMDLSYS_GetTCBSYS( const FLDMMDLSYS *fos )
{
	return( ((FLDMMDLSYS*)fos)->pTCBSys );
}

GFL_TCBSYS * FLDMMDL_GetTCBSYS( FLDMMDL *fmmdl )
{
	return( FLDMMDLSYS_GetTCBSYS(fmmdl->fmmdlsys) );
}

FIELD_MAIN_WORK * FLDMMDLSYS_GetFieldMainWork( FLDMMDLSYS *fldmmdlsys )
{
	return( fldmmdlsys->pFldMainWork );
}

void FLDMMDLSYS_SetBlActCont( FLDMMDLSYS *fldmmdlsys, FLDMMDL_BLACTCONT *pBlActCont )
{
	fldmmdlsys->pBlActCont = pBlActCont;
}

FLDMMDL_BLACTCONT * FLDMMDLSYS_GetBlActCont( FLDMMDLSYS *fldmmdlsys )
{
	GF_ASSERT( fldmmdlsys->pBlActCont != NULL );
	return( fldmmdlsys->pBlActCont );
}

FLDMMDLSYS * FLDMMDL_GetFldMMdlSys( const FLDMMDL *fmmdl )
{
	return( (FLDMMDLSYS*)(fmmdl->fmmdlsys) );
}

void FLDMMDL_SetBlActID( FLDMMDL *fldmmdl, GFL_BBDACT_ACTUNIT_ID blActID )
{
	fldmmdl->blActID = blActID;
}

GFL_BBDACT_ACTUNIT_ID FLDMMDL_GetBlActID( FLDMMDL *fldmmdl )
{
	return( fldmmdl->blActID );
}

const FLDMAPPER * FLDMMDLSYS_GetG3DMapper( const FLDMMDLSYS *fos )
{
	return( fos->pG3DMapper );
}

//--------------------------------------------------------------
///	FLDMMDL_H_LOAD_FILE構造体
//--------------------------------------------------------------
typedef struct
{
	int zone;
	int add_max;			//登録最大数
	int add_count;			//追加数
	const FLDMMDLSYS *sys;	//FLDMMDLSYS *
	FLDMMDL_H *head;		//ヘッダー
}FLDMMDL_H_LOAD_FILE;

///FLDMMDL_H_LOAD_FILEサイズ
#define FLDMMDL_H_LOAD_FILE_SIZE (sizeof(FLDMMDL_H_LOAD_FILE))

//==============================================================================
//	プロトタイプ
//==============================================================================
static void fmmdl_SaveDataSave( void *fsys, FLDMMDL * fmmdl, FLDMMDL_SAVE_DATA_PTR save );
static void fmmdl_SaveDataLoad( FLDMMDL * fmmdl, FLDMMDL_SAVE_DATA_PTR save );
static void fmmdl_DataLoadRecover( const FLDMMDLSYS *fos, FLDMMDL * fmmdl );
static void fmmdl_DataLoadStatusBitRecover( FLDMMDL * fmmdl );
static void fmmdl_DataLoadPosRecover( FLDMMDL * fmmdl );

static void fmmdl_AddFileProc( FLDMMDL_H_LOAD_FILE *work );

static FLDMMDL * fmmdl_SpaceSearch( const FLDMMDLSYS *sys );
static FLDMMDL * fmmdl_AliesSearch( const FLDMMDLSYS *fos, int obj_id,int zone_id );
static void fmmdl_MoveTcbAdd( const FLDMMDLSYS *sys, FLDMMDL * fmmdl );
static void fmmdl_AddHeaderSet(
		FLDMMDL * fmmdl, const FLDMMDL_H *head, void *fsys );
static void fmmdl_AddHeaderPosInit( FLDMMDL * fmmdl, const FLDMMDL_H *head );
static void fmmdl_WorkInit( FLDMMDL * fmmdl, const FLDMMDLSYS *sys );
static void fmmdl_WorkInit_MoveProcInit( FLDMMDL * fmmdl );
static void fmmdl_WorkInit_DrawProcInit( FLDMMDL * fmmdl );
static void fmmdl_WorkClear( FLDMMDL * fmmdl );
static int fmmdl_HeaderAliesCheck(
		const FLDMMDL * fmmdl, int h_zone_id, int max, const FLDMMDL_H *head );
static FLDMMDL * fmmdl_OBJIDZoneIDSearch(
		const FLDMMDLSYS *fos, int obj_id, int zone_id );
static void fmmdl_DeleteAll_OldZoneID( FLDMMDLSYS *fos, u32 zone_id );
static void fmmdl_DrawInitStatusSet( FLDMMDL * fmmdl );
static void fmmdl_DrawEffectFlagInit( FLDMMDL * fmmdl );
static void fmmdl_DrawDeleteStatusSet( FLDMMDL * fmmdl );
static int fmmdl_OBJCodeWkOBJCodeConv( void *fsys, int code );
static void fmmdl_HeightNeedCheckInit( FLDMMDL * fmmdl );
static void fmmdl_MoveWorkInit( FLDMMDL * fmmdl );
static void fmmdl_DrawWorkInit( FLDMMDL * fmmdl );
static void fmmdl_AliesOBJChange( FLDMMDL * fmmdl, const FLDMMDL_H *head, int zone_id );
static void fmmdl_OBJAliesChange( FLDMMDL * fmmdl, int zone_id, const FLDMMDL_H *head );

static void fmmdl_TCB_MoveProc( GFL_TCB * tcb, void *work );
static void fmmdl_TCB_DrawProc( FLDMMDL * fmmdl );

static FLDMMDLSYS * fmmdl_ConstSysLocalGet( const FLDMMDLSYS *sys );
static void fmmdlsys_OBJCountInc( FLDMMDLSYS *sys );
static void fmmdlsys_OBJCountDec( FLDMMDLSYS *sys );
static void fmmdlsys_OBJCountSet( FLDMMDLSYS *sys, int count );
static FLDMMDL * fmmdlsys_FieldOBJWorkGet( const FLDMMDLSYS *sys );

static FLDMMDLSYS * fmmdl_FieldOBJSysGet( const FLDMMDL * fmmdl );

static const FLDMMDL_H * fmmdl_H_OBJIDSearch( int id, int max, const FLDMMDL_H *head );
static int fmmdl_H_AliesCheck( const FLDMMDL_H *head );
static int fmmdl_H_AliesZoneIDGet( const FLDMMDL_H *head );

static const FLDMMDL_MOVE_PROC_LIST * fmmdl_MoveProcListGet( u32 code );
static FLDMMDL_MOVE_PROC_INIT fmmdl_MoveProcList_InitGet(
		const FLDMMDL_MOVE_PROC_LIST *list );
static FLDMMDL_MOVE_PROC fmmdl_MoveProcList_MoveGet(
		const FLDMMDL_MOVE_PROC_LIST *list );
static FLDMMDL_MOVE_PROC_DEL fmmdl_MoveProcList_DeleteGet(
		const FLDMMDL_MOVE_PROC_LIST *list );
static FLDMMDL_DRAW_PROC_PUSH fmmdl_DrawProcList_PushGet(
		const FLDMMDL_DRAW_PROC_LIST *list );
static FLDMMDL_DRAW_PROC_POP fmmdl_DrawProcList_PopGet(
		const FLDMMDL_DRAW_PROC_LIST *list );

static FLDMMDL_DRAW_PROC_INIT fmmdl_DrawProcList_InitGet(
	const FLDMMDL_DRAW_PROC_LIST *list );
static FLDMMDL_DRAW_PROC fmmdl_DrawProcList_DrawGet( const FLDMMDL_DRAW_PROC_LIST *list );
static FLDMMDL_DRAW_PROC_DEL fmmdl_DrawProcList_DeleteGet(
		const FLDMMDL_DRAW_PROC_LIST *list );
static const FLDMMDL_DRAW_PROC_LIST * fmmdl_DrawProcListGet( u32 code );

//==============================================================================
//	フィールド動作モデル	システム
//==============================================================================
//--------------------------------------------------------------
/**
 * FIELD OBJ システム初期化
 * @param		fsys			FIELDSYS_WORK *
 * @param		max 			処理するOBJ最大数
 * @retval		FLDMMDLSYS*	追加されたフィールド動作モデル*
 */
//--------------------------------------------------------------
FLDMMDLSYS * FLDMMDLSYS_Init(
	FIELD_MAIN_WORK *pFldMainWork,
	const FLDMAPPER *pG3DMapper, HEAPID heapID, int max )
{
	FLDMMDLSYS *fos;
	
	fos = GFL_HEAP_AllocClearMemory( heapID, FLDMMDLSYS_SIZE );
	fos->fmmdl_work = GFL_HEAP_AllocClearMemory( heapID, max*FLDMMDL_SIZE );
	fos->fmmdl_max = max;
	fos->heapID = heapID;
	fos->pFldMainWork = pFldMainWork;
	fos->pG3DMapper = pG3DMapper;

	fos->pTCBSysWork = GFL_HEAP_AllocMemory(
		heapID, GFL_TCB_CalcSystemWorkSize(max) );
	fos->pTCBSys = GFL_TCB_Init( max, fos->pTCBSysWork );
	return( fos );
}

//--------------------------------------------------------------
/**
 * FIELD OBJ システム削除
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_Delete( FLDMMDLSYS *fos )
{
	GFL_TCB_Exit( fos->pTCBSys );
	GFL_HEAP_FreeMemory( fos->pTCBSys );
	GFL_HEAP_FreeMemory( fos->fmmdl_work );
	GFL_HEAP_FreeMemory( fos );
}

//--------------------------------------------------------------
/**
 * FIELD OBJ　全てを削除
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_DeleteAll( FLDMMDLSYS *fos )
{
	FLDMMDLSYS_DeleteMMdl( fos );
	FLDMMDLSYS_DrawDelete( fos );
	FLDMMDLSYS_Delete( fos );
}

//--------------------------------------------------------------
/**
 *
 * @param
 * @retval
 *
 */
//--------------------------------------------------------------
void FLDMMDLSYS_UpdateMove( FLDMMDLSYS *fos )
{
	GFL_TCBSYS *tcbsys = fos->pTCBSys;
	if( tcbsys != NULL ){
		GFL_TCB_Main( tcbsys );
	}
}

//--------------------------------------------------------------
/**
 * FIELD OBJ　ゾーン更新時の削除処理
 * @param	sys		FLDMMDLSYS *
 * @param	old_zone	古いゾーンID
 * @param	new_zone	新しいゾーンID
 * @param	head_max	head要素数
 * @param	head	新しいのゾーンに配置するFLDMMDL_H *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_ZoneUpdateDelete( FLDMMDLSYS *fos,
		int old_zone, int new_zone, int head_max, const FLDMMDL_H *head )
{
#if 0
	int ret,max = FLDMMDLSYS_OBJMaxGet( fos );
	FLDMMDL * fmmdl = FLDMMDLSYS_fmmdlWorkGet( fos );
	
	#ifdef DEBUG_FLDMMDL_PRINTF
	OS_Printf( "古いZoneに配置されたフィールド動作モデル　削除開始\n" );
	#endif
	
	while( max ){
		if( FLDMMDL_StatusBitCheck_Use(fmmdl) == TRUE ){
			ret = fmmdl_HeaderAliesCheck( fmmdl, new_zone, head_max, head );
			
			switch( ret ){
			case RET_ALIES_NOT:
				if( FLDMMDL_ZoneIDGet(fmmdl) != new_zone ){
					if( FLDMMDL_StatusBit_CheckEasy(
							fmmdl,FLDMMDL_STABIT_ZONE_DEL_NOT) == FALSE ){
						FLDMMDL_Delete( fmmdl );
					}
				}
				
				break;
			case RET_ALIES_CHANGE:
				#ifdef DEBUG_FLDMMDL_PRINTF
				OS_Printf(
					"エイリアス対象のOBJあり OBJ_ID=%d ZONE_ID=%d GRID X=%d,GRID Z=%d \n",
					FLDMMDL_OBJIDGet(fmmdl),
					new_zone,
					FLDMMDL_NowPosGX_Get(fmmdl), FLDMMDL_NowPosGZ_Get(fmmdl) );
				#endif
				break;
			case RET_ALIES_EXIST:
				#ifdef DEBUG_FLDMMDL_PRINTF
				OS_Printf( "エイリアス対象のOBJが既にエイリアス化しています " );
				OS_Printf( "OBJ_ID=%d ", FLDMMDL_OBJIDGet(fmmdl) );
				OS_Printf( "ZONE_ID=%d ", FLDMMDL_ZoneIDGetAlies(fmmdl) );
				OS_Printf( "追加中のゾーンID=%d ", new_zone );
				OS_Printf( "GRID X=%d, ", FLDMMDL_NowPosGX_Get(fmmdl) );
				OS_Printf( "GRID Z=%d\n", FLDMMDL_NowPosGZ_Get(fmmdl) );
				#endif
				break;
			}
		}
		
		fmmdl++;
		max--;
	}
	
	FLDMMDL_BlActCont_ResmGuestDeleteAll( fos, FLDMMDLSYS_BlActContGet(fos) );
	#ifdef DEBUG_FLDMMDL_PRINTF
	OS_Printf( "古いZoneに配置されたフィールド動作モデル　削除完了\n" );
	#endif
#endif
}

//==============================================================================
//	フィールド動作モデルシステム　パーツ
//==============================================================================
//--------------------------------------------------------------
/**
 * FIELD OBJ 作業領域確保
 * @param	max				処理するOBJ最大数
 * @retval	FLDMMDLSYS	確保した領域*
 */
//--------------------------------------------------------------
#if 0
static FLDMMDLSYS *fmmdlsys_AllocMemory( HEAPID heapID, int max )
{
	int size;
	FLDMMDL * fmmdl;
	FLDMMDLSYS *fos;
	
	fos = GFL_HEAP_AllocClearMemory( heapID, FLDMMDLSYS_SIZE );
	
	size = FLDMMDL_SIZE * max;
	fmmdl = GFL_HEAP_AllocClearMemory( heapID, size );
	
	FLDMMDLSYS_FieldOBJWorkSet( fos, fmmdl );
	return( fos );
}
#endif

//==============================================================================
//	フィールド動作モデル
//==============================================================================
//--------------------------------------------------------------
/**
 * フィールド動作モデルを追加 引数ヘッダー
 * @param	fos			FLDMMDLSYS *
 * @param	head		追加する情報を纏めたFLDMMDL_H *
 * @param	zone_id		ゾーンID
 * @retval	FLDMMDL	追加されたFLDMMDL * 。NULL=追加不可
 */
//--------------------------------------------------------------
FLDMMDL * FLDMMDL_AddH(
	const FLDMMDLSYS *fos, const FLDMMDL_H *header, int zone_id )
{
	FLDMMDL * fmmdl;
	FLDMMDL_H headdata = *header;
	FLDMMDL_H *head = &headdata;
	
	fmmdl = fmmdl_SpaceSearch( fos );
	GF_ASSERT( fmmdl != NULL );
	
	fmmdl_AddHeaderSet( fmmdl, head, NULL );
	fmmdl_WorkInit( fmmdl, fos );
	FLDMMDL_ZoneIDSet( fmmdl, zone_id );
	
	fmmdl_MoveWorkInit( fmmdl );
	fmmdl_DrawWorkInit( fmmdl );
	
	FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_MOVE_START );
	
	fmmdl_MoveTcbAdd( fos, fmmdl );
	fmmdlsys_OBJCountInc( fmmdl_ConstSysLocalGet(fos) );
	
	return( fmmdl );
}

#if 0 //pl
FLDMMDL * FLDMMDL_AddH( const FLDMMDLSYS *fos, const FLDMMDL_H *headtest, int zone_id )
{
	FLDMMDL * fmmdl;
	FLDMMDL_H headdata = *headtest;
	FLDMMDL_H *head = &headdata;
	
#if 0	
	{
		int obj_id = FLDMMDL_H_IDGet( head );
		
		if( fmmdl_H_AliesCheck(head) == FALSE ){	//エイリアスではない
			fmmdl = fmmdl_AliesSearch( fos, obj_id, zone_id );
			
			if( fmmdl != NULL ){		//エイリアスで既に存在している
				//エイリアスからフィールド動作モデルへ
				fmmdl_AliesOBJChange( fmmdl, head, zone_id );
				return( fmmdl );
			}
		}else{		//エイリアスである
			fmmdl = fmmdl_OBJIDZoneIDSearch(
				fos, obj_id, fmmdl_H_AliesZoneIDGet(head) );
			
			if( fmmdl != NULL ){	//既にフィールド動作モデルが存在する
				//フィールド動作モデルからエイリアスへ
				fmmdl_OBJAliesChange( fmmdl, zone_id, head );
				return( fmmdl );
			}
		}
	}
#endif
	
	fmmdl = fmmdl_SpaceSearch( fos );
	
	if( fmmdl == NULL ){
		return( fmmdl );
	}
	
//	fmmdl_AddHeaderSet( fmmdl, head, FLDMMDLSYS_FieldSysWorkGet(fos) );
	fmmdl_WorkInit( fmmdl, fos );
	FLDMMDL_ZoneIDSet( fmmdl, zone_id );
	
	fmmdl_MoveWorkInit( fmmdl );
	fmmdl_DrawWorkInit( fmmdl );
	
	FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_MOVE_START );
	
	fmmdl_MoveTcbAdd( fos, fmmdl );
	fmmdlsys_OBJCountInc( fmmdl_ConstSysLocalGet(fos) );
	
	return( fmmdl );
}
#endif

#if 0
FLDMMDL * FLDMMDL_AddH( const FLDMMDLSYS *fos, const FLDMMDL_H *head, int zone_id )
{
	FLDMMDL * fmmdl;
	
	{
		int obj_id = FLDMMDL_H_IDGet( head );
		
		if( fmmdl_H_AliesCheck(head) == FALSE ){				//エイリアスではない
			fmmdl = fmmdl_AliesSearch( fos, obj_id, zone_id );
			
			if( fmmdl != NULL ){								//エイリアスで既に存在している
				fmmdl_AliesOBJChange( fmmdl, head, zone_id );	//エイリアスからフィールド動作モデルへ
				OS_Printf( "エイリアス -> フィールド動作モデル OBJ_ID %d ", obj_id );
				OS_Printf( "GIRD X=%d ", FLDMMDL_H_PosXGet(head) );
				OS_Printf( "GIRD Z=%d\n", FLDMMDL_H_PosZGet(head) );
				return( fmmdl );
			}
		}else{													//エイリアスである
			fmmdl = fmmdl_OBJIDZoneIDSearch( fos, obj_id, fmmdl_H_AliesZoneIDGet(head) );
			
			if( fmmdl != NULL ){								//既にフィールド動作モデルが存在する
				fmmdl_OBJAliesChange( fmmdl, zone_id, head );	//フィールド動作モデルからエイリアスへ
				OS_Printf( "フィールド動作モデル -> エイリアス OBJ_ID %d ", obj_id );
				OS_Printf( "GIRD X=%d ", FLDMMDL_H_PosXGet(head) );
				OS_Printf( "GIRD Z=%d\n", FLDMMDL_H_PosZGet(head) );
				return( fmmdl );
			}
			
			OS_Printf( "エイリアスが配置されます " );
			OS_Printf( "ZONE_ID %d, OBJ_ID %d ", obj_id, fmmdl_H_AliesZoneIDGet(head) );
			OS_Printf( "GIRD X=%d ", FLDMMDL_H_PosXGet(head) );
			OS_Printf( "GIRD Z=%d\n", FLDMMDL_H_PosZGet(head) );
		}
	}
	
	fmmdl = fmmdl_SpaceSearch( fos );
	
	if( fmmdl == NULL ){
		return( fmmdl );
	}
	
	fmmdl_AddHeaderSet( fmmdl, head, FLDMMDLSYS_FieldSysWorkGet(fos) );
	fmmdl_WorkInit( fmmdl, fos );
	FLDMMDL_ZoneIDSet( fmmdl, zone_id );
	
	fmmdl_MoveWorkInit( fmmdl );
	fmmdl_DrawWorkInit( fmmdl );
	
	FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_MOVE_START );
	
	fmmdl_MoveTcbAdd( fos, fmmdl );
	fmmdlsys_OBJCountInc( fmmdl_ConstSysLocalGet(fos) );
	
	return( fmmdl );
}
#endif

//--------------------------------------------------------------
/**
 * フィールド動作モデルを追加。最低限必要な情報からヘッダー作成して追加。
 * 足りない情報は0でクリアされている。必要あれば追加後に各自で自由に設定。
 * @param	fos			FLDMMDLSYS *
 * @param	x			初期グリッド座標X
 * @param	z			初期グリッド座標Z
 * @param	dir			初期方向。DIR_UP等
 * @param	obj			OBJコード。HERO等
 * @param	move		動作コード。MV_RND等
 * @param	zone_id		ゾーンID ZONE_ID_NOTHING等
 * @retval	FLDMMDL	追加されたFLDMMDL * 。NULL=追加不可
 */
//--------------------------------------------------------------
FLDMMDL * FLDMMDL_AddHMake(
		const FLDMMDLSYS *fos, int x, int z, int dir, int obj, int move, int zone_id )
{
	FLDMMDL_H head;
	FLDMMDL * fmmdl;
	
	FLDMMDL_H_IDSet( &head, 0 );
	FLDMMDL_H_OBJCodeSet( &head, obj );
	FLDMMDL_H_MoveCodeSet( &head, move );
	FLDMMDL_H_EventTypeSet( &head, 0 );
	FLDMMDL_H_EventFlagSet( &head, 0 );
	FLDMMDL_H_EventIDSet( &head, 0 );
	FLDMMDL_H_DirSet( &head, dir );
	FLDMMDL_H_ParamSet( &head, 0, FLDMMDL_PARAM_0 );
	FLDMMDL_H_ParamSet( &head, 0, FLDMMDL_PARAM_1 );
	FLDMMDL_H_ParamSet( &head, 0, FLDMMDL_PARAM_2 );
	FLDMMDL_H_MoveLimitXSet( &head, 0 );
	FLDMMDL_H_MoveLimitZSet( &head, 0 );
	FLDMMDL_H_PosXSet( &head, x );
	FLDMMDL_H_PosZSet( &head, z );
	FLDMMDL_H_PosYSet( &head, 0 );
	
	fmmdl = FLDMMDL_AddH( fos, &head, zone_id );
	
	return( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL_Hテーブルより指定のIDを持ったOBJを追加
 * @param	fos		FLDMMDLSYS *
 * @param	id		追加するhead検索ID
 * @param	zone	追加の際に指定するゾーンID
 * @param	max		head要素数
 * @param	head	FLDMMDL_H *
 * @retval	FLDMMDL *	追加されたFLDMMDL * NULL=id該当無し
 */
//--------------------------------------------------------------
FLDMMDL * FLDMMDL_AddHEvent(
		const FLDMMDLSYS *fos, int id, int max, int zone, const FLDMMDL_H *head )
{
	FLDMMDL * fmmdl = NULL;
	const FLDMMDL_H *hit_head = fmmdl_H_OBJIDSearch( id, max, head );
	
	if( hit_head != NULL ){
		int flag = FLDMMDL_H_EventFlagGet( hit_head );
//		void *fsys = FLDMMDLSYS_FieldSysWorkGet( fos );
#if 0	
		if( CheckEventFlag(fsys,flag) == FALSE ){
			fmmdl = FLDMMDL_AddH( fos, hit_head, zone );
		}
#else
		fmmdl = FLDMMDL_AddH( fos, hit_head, zone );
#endif
	}
	
	return( fmmdl );
}

//--------------------------------------------------------------
/**
 * FIELD OBJ　表示系のみ再設定　削除は行わない
 * @param	fmmdl	FLDMMDL *
 * @param	code	OBJコード HERO等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawResetAlone( FLDMMDL * fmmdl, int code )
{
	FLDMMDL_OBJCodeSet( fmmdl, code );
	fmmdl_DrawInitStatusSet( fmmdl );
	FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_DRAW_PROC_INIT_COMP );
	fmmdl_DrawWorkInit( fmmdl );
}

//--------------------------------------------------------------
/**
 * FIELD OBJ　現在の描画関数を削除して再設定
 * @param	fmmdl	FLDMMDL *
 * @param	code	OBJコード HERO等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawReset( FLDMMDL * fmmdl, int code )
{
	if( FLDMMDL_StatusBit_DrawInitCompCheck(fmmdl) == TRUE ){
		FLDMMDL_DrawDelete( fmmdl );
	}
	
	FLDMMDL_DrawResetAlone( fmmdl, code );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルを削除
 * @param	fmmdl		削除するFLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_Delete( FLDMMDL * fmmdl )
{
	const FLDMMDLSYS *fos;
	
	fos = FLDMMDL_FieldOBJSysGet( fmmdl );
	
	if( FLDMMDLSYS_DrawInitCompCheck(fos) == TRUE ){
		FLDMMDL_DrawDeleteProcCall( fmmdl );
	}
	
	FLDMMDL_MoveDeleteProcCall( fmmdl );
	FLDMMDL_MoveTcbDelete( fmmdl );
	fmmdlsys_OBJCountDec( fmmdl_FieldOBJSysGet(fmmdl) );
	fmmdl_WorkClear( fmmdl );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルを削除　スクリプトのOBJ削除フラグをONにする
 * @param	fmmdl		削除するFLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DeleteEvent( FLDMMDL * fmmdl )
{
	int ev;

	ev = FLDMMDL_EventFlagGet( fmmdl );
//	SetEventFlag( FLDMMDL_FieldSysWorkGet(fmmdl), ev );
	FLDMMDL_Delete( fmmdl );
}

//--------------------------------------------------------------
/**
 * FIELD OBJ　表示系のみ削除
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawDelete( FLDMMDL * fmmdl )
{
	const FLDMMDLSYS *fos;
	
	fos = FLDMMDL_FieldOBJSysGet( fmmdl );
	
	if( FLDMMDLSYS_DrawInitCompCheck(fos) == TRUE ){
		if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_DRAW_PROC_INIT_COMP) ){
			FLDMMDL_DrawDeleteProcCall( fmmdl );
		}
		
		FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_DRAW_PROC_INIT_COMP );
	}
	
	FLDMMDL_OBJCodeSet( fmmdl, OBJCODEMAX );
	
	FLDMMDL_DrawInitProcSet( fmmdl, FLDMMDL_DrawInitProcDummy );
	FLDMMDL_DrawProcSet( fmmdl, FLDMMDL_DrawProcDummy );
	FLDMMDL_DrawDeleteProcSet( fmmdl, FLDMMDL_DrawProcDummy );
	FLDMMDL_DrawPushProcSet( fmmdl, FLDMMDL_DrawPushProcDummy );
	FLDMMDL_DrawPopProcSet( fmmdl, FLDMMDL_DrawPopProcDummy );
}

//--------------------------------------------------------------
/**
 * 現在発生しているフィールド動作モデルを全て削除
 * @param	fmmdl		削除するFLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_DeleteMMdl( FLDMMDLSYS *fos )
{
	int i,max;
	FLDMMDL * fmmdl;
	
	i = 0;
	max = FLDMMDLSYS_OBJMaxGet( fos );
	fmmdl = FLDMMDLSYS_fmmdlWorkGet( fos );
	
	do{
		if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_USE) ){
			FLDMMDL_Delete( fmmdl );
		}
		
		fmmdl++;
		i++;
	}while( i < max );
}

//--------------------------------------------------------------
/**
 * 現在発生しているフィールド動作モデル全てを退避
 * @param	fmmdl		削除するFLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_PushAll( FLDMMDLSYS *fos )
{
	int i,max;
	FLDMMDL * fmmdl;
	
	i = 0;
	max = FLDMMDLSYS_OBJMaxGet( fos );
	fmmdl = FLDMMDLSYS_fmmdlWorkGet( fos );
	
	do{
		if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_USE) ){
			if( FLDMMDL_StatusBitCheck_DrawProcInitComp(fmmdl) == TRUE ){
				FLDMMDL_DrawPushProcCall( fmmdl );
			}
		}
		
		fmmdl++;
		i++;
	}while( i < max );
}

//--------------------------------------------------------------
/**
 * 現在発生しているフィールド動作モデル全てにたいして描画処理の退避を行う
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawProcPushAll( FLDMMDLSYS *fos )
{
	#ifdef DEBUG_FLDMMDL_PRINTF
	if( FLDMMDLSYS_DrawInitCompCheck(fos) != TRUE ){
		OS_Printf( "FLDMMDL 描画処理が存在していないのに" );
		OS_Printf( "FLDMMDL_DrawProcPushAll()が呼ばれています" );
	}
	#endif
	
	GF_ASSERT( FLDMMDLSYS_DrawInitCompCheck(fos) == TRUE );
	
	{
		int i,max;
		FLDMMDL * fmmdl;
	
		i = 0;
		max = FLDMMDLSYS_OBJMaxGet( fos );
		fmmdl = FLDMMDLSYS_fmmdlWorkGet( fos );
	
		do{
			if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_USE) ){
				if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_DRAW_PROC_INIT_COMP) ){
					FLDMMDL_DrawPushProcCall( fmmdl );
					fmmdl_DrawDeleteStatusSet( fmmdl );
				}
			}
		
			fmmdl++;
			i++;
		}while( i < max );
	}
	
	#ifdef DEBUG_FLDMMDL_PRINTF
	OS_Printf( "フィールド動作モデル 描画情報を退避しました\n" );
	#endif
}

//--------------------------------------------------------------
/**
 * 現在発生しているフィールド動作モデル全てを復帰
 * @param	fmmdl		削除するFLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_PopAll( FLDMMDLSYS *fos )
{
	int i,max;
	FLDMMDL * fmmdl;
	
	i = 0;
	max = FLDMMDLSYS_OBJMaxGet( fos );
	fmmdl = FLDMMDLSYS_fmmdlWorkGet( fos );
	
	do{
		if( FLDMMDL_StatusBitCheck_Use(fmmdl) == TRUE ){
			if( FLDMMDL_StatusBitCheck_DrawProcInitComp(fmmdl) == TRUE ){
				FLDMMDL_DrawPopProcCall( fmmdl );
			}
		}
	
		fmmdl++;
		i++;
	}while( i < max );
}

//--------------------------------------------------------------
/**
 * 現在発生しているフィールド動作モデル全てに対して描画処理復帰
 * @param	fmmdl		削除するFLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawProcPopAll( FLDMMDLSYS *fos )
{
	#ifdef DEBUG_FLDMMDL_PRINTF
	if( FLDMMDLSYS_DrawInitCompCheck(fos) != TRUE ){
		OS_Printf("FLDMMDL 描画処理未初期化で描画復帰処理が呼ばれています\n");
	}
	#endif
	GF_ASSERT( FLDMMDLSYS_DrawInitCompCheck(fos) == TRUE );
	
	{
		int i = 0;
		int max = FLDMMDLSYS_OBJMaxGet( fos );
		FLDMMDL * fmmdl = FLDMMDLSYS_fmmdlWorkGet( fos );
	
		do{
			if( FLDMMDL_StatusBitCheck_Use(fmmdl) == TRUE ){
				if( FLDMMDL_StatusBitCheck_DrawProcInitComp(fmmdl) == TRUE ){
					FLDMMDL_DrawPopProcCall( fmmdl );
				}else{
					fmmdl_DrawWorkInit( fmmdl );
				}
				
				fmmdl_DrawInitStatusSet( fmmdl );
				FLDMMDL_DrawInitAfterMoveProcCall( fmmdl );
			}
		
			fmmdl++;
			i++;
		}while( i < max );
	}
	
	#ifdef DEBUG_FLDMMDL_PRINTF
	OS_Printf( "フィールド動作モデル 描画情報を復帰しました\n" );
	#endif
}

//==============================================================================
//	フィールド動作モデル セーブ
//==============================================================================
//--------------------------------------------------------------
/**
 * 現在発生しているフィールド動作モデル全てをセーブデータに保存
 * @param	fos		FLDMMDLSYS *
 * @param	save	保存先FLDMMDL_SAVE_DATA_PTR
 * @param	max		saveの要素数
 * @retval	nothing
 */
//--------------------------------------------------------------
#if 0
void FLDMMDLSYS_DataSaveAll(
		void *fsys, const FLDMMDLSYS *fos, FLDMMDL_SAVE_DATA_PTR save, int max )
{
	int no = 0;
	FLDMMDL * fmmdl;
	
	while( FLDMMDLSYS_FieldOBJSearch(fos,&fmmdl,&no,FLDMMDL_STABIT_USE) ){
		fmmdl_SaveDataSave( fsys, fmmdl, save );
		save++; max--;
		GF_ASSERT( max > 0 && "FLDMMDLSYS_DataSaveAll()異常" );
	}
	
	if( max ){
		GFL_STD_MemClear( save, max * FLDMMDL_SAVE_DATA_SIZE );
	}
}
#endif

//--------------------------------------------------------------
/**
 * セーブしたデータをロード
 * @param	fos		FLDMMDLSYS *
 * @param	save	保存したデータが格納されたFLDMMDL_SAVE_DATA_PTR
 * @param	max		saveの要素数
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_DataLoadAll(
		const FLDMMDLSYS *fos, FLDMMDL_SAVE_DATA_PTR save, int max )
{
	int i = 0;
	FLDMMDL * fmmdl;
	
	while( max ){
		if( (save->status_bit & FLDMMDL_STABIT_USE) ){
			fmmdl = fmmdl_SpaceSearch( fos );
			GF_ASSERT( fmmdl != NULL && "FLDMMDLSYS_DataLoadAll()異常" );
		
			fmmdl_SaveDataLoad( fmmdl, save );
			fmmdl_DataLoadRecover( fos, fmmdl );
		}
		
		save++;
		max--;
	}
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルの情報をセーブデータにセット
 * @param	fmmdl		FLDMMDL *
 * @param	save		FLDMMDL_SAVE_DATA_PTR
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_SaveDataSave( void *fsys, FLDMMDL * fmmdl, FLDMMDL_SAVE_DATA_PTR save )
{
	save->status_bit = FLDMMDL_StatusBit_Get( fmmdl );
	save->move_bit = FLDMMDL_MoveBit_Get( fmmdl );
	save->obj_id = FLDMMDL_OBJIDGet( fmmdl );
	save->zone_id = FLDMMDL_ZoneIDGet( fmmdl );
	save->obj_code = FLDMMDL_OBJCodeGet( fmmdl );
	save->move_code = FLDMMDL_MoveCodeGet( fmmdl );
	save->event_type = FLDMMDL_EventTypeGet( fmmdl );
	save->event_flag = FLDMMDL_EventFlagGet( fmmdl );
	save->event_id = FLDMMDL_EventIDGet( fmmdl );
	save->dir_head = FLDMMDL_DirHeaderGet( fmmdl );
	save->dir_disp = FLDMMDL_DirDispGet( fmmdl );
	save->dir_move = FLDMMDL_DirMoveGet( fmmdl );
	save->param0 = FLDMMDL_ParamGet( fmmdl, FLDMMDL_PARAM_0 );
	save->param1 = FLDMMDL_ParamGet( fmmdl, FLDMMDL_PARAM_1 );
	save->param2 = FLDMMDL_ParamGet( fmmdl, FLDMMDL_PARAM_2 );
	save->move_limit_x = FLDMMDL_MoveLimitXGet( fmmdl );
	save->move_limit_z = FLDMMDL_MoveLimitZGet( fmmdl );
	save->gx_init = FLDMMDL_InitPosGX_Get( fmmdl );
	save->gy_init = FLDMMDL_InitPosGY_Get( fmmdl );
	save->gz_init = FLDMMDL_InitPosGZ_Get( fmmdl );
	save->gx_now = FLDMMDL_NowPosGX_Get( fmmdl );
	save->gy_now = FLDMMDL_NowPosGY_Get( fmmdl );
	save->gz_now = FLDMMDL_NowPosGZ_Get( fmmdl );

	//高さの保存
#if 0
	{
		HEIGHT_TYPE flag;
		fx32 x,y,z;
		VecFx32 vec_pos;
		//グリッドをFX32型にする(グリッドの中心座標)
		x = save->gx_now * 16 * FX32_ONE+(8*FX32_ONE);
		z = save->gz_now * 16 * FX32_ONE+(8*FX32_ONE);
		FLDMMDL_VecPosGet( fmmdl, &vec_pos );

		y = GetHeightPack( fsys, vec_pos.y, x, z, &flag );
		
		if( flag == HEIGHT_FAIL ){
			save->fx32_y = save->gy_now * 8 * FX32_ONE;
//			OS_Printf("height_FAIL:%x\n",save->fx32_y);
		}else{
			save->fx32_y = y;
//			OS_Printf("height_SAC:%x\n",save->fx32_y);
		}
	}
#else
	{
		VecFx32 vec;
		int flag,eflag;
		
		FieldOBJTool_GridCenterPosGet( save->gx_now, save->gz_now, &vec );
		vec.y = FLDMMDL_VecPosYGet( fmmdl );
		
		eflag = FLDMMDL_StatusBitCheck_HeightExpand( fmmdl );
		#if 0
		flag = FieldOBJTool_GetHeightExpand( fsys, &vec, eflag );
		#else
		flag = FALSE;
		#endif
		
		if( flag == FALSE ){
			#if 0
			//これだと高さ取得失敗時、
			//それまで正常に取得していた高さを無視してグリッド単位から
			//実座標に変換してしまい、斜め等の端数が無効となってしまう。
			save->fx32_y = H_GRID_SIZE_FX32( save->gy_now );
			#else
			save->fx32_y = FLDMMDL_VecPosYGet( fmmdl );	
			#endif
		}else{
			#if 1 //PL 高さ取得禁止時は実座標そのまま保存
			if( FLDMMDL_HeightOFFCheck(fmmdl) == TRUE ){
				vec.y = FLDMMDL_VecPosYGet( fmmdl );
			}
			#endif
			
			save->fx32_y = vec.y;
		}
	}
#endif
	
	memcpy( save->move_proc_work,
			FLDMMDL_MoveProcWorkGet(fmmdl), FLDMMDL_MOVE_WORK_SIZE );
	memcpy( save->move_sub_proc_work,
			FLDMMDL_MoveSubProcWorkGet(fmmdl), FLDMMDL_MOVE_SUB_WORK_SIZE );
}

//--------------------------------------------------------------
/**
 * セーブデータの情報をフィールド動作モデルにセット
 * @param	fmmdl		FLDMMDL *
 * @param	save		FLDMMDL_SAVE_DATA_PTR
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_SaveDataLoad( FLDMMDL * fmmdl, FLDMMDL_SAVE_DATA_PTR save )
{
	FLDMMDL_StatusBit_Init( fmmdl, save->status_bit );
	FLDMMDL_MoveBit_Init( fmmdl, save->move_bit );
	FLDMMDL_OBJIDSet( fmmdl,  save->obj_id );
	FLDMMDL_ZoneIDSet( fmmdl, save->zone_id );
	FLDMMDL_OBJCodeSet( fmmdl, save->obj_code );
	FLDMMDL_MoveCodeSet( fmmdl, save->move_code );
	FLDMMDL_EventTypeSet( fmmdl, save->event_type );
	FLDMMDL_EventFlagSet( fmmdl, save->event_flag );
	FLDMMDL_EventIDSet( fmmdl, save->event_id );
	FLDMMDL_DirHeaderSet( fmmdl, save->dir_head );
	FLDMMDL_DirDispSetForce( fmmdl, save->dir_disp );
	FLDMMDL_DirMoveSet( fmmdl, save->dir_move );
	FLDMMDL_ParamSet( fmmdl, save->param0, FLDMMDL_PARAM_0 );
	FLDMMDL_ParamSet( fmmdl, save->param1, FLDMMDL_PARAM_1 );
	FLDMMDL_ParamSet( fmmdl, save->param2, FLDMMDL_PARAM_2 );
	FLDMMDL_MoveLimitXSet( fmmdl, save->move_limit_x );
	FLDMMDL_MoveLimitZSet( fmmdl, save->move_limit_z );
	FLDMMDL_InitPosGX_Set( fmmdl, save->gx_init );
	FLDMMDL_InitPosGY_Set( fmmdl, save->gy_init );
	FLDMMDL_InitPosGZ_Set( fmmdl, save->gz_init );
	FLDMMDL_NowPosGX_Set( fmmdl, save->gx_now );
	FLDMMDL_NowPosGY_Set( fmmdl, save->gy_now );
	FLDMMDL_NowPosGZ_Set( fmmdl, save->gz_now );

	//高さ復帰
	{
		VecFx32 vec = {0,0,0};
		vec.y = save->fx32_y;
		FLDMMDL_VecPosSet( fmmdl, &vec );
	}
	
	memcpy( FLDMMDL_MoveProcWorkGet(fmmdl),
			save->move_proc_work, FLDMMDL_MOVE_WORK_SIZE );
	memcpy( FLDMMDL_MoveSubProcWorkGet(fmmdl),
			save->move_sub_proc_work, FLDMMDL_MOVE_SUB_WORK_SIZE );
}

//--------------------------------------------------------------
/**
 * データロード後の復帰
 * @param	fos		FLDMMDLSYS *
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_DataLoadRecover( const FLDMMDLSYS *fos, FLDMMDL * fmmdl )
{
	fmmdl_DataLoadStatusBitRecover( fmmdl );
	fmmdl_DataLoadPosRecover( fmmdl );
	
	FLDMMDL_FieldOBJSysSet( fmmdl, fos );
	
	fmmdl_WorkInit_MoveProcInit( fmmdl );						//動作関数の初期化のみ
	FLDMMDL_AcmdFree( fmmdl );
	fmmdl_DrawWorkInit( fmmdl );
	
	fmmdl_MoveTcbAdd( fos, fmmdl );
	FLDMMDL_MoveReturnProcCall( fmmdl );
	fmmdlsys_OBJCountInc( fmmdl_ConstSysLocalGet(fos) );
}

//--------------------------------------------------------------
/**
 * データロード後のステータスビット復帰
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_DataLoadStatusBitRecover( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBit_ON( fmmdl,
			FLDMMDL_STABIT_USE |
//			FLDMMDL_STABIT_HEIGHT_GET_NEED |	//セーブ時の高さを信用する
			FLDMMDL_STABIT_MOVE_START );
	
	FLDMMDL_StatusBit_OFF( fmmdl,
			FLDMMDL_STABIT_PAUSE_MOVE |
			FLDMMDL_STABIT_VANISH |
			FLDMMDL_STABIT_DRAW_PROC_INIT_COMP |
			FLDMMDL_STABIT_JUMP_START |
			FLDMMDL_STABIT_JUMP_END |
			FLDMMDL_STABIT_MOVE_END |
			FLDMMDL_STABIT_FELLOW_HIT_NON |
			FLDMMDL_STABIT_TALK_OFF |
			FLDMMDL_STABIT_DRAW_PUSH |
			FLDMMDL_STABIT_BLACT_ADD_PRAC |
			FLDMMDL_STABIT_HEIGHT_GET_OFF );
	
	fmmdl_DrawEffectFlagInit( fmmdl );
}

//--------------------------------------------------------------
/**
 * データロード後の座標復帰
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_DataLoadPosRecover( FLDMMDL * fmmdl )
{
	int pos;
	VecFx32 vec;
	
	FLDMMDL_VecPosGet( fmmdl, &vec );		//Y値のみセット済み

//	OS_Printf("recover_y:%x\n",vec.y);
	
	pos = FLDMMDL_NowPosGX_Get( fmmdl );
	vec.x = GRID_SIZE_FX32( pos ) + FLDMMDL_VEC_X_GRID_OFFS_FX32;
	FLDMMDL_OldPosGX_Set( fmmdl, pos );

	pos = FLDMMDL_NowPosGY_Get( fmmdl );
//	vec.y = H_GRID_SIZE_FX32( pos ) + FLDMMDL_VEC_Y_GRID_OFFS_FX32;	<<ロード時に値がセットされているはず
	FLDMMDL_OldPosGY_Set( fmmdl, pos );
	
	pos = FLDMMDL_NowPosGZ_Get( fmmdl );
	vec.z = GRID_SIZE_FX32( pos ) + FLDMMDL_VEC_Z_GRID_OFFS_FX32;
	FLDMMDL_OldPosGZ_Set( fmmdl, pos );
	
	FLDMMDL_VecPosSet( fmmdl, &vec );
}

//==============================================================================
//	フィールド動作モデル 複数登録
//==============================================================================
#if 0	//用無し

//--------------------------------------------------------------
///	FLDMMDL_AddFile()でファイル名読み込みで使用するバッファサイズ
//--------------------------------------------------------------
#define FLDMMDL_FILE_1LINE_BUF (96)

//--------------------------------------------------------------
/**
 * フィールド動作モデル　ファイル名指定による複数登録
 * @param	sys			FLDMMDLSYS *
 * @param	zone		追加されるゾーンID
 * @param	name		FLDMMDL_Hが記述されたファイル名
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_AddFile( const FLDMMDLSYS *sys, int zone, const char *name )
{
	int i,max;
	char *file,*f_pos;
	char buf[FLDMMDL_FILE_1LINE_BUF];
	FLDMMDL_H *head;
	FLDMMDL_H_LOAD_FILE *load;
	
	if( name == NULL ){
		return;
	}
	
	file = sys_LoadFile( HEAPID_WORLD, name );
	GF_ASSERT( file != NULL && "fieldobj add file alloc error" );
	
	StrTok( file, buf, ',' );								//登録数
	max = AtoI( buf );
	
	OS_Printf( "fieldobj add file load %d\n", max );
	
	head = GFL_HEAP_AllocClearMemoryLo( HEAPID_WORLD, FLDMMDL_H_SIZE * max );
	GF_ASSERT( head != NULL && "fieldobj add file head alloc error" );
	
	f_pos = StrTok( file, buf, RETURN_CODE );				//1行飛ばし
	f_pos = StrTok( f_pos, buf, RETURN_CODE );				//段落飛ばし
	
	for( i = 0; i < max; i++ ){
		f_pos = StrTok( f_pos, buf, ',' );					//ID取得
		FLDMMDL_H_IDSet( &head[i], AtoI(buf) );
		f_pos = StrTok( f_pos, buf, RETURN_CODE );			//1行飛ばし
		
		f_pos = StrTok( f_pos, buf, ',' );					//OBJコード取得
		FLDMMDL_H_OBJCodeSet( &head[i], AtoI(buf) );
		f_pos = StrTok( f_pos, buf, RETURN_CODE );			//1行飛ばし
		
		f_pos = StrTok( f_pos, buf, ',' );					//動作コード取得
		FLDMMDL_H_MoveCodeSet( &head[i], AtoI(buf) );
		f_pos = StrTok( f_pos, buf, RETURN_CODE );			//1行飛ばし
		
		f_pos = StrTok( f_pos, buf, ',' );					//イベントタイプ取得
		FLDMMDL_H_EventTypeSet( &head[i], AtoI(buf) );
		f_pos = StrTok( f_pos, buf, RETURN_CODE );			//1行飛ばし
		
		f_pos = StrTok( f_pos, buf, ',' );					//イベントフラグ取得
		FLDMMDL_H_EventFlagSet( &head[i], AtoI(buf) );
		f_pos = StrTok( f_pos, buf, RETURN_CODE );			//1行飛ばし
		
		f_pos = StrTok( f_pos, buf, ',' );					//イベントID取得
		FLDMMDL_H_EventIDSet( &head[i], AtoI(buf) );
		f_pos = StrTok( f_pos, buf, RETURN_CODE );			//1行飛ばし
		
		f_pos = StrTok( f_pos, buf, ',' );					//方向取得
		FLDMMDL_H_DirSet( &head[i], AtoI(buf) );
		f_pos = StrTok( f_pos, buf, RETURN_CODE );			//1行飛ばし
		
		f_pos = StrTok( f_pos, buf, ',' );					//パラメタ取得
		FLDMMDL_H_ParamSet( &head[i], AtoI(buf) );
		f_pos = StrTok( f_pos, buf, RETURN_CODE );			//1行飛ばし
		
		f_pos = StrTok( f_pos, buf, ',' );					//移動制限X取得
		FLDMMDL_H_MoveLimitXSet( &head[i], AtoI(buf) );
		
		f_pos = StrTok( f_pos, buf, ',' );					//移動制限Z取得
		FLDMMDL_H_MoveLimitZSet( &head[i], AtoI(buf) );
		f_pos = StrTok( f_pos, buf, RETURN_CODE );			//1行飛ばし
		
		f_pos = StrTok( f_pos, buf, ',' );					//X座標取得
		FLDMMDL_H_PosXSet( &head[i], AtoI(buf) );
		
		f_pos = StrTok( f_pos, buf, ',' );					//Z座標取得
		FLDMMDL_H_PosZSet( &head[i], AtoI(buf) );
		f_pos = StrTok( f_pos, buf, RETURN_CODE );			//1行飛ばし
		
		f_pos = StrTok( f_pos, buf, ',' );					//Y座標取得
		FLDMMDL_H_PosYSet( &head[i], AtoI(buf) );
		f_pos = StrTok( f_pos, buf, RETURN_CODE );			//一行飛ばし
		
		f_pos = StrTok( f_pos, buf, RETURN_CODE );			//段落飛ばし
	}
	
	GFL_HEAP_FreeMemory( HEAPID_WORLD, file );
	
	load = GFL_HEAP_AllocClearMemoryLo( HEAPID_WORLD, FLDMMDL_H_LOAD_FILE_SIZE );
	GF_ASSERT( load != NULL && "fieldobj add file work alloc error" );
	
	load->zone = zone;
	load->add_max = max;
	load->add_count = 0;
	load->sys = sys;
	load->head = head;
	
	fmmdl_AddFileProc( load );
	
	OS_Printf( "fieldobj add file load OK\n" );
}
#endif

//--------------------------------------------------------------
/**
 * フィールド動作モデル　バイナリ指定による複数登録
 * @param	sys			FLDMMDLSYS *
 * @param	zone		追加されるゾーンID
 * @param	max			追加する最大数
 * @param	bin			追加するFLDMMDL_Hが配置されているバイナリ*
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_AddBin( const FLDMMDLSYS *sys, int zone, int max, const FLDMMDL_H *bin )
{
	int size;
	FLDMMDL_H *head;
	FLDMMDL_H_LOAD_FILE *load;
	
	#ifdef DEBUG_FLDMMDL_PRINTF
	OS_Printf( "フィールド動作モデル 一括登録開始 配置数 %d\n", max );
	if( max == 0 ){
		OS_Printf( "FLDMMDL_AddBin()　指定最大数異常" );
	}
	#endif
	GF_ASSERT( max );
	
	size = FLDMMDL_H_SIZE *max;
	head = GFL_HEAP_AllocClearMemoryLo( FLDMMDLSYS_GetHeapID(sys), size );
	GF_ASSERT( head != NULL && "fieldobj add bin head alloc error" );
	memcpy( head, bin, size );
	
	load = GFL_HEAP_AllocClearMemoryLo( FLDMMDLSYS_GetHeapID(sys),
			FLDMMDL_H_LOAD_FILE_SIZE );
	GF_ASSERT( load != NULL && "fieldobj add file work alloc error" );
	
	load->zone = zone;
	load->add_max = max;
	load->add_count = 0;
	load->sys = sys;
	load->head = head;
	
	fmmdl_AddFileProc( load );
	
	#ifdef DEBUG_FLDMMDL_PRINTF
	OS_Printf( "フィールド動作モデル 一括登録終了\n" );
	#endif
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル一括登録
 * @param	work	FLDMMDL_H_LOAD_FILE
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_AddFileProc( FLDMMDL_H_LOAD_FILE *work )
{
#if 0
	FLDMMDL * fmmdl;
	void *fsys;
	const FLDMMDL_H *head;
	
	fsys = FLDMMDLSYS_FieldSysWorkGet( work->sys );
	head = work->head;

	do{
		#if 0	
		if( fmmdl_H_AliesCheck(head) == TRUE ||
			CheckEventFlag(fsys,head->event_flag) == FALSE ){
			fmmdl = FLDMMDL_AddH( work->sys, head, work->zone );
			GF_ASSERT( fmmdl != NULL );
		}
		#else
		fmmdl = FLDMMDL_AddH( work->sys, head, work->zone );
		GF_ASSERT( fmmdl != NULL );
		#endif

		head++;
		work->add_count++;
	}while( work->add_count < work->add_max );
	
	GFL_HEAP_FreeMemory( work->head );
	GFL_HEAP_FreeMemory( work );
#endif
}

#if 0	//旧版　エイリアスでのバグあり
static void fmmdl_AddFileProc( FLDMMDL_H_LOAD_FILE *work )
{
	FLDMMDL * fmmdl;
	FIELDSYS_WORK *fsys;
	const FLDMMDL_H *head;
	
	fsys = FLDMMDLSYS_FieldSysWorkGet( work->sys );
	head = work->head;
	
	do{
		if( CheckEventFlag(fsys,head->event_flag) == FALSE ){
			fmmdl = FLDMMDL_AddH( work->sys, head, work->zone );
			GF_ASSERT( fmmdl != NULL && "フィールド動作モデルの一括登録に失敗しました\n" );
		}else{
			OS_Printf( "フィールド動作モデル一括登録中…イベントフラグがFALSEの為、" );
			OS_Printf( "OBJ ID=%d,",FLDMMDL_H_IDGet(head) );
			OS_Printf( "GIRD X=%d,",FLDMMDL_H_PosXGet(head) );
			OS_Printf( "GIRD Z=%d",FLDMMDL_H_PosZGet(head) );
			OS_Printf( "に配置するOBJの追加を無視しました\n" );
		}
	
		head++;
		work->add_count++;
	}while( work->add_count < work->add_max );
	
	GFL_HEAP_FreeMemory( HEAPID_WORLD, work->head );
	GFL_HEAP_FreeMemory( HEAPID_WORLD, work );
}
#endif

//==============================================================================
//	フィールド動作モデル　パーツ
//==============================================================================
//--------------------------------------------------------------
/**
 * フィールド動作モデルの空きを探す
 * @param	sys			FLDMMDLSYS *
 * @retval	FLDMMDL	空きのFLDMMDL*　空きが無い場合はNULL
 */
//--------------------------------------------------------------
static FLDMMDL * fmmdl_SpaceSearch( const FLDMMDLSYS *sys )
{
	int i,max;
	FLDMMDL * fmmdl;
	
	i = 0;
	max = FLDMMDLSYS_OBJMaxGet( sys );
	fmmdl = FLDMMDLSYS_fmmdlWorkGet( sys );
	
	do{
		if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_USE) == 0 ){
			return( fmmdl );
		}
		
		fmmdl++;
		i++;
	}while( i < max );
	
	return( NULL );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル　エイリアスを探す
 * @param	fos			FLDMMDLSYS *
 * @param	obj_id		一致するOBJ ID
 * @param	zone_id		一致するZONE ID
 * @retval	FLDMMDL	一致するFLDMMDL*　一致無し=NULL
 */
//--------------------------------------------------------------
static FLDMMDL * fmmdl_AliesSearch( const FLDMMDLSYS *fos, int obj_id, int zone_id )
{
	int no = 0;
	FLDMMDL * fmmdl;
	
	while( FLDMMDLSYS_FieldOBJSearch(fos,&fmmdl,&no,FLDMMDL_STABIT_USE) == TRUE ){
		if( FLDMMDL_StatusBitCheck_Alies(fmmdl) == TRUE ){
			if( FLDMMDL_OBJIDGet(fmmdl) == obj_id ){
				if( FLDMMDL_ZoneIDGetAlies(fmmdl) == zone_id ){
					return( fmmdl );
				}
			}
		}
	}
	
	return( NULL );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル　TCB動作関数追加
 * @param	sys		FLDMMDLSYS*
 * @param	fmmdl	FLDMMDL*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_MoveTcbAdd( const FLDMMDLSYS *sys, FLDMMDL * fmmdl )
{
	int pri,code;
	GFL_TCB * tcb;
	
	pri = FLDMMDLSYS_TCBStandardPriorityGet( sys );
	code = FLDMMDL_MoveCodeGet( fmmdl );
	
	if( code == MV_PAIR || code == MV_TR_PAIR ){
		pri += FLDMMDL_TCBPRI_OFFS_AFTER;
	}
	
	tcb = GFL_TCB_AddTask(
		FLDMMDLSYS_GetTCBSYS(sys), fmmdl_TCB_MoveProc, fmmdl, pri );
	GF_ASSERT( tcb != NULL && "fmmdl tcb not add error" );
	
	FLDMMDL_MoveTcbPtrSet( fmmdl, tcb );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル　ヘッダー情報反映
 * @param	fmmdl		設定するFLDMMDL * 
 * @param	head		反映する情報を纏めたFLDMMDL_H *
 * @param	fsys		FIELDSYS_WORK *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_AddHeaderSet(
		FLDMMDL * fmmdl, const FLDMMDL_H *head, void *fsys )
{
	FLDMMDL_OBJIDSet( fmmdl, FLDMMDL_H_IDGet(head) );
	FLDMMDL_OBJCodeSet( fmmdl,fmmdl_OBJCodeWkOBJCodeConv(fsys,FLDMMDL_H_OBJCodeGet(head)) );
	FLDMMDL_MoveCodeSet( fmmdl, FLDMMDL_H_MoveCodeGet(head) );
	FLDMMDL_EventTypeSet( fmmdl, FLDMMDL_H_EventTypeGet(head) );
	FLDMMDL_EventFlagSet( fmmdl, FLDMMDL_H_EventFlagGet(head) );
	FLDMMDL_EventIDSet( fmmdl, FLDMMDL_H_EventIDGet(head) );
	FLDMMDL_DirHeaderSet( fmmdl, FLDMMDL_H_DirGet(head) );
	FLDMMDL_ParamSet( fmmdl, FLDMMDL_H_ParamGet(head,FLDMMDL_PARAM_0), FLDMMDL_PARAM_0 );
	FLDMMDL_ParamSet( fmmdl, FLDMMDL_H_ParamGet(head,FLDMMDL_PARAM_1), FLDMMDL_PARAM_1 );
	FLDMMDL_ParamSet( fmmdl, FLDMMDL_H_ParamGet(head,FLDMMDL_PARAM_2), FLDMMDL_PARAM_2 );
	FLDMMDL_MoveLimitXSet( fmmdl, FLDMMDL_H_MoveLimitXGet(head) );
	FLDMMDL_MoveLimitZSet( fmmdl, FLDMMDL_H_MoveLimitZGet(head) );
	
	fmmdl_AddHeaderPosInit( fmmdl, head );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル　座標系初期化
 * @param	fmmdl		FLDMMDL * 
 * @param	head		反映する情報を纏めたFLDMMDL_H *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_AddHeaderPosInit( FLDMMDL * fmmdl, const FLDMMDL_H *head )
{
	int pos;
	VecFx32 vec;
	
	pos = FLDMMDL_H_PosXGet( head );
	vec.x = GRID_SIZE_FX32( pos ) + FLDMMDL_VEC_X_GRID_OFFS_FX32;
	FLDMMDL_InitPosGX_Set( fmmdl, pos );
	FLDMMDL_OldPosGX_Set( fmmdl, pos );
	FLDMMDL_NowPosGX_Set( fmmdl, pos );
	
	pos = FLDMMDL_H_PosYGet( head );							//pos設定はfx32型で来る。
	vec.y = (fx32)pos;
	pos = SIZE_H_GRID_FX32( pos );
	FLDMMDL_InitPosGY_Set( fmmdl, pos );
	FLDMMDL_OldPosGY_Set( fmmdl, pos );
	FLDMMDL_NowPosGY_Set( fmmdl, pos );
	
	pos = FLDMMDL_H_PosZGet( head );
	vec.z = GRID_SIZE_FX32( pos ) + FLDMMDL_VEC_Z_GRID_OFFS_FX32;
	FLDMMDL_InitPosGZ_Set( fmmdl, pos );
	FLDMMDL_OldPosGZ_Set( fmmdl, pos );
	FLDMMDL_NowPosGZ_Set( fmmdl, pos );
	
	FLDMMDL_VecPosSet( fmmdl, &vec );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル　ワーク初期化
 * @param	fmmdl		FLDMMDL * 
 * @param	sys			FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_WorkInit( FLDMMDL * fmmdl, const FLDMMDLSYS *sys )
{
	FLDMMDL_StatusBit_ON( fmmdl,
			FLDMMDL_STABIT_USE |							//使用中
			FLDMMDL_STABIT_HEIGHT_GET_ERROR |				//高さ取得が必要である
			FLDMMDL_STABIT_ATTR_GET_ERROR );				//アトリビュート取得が必要である

#if 0	
	if( FLDMMDL_EventIDAliesCheck(fmmdl) == TRUE ){
		FLDMMDL_StatusBitSet_Alies( fmmdl, TRUE );
	}
#endif	

	FLDMMDL_FieldOBJSysSet( fmmdl, sys );
	FLDMMDL_DirDispSetForce( fmmdl, FLDMMDL_DirHeaderGet(fmmdl) );
	FLDMMDL_DirMoveSet( fmmdl, FLDMMDL_DirHeaderGet(fmmdl) );
	FLDMMDL_AcmdFree( fmmdl );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル 動作関数初期化
 * @param	fmmdl		FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_WorkInit_MoveProcInit( FLDMMDL * fmmdl )
{
	const FLDMMDL_MOVE_PROC_LIST *list;
	
	list = fmmdl_MoveProcListGet( FLDMMDL_MoveCodeGet(fmmdl) );
	FLDMMDL_MoveInitProcSet( fmmdl, fmmdl_MoveProcList_InitGet(list) );
	FLDMMDL_MoveProcSet( fmmdl, fmmdl_MoveProcList_MoveGet(list) );
	FLDMMDL_MoveDeleteProcSet( fmmdl, fmmdl_MoveProcList_DeleteGet(list) );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル 描画関数初期化
 * @param	fmmdl		FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_WorkInit_DrawProcInit( FLDMMDL * fmmdl )
{
	const FLDMMDL_DRAW_PROC_LIST *list;
	u32 code = FLDMMDL_OBJCodeGet( fmmdl );
	
	if( code == NONDRAW ){
		list = &DATA_FieldOBJDraw_Non;
	}else{
		list = &DATA_FieldOBJDraw_Non;
#if 0
		list = fmmdl_DrawProcListGet( code );
#else
		list = &DATA_FieldOBJDraw_Non;
#endif
	}
	
	FLDMMDL_DrawInitProcSet( fmmdl, fmmdl_DrawProcList_InitGet(list) );
	FLDMMDL_DrawProcSet( fmmdl, fmmdl_DrawProcList_DrawGet(list) );
	FLDMMDL_DrawDeleteProcSet( fmmdl, fmmdl_DrawProcList_DeleteGet(list) );
	FLDMMDL_DrawPushProcSet( fmmdl, fmmdl_DrawProcList_PushGet(list) );
	FLDMMDL_DrawPopProcSet( fmmdl, fmmdl_DrawProcList_PopGet(list) );
}

//--------------------------------------------------------------
/**
 * ワーク消去
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_WorkClear( FLDMMDL * fmmdl )
{
	GFL_STD_MemClear( fmmdl, FLDMMDL_SIZE );
}

//--------------------------------------------------------------
/**
 * 指定されたフィールド動作モデルがエイリアス指定かどうかチェック
 * @param	fmmdl		FLDMMDL *
 * @param	h_zone_id	headを読み込むゾーンID
 * @param	max			head要素数
 * @param	head		FLDMMDL_H
 * @retval	int			RET_ALIES_NOT等
 */
//--------------------------------------------------------------
static int fmmdl_HeaderAliesCheck(
		const FLDMMDL * fmmdl, int h_zone_id, int max, const FLDMMDL_H *head )
{
	int obj_id;
	int zone_id;
	
	while( max ){
		obj_id = FLDMMDL_H_IDGet( head );
		
		if( FLDMMDL_OBJIDGet(fmmdl) == obj_id ){					//ID一致
			if( fmmdl_H_AliesCheck(head) == TRUE ){				//エイリアスヘッダー
				zone_id = fmmdl_H_AliesZoneIDGet( head );			//エイリアスの正規ゾーンID
				
				if( FLDMMDL_StatusBitCheck_Alies(fmmdl) == TRUE ){	//対象エイリアス
					if( FLDMMDL_ZoneIDGetAlies(fmmdl) == zone_id ){
						return( RET_ALIES_EXIST );						//Aliesとして既に存在
					}
				}else if( FLDMMDL_ZoneIDGet(fmmdl) == zone_id ){
					return( RET_ALIES_CHANGE );							//Alies対象である
				}
			}else{														//通常ヘッダー
				if( FLDMMDL_StatusBitCheck_Alies(fmmdl) == TRUE ){
					if( FLDMMDL_ZoneIDGetAlies(fmmdl) == h_zone_id ){	//生存エイリアスと一致
						return( RET_ALIES_CHANGE );
					}
				}
			}
		}
		
		max--;
		head++;
	}
	
	return( RET_ALIES_NOT );
}

//--------------------------------------------------------------
/**
 * 指定されたゾーンIDとOBJ IDを持つFLDMMDL *を取得。
 * @param	fos		FLDMMDLSYS *
 * @param	obj_id	OBJ ID
 * @param	zone_id	ゾーンID
 * @retval	FLDMMDL * FLDMMDL *
 */
//--------------------------------------------------------------
static FLDMMDL * fmmdl_OBJIDZoneIDSearch(
		const FLDMMDLSYS *fos, int obj_id, int zone_id )
{
	int no = 0;
	FLDMMDL * fmmdl;
	
	while( FLDMMDLSYS_FieldOBJSearch(fos,&fmmdl,&no,FLDMMDL_STABIT_USE) == TRUE ){
		if( FLDMMDL_OBJIDGet(fmmdl) == obj_id && FLDMMDL_ZoneIDGet(fmmdl) == zone_id ){
			return( fmmdl );
		}
	}
	
	return( NULL );
}

//--------------------------------------------------------------
/**
 * 指定されたゾーンIDを持つもの以外を全て削除
 * @param	fos			FLDMMDLSYS *
 * @param	zone_id		削除しないゾーンID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_DeleteAll_OldZoneID( FLDMMDLSYS *fos, u32 zone_id )
{
	u32 ret;
	int max = FLDMMDLSYS_OBJMaxGet( fos );
	FLDMMDL * fmmdl = FLDMMDLSYS_fmmdlWorkGet( fos );
	
	do{
		if( FLDMMDL_StatusBitCheck_Use(fmmdl) == TRUE ){
			if( FLDMMDL_ZoneIDGet(fmmdl) != zone_id ){
				if( FLDMMDL_StatusBit_CheckEasy(fmmdl,FLDMMDL_STABIT_ZONE_DEL_NOT) == FALSE ){
					FLDMMDL_Delete( fmmdl );
				}
			}
		}
		
		fmmdl++;
		max--;
	}while( max );
}

//--------------------------------------------------------------
/**
 * OBJ IDに一致するFLDMMDL *を検索
 * @param	fos		FLDMMDLSYS *
 * @param	id		検索するOBJ ID
 * @retval	FLDMMDL *	NULL=存在しない
 */
//--------------------------------------------------------------
FLDMMDL * FLDMMDLSYS_OBJIDSearch( const FLDMMDLSYS *fos, int id )
{
	int max;
	FLDMMDL * fmmdl;
	
	GF_ASSERT( fos != NULL && "FLDMMDLSYS_OBJIDSearch() NULL指定です" );
	
	max = FLDMMDLSYS_OBJMaxGet( fos );
	fmmdl = fmmdlsys_FieldOBJWorkGet( fos );
	
	do{
		if( FLDMMDL_StatusBit_CheckEasy(fmmdl,FLDMMDL_STABIT_USE) == TRUE ){
			if( FLDMMDL_StatusBitCheck_Alies(fmmdl) == FALSE ){
				if( FLDMMDL_OBJIDGet(fmmdl) == id ){
					return( fmmdl );
				}
			}
		}
		
		fmmdl++;
		max--;
	}while( max > 0 );
	
	return( NULL );
}

//--------------------------------------------------------------
/**
 * 動作コードに一致するFLDMMDL *を検索
 * @param	fos		FLDMMDLSYS *
 * @param	mv_code		検索する動作コード
 * @retval	FLDMMDL *	NULL=存在しない
 */
//--------------------------------------------------------------
FLDMMDL * FLDMMDLSYS_MoveCodeSearch( const FLDMMDLSYS *fos, int mv_code )
{
	int max;
	FLDMMDL * fmmdl;
	
	max = FLDMMDLSYS_OBJMaxGet( fos );
	fmmdl = fmmdlsys_FieldOBJWorkGet( fos );
	
	do{
		if( FLDMMDL_StatusBit_CheckEasy(fmmdl,FLDMMDL_STABIT_USE) == TRUE ){
			if( FLDMMDL_MoveCodeGet(fmmdl) == mv_code ){
				return( fmmdl );
			}
		}
		
		fmmdl++;
		max--;
	}while( max > 0 );
	
	return( NULL );
}

FLDMMDL * FLDMMDLSYS_OBJIDMoveCodeSearch( const FLDMMDLSYS *fos, int o, int m )
{
	return( NULL );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルワーク検索
 * @param	fos			FLDMMDLSYS *
 * @param	fmmdl		該当するFLDMMDL *格納先
 * @param	no			検索開始ワークno。この関数から戻る際、検索位置+1の値になる
 * @param	check_bit	検索条件ステータスビット。FLDMMDL_STABIT_USE等
 * @retval	int			TRUE=一致あり FALSE=一致なし
 */
//--------------------------------------------------------------
int FLDMMDLSYS_FieldOBJSearch(
	const FLDMMDLSYS *fos, FLDMMDL ** fmmdl, int *no, u32 check_bit )
{
	int max;
	FLDMMDL * check_obj;
	
	max = FLDMMDLSYS_OBJMaxGet( fos );
	
	if( (*no) >= max ){
		return( FALSE );
	}
	
	check_obj = fmmdlsys_FieldOBJWorkGet( fos );
	check_obj = &check_obj[(*no)];
	
	do{
		(*no)++;
		
		if( FLDMMDL_StatusBit_Check(check_obj,check_bit) == check_bit ){
			*fmmdl = check_obj;
			return( TRUE );
		}
		
		check_obj++;
	}while( (*no) < max );
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル描画初期化に行う処理
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_DrawInitStatusSet( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_MOVE_START );
	fmmdl_DrawEffectFlagInit( fmmdl );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル関連エフェクトのフラグ初期化。
 * エフェクト関連のフラグ初期化をまとめる
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_DrawEffectFlagInit( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBit_OFF( fmmdl,
		FLDMMDL_STABIT_SHADOW_SET		|
		FLDMMDL_STABIT_SHADOW_VANISH	|
		FLDMMDL_STABIT_EFFSET_SHOAL		|
		FLDMMDL_STABIT_REFLECT_SET );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル描画削除時に行う処理
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_DrawDeleteStatusSet( FLDMMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 指定されたOBJコードがワーク参照型ならばワーク内OBJコードに変更。
 * 違う場合はそのままで返す。
 * @param	fsys	FIELDSYS_WORK *
 * @param	code	OBJコード。HERO等
 * @retval	int		チェックされたOBJコード
 */
//--------------------------------------------------------------
static int fmmdl_OBJCodeWkOBJCodeConv( void *fsys, int code )
{
	if( code >= WKOBJCODE_ORG && code <= WKOBJCODE_END ){
		#if 0
		code -= WKOBJCODE_ORG;
		code = GetEvDefineObjCode( fsys, code );
		#else
		GF_ASSERT( 0 );
		#endif
	}
	
	return( code );
}

//--------------------------------------------------------------
/**
 * 現在発生しているフィールド動作モデルのOBJコードを参照
 * @param	fmmdl		FLDMMDL * 
 * @param	code		チェックするコード。HERO等
 * @retval	int			TRUE=fmmdl以外にもcodeを持っている奴がいる
 */
//--------------------------------------------------------------
int FLDMMDL_OBJCodeUseSearch( const FLDMMDL * fmmdl, int code )
{
	const FLDMMDLSYS *fos = FLDMMDL_FieldOBJSysGet( fmmdl );
	int check_code,i = FLDMMDLSYS_OBJMaxGet( fos );
	const FLDMMDL * fieldobj = FLDMMDLSYS_FieldOBJWorkGet( fos );
	
	do{
		if( fieldobj != fmmdl ){
			if( FLDMMDL_StatusBitCheck_Use(fieldobj) == TRUE ){
				check_code = FLDMMDL_OBJCodeGet( fieldobj );
				#if 0		
				if( FLDMMDL_OBJCodeSeedCheck(check_code) == TRUE ){
					check_code = FLDMMDL_OBJCodeSeedGet( fieldobj );
				}
				#endif
				if( check_code != OBJCODEMAX && check_code == code ){
					return( TRUE );
				}
			}
		}
		
		FLDMMDLSYS_FieldOBJWorkInc( &fieldobj );
		i--;
	}while( i );
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * 高さ初期化 FLDMMDL_STABIT_HEIGHT_GET_ERRORがOFFの場合は初期化しない
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_HeightNeedCheckInit( FLDMMDL * fmmdl )
{
	if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_HEIGHT_GET_ERROR) ){
		FLDMMDL_VecPosNowHeightGetSet( fmmdl );
	}
}

//--------------------------------------------------------------
/**
 * 動作初期化に行う処理纏め
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_MoveWorkInit( FLDMMDL * fmmdl )
{
	fmmdl_WorkInit_MoveProcInit( fmmdl );
	FLDMMDL_MoveInit( fmmdl );
}

//--------------------------------------------------------------
/**
 * 描画初期化に行う処理纏め
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_DrawWorkInit( FLDMMDL * fmmdl )
{
	const FLDMMDLSYS *fos = FLDMMDL_FieldOBJSysGet( fmmdl );
	
	if( FLDMMDLSYS_DrawInitCompCheck(fos) == FALSE ){
		return; //描画システム初期化完了していない
	}
	
	fmmdl_HeightNeedCheckInit( fmmdl );
	FLDMMDL_DrawStatusSet( fmmdl, 0 );
#if 0
	FLDMMDL_BlActAddPracFlagSet( fmmdl, FALSE );
#endif	
	if( FLDMMDL_StatusBitCheck_DrawProcInitComp(fmmdl) == FALSE ){
		fmmdl_WorkInit_DrawProcInit( fmmdl );
		FLDMMDL_DrawInitProcCall( fmmdl );
		FLDMMDL_StatusBitON_DrawProcInitComp( fmmdl );
	}
}

//--------------------------------------------------------------
/**
 * イベントIDがエイリアスかどうかチェック
 * @param	fmmdl		FLDMMDL * 
 * @retval	int			TRUE=エイリアスIDである FALSE=違う
 */
//--------------------------------------------------------------
int FLDMMDL_EventIDAliesCheck( const FLDMMDL * fmmdl )
{
#if 0
	u16 id = (u16)FLDMMDL_EventIDGet( fmmdl );
	
	if( id == SP_SCRID_ALIES ){
		return( TRUE );
	}
#else
	GF_ASSERT( 0 );
#endif
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル　エイリアスから正規OBJへの変更
 * @param	fmmdl		FLDMMDL * 
 * @param	head		対象のFLDMMDL_H
 * @param	zone_id		正規のゾーンID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_AliesOBJChange( FLDMMDL * fmmdl, const FLDMMDL_H *head, int zone_id )
{
	GF_ASSERT( FLDMMDL_StatusBitCheck_Alies(fmmdl) == TRUE &&
			"fmmdl_AliesOBJChange()aliesではない" );
	
	FLDMMDL_StatusBitSet_Alies( fmmdl, FALSE );
	FLDMMDL_ZoneIDSet( fmmdl, zone_id );
	FLDMMDL_EventIDSet( fmmdl, FLDMMDL_H_EventIDGet(head) );
	FLDMMDL_EventFlagSet( fmmdl, FLDMMDL_H_EventFlagGet(head) );
}	

//--------------------------------------------------------------
/**
 * フィールド動作モデル　正規OBJからエイリアスへの変更
 * @param	fmmdl		FLDMMDL * 
 * @param	head		対象のFLDMMDL_H
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_OBJAliesChange( FLDMMDL * fmmdl, int zone_id, const FLDMMDL_H *head )
{
	GF_ASSERT( fmmdl_H_AliesCheck(head) == TRUE && "fmmdl_OBJAliesChange()aliesではない" );
	
	FLDMMDL_StatusBitSet_Alies( fmmdl, TRUE );
	FLDMMDL_EventIDSet( fmmdl, FLDMMDL_H_EventIDGet(head) );
	FLDMMDL_EventFlagSet( fmmdl, fmmdl_H_AliesZoneIDGet(head) );
	FLDMMDL_ZoneIDSet( fmmdl, zone_id );
}

//--------------------------------------------------------------
/**
 * FLDMMDL *のTCBスタンダードプライオリティを調整して取得
 * @param	fmmdl	FLDMMDL *
 * @param	offset	スタンダードプライオリティに加算する値
 * @retval	int		調節されたプライオリティ
 */
//--------------------------------------------------------------
int FLDMMDL_TCBPriGet( const FLDMMDL * fmmdl, int offset )
{
	int pri;
	
	pri = FLDMMDL_TCBStandardPriorityGet( fmmdl );
	pri += offset;
	
	return( pri );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルの同一チェック。
 * 死亡、入れ替わりが発生しているかチェックする。
 * @param	fmmdl	FLDMMDL *
 * @param	obj_id	同一とみなすOBJ ID
 * @param	zone_id	同一とみなすZONE ID
 * @retval	int		TRUE=同一。FALSE=同一ではない
 */
//--------------------------------------------------------------
int FLDMMDL_CheckSameID( const FLDMMDL * fmmdl, int obj_id, int zone_id )
{
	if( FLDMMDL_StatusBit_CheckEasy(fmmdl,FLDMMDL_STABIT_USE) == FALSE ){
		return( FALSE );
	}
	
	if( FLDMMDL_OBJIDGet(fmmdl) != obj_id ){
		return( FALSE );
	}
	
	if( FLDMMDL_ZoneIDGet(fmmdl) != zone_id ){
		if( FLDMMDL_StatusBitCheck_Alies(fmmdl) == FALSE ){
			return( FALSE );
		}
		
		if( FLDMMDL_ZoneIDGetAlies(fmmdl) != zone_id ){
			return( FALSE );
		}
	}
	
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルの同一チェック。OBJコード含み
 * 死亡、入れ替わりが発生しているかチェックする。
 * @param	fmmdl	FLDMMDL *
 * @param	code	同一とみなすOBJコード
 * @param	obj_id	同一とみなすOBJ ID
 * @param	zone_id	同一とみなすZONE ID
 * @retval	int		TRUE=同一。FALSE=同一ではない
 */
//--------------------------------------------------------------
int FLDMMDL_CheckSameIDOBJCodeIn(
		const FLDMMDL * fmmdl, int code, int obj_id, int zone_id )
{
	if( FLDMMDL_StatusBit_CheckEasy(fmmdl,FLDMMDL_STABIT_USE) == FALSE ){
		return( FALSE );
	}
	
	{
		int ret = FLDMMDL_OBJCodeGetSeedIn( fmmdl );
		
		if( ret != code ){
			return( FALSE );
		}
	}
	
	return( FLDMMDL_CheckSameID(fmmdl,obj_id,zone_id) );
}

//==============================================================================
//	動作関数
//==============================================================================
//--------------------------------------------------------------
/**
 * フィールド動作モデル TCB 動作関数
 * @param	tcb		GFL_TCB *
 * @param	work	tcb work
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_TCB_MoveProc( GFL_TCB * tcb, void *work )
{
	FLDMMDL * fmmdl = (FLDMMDL *)work;
	
	FLDMMDL_Move( fmmdl );
	
	if( FLDMMDL_StatusBitCheck_Use(fmmdl) == FALSE ){
		return;
	}
	
//	fmmdl_TCB_DrawProc( fmmdl );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル TCB 動作関数から呼ばれる描画関数
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdl_TCB_DrawProc( FLDMMDL * fmmdl )
{
	const FLDMMDLSYS *fos = FLDMMDL_FieldOBJSysGet(fmmdl);
	
	if( FLDMMDLSYS_DrawInitCompCheck(fos) == TRUE ){
		FLDMMDL_Draw( fmmdl );
	}
}

//==============================================================================
//	フィールド動作モデルシステム　参照
//==============================================================================
//--------------------------------------------------------------
/**
 * const FLDMMDLSYSをFLDMMDLSYSに
 * @param	sys		FLDMMDLSYS *
 * @retval	FLDMMDLSYS	FLDMMDLSYS *
 */
//--------------------------------------------------------------
static FLDMMDLSYS * fmmdl_ConstSysLocalGet( const FLDMMDLSYS *sys )
{
	return( (FLDMMDLSYS *)sys );
}

//--------------------------------------------------------------
/**
 * OBJ登録可能最大数をセット
 * @param	sys		FLDMMDLSYS *
 * @param	max		最大数
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_OBJMaxSet( FLDMMDLSYS *sys, int max )
{
	sys->fmmdl_max = max;
}

//--------------------------------------------------------------
/**
 * OBJ登録可能最大数を取得
 * @param	sys		FLDMMDLSYS *
 * @retval	int		OBJ最大数
 */
//--------------------------------------------------------------
int FLDMMDLSYS_OBJMaxGet( const FLDMMDLSYS *sys )
{
	return( sys->fmmdl_max );
}

//--------------------------------------------------------------
/**
 * 現在存在しているOBJの数を1増加
 * @param	sys		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdlsys_OBJCountInc( FLDMMDLSYS *sys )
{
	sys->fmmdl_count++;
#ifdef DEBUG_PRINT_FLDMMDL_COUNT
	OS_Printf( "FIELD OBJ ADD, COUNT=0x%x\n", sys->fmmdl_count );
#endif
}

//--------------------------------------------------------------
/**
 * 現在存在しているOBJの数を1減らす
 * @param	sys		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdlsys_OBJCountDec( FLDMMDLSYS *sys )
{
	sys->fmmdl_count--;
#ifdef DEBUG_PRINT_FLDMMDL_COUNT
	OS_Printf( "FIELD OBJ DEL, COUNT=0x%x\n", sys->fmmdl_count );
#endif
}

//--------------------------------------------------------------
/**
 * 現在存在しているOBJの数をセット
 * @param	sys		FLDMMDLSYS *
 * @param	count	セットする数
 * @retval	nothing
 */
//--------------------------------------------------------------
static void fmmdlsys_OBJCountSet( FLDMMDLSYS *sys, int count )
{
	sys->fmmdl_count = count;
}

//--------------------------------------------------------------
/**
 * 現在存在しているOBJの数を取得
 * @param	fos		FLDMMDLSYS *
 * @retval	int		現在存在しているOBJの数
 */
//--------------------------------------------------------------
int FLDMMDLSYS_OBJCountGet( const FLDMMDLSYS *fos )
{
	return( fos->fmmdl_count );
}

//--------------------------------------------------------------
/**
 * システム　ステータスビット初期化
 * @param	sys		FLDMMDLSYS *
 * @param	bit		初期化値。FLDMMDLSYS_STABIT_NON等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_StatusBit_Init( FLDMMDLSYS *sys, u32 bit )
{
	sys->status_bit = bit;
}

//--------------------------------------------------------------
/**
 * システム　ステータスビット ON
 * @param	sys		FLDMMDLSYS *
 * @param	bit		ONにするビット。FLDMMDLSYS_STABIT_NON等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_StatusBit_ON( FLDMMDLSYS *sys, u32 bit )
{
	sys->status_bit |= bit;
}

//--------------------------------------------------------------
/**
 * システム　ステータスビット OFF
 * @param	sys		FLDMMDLSYS *
 * @param	bit		OFFにするビット。FLDMMDLSYS_STABIT_NON等
 * @retval	int		追加されているOBJ数
 */
//--------------------------------------------------------------
void FLDMMDLSYS_StatusBit_OFF( FLDMMDLSYS *sys, u32 bit )
{
	sys->status_bit &= ~bit;
}

//--------------------------------------------------------------
/**
 * システム　ステータスビット チェック
 * 戻り値はステータスビットとチェックするビットをandした値が返る。
 * 例：対象 FIDOBJSYS_STR_BIT_DRAW_INIT_COMP以外はOFF。
 * チェックするビットはFLDMMDLSYS_STABIT_DRAW_INIT_COMP|FLDMMDLSYS_STABIT_MOVE_PROC_STOP
 * 戻り値はFLDMMDLSYS_STABIT_DRAW_PROC_INITが返る。
 * @param	sys				FLDMMDLSYS *
 * @param	bit				チェックするビット。FLDMMDL_STABIT_USE等
 * @retval	u32				チェック後のビット
 */
//--------------------------------------------------------------
u32 FLDMMDLSYS_StatusBit_Check( const FLDMMDLSYS *sys, u32 bit )
{
	return( (sys->status_bit&bit) );
}

//--------------------------------------------------------------
/**
 * TCB基本プライオリティをセット
 * @param	sys				FLDMMDLSYS *
 * @param	pri				TCB基本プライオリティ
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_TCBStandardPrioritySet( FLDMMDLSYS *sys, int pri )
{
	sys->tcb_pri = pri;
}

//--------------------------------------------------------------
/**
 * TCB基本プライオリティ取得
 * @param	sys			FLDMMDLSYS *
 * @retval	u32			TCBプライオリティ
 */
//--------------------------------------------------------------
int FLDMMDLSYS_TCBStandardPriorityGet( const FLDMMDLSYS *sys )
{
	return( sys->tcb_pri );
}

//--------------------------------------------------------------
/**
 * FLDMMDL_BLACT_CONT *取得
 * @param	sys		FLDMMDLSYS *
 * @retval	FLDMMDL_BLACT_CONT	FLDMMDL_BLACT_CONT *
 */
//--------------------------------------------------------------
FLDMMDL_BLACT_CONT * FLDMMDLSYS_BlActContGet( const FLDMMDLSYS *sys )
{
#if 0
	return( &(((FLDMMDLSYS *)sys)->blact_cont) );
#else
	GF_ASSERT( 0 );
	return( NULL );
#endif
}

//--------------------------------------------------------------
/**
 * FLDMMDL_RENDER_CONT_PTRセット
 * @param	sys		FLDMMDLSYS *
 * @param	render	FLDMMDL_RENDER_CONT_PTR
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_RenderContSet( FLDMMDLSYS *sys, FLDMMDL_RENDER_CONT_PTR render )
{
#if 0
	sys->render_cont = render;
#else
	GF_ASSERT( 0 );
#endif
}

//--------------------------------------------------------------
/**
 * FLDMMDL_RENDER_CONT_PTR 取得
 * @param	sys		FLDMMDLSYS *
 * @retval	FLDMMDL_RENDER_CONT_PTR FLDMMDL_RENDER_CONT_PTR
 */
//--------------------------------------------------------------
FLDMMDL_RENDER_CONT_PTR FLDMMDLSYS_RenderContGet( const FLDMMDLSYS *sys )
{
#if 0
	return( sys->render_cont );
#else
	GF_ASSERT( 0 );
	return( NULL );
#endif
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルワークをセット
 * @param	sys		FLDMMDLSYS *
 * @param	ptr		FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_FieldOBJWorkSet( FLDMMDLSYS *sys, FLDMMDL * ptr )
{
	sys->fmmdl_work = ptr;
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルワーク取得
 * @param	sys			FLDMMDLSYS *
 * @retval	FLDMMDL	FLDMMDL * 
 */
//--------------------------------------------------------------
const FLDMMDL * FLDMMDLSYS_FieldOBJWorkGet( const FLDMMDLSYS *sys )
{
	return( sys->fmmdl_work );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルワーク取得　ローカル
 * @param	sys			FLDMMDLSYS *
 * @retval	FLDMMDL	FLDMMDL * 
 */
//--------------------------------------------------------------
static FLDMMDL * fmmdlsys_FieldOBJWorkGet( const FLDMMDLSYS *sys )
{
	return( sys->fmmdl_work );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルワーク取得　FIELD OBJローカル用
 * const無し。フィールド動作モデル以外は使用禁止
 * @param	sys			FLDMMDLSYS *
 * @retval	FLDMMDL	FLDMMDL * 
 */
//--------------------------------------------------------------
FLDMMDL * FLDMMDLSYS_fmmdlWorkGet( const FLDMMDLSYS *sys )
{
	return( sys->fmmdl_work );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルワーク進行。
 * FLDMMDLSYS_FieldOBJWorkGet()で取得したポインタを１つ進める。
 * @param	FLDMMDL	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_FieldOBJWorkInc( const FLDMMDL ** fmmdl )
{
	(*fmmdl)++;
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルワーク進行。非const
 * FLDMMDLSYS_FieldOBJWorkGet()で取得したポインタを１つ進める。
 * @param	FLDMMDL	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_fmmdlWorkInc( FLDMMDL ** fmmdl )
{
	(*fmmdl)++;
}

//--------------------------------------------------------------
/**
 * FIELDSYS_WORK *セット
 * @param	fos		FLDMMDLSYS *
 * @param	fsys	FIELDSYS_WORK *
 * @retval	nothing
 */
//--------------------------------------------------------------
#if 0
void FLDMMDLSYS_FieldSysWorkSet( FLDMMDLSYS *fos, void *fsys )
{
	fos->fieldsys = fsys;
}
#endif

//--------------------------------------------------------------
/**
 * FIELDSYS_WORK *取得
 * @param	fos		FLDMMDLSYS *
 * @retval	FIELDSYS_WORK	FIELDSYS_WORK *
 */
//--------------------------------------------------------------
#if 0
void * FLDMMDLSYS_FieldSysWorkGet( const FLDMMDLSYS *fos )
{
	return( fos->fieldsys );
}
#endif

//--------------------------------------------------------------
/**
 * アーカイブハンドルセット
 * @param	fos		FLDMMDLSYS *
 * @param	handle	ARCHANDLE *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_ArcHandleSet( FLDMMDLSYS *fos, ARCHANDLE *handle )
{
	fos->archandle = handle;
}

//--------------------------------------------------------------
/**
 * アーカイブハンドル取得
 * @param	fos		FLDMMDLSYS *
 * @retval	ARCHANDLE*　ARCHANDLE *
 */
//--------------------------------------------------------------
ARCHANDLE * FLDMMDLSYS_ArcHandleGet( const FLDMMDLSYS *fos )
{
	GF_ASSERT( fos->archandle != NULL && "FLDMMDLSYS_ArcHandleGet()ハンドル無し" );
	return( ((FLDMMDLSYS *)fos)->archandle );
}

//==============================================================================
//	フィールド動作モデル　参照
//==============================================================================
//--------------------------------------------------------------
/**
 * ステータスビット初期化
 * @param	fmmdl			FLDMMDL * 
 * @param	bit				初期化値。FLDMMDL_STABIT_USE等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBit_Init( FLDMMDL * fmmdl, u32 bit )
{
	fmmdl->status_bit = bit;
}

//--------------------------------------------------------------
/**
 * ステータスビット取得
 * @param	fmmdl			FLDMMDL * 
 * @retval	u32				ステータスビット
 */
//--------------------------------------------------------------
u32 FLDMMDL_StatusBit_Get( const FLDMMDL * fmmdl )
{
	return( fmmdl->status_bit );
}

//--------------------------------------------------------------
/**
 * ステータスビット ON
 * @param	fmmdl			FLDMMDL * 
 * @param	bit				ONにしたいビット。FLDMMDL_STABIT_USE等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBit_ON( FLDMMDL * fmmdl, u32 bit )
{
	fmmdl->status_bit |= bit;
}

//--------------------------------------------------------------
/**
 * ステータスビット OFF
 * @param	fmmdl			FLDMMDL * 
 * @param	bit				OFFにしたいビット。FLDMMDL_STABIT_USE等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBit_OFF( FLDMMDL * fmmdl, u32 bit )
{
	fmmdl->status_bit &= ~bit;
}

//--------------------------------------------------------------
/**
 * ステータスビット チェック
 * 戻り値はステータスビットとチェックするビットをandした値が返る。
 * 
 * 例：対象OBJのステータスビットはFIDOBJ_STR_BIT_USE以外はOFF。
 * チェックするビットはFLDMMDL_STABIT_USE|FLDMMDL_STABIT_MOVE_START。
 * 戻り値はFLDMMDL_STABIT_USEが返る。
 * 
 * @param	fmmdl			FLDMMDL * 
 * @param	bit				チェックするビット。FLDMMDL_STABIT_USE等
 * @retval	u32				チェック後のビット
 */
//--------------------------------------------------------------
u32 FLDMMDL_StatusBit_Check( const FLDMMDL * fmmdl, u32 bit )
{
	return( fmmdl->status_bit & bit );
}

//--------------------------------------------------------------
/**
 * ステータスビット 簡易チェック
 * @param	fmmdl			FLDMMDL * 
 * @param	bit				チェックするビット。FLDMMDL_STABIT_USE等
 * @retval	u32				TRUE=bitのどれかが該当。FALSE=該当無し
 */
//--------------------------------------------------------------
u32 FLDMMDL_StatusBit_CheckEasy( const FLDMMDL * fmmdl, u32 bit )
{
	if( (fmmdl->status_bit&bit) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * 動作ビット　初期化
 * @param	fmmdl	FLDMMDL *
 * @param	bit		初期化するビット FLDMMDL_MOVEBIT_NON等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveBit_Init( FLDMMDL * fmmdl, u32 bit )
{
	fmmdl->move_bit = bit;
}

//--------------------------------------------------------------
/**
 * 動作ビット　取得
 * @param	fmmdl	FLDMMDL *
 * @retval	u32		動作ビット
 */
//--------------------------------------------------------------
u32 FLDMMDL_MoveBit_Get( const FLDMMDL * fmmdl )
{
	return( fmmdl->move_bit );
}

//--------------------------------------------------------------
/**
 * 動作ビット　ON
 * @param	fmmdl	FLDMMDL *
 * @param	bit		立てるビット FLDMMDL_MOVEBIT_NON等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveBit_ON( FLDMMDL * fmmdl, u32 bit )
{
	fmmdl->move_bit |= bit;
}

//--------------------------------------------------------------
/**
 * 動作ビット　OFF
 * @param	fmmdl	FLDMMDL *
 * @param	bit		下ろすビット FLDMMDL_MOVEBIT_NON等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveBit_OFF( FLDMMDL * fmmdl, u32 bit )
{
	fmmdl->move_bit &= ~bit;
}

//--------------------------------------------------------------
/**
 * 動作ビット　チェック
 * @param	fmmdl	FLDMMDL *
 * @param	bit		チェックするビット FLDMMDL_MOVEBIT_NON等
 * @retval	nothing
 */
//--------------------------------------------------------------
u32 FLDMMDL_MoveBit_Check( const FLDMMDL * fmmdl, u32 bit )
{
	return( (fmmdl->move_bit & bit) );
}

//--------------------------------------------------------------
/**
 * OBJ IDセット
 * @param	fmmdl	FLDMMDL * 
 * @param	id		obj id
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OBJIDSet( FLDMMDL * fmmdl, u32 obj_id )
{
	fmmdl->obj_id = obj_id;
}

//--------------------------------------------------------------
/**
 * OBJ ID取得
 * @param	fmmdl	FLDMMDL *
 * @retval	u32		OBJ ID
 */
//--------------------------------------------------------------
u32 FLDMMDL_OBJIDGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->obj_id );
}

//--------------------------------------------------------------
/**
 * ZONE IDセット
 * @param	fmmdl	FLDMMDL *
 * @param	zone_id	ゾーンID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_ZoneIDSet( FLDMMDL * fmmdl, int zone_id )
{
	fmmdl->zone_id = zone_id;
}

//--------------------------------------------------------------
/**
 * ZONE ID取得
 * @param	fmmdl	FLDMMDL *
 * @retval	int		ZONE ID
 */
//--------------------------------------------------------------
int FLDMMDL_ZoneIDGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->zone_id );
}

//--------------------------------------------------------------
/**
 * OBJコードセット
 * @param	fmmdl			FLDMMDL * 
 * @param	code			セットするコード
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OBJCodeSet( FLDMMDL * fmmdl, u32 code )
{
	fmmdl->obj_code = code;
}

//--------------------------------------------------------------
/**
 * OBJコード取得
 * @param	fmmdl			FLDMMDL * 
 * @retval	u32				OBJコード
 */
//--------------------------------------------------------------
u32 FLDMMDL_OBJCodeGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->obj_code );
}

//--------------------------------------------------------------
/**
 * OBJコード取得　種チェックあり
 * @param	fmmdl			FLDMMDL * 
 * @retval	u32				OBJコード
 */
//--------------------------------------------------------------
u32 FLDMMDL_OBJCodeGetSeedIn( const FLDMMDL * fmmdl )
{
	u32 code = FLDMMDL_OBJCodeGet( fmmdl );
#if 0	
	if( FLDMMDL_OBJCodeSeedCheck(code) == TRUE ){
		code = FLDMMDL_OBJCodeSeedGet( fmmdl );
	}
#endif
	return( code );
}

//--------------------------------------------------------------
/**
 * 動作コードセット
 * @param	fmmdl			FLDMMDL * 
 * @param	code			動作コード
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveCodeSet( FLDMMDL * fmmdl, u32 code )
{
	fmmdl->move_code = code;
}

//--------------------------------------------------------------
/**
 * 動作コード取得
 * @param	fmmdl			FLDMMDL * 
 * @retval	u32				動作コード
 */
//--------------------------------------------------------------
u32 FLDMMDL_MoveCodeGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->move_code );
}

//--------------------------------------------------------------
/**
 * イベントタイプセット
 * @param	fmmdl		FLDMMDL * 
 * @param	type		Event Type
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_EventTypeSet( FLDMMDL * fmmdl, u32 type )
{
	fmmdl->event_type = type;
}

//--------------------------------------------------------------
/**
 * イベントタイプ取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	u32			Event Type
 */
//--------------------------------------------------------------
u32 FLDMMDL_EventTypeGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->event_type );
}

//--------------------------------------------------------------
/**
 * イベントフラグセット
 * @param	fmmdl		FLDMMDL * 
 * @param	flag		Event Flag
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_EventFlagSet( FLDMMDL * fmmdl, u32 flag )
{
	fmmdl->event_flag = flag;
}

//--------------------------------------------------------------
/**
 * イベントフラグ取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	u32			Event Flag
 */
//--------------------------------------------------------------
u32 FLDMMDL_EventFlagGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->event_flag );
}

//--------------------------------------------------------------
/**
 * イベントIDセット
 * @param	fmmdl		FLDMMDL * 
 * @param	id			Event ID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_EventIDSet( FLDMMDL * fmmdl, u32 id )
{
	fmmdl->event_id = id;
}

//--------------------------------------------------------------
/**
 * イベントID取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	u32			Event ID
 */
//--------------------------------------------------------------
u32 FLDMMDL_EventIDGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->event_id );
}

//--------------------------------------------------------------
/**
 * ヘッダー指定方向セット
 * @param	fmmdl		FLDMMDL * 
 * @param	dir			DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DirHeaderSet( FLDMMDL * fmmdl, int dir )
{
	fmmdl->dir_head = dir;
}

//--------------------------------------------------------------
/**
 * ヘッダー指定方向取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	u32			DIR_UP等
 */
//--------------------------------------------------------------
u32 FLDMMDL_DirHeaderGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->dir_head );
}

//--------------------------------------------------------------
/**
 * 表示方向セット　強制
 * @param	fmmdl			FLDMMDL * 
 * @param	dir				DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DirDispSetForce( FLDMMDL * fmmdl, int dir )
{
	fmmdl->dir_disp_old = fmmdl->dir_disp;
	fmmdl->dir_disp = dir;
}

//--------------------------------------------------------------
/**
 * 表示方向セット　FLDMMDL_STABIT_PAUSE_DIRチェックあり
 * @param	fmmdl			FLDMMDL * 
 * @param	dir				DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DirDispCheckSet( FLDMMDL * fmmdl, int dir )
{
	if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_PAUSE_DIR) == 0 ){
		fmmdl->dir_disp_old = fmmdl->dir_disp;
		fmmdl->dir_disp = dir;
	}
}

//--------------------------------------------------------------
/**
 * 表示方向取得
 * @param	fmmdl			FLDMMDL * 
 * @param	dir				DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
int FLDMMDL_DirDispGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->dir_disp );
}

//--------------------------------------------------------------
/**
 * 過去表示方向取得
 * @param	fmmdl			FLDMMDL * 
 * @param	dir				DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
int FLDMMDL_DirDispOldGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->dir_disp_old );
}

//--------------------------------------------------------------
/**
 * 移動方向セット
 * @param	fmmdl			FLDMMDL * 
 * @param	dir				DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DirMoveSet( FLDMMDL * fmmdl, int dir )
{
	fmmdl->dir_move_old = fmmdl->dir_move;
	fmmdl->dir_move = dir;
}

//--------------------------------------------------------------
/**
 * 移動方向取得
 * @param	fmmdl			FLDMMDL * 
 * @retval	dir				DIR_UP等
 */
//--------------------------------------------------------------
int FLDMMDL_DirMoveGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->dir_move );
}

//--------------------------------------------------------------
/**
 * 過去移動方向取得
 * @param	fmmdl			FLDMMDL * 
 * @retval	dir				DIR_UP等
 */
//--------------------------------------------------------------
int FLDMMDL_DirMoveOldGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->dir_move_old );
}

//--------------------------------------------------------------
/**
 * 表示、移動方向セット　FLDMMDL_STABIT_PAUSE_DIRチェックあり
 * @param	fmmdl			FLDMMDL * 
 * @param	dir				DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DirMoveDispCheckSet( FLDMMDL * fmmdl, int dir )
{
	FLDMMDL_DirDispCheckSet( fmmdl, dir );
	FLDMMDL_DirMoveSet( fmmdl, dir );
}

//--------------------------------------------------------------
/**
 * ヘッダー指定パラメタセット
 * @param	fmmdl			FLDMMDL * 
 * @param	param			パラメタ
 * @param	no				セットするパラメタ番号　FLDMMDL_PARAM_0等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_ParamSet( FLDMMDL * fmmdl, int param, FLDMMDL_H_PARAM no )
{
	switch( no ){
	case FLDMMDL_PARAM_0: fmmdl->param0 = param; break;
	case FLDMMDL_PARAM_1: fmmdl->param1 = param; break;
	case FLDMMDL_PARAM_2: fmmdl->param2 = param; break;
	default: GF_ASSERT( 0 && "FLDMMDL_ParamSet()不正な引数" );
	}
}

//--------------------------------------------------------------
/**
 * ヘッダー指定パラメタ取得
 * @param	fmmdl			FLDMMDL *
 * @param	param			FLDMMDL_PARAM_0等
 * @retval	int				パラメタ
 */
//--------------------------------------------------------------
int FLDMMDL_ParamGet( const FLDMMDL * fmmdl, FLDMMDL_H_PARAM param )
{
	switch( param ){
	case FLDMMDL_PARAM_0: return( fmmdl->param0 );
	case FLDMMDL_PARAM_1: return( fmmdl->param1 );
	case FLDMMDL_PARAM_2: return( fmmdl->param2 );
	}
	
	GF_ASSERT( 0 && "FLDMMDL_ParamGet()不正な引数" );
	return( 0 );
}

//--------------------------------------------------------------
/**
 * 移動制限Xセット
 * @param	fmmdl			FLDMMDL * 
 * @param	x				移動制限
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveLimitXSet( FLDMMDL * fmmdl, int x )
{
	fmmdl->move_limit_x = x;
}

//--------------------------------------------------------------
/**
 * 移動制限X取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	int			移動制限X
 */
//--------------------------------------------------------------
int FLDMMDL_MoveLimitXGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->move_limit_x );
}

//--------------------------------------------------------------
/**
 * 移動制限Zセット
 * @param	fmmdl			FLDMMDL * 
 * @param	y				移動制限
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveLimitZSet( FLDMMDL * fmmdl, int z )
{
	fmmdl->move_limit_z = z;
}

//--------------------------------------------------------------
/**
 * 移動制限Z取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	int			移動制限z
 */
//--------------------------------------------------------------
int FLDMMDL_MoveLimitZGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->move_limit_z );
}

//--------------------------------------------------------------
/**
 * 描画ステータスセット
 * @param	fmmdl		FLDMMDL * 
 * @param	st			DRAW_STA_STOP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawStatusSet( FLDMMDL * fmmdl, u32 st )
{
	fmmdl->draw_status = st;
}

//--------------------------------------------------------------
/**
 * 描画ステータス取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	u32			DRAW_STA_STOP等
 */
//--------------------------------------------------------------
u32 FLDMMDL_DrawStatusGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->draw_status );
}

//--------------------------------------------------------------
/**
 * 動作TCB*セット
 * @param	fmmdl			FLDMMDL * 
 * @param	tcb				GFL_TCB *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveTcbPtrSet( FLDMMDL * fmmdl, GFL_TCB * tcb )
{
	fmmdl->tcb = tcb;
}

//--------------------------------------------------------------
/**
 * 動作TCB*取得
 * @param	fmmdl			FLDMMDL * 
 * @retval	GFL_TCB *			GFL_TCB *
 */
//--------------------------------------------------------------
GFL_TCB * FLDMMDL_MoveTcbPtrGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->tcb );
}

//--------------------------------------------------------------
/**
 * 動作TCB*を削除
 * @param	fmmdl			FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveTcbDelete( const FLDMMDL * fmmdl )
{
	GFL_TCB_DeleteTask( FLDMMDL_MoveTcbPtrGet(fmmdl) );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS *セット
 * @param	fmmdl			FLDMMDL * 
 * @param	sys				FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_FieldOBJSysSet( FLDMMDL * fmmdl, const FLDMMDLSYS *sys )
{
	fmmdl->fmmdlsys = sys;
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS *取得
 * @param	fmmdl			FLDMMDL * 
 * @retval	FLDMMDLSYS	FLDMMDLSYS *
 */
//--------------------------------------------------------------
const FLDMMDLSYS *FLDMMDL_FieldOBJSysGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->fmmdlsys );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS * 取得 not const
 * @param	fmmdl			FLDMMDL * 
 * @retval	FLDMMDLSYS	FLDMMDLSYS *
 */
//--------------------------------------------------------------
static FLDMMDLSYS *fmmdl_FieldOBJSysGet( const FLDMMDL * fmmdl )
{
	return( fmmdl_ConstSysLocalGet(fmmdl->fmmdlsys) );
}

//--------------------------------------------------------------
/**
 * 動作関数用ワーク初期化。
 * sizeがワークサイズを超えていた場合、ASSERT。
 * 動作用ワークはsize分、0で初期化。
 * @param	fmmdl	FLDMMDL * 
 * @param	size	必要なワークサイズ
 * @retval	void*	初期化されたワーク
 */
//--------------------------------------------------------------
void * FLDMMDL_MoveProcWorkInit( FLDMMDL * fmmdl, int size )
{
	void *work;
	
	GF_ASSERT( size <= FLDMMDL_MOVE_WORK_SIZE );
	work = FLDMMDL_MoveProcWorkGet( fmmdl );
	GFL_STD_MemClear( work, size );
	
	return( work );
}

//--------------------------------------------------------------
/**
 * 動作関数用ワーク取得。
 * @param	fmmdl	FLDMMDL * 
 * @retval	void*	ワーク
 */
//--------------------------------------------------------------
void * FLDMMDL_MoveProcWorkGet( FLDMMDL * fmmdl )
{
	return( fmmdl->move_proc_work );
}

//--------------------------------------------------------------
/**
 * 動作補助関数用ワーク初期化。
 * sizeがワークサイズを超えていた場合、ASSERT。
 * @param	fmmdl	FLDMMDL * 
 * @param	size	必要なワークサイズ
 * @retval	void*	初期化されたワーク
 */
//--------------------------------------------------------------
void * FLDMMDL_MoveSubProcWorkInit( FLDMMDL * fmmdl, int size )
{
	u8 *work;
	
	GF_ASSERT( size <= FLDMMDL_MOVE_SUB_WORK_SIZE &&
			"フィールド動作モデル 動作補助ワークサイズがオーバー" );
	
	work = FLDMMDL_MoveSubProcWorkGet( fmmdl );
	GFL_STD_MemClear( work, size );
	
	return( work );
}

//--------------------------------------------------------------
/**
 * 動作補助関数用ワーク取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	void*	ワーク*
 */
//--------------------------------------------------------------
void * FLDMMDL_MoveSubProcWorkGet( FLDMMDL * fmmdl )
{
	return( fmmdl->move_sub_proc_work );
}

//--------------------------------------------------------------
/**
 * 動作補助関数用ワーク初期化。
 * sizeがワークサイズを超えていた場合、ASSERT。
 * @param	fmmdl	FLDMMDL * 
 * @param	size	必要なワークサイズ
 * @retval	void*	初期化されたワーク
 */
//--------------------------------------------------------------
void * FLDMMDL_MoveCmdWorkInit( FLDMMDL * fmmdl, int size )
{
	u8 *work;
	
	GF_ASSERT( size <= FLDMMDL_MOVE_CMD_WORK_SIZE &&
			"フィールド動作モデル 動作コマンドワークサイズがオーバー" );
	
	work = FLDMMDL_MoveCmdWorkGet( fmmdl );
	GFL_STD_MemClear( work, size );
	
	return( work );
}

//--------------------------------------------------------------
/**
 * 動作コマンド用ワーク取得
 * @param	fmmdl	FLDMMDL *
 * @retval	void*	ワーク*
 */
//--------------------------------------------------------------
void * FLDMMDL_MoveCmdWorkGet( FLDMMDL * fmmdl )
{
	return( fmmdl->move_cmd_proc_work );
}

//--------------------------------------------------------------
/**
 * 描画関数用ワーク初期化。
 * sizeがワークサイズを超えていた場合、ASSERT。
 * @param	fmmdl	FLDMMDL * 
 * @param	size	必要なワークサイズ
 * @retval	void*	初期化されたワーク
 */
//--------------------------------------------------------------
void * FLDMMDL_DrawProcWorkInit( FLDMMDL * fmmdl, int size )
{
	u8 *work;
	
	GF_ASSERT( size <= FLDMMDL_DRAW_WORK_SIZE && "fmmdl draw work size over" );
	
	work = FLDMMDL_DrawProcWorkGet( fmmdl );
	GFL_STD_MemClear( work, size );
	
	return( work );
}

//--------------------------------------------------------------
/**
 * 描画関数用ワーク取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	void*	ワーク
 */
//--------------------------------------------------------------
void * FLDMMDL_DrawProcWorkGet( FLDMMDL * fmmdl )
{
	return( fmmdl->draw_proc_work );
}

//--------------------------------------------------------------
/**
 * 初期化関数セット
 * @param	fmmdl	FLDMMDL * 
 * @param	init	初期化関数
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveInitProcSet( FLDMMDL * fmmdl, FLDMMDL_MOVE_PROC_INIT init )
{
	fmmdl->move_init_proc = init;
}

//--------------------------------------------------------------
/**
 * 初期化関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveInitProcCall( FLDMMDL * fmmdl )
{
	fmmdl->move_init_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * 動作関数セット
 * @param	fmmdl	FLDMMDL * 
 * @param	move	動作関数
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveProcSet( FLDMMDL * fmmdl, FLDMMDL_MOVE_PROC move )
{
	fmmdl->move_proc = move;
}

//--------------------------------------------------------------
/**
 * 動作関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveProcCall( FLDMMDL * fmmdl )
{
	fmmdl->move_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * 削除関数登録
 * @param	fmmdl	FLDMMDL * 
 * @param	del		動作関数
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveDeleteProcSet( FLDMMDL * fmmdl, FLDMMDL_MOVE_PROC_DEL del )
{
	fmmdl->move_delete_proc = del;
}

//--------------------------------------------------------------
/**
 * 削除関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveDeleteProcCall( FLDMMDL * fmmdl )
{
	fmmdl->move_delete_proc( fmmdl );
}

#if 0
//--------------------------------------------------------------
/**
 * 復帰関数登録
 * @param	fmmdl	FLDMMDL * 
 * @param	del		動作関数
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveReturnProcSet( FLDMMDL * fmmdl, FLDMMDL_MOVE_PROC_RET ret )
{
	GF_ASSERT( 0 );
//	fmmdl->move_return_proc = ret;
}
#endif

//--------------------------------------------------------------
/**
 * 復帰関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveReturnProcCall( FLDMMDL * fmmdl )
{
#if 0
	fmmdl->move_return_proc( fmmdl );
#else
	const FLDMMDL_MOVE_PROC_LIST *list =
		fmmdl_MoveProcListGet( FLDMMDL_MoveCodeGet(fmmdl) );
	list->return_proc( fmmdl );
#endif
}

//--------------------------------------------------------------
/**
 * 描画初期化関数登録
 * @param	fmmdl	FLDMMDL * 
 * @param	init	初期化関数
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawInitProcSet( FLDMMDL * fmmdl, FLDMMDL_DRAW_PROC_INIT init )
{
	fmmdl->draw_init_proc = init;
}

//--------------------------------------------------------------
/**
 * 描画初期化関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawInitProcCall( FLDMMDL * fmmdl )
{
	fmmdl->draw_init_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * 描画関数登録
 * @param	fmmdl	FLDMMDL * 
 * @param	draw	描画関数
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawProcSet( FLDMMDL * fmmdl, FLDMMDL_DRAW_PROC draw )
{
	fmmdl->draw_proc = draw;
}

//--------------------------------------------------------------
/**
 * 描画関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawProcCall( FLDMMDL * fmmdl )
{
#if 0
	fmmdl->draw_proc( fmmdl );
#endif
}

//--------------------------------------------------------------
/**
 * 描画削除関数登録
 * @param	fmmdl	FLDMMDL * 
 * @param	del		削除関数
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawDeleteProcSet( FLDMMDL * fmmdl, FLDMMDL_DRAW_PROC_DEL del )
{
	fmmdl->draw_delete_proc = del;
}

//--------------------------------------------------------------
/**
 * 描画削除関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawDeleteProcCall( FLDMMDL * fmmdl )
{
	fmmdl->draw_delete_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * 描画退避関数登録
 * @param	fmmdl	FLDMMDL * 
 * @param	push	退避関数
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawPushProcSet( FLDMMDL * fmmdl, FLDMMDL_DRAW_PROC_PUSH push )
{
	fmmdl->draw_push_proc = push;
}

//--------------------------------------------------------------
/**
 * 描画退避関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawPushProcCall( FLDMMDL * fmmdl )
{
	fmmdl->draw_push_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * 描画復帰関数登録
 * @param	fmmdl	FLDMMDL * 
 * @param	pop		退避関数
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawPopProcSet( FLDMMDL * fmmdl, FLDMMDL_DRAW_PROC_POP pop )
{
	fmmdl->draw_pop_proc = pop;
}

//--------------------------------------------------------------
/**
 * 描画復帰関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawPopProcCall( FLDMMDL * fmmdl )
{
	fmmdl->draw_pop_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * アニメーションコマンドコードセット
 * @param	fmmdl	FLDMMDL * 
 * @param	code	AC_DIR_U等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_AcmdCodeSet( FLDMMDL * fmmdl, int code )
{
	fmmdl->acmd_code = code;
}

//--------------------------------------------------------------
/**
 * アニメーションコマンドコード取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	int		AC_DIR_U等
 */
//--------------------------------------------------------------
int FLDMMDL_AcmdCodeGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->acmd_code );
}

//--------------------------------------------------------------
/**
 * アニメーションコマンドシーケンスセット
 * @param	fmmdl	FLDMMDL * 
 * @param	no		シーケンス
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_AcmdSeqSet( FLDMMDL * fmmdl, int no )
{
	fmmdl->acmd_seq = no;
}

//--------------------------------------------------------------
/**
 * アニメーションコマンドシーケンス増加
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_AcmdSeqInc( FLDMMDL * fmmdl )
{
	fmmdl->acmd_seq++;
}

//--------------------------------------------------------------
/**
 * アニメーションコマンドシーケンス取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	int		シーケンス
 */
//--------------------------------------------------------------
int FLDMMDL_AcmdSeqGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->acmd_seq );
}

//--------------------------------------------------------------
/**
 * 現在のマップアトリビュートをセット
 * @param	fmmdl	FLDMMDL *
 * @param	attr	セットするアトリビュート
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_NowMapAttrSet( FLDMMDL * fmmdl, u32 attr )
{
	fmmdl->now_attr = attr;
}

//--------------------------------------------------------------
/**
 * 現在のマップアトリビュートを取得
 * @param	fmmdl	FLDMMDL *
 * @retval	u32		マップアトリビュート
 */
//--------------------------------------------------------------
u32 FLDMMDL_NowMapAttrGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->now_attr );
}

//--------------------------------------------------------------
/**
 * 過去のマップアトリビュートをセット
 * @param	fmmdl	FLDMMDL *
 * @param	attr	セットするアトリビュート
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OldMapAttrSet( FLDMMDL * fmmdl, u32 attr )
{
	fmmdl->old_attr = attr;
}

//--------------------------------------------------------------
/**
 * 過去のマップアトリビュートを取得
 * @param	fmmdl	FLDMMDL *
 * @retval	u32		マップアトリビュート
 */
//--------------------------------------------------------------
u32 FLDMMDL_OldMapAttrGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->old_attr );
}

//--------------------------------------------------------------
/**
 * FIELDSYS_WORK取得
 * @param	fmmdl	FLDMMDL *
 * @retval	FIELDSYS_WORK	FIELDSYS_WORK *
 */
//--------------------------------------------------------------
#if 0
void * FLDMMDL_FieldSysWorkGet( const FLDMMDL * fmmdl )
{
	FLDMMDLSYS *fos;
	
	fos = fmmdl_FieldOBJSysGet( fmmdl );
	return( FLDMMDLSYS_FieldSysWorkGet(fos) );
}
#endif

//--------------------------------------------------------------
/**
 * TCB基本プライオリティ取得
 * @param	fmmdl	FLDMMDL *
 * @retval	int		TCB基本プライオリティ
 */
//--------------------------------------------------------------
int FLDMMDL_TCBStandardPriorityGet( const FLDMMDL * fmmdl )
{
	return( FLDMMDLSYS_TCBStandardPriorityGet(FLDMMDL_FieldOBJSysGet(fmmdl)) );
}

//--------------------------------------------------------------
/**
 * エイリアス時のゾーンID取得。エイリアス時はイベントフラグが指定ゾーンID
 * @param	fmmdl	FLDMMDL *
 * @retval	int		ゾーンID
 */
//--------------------------------------------------------------
int FLDMMDL_ZoneIDGetAlies( const FLDMMDL * fmmdl )
{
	GF_ASSERT( FLDMMDL_StatusBitCheck_Alies(fmmdl) == TRUE &&
			"FLDMMDL_ZoneIDGetAlies()aliesでは無い" );
	return( FLDMMDL_EventFlagGet(fmmdl) );
}

//==============================================================================
//	フィールド動作モデルシステム　ステータスビット簡易
//==============================================================================
//--------------------------------------------------------------
/**
 * フィールド動作モデル全体の動作を完全停止。
 * 動作処理、描画処理を完全停止する。アニメーションコマンドにも反応しない。
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_MoveStopAll( FLDMMDLSYS *fos )
{
	FLDMMDLSYS_StatusBit_ON( fos,
		FLDMMDLSYS_STABIT_MOVE_PROC_STOP | FLDMMDLSYS_STABIT_DRAW_PROC_STOP );
	
	#ifdef DEBUG_FLDMMDL_PRINTF
	OS_Printf( "フィールド動作モデル全体の動作を停止しました\n" );
	#endif
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル全体の動作完全停止を解除
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_MoveStopAllClear( FLDMMDLSYS *fos )
{
	FLDMMDLSYS_StatusBit_OFF( fos,
		FLDMMDLSYS_STABIT_MOVE_PROC_STOP | FLDMMDLSYS_STABIT_DRAW_PROC_STOP );
	
	#ifdef DEBUG_FLDMMDL_PRINTF
	OS_Printf( "フィールド動作モデル全体の動作停止を解除しました\n" );
	#endif
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル全体の動作を一時停止
 * 固有の動作処理（ランダム移動等）を一時停止する。
 * アニメーションコマンドには反応する。
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_MovePauseAll( FLDMMDLSYS *fos )
{
	int max = FLDMMDLSYS_OBJMaxGet( fos );
	FLDMMDL * fmmdl = FLDMMDLSYS_fmmdlWorkGet( fos );
	
	do{
		if( FLDMMDL_StatusBitCheck_Use(fmmdl) ){
			FLDMMDL_MovePause( fmmdl );
		}
		
		fmmdl++;
		max--;
	}while( max );
	
	#ifdef DEBUG_FLDMMDL_PRINTF
	OS_Printf( "フィールド動作モデル全体動作をポーズしました\n" );
	#endif
}

//--------------------------------------------------------------
/**
 * フィールド動作モデル全体動作一時停止を解除
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_MovePauseAllClear( FLDMMDLSYS *fos )
{
	int max = FLDMMDLSYS_OBJMaxGet( fos );
	FLDMMDL * fmmdl = FLDMMDLSYS_fmmdlWorkGet( fos );
	
	do{
		if( FLDMMDL_StatusBitCheck_Use(fmmdl) ){
			FLDMMDL_MovePauseClear( fmmdl );
		}
		
		fmmdl++;
		max--;
	}while( max );
	
	#ifdef DEBUG_FLDMMDL_PRINTF
	OS_Printf( "フィールド動作モデル全体動作ポーズを解除しました\n" );
	#endif
}

//--------------------------------------------------------------
/**
 * 描画処理が初期化されているかどうかチェック
 * @param	fos				FLDMMDLSYS *
 * @retval	int				TRUE=初期化されている
 */
//--------------------------------------------------------------
int FLDMMDLSYS_DrawInitCompCheck( const FLDMMDLSYS *fos )
{
	if( FLDMMDLSYS_StatusBit_Check(fos,FLDMMDLSYS_STABIT_DRAW_INIT_COMP) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルからフィールド動作モデルシステムのビットチェック
 * @param	fmmdl		FLDMMDL *
 * @param	bit			FLDMMDLSYS_STABIT_DRAW_INIT_COMP等
 * @retval	u32			0以外 bitヒット
 */
//--------------------------------------------------------------
u32 FLDMMDL_FieldOBJSysStatusBitCheck( const FLDMMDL * fmmdl, u32 bit )
{
	const FLDMMDLSYS *fos = FLDMMDL_FieldOBJSysGet( fmmdl );
	return( FLDMMDLSYS_StatusBit_Check(fos,bit) );
}

//--------------------------------------------------------------
/**
 * 影を付ける、付けないのセット
 * @param	fos		FLDMMDLSYS *
 * @param	flag	TRUE=影を付ける FALSE=影を付けない
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_ShadowJoinSet( FLDMMDLSYS *fos, int flag )
{
	if( flag == FALSE ){
		FLDMMDLSYS_StatusBit_ON( fos, FLDMMDLSYS_STABIT_SHADOW_JOIN_NOT );
	}else{
		FLDMMDLSYS_StatusBit_OFF( fos, FLDMMDLSYS_STABIT_SHADOW_JOIN_NOT );
	}
}

//--------------------------------------------------------------
/**
 * 影を付ける、付けないのチェック
 * @param	fos		FLDMMDLSYS *
 * @retval	int		FALSE=付けない
 */
//--------------------------------------------------------------
int FLDMMDLSYS_ShadowJoinCheck( const FLDMMDLSYS *fos )
{
	if( FLDMMDLSYS_StatusBit_Check(fos,FLDMMDLSYS_STABIT_SHADOW_JOIN_NOT) ){
		return( FALSE );
	}
	
	return( TRUE );
}

//==============================================================================
//	フィールド動作モデル ステータスビット簡易
//==============================================================================
//--------------------------------------------------------------
/**
 * 使用チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	int		TRUE=使用中
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_Use( const FLDMMDL * fmmdl )
{
	return( FLDMMDL_StatusBit_CheckEasy(fmmdl,FLDMMDL_STABIT_USE) );
}

//--------------------------------------------------------------
/**
 * 移動動作中にする
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitON_Move( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_MOVE );
}

//--------------------------------------------------------------
/**
 * 移動動作中を解除
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitOFF_Move( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_MOVE );
}

//--------------------------------------------------------------
/**
 * 移動動作中チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	int		TRUE=動作中
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_Move( const FLDMMDL * fmmdl )
{
	return( FLDMMDL_StatusBit_CheckEasy(fmmdl,FLDMMDL_STABIT_MOVE) );
}

//--------------------------------------------------------------
/**
 * 移動動作開始にする
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitON_MoveStart( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_MOVE_START );
}

//--------------------------------------------------------------
/**
 * 移動動作開始を解除
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitOFF_MoveStart( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_MOVE_START );
}

//--------------------------------------------------------------
/**
 * 移動動作開始チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	int		TRUE=移動動作開始
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_MoveStart( const FLDMMDL * fmmdl )
{
	return( FLDMMDL_StatusBit_CheckEasy(fmmdl,FLDMMDL_STABIT_MOVE_START) );
}

//--------------------------------------------------------------
/**
 * 移動動作終了にする
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitON_MoveEnd( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_MOVE_END );
}

//--------------------------------------------------------------
/**
 * 移動動作終了を解除
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitOFF_MoveEnd( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_MOVE_END );
}

//--------------------------------------------------------------
/**
 * 移動動作終了チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	int		TRUE=移動終了
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_MoveEnd( const FLDMMDL * fmmdl )
{
	return( FLDMMDL_StatusBit_CheckEasy(fmmdl,FLDMMDL_STABIT_MOVE_END) );
}

//--------------------------------------------------------------
/**
 * 描画初期化完了にする
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitON_DrawProcInitComp( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_DRAW_PROC_INIT_COMP );
}

//--------------------------------------------------------------
/**
 * 描画初期化完了を解除
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitOFF_DrawProcInitComp( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_DRAW_PROC_INIT_COMP );
}

//--------------------------------------------------------------
/**
 * 描画初期化完了チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	int		TRUE=描画初期化完了
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_DrawProcInitComp( const FLDMMDL * fmmdl )
{
	return( FLDMMDL_StatusBit_CheckEasy(fmmdl,FLDMMDL_STABIT_DRAW_PROC_INIT_COMP) );
}

//--------------------------------------------------------------
/**
 * 非表示フラグをチェック
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=非表示　FALSE=表示
 * @retval	nothing
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_Vanish( const FLDMMDL * fmmdl )
{
	return( FLDMMDL_StatusBit_CheckEasy(fmmdl,FLDMMDL_STABIT_VANISH) );
}

//--------------------------------------------------------------
/**
 * 非表示フラグを設定
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=非表示　FALSE=表示
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitSet_Vanish( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_VANISH );
	}else{
		FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_VANISH );
	}
}

//--------------------------------------------------------------
/**
 * OBJ同士の当たり判定フラグを設定
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=ヒットアリ　FALSE=ヒットナシ
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitSet_FellowHit( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_FELLOW_HIT_NON );
	}else{
		FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_FELLOW_HIT_NON );
	}
}

//--------------------------------------------------------------
/**
 * 動作中フラグのセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=動作中　FALSE=停止中
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitSet_Move( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_StatusBitON_Move( fmmdl );
	}else{
		FLDMMDL_StatusBitOFF_Move( fmmdl );
	}
}

//--------------------------------------------------------------
/**
 * 話しかけ可能チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	int		TRUE=可能 FALSE=不可
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_Talk( FLDMMDL * fmmdl )
{
	if( FLDMMDL_StatusBit_CheckEasy(fmmdl,FLDMMDL_STABIT_TALK_OFF) == TRUE ){
		return( FALSE );
	}
	
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * 話しかけ不可フラグをセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=不可 FALSE=可能
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitSet_TalkOFF( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_TALK_OFF );
	}else{
		FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_TALK_OFF );
	}
}

//--------------------------------------------------------------
/**
 * 動作ポーズ
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MovePause( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_PAUSE_MOVE );
}

//--------------------------------------------------------------
/**
 * 動作ポーズ解除
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MovePauseClear( FLDMMDL * fmmdl )
{
	FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_PAUSE_MOVE );
}

//--------------------------------------------------------------
/**
 * 動作ポーズチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	int		TRUE=動作ポーズ
 */
//--------------------------------------------------------------
int FLDMMDL_MovePauseCheck( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_StatusBit_CheckEasy(fmmdl,FLDMMDL_STABIT_PAUSE_MOVE) == TRUE ){
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * ステータスビット 描画処理初期化完了チェック
 * @param	fmmdl		FLDMMDL *
 * @retval	int			TRUE=完了。FALSE=まだ
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBit_DrawInitCompCheck( const FLDMMDL * fmmdl )
{
	const FLDMMDLSYS *fos;
	
	fos = FLDMMDL_FieldOBJSysGet( fmmdl );
	
	if( FLDMMDLSYS_DrawInitCompCheck(fos) == FALSE ){
		return( FALSE );
	}
	
	if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_DRAW_PROC_INIT_COMP) == 0 ){
		return( FALSE );
	}
	
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * 高さ取得を禁止する
 * @param	fmmdl	FLDMMDL *
 * @param	int		TRUE=高さ取得OFF FALSE=ON
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitSet_HeightGetOFF( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_HEIGHT_GET_OFF );
	}else{
		FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_HEIGHT_GET_OFF );
	}
}

//--------------------------------------------------------------
/**
 * 高さ取得が禁止されているかチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	int		TRUE=禁止
 */
//--------------------------------------------------------------
int FLDMMDL_HeightOFFCheck( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_HEIGHT_GET_OFF) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * ゾーン切り替え時の削除禁止
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=禁止 FALSE=禁止しない
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_NotZoneDeleteSet( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_ZONE_DEL_NOT );
	}else{
		FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_ZONE_DEL_NOT );
	}
}

//--------------------------------------------------------------
/**
 * エイリアスフラグをセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=エイリアス FALSE=違う
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitSet_Alies( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_ALIES );
	}else{
		FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_ALIES );
	}
}

//--------------------------------------------------------------
/**
 * エイリアスフラグをチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	flag	TRUE=エイリアス FALSE=違う
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_Alies( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_ALIES) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * 浅瀬エフェクトセットフラグをセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=セット　FALSE=クリア
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitSet_ShoalEffectSet( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_EFFSET_SHOAL );
	}else{
		FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_EFFSET_SHOAL );
	}
}

//--------------------------------------------------------------
/**
 * 浅瀬エフェクトセットフラグをチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	flag	TRUE=セット　FALSE=違う
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_ShoalEffectSet( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_EFFSET_SHOAL) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * アトリビュートオフセット設定OFFセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=セット　FALSE=クリア
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitSet_AttrOffsOFF( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_ATTR_OFFS_OFF );
	}else{
		FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_ATTR_OFFS_OFF );
	}
}

//--------------------------------------------------------------
/**
 * アトリビュートオフセット設定OFFチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	flag	TRUE=OFF　FALSE=違う
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_AttrOffsOFF( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_ATTR_OFFS_OFF) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * 橋移動フラグセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=セット　FALSE=クリア
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitSet_Bridge( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_BRIDGE );
	}else{
		FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_BRIDGE );
	}
}

//--------------------------------------------------------------
/**
 * 橋移動フラグチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	flag	TRUE=橋　FALSE=違う
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_Bridge( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_BRIDGE) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * 映りこみフラグセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=セット　FALSE=クリア
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitSet_Reflect( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_REFLECT_SET );
	}else{
		FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_REFLECT_SET );
	}
}

//--------------------------------------------------------------
/**
 * 映りこみフラグチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	flag	TRUE=セット　FALSE=無し
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_Reflect( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_REFLECT_SET) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * アニメーションコマンドチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	flag	TRUE=コマンドアリ　FALSE=無し
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_Acmd( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_ACMD) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * 拡張高さ反応フラグをセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=セット　FALSE=クリア
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_StatusBitSet_HeightExpand( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_HEIGHT_EXPAND );
	}else{
		FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_HEIGHT_EXPAND );
	}
}

//--------------------------------------------------------------
/**
 * 拡張高さ反応フラグチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	flag	TRUE=拡張高さに反応する　FALSE=無し
 */
//--------------------------------------------------------------
int FLDMMDL_StatusBitCheck_HeightExpand( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_HEIGHT_EXPAND) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//==============================================================================
//	動作ビット簡易
//==============================================================================
//--------------------------------------------------------------
/**
 * アトリビュート取得を停止
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=停止
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveBitSet_AttrGetOFF( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_MoveBit_ON( fmmdl, FLDMMDL_MOVEBIT_ATTR_GET_OFF );
	}else{
		FLDMMDL_MoveBit_OFF( fmmdl, FLDMMDL_MOVEBIT_ATTR_GET_OFF );
	}
}

//--------------------------------------------------------------
/**
 * アトリビュート取得を停止　チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	int	TRUE=停止
 */
//--------------------------------------------------------------
int FLDMMDL_MoveBitCheck_AttrGetOFF( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_MoveBit_Check(fmmdl,FLDMMDL_MOVEBIT_ATTR_GET_OFF) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//==============================================================================
//	座標系参照
//==============================================================================
//--------------------------------------------------------------
/**
 * 初期座標 X座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	int		X座標
 */
//--------------------------------------------------------------
int FLDMMDL_InitPosGX_Get( const FLDMMDL * fmmdl )
{
	return( fmmdl->gx_init );
}

//--------------------------------------------------------------
/**
 * 初期座標 X座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	x		セットする座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_InitPosGX_Set( FLDMMDL * fmmdl, int x )
{
	fmmdl->gx_init = x;
}

//--------------------------------------------------------------
/**
 * 初期座標 Y座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	int		Y
 */
//--------------------------------------------------------------
int FLDMMDL_InitPosGY_Get( const FLDMMDL * fmmdl )
{
	return( fmmdl->gy_init );
}

//--------------------------------------------------------------
/**
 * 初期座標 Y座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	y		セットする座標
 * @retval	int		y座標
 */
//--------------------------------------------------------------
void FLDMMDL_InitPosGY_Set( FLDMMDL * fmmdl, int y )
{
	fmmdl->gy_init = y;
}

//--------------------------------------------------------------
/**
 * 初期座標 z座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	int		z座標
 */
//--------------------------------------------------------------
int FLDMMDL_InitPosGZ_Get( const FLDMMDL * fmmdl )
{
	return( fmmdl->gz_init );
}

//--------------------------------------------------------------
/**
 * 初期座標 z座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	z		セットする座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_InitPosGZ_Set( FLDMMDL * fmmdl, int z )
{
	fmmdl->gz_init = z;
}

//--------------------------------------------------------------
/**
 * 過去座標　X座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	int		X座標
 */
//--------------------------------------------------------------
int FLDMMDL_OldPosGX_Get( const FLDMMDL * fmmdl )
{
	return( fmmdl->gx_old );
}

//--------------------------------------------------------------
/**
 * 過去座標 X座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	x		セットする座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OldPosGX_Set( FLDMMDL * fmmdl, int x )
{
	fmmdl->gx_old = x;
}

//--------------------------------------------------------------
/**
 * 過去座標 Y座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	int		Y
 */
//--------------------------------------------------------------
int FLDMMDL_OldPosGY_Get( const FLDMMDL * fmmdl )
{
	return( fmmdl->gy_old );
}

//--------------------------------------------------------------
/**
 * 過去座標 Y座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	y		セットする座標
 * @retval	int		y座標
 */
//--------------------------------------------------------------
void FLDMMDL_OldPosGY_Set( FLDMMDL * fmmdl, int y )
{
	fmmdl->gy_old = y;
}

//--------------------------------------------------------------
/**
 * 過去座標 z座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	int		z座標
 */
//--------------------------------------------------------------
int FLDMMDL_OldPosGZ_Get( const FLDMMDL * fmmdl )
{
	return( fmmdl->gz_old );
}

//--------------------------------------------------------------
/**
 * 過去座標 z座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	z		セットする座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OldPosGZ_Set( FLDMMDL * fmmdl, int z )
{
	fmmdl->gz_old = z;
}

//--------------------------------------------------------------
/**
 * 現在座標 X座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	int		X座標
 */
//--------------------------------------------------------------
int FLDMMDL_NowPosGX_Get( const FLDMMDL * fmmdl )
{
	return( fmmdl->gx_now );
}

//--------------------------------------------------------------
/**
 * 現在座標 X座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	x		セットする座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_NowPosGX_Set( FLDMMDL * fmmdl, int x )
{
	fmmdl->gx_now = x;
}

//--------------------------------------------------------------
/**
 * 現在座標 X座標増加
 * @param	fmmdl	FLDMMDL * 
 * @param	x		足す値
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_NowPosGX_Add( FLDMMDL * fmmdl, int x )
{
	fmmdl->gx_now += x;
}

//--------------------------------------------------------------
/**
 * 現在座標 Y座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	int		Y
 */
//--------------------------------------------------------------
int FLDMMDL_NowPosGY_Get( const FLDMMDL * fmmdl )
{
	return( fmmdl->gy_now );
}

//--------------------------------------------------------------
/**
 * 現在座標 Y座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	y		セットする座標
 * @retval	int		y座標
 */
//--------------------------------------------------------------
void FLDMMDL_NowPosGY_Set( FLDMMDL * fmmdl, int y )
{
	fmmdl->gy_now = y;
}

//--------------------------------------------------------------
/**
 * 現在座標 Y座標増加
 * @param	fmmdl	FLDMMDL * 
 * @param	y		足す値
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_NowPosGY_Add( FLDMMDL * fmmdl, int y )
{
	fmmdl->gy_now += y;
}

//--------------------------------------------------------------
/**
 * 過去座標 z座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	int		z座標
 */
//--------------------------------------------------------------
int FLDMMDL_NowPosGZ_Get( const FLDMMDL * fmmdl )
{
	return( fmmdl->gz_now );
}

//--------------------------------------------------------------
/**
 * 現在座標 z座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	z		セットする座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_NowPosGZ_Set( FLDMMDL * fmmdl, int z )
{
	fmmdl->gz_now = z;
}

//--------------------------------------------------------------
/**
 * 現在座標 z座標増加
 * @param	fmmdl	FLDMMDL * 
 * @param	z		足す値
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_NowPosGZ_Add( FLDMMDL * fmmdl, int z )
{
	fmmdl->gz_now += z;
}

//--------------------------------------------------------------
/**
 * 実座標取得
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		座標格納先
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_VecPosGet( const FLDMMDL * fmmdl, VecFx32 *vec )
{
	*vec = fmmdl->vec_pos_now;
}

//--------------------------------------------------------------
/**
 * 実座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_VecPosSet( FLDMMDL * fmmdl, const VecFx32 *vec )
{
	fmmdl->vec_pos_now = *vec;
}

//--------------------------------------------------------------
/**
 * 実座標ポインタ取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	VecFx32	実座標ポインタ
 */
//--------------------------------------------------------------
const VecFx32 * FLDMMDL_VecPosPtrGet( const FLDMMDL * fmmdl )
{
	return( &fmmdl->vec_pos_now );
}

//--------------------------------------------------------------
/**
 * 実座標Y値取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	fx32	高さ
 */
//--------------------------------------------------------------
fx32 FLDMMDL_VecPosYGet( const FLDMMDL * fmmdl )
{
	return( fmmdl->vec_pos_now.y );
}

//--------------------------------------------------------------
/**
 * 表示座標オフセット取得
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_VecDrawOffsGet( const FLDMMDL * fmmdl, VecFx32 *vec )
{
	*vec = fmmdl->vec_draw_offs;
}

//--------------------------------------------------------------
/**
 * 表示座標オフセットセット
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_VecDrawOffsSet( FLDMMDL * fmmdl, const VecFx32 *vec )
{
	fmmdl->vec_draw_offs = *vec;
}

//--------------------------------------------------------------
/**
 * 表示座標オフセットポインタ取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	VecFx32		オフセットポインタ
 */
//--------------------------------------------------------------
VecFx32 * FLDMMDL_VecDrawOffsPtrGet( FLDMMDL * fmmdl )
{
	return( &fmmdl->vec_draw_offs );
}

//--------------------------------------------------------------
/**
 * 外部指定表示座標オフセット取得
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_VecDrawOffsOutSideGet( const FLDMMDL * fmmdl, VecFx32 *vec )
{
	*vec = fmmdl->vec_draw_offs_outside;
}

//--------------------------------------------------------------
/**
 * 外部指定表示座標オフセットセット
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_VecDrawOffsOutSideSet( FLDMMDL * fmmdl, const VecFx32 *vec )
{
	fmmdl->vec_draw_offs_outside = *vec;
}

//--------------------------------------------------------------
/**
 * アトリビュート変化座標オフセット取得
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_VecAttrOffsGet( const FLDMMDL * fmmdl, VecFx32 *vec )
{
	*vec = fmmdl->vec_attr_offs;
}

//--------------------------------------------------------------
/**
 * アトリビュート変化座標オフセットセット
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_VecAttrOffsSet( FLDMMDL * fmmdl, const VecFx32 *vec )
{
	fmmdl->vec_attr_offs = *vec;
}

//--------------------------------------------------------------
/**
 * 高さ(グリッド単位)を取得
 * @param	fmmdl	FLDMMDL *
 * @retval	int		高さ。H_GRID単位
 */
//--------------------------------------------------------------
int FLDMMDL_HeightGridGet( const FLDMMDL * fmmdl )
{
	fx32 y = FLDMMDL_VecPosYGet( fmmdl );
	int gy = SIZE_H_GRID_FX32( y );
	return( gy );
}

//==============================================================================
//	FLDMMDL_H	参照
//==============================================================================
//--------------------------------------------------------------
/**
 * ID セット
 * @param	head	FLDMMDL_H
 * @param	id		OBJ ID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_H_IDSet( FLDMMDL_H *head, int id )
{
	head->id = id;
}

//--------------------------------------------------------------
/**
 * ID 取得
 * @param	head	FLDMMDL_H
 * @retval	u32		ID
 */
//--------------------------------------------------------------
int FLDMMDL_H_IDGet( const FLDMMDL_H *head )
{
	return( head->id );
}

//--------------------------------------------------------------
/**
 * OBJコードセット
 * @param	head	FLDMMDL_H
 * @param	code	HERO等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_H_OBJCodeSet( FLDMMDL_H *head, int code )
{
	head->obj_code = code;
}

//--------------------------------------------------------------
/**
 * OBJコード取得
 * @param	head	FLDMMDL_H
 * @retval	int		OBJコード
 */
//--------------------------------------------------------------
int FLDMMDL_H_OBJCodeGet( const FLDMMDL_H *head )
{
	return( head->obj_code );
}

//--------------------------------------------------------------
/**
 * 動作コードセット
 * @param	head	FLDMMDL_H
 * @param	code	MV_DIR_RND等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_H_MoveCodeSet( FLDMMDL_H *head, int code )
{
	head->move_code = code;
}

//--------------------------------------------------------------
/**
 * 動作コード取得
 * @param	head	FLDMMDL_H
 * @retval	int		動作コード
 */
//--------------------------------------------------------------
int FLDMMDL_H_MoveCodeGet( const FLDMMDL_H *head )
{
	return( head->move_code );
}

//--------------------------------------------------------------
/**
 * イベントタイプセット
 * @param	head	FLDMMDL_H
 * @param	type	イベントタイプ
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_H_EventTypeSet( FLDMMDL_H *head, int type )
{
	head->event_type = type;
}

//--------------------------------------------------------------
/**
 * イベントタイプ取得
 * @param	head	FLDMMDL_H
 * @retval	int		イベントタイプ
 */
//--------------------------------------------------------------
int FLDMMDL_H_EventTypeGet( const FLDMMDL_H *head )
{
	return( head->event_type );
}

//--------------------------------------------------------------
/**
 * イベントフラグセット
 * @param	head	FLDMMDL_H
 * @param	flag	イベントフラグ
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_H_EventFlagSet( FLDMMDL_H *head, int flag )
{
	head->event_flag = flag;
}

//--------------------------------------------------------------
/**
 * イベントフラグ取得
 * @param	head	FLDMMDL_H
 * @retval	int		イベントフラグ
 */
//--------------------------------------------------------------
int FLDMMDL_H_EventFlagGet( const FLDMMDL_H *head )
{
	return( head->event_flag );
}

//--------------------------------------------------------------
/**
 * イベントIDセット
 * @param	head	FLDMMDL_H
 * @param	id		イベントID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_H_EventIDSet( FLDMMDL_H *head, int id )
{
	head->event_id = id;
}

//--------------------------------------------------------------
/**
 * イベントID取得
 * @param	head	FLDMMDL_H
 * @retval	int		イベントフラグ
 */
//--------------------------------------------------------------
int FLDMMDL_H_EventIDGet( const FLDMMDL_H *head )
{
	return( head->event_id );
}

//--------------------------------------------------------------
/**
 * 方向セット
 * @param	head	FLDMMDL_H
 * @param	dir		DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_H_DirSet( FLDMMDL_H *head, int dir )
{
	head->dir = dir;
}

//--------------------------------------------------------------
/**
 * 方向取得
 * @param	head	FLDMMDL_H
 * @retval	int		方向
 */
//--------------------------------------------------------------
int FLDMMDL_H_DirGet( const FLDMMDL_H *head )
{
	return( head->dir );
}

//--------------------------------------------------------------
/**
 * 指定パラメタセット
 * @param	head	FLDMMDL_H
 * @param	param	指定パラメタ
 * @param	no		FLDMMDL_PARAM_0等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_H_ParamSet( FLDMMDL_H *head, int param, FLDMMDL_H_PARAM no )
{
	switch( no ){
	case FLDMMDL_PARAM_0: head->param0 = param; break;
	case FLDMMDL_PARAM_1: head->param1 = param; break;
	case FLDMMDL_PARAM_2: head->param2 = param; break;
	default: GF_ASSERT( 0 && "FLDMMDL_H_ParamSet()不正な引数" );
	}
}

//--------------------------------------------------------------
/**
 * 指定パラメタ取得
 * @param	head	FLDMMDL_H
 * @param	no		取得するパラメタ。FLDMMDL_PARAM_0等
 * @retval	int		パラメタ
 */
//--------------------------------------------------------------
int FLDMMDL_H_ParamGet( const FLDMMDL_H *head, FLDMMDL_H_PARAM no )
{
	switch( no ){
	case FLDMMDL_PARAM_0: return( head->param0 );
	case FLDMMDL_PARAM_1: return( head->param1 );
	case FLDMMDL_PARAM_2: return( head->param2 );
	}
	
	GF_ASSERT( 0 && "FLDMMDL_H_ParamGet()不正な引数" );
	return( 0 );
}

//--------------------------------------------------------------
/**
 * 移動制限Xセット
 * @param	head	FLDMMDL_H
 * @param	x		移動制限X
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_H_MoveLimitXSet( FLDMMDL_H *head, int x )
{
	head->move_limit_x = x;
}

//--------------------------------------------------------------
/**
 * 移動制限X取得
 * @param	head	FLDMMDL_H
 * @retval	int		移動制限X
 */
//--------------------------------------------------------------
int FLDMMDL_H_MoveLimitXGet( const FLDMMDL_H *head )
{
	return( head->move_limit_x );
}

//--------------------------------------------------------------
/**
 * 移動制限Zセット
 * @param	head	FLDMMDL_H
 * @param	z		移動制限Z
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_H_MoveLimitZSet( FLDMMDL_H *head, int z )
{
	head->move_limit_z = z;
}

//--------------------------------------------------------------
/**
 * 移動制限Z取得
 * @param	head	FLDMMDL_H
 * @retval	int		移動制限Y
 */
//--------------------------------------------------------------
int FLDMMDL_H_MoveLimitZGet( const FLDMMDL_H *head )
{
	return( head->move_limit_z );
}

//--------------------------------------------------------------
/**
 * X座標セット
 * @param	head	FLDMMDL_H
 * @param	x		X座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_H_PosXSet( FLDMMDL_H *head, int x )
{
	head->gx = x;
}

//--------------------------------------------------------------
/**
 * X座標取得
 * @param	head	FLDMMDL_H
 * @retval	int		X座標
 */
//--------------------------------------------------------------
int FLDMMDL_H_PosXGet( const FLDMMDL_H *head )
{
	return( head->gx );
}

//--------------------------------------------------------------
/**
 * Y座標セット
 * @param	head	FLDMMDL_H
 * @param	y		y座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_H_PosYSet( FLDMMDL_H *head, int y )
{
	head->gy = y;
}

//--------------------------------------------------------------
/**
 * Y座標取得
 * @param	head	FLDMMDL_H
 * @retval	int		Y座標
 */
//--------------------------------------------------------------
int FLDMMDL_H_PosYGet( const FLDMMDL_H *head )
{
	return( head->gy );
}

//--------------------------------------------------------------
/**
 * Z座標セット
 * @param	head	FLDMMDL_H
 * @param	z		Z座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_H_PosZSet( FLDMMDL_H *head, int z )
{
	head->gz = z;
}

//--------------------------------------------------------------
/**
 * Z座標取得
 * @param	head	FLDMMDL_H
 * @retval	int		Y座標
 */
//--------------------------------------------------------------
int FLDMMDL_H_PosZGet( const FLDMMDL_H *head )
{
	return( head->gz );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルヘッダー配列から指定IDを持ったヘッダーを検索
 * @param	id		ヘッダーID
 * @param	max		head要素数
 * @param	head	フィールド動作モデルヘッダーテーブル
 * @retval	FLDMMDL_H idを持つhead内ポインタ。NULL=一致無し
 */
//--------------------------------------------------------------
static const FLDMMDL_H * fmmdl_H_OBJIDSearch( int id, int max, const FLDMMDL_H *head )
{
	int i = 0;
	
	do{
		if( fmmdl_H_AliesCheck(&head[i]) == FALSE ){
			if( FLDMMDL_H_IDGet(&head[i]) == id ){
				return( &head[i] );
			}
		}
		
		i++;
	}while( i < max );
	
	return( NULL );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルヘッダー　エイリアスチェック
 * @param	head	FLDMMDL_H
 * @retval	int		TRUE=エイリアス　FALSE=違う
 */
//--------------------------------------------------------------
static int fmmdl_H_AliesCheck( const FLDMMDL_H *head )
{
	u16 id = (u16)FLDMMDL_H_EventIDGet( head );
#if 0
	if( id == SP_SCRID_ALIES ){ return( TRUE ); }
#endif
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * フィールド動作モデルヘッダー　エイリアス時のゾーンID取得。
 * エイリアス時はイベントフラグがゾーンIDになる
 * @param	head	FLDMMDL_H
 * @retval	int		ゾーンID
 */
//--------------------------------------------------------------
static int fmmdl_H_AliesZoneIDGet( const FLDMMDL_H *head )
{
	GF_ASSERT( fmmdl_H_AliesCheck(head) == TRUE && "fmmdl_H_AliesZoneIDGet()aliesではない" );
	return( FLDMMDL_H_EventFlagGet(head) );
}

//==============================================================================
//	FLDMMDL_MOVE_PROC 参照
//==============================================================================
//--------------------------------------------------------------
/**
 * 動作テーブルから指定のリスト取り出し
 * @param	nothing
 * @retval	FLDMMDL_MOVE_PROC_LIST	FLDMMDL_MOVE_PROC_LISTテーブル
 */
//--------------------------------------------------------------
static const FLDMMDL_MOVE_PROC_LIST * fmmdl_MoveProcListGet( u32 code )
{
	GF_ASSERT( code < MV_CODE_MAX );
	return( DATA_FieldOBJMoveProcListTbl[code] );
}

//--------------------------------------------------------------
/**
 * 動作コード取得
 * @param	list 		FLDMMDL_MOVE_PROC_LIST
 * @retval	u32			MV_PLAYER等
 */
//--------------------------------------------------------------
static u32 fmmdl_MoveProcList_MoveCodeGet( const FLDMMDL_MOVE_PROC_LIST *list )
{
	return( list->move_code );
}

//--------------------------------------------------------------
/**
 * 初期化関数取得
 * @param	list FLDMMDL_MOVE_PROC_LIST
 * @retval	FLDMMDL_MOVE_PROC_INIT listの初期化関数
 */
//--------------------------------------------------------------
static FLDMMDL_MOVE_PROC_INIT fmmdl_MoveProcList_InitGet(
		const FLDMMDL_MOVE_PROC_LIST *list )
{
	return( list->init_proc );
}

//--------------------------------------------------------------
/**
 * 動作関数取得
 * @param	list FLDMMDL_MOVE_PROC_LIST
 * @retval	FLDMMDL_MOVE_PROC listの動作関数
 */
//--------------------------------------------------------------
static FLDMMDL_MOVE_PROC fmmdl_MoveProcList_MoveGet( const FLDMMDL_MOVE_PROC_LIST *list )
{
	return( list->move_proc );
}

//--------------------------------------------------------------
/**
 * 削除関数取得
 * @param	list FLDMMDL_MOVE_PROC_LIST
 * @retval	FLDMMDL_MOVE_PROC_DEL listの動作関数
 */
//--------------------------------------------------------------
static FLDMMDL_MOVE_PROC_DEL fmmdl_MoveProcList_DeleteGet(
		const FLDMMDL_MOVE_PROC_LIST *list )
{
	return( list->delete_proc );
}

//==============================================================================
//	FLDMMDL_DRAW_PROC_LIST FLDMMDL_DRAW_PROC_LIST_REG参照
//==============================================================================
//--------------------------------------------------------------
/**
 * 初期化関数取得
 * @param	list	FLDMMDL_DRAW_PROC_LIST *
 * @retval	FLDMMDL_DRAW_PROC_INIT	FLDMMDL_DRAW_PROC_INIT *
 */
//--------------------------------------------------------------
static FLDMMDL_DRAW_PROC_INIT fmmdl_DrawProcList_InitGet(
	const FLDMMDL_DRAW_PROC_LIST *list )
{
	return( list->init_proc );
}

//--------------------------------------------------------------
/**
 * 描画関数取得
 * @param	list	FLDMMDL_DRAW_PROC_LIST *
 * @retval	FLDMMDL_DRAW_PROC		FLDMMDL_DRAW_PROC *
 */
//--------------------------------------------------------------
static FLDMMDL_DRAW_PROC fmmdl_DrawProcList_DrawGet( const FLDMMDL_DRAW_PROC_LIST *list )
{
	return( list->draw_proc );
}

//--------------------------------------------------------------
/**
 * 削除関数取得
 * @param	list	FLDMMDL_DRAW_PROC_LIST *
 * @retval	FLDMMDL_DRAW_PROC_DEL		FLDMMDL_DRAW_PROC_DEL *
 */
//--------------------------------------------------------------
static FLDMMDL_DRAW_PROC_DEL fmmdl_DrawProcList_DeleteGet(
		const FLDMMDL_DRAW_PROC_LIST *list )
{
	return( list->delete_proc );
}

//--------------------------------------------------------------
/**
 * 退避関数取得
 * @param	list	FLDMMDL_DRAW_PROC_LIST *
 * @retval	FLDMMDL_DRAW_PROC_PUSH		FLDMMDL_DRAW_PROC_PUSH *
 */
//--------------------------------------------------------------
static FLDMMDL_DRAW_PROC_PUSH fmmdl_DrawProcList_PushGet(
		const FLDMMDL_DRAW_PROC_LIST *list )
{
	return( list->push_proc );
}

//--------------------------------------------------------------
/**
 * 復帰関数取得
 * @param	list	FLDMMDL_DRAW_PROC_LIST *
 * @retval	FLDMMDL_DRAW_PROC_PUSH		FLDMMDL_DRAW_PROC_PUSH *
 */
//--------------------------------------------------------------
static FLDMMDL_DRAW_PROC_POP fmmdl_DrawProcList_PopGet(
		const FLDMMDL_DRAW_PROC_LIST *list )
{
	return( list->pop_proc );
}

//--------------------------------------------------------------
/**
 * テーブルから指定のリスト取り出し
 * @param	code	HERO等
 * @retval	list	codeに対応したFLDMMDL_DRAW_PROC_LIST *
 */
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST * fmmdl_DrawProcListGet( u32 code )
{
#if 0
	const FLDMMDL_DRAW_PROC_LIST_REG *tbl = DATA_FieldOBJDrawProcListRegTbl;
	
	do{
		if( tbl->code == code ){
			return( tbl->list );
		}
		
		tbl++;
	}while( tbl->code != OBJCODEMAX );
	
	GF_ASSERT( 0 && "fmmdl_DrawProcListGet()不正なコード" );
	return( NULL );
#else
	return( NULL );
#endif
}

//==============================================================================
//	フィールド動作モデル ツール
//==============================================================================
//--------------------------------------------------------------
/**
 * 指定されたグリッドX,Z座標にいるOBJを取得
 * @param	sys			FLDMMDLSYS *
 * @param	x			検索グリッドX
 * @param	z			検索グリッドZ
 * @param	old_hit		TURE=過去座標も判定する
 * @retval	FLDMMDL	x,z位置にいるFLDMMDL * 。NULL=その座標にOBJはいない
 */
//--------------------------------------------------------------
FLDMMDL * FLDMMDL_SearchGridPos(
	const FLDMMDLSYS *sys, int x, int z, int old_hit )
{
	int max;
	FLDMMDL * fmmdl;
	
	max = FLDMMDLSYS_OBJMaxGet( sys );
	fmmdl = FLDMMDLSYS_fmmdlWorkGet( sys );
	
	do{
		if( FLDMMDL_StatusBit_Check(fmmdl,FLDMMDL_STABIT_USE) ){
			if( old_hit ){
				if( FLDMMDL_OldPosGX_Get(fmmdl) == x &&
					FLDMMDL_OldPosGZ_Get(fmmdl) == z ){
					return( fmmdl );
				}
			}
			
			if( FLDMMDL_NowPosGX_Get(fmmdl) == x &&
				FLDMMDL_NowPosGZ_Get(fmmdl) == z ){
				return( fmmdl );
			}
		}
		
		fmmdl++;
		max--;
	}while( max );
	
	return( NULL );
}

//--------------------------------------------------------------
/**
 * 座標、方向を初期化。fx32型
 * @param	fmmdl	FLDMMDL *
 * @param	vec		初期化座標
 * @param	dir		方向 DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_VecPosDirInit( FLDMMDL * fmmdl, const VecFx32 *vec, int dir )
{
	int gx,gy,gz;
	
	gx = SIZE_GRID_FX32( vec->x );
	FLDMMDL_NowPosGX_Set( fmmdl, gx );
	
	gy = SIZE_H_GRID_FX32( vec->y );
	FLDMMDL_NowPosGY_Set( fmmdl, gy );
	
	gz = SIZE_GRID_FX32( vec->z );
	FLDMMDL_NowPosGZ_Set( fmmdl, gz );
	
	FLDMMDL_VecPosSet( fmmdl, vec );
	FLDMMDL_GPosUpdate( fmmdl );
	
	FLDMMDL_DirDispSetForce( fmmdl, dir );
	
//	なんでいれてたのか
//	fmmdl_WorkInit_MoveProcInit( fmmdl );
//	fmmdl_WorkInit_DrawProcInit( fmmdl );
	
	FLDMMDL_AcmdFree( fmmdl );
	FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_MOVE_START );
	FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_MOVE | FLDMMDL_STABIT_MOVE_END );
}

//--------------------------------------------------------------
/**
 * 座標、方向を初期化。グリッド座標型
 * @param	fmmdl	FLDMMDL *
 * @param	x		グリッドX座標
 * @param	y		グリッドY座標
 * @param	z		グリッドZ座標
 * @param	dir		方向 DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_GPosDirInit( FLDMMDL * fmmdl, int x, int y, int z, int dir )
{
	VecFx32 vec;
	
	vec.x = GRID_SIZE_FX32( x ) + FLDMMDL_VEC_X_GRID_OFFS_FX32;
	FLDMMDL_NowPosGX_Set( fmmdl, x );
	
	vec.y = H_GRID_SIZE_FX32( y ) + FLDMMDL_VEC_Y_GRID_OFFS_FX32;
	FLDMMDL_NowPosGY_Set( fmmdl, y );
	
	vec.z = GRID_SIZE_FX32( z ) + FLDMMDL_VEC_Z_GRID_OFFS_FX32;
	FLDMMDL_NowPosGZ_Set( fmmdl, z );
	
	FLDMMDL_VecPosSet( fmmdl, &vec );
	FLDMMDL_GPosUpdate( fmmdl );
	
	FLDMMDL_DirDispSetForce( fmmdl, dir );
	
//	なんでいれてたのか
//	fmmdl_WorkInit_MoveProcInit( fmmdl );
//	fmmdl_WorkInit_DrawProcInit( fmmdl );

//	高さは引数の値を信用する
//	FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_MOVE_START | FLDMMDL_STABIT_HEIGHT_GET_NEED );
	FLDMMDL_StatusBit_ON( fmmdl, FLDMMDL_STABIT_MOVE_START );
	FLDMMDL_StatusBit_OFF( fmmdl, FLDMMDL_STABIT_MOVE | FLDMMDL_STABIT_MOVE_END );
	
	FLDMMDL_AcmdFree( fmmdl );
}

//--------------------------------------------------------------
/**
 * 動作コード変更
 * @param	fmmdl	FLDMMDL *
 * @param	code	MV_RND等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveCodeChange( FLDMMDL * fmmdl, u32 code )
{
	FLDMMDL_MoveDeleteProcCall( fmmdl );
	FLDMMDL_MoveCodeSet( fmmdl, code );
	fmmdl_WorkInit_MoveProcInit( fmmdl );
	FLDMMDL_MoveInit( fmmdl );
}

//--------------------------------------------------------------
/**
 * OBJ IDを変更
 * @param	fmmdl	FLDMMDL *
 * @param	id		OBJ ID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OBJIDChange( FLDMMDL * fmmdl, int id )
{
	FLDMMDL_OBJIDSet( fmmdl, id );
		
	FLDMMDL_StatusBitON_MoveStart( fmmdl );
	fmmdl_DrawEffectFlagInit( fmmdl );
}

//==============================================================================
//	動作関数ダミー
//==============================================================================
//--------------------------------------------------------------
/**
 * 動作初期化関数ダミー
 * @param	FLDMMDL	FLDMMDL * 
 * @retval	int			TRUE=初期化成功
 */
//--------------------------------------------------------------
void FLDMMDL_MoveInitProcDummy( FLDMMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 動作関数ダミー
 * @param	FLDMMDL	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveProcDummy( FLDMMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 動作削除関数ダミー
 * @param	FLDMMDL	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveDeleteProcDummy( FLDMMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 動作復帰関数ダミー
 * @param	FLDMMDL *	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_MoveReturnProcDummy( FLDMMDL * fmmdl )
{
}

//==============================================================================
//	描画関数ダミー
//==============================================================================
//--------------------------------------------------------------
/**
 * 描画初期化関数ダミー
 * @param	FLDMMDL	FLDMMDL * 
 * @retval	int			TRUE=初期化成功
 */
//--------------------------------------------------------------
void FLDMMDL_DrawInitProcDummy( FLDMMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画関数ダミー
 * @param	FLDMMDL	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawProcDummy( FLDMMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画削除関数ダミー
 * @param	FLDMMDL	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawDeleteProcDummy( FLDMMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画退避関数ダミー
 * @param	FLDMMDL	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawPushProcDummy( FLDMMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * 描画復帰関数ダミー
 * @param	FLDMMDL	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_DrawPopProcDummy( FLDMMDL * fmmdl )
{
}
