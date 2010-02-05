##============================================================================
=pod
     file   zkn_sort_personal_make.pl
     brief  「MONSNO, FORMNO」を○○順に並べて出力する(高い順、低い順、重い順、軽い順)
            「MONSNO」を○○順に並べて出力する(全国図鑑順、地方図鑑順、五十音順)
            五十音順ファイルのどこからアが始まるか、イが始まるか、…を出力する
            おまけ機能：地方図鑑番号を出力する(MONSNOの順番に出力する)
            おまけ機能：頭文字番号を出力する(MONSNOの順番に出力する)
     author Koji Kawada
     data   2010.01.05
     note   perl zkn_sort_personal_make.pl personal_wb.csv

     note   resource/personal/personal_wb.csv(ShiftJIS)
     note   参考  tools/personal_conv/personal_conv.rb(ShiftJIS)

     モジュール名：
=cut 
##============================================================================


##============================================================================
=pod

フォーマット

五十音順ファイルのどこからアが始まるか、イが始まるか、…
    リトルエンディアンのu16
    0<= <0xFFFFその頭文字を持つ名前は、ここから始まる
    0xFFFFその頭文字を持つ名前がない

五十音順ファイルのどこからアが始まるか、イが始まるか、…を記した.hファイル
    start<= <end
    ZKN_SORT_AIUEO_IDX_NONEその頭文字を持つ名前がない

地方図鑑番号
    リトルエンディアンのu16
    1<=地方図鑑に登場し有効な番号
    0地方図鑑に登場しない

頭文字番号
    リトルエンディアンのu8

MONSNOだけのとき
    リトルエンディアンのu16

MONSNOとFORMNOのとき
    リトルエンディアンのu16でMONSNO
    リトルエンディアンのu16でFORMNO
    ( (FORMNO << 10) | (MONSNO << 0) )してひとまとめにしてu16に入れておく
    |15 14 13 12 11 10|09 08 07 06 05 04 03 02 01 00|
    |     FORMNO      |           MONSNO            |
    |  0<=      <64   |        0<=      <1024       |

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
$personal_csv_col_zenkoku_no           =  39;  # 全国図鑑番号。例：フシギダネが1番
$personal_csv_col_chihou_no            =  40;  # 地方図鑑番号。例：フシギダネが231番(シャチでのフシギダネの地方図鑑番号は231番ではないと思われる)
$personal_csv_col_height               = 105;  # csvにあるデータは10倍して整数にしてあるので、表示するときは1/10倍[m]。
$personal_csv_col_weight               = 106;  # csvにあるデータは10倍して整数にしてあるので、表示するときは1/10倍[kg]。

$temp_personal_csv_file_name    = "zkn_sort_personal_make_pl_temp_personal.csv";   # utf8

# 出力ファイル
$sort_high_file_name      = "zkn_sort_high.dat";       # 高い順           # リトルエンディアンのバイナリ
$sort_short_file_name     = "zkn_sort_short.dat";      # 低い順           # リトルエンディアンのバイナリ
$sort_heavy_file_name     = "zkn_sort_heavy.dat";      # 重い順           # リトルエンディアンのバイナリ
$sort_light_file_name     = "zkn_sort_light.dat";      # 軽い順           # リトルエンディアンのバイナリ

$sort_zenkoku_file_name   = "zkn_sort_zenkoku.dat";    # 全国図鑑順       # リトルエンディアンのバイナリ
$sort_chihou_file_name    = "zkn_sort_chihou.dat";     # 地方図鑑順       # リトルエンディアンのバイナリ
$sort_aiueo_file_name     = "zkn_sort_aiueo.dat";      # 五十音順         # リトルエンディアンのバイナリ

$sort_aiueo_idx_file_name = "zkn_sort_aiueo_idx.dat";  # $sort_aiueo_file_nameのどこからアが始まるか、イが始まるか、…  # リトルエンディアンのバイナリ
$sort_aiueo_idx_header_file_name  = "zkn_sort_aiueo_idx.h";  # $sort_aiueo_file_nameのどこからアが始まるか、イが始まるか、…  # shiftjis

$chihou_no_file_name      = "zkn_chihou_no.dat";       # 地方図鑑番号     # リトルエンディアンのバイナリ      # おまけ
$initial_file_name        = "zkn_initial.dat";         # 頭文字番号       # バイナリ                          # おまけ

# デバッグ
$debug_monsno_formno_csv_file_name    = "debug_zkn_sort_personal_make_monsno_formno.csv";  # shiftjis
$debug_monsform_csv_file_name         = "debug_zkn_sort_personal_make_monsform.csv";       # shiftjis

$debug_sort_high_file_name      = "debug_zkn_sort_personal_make_sort_high.csv";     # shiftjis
$debug_sort_short_file_name     = "debug_zkn_sort_personal_make_sort_short.csv";    # shiftjis
$debug_sort_heavy_file_name     = "debug_zkn_sort_personal_make_sort_heavy.csv";    # shiftjis
$debug_sort_light_file_name     = "debug_zkn_sort_personal_make_sort_light.csv";    # shiftjis

$debug_sort_zenkoku_file_name   = "debug_zkn_sort_personal_make_sort_zenkoku.csv";  # shiftjis
$debug_sort_chihou_file_name    = "debug_zkn_sort_personal_make_sort_chihou.csv";   # shiftjis
$debug_sort_aiueo_file_name     = "debug_zkn_sort_personal_make_sort_aiueo.csv";    # shiftjis

$debug_sort_aiueo_idx_csv_file_name = "debug_zkn_sort_personal_make_aiueo_idx.csv";  # shiftjis

