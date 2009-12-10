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
#include <string.h>
#include "system/gfl_use.h"
#include "pt_save.h"
#include "arc_def.h"

#include "../prog/src/test/ariizumi/ari_debug.h"
#include "../prog/src/test/dlplay/dlplay_func.h"
#include "../prog/src/test/dlplay/dlplay_comm_func.h"

#include "dlplay_data_main.h"
#include "dlplay_child_common.h"

#include "pt_save.dat"	//dlplay_child_pt.c 以外で読むな
#include "dp_save.dat"	//dlplay_child_pt.c 以外で読むな

#define PT_SAVE_SIZE (SECTOR_SIZE * SECTOR_MAX)
//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================
enum PT_SAVE_TYPE
{
	PT_MAIN_FIRST	= 0,
	PT_BOX_FIRST	= 1,
	PT_MAIN_SECOND = 2,
	PT_BOX_SECOND	= 3,
};

//======================================================================
//	typedef struct
//======================================================================

//======================================================================
//	proto
//======================================================================

BOOL	DLPlayData_PT_LoadData( DLPLAY_DATA_DATA *d_data );
BOOL	DLPlayData_PT_SaveData( DLPLAY_DATA_DATA *d_data );
void	DLPlayData_PT_SetBoxIndex( DLPLAY_DATA_DATA *d_data , DLPLAY_BOX_INDEX *idxData );
void	DLPlayData_PT_GetPokeSendData( DLPLAY_LARGE_PACKET *packetData , u8 trayIdx , DLPLAY_DATA_DATA *d_data );

u32		DLPlayData_PT_GetStartAddress( const PT_GMDATA_ID id );
u32		DLPlayData_DP_GetStartAddress( const PT_GMDATA_ID id );
u32		DLPlayData_GetStartAddress( const PT_GMDATA_ID id , DLPLAY_CARD_TYPE type );
static	BOOL DLPlayData_CheckDataCorrect( SAVE_FOOTER **pFooterArr , DLPLAY_DATA_DATA *d_data );
static u8	DLPlayData_PT_CompareFooterData( SAVE_FOOTER *fData , BOOL fCorr ,
											 SAVE_FOOTER *sData , BOOL sCorr ,
											 u8	*pos );

void	PT_CalcTool_Decoded(void *data,u32 size,u32 code);
void	PT_CalcTool_Coded(void *data,u32 size,u32 code);
static	u16 PT_CodeRand(u32 *code);
static	u16	PT_PokeParaCheckSum(void *data,u32	size);
static	void	*PokeParaAdrsGet(PT_POKEMON_PARAM *ppp,u32 rnd,u8 id);
static	u8		PokeRareGetPara(u32 id,u32 rnd);
void	PokePasoParaInit(PT_POKEMON_PARAM *ppp);

const u32 DLPlayData_GetSavedataSize( const DLPLAY_CARD_TYPE type );
const u32 DLPlayData_GetBoxDataSize( const DLPLAY_CARD_TYPE type );
const u32 DLPlayData_GetBoxDataStartAddress( const DLPLAY_CARD_TYPE type );


