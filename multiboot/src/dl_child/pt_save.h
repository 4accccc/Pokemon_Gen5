
#define CRC_LOADCHECK (1)
//---------------------------------------------------------------------------
///	セーブに使用しているセクタ数
//---------------------------------------------------------------------------
#define	SAVE_PAGE_MAX		(32)

#define	SAVE_SECTOR_SIZE	(0x1000)
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

	GMDATA_NORMAL_FOTTER,	//WBで追加。セーブのCRCとかカウンタとか入ってる
	//ボックスデータグループ
	GMDATA_ID_BOXDATA,

	GMDATA_BOX_FOTTER,		//WBで追加。ボックスのCRCとかカウンタとか入ってる

	GMDATA_ID_MAX,
}PT_GMDATA_ID;
//=============================================================================
//=============================================================================

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define MAGIC_NUMBER_PT	(0x20060623)
#define	SECTOR_SIZE		(SAVE_SECTOR_SIZE)
#define SECTOR_MAX		(SAVE_PAGE_MAX)

#define	FIRST_MIRROR_START	(0)
#define	SECOND_MIRROR_START	(64)

#define MIRROR1ST	(0)
#define	MIRROR2ND	(1)
#define	MIRRORERROR	(2)

#define HEAPID_SAVE_TEMP	(HEAPID_BASE_APP)

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
 * @brief	セーブワーク構造体
 *
 * 実際のセーブされる部分の構造
 */
//---------------------------------------------------------------------------
typedef struct {
	u8 data[SECTOR_SIZE * SECTOR_MAX];	///<実際のデータ保持領域
}SAVEWORK;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define	WAZA_TEMOTI_MAX		(4)		///<1体のポケモンがもてる技の最大値
#define PERSON_NAME_SIZE	7	// 人物の名前の長さ（自分も含む）
#define MONS_NAME_SIZE		10	// ポケモン名の長さ(バッファサイズ EOM_含まず)
#define	EOM_SIZE			1	// 終了コードの長さ
#define BOX_MAX_TRAY			(18)
#define BOX_MAX_POS				(5*6)
#define BOX_TRAYNAME_BUFSIZE	(20)	// 日本語８文字＋EOM。海外版用の余裕も見てこの程度。
typedef struct pokemon_para_data11 POKEMON_PARA_DATA11;
typedef struct pokemon_para_data12 POKEMON_PARA_DATA12;
typedef struct pokemon_para_data13 POKEMON_PARA_DATA13;
typedef struct pokemon_para_data14 POKEMON_PARA_DATA14;
typedef struct pokemon_para_data15 POKEMON_PARA_DATA15;
typedef struct pokemon_para_data16 POKEMON_PARA_DATA16;
typedef struct pokemon_para_data21 POKEMON_PARA_DATA21;
typedef struct pokemon_para_data22 POKEMON_PARA_DATA22;
typedef struct pokemon_para_data23 POKEMON_PARA_DATA23;
typedef struct pokemon_para_data24 POKEMON_PARA_DATA24;
typedef struct pokemon_para_data25 POKEMON_PARA_DATA25;
typedef struct pokemon_para_data26 POKEMON_PARA_DATA26;
typedef struct pokemon_para_data31 POKEMON_PARA_DATA31;
typedef struct pokemon_para_data32 POKEMON_PARA_DATA32;
typedef struct pokemon_para_data33 POKEMON_PARA_DATA33;
typedef struct pokemon_para_data34 POKEMON_PARA_DATA34;
typedef struct pokemon_para_data35 POKEMON_PARA_DATA35;
typedef struct pokemon_para_data36 POKEMON_PARA_DATA36;
typedef struct pokemon_para_data41 POKEMON_PARA_DATA41;
typedef struct pokemon_para_data42 POKEMON_PARA_DATA42;
typedef struct pokemon_para_data43 POKEMON_PARA_DATA43;
typedef struct pokemon_para_data44 POKEMON_PARA_DATA44;
typedef struct pokemon_para_data45 POKEMON_PARA_DATA45;
typedef struct pokemon_para_data46 POKEMON_PARA_DATA46;

