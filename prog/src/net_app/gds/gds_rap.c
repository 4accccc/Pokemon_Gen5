//==============================================================================
/**
 * @file	gds_rap.c
 * @brief	GDSライブラリをラッパーしたもの
 * @author	matsuda
 * @date	2008.01.09(水)
 */
//==============================================================================
#include <gflib.h>
#include <dwc.h>
#include "net/dwc_rap.h"
#include "savedata\save_control.h"

#include <arc_tool.h>
#include "print/wordset.h"
#include "message.naix"

#include "gds_rap.h"
#include "gds_rap_response.h"
#include "savedata/gds_profile.h"

#include "msg/msg_wifi_system.h"
#include "savedata/playtime.h"

#include "message.naix"
#include "msg/msg_battle_rec.h"
#include "arc_def.h"

#include "battle/btl_net.h"
#include "gamesystem/game_data.h"

//不正チェックのためのinclude
#include "print/global_msg.h"
#include "net/dwc_tool.h"
#include "print/str_tool.h"

//-------------------------------------
///	
//=====================================
#ifdef PM_DEBUG
#if defined(DEBUG_ONLY_FOR_toru_nagihashi) || defined(DEBUG_ONLY_FOR_shimoyamada)
#define GDSRAP_DEBUG_ERRORREQ_ON
#endif//definded
#endif //PM_DEBUG


//==============================================================================
//	定数定義
//==============================================================================
///DWCに渡すワークのサイズ
#define MYDWC_HEAPSIZE		0x20000

///エラーメッセージ展開用バッファのサイズ
#define DWC_ERROR_BUF_NUM		(16*8*2)

///送信前のウェイト(送信しているメッセージを見せる為にウェイトを入れている)
#define GDS_SEND_BEFORE_WAIT	(60)


//--------------------------------------------------------------
//	サブシーケンスの戻り値
//--------------------------------------------------------------
enum{
	SUBSEQ_CONTINUE,		///<サブシーケンス：維持
	SUBSEQ_NEXT,			///<サブシーケンス：次のサブシーケンスへ
	SUBSEQ_PROCCESS_CHANGE,	///<サブシーケンス：サブシーケンステーブル変更
};

//==============================================================================
//	構造体定義
//==============================================================================
///サブプロセスシーケンスの関数型定義
typedef int (*SUBPROCESS_FUNC)(GDS_RAP_WORK *, GDS_RAP_SUB_PROCCESS_WORK *sub_work);


//==============================================================================
//  データ
//==============================================================================
#include "gds_video_bit.cdat"

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
FS_EXTERN_OVERLAY(dpw_common);


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static int GDSRAP_MAIN_Send(GDS_RAP_WORK *gdsrap);
static int GDSRAP_MAIN_Recv(GDS_RAP_WORK *gdsrap);

static int Local_GetResponse(GDS_RAP_WORK *gdsrap);

static BOOL RecvSubProccess_Normal(void *work_gdsrap, void *work_recv_sub_work);
static BOOL RecvSubProccess_DataNumberSetSave(void *work_gdsrap, void *work_recv_sub_work);
static BOOL RecvSubProccess_SystemError(void *work_gdsrap, void *work_recv_sub_work);

static void GdsRap_DisconnectCallback(void* pUserWork, int code, int type, int ret );

static void GdsRap_GetEvilCheckPokeIndex( const GDS_RAP_WORK *cp_gdsrap, u32 idx, int *p_client, int *p_temoti );
//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   GDSライブラリ初期化処理
 *
 * @param   gdsrap				GDSラッパーワークへのポインタ
 * @param   init_data			初期化データ
 *
 * @retval  TRUE
 */
//--------------------------------------------------------------
int GDSRAP_Init(GDS_RAP_WORK *gdsrap, const GDSRAP_INIT_DATA *init_data)
{
	int ret;
	
  GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));

	GFL_STD_MemClear(gdsrap, sizeof(GDS_RAP_WORK));
	gdsrap->heap_id = init_data->heap_id;
	gdsrap->gamedata = init_data->gamedata;
	gdsrap->pSvl = init_data->pSvl;
	gdsrap->response_callback = init_data->response_callback;
	gdsrap->laststat = -1;
	gdsrap->send_req = POKE_NET_GDS_REQCODE_LAST;
	gdsrap->recv_wait_req = POKE_NET_GDS_REQCODE_LAST;
	
	gdsrap->response = GFL_HEAP_AllocMemory(init_data->heap_id, POKE_NET_GDS_GetResponseMaxSize(-1));
	GFL_STD_MemClear(gdsrap->response, POKE_NET_GDS_GetResponseMaxSize(-1));
	OS_TPrintf("レスポンスバッファのサイズ＝%x\n", POKE_NET_GDS_GetResponseMaxSize(-1));

	//メッセージマネージャ作成
	gdsrap->msgman_wifisys = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL, 
		ARCID_MESSAGE, NARC_message_wifi_system_dat, init_data->heap_id);
	gdsrap->wordset = WORDSET_Create(init_data->heap_id);	//単語バッファ作成
	gdsrap->ErrorString = GFL_STR_CreateBuffer(DWC_ERROR_BUF_NUM, init_data->heap_id);
	
	gdsrap->pokenet_auth.PID = init_data->gs_profile_id;
	gdsrap->pokenet_auth.ROMCode = PM_VERSION;
	gdsrap->pokenet_auth.LangCode = PM_LANG;
	ret = POKE_NET_GDS_Initialize(&gdsrap->pokenet_auth, GF_GDS_SERVER_URL, GF_GDS_SERVER_PORT);
	GF_ASSERT(ret == TRUE);		//初期化に失敗する事はありえないはず
	gdsrap->gdslib_initialize = TRUE;

	gdsrap->comm_initialize_ok = TRUE;

  GFL_NET_DWC_SetErrDisconnectCallback(GdsRap_DisconnectCallback, gdsrap );
	
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   GDSライブラリ破棄
 *
 * @param   gdsrap		
 */
