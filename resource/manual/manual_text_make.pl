##============================================================================
=pod
     file   manual_text_make.pl
     brief  ゲーム内マニュアルのテキストのgmmファイルを生成する
            他にもいろいろ生成する
     author Koji Kawada
     date   2010.05.12
     note   perl manual_text_make.pl manual.xls w
     note   perl manual_text_make.pl manual.xls b 

     モジュール名：
=cut 
##============================================================================


$debug_out = 1;  # これが0のときデバッグ出力なし。これが1のときデバッグ出力あり。


##=============================================================================
=pod

********  gmmファイル中の並び  ********

カテゴリ番号1のカテゴリ名(man_txt_cate_no_001)
カテゴリ番号2のカテゴリ名(man_txt_cate_no_002)
...
カテゴリ番号Nのカテゴリ名(man_txt_cate_no_N)

タイトル番号1-1のタイトル名(カテゴリ番号-カテゴリ内タイトル番号)(man_txt_title_no_001_001)
タイトル番号1-2のタイトル名(カテゴリ番号-カテゴリ内タイトル番号)(man_txt_title_no_001_002)
...
タイトル番号2-1のタイトル名(カテゴリ番号-カテゴリ内タイトル番号)(man_txt_title_no_002_001)

タイトル番号1-1の説明1ページ目(man_txt_page_no_001_001_001)
タイトル番号1-1の説明2ページ目(man_txt_page_no_001_001_002)
...
タイトル番号1-1の説明Nページ目(man_txt_page_no_001_001_N)

タイトル番号1-2の説明1ページ目(man_txt_page_no_001_002_001)
タイトル番号1-2の説明2ページ目(man_txt_page_no_001_002_002)
...
タイトル番号1-2の説明Nページ目(man_txt_page_no_001_002_N)

タイトル番号2-1の説明1ページ目(man_txt_page_no_002_001_001)
タイトル番号2-1の説明2ページ目(man_txt_page_no_002_001_002)
...
タイトル番号2-1の説明Nページ目(man_txt_page_no_002_001_N)

********  ********

********  タイトルファイルの中身  ********

↓をタイトル数だけ用意しておく
↓は通し番号順に並んでいる
{
  カテゴリ番号(u16)
  タイトル番号(u16)
  通し番号(u16)
  タイトルのgmmのID(u16)
  オープンフラグ番号(u16)
  リードフラグ番号(u16)
  ページ数(u16)
  ページ0のgmmのID(u16)←ページ数分だけしか用意しないようにするので、
  ページ0の画像のID(u16)←タイトルごとに1塊のサイズが異なることになる。
  ページ1のgmmのID(u16)
  ページ1の画像のID(u16)
  ページ2のgmmのID(u16)
  ページ2の画像のID(u16)
  ページ3のgmmのID(u16)
  ページ3の画像のID(u16)
  ページ4のgmmのID(u16)
  ページ4の画像のID(u16)
  ページ5のgmmのID(u16)
  ページ5の画像のID(u16)
  ページ6のgmmのID(u16)
  ページ6の画像のID(u16)
  ページ7のgmmのID(u16)
  ページ7の画像のID(u16)
  ページ8のgmmのID(u16)
  ページ8の画像のID(u16)
}
{
  ...
}
{
  ...
}

********  ********

********  タイトル開始場所ファイルの中身  ********

総タイトル数(u16)
タイトル0の開始位置(u16)←タイトルファイルのバイト位置
タイトル1の開始位置(u16)
...
タイトル[総タイトル数-1]の開始位置(u16)

********  ********

********  カテゴリファイルの中身  ********

↓をカテゴリ数だけ用意しておく
↓はカテゴリ番号順に並んでいる
{
  カテゴリ番号(u16)
  カテゴリのgmmのID(u16)
  タイトル数(u16)
  タイトルのインデックス(u16)←タイトル開始場所ファイルから参照すれば、タイトルへ到達できるように
  タイトルのインデックス(u16)←タイトル数分だけしか用意しないようにするので、
  タイトルのインデックス(u16)←カテゴリごとに1塊のサイズが異なることになる。
  ...
  タイトルのインデックス(u16)
}
{
  ...
}
{
  ...
}

********  ********

********  カテゴリ開始場所ファイルの中身  ********

総カテゴリ数(u16)
カテゴリ0の開始位置(u16)←カテゴリファイルのバイト位置
カテゴリ1の開始位置(u16)
...
カテゴリ[総カテゴリ数-1]の開始位置(u16)

********  ********

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
$manual_xls_file_name   = $ARGV[0];  # ????.xls  # shiftjis
$manual_make_version    = $ARGV[1];  # w or b

# ツール
$excel_converter_file_name      = $ENV{"PROJECT_ROOT"}."/tools/exceltool/ExcelSeetConv.exe";            # Excelコンバータ

# manual_xls_file_nameのシート
$manual_sheet_name      = "manual";
$read_flag_sheet_name   = "read_flag";

$manual_csv_file_name_shiftjis      = "manual_shiftjis.csv";     # shiftjis
$read_flag_csv_file_name_shiftjis   = "read_flag_shiftjis.csv";  # shiftjis

$manual_csv_file_name      = "manual.csv";     # utf8 
$read_flag_csv_file_name   = "read_flag.csv";  # utf8

# gmm雛形ファイル
$gmm_header_file_name   = "header.file";  # utf8
$gmm_footer_file_name   = "footer.file";  # utf8

# 出力ファイル
# バージョン
$manual_version_w  = "w";
$manual_version_b  = "b";

# gmm
$manual_text_gmm_file_name       = "manual_text_";    # utf8
$manual_text_gmm_file_name_ext   = ".gmm";            # manual_text_w.gmm or manual_text_b.gmm

$manual_text_gmm_row_id_name_cate     = "man_txt_cate_no_";   # 例：man_txt_cate_no_002
$manual_text_gmm_row_id_name_title    = "man_txt_title_no_";  # 例：man_txt_title_no_001_002
$manual_text_gmm_row_id_name_explain  = "man_txt_page_no_";   # 例：man_txt_page_no_001_001_002

# 画像ファイルのリスト
$manual_image_src_list_file_name             = "manual_image_src_";   # 画像ファイルのリスト      # shiftjisでもutf8でも同じ
$manual_image_src_list_file_name_ext         = ".lst";                # manual_image_src_w.lst or manual_image_src_b.lst
$manual_image_conv_bat_file_name             = "manual_image_conv_";  # コンバートバッチファイル  # shiftjisでもutf8でも同じ
$manual_image_conv_bat_file_name_ext         = ".bat";                # manual_image_conv_w.bat or manual_image_conv_b.bat
$manual_image_comp_bat_file_name             = "manual_image_comp_";  # 圧縮バッチファイル        # shiftjisでもutf8でも同じ
$manual_image_comp_bat_file_name_ext         = ".bat";                # manual_image_comp_w.bat or manual_image_comp_b.bat
$manual_image_lz77_nbfs_list_file_name       = "manual_image_";       # 圧縮後のファイルのリスト  # shiftjisでもutf8でも同じ
$manual_image_lz77_nbfs_list_file_name_ext   = ".lst";                # manual_image_w.lst or manual_image_b.lst

$manual_image_src_suffix            = ".bmp";
$manual_image_nbfs_suffix           = ".nbfs";
$manual_image_lz77_nbfs_suffix      = "_lz77.nbfs";

$manual_image_conv_command_name     = "ntexconv";
$manual_image_conv_command_option   = "-no -bg -bgb -bgf direct";
$manual_image_comp_command_name     = "ntrcomp";
$manual_image_comp_command_option   = "-l -o";


# 予約語
$manual_csv_reserve_image_none   = "none";    # 画像ファイルなしのとき

$manual_reserve_open_flag_num    = 3;  # オープンフラグ
$manual_reserve_open_flag_col_open_flag_name   = 0;
$manual_reserve_open_flag_col_open_flag_no     = 1;
$manual_reserve_open_flag_col_open_flag_enum   = 2;

@manual_reserve_open_flag_tbl =
(
  # open_flag_name, open_flag_no, open_flag_enum
  [ "最初",         0,            "START"        ],
  [ "シイギア",     1,            "CGERA"        ],
  [ "エヌクリア",   2,            "NCLEAR"       ],
);

