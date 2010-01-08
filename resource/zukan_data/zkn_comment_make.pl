##============================================================================
=pod
     file   zkn_comment_make.pl
     brief  図鑑の説明テキストのgmmを生成する
            おまけ機能：種族(例：たね(たねポケモン))のgmmを生成する
            おまけ機能：形(例：四足型)を出力する(MONSNOの順番にフォルム0番だけまず出力し、
                        その後フォルムが複数存在するものについて残りのフォルムを出力する)
            おまけ機能：次のフォルムのデータの位置を参照できるファイルを出力する
     author Koji Kawada
     data   2010.01.07
     note   perl zkn_comment_make.pl personal_wb.csv zukan.csv w
            perl zkn_comment_make.pl personal_wb.csv zukan.csv b
            perl zkn_comment_make.pl personal_wb.csv zukan.csv wb

     note   resource/personal/personal_wb.csv(ShiftJIS)
     note   resource/????/zukan.csv(ShiftJIS)
     note   参考  tools/personal_conv/personal_conv.rb(ShiftJIS)

    chihouモジュール名：
=cut 
##============================================================================


##=============================================================================
=pid

フォーマット

形(例：四足型)
    u8(u8だからリトルエンディアンとかない)
    %style_enumの値を出力する(MONSNOの順番にフォルム0番だけまず出力し、
    その後フォルムが複数存在するものについて残りのフォルムを出力する)

次のフォルムのデータの位置を参照できるファイル
    リトルエンディアンのu16
    0次のフォルムなし
    0<次のフォルムのデータの位置(先頭を0とする)
    (MONSNOの順番にフォルム0番だけまず出力し、
    その後フォルムが複数存在するものについて残りのフォルムを出力する)

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

%style_enum =
(
  "丸型"=>0,
  "脚型"=>1,
  "魚型"=>2,
  "昆虫型"=>3,
  "四足型"=>4,
  "四枚羽型"=>5,
  "集合型"=>6,
  "多足型"=>7,
  "直立胴型"=>8,
  "二足尻尾型"=>9,
  "二足人型"=>10,
  "二枚羽型"=>11,
  "腹這脚無型"=>12,
  "腕型"=>13,
);
$style_enum_empty   = 14;  # 入力されていないとき
$style_enum_unknown = 15;  # %style_enumに列挙されていない形のとき

##=============================================================================


##=============================================================================
=pod
    グローバル変数
=cut
##=============================================================================
# 引数
$personal_csv_file_name   = $ARGV[0];  # ????.csv  # shiftjis
$zukan_csv_file_name      = $ARGV[1];  # ????.csv  # shiftjis
$version                  = $ARGV[2];  # w or b or wb

# 定数
# 読み込みファイル
# personal
#$personal_csv_row_data_start           =   0;  # 最初から最後まで全て読み込むようにし、開始終了の指定はしないことにする。
#$personal_csv_row_data_end             = 101;  # personal_csv_row_data_start<=データ<personal_csv_row_data_end
$personal_csv_col_pokename             =   0;  # ポケモン名。例：フシギダネ
$personal_csv_col_form_name            =  99;  # フォルム名。例：una
$personal_csv_col_zenkoku_no           =  39;  # 全国図鑑番号。例：フシギダネが1番
$personal_csv_col_chihou_no            =  40;  # 地方図鑑番号。例：フシギダネが231番(シャチでのフシギダネの地方図鑑番号は231番ではないと思われる)

$temp_personal_csv_file_name    = "zkn_comment_make_pl_temp_personal.csv";   # utf8

# zukan
$zukan_csv_col_pokename                =   0;  # personalと一致しているか確認する
$zukan_csv_col_form_name               =   1;  # personalと一致しているか確認する
$zukan_csv_col_kind                    =   2;  # 種族。例：たね(たねポケモン)
$zukan_csv_col_style                   =   3;  # 形。例：四足型
$zukan_csv_col_w_jpn                   =   4;  # wの説明テキスト。ひらがな。
$zukan_csv_col_w_jpn_kanji             =   5;  # wの説明テキスト。漢字。
$zukan_csv_col_b_jpn                   =   6;  # 改行は垂直タブ(0x0B(VT)(^K)(Ctrl+K))
$zukan_csv_col_b_jpn_kanji             =   7;