//	データの読み込み
BOOL	DLPlayData_PT_LoadData( DLPLAY_DATA_DATA *d_data )
{
	const u32 saveSize = DLPlayData_GetSavedataSize( d_data->cardType_ );
	switch( d_data->subSeq_ )
	{
	case 0:	//初期化およびデータ読み込み
		{
#if DLPLAY_FUNC_USE_PRINT
		char str[256];
		sprintf(str,"Start load data. size:[%d]", saveSize );
		DLPlayFunc_PutString(str,d_data->msgSys_ );
#endif
		d_data->pData_		= GFL_HEAP_AllocClearMemory(  d_data->heapID_, saveSize );
		d_data->pDataMirror_= GFL_HEAP_AllocClearMemory(  d_data->heapID_, saveSize );
	
		d_data->lockID_ = OS_GetLockID();
		GF_ASSERT( d_data->lockID_ != OS_LOCK_ID_ERROR );
		//プラチナは4MBフラッシュ
		CARD_IdentifyBackup( CARD_BACKUP_TYPE_FLASH_4MBITS );
		CARD_LockBackup( (u16)d_data->lockID_ );
		CARD_ReadFlashAsync( 0x0000 , d_data->pData_ , saveSize , NULL , NULL );
		d_data->subSeq_++;
		}
		break;
	case 1:	//ミラーリングデータ読み込み
		//読み込みの完了を待つ
		if( CARD_TryWaitBackupAsync() == TRUE )
		{
			GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
			CARD_ReadFlashAsync( 0x40000 , d_data->pDataMirror_ , saveSize , NULL , NULL );
			d_data->subSeq_++;
			
			DLPlayFunc_PutString("Data1 load complete.",d_data->msgSys_ );
		}

		break;
	case 2:	//読み込みの完了を待つ
		if( CARD_TryWaitBackupAsync() == TRUE )
		{
			GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
			CARD_UnlockBackup( (u16)d_data->lockID_ );
			OS_ReleaseLockID( (u16)d_data->lockID_ );
			d_data->subSeq_++;
			
			DLPlayFunc_PutString("Data2 load complete.",d_data->msgSys_ );
		}
		break;
	case 3:	//各ブロックのCRCチェック
		{
			//おそらくGSでは処理が変わる
			//各ブロックのCRCをチェック(一応通常データも見ておく
			u8 i;
			const u32 boxStartAdd = DLPlayData_GetStartAddress(PT_GMDATA_ID_BOXDATA , d_data->cardType_ );
			SAVE_FOOTER *footer[4];
			footer[PT_MAIN_FIRST]	= (SAVE_FOOTER*)&d_data->pData_[ DLPlayData_GetStartAddress(PT_GMDATA_NORMAL_FOOTER , d_data->cardType_ ) ];
			footer[PT_BOX_FIRST]	= (SAVE_FOOTER*)&d_data->pData_[ DLPlayData_GetStartAddress(PT_GMDATA_BOX_FOOTER , d_data->cardType_ ) ];
			footer[PT_MAIN_SECOND]  = (SAVE_FOOTER*)&d_data->pDataMirror_[ DLPlayData_GetStartAddress(PT_GMDATA_NORMAL_FOOTER , d_data->cardType_ ) ];
			footer[PT_BOX_SECOND]	= (SAVE_FOOTER*)&d_data->pDataMirror_[ DLPlayData_GetStartAddress(PT_GMDATA_BOX_FOOTER , d_data->cardType_ ) ];
#if DEB_ARI
			for( i=0;i<4;i++ ){
				OS_TPrintf("footer[%d] g_count[%2d] b_count[%2d] size[%5d] MGNo[%d] blkID[%02x] crc[%d]\n"
							,i ,footer[i]->g_count ,footer[i]->b_count ,footer[i]->size
							,footer[i]->magic_number ,footer[i]->blk_id ,footer[i]->crc);
			}
#endif
			{
				const BOOL isCorrectData = DLPlayData_CheckDataCorrect( &footer[0] , d_data );
				if( isCorrectData == TRUE )
				{
					if( d_data->boxSavePos_ == DDS_FIRST )
					{
						d_data->pBoxData_ = d_data->pData_ + boxStartAdd;
					}
					else
					{
						d_data->pBoxData_ = d_data->pDataMirror_ + boxStartAdd;
					}
				}
				else
				{
					d_data->errorState_ = DES_MISS_LOAD_BACKUP;
				}

			}

		}
		//GF_ASSERT( d_data->pBoxData_ != NULL );
		d_data->subSeq_++;
		break;
	
 	case 4:	//BOXの解析(本当は親でやる？
		d_data->subSeq_++;
		break;
	case 5:
		d_data->subSeq_ = 0;
		return TRUE;
		break;
	}
	return FALSE;
}

#define DLPLAY_CHECK_SAVE_TICK 1
#if DLPLAY_CHECK_SAVE_TICK
static OSTick dlplaySaveTick;
#endif