$debug_chihou_no_csv_file_name  = "debug_zkn_sort_personal_make_chihou_no.csv";     # shiftjis      # おまけ
$debug_initial_csv_file_name    = "debug_zkn_sort_personal_make_initial.csv";       # shiftjis      # おまけ

# ビット
$monsno_shift    =  0;  # $monsno_formno = ($formno << $formno_shift) | ($monsno << $monsno_shift);
$formno_shift    = 10;
$monsno_mask     = 0b0000001111111111;  # monsnoを取り出すためのマスク
$formno_mask     = 0b1111110000000000;  # formnoを取り出すためのマスク

# aiueo
@aiueo_replace_bar_tbl =
(
  # これで全てに対応できているか不安
  [ 'アァカガサザタダナハバパマヤャラワ'    , 'ア' ],
  [ 'イィキギシジチヂニヒビピミリ'          , 'イ' ],
  [ 'ウゥクグスズツヅヌフブプムユュルヴ'    , 'ウ' ],
  [ 'エェケゲセゼテデネヘベペメレ'          , 'エ' ],
  [ 'オォコゴソゾトドノホボポモヨョロヲ'    , 'オ' ],
#  [ 'ン'                                    , 'ン' ],
);

%aiueo_romaji_tbl =
(
  "ア"=>"A",  "イ"=>"I",  "ウ"=>"U",  "エ"=>"E",  "オ"=>"O",
  "カ"=>"KA", "キ"=>"KI", "ク"=>"KU", "ケ"=>"KE", "コ"=>"KO",
  "サ"=>"SA", "シ"=>"SI", "ス"=>"SU", "セ"=>"SE", "ソ"=>"SO",
  "タ"=>"TA", "チ"=>"TI", "ツ"=>"TU", "テ"=>"TE", "ト"=>"TO",
  "ナ"=>"NA", "ニ"=>"NI", "ヌ"=>"NU", "ネ"=>"NE", "ノ"=>"NO",
  "ハ"=>"HA", "ヒ"=>"HI", "フ"=>"HU", "ヘ"=>"HE", "ホ"=>"HO",
  "マ"=>"MA", "ミ"=>"MI", "ム"=>"MU", "メ"=>"ME", "モ"=>"MO",
  "ヤ"=>"YA",             "ユ"=>"YU",             "ヨ"=>"YO",
  "ラ"=>"RA", "リ"=>"RI", "ル"=>"RU", "レ"=>"RE", "ロ"=>"RO",
  "ワ"=>"WA",                                     "ヲ"=>"WO",
  "ン"=>"N", 
);

$sort_aiueo_idx_none = 0xFFFF;  # @sort_aiueo_idxのどこからアが始まるか、イが始まるか、…においてその頭文字を持つ名前がない

# $sort_aiueo_idx_header_file_nameにて使用するdefine名
$aiueo_start_idx_define_prefix    = "ZKN_SORT_AIUEO_IDX_START_";
$aiueo_end_idx_define_prefix      = "ZKN_SORT_AIUEO_IDX_END_";
$aiueo_none_define                = "ZKN_SORT_AIUEO_IDX_NONE";
$aiueo_all_end_define             = "ZKN_SORT_AIUEO_IDX_ALL_END";

# 変数
# mons
$mons_num;
@mons_tbl = ();  # 0<= <$mons_num
$mons_col_monsname       = 0;  # 例：アーボック
$mons_col_aiueo_monsname = 1;  # 例：アアホツク
$mons_col_initial        = 2;  # 例："ア"=>0
$mons_col_monsno         = 3;  # 1スタート。001がフシギダネ。
$mons_col_form_num       = 4;
$mons_col_form_start     = 5;  # 最後(含まない)は($mons_col_form_start + $form_col_offset_max * $mons_col_form_numの値)

$form_col_offset_form_name   = 0;
$form_col_offset_formno      = 1;  # 0スタート。($mons_col_form_numの値 - 1)が最後(含む)。
$form_col_offset_zenkoku_no  = 2;
$form_col_offset_chihou_no   = 3;
$form_col_offset_height      = 4;
$form_col_offset_weight      = 5;
$form_col_offset_max         = 6;

# 0<=$formno<$mons_col_form_numとすると
# $mons_tbl[ $mons_col_form_start + $form_col_offset_max * $formno + $form_col_offset_form_name ] = $formnoのフォルム名;
# $mons_tbl[ $mons_col_form_start + $form_col_offset_max * $formno + $form_col_offset_formno    ] = $formnoそのもの;
# $mons_tbl[ $mons_col_form_start + $form_col_offset_max * $formno + $form_col_offset_height    ] = $formnoの高さ(10倍整数のまま);
# $mons_tbl[ $mons_col_form_start + $form_col_offset_max * $formno + $form_col_offset_weight    ] = $formnoの重さ(10倍整数のまま);
# となる

%mons_tbl_idx_hash = ();  # ポケモン名からmons_tblのインデックスを得るためのハッシュ

# monsとformを一列にした配列
$monsform_num;
@monsform_tbl = ();  # 0<= <$monsform_num
$monsform_col_mons_tbl_idx    = 0;  # mons_tblのインデックス
$monsform_col_monsno          = 1;  # $mons_col_monsnoの値
$monsform_col_formno          = 2;  # $form_col_offset_formnoの値

# ソート結果
@sort_high_idx     = ();  # monsform_tblのインデックスを高い順に並べた配列
@sort_short_idx    = ();  # monsform_tblのインデックスを低い順に並べた配列
@sort_heavy_idx    = ();  # monsform_tblのインデックスを重い順に並べた配列
@sort_light_idx    = ();  # monsform_tblのインデックスを軽い順に並べた配列

@sort_zenkoku_idx  = ();  # mons_tblのインデックスを全国図鑑順に並べた配列
@sort_chihou_idx   = ();  # mons_tblのインデックスを地方図鑑順に並べた配列
@sort_aiueo_idx    = ();  # mons_tblのインデックスを五十音順に並べた配列

