##============================================================================
=pod
     file   zkn_hw_make.pl
     brief  MONSNO, FORMNOごとの高さgmmと重さgmmを作成する
     author Koji Kawada
     data   2010.01.05
     note   perl zkn_hw_make.pl personal_wb.csv

     note   resource/personal/personal_wb.csv(ShiftJIS)
     note   参考  tools/personal_conv/personal_conv.rb(ShiftJIS)

    モジュール名：
=cut 
##============================================================================
# このソース自体をUTF8で記述し、UTF8で処理を行う
use utf8;
binmode STDIN,  ":encoding(shiftjis)";
binmode STDOUT, ":encoding(shiftjis)";
binmode STDERR, ":encoding(shiftjis)";

# インクルード
use Encode;

##=============================================================================
=pod
    グローバル変数
=cut
##=============================================================================
# 引数
$personal_csv_file_name   = $ARGV[0];  # ????.csv  # shiftjis

# 定数
# 読み込みファイル
# personal
#$personal_csv_row_data_start           =   0;  # 最初から最後まで全て読み込むようにし、開始終了の指定はしないことにする。
#$personal_csv_row_data_end             = 101;  # personal_csv_row_data_start<=データ<personal_csv_row_data_end
$personal_csv_col_pokename             =   0;  # ポケモン名。例：フシギダネ
$personal_csv_col_form_name            =  99;  # フォルム名。例：una
$personal_csv_col_height               = 105;  # csvにあるデータは10倍して整数にしてあるので、表示するときは1/10倍[m]。
$personal_csv_col_weight               = 106;  # csvにあるデータは10倍して整数にしてあるので、表示するときは1/10倍[kg]。

$temp_personal_csv_file_name    = "zkn_hw_make_pl_temp_personal.csv";   # utf8

# 値なし
$no_value = "???.?";

# gmm
$gmm_header_file_name     = "header.file";  # utf8
$gmm_footer_file_name     = "footer.file";  # utf8

# 出力ファイル
# gmm
$height_gmm_file_name      = "zkn_height.gmm";  # utf8
$weight_gmm_file_name      = "zkn_weight.gmm";  # utf8

# デバッグ
$debug_monsno_formno_csv_file_name    = "debug_zkn_hw_make_monsno_formno.csv";  # shiftjis
$debug_mons_form_order_csv_file_name  = "debug_zkn_hw_make_mons_order.csv";     # shiftjis

# 変数
# mons
$mons_num;
@mons_tbl = ();
$mons_col_monsname       = 0;
$mons_col_monsno         = 1;  # 1スタート。001がフシギダネ。
$mons_col_form_num       = 2;
$mons_col_form_start     = 3;  # 最後(含まない)は($mons_col_form_start + $form_col_offset_max * $mons_col_form_numの値)

$form_col_offset_form_name   = 0;
$form_col_offset_formno      = 1;  # 0スタート。($mons_col_form_numの値 - 1)が最後(含む)。
$form_col_offset_height      = 2;
$form_col_offset_weight      = 3;
$form_col_offset_max         = 4;

# 0<=$formno<$mons_col_form_numとすると
# $mons_tbl[ $mons_col_form_start + $form_col_offset_max * $formno + $form_col_offset_form_name ] = $formnoのフォルム名;
# $mons_tbl[ $mons_col_form_start + $form_col_offset_max * $formno + $form_col_offset_formno    ] = $formnoそのもの;
# $mons_tbl[ $mons_col_form_start + $form_col_offset_max * $formno + $form_col_offset_height    ] = $formnoの高さ(10倍整数のまま);
# $mons_tbl[ $mons_col_form_start + $form_col_offset_max * $formno + $form_col_offset_weight    ] = $formnoの重さ(10倍整数のまま);
# となる

%mons_tbl_idx_hash = ();  # ポケモン名からmons_tblのインデックスを得るためのハッシュ

##=============================================================================
=pod
    メイン処理
