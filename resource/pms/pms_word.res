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
	 163,	//とくせい名	1227 ... 1389
	  44,	//トレーナー	1390 ... 1433
	  38,	//ひと	1434 ... 1471
	  48,	//あいさつ	1472 ... 1519
	 103,	//せいかつ	1520 ... 1622
	  47,	//きもち	1623 ... 1669
	  32,	//なんかいことば	1670 ... 1701
	  26,	//ユニオン	1702 ... 1727
	  10,	//ピクチャ	1728 ... 1737
	  61,	//こえ	1738 ... 1798
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
	1481,1734,
};

static const PMS_WORD DupWord_01[] = {
	1100,1532,
};

static const PMS_WORD DupWord_02[] = {
	1028,1398,
};

static const PMS_WORD DupWord_03[] = {
	 93,1217,
};

static const PMS_WORD DupWord_04[] = {
	1494,1736,
};

static const PMS_WORD DupWord_05[] = {
	1289,1403,
};

static const PMS_WORD DupWord_06[] = {
	1472,1729,
};

static const PMS_WORD DupWord_07[] = {
	1644,1730,
};

static const PMS_WORD DupWord_08[] = {
	1510,1737,
};

static const PMS_WORD DupWord_09[] = {
	1511,1728,
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