# @sort_aiueo_idxのどこからアが始まるか、イが始まるか、…
%aiueo_start_idx_tbl =
(
  "ア"=>$sort_aiueo_idx_none, "イ"=>$sort_aiueo_idx_none, "ウ"=>$sort_aiueo_idx_none, "エ"=>$sort_aiueo_idx_none, "オ"=>$sort_aiueo_idx_none,
  "カ"=>$sort_aiueo_idx_none, "キ"=>$sort_aiueo_idx_none, "ク"=>$sort_aiueo_idx_none, "ケ"=>$sort_aiueo_idx_none, "コ"=>$sort_aiueo_idx_none,
  "サ"=>$sort_aiueo_idx_none, "シ"=>$sort_aiueo_idx_none, "ス"=>$sort_aiueo_idx_none, "セ"=>$sort_aiueo_idx_none, "ソ"=>$sort_aiueo_idx_none,
  "タ"=>$sort_aiueo_idx_none, "チ"=>$sort_aiueo_idx_none, "ツ"=>$sort_aiueo_idx_none, "テ"=>$sort_aiueo_idx_none, "ト"=>$sort_aiueo_idx_none,
  "ナ"=>$sort_aiueo_idx_none, "ニ"=>$sort_aiueo_idx_none, "ヌ"=>$sort_aiueo_idx_none, "ネ"=>$sort_aiueo_idx_none, "ノ"=>$sort_aiueo_idx_none,
  "ハ"=>$sort_aiueo_idx_none, "ヒ"=>$sort_aiueo_idx_none, "フ"=>$sort_aiueo_idx_none, "ヘ"=>$sort_aiueo_idx_none, "ホ"=>$sort_aiueo_idx_none,
  "マ"=>$sort_aiueo_idx_none, "ミ"=>$sort_aiueo_idx_none, "ム"=>$sort_aiueo_idx_none, "メ"=>$sort_aiueo_idx_none, "モ"=>$sort_aiueo_idx_none,
  "ヤ"=>$sort_aiueo_idx_none,                             "ユ"=>$sort_aiueo_idx_none,                             "ヨ"=>$sort_aiueo_idx_none,
  "ラ"=>$sort_aiueo_idx_none, "リ"=>$sort_aiueo_idx_none, "ル"=>$sort_aiueo_idx_none, "レ"=>$sort_aiueo_idx_none, "ロ"=>$sort_aiueo_idx_none,
  "ワ"=>$sort_aiueo_idx_none,                                                                                     "ヲ"=>$sort_aiueo_idx_none,
  "ン"=>$sort_aiueo_idx_none, 
);

# 頭文字番号
%initial_to_no_tbl =
(
  "ア"=>0,  "イ"=>1,  "ウ"=>2,  "エ"=>3,  "オ"=>4,
  "カ"=>5,  "キ"=>6,  "ク"=>7,  "ケ"=>8,  "コ"=>9,
  "サ"=>10, "シ"=>11, "ス"=>12, "セ"=>13, "ソ"=>14,
  "タ"=>15, "チ"=>16, "ツ"=>17, "テ"=>18, "ト"=>19,
  "ナ"=>20, "ニ"=>21, "ヌ"=>22, "ネ"=>23, "ノ"=>24,
  "ハ"=>25, "ヒ"=>26, "フ"=>27, "ヘ"=>28, "ホ"=>29,
  "マ"=>30, "ミ"=>31, "ム"=>32, "メ"=>33, "モ"=>34,
  "ヤ"=>35,           "ユ"=>36,           "ヨ"=>37,
  "ラ"=>38, "リ"=>39, "ル"=>40, "レ"=>41, "ロ"=>42,
  "ワ"=>43,                               "ヲ"=>44,
  "ン"=>45, 
);
$initial_to_no_unknown   = 46;  # 不明な頭文字のとき


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

# aiueoソート用の名前を作成する
&MakeAiueoMonsname();

# 頭文字を見て頭文字番号を与える
&SetInitial();

# monsとformを一列にした配列を作成する
&MakeMonsformTable();
&DebugWriteMonsformFile();

# ソート
&SortHigh();
&SortShort();
&SortHeavy();
&SortLight();

&SortZenkoku();
&SortChihou();
&SortAiueo();

# ソート結果を出力する
&WriteSortFileMonsnoFormno( $sort_high_file_name, @sort_high_idx );
&WriteSortFileMonsnoFormno( $sort_short_file_name, @sort_short_idx );
&WriteSortFileMonsnoFormno( $sort_heavy_file_name, @sort_heavy_idx );
&WriteSortFileMonsnoFormno( $sort_light_file_name, @sort_light_idx );

&WriteSortFileMonsno( $sort_zenkoku_file_name, @sort_zenkoku_idx );
&WriteSortFileMonsno( $sort_chihou_file_name, @sort_chihou_idx );
&WriteSortFileMonsno( $sort_aiueo_file_name, @sort_aiueo_idx );

&DebugWriteSortFileMonsnoFormno( $form_col_offset_height, $debug_sort_high_file_name, @sort_high_idx );  #debug
&DebugWriteSortFileMonsnoFormno( $form_col_offset_height, $debug_sort_short_file_name, @sort_short_idx );  #debug
&DebugWriteSortFileMonsnoFormno( $form_col_offset_weight, $debug_sort_heavy_file_name, @sort_heavy_idx );  #debug
&DebugWriteSortFileMonsnoFormno( $form_col_offset_weight, $debug_sort_light_file_name, @sort_light_idx );  #debug

