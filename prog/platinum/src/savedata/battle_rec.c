//==============================================================================
/**
 * @file	battle_rec.c
 * @brief	戦闘録画セーブデータ
 * @author	matsuda
 * @date	2007.03.06(火)
 */
//==============================================================================
#include "common.h"
#include "savedata\savedata.h"
#include "system\gamedata.h"
#include "gflib\strbuf_family.h"
#include "poketool\pokeparty.h"
#include "savedata\pokeparty_local.h"
#include "savedata\mystatus_local.h"
#include "savedata\config.h"

#include "gds_local_common.h"
#include "savedata\battle_rec.h"
#include "itemtool/itemsym.h"
#include "battle/wazano_def.h"
#include "gds_profile_types.h"
#include "savedata/gds_profile.h"
#include "system/snd_perap.h"

#include "application/battle_recorder/gds_battle_rec.h"
#include "savedata/misc.h"


//==============================================================================
//	
//==============================================================================
///データナンバーの型
typedef u64 DATA_NUMBER;
///データナンバーのバイトサイズ
#define DATANUMBER_SIZE		(sizeof(DATA_NUMBER))

///録画ヘッダに格納するポケモン番号の最大要素数
#define HEADER_MONSNO_MAX		(12)

///会話ウィンドウの枚数
#define TALK_WINDOW_MAX			(20)

//--------------------------------------------------------------
/**
 *	戦闘録画用にカスタマイズされたPOKEPARTY
 */
//--------------------------------------------------------------
typedef struct{
	///	保持できるポケモン数の最大
	u16 PokeCountMax;
	///	現在保持しているポケモン数
	u16 PokeCount;
	///	ポケモンデータ
	REC_POKEPARA member[TEMOTI_POKEMAX];
}REC_POKEPARTY;

///<対戦録画用ワーク構造体宣言
struct record_param
{
	REC_DATA	rec_buffer[CLIENT_MAX][REC_BUFFER_SIZE];
};

//----------------------------------------------------------
/**
 *	録画セーブデータ本体（??? bytes）
 */
//----------------------------------------------------------
typedef struct _BATTLE_REC_WORK {
	REC_BATTLE_PARAM	rbp;
	RECORD_PARAM		rp;
	REC_POKEPARTY		rec_party[CLIENT_MAX];
	MYSTATUS			my_status[CLIENT_MAX];
	CONFIG				config;
	u16 padding;
	
	//CRC(必ず最後尾にしておくこと)
	GDS_CRC				crc;
}BATTLE_REC_WORK;

//--------------------------------------------------------------
/**
 *	戦闘録画のヘッダ
 */
//--------------------------------------------------------------
typedef struct _BATTLE_REC_HEADER{
	u16 monsno[HEADER_MONSNO_MAX];	///<ポケモン番号(表示する必要がないのでタマゴの場合は0)
	u8 form_no[HEADER_MONSNO_MAX];	///<ポケモンのフォルム番号

	u16 battle_counter;		///<連勝数
	u8 mode;				///<戦闘モード(ファクトリー50、ファクトリー100、通信対戦...)
	
	u8 padding;				//パディング

	u8 work[16];			///< 予備										16

	//CRCとデータナンバー(必ず最後尾にしておくこと)
	DATA_NUMBER data_number;///<データナンバー(サーバー側でセットされる)。チェックサム対象から外す
	GDS_CRC				crc;
}BATTLE_REC_HEADER;

//--------------------------------------------------------------
/**
 *	録画セーブデータ：GDSプロフィール＋ヘッダ＋本体
 */
//--------------------------------------------------------------
struct _BATTLE_REC_SAVEDATA{
	//殿堂入り以外の外部セーブは必ず一番最初にEX_SAVE_KEY構造体を配置しておくこと
	EX_CERTIFY_SAVE_KEY save_key;			///<認証キー

	GDS_PROFILE profile;			///<GDSプロフィール
	BATTLE_REC_HEADER head;			///<戦闘録画ヘッダ
	BATTLE_REC_WORK rec;			///<戦闘録画本体
};

//==============================================================================
//	
//==============================================================================
static	BATTLE_REC_SAVEDATA * brs=NULL;

//==============================================================================
//	プロトタイプ宣言
//==============================================================================
//static	void BattleRec_BattleParamCreate(BATTLE_PARAM *bp,SAVEDATA *sv);
static void PokeParty_to_RecPokeParty(const POKEPARTY *party, REC_POKEPARTY *rec_party);
static void RecPokeParty_to_PokeParty(REC_POKEPARTY *rec_party, POKEPARTY *party);
static BOOL BattleRec_DataInitializeCheck(SAVEDATA *sv, BATTLE_REC_SAVEDATA *src);
static	BOOL BattleRecordCheckData(SAVEDATA *sv, const BATTLE_REC_SAVEDATA * src);
static	void	BattleRec_Coded(void *data,u32 size,u32 code);
static	void	BattleRec_Decoded(void *data,u32 size,u32 code);
static void RecHeaderCreate(BATTLE_REC_HEADER *head, const BATTLE_REC_WORK *rec, 
	int rec_mode, int counter);
