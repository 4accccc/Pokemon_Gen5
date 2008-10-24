//======================================================================
/**
 *
 * @file	dlplay_child_pt.c	
 * @brief	wbダウンロードプレイ　プラチナ用機種別コード
 * @author	ariizumi
 * @data	08/10/23
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "pt_save.h"

#include "../../pokemon_wb/prog/src/test/ariizumi/ari_debug.h"
#include "../../pokemon_wb/prog/src/test/dlplay/dlplay_func.h"
#include "../../pokemon_wb/prog/src/test/dlplay/dlplay_comm_func.h"

#include "dlplay_data_main.h"
#include "dlplay_child_common.h"

#include "pt_save.dat"	//dlplay_child_pt.c 以外で読むな

#define PT_SAVE_SIZE (SECTOR_SIZE * SECTOR_MAX)
//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================

//======================================================================
//	proto
//======================================================================

BOOL	DLPlayData_PT_LoadData( DLPLAY_DATA_DATA *d_data );
u32		DLPlayData_PT_GetStartAddress( const PT_GMDATA_ID id );

void	PT_CalcTool_Decoded(void *data,u32 size,u32 code);
void	PT_CalcTool_Coded(void *data,u32 size,u32 code);
static	u16 PT_CodeRand(u32 *code);
static	u16	PT_PokeParaCheckSum(void *data,u32	size);
static	void	*PokeParaAdrsGet(PT_POKEMON_PARAM *ppp,u32 rnd,u8 id);

//	データの読み込み
BOOL	DLPlayData_PT_LoadData( DLPLAY_DATA_DATA *d_data )
{
	switch( d_data->subSeq_ )
	{
	case 0:	//初期化およびデータ読み込み
		{

		char str[256];
		sprintf(str,"Start load data with PT size:[%d]",PT_SAVE_SIZE);
		DLPlayFunc_PutString(str,d_data->msgSys_ );

		d_data->pData_		= GFL_HEAP_AllocClearMemory(  d_data->heapID_, PT_SAVE_SIZE );
		d_data->pDataMirror_= GFL_HEAP_AllocClearMemory(  d_data->heapID_, PT_SAVE_SIZE );
	
		d_data->lockID_ = OS_GetLockID();
		GF_ASSERT( d_data->lockID_ != OS_LOCK_ID_ERROR );
		//プラチナは4MBフラッシュ
		CARD_IdentifyBackup( CARD_BACKUP_TYPE_FLASH_4MBITS );
		CARD_LockBackup( (u16)d_data->lockID_ );
		CARD_ReadFlashAsync( 0x0000 , d_data->pData_ , PT_SAVE_SIZE , NULL , NULL );
		d_data->subSeq_++;
		}
		break;
	case 1:	//ミラーリングデータ読み込み
		//読み込みの完了を待つ
		if( CARD_TryWaitBackupAsync() == TRUE )
		{
			GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
			DLPlayFunc_PutString("Data1 load complete.",d_data->msgSys_ );
			CARD_ReadFlashAsync( 0x40000 , d_data->pDataMirror_ ,PT_SAVE_SIZE, NULL , NULL );
			d_data->subSeq_++;
		}

		break;
	case 2:	//読み込みの完了を待つ
		if( CARD_TryWaitBackupAsync() == TRUE )
		{
			GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
			DLPlayFunc_PutString("Data2 load complete.",d_data->msgSys_ );
			CARD_UnlockBackup( (u16)d_data->lockID_ );
			OS_ReleaseLockID( (u16)d_data->lockID_ );
			/*
			{
				//ここからCRCチェック
				const u16 crc1 = MATH_CalcCRC16CCITT( &d_data->crcTable_ , d_data->pData_ , PT_SAVE_SIZE );
				const u16 crc2 = MATH_CalcCRC16CCITT( &d_data->crcTable_ , d_data->pDataMirror_ , PT_SAVE_SIZE );

				{
					char str[128];
					sprintf(str,"Check crc [%d][%d]",crc1,crc2);
					DLPlayFunc_PutString(str,d_data->msgSys_);
				}
			//	GF_ASSERT(crc1==crc2);
			}
			*/
			d_data->subSeq_++;
		}
		break;
	case 3:	//各ブロックのCRCチェック
		{
			//各ブロックのCRCをチェック(一応通常データも見ておく
			u8 i;
			BOOL isCorrect[4];	//データ成否チェック
			const u32 boxStartAdd = DLPlayData_PT_GetStartAddress(GMDATA_ID_BOXDATA);
			SAVE_FOOTER *footer[4];
			footer[0] = (SAVE_FOOTER*)&d_data->pData_[ DLPlayData_PT_GetStartAddress(GMDATA_NORMAL_FOTTER) ];
			footer[1] = (SAVE_FOOTER*)&d_data->pData_[ DLPlayData_PT_GetStartAddress(GMDATA_BOX_FOTTER) ];
			footer[2] = (SAVE_FOOTER*)&d_data->pDataMirror_[ DLPlayData_PT_GetStartAddress(GMDATA_NORMAL_FOTTER) ];
			footer[3] = (SAVE_FOOTER*)&d_data->pDataMirror_[ DLPlayData_PT_GetStartAddress(GMDATA_BOX_FOTTER) ];
#if DEB_ARI
			for( i=0;i<4;i++ ){
				OS_TPrintf("footer[%d] g_count[%2d] b_count[%2d] size[%5d] MGNo[%d] blkID[%02x] crc[%d]\n"
							,i ,footer[i]->g_count ,footer[i]->b_count ,footer[i]->size
							,footer[i]->magic_number ,footer[i]->blk_id ,footer[i]->crc);
			}
#endif
			for( i=0;i<4;i++ )
			{
				//データフッタから整合性をチェック
				if( footer[i]->magic_number == -1 ){
					OS_TPrintf("Data[%d] is blank\n");
					isCorrect[i] = FALSE;
				}
				else if( footer[i]->magic_number != MAGIC_NUMBER_PT ){
					OS_TPrintf("Data[%d] is invalid MAGIC_NUMBER[%x]\n",footer[i]->magic_number);
					isCorrect[i] = FALSE;
				}
				else{
					const u32 addOfs = ( i%2==0 ? 0 : boxStartAdd );
					const u16 crc = MATH_CalcCRC16CCITT( &d_data->crcTable_ 
							, ( i<2 ? d_data->pData_+addOfs : d_data->pDataMirror_+addOfs )
							, footer[i]->size - sizeof(SAVE_FOOTER) );
					OS_TPrintf("Data[%d] crc[%d]:[%d]",i,footer[i]->crc,crc);
					if( footer[i]->crc == crc ){
						OS_TPrintf(" crc check is ok!!\n");
						isCorrect[i] = TRUE;
					}
					else{
						OS_TPrintf(" crc check is NG!!\n");
						isCorrect[i] = FALSE;
					}
				}
			}

	   		//最新データのチェック。生きてるほうが片方ならそっち。
			//両方生きていればb_countの新しいほうが最新
			if( isCorrect[1] == TRUE && isCorrect[3] == FALSE ){
				OS_TPrintf("NewData is 1st!\n");
				d_data->pBoxData_ = d_data->pData_ + boxStartAdd;
			}
			else if( isCorrect[1] == FALSE && isCorrect[3] == TRUE ){
				OS_TPrintf("NewData is 2nd!\n");
				d_data->pBoxData_ = d_data->pDataMirror_ + boxStartAdd;
			}
			else if( isCorrect[1] == TRUE && isCorrect[3] == TRUE ){
				if( footer[1]->b_count > footer[3]->b_count ){
					OS_TPrintf("NewData is 1st!\n");
					d_data->pBoxData_ = d_data->pData_ + boxStartAdd;
				}
				else if( footer[1]->b_count < footer[3]->b_count ){
					OS_TPrintf("NewData is 2md!\n");
					d_data->pBoxData_ = d_data->pDataMirror_ + boxStartAdd;
				}
				else{ 
					OS_TPrintf("NewData is ......?\n");
				}
			}
			else{
				OS_TPrintf("NewData is ......?\n");
			}
		}
		GF_ASSERT( d_data->pBoxData_ != NULL );
		d_data->subSeq_++;
		break;
	
 	case 4:	//BOXの解析(本当は親でやる？
		{
			u8 i;
			PT_BOX_DATA *pBox = (PT_BOX_DATA*)d_data->pBoxData_;
			OS_TPrintf("BOX curr[%d]\n",pBox->currentTrayNumber);

			OS_TPrintf("POKEMON PARAM DECODE!!!\n");
			for( i=0;i<BOX_MAX_POS;i++ )
			{
				PT_POKEMON_PARAM *param = &pBox->ppp[0][i];
				u16 sum;
				PT_CalcTool_Decoded( &param->paradata , sizeof(POKEMON_PASO_PARAM1)*4 , param->checksum );
				sum = PT_PokeParaCheckSum( &param->paradata , sizeof(POKEMON_PASO_PARAM1)*4 );
				OS_TPrintf("[%2d] sum[%5d][%5d] ",i,sum ,param->checksum);
				//pBox->ppp[0][i].paradata;
				{
					POKEMON_PASO_PARAM1	*ppp1 = (POKEMON_PASO_PARAM1 *)PokeParaAdrsGet(param,param->personal_rnd,ID_POKEPARA1);
					POKEMON_PASO_PARAM2	*ppp2 = (POKEMON_PASO_PARAM2 *)PokeParaAdrsGet(param,param->personal_rnd,ID_POKEPARA2);
					POKEMON_PASO_PARAM3	*ppp3 = (POKEMON_PASO_PARAM3 *)PokeParaAdrsGet(param,param->personal_rnd,ID_POKEPARA3);
					POKEMON_PASO_PARAM4	*ppp4 = (POKEMON_PASO_PARAM4 *)PokeParaAdrsGet(param,param->personal_rnd,ID_POKEPARA4);
					if( ppp1->monsno == 0 ){
						OS_TPrintf("None ");
					}
					else{
						OS_TPrintf("id[%d] ",ppp1->monsno);
					}
				}
				
				OS_TPrintf("\n");
			}
		}

		d_data->subSeq_++;
		break;
	}
	return FALSE;
}


