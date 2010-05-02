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
	PMSW_SRC_PICTURE,
	PMSW_SRC_KOE,
};

//----------------------------------------------------------------
/**
 * 各gmmファイル毎の要素数
*/
//----------------------------------------------------------------

static const u16 PMS_SrcElems[] = {
	 652,	//ポケモン名	0 ... 651
	 558,	//わざ名	652 ... 1209
	  17,	//ポケモンタイプ名	1210 ... 1226
	 165,	//とくせい名	1227 ... 1391
	  44,	//トレーナー	1392 ... 1435
	  38,	//ひと	1436 ... 1473
	  48,	//あいさつ	1474 ... 1521
	 103,	//せいかつ	1522 ... 1624
	  47,	//きもち	1625 ... 1671
	  32,	//なんかいことば	1672 ... 1703
	  26,	//ユニオン	1704 ... 1729
	  10,	//ピクチャ	1730 ... 1739
	  61,	//こえ	1740 ... 1800
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
	NARC_message_pms_picture_dat,
	NARC_message_pms_word13_dat,
};

//----------------------------------------------------------------
/**
 * 重複単語テーブル
*/
//----------------------------------------------------------------

static const PMS_WORD DupWord_00[] = {
	1483,1736,
};

static const PMS_WORD DupWord_01[] = {
	1100,1534,
};

static const PMS_WORD DupWord_02[] = {
	1028,1400,
};

static const PMS_WORD DupWord_03[] = {
	 93,1217,
};

static const PMS_WORD DupWord_04[] = {
	1496,1738,
};

static const PMS_WORD DupWord_05[] = {
	1289,1405,
};

static const PMS_WORD DupWord_06[] = {
	1474,1731,
};

static const PMS_WORD DupWord_07[] = {
	1646,1732,
};

static const PMS_WORD DupWord_08[] = {
	1512,1739,
};

static const PMS_WORD DupWord_09[] = {
	1513,1730,
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
	{ DupWord_09, 2 },
};

//----------------------------------------------------------------
/**
 * 関連定数
*/
//----------------------------------------------------------------

#define  PMS_SRCFILE_MAX  (13)

#undef __PMS_WORD_RES__
#endif

