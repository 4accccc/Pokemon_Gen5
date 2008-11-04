//======================================================================
/**
 *
 * @file	dlplay_child_common.h
 * @brief	セーブデータがらみの共通ファイル
 *			バージョン別コードと、dlplay_data_main以外で読むな
 * @author	ariizumi
 * @data	
 */
//======================================================================

#ifndef DLPLAY_CHILD_COMMON_H_
#define DLPLAY_CHILD_COMMON_H_

enum DLPLAY_DATA_SAVEPOS
{
	DDS_FIRST,	//1番データ
	DDS_SECOND,	//2番データ
	DDS_ERROR,	//エラー
};

struct _DLPLAY_DATA_DATA
{
	int	heapID_;
	u8	mainSeq_;
	u8	subSeq_;
	u8	errorState_;

	s32	lockID_;	//カードロック用ID
	MATHCRC16Table	crcTable_;	//CRCチェック用テーブル

	DLPLAY_CARD_TYPE	cardType_;	//本体に刺さっているカードの種類
	u8					boxSavePos_;	//ボックスデータの新しい方
	u8					mainSavePos_;	//メインデータの新しい方

	DLPLAY_MSG_SYS	*msgSys_;	//上から渡して共用
	u8	*pData_;	//読み出したデータ
	u8	*pDataMirror_;	//ミラー用データ
	u8	*pBoxData_;		//読み込み対象のBOXデータアドレス(まだポケモンデータじゃない！
};	//DLPLAY_DATAのDATA・・・ややこしい・・・


#endif //DLPLAY_CHILD_COMMON_H_

