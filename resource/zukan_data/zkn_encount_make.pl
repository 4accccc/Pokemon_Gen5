##============================================================================
=pod
     file   zkn_encount_make.pl
     brief  MONSNOごとの分布データを作成する
     author Koji Kawada
     data   2009.12.28
     note   perl zkn_encount_make.pl encount_wb.csv monsno_hash.rb

     note   resource/encount/encount_wb.csv(ShiftJIS)
            tools/hash/monsno_hash.rb(ShiftJIS)
     note   参考  resource/personal/personal_wb.csv(ShiftJIS)

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
$encount_csv_file_name   = $ARGV[0];  # ????.csv  # shiftjis
$personal_rb_file_name   = $ARGV[1];  # ????.rb   # shiftjis
$version                 = $ARGV[2];  # w or b  # 作成するバージョン

# バージョン
$version_w  = "w";
$version_b  = "b";

# 複数のゾーンIDをまとめた組(グループgroupと呼ぶことにする)を作成するのに必要なファイル
# 読み込むファイル
$excel_converter_file_name      = $ENV{"PROJECT_ROOT"}."/tools/exceltool/ExcelSeetConv.exe";            # Excelコンバータ
$townmap_xls_file_name          = $ENV{"PROJECT_ROOT"}."/resource/townmap/data/townmap_data.xls";       # タウンマップ
$zonetable_xls_file_name        = $ENV{"PROJECT_ROOT"}."/resource/fldmapdata/zonetable/zonetable.xls";  # ゾーン
$zoneid_file_name               = $ENV{"PROJECT_ROOT"}."/resource/fldmapdata/zonetable/zone_id.h";      # ゾーンID  # shiftjis
# 中間ファイル
$temp_townmap_xls_file_name             = "townmap_data.xls";  # ExcelSeetConv.exeがパス付きでファイル名を渡すとうまくコンバートして
$temp_zonetable_xls_file_name           = "zonetable.xls";     # くれなかったので、一旦作業ディレクトリにコピーすることにした。
$temp_townmap_csv_file_name_shiftjis    = "zkn_encount_make_pl_temp_townmap_shiftjis.csv";     # shiftjis
$townmap_csv_file_name                  = "zkn_encount_make_pl_temp_townmap.csv";              # utf8
                   # tempと付いていないけど中間ファイルなので不要になったら削除すること
$temp_zonetable_csv_file_name_shiftjis  = "zkn_encount_make_pl_temp_zonetable_shiftjis.csv";   # shiftjis
$zonetable_csv_file_name                = "zkn_encount_make_pl_temp_zonetable.csv";            # utf8
                   # tempと付いていないけど中間ファイルなので不要になったら削除すること 
$temp_zoneid_file_name                  = "zkn_encount_make_pl_temp_zoneid.h";                # utf8 

# 定数
# encount
#$encount_csv_row_data_start           =   0;  # 最初から最後まで全て読み込むようにし、開始終了の指定はしないことにする。
#$encount_csv_row_data_end             = 101;  # encount_csv_row_data_start<=データ<encount_csv_row_data_end
$encount_csv_col_zonename             =   1;  # ゾーン名(地名を更に細かく分けたもの)。半角英数。
$encount_csv_col_season               =   2;  # 季節名。春/夏/秋/冬。
$encount_csv_col_version_difference   =   3;  # バージョン違い。"●"のときバージョン違いあり。

# (w) or (w&b)
$encount_csv_col_ground_l_start       = 141;
$encount_csv_col_ground_l_num         =  12;
$encount_csv_col_ground_h_start       = 153;
$encount_csv_col_ground_h_num         =  12;
$encount_csv_col_ground_sp_start      = 165;
$encount_csv_col_ground_sp_num        =  12;
$encount_csv_col_water_start          = 177;
$encount_csv_col_water_num            =   5;
$encount_csv_col_water_sp_start       = 182;
$encount_csv_col_water_sp_num         =   5;
$encount_csv_col_fishing_start        = 187;
$encount_csv_col_fishing_num          =   5;
$encount_csv_col_fishing_sp_start     = 192;
$encount_csv_col_fishing_sp_num       =   5;

# (b)
$encount_csv_col_ground_l_start_b       = 334;
$encount_csv_col_ground_h_start_b       = 346;
$encount_csv_col_ground_sp_start_b      = 358;
$encount_csv_col_water_start_b          = 370;
$encount_csv_col_water_sp_start_b       = 375;
$encount_csv_col_fishing_start_b        = 380;
$encount_csv_col_fishing_sp_start_b     = 385;

$encount_csv_version_difference_mark  = "●";

$temp_encount_csv_file_name    = "zkn_encount_make_pl_temp_encount.csv";   # utf8

# personal
$temp_personal_rb_file_name    = "zkn_encount_make_pl_temp_personal.rb";   # utf8

# define
$season_bitflag_none   = 0b00000000;
$season_bitflag_spring = 0b00000001;
$season_bitflag_summer = 0b00000010;
$season_bitflag_autumn = 0b00000100;
$season_bitflag_winter = 0b00001000;
$season_bitflag_year   = $season_bitflag_spring | $season_bitflag_summer | $season_bitflag_autumn | $season_bitflag_winter;  # 0b00001111;  # 一年中
%season_bitflag_tbl =
(
  "春"=>$season_bitflag_spring,
  "夏"=>$season_bitflag_summer,
  "秋"=>$season_bitflag_autumn,
  "冬"=>$season_bitflag_winter,
  ""  =>$season_bitflag_year,      # 空セルなら一年中
);

$place_bitflag_none       = 0b00000000;
$place_bitflag_ground_l   = 0b00000001;
$place_bitflag_ground_h   = 0b00000010;
$place_bitflag_ground_sp  = 0b00000100;
$place_bitflag_water      = 0b00001000;
$place_bitflag_water_sp   = 0b00010000;
$place_bitflag_fishing    = 0b00100000;
$place_bitflag_fishing_sp = 0b01000000;

# その他
# hash
%monsno_hash = ();  # ポケモン名からMONSNOを得るためのハッシュ  # %mons_tbl_idx_hashを使用したので、%monsno_hashは使用しなかった

# monsの一つの季節のデータ
$mons_col_season_data_num;  # = 1 + $zone_num;  # unknown, ゾーン0, ゾーン1, ..., ゾーンzone_num-1  # unknownが0のとき不明でない、1のとき不明
$mons_col_season_ofs_unknown      = 0;  # その季節がunknownか否か
$mons_col_season_ofs_zone_start   = 1;  # その季節におけるゾーンごとの生息地情報の開始位置

$mons_col_group_season_data_num;  # = 1 + $lead_zonename_num;  # unknown, グループ0, グループ1, ..., グループlead_zonename_num-1  # unknownが0のとき不明でない、1のとき不明
$mons_col_group_season_ofs_unknown     = 0;  # その季節がunknownか否か
$mons_col_group_season_ofs_zone_start  = 1;  # その季節におけるグループごとの生息地情報の開始位置