=cut
##=============================================================================
# UTF8だけで作業できるようにする
&EncodeFileFromShiftjisToUtf8( $personal_csv_file_name, $temp_personal_csv_file_name );
$personal_csv_file_name = $temp_personal_csv_file_name;  # これ以降に使用するファイルを差し替えておく

# personalファイルを読み込み、ハッシュを作成する
&ReadPersonalFile();
&DebugWriteMonsnoFormnoFile();  #debug

# MONSNO, FORMNOごとの高さgmmと重さgmmを出力する
&DebugWriteMonsFormOrderFile();  #debug
&WriteHWFile(0);  # height
&WriteHWFile(1);  # weight

# 不要なファイルを削除する
&DeleteTemp();
&DebugDeleteFile();  #debug

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
  unlink( $temp_personal_csv_file_name );
}

##-------------------------------------
### デバッグ用に作成したものを削除する
##=====================================
sub DebugDeleteFile
{
  unlink( $debug_monsno_formno_csv_file_name );
  unlink( $debug_mons_form_order_csv_file_name );
}

##-------------------------------------
### personalファイルを読み込む
##=====================================
sub ReadPersonalFile
{
  open( DATA, "<:encoding(utf8)",  $personal_csv_file_name );
 
  $mons_num = 0;
  
  my $mons_or_form = 0;  # ポケモンの0番目のフォルムを処理中か、1番目以降のフォルムを処理中か。  # 0=mons; 1=form;


  # 値に改行コードを含む CSV形式を扱う

  while (my $line = <DATA>) {
    $line .= <DATA> while ($line =~ tr/"// % 2 and !eof(DATA));

    $line =~ s/(?:\x0D\x0A|[\x0D\x0A])?$/,/;
    @values = map {/^"(.*)"$/s ? scalar($_ = $1, s/""/"/g, $_) : $_}
                  ($line =~ /("[^"]*(?:""[^"]*)*"|[^,]*),/g);

    # @values を処理する
   

    my $value_num = @values;
    my $value_count = 0;
    for( my $i=0; $i<$value_num; $i++ )  # tools/personal_conv/personal_conv.rbによると、pokenameしかない行はゴミレコード
    {
      if( $values[$i] ne "" )
      {
        $value_count++;
      }
    }

    if( $value_count <= 1 )
    {
      #サーバからのエクスポートでゴミレコードが入ることがあるので、排除する
    }
    else
    {
      if( $mons_or_form == 0 )  # ポケモンの0番目のフォルムを処理中
      {
        if( $values[$personal_csv_col_pokename] eq "" )
        {
          $mons_or_form = 1;  # 次から別フォルム
        }
        else
        {
          my $monsname = $values[$personal_csv_col_pokename];
          my $idx = $mons_num;
          $mons_tbl_idx_hash{ $monsname } = $idx;  # ポケモン名からmons_tblのインデックスを得るためのハッシュ

          $mons_tbl[$idx][$mons_col_monsname] = $monsname;
          $mons_tbl[$idx][$mons_col_monsno  ] = $idx +1;
          $mons_tbl[$idx][$mons_col_form_num] = 0;

          my $formno = $mons_tbl[$idx][$mons_col_form_num];
          my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;
          $mons_tbl[$idx][$form_col_start + $form_col_offset_form_name] = $values[$personal_csv_col_form_name];
          $mons_tbl[$idx][$form_col_start + $form_col_offset_formno   ] = $formno;
          $mons_tbl[$idx][$form_col_start + $form_col_offset_height   ] = $values[$personal_csv_col_height];
          $mons_tbl[$idx][$form_col_start + $form_col_offset_weight   ] = $values[$personal_csv_col_weight];
          $mons_tbl[$idx][$mons_col_form_num] += 1;
          $mons_num++;
        }
      }
      else  # if( $mons_or_form == 1 )  # 1番目以降のフォルムを処理中
      {
        my $monsname = $values[$personal_csv_col_pokename];
        my $idx = $mons_tbl_idx_hash{ $monsname };
        if( !defined($idx) )
        {
          # 存在しないポケモン名
          die "pokename \"$values[$personal_csv_col_pokename]\" error, stopped";
        }

        my $formno = $mons_tbl[$idx][$mons_col_form_num];
        my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;
        $mons_tbl[$idx][$form_col_start + $form_col_offset_form_name] = $values[$personal_csv_col_form_name];
        $mons_tbl[$idx][$form_col_start + $form_col_offset_formno   ] = $formno;
        $mons_tbl[$idx][$form_col_start + $form_col_offset_height   ] = $values[$personal_csv_col_height];
        $mons_tbl[$idx][$form_col_start + $form_col_offset_weight   ] = $values[$personal_csv_col_weight];
        $mons_tbl[$idx][$mons_col_form_num] += 1;
      }
    }


  }


  close( DATA );
}

##-------------------------------------
### mons_tblの中身を確認する
##=====================================
sub DebugWriteMonsnoFormnoFile
{
  open( FH, ">:encoding(shiftjis)", $debug_monsno_formno_csv_file_name );

  print FH "monsname,monsno,form_num,form_name,formno,height,weight,form_name,formno,height,weight,\r\n";  # 0D 0A

  for( my $idx=0; $idx<$mons_num; $idx++ )
  {
    print FH "$mons_tbl[$idx][$mons_col_monsname],";
    print FH "$mons_tbl[$idx][$mons_col_monsno  ],";
    print FH "$mons_tbl[$idx][$mons_col_form_num],";

    my $form_num = $mons_tbl[$idx][$mons_col_form_num];
    for( my $formno=0; $formno<$form_num; $formno++ )
    {
      my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;
      print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_form_name],";
      print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_formno   ],";
      print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_height   ],";
      print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_weight   ],";
    }

    print FH "\r\n";  # 0D 0A
  }

  close( FH );
}