$temp_zukan_csv_file_name    = "zkn_comment_make_pl_temp_zukan.csv";   # utf8

# 値なし
$comment_no_value = " ";

$kind_no_value = "？？？？？ポケモン";

# gmm
$gmm_header_file_name   = "header.file";  # utf8
$gmm_footer_file_name   = "footer.file";  # utf8

# 出力ファイル
$comment_w_row_id      = "ZKN_COMMENT_00_";
$comment_b_row_id      = "ZKN_COMMENT_01_";
$comment_w_file_name   = "zkn_comment_00.gmm";    # utf8
$comment_b_file_name   = "zkn_comment_01.gmm";    # utf8

$kind_row_id           = "ZKN_TYPE_";
$kind_file_name        = "zkn_type.gmm";          # utf8

$style_file_name       = "zkn_style.dat";         # バイナリ(u8だからリトルエンディアンとかない)

$next_form_pos_file_name = "zkn_next_form_pos.dat";  # リトルエンディアンのバイナリ

# デバッグ
$debug_monsno_formno_csv_file_name    = "debug_zkn_comment_make_monsno_formno.csv";  # shiftjis
$debug_mons_form_order_csv_file_name  = "debug_zkn_comment_make_mons_order.csv";     # shiftjis

$debug_style_csv_file_name            = "debug_zkn_comment_make_style.csv";          # shiftjis

# 変数
# mons
$mons_num;
@mons_tbl = ();  # 0<= <$mons_num
$mons_col_monsname       = 0;  # 例：アーボック
$mons_col_monsno         = 1;  # 1スタート。001がフシギダネ。
$mons_col_form_num       = 2;
$mons_col_form_start     = 3;  # 最後(含まない)は($mons_col_form_start + $form_col_offset_max * $mons_col_form_numの値)

$form_col_offset_form_name     =  0;
$form_col_offset_formno        =  1;  # 0スタート。($mons_col_form_numの値 - 1)が最後(含む)。
$form_col_offset_zenkoku_no    =  2;
$form_col_offset_chihou_no     =  3;
$form_col_offset_kind          =  4;
$form_col_offset_style         =  5;
$form_col_offset_w_jpn         =  6;  # 改行を\r\n(0D 0A)に直しておく
$form_col_offset_w_jpn_kanji   =  7;
$form_col_offset_b_jpn         =  8;
$form_col_offset_b_jpn_kanji   =  9;
$form_col_offset_max           = 10;

# 0<=$formno<$mons_col_form_numとすると
# $mons_tbl[ $mons_col_form_start + $form_col_offset_max * $formno + $form_col_offset_form_name  ] = $formnoのフォルム名;
# $mons_tbl[ $mons_col_form_start + $form_col_offset_max * $formno + $form_col_offset_formno     ] = $formnoそのもの;
# $mons_tbl[ $mons_col_form_start + $form_col_offset_max * $formno + $form_col_offset_zenkoku_no ] = $formnoの全国図鑑番号;
# $mons_tbl[ $mons_col_form_start + $form_col_offset_max * $formno + $form_col_offset_chihou_no  ] = $formnoの地方図鑑番号;
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

&EncodeFileFromShiftjisToUtf8( $zukan_csv_file_name, $temp_zukan_csv_file_name );
$zukan_csv_file_name = $temp_zukan_csv_file_name;  # これ以降に使用するファイルを差し替えておく

# personalファイルを読み込み、ハッシュを作成する
&ReadPersonalFile();

# zukanファイルを読み込み、データを適切に直す
&ReadZukanFile();
&ChangeReturnCode();  # 改行のコードを直す
&AddKindPokemon();  # 種族が○○ポケモンとなるようにポケモンを追加する

# 確認する
&DebugWriteMonsnoFormnoFile();  #debug
&DebugWriteMonsFormOrderFile();  #debug

# 図鑑の説明テキストのgmmを出力する
&WriteCommentFile();

# 種族(例：たね(たねポケモン))のgmmを出力する
&WriteKindFile();

