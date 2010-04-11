//==============================================================================
/**
 * @file    save_outside.c
 * @brief   セーブ管理外領域のセーブシステム
 * @author  matsuda
 * @date    2010.04.10(土)
 */
//==============================================================================
#include "savedata/save_control.h"



extern SAVE_CONTROL_WORK * SaveControl_SystemInit(HEAPID heap_id);
extern LOAD_RESULT SaveControl_Load(SAVE_CONTROL_WORK *ctrl);
extern SAVE_RESULT SaveControl_Save(SAVE_CONTROL_WORK *ctrl);
extern void SaveControl_SaveAsyncInit(SAVE_CONTROL_WORK *ctrl);
extern SAVE_RESULT SaveControl_SaveAsyncMain(SAVE_CONTROL_WORK *ctrl);
extern void * SaveControl_DataPtrGet(SAVE_CONTROL_WORK *ctrl, GFL_SVDT_ID gmdata_id);
extern void SaveControl_Erase(SAVE_CONTROL_WORK *ctrl);


//--------------------------------------------------------------
//  外部セーブ関連
//--------------------------------------------------------------
extern void * SaveControl_Extra_DataPtrGet(
  SAVE_CONTROL_WORK *ctrl, SAVE_EXTRA_ID extra_id, GFL_SVDT_ID gmdata_id);
extern void SaveControl_Extra_ClearData(SAVE_CONTROL_WORK * ctrl, SAVE_EXTRA_ID extra_id);
extern void SaveControl_Extra_Erase(
  SAVE_CONTROL_WORK *ctrl, SAVE_EXTRA_ID extra_id, u32 heap_temp_id);

//==================================================================
/**
 * 管理外セーブデータシステムを作成
 *
 * @param   heap_id		
 *
 * @retval  OUTSIDE_SAVE_CONTROL *		
 */
//==================================================================
OUTSIDE_SAVE_CONTROL * OutsideSave_SystemInit(HEAPID heap_id)
{
  OUTSIDE_SAVE_CONTROL * outsv;
  
  outsv = GFL_HEAP_AllocClearMemory(heap_id, sizeof(OUTSIDE_SAVE_CONTROL));
  return outsv;
}

//==================================================================
/**
 * 管理外セーブシステムを破棄
 *
 * @param   ctrl		
 */
//==================================================================
void OutsideSave_SystemExit(SAVE_CONTROL_WORK *ctrl)
{
  GFL_HEAP_FreeMemory(ctrl);
}

///管理外セーブデータシステム構造体
typedef struct{
  GFL_SAVEDATA *sv;
  BOOL data_exists;         ///<TRUE:データが存在する　FALSE:存在しない
  BOOL data_break;          ///<TRUE:データが破壊されている
}OUTSIDE_SAVE_CONTROL;

///管理外セーブデータ識別ID
enum{
  OUTSIDE_GMDATA_ID_MYSTERY,  //不思議な贈り物
};

//--------------------------------------------------------------
//  管理外セーブデータのテーブル
//--------------------------------------------------------------
static const GFL_SAVEDATA_TABLE SaveDataTbl_Outside[] = {
  {
    OUTSIDE_GMDATA_ID_MYSTERY,
    (FUNC_GET_SIZE)BattleRec_GetWorkSize,
    (FUNC_INIT_WORK)BattleRec_WorkInit,
  },
};

//--------------------------------------------------------------
/**
 * @brief   管理外セーブパラメータ
 */
//--------------------------------------------------------------
static const GFL_SVLD_PARAM SaveParam_OutsideTbl = {
  {//戦闘録画：自分
    SaveDataTbl_Outside,
    NELEMS(SaveDataTbl_Outside),
    OUTSIDE_MM_MYSTERY,              //バックアップ領域先頭アドレス
    OUTSIDE_MM_MYSTERY_MIRROR,       //ミラーリング領域先頭アドレス
    SAVESIZE_OUTSIDE_MYSTERY,        //使用するバックアップ領域の大きさ
    SAVE_MAGIC_NUMBER,
  },
};


//==================================================================
/**
 * 管理外セーブデータのロード
 *
 * @param   ctrl		    セーブデータ管理ワークへのポインタ
 * @param   heap_id		  読み込んだデータをこのヒープに展開
 *
 * @retval  LOAD_RESULT		ロード結果
 * 
 * 使用が終わったら必ずOutsideSave_Unloadで解放してください(ロード出来なかったとしても)
 */
