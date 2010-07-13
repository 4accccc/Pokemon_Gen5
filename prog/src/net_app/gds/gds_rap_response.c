//==============================================================================
/**
 * @file	gds_rap_response.c
 * @brief	gds_rap.cから受信データの判定や取得部分を独立させたもの
 * @author	matsuda
 * @date	2008.01.13(日)
 */
//==============================================================================
#include <gflib.h>
#include <dwc.h>
#include "net/dwc_rap.h"
#include "savedata\save_control.h"
#include "savedata/gds_profile.h"
#include "net_app/gds/gds_profile_local.h"
#include "battle/btl_common.h"

#include "savedata/gds_profile.h"

#include <arc_tool.h>
#include "print/wordset.h"
#include "message.naix"

#include "gds_rap.h"
#include "gds_rap_response.h"

#include "msg/msg_wifi_system.h"



//--------------------------------------------------------------
/**
 * @brief   レスポンスデータ解釈：ドレスアップショット アップロード
 *
 * @param   gdsrap		
 * @param   res		
 *
 * @retval  TRUE:成功
 * @retval  FALSE:何らかのエラーが発生(内容はres->Result参照)
 */
//--------------------------------------------------------------
int GDS_RAP_RESPONSE_MusicalShot_Upload(GDS_RAP_WORK *gdsrap, POKE_NET_RESPONSE *res)
{
	POKE_NET_GDS_RESPONSE_MUSICALSHOT_REGIST *param;
	int ret = FALSE;
	
	switch(res->Result){
	//!< 登録成功
	case POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_SUCCESS:
		param = (POKE_NET_GDS_RESPONSE_MUSICALSHOT_REGIST *)(res->Param);
		OS_TPrintf("ミュージカルショットアップロード成功。登録コード = %d\n", param->Code);
		ret = TRUE;
		break;
	//!< ユーザー認証エラー
	case POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_ERROR_AUTH:
		OS_TPrintf("ミュージカルショット登録エラー！:ユーザー認証エラー\n");
		break;
	//!< すでに登録されている
	case POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_ERROR_ALREADY:
		OS_TPrintf("ミュージカルショット登録エラー！:既に登録されている\n");
		break;
	//!< 不正なデータ
	case POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_ERROR_ILLEGAL:
		OS_TPrintf("ミュージカルショット登録エラー！:不正データ\n");
		break;
	//!< 不正なポケモン番号
  case POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_ERROR_ILLEGALPOKEMON:
		OS_TPrintf("ミュージカルショット登録エラー！:不正なポケモン番号\n");
		break;
	//!< 不正なユーザープロフィール
	case POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_ERROR_ILLEGALPROFILE:
		OS_TPrintf("ミュージカルショット登録エラー！:不正プロフィール\n");
		break;
	//!< その他エラー
	case POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_REGIST_ERROR_UNKNOWN:
	default:
		OS_TPrintf("ミュージカルショット登録エラー！:その他のエラー\n");
		break;
	}
	
	return ret;
}

//--------------------------------------------------------------
/**
 * @brief   レスポンスデータ解釈：ミュージカルショット ダウンロード
 *
 * @param   gdsrap		
 * @param   res		
 *
 * @retval  TRUE:成功
 * @retval  FALSE:何らかのエラーが発生(内容はres->Result参照)
 */
