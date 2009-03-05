########################################################
#
#
#		グラフィックファイルいろいろ生成コンバータ
#
#
########################################################


#入力出力ファイる
$GRADATA_FILE_NAME		= "wflby_gradata.xls";
$GRADATA_TXTFILE_NAME	= "gradata.txt";
$GRADATA_OUTGRADATAFILE	= "wflby_3dmapobj_data.c";
$GRADATA_ARCLIST_FILE	= "list.txt";
$GRADATA_CONVLIST_FILE	= "convlist.bat";


@GRADATA_FILE = undef;			#グラフィックデータファイル

@GRA_LIST = undef;				#アーカイブするファイルのリスト
@GRA_CONVLIST = undef;			#コンバートするファイルのリスト

@GRA_NORMAL_FIRE = undef;		#ノーマル火の部屋ファイル
@GRA_NORMAL_WATER = undef;		#ノーマル水の部屋ファイル
@GRA_NORMAL_SPARK = undef;		#ノーマル電気の部屋ファイル
@GRA_NORMAL_GRASS = undef;		#ノーマル草の部屋ファイル
@GRA_NORMAL_SPECIAL = undef;	#ノーマル特殊の部屋ファイル

@GRA_SPRING_FIRE = undef;		#春火の部屋ファイル
@GRA_SPRING_WATER = undef;		#春水の部屋ファイル
@GRA_SPRING_SPARK = undef;		#春電気の部屋ファイル
@GRA_SPRING_GRASS = undef;		#春水の部屋ファイル
@GRA_SPRING_SPECIAL = undef;	#春特殊の部屋ファイル

@GRA_SUMMER_FIRE = undef;		#夏火の部屋ファイル
@GRA_SUMMER_WATER = undef;		#夏水の部屋ファイル
@GRA_SUMMER_SPARK = undef;		#夏電気の部屋ファイル
@GRA_SUMMER_GRASS = undef;		#夏草の部屋ファイル
@GRA_SUMMER_SPECIAL = undef;	#夏特殊の部屋ファイル

@GRA_FALL_FIRE = undef;			#秋火の部屋ファイル
@GRA_FALL_WATER = undef;		#秋水の部屋ファイル
@GRA_FALL_SPARK = undef;		#秋電気の部屋ファイル
@GRA_FALL_GRASS = undef;		#秋草の部屋ファイル
@GRA_FALL_SPECIAL = undef;		#秋特殊の部屋ファイル

@GRA_WINTER_FIRE = undef;		#冬火の部屋ファイル
@GRA_WINTER_WATER = undef;		#冬水の部屋ファイル
@GRA_WINTER_SPARK = undef;		#冬電気の部屋ファイル
@GRA_WINTER_GRASS = undef;		#冬草の部屋ファイル
@GRA_WINTER_SPECIAL = undef;	#冬特殊の部屋ファイル