&DebugWriteSortFileMonsno( $form_col_offset_zenkoku_no, $debug_sort_zenkoku_file_name, @sort_zenkoku_idx );  #debug
&DebugWriteSortFileMonsno( $form_col_offset_chihou_no, $debug_sort_chihou_file_name, @sort_chihou_idx );  #debug
&DebugWriteSortFileMonsno( $mons_col_aiueo_monsname-$mons_col_form_start, $debug_sort_aiueo_file_name, @sort_aiueo_idx );  # $mons_col_aiueo_monsnameが表示されるように工夫した引数を渡しています  #debug

# @sort_aiueo_idxのどこからアが始まるか、イが始まるか、…
&MakeAiueoStartIndexTable();
#&WriteSortAiueoIndexFile();  # これはやめて.hを出力することにした
&WriteSortAiueoIndexHeaderFile();
&DebugWriteSortAiueoIndexFile();  #debug

# 地方図鑑番号を出力する
&WriteChihouNoFile();                    # おまけ
&DebugWriteChihouNoFile();  #debug       # おまけ

# 頭文字番号を出力する
&WriteInitialFile();                     # おまけ
&DebugWriteInitialFile();  #debug        # おまけ

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
  unlink( $debug_monsform_csv_file_name );

  unlink( $debug_sort_high_file_name );
  unlink( $debug_sort_short_file_name );
  unlink( $debug_sort_heavy_file_name );
  unlink( $debug_sort_light_file_name );

  unlink( $debug_sort_zenkoku_file_name );
  unlink( $debug_sort_chihou_file_name );
  unlink( $debug_sort_aiueo_file_name );

  unlink( $debug_sort_aiueo_idx_csv_file_name );

  unlink( $debug_chihou_no_csv_file_name );
  unlink( $debug_initial_csv_file_name );
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
          $mons_tbl[$idx][$form_col_start + $form_col_offset_form_name ] = $values[$personal_csv_col_form_name];
          $mons_tbl[$idx][$form_col_start + $form_col_offset_formno    ] = $formno;
          $mons_tbl[$idx][$form_col_start + $form_col_offset_zenkoku_no] = $values[$personal_csv_col_zenkoku_no];
          $mons_tbl[$idx][$form_col_start + $form_col_offset_chihou_no ] = $values[$personal_csv_col_chihou_no];
          $mons_tbl[$idx][$form_col_start + $form_col_offset_height    ] = $values[$personal_csv_col_height];
          $mons_tbl[$idx][$form_col_start + $form_col_offset_weight    ] = $values[$personal_csv_col_weight];
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
        $mons_tbl[$idx][$form_col_start + $form_col_offset_form_name ] = $values[$personal_csv_col_form_name];
        $mons_tbl[$idx][$form_col_start + $form_col_offset_formno    ] = $formno;
        $mons_tbl[$idx][$form_col_start + $form_col_offset_zenkoku_no] = $values[$personal_csv_col_zenkoku_no];
        $mons_tbl[$idx][$form_col_start + $form_col_offset_chihou_no ] = $values[$personal_csv_col_chihou_no];
        $mons_tbl[$idx][$form_col_start + $form_col_offset_height    ] = $values[$personal_csv_col_height];
        $mons_tbl[$idx][$form_col_start + $form_col_offset_weight    ] = $values[$personal_csv_col_weight];
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

  print FH "monsname,monsno,form_num,form_name,formno,zenkoku_no,chihou_no,height,weight,form_name,formno,zenkoku_no,chihou_no,height,weight,\r\n";  # 0D 0A

  for( my $idx=0; $idx<$mons_num; $idx++ )
  {
    print FH "$mons_tbl[$idx][$mons_col_monsname],";
    print FH "$mons_tbl[$idx][$mons_col_monsno  ],";
    print FH "$mons_tbl[$idx][$mons_col_form_num],";

    my $form_num = $mons_tbl[$idx][$mons_col_form_num];
    for( my $formno=0; $formno<$form_num; $formno++ )
    {
      my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;
      print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_form_name ],";
      print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_formno    ],";
      print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_zenkoku_no],";
      print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_chihou_no ],";
      print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_height    ],";
      print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_weight    ],";
    }

    print FH "\r\n";  # 0D 0A
  }

  close( FH );
}

##-------------------------------------
### monsとformを一列にした配列を作成する
##=====================================
sub MakeMonsformTable
{
  $monsform_num = 0;

  for( my $idx=0; $idx<$mons_num; $idx++ )
  {
    my $form_num = $mons_tbl[$idx][$mons_col_form_num];
    for( my $formno=0; $formno<$form_num; $formno++ )
    {
      $monsform_tbl[$monsform_num][ $monsform_col_mons_tbl_idx ] = $idx;
      $monsform_tbl[$monsform_num][ $monsform_col_monsno       ] = $mons_tbl[$idx][$mons_col_monsno  ];
     
      my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;
      $monsform_tbl[$monsform_num][ $monsform_col_formno       ] = $mons_tbl[$idx][$form_col_start + $form_col_offset_formno    ];

      $monsform_num++;
    }
  }
}

##-------------------------------------
### monsform_tblの中身を確認する
##=====================================
sub DebugWriteMonsformFile
{
  open( FH, ">:encoding(shiftjis)", $debug_monsform_csv_file_name );

  print FH "monsname,monsno,formno,\r\n";  # 0D 0A

  for( my $idx=0; $idx<$monsform_num; $idx++ )
  {
    print FH "$mons_tbl[ $monsform_tbl[$idx][$monsform_col_mons_tbl_idx] ][$mons_col_monsname],";
    print FH "$monsform_tbl[$idx][$monsform_col_monsno],";
    print FH "$monsform_tbl[$idx][$monsform_col_formno],";
    print FH "\r\n";  # 0D 0A
  }

  close( FH );
}

