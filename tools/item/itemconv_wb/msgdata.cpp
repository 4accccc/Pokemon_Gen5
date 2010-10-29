#include "common.h"
#include "msgdata.h"

// アイテム表の文字テーブル
const s8 ListStrTbl[][3] = {
	"あ","い","う","え","お",	"か","き","く","け","こ",
	"さ","し","す","せ","そ",	"た","ち","つ","て","と",
	"な","に","ぬ","ね","の",	"は","ひ","ふ","へ","ほ",
	"ま","み","む","め","も",	"や","ゆ","よ",
	"ら","り","る","れ","ろ",	"わ","を","ん",
	"が","ぎ","ぐ","げ","ご",	"ざ","じ","ず","ぜ","ぞ",
	"だ","ぢ","づ","で","ど",	"ば","び","ぶ","べ","ぼ",
	"ぱ","ぴ","ぷ","ぺ","ぽ",	"っ","ゃ","ゅ","ょ",
	"ぁ","ぃ","ぅ","ぇ","ぉ",

	"ア","イ","ウ","エ","オ",	"カ","キ","ク","ケ","コ",
	"サ","シ","ス","セ","ソ",	"タ","チ","ツ","テ","ト",
	"ナ","ニ","ヌ","ネ","ノ",	"ハ","ヒ","フ","ヘ","ホ",
	"マ","ミ","ム","メ","モ",	"ヤ","ユ","ヨ",
	"ラ","リ","ル","レ","ロ",	"ワ","ヲ","ン",
	"ガ","ギ","グ","ゲ","ゴ",	"ザ","ジ","ズ","ゼ","ゾ",
	"ダ","ヂ","ヅ","デ","ド",	"バ","ビ","ブ","ベ","ボ",
	"パ","ピ","プ","ペ","ポ",	"ッ","ャ","ュ","ョ",
	"ァ","ィ","ゥ","ェ","ォ",

	"　","０","１","２","３","４","５","６","７","８","９",

	"Ａ","Ｂ","Ｃ","Ｄ","Ｅ","Ｆ","Ｇ","Ｈ","Ｉ","Ｊ","Ｋ",
	"Ｌ","Ｍ","Ｎ","Ｏ","Ｐ","Ｑ","Ｒ","Ｓ","Ｔ","Ｕ","Ｖ",
	"Ｗ","Ｘ","Ｙ","Ｚ",

	"ａ","ｂ","ｃ","ｄ","ｅ","ｆ","ｇ","ｈ","ｉ","ｊ","ｋ",
	"ｌ","ｍ","ｎ","ｏ","ｐ","ｑ","ｒ","ｓ","ｔ","ｕ","ｖ",
	"ｗ","ｘ","ｙ","ｚ",

	"！","？","。","ー","・","…","『","』","「","」","♂",
	"♀","円","．","×","／",
};

// ゲーム内フォント番号テーブル
const u16 PM_StrTbl[] = {
	 a_,  i_,  u_,  e_,  o_,	ka_, ki_, ku_, ke_, ko_,
	sa_, si_, su_, se_, so_,	ta_, ti_, tu_, te_, to_,
	na_, ni_, nu_, ne_, no_,	ha_, hi_, hu_, he_, ho_,
	ma_, mi_, mu_, me_, mo_,	ya_, yu_, yo_,
	ra_, ri_, ru_, re_, ro_,	wa_, wo_, n_,
	ga_, gi_, gu_, ge_, go_,	za_, zi_, zu_, ze_, zo_,
	da_, di_, du_, de_, do_,	ba_, bi_, bu_, be_, bo_,
	pa_, pi_, pu_, pe_, po_,	ttu_,yya_,yyu_,yyo_,
	aa_, ii_, uu_, ee_, oo_,

	 A_,  I_,  U_,  E_,  O_,	KA_, KI_, KU_, KE_, KO_,
	SA_, SI_, SU_, SE_, SO_,	TA_, TI_, TU_, TE_, TO_,
	NA_, NI_, NU_, NE_, NO_,	HA_, HI_, HU_, HE_, HO_,
	MA_, MI_, MU_, ME_, MO_,	YA_, YU_, YO_,
	RA_, RI_, RU_, RE_, RO_,	WA_, WO_, N_,
	GA_, GI_, GU_, GE_, GO_,	ZA_, ZI_, ZU_, ZE_, ZO_,
	DA_, DI_, DU_, DE_, DO_,	BA_, BI_, BU_, BE_, BO_,
	PA_, PI_, PU_, PE_, PO_,	TTU_,YYA_,YYU_,YYO_,
	AA_, II_, UU_, EE_, OO_,

	spc_, n0_, n1_, n2_, n3_, n4_, n5_, n6_, n7_, n8_, n9_,

	A__, B__, C__, D__, E__, F__, G__, H__, I__, J__, K__,
	L__, M__, N__, O__, P__, Q__, R__, S__, T__, U__, V__,
	W__, X__, Y__, Z__,

	a__, b__, c__, d__, e__, f__, g__, h__, i__, j__, k__,
	l__, m__, n__, o__, p__, q__, r__, s__, t__, u__, v__,
	w__, x__, y__, z__,

	gyoe_, hate_, kten_, bou_, spc_, tenten_, kako2_, kakot2_, kako_, kakot_, osu_,
	mesu_, yen_, ten_, times_, sura_,
};