#バッファ内INDEXENUM
$GRA_ENUM_DOUZOU00			=  0;		#大きい銅像
$GRA_ENUM_DOUZOU0100		=  1;		#小さい銅像００
$GRA_ENUM_DOUZOU0101		=  2;		#小さい銅像０１
$GRA_ENUM_DOUZOU0102		=  3;		#小さい銅像０２
$GRA_ENUM_DOUZOU0103		=  4;		#小さい銅像０３
$GRA_ENUM_TREE				=  5;		#木
$GRA_ENUM_MG00				=  6;		#ミニゲーム００
$GRA_ENUM_MG01				=  7;		#ミニゲーム０１
$GRA_ENUM_MG02				=  8;		#ミニゲーム０２
$GRA_ENUM_TABLE				=  9;		#テーブル
$GRA_ENUM_KANBAN			= 10;		#テーブル
$GRA_ENUM_LAMP00			= 11;		#テーブル
$GRA_ENUM_LAMP01			= 12;		#テーブル
$GRA_ENUM_HANABI			= 13;		#花火
$GRA_ENUM_HANABIBIG			= 14;		#花火最後
$GRA_ENUM_BRHANABI			= 15;		#銅像花火
$GRA_ENUM_BRHANABIBIG		= 16;		#銅像花火最後
$GRA_ENUM_ANKETO			= 17;		#銅像花火最後
$GRA_ENUM_DOUZOU00ANM00		= 18;		#大きい銅像
$GRA_ENUM_DOUZOU00ANM01		= 19;		#大きい銅像
$GRA_ENUM_DOUZOU00ANM02		= 20;		#大きい銅像
$GRA_ENUM_DOUZOU0100ANM00	= 21;		#小さい銅像００
$GRA_ENUM_DOUZOU0100ANM01	= 22;		#小さい銅像００
$GRA_ENUM_DOUZOU0100ANM02	= 23;		#小さい銅像００
$GRA_ENUM_DOUZOU0101ANM00	= 24;		#小さい銅像０１
$GRA_ENUM_DOUZOU0101ANM01	= 25;		#小さい銅像０１
$GRA_ENUM_DOUZOU0101ANM02	= 26;		#小さい銅像０１
$GRA_ENUM_DOUZOU0102ANM00	= 27;		#小さい銅像０２
$GRA_ENUM_DOUZOU0102ANM01	= 28;		#小さい銅像０２
$GRA_ENUM_DOUZOU0102ANM02	= 29;		#小さい銅像０２
$GRA_ENUM_DOUZOU0103ANM00	= 30;		#小さい銅像０３
$GRA_ENUM_DOUZOU0103ANM01	= 31;		#小さい銅像０３
$GRA_ENUM_DOUZOU0103ANM02	= 32;		#小さい銅像０３
$GRA_ENUM_TREEANM00			= 33;		#木
$GRA_ENUM_TREEANM01			= 34;		#木
$GRA_ENUM_TREEANM02			= 35;		#木
$GRA_ENUM_MG00ANM00			= 36;		#ミニゲーム００
$GRA_ENUM_MG00ANM01			= 37;		#ミニゲーム００
$GRA_ENUM_MG00ANM02			= 38;		#ミニゲーム００
$GRA_ENUM_MG01ANM00			= 39;		#ミニゲーム０１
$GRA_ENUM_MG01ANM01			= 40;		#ミニゲーム０１
$GRA_ENUM_MG01ANM02			= 41;		#ミニゲーム０１
$GRA_ENUM_MG02ANM00			= 42;		#ミニゲーム０２
$GRA_ENUM_MG02ANM01			= 43;		#ミニゲーム０２
$GRA_ENUM_MG02ANM02			= 44;		#ミニゲーム０２
$GRA_ENUM_TABLEANM00		= 45;		#ミニゲーム０２
$GRA_ENUM_TABLEANM01		= 46;		#ミニゲーム０２
$GRA_ENUM_TABLEANM02		= 47;		#ミニゲーム０２
$GRA_ENUM_KANBANANM00		= 48;		#カンバン
$GRA_ENUM_KANBANANM01		= 49;		#カンバン
$GRA_ENUM_KANBANANM02		= 50;		#カンバン
$GRA_ENUM_LAMP00ANM00		= 51;		#ランプ００
$GRA_ENUM_LAMP00ANM01		= 52;		#ランプ００
$GRA_ENUM_LAMP00ANM02		= 53;		#ランプ００
$GRA_ENUM_LAMP01ANM00		= 54;		#ランプ０１
$GRA_ENUM_LAMP01ANM01		= 55;		#ランプ０１
$GRA_ENUM_LAMP01ANM02		= 56;		#ランプ０１
$GRA_ENUM_HANABIANM00		= 57;		#花火
$GRA_ENUM_HANABIANM01		= 58;		#花火
$GRA_ENUM_HANABIANM02		= 59;		#花火
$GRA_ENUM_HANABIBIGANM00	= 60;		#花火最後
$GRA_ENUM_HANABIBIGANM01	= 61;		#花火最後
$GRA_ENUM_HANABIBIGANM02	= 62;		#花火最後
$GRA_ENUM_BRHANABIANM00		= 63;		#銅像花火
$GRA_ENUM_BRHANABIANM01		= 64;		#銅像花火
$GRA_ENUM_BRHANABIANM02		= 65;		#銅像花火
$GRA_ENUM_BRHANABIBIGANM00	= 66;		#銅像花火最後
$GRA_ENUM_BRHANABIBIGANM01	= 67;		#銅像花火最後
$GRA_ENUM_BRHANABIBIGANM02	= 68;		#銅像花火最後
$GRA_ENUM_ANKETOANM00		= 69;		#アンケート
$GRA_ENUM_ANKETOANM01		= 70;		#アンケート
$GRA_ENUM_ANKETOANM02		= 71;		#アンケート
$GRA_ENUM_FLOAT00IMD		= 72;		#フロート00IMD
$GRA_ENUM_FLOAT01IMD		= 73;		#フロート01IMD
$GRA_ENUM_FLOAT00TEX00		= 74;		#フロート00色00
$GRA_ENUM_FLOAT00TEX01		= 75;		#フロート00色01
$GRA_ENUM_FLOAT00TEX02		= 76;		#フロート00色02
$GRA_ENUM_FLOAT01TEX00		= 77;		#フロート01色00
$GRA_ENUM_FLOAT01TEX01		= 78;		#フロート01色01
$GRA_ENUM_FLOAT01TEX02		= 79;		#フロート01色02
$GRA_ENUM_FLOAT00ANM00		= 80;		#フロート00アニメ00
$GRA_ENUM_FLOAT00ANM01		= 81;		#フロート00アニメ01
$GRA_ENUM_FLOAT00ANM02		= 82;		#フロート00アニメ02
$GRA_ENUM_FLOAT00ANM03		= 83;		#フロート00アニメ03
$GRA_ENUM_FLOAT01ANM00		= 84;		#フロート01アニメ00
$GRA_ENUM_FLOAT01ANM01		= 85;		#フロート01アニメ01
$GRA_ENUM_FLOAT01ANM02		= 86;		#フロート01アニメ02
$GRA_ENUM_FLOAT01ANM03		= 87;		#フロート01アニメ03
$GRA_ENUM_FLOAT00NODENUM	= 88;		#フロート00NODE数
$GRA_ENUM_FLOAT01NODENUM	= 89;		#フロート01NODE数
$GRA_ENUM_FLOAT00ANM01NODE	= 90;		#フロート00アニメ01NODE
$GRA_ENUM_FLOAT00ANM02NODE	= 91;		#フロート00アニメ02NODE
$GRA_ENUM_FLOAT00ANM03NODE	= 92;		#フロート00アニメ03NODE
$GRA_ENUM_FLOAT01ANM01NODE	= 93;		#フロート01アニメ01NODE
$GRA_ENUM_FLOAT01ANM02NODE	= 94;		#フロート01アニメ02NODE
$GRA_ENUM_FLOAT01ANM03NODE	= 95;		#フロート01アニメ03NODE
$GRA_ENUM_MAP				= 96;		#地形
$GRA_ENUM_MAT				= 97;		#マット
$GRA_ENUM_NODENUM			= 98;		#地形ノード数
$GRA_ENUM_MAPANM00			= 99;		#地形アニメ００
$GRA_ENUM_MAPANM01			=100;		#地形アニメ０１
$GRA_ENUM_MAPANM02			=101;		#地形アニメ０２
$GRA_ENUM_MAPANM03			=102;		#地形アニメ０３
$GRA_ENUM_MAPANM04			=103;		#地形ポール
$GRA_ENUM_MAPANM00NODE		=104;		#地形アニメ００ノード
$GRA_ENUM_MAPANM01NODE		=105;		#地形アニメ０１ノード
$GRA_ENUM_MAPANM02NODE		=106;		#地形アニメ０２ノード
$GRA_ENUM_MAPANM03NODE		=107;		#地形アニメ０３ノード
$GRA_ENUM_MAPANM04NODE		=108;		#地形ポールノード
$GRA_ENUM_NUM				=109;		#数