//--------------------------------------------------------------
int GDS_RAP_RESPONSE_MusicalShot_Download(GDS_RAP_WORK *gdsrap, POKE_NET_RESPONSE *res)
{
	POKE_NET_GDS_RESPONSE_MUSICALSHOT_GET *param;
	int ret = FALSE;
	
	switch(res->Result){
	//!< 取得成功
	case POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_GET_SUCCESS:
		param = (POKE_NET_GDS_RESPONSE_MUSICALSHOT_GET *)(res->Param);
		OS_TPrintf("ミュージカルショット受信成功。ヒット件数 = %d\n", param->HitCount);
		ret = TRUE;
		break;
	//!< ユーザー認証エラー
	case POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_GET_ERROR_AUTH:
		OS_TPrintf("ミュージカルショット受信エラー！:ユーザー認証エラー\n");
		break;
	//!< ポケモン番号エラー
	case POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_GET_ERROR_ILLEGALPOKEMON:
		OS_TPrintf("ミュージカルショット受信エラー！:ポケモン番号エラー\n");
		break;
	//!< その他エラー
	case POKE_NET_GDS_RESPONSE_RESULT_MUSICALSHOT_GET_ERROR_UNKNOWN:
	default:
		OS_TPrintf("ミュージカルショット受信エラー！:その他のエラー\n");
		break;
	}
	
	return ret;
}

//--------------------------------------------------------------
/**
 * @brief   受信バッファからデータが入っている部分のポインタを取得：ミュージカルショット
 *
 * @param   gdsrap		
 * @param   mus_array		ポインタをセットする配列データ
 * @param   array_max		mus_arrayの配列要素数
 *
 * @retval  受信件数
 *
 * mus_array[0] ～ mus_array[x] まで受信データへのポインタがセットされます
 */
//--------------------------------------------------------------
int GDS_RAP_RESPONSE_MusicalShot_Download_RecvPtr_Set(GDS_RAP_WORK *gdsrap, MUSICAL_SHOT_RECV **mus_array, int array_max)
{
	POKE_NET_RESPONSE *res;
	POKE_NET_GDS_RESPONSE_MUSICALSHOT_GET *param;
	POKE_NET_GDS_RESPONSE_MUSICALSHOT_RECVDATA *mus_recv;
	int i, hit_count;
	
	res = POKE_NET_GDS_GetResponse();
	param = (POKE_NET_GDS_RESPONSE_MUSICALSHOT_GET *)(res->Param);
	
	hit_count = param->HitCount;
	if(hit_count > array_max){
		hit_count = array_max;
	//	GF_ASSERT(0);	//用意しているバッファよりも受信件数が多い
	}

	mus_recv = (POKE_NET_GDS_RESPONSE_MUSICALSHOT_RECVDATA *)(param->Data);
	for(i = 0; i < hit_count; i++){
		mus_array[i] = &mus_recv[i].Data;
	}
	for( ; i < array_max; i++){
		mus_array[i] = NULL;
	}

	return hit_count;
}

//--------------------------------------------------------------
/**
 * @brief   レスポンスデータ解釈：バトルビデオ登録
 *
 * @param   gdsrap		
 * @param   res		
 *
 * @retval  TRUE:成功
 * @retval  FALSE:何らかのエラーが発生(内容はres->Result参照)
 */
//--------------------------------------------------------------
int GDS_RAP_RESPONSE_BattleVideo_Upload(GDS_RAP_WORK *gdsrap, POKE_NET_RESPONSE *res)
{
	POKE_NET_GDS_RESPONSE_BATTLEDATA_REGIST *param;
	int ret = FALSE;
	
	switch(res->Result){
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_SUCCESS:		//!< 登録成功
		param = (POKE_NET_GDS_RESPONSE_BATTLEDATA_REGIST *)(res->Param);
		OS_TPrintf("バトルビデオ登録受信成功：登録コード%d\n", param->Code);
		ret = TRUE;
		break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_AUTH:		//!< ユーザー認証エラー
		OS_TPrintf("バトルビデオ登録受信エラー！:ユーザー認証エラー\n");
		break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ALREADY:	//!< すでに登録されている
		OS_TPrintf("バトルビデオ登録受信エラー！:既に登録されている\n");
		break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGAL:	//!< 不正なデータ
		OS_TPrintf("バトルビデオ登録受信エラー！:不正データ\n");
		break;
  case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGAL_RANKINGTYPE:  //!< 不正なランキング種別
    OS_TPrintf("バトルビデオ登録受信エラー！:不正なランキング種別\n");
    break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGALPROFILE:	//!< 不正なユーザープロフィール
		OS_TPrintf("バトルビデオ登録受信エラー！:不正なユーザー\n");
		break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_ILLEGALPOKEMON_VERIFY: //!< ポケモン署名でエラー
		OS_TPrintf("バトルビデオ登録受信エラー！:ポケモン署名でエラー\n");
	  break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_REGIST_ERROR_UNKNOWN:	//!< その他エラー
	default:
		OS_TPrintf("バトルビデオ登録受信エラー！:その他のエラー %d\n", res->Result);
		break;
	}

	return ret;
}