static const s8	str00[] = "A";
static const s8	str01[] = "I";
static const s8	str02[] = "U";
static const s8	str03[] = "E";
static const s8	str04[] = "O";

static const s8	str05[] = "KA";
static const s8	str06[] = "KI";
static const s8	str07[] = "KU";
static const s8	str08[] = "KE";
static const s8	str09[] = "KO";

static const s8	str0a[] = "SA";
static const s8	str0b[] = "SI";
static const s8	str0c[] = "SU";
static const s8	str0d[] = "SE";
static const s8	str0e[] = "SO";
		
static const s8	str0f[] = "TA";
static const s8	str10[] = "TI";
static const s8	str11[] = "TU";
static const s8	str12[] = "TE";
static const s8	str13[] = "TO";

static const s8	str14[] = "NA";
static const s8	str15[] = "NI";
static const s8	str16[] = "NU";
static const s8	str17[] = "NE";
static const s8	str18[] = "NO";
		
static const s8	str19[] = "HA";
static const s8	str1a[] = "HI";
static const s8	str1b[] = "HU";
static const s8	str1c[] = "HE";
static const s8	str1d[] = "HO";

static const s8	str1e[] = "MA";
static const s8	str1f[] = "MI";
static const s8	str20[] = "MU";
static const s8	str21[] = "ME";
static const s8	str22[] = "MO";
		
static const s8	str23[] = "YA";
static const s8	str24[] = "YU";
static const s8	str25[] = "YO";

static const s8	str26[] = "RA";
static const s8	str27[] = "RI";
static const s8	str28[] = "RU";
static const s8	str29[] = "RE";
static const s8	str2a[] = "RO";
		
static const s8	str2b[] = "WA";
static const s8	str2c[] = "WO";
static const s8	str2d[] = "N";

static const s8	str2e[] = "GA";
static const s8	str2f[] = "GI";
static const s8	str30[] = "GU";
static const s8	str31[] = "GE";
static const s8	str32[] = "GO";

static const s8	str33[] = "ZA";
static const s8	str34[] = "ZI";
static const s8	str35[] = "ZU";
static const s8	str36[] = "ZE";
static const s8	str37[] = "ZO";

static const s8	str38[] = "DA";
static const s8	str39[] = "DI";
static const s8	str3a[] = "DU";
static const s8	str3b[] = "DE";
static const s8	str3c[] = "DO";
		
static const s8	str3d[] = "BA";
static const s8	str3e[] = "BI";
static const s8	str3f[] = "BU";
static const s8	str40[] = "BE";
static const s8	str41[] = "BO";

static const s8	str42[] = "PA";
static const s8	str43[] = "PI";
static const s8	str44[] = "PU";
static const s8	str45[] = "PE";
static const s8	str46[] = "PO";
		
static const s8	str4c[] = "0";
static const s8	str4d[] = "1";
static const s8	str4e[] = "2";
static const s8	str4f[] = "3";
static const s8	str50[] = "4";
static const s8	str51[] = "5";
static const s8	str52[] = "6";
static const s8	str53[] = "7";
static const s8	str54[] = "8";
static const s8	str55[] = "9";