typedef struct pt_box_data PT_BOX_DATA;
typedef struct pt_pokemon_param PT_POKEMON_PARAM;
typedef struct pokemon_paso_param1 POKEMON_PASO_PARAM1;
typedef struct pokemon_paso_param2 POKEMON_PASO_PARAM2;
typedef struct pokemon_paso_param3 POKEMON_PASO_PARAM3;
typedef struct pokemon_paso_param4 POKEMON_PASO_PARAM4;

struct pokemon_paso_param1
{
	u16	monsno;								//02h	モンスターナンバー
	u16 item;								//04h	所持アイテムナンバー
	u32	id_no;								//08h	IDNo
	u32	exp;								//0ch	経験値
	u8	friend;								//0dh	なつき度
	u8	speabino;							//0eh	特殊能力
	u8	mark;								//0fh	ポケモンにつけるマーク（ボックス）
	u8	country_code;						//10h	国コード
	u8	hp_exp;								//11h	HP努力値
	u8	pow_exp;							//12h	攻撃力努力値
	u8	def_exp;							//13h	防御力努力値
	u8	agi_exp;							//14h	素早さ努力値
	u8	spepow_exp;							//15h	特攻努力値
	u8	spedef_exp;							//16h	特防努力値
	u8	style;								//17h	かっこよさ
	u8	beautiful;							//18h	うつくしさ
	u8	cute;								//19h	かわいさ
	u8	clever;								//1ah	かしこさ
	u8	strong;								//1bh	たくましさ
	u8	fur;								//1ch	毛艶
	u32	sinou_ribbon;						//20h	シンオウ系リボン（ビットフィールドでアクセスするとプログラムメモリを消費するので、ビットシフトに変更）
//以下内訳
#if 0
	u32	sinou_champ_ribbon					:1;	//シンオウチャンプリボン
	u32	sinou_battle_tower_ttwin_first		:1;	//シンオウバトルタワータワータイクーン勝利1回目
	u32	sinou_battle_tower_ttwin_second		:1;	//シンオウバトルタワータワータイクーン勝利1回目
	u32	sinou_battle_tower_2vs2_win50		:1;	//シンオウバトルタワータワーダブル50連勝
	u32	sinou_battle_tower_aimulti_win50	:1;	//シンオウバトルタワータワーAIマルチ50連勝
	u32	sinou_battle_tower_siomulti_win50	:1;	//シンオウバトルタワータワー通信マルチ50連勝
	u32	sinou_battle_tower_wifi_rank5		:1;	//シンオウバトルタワーWifiランク５入り
	u32	sinou_syakki_ribbon					:1;	//シンオウしゃっきリボン
	u32	sinou_dokki_ribbon					:1;	//シンオウどっきリボン
	u32	sinou_syonbo_ribbon					:1;	//シンオウしょんぼリボン

	u32	sinou_ukka_ribbon					:1;	//シンオウうっかリボン
	u32	sinou_sukki_ribbon					:1;	//シンオウすっきリボン
	u32	sinou_gussu_ribbon					:1;	//シンオウぐっすリボン
	u32	sinou_nikko_ribbon					:1;	//シンオウにっこリボン
	u32	sinou_gorgeous_ribbon				:1;	//シンオウゴージャスリボン
	u32	sinou_royal_ribbon					:1;	//シンオウロイヤルリボン
	u32	sinou_gorgeousroyal_ribbon			:1;	//シンオウゴージャスロイヤルリボン
	u32	sinou_ashiato_ribbon				:1;	//シンオウあしあとリボン
	u32	sinou_record_ribbon					:1;	//シンオウレコードリボン
	u32	sinou_history_ribbon				:1;	//シンオウヒストリーリボン

