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
};

//----------------------------------------------------------------
/**
 * 各gmmファイル毎の要素数
*/
//----------------------------------------------------------------

static const u16 PMS_SrcElems[] = {
	 652,	//ポケモン名	0 ... 651
	 545,	//わざ名	652 ... 1196
	  17,	//ポケモンタイプ名	1197 ... 1213
	 160,	//とくせい名	1214 ... 1373
	  38,	//トレーナー	1374 ... 1411
	  38,	//ひと	1412 ... 1449
	 107,	//あいさつ	1450 ... 1556
	 104,	//せいかつ	1557 ... 1660
	  10,	//きもち	1661 ... 1670
	  32,	//なんかいことば	1671 ... 1702
	  23,	//ユニオン	1703 ... 1725
	  10,	//ピクチャ	1726 ... 1735
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
};

//----------------------------------------------------------------
/**
 * 重複単語テーブル
*/
//----------------------------------------------------------------

static const PMS_WORD DupWord_00[] = {
	1460,1667,1732,
};

static const PMS_WORD DupWord_01[] = {
	1666,1731,
};

static const PMS_WORD DupWord_02[] = {
	1100,1568,
};

static const PMS_WORD DupWord_03[] = {
	1664,1729,
};

static const PMS_WORD DupWord_04[] = {
	1028,1380,
};

static const PMS_WORD DupWord_05[] = {
	 93,1204,
};

static const PMS_WORD DupWord_06[] = {
	1500,1669,1734,
};

static const PMS_WORD DupWord_07[] = {
	1276,1384,
};

static const PMS_WORD DupWord_08[] = {
	1450,1662,1727,
};

static const PMS_WORD DupWord_09[] = {
	1668,1733,
};

static const PMS_WORD DupWord_10[] = {
	1663,1728,
};

static const PMS_WORD DupWord_11[] = {
	1665,1730,
};

static const PMS_WORD DupWord_12[] = {
	1523,1670,1735,
};

static const PMS_WORD DupWord_13[] = {
	1524,1661,1726,
};

static const PMS_WORD DupWord_14[] = {
	796,1364,
};

static const struct {
    const PMS_WORD* data;
    int   count;
}DupWordTable[] = {
	{ DupWord_00, 3 },
	{ DupWord_01, 2 },
	{ DupWord_02, 2 },
	{ DupWord_03, 2 },
	{ DupWord_04, 2 },
	{ DupWord_05, 2 },
	{ DupWord_06, 3 },
	{ DupWord_07, 2 },
	{ DupWord_08, 3 },
	{ DupWord_09, 2 },
	{ DupWord_10, 2 },
	{ DupWord_11, 2 },
	{ DupWord_12, 3 },
	{ DupWord_13, 3 },
	{ DupWord_14, 2 },
};

//----------------------------------------------------------------
/**
 * 関連定数
*/
//----------------------------------------------------------------

#define  PMS_SRCFILE_MAX  (12)

#undef __PMS_WORD_RES__
#endif

