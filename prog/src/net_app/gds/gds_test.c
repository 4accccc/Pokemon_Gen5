//==============================================================================
/**
 * @file  gds_test.c
 * @brief GDSライブラリテスト用ソース
 * @author  matsuda
 * @date  2008.01.09(水)
 */
//==============================================================================
#include <gflib.h>
#include "system/main.h"
#include "gds_rap.h"
#include "gds_rap_response.h"
#include "savedata/musical_save.h"
#include "musical\musical_define.h"
#include "gamesystem/game_data.h"
#include "musical/musical_debug.h"
#include "gds_test.h"


#ifdef PM_DEBUG //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//==============================================================================
//  定数定義
//==============================================================================
#define HEAPID_GDS_TEST     (GFL_HEAPID_APP)


//==============================================================================
//  構造体定義
//==============================================================================
typedef struct{
//  IMC_TELEVISION_SAVEDATA* dummy_imc;
//  POKEMON_PARAM *pp;
  int monsno;
  int count;
}MUSICAL_ALL_UP;

typedef struct _GDS_TEST_SYS{
  //システム系
  SAVE_CONTROL_WORK *sv;

  //通信パラメータ
  GDS_PROFILE_PTR gds_profile_ptr;

  //通信
  GDS_RAP_WORK gdsrap;  ///<GDSライブラリ、NitroDWC関連のワーク構造体

  //ローカルワーク
  int seq;

  //受信データ
  u64 data_number;

  MUSICAL_SHOT_DATA musshot;
  
  //ミュージカル全アップ用ワーク
  MUSICAL_ALL_UP dau;
}GDS_TEST_SYS;

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
GFL_PROC_RESULT GdsTestProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
GFL_PROC_RESULT GdsTestProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
GFL_PROC_RESULT GdsTestMusicalAllProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
GFL_PROC_RESULT GdsTestProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static int GdsTest_Main(GDS_TEST_SYS *testsys);
static int GdsTest_DressUpload(GDS_TEST_SYS *testsys);
static void Response_MusicalRegist(void *work, const GDS_RAP_ERROR_INFO *error_info);
static void Response_MusicalGet(void *work, const GDS_RAP_ERROR_INFO *error_info);
static void Response_BattleVideoRegist(void *work, const GDS_RAP_ERROR_INFO *error_info);
static void Response_BattleVideoSearch(void *work, const GDS_RAP_ERROR_INFO *error_info);
static void Response_BattleVideoDataGet(void *work, const GDS_RAP_ERROR_INFO *error_info);
static void Response_BattleVideoFavorite(void *work, const GDS_RAP_ERROR_INFO *error_info);


//==============================================================================
//  データ
//==============================================================================
///GDSテストプロセス定義データ
const GFL_PROC_DATA GdsTestProcData = {
	GdsTestProc_Init,
	GdsTestProc_Main,
	GdsTestProc_End,
};

///GDSミュージカルショット全送信プロセス定義データ
const GFL_PROC_DATA GdsTestMusicalAllProcData = {
	GdsTestProc_Init,
	GdsTestMusicalAllProc_Main,
	GdsTestProc_End,
};

//--------------------------------------------------------------
//  ミュージカルデータ
//--------------------------------------------------------------
#include "musical/musical_entry.cdat"