	u32	sinou_legend_ribbon					:1;	//シンオウレジェンドリボン
	u32	sinou_red_ribbon					:1;	//シンオウレッドリボン
	u32	sinou_green_ribbon					:1;	//シンオウグリーンリボン
	u32	sinou_blue_ribbon					:1;	//シンオウブルーリボン
	u32	sinou_festival_ribbon				:1;	//シンオウフェスティバルリボン
	u32	sinou_carnival_ribbon				:1;	//シンオウカーニバルリボン
	u32	sinou_classic_ribbon				:1;	//シンオウクラシックリボン
	u32	sinou_premiere_ribbon				:1;	//シンオウプレミアリボン

	u32	sinou_amari_ribbon					:4;	//20h	あまり
#endif
} ;
	
struct pokemon_paso_param2  
{
	u16	waza[WAZA_TEMOTI_MAX];				//08h	所持技
	u8	pp[WAZA_TEMOTI_MAX];				//0ch	所持技PP
	u8	pp_count[WAZA_TEMOTI_MAX];			//10h	所持技PP_COUNT
	u32	hp_rnd			:5;					//		HP乱数
	u32	pow_rnd			:5;					//		攻撃力乱数
	u32	def_rnd			:5;					//		防御力乱数
	u32	agi_rnd			:5;					//		素早さ乱数
	u32	spepow_rnd		:5;					//		特攻乱数
	u32	spedef_rnd		:5;					//		特防乱数
	u32	tamago_flag		:1;					//		タマゴフラグ（0:タマゴじゃない　1:タマゴだよ）
	u32	nickname_flag	:1;					//14h	ニックネームをつけたかどうかフラグ（0:つけていない　1:つけた）

	u32	old_ribbon;							//18h	過去作のリボン系（ビットフィールドでアクセスするとプログラムメモリを消費するので、ビットシフトに変更）
//以下内訳
#if 0
	u32	stylemedal_normal		:1;			//	かっこよさ勲章(ノーマル)(AGBコンテスト)
	u32	stylemedal_super		:1;			//	かっこよさ勲章(スーパー)(AGBコンテスト)
	u32	stylemedal_hyper		:1;			//	かっこよさ勲章(ハイパー)(AGBコンテスト)
	u32	stylemedal_master		:1;			//	かっこよさ勲章(マスター)(AGBコンテスト)
#define BOX_TRAYNAME_BUFSIZE	(20)	// 日本語８文字＋EOM。海外版用の余裕も見てこの程度。
	u32	beautifulmedal_normal	:1;			//	うつくしさ勲章(ノーマル)(AGBコンテスト)
	u32	beautifulmedal_super	:1;			//	うつくしさ勲章(スーパー)(AGBコンテスト)
	u32	beautifulmedal_hyper	:1;			//	うつくしさ勲章(ハイパー)(AGBコンテスト)
	u32	beautifulmedal_master	:1;			//	うつくしさ勲章(マスター)(AGBコンテスト)
	u32	cutemedal_normal		:1;			//	かわいさ勲章(ノーマル)(AGBコンテスト)
	u32	cutemedal_super			:1;			//	かわいさ勲章(スーパー)(AGBコンテスト)
	u32	cutemedal_hyper			:1;			//	かわいさ勲章(ハイパー)(AGBコンテスト)
	u32	cutemedal_master		:1;			//	かわいさ勲章(マスター)(AGBコンテスト)
	u32	clevermedal_normal		:1;			//	かしこさ勲章(ノーマル)(AGBコンテスト)
	u32	clevermedal_super		:1;			//	かしこさ勲章(スーパー)(AGBコンテスト)
	u32	clevermedal_hyper		:1;			//	かしこさ勲章(ハイパー)(AGBコンテスト)
	u32	clevermedal_master		:1;			//	かしこさ勲章(マスター)(AGBコンテスト)
	u32	strongmedal_normal		:1;			//	たくましさ勲章(ノーマル)(AGBコンテスト)
	u32	strongmedal_super		:1;			//	たくましさ勲章(スーパー)(AGBコンテスト)
	u32	strongmedal_hyper		:1;			//	たくましさ勲章(ハイパー)(AGBコンテスト)
	u32	strongmedal_master		:1;			//	たくましさ勲章(マスター)(AGBコンテスト)

