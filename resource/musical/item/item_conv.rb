#==========================================================
#	ミュージカル・アイテムデータ変換Ruby
#	
#===========================================================

$KCODE = "SJIS"

#============================
#	define
#============================
STR_END = "#END"
HASH_ERROR = 255


#============================
#	proto
#============================

#データIdx
DATA_IDX_ID 		= 0
DATA_IDX_NAME 		= 1
DATA_IDX_TEXSIZE	= 2
DATA_IDX_OFFSET_X	= 3
DATA_IDX_OFFSET_Y	= 4
DATA_IDX_EQUIP_EAR	= 5
DATA_IDX_EQUIP_HEAD	= 6
DATA_IDX_EQUIP_EYE	= 7
DATA_IDX_EQUIP_FACE	= 8
DATA_IDX_EQUIP_BODY	= 9
DATA_IDX_EQUIP_WAIST	= 10
DATA_IDX_EQUIP_HAND	= 11
DATA_IDX_IS_BACK	= 12
DATA_IDX_IS_FRONT	= 13
DATA_IDX_CONDITION	= 14
DATA_IDX_USETYPE	= 15
DATA_IDX_CAN_REVERSE	= 16

MUS_POKE_EQU_TYPE_EAR	= 0
MUS_POKE_EQU_TYPE_HEAD	= 1
MUS_POKE_EQU_TYPE_EYE	= 2
MUS_POKE_EQU_TYPE_FACE	= 3
MUS_POKE_EQU_TYPE_BODY	= 4
MUS_POKE_EQU_TYPE_WAIST	= 5
MUS_POKE_EQU_TYPE_HAND	= 6
MUS_POKE_EQU_IS_BACK	= 7
MUS_POKE_EQUIP_TYPE_MAX	= 8


TEX_HASH_DATA = {
	"GFL_BBD_TEXSIZ_32x32"=>18 ,
	"GFL_BBD_TEXSIZ_32x64"=>19 ,
	"GFL_BBD_TEXSIZ_64x32"=>26 ,
	"GFL_BBD_TEXSIZ_64x64"=>27 }
CON_HASH_DATA = {
	"クール"=>0 ,
	"キュート"=>1 ,
	"エレガント"=>2 ,
	"ユニーク"=>3 }

USETYPE_HASH_DATA = {
	"回転"=>0 ,
	"発光"=>1 ,
	"浮遊"=>2 ,
	"投擲"=>3 ,
	"使用"=>4 }


#============================
#	main
#============================

dataFileName = ARGV[0]
outFileName = ARGV[1]
if( dataFileName == nil )
	printf( "data file is not found!!\n" )
	exit 1
end
if( outFileName == nil )
	printf( "output file is not found!!\n" )
	exit 1
end

printf("datafile   [%s]\n",dataFileName);
printf("outputfile [%s]\n",outFileName);

dataFile = File.open( dataFileName, "r" );
outFile = File.open( outFileName, "wb" );

#ファイルの解釈開始

line = dataFile.gets #先頭行はラベルなので抜かす
#hash生成
TEX_HASH_DATA.default = HASH_ERROR

no = 1
#パラメタコンバート
while line = dataFile.gets
	if( line.include?(STR_END) )
		break
	end
	
	str = line.split( "\t" )
=begin
	printf("[%s]",str[DATA_IDX_NAME])
	printf("[%s](%d)",str[DATA_IDX_TEXSIZE],TEX_HASH_DATA[str[DATA_IDX_TEXSIZE]])
	printf("[%s]",str[DATA_IDX_OFFSET_X])
	printf("[%s]",str[DATA_IDX_OFFSET_Y])
	printf("[%s]",str[DATA_IDX_EQUIP_HEAD])
	printf("[%s]",str[DATA_IDX_EQUIP_EAR])
	printf("[%s]",str[DATA_IDX_EQUIP_BODY])
	printf("[%s]",str[DATA_IDX_EQUIP_WAIST])
	printf("[%s]",str[DATA_IDX_EQUIP_HAND])
	printf("\n")
