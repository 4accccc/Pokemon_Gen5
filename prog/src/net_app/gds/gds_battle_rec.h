//==============================================================================
/**
 *
 * 戦闘録画
 *
 */
//==============================================================================
#ifndef __GT_GDS_BATTLE_REC_H__
#define __GT_GDS_BATTLE_REC_H__


#include "gds_define.h"
#include "gds_profile_local.h"
#include "gds_battle_rec_sub.h"

#include "pm_define.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_tool_def.h"
#include "savedata/gds_profile.h"
#include "net_app/gds/gds_profile_local.h"
#include "battle/btl_common.h"
#include "savedata\battle_rec.h"
#include "savedata\battle_rec_local.h"
#include "gds_battle_rec.h"
#include "gds_ranking.h"
#include "gds_boxshot.h"
#include "gds_dressup.h"


//----------------------------------------------------------
/**
 *	録画セーブデータ本体（6116byte）
 */
//----------------------------------------------------------
typedef struct {
  BTLREC_SETUP_SUBSET       setupSubset;   ///< バトル画面セットアップパラメータのサブセット
  BTLREC_OPERATION_BUFFER   opBuffer;      ///< クライアント操作内容の保存バッファ

  REC_POKEPARTY         rec_party[ BTL_CLIENT_MAX ];
  BTLREC_CLIENT_STATUS  clientStatus[ BTL_CLIENT_MAX ];

  u16 magic_key;
  u16 padding;
  
  //u32         checksum; //チェックサム  4byte
  GDS_CRC crc;              ///< CRC            4
}GT_BATTLE_REC_WORK;

//--------------------------------------------------------------
/**
 *	戦闘録画のヘッダ
 *		68byte
 */
//--------------------------------------------------------------
typedef struct {
	u16 monsno[GT_HEADER_MONSNO_MAX];	///<ポケモン番号(表示する必要がないのでタマゴの場合は0)	24
	u8 form_no_and_sex[GT_HEADER_MONSNO_MAX];	///<6..0bit目：ポケモンのフォルム番号　7bit目：ポケモンの性別		12

  u16 battle_counter;   ///<連勝数                        2
  u16 mode;        ///<戦闘モード(ファクトリー50、ファクトリー100、通信対戦...)
  
  u16 magic_key;      ///<マジックキー
  u8 secure;        ///<TRUE:安全が保障されている。　FALSE：再生した事がない
  
  u8 server_vesion;   ///<バトルのサーバーバージョン
  u8 work[12];      ///< 予備
	
	GT_DATA_NUMBER data_number;		///<データナンバー(サーバー側でセットされる)	8
	GT_GDS_CRC crc;							///< CRC						4
}GT_BATTLE_REC_HEADER;

//--------------------------------------------------------------
/**
 *	送信データ：戦闘録画
 *	7500byte
 */
//--------------------------------------------------------------
typedef struct {
	GT_GDS_PROFILE profile;		///<GDSプロフィール		88
	GT_BATTLE_REC_HEADER head;		///<戦闘録画のヘッダ	100
	GT_BATTLE_REC_WORK rec;		///<録画データ本体			7272
}GT_BATTLE_REC_SEND;

//--------------------------------------------------------------
/**
 *	送信データ：検索リクエスト
 *	16byte
 */
//--------------------------------------------------------------
typedef struct{
	u16 monsno;					///< ポケモン番号
	u8 battle_mode;				///< 検索施設
	u8 country_code;			///< 住んでいる国コード
	u8 local_code;				///< 住んでいる地方コード
	
	u8 padding[3];				//パディング
	
	u32 server_version;			///< ROMのサーバーバージョン
	
	u8 work[4];					///<予備
}GT_BATTLE_REC_SEARCH_SEND;

//--------------------------------------------------------------
/**
 *	送信データ：ランキング取得
 */
//--------------------------------------------------------------
typedef struct{
	u32 server_version;			///< ROMのサーバーバージョン
}GT_BATTLE_REC_RANKING_SEARCH_SEND;


//--------------------------------------------------------------
/**
 *	受信データ：概要一覧取得時
 *	228byte
 */
//--------------------------------------------------------------
typedef struct {
	GT_GDS_PROFILE profile;		///<GDSプロフィール
	GT_BATTLE_REC_HEADER head;		///<戦闘録画のヘッダ
}GT_BATTLE_REC_OUTLINE_RECV;

//--------------------------------------------------------------
/**
 *	受信データ：一件の検索結果取得時
 *	7500byte
 */
//--------------------------------------------------------------
typedef struct{
	GT_GDS_PROFILE profile;		///<GDSプロフィール
	GT_BATTLE_REC_HEADER head;		///<戦闘録画のヘッダ
	GT_BATTLE_REC_WORK rec;		///<戦闘録画本体
}GT_BATTLE_REC_RECV;


#endif	//__GT_GDS_BATTLE_REC_H__

