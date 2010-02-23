/**
 *  @file mailsys.c
 *  @brief  メールフローコントロールフレーム
 *  @author Akito Mori / Miyuki Iwasawa
 *  @date 09.11.10
 */
 
#include <gflib.h>
#include <procsys.h>

#include "system/main.h"
#include "poke_tool/poke_tool.h"
#include "item/itemsym.h"
#include "item/item.h"
#include "sound/pm_sndsys.h"
#include "font/font.naix"
#include "gamesystem/game_data.h"
#include "app/mailtool.h"
#include "savedata/mail_util.h"
#include "savedata/config.h"
#include "mail_param.h"
#include "mailview.h"
#include "app/pms_input.h"
#include "mail_snd_def.h"

//オーバーレイプロセス定義データ
const GFL_PROC_DATA MailSysProcData = {
    MailSysProc_Init,
    MailSysProc_Main,
    MailSysProc_End,
};


enum{
  MAILCASE_INIT,
  MAILCASE_WAIT,
  MAILCASE_END,
  WORDCASE_INIT,
  WORDCASE_WAIT,
  WORDCASE_END,
};

//================================================================
//構造体宣言
//================================================================
//メールシステムメインワーク
typedef struct _MAIL_MAIN_DAT{
  int heapID;   ///<ヒープID
  u16 ret_val;  ///<リターン値
  u16 ret_no;   ///<リターン編集NO

  void  *app_wk;      ///<簡易会話モジュールワークの保存
  GFL_PROC *proc;     ///<サブプロセスワーク
  MAIL_TMP_DATA *dat; ///<メールデータテンポラリ

  PMS_DATA  tmpPms;   //<簡易会話テンポラリ

  GFL_PROCSYS *localProcSys;

}MAIL_MAIN_DAT;

//================================================================
//プロトタイプ宣言　ローカル
//================================================================
static MAIL_TMP_DATA* MailSys_AllocTmpData(const MAIL_DATA* org,int heapID);
static void MailSys_ReleaseTmpData(MAIL_TMP_DATA* tmp);
static void MailSys_SetTmpData(MAIL_DATA* org,MAIL_TMP_DATA* tmp);


//=============================================================================================
/**
 * @brief メール作成呼び出し　初期化
 *
 * @param   proc    
 * @param   seq   
 * @param   pwk   
 * @param   mywk    
 *
 * @retval  GFL_PROC_RESULT   
 */
//=============================================================================================
GFL_PROC_RESULT MailSysProc_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  MAIL_MAIN_DAT* wk;
  MAIL_PARAM* param = pwk;
  

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MAILSYS,0x1000 );
  wk = GFL_PROC_AllocWork( proc, sizeof(MAIL_MAIN_DAT), HEAPID_MAILSYS);
  MI_CpuClearFast(wk,sizeof(MAIL_MAIN_DAT));

  //ヒープID保存
  wk->heapID = HEAPID_MAILSYS;

  //データテンポラリ作成
  wk->dat = MailSys_AllocTmpData( param->pDat, wk->heapID );

  if(param->mode == MAIL_MODE_CREATE){
    //無効ナンバーが格納されているので、上位ワークから取得する
    wk->dat->design = param->designNo; //デザインNo適用
  }
  if(wk->dat->design >= MAIL_DESIGN_MAX){
    wk->dat->design = 0;
  }
  wk->dat->val = param->mode;

  // メールシステム内PROCSYS作成
  wk->localProcSys = GFL_PROC_LOCAL_boot( HEAPID_MAILSYS );
  
  //現在のキー動作モードを取得
//  wk->dat->kt_status = param->pKeytouch;

  return GFL_PROC_RES_FINISH;
}

//=============================================================================================
/**
 * @brief メール作成メイン
 *
 * @param   proc    
 * @param   seq   
 * @param   pwk   
 * @param   mywk    
 *
 * @retval  GFL_PROC_RESULT   
 */
