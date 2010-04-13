//==============================================================================
/**
 * @file	battle_rec_ov.c
 * @brief	オーバーレイbattle_recorderでのみ使用する録画関連ツール類
 * @author	matsuda
 * @date	2008.05.15(木)
 *
 * 本来ならbattle_rec.cに含めるものだが、常駐メモリの関係上、バトルレコーダーでしか
 * 使用しないようなものは、ここに移動
 */
//==============================================================================
#include <gflib.h>
#include "savedata\save_control.h"
#include "savedata\mystatus_local.h"
#include "savedata\config.h"

#include "gds_local_common.h"
#include "savedata\battle_rec.h"
#include "waza_tool/wazano_def.h"
#include "net_app/gds/gds_profile_local.h"
#include "savedata/gds_profile.h"

#include "pm_define.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_tool_def.h"
#include "savedata/gds_profile.h"
#include "net_app/gds/gds_profile_local.h"
#include "battle/btl_common.h"
#include "savedata\battle_rec.h"
#include "savedata\battle_rec_local.h"

#include "savedata/misc.h"



//==============================================================================
//	プロトタイプ宣言
//==============================================================================
//static void ErrorNameSet(STRBUF *dest_str, int heap_id);

//==============================================================================
//	
//==============================================================================
extern BATTLE_REC_SAVEDATA * brs;


//==============================================================================
//	
//==============================================================================

//--------------------------------------------------------------
/**
 * @brief   GDSで受信したデータをセーブする
 *
 * @param   sv				セーブデータへのポインタ
 * @param   recv_data		BATTLE_REC_RECV構造体
 * @param   num				LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…
 * @param   secure		視聴済みフラグ(TRUE:視聴済みにしてセーブする。
 * 						FALSE:フラグ操作しない(サーバーでsecureを立てているなら立ったままになる)
 * @param   work0		セーブ進行を制御するワークへのポインタ(最初は0クリアした状態で呼んで下さい)
 * @param   work1		セーブ進行を制御するワークへのポインタ(最初は0クリアした状態で呼んで下さい)
 *
 * @retval  SAVE_RESULT_CONTINUE	セーブ処理継続中
 * @retval  SAVE_RESULT_LAST		セーブ処理継続中、最後の一つ前
 * @retval  SAVE_RESULT_OK			セーブ正常終了
 * @retval  SAVE_RESULT_NG			セーブ失敗終了
 *
 * GDSで受信したデータはbrsにコピーした時点(BattleRec_DataSet)で復号されているので、
 * 内部で暗号化してからセーブを行う
 */