%manual_reserve_open_flag_tbl_idx_hash_from_open_flag_name = ();  # open_flag名から@manual_open_flag_tblのインデックスを得るためのハッシュ
# open_flag名からopen_flag番号を得るには
# open_flag番号 = $manual_reserve_open_flag_tbl[ $manual_reserve_open_flag_tbl_idx_hash_from_open_flag_name{open_flag名} ][$manual_reserve_open_flag_col_open_flag_no];
# となる。
%manual_reserve_open_flag_no_hash_from_open_flag_name = ();  # open_flag名からopen_flag番号を得るためのハッシュ
$manual_reserve_open_flag_enum_prefix       = "MANUAL_OPEN_FLAG_";
$manual_reserve_open_flag_enum_file_name    = "manual_open_flag.h";  # shiftjisでもutf8でも同じ

$manual_csv_reserve_version_w = "ホワイト";
$manual_csv_reserve_version_b = "ブラック";


# 限界値
$manual_page_max_limit = 9;  # 9ページ(含む)までならOK


# csvファイル共通のラベル名
$csv_data_start_label_name   = "data_start";
$csv_data_end_label_name     = "data_end";

# manual_csv_file_nameのラベル名と列番号(column)(0スタート)
$manual_csv_cate_no_label_name          = "cate_no";
$manual_csv_cate_name_hira_label_name   = "cate_name_hira";
$manual_csv_cate_name_kanji_label_name  = "cate_name_kanji";
$manual_csv_title_no_label_name         = "title_no";
$manual_csv_title_name_hira_label_name  = "title_name_hira";
$manual_csv_title_name_kanji_label_name = "title_name_kanji";
$manual_csv_page_max_label_name         = "page_max";
$manual_csv_page_no_label_name          = "page_no";
$manual_csv_explain_hira_label_name     = "explain_hira";
$manual_csv_explain_kanji_label_name    = "explain_kanji";
$manual_csv_image_label_name            = "image";
$manual_csv_open_flag_label_name        = "open_flag";
$manual_csv_read_flag_name_label_name   = "read_flag_name";
$manual_csv_serial_no_label_name        = "serial_no";
$manual_csv_version_label_name          = "version";

$manual_csv_cate_no_col;
$manual_csv_cate_name_hira_col;
$manual_csv_cate_name_kanji_col;
$manual_csv_title_no_col;
$manual_csv_title_name_hira_col;
$manual_csv_title_name_kanji_col;
$manual_csv_page_max_col;
$manual_csv_page_no_col;
$manual_csv_explain_hira_col;
$manual_csv_explain_kanji_col;
$manual_csv_image_col;
$manual_csv_open_flag_col;
$manual_csv_read_flag_name_col;
$manual_csv_serial_no_col;
$manual_csv_version_col;

# manual_cate
$manual_cate_num;
$manual_cate_first_num;                 # $manual_cate_col_firstが1のものの個数
@manual_cate_tbl = ();
$manual_cate_col_cate_no              = 0;
$manual_cate_col_cate_name_hira       = 1;
$manual_cate_col_cate_name_kanji      = 2;
$manual_cate_col_cate_name_gmm_row_id = 3;
$manual_cate_col_title_num            = 4;  # このカテゴリに属するタイトルの個数
$manual_cate_col_first                = 5;  # 初登場なら1、2回目以降なら0
$manual_cate_col_max                  = 6;
#%manual_cate_tbl_idx_hash_from_cate_name_hira = ();
%manual_cate_tbl_idx_hash_from_cate_no        = ();

# manual_title
$manual_title_num;
$manual_title_first_num;                 # $manual_title_col_firstが1のものの個数
@manual_title_tbl = ();
$manual_title_col_cate_no               = 0;
$manual_title_col_title_no              = 1;
$manual_title_col_title_name_hira       = 2;
$manual_title_col_title_name_kanji      = 3;
$manual_title_col_title_name_gmm_row_id = 4;
$manual_title_col_page_max              = 5;
$manual_title_col_open_flag             = 6;
$manual_title_col_read_flag_name        = 7;
$manual_title_col_serial_no             = 8;
$manual_title_col_data_idx              = 9;   # タイトル開始場所ファイルの何番目のデータ(先頭の総タイトル数のところを除いて何番目)(0番スタート)を参照すればいいか
$manual_title_col_first                 = 10;  # 初登場なら1、2回目以降なら0
$manual_title_col_max                   = 11;
#%manual_title_tbl_idx_hash_from_title_name_hira   = ();
%manual_title_tbl_idx_hash_from_cate_no_title_no  = ();  # cate_no * $manual_title_hash_cate_no_times + title_no
$manual_title_hash_cate_no_times        = 1000;

# manual_explain
$manual_explain_num;
$manual_explain_first_num;                 # $manual_explain_col_firstが1のものの個数
@manual_explain_tbl = ();
$manual_explain_col_cate_no              = 0;
$manual_explain_col_title_no             = 1;
$manual_explain_col_page_no              = 2;
$manual_explain_col_explain_hira         = 3;
$manual_explain_col_explain_kanji        = 4;
$manual_explain_col_explain_gmm_row_id   = 5;
$manual_explain_col_image                = 6;
$manual_explain_col_image_arc_id         = 7;
$manual_explain_col_first                = 8;  # 初登場なら1、2回目以降なら0
$manual_explain_col_max                  = 9;
%manual_explain_tbl_idx_hash_from_cate_no_title_no_page_no  = ();  # cate_no * $manual_explain_hash_cate_no_times + title_no * $manual_explain_hash_title_no_times + page_no
$manual_explain_hash_cate_no_times       = 1000*1000;
$manual_explain_hash_title_no_times      = 1000;
%manual_explain_image_arc_id_hash_from_image = ();  # 同じ名前の画像ファイルは同じarc_idになるようにしておく

# manual_invalid  # 初期化用の不正な値         # 正しい値
$manual_invalid_cate_no           = 0;         # 1<=
$manual_invalid_cate_name_hira    = "";        # 空でない
$manual_invalid_cate_name_kanji   = "";        # 空でない
$manual_invalid_title_no          = 0;         # 1<=
$manual_invalid_title_name_hira   = "";        # 空でない
$manual_invalid_title_name_kanji  = "";        # 空でない
$manual_invalid_page_max          = 0;         # 1<=
$manual_invalid_open_flag         = 256;       # <=255
$manual_invalid_read_flag_name    = "";        # 空でない
$manual_invalid_serial_no         = 0;         # 1<=

# read_flag_csv_file_nameのラベル名と列番号(column)(0スタート)
$read_flag_csv_read_flag_name_label_name   = "read_flag_name";
$read_flag_csv_read_flag_no_label_name     = "read_flag_no";

$read_flag_csv_read_flag_name_col;
$read_flag_csv_read_flag_no_col;

# read_flag
$read_flag_num;
@read_flag_tbl = ();
$read_flag_col_read_flag_name   = 0;
$read_flag_col_read_flag_no     = 1;
$read_flag_col_max              = 2;
%read_flag_tbl_idx_hash_from_read_flag_name = ();  # read_flag_nameから@read_flag_tblのインデックスを得るためのハッシュ
                                                   # つまり
                                                   # $read_flag_tbl[$i][$read_flag_col_read_flag_name] = "リードフラグ名";
                                                   # $read_flag_tbl_idx_hash{ "リードフラグ名" } = $i;
                                                   # が成り立つ。
%read_flag_tbl_idx_hash_from_read_flag_no   = ();  # read_flag_noから@read_flag_tblのインデックスを得るためのハッシュ


# 最終データ
# 定義ファイル
$manual_data_define_file_name      = "manual_data_def_";  # shiftjisでもutf8でも同じ
$manual_data_define_file_name_ext  = ".h";                # manual_data_def_w.h or manual_data_def_b.h

# 定数
$manual_data_gmm_none_no       = 0xFFFF;                    # テキストなしなのでgmmファイル中のIDなし
$manual_data_gmm_none_name     = "MANUAL_DATA_GMM_NONE";
$manual_data_image_none_no     = 0xFFFF;                    # 画像ファイルなしなのでnaix中のIDなし
$manual_data_image_none_name   = "MANUAL_DATA_IMAGE_NONE";

# 画像がないときに使用する画像のID
$manual_no_image_arc_id;  # 画像の1番最後に追加する(即ち画像の総数をIDとする)
$manual_no_image_name          = "MANUAL_DATA_NO_IMAGE_ID";
$manual_no_image_file_name     = "bg.bmp";

