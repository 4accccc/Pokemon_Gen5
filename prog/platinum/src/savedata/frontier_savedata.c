//==============================================================================
/**
 * @file	ファイル名
 * @brief	簡単な説明を書く
 * @author	matsuda
 * @date	2007.04.26(木)
 */
//==============================================================================
#include "common.h"
#include "savedata/savedata_def.h"
#include "savedata/savedata.h"

#include "system/gamedata.h"
#include "system/pms_data.h"
#include "system/buflen.h"
#include "system/msgdata.h"
#include "battle/battle_common.h"
#include "battle/b_tower_data.h"

#include "savedata/frontier_savedata.h"
#include "frontier_savedata_local.h"

#include "b_tower_local.h"

#include "savedata/b_tower.h"
#include "savedata/factory_savedata.h"
#include "savedata/stage_savedata.h"
#include "savedata/castle_savedata.h"
#include "savedata/roulette_savedata.h"
#include "field/b_tower_scr_def.h"
#include "libdpw/dpw_bt.h"

#include "msgdata/msg.naix"
#include "msgdata/msg_btower_app.h"



//--------------------------------------------------------------
/**
 * @brief   タワー　セーブデータブロックサイズを取得
 *
 * @retval  セーブデータブロックサイズ
 */
//--------------------------------------------------------------
int FrontierData_GetWorkSize(void)
{
	return sizeof(FRONTIER_SAVEWORK);
}

//--------------------------------------------------------------
/**
 * @brief   フロンティア　セーブデータブロック初期化
 *
 * @param   save		
 */
//--------------------------------------------------------------
void FrontierData_Init(FRONTIER_SAVEWORK* save)
{
	//共通ワーク初期化
//	TowerPlayData_Clear(&save->play);
	MI_CpuClear8(save, sizeof(FRONTIER_SAVEWORK));
	
	//バトルタワー初期化
	TowerScoreData_Clear(&save->tower.score);
	TowerPlayerMsg_Clear(&save->tower.player_msg);
	TowerWifiData_Clear(&save->tower.wifi);
	
	//ファクトリー初期化
	FACTORYSCORE_Init(&save->factory.score);	

	//ステージ初期化
	STAGESCORE_Init(&save->stage.score);	

	//キャッスル初期化
	CASTLESCORE_Init(&save->castle.score);	

	//ルーレット初期化
	ROULETTESCORE_Init(&save->roulette.score);	

#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
	SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
	return;
}

//--------------------------------------------------------------
/**
 * @brief   フロンティアのセーブデータポインタ取得
 *
 * @param   sv		
 *
 * @retval  フロンティアセーブデータポインタ
 */
//--------------------------------------------------------------
FRONTIER_SAVEWORK * SaveData_GetFrontier(SAVEDATA * sv)
{
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
	SVLD_CheckCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
	return SaveData_Get(sv, GMDATA_ID_FRONTIER);
}



//==============================================================================
//	フロンティアレコード
//==============================================================================
#if 1	//配列にした

//--------------------------------------------------------------
/**
 * @brief   フロンティアレコードデータ取得
 *
 * @param   save			
 * @param   id				取り出すデータID(FRID_???)
 * @param   friend_no		友達手帳番号(友達データでない場合はFRONTIER_RECORD_NOT_FRIEND指定)
 *
 * @retval  レコードデータ
 */
//--------------------------------------------------------------
u16 FrontierRecord_Get(FRONTIER_SAVEWORK* save, int id, int friend_no)
{
	FRONTIER_RECORD_SAVEWORK *frs = &save->frontier_record;
	
	if(id < FRID_MAX){
		//GF_ASSERT(friend_no == FRONTIER_RECORD_NOT_FRIEND);
		if(id >= FRID_TOWER_MULTI_WIFI_CLEAR_BIT){
			int access_id, shift_num;
			
			if(friend_no < 16){
				access_id = id;
				shift_num = friend_no;
			}
			else{
				access_id = id + 1;
				shift_num = friend_no - 16;
			}
			return (frs->record[access_id] >> friend_no) & 1;
		}
		return frs->record[id];
	}
	else{	//友達手帳毎のデータ
		//GF_ASSERT(friend_no != FRONTIER_RECORD_NOT_FRIEND);
		
		if( friend_no == -1 ){
			OS_Printf( "friend_no = -1 まだ接続していないので友達番号を取得出来ません！\n" );
			GF_ASSERT(0);
			return 0;
		}
		return frs->wififriend_record[friend_no][id - FRID_MAX];
	}
}

//--------------------------------------------------------------
/**
 * @brief   フロンティアレコードデータセット
 *
 * @param   save		
 * @param   id			データID(FRID_???)
 * @param   friend_no	友達手帳番号(友達データでない場合はFRONTIER_RECORD_NOT_FRIEND指定)
 * @param   data		セットするデータ
 *
 * @retval	セット後の値
 */
