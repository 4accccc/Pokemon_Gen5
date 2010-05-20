#=============================================================================================
#
#	スタッフロール：コマンドデータ作成
#	2010/04/23 by nakahiro
#
#=============================================================================================

#---------------------------------------------------------------------------------------------
#	定数定義
#---------------------------------------------------------------------------------------------


#---------------------------------------------------------------------------------------------
#	グローバル変数
#---------------------------------------------------------------------------------------------
$CSV_FILE = $ARGV[0];		# 元ファイル
$CNV_FILE = 0;				# 作成ファイル

# ラベルテーブル
@LabelName = (
	"一括",
	"クリア",
	"スクロール開始",
	"スクロール停止",
	"終了",
	"ロゴ",
	"３Ｄ開始",
	"３Ｄ停止",
	"３Ｄカメラ移動",
	"バージョン別",
);
$LABEL_NAME_MAX = 10;

# ラベル変換値
@LabelConv = (
	1,		# ITEMLIST_LABEL_STR_PUT
	2,		# ITEMLIST_LABEL_STR_CLEAR
	3,		# ITEMLIST_LABEL_SCROLL_START
	4,		# ITEMLIST_LABEL_SCROLL_STOP
	5,		# ITEMLIST_LABEL_END
	6,		# ITEMLIST_LABEL_LOGO_PUT
	7,		# ITEMLIST_LABEL_3D_PUT
	8,		# ITEMLIST_LABEL_3D_CLEAR
	9,		# ITEMLIST_LABEL_3D_CAMERA_REQUEST
	10,		# ITEMLIST_LABEL_VERSION
);

# 表示モードテーブル
@PutMode = (
	"左",
	"右",
	"中央",
);
$PUT_MODE_MAX = 3;

# フォントテーブル
@FontType = (
	"大",
	"小",
);
$FONT_TYPE_MAX = 2;

# フォントカラーテーブル
@FontColor = (
	"黒",
	"赤",
	"青",
	"黄",
	"緑",
	"オレンジ",
	"ピンク",
);
$FONT_COLOR_MAX = 7;


#---------------------------------------------------------------------------------------------
#	メイン処理
#---------------------------------------------------------------------------------------------

&SUB_MakeCnvFileName();	# コンバートファイル名作成
&SUB_DataConv();		# データコンバート

exit;


#---------------------------------------------------------------------------------------------
#	コンバートファイル名作成
#---------------------------------------------------------------------------------------------
sub SUB_MakeCnvFileName {
	$CNV_FILE = $CSV_FILE;
	$CNV_FILE =~ s/.csv/.dat/;
}


#---------------------------------------------------------------------------------------------
#	データコンバート
#---------------------------------------------------------------------------------------------

