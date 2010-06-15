//==============================================================================
/**
 * @file	save_control.c
 * @brief	セーブコントロール関連
 * @author	matsuda
 * @date	2008.08.28(木)
 */
//==============================================================================
#include <gflib.h>
#include <backup_system.h>
#include "system/main.h"
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/player_data.h"
#include "savedata/musical_save.h"
#include "savedata/save_outside.h"


//==============================================================================
//  定数定義
//==============================================================================
///セーブ系の関数内でテンポラリとして使用するヒープのID
#define HEAPID_SAVE_TEMP        (GFL_HEAPID_APP)

//==============================================================================
//	構造体定義
//==============================================================================
///セーブデータ管理ワーク構造体
struct _SAVE_CONTROL_WORK{
	u8 new_data_flag;			///<TRUE:新規データ
	u8 data_exists;			///<データが存在するかどうか
	u8 total_save_flag;		///<TRUE:全体セーブ
	u8 backup_now_save_mode_setup;    ///<TRUE:初回セットアップのフラグバックアップ
	u8 outside_data_exists;   ///<TRUE:管理外セーブが存在する
	u8 outside_data_break;    ///<TRUE:管理外セーブ破損
	u8 padding[3];
	u32 first_status;			///<一番最初のセーブデータチェック結果(bit指定)
	GFL_SAVEDATA *sv_normal;	///<ノーマル用セーブデータへのポインタ
	GFL_SAVEDATA *sv_extra[SAVE_EXTRA_ID_MAX];
};

//==============================================================================
//	ローカル変数
//==============================================================================
///セーブデータへのポインタ
static SAVE_CONTROL_WORK *SaveControlWork = NULL;
extern int BATTLE_EXAMINATION_SAVE_GetWorkSize(void);

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _OutsideSave_SaveErase(void);

//--------------------------------------------------------------
//  オーバーレイID
//--------------------------------------------------------------
FS_EXTERN_OVERLAY(outside_save);



//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ構造の初期化
 */
//---------------------------------------------------------------------------
SAVE_CONTROL_WORK * SaveControl_SystemInit(HEAPID heap_id)
{
	SAVE_CONTROL_WORK *ctrl;
	LOAD_RESULT load_ret;
  SAVE_EXTRA_ID extra_id;
  BOOL outside_exists = FALSE;
  BOOL outside_break = FALSE;
  
  //内部セーブでヒープを使用しきる前に管理外セーブの存在チェックを行う
  {
    OUTSIDE_SAVE_CONTROL *outsv;
    
    GFL_OVERLAY_Load( FS_OVERLAY_ID(outside_save) );
    
    outsv = OutsideSave_SystemLoad(heap_id);
    outside_exists = OutsideSave_GetExistFlag(outsv);
    outside_break = OutsideSave_GetBreakFlag(outsv);
    OutsideSave_SystemUnload(outsv);
    
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(outside_save) );
  }
  
	ctrl = GFL_HEAP_AllocClearMemory(heap_id, sizeof(SAVE_CONTROL_WORK));
	SaveControlWork = ctrl;
	ctrl->new_data_flag = TRUE;			//新規データ
	ctrl->total_save_flag = TRUE;		//全体セーブ
	ctrl->data_exists = FALSE;			//データは存在しない
	ctrl->outside_data_exists = outside_exists;
	ctrl->outside_data_break = outside_break;
	ctrl->sv_normal = GFL_SAVEDATA_Create(&SaveParam_Normal, heap_id);

#if DEBUG_ONLY_FOR_ohno
  {  //各データのサイズを得る
    int i;
    for(i=0;i<SaveParam_Normal.table_max;i++){
      FUNC_GET_SIZE func = SaveParam_Normal.table[i].get_size;
      
        OS_TPrintf(">><< size %d\n",func());
    }
  }