//--------------------------------------------------------------
/**
 * @brief   受信バッファからデータ取得：バトルビデオ登録
 *
 * @param   gdsrap		
 * 
 * @retval	登録コード
 */
//--------------------------------------------------------------
u64 GDS_RAP_RESPONSE_BattleVideo_Upload_DataGet(GDS_RAP_WORK *gdsrap)
{
	POKE_NET_RESPONSE *res;
	POKE_NET_GDS_RESPONSE_BATTLEDATA_REGIST *param;
	
	res = POKE_NET_GDS_GetResponse();
	param = (POKE_NET_GDS_RESPONSE_BATTLEDATA_REGIST *)(res->Param);
	return param->Code;
}
//--------------------------------------------------------------
/**
 * @brief   レスポンスデータ解釈：バトルビデオ検索
 *
 * @param   gdsrap		
 * @param   res		
 *
 * @retval  TRUE:成功
 * @retval  FALSE:何らかのエラーが発生(内容はres->Result参照)
 */
//--------------------------------------------------------------
int GDS_RAP_RESPONSE_BattleVideo_Search_Download(GDS_RAP_WORK *gdsrap, POKE_NET_RESPONSE *res)
{
	POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH *param;
	int ret = FALSE;
	
	switch(res->Result){
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_SUCCESS:		//!< 取得成功
		param = (POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH *)(res->Param);
		OS_TPrintf("バトルビデオ検索受信成功：ヒット件数=%d\n", param->HitCount);
		ret = TRUE;
		break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_ERROR_AUTH:		//!< ユーザー認証エラー
		OS_TPrintf("バトルビデオ検索受信エラー！:ユーザー認証エラー\n");
		break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_ERROR_ILLEGALPARAM:	//!< 検索パラメータエラー
		OS_TPrintf("バトルビデオ検索受信エラー！:検索パラメータエラー\n");
		break;
  case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_ERROR_ILLEGALRANKINGTYPE: //!< 検索パラメータエラー(ランキング種別が不正)
		OS_TPrintf("バトルビデオ検索受信エラー！:ランキング種別エラー\n");
		break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_SEARCH_ERROR_UNKNOWN:	//!< その他エラー
	default:
		OS_TPrintf("バトルビデオ検索受信エラー！:その他のエラー\n");
		break;
	}

	return ret;
}

//--------------------------------------------------------------
/**
 * @brief   受信バッファからデータが入っている部分のポインタを取得：バトルビデオ検索
 *
 * @param   gdsrap		
 * @param   outline_array	ポインタをセットする配列データ
 * @param   array_max		outline_arrayの配列要素数
 *
 * @retval  受信件数
 *
 * outline_array[0] ～ outline_array[x] まで受信データへのポインタがセットされます
 */