#グラフィックリスト内データ定数
$GRADATA_ENUM_ROOMNAME			=  0;		#部屋の名前
$GRADATA_ENUM_DOUZOU00			=  1;		#銅像大IMD
$GRADATA_ENUM_DOUZOU00_A00		=  2;		#銅像大常駐アニメ
$GRADATA_ENUM_DOUZOU00_A01		=  3;		#銅像大常駐アニメ
$GRADATA_ENUM_DOUZOU00_A02		=  4;		#銅像大常駐アニメ
$GRADATA_ENUM_DOUZOU01_00		=  5;		#銅像小IMD
$GRADATA_ENUM_DOUZOU01_00A		=  6;		#銅像小アニメ
$GRADATA_ENUM_DOUZOU01_01		=  7;		#銅像小IMD
$GRADATA_ENUM_DOUZOU01_01A		=  8;		#銅像小アニメ
$GRADATA_ENUM_DOUZOU01_02		=  9;		#銅像小IMD
$GRADATA_ENUM_DOUZOU01_02A		= 10;		#銅像小アニメ
$GRADATA_ENUM_DOUZOU01_03		= 11;		#銅像小IMD
$GRADATA_ENUM_DOUZOU01_03A		= 12;		#銅像小アニメ
$GRADATA_ENUM_DOUZOU01_03A1		= 13;		#銅像小アニメ
$GRADATA_ENUM_DOUZOU01_03A2		= 14;		#銅像小アニメ
$GRADATA_ENUM_TREE				= 15;		#木IMD
$GRADATA_ENUM_KANBAN			= 16;		#看板IMD
$GRADATA_ENUM_KANBANANM			= 17;		#看板IMD
$GRADATA_ENUM_TABLE				= 18;		#机IMD
$GRADATA_ENUM_LAMP00			= 19;		#ランプ００IMD
$GRADATA_ENUM_LAMP01			= 20;		#ランプ０１IMD
$GRADATA_ENUM_LAMPANM_00		= 21;		#ランプ点灯アニメ
$GRADATA_ENUM_LAMPANM_01		= 22;		#ランプゆれアニメ
$GRADATA_ENUM_LAMPANM_02		= 23;		#ランプゆれアニメ
$GRADATA_ENUM_HANABI			= 24;		#花火
$GRADATA_ENUM_HANABIANM00		= 25;		#花火通常アニメ
$GRADATA_ENUM_HANABIANM01		= 26;		#花火通常アニメ
$GRADATA_ENUM_HANABIANM02		= 27;		#花火通常アニメ
$GRADATA_ENUM_HANABIBIG			= 28;		#花火最後
$GRADATA_ENUM_HANABIBIGANM00	= 29;		#花火最後アニメ
$GRADATA_ENUM_HANABIBIGANM01	= 30;		#花火最後アニメ
$GRADATA_ENUM_BRHANABI			= 31;		#花火
$GRADATA_ENUM_BRHANABIANM00		= 32;		#花火通常アニメ
$GRADATA_ENUM_BRHANABIANM01		= 33;		#花火通常アニメ
$GRADATA_ENUM_BRHANABIANM02		= 34;		#花火通常アニメ
$GRADATA_ENUM_BRHANABIBIG		= 35;		#花火最後
$GRADATA_ENUM_BRHANABIBIGANM00	= 36;		#花火最後アニメ
$GRADATA_ENUM_BRHANABIBIGANM01	= 37;		#花火最後アニメ
$GRADATA_ENUM_BRHANABIBIGANM02	= 38;		#花火最後アニメ
$GRADATA_ENUM_ANKETO			= 39;		#花火最後アニメ
$GRADATA_ENUM_FLOAT00_00		= 40;		#フロート00色00IMD
$GRADATA_ENUM_FLOAT00_01		= 41;		#フロート00色01IMD
$GRADATA_ENUM_FLOAT00_02		= 42;		#フロート00色02IMD
$GRADATA_ENUM_FLOAT00_ANM00		= 43;		#フロート00アニメ00
$GRADATA_ENUM_FLOAT00_ANM01		= 44;		#フロート00アニメ01
$GRADATA_ENUM_FLOAT00_ANM01N	= 45;		#フロート00アニメ01NODE
$GRADATA_ENUM_FLOAT00_ANM02		= 46;		#フロート00アニメ02
$GRADATA_ENUM_FLOAT00_ANM02N	= 47;		#フロート00アニメ02NODE
$GRADATA_ENUM_FLOAT00_ANM03		= 48;		#フロート00アニメ03
$GRADATA_ENUM_FLOAT00_ANM03N	= 49;		#フロート00アニメ03NODE
$GRADATA_ENUM_FLOAT01_00		= 50;		#フロート01色00IMD
$GRADATA_ENUM_FLOAT01_01		= 51;		#フロート01色01IMD
$GRADATA_ENUM_FLOAT01_02		= 52;		#フロート01色02IMD
$GRADATA_ENUM_FLOAT01_ANM00		= 53;		#フロート01アニメ00
$GRADATA_ENUM_FLOAT01_ANM01		= 54;		#フロート01アニメ01
$GRADATA_ENUM_FLOAT01_ANM01N	= 55;		#フロート01アニメ01NODE
$GRADATA_ENUM_FLOAT01_ANM02		= 56;		#フロート01アニメ02
$GRADATA_ENUM_FLOAT01_ANM02N	= 57;		#フロート01アニメ02NODE
$GRADATA_ENUM_FLOAT01_ANM03		= 58;		#フロート01アニメ03
$GRADATA_ENUM_FLOAT01_ANM03N	= 59;		#フロート01アニメ03NODE
$GRADATA_ENUM_MAP				= 60;		#マップ
$GRADATA_ENUM_MAT				= 61;		#マット
$GRADATA_ENUM_MAPANM00			= 62;		#地形アニメ００
$GRADATA_ENUM_MAPANM00NODE		= 63;		#地形アニメ００ノード
$GRADATA_ENUM_MAPANM01			= 64;		#地形アニメ０１
$GRADATA_ENUM_MAPANM01NODE		= 65;		#地形アニメ０１ノード
$GRADATA_ENUM_MAPANM02			= 66;		#地形アニメ０２
$GRADATA_ENUM_MAPANM02NODE		= 67;		#地形アニメ０２ノード
$GRADATA_ENUM_MAPANM03			= 68;		#地形アニメ０２
$GRADATA_ENUM_MAPANM03NODE		= 69;		#地形アニメ０２ノード
$GRADATA_ENUM_MAPPOLL			= 70;		#地形ポール
$GRADATA_ENUM_MAPPOLLNODE		= 71;		#地形ポールノード
$GRADATA_ENUM_MG00				= 72;		#ミニゲーム００
$GRADATA_ENUM_MG00ANM00			= 73;		#ミニゲーム００アニメ００
$GRADATA_ENUM_MG00ANM01			= 74;		#ミニゲーム００アニメ０１
$GRADATA_ENUM_MG01				= 75;		#ミニゲーム０１
$GRADATA_ENUM_MG01ANM00			= 76;		#ミニゲーム０１アニメ００
$GRADATA_ENUM_MG01ANM01			= 77;		#ミニゲーム０１アニメ０１
$GRADATA_ENUM_MG02				= 78;		#ミニゲーム０２
$GRADATA_ENUM_MG02ANM00			= 79;		#ミニゲーム０２アニメ００
$GRADATA_ENUM_MG02ANM01			= 80;		#ミニゲーム０２アニメ０１
$GRADATA_ENUM_NUM				= 81;		#数
                                 
                                 
#ノード名がDUMMYのときの値       
$NODE_DUMMY		= 0xffffffff;    
                                 
#マップ配置オブジェアニメ数      
$MAPOBJ_ANM_MAX		= 2;	    #wflby_3dmapobj_data.hの配置オブジェアニメ最大数とあわせること
                                 
#NARC_NAME                       
$GRA_NARC_TOP				= "NARC_wifi_lobby_";
                                 
                                 
#ファイル読み込み開始オフセッ ト 
$GRADATA_YOFFS				= 3; 
                                 
                                 
#NODEインデックス作成            
system( "perl node_h_make.pl" ); 
                                 
#ファイル読み込み                
&filein();                       
                                 
#各部屋のデータを取得する        
&filedataset( \@GRA_NORMAL_FIRE,    0 );
&filedataset( \@GRA_NORMAL_WATER,	1 );
&filedataset( \@GRA_NORMAL_SPARK,	2 );
&filedataset( \@GRA_NORMAL_GRASS,	3 );
&filedataset( \@GRA_NORMAL_SPECIAL,	4 );

&filedataset( \@GRA_SPRING_FIRE,	5 );
&filedataset( \@GRA_SPRING_WATER,	6 );
&filedataset( \@GRA_SPRING_SPARK,	7 );
&filedataset( \@GRA_SPRING_GRASS,	8 );
&filedataset( \@GRA_SPRING_SPECIAL,	9 );

