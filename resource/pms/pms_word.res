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
	 560,	//わざ名	652 ... 1211
	  17,	//ポケモンタイプ名	1212 ... 1228
	 165,	//とくせい名	1229 ... 1393
	  44,	//トレーナー	1394 ... 1437
	  38,	//ひと	1438 ... 1475
	  48,	//あいさつ	1476 ... 1523
	 103,	//せいかつ	1524 ... 1626
	  47,	//きもち	1627 ... 1673
	  32,	//なんかいことば	1674 ... 1705
	  26,	//ユニオン	1706 ... 1731
	  10,	//ピクチャ	1732 ... 1741
	  61,	//こえ	1742 ... 1802
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
	1100,1536,
};

static const PMS_WORD DupWord_01[] = {
	1028,1402,
};

static const PMS_WORD DupWord_02[] = {
	 93,1219,
};

static const PMS_WORD DupWord_03[] = {
	1291,1407,
};

static const struct {
    const PMS_WORD* data;
    int   count;
}DupWordTable[] = {
	{ DupWord_00, 2 },
	{ DupWord_01, 2 },
	{ DupWord_02, 2 },
	{ DupWord_03, 2 },
};

//----------------------------------------------------------------
/**
 * 関連定数
*/
//----------------------------------------------------------------

#define  PMS_SRCFILE_MAX  (13)

#undef __PMS_WORD_RES__
#endif

