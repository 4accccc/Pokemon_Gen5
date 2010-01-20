#====================================================================================
#
#	nceファイルから板ポリ１枚で描画するパラメータを抽出する
#
#====================================================================================
use File::Basename;

use constant FORMAT_ID		=>	4;
use constant BYTE_ORDER		=>	2;
use constant FORMAT_VERSION	=>	2;
use constant FILE_SIZE		=>	4;
use constant HEADER_SIZE	=>	2;
use constant DATA_BLOCKS	=>	2;

use constant NCE_HEADER_SIZE=>	FORMAT_ID + BYTE_ORDER + FORMAT_VERSION + FILE_SIZE + HEADER_SIZE + DATA_BLOCKS;

use constant BLOCK_TYPE		=>	4;
use constant BLOCK_SIZE		=>	4;
use constant CELLS			=>	4;
use constant OBJS			=>	4;

use constant CELL_HEADER_SIZE=>	BLOCK_TYPE + BLOCK_SIZE + CELLS;

use constant OAM_POS_X		=>	2;
use constant OAM_POS_Y		=>	2;
use constant OAM_RS_MODE	=>	1;
use constant OAM_RS_PARAM	=>	1;
use constant OAM_HFLIP		=>	1;
use constant OAM_VFLIP		=>	1;
use constant OAM_RSD_ENABLE	=>	1;
use constant OAM_OBJ_MODE	=>	1;
use constant OAM_MOSAIC		=>	1;
use constant OAM_COLOR_MODE	=>	1;
use constant OAM_SHAPE		=>	1;
use constant OAM_SIZE		=>	1;
use constant OAM_PRIORITY	=>	1;
use constant OAM_COLOR_PARAM=>	1;
use constant OAM_CHAR_NAME	=>	2;
use constant PADDING		=>	2;

use constant OBJ_SIZE		=>	OAM_POS_X +		OAM_POS_Y +			 OAM_RS_MODE +		OAM_RS_PARAM + 
								OAM_HFLIP +		OAM_VFLIP +			 OAM_RSD_ENABLE +	OAM_OBJ_MODE +
								OAM_MOSAIC +	OAM_COLOR_MODE +	 OAM_SHAPE +		OAM_SIZE +
								OAM_PRIORITY +	OAM_COLOR_PARAM +	OAM_CHAR_NAME +		PADDING;

use constant MCSS_SHIFT		=>	8;			#ポリゴン1辺の重み（FX32_SHIFTと同値）

#====================================================================================
#
#	OBJ形状とOBJサイズから導く縦横サイズテーブル
#
#====================================================================================

	@obj_size_x = ( [ 8, 16, 32, 64],
					[16, 32, 32, 64],
					[ 8,  8, 16, 32] );

	@obj_size_y = ( [ 8, 16, 32, 64],
					[ 8,  8, 16, 32],
					[16, 32, 32, 64] );