# 形(例：四足型)を出力する
&WriteStyleFile();
&DebugWriteStyleFile();  #debug

# 次のフォルムのデータの位置を参照できるファイルを出力する
&WriteNextFormPosFile();

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
  unlink( $temp_zukan_csv_file_name );
}

##-------------------------------------
### デバッグ用に作成したものを削除する
##=====================================
sub DebugDeleteFile
{
  unlink( $debug_monsno_formno_csv_file_name );
  unlink( $debug_mons_form_order_csv_file_name );
  unlink( $debug_style_csv_file_name );
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
        $mons_tbl[$idx][$mons_col_form_num] += 1;
      }
    }


  }


  close( DATA );
}

##-------------------------------------
### zukanファイルを読み込む
##=====================================
sub ReadZukanFile
{
  open( DATA, "<:encoding(utf8)",  $zukan_csv_file_name );
 

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
    $value_count = 2;  # zukan.csvにゴミレコードは入らないはず

    if( $value_count <= 1 )
    {
      #サーバからのエクスポートでゴミレコードが入ることがあるので、排除する
    }
    else
    {
      my $monsname = $values[$zukan_csv_col_pokename];
      my $idx = $mons_tbl_idx_hash{ $monsname };
      if( !defined($idx) )  # undefは0とか""らしいが、definedは0とか""と区別してくれているみたい
      {
        # 存在しないポケモン名
        #die "pokename \"$values[$zukan_csv_col_pokename]\" error, stopped";
        next;  # 開発中は存在しないポケモン名はとばしたほうがよさそう
      }
     
      my $form_name = $values[$zukan_csv_col_form_name];
      my $formno;
      my $form_col_start;

      my $form_name_exist = 0;
      for( $formno=0; $formno<$mons_tbl[$idx][$mons_col_form_num]; $formno++ )
      {
        $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;
        if( $form_name eq $mons_tbl[$idx][$form_col_start + $form_col_offset_form_name] )
        {
          $form_name_exist = 1;
          last;
        }
      }
      if( $form_name_exist == 0 )
      {
        # 存在しないフォルム名
        #die "pokename \"$values[$zukan_csv_col_pokename]\", form_name \"$values[$zukan_csv_col_form_name]\" error, stopped";
        $formno = 0;  # フォルムごとの説明テキストが存在せず全フォルムで説明テキストが共通の場合は、
                      # 0番のフォルムを参照するようにしないといけないかな？
                      # ゲームにおいて表示する場合は、文字の長さが0だったら、という処理になるのかな？
      }
      
      $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;
      $mons_tbl[$idx][$form_col_start + $form_col_offset_kind            ] = $values[$zukan_csv_col_kind         ];
      $mons_tbl[$idx][$form_col_start + $form_col_offset_style           ] = $values[$zukan_csv_col_style        ];
      $mons_tbl[$idx][$form_col_start + $form_col_offset_w_jpn           ] = $values[$zukan_csv_col_w_jpn        ];
      $mons_tbl[$idx][$form_col_start + $form_col_offset_w_jpn_kanji     ] = $values[$zukan_csv_col_w_jpn_kanji  ];
      $mons_tbl[$idx][$form_col_start + $form_col_offset_b_jpn           ] = $values[$zukan_csv_col_b_jpn        ];
      $mons_tbl[$idx][$form_col_start + $form_col_offset_b_jpn_kanji     ] = $values[$zukan_csv_col_b_jpn_kanji  ];
    }


  }


  close( DATA );
}

##-------------------------------------
### 改行のコードを直す
##=====================================
sub ChangeReturnCode
{
  for( my $idx=0; $idx<$mons_num; $idx++ )
  {
    my $form_num = $mons_tbl[$idx][$mons_col_form_num];
    for( my $formno=0; $formno<$form_num; $formno++ )
    {
      my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;

      $mons_tbl[$idx][$form_col_start + $form_col_offset_w_jpn           ] =~ s/\v/\r\n/g;  # 0B -> 0D 0A
      $mons_tbl[$idx][$form_col_start + $form_col_offset_w_jpn_kanji     ] =~ s/\v/\r\n/g;  # 0B -> 0D 0A
      $mons_tbl[$idx][$form_col_start + $form_col_offset_b_jpn           ] =~ s/\v/\r\n/g;  # 0B -> 0D 0A
      $mons_tbl[$idx][$form_col_start + $form_col_offset_b_jpn_kanji     ] =~ s/\v/\r\n/g;  # 0B -> 0D 0A
    }
  }
}