//==================================================================
LOAD_RESULT OutsideSave_Load(SAVE_CONTROL_WORK *ctrl, HEAPID heap_id)
{
	LOAD_RESULT load_ret;

  GF_ASSERT(ctrl->sv == NULL);
	ctrl->sv = GFL_SAVEDATA_Create(&SaveParam_OutsideTbl, heap_id);
	
	load_ret = GFL_BACKUP_Load(ctrl->sv, heap_id);

	switch(load_ret){
	case LOAD_RESULT_OK:
	case LOAD_RESULT_NG:
		ctrl->data_exists = TRUE;
		break;
	case LOAD_RESULT_BREAK:			///<破壊、復旧不能
	  ctrl->data_break = TRUE;
	  //break fall throw
	case LOAD_RESULT_NULL:		  ///<データなし
	case LOAD_RESULT_ERROR:			///<機器故障(Flashが無い)などで読み取り不能
	default:
    ctrl->data_exists = FALSE;
	  break;
	}
	
	OS_TPrintf("管理外セーブロード結果 load_ret = %d\n", load_ret);
	return load_ret;
}

//==================================================================
/**
 * 管理外セーブデータをアンロード
 *
 * @param   ctrl		
 */
//==================================================================
void OutsideSave_Unload(SAVE_CONTROL_WORK *ctrl)
{
  GFL_SAVEDATA_Delete(ctrl->sv);
  ctrl->sv = NULL;
}

//--------------------------------------------------------------
/**
 * @brief   管理外セーブデータの分割セーブ初期化
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 * @retval  セーブ結果
 */
//--------------------------------------------------------------
void OutsideSave_SaveAsyncInit(SAVE_CONTROL_WORK *ctrl)
{
	GFL_BACKUP_SAVEASYNC_Init(ctrl->sv);
}

//--------------------------------------------------------------
/**
 * @brief   管理外セーブデータの分割セーブメイン処理
 * @param   ctrl		セーブデータ管理ワークへのポインタ
 * @retval  セーブ結果
 */
//--------------------------------------------------------------
SAVE_RESULT OutsideSave_SaveAsyncMain(SAVE_CONTROL_WORK *ctrl)
{
	SAVE_RESULT result;
	
	result = GFL_BACKUP_SAVEASYNC_Main(ctrl->sv);
	if(result == SAVE_RESULT_OK){
		ctrl->data_exists = TRUE;
	}
	return result;
}

//==================================================================
/**
 * 管理外セーブデータの各セーブワークのポインタを取得する
 *
 * @param   ctrl		    
 * @param   gmdata_id		取得したいセーブデータのID
 *
 * @retval  void *		  セーブ領域へのポインタ
 */
//==================================================================
void * OutsideSave_DataPtrGet(SAVE_CONTROL_WORK *ctrl, GFL_SVDT_ID gmdata_id)
{
	return GFL_SAVEDATA_Get(ctrl->sv, gmdata_id);
}


//==================================================================
/**
 * 管理外セーブデータ領域を消去する
 *
 * @param   heap_temp_id		
 */
//==================================================================
void SaveControl_Outside_Erase(HEAPID heap_temp_id)
{
  GFL_BACKUP_FlashErase(heap_temp_id, OUTSIDE_MM_MYSTERY, 0x800);
}


///管理外セーブ：破壊状況
typedef enum{
  OUTSIDE_BREAK_OK,     ///<両面正常
  OUTSIDE_BREAK_A,      ///<A面破壊
  OUTSIDE_BREAK_B,      ///<B面破壊
  OUTSIDE_BREAK_ALL,    ///<両面破壊
}OUTSIDE_BREAK;

///管理外セーブ：マジックナンバー
#define OUTSIDE_MAGIC_NUMBER    (0xa10f49ae)

///管理外不思議データで所持できるカード数
#define OUTSIDE_MYSTERY_MAX     (3)

///ミラーリングの数
enum{
  _MIRROR_A,
  _MIRROR_B,
  
  _MIRROR_NUM,
};

///管理外セーブ：不思議な贈り物
typedef struct{
  u8 recv_flag[MYSTERY_DATA_MAX_EVENT / 8];		//256 * 8 = 2048 bit
  GIFT_PACK_DATA card[OUTSIDE_MYSTERY_MAX];		// カード情報
  RECORD_CRC crc;   //CRC & 暗号化キー   4 byte
}OUTSIDE_MYSTERY;

///管理外セーブ：実際にフラッシュに記録されるデータ(これがこのままフラッシュに書かれる)
typedef struct{
  u32 magic_number;   ///<データの存在有無を示すマジックナンバー
                      ///< (削除はここだけを消す為、必ず先頭に配置する事！！)
  u32 global_count;
  u16 crc;            ///<OUTSIDE_MYSTERYのCRC値
  u16 padding;
  OUTSIDE_MYSTERY mystery;
}OUTSIDE_MYSTERY_SAVEDATA;