static SAVE_RESULT Local_BattleRecSave(SAVEDATA *sv, BATTLE_REC_SAVEDATA *work, int num, int send_flag_reset);


//------------------------------------------------------------------
/**
 * セーブデータサイズを返す
 *
 * @retval  int		
 */
//------------------------------------------------------------------
int BattleRec_GetWorkSize( void )
{
#if 0
	OS_TPrintf("pokeparty_size = %d\n", sizeof(POKEPARTY) * 4);
	OS_TPrintf("mystatus_size = %d\n", sizeof(MYSTATUS) * 4);
	OS_TPrintf("config_size = %d\n", sizeof(CONFIG));
	
	OS_TPrintf("poke_paso = %d\n", sizeof(POKEMON_PASO_PARAM));
	OS_TPrintf("poke_calc = %d\n", sizeof(POKEMON_CALC_PARAM));
	
	OS_TPrintf("mail_data = %d\n", sizeof(_MAIL_DATA));

	OS_TPrintf("REC_POKEPARTY = %d\n", sizeof(REC_POKEPARTY));
	OS_TPrintf("REC_POKEPARA = %d\n", sizeof(REC_POKEPARA));
	OS_TPrintf("RECORD_PARAM = %d\n", sizeof(RECORD_PARAM));

	OS_TPrintf("TRAINER_DATA = %d\n", sizeof(TRAINER_DATA));
	OS_TPrintf("REC_BATTLE_PARAM = %d\n", sizeof(REC_BATTLE_PARAM));
	
	OS_TPrintf("BATTLE_REC_WORK = %d\n", sizeof(BATTLE_REC_WORK));
	OS_TPrintf("BATTLE_REC_HEADER = %d\n", sizeof(BATTLE_REC_HEADER));
	OS_TPrintf("_BATTLE_REC_SAVEDATA = %d\n", sizeof(struct _BATTLE_REC_SAVEDATA));

	OS_TPrintf("GDS_PROFILE = %d\n", sizeof(GDS_PROFILE));
#endif

	//セクター2ページ分よりもサイズが大きくなったらExtraSaveDataTableの各録画データの
	//ページオフセットも+3単位に直す
	GF_ASSERT(sizeof(BATTLE_REC_SAVEDATA) < SAVE_SECTOR_SIZE * 2);

	return sizeof(BATTLE_REC_SAVEDATA);
}

//--------------------------------------------------------------
/**
 * @brief   ワーク初期化
 *
 * @param   rec		
 */
//--------------------------------------------------------------
void BattleRec_WorkInit(BATTLE_REC_SAVEDATA *rec)
{
	MI_CpuClear32( rec, sizeof(BATTLE_REC_SAVEDATA) );
	rec->save_key = EX_CERTIFY_SAVE_KEY_NO_DATA;
}

//------------------------------------------------------------------
/**
 * データ初期化
 *
 * @param   heapID	メモリ確保するためのヒープID		
 *
 */
//------------------------------------------------------------------
void BattleRec_Init(SAVEDATA *sv,int heapID,LOAD_RESULT *result)
{
	if(brs != NULL){
		sys_FreeMemoryEz(brs);
		brs = NULL;
	}
	//ワークを確保する為だけのLOADなので、LOADDATA_MYREC固定
	brs=SaveData_Extra_LoadBattleRecData(sv,heapID,result,LOADDATA_MYREC);
	BattleRec_WorkInit(brs);
}

//------------------------------------------------------------------
/**
 * 対戦録画データの破棄
 */
//------------------------------------------------------------------
void BattleRec_Exit(void)
{
	GF_ASSERT(brs);
	sys_FreeMemoryEz(brs);
	brs=NULL;
}

//------------------------------------------------------------------
/**
 * 対戦録画データのメモリを確保しているかチェック
 *
 * @retval	TRUE:存在する　FALSE:存在しない
 */
//------------------------------------------------------------------
BOOL BattleRec_DataExistCheck(void)
{
	return (brs!=NULL);
}

//--------------------------------------------------------------
/**
 * @brief   ワークのアドレスを取得
 *
 * @retval	brsに格納されているワークアドレス		
 */
//--------------------------------------------------------------
BATTLE_REC_SAVEDATA * BattleRec_WorkAdrsGet( void )
{
	GF_ASSERT(brs);

	return brs;
}

//--------------------------------------------------------------
/**
 * @brief   認証キーを除いた戦闘録画ワークのアドレスを取得
 *
 * @retval	brsに格納されているワークアドレス(認証キー除く)
 */
//--------------------------------------------------------------
void * BattleRec_RecWorkAdrsGet( void )
{
	u8 *work;
	
	GF_ASSERT(brs);
	
	work = (u8*)brs;
	return &work[sizeof(EX_CERTIFY_SAVE_KEY)];
}

//------------------------------------------------------------------
/**
 * 対戦録画データのロード
 *
 * @param	sv		セーブデータ構造体へのポインタ
 * @param	heapID	データをロードするメモリを確保するためのヒープID
 * @param	result	ロード結果を格納するワークRECLOAD_RESULT_NULL,OK,NG,ERROR
 * @param	bp		ロードしたデータから生成するBATTLE_PARAM構造体へのポインタ
 * @param	num		ロードするデータナンバー（LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…）
 *
 * @retval	TRUE
 */
