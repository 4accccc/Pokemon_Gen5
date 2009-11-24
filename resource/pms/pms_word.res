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
	 583,	//ポケモン名	0 ... 582
	 529,	//わざ名	583 ... 1111
	  18,	//ポケモンタイプ名	1112 ... 1129
	 124,	//とくせい名	1130 ... 1253
	  38,	//トレーナー	1254 ... 1291
	  38,	//ひと	1292 ... 1329
	 107,	//あいさつ	1330 ... 1436
	 104,	//せいかつ	1437 ... 1540
	  10,	//きもち	1541 ... 1550
	  32,	//なんかいことば	1551 ... 1582
	  23,	//ユニオン	1583 ... 1605
	  10,	//ピクチャ	1606 ... 1615
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
	1340,1547,1612,
};

static const PMS_WORD DupWord_01[] = {
	1546,1611,
};

static const PMS_WORD DupWord_02[] = {
	1031,1448,
};

static const PMS_WORD DupWord_03[] = {
	1544,1609,
};

static const PMS_WORD DupWord_04[] = {
	959,1260,
};

static const PMS_WORD DupWord_05[] = {
	 93,1119,
};

static const PMS_WORD DupWord_06[] = {
	1380,1549,1614,
};

static const PMS_WORD DupWord_07[] = {
	1192,1264,
};

static const PMS_WORD DupWord_08[] = {
	1330,1542,1607,
};

static const PMS_WORD DupWord_09[] = {
	1548,1613,
};

static const PMS_WORD DupWord_10[] = {
	1543,1608,
};

static const PMS_WORD DupWord_11[] = {
	1545,1610,
};

static const PMS_WORD DupWord_12[] = {
	1403,1550,1615,
};

static const PMS_WORD DupWord_13[] = {
	1404,1541,1606,
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
};

//----------------------------------------------------------------
/**
 * 関連定数
*/
//----------------------------------------------------------------

#define  PMS_SRCFILE_MAX  (12)

#undef __PMS_WORD_RES__
#endif