##-------------------------------------
### ソート
##=====================================
sub SortHigh
{
  for( my $i=0; $i<$monsform_num; $i++ )
  {
    $sort_high_idx[$i] = $i;
  }
  @sort_high_idx = sort SortHighFunc @sort_high_idx;
}
sub SortHighFunc
{
  # 比較関数
  #   $a < $b : -1
  #   $a > $b : 1
  #   $a = $b : 0
  
  my $ret;
  my $a_idx               = $monsform_tbl[$a][$monsform_col_mons_tbl_idx];
  my $a_formno            = $monsform_tbl[$a][$monsform_col_formno];
  my $a_form_col_start    = $mons_col_form_start + $form_col_offset_max * $a_formno;
  my $b_idx               = $monsform_tbl[$b][$monsform_col_mons_tbl_idx];
  my $b_formno            = $monsform_tbl[$b][$monsform_col_formno];
  my $b_form_col_start    = $mons_col_form_start + $form_col_offset_max * $b_formno;

  my $a_form_col_target   = $a_form_col_start + $form_col_offset_height;
  my $b_form_col_target   = $b_form_col_start + $form_col_offset_height;
   
  $ret = ( $mons_tbl[$b_idx][$b_form_col_target] <=> $mons_tbl[$a_idx][$a_form_col_target] );
  return($ret);
}

sub SortShort
{
  for( my $i=0; $i<$monsform_num; $i++ )
  {
    $sort_short_idx[$i] = $i;
  }
  @sort_short_idx = sort SortShortFunc @sort_short_idx;
}
sub SortShortFunc
{
  my $ret;
  my $a_idx               = $monsform_tbl[$a][$monsform_col_mons_tbl_idx];
  my $a_formno            = $monsform_tbl[$a][$monsform_col_formno];
  my $a_form_col_start    = $mons_col_form_start + $form_col_offset_max * $a_formno;
  my $b_idx               = $monsform_tbl[$b][$monsform_col_mons_tbl_idx];
  my $b_formno            = $monsform_tbl[$b][$monsform_col_formno];
  my $b_form_col_start    = $mons_col_form_start + $form_col_offset_max * $b_formno;

  my $a_form_col_target   = $a_form_col_start + $form_col_offset_height;
  my $b_form_col_target   = $b_form_col_start + $form_col_offset_height;
   
  $ret = ( $mons_tbl[$a_idx][$a_form_col_target] <=> $mons_tbl[$b_idx][$b_form_col_target] );
  return($ret);
}

sub SortHeavy
{
  for( my $i=0; $i<$monsform_num; $i++ )
  {
    $sort_heavy_idx[$i] = $i;
  }
  @sort_heavy_idx = sort SortHeavyFunc @sort_heavy_idx;
}
sub SortHeavyFunc
{
  my $ret;
  my $a_idx               = $monsform_tbl[$a][$monsform_col_mons_tbl_idx];
  my $a_formno            = $monsform_tbl[$a][$monsform_col_formno];
  my $a_form_col_start    = $mons_col_form_start + $form_col_offset_max * $a_formno;
  my $b_idx               = $monsform_tbl[$b][$monsform_col_mons_tbl_idx];
  my $b_formno            = $monsform_tbl[$b][$monsform_col_formno];
  my $b_form_col_start    = $mons_col_form_start + $form_col_offset_max * $b_formno;

  my $a_form_col_target   = $a_form_col_start + $form_col_offset_weight;
  my $b_form_col_target   = $b_form_col_start + $form_col_offset_weight;
   
  $ret = ( $mons_tbl[$b_idx][$b_form_col_target] <=> $mons_tbl[$a_idx][$a_form_col_target] );
  return($ret);
}

sub SortLight
{
  for( my $i=0; $i<$monsform_num; $i++ )
  {
    $sort_light_idx[$i] = $i;
  }
  @sort_light_idx = sort SortLightFunc @sort_light_idx;
}
sub SortLightFunc
{
  my $ret;
  my $a_idx               = $monsform_tbl[$a][$monsform_col_mons_tbl_idx];
  my $a_formno            = $monsform_tbl[$a][$monsform_col_formno];
  my $a_form_col_start    = $mons_col_form_start + $form_col_offset_max * $a_formno;
  my $b_idx               = $monsform_tbl[$b][$monsform_col_mons_tbl_idx];
  my $b_formno            = $monsform_tbl[$b][$monsform_col_formno];
  my $b_form_col_start    = $mons_col_form_start + $form_col_offset_max * $b_formno;

  my $a_form_col_target   = $a_form_col_start + $form_col_offset_weight;
  my $b_form_col_target   = $b_form_col_start + $form_col_offset_weight;
   
  $ret = ( $mons_tbl[$a_idx][$a_form_col_target] <=> $mons_tbl[$b_idx][$b_form_col_target] );
  return($ret);
}

sub SortZenkoku
{
  for( my $i=0; $i<$mons_num; $i++ )
  {
    $sort_zenkoku_idx[$i] = $i;
  }
  @sort_zenkoku_idx = sort SortZenkokuFunc @sort_zenkoku_idx;
}
sub SortZenkokuFunc
{
  my $ret;
  my $formno = 0;
  my $a_form_col_start    = $mons_col_form_start + $form_col_offset_max * $formno;
  my $b_form_col_start    = $mons_col_form_start + $form_col_offset_max * $formno;

  my $a_form_col_target   = $a_form_col_start + $form_col_offset_zenkoku_no;
  my $b_form_col_target   = $b_form_col_start + $form_col_offset_zenkoku_no;
   
  $ret = ( $mons_tbl[$a][$a_form_col_target] <=> $mons_tbl[$b][$b_form_col_target] );
  return($ret);
}