//セーブは１つの関数で、セーブ→同期待ち→ラストセーブを行う。
BOOL	DLPlayData_PT_SaveData( DLPLAY_DATA_DATA *d_data )
{
	switch( d_data->subSeq_ )
	{
	case 0:	//データの加工
		{
			u8 i,bi;
			PT_BOX_DATA *pBox = (PT_BOX_DATA*)d_data->pBoxData_;
			/*
			//ポケモンデータの暗号化
			for( bi=0;bi<BOX_MAX_TRAY;bi++)
			{
				for( i=0;i<BOX_MAX_POS;i++ )
				{
					PT_POKEMON_PARAM *param = &pBox->ppp[bi][i];
					u16 sum;
					sum = PT_PokeParaCheckSum( &param->paradata , sizeof(POKEMON_PASO_PARAM1)*4 );
					PT_CalcTool_Coded( &param->paradata , sizeof(POKEMON_PASO_PARAM1)*4 , param->checksum );
				}
			}
			
			//ポケモンを消す(消した後に暗号化もするので、暗号化の後
			GF_ASSERT( d_data->selectBoxNumber_ != SELECT_BOX_INVALID );
			for( i=0;i<BOX_MAX_POS;i++ )
			{
				PT_POKEMON_PARAM *param = &pBox->ppp[d_data->selectBoxNumber_][i];
				PokePasoParaInit( param );
			}
			*/
			//footerの加工
			//BOX
			{
				SAVE_FOOTER *boxFooter,*mainFooter;
				if( d_data->boxSavePos_ == DDS_FIRST ){
					boxFooter = (SAVE_FOOTER*)&d_data->pData_[ DLPlayData_GetStartAddress(PT_GMDATA_BOX_FOOTER,d_data->cardType_) ];
				}
				else{
					boxFooter = (SAVE_FOOTER*)&d_data->pDataMirror_[ DLPlayData_GetStartAddress(PT_GMDATA_BOX_FOOTER,d_data->cardType_) ];
				}

				if( d_data->mainSavePos_ == DDS_FIRST ){
					mainFooter = (SAVE_FOOTER*)&d_data->pData_[ DLPlayData_GetStartAddress(PT_GMDATA_NORMAL_FOOTER,d_data->cardType_) ];
					mainFooter->crc = MATH_CalcCRC16CCITT( &d_data->crcTable_, d_data->pData_, mainFooter->size - sizeof(SAVE_FOOTER) );
				}
				else{
					mainFooter = (SAVE_FOOTER*)&d_data->pDataMirror_[ DLPlayData_GetStartAddress(PT_GMDATA_NORMAL_FOOTER,d_data->cardType_) ];
					mainFooter->crc = MATH_CalcCRC16CCITT( &d_data->crcTable_, d_data->pDataMirror_, mainFooter->size - sizeof(SAVE_FOOTER) );
				}

				boxFooter->crc = MATH_CalcCRC16CCITT( &d_data->crcTable_, d_data->pBoxData_, boxFooter->size - sizeof(SAVE_FOOTER) );
				GF_ASSERT( boxFooter->g_count == mainFooter->g_count );
				boxFooter->g_count++;
				mainFooter->g_count++;
				boxFooter->b_count++;
				mainFooter->b_count++;
			}
			
			//カードのロック
			d_data->lockID_ = OS_GetLockID();
			GF_ASSERT( d_data->lockID_ != OS_LOCK_ID_ERROR );
			CARD_LockBackup( (u16)d_data->lockID_ );
			d_data->subSeq_++;
			
		}
		break;

	case 1:	//進行データのセーブ
		{
			//読み込んだほうとは反対側に書く
			const u32 saveAddress = ( d_data->mainSavePos_ == DDS_FIRST ? 0x40000 : 0x00000 );
			const u32 saveSize = DLPlayData_GetStartAddress( PT_GMDATA_NORMAL_FOOTER+1 , d_data->cardType_ );
			void *pData;
			if( d_data->mainSavePos_ == DDS_FIRST ){
				pData = d_data->pData_;
			}
			else{
				pData = d_data->pDataMirror_;
			}
			//セーブ開始！

			OS_TPrintf("[[%d]]\n",saveSize);
			CARD_WriteAndVerifyFlashAsync( saveAddress , pData , saveSize , NULL , NULL );
			
			DLPlayFunc_PutString("saveMain......",d_data->msgSys_ );
			d_data->subSeq_++;
#if DLPLAY_CHECK_SAVE_TICK
			dlplaySaveTick = OS_GetTick();
#endif
		}

		break;

	case 2:	//BOXデータのCRCセットからセーブ(footerとか一緒に使うから
		if( CARD_TryWaitBackupAsync() == TRUE )
		{
			u32 saveAddress;
			const u32 saveSize = DLPlayData_GetBoxDataSize( d_data->cardType_ )+sizeof( SAVE_FOOTER );
			GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );

			//セーブ開始！
			//読み込んだほうとは反対側に書く
			saveAddress = ( d_data->boxSavePos_ == DDS_FIRST ? 0x40000 : 0x00000 );
			saveAddress += DLPlayData_GetStartAddress(PT_GMDATA_ID_BOXDATA,d_data->cardType_);

			CARD_WriteAndVerifyFlashAsync( saveAddress , d_data->pBoxData_ , saveSize-1 , NULL , NULL );
			
			OS_TPrintf("[[%d]]\n",saveSize);
			DLPlayFunc_PutString("saveBox......",d_data->msgSys_);
			d_data->subSeq_++;
#if DLPLAY_CHECK_SAVE_TICK
			{
				const OSTick nowTick = OS_GetTick();
				OS_TPrintf("-SAVE TICK CHECK- SAVE_MAIN[%d]\n",OS_TicksToMilliSeconds( nowTick - dlplaySaveTick ));
			}
#endif

		}
		break;
	case 3:	//書き込みの完了を待つ
		if( CARD_TryWaitBackupAsync() == TRUE )
		{
			GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
			DLPlayFunc_PutString("Save complete.",d_data->msgSys_ );
			//ここでLockは保持したまま
//			CARD_UnlockBackup( (u16)d_data->lockID_ );
//			OS_ReleaseLockID( (u16)d_data->lockID_ );
			d_data->isFinishSaveFirst_ = TRUE;
			d_data->subSeq_++;
#if DLPLAY_CHECK_SAVE_TICK
			{
				const OSTick nowTick = OS_GetTick();
				OS_TPrintf("-SAVE TICK CHECK- SAVE_WAIT[%d]\n",OS_TicksToMilliSeconds( nowTick - dlplaySaveTick ));
			}
#endif
		}
		break;
	case 4:	//最後の同期を待つ
		if( d_data->permitLastSaveFirst_ == TRUE )
		{
			d_data->subSeq_++;
		}
		break;
	case 5:	//最後のセーブ
		{
			u32 saveAddress;
			const u32 saveSize = DLPlayData_GetBoxDataSize( d_data->cardType_ )+sizeof( SAVE_FOOTER );
			GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );

			//セーブ開始！
			//読み込んだほうとは反対側に書く
			saveAddress = ( d_data->boxSavePos_ == DDS_FIRST ? 0x40000 : 0x00000 );
			saveAddress += DLPlayData_GetStartAddress(PT_GMDATA_ID_BOXDATA,d_data->cardType_);
			saveAddress += saveSize-1;

			CARD_WriteAndVerifyFlashAsync( saveAddress , d_data->pBoxData_+saveSize-1 , 1 , NULL , NULL );
			
			DLPlayFunc_PutString("saveBox......",d_data->msgSys_ );
			d_data->subSeq_++;
#if DLPLAY_CHECK_SAVE_TICK
			{
				const OSTick nowTick = OS_GetTick();
				OS_TPrintf("-SAVE TICK CHECK- SAVE_LAST[%d]\n",OS_TicksToMilliSeconds( nowTick - dlplaySaveTick ));
			}
#endif
		}
		break;

	case 6:	//最終確認
		if( CARD_TryWaitBackupAsync() == TRUE )
		{
			GF_ASSERT( CARD_GetResultCode() == CARD_RESULT_SUCCESS );
			DLPlayFunc_PutString("Save complete!!.",d_data->msgSys_ );
			//ここでLockは保持したまま
			CARD_UnlockBackup( (u16)d_data->lockID_ );
			OS_ReleaseLockID( (u16)d_data->lockID_ );
			d_data->subSeq_++;
			d_data->isFinishSaveSecond_ = TRUE;	//2番目は無い
			d_data->isFinishSaveAll_ = TRUE;
		}
		break;
	}
	return FALSE;
}

