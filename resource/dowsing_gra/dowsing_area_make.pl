##============================================================================
=pod
     file   dowsing_area_make.pl
     brief  ダウジングのサーチ範囲内における
            アイテムの位置とロッドの状態の対応テーブル
            を作成する。
            同時に、
            ロッドの状態のenum値を記したヘッダーファイル
            も作成する。
     author Koji Kawada
     data   2010.02.17
     note   perl dowsing_area_make.pl dowsing_area.csv

     note   resource/dowsing_gra/dowsing_area.csv(ShiftJIS)

     モジュール名：
=cut 
##============================================================================


##=============================================================================
=pid

フォーマット
    u8(u8だからリトルエンディアンとかない)
    横32 * 縦24 で
      0   1   2 ...  31
     32  33  34 ...  63
    ...
    736 737 738 ... 767
    という順番に並んでいる。
    自分の位置を原点(0,0)としたときのアイテムの位置を(x,y)とすると
    (→xが大きくなる、↓yが大きくなる)
    -15<=x<=16 && -11<=y<=12 のとき
    [(y+11)*32 + (x+15)] 
    がロッドの状態となる。
    この範囲外ではNNとなる。
    別の書き方をすると、
    0<=x+origin_x<width && 0<=y+origin_y<height のとき
    [(y+origin_y)*width + (x+origin_x)]
    となる。

=cut
##=============================================================================


# このソース自体をUTF8で記述し、UTF8で処理を行う
use utf8;
binmode STDIN,  ":encoding(shiftjis)";
binmode STDOUT, ":encoding(shiftjis)";
binmode STDERR, ":encoding(shiftjis)";

# インクルード
use Encode;


##=============================================================================

%rod_easy_enum =
(
  "UU"=>0,
  "DD"=>1,
  "LL"=>2,
  "RR"=>3,
  "LU"=>4,
  "LD"=>5,
  "RU"=>6,
  "RD"=>7,
  "AA"=>8,
  "NN"=>9,
);
$rod_easy_enum_unknown = 10;  # 入力されていないときや、%rod_easy_enumに列挙されていないもののとき

@rod_enum =
(
  [ "ROD_U"    , $rod_easy_enum{"UU"}    ],
  [ "ROD_D"    , $rod_easy_enum{"DD"}    ],
  [ "ROD_L"    , $rod_easy_enum{"LL"}    ],
  [ "ROD_R"    , $rod_easy_enum{"RR"}    ],
  [ "ROD_LU"   , $rod_easy_enum{"LU"}    ],
  [ "ROD_LD"   , $rod_easy_enum{"LD"}    ],
  [ "ROD_RU"   , $rod_easy_enum{"RU"}    ],
  [ "ROD_RD"   , $rod_easy_enum{"RD"}    ],
  [ "ROD_ABOVE", $rod_easy_enum{"AA"}    ],
  [ "ROD_NONE" , $rod_easy_enum{"NN"}    ],
  [ "ROD_MAX"  , $rod_easy_enum{"NN"} +1 ],
);

##=============================================================================


##=============================================================================
=pod
    グローバル変数
=cut
##=============================================================================
# 引数
$dowsing_area_csv_file_name   = $ARGV[0];  # ????.csv  # shiftjis

# 定数
# 読み込みファイル
$temp_dowsing_area_csv_file_name    = "dowsing_area_make_pl_temp_dowsing_area.csv";   # utf8

# 出力ファイル
$dowsing_area_file_name                 = "dowsing_area.dat";    # utf8
$dowsing_rod_enum_header_file_name      = "dowsing_rod_enum.h";  # shiftjis 

# 横、縦のサイズ
$dowsing_area_width  = 32;
$dowsing_area_height = 24;

# 原点
$dowsing_area_origin_x   = 15;
$dowsing_area_origin_y   = 11;

# 変数
@dowsing_area_tbl = ();


##=============================================================================
=pod
    メイン処理
=cut
##=============================================================================
# UTF8だけで作業できるようにする
&EncodeFileFromShiftjisToUtf8( $dowsing_area_csv_file_name, $temp_dowsing_area_csv_file_name );
$dowsing_area_csv_file_name = $temp_dowsing_area_csv_file_name;  # これ以降に使用するファイルを差し替えておく

# ファイルを読み込み、アイテムの位置とロッドの状態の対応テーブルを完成させる
&ReadFileAndMakeTable();

# ファイルを出力する
&WriteDataFile();
&WriteHeaderFile();

# 不要なファイルを削除する
&DeleteTemp();

# 終了
exit;

##=============================================================================
=pod
    サブルーチン
