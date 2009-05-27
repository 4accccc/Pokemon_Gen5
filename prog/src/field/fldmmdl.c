//======================================================================
/**
 * @file	fldmmdl.c
 * @brief	フィールド動作モデル
 * @author	kagaya
 * @data	05.07.13
 */
//======================================================================
#include "fldmmdl.h"
#include "fldmmdl_procdraw.h"

#include "arc_def.h"
#include "arc/fieldmap/fldmmdl_mdlparam.naix"

//======================================================================
//	define
//======================================================================
//--------------------------------------------------------------
//	debug
//--------------------------------------------------------------

//--------------------------------------------------------------
//	FLDMMDL 動作、描画関数ワークサイズ (byte size)
//--------------------------------------------------------------
#define FLDMMDL_MOVE_WORK_SIZE		(16)	///<動作関数用ワークサイズ
#define FLDMMDL_MOVE_SUB_WORK_SIZE	(16)	///<動作サブ関数用ワークサイズ
#define FLDMMDL_MOVE_CMD_WORK_SIZE	(16)	///<動作コマンド用ワークサイズ
#define FLDMMDL_DRAW_WORK_SIZE		(32)	///<描画関数用ワークサイズ

//--------------------------------------------------------------
///	エイリアスシンボル
//--------------------------------------------------------------
enum
{
	RET_ALIES_NOT = 0,	//エイリアスではない
	RET_ALIES_EXIST,	//エイリアスとして既に存在している
	RET_ALIES_CHANGE,	//エイリアス変更が必要である
};

//======================================================================
//	struct
//======================================================================
//--------------------------------------------------------------
///	FLDMMDLSYS構造体
//--------------------------------------------------------------
struct _TAG_FLDMMDLSYS
{
	u32 status_bit;					///<ステータスビット
	u16 fmmdl_max;					///<FLDMMDL最大数
	HEAPID sysHeapID;				///<システム用 ヒープID
	FLDMMDL *pFldMMdlBuf;			///<FLDMMDLワーク *
	
	HEAPID heapID;					///<ヒープID
	s16 fmmdl_count;				///<フィールド動作モデル現在数
	u16 tcb_pri;					///<TCBプライオリティ
	u16 dummy;						///<余り
	const FLDMAPPER *pG3DMapper;	///<FLDMAPPER
	
	void *pTCBSysWork;				///<TCBワーク
	GFL_TCBSYS *pTCBSys;			///<TCBSYS*
	
	FLDMMDL_BLACTCONT *pBlActCont;	///<FLDMMDL_BLACTCONT
	
	u8 *pOBJCodeParamBuf;			///<OBJCODE_PARAMバッファ
	const OBJCODE_PARAM *pOBJCodeParamTbl; ///<OBJCODE_PARAM
};

#define FLDMMDLSYS_SIZE (sizeof(FLDMMDLSYS)) ///<FLDMMDLSYSサイズ

//--------------------------------------------------------------
///	FLDMMDL構造体
//--------------------------------------------------------------
struct _TAG_FLDMMDL
{
	u32 status_bit;				///<ステータスビット
	u32 move_bit;				///<動作ビット
	
	u16 obj_id;					///<OBJ ID
	u16 zone_id;				///<ゾーン ID
	u16 obj_code;				///<OBJコード
	u16 move_code;				///<動作コード
	u16 event_type;				///<イベントタイプ
	u16 event_flag;				///<イベントフラグ
	u16 event_id;				///<イベントID
	u16 dir_head;				///<FLDMMDL_H指定方向
	u16 dir_disp;				///<現在向いている方向
	u16 dir_move;				///<現在動いている方向
	u16 dir_disp_old;			///<過去の動いていた方向
	u16 dir_move_old;			///<過去の動いていた方向
	
	int param0;					///<ヘッダ指定パラメタ
	int param1;					///<ヘッダ指定パラメタ
	int param2;					///<ヘッダ指定パラメタ
	
	u16 acmd_code;				///<アニメーションコマンドコード
	u16 acmd_seq;				///<アニメーションコマンドシーケンス
	u16 draw_status;			///<描画ステータス
	s16 move_limit_x;			///<X方向移動制限
	s16 move_limit_z;			///<Z方向移動制限
	s16 gx_init;				///<初期グリッドX
	s16 gy_init;				///<初期グリッドY
	s16 gz_init;				///<初期グリッドZ
	s16 gx_old;					///<過去グリッドX
	s16 gy_old;					///<過去グリッドY
	s16 gz_old;					///<過去グリッドZ
	s16 gx_now;					///<現在グリッドX
	s16 gy_now;					///<現在グリッドY
	s16 gz_now;					///<現在グリッドZ
	VecFx32 vec_pos_now;		///<現在実数座標
	VecFx32 vec_draw_offs;		///<表示座標オフセット
	VecFx32 vec_draw_offs_outside;	///<外部指定表示座標オフセット
	VecFx32 vec_attr_offs;		///<アトリビュートによる座標オフセット
	u32 now_attr;				///<現在のマップアトリビュート
	u32 old_attr;				///<過去のマップアトリビュート
	
	GFL_TCB *pTCB;				///<動作関数TCB*
	const FLDMMDLSYS *pFldMMdlSys;///<FLDMMDLSYS*
	
	const FLDMMDL_MOVE_PROC_LIST *move_proc_list; ///<動作関数リスト
	const FLDMMDL_DRAW_PROC_LIST *draw_proc_list; ///<描画関数リスト
	
	u8 move_proc_work[FLDMMDL_MOVE_WORK_SIZE];///動作関数用ワーク
	u8 move_sub_proc_work[FLDMMDL_MOVE_SUB_WORK_SIZE];///動作サブ関数用ワーク
	u8 move_cmd_proc_work[FLDMMDL_MOVE_CMD_WORK_SIZE];///動作コマンド用ワーク
	u8 draw_proc_work[FLDMMDL_DRAW_WORK_SIZE];///描画関数用ワーク
};

#define FLDMMDL_SIZE (sizeof(FLDMMDL)) ///<FLDMMDLサイズ 224

//--------------------------------------------------------------
///	FLDMMDL_SAVEWORK構造体 size 80
//--------------------------------------------------------------
typedef struct
{
	u32 status_bit;			///<ステータスビット
	u32 move_bit;			///<動作ビット
	
	u8 obj_id;				///<OBJ ID
	u8 move_code;			///<動作コード
	s8 move_limit_x;		///<X方向移動制限
	s8 move_limit_z;		///<Z方向移動制限
	
	s8 dir_head;			///<FLDMMDL_H指定方向
	s8 dir_disp;			///<現在向いている方向
	s8 dir_move;			///<現在動いている方向
	u8 dummy;				///<ダミー
	
	u16 zone_id;			///<ゾーン ID
	u16 obj_code;			///<OBJコード
	u16 event_type;			///<イベントタイプ
	u16 event_flag;			///<イベントフラグ
	u16 event_id;			///<イベントID
	s16 param0;				///<ヘッダ指定パラメタ
	s16 param1;				///<ヘッダ指定パラメタ
	s16 param2;				///<ヘッダ指定パラメタ
	s16 gx_init;			///<初期グリッドX
	s16 gy_init;			///<初期グリッドY
	s16 gz_init;			///<初期グリッドZ
	s16 gx_now;				///<現在グリッドX
	s16 gy_now;				///<現在グリッドY
	s16 gz_now;				///<現在グリッドZ
	fx32 fx32_y;			///<fx32型の高さ値
	u8 move_proc_work[FLDMMDL_MOVE_WORK_SIZE];///<動作関数用ワーク
	u8 move_sub_proc_work[FLDMMDL_MOVE_SUB_WORK_SIZE];///<動作サブ関数用ワーク
}FLDMMDL_SAVEWORK;

//--------------------------------------------------------------
///	FLDMMDL_SAVEDATA構造体
//--------------------------------------------------------------
struct _TAG_FLDMMDL_SAVEDATA
{
	FLDMMDL_SAVEWORK SaveWorkBuf[FLDMMDL_SAVEMMDL_MAX];
};

//======================================================================
//	proto
//======================================================================
//FLDMMDLSYS プロセス
static void FldMMdlSys_DeleteProc( FLDMMDLSYS *fos );

//FLDMMDL 追加、削除
static void FldMMdl_SetHeader(
	FLDMMDL * fmmdl, const FLDMMDL_HEADER *head, void *sys );
static void FldMMdl_SetHeaderPos(FLDMMDL *fmmdl,const FLDMMDL_HEADER *head);
static void FldMMdl_InitWork( FLDMMDL * fmmdl, const FLDMMDLSYS *sys );
static void FldMMdl_InitCallMoveProcWork( FLDMMDL * fmmdl );
static void FldMMdl_InitMoveWork( const FLDMMDLSYS *fos, FLDMMDL * fmmdl );
#if 0
static void FldMMdlSys_CheckSetInitMoveWork( FLDMMDLSYS *fos );
static void FldMMdlSys_CheckSetInitDrawWork( FLDMMDLSYS *fos );
#endif

//FLDMMDL 動作関数
static void FldMMdl_TCB_MoveProc( GFL_TCB * tcb, void *work );
static void FldMMdl_TCB_DrawProc( FLDMMDL * fmmdl );

//FLDMMDL_SAVEDATA
static void FldMMdl_SaveData_SaveFldMMdl(
	const FLDMMDL *fmmdl, FLDMMDL_SAVEWORK *save );
static void FldMMdl_SaveData_LoadFldMMdl(
	FLDMMDL *fmmdl, const FLDMMDL_SAVEWORK *save, const FLDMMDLSYS *fos );

//FLDMMDLSYS 設定、参照
static void FldMMdlSys_OnStatusBit(
	FLDMMDLSYS *fmmdlsys, FLDMMDLSYS_STABIT bit );
static void FldMMdlSys_OffStatusBit(
	FLDMMDLSYS *fmmdlsys, FLDMMDLSYS_STABIT bit );
static void FldMMdlSys_IncrementOBJCount( FLDMMDLSYS *fmmdlsys );
static void FldMMdlSys_DecrementOBJCount( FLDMMDLSYS *fmmdlsys );

//FLDMMDLSYS ツール
static FLDMMDL * FldMMdlSys_SearchSpaceFldMMdl( const FLDMMDLSYS *sys );
static FLDMMDL * FldMMdlSys_SearchAlies(
	const FLDMMDLSYS *fos, int obj_id, int zone_id );

//FLDMMDL ツール
static void FldMMdl_AddTCB( FLDMMDL *fmmdl, const FLDMMDLSYS *sys );
static void FldMMdl_DeleteTCB( FLDMMDL *fmmdl );
static void FldMMdl_InitDrawWork( FLDMMDL *fmmdl );
static void FldMMdl_InitCallDrawProcWork( FLDMMDL * fmmdl );
static void FldMMdl_InitDrawEffectFlag( FLDMMDL * fmmdl );
static void FldMMdl_ClearWork( FLDMMDL *fmmdl );
static int FldMMdl_CheckHeaderAlies(
		const FLDMMDL *fmmdl, int h_zone_id, int max,
		const FLDMMDL_HEADER *head );
static FLDMMDL * FldMMdl_SearchOBJIDZoneID(
		const FLDMMDLSYS *fos, int obj_id, int zone_id );
static void FldMMdl_InitDrawStatus( FLDMMDL * fmmdl );
static void FldMMdl_SetDrawDeleteStatus( FLDMMDL * fmmdl );
static void FldMMdl_ChangeAliesOBJ(
	FLDMMDL *fmmdl, const FLDMMDL_HEADER *head, int zone_id );
static void FldMMdl_ChangeOBJAlies(
	FLDMMDL * fmmdl, int zone_id, const FLDMMDL_HEADER *head );

//OBJCODE_PARAM
static void FldMMdlSys_InitOBJCodeParam( FLDMMDLSYS *fmmdlsys );
static void FldMMdlSys_DeleteOBJCodeParam( FLDMMDLSYS *fmmdlsys );

//parts
static u16 WorkOBJCode_GetOBJCode( void *fsys, int code );
static const FLDMMDL_MOVE_PROC_LIST * MoveProcList_GetList( u16 code );
static const FLDMMDL_DRAW_PROC_LIST * DrawProcList_GetList(
		FLDMMDL_DRAWPROCNO no );
static BOOL FldMMdlHeader_CheckAlies( const FLDMMDL_HEADER *head );
static int FldMMdlHeader_GetAliesZoneID( const FLDMMDL_HEADER *head );

//======================================================================
//	フィールド動作モデル　システム
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDLSYS システム作成
 * @param	heapID	HEAPID
 * @param	max	動作モデル最大数
 * @retval	FLDMMDLSYS* 作成されたFLDMMDLSYS*
 */