	u32	champ_ribbon	:1;					//		チャンプリボン
	u32	winning_ribbon	:1;					//		ウィニングリボン
	u32	victory_ribbon	:1;					//		ビクトリーリボン
	u32	bromide_ribbon	:1;					//		ブロマイドリボン
	u32	ganba_ribbon	:1;					//		がんばリボン
	u32	marine_ribbon	:1;				//ポケモンパラメータ取得系の定義
enum{
	ID_POKEPARADATA11=0,	//0
	ID_POKEPARADATA12,		//1
	ID_POKEPARADATA13,		//2
	ID_POKEPARADATA14,		//3
	ID_POKEPARADATA15,		//4
	ID_POKEPARADATA16,		//5
	ID_POKEPARADATA21,		//6
	ID_POKEPARADATA22,		//7
	ID_POKEPARADATA23,		//8
	ID_POKEPARADATA24,		//9
	ID_POKEPARADATA25,		//10
	ID_POKEPARADATA26,		//11
	ID_POKEPARADATA31,		//12
	ID_POKEPARADATA32,		//13
	ID_POKEPARADATA33,		//14
	ID_POKEPARADATA34,		//15
	ID_POKEPARADATA35,		//16
	ID_POKEPARADATA36,		//17
	ID_POKEPARADATA41,		//18
	ID_POKEPARADATA42,		//19
	ID_POKEPARADATA43,		//20
	ID_POKEPARADATA44,		//21
	ID_POKEPARADATA45,		//22
	ID_POKEPARADATA46,		//23

	ID_POKEPARADATA51,		//24
	ID_POKEPARADATA52,		//25
	ID_POKEPARADATA53,		//26
	ID_POKEPARADATA54,		//27
	ID_POKEPARADATA55,		//28
	ID_POKEPARADATA56,		//29

	ID_POKEPARADATA61,		//30
	ID_POKEPARADATA62,		//31
};

enum{
	ID_POKEPARA1=0,
	ID_POKEPARA2,
	ID_POKEPARA3,
	ID_POKEPARA4,
};
	//		マリンリボン
	u32	land_ribbon		:1;					//		ランドリボン
	u32	sky_ribbon		:1;					//		スカイリボン
	u32	country_ribbon	:1;					//		カントリーリボン
	u32	national_ribbon	:1;					//		ナショナルリボン
	u32	earth_ribbon	:1;					//		アースリボン
	u32	world_ribbon	:1;					//		ワールドリボン
#endif

	u8	event_get_flag	:1;					//		イベントで配布したことを示すフラグ
	u8	sex				:2;					//   	ポケモンの性別
	u8	form_no			:5;					//19h	形状ナンバー（アンノーン、デオキシス、ミノメスなど用）
	u8	dummy_p2_1;							//1ah	あまり
	u16	dummy_p2_2;							//1ch	あまり
	u16	new_get_place;						//1eh	捕まえた場所（なぞの場所対応用）
	u16	new_birth_place;					//20h	生まれた場所（なぞの場所対応用）
};
	