# mons
$mons_num;
@mons_tbl = ();
$mons_col_monsname       = 0;
$mons_col_monsno         = 1;
$mons_col_year           = 2;  # 0のとき一年中同じでない、1のとき一年中同じ(なので春夏秋冬全てに同じデータが入っている)  # これは途中経過なのでyearには値をいれず放置した。mons_col_group_のyearには値を入れた。
$mons_col_spring         = 3;                                             # これは途中経過なのでunknownには値をいれず放置した。mons_col_group_のunknownには値を入れた。
$mons_col_summer;      # = $mons_col_spring + $mons_col_season_data_num;  # これは途中経過なのでunknownには値をいれず放置した。mons_col_group_のunknownには値を入れた。
$mons_col_autumn;      # = $mons_col_summer + $mons_col_season_data_num;  # これは途中経過なのでunknownには値をいれず放置した。mons_col_group_のunknownには値を入れた。
$mons_col_winter;      # = $mons_col_autumn + $mons_col_season_data_num;  # これは途中経過なのでunknownには値をいれず放置した。mons_col_group_のunknownには値を入れた。
$mons_col_end;         # = $mons_col_winter + $mons_col_season_data_num;

$mons_col_group_year;     # = $mons_col_end;
$mons_col_group_start;    # = $mons_col_group_year +1;
$mons_col_group_spring;   # = $mons_col_group_start;
$mons_col_group_summer;   # = $mons_col_group_spring + $mons_col_group_season_data_num;
$mons_col_group_autumn;   # = $mons_col_group_summer + $mons_col_group_season_data_num;
$mons_col_group_winter;   # = $mons_col_group_autumn + $mons_col_group_season_data_num;
$mons_col_group_end;      # = $mons_col_group_winter + $mons_col_group_season_data_num;

%mons_tbl_idx_hash = ();  # ポケモン名からmons_tblのインデックスを得るためのハッシュ
$mons_area_pre_file_name  = "zkn_area_monsno_";  # リトルエンディアンのバイナリ
$mons_area_post_file_name = ".dat";              # 例：zkn_area_monsno_001.dat
$debug_mons_file_name = "debug_zkn_encount_make_mons.csv";  # shiftjis

# バージョンによって$mons_area_pre_file_nameの名前を変更する
# version w
$mons_area_pre_file_name_w  = "zkn_area_w_monsno_";
# version b
$mons_area_pre_file_name_b  = "zkn_area_b_monsno_";


# zone
$zoneseason_num;  # 「ゾーンAの春」「ゾーンAの夏」を別ものとして数えたときの個数
@zoneseason_tbl = ();
$zoneseason_col_zonename          = 0;
$zoneseason_col_zoneid            = 1;  # zone_hashで決めた「このファイル内で決めたゾーンID」の値
$zoneseason_col_season            = 2;
$zoneseason_col_ground_l_start    = 3;
$zoneseason_col_ground_h_start    = $zoneseason_col_ground_l_start     + $encount_csv_col_ground_l_num;
$zoneseason_col_ground_sp_start   = $zoneseason_col_ground_h_start     + $encount_csv_col_ground_h_num;
$zoneseason_col_water_start       = $zoneseason_col_ground_sp_start    + $encount_csv_col_ground_sp_num;
$zoneseason_col_water_sp_start    = $zoneseason_col_water_start        + $encount_csv_col_water_num;
$zoneseason_col_fishing_start     = $zoneseason_col_water_sp_start     + $encount_csv_col_water_sp_num;
$zoneseason_col_fishing_sp_start  = $zoneseason_col_fishing_start      + $encount_csv_col_fishing_num;
$zoneseason_col_end               = $zoneseason_col_fishing_sp_start   + $encount_csv_col_fishing_sp_num; 
$debug_zoneseason_file_name = "debug_zkn_encount_make_zoneseason.csv";  # shiftjis
$zone_num;  # 「ゾーンAの春」「ゾーンAの夏」を同じものとして数えたときの個数。
            # encount.csvに出てきたゾーンの個数なので、pokemon_wbに存在する全ゾーンの個数とは異なります。
%zone_hash = ();  # ゾーン名から「このファイル内で決めたゾーンID」を得るためのハッシュ
                  # 「このファイル内で決めたゾーンID」はこのファイル内で決めたものなので、pokemon_wbのものと値が異なります。
                  # 「このファイル内で決めたゾーンID」は@mons_tblのcolumnの個数にも関わっているので、
                  # 必ず0<=「このファイル内で決めたゾーンID」<zone_numを満たすこと。
@zoneid_zonename_tbl = ();  # zone_hashを用いて $zoneid_zonename_tbl[ $zone_hash{ ゾーン名 } ] = ゾーン名 となるような配列を作成する
%zone_hash_for_season = ();  # ゾーンごとの季節を記録しておく。$zone_hash_for_seazon{ ゾーン名 } = 季節のビットが立った値  # $zone_numカウント用

# MONSNOごとの分布データのファイルのリスト
$mons_area_file_list_name = "zkn_area_file_list.lst";  # shiftjis

# バージョンによって$mons_area_file_list_nameの名前を変更する
# version w
$mons_area_file_list_name_w  = "zkn_area_w_file_list.lst";
# version b
$mons_area_file_list_name_b  = "zkn_area_b_file_list.lst";


# pokemon_wbのゾーンID
%zonename_to_real_zoneid_hash = ();  # ゾーン名から「pokemon_wbで決めたゾーンID」を得るためのハッシュ
                                     # %zonename_to_real_zoneid_hashはエラーの検出にしか使っていないので、
                                     # "/resource/fldmapdata/zonetable/zone_id.h"を読み込むことなく
                                     # 図鑑データを生成することもできた。
                                     # ゾーンIDの数値を一切使わず、ゾーン名(R03R0301など)で済ませられるので。

# ゾーン名からグループ名を得るためのハッシュ
%zonename_to_group_hash = ();
# グループ名からグループの代表ゾーン名を得るためのハッシュ
%group_to_lead_zonename_hash = ();
# 代表ゾーン名の個数、配列
$lead_zonename_num;
@lead_zonename_tbl = ();
%lead_zonename_hash = ();  # $lead_zonename_hash{ 代表ゾーン名 } = @lead_zonename_tblの配列添え字 となるようなハッシュ

$lead_real_zoneid_file_name = "zkn_area_zone_group.cdat";  # zkn_area_monsno_???.datに書かれているゾーンを順番通りにCソースファイルに列挙しておく
                                                        # townmap_data.xlsと同じ個数同じ並びになっている
$lead_real_zoneid_prefix = "ZONE_ID_";  # pokemon_wbのゾーンIDの数値をそのまま書かずに、ZONE_ID_で始まるdefine名を書いておく


##=============================================================================
=pod
    メイン処理
=cut
##=============================================================================
# UTF8だけで作業できるようにする
&EncodeFileFromShiftjisToUtf8( $encount_csv_file_name, $temp_encount_csv_file_name );
$encount_csv_file_name = $temp_encount_csv_file_name;  # これ以降に使用するファイルを差し替えておく

&EncodeFileFromShiftjisToUtf8( $personal_rb_file_name, $temp_personal_rb_file_name );
$personal_rb_file_name = $temp_personal_rb_file_name;  # これ以降に使用するファイルを差し替えておく

# バージョンによって変更するものを変更しておく
&InitVersion();

# グループ前処理
&InitGroup();