#endif

  //---------------- 内部セーブ＆外部セーブ ----------------
	//データ存在チェックを行っている
	load_ret = GFL_BACKUP_Load(ctrl->sv_normal, HEAPID_SAVE_TEMP);
  if(load_ret == LOAD_RESULT_OK || load_ret == LOAD_RESULT_NG){
    //データを読めても初回セットアップしかされていないならデータ無し扱いにする
    if(SaveData_GetNowSaveModeSetup(ctrl) == TRUE){
      load_ret = LOAD_RESULT_NULL;
      OS_TPrintf("初回セットアップのみの為、データ無し扱い\n");
    }
  }
	ctrl->first_status = 0;
	if(outside_break == TRUE){
    ctrl->first_status |= FST_OUTSIDE_MYSTERY_BREAK_BIT;
  }
	switch(load_ret){
	case LOAD_RESULT_OK:				///<データ正常読み込み
		ctrl->total_save_flag = FALSE;	//全体セーブの必要はない
										//NGの場合はTRUEのままなので全体セーブになる
		//break;
		/* FALL THROUGH */
	case LOAD_RESULT_NG:				///<データ異常
		//まともなデータがあるようなので読み込む　OK or NG(正常読み出しかミラー読み出しが出来た)
		ctrl->new_data_flag = FALSE;		//新規データではない
		ctrl->data_exists = TRUE;			//データは存在する
		if(load_ret == LOAD_RESULT_NG){	//OKもNGも両方ここを通るので改めてチェック
			ctrl->first_status |= FST_NORMAL_NG_BIT;
		}

	  for(extra_id = 0; extra_id < SAVE_EXTRA_ID_MAX; extra_id++){
      if(SaveData_CheckExtraMagicKey(ctrl, extra_id) == TRUE){  //セーブ経験のある外部だけチェック
        LOAD_RESULT extra_result;
        extra_result = SaveControl_Extra_Load(ctrl, extra_id, HEAPID_SAVE_TEMP);
        SaveControl_Extra_Unload(ctrl, extra_id);
        switch(extra_result){
      	case LOAD_RESULT_OK:				///<データ正常読み込み
      	case LOAD_RESULT_NG:				///<データ異常
          if(extra_id == SAVE_EXTRA_ID_MUSICAL_DIST){
            MUSICAL_SAVE* mus_save = MUSICAL_SAVE_GetMusicalSave( ctrl );
            MUSICAL_SAVE_SetEnableDistributData( mus_save, TRUE );
          }
      	  break;
      	case LOAD_RESULT_NULL:  		///<データなし
      	case LOAD_RESULT_BREAK:			///<破壊、復旧不能
      	case LOAD_RESULT_ERROR:			///<機器故障などで読み取り不能
    			ctrl->first_status |= 1 << (FST_EXTRA_START + extra_id);
          if(extra_id == SAVE_EXTRA_ID_MUSICAL_DIST){
            MUSICAL_SAVE* mus_save = MUSICAL_SAVE_GetMusicalSave( ctrl );
            MUSICAL_SAVE_SetEnableDistributData( mus_save, FALSE );
          }
      	  break;
      	}
      }
      else if(extra_id == SAVE_EXTRA_ID_MUSICAL_DIST){
        MUSICAL_SAVE* mus_save = MUSICAL_SAVE_GetMusicalSave( ctrl );
        MUSICAL_SAVE_SetEnableDistributData( mus_save, FALSE );
      }
    }
		break;
	case LOAD_RESULT_BREAK:			///<破壊、復旧不能
		ctrl->first_status |= FST_NORMAL_BREAK_BIT;
		//break;
		/* FALL THROUGH */
	case LOAD_RESULT_NULL:		///<データなし
	case LOAD_RESULT_ERROR:			///<機器故障(Flashが無い)などで読み取り不能
		//新規 or データ破壊なのでクリアする
		GFL_SAVEDATA_Clear(ctrl->sv_normal);
		break;
	default:
		GF_ASSERT(0); //LOAD:例外エラー！
		break;
	}

	return ctrl;
}

//==================================================================
/**
 * セーブシステムを破棄
 */
//==================================================================
void SaveControl_SystemExit(void)
{
  if(SaveControlWork != NULL){
    GFL_SAVEDATA_Delete(SaveControlWork->sv_normal);
    GFL_HEAP_FreeMemory(SaveControlWork);
    SaveControlWork = NULL;
  }
}