//--------------------------------------------------------------
/**
 * @brief   プロセス関数：初期化
 *
 * @param   proc    プロセスデータ
 * @param   seq     シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
GFL_PROC_RESULT GdsTestProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GDS_TEST_PARENT *gtp = pwk;
  GDS_TEST_SYS *testsys;
  
  testsys = GFL_PROC_AllocWork(proc, sizeof(GDS_TEST_SYS), HEAPID_GDS_TEST );
  GFL_STD_MemClear(testsys, sizeof(GDS_TEST_SYS));
  
  testsys->sv = GAMEDATA_GetSaveControlWork(gtp->gamedata);
  
  testsys->gds_profile_ptr = GDS_Profile_AllocMemory(HEAPID_GDS_TEST);
  GDS_Profile_MyDataSet(testsys->gds_profile_ptr, gtp->gamedata);

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：メイン
 *
 * @param   proc    プロセスデータ
 * @param   seq     シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
GFL_PROC_RESULT GdsTestProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GDS_TEST_SYS *testsys = mywk;
  GDS_TEST_PARENT *gtp = pwk;
  int ret;
  enum{
    SEQ_COMM_INIT,
    SEQ_COMM_MAIN,
    SEQ_COMM_END,

    SEQ_END,
  };

  switch( *seq ){
  case SEQ_COMM_INIT:
    {
      GDSRAP_INIT_DATA init_data;

      init_data.gs_profile_id = MyStatus_GetProfileID(GAMEDATA_GetMyStatus(gtp->gamedata));
      init_data.heap_id = HEAPID_GDS_TEST;
      init_data.gamedata = gtp->gamedata;
      init_data.pSvl = gtp->pSvl;

      init_data.response_callback.callback_work = testsys;
      init_data.response_callback.func_musicalshot_regist = Response_MusicalRegist;
      init_data.response_callback.func_musicalshot_get = Response_MusicalGet;
      init_data.response_callback.func_battle_video_regist = Response_BattleVideoRegist;
      init_data.response_callback.func_battle_video_search_get = Response_BattleVideoSearch;
      init_data.response_callback.func_battle_video_data_get = Response_BattleVideoDataGet;
      init_data.response_callback.func_battle_video_favorite_regist = Response_BattleVideoFavorite;

      GDSRAP_Init(&testsys->gdsrap, &init_data);  //通信ライブラリ初期化
    }
    (*seq)++;
    break;

  case SEQ_COMM_MAIN:
    if(GdsTest_Main(testsys) == FALSE){
      (*seq)++;
    }
    break;
  case SEQ_COMM_END:
    GDSRAP_Exit(&testsys->gdsrap);
    (*seq)++;
    break;

  case SEQ_END:
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：メイン ※ミュージカル全ポケモンアップロード
 *
 * @param   proc    プロセスデータ
 * @param   seq     シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
GFL_PROC_RESULT GdsTestMusicalAllProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GDS_TEST_SYS * testsys  = mywk;
  GDS_TEST_PARENT *gtp = pwk;
  int ret;
  enum{
    SEQ_COMM_INIT,
    SEQ_COMM_MAIN,
    SEQ_COMM_END,

    SEQ_END,
  };

  switch( *seq ){
  case SEQ_COMM_INIT:
    {
      GDSRAP_INIT_DATA init_data;

      init_data.gs_profile_id = MyStatus_GetProfileID(GAMEDATA_GetMyStatus(gtp->gamedata));
      init_data.heap_id = HEAPID_GDS_TEST;
      init_data.gamedata = gtp->gamedata;
      init_data.pSvl = gtp->pSvl;

      init_data.response_callback.callback_work = testsys;
      init_data.response_callback.func_musicalshot_regist = Response_MusicalRegist;
      init_data.response_callback.func_musicalshot_get = Response_MusicalGet;
      init_data.response_callback.func_battle_video_regist = Response_BattleVideoRegist;
      init_data.response_callback.func_battle_video_search_get = Response_BattleVideoSearch;
      init_data.response_callback.func_battle_video_data_get = Response_BattleVideoDataGet;
      init_data.response_callback.func_battle_video_favorite_regist = Response_BattleVideoFavorite;

      GDSRAP_Init(&testsys->gdsrap, &init_data);  //通信ライブラリ初期化
    }
    (*seq)++;
    break;

  case SEQ_COMM_MAIN:
    if(GdsTest_DressUpload(testsys) == FALSE){
      (*seq)++;
    }
    break;
  case SEQ_COMM_END:
    GDSRAP_Exit(&testsys->gdsrap);
    (*seq)++;
    break;

  case SEQ_END:
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   プロセス関数：終了
 *
 * @param   proc    プロセスデータ
 * @param   seq     シーケンス
 *
 * @retval  処理状況
 */