# personalファイルを読み込み、ハッシュを作成する
&ReadPersonalFile();
&MakePersonalHash();

# encountファイルを読み込む
&ReadEncountFile();
&MakeZoneidZonenameTbl();
&DebugWriteZoneseasonFile();  #debug

# mons_tblのcolumn(横列)のインデックスを確定する
&InitMonsTblCol();

# mons_tblに対して、出現ゾーンチェック入れを行う
&MakeMonsTblZone();

# ゾーン名から「pokemon_wbで決めたゾーンID」を得るためのハッシュや、
# グループ、代表ゾーン名の情報をまとめる
&MakeZonenameToRealZoneidHash();
&MakeZonenameToGroupHash();
&MakeLeadZonenameTableAndGroupToLeadZonenameHash();

# グループをまとめて代表ゾーン名に集約する
&InitMonsTblColForGroup();
&MakeMonsTblLeadZoneForGroup();

# 生息地が不明か
&CheckUnknownForGroup();
# 一年中同じ分布か
&CheckYearForGroup();

# MONSNOごとの分布データをファイルに書き出す
#&WriteMonsAreaFile();
#&DebugWriteMonsAreaFile();  #debug

&WriteMonsAreaFileForGroup();
&DebugWriteMonsAreaFileForGroup();  #debug

# MONSNOごとの分布データのファイルのリストを書き出す
&WriteMonsAreaFileList();

# ゾーンを順番通りにCソースファイルに列挙する
&WriteLeadRealZoneidFile();

# グループ後処理
&ExitGroup();

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
  unlink( $temp_encount_csv_file_name );
  unlink( $temp_personal_rb_file_name );
}

##-------------------------------------
### デバッグ用に作成したものを削除する
##=====================================
sub DebugDeleteFile()
{
  unlink( $debug_zoneseason_file_name );
  unlink( $debug_mons_file_name );
}

