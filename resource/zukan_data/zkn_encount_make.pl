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

# 定数
# encount
#$encount_csv_row_data_start           =   0;  # 最初から最後まで全て読み込むようにし、開始終了の指定はしないことにする。
#$encount_csv_row_data_end             = 101;  # encount_csv_row_data_start<=データ<encount_csv_row_data_end
$encount_csv_col_zonename             =   1;  # ゾーン名(地名を更に細かく分けたもの)。半角英数。
$encount_csv_col_season               =   2;  # 季節名。春/夏/秋/冬。
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
  "夏"=>$season_bitflag_spring,
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
%monsno_hash = ();  # ポケモン名からMONSNOを得るためのハッシュ  # 使用しなかった

# mons
$mons_num;
@mons_tbl = ();
$mons_col_monsname       = 0;
$mons_col_monsno         = 1;
$mons_col_unknown        = 2;
$mons_col_year           = 3;
$mons_col_spring         = 4;
$mons_col_summer;      # = $mons_col_spring + $zone_num;
$mons_col_autumn;      # = $mons_col_summer + $zone_num;
$mons_col_winter;      # = $mons_col_autumn + $zone_num;
$mons_col_end;         # = $mons_col_winter + $zone_num;
%mons_tbl_idx_hash = ();  # ポケモン名からmons_tblのインデックスを得るためのハッシュ
$mons_area_pre_file_name  = "zkn_area_monsno_";  # リトルエンディアンのバイナリ
$mons_area_post_file_name = ".dat";              # 例：zkn_area_monsno_001.dat
$debug_mons_file_name = "debug_zkn_encount_make_mons.csv";  # shiftjis

# zone
$zoneseason_num;  # 「ゾーンAの春」「ゾーンAの夏」を別ものとして数えたときの個数
@zoneseason_tbl = ();
$zoneseason_col_zonename          = 0;
$zoneseason_col_zoneid            = 1;  # zone_hashで決めたゾーンID
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
$zone_num;  # 「ゾーンAの春」「ゾーンAの夏」を同じものとして数えたときの個数
%zone_hash = ();  # ゾーン名からゾーンIDを得るためのハッシュ
                  # ゾーンIDはこのファイル内で決めたものなので、pokemon_wbのものと値が異なる可能性あり！
                  # ゾーンIDは@mons_tblのcolumnの個数にも関わっているので、必ず0<=ゾーンID<zone_numを満たすこと！
@zoneid_zonename_tbl = ();  # zone_hashを用いて $zoneid_zonename_tbl[ $zone_hash{ ゾーン名 } ] = ゾーン名 となるような配列を作成する
%zone_hash_for_season = ();  # ゾーンごとの季節を記録しておく  # $zone_numカウント用

# MONSNOごとの分布データのファイルのリスト
$mons_area_file_list_name = "zkn_area_file_list.lst";  # shiftjis

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

# ゾーン名を地名ごとにまとめる


# 生息地が不明か


# 一年中同じ分布か


# MONSNOごとの分布データをファイルに書き出す
&WriteMonsAreaFile();
&DebugWriteMonsAreaFile();  #debug

# MONSNOごとの分布データのファイルのリストを書き出す
&WriteMonsAreaFileList();

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

    my $zone_season_bitflag = $zone_hash_for_season{ $values[$encount_csv_col_zonename] };
    if( !defined($zone_season_bitflag) )
    {
      $zone_hash_for_season{ $values[$encount_csv_col_zonename] } = $season_bitflag_tbl{ $values[$encount_csv_col_season] };
      $zone_hash{ $values[$encount_csv_col_zonename] } = $zone_num;
      $zone_num++;
    }
    else
    {
      if( $zone_hash_for_season{ $values[$encount_csv_col_zonename] } & $season_bitflag_tbl{ $values[$encount_csv_col_season] } )
      {
        # 既出の季節
        die "season overlap \"$values[$encount_csv_col_season]\" error, stopped";
      }
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

  $mons_col_summer = $mons_col_spring + $zone_num;
  $mons_col_autumn = $mons_col_summer + $zone_num;
  $mons_col_winter = $mons_col_autumn + $zone_num;
  $mons_col_end    = $mons_col_winter + $zone_num;

  for( my $i=0; $i<$mons_num; $i++ )
  {
    $mons_tbl[$i][$mons_col_unknown] = 0;
    $mons_tbl[$i][$mons_col_year] = 0;
    for( my $j=$mons_col_spring; $j<$mons_col_end; $j++ )
    {
      $mons_tbl[$i][$j] = $place_bitflag_none;
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
              $mons_tbl[ $mons_tbl_idx ][ $$s[1] + $zoneseason_tbl[$i][$zoneseason_col_zoneid] ] |= $$ref[2];
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
    
    $buf = pack "v", $mons_tbl[$i][$mons_col_unknown];      # 符号なし16ビット整数。リトルエンディアン。
    print FH "$buf";
    $buf = pack "v", $mons_tbl[$i][$mons_col_year];         # 符号なし16ビット整数。リトルエンディアン。
    print FH "$buf";
    
    for( my $j=$mons_col_spring; $j<$mons_col_end; $j++ )
    {
       $buf = pack "v", $mons_tbl[$i][$j];
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

  print FH "name,monsno,unknown,year,";
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
    for( my $j=$mons_col_spring; $j<$mons_col_end; $j++ )
    {
      if(    $j==$mons_col_summer
          || $j==$mons_col_autumn
          || $j==$mons_col_winter )  # 横に長過ぎるので季節ごとに折り返すことにする
      {
        print FH "\r\n";  # 0D 0A
        for( my $h=0; $h<$mons_col_spring; $h++ )
        {
          print FH ",";
        }
      }
      printf FH "%b,", $mons_tbl[$i][$j];
#      printf FH Encode::encode( 'shiftjis', "%b,", $mons_tbl[$i][$j] );  # これだとdoes not map to shiftjisというエラーが出てしまう！

    }
    print FH "\r\n";  # 0D 0A
#    print FH Encode::encode( 'shiftjis', "\r\n" );  # 0D 0A  # これだとdoes not map to shiftjisというエラーが出てしまう！
  }

  close( FH );
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