//--------------------------------------------------------------
GFL_PROC_RESULT GdsTestProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GDS_TEST_SYS * testsys = mywk;
  int i;

  GDS_Profile_FreeMemory(testsys->gds_profile_ptr);
  GFL_PROC_FreeWork(proc);        // ワーク開放

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
/**
 * @brief   メイン
 *
 * @param   testsys
 *
 * @retval
 *
 *
 */
//--------------------------------------------------------------
static int GdsTest_Main(GDS_TEST_SYS *testsys)
{
  GDS_RAP_ERROR_INFO *error_info;
  enum{
    SEQ_WIFI_CONNECT,

    SEQ_MUSICAL_UPLOAD,
    SEQ_MUSICAL_UPLOAD_ERROR_CHECK,

    SEQ_MUSICAL_DOWNLOAD,
    SEQ_MUSICAL_DOWNLOAD_ERROR_CHECK,

    SEQ_VIDEO_UPLOAD,
    SEQ_VIDEO_UPLOAD_ERROR_CHECK,

    SEQ_VIDEO_SEARCH,
    SEQ_VIDEO_SEARCH_ERROR_CHECK,

    SEQ_VIDEO_DATA_GET,
    SEQ_VIDEO_DATA_GET_ERROR_CHECK,

    SEQ_VIDEO_FAVORITE,
    SEQ_VIDEO_FAVORITE_ERROR_CHECK,

    SEQ_WIFI_CLEANUP,

    SEQ_EXIT,
  };

  if(GDSRAP_MoveStatusAllCheck(&testsys->gdsrap) == TRUE){
    switch(testsys->seq){
    case SEQ_WIFI_CONNECT:  //WIFI接続
      //GDSRAP_ProccessReq(&testsys->gdsrap, GDSRAP_PROCESS_REQ_INTERNET_CONNECT);
      testsys->seq++;
      break;

    case SEQ_MUSICAL_UPLOAD:  //ミュージカル送信
      {
        MUSICAL_DEBUG_CreateDummyData( &testsys->musshot, MUSICAL_ENTRY_ARR[0], HEAPID_GDS_TEST );
        if(GDSRAP_Tool_Send_MusicalShotUpload(
            &testsys->gdsrap, testsys->gds_profile_ptr, &testsys->musshot) == TRUE){
          OS_TPrintf("自分のミュージカルショット送信\n");
          testsys->seq++;
        }
      }
      break;
    case SEQ_MUSICAL_UPLOAD_ERROR_CHECK:
      if(GDSRAP_ErrorInfoGet(&testsys->gdsrap, &error_info) == TRUE){
        //エラー発生時の処理
        //エラーメッセージの表示等はコールバックで行われるので、
        //ここでは表示後の処理。(アプリ終了とか特定のメニューに戻すとか)
        OS_TPrintf("ミュージカルショット送信：エラー発生\n");
        testsys->seq++;
      }
      else{
        OS_TPrintf("ミュージカルショット送信：成功\n");
        testsys->seq++;
      }
      break;

    case SEQ_MUSICAL_DOWNLOAD:  //ミュージカルダウンロード
      if(GDSRAP_Tool_Send_MusicalShotDownload(&testsys->gdsrap, MUSICAL_ENTRY_ARR[0]) == TRUE){
        OS_TPrintf("ミュージカルショットダウンロードリクエスト\n");
        testsys->seq++;
      }
      break;
    case SEQ_MUSICAL_DOWNLOAD_ERROR_CHECK:
      if(GDSRAP_ErrorInfoGet(&testsys->gdsrap, &error_info) == TRUE){
        //エラー発生時の処理
        //エラーメッセージの表示等はコールバックで行われるので、
        //ここでは表示後の処理。(アプリ終了とか特定のメニューに戻すとか)
        OS_TPrintf("ミュージカルショットダウンロードリクエスト：エラー発生\n");
        testsys->seq++;
      }
      else{
        OS_TPrintf("ミュージカルショットダウンロードリクエスト：成功\n");
        testsys->seq++;
      }
      break;

    case SEQ_VIDEO_UPLOAD:  //
      {
        LOAD_RESULT result;

        BattleRec_Load(testsys->sv, HEAPID_GDS_TEST, &result, LOADDATA_MYREC);

        if(GDSRAP_Tool_Send_BattleVideoUpload(
            &testsys->gdsrap, testsys->gds_profile_ptr) == TRUE){
          OS_TPrintf("自分のバトルビデオをアップロード\n");
          testsys->seq++;
        }
      }
      break;
    case SEQ_VIDEO_UPLOAD_ERROR_CHECK:
      BattleRec_Exit();
      if(GDSRAP_ErrorInfoGet(&testsys->gdsrap, &error_info) == TRUE){
        //エラー発生時の処理
        //エラーメッセージの表示等はコールバックで行われるので、
        //ここでは表示後の処理。(アプリ終了とか特定のメニューに戻すとか)
        OS_TPrintf("自分のバトルビデオをアップロード：エラー発生\n");
        testsys->seq++;
      }
      else{
        OS_TPrintf("自分のバトルビデオをアップロード：成功\n");
        testsys->seq++;
      }
      break;

    case SEQ_VIDEO_SEARCH:  //
      //最新30件
      if(GDSRAP_Tool_Send_BattleVideoNewDownload(&testsys->gdsrap) == TRUE){
        OS_TPrintf("最新30件要求\n");
        testsys->seq++;
      }
      break;
    case SEQ_VIDEO_SEARCH_ERROR_CHECK:
      if(GDSRAP_ErrorInfoGet(&testsys->gdsrap, &error_info) == TRUE){
        //エラー発生時の処理
        //エラーメッセージの表示等はコールバックで行われるので、
        //ここでは表示後の処理。(アプリ終了とか特定のメニューに戻すとか)
        OS_TPrintf("最新30件要求：エラー発生\n");
        testsys->seq++;
      }
      else{
        OS_TPrintf("最新30件要求：成功\n");
        testsys->seq++;
      }
      break;

    case SEQ_VIDEO_DATA_GET:  //
      if(GDSRAP_Tool_Send_BattleVideo_DataDownload(
          &testsys->gdsrap, testsys->data_number) == TRUE){
        OS_TPrintf("データナンバー指定でダウンロード dataNumber=%d\n", testsys->data_number);
        testsys->seq++;
      }
      break;
    case SEQ_VIDEO_DATA_GET_ERROR_CHECK:
      if(GDSRAP_ErrorInfoGet(&testsys->gdsrap, &error_info) == TRUE){
        //エラー発生時の処理
        //エラーメッセージの表示等はコールバックで行われるので、
        //ここでは表示後の処理。(アプリ終了とか特定のメニューに戻すとか)
        OS_TPrintf("データナンバー指定でダウンロード：エラー発生\n");
        testsys->seq++;
      }
      else{
        OS_TPrintf("データナンバー指定でダウンロード：成功\n");
        testsys->seq++;
      }
      break;

    case SEQ_VIDEO_FAVORITE:  //
      if(GDSRAP_Tool_Send_BattleVideo_DataDownload(
          &testsys->gdsrap, testsys->data_number) == TRUE){
        OS_TPrintf("データナンバー指定でお気に入り送信\n");
        testsys->seq++;
      }
      break;
    case SEQ_VIDEO_FAVORITE_ERROR_CHECK:
      if(GDSRAP_ErrorInfoGet(&testsys->gdsrap, &error_info) == TRUE){
        //エラー発生時の処理
        //エラーメッセージの表示等はコールバックで行われるので、
        //ここでは表示後の処理。(アプリ終了とか特定のメニューに戻すとか)
        OS_TPrintf("データナンバー指定でお気に入り送信：エラー発生\n");
        testsys->seq++;
      }
      else{
        OS_TPrintf("データナンバー指定でお気に入り送信：成功\n");
        testsys->seq++;
      }
      break;

    case SEQ_WIFI_CLEANUP:  //WIFI切断
      //GDSRAP_ProccessReq(&testsys->gdsrap, GDSRAP_PROCESS_REQ_INTERNET_CLEANUP);
      testsys->seq++;
      break;

    case SEQ_EXIT:
      return FALSE;
    }
  }

  GDSRAP_Main(&testsys->gdsrap);
  return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   メイン(ミュージカルに全ポケモンアップロード
 *
 * @param   testsys
 *
 * @retval
 *
 *
 */
//--------------------------------------------------------------
static int GdsTest_DressUpload(GDS_TEST_SYS *testsys)
{
#if 0
  MUSICAL_ALL_UP *dau = &testsys->dau;
  GDS_RAP_ERROR_INFO *error_info;
  enum{
    SEQ_INIT,

    SEQ_POKE_CREATE,
    SEQ_MUSICAL_UPLOAD,
    SEQ_MUSICAL_UPLOAD_ERROR_CHECK,

    SEQ_EXIT,
    SEQ_TRG_WAIT,
  };
  // 文字列取得
  STRBUF *tempbuf, *destbuf;

  if(GDSRAP_MoveStatusAllCheck(&testsys->gdsrap) == TRUE){
    switch(testsys->seq){
    case SEQ_INIT:
      dau->monsno = 1;
      dau->pp = PokemonParam_AllocWork(testsys->heap_id);
      dau->dummy_imc = ImcSaveData_TelevisionAllocWork(testsys->heap_id);
      testsys->seq++;
      //break;

    case SEQ_POKE_CREATE:
      PokeParaSetPowRnd(dau->pp, dau->monsno, 20, 1234, gf_rand());//9876);
      PokeParaPut(dau->pp, ID_PARA_oyaname, MyStatus_GetMyName(SaveData_GetMyStatus(testsys->sv)));
      Debug_ImcSaveData_MakeTelevisionDummyData(dau->dummy_imc, dau->pp, 5);
      testsys->seq++;
      //break;

    case SEQ_MUSICAL_UPLOAD:  //ミュージカル送信
      if(GDSRAP_Tool_Send_MusicalUpload(&testsys->gdsrap, testsys->gds_profile_ptr,
          dau->dummy_imc) == TRUE){
        DEBUG_GDSRAP_SaveFlagReset(&testsys->gdsrap);
        OS_TPrintf("monsno = %d番 送信中 %d件目\n", dau->monsno, dau->count+1);
        // ウインドウ枠描画＆メッセージ領域クリア
        GF_BGL_BmpWinDataFill(&testsys->win[TEST_BMPWIN_TITLE], 15 );
        // 文字列描画開始
        WORDSET_RegisterPokeMonsName(testsys->wordset, 0, PPPPointerGet(dau->pp));
        WORDSET_RegisterNumber(testsys->wordset, 1, dau->count+1, 1,
          NUMBER_DISPTYPE_LEFT, NUMBER_CODETYPE_DEFAULT);
        destbuf = STRBUF_Create(256, testsys->heap_id);
        tempbuf = MSGMAN_AllocString(testsys->msgman, DMMSG_GDS_MUSICAL_UPLOAD);
        WORDSET_ExpandStr( testsys->wordset, destbuf, tempbuf);
        GF_STR_PrintSimple( &testsys->win[TEST_BMPWIN_TITLE],
          FONT_TALK, destbuf, 0, 0, MSG_ALLPUT, NULL);
        STRBUF_Delete(destbuf);
        STRBUF_Delete(tempbuf);
        testsys->seq++;
      }
      break;
    case SEQ_MUSICAL_UPLOAD_ERROR_CHECK:
      if(GDSRAP_ErrorInfoGet(&testsys->gdsrap, &error_info) == TRUE){
        //エラー発生時の処理
        //エラーメッセージの表示等はコールバックで行われるので、
        //ここでは表示後の処理。(アプリ終了とか特定のメニューに戻すとか)
        OS_TPrintf("エラーが発生しました\n");
        tempbuf = MSGMAN_AllocString(testsys->msgman, DMMSG_GDS_MUSICAL_UPLOAD_ERROR);
        GF_STR_PrintSimple( &testsys->win[TEST_BMPWIN_TITLE],
          FONT_TALK, tempbuf, 0, 0, MSG_ALLPUT, NULL);
        STRBUF_Delete(tempbuf);
        testsys->seq = SEQ_TRG_WAIT;
      }
      else{
        dau->count++;
        if(dau->count >= BR_MUSICAL_VIEW_MAX){
          dau->monsno++;
          dau->count = 0;
          if(dau->monsno > MONSNO_END-1){ //-1=アルセウスを除く(不正扱いされるので)
            testsys->seq++;
          }
          else{
            testsys->seq = SEQ_POKE_CREATE;
          }
        }
        else{
          testsys->seq = SEQ_POKE_CREATE;
        }
      }
      break;

    case SEQ_EXIT:
      sys_FreeMemoryEz(dau->dummy_imc);
      sys_FreeMemoryEz(dau->pp);
      return FALSE;

    case SEQ_TRG_WAIT:
      if(sys.trg & PAD_BUTTON_A){
        testsys->seq = SEQ_EXIT;
      }
      break;
    }
  }

  GDSRAP_Main(&testsys->gdsrap);
#endif
  return TRUE;
}

//==============================================================================
//
//  通信データレスポンスコールバック関数
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   ミュージカルアップロード時のレスポンスコールバック
 *
 * @param   work
 * @param   error_info    エラー情報
 */
//--------------------------------------------------------------
static void Response_MusicalRegist(void *work, const GDS_RAP_ERROR_INFO *error_info)
{
  GDS_TEST_SYS *testsys = work;

  OS_TPrintf("ミュージカルショットのアップロードレスポンス取得\n");
  if(error_info->occ == TRUE){
    //TRUEならばエラー発生しているので、ここでメニューを戻すとかアプリ終了モードへ移行とかする
  }
  else{
    //正常時ならば受信バッファからデータ取得などを行う
    //アップロードの場合は特に必要なし
  }
}

//--------------------------------------------------------------
/**
 * @brief   ミュージカルダウンロード時のレスポンスコールバック
 *
 * @param   work
 * @param   error_info    エラー情報
 */
//--------------------------------------------------------------
static void Response_MusicalGet(void *work, const GDS_RAP_ERROR_INFO *error_info)
{
  GDS_TEST_SYS *testsys = work;

  OS_TPrintf("ミュージカルショットのダウンロードレスポンス取得\n");
  if(error_info->occ == TRUE){
    //TRUEならばエラー発生しているので、ここでメニューを戻すとかアプリ終了モードへ移行とかする
  }
  else{
    //正常時ならば受信バッファからデータ取得などを行う
    //アップロードの場合は特に必要なし

  //  GDS_RAP_RESPONSE_MusicalShot_Download_RecvPtr_Set(GDS_RAP_WORK *gdsrap, GDS_MUSICAL_RECV **dress_array, int array_max);
  }
}

//--------------------------------------------------------------
/**
 * @brief   バトルビデオ登録時のレスポンスコールバック
 *
 * @param   work
 * @param   error_info    エラー情報
 */
//--------------------------------------------------------------
static void Response_BattleVideoRegist(void *work, const GDS_RAP_ERROR_INFO *error_info)
{
  GDS_TEST_SYS *testsys = work;

  OS_TPrintf("バトルビデオ登録時のダウンロードレスポンス取得\n");
  if(error_info->occ == TRUE){
    //TRUEならばエラー発生しているので、ここでメニューを戻すとかアプリ終了モードへ移行とかする
    switch(error_info->result){
    case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_ERROR_AUTH:    //!< ユーザー認証エラー
      OS_TPrintf("バトルビデオデータ取得受信エラー！:ユーザー認証エラー\n");
      break;
    case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_ERROR_ILLEGALCODE: //!< コードエラー
      OS_TPrintf("バトルビデオデータ取得受信エラー！:コードエラー\n");
      break;
  	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGALPOKEMON_VERIFY: //!< ポケモン署名でエラー
  		OS_TPrintf("バトルビデオ登録受信エラー！:ポケモン署名でエラー\n");
  		break;
    case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_ERROR_UNKNOWN:   //!< その他エラー
    default:
      OS_TPrintf("バトルビデオデータ取得受信エラー！:その他のエラー\n");
      break;
    }
  }
  else{
    //正常時ならば受信バッファからデータ取得などを行う
    //アップロードの場合は特に必要なし
    u64 data_number;

    data_number = GDS_RAP_RESPONSE_BattleVideo_Upload_DataGet(&testsys->gdsrap);
    testsys->data_number = data_number;
    OS_TPrintf("登録コード＝%d\n", data_number);
  }
}

//--------------------------------------------------------------
/**
 * @brief   バトルビデオ検索時のレスポンスコールバック
 *
 * @param   work
 * @param   error_info    エラー情報
 */
//--------------------------------------------------------------
static void Response_BattleVideoSearch(void *work, const GDS_RAP_ERROR_INFO *error_info)
{
  GDS_TEST_SYS *testsys = work;

  OS_TPrintf("バトルビデオ検索のダウンロードレスポンス取得\n");
  if(error_info->occ == TRUE){
    //TRUEならばエラー発生しているので、ここでメニューを戻すとかアプリ終了モードへ移行とかする
  }
  else{
    //正常時ならば受信バッファからデータ取得などを行う
    //アップロードの場合は特に必要なし

  //  int GDS_RAP_RESPONSE_Boxshot_Download_RecvPtr_Set(GDS_RAP_WORK *gdsrap, BOX_SHOT_RECV **box_array, int array_max);
  }
}

//--------------------------------------------------------------
/**
 * @brief   バトルビデオデータダウンロード時のレスポンスコールバック
 *
 * @param   work
 * @param   error_info    エラー情報
 */
//--------------------------------------------------------------
static void Response_BattleVideoDataGet(void *work, const GDS_RAP_ERROR_INFO *error_info)
{
  GDS_TEST_SYS *testsys = work;

  OS_TPrintf("バトルビデオデータ取得のダウンロードレスポンス取得\n");
  if(error_info->occ == TRUE){
    //TRUEならばエラー発生しているので、ここでメニューを戻すとかアプリ終了モードへ移行とかする
  }
  else{
    //正常時ならば受信バッファからデータ取得などを行う
    //アップロードの場合は特に必要なし

  //  int GDS_RAP_RESPONSE_Boxshot_Download_RecvPtr_Set(GDS_RAP_WORK *gdsrap, BOX_SHOT_RECV **box_array, int array_max);
  }
}

//--------------------------------------------------------------
/**
 * @brief   バトルビデオお気に入り登録のレスポンスコールバック
 *
 * @param   work
 * @param   error_info    エラー情報
 */
//--------------------------------------------------------------
static void Response_BattleVideoFavorite(void *work, const GDS_RAP_ERROR_INFO *error_info)
{
  GDS_TEST_SYS *testsys = work;

  OS_TPrintf("バトルビデオお気に入り登録のダウンロードレスポンス取得\n");
  if(error_info->occ == TRUE){
    //TRUEならばエラー発生しているので、ここでメニューを戻すとかアプリ終了モードへ移行とかする
  }
  else{
    //正常時ならば受信バッファからデータ取得などを行う
    //アップロードの場合は特に必要なし

  //  int GDS_RAP_RESPONSE_Boxshot_Download_RecvPtr_Set(GDS_RAP_WORK *gdsrap, BOX_SHOT_RECV **box_array, int array_max);
  }
}

#endif //PM_DEBUG +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
