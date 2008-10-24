
#define CRC_LOADCHECK (1)
//---------------------------------------------------------------------------
///	セーブに使用しているセクタ数
//---------------------------------------------------------------------------
#define	SAVE_PAGE_MAX		(32)

#define	SAVE_SECTOR_SIZE	(0x1000)
//---------------------------------------------------------------------------
///	セーブブロックの定義
//---------------------------------------------------------------------------
typedef enum {
	SVBLK_ID_NORMAL = 0,	///<通常ゲーム進行データ
	SVBLK_ID_BOX,			///<ポケモンボックス

	SVBLK_ID_MAX,
}SVBLK_ID;

//---------------------------------------------------------------------------
///	セーブ項目の定義
//---------------------------------------------------------------------------
typedef enum {
	//ノーマルデータグループ
	GMDATA_ID_SYSTEM_DATA,
	GMDATA_ID_PLAYER_DATA,
	GMDATA_ID_TEMOTI_POKE,
	GMDATA_ID_TEMOTI_ITEM,
	GMDATA_ID_EVENT_WORK,
	GMDATA_ID_POKETCH_DATA,
	GMDATA_ID_SITUATION,
	GMDATA_ID_ZUKANWORK,
	GMDATA_ID_SODATEYA,
	GMDATA_ID_FRIEND,
	GMDATA_ID_MISC,
	GMDATA_ID_FIELDOBJSV,
	GMDATA_ID_UNDERGROUNDDATA,
    GMDATA_ID_REGULATION,
	GMDATA_ID_IMAGECLIPDATA,
	GMDATA_ID_MAILDATA,
	GMDATA_ID_PORUTODATA,
	GMDATA_ID_RANDOMGROUP,
	GMDATA_ID_FNOTE,
	GMDATA_ID_TRCARD,
	GMDATA_ID_RECORD,
	GMDATA_ID_CUSTOM_BALL,
	GMDATA_ID_PERAPVOICE,
	GMDATA_ID_FRONTIER,
	GMDATA_ID_SP_RIBBON,
	GMDATA_ID_ENCOUNT,
	GMDATA_ID_WORLDTRADEDATA,
	GMDATA_ID_TVWORK,
	GMDATA_ID_GUINNESS,
	GMDATA_ID_WIFILIST,
	GMDATA_ID_WIFIHISTORY,
	GMDATA_ID_FUSHIGIDATA,
	GMDATA_ID_POKEPARKDATA,
	GMDATA_ID_CONTEST,
	GMDATA_ID_PMS,
	GMDATA_ID_EMAIL,
	GMDATA_ID_WFHIROBA,

	//ボックスデータグループ
	GMDATA_ID_BOXDATA,

	GMDATA_ID_MAX,
}GMDATA_ID;
//=============================================================================
//=============================================================================

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define MAGIC_NUMBER	(0x20060623)
#define	SECTOR_SIZE		(SAVE_SECTOR_SIZE)
#define SECTOR_MAX		(SAVE_PAGE_MAX)

#define	FIRST_MIRROR_START	(0)
#define	SECOND_MIRROR_START	(64)

#define MIRROR1ST	(0)
#define	MIRROR2ND	(1)
#define	MIRRORERROR	(2)

#define HEAPID_SAVE_TEMP	(HEAPID_BASE_APP)


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define	SEC_DATA_SIZE		SECTOR_SIZE


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
typedef struct {
	u32 g_count;		///<グローバルカウンタ（MYデータ、BOXデータ共有）
	u32 b_count;		///<ブロック内カウンタ（MYデータとBOXデータとで独立）
	u32 size;			///<データサイズ（フッタサイズ含む）
	u32 magic_number;	///<マジックナンバー
	u8 blk_id;			///<対象のブロック指定ID
	u16 crc;			///<データ全体のCRC値
}SAVE_FOOTER;

//---------------------------------------------------------------------------
/**
 * @brief	セーブブロックデータ情報
 */
//---------------------------------------------------------------------------
typedef struct {
	u8 id;				///<ブロック定義ID
	u8 start_sec;		///<開始セクタ位置
	u8 use_sec;			///<占有セクタ数
	u32 start_ofs;		///<セーブデータでの開始オフセット
	u32 size;			///<セーブデータの占有サイズ
}SVBLK_INFO;

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ項目ごとの情報定義
 */
//---------------------------------------------------------------------------
typedef struct {
	GMDATA_ID gmdataID;	///<セーブデータ識別ID
	u32 size;			///<データサイズ格納
	u32 address;		///<データ開始位置
	u16 crc;			///<エラー検出用CRCコード格納
	u16 blockID;		///<所属ブロックのID
}SVPAGE_INFO;

//---------------------------------------------------------------------------
/**
 * @brief	セーブワーク構造体
 *
 * 実際のセーブされる部分の構造
 */
//---------------------------------------------------------------------------
typedef struct {
	u8 data[SECTOR_SIZE * SECTOR_MAX];	///<実際のデータ保持領域
}SAVEWORK;


//---------------------------------------------------------------------------
/**
 * @brief	分割転送制御用ワーク
 */
//---------------------------------------------------------------------------
typedef struct {
	BOOL total_save_mode;
	int block_start;
	int block_current;
	int block_end;
	s32 lock_id;
	int div_seq;
	u32 g_backup;
	u32 b_backup[SVBLK_ID_MAX];
	volatile BOOL lock_flg;
}NEWDIVSV_WORK;

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ構造体
 *
 * セーブデータ自体と、制御用ワークをまとめたもの
 */
//---------------------------------------------------------------------------
typedef struct {
	BOOL flash_exists;			///<バックアップFLASHが存在するかどうか
	BOOL data_exists;			///<データが存在するかどうか
	BOOL new_data_flag;			///<「さいしょから」のデータかどうか
	BOOL total_save_flag;		///<全体セーブが必要かどうかのフラグ
//	LOAD_RESULT first_status;	///<一番最初のセーブデータチェック結果
	u32 first_status;			///<一番最初のセーブデータチェック結果(bit指定)
#if !(CRC_LOADCHECK)
	MATHCRC16Table crc_table;	///<CRC算出用テーブル
#endif //CRC_LOADCHECK
	SAVEWORK svwk;				///<セーブデータ本体

	u32 global_counter;
	u32 current_counters[SVBLK_ID_MAX];
	u8 current_side[SVBLK_ID_MAX];

	///セーブ項目データ情報
	SVPAGE_INFO pageinfo[GMDATA_ID_MAX];

	///セーブブロックデータ情報
	SVBLK_INFO blkinfo[SVBLK_ID_MAX];

	///分割転送制御用ワーク
	NEWDIVSV_WORK ndsw;

	int dendou_sector_switch;
	u32 dendou_counter;
}_SAVEDATA_PT;