///管理外セーブコントロールシステム
typedef struct{
  OUTSIDE_MYSTERY_SAVEDATA mystery_save;
  OUTSIDE_BREAK break_flag;        ///<フラッシュの破壊状況(OUTSIDE_BREAK_xxx)
  u8 data_exists;                  ///<TRUE:データが存在する  FALSE:存在しない
  u8 padding[3];
}OUTSIDE_SAVE_CONTROL;


//==================================================================
/**
 * 管理外セーブのシステムを作成し、ロード
 *
 * @param   heap_id		
 *
 * @retval  OUTSIDE_MYSTERY_SAVESYSTEM *		作成された管理外セーブシステムへのポインタ
 */
//==================================================================
OUTSIDE_MYSTERY_SAVESYSTEM * OutsideSave_SystemLoad(HEAPID heap_id)
{
  OUTSIDE_SAVE_CONTROL *outsv_ctrl;
  OUTSIDE_MYSTERY_SAVESYSTEM *buf[OUTSIDE_MIRROR_NUM];
  BOOL load_ret[OUTSIDE_MIRROR_NUM];  //FALSE:データが存在しない
  BOOL crc_break[OUTSIDE_MIRROR_NUM]; //TRUE:CRCが一致しない
  int no;
  int ok_side = -1;
  
  outsv_ctrl = GFL_HEAP_AllocClearMemory(heap_id, sizeof(OUTSIDE_SAVE_CONTROL));
  buf[_MIRROR_A] = &oubsv_ctrl->mystery_save;  //A面はシステムのバッファをそのまま使用
  buf[_MIRROR_B] = GFL_HEAP_AllocClearMemory(heap_id, sizeof(OUTSIDE_MYSTERY_SAVESYSTEM));
  
  for(no = 0; no < OUTSIDE_MIRROR_NUM; no++){
    load_ret[no] = FALSE;
    crc_break[no] = FALSE;
  }
  
  load_ret[_MIRROR_A] = GFL_BACKUP_DirectFlashLoad(OUTSIDE_MM_MYSTERY, buf_a, SAVESIZE_OUTSIDE_MYSTERY);
  load_ret[_MIRROR_B] = GFL_BACKUP_DirectFlashLoad(OUTSIDE_MM_MYSTERY_MIRROR, buf_b, SAVESIZE_OUTSIDE_MYSTERY);
  
  for(no = 0; no < OUTSIDE_MIRROR_NUM; no++){
    if(load_ret[no] == TRUE){
      //マジックナンバーでデータの存在チェック
      if(buf[no]->magic_number != OUTSIDE_MAGIC_NUMBER){
        load_ret[no] = FALSE;
        continue;
      }
      
      //CRCチェック
      if(GFL_STD_CrcCalc(&buf[no]->mystery, sizeof(OUTSIDE_MYSTERY)) != buf[no]->crc){
        crc_break[no] = TRUE;
      }
    }
  }
  
  //ロード結果とCRCをチェックして、使用する面と破壊状況を調べる
  outsv_ctrl->data_exists = FALSE;
  if(load_ret[_MIRROR_A] == FALSE && load_ret[_MIRROR_B] == FALSE){
    //両面データなし
    ;
  }
  else if(load_ret[_MIRROR_A] == TRUE && load_ret[_MIRROR_B] == FALSE){
    if(crc_break[_MIRROR_A] == TRUE){
      outsv_ctrl->break_flag = OUTSIDE_BREAK_ALL; //A面のみあるが、データ破壊
    }
    else{
      ok_side = _MIRROR_A;
    }
  }
  else if(load_ret[_MIRROR_A] == FALSE && load_ret[_MIRROR_B] == TRUE){
    if(crc_break[_MIRROR_B] == TRUE){
      outsv_ctrl->break_flag = OUTSIDE_BREAK_ALL; //B面のみあるが、データ破壊
    }
    else{
      ok_side = _MIRROR_B;
    }
  }
  else if(load_ret[_MIRROR_A] == TRUE && load_ret[_MIRROR_B] == TRUE){
    if(crc_break[_MIRROR_A] == TRUE && crc_break[_MIRROR_B] == TRUE){
      outsv_ctrl->break_flag = OUTSIDE_BREAK_ALL; //両面データ破壊
    }
    else if(crc_break[_MIRROR_A] == FALSE && crc_break[_MIRROR_B] == FALSE){
      if(buf[_MIRROR_A]->global_count >= buf[_MIRROR_B]->global_count
          || (buf[_MIRROR_A]->global_count == 0 && buf[_MIRROR_B]->global_count == 0xffffffff)){
        ok_side = _MIRROR_A;
      }
      else{
        ok_side = _MIRROR_B;
      }
    }
    else if(crc_break[_MIRROR_A] == FALSE){
      outsv_ctrl->break_flag = OUTSIDE_BREAK_B;
      ok_side = _MIRROR_A;
    }
    else{
      outsv_ctrl->break_flag = OUTSIDE_BREAK_A;
      ok_side = _MIRROR_B;
    }
  }
  
  //B面バッファは破棄するので、正常なデータがB面にあった場合はA面バッファにコピー
  if(ok_side == _MIRROR_B){
    GFL_STD_MemCopy(buf[_MIRROR_B], buf[_MIRROR_A], sizeof(OUTSIDE_MYSTERY_SAVESYSTEM));
  }
  GFL_HEAP_FreeMemory(buf[_MIRROR_B]);
  return outsv_ctrl;
}