//--------------------------------------------------------------
void GDSRAP_Exit(GDS_RAP_WORK *gdsrap)
{
  GFL_NET_DWC_SetErrDisconnectCallback(NULL, NULL );

  if(gdsrap->p_nhttp != NULL){
    NHTTP_RAP_PokemonEvilCheckDelete( gdsrap->p_nhttp );
    NHTTP_RAP_End( gdsrap->p_nhttp );
  }

	//GDSライブラリ解放
	POKE_NET_GDS_Release();
	gdsrap->gdslib_initialize = FALSE;

	//メッセージマネージャ削除
	GFL_STR_DeleteBuffer(gdsrap->ErrorString);
	WORDSET_Delete(gdsrap->wordset);
	GFL_MSG_Delete(gdsrap->msgman_wifisys);

	GFL_HEAP_FreeMemory(gdsrap->response);

  GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
}



//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   送信リクエスト：ミュージカルショット登録
 *
 * @param   gdsrap			
 * @param   profile		
 * @param   dress		
 *
 * @retval  TRUE：リクエストを受け付けた
 * @retval  FALSE：受け付けられなかった
 */
//--------------------------------------------------------------
int GDSRAP_Tool_Send_MusicalShotUpload(GDS_RAP_WORK *gdsrap, GDS_PROFILE_PTR gpp, const MUSICAL_SHOT_DATA *musshot)
{
	if(GDSRAP_MoveStatusAllCheck(gdsrap) == FALSE){
		return FALSE;
	}
  
  gdsrap->send_buf.mus_send.profile = *gpp;
  gdsrap->send_buf.mus_send.mus_shot = *musshot;
	
	gdsrap->send_before_wait = GDS_SEND_BEFORE_WAIT;
	gdsrap->send_req = POKE_NET_GDS_REQCODE_MUSICALSHOT_REGIST;
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   送信リクエスト：ミュージカルショット ダウンロード
 *
 * @param   gdsrap			
 * @param   monsno			受信するポケモン番号
 *
 * @retval  TRUE：リクエストを受け付けた
 * @retval  FALSE：受け付けられなかった
 */
//--------------------------------------------------------------
int GDSRAP_Tool_Send_MusicalShotDownload(GDS_RAP_WORK *gdsrap, int monsno)
{
	if(GDSRAP_MoveStatusAllCheck(gdsrap) == FALSE){
		return FALSE;
	}

	gdsrap->send_buf.sub_para.musical.recv_monsno = monsno;
	
	gdsrap->send_req = POKE_NET_GDS_REQCODE_MUSICALSHOT_GET;
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   送信リクエスト：バトルデータ登録
 *
 * @param   gdsrap		
 * @param   gpp						GDSプロフィールへのポインタ
 *
 * @retval  TRUE：リクエストを受け付けた
 * @retval  FALSE：受け付けられなかった
 * 
 * 録画データはサイズが巨大な為、コピーせずに、brsで読み込んでいるデータをそのまま送信します。
 * brsにデータを展開してからこの関数を使用するようにしてください。
 *
 * ！！この関数使用後、brsに読み込んでいるプロフィールデータの書き換え、
 * 	   録画データ本体の暗号化を行うのでこのままセーブしたり再生しないように注意！！
 */
//--------------------------------------------------------------
int GDSRAP_Tool_Send_BattleVideoUpload(GDS_RAP_WORK *gdsrap, GDS_PROFILE_PTR gpp)
{
	GDS_PROFILE_PTR profile_ptr;
	BATTLE_REC_SEND *br_send = BattleRec_RecWorkAdrsGet();
	
	if(GDSRAP_MoveStatusAllCheck(gdsrap) == FALSE){
		return FALSE;
	}

  //不正チェックのため名前を書き換えるので、
  //書き戻すためバッファに名前を入れる
  {
    int i;
    int client, poke;
    int client_max, temoti_max;
    REC_POKEPARA *param;

    BattleRec_ClientTemotiGet(br_send->head.mode, &client_max, &temoti_max);
    for( i = 0; i < GDS_VIDEO_EVIL_CHECK_NUM; i++ )
    {
      //書き換えるためにインデックスを取得
      GdsRap_GetEvilCheckPokeIndex( gdsrap, i, &client, &poke );

      param = &br_send->rec.rec_party[client].member[poke];
      STRTOOL_Copy( param->nickname, gdsrap->nickname[i], MONS_NAME_SIZE+EOM_SIZE );
    }
  }
  gdsrap->evilcheck_write = FALSE;

  //不正チェックを行う回数をクリア
  gdsrap->evil_check_loop  = 0;
  
	//録画データは巨大な為、コピーせずに、そのままbrsのデータを送信する
	gdsrap->send_buf.battle_rec_send_ptr = br_send;
	
	//GDSプロフィールのみ、最新のを適用する為、上書きする
	profile_ptr = BattleRec_GDSProfilePtrGet();
	GFL_STD_MemCopy(gpp, profile_ptr, sizeof(GDS_PROFILE));
	
	gdsrap->send_before_wait = GDS_SEND_BEFORE_WAIT;
	gdsrap->send_req = POKE_NET_GDS_REQCODE_BATTLEDATA_REGIST;
	return TRUE;
}

///強制的にフリーワードの自己紹介メッセージをセットする
void DEBUG_GDSRAP_SendVideoProfileFreeWordSet(GDS_RAP_WORK *gdsrap, u16 *set_code)
{
#ifdef PM_DEBUG
	GDS_PROFILE_PTR profile_ptr;

	profile_ptr = BattleRec_GDSProfilePtrGet();
	GFL_STD_MemCopy16(set_code, profile_ptr->event_self_introduction, EVENT_SELF_INTRO*2);
	profile_ptr->message_flag = 1;
	DEBUG_BattleRec_SecureFlagSet(GAMEDATA_GetSaveControlWork(gdsrap->gamedata));
#endif
}

//--------------------------------------------------------------
/**
 * @brief   送信リクエスト：バトルビデオダウンロード(詳細検索)
 *
 * @param   gdsrap		
 * @param   monsno			ポケモン番号(指定なし：BATTLE_REC_SEARCH_MONSNO_NONE)
 * @param   search_mode_no	検索施設(BATTLEMODE_SEARCH_NO)
 * @param   country_code	国コード(指定なし：BATTLE_REC_SEARCH_COUNTRY_CODE_NONE)
 * @param   local_code		地方コード(指定なし：BATTLE_REC_SEARCH_LOCAL_CODE_NONE)
 *
 * @retval  TRUE：リクエストを受け付けた
 * @retval  FALSE：受け付けられなかった
 */
//--------------------------------------------------------------
int GDSRAP_Tool_Send_BattleVideoSearchDownload(GDS_RAP_WORK *gdsrap, u16 monsno, BATTLEMODE_SEARCH_NO search_mode_no, u8 country_code, u8 local_code)
{
	if(GDSRAP_MoveStatusAllCheck(gdsrap) == FALSE){
		return FALSE;
	}
  
  GF_ASSERT(search_mode_no < NELEMS(BattleModeBitTbl));
  
	GFL_STD_MemClear(&gdsrap->send_buf.battle_rec_search, sizeof(BATTLE_REC_SEARCH_SEND));
	gdsrap->send_buf.battle_rec_search.monsno = monsno;
	gdsrap->send_buf.battle_rec_search.battle_mode = BattleModeBitTbl[search_mode_no].mode;
	gdsrap->send_buf.battle_rec_search.battle_mode_bit_mask = BattleModeBitTbl[search_mode_no].bit_mask;
	gdsrap->send_buf.battle_rec_search.country_code = country_code;
	gdsrap->send_buf.battle_rec_search.local_code = local_code;
	gdsrap->send_buf.battle_rec_search.server_version = BTL_NET_SERVER_VERSION;

	gdsrap->send_req_option = POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_CONDITION;
	gdsrap->send_req = POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH;
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   送信リクエスト：バトルビデオダウンロード(最新30件)
 *
 * @param   gdsrap		
 *
 * @retval  TRUE：リクエストを受け付けた
 * @retval  FALSE：受け付けられなかった
 */
//--------------------------------------------------------------
int GDSRAP_Tool_Send_BattleVideoNewDownload(GDS_RAP_WORK *gdsrap)
{
	if(GDSRAP_MoveStatusAllCheck(gdsrap) == FALSE){
		return FALSE;
	}

	gdsrap->send_req_option = POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_RANKING_LATEST;
	gdsrap->send_req = POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH;
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   送信リクエスト：バトルビデオダウンロード(バトルサブウェイランキング)
 *
 * @param   gdsrap		
 *
 * @retval  TRUE：リクエストを受け付けた
 * @retval  FALSE：受け付けられなかった
 */
//--------------------------------------------------------------
int GDSRAP_Tool_Send_BattleVideoSubwayDownload(GDS_RAP_WORK *gdsrap)
{
	if(GDSRAP_MoveStatusAllCheck(gdsrap) == FALSE){
		return FALSE;
	}

	gdsrap->send_req_option = POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_RANKING_SUBWAY;
	gdsrap->send_req = POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH;
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   送信リクエスト：バトルビデオダウンロード(通信対戦ランキング)
 *
 * @param   gdsrap		
 *
 * @retval  TRUE：リクエストを受け付けた
 * @retval  FALSE：受け付けられなかった
 */
//--------------------------------------------------------------
int GDSRAP_Tool_Send_BattleVideoCommDownload(GDS_RAP_WORK *gdsrap)
{
	if(GDSRAP_MoveStatusAllCheck(gdsrap) == FALSE){
		return FALSE;
	}

	gdsrap->send_req_option = POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_RANKING_COMM;
	gdsrap->send_req = POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH;
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   送信リクエスト：バトルビデオ取得
 *
 * @param   gdsrap		
 * @param   data_number	データナンバー
 *
 * @retval  TRUE：リクエストを受け付けた
 * @retval  FALSE：受け付けられなかった
 */
//--------------------------------------------------------------
int GDSRAP_Tool_Send_BattleVideo_DataDownload(GDS_RAP_WORK *gdsrap, u64 data_number)
{
	if(GDSRAP_MoveStatusAllCheck(gdsrap) == FALSE){
		return FALSE;
	}

	gdsrap->send_buf.sub_para.rec.data_number = data_number;

	gdsrap->send_req = POKE_NET_GDS_REQCODE_BATTLEDATA_GET;
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   送信リクエスト：バトルビデオお気に入り登録
 *
 * @param   gdsrap		
 * @param   data_number	データナンバー
 *
 * @retval  TRUE：リクエストを受け付けた
 * @retval  FALSE：受け付けられなかった
 */
//--------------------------------------------------------------
int GDSRAP_Tool_Send_BattleVideo_FavoriteUpload(GDS_RAP_WORK *gdsrap, u64 data_number)
{
	if(GDSRAP_MoveStatusAllCheck(gdsrap) == FALSE){
		return FALSE;
	}

	gdsrap->send_buf.sub_para.rec.data_number = data_number;

	gdsrap->send_req = POKE_NET_GDS_REQCODE_BATTLEDATA_FAVORITE;
	return TRUE;
}




//--------------------------------------------------------------
/**
 * @brief   メイン処理
 *
 * @param   gdsrap		
 *
 * @retval  
 *
 *
 */
//--------------------------------------------------------------
int GDSRAP_Main(GDS_RAP_WORK *gdsrap)
{
	int ret;

#if 0	//gds_main.c で呼ばれるようになった
	if(gdsrap->comm_initialize_ok){
		// 受信強度リンクを反映させる
		DWC_UpdateConnection();

		// Dpw_Tr_Main() だけは例外的にいつでも呼べる
	//	Dpw_Tr_Main();

		// 通信状態を確認してアイコンの表示を変える
		WirelessIconEasy_SetLevel( WM_LINK_LEVEL_3 - DWC_GetLinkLevel() );
	}
#endif

	//ネット接続中のみの処理
	if(gdsrap->gdslib_initialize == TRUE){
//		DWC_ProcessInet();
		
		//送信データ確認
		if(gdsrap->send_req != POKE_NET_GDS_REQCODE_LAST){
			if(gdsrap->send_before_wait > 0){
				gdsrap->send_before_wait--;
			}
			else{
				switch(POKE_NET_GDS_GetStatus()){
				case POKE_NET_GDS_STATUS_INITIALIZED:	//初期化済み
				case POKE_NET_GDS_STATUS_ABORTED:		//!< 中断終了
				case POKE_NET_GDS_STATUS_FINISHED:		//!< 正常終了
				case POKE_NET_GDS_STATUS_ERROR:			//!< エラー終了
					ret = GDSRAP_MAIN_Send(gdsrap);
					if(ret == TRUE){
						OS_TPrintf("data send! req_code = %d\n", gdsrap->send_req);
						gdsrap->send_req = POKE_NET_GDS_REQCODE_LAST;
						gdsrap->local_seq = 0;
					}
					break;
				default:	//今は送信出来ない
					OS_TPrintf("send wait ... status = %d\n", POKE_NET_GDS_GetStatus());
					break;
				}
			}
		}
		
		//受信データ確認
		GDSRAP_MAIN_Recv(gdsrap);
		
		//受信サブプロセス実行(データ受信後、制御をフリーに戻す前に実行する処理)
		if(gdsrap->recv_sub_work.recv_sub_proccess != NULL){
			if(gdsrap->recv_sub_work.recv_sub_proccess(gdsrap, &gdsrap->recv_sub_work) == TRUE){
				gdsrap->recv_sub_work.recv_sub_proccess = NULL;
				//受信データのコールバック呼び出し
				if(gdsrap->recv_sub_work.user_callback_func != NULL){
					gdsrap->recv_sub_work.user_callback_func(
						gdsrap->response_callback.callback_work, &gdsrap->error_info);
					gdsrap->recv_sub_work.user_callback_func = NULL;
				}
				GFL_STD_MemClear(&gdsrap->recv_sub_work, sizeof(GDS_RECV_SUB_PROCCESS_WORK));
				gdsrap->recv_wait_req = POKE_NET_GDS_REQCODE_LAST;
			}
		}
	}
	
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   送信処理
 *
 * @param   gdsrap		
 *
 * @retval  TRUE:送信成功
 * @retval  FALSE:送信失敗
 */
//--------------------------------------------------------------
static int GDSRAP_MAIN_Send(GDS_RAP_WORK *gdsrap)
{
  enum
  {
    // バトルビデオ登録のシーケンス
    SEQ_START_EVILCHECK,
    SEQ_WAIT_EVILCHECK,
    SEQ_START_SEND,
    SEQ_SEND_VIDEO,
  };
	int ret = FALSE;
	
	//リクエストにデータが貯まっていれば送信
	switch(gdsrap->send_req){
	case POKE_NET_GDS_REQCODE_MUSICALSHOT_REGIST:	// ミュージカルショット登録
		ret = POKE_NET_GDS_MusicalShotRegist(&gdsrap->send_buf.mus_send, gdsrap->response);
		break;
	case POKE_NET_GDS_REQCODE_MUSICALSHOT_GET:		// ミュージカルショット取得
		ret = POKE_NET_GDS_MusicalShotGet(gdsrap->send_buf.sub_para.musical.recv_monsno, 
			gdsrap->response);
		break;
	case POKE_NET_GDS_REQCODE_BATTLEDATA_REGIST:	// バトルビデオ登録

	  switch(gdsrap->local_seq){
    case SEQ_START_EVILCHECK:
      {
        BATTLE_REC_SEND *br_send = BattleRec_RecWorkAdrsGet();
        int client_max, temoti_max, i;
        BOOL ret;

        GF_ASSERT(gdsrap->p_nhttp == NULL);
        gdsrap->p_nhttp = NHTTP_RAP_Init( gdsrap->heap_id, 
            MyStatus_GetProfileID( GAMEDATA_GetMyStatus(gdsrap->gamedata) ), gdsrap->pSvl );
        //不正検査領域確保
        NHTTP_RAP_PokemonEvilCheckCreate( gdsrap->p_nhttp, gdsrap->heap_id, 
            sizeof(REC_POKEPARA) * GDS_VIDEO_EVIL_CHECK_NUM, NHTTP_POKECHK_VIDIO );

        //ポケモン登録
        BattleRec_ClientTemotiGet(br_send->head.mode, &client_max, &temoti_max);
        gdsrap->evil_check_count = 0;
        for(i = 0; i < client_max; i++){
          NHTTP_RAP_PokemonEvilCheckAdd(gdsrap->p_nhttp, 
              br_send->rec.rec_party[i].member, 
              sizeof(REC_POKEPARA) * br_send->rec.rec_party[i].PokeCount);//temoti_max);
          gdsrap->evil_check_count += br_send->rec.rec_party[i].PokeCount;
        }

        //不正検査 コネクション作成
        ret = NHTTP_RAP_PokemonEvilCheckConectionCreate(gdsrap->p_nhttp);
        GF_ASSERT(ret);

        ret = NHTTP_RAP_StartConnect( gdsrap->p_nhttp ) == NHTTP_ERROR_NONE;
        GF_ASSERT( ret );

        gdsrap->local_seq++;
      }
      break;

	  case SEQ_WAIT_EVILCHECK:
	    {
        NHTTPError error;
        error = NHTTP_RAP_Process( gdsrap->p_nhttp );
        if( NHTTP_ERROR_NONE == error ){
          void *p_data;
          int i;
          int poke_result;
          
          p_data  = NHTTP_RAP_GetRecvBuffer( gdsrap->p_nhttp );
          //送られてきた状態は正常か
          if( NHTTP_RAP_EVILCHECK_GetStatusCode( p_data ) == 0 )    // 正常
          {
            gdsrap->error_nhttp = FALSE;
            // 署名を取得
            { 
              const s8 *cp_sign  = NHTTP_RAP_EVILCHECK_GetSign( p_data, gdsrap->evil_check_count );
              GFL_STD_MemCopy( cp_sign, gdsrap->sign, NHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN );
            }

            OS_TPrintf("不正検査成功\n");
            for( i = 0; i < gdsrap->evil_check_count; i++ ){
              poke_result  = NHTTP_RAP_EVILCHECK_GetPokeResult( p_data, i );
              gdsrap->nhttp_last_error = poke_result;
              if( poke_result != NHTTP_RAP_EVILCHECK_RESULT_OK ){
                OS_TPrintf("ポケモン認証エラー %d番 error=%d\n", i, poke_result);
              }
            }

            if( gdsrap->evil_check_loop == 1 )
            {
              //名前を変えたので、CRCを再計算する
              BattleRec_CalcCrcRec( BattleRec_WorkPtrGet() );
            }

            gdsrap->local_seq++;
          }
          else
          {
            BATTLE_REC_SEND *br_send = BattleRec_RecWorkAdrsGet();

            int client, poke;
            int client_max, temoti_max;
            REC_POKEPARA *param;

            OS_TPrintf("不正検査%d失敗\n", gdsrap->evil_check_loop);
            if( gdsrap->evil_check_loop == 1 )
            {
              gdsrap->error_nhttp = TRUE;
            }

            BattleRec_ClientTemotiGet(br_send->head.mode, &client_max, &temoti_max);
            for( i = 0; i < gdsrap->evil_check_count; i++ ){
              poke_result  = NHTTP_RAP_EVILCHECK_GetPokeResult( p_data, i );
              gdsrap->nhttp_last_error = poke_result;
              if( poke_result != NHTTP_RAP_EVILCHECK_RESULT_OK ){
                OS_TPrintf("ポケモン認証エラー %d番 error=%d\n", i, poke_result);

                //名前を書き換えて再度送る
                if( gdsrap->evil_check_loop == 0 )
                {
                  //書き換えるためにインデックスを取得
                  GdsRap_GetEvilCheckPokeIndex( gdsrap, i, &client, &poke );

                  param = &br_send->rec.rec_party[client].member[poke];
                  GFL_MSG_GetStringRaw( GlobalMsg_PokeName, param->monsno, param->nickname, MONS_NAME_SIZE+EOM_SIZE );
                  OS_TPrintf("ニックネーム書き換え cliend_idx=%d poke_idx=%d\n", client, poke );
                  gdsrap->evilcheck_write = TRUE;
                }
              }
            }
            
            if( gdsrap->evil_check_loop == 0 )
            {
              gdsrap->local_seq = SEQ_START_EVILCHECK;
              gdsrap->evil_check_loop++;
            }
            else
            {
              OS_TPrintf( "２回目の不正！\n" );
              gdsrap->local_seq++;
            }
          }

          NHTTP_RAP_PokemonEvilCheckDelete( gdsrap->p_nhttp );
          NHTTP_RAP_End( gdsrap->p_nhttp );
          gdsrap->p_nhttp = NULL; 
        }
        else{
          OS_TPrintf("不正検査%d回中...\n", gdsrap->evil_check_loop);

          if( NHTTP_ERROR_BUSY != error )
          {
            if(gdsrap->p_nhttp != NULL){
              NHTTP_RAP_PokemonEvilCheckDelete( gdsrap->p_nhttp );
              NHTTP_RAP_ErrorClean( gdsrap->p_nhttp);
              NHTTP_RAP_End( gdsrap->p_nhttp );
              gdsrap->p_nhttp = NULL;
              NAGI_Printf( "GDS_RAP NTHHP　エラークリーン\n" );
            }
          }
          return FALSE;
        }
      }
      break;

    case SEQ_START_SEND:
      //brsに展開されている録画データ本体は復号化されているので、送信する前に再度暗号化する
      BattleRec_GDS_SendData_Conv(GAMEDATA_GetSaveControlWork(gdsrap->gamedata));
      gdsrap->local_seq++;
      break;

	  case SEQ_SEND_VIDEO:
  		ret = POKE_NET_GDS_BattleDataRegist(gdsrap->send_buf.battle_rec_send_ptr, 
  			gdsrap->sign, NHTTP_RAP_EVILCHECK_RESPONSE_SIGN_LEN, gdsrap->response);
  		if(ret == TRUE){
  			OS_TPrintf("バトルビデオ登録リクエスト完了\n");
  		}
  		break;
  	}
  	break;
	case POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH:	// バトルビデオ検索
		switch(gdsrap->send_req_option){
		case POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_CONDITION:	//詳細検索
			ret = POKE_NET_GDS_BattleDataSearchCondition(&gdsrap->send_buf.battle_rec_search,
				gdsrap->response);
			break;
		case POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_RANKING_LATEST:	//最新30件
		  ret = POKE_NET_GDS_BattleDataSearchRankingLatest(BTL_NET_SERVER_VERSION, gdsrap->response);
			break;
		case POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_RANKING_SUBWAY:	//地下鉄ランキング
			ret = POKE_NET_GDS_BattleDataSearchRankingSubway(BTL_NET_SERVER_VERSION, gdsrap->response);
			break;
	  case POKE_NET_GDS_REQUEST_BATTLEDATA_SEARCHTYPE_RANKING_COMM:   //通信対戦ランキング
	    ret = POKE_NET_GDS_BattleDataSearchRankingComm(BTL_NET_SERVER_VERSION, gdsrap->response);
	    break;
		}
		if(ret == TRUE){
			OS_TPrintf("バトルビデオ検索リクエスト完了\n");
		}
		break;
	case POKE_NET_GDS_REQCODE_BATTLEDATA_GET:	// バトルビデオ取得
		ret = POKE_NET_GDS_BattleDataGet(gdsrap->send_buf.sub_para.rec.data_number, 
			BTL_NET_SERVER_VERSION, gdsrap->response);
		if(ret == TRUE){
			OS_TPrintf("バトルビデオ取得リクエスト完了\n");
		}
		break;
	case POKE_NET_GDS_REQCODE_BATTLEDATA_FAVORITE:	// バトルビデオお気に入り登録
		ret = POKE_NET_GDS_BattleDataFavorite(gdsrap->send_buf.sub_para.rec.data_number, 
			gdsrap->response);
		if(ret == TRUE){
			OS_TPrintf("バトルビデオお気に入り登録リクエスト完了\n");
		}
		break;
	default:
		OS_Printf("-not supported\n");
		GF_ASSERT(0);
		break;
	}
	
	if(ret == TRUE){
		OS_TPrintf("送信成功 send_req = %d\n", gdsrap->send_req);
		gdsrap->recv_wait_req = gdsrap->send_req;
	}
	else{
		OS_TPrintf("送信失敗。send_req = %d\n", gdsrap->send_req);
	}
	return ret;
}

//--------------------------------------------------------------
/**
 * @brief   受信ステータス確認
 *
 * @param   gdsrap		
 *
 * @retval  
 */
//--------------------------------------------------------------
static int GDSRAP_MAIN_Recv(GDS_RAP_WORK *gdsrap)
{
	if(gdsrap->recv_wait_req == POKE_NET_GDS_REQCODE_LAST){
		return TRUE;
	}
	
	// ステータス取得
	gdsrap->stat = POKE_NET_GDS_GetStatus();

#ifdef GDSRAP_DEBUG_ERRORREQ_ON
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    gdsrap->stat  = POKE_NET_GDS_STATUS_ERROR;
  }
#endif //GDSRAP_DEBUG_ERRORREQ_ON

	if(	gdsrap->stat != gdsrap->laststat ){
		// ステータスに変化があった
		switch(gdsrap->stat){
		case POKE_NET_GDS_STATUS_INACTIVE:					// 非稼動中
			OS_Printf("-inactive\n");
			break;
		case POKE_NET_GDS_STATUS_INITIALIZED:				// 初期化済み
			OS_Printf("-initialized\n");
			break;
		case POKE_NET_GDS_STATUS_REQUEST:					// リクエスト発行
			OS_Printf("-request\n");
			break;
		case POKE_NET_GDS_STATUS_NETSETTING:				// ネットワーク設定中
			OS_Printf("-netsetting\n");
			break;
		case POKE_NET_GDS_STATUS_CONNECTING:				// 接続中
			OS_Printf("-connecting\n");
			break;
		case POKE_NET_GDS_STATUS_SENDING:					// 送信中
			OS_Printf("-sending\n");
			break;
		case POKE_NET_GDS_STATUS_RECEIVING:					// 受信中
			OS_Printf("-receiving\n");
			break;
		case POKE_NET_GDS_STATUS_ABORTED:					// 中断
			OS_Printf("-aborted\n");
			gdsrap->error_info.type = GDS_ERROR_TYPE_STATUS;
			gdsrap->error_info.req_code = 0;
			gdsrap->error_info.result = POKE_NET_GDS_STATUS_ABORTED;
			gdsrap->error_info.occ = TRUE;
		//	gdsrap->recv_wait_req = POKE_NET_GDS_REQCODE_LAST;
			gdsrap->recv_sub_work.recv_sub_proccess = RecvSubProccess_SystemError;
			break;
		case POKE_NET_GDS_STATUS_FINISHED:					// 正常終了
			OS_Printf("-finished\n");
			gdsrap->recv_sub_work.recv_sub_proccess = RecvSubProccess_Normal;
			// レスポンス処理
			Local_GetResponse(gdsrap);
			break;
		case POKE_NET_GDS_STATUS_ERROR:						// エラー終了
			OS_Printf("-error lasterrorcode:%d\n" ,POKE_NET_GDS_GetLastErrorCode());
			gdsrap->error_info.type = GDS_ERROR_TYPE_LIB;
			gdsrap->error_info.req_code = 0;
			gdsrap->error_info.result = POKE_NET_GDS_GetLastErrorCode();
			gdsrap->error_info.occ = TRUE;

#ifdef GDSRAP_DEBUG_ERRORREQ_ON
      if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
      {
        gdsrap->error_info.result  = POKE_NET_GDS_LASTERROR_CONNECT;
      }
#endif //GDSRAP_DEBUG_ERRORREQ_ON
		//	gdsrap->recv_wait_req = POKE_NET_GDS_REQCODE_LAST;
			gdsrap->recv_sub_work.recv_sub_proccess = RecvSubProccess_SystemError;
			break;
		}
		gdsrap->laststat = gdsrap->stat;
	}
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   データ受信後のサブプロセス：特に何も処理しない場合
 *
 * @param   gdsrap		
 *
 * @retval  TRUE:処理完了。　FALSE:処理継続中
 */
//--------------------------------------------------------------
static BOOL RecvSubProccess_Normal(void *work_gdsrap, void *work_recv_sub_work)
{
	GDS_RAP_WORK *gdsrap = work_gdsrap;
	GDS_RECV_SUB_PROCCESS_WORK *recv_sub_work = work_recv_sub_work;

	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   データ受信後のサブプロセス：録画データにデータナンバーをセットしてセーブする
 *
 * @param   gdsrap		
 * @param   recv_sub_work		
 *
 * @retval  
 */
//--------------------------------------------------------------
static BOOL RecvSubProccess_DataNumberSetSave(void *work_gdsrap, void *work_recv_sub_work)
{
	GDS_RAP_WORK *gdsrap = work_gdsrap;
	GDS_RECV_SUB_PROCCESS_WORK *recv_sub_work = work_recv_sub_work;
	POKE_NET_RESPONSE *res;
	POKE_NET_GDS_RESPONSE_BATTLEDATA_REGIST *param;
	SAVE_RESULT ret;
	
  //名前を書き戻す処理
  //不正チェックのため名前を書き換えるので、
  //書き戻すためバッファに名前を入れる
  if( recv_sub_work->recv_save_seq0 == 0 )
  {
    if( gdsrap->evilcheck_write )
    {
      int i;
      int client, poke;
      int client_max, temoti_max;
      REC_POKEPARA *param;
      BATTLE_REC_SEND *br_send = BattleRec_RecWorkAdrsGet();

      //すでに暗号化されているので復号化
      BattleRec_DataDecoded();

      //名前書き戻し
      BattleRec_ClientTemotiGet(br_send->head.mode, &client_max, &temoti_max);
      for( i = 0; i < GDS_VIDEO_EVIL_CHECK_NUM; i++ )
      {
        //書き換えるためにインデックスを取得
        GdsRap_GetEvilCheckPokeIndex( gdsrap, i, &client, &poke );

        param = &br_send->rec.rec_party[client].member[poke];
        STRTOOL_Copy( gdsrap->nickname[i], param->nickname, MONS_NAME_SIZE+EOM_SIZE );
      }
      BattleRec_CalcCrcRec( BattleRec_WorkPtrGet() );

      //再度暗号化
      BattleRec_DataCoded();
    }
  }

  //セーブ処理
	res = POKE_NET_GDS_GetResponse();
	param = (POKE_NET_GDS_RESPONSE_BATTLEDATA_REGIST *)(res->Param);
	ret = BattleRec_GDS_MySendData_DataNumberSetSave(
		gdsrap->gamedata, param->Code, 
		&recv_sub_work->recv_save_seq0, &recv_sub_work->recv_save_seq1, gdsrap->heap_id);
	if(ret == SAVE_RESULT_OK || ret == SAVE_RESULT_NG){
		OS_TPrintf("外部セーブ完了\n");
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   データ受信後のサブプロセス：システムエラーが発生した場合のケア処理
 *
 * @param   gdsrap		
 *
 * @retval  
 */
//--------------------------------------------------------------
static BOOL RecvSubProccess_SystemError(void *work_gdsrap, void *work_recv_sub_work)
{
	GDS_RAP_WORK *gdsrap = work_gdsrap;
	GDS_RECV_SUB_PROCCESS_WORK *recv_sub_work = work_recv_sub_work;
	
	OS_TPrintf("システムエラーが発生した為、ケア処理中 laststat=%d, recv_wait_req=%d\n", gdsrap->laststat, gdsrap->recv_wait_req);
	
	//送信前のセーブは無くなったし、ローカルでの送信済みチェックも無くなったので
	//ケア処理は無くなった
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  切断コールバック
 *
 *	@param	pUserWork   ユーザーワーク
 *	@param	code        エラーコード
 *	@param	type        エラー種類
 *	@param	ret         エラーリターン
 */
//-----------------------------------------------------------------------------
static void GdsRap_DisconnectCallback(void* pUserWork, int code, int type, int ret )
{
  GDS_RAP_WORK  *gdsrap = pUserWork;

  switch( type )
  {
    //切断エラー
  case DWC_ETYPE_SHUTDOWN_FM:
  case DWC_ETYPE_DISCONNECT:
    //NHTTPが解放していないならば先に呼ぶ
    if(gdsrap->p_nhttp != NULL){
      NHTTP_RAP_PokemonEvilCheckDelete( gdsrap->p_nhttp );
      NHTTP_RAP_ErrorClean( gdsrap->p_nhttp);
      NHTTP_RAP_End( gdsrap->p_nhttp );
      gdsrap->p_nhttp = NULL;
      NAGI_Printf( "切断コールバック　エラークリーン\n" );
    }
    break;
  }

  NAGI_Printf( "切断コールバック\n" );
}

//----------------------------------------------------------------------------
/**
 *	@brief  連続インデックスから、クライアントインデックスと手持ちインデックスを取得
 *
 *	@param	const GDS_RAP_WORK *cp_gdsrap GDSラップ
 *	@param	idx         連続インデックス
 *	@param	*p_client   クライアントインデックス
 *	@param	*p_temoti   手持ちインデックス
 */
//-----------------------------------------------------------------------------
static void GdsRap_GetEvilCheckPokeIndex( const GDS_RAP_WORK *cp_gdsrap, u32 idx, int *p_client, int *p_temoti )
{
  int client_max, temoti_max;
  int index = 0;
  BATTLE_REC_SEND *br_send = BattleRec_RecWorkAdrsGet();
  BattleRec_ClientTemotiGet(br_send->head.mode, &client_max, &temoti_max);

  for( *p_client = 0; (*p_client) < client_max; (*p_client)++ )
  {
    for( *p_temoti  = 0; (*p_temoti) < br_send->rec.rec_party[*p_client].PokeCount; (*p_temoti)++ )
    {
      if( index == idx )
      {
        return;
      }
      index++;
    }
  }
}
//--------------------------------------------------------------
/**
 * @brief   受信データ解釈
 *
 * @param   gdsrap		
 *
 * @retval  TRUE:正常終了。　FALSE:エラー発生(エラー情報はerror_info参照)
 */
//--------------------------------------------------------------
static int Local_GetResponse(GDS_RAP_WORK *gdsrap)
{
	POKE_NET_RESPONSE *res;
	int size;
	int result = TRUE;
	
	res = POKE_NET_GDS_GetResponse();
	size = POKE_NET_GDS_GetResponseSize();
	
	GF_ASSERT(gdsrap->recv_wait_req == res->ReqCode);
	
	gdsrap->recv_sub_work.user_callback_func = NULL;
	
	// == ここでそれぞれのレスポンスに対する処理を行う ==
	switch(res->ReqCode){
	case POKE_NET_GDS_REQCODE_MUSICALSHOT_REGIST:		// ミュージカルショット登録
		OS_TPrintf("受信：ミュージカルショットアップロード\n");
		result = GDS_RAP_RESPONSE_MusicalShot_Upload(gdsrap, res);
		gdsrap->recv_sub_work.user_callback_func = gdsrap->response_callback.func_musicalshot_regist;
		break;
	case POKE_NET_GDS_REQCODE_MUSICALSHOT_GET:			// ミュージカルショット取得
		OS_TPrintf("受信：ミュージカルショット取得\n");
		result = GDS_RAP_RESPONSE_MusicalShot_Download(gdsrap, res);
		gdsrap->recv_sub_work.user_callback_func = gdsrap->response_callback.func_musicalshot_get;
		break;
	case POKE_NET_GDS_REQCODE_BATTLEDATA_REGIST:		// バトルビデオ登録
		OS_TPrintf("受信：バトルビデオ登録\n");
		result = GDS_RAP_RESPONSE_BattleVideo_Upload(gdsrap, res);
		gdsrap->recv_sub_work.user_callback_func = gdsrap->response_callback.func_battle_video_regist;
		if(result == TRUE){
			//登録コードを自分のデータに入れて、外部セーブ実行
			gdsrap->recv_sub_work.recv_sub_proccess = RecvSubProccess_DataNumberSetSave;
		}
		break;
	case POKE_NET_GDS_REQCODE_BATTLEDATA_SEARCH:		// バトルビデオ検索
		OS_TPrintf("受信：バトルビデオ検索\n");
		result = GDS_RAP_RESPONSE_BattleVideo_Search_Download(gdsrap, res);
		gdsrap->recv_sub_work.user_callback_func = gdsrap->response_callback.func_battle_video_search_get;
		break;
	case POKE_NET_GDS_REQCODE_BATTLEDATA_GET:		// バトルビデオ取得
		OS_TPrintf("受信：バトルビデオ取得\n");
		result = GDS_RAP_RESPONSE_BattleVideo_Data_Download(gdsrap, res);
		gdsrap->recv_sub_work.user_callback_func = gdsrap->response_callback.func_battle_video_data_get;
		break;
	case POKE_NET_GDS_REQCODE_BATTLEDATA_FAVORITE:		// バトルビデオお気に入り登録
		OS_TPrintf("受信：バトルビデオお気に入り登録\n");
		result = GDS_RAP_RESPONSE_BattleVideo_Favorite_Upload(gdsrap, res);
		gdsrap->recv_sub_work.user_callback_func = gdsrap->response_callback.func_battle_video_favorite_regist;
		break;
		
	case POKE_NET_GDS_REQCODE_DEBUG_MESSAGE:			// デバッグ用リクエスト
	default:
		OS_TPrintf("受信：未定義なもの\n");
		break;
	}
	
	//エラー情報をセット
	if(result == FALSE){
		gdsrap->error_info.type = GDS_ERROR_TYPE_APP;
		gdsrap->error_info.req_code = res->ReqCode;
		gdsrap->error_info.result = res->Result;
		gdsrap->error_info.occ = TRUE;
	}
	else{
		gdsrap->error_info.occ = FALSE;
	}
	
	return result;
}

//--------------------------------------------------------------
/**
 * @brief   エラー情報を取得する
 *
 * @param   gdsrap			
 * @param   error_info		エラー情報代入先
 *
 * @retval  TRUE:エラーが発生している
 * @retval  FALSE:エラー無し
 */
//--------------------------------------------------------------
BOOL GDSRAP_ErrorInfoGet(GDS_RAP_WORK *gdsrap, GDS_RAP_ERROR_INFO **error_info)
{
	*error_info = &gdsrap->error_info;
	return gdsrap->error_info.occ;
}

//--------------------------------------------------------------
/**
 * @brief   エラー情報をクリアする
 *
 * @param   gdsrap		
 */
//--------------------------------------------------------------
void GDSRAP_ErrorInfoClear(GDS_RAP_WORK *gdsrap)
{
	GFL_STD_MemClear(&gdsrap->error_info, sizeof(GDS_RAP_ERROR_INFO));
}

//--------------------------------------------------------------
/**
 * @brief   GDSRAPが何らかの動作を実行、又は受付、待ち中、などをしているか確認する
 *
 * @param   gdsrap		
 *
 * @retval  TRUE:何の処理も実行、受付していない
 * @retval  FALSE:何らかの処理を実行、受付、待っている
 */
//--------------------------------------------------------------
BOOL GDSRAP_MoveStatusAllCheck(GDS_RAP_WORK *gdsrap)
{
	if(gdsrap->send_req == POKE_NET_GDS_REQCODE_LAST
			&& gdsrap->recv_wait_req == POKE_NET_GDS_REQCODE_LAST){
		return TRUE;
	}
	return FALSE;
}



//==============================================================================
//	デバッグ用ツール
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   デバッグツール：セーブ実行フラグをOFFする
 *
 * @param   gdsrap		
 */
//--------------------------------------------------------------
void DEBUG_GDSRAP_SaveFlagReset(GDS_RAP_WORK *gdsrap)
{
#ifdef PM_DEBUG
//	gdsrap->send_before_div_save = 0;
#endif
}