void	DLPlayData_PT_SetBoxIndex( DLPLAY_DATA_DATA *d_data , DLPLAY_BOX_INDEX *idxData )
{
	u8 i,bi;
	PT_BOX_DATA *pBox = (PT_BOX_DATA*)d_data->pBoxData_;
	char name_str[256];
	u8 num=0;

	//レベル解読用にパーソナルデータとレベルテーブル
	POKEMON_PERSONAL_DATA* perData;
	u32	expTbl[101];
	//メモリの取得開放はまとめてやっちゃう
	perData = GFL_HEAP_AllocMemory( d_data->heapID_ , sizeof(POKEMON_PERSONAL_DATA));

	OS_TPrintf("POKEMON PARAM DECODE!!!\n");
	for( bi=0;bi<BOX_MAX_TRAY;bi++)
	{
		DLPlayFunc_DPTStrCode_To_UTF16( pBox->trayName[bi] , idxData->boxName_[bi] , BOX_MONS_NAME_LEN );
		OS_TPrintf("------BOX[%d]------\n",bi);
		for( i=0;i<BOX_MAX_POS;i++ )
		{
			DLPLAY_MONS_INDEX *pokeData = &idxData->pokeData_[bi][i];
			PT_POKEMON_PARAM *param = &pBox->ppp[bi][i];
			u16 sum;
			PT_CalcTool_Decoded( &param->paradata , sizeof(POKEMON_PASO_PARAM1)*4 , param->checksum );
			sum = PT_PokeParaCheckSum( &param->paradata , sizeof(POKEMON_PASO_PARAM1)*4 );
			//FIXME:サムチェック
			OS_TPrintf("[%2d-%2d] sum[%5d][%5d] ",bi,i,sum ,param->checksum);
			{
				POKEMON_PASO_PARAM1	*ppp1 = (POKEMON_PASO_PARAM1 *)PokeParaAdrsGet(param,param->personal_rnd,ID_POKEPARA1);
				POKEMON_PASO_PARAM2	*ppp2 = (POKEMON_PASO_PARAM2 *)PokeParaAdrsGet(param,param->personal_rnd,ID_POKEPARA2);
				POKEMON_PASO_PARAM3	*ppp3 = (POKEMON_PASO_PARAM3 *)PokeParaAdrsGet(param,param->personal_rnd,ID_POKEPARA3);
				POKEMON_PASO_PARAM4	*ppp4 = (POKEMON_PASO_PARAM4 *)PokeParaAdrsGet(param,param->personal_rnd,ID_POKEPARA4);
				if( ppp1->monsno == 0 ){
					pokeData->name_[0] = 0xffff;
					pokeData->pokeNo_ = 0;
					pokeData->lv_ = 0;
					pokeData->sex_ = 0;
					pokeData->form_ = 0;
					pokeData->rare_ = 0;
					OS_TPrintf("None");
				}
				else
				{
					u8 lv;
					pokeData->pokeNo_	= ppp1->monsno;
					pokeData->sex_		= ppp2->sex;
					pokeData->form_		= ppp2->form_no;
					DLPlayFunc_DPTStrCode_To_UTF16( ppp3->nickname , pokeData->name_ , MONS_NAME_SIZE+EOM_SIZE );
					OS_TPrintf("No[%3d]",ppp1->monsno);
					//レベル計算
					GFL_ARC_LoadData( perData , ARCID_POKE_PERSONAL , ppp1->monsno );
					GFL_ARC_LoadData( expTbl , ARCID_GRAWTBL , perData->grow );
					for( lv=1;lv<101;lv++ ){
						if( expTbl[lv] > ppp1->exp){break;}
					}
					pokeData->lv_ = lv-1;
					OS_TPrintf("lv[%2d]",lv-1);
					//レアチェック
					if( PokeRareGetPara(ppp1->id_no,param->personal_rnd) == TRUE ){
						pokeData->rare_		= 1;
						OS_TPrintf("Rare!! ");
					}
					else{
						pokeData->rare_		= 0;
					}
					//タマゴチェック
					if( ppp2->tamago_flag == 1 ||
						param->fusei_tamago_flag == 1 ){
						pokeData->isEgg_ = 1;
						OS_TPrintf("Egg!! ");
					}
					else{
						pokeData->isEgg_ = 0;
					}

					
				}
				OS_TPrintf("\n");
			}
		}
	}
	DLPlayFunc_PutString("Data load & decode is complete.",d_data->msgSys_ );
	
	GFL_HEAP_FreeMemory( perData );
}