//==================================================================
/**
 * 管理外セーブシステムを破棄する
 *
 * @param   outsv_ctrl		
 */
//==================================================================
void OutsideSave_SystemUnload(OUTSIDE_MYSTERY_SAVESYSTEM *outsv_ctrl)
{
  GFL_HEAP_FreeMemory(outsv_ctrl);
}

//==================================================================
/**
 * 管理外セーブ：分割セーブ初期化処理
 *
 * @param   outsv_ctrl		
 */
//==================================================================
void OutsideSave_SaveAsyncInit(OUTSIDE_MYSTERY_SAVESYSTEM *outsv_ctrl)
{
  outsv_ctrl->save_seq = 0;
  if(outsv_ctrl->break_flag == OUTSIDE_BREAK_B){
    outsv_ctrl->write_side = _MIRROR_B; //B面が壊れている場合はB面から
  }
  else{
    outsv_ctrl->write_side = _MIRROR_A; //それ以外は全てA面から
  }
  
  outsv_ctrl->mystery_save.magic_number = OUTSIDE_MAGIC_NUMBER;
  outsv_ctrl->mystery_save.global_counter++;
  outsv_ctrl->mystery_save.crc 
    = GFL_STD_CrcCalc(&outsv_ctrl->mystery_save.mystery, sizeof(OUTSIDE_MYSTERY));
}

//==================================================================
/**
 * 管理外セーブ：分割セーブメイン処理
 *
 * @param   outsv_ctrl		
 */
//==================================================================
void OutsideSave_SaveAsyncMain(OUTSIDE_MYSTERY_SAVESYSTEM *outsv_ctrl)
{
  u32 flash_src;
  BOOL err_result;
  
  switch(outsv_ctrl->save_seq){
  case 0:
  case 2:
    OS_TPrintf("管理外セーブ write_side = %d\n", outsv_ctrl->write_side);
    flash_src = outsv_ctrl->write_side == _MIRROR_A ? OUTSIDE_MM_MYSTERY : OUTSIDE_MM_MYSTERY_MIRROR;
    outsv_ctrl->lock_id = GFL_BACKUP_DirectFlashSaveAsyncInit(
      flash_src, &outsv_ctrl->mystery_save, sizeof(OUTSIDE_MYSTERY_SAVEDATA) );
    outsv_ctrl->save_seq++;
    break;
  case 1:
  case 3:
    if(GFL_BACKUP_DirectFlashSaveAsyncMain(outsv_ctrl->lock_id, &err_result) == TRUE){
      OS_TPrintf("管理外セーブ result(1=成功) = %d\n", err_result);
      outsv_ctrl->write_side ^= 1;
      outsv_ctrl->save_seq++;
    }
    break;
  default:
    return TRUE;
  }
  return FALSE;
}

//==================================================================
/**
 * 管理外セーブ：消去
 *
 * 消去用のメモリ確保の必要性や速度などの考慮から、マジックナンバーだけを消去します
 */
//==================================================================
void OutsideSave_SaveErase(void)
{
  u32 erase_magic_number = 0;
  
  OS_TPrintf("== outside Erase start  ==\n");
  //マジックナンバーだけを消す
  GFL_BACKUP_DirectFlashSave(OUTSIDE_MM_MYSTERY, &erase_magic_number, sizeof(u32));
  GFL_BACKUP_DirectFlashSave(OUTSIDE_MM_MYSTERY_MIRROR, &erase_magic_number, sizeof(u32));
  OS_TPrintf("== outside Erase finish ==\n");
}