##-------------------------------------
### personalファイルを読み込む
##=====================================
sub ReadPersonalFile
{
  open( FH, "<:encoding(utf8)", $personal_rb_file_name );

  my $start_word = "{";
  my $end_word   = "}";
  my $flag       = 0;
  $mons_num = 0;

  while( my $line = <FH> )
  {
    if( $flag == 0 )
    {
      if( index( $line, $start_word ) >= 0 )
      {
        $flag = 1;
      }
    }
    else
    {
      if( index( $line, $end_word ) >= 0 )
      {
         last;
      }
      else
      {
        my @word = split( /(\"|=|>|,)/, $line );  # ()を付けてデリミタも残してみる
        $mons_tbl[$mons_num][$mons_col_monsname] = $word[2];
        $mons_tbl[$mons_num][$mons_col_monsno]   = $word[8];
#debug        print "$mons_tbl[$mons_num][$mons_col_monsname], $mons_tbl[$mons_num][$mons_col_monsno]\r\n";  # 0D 0A
        $mons_num++;
      }
    }
  }

  close( FH );
}

##-------------------------------------
### personalのハッシュを作成する
##=====================================
sub MakePersonalHash
{
  for( my $i=0; $i<$mons_num; $i++ )
  {
    $monsno_hash{ $mons_tbl[$i][$mons_col_monsname] } = $mons_tbl[$i][$mons_col_monsno]; 
    $mons_tbl_idx_hash{ $mons_tbl[$i][$mons_col_monsname] } = $i; 
  }

=pod
#debug
  my $i;
  $i = 0;
  my @monsno_hash_key = keys(%monsno_hash);
  foreach my $k ( @monsno_hash_key )
  {
    print "$i, $k\r\n";  # 0D 0A
    $i++;
  }
  $i = 0;
  my @mons_tbl_idx_hash_key = keys(%mons_tbl_idx_hash);
  foreach my $k ( @mons_tbl_idx_hash_key )
  {
    print "$i, $k\r\n";  # 0D 0A
    $i++;
  }
=cut
}

##-------------------------------------
### encountファイルを読み込む
##=====================================
sub ReadEncountFile
{
  open( DATA, "<:encoding(utf8)",  $encount_csv_file_name );
 
  $zoneseason_num = 0;
  $zone_num = 0;

  # (w) or (w&b)
  my @col_tbl =
  (
    [ $encount_csv_col_ground_l_start,      $encount_csv_col_ground_l_num,      $zoneseason_col_ground_l_start      ],
    [ $encount_csv_col_ground_h_start,      $encount_csv_col_ground_h_num,      $zoneseason_col_ground_h_start      ],
    [ $encount_csv_col_ground_sp_start,     $encount_csv_col_ground_sp_num,     $zoneseason_col_ground_sp_start     ],
    [ $encount_csv_col_water_start,         $encount_csv_col_water_num,         $zoneseason_col_water_start         ],
    [ $encount_csv_col_water_sp_start,      $encount_csv_col_water_sp_num,      $zoneseason_col_water_sp_start      ],
    [ $encount_csv_col_fishing_start,       $encount_csv_col_fishing_num,       $zoneseason_col_fishing_start       ],
    [ $encount_csv_col_fishing_sp_start,    $encount_csv_col_fishing_sp_num,    $zoneseason_col_fishing_sp_start    ],
  );

  # (b)
  my @col_tbl_b =
  (
    [ $encount_csv_col_ground_l_start_b,      $encount_csv_col_ground_l_num,      $zoneseason_col_ground_l_start      ],
    [ $encount_csv_col_ground_h_start_b,      $encount_csv_col_ground_h_num,      $zoneseason_col_ground_h_start      ],
    [ $encount_csv_col_ground_sp_start_b,     $encount_csv_col_ground_sp_num,     $zoneseason_col_ground_sp_start     ],
    [ $encount_csv_col_water_start_b,         $encount_csv_col_water_num,         $zoneseason_col_water_start         ],
    [ $encount_csv_col_water_sp_start_b,      $encount_csv_col_water_sp_num,      $zoneseason_col_water_sp_start      ],
    [ $encount_csv_col_fishing_start_b,       $encount_csv_col_fishing_num,       $zoneseason_col_fishing_start       ],
    [ $encount_csv_col_fishing_sp_start_b,    $encount_csv_col_fishing_sp_num,    $zoneseason_col_fishing_sp_start    ],
  );


  # 値に改行コードを含む CSV形式を扱う

  while (my $line = <DATA>) {
    $line .= <DATA> while ($line =~ tr/"// % 2 and !eof(DATA));

    $line =~ s/(?:\x0D\x0A|[\x0D\x0A])?$/,/;
    @values = map {/^"(.*)"$/s ? scalar($_ = $1, s/""/"/g, $_) : $_}
                  ($line =~ /("[^"]*(?:""[^"]*)*"|[^,]*),/g);

    # @values を処理する
   

    $zoneseason_tbl[$zoneseason_num][$zoneseason_col_zonename] = $values[$encount_csv_col_zonename];
    $zoneseason_tbl[$zoneseason_num][$zoneseason_col_season]   = $season_bitflag_tbl{ $values[$encount_csv_col_season] };
    if( !defined($zoneseason_tbl[$zoneseason_num][$zoneseason_col_season]) )
    {
      # 存在しない季節名
      die "season \"$zoneseason_tbl[$zoneseason_num][$zoneseason_col_season]\" error, stopped";
    }

    my $version_difference = 0;  # 0のときバージョン違いなし、1のときバージョン違いあり。
    if( $values[$encount_csv_col_version_difference] eq $encount_csv_version_difference_mark )
    {
      $version_difference = 1;
    }
    
    if(    ( $version eq $version_w )
        || ( $version_difference == 0 ) )
    {
      foreach my $ref (@col_tbl)
      {
        for( my $i=0; $i<$$ref[1]; $i++ )
        {
          $zoneseason_tbl[$zoneseason_num][$$ref[2]+$i] = $mons_tbl_idx_hash{ $values[$$ref[0]+$i] };
          if( $values[$$ref[0]+$i] ne "" && (!defined($zoneseason_tbl[$zoneseason_num][$$ref[2]+$i])) )
          {
            # 存在しないポケモン名
            die "pokemon name \"$values[$$ref[0]+$i]\" error, stopped";
          }
        }
      }
    }
    else
    {
      foreach my $ref (@col_tbl_b)
      {
        for( my $i=0; $i<$$ref[1]; $i++ )
        {
          $zoneseason_tbl[$zoneseason_num][$$ref[2]+$i] = $mons_tbl_idx_hash{ $values[$$ref[0]+$i] };
          if( $values[$$ref[0]+$i] ne "" && (!defined($zoneseason_tbl[$zoneseason_num][$$ref[2]+$i])) )
          {
            # 存在しないポケモン名
            die "pokemon name \"$values[$$ref[0]+$i]\" error, stopped";
          }
        }
      }
    }

    my $zone_season_bitflag = $zone_hash_for_season{ $values[$encount_csv_col_zonename] };
    if( !defined($zone_season_bitflag) )  # 初登場のゾーン。だから、「このファイルで決めたゾーンID」をここで決める。
    {
      $zone_hash_for_season{ $values[$encount_csv_col_zonename] } = $season_bitflag_tbl{ $values[$encount_csv_col_season] };
      $zone_hash{ $values[$encount_csv_col_zonename] } = $zone_num;
      $zone_num++;
    }
    else  # 2度目(以降)の登場になるゾーン(季節が違う)。だから、「このファイルで決めたゾーンID」は既に決まっているので、ここでは出てきた季節を覚えておくことしかしない。
    {
      if( $zone_hash_for_season{ $values[$encount_csv_col_zonename] } & $season_bitflag_tbl{ $values[$encount_csv_col_season] } )  # 出てきた季節を覚える前に、既出の季節でないかチェックする
      {
        # 既出の季節
        die "season overlap \"$values[$encount_csv_col_zonename]\" \"$values[$encount_csv_col_season]\" error, stopped";
      }
      $zone_hash_for_season{ $values[$encount_csv_col_zonename] } |= $season_bitflag_tbl{ $values[$encount_csv_col_season] };
    }
    $zoneseason_tbl[$zoneseason_num][$zoneseason_col_zoneid] = $zone_hash{ $values[$encount_csv_col_zonename] };

    $zoneseason_num++;


  }


  close( DATA );
}

##-------------------------------------
### zone_hashを用いて $zoneid_zonename_tbl[ $zone_hash{ ゾーン名 } ] = ゾーン名 となるような配列を作成する
##=====================================
sub MakeZoneidZonenameTbl
{
  my @zone_hash_key = keys(%zone_hash);
  foreach my $k ( @zone_hash_key )
  {
    $zoneid_zonename_tbl[ $zone_hash{$k} ] = $k;
  }
}

##-------------------------------------
### encountファイルの中身を確認する
##=====================================
sub DebugWriteZoneseasonFile
{
  open( FH, ">:encoding(shiftjis)", $debug_zoneseason_file_name );

  for( my $i=0; $i<$zoneseason_num; $i++ )
  {
    for( my $j=0; $j<$zoneseason_col_end; $j++ )
    {
#      print FH "$zoneseason_tbl[$i][$j],";
      print FH Encode::encode( 'shiftjis', "$zoneseason_tbl[$i][$j]," );
    }
#    print FH "\r\n";  # 0D 0A
    print FH Encode::encode( 'shiftjis', "\r\n" );  # 0D 0A
  }

  close( FH );
}

##-------------------------------------
### mons_tblのcolumn(横列)のインデックスを確定し初期化する
##=====================================
sub InitMonsTblCol
{
  # $zone_numが確定してから行うこと！
#debug  print "zone_num = $zone_num\r\n";  # 0D 0A

  $mons_col_season_data_num = 1 + $zone_num; 

  $mons_col_summer = $mons_col_spring + $mons_col_season_data_num;
  $mons_col_autumn = $mons_col_summer + $mons_col_season_data_num;
  $mons_col_winter = $mons_col_autumn + $mons_col_season_data_num;
  $mons_col_end    = $mons_col_winter + $mons_col_season_data_num;

  for( my $i=0; $i<$mons_num; $i++ )
  {
    # year
    $mons_tbl[$i][$mons_col_year] = 0;

    # unknown
    $mons_tbl[$i][$mons_col_spring + $mons_col_season_ofs_unknown] = 0;
    $mons_tbl[$i][$mons_col_summer + $mons_col_season_ofs_unknown] = 0;
    $mons_tbl[$i][$mons_col_autumn + $mons_col_season_ofs_unknown] = 0;
    $mons_tbl[$i][$mons_col_winter + $mons_col_season_ofs_unknown] = 0;

    # ゾーンごと
    for( my $j=0; $j<$zone_num; $j++ )
    {
      $mons_tbl[$i][$mons_col_spring + $mons_col_season_ofs_zone_start + $j] = $place_bitflag_none;
      $mons_tbl[$i][$mons_col_summer + $mons_col_season_ofs_zone_start + $j] = $place_bitflag_none;
      $mons_tbl[$i][$mons_col_autumn + $mons_col_season_ofs_zone_start + $j] = $place_bitflag_none;
      $mons_tbl[$i][$mons_col_winter + $mons_col_season_ofs_zone_start + $j] = $place_bitflag_none;
    }
  }
}

##-------------------------------------
### mons_tblに対して、出現ゾーンチェック入れを行う
##=====================================
sub MakeMonsTblZone
{
  my @col_tbl =
  (
    [ $zoneseason_col_ground_l_start,   $zoneseason_col_ground_l_start   + $encount_csv_col_ground_l_num,   $place_bitflag_ground_l   ],
    [ $zoneseason_col_ground_h_start,   $zoneseason_col_ground_h_start   + $encount_csv_col_ground_h_num,   $place_bitflag_ground_h   ],
    [ $zoneseason_col_ground_sp_start,  $zoneseason_col_ground_sp_start  + $encount_csv_col_ground_sp_num,  $place_bitflag_ground_sp  ],
    [ $zoneseason_col_water_start,      $zoneseason_col_water_start      + $encount_csv_col_water_num,      $place_bitflag_water      ],
    [ $zoneseason_col_water_sp_start,   $zoneseason_col_water_sp_start   + $encount_csv_col_water_sp_num,   $place_bitflag_water_sp   ],
    [ $zoneseason_col_fishing_start,    $zoneseason_col_fishing_start    + $encount_csv_col_fishing_num,    $place_bitflag_fishing    ],
    [ $zoneseason_col_fishing_sp_start, $zoneseason_col_fishing_sp_start + $encount_csv_col_fishing_sp_num, $place_bitflag_fishing_sp ],
  );

  my @season_tbl = 
  (
    [ $season_bitflag_spring, $mons_col_spring ],
    [ $season_bitflag_summer, $mons_col_summer ],
    [ $season_bitflag_autumn, $mons_col_autumn ],
    [ $season_bitflag_winter, $mons_col_winter ],
  );

  for( my $i=0; $i<$zoneseason_num; $i++ )
  {
    foreach my $ref (@col_tbl)
    {
      for( my $j=$$ref[0]; $j<$$ref[1]; $j++ )
      {
        my $mons_tbl_idx = $zoneseason_tbl[$i][$j];
        if( defined($mons_tbl_idx) )
        {
          foreach my $s (@season_tbl)
          {
            if( $zoneseason_tbl[$i][$zoneseason_col_season] & $$s[0] )
            {
              $mons_tbl[ $mons_tbl_idx ][ $$s[1] + $mons_col_season_ofs_zone_start + $zoneseason_tbl[$i][$zoneseason_col_zoneid] ] |= $$ref[2];
            }
          }
        }
      }
    }
  }
}

##-------------------------------------
### MONSNOごとの分布データをファイルに書き出す
##=====================================
sub WriteMonsAreaFile
{
  for( my $i=0; $i<$mons_num; $i++ )
  {
    my $mons_area_file_name = sprintf( "%s%03d%s", $mons_area_pre_file_name, $mons_tbl[$i][$mons_col_monsno], $mons_area_post_file_name );
    open( FH, ">", $mons_area_file_name );
    binmode FH;

    my $buf;
   
    # 一年中同じか否か
    $buf = pack "C", $mons_tbl[$i][$mons_col_year];         # 符号なし8ビット整数。(u8だからリトルエンディアンとかない)
    print FH "$buf";
   
    # 春の不明か否かとゾーンごとの生息情報、夏の不明か否かとゾーンごとの生息情報、秋の...
    for( my $j=$mons_col_spring; $j<$mons_col_end; $j++ )
    {
       $buf = pack "C", $mons_tbl[$i][$j];         # 符号なし8ビット整数。(u8だからリトルエンディアンとかない)
       print FH "$buf";
    }

    close( FH );
  }
}

##-------------------------------------
### mons_tblの中身を確認する
##=====================================
sub DebugWriteMonsAreaFile
{
  open( FH, ">:encoding(shiftjis)", $debug_mons_file_name );

  print FH "name,monsno,year,unknown,";
  for( my $i=0; $i<$zone_num; $i++ )
  {
    print FH "$zoneid_zonename_tbl[$i],";
  }
  print FH "\r\n";  # 0D 0A

  for( my $i=0; $i<$mons_num; $i++ )
  {
    for( my $j=0; $j<$mons_col_spring; $j++ )
    {
      print FH "$mons_tbl[$i][$j],";
#      print FH Encode::encode( 'shiftjis', "$mons_tbl[$i][$j]," );  # これだとdoes not map to shiftjisというエラーが出てしまう！
    }

    for( my $s=0; $s<4; $s++ )
    {
      my $season_start = 0;

      if( $s == 0 )
      {
        $season_start = $mons_col_spring;
      }
      elsif( $s == 1 )
      {
        $season_start = $mons_col_summer;
      }
      elsif( $s == 2 )
      {
        $season_start = $mons_col_autumn;
      }
      else
      {
        $season_start = $mons_col_winter;
      }

      if( $s != 0 )
      {
        for( my $h=0; $h<$mons_col_spring; $h++ )
        {
          print FH ",";  # 横に長過ぎるので季節ごとに折り返すことにする
        }
      }
      printf FH "$mons_tbl[$i][$season_start + $mons_col_season_ofs_unknown],";
      for( my $j=0; $j<$zone_num; $j++ )
      {
        printf FH "%b,", $mons_tbl[$i][$season_start + $mons_col_season_ofs_zone_start + $j];
      }
      print FH "\r\n";  # 0D 0A  # 横に長過ぎるので季節ごとに折り返すことにする
    }
  }

  close( FH );

  # 参考までに残しておくエラーが出てしまう記述
#      printf FH Encode::encode( 'shiftjis', "%b,", $mons_tbl[$i][$j] );  # これだとdoes not map to shiftjisというエラーが出てしまう！
#    print FH Encode::encode( 'shiftjis', "\r\n" );  # 0D 0A  # これだとdoes not map to shiftjisというエラーが出てしまう！

}

##-------------------------------------
### MONSNOごとの分布データのファイルのリストを書き出す
##=====================================
sub WriteMonsAreaFileList
{
  open( FH, ">:encoding(shiftjis)", $mons_area_file_list_name );

  for( my $i=0; $i<$mons_num; $i++ )
  {
    my $mons_area_file_name = sprintf( "%s%03d%s", $mons_area_pre_file_name, $mons_tbl[$i][$mons_col_monsno], $mons_area_post_file_name );
    print FH "\"$mons_area_file_name\"\r\n";  # 0D 0A
  }

  close( FH );
}

##-------------------------------------
### グループ前処理
##=====================================
sub InitGroup
{
  system( 'cp '.$townmap_xls_file_name.' .' );
  system( 'cp '.$zonetable_xls_file_name.' .' );

  $townmap_xls_file_name    = $temp_townmap_xls_file_name;    # これ以降に使用するファイルを差し替えておく
  $zonetable_xls_file_name  = $temp_zonetable_xls_file_name;  # これ以降に使用するファイルを差し替えておく

  &ConvertExcelToCsv( $townmap_xls_file_name, "data", $temp_townmap_csv_file_name_shiftjis );
  &ConvertExcelToCsv( $zonetable_xls_file_name, "Sheet1", $temp_zonetable_csv_file_name_shiftjis );

  # UTF8だけで作業できるようにする
  &EncodeFileFromShiftjisToUtf8( $temp_townmap_csv_file_name_shiftjis, $townmap_csv_file_name );
  &EncodeFileFromShiftjisToUtf8( $temp_zonetable_csv_file_name_shiftjis, $zonetable_csv_file_name );

  &EncodeFileFromShiftjisToUtf8( $zoneid_file_name, $temp_zoneid_file_name );
  $zoneid_file_name = $temp_zoneid_file_name;  # これ以降に使用するファイルを差し替えておく
}

##-------------------------------------
### グループ後処理
##=====================================
sub ExitGroup
{
  # 不要なファイルを削除する
  &DeleteTempGroup();
  &DebugDeleteGroupFile();  #debug
}
sub DeleteTempGroup
{
  unlink( $temp_townmap_xls_file_name );
  unlink( $temp_zonetable_xls_file_name );
  unlink( $temp_townmap_csv_file_name_shiftjis );
  unlink( $temp_zonetable_csv_file_name_shiftjis );
  unlink( $townmap_csv_file_name );
  unlink( $zonetable_csv_file_name );
  unlink( $temp_zoneid_file_name );
}
sub DebugDeleteGroupFile()
{
  # ない
}

##-------------------------------------
### Excelファイルをcsvファイルにコンバートする
##=====================================
sub ConvertExcelToCsv
{
  my( $xls_file_name, $xls_sheet_name, $csv_file_name ) = @_;
  system( $excel_converter_file_name.' -o '.$csv_file_name.' -n '.$xls_sheet_name.' -s csv '.$xls_file_name );
}

##-------------------------------------
### ゾーン名から「pokemon_wbで決めたゾーンID」を得るためのハッシュをつくる
##=====================================
sub MakeZonenameToRealZoneidHash()
{
  open( FH, "<:encoding(utf8)", $zoneid_file_name );

=pid
ファイルの中身
#define ZONE_ID_C04R0102         ( 85)
#define ZONE_ID_C08GYM0101       (144)
#define ZONE_ID_MAX	(476)
=cut

  my $pre_word           = "#define ZONE_ID_";
  my $pre_word_except    = "#define ZONE_ID_MAX";
  my $space_word         = " ";
  my $kakko_open_word    = "(";
  my $kakko_close_word   = ")";
  my $start_pos = length($pre_word);

  while( my $line = <FH> )
  {
    if( index( $line, $pre_word ) == 0 && index( $line, $pre_word_except ) != 0 )
    {
      my $end_pos = index( $line, $space_word, $start_pos );  # $start_pos<= <$end_pos
      my $zonename = substr( $line, $start_pos, $end_pos - $start_pos );

      my $no_end_pos = rindex( $line, $kakko_close_word );  # $no_start_pos<= <$no_end_pos
      my $no_start_pos_kakko_open = rindex( $line, $kakko_open_word );
      my $no_start_pos_space      = rindex( $line, $space_word );
      my $no_start_pos;
      if( $no_start_pos_kakko_open > $no_start_pos_space )
      {
        $no_start_pos = $no_start_pos_kakko_open +1;
      }
      else
      {
        $no_start_pos = $no_start_pos_space +1;
      }
      my $real_zoneid = substr( $line, $no_start_pos, $no_end_pos - $no_start_pos );

      # ゾーン名から「pokemon_wbで決めたゾーンID」を得るためのハッシュ
      $zonename_to_real_zoneid_hash{ $zonename } = $real_zoneid; 
    }
  }

  close( FH );

=pid
  # 確認
  my @zonename_to_real_zoneid_hash_key = keys(%zonename_to_real_zoneid_hash);
  foreach my $k ( @zonename_to_real_zoneid_hash_key )
  {
    printf "%s*%d\r\n", $k, $zonename_to_real_zoneid_hash{ $k };
  }
=cut
}
##-------------------------------------
### ゾーン名からグループ名を得るためのハッシュをつくる
##=====================================
sub MakeZonenameToGroupHash()
{
  open( DATA, "<:encoding(utf8)", $zonetable_csv_file_name );

  my $line_no = 0;
  my $line_end = "END";
  my $group_col_name = "ZONE_GROUP";
  my $group_col_no;
  my $zonename_col_no = 0;


  # 値に改行コードを含む CSV形式を扱う

  while (my $line = <DATA>) {
    $line .= <DATA> while ($line =~ tr/"// % 2 and !eof(DATA));

    $line =~ s/(?:\x0D\x0A|[\x0D\x0A])?$/,/;
    @values = map {/^"(.*)"$/s ? scalar($_ = $1, s/""/"/g, $_) : $_}
                  ($line =~ /("[^"]*(?:""[^"]*)*"|[^,]*),/g);

    # @values を処理する
   

    if( $line_no == 0 )
    {
      $group_col_no = 0;
      foreach my $word ( @values )
      {
        if( $group_col_name eq $word )
        {
          last;
        }
        $group_col_no++;
      }
    }
    elsif( $values[$zonename_col_no] eq $line_end )
    {
      last;
    }
    else
    {
      my $zonename = $values[$zonename_col_no];
      my $group = $values[$group_col_no];
      if( !defined( $zonename_to_real_zoneid_hash{ $zonename } ) )
      {
        # 存在しないゾーン名
        die "zonename \"$zonename\" error, stopped";
      }
      else
      {
        $zonename_to_group_hash{ $zonename } = $group;
      }
    }
    $line_no++;
  

  }


  close( DATA );


=pid
  # 確認
  my @zonename_to_group_hash_key = keys(%zonename_to_group_hash);
  foreach my $k ( @zonename_to_group_hash_key )
  {
    printf "%s*%s\r\n", $k, $zonename_to_group_hash{ $k };
  }
=cut
}
##-------------------------------------
### 代表ゾーン名の個数、配列をつくり、グループ名からグループの代表ゾーン名を得るためのハッシュをつくる
##=====================================
sub MakeLeadZonenameTableAndGroupToLeadZonenameHash()
{
  open( DATA, "<:encoding(utf8)", $townmap_csv_file_name );

  my $line_start        = "#file_start";
  my $line_end          = "#file_end";
  my $zone_col_name     = "#zone";
  my $zone_col_no;
  my $step              = 0;  # 0のとき$line_startまで
                              # 1のときデータ読み込み中で$line_endまで
                              # 2のとき$line_endから
  my $pre_zone_word     = "ZONE_ID_";
  my $start_pos         = length($pre_zone_word);
  
  $lead_zonename_num    = 0;  # 代表ゾーン名の個数


  # 値に改行コードを含む CSV形式を扱う

  while (my $line = <DATA>) {
    $line .= <DATA> while ($line =~ tr/"// % 2 and !eof(DATA));

    $line =~ s/(?:\x0D\x0A|[\x0D\x0A])?$/,/;
    @values = map {/^"(.*)"$/s ? scalar($_ = $1, s/""/"/g, $_) : $_}
                  ($line =~ /("[^"]*(?:""[^"]*)*"|[^,]*),/g);

    # @values を処理する
   

    if( $step == 0 )
    {
      if( $values[0] eq $line_start )
      {
        $zone_col_no = 0;
        foreach my $word ( @values )
        {
          if( $zone_col_name eq $word )
          {
            last;
          }
          $zone_col_no++;
        }
        $step = 1;
      }
    }
    elsif( $step == 1 )
    {
      if( $values[0] eq $line_end )
      {
        $step = 2;
      }
      else
      {
        my $zone_word = $values[$zone_col_no];
        my $zonename = substr( $zone_word, $start_pos, length($zone_word)-$start_pos );

        $lead_zonename_tbl[$lead_zonename_num] = $zonename;  # 代表ゾーン名の配列
        $lead_zonename_hash{$zonename} = $lead_zonename_num;  # 代表ゾーン名から配列添え字を得るハッシュ

        my $group = $zonename_to_group_hash{ $zonename };
        if( !defined( $group ) )
        {
          # 存在しないゾーン名
          die "zonename \"$zonename\" error, stopped";
        }
        else
        {
          if( defined( $group_to_lead_zonename_hash{ $group } ) )
          {
            # 既に代表ゾーン名が設定してあるグループ
#今は見逃す            die "group \"$group\", lead_zonename \"$group_to_lead_zonename_hash{ $group }\", new lead_zonename \"$zonename\",  error, stopped";
          }
          else
          {
            $group_to_lead_zonename_hash{ $group } = $zonename;
          }
        }
        
        $lead_zonename_num++;  # 代表ゾーン名の個数
      }
    }
    else  # if( $step == 2 )
    {
      last;
    }


  }


  close( DATA );


=pid
  # 確認
  printf "%d\r\n", $lead_zonename_num;
  my @group_to_lead_zonename_hash_key = keys(%group_to_lead_zonename_hash);
  foreach my $k ( @group_to_lead_zonename_hash_key )
  {
    printf "%s*%s\r\n", $k, $group_to_lead_zonename_hash{ $k };
  }
=cut
}

##-------------------------------------
### mons_tblのcolumn(横列)のインデックスを確定し初期化する for グループ
##=====================================
sub InitMonsTblColForGroup
{
  # $lead_zonename_numが確定してから行うこと！
#debug  print "lead_zonename_num = $lead_zonename_num\r\n";  # 0D 0A

  $mons_col_group_season_data_num = 1 + $lead_zonename_num; 

  $mons_col_group_year      = $mons_col_end;
  $mons_col_group_start     = $mons_col_group_year +1;
  $mons_col_group_spring    = $mons_col_group_start;
  $mons_col_group_summer    = $mons_col_group_spring + $mons_col_group_season_data_num;
  $mons_col_group_autumn    = $mons_col_group_summer + $mons_col_group_season_data_num;
  $mons_col_group_winter    = $mons_col_group_autumn + $mons_col_group_season_data_num;
  $mons_col_group_end       = $mons_col_group_winter + $mons_col_group_season_data_num;

  for( my $i=0; $i<$mons_num; $i++ )
  {
    # year
    $mons_tbl[$i][$mons_col_group_year] = 0;

    # unknown
    $mons_tbl[$i][$mons_col_group_spring + $mons_col_group_season_ofs_unknown] = 0;
    $mons_tbl[$i][$mons_col_group_summer + $mons_col_group_season_ofs_unknown] = 0;
    $mons_tbl[$i][$mons_col_group_autumn + $mons_col_group_season_ofs_unknown] = 0;
    $mons_tbl[$i][$mons_col_group_winter + $mons_col_group_season_ofs_unknown] = 0;

    # グループごと
    for( my $j=0; $j<$lead_zonename_num; $j++ )
    {
      $mons_tbl[$i][$mons_col_group_spring + $mons_col_group_season_ofs_zone_start + $j] = $place_bitflag_none;
      $mons_tbl[$i][$mons_col_group_summer + $mons_col_group_season_ofs_zone_start + $j] = $place_bitflag_none;
      $mons_tbl[$i][$mons_col_group_autumn + $mons_col_group_season_ofs_zone_start + $j] = $place_bitflag_none;
      $mons_tbl[$i][$mons_col_group_winter + $mons_col_group_season_ofs_zone_start + $j] = $place_bitflag_none;
    }
  }
}

##-------------------------------------
### mons_tblに対して、グループをまとめて代表ゾーン名に集約し、出現代表ゾーンチェック入れを行う for グループ
##=====================================
sub MakeMonsTblLeadZoneForGroup
{
  for( my $i=0; $i<$mons_num; $i++ )
  {
    for( my $j=0; $j<$zone_num; $j++ )
    {
      my $zonename        = $zoneid_zonename_tbl[$j];
      my $group           = $zonename_to_group_hash{ $zonename };
      my $lead_zonename   = $group_to_lead_zonename_hash{ $group };

      if( !defined( $lead_zonename ) )
      {
        # 代表ゾーン名が不明なゾーン
        die "There is no lead_zonename of \"$zonename\" error, stopped";
      }
      else
      {
        my $lead_zone_idx = $lead_zonename_hash{$lead_zonename};

        $mons_tbl[$i][$mons_col_group_spring + $mons_col_group_season_ofs_zone_start + $lead_zone_idx] |= $mons_tbl[$i][$mons_col_spring + $mons_col_season_ofs_zone_start + $j];
        $mons_tbl[$i][$mons_col_group_summer + $mons_col_group_season_ofs_zone_start + $lead_zone_idx] |= $mons_tbl[$i][$mons_col_summer + $mons_col_season_ofs_zone_start + $j];
        $mons_tbl[$i][$mons_col_group_autumn + $mons_col_group_season_ofs_zone_start + $lead_zone_idx] |= $mons_tbl[$i][$mons_col_autumn + $mons_col_season_ofs_zone_start + $j];
        $mons_tbl[$i][$mons_col_group_winter + $mons_col_group_season_ofs_zone_start + $lead_zone_idx] |= $mons_tbl[$i][$mons_col_winter + $mons_col_season_ofs_zone_start + $j];
      }
    }
  }
}

##-------------------------------------
### 生息地が不明か for グループ
##=====================================
sub CheckUnknownForGroup
{
  my @season_tbl = 
  (
    [ $mons_col_group_spring + $mons_col_group_season_ofs_unknown, $mons_col_group_spring + $mons_col_group_season_ofs_zone_start ],
    [ $mons_col_group_summer + $mons_col_group_season_ofs_unknown, $mons_col_group_summer + $mons_col_group_season_ofs_zone_start ],
    [ $mons_col_group_autumn + $mons_col_group_season_ofs_unknown, $mons_col_group_autumn + $mons_col_group_season_ofs_zone_start ],
    [ $mons_col_group_winter + $mons_col_group_season_ofs_unknown, $mons_col_group_winter + $mons_col_group_season_ofs_zone_start ],
  );

  for( my $i=0; $i<$mons_num; $i++ )
  {
    foreach my $ref (@season_tbl)
    {
      my $unknown = 1;
      for( my $j=0; $j<$lead_zonename_num; $j++ )
      {
        if( $mons_tbl[$i][$$ref[1] + $j] != $place_bitflag_none )
        {
          $unknown = 0;
          last;
        }
      }
      $mons_tbl[$i][$$ref[0]] = $unknown;
    }
  }
}

##-------------------------------------
### 一年中同じ分布か for グループ
##=====================================
sub CheckYearForGroup
{
  my @season_tbl = 
  (
    [ $mons_col_group_spring + $mons_col_group_season_ofs_unknown, $mons_col_group_spring + $mons_col_group_season_ofs_zone_start ],
    [ $mons_col_group_summer + $mons_col_group_season_ofs_unknown, $mons_col_group_summer + $mons_col_group_season_ofs_zone_start ],
    [ $mons_col_group_autumn + $mons_col_group_season_ofs_unknown, $mons_col_group_autumn + $mons_col_group_season_ofs_zone_start ],
    [ $mons_col_group_winter + $mons_col_group_season_ofs_unknown, $mons_col_group_winter + $mons_col_group_season_ofs_zone_start ],
  );

=bid
  # リファレンス版(リファレンス版も変数に代入版もどちらも正しいです)
  for( my $i=0; $i<$mons_num; $i++ )
  {
    my $year = 1;
    my $base = $season_tbl[0];
    
    # unknownの場合
    if( $mons_tbl[$i][$base->[0]] == 1 )  # リファレンス
    {
      for( my $s=1; $s<4; $s++ )
      {
        my $ref = $season_tbl[$s];
        if( $mons_tbl[$i][$ref->[0]] != 1 )  # リファレンス
        {
          $year = 0;
          last;
        }
      } 
    }
    # unknownでない場合
    else
    {
      for( my $s=1; $s<4; $s++ )
      {
        my $ref = $season_tbl[$s];
        for( my $j=0; $j<$lead_zonename_num; $j++ )
        {
          if( $mons_tbl[$i][$base->[1] + $j] != $mons_tbl[$i][$ref->[1] + $j] )  # リファレンス
          {
            $year = 0;
            last;
          }
        }
      }
    }

    $mons_tbl[$i][$mons_col_group_year] = $year;
  }
=cut

  # 変数に代入版(リファレンス版も変数に代入版もどちらも正しいです)
  for( my $i=0; $i<$mons_num; $i++ )
  {
    my $year = 1;
    my @base = @{$season_tbl[0]};
    
    # unknownの場合
    if( $mons_tbl[$i][$base[0]] == 1 )
    {
      for( my $s=1; $s<4; $s++ )
      {
        my @ref = @{$season_tbl[$s]};
        if( $mons_tbl[$i][$ref[0]] != 1 )
        {
          $year = 0;
          last;
        }
      } 
    }
    # unknownでない場合
    else
    {
      for( my $s=1; $s<4; $s++ )
      {
        my @ref = @{$season_tbl[$s]};
        for( my $j=0; $j<$lead_zonename_num; $j++ )
        {
          if( $mons_tbl[$i][$base[1] + $j] != $mons_tbl[$i][$ref[1] + $j] )
          {
            $year = 0;
            last;
          }
        }
      }
    }

    $mons_tbl[$i][$mons_col_group_year] = $year;
  }

}

##-------------------------------------
### MONSNOごとの分布データをファイルに書き出す for グループ
##=====================================
sub WriteMonsAreaFileForGroup
{
  for( my $i=0; $i<$mons_num; $i++ )
  {
    my $mons_area_file_name = sprintf( "%s%03d%s", $mons_area_pre_file_name, $mons_tbl[$i][$mons_col_monsno], $mons_area_post_file_name );
    open( FH, ">", $mons_area_file_name );
    binmode FH;

    my $buf;
   
    # 一年中同じか否か
    $buf = pack "C", $mons_tbl[$i][$mons_col_group_year];         # 符号なし8ビット整数。(u8だからリトルエンディアンとかない)
    print FH "$buf";
   
    # 春の不明か否かとゾーンごとの生息情報、夏の不明か否かとゾーンごとの生息情報、秋の...
    for( my $j=$mons_col_group_start; $j<$mons_col_group_end; $j++ )
    {
       $buf = pack "C", $mons_tbl[$i][$j];         # 符号なし8ビット整数。(u8だからリトルエンディアンとかない)
       print FH "$buf";
    }

    close( FH );
  }
}

##-------------------------------------
### mons_tblの中身を確認する for グループ
##=====================================
sub DebugWriteMonsAreaFileForGroup
{
  open( FH, ">:encoding(shiftjis)", $debug_mons_file_name );

  print FH "name,monsno,year,unknown,";
  for( my $i=0; $i<$lead_zonename_num; $i++ )
  {
    print FH "$lead_zonename_tbl[$i],";
  }
  print FH "\r\n";  # 0D 0A

  for( my $i=0; $i<$mons_num; $i++ )
  {
    for( my $j=0; $j<=$mons_col_monsno; $j++ )  # monsname, monsno
    {
      print FH "$mons_tbl[$i][$j],";
    }
    print FH "$mons_tbl[$i][$mons_col_group_year],";  # year

    for( my $s=0; $s<4; $s++ )
    {
      my $season_start = 0;

      if( $s == 0 )
      {
        $season_start = $mons_col_group_spring;
      }
      elsif( $s == 1 )
      {
        $season_start = $mons_col_group_summer;
      }
      elsif( $s == 2 )
      {
        $season_start = $mons_col_group_autumn;
      }
      else
      {
        $season_start = $mons_col_group_winter;
      }

      if( $s != 0 )
      {
        for( my $h=0; $h<=$mons_col_monsno; $h++ )  # monsname, monsno
        {
          print FH ",";  # 横に長過ぎるので季節ごとに折り返すことにする
        }
        print FH ",";  # 横に長過ぎるので季節ごとに折り返すことにする  # year
      }

      printf FH "$mons_tbl[$i][$season_start + $mons_col_group_season_ofs_unknown],";
      
      for( my $j=0; $j<$lead_zonename_num; $j++ )
      {
        printf FH "%b,", $mons_tbl[$i][$season_start + $mons_col_group_season_ofs_zone_start + $j];
      }
      print FH "\r\n";  # 0D 0A  # 横に長過ぎるので季節ごとに折り返すことにする
    }
  }

  close( FH );
}

##-------------------------------------
### ゾーンを順番通りにCソースファイルに列挙する
##=====================================
sub WriteLeadRealZoneidFile
{
  my $max_name = "ZKN_AREA_ZONE_GROUP_MAX";

  open( FH, ">:encoding(shiftjis)", $lead_real_zoneid_file_name );

  # 説明
  printf FH "//============================================================================\r\n";
  printf FH "/**\r\n";
  printf FH " *  \@file   %s\r\n", $lead_real_zoneid_file_name;
  printf FH " *  \@brief  zkn_area_monsno_???.datに書かれているゾーンを書かれている順番通りに列挙しておく\r\n";
  printf FH " *  \@author zkn_encount_make.pl\r\n";
  printf FH " *  \@data   \r\n";
  printf FH " *  \@note   zkn_encount_make.plで生成されました。\r\n";
  printf FH " *           Cソースファイルにインクルードして使用して下さい。\r\n";
  printf FH " *\r\n";
  printf FH " *  モジュール名：\r\n";
  printf FH " */\r\n";
  printf FH "//============================================================================\r\n";
  printf FH "#pragma once\r\n";
  printf FH "\r\n";  # 0D 0A

  # 個数
  printf FH "#define %s (%d)\r\n", $max_name, $lead_zonename_num;
  printf FH "\r\n";  # 0D 0A

  # 配列
  printf FH "static const u16 zkn_area_zone_group[%s] =\r\n", $max_name;
  printf FH "{\r\n";

  for( my $i=0; $i<$lead_zonename_num; $i++ )
  {
    printf FH "  %s%s,  // %d\r\n", $lead_real_zoneid_prefix, $lead_zonename_tbl[$i], $zonename_to_real_zoneid_hash{ $lead_zonename_tbl[$i] };
  }

  printf FH "};\r\n";

  close( FH );
}

##-------------------------------------
### バージョンによって変更するものを変更しておく
##=====================================
sub InitVersion
{
  if( $version eq $version_w )
  {
    $mons_area_pre_file_name  = $mons_area_pre_file_name_w;
    $mons_area_file_list_name = $mons_area_file_list_name_w;
  }
  else
  {
    $mons_area_pre_file_name  = $mons_area_pre_file_name_b;
    $mons_area_file_list_name = $mons_area_file_list_name_b;
  }
}