sub SortChihou
{
  for( my $i=0; $i<$mons_num; $i++ )
  {
    $sort_chihou_idx[$i] = $i;
  }
  @sort_chihou_idx = sort SortChihouFunc @sort_chihou_idx;
}
sub SortChihouFunc
{
  my $ret;
  my $formno = 0;
  my $a_form_col_start    = $mons_col_form_start + $form_col_offset_max * $formno;
  my $b_form_col_start    = $mons_col_form_start + $form_col_offset_max * $formno;

  my $a_form_col_target   = $a_form_col_start + $form_col_offset_chihou_no;
  my $b_form_col_target   = $b_form_col_start + $form_col_offset_chihou_no;
   
  $ret = ( $mons_tbl[$a][$a_form_col_target] <=> $mons_tbl[$b][$b_form_col_target] );
  return($ret);
}

sub SortAiueo
{
  for( my $i=0; $i<$mons_num; $i++ )
  {
    $sort_aiueo_idx[$i] = $i;
  }
  @sort_aiueo_idx = sort SortAiueoFunc @sort_aiueo_idx;
}
sub SortAiueoFunc
{
  my $ret;
  $ret = ( $mons_tbl[$a][$mons_col_aiueo_monsname] cmp $mons_tbl[$b][$mons_col_aiueo_monsname] );
  return($ret);
}

##-------------------------------------
### ソート結果を出力する
##=====================================
sub WriteSortFileMonsnoFormno
{
  local( $file_name, @sorted_idx ) = @_;

  open( FH, ">", $file_name );

  binmode FH;

  my $buf;
  my $monsno;
  my $formno;
  my $monsno_formno;

  for( my $i=0; $i<$monsform_num; $i++ )
  {
    my $si                = $sorted_idx[$i];

    $monsno            = $monsform_tbl[$si][$monsform_col_monsno];
    $formno            = $monsform_tbl[$si][$monsform_col_formno];
  
    $monsno_formno = ($formno << $formno_shift) | ($monsno << $monsno_shift);
    $buf = pack "v", $monsno_formno;  # packは最後に行う  # 符号なし16ビット整数。リトルエンディアン。
    print FH "$buf";
  }

  close( FH );
}

sub WriteSortFileMonsno
{
  local( $file_name, @sorted_idx ) = @_;

  open( FH, ">", $file_name );

  binmode FH;

  my $buf;
  my $monsno;

  for( my $i=0; $i<$mons_num; $i++ )
  {
    my $si                = $sorted_idx[$i];

    $monsno            = $mons_tbl[$si][$mons_col_monsno];
  
    $buf = pack "v", $monsno;  # 符号なし16ビット整数。リトルエンディアン。
    print FH "$buf";
  }

  close( FH );
}

sub DebugWriteSortFileMonsnoFormno
{
  local( $form_col_offset_target, $file_name, @sorted_idx ) = @_;

  open( FH, ">:encoding(shiftjis)", $file_name );

  print FH "pokename,target,monsno,formno,\r\n";  # 0D 0A

  for( my $i=0; $i<$monsform_num; $i++ )
  {
    my $si                = $sorted_idx[$i];

    my $idx               = $monsform_tbl[$si][$monsform_col_mons_tbl_idx];
    my $formno            = $monsform_tbl[$si][$monsform_col_formno];
    my $form_col_start    = $mons_col_form_start + $form_col_offset_max * $formno;
    
    my $form_col_target   = $form_col_start + $form_col_offset_target;

    print FH "$mons_tbl[$idx][$mons_col_monsname],";
    print FH "$mons_tbl[$idx][$form_col_target],";
    print FH "$monsform_tbl[$si][$monsform_col_monsno],";
    print FH "$monsform_tbl[$si][$monsform_col_formno],";

    print FH "\r\n";  # 0D 0A
  }

  close( FH );
}

sub DebugWriteSortFileMonsno
{
  local( $form_col_offset_target, $file_name, @sorted_idx ) = @_;

  open( FH, ">:encoding(shiftjis)", $file_name );

  print FH "pokename,target,monsno,\r\n";  # 0D 0A

  for( my $i=0; $i<$mons_num; $i++ )
  {
    my $si                = $sorted_idx[$i];

    my $formno            = 0;
    my $form_col_start    = $mons_col_form_start + $form_col_offset_max * $formno;
    
    my $form_col_target   = $form_col_start + $form_col_offset_target;

    print FH "$mons_tbl[$si][$mons_col_monsname],";
    print FH "$mons_tbl[$si][$form_col_target],";
    print FH "$mons_tbl[$si][$mons_col_monsno],";
    
    print FH "\r\n";  # 0D 0A
  }

  close( FH );
}

##-------------------------------------
### 地方図鑑番号を出力する
##=====================================
sub WriteChihouNoFile
{
  open( FH, ">", $chihou_no_file_name );

  binmode FH;

  my $buf;
  my $chihou_no;

  # MONSNO 0番
  $chihou_no = 0;
  $buf = pack "v", $chihou_no;  # 符号なし16ビット整数。リトルエンディアン。
  print FH "$buf";

  # MONSNO 1番から
  for( my $i=0; $i<$mons_num; $i++ )
  {
    my $formno            = 0;
    my $form_col_start    = $mons_col_form_start + $form_col_offset_max * $formno;
    
    my $form_col_chihou_no   = $form_col_start + $form_col_offset_chihou_no;

    $chihou_no = 0;
    if( $mons_tbl[$i][$form_col_chihou_no] ne "" )
    {
      $chihou_no = $mons_tbl[$i][$form_col_chihou_no];
    }

    $buf = pack "v", $chihou_no;  # 符号なし16ビット整数。リトルエンディアン。
    print FH "$buf";
  }

  close( FH );
}