static const s8	str56[] = "a";
static const s8	str57[] = "b";
static const s8	str58[] = "c";
static const s8	str59[] = "d";
static const s8	str5a[] = "e";
static const s8	str5b[] = "f";
static const s8	str5c[] = "g";
static const s8	str5d[] = "h";
static const s8	str5e[] = "i";
static const s8	str5f[] = "j";
static const s8	str60[] = "k";
static const s8	str61[] = "l";
static const s8	str62[] = "m";
static const s8	str63[] = "n";
static const s8	str64[] = "o";
static const s8	str65[] = "p";
static const s8	str66[] = "q";
static const s8	str67[] = "r";
static const s8	str68[] = "s";
static const s8	str69[] = "t";
static const s8	str6a[] = "u";
static const s8	str6b[] = "v";
static const s8	str6c[] = "w";
static const s8	str6d[] = "x";
static const s8	str6e[] = "y";
static const s8	str6f[] = "z";

const s8 * SymStrTbl[] = {
	str00,str01,str02,str03,str04,		str05,str06,str07,str08,str09,
	str0a,str0b,str0c,str0d,str0e,		str0f,str10,str11,str12,str13,
	str14,str15,str16,str17,str18,		str19,str1a,str1b,str1c,str1d,
	str1e,str1f,str20,str21,str22,		str23,str24,str25,
	str26,str27,str28,str29,str2a,		str2b,str2c,str2d,
	str2e,str2f,str30,str31,str32,		str33,str34,str35,str36,str37,
	str38,str39,str3a,str3b,str3c,		str3d,str3e,str3f,str40,str41,
	str42,str43,str44,str45,str46,		str00,str23,str24,str25,
	str00,str01,str02,str03,str04,

	str00,str01,str02,str03,str04,		str05,str06,str07,str08,str09,
	str0a,str0b,str0c,str0d,str0e,		str0f,str10,str11,str12,str13,
	str14,str15,str16,str17,str18,		str19,str1a,str1b,str1c,str1d,
	str1e,str1f,str20,str21,str22,		str23,str24,str25,
	str26,str27,str28,str29,str2a,		str2b,str2c,str2d,
	str2e,str2f,str30,str31,str32,		str33,str34,str35,str36,str37,
	str38,str39,str3a,str3b,str3c,		str3d,str3e,str3f,str40,str41,
	str42,str43,str44,str45,str46,		str00,str23,str24,str25,
	str00,str01,str02,str03,str04,

	str00,str4c,str4d,str4e,str4f,str50,str51,str52,str53,str54,str55,

	str56,str57,str58,str59,str5a,str5b,str5c,str5d,str5e,str5f,str60,str61,str62,
	str63,str64,str65,str66,str67,str68,str69,str6a,str6b,str6c,str6d,str6e,str6f,

	str56,str57,str58,str59,str5a,str5b,str5c,str5d,str5e,str5f,str60,str61,str62,
	str63,str64,str65,str66,str67,str68,str69,str6a,str6b,str6c,str6d,str6e,str6f,

	str00,str00,str00,str00,str00,str00,str00,str00,str00,str00,str00,str00,str00,str00,str00,str00,
};







u16 PM_StrGet( s8 * str )
{
	s8	cpy[3];
	u8	i;

	memcpy( cpy, str, 3 );
	cpy[2] = '\0';

	for( i=0; i<228; i++ ){
		if( strcmp( cpy, ListStrTbl[i] ) == 0 ){
			return PM_StrTbl[i];
		}
	}
	return EOM_;
}

const s8 * SYM_StrGet( s8 * str )
{
	s8	cpy[3];
	u8	i;

	memcpy( cpy, str, 3 );
	cpy[2] = '\0';

	for( i=0; i<228; i++ ){
		if( strcmp( cpy, ListStrTbl[i] ) == 0 ){
			return SymStrTbl[i];
		}
	}
	return SymStrTbl[0];
}

const s8 * PM_SymChange( u16 buf )
{
	u8	i;

	if( buf == EOM_ ){ return '\0'; }

	for( i=0; i<228; i++ ){
		if( buf == PM_StrTbl[i] ){
			return SymStrTbl[i];
		}
	}
	return '\0';
}