##-------------------------------------
### 種族が○○ポケモンとなるようにポケモンを追加する
##=====================================
sub AddKindPokemon
{
  for( my $idx=0; $idx<$mons_num; $idx++ )
  {
    my $form_num = $mons_tbl[$idx][$mons_col_form_num];
    for( my $formno=0; $formno<$form_num; $formno++ )
    {
      my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;

      $mons_tbl[$idx][$form_col_start + $form_col_offset_kind] .= "ポケモン";  # 0B -> 0D 0A
    }
  }
}

##-------------------------------------
### mons_tblの中身を確認する
##=====================================
sub DebugWriteMonsnoFormnoFile
{
  open( FH, ">:encoding(shiftjis)", $debug_monsno_formno_csv_file_name );

  print FH "monsname,monsno,form_num,form_name,formno,zenkoku_no,chihou_no,kind,style,w_jpn,w_jpn_kanji,b_jpn,b_jpn_kanji,form_name,formno,zenkoku_no,chihou_no,kind,style,w_jpn,w_jpn_kanji,b_jpn,b_jpn_kanji,\r\n";  # 0D 0A

  for( my $idx=0; $idx<$mons_num; $idx++ )
  {
    print FH "$mons_tbl[$idx][$mons_col_monsname],";
    print FH "$mons_tbl[$idx][$mons_col_monsno  ],";
    print FH "$mons_tbl[$idx][$mons_col_form_num],";

    my $form_num = $mons_tbl[$idx][$mons_col_form_num];
    for( my $formno=0; $formno<$form_num; $formno++ )
    {
      my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;

      for( my $form_col=0; $form_col<$form_col_offset_max; $form_col++ )
      {
        my $dq = 0;
        if( $form_col_offset_w_jpn <= $form_col && $form_col <= $form_col_offset_b_jpn_kanji )
        {
          $dq = 1;
        }

        if( $dq == 1 )
        {
          print FH "\"";
        }
        print FH "$mons_tbl[$idx][$form_col_start + $form_col]";
        if( $dq == 1 )
        {
          print FH "\"";
        }
        print FH ",";
      }
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
### 図鑑の説明テキストのgmmを出力する
##=====================================
sub WriteCommentFile
{
  if( $version eq "w" || $version eq "wb" )
  {
    &WriteGmmFileMonsnoFormno( $comment_w_file_name, $comment_w_row_id, $form_col_offset_w_jpn, $form_col_offset_w_jpn_kanji, $comment_no_value );
  }
  if( $version eq "b" || $version eq "wb" )
  {
    &WriteGmmFileMonsnoFormno( $comment_b_file_name, $comment_b_row_id, $form_col_offset_b_jpn, $form_col_offset_b_jpn_kanji, $commen_no_value );
  }
}

##-------------------------------------
### 種族(例：たね(たねポケモン))のgmmを出力する
##=====================================
sub WriteKindFile
{
  &WriteGmmFileMonsno( $kind_file_name, $kind_row_id, $form_col_offset_kind, $form_col_offset_kind, $kind_no_value );
}

##-------------------------------------
### MONSNO, FORMNOごとのgmmを出力する
##=====================================
sub WriteGmmFileMonsnoFormno
{
  # DebugWriteMonsFormOrderFileと同じ順番になるように、
  # DebugWriteMonsFormOrderFileと同じ構成にしてある。(DebugWriteMonsFormOrderFileを先につくった)

  local( $file_name, $row_id, $form_col_offset_jpn, $form_col_offset_jpn_kanji, $no_value ) = @_;

  open( FH, ">:encoding(utf8)", $file_name );

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
  printf FH "\t<row id=\"%s%03d_%03d\">\r\n", $row_id, 0, 0;  # 0D 0A
  printf FH "\t\t<language name=\"JPN\">";
  printf FH "$no_value";
  printf FH "</language>\r\n";
  printf FH "\t\t<language name=\"JPN_KANJI\">";
  printf FH "$no_value";
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

    printf FH "\t<row id=\"%s%03d_%03d\">\r\n", $row_id, $mons_tbl[$idx][$mons_col_monsno], $mons_tbl[$idx][$form_col_start + $form_col_offset_formno   ];
    printf FH "\t\t<language name=\"JPN\">";
    printf FH "%s", $mons_tbl[$idx][$form_col_start + $form_col_offset_jpn       ];
    printf FH "</language>\r\n";
    printf FH "\t\t<language name=\"JPN_KANJI\">";
    printf FH "%s", $mons_tbl[$idx][$form_col_start + $form_col_offset_jpn_kanji ];
    printf FH "</language>\r\n";
    printf FH "\t</row>\r\n";
    printf FH "\r\n";
      
    $curr_raw++;
  }

  # タマゴ
  # $curr_raw = $monsno_tamago;
  printf FH "\t<row id=\"%s%03d_%03d\">\r\n", $row_id, $monsno_tamago, 0;  # 0D 0A
  printf FH "\t\t<language name=\"JPN\">";
  printf FH "$no_value";
  printf FH "</language>\r\n";
  printf FH "\t\t<language name=\"JPN_KANJI\">";
  printf FH "$no_value";
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
       
        printf FH "\t<row id=\"%s%03d_%03d\">\r\n", $row_id, $mons_tbl[$idx][$mons_col_monsno], $mons_tbl[$idx][$form_col_start + $form_col_offset_formno   ];
        printf FH "\t\t<language name=\"JPN\">";
        printf FH "%s", $mons_tbl[$idx][$form_col_start + $form_col_offset_jpn       ];
        printf FH "</language>\r\n";
        printf FH "\t\t<language name=\"JPN_KANJI\">";
        printf FH "%s", $mons_tbl[$idx][$form_col_start + $form_col_offset_jpn_kanji ];
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

##-------------------------------------
### MONSNOごとのgmmを出力する
##=====================================
sub WriteGmmFileMonsno
{
  local( $file_name, $row_id, $form_col_offset_jpn, $form_col_offset_jpn_kanji, $no_value ) = @_;

  open( FH, ">:encoding(utf8)", $file_name );

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
  printf FH "\t<row id=\"%s%03d\">\r\n", $row_id, 0;  # 0D 0A
  printf FH "\t\t<language name=\"JPN\">";
  printf FH "$no_value";
  printf FH "</language>\r\n";
  printf FH "\t\t<language name=\"JPN_KANJI\">";
  printf FH "$no_value";
  printf FH "</language>\r\n";
  printf FH "\t</row>\r\n";
  printf FH "\r\n";
  # 「次のフォルムの場所」が0のとき、次のフォルムはない
  # 「次のフォルムの場所」が0以外のとき、その行に次にフォルムが書かれている(行は先頭0行からの行数)
  $curr_raw++;

  # 全ポケモンのフォルム0番だけを出力する
  # 1 <= $curr_raw < 1+ $mons_num
  for( my $idx=0; $idx<$mons_num; $idx++ )
  {
    my $formno = 0;
    my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;

    printf FH "\t<row id=\"%s%03d\">\r\n", $row_id, $mons_tbl[$idx][$mons_col_monsno];
    printf FH "\t\t<language name=\"JPN\">";
    printf FH "%s", $mons_tbl[$idx][$form_col_start + $form_col_offset_jpn       ];
    printf FH "</language>\r\n";
    printf FH "\t\t<language name=\"JPN_KANJI\">";
    printf FH "%s", $mons_tbl[$idx][$form_col_start + $form_col_offset_jpn_kanji ];
    printf FH "</language>\r\n";
    printf FH "\t</row>\r\n";
    printf FH "\r\n";
      
    $curr_raw++;
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

##-------------------------------------
# 形(例：四足型)を出力する
##=====================================
sub WriteStyleFile
{
  # DebugWriteMonsFormOrderFileと同じ順番になるように、
  # DebugWriteMonsFormOrderFileと同じ構成にしてある。(DebugWriteMonsFormOrderFileを先につくった)

  open( FH, ">", $style_file_name );
  binmode FH;
  my $buf;
  my $style_no = 0;

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
  $buf = pack "C", $style_no;  # 符号なし8ビット整数。(u8だからリトルエンディアンとかない)
  print FH "$buf";
  # 「次のフォルムの場所」が0のとき、次のフォルムはない
  # 「次のフォルムの場所」が0以外のとき、その行に次にフォルムが書かれている(行は先頭0行からの行数)
  $curr_raw++;

  # まず全ポケモンのフォルム0番だけを出力する
  # 1 <= $curr_raw < 1+ $mons_num
  for( my $idx=0; $idx<$mons_num; $idx++ )
  {
    my $formno = 0;
    my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;

    if( $mons_tbl[$idx][$form_col_start + $form_col_offset_style] eq "" )
    {
      $style_no = $style_enum_empty;
    }
    else
    {
      if( exists( $style_enum{ $mons_tbl[$idx][$form_col_start + $form_col_offset_style] } ) )
      {
        $style_no = $style_enum{ $mons_tbl[$idx][$form_col_start + $form_col_offset_style] };
      }
      else
      {
        $style_no = $style_enum_unknown;
        
        # %style_enumに列挙されていない形
        die "style \"$mons_tbl[$idx][$form_col_start + $form_col_offset_style]\", \"$mons_tbl[$idx][$mons_col_monsname]\", \"$mons_tbl[$idx][$form_col_start + $form_col_offset_form_name]\" error, stopped";
      }
    }
    $buf = pack "C", $style_no;  # 符号なし8ビット整数。(u8だからリトルエンディアンとかない)
    print FH "$buf";

    $curr_raw++;
  }

  # タマゴ
  # $curr_raw = $monsno_tamago;
  $buf = pack "C", $style_no;  # 符号なし8ビット整数。(u8だからリトルエンディアンとかない)
  print FH "$buf";
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
 
        if( $mons_tbl[$idx][$form_col_start + $form_col_offset_style] eq "" )
        {
          $style_no = $style_enum_empty;
        }
        else
        {
          if( exists( $style_enum{ $mons_tbl[$idx][$form_col_start + $form_col_offset_style] } ) )
          {
            $style_no = $style_enum{ $mons_tbl[$idx][$form_col_start + $form_col_offset_style] };
          }
          else
          {
            $style_no = $style_enum_unknown;
        
            # %style_enumに列挙されていない形
            die "style \"$mons_tbl[$idx][$form_col_start + $form_col_offset_style]\", \"$mons_tbl[$idx][$mons_col_monsname]\", \"$mons_tbl[$idx][$form_col_start + $form_col_offset_form_name]\" error, stopped";
          }
        }
        $buf = pack "C", $style_no;  # 符号なし8ビット整数。(u8だからリトルエンディアンとかない)
        print FH "$buf";
   
        $curr_raw++;
      }
    }
  }

  close( FH );
}

sub DebugWriteStyleFile
{
  # DebugWriteMonsFormOrderFileと同じ順番になるように、
  # DebugWriteMonsFormOrderFileと同じ構成にしてある。(DebugWriteMonsFormOrderFileを先につくった)

  open( FH, ">:encoding(shiftjis)", $debug_style_csv_file_name );
  my $style_no = 0;

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
  print FH "ーーーーー,monsno,フォルム数,フォルム名,formno,次のフォルムの場所,形,形番号,";
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

    print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_style],";
    if( $mons_tbl[$idx][$form_col_start + $form_col_offset_style] eq "" )
    {
      $style_no = $style_enum_empty;
    }
    else
    {
      if( exists( $style_enum{ $mons_tbl[$idx][$form_col_start + $form_col_offset_style] } ) )
      {
        $style_no = $style_enum{ $mons_tbl[$idx][$form_col_start + $form_col_offset_style] };
      }
      else
      {
        $style_no = $style_enum_unknown;
        
        # %style_enumに列挙されていない形
        die "style \"$mons_tbl[$idx][$form_col_start + $form_col_offset_style]\", \"$mons_tbl[$idx][$mons_col_monsname]\", \"$mons_tbl[$idx][$form_col_start + $form_col_offset_form_name]\" error, stopped";
      }
    }
    print FH "$style_no,";

    print FH "\r\n";  # 0D 0A
    
    $curr_raw++;
  }

  # タマゴ
  # $curr_raw = $monsno_tamago;
  print FH "$curr_raw,";
  print FH "タマゴ,$monsno_tamago,1,フォルム名,0,次のフォルムの場所,形,形番号,";
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

        print FH "$mons_tbl[$idx][$form_col_start + $form_col_offset_style],";
        if( $mons_tbl[$idx][$form_col_start + $form_col_offset_style] eq "" )
        {
          $style_no = $style_enum_empty;
        }
        else
        {
          if( exists( $style_enum{ $mons_tbl[$idx][$form_col_start + $form_col_offset_style] } ) )
          {
            $style_no = $style_enum{ $mons_tbl[$idx][$form_col_start + $form_col_offset_style] };
          }
          else
          {
            $style_no = $style_enum_unknown;
        
            # %style_enumに列挙されていない形
            die "style \"$mons_tbl[$idx][$form_col_start + $form_col_offset_style]\", \"$mons_tbl[$idx][$mons_col_monsname]\", \"$mons_tbl[$idx][$form_col_start + $form_col_offset_form_name]\" error, stopped";
          }
        }
        print FH "$style_no,";

        print FH "\r\n";  # 0D 0A
    
        $curr_raw++;
      }
    }
  }

  close( FH );
}