struct pokemon_paso_param3 
{
	STRCODE	nickname[MONS_NAME_SIZE+EOM_SIZE];	//16h	ニックネーム(MONS_NAME_SIZE=10)+(EOM_SIZE=1)=11
	u8	pref_code;								//18h	都道府県コード
	u8	get_cassette;							//		捕まえたカセットバージョン
	u64	new_ribbon;								//20h	新リボン系（ビットフィールドでアクセスするとプログラムメモリを消費するので、ビットシフトに変更）
//以下内訳
#if 0
	u32	trial_stylemedal_normal			:1;		//	かっこよさ勲章(ノーマル)(トライアル)
	u32	trial_stylemedal_super			:1;		//	かっこよさ勲章(スーパー)(トライアル)
	u32	trial_stylemedal_hyper			:1;		//	かっこよさ勲章(ハイパー)(トライアル)
	u32	trial_stylemedal_master			:1;		//	かっこよさ勲章(マスター)(トライアル)
	u32	trial_beautifulmedal_normal		:1;		//	うつくしさ勲章(ノーマル)(トライアル)
	u32	trial_beautifulmedal_super		:1;		//	うつくしさ勲章(スーパー)(トライアル)
	u32	trial_beautifulmedal_hyper		:1;		//	うつくしさ勲章(ハイパー)(トライアル)
	u32	trial_beautifulmedal_master		:1;		//	うつくしさ勲章(マスター)(トライアル)
	u32	trial_cutemedal_normal			:1;		//	かわいさ勲章(ノーマル)(トライアル)
	u32	trial_cutemedal_super			:1;		//	かわいさ勲章(スーパー)(トライアル)
	u32	trial_cutemedal_hyper			:1;		//	かわいさ勲章(ハイパー)(トライアル)
	u32	trial_cutemedal_master			:1;		//	かわいさ勲章(マスター)(トライアル)
	u32	trial_clevermedal_normal		:1;		//	かしこさ勲章(ノーマル)(トライアル)
	u32	trial_clevermedal_super			:1;		//	かしこさ勲章(スーパー)(トライアル)
	u32	trial_clevermedal_hyper			:1;		//	かしこさ勲章(ハイパー)(トライアル)
	u32	trial_clevermedal_master		:1;		//	かしこさ勲章(マスター)(トライアル)
	u32	trial_strongmedal_normal		:1;		//	たくましさ勲章(ノーマル)(トライアル)
	u32	trial_strongmedal_super			:1;		//	たくましさ勲章(スーパー)(トライアル)
	u32	trial_strongmedal_hyper			:1;		//	たくましさ勲章(ハイパー)(トライアル)
	u32	trial_strongmedal_master		:1;		//	たくましさ勲章(マスター)(トライアル)
	u32 amari_ribbon					:12;	//20h	あまり
	u32	amari;									//20h	あまり
#endif
};
	
struct pokemon_paso_param4
{
	STRCODE	oyaname[7+EOM_SIZE];				//10h	親の名前(PERSON_NAME_SIZE=7)+(EOM_SIZE_=1)=8*2(STRCODE=u16)

	u8	get_year;								//11h	捕まえた年
	u8	get_month;								//12h	捕まえた月
	u8	get_day;								//13h	捕まえた日
	u8	birth_year;								//14h	生まれた年

	u8	birth_month;							//15h	生まれた月
	u8	birth_day;								//16h	生まれた日
	u16	get_place;								//18h	捕まえた場所

	u16	birth_place;							//1ah	生まれた場所
	u8	pokerus;								//1bh	ポケルス
	u8	get_ball;								//1ch	捕まえたボール

	u8	get_level		:7;						//1dh	捕まえたレベル
	u8	oyasex			:1;						//1dh	親の性別
	u8	get_ground_id;							//1eh	捕まえた場所の地形アトリビュート（ミノッチ用）
	u16	dummy_p4_1;								//20h	あまり
};

///<ボックスポケモン分の構造体宣言
struct pt_pokemon_param 
{
	u32	personal_rnd;							//04h	個性乱数
	u16	pp_fast_mode		:1;					//06h	暗号／復号／チェックサム生成を後回しにして、処理を高速化モード
	u16	ppp_fast_mode		:1;					//06h	暗号／復号／チェックサム生成を後回しにして、処理を高速化モード
	u16	fusei_tamago_flag	:1;					//06h	ダメタマゴフラグ
	u16						:13;				//06h
	u16	checksum;								//08h	チェックサム

	u8	paradata[sizeof(POKEMON_PASO_PARAM1)+
				 sizeof(POKEMON_PASO_PARAM2)+
				 sizeof(POKEMON_PASO_PARAM3)+
				 sizeof(POKEMON_PASO_PARAM4)];	//88h
};


