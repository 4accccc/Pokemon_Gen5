//=============================================================================
/**
 *
 *	@file		fes_rom.c
 *	@brief  フェスミッション配信アプリケーション システムプロセス 
 *	@author	Miyuki Iwasawa
 *	@data		2012.03.29
 *
 */
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

// 簡易会話表示システム
#include "system/pms_draw.h"

//FONT
#include "print/gf_font.h"
#include "font/font.naix"

//PRINT_QUE
#include "print/printsys.h"

#include "print/wordset.h"

//INFOWIN
#include "infowin/infowin.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

//アプリ共通素材
#include "app/app_menu_common.h"
#include "app/name_input.h"

//アーカイブ
#include "arc_def.h"

//外部公開
#include "test/fes_rom_sys.h"
#include "fes_rom.h"

#include "message.naix"
#include "msg/msg_beacon_detail.h"  // GMM
#include "config_gra.naix"		// タッチバーカスタムボタン用サンプルにタウンマップリソース

//=============================================================================
// 下記defineをコメントアウトすると、機能を取り除けます
//=============================================================================

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

#define FES_ROM_SYS_HEAP_SIZE  (0x10000)

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================


//=============================================================================
/**
 *							データ定義
 */
//=============================================================================
enum{
 SEQ_SYS_FES_CALL,
 SEQ_SYS_FES_WAIT,
 SEQ_SYS_NAME_IN,
 SEQ_SYS_FADEIN,
 SEQ_SYS_FADEOUT,
 SEQ_SYS_EXIT,
};


//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
//-------------------------------------
///	PROC
//=====================================
static GFL_PROC_RESULT FesRomSystemProcInit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT FesRomSystemProcMain( GFL_PROC *proc, int *seq, void *pwk, void *mywk );
static GFL_PROC_RESULT FesRomSystemProcExit( GFL_PROC *proc, int *seq, void *pwk, void *mywk );


static int seq_FadeIn( FES_ROM_SYS_WORK* wk );
static int seq_FadeOut( FES_ROM_SYS_WORK* wk );
static int seq_FesCall( FES_ROM_SYS_WORK* wk );
static int seq_FesWait( FES_ROM_SYS_WORK* wk );
static int seq_NameIn( FES_ROM_SYS_WORK* wk );

//-------------------------------------
///	汎用処理ユーティリティ
//=====================================
static void _sub_DataSetup(FES_ROM_SYS_WORK* wk);
static void _sub_DataExit(FES_ROM_SYS_WORK* wk);
static void _sub_SystemSetup( FES_ROM_SYS_WORK* wk);
static void _sub_SystemExit( FES_ROM_SYS_WORK* wk);

static BOOL _sub_LocalProcWait( GFL_PROCSYS* localProc );

//=============================================================================
/**
 *								外部公開
 */
//=============================================================================
const GFL_PROC_DATA FesRomSysProcData = 
{
	FesRomSystemProcInit,
	FesRomSystemProcMain,
	FesRomSystemProcExit,
};
//=============================================================================
/**
 *								PROC
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 初期化処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					FES_ROM_SYSTEM_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT FesRomSystemProcInit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{
	FES_ROM_SYS_WORK *wk;

	//ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_FES_ROM_SYS, FES_ROM_SYS_HEAP_SIZE );
  wk = GFL_PROC_AllocWork( proc, sizeof(FES_ROM_SYS_WORK), HEAPID_FES_ROM_SYS );
  GFL_STD_MemClear( wk, sizeof(FES_ROM_SYS_WORK) );
	
  //引数取得
//	wk->param	= pwk;

  // 初期化
  wk->heapID = HEAPID_FES_ROM_SYS;
  wk->tmpHeapID = GFL_HEAP_LOWID( HEAPID_FES_ROM_SYS );
	
  _sub_SystemSetup( wk );
  _sub_DataSetup( wk );
  
  //とりあえずスリープ禁止
  GFL_UI_SleepDisable(0xFF);

  return GFL_PROC_RES_FINISH;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 終了処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					FES_ROM_SYSTEM_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT FesRomSystemProcExit( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	FES_ROM_SYS_WORK* wk = mywk;

  _sub_DataExit( wk );
  _sub_SystemExit( wk );

	//PROC用メモリ解放
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_FES_ROM_SYS );

  return GFL_PROC_RES_FINISH;
}
//-----------------------------------------------------------------------------
/**
 *	@brief  PROC 主処理
 *
 *	@param	GFL_PROC *procプロセスシステム
 *	@param	*seq					シーケンス
 *	@param	*pwk					FES_ROM_SYSTEM_PARAM
 *	@param	*mywk					PROCワーク
 *
 *	@retval	終了コード
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT FesRomSystemProcMain( GFL_PROC *proc, int *seq, void *pwk, void *mywk )
{ 
	FES_ROM_SYS_WORK* wk = mywk;

  switch(*seq){
  case SEQ_SYS_FES_CALL:
    *seq = seq_FesCall( wk );
    break;
  case SEQ_SYS_FES_WAIT:
    *seq = seq_FesWait( wk );
    break;
  case SEQ_SYS_NAME_IN:
    *seq = seq_NameIn( wk );
    break;
  case SEQ_SYS_FADEOUT:
    *seq = seq_FadeOut( wk );
    break;
  case SEQ_SYS_EXIT:
    return GFL_PROC_RES_FINISH;
  }
  GFL_TCBL_Main( wk->pTcbSys );

  return GFL_PROC_RES_CONTINUE;
}
//=============================================================================
/**
 *								static関数
 */