=end
	#テクスチャ形式
	texIdx = TEX_HASH_DATA[str[DATA_IDX_TEXSIZE]]
	if( texIdx == HASH_ERROR )
		printf("テクスチャの形式が間違っています[No:%d]",no)
	end
	ary = Array( texIdx )
	outFile.write( ary.pack("I*") )
	
	#オフセット
	ary = Array( str[DATA_IDX_OFFSET_X].to_i )
	outFile.write( ary.pack("c*") )
	ary = Array( str[DATA_IDX_OFFSET_Y].to_i )
	outFile.write( ary.pack("c*") )

	#装備bit
	equipBit =	0
	mainPos = MUS_POKE_EQUIP_TYPE_MAX
	if( str[DATA_IDX_EQUIP_EAR].to_i != 0 )
		equipBit += 1
		if( str[DATA_IDX_EQUIP_EAR].to_i == 2 )
			mainPos = MUS_POKE_EQU_TYPE_EAR
		end
	end
	if( str[DATA_IDX_EQUIP_HEAD].to_i != 0 )
		equipBit += 2
		if( str[DATA_IDX_EQUIP_HEAD].to_i == 2 )
			mainPos = MUS_POKE_EQU_TYPE_HEAD
		end
	end
	if( str[DATA_IDX_EQUIP_EYE].to_i != 0 )
		equipBit += 4
		if( str[DATA_IDX_EQUIP_EYE].to_i == 2 )
			mainPos = MUS_POKE_EQU_TYPE_EYE
		end
	end
	if( str[DATA_IDX_EQUIP_FACE].to_i != 0 )
		equipBit += 8
		if( str[DATA_IDX_EQUIP_FACE].to_i == 2 )
			mainPos = MUS_POKE_EQU_TYPE_FACE
		end
	end
	if( str[DATA_IDX_EQUIP_BODY].to_i != 0 )
		equipBit += 16
		if( str[DATA_IDX_EQUIP_BODY].to_i == 2 )
			mainPos = MUS_POKE_EQU_TYPE_BODY
		end
	end
	if( str[DATA_IDX_EQUIP_WAIST].to_i != 0 )
		equipBit += 32
		if( str[DATA_IDX_EQUIP_WAIST].to_i == 2 )
			mainPos = MUS_POKE_EQU_TYPE_WAIST
		end
	end
	if( str[DATA_IDX_EQUIP_HAND].to_i != 0 )
		equipBit += 64
		if( str[DATA_IDX_EQUIP_HAND].to_i == 2 )
			mainPos = MUS_POKE_EQU_TYPE_HAND
		end
	end
	if( str[DATA_IDX_IS_BACK].to_i != 0 )
		equipBit += 128
	end
	if( str[DATA_IDX_IS_FRONT].to_i != 0 )
		equipBit += 256
	end
	if( str[DATA_IDX_CAN_REVERSE].to_i != 0 )
		equipBit += 512
	end

	ary = Array( equipBit )
	outFile.write( ary.pack("S") )

	ary = Array( mainPos )
	outFile.write( ary.pack("C") )

	conIdx = CON_HASH_DATA[str[DATA_IDX_CONDITION]]
	if( conIdx == HASH_ERROR )
		printf("コンディションの形式が間違っています[No:%d]",no)
	end
	ary = Array( conIdx )
	outFile.write( ary.pack("C") )

	useIdx = USETYPE_HASH_DATA[str[DATA_IDX_USETYPE]]
	if( useIdx == HASH_ERROR )
		printf("使用タイプの形式が間違っています[No:%d]",no)
	end
	ary = Array( useIdx )
	outFile.write( ary.pack("C") )

	#Padding
	ary = Array( 0 )
	outFile.write( ary.pack("C") )

	no = no+1
end

#ファイルの解釈終了

dataFile.close
outFile.close