struct pt_box_data
{
	u32					currentTrayNumber;
	PT_POKEMON_PARAM	ppp[BOX_MAX_TRAY][BOX_MAX_POS];
	STRCODE				trayName[BOX_MAX_TRAY][BOX_TRAYNAME_BUFSIZE];
	u8					wallPaper[BOX_MAX_TRAY];
	u8					daisukiBitFlag;
};


//ポケモンパラメータ取得系の定義
enum{
	ID_POKEPARADATA11=0,	//0
	ID_POKEPARADATA12,		//1
	ID_POKEPARADATA13,		//2
	ID_POKEPARADATA14,		//3
	ID_POKEPARADATA15,		//4
	ID_POKEPARADATA16,		//5
	ID_POKEPARADATA21,		//6
	ID_POKEPARADATA22,		//7
	ID_POKEPARADATA23,		//8
	ID_POKEPARADATA24,		//9
	ID_POKEPARADATA25,		//10
	ID_POKEPARADATA26,		//11
	ID_POKEPARADATA31,		//12
	ID_POKEPARADATA32,		//13
	ID_POKEPARADATA33,		//14
	ID_POKEPARADATA34,		//15
	ID_POKEPARADATA35,		//16
	ID_POKEPARADATA36,		//17
	ID_POKEPARADATA41,		//18
	ID_POKEPARADATA42,		//19
	ID_POKEPARADATA43,		//20
	ID_POKEPARADATA44,		//21
	ID_POKEPARADATA45,		//22
	ID_POKEPARADATA46,		//23

	ID_POKEPARADATA51,		//24
	ID_POKEPARADATA52,		//25
	ID_POKEPARADATA53,		//26
	ID_POKEPARADATA54,		//27
	ID_POKEPARADATA55,		//28
	ID_POKEPARADATA56,		//29

	ID_POKEPARADATA61,		//30
	ID_POKEPARADATA62,		//31
};

enum{
	ID_POKEPARA1=0,
	ID_POKEPARA2,
	ID_POKEPARA3,
	ID_POKEPARA4,
};

struct pokemon_para_data11
{
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM4	ppp4;
};

struct pokemon_para_data12
{
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM3	ppp3;
};

struct pokemon_para_data13
{
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM4	ppp4;
};

struct pokemon_para_data14
{
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM2	ppp2;
};

struct pokemon_para_data15
{
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM3	ppp3;
};

struct pokemon_para_data16
{
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM2	ppp2;
};

struct pokemon_para_data21
{
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM4	ppp4;
};

struct pokemon_para_data22
{
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM3	ppp3;
};

struct pokemon_para_data23
{
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM4	ppp4;
};

struct pokemon_para_data24
{
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM1	ppp1;
};

struct pokemon_para_data25
{
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM3	ppp3;
};

struct pokemon_para_data26
{
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM1	ppp1;
};

struct pokemon_para_data31
{
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM4	ppp4;
};

struct pokemon_para_data32
{
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM2	ppp2;
};

struct pokemon_para_data33
{
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM4	ppp4;
};

struct pokemon_para_data34
{
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM1	ppp1;
};

struct pokemon_para_data35
{
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM2	ppp2;
};

struct pokemon_para_data36
{
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM1	ppp1;
};

struct pokemon_para_data41
{
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM3	ppp3;
};

struct pokemon_para_data42
{
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM2	ppp2;
};

struct pokemon_para_data43
{
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM3	ppp3;
};

struct pokemon_para_data44
{
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM1	ppp1;
};

struct pokemon_para_data45
{
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM1	ppp1;
	POKEMON_PASO_PARAM2	ppp2;
};

struct pokemon_para_data46
{
	POKEMON_PASO_PARAM4	ppp4;
	POKEMON_PASO_PARAM3	ppp3;
	POKEMON_PASO_PARAM2	ppp2;
	POKEMON_PASO_PARAM1	ppp1;
};