##-------------------------------------
### 次のフォルムのデータの位置を参照できるファイルを出力する
##=====================================
sub WriteNextFormPosFile
{
  # DebugWriteMonsFormOrderFileと同じ順番になるように、
  # DebugWriteMonsFormOrderFileと同じ構成にしてある。(DebugWriteMonsFormOrderFileを先につくった)

  open( FH, ">", $next_form_pos_file_name );
  binmode FH;
  my $buf;
  my $next_form_pos = 0;

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
  $next_form_pos = 0;
  $buf = pack "v", $next_form_pos;  # 符号なし16ビット整数。リトルエンディアン。
  print FH "$buf";
  # 「次のフォルムの場所」が0のとき、次のフォルムはない
  # 「次のフォルムの場所」が0以外のとき、その行に次にフォルムが書かれている(行は先頭0行からの行数)
  $curr_raw++;

  # まず全ポケモンのフォルム0番だけを出力する
  # 1 <= $curr_raw < 1+ $mons_num
  for( my $idx=0; $idx<$mons_num; $idx++ )
  {
    my $formno = 0;
    my $form_col_start = $mons_col_form_start + $form_col_offset_max * $formno;

    my $next_form_raw = 0;  # 次のフォルムはない
    my $form_num = $mons_tbl[$idx][$mons_col_form_num];
    if( $form_num > $formno +1 )
    {
      $next_form_raw = $mons_end_raw + $other_form_total_num;  # その行に次にフォルムが書かれている(行は先頭0行からの行数)
      $other_form_total_num += $form_num;
    }
    $next_form_pos = $next_form_raw;
    $buf = pack "v", $next_form_pos;  # 符号なし16ビット整数。リトルエンディアン。
    print FH "$buf";
    
    $curr_raw++;
  }

  # タマゴ
  # $curr_raw = $monsno_tamago;
  $next_form_pos = 0;
  $buf = pack "v", $next_form_pos;  # 符号なし16ビット整数。リトルエンディアン。
  print FH "$buf";
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
 
        my $next_form_raw = 0;  # 次のフォルムはない
        if( $form_num > $formno +1 )
        {
          $next_form_raw = $curr_raw +1;  # その行に次にフォルムが書かれている(行は先頭0行からの行数)
        }
        $next_form_pos = $next_form_raw;
        $buf = pack "v", $next_form_pos;  # 符号なし16ビット整数。リトルエンディアン。
        print FH "$buf";
    
        $curr_raw++;
      }
    }
  }

  close( FH );
}

