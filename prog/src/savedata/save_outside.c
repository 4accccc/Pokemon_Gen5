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

#include "savedata/mystery_data.h"
#include "savedata/mystery_data_local.h"

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
  SAVE_MIRROR_MYSTERY_A,
  SAVE_MIRROR_MYSTERY_B,
  
  SAVE_PERMANENT,

  SAVE_NUM,
};


//==============================================================================
//  構造体定義
//==============================================================================
typedef struct
{
  u32 magic_number;   ///<データの存在有無を示すマジックナンバー
                      ///< (削除はここだけを消す為、必ず先頭に配置する事！！)
  u32 global_count;
  u16 crc;            ///<OUTSIDE_MYSTERYのCRC値
  u16 size;
}OUTSIDE_RECORD;

typedef struct
{
  OUTSIDE_RECORD rec;
  u8  data[0];
}OUTSIDE_RECORD_ABSTRUCT;

///管理外セーブ：実際にフラッシュに記録されるデータ(これがこのままフラッシュに書かれる)
typedef struct{
#if 0
  u32 magic_number;   ///<データの存在有無を示すマジックナンバー
                      ///< (削除はここだけを消す為、必ず先頭に配置する事！！)
  u32 global_count;
  u16 crc;            ///<OUTSIDE_MYSTERYのCRC値
  u16 padding;
#else
  OUTSIDE_RECORD rec;
#endif
  OUTSIDE_MYSTERY mystery;
}OUTSIDE_MYSTERY_SAVEDATA;

typedef struct
{
  OUTSIDE_RECORD rec;
  u8 permanent[OUTSIDE_PERMANENT_WORK_SIZE];
}OUTSIDE_PERMANENT_SAVEDATA;

///管理外セーブコントロールシステム
struct _OUTSIDE_SAVE_CONTROL{
  OUTSIDE_MYSTERY_SAVEDATA mystery_save;
  OUTSIDE_PERMANENT_SAVEDATA permanent_save;
  OUTSIDE_BREAK break_flag;        ///<フラッシュの破壊状況(OUTSIDE_BREAK_xxx)
  u8 data_exists;                  ///<TRUE:データが存在する  FALSE:存在しない
  u8 save_seq;
  u8 write_side;
  u8 permanent_data_exists;
  u16 lock_id;
  u8  permanent_break_flag;  //TRUEで破壊  FALSEで破壊していない
  u8 padding2;
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
  OUTSIDE_RECORD_ABSTRUCT *buf[SAVE_NUM];
  BOOL load_ret[SAVE_NUM];  //FALSE:データが存在しない
  BOOL crc_break[SAVE_NUM]; //TRUE:CRCが一致しない
  int no;
  int ok_side = -1;
  
  outsv_ctrl = GFL_HEAP_AllocClearMemory(heap_id, sizeof(OUTSIDE_SAVE_CONTROL));
  buf[SAVE_MIRROR_MYSTERY_A] = (OUTSIDE_RECORD_ABSTRUCT *)&outsv_ctrl->mystery_save;  //A面はシステムのバッファをそのまま使用
  buf[SAVE_MIRROR_MYSTERY_B] = GFL_HEAP_AllocClearMemory(heap_id, sizeof(OUTSIDE_MYSTERY_SAVEDATA));
  buf[SAVE_PERMANENT] = (OUTSIDE_RECORD_ABSTRUCT *)&outsv_ctrl->permanent_save;
  
  for(no = 0; no < SAVE_NUM; no++){
    load_ret[no] = FALSE;
    crc_break[no] = FALSE;
  }
  
  load_ret[SAVE_MIRROR_MYSTERY_A] = GFL_BACKUP_DirectFlashLoad(OUTSIDE_MM_MYSTERY, buf[SAVE_MIRROR_MYSTERY_A], sizeof(OUTSIDE_MYSTERY_SAVEDATA));
  load_ret[SAVE_MIRROR_MYSTERY_B] = GFL_BACKUP_DirectFlashLoad(OUTSIDE_MM_MYSTERY_MIRROR, buf[SAVE_MIRROR_MYSTERY_B], sizeof(OUTSIDE_MYSTERY_SAVEDATA));
  load_ret[SAVE_PERMANENT] = GFL_BACKUP_DirectFlashLoad(OUTSIDE_MM_PERMANENT, buf[SAVE_PERMANENT], sizeof(OUTSIDE_PERMANENT_SAVEDATA));

  buf[SAVE_MIRROR_MYSTERY_A]->rec.size = sizeof(OUTSIDE_MYSTERY);
  buf[SAVE_MIRROR_MYSTERY_B]->rec.size = sizeof(OUTSIDE_MYSTERY);
  buf[SAVE_PERMANENT]->rec.size = OUTSIDE_PERMANENT_WORK_SIZE;
  
