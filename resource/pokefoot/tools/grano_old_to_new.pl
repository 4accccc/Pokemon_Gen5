##============================================================================
=pod
     file   grano_old_to_new.pl
     brief  ポケモンの前世代のグラフィックNoを今世代のグラフィックNoに変更するための対応テーブルを生成する
     author Koji Kawada
     data   2010.05.07
     note   perl grano_old_to_new.pl pmpl_personal.csv personal_wb.csv

     note   pmpl_personal.csvはダイヤモンドパールのパーソナル(ShiftJIS)
     note   シャチのパーソナルresource/personal/personal_wb.csv(ShiftJIS)
     note   出力ファイルには数字しか書かれていないので、ShiftJISでもUTF8でも同じ。
     note   参考  tools/personal_conv/personal_conv.rb(ShiftJIS)
     note   モンスターNoとは全国図鑑Noのことである。
     note   グラフィックNoとは今世代の新規ポケモン開発中に変わることがないデザイナ番号のことである。

     モジュール名：
=cut 
##============================================================================


##=============================================================================
=pod

出力ファイルのフォーマット
493,648
000,000
001,001
...
432,387
433,388
...

ダイヤモンドパールの000を除いた個数(つまり000も含めた全個数は494),シャチの000を除いた個数(つまり000も含めた全個数は649)
ダイヤモンドパールのグラフィックNo,シャチのグラフィックNo
ダイヤモンドパールのグラフィックNo,シャチのグラフィックNo
...
ダイヤモンドパールのグラフィックNo,シャチのグラフィックNo
ダイヤモンドパールのグラフィックNo,シャチのグラフィックNo
...

ダイヤモンドパールのパーソナルにおいて、ルビーサファイアまでに登場したポケモンは、グラフィックNoとモンスターNoが一致している。
シャチのパーソナルにおいて、ダイヤモンドパールまでに登場したポケモンは、グラフィックNoとモンスターNoが一致している。
上の例のシャチのグラフィックNo387はナエトル、シャチのグラフィックNo388はハヤシガメ。
前世代のほうがデータが少ないので、足りないところはコピーしたい前世代のグラフィックNoを書いておく。
→訂正：前世代のファイルだとncgが扱い難いので、今世代用に用意した雛形ファイルのグラフィックNoを書いておく。

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
=pod
    グローバル変数
=cut
##=============================================================================
# 引数
$old_personal_csv_file_name        = $ARGV[0];  # ????.csv  # shiftjis
$new_personal_csv_file_name        = $ARGV[1];  # ????.csv  # shiftjis

# 定数
# 読み込みファイル
# old_personal
$old_personal_csv_col_pokename     =   2;  # ポケモン名。例：ナエトル
$old_personal_csv_col_grano        =  87;  # グラフィックNo。例：ナエトルが432番

# new_personal
$new_personal_csv_col_pokename     =   0;  # ポケモン名。例：ナエトル
$new_personal_csv_col_monsno       =  39;  # モンスターNo。例：ナエトルが387番。例：ツタージャがコロコロ変わる
$new_personal_csv_col_grano        =  98;  # グラフィックNo。例：ナエトルが387番。例：ツタージャが551番

# utf8に変換したtempファイル
$temp_old_personal_csv_file_name   = "grano_old_to_new_pl_temp_old_personal.csv";   # utf8
$temp_new_personal_csv_file_name   = "grano_old_to_new_pl_temp_new_personal.csv";   # utf8

# 出力ファイル
$grano_old_to_new_file_name         = "grano_old_to_new.csv";    # 出力ファイルには数字しか書かれていないので、ShiftJISでもUTF8でも同じ。

# old_personalのデータの枕、終わりなど
$old_personal_grano_head           = "PMDP_";    # PMDP_432
$old_personal_grano_no_start       = 5;          # 0スタートで5番目から数字
$old_personal_grano_no_num         = 3;          # 数字は3文字
$old_personal_csv_end_pokename     = "タマゴ";   # 終わりを示すポケモン名
#$old_copy_grano                    = 201;        # データの足りないところには「前世代のグラフィックNoで201番(アンノーン)」を書いておく。
$old_copy_grano                    = 600;        # →訂正：前世代のファイルだとncgが扱い難いので、今世代用に用意した雛形ファイルのグラフィックNoを書いておく。

# new_personalのデータの枕、終わりなど
$new_personal_csv_end_pokename     = "";         # 終わりを示すポケモン名

# 変数
# old_mons
$old_mons_num;  # 000を除いた個数
@old_mons_tbl = ();  # 0<= <$old_mons_num。000のデータはここには含まれていない。
$old_mons_col_pokename   = 0;  # 例：ナエトル
$old_mons_col_grano      = 1;  # 1スタート。432がナエトル
%old_mons_tbl_idx_hash = ();  # ポケモン名からold_mons_tblのインデックスを得るためのハッシュ

# new_mons
$new_mons_num;  # 000を除いた個数
@new_mons_tbl = ();  # 0<= <$new_mons_num。000のデータはここには含まれていない。
$new_mons_col_pokename   = 0;  # 例：ナエトル
$new_mons_col_monsno     = 1;  # 1スタート。387がナエトル
$new_mons_col_grano      = 2;  # 1スタート。387がナエトル
%new_mons_tbl_idx_hash = ();  # ポケモン名からnew_mons_tblのインデックスを得るためのハッシュ


##=============================================================================
=pod
    メイン処理
=cut
##=============================================================================
# UTF8だけで作業できるようにする
&EncodeFileFromShiftjisToUtf8( $old_personal_csv_file_name, $temp_old_personal_csv_file_name );
$old_personal_csv_file_name = $temp_old_personal_csv_file_name;  # これ以降に使用するファイルを差し替えておく