##-------------------------------------
### 出力の順番を確認するためのファイル
##=====================================
sub DebugWriteMonsFormOrderFile
{
  open( FH, ">:encoding(shiftjis)", $debug_mons_form_order_csv_file_name );

  my $curr_raw = 0;
  my $mons_end_raw = $mons_num +2;  # 0 <= タマゴも加えたmons < $mons_end_raw
  my $monsno_tamago = $mons_end_raw -1;
  #              000    ーーーーー
  #              001    フシギダネ
  # $mons_end_raw -2    オニソコ(最後のポケモン)
  # $mons_end_raw -1    タマゴ(ダメタマゴは加えなかった)
  my $other_form_total_num = 0;  # 1個しかフォルムがないときは、他のフォルムは0個となる。

  # monsno=0番目のーーーーー
  # $curr_raw = 0
  print FH "$curr_raw,";
  print FH "ーーーーー,monsno,フォルム数,フォルム名,formno,次のフォルムの場所,";
  print FH "\r\n";  # 0D 0A
  # 「次のフォルムの場所」が0のとき、次のフォルムはない
  # 「次のフォルムの場所」が0以外のとき、その行に次にフォルムが書かれている(行は先頭0行からの行数)
  $curr_raw++;

  # まず全ポケモンのフォルム0番だけを出力する
  # 1 <= $curr_raw < 1+ $mons_num
  for( my $idx=0; $idx<$mons_num; $idx++ )
  {
    print FH "$curr_raw,";
    print FH "$mons_tbl[$idx][$mons_col_monsname],";
    print FH "$mons_tbl[$idx][$mons_col_monsno  ],";
    print FH "$mons_tbl[$idx][$mons_col_form_num],";

    my $formno = 0;
    my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;
    print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_form_name],";
    print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_formno   ],";

    my $next_form_raw = 0;  # 次のフォルムはない
    my $form_num = $mons_tbl[$idx][$mons_col_form_num];
    if( $form_num > $formno +1 )
    {
      $next_form_raw = $mons_end_raw + $other_form_total_num;  # その行に次にフォルムが書かれている(行は先頭0行からの行数)
      $other_form_total_num += $form_num;
    }
    print FH "$next_form_raw,";

    print FH "\r\n";  # 0D 0A
    
    $curr_raw++;
  }

  # タマゴ
  # $curr_raw = $monsno_tamago;
  print FH "$curr_raw,";
  print FH "タマゴ,$monsno_tamago,1,フォルム名,0,次のフォルムの場所,";
  print FH "\r\n";  # 0D 0A
  $curr_raw++;
 
  # 他のフォルムがあるポケモンについてだけ、そのフォルムをポケモンごとにまとめて出力する
  # $mons_end_raw <= $curr_raw
  for( my $idx=0; $idx<$mons_num; $idx++ )
  {
    my $form_num = $mons_tbl[$idx][$mons_col_form_num];
    if( $form_num > 1 )
    {
      for( my $formno=1; $formno<$form_num; $formno++ )
      {
        print FH "$curr_raw,";
        print FH "$mons_tbl[$idx][$mons_col_monsname],";
        print FH "$mons_tbl[$idx][$mons_col_monsno  ],";
        print FH "$mons_tbl[$idx][$mons_col_form_num],";

        my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;
        print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_form_name],";
        print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_formno   ],";
 
        my $next_form_raw = 0;  # 次のフォルムはない
        if( $form_num > $formno +1 )
        {
          $next_form_raw = $curr_raw +1;  # その行に次にフォルムが書かれている(行は先頭0行からの行数)
        }
        print FH "$next_form_raw,";

        print FH "\r\n";  # 0D 0A
    
        $curr_raw++;
      }
    }
  }

  close( FH );
}