sub DebugWriteChihouNoFile
{
  open( FH, ">:encoding(shiftjis)", $debug_chihou_no_csv_file_name );

  my $chihou_no;
  
  print FH "pokename,chihou_no,monsno,\r\n";  # 0D 0A

  # MONSNO 0番
  $chihou_no = 0;
  print FH "ーーーーー,$chihou_no,0,\r\n";  # 0D 0A

  # MONSNO 1番から
  for( my $i=0; $i<$mons_num; $i++ )
  {
    my $formno            = 0;
    my $form_col_start    = $mons_col_form_start + $form_col_offset_max * $formno;
    
    my $form_col_chihou_no   = $form_col_start + $form_col_offset_chihou_no;

    $chihou_no = 0;
    if( $mons_tbl[$i][$form_col_chihou_no] ne "" )
    {
      $chihou_no = $mons_tbl[$i][$form_col_chihou_no];
    }

    print FH "$mons_tbl[$i][$mons_col_monsname],";
    print FH "$chihou_no,";
    print FH "$mons_tbl[$i][$mons_col_monsno],";
    
    print FH "\r\n";  # 0D 0A
  }

  close( FH );
}


##-------------------------------------
### 頭文字番号を出力する
##=====================================
sub WriteInitialFile
{
  open( FH, ">", $initial_file_name );

  binmode FH;

  my $buf;
  my $initial;

  # MONSNO 0番
  $initial = $initial_to_no_unknown;
  $buf = pack "C", $initial;  # 符号なし8ビット整数。(u8だからリトルエンディアンとかない)
  print FH "$buf";

  # MONSNO 1番から
  for( my $i=0; $i<$mons_num; $i++ )
  {
    $initial = $initial_to_no_unknown;
    if( $mons_tbl[$i][$mons_col_initial] ne "" )
    {
      $initial = $mons_tbl[$i][$mons_col_initial];
    }
    $buf = pack "C", $initial;  # 符号なし8ビット整数。(u8だからリトルエンディアンとかない)
    print FH "$buf";
  }

  close( FH );
}

sub DebugWriteInitialFile
{
  open( FH, ">:encoding(shiftjis)", $debug_initial_csv_file_name );

  my $initial;
  
  print FH "pokename,initial,monsno,\r\n";  # 0D 0A

  # MONSNO 0番
  $initial = $initial_to_no_unknown;
  print FH "ーーーーー,$initial,0,\r\n";  # 0D 0A

  # MONSNO 1番から
  for( my $i=0; $i<$mons_num; $i++ )
  {
    $initial = $initial_to_no_unknown;
    if( $mons_tbl[$i][$mons_col_initial] ne "" )
    {
      $initial = $mons_tbl[$i][$mons_col_initial];
    }

    print FH "$mons_tbl[$i][$mons_col_monsname],";
    print FH "$initial,";
    print FH "$mons_tbl[$i][$mons_col_monsno],";
    
    print FH "\r\n";  # 0D 0A
  }

  close( FH );
}

##-------------------------------------
### 頭文字を見て頭文字番号を与える
##=====================================
sub SetInitial
{
  # MakeAiueoMonsname()が済み、
  # $mons_tbl[$i][$mons_col_aiueo_monsname]に値が
  # 設定されてから呼ぶこと。

  for( my $i=0; $i<$mons_num; $i++ )
  {
    my $str = $mons_tbl[$i][$mons_col_aiueo_monsname];
    my $c = substr( $str, 0, 1 );
    my $no = $initial_to_no_tbl{$c};
    if( !defined($no) )
    {
      $no = $initial_to_no_unknown;
    }
    $mons_tbl[$i][$mons_col_initial] = $no;
  }
}

##-------------------------------------
### aiueoソート用の名前を作成する
##=====================================
sub MakeAiueoMonsname
{
  for( my $i=0; $i<$mons_num; $i++ )
  {
    my $str = $mons_tbl[$i][$mons_col_monsname];

    $str = &ReplaceAiueoPoint( $str );
    $str = &ReplaceAiueoBar( $str );

    $mons_tbl[$i][$mons_col_aiueo_monsname] = $str;
  }
}

# パ→ハ、ッ→ツ、など
sub ReplaceAiueoPoint
{
  my $str = $_[0];

  # これで全てに対応できているか不安
  $str =~ tr/ァ,ィ,ゥ,ェ,ォ/ア,イ,ウ,エ,オ/;
  $str =~ tr/ッ/ツ/;
  $str =~ tr/ャ,ュ,ョ/ヤ,ユ,ヨ/;
  $str =~ tr/ガ,ギ,グ,ゲ,ゴ/カ,キ,ク,ケ,コ/;
  $str =~ tr/ザ,ジ,ズ,ゼ,ゾ/サ,シ,ス,セ,ソ/;
  $str =~ tr/ダ,ヂ,ヅ,デ,ド/タ,チ,ツ,テ,ト/;
  $str =~ tr/バ,ビ,ブ,ベ,ボ/ハ,ヒ,フ,ヘ,ホ/;
  $str =~ tr/パ,ピ,プ,ペ,ポ/ハ,ヒ,フ,ヘ,ホ/;

  return ($str);
}

# ー→ア、など
sub ReplaceAiueoBar
{
  my $str = $_[0];
  my @list;
  my $max;
  my $i;
  my $t;
  my $ref;

  @list = split( /ー/, $str );
  $max = @list - 1;
  if( $str =~ /ー$/ )
  {
    $max++;
  }

  for($i=0; $i<$max; $i++)
  {
    $t = substr($list[$i], length($list[$i])-1, 1);

    foreach $ref (@aiueo_replace_bar_tbl)
    {
      if( index( $$ref[0], $t ) >= 0 )
      {
        $list[$i] .= $$ref[1];
        last;
      }
    }
  }

  $str = "";
  foreach $t (@list)
  {
    $str .= $t;
  }

  return ($str);
}

