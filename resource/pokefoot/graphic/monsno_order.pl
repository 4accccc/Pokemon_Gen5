##============================================================================
=pod
     file   monsno_order.pl
     brief  名前にグラフィックNoがついたファイルを、モンスターNo順に並べて出力する
     author Koji Kawada
     data   2010.05.07
     note   perl monsno_order.pl personal_wb.csv

     note   シャチのパーソナルresource/personal/personal_wb.csv(ShiftJIS)
     note   出力ファイルには数字とアルファベットしか書かれていないので、ShiftJISでもUTF8でも同じ。
     note   参考  tools/personal_conv/personal_conv.rb(ShiftJIS)
     note   モンスターNoとは全国図鑑Noのことである。

     モジュール名：
=cut 
##============================================================================


##=============================================================================
=pod

出力ファイルのフォーマット
"poke_foot.NCLR"
"poke_foot_000.NANR"
"poke_foot_000.NCER"
"poke_foot_000.NCGR"
"poke_foot_001.NCGR"
"poke_foot_002.NCGR"
...

poke_foot_グラフィックNo.NCGRがモンスターNo順に並んでいる。

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
$personal_csv_file_name        = $ARGV[0];  # ????.csv  # shiftjis

# 定数
# 読み込みファイル
# personal
$personal_csv_col_pokename     =   0;  # ポケモン名。例：ナエトル
$personal_csv_col_monsno       =  39;  # モンスターNo。例：ナエトルが387番
$personal_csv_col_grano        =  98;  # グラフィックNo。例：ツタージャが551番

$personal_csv_end_pokename     = "";

# utf8に変換したtempファイル
$temp_personal_csv_file_name   = "monsno_order_pl_temp_personal.csv";   # utf8

# 出力ファイル
$monsno_order_file_name        = "pokefoot.scr";    # 出力ファイルには数字とアルファベットしか書かれていないので、ShiftJISでもUTF8でも同じ。

# 出力内容
$nclr_file_name         = "poke_foot.NCLR";
$nanr_file_name         = "poke_foot_000.NANR";
$ncer_file_name         = "poke_foot_000.NCER";
$nanr_file_name_new     = "poke_foot_600.NANR";
$ncer_file_name_new     = "poke_foot_600.NCER";
$ncgr_file_name_head    = "poke_foot_";
$ncgr_file_name_tail    = ".NCGR";

# 変数
# mons
$mons_num;  # 000を除いた個数
@mons_tbl = ();  # 0<= <$mons_num。000のデータはここには含まれていない。  # mons_tblのインデックスとmonsnoは同じ順番なので、mons_tblの並びをそのまま利用。
$mons_col_pokename   = 0;  # 例：ナエトル
$mons_col_monsno     = 1;  # 1スタート。387がナエトル
$mons_col_grano      = 2;  # 1スタート。387がナエトル
%mons_tbl_idx_hash = ();  # ポケモン名からmons_tblのインデックスを得るためのハッシュ


##=============================================================================
=pod
    メイン処理
=cut
##=============================================================================
# UTF8だけで作業できるようにする
&EncodeFileFromShiftjisToUtf8( $personal_csv_file_name, $temp_personal_csv_file_name );
$personal_csv_file_name = $temp_personal_csv_file_name;  # これ以降に使用するファイルを差し替えておく

# personalファイルを読み込む
&ReadPersonalFile();

# ファイル出力
&WriteMonsnoOrderFile();

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
  unlink( $temp_personal_csv_file_name );
}


##-------------------------------------
### personalファイルを読み込む
##=====================================
sub ReadPersonalFile
{
  open( DATA, "<:encoding(utf8)",  $personal_csv_file_name );
 
  $mons_num = 0;
  

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
      if( $values[$personal_csv_col_pokename] eq $personal_csv_end_pokename )
      {
        # 終わりを示すポケモン名
        last;
      }
      else
      {
        my $pokename        = $values[$personal_csv_col_pokename];

        if( defined( $mons_tbl_idx_hash{ $pokename } ) )
        {
          # 同じポケモン名が2度出現した
          die "pokename \"$values[$personal_csv_col_pokename]\" error, stopped";
        } 

        my $monsno          = $values[$personal_csv_col_monsno];
        my $grano           = $values[$personal_csv_col_grano];
        my $idx = $mons_num;

        $mons_tbl_idx_hash{ $pokename } = $idx;  # ポケモン名からmons_tblのインデックスを得るためのハッシュ
        
        $mons_tbl[$idx][$mons_col_pokename] = $pokename;
        $mons_tbl[$idx][$mons_col_monsno  ] = $monsno;  # mons_tblのインデックスとmonsnoは同じ順番なので、mons_tblの並びをそのまま利用。
        $mons_tbl[$idx][$mons_col_grano   ] = $grano;

        $mons_num++;
      }
    }


  }


  close( DATA );
}


##-------------------------------------
### ファイル出力
##=====================================
sub WriteMonsnoOrderFile
{
  my $grano;

  open( FH, ">:encoding(utf8)", $monsno_order_file_name );

  printf FH "\"%s\"\r\n", $nclr_file_name;  # 0D 0A
  printf FH "\"%s\"\r\n", $nanr_file_name;  # 0D 0A
  printf FH "\"%s\"\r\n", $ncer_file_name;  # 0D 0A
  printf FH "\"%s\"\r\n", $nanr_file_name_new;  # 0D 0A
  printf FH "\"%s\"\r\n", $ncer_file_name_new;  # 0D 0A

  $grano = 0;
  printf FH "\"%s%03d%s\"\r\n", $ncgr_file_name_head, $grano, $ncgr_file_name_tail;  # 0D 0A

  for( my $i=0; $i<$mons_num; $i++ )  # mons_tblのインデックスとmonsnoは同じ順番なので、mons_tblの並びをそのまま利用。
  {
    $grano  = $mons_tbl[$i][$mons_col_grano];
    printf FH "\"%s%03d%s\"\r\n", $ncgr_file_name_head, $grano, $ncgr_file_name_tail;  # 0D 0A
  }

  close( FH );
}