void	DLPlayData_PT_GetPokeSendData( DLPLAY_LARGE_PACKET *packetData , u8 trayIdx , DLPLAY_DATA_DATA *d_data )
{
	PT_BOX_DATA *pBox = (PT_BOX_DATA*)d_data->pBoxData_;
	GF_ASSERT( sizeof(PT_POKEMON_PARAM)*BOX_MAX_POS <= LARGEPACKET_POKE_SIZE );
	GFL_STD_MemCopy( (void*)&pBox->ppp[trayIdx] , (void*)&packetData->pokeData_ , sizeof( PT_POKEMON_PARAM )*BOX_MAX_POS );

//	return (u8*)d_data->pBoxData_;
	
}

//データの関連性、整合性をチェックする
static	BOOL DLPlayData_CheckDataCorrect( SAVE_FOOTER **pFooterArr , DLPLAY_DATA_DATA *d_data )
{
	u8 i;
	u8	mainDataNum,boxDataNum;
	BOOL isCorrect[4];	//データ成否チェック
	//まずデータ単体のチェック
	for( i=0;i<4;i++ )
	{
		const u32 mainEndAdd = DLPlayData_GetStartAddress( PT_GMDATA_NORMAL_FOOTER+1 , d_data->cardType_ );
		const u32 boxEndAdd = DLPlayData_GetStartAddress( PT_GMDATA_BOX_FOOTER+1 , d_data->cardType_ );
		const u32 saveSize = ( i%2==0 ? mainEndAdd : boxEndAdd - mainEndAdd );
		const u32 addOfs = ( i%2==0 ? 0 : mainEndAdd );
		//データフッタから整合性をチェック
		//マジックナンバー
		if( pFooterArr[i]->magic_number == -1 ){
			OS_TPrintf("Data[%d] is blank\n");
			isCorrect[i] = FALSE;
			continue;
		}
		else if( pFooterArr[i]->magic_number != MAGIC_NUMBER_PT ){
			OS_TPrintf("Data[%d] is invalid MAGIC_NUMBER[%x]\n",pFooterArr[i]->magic_number);
			isCorrect[i] = FALSE;
			continue;
		}
		//ブロックID(MAINは0 BOXは1
		if( pFooterArr[i]->blk_id != ( i%2==0 ? 0 : 1 ) )
		{
			OS_TPrintf(" BlockID is NG!!\n");
			isCorrect[i] = FALSE;
			continue;
		}

		//サイズのチェック
		if( pFooterArr[i]->size != saveSize )
		{
			OS_TPrintf(" Datasize is NG!!\n");
			isCorrect[i] = FALSE;
			continue;
		}
		//CRCチェック
		{
			//サイズが正しいのを確認してからやらないとマズイ
			const u16 crc = MATH_CalcCRC16CCITT( &d_data->crcTable_ 
					, ( i<2 ? d_data->pData_+addOfs : d_data->pDataMirror_+addOfs )
					, pFooterArr[i]->size - sizeof(SAVE_FOOTER) );
			OS_TPrintf("Data[%d] crc[%d]:[%d]",i,pFooterArr[i]->crc,crc);
			if( pFooterArr[i]->crc != crc ){
				OS_TPrintf(" crc check is NG!!\n");
				isCorrect[i] = FALSE;
				continue;
			}
		}

		isCorrect[i] = TRUE;
	}
	mainDataNum = DLPlayData_PT_CompareFooterData( pFooterArr[ PT_MAIN_FIRST ] ,isCorrect[ PT_MAIN_FIRST ],
												   pFooterArr[ PT_MAIN_SECOND ] ,isCorrect[ PT_MAIN_SECOND ] ,
												   &d_data->mainSavePos_ );
	boxDataNum = DLPlayData_PT_CompareFooterData(  pFooterArr[ PT_BOX_FIRST ] ,isCorrect[ PT_BOX_FIRST ],
												   pFooterArr[ PT_BOX_SECOND ] ,isCorrect[ PT_BOX_SECOND ] ,
												   &d_data->boxSavePos_ );
	if( mainDataNum == 0 || boxDataNum == 0 )
	{
		return FALSE;
	}
	else if( mainDataNum == 1 && boxDataNum == 1 )
	{
		if( d_data->mainSavePos_ == d_data->boxSavePos_ )
		{
			//初回セーブ
			return TRUE;
		}
		else
		{
			//片側破壊
			return FALSE;
		}
	}
	else if( mainDataNum == 1 && boxDataNum == 2 )
	{
		//進行データ破壊
		return FALSE;
	}
	else if( mainDataNum == 2 && boxDataNum == 1 )
	{
		if( pFooterArr[ d_data->mainSavePos_*2 ]->g_count ==
			pFooterArr[ d_data->boxSavePos_*2+1]->g_count )
		{
			//初回セーブ後、全体セーブなし
			return TRUE;
		}
		else
		{
			//BOXデータ破壊
			return FALSE;
		}
	}
	else
	{
		if( pFooterArr[ d_data->mainSavePos_*2 ]->g_count ==
			pFooterArr[ d_data->boxSavePos_*2+1]->g_count )
		{
			//問題なし
			return TRUE;
		}
		else
		{
			//進行とBOXの間でセーブ中断
			return FALSE;
		}
	}
	return TRUE;
}