##-------------------------------------
### @sort_aiueo_idxのどこからアが始まるか、イが始まるか、…
##=====================================
sub MakeAiueoStartIndexTable
{
  for( my $i=0; $i<$mons_num; $i++ )
  {
    my $si = $sort_aiueo_idx[$i];
    my $head_key = substr( $mons_tbl[$si][$mons_col_aiueo_monsname], 0, 1 );
    if( $aiueo_start_idx_tbl{ $head_key } == $sort_aiueo_idx_none )
    {
      $aiueo_start_idx_tbl{ $head_key } = $i;
    }
  }
}

sub WriteSortAiueoIndexFile
{
  open( FH, ">", $sort_aiueo_idx_file_name );

  binmode FH;

  my @aiueo_start_idx_tbl_key = keys(%aiueo_start_idx_tbl);
  @aiueo_start_idx_tbl_key = sort { $a cmp $b } @aiueo_start_idx_tbl_key;
  foreach my $k ( @aiueo_start_idx_tbl_key )
  {
    my $buf = pack "v", $aiueo_start_idx_tbl{$k};  # 符号なし16ビット整数。リトルエンディアン。
    print FH "$buf";
  }

  close( FH );
}

sub DebugWriteSortAiueoIndexFile
{
  open( FH, ">:encoding(shiftjis)", $debug_sort_aiueo_idx_csv_file_name );

  print FH "key,idx,\r\n";  # 0D 0A
  
  my @aiueo_start_idx_tbl_key = keys(%aiueo_start_idx_tbl);
  @aiueo_start_idx_tbl_key = sort { $a cmp $b } @aiueo_start_idx_tbl_key;
  foreach my $k ( @aiueo_start_idx_tbl_key )
  {
    print FH "$k,";
    print FH "$aiueo_start_idx_tbl{$k},";
    print FH "\r\n";  # 0D 0A
  }

  close( FH );
}

sub WriteSortAiueoIndexHeaderFile
{
  open( FH, ">:encoding(shiftjis)", $sort_aiueo_idx_header_file_name );
  
  my @aiueo_start_idx_tbl_key = keys(%aiueo_start_idx_tbl);
  @aiueo_start_idx_tbl_key = sort { $a cmp $b } @aiueo_start_idx_tbl_key;

  my $key_num = @aiueo_start_idx_tbl_key;
  my @aiueo_start_idx = ();
  my @aiueo_end_idx   = ();

  for( my $i=0; $i<$key_num; $i++ )
  {
    my $k = $aiueo_start_idx_tbl_key[$i];
    $aiueo_start_idx[$i] = $aiueo_start_idx_tbl{$k};
    if( $aiueo_start_idx[$i] == $sort_aiueo_idx_none )
    {
      $aiueo_end_idx[$i] = $sort_aiueo_idx_none;
    }
    else
    {
      my $end_idx = $sort_aiueo_idx_none;
      for( my $j=$i+1; $j<$key_num; $j++ )
      {
        my $l = $aiueo_start_idx_tbl_key[$j];
        $end_idx = $aiueo_start_idx_tbl{$l};
        if( $end_idx != $sort_aiueo_idx_none )
        {
          last;
        }
      }
      if( $end_idx == $sort_aiueo_idx_none )
      {
        $aiueo_end_idx[$i] = $mons_num;
      }
      else
      {
        $aiueo_end_idx[$i] = $end_idx;
      }
    }
  }

  # 説明
  printf FH "//============================================================================\r\n";
  printf FH "/**\r\n";
  printf FH " *  \@file   %s\r\n", $sort_aiueo_idx_header_file_name;
  printf FH " *  \@brief  %sのインデックス\r\n", $sort_aiueo_file_name;
  printf FH " *  \@author zkn_sort_personal_make.pl\r\n";
  printf FH " *  \@data   \r\n";
  printf FH " *  \@note   zkn_sort_personal_make.plで生成されました。\r\n";
  printf FH " *\r\n";
  printf FH " *  モジュール名：ZKN_SORT_AIUEO_IDX\r\n";
  printf FH " */\r\n";
  printf FH "//============================================================================\r\n";
  printf FH "#pragma once\r\n";
  printf FH "\r\n";  # 0D 0A

  # なし
  printf FH "#define %s  (0x%X)\r\n", $aiueo_none_define, $sort_aiueo_idx_none;  # 0D 0A
  printf FH "\r\n";  # 0D 0A
  
  # a i u e o ka ki ku ke ko ...
  for( my $i=0; $i<$key_num; $i++ )
  {
    my $k = $aiueo_start_idx_tbl_key[$i];
    my $str_start  = $aiueo_none_define;
    my $str_end    = $aiueo_none_define;
    if( $aiueo_start_idx[$i] != $sort_aiueo_idx_none )
    {
      $str_start = sprintf( "%d", $aiueo_start_idx[$i] );
    }
    if( $aiueo_end_idx[$i] != $sort_aiueo_idx_none )
    {
      $str_end = sprintf( "%d", $aiueo_end_idx[$i] );
    }
    printf FH "#define %s%s  (%s)\r\n", $aiueo_start_idx_define_prefix,$aiueo_romaji_tbl{$k},$str_start;  # 0D 0A
    printf FH "#define %s%s    (%s)\r\n", $aiueo_end_idx_define_prefix,$aiueo_romaji_tbl{$k},$str_end;  # 0D 0A
  }
  printf FH "\r\n";  # 0D 0A
  
  # 終わり
  printf FH "#define %s  (%d)\r\n", $aiueo_all_end_define, $mons_num;  # 0D 0A
  
  close( FH );
}