//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ管理ワークへのポインタ取得
 * @return	SAVE_CONTROL_WORK	セーブデータ管理ワークへのポインタ
 *
 * 基本的にはセーブデータへのグローバル参照は避けたい。そのため、この関数を
 * 使用する箇所は厳重に制限されなければならない。できればプログラマリーダーの
 * 許可がなければ使用できないようにしたい。
 * 変なアクセスをしたら修正がかかります。使用方法には注意してください。
 */
//---------------------------------------------------------------------------
SAVE_CONTROL_WORK * SaveControl_GetPointer(void)
{
	GF_ASSERT(SaveControlWork != NULL);
  OS_TPrintf("SAVE_CONTROL_WORKへのGlobal参照\n");
	return SaveControlWork;
}

//--------------------------------------------------------------
/**
 * @brief   通常データのロード
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 * @retval  ロード結果
 */
//--------------------------------------------------------------
LOAD_RESULT SaveControl_Load(SAVE_CONTROL_WORK *ctrl)
{
	LOAD_RESULT result = GFL_BACKUP_Load(ctrl->sv_normal, HEAPID_SAVE_TEMP);
	
	switch(result){
	case LOAD_RESULT_OK:
	case LOAD_RESULT_NG:
		ctrl->data_exists = TRUE;
    {
      MUSICAL_SAVE* mus_save = MUSICAL_SAVE_GetMusicalSave( ctrl );
      MUSICAL_SAVE_SetEnableDistributData( mus_save, FALSE );
      if(SaveData_CheckExtraMagicKey(ctrl, SAVE_EXTRA_ID_MUSICAL_DIST) == TRUE){
        LOAD_RESULT extra_result;
        extra_result = SaveControl_Extra_Load(ctrl, SAVE_EXTRA_ID_MUSICAL_DIST, HEAPID_SAVE_TEMP);
        SaveControl_Extra_Unload(ctrl, SAVE_EXTRA_ID_MUSICAL_DIST);
        switch(extra_result){
      	case LOAD_RESULT_OK:				///<データ正常読み込み
      	case LOAD_RESULT_NG:				///<データ異常
          MUSICAL_SAVE_SetEnableDistributData( mus_save, TRUE );
          break;
        }
      }
    }
    
    //破壊されている外部セーブがあるならリンク情報を初期化する
    if(ctrl->first_status > 0){
      SAVE_EXTRA_ID extra_id;
      for(extra_id = 0; extra_id < SAVE_EXTRA_ID_MAX; extra_id++){
        if(ctrl->first_status & (1 << (FST_EXTRA_START + extra_id))){
          SaveData_ClearExtraMagicKey(ctrl, extra_id);
        }
      }
    }
		break;
	}
	return result;
}

//--------------------------------------------------------------
/**
 * @brief   通常データのセーブ
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 * @retval  セーブ結果
 */
//--------------------------------------------------------------
SAVE_RESULT SaveControl_Save(SAVE_CONTROL_WORK *ctrl)
{
	SAVE_RESULT result;

  SaveData_SetNowSaveModeSetupOFF(ctrl);
  
	result = GFL_BACKUP_Save(ctrl->sv_normal);
	if(result == SAVE_RESULT_OK){
    if(SaveData_GetNowSaveModeSetup(ctrl) == FALSE){
  		ctrl->new_data_flag = FALSE;
	  	ctrl->data_exists = TRUE;
	  }
	}
	return result;
}

//--------------------------------------------------------------
/**
 * @brief   通常データの分割セーブ初期化
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 * @retval  セーブ結果
 */
//--------------------------------------------------------------
void SaveControl_SaveAsyncInit(SAVE_CONTROL_WORK *ctrl)
{
  ctrl->backup_now_save_mode_setup = SaveData_GetNowSaveModeSetup(ctrl);
  SaveData_SetNowSaveModeSetupOFF(ctrl);
  
	GFL_BACKUP_SAVEASYNC_Init(ctrl->sv_normal);
}