//------------------------------------------------------------------
BOOL BattleRec_Load(SAVEDATA *sv,int heapID,LOAD_RESULT *result,BATTLE_PARAM *bp,int num)
{
	BATTLE_REC_WORK *rec;
	BATTLE_REC_HEADER *head;
	
	//すでに読み込まれているデータがあるなら、破棄する
	if(brs){
		sys_FreeMemoryEz(brs);
		brs=NULL;
	}
	//本来は、numによって読み込み先を変える
	brs=SaveData_Extra_LoadBattleRecData(sv,heapID,result,num);
	if(*result != LOAD_RESULT_OK){
		*result = RECLOAD_RESULT_ERROR;
		return TRUE;
	}
	
	//読み出したデータに録画データが入っているかチェック
	if(BattleRec_DataInitializeCheck(sv, brs) == TRUE){
		*result = RECLOAD_RESULT_NULL;	//初期化データの為、データなし
		return TRUE;
	}
	
	rec = &brs->rec;
	head = &brs->head;
	
	//復号
#if 0
	BattleRec_Decoded(rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE, rec->checksum);
#else
	BattleRec_Decoded(rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE, 
		rec->crc.crc16ccitt_hash + ((rec->crc.crc16ccitt_hash ^ 0xffff) << 16));
#endif

	//データの整合性チェック
	if(BattleRecordCheckData(sv, brs) == FALSE){
	#ifdef OSP_REC_ON
		OS_TPrintf("不正な録画データ\n");
	#endif
		*result = RECLOAD_RESULT_NG;
		return TRUE;
	}

	//読み出しデータをBATTLE_PARAMにセット
	if(bp){
		BattleRec_BattleParamCreate(bp,sv);
	}

	*result = RECLOAD_RESULT_OK;	
	return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   対戦録画のセーブ処理をまとめたもの
 *
 * @param   sv		セーブデータへのポインタ
 * @param   num		LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…
 * @param   send_flag_reset		TRUE:送信済みフラグをリセットする
 *
 * @retval  セーブ結果
 */
//--------------------------------------------------------------
static SAVE_RESULT Local_BattleRecSave(SAVEDATA *sv, BATTLE_REC_SAVEDATA *work, int num, int send_flag_reset)
{
	SAVE_RESULT result;
	
	result = SaveData_Extra_SaveBattleRecData(sv,work,num);
	if(result == SAVE_RESULT_OK){
		if(send_flag_reset == TRUE){
			MISC_SetGdsSend_Video(SaveData_GetMisc(sv), 0);
		}
		result = SaveData_Save(sv);
	}
	return result;
}

//------------------------------------------------------------------
/**
 * 対戦録画データのセーブ
 *
 * @param	sv				セーブデータ構造体へのポインタ
 * @param	data			セーブデーへのポインタ
 * @param   rec_mode		録画モード(RECMODE_???)
 * @param   fight_count		何戦目か
 * @param	num		ロードするデータナンバー（LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…）
 *
 * @retval	セーブ結果
 */
//------------------------------------------------------------------
SAVE_RESULT BattleRec_Save(SAVEDATA *sv, int rec_mode, int fight_count, int num)
{
	BATTLE_REC_HEADER *head;
	BATTLE_REC_WORK *rec;
	SAVE_RESULT result;
	
	//データがないときは、何もしない
	if(brs==NULL){
		return	SAVE_RESULT_NG;
	}
	head = &brs->head;
	rec = &brs->rec;
	
	//録画データ本体を元にヘッダデータ作成
	RecHeaderCreate(head, rec, rec_mode, fight_count);

	//CRC作成
	head->crc.crc16ccitt_hash = SaveData_CalcCRC(sv, head, 
		sizeof(BATTLE_REC_HEADER) - GDS_CRC_SIZE - DATANUMBER_SIZE);
	rec->crc.crc16ccitt_hash = SaveData_CalcCRC(sv, rec, 
		sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE);

	//CRCをキーにして暗号化
	BattleRec_Coded(rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE, 
		rec->crc.crc16ccitt_hash + ((rec->crc.crc16ccitt_hash ^ 0xffff) << 16));
	
	return Local_BattleRecSave(sv, brs, num, TRUE);
}

//--------------------------------------------------------------
/**
 * @brief   指定位置の録画データを削除(初期化)してセーブ実行
 *
 * @param   sv			
 * @param   heap_id		
 * @param   num			
 *
 * @retval  
 *
 *
 */
//--------------------------------------------------------------
SAVE_RESULT BattleRec_SaveDataErase(SAVEDATA *sv, int heap_id, int num)
{
	BATTLE_REC_SAVEDATA *work;
	SAVE_RESULT result;
	LOAD_RESULT load_result;
	
	work = SaveData_Extra_LoadBattleRecData(sv, heap_id, &load_result, num);
	BattleRec_WorkInit(work);
	result = Local_BattleRecSave(sv, work, num, TRUE);
	sys_FreeMemoryEz(work);
	return result;
}

//--------------------------------------------------------------
/**
 * @brief   録画データ本体を元にヘッダーデータを作成
 *
 * @param   head		ヘッダーデータ代入先
 * @param   rec			録画データ本体
 * @param   rec_mode	録画モード(RECMODE_???)
 * @param   counter		何戦目か
 */
//--------------------------------------------------------------
static void RecHeaderCreate(BATTLE_REC_HEADER *head, const BATTLE_REC_WORK *rec, 
	int rec_mode, int counter)
{
	int client, temoti, client_max, temoti_max, n;
	const REC_POKEPARA *para;
		
	MI_CpuClear8(head, sizeof(BATTLE_REC_HEADER));
	
	switch(rec_mode){
	case RECMODE_TOWER_MULTI:
	case RECMODE_FACTORY_MULTI:
	case RECMODE_FACTORY_MULTI100:
	case RECMODE_STAGE_MULTI:
	case RECMODE_CASTLE_MULTI:
	case RECMODE_ROULETTE_MULTI:
	case RECMODE_COLOSSEUM_MULTI:
		client_max = CLIENT_MAX;
		temoti_max = TEMOTI_POKEMAX / 2;
		break;
	default:
		client_max = CLIENT_MAX / 2;
		temoti_max = TEMOTI_POKEMAX;
		break;
	}

	n = 0;
	for(client = 0; client < client_max; client++){
		for(temoti = 0; temoti < temoti_max; temoti++){
			para = &(rec->rec_party[client].member[temoti]);
			if(para->tamago_flag == 0 && para->fusei_tamago_flag == 0){
				head->monsno[n] = para->monsno;
				head->form_no[n] = para->form_no;
			}
			n++;
		}
	}
	
	head->battle_counter = counter;
	head->mode = rec_mode;
}

//--------------------------------------------------------------
/**
 * @brief   対戦録画データが初期化されたデータか調べる
 *
 * @param   src		対戦録画データへのポインタ
 *
 * @retval  TRUE:初期化されているデータ
 * @retval  FALSE:何らかのデータが入っている
 */
//--------------------------------------------------------------
static BOOL BattleRec_DataInitializeCheck(SAVEDATA *sv, BATTLE_REC_SAVEDATA *src)
{
	BATTLE_REC_WORK *rec = &src->rec;
	BATTLE_REC_HEADER *head = &src->head;

	if(SaveData_GetExtraInitFlag(sv) == FALSE){
		return TRUE;	//外部セーブデータが初期化されていないのでデータ無しと判定する
	}
	
	//※check　もう少し細かく調べる
	if(rec->crc.crc16ccitt_hash == 0 || head->crc.crc16ccitt_hash == 0){
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   セーブデータの整合性チェック
 *
 * @param   src		データの先頭アドレス
 *
 * @retval  TRUE:正しい。　FALSE:不正
 */
//--------------------------------------------------------------
static	BOOL BattleRecordCheckData(SAVEDATA *sv, const BATTLE_REC_SAVEDATA * src)
{
	const BATTLE_REC_WORK *rec = &src->rec;
	const BATTLE_REC_HEADER *head = &src->head;
	u16 hash;
	
	if(rec->crc.crc16ccitt_hash == 0 || head->crc.crc16ccitt_hash == 0){
		return FALSE;
	}
	
	//ヘッダーのCRCハッシュ計算
	hash = SaveData_CalcCRC(sv, head, 
		sizeof(BATTLE_REC_HEADER) -GDS_CRC_SIZE-DATANUMBER_SIZE);
	if(hash != head->crc.crc16ccitt_hash){
	#ifdef OSP_REC_ON
		OS_TPrintf("ヘッダーのCRCハッシュ不正\n");
	#endif
		return FALSE;
	}
	
	//本体全体のCRCハッシュ計算
	hash = SaveData_CalcCRC(sv, rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE);
	if (hash != rec->crc.crc16ccitt_hash) {
	#ifdef OSP_REC_ON
		OS_TPrintf("録画データ本体のCRCハッシュ不正\n");
	#endif
		return FALSE;
	}
	
	//ポケモンパラメータの不正チェック
	{
		int client, temoti, wazano;
		const REC_POKEPARA *para;
		
		for(client = 0; client < CLIENT_MAX; client++){
			for(temoti = 0; temoti < TEMOTI_POKEMAX; temoti++){
				para = &(rec->rec_party[client].member[temoti]);
			#if 0
				//ダメタマゴ
				if(para->fusei_tamago_flag == 1){
					OS_TPrintf("ダメタマゴが混じっている\n");
					return FALSE;
				}
			#endif
				//不正なポケモン番号
				if(para->monsno > MONSNO_MAX){
				#ifdef OSP_REC_ON
					OS_TPrintf("不正なポケモン番号\n");
				#endif
					return FALSE;
				}
				//不正なアイテム番号
				if(para->item > ITEM_DATA_MAX){
				#ifdef OSP_REC_ON
					OS_TPrintf("不正なアイテム番号\n");
				#endif
					return FALSE;
				}
				//不正な技番号
				for(wazano = 0; wazano < WAZA_TEMOTI_MAX; wazano++){
					if(para->waza[wazano] > WAZANO_MAX){
					#ifdef OSP_REC_ON
						OS_TPrintf("不正な技番号\n");
					#endif
						return FALSE;
					}
				}
			}
		}
	}
	
	return TRUE;
}

//============================================================================================
/**
 *	暗号処理
 *
 * @param[in]	data	暗号化するデータのポインタ
 * @param[in]	size	暗号化するデータのサイズ
 * @param[in]	code	暗号化キーの初期値
 */
//============================================================================================
static	void	BattleRec_Coded(void *data,u32 size,u32 code)
{
	CalcTool_Coded(data, size, code);
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
static	void	BattleRec_Decoded(void *data,u32 size,u32 code)
{
	CalcTool_Decoded(data,size,code);
}


//============================================================================================
/**
 *	戦闘録画処理
 *
 * @param[in]	client_no	録画を行うClientNo
 * @param[in]	pos			録画データの格納場所
 * @param[in]	data		録画データ
 */
//============================================================================================
void BattleRec_DataRec(int client_no,int pos,REC_DATA data)
{
	if(brs==NULL){
		return;
	}
	brs->rec.rp.rec_buffer[client_no][pos]=data;
}

//============================================================================================
/**
 *	戦闘再生データ取得処理
 *
 * @param[in]	rp			戦闘システムワークの構造体ポインタ
 * @param[in]	client_no	録画を行うClientNo
 * @param[in]	pos			録画データの格納場所
 */
//============================================================================================
REC_DATA BattleRec_DataPlay(int client_no,int pos)
{
	GF_ASSERT(brs!=NULL);
	return brs->rec.rp.rec_buffer[client_no][pos];
}

//============================================================================================
/**
 *	BATTLE_PARAM構造体保存処理
 *
 * @param[in]	bp			BATTLE_PARAM構造体へのポインタ
 */
//============================================================================================
void BattleRec_BattleParamRec(BATTLE_PARAM *bp)
{
	int i;
	BATTLE_REC_WORK *rec;
	REC_BATTLE_PARAM *rbp;
	
	if(brs==NULL){
		return;
	}

	rec = &brs->rec;
	rbp = &rec->rbp;
#if 0
	rec->bp=*bp;
#else
	rbp->fight_type = bp->fight_type;
	rbp->win_lose_flag = bp->win_lose_flag;
//	rbp->btr = bp->btr;
	rbp->bg_id = bp->bg_id;
	rbp->ground_id = bp->ground_id;
	rbp->place_id = bp->place_id;
	rbp->zone_id = bp->zone_id;
	rbp->time_zone = bp->time_zone;
	rbp->shinka_place_mode = bp->shinka_place_mode;
	rbp->contest_see_flag = bp->contest_see_flag;
	rbp->mizuki_flag = bp->mizuki_flag;
	rbp->get_pokemon_client = bp->get_pokemon_client;
	rbp->weather = bp->weather;
	rbp->level_up_flag = bp->level_up_flag;
	rbp->battle_status_flag = bp->battle_status_flag;
	rbp->safari_ball = bp->safari_ball;
	rbp->regulation_flag = bp->regulation_flag;
	rbp->rand = bp->rand;
	rbp->comm_id = bp->comm_id;
	rbp->dummy = bp->dummy;
	rbp->total_turn = bp->total_turn;
	for(i = 0; i < CLIENT_MAX; i++){
		rbp->trainer_id[i] = bp->trainer_id[i];
		rbp->trainer_data[i] = bp->trainer_data[i];
		rbp->server_version[i] = bp->server_version[i];
		rbp->comm_stand_no[i] = bp->comm_stand_no[i];
		rbp->voice_waza_param[i] = bp->voice_waza_param[i];
	}
#endif

	//-- REC_BATTLE_PARAMではない場所に保存するデータをセット --//
	for(i=0;i<CLIENT_MAX;i++){
		PokeParty_to_RecPokeParty(bp->poke_party[i], &rec->rec_party[i]);
		MyStatus_Copy(bp->my_status[i],&rec->my_status[i]);
		rbp->voice_waza_param[i]=Snd_PerapVoiceWazaParamGet(bp->poke_voice[i]);
	}
	CONFIG_Copy(bp->config,&rec->config);
}

//============================================================================================
/**
 *	BATTLE_PARAM構造体生成処理
 *
 * @param[in]	bp			BATTLE_PARAM構造体へのポインタ
 * @param[in]	sv			セーブ領域へのポインタ
 */
//============================================================================================
void BattleRec_BattleParamCreate(BATTLE_PARAM *bp,SAVEDATA *sv)
{
	int	i;
	BATTLE_REC_WORK *rec = &brs->rec;
	
	bp->fight_type			=	rec->rbp.fight_type;
//	bp->win_lose_flag		=	rec->rbp.win_lose_flag;
//	bp->btr					=	rec->rbp.btr;
	bp->bg_id				=	rec->rbp.bg_id;
	bp->ground_id			=	rec->rbp.ground_id;
	bp->place_id			=	rec->rbp.place_id;
	bp->zone_id				=	rec->rbp.zone_id;
	bp->time_zone			=	rec->rbp.time_zone;
	bp->shinka_place_mode	=	rec->rbp.shinka_place_mode;
	bp->contest_see_flag	=	rec->rbp.contest_see_flag;
	bp->mizuki_flag			=	rec->rbp.mizuki_flag;
	bp->get_pokemon_client	=	rec->rbp.get_pokemon_client;
	bp->weather				=	rec->rbp.weather;
//	bp->level_up_flag		=	rec->rbp.level_up_flag;
	bp->battle_status_flag	=	rec->rbp.battle_status_flag|BATTLE_STATUS_FLAG_REC_BATTLE;
	bp->safari_ball			=	rec->rbp.safari_ball;
	bp->regulation_flag		=	rec->rbp.regulation_flag;
	bp->rand				=	rec->rbp.rand;
	bp->comm_id				=	rec->rbp.comm_id;
//	bp->total_turn			=	rec->rbp.total_turn;

	bp->win_lose_flag		=	0;
	bp->level_up_flag		=	0;

	ZukanWork_Copy(SaveData_GetZukanWork(sv),bp->zw);

	for(i=0;i<CLIENT_MAX;i++){
		bp->trainer_id[i]=rec->rbp.trainer_id[i];
		bp->trainer_data[i]=rec->rbp.trainer_data[i];
		bp->server_version[i]=rec->rbp.server_version[i];
		bp->comm_stand_no[i]=rec->rbp.comm_stand_no[i];
		RecPokeParty_to_PokeParty(&rec->rec_party[i], bp->poke_party[i]);
		MyStatus_Copy(&rec->my_status[i],bp->my_status[i]);
		bp->voice_waza_param[i]=rec->rbp.voice_waza_param[i];
	}
//	CONFIG_Copy(&rec->config,bp->config);
	CONFIG_Copy(SaveData_GetConfig(sv), bp->config);
	bp->config->window_type = rec->config.window_type;
	if(bp->config->window_type >= TALK_WINDOW_MAX){
		bp->config->window_type = 0;	//金銀で会話ウィンドウが増える事を考えてケアを入れておく
	}
}

//--------------------------------------------------------------
/**
 * @brief   POKEPARTYをREC_POKEPARTYに変換する
 *
 * @param   party			変換元データへのポインタ
 * @param   rec_party		変換後のデータ代入先
 */
//--------------------------------------------------------------
static void PokeParty_to_RecPokeParty(const POKEPARTY *party, REC_POKEPARTY *rec_party)
{
	int i;
	POKEMON_PARAM *pp;
	
	MI_CpuClear8(rec_party, sizeof(REC_POKEPARTY));
	
	rec_party->PokeCountMax = PokeParty_GetPokeCountMax(party);
	rec_party->PokeCount = PokeParty_GetPokeCount(party);
	
	for(i = 0; i < rec_party->PokeCount; i++){
		pp = PokeParty_GetMemberPointer(party, i);
		PokePara_to_RecPokePara(pp, &rec_party->member[i]);
	}
}

//--------------------------------------------------------------
/**
 * @brief   REC_POKEPARTYをPOKEPARTYに変換する
 *
 * @param   rec_party		変換元データへのポインタ
 * @param   party			変換後のデータ代入先
 */
//--------------------------------------------------------------
static void RecPokeParty_to_PokeParty(REC_POKEPARTY *rec_party, POKEPARTY *party)
{
	int i;
	POKEMON_PARAM *pp;
	u8 cb_id_para = 0;
	
	pp = PokemonParam_AllocWork(HEAPID_WORLD);
	
	PokeParty_Init(party, rec_party->PokeCountMax);
	for(i = 0; i < rec_party->PokeCount; i++){
		RecPokePara_to_PokePara(&rec_party->member[i], pp);
		PokeParaPut(pp, ID_PARA_cb_id, &cb_id_para);	//カスタムボールは出ないようにする
		PokeParty_Add(party, pp);
	}
	
	sys_FreeMemoryEz(pp);
}

//------------------------------------------------------------------
/**
 * 録画データ１件追加
 *
 * @param   data		殿堂入りセーブデータポインタ
 * @param   party		殿堂入りパーティーのデータ
 * @param   date		殿堂入り日付データ（NITROのデータそのまま）
 *
 */
//------------------------------------------------------------------
/*
void BattleRec_AddRecord( BATTLE_REC_SAVEDATA* data, const u8 *src)
{
	GF_ASSERT(data != NULL);

	MI_CpuCopy32(src, data, sizeof(BATTLE_REC_SAVEDATA));
}
*/

//------------------------------------------------------------------
/**
 * 殿堂入りレコードから、ポケモンデータ取得
 *
 * @param   data		殿堂入りデータポインタ
 * @param   index		レコードインデックス（0が最新で、順に旧くなっていく）
 * @param   poke_pos	何体目のポケモンか（0～5）
 * @param   poke_data	ポケモンデータを受け取る構造体のポインタ
 */
//------------------------------------------------------------------
/*
void BattleRec_GetRecord( const BATTLE_REC_SAVEDATA* data, int index, DENDOU_POKEMON_DATA* poke_data )
{
	const DENDOU_POKEMON_DATA_INSIDE*  recPoke;
	int i;

	GF_ASSERT(data != NULL);
	GF_ASSERT(data->savePoint < DENDOU_RECORD_MAX);
	GF_ASSERT(index < DENDOU_RECORD_MAX);

	index = recIndex_to_datIndex( data, index );
	recPoke = &(data->record[index].pokemon[poke_pos]);

	poke_data->monsno = recPoke->monsno;
	poke_data->level = recPoke->level;
	poke_data->personalRandom = recPoke->personalRandom;
	poke_data->idNumber = recPoke->idNumber;
	poke_data->formNumber = recPoke->formNumber;

	STRBUF_SetStringCode( poke_data->nickname, recPoke->nickname );
	STRBUF_SetStringCode( poke_data->oyaname, recPoke->oyaname );

	for(i=0; i<4; i++)
	{
		poke_data->waza[i] = recPoke->waza[i];
	}
}
*/

//==============================================================================
//
//	データ取得
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   戦闘録画データから、ヘッダーを別途確保したワークにコピーする
 *
 * @param   src		戦闘録画セーブデータへのポインタ
 * @param   heap_id	ヒープID
 *
 * @retval  確保されたヘッダーデータへのポインタ
 *
 * BattleRec_Load or BattleRec_Initを使用してbrsにデータをロードしている必要があります。
 */
//--------------------------------------------------------------
BATTLE_REC_HEADER_PTR BattleRec_HeaderAllocCopy(int heap_id)
{
	BATTLE_REC_HEADER_PTR head;
	
	GF_ASSERT(brs != NULL);
	
	head = sys_AllocMemory(heap_id, sizeof(BATTLE_REC_HEADER));
	MI_CpuCopy32(&brs->head, head, sizeof(BATTLE_REC_HEADER));
	return head;
}

//--------------------------------------------------------------
/**
 * @brief   戦闘録画データから、GDSプロフィールを別途確保したワークにコピーする
 *
 * @param   src		戦闘録画セーブデータへのポインタ
 * @param   heap_id	ヒープID
 *
 * @retval  確保されたGDSプロフィールデータへのポインタ
 *
 * BattleRec_Load or BattleRec_Initを使用してbrsにデータをロードしている必要があります。
 */
//--------------------------------------------------------------
GDS_PROFILE_PTR BattleRec_GDSProfileAllocCopy(int heap_id)
{
	GDS_PROFILE_PTR profile;

	GF_ASSERT(brs != NULL);
	
	profile = sys_AllocMemory(heap_id, sizeof(GDS_PROFILE));
	MI_CpuCopy32(&brs->profile, profile, sizeof(GDS_PROFILE));
	return profile;
	
}

//--------------------------------------------------------------
/**
 * @brief   GDSプロフィールのポインタ取得
 *
 * @param   src		戦闘録画セーブデータへのポインタ
 *
 * @retval  プロフィールのポインタ
 *
 * BattleRec_Load or BattleRec_Initを使用してbrsにデータをロードしている必要があります。
 */
//--------------------------------------------------------------
GDS_PROFILE_PTR BattleRec_GDSProfilePtrGet(void)
{
	GF_ASSERT(brs != NULL);
	return &brs->profile;
}

//--------------------------------------------------------------
/**
 * @brief   録画本体のポインタ取得
 *
 * @param   src		戦闘録画セーブデータへのポインタ
 *
 * @retval  録画本体のポインタ
 *
 * BattleRec_Load or BattleRec_Initを使用してbrsにデータをロードしている必要があります。
 */
//--------------------------------------------------------------
BATTLE_REC_WORK_PTR BattleRec_WorkPtrGet(void)
{
	GF_ASSERT(brs != NULL);
	return &brs->rec;
}

//--------------------------------------------------------------
/**
 * @brief   録画ヘッダのポインタ取得
 *
 * @param   src		戦闘録画セーブデータへのポインタ
 *
 * @retval  録画ヘッダのポインタ
 *
 * BattleRec_Load or BattleRec_Initを使用してbrsにデータをロードしている必要があります。
 */
//--------------------------------------------------------------
BATTLE_REC_HEADER_PTR BattleRec_HeaderPtrGet(void)
{
	GF_ASSERT(brs != NULL);
	return &brs->head;
}

//--------------------------------------------------------------
/**
 * @brief   読み込んでいる録画データに対して、指定データで上書きする
 *
 * @param   gpp			GDSプロフィール
 * @param   head		録画ヘッダ
 * @param   rec			録画本体
 * @param	bp			そのまま再生する場合はBATTLE_PARAM構造体へのポインタを渡す
 * @param   sv			セーブデータへのポインタ
 *
 * BattleRec_Load or BattleRec_Initを使用してbrsにデータをロードしている必要があります。
 */
//--------------------------------------------------------------
void BattleRec_DataSet(GDS_PROFILE_PTR gpp, BATTLE_REC_HEADER_PTR head, BATTLE_REC_WORK_PTR rec, BATTLE_PARAM *bp, SAVEDATA *sv)
{
	GF_ASSERT(brs != NULL);
	MI_CpuCopy8(head, &brs->head, sizeof(BATTLE_REC_HEADER));
	MI_CpuCopy8(rec, &brs->rec, sizeof(BATTLE_REC_WORK));
	MI_CpuCopy8(gpp, &brs->profile, sizeof(GDS_PROFILE));

	//復号
	BattleRec_Decoded(&brs->rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE, 
		brs->rec.crc.crc16ccitt_hash + ((brs->rec.crc.crc16ccitt_hash ^ 0xffff) << 16));
	
	if(bp != NULL){
		BattleRec_BattleParamCreate(bp,sv);
	}
}

//--------------------------------------------------------------
/**
 * @brief   録画ヘッダのパラメータ取得
 *
 * @param   head		録画ヘッダへのポインタ
 * @param   index		データINDEX(RECHEAD_IDX_???)
 * @param   param		パラメータ
 *
 * @retval  取得データ
 */
//--------------------------------------------------------------
u64 RecHeader_ParamGet(BATTLE_REC_HEADER_PTR head, int index, int param)
{
	GF_ASSERT(DATANUMBER_SIZE <= sizeof(u64));
	
	switch(index){
	case RECHEAD_IDX_MONSNO:
		GF_ASSERT(param < HEADER_MONSNO_MAX);
		if(head->monsno[param] > MONSNO_END){
			return 0;
		}
		return head->monsno[param];
	case RECHEAD_IDX_FORM_NO:
		GF_ASSERT(param < HEADER_MONSNO_MAX);
		return head->form_no[param];
	case RECHEAD_IDX_COUNTER:
		return head->battle_counter;
	case RECHEAD_IDX_MODE:
		if(head->mode >= RECMODE_MAX){
			return RECMODE_COLOSSEUM_SINGLE;
		}
		return head->mode;
	case RECHEAD_IDX_DATA_NUMBER:
		return head->data_number;
	}
	
	GF_ASSERT(0);	//不明なINDEX
	return 0;
}

//--------------------------------------------------------------
/**
 * @brief   戦闘録画ヘッダをAllocする
 *
 * @param   heap_id		ヒープID
 *
 * @retval  GDSプロフィールワークへのポインタ
 */
//--------------------------------------------------------------
BATTLE_REC_HEADER_PTR BattleRec_Header_AllocMemory(int heap_id)
{
	BATTLE_REC_HEADER_PTR brhp;
	
	brhp = sys_AllocMemory(heap_id, sizeof(BATTLE_REC_HEADER));
	MI_CpuClear8(brhp, sizeof(BATTLE_REC_HEADER));
	return brhp;
}

//--------------------------------------------------------------
/**
 * @brief   戦闘録画ヘッダを解放
 *
 * @param   brhp		GDSプロフィールへのポインタ
 */
//--------------------------------------------------------------
void BattleRec_Header_FreeMemory(BATTLE_REC_HEADER_PTR brhp)
{
	sys_FreeMemoryEz(brhp);
}

//--------------------------------------------------------------
/**
 * @brief   GDSで受信したデータをセーブする
 *
 * @param   sv				セーブデータへのポインタ
 * @param   recv_data		GT_BATTLE_REC_RECV構造体
 * @param   num				LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…
 *
 * @retval	SAVE_RESULT_OK		セーブ成功
 * @retval	SAVE_RESULT_NG		セーブ失敗
 */
//--------------------------------------------------------------
SAVE_RESULT BattleRec_GDS_RecvData_Save(SAVEDATA *sv, int num)
{
	SAVE_RESULT result;
	
	GF_ASSERT(brs);
	
	//CRCをキーにして暗号化
	//※GDSで受信したデータはbrsにコピーした時点(BattleRec_DataSet)で復号されているので。
	BattleRec_Coded(&brs->rec, sizeof(BATTLE_REC_WORK) - GDS_CRC_SIZE, 
		brs->rec.crc.crc16ccitt_hash + ((brs->rec.crc.crc16ccitt_hash ^ 0xffff) << 16));

	result = Local_BattleRecSave(sv, brs, num, FALSE);
	return result;
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
void BattleRec_GDS_SendData_Conv(SAVEDATA *sv)
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
 * 
 * @retval	SAVE_RESULT_OK		セーブ成功
 * @retval	SAVE_RESULT_NG		セーブ失敗
 */
//--------------------------------------------------------------
SAVE_RESULT BattleRec_GDS_MySendData_DataNumberSetSave(SAVEDATA *sv, u64 data_number)
{
	SAVE_RESULT result;

	GF_ASSERT(brs);
	
	//データナンバーは元々CRCの対象からはずれているので、CRCを再作成する必要はない
	brs->head.data_number = data_number;

	//送信時に行っているBattleRec_GDS_SendData_Conv関数で既に暗号化しているはずなので
	//そのままセーブを行う
	result = Local_BattleRecSave(sv, brs, LOADDATA_MYREC, FALSE);
	return result;
}