//=============================================================================

/*
 *  @brief  フェス配信画面プロセスコール
 */
static int seq_FesCall( FES_ROM_SYS_WORK* wk )
{
  FES_ROM_PARAM* param;

  wk->proc_wk = GFL_HEAP_AllocClearMemory( wk->heapID, sizeof(FES_ROM_PARAM) );
  param = (FES_ROM_PARAM*)wk->proc_wk;

  //呼び出しパラメータ設定
  param->sys = wk;
  param->mode = FESROM_APP_MODE_BASE;

  GFL_PROC_LOCAL_CallProc( wk->localProc, NULL, &FesRomProcData, wk->proc_wk );

  wk->sub_seq = 0;
  return SEQ_SYS_FES_WAIT;
}

/*
 *  @brief  フェス配信画面プロセス待ち
 */
static int seq_FesWait( FES_ROM_SYS_WORK* wk )
{
  FES_ROM_PARAM* param = (FES_ROM_PARAM*)wk->proc_wk;

  if( _sub_LocalProcWait( wk->localProc ) == FALSE ){
    return SEQ_SYS_FES_WAIT;
  }
  GFL_HEAP_FreeMemory( wk->proc_wk );
  wk->proc_wk = NULL;

  switch( wk->ret_mode ){
  case FESROM_APP_RET_EDIT_NAME:
  case FESROM_APP_RET_EDIT_SELFMSG:
    return SEQ_SYS_NAME_IN;
  }
  return SEQ_SYS_FES_CALL;
}

/*
 *  @brief  フェス配信名前入力画面プロセスコール
 */
static int seq_NameIn( FES_ROM_SYS_WORK* wk )
{
  switch( wk->seq ){
  case 0:
    {
      MISC *p_misc     = SaveData_GetMisc( SaveControl_GetPointer() );
      int msg_len, in_mode;
      
      if( wk->ret_mode == FESROM_APP_RET_EDIT_NAME ){
        msg_len = NAMEIN_PERSON_LENGTH;
        in_mode = NAMEIN_MYNAME;
        GFL_STR_CopyBuffer( wk->str_tmp, wk->save_wk.str_name );
      }else{
        msg_len = GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN;
        in_mode = NAMEIN_GREETING_WORD;
        GFL_STR_CopyBuffer( wk->str_tmp, wk->save_wk.str_selfmsg );
      }
      wk->proc_wk	= NAMEIN_AllocParam( wk->heapID, in_mode,
   																     NAMEIN_TRAINER_VIEW_UNKNOWN, 0,
   																     msg_len, wk->str_tmp, p_misc );

      GFL_PROC_LOCAL_CallProc( wk->localProc, FS_OVERLAY_ID(namein), &NameInputProcData, wk->proc_wk );
    }
    wk->seq++;
    break;
  case 1:
    if( _sub_LocalProcWait( wk->localProc ) == FALSE ){
      break;
    }
    {
      NAMEIN_PARAM* param = (NAMEIN_PARAM*)wk->proc_wk;
      if( param->cancel == FALSE ){
        if( wk->ret_mode == FESROM_APP_RET_EDIT_NAME ){
          NAMEIN_CopyStr( param, wk->save_wk.str_name );
        }else{
          NAMEIN_CopyStr( param, wk->save_wk.str_selfmsg );
        }
      }
      NAMEIN_FreeParam(wk->proc_wk);
    }
    wk->seq = 0;
    return SEQ_SYS_FES_CALL;
  }
  return SEQ_SYS_NAME_IN;
}