//--------------------------------------------------------------
int GDS_RAP_RESPONSE_BattleVideoSearch_Download_RecvPtr_Set(GDS_RAP_WORK *gdsrap, BATTLE_REC_OUTLINE_RECV **outline_array, int array_max)
{
	POKE_NET_RESPONSE *res;
	POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH *param;
	POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH_RECVDATA *outline_recv;
	int i, hit_count;
	
	res = POKE_NET_GDS_GetResponse();
	param = (POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH *)(res->Param);
	
	hit_count = param->HitCount;
	if(hit_count > array_max){
		hit_count = array_max;
		GF_ASSERT_HEAVY(0);	//用意しているバッファよりも受信件数が多い
	}

	outline_recv = (POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH_RECVDATA *)(param->Data);
	for(i = 0; i < hit_count; i++){
		outline_array[i] = &outline_recv[i].Data;
		if(outline_recv[i].Code != outline_array[i]->head.data_number){
			//ヘッダとデータナンバーが違う場合はヘッダにナンバーを入れられてないのでこっちで入れる
			outline_array[i]->head.data_number = outline_recv[i].Code;
			//CRC作成
			outline_array[i]->head.crc.crc16ccitt_hash = GFL_STD_CrcCalc(
				&outline_array[i]->head, 
				sizeof(BATTLE_REC_HEADER) - GDS_CRC_SIZE - sizeof(outline_recv[i].Code));
		}
	}
	for( ; i < array_max; i++){
		outline_array[i] = NULL;
	}
	
	return hit_count;
}

//--------------------------------------------------------------
/**
 * @brief   受信バッファからデータをコピーします：バトルビデオ検索
 *
 * @param   gdsrap		
 * @param   outline_array	ポインタをセットする配列データ
 * @param   array_max		outline_arrayの配列要素数
 *
 * @retval  受信件数
 *
 * outline_array[0] ～ outline_array[x] まで受信データへのポインタがセットされます
 */
//--------------------------------------------------------------
int GDS_RAP_RESPONSE_BattleVideoSearch_Download_DataCopy(GDS_RAP_WORK *gdsrap, BATTLE_REC_OUTLINE_RECV *outline_array, int array_max)
{
	POKE_NET_RESPONSE *res;
	POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH *param;
	POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH_RECVDATA *outline_recv;
	int i, hit_count;
	
	res = POKE_NET_GDS_GetResponse();
	param = (POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH *)(res->Param);
	
	hit_count = param->HitCount;
	if(hit_count > array_max){
		hit_count = array_max;
		GF_ASSERT_HEAVY(0);	//用意しているバッファよりも受信件数が多い
	}

	GFL_STD_MemClear(outline_array, sizeof(BATTLE_REC_OUTLINE_RECV) * array_max);
	outline_recv = (POKE_NET_GDS_RESPONSE_BATTLEDATA_SEARCH_RECVDATA *)(param->Data);
	for(i = 0; i < hit_count; i++){
		if(outline_recv[i].Code != outline_recv[i].Data.head.data_number){
			//ヘッダとデータナンバーが違う場合はヘッダにナンバーを入れられてないのでこっちで入れる
			outline_recv[i].Data.head.data_number = outline_recv[i].Code;
			//CRC作成
			outline_recv[i].Data.head.crc.crc16ccitt_hash = GFL_STD_CrcCalc(
				&outline_recv[i].Data.head, 
				sizeof(BATTLE_REC_HEADER) - GDS_CRC_SIZE - sizeof(outline_recv[i].Code));
		}
		outline_array[i] = outline_recv[i].Data;
	}
	
	return hit_count;
}

//--------------------------------------------------------------
/**
 * @brief   レスポンスデータ解釈：バトルビデオデータ取得
 *
 * @param   gdsrap		
 * @param   res		
 *
 * @retval  TRUE:成功
 * @retval  FALSE:何らかのエラーが発生(内容はres->Result参照)
 */
//--------------------------------------------------------------
int GDS_RAP_RESPONSE_BattleVideo_Data_Download(GDS_RAP_WORK *gdsrap, POKE_NET_RESPONSE *res)
{
	POKE_NET_GDS_RESPONSE_BATTLEDATA_GET *param;
	int ret = FALSE;
	
	switch(res->Result){
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_SUCCESS:			//!< 取得成功
		param = (POKE_NET_GDS_RESPONSE_BATTLEDATA_GET *)(res->Param);
		OS_TPrintf("バトルビデオデータ取得受信成功\n");
		ret = TRUE;
		break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_ERROR_AUTH:		//!< ユーザー認証エラー
		OS_TPrintf("バトルビデオデータ取得受信エラー！:ユーザー認証エラー\n");
		break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_ERROR_ILLEGALCODE:	//!< コードエラー
		OS_TPrintf("バトルビデオデータ取得受信エラー！:コードエラー\n");
		break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_GET_ERROR_UNKNOWN:		//!< その他エラー
	default:
		OS_TPrintf("バトルビデオデータ取得受信エラー！:その他のエラー\n");
		break;
	}

	return ret;
}

