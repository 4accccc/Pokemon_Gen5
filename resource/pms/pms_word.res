// このファイルはコンバータで自動生成されています
// 手作業での編集は行わないようにお願いします
// Game Freak   taya


#ifdef __PMS_WORD_RES__	// 単一のファイルからのみincludeを許可する

//----------------------------------------------------------------
/**
 * 各gmmファイル指定のための定数
*/
//----------------------------------------------------------------

enum {
	PMSW_SRC_POKEMON,
	PMSW_SRC_SKILL,
	PMSW_SRC_TYPE,
	PMSW_SRC_TOKUSEI,
	PMSW_SRC_TRAINER,
	PMSW_SRC_HITO,
	PMSW_SRC_AISATSU,
	PMSW_SRC_SEIKATSU,
	PMSW_SRC_KIMOTI,
	PMSW_SRC_NANKAI,
	PMSW_SRC_UNION,
};

//----------------------------------------------------------------
/**
 * 各gmmファイル毎の要素数
*/
//----------------------------------------------------------------

static const u16 PMS_SrcElems[] = {
	 583,	//ポケモン名	0 ... 582
	 468,	//わざ名	583 ... 1050
	  18,	//ポケモンタイプ名	1051 ... 1068
	 124,	//とくせい名	1069 ... 1192
	  38,	//トレーナー	1193 ... 1230
	  38,	//ひと	1231 ... 1268
	 107,	//あいさつ	1269 ... 1375
	 104,	//せいかつ	1376 ... 1479
	  10,	//きもち	1480 ... 1489
	  32,	//なんかいことば	1490 ... 1521
	  23,	//ユニオン	1522 ... 1544
};

//----------------------------------------------------------------
/**
 * 各gmmファイルのアーカイブ内ファイルID
*/
//----------------------------------------------------------------

static const u16 PMS_SrcFileID[] = {
	NARC_message_monsname_dat,
	NARC_message_wazaname_dat,
	NARC_message_typename_dat,
	NARC_message_tokusei_dat,
	NARC_message_pms_word06_dat,
	NARC_message_pms_word07_dat,
	NARC_message_pms_word08_dat,
	NARC_message_pms_word09_dat,
	NARC_message_pms_word10_dat,
	NARC_message_pms_word11_dat,
	NARC_message_pms_word12_dat,
};

//----------------------------------------------------------------
/**
 * 重複単語テーブル
*/
//----------------------------------------------------------------

static const PMS_WORD DupWord_00[] = {
	1279,1486,
};

static const PMS_WORD DupWord_01[] = {
	1031,1387,
};

static const PMS_WORD DupWord_02[] = {
	959,1199,
};

static const PMS_WORD DupWord_03[] = {
	 93,1058,
};

static const PMS_WORD DupWord_04[] = {
	1319,1488,
};

static const PMS_WORD DupWord_05[] = {
	1131,1203,
};

static const PMS_WORD DupWord_06[] = {
	1269,1481,
};

static const PMS_WORD DupWord_07[] = {
	1342,1489,
};

static const PMS_WORD DupWord_08[] = {
	1343,1480,
};

static const struct {
    const PMS_WORD* data;
    int   count;
}DupWordTable[] = {
	{ DupWord_00, 2 },
	{ DupWord_01, 2 },
	{ DupWord_02, 2 },
	{ DupWord_03, 2 },
	{ DupWord_04, 2 },
	{ DupWord_05, 2 },
	{ DupWord_06, 2 },
	{ DupWord_07, 2 },
	{ DupWord_08, 2 },
};

//----------------------------------------------------------------
/**
 * 関連定数
*/
//----------------------------------------------------------------

#define  PMS_SRCFILE_MAX  (11)

#undef __PMS_WORD_RES__
#endif