//各データブロックの開始位置を求める。PTと違ってフッタも一つのブロックとして計算する
u32		DLPlayData_PT_GetStartAddress( const PT_GMDATA_ID id )
{
	u32 temp = 0;
	u8 i;
	for( i=0;i<id;i++ ){
		temp += DLPLAY_PT_SAVESIZETABLE[i];
	}
	return temp;
}


//パラメータ暗号化系
//============================================================================================
/**
 *	暗号処理
 *
 * @param[in]	data	暗号化するデータのポインタ
 * @param[in]	size	暗号化するデータのサイズ
 * @param[in]	code	暗号化キーの初期値
 */
//============================================================================================
void	PT_CalcTool_Coded(void *data,u32 size,u32 code)
{
	int	i;
	u16	*data_p=(u16 *)data;

	//暗号は、乱数暗号キーでマスク
	for(i=0;i<size/2;i++){
		data_p[i]^=PT_CodeRand(&code);
	}
}

//============================================================================================
/**
 *	復号処理
 *
 * @param[in]	data	復号するデータのポインタ
 * @param[in]	size	復号するデータのサイズ
 * @param[in]	code	暗号化キーの初期値
 */
//============================================================================================
void	PT_CalcTool_Decoded(void *data,u32 size,u32 code)
{
	PT_CalcTool_Coded(data,size,code);
}