//--------------------------------------------------------------
/**
 * @brief   受信バッファからデータが入っている部分のポインタを取得：バトルビデオデータ取得
 *
 * @param   gdsrap		
 * @param   rec			録画データが入っているデータへのポインタ代入先
 *
 * @retval  登録コード
 */
//--------------------------------------------------------------
int GDS_RAP_RESPONSE_BattleVideoData_Download_RecvPtr_Set(GDS_RAP_WORK *gdsrap, BATTLE_REC_RECV **rec)
{
	POKE_NET_RESPONSE *res;
	POKE_NET_GDS_RESPONSE_BATTLEDATA_GET *param;
	
	res = POKE_NET_GDS_GetResponse();
	param = (POKE_NET_GDS_RESPONSE_BATTLEDATA_GET *)(res->Param);
	
	*rec = &param->Data;
	if(param->Code != param->Data.head.data_number){
		//ヘッダとデータナンバーが違う場合はヘッダにナンバーを入れられてないのでこっちで入れる
		param->Data.head.data_number = param->Code;
		//CRC作成
		(*rec)->head.crc.crc16ccitt_hash = GFL_STD_CrcCalc(&(*rec)->head, 
			sizeof(BATTLE_REC_HEADER) - GDS_CRC_SIZE - sizeof(param->Code));
	}
	
	return param->Code;
}

//--------------------------------------------------------------
/**
 * @brief   レスポンスデータ解釈：バトルビデオお気に入り登録
 *
 * @param   gdsrap		
 * @param   res		
 *
 * @retval  TRUE:成功
 * @retval  FALSE:何らかのエラーが発生(内容はres->Result参照)
 */
//--------------------------------------------------------------
int GDS_RAP_RESPONSE_BattleVideo_Favorite_Upload(GDS_RAP_WORK *gdsrap, POKE_NET_RESPONSE *res)
{
//	POKE_NET_GDS_RESPONSE_BATTLEDATA_FAVORITE *param;
	int ret = FALSE;
	
	switch(res->Result){
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_SUCCESS:			//!< お気に入り登録成功
//		param = (POKE_NET_GDS_RESPONSE_BATTLEDATA_FAVORITE *)(res->Param);
		OS_TPrintf("バトルビデオデータお気に入り登録受信成功\n");
		ret = TRUE;
		break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_ERROR_AUTH:		//!< ユーザー認証エラー
		OS_TPrintf("バトルビデオお気に入り登録受信エラー！:ユーザー認証エラー\n");
		break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_ERROR_ILLEGALCODE:	//!< コードエラー
		OS_TPrintf("バトルビデオお気に入り登録受信エラー！:コードエラー\n");
		break;
	case POKE_NET_GDS_RESPONSE_RESULT_BATTLEDATA_FAVORITE_ERROR_UNKNOWN:	//!< その他エラー
	default:
		OS_TPrintf("バトルビデオお気に入り登録受信エラー！:その他のエラー\n");
		break;
	}

	return ret;
}

//--------------------------------------------------------------
/**
 * @brief   受信バッファからデータが入っている部分のポインタを取得：バトルビデオお気に入り登録
 *
 * @param   gdsrap		
 *
 * @retval  登録コード
 */
//--------------------------------------------------------------
void GDS_RAP_RESPONSE_BattleVideoFavorite_Upload_RecvPtr_Set(GDS_RAP_WORK *gdsrap)
{
	//現状、取得するものは何も無い
	return;
}