  for(no = 0; no < SAVE_NUM; no++){
    if(load_ret[no] == TRUE){
      //マジックナンバーでデータの存在チェック
      if(buf[no]->rec.magic_number != OUTSIDE_MAGIC_NUMBER){
        load_ret[no] = FALSE;
        NAGI_Printf( "[%d]のmagic_numberが違う[%d]\n", no, buf[no]->rec.magic_number );
        continue;
      }
      
      //CRCチェック
      if(GFL_STD_CrcCalc(buf[no]->data, buf[no]->rec.size) != buf[no]->rec.crc){
        crc_break[no] = TRUE;
        NAGI_Printf( "[%d]のcrcが違う[%x]!=[%x]\n", no, GFL_STD_CrcCalc(buf[no]->data, buf[no]->rec.size) , buf[no]->rec.crc );
      }
    }
  }
  
  //ロード結果とCRCをチェックして、使用する面と破壊状況を調べる
  //不思議の破壊状況チェック
  outsv_ctrl->data_exists = FALSE;
  if(load_ret[SAVE_MIRROR_MYSTERY_A] == FALSE && load_ret[SAVE_MIRROR_MYSTERY_B] == FALSE){
    //両面データなし
    ;
  }
  else if(load_ret[SAVE_MIRROR_MYSTERY_A] == TRUE && load_ret[SAVE_MIRROR_MYSTERY_B] == FALSE){
    if(crc_break[SAVE_MIRROR_MYSTERY_A] == TRUE){
      outsv_ctrl->break_flag = OUTSIDE_BREAK_ALL; //A面のみあるが、データ破壊
    }
    else{
      ok_side = SAVE_MIRROR_MYSTERY_A;
    }
  }
  else if(load_ret[SAVE_MIRROR_MYSTERY_A] == FALSE && load_ret[SAVE_MIRROR_MYSTERY_B] == TRUE){
    if(crc_break[SAVE_MIRROR_MYSTERY_B] == TRUE){
      outsv_ctrl->break_flag = OUTSIDE_BREAK_ALL; //B面のみあるが、データ破壊
    }
    else{
      ok_side = SAVE_MIRROR_MYSTERY_B;
    }
  }
  else if(load_ret[SAVE_MIRROR_MYSTERY_A] == TRUE && load_ret[SAVE_MIRROR_MYSTERY_B] == TRUE){
    if(crc_break[SAVE_MIRROR_MYSTERY_A] == TRUE && crc_break[SAVE_MIRROR_MYSTERY_B] == TRUE){
      outsv_ctrl->break_flag = OUTSIDE_BREAK_ALL; //両面データ破壊
    }
    else if(crc_break[SAVE_MIRROR_MYSTERY_A] == FALSE && crc_break[SAVE_MIRROR_MYSTERY_B] == FALSE){
      if(buf[SAVE_MIRROR_MYSTERY_A]->rec.global_count >= buf[SAVE_MIRROR_MYSTERY_B]->rec.global_count
          || (buf[SAVE_MIRROR_MYSTERY_A]->rec.global_count == 0 && buf[SAVE_MIRROR_MYSTERY_B]->rec.global_count == 0xffffffff)){
        ok_side = SAVE_MIRROR_MYSTERY_A;
      }
      else{
        ok_side = SAVE_MIRROR_MYSTERY_B;
      }
    }
    else if(crc_break[SAVE_MIRROR_MYSTERY_A] == FALSE){
      outsv_ctrl->break_flag = OUTSIDE_BREAK_B;
      ok_side = SAVE_MIRROR_MYSTERY_A;
    }
    else{
      outsv_ctrl->break_flag = OUTSIDE_BREAK_A;
      ok_side = SAVE_MIRROR_MYSTERY_B;
    }
  }

  //キーシステムの破壊状況チェック
  outsv_ctrl->permanent_data_exists = FALSE;
  outsv_ctrl->permanent_break_flag  = FALSE;
  if( load_ret[SAVE_PERMANENT] == FALSE )
  {
    //データなし。ならば、初期化を行う
    OUTSIDE_PERMANENT_Init( (OUTSIDE_PERMANENT*)outsv_ctrl->permanent_save.permanent );
  }
  else
  {
    if( crc_break[SAVE_PERMANENT] == TRUE )
    {
      //あったが壊れていた
      outsv_ctrl->permanent_break_flag = TRUE;
      OUTSIDE_PERMANENT_Init( (OUTSIDE_PERMANENT*)outsv_ctrl->permanent_save.permanent );
    }
    else
    {
      //正常に読み取った
      outsv_ctrl->permanent_data_exists = TRUE;
    }
  }
  
