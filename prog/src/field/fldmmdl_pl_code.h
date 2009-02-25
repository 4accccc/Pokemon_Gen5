//--------------------------------------------------------------
//	プラチナ　フィールド動作モデル 表示コード
//--------------------------------------------------------------
#define HERO			0x0000		///<主人公
#define BABYBOY1		0x0001		///<男の子
#define BABYGIRL1		0x0002		///<女の子
#define BOY1			0x0003		///<少年
#define BOY2			0x0004		///<活発少年
#define BOY3			0x0005		///<麦わら少年
#define GIRL1			0x0006		///<少女
#define GIRL2			0x0007		///<活発少女
#define GIRL3			0x0008		///<お洒落少女
#define MAN1			0x0009		///<お兄さん
#define MAN2			0x000a		///<ツッパリお兄さん
#define MAN3			0x000b		///<トレーナーお兄さん
#define WOMAN1			0x000c		///<お姉さん
#define WOMAN2			0x000d		///<お洒落お姉さん
#define WOMAN3			0x000e		///<トレーナーお姉さん
#define MIDDLEMAN1		0x000f		///<おじさん
#define MIDDLEWOMAN1	0x0010		///<おばさん
#define OLDMAN1			0x0011		///<和風爺さん
#define OLDWOMAN1		0x0012		///<和風婆さん
#define BIGMAN			0x0013		///<太ったお兄さん
#define MOUNT			0x0014		///<山男
#define CYCLEHERO		0x0015		///<主人公自転車
#define REPORTER		0x0016		///<
#define CAMERAMAN		0x0017		///<
#define SHOPM1			0x0018		///<
#define SHOPW1			0x0019		///<
#define PCWOMAN1		0x001a		///<
#define PCWOMAN2		0x001b		///<
#define PCWOMAN3		0x001c		///<
#define ASSISTANTM		0x001d		///<
#define ASSISTANTW		0x001e		///<
#define BADMAN			0x001f		///<
#define SKIERM			0x0020		///<
#define SKIERW			0x0021		///<
#define POLICEMAN		0x0022		///<
#define IDOL			0x0023		///<
#define GENTLEMAN		0x0024		///<
#define LADY			0x0025		///<
#define CYCLEM			0x0026		///<
#define CYCLEW			0x0027		///<
#define WORKMAN			0x0028		///<
#define FARMER			0x0029		///<
#define COWGIRL			0x002a		///<
#define CLOWN			0x002b		///<
#define ARTIST			0x002c		///<
#define SPORTSMAN		0x002d		///<
#define SWIMMERM		0x002e		///<
#define SWIMMERW		0x002f		///<
#define BEACHGIRL		0x0030		///<
#define BEACHBOY		0x0031		///<
#define EXPLORE			0x0032		///<
#define FIGHTER			0x0033		///<
#define CAMPBOY			0x0034		///<
#define PICNICGIRL		0x0035		///<
#define FISHING			0x0036		///<
#define AMBRELLA		0x0037		///<
#define SEAMAN			0x0038		///<
#define BRINGM			0x0039		///<
#define BRINGW			0x003a		///<
#define WAITER			0x003b		///<
#define WAITRESS		0x003c		///<
#define VETERAN			0x003d		///<
#define GORGGEOUSM		0x003e		///<
#define GORGGEOUSW		0x003f		///<
#define BOY4			0x0040		///<
#define GIRL4			0x0041		///<
#define MAN4			0x0042		///<
#define WOMAN4			0x0043		///<
#define MAN5			0x0044		///<
#define WOMAN5			0x0045		///<
#define MYSTERY			0x0046		///<
#define PIKACHU			0x0047		///<
#define PIPPI			0x0048		///<
#define PURIN			0x0049		///<
#define KODUCK			0x004a		///<
#define PRASLE			0x004b		///<
#define MINUN			0x004c		///<
#define MARIL			0x004d		///<
#define ACHAMO			0x004e		///<
#define ENECO			0x004f		///<
#define GONBE			0x0050		///<
#define BABY			0x0051		///<
#define MIDDLEMAN2		0x0052		///<
#define MIDDLEWOMAN2	0x0053		///<
#define ROCK			0x0054		///<
#define BREAKROCK		0x0055		///<
#define TREE			0x0056		///<
#define MONATERBALL		0x0057		///<
#define SHADOW			0x0058		///<
#define BOUGH			0x0059		///<
#define FALLTREE		0x005a		///<
#define BOARD_A			0x005b		///<看板
#define BOARD_B			0x005c		///<看板
#define BOARD_C			0x005d		///<看板
#define BOARD_D			0x005e		///<看板
#define BOARD_E			0x005f		///<看板
#define BOARD_F			0x0060		///<看板
#define HEROINE			0x0061		///<
#define CYCLEHEROINE	0x0062		///<
#define DOCTOR			0x0063
#define SEED			0x0064
#define WKOBJCODE00		0x0065
#define WKOBJCODE01		0x0066
#define WKOBJCODE02		0x0067
#define WKOBJCODE03		0x0068
#define WKOBJCODE04		0x0069
#define WKOBJCODE05		0x006a
#define WKOBJCODE06		0x006b
#define WKOBJCODE07		0x006c
#define WKOBJCODE08		0x006d
#define WKOBJCODE09		0x006e
#define WKOBJCODE10		0x006f
#define WKOBJCODE11		0x0070
#define WKOBJCODE12		0x0071
#define WKOBJCODE13		0x0072
#define WKOBJCODE14		0x0073
#define WKOBJCODE15		0x0074
#define BANCODE			0x0075	//コード追加ミス　使用禁止
#define SNOWBALL		0x0076
#define GINGABOSS		0x0078
#define GKANBU1			0x0079
#define GKANBU2			0x007a
#define GKANBU3			0x007b
#define GINGAM			0x007c
#define GINGAW			0x007d
#define LEADER1			0x007e
#define LEADER2			0x007f
#define LEADER3			0x0080
#define LEADER4			0x0081
#define LEADER5			0x0082
#define LEADER6			0x0083
#define LEADER7			0x0084
#define LEADER8			0x0085
#define BIGFOUR1		0x0086
#define BIGFOUR2		0x0087
#define BIGFOUR3		0x0088
#define BIGFOUR4		0x0089
#define CHAMPION		0x008a
#define PAPA			0x008b
#define MAMA			0x008c
#define SEVEN1			0x008d
#define SEVEN2			0x008e
#define SEVEN3			0x008f
#define SEVEN4			0x0090
#define SEVEN5			0x0091
#define SEVEN6			0x0092
#define SEVEN7			0x0093
#define RIVEL			0x0094
#define BOAT			0x0095
#define TRAIN			0x0096
#define SPPOKE1			0x0097
#define SPPOKE2			0x0098
#define SPPOKE3			0x0099
#define SPPOKE4			0x009a
#define SPPOKE5			0x009b
#define SPPOKE6			0x009c
#define SPPOKE7			0x009d
#define SPPOKE8			0x009e
#define SPPOKE9			0x009f
#define SPPOKE10		0x00a0
#define SPPOKE11		0x00a1
#define OBAKE			0x00a2
#define WOMAN6			0x00a3
#define OLDMAN2			0x00a4
#define OLDWOMAN2		0x00a5
#define OOKIDO			0x00a6
#define MIKAN			0x00a7
#define SUNGLASSES		0x00a8
#define TOWERBOSS		0x00a9
#define SNOWCOVER		0x00aa
#define TREECOVER		0x00ab
#define ROCKCOVER		0x00ac
#define BIRD			0x00ad
#define BAG				0x00ae
#define MAID			0x00af
#define SPHERO			0x00b0
#define SPHEROINE		0x00b1
#define	SWIMHERO		0x00b2
#define SWIMHEROINE		0x00b3
#define WATERHERO		0x00b4
#define WATERHEROINE	0x00b5
#define VENTHOLE		0x00b6
#define BOOK			0x00b7
#define SPPOKE12		0x00b8
#define BALLOON			0x00b9
#define CONTESTHERO		0x00ba
#define CONTESTHEROINE	0x00bb
#define FISHINGHERO		0x00bc
#define FISHINGHEROINE	0x00bd
#define MOSS			0x00be
#define FREEZES			0x00bf
#define POLE			0x00c0
#define DELIVERY		0x00c1
#define DSBOY			0x00c2
#define KOIKING			0x00c3
#define POKEHERO		0x00c4
#define POKEHEROINE		0x00c5
#define SAVEHERO		0x00c6
#define SAVEHEROINE		0x00c7
#define BANZAIHERO		0x00c8
#define BANZAIHEROINE	0x00c9
#define DOOR			0x00ca
#define MONUMENT		0x00cb
#define PATIRITUSU		0x00cc
#define KINOCOCO		0x00cd
#define MIMITUTO		0x00ce
#define KOLUCKY			0x00cf
#define WANRIKY			0x00d0
#define DOOR2			0x00d1
#define GHOSTHERO		0x00d2
#define GHOSTHEROINE	0x00d3
#define RTHERO			0x00d4
#define ICPO			0x00d5
#define GKANBU4			0x00d6
#define BRAINS1			0x00d7
#define BRAINS2			0x00d8
#define BRAINS3			0x00d9
#define BRAINS4			0x00da
#define PRINCESS		0x00db
#define NAETLE			0x00dc
#define HAYASHIGAME		0x00dd
#define DODAITOSE		0x00de
#define HIKOZARU		0x00df
#define MOUKAZARU		0x00e0
#define GOUKAZARU		0x00e1
#define POCHAMA			0x00e2
#define POTTAISHI		0x00e3
#define EMPERTE			0x00e4
#define GUREGGRU		0x00e5
#define GIRATINAORIGIN	0x00e6
#define BFSM			0x00e7
#define BFSW1			0x00e8
#define BFSW2			0x00e9
#define WIFISM			0x00ea
#define WIFISW			0x00eb
#define ROTOM			0x00ec
#define ROTOMF			0x00ed
#define ROTOMS			0x00ee
#define ROTOMI			0x00ef
#define ROTOMW			0x00f0
#define ROTOMG			0x00f1
#define DIRECTOR		0x00f2
#define FIREBOX			0x00f3
#define SKYBOX			0x00f4
#define ICEBOX			0x00f5
#define WATERBOX		0x00f6
#define GRASSBOX		0x00f7
#define GSET1			0x00f8
#define GSET2			0x00f9
#define MONUMENT2A		0x00fa
#define MONUMENT2B		0x00fb
#define DPHERO			0x00fc
#define DPHEROINE		0x00fd
#define DPCONTESTHERO	0x00fe
#define DPCONTESTHEROINE	0x00ff
#define BSHERO			0x0100
#define BSHEROINE		0x0101
#define SWIMHERORT		0x0102
#define SWIMHEROINERT	0x0103
#define SPHERORT		0x0104
#define SPHEROINERT		0x0105
#define ROTOMWALL		0x0106
#define RTHEROINE		0x0107
#define BLDAGUNOMU		0x0108
#define TW7YUKUSHI		0x0109
#define TW7EMULIT		0x010a
#define TW7AGUNOMU		0x010b
#define SAVEHERORT		0x010c
#define SAVEHEROINERT	0x010d
#define POKEHERORT		0x010e
#define POKEHEROINERT	0x010f
#define GSTSAVEHERO		0x0110
#define GSTSAVEHEROINE	0x0111
#define GSTPOKEHERO		0x0112
#define GSTPOKEHEROINE	0x0113

