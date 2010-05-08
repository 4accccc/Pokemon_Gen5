##============================================================================
=pod
     file   rename_nitro_file.pl
     brief  NitroCharacterのファイル名を変更する。ファイル内にも変更を加える。
     author Koji Kawada
     data   2010.05.07
     note   perl rename_nitro_file.pl

     note   ダイヤモンドパールのpoke_foot_000.nce内に書かれている番号を
            ダイヤモンドパールのグラフィックNoからシャチのグラフィックNoに変更する。
     note   ダイヤモンドパールのpoke_foot_000.nceのファイル名にある番号を
            ダイヤモンドパールのグラフィックNoからシャチのグラフィックNoに変更する。
     note   ダイヤモンドパールのpoke_foot_000.ncgのファイル名にある番号を
            ダイヤモンドパールのグラフィックNoからシャチのグラフィックNoに変更する。
     note   grano_old_to_new.csvはgrano_old_to_new.plで作成したもの。

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
use File::Copy;


##=============================================================================
=pod
    グローバル変数
=cut
##=============================================================================
# 引数
# なし

# 定数
# 読み込みファイル
# ダイヤモンドパールのグラフィックNoからシャチのグラフィックNoへの変換を書いたファイル
$grano_old_to_new_csv_file_name     = "grano_old_to_new.csv";  # 読み込みファイルには数字しか書かれていないので、ShiftJISでもUTF8でも同じ。

$grano_old_to_new_csv_row_mons_num      = 0;
$grano_old_to_new_csv_col_old_mons_num  = 0;
$grano_old_to_new_csv_col_new_mons_num  = 1;
$grano_old_to_new_csv_col_old_grano     = 0;
$grano_old_to_new_csv_col_new_grano     = 1;

# パス
$read_path                   = "../graphic/";      # この後にファイル名を続けるので/で終わっていること
$write_path                  = "../new_graphic/";  # この後にファイル名を続けるので/で終わっていること

# nce
$nce_head_file_name     = "poke_foot_";
$nce_tail_file_name     = ".nce";
$nce_inside_no_start    = 0x2A6;  # ファイル内に書かれている番号の開始位置
$nce_inside_no_num      = 3;      # ファイル内に書かれている番号のバイト数

# ncg
$ncg_head_file_name     = "poke_foot_";
$ncg_tail_file_name     = ".ncg";

# 変数
$mons_num;  # 000を含めた個数  # 個数の多い$grano_old_to_new_csv_col_new_mons_numを採用。
@mons_tbl = ();  # 0<= <$mons_num。000のデータはこの先頭に含まれている。
$mons_col_old_grano  = 0;  # 例：432（ナエトル）
$mons_col_new_grano  = 1;  # 387（ナエトル）


##=============================================================================
=pod
    メイン処理
=cut
##=============================================================================
# ダイヤモンドパールのグラフィックNoからシャチのグラフィックNoへの変換を書いたファイルを読み込む
&ReadGranoOldToNewCsvFile();

# NitroCharacterのファイル名を変更する。ファイル内にも変更を加える。
&RenameNitroFile();

# 終了
exit;


##=============================================================================
=pod
    サブルーチン
=cut
##=============================================================================
##-------------------------------------
### ダイヤモンドパールのグラフィックNoからシャチのグラフィックNoへの変換を書いたファイルを読み込む
##=====================================
sub ReadGranoOldToNewCsvFile
{
  open( DATA, "<:encoding(utf8)",  $grano_old_to_new_csv_file_name );

  my $line_num = 0;
  my $tbl_line = 0;


  # 値に改行コードを含む CSV形式を扱う

  while (my $line = <DATA>) {
    $line .= <DATA> while ($line =~ tr/"// % 2 and !eof(DATA));

    $line =~ s/(?:\x0D\x0A|[\x0D\x0A])?$/,/;
    @values = map {/^"(.*)"$/s ? scalar($_ = $1, s/""/"/g, $_) : $_}
                  ($line =~ /("[^"]*(?:""[^"]*)*"|[^,]*),/g);

    # @values を処理する


    if( $line_num == $grano_old_to_new_csv_row_mons_num )
    {
      $mons_num = $values[$grano_old_to_new_csv_col_new_mons_num] +1;  # 000を含めた個数  # 個数の多い$grano_old_to_new_csv_col_new_mons_numを採用。
    }
    else
    {
      $mons_tbl[$tbl_line][$mons_col_old_grano] = $values[$grano_old_to_new_csv_col_old_grano];
      $mons_tbl[$tbl_line][$mons_col_new_grano] = $values[$grano_old_to_new_csv_col_new_grano];

      $tbl_line++;
    }
    $line_num++;


  }


  close( DATA );
}

##-------------------------------------
### NitroCharacterのファイル名を変更する。ファイル内にも変更を加える。
##=====================================
sub RenameNitroFile
{
  for( my $i=0; $i<$mons_num; $i++ )
  {
    my $src_nce_file_name = sprintf( "%s%s%03d%s", $read_path, $nce_head_file_name, $mons_tbl[$i][$mons_col_old_grano], $nce_tail_file_name );
    my $dst_nce_file_name = sprintf( "%s%s%03d%s", $write_path, $nce_head_file_name, $mons_tbl[$i][$mons_col_new_grano], $nce_tail_file_name );
    copy $src_nce_file_name, $dst_nce_file_name;
    
    my $src_ncg_file_name = sprintf( "%s%s%03d%s", $read_path, $ncg_head_file_name, $mons_tbl[$i][$mons_col_old_grano], $ncg_tail_file_name );
    my $dst_ncg_file_name = sprintf( "%s%s%03d%s", $write_path, $ncg_head_file_name, $mons_tbl[$i][$mons_col_new_grano], $ncg_tail_file_name );
    copy $src_ncg_file_name, $dst_ncg_file_name;
    
    if( $mons_tbl[$i][$mons_col_old_grano] != $mons_tbl[$i][$mons_col_new_grano] )
    {
      open( FH, "+<", $dst_nce_file_name );
      binmode FH;

      seek FH, $nce_inside_no_start, 0;

      my $str_no = sprintf( "%s", $mons_tbl[$i][$mons_col_new_grano] );
      my $buf = pack "a3", $str_no;

      print FH "$buf";

      close( FH );
    }
    else
    {
      # 変更しなくていいので、コピーするだけでOK
    }
  }
}