&filedataset( \@GRA_SUMMER_FIRE,	10 );
&filedataset( \@GRA_SUMMER_WATER,	11 );
&filedataset( \@GRA_SUMMER_SPARK,	12 );
&filedataset( \@GRA_SUMMER_GRASS,	13 );
&filedataset( \@GRA_SUMMER_SPECIAL,	14 );

&filedataset( \@GRA_FALL_FIRE,	15 );
&filedataset( \@GRA_FALL_WATER,	16 );
&filedataset( \@GRA_FALL_SPARK,	17 );
&filedataset( \@GRA_FALL_GRASS,	18 );
&filedataset( \@GRA_FALL_SPECIAL,19 );

&filedataset( \@GRA_WINTER_FIRE,	20 );
&filedataset( \@GRA_WINTER_WATER,	21 );
&filedataset( \@GRA_WINTER_SPARK,	22 );
&filedataset( \@GRA_WINTER_GRASS,	23 );
&filedataset( \@GRA_WINTER_SPECIAL,	24 );


#グラフィック部屋データ書き出し
&fileout_gradata();

#読み込んだので生成ファイルを消す
system( "rm $GRADATA_TXTFILE_NAME" );
system( "rm *.h" );

exit(0);

sub filein{
	system( "../../tools/exceltool/ExcelSeetConv.exe -o $GRADATA_TXTFILE_NAME -p 0 -s csv $GRADATA_FILE_NAME" );
	open( FILEIN, $GRADATA_TXTFILE_NAME );
	@GRADATA_FILE = <FILEIN>;
	close( FILEIN );

}