=cut
##=============================================================================
##-------------------------------------
### ShiftJISファイルからUTF8ファイルを作成する
##=====================================
sub EncodeFileFromShiftjisToUtf8
{
  my $in_file   = $_[0];  # shiftjis
  my $out_file  = $_[1];  # utf8

  open( IN, "<", $in_file );
  open( OUT, ">", $out_file );

  while( <IN> )
  {
    $line = Encode::decode( 'shiftjis', $_ );
    print OUT Encode::encode( 'utf8', $line );
  }

  close( OUT );
  close( IN );
}

##-------------------------------------
### 一時的に作成したものを削除する
##=====================================
sub DeleteTemp
{
  unlink( $temp_dowsing_area_csv_file_name );
}

##-------------------------------------
### ファイルを読み込み、アイテムの位置とロッドの状態の対応テーブルを完成させる
##=====================================
sub ReadFileAndMakeTable
{
  open( DATA, "<:encoding(utf8)",  $dowsing_area_csv_file_name );
 
  my $row = 0;  # height  # 行数


  # 値に改行コードを含む CSV形式を扱う

  while (my $line = <DATA>) {
    $line .= <DATA> while ($line =~ tr/"// % 2 and !eof(DATA));

    $line =~ s/(?:\x0D\x0A|[\x0D\x0A])?$/,/;
    @values = map {/^"(.*)"$/s ? scalar($_ = $1, s/""/"/g, $_) : $_}
                  ($line =~ /("[^"]*(?:""[^"]*)*"|[^,]*),/g);

    # @values を処理する
   

    my $col = 0;  # width   # 列数
    for( my $col=0; $col<$dowsing_area_width; $col++ )  # width   # 列数
    {
      my $rod_val;
      if( exists( $rod_easy_enum{ $values[$col] } ) )
      {
        $rod_val = $rod_easy_enum{ $values[$col] };
      }
      else
      {
        $rod_val = $rod_easy_enum_unknown;

        # %rod_easy_enumに列挙されていないもの
        die "rod_easy_enum \"$rod_easy_enum{ $values[$col] }\" error, stopped";
      }
      $dowsing_area_tbl[$row * $dowsing_area_width + $col] = $rod_val;
    }
    $row++;


  }


  close( DATA );
}

##-------------------------------------
### アイテムの位置とロッドの状態の対応テーブルのバイナリファイルを出力する
##=====================================
sub WriteDataFile
{
  open( FH, ">", $dowsing_area_file_name );
  binmode FH;

  for( my $row=0; $row<$dowsing_area_height; $row++ )  # height   # 行数
  {
    for( my $col=0; $col<$dowsing_area_width; $col++ )  # width   # 列数
    {
      my $buf = pack "C", $dowsing_area_tbl[$row * $dowsing_area_width + $col];  # 符号なし8ビット整数。(u8だからリトルエンディアンとかない)
      print FH "$buf";
    }
  }

  close( FH );
}

##-------------------------------------
### ロッドの状態のenum値を記したヘッダーファイルを出力する
##=====================================
sub WriteHeaderFile
{
  open( FH, ">:encoding(shiftjis)", $dowsing_rod_enum_header_file_name );

  # 説明
  printf FH "//============================================================================\r\n";
  printf FH "/**\r\n";
  printf FH " *  \@file   %s\r\n", $dowsing_rod_enum_header_file_name;
  printf FH " *  \@brief  ロッドの状態のenum値を記したヘッダーファイル\r\n";
  printf FH " *  \@author dowsing_area_make.pl\r\n";
  printf FH " *  \@date   \r\n";
  printf FH " *  \@note   dowsing_area_make.plで生成されました。\r\n";
  printf FH " *\r\n";
  printf FH " *  モジュール名：\r\n";
  printf FH " */\r\n";
  printf FH "//============================================================================\r\n";
  printf FH "#pragma once\r\n";
  printf FH "\r\n";  # 0D 0A

  # enum
  # 前
  printf FH "typedef enum\r\n";
  printf FH "{\r\n";
  
  # 値
  for( my $i=0; $i<@rod_enum; $i++ )
  {
    printf FH "  %s    =  %d,\r\n", $rod_enum[$i][0], $rod_enum[$i][1];  # 0D 0A
  }

  # 後
  printf FH "}\r\n";
  printf FH "ROD;\r\n";
  printf FH "\r\n";
  
  # define
  printf FH "#define AREA_WIDTH    (%d)\r\n", $dowsing_area_width;  # 0D 0A
  printf FH "#define AREA_HEIGHT    (%d)\r\n", $dowsing_area_height;  # 0D 0A
  printf FH "#define AREA_ORIGIN_X    (%d)\r\n", $dowsing_area_origin_x;  # 0D 0A
  printf FH "#define AREA_ORIGIN_Y    (%d)\r\n", $dowsing_area_origin_y;  # 0D 0A

  close( FH );
}