//--------------------------------------------------------------
/**
 * @brief   通常データの分割セーブメイン処理
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 * @retval  セーブ結果
 */
//--------------------------------------------------------------
SAVE_RESULT SaveControl_SaveAsyncMain(SAVE_CONTROL_WORK *ctrl)
{
	SAVE_RESULT result;
	
	if(ctrl->outside_data_exists == TRUE || ctrl->outside_data_break == TRUE){
    _OutsideSave_SaveErase();
    ctrl->outside_data_exists = FALSE;
    return SAVE_RESULT_CONTINUE;
  }
  
	result = GFL_BACKUP_SAVEASYNC_Main(ctrl->sv_normal);
	if(result == SAVE_RESULT_OK){
    if(SaveData_GetNowSaveModeSetup(ctrl) == FALSE){
  		ctrl->new_data_flag = FALSE;
	  	ctrl->data_exists = TRUE;
	  }
	}
	return result;
}

//==================================================================
/**
 * 分割セーブのキャンセル処理
 *
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 */
//==================================================================
void SaveControl_SaveAsyncCancel(SAVE_CONTROL_WORK *ctrl)
{
  GFL_BACKUP_SAVEASYNC_Cancel(ctrl->sv_normal);
  if(ctrl->backup_now_save_mode_setup == TRUE){
    SaveData_SetNowSaveModeSetupON(ctrl);
  }
}

//==================================================================
/**
 * SaveControl_SaveAsyncMainで実行中のセーブが何バイト目まで書き込んだかを取得する
 *
 * @param   ctrl		
 *
 * @retval  u32	  書き込みが完了したサイズ(SaveControl_GetActualSizeのactual_size * 2(両面サイズ))
 */
//==================================================================
u32 SaveControl_GetSaveAsyncMain_WritingSize(SAVE_CONTROL_WORK *ctrl)
{
  return GFL_BACKUP_SAVEASYNC_Main_WritingSize(ctrl->sv_normal);
}

//--------------------------------------------------------------
/**
 * @brief   通常セーブデータの各セーブワークのポインタを取得する
 *
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 * @param   gmdata_id	取得したいセーブデータのID
 *
 * @retval  セーブ領域へのポインタ
 */
//--------------------------------------------------------------
void * SaveControl_DataPtrGet(SAVE_CONTROL_WORK *ctrl, GFL_SVDT_ID gmdata_id)
{
	return GFL_SAVEDATA_Get(ctrl->sv_normal, gmdata_id);
}

//==================================================================
/**
 * セーブデータワークの先頭アドレスを取得
 *
 * @param   ctrl    		セーブデータ管理ワークへのポインタ
 * @param   get_size		セーブデータの実サイズが代入されるワークのポインタ
 *
 * @retval  const void *		セーブデータワークの先頭アドレス
 *
 * GPFでセーブデータ全転送用に用意した関数です
 * 他の箇所では使用しないこと!
 */
//==================================================================
const void * SaveControl_GetSaveWorkAdrs(SAVE_CONTROL_WORK *ctrl, u32 *get_size)
{
  return GFL_SAVEDATA_GetSaveWorkAdrs(ctrl->sv_normal, get_size);
}

//--------------------------------------------------------------
/**
 * @brief   新規データフラグを取得する
 *
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 *
 * @retval  TRUE:新規データである
 */
//--------------------------------------------------------------
BOOL SaveControl_NewDataFlagGet(const SAVE_CONTROL_WORK *ctrl)
{
	return ctrl->new_data_flag;
}

//---------------------------------------------------------------------------
/**
 * @brief	最初の読み込み結果を返す
 * @param	sv			セーブデータ構造へのポインタ
 * @return	LOAD_RESULT	読み込み結果（savedata_def.h参照）
 */