//=============================================================================================
GFL_PROC_RESULT MailSysProc_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  MAIL_MAIN_DAT* wk = mywk;
  MAIL_PARAM* param = pwk;

  // オーバーレイID宣言
  FS_EXTERN_OVERLAY(pmsinput);
  FS_EXTERN_OVERLAY(ui_common);
  
  // プロセス定義データ
  static const GFL_PROC_DATA MailViewProcData = {
    MailViewProc_Init,
    MailViewProc_Main,
    MailViewProc_End,
  };
  static const GFL_PROC_DATA PMSProcData = {
    PMSInput_Init,
    PMSInput_Main,
    PMSInput_Quit,
  };

  //ローカルPROCの更新処理
  GFL_PROC_MAIN_STATUS proc_status = GFL_PROC_LOCAL_Main( wk->localProcSys );


  switch(*seq){
  case MAILCASE_INIT:
    wk->dat->val = param->mode; //動作モード値復帰
    GFL_PROC_LOCAL_CallProc( wk->localProcSys, NO_OVERLAY_ID, &MailViewProcData, wk->dat );
    *seq = MAILCASE_WAIT;
    break;
  case MAILCASE_WAIT:
    if(proc_status != GFL_PROC_MAIN_VALID){
      switch(wk->dat->val){
      case VIEW_END_CANCEL:
        *seq = MAILCASE_END;
        break;
      case VIEW_END_DECIDE:
        *seq = MAILCASE_END;
        break;
      default:
        *seq = WORDCASE_INIT; 
        break;
      }
    }
    break;
  case MAILCASE_END:
    //データ取得
    if( param->mode == MAIL_MODE_CREATE){
      if( wk->dat->val == VIEW_END_DECIDE){
        MailSys_SetTmpData(param->pDat,wk->dat);  //メールデータを書き換え
        //スコア加算処理
//        RECORD_Score_Add(SaveData_GetRecord(param->savedata),SCORE_ID_WRITE_MAIL);
//        RECORD_Inc(SaveData_GetRecord(param->savedata), RECID_MAIL_WRITE );
        param->ret_val = 1; //戻り値設定
      }else{
        param->ret_val = 0; //戻り値設定
      }
    }
    return GFL_PROC_RES_FINISH;
  case WORDCASE_INIT:
    GFL_OVERLAY_Load( FS_OVERLAY_ID(pmsinput));
//    GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));
    wk->app_wk = PMSI_PARAM_Create( PMSI_MODE_SENTENCE, PMSI_GUIDANCE_DEFAULT,
                                    NULL,TRUE,param->savedata,wk->heapID);

    //初期データセット
    if(PMSDAT_IsEnabled(&(wk->dat->msg[wk->dat->cntNo]))){
      //既に入力文がある
      PMSDAT_Copy(&(wk->tmpPms),&(wk->dat->msg[wk->dat->cntNo]));
    }else{
      //まだ空
      PMSDAT_Init(&(wk->tmpPms),PMS_TYPE_MAIL); 
    }
    PMSI_PARAM_SetInitializeDataSentence(wk->app_wk,&(wk->tmpPms));
    
    GFL_PROC_LOCAL_CallProc( wk->localProcSys, NO_OVERLAY_ID, &PMSProcData,  wk->app_wk );
    *seq     = WORDCASE_WAIT;
    break;
  case WORDCASE_WAIT:
    if(proc_status != GFL_PROC_MAIN_VALID){
      // 簡易会話取得
      if( PMSI_PARAM_CheckCanceled(wk->app_wk) == FALSE)
      {
        //変更があれば書き戻し
        PMSI_PARAM_GetInputDataSentence( wk->app_wk,  &(wk->dat->msg[wk->dat->cntNo]));
      }
      PMSI_PARAM_Delete(wk->app_wk);
      GFL_OVERLAY_Unload( FS_OVERLAY_ID(pmsinput));
  //    GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));    
  
      *seq = MAILCASE_INIT; 
    }
    break;
  }
  return GFL_PROC_RES_CONTINUE;
}