# タイトルファイル
$manual_data_title_file_name           = "manual_data_title_";        # リトルエンディアンのバイナリ
$manual_data_title_file_name_ext       = ".dat";                      # manual_data_title_w.dat or manual_data_title_b.dat
# タイトル開始場所ファイル
$manual_data_title_ref_file_name       = "manual_data_title_ref_";    # リトルエンディアンのバイナリ
$manual_data_title_ref_file_name_ext   = ".dat";                      # manual_data_title_ref_w.dat or manual_data_title_ref_b.dat
# カテゴリファイル
$manual_data_cate_file_name            = "manual_data_cate_";         # リトルエンディアンのバイナリ
$manual_data_cate_file_name_ext        = ".dat";                      # manual_data_cate_w.dat or manual_data_cate_b.dat
# カテゴリ開始場所ファイル
$manual_data_cate_ref_file_name        = "manual_data_cate_ref_";     # リトルエンディアンのバイナリ
$manual_data_cate_ref_file_name_ext    = ".dat";                      # manual_data_cate_ref_w.dat or manual_data_cate_ref_b.dat


#if( $debug_out == 1 ) ↓
# タイトルファイル
$debug_manual_data_title_file_name           = "debug_manual_data_title_";      # shiftjisでもutf8でも同じ
$debug_manual_data_title_file_name_ext       = ".csv";                          # debug_manual_data_title_w.csv or debug_manual_data_title_b.csv
# タイトル開始場所ファイル
$debug_manual_data_title_ref_file_name       = "debug_manual_data_title_ref_";  # shiftjisでもutf8でも同じ
$debug_manual_data_title_ref_file_name_ext   = ".csv";                          # debug_manual_data_title_ref_w.csv or debug_manual_data_title_ref_b.csv
# カテゴリファイル
$debug_manual_data_cate_file_name            = "debug_manual_data_cate_";       # shiftjisでもutf8でも同じ
$debug_manual_data_cate_file_name_ext        = ".csv";                          # debug_manual_data_cate_w.csv or debug_manual_data_cate_b.csv
# カテゴリ開始場所ファイル
$debug_manual_data_cate_ref_file_name        = "debug_manual_data_cate_ref_";   # shiftjisでもutf8でも同じ
$debug_manual_data_cate_ref_file_name_ext    = ".csv";                          # debug_manual_data_cate_ref_w.csv or debug_manual_data_cate_ref_b.csv
#if( $debug_out == 1 ) ↑


##=============================================================================
=pod
    メイン処理
=cut
##=============================================================================
# Excelファイルをcsvファイルにコンバートする
&ConvertExcelToCsv( $manual_xls_file_name, $manual_sheet_name, $manual_csv_file_name_shiftjis );
&ConvertExcelToCsv( $manual_xls_file_name, $read_flag_sheet_name, $read_flag_csv_file_name_shiftjis );

# UTF8だけで作業できるようにする
&EncodeFileFromShiftjisToUtf8( $manual_csv_file_name_shiftjis, $manual_csv_file_name );
&EncodeFileFromShiftjisToUtf8( $read_flag_csv_file_name_shiftjis, $read_flag_csv_file_name );

# 予約語オープンフラグのテーブルを作成する
&MakeManualOpenFlagTable();

# read_flagのテーブルやハッシュをつくる
&ReadReadFlagCsvFile();

# manualのテーブルをつくる
&ReadManualCsvFile();

# manualのハッシュをつくり、初登場か否か調べる
&CheckManualTbl();

# gmmファイルを出力する
&WriteManualTextGmmFile();

# 画像ファイルのコンバートバッチファイルを作成する
&WriteManualImageListFile();

# 最終データを出力する
&WriteDataFile();

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
### Excelファイルをcsvファイルにコンバートする
##=====================================
sub ConvertExcelToCsv
{
  my( $xls_file_name, $xls_sheet_name, $csv_file_name ) = @_;
  system( $excel_converter_file_name.' -o '.$csv_file_name.' -n '.$xls_sheet_name.' -s csv '.$xls_file_name );
}

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
    #$line = Encode::decode( 'shiftjis', $_ );
    $line = Encode::decode( 'cp932', $_ );  # WindowsのShiftJISはcp932。これにしておかないとハイフンとかチルダがうまくいかないらしい。
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
  unlink( $manual_csv_file_name_shiftjis );
  unlink( $read_flag_csv_file_name_shiftjis );
  
  unlink( $manual_csv_file_name );
  unlink( $read_flag_csv_file_name );
}