//---------------------------------------------------------------------------
u32 SaveControl_GetLoadResult(const SAVE_CONTROL_WORK * sv)
{
	return sv->first_status;
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ存在フラグを取得
 * @param	sv			セーブデータ構造へのポインタ
 * @return	BOOL		TRUEのとき、セーブデータが存在する
 */
//---------------------------------------------------------------------------
BOOL SaveData_GetExistFlag(SAVE_CONTROL_WORK * sv)
{
	return sv->data_exists;
}

//---------------------------------------------------------------------------
/**
 * @brief	データ上書きチェック(他人のレポート上書きチェック)
 * @param	sv			セーブデータ構造へのポインタ
 * @retval	TRUE		既にあるデータに別のデータを上書きしようとしている
 * @retval	FALSE		データがないか、既存データである
 */
//---------------------------------------------------------------------------
BOOL SaveControl_IsOverwritingOtherData(SAVE_CONTROL_WORK * sv)
{
	if (SaveControl_NewDataFlagGet(sv) == TRUE && SaveData_GetExistFlag(sv) == TRUE) {
		return TRUE;
	} else {
		return FALSE;
	}
}

//---------------------------------------------------------------------------
/**
 * @brief	管理外セーブデータ存在フラグを取得
 * @param	sv			セーブデータ構造へのポインタ
 * @return	BOOL		TRUEのとき、セーブデータが存在する
 */
//---------------------------------------------------------------------------
BOOL SaveData_GetOutsideExistFlag(SAVE_CONTROL_WORK * sv)
{
	return sv->outside_data_exists;
}

//---------------------------------------------------------------------------
/**
 * @brief	管理外セーブデータ存在フラグをセット
 * @param	sv			              セーブデータ構造へのポインタ
 * @param outside_data_exists		TRUEのとき、セーブデータが存在する
 */
//---------------------------------------------------------------------------
void SaveData_SetOutsideExistFlag(SAVE_CONTROL_WORK * sv, BOOL outside_data_exists)
{
	sv->outside_data_exists = outside_data_exists;
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータの初期化
 * @param	sv			セーブデータ構造へのポインタ
 *
 * SaveControl_Eraseと違い、フラッシュに書き込まない。
 * セーブデータがある状態で「さいしょから」遊ぶ場合などの初期化処理
 */
//---------------------------------------------------------------------------
void SaveControl_ClearData(SAVE_CONTROL_WORK * ctrl)
{
	ctrl->new_data_flag = TRUE;				//新規データである
	ctrl->total_save_flag = TRUE;			//全体セーブする必要がある
	GFL_SAVEDATA_Clear(ctrl->sv_normal);
}

//--------------------------------------------------------------
/**
 * @brief   セーブデータの消去
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 */
//--------------------------------------------------------------
void SaveControl_Erase(SAVE_CONTROL_WORK *ctrl)
{
	GFL_BACKUP_Erase(ctrl->sv_normal, HEAPID_SAVE_TEMP);
}

//--------------------------------------------------------------
/**
 * セーブを実行した場合のセーブサイズを取得する
 *
 * @param   sv		        セーブデータ構造へのポインタ
 * @param   actual_size		セーブされる実サイズ(CRCテーブルなどのシステム系のデータは除きます)
 * @param   total_size		セーブ全体のサイズ(CRCテーブルなどのシステム系のデータは除きます)
 *
 * 差分比較の為、フラッシュアクセスします。
 */
//--------------------------------------------------------------
void SaveControl_GetActualSize(SAVE_CONTROL_WORK *ctrl, u32 *actual_size, u32 *total_size)
{
  GFL_SAVEDATA_GetActualSize(ctrl->sv_normal, actual_size, total_size);
}

//==================================================================
/**
 * 指定IDの部分だけをフラッシュから直接ロードする
 *
 * @param   sv            
 * @param   gmdataid		  ロードする対象のID
 * @param   side_a_or_b		0:A面からロード　　1:B面からロード
 * @param   dst		        データ展開先
 * @param   load_size     ロードするバイトサイズ
 *
 * @retval  BOOL		TRUE:ロード成功　FALSE:ロード失敗
 */
//==================================================================
BOOL SaveControl_PageFlashLoad(SAVE_CONTROL_WORK *ctrl, GFL_SVDT_ID gmdataid, BOOL side_a_or_b, void *dst, u32 load_size)
{
  return GFL_BACKUP_Page_FlashLoad(ctrl->sv_normal, gmdataid, side_a_or_b, dst, load_size);
}

//==================================================================
/**
 * 外部セーブデータのロード
 *
 * @param   ctrl		    セーブデータ管理ワークへのポインタ
 * @param   extra_id		外部セーブデータ番号
 * @param   heap_id		  読み込んだデータをこのヒープに展開
 *
 * @retval  LOAD_RESULT		ロード結果
 * 
 * 使用が終わったら必ずSaveControl_Extra_Unloadで解放してください(ロード出来なかったとしても)
 */
//==================================================================
LOAD_RESULT SaveControl_Extra_Load(SAVE_CONTROL_WORK *ctrl, SAVE_EXTRA_ID extra_id, HEAPID heap_id)
{
	LOAD_RESULT load_ret;
  u32 *link_ptr;

  GF_ASSERT(ctrl->sv_extra[extra_id] == NULL);
	ctrl->sv_extra[extra_id] = GFL_SAVEDATA_Create(&SaveParam_ExtraTbl[extra_id], heap_id);
	
	if(SaveData_CheckExtraMagicKey(ctrl, extra_id) == FALSE){
    //現在の内部セーブに関連付けられている外部は無い為、システムだけ作成し、ロード処理は行わない
    return LOAD_RESULT_NULL;
  }
	
  link_ptr = SaveData_GetExtraLinkPtr(ctrl, extra_id);
	load_ret = GFL_BACKUP_Extra_Load(ctrl->sv_extra[extra_id], heap_id, *link_ptr);
	
	OS_TPrintf("外部セーブロード結果 extra_id = %d, load_ret = %d\n", extra_id, load_ret);
	return load_ret;
}

//==================================================================
/**
 * 外部セーブデータの解放
 *
 * @param   ctrl		
 * @param   extra_id		外部セーブデータ番号
 */
//==================================================================
void SaveControl_Extra_Unload(SAVE_CONTROL_WORK *ctrl, SAVE_EXTRA_ID extra_id)
{
  GF_ASSERT(ctrl->sv_extra[extra_id] != NULL);
  GFL_SAVEDATA_Delete(ctrl->sv_extra[extra_id]);
  ctrl->sv_extra[extra_id] = NULL;
}

//==================================================================
/**
 * 外部セーブが既にロード済みか調べる
 *
 * @param   ctrl		
 * @param   extra_id		外部セーブデータ番号
 *
 * @retval  BOOL		TRUE:ロード済み。　FALSE:ロードされていない
 */
//==================================================================
BOOL SaveControl_Extra_CheckLoad(SAVE_CONTROL_WORK *ctrl, SAVE_EXTRA_ID extra_id)
{
  if(ctrl->sv_extra[extra_id] != NULL){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * 外部セーブデータのロード(セーブワークは外側から渡す)
 *
 * @param   ctrl		    セーブデータ管理ワークへのポインタ
 * @param   extra_id		外部セーブデータ番号
 * @param   heap_id		  セーブシステムで使用するヒープID
 * @param   work        ロードしたデータをこのワークに展開
 * @param   work_size   workのサイズ
 *
 * @retval  LOAD_RESULT		ロード結果
 * 
 * 使用が終わったら必ずSaveControl_Extra_UnloadWorkで解放してください(ロード出来なかったとしても)
 */
//==================================================================
LOAD_RESULT SaveControl_Extra_LoadWork(SAVE_CONTROL_WORK *ctrl, SAVE_EXTRA_ID extra_id, HEAPID heap_id, void *work, u32 work_size)
{
	LOAD_RESULT load_ret;
  u32 *link_ptr;
  
  GF_ASSERT(ctrl->sv_extra[extra_id] == NULL);
  ctrl->sv_extra[extra_id] 
    = GFL_SAVEDATA_CreateEx(&SaveParam_ExtraTbl[extra_id], heap_id, work, work_size, TRUE );

	if(SaveData_CheckExtraMagicKey(ctrl, extra_id) == FALSE){
    //現在の内部セーブに関連付けられている外部は無い為、システムだけ作成し、ロード処理は行わない
    return LOAD_RESULT_NULL;
  }

  link_ptr = SaveData_GetExtraLinkPtr(ctrl, extra_id);
	load_ret = GFL_BACKUP_Extra_Load(ctrl->sv_extra[extra_id], heap_id, *link_ptr);
	
	OS_TPrintf("外部セーブロード結果 extra_id = %d, load_ret = %d\n", extra_id, load_ret);
	return load_ret;
}

//==================================================================
/**
 * 外部セーブデータのシステムのみを作成(セーブワークは外側から渡す)
 *    workで渡されているワークの内容がそのままロードしたデータとして認識し、
 *    フラッシュからデータはロードしない
 *    　※フラッシュからロードしたい場合はこの関数ではなく、SaveControl_Extra_LoadWorkを
 *        使用してください。
 *    外部セーブで、セーブ直前にセーブシステムを作成する場合の使用を想定しています。
 *
 * @param   ctrl		    セーブデータ管理ワークへのポインタ
 * @param   extra_id		外部セーブデータ番号
 * @param   heap_id		  セーブシステムで使用するヒープID
 * @param   work        ロードしたデータをこのワークに展開
 * @param   work_size   workのサイズ
 * 
 * 使用が終わったら必ずSaveControl_Extra_UnloadWorkで解放してください
 */
//==================================================================
void SaveControl_Extra_SystemSetup(SAVE_CONTROL_WORK *ctrl, SAVE_EXTRA_ID extra_id, HEAPID heap_id, void *work, u32 work_size)
{
  GF_ASSERT(ctrl->sv_extra[extra_id] == NULL);
  ctrl->sv_extra[extra_id] 
    = GFL_SAVEDATA_CreateEx(&SaveParam_ExtraTbl[extra_id], heap_id, work, work_size, FALSE );
	
	OS_TPrintf("外部セーブロード無しセットアップ extra_id = %d\n", extra_id);
}

//==================================================================
/**
 * 外部セーブデータの解放
 *
 * @param   ctrl		
 * @param   extra_id		外部セーブデータ番号
 */
//==================================================================
void SaveControl_Extra_UnloadWork(SAVE_CONTROL_WORK *ctrl, SAVE_EXTRA_ID extra_id)
{
  GF_ASSERT(ctrl->sv_extra[extra_id] != NULL);
  GFL_SAVEDATA_Delete(ctrl->sv_extra[extra_id]);
  ctrl->sv_extra[extra_id] = NULL;
}

//--------------------------------------------------------------
/**
 * @brief   外部データの分割セーブ初期化
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 * @param   extra_id		外部セーブデータ番号
 * @retval  セーブ結果
 */
//--------------------------------------------------------------
void SaveControl_Extra_SaveAsyncInit(SAVE_CONTROL_WORK *ctrl, SAVE_EXTRA_ID extra_id)
{
  GF_ASSERT(ctrl->sv_extra[extra_id] != NULL);
  
  SaveData_SetExtraMagicKey(ctrl, extra_id);
  GFL_BACKUP_SAVEASYNC_EXTRA_Init(ctrl->sv_normal, ctrl->sv_extra[extra_id], 
    SaveData_GetExtraLinkPtr(ctrl, extra_id));
}

//--------------------------------------------------------------
/**
 * @brief   外部データの分割セーブメイン処理
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 * @param   extra_id		外部セーブデータ番号
 * @retval  セーブ結果
 */
//--------------------------------------------------------------
SAVE_RESULT SaveControl_Extra_SaveAsyncMain(SAVE_CONTROL_WORK *ctrl, SAVE_EXTRA_ID extra_id)
{
	SAVE_RESULT result;
	
  GF_ASSERT(ctrl->sv_extra[extra_id] != NULL);
  result = GFL_BACKUP_SAVEASYNC_EXTRA_Main(ctrl->sv_normal, ctrl->sv_extra[extra_id]);
	return result;
}

//--------------------------------------------------------------
/**
 * @brief   外部セーブデータの各セーブワークのポインタを取得する
 *
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 * @param   extra_id		外部セーブデータ番号
 * @param   gmdata_id	取得したいセーブデータのID
 *
 * @retval  セーブ領域へのポインタ
 */
//--------------------------------------------------------------
void * SaveControl_Extra_DataPtrGet(SAVE_CONTROL_WORK *ctrl, SAVE_EXTRA_ID extra_id, GFL_SVDT_ID gmdata_id)
{
  GF_ASSERT(ctrl->sv_extra[extra_id] != NULL);
	return GFL_SAVEDATA_Get(ctrl->sv_extra[extra_id], gmdata_id);
}

//---------------------------------------------------------------------------
/**
 * @brief	  外部セーブデータの初期化
 * @param	sv			セーブデータ構造へのポインタ
 * @param   extra_id		外部セーブデータ番号
 *
 * SaveControl_Eraseと違い、フラッシュに書き込まない。
 * セーブデータがある状態で「さいしょから」遊ぶ場合などの初期化処理
 */
//---------------------------------------------------------------------------
void SaveControl_Extra_ClearData(SAVE_CONTROL_WORK * ctrl, SAVE_EXTRA_ID extra_id)
{
  GF_ASSERT(ctrl->sv_extra[extra_id] != NULL);
	GFL_SAVEDATA_Clear(ctrl->sv_extra[extra_id]);
}

//--------------------------------------------------------------
/**
 * @brief   外部セーブデータの消去
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 * @param   extra_id		外部セーブデータ番号
 * @param   heap_temp_id    この関数内でのみテンポラリとして使用するヒープID
 */
//--------------------------------------------------------------
void SaveControl_Extra_Erase(SAVE_CONTROL_WORK *ctrl, SAVE_EXTRA_ID extra_id, u32 heap_temp_id)
{
  GF_ASSERT(ctrl->sv_extra[extra_id] != NULL);
	GFL_BACKUP_Erase(ctrl->sv_extra[extra_id], heap_temp_id);
}

//==================================================================
/**
 * 管理外セーブ：消去
 *
 * 消去用のメモリ確保の必要性や速度などの考慮から、マジックナンバーだけを消去します
 *
 * 内部セーブ実行時に処理を行うので、これだけsave_control.cと同じ常駐に配置
 */
//==================================================================
static void _OutsideSave_SaveErase(void)
{
  u32 erase_magic_number = 0;
  
  OS_TPrintf("== outside Erase start  ==\n");
  //マジックナンバーだけを消す
  GFL_BACKUP_DirectFlashSave(OUTSIDE_MM_MYSTERY, &erase_magic_number, sizeof(u32));
  GFL_BACKUP_DirectFlashSave(OUTSIDE_MM_MYSTERY_MIRROR, &erase_magic_number, sizeof(u32));
  OS_TPrintf("== outside Erase finish ==\n");
}



//==============================================================================
//	デバッグ用関数
//==============================================================================
#ifdef PM_DEBUG
GFL_SAVEDATA * DEBUG_SaveData_PtrGet(void)
{
	SAVE_CONTROL_WORK *ctrl = SaveControl_GetPointer();
	return ctrl->sv_normal;
}

GFL_SAVEDATA * DEBUG_SaveDataExtra_PtrGet(SAVE_EXTRA_ID extra_id)
{
	SAVE_CONTROL_WORK *ctrl = SaveControl_GetPointer();
	return ctrl->sv_extra[extra_id];
}

///管理外セーブを破壊する
void DEBUG_OutsideSave_Brea(int mirror)
{
  u64 break_data = 0x401a9f4a;
  
  if(mirror == 0){
    GFL_BACKUP_DirectFlashSave(OUTSIDE_MM_MYSTERY + 12, &break_data, sizeof(break_data));
  }
  else{
    GFL_BACKUP_DirectFlashSave(OUTSIDE_MM_MYSTERY_MIRROR + 12, &break_data, sizeof(break_data));
  }
}
#endif