//============================================================================================
/**
 *	乱数暗号キー生成ルーチン
 *
 * @param[in,out]	code	暗号キー格納ワークのポインタ
 *
 * @return	暗号キー格納ワークの上位2バイトを暗号キーとして返す
 */
//============================================================================================
static	u16 PT_CodeRand(u32 *code)
{
    code[0] = code[0] *1103515245L + 24691;
    return (u16)(code[0] / 65536L) ;
}
//============================================================================================
/**
 *	ポケモンパラメータのチェックサムを生成
 *
 * @param[in]	data	ポケモンパラメータ構造体のポインタ
 * @param[in]	size	チェックサムを生成するデータのサイズ
 *
 * @return	生成したチェックサム
 */
//============================================================================================
static	u16	PT_PokeParaCheckSum(void *data,u32	size)
{
	int	i;
	u16	*data_p=(u16 *)data;
	u16	sum=0;

	for(i=0;i<size/2;i++){
		sum+=data_p[i];
	}

	return sum;
}


//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体内のメンバのアドレスを取得
 *
 * @param[in]	ppp	取得したいボックスポケモンパラメータ構造体のポインタ
 * @param[in]	rnd	構造体入れ替えのキー
 * @param[in]	id	取り出したいメンバのインデックス（poke_tool.hに定義）
 *
 * @return	取得したアドレス
 */
