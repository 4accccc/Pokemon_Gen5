//=============================================================================
/**
 * @file	savedata.h
 * @brief	セーブデータ用ヘッダ
 * @author	tamada	GAME FREAK Inc.
 * @date	2005.10.12
 *
 * このヘッダはセーブデータの生成処理など、直接セーブデータにかかわる部分にのみ必要。
 * それ以外の箇所ではsavedata_def.hのSAVEDATA型経由でアクセスできればよい。
 * そのため、このヘッダをインクルードする箇所は限定されるはず
 */
//=============================================================================
#pragma	once

#include "savedata_def.h"

///---------------------------------------------------------------------------
//---------------------------------------------------------------------------
typedef u32 (*FUNC_GET_SIZE)(void);

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
typedef void (*FUNC_INIT_WORK)(void *);

typedef u32 DATA_ID;

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ内容データ用の構造体定義
 */
//---------------------------------------------------------------------------
typedef struct {
	DATA_ID gmdataID;				///<セーブデータ識別ID
	FUNC_GET_SIZE get_size;			///<セーブデータサイズ取得関数
	FUNC_INIT_WORK	init_work;		///<セーブデータ初期化関数
}SAVEDATA_TABLE;

//---------------------------------------------------------------------------
///	セーブデータ定義構造体
//---------------------------------------------------------------------------
typedef struct {
	const SAVEDATA_TABLE * table;
	u32 table_max;
	u32 savearea_top_address;
	u32 savearea_size;
	u32 magic_number;
}GFL_SVLD_PARAM;



//============================================================================================
//============================================================================================
//---------------------------------------------------------------------------
//	セーブデータ制御関数
//---------------------------------------------------------------------------
//システム起動
extern void GFL_BACKUP_Init(u32 heap_save_id, u32 heap_temp_id);
//システム終了
extern void GFL_BACKUP_Exit(void);
//フラッシュ存在チェック
extern BOOL GFL_BACKUP_IsEnableFlash(const SAVEDATA * sv);

//セーブロードシステム初期化
extern SAVEDATA * SaveData_System_Init(u32 heap_save_id, u32 heap_temp_id);

extern SAVEDATA * GFL_SVLD_Create(const GFL_SVLD_PARAM * sv_param);
extern void GFL_SVLD_Delete(SAVEDATA * sv);

//ロード処理
extern BOOL SaveData_Load(SAVEDATA * sv);
//セーブ処理
extern SAVE_RESULT SaveData_Save(SAVEDATA * sv);
//消去処理
extern BOOL SaveData_Erase(SAVEDATA * sv);

//セーブデータ初期化（書き込まない）
extern void SaveData_ClearData(SAVEDATA * sv);

//最初の読み込み時のセーブデータ状態取得
extern LOAD_RESULT SaveData_GetLoadResult(const SAVEDATA * sv);

//データが存在するかどうかの判別フラグ取得
extern BOOL SaveData_GetExistFlag(const SAVEDATA * sv);

//分割セーブ：初期化
extern void SaveData_DivSave_Init(SAVEDATA * sv);
//分割セーブ：メイン
extern SAVE_RESULT SaveData_DivSave_Main(SAVEDATA * sv);
//分割セーブ：キャンセル
extern void SaveData_DivSave_Cancel(SAVEDATA * sv);

//セーブデータ巻き戻し（デバッグ用）
extern void SaveData_Rewind(SAVEDATA * sv);

//---------------------------------------------------------------------------
//	セーブデータアクセス用関数
//---------------------------------------------------------------------------
extern void * SaveData_Get(SAVEDATA * sv, DATA_ID gmdataID);
extern const void * SaveData_GetReadOnlyData(const SAVEDATA * sv, DATA_ID gmdataID);


// インデックスを作る時のみ有効
// 通常はOFFなのでONになってるのを見つけたら無条件にOFFで良い
//#define CREATE_INDEX

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