//--------------------------------------------------------------
u16 FrontierRecord_Set(FRONTIER_SAVEWORK* save, int id, int friend_no, u16 data)
{
	FRONTIER_RECORD_SAVEWORK *frs = &save->frontier_record;

	if(data > FRONTIER_RECORD_LIMIT){
		data = FRONTIER_RECORD_LIMIT;
	}
	
	if(id < FRID_MAX){
		if(id >= FRID_TOWER_MULTI_WIFI_CLEAR_BIT){
			int access_id, shift_num;

			GF_ASSERT(friend_no != FRONTIER_RECORD_NOT_FRIEND);
			
			if(friend_no < 16){
				access_id = id;
				shift_num = friend_no;
			}
			else{
				access_id = id + 1;
				shift_num = friend_no - 16;
			}
			
			if(data == 0){
				frs->record[access_id] &= 0xffff ^ (1 << shift_num);
			}
			else{
				frs->record[access_id] |= 1 << shift_num;
			}
		}
		else{
			GF_ASSERT(friend_no == FRONTIER_RECORD_NOT_FRIEND);
			frs->record[id] = data;
		}
	}
	else{	//友達手帳毎のデータ
		GF_ASSERT(friend_no != FRONTIER_RECORD_NOT_FRIEND);
		frs->wififriend_record[friend_no][id - FRID_MAX] = data;
	}
	
#if (CRC_LOADCHECK && CRCLOADCHECK_GMDATA_ID_FRONTIER)
	SVLD_SetCrc(GMDATA_ID_FRONTIER);
#endif //CRC_LOADCHECK
	return data;
}

//--------------------------------------------------------------
/**
 * @brief   フロンティアレコードデータ 加算
 *
 * @param   save		
 * @param   id			取り出すデータID(FRID_???)
 * @param   friend_no	友達手帳番号(友達データでない場合はFRONTIER_RECORD_NOT_FRIEND指定)
 * @param   add_value	加算値
 * 
 * @retval	更新された値
 */
//--------------------------------------------------------------
u16 FrontierRecord_Add(FRONTIER_SAVEWORK* save, int id, int friend_no, int add_value)
{
	FRONTIER_RECORD_SAVEWORK *frs = &save->frontier_record;
	u16 data;
	
	data = FrontierRecord_Get(save, id, friend_no);
	data += add_value;
	return FrontierRecord_Set(save, id, friend_no, data);
}

//--------------------------------------------------------------
/**
 * @brief   フロンティアレコードデータ 減算
 *
 * @param   save		
 * @param   id			取り出すデータID(FRID_???)
 * @param   friend_no	友達手帳番号(友達データでない場合はFRONTIER_RECORD_NOT_FRIEND指定)
 * @param   sub_value	減算値
 * 
 * @retval	更新された値
 */
//--------------------------------------------------------------
u16 FrontierRecord_Sub(FRONTIER_SAVEWORK* save, int id, int friend_no, int sub_value)
{
	FRONTIER_RECORD_SAVEWORK *frs = &save->frontier_record;
	int data;
	
	data = FrontierRecord_Get(save, id, friend_no);
	data -= sub_value;
	if(data < 0){
		data = 0;
	}
	return FrontierRecord_Set(save, id, friend_no, data);
}

//--------------------------------------------------------------
/**
 * @brief   フロンティアレコードデータ インクリメント
 *
 * @param   save		
 * @param   id			取り出すデータID(FRID_???)
 * @param   friend_no	友達手帳番号(友達データでない場合はFRONTIER_RECORD_NOT_FRIEND指定)
 * 
 * @retval	更新された値
 */
//--------------------------------------------------------------
u16 FrontierRecord_Inc(FRONTIER_SAVEWORK* save, int id, int friend_no)
{
	return FrontierRecord_Add(save, id, friend_no, 1);
}

//--------------------------------------------------------------
/**
 * @brief   大きければレコードを更新する
 *
 * @param   save		
 * @param   id			データID(FRID_???)
 * @param   friend_no	友達手帳番号(友達データでない場合はFRONTIER_RECORD_NOT_FRIEND指定)
 * @param   data		セットするデータ
 *
 * @retval  結果の値
 */
//--------------------------------------------------------------
u16 FrontierRecord_SetIfLarge(FRONTIER_SAVEWORK *save, int id, int friend_no, u16 data)
{
	u16 now;
	
	now = FrontierRecord_Get(save, id, friend_no);

	if(now < data){
		return FrontierRecord_Set(save, id, friend_no, data);
	} else {
		if(now > FRONTIER_RECORD_LIMIT){	//現在値がリミットオーバーしていた時の訂正処理
			return FrontierRecord_Set(save, id, friend_no, FRONTIER_RECORD_LIMIT);
		}
		return now;
	}
}

#else