/*
 *  @brief  フェードイン
 */
static int seq_FadeIn( FES_ROM_SYS_WORK* wk )
{
  switch( wk->seq ){
  case 0:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0 );
    wk->seq++;
    break;
  case 1:
    if( !GFL_FADE_CheckFade() ){	
      wk->seq = 0;
			return SEQ_SYS_FES_CALL;
		}
    break;
  }
  return SEQ_SYS_FADEIN; 
}

/*
 *  @brief  フェードアウト
 */
static int seq_FadeOut( FES_ROM_SYS_WORK* wk )
{
  switch( wk->seq ){
  case 0:
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0 );
    wk->seq++;
    break;
  case 1:
    if( !GFL_FADE_CheckFade() ){	
      wk->seq = 0;
      return SEQ_SYS_EXIT; 
		}
    break;
  }
  return SEQ_SYS_FADEOUT; 
}

////////////////////////////////////////////////////////////////////////////////////
/***********************************************************************************
 *
 ***********************************************************************************/
////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------
/**
 * データ関連のセットアップ
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_DataSetup( FES_ROM_SYS_WORK* wk )
{
  wk->save_enable_f = FesRomSys_SaveLoad( wk );
}

//--------------------------------------------------------------
/**
 * データ関連の破棄＆終了処理
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_DataExit( FES_ROM_SYS_WORK* wk )
{
}

//--------------------------------------------------------------
/**
 * システム関連のセットアップ
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_SystemSetup( FES_ROM_SYS_WORK* wk)
{
  wk->localProc = GFL_PROC_LOCAL_boot( wk->heapID );
  
  wk->pTcbSys = GFL_TCBL_Init( wk->heapID, wk->heapID, 64, 128 );

  GFL_FONTSYS_SetDefaultColor();

  MI_CpuClear8( &wk->save_wk, sizeof(FES_ROM_SAVE_WK) );
  wk->save_wk.str_name = GFL_STR_CreateBuffer( BUFLEN_PERSON_NAME, wk->heapID );
  wk->save_wk.str_selfmsg = GFL_STR_CreateBuffer( BUFLEN_FESROM_SELFMSG, wk->heapID );
  
  wk->str_tmp = GFL_STR_CreateBuffer( BUFLEN_FESROM_SELFMSG, wk->heapID );
}

//--------------------------------------------------------------
/**
 * システム関連の破棄
 *
 * @param   wk		
 */
//--------------------------------------------------------------
static void _sub_SystemExit( FES_ROM_SYS_WORK* wk)
{
  GFL_STR_DeleteBuffer(wk->str_tmp);

  GFL_STR_DeleteBuffer(wk->save_wk.str_selfmsg);
  GFL_STR_DeleteBuffer(wk->save_wk.str_name);

  GFL_FONTSYS_SetDefaultColor();
 
  GFL_TCBL_Exit( wk->pTcbSys );

  GFL_PROC_LOCAL_Exit( wk->localProc );
}

//------------------------------------------------------------------------------------------------------
// LOCAL PROC 終了待ち
//------------------------------------------------------------------------------------------------------
static BOOL _sub_LocalProcWait( GFL_PROCSYS* localProc )
{
  if( GFL_PROC_LOCAL_Main(localProc) != GFL_PROC_MAIN_VALID ){
    return TRUE;
  }
  return FALSE;
}

//==================================================================
///セーブ関連
//==================================================================

/*
 *  @brief  セーブワーク初期化
 */

//==================================================================
/**
 * 現在の設定でセーブを実行する
 *
 * @param   control		
 */
//==================================================================
void FesRomSys_SaveWrite( FES_ROM_SYS_WORK* wk )
{
  //編集用データをコピー
  MI_CpuCopy8( &wk->save_wk.save, &wk->save, sizeof(FES_ROM_SAVE));
  GFL_STR_GetStringCode( wk->save_wk.str_name, wk->save.name, SAVELEN_PLAYER_NAME + EOM_SIZE);
  GFL_STR_GetStringCode( wk->save_wk.str_selfmsg,  wk->save.selfmsg, GAMEBEACON_SELFINTRODUCTION_MESSAGE_LEN + EOM_SIZE);
  //マジックキー設定
  wk->save.magic_key = FES_ROM_SAVE_MAGICKEY;
#if 0
  save_data.union_index = control->union_index;
  save_data.powergroup = control->powergroup;
  save_data.beacon_space_time = control->powergroup.beacon_space_time;
#endif
  
  GFL_BACKUP_DirectFlashSave(0, &wk->save, sizeof(FES_ROM_SAVE));
  PMSND_PlaySE(SEQ_SE_SAVE);
}