  //B面バッファは破棄するので、正常なデータがB面にあった場合はA面バッファにコピー
  if(ok_side == SAVE_MIRROR_MYSTERY_B){
    GFL_STD_MemCopy(buf[SAVE_MIRROR_MYSTERY_B], buf[SAVE_MIRROR_MYSTERY_A], sizeof(OUTSIDE_MYSTERY_SAVEDATA));
  }
  GFL_HEAP_FreeMemory(buf[SAVE_MIRROR_MYSTERY_B]);

  if(ok_side != -1){
    outsv_ctrl->data_exists = TRUE;
  }
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
    outsv_ctrl->write_side = SAVE_MIRROR_MYSTERY_B; //B面が壊れている場合はB面から
  }
  else{
    outsv_ctrl->write_side = SAVE_MIRROR_MYSTERY_A; //それ以外は全てA面から
  }
  
  outsv_ctrl->mystery_save.rec.magic_number = OUTSIDE_MAGIC_NUMBER;
  outsv_ctrl->mystery_save.rec.global_count++;
  outsv_ctrl->mystery_save.rec.crc 
    = GFL_STD_CrcCalc(&outsv_ctrl->mystery_save.mystery, sizeof(OUTSIDE_MYSTERY));
  NAGI_Printf( "MYSTERYののcrc[%x]\n", outsv_ctrl->mystery_save.rec.crc );
  outsv_ctrl->permanent_save.rec.magic_number = OUTSIDE_MAGIC_NUMBER;
  outsv_ctrl->permanent_save.rec.global_count++;
  outsv_ctrl->permanent_save.rec.crc 
    = GFL_STD_CrcCalc(&outsv_ctrl->permanent_save.permanent, OUTSIDE_PERMANENT_WORK_SIZE);
  NAGI_Printf( "PERMANENTののcrc[%x]\n", outsv_ctrl->mystery_save.rec.crc );
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
    flash_src = outsv_ctrl->write_side == SAVE_MIRROR_MYSTERY_A ? OUTSIDE_MM_MYSTERY : OUTSIDE_MM_MYSTERY_MIRROR;
    outsv_ctrl->lock_id = GFL_BACKUP_DirectFlashSaveAsyncInit(
      flash_src, &outsv_ctrl->mystery_save, sizeof(OUTSIDE_MYSTERY_SAVEDATA) );
    outsv_ctrl->save_seq++;
    break;
  case 1:
  case 3:
  case 5:
    if(GFL_BACKUP_DirectFlashSaveAsyncMain(outsv_ctrl->lock_id, &err_result) == TRUE){
      OS_TPrintf("管理外セーブ result(1=成功) = %d\n", err_result);
      outsv_ctrl->write_side ^= 1;
      outsv_ctrl->save_seq++;
    }
    break;
  case 4:
    OS_TPrintf("管理外セーブ 永続情報\n");
    outsv_ctrl->lock_id = GFL_BACKUP_DirectFlashSaveAsyncInit(
      OUTSIDE_MM_PERMANENT, &outsv_ctrl->permanent_save, sizeof(OUTSIDE_PERMANENT_SAVEDATA) );
    outsv_ctrl->save_seq++;
    break;
  default:
    outsv_ctrl->break_flag = OUTSIDE_BREAK_OK;
    outsv_ctrl->data_exists = TRUE;
    return TRUE;
  }
  return FALSE;
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
  if(outsv_ctrl->break_flag != OUTSIDE_BREAK_ALL){
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
  MYSTERY_ORIGINAL_DATA *normal_mystery = SaveControl_DataPtrGet(normal_ctrl, GMDATA_ID_MYSTERYDATA);
  OUTSIDE_MYSTERY *outside_mystery = &outsv_ctrl->mystery_save.mystery;
  int i;

  //復号化
  MYSTERYDATA_Decoded( normal_mystery, MYSTERY_DATA_TYPE_ORIGINAL );
  MYSTERYDATA_Decoded( outside_mystery, MYSTERY_DATA_TYPE_OUTSIDE );

  //データを入れる
  for(i = 0; i < MYSTERY_DATA_MAX_EVENT / 8; i++){
    normal_mystery->recv_flag[i] = outside_mystery->recv_flag[i];
  }
  for(i = 0; i < OUTSIDE_MYSTERY_MAX; i++){
    normal_mystery->card[i] = outside_mystery->card[i];
  }

  //暗号化
  MYSTERYDATA_Coded( normal_mystery, MYSTERY_DATA_TYPE_ORIGINAL );
  MYSTERYDATA_Coded( outside_mystery, MYSTERY_DATA_TYPE_OUTSIDE );
}