static u8 CheckStr( s8 * str )
{
	if( strcmp( str, "っ" ) == 0 ){ return 1; }
	if( strcmp( str, "ッ" ) == 0 ){ return 1; }

	if( strcmp( str, "ー" ) == 0 ){ return 2; }

	if( strcmp( str, "ゃ" ) == 0 ){ return 3; }
	if( strcmp( str, "ャ" ) == 0 ){ return 3; }
	if( strcmp( str, "ゅ" ) == 0 ){ return 4; }
	if( strcmp( str, "ュ" ) == 0 ){ return 4; }
	if( strcmp( str, "ょ" ) == 0 ){ return 5; }
	if( strcmp( str, "ョ" ) == 0 ){ return 5; }

	if( strcmp( str, "ぁ" ) == 0 ){ return 6; }
	if( strcmp( str, "ァ" ) == 0 ){ return 6; }
	if( strcmp( str, "ぃ" ) == 0 ){ return 7; }
	if( strcmp( str, "ィ" ) == 0 ){ return 7; }
	if( strcmp( str, "ぅ" ) == 0 ){ return 8; }
	if( strcmp( str, "ゥ" ) == 0 ){ return 8; }
	if( strcmp( str, "ぇ" ) == 0 ){ return 9; }
	if( strcmp( str, "ェ" ) == 0 ){ return 9; }
	if( strcmp( str, "ぉ" ) == 0 ){ return 10; }
	if( strcmp( str, "ォ" ) == 0 ){ return 10; }

	if( strcmp( str, "う" ) == 0 ){ return 11; }
	if( strcmp( str, "ウ" ) == 0 ){ return 11; }

	if( strcmp( str, "ふ" ) == 0 ){ return 12; }
	if( strcmp( str, "フ" ) == 0 ){ return 12; }

	if( strcmp( str, "じ" ) == 0 ){ return 13; }
	if( strcmp( str, "ジ" ) == 0 ){ return 13; }

	return 0;
}

void MakeNewSymbol( s8 * str, s8 * buf )
{
	s8 * p;
	s8	cpy[3];
	s8	tmp[3];

	u16	i;

	u8	d_flg;

	d_flg = 0;
	for( i=0; i<512; i++ ){
		if( str[i*2] == '\0' ){ break; }

		if( i == 0 ){
			strcat( buf, SYM_StrGet( &str[i*2] ) );
		}else{
			memcpy( cpy, &str[i*2], 3 );
			cpy[2] = '\0';

			switch( CheckStr( cpy ) ){
			case 1:		// 「っ」「ッ」
				d_flg = 1;
				break;

			case 2:		// 「ー」
				p = strchr( buf, '\0' );
				p--;
				p[1] = p[0];
				p[2] = '\0';
				break;

			case 3:		// 「ゃ」「ャ」
			case 4:		// 「ゅ」「ュ」
			case 5:		// 「ょ」「ョ」
				p = strchr( buf, '\0' );
				p--;
				*p = '\0';
				strcat( buf, SYM_StrGet( cpy ) );
				break;

			case 6:		// 「ぁ」「ァ」
			case 7:		// 「ぃ」「ィ」
			case 8:		// 「ぅ」「ゥ」
			case 9:		// 「ぇ」「ェ」
			case 10:	// 「ぉ」「ォ」
				memcpy( tmp, &str[(i-1)*2], 3 );
				tmp[2] = '\0';

				switch( CheckStr( tmp ) ){
				case 11:	// 「う」「ウ」
					p = strchr( buf, '\0' );
					p--;
					p[0] = 'W';
					p[1] = 'H';
					p[2] = '\0';
					strcat( buf, SYM_StrGet( cpy ) );
					break;

				case 12:	// 「ふ」「フ」
					p = strchr( buf, '\0' );
					p -= 2;
					p[0] = 'F';
					p[1] = '\0';
					strcat( buf, SYM_StrGet( cpy ) );
					break;

				case 13:	// 「じ」「ジ」
					p = strchr( buf, '\0' );
					p -= 2;
					p[0] = 'J';
					p[1] = '\0';
					strcat( buf, SYM_StrGet( cpy ) );
					break;

				default:
					p = strchr( buf, '\0' );
					p--;
					p[0] = 'H';
					p[1] = '\0';
					strcat( buf, SYM_StrGet( cpy ) );
				}
				break;

			default:
				if( d_flg == 1 ){
					strcat( buf, SYM_StrGet( cpy ) );
					p = strchr( buf, '\0' );
					p--;
					*p = '\0';
					d_flg = 0;
				}
				strcat( buf, SYM_StrGet( cpy ) );
			}
		}
	}
}