sub filedataset{
	my( $buff, $idx ) = @_;
	my( @onedata, $list_set ); 
	my( @node_h );

	$idx += $GRADATA_YOFFS;
	
	#@GRADATA_FILEの$idx番目の要素を@$buffに格納する
	$GRADATA_FILE[ $idx ] =~ s/\r\n//g;
	@onedata = split( /,/, $GRADATA_FILE[ $idx ] );	#,で区切って配列にする

	#配置オブジェ
	{
		#大きい銅像
		$list_set = $onedata[ $GRADATA_ENUM_DOUZOU00 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_DOUZOU00 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_DOUZOU00 ] =~ s/\./_/;

		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_DOUZOU00_A00 ], $buff, $GRA_ENUM_DOUZOU00ANM00, $$buff[ $GRA_ENUM_DOUZOU00 ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_DOUZOU00_A01 ], $buff, $GRA_ENUM_DOUZOU00ANM01, $$buff[ $GRA_ENUM_DOUZOU00 ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_DOUZOU00_A02 ], $buff, $GRA_ENUM_DOUZOU00ANM02, $$buff[ $GRA_ENUM_DOUZOU00 ] );


		#小さい銅像00
		$list_set = $onedata[ $GRADATA_ENUM_DOUZOU01_00 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_DOUZOU0100 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_DOUZOU0100 ] =~ s/\./_/;
		
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_DOUZOU01_00A ], $buff, $GRA_ENUM_DOUZOU0100ANM00, $$buff[ $GRA_ENUM_DOUZOU0100 ] );

		#アニメはないのでモデルと同じメッセージを入れときます
		$$buff[ $GRA_ENUM_DOUZOU0100ANM01 ] = $$buff[ $GRA_ENUM_DOUZOU0100 ];
		$$buff[ $GRA_ENUM_DOUZOU0100ANM02 ] = $$buff[ $GRA_ENUM_DOUZOU0100 ];

		
		#小さい銅像01
		$list_set = $onedata[ $GRADATA_ENUM_DOUZOU01_01 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_DOUZOU0101 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_DOUZOU0101 ] =~ s/\./_/;
		
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_DOUZOU01_01A ], $buff, $GRA_ENUM_DOUZOU0101ANM00, $$buff[ $GRA_ENUM_DOUZOU0101 ] );

		#アニメはないのでモデルと同じメッセージを入れときます
		$$buff[ $GRA_ENUM_DOUZOU0101ANM01 ] = $$buff[ $GRA_ENUM_DOUZOU0101 ];
		$$buff[ $GRA_ENUM_DOUZOU0101ANM02 ] = $$buff[ $GRA_ENUM_DOUZOU0101 ];


		#小さい銅像02
		$list_set = $onedata[ $GRADATA_ENUM_DOUZOU01_02 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_DOUZOU0102 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_DOUZOU0102 ] =~ s/\./_/;
		
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_DOUZOU01_02A ], $buff, $GRA_ENUM_DOUZOU0102ANM00, $$buff[ $GRA_ENUM_DOUZOU0102 ] );

		#アニメはないのでモデルと同じメッセージを入れときます
		$$buff[ $GRA_ENUM_DOUZOU0102ANM01 ] = $$buff[ $GRA_ENUM_DOUZOU0102 ];
		$$buff[ $GRA_ENUM_DOUZOU0102ANM02 ] = $$buff[ $GRA_ENUM_DOUZOU0102 ];


		#小さい銅像03
		$list_set = $onedata[ $GRADATA_ENUM_DOUZOU01_03 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_DOUZOU0103 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_DOUZOU0103 ] =~ s/\./_/;
		
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_DOUZOU01_03A ], $buff, $GRA_ENUM_DOUZOU0103ANM00, $$buff[ $GRA_ENUM_DOUZOU0103 ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_DOUZOU01_03A1 ], $buff, $GRA_ENUM_DOUZOU0103ANM01, $$buff[ $GRA_ENUM_DOUZOU0103 ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_DOUZOU01_03A2 ], $buff, $GRA_ENUM_DOUZOU0103ANM02, $$buff[ $GRA_ENUM_DOUZOU0103 ] );

		

		#木
		$list_set = $onedata[ $GRADATA_ENUM_TREE ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_TREE ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_TREE ] =~ s/\./_/;

		#木アニメはないのでモデルと同じメッセージを入れときます
		$$buff[ $GRA_ENUM_TREEANM00 ] = $$buff[ $GRA_ENUM_TREE ];
		$$buff[ $GRA_ENUM_TREEANM01 ] = $$buff[ $GRA_ENUM_TREE ];
		$$buff[ $GRA_ENUM_TREEANM02 ] = $$buff[ $GRA_ENUM_TREE ];


		#机
		$list_set = $onedata[ $GRADATA_ENUM_TABLE ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_TABLE ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_TABLE ] =~ s/\./_/;

		#アニメはないのでモデルと同じメッセージを入れときます
		$$buff[ $GRA_ENUM_TABLEANM00 ] = $$buff[ $GRA_ENUM_TABLE ];
		$$buff[ $GRA_ENUM_TABLEANM01 ] = $$buff[ $GRA_ENUM_TABLE ];
		$$buff[ $GRA_ENUM_TABLEANM02 ] = $$buff[ $GRA_ENUM_TABLE ];


		#看板
		$list_set = $onedata[ $GRADATA_ENUM_KANBAN ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_KANBAN ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_KANBAN ] =~ s/\./_/;

		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_KANBANANM ], $buff, $GRA_ENUM_KANBANANM00, $$buff[ $GRA_ENUM_KANBAN ] );

		#アニメはないのでモデルと同じメッセージを入れときます
		$$buff[ $GRA_ENUM_KANBANANM01 ] = $$buff[ $GRA_ENUM_KANBAN ];
		$$buff[ $GRA_ENUM_KANBANANM02 ] = $$buff[ $GRA_ENUM_KANBAN ];
		

		#ランプ００
		$list_set = $onedata[ $GRADATA_ENUM_LAMP00 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_LAMP00 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_LAMP00 ] =~ s/\./_/;

		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_LAMPANM_00 ], $buff, $GRA_ENUM_LAMP00ANM00, $$buff[ $GRA_ENUM_LAMP00 ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_LAMPANM_01 ], $buff, $GRA_ENUM_LAMP00ANM01, $$buff[ $GRA_ENUM_LAMP00 ] );

		#アニメはないのでモデルと同じメッセージを入れときます
		$$buff[ $GRA_ENUM_LAMP00ANM02 ] = $$buff[ $GRA_ENUM_LAMP00 ];


		#ランプ０１
		$list_set = $onedata[ $GRADATA_ENUM_LAMP01 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_LAMP01 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_LAMP01 ] =~ s/\./_/;

		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_LAMPANM_00 ], $buff, $GRA_ENUM_LAMP01ANM00, $$buff[ $GRA_ENUM_LAMP01 ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_LAMPANM_02 ], $buff, $GRA_ENUM_LAMP01ANM01, $$buff[ $GRA_ENUM_LAMP01 ] );

		#アニメはないのでモデルと同じメッセージを入れときます
		$$buff[ $GRA_ENUM_LAMP01ANM02 ] = $$buff[ $GRA_ENUM_LAMP01 ];

		#花火
		$list_set = $onedata[ $GRADATA_ENUM_HANABI ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_HANABI ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_HANABI ] =~ s/\./_/;

		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_HANABIANM00 ], $buff, $GRA_ENUM_HANABIANM00, $$buff[ $GRA_ENUM_HANABI ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_HANABIANM01 ], $buff, $GRA_ENUM_HANABIANM01, $$buff[ $GRA_ENUM_HANABI ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_HANABIANM02 ], $buff, $GRA_ENUM_HANABIANM02, $$buff[ $GRA_ENUM_HANABI ] );


		#花火最後
		$list_set = $onedata[ $GRADATA_ENUM_HANABIBIG ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_HANABIBIG ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_HANABIBIG ] =~ s/\./_/;

		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_HANABIBIGANM00 ], $buff, $GRA_ENUM_HANABIBIGANM00, $$buff[ $GRA_ENUM_HANABIBIG ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_HANABIBIGANM01 ], $buff, $GRA_ENUM_HANABIBIGANM01, $$buff[ $GRA_ENUM_HANABIBIG ] );

		#アニメはないのでモデルと同じメッセージを入れときます
		$$buff[ $GRA_ENUM_HANABIBIGANM02 ] = $$buff[ $GRA_ENUM_HANABIBIG ];


		#銅像花火
		$list_set = $onedata[ $GRADATA_ENUM_BRHANABI ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_BRHANABI ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_BRHANABI ] =~ s/\./_/;

		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_BRHANABIANM00 ], $buff, $GRA_ENUM_BRHANABIANM00, $$buff[ $GRA_ENUM_BRHANABI ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_BRHANABIANM01 ], $buff, $GRA_ENUM_BRHANABIANM01, $$buff[ $GRA_ENUM_BRHANABI ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_BRHANABIANM02 ], $buff, $GRA_ENUM_BRHANABIANM02, $$buff[ $GRA_ENUM_BRHANABI ] );

		#銅像花火最後
		$list_set = $onedata[ $GRADATA_ENUM_BRHANABIBIG ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_BRHANABIBIG ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_BRHANABIBIG ] =~ s/\./_/;

		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_BRHANABIBIGANM00 ], $buff, $GRA_ENUM_BRHANABIBIGANM00, $$buff[ $GRA_ENUM_BRHANABIBIG ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_BRHANABIBIGANM01 ], $buff, $GRA_ENUM_BRHANABIBIGANM01, $$buff[ $GRA_ENUM_BRHANABIBIG ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_BRHANABIBIGANM02 ], $buff, $GRA_ENUM_BRHANABIBIGANM02, $$buff[ $GRA_ENUM_BRHANABIBIG ] );

		#アンケート
		$list_set = $onedata[ $GRADATA_ENUM_ANKETO ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_ANKETO ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_ANKETO ] =~ s/\./_/;

		#木アニメはないのでモデルと同じメッセージを入れときます
		$$buff[ $GRA_ENUM_ANKETOANM00 ] = $$buff[ $GRA_ENUM_ANKETO ];
		$$buff[ $GRA_ENUM_ANKETOANM01 ] = $$buff[ $GRA_ENUM_ANKETO ];
		$$buff[ $GRA_ENUM_ANKETOANM02 ] = $$buff[ $GRA_ENUM_ANKETO ];


		#ミニゲーム０
		$list_set = $onedata[ $GRADATA_ENUM_MG00 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_MG00 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_MG00 ] =~ s/\./_/;
		
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_MG00ANM00 ], $buff, $GRA_ENUM_MG00ANM00, $$buff[ $GRA_ENUM_MG00 ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_MG00ANM01 ], $buff, $GRA_ENUM_MG00ANM01, $$buff[ $GRA_ENUM_MG00 ] );

		#アニメはないのでモデルと同じメッセージを入れときます
		$$buff[ $GRA_ENUM_MG00ANM02 ] = $$buff[ $GRA_ENUM_MG00 ];


		#ミニゲーム１
		$list_set = $onedata[ $GRADATA_ENUM_MG01 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_MG01 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_MG01 ] =~ s/\./_/;

		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_MG01ANM00 ], $buff, $GRA_ENUM_MG01ANM00, $$buff[ $GRA_ENUM_MG01 ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_MG01ANM01 ], $buff, $GRA_ENUM_MG01ANM01, $$buff[ $GRA_ENUM_MG01 ] );
		
		#アニメはないのでモデルと同じメッセージを入れときます
		$$buff[ $GRA_ENUM_MG01ANM02 ] = $$buff[ $GRA_ENUM_MG01 ];

		#ミニゲーム２
		$list_set = $onedata[ $GRADATA_ENUM_MG02 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_MG02 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_MG02 ] =~ s/\./_/;

		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_MG02ANM00 ], $buff, $GRA_ENUM_MG02ANM00, $$buff[ $GRA_ENUM_MG02 ] );
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_MG02ANM01 ], $buff, $GRA_ENUM_MG02ANM01, $$buff[ $GRA_ENUM_MG02 ] );
		
		#アニメはないのでモデルと同じメッセージを入れときます
		$$buff[ $GRA_ENUM_MG02ANM02 ] = $$buff[ $GRA_ENUM_MG02 ];

	
	}

	#FLOAT
	{
		#フロート00IMD
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT00_00 ];
		&gra_conv_list_add( $list_set." -emdl" );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT00IMD ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT00IMD ] =~ s/\./_/;

		#フロート01IMD
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT01_00 ];
		&gra_conv_list_add( $list_set." -emdl" );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT01IMD ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT01IMD ] =~ s/\./_/;

		#フロート00TEX00
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT00_00 ];
		&gra_conv_list_add( $list_set." -etex" );		#コンバートリストに設定
		$list_set =~ s/imd/nsbtx/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT00TEX00 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT00TEX00 ] =~ s/\./_/;
		#フロート00TEX01
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT00_01 ];
		&gra_conv_list_add( $list_set." -etex" );		#コンバートリストに設定
		$list_set =~ s/imd/nsbtx/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT00TEX01 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT00TEX01 ] =~ s/\./_/;
		#フロート00TEX02
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT00_02 ];
		&gra_conv_list_add( $list_set." -etex" );		#コンバートリストに設定
		$list_set =~ s/imd/nsbtx/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT00TEX02 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT00TEX02 ] =~ s/\./_/;

		#フロート01TEX00
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT01_00 ];
		&gra_conv_list_add( $list_set." -etex" );		#コンバートリストに設定
		$list_set =~ s/imd/nsbtx/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT01TEX00 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT01TEX00 ] =~ s/\./_/;
		#フロート01TEX01
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT01_01 ];
		&gra_conv_list_add( $list_set." -etex" );		#コンバートリストに設定
		$list_set =~ s/imd/nsbtx/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT01TEX01 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT01TEX01 ] =~ s/\./_/;
		#フロート01TEX02
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT01_02 ];
		&gra_conv_list_add( $list_set." -etex" );		#コンバートリストに設定
		$list_set =~ s/imd/nsbtx/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT01TEX02 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT01TEX02 ] =~ s/\./_/;

		#フロート00ANM00
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT00_ANM00 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set = &anm_file_change( $list_set );	#アニメファイル名を変更
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT00ANM00 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT00ANM00 ] =~ s/\./_/;
		#フロート00ANM01
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT00_ANM01 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set = &anm_file_change( $list_set );	#アニメファイル名を変更
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT00ANM01 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT00ANM01 ] =~ s/\./_/;
		#フロート00ANM02
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT00_ANM02 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set = &anm_file_change( $list_set );	#アニメファイル名を変更
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT00ANM02 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT00ANM02 ] =~ s/\./_/;

		#フロート00ANM03
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT00_ANM03 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set = &anm_file_change( $list_set );	#アニメファイル名を変更
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT00ANM03 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT00ANM03 ] =~ s/\./_/;

		#フロート01ANM00
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT01_ANM00 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set = &anm_file_change( $list_set );	#アニメファイル名を変更
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT01ANM00 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT01ANM00 ] =~ s/\./_/;
		#フロート01ANM01
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT01_ANM01 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set = &anm_file_change( $list_set );	#アニメファイル名を変更
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT01ANM01 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT01ANM01 ] =~ s/\./_/;
		#フロート01ANM02
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT01_ANM02 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set = &anm_file_change( $list_set );	#アニメファイル名を変更
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT01ANM02 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT01ANM02 ] =~ s/\./_/;

		#フロート01ANM03
		$list_set = $onedata[ $GRADATA_ENUM_FLOAT01_ANM03 ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set = &anm_file_change( $list_set );	#アニメファイル名を変更
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_FLOAT01ANM03 ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_FLOAT01ANM03 ] =~ s/\./_/;


		#ノードデータ
		#FLOAT00
		{
			#ファイル名からノードヘッダを読み込み
			$list_set = $onedata[ $GRADATA_ENUM_FLOAT00_00 ];
			$list_set =~ s/imd/h/g;
			open( FILEIN, "$list_set" );
			@node_h = <FILEIN>;
			close( FILEIN );

			#NODEMAX
			$$buff[ $GRA_ENUM_FLOAT00NODENUM ] = &get_node_h_max( \@node_h );

			#FLOAT00ANM01NODE
			$list_set = $onedata[ $GRADATA_ENUM_FLOAT00_ANM01N ];
			$$buff[ $GRA_ENUM_FLOAT00ANM01NODE ] = &get_node_h_nodeno( \@node_h, $list_set );

			#FLOAT00ANM02NODE
			$list_set = $onedata[ $GRADATA_ENUM_FLOAT00_ANM02N ];
			$$buff[ $GRA_ENUM_FLOAT00ANM02NODE ] = &get_node_h_nodeno( \@node_h, $list_set );

			#FLOAT00ANM03NODE
			$list_set = $onedata[ $GRADATA_ENUM_FLOAT00_ANM03N ];
			$$buff[ $GRA_ENUM_FLOAT00ANM03NODE ] = &get_node_h_nodeno( \@node_h, $list_set );
		}
		#FLOAT01
		{
			#ファイル名からノードヘッダを読み込み
			$list_set = $onedata[ $GRADATA_ENUM_FLOAT01_00 ];
			$list_set =~ s/imd/h/g;
			open( FILEIN, "$list_set" );
			@node_h = <FILEIN>;
			close( FILEIN );

			#NODEMAX
			$$buff[ $GRA_ENUM_FLOAT01NODENUM ] = &get_node_h_max( \@node_h );

			#FLOAT01ANM01NODE
			$list_set = $onedata[ $GRADATA_ENUM_FLOAT01_ANM01N ];
			$$buff[ $GRA_ENUM_FLOAT01ANM01NODE ] = &get_node_h_nodeno( \@node_h, $list_set );

			#FLOAT01ANM02NODE
			$list_set = $onedata[ $GRADATA_ENUM_FLOAT01_ANM02N ];
			$$buff[ $GRA_ENUM_FLOAT01ANM02NODE ] = &get_node_h_nodeno( \@node_h, $list_set );

			#FLOAT01ANM03NODE
			$list_set = $onedata[ $GRADATA_ENUM_FLOAT01_ANM03N ];
			$$buff[ $GRA_ENUM_FLOAT01ANM03NODE ] = &get_node_h_nodeno( \@node_h, $list_set );
		}
	}
	
	#マップ
	{
		#MAP
		$list_set = $onedata[ $GRADATA_ENUM_MAP ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_MAP ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_MAP ] =~ s/\./_/;

		#MAT
		$list_set = $onedata[ $GRADATA_ENUM_MAT ];
		&gra_conv_list_add( $list_set );		#コンバートリストに設定
		$list_set =~ s/imd/nsbmd/;
		&gra_list_add( $list_set );
		$$buff[ $GRA_ENUM_MAT ] = "$GRA_NARC_TOP".$list_set;
		$$buff[ $GRA_ENUM_MAT ] =~ s/\./_/;

		#MAPANM00
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_MAPANM00 ], $buff, $GRA_ENUM_MAPANM00, $$buff[ $GRA_ENUM_MAP ] );

		#MAPANM01
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_MAPANM01 ], $buff, $GRA_ENUM_MAPANM01, $$buff[ $GRA_ENUM_MAP ] );

		#MAPANM02
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_MAPANM02 ], $buff, $GRA_ENUM_MAPANM02, $$buff[ $GRA_ENUM_MAP ] );

		#MAPANM03
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_MAPANM03 ], $buff, $GRA_ENUM_MAPANM03, $$buff[ $GRA_ENUM_MAP ] );

		#MAPANMPOLL
		&set3dmapobj_anm( $onedata[ $GRADATA_ENUM_MAPPOLL ], $buff, $GRA_ENUM_MAPANM04, $$buff[ $GRA_ENUM_MAP ] );


		#MAPファイル名からノードヘッダを読み込み
		$list_set = $onedata[ $GRADATA_ENUM_MAP ];
		$list_set =~ s/imd/h/g;
		open( FILEIN, "$list_set" );
		@node_h = <FILEIN>;
		close( FILEIN );


		#NODEMAX
		$$buff[ $GRA_ENUM_NODENUM ] = &get_node_h_max( \@node_h );

		#MAPANM00NODE
		$list_set = $onedata[ $GRADATA_ENUM_MAPANM00NODE ];
		if( $list_set eq "DUMMY" ){	#DUMMYが入っているところは使っていないところなのでどんな数字でも大丈夫
			$$buff[ $GRA_ENUM_MAPANM00NODE ] = 0;
		}else{
			$$buff[ $GRA_ENUM_MAPANM00NODE ] = &get_node_h_nodeno( \@node_h, $list_set );
		}

		#MAPANM01NODE
		$list_set = $onedata[ $GRADATA_ENUM_MAPANM01NODE ];
		if( $list_set eq "DUMMY" ){	#DUMMYが入っているところは使っていないところなのでどんな数字でも大丈夫
			$$buff[ $GRA_ENUM_MAPANM01NODE ] = 0;
		}else{
			$$buff[ $GRA_ENUM_MAPANM01NODE ] = &get_node_h_nodeno( \@node_h, $list_set );
		}

		#MAPANM02NODE
		$list_set = $onedata[ $GRADATA_ENUM_MAPANM02NODE ];
		if( $list_set eq "DUMMY" ){	#DUMMYが入っているところは使っていないところなのでどんな数字でも大丈夫
			$$buff[ $GRA_ENUM_MAPANM02NODE ] = 0;
		}else{
			$$buff[ $GRA_ENUM_MAPANM02NODE ] = &get_node_h_nodeno( \@node_h, $list_set );
		}

		#MAPANM03NODE
		$list_set = $onedata[ $GRADATA_ENUM_MAPANM03NODE ];
		if( $list_set eq "DUMMY" ){	#DUMMYが入っているところは使っていないところなのでどんな数字でも大丈夫
			$$buff[ $GRA_ENUM_MAPANM03NODE ] = 0;
		}else{
			$$buff[ $GRA_ENUM_MAPANM03NODE ] = &get_node_h_nodeno( \@node_h, $list_set );
		}

		#MAPANM04NODE
		$list_set = $onedata[ $GRADATA_ENUM_MAPPOLLNODE ];
		if( $list_set eq "DUMMY" ){	#DUMMYが入っているところは使っていないところなのでどんな数字でも大丈夫
			$$buff[ $GRA_ENUM_MAPANM04NODE ] = 0;
		}else{
			$$buff[ $GRA_ENUM_MAPANM04NODE ] = &get_node_h_nodeno( \@node_h, $list_set );
		}
	}
}