&EncodeFileFromShiftjisToUtf8( $new_personal_csv_file_name, $temp_new_personal_csv_file_name );
$new_personal_csv_file_name = $temp_new_personal_csv_file_name;  # これ以降に使用するファイルを差し替えておく

# personalファイルを読み込む
&ReadOldPersonalFile();
&ReadNewPersonalFile();

# 対応テーブルを出力する
&WriteGranoToMonsnoFile();

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
  unlink( $temp_old_personal_csv_file_name );
  unlink( $temp_new_personal_csv_file_name );
}


##-------------------------------------
### personalファイルを読み込む
##=====================================
sub ReadOldPersonalFile
{
  open( DATA, "<:encoding(utf8)",  $old_personal_csv_file_name );
 
  $old_mons_num = 0;
  

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
      if( $values[$old_personal_csv_col_pokename] eq $old_personal_csv_end_pokename )
      {
        # 終わりを示すポケモン名
        last;
      }
      else
      {
        my $pokename        = $values[$old_personal_csv_col_pokename];

        if( defined( $old_mons_tbl_idx_hash{ $pokename } ) )
        {
          # 同じポケモン名が2度出現した
          die "pokename \"$values[$old_personal_csv_col_pokename]\" error, stopped";
        } 

        my $grano_with_head = $values[$old_personal_csv_col_grano];
        my $grano           = substr( $grano_with_head, $old_personal_grano_no_start, $old_personal_grano_no_num );
        my $idx = $old_mons_num;

        $old_mons_tbl_idx_hash{ $pokename } = $idx;  # ポケモン名からold_mons_tblのインデックスを得るためのハッシュ
        
        $old_mons_tbl[$idx][$old_mons_col_pokename] = $pokename;
        $old_mons_tbl[$idx][$old_mons_col_grano   ] = $grano;

        $old_mons_num++;
      }
    }


  }


  close( DATA );
}

sub ReadNewPersonalFile
{
  open( DATA, "<:encoding(utf8)",  $new_personal_csv_file_name );
 
  $new_mons_num = 0;
  

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
      if( $values[$new_personal_csv_col_pokename] eq $new_personal_csv_end_pokename )
      {
        # 終わりを示すポケモン名
        last;
      }
      else
      {
        my $pokename        = $values[$new_personal_csv_col_pokename];

        if( defined( $new_mons_tbl_idx_hash{ $pokename } ) )
        {
          # 同じポケモン名が2度出現した
          die "pokename \"$values[$new_personal_csv_col_pokename]\" error, stopped";
        } 

        my $monsno          = $values[$new_personal_csv_col_monsno];
        my $grano           = $values[$new_personal_csv_col_grano];
        my $idx = $new_mons_num;

        $new_mons_tbl_idx_hash{ $pokename } = $idx;  # ポケモン名からnew_mons_tblのインデックスを得るためのハッシュ
        
        $new_mons_tbl[$idx][$new_mons_col_pokename] = $pokename;
        $new_mons_tbl[$idx][$new_mons_col_monsno  ] = $monsno;
        $new_mons_tbl[$idx][$new_mons_col_grano   ] = $grano;

        $new_mons_num++;
      }
    }


  }


  close( DATA );
}


##-------------------------------------
### 対応テーブルを出力する
##=====================================
sub WriteGranoToMonsnoFile
{
#=pod
  printf "old\r\n";
  for( my $i=0; $i<$old_mons_num; $i++ )
  {
    printf "%s,%d\r\n", $old_mons_tbl[$i][$old_mons_col_pokename], $old_mons_tbl[$i][$old_mons_col_grano];
  }
  printf "new\r\n";
  for( my $i=0; $i<$new_mons_num; $i++ )
  {
    printf "%s,%d,%d\r\n", $new_mons_tbl[$i][$new_mons_col_pokename], $new_mons_tbl[$i][$new_mons_col_monsno], $new_mons_tbl[$i][$new_mons_col_grano];
  }
#=cut


  open( FH, ">:encoding(utf8)", $grano_old_to_new_file_name );

  # 個数
  printf FH "%d,%d\r\n", $old_mons_num, $new_mons_num;  # 0D 0A
  
  # 000を先頭に追加 
  printf FH "0,0\r\n";  # 0D 0A

  # 各データ
  # oldにもnewにもあるデータ 
  for( my $i=0; $i<$old_mons_num; $i++ )
  {
    my $old_grano = $old_mons_tbl[$i][$old_mons_col_grano];
    my $pokename  = $old_mons_tbl[$i][$old_mons_col_pokename];

    if( !defined( $new_mons_tbl_idx_hash{ $pokename } ) )
    {
      # oldに存在するポケモン名がnewに存在しない
      die "pokename \"$pokename\" error, stopped";
    }
    
    my $new_idx    = $new_mons_tbl_idx_hash{ $pokename };
    my $new_monsno = $new_mons_tbl[$new_idx][$new_mons_col_monsno];
    my $new_grano  = $new_mons_tbl[$new_idx][$new_mons_col_grano];
  
    printf FH "%d,%d\r\n", $old_grano,$new_grano;  # 0D 0A
  }
  # newにしかないデータ 
  for( my $i=$old_mons_num; $i<$new_mons_num; $i++ )
  {
    my $old_grano  = $old_copy_grano;
    my $new_idx    = $i;
    my $new_monsno = $new_mons_tbl[$new_idx][$new_mons_col_monsno];
    my $new_grano  = $new_mons_tbl[$new_idx][$new_mons_col_grano];

    printf FH "%d,%d\r\n", $old_grano,$new_grano;  # 0D 0A
  }

  close( FH );
}