#define OBJCODEMAX		0xffff		///<OBJコード最大

//--------------------------------------------------------------
///	ワーク参照型OBJコード　チェックシンボル
//--------------------------------------------------------------
#define WKOBJCODE_ORG	(WKOBJCODE00)
#define WKOBJCODE_END	(WKOBJCODE15)

//--------------------------------------------------------------
///	木の実専用OBJコード　通常のOBJコードに重ならない様、注意
/// ※マップエディタでの配置は禁止
//--------------------------------------------------------------
#define SEEDCODEORG (0x1000)
#define SEED00				(SEEDCODEORG + 0x00)
#define SEED01A                (SEEDCODEORG + 0x01)
#define SEED01B                (SEEDCODEORG + 0x02)
#define SEED01C                (SEEDCODEORG + 0x03)
#define SEED02A                (SEEDCODEORG + 0x04)
#define SEED02B                (SEEDCODEORG + 0x05)
#define SEED02C                (SEEDCODEORG + 0x06)
#define SEED03A                (SEEDCODEORG + 0x07)
#define SEED03B                (SEEDCODEORG + 0x08)
#define SEED03C                (SEEDCODEORG + 0x09)
#define SEED04A                (SEEDCODEORG + 0x0a)
#define SEED04B                (SEEDCODEORG + 0x0b)
#define SEED04C                (SEEDCODEORG + 0x0c)
#define SEED05A                (SEEDCODEORG + 0x0d)
#define SEED05B                (SEEDCODEORG + 0x0e)
#define SEED05C                (SEEDCODEORG + 0x0f)
#define SEED06A                (SEEDCODEORG + 0x10)
#define SEED06B                (SEEDCODEORG + 0x11)
#define SEED06C                (SEEDCODEORG + 0x12)
#define SEED07A                (SEEDCODEORG + 0x13)
#define SEED07B                (SEEDCODEORG + 0x14)
#define SEED07C                (SEEDCODEORG + 0x15)
#define SEED08A                (SEEDCODEORG + 0x16)
#define SEED08B                (SEEDCODEORG + 0x17)
#define SEED08C                (SEEDCODEORG + 0x18)
#define SEED09A                (SEEDCODEORG + 0x19)
#define SEED09B                (SEEDCODEORG + 0x1a)
#define SEED09C                (SEEDCODEORG + 0x1b)
#define SEED10A                (SEEDCODEORG + 0x1c)
#define SEED10B                (SEEDCODEORG + 0x1d)
#define SEED10C                (SEEDCODEORG + 0x1e)
#define SEED11A                (SEEDCODEORG + 0x1f)
#define SEED11B                (SEEDCODEORG + 0x20)
#define SEED11C                (SEEDCODEORG + 0x21)
#define SEED12A                (SEEDCODEORG + 0x22)
#define SEED12B                (SEEDCODEORG + 0x23)
#define SEED12C                (SEEDCODEORG + 0x24)
#define SEED13A                (SEEDCODEORG + 0x25)
#define SEED13B                (SEEDCODEORG + 0x26)
#define SEED13C                (SEEDCODEORG + 0x27)
#define SEED14A                (SEEDCODEORG + 0x28)
#define SEED14B                (SEEDCODEORG + 0x29)
#define SEED14C                (SEEDCODEORG + 0x2a)
#define SEED15A                (SEEDCODEORG + 0x2b)
#define SEED15B                (SEEDCODEORG + 0x2c)
#define SEED15C                (SEEDCODEORG + 0x2d)
#define SEED16A                (SEEDCODEORG + 0x2e)
#define SEED16B                (SEEDCODEORG + 0x2f)
#define SEED16C                (SEEDCODEORG + 0x30)
#define SEED17A                (SEEDCODEORG + 0x31)
#define SEED17B                (SEEDCODEORG + 0x32)
#define SEED17C                (SEEDCODEORG + 0x33)
#define SEED18A                (SEEDCODEORG + 0x34)
#define SEED18B                (SEEDCODEORG + 0x35)
#define SEED18C                (SEEDCODEORG + 0x36)
#define SEED19A                (SEEDCODEORG + 0x37)
#define SEED19B                (SEEDCODEORG + 0x38)
#define SEED19C                (SEEDCODEORG + 0x39)
#define SEED20A                (SEEDCODEORG + 0x3a)
#define SEED20B                (SEEDCODEORG + 0x3b)
#define SEED20C                (SEEDCODEORG + 0x3c)
#define SEED21A                (SEEDCODEORG + 0x3d)
#define SEED21B                (SEEDCODEORG + 0x3e)
#define SEED21C                (SEEDCODEORG + 0x3f)
#define SEED22A                (SEEDCODEORG + 0x40)
#define SEED22B                (SEEDCODEORG + 0x41)
#define SEED22C                (SEEDCODEORG + 0x42)
#define SEED23A                (SEEDCODEORG + 0x43)
#define SEED23B                (SEEDCODEORG + 0x44)
#define SEED23C                (SEEDCODEORG + 0x45)
#define SEED24A                (SEEDCODEORG + 0x46)
#define SEED24B                (SEEDCODEORG + 0x47)
#define SEED24C                (SEEDCODEORG + 0x48)
#define SEED25A                (SEEDCODEORG + 0x49)
#define SEED25B                (SEEDCODEORG + 0x4a)
#define SEED25C                (SEEDCODEORG + 0x4b)
#define SEED26A                (SEEDCODEORG + 0x4c)
#define SEED26B                (SEEDCODEORG + 0x4d)
#define SEED26C                (SEEDCODEORG + 0x4e)
#define SEED27A                (SEEDCODEORG + 0x4f)
#define SEED27B                (SEEDCODEORG + 0x50)
#define SEED27C                (SEEDCODEORG + 0x51)
#define SEED28A                (SEEDCODEORG + 0x52)
#define SEED28B                (SEEDCODEORG + 0x53)
#define SEED28C                (SEEDCODEORG + 0x54)
#define SEED29A                (SEEDCODEORG + 0x55)
#define SEED29B                (SEEDCODEORG + 0x56)
#define SEED29C                (SEEDCODEORG + 0x57)
#define SEED30A                (SEEDCODEORG + 0x58)
#define SEED30B                (SEEDCODEORG + 0x59)
#define SEED30C                (SEEDCODEORG + 0x5a)
#define SEED31A                (SEEDCODEORG + 0x5b)
#define SEED31B                (SEEDCODEORG + 0x5c)
#define SEED31C                (SEEDCODEORG + 0x5d)
#define SEED32A                (SEEDCODEORG + 0x5e)
#define SEED32B                (SEEDCODEORG + 0x5f)
#define SEED32C                (SEEDCODEORG + 0x60)
#define SEED33A                (SEEDCODEORG + 0x61)
#define SEED33B                (SEEDCODEORG + 0x62)
#define SEED33C                (SEEDCODEORG + 0x63)
#define SEED34A                (SEEDCODEORG + 0x64)
#define SEED34B                (SEEDCODEORG + 0x65)
#define SEED34C                (SEEDCODEORG + 0x66)
#define SEED35A                (SEEDCODEORG + 0x67)
#define SEED35B                (SEEDCODEORG + 0x68)
#define SEED35C                (SEEDCODEORG + 0x69)
#define SEED36A                (SEEDCODEORG + 0x6a)
#define SEED36B                (SEEDCODEORG + 0x6b)
#define SEED36C                (SEEDCODEORG + 0x6c)
#define SEED37A                (SEEDCODEORG + 0x6d)
#define SEED37B                (SEEDCODEORG + 0x6e)
#define SEED37C                (SEEDCODEORG + 0x6f)
#define SEED38A                (SEEDCODEORG + 0x70)
#define SEED38B                (SEEDCODEORG + 0x71)
#define SEED38C                (SEEDCODEORG + 0x72)
#define SEED39A                (SEEDCODEORG + 0x73)
#define SEED39B                (SEEDCODEORG + 0x74)
#define SEED39C                (SEEDCODEORG + 0x75)
#define SEED40A                (SEEDCODEORG + 0x76)
#define SEED40B                (SEEDCODEORG + 0x77)
#define SEED40C                (SEEDCODEORG + 0x78)
#define SEED41A                (SEEDCODEORG + 0x79)
#define SEED41B                (SEEDCODEORG + 0x7a)
#define SEED41C                (SEEDCODEORG + 0x7b)
#define SEED42A                (SEEDCODEORG + 0x7c)
#define SEED42B                (SEEDCODEORG + 0x7d)
#define SEED42C                (SEEDCODEORG + 0x7e)
#define SEED43A                (SEEDCODEORG + 0x7f)
#define SEED43B                (SEEDCODEORG + 0x80)
#define SEED43C                (SEEDCODEORG + 0x81)
#define SEED44A                (SEEDCODEORG + 0x82)
#define SEED44B                (SEEDCODEORG + 0x83)
#define SEED44C                (SEEDCODEORG + 0x84)
#define SEED45A                (SEEDCODEORG + 0x85)
#define SEED45B                (SEEDCODEORG + 0x86)
#define SEED45C                (SEEDCODEORG + 0x87)
#define SEED46A                (SEEDCODEORG + 0x88)
#define SEED46B                (SEEDCODEORG + 0x89)
#define SEED46C                (SEEDCODEORG + 0x8a)
#define SEED47A                (SEEDCODEORG + 0x8b)
#define SEED47B                (SEEDCODEORG + 0x8c)
#define SEED47C                (SEEDCODEORG + 0x8d)
#define SEED48A                (SEEDCODEORG + 0x8e)
#define SEED48B                (SEEDCODEORG + 0x8f)
#define SEED48C                (SEEDCODEORG + 0x90)
#define SEED49A                (SEEDCODEORG + 0x91)
#define SEED49B                (SEEDCODEORG + 0x92)
#define SEED49C                (SEEDCODEORG + 0x93)
#define SEED50A                (SEEDCODEORG + 0x94)
#define SEED50B                (SEEDCODEORG + 0x95)
#define SEED50C                (SEEDCODEORG + 0x96)
#define SEED51A                (SEEDCODEORG + 0x97)
#define SEED51B                (SEEDCODEORG + 0x98)
#define SEED51C                (SEEDCODEORG + 0x99)
#define SEED52A                (SEEDCODEORG + 0x9a)
#define SEED52B                (SEEDCODEORG + 0x9b)
#define SEED52C                (SEEDCODEORG + 0x9c)
#define SEED53A                (SEEDCODEORG + 0x9d)
#define SEED53B                (SEEDCODEORG + 0x9e)
#define SEED53C                (SEEDCODEORG + 0x9f)
#define SEED54A                (SEEDCODEORG + 0xa0)
#define SEED54B                (SEEDCODEORG + 0xa1)
#define SEED54C                (SEEDCODEORG + 0xa2)
#define SEED55A                (SEEDCODEORG + 0xa3)
#define SEED55B                (SEEDCODEORG + 0xa4)
#define SEED55C                (SEEDCODEORG + 0xa5)
#define SEED56A                (SEEDCODEORG + 0xa6)
#define SEED56B                (SEEDCODEORG + 0xa7)
#define SEED56C                (SEEDCODEORG + 0xa8)
#define SEED57A                (SEEDCODEORG + 0xa9)
#define SEED57B                (SEEDCODEORG + 0xaa)
#define SEED57C                (SEEDCODEORG + 0xab)
#define SEED58A                (SEEDCODEORG + 0xac)
#define SEED58B                (SEEDCODEORG + 0xad)
#define SEED58C                (SEEDCODEORG + 0xae)
#define SEED59A                (SEEDCODEORG + 0xaf)
#define SEED59B                (SEEDCODEORG + 0xb0)
#define SEED59C                (SEEDCODEORG + 0xb1)
#define SEED60A                (SEEDCODEORG + 0xb2)
#define SEED60B                (SEEDCODEORG + 0xb3)
#define SEED60C                (SEEDCODEORG + 0xb4)
#define SEED61A                (SEEDCODEORG + 0xb5)
#define SEED61B                (SEEDCODEORG + 0xb6)
#define SEED61C                (SEEDCODEORG + 0xb7)
#define SEED62A                (SEEDCODEORG + 0xb8)
#define SEED62B                (SEEDCODEORG + 0xb9)
#define SEED62C                (SEEDCODEORG + 0xba)
#define SEED63A                (SEEDCODEORG + 0xbb)
#define SEED63B                (SEEDCODEORG + 0xbc)
#define SEED63C                (SEEDCODEORG + 0xbd)
#define SEED64A                (SEEDCODEORG + 0xbe)
#define SEED64B                (SEEDCODEORG + 0xbf)
#define SEED64C                (SEEDCODEORG + 0xc0)

//--------------------------------------------------------------
///	特殊OBJコード
/// ※マップエディタでの配置は禁止
//--------------------------------------------------------------
#define NONDRAW			(0x2000)

