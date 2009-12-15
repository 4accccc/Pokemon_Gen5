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
	 626,	//ポケモン名	0 ... 625
	 529,	//わざ名	626 ... 1154
	  17,	//ポケモンタイプ名	1155 ... 1171
	 160,	//とくせい名	1172 ... 1331
	  38,	//トレーナー	1332 ... 1369
	  38,	//ひと	1370 ... 1407
	 107,	//あいさつ	1408 ... 1514
	 104,	//せいかつ	1515 ... 1618
	  10,	//きもち	1619 ... 1628
	  32,	//なんかいことば	1629 ... 1660
	  23,	//ユニオン	1661 ... 1683
	  10,	//ピクチャ	1684 ... 1693
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
	1418,1625,1690,
};

static const PMS_WORD DupWord_01[] = {
	1624,1689,
};

static const PMS_WORD DupWord_02[] = {
	1074,1526,
};

static const PMS_WORD DupWord_03[] = {
	1622,1687,
};

static const PMS_WORD DupWord_04[] = {
	1002,1338,
};

static const PMS_WORD DupWord_05[] = {
	 93,1162,
};

static const PMS_WORD DupWord_06[] = {
	1458,1627,1692,
};

static const PMS_WORD DupWord_07[] = {
	1234,1342,
};

static const PMS_WORD DupWord_08[] = {
	1408,1620,1685,
};

static const PMS_WORD DupWord_09[] = {
	1626,1691,
};

static const PMS_WORD DupWord_10[] = {
	1621,1686,
};

static const PMS_WORD DupWord_11[] = {
	1623,1688,
};

static const PMS_WORD DupWord_12[] = {
	1481,1628,1693,
};

static const PMS_WORD DupWord_13[] = {
	1482,1619,1684,
};

static const PMS_WORD DupWord_14[] = {
	770,1322,
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