/**
 *  @brief  メール作成終了
 */
GFL_PROC_RESULT MailSysProc_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk )
{
  MAIL_MAIN_DAT *wk = mywk;

  // メールシステム内PROCSYS終了
  GFL_PROC_LOCAL_Exit( wk->localProcSys );

  //メールデータテンポラリ領域解放
  MailSys_ReleaseTmpData(wk->dat);

  //ワークエリア解放
  GFL_PROC_FreeWork(proc);
  
  GFL_HEAP_CheckHeapSafe(wk->heapID);

  GFL_HEAP_DeleteHeap(wk->heapID);

  return GFL_PROC_RES_FINISH;
}

/**
 *  @brief  メールデータ　テンポラリ作成
 *
 *  ＊内部でメモリをアロケートするので必ず呼び出し側が
 *  　MailSys_ReleaseTmpData()で解放すること
 */


//----------------------------------------------------------------------------------
/**
 * @brief メールデータ　テンポラリ作成
 *  ＊内部でメモリをアロケートするので必ず呼び出し側が
 *  　MailSys_ReleaseTmpData()で解放すること
 *
 * @param   org   
 * @param   heapID    
 *
 * @retval  MAIL_TMP_DATA*    
 */
//----------------------------------------------------------------------------------
static MAIL_TMP_DATA* MailSys_AllocTmpData(const MAIL_DATA* org,int heapID)
{
  u16 i;
  MAIL_TMP_DATA* tmp;

  tmp = GFL_HEAP_AllocMemory( heapID, sizeof(MAIL_TMP_DATA) );
  MI_CpuClearFast(tmp,sizeof(MAIL_TMP_DATA));
  
  tmp->val = 0;

  tmp->writerID = MailData_GetWriterID(org);
  tmp->name = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, heapID );
  GFL_STR_SetStringCode( tmp->name, MailData_GetWriterName((MAIL_DATA*)org) );
  tmp->design = MailData_GetDesignNo(org);
  tmp->lang   = MailData_GetCountryCode(org);
  tmp->ver    = MailData_GetCasetteVersion(org);

  for(i = 0;i < MAILDAT_ICONMAX;i++){
    tmp->icon[i].dat = MailData_GetIconParamByIndex( org, i, MAIL_ICONPRM_ALL, MailData_GetFormBit(org));
  }
  for(i = 0;i < MAILDAT_MSGMAX;i++){
    PMSDAT_Copy( &tmp->msg[i], MailData_GetMsgByIndex((MAIL_DATA*)org, i) );
  }
  return tmp;
}


//----------------------------------------------------------------------------------
/**
 * @brief メールデータ　テンポラリ解放
 *  ＊MailSys_AllocTmpData()で確保したメモリ領域を開放する
 *
 * @param   tmp   
 */
//----------------------------------------------------------------------------------
static void MailSys_ReleaseTmpData(MAIL_TMP_DATA* tmp)
{
  if(tmp->name != NULL){
    GFL_STR_DeleteBuffer(tmp->name);
  }
  GFL_HEAP_FreeMemory(tmp);
}


//----------------------------------------------------------------------------------
/**
 * @brief メールデータ　テンポラリデータ書き出し
 *
 * @param   org   入力済みメールデータ
 * @param   tmp   書き出し先
 */
//----------------------------------------------------------------------------------
static void MailSys_SetTmpData( MAIL_DATA* org, MAIL_TMP_DATA* tmp )
{
  u16 i;

  //簡易文コピー
  for(i = 0;i < MAILDAT_MSGMAX;i++){
    MailData_SetMsgByIndex(org,&tmp->msg[i],i);
  }

  //データを有効化するためにデザインNOを設定
  MailData_SetDesignNo(org,tmp->design);
}