//============================================================================================
static	void	*PokeParaAdrsGet( PT_POKEMON_PARAM *ppp,u32 rnd,u8 id)
{
	void	*ret;
	rnd=(rnd&0x0003e000)>>13;

	GF_ASSERT_MSG(rnd<=ID_POKEPARADATA62,"PokeParaAdrsGet:Index Over!");

	switch(rnd){
	case ID_POKEPARADATA11:
	case ID_POKEPARADATA51:
		{
			POKEMON_PARA_DATA11 *PPD11=(POKEMON_PARA_DATA11 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD11->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD11->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD11->ppp3;
					break;

				case ID_POKEPARA4:
					ret=&PPD11->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA12:
	case ID_POKEPARADATA52:
		{
			POKEMON_PARA_DATA12 *PPD12=(POKEMON_PARA_DATA12 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD12->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD12->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD12->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD12->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA13:
	case ID_POKEPARADATA53:
		{
			POKEMON_PARA_DATA13 *PPD13=(POKEMON_PARA_DATA13 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD13->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD13->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD13->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD13->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA14:
	case ID_POKEPARADATA54:
		{
			POKEMON_PARA_DATA14 *PPD14=(POKEMON_PARA_DATA14 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD14->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD14->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD14->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD14->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA15:
	case ID_POKEPARADATA55:
		{
			POKEMON_PARA_DATA15 *PPD15=(POKEMON_PARA_DATA15 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD15->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD15->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD15->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD15->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA16:
	case ID_POKEPARADATA56:
		{
			POKEMON_PARA_DATA16 *PPD16=(POKEMON_PARA_DATA16 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD16->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD16->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD16->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD16->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA21:
	case ID_POKEPARADATA61:
		{
			POKEMON_PARA_DATA21 *PPD21=(POKEMON_PARA_DATA21 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD21->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD21->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD21->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD21->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA22:
	case ID_POKEPARADATA62:
		{
			POKEMON_PARA_DATA22 *PPD22=(POKEMON_PARA_DATA22 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD22->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD22->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD22->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD22->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA23:
		{
			POKEMON_PARA_DATA23 *PPD23=(POKEMON_PARA_DATA23 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD23->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD23->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD23->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD23->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA24:
		{
			POKEMON_PARA_DATA24 *PPD24=(POKEMON_PARA_DATA24 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD24->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD24->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD24->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD24->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA25:
		{
			POKEMON_PARA_DATA25 *PPD25=(POKEMON_PARA_DATA25 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD25->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD25->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD25->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD25->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA26:
		{
			POKEMON_PARA_DATA26 *PPD26=(POKEMON_PARA_DATA26 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD26->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD26->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD26->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD26->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA31:
		{
			POKEMON_PARA_DATA31 *PPD31=(POKEMON_PARA_DATA31 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD31->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD31->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD31->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD31->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA32:
		{
			POKEMON_PARA_DATA32 *PPD32=(POKEMON_PARA_DATA32 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD32->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD32->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD32->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD32->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA33:
		{
			POKEMON_PARA_DATA33 *PPD33=(POKEMON_PARA_DATA33 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD33->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD33->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD33->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD33->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA34:
		{
			POKEMON_PARA_DATA34 *PPD34=(POKEMON_PARA_DATA34 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD34->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD34->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD34->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD34->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA35:
		{
			POKEMON_PARA_DATA35 *PPD35=(POKEMON_PARA_DATA35 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD35->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD35->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD35->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD35->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA36:
		{
			POKEMON_PARA_DATA36 *PPD36=(POKEMON_PARA_DATA36 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD36->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD36->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD36->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD36->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA41:
		{
			POKEMON_PARA_DATA41 *PPD41=(POKEMON_PARA_DATA41 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD41->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD41->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD41->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD41->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA42:
		{
			POKEMON_PARA_DATA42 *PPD42=(POKEMON_PARA_DATA42 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD42->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD42->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD42->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD42->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA43:
		{
			POKEMON_PARA_DATA43 *PPD43=(POKEMON_PARA_DATA43 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD43->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD43->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD43->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD43->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA44:
		{
			POKEMON_PARA_DATA44 *PPD44=(POKEMON_PARA_DATA44 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD44->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD44->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD44->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD44->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA45:
		{
			POKEMON_PARA_DATA45 *PPD45=(POKEMON_PARA_DATA45 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD45->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD45->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD45->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD45->ppp4;
					break;
			}
			break;
		}
	case ID_POKEPARADATA46:
		{
			POKEMON_PARA_DATA46 *PPD46=(POKEMON_PARA_DATA46 *)&ppp->paradata;
			switch(id){
				case ID_POKEPARA1:
					ret=&PPD46->ppp1;
					break;
				case ID_POKEPARA2:
					ret=&PPD46->ppp2;
					break;
				case ID_POKEPARA3:
					ret=&PPD46->ppp3;
					break;
				case ID_POKEPARA4:
					ret=&PPD46->ppp4;
					break;
			}
			break;
		}
	}
	return	ret;
}