#マップ配置オブジェアニメデータ設定関数
#マップオブジェアニメデータ設定関数
sub set3dmapobj_anm{
	my( $list_set, $buff, $buffidx, $mdlfile ) = @_;

	#モデルファイルじゃないかチェック
	if( $list_set =~ /imd/ ){

		print( "not anm $list_set\n" );
		$$buff[ $buffidx ] = $mdlfile;
	}else{

		if( $list_set eq "DUMMY" ){
			$$buff[ $buffidx ] = $mdlfile;
		}else{
			&gra_conv_list_add( $list_set );		#コンバートリストに設定
			$list_set = &anm_file_change( $list_set );	#アニメファイル名を変更
			&gra_list_add( $list_set );
			$$buff[ $buffidx ] = "$GRA_NARC_TOP".$list_set;
			$$buff[ $buffidx ] =~ s/\./_/;
		}
	}
}


#node_hからノード最大数を取得
sub get_node_h_max{
	my( $file ) = @_;
	
	foreach $data ( @$file ){
		if( $data =~ /NODEIDXMAX[\t]*\(([0-9]*)\)/ ){
			return $1;
		}
	}
	print( "NODEINXMAX none\n" );
	exit(1);
}

#node_hからノード名のノードNOを取得
sub get_node_h_nodeno{
	my( $file, $name ) = @_;

	#サーチする前にその名前がDUMMYのときはダミーの値を返す
	if( $name eq "DUMMY" ){
		return $NODE_DUMMY;
	}
	
	foreach $data ( @$file ){
		if( $data =~ /$name[\t]*\(([0-9]*)\)/ ){
			return $1;
		}
	}

	print( "NODENO none $name\n" );
	exit(1);
}


