//==============================================================================
/**
 * @file    save_outside.c
 * @brief   セーブ管理外領域のセーブシステム
 * @author  matsuda
 * @date    2010.04.10(土)
 */
//==============================================================================
#include "savedata/save_control.h"
#include "savedata/save_tbl.h"
#include "savedata/save_outside.h"



//==============================================================================
//  定数定義
//==============================================================================

///管理外セーブ：破壊状況
typedef enum{
  OUTSIDE_BREAK_OK,     ///<両面正常
  OUTSIDE_BREAK_A,      ///<A面破壊
  OUTSIDE_BREAK_B,      ///<B面破壊
  OUTSIDE_BREAK_ALL,    ///<両面破壊
}OUTSIDE_BREAK;

///管理外セーブ：マジックナンバー
#define OUTSIDE_MAGIC_NUMBER    (0xa10f49ae)

///ミラーリングの数
enum{
  _MIRROR_A,
  _MIRROR_B,
  
  _MIRROR_NUM,
};


//==============================================================================
//  構造体定義
//==============================================================================
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
struct _OUTSIDE_SAVE_CONTROL{
  OUTSIDE_MYSTERY_SAVEDATA mystery_save;
  OUTSIDE_BREAK break_flag;        ///<フラッシュの破壊状況(OUTSIDE_BREAK_xxx)
  u8 data_exists;                  ///<TRUE:データが存在する  FALSE:存在しない
  u8 save_seq;
  u8 write_side;
  u8 padding;
  u16 lock_id;
  u16 padding2;
};



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * 管理外セーブのシステムを作成し、ロード
 *
 * @param   heap_id		
 *
 * @retval  OUTSIDE_SAVE_CONTROL *		作成された管理外セーブシステムへのポインタ
 */
//==================================================================
OUTSIDE_SAVE_CONTROL * OutsideSave_SystemLoad(HEAPID heap_id)
{
  OUTSIDE_SAVE_CONTROL *outsv_ctrl;
  OUTSIDE_MYSTERY_SAVEDATA *buf[_MIRROR_NUM];
  BOOL load_ret[_MIRROR_NUM];  //FALSE:データが存在しない
  BOOL crc_break[_MIRROR_NUM]; //TRUE:CRCが一致しない
  int no;
  int ok_side = -1;
  
  outsv_ctrl = GFL_HEAP_AllocClearMemory(heap_id, sizeof(OUTSIDE_SAVE_CONTROL));
  buf[_MIRROR_A] = &outsv_ctrl->mystery_save;  //A面はシステムのバッファをそのまま使用
  buf[_MIRROR_B] = GFL_HEAP_AllocClearMemory(heap_id, sizeof(OUTSIDE_SAVE_CONTROL));
  
  for(no = 0; no < _MIRROR_NUM; no++){
    load_ret[no] = FALSE;
    crc_break[no] = FALSE;
  }
  
  load_ret[_MIRROR_A] = GFL_BACKUP_DirectFlashLoad(OUTSIDE_MM_MYSTERY, buf[_MIRROR_A], SAVESIZE_OUTSIDE_MYSTERY);
  load_ret[_MIRROR_B] = GFL_BACKUP_DirectFlashLoad(OUTSIDE_MM_MYSTERY_MIRROR, buf[_MIRROR_B], SAVESIZE_OUTSIDE_MYSTERY);
  
  for(no = 0; no < _MIRROR_NUM; no++){
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
    GFL_STD_MemCopy(buf[_MIRROR_B], buf[_MIRROR_A], sizeof(OUTSIDE_SAVE_CONTROL));
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
void OutsideSave_SystemUnload(OUTSIDE_SAVE_CONTROL *outsv_ctrl)
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
void OutsideSave_SaveAsyncInit(OUTSIDE_SAVE_CONTROL *outsv_ctrl)
{
  outsv_ctrl->save_seq = 0;
  if(outsv_ctrl->break_flag == OUTSIDE_BREAK_B){
    outsv_ctrl->write_side = _MIRROR_B; //B面が壊れている場合はB面から
  }
  else{
    outsv_ctrl->write_side = _MIRROR_A; //それ以外は全てA面から
  }
  
  outsv_ctrl->mystery_save.magic_number = OUTSIDE_MAGIC_NUMBER;
  outsv_ctrl->mystery_save.global_count++;
  outsv_ctrl->mystery_save.crc 
    = GFL_STD_CrcCalc(&outsv_ctrl->mystery_save.mystery, sizeof(OUTSIDE_MYSTERY));
}

//==================================================================
/**
 * 管理外セーブ：分割セーブメイン処理
 *
 * @param   outsv_ctrl		
 *
 * @retval  BOOL		TRUE:セーブ完了　FALSE:セーブ処理継続中
 */
//==================================================================
BOOL OutsideSave_SaveAsyncMain(OUTSIDE_SAVE_CONTROL *outsv_ctrl)
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
    outsv_ctrl->break_flag = OUTSIDE_BREAK_OK;
    outsv_ctrl->data_exists = TRUE;
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


//==============================================================================
//  ツール
//==============================================================================
//==================================================================
/**
 * セーブデータの存在フラグの取得
 *
 * @param   outsv_ctrl		
 *
 * @retval  BOOL		TRUE:データが存在する　FALSE:データが無い
 */
//==================================================================
BOOL OutsideSave_GetExistFlag(OUTSIDE_SAVE_CONTROL *outsv_ctrl)
{
  return outsv_ctrl->data_exists;
}

//==================================================================
/**
 * セーブデータの破壊フラグの取得
 *
 * @param   outsv_ctrl		
 *
 * @retval  BOOL		TRUE:両面データ破壊　FALSE:正常な面がある
 */
//==================================================================
BOOL OutsideSave_GetBreakFlag(OUTSIDE_SAVE_CONTROL *outsv_ctrl)
{
  if(outsv_ctrl->data_exists == FALSE || outsv_ctrl->break_flag != OUTSIDE_BREAK_ALL){
    return FALSE;
  }
  return TRUE;
}

//==================================================================
/**
 * 不思議な贈り物データへのポインタを取得する
 *
 * @param   outsv_ctrl		
 *
 * @retval  OUTSIDE_MYSTERY *		
 */
//==================================================================
OUTSIDE_MYSTERY * OutsideSave_GetMysterPtr(OUTSIDE_SAVE_CONTROL *outsv_ctrl)
{
  GF_ASSERT(outsv_ctrl->data_exists == TRUE);
  return &outsv_ctrl->mystery_save.mystery;
}

//==================================================================
/**
 * 管理外セーブのデータを通常セーブ領域にコピーする
 *
 * @param   outsv_ctrl		管理外セーブシステムへのポインタ
 * @param   normal_ctrl		通常セーブシステムへのポインタ
 */
//==================================================================
void OutsideSave_MysteryData_Outside_to_Normal(OUTSIDE_SAVE_CONTROL *outsv_ctrl, SAVE_CONTROL_WORK *normal_ctrl)
{
  MYSTERY_DATA *normal_mystery = SaveData_GetMysteryData(normal_ctrl);
  OUTSIDE_MYSTERY *outside_mystery = &outsv_ctrl->mystery_save.mystery;
  int i;
  
  for(i = 0; i < MYSTERY_DATA_MAX_EVENT / 8; i++){
    normal_mystery->recv_flag[i] = outside_mystery->recv_flag[i];
  }
  for(i = 0; i < OUTSIDE_MYSTERY_MAX; i++){
    normal_mystery->card[i] = outside_mystery->card[i];
  }
  normal_mystery->crc = outside_mystery->crc;
}