//--------------------------------------------------------------
FLDMMDLSYS * FLDMMDLSYS_CreateSystem( HEAPID heapID, u32 max )
{
	FLDMMDLSYS *fos;
	fos = GFL_HEAP_AllocClearMemory( heapID, FLDMMDLSYS_SIZE );
	fos->pFldMMdlBuf = GFL_HEAP_AllocClearMemory( heapID, max*FLDMMDL_SIZE );
	fos->fmmdl_max = max;
	fos->sysHeapID = heapID;
	return( fos );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS システム開放
 * @param	fos	FLDMMDLSYS*
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_FreeSystem( FLDMMDLSYS *fos )
{
	GFL_HEAP_FreeMemory( fos->pFldMMdlBuf );
	GFL_HEAP_FreeMemory( fos->pFldMMdlBuf );
}

//======================================================================
//	フィールド動作モデル　システム　プロセス
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDLSYS システム 動作プロセスセットアップ
 * @param	fos	FLDMMDLSYS*
 * @param	heapID	プロセス用HEAPID
 * @param	pG3DMapper FLDMAPPER
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_SetupProc(
	FLDMMDLSYS *fos, HEAPID heapID, const FLDMAPPER *pG3DMapper )
{
	fos->heapID = heapID;
	fos->pG3DMapper = pG3DMapper;
	
	FldMMdlSys_InitOBJCodeParam( fos );
	
	fos->pTCBSysWork = GFL_HEAP_AllocMemory(
		heapID, GFL_TCB_CalcSystemWorkSize(fos->fmmdl_max) );
	fos->pTCBSys = GFL_TCB_Init( fos->fmmdl_max, fos->pTCBSysWork );
	
	FldMMdlSys_OnStatusBit( fos, FLDMMDLSYS_STABIT_MOVE_INIT_COMP );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS システム　動作プロセス削除
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdlSys_DeleteProc( FLDMMDLSYS *fos )
{
	FldMMdlSys_DeleteOBJCodeParam( fos );
	GFL_TCB_Exit( fos->pTCBSys );
	GFL_HEAP_FreeMemory( fos->pTCBSysWork );
	FldMMdlSys_OffStatusBit( fos, FLDMMDLSYS_STABIT_MOVE_INIT_COMP );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS 全てのプロセス削除
 * @param	fos		FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_DeleteProc( FLDMMDLSYS *fos )
{
//	FLDMMDLSYS_DeleteMMdl( fos );
	FLDMMDLSYS_DeleteDraw( fos );
	FldMMdlSys_DeleteProc( fos );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS 動作モデル更新
 * @param	fos	FLDMMDLSYS
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_UpdateProc( FLDMMDLSYS *fos )
{
	GFL_TCBSYS *tcbsys = fos->pTCBSys;
	GF_ASSERT( tcbsys != NULL );
	GFL_TCB_Main( tcbsys );
}

//======================================================================
//	フィールド動作モデル　システム　描画プロセス
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDLSYS システム　描画プロセスセットアップ
 * @param	fos	FLDMMDLSYS*
 * @param	heapID	プロセス用HEAPID
 * @param	pG3DMapper FLDMAPPER
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_SetupDrawProc( FLDMMDLSYS *fos )
{
	FLDMMDLSYS_SetCompleteDrawInit( fos, TRUE );
}

//======================================================================
//	フィールド動作モデル追加、削除
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDLSYS フィールド動作モデルを追加
 * @param	fos			FLDMMDLSYS *
 * @param	header		追加する情報を纏めたFLDMMDL_HEADER *
 * @param	zone_id		ゾーンID
 * @retval	FLDMMDL	追加されたFLDMMDL *
 */
//--------------------------------------------------------------
FLDMMDL * FLDMMDLSYS_AddFldMMdl(
	const FLDMMDLSYS *fos, const FLDMMDL_HEADER *header, int zone_id )
{
	FLDMMDL *fmmdl;
	FLDMMDL_HEADER header_data = *header;
	const FLDMMDL_HEADER *head = &header_data;
	
	fmmdl = FldMMdlSys_SearchSpaceFldMMdl( fos );
	GF_ASSERT( fmmdl != NULL );
	
	FldMMdl_SetHeader( fmmdl, head, NULL );
	FldMMdl_InitWork( fmmdl, fos );
	FLDMMDL_SetZoneID( fmmdl, zone_id );
	
	if( FLDMMDLSYS_CheckStatusBit(fos,FLDMMDLSYS_STABIT_MOVE_INIT_COMP) ){
		FldMMdl_InitMoveWork( fos, fmmdl );
		FLDMMDL_InitMoveProc( fmmdl );
	}
	
	if( FLDMMDLSYS_CheckStatusBit(fos,FLDMMDLSYS_STABIT_DRAW_INIT_COMP) ){
		FldMMdl_InitDrawWork( fmmdl );
	}
	
	FldMMdlSys_IncrementOBJCount( (FLDMMDLSYS*)FLDMMDL_GetFldMMdlSys(fmmdl) );
	return( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS フィールド動作モデルを追加　複数
 * @param	fos			FLDMMDLSYS *
 * @param	header		追加する情報を纏めたFLDMMDL_HEADER *
 * @param	zone_id		ゾーンID
 * @param	count		header要素数
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_SetFldMMdl( const FLDMMDLSYS *fos,
	const FLDMMDL_HEADER *header, int zone_id, int count )
{
	GF_ASSERT( count > 0 );
	GF_ASSERT( header != NULL );
	
	KAGAYA_Printf( "FLDMMDLSYS_SetFldMMdl Count %d\n", count );
	
	do{
		FLDMMDLSYS_AddFldMMdl( fos, header, zone_id );
		header++;
		count--;
	}while( count );
}

//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデルを削除
 * @param	fmmdl		削除するFLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_Delete( FLDMMDL * fmmdl )
{
	const FLDMMDLSYS *fos;
	
	fos = FLDMMDL_GetFldMMdlSys( fmmdl );
	
	if( FLDMMDLSYS_CheckCompleteDrawInit(fos) == TRUE ){
		FLDMMDL_CallDrawDeleteProc( fmmdl );
	}
	
	if( FLDMMDL_CheckMoveBit(fmmdl,FLDMMDL_MOVEBIT_MOVEPROC_INIT) ){
		FLDMMDL_CallMoveDeleteProc( fmmdl );
		FldMMdl_DeleteTCB( fmmdl );
	}
	
	FldMMdlSys_DecrementOBJCount( (FLDMMDLSYS*)(fmmdl->pFldMMdlSys) );
	FldMMdl_ClearWork( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS 現在発生しているフィールド動作モデルを全て削除
 * @param	fos	FLDMMDLSYS
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_DeleteMMdl( const FLDMMDLSYS *fos )
{
	u32 no = 0;
	FLDMMDL *fmmdl;
	
	while( FLDMMDLSYS_SearchUseFldMMdl(fos,&fmmdl,&no) ){
		FLDMMDL_Delete( fmmdl );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデル　ヘッダー情報反映
 * @param	fmmdl		設定するFLDMMDL * 
 * @param	head		反映する情報を纏めたFLDMMDL_HEADER *
 * @param	fsys		FIELDSYS_WORK *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_SetHeader(
	FLDMMDL * fmmdl, const FLDMMDL_HEADER *head, void *sys )
{
	FLDMMDL_SetOBJID( fmmdl, head->id );
	FLDMMDL_SetOBJCode( fmmdl, WorkOBJCode_GetOBJCode(sys,head->obj_code) );
	FLDMMDL_SetMoveCode( fmmdl, head->move_code );
	FLDMMDL_SetEventType( fmmdl, head->event_type );
	FLDMMDL_SetEventFlag( fmmdl, head->event_flag );
	FLDMMDL_SetEventID( fmmdl, head->event_id );
	fmmdl->dir_head = head->dir;
	FLDMMDL_SetParam( fmmdl, head->param0, FLDMMDL_PARAM_0 );
	FLDMMDL_SetParam( fmmdl, head->param1, FLDMMDL_PARAM_1 );
	FLDMMDL_SetParam( fmmdl, head->param2, FLDMMDL_PARAM_2 );
	FLDMMDL_SetMoveLimitX( fmmdl, head->move_limit_x );
	FLDMMDL_SetMoveLimitZ( fmmdl, head->move_limit_z );
	
	FldMMdl_SetHeaderPos( fmmdl, head );
}

//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデル　座標系初期化
 * @param	fmmdl		FLDMMDL * 
 * @param	head		反映する情報を纏めたFLDMMDL_HEADER *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_SetHeaderPos( FLDMMDL *fmmdl, const FLDMMDL_HEADER *head )
{
	int pos;
	VecFx32 vec;
	
	pos = head->gx;
	vec.x = GRID_SIZE_FX32( pos ) + FLDMMDL_VEC_X_GRID_OFFS_FX32;
	FLDMMDL_SetInitGridPosX( fmmdl, pos );
	FLDMMDL_SetOldGridPosX( fmmdl, pos );
	FLDMMDL_SetGridPosX( fmmdl, pos );
	
	pos = head->y;		//pos設定はfx32型で来る。
	vec.y = (fx32)pos;
	pos = SIZE_GRID_FX32( pos );
	FLDMMDL_SetInitGridPosY( fmmdl, pos );
	FLDMMDL_SetOldGridPosY( fmmdl, pos );
	FLDMMDL_SetGridPosY( fmmdl, pos );
	
	pos = head->gz;
	vec.z = GRID_SIZE_FX32( pos ) + FLDMMDL_VEC_Z_GRID_OFFS_FX32;
	FLDMMDL_SetInitGridPosZ( fmmdl, pos );
	FLDMMDL_SetOldGridPosZ( fmmdl, pos );
	FLDMMDL_SetGridPosZ( fmmdl, pos );
	
	FLDMMDL_SetVectorPos( fmmdl, &vec );
}

//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデル　ワーク初期化
 * @param	fmmdl		FLDMMDL * 
 * @param	sys			FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_InitWork( FLDMMDL * fmmdl, const FLDMMDLSYS *sys )
{
	FLDMMDL_OnStatusBit( fmmdl,
		FLDMMDL_STABIT_USE |				//使用中
		FLDMMDL_STABIT_HEIGHT_GET_ERROR |	//高さ取得が必要
		FLDMMDL_STABIT_ATTR_GET_ERROR );	//アトリビュート取得が必要
	
	if( FLDMMDL_CheckAliesEventID(fmmdl) == TRUE ){
		FLDMMDL_SetStatusBitAlies( fmmdl, TRUE );
	}
	
	fmmdl->pFldMMdlSys = sys;
	FLDMMDL_SetForceDirDisp( fmmdl, FLDMMDL_GetDirHeader(fmmdl) );
	FLDMMDL_SetDirMove( fmmdl, FLDMMDL_GetDirHeader(fmmdl) );
	FLDMMDL_FreeAcmd( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデル 動作関数初期化
 * @param	fmmdl		FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_InitCallMoveProcWork( FLDMMDL * fmmdl )
{
	fmmdl->move_proc_list =
		MoveProcList_GetList( FLDMMDL_GetMoveCode(fmmdl) );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作初期化に行う処理纏め
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_InitMoveWork( const FLDMMDLSYS *fos, FLDMMDL *fmmdl )
{
	FldMMdl_InitCallMoveProcWork( fmmdl );
	FldMMdl_AddTCB( fmmdl, fos );
	FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_MOVE_START );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS 動作初期化を行っていない動作モデルに対して初期化処理をセット
 * @param	fos	FLDMMDLSYS*
 * @retval	nothing
 */
//--------------------------------------------------------------
#if 0
static void FldMMdlSys_CheckSetInitMoveWork( FLDMMDLSYS *fos )
{
	u32 i = 0;
	FLDMMDL *fmmdl;
	
	while( FLDMMDLSYS_SearchUseFldMMdl(fos,&fmmdl,&i) == TRUE ){
		if( FLDMMDL_CheckMoveBit(fmmdl,	//初期化関数呼び出しまだ
			FLDMMDL_MOVEBIT_MOVEPROC_INIT) == 0 ){
			FLDMMDL_InitMoveProc( fmmdl );
		}
	}
}
#endif

//--------------------------------------------------------------
/**
 * FLDMMDLSYS 描画初期化を行っていない動作モデルに対して初期化処理をセット
 * @param	fos	FLDMMDLSYS*
 * @retval	nothing
 */
//--------------------------------------------------------------
#if 0
static void FldMMdlSys_CheckSetInitDrawWork( FLDMMDLSYS *fos )
{
	u32 i = 0;
	FLDMMDL *fmmdl;
	
	while( FLDMMDLSYS_SearchUseFldMMdl(fos,&fmmdl,&i) == TRUE ){
		if( FLDMMDL_CheckStatusBitCompletedDrawInit(fmmdl) == FALSE ){
			FldMMdl_InitDrawWork( fmmdl );
		}
	}
}
#endif

//======================================================================
//	FLDMMDLSYS PUSH POP
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDLSYS 動作モデル 退避
 * @param	fmmdlsys	FLDMMDLSYS
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_Push( FLDMMDLSYS *fmmdlsys )
{
	u32 no = 0;
	FLDMMDL *fmmdl;
	
	#ifdef DEBUG_FLDMMDL_PRINT
	if( FLDMMDLSYS_CheckCompleteDrawInit(fmmdlsys) == FALSE ){
		GF_ASSERT( 0 && "WARNING!! 動作モデル 描画未初期化\n" );
	}
	#endif
	
	while( FLDMMDLSYS_SearchUseFldMMdl(fmmdlsys,&fmmdl,&no) == TRUE ){
		{ //動作処理の退避
			FldMMdl_DeleteTCB( fmmdl );
			FLDMMDL_OnMoveBit( fmmdl,
				FLDMMDL_MOVEBIT_NEED_MOVEPROC_RECOVER );
		}
		
		{ //描画処理の退避
			FLDMMDL_CallDrawPushProc( fmmdl );
			FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_DRAW_PUSH );
		}
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS 動作モデル復帰
 * @param	fmmdlsys	FLDMMDLSYS
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_Pop( FLDMMDLSYS *fmmdlsys )
{
	u32 no = 0;
	FLDMMDL *fmmdl;
	
	while( FLDMMDLSYS_SearchUseFldMMdl(fmmdlsys,&fmmdl,&no) == TRUE ){
		{	//動作処理復帰
			FldMMdl_InitMoveWork( fmmdlsys, fmmdl ); //ワーク初期化
			
			if( FLDMMDL_CheckMoveBit(fmmdl,	//初期化関数呼び出しまだ
				FLDMMDL_MOVEBIT_MOVEPROC_INIT) == 0 ){
				FLDMMDL_InitMoveProc( fmmdl );
			}
			
			if( FLDMMDL_CheckMoveBit(fmmdl, //復元関数呼び出しが必要
				FLDMMDL_MOVEBIT_NEED_MOVEPROC_RECOVER) ){
				FLDMMDL_CallMovePopProc( fmmdl );
				FLDMMDL_OffMoveBit( fmmdl,
					FLDMMDL_MOVEBIT_NEED_MOVEPROC_RECOVER );
			}
		}
		
		{	//描画処理復帰
			if( FLDMMDL_CheckStatusBitCompletedDrawInit(fmmdl) == FALSE ){
				FldMMdl_InitDrawWork( fmmdl );
			}
			
			if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_DRAW_PUSH) ){
				FLDMMDL_CallDrawPopProc( fmmdl );
				FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_DRAW_PUSH );
			}
		}
	}
}

//======================================================================
//	FLDMMDL_SAVEDATA
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDL_SAVEDATA セーブデータ バッファサイズ取得
 * @param	nothing
 * @retval	u32	サイズ
 */
//--------------------------------------------------------------
u32 FLDMMDL_SAVEDATA_GetWorkSize( void )
{
	return( sizeof(FLDMMDL_SAVEDATA) );
}

//--------------------------------------------------------------
/**
 * FLDMMDL_SAVEDATA セーブデータ バッファ初期化
 * @param	p	FLDMMDL_SAVEDATA
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SAVEDATA_Init( void *p )
{
	MI_CpuClear8( p, FLDMMDL_SAVEDATA_GetWorkSize() );
}

//--------------------------------------------------------------
/**
 * FLDMMDL_SAVEDATA 動作モデルセーブ
 * @param	fmmdlsys セーブするFLDMMDLSYS
 * @param	savedata LDMMDL_SAVEDATA
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SAVEDATA_Save(
	FLDMMDLSYS *fmmdlsys, FLDMMDL_SAVEDATA *savedata )
{
	u32 no = 0;
	FLDMMDL *fmmdl;
	FLDMMDL_SAVEWORK *save = savedata->SaveWorkBuf;
	
	while( FLDMMDLSYS_SearchUseFldMMdl(fmmdlsys,&fmmdl,&no) == TRUE ){
		FldMMdl_SaveData_SaveFldMMdl( fmmdl, save );
		save++;
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL_SAVEDATA 動作モデルロード
 * @param	fmmdlsys	FLDMMDLSYS
 * @param	save	ロードするFLDMMDL_SAVEWORK
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SAVEDATA_Load(
	FLDMMDLSYS *fmmdlsys, FLDMMDL_SAVEDATA *savedata )
{
	u32 no = 0;
	FLDMMDL *fmmdl;
	FLDMMDL_SAVEWORK *save = savedata->SaveWorkBuf;
	
	while( no < FLDMMDL_SAVEMMDL_MAX ){
		if( (save->status_bit&FLDMMDL_STABIT_USE) ){
			fmmdl = FldMMdlSys_SearchSpaceFldMMdl( fmmdlsys );
			FldMMdl_SaveData_LoadFldMMdl( fmmdl, save, fmmdlsys );
		}
		save++;
		no++;
	}

  fmmdlsys->fmmdl_count = no;
}

//--------------------------------------------------------------
/**
 * FLDMMDL_SAVEDATA 動作モデル　セーブ
 * @param	fldmmdl		セーブするFLDMMDL*
 * @param	save FLDMMDL_SAVEWORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_SaveData_SaveFldMMdl(
	const FLDMMDL *fmmdl, FLDMMDL_SAVEWORK *save )
{
	save->status_bit = FLDMMDL_GetStatusBit( fmmdl );
	save->move_bit = FLDMMDL_GetMoveBit( fmmdl );
	save->obj_id = FLDMMDL_GetOBJID( fmmdl );
	save->zone_id = FLDMMDL_GetZoneID( fmmdl );
	save->obj_code = FLDMMDL_GetOBJCode( fmmdl );
	save->move_code = FLDMMDL_GetMoveCode( fmmdl );
	save->event_type = FLDMMDL_GetEventType( fmmdl );
	save->event_flag = FLDMMDL_GetEventFlag( fmmdl );
	save->event_id = FLDMMDL_GetEventID( fmmdl );
	save->dir_head = FLDMMDL_GetDirHeader( fmmdl );
	save->dir_disp = FLDMMDL_GetDirDisp( fmmdl );
	save->dir_move = FLDMMDL_GetDirMove( fmmdl );
	save->param0 = FLDMMDL_GetParam( fmmdl, FLDMMDL_PARAM_0 );
	save->param1 = FLDMMDL_GetParam( fmmdl, FLDMMDL_PARAM_1 );
	save->param2 = FLDMMDL_GetParam( fmmdl, FLDMMDL_PARAM_2 );
	save->move_limit_x = FLDMMDL_GetMoveLimitX( fmmdl );
	save->move_limit_z = FLDMMDL_GetMoveLimitZ( fmmdl );
	save->gx_init = FLDMMDL_GetInitGridPosX( fmmdl );
	save->gy_init = FLDMMDL_GetInitGridPosY( fmmdl );
	save->gz_init = FLDMMDL_GetInitGridPosZ( fmmdl );
	save->gx_now = FLDMMDL_GetGridPosX( fmmdl );
	save->gy_now = FLDMMDL_GetGridPosY( fmmdl );
	save->gz_now = FLDMMDL_GetGridPosZ( fmmdl );
	save->fx32_y = FLDMMDL_GetVectorPosY( fmmdl );
	
	MI_CpuCopy8( FLDMMDL_GetMoveProcWork((FLDMMDL*)fmmdl),
		save->move_proc_work, FLDMMDL_MOVE_WORK_SIZE );
	MI_CpuCopy8( FLDMMDL_GetMoveSubProcWork((FLDMMDL*)fmmdl),
		save->move_sub_proc_work, FLDMMDL_MOVE_SUB_WORK_SIZE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL_SAVEDATA 動作モデル　ロード
 * @param	fldmmdl		セーブするFLDMMDL*
 * @param	save FLDMMDL_SAVEWORK
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_SaveData_LoadFldMMdl(
	FLDMMDL *fmmdl, const FLDMMDL_SAVEWORK *save, const FLDMMDLSYS *fos )
{
	FldMMdl_ClearWork( fmmdl );

	fmmdl->status_bit = save->status_bit;
	fmmdl->move_bit = save->move_bit;
	FLDMMDL_SetOBJID( fmmdl, save->obj_id );
	FLDMMDL_SetZoneID( fmmdl, save->zone_id );
	FLDMMDL_SetOBJCode( fmmdl, save->obj_code ); 
	FLDMMDL_SetMoveCode( fmmdl, save->move_code );
	FLDMMDL_SetEventType( fmmdl, save->event_type );
	FLDMMDL_SetEventFlag( fmmdl, save->event_flag );
	FLDMMDL_SetEventID( fmmdl, save->event_id );
	fmmdl->dir_head = save->dir_head;
	FLDMMDL_SetForceDirDisp( fmmdl, save->dir_disp );
	FLDMMDL_SetDirMove( fmmdl, save->dir_move );
	FLDMMDL_SetParam( fmmdl, save->param0, FLDMMDL_PARAM_0 );
	FLDMMDL_SetParam( fmmdl, save->param1, FLDMMDL_PARAM_1 );
	FLDMMDL_SetParam( fmmdl, save->param2, FLDMMDL_PARAM_2 );
	FLDMMDL_SetMoveLimitX( fmmdl, save->move_limit_x );
	FLDMMDL_SetMoveLimitZ( fmmdl, save->move_limit_z );
	FLDMMDL_SetInitGridPosX( fmmdl, save->gx_init );
	FLDMMDL_SetInitGridPosY( fmmdl, save->gy_init );
	FLDMMDL_SetInitGridPosZ( fmmdl, save->gz_init );
	FLDMMDL_SetGridPosX( fmmdl, save->gx_now );
	FLDMMDL_SetGridPosY( fmmdl, save->gy_now );
	FLDMMDL_SetGridPosZ( fmmdl, save->gz_now );
	
	MI_CpuCopy8( save->move_proc_work,
		FLDMMDL_GetMoveProcWork((FLDMMDL*)fmmdl), FLDMMDL_MOVE_WORK_SIZE );
	MI_CpuCopy8( save->move_sub_proc_work,
		FLDMMDL_GetMoveSubProcWork((FLDMMDL*)fmmdl), FLDMMDL_MOVE_SUB_WORK_SIZE );
	
	fmmdl->pFldMMdlSys = fos;
	
	{ //座標復帰
		s16 grid;
		VecFx32 pos = {0,0,0};
		
		grid = FLDMMDL_GetGridPosX( fmmdl );
		FLDMMDL_SetOldGridPosX( fmmdl, grid );
		pos.x = GRID_SIZE_FX32( grid ) + FLDMMDL_VEC_X_GRID_OFFS_FX32;
		
		grid = FLDMMDL_GetGridPosY( fmmdl );
		FLDMMDL_SetOldGridPosY( fmmdl, grid );
		pos.y = save->fx32_y; //セーブ時の高さを信用する
	
		grid = FLDMMDL_GetGridPosZ( fmmdl );
		FLDMMDL_SetOldGridPosZ( fmmdl, grid );
		pos.z = GRID_SIZE_FX32( grid ) + FLDMMDL_VEC_Z_GRID_OFFS_FX32;
	
		FLDMMDL_SetVectorPos( fmmdl, &pos );
	}

	{ //ステータスビット復帰
		FLDMMDL_OnStatusBit( fmmdl,
			FLDMMDL_STABIT_USE |
//			FLDMMDL_STABIT_HEIGHT_GET_NEED | //セーブ時の高さを信用する
			FLDMMDL_STABIT_MOVE_START );
		
		FLDMMDL_OffStatusBit( fmmdl,
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
		
		FLDMMDL_OffStatusBit( fmmdl,
			FLDMMDL_STABIT_SHADOW_SET |
			FLDMMDL_STABIT_SHADOW_VANISH |
			FLDMMDL_STABIT_EFFSET_SHOAL	 |
			FLDMMDL_STABIT_REFLECT_SET );
		
		if( FLDMMDL_CheckMoveBit(fmmdl,FLDMMDL_MOVEBIT_MOVEPROC_INIT) == 0 ){
			FLDMMDL_OnMoveBit( fmmdl, FLDMMDL_MOVEBIT_NEED_MOVEPROC_RECOVER );
		}
	}
}


//======================================================================
//	FLDMMDL 動作関数
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDL 動作部分実行
 * @param	fmmdl	FLDMMDL*
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_UpdateMoveProc( FLDMMDL *fmmdl )
{
	FLDMMDL_UpdateMove( fmmdl );
	
	if( FLDMMDL_CheckStatusBitUse(fmmdl) == TRUE ){
		FldMMdl_TCB_DrawProc( fmmdl );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL TCB 動作関数
 * @param	tcb		GFL_TCB *
 * @param	work	tcb work
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_TCB_MoveProc( GFL_TCB * tcb, void *work )
{
	FLDMMDL *fmmdl = (FLDMMDL *)work;
	FLDMMDL_UpdateMoveProc( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL TCB 動作関数から呼ばれる描画関数
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_TCB_DrawProc( FLDMMDL * fmmdl )
{
	const FLDMMDLSYS *fos = FLDMMDL_GetFldMMdlSys(fmmdl);
	
	if( FLDMMDLSYS_CheckCompleteDrawInit(fos) == TRUE ){
		FLDMMDL_UpdateDraw( fmmdl );
	}
}

//======================================================================
//	FLDMMDL アニメーションコマンド
//======================================================================
//--------------------------------------------------------------
/**
 * アニメーションコマンドが可能かチェック
 * @param	fmmdl		対象となるFLDMMDL * 
 * @retval	int			TRUE=可能。FALSE=無理
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckPossibleAcmd( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_USE) == 0 ){
		return( FALSE );
	}
	
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_MOVE) ){
		return( FALSE );
	}
	
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_ACMD) &&
		FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_ACMD_END) == 0 ){
		return( FALSE );
	}
	
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * アニメーションコマンドセット
 * @param	fmmdl		対象となるFLDMMDL * 
 * @param	code		実行するコード。AC_DIR_U等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetAcmd( FLDMMDL * fmmdl, u16 code )
{
	GF_ASSERT( code < ACMD_MAX );
	FLDMMDL_SetAcmdCode( fmmdl, code );
	FLDMMDL_SetAcmdSeq( fmmdl, 0 );
	FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_ACMD );
	FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_ACMD_END );
}

//--------------------------------------------------------------
/**
 * コマンドセット
 * @param	fmmdl		対象となるFLDMMDL * 
 * @param	code		実行するコード。AC_DIR_U等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetLocalAcmd( FLDMMDL * fmmdl, u16 code )
{
	FLDMMDL_SetAcmdCode( fmmdl, code );
	FLDMMDL_SetAcmdSeq( fmmdl, 0 );
	FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_ACMD_END );
}

//--------------------------------------------------------------
/**
 * アニメーションコマンド終了チェック。
 * @param	fmmdl		対象となるFLDMMDL * 
 * @retval	int			TRUE=終了
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckEndAcmd( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_ACMD) == 0 ){
		return( TRUE );
	}
	
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_ACMD_END) == 0 ){
		return( FALSE );
	}
	
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * アニメーションコマンド終了チェックと開放。
 * アニメーションコマンドが終了していない場合は開放されない。
 * @param	fmmdl		対象となるFLDMMDL * 
 * @retval	BOOL	TRUE=終了している。
 */
//--------------------------------------------------------------
BOOL FLDMMDL_EndAcmd( FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_ACMD) == 0 ){
		return( TRUE );
	}
	
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_ACMD_END) == 0 ){
		return( FALSE );
	}
	
	FLDMMDL_OffStatusBit(
		fmmdl, FLDMMDL_STABIT_ACMD|FLDMMDL_STABIT_ACMD_END );
	
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * アニメーションコマンド開放。
 * アニメーションコマンドが終了していなくとも強制開放。
 * @param	fmmdl		対象となるFLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_FreeAcmd( FLDMMDL * fmmdl )
{
	FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_ACMD );
	FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_ACMD_END ); //ローカルコマンドフラグ
	FLDMMDL_SetAcmdCode( fmmdl, ACMD_NOT );
	FLDMMDL_SetAcmdSeq( fmmdl, 0 );
}

//======================================================================
//	FLDMMDLSYS パラメタ設定、参照
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDLSYS ステータスビットチェック
 * @param	fmmdlsys	FLDMMDLSYS*
 * @param	bit	FLDMMDLSYS_STABIT
 * @retval	u32	(status bit & bit)
 */
//--------------------------------------------------------------
u32 FLDMMDLSYS_CheckStatusBit(
	const FLDMMDLSYS *fmmdlsys, FLDMMDLSYS_STABIT bit )
{
	return( (fmmdlsys->status_bit&bit) );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS ステータスビット ON
 * @param	fmmdlsys	FLDMMDLSYS*
 * @param	bit	FLDMMDLSYS_STABIT
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdlSys_OnStatusBit(
	FLDMMDLSYS *fmmdlsys, FLDMMDLSYS_STABIT bit )
{
	fmmdlsys->status_bit |= bit;
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS ステータスビット OFF
 * @param	fmmdlsys	FLDMMDLSYS*
 * @param	bit	FLDMMDLSYS_STABIT
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdlSys_OffStatusBit(
	FLDMMDLSYS *fmmdlsys, FLDMMDLSYS_STABIT bit )
{
	fmmdlsys->status_bit &= ~bit;
}

//--------------------------------------------------------------
/**
 * FMMDLSYS 動作モデル最大数を取得
 * @param	fmmdlsys	FLDMMDLSYS*
 * @retval	u16	最大数
 */
//--------------------------------------------------------------
u16 FLDMMDLSYS_GetFldMMdlMax( const FLDMMDLSYS *fmmdlsys )
{
	return( fmmdlsys->fmmdl_max );
}

//--------------------------------------------------------------
/**
 * FMMDLSYS 現在発生している動作モデルの数を取得
 * @param	fmmdlsys	FLDMMDLSYS*
 * @retval	u16	発生数
 */
//--------------------------------------------------------------
u16 FLDMMDLSYS_GetFldMMdlCount( const FLDMMDLSYS *fmmdlsys )
{
	return( fmmdlsys->fmmdl_count );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS TCBプライオリティを取得
 * @param	fmmdlsys	FLDMMDLSYS*
 * @retval	u16	TCBプライオリティ
 */
//--------------------------------------------------------------
u16 FLDMMDLSYS_GetTCBPriority( const FLDMMDLSYS *fmmdlsys )
{
	return( fmmdlsys->tcb_pri );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS HEAPID取得
 * @param	fmmdlsys	FLDMMDLSYS*
 * @retval	HEAPID HEAPID
 */
//--------------------------------------------------------------
HEAPID FLDMMDLSYS_GetHeapID( const FLDMMDLSYS *fmmdlsys )
{
	return( fmmdlsys->heapID );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS 現在発生しているOBJ数を1増加
 * @param	fmmdlsys	FLDMMDLSYS*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdlSys_IncrementOBJCount( FLDMMDLSYS *fmmdlsys )
{
	fmmdlsys->fmmdl_count++;
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS 現在発生しているOBJ数を1減少
 * @param	fmmdlsys	FLDMMDLSYS*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdlSys_DecrementOBJCount( FLDMMDLSYS *fmmdlsys )
{
	fmmdlsys->fmmdl_count--;
	GF_ASSERT( fmmdlsys->fmmdl_count >= 0 );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS GFL_TCBSYS*取得
 * @param	fos	FLDMMDLSYS*
 * @retval	GFL_TCBSYS*
 */
//--------------------------------------------------------------
GFL_TCBSYS * FLDMMDLSYS_GetTCBSYS( FLDMMDLSYS *fos )
{
	return( fos->pTCBSys );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS FLDMMDL_BLACTCONTセット
 * @param	fmmdlsys	FLDMMDLSYS
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_SetBlActCont(
	FLDMMDLSYS *fmmdlsys, FLDMMDL_BLACTCONT *pBlActCont )
{
	fmmdlsys->pBlActCont = pBlActCont;
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS FLDMMDL_BLACTCONT取得
 * @param	fmmdlsys FLDMMDLSYS
 * @retval	FLDMMDL_BLACTCONT*
 */
//--------------------------------------------------------------
FLDMMDL_BLACTCONT * FLDMMDLSYS_GetBlActCont( FLDMMDLSYS *fmmdlsys )
{
	GF_ASSERT( fmmdlsys->pBlActCont != NULL );
	return( fmmdlsys->pBlActCont );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS FLDMAPPER取得
 * @param	fmmdlsys	FLDMMDLSYS
 * @retval	FLDMAPPER* FLDMAPPER*
 */
//--------------------------------------------------------------
const FLDMAPPER * FLDMMDLSYS_GetG3DMapper( const FLDMMDLSYS *fos )
{
	GF_ASSERT( fos->pG3DMapper != NULL);
	return( fos->pG3DMapper );
}

//======================================================================
//	FLDMMDL　パラメタ参照、設定
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDL ステータスビットON
 * @param	fmmdl	fmmdl
 * @param	bit		FLDMMDL_STABIT
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OnStatusBit( FLDMMDL *fmmdl, FLDMMDL_STABIT bit )
{
	fmmdl->status_bit |= bit;
}

//--------------------------------------------------------------
/**
 * FLDMMDL ステータスビットOFF
 * @param	fmmdl	fmmdl
 * @param	bit		FLDMMDL_STABIT
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OffStatusBit( FLDMMDL *fmmdl, FLDMMDL_STABIT bit )
{
	fmmdl->status_bit &= ~bit;
}

//--------------------------------------------------------------
/**
 * FLDMMDL ステータスビット取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	FLDMMDL_STABIT ステータスビット
 */
//--------------------------------------------------------------
FLDMMDL_STABIT FLDMMDL_GetStatusBit( const FLDMMDL * fmmdl )
{
	return( fmmdl->status_bit );
}

//--------------------------------------------------------------
/**
 * FLDMMDL ステータスビットチェック
 * @param	fmmdl	FLDMMDL*
 * @param	bit	FLDMMDL_STABIT
 * @retval	u32	(status bit & bit)
 */
//--------------------------------------------------------------
u32 FLDMMDL_CheckStatusBit( const FLDMMDL *fmmdl, FLDMMDL_STABIT bit )
{
	return( (fmmdl->status_bit&bit) );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作ビット　取得
 * @param	fmmdl	FLDMMDL *
 * @retval	u32		動作ビット
 */
//--------------------------------------------------------------
FLDMMDL_MOVEBIT FLDMMDL_GetMoveBit( const FLDMMDL * fmmdl )
{
	return( fmmdl->move_bit );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作ビット　ON
 * @param	fmmdl	FLDMMDL *
 * @param	bit		立てるビット FLDMMDL_MOVEBIT_NON等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OnMoveBit( FLDMMDL * fmmdl, FLDMMDL_MOVEBIT bit )
{
	fmmdl->move_bit |= bit;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作ビット　OFF
 * @param	fmmdl	FLDMMDL *
 * @param	bit		下ろすビット FLDMMDL_MOVEBIT_NON等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OffMoveBit( FLDMMDL * fmmdl, FLDMMDL_MOVEBIT bit )
{
	fmmdl->move_bit &= ~bit;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作ビット　チェック
 * @param	fmmdl	FLDMMDL *
 * @param	bit		チェックするビット FLDMMDL_MOVEBIT_NON等
 * @retval	nothing
 */
//--------------------------------------------------------------
u32 FLDMMDL_CheckMoveBit( const FLDMMDL * fmmdl, FLDMMDL_MOVEBIT bit )
{
	return( (fmmdl->move_bit & bit) );
}

//--------------------------------------------------------------
/**
 * FLDMMDL OBJ IDセット
 * @param	fmmdl	FLDMMDL * 
 * @param	id		obj id
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetOBJID( FLDMMDL * fmmdl, u16 obj_id )
{
	fmmdl->obj_id = obj_id;
}

//--------------------------------------------------------------
/**
 * FLDMMDL OBJ ID取得
 * @param	fmmdl	FLDMMDL *
 * @retval	u16		OBJ ID
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetOBJID( const FLDMMDL * fmmdl )
{
	return( fmmdl->obj_id );
}

//--------------------------------------------------------------
/**
 * FLDMMDL ZONE IDセット
 * @param	fmmdl	FLDMMDL *
 * @param	zone_id	ゾーンID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetZoneID( FLDMMDL * fmmdl, u16 zone_id )
{
	fmmdl->zone_id = zone_id;
}

//--------------------------------------------------------------
/**
 * FLDMMDL ZONE ID取得
 * @param	fmmdl	FLDMMDL *
 * @retval	int		ZONE ID
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetZoneID( const FLDMMDL * fmmdl )
{
	return( fmmdl->zone_id );
}

//--------------------------------------------------------------
/**
 * FLDMMDL OBJコードセット
 * @param	fmmdl			FLDMMDL * 
 * @param	code			セットするコード
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetOBJCode( FLDMMDL * fmmdl, u16 code )
{
	fmmdl->obj_code = code;
}

//--------------------------------------------------------------
/**
 * FLDMMDL OBJコード取得
 * @param	fmmdl			FLDMMDL * 
 * @retval	u16				OBJコード
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetOBJCode( const FLDMMDL * fmmdl )
{
	return( fmmdl->obj_code );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作コードセット
 * @param	fmmdl			FLDMMDL * 
 * @param	code			動作コード
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetMoveCode( FLDMMDL * fmmdl, u16 code )
{
	fmmdl->move_code = code;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作コード取得
 * @param	fmmdl			FLDMMDL * 
 * @retval	u32				動作コード
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetMoveCode( const FLDMMDL * fmmdl )
{
	return( fmmdl->move_code );
}

//--------------------------------------------------------------
/**
 * FLDMMDL イベントタイプセット
 * @param	fmmdl		FLDMMDL * 
 * @param	type		Event Type
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetEventType( FLDMMDL * fmmdl, u16 type )
{
	fmmdl->event_type = type;
}

//--------------------------------------------------------------
/**
 * FLDMMDL イベントタイプ取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	u32			Event Type
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetEventType( const FLDMMDL * fmmdl )
{
	return( fmmdl->event_type );
}

//--------------------------------------------------------------
/**
 * FLDMMDL イベントフラグセット
 * @param	fmmdl		FLDMMDL * 
 * @param	flag		Event Flag
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetEventFlag( FLDMMDL * fmmdl, u16 flag )
{
	fmmdl->event_flag = flag;
}

//--------------------------------------------------------------
/**
 * FLDMMDL イベントフラグ取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	u16			Event Flag
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetEventFlag( const FLDMMDL * fmmdl )
{
	return( fmmdl->event_flag );
}

//--------------------------------------------------------------
/**
 * FLDMMDL イベントIDセット
 * @param	fmmdl		FLDMMDL *
 * @param	id			Event ID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetEventID( FLDMMDL * fmmdl, u16 id )
{
	fmmdl->event_id = id;
}

//--------------------------------------------------------------
/**
 * FLDMMDL イベントID取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	u16			Event ID
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetEventID( const FLDMMDL * fmmdl )
{
	return( fmmdl->event_id );
}

//--------------------------------------------------------------
/**
 * FLDMMDL イベントIDがエイリアスかどうかチェック
 * @param	fmmdl		FLDMMDL * 
 * @retval	int			TRUE=エイリアスIDである FALSE=違う
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckAliesEventID( const FLDMMDL * fmmdl )
{
	u16 id = (u16)FLDMMDL_GetEventID( fmmdl );
	
	if( id == SP_SCRID_ALIES ){
		return( TRUE );
	}

	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL ヘッダー指定方向取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	u32			DIR_UP等
 */
//--------------------------------------------------------------
u32 FLDMMDL_GetDirHeader( const FLDMMDL * fmmdl )
{
	return( fmmdl->dir_head );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 表示方向セット　強制
 * @param	fmmdl			FLDMMDL * 
 * @param	dir				DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetForceDirDisp( FLDMMDL * fmmdl, u16 dir )
{
	fmmdl->dir_disp_old = fmmdl->dir_disp;
	fmmdl->dir_disp = dir;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 表示方向セット
 * @param	fmmdl			FLDMMDL * 
 * @param	dir				DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetDirDisp( FLDMMDL * fmmdl, u16 dir )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_PAUSE_DIR) == 0 ){
		fmmdl->dir_disp_old = fmmdl->dir_disp;
		fmmdl->dir_disp = dir;
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL 表示方向取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	u16 	DIR_UP等
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetDirDisp( const FLDMMDL * fmmdl )
{
	return( fmmdl->dir_disp );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 過去表示方向取得
 * @param	fmmdl			FLDMMDL * 
 * @retval	dir				DIR_UP等
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetDirDispOld( const FLDMMDL * fmmdl )
{
	return( fmmdl->dir_disp_old );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動方向セット
 * @param	fmmdl			FLDMMDL * 
 * @param	dir				DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetDirMove( FLDMMDL * fmmdl, u16 dir )
{
	fmmdl->dir_move_old = fmmdl->dir_move;
	fmmdl->dir_move = dir;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動方向取得
 * @param	fmmdl			FLDMMDL * 
 * @retval	u16		DIR_UP等
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetDirMove( const FLDMMDL * fmmdl )
{
	return( fmmdl->dir_move );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 過去移動方向取得
 * @param	fmmdl			FLDMMDL * 
 * @retval	u16	DIR_UP等
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetDirMoveOld( const FLDMMDL * fmmdl )
{
	return( fmmdl->dir_move_old );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 表示、移動方向セット
 * @param	fmmdl			FLDMMDL * 
 * @param	dir				DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetDirAll( FLDMMDL * fmmdl, u16 dir )
{
	FLDMMDL_SetDirDisp( fmmdl, dir );
	FLDMMDL_SetDirMove( fmmdl, dir );
}

//--------------------------------------------------------------
/**
 * FLDMMDL ヘッダー指定パラメタセット
 * @param	fmmdl	FLDMMDL * 
 * @param	param	パラメタ
 * @param	no		セットするパラメタ番号　FLDMMDL_PARAM_0等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetParam( FLDMMDL *fmmdl, int param, FLDMMDL_H_PARAM no )
{
	switch( no ){
	case FLDMMDL_PARAM_0: fmmdl->param0 = param; break;
	case FLDMMDL_PARAM_1: fmmdl->param1 = param; break;
	case FLDMMDL_PARAM_2: fmmdl->param2 = param; break;
	default: GF_ASSERT( 0 );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL ヘッダー指定パラメタ取得
 * @param	fmmdl			FLDMMDL *
 * @param	param			FLDMMDL_PARAM_0等
 * @retval	int				パラメタ
 */
//--------------------------------------------------------------
int FLDMMDL_GetParam( const FLDMMDL * fmmdl, FLDMMDL_H_PARAM param )
{
	switch( param ){
	case FLDMMDL_PARAM_0: return( fmmdl->param0 );
	case FLDMMDL_PARAM_1: return( fmmdl->param1 );
	case FLDMMDL_PARAM_2: return( fmmdl->param2 );
	}
	
	GF_ASSERT( 0 );
	return( 0 );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動制限Xセット
 * @param	fmmdl			FLDMMDL * 
 * @param	x				移動制限
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetMoveLimitX( FLDMMDL * fmmdl, s16 x )
{
	fmmdl->move_limit_x = x;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動制限X取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	s16			移動制限X
 */
//--------------------------------------------------------------
s16 FLDMMDL_GetMoveLimitX( const FLDMMDL * fmmdl )
{
	return( fmmdl->move_limit_x );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動制限Zセット
 * @param	fmmdl			FLDMMDL * 
 * @param	z				移動制限
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetMoveLimitZ( FLDMMDL * fmmdl, s16 z )
{
	fmmdl->move_limit_z = z;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動制限Z取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	s16		移動制限z
 */
//--------------------------------------------------------------
s16 FLDMMDL_GetMoveLimitZ( const FLDMMDL * fmmdl )
{
	return( fmmdl->move_limit_z );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画ステータスセット
 * @param	fmmdl		FLDMMDL * 
 * @param	st			DRAW_STA_STOP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetDrawStatus( FLDMMDL * fmmdl, u16 st )
{
	fmmdl->draw_status = st;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画ステータス取得
 * @param	fmmdl		FLDMMDL * 
 * @retval	u16			DRAW_STA_STOP等
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetDrawStatus( const FLDMMDL * fmmdl )
{
	return( fmmdl->draw_status );
}

//--------------------------------------------------------------
/**
 * FLDMMDL FLDMMDLSYS *取得
 * @param	fmmdl			FLDMMDL * 
 * @retval	FLDMMDLSYS	FLDMMDLSYS *
 */
//--------------------------------------------------------------
const FLDMMDLSYS * FLDMMDL_GetFldMMdlSys( const FLDMMDL *fmmdl )
{
	return( fmmdl->pFldMMdlSys );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作関数用ワーク初期化。
 * sizeがワークサイズを超えていた場合、ASSERT。
 * 動作用ワークはsize分、0で初期化。
 * @param	fmmdl	FLDMMDL * 
 * @param	size	必要なワークサイズ
 * @retval	void*	初期化されたワーク
 */
//--------------------------------------------------------------
void * FLDMMDL_InitMoveProcWork( FLDMMDL * fmmdl, int size )
{
	void *work;
	GF_ASSERT( size <= FLDMMDL_MOVE_WORK_SIZE );
	work = FLDMMDL_GetMoveProcWork( fmmdl );
	GFL_STD_MemClear( work, size );
	return( work );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作関数用ワーク取得。
 * @param	fmmdl	FLDMMDL * 
 * @retval	void*	ワーク
 */
//--------------------------------------------------------------
void * FLDMMDL_GetMoveProcWork( FLDMMDL * fmmdl )
{
	return( fmmdl->move_proc_work );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作補助関数用ワーク初期化。
 * sizeがワークサイズを超えていた場合、ASSERT。
 * @param	fmmdl	FLDMMDL * 
 * @param	size	必要なワークサイズ
 * @retval	void*	初期化されたワーク
 */
//--------------------------------------------------------------
void * FLDMMDL_InitMoveSubProcWork( FLDMMDL * fmmdl, int size )
{
	u8 *work;
	
	GF_ASSERT( size <= FLDMMDL_MOVE_SUB_WORK_SIZE );
	work = FLDMMDL_GetMoveSubProcWork( fmmdl );
	GFL_STD_MemClear( work, size );
	return( work );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作補助関数用ワーク取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	void*	ワーク*
 */
//--------------------------------------------------------------
void * FLDMMDL_GetMoveSubProcWork( FLDMMDL * fmmdl )
{
	return( fmmdl->move_sub_proc_work );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作コマンド用ワーク初期化。
 * sizeがワークサイズを超えていた場合、ASSERT。
 * @param	fmmdl	FLDMMDL * 
 * @param	size	必要なワークサイズ
 * @retval	void*	初期化されたワーク
 */
//--------------------------------------------------------------
void * FLDMMDL_InitMoveCmdWork( FLDMMDL * fmmdl, int size )
{
	u8 *work;
	
	GF_ASSERT( size <= FLDMMDL_MOVE_CMD_WORK_SIZE );
	work = FLDMMDL_GetMoveCmdWork( fmmdl );
	GFL_STD_MemClear( work, size );
	return( work );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作コマンド用ワーク取得
 * @param	fmmdl	FLDMMDL *
 * @retval	void*	ワーク*
 */
//--------------------------------------------------------------
void * FLDMMDL_GetMoveCmdWork( FLDMMDL * fmmdl )
{
	return( fmmdl->move_cmd_proc_work );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画関数用ワーク初期化。
 * sizeがワークサイズを超えていた場合、ASSERT。
 * @param	fmmdl	FLDMMDL * 
 * @param	size	必要なワークサイズ
 * @retval	void*	初期化されたワーク
 */
//--------------------------------------------------------------
void * FLDMMDL_InitDrawProcWork( FLDMMDL * fmmdl, int size )
{
	u8 *work;
	
	GF_ASSERT( size <= FLDMMDL_DRAW_WORK_SIZE );
	work = FLDMMDL_GetDrawProcWork( fmmdl );
	GFL_STD_MemClear( work, size );
	return( work );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画関数用ワーク取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	void*	ワーク
 */
//--------------------------------------------------------------
void * FLDMMDL_GetDrawProcWork( FLDMMDL * fmmdl )
{
	return( fmmdl->draw_proc_work );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作初期化関数実行
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_CallMoveInitProc( FLDMMDL * fmmdl )
{
	GF_ASSERT( fmmdl->move_proc_list );
	GF_ASSERT( fmmdl->move_proc_list->init_proc );
	fmmdl->move_proc_list->init_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作関数実行
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_CallMoveProc( FLDMMDL * fmmdl )
{
	GF_ASSERT( fmmdl->move_proc_list );
	GF_ASSERT( fmmdl->move_proc_list->move_proc );
	fmmdl->move_proc_list->move_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 削除関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_CallMoveDeleteProc( FLDMMDL * fmmdl )
{
	GF_ASSERT( fmmdl->move_proc_list );
	GF_ASSERT( fmmdl->move_proc_list->delete_proc );
	fmmdl->move_proc_list->delete_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 復帰関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_CallMovePopProc( FLDMMDL * fmmdl )
{
	GF_ASSERT( fmmdl->move_proc_list );
	GF_ASSERT( fmmdl->move_proc_list->recover_proc );
	fmmdl->move_proc_list->recover_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画初期化関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_CallDrawInitProc( FLDMMDL * fmmdl )
{
	GF_ASSERT( fmmdl->draw_proc_list );
	GF_ASSERT( fmmdl->draw_proc_list->init_proc );
	fmmdl->draw_proc_list->init_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_CallDrawProc( FLDMMDL * fmmdl )
{
	GF_ASSERT( fmmdl->draw_proc_list );
	GF_ASSERT( fmmdl->draw_proc_list->draw_proc );
	fmmdl->draw_proc_list->draw_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画削除関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_CallDrawDeleteProc( FLDMMDL * fmmdl )
{
	GF_ASSERT( fmmdl->draw_proc_list );
	GF_ASSERT( fmmdl->draw_proc_list->delete_proc );
	fmmdl->draw_proc_list->delete_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画退避関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_CallDrawPushProc( FLDMMDL * fmmdl )
{
	GF_ASSERT( fmmdl->draw_proc_list );
	GF_ASSERT( fmmdl->draw_proc_list->push_proc );
	fmmdl->draw_proc_list->push_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画復帰関数実行
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_CallDrawPopProc( FLDMMDL * fmmdl )
{
	GF_ASSERT( fmmdl->draw_proc_list );
	GF_ASSERT( fmmdl->draw_proc_list->pop_proc );
	fmmdl->draw_proc_list->pop_proc( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画取得関数実行
 * @param	fmmdl	FLDMMDL*
 * @param	state	取得関数に与える情報
 * @retval	nothing
 */
//--------------------------------------------------------------
u32 FLDMMDL_CallDrawGetProc( FLDMMDL *fmmdl, u32 state )
{
	GF_ASSERT( fmmdl->draw_proc_list );
	GF_ASSERT( fmmdl->draw_proc_list->get_proc );
	return( fmmdl->draw_proc_list->get_proc(fmmdl,state) );
}

//--------------------------------------------------------------
/**
 * FLDMMDL アニメーションコマンドコードセット
 * @param	fmmdl	FLDMMDL * 
 * @param	code	AC_DIR_U等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetAcmdCode( FLDMMDL * fmmdl, u16 code )
{
	fmmdl->acmd_code = code;
}

//--------------------------------------------------------------
/**
 * FLDMMDL アニメーションコマンドコード取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	u16	AC_DIR_U等
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetAcmdCode( const FLDMMDL * fmmdl )
{
	return( fmmdl->acmd_code );
}

//--------------------------------------------------------------
/**
 * FLDMMDL アニメーションコマンドシーケンスセット
 * @param	fmmdl	FLDMMDL * 
 * @param	no		シーケンス
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetAcmdSeq( FLDMMDL * fmmdl, u16 no )
{
	fmmdl->acmd_seq = no;
}

//--------------------------------------------------------------
/**
 * FLDMMDL アニメーションコマンドシーケンス増加
 * @param	fmmdl	FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_IncAcmdSeq( FLDMMDL * fmmdl )
{
	fmmdl->acmd_seq++;
}

//--------------------------------------------------------------
/**
 * FLDMMDL アニメーションコマンドシーケンス取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	u16 シーケンス
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetAcmdSeq( const FLDMMDL * fmmdl )
{
	return( fmmdl->acmd_seq );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 現在のマップアトリビュートをセット
 * @param	fmmdl	FLDMMDL *
 * @param	attr	セットするアトリビュート
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetMapAttr( FLDMMDL * fmmdl, u32 attr )
{
	fmmdl->now_attr = attr;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 現在のマップアトリビュートを取得
 * @param	fmmdl	FLDMMDL *
 * @retval	u32		マップアトリビュート
 */
//--------------------------------------------------------------
u32 FLDMMDL_GetMapAttr( const FLDMMDL * fmmdl )
{
	return( fmmdl->now_attr );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 過去のマップアトリビュートをセット
 * @param	fmmdl	FLDMMDL *
 * @param	attr	セットするアトリビュート
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetMapAttrOld( FLDMMDL * fmmdl, u32 attr )
{
	fmmdl->old_attr = attr;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 過去のマップアトリビュートを取得
 * @param	fmmdl	FLDMMDL *
 * @retval	u32		マップアトリビュート
 */
//--------------------------------------------------------------
u32 FLDMMDL_GetMapAttrOld( const FLDMMDL * fmmdl )
{
	return( fmmdl->old_attr );
}

//--------------------------------------------------------------
/**
 * FLDMMDL エイリアス時のゾーンID取得。
 * ※エイリアス時はイベントフラグが指定ゾーンID
 * @param	fmmdl	FLDMMDL *
 * @retval	u16 ゾーンID
 */
//--------------------------------------------------------------
u16 FLDMMDL_GetZoneIDAlies( const FLDMMDL * fmmdl )
{
	GF_ASSERT( FLDMMDL_CheckStatusBitAlies(fmmdl) == TRUE );
	return( FLDMMDL_GetEventFlag(fmmdl) );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 初期座標 グリッドX座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	s16 X座標
 */
//--------------------------------------------------------------
s16 FLDMMDL_GetInitGridPosX( const FLDMMDL * fmmdl )
{
	return( fmmdl->gx_init );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 初期座標 グリッドX座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	x		セットする座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetInitGridPosX( FLDMMDL * fmmdl, s16 x )
{
	fmmdl->gx_init = x;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 初期座標 Y座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	s16		Y
 */
//--------------------------------------------------------------
s16 FLDMMDL_GetInitGridPosY( const FLDMMDL * fmmdl )
{
	return( fmmdl->gy_init );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 初期座標 Y座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	y		セットする座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetInitGridPosY( FLDMMDL * fmmdl, s16 y )
{
	fmmdl->gy_init = y;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 初期座標 z座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	s16		z座標
 */
//--------------------------------------------------------------
s16 FLDMMDL_GetInitGridPosZ( const FLDMMDL * fmmdl )
{
	return( fmmdl->gz_init );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 初期座標 z座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	z		セットする座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetInitGridPosZ( FLDMMDL * fmmdl, s16 z )
{
	fmmdl->gz_init = z;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 過去座標　X座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	s16		X座標
 */
//--------------------------------------------------------------
s16 FLDMMDL_GetOldGridPosX( const FLDMMDL * fmmdl )
{
	return( fmmdl->gx_old );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 過去座標 X座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	x		セットする座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetOldGridPosX( FLDMMDL * fmmdl, s16 x )
{
	fmmdl->gx_old = x;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 過去座標 Y座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	s16		Y
 */
//--------------------------------------------------------------
s16 FLDMMDL_GetOldGridPosY( const FLDMMDL * fmmdl )
{
	return( fmmdl->gy_old );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 過去座標 Y座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	y		セットする座標
 * @retval	s16		y座標
 */
//--------------------------------------------------------------
void FLDMMDL_SetOldGridPosY( FLDMMDL * fmmdl, s16 y )
{
	fmmdl->gy_old = y;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 過去座標 z座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	s16		z座標
 */
//--------------------------------------------------------------
s16 FLDMMDL_GetOldGridPosZ( const FLDMMDL * fmmdl )
{
	return( fmmdl->gz_old );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 過去座標 z座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	z		セットする座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetOldGridPosZ( FLDMMDL * fmmdl, s16 z )
{
	fmmdl->gz_old = z;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 現在座標 X座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	s16		X座標
 */
//--------------------------------------------------------------
s16 FLDMMDL_GetGridPosX( const FLDMMDL * fmmdl )
{
	return( fmmdl->gx_now );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 現在座標 X座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	x		セットする座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetGridPosX( FLDMMDL * fmmdl, s16 x )
{
	fmmdl->gx_now = x;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 現在座標 X座標増加
 * @param	fmmdl	FLDMMDL * 
 * @param	x		足す値
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_AddGridPosX( FLDMMDL * fmmdl, s16 x )
{
	fmmdl->gx_now += x;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 現在座標 Y座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	s16		Y
 */
//--------------------------------------------------------------
s16 FLDMMDL_GetGridPosY( const FLDMMDL * fmmdl )
{
	return( fmmdl->gy_now );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 現在座標 Y座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	y		セットする座標
 * @retval	s16		y座標
 */
//--------------------------------------------------------------
void FLDMMDL_SetGridPosY( FLDMMDL * fmmdl, s16 y )
{
	fmmdl->gy_now = y;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 現在座標 Y座標増加
 * @param	fmmdl	FLDMMDL * 
 * @param	y		足す値
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_AddGridPosY( FLDMMDL * fmmdl, s16 y )
{
	fmmdl->gy_now += y;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 過去座標 z座標取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	s16		z座標
 */
//--------------------------------------------------------------
s16 FLDMMDL_GetGridPosZ( const FLDMMDL * fmmdl )
{
	return( fmmdl->gz_now );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 現在座標 z座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	z		セットする座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetGridPosZ( FLDMMDL * fmmdl, s16 z )
{
	fmmdl->gz_now = z;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 現在座標 z座標増加
 * @param	fmmdl	FLDMMDL * 
 * @param	z		足す値
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_AddGridPosZ( FLDMMDL * fmmdl, s16 z )
{
	fmmdl->gz_now += z;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 実座標ポインタ取得
 * @param	fmmdl	FLDMMDL * 
 * @retval VecFx32*
 */
//--------------------------------------------------------------
const VecFx32 * FLDMMDL_GetVectorPosAddress( const FLDMMDL * fmmdl )
{
  return( &fmmdl->vec_pos_now );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 実座標取得
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		座標格納先
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_GetVectorPos( const FLDMMDL * fmmdl, VecFx32 *vec )
{
	*vec = fmmdl->vec_pos_now;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 実座標セット
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetVectorPos( FLDMMDL * fmmdl, const VecFx32 *vec )
{
	fmmdl->vec_pos_now = *vec;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 実座標Y値取得
 * @param	fmmdl	FLDMMDL * 
 * @retval	fx32	高さ
 */
//--------------------------------------------------------------
fx32 FLDMMDL_GetVectorPosY( const FLDMMDL * fmmdl )
{
	return( fmmdl->vec_pos_now.y );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 表示座標オフセット取得
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_GetVectorDrawOffsetPos( const FLDMMDL * fmmdl, VecFx32 *vec )
{
	*vec = fmmdl->vec_draw_offs;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 表示座標オフセットセット
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetVectorDrawOffsetPos( FLDMMDL * fmmdl, const VecFx32 *vec )
{
	fmmdl->vec_draw_offs = *vec;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 外部指定表示座標オフセット取得
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_GetVectorOuterDrawOffsetPos( const FLDMMDL * fmmdl, VecFx32 *vec )
{
	*vec = fmmdl->vec_draw_offs_outside;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 外部指定表示座標オフセットセット
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetVectorOuterDrawOffsetPos( FLDMMDL * fmmdl, const VecFx32 *vec )
{
	fmmdl->vec_draw_offs_outside = *vec;
}

//--------------------------------------------------------------
/**
 * FLDMMDL アトリビュート変化座標オフセット取得
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_GetVectorAttrDrawOffsetPos( const FLDMMDL * fmmdl, VecFx32 *vec )
{
	*vec = fmmdl->vec_attr_offs;
}

//--------------------------------------------------------------
/**
 * FLDMMDL アトリビュート変化座標オフセットセット
 * @param	fmmdl	FLDMMDL * 
 * @param	vec		セット座標
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetVectorAttrDrawOffsetPos( FLDMMDL * fmmdl, const VecFx32 *vec )
{
	fmmdl->vec_attr_offs = *vec;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 高さ(グリッド単位)を取得
 * @param	fmmdl	FLDMMDL *
 * @retval	s16		高さ。H_GRID単位
 */
//--------------------------------------------------------------
s16 FLDMMDL_GetHeightGrid( const FLDMMDL * fmmdl )
{
	fx32 y = FLDMMDL_GetVectorPosY( fmmdl );
	s16 gy = SIZE_GRID_FX32( y );
	return( gy );
}

//--------------------------------------------------------------
/**
 * FLDMMDL FLDMMDL_BLACTCONTを取得
 * @param	fmmdl	FLDMMDL *
 * @retval	FLDMMDL_BLACTCONT*
 */
//--------------------------------------------------------------
FLDMMDL_BLACTCONT * FLDMMDL_GetBlActCont( FLDMMDL *fmmdl )
{
	return( FLDMMDLSYS_GetBlActCont((FLDMMDLSYS*)FLDMMDL_GetFldMMdlSys(fmmdl)) );
}

//======================================================================
//	FLDMMDLSYS ステータス操作
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDLSYS 描画処理が初期化されているかどうかチェック
 * @param	fmmdlsys	FLDMMDLSYS *
 * @retval	BOOL	TRUE=初期化されている
 */
//--------------------------------------------------------------
BOOL FLDMMDLSYS_CheckCompleteDrawInit( const FLDMMDLSYS *fmmdlsys )
{
	if( FLDMMDLSYS_CheckStatusBit(
			fmmdlsys,FLDMMDLSYS_STABIT_DRAW_INIT_COMP) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS 描画処理初期化完了セット
 * @param	fmmdlsys	FLDMMDLSYS*
 * @param	flag	TRUE=初期化完了
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_SetCompleteDrawInit( FLDMMDLSYS *fmmdlsys, BOOL flag )
{
	if( flag == TRUE ){
		FldMMdlSys_OnStatusBit( fmmdlsys, FLDMMDLSYS_STABIT_DRAW_INIT_COMP );
	}else{
		FldMMdlSys_OffStatusBit( fmmdlsys, FLDMMDLSYS_STABIT_DRAW_INIT_COMP );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS 影を付ける、付けないのセット
 * @param	fmmdlsys FLDMMDLSYS *
 * @param	flag	TRUE=影を付ける FALSE=影を付けない
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_SetJoinShadow( FLDMMDLSYS *fos, BOOL flag )
{
	if( flag == FALSE ){
		FldMMdlSys_OnStatusBit( fos, FLDMMDLSYS_STABIT_SHADOW_JOIN_NOT );
	}else{
		FldMMdlSys_OffStatusBit( fos, FLDMMDLSYS_STABIT_SHADOW_JOIN_NOT );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS 影を付ける、付けないのチェック
 * @param	fmmdlsys FLDMMDLSYS *
 * @retval	BOOL TRUE=付ける
 */
//--------------------------------------------------------------
BOOL FLDMMDLSYS_CheckJoinShadow( const FLDMMDLSYS *fos )
{
	if( FLDMMDLSYS_CheckStatusBit(fos,FLDMMDLSYS_STABIT_SHADOW_JOIN_NOT) ){
		return( FALSE );
	}
	return( TRUE );
}

//======================================================================
//	FLDMMDLSYS プロセス操作
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDLSYS フィールド動作モデル全体の動作を完全停止。
 * 動作処理、描画処理を完全停止する。アニメーションコマンドにも反応しない。
 * @param	fmmdlsys	FLDMMDLSYS*
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_StopProc( FLDMMDLSYS *fmmdlsys )
{
	FldMMdlSys_OnStatusBit( fmmdlsys,
		FLDMMDLSYS_STABIT_MOVE_PROC_STOP|FLDMMDLSYS_STABIT_DRAW_PROC_STOP );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS FLDMMDLSYS_StopProc()の解除。
 * @param	fmmdlsys	FLDMMDLSYS*
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_PlayProc( FLDMMDLSYS *fmmdlsys )
{
	FldMMdlSys_OffStatusBit( fmmdlsys,
		FLDMMDLSYS_STABIT_MOVE_PROC_STOP|FLDMMDLSYS_STABIT_DRAW_PROC_STOP );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS フィールド動作モデル全体の動作を一時停止
 * 固有の動作処理（ランダム移動等）を一時停止する。
 * アニメーションコマンドには反応する。
 * @param	fmmdlsys	FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_PauseMoveProc( FLDMMDLSYS *fmmdlsys )
{
	u32 no = 0;
	FLDMMDL *fmmdl;
	
	while( FLDMMDLSYS_SearchUseFldMMdl(fmmdlsys,&fmmdl,&no) == TRUE ){
		FLDMMDL_OnStatusBitMoveProcPause( fmmdl );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS フィールド動作モデル全体の一時停止を解除
 * @param	fmmdlsys	FLDMMDLSYS *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_ClearPauseMoveProc( FLDMMDLSYS *fmmdlsys )
{
	u32 no = 0;
	FLDMMDL *fmmdl;

	while( FLDMMDLSYS_SearchUseFldMMdl(fmmdlsys,&fmmdl,&no) == TRUE ){
		FLDMMDL_OffStatusBitMoveProcPause( fmmdl );
	}
}

//======================================================================
//	FLDMMDL ステータスビット関連
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデルから
 * フィールド動作モデルシステムのビットチェック
 * @param	fmmdl		FLDMMDL*
 * @param	bit			FLDMMDLSYS_STABIT_DRAW_INIT_COMP等
 * @retval	u32			0以外 bitヒット
 */
//--------------------------------------------------------------
u32 FLDMMDL_CheckFldMMdlSysStatusBit(
	const FLDMMDL *fmmdl, FLDMMDLSYS_STABIT bit )
{
	const FLDMMDLSYS *fos = FLDMMDL_GetFldMMdlSys( fmmdl );
	return( FLDMMDLSYS_CheckStatusBit(fos,bit) );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 使用チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL	TRUE=使用中
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitUse( const FLDMMDL *fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_USE) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動動作中にする
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OnStatusBitMove( FLDMMDL *fmmdl )
{
	FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_MOVE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動動作中を解除
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OffStatusBitMove( FLDMMDL * fmmdl )
{
	FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_MOVE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動動作中チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=動作中
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitMove( const FLDMMDL *fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_MOVE) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動動作開始にする
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OnStatusBitMoveStart( FLDMMDL * fmmdl )
{
	FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_MOVE_START );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動動作開始を解除
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OffStatusBitMoveStart( FLDMMDL * fmmdl )
{
	FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_MOVE_START );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動動作開始チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=移動動作開始
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitMoveStart( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_MOVE_START) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動動作終了にする
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OnStatusBitMoveEnd( FLDMMDL * fmmdl )
{
	FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_MOVE_END );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動動作終了を解除
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OffStatusBitMoveEnd( FLDMMDL * fmmdl )
{
	FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_MOVE_END );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 移動動作終了チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=移動終了
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitMoveEnd( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_MOVE_END) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画初期化完了にする
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OnStatusBitCompletedDrawInit( FLDMMDL * fmmdl )
{
	FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_DRAW_PROC_INIT_COMP );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画初期化完了を解除
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OffStatusBitCompletedDrawInit( FLDMMDL * fmmdl )
{
	FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_DRAW_PROC_INIT_COMP );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画初期化完了チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=描画初期化完了
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitCompletedDrawInit( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(
			fmmdl,FLDMMDL_STABIT_DRAW_PROC_INIT_COMP) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 非表示フラグをチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=非表示　FALSE=表示
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitVanish( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_VANISH) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 非表示フラグを設定
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=非表示　FALSE=表示
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetStatusBitVanish( FLDMMDL * fmmdl, BOOL flag )
{
	if( flag == TRUE ){
		FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_VANISH );
	}else{
		FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_VANISH );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL OBJ同士の当たり判定フラグを設定
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=ヒットアリ　FALSE=ヒットナシ
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetStatusBitFellowHit( FLDMMDL * fmmdl, BOOL flag )
{
	if( flag == TRUE ){
		FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_FELLOW_HIT_NON );
	}else{
		FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_FELLOW_HIT_NON );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作中フラグのセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=動作中　FALSE=停止中
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetStatusBitMove( FLDMMDL * fmmdl, int flag )
{
	if( flag == TRUE ){
		FLDMMDL_OnStatusBitMove( fmmdl );
	}else{
		FLDMMDL_OffStatusBitMove( fmmdl );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL 話しかけ可能チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=可能 FALSE=不可
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitTalk( FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_TALK_OFF) ){
		return( FALSE );
	}
	return( TRUE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 話しかけ不可フラグをセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=不可 FALSE=可能
 */
//--------------------------------------------------------------
void FLDMMDL_SetStatusBitTalkOFF( FLDMMDL * fmmdl, BOOL flag )
{
	if( flag == TRUE ){
		FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_TALK_OFF );
	}else{
		FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_TALK_OFF );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作ポーズ
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OnStatusBitMoveProcPause( FLDMMDL * fmmdl )
{
	FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_PAUSE_MOVE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作ポーズ解除
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_OffStatusBitMoveProcPause( FLDMMDL * fmmdl )
{
	FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_PAUSE_MOVE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作ポーズチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=動作ポーズ
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitMoveProcPause( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_PAUSE_MOVE) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL ステータスビット 描画処理初期化完了チェック
 * @param	fmmdl		FLDMMDL *
 * @retval	BOOL TRUE=完了。FALSE=まだ
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckCompletedDrawInit( const FLDMMDL * fmmdl )
{
	const FLDMMDLSYS *fos;
	
	fos = FLDMMDL_GetFldMMdlSys( fmmdl );
	
	if( FLDMMDLSYS_CheckCompleteDrawInit(fos) == FALSE ){
		return( FALSE );
	}
	
	if( FLDMMDL_CheckStatusBitCompletedDrawInit(fmmdl) ){
		return( TRUE );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 高さ取得を禁止する
 * @param	fmmdl	FLDMMDL *
 * @param	int		TRUE=高さ取得OFF FALSE=ON
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetStatusBitHeightGetOFF( FLDMMDL * fmmdl, BOOL flag )
{
	if( flag == TRUE ){
		FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_HEIGHT_GET_OFF );
	}else{
		FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_HEIGHT_GET_OFF );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL 高さ取得が禁止されているかチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=禁止
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitHeightGetOFF( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_HEIGHT_GET_OFF) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL ゾーン切り替え時の削除禁止
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=禁止 FALSE=禁止しない
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetStatusBitNotZoneDelete( FLDMMDL * fmmdl, BOOL flag )
{
	if( flag == TRUE ){
		FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_ZONE_DEL_NOT );
	}else{
		FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_ZONE_DEL_NOT );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL エイリアスフラグをセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=エイリアス FALSE=違う
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetStatusBitAlies( FLDMMDL * fmmdl, BOOL flag )
{
	if( flag == TRUE ){
		FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_ALIES );
	}else{
		FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_ALIES );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL エイリアスフラグをチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=エイリアス FALSE=違う
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitAlies( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_ALIES) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 浅瀬エフェクトセットフラグをセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=セット　FALSE=クリア
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetStatusBitShoalEffect( FLDMMDL * fmmdl, BOOL flag )
{
	if( flag == TRUE ){
		FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_EFFSET_SHOAL );
	}else{
		FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_EFFSET_SHOAL );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL 浅瀬エフェクトセットフラグをチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=セット　FALSE=違う
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitShoalEffect( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_EFFSET_SHOAL) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL アトリビュートオフセット設定OFFセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=セット　FALSE=クリア
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetStatusBitAttrOffsetOFF( FLDMMDL * fmmdl, BOOL flag )
{
	if( flag == TRUE ){
		FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_ATTR_OFFS_OFF );
	}else{
		FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_ATTR_OFFS_OFF );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL アトリビュートオフセット設定OFFチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=OFF　FALSE=違う
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitAttrOffsetOFF( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_ATTR_OFFS_OFF) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 橋移動フラグセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=セット　FALSE=クリア
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetStatusBitBridge( FLDMMDL * fmmdl, BOOL flag )
{
	if( flag == TRUE ){
		FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_BRIDGE );
	}else{
		FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_BRIDGE );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL 橋移動フラグチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=橋　FALSE=違う
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitBridge( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_BRIDGE) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 映りこみフラグセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=セット　FALSE=クリア
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetStatusBitReflect( FLDMMDL * fmmdl, BOOL flag )
{
	if( flag == TRUE ){
		FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_REFLECT_SET );
	}else{
		FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_REFLECT_SET );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL 映りこみフラグチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=セット　FALSE=無し
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitReflect( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_REFLECT_SET) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL アニメーションコマンドチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=コマンドアリ　FALSE=無し
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitAcmd( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_ACMD) ){
		return( TRUE );
	}
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 拡張高さ反応フラグをセット
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=セット　FALSE=クリア
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetStatusBitHeightExpand( FLDMMDL * fmmdl, BOOL flag )
{
	if( flag == TRUE ){
		FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_HEIGHT_EXPAND );
	}else{
		FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_HEIGHT_EXPAND );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL 拡張高さ反応フラグチェック
 * @param	fmmdl	FLDMMDL *
 * @retval	BOOL TRUE=拡張高さに反応する　FALSE=無し
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckStatusBitHeightExpand( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_HEIGHT_EXPAND) ){
		return( TRUE );
	}
	return( FALSE );
}

//======================================================================
//	FLDMMDL 動作ビット関連
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDL アトリビュート取得を停止
 * @param	fmmdl	FLDMMDL *
 * @param	flag	TRUE=停止
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_SetMoveBitAttrGetOFF( FLDMMDL * fmmdl, BOOL flag )
{
	if( flag == TRUE ){
		FLDMMDL_OnMoveBit( fmmdl, FLDMMDL_MOVEBIT_ATTR_GET_OFF );
	}else{
		FLDMMDL_OffMoveBit( fmmdl, FLDMMDL_MOVEBIT_ATTR_GET_OFF );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL アトリビュート取得を停止　チェック
 * @param	fmmdl	FLDMMDL *
 * @retval	int	TRUE=停止
 */
//--------------------------------------------------------------
int FLDMMDL_CheckMoveBitAttrGetOFF( const FLDMMDL * fmmdl )
{
	if( FLDMMDL_CheckMoveBit(fmmdl,FLDMMDL_MOVEBIT_ATTR_GET_OFF) ){
		return( TRUE );
	}
	return( FALSE );
}

//======================================================================
//	FLDMMDLSYS ツール
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDLSYS 使用しているフィールド動作モデルを探す。
 * @param	fos	FLDMMDLSYS *
 * @param	fmmdl	FLDMMDL*格納先
 * @param	no	検索開始ワークno。先頭から検索する際は初期値0を指定。
 * @retval	BOOL TRUE=動作モデル取得した FALSE=noから終端まで検索し取得無し。
 * 引数noは呼び出し後、取得位置+1の値になる。
 *
 * ※例：OBJ ID 1番の動作モデルを探す。
 * u32 no=0;
 * FLDMMDL *fmmdl;
 * while( FLDMMDLSYS_SearchUseFldMMdl(fos,&fmmdl,&no) == TRUE ){
 * 		if( FLDMMDL_GetOBJID(fmmdl) == 1 ){
 * 			break;
 * 		}
 * }
 */
//--------------------------------------------------------------
BOOL FLDMMDLSYS_SearchUseFldMMdl(
	const FLDMMDLSYS *fos, FLDMMDL **fmmdl, u32 *no )
{
	u32 max = FLDMMDLSYS_GetFldMMdlMax( fos );
	
	if( (*no) < max ){
		FLDMMDL *check_obj = &(((FLDMMDLSYS*)fos)->pFldMMdlBuf[*no]);
		
		do{
			(*no)++;
			if( FLDMMDL_CheckStatusBit(check_obj,FLDMMDL_STABIT_USE) ){
				*fmmdl = check_obj;
				return( TRUE );
			}
			check_obj++;
		}while( (*no) < max );
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS 指定されたグリッドX,Z座標にいるOBJを取得
 * @param	sys			FLDMMDLSYS *
 * @param	x			検索グリッドX
 * @param	z			検索グリッドZ
 * @param	old_hit		TURE=過去座標も判定する
 * @retval	FLDMMDL	x,z位置にいるFLDMMDL * 。NULL=その座標にOBJはいない
 */
//--------------------------------------------------------------
FLDMMDL * FLDMMDLSYS_SearchGridPos(
	const FLDMMDLSYS *sys, s16 x, s16 z, BOOL old_hit )
{
	u32 no = 0;
	FLDMMDL *fmmdl;
	
	while( FLDMMDLSYS_SearchUseFldMMdl(sys,&fmmdl,&no) == TRUE ){
		if( old_hit ){
			if( FLDMMDL_GetOldGridPosX(fmmdl) == x &&
				FLDMMDL_GetOldGridPosZ(fmmdl) == z ){
				return( fmmdl );
			}
		}
		
		if( FLDMMDL_GetGridPosX(fmmdl) == x &&
			FLDMMDL_GetGridPosZ(fmmdl) == z ){
			return( fmmdl );
		}
	}
	
	return( NULL );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS 動作コードに一致するFLDMMDL *を検索
 * @param	fos		FLDMMDLSYS *
 * @param	mv_code		検索する動作コード
 * @retval	FLDMMDL *	NULL=存在しない
 */
//--------------------------------------------------------------
FLDMMDL * FLDMMDLSYS_SearchMoveCode( const FLDMMDLSYS *fos, u16 mv_code )
{
	u32 no = 0;
	FLDMMDL *fmmdl;
	
	while( FLDMMDLSYS_SearchUseFldMMdl(fos,&fmmdl,&no) == TRUE ){
		if( FLDMMDL_GetMoveCode(fmmdl) == mv_code ){
			return( fmmdl );
		}
	}
	
	return( NULL );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS OBJ IDに一致するFLDMMDL *を検索
 * @param	fos		FLDMMDLSYS *
 * @param	id		検索するOBJ ID
 * @retval	FLDMMDL *	NULL=存在しない
 */
//--------------------------------------------------------------
FLDMMDL * FLDMMDLSYS_SearchOBJID( const FLDMMDLSYS *fos, u16 id )
{
	u32 no = 0;
	FLDMMDL *fmmdl;

	while( FLDMMDLSYS_SearchUseFldMMdl(fos,&fmmdl,&no) == TRUE ){
		if( FLDMMDL_CheckStatusBitAlies(fmmdl) == FALSE ){
			if( FLDMMDL_GetOBJID(fmmdl) == id ){
				return( fmmdl );
			}
		}
	}
	
	return( NULL );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS フィールド動作モデルの空きを探す
 * @param	sys			FLDMMDLSYS *
 * @retval	FLDMMDL	空きのFLDMMDL*　空きが無い場合はNULL
 */
//--------------------------------------------------------------
static FLDMMDL * FldMMdlSys_SearchSpaceFldMMdl( const FLDMMDLSYS *sys )
{
	int i,max;
	FLDMMDL *fmmdl;
	
	i = 0;
	max = FLDMMDLSYS_GetFldMMdlMax( sys );
	fmmdl = ((FLDMMDLSYS*)sys)->pFldMMdlBuf;
	
	do{
		if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_USE) == 0 ){
			return( fmmdl );
		}
		
		fmmdl++;
		i++;
	}while( i < max );
	
	return( NULL );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS フィールド動作モデル　エイリアスを探す
 * @param	fos			FLDMMDLSYS *
 * @param	obj_id		一致するOBJ ID
 * @param	zone_id		一致するZONE ID
 * @retval	FLDMMDL	一致するFLDMMDL*　一致無し=NULL
 */
//--------------------------------------------------------------
static FLDMMDL * FldMMdlSys_SearchAlies(
	const FLDMMDLSYS *fos, int obj_id, int zone_id )
{
	u32 no = 0;
	FLDMMDL * fmmdl;
	
	while( FLDMMDLSYS_SearchUseFldMMdl(fos,&fmmdl,&no) ){
		if( FLDMMDL_CheckStatusBitAlies(fmmdl) == TRUE ){
			if( FLDMMDL_GetOBJID(fmmdl) == obj_id ){
				if( FLDMMDL_GetZoneIDAlies(fmmdl) == zone_id ){
					return( fmmdl );
				}
			}
		}
	}
	
	return( NULL );
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS フィールド動作モデル ゾーン更新時の動作モデル削除
 * @param	fos	FLDMMDLSYS
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDLSYS_DeleteZoneUpdateFldMMdl( FLDMMDLSYS *fos )
{
	u32 no = 0;
	FLDMMDL *fmmdl;
	
	while( FLDMMDLSYS_SearchUseFldMMdl(fos,&fmmdl,&no) ){
		//本来であれば更にエイリアスチェックが入る
		if( FLDMMDL_CheckStatusBit(
				fmmdl,FLDMMDL_STABIT_ZONE_DEL_NOT) == 0 ){
			if( FLDMMDL_GetOBJID(fmmdl) == 0xff ){
				GF_ASSERT( 0 );
			}
			FLDMMDL_Delete( fmmdl );
		}
	}
}

//======================================================================
//	FLDMMDL ツール
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデル　TCB動作関数追加
 * @param	fmmdl	FLDMMDL*
 * @param	sys		FLDMMDLSYS*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_AddTCB( FLDMMDL *fmmdl, const FLDMMDLSYS *sys )
{
	int pri,code;
	GFL_TCB * tcb;
	
	pri = FLDMMDLSYS_GetTCBPriority( sys );
	code = FLDMMDL_GetMoveCode( fmmdl );
	
	if( code == MV_PAIR || code == MV_TR_PAIR ){
		pri += FLDMMDL_TCBPRI_OFFS_AFTER;
	}
	
	tcb = GFL_TCB_AddTask( FLDMMDLSYS_GetTCBSYS((FLDMMDLSYS*)sys),
			FldMMdl_TCB_MoveProc, fmmdl, pri );
	GF_ASSERT( tcb != NULL );
	
	fmmdl->pTCB = tcb;
}

//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデル　TCB動作関数削除
 * @param	fmmdl	FLDMMDL*
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_DeleteTCB( FLDMMDL *fmmdl )
{
	GF_ASSERT( fmmdl->pTCB );
	GFL_TCB_DeleteTask( fmmdl->pTCB );
	fmmdl->pTCB = NULL;
}

//--------------------------------------------------------------
/**
 * FLDMMDL 現在発生しているフィールド動作モデルのOBJコードを参照
 * @param	fmmdl	FLDMMDL * 
 * @param	code	チェックするコード。HERO等
 * @retval	BOOL	TRUE=fmmdl以外にもcodeを持っている奴がいる
 */
//--------------------------------------------------------------
BOOL FLDMMDL_SearchUseOBJCode( const FLDMMDL *fmmdl, u16 code )
{
	u32 no = 0;
	u16 check_code;
	FLDMMDL *cmmdl;
	const FLDMMDLSYS *fos = FLDMMDL_GetFldMMdlSys( fmmdl );
	
	while( FLDMMDLSYS_SearchUseFldMMdl(fos,&cmmdl,&no) == TRUE ){
		if( cmmdl != fmmdl ){
			check_code = FLDMMDL_GetOBJCode( cmmdl );
			if( check_code != OBJCODEMAX && check_code == code ){
				return( TRUE );
			}
		}
	}
	
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 座標、方向を初期化。
 * @param	fmmdl	FLDMMDL *
 * @param	vec		初期化座標
 * @param	dir		方向 DIR_UP等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_InitPosition( FLDMMDL * fmmdl, const VecFx32 *vec, u16 dir )
{
	int grid = SIZE_GRID_FX32( vec->x );
	FLDMMDL_SetGridPosX( fmmdl, grid );
	
	grid = SIZE_GRID_FX32( vec->y );
	FLDMMDL_SetGridPosY( fmmdl, grid );
	
	grid = SIZE_GRID_FX32( vec->z );
	FLDMMDL_SetGridPosZ( fmmdl, grid );
	
	FLDMMDL_SetVectorPos( fmmdl, vec );
	FLDMMDL_UpdateGridPosCurrent( fmmdl );
	
	FLDMMDL_SetForceDirDisp( fmmdl, dir );
	
	FLDMMDL_FreeAcmd( fmmdl );
	FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_MOVE_START );
	FLDMMDL_OffStatusBit( fmmdl, FLDMMDL_STABIT_MOVE|FLDMMDL_STABIT_MOVE_END );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 動作コード変更
 * @param	fmmdl	FLDMMDL *
 * @param	code	MV_RND等
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_ChangeMoveCode( FLDMMDL *fmmdl, u16 code )
{
	FLDMMDL_CallMoveDeleteProc( fmmdl );
	FLDMMDL_SetMoveCode( fmmdl, code );
	FldMMdl_InitCallMoveProcWork( fmmdl );
	FLDMMDL_InitMoveProc( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 描画初期化に行う処理纏め
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_InitDrawWork( FLDMMDL *fmmdl )
{
	const FLDMMDLSYS *fos = FLDMMDL_GetFldMMdlSys( fmmdl );
	
	if( FLDMMDLSYS_CheckCompleteDrawInit(fos) == FALSE ){
		return; //描画システム初期化完了していない
	}
	
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_HEIGHT_GET_ERROR) ){
		FLDMMDL_UpdateCurrentHeight( fmmdl );
	}
	
	FLDMMDL_SetDrawStatus( fmmdl, 0 );
	#ifndef FLDMMDL_PL_NULL
	FLDMMDL_BlActAddPracFlagSet( fmmdl, FALSE );
	#endif
	
	if( FLDMMDL_CheckStatusBitCompletedDrawInit(fmmdl) == FALSE ){
		FldMMdl_InitCallDrawProcWork( fmmdl );
		FLDMMDL_CallDrawInitProc( fmmdl );
		FLDMMDL_OnStatusBitCompletedDrawInit( fmmdl );
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデル 描画関数初期化
 * @param	fmmdl		FLDMMDL * 
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_InitCallDrawProcWork( FLDMMDL * fmmdl )
{
	const FLDMMDL_DRAW_PROC_LIST *list;
	u16 code = FLDMMDL_GetOBJCode( fmmdl );
	const OBJCODE_PARAM *prm = FLDMMDL_GetOBJCodeParam( fmmdl, code );
	list = DrawProcList_GetList( prm->draw_proc_no );
	fmmdl->draw_proc_list = list;
}

//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデル関連エフェクトのフラグ初期化。
 * エフェクト関連のフラグ初期化をまとめる。
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_InitDrawEffectFlag( FLDMMDL * fmmdl )
{
	FLDMMDL_OffStatusBit( fmmdl,
		FLDMMDL_STABIT_SHADOW_SET		|
		FLDMMDL_STABIT_SHADOW_VANISH	|
		FLDMMDL_STABIT_EFFSET_SHOAL		|
		FLDMMDL_STABIT_REFLECT_SET );
}

//--------------------------------------------------------------
/**
 * FLDMMDL OBJ IDを変更
 * @param	fmmdl	FLDMMDL *
 * @param	id		OBJ ID
 * @retval	nothing
 */
//--------------------------------------------------------------
void FLDMMDL_ChangeOBJID( FLDMMDL * fmmdl, u16 id )
{
	FLDMMDL_SetOBJID( fmmdl, id );
	FLDMMDL_OnStatusBitMoveStart( fmmdl );
	FldMMdl_InitDrawEffectFlag( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL ワーク消去
 * @param	fmmdl	FLDMMDL
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_ClearWork( FLDMMDL *fmmdl )
{
	GFL_STD_MemClear( fmmdl, FLDMMDL_SIZE );
}

//--------------------------------------------------------------
/**
 * FLDMMDL 指定されたフィールド動作モデルがエイリアス指定かどうかチェック
 * @param	fmmdl		FLDMMDL *
 * @param	h_zone_id	headを読み込むゾーンID
 * @param	max			head要素数
 * @param	head		FLDMMDL_H
 * @retval	int			RET_ALIES_NOT等
 */
//--------------------------------------------------------------
static int FldMMdl_CheckHeaderAlies(
		const FLDMMDL *fmmdl, int h_zone_id, int max,
		const FLDMMDL_HEADER *head )
{
	u16 obj_id;
	int zone_id;
	
	while( max ){
		obj_id = head->id;
		
		if( FLDMMDL_GetOBJID(fmmdl) == obj_id ){
			//エイリアスヘッダー
			if( FldMMdlHeader_CheckAlies(head) == TRUE ){
				//エイリアスの正規ゾーンID
				zone_id = FldMMdlHeader_GetAliesZoneID( head );
				//対象エイリアス
				if( FLDMMDL_CheckStatusBitAlies(fmmdl) == TRUE ){
					if( FLDMMDL_GetZoneIDAlies(fmmdl) == zone_id ){
						return( RET_ALIES_EXIST );	//Aliesとして既に存在
					}
				}else if( FLDMMDL_GetZoneID(fmmdl) == zone_id ){
					return( RET_ALIES_CHANGE );		//Alies対象である
				}
			}else{ //通常ヘッダー
				if( FLDMMDL_CheckStatusBitAlies(fmmdl) == TRUE ){
					//生存エイリアスと一致
					if( FLDMMDL_GetZoneIDAlies(fmmdl) == h_zone_id ){
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
 * FLDMMDL 指定されたゾーンIDとOBJ IDを持つFLDMMDL *を取得。
 * @param	fos		FLDMMDLSYS *
 * @param	obj_id	OBJ ID
 * @param	zone_id	ゾーンID
 * @retval	FLDMMDL * FLDMMDL *
 */
//--------------------------------------------------------------
static FLDMMDL * FldMMdl_SearchOBJIDZoneID(
		const FLDMMDLSYS *fos, int obj_id, int zone_id )
{
	u32 no = 0;
	FLDMMDL *fmmdl;
	
	while( FLDMMDLSYS_SearchUseFldMMdl(fos,&fmmdl,&no) == TRUE ){
		if( FLDMMDL_GetOBJID(fmmdl) == obj_id &&
			FLDMMDL_GetZoneID(fmmdl) == zone_id ){
			return( fmmdl );
		}
	}
	
	return( NULL );
}

//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデル描画初期化に行う処理
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_InitDrawStatus( FLDMMDL * fmmdl )
{
	FLDMMDL_OnStatusBit( fmmdl, FLDMMDL_STABIT_MOVE_START );
	FldMMdl_InitDrawEffectFlag( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデル描画削除時に行う処理
 * @param	fmmdl	FLDMMDL *
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_SetDrawDeleteStatus( FLDMMDL * fmmdl )
{
}

//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデル　エイリアスから正規OBJへの変更
 * @param	fmmdl		FLDMMDL * 
 * @param	head		対象のFLDMMDL_H
 * @param	zone_id		正規のゾーンID
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_ChangeAliesOBJ(
	FLDMMDL *fmmdl, const FLDMMDL_HEADER *head, int zone_id )
{
	GF_ASSERT( FLDMMDL_CheckStatusBitAlies(fmmdl) == TRUE );
	FLDMMDL_SetStatusBitAlies( fmmdl, FALSE );
	FLDMMDL_SetZoneID( fmmdl, zone_id );
	FLDMMDL_SetEventID( fmmdl, head->event_id );
	FLDMMDL_SetEventFlag( fmmdl, head->event_flag );
}

//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデル　正規OBJからエイリアスへの変更
 * @param	fmmdl		FLDMMDL * 
 * @param	head		対象のFLDMMDL_H
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdl_ChangeOBJAlies(
	FLDMMDL * fmmdl, int zone_id, const FLDMMDL_HEADER *head )
{
	GF_ASSERT( FldMMdlHeader_CheckAlies(head) == TRUE );
	FLDMMDL_SetStatusBitAlies( fmmdl, TRUE );
	FLDMMDL_SetEventID( fmmdl, head->event_id );
	FLDMMDL_SetEventFlag( fmmdl, FldMMdlHeader_GetAliesZoneID(head) );
	FLDMMDL_SetZoneID( fmmdl, zone_id );
}

//--------------------------------------------------------------
/**
 * FLDMMDL フィールド動作モデルの同一チェック。
 * 死亡、入れ替わりが発生しているかチェックする。
 * @param	fmmdl	FLDMMDL *
 * @param	obj_id	同一とみなすOBJ ID
 * @param	zone_id	同一とみなすZONE ID
 * @retval	int		TRUE=同一。FALSE=同一ではない
 */
//--------------------------------------------------------------
BOOL FLDMMDL_CheckSameID( const FLDMMDL * fmmdl, u16 obj_id, u16 zone_id )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_USE) == 0 ){
		return( FALSE );
	}
	
	if( FLDMMDL_GetOBJID(fmmdl) != obj_id ){
		return( FALSE );
	}
	
	if( FLDMMDL_GetZoneID(fmmdl) != zone_id ){
		if( FLDMMDL_CheckStatusBitAlies(fmmdl) == FALSE ){
			return( FALSE );
		}
		
		if( FLDMMDL_GetZoneIDAlies(fmmdl) != zone_id ){
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
BOOL FLDMMDL_CheckSameIDCode(
		const FLDMMDL * fmmdl, u16 code, u16 obj_id, u16 zone_id )
{
	if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_USE) ){
		return( FALSE );
	}
	
	{
		u16 ret = FLDMMDL_GetOBJCode( fmmdl );
		if( ret != code ){
			return( FALSE );
		}
	}
	
	return( FLDMMDL_CheckSameID(fmmdl,obj_id,zone_id) );
}

//======================================================================
//	OBJCODE_PARAM
//======================================================================
//--------------------------------------------------------------
/**
 * FLDMMDLSYS OBJCODE_PARAM 初期化
 * @param	fmmdlsys	FLDMMDLSYS
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdlSys_InitOBJCodeParam( FLDMMDLSYS *fmmdlsys )
{
	u8 *p = GFL_ARC_LoadDataAlloc( ARCID_FLDMMDL_PARAM, 
			NARC_fldmmdl_mdlparam_fldmmdl_mdlparam_bin,
			fmmdlsys->heapID );
	fmmdlsys->pOBJCodeParamBuf = p;
	fmmdlsys->pOBJCodeParamTbl = (const OBJCODE_PARAM*)(&p[OBJCODE_PARAM_TOTAL_NUMBER_SIZE]);
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS OBJCODE_PARAM 削除
 * @param	fmmdlsys	FLDMMDLSYS
 * @retval	nothing
 */
//--------------------------------------------------------------
static void FldMMdlSys_DeleteOBJCodeParam( FLDMMDLSYS *fmmdlsys )
{
	GFL_HEAP_FreeMemory( fmmdlsys->pOBJCodeParamBuf );
	fmmdlsys->pOBJCodeParamBuf = NULL;
}

//--------------------------------------------------------------
/**
 * FLDMMDLSYS OBJCODE_PARAM 取得
 * @param	fmmdlsys	FLDMMDLSYS *
 * @param	code	取得するOBJコード
 * @retval	OBJCODE_PARAM*
 */
//--------------------------------------------------------------
const OBJCODE_PARAM * FLDMMDLSYS_GetOBJCodeParam(
		const FLDMMDLSYS *fmmdlsys, u16 code )
{
	GF_ASSERT( code < OBJCODEMAX );
	return( &(fmmdlsys->pOBJCodeParamTbl[code]) );
}

//--------------------------------------------------------------
/**
 * FLDMMDL OBJCODE_PARAM 取得
 * @param	fmmdl	FLDMMDL*
 * @param	code	取得するOBJコード
 * @retval	OBJCODE_PARAM*
 */
//--------------------------------------------------------------
const OBJCODE_PARAM * FLDMMDL_GetOBJCodeParam(
		const FLDMMDL *fmmdl, u16 code )
{
	const FLDMMDLSYS *fmmdlsys = FLDMMDL_GetFldMMdlSys( fmmdl );
	return( FLDMMDLSYS_GetOBJCodeParam(fmmdlsys,code) );
}

//======================================================================
//	parts
//======================================================================
//--------------------------------------------------------------
/**
 * 指定されたOBJコードがワーク参照型ならばワーク内OBJコードに変更。
 * 違う場合はそのままで返す。
 * @param	fsys	FIELDSYS_WORK *
 * @param	code	OBJコード。HERO等
 * @retval	int		チェックされたOBJコード
 */
//--------------------------------------------------------------
static u16 WorkOBJCode_GetOBJCode( void *fsys, int code )
{
	#ifndef FLDMMDL_PL_NULL
	if( code >= WKOBJCODE_ORG && code <= WKOBJCODE_END ){
		code -= WKOBJCODE_ORG;
		code = GetEvDefineObjCode( fsys, code );
	}
	#else
//	GF_ASSERT( 0 );
	#endif
	
	return( code );
}

//--------------------------------------------------------------
/**
 * 指定された動作コードの関数リストを取得
 * @param	code	動作コード
 * @retval FLDMMDL_MOVE_PROC_LIST
 */
//--------------------------------------------------------------
static const FLDMMDL_MOVE_PROC_LIST * MoveProcList_GetList( u16 code )
{
	GF_ASSERT( code < MV_CODE_MAX );
	return( DATA_FieldOBJMoveProcListTbl[code] );
}

//--------------------------------------------------------------
/**
 * 指定されたOBJコードの描画関数リストを取得
 * @param	code	OBJコード
 * @retval	FLDMMDL_DRAW_PROC_LIST*
 */
//--------------------------------------------------------------
static const FLDMMDL_DRAW_PROC_LIST * DrawProcList_GetList(
		FLDMMDL_DRAWPROCNO no )
{
	GF_ASSERT( no < FLDMMDL_DRAWPROCNO_MAX );
	return( DATA_FLDMMDL_DRAW_PROC_LIST_Tbl[no] );
}

//--------------------------------------------------------------
/**
 * FLDMMDL_HEADER エイリアスチェック
 * @param	head	FIELD_OBJ_H
 * @retval	int		TRUE=エイリアス　FALSE=違う
 */
//--------------------------------------------------------------
static BOOL FldMMdlHeader_CheckAlies( const FLDMMDL_HEADER *head )
{
	u16 id = head->event_id;
	if( id == SP_SCRID_ALIES ){ return( TRUE ); }
	return( FALSE );
}

//--------------------------------------------------------------
/**
 * フィールドOBJヘッダー　エイリアス時のゾーンID取得。
 * エイリアス時はイベントフラグがゾーンIDになる
 * @param	head	FIELD_OBJ_H
 * @retval	int		ゾーンID
 */
//--------------------------------------------------------------
static int FldMMdlHeader_GetAliesZoneID( const FLDMMDL_HEADER *head )
{
	GF_ASSERT( FldMMdlHeader_CheckAlies(head) == TRUE );
	return( (int)head->event_flag );
}

//======================================================================
//	動作関数ダミー
//======================================================================
//--------------------------------------------------------------
/**
 * 動作初期化関数ダミー
 * @param	FLDMMDL	FLDMMDL * 
 * @retval	nothing
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

//======================================================================
//	描画関数ダミー
//======================================================================
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

//======================================================================
//	debug
//======================================================================
#ifdef DEBUG_FLDMMDL
//--------------------------------------------------------------
/**
 * デバッグ　動作モデル　OBJコード文字列を取得(ASCII)
 * @param	code OBJコード
 * @param	heapID buf領域確保用HEAPID
 * @retval	u8* 文字列が格納されたu8*。使用後GFL_HEAP_FreeMemory()が必要。
 * 文字列長はDEBUG_OBJCODE_STR_LENGTH。
 */
//--------------------------------------------------------------
u8 * DEBUG_FLDMMDL_GetOBJCodeString( u16 code, HEAPID heapID )
{
	u8 *buf;
	GF_ASSERT( code < OBJCODEMAX );
	buf = GFL_HEAP_AllocClearMemoryLo(
			heapID, DEBUG_OBJCODE_STR_LENGTH );
	GFL_ARC_LoadDataOfs( buf, ARCID_FLDMMDL_PARAM,
			NARC_fldmmdl_mdlparam_fldmmdl_objcodestr_bin,
			DEBUG_OBJCODE_STR_LENGTH * code, DEBUG_OBJCODE_STR_LENGTH );
	return( buf );
}
#endif //DEBUG_FLDMMDL

//======================================================================
//
//======================================================================
#if 0
//--------------------------------------------------------------
/**
 * FLDMMDL_BUFFER フィールド動作モデルバッファからロード
 * @param
 * @retval
 */
//--------------------------------------------------------------
FLDMMDL * FLDMMDL_BUFFER_LoadFldMMdl(
	FLDMMDL_BUFFER *buf, FLDMMDLSYS *fos, int no )
{
	FLDMMDL *fmmdl;
	
	fmmdl = FldMMdlSys_SearchSpaceFldMMdl( fos );
	GF_ASSERT( fmmdl != NULL );
	
	OS_Printf( "FLDMMDL LoadNo %d\n", no );
	
	*fmmdl = buf->fldMMdlBuf[no];
	
	FldMMdl_InitWork( fmmdl, fos );
	
	if( FLDMMDL_CheckMoveBit(fmmdl,FLDMMDL_MOVEBIT_MOVEPROC_INIT) == 0 ){
		FldMMdl_InitMoveWork( fmmdl );
	}else{
		FldMMdl_InitCallMoveProcWork( fmmdl );
	}
	
	FLDMMDL_OffStatusBitCompletedDrawInit( fmmdl );
	FldMMdl_InitDrawWork( fmmdl );
	
	FldMMdlSys_AddFldMMdlTCB( fos, fmmdl );
	FldMMdlSys_IncrementOBJCount( (FLDMMDLSYS*)FLDMMDL_GetFldMMdlSys(fmmdl) );
	
	return( fmmdl );
}

//--------------------------------------------------------------
/**
 * FLDMMDL_BUFFER フィールド動作モデルバッファからロード
 * @param
 * @retval
 */
//--------------------------------------------------------------
void FLDMMDL_BUFFER_LoadBuffer( FLDMMDL_BUFFER *buf, FLDMMDLSYS *fos )
{
	int i;
	FLDMMDL *fmmdl;

	for( i = 0; i < FLDMMDL_BUFFER_MAX; i++ ){
		fmmdl = &buf->fldMMdlBuf[i];
		if( FLDMMDL_CheckStatusBit(fmmdl,FLDMMDL_STABIT_USE) ){
			FLDMMDL_BUFFER_LoadFldMMdl( buf, fos, i );
		}
	}
}

//--------------------------------------------------------------
/**
 * FLDMMDL_BUFFER フィールド動作モデルバッファへセーブ
 * @param
 * @retval
 */
//--------------------------------------------------------------
void FLDMMDL_BUFFER_SaveBuffer( FLDMMDL_BUFFER *buf, FLDMMDLSYS *fos )
{
	FLDMMDL *fmmdl;
	u32 i = 0, no = 0;
	
	FLDMMDL_BUFFER_InitBuffer( buf );
	while( FLDMMDLSYS_SearchUseFldMMdl(fos,&fmmdl,&i) == TRUE ){
		OS_Printf( "FLDMMDL BUFFER WorkNo %d, BufferNo %d Save\n", i-1, no );
		buf->fldMMdlBuf[no] = *fmmdl;
		no++;
	}
}
#endif
