//==============================================================================
/**
 * @file    mystery_data_local.h
 * @brief   不思議な贈り物ローカルヘッダ
 * @author  matsuda
 * @date    2010.04.11(日)
 *
 * ここに色々な解説等を書いてもよい
 *
 */
//==============================================================================
#pragma once


//------------------------------------------------------------------
/**
 * @brief	ふしぎデータの定義
 */
//------------------------------------------------------------------
///管理外不思議データで所持できるカード数
#define OUTSIDE_MYSTERY_MAX     (3)

typedef struct{
  u16 crc16ccitt_hash;  ///<CRC
  u16 coded_number;   ///<暗号化キー
}RECORD_CRC;


typedef struct {
  u8 recv_flag[MYSTERY_DATA_MAX_EVENT / 8];		//256 * 8 = 2048 bit
  GIFT_PACK_DATA card[GIFT_DATA_MAX];			// カード情報
  RECORD_CRC crc;   //CRC & 暗号化キー   4 byte
}MYSTERY_ORIGINAL_DATA;

///管理外セーブ：不思議な贈り物
struct _OUTSIDE_MYSTERY {
  u8 recv_flag[MYSTERY_DATA_MAX_EVENT / 8];		//256 * 8 = 2048 bit
  GIFT_PACK_DATA card[OUTSIDE_MYSTERY_MAX];		// カード情報
  RECORD_CRC crc;   //CRC & 暗号化キー   4 byte
};