##-------------------------------------
### read_flagのテーブルやハッシュをつくる
##=====================================
sub ReadReadFlagCsvFile
{
  open( DATA, "<:encoding(utf8)", $read_flag_csv_file_name );

  my $row_no          = 0;  # csvの行番号(0スタート)
  my $col_no;               # csvの列番号(0スタート)
  my $idx             = 0;  # データとしてテーブルに保持されたときのテーブルのインデックス

  my $is_read_row     = 0;  # 0のとき行を読む必要はない。1のとき行を読む必要がある。$data_start_row<行<$data_end_rowを満たす行なら読む必要がある。
  my $is_read_col     = 0;  # 0のとき列を読む必要はない。1のとき列を読む必要がある。$data_start_col<列<$data_end_colを満たす列なら読む必要がある。
  my $data_start_row;
  my $data_end_row;
  my $data_start_col;
  my $data_end_col;


  # 値に改行コードを含む CSV形式を扱う

  while (my $line = <DATA>) {
    $line .= <DATA> while ($line =~ tr/"// % 2 and !eof(DATA));

    $line =~ s/(?:\x0D\x0A|[\x0D\x0A])?$/,/;
    @values = map {/^"(.*)"$/s ? scalar($_ = $1, s/""/"/g, $_) : $_}
                  ($line =~ /("[^"]*(?:""[^"]*)*"|[^,]*),/g);

    # @values を処理する
   

    $col_no = 0;
    if( $is_read_row == 0 )
    {
      foreach my $word ( @values )
      {
        if( $is_read_col == 0 )
        {
          if( $word eq $csv_data_start_label_name )
          {
            $data_start_row = $row_no;
            $data_start_col = $col_no;
            $is_read_col = 1;
          }
        }
        else
        {
          if( $word eq $csv_data_end_label_name )
          {
            $is_read_row = 1;
            $is_read_col = 0;
            $data_end_col = $col_no;
            last;
          }
          elsif( $word eq $read_flag_csv_read_flag_name_label_name )
          {
            $read_flag_csv_read_flag_name_col = $col_no;
          }
          elsif( $word eq $read_flag_csv_read_flag_no_label_name )
          {
            $read_flag_csv_read_flag_no_col = $col_no;
          }
        }
        $col_no++;
      }
    }
    else
    {
      if( $values[$data_start_col] eq $csv_data_end_label_name )
      {
        $is_read_row = 0;
        $data_end_row = $row_no;
        last;
      }
      else
      {
        $read_flag_tbl[$idx][$read_flag_col_read_flag_name] = $values[$read_flag_csv_read_flag_name_col];
        $read_flag_tbl[$idx][$read_flag_col_read_flag_no]   = $values[$read_flag_csv_read_flag_no_col];
        
        if( defined( $read_flag_tbl_idx_hash_from_read_flag_name{ $values[$read_flag_csv_read_flag_name_col] } ) )
        {
          # 既に定義されているリードフラグ名
          die "read_flag_name \"$values[$read_flag_csv_read_flag_name_col]\" error, stopped";
        }
        if( defined( $read_flag_tbl_idx_hash_from_read_flag_no{ $values[$read_flag_csv_read_flag_no_col] } ) )
        {
          # 既に定義されているリードフラグ番号
          die "read_flag_no \"$values[$read_flag_csv_read_flag_no_col]\" error, stopped";
        }
        
        $read_flag_tbl_idx_hash_from_read_flag_name{ $values[$read_flag_csv_read_flag_name_col] } = $idx;
        $read_flag_tbl_idx_hash_from_read_flag_no{ $values[$read_flag_csv_read_flag_name_col] } = $idx;
        $idx++;
      }
    }
    $row_no++;


  }


  $read_flag_num = $idx;


  close( DATA );


#=pod
  # 確認
  for( my $i=0; $i<$read_flag_num; $i++ )
  {
    for( my $j=0; $j<$read_flag_col_max; $j++ )
    {
      print "$read_flag_tbl[$i][$j]".", ";
    }
    print "\r\n";
  }
#=cut
}

##-------------------------------------
### manualのテーブルをつくる
##=====================================
sub ReadManualCsvFile
{
  open( DATA, "<:encoding(utf8)", $manual_csv_file_name );

  my $row_no          = 0;  # csvの行番号(0スタート)
  my $col_no;               # csvの列番号(0スタート)
  my $cate_idx        = 0;  # データとしてテーブルに保持されたときのテーブルのインデックス
  my $title_idx       = 0;  # データとしてテーブルに保持されたときのテーブルのインデックス
  my $explain_idx     = 0;  # データとしてテーブルに保持されたときのテーブルのインデックス

  my $is_read_row     = 0;  # 0のとき行を読む必要はない。1のとき行を読む必要がある。$data_start_row<行<$data_end_rowを満たす行なら読む必要がある。
  my $is_read_col     = 0;  # 0のとき列を読む必要はない。1のとき列を読む必要がある。$data_start_col<列<$data_end_colを満たす列なら読む必要がある。
  my $data_start_row;
  my $data_end_row;
  my $data_start_col;
  my $data_end_col;

  # 空欄の場合は最後に書かれていた値が採用されるので、最後に書かれていた値を保持しておく
  my $curr_cate_no           = $manual_invalid_cate_no;           # 不正な値で初期化しておく
  my $curr_cate_name_hira    = $manual_invalid_cate_name_hira;
  my $curr_cate_name_kanji   = $manual_invalid_cate_name_kanji;
  my $curr_title_no          = $manual_invalid_title_no;
  my $curr_title_name_hira   = $manual_invalid_title_name_hira;
  my $curr_title_name_kanji  = $manual_invalid_title_name_kanji;
  my $curr_page_max          = $manual_invalid_page_max;
  my $curr_open_flag         = $manual_invalid_open_flag;
  my $curr_read_flag_name    = $manual_invalid_read_flag_name;
  my $curr_serial_no         = $manual_invalid_serial_no;
  # versionに関しては最後に書かれていた値は採用されず、毎行の値を確認する


  # 値に改行コードを含む CSV形式を扱う

  while (my $line = <DATA>) {
    $line .= <DATA> while ($line =~ tr/"// % 2 and !eof(DATA));

    $line =~ s/(?:\x0D\x0A|[\x0D\x0A])?$/,/;
    @values = map {/^"(.*)"$/s ? scalar($_ = $1, s/""/"/g, $_) : $_}
                  ($line =~ /("[^"]*(?:""[^"]*)*"|[^,]*),/g);

    # @values を処理する
   

    $col_no = 0;
    if( $is_read_row == 0 )
    {
      foreach my $word ( @values )
      {
        if( $is_read_col == 0 )
        {
          if( $word eq $csv_data_start_label_name )
          {
            $data_start_row = $row_no;
            $data_start_col = $col_no;
            $is_read_col = 1;
          }
        }
        else
        {
          if( $word eq $csv_data_end_label_name )
          {
            $is_read_row = 1;
            $is_read_col = 0;
            $data_end_col = $col_no;
            last;
          }
          elsif( $word eq $manual_csv_cate_no_label_name          )
          {
            $manual_csv_cate_no_col = $col_no; 
          }
          elsif( $word eq $manual_csv_cate_name_hira_label_name   )
          {
            $manual_csv_cate_name_hira_col = $col_no; 
          }
          elsif( $word eq $manual_csv_cate_name_kanji_label_name  )
          {
            $manual_csv_cate_name_kanji_col = $col_no; 
          }
          elsif( $word eq $manual_csv_title_no_label_name         )
          {
            $manual_csv_title_no_col = $col_no; 
          }
          elsif( $word eq $manual_csv_title_name_hira_label_name  )
          {
            $manual_csv_title_name_hira_col = $col_no; 
          }
          elsif( $word eq $manual_csv_title_name_kanji_label_name )
          {
            $manual_csv_title_name_kanji_col = $col_no; 
          }
          elsif( $word eq $manual_csv_page_max_label_name         )
          {
            $manual_csv_page_max_col = $col_no; 
          }
          elsif( $word eq $manual_csv_page_no_label_name          )
          {
            $manual_csv_page_no_col = $col_no; 
          }
          elsif( $word eq $manual_csv_explain_hira_label_name     )
          {
            $manual_csv_explain_hira_col = $col_no; 
          }
          elsif( $word eq $manual_csv_explain_kanji_label_name    )
          {
            $manual_csv_explain_kanji_col = $col_no; 
          }
          elsif( $word eq $manual_csv_image_label_name            )
          {
            $manual_csv_image_col = $col_no; 
          }
          elsif( $word eq $manual_csv_open_flag_label_name        )
          {
            $manual_csv_open_flag_col = $col_no; 
          }
          elsif( $word eq $manual_csv_read_flag_name_label_name   )
          {
            $manual_csv_read_flag_name_col = $col_no; 
          }
          elsif( $word eq $manual_csv_serial_no_label_name        )
          {
            $manual_csv_serial_no_col = $col_no; 
          }
          elsif( $word eq $manual_csv_version_label_name        )
          {
            $manual_csv_version_col = $col_no; 
          }
        }
        $col_no++;
      }
    }
    else
    {
      if( $values[$data_start_col] eq $csv_data_end_label_name )
      {
        $is_read_row = 0;
        $data_end_row = $row_no;
        last;
      }
      else
      {
        # version
        if( $manual_make_version eq $manual_version_w )  # ホワイト
        {
          if( $values[$manual_csv_version_col] ne "" && $values[$manual_csv_version_col] ne $manual_csv_reserve_version_w )
          {
            next;  # バージョン違いなので、この行は飛ばす
          }
        }
        elsif( $manual_make_version eq $manual_version_b )  # ブラック
        {
          if( $values[$manual_csv_version_col] ne "" && $values[$manual_csv_version_col] ne $manual_csv_reserve_version_b )
          {
            next;  # バージョン違いなので、この行は飛ばす
          }
        }
        else  # バージョン不明
        {
          die "manual_make_version \"$manual_make_version\" error, stopped";
          next;
        }

        # manual_cate
        if( $values[$manual_csv_cate_no_col] eq "" )
        {
          if( $curr_cate_no == $manual_invalid_cate_no )
          {
            # 一度も正しい値が出現しておらず、不正な値のまま
            die "curr_cate_no \"$curr_cate_no\" error, stopped";
          }
          else
          {
            $values[$manual_csv_cate_no_col] = $curr_cate_no;
          }
        }
        else
        {
          $curr_cate_no = $values[$manual_csv_cate_no_col];
        }
        $manual_cate_tbl[$cate_idx][$manual_cate_col_cate_no] = $values[$manual_csv_cate_no_col];
        
        # ここから先のエラーチェックは省略。後日余力があるときに記述する予定。

        if( $values[$manual_csv_cate_name_hira_col] eq "" )
        {
          $values[$manual_csv_cate_name_hira_col] = $curr_cate_name_hira;
        }
        $curr_cate_name_hira = $values[$manual_csv_cate_name_hira_col];
        $manual_cate_tbl[$cate_idx][$manual_cate_col_cate_name_hira] = $values[$manual_csv_cate_name_hira_col];

        if( $values[$manual_csv_cate_name_kanji_col] eq "" )
        {
          $values[$manual_csv_cate_name_kanji_col] = $curr_cate_name_kanji;
        }
        $curr_cate_name_kanji = $values[$manual_csv_cate_name_kanji_col];
        $manual_cate_tbl[$cate_idx][$manual_cate_col_cate_name_kanji] = $values[$manual_csv_cate_name_kanji_col];

        # manual_title
        $manual_title_tbl[$title_idx][$manual_title_col_cate_no] = $manual_cate_tbl[$cate_idx][$manual_cate_col_cate_no];
        
        if( $values[$manual_csv_title_no_col] eq "" )
        {
          $values[$manual_csv_title_no_col] = $curr_title_no;
        }
        $curr_title_no = $values[$manual_csv_title_no_col];
        $manual_title_tbl[$title_idx][$manual_title_col_title_no] = $values[$manual_csv_title_no_col];
 
        if( $values[$manual_csv_title_name_hira_col] eq "" )
        {
          $values[$manual_csv_title_name_hira_col] = $curr_title_name_hira;
        }
        $curr_title_name_hira = $values[$manual_csv_title_name_hira_col];
        $manual_title_tbl[$title_idx][$manual_title_col_title_name_hira] = $values[$manual_csv_title_name_hira_col];
 
        if( $values[$manual_csv_title_name_kanji_col] eq "" )
        {
          $values[$manual_csv_title_name_kanji_col] = $curr_title_name_kanji;
        }
        $curr_title_name_kanji = $values[$manual_csv_title_name_kanji_col];
        $manual_title_tbl[$title_idx][$manual_title_col_title_name_kanji] = $values[$manual_csv_title_name_kanji_col];
 
        if( $values[$manual_csv_page_max_col] eq "" )
        {
          $values[$manual_csv_page_max_col] = $curr_page_max;
        }
        $curr_page_max = $values[$manual_csv_page_max_col];
        if( $values[$manual_csv_page_max_col] > $manual_page_max_limit )
        {
          # 限界地を越えているので
          die "page_max \"$values[$manual_csv_page_max_col]\" error, stopped";
        }
        $manual_title_tbl[$title_idx][$manual_title_col_page_max] = $values[$manual_csv_page_max_col];
 
        if( $values[$manual_csv_open_flag_col] eq "" )
        {
          $values[$manual_csv_open_flag_col] = $curr_open_flag;
        }
        $curr_open_flag = $values[$manual_csv_open_flag_col];
        $manual_title_tbl[$title_idx][$manual_title_col_open_flag] = $values[$manual_csv_open_flag_col];
 
        if( $values[$manual_csv_read_flag_name_col] eq "" )
        {
          $values[$manual_csv_read_flag_name_col] = $curr_read_flag_name;
        }
        $curr_read_flag_name = $values[$manual_csv_read_flag_name_col];
        $manual_title_tbl[$title_idx][$manual_title_col_read_flag_name] = $values[$manual_csv_read_flag_name_col];
 
        if( $values[$manual_csv_serial_no_col] eq "" )
        {
          $values[$manual_csv_serial_no_col] = $curr_serial_no;
        }
        $curr_serial_no = $values[$manual_csv_serial_no_col];
        $manual_title_tbl[$title_idx][$manual_title_col_serial_no] = $values[$manual_csv_serial_no_col];
 
        # manual_explain
        $manual_explain_tbl[$explain_idx][$manual_explain_col_cate_no]  = $manual_cate_tbl[$cate_idx][$manual_cate_col_cate_no];
        $manual_explain_tbl[$explain_idx][$manual_explain_col_title_no] = $manual_title_tbl[$title_idx][$manual_title_col_title_no];

        if( $values[$manual_csv_page_no_col] eq "" )
        {
          $values[$manual_csv_page_no_col] = $curr_page_no;
        }
        $curr_page_no = $values[$manual_csv_page_no_col];
        $manual_explain_tbl[$explain_idx][$manual_explain_col_page_no] = $values[$manual_csv_page_no_col];
 
        if( $values[$manual_csv_explain_hira_col] eq "" )
        {
          $values[$manual_csv_explain_hira_col] = $curr_explain_hira;
        }
        $curr_explain_hira = $values[$manual_csv_explain_hira_col];
        $manual_explain_tbl[$explain_idx][$manual_explain_col_explain_hira] = $values[$manual_csv_explain_hira_col];
 
        if( $values[$manual_csv_explain_kanji_col] eq "" )
        {
          $values[$manual_csv_explain_kanji_col] = $curr_explain_kanji;
        }
        $curr_explain_kanji = $values[$manual_csv_explain_kanji_col];
        $manual_explain_tbl[$explain_idx][$manual_explain_col_explain_kanji] = $values[$manual_csv_explain_kanji_col];
 
        if( $values[$manual_csv_image_col] eq "" )
        {
          $values[$manual_csv_image_col] = $curr_image;
        }
        $curr_image = $values[$manual_csv_image_col];
        $manual_explain_tbl[$explain_idx][$manual_explain_col_image] = $values[$manual_csv_image_col];
 
        $cate_idx++;
        $title_idx++;
        $explain_idx++;
      }
    }
    $row_no++;


  }


  $manual_cate_num    = $cate_idx;
  $manual_title_num   = $title_idx;
  $manual_explain_num = $explain_idx;


  close( DATA );


#=pod
  # 確認
  for( my $i=0; $i<$manual_cate_num; $i++ )
  {
    for( my $j=0; $j<$manual_cate_col_max; $j++ )
    {
      print "$manual_cate_tbl[$i][$j]".", ";
    }
    print "\r\n";
  }

  for( my $i=0; $i<$manual_title_num; $i++ )
  {
    for( my $j=0; $j<$manual_title_col_max; $j++ )
    {
      print "$manual_title_tbl[$i][$j]".", ";
    }
    print "\r\n";
  }

  for( my $i=0; $i<$manual_explain_num; $i++ )
  {
    for( my $j=0; $j<$manual_explain_col_max; $j++ )
    {
      print "$manual_explain_tbl[$i][$j]".", ";
    }
    print "\r\n";
  }
#=cut
}

##-------------------------------------
### manualのハッシュをつくり、初登場か否か調べる
##=====================================
sub CheckManualTbl
{
  # manual_cate
  $manual_cate_first_num = 0;
  for( my $i=0; $i<$manual_cate_num; $i++ )
  {
    my $hash_no = $manual_cate_tbl[$i][$manual_cate_col_cate_no];
    if( defined( $manual_cate_tbl_idx_hash_from_cate_no{ $hash_no } ) )
    {
      # 2回目以降の登場
      $manual_cate_tbl[$i][$manual_cate_col_first] = 0;
    }
    else
    {
      # 初登場
      $manual_cate_tbl[$i][$manual_cate_col_first] = 1;
      $manual_cate_tbl_idx_hash_from_cate_no{ $hash_no } = $i;
      $manual_cate_first_num++;
    }
  }

  # manual_title
  $manual_title_first_num = 0;
  for( my $i=0; $i<$manual_title_num; $i++ )
  {
    my $hash_no = $manual_title_tbl[$i][$manual_title_col_cate_no] * $manual_title_hash_cate_no_times + $manual_title_tbl[$i][$manual_title_col_title_no];
    if( defined( $manual_title_tbl_idx_hash_from_cate_no_title_no{ $hash_no } ) )
    {
      # 2回目以降の登場
      $manual_title_tbl[$i][$manual_title_col_first] = 0;
    }
    else
    {
      # 初登場
      $manual_title_tbl[$i][$manual_title_col_first] = 1;
      $manual_title_tbl_idx_hash_from_cate_no_title_no{ $hash_no } = $i;
      $manual_title_first_num++;
    }
  }

  # manual_explain
  $manual_explain_first_num = 0;
  for( my $i=0; $i<$manual_explain_num; $i++ )
  {
    my $hash_no = $manual_explain_tbl[$i][$manual_explain_col_cate_no] * $manual_explain_hash_cate_no_times + $manual_explain_tbl[$i][$manual_explain_col_title_no] * $manual_explain_hash_title_no_times + $manual_explain_tbl[$i][$manual_explain_col_page_no] ;
    if( defined( $manual_explain_tbl_idx_hash_from_cate_no_title_no_page_no{ $hash_no } ) )
    {
      # 2回目以降の登場
      $manual_explain_tbl[$i][$manual_explain_col_first] = 0;
    }
    else
    {
      # 初登場
      $manual_explain_tbl[$i][$manual_explain_col_first] = 1;
      $manual_explain_tbl_idx_hash_from_cate_no_title_no_page_no{ $hash_no } = $i;
      $manual_explain_first_num++;
    }
  }


#=pod
  # 確認
  for( my $i=0; $i<$manual_cate_num; $i++ )
  {
    if( $manual_cate_tbl[$i][$manual_cate_col_first] == 1 )
    {
      for( my $j=0; $j<$manual_cate_col_max; $j++ )
      {
        print "$manual_cate_tbl[$i][$j]".", ";
      }
      print "\r\n";
    }
  }

  for( my $i=0; $i<$manual_title_num; $i++ )
  {
    if( $manual_title_tbl[$i][$manual_title_col_first] == 1 )
    {
      for( my $j=0; $j<$manual_title_col_max; $j++ )
      {
        print "$manual_title_tbl[$i][$j]".", ";
      }
      print "\r\n";
    }
  }

  for( my $i=0; $i<$manual_explain_num; $i++ )
  {
    if( $manual_explain_tbl[$i][$manual_explain_col_first] == 1 )
    {
      for( my $j=0; $j<$manual_explain_col_max; $j++ )
      {
        print "$manual_explain_tbl[$i][$j]".", ";
      }
      print "\r\n";
    }
  }
#=cut
}

##-------------------------------------
### gmmファイルを出力する
##=====================================
sub WriteManualTextGmmFile
{
  my $manual_text_gmm_file_name_complete = &MakeFileNameWithVersion( $manual_text_gmm_file_name, $manual_text_gmm_file_name_ext );


  my $number_row_id = 0;


  # 出力ファイルをオープン
  open( FH, ">:encoding(utf8)", $manual_text_gmm_file_name_complete );


  # header
  open( HEADER, "<:encoding(utf8)", $gmm_header_file_name );
  while( <HEADER> )
  {
    print FH $_;
  }
  close( HEADER );


  my $row_id;

  # manual_cate
  for( my $i=0; $i<$manual_cate_num; $i++ )
  {
    if( $manual_cate_tbl[$i][$manual_cate_col_first] == 1 )
    {
      $row_id = sprintf( "%s%03d", $manual_text_gmm_row_id_name_cate, $manual_cate_tbl[$i][$manual_cate_col_cate_no] );
      &WriteManualTextGmmRow( $row_id, $manual_cate_tbl[$i][$manual_cate_col_cate_name_hira], $manual_cate_tbl[$i][$manual_cate_col_cate_name_kanji] );
      $manual_cate_tbl[$i][$manual_cate_col_cate_name_gmm_row_id] = $number_row_id;
      $number_row_id++;
    }
  }

  # manual_title
  for( my $i=0; $i<$manual_title_num; $i++ )
  {
    if( $manual_title_tbl[$i][$manual_title_col_first] == 1 )
    {
      $row_id = sprintf( "%s%03d_%03d", $manual_text_gmm_row_id_name_title, $manual_title_tbl[$i][$manual_title_col_cate_no], $manual_title_tbl[$i][$manual_title_col_title_no] );
      &WriteManualTextGmmRow( $row_id, $manual_title_tbl[$i][$manual_title_col_title_name_hira], $manual_title_tbl[$i][$manual_title_col_title_name_kanji] );
      $manual_title_tbl[$i][$manual_title_col_title_name_gmm_row_id] = $number_row_id;
      $number_row_id++;
    }
  }

  # manual_explain
  for( my $i=0; $i<$manual_explain_num; $i++ )
  {
    if( $manual_explain_tbl[$i][$manual_explain_col_first] == 1 )
    {
      $row_id = sprintf( "%s%03d_%03d_%03d", $manual_text_gmm_row_id_name_explain, $manual_explain_tbl[$i][$manual_explain_col_cate_no], $manual_explain_tbl[$i][$manual_explain_col_title_no], $manual_explain_tbl[$i][$manual_explain_col_page_no] );
      &WriteManualTextGmmRow( $row_id, $manual_explain_tbl[$i][$manual_explain_col_explain_hira], $manual_explain_tbl[$i][$manual_explain_col_explain_kanji] );
      $manual_explain_tbl[$i][$manual_explain_col_explain_gmm_row_id] = $number_row_id;
      $number_row_id++;
    }
  }


  # footer
  open( FOOTER, "<:encoding(utf8)", $gmm_footer_file_name );
  while( <FOOTER> )
  {
    print FH $_;
  }
  close( FOOTER );


  # 出力ファイルをクローズ
  close( FH );
}

##-------------------------------------
### gmmの1行を出力する
##=====================================
sub WriteManualTextGmmRow
{
  local( $row_id, $hira, $kanji ) = @_;
 
  printf FH "\t<row id=\"%s\">\r\n", $row_id;
  printf FH "\t\t<language name=\"JPN\">";
  printf FH "%s", $hira;
  printf FH "</language>\r\n";
  printf FH "\t\t<language name=\"JPN_KANJI\">";
  printf FH "%s", $kanji;
  printf FH "</language>\r\n";
  printf FH "\t</row>\r\n";
  printf FH "\r\n";
}

##-------------------------------------
### 画像ファイルのコンバートバッチファイルを作成する
##=====================================
sub WriteManualImageListFile
{
  my $manual_image_src_list_file_name_complete       = &MakeFileNameWithVersion( $manual_image_src_list_file_name,       $manual_image_src_list_file_name_ext );
  my $manual_image_conv_bat_file_name_complete       = &MakeFileNameWithVersion( $manual_image_conv_bat_file_name,       $manual_image_conv_bat_file_name_ext );
  my $manual_image_comp_bat_file_name_complete       = &MakeFileNameWithVersion( $manual_image_comp_bat_file_name,       $manual_image_comp_bat_file_name_ext );
  my $manual_image_lz77_nbfs_list_file_name_complete = &MakeFileNameWithVersion( $manual_image_lz77_nbfs_list_file_name, $manual_image_lz77_nbfs_list_file_name_ext );


  my $number_image_id = 0;

  # 出力ファイルをオープン
  open( FH_SRC,  ">:encoding(utf8)", $manual_image_src_list_file_name_complete );
  open( FH_CONV, ">:encoding(utf8)", $manual_image_conv_bat_file_name_complete );
  open( FH_COMP, ">:encoding(utf8)", $manual_image_comp_bat_file_name_complete );
  open( FH_LZ77, ">:encoding(utf8)", $manual_image_lz77_nbfs_list_file_name_complete );

  # manual_explain
  for( my $i=0; $i<$manual_explain_num; $i++ )
  {
    if( $manual_explain_tbl[$i][$manual_explain_col_first] == 1 )
    {
      if( $manual_explain_tbl[$i][$manual_explain_col_image] eq $manual_csv_reserve_image_none )
      {
        # 画像なし
        $manual_explain_tbl[$i][$manual_explain_col_image_arc_id] = $manual_data_image_none_no;
      }
      else
      {
        # 画像あり
        # 同じ名前の画像ファイルは同じarc_idになるようにしておく
        if( defined( $manual_explain_image_arc_id_hash_from_image{ $manual_explain_tbl[$i][$manual_explain_col_image] } ) )
        {
          # 既出の画像
          my $curr_number_image_id = $manual_explain_image_arc_id_hash_from_image{ $manual_explain_tbl[$i][$manual_explain_col_image] };
          $manual_explain_tbl[$i][$manual_explain_col_image_arc_id] = $curr_number_image_id;
        }
        else
        {
          # 初登場の画像
          my $ri = rindex( $manual_explain_tbl[$i][$manual_explain_col_image], $manual_image_src_suffix );
          my $file_name = substr( $manual_explain_tbl[$i][$manual_explain_col_image], 0, $ri );
          my $file_name_bmp        = $file_name . $manual_image_src_suffix;
          my $file_name_nbfs       = $file_name . $manual_image_nbfs_suffix;
          my $file_name_lz77_nbfs  = $file_name . $manual_image_lz77_nbfs_suffix;

          # 画像ファイルのリスト
          printf FH_SRC "%s\r\n", $file_name_bmp;
          # コンバートバッチファイル
          printf FH_CONV "%s %s %s\r\n", $manual_image_conv_command_name, $file_name_bmp, $manual_image_conv_command_option;
          # 圧縮バッチファイル
          printf FH_COMP "%s %s %s %s\r\n", $manual_image_comp_command_name, $manual_image_comp_command_option, $file_name_lz77_nbfs, $file_name_nbfs;
          # 圧縮後のファイルのリスト
          printf FH_LZ77 "\"%s\"\r\n", $file_name_lz77_nbfs;
        
          $manual_explain_tbl[$i][$manual_explain_col_image_arc_id] = $number_image_id;
          
          $manual_explain_image_arc_id_hash_from_image{ $manual_explain_tbl[$i][$manual_explain_col_image] } = $number_image_id;  # ハッシュに登録しておく
          
          $number_image_id++;
        }
      }
    }
  }

  # 画像がないときに使用する画像を最後に追加する
  {
    # 画像がないときに使用する画像のID
    $manual_no_image_arc_id = $number_image_id;  # 画像の1番最後に追加する(即ち画像の総数をIDとする)

    my $ri = rindex( $manual_no_image_file_name, $manual_image_src_suffix );
    my $file_name = substr( $manual_no_image_file_name, 0, $ri );
    my $file_name_bmp        = $file_name . $manual_image_src_suffix;
    my $file_name_nbfs       = $file_name . $manual_image_nbfs_suffix;
    my $file_name_lz77_nbfs  = $file_name . $manual_image_lz77_nbfs_suffix;

    # 画像ファイルのリスト
    printf FH_SRC "%s\r\n", $file_name_bmp;
    # コンバートバッチファイル
    printf FH_CONV "%s %s %s\r\n", $manual_image_conv_command_name, $file_name_bmp, $manual_image_conv_command_option;
    # 圧縮バッチファイル
    printf FH_COMP "%s %s %s %s\r\n", $manual_image_comp_command_name, $manual_image_comp_command_option, $file_name_lz77_nbfs, $file_name_nbfs;
    # 圧縮後のファイルのリスト
    printf FH_LZ77 "\"%s\"\r\n", $file_name_lz77_nbfs;
  }

  # 画像なしのときも使用する画像があるので、その画像のIDを書いておく
  for( my $i=0; $i<$manual_explain_num; $i++ )
  {
    if( $manual_explain_tbl[$i][$manual_explain_col_first] == 1 )
    {
      if( $manual_explain_tbl[$i][$manual_explain_col_image] eq $manual_csv_reserve_image_none )
      {
        # 画像なし
        $manual_explain_tbl[$i][$manual_explain_col_image_arc_id] = $manual_no_image_arc_id;
      }
    }
  }

  # 出力ファイルをクローズ
  close( FH_SRC );
  close( FH_CONV );
  close( FH_COMP );
  close( FH_LZ77 );
}

##-------------------------------------
### 予約語オープンフラグのテーブルを作成する
##=====================================
sub MakeManualOpenFlagTable
{
  # 出力ファイルをオープン
  open( FH,  ">:encoding(utf8)", $manual_reserve_open_flag_enum_file_name );

  printf FH "enum{\r\n";
  
  my $i = 0;

  foreach $ref (@manual_reserve_open_flag_tbl)
  {
    # ハッシュ
    $manual_reserve_open_flag_tbl_idx_hash_from_open_flag_name{ $$ref[$manual_reserve_open_flag_col_open_flag_name] } = $i;
    $manual_reserve_open_flag_no_hash_from_open_flag_name{ $$ref[$manual_reserve_open_flag_col_open_flag_name] } = $$ref[$manual_reserve_open_flag_col_open_flag_no];

    printf FH "  %s%s = %d,\r\n", $manual_reserve_open_flag_enum_prefix, $$ref[$manual_reserve_open_flag_col_open_flag_enum], $$ref[$manual_reserve_open_flag_col_open_flag_no];

    $i++;
  }

  printf FH "};\r\n";

  # 出力ファイルをクローズ
  close( FH );
}

##-------------------------------------
### 最終データを出力する
##=====================================
sub WriteDataFile
{
  # 定義ファイル
  &WriteDataDefineFile();
  # タイトルファイル
  &WriteDataTitleFile();
  # カテゴリファイル
  &WriteDataCateFile();
}

sub WriteDataDefineFile
{
  my $manual_data_define_file_name_complete = &MakeFileNameWithVersion( $manual_data_define_file_name, $manual_data_define_file_name_ext );


  # 出力ファイルをオープン
  open( FH,  ">:encoding(utf8)", $manual_data_define_file_name_complete );

  printf FH "#define %s (%d)\r\n", $manual_data_gmm_none_name, $manual_data_gmm_none_no;
  printf FH "#define %s (%d)\r\n", $manual_data_image_none_name, $manual_data_image_none_no;
  printf FH "#define %s (%d)\r\n", $manual_no_image_name, $manual_no_image_arc_id;

  # 出力ファイルをクローズ
  close( FH );
}

sub WriteDataTitleFile
{
  my $manual_data_title_file_name_complete     = &MakeFileNameWithVersion( $manual_data_title_file_name,     $manual_data_title_file_name_ext );
  my $manual_data_title_ref_file_name_complete = &MakeFileNameWithVersion( $manual_data_title_ref_file_name, $manual_data_title_ref_file_name_ext );

  my $debug_manual_data_title_file_name_complete;
  my $debug_manual_data_title_ref_file_name_complete;
  if( $debug_out == 1 )
  {
    $debug_manual_data_title_file_name_complete     = &MakeFileNameWithVersion( $debug_manual_data_title_file_name,     $debug_manual_data_title_file_name_ext );
    $debug_manual_data_title_ref_file_name_complete = &MakeFileNameWithVersion( $debug_manual_data_title_ref_file_name, $debug_manual_data_title_ref_file_name_ext );
  }


  my $title_byte = 0;
  my $title_data_idx = 0;
  my $buf;

  # 出力ファイルをオープン
  open( FH_MAIN,  ">", $manual_data_title_file_name_complete );  # タイトルファイル
  open( FH_REF,  ">", $manual_data_title_ref_file_name_complete );  # タイトル開始場所ファイル

  binmode FH_MAIN;
  binmode FH_REF;

  if( $debug_out == 1 )
  {
    open( D_FH_MAIN,  ">:encoding(utf8)", $debug_manual_data_title_file_name_complete );  # タイトルファイル
    open( D_FH_REF,  ">:encoding(utf8)", $debug_manual_data_title_ref_file_name_complete );  # タイトル開始場所ファイル
  }

  # タイトル開始場所ファイル
  $buf = pack "v", $manual_title_first_num;  # 符号なし16ビット整数。リトルエンディアン。
  print FH_REF "$buf";

  if( $debug_out == 1 )
  {
    printf D_FH_REF "%d\r\n", $manual_title_first_num;
  }

  for( my $i=0; $i<$manual_title_num; $i++ )
  {
    if( $manual_title_tbl[$i][$manual_title_col_first] == 1 )
    {
      # タイトル開始場所ファイル
      $buf = pack "v", $title_byte;  # 符号なし16ビット整数。リトルエンディアン。
      print FH_REF "$buf";

      if( $debug_out == 1 )
      {
        printf D_FH_REF "%d\r\n", $title_byte;
      }

      # タイトルファイル
      $manual_title_tbl[$i][$manual_title_col_data_idx] = $title_data_idx;
      
      $buf = pack "v", $manual_title_tbl[$i][$manual_title_col_cate_no];  # 符号なし16ビット整数。リトルエンディアン。
      print FH_MAIN "$buf";
      $title_byte += 2;

      if( $debug_out == 1 )
      {
        printf D_FH_MAIN "%d,", $manual_title_tbl[$i][$manual_title_col_cate_no];
      }

      $buf = pack "v", $manual_title_tbl[$i][$manual_title_col_title_no];  # 符号なし16ビット整数。リトルエンディアン。
      print FH_MAIN "$buf";
      $title_byte += 2;

      if( $debug_out == 1 )
      {
        printf D_FH_MAIN "%d,", $manual_title_tbl[$i][$manual_title_col_title_no];
      }

      $buf = pack "v", $manual_title_tbl[$i][$manual_title_col_serial_no];  # 符号なし16ビット整数。リトルエンディアン。
      print FH_MAIN "$buf";
      $title_byte += 2;

      if( $debug_out == 1 )
      {
        printf D_FH_MAIN "%d,", $manual_title_tbl[$i][$manual_title_col_serial_no];
      }

      $buf = pack "v", $manual_title_tbl[$i][$manual_title_col_title_name_gmm_row_id];  # 符号なし16ビット整数。リトルエンディアン。
      print FH_MAIN "$buf";
      $title_byte += 2;

      if( $debug_out == 1 )
      {
        printf D_FH_MAIN "%d,", $manual_title_tbl[$i][$manual_title_col_title_name_gmm_row_id];
      }

      my $open_flag_name = $manual_title_tbl[$i][$manual_title_col_open_flag];
      my $open_flag_no = $manual_reserve_open_flag_no_hash_from_open_flag_name{ $open_flag_name };
      if( !defined( $open_flag_no ) )
      {
        # テーブルに存在しないオープンフラグ名
        die "open_flag_name \"$open_flag_name\" error, stopped";
      }
      $buf = pack "v", $open_flag_no;  # 符号なし16ビット整数。リトルエンディアン。
      print FH_MAIN "$buf";
      $title_byte += 2;

      if( $debug_out == 1 )
      {
        printf D_FH_MAIN "%d,", $open_flag_no;
      }

      my $read_flag_name = $manual_title_tbl[$i][$manual_title_col_read_flag_name];
      my $read_flag_tbl_idx = $read_flag_tbl_idx_hash_from_read_flag_name{ $read_flag_name };
      if( !defined( $read_flag_tbl_idx ) )
      {
        # テーブルに存在しないリードフラグ名
        die "read_flag_name \"$read_flag_name\" error, stopped";
      }
      my $read_flag_no = $read_flag_tbl[$read_flag_tbl_idx][$read_flag_col_read_flag_no];
      $buf = pack "v", $read_flag_no;  # 符号なし16ビット整数。リトルエンディアン。
      print FH_MAIN "$buf";
      $title_byte += 2;

      if( $debug_out == 1 )
      {
        printf D_FH_MAIN "%d,", $read_flag_no;
      }

      $buf = pack "v", $manual_title_tbl[$i][$manual_title_col_page_max];  # 符号なし16ビット整数。リトルエンディアン。
      print FH_MAIN "$buf";
      $title_byte += 2;

      if( $debug_out == 1 )
      {
        printf D_FH_MAIN "%d,", $manual_title_tbl[$i][$manual_title_col_page_max];
      }

      my $page_count = 0;

      for( my $j=0; $j<$manual_explain_num; $j++ )
      {
        if( $manual_explain_tbl[$j][$manual_explain_col_first] == 1 )
        {
          if( $manual_explain_tbl[$j][$manual_explain_col_cate_no] == $manual_title_tbl[$i][$manual_title_col_cate_no] && $manual_explain_tbl[$j][$manual_explain_col_title_no] == $manual_title_tbl[$i][$manual_title_col_title_no] )
          {
            $buf = pack "v", $manual_explain_tbl[$j][$manual_explain_col_explain_gmm_row_id];  # 符号なし16ビット整数。リトルエンディアン。
            print FH_MAIN "$buf";
            $title_byte += 2;

            if( $debug_out == 1 )
            {
              printf D_FH_MAIN "%d,", $manual_explain_tbl[$j][$manual_explain_col_explain_gmm_row_id];
            }

            $buf = pack "v", $manual_explain_tbl[$j][$manual_explain_col_image_arc_id];  # 符号なし16ビット整数。リトルエンディアン。
            print FH_MAIN "$buf";
            $title_byte += 2;

            if( $debug_out == 1 )
            {
              printf D_FH_MAIN "%d,", $manual_explain_tbl[$j][$manual_explain_col_image_arc_id];
            }

            $page_count++;
          }
        }
      }

      if( $manual_title_tbl[$i][$manual_title_col_page_max] != $page_count )
      {
        # ページ数が合っていない
        die "page_count \"$page_count\", page_max \"$manual_title_tbl[$i][$manual_title_col_page_max]\" error, stopped";
      }

      if( $debug_out == 1 )
      {
        printf D_FH_MAIN "\r\n";
      }

      $title_data_idx++;
    }
  }

  # 出力ファイルをクローズ
  close( FH_MAIN );
  close( FH_REF );

  if( $debug_out == 1 )
  {
    close( D_FH_MAIN );
    close( D_FH_REF );
  }
}

sub WriteDataCateFile
{
  my $manual_data_cate_file_name_complete     = &MakeFileNameWithVersion( $manual_data_cate_file_name,     $manual_data_cate_file_name_ext );
  my $manual_data_cate_ref_file_name_complete = &MakeFileNameWithVersion( $manual_data_cate_ref_file_name, $manual_data_cate_ref_file_name_ext );

  my $debug_manual_data_cate_file_name_complete;
  my $debug_manual_data_cate_ref_file_name_complete;
  if( $debug_out == 1 )
  {
    $debug_manual_data_cate_file_name_complete     = &MakeFileNameWithVersion( $debug_manual_data_cate_file_name,     $debug_manual_data_cate_file_name_ext );
    $debug_manual_data_cate_ref_file_name_complete = &MakeFileNameWithVersion( $debug_manual_data_cate_ref_file_name, $debug_manual_data_cate_ref_file_name_ext );
  }


  my $cate_byte = 0;
  my $buf;

  # 出力ファイルをオープン
  open( FH_MAIN,  ">", $manual_data_cate_file_name_complete );  # カテゴリファイル
  open( FH_REF,  ">", $manual_data_cate_ref_file_name_complete );  # カテゴリ開始場所ファイル

  binmode FH_MAIN;
  binmode FH_REF;

  if( $debug_out == 1 )
  {
    open( D_FH_MAIN,  ">:encoding(utf8)", $debug_manual_data_cate_file_name_complete );  # カテゴリファイル
    open( D_FH_REF,  ">:encoding(utf8)", $debug_manual_data_cate_ref_file_name_complete );  # カテゴリ開始場所ファイル
  }

  # カテゴリ開始場所ファイル
  $buf = pack "v", $manual_cate_first_num;  # 符号なし16ビット整数。リトルエンディアン。
  print FH_REF "$buf";

  if( $debug_out == 1 )
  {
    printf D_FH_REF "%d\r\n", $manual_cate_first_num;
  }

  for( my $i=0; $i<$manual_cate_num; $i++ )
  {
    if( $manual_cate_tbl[$i][$manual_cate_col_first] == 1 )
    {
      # カテゴリ開始場所ファイル
      $buf = pack "v", $cate_byte;  # 符号なし16ビット整数。リトルエンディアン。
      print FH_REF "$buf";

      if( $debug_out == 1 )
      {
        printf D_FH_REF "%d\r\n", $cate_byte;
      }

      # カテゴリファイル
      $buf = pack "v", $manual_cate_tbl[$i][$manual_cate_col_cate_no];  # 符号なし16ビット整数。リトルエンディアン。
      print FH_MAIN "$buf";
      $cate_byte += 2;

      if( $debug_out == 1 )
      {
        printf D_FH_MAIN "%d,", $manual_cate_tbl[$i][$manual_cate_col_cate_no];
      }

      $buf = pack "v", $manual_cate_tbl[$i][$manual_cate_col_cate_name_gmm_row_id];  # 符号なし16ビット整数。リトルエンディアン。
      print FH_MAIN "$buf";
      $cate_byte += 2;

      if( $debug_out == 1 )
      {
        printf D_FH_MAIN "%d,", $manual_cate_tbl[$i][$manual_cate_col_cate_name_gmm_row_id];
      }

      # このカテゴリに属するタイトルの個数をまず数える
      my $title_num = 0;
      for( my $j=0; $j<$manual_title_num; $j++ )
      {
        if( $manual_title_tbl[$j][$manual_title_col_first] == 1 )
        {
          if( $manual_title_tbl[$j][$manual_title_col_cate_no] == $manual_cate_tbl[$i][$manual_cate_col_cate_no] )
          {
            $title_num++;
          }
        }
      }
      $manual_cate_tbl[$i][$manual_cate_col_title_num] = $title_num;

      $buf = pack "v", $manual_cate_tbl[$i][$manual_cate_col_title_num];  # 符号なし16ビット整数。リトルエンディアン。
      print FH_MAIN "$buf";
      $cate_byte += 2;

      if( $debug_out == 1 )
      {
        printf D_FH_MAIN "%d,", $manual_cate_tbl[$i][$manual_cate_col_title_num];
      }
     
      # このカテゴリに属するタイトルのインデックスを出力する
      for( my $j=0; $j<$manual_title_num; $j++ )
      {
        if( $manual_title_tbl[$j][$manual_title_col_first] == 1 )
        {
          if( $manual_title_tbl[$j][$manual_title_col_cate_no] == $manual_cate_tbl[$i][$manual_cate_col_cate_no] )
          {
            $buf = pack "v", $manual_title_tbl[$j][$manual_title_col_data_idx];  # 符号なし16ビット整数。リトルエンディアン。
            print FH_MAIN "$buf";
            $cate_byte += 2;

            if( $debug_out == 1 )
            {
              printf D_FH_MAIN "%d,", $manual_title_tbl[$j][$manual_title_col_data_idx];
            }
          }
        }
      }

      if( $debug_out == 1 )
      {
        printf D_FH_MAIN "\r\n";
      }
    }
  }

  # 出力ファイルをクローズ
  close( FH_MAIN );
  close( FH_REF );

  if( $debug_out == 1 )
  {
    close( D_FH_MAIN );
    close( D_FH_REF );
  }
}

##-------------------------------------
### バージョン名付きの名前を得る
##=====================================
sub MakeFileNameWithVersion
{
  local( $file_name, $file_name_ext ) = @_;

  my $file_name_complete;

  if( $manual_make_version eq $manual_version_w )  # ホワイト
  {
    $file_name_complete = sprintf( "%s%s%s", $file_name, $manual_version_w, $file_name_ext );
  }
  else  # elsif( $manual_make_version eq $manual_version_b )  # ブラック
  {
    $file_name_complete = sprintf( "%s%s%s", $file_name, $manual_version_b, $file_name_ext );
  }

  return $file_name_complete;
}