#====================================================================================
#
#	処理開始
#
#====================================================================================

	$argc = @ARGV;

	if( $argc < 2 ){
		print "error:perl perl_file read_file write_dir\n";
		die;
	}

	$write_file = basename( @ARGV[0], '.nce' );
	$write_file = @ARGV[1] . $write_file . '.NCEC';

	open( READ_NCE, @ARGV[0] ) or die "[@ARGV[0]]", $!;
	open( WRITE_NCE, "> $write_file") or die "[$write_file]", $!;

	binmode READ_NCE;
	binmode WRITE_NCE;

	#ヘッダーデータを読み込み
	read READ_NCE, $header, NCE_HEADER_SIZE; 

	#読み込んだデータをセパレート
	($format_id, $byte_order, $format_version, $file_size, $header_size, $data_blocks) = unpack "a4 S S L S S", $header;

	if( $format_id ne "NCOB" ){
		print "NitroCharacterのnceファイルではありません\n";
		die;
	}

	#セルデータのヘッダーを読み込み
	read READ_NCE, $header, CELL_HEADER_SIZE; 

	#読み込んだデータをセパレート
	($block_type, $block_size, $cells) = unpack "a4 L L", $header;

	if( $block_type ne "CELL" ){
		print "NitroCharacterのnceファイルではありません\n";
		die;
	}

	#セルの枚数を書き出す
	$write = pack "L",$cells;
	print WRITE_NCE $write;

  @write_data;
  @cell_index;
  @cell_min_x;
  @cell_min_y;
  @cell_max_x;
  @cell_max_y;

	#セルデータ読み込み
	for( $cell = 0 ; $cell < $cells ; $cell++ ){
		#セルを構成するOBJの数を読み込み
		read READ_NCE, $objs, OBJS;
		$objs = unpack "L", $objs;
		$mepachi = 0;
		$mepachi_min_x = 0;
		$mepachi_min_y = 0;
		$mepachi_max_x = 0;
		$mepachi_max_y = 0;
		$mepachi_size_x = 0;
		$mepachi_size_y = 0;
		$mepachi_char = 0;
		$min_x		= 256;
		$max_x		= -256;
		$min_y		= 256;
		$max_y		= -256;
		$min_max_flag = 0;
		$dress_up_cell = 0;
		for( $obj = 0 ; $obj < $objs ; $obj++ ){
			#OBJデータ読み込み
			read READ_NCE, $obj_data, OBJ_SIZE;
			#OBJデータをセパレート
			( $oam_pos_x,
			  $oam_pos_y,
			  $oam_rs_mode,
			  $oam_rs_param,
			  $oam_hflip,
			  $oam_vflip,
			  $oam_rsd_enable,
			  $oam_obj_mode,
			  $oam_mosaic,
			  $oam_color_mode,
			  $oam_shape,
			  $oam_size,
			  $oam_priority,
			  $oam_color_param,
			  $oam_char_name,
			  $padding ) = unpack "s s C C C C C C C C C C C C S S", $obj_data;

			#倍角フラグがONのときの補正計算をする
			if( $oam_rsd_enable ){
				$x1 = $oam_pos_x + ( $obj_size_x[$oam_shape][$oam_size] / 2 ) ;
				$x2 = $x1 + $obj_size_x[$oam_shape][$oam_size];
				$y1 = $oam_pos_y + ( $obj_size_y[$oam_shape][$oam_size] / 2 );
				$y2 = $y1 + $obj_size_y[$oam_shape][$oam_size];
			}
			else{
				$x1 = $oam_pos_x;
				$x2 = $oam_pos_x+$obj_size_x[$oam_shape][$oam_size];
				$y1 = $oam_pos_y;
				$y2 = $oam_pos_y+$obj_size_y[$oam_shape][$oam_size];
			}

			#OBJモードがONのときはメパチ用キャラなので、別コンバートする
			if( $oam_obj_mode ){
				#OBJのサイズを見て各座標の最小値と最大値を求めておく
				if( $mepachi == 0 ){
					$mepachi_min_x	= $x1;
					$mepachi_max_x	= $x2;
					$mepachi_min_y	= $y1;
					$mepachi_max_y	= $y2;
					$mepachi_char	= $oam_char_name;
				}
				else{
					if( $mepachi_min_x > $x1 ){
						$mepachi_min_x	= $x1;
						$mepachi_char	= $oam_char_name;
					}
					if( $mepachi_max_x < $x2 ){
						$mepachi_max_x = $x2;
					}
					if( $mepachi_min_y > $y1 ){
						$mepachi_min_y	= $y1;
						$mepachi_char	= $oam_char_name;
					}
					if( $mepachi_max_y < $y2 ){
						$mepachi_max_y = $y2;
					}
				}
				$mepachi++;
			}
			else{
				#OBJのサイズを見て各座標の最小値と最大値を求めておく
				if( $min_x > $x1 ){
					$min_x		= $x1;
					$char_name	= $oam_char_name;
				}
				if( $max_x < $x2 ){
					$max_x = $x2;
				}
				if( $min_y > $y1 ){
					$min_y		= $y1;
					$char_name	= $oam_char_name;
				}
				if( $max_y < $y2 ){
					$max_y = $y2;
				}
				$min_max_flag = 1;
			}
		}
		if( $min_max_flag ){
			$size_x = $max_x - $min_x;
			$size_y = $max_y - $min_y;
		}
		else{
			$size_x = 0;
			$size_y = 0;
		}

    push @cell_min_x, $min_x;
    push @cell_min_y, $min_y;
    push @cell_max_x, $max_x;
    push @cell_max_y, $max_y;

=pod
		print "cell:$cell min_x:$min_x min_y:$min_y max_x:$max_x max_y:$max_y\n";
		print "size_x:$size_x size_y:$size_y char_name:$char_name\n";
=cut

		$mepachi_size_x = $mepachi_max_x - $mepachi_min_x;
		$mepachi_size_y = $mepachi_max_y - $mepachi_min_y;

		$min_y *= -1;
		$mepachi_min_y *= -1;

=pod
		#セルの情報を書き出す
		$write = pack "S C C l l S C C l l", $char_name, $size_x, $size_y, $min_x, $min_y, $mepachi_char, $mepachi_size_x, $mepachi_size_y, $mepachi_min_x, $mepachi_min_y;
		print WRITE_NCE $write;
=cut

#前もって計算をしておくコンバート処理
#=pod
		$min_x = $min_x << MCSS_SHIFT;
		$min_y = $min_y << MCSS_SHIFT;
		$size_x = $size_x << 12;
		$size_y = $size_y << 12;
		$tex_s = ( ( $char_name % 32 ) * 8 ) << 12;
		$tex_t = ( ( $char_name >> 5 ) * 8 ) << 12;

#デバッグ表示
=pod
		$tex_ss = ( ( $char_name % 32 ) * 8 );
		$tex_tt = ( ( $char_name >> 5 ) * 8 );
		print "tex_s:$tex_ss\n";
		print "tex_t:$tex_tt\n\n";
=cut

		$mepachi_min_x = $mepachi_min_x << MCSS_SHIFT;
		$mepachi_min_y = $mepachi_min_y << MCSS_SHIFT;
		$mepachi_size_x = $mepachi_size_x << 12;
		$mepachi_size_y = $mepachi_size_y << 12;
		$mepachi_tex_s = ( ( $mepachi_char % 32 ) * 8 ) << 12;
		$mepachi_tex_t = ( ( $mepachi_char >> 5 ) * 8 ) << 12;

		#セルの情報を書き出す
#		$write = pack "l l l l l l l l l l l l", $min_x, $min_y, $size_x, $size_y, $tex_s, $tex_t, $mepachi_min_x, $mepachi_min_y, $mepachi_size_x, $mepachi_size_y, $mepachi_tex_s, $mepachi_tex_t;
#		print WRITE_NCE $write;
		push @write_data , pack "l l l l l l l l l l l l", $min_x, $min_y, $size_x, $size_y, $tex_s, $tex_t, $mepachi_min_x, $mepachi_min_y, $mepachi_size_x, $mepachi_size_y, $mepachi_tex_s, $mepachi_tex_t;
#		print WRITE_NCE $write;
#=cut
	}

  #静止アニメ用データ生成

  #読み飛ばすデータの処理
  @skip_block = ( "CNUM", "CHAR", "GRP ", "ANIM", "ACTL", "MODE", "LABL", "CMNT", "CCMT" );
  $skip_cnt = @skip_block;

	for( $skip = 0 ; $skip < $skip_cnt ; $skip++ ){
	  #ヘッダーを読み込み
  	read READ_NCE, $header, BLOCK_TYPE + BLOCK_SIZE; 
  	($block_type, $block_size) = unpack "a4 L", $header;

  	if( $block_type ne @skip_block[ $skip ] ){
  	  if( @skip_block[ $skip ] eq "CHAR" ){
        $skip++;
      }
  	  if( $block_type ne @skip_block[ $skip ] ){
        print @skip_block[ $skip ] . "\n";
  	  	print "NitroCharacterのnceファイルではありません\n";
  	  	die;
      }
  	}
    if( $block_type eq "ANIM" ){
      read READ_NCE, $data, 4;
  	  ($cell_anms) = unpack "L", $data;
      for( $i = 0 ; $i < $cell_anms ; $i++ ){
        read READ_NCE, $data, 12;
  	    ($anm_label, $anm_cmnt, $cells) = unpack "L L L", $data;
        for( $j = 0 ; $j < $cells ; $j++ ){
          read READ_NCE, $data, 24;
  	      ($index, $wait, $rot, $scaleX, $scaleY, $transX, $transY) = unpack "S S l l l l l", $data;
          if( $j == 0 ){
            push @cell_index, $index;
          }
        }
      }
    }
    else{
      #ブロックサイズ分読み飛ばし
  	  read READ_NCE, $header, $block_size - 8;
    }
  }

  #ヘッダーを読み込み
	read READ_NCE, $header, 12; 
 	($block_type, $block_size, $cell_anim_max) = unpack "a4 L L", $header;

 	if( $block_type ne "ECMT" ){
    print "ECMT\n";
    print "NitroCharacterのnceファイルではありません\n";
 	  die;
 	} 

  for( $cell_anim = 0 ; $cell_anim < $cell_anim_max ; $cell_anim++ ){
	  read READ_NCE, $header, 4; 
 	  ($comment_size) = unpack "L", $header;
	  read READ_NCE, $data, $comment_size; 
 	  ($comment) = unpack "a4", $data;
    if( $comment eq "stop" ){
      push( @stop_cell, 1 ); 
    }
    else{
      push( @stop_cell, 0 ); 
    }
  }
	close READ_NCE;

  #nmcファイル読み込み
  $read_file = dirname( @ARGV[0] ) . "/" . basename( @ARGV[0], ".nce" ) . ".nmc";

	open( READ_NMC, $read_file ) or die "[$read_file]", $!;
	binmode READ_NMC;

	#ヘッダーデータを読み込み
	read READ_NMC, $header, NCE_HEADER_SIZE; 

	#読み込んだデータをセパレート
	($format_id, $byte_order, $format_version, $file_size, $header_size, $data_blocks) = unpack "a4 S S L S S", $header;

	if( $format_id ne "NCMC" ){
    print "NitroCharacterのnmcファイルではありません\n";
 	  die;
  }

  #ヘッダーを読み込み
	read READ_NMC, $header, 12; 
 	($block_type, $block_size, $multi_cells) = unpack "a4 L L", $header;

	if( $block_type ne "MCEL" ){
    print "NitroCharacterのnmcファイルではありません\n";
 	  die;
  }

  for( $mc = 0 ; $mc < $multi_cells ; $mc++ ){
	  read READ_NMC, $header, 4; 
 	  ($cell_anms) = unpack "L", $header;
    push( @cell_anm_max, $cell_anms ); 
    for( $ca = 0 ; $ca < $cell_anms ; $ca++ ){
	    read READ_NMC, $header, 12; 
 	    ($cell_anm_pos_x, $cell_anm_pos_y, $cell_anm_index) = unpack "l l L", $header;
      $mcell_anms[$mc][$ca] = $cell_anm_index;

      $index = $cell_index[ $cell_anm_index ];

		  if( $size_min_x > $cell_anm_pos_x + $cell_min_x[ $index ] ){
			  $size_min_x	= $cell_anm_pos_x + $cell_min_x[ $index ];
		  }
		  if( $size_max_x < $cell_anm_pos_x + $cell_max_x[ $index ] ){
		    $size_max_x = $cell_anm_pos_x + $cell_max_x[ $index ];
		  }
		  if( $size_min_y > $cell_anm_pos_y + $cell_min_y[ $index ] ){
			  $size_min_y	= $cell_anm_pos_y + $cell_min_y[ $index ];
		  }
		  if( $size_max_y < $cell_anm_pos_y + $cell_max_y[ $index ] ){
		    $size_max_y = $cell_anm_pos_y + $cell_max_y[ $index ];
		  }
    }
  }

  #マルチセルのおおよその大きさを算出
  $size_x = $size_max_x - $size_min_x;
  $size_y = $size_max_y - $size_min_y;

  $write = pack "s s", $size_x, $size_y;
  print WRITE_NCE $write;

  for( $i = 0 ; $i < @write_data ; $i++ )
  {
    print WRITE_NCE $write_data[ $i ];
  }

  #ヘッダーを読み込み
	read READ_NMC, $header, 8; 
 	($block_type, $block_size) = unpack "a4 L", $header;

	if( $block_type ne "CCTL" ){
    print "NitroCharacterのnmcファイルではありません\n";
 	  die;
  }

  #ブロックサイズ分読み飛ばし
	read READ_NMC, $header, $block_size - 8;

  #ヘッダーを読み込み
	read READ_NMC, $header, 8; 
 	($block_type, $block_size) = unpack "a4 L", $header;

	if( $block_type ne "GRP " ){
    print "NitroCharacterのnmcファイルではありません\n";
 	  die;
  }

  #ブロックサイズ分読み飛ばし
	read READ_NMC, $header, $block_size - 8;

  #ヘッダーを読み込み
	read READ_NMC, $header, 12; 
 	($block_type, $block_size, $multi_cell_anms) = unpack "a4 L L", $header;

	if( $block_type ne "ANIM" ){
    print "NitroCharacterのnmcファイルではありません\n";
 	  die;
  }

  @node = ();

  if( $multi_cell_anms > 1 ){
    for( $mca = 0 ; $mca < $multi_cell_anms ; $mca++ ){
	    read READ_NMC, $header, 12; 
      ($label_index, $cmnt_index, $multi_cells) = unpack "L L L", $header;
      for( $mc = 0 ; $mc < $multi_cells ; $mc++ ){
	      read READ_NMC, $header, 24; 
 	      ($cell_index, $frame, $rot, $scale_x, $scale_y, $trans_x, $trans_y) = unpack "S S L L L L L", $header;
        for( $i = 0; $i < $cell_anm_max[$cell_index] ; $i++ ){
          $sc = $mcell_anms[$cell_index][$i];
          if( $stop_cell[$sc] == 1 ){
            push( @node, $i );
          }
        }
      }
    }
  }
  else{
	  read READ_NMC, $header, $block_size - 12;
  }

 	read READ_NMC, $header, BLOCK_TYPE + BLOCK_SIZE; 
 	($block_type, $block_size) = unpack "a4 L", $header;

	if( $block_type ne "ACTL" ){
    print "NitroCharacterのnmcファイルではありません\n";
 	  die;
  }

  #ブロックサイズ分読み飛ばし
	read READ_NMC, $header, $block_size - 8;

 	read READ_NMC, $header, 12; 
 	($block_type, $block_size, $multi_cell_anms) = unpack "a4 L L", $header;

	if( $block_type ne "LABL" ){
    print "NitroCharacterのnmcファイルではありません\n";
 	  die;
  }

 	read READ_NMC, $data, 64; 
  ($label) = unpack "a7", $data;

  $non_stop = 0;

  if( $label eq "nonstop" ){
    $non_stop = 1;
  }

  $node_cnt = @node;
  
  if( $non_stop == 1 ){
		$write = pack "L", 0x000000ff;
	  print WRITE_NCE $write;
  }
  elsif( $node_cnt == 0 ){
		$write = pack "L", 0;
	  print WRITE_NCE $write;
  }
  else{
    $padding = 3 - $node_cnt;
		$write = pack "C", $node_cnt;
	  print WRITE_NCE $write;
    for( $i = 0 ; $i < $node_cnt ; $i++ ){
		  $write = pack "C", $node[$i];
	    print WRITE_NCE $write;
    }
    for( $i = 0 ; $i < $padding ; $i++ ){
		  $write = pack "C", 0;
	    print WRITE_NCE $write;
    }
  }

	close READ_NMC;
	close WRITE_NCE;