//--------------------------------------------------------------
SAVE_RESULT BattleRec_GDS_RecvData_Save(GAMEDATA *gamedata, int num, u8 secure, u16 *work0, u16 *work1, HEAPID heap_id)
{
	SAVE_RESULT result;
	
	switch(*work0){
	case 0:
		GF_ASSERT(brs);
		
		if(secure == TRUE){
			brs->head.secure = secure;
			//secureフラグが更新されたので、再度CRCを作り直す
			brs->head.magic_key = REC_OCC_MAGIC_KEY;
			brs->head.crc.crc16ccitt_hash = GFL_STD_CrcCalc(&brs->head, 
				sizeof(BATTLE_REC_HEADER) - GDS_CRC_SIZE - DATANUMBER_SIZE);
		}
		
		//CRCをキーにして暗号化
		//※GDSで受信したデータはbrsにコピーした時点(BattleRec_DataSet)で復号されているので。
		BattleRec_Coded(&brs->rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE, 
			brs->rec.crc.crc16ccitt_hash + ((brs->rec.crc.crc16ccitt_hash ^ 0xffff) << 16));
		(*work0)++;
		break;
	case 1:
		result = Local_BattleRecSave(gamedata, brs, num, work1, heap_id);
		return result;
	}
	return SAVE_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   GDSで送信する為に、データに変換処理を行う
 *
 * @param   sv		セーブデータへのポインタ
 *
 * この関数使用後のbrsは、このまま再生したり保存してはいけない形になっているので、
 * GDSで送信し終わったら破棄すること！
 * 但し、BattleRec_GDS_MySendData_DataNumberSetSave関数を使用する場合はOK。
 */
//--------------------------------------------------------------
void BattleRec_GDS_SendData_Conv(SAVE_CONTROL_WORK *sv)
{
	GF_ASSERT(brs);
	
	//※brsに展開されているデータは、本体が複合化されているので送信前に再度暗号化を行う
	BattleRec_Coded(&brs->rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE, 
		brs->rec.crc.crc16ccitt_hash + ((brs->rec.crc.crc16ccitt_hash ^ 0xffff) << 16));
}

//--------------------------------------------------------------
/**
 * @brief   GDSで送信したbrsに対して、データナンバーをセットしセーブを行う
 *
 * @param   sv				セーブデータへのポインタ
 * @param   data_number		データナンバー
 * @param   work		セーブ進行を制御するワークへのポインタ(最初は0クリアした状態で呼んで下さい)
 * @param   work		セーブ進行を制御するワークへのポインタ(最初は0クリアした状態で呼んで下さい)
 * 
 * @retval	SAVE_RESULT_OK		セーブ成功
 * @retval	SAVE_RESULT_NG		セーブ失敗
 */
//--------------------------------------------------------------
SAVE_RESULT BattleRec_GDS_MySendData_DataNumberSetSave(GAMEDATA *gamedata, u64 data_number, u16 *work0, u16 *work1, HEAPID heap_id)
{
	SAVE_RESULT result;

	switch(*work0){
	case 0:
		GF_ASSERT(brs);
		
		//データナンバーは元々CRCの対象からはずれているので、CRCを再作成する必要はない
		brs->head.data_number = data_number;
		(*work0)++;
		break;
	case 1:
		//送信時に行っているBattleRec_GDS_SendData_Conv関数で既に暗号化しているはずなので
		//そのままセーブを行う
		result = Local_BattleRecSave(gamedata, brs, LOADDATA_MYREC, work1, heap_id);
		return result;
	}
	return SAVE_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief   バトルレコーダー(オフライン)でビデオを見た後、brsに読み込んでいるバトルビデオに
 * 			視聴済みフラグをセットしてセーブする
 * 			※通常セーブ込み
 *
 * @param   sv		セーブデータへのポインタ
 * @param	num		セーブするデータナンバー（LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…）
 * @param   work		セーブ進行を制御するワークへのポインタ(最初は0クリアした状態で呼んで下さい)
 * @param   work		セーブ進行を制御するワークへのポインタ(最初は0クリアした状態で呼んで下さい)
 *
 * @retval  SAVE_RESULT_???
 *
 * 内部で本体の暗号化処理を行う為、この関数以後はbrsは解放してください。
 */
//--------------------------------------------------------------
SAVE_RESULT BattleRec_SecureSetSave(GAMEDATA *gamedata, int num, u16 *work0, u16 *work1, HEAPID heap_id)
{
	SAVE_RESULT result;

  switch(*work0){
  case 0:
  	GF_ASSERT(brs != NULL);
  	
  	brs->head.secure = TRUE;
  	//secureフラグが更新されたので、再度CRCを作り直す
  	brs->head.magic_key = REC_OCC_MAGIC_KEY;
  	brs->head.crc.crc16ccitt_hash = GFL_STD_CrcCalc(&brs->head, 
  		sizeof(BATTLE_REC_HEADER) - GDS_CRC_SIZE - DATANUMBER_SIZE);

  	//CRCをキーにして暗号化
  	BattleRec_Coded(&brs->rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE, 
  		brs->rec.crc.crc16ccitt_hash + ((brs->rec.crc.crc16ccitt_hash ^ 0xffff) << 16));
    (*work0)++;
    break;
  case 1:
	  result = Local_BattleRecSave(gamedata, brs, num, work1, heap_id);
	  return result;
	}
	return SAVE_RESULT_CONTINUE;
}



#ifdef PM_DEBUG
//--------------------------------------------------------------
/**
 * @brief   読み込み済みのbrsに対してsecureフラグをセットする
 *
 * @param   sv		
 */
//--------------------------------------------------------------
void DEBUG_BattleRec_SecureFlagSet(SAVE_CONTROL_WORK *sv)
{
	GF_ASSERT(brs != NULL);
	
	brs->head.secure = TRUE;
	//secureフラグが更新されたので、再度CRCを作り直す
	brs->head.magic_key = REC_OCC_MAGIC_KEY;
	brs->head.crc.crc16ccitt_hash = GFL_STD_CrcCalc(&brs->head, 
		sizeof(BATTLE_REC_HEADER) - GDS_CRC_SIZE - DATANUMBER_SIZE);
}
#endif	//PM_DEBUG