//データの新しいほうと、正しい個数を取得
static u8	DLPlayData_PT_CompareFooterData( SAVE_FOOTER *fData , BOOL fCorr ,
											 SAVE_FOOTER *sData , BOOL sCorr ,
											 u8 *pos )
{
	if( fCorr == TRUE && sCorr == TRUE )
	{
		//両方正しい
		if( fData->g_count != sData->g_count )
		{
			//グローバルが違うので全体セーブ後
			if( fData->b_count > sData->b_count )
			{
				GF_ASSERT( fData->g_count > sData->g_count );
				*pos = DDS_FIRST;
			}
			else if( fData->b_count < sData->b_count )
			{
				GF_ASSERT( fData->g_count < sData->g_count );
				*pos = DDS_SECOND;
			}
			else
			{
				//ありえないエラー(一応2Mフラッシュがあるらしい・・・
				GF_ASSERT( FALSE );
				*pos = DDS_FIRST;
				return 0;
			}
		}
		else
		{
			//グローバルが一緒な時は部分セーブ
			if( fData->b_count > sData->b_count )
			{
				*pos = DDS_FIRST;
			}
			else if( fData->b_count < sData->b_count )
			{
				*pos = DDS_SECOND;
			}
			else
			{
				//ありえないエラー(一応2Mフラッシュがあるらしい・・・
				GF_ASSERT( FALSE );
				*pos = DDS_FIRST;
				return 0;
			}
		}
		return 2;
	}
	else
	if( fCorr == TRUE && sCorr == FALSE )
	{
		//片方だけ生きてる
		*pos = DDS_FIRST;
		return 1;
	}
	else if( fCorr == FALSE && sCorr == TRUE )
	{
		//片方だけ生きてる
		*pos = DDS_SECOND;
		return 1;
	}
	else
	{
		//両方アウト
		*pos = DDS_ERROR;
		return 0;
	}
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

//各データブロックの開始位置を求める。PTと違ってフッタも一つのブロックとして計算する
u32		DLPlayData_DP_GetStartAddress( const PT_GMDATA_ID id )
{
	u32 temp = 0;
	u8 i;
	//EMAIL以降になるとPT専用のIDが2個入っているので、ここで調整
	PT_GMDATA_ID max = id;
	if( max >= PT_GMDATA_ID_EMAIL ){ max-=2; }
	for( i=0;i<max;i++ ){
		temp += DLPLAY_DP_SAVESIZETABLE[i];
	}
	return temp;
}

//ダイパプラチナで共通化するための関数
u32		DLPlayData_GetStartAddress( const PT_GMDATA_ID id , DLPLAY_CARD_TYPE type )
{
	if( type == CARD_TYPE_DP ){
		return DLPlayData_DP_GetStartAddress( id );
	}
	else if( type == CARD_TYPE_PT ){
		return DLPlayData_PT_GetStartAddress( id );
	}
	
	GF_ASSERT( NULL );
	return 0;
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

//ID(pokenoじゃない！)と乱数でレアかどうか求める
static u8		PokeRareGetPara(u32 id,u32 rnd)
{
	return((((id&0xffff0000)>>16)^(id&0xffff)^((rnd&0xffff0000)>>16)^(rnd&0xffff))<8);
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
//============================================================================================
/**
 *	ボックスポケモンパラメータ構造体を初期化する
 *
 * @param[in]	ppp	初期化するボックスポケモンパラメータ構造体のポインタ
 */
//============================================================================================
void	PokePasoParaInit(PT_POKEMON_PARAM *ppp)
{
	MI_CpuClearFast(ppp,sizeof(PT_POKEMON_PARAM));
//個性乱数セットしたところで暗号化
	PT_CalcTool_Coded(ppp->paradata,sizeof(POKEMON_PASO_PARAM1)*4,ppp->checksum);
}


//////////////////////////////////////////////////////////////////////////////////////////////
//	各種ROMごとの数値取得関数
//////////////////////////////////////////////////////////////////////////////////////////////
const u32 DLPlayData_GetSavedataSize( const DLPLAY_CARD_TYPE type )
{
	GF_ASSERT( type < CARD_TYPE_INVALID );
	{
		const sizeArr[] = { PT_SAVE_SIZE , PT_SAVE_SIZE , PT_SAVE_SIZE };
		return sizeArr[type];
	}
}

const u32 DLPlayData_GetBoxDataSize( const DLPLAY_CARD_TYPE type )
{
	GF_ASSERT( type < CARD_TYPE_INVALID );
	switch( type )
	{
	case CARD_TYPE_DP:
		return DLPlayData_DP_GetStartAddress( PT_GMDATA_BOX_FOOTER ) - DLPlayData_DP_GetStartAddress( PT_GMDATA_ID_BOXDATA );
		break;
	case CARD_TYPE_PT:
		return DLPlayData_PT_GetStartAddress( PT_GMDATA_BOX_FOOTER ) - DLPlayData_PT_GetStartAddress( PT_GMDATA_ID_BOXDATA );
		break;
	case CARD_TYPE_GS:
		return 0;
		break;
	}
	return 0;
}

const u32 DLPlayData_GetBoxDataStartAddress( const DLPLAY_CARD_TYPE type )
{
	GF_ASSERT( type < CARD_TYPE_INVALID );
	switch( type )
	{
	case CARD_TYPE_DP:
		return DLPlayData_DP_GetStartAddress( PT_GMDATA_ID_BOXDATA );
		break;
	case CARD_TYPE_PT:
		return DLPlayData_PT_GetStartAddress( PT_GMDATA_ID_BOXDATA );
		break;
	case CARD_TYPE_GS:
		return 0;
		break;
	}
	return 0;
}