#アニメファイル名チェンジ関数
sub anm_file_change{
	my( $filename ) = @_;

	$filename =~ s/imd/nsbmd/;
	$filename =~ s/ica/nsbca/;
	$filename =~ s/ima/nsbma/;
	$filename =~ s/itp/nsbtp/;
	$filename =~ s/ita/nsbta/;
	$filename =~ s/iva/nsbva/;
	
	return $filename;
}


#グラフィックアーカイブリストに追加
sub gra_list_add{
	my( $str ) = @_;
	my( $i );

	
	#サーチする
	for( $i=0; $i<@GRA_LIST; $i++ ){
		if( $str eq $GRA_LIST[ $i ] ){
			#重複するファイルがあるので追加しない
			return;
		}else{
		}
	}
	$GRA_LIST[ $i ] = $str;
}

#コンバートリストに追加
sub gra_conv_list_add{
	my( $str ) = @_;
	my( $i );

	#サーチする
	for( $i=0; $i<@GRA_LIST; $i++ ){
		if( $str eq $GRA_LIST[ $i ] ){
			#重複するファイルがあるので追加しない
			return;
		}else{
		}
	}
	$GRA_CONVLIST[ $i ] = $str;
}



#グラフィック部屋データ書き出し
sub fileout_gradata()
{
	my( $i );
	
	#アーカイブリスト
	open( FILEOUT, ">$GRADATA_ARCLIST_FILE");
	for( $i=1; $i<@GRA_LIST; $i++ ){	#なぜかidx0に空白があるので1らか出す
		print( FILEOUT "\"".$GRA_LIST[ $i ]."\"\n" );
	}
	close( FILEOUT );

	#コンバートリスト
	open( FILEOUT, ">$GRADATA_CONVLIST_FILE" );
	for( $i=1; $i<@GRA_CONVLIST; $i++ ){	#なぜかidx0に空白があるので1らか出す
		print( FILEOUT "g3dcvtr ".$GRA_CONVLIST[ $i ]."\n" );
	}
	close( FILEOUT );


	#部屋別グラフィックデータ
	open( FILEOUT, ">$GRADATA_OUTGRADATAFILE" );
	print( FILEOUT "// このファイルはpl_conv/wifi_lobby/wflby_gradta_conv.plで自動生成されます\n\n\n\n\n\n" );
	print( FILEOUT "static const u32 sc_data[ WFLBY_SEASON_NUM * WFLBY_ROOM_TYPE  ] = {\n" );

	print( FILEOUT "// ノーマル　火の部屋\n" );
	for( $i=0; $i<@GRA_NORMAL_FIRE; $i++ ){
		print( FILEOUT "\t".$GRA_NORMAL_FIRE[ $i ].",\n" );
	}
	print( FILEOUT "// ノーマル　	水の部屋\n" );
	for( $i=0; $i<@GRA_NORMAL_WATER; $i++ ){
		print( FILEOUT "\t".$GRA_NORMAL_WATER[ $i ].",\n" );
	}
	print( FILEOUT "// ノーマル　	電気の部屋\n" );
	for( $i=0; $i<@GRA_NORMAL_SPARK; $i++ ){
		print( FILEOUT "\t".$GRA_NORMAL_SPARK[ $i ].",\n" );
	}
	print( FILEOUT "// ノーマル　	草の部屋\n" );
	for( $i=0; $i<@GRA_NORMAL_GRASS; $i++ ){
		print( FILEOUT "\t".$GRA_NORMAL_GRASS[ $i ].",\n" );
	}
	print( FILEOUT "// ノーマル　	特殊の部屋\n" );
	for( $i=0; $i<@GRA_NORMAL_SPECIAL; $i++ ){
		print( FILEOUT "\t".$GRA_NORMAL_SPECIAL[ $i ].",\n" );
	}


	print( FILEOUT "// 春　火の部屋\n" );
	for( $i=0; $i<@GRA_SPRING_FIRE; $i++ ){
		print( FILEOUT "\t".$GRA_SPRING_FIRE[ $i ].",\n" );
	}
	print( FILEOUT "// 春　	水の部屋\n" );
	for( $i=0; $i<@GRA_SPRING_WATER; $i++ ){
		print( FILEOUT "\t".$GRA_SPRING_WATER[ $i ].",\n" );
	}
	print( FILEOUT "// 春　	電気の部屋\n" );
	for( $i=0; $i<@GRA_SPRING_SPARK; $i++ ){
		print( FILEOUT "\t".$GRA_SPRING_SPARK[ $i ].",\n" );
	}
	print( FILEOUT "// 春　	草の部屋\n" );
	for( $i=0; $i<@GRA_SPRING_GRASS; $i++ ){
		print( FILEOUT "\t".$GRA_SPRING_GRASS[ $i ].",\n" );
	}
	print( FILEOUT "// 春　	特殊の部屋\n" );
	for( $i=0; $i<@GRA_SPRING_SPECIAL; $i++ ){
		print( FILEOUT "\t".$GRA_SPRING_SPECIAL[ $i ].",\n" );
	}

	print( FILEOUT "// 夏　火の部屋\n" );
	for( $i=0; $i<@GRA_SUMMER_FIRE; $i++ ){
		print( FILEOUT "\t".$GRA_SUMMER_FIRE[ $i ].",\n" );
	}
	print( FILEOUT "// 夏　	水の部屋\n" );
	for( $i=0; $i<@GRA_SUMMER_WATER; $i++ ){
		print( FILEOUT "\t".$GRA_SUMMER_WATER[ $i ].",\n" );
	}
	print( FILEOUT "// 夏　	電気の部屋\n" );
	for( $i=0; $i<@GRA_SUMMER_SPARK; $i++ ){
		print( FILEOUT "\t".$GRA_SUMMER_SPARK[ $i ].",\n" );
	}
	print( FILEOUT "// 夏　	草の部屋\n" );
	for( $i=0; $i<@GRA_SUMMER_GRASS; $i++ ){
		print( FILEOUT "\t".$GRA_SUMMER_GRASS[ $i ].",\n" );
	}
	print( FILEOUT "// 夏　	特殊の部屋\n" );
	for( $i=0; $i<@GRA_SUMMER_SPECIAL; $i++ ){
		print( FILEOUT "\t".$GRA_SUMMER_SPECIAL[ $i ].",\n" );
	}

	print( FILEOUT "// 秋　火の部屋\n" );
	for( $i=0; $i<@GRA_FALL_FIRE; $i++ ){
		print( FILEOUT "\t".$GRA_FALL_FIRE[ $i ].",\n" );
	}
	print( FILEOUT "// 秋　	水の部屋\n" );
	for( $i=0; $i<@GRA_FALL_WATER; $i++ ){
		print( FILEOUT "\t".$GRA_FALL_WATER[ $i ].",\n" );
	}
	print( FILEOUT "// 秋　	電気の部屋\n" );
	for( $i=0; $i<@GRA_FALL_SPARK; $i++ ){
		print( FILEOUT "\t".$GRA_FALL_SPARK[ $i ].",\n" );
	}
	print( FILEOUT "// 秋　	草の部屋\n" );
	for( $i=0; $i<@GRA_FALL_GRASS; $i++ ){
		print( FILEOUT "\t".$GRA_FALL_GRASS[ $i ].",\n" );
	}
	print( FILEOUT "// 秋　	特殊の部屋\n" );
	for( $i=0; $i<@GRA_FALL_SPECIAL; $i++ ){
		print( FILEOUT "\t".$GRA_FALL_SPECIAL[ $i ].",\n" );
	}

	print( FILEOUT "// 冬　火の部屋\n" );
	for( $i=0; $i<@GRA_WINTER_FIRE; $i++ ){
		print( FILEOUT "\t".$GRA_WINTER_FIRE[ $i ].",\n" );
	}
	print( FILEOUT "// 冬　	水の部屋\n" );
	for( $i=0; $i<@GRA_WINTER_WATER; $i++ ){
		print( FILEOUT "\t".$GRA_WINTER_WATER[ $i ].",\n" );
	}
	print( FILEOUT "// 冬　	電気の部屋\n" );
	for( $i=0; $i<@GRA_WINTER_SPARK; $i++ ){
		print( FILEOUT "\t".$GRA_WINTER_SPARK[ $i ].",\n" );
	}
	print( FILEOUT "// 冬　	草の部屋\n" );
	for( $i=0; $i<@GRA_WINTER_GRASS; $i++ ){
		print( FILEOUT "\t".$GRA_WINTER_GRASS[ $i ].",\n" );
	}
	print( FILEOUT "// 冬　	特殊の部屋\n" );
	for( $i=0; $i<@GRA_WINTER_SPECIAL; $i++ ){
		print( FILEOUT "\t".$GRA_WINTER_SPECIAL[ $i ].",\n" );
	}

	print( FILEOUT "};\n" );
	
	close( FILEOUT );
}