//==================================================================
/**
 * セーブデータがあればロードを行う
 *
 * @param   control		
 *
 * @retval  BOOL		TRUE:ロード成功　FALSE：データ無し
 */
//==================================================================
BOOL FesRomSys_SaveLoad(  FES_ROM_SYS_WORK* wk )
{
  BOOL ret = FALSE;
 
  wk->data_enable_f = FALSE;
  wk->save_enable_f = FALSE;

  ret = GFL_BACKUP_DirectFlashLoad(0, &wk->save, sizeof(FES_ROM_SAVE));

  if( GFL_UI_KEY_GetCont() == (PAD_BUTTON_START|PAD_BUTTON_SELECT|PAD_BUTTON_Y)){
    if( ret ) wk->save.magic_key = 0; //セーブ消去
  }
  if(ret == FALSE || wk->save.magic_key != FES_ROM_SAVE_MAGICKEY){
    FesRomSys_SaveInit( &wk->save );
    ret = FALSE;
  }

  //編集用データをコピー
  MI_CpuCopy8( &wk->save, &wk->save_wk.save, sizeof(FES_ROM_SAVE));
  GFL_STR_SetStringCode( wk->save_wk.str_name, wk->save.name );
  GFL_STR_SetStringCode( wk->save_wk.str_selfmsg, wk->save.selfmsg );

  {
    //有効なミッションデータがあるか？
    int i;
    for(i = 0;i < FESROM_MISSION_DATA_MAX;i++){
      if( wk->save.mission[i].send.id != 0 ){
        wk->data_enable_f = TRUE;
        break;
      }
    }
    if( !wk->data_enable_f) ret = FALSE;
  }
#if 0
  GFL_STR_SetStringCode( control->name, load_data.name );
  GFL_STR_SetStringCode( control->selfmsg, load_data.selfmsg );
  control->union_index = load_data.union_index;
  control->powergroup = load_data.powergroup;
  control->powergroup.beacon_space_time = load_data.beacon_space_time;
  
  {
    int i,power_count=0;
    for(i = 0; i < HAISIN_POWER_MAX; i++){
      if(control->powergroup.hp[i].g_power_id < GPOWER_ID_MAX){
        power_count++;
      }
    }
    if(power_count != 0){
      control->powergroup.data_num = power_count;
    }
  }
#endif
  return ret;
}

//==================================================================
/**
 * セーブデータ初期化
 *
 * @param   control		
 */
//==================================================================
void FesRomSys_SaveInit( FES_ROM_SAVE* sv )
{
  int i;

  static STRCODE name[] = L"ポケフェス";
  static STRCODE selfmsg[] = L"こんにちは";

  MI_CpuClear8( sv, sizeof( FES_ROM_SAVE ) );

  //名前
  MI_CpuCopy16( name, sv->name, sizeof(STRCODE)*NELEMS(name));
  sv->name[NELEMS(name)-1] = GFL_STR_GetEOMCode();
  //あいさつ
  MI_CpuCopy16( selfmsg, sv->selfmsg, sizeof(STRCODE)*NELEMS(selfmsg));
  sv->selfmsg[NELEMS(selfmsg)-1] = GFL_STR_GetEOMCode();

  sv->beacon_interval = 3;  //3秒
  sv->start_time = FES_ROM_TIME_UNIT_HOUR*8;  //8時～
  sv->end_time = FES_ROM_TIME_UNIT_HOUR*20;   //～20時

  for( i = 0; i < FESROM_MISSION_DATA_MAX; i++){
    FesRomSys_MissionSaveInit( sv, i );
  }
}

void FesRomSys_MissionSaveInit( FES_ROM_SAVE* sv, int idx )
{
  MI_CpuClear8(&sv->mission[idx],sizeof(FESROM_MISSION));
  sv->mission[idx].send.id = 0;
  sv->mission[idx].send.time = 1;
  sv->mission[idx].send.blank = 1;
  sv->mission[idx].send.odds = 0;
}



