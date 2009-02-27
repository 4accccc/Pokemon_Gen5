//--------------------------------------------------------------
//	プラチナ　フィールド動作モデル 表示コード
//--------------------------------------------------------------
#define PLHERO			0x0000		///<主人公
#define PLBABYBOY1		0x0001		///<男の子
#define PLBABYGIRL1		0x0002		///<女の子
#define PLBOY1			0x0003		///<少年
#define PLBOY2			0x0004		///<活発少年
#define PLBOY3			0x0005		///<麦わら少年
#define PLGIRL1			0x0006		///<少女
#define PLGIRL2			0x0007		///<活発少女
#define PLGIRL3			0x0008		///<お洒落少女
#define PLMAN1			0x0009		///<お兄さん
#define PLMAN2			0x000a		///<ツッパリお兄さん
#define PLMAN3			0x000b		///<トレーナーお兄さん
#define PLWOMAN1			0x000c		///<お姉さん
#define PLWOMAN2			0x000d		///<お洒落お姉さん
#define PLWOMAN3			0x000e		///<トレーナーお姉さん
#define PLMIDDLEMAN1		0x000f		///<おじさん
#define PLMIDDLEWOMAN1	0x0010		///<おばさん
#define PLOLDMAN1			0x0011		///<和風爺さん
#define PLOLDWOMAN1		0x0012		///<和風婆さん
#define PLBIGMAN			0x0013		///<太ったお兄さん
#define PLMOUNT			0x0014		///<山男
#define PLCYCLEHERO		0x0015		///<主人公自転車
#define PLREPORTER		0x0016		///<
#define PLCAMERAMAN		0x0017		///<
#define PLSHOPM1			0x0018		///<
#define PLSHOPW1			0x0019		///<
#define PLPCWOMAN1		0x001a		///<
#define PLPCWOMAN2		0x001b		///<
#define PLPCWOMAN3		0x001c		///<
#define PLASSISTANTM		0x001d		///<
#define PLASSISTANTW		0x001e		///<
#define PLBADMAN			0x001f		///<
#define PLSKIERM			0x0020		///<
#define PLSKIERW			0x0021		///<
#define PLPOLICEMAN		0x0022		///<
#define PLIDOL			0x0023		///<
#define PLGENTLEMAN		0x0024		///<
#define PLLADY			0x0025		///<
#define PLCYCLEM			0x0026		///<
#define PLCYCLEW			0x0027		///<
#define PLWORKMAN			0x0028		///<
#define PLFARMER			0x0029		///<
#define PLCOWGIRL			0x002a		///<
#define PLCLOWN			0x002b		///<
#define PLARTIST			0x002c		///<
#define PLSPORTSMAN		0x002d		///<
#define PLSWIMMERM		0x002e		///<
#define PLSWIMMERW		0x002f		///<
#define PLBEACHGIRL		0x0030		///<
#define PLBEACHBOY		0x0031		///<
#define PLEXPLORE			0x0032		///<
#define PLFIGHTER			0x0033		///<
#define PLCAMPBOY			0x0034		///<
#define PLPICNICGIRL		0x0035		///<
#define PLFISHING			0x0036		///<
#define PLAMBRELLA		0x0037		///<
#define PLSEAMAN			0x0038		///<
#define PLBRINGM			0x0039		///<
#define PLBRINGW			0x003a		///<
#define PLWAITER			0x003b		///<
#define PLWAITRESS		0x003c		///<
#define PLVETERAN			0x003d		///<
#define PLGORGGEOUSM		0x003e		///<
#define PLGORGGEOUSW		0x003f		///<
#define PLBOY4			0x0040		///<
#define PLGIRL4			0x0041		///<
#define PLMAN4			0x0042		///<
#define PLWOMAN4			0x0043		///<
#define PLMAN5			0x0044		///<
#define PLWOMAN5			0x0045		///<
#define PLMYSTERY			0x0046		///<
#define PLPIKACHU			0x0047		///<
#define PLPIPPI			0x0048		///<
#define PLPURIN			0x0049		///<
#define PLKODUCK			0x004a		///<
#define PLPRASLE			0x004b		///<
#define PLMINUN			0x004c		///<
#define PLMARIL			0x004d		///<
#define PLACHAMO			0x004e		///<
#define PLENECO			0x004f		///<
#define PLGONBE			0x0050		///<
#define PLBABY			0x0051		///<
#define PLMIDDLEMAN2		0x0052		///<
#define PLMIDDLEWOMAN2	0x0053		///<
#define PLROCK			0x0054		///<
#define PLBREAKROCK		0x0055		///<
#define PLTREE			0x0056		///<
#define PLMONATERBALL		0x0057		///<
#define PLSHADOW			0x0058		///<
#define PLBOUGH			0x0059		///<
#define PLFALLTREE		0x005a		///<
#define PLBOARD_A			0x005b		///<看板
#define PLBOARD_B			0x005c		///<看板
#define PLBOARD_C			0x005d		///<看板
#define PLBOARD_D			0x005e		///<看板
#define PLBOARD_E			0x005f		///<看板
#define PLBOARD_F			0x0060		///<看板
#define PLHEROINE			0x0061		///<
#define PLCYCLEHEROINE	0x0062		///<
#define PLDOCTOR			0x0063
#define PLSEED			0x0064
#define PLWKOBJCODE00		0x0065
#define PLWKOBJCODE01		0x0066
#define PLWKOBJCODE02		0x0067
#define PLWKOBJCODE03		0x0068
#define PLWKOBJCODE04		0x0069
#define PLWKOBJCODE05		0x006a
#define PLWKOBJCODE06		0x006b
#define PLWKOBJCODE07		0x006c
#define PLWKOBJCODE08		0x006d
#define PLWKOBJCODE09		0x006e
#define PLWKOBJCODE10		0x006f
#define PLWKOBJCODE11		0x0070
#define PLWKOBJCODE12		0x0071
#define PLWKOBJCODE13		0x0072
#define PLWKOBJCODE14		0x0073
#define PLWKOBJCODE15		0x0074
#define PLBANCODE			0x0075	//コード追加ミス　使用禁止
#define PLSNOWBALL		0x0076
#define PLGINGABOSS		0x0078
#define PLGKANBU1			0x0079
#define PLGKANBU2			0x007a
#define PLGKANBU3			0x007b
#define PLGINGAM			0x007c
#define PLGINGAW			0x007d
#define PLLEADER1			0x007e
#define PLLEADER2			0x007f
#define PLLEADER3			0x0080
#define PLLEADER4			0x0081
#define PLLEADER5			0x0082
#define PLLEADER6			0x0083
#define PLLEADER7			0x0084
#define PLLEADER8			0x0085
#define PLBIGFOUR1		0x0086
#define PLBIGFOUR2		0x0087
#define PLBIGFOUR3		0x0088
#define PLBIGFOUR4		0x0089
#define PLCHAMPION		0x008a
#define PLPAPA			0x008b
#define PLMAMA			0x008c
#define PLSEVEN1			0x008d
#define PLSEVEN2			0x008e
#define PLSEVEN3			0x008f
#define PLSEVEN4			0x0090
#define PLSEVEN5			0x0091
#define PLSEVEN6			0x0092
#define PLSEVEN7			0x0093
#define PLRIVEL			0x0094
#define PLBOAT			0x0095
#define PLTRAIN			0x0096
#define PLSPPOKE1			0x0097
#define PLSPPOKE2			0x0098
#define PLSPPOKE3			0x0099
#define PLSPPOKE4			0x009a
#define PLSPPOKE5			0x009b
#define PLSPPOKE6			0x009c
#define PLSPPOKE7			0x009d
#define PLSPPOKE8			0x009e
#define PLSPPOKE9			0x009f
#define PLSPPOKE10		0x00a0
#define PLSPPOKE11		0x00a1
#define PLOBAKE			0x00a2
#define PLWOMAN6			0x00a3
#define PLOLDMAN2			0x00a4
#define PLOLDWOMAN2		0x00a5
#define PLOOKIDO			0x00a6
#define PLMIKAN			0x00a7
#define PLSUNGLASSES		0x00a8
#define PLTOWERBOSS		0x00a9
#define PLSNOWCOVER		0x00aa
#define PLTREECOVER		0x00ab
#define PLROCKCOVER		0x00ac
#define PLBIRD			0x00ad
#define PLBAG				0x00ae
#define PLMAID			0x00af
#define PLSPHERO			0x00b0
#define PLSPHEROINE		0x00b1
#define PLSWIMHERO		0x00b2
#define PLSWIMHEROINE		0x00b3
#define PLWATERHERO		0x00b4
#define PLWATERHEROINE	0x00b5
#define PLVENTHOLE		0x00b6
#define PLBOOK			0x00b7
#define PLSPPOKE12		0x00b8
#define PLBALLOON			0x00b9
#define PLCONTESTHERO		0x00ba
#define PLCONTESTHEROINE	0x00bb
#define PLFISHINGHERO		0x00bc
#define PLFISHINGHEROINE	0x00bd
#define PLMOSS			0x00be
#define PLFREEZES			0x00bf
#define PLPOLE			0x00c0
#define PLDELIVERY		0x00c1
#define PLDSBOY			0x00c2
#define PLKOIKING			0x00c3
#define PLPOKEHERO		0x00c4
#define PLPOKEHEROINE		0x00c5
#define PLSAVEHERO		0x00c6
#define PLSAVEHEROINE		0x00c7
#define PLBANZAIHERO		0x00c8
#define PLBANZAIHEROINE	0x00c9
#define PLDOOR			0x00ca
#define PLMONUMENT		0x00cb
#define PLPATIRITUSU		0x00cc
#define PLKINOCOCO		0x00cd
#define PLMIMITUTO		0x00ce
#define PLKOLUCKY			0x00cf
#define PLWANRIKY			0x00d0
#define PLDOOR2			0x00d1
#define PLGHOSTHERO		0x00d2
#define PLGHOSTHEROINE	0x00d3
#define PLRTHERO			0x00d4
#define PLICPO			0x00d5
#define PLGKANBU4			0x00d6
#define PLBRAINS1			0x00d7
#define PLBRAINS2			0x00d8
#define PLBRAINS3			0x00d9
#define PLBRAINS4			0x00da
#define PLPRINCESS		0x00db
#define PLNAETLE			0x00dc
#define PLHAYASHIGAME		0x00dd
#define PLDODAITOSE		0x00de
#define PLHIKOZARU		0x00df
#define PLMOUKAZARU		0x00e0
#define PLGOUKAZARU		0x00e1
#define PLPOCHAMA			0x00e2
#define PLPOTTAISHI		0x00e3
#define PLEMPERTE			0x00e4
#define PLGUREGGRU		0x00e5
#define PLGIRATINAORIGIN	0x00e6
#define PLBFSM			0x00e7
#define PLBFSW1			0x00e8
#define PLBFSW2			0x00e9
#define PLWIFISM			0x00ea
#define PLWIFISW			0x00eb
#define PLROTOM			0x00ec
#define PLROTOMF			0x00ed
#define PLROTOMS			0x00ee
#define PLROTOMI			0x00ef
#define PLROTOMW			0x00f0
#define PLROTOMG			0x00f1
#define PLDIRECTOR		0x00f2
#define PLFIREBOX			0x00f3
#define PLSKYBOX			0x00f4
#define PLICEBOX			0x00f5
#define PLWATERBOX		0x00f6
#define PLGRASSBOX		0x00f7
#define PLGSET1			0x00f8
#define PLGSET2			0x00f9
#define PLMONUMENT2A		0x00fa
#define PLMONUMENT2B		0x00fb
#define PLDPHERO			0x00fc
#define PLDPHEROINE		0x00fd
#define PLDPCONTESTHERO	0x00fe
#define PLDPCONTESTHEROINE	0x00ff
#define PLBSHERO			0x0100
#define PLBSHEROINE		0x0101
#define PLSWIMHERORT		0x0102
#define PLSWIMHEROINERT	0x0103
#define PLSPHERORT		0x0104
#define PLSPHEROINERT		0x0105
#define PLROTOMWALL		0x0106
#define PLRTHEROINE		0x0107
#define PLBLDAGUNOMU		0x0108
#define PLTW7YUKUSHI		0x0109
#define PLTW7EMULIT		0x010a
#define PLTW7AGUNOMU		0x010b
#define PLSAVEHERORT		0x010c
#define PLSAVEHEROINERT	0x010d
#define PLPOKEHERORT		0x010e
#define PLPOKEHEROINERT	0x010f
#define PLGSTSAVEHERO		0x0110
#define PLGSTSAVEHEROINE	0x0111
#define PLGSTPOKEHERO		0x0112
#define PLGSTPOKEHEROINE	0x0113

#define PLOBJCODEMAX		0xffff		///<OBJコード最大

//--------------------------------------------------------------
///	ワーク参照型OBJコード　チェックシンボル
//--------------------------------------------------------------
#define PLWKOBJCODE_ORG	(WKOBJCODE00)
#define PLWKOBJCODE_END	(WKOBJCODE15)

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
#define PLNONDRAW			(0x2000)