いっそのこと構造体ではなく配列にしてしまうか？
Local系は_LOCAL_TOWERのsizeofとMULTI_WIFI_RENSHOUのインデックスを渡せば
全施設共通で使えるようになる
static u16 * Local_TowerDataAdrsGet(FRONTIER_RECORD_PTR frp, int id)
{
	u16 *tower;
	tower = (u16*)(&frp->tower);
	if(id < FRID_TOWER_MULTI_WIFI_RENSHOU){
		ret = tower[id];
	}
	else{
		ret = tower[id + sizeof(_LOCAL_TOWER) * friend_no];
	}
	return ret;
}

int FrontierRecord_Get(FRONTIER_RECORD_PTR frp, int id, int friend_no)
{
	FRONTIER_RECORD_TOWER *tower;
	int ret;
	
	if(id >= FRID_TOWER_START && id <= FRID_TOWER_END){
		//タワー
		u16 *tower;
		tower = (u16*)(&frp->tower);
		if(id < FRID_TOWER_MULTI_WIFI_RENSHOU){
			ret = tower[id];
		}
		else{
			ret = tower[id + sizeof(_LOCAL_TOWER) * friend_no];
		}
		return ret;
	}
	else if(id >= FRID_FACTORY_START && id <= FRID_FACTORY_END){
		//ファクトリー
		u16 *factory;
		factory = (u16*)(&frp->factory);
		if(id < FRID_FACTORY_MULTI_WIFI_RENSHOU){
			ret = factory[id];
		}
		else{
			ret = factory[id + sizeof(_LOCAL_FACTORY) * friend_no];
		}
		return ret;
	}
	else if(id >= FRID_STAGE_START && id <= FRID_STAGE_END){
		//ステージ
	}
	else{
		GF_ASSERT(0);	//不明なID
		return 0;
	}
	
	GF_ASSERT(id >= FRID_TOWER_RENSHOU_SINGLE && id <= FRID_TOWER_WIFI_DL_CNT);
	
	tower = &frp->tower;
	if(id < FRID_TOWER_WIFI_RENSHOU){
		ret = ((u16*)tower)[id];
	}
	else{
		ret = ((u16*)tower)[id + 8 * friend_no];
	}
	
	return ret;
}

int FrontierRecord_FactoryGet(FRONTIER_RECORD_PTR frp, int id, int friend_no)
{
#if 1	//コード量を減らす為に計算で出す
	FRONTIER_RECORD_FACTORY *factory;
	int ret;
	
	GF_ASSERT(id >= FRID_FACTORY_RENSHOU_SINGLE && id <= FRID_FACTORY100_TRADE_COMM_MULTI_CNT);
	
	factory = &frp->factory;
	if(id < FRID_FACTORY_WIFI_RENSHOU){
		ret = ((u16*)factory)[id];
	}
	else{
		ret = ((u16*)factory)[id + 8 * friend_no];
	}
	
	return ret;
#else
	switch(id){
	case FRID_FACTORY_RENSHOU_SINGLE:
		break;
	case FRID_FACTORY_RENSHOU_SINGLE_CNT:
		break;
	case FRID_FACTORY_TRADE_SINGLE:
		break;
	case FRID_FACTORY_TRADE_SINGLE_CNT:
		break;

	case FRID_FACTORY_RENSHOU_DOUBLE:
		break;
	case FRID_FACTORY_RENSHOU_DOUBLE_CNT:
		break;
	case FRID_FACTORY_TRADE_DOUBLE:
		break;
	case FRID_FACTORY_TRADE_DOUBLE_CNT:
		break;
	case FRID_FACTORY_RENSHOU_COMM_MULTI:
		break;
	case FRID_FACTORY_RENSHOU_COMM_MULTI_CNT:
		break;
	case FRID_FACTORY_TRADE_COMM_MULTI:
		break;
	case FRID_FACTORY_TRADE_COMM_MULTI_CNT:
		break;

	case FRID_FACTORY100_RENSHOU_SINGLE:
		break;
	case FRID_FACTORY100_RENSHOU_SINGLE_CNT:
		break;
	case FRID_FACTORY100_TRADE_SINGLE:
		break;
	case FRID_FACTORY100_TRADE_SINGLE_CNT:
		break;

	case FRID_FACTORY100_RENSHOU_DOUBLE:
		break;
	case FRID_FACTORY100_RENSHOU_DOUBLE_CNT:
		break;
	case FRID_FACTORY100_TRADE_DOUBLE:
		break;
	case FRID_FACTORY100_TRADE_DOUBLE_CNT:
		break;

	case FRID_FACTORY100_RENSHOU_COMM_MULTI:
		break;
	case FRID_FACTORY100_RENSHOU_COMM_MULTI_CNT:
		break;
	case FRID_FACTORY100_TRADE_COMM_MULTI:
		break;
	case FRID_FACTORY100_TRADE_COMM_MULTI_CNT:
		break;
	
	default:
		GF_ASSERT(0);
		return 0;
	}
#endif
}
#endif