//==================================================================
/**
 * 管理外セーブ全体をクリアする
 *
 * @param   heap_id		
 */
//==================================================================
void OutsideSave_FlashAllErase(HEAPID heap_id)
{
  u32 erase_size = SAVESIZE_OUTSIDE_MYSTERY * 2 + SAVESIZE_OUTSIDE_PERMANENT;  //*2=ミラー分と永続データ
  u8 *erase_buffer = GFL_HEAP_AllocMemory(heap_id, erase_size);
  
  GFL_STD_MemFill32(erase_buffer, 0xffffffff, erase_size);
  GFL_BACKUP_DirectFlashSave(OUTSIDE_MM_MYSTERY, erase_buffer, erase_size);
  
  GFL_HEAP_FreeMemory(erase_buffer);
}
//----------------------------------------------------------------------------
/**
 *	@brief  管理外セーブから永続情報存在フラグを取得
 *
 *	@param	OUTSIDE_SAVE_CONTROL *outsv_ctrl管理外セーブ
 *
 *	@return　TRUE存在 FALSEない
 */
//-----------------------------------------------------------------------------
BOOL OutsideSave_GetPermanentExistFlag(const OUTSIDE_SAVE_CONTROL *outsv_ctrl)
{
  return outsv_ctrl->permanent_data_exists;
}
//----------------------------------------------------------------------------
/**
 *	@brief  管理外セーブから永続情報壊れたフラグ取得
 *
 *	@param	OUTSIDE_SAVE_CONTROL *outsv_ctrl  管理外セーブ
 *
 *	@return TRUE　壊れてる FALSE壊れてない
 */
//-----------------------------------------------------------------------------
BOOL OutsideSave_GetPermanentBreakFlag(const OUTSIDE_SAVE_CONTROL *outsv_ctrl)
{
  return outsv_ctrl->permanent_break_flag;
}
//----------------------------------------------------------------------------
/**
 *	@brief  管理外セーブから永続情報取得
 *
 *	@param	*outsv_ctrl 管理外セーブ
 *
 *	@return 永続情報
 */
//-----------------------------------------------------------------------------
OUTSIDE_PERMANENT * OutsideSave_GetPermanentPtr(OUTSIDE_SAVE_CONTROL *outsv_ctrl)
{
  return (OUTSIDE_PERMANENT *)&outsv_ctrl->permanent_save.permanent;
}

//----------------------------------------------------------------------------
/**
 *	@brief  管理外セーブから永続情報をAllocしたメモリにコピーし取得
 *	        なので、管理外セーブ自体を終了させても問題なし
 *
 *	@param	*outsv_ctrl 管理外セーブ
 *
 *	@return 永続情報
 */
//-----------------------------------------------------------------------------
OUTSIDE_PERMANENT * OutsideSave_AllocCopyPermanent(OUTSIDE_SAVE_CONTROL *outsv_ctrl, HEAPID heapID )
{
  OUTSIDE_PERMANENT *p_dst = GFL_HEAP_AllocClearMemory( heapID, OUTSIDE_PERMANENT_WORK_SIZE );
  const OUTSIDE_PERMANENT *cp_src = OutsideSave_GetPermanentPtr(outsv_ctrl);

  GFL_STD_MemCopy( cp_src, p_dst, OUTSIDE_PERMANENT_WORK_SIZE );
  return p_dst;
}
//==============================================================================
//  デバッグ用
//==============================================================================
#ifdef PM_DEBUG
//==================================================================
/**
 * デバッグ用：セーブデータの破壊フラグの取得
 *
 * @param   outsv_ctrl		
 * @param   ret_a		      A面結果代入先(TRUE:データ破壊)
 * @param   ret_b		      B面結果代入先(TRUE:データ破壊)
 *
 * @retval  BOOL		TRUE:データあり　FALSE:データ無し
 */
//==================================================================
BOOL DEBUG_OutsideSave_GetBreak(OUTSIDE_SAVE_CONTROL *outsv_ctrl, BOOL *ret_a, BOOL *ret_b)
{
  *ret_a = FALSE;
  *ret_b = FALSE;
  switch(outsv_ctrl->break_flag){
  case OUTSIDE_BREAK_OK:     //両面正常
    return outsv_ctrl->data_exists;
  case OUTSIDE_BREAK_A:      //A面破壊
    *ret_a = TRUE;
    break;
  case OUTSIDE_BREAK_B:      //B面破壊
    *ret_b = TRUE;
    break;
  case OUTSIDE_BREAK_ALL:    //両面破壊
    *ret_a = TRUE;
    *ret_b = TRUE;
    break;
  }
  
  return TRUE;
}
#endif  //PM_DEBUG