sub SUB_DataConv {
#	my( $write, $val );

	# 元ファイルオープン
	open( FP_CSV, "< " . $CSV_FILE );
	$fp_csv = <FP_CSV>;					# ダミー
	@fp_csv = <FP_CSV>;
	close( FP_CSV );

	# 作成ファイルオープン
	open( FP_CNV, "> " . $CNV_FILE );
	binmode( FP_CNV );

	$msgIdx = 0;

	$cnt = 0;
	foreach $one ( @fp_csv ){
		$one =~ s/\r/\n/g;				# 改行が\rのものと\nのものがあるので、\rを\nに置換する
		$one =~ s/\n/,\n/g;				# "\n"を",\n"に変更
		@line = split( ",", $one );		# ","で分割

# typedef struct {
#	u32	msgIdx;
#
#	u16	wait;
#	u8	label;
#	u8	labelType;
#
#	u8	color;
#	u8	font;
#	u16	putMode;
#
#	s16	px;
#	s16	offs_x;
# }ITEMLIST_DATA;

		# 文字列 [0]
		if( $line[0] eq "" ){
			$val = 0xffff + 0;
#			print( "[0] ". $val . ", " );
			$write = pack "V", $val;
			print( FP_CNV $write );
		}else{
			$val = $msgIdx + 0;
#			print( "[0] ". $val . ", " );
			$write = pack "V", $val;
			print( FP_CNV $write );
			$msgIdx++;
		}

		# ウェイト / Ｙ座標 [3]
		if( $line[3] eq "" ){
			$val = 0 + 0;
#			print( "[1] ". $val . ", " );
			$write = pack "v", $val;
			print( FP_CNV $write );
		}else{
			$val = $line[3] + 0;
#			print( "[1] ". $val . ", " );
			$write = pack "v", $val;
			print( FP_CNV $write );
		}

		# ラベル [1]
		if( $line[1] eq "" ){
			$val = 0 + 0;
#			print( "[2] ". $val . ", \n" );
			$write = pack "C", $val;
			print( FP_CNV $write );
		}else{
#			print( $line[1] . ", \n" );
			for( $i=0; $i<$LABEL_NAME_MAX; $i++ ){
#				print( $LabelName[$i] . ", \n" );
				if( $line[1] eq $LabelName[$i] ){
					$val = $LabelConv[$i] + 0;
#					print( "[2] ". $line[1] . " " . $val . ", \n" );
#					print( "[2] ". $val . ", \n" );
					$write = pack "C", $val;
					print( FP_CNV $write );
					last;
				}
			}
		}

		# ラベルタイプ [2]
		if( $line[2] eq "" ){
			$val = 0 + 0;
#			print( "[3] ". $val . ", " );
			$write = pack "C", $val;
			print( FP_CNV $write );
		}else{
			$val = $line[2] + 0;
#			print( "[3] ". $val . ", " );
			$write = pack "C", $val;
			print( FP_CNV $write );
		}

		# カラー [8]
		if( $line[8] eq "" ){
			$val = 0 + 0;
#			print( "[4] ". $val . ", " );
			$write = pack "C", $val;
			print( FP_CNV $write );
		}else{
			for( $i=0; $i<$FONT_COLOR_MAX; $i++ ){
				if( $line[8] eq $FontColor[$i] ){
					$val = $i + 0;
#					print( "[4] ". $val . ", " );
					$write = pack "C", $val;
					print( FP_CNV $write );
					last;
				}
			}
		}

		# フォント [7]
		if( $line[7] eq "" ){
			$val = 0 + 0;
#			print( "[5] ". $val . ", " );
			$write = pack "C", $val;
			print( FP_CNV $write );
		}else{
			for( $i=0; $i<$FONT_TYPE_MAX; $i++ ){
				if( $line[7] eq $FontType[$i] ){
					$val = $i + 0;
#					print( "[5] ". $val . ", " );
					$write = pack "C", $val;
					print( FP_CNV $write );
					last;
				}
			}
		}

		# 表示モード [4]
		if( $line[4] eq "" ){
			$val = 0 + 0;
#			print( "[6] ". $val . ", " );
			$write = pack "v", $val;
			print( FP_CNV $write );
		}else{
			for( $i=0; $i<$PUT_MODE_MAX; $i++ ){
				if( $line[4] eq $PutMode[$i] ){
					$val = $i + 0;
#					print( "[6] ". $val . ", " );
					$write = pack "v", $val;
					print( FP_CNV $write );
					last;
				}
			}
		}

		# 表示基準Ｘ座標 [5]
		if( $line[5] eq "" ){
			$val = 0 + 0;
#			print( "[7] ". $val . ", " );
			$write = pack "v", $val;
			print( FP_CNV $write );
		}else{
			$val = $line[5] + 0;
#			print( "[7] ". $val . ", " );
			$write = pack "v", $val;
			print( FP_CNV $write );
		}

		# Ｘ座標オフセット [6]
		if( $line[6] eq "" ){
			$val = 0 + 0;
#			print( "[8] ". $val . "\n" );
			$write = pack "v", $val;
			print( FP_CNV $write );
		}else{
			$val = $line[6] + 0;
#			print( "[8] ". $val . "\n" );
			$write = pack "v", $val;
			print( FP_CNV $write );
		}

#		print( $line[1] . "\n" );
	}

	close( FP_CNV );
}