##-------------------------------------
### MONSNO, FORMNOごとの高さgmm(重さgmm)を出力する
##=====================================
sub WriteHWFile
{
  # DebugWriteMonsFormOrderFileと同じ順番になるように、
  # DebugWriteMonsFormOrderFileと同じ構成にしてある。(DebugWriteMonsFormOrderFileを先につくった)

  # 0=height
  # 1=weight
  my $hw = $_[0];
  my $hw_gmm_file_name;
  my $hw_row_id;
  my $hw_unit;
  my $hw_form_col_offset;
  my $text;

  if( $hw == 0 )  # height
  {
    $hw_gmm_file_name = $height_gmm_file_name;
    $hw_row_id = "ZKN_HEIGHT_";
    $hw_unit = "m";
    $hw_form_col_offset = $form_col_offset_height;
  }
  else  # if( $hw == 1 )  # weight
  {
    $hw_gmm_file_name = $weight_gmm_file_name;
    $hw_row_id = "ZKN_WEIGHT_";
    $hw_unit = "kg";
    $hw_form_col_offset = $form_col_offset_weight;
  }

  open( FH, ">:encoding(utf8)", $hw_gmm_file_name );

  # header
  open( HEADER, "<:encoding(utf8)", $gmm_header_file_name );
  while( <HEADER> )
  {
    print FH $_;
  }
  close( HEADER );

  # 本体
  my $curr_raw = 0;
  my $mons_end_raw = $mons_num +2;  # 0 <= タマゴも加えたmons < $mons_end_raw
  my $monsno_tamago = $mons_end_raw -1;
  #              000    ーーーーー
  #              001    フシギダネ
  # $mons_end_raw -2    オニソコ(最後のポケモン)
  # $mons_end_raw -1    タマゴ(ダメタマゴは加えなかった)
  my $other_form_total_num = 0;  # 1個しかフォルムがないときは、他のフォルムは0個となる。

  # monsno=0番目のーーーーー
  # $curr_raw = 0
  printf FH "\t<row id=\"%s%03d_%03d\">\r\n", $hw_row_id, 0, 0;  # 0D 0A
  printf FH "\t\t<language name=\"JPN\">";
  $text = sprintf( "%s%s", $no_value, $hw_unit );
  $text =~ tr/0,1,2,3,4,5,6,7,8,9,\.,m,k,g,\?/０,１,２,３,４,５,６,７,８,９,．,ｍ,ｋ,ｇ,？/;  # 半角を全角にする
  printf FH "$text";
  printf FH "</language>\r\n";
  printf FH "\t\t<language name=\"JPN_KANJI\">";
  printf FH "$text";
  printf FH "</language>\r\n";
  printf FH "\t</row>\r\n";
  printf FH "\r\n";
  # 「次のフォルムの場所」が0のとき、次のフォルムはない
  # 「次のフォルムの場所」が0以外のとき、その行に次にフォルムが書かれている(行は先頭0行からの行数)
  $curr_raw++;

  # まず全ポケモンのフォルム0番だけを出力する
  # 1 <= $curr_raw < 1+ $mons_num
  for( my $idx=0; $idx<$mons_num; $idx++ )
  {
    my $formno = 0;
    my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;

    printf FH "\t<row id=\"%s%03d_%03d\">\r\n", $hw_row_id, $mons_tbl[$idx][$mons_col_monsno], $mons_tbl[$idx][$form_col_start + $form_col_offset_formno   ];
    printf FH "\t\t<language name=\"JPN\">";
    $text = sprintf( "%.1f%s", $mons_tbl[$idx][$form_col_start + $hw_form_col_offset] /10, $hw_unit );
    $text =~ tr/0,1,2,3,4,5,6,7,8,9,\.,m,k,g,\?/０,１,２,３,４,５,６,７,８,９,．,ｍ,ｋ,ｇ,？/;  # 半角を全角にする
    printf FH "$text";
    printf FH "</language>\r\n";
    printf FH "\t\t<language name=\"JPN_KANJI\">";
    printf FH "$text";
    printf FH "</language>\r\n";
    printf FH "\t</row>\r\n";
    printf FH "\r\n";
      
    $curr_raw++;
  }

  # タマゴ
  # $curr_raw = $monsno_tamago;
  printf FH "\t<row id=\"%s%03d_%03d\">\r\n", $hw_row_id, $monsno_tamago, 0;  # 0D 0A
  printf FH "\t\t<language name=\"JPN\">";
  $text = sprintf( "%s%s", $no_value, $hw_unit );
  $text =~ tr/0,1,2,3,4,5,6,7,8,9,\.,m,k,g,\?/０,１,２,３,４,５,６,７,８,９,．,ｍ,ｋ,ｇ,？/;  # 半角を全角にする
  printf FH "$text";
  printf FH "</language>\r\n";
  printf FH "\t\t<language name=\"JPN_KANJI\">";
  printf FH "$text";
  printf FH "</language>\r\n";
  printf FH "\t</row>\r\n";
  printf FH "\r\n";
  $curr_raw++;
 
  # 他のフォルムがあるポケモンについてだけ、そのフォルムをポケモンごとにまとめて出力する
  # $mons_end_raw <= $curr_raw
  for( my $idx=0; $idx<$mons_num; $idx++ )
  {
    my $form_num = $mons_tbl[$idx][$mons_col_form_num];
    if( $form_num > 1 )
    {
      for( my $formno=1; $formno<$form_num; $formno++ )
      {
        my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;
       
        printf FH "\t<row id=\"%s%03d_%03d\">\r\n", $hw_row_id, $mons_tbl[$idx][$mons_col_monsno], $mons_tbl[$idx][$form_col_start + $form_col_offset_formno   ];
        printf FH "\t\t<language name=\"JPN\">";
        $text = sprintf( "%.1f%s", $mons_tbl[$idx][$form_col_start + $hw_form_col_offset] /10, $hw_unit );
        $text =~ tr/0,1,2,3,4,5,6,7,8,9,\.,m,k,g,\?/０,１,２,３,４,５,６,７,８,９,．,ｍ,ｋ,ｇ,？/;  # 半角を全角にする
        printf FH "$text";
        printf FH "</language>\r\n";
        printf FH "\t\t<language name=\"JPN_KANJI\">";
        printf FH "$text";
        printf FH "</language>\r\n";
        printf FH "\t</row>\r\n";
        printf FH "\r\n";

        $curr_raw++;
      }
    }
  }

  # footer
  open( FOOTER, "<:encoding(utf8)", $gmm_footer_file_name );
  while( <FOOTER> )
  {
    print FH $_;
  }
  close( FOOTER );

  close( FH );
}

